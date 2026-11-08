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

#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/ipv6.h>

#ifdef JSONC
#include <json.h>
#else
#include <json-c/json.h>
#endif
#include <libubus.h>
#include <libubox/uloop.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <uci.h>

#include "mipc-wan-common.h"

#include "mipc_msg_host.h"
#include "mipc_msg_tlv_api.h"
#include "platform.h"
#include "quectel_atcid_sender.h"

#define JSON_ADD_OBJ(obj1, field, obj2, type) \
{ \
    json_object *tmp_obj = json_get_field(obj2, field, type); \
    if (tmp_obj) \
        json_object_object_add(obj1, field, tmp_obj); \
}

#define SIM_WAITING_TIME 30
#define NETWORK_WAITING_TIME 40
#define AT_URC_RESPONSE_TIME 10
#define GET_GLOBAL_IPV6_TIME 6

#define UCI_CONFIG_FILE "/etc/config/system"

#define esnprintf(...)\
{\
 int ret;\
 ret = snprintf(__VA_ARGS__);\
 if(ret < 0) LOGD("snprintf return error\n");\
}

#define MIPC_DFLT_BEARER_BITMASK 0xfffdffff

#define QL_GET_GLOBAL_IPV6

int g_log_hidden = 0;
int default_mtu = 1500;

static pthread_mutex_t event_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  event_queue_cond;

#ifdef _VOICE_SUPPORT
extern void test_mipc_conference_req_hdlr(void);
extern void test_mipc_dial_req_hdlr(int order, char* param);
extern void test_mipc_ss_req_hdlr(int mode);
extern void test_mipc_hangup_req_hdlr(void);
extern void test_mipc_answer_req_hdlr(void);
extern void test_mipc_get_call_status_req_hdlr(void);
extern void start_listen_dtmf(int mode);
#endif

/* Function prototype */
void nw_radio_sw_state_set(char* param, mipc_msg_sim_ps_id_enum sim);
void show_sim_status(mipc_msg_sim_ps_id_enum sim);

static bool check_sim_id(mipc_msg_sim_ps_id_enum sim) {
    if (sim == MIPC_SIM0 || sim == MIPC_SIM1 || sim == MIPC_PS0 || sim == MIPC_PS1) {
        return true;
    } else {
        LOGD("sim_id:%d Error\n", sim);
        return false;
    }
}

static const char *nw_ps_state_name(int state)
{
#define NW_PS_STATE_NAME(state) case state: return #state;
    switch(state)
    {
        NW_PS_STATE_NAME(MIPC_NW_PS_DETACH);
        NW_PS_STATE_NAME(MIPC_NW_PS_ATTACH);
    default:
        return "UNKNOWN_PS_STATE";
    }
#undef NW_PS_STATE_NAME
}

static const char *sim_state_name(int state)
{
#define SIM_STATE_NAME(state) case state: return #state;
    switch (state)
    {
        SIM_STATE_NAME(MIPC_SIM_STATE_UNKNOWN);
        SIM_STATE_NAME(MIPC_SIM_STATE_OFFEMPTY);
        SIM_STATE_NAME(MIPC_SIM_STATE_OFF);
        SIM_STATE_NAME(MIPC_SIM_STATE_EMPTY);
        SIM_STATE_NAME(MIPC_SIM_STATE_NOTREADY);
        SIM_STATE_NAME(MIPC_SIM_STATE_ACTIVE);
        SIM_STATE_NAME(MIPC_SIM_STATE_ERROR);
        SIM_STATE_NAME(MIPC_SIM_STATE_ACTIVE_ESIM);
        SIM_STATE_NAME(MIPC_SIM_STATE_ACTIVE_ESIM_NOPROFILE);
    default:
        return "UNKNOWN_SIM_STATE";
    }
#undef SIM_STATE_NAME
}

static const char *sim_status_name(int state)
{
#define SIM_STATUS_NAME(state) case state: return #state;
    switch (state)
    {
        SIM_STATUS_NAME(MIPC_SIM_STATUS_NOT_INSERT);
        SIM_STATUS_NAME(MIPC_SIM_STATUS_BUSY);
        SIM_STATUS_NAME(MIPC_SIM_STATUS_READY);
        SIM_STATUS_NAME(MIPC_SIM_STATUS_SIM_PIN);
        SIM_STATUS_NAME(MIPC_SIM_STATUS_SIM_PUK);
        SIM_STATUS_NAME(MIPC_SIM_STATUS_CARD_RESTRICTED);
        SIM_STATUS_NAME(MIPC_SIM_STATUS_EMPT_EUICC);
        SIM_STATUS_NAME(MIPC_SIM_STATUS_COMPLETE_READY);
    default:
        return "UNKNOWN_SIM_STATUS";
    }
#undef SIM_STATUS_NAME
}

static const char *sim_pin_state_name(int state)
{
#define SIM_PIN_STATE_NAME(state) case state: return #state;
    switch (state)
    {
        SIM_PIN_STATE_NAME(MIPC_SIM_PIN_STATE_UNLOCKED);
        SIM_PIN_STATE_NAME(MIPC_SIM_PIN_STATE_LOCKED);
    default:
        return "UNKNOWN_PIN_STATE";
    }
#undef SIM_PIN_STATE_NAME
}

static const char *radio_state_name(int state)
{
#define RADIO_STATE_NAME(state) case state: return #state;
    switch (state)
    {
        RADIO_STATE_NAME(MIPC_NW_RADIO_STATE_OFF);
        RADIO_STATE_NAME(MIPC_NW_RADIO_STATE_ON);
    default:
        return "UNKNOWN_RADIO_STATE";
    }
#undef RADIO_STATE_NAME
}

static const char *nw_register_state(int state)
{
#define NW_REG_STATE_NAME(state) case state: return #state;
    switch (state)
    {
        NW_REG_STATE_NAME(MIPC_NW_REGISTER_STATE_NOT_REGISTERED);
        NW_REG_STATE_NAME(MIPC_NW_REGISTER_STATE_HOME);
        NW_REG_STATE_NAME(MIPC_NW_REGISTER_STATE_SEARCHING);
        NW_REG_STATE_NAME(MIPC_NW_REGISTER_STATE_DENIED);
        NW_REG_STATE_NAME(MIPC_NW_REGISTER_STATE_UNKNOWN);
        NW_REG_STATE_NAME(MIPC_NW_REGISTER_STATE_ROAMING);
        NW_REG_STATE_NAME(MIPC_NW_REGISTER_STATE_HOME_SMS_ONLY);
        NW_REG_STATE_NAME(MIPC_NW_REGISTER_STATE_ROAMING_SMS_ONLY);
        NW_REG_STATE_NAME(MIPC_NW_REGISTER_STATE_ATTACHED_ECC_ONLY);
        NW_REG_STATE_NAME(MIPC_NW_REGISTER_STATE_HOME_CSFB_NOT_PREF);
        NW_REG_STATE_NAME(MIPC_NW_REGISTER_STATE_ROAMING_CSFB_NOT_PREF);
    default:
        return "UNKNOWN_NW_REG_STATE";
    }
#undef NW_REG_STATE_NAME
}

static inline json_object *
json_check_type(json_object *obj, json_type type)
{
    if (!obj)
        return NULL;

    if (json_object_get_type(obj) != type)
        return NULL;

    return obj;
}

static inline json_object *
json_get_field(json_object *obj, const char *name, json_type type)
{
    return json_object_object_get_ex(obj, name, &obj) ?
           json_check_type(obj, type) : NULL;
}

int get_int(json_object *obj, const char *name, int def_val)
{
    json_object *val_obj = json_get_field(obj, name, json_type_int);
    if (val_obj != NULL)
        return json_object_get_int(val_obj);

    return def_val;
}

char* get_string(json_object *obj, const char *name)
{
    json_object *val_obj = json_get_field(obj, name, json_type_string);
    if (val_obj != NULL)
        return (char*) json_object_get_string(val_obj);

    return NULL;
}

void add_json_int(json_object *dest_obj, json_object *src_obj, const char *field)
{
    int tmp_val = get_int(src_obj, field, 0xFFFF);

    if (tmp_val == 0xFFFF)
        return;

    json_object_object_add(dest_obj, field, json_object_new_int(tmp_val));
}

void add_json_string(json_object *dest_obj, json_object *src_obj, const char *field)
{
    char *tmp_str = get_string(src_obj, field);
    json_object *tmp_obj = NULL;

    if (tmp_str == NULL)
        return;

    tmp_obj = json_object_new_string(tmp_str);
    if (tmp_obj == NULL)
        return;

    json_object_object_add(dest_obj, field, tmp_obj);
}

static struct option cmd_opts[] =
{
    {"sim_mapping", optional_argument, 0, 'a'},
    {"apn_provision_by_sim", optional_argument, 0, 'b'},
    {"data_call_deact", required_argument, 0, 'c'},
    {"data_call_act", required_argument, 0, 'd'},
    {"apn_ia_get", optional_argument, 0, 'e'},
    {"at_cmd", required_argument, 0, 'f'},
    {"get_imeisv", no_argument, 0, 'g'},
    {"help", no_argument, 0, 'h'},
    {"sim_pin_info_get", optional_argument, 0, 'i'},
    {"modem_poweroff",  optional_argument, 0, 'j'},
    {"check_ia", required_argument, 0, 'k'},
    {"data_call_deact_apn", required_argument, 0, 'l'},
    {"sim_verify_pin", required_argument, 0, 'm'},
    {"nw_radio_state_get", optional_argument, 0, 'n'},
    {"check_nw_status", optional_argument, 0, 'o'},
    {"apn_ia_set", optional_argument, 0, 'p'},
    {"apn_profile_set", required_argument, 0, 'q'}, //telephony added
    {"apn_profile_get", optional_argument, 0, 'r'},
    {"sim_get_state", optional_argument, 0, 's'},
    {"data_call_get", optional_argument, 0, 't'},
    {"get_imei", no_argument, 0, 'u'},
    {"data_call_act_type", required_argument, 0, 'v'},
    {"nw_radio_state_set", required_argument, 0, 'w'},
    {"modem_poweroff_with_cause", required_argument, 0, 'x'},
    {"apn_ia_check", required_argument, 0, 'y'},
    {"get_sim_insert", no_argument, 0, 'z'},  //telephony added
    {"nw_change_hw_radio", optional_argument, 0, 'A'},  //telephony added
    {"nw_get_rat", optional_argument, 0, 'B'},  //telephony added
    {"nw_set_rat", required_argument, 0, 'C'},  //telephony added
    {"ims_get_config", optional_argument, 0, 'D'}, //telephony added
    {"ims_set_config", required_argument, 0, 'E'}, //telephony added
    {"ims_get_status", optional_argument, 0, 'F'}, //telephony added
    {"show_sim_status", optional_argument, 0, 'G'}, //telephony added
    {"show_register_status", optional_argument, 0, 'H'}, //telephony added
    {"dtmf_detect", no_argument, 0, 'I'},
    {"set_speech", required_argument, 0, 'J'},
    {"nw_show_register_rat", no_argument, 0, 'K'},  //telephony added
    {"nw_get_signal", optional_argument, 0, 'L'},  //telephony added
    {"at_cmd_old", required_argument, 0, 'M'},
    {"ql_set_radio_on", no_argument, 0, 'N'}, // add by Chris.Wu 2022.06.08 : Only used to automatically turn on the radio frequency during startup
    {"get_call_status", no_argument, 0, '1'},
    {"dial_request", optional_argument, 0, '2'},
    {"detect_and_dial", no_argument, 0, '3'},
    {"answer_ringing", no_argument, 0, '4'},
    {"answer_waiting", no_argument, 0, '5'},
    {"merge_request", no_argument, 0, '6'},
    {"play_tone", no_argument, 0, '7'},
    {"swap_request", no_argument, 0, '8'},
    {"hangup_foreground", no_argument, 0, '9'},
    {"hangup_all", no_argument, 0, '0'},
    {0, 0, 0, 0}
};

void addr4_to_str(char *addr, mipc_data_v4_addr_struct4 *srcAddr)
{
    memset(addr, 0, INET_ADDRSTRLEN);
    esnprintf(addr, INET_ADDRSTRLEN, "%u.%u.%u.%u", srcAddr->addr[0], srcAddr->addr[1], srcAddr->addr[2], srcAddr->addr[3]);
}

void addr6_to_str(char *addr, mipc_data_v6_addr_struct4 *srcAddr)
{
    memset(addr, 0, INET6_ADDRSTRLEN);
    esnprintf(addr, INET6_ADDRSTRLEN, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
            srcAddr->addr[0], srcAddr->addr[1], srcAddr->addr[2], srcAddr->addr[3],
            srcAddr->addr[4], srcAddr->addr[5], srcAddr->addr[6], srcAddr->addr[7],
            srcAddr->addr[8], srcAddr->addr[9], srcAddr->addr[10], srcAddr->addr[11],
            srcAddr->addr[12], srcAddr->addr[13], srcAddr->addr[14], srcAddr->addr[15]);
}

in_addr_t addr4_to_Ipv4Addr (mipc_data_v4_addr_struct4 *mipcAddr)
{
    in_addr_t addr = 0;

    if (mipcAddr == NULL) {
        return 0;
    }

    addr = (mipcAddr->addr[0] << 24) | 
           (mipcAddr->addr[1] << 16) | 
           (mipcAddr->addr[2] << 8) | 
           (mipcAddr->addr[3]);

    addr = htonl(addr);

    return(addr); 
}

in_addr_t prefixLengthToIpv4Netmask(int prefix_length)
{
    in_addr_t mask = 0;

    // C99 (6.5.7): shifts of 32 bits have undefined results
    if (prefix_length <= 0 || prefix_length > 32) {
        return 0;
    }

    mask = ~mask << (32 - prefix_length);
    mask = htonl(mask);

    return mask;
}

static void init_sockaddr_in(struct sockaddr *sa, in_addr_t addr)
{
    struct sockaddr_in *sin = (struct sockaddr_in *) sa;
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    sin->sin_addr.s_addr = addr;
}

static int system_get_sysctl(const char *path, char *buf, const size_t buf_sz)
{
    int fd, ret = -1;
    ssize_t len;

    fd = open(path, O_RDONLY);
    if (fd < 0)
        goto out;

    len = read(fd, buf, buf_sz - 1);
    if (len < 0)
        goto out;

    ret = buf[len] = 0;

out:
    if (fd >= 0)
        close(fd);

    return ret;
}

int system_get_bridge_mode()
{
    char buf[10];

    if (!system_get_sysctl("/proc/ccmni/bridge_mode_control",
        buf, sizeof(buf))) {
        LOGD("bridge_mode_control:%s\n", buf);
        return strtoul(buf, NULL, 0);
    }

    LOGD("Failed to get bridge_mode_control\n");
    return 0;
}

#ifdef CONFIG_ETHERNET_PDU
// EthPdu +
void system_get_ethernet_pdu_setting(int* eth_mode, int* eth_id)
{
    char *endptr;
    char buf[10];
    char *p_buf;
    char *p_token = NULL;
    char *p_delimiter = " \t";

    if (!system_get_sysctl("/proc/ccmni/eth_pdu_support",
            buf, sizeof(buf)))
    {
        LOGD("eth_pdu_support:%s\n", buf);

        p_buf = buf;
        p_token = strsep(&p_buf, p_delimiter);
        if (p_token)
            *eth_mode = (int)strtoul(p_token, &endptr, 10);

        if (*eth_mode == 1)
        {
            p_token = strsep(&p_buf, p_delimiter);
            if (p_token)
                *eth_id = (int)strtoul(p_token, &endptr, 10);
        }
    }
}
// EthPdu -
#endif

static void set_ipv6_default_route(char* ifname, int on)
{
    char *path = NULL;
    const char *value = on ? "1" : "0";
    unsigned int size = strlen(value);
    int fd;
    int ret;

    // full path: /proc/sys/net/ipv6/conf/ccmni0/accept_ra_defrtr
    ret = asprintf(&path, "/proc/sys/net/ipv6/conf/%s/%s", ifname, "accept_ra_defrtr");
    if (ret < 0) {
        LOGE("Failed to allocate path");
        return;
    }
    LOGD("set path %s to %s", path, value);
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        LOGE("Failed to open %s: %s", path, strerror(errno));
        goto err;
    }

    ret = write(fd, value, size);
    if (ret <= 0 || ((unsigned int)ret != size))
        LOGE("Failed to write %s: ret=%d, errno=%d:%s", path, ret, errno, strerror(errno));

err:
    if (fd >= 0)
        close(fd);
    free(path);
}

