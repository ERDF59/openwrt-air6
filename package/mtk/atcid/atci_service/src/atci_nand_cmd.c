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

#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//#include "atci_battery_cmd.h"
#include "atci_nand_cmd.h"
#include "atci_service.h"
#include "atci_util.h"
#include "atci_generic_cmd_table.h"

//4 types of operation codes: ACTION / TEST(=?)

int nand_cmd_handler(char* cmdline, ATOP_t at_op, char* response) {
    int fd, ret;
    const char *nand_type_path = "/sys/class/mtd/mtd0/type";
    char buf[5];

    ALOGD("[%s]handle cmdline:%s", __FUNCTION__, cmdline);

    switch(at_op) {
        case AT_ACTION_OP:
            fd = open(nand_type_path, O_RDONLY);
            if(fd < 0) {
                /* mtd 0 not detected */
                ret = snprintf(response, MAX_AT_RESPONSE, "\r\nERROR\r\n");
                if (ret < 0)
                    ALOGD("%s: snprintf fail,at_op %d, ret %d\n", __FUNCTION__, at_op, ret);
                close(fd);
                return 0;
            }
            read(fd, buf, 4);
            if(!strcmp(buf, "nand"))
                ret = snprintf(response, MAX_AT_RESPONSE, "\r\nNAND OK\r\n");
            else
                ret = snprintf(response, MAX_AT_RESPONSE, "\r\nERROR\r\n");
            if (ret < 0)
                ALOGD("%s: snprintf fail,at_op %d, ret %d\n", __FUNCTION__, at_op, ret);
            close(fd);
            break;
        default:
            ALOGD("Unknown at_op :%d", at_op);
            break;
    }

    return 0;
}
