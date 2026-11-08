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

#include "platform.h"
#include "mipc_hashmap.h"
#include "mipc_msg.h"
#include "mipc_msg_host.h"
#include "mipc_internal.h"

#ifdef ENABLE_TRM
#include "mipc_trm.h"
#endif

#define RECV_BUF_LEN (65000)
static unsigned char *recv_buf_ptr;
static uint32_t inited = 0;
static mipc_hashmap_t *transaction_hashmap_ptr;
static mipc_hashmap_t *ind_hashmap_ptr;
static mipc_hashmap_t *cmd_hashmap_ptr;
static MUTEX *mutex_ptr;
static MUTEX *write_mutex_ptr; //for write access
static THREAD *thread_ptr = 0;

static uint32_t default_timeout_ms = 300000;

static mipc_hashmap_t *tls_timeout_hashmap_ptr;
static MUTEX *tls_mutex_ptr;

#ifdef UNIT_TEST
static mipc_msg_t *unit_test_set_msg_ptr = NULL;
void UNIT_TEST_SET_MSG(mipc_msg_t *msg_ptr)
{
    unit_test_set_msg_ptr = msg_ptr;
}

void UNIT_TEST_SEND_MSG(mipc_msg_t *msg_ptr)
{
    uint8_t *buf_ptr;
    uint16_t buf_len;

    buf_ptr = mipc_msg_serialize(msg_ptr, &buf_len);

    UNIT_TEST_READ_BUF_ADD(buf_ptr, buf_len);

    FREE(buf_ptr);
}
#endif

#if 0
static void dump_tlv_cb(mipc_msg_tlv_t *tlv_ptr, void *cb_priv_ptr)
{
    uint8_t *p = (uint8_t *)MIPC_MSG_GET_TLV_VAL_PTR(tlv_ptr);
    uint16_t i;

    InfoMsg("t=0x%04x l=%d", tlv_ptr->type, tlv_ptr->len);
    for (i = 0; i < tlv_ptr->len;) {
        if ((tlv_ptr->len - i) >= 16) {
            Info2Msg("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", p[i], p[i + 1], p[i + 2], p[i + 3], p[i + 4], p[i + 5], p[i + 6], p[i + 7], p[i + 8], p[i + 9], p[i + 10], p[i + 11], p[i + 12], p[i + 13], p[i + 14], p[i + 15]);
            i += 16;
        } else {
            switch (tlv_ptr->len - i) {
            case 1:
                Info2Msg("%02X", p[i]);
                break;
            case 2:
                Info2Msg("%02X %02X", p[i], p[i + 1]);
                break;
            case 3:
                Info2Msg("%02X %02X %02X", p[i], p[i + 1], p[i + 2]);
                break;
            case 4:
                Info2Msg("%02X %02X %02X %02X", p[i], p[i + 1], p[i + 2], p[i + 3]);
                break;
            case 5:
                Info2Msg("%02X %02X %02X %02X %02X", p[i], p[i + 1], p[i + 2], p[i + 3], p[i + 4]);
                break;
            case 6:
                Info2Msg("%02X %02X %02X %02X %02X %02X", p[i], p[i + 1], p[i + 2], p[i + 3], p[i + 4], p[i + 5]);
                break;
            case 7:
                Info2Msg("%02X %02X %02X %02X %02X %02X %02X", p[i], p[i + 1], p[i + 2], p[i + 3], p[i + 4], p[i + 5], p[i + 6]);
                break;
            case 8:
                Info2Msg("%02X %02X %02X %02X %02X %02X %02X %02X", p[i], p[i + 1], p[i + 2], p[i + 3], p[i + 4], p[i + 5], p[i + 6], p[i + 7]);
                break;
            case 9:
                Info2Msg("%02X %02X %02X %02X %02X %02X %02X %02X %02X", p[i], p[i + 1], p[i + 2], p[i + 3], p[i + 4], p[i + 5], p[i + 6], p[i + 7], p[i + 8]);
                break;
            case 10:
                Info2Msg("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", p[i], p[i + 1], p[i + 2], p[i + 3], p[i + 4], p[i + 5], p[i + 6], p[i + 7], p[i + 8], p[i + 9]);
                break;
            case 11:
                Info2Msg("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", p[i], p[i + 1], p[i + 2], p[i + 3], p[i + 4], p[i + 5], p[i + 6], p[i + 7], p[i + 8], p[i + 9], p[i + 10]);
                break;
            case 12:
                Info2Msg("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", p[i], p[i + 1], p[i + 2], p[i + 3], p[i + 4], p[i + 5], p[i + 6], p[i + 7], p[i + 8], p[i + 9], p[i + 10], p[i + 11]);
                break;
            case 13:
                Info2Msg("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", p[i], p[i + 1], p[i + 2], p[i + 3], p[i + 4], p[i + 5], p[i + 6], p[i + 7], p[i + 8], p[i + 9], p[i + 10], p[i + 11], p[i + 12]);
                break;
            case 14:
                Info2Msg("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", p[i], p[i + 1], p[i + 2], p[i + 3], p[i + 4], p[i + 5], p[i + 6], p[i + 7], p[i + 8], p[i + 9], p[i + 10], p[i + 11], p[i + 12], p[i + 13]);
                break;
            case 15:
                Info2Msg("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", p[i], p[i + 1], p[i + 2], p[i + 3], p[i + 4], p[i + 5], p[i + 6], p[i + 7], p[i + 8], p[i + 9], p[i + 10], p[i + 11], p[i + 12], p[i + 13], p[i + 14]);
                break;
            }
            break;
        }
    }
}
#endif

