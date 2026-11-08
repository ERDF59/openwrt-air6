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

#include <linux/reboot.h>
#include <stdint.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "atci_system_cmd.h"
#include "atci_service.h"
#include "atci_util.h"
#include "atci_generic_cmd_table.h"

#include <sys/reboot.h>
#include <sys/syscall.h>

#ifndef bool
#define bool unsigned char
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define FBOOT_FILE "/data/system/called_pre_boots.dat"
#define RESTART_PASS "RESTARTING SYSTEM"
#define RESTART_FAIL  "ERROR"
#define REBSTART_REASON "AT%RESTART"

#define RESTART_WAITING_TIME 5 //5 sec

int system_check_fboot() {
    int fd, ret = 0;
    fd = open(FBOOT_FILE, O_RDONLY);
    if (fd < 0) {
        // we've NOT fbooted
        ret = 0;
    } else {
        // we've fbooted
        ret = 1;
        close(fd);
    }
    ALOGD("check fboot :%d", ret);
    return ret;
}

static void* restart_routine(void *arg) {
    UNUSED(arg);
    ALOGD("Ready to restarting system after %d seconds!", RESTART_WAITING_TIME);
    usleep(RESTART_WAITING_TIME*1000*1000);
    sync();
    ALOGD("byebye");

    syscall(SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART, NULL);

    return 0;
}

int invoke_restart(){
    int ret = 0;
    pthread_attr_t attr;
    pthread_t restart_thread;

    pthread_attr_init(&attr);

    ret = pthread_create(&restart_thread, &attr, restart_routine, NULL);
    if (ret != 0) {
        ALOGE("fail to create thread for restarting");
    }

    return ret;
}

int system_fboot_handler(char* cmdline, ATOP_t at_op, char* response){
    int retval = 0;

    ALOGD("[%s]handle cmdline:%s", __FUNCTION__, cmdline);

    switch(at_op){
        case AT_ACTION_OP:
        case AT_READ_OP:
        case AT_TEST_OP:
        case AT_SET_OP:
            retval = system_check_fboot();
            snprintf(response, MAX_AT_RESPONSE, "\r\n%d\r\nOK\r\n\r\n", retval);
            break;
        default:
            ALOGD("unknown at_op :%d", at_op);
            break;
    }

    return retval;
}

int system_poweroff_handler(char* cmdline, ATOP_t at_op, char* response){
    int retval = 0, err = 0;
    int delay = 0, force_mode = 0, non_sync_mode = 0;
    int i = 0;
    int s_err;
    char cmd[50];

    ALOGD("[%s]handle cmdline:%s", __FUNCTION__, cmdline);

    if (cmdline != NULL) {
        if (strlen(cmdline) == 1) {
            force_mode = cmdline[0] == 'f' ? 0 : 1;
            non_sync_mode = cmdline[0] == 'n' ? 0 : 1;
        }
        if (force_mode) {
            system("/sbin/poweroff -f");
            return AT_OK;
        }
        if (non_sync_mode) {
            system("/sbin/poweroff -n");
            return AT_OK;
        }
        for (i = 0; i < strlen(cmdline); i++) {
            if ((cmdline[0] < '0') || (cmdline[0] > '9')) {
                err = 1;
                break;
            }
        }
        if (!err) {
            delay = atoi(cmdline);
            if ((delay < 0) || (delay > 65535))
                err = 1;
        }

    }

    switch (at_op) {
        case AT_TEST_OP:
            s_err = snprintf(response, MAX_AT_RESPONSE,
                    "Usage: poweroff [DELAY|n|f]\n\r"
                    "  DELAY: 0-65535, delay interval\n\r"
                    "  n, Do not sync\n\r"
                    "  f, Force (don't go through init)\n\r");
	    if (s_err < 0){
	        ALOGD("AT_TEST_OP sprintf error\n");
		return AT_ERROR;
	    }
            return AT_OK;
            break;
        case AT_ACTION_OP:
            system("/sbin/poweroff");
            break;
        case AT_SET_OP:
            if (err) {
                s_err = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\npoweroff failed\r\n\r\n");
		if (s_err < 0) {
		    ALOGD("AT_SET_OP sprintf error\n");
		}
                return AT_ERROR;
            }
            s_err = snprintf(cmd, sizeof(cmd), "/sbin/poweroff -d %d", delay);
	    if (s_err < 0) {
                ALOGD("POWEROFF DELAY sprintf error\n");
		retval = AT_ERROR;
	    } else {
                system(cmd);
	    }
            s_err = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\npoweroff successed\r\n\r\n");
	    if (s_err < 0) {
	        ALOGD("POWEROFF SUCCESS sprintf error\n");
		retval = AT_ERROR;
	    }
            break;
        default:
            ALOGD("unknown at_op :%d", at_op);
            retval = AT_ERROR;
            break;
    }

    return retval;
}

int system_restart_handler(char* cmdline, ATOP_t at_op, char* response){
    int retval = 0, err = 0;

    ALOGD("[%s]handle cmdline:%s", __FUNCTION__, cmdline);

    if (cmdline!=NULL)
        err = cmdline[0] == '\0' ? 0 : 1;

    switch (at_op) {
        case AT_ACTION_OP:
        case AT_READ_OP:
        case AT_TEST_OP:
        case AT_SET_OP:
            if (at_op == AT_SET_OP && err) //has character next to =
                snprintf(response, MAX_AT_RESPONSE, "\r\n\r\n%s\r\n\r\n", RESTART_FAIL);
            else {
                invoke_restart();
                snprintf(response, MAX_AT_RESPONSE, "\r\n\r\n%s\r\n\r\n", RESTART_PASS);
            }
            break;
        default:
            ALOGD("unknown at_op :%d", at_op);
            break;
    }

    return retval;
}

int system_safe_restart_handler(char* cmdline, ATOP_t at_op, char* response) {
    UNUSED(at_op);
    UNUSED(response);
    ALOGD("[%s] %s", __FUNCTION__, cmdline);
    system("svc power reboot");
    return 0;
}