static int get_cfg_value(char *name, char *value, int value_buf_len) {
    int ret = 0;
    struct uci_context *uci_ctx = NULL;
    const char *value_data = NULL;
    struct uci_package *pkg = NULL;
    struct uci_element *e = NULL;

    if (NULL == name || NULL == value)
        return 0;

    uci_ctx = uci_alloc_context();
    if (!uci_ctx)
        return 0;

    if (UCI_OK != uci_load(uci_ctx, UCI_CONFIG_FILE, &pkg)) {
        LOGE( "uci_load(%s) fail!\n", UCI_CONFIG_FILE );
        goto cleanup;
    }
    uci_foreach_element(&pkg->sections, e) {
        struct uci_section *s = uci_to_section( e );
        if (NULL != (value_data = uci_lookup_option_string(uci_ctx, s, name))) {
            strncpy(value, value_data, value_buf_len - 1);
            value[value_buf_len - 1] = '\0';
            ret = 1;
        }
    }
    uci_unload(uci_ctx, pkg);

cleanup:
    uci_free_context(uci_ctx);
    uci_ctx = NULL;

    return ret;
}

/* Todo: Need to align netagent refactoring */
void ifc_update_addr(const char *name, mipc_data_v4_addr_struct4 *v4_addr, int prefixlen)
{
    int ifc_ctl_sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    struct ifreq ifr;
    in_addr_t addr, mask;

    int ret = 0;

    if (ifc_ctl_sock < 0) {
        LOGE("Create socket fail:%d\n", errno);
        return;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    ret = ioctl(ifc_ctl_sock, SIOCGIFADDR, &ifr);
    if (ret == 0) {
        close(ifc_ctl_sock);
        return;
    }

    addr = addr4_to_Ipv4Addr(v4_addr);
    init_sockaddr_in(&ifr.ifr_addr, addr);
    ret = ioctl(ifc_ctl_sock, SIOCSIFADDR, &ifr);
    if (ret < 0)
        LOGE("SIOCSIFADDR errno:%d\n", ret);
    else
        LOGD("Configure IPv4 addr\n");

    mask = prefixLengthToIpv4Netmask(prefixlen);
    init_sockaddr_in(&ifr.ifr_addr, mask);

    ret = ioctl(ifc_ctl_sock, SIOCSIFNETMASK, &ifr);
    if (ret < 0)
        LOGE("SIOCSIFNETMASK errno:%d\n", ret);
    else
        LOGD("Configure IPv4 netmask\n");

    close(ifc_ctl_sock);
}

void ifc_update_v6addr(const char *name, char *addr, int prefixlen)
{
    int ifc_ctl_sock = socket(AF_INET6, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    struct in6_ifreq ifr6;
    struct ifreq ifr;
    int ret = 0;

    if (ifc_ctl_sock < 0) {
        LOGE("Create socket fail:%d\n", errno);
        return;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    ret = ioctl(ifc_ctl_sock, SIOCGIFINDEX, &ifr);
    if (ret < 0) {
        LOGE("SIOCGIFINDEX fail:%d\n", errno);
        close(ifc_ctl_sock);
        return;
    }

    memset(&ifr6, 0, sizeof(ifr6));
    ret = inet_pton(AF_INET6, addr, &ifr6.ifr6_addr);
    if (ret <= 0) {
        LOGE("inet_pton:%d\n", errno);
        close(ifc_ctl_sock);
        return;
    }
    ifr6.ifr6_prefixlen = prefixlen;
    ifr6.ifr6_ifindex = ifr.ifr_ifindex;

    ret = ioctl(ifc_ctl_sock, SIOCSIFADDR, &ifr6);
    if (ret < 0)
        LOGE("SIOCSIFADDR errno:%d\n", ret);
    else if (g_log_hidden)
        LOGD("Configure IPv6 addr done for %s\n", name);
    else
        LOGD("Configure IPv6 addr done for %s:%s\n", name, addr);

    close(ifc_ctl_sock);
}

void get_apn_by_sim(int mcc, int mnc)
{
    int fd;
    struct stat sb;
    struct json_object *json, *obj;
    char *buff, plmn[7] = {'\0'};

#ifdef CONFIG_ETHERNET_PDU
    // EthPdu +
    int eth_mode = 0;
    int eth_id = 0;

    system_get_ethernet_pdu_setting(&eth_mode, &eth_id);

    if (eth_mode == 1 && eth_id > 1 && eth_id < 20)
    {
        LOGD("Load ethernet apn\n");
        fd = open("/etc/ethernet_apn.json", O_RDONLY);
    }
    else
#endif
    {
        LOGD("Load default apn\n");
        fd = open("/etc/default_apn.json", O_RDONLY);
    }
    //fd = open("/etc/default_apn.json", O_RDONLY);
    // EthPdu -

    if (fd < 0)
    {
        LOGE("Fail to open file:%d\n", errno);
        return;
    }
    if (fstat(fd, &sb) < 0)
    {
        LOGE("Fail to get file stat:%d\n", errno);
        goto err;
    }
    buff = (char*) mmap(0, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (buff == MAP_FAILED)
    {
        LOGE("Fail to get file stat:%d\n", errno);
        goto err;
    }

    json = json_tokener_parse(buff);
    if (json != NULL)
    {
        int len = json_object_array_length(json);

        for (int i = 0; i < len; i++)
        {
            json_object *mcc_obj, *mnc_obj;

            obj = json_object_array_get_idx(json, i);
            mcc_obj = json_get_field(obj, "mcc", json_type_int);
            mnc_obj = json_get_field(obj, "mnc", json_type_int);
            if (mcc_obj == NULL || mnc_obj == NULL)
                continue;

            int tmp_mcc = json_object_get_int(mcc_obj);
            int tmp_mnc = json_object_get_int(mnc_obj);
            if (tmp_mcc == mcc && tmp_mnc == mnc)
            {
                json_object *pdn_obj = json_object_new_object();
                int n;
                add_json_string(pdn_obj, obj, "apn");
                add_json_int(pdn_obj,obj, "protocol");
                add_json_int(pdn_obj, obj, "roaming_protocol");
                add_json_string(pdn_obj, obj, "iaapn");
                add_json_int(pdn_obj, obj, "iaprotocol");
                add_json_int(pdn_obj, obj, "iaroaming_protocol");
                add_json_int(pdn_obj, obj, "auth_type");
                add_json_string(pdn_obj, obj, "user");
                add_json_string(pdn_obj, obj, "password");
                add_json_int(pdn_obj, obj, "mtu");

                n = snprintf(plmn, sizeof(plmn), "%d%03d", tmp_mcc, tmp_mnc);
                if (n < 0 || n >= sizeof(plmn))
                {
                    LOGE("snprintf error\n");
                    json_object_put(json);
                    munmap(buff, sb.st_size);
                    goto err;
                }
                json_object_object_add(pdn_obj, "plmn", json_object_new_string(plmn));

                printf("%s\n", json_object_to_json_string(pdn_obj));
                json_object_put(pdn_obj);
                LOGD("APN is found\n");
                break;
            }
        }
        json_object_put(json);
    } else {
        LOGE("Fail to load json");
    }
    munmap(buff, sb.st_size);

err:
    close(fd);
}

void update_apn_profile(uint8_t sim)
{
    int ret; 
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;

    if (!check_sim_id(sim))
        return;
    
    msg_req_ptr = mipc_msg_init(MIPC_SIM_IMSI_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    LOGD("update_apn_profile for SIM%d\n", sim);
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);
    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        char* imsi = mipc_sim_imsi_cnf_get_imsi(msg_cnf_ptr, NULL);
        int mcc, mnc;
        uint8_t mnc_len = 0;
        
        mnc_len = mipc_sim_imsi_cnf_get_mnc_len(msg_cnf_ptr, 0);   
        if (mnc_len == 3) { 
            ret = sscanf(imsi, "%3d%3d", &mcc, &mnc);
        } else if (mnc_len == 2) { 
            ret = sscanf(imsi, "%3d%2d", &mcc, &mnc);
        } else {
            LOGD("No mnc length imsi:%s mnc_len:%d\n", imsi, mnc_len);
            ret = sscanf(imsi, "%3d%2d", &mcc, &mnc);
        }

        if (ret != 2){ 
            LOGD("sscanf return error\n");
        } else {
            LOGD("MCC:%3d MNC:%03d mnc_len:%d\n", mcc, mnc, mnc_len);
            get_apn_by_sim(mcc, mnc);
        }
    }

    mipc_msg_deinit(msg_cnf_ptr);
}

void sim_status_cb(mipc_msg_t *msg_ptr, void *priv)
{
    mipc_sim_status_const_enum sim_status = mipc_sim_status_ind_get_status(msg_ptr, mipc_sim_status_const_NONE);
    int ret = 0;

    LOGD("SIM status:%s\n", sim_status_name(sim_status));
    if ((sim_status == MIPC_SIM_STATUS_READY) ||
            (sim_status == MIPC_SIM_STATUS_COMPLETE_READY)) {
            ret = pthread_mutex_lock(&event_queue_mutex);
            if (ret != 0) {
                LOGE("Error locking mutex: %s\n", strerror(ret));
                return;
            }
            ret = pthread_cond_signal(&event_queue_cond);
            pthread_mutex_unlock(&event_queue_mutex);
            if (ret != 0) {
                LOGE("Error cond siganl: %s\n", strerror(ret));
            }
    }
}

mipc_sim_status_const_enum sim_get_status(mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result = MIPC_RESULT_FAILURE;
    //modified by Quectel
    //mipc_sim_status_const_enum sim_status = MIPC_SIM_STATUS_NOT_INSERT;
    mipc_sim_status_const_enum sim_status = mipc_sim_status_const_NONE;

    if (!check_sim_id(sim))
        return sim_status;

    msg_req_ptr = mipc_msg_init(MIPC_SIM_STATUS_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return sim_status;
    }
    mipc_msg_add_tlv_uint8(msg_req_ptr, MIPC_SIM_STATUS_REQ_T_MODE, 1);
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
        //modified by Quectel
        //sim_status = mipc_sim_status_cnf_get_status(msg_cnf_ptr, MIPC_SIM_STATUS_BUSY);
        sim_status = mipc_sim_status_cnf_get_status(msg_cnf_ptr, mipc_sim_status_const_NONE);
    else
        LOGE("Failed to execute with SIM%d sim_get_status:%d\n", sim, result);
    mipc_msg_deinit(msg_cnf_ptr);

    LOGD("sim=%d result=%d sim_status=%d\n", sim, result, sim_status );
    return sim_status;
}

static void mipc_at_ind_cb(mipc_msg_t *msg_ptr, void *priv_ptr)
{
    char *urc_ptr;
    uint16_t urc_len = 0;
    uint8_t  urc = 0;
    int ret = 0;

    urc_ptr = mipc_sys_at_ind_get_atcmd(msg_ptr, &urc_len);
    if (urc_ptr == NULL) {
        LOGD("urc_ptr return NULL\n");
        return; 
    }else {
        LOGD("[%d]%s", urc_len, urc_ptr);
        printf("%s", urc_ptr);

        /* Check the expected URC result */
        urc = (strstr(urc_ptr, "+ENWINFO") != NULL) ? 1 : 0;
        if (urc == 0)
            return;
    }

    ret = pthread_mutex_lock(&event_queue_mutex);
    if (ret != 0) {
        LOGE("Error locking mutex: %s\n", strerror(ret));
        return;
    }
    ret = pthread_cond_signal(&event_queue_cond);
    pthread_mutex_unlock(&event_queue_mutex);
    if (ret != 0) {
        LOGE("Error cond siganl: %s\n", strerror(ret));
    }
}

void run_at_cmd(char* cmd, mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;
    uint16_t atcmd_res_len;
    uint8_t  urc = 0;
    struct timespec timeout;
    int ret;

    if (!check_sim_id(sim))
        return;

    msg_req_ptr = mipc_msg_init(MIPC_SYS_AT_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    urc = (strstr(cmd, "+EINFO") != NULL) ? 1 : 0;
    LOGD("AT command execution:%s:%d\n", cmd, urc);

    if (urc)
        mipc_msg_register_ind(sim, MIPC_SYS_AT_IND, (void*)mipc_at_ind_cb, NULL);

    mipc_msg_add_tlv(msg_req_ptr, MIPC_SYS_AT_REQ_T_ATCMD, strlen(cmd), cmd);
    mipc_msg_set_timeout_once(5*1000);
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS) {
        char *atcmd_res_ptr = mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_SYS_AT_CNF_T_ATCMD, &atcmd_res_len);
        printf("AT response:%s\n", atcmd_res_ptr);

        if (urc) {
            clock_gettime(CLOCK_MONOTONIC, &timeout);
            timeout.tv_sec = timeout.tv_sec + AT_URC_RESPONSE_TIME;
            timeout.tv_nsec = 0;
            ret = pthread_mutex_lock(&event_queue_mutex);
            if (ret != 0) {
                LOGE("Error locking mutex: %s\n", strerror(ret));
                goto exit;
            }
            ret = pthread_cond_timedwait(&event_queue_cond, &event_queue_mutex, &timeout);
            pthread_mutex_unlock(&event_queue_mutex);
            if (ret == ETIMEDOUT) {
                LOGD("Timeout for URC response\n");
                printf("After %d seconds, no URC response from modem\n", AT_URC_RESPONSE_TIME);
            } else if (ret) {
                LOGD("Error for network wait:%d\n", ret);
                printf("Error:%d when waiting for URC response\n", ret);
            }
        }
    } else {
        printf("Failed to execute:%d\n", result);
    }

    if (urc)
        mipc_msg_register_ind(sim, MIPC_SYS_AT_IND, NULL, NULL);
exit:
    mipc_msg_deinit(msg_cnf_ptr);
}

void quectel_run_at_cmd(char *cmd)
{
    COMM_DEV *pcomm_dev = NULL;
	atcmd_sender_msg_t m_atcmd_sender;
	int ret = 0;

	pcomm_dev = ql_atcid_sender_init(TYPE_ADB_ATCI_SOCKET);
	memset(&m_atcmd_sender, 0, sizeof(atcmd_sender_msg_t));
	strncpy(m_atcmd_sender.at_cmd, cmd, strlen(cmd));
	strncat(m_atcmd_sender.at_cmd, "\r\n", strlen("\r\n")+1);
	m_atcmd_sender.timeout = 300;
	
	ret = ql_atcid_sender_sending(pcomm_dev, &m_atcmd_sender);
	if (ret > 0)
	{
		if(m_atcmd_sender.at_recv != NULL)
		{
			printf("AT response:%s\n", m_atcmd_sender.at_recv);
			free(m_atcmd_sender.at_recv);
		}
	}
    else
    {
        printf("Failed to execute AT.\n");
    }
	ql_atcid_sender_deinit(pcomm_dev);
}

mipc_sim_state_const_enum sim_get_state(bool is_dump, mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;
    mipc_sim_state_const_enum sim_state = mipc_sim_state_const_NONE;

    if (!check_sim_id(sim))
        return sim_state;

    msg_req_ptr = mipc_msg_init(MIPC_SIM_STATE_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return sim_state;
    }
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        sim_state = mipc_sim_state_cnf_get_state(msg_cnf_ptr, MIPC_SIM_STATE_UNKNOWN);

        LOGD("SIM state:%s\n", sim_state_name(sim_state));
        if (is_dump)
            printf("%s", sim_state_name(sim_state));
    } else {
        LOGE("Failed to execute with MIPC_SIM_STATE_REQ:%d\n", result);
        switch (result) {
            case MIPC_RESULT_SIM_NOT_INSERTED:
                if (is_dump)
                    printf("MIPC_RESULT_SIM_NOT_INSERTED");
                sim_state = MIPC_SIM_STATE_EMPTY;
                break;
            case MIPC_RESULT_BAD_SIM:
                if (is_dump)
                    printf("MIPC_RESULT_BAD_SIM");
                sim_state = MIPC_SIM_STATE_ERROR;
            default:
                break;
        }
    }
    mipc_msg_deinit(msg_cnf_ptr);

    return sim_state;
}

void apn_provision_by_sim(mipc_msg_sim_ps_id_enum sim)
{
    int is_sim_ready = 0, ret;
    struct timespec timeout;
    mipc_sim_status_const_enum sim_status = MIPC_SIM_STATUS_NOT_INSERT;

    if (!check_sim_id(sim))
        return;
    
    sim_status = sim_get_status(sim);

    if ((sim_status == MIPC_SIM_STATUS_READY) ||
                (sim_status == MIPC_SIM_STATUS_COMPLETE_READY)) {
        update_apn_profile(sim);
        return;
    } else if (sim_status == MIPC_SIM_STATUS_NOT_INSERT) {
        LOGD("SIM%d is not inserted", sim);
        return;
    }

    clock_gettime(CLOCK_MONOTONIC, &timeout);
    timeout.tv_sec = timeout.tv_sec + SIM_WAITING_TIME;
    timeout.tv_nsec = 0;

    ret = pthread_mutex_lock(&event_queue_mutex);
    if (ret != 0) {
       LOGE("Error locking mutex: %s\n", strerror(ret));
       return;
    }
    mipc_msg_register_ind(sim, MIPC_SIM_STATUS_IND, sim_status_cb, &is_sim_ready);
    ret = pthread_cond_timedwait(&event_queue_cond, &event_queue_mutex, &timeout);
    pthread_mutex_unlock(&event_queue_mutex);
    if (ret == ETIMEDOUT)
        LOGD("Timeout to wait for SIM%d ready\n", sim);
    else if (ret)
        LOGD("Error for sim wait:%d\n", errno);
    else
        update_apn_profile(sim);

    mipc_msg_register_ind(sim, MIPC_SIM_STATUS_IND, NULL, NULL);
}

void get_sim_insert()
{
    uint8_t sim_insert = 0;
    mipc_sim_status_const_enum sim_status;
    mipc_msg_sim_ps_id_enum sim;

    // sim_insert=0 No SIM insert
    //            1 SIM1 insert
    //            2 SIM2 insert
    //            3 SIM1 and SIM2 both insert
    for (sim = 1; sim < 4; sim = sim << 1) {

        sim_status = sim_get_status(sim);

        if (sim_status != MIPC_SIM_STATUS_NOT_INSERT) {
            sim_insert += sim;
        }
    }
    LOGD("sim_insert=%d\n", sim_insert);
    printf("%d", sim_insert);
}
void sim_get_mapping()
{
    mipc_msg_t *msg_req_ptr = mipc_msg_init(MIPC_SYS_GET_MAPPING_REQ, MIPC_PS0);
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;
    uint8_t i;
    mipc_sys_mapping_struct4 *mapping_list;

    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        uint8_t count = mipc_sys_get_mapping_cnf_get_mapping_count(msg_cnf_ptr, 0);
        LOGD("Mapping count:%d\n", count);
        mapping_list = mipc_sys_get_mapping_cnf_get_mapping_list(msg_cnf_ptr, NULL);
        for (i = 0; i < count; i++){
            if (i == 0)
                printf("%d", mapping_list->ps_id);
            LOGD("SIM mapping list:SIM%d -> PS%d\n", i, mapping_list->ps_id);
            mapping_list++;
        }
    }
    else
        LOGE("Failed to execute with  MIPC_SYS_GET_MAPPING_REQ:%d\n", result);
    mipc_msg_deinit(msg_cnf_ptr);
}

