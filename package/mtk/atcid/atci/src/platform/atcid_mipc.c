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
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <uci.h>   //modify by quectel for DSDS
#include "ql_utils.h"
#include "atcid_mipc.h"
#include "atcid_adaptation.h"
#include "atcid_common.h"
#include "atcid_serial.h"
#include "atcid_async_notify.h"
#include "mipc_msg_tlv_api.h"
#include "mipc_msg_host.h"
#include "mipc_msg_tlv_api.h"
#include "quectel_version.h"

#if 1//(!defined(MTK_DATACARD_ATCI)) || defined(TELEMATICS) || defined(RDKB)
#include "mipc_trm.h"
#endif
#include "platform.h"

#define ATCI_SIM "persist.vendor.service.atci.sim"

static pthread_t s_tid_atcid_urc;
bool listen_URC = false;
bool atci_mipc_inited = false;
bool atci_mipc_regCB = false;
static int acti_mipc_md_status = 0;
pthread_mutex_t urc_mutex;

extern Serial serial;

#if 1//(!defined(MTK_DATACARD_ATCI)) || defined(TELEMATICS) || defined(RDKB)
static void atcid_mipc_md_event_cb(void *priv_ptr, struct md_status_event* event_ptr)
{
    LOGATCI(LOG_INFO, "Received md event %d!!!",event_ptr->event_type);
    if((0 == acti_mipc_md_status) && (event_ptr->event_type == 11)) { //MD_STA_EV_STOP
        LOGATCI(LOG_INFO, "reset mipc_inited exit");
        acti_mipc_md_status = 1;
        listen_URC = false;
        atci_mipc_inited = false;
        LOGATCI(LOG_INFO, "reset mipc_inited true");
        LOGATCI(LOG_INFO, "Received MD_STA_EV_STOP event!!!");
        exit(0);
    }
}
#endif

static void atcid_mipc_ind_cb(mipc_msg_t *msg_ptr, void *priv_ptr) {
    char *urc_ptr;
    uint16_t urc_len;
    char *pos = NULL, *final_urc_ptr = NULL;
    uint16_t wio_fmt_urc_len = 0;

    urc_ptr = mipc_sys_at_ind_get_atcmd(msg_ptr, &urc_len);

    LOGATCI(LOG_INFO, "URC to atcid =%s\n", urc_ptr);

    pos = strchr(urc_ptr,'+');
    if(pos != NULL)
    {
        wio_fmt_urc_len = strlen(pos);
        while(1)
        {
            if((pos[wio_fmt_urc_len-1] == 0x0d) || (pos[wio_fmt_urc_len-1] == 0x0a))
                wio_fmt_urc_len--;
            else
                break;
        }
        LOGATCI(LOG_INFO, "URC final wio_fmt_urc_len =%d", wio_fmt_urc_len);
        final_urc_ptr = (char *)malloc(wio_fmt_urc_len+5);
        memset(final_urc_ptr, 0, wio_fmt_urc_len+5);
        strncat(final_urc_ptr, "\r\n", 3);
        strncat(final_urc_ptr, pos, wio_fmt_urc_len);
        strncat(final_urc_ptr, "\r\n", 3);

        pthread_mutex_lock(&urc_mutex);
        delayed_async_print(final_urc_ptr, strlen(final_urc_ptr));
        pthread_mutex_unlock(&urc_mutex);

        free(final_urc_ptr);
    }
}

static void atcid_mipc_ims_ind_cb(mipc_msg_t *msg_ptr, void *priv_ptr) 
{
    char *urc_ptr;
    uint16_t urc_len;
    char *pos = NULL;
    char final_urc_ptr[128] = "0";
    uint16_t wio_fmt_urc_len = 0;
    int i = 0 ;
    urc_ptr = mipc_ims_state_ind_get_uri(msg_ptr, &urc_len);
    LOGATCI(LOG_INFO, "URC to atcid =%s", urc_ptr);
    if (urc_ptr != NULL) 
    {
        pos = strstr(urc_ptr,"tel:");
        if(pos != NULL)
        {
            wio_fmt_urc_len = strlen(pos);
            LOGATCI(LOG_INFO, "URC final wio_fmt_urc_len =%d is %s", wio_fmt_urc_len, final_urc_ptr);
            for (i = 0; i < wio_fmt_urc_len; i++)
            {
                if (pos[i] == ',')
                {
                    LOGATCI(LOG_INFO, "delate unused string");
                    pos[i]='\0';
                }
            }
            while(1)
            {
                if((pos[wio_fmt_urc_len-1] == 0x0d) || (pos[wio_fmt_urc_len-1] == 0x0a))
                    wio_fmt_urc_len--;
                else
                    break;
            }
            snprintf(final_urc_ptr, sizeof(final_urc_ptr), "\r\n+QIND: %s\r\n",pos);
            LOGATCI(LOG_INFO, "URC final wio_fmt_urc_len =%d is %s", wio_fmt_urc_len, final_urc_ptr);
            pthread_mutex_lock(&urc_mutex);
            delayed_async_print(final_urc_ptr, strlen(final_urc_ptr));
            pthread_mutex_unlock(&urc_mutex);
        }
        else
        {
            LOGATCI(LOG_ERR, "Could't get Phone Number form MSISDN");
        }
    }
    else
    {
        LOGATCI(LOG_ERR, "Could't get info form IMS_IND");
    }
}

