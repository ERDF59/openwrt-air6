/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  Copyright (c) [2020], MediaTek Inc. All rights reserved.
*  This software/firmware and related documentation ("MediaTek Software") are
*  protected under relevant copyright laws.
*
*  The information contained herein is confidential and proprietary to
*  MediaTek Inc. and/or its licensors. Except as otherwise provided in the
*  applicable licensing terms with MediaTek Inc. and/or its licensors, any
*  reproduction, modification, use or disclosure of MediaTek Software, and
*  information contained herein, in whole or in part, shall be strictly
*  prohibited.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <inttypes.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>

#include "platform.h"
#include "mipc_msg_host.h"
#include "mipc_internal.h"
#include "mipc_msg_tlv_api.h"
#include "mipc-wan-common.h"
#include "libdtmf_detect.h"

/***************************************************************************************************/
#define DIAL_TIMER 3
#define CONST_DIAL_ADDRESS "0980580350"
#define CONST_DIAL_ADDRESS_SECOND "0906780660"
#define CONST_DIAL_ADDRESS_TYPE 0
/*
0       MIPC_CONST.CALL_DIAL_ADDRESS_TYPE_NONE
1       MIPC_CONST.CALL_DIAL_ADDRESS_TYPE_SIP_URI
2       MIPC_CONST.CALL_DIAL_ADDRESS_TYPE_NUMBER
*/

#define CONST_SS_ACTION 2
/*
0       MIPC_CONST.CALL_SS_ACTION_RELEASE_ALL_HELD_OR_WAITING_CALL
1       MIPC_CONST.CALL_SS_ACTION_RELEASE_ALL_ACTIVE_AND_ACCEPT_CALL
2       MIPC_CONST.CALL_SS_ACTION_PLACE_ALL_ACTIVE_CALL_ON_HOLD_AND_ACCEPT_CALL
4       MIPC_CONST.CALL_SS_ACTION_EXPLICIT_CALL_AND_TRANSFER
5       MIPC_CONST.CALL_SS_ACTION_COMPLETION_CALL_BUSY_SUBSCRIBER
131     MIPC_CONST.CALL_SS_ACTION_HOLD_CALL
132     MIPC_CONST.CALL_SS_ACTION_RESUME_CALL
*/
#define CONST_SS_CALLID 1

#define CONST_HANGUP_MODE 1
/*
0       MIPC_CONST.CALL_HANGUP_MODE_HANGUP
1       MIPC_CONST.CALL_HANGUP_MODE_HANGUP_ALL
2       MIPC_CONST.CALL_HANGUP_MODE_FORCE_HANGUP
*/
#define CONST_HANGUP_CALLID 1
#define CONST_ANSWER_CALLID 1
#define CONST_CONFERENCE_ACTION 0
/*
0       MIPC_CONST.CALL_CONF_ACTION_MERGE
1       MIPC_CONST.CALL_CONF_ACTION_ADD_PARTICIPANT
2       MIPC_CONST.CALL_CONF_ACTION_REMOVE_PARTICIPANT
3       MIPC_CONST.CALL_CONF_ACTION_SPLIT
*/
// =========== COMMON PART ==============
typedef enum{
    SYNC = 0,
    ASYNC = 1,
}test_mipc_sync_enum;

#define TEST_NOMANDATORY_UINT8       (0xFF)
#define TEST_NOMANDATORY_UINT16      (0xFFFF)
#define TEST_NOMANDATORY_UINT32      (0xFFFFFFFF)

extern int g_log_hidden;

static void async_cnf_result_print(mipc_msg_t *cnf_ptr)
{
    mipc_result_enum result;
    if (NULL == cnf_ptr) return;
    result = mipc_get_result(cnf_ptr);
    LOGD("[Call Ctrl]result is:0x%08X\n", result);
}

static void sync_cnf_result_print(mipc_msg_t *cnf_ptr)
{
    mipc_result_enum result;
    if (NULL == cnf_ptr) return;
    result = mipc_get_result(cnf_ptr);
    LOGD("[Call Ctrl]result is:0x%08X\n", result);
}

