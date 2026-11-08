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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "at_tok.h"
#include "atci_cct_cmd.h"
#include "atci_util.h"
#include "atci_pcie_cmd.h"

#define PCIE_SMT_DEV_NAME	"/dev/pcie_smt"
#define IOCTL_DEV_IOCTLID	'P'
#define PCIE_SMT_TEST_SLOT	_IOW(IOCTL_DEV_IOCTLID, 0, int)

static int mtk_pcie_loopback_test(int slot)
{
	int fd;
	int ret = -1;

	fd = open(PCIE_SMT_DEV_NAME, O_RDWR);
	if (fd < 0) {
		printf("pcie[%d]: dev open failed\n", slot);
		return fd;
	}

	ret = ioctl(fd, PCIE_SMT_TEST_SLOT, slot);
	if (ret < 0)
		printf("pcie: slot [%d] test failed\n", slot);

	close(fd);

	return ret;
}

#define PCIE_MODE_EP	0
#define PCIE_MODE_RC	1

int pcie_cmd_handler(char* cmdline, ATOP_t at_op, char* response)
{
	int pcie_ops = -1;
	int ret = 0;

	switch(at_op) {
        case AT_SET_OP:
            if (at_tok_nextint(&cmdline, &pcie_ops) == -1) {
		    printf("failed to get mode\r\n");
		    goto err;
	    }

	    if (pcie_ops != PCIE_MODE_RC) {
		    printf("unknow PCIE cmd: %s\r\n", cmdline);
		    goto err;
	    }

	    /* get pcie slot */
	    if (at_tok_nextint(&cmdline, &pcie_ops) == -1) {
		    printf("failed to get PCIe slot\r\n");
		    goto err;
	    }

	    ret = mtk_pcie_loopback_test(pcie_ops);
	    if (ret) {
		    printf("PCIe loopback test failed\r\n");
		    goto err;
	    } else {
		    ret = snprintf(response, sizeof(*response), "OK\r\n");
		    if(ret < 0) {
			printf("sprintf fail\r\n");
			goto err;
		    } else {
			goto out;
		    }
	    }
	    break;
	default:
	    printf("pcie_op(failed) at_op(%d) is not supported\r\n", at_op);
	}

err:
	ret = snprintf(response, sizeof(*response), "ERROR\r\n");
	if(ret < 0)
	    printf("sprintf fail\r\n");
out:
	printf("%s: response = %s\r\n", __func__, response);
	return 0;
}