void sim_set_mapping(char* ps_id)
{
    mipc_msg_t *msg_req_ptr = mipc_msg_init(MIPC_SYS_SET_MAPPING_REQ, MIPC_PS0);
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;
    uint8_t count, i;
    int8_t id;
    long val;
    char *endptr;
    errno = 0;
    mipc_sys_mapping_struct4 mapping_list[2], *done_list;

    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    errno = 0;
    val = strtol(ps_id, &endptr, 10);
    if (errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) {
        LOGD("strtol return error\n");
        mipc_msg_deinit(msg_req_ptr);
        return;
    }
    id = (int8_t)val; 
    if (ps_id == endptr || id < 0) {
        LOGE("Can't get the valid PS ID:%s:%d errno:%d\n", optarg, id, errno);
        mipc_msg_deinit(msg_req_ptr);
        return;
    }
    if (id != 0 && id != 1) {
        LOGE("The PS id should be 0 or 1, not %d\n", id);
        mipc_msg_deinit(msg_req_ptr);
        return;
    }

    /* Radio off before update SIM mapping */
    nw_radio_sw_state_set("0", MIPC_PS0);

    mipc_sys_set_mapping_req_add_mapping_count(msg_req_ptr, 2);
    mapping_list[0].ps_id = id;
    mapping_list[1].ps_id = (id == 0) ? 1 : 0;
    mipc_msg_add_tlv(msg_req_ptr, MIPC_SYS_SET_MAPPING_REQ_T_MAPPING_LIST, sizeof(mipc_sys_mapping_struct4) * 2, (const void *)&mapping_list);

    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        count = mipc_sys_set_mapping_cnf_get_mapping_count(msg_cnf_ptr, 0);
        LOGD("Mapping count:%d\n", count);
        done_list = mipc_sys_set_mapping_cnf_get_mapping_list(msg_cnf_ptr, NULL);
        for (i = 0; i < count; i++){
            printf("SIM mapping list:SIM%d -> PS%d\n", i, done_list->ps_id);
            LOGD("SIM mapping list:SIM%d -> PS%d\n", i, done_list->ps_id);
            done_list++;
        }
    }
    else
        LOGE("Failed to execute with  MIPC_SYS_SET_MAPPING_REQ:%d\n", result);
    mipc_msg_deinit(msg_cnf_ptr);

    /* Radio on after update SIM mapping */
    nw_radio_sw_state_set("1", MIPC_PS0);
}

void sim_pin_info_get(mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;
    mipc_sim_pin_type_const_enum pin_type;
    mipc_sim_pin_state_const_enum pin_state;
    uint32_t count;

    if (!check_sim_id(sim))
        return;
    
    msg_req_ptr = mipc_msg_init(MIPC_SIM_GET_PIN_INFO_REQ, sim);

    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    /* Todo */
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        pin_type = mipc_sim_get_pin_info_cnf_get_pin_type(msg_cnf_ptr, mipc_sim_pin_type_const_NONE);
        LOGD("SIM%d PIN Type:%d\n", sim, pin_type);
        pin_state = mipc_sim_get_pin_info_cnf_get_pin_state(msg_cnf_ptr, 0xFF);
        printf("PIN State:%s\n", sim_pin_state_name(pin_state));
        count = mipc_sim_get_pin_info_cnf_get_remaining_attempts(msg_cnf_ptr, 0xFF);
        LOGD("Attemps:%d\n", count);
    }
    else
        LOGE("Failed to execute with  MIPC_SIM_VERIFY_PIN_REQ:%d\n", result);
    mipc_msg_deinit(msg_cnf_ptr);
}

void sim_verify_pin(char* pin_code, mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;
    mipc_sim_pin_type_const_enum pin_type;
    mipc_sim_pin_state_const_enum pin_state;
    uint32_t count;

    if (strlen(pin_code) != 4)
    {
        printf("ERROR\n");
        return;
    }

    if (!check_sim_id(sim))
        return;

    if (sim_get_status(sim) == MIPC_SIM_STATUS_READY) {
        printf("SIM already ready.\n");
        return;
    }
    msg_req_ptr = mipc_msg_init(MIPC_SIM_VERIFY_PIN_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    if (sim_get_status(sim) == MIPC_SIM_STATUS_SIM_PIN) {
        mipc_sim_verify_pin_req_add_pin_type(msg_req_ptr, MIPC_SIM_PIN_TYPE_PIN1);
    } else {
        printf("Only support to unlock SIM%d PIN\n", sim);
        mipc_msg_deinit(msg_req_ptr);
        return;
    }
    mipc_sim_verify_pin_req_add_pin_code(msg_req_ptr, strlen(pin_code), pin_code);
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        pin_type = mipc_sim_verify_pin_cnf_get_pin_type(msg_cnf_ptr, mipc_sim_pin_type_const_NONE);
        LOGD("PIN Type:%d\n", pin_type);
        pin_state = mipc_sim_verify_pin_cnf_get_pin_state(msg_cnf_ptr, 0xFF);
        printf("PIN State:%s\n", sim_pin_state_name(pin_state));
        count = mipc_sim_verify_pin_cnf_get_remaining_attempts(msg_cnf_ptr, 0xFF);
        LOGD("Attemps:%d\n", count);
    }
    else
        LOGE("Failed to execute with  SIM%d MIPC_SIM_VERIFY_PIN_REQ:%d\n", sim, result);
    mipc_msg_deinit(msg_cnf_ptr);
}

int nw_radio_state_get(int hw_radio, mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;
    mipc_nw_radio_state_const_enum radio_state = MIPC_NW_RADIO_STATE_OFF;

    if (!check_sim_id(sim))
        return radio_state;

    msg_req_ptr = mipc_msg_init(MIPC_NW_GET_RADIO_STATE_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return radio_state;
    }
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        if (hw_radio == 1) {
            radio_state = mipc_nw_get_radio_state_cnf_get_hw_state(msg_cnf_ptr, MIPC_NW_RADIO_STATE_OFF);
            LOGD("SIM%d HW radio state:%s\n", sim, radio_state_name(radio_state));
        } else {
            radio_state = mipc_nw_get_radio_state_cnf_get_sw_state(msg_cnf_ptr, MIPC_NW_RADIO_STATE_OFF);
            printf("SIM%d SW radio state:%s\n", sim, radio_state_name(radio_state));
            LOGD("SIM%d SW radio state: %s\n", sim, radio_state_name(radio_state));
        }
    }
    else
        LOGE("SIM%d Failed to execute with MIPC_NW_GET_RADIO_STATE_REQ:%d\n", sim, result);
    mipc_msg_deinit(msg_cnf_ptr);

    return radio_state;
}

void nw_radio_sw_state_set(char* param, mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_nw_radio_state_const_enum prev_radio_state;
    mipc_nw_radio_state_const_enum radio_state = MIPC_NW_RADIO_STATE_OFF;
    mipc_result_enum result;
    int retry = 0;

    if (param[0] == '1')
        radio_state = MIPC_NW_RADIO_STATE_ON;

    if (!check_sim_id(sim))
        return;

    prev_radio_state = nw_radio_state_get(0, sim);
    LOGD("set sim%d radio state:%s current radio state:%s\n", sim, radio_state_name(radio_state), radio_state_name(prev_radio_state));
    if (prev_radio_state == radio_state)
        return;
retry:
    retry++;
    msg_req_ptr = mipc_msg_init(MIPC_NW_SET_RADIO_STATE_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    mipc_nw_set_radio_state_req_add_sw_state(msg_req_ptr, radio_state);
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        mipc_nw_radio_state_const_enum sw_radio_state = mipc_nw_get_radio_state_cnf_get_sw_state(msg_cnf_ptr, MIPC_NW_RADIO_STATE_OFF);
        mipc_nw_radio_state_const_enum hw_radio_state = mipc_nw_get_radio_state_cnf_get_hw_state(msg_cnf_ptr, MIPC_NW_RADIO_STATE_OFF);
        LOGD("radio state SIM%d SW:%s HW:%s\n", sim, radio_state_name(sw_radio_state), radio_state_name(hw_radio_state));
    }
    else
    {
        LOGE("Failed to execute with SIM%d MIPC_NW_SET_RADIO_STATE_REQ:%d\n", sim, result);
        if(retry < 3)
        {
            goto retry;
        }
    } 
    mipc_msg_deinit(msg_cnf_ptr);
}

void apn_ia_set(char* apn, int ip_type, int roaming_type, 
                char* userid, char* password, int auth_type, uint32_t bearer_bitmask, mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;
    uint8_t apn_count;

    if (!check_sim_id(sim))
        return;

    msg_req_ptr = mipc_msg_init(MIPC_APN_SET_IA_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    if (apn == NULL)
        mipc_apn_set_ia_req_add_apn(msg_req_ptr, MIPC_MAX_APN_LEN, "");
    else
    mipc_apn_set_ia_req_add_apn(msg_req_ptr, MIPC_MAX_APN_LEN, apn);
    mipc_apn_set_ia_req_add_pdp_type(msg_req_ptr, ip_type);
    mipc_apn_set_ia_req_add_roaming_type(msg_req_ptr, roaming_type);

    mipc_apn_set_ia_req_add_auth_type(msg_req_ptr, auth_type);
    mipc_apn_set_ia_req_add_userid(msg_req_ptr, MIPC_MAX_USERID_LEN, userid);
    mipc_apn_set_ia_req_add_password(msg_req_ptr, MIPC_MAX_PASSWORD_LEN, password);

    mipc_apn_set_ia_req_add_bearer_bitmask(msg_req_ptr, bearer_bitmask);
    mipc_apn_set_ia_req_add_compression(msg_req_ptr, MIPC_APN_COMPRESSION_ENABLE);

    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        apn_count = mipc_apn_set_ia_cnf_get_ia_count(msg_cnf_ptr, 0);
        LOGD("SIM%d IA APN count:%d\n", sim, apn_count);
        if (apn_count > 0)
        {
            mipc_apn_ia_struct4* ia = mipc_apn_get_ia_cnf_get_ia_list(msg_cnf_ptr, NULL);
            LOGD("IA apn:%s pdp_type:%d roaming_type:%d bearer_bitmask:0x%x\n", 
                 ia->apn, ia->pdp_type, ia->roaming_type, ia->bearer_bitmask);
        }
    }
    else
        LOGE("Failed to execute with  MIPC_APN_SET_IA_REQ:%d\n", result);
    mipc_msg_deinit(msg_cnf_ptr);
}

int apn_ia_get(char* apn, int* ip_type, int* roaming_type, mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;

    if (!check_sim_id(sim))
        return 0;

    msg_req_ptr = mipc_msg_init(MIPC_APN_GET_IA_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return 0;
    }
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        uint8_t apn_count = mipc_apn_set_ia_cnf_get_ia_count(msg_cnf_ptr, 0);

        LOGD("SIM%d IA APN count:%d\n", sim, apn_count);
        if (apn_count > 0)
        {
            mipc_apn_ia_struct4* ia = mipc_apn_get_ia_cnf_get_ia_list(msg_cnf_ptr, NULL);
            if (apn != NULL) {
                memset(apn, 0, MIPC_MAX_APN_LEN);
                strncpy(apn, ia->apn, MIPC_MAX_APN_LEN - 1);
                apn[MIPC_MAX_APN_LEN - 1] = '\0';
            }
            *ip_type = ia->pdp_type;
            *roaming_type = ia->roaming_type;

            printf("IA apn:%s pdp_type:%d roaming_type:%d bearer_bitmask:0x%x\n", 
                   ia->apn, ia->pdp_type, ia->roaming_type, ia->bearer_bitmask);
            LOGD("IA apn:%s pdp_type:%d roaming_type:%d bearer_bitmask:0x%x\n", 
                   ia->apn, ia->pdp_type, ia->roaming_type, ia->bearer_bitmask);
            mipc_msg_deinit(msg_cnf_ptr);
            return apn_count;
        }
    }
    else
    {
        LOGE("Failed to execute with  MIPC_APN_GET_IA_REQ:%d\n", result);
        printf("Error to get IA\n");
    }
    mipc_msg_deinit(msg_cnf_ptr);

    return 0;
}

void apn_profile_get(mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;

    if (!check_sim_id(sim))
        return;

    msg_req_ptr = mipc_msg_init(MIPC_APN_LIST_PROFILE_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        uint8_t apn_count = mipc_apn_list_profile_cnf_get_apn_count(msg_cnf_ptr, 0);

        printf("APN Profile count:%d\n", apn_count);
        while (apn_count)
        {
            mipc_apn_profile_struct4* profile = mipc_apn_list_profile_cnf_get_apn_list(msg_cnf_ptr, NULL);
            printf("APN Profile[%d]:%s pdp_type:%d apn_type:%d", apn_count, profile->apn, profile->pdp_type, profile->apn_type);
            apn_count--;
            profile++;
        }
    }
    else
    {
        LOGE("Failed to execute with  MIPC_APN_LIST_PROFILE_REQ:%d\n", result);
        printf("Error to get APN profile\n");
    }
    mipc_msg_deinit(msg_cnf_ptr);
}

static void data_call_get_ipv6_dnses(mipc_msg_t *msg_cnf_ptr, json_object *pdn_obj, int v6_dns_count)
{
    char v6_dns[2][INET6_ADDRSTRLEN+1];
    mipc_data_v6_addr_struct4* addr = mipc_data_act_call_cnf_get_dns_v6_0(msg_cnf_ptr, NULL);

    addr6_to_str(v6_dns[0], addr);
    json_object_object_add(pdn_obj, "v6_dns1", json_object_new_string(v6_dns[0]));
    v6_dns_count--;
    if (v6_dns_count > 0) {
        addr = mipc_data_act_call_cnf_get_dns_v6_1(msg_cnf_ptr, NULL);
        addr6_to_str(v6_dns[1], addr);
        json_object_object_add(pdn_obj, "v6_dns2", json_object_new_string(v6_dns[1]));
    }
}

static inline int is_non_linklocal(const char* ipv6)
{
     if (strncasecmp("FE80", ipv6, strlen("FE80")) == 0) {
         return 0;
     }

     return 1;
}