#ifdef WAKEUP_ALL_MIPC_CALLBACK
void mipc_hashmap_wakeup_all_cb(mipc_hashmap_element_t *hashmap_element_ptr, void *cb_priv_ptr)
{
    mipc_transaction_t tx;

    MEMCPY(&tx, MIPC_HASHMAP_GET_VAL_PTR(hashmap_element_ptr), sizeof(mipc_transaction_t));
    switch (tx.transaction_type) {
    case MIPC_TRANSACTION_TYPE_SYNC:
        WAKE_EVENT(tx.event_ptr, 0);
        break;
    case MIPC_TRANSACTION_TYPE_ASYNC:
        if (tx.msg_cb) {
            DbgMsg("async CNF callback start");
            tx.msg_cb(0, tx.cb_priv_ptr);
            DbgMsg("async CNF callback end");
        }
        break;
    }
}
#endif

static int8_t mipc_ind_t_compare_without_key(mipc_ind_t *a, mipc_ind_t *b)
{
    if(a->msg_cb != b->msg_cb || a->cb_priv_ptr != b->cb_priv_ptr) {
        return -1;
    }
    return 0;
}

static uint32_t rx_thread(void *priv_ptr)
{
    mipc_msg_t *msg_ptr;
    mipc_msg_hdr_t *msg_hdr_ptr;
    mipc_hashmap_element_t *hashmap_element_ptr;
    uint16_t recv_buf_ptr_len;
    uint16_t total_read_bytes;
    uint16_t msg_id;
    uint16_t msg_txid;
    uint32_t key;

    for (;;) {
        memset(recv_buf_ptr, 0, RECV_BUF_LEN);

        DbgMsg("Start to READCOM");

        //read header
        total_read_bytes = READCOM(recv_buf_ptr, sizeof(mipc_msg_hdr_t));
#ifdef UNIT_TEST
        if (total_read_bytes == 0) continue;
#endif
        if (total_read_bytes != sizeof(mipc_msg_hdr_t)) {
            ErrMsg("read error, rx_thread stop");
#ifdef GIGAUT
            break;
#else
            return -1;
#endif
        }

        msg_hdr_ptr = (mipc_msg_hdr_t*)&recv_buf_ptr[0];
        if (msg_hdr_ptr->msg_len) {
            recv_buf_ptr_len = total_read_bytes + msg_hdr_ptr->msg_len;
            //read payload
            total_read_bytes += READCOM(recv_buf_ptr + total_read_bytes, recv_buf_ptr_len - total_read_bytes);
#ifdef UNIT_TEST
            if (total_read_bytes == 0) continue;
#endif
        } else {
            recv_buf_ptr_len = total_read_bytes;
        }
        if (total_read_bytes != recv_buf_ptr_len) {
            ErrMsg("read error, rx_thread stop");
#ifdef GIGAUT
            break;
#else
            return -1;
#endif
        }
        if ((msg_ptr = mipc_msg_deserialize(recv_buf_ptr, recv_buf_ptr_len)) == NULL) {
            //error handling
            ErrMsg("MSG deserialize failed:");
            WRITECAP(recv_buf_ptr_len, recv_buf_ptr, MIPC_DEVICE_TO_HOST);
            continue;
        }

        msg_id = MTOHS(msg_ptr->hdr.msg_id);
        msg_txid = MTOHS(msg_ptr->hdr.msg_txid);

        //dump recv message
        WRITECAP(recv_buf_ptr_len, recv_buf_ptr, MIPC_DEVICE_TO_HOST);

        switch (msg_id & MIPC_MSG_TYPE_MASK) {
        case MIPC_MSG_TYPE_REQ_CNF: {//receive CNF
            LOCK_MUTEX(mutex_ptr);
            if ((hashmap_element_ptr = mipc_hashmap_get(transaction_hashmap_ptr, &msg_txid, sizeof(msg_txid)))) {
                mipc_transaction_t tx;
                //copy first and free hashmap_element_ptr
                MEMCPY(&tx, MIPC_HASHMAP_GET_VAL_PTR(hashmap_element_ptr), sizeof(mipc_transaction_t));
                mipc_hashmap_del(hashmap_element_ptr);
                UNLOCK_MUTEX(mutex_ptr);

                switch (tx.transaction_type) {
                case MIPC_TRANSACTION_TYPE_SYNC:
                    WAKE_EVENT(tx.event_ptr, msg_ptr);
                    break;
                case MIPC_TRANSACTION_TYPE_ASYNC:
                    if (tx.msg_cb) {
                        DbgMsg("async CNF callback start");
                        tx.msg_cb(msg_ptr, tx.cb_priv_ptr);
                        DbgMsg("async CNF callback end");
                    } else {
                        DbgMsg("async no CNF callback");
                    }
                    //free
                    mipc_msg_deinit(msg_ptr);
                    break;
                }
            } else {
                //NOTICE
                UNLOCK_MUTEX(mutex_ptr);

                //no found, just free
                mipc_msg_deinit(msg_ptr);
            }
            break;
        }
        case MIPC_MSG_TYPE_IND:  {//receive IND
            int32_t i;
            uint8_t sim_ps_id = msg_ptr->hdr.msg_sim_ps_id;
            mipc_ind_t tmp_ind = {0};
            for (i = 0; i < 8; i++) {
                if (sim_ps_id & (1 << i)) {
                    key = ((1 << i) << 16) | msg_id;
                    LOCK_MUTEX(mutex_ptr);
                    if ((hashmap_element_ptr = mipc_hashmap_get(ind_hashmap_ptr, &key, sizeof(key)))) {
                        mipc_ind_t *ind_ptr;
                        msg_ptr->hdr.msg_sim_ps_id = (1 << i);
                        //NOTICE
                        UNLOCK_MUTEX(mutex_ptr);

                        ind_ptr = (mipc_ind_t *)MIPC_HASHMAP_GET_VAL_PTR(hashmap_element_ptr);
                        //ind_ptr will not be NULL if hashmap_element_ptr is not NULL
                        //whether hashmap_element_ptr is NULL had been checked in the previous if condition
                        //to avoid both sim&ps reg called twice(compare without key)
                        if(0 == mipc_ind_t_compare_without_key(ind_ptr, &tmp_ind)) {
                            DbgMsg("found a DUP cb");
                            continue;
                        }
                        if (ind_ptr->msg_cb) {
                            DbgMsg("IND callback start");
                            ind_ptr->msg_cb(msg_ptr, ind_ptr->cb_priv_ptr);
                            DbgMsg("IND callback end");
                        } else {
                            DbgMsg("IND no callback");
                        }
                        MEMCPY(&tmp_ind, (void*)ind_ptr, sizeof(mipc_ind_t));
                    } else {
                        //NOTICE
                        UNLOCK_MUTEX(mutex_ptr);
                    }
                }
            }
            if (msg_ptr) {
                mipc_msg_deinit(msg_ptr);
                msg_ptr = NULL;
            }
            break;
        }
        case MIPC_MSG_TYPE_CMD_RSP: {//receive CMD
            key = msg_id;
            LOCK_MUTEX(mutex_ptr);
            if ((hashmap_element_ptr = mipc_hashmap_get(cmd_hashmap_ptr, &key, sizeof(key)))) {
                mipc_cmd_t *cmd_ptr;

                //NOTICE
                UNLOCK_MUTEX(mutex_ptr);

                cmd_ptr = (mipc_cmd_t *)MIPC_HASHMAP_GET_VAL_PTR(hashmap_element_ptr);
                //cmd_ptr will not be NULL if hashmap_element_ptr is not NULL
                //whether hashmap_element_ptr is NULL had been checked in the previous if condition

                if (cmd_ptr->msg_cb) {
                    DbgMsg("CMD callback start");
                    cmd_ptr->msg_cb(msg_ptr, cmd_ptr->cb_priv_ptr);
                    DbgMsg("CMD callback end");
                } else{
                    DbgMsg("CMD no callback");
                }
            } else {
                //NOTICE
                UNLOCK_MUTEX(mutex_ptr);
            }
            //always free
            mipc_msg_deinit(msg_ptr);
            break;
        }
        default:
            //free
            mipc_msg_deinit(msg_ptr);
            break;
        }
        //port will be closed
        if (msg_id == MIPC_INTERNAL_CLOSE_CNF) {
            break;
        }
    }

#ifdef WAKEUP_ALL_MIPC_CALLBACK
    mipc_hashmap_iterate(transaction_hashmap_ptr, mipc_hashmap_wakeup_all_cb, NULL);
#endif
    return 0;
}

