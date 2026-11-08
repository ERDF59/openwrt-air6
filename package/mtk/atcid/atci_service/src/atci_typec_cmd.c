/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifdef TELEMATICS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "atci_typec_cmd.h"
#include "atci_service.h"
#include "atci_util.h"
#include "atci_generic_cmd_table.h"
#define TAG                 "[USB TYPEC] "
#define CURRENT_SPEED_PATH "/sys/devices/platform/11201000.usb/udc/11201000.usb/current_speed"
enum {
    ITEM_FAIL,
    ITEM_PASS,
};

static int check_current_speed()
{
    FILE *fp;
    char buf[20];
    int ret = 0;
    memset(buf, 0x00, sizeof(buf));
    fp = fopen(CURRENT_SPEED_PATH, "r");
    if (fp != NULL) {
	if (fgets(buf, 20, fp) != NULL)	{
	    ALOGD(TAG "CURRETN_SPEED = %s\n", buf);
	    if (strstr(buf, "super-speed") != NULL)
		ret = 1;
	}
    }
    return ret;
}

int usb_typec_cmd_handler(char* cmdline, ATOP_t at_op, char* response)
{
    int chosen;
    int n;
    ALOGD("--- USB MicroB SMT ---\n");
    ALOGD("handle cmdline:%s", cmdline);
    if (check_current_speed())
	chosen = ITEM_PASS;
    else
	chosen = ITEM_FAIL;
    if (chosen == ITEM_PASS) {
	n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nOK\r\n\r\n");
	if (n < 0) {
	    pr_err("%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
	}
    } else {
	n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nFAILED\r\n\r\n");
	if (n < 0) {
	    pr_err("%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
	}
    }
    return 0;
}
#else
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/ioctl.h>
#include <limits.h>

#include <stdbool.h>

#include "atci_typec_cmd.h"
#include "atci_service.h"
#include "atci_util.h"
#include "atci_generic_cmd_table.h"

#define TAG                 "[USB TYPEC] "
#define TYPEC_LOOPBACK_PATH "/proc/mtk_usb/usb-phy0/u3_phy/loopback_test"
#define TYPEC_SWITCH_PATH "/sys/devices/platform/typec_mux_switch/sw"
#define RETRY_COUNT 20

enum {
	ITEM_FAIL,
	ITEM_PASS,
};

int g_result_usb_typec = ITEM_FAIL;

static int check_sel_path(void)
{
	int fd_mux = -1;

	fd_mux = open(TYPEC_SWITCH_PATH, O_WRONLY, 0);
	if (fd_mux < 0) {
		ALOGD(TAG "Can't open %s\n", TYPEC_SWITCH_PATH);
		return 0;
	}
	close(fd_mux);
	return 1;
}

static bool usb_u3_loopback(void)
{
    int fd = -1;
    int fd_mux = -1;

    char ret = '9';
    int side_1 = 0;
    int side_2 = 0;

    fd_mux = open(TYPEC_SWITCH_PATH , O_WRONLY, 0);
    if (fd_mux < 0) {
	ALOGD(TAG "Can't open %s\n", TYPEC_SWITCH_PATH );
	goto end;
    }

    /* Side 1 */
    write(fd_mux, "1", 1);

    usleep(200 * 1000);

    fd = open(TYPEC_LOOPBACK_PATH , O_RDONLY, 0);
    if (fd < 0) {
	ALOGD(TAG "Can't open %s\n", TYPEC_LOOPBACK_PATH );
    } else {
	if (read(fd, &ret, sizeof(ret)) <= 0) {
	    ALOGD(TAG "Read fail (ret1)\n");
	}
	close(fd);
    }

    /*success return char '1', fail return char '0'.*/
    if (ret == '1')
	side_1 = 1;
    else
	side_1 = 0;


    /* Side 2 */
    write(fd_mux, "2", 1);

    usleep(200 * 1000);

    fd = open(TYPEC_LOOPBACK_PATH , O_RDONLY, 0);
    if (fd < 0) {
	ALOGD(TAG "Can't open %s\n", TYPEC_LOOPBACK_PATH );
    } else {
	if (read(fd, &ret, sizeof(ret)) <= 0){
	    ALOGD(TAG "Read fail (ret2)\n");
	}
	close(fd);
    }

    if (ret == '1')
	side_2 = 1;
    else
	side_2 = 0;

    ALOGD(TAG "side_1=%d, side_2=%d\n", side_1, side_2);
end:
    if (fd_mux >= 0)
	close(fd_mux);

    return side_1 && side_2;
}

int usb_typec_cmd_handler(char* cmdline, ATOP_t at_op, char* response)
{
    int chosen;
    int n, retry;

    chosen = ITEM_FAIL;
    retry = 0;
    ALOGD("handle cmdline:%s", cmdline);
    n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nFAILED\r\n\r\n");
    if (n < 0) {
	pr_err("%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
    }

    do {
	ALOGD(TAG "retry_cnt=%d\n", retry);
	if (check_sel_path()) {
	    if(usb_u3_loopback()) {
		chosen = ITEM_PASS;
		break;
	    } else
		retry++;
	} else
	    break;
    } while (retry < RETRY_COUNT);

    ALOGD(TAG "g_result_usb_typec=%x\n", g_result_usb_typec);

    if (chosen == ITEM_PASS) {
	n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nOK\r\n\r\n");
	if (n < 0) {
	    pr_err("%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
	}
    } else {
	n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nFAILED\r\n\r\n");
	if (n < 0) {
	    pr_err("%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
	}
    }

    return 0;
}
#endif