static void data_call_get_ipv6_addr(mipc_msg_t *msg_cnf_ptr, json_object *pdn_obj, char* ifname, int check_v6_gw)
{
    char v6_addr[INET6_ADDRSTRLEN+1];
    int  netmask;
    mipc_data_v6_addr_struct4* addr = mipc_data_act_call_cnf_get_v6_0(msg_cnf_ptr, NULL);

    netmask = mipc_data_act_call_cnf_get_ipv6_netmask(msg_cnf_ptr, 64);
    LOGD("IPv6 address netmask: %d\n", netmask);

    addr6_to_str(v6_addr, addr);

    /* To get the global IPv6 address in routing mode */
    if (check_v6_gw) {
        /* Learn default router in Router Advertisement */
        set_ipv6_default_route(ifname, 1);
        ifc_update_v6addr(ifname, v6_addr, netmask);
        if (get_global_ipv6_addr(ifname, v6_addr) != 0) {
            if (check_global_ipv6_addr(ifname, v6_addr) != 0) {
                get_global_ipv6_addr(ifname, v6_addr);
            }
		}
    } else {
        set_ipv6_default_route(ifname, 0);
    }

    json_object_object_add(pdn_obj, "v6_addr", json_object_new_string(v6_addr));
    json_object_object_add(pdn_obj, "v6_prefix", json_object_new_int(64));

    /*
     * This is a mandatory information for clatd.
     * For clatd, it will send below ubus command to add dependency.
     * ubus call network.interface notify_proto '{ "action": 6, "host": "::", "ifname": "wan", "interface": "clatd" }'
     * Need to find :: in the routing information of wan interface
     */
    get_ipv6_default_gw(ifname, v6_addr);
    if (strlen(v6_addr) > 0)
            json_object_object_add(pdn_obj, "v6_gw", json_object_new_string(v6_addr));
}

static void data_call_get_ipv4_dnses(mipc_msg_t *msg_cnf_ptr, json_object *pdn_obj, int v4_dns_count)
{
    char v4_dns[2][INET_ADDRSTRLEN+1];
    mipc_data_v4_addr_struct4* addr = mipc_data_act_call_cnf_get_dns_v4_0(msg_cnf_ptr, NULL);

    addr4_to_str(v4_dns[0], addr);
    json_object_object_add(pdn_obj, "v4_dns1", json_object_new_string(v4_dns[0]));
    v4_dns_count--;
    if (v4_dns_count > 0) {
        addr = mipc_data_act_call_cnf_get_dns_v4_1(msg_cnf_ptr, NULL);
        addr4_to_str(v4_dns[1], addr);
        json_object_object_add(pdn_obj, "v4_dns2", json_object_new_string(v4_dns[1]));
    }
}

static int cal_odu_info(int pdn_addr, int* gw_addr, int* odu_addr, int* netmask)
{
    int base_offset = 8;
    int tmp_addr;

    *netmask = 29;
    while (base_offset <= 256) {
        tmp_addr = pdn_addr % base_offset;

        /* all zero & one are reserved addresses for subnet */
        if ( tmp_addr == 0 || (tmp_addr == (base_offset - 1)) ) {
            base_offset *= 2;
            *netmask = *netmask - 1;
            continue;
        }

        /* Make sure gateway & odu addresses in the subnet */
        if (tmp_addr + 2 < base_offset) {
            *gw_addr = pdn_addr + 1;
            *odu_addr = pdn_addr + 2;
        } else {
            *gw_addr = pdn_addr - 1;
            *odu_addr = pdn_addr - 2;
        }
        return 1;
    }

    return 0;
}

static void data_call_get_ipv4_addr(mipc_msg_t *msg_cnf_ptr, json_object *pdn_obj, char* ifname, int check_v4_addr)
{
    char v4_addr[INET_ADDRSTRLEN+1];
    char v4_gw[INET_ADDRSTRLEN+1];
    int gw_addr = 254, odu_addr = 253, netmask = 24;
    int dhcp_offset = 0;
    mipc_data_v4_addr_struct4* addr = mipc_data_act_call_cnf_get_v4_0(msg_cnf_ptr, NULL);

    addr4_to_str(v4_addr, addr);
    json_object_object_add(pdn_obj, "v4_addr", json_object_new_string(v4_addr));

    netmask = mipc_data_act_call_cnf_get_ipv4_netmask(msg_cnf_ptr, 24);
    LOGD("IPv4 address netmask: %d.%d.%d.%d %d\n", 
          addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3], netmask);

    dhcp_offset = addr->addr[3];
    if (!cal_odu_info(dhcp_offset, &gw_addr, &odu_addr, &netmask)) {
        /* TBD: To be discussed how to handle */
        LOGE("Wrong PDP address:%d\n", dhcp_offset);
    }

    /* Todo: align netagent refactoring */
    if (check_v4_addr) {
        LOGD("Check IPv4 address for non-ODU\n");
        ifc_update_addr(ifname, addr, netmask);
    }

    json_object_object_add(pdn_obj, "v4_mask", json_object_new_int(netmask));

    addr->addr[3] = gw_addr;
    addr4_to_str(v4_gw, addr);
    json_object_object_add(pdn_obj, "v4_gw", json_object_new_string(v4_gw));

    addr->addr[3] = odu_addr;
    addr4_to_str(v4_gw, addr);
    json_object_object_add(pdn_obj, "odu_addr", json_object_new_string(v4_gw));

    json_object_object_add(pdn_obj, "odu_dhcp_offset", json_object_new_int(dhcp_offset));
}

int data_call_get(mipc_msg_sim_ps_id_enum sim)
{
    uint8_t id, cid = 0xff;

    if (!check_sim_id(sim))
        return -1;

    for (id = 0; id < 8; id++)
    {
        mipc_msg_t *msg_req_ptr = mipc_msg_init(MIPC_DATA_GET_CALL_REQ, sim);
        mipc_msg_t *msg_cnf_ptr;
        mipc_result_enum result;

        if (msg_req_ptr == NULL) {
            LOGD("msg_req_ptr is NULL\n");
            return -1;
        }
        mipc_data_get_call_req_add_id(msg_req_ptr, id);
        msg_cnf_ptr = mipc_msg_sync(msg_req_ptr);
        mipc_msg_deinit(msg_req_ptr);

        result = mipc_get_result(msg_cnf_ptr);
        if (result != MIPC_RESULT_SUCCESS)
        {
            mipc_msg_deinit(msg_cnf_ptr);
            continue;
        }

        do {
            char* apn = mipc_data_get_call_cnf_get_apn(msg_cnf_ptr, NULL);
            mipc_apn_pdp_type_const_enum pdp_type = mipc_data_get_call_cnf_get_pdp_type(msg_cnf_ptr, MIPC_APN_PDP_TYPE_DEFAULT);
            uint8_t v4_count = mipc_data_get_call_cnf_get_v4_count(msg_cnf_ptr, 0);
            uint8_t v6_count = mipc_data_get_call_cnf_get_v6_count(msg_cnf_ptr, 0);
            uint8_t v4_dns_count = mipc_data_get_call_cnf_get_dns_v4_count(msg_cnf_ptr, 0);
            uint8_t v6_dns_count = mipc_data_get_call_cnf_get_dns_v6_count(msg_cnf_ptr, 0);
            uint32_t v4_mtu = mipc_data_get_call_cnf_get_mtu_v4(msg_cnf_ptr, 0);
            uint32_t v6_mtu = mipc_data_get_call_cnf_get_mtu_v6(msg_cnf_ptr, 0);
            uint32_t iid = mipc_data_get_call_cnf_get_interface_id(msg_cnf_ptr, 0);
            json_object *pdn_obj = json_object_new_object();
            char ifname[IFNAMSIZ];
            cid = mipc_data_get_call_cnf_get_id(msg_cnf_ptr, 0xFF);

            esnprintf(ifname, sizeof(ifname), MTK_MOBILE_PREFIX_INF_NAME"%u", iid);
            
            if (pdn_obj == NULL) {
                LOGE("pdn_obj is NULL\n");
                return -1;
            }
            json_object_object_add(pdn_obj, "CID", json_object_new_int(cid));
            json_object_object_add(pdn_obj, "APN", json_object_new_string(apn));
            json_object_object_add(pdn_obj, "PDP_TYPE", json_object_new_int(pdp_type));
            json_object_object_add(pdn_obj, "v4_count", json_object_new_int(v4_count));
            if (v4_count > 0) data_call_get_ipv4_addr(msg_cnf_ptr, pdn_obj, ifname, 0);
            json_object_object_add(pdn_obj, "v4_dns_count", json_object_new_int(v4_dns_count));
            if (v4_dns_count > 0) data_call_get_ipv4_dnses(msg_cnf_ptr, pdn_obj, v4_dns_count);

            json_object_object_add(pdn_obj, "v6_count", json_object_new_int(v6_count));
            if (v6_count > 0) data_call_get_ipv6_addr(msg_cnf_ptr, pdn_obj, ifname, 0);
            json_object_object_add(pdn_obj, "v6_dns_count", json_object_new_int(v6_dns_count));
            if (v6_dns_count > 0) data_call_get_ipv6_dnses(msg_cnf_ptr, pdn_obj, v6_dns_count);

            json_object_object_add(pdn_obj, "v4_mtu", json_object_new_int(v4_mtu));
            json_object_object_add(pdn_obj, "v6_mtu", json_object_new_int(v6_mtu));
            json_object_object_add(pdn_obj, "iface", json_object_new_string(ifname));
            printf("%s\n", json_object_to_json_string(pdn_obj));
            json_object_put(pdn_obj);

            mipc_msg_deinit(msg_cnf_ptr);

            if (cid != 0xff)
                return cid;
        }
        while(0);
    }

    return cid;
}

#ifdef CONFIG_ETHERNET_PDU
// EthPdu +
void data_call_set_reserved_if_id(int id, mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result = MIPC_RESULT_FAILURE;
    bool reserved_if_id[20] = {0};

    reserved_if_id[id] = 1;

    if (!check_sim_id(sim))
        return;

    LOGD("Set ethernet id %d to modem\n", id);

    // MIPC_DATA_SET_RESERVED_IF_ID_REQ
    // MIPC_DATA_SET_RESERVED_IF_ID_CNF
    msg_req_ptr = mipc_msg_init(MIPC_DATA_SET_RESERVED_IF_ID_REQ, sim);
    mipc_data_set_reserved_if_id_req_add_if_id_list(msg_req_ptr, 20, reserved_if_id);

    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
        LOGD("set reserved if id successfully:%d\n", id);
    else
        LOGE("set reserved if id failed:%d:%d\n", id, result);

    mipc_msg_deinit(msg_cnf_ptr);
}
// EthPdu -
#endif

void data_call_act(char* param, mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result = MIPC_RESULT_FAILURE;
    json_object *obj;
    json_object *pdn_obj;
    char cmd[128], *apn = NULL, *username = NULL, *password = NULL;
    uint8_t ip_type, auth_type, mode;
    uint8_t roaming_type;
    uint8_t enable_clatd = 0;
    int mtu = default_mtu;
    uint32_t bearer_bitmask = MIPC_DFLT_BEARER_BITMASK;
    char *bearer_bitmask_string = NULL;
    char *endptr = NULL;

    if (!check_sim_id(sim))
        return;

#ifdef CONFIG_ETHERNET_PDU
    // EthPdu +
    int eth_mode = 0;
    int eth_id = 0;

    system_get_ethernet_pdu_setting(&eth_mode, &eth_id);

    if (eth_mode == 1 && eth_id > 1 && eth_id < 20)
    {
        data_call_set_reserved_if_id(eth_id, sim);
        char buf[50];
        if (!system_get_sysctl("/mnt/vendor/nvcfg/mac1",
                buf, sizeof(buf)))
        {
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, sizeof(cmd), "ifconfig ccmni%d hw ether %s", eth_id, buf);
            LOGE("Write MAC: %s", cmd);
            system(cmd);
        }
    }
    // EthPdu -
#endif

    pdn_obj = json_object_new_object();
    if (pdn_obj == NULL)
    {
        LOGE("Failed to allocate json object\n");
        return;
    }

    obj = json_tokener_parse(param);
    if (obj == NULL || json_object_get_type(obj) != json_type_object)
    {
        LOGE("Failed to parse json data\n");
        goto error;
    }

    apn = get_string(obj, "apn");
    if (!apn)
    {
        LOGE("Failed to get APN data\n");
        json_object_put(obj);
        goto error;
    }

#ifdef CONFIG_ETHERNET_PDU
    // EthPdu +
    if (eth_mode == 1 && eth_id > 1 && eth_id < 20)
    {
        ip_type = get_int(obj, "ip_type", MIPC_APN_PDP_TYPE_ETHERNET);
        roaming_type = get_int(obj, "roaming_type", MIPC_APN_PDP_TYPE_ETHERNET);
    }
    else
    // EthPdu -
#endif
    {
        ip_type = get_int(obj, "ip_type", MIPC_APN_PDP_TYPE_IPV4V6);
        roaming_type = get_int(obj, "roaming_type", MIPC_APN_PDP_TYPE_IPV4V6);
    }
    auth_type = get_int(obj, "auth_type", MIPC_APN_AUTH_TYPE_NONE);
    mtu = get_int(obj, "mtu", default_mtu);
    mode = get_int(obj, "mode", ROUTING_MODE);
    if (mode < ROUTING_MODE || mode > BRIDGE_MODE) {
        LOGD("mode(%d) is out-of-range, restore to routing\n", mode);
        mode = ROUTING_MODE;
    }
    username = get_string(obj, "username");
    password = get_string(obj, "password");
    bearer_bitmask_string = get_string(obj, "bearer_bitmask");
    if (bearer_bitmask_string) 
    {
        LOGD("bearer_bitmask_string: %s\n", bearer_bitmask_string);
        errno = 0;
        bearer_bitmask = strtoll(bearer_bitmask_string, &endptr, 16);
        if (errno == ERANGE || errno == EINVAL) {
            LOGD("strtol return error: %d\n", errno);
            goto error;
        }
    }
    else 
        LOGD("bearer_bitmask_string: NULL\n");

    LOGD("sim%d ip_type:%d roaming_type:%d auth_type:%d apn:%s mtu:%d mode:%d bearer_bitmask:0x%x sim:%d\n", 
        sim, ip_type, roaming_type, auth_type, apn, mtu, mode, bearer_bitmask, sim);

    msg_req_ptr = mipc_msg_init(MIPC_DATA_ACT_CALL_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        goto error;
    }
    if (apn != NULL)
    {
        if (apn[0] == '0')
            mipc_data_act_call_req_add_apn(msg_req_ptr, MIPC_MAX_APN_LEN, "");
        else
    		mipc_data_act_call_req_add_apn(msg_req_ptr, MIPC_MAX_APN_LEN, apn);
    }
    else
        mipc_data_act_call_req_add_apn(msg_req_ptr, MIPC_MAX_APN_LEN, "");
    mipc_data_act_call_req_add_apn_type(msg_req_ptr, MIPC_APN_TYPE_DEFAULT);
    mipc_data_act_call_req_add_pdp_type(msg_req_ptr, ip_type);
    mipc_data_act_call_req_add_roaming_type(msg_req_ptr, roaming_type);

    mipc_data_act_call_req_add_auth_type(msg_req_ptr, auth_type);
    if (username != NULL)
        mipc_data_act_call_req_add_userid(msg_req_ptr, MIPC_MAX_USERID_LEN, username);
    else
        mipc_data_act_call_req_add_userid(msg_req_ptr, MIPC_MAX_USERID_LEN, "");
    if (password != NULL)
        mipc_data_act_call_req_add_password(msg_req_ptr, MIPC_MAX_PASSWORD_LEN, password);
    else
        mipc_data_act_call_req_add_password(msg_req_ptr, MIPC_MAX_PASSWORD_LEN, "");
    mipc_data_act_call_req_add_ipv4v6_fallback(msg_req_ptr, MIPC_DATA_FALLBACK_TYPE_IPV4_FIRST);
    mipc_data_act_call_req_add_bearer_bitmask(msg_req_ptr, bearer_bitmask);

#ifdef CONFIG_ETHERNET_PDU
    // EthPdu +
    if (eth_mode == 1 && eth_id > 1 && eth_id < 20)
    {
        LOGD("Add ethernet id to data_call_act_req\n");
        mipc_data_act_call_req_add_if_id(msg_req_ptr, (uint8_t)eth_id);
        // New TLV: MIPC_DATA_ACT_CALL_REQ_T_IF_I
        // defined in mipc_data_tft_info_struct4. Ref tomipc_msg_tlv_const.h
    }
    // EthPdu -