int32_t mipc_init_debug(const char *process_name_ptr)
{
    if (process_name_ptr == NULL) {
        return -1;
    }
    OPENCAP(process_name_ptr);
    return mipc_init(process_name_ptr);
}


int32_t mipc_init(const char *process_name_ptr)
{
    static int32_t singleton = 0;
#ifndef UNIT_TEST
    mipc_msg_t* msg_req_ptr;
    mipc_msg_t* msg_cnf_ptr;
    mipc_msg_t* msg_test_req_ptr;
    mipc_msg_t* msg_test_cnf_ptr;
#endif

    if (process_name_ptr == NULL) {
        return -1;
    }

#ifdef GIGAUT
    {
        char com_path[128] = {0};
        snprintf(com_path, sizeof(com_path)-1, "%s", process_name_ptr);
        SETCOM(com_path);
    }
#endif

    if (OPENCOM(FINDCOM(process_name_ptr)) < 0) {
        return -2;
    }

    //only allocate once
    if (singleton == 0) {
        INIT();
        recv_buf_ptr = ALLOC(RECV_BUF_LEN);
        if (recv_buf_ptr == NULL) {
            return -3;
        }
        transaction_hashmap_ptr = mipc_hashmap_init(32, NULL);
        ind_hashmap_ptr = mipc_hashmap_init(32, NULL);
        cmd_hashmap_ptr = mipc_hashmap_init(32, NULL);
        mutex_ptr = CREATE_MUTEX();
        write_mutex_ptr = CREATE_MUTEX();
        //
        tls_timeout_hashmap_ptr = mipc_hashmap_init(32, NULL);
        tls_mutex_ptr = CREATE_MUTEX();
        mipc_create_txid_mutex();

        singleton = 1;
        inited = 1;
    }

#ifdef ENABLE_TRM
    //init libtrm
    libtrm_init();
#endif

    //create rx thread
    if (thread_ptr){
        ErrMsg("create thread failed");
        return -1;
    }else{
        thread_ptr = CREATE_THREAD(rx_thread, NULL);
    }

#ifndef UNIT_TEST
    //sync to modem
    //never change MIPC_MSG_ALL to other value
    msg_test_req_ptr = mipc_msg_init(MIPC_INTERNAL_TEST_REQ, MIPC_MSG_ALL);
    mipc_ret_cause_enum causeValue = ERR_NO_ERR;
    mipc_msg_set_timeout_once(100);
    causeValue = mipc_msg_sync_timeout_with_cause(msg_test_req_ptr, &msg_test_cnf_ptr);
    while (causeValue != ERR_NO_ERR) {
        ErrMsg("MIPC_INTERNAL_TEST_REQ sync fail %d", causeValue);
        mipc_msg_set_timeout_once(100);
        causeValue = mipc_msg_sync_timeout_with_cause(msg_test_req_ptr, &msg_test_cnf_ptr);
    }
    mipc_msg_deinit(msg_test_req_ptr);
    if (msg_test_cnf_ptr) {
        mipc_msg_deinit(msg_test_cnf_ptr);
    }
    msg_req_ptr = mipc_msg_init(MIPC_INTERNAL_OPEN_REQ, MIPC_MSG_ALL);
    mipc_msg_add_tlv_uint32(msg_req_ptr, MIPC_INTERNAL_OPEN_REQ_T_VERSION, MIPC_VERSION);
    mipc_msg_add_tlv(msg_req_ptr, MIPC_INTERNAL_OPEN_REQ_T_CLIENT_NAME, strlen(process_name_ptr) + 1, process_name_ptr);
    mipc_msg_add_tlv_uint8(msg_req_ptr, MIPC_INTERNAL_OPEN_REQ_T_USIR_SUPPORT, MIPC_BOOLEAN_TRUE);
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    if (msg_cnf_ptr) {
        //get version
        DbgMsg("Target version: %x", mipc_msg_get_val_uint32(msg_cnf_ptr, MIPC_INTERNAL_OPEN_CNF_T_VERSION, 0xffffffff));
        //get timeout (default is 5 minutes)
        DbgMsg("Default timeout: %u ms", mipc_msg_get_val_uint32(msg_cnf_ptr, MIPC_INTERNAL_OPEN_CNF_T_TIMEOUT, 300000));
        mipc_msg_deinit(msg_cnf_ptr);
    } else {
        return -1;
    }
#endif

    return 0;
}