static void mipc_msg_deal(mipc_msg_t *req_ptr, test_mipc_sync_enum sync, MIPC_MSG_CB cnf_cb, void *cb_priv_ptr)
{
    mipc_msg_t *cnf_ptr = NULL;

    LOGD("------------------[Call Ctrl]start mipc_msg_deal---------------------\n");
    if(SYNC == sync){
        LOGD("------------------[Call Ctrl]start SYNC---------------------\n");
        cnf_ptr = mipc_msg_sync(req_ptr);
        mipc_msg_deinit(req_ptr);
        if(NULL == cnf_cb){
            sync_cnf_result_print(cnf_ptr);
        }else{
            cnf_cb(cnf_ptr, cb_priv_ptr);
        }
        mipc_msg_deinit(cnf_ptr);
    }else if(ASYNC == sync){
        LOGD("------------------[Call Ctrl]start ASYNC---------------------\n");
        if(NULL == cnf_cb){
            mipc_msg_async(req_ptr, async_cnf_result_print, cb_priv_ptr);
        }
        else{
            mipc_msg_async(req_ptr, cnf_cb, cb_priv_ptr);
        }
        mipc_msg_deinit(req_ptr);
    }
}
// =========== COMMON PART ==============

// =========== DIAL ==============
static char g_dial_address[MIPC_MAX_DIAL_ADDRESS_LEN];
sem_t semaphore;

void dtmf_cb(void *cb_priv_ptr) {
    char result[10] = {'\0'};
    int n = snprintf(result, sizeof(result), "%c", convertToDigit(*(int *)cb_priv_ptr));
    if (n > 0) {
        strncat(g_dial_address, result, MIPC_MAX_DIAL_ADDRESS_LEN - 1);
        g_dial_address[MIPC_MAX_DIAL_ADDRESS_LEN - 1] = '\0';
        if (!g_log_hidden) {
            printf("dtmf_cb result: %s\n", result);
            LOGD("dtmf_cb result: %s\n", result);
        } else {
            printf("dtmf_cb result: %s\n", "x");
            LOGD("dtmf_cb result: %s\n", "x");
        }
    }
    if (!g_log_hidden) {
        printf("g_dial_address: %s\n", g_dial_address);
        LOGD("g_dial_address: %s\n", g_dial_address);
    }
    alarm(DIAL_TIMER);
}

typedef struct{
    int8_t      dial_address[MIPC_MAX_DIAL_ADDRESS_LEN];
    uint32_t    dial_address_type;
    uint32_t    type;
    uint32_t    domain;
    uint32_t    ecc_retry_domain;
    uint16_t    ecc_category;
    uint8_t     clir;
}mipc_dial_req_tlv_list_struct;

static void set_dial_req_tlv_list_nomandatory(mipc_dial_req_tlv_list_struct *tlv_list_prt)
{
    memset(tlv_list_prt->dial_address, 0, sizeof(tlv_list_prt->dial_address));
    tlv_list_prt->dial_address_type = TEST_NOMANDATORY_UINT32;
    tlv_list_prt->type = TEST_NOMANDATORY_UINT32;
    tlv_list_prt->domain = TEST_NOMANDATORY_UINT32;
    tlv_list_prt->ecc_retry_domain = TEST_NOMANDATORY_UINT32;
    tlv_list_prt->ecc_category = TEST_NOMANDATORY_UINT16;
    tlv_list_prt->clir = TEST_NOMANDATORY_UINT8;
}
static void dial_req_test(const mipc_dial_req_tlv_list_struct *tlv_list_prt, test_mipc_sync_enum sync, MIPC_MSG_CB cnf_cb, void *cb_priv_ptr)
{
    mipc_msg_t *req_ptr = NULL;

    LOGD("------------------[Call Ctrl]start dial_req_test---------------------\n");
    if(0 == tlv_list_prt->dial_address[0] || TEST_NOMANDATORY_UINT32 == tlv_list_prt->dial_address_type) {
        LOGD("dial_req_test: missing mandatory para\n");
        return ;
    }

    req_ptr = mipc_msg_init(MIPC_CALL_DIAL_REQ, MIPC_MSG_PS0);
    mipc_call_dial_req_add_dial_address(req_ptr, strlen((const char*) tlv_list_prt->dial_address), (char*)tlv_list_prt->dial_address);
    mipc_call_dial_req_add_dial_address_type(req_ptr, tlv_list_prt->dial_address_type);
    if(TEST_NOMANDATORY_UINT32 != tlv_list_prt->type){
        mipc_call_dial_req_add_type(req_ptr, tlv_list_prt->type);
    }
    if(TEST_NOMANDATORY_UINT32 != tlv_list_prt->domain){
        mipc_call_dial_req_add_domain(req_ptr, tlv_list_prt->domain);
    }
    if(TEST_NOMANDATORY_UINT32 != tlv_list_prt->ecc_retry_domain){
        mipc_call_dial_req_add_ecc_retry_domain(req_ptr, tlv_list_prt->ecc_retry_domain);
    }
    if(TEST_NOMANDATORY_UINT16 != tlv_list_prt->ecc_category){
        mipc_call_dial_req_add_ecc_category(req_ptr, tlv_list_prt->ecc_category);
    }
    if(TEST_NOMANDATORY_UINT8 != tlv_list_prt->clir){
        mipc_call_dial_req_add_clir(req_ptr, tlv_list_prt->clir);
    }

    mipc_msg_deal(req_ptr, sync, cnf_cb, cb_priv_ptr);
}