static void *ql_sim_imsi_check(void *arg)
{
    mipc_msg_t *msg_plmn_req_ptr = mipc_msg_init(MIPC_SIM_IMSI_REQ, MIPC_PS0);
    mipc_msg_t *msg_plmn_cnf_ptr;
    mipc_result_enum result;
    char plmn[7] = {0};
    char* imsi = NULL;
    uint8_t mnc_len = 0;
    char at_to_run[150] = {0};

    sleep(1);
    if (msg_plmn_req_ptr == NULL) 
    {
        LOGATCI(LOG_ERR, "msg_req_ptr is NULL\n");
        return NULL;
    }
    mipc_msg_set_timeout_once(3*1000);
    msg_plmn_cnf_ptr = mipc_msg_sync_timeout(msg_plmn_req_ptr);
    mipc_msg_deinit(msg_plmn_req_ptr);
    result = mipc_get_result(msg_plmn_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        imsi = mipc_sim_imsi_cnf_get_imsi(msg_plmn_cnf_ptr, NULL);
        
        mnc_len = mipc_sim_imsi_cnf_get_mnc_len(msg_plmn_cnf_ptr, 0);   
        if (mnc_len == 2) 
        {
            if (atoi(strncpy(plmn,imsi,5)) == 50501)
            {
                if(!strncmp(get_project_name(), "RG620T-EU", strlen("RG620T-EU")))
                {
                    snprintf(at_to_run, sizeof(at_to_run), "AT+ECFGSET=\"user_agent\", \"Telstra Quectel %s MT6990 %s\"\r\n", \
                        get_project_name(), get_project_rev_head());
                    run_at_cmd(at_to_run);
                }
                
            }
        }
    }
    mipc_msg_deinit(msg_plmn_cnf_ptr);
    return NULL;
}

static void atcid_mipc_sim_ind_cb(mipc_msg_t *msg_ptr, void *priv_ptr)
{
    unsigned int status = 0;
    pthread_t sim_pid;
    status  = mipc_sim_status_ind_get_status(msg_ptr, 0);
    LOGATCI(LOG_INFO, "status = %u\n", status);
    if(status == MIPC_SIM_STATUS_COMPLETE_READY)
    {
        pthread_create(&sim_pid, NULL, ql_sim_imsi_check, NULL);
    }
    return;
}

static void waitModemReady()
{
    //wait MD ready
    do {
        char buf[64];
        int ccci_fd = open("/sys/kernel/ccci/boot", O_RDONLY);
        if (ccci_fd >= 0) {
            memset(buf, 0, sizeof(buf));
            read(ccci_fd, buf, sizeof(buf));
            close(ccci_fd);

            if (strncmp(buf, "md1:4", 5) == 0) {
                //MD1 is ready
                LOGATCI(LOG_INFO, "MD ready!!!");
                break;
            }
        }
        LOGATCI(LOG_ERR, "MD not ready!!!");
        sleep(1);
    } while (1);
    return;
}

void* waitMIPCUrc(void *param) {
    UNUSED(param);

    waitModemReady();

    LOGATCI(LOG_INFO, "atcid start to wait for MIPC URC!!!");
    if(!listen_URC) {

        if (!atci_mipc_inited) {
        #if defined(TELEMATICS) || defined(RDKB)
            SETCOM("/dev/ttyCMIPC2");
        #endif
            mipc_init("atci");
            atci_mipc_inited = true;
            LOGATCI(LOG_INFO, "set mipc_inited true");
        }

    #if 1//(!defined(MTK_DATACARD_ATCI)) || defined(TELEMATICS) || defined(RDKB)
        if (!atci_mipc_regCB) {
            mipc_md_event_register(atcid_mipc_md_event_cb,NULL);
            atci_mipc_regCB = true;
        }
    #endif

        mipc_msg_register_ind(MIPC_MSG_PS0, MIPC_SYS_AT_IND, (void*)atcid_mipc_ind_cb, NULL);
        mipc_msg_register_ind(MIPC_MSG_PS1, MIPC_SYS_AT_IND, (void*)atcid_mipc_ind_cb, NULL);
        mipc_msg_register_ind(MIPC_MSG_PS0, MIPC_IMS_STATE_IND, (void*)atcid_mipc_ims_ind_cb, NULL);
        mipc_msg_register_ind(MIPC_MSG_PS1, MIPC_IMS_STATE_IND, (void*)atcid_mipc_ims_ind_cb, NULL);

        mipc_msg_register_ind(MIPC_MSG_PS0, MIPC_SIM_STATUS_IND, (void*)atcid_mipc_sim_ind_cb, NULL);
        listen_URC = true;
    }
    return NULL;
}

