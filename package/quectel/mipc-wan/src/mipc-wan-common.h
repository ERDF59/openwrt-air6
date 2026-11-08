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

#ifndef MIPC_WAN_COMMON_H
#define MIPC_WAN_COMMON_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#define LOGD(fmt, ...) syslog(LOG_DEBUG, "%s:" fmt, __func__, ## __VA_ARGS__)
#define LOGI(fmt, ...) syslog(LOG_INFO, "%s:" fmt, __func__, ## __VA_ARGS__)
#define LOGW(fmt, ...) syslog(LOG_WARNING, "%s:" fmt, __func__, ## __VA_ARGS__)
#define LOGE(fmt, ...) syslog(LOG_ERR, "%s:" fmt, __func__, ## __VA_ARGS__)

#define NIP6_FMT "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x"
#define NIP6(addr)                  \
    (unsigned)((addr).s6_addr[0]),          \
    (unsigned)((addr).s6_addr[1]),          \
    (unsigned)((addr).s6_addr[2]),          \
    (unsigned)((addr).s6_addr[3]),          \
    (unsigned)((addr).s6_addr[4]),          \
    (unsigned)((addr).s6_addr[5]),          \
    (unsigned)((addr).s6_addr[6]),          \
    (unsigned)((addr).s6_addr[7]),          \
    (unsigned)((addr).s6_addr[8]),          \
    (unsigned)((addr).s6_addr[9]),          \
    (unsigned)((addr).s6_addr[10]),         \
    (unsigned)((addr).s6_addr[11]),         \
    (unsigned)((addr).s6_addr[12]),         \
    (unsigned)((addr).s6_addr[13]),         \
    (unsigned)((addr).s6_addr[14]),         \
    (unsigned)((addr).s6_addr[15])

int check_global_ipv6_addr(char* ifname, char* ipv6_addr);
int get_global_ipv6_addr(char* ifname, char* ipv6_addr);
int get_ipv6_default_gw(char* ifname, char* ipv6_addr);
int set_odu_br_vlan(char* ifname, int vlan);
int get_odu_br_vlan(char* ifname);
int set_odu_br_mac(char* ifname, char* idu_mac);
int get_odu_br_mac(char* ifname, char* idu_mac);

#define ROUTING_MODE 1
#define BRIDGE_MODE 2

#define MTK_MOBILE_PREFIX_INF_NAME "ccmni"

#endif