void mipc_deinit()
{
    mipc_msg_t* msg_req_ptr;
    mipc_msg_t* msg_cnf_ptr;

    if (inited == 0) return;

#ifdef ENABLE_TRM
    //deinit libtrm
    libtrm_deinit();
#endif

    //sync to modem
    msg_req_ptr = mipc_msg_init(MIPC_INTERNAL_CLOSE_REQ, MIPC_MSG_ALL);
    mipc_msg_set_timeout_once(2000);
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);
    mipc_msg_deinit(msg_cnf_ptr);

    //close com port
    CLOSECOM();

    //clean all hashmap
    LOCK_MUTEX(mutex_ptr);
    mipc_hashmap_clean(transaction_hashmap_ptr);
    mipc_hashmap_clean(ind_hashmap_ptr);
    mipc_hashmap_clean(cmd_hashmap_ptr);
    UNLOCK_MUTEX(mutex_ptr);

    LOCK_MUTEX(tls_mutex_ptr);
    mipc_hashmap_clean(tls_timeout_hashmap_ptr);
    UNLOCK_MUTEX(tls_mutex_ptr);

    DELETE_THREAD(thread_ptr);
    thread_ptr = 0;
}


void mipc_set_host_api_timeout(uint32_t timeout_ms)
{
    default_timeout_ms = timeout_ms;
}

void mipc_msg_set_timeout_once(uint32_t timeout_ms)
{
    //
    uint64_t tid;
    mipc_hashmap_element_t *hashmap_element_ptr;

    //get thread speific event
    tid = GETTID();
    LOCK_MUTEX(tls_mutex_ptr);
    if (hashmap_element_ptr = mipc_hashmap_get(tls_timeout_hashmap_ptr, &tid, sizeof(tid))) {
        //update value if existence
        uint32_t *timeout_ptr = (uint32_t *)MIPC_HASHMAP_GET_VAL_PTR(hashmap_element_ptr);
        *timeout_ptr = timeout_ms;
    } else {
        //add it if non-existence
        mipc_hashmap_add(tls_timeout_hashmap_ptr, &tid, sizeof(tid), &timeout_ms, sizeof(timeout_ms));
    }
    UNLOCK_MUTEX(tls_mutex_ptr);
}

static uint32_t mipc_msg_get_result(mipc_msg_t* msg_cnf_ptr)
{
    if(NULL != msg_cnf_ptr && NULL != mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_T_RESULT, NULL)) {
        return MTOHL(*(uint32_t *)mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_T_RESULT, NULL));
    } else {
        return 0xffffffff;
    }   
}

static int _mipc_msg_send(mipc_msg_t *msg_ptr)
{
    uint8_t* send_buf_ptr;
    uint16_t send_buf_len = 0;
    uint16_t total_write_bytes = 0;

    send_buf_ptr = mipc_msg_serialize(msg_ptr, &send_buf_len);
	if (send_buf_ptr == NULL){
        //serialize error
        return -2;
    }

    //dump recv message
    WRITECAP(send_buf_len, send_buf_ptr, MIPC_HOST_TO_DEVICE);

    LOCK_MUTEX(write_mutex_ptr);
    total_write_bytes = WRITECOM(send_buf_ptr, send_buf_len);
    UNLOCK_MUTEX(write_mutex_ptr);

    FREE(send_buf_ptr);
    if (total_write_bytes != send_buf_len) {
        ErrMsg("write error");
        return -1;
    }
#ifdef UNIT_TEST
    if (unit_test_set_msg_ptr) {
        UNIT_TEST_SEND_MSG(unit_test_set_msg_ptr);
        unit_test_set_msg_ptr = NULL;
    }
#endif
    return 0;
}