void* print_urc(void *param)
{
    while(1)
    {
        if((!get_at_cmd_in_progress()) && (get_async_list_count() > 0)) {
            delayed_async_output_list();
        }
        else
        {
            usleep(100*1000);
        }
    }
}

void startUrcThread() {
    int ret = 0;

    pthread_mutex_init(&urc_mutex, NULL);
    LOGATCI(LOG_INFO, "create atcid urc thread");
    ret = pthread_create(&s_tid_atcid_urc, NULL, waitMIPCUrc, NULL);
    atcid_async_notify_init();
    if (ret != 0) {
        LOGATCI(LOG_ERR, "fail to create atcid urc thread. errno:%d", errno);
        return;
    }

    LOGATCI(LOG_INFO, "create atcid urc thread successfully");
    pthread_join(s_tid_atcid_urc, NULL);

    pthread_t tid_print_urc;
    ret = pthread_create(&tid_print_urc, NULL, print_urc, NULL);
    if (ret != 0) {
        ALOGE("fail to create atcid urc thread. errno:%d", errno);
        return;
    }
}

void stopUrcThread() {
    listen_URC = false;
}

int check_chinese_char(char *ptr)
{
    int i;
    for(i = 0;i < strlen(ptr);i++)
    {
        if (((*(ptr+i))&0x80) > 0)
        {
            return 0;
        }
    }
    return 1;
}

