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

#ifndef _MIPC_H_
#define _MIPC_H_

/*!
    \defgroup COMMON
    @{
*/

#include "platform.h"
#include "mipc_msg_tlv_const.h"

typedef enum MIPC_MSG_enum mipc_msg_id_enum;
typedef enum mipc_result_const_enum mipc_result_enum;

#define MIPC_VERSION (0x00000002)

#define MIPC_SIM0   (1 << 0)
#define MIPC_SIM1   (1 << 1)
#define MIPC_SIM2   (1 << 2)
#define MIPC_SIM3   (1 << 3)
#define MIPC_PS_START (4)
#define MIPC_PS0    (1 << 4)
#define MIPC_PS1    (1 << 5)
#define MIPC_PS2    (1 << 6)
#define MIPC_PS3    (1 << 7)
#define MIPC_ALL    (0xff)

//#define MIPC_T_LOCAL_START (0x100)

#define MIPC_MSG_TYPE_MASK              (0xC000)
#define MIPC_MSG_TYPE_REQ_CNF           (0x0000)
#define MIPC_MSG_TYPE_IND               (0x4000)
#define MIPC_MSG_TYPE_CMD_RSP           (0x8000)
#define MIPC_MSG_TYPE_NTF               (0xC000)
//


/*!
    @}
*/

#endif