mipc_msg_t *_mipc_msg_sync_timeout(mipc_msg_t *msg_ptr, int32_t *ret_cause_ptr)
{
    EVENT *event_ptr;
    mipc_transaction_t tx;
    mipc_msg_t *msg_cnf_ptr;
    //
    uint64_t tid;
    mipc_hashmap_element_t *hashmap_element_ptr;
    uint32_t timeout_ms;
    int32_t ret_cause = 0;

    if (msg_ptr == NULL) {
        if (ret_cause_ptr) {
            *ret_cause_ptr = ERR_UNEXPECTED_MSG;
        }
        return NULL;
    }

    if ((MTOHS(msg_ptr->hdr.msg_id) & MIPC_MSG_TYPE_MASK) != MIPC_MSG_TYPE_REQ_CNF) {
        if (ret_cause_ptr) {
            *ret_cause_ptr = ERR_UNEXPECTED_MSG_TYPE;
        }
        return NULL;
    }

    //get thread speific event
    tid = GETTID();
    LOCK_MUTEX(tls_mutex_ptr);
    if (hashmap_element_ptr = mipc_hashmap_get(tls_timeout_hashmap_ptr, &tid, sizeof(tid))) {
        uint32_t *timeout_ptr = (uint32_t *)MIPC_HASHMAP_GET_VAL_PTR(hashmap_element_ptr);
        timeout_ms = *timeout_ptr;
        mipc_hashmap_del(hashmap_element_ptr); //thread specific timeout only active once
    } else {
        timeout_ms = default_timeout_ms;
    }
    UNLOCK_MUTEX(tls_mutex_ptr);

    //create event
    event_ptr = CREATE_EVENT(timeout_ms);

	if (event_ptr == NULL){
        // create event fail
        msg_cnf_ptr = NULL;
        return msg_cnf_ptr;
    }

    //fill
    memset(&tx, 0, sizeof(tx));
    tx.msg_txid = MTOHS(msg_ptr->hdr.msg_txid);
    tx.transaction_type = MIPC_TRANSACTION_TYPE_SYNC;
    tx.event_ptr = event_ptr;
    LOCK_MUTEX(mutex_ptr);
    //add to hashmap
    mipc_hashmap_add(transaction_hashmap_ptr, &tx.msg_txid, sizeof(tx.msg_txid), &tx, sizeof(tx));
    UNLOCK_MUTEX(mutex_ptr);

    if (_mipc_msg_send(msg_ptr) != 0) {
        ret_cause = ERR_WRITE_ERROR;
    }

    //send and wait complete for synchronous operation
    if ( ret_cause == 0 && WAIT_EVENT(event_ptr) == 0) {
        //get result
        msg_cnf_ptr = (mipc_msg_t *)event_ptr->result_ptr;
    } else {
        //timeout
        msg_cnf_ptr = NULL;
        if (ret_cause == 0) {
            ret_cause = ERR_TIME_OUT;
        }

        //remove hashmap element
        LOCK_MUTEX(mutex_ptr);
        if ((hashmap_element_ptr = mipc_hashmap_get(transaction_hashmap_ptr, &tx.msg_txid, sizeof(tx.msg_txid)))) {
            mipc_hashmap_del(hashmap_element_ptr);
        }
        UNLOCK_MUTEX(mutex_ptr);
    }
    //free event
    DELETE_EVENT(event_ptr);
    event_ptr = 0;

    if (ret_cause_ptr) {
        *ret_cause_ptr = ret_cause;
    }

    return msg_cnf_ptr;
}

mipc_msg_t *mipc_msg_sync_timeout(mipc_msg_t *msg_ptr) {
    return _mipc_msg_sync_timeout(msg_ptr, 0);
}

mipc_ret_cause_enum mipc_msg_sync_timeout_with_cause(mipc_msg_t *msg_ptr, mipc_msg_t **cnf_ptr)
{
    mipc_msg_t *cnf = 0;
    int32_t ret_cause = 0;

    cnf = _mipc_msg_sync_timeout(msg_ptr, &ret_cause);

    if (cnf_ptr) {
        *cnf_ptr = cnf;
    } else {
        mipc_msg_deinit(cnf);
    }

    return ret_cause;
}

mipc_msg_t *mipc_msg_sync(mipc_msg_t *msg_ptr)
{
    mipc_msg_set_timeout_once(0);
    return mipc_msg_sync_timeout(msg_ptr);
}

mipc_ret_cause_enum mipc_msg_sync_with_cause(mipc_msg_t *msg_ptr, mipc_msg_t **cnf_ptr)
{
    mipc_msg_t *cnf = 0;
    int32_t ret_cause = 0;

    mipc_msg_set_timeout_once(0);
    cnf = _mipc_msg_sync_timeout(msg_ptr, &ret_cause);

    if (cnf_ptr) {
        *cnf_ptr = cnf;
    } else {
        mipc_msg_deinit(cnf);
    }

    return ret_cause;
}

mipc_ret_cause_enum mipc_msg_async(mipc_msg_t *msg_ptr, void *msg_cb, void *cb_priv_ptr)
{
    mipc_transaction_t tx;
    if(NULL == msg_ptr)
    {
        DbgMsg("Msgptr is NULL");
        return -1;
    }

    //fill
    memset(&tx, 0, sizeof(tx));
    tx.msg_txid = MTOHS(msg_ptr->hdr.msg_txid);
    tx.transaction_type = MIPC_TRANSACTION_TYPE_ASYNC;
    tx.msg_cb = (MIPC_MSG_CB)msg_cb;
    tx.cb_priv_ptr = cb_priv_ptr;
    LOCK_MUTEX(mutex_ptr);
    //add to hashmap
    mipc_hashmap_add(transaction_hashmap_ptr, &tx.msg_txid, sizeof(tx.msg_txid), &tx, sizeof(tx));
    UNLOCK_MUTEX(mutex_ptr);

    //send
    if(_mipc_msg_send(msg_ptr) != 0)
    {
        return ERR_WRITE_ERROR;
    }

    return 0;
}