#endif

    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        uint8_t tid = mipc_data_act_call_cnf_get_id(msg_cnf_ptr, 0);
        mipc_apn_pdp_type_const_enum pdp_type = mipc_data_act_call_cnf_get_pdp_type(msg_cnf_ptr, MIPC_APN_PDP_TYPE_DEFAULT);
        uint8_t v4_addr_count = mipc_data_act_call_cnf_get_v4_count(msg_cnf_ptr, 0);
        uint8_t v6_addr_count = mipc_data_act_call_cnf_get_v6_count(msg_cnf_ptr, 0);
        uint8_t v4_dns_count = mipc_data_act_call_cnf_get_dns_v4_count(msg_cnf_ptr, 0);
        uint8_t v6_dns_count = mipc_data_act_call_cnf_get_dns_v6_count(msg_cnf_ptr, 0);
        uint32_t v4_mtu = mipc_data_get_call_cnf_get_mtu_v4(msg_cnf_ptr, 0);
        uint32_t v6_mtu = mipc_data_get_call_cnf_get_mtu_v6(msg_cnf_ptr, 0);
        uint32_t ifid = mipc_data_act_call_cnf_get_interface_id(msg_cnf_ptr, 0xff);
        char ifname[IFNAMSIZ];
        int n;

        if (ifid == 0xff)
        {
            LOGE("No interfce ID\n");
            result = MIPC_RESULT_FAILURE;
            goto error2;
        }
        else
        {
            n = snprintf(ifname, sizeof(ifname), MTK_MOBILE_PREFIX_INF_NAME"%u", ifid);
            if (n < 0 || n >= sizeof(ifname))
            {
                LOGE("snprintf error\n");
                goto error2;
            }
            json_object_object_add(pdn_obj, "ifname", json_object_new_string(ifname));
        }

        LOGD("Tid:%d IP type:%d addr v4:%d v6:%d dns v4:%d v6:%d v4_mtu:%d v6_mtu:%d ifid:%d\n",
             tid, pdp_type, v4_addr_count, v6_addr_count, v4_dns_count, v6_dns_count,
             v4_mtu, v6_mtu, ifid);
        json_object_object_add(pdn_obj, "cid", json_object_new_int(tid));
        if (v4_addr_count > 0) data_call_get_ipv4_addr(msg_cnf_ptr, pdn_obj, ifname, mode == ROUTING_MODE);
        if (v4_dns_count > 0) data_call_get_ipv4_dnses(msg_cnf_ptr, pdn_obj, v4_dns_count);
        if (v6_addr_count > 0) data_call_get_ipv6_addr(msg_cnf_ptr, pdn_obj, ifname, mode == ROUTING_MODE);
        if (v6_dns_count > 0) data_call_get_ipv6_dnses(msg_cnf_ptr, pdn_obj, v6_dns_count);
        if (v4_mtu == 0) v4_mtu = default_mtu;
        if (v6_mtu == 0) v6_mtu = default_mtu;
        mtu = v4_mtu;
        if (mtu != default_mtu) {
            memset(cmd, 0, sizeof(cmd));
            n = snprintf(cmd, sizeof(cmd), "ip link set dev %s mtu %d", ifname, mtu);
            if (n < 0 || n >= sizeof(cmd))
            {
                LOGE("snprintf error\n");
                goto error2;
            }
            LOGD("Update MTU:%s", cmd);
            system(cmd);
        }
        json_object_object_add(pdn_obj, "v4_mtu", json_object_new_int(mtu));
        json_object_object_add(pdn_obj, "v6_mtu", json_object_new_int(v6_mtu));

        /* Check IPv6 only support */
        if (mode == ROUTING_MODE && v4_addr_count == 0 && v6_addr_count > 0) {
            LOGD("Enable clatd setting\n");
            enable_clatd = 1;
            json_object_object_add(pdn_obj, "clatd", json_object_new_int(enable_clatd));
        }
    }
    else
    {
        LOGE("sim:%d Failed to execute with  MIPC_DATA_ACT_CALL_REQ:%d\n", sim, result);
    }
error2:
    mipc_msg_deinit(msg_cnf_ptr);

error:
    if (obj)
        json_object_put(obj);

    json_object_object_add(pdn_obj, "result", json_object_new_int(result));
    if (!g_log_hidden)
        LOGD("Dump:%s\n", json_object_to_json_string(pdn_obj));

    /* Dump json info to script */
    printf("%s", json_object_to_json_string(pdn_obj));
    json_object_put(pdn_obj);
}

void data_call_act_type(char* param, mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result = MIPC_RESULT_FAILURE;
    json_object *obj;
    json_object *pdn_obj;
    char cmd[128], *apn = NULL, *username = NULL, *password = NULL;
    uint8_t ip_type, auth_type, mode;
    uint8_t roaming_type, ipv4v6_fallback;
    uint8_t enable_clatd = 0;
    int mtu;
    uint32_t apn_type;
    uint32_t bearer_bitmask = MIPC_DFLT_BEARER_BITMASK;
    char *bearer_bitmask_string = NULL;
    char *endptr = NULL;

    if (!check_sim_id(sim))
        return;

    pdn_obj = json_object_new_object();
    if (pdn_obj == NULL)
    {
        LOGE("Failed to allocate json object\n");
        return;
    }

    obj = json_tokener_parse(param);
    if (obj == NULL || json_object_get_type(obj) != json_type_object)
    {
        LOGE("Failed to parse json data\n");
        goto error;
    }

    apn = get_string(obj, "apn");
    if (!apn)
    {
        LOGE("Failed to get APN data\n");
        json_object_put(obj);
        goto error;
    }

    apn_type = get_int(obj, "apn_type", MIPC_APN_TYPE_DUN);
    ip_type = get_int(obj, "ip_type", MIPC_APN_PDP_TYPE_IPV4V6);
    roaming_type = get_int(obj, "roaming_type", MIPC_APN_PDP_TYPE_IPV4V6);
    auth_type = get_int(obj, "auth_type", MIPC_APN_AUTH_TYPE_NONE);
    ipv4v6_fallback = get_int(obj, "ipv4v6_fallback", MIPC_DATA_FALLBACK_TYPE_IPV4_FIRST);
    mtu = get_int(obj, "mtu", 1500);
    mode = get_int(obj, "mode", ROUTING_MODE);
    if (mode < ROUTING_MODE || mode > BRIDGE_MODE) {
        LOGD("mode(%d) is out-of-range, restore to routing\n", mode);
        mode = ROUTING_MODE;
    }
    username = get_string(obj, "username");
    password = get_string(obj, "password");
    bearer_bitmask_string = get_string(obj, "bearer_bitmask");
    if (bearer_bitmask_string)
    {
        LOGD("bearer_bitmask_string: %s\n", bearer_bitmask_string);
        errno = 0;
        bearer_bitmask = strtoll(bearer_bitmask_string, &endptr, 16);
        if (errno == ERANGE || errno == EINVAL) {
            LOGD("strtol return error: %d\n", errno);
            goto error;
        }
    }
    else
        LOGD("bearer_bitmask_string: NULL\n");

    LOGD("apn_type:0x%x ip_type:%d roaming_type:%d auth_type:%d apn:%s mtu:%d mode:%d ipv4v6_fallback:%d bearer_bitmask:%x\n",
        apn_type, ip_type, roaming_type, auth_type, apn, mtu, mode, ipv4v6_fallback, bearer_bitmask);

    msg_req_ptr = mipc_msg_init(MIPC_DATA_ACT_CALL_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        goto error;
    }
    mipc_data_act_call_req_add_apn(msg_req_ptr, MIPC_MAX_APN_LEN, apn);
    mipc_data_act_call_req_add_apn_type(msg_req_ptr, apn_type);
    mipc_data_act_call_req_add_pdp_type(msg_req_ptr, ip_type);
    mipc_data_act_call_req_add_roaming_type(msg_req_ptr, roaming_type);

    mipc_data_act_call_req_add_auth_type(msg_req_ptr, auth_type);
    if (username != NULL)
        mipc_data_act_call_req_add_userid(msg_req_ptr, MIPC_MAX_USERID_LEN, username);
    else
        mipc_data_act_call_req_add_userid(msg_req_ptr, MIPC_MAX_USERID_LEN, "");
    if (password != NULL)
        mipc_data_act_call_req_add_password(msg_req_ptr, MIPC_MAX_PASSWORD_LEN, password);
    else
        mipc_data_act_call_req_add_password(msg_req_ptr, MIPC_MAX_PASSWORD_LEN, "");
    mipc_data_act_call_req_add_ipv4v6_fallback(msg_req_ptr, ipv4v6_fallback);
    mipc_data_act_call_req_add_bearer_bitmask(msg_req_ptr, bearer_bitmask);

    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        uint8_t tid = mipc_data_act_call_cnf_get_id(msg_cnf_ptr, 0);
        mipc_apn_pdp_type_const_enum pdp_type = mipc_data_act_call_cnf_get_pdp_type(msg_cnf_ptr, MIPC_APN_PDP_TYPE_DEFAULT);
        uint8_t v4_addr_count = mipc_data_act_call_cnf_get_v4_count(msg_cnf_ptr, 0);
        uint8_t v6_addr_count = mipc_data_act_call_cnf_get_v6_count(msg_cnf_ptr, 0);
        uint8_t v4_dns_count = mipc_data_act_call_cnf_get_dns_v4_count(msg_cnf_ptr, 0);
        uint8_t v6_dns_count = mipc_data_act_call_cnf_get_dns_v6_count(msg_cnf_ptr, 0);
        uint32_t v4_mtu = mipc_data_get_call_cnf_get_mtu_v4(msg_cnf_ptr, 0);
        uint32_t v6_mtu = mipc_data_get_call_cnf_get_mtu_v6(msg_cnf_ptr, 0);
        uint32_t ifid = mipc_data_act_call_cnf_get_interface_id(msg_cnf_ptr, 0xff);
        char ifname[IFNAMSIZ];
        int n;

        if (ifid == 0xff)
        {
            LOGE("No interfce ID\n");
            result = MIPC_RESULT_FAILURE;
            goto error2;
        }
        else
        {
            n = snprintf(ifname, sizeof(ifname), MTK_MOBILE_PREFIX_INF_NAME"%u", ifid);
            if (n < 0 || n >= sizeof(ifname))
            {
                LOGE("snprintf error\n");
                goto error2;
            }
            json_object_object_add(pdn_obj, "ifname", json_object_new_string(ifname));
        }

        LOGD("Tid:%d IP type:%d addr v4:%d v6:%d dns v4:%d v6:%d v4_mtu:%d v6_mtu:%d ifid:%d\n",
             tid, pdp_type, v4_addr_count, v6_addr_count, v4_dns_count, v6_dns_count,
             v4_mtu, v6_mtu, ifid);
        json_object_object_add(pdn_obj, "cid", json_object_new_int(tid));
        if (v4_addr_count > 0) data_call_get_ipv4_addr(msg_cnf_ptr, pdn_obj, ifname, mode == ROUTING_MODE);
        if (v4_dns_count > 0) data_call_get_ipv4_dnses(msg_cnf_ptr, pdn_obj, v4_dns_count);
        if (v6_addr_count > 0) data_call_get_ipv6_addr(msg_cnf_ptr, pdn_obj, ifname, mode == ROUTING_MODE);
        if (v6_dns_count > 0) data_call_get_ipv6_dnses(msg_cnf_ptr, pdn_obj, v6_dns_count);
        if (v4_mtu == 0) v4_mtu = 1500;
        if (v6_mtu == 0) v6_mtu = 1500;
        if (v4_mtu > 0 && v4_mtu < mtu)
            mtu = v4_mtu;
        if (mtu > 0 && mtu < 1500) {
            memset(cmd, 0, sizeof(cmd));
            n = snprintf(cmd, sizeof(cmd), "ip link set dev %s mtu %d", ifname, mtu);
            if (n < 0 || n >= sizeof(cmd))
            {
                LOGE("snprintf error\n");
                goto error2;
            }
            LOGD("Update MTU:%s", cmd);
            system(cmd);
        }
        json_object_object_add(pdn_obj, "v4_mtu", json_object_new_int(mtu));
        json_object_object_add(pdn_obj, "v6_mtu", json_object_new_int(v6_mtu));

        /* Check IPv6 only support */
        if (mode == ROUTING_MODE && v4_addr_count == 0 && v6_addr_count > 0) {
            LOGD("Enable clatd setting\n");
            enable_clatd = 1;
            json_object_object_add(pdn_obj, "clatd", json_object_new_int(enable_clatd));
        }
    }
    else
    {
        LOGE("Failed to execute with  MIPC_DATA_ACT_CALL_REQ:%d\n", result);
    }
error2:
    mipc_msg_deinit(msg_cnf_ptr);

error:
    if (obj)
        json_object_put(obj);

    json_object_object_add(pdn_obj, "result", json_object_new_int(result));
    if (!g_log_hidden)
        LOGD("Dump:%s\n", json_object_to_json_string(pdn_obj));

    /* Dump json info to script */
    printf("%s", json_object_to_json_string(pdn_obj));
    json_object_put(pdn_obj);
}

void data_call_deact(int id, mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;

    if (!check_sim_id(sim))
        return;

    msg_req_ptr = mipc_msg_init(MIPC_DATA_DEACT_CALL_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    mipc_data_deact_call_req_add_id(msg_req_ptr, id);

    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
        LOGD("sim%d data call deactivation successfully:%d\n", sim, id);
    else
        LOGE("sim%d data call failed to deactivation id:%d result:%d\n", sim, id, result);

    mipc_msg_deinit(msg_cnf_ptr);
}

void data_call_deact_apn(const char* apn_ptr, mipc_msg_sim_ps_id_enum sim)
{
    char apn[MIPC_MAX_APN_LEN] = {0};
    uint8_t id, cid, len = (MIPC_MAX_APN_LEN - 1);

    if (!check_sim_id(sim))
        return;

    if (apn_ptr == NULL) {
        LOGE("APN param is NULL\n");
        return;
    }
    strncpy(apn, apn_ptr, MIPC_MAX_APN_LEN - 1);
    apn[MIPC_MAX_APN_LEN - 1] = '\0';
    LOGD("SIM%d APN:%s deact data call APN\n", sim, apn);

    for (id = 0; id < 8; id++)
    {
        mipc_msg_t *msg_req_ptr = mipc_msg_init(MIPC_DATA_GET_CALL_REQ, sim);
        mipc_msg_t *msg_cnf_ptr;
        mipc_result_enum result;
        char* apn2;

        if (msg_req_ptr == NULL) {
            LOGD("msg_req_ptr is NULL\n");
            return;
        }
        mipc_data_get_call_req_add_id(msg_req_ptr, id);
        msg_cnf_ptr = mipc_msg_sync(msg_req_ptr);
        mipc_msg_deinit(msg_req_ptr);

        result = mipc_get_result(msg_cnf_ptr);
        if (result != MIPC_RESULT_SUCCESS)
        {
            mipc_msg_deinit(msg_cnf_ptr);
            continue;
        }

        apn2 = mipc_data_get_call_cnf_get_apn(msg_cnf_ptr, NULL);
        cid = mipc_data_get_call_cnf_get_id(msg_cnf_ptr, 0xFF);
        mipc_msg_deinit(msg_cnf_ptr);

        if (cid == 0xff)
            continue;

        if (strlen(apn) < len) len = strlen(apn);
        if (!strncasecmp(apn, apn2, len)) {
            data_call_deact(cid, sim);
            return;
        }
        while(0);
    }

}

void get_imei()
{
    mipc_msg_t *msg_req_ptr = mipc_msg_init(MIPC_SYS_GET_INFO_REQ, MIPC_PS0);
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;

    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        uint16_t len = 0;
        char* imei = mipc_sys_get_info_cnf_get_device_id(msg_cnf_ptr, &len);

        if (imei) {
            imei[len] = '\0';
            printf("%s\n", imei);
        } else
            LOGE("No IMEI:%d\n", len);
    }
    else
    {
        LOGE("Failed to execute with  MIPC_SYS_GET_INFO_CNF:%d\n", result);
    }
    mipc_msg_deinit(msg_cnf_ptr);
}

void get_imeisv()
{
    mipc_msg_t *msg_req_ptr = mipc_msg_init(MIPC_SYS_GET_INFO_REQ, MIPC_PS0);
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;

    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        uint16_t len = 0;
        char* imeisv = mipc_sys_get_info_cnf_get_imeisv(msg_cnf_ptr, &len);

        if (imeisv) {
            imeisv[len] = '\0';
            printf("%s\n", imeisv);
        } else
            LOGE("No IMEI:%d\n", len);
    }
    else
    {
        LOGE("Failed to execute with  MIPC_SYS_GET_INFO_CNF:%d\n", result);
    }
    mipc_msg_deinit(msg_cnf_ptr);
}

