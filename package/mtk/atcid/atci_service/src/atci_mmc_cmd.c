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

#include "atci_mmc_cmd.h"
#include "atci_service.h"
#include "atci_util.h"
#include "atci_generic_cmd_table.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/mount.h>   /* for BLKGETSIZE */


#define  MMC_DIR                "/mnt/sdcard"
#define  MMC_OPR_FAIL          -1
#define  MMC_OPR_SUCCESS        0
#define  MMC_DEV                 "/dev/block/mmcblk1"

#define TAG                 "[MCARD] "



/* for boot type usage */
#define BOOTDEV_NAND            (0)
#define BOOTDEV_SDMMC           (1)
#define BOOTDEV_UFS             (2)

/* should be moved to customized part */
#define MAX_NUM_SDCARDS     (3)
#define MIN_SDCARD_IDX      (0)
#define MAX_SDCARD_IDX      (MAX_NUM_SDCARDS + MIN_SDCARD_IDX - 1)

#define uistr_info_sd			"SD Card"
#define uistr_info_emmc_sd_avail		"Card Avail "
#define uistr_info_emmc_sd_yes		"Yes"
#define uistr_info_emmc_sd_no		"No"
#define uistr_info_emmc_sd_total_size	"Total Size "
#define MAX_LINE_SIZE			40



struct mcard {
    int          id;
    char         dev_path[512];
    char         sys_path[512];
    char         info[1024];
    char        *mntpnt;
    bool         mounted;
    bool         avail;
    int          blocknum;
    unsigned int checksum;
    const char  *format_stat;
};




struct mcard_array{
    struct mcard* mcard[1];
    char info[2048];
    char *ptr_step;
    int mcard_no;
    bool isFormatting;
    bool exit_thd;
    struct itemview *iv;
};


static bool mcard_avail(struct mcard *mc)
{
    char name[20] = {0};
    char *ptr;
    DIR *dp;
    struct dirent *dirp;
    int ret = 0;

    if (mc->id < MIN_SDCARD_IDX || mc->id > MAX_SDCARD_IDX)
        return false;
    ALOGD("jyj debug Start mcard_availd func! \n");

    ret = snprintf(name, sizeof(name), "mmc%d", mc->id - MIN_SDCARD_IDX);
    if (ret < 0) {
        ALOGD("jyj debug set name(mmc%d) fail", mc->id - MIN_SDCARD_IDX);
    }

    ptr  = &mc->sys_path[0];
    ptr += snprintf(ptr, sizeof(mc->sys_path), "/sys/class/mmc_host/%s", name);

    if (NULL == (dp = opendir(mc->sys_path)))
        goto error;

    while (NULL != (dirp = readdir(dp))) {
        if (strstr(dirp->d_name, name)) {
            ptr += snprintf(ptr, sizeof(dirp->d_name) + 1, "/%s", dirp->d_name);
            break;
        }
    }

    ALOGD("jyj debug readdir suc \n");
    closedir(dp);

    if (!dirp)
        goto error;

    return true;

error:
    return false;
}


static int get_boot_type(void) {
    int fd;
    ssize_t s;
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
}

