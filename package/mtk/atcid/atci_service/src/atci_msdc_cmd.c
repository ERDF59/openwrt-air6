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

#include "atci_msdc_cmd.h"
#include "atci_service.h"
#include "atci_util.h"
#include "atci_generic_cmd_table.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>

#define  MMC_DIR                "/mnt/sdcard"
#define  MMC_OPR_FAIL          -1
#define  MMC_OPR_SUCCESS        0
#define  MMC_DEV                 "/dev/block/mmcblk1"

/******************************************************************************
  * Structure
  *****************************************************************************/

int bootdev_test_result = 0;


struct bootdev {
    int           id;
    char          sys_path[512];
    char          info[1024];
    unsigned int     csd[4];
    unsigned char ext_csd[512];
    unsigned int  capacity;
    unsigned int  boot_size;
    bool          avail;
    struct itemview   *iv;
};



#define BOOTDEV_BLKDEV_SIZE_PATH_UFS   "/sys/class/block/sdc/size"
#define BOOTDEV_BLKDEV_SIZE_PATH_EMMC  "/sys/class/block/mmcblk0/size"
#define UISTR_INFO_BOOTDEV_UFS         "UFS"
#define UISTR_INFO_BOOTDEV_EMMC       "eMMC"
#define TAG                            "[STORAGE] "
#define ITEM_BOOTDEV                   ITEM_STORAGE

#define uistr_info_emmc_sd_avail    "Card Avail "
#define uistr_info_emmc_sd_yes		"Yes"
#define uistr_info_emmc_sd_no		"No"
#define uistr_info_emmc_sd_total_size	"Total Size "
#define MAX_LINE_SIZE			40



/* for boot type usage */
#define BOOTDEV_NAND            (0)
#define BOOTDEV_SDMMC           (1)
#define BOOTDEV_UFS             (2)


/* should be moved to customized part */
#define MAX_NUM_SDCARDS  (3)
#define MIN_SDCARD_IDX   (0)
#define MAX_SDCARD_IDX   (MAX_NUM_SDCARDS + MIN_SDCARD_IDX - 1)



static int get_boot_type(void) {
 /*
    int fd;
    size_t s;
    char boot_type[4] = {'0'};

    fd = open("/sys/class/BOOT/BOOT/boot/boot_type", O_RDONLY);
    if (fd < 0) {
        ALOGD("fail to open: %s\n", "/sys/class/BOOT/BOOT/boot/boot_type");
        return -1;
    }

    s = read(fd, (void *)&boot_type, sizeof(boot_type) - 1);
    close(fd);

    if (s <= 0) {
        ALOGD("could not read boot type sys file\n");
        return -1;
    }

    boot_type[s] = '\0';

    return atoi((char *)&boot_type);
*/
/*bypass get boot type because there is no node in openwrt*/

  return 1;
}


static bool bootdev_avail(struct bootdev *dev)
{
    char name[20];
    char *ptr;
    DIR *dp;
    struct dirent *dirp;
    int mt_boot_type = get_boot_type();
    int ret = 0;

    ALOGD("jyj debug the boot_type = %d !\n", mt_boot_type);
    if (mt_boot_type == BOOTDEV_UFS) {
        ret = snprintf(name, sizeof(name), "host%d", dev->id);
        if(ret < 0) {
            ALOGD("jyj debug UFS set name(host%d) fail\n", dev->id);
            goto error;
        }
        ptr  = &(dev->sys_path[0]);
        ptr += snprintf(ptr, sizeof(dev->sys_path), "/sys/class/scsi_host");
    } else {
        if (dev->id < MIN_SDCARD_IDX || dev->id > MAX_SDCARD_IDX)
            return false;

        ret = snprintf(name, sizeof(name), "mmc%d", dev->id - MIN_SDCARD_IDX);
        if(ret < 0) {
            ALOGD("jyj debug MMC set name(mmc%d) fail\n", dev->id - MIN_SDCARD_IDX);
            goto error;
        }
        ptr  = &(dev->sys_path[0]);
        ptr += snprintf(ptr, sizeof(dev->sys_path), "/sys/class/mmc_host/%s", name);
    }

    ALOGD("target path: %s\n", dev->sys_path);
    ALOGD("target name: %s\n", name);

    if (NULL == (dp = opendir(dev->sys_path))) {
        ALOGD("opendir %s fail!\n", dev->sys_path);
        goto error;
    }

    while (NULL != (dirp = readdir(dp))) {
        ALOGD("file name: %s\n", dirp->d_name);
        if (strstr(dirp->d_name, name)) {
            ptr += snprintf(ptr, sizeof(dirp->d_name) + 1, "/%s", dirp->d_name);
            break;
        }
    }

    closedir(dp);

    if (!dirp) {
        ALOGD("NULL dirp!\n");
        goto error;
    }

    return true;

 error:
    return false;
}



