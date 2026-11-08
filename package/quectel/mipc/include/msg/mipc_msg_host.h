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

#ifndef _MIPC_MSG_HOST_H_
#define _MIPC_MSG_HOST_H_

#include "platform.h"
#include "mipc.h"
#include "mipc_hashmap.h"
#include "mipc_msg.h"

/*!
    \brief MIPC MSG HOST API result
*/
typedef enum {
    ERR_NO_ERR                  = 0,
    ERR_UNEXPECTED_MSG          = -1,
    ERR_UNEXPECTED_MSG_TYPE     = -2,
    ERR_CB_IS_NULL              = -3,
    ERR_WRITE_ERROR             = -4,
    ERR_TIME_OUT                = -5
}mipc_ret_cause_enum;

typedef void (*MIPC_MSG_CB)(mipc_msg_t *msg_ptr, void *priv_ptr);

typedef enum {
    MIPC_TRANSACTION_TYPE_SYNC = 0,
    MIPC_TRANSACTION_TYPE_ASYNC = 1
} mipc_transaction_type_enum;

typedef enum{
    MIPC_HOST_TO_DEVICE = 0,
    MIPC_DEVICE_TO_HOST = 1
} mipc_msg_direction_enum;

typedef struct {
    uint16_t msg_txid;
    mipc_transaction_type_enum transaction_type;
    //for sync
    EVENT *event_ptr;
    //for async
    MIPC_MSG_CB msg_cb;
    void *cb_priv_ptr;
} mipc_transaction_t;

typedef struct {
    uint32_t key;
    MIPC_MSG_CB msg_cb;
    void *cb_priv_ptr;
} mipc_ind_t;

typedef struct {
    uint32_t key;
    MIPC_MSG_CB msg_cb;
    void *cb_priv_ptr;
} mipc_cmd_t;

#ifdef __cplusplus
extern "C" {
#endif

//
DLL_EXPORT mipc_msg_t *mipc_msg_sync_timeout(mipc_msg_t *msg_ptr);
DLL_EXPORT mipc_ret_cause_enum mipc_msg_sync_timeout_with_cause(mipc_msg_t *msg_ptr, mipc_msg_t **cnf_ptr);
DLL_EXPORT mipc_msg_t *mipc_msg_sync(mipc_msg_t *msg_ptr);
DLL_EXPORT mipc_ret_cause_enum mipc_msg_sync_with_cause(mipc_msg_t *msg_ptr, mipc_msg_t **cnf_ptr);
DLL_EXPORT mipc_ret_cause_enum mipc_msg_async(mipc_msg_t *msg_ptr, void *msg_cb, void *cb_priv_ptr);
//
DLL_EXPORT int32_t mipc_msg_register_ind(mipc_msg_sim_ps_id_enum sim_ps_id, mipc_msg_id_enum msg_id, void *msg_cb, void *cb_priv_ptr);
//register MIPC_INDs(defined in group, only in specific <sim or ps> defined in sim_ps_id) with relate cb&cb_priv(defined in _group)
DLL_EXPORT int32_t mipc_msg_register_ind_group(mipc_msg_sim_ps_id_enum sim_ps_id, uint16_t num, mipc_msg_id_enum *msg_id_group, void **cb_group, void **cb_priv_group);
//unregister MIPC_INDs(defined in group) only in specific <sim or ps>(defined in sim_ps_id)
DLL_EXPORT int32_t mipc_msg_unregister_ind_group(mipc_msg_sim_ps_id_enum sim_ps_id, uint8_t unreg_all, uint16_t num, mipc_msg_id_enum *group);
//
DLL_EXPORT int32_t mipc_msg_unregister_ind(mipc_msg_sim_ps_id_enum sim_ps_id, mipc_msg_id_enum msg_id);
//
DLL_EXPORT int32_t mipc_msg_register_cmd(mipc_msg_id_enum msg_id, void *msg_cb, void *cb_priv_ptr);
//
DLL_EXPORT int32_t mipc_msg_unregister_cmd(mipc_msg_id_enum msg_id);
//
DLL_EXPORT int32_t mipc_msg_rsp(mipc_msg_t *msg_ptr);
//
DLL_EXPORT int32_t mipc_msg_ntf(mipc_msg_t *msg_ptr);

DLL_EXPORT int32_t mipc_init(const char *process_name_ptr);
DLL_EXPORT int32_t mipc_init_debug(const char *process_name_ptr);
DLL_EXPORT void mipc_deinit();
DLL_EXPORT void mipc_msg_set_timeout_once(uint32_t timeout_ms);
DLL_EXPORT void mipc_set_host_api_timeout(uint32_t timeout_ms);

DLL_EXPORT void UNIT_TEST_SET_MSG(mipc_msg_t *msg_ptr);
DLL_EXPORT void UNIT_TEST_SEND_MSG(mipc_msg_t *msg_ptr);

#define MIPC_HOST_SUPPORT_ONLY_TO_GEN98
#ifdef MIPC_HOST_SUPPORT_ONLY_TO_GEN98
//TO BE REMOVED in NEXT_GEN bellow defines
//cb_mode2(MIPC_API_CB) was removed in mipc_lib, leave these interfaces(xxx_api) for legacy code build pass only(would get err if the api_cb is not NULL)
typedef void (*MIPC_API_CB)(
    mipc_msg_sim_ps_id_enum sim_ps_id,
    void *result_ptr,
    void *cb_priv_ptr
);
DLL_EXPORT mipc_ret_cause_enum mipc_msg_async_api(mipc_msg_t *msg_ptr, void *msg_cb, MIPC_API_CB api_cb, void *cb_priv_ptr);
DLL_EXPORT int32_t mipc_msg_register_ind_api(mipc_msg_sim_ps_id_enum sim_ps_id, mipc_msg_id_enum msg_id, void *msg_cb, MIPC_API_CB api_cb, void *cb_priv_ptr);
DLL_EXPORT int32_t mipc_msg_unregister_ind_api(mipc_msg_sim_ps_id_enum sim_ps_id, mipc_msg_id_enum msg_id);
DLL_EXPORT int32_t mipc_msg_register_cmd_api(mipc_msg_id_enum msg_id, void *msg_cb, MIPC_API_CB api_cb, void *cb_priv_ptr);
DLL_EXPORT int32_t mipc_msg_unregister_cmd_api(mipc_msg_id_enum msg_id);
//TO BE REMOVED in NEXT_GEN upper defines
#endif /* MIPC_HOST_SUPPORT_ONLY_TO_GEN98 */

#ifdef __cplusplus
}
#endif

#endif
