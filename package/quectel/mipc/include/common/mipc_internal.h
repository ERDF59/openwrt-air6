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

#ifndef _MIPC_INTERNAL_H_
#define _MIPC_INTERNAL_H_

/*!
    \defgroup INTERNAL
    @{
*/

#include "platform.h"
#include "mipc_msg_tlv_const.h"

/** Filter rule */
#define IPC_FILTER_BY_PROTOCOL      (0x0001 << 0)
#define IPC_FILTER_BY_SRC_PORT      (0x0001 << 1)
#define IPC_FILTER_BY_DST_PORT      (0x0001 << 2)
#define IPC_FILTER_BY_SRC_IPV4      (0x0001 << 3)
#define IPC_FILTER_BY_DST_IPV4      (0x0001 << 4)
#define IPC_FILTER_BY_SRC_IPV6      (0x0001 << 5)
#define IPC_FILTER_BY_DST_IPV6      (0x0001 << 6)
#define IPC_FILTER_BY_ICMPV4_TYPE   (0x0001 << 7)
#define IPC_FILTER_BY_ICMPV6_TYPE   (0x0001 << 8)
#define IPC_FILTER_BY_EBI           (0x0001 << 9)
#define IPC_FILTER_BY_PDN_ID        (0x0001 << 10)
#define IPC_FILTER_BY_NETIF_ID      (0x0001 << 11)
#define IPC_FILTER_BY_TCP_FLAGS     (0x0001 << 12)
#define IPC_FILTER_BY_SPI           (0x0001 << 13)

#define IPC_IP_TYPE_IPV4            1
#define IPC_IP_TYPE_IPV6            2

/*
 * IANA IP Protocol Numbers.
 * Ref: http://www.iana.org/assignments/protocol-numbers/protocol-numbers.xml
 */
#define IPC_HDR_PROT_IPV6_HOP           0 /* IPv6 Hop-by-Hop Option */
#define IPC_HDR_PROT_ICMP               1 /* Internet Control Message */
#define IPC_HDR_PROT_IPV4_ENC           4 /* IPv4 encapsulation */
#define IPC_HDR_PROT_TCP                6 /* Transmission Control */
#define IPC_HDR_PROT_UDP                17 /* User Datagram */
#define IPC_HDR_PROT_IPV6_ENC           41 /* IPv6 encapsulation */
#define IPC_HDR_PROT_IPV6_ROUTE         43 /* Routing Header for IPv6 */
#define IPC_HDR_PROT_IPV6_FRAG          44 /* Fragment Header for IPv6 */
#define IPC_HDR_PROT_ESP                50 /* Encap Security Payload */
#define IPC_HDR_PROT_AH                 51 /* Authentication Header */
#define IPC_HDR_PROT_ICMPV6             58 /* ICMP for IPv6 */
#define IPC_HDR_PROT_IPV6_NONEXT        59 /* No Next Header for IPv6 */
#define IPC_HDR_PROT_IPV6_DEST          60 /* Destination Options for IPv6 */

/*
 * TCP_FLAG.
 */
#define IPC_HDR_TCP_FLAG_FIN    0x01
#define IPC_HDR_TCP_FLAG_SYN    0x02
#define IPC_HDR_TCP_FLAG_RST    0x04    // Reset
#define IPC_HDR_TCP_FLAG_PUSH   0x08
#define IPC_HDR_TCP_FLAG_ACK    0x10
#define IPC_HDR_TCP_FLAG_URG    0x20    // Urgent
#define IPC_HDR_TCP_FLAG_ECE    0x40    // ECN Echo
#define IPC_HDR_TCP_FLAG_CWR    0x80    // Congestion Window Reduced
#define IPC_HDR_TCP_FLAG_NS     0x100   // ECN-nonce concealment protection (added to header by RFC 3540)

typedef enum {
    NCCMNI_NET_IF_0 = 0,
    NCCMNI_NET_IF_1,
    NCCMNI_NET_IF_2,
    NCCMNI_NET_IF_3,
    NCCMNI_NET_IF_4,
    NCCMNI_NET_IF_5,
    NCCMNI_NET_IF_6,
    NCCMNI_NET_IF_7,
    NCCMNI_NET_IF_8,
    NCCMNI_NET_IF_9,
    NCCMNI_NET_IF_10,
    NCCMNI_NET_IF_11,
    NCCMNI_NET_IF_12,
    NCCMNI_NET_IF_13,
    NCCMNI_NET_IF_14,
    NCCMNI_NET_IF_15,
    NCCMNI_NET_IF_16,
    NCCMNI_NET_IF_17,
    NCCMNI_NET_IF_18,
    NCCMNI_NET_IF_19,
    NCCMNI_NET_IF_20,
    NCCMNI_NET_IF_21,
    NCCMNI_NET_IF_22,
    NCCMNI_NET_IF_23,
    NCCMNI_NET_IF_24,
    NCCMNI_NET_IF_25,
    NCCMNI_NET_IF_26,
    NCCMNI_NET_IF_27,
    NCCMNI_NET_IF_28,
    NCCMNI_NET_IF_29,
    NCCMNI_NET_IF_30,
    NCCMNI_NET_IF_31,
    NCCMNI_NET_IF_MAX = 0xFF,
} NCCMNI_NET_IF_T;

#pragma pack (push)
#pragma pack (1)
/*the largest fields is 4 bytes*/

/** @set filter req struct */
typedef struct {
    uint8_t nccmni_net_if;
    uint8_t reserve1;
    uint16_t nccmni_seq;
    uint32_t valid_field; //mandatory, a bitmap, refer to IPC_FILTER_BY_XXX
    uint8_t ip_type; //IPC_IP_TYPE_IPV4 or IPC_IP_TYPE_IPV6
    uint8_t ctrl_protocol; //IPC_HDR_PROT_XXX
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t tcp_flags; //IPC_HDR_TCP_FLAG_XXX
    uint32_t spi; //
    uint8_t src_v4_addr[4]; //Source IPv4 address to filter.
    uint8_t dst_v4_addr[4]; //Destination IPv4 address to filter.
    uint8_t src_v6_addr[16]; //Source IPv6 address to filter.
    uint8_t dst_v6_addr[16]; //Destination IPv6 address to filter.
    uint8_t icmpv4_type;    // Type field of ICMPv4 header to filter.
    uint8_t icmpv6_type;    // Type field of ICMPv6 header to filter.
    uint16_t reserve;
    uint32_t features;
} mipc_internal_set_filter_req_t_struct4;

typedef struct {
    uint8_t nccmni_net_if;
    uint8_t reserve1;
    uint16_t nccmni_seq;
    int32_t filter_id; //>=0: filter id, <0: set failed
} mipc_internal_set_filter_cnf_t_struct4;

typedef struct {
    uint8_t nccmni_net_if;
    uint8_t reserve1;
    uint16_t nccmni_seq;
    int32_t is_deregister_all_filter; //0: deregister the filter id only, 1: deregister all filter
    int32_t filter_id; //filter id to deregister
} mipc_internal_reset_filter_req_t_struct4;

typedef struct {
    uint8_t nccmni_net_if;
    uint8_t reserve1;
    uint16_t nccmni_seq;
    int32_t is_success; //1: success
} mipc_internal_reset_filter_cnf_t_struct4;

/*the largest fields is 8 bytes*/
#pragma pack (pop)

/*!
    @}
*/

#endif