static void bootdev_update_info(struct bootdev *dev, char *info,float *ret)
{
    char *ptr;
    int inode = 0;
    unsigned long sector_cnt = 0;
    bootdev_test_result = 0;  /* 0: test no pass, 1: test pass */
    char buf[1024];
    ssize_t byte_read;
    int mt_boot_type = get_boot_type();

    ALOGD("jyj debug start the %s !\n", __FUNCTION__);

    dev->avail = bootdev_avail(dev);

    ALOGD("avail: %d,mt_boot_type = %d\n", dev->avail,mt_boot_type);

    if (mt_boot_type == BOOTDEV_UFS) {
        inode = open(BOOTDEV_BLKDEV_SIZE_PATH_UFS, O_RDONLY);

        if (inode < 0) {
            ALOGD("open %s fail!\n", BOOTDEV_BLKDEV_SIZE_PATH_UFS);
            return;
        }
    } else {
        /* Use eMMC for default boot type */
        inode = open(BOOTDEV_BLKDEV_SIZE_PATH_EMMC, O_RDONLY);
        ALOGD("jyj debug open eMMC suc inode = %d\n", inode);
        if (inode < 0) {
            ALOGD("open %s fail!\n", BOOTDEV_BLKDEV_SIZE_PATH_EMMC);
            return;
        }
    }

    memset(buf, 0, sizeof(buf));
    byte_read = read(inode, buf, sizeof(buf) - 1);

    ALOGD("byte_read: %d\n", (int)byte_read);

    sector_cnt = atol(buf);

    ALOGD("sector_cnt: %llu\n", sector_cnt);

    dev->capacity = (float)(sector_cnt * 512 / 1024);

    ALOGD("capacity: %d MB\n", dev->capacity / 1024);

    close(inode);

    ptr  = info;
    if (mt_boot_type == BOOTDEV_UFS)
        ptr += snprintf(ptr, MAX_LINE_SIZE, "%s\n", UISTR_INFO_BOOTDEV_UFS);
    else
        ptr += snprintf(ptr, MAX_LINE_SIZE, "%s\n", UISTR_INFO_BOOTDEV_EMMC);

    ALOGD("jyj debug close eMMCinode suc inode = %d\n", inode);


    ptr += snprintf(ptr, MAX_LINE_SIZE, "%s: %s\n", uistr_info_emmc_sd_avail,
        dev->avail ? uistr_info_emmc_sd_yes : uistr_info_emmc_sd_no);
    ptr += snprintf(ptr, MAX_LINE_SIZE, "%s: %.2f GB\n", uistr_info_emmc_sd_total_size,
        (float)(dev->capacity) / (1024 * 1024));

    /*
     * UFS and emmc share the same below structure: ftm_ata_emmc.
     *
     * Notice: Any change here must sync with SP ATA Tool's structure definition
     *         in PC side.
     */
    ALOGD("jyj debug function:%s succ dev->cap = %f\n",__FUNCTION__, dev->capacity);
    *ret = (float)(dev->capacity)/(1024 * 1024);

    bootdev_test_result = 1;  /* all things done, mark successful */

    return;
}

int msdc_cmd_handler(char* cmdline, ATOP_t at_op, char* response)
{
    float ret = 0;
    struct bootdev *dev = NULL;
    int pr_ret = 0;

    dev = (struct bootdev*)malloc(sizeof(struct bootdev));

    if(!dev){
        ALOGD("jyj debug fail to malloc memery for eMMC!\n");
        pr_ret = snprintf(response, MAX_AT_RESPONSE, "ERROR!!!\r\n");
        if (pr_ret < 0) {
            ALOGD("jyj debug fail to print dev malloc error message\n");
        }
        return 0;
    }

    dev->id = 0;
    dev->avail = false;
    ALOGD("eMMC cmdline=%s, at_op=%d, \n", cmdline, at_op);

    switch(at_op){
        case AT_ACTION_OP:
        case AT_READ_OP:
        case AT_TEST_OP:
        ALOGD("jyj debug start to run bootdev update info!\n");
        bootdev_update_info(dev, dev->info,&ret);
        if (ret == 0){
            ALOGD("jyj debug fail!!, \n");
            pr_ret = snprintf(response, MAX_AT_RESPONSE, "ERROR!!!\r\n");
            if (pr_ret < 0) {
                ALOGD("jyj debug fail to print update info error message\n");
            }
        }else{
            ALOGD("jyj debug eMMC success!!cap = %d \n",ret);
            pr_ret = snprintf(response, MAX_AT_RESPONSE, "eMMC TEST OK!, cap = %f\r\n", ret);
            if (pr_ret < 0) {
                ALOGD("jyj debug fail to print update info success message\n");
            }
        }
        break;

        case AT_SET_OP:
            break;

        default:
        break;
    }

    free(dev);

    return 0;
}


