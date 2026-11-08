/*

    *Copyright :

    *Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.

    *Quectel Wireless Solutions Proprietary and Confidential.
*/

#include "lib_speed_monitor.h"
#include <time.h>
#include "lib_ql_utils.h"

#define SPEED_MIT_VERSION "V4.0.0"

#define USE_LIB_SPEED_MONITOR_V1   1   //0 : V2, 1 : V1

static void show_usage(char **argv)
{
    printf("%s tool version:%s\r\n",argv[0],SPEED_MIT_VERSION);
    printf("%s -a/A [mac_addr]\r\n",argv[0]);
    printf("   pop all lan devices total packets if use 'A' open speed_monitor debug log\r\n");
    printf("Example: %s -a/A 6c:16:32:0d:55:49 \r\n",argv[0]);
    printf("   pop assign lan devices(6c:16:32:0d:55:49) total packets if use 'A' open speed_monitor debug log\r\n");
}
#if USE_LIB_SPEED_MONITOR_V1
static void dump_data(void *arg_in)
{
  int i = 0;
  struct hwnat_mib_all_ip_mac_args_total *argsSpeed = NULL;
  argsSpeed = (struct hwnat_mib_all_ip_mac_args_total *)arg_in;

  printf("\r\n==========================entry_dump===============================\r\n");
  printf("%-17s  %-12s  %-12s  %-16s  %-16s\n","MAC","tx/bps","rx/bps","tx","rx");
  for(i=0; i<argsSpeed->total_num; i++) {
        printf("%02X:%02X:%02X:%02X:%02X:%02X  %-12ld  %-12ld  %-16ld  %-16ld\n",
          NMACQUAD(argsSpeed->entries[i].packets_total.mac_addr),
          argsSpeed->entries[i].packets_currect.packets.tx_bytes*8,
          argsSpeed->entries[i].packets_currect.packets.rx_bytes*8,
          argsSpeed->entries[i].packets_total.packets.tx_bytes*8,
          argsSpeed->entries[i].packets_total.packets.rx_bytes*8);
  }
}

static void showTotal(unsigned char *mac_addr)
{
  int is_match_mac = 0;
  unsigned char __mac_addr[ETH_ALEN] = {0};
  struct tm *t;
  time_t tt;
  static struct hwnat_mib_all_ip_mac_args_total argsSpeed;

  serialize_mac(__mac_addr, mac_addr);

  is_match_mac = is_valid_ether_addr(mac_addr);

  printf("is_match_mac:%d %02X:%02X:%02X:%02X:%02X:%02X\r\n",
          is_match_mac, NMACQUAD(__mac_addr));

  while (1)
  {
    sleep(1);
    time(&tt);
    t = localtime(&tt);
    if(ql_pop_lan_devices_packets_total(&argsSpeed, __mac_addr, is_match_mac) != QL_STATUS_OK){
        continue;
    }
    printf("start                             ");
    printf("%2d/%02d/%04d %02d:%02d:%02d\n", t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec);

    /*SHOW IP/MAC tx_bytes & rx_bytes*/
    dump_data(&argsSpeed);
    printf("========\r\n\r\n");
  }

}

#else

static void _dump_ip_mac_args_total_speed_data(void *arg_in)
{
    int i = 0;
    struct hwnat_mib_all_mac_total_args *argsSpeed = NULL;
    argsSpeed = (struct hwnat_mib_all_mac_total_args *)arg_in;

    printf("\r\n==========================entry_dump===============================\r\n");
    printf("%-17s  %-12s  %-12s  %-16s  %-16s\n","MAC","tx/bps","rx/bps","tx","rx");
    for(i=0; i<argsSpeed->entry_num; i++) {
          printf("%02x:%02x:%02x:%02x:%02x:%02x  %-12ld  %-12ld  %-16ld  %-16ld \n",
            NMACQUAD(argsSpeed->entries[i].mib.mac_addr),
            argsSpeed->entries[i].tx_bps*8,
            argsSpeed->entries[i].rx_bps*8,
            argsSpeed->entries[i].mib.mib_ip.tx_bytes*8,
            argsSpeed->entries[i].mib.mib_ip.rx_bytes*8);
    }
}

static void showTotal(unsigned char *mac_addr)
{
    int is_match_mac = 0;
    unsigned char __mac_addr[ETH_ALEN] = {0};
    struct tm *t;
    time_t tt;
    struct hwnat_mib_all_mac_total_args argsSpeed = {0};

    serialize_mac(__mac_addr, mac_addr);

    is_match_mac = is_valid_ether_addr(mac_addr);

    printf("is_match_mac:%d %02x:%02x:%02x:%02x:%02x:%02x\r\n",
            is_match_mac, NMACQUAD(__mac_addr));

    while (1)
    {
      sleep(1);
      time(&tt);
      t = localtime(&tt);
      if(ql_pop_lan_devices_packets_total_v2(&argsSpeed, __mac_addr, is_match_mac) != QL_STATUS_OK){
          continue;
      }
      printf("             start             ");
      printf("%2d/%02d/%04d %02d:%02d:%02d\n", t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec);

      /*SHOW IP/MAC tx_bytes & rx_bytes*/
      _dump_ip_mac_args_total_speed_data(&argsSpeed);
      printf("========\r\n\r\n");
    }

}
#endif

int main(int argc, char* argv[])
{
    char options[] = "haA";
    int	opt;
    unsigned char assign_mac[ETH_ALEN] = {0};

    printf("< Quectel OpenLinux: speed_monitor version:%s >\n", SPEED_MIT_VERSION);
    while ((opt = getopt (argc, argv, options)) != -1) {
        //printf("argc:%d opt %c\r\n", argc, opt);
        switch (opt) {
            case 'h':
                show_usage(argv);
                return 0;
            case 'A':
                set_open_debug(1);
            case 'a':
                if (argc == 3) {
                    if (str_to_mac(assign_mac, argv[2]) != 0) {
                        show_usage(argv);
                        return 0;
                    }
                }
                showTotal(assign_mac);
                return 0;
            default:
                break;
        }
    }

    show_usage(argv);
}

