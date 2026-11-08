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
 * MediaTek Inc. (C) 2020. All rights reserved.
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

enum
{
	ACDK_ENBALE_AUTOSLEEP = 0,
	ACDK_DISABLE_AUTOSLEEP = 1,
	ACDK_AUTOSLEEP_TYPE_ENUM = 2,
};

static char sys_power_state[] = "/sys/power/autosleep";

static int enableAutoSuspend(void) {
	char *buf="mem";
	int fd,size,len;

	if ((fd = open(sys_power_state, O_CLOEXEC | O_RDWR)) < 0) {
		ALOGD("%s(%d):open %s is fail\n", __func__, __LINE__, sys_power_state);
		return -1;
	} else
		ALOGD("%s(%d):open %s is success\n", __func__, __LINE__, sys_power_state);

	len = strlen(buf);
	if ((size = write( fd, buf, len)) < 0){
		ALOGD("%s(%d):write %s is fail\n", __func__, __LINE__, buf);
		return -1;
	} else
		ALOGD("Write:%s OK\n",buf);

	if (close(fd) < 0){
		ALOGD("%s(%d):close %s is fail\n", __func__, __LINE__, sys_power_state);
		return -1;
	} else
		ALOGD("%s(%d):close %s is success\n", __func__, __LINE__, sys_power_state);

	return 0;
}

static int disableAutoSuspend(void) {
	char *buf="off";
	int fd,size,len;

	if ((fd = open(sys_power_state, O_CLOEXEC | O_RDWR)) < 0) {
		ALOGD("%s(%d):open %s is fail\n", __func__, __LINE__, sys_power_state);
		return -1;
	} else
		ALOGD("%s(%d):open %s is success\n", __func__, __LINE__, sys_power_state);

	len = strlen(buf);
	if ((size = write( fd, buf, len)) < 0){
		ALOGD("%s(%d):write %s is fail\n", __func__, __LINE__, buf);
		return -1;
	} else
		ALOGD("Write:%s OK\n",buf);

	if (close(fd) < 0){
		ALOGD("%s(%d):close %s is fail\n", __func__, __LINE__, sys_power_state);
		return -1;
	} else
		ALOGD("%s(%d):close %s is success\n", __func__, __LINE__, sys_power_state);

	return 0;
}

int auto_sleep_cmd_handler(char* cmdline, ATOP_t at_op, char* response)
{
	int ret = 0;
	int s_err = 0;
	int CCT_Operation = -1;

	const char successResponse[] = "\r\n\r\nOK\r\n\r\n";
	const char failureResponse[] = "\r\n\r\nFail\r\n\r\n";

	ALOGD("handle cmdline:%s", cmdline);

	switch (at_op)
	{
		case AT_ACTION_OP:
		case AT_READ_OP:
		case AT_TEST_OP:
		case AT_SET_OP:
			if (at_tok_nextint(&cmdline, &CCT_Operation) == -1) {
				ALOGD("Grab output size failed");
				s_err = snprintf(response, MAX_AT_RESPONSE, "%s", failureResponse);
				if (s_err < 0)
					ALOGE("%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, s_err);
				return -1;
			}
			if (CCT_Operation == ACDK_ENBALE_AUTOSLEEP) {
				ret = enableAutoSuspend();
			} else if (CCT_Operation == ACDK_DISABLE_AUTOSLEEP) {
				ret = disableAutoSuspend();
			}
			if (ret < 0) {
				s_err = snprintf(response, MAX_AT_RESPONSE, "%s", failureResponse);
			} else {
				s_err = snprintf(response, MAX_AT_RESPONSE, "%s", successResponse);
			}

			if (s_err < 0)
				ALOGE("%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, s_err);
		break;
		default:
		break;
	}

	return ret;
}

