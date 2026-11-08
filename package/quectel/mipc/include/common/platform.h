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

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#if !defined(KAL_ON_OSCAR)
#include <stdint.h>
#include <limits.h>
#else
typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
#endif

#if (__WORDSIZE == 64)
#define BUILD_64 (1)
#endif

//we don't use long double, so the largest base data type size will be 8 bytes
//aligned to 2^3 = 8
#define ALIGN_BITS (3)
#define ALIGN_SIZE (1 << ALIGN_BITS)
#define ALIGN(x) ((((x) + (ALIGN_SIZE - 1)) >> ALIGN_BITS) << ALIGN_BITS)

#if (BUILD_64 == 1)
#define POINTER_TO_UINT(x) ((unsigned long long)(x))
#else
#define POINTER_TO_UINT(x) ((unsigned long)(x))
#endif

#if defined(_MSC_VER) && !defined(KAL_ON_OSCAR)
#include "platform_windows.h"
#elif defined (_LINUX)
#ifndef offsetof
#define offsetof(t, m) ((size_t)&((t *)0)->m)
#endif
#include "platform_linux.h"
#elif defined (__ZEPHYR__)
#ifndef offsetof
#define offsetof(t, m) ((size_t)&((t *)0)->m)
#endif
#include "platform_zephyr.h"
#else
#ifndef offsetof
#define offsetof(t, m) ((size_t)&((t *)0)->m)
#endif
#include "platform_mtk.h"
#endif

#endif