void run_at_cmd(char* cmd) {
    if (!check_chinese_char(cmd))
    {
        goto err;
    }
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;
    char *atcicmd_req_ptr, *atcmd_res_ptr, *atci_res;
    uint16_t atcmd_req_len, atcmd_res_len;

    char simIDProperty[MTK_PROPERTY_VALUE_MAX]="0";
    int simID = 0;

    int char_sub = 0x1a;
    int char_esc = 0x1b;
    bool contain_sub_esc = false;
    char *pLine = cmd;

    LOGATCI(LOG_INFO, "AT command execution:%s", cmd);

    if (!check_modem_status())
    {
        goto err;
    }

    #ifdef QUECTEL_DSDS_ESUO   //modify by quectel for DSDS,start    
        //atci_property_get(ATCI_SIM, simIDProperty, "0");
        atci_property_get("atci_sim", simIDProperty, "0");
        simID = atoi(simIDProperty);
        if (simID == 0) {
            //LOGATCI(LOG_INFO, "AT command execution on SIM1 (init msg...)");
            msg_req_ptr = mipc_msg_init(MIPC_SYS_AT_REQ, MIPC_SIM0);
            //LOGATCI(LOG_INFO, "AT command execution on SIM1    (init msg done)");
        } else {
            //LOGATCI(LOG_INFO, "AT command execution on SIM2 (init msg...)");
            msg_req_ptr = mipc_msg_init(MIPC_SYS_AT_REQ, MIPC_SIM1);
            //LOGATCI(LOG_INFO, "AT command execution on SIM2 (init msg done) ");
        }    
    #else   //modify by quectel for DSDS,end
        atci_property_get(ATCI_SIM, simIDProperty, "0");
        simID = atoi(simIDProperty);
        if (simID == 0) {
            //LOGATCI(LOG_INFO, "AT command execution on MIPC_PS0 (init msg...)");
            msg_req_ptr = mipc_msg_init(MIPC_SYS_AT_REQ, MIPC_PS0);
            //LOGATCI(LOG_INFO, "AT command execution on SIM1    (init msg done)");
        } else {
            //LOGATCI(LOG_INFO, "AT command execution on MIPC_PS1 (init msg...)");
            msg_req_ptr = mipc_msg_init(MIPC_SYS_AT_REQ, MIPC_PS1);
            //LOGATCI(LOG_INFO, "AT command execution on SIM2 (init msg done) ");
        }
    #endif

    //LOGATCI(LOG_INFO, "listen_URC = %d, atci_mipc_inited = %d, atci_mipc_regCB = %d", listen_URC, atci_mipc_inited, atci_mipc_regCB);

    if (!listen_URC) {
    #if defined(TELEMATICS) || defined(RDKB)
        SETCOM("/dev/ttyCMIPC2");
    #endif
        startUrcThread();
        atci_mipc_inited = true;
        LOGATCI(LOG_INFO, "set mipc_inited true");

    #if 1//(!defined(MTK_DATACARD_ATCI)) || defined(TELEMATICS) || defined(RDKB)
        LOGATCI(LOG_INFO, "check atci_mipc_regCB");
        if (!atci_mipc_regCB) {
            mipc_md_event_register(atcid_mipc_md_event_cb,NULL);
            atci_mipc_regCB = true;
            LOGATCI(LOG_INFO, "set atci_mipc_regCB true");
        }
    #endif

    }

    //LOGATCI(LOG_INFO, "AT command string format processing...");

    while(*pLine != '\0') {
        int currChar = (int)*pLine;
        if(char_sub == currChar || char_esc == currChar) {
            contain_sub_esc = true;
        }
        pLine++;
    }

    //LOGATCI(LOG_INFO, "AT command string format processing... (2)");

    // add '\r' for all cmd
    // In AT spec, the string need including '\r' but not is the case that is the last sms message finished by Ctrl+Z or ESC
    // In MIPC spec said that the size passing need including '\0'
    if (!contain_sub_esc) {
        atcmd_req_len = strlen(cmd);
        atcicmd_req_ptr = (char *)malloc(atcmd_req_len + 2);  // 1 for '\r', 1 for '\0'

        if (atcicmd_req_ptr == NULL) {
            LOGATCI(LOG_INFO, "AT command execution : atcicmd_req_ptr alloc fail");
            sendCommandResponse("Unknown error\n");
        } else {
            strncpy(atcicmd_req_ptr, cmd, atcmd_req_len);
            atcicmd_req_ptr[atcmd_req_len] = '\r';
            atcicmd_req_ptr[atcmd_req_len + 1] = '\0';
            atcmd_req_len +=2;

            LOGATCI(LOG_INFO, "AT command execution (with \\r\\0):%s", atcicmd_req_ptr);
        }
    } else {
        atcmd_req_len = strlen(cmd);
        atcicmd_req_ptr = (char *)malloc(atcmd_req_len + 1);  // 1 for '\0'

        if (atcicmd_req_ptr == NULL) {
            LOGATCI(LOG_INFO, "AT command execution : atcicmd_req_ptr alloc fail");
            sendCommandResponse("Unknown error\n");
        } else {
            strncpy(atcicmd_req_ptr, cmd, atcmd_req_len);
            atcicmd_req_ptr[atcmd_req_len] = '\0';
            atcmd_req_len +=1;

            LOGATCI(LOG_INFO, "AT command execution (with \\r):%s", atcicmd_req_ptr);
        }
    }

    //LOGATCI(LOG_INFO, "AT command string format processing... (done)");

    //LOGATCI(LOG_INFO, "start mipc_msg_add_tlv");
    mipc_msg_add_tlv(msg_req_ptr, MIPC_SYS_AT_REQ_T_ATCMD, atcmd_req_len, atcicmd_req_ptr);

    //LOGATCI(LOG_INFO, "start mipc_msg_sync_timeout");
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);

    //LOGATCI(LOG_INFO, "start mipc_msg_deinit");
    mipc_msg_deinit(msg_req_ptr);

    //LOGATCI(LOG_INFO, "start mipc_get_result");
    result = mipc_get_result(msg_cnf_ptr);

    free(atcicmd_req_ptr);

    if (result == MIPC_RESULT_SUCCESS) {
        atcmd_res_ptr = mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_SYS_AT_CNF_T_ATCMD, &atcmd_res_len);
        int res_len = strlen(atcmd_res_ptr);
        //LOGATCI(LOG_INFO, "res_len is:%d, atcmd_res_len is %d\n", res_len, atcmd_res_len);
        atci_res = (char *)malloc(res_len + 3); //2+1, 2 for adding \r\n before response, 1 for termination

        if (atci_res == NULL) {
            LOGATCI(LOG_INFO, "AT command execution : atci_res alloc fail");
            sendCommandResponse("Unknown error\n");
        } else {
            strncpy(atci_res, "\r\n", 3); //add \r\n
            strncpy(atci_res + 2, atcmd_res_ptr, res_len); //copy reponse string
            atci_res[res_len + 2] = '\0';  // add termination

            LOGATCI(LOG_INFO, "AT response:%s with len:%d\n", atci_res, res_len);
            //send respons to atcid
            sendCommandResponse(atci_res);
            free(atci_res);
        }
    } else {
        sendCommandResponse("Unknown error\n");
        LOGATCI(LOG_INFO, "Failed to execute:%d\n", result);
    }
    mipc_msg_deinit(msg_cnf_ptr);

    if (!listen_URC) {
        //mipc_deinit();
    }
    return ;
err:
    LOGATCI(LOG_ERR, "modem crash !!! Get some info failed\n");
    sendCommandResponse("\r\nCME ERROR: 6666\r\n");
}

