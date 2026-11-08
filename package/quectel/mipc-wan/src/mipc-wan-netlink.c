/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  Copyright (c) [2020], MediaTek Inc. All rights reserved.
*  This software/firmware and related documentation ("MediaTek Software") are
*  protected under relevant copyright laws.
*
*  The information contained herein is confidential and proprietary to
*  MediaTek Inc. and/or its licensors. Except as otherwise provided in the
*  applicable licensing terms with MediaTek Inc. and/or its licensors, any
*  reproduction, modification, use or disclosure of MediaTek Software, and
*  information contained herein, in whole or in part, shall be strictly
*  prohibited.
*****************************************************************************/

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/ipv6.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/epoll.h>

#include "mipc-wan-common.h"

#define EPOLL_LISTEN_MAX_CNT    32
#define EPOLL_LISTEN_TIMEOUT    10 * 1000 /* ms */
#define NETLINK_BUFFER_SIZE     4096

extern int g_log_hidden;

static inline uint32_t rta_getattr_u32(const struct rtattr *rta)
{
	return *(uint32_t *)RTA_DATA(rta);
}

static inline int is_non_linklocal(const char* ipv6)
{
     if (strncasecmp("FE80", ipv6, strlen("FE80")) == 0) {
         return 0;
     }

     return 1;
}

int nl_netlink_handle(int sock, int ifindex, char* ipv6_addr)
{
    char buffer[NETLINK_BUFFER_SIZE];
    struct nlmsghdr *nlh;
    int s_err;//Check snprintf error status

    memset(&buffer, 0, sizeof(buffer));
    nlh = (struct nlmsghdr *)buffer;

    while (1) {
        int len = recv(sock, nlh, NETLINK_BUFFER_SIZE - 1, 0);
        if (len == -1) break;
        LOGD("rx len:%d \n", len);

        while ((NLMSG_OK(nlh, len)) && (nlh->nlmsg_type != NLMSG_DONE)) {
            LOGD("nlh->nlmsg_type:%d \n", nlh->nlmsg_type);
            if (nlh->nlmsg_type == RTM_NEWADDR) {
                struct ifaddrmsg *ifa = (struct ifaddrmsg *) NLMSG_DATA(nlh);
                struct rtattr *rth = IFA_RTA(ifa);
                int rtl = IFA_PAYLOAD(nlh);

                if (ifa->ifa_index != ifindex) {
                    nlh = NLMSG_NEXT(nlh, len);
                    continue;
                }

                while (rtl && RTA_OK(rth, rtl)) {
                    struct in6_addr *in6p = (struct in6_addr *) RTA_DATA(rth);

                    if (rth->rta_type == IFA_ADDRESS) {
                        s_err = snprintf(ipv6_addr, INET6_ADDRSTRLEN + 1, NIP6_FMT, NIP6(*in6p));
                        if (s_err < 0) {
                            LOGD("snprintf error\n");
                        } else {
                              if (is_non_linklocal(ipv6_addr)) {
                                  if (!g_log_hidden)
                                      LOGD("Return IPv6 addr:%s scope:%d\n", ipv6_addr, ifa->ifa_scope);
                                  else
                                      LOGD("Return IPv6 addr\n");
                                  return 0;
                              }      
                        }  
		    }   
                    rth = RTA_NEXT(rth, rtl);
                }
            }
            nlh = NLMSG_NEXT(nlh, len);
        }
    }

    return -1;
}

int epoll_event_handle(int epoll_fd, int sock_fd, int ifindex, char* ipv6_addr)
{
    int i;
    int sfd;
    struct epoll_event events[EPOLL_LISTEN_MAX_CNT];

    memset(events, 0, sizeof(events));
    while (1)
    {
        /* wait epoll event */
        int fd_cnt = epoll_wait(epoll_fd, events, EPOLL_LISTEN_MAX_CNT, EPOLL_LISTEN_TIMEOUT);
        if (!fd_cnt) {
            LOGE("Timeout to wait for IPv6 event\n");
            return -1;
        }
        LOGD("Process fd count:%d\n", fd_cnt);
        for (i = 0; i < fd_cnt; i++)
        {
            sfd = events[i].data.fd;
            if(events[i].events & EPOLLIN)
            {
                if (sfd == sock_fd)
                {
                    if (!nl_netlink_handle(sfd, ifindex, ipv6_addr)) {
                        LOGD("Get IPv6 address\n");
                        return 0;
                    }
                }
            }
        }
    }
    return -1;
}

int rtnl_receive(int sock, struct msghdr *msg, int flags)
{
    int len;

    /* Get the message */
    do {
        len = recvmsg(sock, msg, flags);
    } while (len < 0 && (errno == EINTR || errno == EAGAIN));

    if (len < 0) {
        LOGE("Can't receive from socket:%d\n", errno);
        return -1;
    } else if (len == 0) {
        LOGD("EOF in the link\n");
        return 0;
    }

    return len;
}