void test_mipc_dial_req_hdlr(int order , char* param)
{
    mipc_dial_req_tlv_list_struct tlv_list = {0};

    set_dial_req_tlv_list_nomandatory(&tlv_list);
    LOGD("------------------[Call Ctrl]Start test_mipc_dial_req_hdlr---------------------\n");
    LOGD("------------------[Call Ctrl]please input dial number ot string\n");
    //scanf("%s", tlv_list.dial_address);
    if (param != NULL) {
        if (!g_log_hidden) {
            LOGD("------------------[Call Ctrl]dial number: %s\n", param);
            printf("------------------[Call Ctrl]dial number: %s\n", param);
        }
        strncpy((char*)tlv_list.dial_address, param, MIPC_MAX_DIAL_ADDRESS_LEN - 1);
        tlv_list.dial_address[MIPC_MAX_DIAL_ADDRESS_LEN - 1] = '\0';
    } else if (order == 1) {
        strncpy((char*)tlv_list.dial_address, CONST_DIAL_ADDRESS, strlen(CONST_DIAL_ADDRESS) + 1);
    } else {
        strncpy((char*)tlv_list.dial_address, CONST_DIAL_ADDRESS_SECOND,
                strlen(CONST_DIAL_ADDRESS_SECOND) + 1);
    }
    LOGD("------------------[Call Ctrl]please input dial address type refer to CALL_DIAL_ADDRESS_TYPE\n");
    //scanf("%d", &tlv_list.dial_address_type);
    tlv_list.dial_address_type = CONST_DIAL_ADDRESS_TYPE;

    if (!g_log_hidden) {
        LOGD("------------------[Call Ctrl]final dial number: %s\n", (char*)tlv_list.dial_address);
        printf("------------------[Call Ctrl]final dial number: %s\n", (char*)tlv_list.dial_address);
    } else {
        LOGD("------------------[Call Ctrl]final dial number: %s\n", "xxxxxx");
        printf("------------------[Call Ctrl]final dial number: %s\n", "xxxxxx");
    }
    dial_req_test(&tlv_list, SYNC, NULL, NULL);
    sem_post(&semaphore);
}

void test_mipc_dial_req_hdlr2()
{
    printf("------------------[Call Ctrl]Start test_mipc_dial_req_hdlr2---------------------\n");
    libdtmf_detect_listen(0, NULL);
    mipc_init("mipc_wan_cli");
    mipc_dial_req_tlv_list_struct tlv_list = {0};

    set_dial_req_tlv_list_nomandatory(&tlv_list);
    LOGD("------------------[Call Ctrl]Start test_mipc_dial_req_hdlr---------------------\n");
    LOGD("------------------[Call Ctrl]please input dial number ot string\n");
    //scanf("%s", tlv_list.dial_address);
    strncpy((char*)tlv_list.dial_address, g_dial_address, MIPC_MAX_DIAL_ADDRESS_LEN - 1);
    tlv_list.dial_address[MIPC_MAX_DIAL_ADDRESS_LEN - 1] = '\0';
    LOGD("------------------[Call Ctrl]please input dial address type refer to CALL_DIAL_ADDRESS_TYPE\n");
    //scanf("%d", &tlv_list.dial_address_type);
    tlv_list.dial_address_type = CONST_DIAL_ADDRESS_TYPE;

    if (!g_log_hidden) {
        LOGD("------------------[Call Ctrl]final dial number: %s\n", (char*)tlv_list.dial_address);
        printf("------------------[Call Ctrl]final dial number: %s\n", (char*)tlv_list.dial_address);
    } else {
        LOGD("------------------[Call Ctrl]final dial number: %s\n", "xxxxxx");
        printf("------------------[Call Ctrl]final dial number: %s\n", "xxxxxx");
    }
    dial_req_test(&tlv_list, SYNC, NULL, NULL);
}

