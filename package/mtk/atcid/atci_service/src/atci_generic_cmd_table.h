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

#ifndef ATCI_GENERIC_CMD_TABLE_H
#define ATCI_GENERIC_CMD_TABLE_H

#define MAX_AT_COMMAND_LEN 32
#define MAX_AT_RIL_COMMAND_LEN 512
#define MAX_AT_RESPONSE 2048
#define MAX_DATA_SIZE 6144

#include "atci_system_cmd.h"
#if defined(ENABLE_GPS_AT_CMD)
    #include "atci_gps_cmd.h"
#endif
#if defined(ENABLE_WLAN_AT_CMD)
    #include "atci_wlan_cmd.h"
#endif

#if defined(ENABLE_BLK_VIBR_AT_CMD)
#include "atci_lcdbacklight_vibrator_cmd.h"
#endif
#include "atci_pq_cmd.h"
#if defined(ENABLE_CCAP_AT_CMD)
    #include "atci_cct_cmd.h"
#endif
#include "atci_touch_cmd.h"
#include "atci_bts_cmd.h"
#include "atci_nand_cmd.h"
#include "atci_suspend_cmd.h"
#include "atci_sgmii_cmd.h"
#include "atci_rgmii_cmd.h"
#include "atci_gpio_cmd.h"
#include "atci_kpd_cmd.h"
#include "atci_pcie_cmd.h"
#include "atci_typec_cmd.h"

#include "atci_mmc_cmd.h"
#include "atci_msdc_cmd.h"
#include "atci_led_test_cmd.h"
#include "atci_lcd_test_cmd.h"
#include "atci_efuse_cmd.h"

int process_generic_command(char* line, int size);
char* cut_cmd_line(char* line);

typedef struct generic_cmd_type
{
    char cmdName[MAX_AT_COMMAND_LEN];                           //The prefix of AT command name
    ATOP_t opType;                                              //The suppport operation type
    //Generic command handler function
    int (*cmd_handle_func)(char* cmdline, ATOP_t opType, char* response);
} generic_cmd_type;


//Handle those commands in ATCI generic service
static generic_cmd_type generic_cmd_table[] = {
#if defined(ENABLE_GPS_AT_CMD)
        {"AT%GPS", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_gps_handler}, //Add for GPS test
        {"AT%GNSS", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, pas_gnss_handler},
        {"AT%CWGPS", AT_TEST_OP | AT_SET_OP | AT_READ_OP, pas_cw_handler},
#endif
#if defined(ENABLE_BLK_VIBR_AT_CMD)
        {"AT%LEDON",      AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, lcdbacklight_power_on_cmd_handler},
#endif
        {"AT%LEDTEST",      AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, led_test_cmd_handler},
        {"AT%LCDTEST",      AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, lcd_test_cmd_handler},
        {"AT%FBOOT", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, system_fboot_handler},
        {"AT%RESTART", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, system_restart_handler},
        {"AT%POWEROFF", AT_ACTION_OP | AT_TEST_OP | AT_SET_OP, system_poweroff_handler},
        {"AT%SAFERESTART", AT_ACTION_OP, system_safe_restart_handler},
#if defined(ENABLE_CCAP_AT_CMD)
        {"AT%CCT", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, cct_cmd_handler},
#endif
        {"AT%NAND", AT_ACTION_OP, nand_cmd_handler},
#if defined(ENABLE_BLK_VIBR_AT_CMD)
        {"AT+CBKLT", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, lcd_backlight_cmd_handler},
#endif
        {"AT+BTS", AT_READ_OP, bts_cmd_handler},
        {"AT%SLEEP", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, auto_sleep_cmd_handler},
        {"AT%SGMII", AT_ACTION_OP, sgmii_cmd_handler},
        {"AT%RGMII", AT_ACTION_OP, rgmii_cmd_handler},
        {"AT+KEY", AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, keypad_cmd_handler},
        {"AT%PCIE", AT_SET_OP, pcie_cmd_handler},
        {"AT%MEMCARD",      AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, mmc_cmd_handler},
        {"AT%EMMC",      AT_ACTION_OP | AT_READ_OP | AT_TEST_OP | AT_SET_OP, msdc_cmd_handler},
        {"AT+USBTYPEC", AT_ACTION_OP, usb_typec_cmd_handler},
        {"AT+TGPIO", AT_SET_OP, gpio_cmd_handler},
        {"AT%EHUID", AT_ACTION_OP | AT_READ_OP, efuse_huid_cmd_handler},
    };
#endif
