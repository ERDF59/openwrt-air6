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

#ifndef _PLATFORM_MTK_H_
#define _PLATFORM_MTK_H_

#include "kal_public_defs.h"
#include "kal_public_api.h"

#define DLL_EXPORT

#define HTOML(x) (x)
#define HTOMS(x) (x)
#define MTOHL(x) (x)
#define MTOHS(x) (x)
#define ALLOC get_ctrl_buffer
#define FREE free_ctrl_buffer
#define SLEEP(x) kal_sleep_task(x)
#define MEMCPY memcpy
#define MEMCMP memcmp
#define MEMSET memset

#define SRC_FILE_OFFSET (0)
#define ErrMsg(format, ...)
#define DbgMsg(format, ...)
#define InfoMsg(format, ...)
#define Info2Msg(format, ...)

#endif
