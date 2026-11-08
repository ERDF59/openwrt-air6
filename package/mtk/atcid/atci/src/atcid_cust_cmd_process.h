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

#ifndef ATCID_CUST_CMD_PROCESS_H
#define ATCID_CUST_CMD_PROCESS_H

#include "atcid_cmd_dispatch.h"
#include "atcid_cust_cmd.h"
#include "atcid_sim_cmd.h"
#include "atcid_universal_ant_cmd.h"

typedef struct custom_cmd_type
{
    //The prefix of AT command name
    char cmdName[MAX_AT_COMMAND_LEN];
    ATOP_t opType;
    //Command Handler function in case of no IOControl support
    ATRESPONSE_t (*cmd_handle_func)(char* cmdline, ATOP_t opType, char* response);
} customcmd_type;

static customcmd_type custom_cmd_table[] = {
        {"ATQ0V1E", AT_BASIC_OP, pas_echo_handler},                 //Add for device echo
        {"ATE0", AT_BASIC_OP, pas_echo_handler},                    //Add for device echo
        {"ATE1", AT_BASIC_OP, pas_echo_handler},                    //Add for device echo
        {"AT+CCLK", AT_TEST_OP | AT_SET_OP | AT_READ_OP, pas_cclk_handler},
        {"AT+CSDF", AT_TEST_OP | AT_SET_OP | AT_READ_OP, pas_csdf_handler},
        {"AT+ESUO", AT_TEST_OP | AT_SET_OP | AT_READ_OP, pas_esuo_handler},
        {"AT%REBOOT", AT_ACTION_OP, pas_reboot_handler},
        {"AT%EURC", AT_TEST_OP | AT_SET_OP | AT_READ_OP, pas_eurc_handler},
        {"AT+SIM", AT_SET_OP, sim_cmd_handler},
        {"AT+UANT", AT_SET_OP, universal_ant_test_handler},
        {"AT+GDCXO", AT_READ_OP, pas_check_clock_handler},
    };

#ifdef  QUECTEL_AT_SUPPORT

#include "ql_atcmd_table.h"
#undef  QUECTEL_ATCMD
#define QUECTEL_ATCMD(name,op,cb) {name,op,cb}, 

static customcmd_type quectel_atcmd_table[] = 
{
        #include "ql_atcmd_def.h"
};

int quectel_common_at_hdlr(char* line); 

#endif // ifdef  QUECTEL_AT_SUPPORT

#endif