int32_t mipc_msg_unregister_ind(mipc_msg_sim_ps_id_enum sim_ps_id, mipc_msg_id_enum msg_id)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_hashmap_element_t *hashmap_element_ptr;
    uint32_t key = sim_ps_id << 16 | msg_id;

    LOCK_MUTEX(mutex_ptr);
    //check exist
    if ((hashmap_element_ptr = mipc_hashmap_get(ind_hashmap_ptr, &key, sizeof(key)))) {
        mipc_hashmap_del(hashmap_element_ptr);

        msg_req_ptr = mipc_msg_init(MIPC_INTERNAL_UNREGISTER_IND_REQ, sim_ps_id);
        mipc_msg_add_tlv_uint16(msg_req_ptr, MIPC_INTERNAL_UNREGISTER_IND_REQ_T_MSG_ID, msg_id);
    } else {
        UNLOCK_MUTEX(mutex_ptr);
        return -1;
    }

    UNLOCK_MUTEX(mutex_ptr);

#ifndef UNIT_TEST
    //send to modem
    msg_cnf_ptr = mipc_msg_sync(msg_req_ptr);

    if (msg_cnf_ptr != NULL && msg_req_ptr != NULL) {
        DbgMsg("Unregister IND MSG_ID=%04x Result:%x", msg_req_ptr->hdr.msg_id, mipc_msg_get_result(msg_cnf_ptr));
    }

    mipc_msg_deinit(msg_cnf_ptr);
#endif

    mipc_msg_deinit(msg_req_ptr);

    return 0;
}

int32_t mipc_msg_register_ind(mipc_msg_sim_ps_id_enum sim_ps_id, mipc_msg_id_enum msg_id, void *msg_cb, void *cb_priv_ptr)
{
    mipc_msg_t* msg_req_ptr;
    mipc_msg_t* msg_cnf_ptr;
    mipc_ind_t ind;
    mipc_hashmap_element_t *hashmap_element_ptr;
    uint32_t key = sim_ps_id << 16 | msg_id;

    LOCK_MUTEX(mutex_ptr);
    //check exist
    if ((hashmap_element_ptr = mipc_hashmap_get(ind_hashmap_ptr, &key, sizeof(key)))) {
        if (msg_cb == NULL) {
            mipc_hashmap_del(hashmap_element_ptr);

            msg_req_ptr = mipc_msg_init(MIPC_INTERNAL_UNREGISTER_IND_REQ, sim_ps_id);
            mipc_msg_add_tlv_uint16(msg_req_ptr, MIPC_INTERNAL_UNREGISTER_IND_REQ_T_MSG_ID, msg_id);
        } else {
            UNLOCK_MUTEX(mutex_ptr);
            return -1;
        }
    } else {
        //register callback
        memset(&ind, 0, sizeof(ind));
        ind.key = key;
        ind.msg_cb = (MIPC_MSG_CB)msg_cb;
        ind.cb_priv_ptr = cb_priv_ptr;
        //add to hashmap
        mipc_hashmap_add(ind_hashmap_ptr, &ind.key, sizeof(ind.key), &ind, sizeof(ind));

        msg_req_ptr = mipc_msg_init(MIPC_INTERNAL_REGISTER_IND_REQ, sim_ps_id);
        mipc_msg_add_tlv_uint16(msg_req_ptr, MIPC_INTERNAL_REGISTER_IND_REQ_T_MSG_ID, msg_id);
    }
    UNLOCK_MUTEX(mutex_ptr);

#ifndef UNIT_TEST
    //send to modem
    msg_cnf_ptr = mipc_msg_sync(msg_req_ptr);

    if (msg_cnf_ptr != NULL && msg_req_ptr != NULL) {
        DbgMsg("Register IND MSGID=%04x Result:%x", msg_req_ptr->hdr.msg_id, mipc_msg_get_result(msg_cnf_ptr));
    }
    
    mipc_msg_deinit(msg_cnf_ptr);
#endif

    mipc_msg_deinit(msg_req_ptr);

    return 0;
}