void dtmf_detect_done()
{
    printf("------------------DTMF detect done---------------------\n");
    libdtmf_detect_listen(0, NULL);
}

void timeout_handler(int signal)
{
    sem_post(&semaphore);
}

void start_listen_dtmf(int mode) {
    sem_init(&semaphore, 0, 0);
    memset(g_dial_address, 0, sizeof(g_dial_address));
    libdtmf_detect_listen(1, dtmf_cb);
    if (signal(SIGALRM, timeout_handler) == SIG_ERR) {
        printf("------------------signal failed---------------------\n");
        LOGD("------------------signal failed---------------------\n");
        return;
    }
    alarm(DIAL_TIMER);
    sem_wait(&semaphore);
    if (mode == 1) {
        test_mipc_dial_req_hdlr2();
    } else {
        dtmf_detect_done();
    }
}
// =========== DIAL ==============
// =========== SS ==============
static void ss_req_test(const uint32_t action, const uint32_t callid, test_mipc_sync_enum sync, MIPC_MSG_CB cnf_cb, void *cb_priv_ptr)
{
    mipc_msg_t *req_ptr = NULL;

    LOGD("------------------[Call Ctrl]start ss_req_test---------------------\n");
    if(TEST_NOMANDATORY_UINT32 == action || TEST_NOMANDATORY_UINT32 == callid) {
        LOGD("ss_req_test: missing mandatory para\n");
        return ;
    }

    req_ptr = mipc_msg_init(MIPC_CALL_SS_REQ, MIPC_MSG_PS0);
    mipc_call_ss_req_add_action(req_ptr, action);
    mipc_call_ss_req_add_callid(req_ptr, callid);

    mipc_msg_deal(req_ptr, sync, cnf_cb, cb_priv_ptr);
}

void test_mipc_ss_req_hdlr(int mode)
{
    uint32_t action;
    uint32_t callid;

    LOGD("------------------[Call Ctrl]Start test_mipc_ss_req_hdlr---------------------\n");
    LOGD("------------------[Call Ctrl]please input ss req action\n");
    //scanf("%d", &action);
    action = mode;
    LOGD("------------------[Call Ctrl]please input ss req callid\n");
    //scanf("%d", &callid);
    callid = CONST_SS_CALLID;

    ss_req_test(action, callid, SYNC, NULL, NULL);
}
// =========== SS ==============
// =========== Hangup ==============
typedef struct{
    uint32_t    mode;
    uint32_t    callid;
    uint32_t    cause;
}mipc_hangup_req_tlv_list_struct;

static void set_hangup_req_tlv_list_nomandatory(mipc_hangup_req_tlv_list_struct *tlv_list_prt)
{
    tlv_list_prt->mode = TEST_NOMANDATORY_UINT32;
    tlv_list_prt->callid = TEST_NOMANDATORY_UINT32;
    tlv_list_prt->cause = TEST_NOMANDATORY_UINT32;
}

static void hangup_req_test(const mipc_hangup_req_tlv_list_struct *tlv_list_prt, test_mipc_sync_enum sync, MIPC_MSG_CB cnf_cb, void *cb_priv_ptr)
{
    mipc_msg_t *req_ptr = NULL;

    LOGD("------------------[Call Ctrl]start hangup_req_test---------------------\n");
    if(TEST_NOMANDATORY_UINT32 == tlv_list_prt->mode || TEST_NOMANDATORY_UINT32 == tlv_list_prt->callid) {
        LOGD("hangup_req_test: missing mandatory para\n");
        return ;
    }

    req_ptr = mipc_msg_init(MIPC_CALL_HANGUP_REQ, MIPC_MSG_PS0);
    mipc_call_hangup_req_add_mode(req_ptr, tlv_list_prt->mode);
    mipc_call_hangup_req_add_callid(req_ptr, tlv_list_prt->callid);

    if(TEST_NOMANDATORY_UINT32 != tlv_list_prt->cause){
        mipc_call_dial_req_add_type(req_ptr, tlv_list_prt->cause);
    }

    mipc_msg_deal(req_ptr, sync, cnf_cb, cb_priv_ptr);

}