char buf[512];
static void mcard_update_info(struct mcard *mc, char *info, unsigned int *ret_cap)
{
    char *ptr;
    int rc, fd;
    unsigned int nr_sec = 0;
    unsigned int sd_total_size = 0;
//    int mt_boot_type = get_boot_type();
/*bypass bootdev for sdcard*/

//#if defined(_DATACARD_) || defined(TELEMATICS)
    int mt_boot_type = BOOTDEV_NAND;
//#else
//    int mt_boot_type = BOOTDEV_SDMMC;
//#endif
    ALOGD("jyj debug Start mcard update info! \n");

    mc->avail = mcard_avail(mc);
    mc->avail = 1;

    ALOGD("jyj debug Start mt_boot_type info!= %d \n",mt_boot_type);

    if(mt_boot_type == BOOTDEV_SDMMC) {
        if ((fd = open("/dev/mmcblk1", O_RDONLY, 0644)) < 0){
            mc->avail = 0;
            ALOGD("jyj debug open fail & fd = %d errno:%d\n",fd, errno);
        }
    } else {
        if ((fd = open("/dev/mmcblk0", O_RDONLY, 0644)) < 0) {
            mc->avail = 0;
            ALOGD("jyj debug open fail & fd = %d errno:%d\n",fd, errno);
        }
    }

    ALOGD("jyj debug mc->avail != %d \n",mc->avail);

    if(mc->avail != 0) {
        rc = read(fd, buf, 512);
        ALOGD("jyj debug RC = %d,fd = %d,mc->avail= %d! \n",rc,fd,mc->avail);
        if (rc <= 0) {
            mc->avail = 0;
            goto out;
        }

        ALOGD("jyj debug RC info!= %d ,fd = %d\n",rc,fd);

        if ((ioctl(fd, BLKGETSIZE, &nr_sec)) == -1) {
            ALOGD("BLKGETSIZE fail \n");
        } else {
            sd_total_size = (nr_sec /2048);
        }
        ALOGD("jyj debug at return time sd_total_size info!= %d *ret_cap=%d\n",sd_total_size,*ret_cap);
    }

out:
    /* prepare info */
    ptr  = info;

    ptr += snprintf(ptr, MAX_LINE_SIZE, "%s: \n", uistr_info_sd);

    if (mc->avail)
        ptr += snprintf(ptr, MAX_LINE_SIZE, "%s: %s\n", uistr_info_emmc_sd_avail, uistr_info_emmc_sd_yes);
    else
        ptr += snprintf(ptr, MAX_LINE_SIZE, "%s: %s\n", uistr_info_emmc_sd_avail, uistr_info_emmc_sd_no);

    ptr += snprintf(ptr, MAX_LINE_SIZE, "%s: %d MB\n", uistr_info_emmc_sd_total_size, sd_total_size);

    ALOGD("jyj debug function:%s return suc, total_size= %d *ret_cap=%d\n",__FUNCTION__,sd_total_size,*ret_cap);
    *ret_cap=sd_total_size;

    if (fd >=0)
        close(fd);

    return;
}



int mmc_cmd_handler(char* cmdline, ATOP_t at_op, char* response)
{
    int ret = 0;
    unsigned int ret_csq = 0;
    struct mcard *mc = NULL;
    struct mcard_array *ma = NULL;
    ALOGD("%s\n", __FUNCTION__);
    ALOGD("jyj debug next to run!!\n");

    mc = (struct mcard*)malloc(sizeof(struct mcard));

    if(!mc){
        ALOGD("jyj debug fail to mc malloc memery\n");
        ret = snprintf(response, MAX_AT_RESPONSE, "ERROR!!!\r\n");
        if(ret < 0)
            ALOGD("jyj debug mc snprintf fail\n");
        return 0;
    }
    ma = (struct mcard_array*)malloc(sizeof(struct mcard_array));
    if(!ma){
        ALOGD("jyj debug fail to ma malloc memery\n");
        ret = snprintf(response, MAX_AT_RESPONSE, "ERROR!!!\r\n");
        if(ret < 0)
            ALOGD("jyj debug ma snprintf fail\n");
        free(mc);
        return 0;
    }
    memset(ma, 0, sizeof(struct mcard_array));
    mc->id = 0;
    ALOGD("mc->id=%d,mc=%x,ma= %x \n", mc->id,mc,ma);

    mc->mounted  = false;
    mc->avail    = false;
    mc->mntpnt   = NULL;
    mc->checksum = -1;
    mc->blocknum = -1;
    mc->format_stat = "Default";
    ma->mcard[0] = mc;
    ma->mcard_no += 1;

    ALOGD("MMCcard cmdline=%s, at_op=%d, \n", cmdline, at_op);

    switch(at_op){
        case AT_ACTION_OP:
        case AT_READ_OP:
        case AT_TEST_OP:
        mcard_update_info(ma->mcard[0],ma->info,&ret_csq);

        ALOGD("jyj debug now the ret_csq=%d, \n", ret_csq);

        if (ret_csq == 0){
            ALOGD("jyj debug fail!!, \n");
            ret = snprintf(response, MAX_AT_RESPONSE, "ERROR!!!\r\n");
            if(ret < 0)
                ALOGD("jyj debug print update info error message fail\n");
        }
        else{
            ALOGD("jyj debug success!!cap = %d \n",ret_csq);
            ret = snprintf(response, MAX_AT_RESPONSE, "SDcard TEST OK!, cap = %d\r\n", ret_csq);
            if(ret < 0)
                ALOGD("jyj debug print update info ok\n");
        }
        break;

        case AT_SET_OP:
            break;

        default:
            break;
    }
    free(ma);
    free(mc);/*free memory*/

    return 0;
}



