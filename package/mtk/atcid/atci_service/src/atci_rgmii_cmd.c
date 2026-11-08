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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>

#ifndef __GLIBC__
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#endif
#include "atci_rgmii_cmd.h"

#define MAX_AT_RESPONSE	2048

#define SYSFS_ETHER_NODE_2737 "/sys/devices/platform/1101e000.ethernet/stmmac"

#define SYSFS_ETHER_NODE_2735 "/sys/devices/platform/11021000.ethernet/stmmac"

static int stmmac_set()
{
#if defined(TELEMATICS)
	int fd;
	int len;
	char buf[64]={0};
	char mt2737[] = "mt2737";
	char mt2735[] = "mt2735";

	if (strcmp(PLATFORM, mt2737) == 0)
		fd = open(SYSFS_ETHER_NODE_2737, O_WRONLY);
	else if (strcmp(PLATFORM, mt2735) == 0)
		fd = open(SYSFS_ETHER_NODE_2735, O_WRONLY);
	else
		ALOGE("There is no suitable project!\n");

	if (fd < 0)
	{
		ALOGE("Can't access ethernet\n");
		return fd;
	}

	/*set smt result*/
	len = snprintf(buf, sizeof(buf), "smt");
	if (len < 0)
		goto err;

	write(fd, buf, strlen(buf)+1);
	ALOGE("smt\n");
err:
	close(fd);
#endif
	return 0;
}

static int stmmac_get()
{
#if defined(TELEMATICS)
	int fd;
	int len;
	char *buf = NULL;
	char deststr[] = "eth_smt_result:";
	char *s = NULL;
	char mt2737[] = "mt2737";
	char mt2735[] = "mt2735";

	if (strcmp(PLATFORM, mt2737) == 0)
		fd = open(SYSFS_ETHER_NODE_2737, O_RDONLY);
	else if (strcmp(PLATFORM, mt2735) == 0)
		fd = open(SYSFS_ETHER_NODE_2735, O_RDONLY);
	else
		ALOGE("There is no suitable project!\n");

	if (fd < 0)
	{
		ALOGE("Can't access ethernet\n");
		return fd;
	}

	buf = (char *)malloc(7200);
	len = read(fd, buf, 7200);
	buf[7199] = '\0';
	close(fd);

	if(len == -1)
	{
		ALOGE("Can't read ethernet file\n");
		goto FAILED_RESULT;
	} else {
		s = strstr(buf, deststr);

		if(s != NULL) {
			if(0 == strncmp((s+15), "1", 1)) {
				ALOGE("smt success\n");
				free(buf);
				return 0;
			} else {
				ALOGE("smt failed\n");
				goto FAILED_RESULT;
			}
		} else {
			ALOGE("Can't find smt\n");
			goto FAILED_RESULT;
		}
	}

FAILED_RESULT:
	free(buf);
	return -1;
#else
	return 0;
#endif
}

int rgmii_cmd_handler(char* cmdline, ATOP_t at_op, char* response)
{
	int val = -1;
	int ret;

	if (at_op == AT_ACTION_OP) {
		stmmac_set();

		val = stmmac_get();
		if (val == 0) {
			ret = snprintf(response, MAX_AT_RESPONSE,"\r\nOK\r\n\r\n");
			if (ret < 0)
				goto err;
		} else {
			ret = snprintf(response, MAX_AT_RESPONSE,"\r\nERROR\r\n\r\n");
			if (ret < 0)
				goto err;
		}
	}

err:
	return val;
}
