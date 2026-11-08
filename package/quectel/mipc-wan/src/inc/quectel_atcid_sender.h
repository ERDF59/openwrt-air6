/*

    *Copyright :

    *Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.

    *Quectel Wireless Solutions Proprietary and Confidential.
*/
#ifndef QUECTEL_ATCID_SENDER_H
#define QUECTEL_ATCID_SENDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#define ATCMD_MAX_LEN 1024

typedef struct COMM_DEV
{
    char device_name[50];
    int fd;
} COMM_DEV;

typedef enum ATCID_INIT_TYPE
{
    TYPE_LOCAL_LOOPBACK = 1,  // use local loopback 127.0.0.1
    TYPE_ADB_ATCI_SOCKET = 2, // use /dev/adb_atci_socket device
} ATCID_INIT_TYPE;

typedef struct atcmd_sender_msg_t
{
    unsigned int timeout;           //timeout for second, if zero, default 5s
    char at_cmd[ATCMD_MAX_LEN];
    char *at_recv;
} atcmd_sender_msg_t;

#define QUECTEL_ATCID_SENDER_ERROR_FDISSET_FAILED (-4)
#define QUECTEL_ATCID_SENDER_ERROR_RECONNECT (-3)
#define QUECTEL_ATCID_SENDER_ERROR_SHORT_TIMEOUT (-2)
#define QUECTEL_ATCID_SENDER_ERROR           (-1)

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief Initialize the connection with atcid
  @param[in] type 
  TYPE_LOCAL_LOOPBACK
  TYPE_ADB_ATCI_SOCKET
    
  @return
  COMM_DEV - file description
  NULL - Register failed
  */
/*-----------------------------------------------------------------------------------------------*/
COMM_DEV *ql_atcid_sender_init(ATCID_INIT_TYPE type);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief Deinit the connection with atcid
  @param[in] pcomm_dev 
  Here is the return value in ql_atcid_sender_init
    
  @return
  Without return
  */
/*-----------------------------------------------------------------------------------------------*/
void ql_atcid_sender_deinit(COMM_DEV *pcomm_dev);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief Use to send AT command for short string return
  @param[in] pcomm_dev 
  Here is the return value in ql_atcid_sender_init
  @param[in] atcmd_sender 
  A struct for sending AT command
     timeout - Used to set the timeout for return, in seconds, max 300s (5 min)
     at_cmd - AT command you send, like: AT+CFUN?
     at_recv - Must be NULL here. If AT receive success, it will malloc and AT feedback will store here
               so you MUST free it after use
    
  @return
  0 - Success
  QUECTEL_ATCID_SENDER_ERROR - on error

  */
/*-----------------------------------------------------------------------------------------------*/
int ql_atcid_sender_sending(COMM_DEV *pcomm_dev, atcmd_sender_msg_t *atcmd_sender);

/* -------------------------- Callback --------------------------*/
/*-----------------------------------------------------------------------------------------------*/
/**
  @brief AT indicator for callback
  @param[in] at_recv 
  AT URC message, is same as you can see in AT COM PORT
  @param[in] at_recv_len
  length for at_recv
    
  @return
  NULL
  */
/*-----------------------------------------------------------------------------------------------*/
typedef void (*AT_IND_FUNC)(const char *at_recv, uint16_t at_recv_len);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief use for register AT callback
  @param[in] ind_cb 
  see above
    
  @return
  0 - Success
  Other value - Register failed

  Note: This api is no need to depends on ql_atcid_sender_init
  */
/*-----------------------------------------------------------------------------------------------*/
int ql_atcid_sender_init_cb(AT_IND_FUNC ind_cb);

#ifdef __cplusplus
}
#endif
#endif