void nw_set_reg(int rat_type, mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;

    if (!check_sim_id(sim))
        return;

    msg_req_ptr = mipc_msg_init(MIPC_NW_SET_REGISTER_STATE_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    mipc_msg_add_tlv_uint8(msg_req_ptr, MIPC_NW_SET_REGISTER_STATE_REQ_T_MODE, MIPC_NW_REGISTER_MODE_AUTOMATIC);
    if (rat_type > 0)
        mipc_msg_add_tlv_uint8(msg_req_ptr, MIPC_NW_SET_REGISTER_STATE_REQ_T_RAT_MODE, rat_type);
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        mipc_nw_reg_state_struct4* reg_state = mipc_nw_set_register_state_cnf_get_state(msg_cnf_ptr, NULL);
        mipc_nw_register_mode_const_enum reg_mode = mipc_nw_set_register_state_cnf_get_mode(msg_cnf_ptr, mipc_nw_register_mode_const_NONE);
        char* network_name = mipc_nw_set_register_state_cnf_get_nw_name(msg_cnf_ptr, NULL);

        LOGD("ps_state=%d, cs_state=%d plmn=%s is_roaming:%d reg_mode=%d\n nw name:%s\n",
                reg_state->ps_state, reg_state->cs_state, reg_state->plmn, reg_state->is_roaming,
                reg_mode, network_name);
    }
    else
    {
        LOGE("Failed to execute with  MIPC_SYS_GET_INFO_CNF:%d\n", result);
    }
    mipc_msg_deinit(msg_cnf_ptr);
}

void usage()
{
    int i = 0, size = sizeof(cmd_opts) / sizeof(cmd_opts[0]) - 1;

    printf("\n\n\n\n\n\nUsage: below options are supported:%d\n", size);

    for (i = 0; i < size; i++)
        printf("%s\n", cmd_opts[i].name);
}

// Add for HW radio button
void nw_change_hw_ratio(mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_nw_radio_state_const_enum prev_radio_state;
    mipc_nw_radio_state_const_enum radio_state = MIPC_NW_RADIO_STATE_OFF;
    mipc_result_enum result;

    if (!check_sim_id(sim))
        return;

    prev_radio_state = nw_radio_state_get(1, sim);
    printf("change_hw_ratio_setting: current radio state:%s\n", radio_state_name(prev_radio_state));

    msg_req_ptr = mipc_msg_init(MIPC_NW_SET_RADIO_STATE_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    if (prev_radio_state == MIPC_NW_RADIO_STATE_OFF)
    {
        mipc_nw_set_radio_state_req_add_hw_state(msg_req_ptr, MIPC_NW_RADIO_STATE_ON);
    }
    else
    {
        mipc_nw_set_radio_state_req_add_hw_state(msg_req_ptr, MIPC_NW_RADIO_STATE_OFF);
    }
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        radio_state = mipc_nw_get_radio_state_cnf_get_hw_state(msg_cnf_ptr, MIPC_NW_RADIO_STATE_OFF);
        printf("change_hw_ratio_setting done, get SIM%d HW radio state:%s\n", sim, radio_state_name(radio_state));
    }
    else
    {
        printf("Failed to execute with SIM%d change_hw_ratio_setting\n", sim);
    }
    mipc_msg_deinit(msg_cnf_ptr);
}

uint16_t nw_get_register_rat(mipc_msg_sim_ps_id_enum sim) {
    mipc_msg_t* msg_req_ptr;
    mipc_msg_t* msg_cnf_ptr;
    int result = 0;
    uint16_t nw_rat = MIPC_NW_DATA_SPEED_NONE_SPECIFIED;

    if (!check_sim_id(sim))
        return nw_rat;

    msg_req_ptr = mipc_msg_init(MIPC_NW_GET_PS_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return nw_rat;
    }
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);
    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)  // modem should define the return value enum.
    {
        mipc_nw_ps_reg_info_struct4 *reg_info = mipc_nw_get_ps_cnf_get_reg_info(msg_cnf_ptr, NULL);
        nw_rat = reg_info->rat;
        LOGD("reg_info->rat = 0x%x\n", nw_rat);

    } else {
        LOGE("MIPC_NW_GET_PS_REQ fail\n");
    }

    mipc_msg_deinit(msg_cnf_ptr);

    return nw_rat;
}

uint16_t nw_show_register_rat(mipc_msg_sim_ps_id_enum sim) {
    uint16_t nw_rat = MIPC_NW_DATA_SPEED_NONE_SPECIFIED;

    if (!check_sim_id(sim))
        return nw_rat;

    nw_rat = nw_get_register_rat(sim);
    switch (nw_rat) {
        // 4G & 5G
        case MIPC_NW_DATA_SPEED_LTE:
        case MIPC_NW_DATA_SPEED_LTE_CA:
            printf("Current registered: 4G, RAT=0x%x\n", nw_rat);
            LOGD("Current registered: 4G, RAT=0x%x\n", nw_rat);
            break;
        case MIPC_NW_DATA_SPEED_ENDC:
        case MIPC_NW_DATA_SPEED_NR:
            printf("Current registered: 5G, RAT=0x%x\n", nw_rat);
            LOGD("Current registered: 5G, RAT=0x%x\n", nw_rat);
            break;
        // Others
        default:
            printf("Current registered: RAT=0x%x\n", nw_rat);
            LOGD("Current registered: RAT=0x%x\n", nw_rat);
            break;
    }

    return nw_rat;
}

int nw_get_rat(mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    uint32_t result = mipc_result_const_NONE;
    int rat_mode = 19;

    if (!check_sim_id(sim))
        return rat_mode;

    do {
        msg_req_ptr = mipc_msg_init(MIPC_NW_GET_RAT_REQ, sim);
        if (msg_req_ptr == NULL) {
            LOGD("msg_req_ptr is NULL\n");
            return rat_mode;
        }
        msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
        mipc_msg_deinit(msg_req_ptr);
        result = mipc_get_result(msg_cnf_ptr);
        if (result == MIPC_RESULT_SUCCESS)  // modem should define the return value enum.
        {
            rat_mode = mipc_nw_get_rat_cnf_get_rat_mode(msg_cnf_ptr, rat_mode);
        }
        mipc_msg_deinit(msg_cnf_ptr);

        LOGD("SIM%d nw_get_rat: %d\n", sim, rat_mode);
        printf("SIM%d RAT Mode Value: %d\n", sim, rat_mode);

        switch(rat_mode) {
            case 0: //GSM only
                printf("Current: 2G only\n");
                break;
            case 1: //WCDMA only
                printf("Current: 3G only\n");
                break;
            case 3: //LTE only
                printf("Current: 4G only\n");
                break;
            case 15: //NR only
                printf("Current: 5G only\n");
                break;
            case 19: //NR, LTE
                printf("Current: 4/5G\n");
                break;
            case 21: //NR, LTE, WCDMA
                printf("Current: 3/4/5G\n");
                break;
            default:
                printf("Current: %d\n", rat_mode);
        }
    } while(0);

    return rat_mode;
}

void nw_set_rat(int rat_type, mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    uint32_t result = 0;

    if (!check_sim_id(sim))
        return;

    do {
        msg_req_ptr = mipc_msg_init(MIPC_NW_SET_RAT_REQ, sim);
        if (msg_req_ptr == NULL) {
            LOGD("msg_req_ptr is NULL\n");
            return;
        }
        mipc_msg_add_tlv_uint8(msg_req_ptr, MIPC_NW_SET_RAT_REQ_T_RAT, rat_type);
        msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
        mipc_msg_deinit(msg_req_ptr);

        if (!msg_cnf_ptr) {
            printf("nw_set_rat: set rat fail!\n");
            break;
        }

        result = mipc_get_result(msg_cnf_ptr);
        if (result == MIPC_RESULT_SUCCESS)  // modem should define the return value enum.
        {
            printf("nw_set_rat success, rat = %d\n", rat_type);
        }
        else
        {
            printf("nw_set_rat fail. result: %u\n", result);
        }
        mipc_msg_deinit(msg_cnf_ptr);
    } while (0);
}

int32_t nw_get_signal(mipc_msg_sim_ps_id_enum sim) {
    mipc_msg_t* msg_req_ptr;
    mipc_msg_t* msg_cnf_ptr;
    int result = 0;
    int32_t rsrp = 0;
    uint16_t nw_rat = MIPC_NW_DATA_SPEED_NONE_SPECIFIED;

    if (!check_sim_id(sim))
        return nw_rat;

    nw_rat = nw_get_register_rat(sim);
    msg_req_ptr = mipc_msg_init(MIPC_NW_GET_SIGNAL_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return 0;
    }
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);
    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)  // modem should define the return value enum.
    {
        // 5G
        if (nw_rat == MIPC_NW_DATA_SPEED_ENDC || nw_rat == MIPC_NW_DATA_SPEED_NR) {
            mipc_nw_nr_signal_strength_struct4 *p_data = (mipc_nw_nr_signal_strength_struct4 *)
                    mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_NW_GET_SIGNAL_CNF_T_NR_SIGNAL, NULL);

            if (p_data == NULL) {
                printf("RAT 5G, Can't get RSRP");
                LOGE("Failed with NULL NR signal data\n");
                return 0;
            }

            rsrp = p_data->ss_rsrp*-1;
            printf("RAT 5G, RSRP=%d", rsrp);
            LOGD("nw_rat=0x%x, RSRP=%d", nw_rat, rsrp);

        // 4G
        } else if (nw_rat == MIPC_NW_DATA_SPEED_LTE || nw_rat == MIPC_NW_DATA_SPEED_LTE_CA) {
            mipc_nw_lte_signal_strength_struct4 *p_data = (mipc_nw_lte_signal_strength_struct4 *)
                    mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_NW_GET_SIGNAL_CNF_T_LTE_SIGNAL, NULL);

            if (p_data == NULL) {
                printf("RAT 4G, Can't get RSRP");
                LOGE("Failed with NULL LTE signal data\n");
                return 0;
            }

            rsrp = p_data->rsrp*-1;
            printf("RAT 4G, RSRP=%d", rsrp);
            LOGD("nw_rat=0x%x, RSRP=%d", nw_rat, rsrp);

        } else {
            printf("RAT not 4/5G, no RSRP");
            LOGD("nw_rat=0x%x, no RSRP", nw_rat);
        }

    } else {
        LOGE("MIPC_NW_GET_SIGNAL_REQ fail\n");
    }

    mipc_msg_deinit(msg_cnf_ptr);

    return rsrp;
}

void ims_get_config(mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;
    void *config = NULL;

    if (!check_sim_id(sim))
        return;

    msg_req_ptr = mipc_msg_init(MIPC_IMS_GET_CONFIG_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    mipc_ims_get_config_req_add_class(msg_req_ptr, MIPC_IMS_CONFIG_CLASS_IMS_FEATURE);

    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)  // modem should define the return value enum.
    {
        config = mipc_ims_get_config_cnf_get_data(msg_cnf_ptr, NULL);
        if ( *((uint8_t*)config) == 0) {
            printf("Current: Off\n");
            LOGD("SIM%d ims_get_config(), Current: Off\n", sim);
        } else {
            printf("Current: On(%d)\n", *((uint8_t*)config));
            LOGD("SIM%d ims_get_config(), Current: On(%d)\n", sim, *((uint8_t*)config));
        }
    }
    else
    {
        printf("SIM%d ims_get_config fail. result: %d\n", sim, result);
    }
    mipc_msg_deinit(msg_cnf_ptr);
}

void ims_set_config(char* param, mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;
    uint8_t data[6]={0,0,0,0,1,1};

    if (!check_sim_id(sim))
        return;

    data[0] = atoi(param);

    msg_req_ptr = mipc_msg_init(MIPC_IMS_SET_CONFIG_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    mipc_ims_set_config_req_add_class(msg_req_ptr, MIPC_IMS_CONFIG_CLASS_IMS_FEATURE);
    mipc_ims_set_config_req_add_data(msg_req_ptr, 6, (void*)data);

    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)  // modem should define the return value enum.
    {
        printf("ims_set_config success. volte: %d\n", data[0]);
        LOGD("SIM%d ims_set_config success. volte: %d\n", sim, data[0]);
    }
    else
    {
        printf("ims_set_config fail. result: %d\n", result);
        LOGD("SIM%d ims_set_config fail. result: %d\n", sim, result);
    }
    mipc_msg_deinit(msg_cnf_ptr);

    // Check set result
    ims_get_config(sim);
}

void ims_get_status(mipc_msg_sim_ps_id_enum sim) {
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    //mipc_result_enum result;

    if (!check_sim_id(sim))
        return;

    msg_req_ptr = mipc_msg_init(MIPC_IMS_GET_STATE_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }

    mipc_msg_add_tlv_uint8(msg_req_ptr,
            MIPC_IMS_GET_STATE_REQ_T_EVENT, MIPC_IMS_STATE_IND_EVENT_REG_RESP);

    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    do {
        void* val_ptr;
        val_ptr = mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_T_RESULT, NULL);
        if (NULL == val_ptr) break;
        //printf("----------------SS GET IMS STATE RESULT:%d \n", *((uint32_t*)val_ptr));

        val_ptr = mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_IMS_GET_STATE_CNF_T_EVENT, NULL);
        if (NULL == val_ptr) break;
        //printf("----------------MIPC_IMS_GET_STATE_CNF_T_EVENT:%d \n", *((uint8_t*)val_ptr));

        val_ptr = mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_IMS_GET_STATE_CNF_T_REG_STATE, NULL);
        if (NULL == val_ptr) break;
        //printf("----------------MIPC_IMS_GET_STATE_CNF_T_REG_STATE:%d \n", *((uint8_t*)val_ptr));
        if (*((uint8_t*)val_ptr) == 1) {
            printf("Registered");
            LOGD("SIM%d ims_get_status(): Registered", sim);
        } else {
            printf("Unregistered");
            LOGD("SIM%d ims_get_status(): Unregistered", sim);
        }

        val_ptr = mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_IMS_GET_STATE_CNF_T_EXT_INFO, NULL);
        if (NULL == val_ptr) break;
        //printf("----------------MIPC_IMS_GET_STATE_CNF_T_EXT_INFO:%d \n", *((uint32_t*)val_ptr));

        val_ptr = mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_IMS_GET_STATE_CNF_T_WFC, NULL);
        if (NULL == val_ptr) break;
        //printf("----------------MIPC_IMS_GET_STATE_CNF_T_WFC:%d \n", *((uint8_t*)val_ptr));

        val_ptr = mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_IMS_GET_STATE_CNF_T_ACCOUNT_ID, NULL);
        if (NULL == val_ptr) break;
        //printf("----------------MIPC_IMS_GET_STATE_CNF_T_ACCOUNT_ID:%d \n", *((uint32_t*)val_ptr));

        val_ptr = mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_IMS_GET_STATE_CNF_T_URI, NULL);
        if (NULL == val_ptr) break;
        //printf("----------------MIPC_IMS_GET_STATE_CNF_T_URI:%s \n", (uint8_t*)val_ptr);

        val_ptr = mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_IMS_GET_STATE_CNF_T_EXPIRE_TIME, NULL);
        if (NULL == val_ptr) break;
        //printf("----------------MIPC_IMS_GET_STATE_CNF_T_EXPIRE_TIME:%d \n", *((uint32_t*)val_ptr));

        val_ptr = mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_IMS_GET_STATE_CNF_T_ERROR_CODE, NULL);
        if (NULL == val_ptr) break;
        //printf("----------------MIPC_IMS_GET_STATE_CNF_T_ERROR_CODE:%d \n", *((uint32_t*)val_ptr));

        val_ptr = mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_IMS_GET_STATE_CNF_T_ERROR_MESSAGE, NULL);
        if (NULL == val_ptr) break;
        //printf("----------------MIPC_IMS_GET_STATE_CNF_T_ERROR_MESSAGE:%s \n", (uint8_t*)val_ptr);

    }while(0);

    mipc_msg_deinit(msg_cnf_ptr);

}

void show_sim_status(mipc_msg_sim_ps_id_enum sim) {
    mipc_sim_status_const_enum state;

    if (!check_sim_id(sim))
        return;

    state = sim_get_status(sim);
    /* Output print format */
    printf("%s", sim_status_name(state));
    LOGD("%s", sim_status_name(state));
}

uint8_t show_register_status(mipc_msg_sim_ps_id_enum sim) {
    mipc_msg_t* msg_req_ptr;
    mipc_msg_t* msg_cnf_ptr;
    int result = 0;
    uint8_t nw_status = MIPC_NW_REGISTER_STATE_NOT_REGISTERED;

    if (!check_sim_id(sim))
        return nw_status;

    msg_req_ptr = mipc_msg_init(MIPC_NW_GET_PS_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return nw_status;
    }
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);
    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)  // modem should define the return value enum.
    {
        mipc_nw_ps_reg_info_struct4 *reg_info = mipc_nw_get_ps_cnf_get_reg_info(msg_cnf_ptr, NULL);
        nw_status = reg_info->stat;
        printf("%s", nw_register_state(reg_info->stat));
        LOGD("SIM:%d %s\n", sim, nw_register_state(reg_info->stat));
    }
    mipc_msg_deinit(msg_cnf_ptr);

    return nw_status;
}

