/*

    *Copyright :

    *Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.

    *Quectel Wireless Solutions Proprietary and Confidential.
*/
#ifndef QUECTEL_LIB_SPEED_MONITOR_H
#define QUECTEL_LIB_SPEED_MONITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/syscall.h>

#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/sockios.h>
#include <linux/netlink.h>
#include <linux/types.h>
#include <errno.h>
#include "log.h"
#include "ql_err_code.h"
#include "lib_ql_utils.h"

/* Note: It is recommended to use the v2 version of the API */

//hwnat ioctl cmd
#define HW_NAT_MIB_GET_ALL_IP_MAC_TOTAL	(0x101)       // get all ip_mac mib total data && real time mib data

#define FOE_4TB_SIZ 1024
#define FOE_4TB_SIZ_ALL_MID (FOE_4TB_SIZ)
#define HWNAT_MAX_MIB_IP_ENTRY_NUM (512)

/* Compatible design: enable lib_speed_monitor v1 */
#define LIB_SPEED_MONITOR_V1_ENABLE    1   //0 : disable, 1 : enable 

#if LIB_SPEED_MONITOR_V1_ENABLE
struct hwnat_mib_packets_8B {
	unsigned long tx_bytes;
	unsigned long rx_bytes;
	unsigned long tx_packets;
	unsigned long rx_packets;
};

struct hwnat_mib_ip_mac {
	union {
		unsigned int ipv4_addr;
		unsigned int ipv6_addr[4];
		}ip;
	unsigned char mac_addr[ETH_ALEN];
	unsigned int is_ipv4;
  struct hwnat_mib_packets_8B packets;
};

struct hwnat_mib_ip_mac_total {
	struct hwnat_mib_ip_mac packets_total;
	struct hwnat_mib_ip_mac packets_currect;
};

struct hwnat_mib_all_ip_mac_args_total {
	unsigned int total_num;
	struct hwnat_mib_ip_mac_total entries[FOE_4TB_SIZ_ALL_MID];
};

int ql_pop_lan_devices_packets_total(struct hwnat_mib_all_ip_mac_args_total *ip_mac_args_total, unsigned char *mac_addr, int is_match_mac);
void dump_ip_mac_args_total_speed_data(void *arg_in);
#endif

struct hwnat_mib_ip {
	union {
		unsigned int ipv4_addr;
		unsigned int ipv6_addr[4];
		}ip;
	unsigned int is_ipv4;
	unsigned long long tx_bytes;
	unsigned long long rx_bytes;
	unsigned long long tx_packets;
	unsigned long long rx_packets;
};

struct hwnat_mib_ip_mac_t {
	struct hwnat_mib_ip mib_ip;
	unsigned char mac_addr[ETH_ALEN];
};

/* real time mib data for each ip address */
struct hwnat_mib_all_ip_mac_args {
	unsigned int entry_num;
	struct hwnat_mib_ip_mac_t entries[HWNAT_MAX_MIB_IP_ENTRY_NUM];
};

struct hwnat_mib_mac_total {
	/* total mib data */
	struct hwnat_mib_ip_mac_t mib;
	/* traffic rate */
	unsigned long tx_bps;
	unsigned long rx_bps;
};

/* total mib data for each mac address */
struct hwnat_mib_all_mac_total_args {
	unsigned int entry_num;
	struct hwnat_mib_mac_total entries[HWNAT_MAX_MIB_IP_ENTRY_NUM];
};

struct hwnat_mib_all_ip_mac_args_ext {
	struct hwnat_mib_all_mac_total_args  mac_total_args;   /* total mib data */
	struct hwnat_mib_all_ip_mac_args ip_mac_args;  /* real time mib data */
};

void set_open_debug(int open_log);
int get_open_debug(void);
int ql_pop_lan_devices_packets_total_v2(struct hwnat_mib_all_mac_total_args *all_mac_total_args, unsigned char *mac_addr, int is_match_mac);
int ql_pop_lan_devices_all_ip_mac_args_v2(struct hwnat_mib_all_ip_mac_args *all_ip_mac_args);
void dump_all_mac_total_args_speed_data_v2(void *arg_in);
void dump_all_ip_mac_args_speed_data_v2(void *arg_in);
#endif