int32_t mipc_msg_register_ind_group(mipc_msg_sim_ps_id_enum sim_ps_id, uint16_t num, mipc_msg_id_enum *msg_id_group, void **cb_group, void **cb_priv_group)
{
    uint16_t i = 0, j = 0;
    uint32_t key = 0;
    mipc_ind_t ind = {0};
    mipc_msg_t* msg_req_ptr = NULL;
    mipc_msg_t* msg_cnf_ptr = NULL;
    mipc_msg_tlv_array_t *a_ptr = NULL;

    if(NULL == msg_id_group || NULL == cb_group) {
        DbgMsg("Register IND fail because invalid group");
        return -1;
    }
    LOCK_MUTEX(mutex_ptr);
    //check
    for(i=0;i<num;i++) {
        for(j=0;j<8;j++) {
            if(0!=((1<<j)&sim_ps_id)) {
                key = ((1<<(j+16))|(msg_id_group[i]));
                if (NULL == cb_group[i] || NULL != mipc_hashmap_get(ind_hashmap_ptr, &key, sizeof(key))) {
                    DbgMsg("Register key=%08x fail", key);
                    //cb = null or already register
                    UNLOCK_MUTEX(mutex_ptr);
                    return -1;
                }
            }
        }
    }
    //add hashmap
    for(i=0;i<num;i++) {
        for(j=0;j<8;j++) {
            if(0!=((1<<j)&sim_ps_id)) {
                key = ((1<<(j+16))|(msg_id_group[i]));
                memset(&ind, 0, sizeof(ind));
                ind.key = key;
                ind.msg_cb = (MIPC_MSG_CB)(cb_group[i]);
                ind.cb_priv_ptr = NULL != cb_priv_group ? cb_priv_group[i] : NULL;
                mipc_hashmap_add(ind_hashmap_ptr, &ind.key, sizeof(ind.key), &ind, sizeof(ind));
                printf("\naddddd_key=%08x",key);
            }
        }
    }
    //add and send
    msg_req_ptr = mipc_msg_init(MIPC_INTERNAL_REGISTER_IND_REQ, sim_ps_id);
    a_ptr = mipc_msg_add_tlv_array(msg_req_ptr, MIPC_INTERNAL_REGISTER_IND_REQ_T_MSG_ID_GROUP, num);
    for(i=0; i<num; i++) {
        mipc_msg_add_idx_uint16(msg_req_ptr, a_ptr, i, msg_id_group[i]);
    }
    UNLOCK_MUTEX(mutex_ptr);

#ifndef UNIT_TEST
    //send to modem
    msg_cnf_ptr = mipc_msg_sync(msg_req_ptr);

    if (msg_cnf_ptr != NULL && msg_req_ptr != NULL) {
        DbgMsg("Register IND MSGID=%04x Result:%x", msg_req_ptr->hdr.msg_id, mipc_msg_get_result(msg_cnf_ptr));
    }

    mipc_msg_deinit(msg_cnf_ptr);
#endif
	mipc_msg_deinit(msg_req_ptr);

    return 0;
}

void mipc_unreg_ind_all_cb(mipc_hashmap_element_t *hashmap_element_ptr, void *cb_priv_ptr)
{
    uint32_t key = *((uint32_t*)(hashmap_element_ptr->key_ptr));
    if(0 != (((uint32_t)cb_priv_ptr)&(key))) {
        mipc_hashmap_del(hashmap_element_ptr);
    }
}

int32_t mipc_msg_unregister_ind_group(mipc_msg_sim_ps_id_enum sim_ps_id, uint8_t unreg_all, uint16_t num, mipc_msg_id_enum *group)
{
    uint16_t i = 0, j = 0;
    uint32_t key = 0;
    mipc_ind_t ind = {0};
    mipc_msg_t* msg_req_ptr = NULL;
    mipc_msg_t* msg_cnf_ptr = NULL;
    mipc_msg_tlv_array_t *a_ptr = NULL;
    mipc_hashmap_element_t *hashmap_element_ptr = NULL;

    if(0 == unreg_all && NULL == group) {
        DbgMsg("Unregister IND fail because invalid group");
        return -1;
    }
    //remove hashmap element
    LOCK_MUTEX(mutex_ptr);
    if(unreg_all) {
        mipc_hashmap_iterate(ind_hashmap_ptr, mipc_unreg_ind_all_cb, ((void*)(sim_ps_id<<16)));
    } else {
        for(i=0;i<num;i++){
            for(j=0; j<8; j++) {
                if(0!=((1<<j)&sim_ps_id)) {
                    key = ((1<<(j+16))|(group[i]));
                    if((hashmap_element_ptr = mipc_hashmap_get(ind_hashmap_ptr, &key, sizeof(key)))) {
                        mipc_hashmap_del(hashmap_element_ptr);
                        printf("\ndelete_key=%08x",key);
                    }
                }
            }
        }
    }
    UNLOCK_MUTEX(mutex_ptr);
    //make mipc_msg&send to modem
    msg_req_ptr = mipc_msg_init(MIPC_INTERNAL_UNREGISTER_IND_REQ, sim_ps_id);
    if(unreg_all) {
        mipc_msg_add_tlv_uint8(msg_req_ptr, MIPC_INTERNAL_UNREGISTER_IND_REQ_T_UNREG_ALL, MIPC_BOOLEAN_TRUE);
    } else {
        a_ptr = mipc_msg_add_tlv_array(msg_req_ptr, MIPC_INTERNAL_UNREGISTER_IND_REQ_T_MSG_ID_GROUP, num);
        for(i=0; i<num; i++) {
            mipc_msg_add_idx_uint16(msg_req_ptr, a_ptr, i, group[i]);
        }
    }
#ifndef UNIT_TEST
    //send to modem
    msg_cnf_ptr = mipc_msg_sync(msg_req_ptr);

    if (msg_cnf_ptr != NULL && msg_req_ptr != NULL) {
        DbgMsg("Unregister IND MSGID=%04x Result:%x", msg_req_ptr->hdr.msg_id, mipc_msg_get_result(msg_cnf_ptr));
    }

    mipc_msg_deinit(msg_cnf_ptr);
#endif
	mipc_msg_deinit(msg_req_ptr);

    return 0;
}