void test_mipc_hangup_req_hdlr(void)
{
    mipc_hangup_req_tlv_list_struct tlv_list = {0};

    set_hangup_req_tlv_list_nomandatory(&tlv_list);
    LOGD("------------------[Call Ctrl]Start test_mipc_hangup_req_hdlr---------------------\n");
    LOGD("------------------[Call Ctrl]please input hangup mode\n");
    //scanf("%d", &tlv_list.mode);
    tlv_list.mode = CONST_HANGUP_MODE;
    LOGD("------------------[Call Ctrl]please input hangup callid\n");
    //scanf("%d", &tlv_list.callid);
    tlv_list.callid = CONST_HANGUP_CALLID;

    hangup_req_test(&tlv_list, SYNC, NULL, NULL);
}
// =========== Hangup ==============


// =========== Answer ==============
typedef struct{
    uint32_t    mode;
    uint32_t    callid;
}mipc_answer_req_tlv_list_struct;

static void set_answer_req_tlv_list_nomandatory(mipc_answer_req_tlv_list_struct *tlv_list_prt)
{
    tlv_list_prt->mode = TEST_NOMANDATORY_UINT32;
    tlv_list_prt->callid = TEST_NOMANDATORY_UINT32;

}

static void answer_req_test(const mipc_answer_req_tlv_list_struct *tlv_list_prt, test_mipc_sync_enum sync, MIPC_MSG_CB cnf_cb, void *cb_priv_ptr)
{
    mipc_msg_t *req_ptr = NULL;

    LOGD("------------------[Call Ctrl]start answer_req_test---------------------\n");
    if(TEST_NOMANDATORY_UINT32 == tlv_list_prt->callid) {
        LOGD("missing mandatory para\n");
        return ;
    }

    req_ptr = mipc_msg_init(MIPC_CALL_ANSWER_REQ, MIPC_MSG_PS0);
    mipc_call_answer_req_add_callid(req_ptr, tlv_list_prt->callid);
    if(TEST_NOMANDATORY_UINT32 != tlv_list_prt->mode){
        mipc_call_answer_req_add_mode(req_ptr, tlv_list_prt->mode);
    }

    mipc_msg_deal(req_ptr, sync, cnf_cb, cb_priv_ptr);
}

void test_mipc_answer_req_hdlr(void)
{
    mipc_answer_req_tlv_list_struct tlv_list = {0};

    set_answer_req_tlv_list_nomandatory(&tlv_list);
    LOGD("------------------[Call Ctrl]Start test_mipc_answer_req_hdlr---------------------\n");
    LOGD("------------------[Call Ctrl]please input answer callid\n");
    //scanf("%d", &tlv_list.callid);
    tlv_list.callid = CONST_ANSWER_CALLID;

    answer_req_test(&tlv_list, SYNC, NULL, NULL);
}
// =========== Answer ==============


// =========== Merge ==============
typedef struct{
    uint32_t    conf_callid;
    uint32_t    action;
    int8_t      number[MIPC_MAX_CALL_NUMBER_LEN];
    uint32_t    target_callid;
}mipc_conference_req_tlv_list_struct;

static void set_conference_req_tlv_list_nomandatory(mipc_conference_req_tlv_list_struct *tlv_list_prt)
{
    tlv_list_prt->conf_callid= TEST_NOMANDATORY_UINT32;
    tlv_list_prt->action= TEST_NOMANDATORY_UINT32;
    memset(tlv_list_prt->number, 0, sizeof(tlv_list_prt->number));
    tlv_list_prt->target_callid= TEST_NOMANDATORY_UINT32;
}
static void conference_req_test(const mipc_conference_req_tlv_list_struct *tlv_list_prt, test_mipc_sync_enum sync, MIPC_MSG_CB cnf_cb, void *cb_priv_ptr)
{
    mipc_msg_t *req_ptr = NULL;

    LOGD("------------------[Call Ctrl]start conference_req_test---------------------\n");
    if(TEST_NOMANDATORY_UINT32 == tlv_list_prt->action) {
        LOGD("missing mandatory para\n");
        return ;
    }

    req_ptr = mipc_msg_init(MIPC_CALL_CONFERENCE_REQ, MIPC_MSG_PS0);
    mipc_call_conference_req_add_action(req_ptr, tlv_list_prt->action);
    if(TEST_NOMANDATORY_UINT32 != tlv_list_prt->conf_callid){
        mipc_call_conference_req_add_conf_callid(req_ptr, tlv_list_prt->conf_callid);
    }
    if(0 != tlv_list_prt->number[0]){
        mipc_call_conference_req_add_number(req_ptr, strlen((const char*)tlv_list_prt->number), (char*)tlv_list_prt->number);
    }
    if(TEST_NOMANDATORY_UINT32 != tlv_list_prt->target_callid){
        mipc_call_conference_req_add_target_callid(req_ptr, tlv_list_prt->target_callid);
    }

    mipc_msg_deal(req_ptr, sync, cnf_cb, cb_priv_ptr);
}

