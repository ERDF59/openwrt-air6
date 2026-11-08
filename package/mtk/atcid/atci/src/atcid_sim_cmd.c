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
#include "atcid_sim_cmd.h"
#include "mipc_msg.h"
#include "mipc_msg_tlv_api.h"
#include "mipc_msg_tlv_const.h"
#include "mipc_msg_host.h"


typedef struct {
    uint8_t card_state[20];
    uint8_t slots_state;
    uint8_t logical_idx;
    uint8_t eid[32];
    uint8_t atr[80];
    uint8_t iccid[20];
} atci_sim_slots_info_struct;


typedef struct {
    /** @brief result code*/
    mipc_result_enum result_code;

    uint8_t sim_count;
    mipc_sim_slots_info_struct4 info[4];
} atci_sim_slots_status_struct;

// MIPC_MSG.SIM_STATE_CNF
enum atci_sim_slots_status_cnf_tlv_enum {
    mipc_sim_slots_status_cnf_tlv_NONE = 0,
    // sim slote state
    // type = uint8_t
    ATCI_SIM_SLOTS_STATUS_CNF_T_SIM_COUNT                              = 0x100,
    // sim slote id
    // type = struct_array
    ATCI_SIM_SLOTS_STATUS_CNF_T_SLOTS_INFO                             = 0x101,
};

#define ATCI_SIM_GET_EUICC_SLOTE_STATUS_REQ 0x53b

extern bool atci_mipc_inited;

mipc_result_enum atci_sim_slots_status_get_cnf_decode(mipc_msg_t *msg_cnf_ptr, atci_sim_slots_status_struct *result_ptr)
{
    void* val_ptr;
    uint16_t info_len = 0;

    if (msg_cnf_ptr == NULL) { //timeout
        if (result_ptr) {
            result_ptr->result_code = MIPC_RESULT_TIMEOUT;
        }
        return MIPC_RESULT_TIMEOUT;
    }

    do {
        result_ptr->result_code = mipc_msg_get_val_uint32(msg_cnf_ptr, MIPC_T_RESULT, MIPC_RESULT_FAILURE);
        LOGATCI(LOG_INFO, "detect_sim_state result_code = %d\n", result_ptr->result_code);
        if (result_ptr->result_code != MIPC_RESULT_SUCCESS) {
            break;
        }

        uint8_t sim_count = mipc_msg_get_val_uint8(msg_cnf_ptr, MIPC_SIM_GET_EUICC_SLOTS_STATUS_CNF_T_SLOTS_INFO_COUNT, 0xff);
        LOGATCI(LOG_INFO, "detect_sim_state sim_count = %d\n", sim_count);
        if (0xff == sim_count) {
            break;
        }
        result_ptr->sim_count = sim_count;
        if (sim_count >= 1) {
            val_ptr = mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_SIM_GET_EUICC_SLOTS_STATUS_CNF_T_SLOTS_INFO_LIST, &info_len);
            LOGATCI(LOG_INFO, "detect_sim_state info_len=%d\n", info_len);
            if (NULL == val_ptr) break;
            memcpy(result_ptr->info, val_ptr, info_len);
            LOGATCI(LOG_INFO, "detect_sim_state state=%s\n", (char*)val_ptr);
        }
    } while (0);
    return result_ptr->result_code;
}

 mipc_result_enum atci_sim_slots_status_get_sync(atci_sim_slots_status_struct *result_ptr) {
    mipc_msg_t *msg_req_ptr = mipc_msg_init(MIPC_SIM_GET_EUICC_SLOTS_STATUS_REQ, MIPC_MSG_PS0);
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum ret;

    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    LOGATCI(LOG_INFO, "detect_sim_state detect started\n");
    ret = atci_sim_slots_status_get_cnf_decode(msg_cnf_ptr, result_ptr);
    mipc_msg_deinit(msg_req_ptr);
    mipc_msg_deinit(msg_cnf_ptr);
    return ret;

}

