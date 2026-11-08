/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "atci_service.h"
#include "atci_generic_cmd_table.h"
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/un.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>


#define SOCKET_NAME_ATCI "/dev/atci-service"

int s_fdAtci_generic_listen = -1;
int s_fdAtci_generic_command = -1;

static pthread_t s_tid_atci_service;

int main_exit_signal = 0;
void setMainExit() {
    LOGATCI(LOG_DEBUG, "atci setMainExit !!! %d", main_exit_signal);
    main_exit_signal++;
    pthread_exit(0);
}

void waitForAtcidData() {
    int ret = 0 ;
    fd_set rfds;

    LOGATCI(LOG_DEBUG, "atci start to wait for atci data !!!");

    for (;;) {
        if (s_fdAtci_generic_command > 0) {
            FD_ZERO(&rfds);
            FD_SET(s_fdAtci_generic_command, &rfds);
            ret = select(s_fdAtci_generic_command+1, &rfds, NULL, NULL, NULL);
            if (ret < 0) {
                if (errno == EINTR || errno == EAGAIN) continue;
                LOGATCI(LOG_ERR, "waitForAtcidData atci-usb fail to select. error (%d)", errno);
                close(s_fdAtci_generic_command);
                s_fdAtci_generic_command = -1;
                return;
            }

            if (FD_ISSET(s_fdAtci_generic_command, &rfds)) {
                int recvLen = 0;
                char data[MAX_DATA_SIZE+1];
                memset(data, 0, sizeof(data));
                recvLen = recv(s_fdAtci_generic_command, data, MAX_DATA_SIZE, 0);
                if(recvLen > 0) {
                    LOGATCI(LOG_DEBUG, "The receive data from ATCID is %s with len:%d", data, recvLen);
                    process_generic_command(data, recvLen);
                } else {
                    LOGATCI(LOG_DEBUG,
                        "atci_service recvLen <= 0, exit, waitForAtcidData, recvLen = %d",
                        recvLen);
                    close(s_fdAtci_generic_command);
                    s_fdAtci_generic_command = -1;
                    return;
                }
            }
        }
    }
}

void* waitForAtcidConnect(void *param){
    UNUSED(param);
    int ret;
    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);
    fd_set rfds;
    LOGATCI(LOG_INFO, "atci start to wait for atcid connect!!!");

    for (;;) {
        FD_ZERO(&rfds);
        FD_SET(s_fdAtci_generic_listen, &rfds);
        LOGATCI(LOG_INFO, "wait client connect");
        ret = select(s_fdAtci_generic_listen+1, &rfds, NULL, NULL, NULL);

        if (ret < 0) {
            if (errno == EINTR || errno == EAGAIN) continue;
            LOGATCI(LOG_ERR, " waitForAtcidConnect fail to select. error (%d)", errno);
            setMainExit();
        }

        if (FD_ISSET(s_fdAtci_generic_listen, &rfds)) {
            LOGATCI(LOG_DEBUG, "select atci service socket successfully!!!");
            s_fdAtci_generic_command = accept(s_fdAtci_generic_listen,
                    (struct sockaddr *)&peeraddr, &socklen);
            if (s_fdAtci_generic_command < 0) {
                LOGATCI(LOG_ERR, "fail to accept atci service socket. errno:%d", errno);
                setMainExit();
            }
            LOGATCI(LOG_DEBUG, "accept atci service socket '%d' successfully!!!", s_fdAtci_generic_command);

            ret = fcntl(s_fdAtci_generic_command, F_SETFL, O_NONBLOCK);
            if (ret < 0) {
                LOGATCI(LOG_ERR, "fail to set atci-usb client socket O_NONBLOCK. errno: %d", errno);
            }

            LOGATCI(LOG_INFO, "atci: new atci-service connection");
            waitForAtcidData();
        } else {
            LOGATCI(LOG_DEBUG, "atci service is exit, waitForAtcidConnect");
            if (s_fdAtci_generic_listen != -1) {
                close(s_fdAtci_generic_listen);
            }
            setMainExit();
        }
    }
}

void processAtcidResponse(char* data, int dataLen){
    int sendLen = 0;

    if (dataLen > 0) {
        sendLen = send(s_fdAtci_generic_command, data, dataLen, 0);
        if (sendLen != dataLen) {
            LOGATCI(LOG_ERR, "Data lost when send command response to atcid socket. errno = %d", errno);
        }
    } else {
        LOGATCI(LOG_ERR, "No data to send");
    }
}

void startAtciServThread(){
    int ret = 0;

    LOGATCI(LOG_DEBUG, "create atci service socket thread");
    ret = pthread_create(&s_tid_atci_service, NULL, waitForAtcidConnect, NULL);
    if (ret != 0) {
        LOGATCI(LOG_ERR, "fail to create atci-usb server socket. errno:%d", errno);
        return;
    }

    LOGATCI(LOG_DEBUG, "create atci service socket thread successfully");
}

int get_control_socket(char* name) {

    struct sockaddr_un server;

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        LOGATCI(LOG_ERR, "Can't open stream socket (%s)", name);
        return 0;
    }

    server.sun_family = AF_UNIX;
    memset(server.sun_path, '\0', sizeof(server.sun_path));
    strncpy(server.sun_path, name, sizeof(server.sun_path) - 1);
    unlink(name);

    if (bind(fd, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
        LOGATCI(LOG_ERR, "bind failed name=[%s] reason=[%s]\n", name, strerror(errno));
        close(fd);
        return 0;
    }
    return fd;
}


/*
Initial the name socket to listen the socket request from atcid process
*/
void initSocket(){
    int ret = 0;

    // Initalize the ATCI socket for native ATCI service
    s_fdAtci_generic_listen = get_control_socket(SOCKET_NAME_ATCI);

    if (s_fdAtci_generic_listen < 0) {
        LOGATCI(LOG_ERR, "Failed to get socket '" SOCKET_NAME_ATCI "' erron:%d", errno);
    } else {
        ret = listen(s_fdAtci_generic_listen, 4);
        if (ret < 0) {
            LOGATCI(LOG_ERR, "Failed to listen on control socket '%d': %s",
                 s_fdAtci_generic_listen, strerror(errno));
        }
    }

}

void sigpipe_handler(int s) {
    UNUSED(s);
    LOGATCI(LOG_DEBUG, "Caught SIGPIPE\n");
}

/*
* Purpose:  The main program loop
* Return:    0
*/
int main() {
    LOGATCI(LOG_DEBUG, "Start to run atci service");
    signal(SIGPIPE, sigpipe_handler);
    initSocket();

    startAtciServThread();

    while (main_exit_signal == 0) {
        sleep(5);
    }
    LOGATCI(LOG_DEBUG, "exit atci service %d", main_exit_signal);
    if (main_exit_signal != 0) {
        exit(-1);
    }
    pthread_join(s_tid_atci_service, NULL);

    return 0;
}