void ps_nw_status_cb(mipc_msg_t *msg_ptr, void *priv)
{
    uint8_t ps_state = mipc_nw_ps_ind_get_tach(msg_ptr, MIPC_NW_PS_DETACH);
    int ret = 0;

    LOGD("PS status:%s\n", nw_ps_state_name(ps_state));
    if (ps_state == MIPC_NW_PS_ATTACH) {
            ret = pthread_mutex_lock(&event_queue_mutex);
            if (ret != 0) {
               LOGE("Error locking mutex: %s\n", strerror(ret));
               return;
            }
            ret = pthread_cond_signal(&event_queue_cond);
            pthread_mutex_unlock(&event_queue_mutex);
            if (ret != 0) {
                LOGE("Error cond siganl: %s\n", strerror(ret));
            }
    }
}

uint8_t check_nw_status(mipc_msg_sim_ps_id_enum sim)
{
    int ret, is_network_ready = 0;
    uint8_t ps_status = MIPC_NW_REGISTER_STATE_NOT_REGISTERED;
    struct timespec timeout;

    if (!check_sim_id(sim))
        return 1;

    ps_status = show_register_status(sim);
    if (ps_status == MIPC_NW_REGISTER_STATE_HOME || ps_status == MIPC_NW_REGISTER_STATE_ROAMING)
        return 0;

    clock_gettime(CLOCK_MONOTONIC, &timeout);
    timeout.tv_sec = timeout.tv_sec + NETWORK_WAITING_TIME;
    timeout.tv_nsec = 0;

    ret = pthread_mutex_lock(&event_queue_mutex);
    if (ret != 0) {
       LOGE("Error locking mutex: %s\n", strerror(ret));
       return 1;
    }
    mipc_msg_register_ind(sim, MIPC_NW_PS_IND, ps_nw_status_cb, &is_network_ready);
    ret = pthread_cond_timedwait(&event_queue_cond, &event_queue_mutex, &timeout);
    pthread_mutex_unlock(&event_queue_mutex);
    if (ret == ETIMEDOUT)
        LOGD("Timeout to wait for SIM%d network ready\n", sim);
    else if (ret != 0)
        LOGD("Error for SIM%d network wait:%d\n", sim, errno);

    mipc_msg_register_ind(sim, MIPC_NW_PS_IND, NULL, NULL);
    return 1;
}

void sync_modem_data_setting(int data_roaming, mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;

    if (!check_sim_id(sim))
        return;

    msg_req_ptr = mipc_msg_init(MIPC_DATA_SET_CONFIG_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    /* AT+ECNCFG=1,0,0,0,0,0 */
    /* AT+ECNCFG=1,0/1,0,0,0,0 */
    mipc_data_set_config_req_add_mobile_data(msg_req_ptr, MIPC_DATA_CONFIG_TYPE_ENABLE);
    // check if enable data roaming
    LOGD("data_roaming=%d\n", data_roaming);
    if (data_roaming == 1)
        mipc_data_set_config_req_add_data_roaming(msg_req_ptr, MIPC_DATA_CONFIG_TYPE_ENABLE);
    else
        mipc_data_set_config_req_add_data_roaming(msg_req_ptr, MIPC_DATA_CONFIG_TYPE_DISABLE);
    mipc_data_set_config_req_add_volte(msg_req_ptr, MIPC_DATA_CONFIG_TYPE_DISABLE);
    mipc_data_set_config_req_add_ims_test_mode(msg_req_ptr, MIPC_DATA_CONFIG_TYPE_DISABLE);
    mipc_data_set_config_req_add_data_domestic_roaming(msg_req_ptr, MIPC_DATA_CONFIG_TYPE_DISABLE);
    mipc_data_set_config_req_add_data_international_roaming(msg_req_ptr, MIPC_DATA_CONFIG_TYPE_DISABLE);

    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);
    result = mipc_get_result(msg_cnf_ptr);
    LOGD("Result is %d\n", result);
    mipc_msg_deinit(msg_cnf_ptr);
}

void set_perf_data_simslot(mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;
  
    if (!check_sim_id(sim))
        return;

    msg_req_ptr = mipc_msg_init(MIPC_DATA_SET_DATA_ALLOW_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    /* At+EDATASIM=1 */
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);
    result = mipc_get_result(msg_cnf_ptr);
    LOGD("SIM%d Result is %d\n", sim, result);
    mipc_msg_deinit(msg_cnf_ptr);
}

void check_ia(char* apn_ptr, int rat_type, int ip_type, int roaming_type, 
              char* userid_ptr, char* password_ptr, int auth_type, uint32_t bearer_bitmask, int data_roaming, mipc_msg_sim_ps_id_enum sim)
{
    int re_attach = 0, current_rat = 0, apn_cnt = 0, ia_ip_type = 0, ia_roaming_type = 0;
    char apn[MIPC_MAX_APN_LEN] = {0};
    char ia_apn[MIPC_MAX_APN_LEN] = {0};
    char userid[MIPC_MAX_USERID_LEN] = {'\0'};
    char password[MIPC_MAX_PASSWORD_LEN] = {'\0'};
    uint8_t nw_status = MIPC_NW_REGISTER_STATE_NOT_REGISTERED;

    if (!check_sim_id(sim))
        return;

    LOGD("SIM%d check_ia\n", sim);
    if (apn_ptr == NULL) {
        LOGE("APN param is NULL\n");
        return;
    }
    strncpy(apn, apn_ptr, MIPC_MAX_APN_LEN - 1);
    apn[MIPC_MAX_APN_LEN - 1] = '\0';

    sync_modem_data_setting(data_roaming, sim);
    set_perf_data_simslot(sim);

    /* Check RAT Type */
    if (rat_type > 0) {
        current_rat = nw_get_rat(sim);
        if (current_rat != rat_type) {
            nw_set_rat(rat_type, sim);
            nw_set_reg(rat_type, sim);
            re_attach = 1;
        }
    }

    if (ip_type > 3 || ip_type <= 0)
        ip_type = MIPC_APN_PDP_TYPE_IPV4V6;

    if (roaming_type > 3 || roaming_type <= 0)
        roaming_type = MIPC_APN_PDP_TYPE_IPV4V6;

    if (userid_ptr != NULL) {
        strncpy(userid, userid_ptr, MIPC_MAX_USERID_LEN - 1);
    }
    
    if (password_ptr != NULL) {
        strncpy(password, password_ptr, MIPC_MAX_PASSWORD_LEN - 1);
    }
 
    if (apn[0] == '0')
    {
        apn_ia_set(NULL, ip_type, roaming_type, userid, password, auth_type, bearer_bitmask, sim);
        re_attach = 1;
    }
    else
    {
    apn_cnt = apn_ia_get(ia_apn, &ia_ip_type, &ia_roaming_type, sim);
    if (apn_cnt == 0 || (apn_cnt > 0 && strcasecmp(ia_apn, apn) != 0) || 
        (ip_type != ia_ip_type) || (roaming_type != ia_roaming_type)) {
        apn_ia_set(apn, ip_type, roaming_type, userid, password, auth_type, bearer_bitmask, sim);
        re_attach = 1;
    }
    }

#if 0
    nw_status = show_register_status();
    if (nw_status != MIPC_NW_REGISTER_STATE_HOME && nw_status != MIPC_NW_REGISTER_STATE_ROAMING)
        re_attach = 1;
#endif

    LOGD("ia_apn:%s:%s apn_cnt:%d nw_status:%d rat_type:%d:%d ip_type:%d:%d roaming_type:%d:%d re_attach:%d sim:%d\n",
        ia_apn, apn, apn_cnt, nw_status, current_rat, rat_type, ia_ip_type, ip_type, ia_roaming_type, roaming_type, re_attach, sim);

    if (re_attach)
        nw_radio_sw_state_set("0", sim);
    nw_radio_sw_state_set("1", sim);
}

void apn_ia_check(char* param)
{
    json_object *obj;
    json_object *pdn_obj;
    char *apn_ptr = NULL, *username_ptr = NULL, *password_ptr = NULL;
    char *bearer_bitmask_string = NULL, *endptr = NULL;
    int ip_type, auth_type, rat_type, roaming_type, data_roaming, sim;
    int re_attach = 0, current_rat = 0, apn_cnt = 0, ia_ip_type = 0, ia_roaming_type = 0;
    char apn[MIPC_MAX_APN_LEN] = {0};
    char ia_apn[MIPC_MAX_APN_LEN] = {0};
    char username[MIPC_MAX_USERID_LEN] = {'\0'};
    char password[MIPC_MAX_PASSWORD_LEN] = {'\0'};
    uint32_t bearer_bitmask = MIPC_DFLT_BEARER_BITMASK;


    pdn_obj = json_object_new_object();
    if (pdn_obj == NULL)
    {
        LOGE("Failed to allocate json object\n");
        return;
    }

    obj = json_tokener_parse(param);
    if (obj == NULL || json_object_get_type(obj) != json_type_object)
    {
        LOGE("Failed to parse json data\n");
        goto error;
    }

    apn_ptr = get_string(obj, "apn");
    if (apn_ptr == NULL)
    {
        LOGE("APN param is NULL\n");
        json_object_put(obj);
        goto error;
    }
    strncpy(apn, apn_ptr, MIPC_MAX_APN_LEN - 1);
    apn[MIPC_MAX_APN_LEN - 1] = '\0';

    rat_type = get_int(obj, "rat_type", 21); // 21: NR + LTE + UMTS
    ip_type = get_int(obj, "ip_type", MIPC_APN_PDP_TYPE_IPV4V6);
    roaming_type = get_int(obj, "roaming_type", MIPC_APN_PDP_TYPE_IPV4V6);
    auth_type = get_int(obj, "auth_type", MIPC_APN_AUTH_TYPE_NONE);
    data_roaming = get_int(obj, "data_roaming", 0);
    sim = get_int(obj, "sim", MIPC_PS0);
    if (!check_sim_id(sim))
        goto error;

    LOGD("SIM%d apn_ia_check\n", sim);

    username_ptr = get_string(obj, "username");
    password_ptr = get_string(obj, "password");
    bearer_bitmask_string = get_string(obj, "bearer_bitmask");

    if (bearer_bitmask_string)
    {
        LOGD("bearer_bitmask_string: %s\n", bearer_bitmask_string);
        errno = 0;
        bearer_bitmask = strtoll(bearer_bitmask_string, &endptr, 16);
        if (errno == ERANGE || errno == EINVAL) {
            LOGD("strtol return error: %d\n", errno);
            goto error;
        }
    }

    sync_modem_data_setting(data_roaming, sim);
    set_perf_data_simslot(sim);

    /* Check RAT Type */
    if (rat_type > 0) {
        current_rat = nw_get_rat(sim);
        if (current_rat != rat_type) {
            nw_set_rat(rat_type, sim);
            nw_set_reg(rat_type, sim);
            re_attach = 1;
        }
    }

    if (ip_type > 3 || ip_type <= 0)
        ip_type = MIPC_APN_PDP_TYPE_IPV4V6;

    if (roaming_type > 3 || roaming_type <= 0)
        roaming_type = MIPC_APN_PDP_TYPE_IPV4V6;

    if (username_ptr != NULL) {
        strncpy(username, username_ptr, MIPC_MAX_USERID_LEN - 1);
    }

    if (password_ptr != NULL) {
        strncpy(password, password_ptr, MIPC_MAX_PASSWORD_LEN - 1);
    }

    apn_cnt = apn_ia_get(ia_apn, &ia_ip_type, &ia_roaming_type, sim);
    if (apn_cnt == 0 || (apn_cnt > 0 && strcasecmp(ia_apn, apn) != 0) ||
        (ip_type != ia_ip_type) || (roaming_type != ia_roaming_type)) {
        apn_ia_set(apn, ip_type, roaming_type, username, password, auth_type, bearer_bitmask, sim);
        re_attach = 1;
    }

    LOGD("apn(ia:new):%s:%s apn_cnt:%d rat_type(cr:new):%d:%d ip_type(ia:new):%d:%d roaming_type(ia:new):%d:%d re_attach:%d sim:%d\n",
        ia_apn, apn, apn_cnt, current_rat, rat_type, ia_ip_type, ip_type, ia_roaming_type, roaming_type, re_attach, sim);

    if (re_attach)
        nw_radio_sw_state_set("0", sim);
    nw_radio_sw_state_set("1", sim);

error:
    if (obj)
        json_object_put(obj);
    json_object_put(pdn_obj);
}

void mipc_apn_profile_update(
        mipc_msg_sim_ps_id_enum sim,
        char *plmnid_ptr,
        char *apn_ptr,
        mipc_apn_type_const_enum apn_type,
        mipc_apn_pdp_type_const_enum pdp_type,
        mipc_apn_pdp_type_const_enum roaming_type,
        mipc_apn_auth_type_const_enum auth_type,
        char *userid_ptr, char *password_ptr,
        uint32_t bearer_bitmask,
        mipc_apn_compression_const_enum compression)
{
    mipc_msg_t *msg_req_ptr;
    mipc_msg_t *msg_cnf_ptr;
    mipc_result_enum result;

    if (!check_sim_id(sim))
        return;

    msg_req_ptr = mipc_msg_init(MIPC_APN_ADD_PROFILE_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    mipc_msg_add_tlv_uint32(msg_req_ptr, MIPC_APN_ADD_PROFILE_REQ_T_ID, 1);
    if (plmnid_ptr) {
        mipc_msg_add_tlv(msg_req_ptr, MIPC_APN_ADD_PROFILE_REQ_T_PLMN_ID, 7, plmnid_ptr);
    }
    if (apn_ptr) {
        mipc_msg_add_tlv(msg_req_ptr, MIPC_APN_ADD_PROFILE_REQ_T_APN, 100, apn_ptr);
    }
    mipc_msg_add_tlv_uint8(msg_req_ptr, MIPC_APN_ADD_PROFILE_REQ_T_APN_TYPE, apn_type);
    mipc_msg_add_tlv_uint8(msg_req_ptr, MIPC_APN_ADD_PROFILE_REQ_T_PDP_TYPE, pdp_type);
    mipc_msg_add_tlv_uint8(msg_req_ptr, MIPC_APN_ADD_PROFILE_REQ_T_ROAMING_TYPE, roaming_type);
    mipc_msg_add_tlv_uint8(msg_req_ptr, MIPC_APN_ADD_PROFILE_REQ_T_AUTH_TYPE, auth_type);
    if (userid_ptr) {
        mipc_msg_add_tlv(msg_req_ptr, MIPC_APN_ADD_PROFILE_REQ_T_USERID, MIPC_MAX_USERID_LEN, userid_ptr);
    }
    if (password_ptr) {
        mipc_msg_add_tlv(msg_req_ptr, MIPC_APN_ADD_PROFILE_REQ_T_PASSWORD, MIPC_MAX_PASSWORD_LEN, password_ptr);
    }
    mipc_msg_add_tlv_uint32(msg_req_ptr, MIPC_APN_ADD_PROFILE_REQ_T_BEARER_BITMASK, bearer_bitmask);
    mipc_msg_add_tlv_uint8(msg_req_ptr, MIPC_APN_ADD_PROFILE_REQ_T_COMPRESSION, compression);

    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);

    if (result == MIPC_RESULT_SUCCESS)  // modem should define the return value enum.
    {
        printf("mipc_apn_profile_update success: %s, result: %d\n", apn_ptr, result);
        LOGD("mipc_apn_profile_update success: %s, result: %d\n", apn_ptr, result);
    }
    else
    {
        printf("mipc_apn_profile_update fail: %s. result: %d\n", apn_ptr, result);
        LOGD("mipc_apn_profile_update fail: %s. result: %d\n", apn_ptr, result);
    }
    mipc_msg_deinit(msg_req_ptr);
}