ATRESPONSE_t detect_sim_present(int sim_count, char* response) {
    int pass_count = 0;
    ATRESPONSE_t ret = AT_OK;
    //LOGATCI(LOG_INFO, "detect_sim_present execution sim count:%d\n", sim_count);
    LOGATCI(LOG_INFO, "detect_sim_present execution sim count:%d\n", sim_count);

    if (sim_count <=0 || sim_count > 4) {
        sim_count = 1;
    }
    mipc_sys_sim_ps_const_enum sims[] = {
       MIPC_MSG_PS0,
       MIPC_MSG_PS1,
       MIPC_MSG_PS2,
       MIPC_MSG_PS3
       };

    for (int i =0; i < sim_count; i++) {
        mipc_sys_sim_ps_const_enum sim_ps_id = sims[i];
        mipc_msg_t *msg_radio_req_ptr = mipc_msg_init(MIPC_NW_SET_RADIO_STATE_REQ, sim_ps_id);
        mipc_msg_t *msg_req_ptr = mipc_msg_init(MIPC_SIM_STATE_REQ, sim_ps_id);
        mipc_msg_t *msg_cnf_ptr;
        mipc_result_enum result;
        mipc_sim_state_const_enum sim_state;

#if (!defined(MTK_DATACARD_ATCI)) || defined(TELEMATICS)
        // Make sure radio on before SIM detection
        mipc_nw_set_radio_state_req_add_sw_state(msg_radio_req_ptr, MIPC_NW_RADIO_STATE_ON);
        mipc_nw_set_radio_state_req_add_hw_state(msg_radio_req_ptr, MIPC_NW_RADIO_STATE_ON);

        msg_cnf_ptr = mipc_msg_sync_timeout(msg_radio_req_ptr);

        mipc_msg_deinit(msg_radio_req_ptr);
        mipc_msg_deinit(msg_cnf_ptr);

        sleep(2);
        LOGATCI(LOG_INFO, "Radio on with sleep 2 sec.\n");
#else
        // MD don't allow to set radio in meta mode
        // and need MD has auto power on config, AP can skip it
        mipc_msg_deinit(msg_radio_req_ptr);
#endif

        //uint32_t sim_val;
        msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
        LOGATCI(LOG_INFO, "detect_sim_present detect started\n");
        mipc_msg_deinit(msg_req_ptr);
        result = mipc_get_result(msg_cnf_ptr);
        if (result == MIPC_RESULT_SUCCESS) {
           sim_state = mipc_sim_state_cnf_get_state(msg_cnf_ptr, MIPC_SIM_STATE_UNKNOWN);
           //sim_val = mipc_sim_state_cnf_get_sim_id(msg_cnf_ptr, 0);
           //LOGD("SIM ID:0x%x\n", sim_val);
           if (sim_state == MIPC_SIM_STATE_ACTIVE
               || sim_state == MIPC_SIM_STATE_OFF) {
               pass_count++;
           } else if (sim_state == MIPC_SIM_STATE_ACTIVE_ESIM
               || sim_state == MIPC_SIM_STATE_ACTIVE_ESIM_NOPROFILE) {
               pass_count++;
           }
           LOGATCI(LOG_INFO, "detect_sim_present sim%d, state=%d, pass_count=%d\n", i+1, sim_state, pass_count);
        } else {
           LOGATCI(LOG_ERR, "detect_sim_present Failed to execute\n");
        }
        mipc_msg_deinit(msg_cnf_ptr);
    }

   /* //retry
    if (pass_count != sim_count) {
       pass_count = 0;
       atci_sim_slots_status_struct slots_status = {
           mipc_result_const_NONE,
           0,
           {{"", 0, 0, "", "", ""}, {"", 0, 0, "", "", ""}, {"", 0, 0, "", "", ""}, {"", 0, 0, "", "", ""}}
       };
       mipc_api_result_enum result = atci_sim_slots_status_get_sync(&slots_status);
       if (result == MIPC_API_RESULT_SUCCESS) {
           for (int i =0; i < sim_count; i++) {
               if (strcmp((char*)slots_status.info[i].card_state, "EMPTY_EUICC") == 0
                || strcmp((char*)slots_status.info[i].card_state, "READY") == 0
                || strcmp((char*)slots_status.info[i].card_state, "SIM PIN") == 0) {
                   pass_count++;
               }
               ALOGI("retry detect sim%d, state=%s, pass_count=%d", i+1, slots_status.info[i].card_state, pass_count);
           }
       }
    } */

    if (pass_count == sim_count) {
        ret = AT_OK;
    } else {
        ret = AT_ERROR;
    }
    return ret;
}


ATRESPONSE_t detect_sim_state(int sim_count, char* response) {
    //LOGATCI(LOG_INFO, "detect_sim_state execution sim count:%d\n", sim_count);
    LOGATCI(LOG_INFO, "detect_sim_state execution sim count:%d\n", sim_count);
    ATRESPONSE_t ret = AT_OK;
    if (sim_count <=0 || sim_count > 4) {
        sim_count = 1;
    }

    atci_sim_slots_status_struct slots_status;
    memset(&slots_status, 0, sizeof(atci_sim_slots_status_struct));
    mipc_result_enum result = atci_sim_slots_status_get_sync(&slots_status);
    if (result == MIPC_RESULT_SUCCESS) {

        if (sim_count == 2) {
            snprintf(response, MAX_AT_RESPONSE, "\r\n%s, %s\r\n", slots_status.info[0].card_state, slots_status.info[1].card_state);
            LOGATCI(LOG_INFO, "detect_sim_state sim1state=%s, sim2state=%s\n", slots_status.info[0].card_state, slots_status.info[1].card_state);
        } else {
            LOGATCI(LOG_INFO, "detect_sim_state sim1, state=%s\n", slots_status.info[0].card_state);
            snprintf(response, MAX_AT_RESPONSE, "\r\n%s\r\n", slots_status.info[0].card_state);
        }
    } else {
       ret = AT_ERROR;
    }
    return ret;
}

