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

#include "atcid_serial.h"
#include "atcid_cmd_dispatch.h"
#include "atcid_cust_cmd_process.h"
#include <string.h>
#include "stdio.h"

static int check_atcmd_input_char(char *ptr)
{
    int i = 0;
    char vulchar[] = {'`', ';', '|', '&', '<', '>', '$'};
    char *temp_ptr = NULL;
 
    if (NULL == ptr)
    {
        LOGATCI(LOG_ERR,"error: atcmd is null");
        return -1;
    }
 
    for (i = 0; i < sizeof(vulchar)/sizeof(char); i++) {
        temp_ptr = strchr(ptr, vulchar[i]);
        if (NULL != temp_ptr)
        {
            LOGATCI(LOG_ERR,"error: atcmd has special char: %c\n", vulchar[i]);
            return -1;
        } else {
           temp_ptr = NULL;
        }
    }
     
    return 0;
}
#ifdef  QUECTEL_AT_SUPPORT
int quectel_command_hdlr(char* line) 
{
    int i = 0, data_len = -1;
    int ret = -1;
    int table_size = 0;
    char response[MAX_AT_RESPONSE];
    char* line_cut = cut_cmd_line(line);
    
    memset(response, 0, sizeof(response));
    table_size = (sizeof(quectel_atcmd_table)/sizeof(customcmd_type));
    if (line_cut == NULL) 
    {
        return 0;
    }

    for (i = 0; i < table_size;i++) 
    {
        if (strcmp(line_cut, quectel_atcmd_table[i].cmdName) == 0) 
        {
            LOGATCI(LOG_DEBUG,"quectel command table[%d]:%s,size=%d",i,quectel_atcmd_table[i].cmdName,table_size);
            ATOP_t at_op = getAtOp(&line);
            LOGATCI(LOG_DEBUG,"The command op is %02x; Support op is %02x", at_op, quectel_atcmd_table[i].opType);
            if ((at_op & quectel_atcmd_table[i].opType) == 0) 
            {
                sprintf(response, "\r\n\r\nNOT IMPLEMENTED\r\n\r\n");
                data_len = strlen(response);

                LOGATCI(LOG_ERR,"[%s]:at_op not support!!!",quectel_atcmd_table[i].cmdName);
                writeDataToserialByResponseType(AT_NOT_IMPL);
                free(line_cut);
                return 1;
            }
            if (check_atcmd_input_char(line) < 0)
            {
                sprintf(response, "\r\n\r\nERROR\r\n\r\n");
                data_len = strlen(response);
                writeDataToserialByResponseType(AT_ERROR);
                free(line_cut);
                return 1;
            }

            ret = quectel_atcmd_table[i].cmd_handle_func(line, at_op, response);
            data_len = strlen(response);
            if (data_len >= 0) 
            {
                LOGATCI(LOG_DEBUG,"send_at_response:%s",response);
                writeDataToSerial(response, strlen(response));
            } 
            else 
            {
                LOGATCI(LOG_ERR,"NULL response from command handler function:%s", quectel_atcmd_table[i].cmdName);
                writeDataToserialByResponseType(ret);
            }
            free(line_cut);
            return 1; //The command is handled here
        }
    }
    free(line_cut);
    return 0;
}
#endif //#ifdef  QUECTEL_AT_SUPPORT

int generic_command_hdlr(char* line) {
    int i = 0;
    int table_size = 0;
    char* line_cut = cut_cmd_line(line);

    table_size = (sizeof(generic_cmd_table)/sizeof(generic_cmd_type));

    if (line_cut == NULL) {
        return 0;
    }

    if (strcmp(line_cut, "AT+CPMS") == 0 ||
            strcmp(line_cut, "AT+CMSS") == 0 ||
            strcmp(line_cut, "AT+CMGL") == 0 ||
            strcmp(line_cut, "AT+CMGR") == 0 ||
            strcmp(line_cut, "AT+CMGF") == 0) {
        free(line_cut);
        return 0;
    }

    if (strlen(line_cut) > strlen(AT_STRING) && isalpha(line_cut[strlen(AT_STRING)])) {
        // Basic command
        line_cut[strlen(AT_STRING) + 1] = '\0';
    }

    //LOGATCI(LOG_DEBUG, "The size of generic_cmd_table is %d", table_size);
    for (i = 0; i < table_size; i++) {
        if (strcmp(line_cut, generic_cmd_table[i].cmdName) == 0) {
            free(line_cut);
            return 1;
        }
    }
    free(line_cut);
    return 0;
}

