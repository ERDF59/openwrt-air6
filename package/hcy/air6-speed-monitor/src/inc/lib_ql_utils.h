/*

    *Copyright :

    *Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.

    *Quectel Wireless Solutions Proprietary and Confidential.
*/


#ifndef _UTILS_H
#define _UTILS_H

#ifndef NMACQUAD
#define NMACQUAD(addr) \
	((unsigned char *)addr)[3], \
    ((unsigned char *)addr)[2], \
	((unsigned char *)addr)[1], \
    ((unsigned char *)addr)[0], \
        ((unsigned char *)addr)[5], \
        ((unsigned char *)addr)[4]
#endif

#ifndef NIPQUAD
#define NIPQUAD(addr) \
	((unsigned char *)addr)[3], \
        ((unsigned char *)addr)[2], \
        ((unsigned char *)addr)[1], \
        ((unsigned char *)addr)[0]
#endif

#ifndef NIPHALF
#define NIPHALF(addr) \
        ((unsigned short *)addr)[1], \
        ((unsigned short *)addr)[0]
#endif

#ifndef ETH_ALEN
#define ETH_ALEN  (6)  /* 0xAABBCCDDEEFF */
#endif

#define MAC_STR_LEN      (2*6 + 5) /* AA:BB:CC:DD:EE:FF */
#define HZ 1
#define is_zero_addr 1

/**
 * is_zero_ether_addr - Determine if give Ethernet address is all zeros.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return true if the address is all zeroes.
 *
 * Please note: addr must be aligned to u16.
 */
static inline bool is_zero_ether_addr(const unsigned char *addr)
{
	return ((*(const unsigned int *)addr) | (*(const unsigned short *)(addr + 4))) == 0;
}

/**
 * is_multicast_ether_addr - Determine if the Ethernet address is a multicast.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return true if the address is a multicast address.
 * By definition the broadcast address is also a multicast address.
 */
static inline bool is_multicast_ether_addr(const unsigned char *addr)
{
	unsigned int a = *(const unsigned int *)addr;
	return 0x01 & a;
}

/**
 * is_valid_ether_addr - Determine if the given Ethernet address is valid
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Check that the Ethernet address (MAC) is not 00:00:00:00:00:00, is not
 * a multicast address, and is not FF:FF:FF:FF:FF:FF.
 *
 * Return true if the address is valid.
 *
 * Please note: addr must be aligned to u16.
 */
static inline bool is_valid_ether_addr(const unsigned char *addr)
{
	/* FF:FF:FF:FF:FF:FF is a multicast address so we don't need to
	 * explicitly check for it here. */
	return !is_multicast_ether_addr(addr) && !is_zero_ether_addr(addr);
}

int str_to_mac (unsigned char *mac, char *str);
int str_to_ip (unsigned int *ip, char *str);
int str_to_ipv6 (unsigned int *ipv6, char *str, unsigned int byte);
char *ipv4_to_str(const unsigned int *addr);
char *ipv6_to_str (const unsigned int ipv6_0, const unsigned int ipv6_1, const unsigned int ipv6_2, const unsigned int ipv6_3);
void mac_to_str(const unsigned char *mac, char *str);
int is_valid_ipv4_str(unsigned char * ipv4_str);
int is_valid_ipv6_str(const char *ipv6_str);
int is_valid_mac_str(char* mac_str);
void serialize_mac(unsigned char *dst_mac, unsigned char *src_mac);

#endif /* _UTILS_H */