ATRESPONSE_t detect_sim_status(int sim_count, char* response, int mode) {
    //LOGATCI(LOG_INFO, "detect_sim_present execution sim count:%d\n", sim_count);
    LOGATCI(LOG_INFO, "detect_sim_status execution sim count:%d, mode:%d\n", sim_count, mode);
    ATRESPONSE_t ret = AT_OK;
    if (sim_count <=0 || sim_count > 4) {
        sim_count = 1;
    }
    mipc_sys_sim_ps_const_enum sims[] = {
       MIPC_MSG_PS0,
       MIPC_MSG_PS1,
       MIPC_MSG_PS2,
       MIPC_MSG_PS3
       };
    uint8_t status[4];

    for (int i =0; i < sim_count; i++) {
        mipc_msg_t* msg_req_ptr;
        mipc_msg_t* msg_cnf_ptr;
        mipc_result_enum result;

        msg_req_ptr = mipc_msg_init(MIPC_SIM_STATUS_REQ, sims[i]);
        // 1 means full mode
        mipc_msg_add_tlv_uint8(msg_req_ptr, MIPC_SIM_STATUS_REQ_T_MODE, 0);
        msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
        mipc_msg_deinit(msg_req_ptr);
        result = mipc_msg_get_val_uint32(msg_cnf_ptr, MIPC_T_RESULT, MIPC_RESULT_FAILURE);
        if (result == MIPC_RESULT_SUCCESS) {
            //get result status
            status[i] = mipc_msg_get_val_uint8(msg_cnf_ptr, MIPC_SIM_STATUS_CNF_T_STATUS, 0xFF);
            LOGATCI(LOG_INFO, "detect_sim_status status[%d]: %d\n", i, status[i]);
        } else {
            LOGATCI(LOG_INFO, "detect_sim_status status[%d] failed, result=%d\n", i, result);
            status[i] = 0;
        }
        mipc_msg_deinit(msg_cnf_ptr);
    }

    if (mode == 2) {
        if (sim_count == 2) {
            snprintf(response, MAX_AT_RESPONSE, "\r\n%d, %d\r\n", status[0], status[1]);
        } else {
            snprintf(response, MAX_AT_RESPONSE, "\r\n%d\r\n", status[0]);
        }
    } else {
        if (sim_count == 1 && status[0] > MIPC_SIM_STATUS_BUSY) {
            ret = AT_OK;
        } else if (sim_count == 2 && status[0] > MIPC_SIM_STATUS_BUSY && status[1] > MIPC_SIM_STATUS_BUSY) {
            ret = AT_OK;
        } else {
            ret = AT_ERROR;
        }
    }
    return ret;
}

ATRESPONSE_t sim_cmd_handler(char* cmdline, ATOP_t at_op, char* response) {
    LOGATCI(LOG_DEBUG, "\ndetect_sim cmdline=%s, at_op=%d, atci_mipc_inited=%d\n", cmdline, at_op, atci_mipc_inited);
    ATRESPONSE_t ret = AT_OK;
    if (!atci_mipc_inited) {
    #if defined(TELEMATICS) || defined(RDKB)
        SETCOM("/dev/ttyCMIPC2");
    #endif
        mipc_init("atci");
        atci_mipc_inited = true;
        LOGATCI(LOG_INFO, "set mipc_inited true (SIM)");
    }
    LOGATCI(LOG_DEBUG, "\ndetect_sim check mipc init done");
    // AT+SIM=x,y
    // x indicate sim count (1, 4)
    // y indicate test mode, 1 normal mode, 2 enhanced mode
    switch(at_op){
        case AT_SET_OP:
            if (strncmp(cmdline, "1,1", 3) == 0) {
                ret = detect_sim_present(1, response);
            } else if (strncmp(cmdline, "2,1", 3) == 0) {
                ret = detect_sim_status(2, response, 1);
            } else if (strncmp(cmdline, "1,2", 3) == 0) {
                ret = detect_sim_status(1, response, 2);
            } else if (strncmp(cmdline, "2,2", 3) == 0) {
                ret = detect_sim_status(2, response, 2);
            } else if (strncmp(cmdline, "1,3", 3) == 0) {
                ret = detect_sim_state(1, response);
            } else if (strncmp(cmdline, "2,3", 3) == 0) {
                ret = detect_sim_state(2, response);
            }else {
                snprintf(response, MAX_AT_RESPONSE, "\r\ndetect_sim AT+SIM Parameter wrong\r\n");
                ret = AT_ERROR;
            }
            break;
        default:
            break;
    }
    //mipc_deinit();
    return ret;
 }