int32_t mipc_msg_unregister_cmd(mipc_msg_id_enum msg_id)
{
    mipc_msg_t* msg_req_ptr;
    mipc_msg_t* msg_cnf_ptr;
    mipc_hashmap_element_t *hashmap_element_ptr;
    uint32_t key = msg_id;

    LOCK_MUTEX(mutex_ptr);
    //check exist
    if ((hashmap_element_ptr = mipc_hashmap_get(cmd_hashmap_ptr, &key, sizeof(key)))) {

        mipc_hashmap_del(hashmap_element_ptr);
        msg_req_ptr = mipc_msg_init(MIPC_INTERNAL_UNREGISTER_CMD_REQ, MIPC_MSG_ALL);
        mipc_msg_add_tlv_uint16(msg_req_ptr, MIPC_INTERNAL_UNREGISTER_CMD_REQ_T_MSG_ID, msg_id);

    } else {
        UNLOCK_MUTEX(mutex_ptr);
        return -1;
    }
    UNLOCK_MUTEX(mutex_ptr);

#ifndef UNIT_TEST
    //send to modem
    msg_cnf_ptr = mipc_msg_sync(msg_req_ptr);

    if (msg_cnf_ptr != NULL && msg_req_ptr != NULL) {
        DbgMsg("Unregister CMD MSGID=%04x Result:%x", msg_req_ptr->hdr.msg_id, mipc_msg_get_result(msg_cnf_ptr));
    }

    mipc_msg_deinit(msg_cnf_ptr);
#endif
	mipc_msg_deinit(msg_req_ptr);

    return 0;
}

int32_t mipc_msg_register_cmd(mipc_msg_id_enum msg_id, void *msg_cb, void *cb_priv_ptr)
{
    mipc_msg_t* msg_req_ptr;
    mipc_msg_t* msg_cnf_ptr;
    mipc_cmd_t cmd;
    mipc_hashmap_element_t *hashmap_element_ptr;
    uint32_t key = msg_id;

    LOCK_MUTEX(mutex_ptr);
    //check exist
    if ((hashmap_element_ptr = mipc_hashmap_get(cmd_hashmap_ptr, &key, sizeof(key)))) {
        if (msg_cb == NULL) {
            mipc_hashmap_del(hashmap_element_ptr);

            msg_req_ptr = mipc_msg_init(MIPC_INTERNAL_UNREGISTER_CMD_REQ, MIPC_MSG_ALL);
            mipc_msg_add_tlv_uint16(msg_req_ptr, MIPC_INTERNAL_UNREGISTER_CMD_REQ_T_MSG_ID, msg_id);
        } else {
            UNLOCK_MUTEX(mutex_ptr);
            return -1;
        }
    } else {
        //register callback
        memset(&cmd, 0, sizeof(cmd));
        cmd.key = key;
        cmd.msg_cb = (MIPC_MSG_CB)msg_cb;
        cmd.cb_priv_ptr = cb_priv_ptr;
        //add to hashmap
        mipc_hashmap_add(cmd_hashmap_ptr, &cmd.key, sizeof(cmd.key), &cmd, sizeof(cmd));

        msg_req_ptr = mipc_msg_init(MIPC_INTERNAL_REGISTER_CMD_REQ, MIPC_MSG_ALL);
        mipc_msg_add_tlv_uint16(msg_req_ptr, MIPC_INTERNAL_REGISTER_CMD_REQ_T_MSG_ID, msg_id);
    }
    UNLOCK_MUTEX(mutex_ptr);

#ifndef UNIT_TEST
    //send to modem
    msg_cnf_ptr = mipc_msg_sync(msg_req_ptr);

    if (msg_cnf_ptr != NULL && msg_req_ptr != NULL) {
        DbgMsg("Register CMD MSGID=%04x Result:%x", msg_req_ptr->hdr.msg_id, mipc_msg_get_result(msg_cnf_ptr));
    }

    mipc_msg_deinit(msg_cnf_ptr);
#endif

    mipc_msg_deinit(msg_req_ptr);

    return 0;
}

int32_t mipc_msg_rsp(mipc_msg_t *msg_rsp_ptr)
{
    //send
    _mipc_msg_send(msg_rsp_ptr);

    return 0;
}

int32_t mipc_msg_ntf(mipc_msg_t *msg_ntf_ptr)
{
    //send
    _mipc_msg_send(msg_ntf_ptr);

    return 0;
}

#ifdef MIPC_HOST_SUPPORT_ONLY_TO_GEN98
//TO BE REMOVED in NEXT_GEN bellow defines
//cb_mode2(MIPC_API_CB) was removed in mipc_lib, leave these interfaces(xxx_api) for legacy code build pass only(would get err if the api_cb is not NULL)
mipc_ret_cause_enum mipc_msg_async_api(mipc_msg_t *msg_ptr, void *msg_cb, MIPC_API_CB api_cb, void *cb_priv_ptr)
{
    return NULL!=api_cb ? ERR_UNEXPECTED_MSG : mipc_msg_async(msg_ptr, msg_cb, cb_priv_ptr);
}

int32_t mipc_msg_register_ind_api(mipc_msg_sim_ps_id_enum sim_ps_id, mipc_msg_id_enum msg_id, void *msg_cb, MIPC_API_CB api_cb, void *cb_priv_ptr)
{
    return NULL!=api_cb ? ERR_UNEXPECTED_MSG : mipc_msg_register_ind(sim_ps_id, msg_id, msg_cb, cb_priv_ptr);

}

int32_t mipc_msg_unregister_ind_api(mipc_msg_sim_ps_id_enum sim_ps_id, mipc_msg_id_enum msg_id)
{
    return mipc_msg_unregister_ind(sim_ps_id, msg_id);
}

int32_t mipc_msg_register_cmd_api(mipc_msg_id_enum msg_id, void *msg_cb, MIPC_API_CB api_cb, void *cb_priv_ptr)
{
    return NULL!=api_cb ? ERR_UNEXPECTED_MSG : mipc_msg_register_cmd(msg_id, msg_cb, cb_priv_ptr);
}

int32_t mipc_msg_unregister_cmd_api(mipc_msg_id_enum msg_id)
{
    return mipc_msg_unregister_cmd(msg_id);
}
//TO BE REMOVED in NEXT_GEN upper defines
#endif /* MIPC_HOST_SUPPORT_ONLY_TO_GEN98 */