void test_mipc_conference_req_hdlr(void)
{
    mipc_conference_req_tlv_list_struct tlv_list = {0};

    set_conference_req_tlv_list_nomandatory(&tlv_list);
    LOGD("------------------[Call Ctrl]Start test_mipc_conference_req_hdlr---------------------\n");
    LOGD("------------------[Call Ctrl]please input action\n");
    //scanf("%d", &tlv_list.action);
    tlv_list.action = CONST_CONFERENCE_ACTION;

    conference_req_test(&tlv_list, SYNC, NULL, NULL);
}
// =========== Merge ==============
// =========== Get Call Status ==============
static inline uint32_t mipc_call_get_call_status_cnf_get_callid_idx(mipc_msg_t *msg_ptr, uint16_t idx, uint32_t def_val)
{
    if (!msg_ptr) return def_val;
    return (uint32_t)mipc_msg_get_idx_uint32(msg_ptr, MIPC_CALL_GET_CALL_STATUS_CNF_T_CALLID, idx, def_val);
}

static inline mipc_call_clcc_state_const_enum mipc_call_get_call_status_cnf_get_call_clcc_state_idx(mipc_msg_t *msg_ptr, uint16_t idx, mipc_call_clcc_state_const_enum def_val)
{
    if (!msg_ptr) return def_val;
    return (mipc_call_clcc_state_const_enum)mipc_msg_get_idx_uint32(msg_ptr, MIPC_CALL_GET_CALL_STATUS_CNF_T_CALL_CLCC_STATE, idx, def_val);
}

static void get_call_status_cnf_print(mipc_msg_t *cnf_ptr, void* cb_priv)
{
    mipc_result_enum result;
    uint32_t count = 0;
    uint32_t i = 0;

    LOGD("------------------[Call Ctrl]start get_call_status_cnf_print---------------------\n");
    if (NULL == cnf_ptr) return;
    result = mipc_get_result(cnf_ptr);
    printf("result:%d,", result);
    LOGD("result:%d,", result);
    count = mipc_call_get_call_status_cnf_get_count(cnf_ptr, TEST_NOMANDATORY_UINT32);
    printf("count:%d     ", count);
    LOGD("count:%d     ", count);
    for(i = 0; i < count; ++i){
        printf("call_id[%d]:%d,", i, mipc_call_get_call_status_cnf_get_callid_idx(cnf_ptr, i, TEST_NOMANDATORY_UINT32));
        LOGD("call_id[%d]:%d,", i, mipc_call_get_call_status_cnf_get_callid_idx(cnf_ptr, i, TEST_NOMANDATORY_UINT32));
        printf("call_state[%d]:%d,", i, mipc_call_get_call_status_cnf_get_call_clcc_state_idx(cnf_ptr, i, mipc_call_clcc_state_const_NONE));
        LOGD("call_state[%d]:%d,", i, mipc_call_get_call_status_cnf_get_call_clcc_state_idx(cnf_ptr, i, mipc_call_clcc_state_const_NONE));
    }
}

static void get_call_status_req_test(const uint32_t callid, test_mipc_sync_enum sync, MIPC_MSG_CB cnf_cb, void *cb_priv_ptr)
{
    mipc_msg_t *req_ptr = NULL;

    LOGD("------------------[Call Ctrl]start get_call_status_req_test---------------------\n");
    req_ptr = mipc_msg_init(MIPC_CALL_GET_CALL_STATUS_REQ, MIPC_MSG_PS0);
    if(TEST_NOMANDATORY_UINT32 != callid){
        mipc_call_get_call_status_req_add_callid(req_ptr, callid);
    }

    mipc_msg_deal(req_ptr, sync, cnf_cb, cb_priv_ptr);
}

void test_mipc_get_call_status_req_hdlr(void)
{
    uint32_t callid = TEST_NOMANDATORY_UINT32;

    LOGD("------------------[Call Ctrl]Start test_mipc_get_call_status_req_hdlr---------------------\n");

    get_call_status_req_test(callid, SYNC, get_call_status_cnf_print, NULL);
}
// =========== Get Call Status ==============
