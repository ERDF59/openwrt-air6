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

#include "atci_lcd_test_cmd.h"
#include "atci_service.h"
#include "at_tok.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <syslog.h>
#include "atci_generic_cmd_table.h"

const char *LCD_DEV_PATH = "/sys/class/leds";
const char *LCD_FILE_NAME = "lcd-backlight";
const char *LCD_FILE_BRIGHTNESS = "brightness";

#define MAX_FILE_LENGTH 512
#define MAX_CMD_LENGTH 64

#define LCD_BRIGHTNESS_ON 255
#define LCD_BRIGHTNESS_OFF 0

void update_lcd(unsigned int loop_times, unsigned int delay_ms);

const int LCD_BRIGHTNESS_LEVELS[] = {
    0, 32, 64, 96, 128, 160, 192, 224, 255
};
#define LCD_LEVEL_SIZE (sizeof(LCD_BRIGHTNESS_LEVELS) / sizeof(const int))

int lcd_test_cmd_handler(__attribute__((unused)) char *cmdline, ATOP_t at_op, char *response) {
    int loop_times = 0;
    int delay_ms = 0;
    int n;

    syslog(LOG_INFO, "Enter %s()", __func__);

    switch(at_op) {
        // AT%LCDTEST=<loop_times>,<delay_ms>
        case AT_SET_OP:
            if (at_tok_nextint(&cmdline, &loop_times) == -1) {
		n = snprintf(response, MAX_AT_RESPONSE,
			"\r\n\r\nLCDTEST SET_OP FAILED(loop_times)\r\n\r\n");
		if (n < 0) {
		    syslog(LOG_ERR,
			    "%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
		}
                break;
            }
            if (at_tok_nextint(&cmdline, &delay_ms) == -1) {
		n = snprintf(response, MAX_AT_RESPONSE,
			"\r\n\r\nLCDTEST SET_OP FAILED(delay_ms)\r\n\r\n");
		if (n < 0) {
		    syslog(LOG_ERR,
			    "%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
		}
                break;
            }
            update_lcd(loop_times, delay_ms);
	    n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nLCDTEST ON\r\n\r\n");
	    if (n < 0) {
		syslog(LOG_ERR,
			"%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
	    }
            break;
        case AT_ACTION_OP:
	    n = snprintf(response, MAX_AT_RESPONSE,
		    "\r\n\r\nLCDTEST ACTION OP NOT SUPPORT\r\n\r\n");
	    if (n < 0) {
		syslog(LOG_ERR,
			"%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
	    }
            break;
        case AT_READ_OP:
            n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nLCDTEST READ OP NOT SUPPORT\r\n\r\n");
	    if (n < 0) {
		syslog(LOG_ERR,
			"%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
	    }
            break;
        case AT_TEST_OP:
            n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nLCDTEST TEST OP NOT SUPPORT\r\n\r\n");
	    if (n < 0) {
		syslog(LOG_ERR,
			"%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
	    }
            break;
        default:
            break;
    }

    return 0;
}

void update_lcd(unsigned int loop_times, unsigned int delay_ms) {
    char file[MAX_FILE_LENGTH] = {0};
    char cmd[MAX_CMD_LENGTH] = {0};

    unsigned int i = 0, j = 0;
    int fd;
    int written = 0;
    int n;

    syslog(LOG_INFO, "Enter update_lcd(%d, %d)", loop_times, delay_ms);

    n = snprintf(file, MAX_FILE_LENGTH, "%s/%s/%s",
	LCD_DEV_PATH, LCD_FILE_NAME, LCD_FILE_BRIGHTNESS);
    if (n < 0) {
        syslog(LOG_ERR, "%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
        goto ERROR;
    }
    fd = open(file, O_RDWR);
    if (fd < 0) {
        syslog(LOG_ERR, "update_lcd() open LED %s failed!", file);
        goto ERROR;
    }
    syslog(LOG_INFO, "Open %s success", file);

    for (i = 0; i < loop_times; i++) {
        for (j = 0; j < LCD_LEVEL_SIZE; j++) {
            n = snprintf(cmd, MAX_CMD_LENGTH, "%d", LCD_BRIGHTNESS_LEVELS[j]);
	    if (n < 0) {
		syslog(LOG_ERR, "%s:%s:%d - snprintf error(%d)!\n",
		    __FILE__, __func__, __LINE__, n);
		goto ERROR;
	    }
            written = write(fd, cmd, strlen(cmd));
            if (written <= 0) goto ERROR;
            syslog(LOG_INFO, "Write brightness %d success", LCD_BRIGHTNESS_LEVELS[j]);
            usleep(delay_ms * 1000);
        }
    }
    close(fd);

    return;

ERROR:
    if (fd >= 0) {
        close(fd);
    }
    syslog(LOG_ERR, "update_lcd() failed!");
    return;
}