void apn_profile_set(char* apn_ptr, mipc_msg_sim_ps_id_enum sim)
{
    mipc_msg_t* msg_req_ptr;
    mipc_msg_t* msg_cnf_ptr;
    char apn[MIPC_MAX_APN_LEN] = {0};
    char *temp_ptr;
    uint8_t mnc_len;
    char mcc_mnc[7] = {0};
    int n; //check snprintf error status

    if (!check_sim_id(sim))
        return;

    if (apn_ptr == NULL) {
        LOGE("APN param is NULL\n");
        return;
    }
    strncpy(apn, apn_ptr, MIPC_MAX_APN_LEN - 1);
    apn[MIPC_MAX_APN_LEN - 1] = '\0';

    // Get mcc mnc
    msg_req_ptr = mipc_msg_init(MIPC_SIM_IMSI_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    //no TLV for this message, send to modem directly
    msg_cnf_ptr = mipc_msg_sync(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    //get result
    //IMSI
    temp_ptr = (char *)mipc_msg_get_val_ptr(msg_cnf_ptr, MIPC_SIM_IMSI_CNF_T_IMSI, NULL);
    if (temp_ptr) {
        printf("IMSI: %s\n", temp_ptr);
    }
    mnc_len = mipc_msg_get_val_uint8(msg_cnf_ptr, MIPC_SIM_IMSI_CNF_T_MNC_LEN, 0xff);
    if (mnc_len != 2 && mnc_len != 3) {
        LOGD("apn_profile_set invalid mnc_len = %d", mnc_len);
    } else {
        n = snprintf(mcc_mnc, (4 + mnc_len), "%s", temp_ptr);
        if (n < 0 || n >= (4 + mnc_len)){
            LOGD("snprintf return error\n");
        } else {
            LOGD("SIM%d apn_profile_set imsi: %s, mccmnc: %s, apn: %s\n", sim, temp_ptr, mcc_mnc, apn);

            if (strlen(apn) == 0) {
                mipc_apn_profile_update(
                        sim,
                        mcc_mnc,
                        (char *)"ims",
                        MIPC_APN_TYPE_IMS,
                        MIPC_APN_PDP_TYPE_IPV4V6,
                        MIPC_APN_PDP_TYPE_IPV4V6,
                        MIPC_APN_AUTH_TYPE_NONE,
                        (char *)"", (char *)"",
                        0xffffffff,
                        MIPC_APN_COMPRESSION_ENABLE);
            } else {
                mipc_apn_profile_update(
                        sim,
                        mcc_mnc,
                        apn,
                        MIPC_APN_TYPE_IMS,
                        MIPC_APN_PDP_TYPE_IPV4V6,
                        MIPC_APN_PDP_TYPE_IPV4V6,
                        MIPC_APN_AUTH_TYPE_NONE,
                        (char *)"", (char *)"",
                        0xffffffff,
                        MIPC_APN_COMPRESSION_ENABLE);
            }
        } 
    }
    mipc_msg_deinit(msg_cnf_ptr);
}

void modem_poweroff(mipc_msg_sim_ps_id_enum sim) {
    mipc_msg_t* msg_req_ptr = NULL;
    mipc_msg_t* msg_cnf_ptr = NULL;
    int result = 0;

    if (!check_sim_id(sim))
        return;

    msg_req_ptr = mipc_msg_init(MIPC_NW_SET_RADIO_STATE_REQ, sim);
    if (msg_req_ptr == NULL) {
        LOGD("msg_req_ptr is NULL\n");
        return;
    }
    mipc_nw_set_radio_state_req_add_sw_state(msg_req_ptr, MIPC_NW_RADIO_STATE_OFF);

    /*  Modem will perform ECUSD/EMDT/EFUN/EPOF */
    mipc_nw_set_radio_state_req_add_cause(msg_req_ptr, 0x80);
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        printf("Power off MD done\n");
        LOGD("Power off MD done");
    } else {
        printf("Failed to power off MD\n");
        LOGD("Failed to power off MD");
    }
    mipc_msg_deinit(msg_cnf_ptr);
}

void ql_set_radio_on()
{
    LOGD("entry");

    run_at_cmd("AT+EFUN=1", MIPC_SIM0);

    if (nw_radio_state_get(0,MIPC_SIM0) == MIPC_NW_RADIO_STATE_OFF)
    {
        LOGD("retry to set radio on");
        run_at_cmd("AT+EFUN=1", MIPC_SIM0);
    }

    LOGD("exit");
}

void modem_poweroff_with_cause(char* param, mipc_msg_sim_ps_id_enum sim) {
    mipc_msg_t* msg_req_ptr = NULL;
    mipc_msg_t* msg_cnf_ptr = NULL;
    int result = 0;

    if (!check_sim_id(sim))
        return;

    msg_req_ptr = mipc_msg_init(MIPC_NW_SET_RADIO_STATE_REQ, sim);
    mipc_nw_set_radio_state_req_add_sw_state(msg_req_ptr, MIPC_NW_RADIO_STATE_OFF);

    /*  Modem will perform ECUSD/EMDT/EFUN/EPOF */
    mipc_nw_set_radio_state_req_add_cause(msg_req_ptr, atoi(param));
    msg_cnf_ptr = mipc_msg_sync_timeout(msg_req_ptr);
    mipc_msg_deinit(msg_req_ptr);

    result = mipc_get_result(msg_cnf_ptr);
    if (result == MIPC_RESULT_SUCCESS)
    {
        printf("Power off MD with cause:%d done\n", atoi(param));
        LOGD("Power off MD with cause:%d done", atoi(param));
    } else {
        printf("Failed to power off MD with cause:%d\n", atoi(param));
        LOGD("Failed to power off MD with cause:%d", atoi(param));
    }
    mipc_msg_deinit(msg_cnf_ptr);
}
int
main (int argc, char *argv[])
{
    int ch, ret;
    int option_index = 0, ip_type = 0, roaming_type = 0, id;
    char *endptr;
    pthread_condattr_t condattr;
    char config_str[8];
    uint32_t bitmask;

    openlog("mipc_cli", LOG_PID, LOG_USER);
    if(strcasecmp(argv[1],"--at_cmd") != 0)
    {
        if(strcasecmp(argv[1],"--ql_set_radio_on") != 0)
	{
    		ret = mipc_init("mipc_wan_cli");
	}
        else
	{
        	ret = mipc_init("sub_monitor");
	}
	if (ret) {
		LOGD("Fail to init MIPC:%d MIPC port maybe occupied\n", ret);
	        closelog();
	        return ret;
	}
    }
    ret = pthread_condattr_init(&condattr);
    if (ret < 0) {
        	LOGD("init condattr fail:%d\n", ret);
        	mipc_deinit();
        	closelog();
        	return ret;
    }
    pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
    pthread_cond_init(&event_queue_cond, &condattr);
    pthread_mutex_init(&event_queue_mutex, 0);

    memset(config_str, 0, sizeof(config_str));
    if (get_cfg_value("enable_sensitive_log", config_str, sizeof(config_str)))
        g_log_hidden = (atoi(config_str) != 1) ? 1 : 0;

	/*Add M & N options by Quectel*/
    while ((ch = getopt_long(argc, argv, "a:b:c:d:e:g:h:i:j:k:l:m:n:o:p:q:r:s:t:u:v:w:x:y:z:A:B:C:D:E:F:G:H:I:J:K:L:M:N:1:2:3:4:5:6:7:8:9:0", cmd_opts, &option_index)) != -1)
    {
        LOGD("mipc operation start, command: %c log: %d argc: %d\n", ch, g_log_hidden, argc);
        switch (ch)
        {
        case 'a':
            LOGD("argc:%d\n", argc);
            if (argc == 2)
                sim_get_mapping();
            else if (argc == 3)
                sim_set_mapping(argv[2]);
            break;
        case 'b':
            if (argc == 2)
                apn_provision_by_sim(MIPC_PS0);
            else if (argc == 3)
                apn_provision_by_sim(atoi(argv[2]));
            break;
        case 'c':
            errno = 0;
            id = (int)strtol(argv[2], &endptr, 10);
            if ((errno == ERANGE && (id == LONG_MAX || id == LONG_MIN))
                || (errno != 0 && id == 0))
            {
                LOGE("strtol out of range\n");
                break;
            }
            if (argv[2] == endptr || id < 0)
                LOGE("Can't find the active PDN for %s (%d) due to %d\n", argv[2], id, errno);
            else {
                if (argc == 3)
                    data_call_deact(id, MIPC_PS0);
                else if (argc == 4)
                    data_call_deact(id, atoi(argv[3]));
            }
            break;
        case 'd':
            if (argc == 3)
                data_call_act(optarg, MIPC_PS0);
            else if (argc == 4)
                data_call_act(optarg, atoi(argv[3]));
            break;
        case 'e':
            if (argc == 5)
                apn_ia_get(NULL, &ip_type, &roaming_type, MIPC_PS0);
            else if (argc == 6)
                apn_ia_get(NULL, &ip_type, &roaming_type, atoi(argv[5]));
            break;
        case 'f':
            if (argc == 3)
            	quectel_run_at_cmd(optarg);
            else if (argc == 4)
                run_at_cmd(argv[2], atoi(argv[3]));
            break;
            break;
        case 'g':
            get_imeisv();
            break;
        case 'i':
            if (argc == 2)
                sim_pin_info_get(MIPC_PS0);
            else if (argc == 3)
                sim_pin_info_get(atoi(argv[2]));
            break;
        case 'j':
            if (argc == 2)
                modem_poweroff(MIPC_PS0);
            else if (argc == 3)
                modem_poweroff(atoi(argv[2]));
            break;
        case 'k':
            if (argc == 3)
                check_ia(optarg, 0, 0, 0, NULL, NULL, 0, MIPC_DFLT_BEARER_BITMASK, 0, MIPC_PS0);
            else if (argc == 4)
                check_ia(argv[2], atoi(argv[3]), 0, 0, NULL, NULL, 0,MIPC_DFLT_BEARER_BITMASK, 0, MIPC_PS0);
            else if (argc == 5)
                check_ia(argv[2], atoi(argv[3]), atoi(argv[4]), 0, NULL, NULL, 0, MIPC_DFLT_BEARER_BITMASK, 0, MIPC_PS0);
            else if (argc == 6)
                check_ia(argv[2], atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), NULL, NULL, 0, MIPC_DFLT_BEARER_BITMASK, 0, MIPC_PS0);
            else if (argc == 11) {
                errno = 0;
                bitmask = strtol(argv[9], NULL, 16);
                LOGD("bitmask:0x%x errno:%d dataroaming:%d\n", bitmask, errno, atoi(argv[10]));
                if (errno != ERANGE && errno != EINVAL) {
                    check_ia(argv[2], atoi(argv[3]), atoi(argv[4]), atoi(argv[5]),
                             argv[6], argv[7], atoi(argv[8]), bitmask, atoi(argv[10]), MIPC_PS0);
                }
            } else
                LOGD("Invalid number of arguments: %d \n", argc);
            break;
        case 'l':
            if (argc == 3)
                data_call_deact_apn(optarg, MIPC_PS0);
            else if (argc == 4)
                data_call_deact_apn(argv[2], atoi(argv[3]));
            break;
        case 'm':
            if (argc == 3)
                sim_verify_pin(optarg, MIPC_PS0);
            else if (argc == 4)
                sim_verify_pin(argv[2], atoi(argv[3]));
            break;
        case 'n':
            LOGD("number of arguments:%d\n", argc);
            if (argc == 2)
                nw_radio_state_get(0, MIPC_PS0);            
            else if (argc == 3)
                nw_radio_state_get(0, atoi(argv[2]));
            break;
        case 'o':
            if (argc == 2)
                check_nw_status(MIPC_PS0);
            else if (argc == 3)
                check_nw_status(atoi(argv[2]));
            break;
        case 'p':
            if (argc == 3)
            	apn_ia_set(optarg, MIPC_APN_PDP_TYPE_IPV4V6, MIPC_APN_PDP_TYPE_IPV4V6,"", "", 0, MIPC_DFLT_BEARER_BITMASK, MIPC_PS0);
            else if (argc == 5)
                apn_ia_set(argv[2], atoi(argv[3]), atoi(argv[4]), "", "", 0, MIPC_DFLT_BEARER_BITMASK, MIPC_PS0);
            else if (argc == 9) {
                errno = 0;
                bitmask = strtol(argv[8], NULL, 16);
                LOGD("bitmask:0x%x errno:%d \n", bitmask, errno);
                if (errno != ERANGE && errno != EINVAL) {
                    apn_ia_set(argv[2], atoi(argv[3]), atoi(argv[4]), argv[5], argv[6], atoi(argv[7]),
                               bitmask, MIPC_PS0);
                }
            }
            else if (argc == 10) {
                errno = 0;
                bitmask = strtol(argv[8], NULL, 16);
                LOGD("SIM%d bitmask:0x%x errno:%d \n", atoi(argv[9]), bitmask, errno);  
                if (errno != ERANGE && errno != EINVAL) { 
                    apn_ia_set(argv[2], atoi(argv[3]), atoi(argv[4]), argv[5], argv[6], atoi(argv[7]), 
                               bitmask, atoi(argv[9]));
                }
            } else 
                LOGD("Invalid number of arguments: %d \n", argc);  
            break;
        case 'q':
            if (argc == 3)
                apn_profile_set(argv[2], MIPC_PS0);
            else if (argc == 4)
                apn_profile_set(argv[2], atoi(argv[3]));            
            break;
        case 'r':
            if (argc == 2)
                apn_profile_get(MIPC_PS0);
            else if (argc == 3)
                apn_profile_get(atoi(argv[2]));
            break;
        case 's':
            if (argc == 2)
                sim_get_state(1, MIPC_PS0);
            else if (argc == 3)
                sim_get_state(1, atoi(argv[2]));
            break;
            break;
        case 't':
            if (argc == 2)
                data_call_get(MIPC_PS0);
            else if (argc == 3)
                data_call_get(atoi(argv[2]));
            break;
        case 'u':
            get_imei();
            break;
        case 'v':
            if (argc == 3)
                data_call_act_type(optarg, MIPC_PS0);
            else if (argc == 4)
                data_call_act_type(argv[2], atoi(argv[3]));
            break;
        case 'w':
            if (argc == 3)
                nw_radio_sw_state_set(optarg, MIPC_PS0);
            else if (argc == 4)
                nw_radio_sw_state_set(argv[2], atoi(argv[3]));
            break;
        case 'x':
            if (argc == 3)
                modem_poweroff_with_cause(optarg, MIPC_PS0);
            else if (argc == 4)
                modem_poweroff_with_cause(argv[2], atoi(argv[3]));
            break;
        case 'y':
            apn_ia_check(optarg);
            break;
        case 'z':
            get_sim_insert();
            break;
        case 'A': // change hw radio state
            if (argc == 2)
                nw_change_hw_ratio(MIPC_PS0);
            else if (argc == 3)
                nw_change_hw_ratio(atoi(argv[2]));
            break;
        case 'B': // get rat
            if (argc == 2)
                nw_get_rat(MIPC_PS0);
            else if (argc == 3)
                nw_get_rat(atoi(argv[2]));
            break;
        case 'C': // set rat
            if (argc == 3)
                nw_set_rat(atoi(optarg), MIPC_PS0);
            else if (argc == 4)
                nw_set_rat(atoi(argv[2]), atoi(argv[3]));
            break;
        case 'D': // Get ims config
            if (argc == 2)
                ims_get_config(MIPC_PS0);
            else if (argc == 3)
                ims_get_config(atoi(argv[2]));
            break;
        case 'E': // Set ims config
            if (argc == 3)
                ims_set_config(optarg, MIPC_PS0);
            else if (argc == 4)
                ims_set_config(argv[2], atoi(argv[3]));
            break;
        case 'F': // Get ims Status
            if (argc == 2)
                ims_get_status(MIPC_PS0);
            else if (argc == 3)
                ims_get_status(atoi(argv[2]));
            break;
        case 'G': // Show sim status
            if (argc == 2)
                show_sim_status(MIPC_PS0);
            else if (argc == 3)
                show_sim_status(atoi(argv[2]));
            break;
        case 'H': // Show register status
            if (argc == 2)
                show_register_status(MIPC_PS0);
            else if (argc == 3)
                show_register_status(atoi(argv[2]));
            break;
#ifdef _VOICE_SUPPORT
        case 'I':
            //mipc_deinit();
            start_listen_dtmf(0);
            break;
        case 'J':
            // Set Speech
            break;
        case 'K':
            if (argc == 2)
                nw_show_register_rat(MIPC_PS0);
            else if (argc == 3)
                nw_show_register_rat(atoi(argv[2]));
            break;
        case 'L':
            if (argc == 2)
                nw_get_signal(MIPC_PS0);
            else if (argc == 3)
                nw_get_signal(atoi(argv[2]));
            break;
        case 'M':
            if (argc == 3)
                run_at_cmd(argv[2], MIPC_PS0);
            else if (argc == 4)
                run_at_cmd(argv[2], atoi(argv[3]));
            break;
        case 'N':
            ql_set_radio_on();
            break;
        case '1':
            test_mipc_get_call_status_req_hdlr();
            break;
        case '2':
            if (!g_log_hidden) {
                LOGD("dial number: %s\n", optarg);
                printf("dial number: %s\n", optarg);
            }
            test_mipc_dial_req_hdlr(1, optarg);
            break;
        case '3':
            mipc_deinit();
            start_listen_dtmf(1);
            break;
        case '4':
            test_mipc_answer_req_hdlr();
            break;
        case '5':
            test_mipc_ss_req_hdlr(2);
            break;
        case '6':
            test_mipc_conference_req_hdlr();
            break;
        case '7':
            break;
        case '8':
            test_mipc_ss_req_hdlr(2);
            break;
        case '9':
            test_mipc_ss_req_hdlr(1);
            break;
        case '0':
            test_mipc_hangup_req_hdlr();
            break;
#endif
        case 'h':
        default:
            usage();
            break;
        }
    }

    pthread_mutex_destroy(&event_queue_mutex);
    pthread_cond_destroy(&event_queue_cond);
    pthread_condattr_destroy(&condattr);

    LOGD("mipc operation end, command.");
    mipc_deinit();
    closelog();

    return 0;
}