int generic_sys_command_hdlr(char* line) {
    int i = 0;
    int table_size = 0;
    char* line_cut = cut_cmd_line(line);

    table_size = (sizeof(generic_sys_cmd_table)/sizeof(generic_cmd_type));

    if (line_cut == NULL) {
        return 0;
    }

    if (strlen(line_cut) > strlen(AT_STRING) && isalpha(line_cut[strlen(AT_STRING)])) {
        // Basic command
        line_cut[strlen(AT_STRING) + 1] = '\0';
    }

    //LOGATCI(LOG_DEBUG, "The size of generic_cmd_table is %d", table_size);
    for (i = 0; i < table_size; i++) {
        if (strcmp(line_cut, generic_sys_cmd_table[i].cmdName) == 0) {
            free(line_cut);
            return 1;
        }
    }
    free(line_cut);
    return 0;
}


int custom_command_hdlr(char* line) {
    int i = 0, ret = 0;
    int table_size = 0;
    char response[MAX_AT_RESPONSE];
    char* line_cut = cut_cmd_line(line);


    LOGATCI(LOG_DEBUG, "Enter");

    if (line_cut == NULL) {
        return 0;
    }

    memset(response, 0, sizeof(response));
    table_size = (sizeof(custom_cmd_table)/sizeof(customcmd_type));

    //LOGATCI(LOG_DEBUG, "table_size = %d", table_size);

    for (i = 0; i < table_size; i++) {
        //LOGATCI(LOG_DEBUG, "line_cut= %s, cmd = %s", line_cut, custom_cmd_table[i].cmdName);

        if (strcmp(line_cut, custom_cmd_table[i].cmdName) == 0) {
            ATOP_t at_op = getAtOp(&line);
            LOGATCI(LOG_DEBUG, "The command op is %d; Support op is %d", at_op,
                custom_cmd_table[i].opType);
            if ((at_op & custom_cmd_table[i].opType) == 0) {
                writeDataToserialByResponseType(AT_NOT_IMPL);
            } else {
                ret = custom_cmd_table[i].cmd_handle_func(line, at_op, response);
                if (strlen(response) > 0) {
                    writeDataToSerial(response, strlen(response));
                }

                writeDataToserialByResponseType(ret);

            }
            free(line_cut);
            return 1; //The command is handled here
        }
    }
    free(line_cut);
    return 0;
}

int process_cmd_line(char* line) {
    LOGATCI(LOG_DEBUG,"Enter");

    //Check the command is belonged to customized command table

    if (custom_command_hdlr(line)) {
        LOGATCI(LOG_DEBUG, "handle in custom_command_hdlr");
        return ATCI_TYPE;
    }

    if (generic_command_hdlr(line)) {
        LOGATCI(LOG_DEBUG, "handle in generic_command_hdlr");
        return GENERIC_TYPE;
    }

    if (generic_sys_command_hdlr(line)) {
        LOGATCI(LOG_DEBUG, "handle in generic_command_hdlr");
        return GENERIC_SYS_TYPE;
    }

#ifdef  QUECTEL_AT_SUPPORT
    if(quectel_command_hdlr(line))
    {
        LOGATCI(LOG_DEBUG, "handle in quectel_command_hdlr");
        return QUECTEL_COMMON_TYPE;
    }
#endif
    LOGATCI(LOG_DEBUG, "handle in MD");
    return RIL_TYPE;
}

char* cut_cmd_line(char* line) {
    char* result;
    int i = 0;
    int size = strlen(line);
    int len = size;
    for (i = 0; i < size; i++) {
        if (line[i] == '=' || line[i] == '?') {
            len = i;
            break;
        }
    }
    result = (char*)malloc((len + 1) * sizeof(char));
    if (result == NULL) {
        LOGATCI(LOG_DEBUG, "malloc result fail, return NULL");
        return NULL;
    }
    memcpy(result, line, len);
    result[len] = '\0';
    return result;
}
