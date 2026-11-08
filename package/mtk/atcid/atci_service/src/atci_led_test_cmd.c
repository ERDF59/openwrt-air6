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

#include "atci_led_test_cmd.h"
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

const char *LED_DEV_PATH = "/sys/class/leds";
const char *LED_FILE_BRIGHTNESS = "brightness";
const char *LED_FILE_TRIGGER = "trigger";
const char *LED_FILE_DELAYON = "delay_on";
const char *LED_FILE_DELAYOFF = "delay_off";
const char *LED_TRIGGER_TIMER = "timer";

#define TEST_LCD 0
#define TEST_GPIO_LED 1
#define TEST_PMIC_LED 1
#define TEST_CONSYS_LED 0

#define MAX_FILE_LENGTH 512
#define MAX_CMD_LENGTH 128

#define LED_SET_ON (1)
#define LED_SET_OFF (0)

#define LED_BRIGHTNESS_GPIO_ON 255
#define LED_BRIGHTNESS_PMIC_ON 255
#define LED_BRIGHTNESS_OFF 0
#define LED_DELAY_ON_MS 500
#define LED_DELAY_OFF_MS 500

bool g_led_test_on = false;

typedef enum {
    LED_TYPE_LCD = 0,
    LED_TYPE_GPIO,
    LED_TYPE_PMIC,
    LED_TYPE_CONSYS
} LED_TYPE;

void update_led(LED_TYPE type, bool on);

// LED device under /sys/class/leds/
const char *lcd_dev_list[] = {
    "lcd-backlight",
};
#define LCD_DEV_SIZE (sizeof(lcd_dev_list) / sizeof(const char *))

// GPIO LED device under /sys/class/leds/
const char *led_gpio_dev_list[] = {
    "led9501:red:gpio",
    "led9502:red:gpio",
    "led9504:green:gpio",
    "led9505:red:gpio",
    "led9506:green:gpio",
};
#define GPIO_LED_DEV_SIZE (sizeof(led_gpio_dev_list) / sizeof(const char *))

// PMIC LED device under /sys/class/leds/
const char *led_pmic_dev_list[] = {
    "led9515:green:pmic",
    "led9516:green:pmic",
    "led9517:red:system",
};
#define PMIC_LED_DEV_SIZE (sizeof(led_pmic_dev_list) / sizeof(const char *))

