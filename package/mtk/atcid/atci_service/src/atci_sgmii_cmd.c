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
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>


#include "atci_sgmii_cmd.h"
#include "atci_service.h"
#include "atci_util.h"


#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
#define MAX_AT_RESPONSE 2048

#define BIT(nr)         (1UL << (nr))


int sgmii_cmd_handler(char* cmdline, ATOP_t at_op, char* response) {
    int fd, i;
	int ret = 0;
#ifdef CONFIG_COLGIN_SUPPORT
    off_t sgmii_2_offset = 0x10000; //offset of SGMII-2
    off_t gen_ctrl_0_addr = 0x100600C4,  //PAT_GEN_CTRL_0 of SGMII-1
          gen_ctrl_3_addr = 0x100600d0;  //PAT_GEN_CTRL_3 of SGMII-1
#else
/*MT6990 sgmii reg	0x11304000 	0x11305000*/
    off_t sgmii_2_offset = 0x1000; //offset of SGMII-2
    off_t gen_ctrl_0_addr = 0x113040C4,  //PAT_GEN_CTRL_0 of SGMII-1
          gen_ctrl_3_addr = 0x113040d0;  //PAT_GEN_CTRL_3 of SGMII-1
#endif

    void *sgmii_map_base = NULL;



    ALOGD("handle cmdline:%s", cmdline);
    ret += snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nFAILED\r\n\r\n");

    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        ALOGD("Error at line %d, file %s (%d) [%s]\n",
                __LINE__, __FILE__, errno, strerror(errno));
        goto FAILED_OPEN;
    }

    system("ifconfig eth1 up");
	usleep(100000);
    system("ifconfig eth0 up");
	usleep(100000);

    for (i = 0; i < 2; i++) {	// Test SGMII-1 and SGMII-2.
        uint32_t *ptr1 = NULL, *ptr2 = NULL;
        uint8_t test = 0;

        /* Map one page */
        sgmii_map_base = mmap(0, 2*MAP_SIZE,
                            PROT_READ | PROT_WRITE, MAP_SHARED,
                            fd, gen_ctrl_0_addr & ~MAP_MASK);
        if(sgmii_map_base == (void *) -1) {
            ALOGD("Error at line %d, file %s (%d) [%s]\n",
                    __LINE__, __FILE__, errno, strerror(errno));
            goto FAILED_MMAP;
        }

        printf("SGMII dump start:\n");
        ptr1 = (uint32_t *)(sgmii_map_base + (gen_ctrl_0_addr & MAP_MASK));
        printf("Value at 0x%lX (%p): 0x%X\n", gen_ctrl_0_addr, ptr1, *ptr1);
        ptr2 = (uint32_t *)(sgmii_map_base + (gen_ctrl_3_addr & MAP_MASK));
        printf("Value at 0x%lX (%p): 0x%X\n", gen_ctrl_3_addr, ptr2, *ptr2);

        *ptr1 |= BIT(0) | BIT(4);  //PRBS_EN and RENEW_MODE.

        printf("After modifing, value at 0x%lX (%p): 0x%X\n", gen_ctrl_0_addr, ptr1, *ptr1);
        usleep(500);      //500 micro-second
        *ptr1 |= BIT(1);  //PRBS_CHECK
        printf("Modify again, value at 0x%lX (%p): 0x%X\n", gen_ctrl_0_addr, ptr1, *ptr1);
        usleep(10000);    //10 ms

        printf("Wait 10ms, value at 0x%lX (%p): 0x%X\n", gen_ctrl_0_addr, ptr1, *ptr1);
        printf("Wait 10ms, value at 0x%lX (%p): 0x%X\n", gen_ctrl_3_addr, ptr2, *ptr2);

        *ptr1 = 0x11;
        usleep(10000);    //10 ms
        printf("After toggle, value at 0x%lX (%p): 0x%X\n", gen_ctrl_0_addr, ptr1, *ptr1);
        printf("After toggle, value at 0x%lX (%p): 0x%X\n", gen_ctrl_3_addr, ptr2, *ptr2);

        *ptr1 = 0x13;
        usleep(10000);    //10 ms
        printf("Toggle back, value at 0x%lX (%p): 0x%X\n", gen_ctrl_0_addr, ptr1, *ptr1);
        printf("Toggle back, value at 0x%lX (%p): 0x%X\n", gen_ctrl_3_addr, ptr2, *ptr2);

        test = *ptr1 & BIT(6)  //BIST_OK
                && *ptr1 & BIT(7)  //BIST_RUN
                && (*ptr1 & BIT(8)) == 0  //BIST_ERR
                && (*ptr2 & 0xffff) == 0;  //BIST_ERR_CNT
        if (!test) {
            ALOGD( "Test %d fail at line %d, file %s (%d) [%s]\n",
                    i, __LINE__, __FILE__, errno, strerror(errno));
            goto FAILED_TEST;
        }

        gen_ctrl_0_addr += sgmii_2_offset;
        gen_ctrl_3_addr += sgmii_2_offset;
    }


    ret += snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nOK\r\n\r\n");

FAILED_TEST:

FAILED_MMAP:
    munmap(sgmii_map_base, MAP_SIZE);

FAILED_OPEN:
    close(fd);
    return ret;
}