static int rtnl_recvmsg(int sock, struct msghdr *msg, char **answer)
{
    struct iovec *iov = msg->msg_iov;
    char *buf;
    int len;

    iov->iov_base = NULL;
    iov->iov_len = 0;

    /* MSG_PEEK gets only message size in the socket without reading */
    len = rtnl_receive(sock, msg, MSG_PEEK | MSG_TRUNC);
    if (len < 0)
        return len;

    /* Allocate memory for the message */
    buf = malloc(len);
    if (!buf) {
        LOGE("malloc failed\n");
        return -ENOMEM;
    }

    iov->iov_base = buf;
    iov->iov_len = len;

    /* Read message to the buffer */
    len = rtnl_receive(sock, msg, 0);
    if (len < 0) {
        free(buf);
        return len;
    }

    *answer = buf;

    return len;
}

void parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
	memset(tb, 0, sizeof(struct rtattr *) * (max + 1));

	while (RTA_OK(rta, len)) {
		if (rta->rta_type <= max) {
			tb[rta->rta_type] = rta;
		}

		rta = RTA_NEXT(rta,len);
	}
}

int dump_route(struct nlmsghdr* msghdr, char* ifname, char* gw_addr)
{
    struct rtmsg* r = NLMSG_DATA(msghdr);
    int len = msghdr->nlmsg_len;
    struct rtattr* tb[RTA_MAX + 1];
    uint32_t if_idx;

    len -= NLMSG_LENGTH(sizeof(*r));
    if (len < 0) {
        LOGE("Wrong message length:%d:%d:%ld\n", msghdr->nlmsg_len, len, NLMSG_LENGTH(sizeof(*r)));
        return -1;
    }

    if_idx = if_nametoindex(ifname);
    if (!if_idx) {
        LOGE("Can't map iface name(%s) to index:%d\n", ifname, errno);
        return -1;
    }

    /* Parse message */
    parse_rtattr(tb, RTA_MAX, RTM_RTA(r), len);
    if (r->rtm_family != AF_INET6) {
        return -1;
    }

    if (tb[RTA_OIF]) {
        if (rta_getattr_u32(tb[RTA_OIF]) != if_idx)
            return -1;
    }

    /* Do the same thing for rest of the fields */
    if (tb[RTA_GATEWAY]) {
        inet_ntop(AF_INET6, RTA_DATA(tb[RTA_GATEWAY]), gw_addr, INET6_ADDRSTRLEN);
        if (!g_log_hidden)
            LOGD("IPv6 gateway via %s\n", gw_addr);
        else
            LOGD("IPv6 gateway found\n");
        return 0;
    }

    return -1;
}

int get_route_dump_response(int sock, char* ifname, char* gw_addr)
{
    struct sockaddr_nl nladdr;
    struct iovec iov;
    struct msghdr msg = {
        .msg_name = &nladdr,
        .msg_namelen = sizeof(nladdr),
        .msg_iov = &iov,
        .msg_iovlen = 1,
    };
    char *buf = NULL;
    int len = rtnl_recvmsg(sock, &msg, &buf);
    /* Pointer to the messages head */
    struct nlmsghdr *h = (struct nlmsghdr *)buf;
    int msglen = len;

    LOGD("Dump IPv6 default gateway:%d\n", msglen);

    /* Iterate through all messages in buffer */
    while (NLMSG_OK(h, msglen)) {
        if (h->nlmsg_flags & NLM_F_DUMP_INTR) {
            LOGE("Dump was interrupted\n");
            free(buf);
            return -1;
        }

        if (nladdr.nl_pid != 0) {
            continue;
        }

        if (h->nlmsg_type == NLMSG_ERROR) {
            LOGE("netlink reported error");
            free(buf);
            return -1;
        }

        /* Decode and print single message */
        if (!dump_route(h, ifname, gw_addr)) {
            free(buf);
            return 0;
        }

        h = NLMSG_NEXT(h, msglen);
    }

    if (buf)
        free(buf);

    return len;
}