int led_test_cmd_handler(__attribute__((unused)) char *cmdline, ATOP_t at_op, char *response) {
    int  led_operation = -1;
    int  n;

    syslog(LOG_INFO, "Enter %s()", __func__);

    switch(at_op) {
        // AT%LEDTEST=x
        case AT_SET_OP:
            if (at_tok_nextint(&cmdline, &led_operation) == -1) {
                syslog(LOG_ERR, "Invalid commnad, format AT&LEDTEST=x");
		n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nERROR\r\n\r\n");
		if (n < 0) {
		    syslog(LOG_ERR,
			    "%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
		}
                break;
            }
            if (LED_SET_ON == led_operation) {
                if (g_led_test_on) {
                    syslog(LOG_INFO, "Already ON, skip AT\%LEDTEST=1");
		    n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nOK\r\n\r\n");
		    if (n < 0) {
			syslog(LOG_ERR,
				"%s:%s:%d - snprintf error(%d)!\n",
					__FILE__, __func__, __LINE__, n);
		    }
                    break;
                }
                if (TEST_LCD) update_led(LED_TYPE_LCD, true);
                if (TEST_GPIO_LED) update_led(LED_TYPE_GPIO, true);
                if (TEST_PMIC_LED) update_led(LED_TYPE_PMIC, true);
                if (TEST_CONSYS_LED) update_led(LED_TYPE_CONSYS, true);

                syslog(LOG_INFO, "LED%TEST=1 SUCCESS!");
		n = snprintf(response, MAX_AT_RESPONSE,"\r\n\r\nOK\r\n\r\n");
		if (n < 0) {
		    syslog(LOG_ERR,
			    "%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
		}
                g_led_test_on = true;
            } else if (LED_SET_OFF == led_operation) {
                if (!g_led_test_on) {
                    syslog(LOG_INFO, "Already OFF, skip AT\%LEDTEST=1");
		    n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nOK\r\n\r\n");
		    if (n < 0) {
			syslog(LOG_ERR,
				"%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
		    }
                    break;
                }
                if (TEST_LCD) update_led(LED_TYPE_LCD, false);
                if (TEST_GPIO_LED) update_led(LED_TYPE_GPIO, false);
                if (TEST_PMIC_LED) update_led(LED_TYPE_PMIC, false);
                if (TEST_CONSYS_LED) update_led(LED_TYPE_CONSYS, false);

                syslog(LOG_INFO, "LED%TEST=0 SUCCESS!");
		n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nOK\r\n\r\n");
		if (n < 0) {
		    syslog(LOG_ERR,
			    "%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
		}
                g_led_test_on = false;
            } else {
                syslog(LOG_ERR, "Unexpected operation code %d", led_operation);
		n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nERROR\r\n\r\n");
		if (n < 0) {
		    syslog(LOG_ERR,
			    "%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
		}
            }
            break;
        case AT_ACTION_OP:
            syslog(LOG_ERR, "LEDTEST ACTION_OP not supported!");
	    n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nERROR\r\n\r\n");
	    if (n < 0) {
		syslog(LOG_ERR,
			"%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
	    }
            break;
        case AT_READ_OP:
            syslog(LOG_ERR, "LEDTEST READ_OP not supported!");
	    n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nERROR\r\n\r\n");
	    if (n < 0) {
		syslog(LOG_ERR,
			"%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
	    }
            break;
        case AT_TEST_OP:
            syslog(LOG_ERR, "LEDTEST TEST_OP not supported!");
	    n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nERROR\r\n\r\n");
	    if (n < 0) {
		syslog(LOG_ERR,
			"%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
	    }
            break;
        default:
            syslog(LOG_ERR, "LEDTEST operation %d not supported!", at_op);
	    n = snprintf(response, MAX_AT_RESPONSE, "\r\n\r\nERROR\r\n\r\n");
	    if (n < 0) {
		syslog(LOG_ERR,
			"%s:%s:%d - snprintf error(%d)!\n", __FILE__, __func__, __LINE__, n);
	    }
            break;
    }

    return 0;
}

void update_led(LED_TYPE type, bool on) {
    char file[MAX_FILE_LENGTH] = {0};
    char cmd[MAX_CMD_LENGTH] = {0};

    unsigned int i = 0;
    int fd;
    int written = 0;
    const char **dev_list = NULL;
    unsigned int dev_size = 0;
    bool bConsysLed = false;
    unsigned int max_brightness = LED_BRIGHTNESS_GPIO_ON;
    int n;

    switch (type) {
        case LED_TYPE_LCD:
            dev_list = &(lcd_dev_list[0]);
            dev_size = LCD_DEV_SIZE;
            break;
        case LED_TYPE_GPIO:
            dev_list = &(led_gpio_dev_list[0]);
            dev_size = GPIO_LED_DEV_SIZE;
            max_brightness = LED_BRIGHTNESS_GPIO_ON;
            break;
        case LED_TYPE_PMIC:
            dev_list = &(led_pmic_dev_list[0]);
            dev_size = PMIC_LED_DEV_SIZE;
            max_brightness = LED_BRIGHTNESS_PMIC_ON;
            break;
        case LED_TYPE_CONSYS:
            bConsysLed = true;
            break;
        default:
            goto ERROR;
    }

    syslog(LOG_INFO, "Enter update_led(%d, %d)", type, on);

    if (bConsysLed) {
        // Use iwpriv command to test
        //      iwpriv ra0 set led_setting=AA-BB-CC-DD-EE-FF-GG-HH
        // HH 03 is blink in 1 HZ

        // index 00: 2.4G LED
        system("iwpriv ra0 set led_setting=00-00-00-00-00-00-00-03");

        // index 01: 5G LED
        system("iwpriv ra0 set led_setting=01-00-00-00-00-00-00-03");

        // index 02: WPS LED
        system("iwpriv ra0 set led_setting=02-00-00-00-00-00-00-03");

        return;
    }

    // Blink LCD
    if (on) {
        for (i = 0; i < dev_size; i++) {
            // Force set brightness to max value
	    n = snprintf(file, MAX_FILE_LENGTH,
		    "%s/%s/%s", LED_DEV_PATH, dev_list[i], LED_FILE_BRIGHTNESS);
	    if (n < 0) {
		syslog(LOG_ERR, "%s:%s:%d - snprintf error(%d)!\n",
			__FILE__, __func__, __LINE__, n);
		goto ERROR;
	    }
            fd = open(file, O_RDWR);
            if (fd < 0) {
                syslog(LOG_ERR, "update_lcd() open LED %s failed!", file);
                continue;
            }
            syslog(LOG_INFO, "Open %s success", file);

            n = snprintf(cmd, MAX_CMD_LENGTH, "%d", max_brightness);
	    if (n < 0) {
		syslog(LOG_ERR, "%s:%s:%d - snprintf error(%d)!\n",
			__FILE__, __func__, __LINE__, n);
		goto ERROR;
	    }
            written = write(fd, cmd, strlen(cmd));
            if (written <= 0) {
                syslog(LOG_ERR, "write brightness failed!");
                close(fd);
                continue;
            }
            syslog(LOG_INFO, "Write brightness %d success", max_brightness);
            close(fd);

            // Set LED trigger to "timer"
	    n = snprintf(file, MAX_FILE_LENGTH,
		    "%s/%s/%s", LED_DEV_PATH, dev_list[i], LED_FILE_TRIGGER);
	    if (n < 0) {
		syslog(LOG_ERR, "%s:%s:%d - snprintf error(%d)!\n",
			__FILE__, __func__, __LINE__, n);
		goto ERROR;
	    }
            fd = open(file, O_RDWR);
            if (fd < 0) continue;
            syslog(LOG_INFO, "Open %s success", file);

            n = snprintf(cmd, MAX_CMD_LENGTH, "%s", LED_TRIGGER_TIMER);
	    if (n < 0) {
		syslog(LOG_ERR, "%s:%s:%d - snprintf error(%d)!\n",
			__FILE__, __func__, __LINE__, n);
		goto ERROR;
	    }
            written = write(fd, cmd, strlen(cmd));
            if (written <= 0) {
                syslog(LOG_ERR, "write trigger failed!");
                close(fd);
                continue;
            }
            syslog(LOG_INFO, "Write trigger %s success", LED_TRIGGER_TIMER);
            close(fd);

            // Set LED blinking in 1 HZ
	    n = snprintf(file, MAX_FILE_LENGTH,
		    "%s/%s/%s", LED_DEV_PATH, dev_list[i], LED_FILE_DELAYON);
	    if (n < 0) {
		syslog(LOG_ERR, "%s:%s:%d - snprintf error(%d)!\n",
			__FILE__, __func__, __LINE__, n);
		goto ERROR;
	    }
            fd = open(file, O_RDWR);
            if (fd < 0) continue;
            syslog(LOG_INFO, "Open %s success", file);

            n = snprintf(cmd, MAX_CMD_LENGTH, "%d", LED_DELAY_ON_MS);
	    if (n < 0) {
		syslog(LOG_ERR, "%s:%s:%d - snprintf error(%d)!\n",
			__FILE__, __func__, __LINE__, n);
		goto ERROR;
	    }
            written = write(fd, cmd, strlen(cmd));
            if (written <= 0) {
                syslog(LOG_ERR, "write delay_on failed!");
                close(fd);
                continue;
            }
            syslog(LOG_INFO, "Write delay_on %d success", LED_DELAY_ON_MS);
            close(fd);

	    n = snprintf(file, MAX_FILE_LENGTH,
		    "%s/%s/%s", LED_DEV_PATH, dev_list[i], LED_FILE_DELAYOFF);
	    if (n < 0) {
		syslog(LOG_ERR, "%s:%s:%d - snprintf error(%d)!\n",
			__FILE__, __func__, __LINE__, n);
		goto ERROR;
	    }
            fd = open(file, O_RDWR);
            if (fd < 0) continue;
            syslog(LOG_INFO, "Open %s success", file);

	    n = snprintf(cmd, MAX_CMD_LENGTH, "%d", LED_DELAY_OFF_MS);
	    if (n < 0) {
		syslog(LOG_ERR, "%s:%s:%d - snprintf error(%d)!\n",
			__FILE__, __func__, __LINE__, n);
		goto ERROR;
	    }
            written = write(fd, cmd, strlen(cmd));
            if (written <= 0) {
                syslog(LOG_ERR, "write delay_off failed!");
                close(fd);
                continue;
            }
            syslog(LOG_INFO, "Write delay_off %d success", LED_DELAY_OFF_MS);
            close(fd);
        }
    // OFF LCD
    } else {
        for (i = 0; i < dev_size; i++) {
            // Force set brightness to LED_BRIGHTNESS_OFF(0)
	    n = snprintf(file, MAX_FILE_LENGTH,
		    "%s/%s/%s", LED_DEV_PATH, dev_list[i], LED_FILE_BRIGHTNESS);
	    if (n < 0) {
		syslog(LOG_ERR, "%s:%s:%d - snprintf error(%d)!\n",
			__FILE__, __func__, __LINE__, n);
		goto ERROR;
	    }
            fd = open(file, O_RDWR);
            if (fd < 0) continue;
            syslog(LOG_INFO, "Open %s success", file);

	    n = snprintf(cmd, MAX_CMD_LENGTH, "%d", LED_BRIGHTNESS_OFF);
	    if (n < 0) {
		syslog(LOG_ERR, "%s:%s:%d - snprintf error(%d)!\n",
			__FILE__, __func__, __LINE__, n);
		goto ERROR;
	    }
            written = write(fd, cmd, strlen(cmd));
            if (written <= 0) {
                syslog(LOG_ERR, "write brightness 0 failed!");
                close(fd);
                continue;
            }
            syslog(LOG_INFO, "Write brightness %d success", LED_BRIGHTNESS_OFF);
            close(fd);
        }
    }
    return;

ERROR:
    if (fd >= 0) {
        close(fd);
    }
    syslog(LOG_ERR, "update_led(%d) failed!", on);
    return;
}