int get_ipv6_default_gw(char* ifname, char* ipv6_addr)
{
    struct sockaddr_nl addr;
    int sock_fd = 0;
    ssize_t sent;
    struct {
        struct nlmsghdr nlh;
        struct rtmsg rtm;
    } nl_request;

    if ((sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) == -1) {
        LOGE("couldn't open NETLINK_ROUTE socket:%d\n", errno);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    if (bind(sock_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        LOGE("couldn't bind:%d\n", errno);
        close(sock_fd);
        return -1;
    }

    memset(&nl_request, 0, sizeof(nl_request));
    nl_request.nlh.nlmsg_type = RTM_GETROUTE;
    nl_request.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nl_request.nlh.nlmsg_len = sizeof(nl_request);
    nl_request.rtm.rtm_family = AF_INET6;
    nl_request.rtm.rtm_flags = RTM_F_CLONED;

    sent = send(sock_fd, &nl_request, sizeof(nl_request), 0);
    if (sent < 0) {
        perror("Failed to perfom request");
        close(sock_fd);
        return -1;
    }

    memset(ipv6_addr, 0, sizeof(*ipv6_addr));
    get_route_dump_response(sock_fd, ifname, ipv6_addr);

    close(sock_fd);
    return 0;
}

int get_global_ipv6_addr(char* ifname, char* ipv6_addr)
{
    struct sockaddr_nl addr;
    int sock_fd = 0;
    int s_err; //check snprintf error
    ssize_t len;
    struct {
        struct nlmsghdr nlh;
        struct ifaddrmsg ifa;
    } nl_request;
    int ifindex = if_nametoindex(ifname);
    char buffer[NETLINK_BUFFER_SIZE];
    struct nlmsghdr *nlh;

    if (!ifindex) {
        LOGE("if_nametoindex failed:%s:%d\n", ifname, errno);
        return -1;
    }

    if ((sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) == -1) {
        LOGE("couldn't open NETLINK_ROUTE socket:%d\n", errno);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    if (bind(sock_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        LOGE("couldn't bind:%d\n", errno);
        close(sock_fd);
        return -1;
    }

    memset(&nl_request, 0, sizeof(nl_request));
    nl_request.nlh.nlmsg_type = RTM_GETADDR;
    nl_request.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nl_request.nlh.nlmsg_len = sizeof(nl_request);
    nl_request.ifa.ifa_family = AF_INET6;
    nl_request.ifa.ifa_index = ifindex;

    len = send(sock_fd, &nl_request, sizeof(nl_request), 0);
    if (len < 0) {
        LOGE("Failed to send request:%d\n", errno);
        close(sock_fd);
        return -1;
    }

    memset(&buffer, 0, sizeof(buffer));
    nlh = (struct nlmsghdr *)buffer;
    len = recv(sock_fd, nlh, NETLINK_BUFFER_SIZE - 1, 0);
    if (len < 0) {
        LOGE("Failed to recv request:%d\n", errno);
        close(sock_fd);
        return -1;
    }

    while ((NLMSG_OK(nlh, len)) && (nlh->nlmsg_type != NLMSG_DONE)) {
        if (nlh->nlmsg_type == RTM_NEWADDR) {
            struct ifaddrmsg *ifa = (struct ifaddrmsg *) NLMSG_DATA(nlh);
            struct rtattr *rth = IFA_RTA(ifa);
            int rtl = IFA_PAYLOAD(nlh);

            if (ifa->ifa_index != ifindex) {
                nlh = NLMSG_NEXT(nlh, len);
                continue;
            }
            while (rtl && RTA_OK(rth, rtl)) {
                struct in6_addr *in6p = (struct in6_addr *) RTA_DATA(rth);

                if (rth->rta_type == IFA_ADDRESS) {
                    memset(ipv6_addr, 0, INET6_ADDRSTRLEN + 1);
                    s_err = snprintf(ipv6_addr, INET6_ADDRSTRLEN + 1 , NIP6_FMT, NIP6(*in6p));
                    if (s_err < 0) {
                        LOGD("snprintf error\n");
                    } else {
                        if (is_non_linklocal(ipv6_addr)) {
                            if (!g_log_hidden)
                                LOGD("Return IPv6 addr:%s scope:%d %d\n", ipv6_addr, ifa->ifa_scope, ifa->ifa_prefixlen);
                            else
                                LOGD("Return IPv6 addr\n");
                            close(sock_fd);
                            return 0;
                         }
                    }  
                }
                rth = RTA_NEXT(rth, rtl);
            }
        }
        nlh = NLMSG_NEXT(nlh, len);
    }

    LOGD("No global IPv6 addr\n");
    close(sock_fd);
    return -1;
}

int check_global_ipv6_addr(char* ifname, char* ipv6_addr)
{
    struct sockaddr_nl addr;
    struct epoll_event ev;
    int epoll_fd = 0, sock_fd = 0;
    struct timeval tv;
    int ifindex = if_nametoindex(ifname);
	int ret;

    LOGD("Run %s\n", __func__);

    if (!ifindex) {
        LOGE("if_nametoindex failed:%s:%d\n", ifname, errno);
        return -1;
    }

    epoll_fd = epoll_create(EPOLL_LISTEN_MAX_CNT);
    if (epoll_fd < 0) {
        LOGE("epoll create failure:%d\n", errno);
        return -1;
    }

    if ((sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) == -1) {
        LOGE("couldn't open NETLINK_ROUTE socket:%d\n", errno);
        goto err;
    }
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) == -1) {
        LOGE("couldn't set setsockopt socket:%d\n", errno);
        goto err;
    }

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = RTMGRP_IPV6_IFADDR | RTMGRP_IPV6_PREFIX;

    if (bind(sock_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        LOGE("couldn't bind:%d\n", errno);
        goto err;
    }

    ev.data.fd = sock_fd;
    ev.events = EPOLLIN | EPOLLET;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &ev) < 0) {
        LOGE("epoll add sock_fd error");
        goto err;
    }

    ret = epoll_event_handle(epoll_fd, sock_fd, ifindex, ipv6_addr);

    close(epoll_fd);
    close(sock_fd);

    return ret;

err:
    close(epoll_fd);
    if (sock_fd >= 0)
        close(sock_fd);

    return -1;
}
