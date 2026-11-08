#ifndef _MIPC_CHECKER_H_
#define _MIPC_CHECKER_H_

typedef struct {
    uint16_t tlv_type;
    uint16_t option;
    union {
        struct {
            uint32_t min_value;
            uint32_t max_value;
        } min_max;
        struct {
            uint32_t fixed_value;
        } fixed;
        struct {
            uint32_t count_type;
            uint32_t struct_size;
        } count_struct_size;
        struct {
            uint32_t multiple_value;
        } multiple;
    } check_item;
} mipc_tlv_checker_info_t;

typedef struct {
    uint16_t msg_id;
    char *name_string;
    uint32_t checker_info_size;
    mipc_tlv_checker_info_t *checker_info;
} mipc_checker_info_t;

#define MIPC_CHECKER_TYPE_MASK (0x8000)
#define MIPC_CHECKER_TYPE_INT (0x8000)
#define MIPC_CHECKER_TYPE_NON_INT (0x0000)

#define MIPC_CHECKER_OPTION_MASK (0x1F00)
#define MIPC_CHECKER_OPTION_MIN (0x0100)
#define MIPC_CHECKER_OPTION_MAX (0x0200)
#define MIPC_CHECKER_OPTION_FIXED (0x0400)
#define MIPC_CHECKER_OPTION_COUNT_STRUCT (0x0800)
#define MIPC_CHECKER_OPTION_MULTIPLE (0x1000)

#define MIPC_CHECKER_INT_TYPE_MASK (0x0080)
#define MIPC_CHECKER_INT_TYPE_SIGNED (0x0080)
#define MIPC_CHECKER_INT_TYPE_UNSIGNED (0x0000)

#define MIPC_CHECKER_INT_SIZE_MASK (0x0007)
#define MIPC_CHECKER_INT_SIZE_8 (0x0001)
#define MIPC_CHECKER_INT_SIZE_16 (0x0002)
#define MIPC_CHECKER_INT_SIZE_32 (0x0004)


int mipc_check_int(mipc_msg_t *msg_ptr, mipc_msg_tlv_t *tlv_ptr, mipc_tlv_checker_info_t *tlv_checker_info);
int mipc_check_non_int(mipc_msg_t *msg_ptr, mipc_msg_tlv_t *tlv_ptr, mipc_tlv_checker_info_t *tlv_checker_info);
int mipc_check_non_int_value(uint32_t val, mipc_msg_t *msg_ptr, mipc_tlv_checker_info_t *tlv_checker_info);

mipc_tlv_checker_info_t mipc_sys_at_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_AT_REQ_T_ATCMD, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 2048},
};

mipc_tlv_checker_info_t mipc_sys_reboot_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_REBOOT_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
};

mipc_tlv_checker_info_t mipc_sys_get_mapping_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_GET_MAPPING_REQ_T_MSG, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 255},
};

mipc_tlv_checker_info_t mipc_sys_set_mapping_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_SET_MAPPING_REQ_T_MAPPING_LIST, .option = 0x800, .check_item.count_struct_size.count_type = MIPC_SYS_SET_MAPPING_REQ_T_MAPPING_COUNT, .check_item.count_struct_size.struct_size = 4},
};

mipc_tlv_checker_info_t mipc_sys_set_thermal_sensor_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_SET_THERMAL_SENSOR_REQ_T_CONFIG_LIST, .option = 0x1000, .check_item.multiple.multiple_value = 28},
    {.tlv_type = MIPC_SYS_SET_THERMAL_SENSOR_REQ_T_CONFIG_E, .option = 0x400, .check_item.fixed.fixed_value = 28},
};

mipc_tlv_checker_info_t mipc_sys_set_config_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_SET_CONFIG_REQ_T_CLASS, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 4},
    {.tlv_type = MIPC_SYS_SET_CONFIG_REQ_T_TYPE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 32},
    {.tlv_type = MIPC_SYS_SET_CONFIG_REQ_T_DATA, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 60000},
};

mipc_tlv_checker_info_t mipc_sys_get_config_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_GET_CONFIG_REQ_T_CLASS, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 4},
    {.tlv_type = MIPC_SYS_GET_CONFIG_REQ_T_TYPE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 32},
};

mipc_tlv_checker_info_t mipc_sys_reg_config_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_REG_CONFIG_REQ_T_CLASS, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 4},
    {.tlv_type = MIPC_SYS_REG_CONFIG_REQ_T_TYPE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 32},
};

mipc_tlv_checker_info_t mipc_sys_set_adpclk_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_SET_ADPCLK_REQ_T_STATE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_sys_write_nvram_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_WRITE_NVRAM_REQ_T_DATA, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 60000},
};

mipc_tlv_checker_info_t mipc_sys_auth_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_AUTH_REQ_T_ENCDATA, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 256},
};

mipc_tlv_checker_info_t mipc_sys_mcf_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_MCF_REQ_T_OP, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 11},
    {.tlv_type = MIPC_SYS_MCF_REQ_T_DUMP_LIDS, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 1024},
    {.tlv_type = MIPC_SYS_MCF_REQ_T_VALUE, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 512},
    {.tlv_type = MIPC_SYS_MCF_REQ_T_CONFIG1, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 1024},
    {.tlv_type = MIPC_SYS_MCF_REQ_T_CONFIG, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 1024},
};

mipc_tlv_checker_info_t mipc_sys_set_sar_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_SET_SAR_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_sys_set_geo_location_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_SET_GEO_LOCATION_REQ_T_ACCOUNT_ID, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
    {.tlv_type = MIPC_SYS_SET_GEO_LOCATION_REQ_T_BROADCAST_FLAG, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_SYS_SET_GEO_LOCATION_REQ_T_UE_WLAN_MAC, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SYS_SET_GEO_LOCATION_REQ_T_ALTITUDE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SYS_SET_GEO_LOCATION_REQ_T_ACCURACY_SEMI_MAJOR_AXIS, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SYS_SET_GEO_LOCATION_REQ_T_ACCURACY_SEMI_MINOR_AXIS, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SYS_SET_GEO_LOCATION_REQ_T_ACCURACY_VERTICAL_AXIS, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SYS_SET_GEO_LOCATION_REQ_T_LATITUDE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SYS_SET_GEO_LOCATION_REQ_T_LONGITUDE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SYS_SET_GEO_LOCATION_REQ_T_ACCURACY, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SYS_SET_GEO_LOCATION_REQ_T_METHOD, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SYS_SET_GEO_LOCATION_REQ_T_CITY, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SYS_SET_GEO_LOCATION_REQ_T_STATE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SYS_SET_GEO_LOCATION_REQ_T_ZIP, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SYS_SET_GEO_LOCATION_REQ_T_COUNTRY_CODE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_sys_set_dsbp_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_SET_DSBP_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 127},
};

mipc_tlv_checker_info_t mipc_sys_send_sar_ind_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_SEND_SAR_IND_REQ_T_CMD_PARAM, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 256},
};

mipc_tlv_checker_info_t mipc_sys_get_thermal_actuator_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_GET_THERMAL_ACTUATOR_REQ_T_GET_ACTUATOR_NUM, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_sys_get_thermal_sensor_runtime_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_GET_THERMAL_SENSOR_RUNTIME_REQ_T_GET_MD_AUTO_ENABLE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_sys_set_thermal_runtime_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_SET_THERMAL_RUNTIME_REQ_T_RESET_CFG_FROM_NV, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_SYS_SET_THERMAL_RUNTIME_REQ_T_MD_AUTO_ENABLE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_SYS_SET_THERMAL_RUNTIME_REQ_T_TRIP_CHANGE, .option = 0x400, .check_item.fixed.fixed_value = 16},
    {.tlv_type = MIPC_SYS_SET_THERMAL_RUNTIME_REQ_T_SAVE_CFG_TO_NV, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_sys_set_or_get_sbp_info_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_SET_OR_GET_SBP_INFO_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 8},
    {.tlv_type = MIPC_SYS_SET_OR_GET_SBP_INFO_REQ_T_FEATURE_STR, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_sys_hba_establish_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_HBA_ESTABLISH_REQ_T_PROXY_KEY, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SYS_HBA_ESTABLISH_REQ_T_V4_SRC_ADDR, .option = 0x400, .check_item.fixed.fixed_value = 4},
    {.tlv_type = MIPC_SYS_HBA_ESTABLISH_REQ_T_V4_DEST_ADDR, .option = 0x400, .check_item.fixed.fixed_value = 4},
    {.tlv_type = MIPC_SYS_HBA_ESTABLISH_REQ_T_V6_SRC_ADDR, .option = 0x400, .check_item.fixed.fixed_value = 16},
    {.tlv_type = MIPC_SYS_HBA_ESTABLISH_REQ_T_V6_DEST_ADDR, .option = 0x400, .check_item.fixed.fixed_value = 16},
    {.tlv_type = MIPC_SYS_HBA_ESTABLISH_REQ_T_IS_IPV6, .option = 0x8302, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_SYS_HBA_ESTABLISH_REQ_T_HB_PATTERN, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SYS_HBA_ESTABLISH_REQ_T_HB_ACK_PATTERN, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SYS_HBA_ESTABLISH_REQ_T_ALLOW_DYNAMIC_CYCLE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_sys_hba_ctrl_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_HBA_CTRL_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 127},
    {.tlv_type = MIPC_SYS_HBA_CTRL_REQ_T_PROXY_KEY, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_sys_hba_send_now_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_HBA_SEND_NOW_REQ_T_PROXY_KEY, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_sys_hba_resume_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_HBA_RESUME_REQ_T_PROXY_KEY, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_sys_mia_start_scenario_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_MIA_START_SCENARIO_REQ_T_SCENARIO_METRICS, .option = 0x400, .check_item.fixed.fixed_value = 16},
};

mipc_tlv_checker_info_t mipc_sys_mia_stop_scenario_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_MIA_STOP_SCENARIO_REQ_T_SCENARIO_METRICS, .option = 0x400, .check_item.fixed.fixed_value = 16},
};

mipc_tlv_checker_info_t mipc_sys_mia_update_metrics_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_MIA_UPDATE_METRICS_REQ_T_SCENARIO_METRICS, .option = 0x400, .check_item.fixed.fixed_value = 16},
};

mipc_tlv_checker_info_t mipc_sys_set_sleep_mode_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_SET_SLEEP_MODE_REQ_T_SLEEP_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_sys_send_consys_wifi_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_SEND_CONSYS_WIFI_REQ_T_PAYLOAD, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3072},
};

mipc_tlv_checker_info_t mipc_sys_send_consys_bt_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_SEND_CONSYS_BT_REQ_T_PAYLOAD, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3072},
};

mipc_tlv_checker_info_t mipc_apn_set_ia_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_APN_SET_IA_REQ_T_APN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 100},
    {.tlv_type = MIPC_APN_SET_IA_REQ_T_PDP_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 5},
    {.tlv_type = MIPC_APN_SET_IA_REQ_T_ROAMING_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 5},
    {.tlv_type = MIPC_APN_SET_IA_REQ_T_AUTH_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
    {.tlv_type = MIPC_APN_SET_IA_REQ_T_USERID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 64},
    {.tlv_type = MIPC_APN_SET_IA_REQ_T_PASSWORD, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 64},
    {.tlv_type = MIPC_APN_SET_IA_REQ_T_COMPRESSION, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_apn_add_profile_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_APN_ADD_PROFILE_REQ_T_COMPRESSION, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_APN_ADD_PROFILE_REQ_T_ENABLED, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_APN_ADD_PROFILE_REQ_T_PLMN_ID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 7},
    {.tlv_type = MIPC_APN_ADD_PROFILE_REQ_T_APN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 100},
    {.tlv_type = MIPC_APN_ADD_PROFILE_REQ_T_APN_TYPE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2147483648},
    {.tlv_type = MIPC_APN_ADD_PROFILE_REQ_T_PDP_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 5},
    {.tlv_type = MIPC_APN_ADD_PROFILE_REQ_T_ROAMING_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 5},
    {.tlv_type = MIPC_APN_ADD_PROFILE_REQ_T_AUTH_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
    {.tlv_type = MIPC_APN_ADD_PROFILE_REQ_T_USERID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 64},
    {.tlv_type = MIPC_APN_ADD_PROFILE_REQ_T_PASSWORD, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 64},
};

mipc_tlv_checker_info_t mipc_apn_set_profile_status_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_APN_SET_PROFILE_STATUS_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_apn_set_vzw_apn_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_APN_SET_VZW_APN_REQ_T_NETWORK_IDENTIFIER, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 100},
    {.tlv_type = MIPC_APN_SET_VZW_APN_REQ_T_PDP_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 5},
    {.tlv_type = MIPC_APN_SET_VZW_APN_REQ_T_APN_BEARER, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 10},
    {.tlv_type = MIPC_APN_SET_VZW_APN_REQ_T_ENABLED, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
};

mipc_tlv_checker_info_t mipc_apn_add_profile_list_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_APN_ADD_PROFILE_LIST_REQ_T_PROFILE_LIST, .option = 0x400, .check_item.fixed.fixed_value = 284},
};

mipc_tlv_checker_info_t mipc_apn_set_ia_md_prefer_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_APN_SET_IA_MD_PREFER_REQ_T_IA_MD_PREFER, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
};

mipc_tlv_checker_info_t mipc_data_act_call_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_ACT_CALL_REQ_T_APN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 100},
    {.tlv_type = MIPC_DATA_ACT_CALL_REQ_T_REUSE_FLAG, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
    {.tlv_type = MIPC_DATA_ACT_CALL_REQ_T_IF_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_DATA_ACT_CALL_REQ_T_URSP_TRAFFIC_DESC, .option = 0x400, .check_item.fixed.fixed_value = 672},
    {.tlv_type = MIPC_DATA_ACT_CALL_REQ_T_URSP_UE_LOCAL_CONF, .option = 0x400, .check_item.fixed.fixed_value = 108},
    {.tlv_type = MIPC_DATA_ACT_CALL_REQ_T_TSN_INFO, .option = 0x400, .check_item.fixed.fixed_value = 8212},
    {.tlv_type = MIPC_DATA_ACT_CALL_REQ_T_APN_TYPE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2147483648},
    {.tlv_type = MIPC_DATA_ACT_CALL_REQ_T_PDP_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 5},
    {.tlv_type = MIPC_DATA_ACT_CALL_REQ_T_ROAMING_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 5},
    {.tlv_type = MIPC_DATA_ACT_CALL_REQ_T_AUTH_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
    {.tlv_type = MIPC_DATA_ACT_CALL_REQ_T_USERID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 64},
    {.tlv_type = MIPC_DATA_ACT_CALL_REQ_T_PASSWORD, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 64},
    {.tlv_type = MIPC_DATA_ACT_CALL_REQ_T_IPV4V6_FALLBACK, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 127},
};

mipc_tlv_checker_info_t mipc_data_deact_call_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_DEACT_CALL_REQ_T_DEACT_REASON, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 127},
};

mipc_tlv_checker_info_t mipc_data_set_packet_filter_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_SET_PACKET_FILTER_REQ_T_FILTER_LIST, .option = 0x800, .check_item.count_struct_size.count_type = MIPC_DATA_SET_PACKET_FILTER_REQ_T_FILTER_COUNT, .check_item.count_struct_size.struct_size = 388},
};

mipc_tlv_checker_info_t mipc_data_get_pco_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_GET_PCO_REQ_T_APN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 100},
    {.tlv_type = MIPC_DATA_GET_PCO_REQ_T_PCO_IE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 5},
};

mipc_tlv_checker_info_t mipc_data_set_config_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_SET_CONFIG_REQ_T_MOBILE_DATA, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_DATA_SET_CONFIG_REQ_T_DATA_ROAMING, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_DATA_SET_CONFIG_REQ_T_VOLTE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_DATA_SET_CONFIG_REQ_T_IMS_TEST_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_DATA_SET_CONFIG_REQ_T_DATA_DOMESTIC_ROAMING, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_DATA_SET_CONFIG_REQ_T_DATA_INTERNATIONAL_ROAMING, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
};

mipc_tlv_checker_info_t mipc_data_abort_call_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_ABORT_CALL_REQ_T_APN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 100},
};

mipc_tlv_checker_info_t mipc_data_get_call_info_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_GET_CALL_INFO_REQ_T_APN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 100},
};

mipc_tlv_checker_info_t mipc_data_retry_timer_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_RETRY_TIMER_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_DATA_RETRY_TIMER_REQ_T_APN_NAME, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 100},
};

mipc_tlv_checker_info_t mipc_data_set_link_capacity_reporting_criteria_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_SET_LINK_CAPACITY_REPORTING_CRITERIA_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_DATA_SET_LINK_CAPACITY_REPORTING_CRITERIA_REQ_T_THRESHOLD_DL_KBPS_LIST, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 60},
    {.tlv_type = MIPC_DATA_SET_LINK_CAPACITY_REPORTING_CRITERIA_REQ_T_THRESHOLD_UL_KBPS_LIST, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 60},
};

mipc_tlv_checker_info_t mipc_data_set_lgdcont_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_SET_LGDCONT_REQ_T_APN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 100},
    {.tlv_type = MIPC_DATA_SET_LGDCONT_REQ_T_REQUEST_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 4},
    {.tlv_type = MIPC_DATA_SET_LGDCONT_REQ_T_RAT_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
};

mipc_tlv_checker_info_t mipc_data_set_nssai_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_SET_NSSAI_REQ_T_APN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 100},
    {.tlv_type = MIPC_DATA_SET_NSSAI_REQ_T_SNSSAI, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_data_keepalive_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_KEEPALIVE_REQ_T_START_KEEPALIVE, .option = 0x400, .check_item.fixed.fixed_value = 156},
};

mipc_tlv_checker_info_t mipc_data_set_psi_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_SET_PSI_REQ_T_ACTION, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 127},
    {.tlv_type = MIPC_DATA_SET_PSI_REQ_T_APN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 100},
};

mipc_tlv_checker_info_t mipc_data_get_nssai_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_GET_NSSAI_REQ_T_NSSAI_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 127},
    {.tlv_type = MIPC_DATA_GET_NSSAI_REQ_T_PLMN_ID, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
};

mipc_tlv_checker_info_t mipc_data_set_ursp_preconf_ue_policy_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_SET_URSP_PRECONF_UE_POLICY_REQ_T_PLMN_ID, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
    {.tlv_type = MIPC_DATA_SET_URSP_PRECONF_UE_POLICY_REQ_T_RULE_LIST, .option = 0x400, .check_item.fixed.fixed_value = 16},
};

mipc_tlv_checker_info_t mipc_data_get_ursp_ue_policy_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_GET_URSP_UE_POLICY_REQ_T_PLMN_ID, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
};

mipc_tlv_checker_info_t mipc_data_set_reserved_if_id_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_SET_RESERVED_IF_ID_REQ_T_IF_ID_LIST, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 20},
};

mipc_tlv_checker_info_t mipc_data_set_pco_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_SET_PCO_REQ_T_APN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 100},
    {.tlv_type = MIPC_DATA_SET_PCO_REQ_T_PCO_LIST, .option = 0x400, .check_item.fixed.fixed_value = 88},
};

mipc_tlv_checker_info_t mipc_data_mod_call_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_MOD_CALL_REQ_T_4GQOS, .option = 0x400, .check_item.fixed.fixed_value = 28},
    {.tlv_type = MIPC_DATA_MOD_CALL_REQ_T_5GQOS, .option = 0x400, .check_item.fixed.fixed_value = 56},
    {.tlv_type = MIPC_DATA_MOD_CALL_REQ_T_TFT_TLV_ARRAY, .option = 0x400, .check_item.fixed.fixed_value = 164},
};

mipc_tlv_checker_info_t mipc_data_set_tsn_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_SET_TSN_REQ_T_PMIC, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 8192},
};

mipc_tlv_checker_info_t mipc_data_set_ip_tuple_packet_filter_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_SET_IP_TUPLE_PACKET_FILTER_REQ_T_IP_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 127},
    {.tlv_type = MIPC_DATA_SET_IP_TUPLE_PACKET_FILTER_REQ_T_PROTOCOL, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 127},
    {.tlv_type = MIPC_DATA_SET_IP_TUPLE_PACKET_FILTER_REQ_T_IP_SRC, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_DATA_SET_IP_TUPLE_PACKET_FILTER_REQ_T_IP_DST, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_data_act_dedicated_call_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_ACT_DEDICATED_CALL_REQ_T_4GQOS, .option = 0x400, .check_item.fixed.fixed_value = 28},
    {.tlv_type = MIPC_DATA_ACT_DEDICATED_CALL_REQ_T_5GQOS, .option = 0x400, .check_item.fixed.fixed_value = 56},
    {.tlv_type = MIPC_DATA_ACT_DEDICATED_CALL_REQ_T_TFT_TLV_ARRAY, .option = 0x400, .check_item.fixed.fixed_value = 164},
};

mipc_tlv_checker_info_t mipc_internal_open_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_INTERNAL_OPEN_REQ_T_CLIENT_NAME, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 160},
    {.tlv_type = MIPC_INTERNAL_OPEN_REQ_T_USIR_SUPPORT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_internal_unregister_ind_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_INTERNAL_UNREGISTER_IND_REQ_T_UNREG_ALL, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_internal_set_filter_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_INTERNAL_SET_FILTER_REQ_T_STRUCT, .option = 0x400, .check_item.fixed.fixed_value = 68},
};

mipc_tlv_checker_info_t mipc_internal_reset_filter_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_INTERNAL_RESET_FILTER_REQ_T_STRUCT, .option = 0x400, .check_item.fixed.fixed_value = 12},
};

mipc_tlv_checker_info_t mipc_internal_eif_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_INTERNAL_EIF_REQ_T_CMD, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 8},
    {.tlv_type = MIPC_INTERNAL_EIF_REQ_T_NEW_ADDR, .option = 0x400, .check_item.fixed.fixed_value = 32},
    {.tlv_type = MIPC_INTERNAL_EIF_REQ_T_OLD_ADDR, .option = 0x400, .check_item.fixed.fixed_value = 32},
    {.tlv_type = MIPC_INTERNAL_EIF_REQ_T_ADDRESS_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
};

mipc_tlv_checker_info_t mipc_internal_set_long_standby_monitor_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_INTERNAL_SET_LONG_STANDBY_MONITOR_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_nw_set_radio_state_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_RADIO_STATE_REQ_T_SW_STATE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_NW_SET_RADIO_STATE_REQ_T_CAUSE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 132},
    {.tlv_type = MIPC_NW_SET_RADIO_STATE_REQ_T_HW_STATE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_nw_set_register_state_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_REGISTER_STATE_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 7},
    {.tlv_type = MIPC_NW_SET_REGISTER_STATE_REQ_T_OPER, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_NW_SET_REGISTER_STATE_REQ_T_ACT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 13},
};

mipc_tlv_checker_info_t mipc_nw_set_ps_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_PS_REQ_T_ACTION, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_NW_SET_PS_REQ_T_CTRL_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_nw_set_signal_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_SIGNAL_REQ_T_THRESHOLD_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_nw_set_preferred_provider_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_PREFERRED_PROVIDER_REQ_T_LIST, .option = 0x1000, .check_item.multiple.multiple_value = 144},
};

mipc_tlv_checker_info_t mipc_nw_set_home_provider_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_HOME_PROVIDER_REQ_T_PROVIDER, .option = 0x400, .check_item.fixed.fixed_value = 144},
};

mipc_tlv_checker_info_t mipc_nw_set_idle_hint_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_IDLE_HINT_REQ_T_STATUS, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
};

mipc_tlv_checker_info_t mipc_nw_set_rat_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_RAT_REQ_T_BIT_RAT, .option = 0x8304, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_nw_get_provider_name_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_GET_PROVIDER_NAME_REQ_T_PLMN_ID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 7},
};

mipc_tlv_checker_info_t mipc_nw_set_nr_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_NR_REQ_T_ACT_OPERATION, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
};

mipc_tlv_checker_info_t mipc_nw_get_band_mode_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_GET_BAND_MODE_REQ_T_OPTION, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_nw_set_band_mode_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_BAND_MODE_REQ_T_LTE_BAND, .option = 0x400, .check_item.fixed.fixed_value = 32},
    {.tlv_type = MIPC_NW_SET_BAND_MODE_REQ_T_NR_BAND, .option = 0x400, .check_item.fixed.fixed_value = 128},
    {.tlv_type = MIPC_NW_SET_BAND_MODE_REQ_T_NR_NSA_BAND, .option = 0x400, .check_item.fixed.fixed_value = 128},
};

mipc_tlv_checker_info_t mipc_nw_set_channel_lock_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_CHANNEL_LOCK_REQ_T_CH_LOCK_INFO_LIST, .option = 0x1000, .check_item.multiple.multiple_value = 152},
    {.tlv_type = MIPC_NW_SET_CHANNEL_LOCK_REQ_T_CH_LOCK_INFO_LIST_V1, .option = 0x400, .check_item.fixed.fixed_value = 276},
};

mipc_tlv_checker_info_t mipc_nw_set_prefer_rat_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_PREFER_RAT_REQ_T_RAT_LIST, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 4},
};

mipc_tlv_checker_info_t mipc_nw_set_lte_carrier_aggregation_switch_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_LTE_CARRIER_AGGREGATION_SWITCH_REQ_T_STATUS, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_nw_set_ps_cs_registration_state_roaming_type_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_PS_CS_REGISTRATION_STATE_ROAMING_TYPE_REQ_T_PS_CS_REG_ROAMING_INFO, .option = 0x400, .check_item.fixed.fixed_value = 6},
};

mipc_tlv_checker_info_t mipc_nw_cell_measurement_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_CELL_MEASUREMENT_REQ_T_LTE_BAND, .option = 0x400, .check_item.fixed.fixed_value = 32},
    {.tlv_type = MIPC_NW_CELL_MEASUREMENT_REQ_T_NR_BAND, .option = 0x400, .check_item.fixed.fixed_value = 128},
    {.tlv_type = MIPC_NW_CELL_MEASUREMENT_REQ_T_SCAN_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_NW_CELL_MEASUREMENT_REQ_T_RAT, .option = 0x8302, .check_item.min_max.min_value = 4, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_nw_cell_band_white_list_lock_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_CELL_BAND_WHITE_LIST_LOCK_REQ_T_LTE_BAND, .option = 0x400, .check_item.fixed.fixed_value = 32},
    {.tlv_type = MIPC_NW_CELL_BAND_WHITE_LIST_LOCK_REQ_T_NR_BAND, .option = 0x400, .check_item.fixed.fixed_value = 128},
    {.tlv_type = MIPC_NW_CELL_BAND_WHITE_LIST_LOCK_REQ_T_LTE_CELL_V1_TLV_ARRAY, .option = 0x400, .check_item.fixed.fixed_value = 180},
    {.tlv_type = MIPC_NW_CELL_BAND_WHITE_LIST_LOCK_REQ_T_NR_CELL_V3_TLV_ARRAY, .option = 0x400, .check_item.fixed.fixed_value = 208},
    {.tlv_type = MIPC_NW_CELL_BAND_WHITE_LIST_LOCK_REQ_T_LTE_CELL_LIST, .option = 0x1000, .check_item.multiple.multiple_value = 180},
    {.tlv_type = MIPC_NW_CELL_BAND_WHITE_LIST_LOCK_REQ_T_NR_CELL_LIST, .option = 0x1000, .check_item.multiple.multiple_value = 184},
    {.tlv_type = MIPC_NW_CELL_BAND_WHITE_LIST_LOCK_REQ_T_NR_NSA_BAND, .option = 0x400, .check_item.fixed.fixed_value = 128},
};

mipc_tlv_checker_info_t mipc_nw_set_urc_enable_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_URC_ENABLE_REQ_T_TYPE, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_nw_set_search_store_frequency_info_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_SEARCH_STORE_FREQUENCY_INFO_REQ_T_PLMN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 7},
    {.tlv_type = MIPC_NW_SET_SEARCH_STORE_FREQUENCY_INFO_REQ_T_ARFCN, .option = 0x800, .check_item.count_struct_size.count_type = MIPC_NW_SET_SEARCH_STORE_FREQUENCY_INFO_REQ_T_COUNT, .check_item.count_struct_size.struct_size = 4},
};

mipc_tlv_checker_info_t mipc_nw_set_select_femtocell_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_SELECT_FEMTOCELL_REQ_T_PLMN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 7},
    {.tlv_type = MIPC_NW_SET_SELECT_FEMTOCELL_REQ_T_CSG_ID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 10},
};

mipc_tlv_checker_info_t mipc_nw_set_smart_rat_switch_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_SMART_RAT_SWITCH_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 9},
};

mipc_tlv_checker_info_t mipc_nw_get_smart_rat_switch_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_GET_SMART_RAT_SWITCH_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 9},
};

mipc_tlv_checker_info_t mipc_nw_set_ps_test_mode_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_PS_TEST_MODE_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 5},
};

mipc_tlv_checker_info_t mipc_nw_set_signal_report_criteria_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_SIGNAL_REPORT_CRITERIA_REQ_T_RAT, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 5},
    {.tlv_type = MIPC_NW_SET_SIGNAL_REPORT_CRITERIA_REQ_T_SIGNAL_MEASUREMENT, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 9},
    {.tlv_type = MIPC_NW_SET_SIGNAL_REPORT_CRITERIA_REQ_T_THRESHOLD, .option = 0x400, .check_item.fixed.fixed_value = 41},
};

mipc_tlv_checker_info_t mipc_nw_set_ca_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_CA_REQ_T_RAT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_NW_SET_CA_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_nw_get_lte_1xrtt_cell_list_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_GET_LTE_1XRTT_CELL_LIST_REQ_T_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_nw_get_ca_link_capability_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_GET_CA_LINK_CAPABILITY_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_nw_get_ca_link_enable_status_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_GET_CA_LINK_ENABLE_STATUS_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 2, .check_item.min_max.max_value = 3},
    {.tlv_type = MIPC_NW_GET_CA_LINK_ENABLE_STATUS_REQ_T_BAND_COMBO, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_nw_set_tm9_enable_status_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_TM9_ENABLE_STATUS_REQ_T_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_NW_SET_TM9_ENABLE_STATUS_REQ_T_STATUS, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_nw_get_omadm_conf_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_GET_OMADM_CONF_REQ_T_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 7},
};

mipc_tlv_checker_info_t mipc_nw_set_omadm_conf_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_OMADM_CONF_REQ_T_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 7},
    {.tlv_type = MIPC_NW_SET_OMADM_CONF_REQ_T_NODE_VALUE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 64},
};

mipc_tlv_checker_info_t mipc_nw_set_ca_link_enable_status_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_CA_LINK_ENABLE_STATUS_REQ_T_LIST_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_NW_SET_CA_LINK_ENABLE_STATUS_REQ_T_CA_COMB_LIST, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 1632},
};

mipc_tlv_checker_info_t mipc_nw_set_tuw_timer_length_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_TUW_TIMER_LENGTH_REQ_T_TUW, .option = 0x1000, .check_item.multiple.multiple_value = 5},
};

mipc_tlv_checker_info_t mipc_nw_os_id_update_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_OS_ID_UPDATE_REQ_T_OS_ID_LIST, .option = 0x400, .check_item.fixed.fixed_value = 16},
};

mipc_tlv_checker_info_t mipc_nw_start_network_scan_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_START_NETWORK_SCAN_REQ_T_SCAN_TYPE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_NW_START_NETWORK_SCAN_REQ_T_INCREMENTAL_RESULTS, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_NW_START_NETWORK_SCAN_REQ_T_RECORD_LIST, .option = 0x400, .check_item.fixed.fixed_value = 172},
    {.tlv_type = MIPC_NW_START_NETWORK_SCAN_REQ_T_PLMN_LIST, .option = 0x400, .check_item.fixed.fixed_value = 7},
};

mipc_tlv_checker_info_t mipc_nw_set_pref_nssai_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_PREF_NSSAI_REQ_T_PREFERRED_NSSAI_3GPP_LIST, .option = 0x400, .check_item.fixed.fixed_value = 16},
    {.tlv_type = MIPC_NW_SET_PREF_NSSAI_REQ_T_PREFERRED_NSSAI_NON3GPP_LIST, .option = 0x400, .check_item.fixed.fixed_value = 16},
};

mipc_tlv_checker_info_t mipc_nw_set_default_nssai_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_DEFAULT_NSSAI_REQ_T_DEFAULT_CONFIGURED_NSSAI_LIST, .option = 0x400, .check_item.fixed.fixed_value = 16},
};

mipc_tlv_checker_info_t mipc_nw_get_nssai_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_GET_NSSAI_REQ_T_NSSAI_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 127},
    {.tlv_type = MIPC_NW_GET_NSSAI_REQ_T_PLMN_ID, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
};

mipc_tlv_checker_info_t mipc_nw_set_5guc_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_5GUC_REQ_T_NSA_BAND, .option = 0x400, .check_item.fixed.fixed_value = 128},
    {.tlv_type = MIPC_NW_SET_5GUC_REQ_T_SA_BAND, .option = 0x400, .check_item.fixed.fixed_value = 128},
    {.tlv_type = MIPC_NW_SET_5GUC_REQ_T_BW_CHECK_ENABLE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_nw_set_cag_select_mode_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_CAG_SELECT_MODE_REQ_T_PLMN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 7},
};

mipc_tlv_checker_info_t mipc_nw_set_allowed_mcc_list_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_ALLOWED_MCC_LIST_REQ_T_ACTION, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_NW_SET_ALLOWED_MCC_LIST_REQ_T_ALLOWED_MCC_LIST, .option = 0x400, .check_item.fixed.fixed_value = 44},
};

mipc_tlv_checker_info_t mipc_sim_pin_protect_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_PIN_PROTECT_REQ_T_PIN_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 18},
    {.tlv_type = MIPC_SIM_PIN_PROTECT_REQ_T_PIN_OP, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_SIM_PIN_PROTECT_REQ_T_PIN_CODE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_sim_change_pin_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_CHANGE_PIN_REQ_T_PIN_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 18},
    {.tlv_type = MIPC_SIM_CHANGE_PIN_REQ_T_OLD_PIN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_SIM_CHANGE_PIN_REQ_T_NEW_PIN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_sim_verify_pin_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_VERIFY_PIN_REQ_T_PIN_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 18},
    {.tlv_type = MIPC_SIM_VERIFY_PIN_REQ_T_PIN_CODE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_sim_unblock_pin_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_UNBLOCK_PIN_REQ_T_PIN_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 18},
    {.tlv_type = MIPC_SIM_UNBLOCK_PIN_REQ_T_PUK_CODE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_SIM_UNBLOCK_PIN_REQ_T_PIN_CODE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_sim_imsi_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_IMSI_REQ_T_APP_ID, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
};

mipc_tlv_checker_info_t mipc_sim_open_channel_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_OPEN_CHANNEL_REQ_T_APP_ID, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 20},
};

mipc_tlv_checker_info_t mipc_sim_channel_restricted_access_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_CHANNEL_RESTRICTED_ACCESS_REQ_T_APP_ID, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
    {.tlv_type = MIPC_SIM_CHANNEL_RESTRICTED_ACCESS_REQ_T_FCP_CONVERT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_SIM_CHANNEL_RESTRICTED_ACCESS_REQ_T_CMD, .option = 0x8301, .check_item.min_max.min_value = 176, .check_item.min_max.max_value = 242},
    {.tlv_type = MIPC_SIM_CHANNEL_RESTRICTED_ACCESS_REQ_T_DATA, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 65535},
    {.tlv_type = MIPC_SIM_CHANNEL_RESTRICTED_ACCESS_REQ_T_PATH, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 24},
};

mipc_tlv_checker_info_t mipc_sim_channel_generic_access_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_CHANNEL_GENERIC_ACCESS_REQ_T_APP_ID, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
    {.tlv_type = MIPC_SIM_CHANNEL_GENERIC_ACCESS_REQ_T_APDU, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 261},
};

mipc_tlv_checker_info_t mipc_sim_long_apdu_access_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_LONG_APDU_ACCESS_REQ_T_APP_ID, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 20},
    {.tlv_type = MIPC_SIM_LONG_APDU_ACCESS_REQ_T_PATH_ID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 24},
    {.tlv_type = MIPC_SIM_LONG_APDU_ACCESS_REQ_T_LOCAL_PIN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_SIM_LONG_APDU_ACCESS_REQ_T_BINARY_DATA, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 65535},
};

mipc_tlv_checker_info_t mipc_sim_file_status_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_FILE_STATUS_REQ_T_AID, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 20},
    {.tlv_type = MIPC_SIM_FILE_STATUS_REQ_T_FILE_PATH, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 24},
};

mipc_tlv_checker_info_t mipc_sim_set_reset_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_SET_RESET_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_sim_set_terminal_capability_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_SET_TERMINAL_CAPABILITY_REQ_T_TC, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 256},
};

mipc_tlv_checker_info_t mipc_sim_set_pin_ex_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_SET_PIN_EX_REQ_T_PIN_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 18},
    {.tlv_type = MIPC_SIM_SET_PIN_EX_REQ_T_OP, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
    {.tlv_type = MIPC_SIM_SET_PIN_EX_REQ_T_PIN_CODE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_SIM_SET_PIN_EX_REQ_T_NEW_PIN_CODE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_SIM_SET_PIN_EX_REQ_T_AID, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 20},
};

mipc_tlv_checker_info_t mipc_sim_get_pin_ex_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_GET_PIN_EX_REQ_T_AID, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 20},
};

mipc_tlv_checker_info_t mipc_sim_get_gsm_auth_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_GET_GSM_AUTH_REQ_T_RAND1, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_SIM_GET_GSM_AUTH_REQ_T_RAND2, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_SIM_GET_GSM_AUTH_REQ_T_RAND3, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_sim_get_ext_auth_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_GET_EXT_AUTH_REQ_T_CMD_DATA, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 256},
    {.tlv_type = MIPC_SIM_GET_EXT_AUTH_REQ_T_APP_ID, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
};

mipc_tlv_checker_info_t mipc_sim_get_facility_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_GET_FACILITY_REQ_T_APP_ID, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
    {.tlv_type = MIPC_SIM_GET_FACILITY_REQ_T_FACILITY, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 3},
};

mipc_tlv_checker_info_t mipc_sim_set_facility_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_SET_FACILITY_REQ_T_APP_ID, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
    {.tlv_type = MIPC_SIM_SET_FACILITY_REQ_T_FACILITY, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 3},
    {.tlv_type = MIPC_SIM_SET_FACILITY_REQ_T_PASS_WORD, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_sim_access_profile_apdu_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_ACCESS_PROFILE_APDU_REQ_T_APDU, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 522},
};

mipc_tlv_checker_info_t mipc_sim_set_physical_slots_mapping_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_SET_PHYSICAL_SLOTS_MAPPING_REQ_T_SLOTS_MAPPING_LIST, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
};

mipc_tlv_checker_info_t mipc_sim_extended_channel_generic_access_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_EXTENDED_CHANNEL_GENERIC_ACCESS_REQ_T_DATA, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 65535},
};

mipc_tlv_checker_info_t mipc_sim_uicc_file_access_record_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_UICC_FILE_ACCESS_RECORD_REQ_T_APP_ID, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_SIM_UICC_FILE_ACCESS_RECORD_REQ_T_DATA, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SIM_UICC_FILE_ACCESS_RECORD_REQ_T_PATH, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 24},
    {.tlv_type = MIPC_SIM_UICC_FILE_ACCESS_RECORD_REQ_T_PIN2, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_sim_uicc_file_access_binary_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_UICC_FILE_ACCESS_BINARY_REQ_T_APP_ID, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_SIM_UICC_FILE_ACCESS_BINARY_REQ_T_DATA, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SIM_UICC_FILE_ACCESS_BINARY_REQ_T_PATH, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 24},
    {.tlv_type = MIPC_SIM_UICC_FILE_ACCESS_BINARY_REQ_T_PIN2, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_sim_get_sim_auth_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_GET_SIM_AUTH_REQ_T_AID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 40},
    {.tlv_type = MIPC_SIM_GET_SIM_AUTH_REQ_T_CMD_DATA, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 256},
};

mipc_tlv_checker_info_t mipc_sim_sml_set_allowed_carriers_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_SML_SET_ALLOWED_CARRIERS_REQ_T_ALLOWED_CARRIERS_PRIORITIZED, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_SIM_SML_SET_ALLOWED_CARRIERS_REQ_T_ALLOWED_CARRIERS, .option = 0x1000, .check_item.multiple.multiple_value = 30},
    {.tlv_type = MIPC_SIM_SML_SET_ALLOWED_CARRIERS_REQ_T_EXCLUDED_CARRIERS, .option = 0x800, .check_item.count_struct_size.count_type = MIPC_SIM_SML_SET_ALLOWED_CARRIERS_REQ_T_EXCLUDED_CARRIERS_NUM, .check_item.count_struct_size.struct_size = 30},
};

mipc_tlv_checker_info_t mipc_sim_sml_enter_sim_depersonalization_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_SML_ENTER_SIM_DEPERSONALIZATION_REQ_T_CATEGORY, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SIM_SML_ENTER_SIM_DEPERSONALIZATION_REQ_T_PIN_CODE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_sim_sml_get_lock_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_SML_GET_LOCK_REQ_T_CATEGORY, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
};

mipc_tlv_checker_info_t mipc_sim_sml_set_lock_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_SML_SET_LOCK_REQ_T_CATEGORY, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SIM_SML_SET_LOCK_REQ_T_OP, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 5},
    {.tlv_type = MIPC_SIM_SML_SET_LOCK_REQ_T_KEY, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SIM_SML_SET_LOCK_REQ_T_DATA_IMSI, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 20},
    {.tlv_type = MIPC_SIM_SML_SET_LOCK_REQ_T_GID1, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 3},
    {.tlv_type = MIPC_SIM_SML_SET_LOCK_REQ_T_GID2, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 3},
};

mipc_tlv_checker_info_t mipc_sim_sml_enter_device_depersonalization_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_SML_ENTER_DEVICE_DEPERSONALIZATION_REQ_T_KEY, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_sim_sml_rsu_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_SML_RSU_REQ_T_REQUEST_ID, .option = 0x8304, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 14},
    {.tlv_type = MIPC_SIM_SML_RSU_REQ_T_DATA, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 65535},
    {.tlv_type = MIPC_SIM_SML_RSU_REQ_T_RSV_STRING, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 65535},
};

mipc_tlv_checker_info_t mipc_sim_vsim_plug_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_VSIM_PLUG_REQ_T_SIM_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_sim_vsim_set_timer_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_VSIM_SET_TIMER_REQ_T_TIMER, .option = 0x8304, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 255},
};

mipc_tlv_checker_info_t mipc_sim_vsim_reset_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_VSIM_RESET_REQ_T_DATA, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 522},
};

mipc_tlv_checker_info_t mipc_sim_vsim_apdu_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_VSIM_APDU_REQ_T_DATA, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 522},
};

mipc_tlv_checker_info_t mipc_sim_vsim_auth_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SIM_VSIM_AUTH_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_sms_cfg_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SMS_CFG_REQ_T_FORMAT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_CFG_REQ_T_SET_SCA, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 32},
    {.tlv_type = MIPC_SMS_CFG_REQ_T_PREFER_STORAGE_C2K, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_CFG_REQ_T_W_R_D_STORAGE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_CFG_REQ_T_SAVE_SETTING, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_SMS_CFG_REQ_T_GET_SCA, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_SMS_CFG_REQ_T_PREFER_ACK, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_CFG_REQ_T_PREFER_STORAGE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_CFG_REQ_T_SET_HOST_MEM_AVAILABLE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_SMS_CFG_REQ_T_TEXT_MODE_PARAM_ACTION, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 255},
};

mipc_tlv_checker_info_t mipc_sms_send_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SMS_SEND_REQ_T_FORMAT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_SEND_REQ_T_PDU, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 256},
    {.tlv_type = MIPC_SMS_SEND_REQ_T_SAVE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_SEND_REQ_T_MORE_MSG_TO_SEND, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_SEND_REQ_T_PDU_C2K, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 256},
    {.tlv_type = MIPC_SMS_SEND_REQ_T_NUM_C2K, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 32},
};

mipc_tlv_checker_info_t mipc_sms_read_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SMS_READ_REQ_T_FORMAT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_READ_REQ_T_FLAG, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_READ_REQ_T_STATUS_UNCHANGE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_SMS_READ_REQ_T_STORAGE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
};

mipc_tlv_checker_info_t mipc_sms_delete_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SMS_DELETE_REQ_T_FLAG, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_DELETE_REQ_T_FORMAT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_DELETE_REQ_T_STORAGE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
};

mipc_tlv_checker_info_t mipc_sms_get_store_status_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SMS_GET_STORE_STATUS_REQ_T_STORAGE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_GET_STORE_STATUS_REQ_T_FORMAT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
};

mipc_tlv_checker_info_t mipc_sms_write_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SMS_WRITE_REQ_T_FORMAT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_WRITE_REQ_T_PDU, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 256},
    {.tlv_type = MIPC_SMS_WRITE_REQ_T_STORAGE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_WRITE_REQ_T_STATUS, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_WRITE_REQ_T_PDU_C2K, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 256},
};

mipc_tlv_checker_info_t mipc_sms_cbm_cfg_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SMS_CBM_CFG_REQ_T_MSG_ID_CFG_TYPE, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 253},
    {.tlv_type = MIPC_SMS_CBM_CFG_REQ_T_PDU_3GPP_SEG_ENABLE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_SMS_CBM_CFG_REQ_T_DCS_CFG_TYPE, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 253},
    {.tlv_type = MIPC_SMS_CBM_CFG_REQ_T_FORMAT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_CBM_CFG_REQ_T_C2K_CBM_ENABLE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_sms_scbm_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SMS_SCBM_REQ_T_QUIT_SCBM_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_sms_domain_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SMS_DOMAIN_REQ_T_CGSMS_VALUE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
    {.tlv_type = MIPC_SMS_DOMAIN_REQ_T_SMS_IMS_VALUE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_SMS_DOMAIN_REQ_T_CGSMS_ACTION, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_SMS_DOMAIN_REQ_T_SMS_IMS_ACTION, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 2},
};

mipc_tlv_checker_info_t mipc_ss_send_ussd_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SS_SEND_USSD_REQ_T_PAYLOAD, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 160},
    {.tlv_type = MIPC_SS_SEND_USSD_REQ_T_LANG, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 32},
};

mipc_tlv_checker_info_t mipc_ss_set_call_waiting_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SS_SET_CALL_WAITING_REQ_T_CW_ENABLE_DISABLE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_SS_SET_CALL_WAITING_REQ_T_SERVICE_CLASS, .option = 0x8302, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 512},
};

mipc_tlv_checker_info_t mipc_ss_query_call_waiting_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SS_QUERY_CALL_WAITING_REQ_T_SERVICE_CLASS, .option = 0x8302, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 512},
};

mipc_tlv_checker_info_t mipc_ss_set_call_forward_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SS_SET_CALL_FORWARD_REQ_T_SS_OPERATION, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 4},
    {.tlv_type = MIPC_SS_SET_CALL_FORWARD_REQ_T_CALL_FORWARD_REASON, .option = 0x8301, .check_item.min_max.min_value = 2, .check_item.min_max.max_value = 68},
    {.tlv_type = MIPC_SS_SET_CALL_FORWARD_REQ_T_DIAL_NUMBER, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 182},
    {.tlv_type = MIPC_SS_SET_CALL_FORWARD_REQ_T_SERVICE_CLASS, .option = 0x8302, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 512},
    {.tlv_type = MIPC_SS_SET_CALL_FORWARD_REQ_T_TIME_SLOT_BEGIN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SS_SET_CALL_FORWARD_REQ_T_TIME_SLOT_END, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_ss_query_call_forward_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SS_QUERY_CALL_FORWARD_REQ_T_SERVICE_CLASS, .option = 0x8302, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 512},
    {.tlv_type = MIPC_SS_QUERY_CALL_FORWARD_REQ_T_CALL_FORWARD_REASON, .option = 0x8301, .check_item.min_max.min_value = 2, .check_item.min_max.max_value = 68},
};

mipc_tlv_checker_info_t mipc_ss_set_call_barring_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SS_SET_CALL_BARRING_REQ_T_LOCK, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_SS_SET_CALL_BARRING_REQ_T_FACILITY, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 8},
    {.tlv_type = MIPC_SS_SET_CALL_BARRING_REQ_T_PASSWORD, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 4},
    {.tlv_type = MIPC_SS_SET_CALL_BARRING_REQ_T_SERVICE_CLASS, .option = 0x8302, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 512},
};

mipc_tlv_checker_info_t mipc_ss_query_call_barring_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SS_QUERY_CALL_BARRING_REQ_T_FACILITY, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 8},
    {.tlv_type = MIPC_SS_QUERY_CALL_BARRING_REQ_T_SERVICE_CLASS, .option = 0x8302, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 512},
};

mipc_tlv_checker_info_t mipc_ss_change_barring_password_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SS_CHANGE_BARRING_PASSWORD_REQ_T_FACILITY, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 8},
    {.tlv_type = MIPC_SS_CHANGE_BARRING_PASSWORD_REQ_T_OLD_PWD, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 9},
    {.tlv_type = MIPC_SS_CHANGE_BARRING_PASSWORD_REQ_T_NEW_PWD, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 9},
    {.tlv_type = MIPC_SS_CHANGE_BARRING_PASSWORD_REQ_T_NEW_PWD_CONFIRM, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 9},
};

mipc_tlv_checker_info_t mipc_ss_run_gba_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SS_RUN_GBA_REQ_T_NAF_FQDN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 256},
    {.tlv_type = MIPC_SS_RUN_GBA_REQ_T_NAF_SECURE_PROTOCOL_ID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 256},
    {.tlv_type = MIPC_SS_RUN_GBA_REQ_T_FORCE_RUN, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_ss_setup_xcap_user_agent_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SS_SETUP_XCAP_USER_AGENT_REQ_T_STR, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_ss_set_xcap_cfg_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_SS_SET_XCAP_CFG_REQ_T_CFG_NAME, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_SS_SET_XCAP_CFG_REQ_T_VALUE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_stk_set_pac_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_STK_SET_PAC_REQ_T_PAC_BITMASK_PTR, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 32},
};

mipc_tlv_checker_info_t mipc_stk_send_terminal_response_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_STK_SEND_TERMINAL_RESPONSE_REQ_T_TR_PTR_V2, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 512},
    {.tlv_type = MIPC_STK_SEND_TERMINAL_RESPONSE_REQ_T_TR_PTR, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 261},
};

mipc_tlv_checker_info_t mipc_stk_send_envelope_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_STK_SEND_ENVELOPE_REQ_T_ENVELOPE_PTR, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 261},
};

mipc_tlv_checker_info_t mipc_call_dial_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_DIAL_REQ_T_DIAL_ADDRESS, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 180},
    {.tlv_type = MIPC_CALL_DIAL_REQ_T_DIAL_ADDRESS_TYPE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_CALL_DIAL_REQ_T_TYPE, .option = 0x8304, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 4},
    {.tlv_type = MIPC_CALL_DIAL_REQ_T_DOMAIN, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
    {.tlv_type = MIPC_CALL_DIAL_REQ_T_ECC_RETRY_DOMAIN, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
    {.tlv_type = MIPC_CALL_DIAL_REQ_T_CLIR, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_CALL_DIAL_REQ_T_IS_ECC_TESTING, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_CALL_DIAL_REQ_T_CLIR_EXT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
};

mipc_tlv_checker_info_t mipc_call_ss_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_SS_REQ_T_ACTION, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 132},
    {.tlv_type = MIPC_CALL_SS_REQ_T_ECT_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
    {.tlv_type = MIPC_CALL_SS_REQ_T_ECT_NUMBER, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_call_hangup_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_HANGUP_REQ_T_MODE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_CALL_HANGUP_REQ_T_CAUSE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
};

mipc_tlv_checker_info_t mipc_call_answer_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_ANSWER_REQ_T_MODE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
};

mipc_tlv_checker_info_t mipc_call_conference_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_CONFERENCE_REQ_T_ACTION, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 4},
    {.tlv_type = MIPC_CALL_CONFERENCE_REQ_T_NUMBER, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_call_dtmf_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_DTMF_REQ_T_MODE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
    {.tlv_type = MIPC_CALL_DTMF_REQ_T_DIGIT, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_call_set_ecc_list_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_SET_ECC_LIST_REQ_T_INFO_LIST, .option = 0x800, .check_item.count_struct_size.count_type = MIPC_CALL_SET_ECC_LIST_REQ_T_INFO_COUNT, .check_item.count_struct_size.struct_size = 20},
};

mipc_tlv_checker_info_t mipc_call_ivs_onekey_ecall_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_IVS_ONEKEY_ECALL_REQ_T_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
    {.tlv_type = MIPC_CALL_IVS_ONEKEY_ECALL_REQ_T_MSD_FORMAT, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_CALL_IVS_ONEKEY_ECALL_REQ_T_MSD, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 140},
    {.tlv_type = MIPC_CALL_IVS_ONEKEY_ECALL_REQ_T_DOMAIN, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
};

mipc_tlv_checker_info_t mipc_call_set_sip_header_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_SET_SIP_HEADER_REQ_T_VALUE_PAIR, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 232},
};

mipc_tlv_checker_info_t mipc_call_set_call_additional_info_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_SET_CALL_ADDITIONAL_INFO_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 3},
    {.tlv_type = MIPC_CALL_SET_CALL_ADDITIONAL_INFO_REQ_T_TYPE, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_CALL_SET_CALL_ADDITIONAL_INFO_REQ_T_ADDITIONAL_INFO, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 1950},
};

mipc_tlv_checker_info_t mipc_call_peer_rtt_modify_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_PEER_RTT_MODIFY_REQ_T_RESULT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_call_local_rtt_modify_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_LOCAL_RTT_MODIFY_REQ_T_OP, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_call_rtt_text_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_RTT_TEXT_REQ_T_TEXT, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 1024},
};

mipc_tlv_checker_info_t mipc_call_rtt_mode_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_RTT_MODE_REQ_T_OP, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
};

mipc_tlv_checker_info_t mipc_call_set_rcs_state_and_feature_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_SET_RCS_STATE_AND_FEATURE_REQ_T_STATE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_CALL_SET_RCS_STATE_AND_FEATURE_REQ_T_FEATURE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 8192},
};

mipc_tlv_checker_info_t mipc_call_pull_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_PULL_REQ_T_URI, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_CALL_PULL_REQ_T_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_call_set_ims_call_mode_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_SET_IMS_CALL_MODE_REQ_T_OP, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 2},
};

mipc_tlv_checker_info_t mipc_call_set_tty_mode_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_SET_TTY_MODE_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
};

mipc_tlv_checker_info_t mipc_call_set_voice_domain_preference_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_SET_VOICE_DOMAIN_PREFERENCE_REQ_T_SETTING, .option = 0x8304, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 4},
};

mipc_tlv_checker_info_t mipc_call_conference_dial_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_CONFERENCE_DIAL_REQ_T_TYPE, .option = 0x8304, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_CALL_CONFERENCE_DIAL_REQ_T_CLIR, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_CALL_CONFERENCE_DIAL_REQ_T_DIAL_ADDRESS, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 180},
    {.tlv_type = MIPC_CALL_CONFERENCE_DIAL_REQ_T_CLIR_EXT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
};

mipc_tlv_checker_info_t mipc_call_set_gwsd_mode_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_SET_GWSD_MODE_REQ_T_ACTION, .option = 0x8304, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_CALL_SET_GWSD_MODE_REQ_T_KA_CYCLE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_call_approve_csfb_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_APPROVE_CSFB_REQ_T_IS_APPROVE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_call_set_gwsd_ka_pdcp_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_SET_GWSD_KA_PDCP_REQ_T_PDATA, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_call_set_gwsd_ka_ipdata_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_SET_GWSD_KA_IPDATA_REQ_T_PDATA, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_call_uis_info_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_UIS_INFO_REQ_T_TYPE, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_CALL_UIS_INFO_REQ_T_DATA, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_call_data_prefer_set_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_DATA_PREFER_SET_REQ_T_STATE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_CALL_DATA_PREFER_SET_REQ_T_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_call_dc_create_sip_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_DC_CREATE_SIP_REQ_T_LOCAL_ADDR, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_CALL_DC_CREATE_SIP_REQ_T_REMOTE_TLS_ID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 256},
    {.tlv_type = MIPC_CALL_DC_CREATE_SIP_REQ_T_LOCAL_TLS_ID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 256},
    {.tlv_type = MIPC_CALL_DC_CREATE_SIP_REQ_T_REMOTE_FP_STR, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 192},
    {.tlv_type = MIPC_CALL_DC_CREATE_SIP_REQ_T_LOCAL_FP_STR, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 192},
    {.tlv_type = MIPC_CALL_DC_CREATE_SIP_REQ_T_QOS_HINT, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 64},
    {.tlv_type = MIPC_CALL_DC_CREATE_SIP_REQ_T_DCMAP_LABEL, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 80},
    {.tlv_type = MIPC_CALL_DC_CREATE_SIP_REQ_T_DCMAP_SUBPROTOCOL, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 10},
    {.tlv_type = MIPC_CALL_DC_CREATE_SIP_REQ_T_REMOTE_ADDR, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_call_dc_modify_sip_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_DC_MODIFY_SIP_REQ_T_LOCAL_ADDR, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_CALL_DC_MODIFY_SIP_REQ_T_REMOTE_TLS_ID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 256},
    {.tlv_type = MIPC_CALL_DC_MODIFY_SIP_REQ_T_LOCAL_TLS_ID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 256},
    {.tlv_type = MIPC_CALL_DC_MODIFY_SIP_REQ_T_REMOTE_FP_STR, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 192},
    {.tlv_type = MIPC_CALL_DC_MODIFY_SIP_REQ_T_LOCAL_FP_STR, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 192},
    {.tlv_type = MIPC_CALL_DC_MODIFY_SIP_REQ_T_QOS_HINT, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 64},
    {.tlv_type = MIPC_CALL_DC_MODIFY_SIP_REQ_T_DCMAP_LABEL, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 80},
    {.tlv_type = MIPC_CALL_DC_MODIFY_SIP_REQ_T_DCMAP_SUBPROTOCOL, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 10},
    {.tlv_type = MIPC_CALL_DC_MODIFY_SIP_REQ_T_REMOTE_ADDR, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_ims_set_config_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_IMS_SET_CONFIG_REQ_T_CLASS, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
    {.tlv_type = MIPC_IMS_SET_CONFIG_REQ_T_TYPE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 32},
    {.tlv_type = MIPC_IMS_SET_CONFIG_REQ_T_DATA, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 60000},
};

mipc_tlv_checker_info_t mipc_ims_get_config_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_IMS_GET_CONFIG_REQ_T_CLASS, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
    {.tlv_type = MIPC_IMS_GET_CONFIG_REQ_T_TYPE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 32},
};

mipc_tlv_checker_info_t mipc_ims_get_state_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_IMS_GET_STATE_REQ_T_EVENT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
};

mipc_tlv_checker_info_t mipc_ims_set_pdis_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_IMS_SET_PDIS_REQ_T_METHOD, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
};

mipc_tlv_checker_info_t mipc_ims_set_naptr_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_IMS_SET_NAPTR_REQ_T_MOD_ID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 32},
    {.tlv_type = MIPC_IMS_SET_NAPTR_REQ_T_FLAGS, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 100},
    {.tlv_type = MIPC_IMS_SET_NAPTR_REQ_T_SERVICE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 100},
    {.tlv_type = MIPC_IMS_SET_NAPTR_REQ_T_REGEXP, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 100},
    {.tlv_type = MIPC_IMS_SET_NAPTR_REQ_T_FQDN, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 256},
};

mipc_tlv_checker_info_t mipc_ims_set_eireg_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_IMS_SET_EIREG_REQ_T_REG_STATE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_IMS_SET_EIREG_REQ_T_REG_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_IMS_SET_EIREG_REQ_T_DEREG_CAUSE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_IMS_SET_EIREG_REQ_T_IMS_RAT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 4},
    {.tlv_type = MIPC_IMS_SET_EIREG_REQ_T_SIP_URI_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_IMS_SET_EIREG_REQ_T_IMS_RETRY, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_ims_set_scm_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_IMS_SET_SCM_REQ_T_APPLICATION, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 127},
    {.tlv_type = MIPC_IMS_SET_SCM_REQ_T_INDICATION, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 127},
};

mipc_tlv_checker_info_t mipc_ims_set_service_session_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_IMS_SET_SERVICE_SESSION_REQ_T_SERVICE_TYPE, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 127},
    {.tlv_type = MIPC_IMS_SET_SERVICE_SESSION_REQ_T_SERVICE_STATUS, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 127},
};

mipc_tlv_checker_info_t mipc_ims_set_uac_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_IMS_SET_UAC_REQ_T_SERVICE_TYPE, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 127},
    {.tlv_type = MIPC_IMS_SET_UAC_REQ_T_SERVICE_STATUS, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 127},
};

mipc_tlv_checker_info_t mipc_ims_set_evodata_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_IMS_SET_EVODATA_REQ_T_EN, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_IMS_SET_EVODATA_REQ_T_DATA_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_IMS_SET_EVODATA_REQ_T_ALLOW_RAT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_ims_enable_data_channel_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_IMS_ENABLE_DATA_CHANNEL_REQ_T_ENABLE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_ims_cfg_addition_service_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_IMS_CFG_ADDITION_SERVICE_REQ_T_SERVICE_CAP, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 64},
};

mipc_tlv_checker_info_t mipc_wfc_register_cell_signal_ind_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_WFC_REGISTER_CELL_SIGNAL_IND_REQ_T_SIGNAL_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_WFC_REGISTER_CELL_SIGNAL_IND_REQ_T_ENABLE_RPT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_wfc_set_emc_aid_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_WFC_SET_EMC_AID_REQ_T_AID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 20},
};

mipc_tlv_checker_info_t mipc_wfc_cfg_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_WFC_CFG_REQ_T_LOCATION_ENABLE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_WFC_CFG_REQ_T_WFC_PREFER, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
};

mipc_tlv_checker_info_t mipc_phb_set_upb_entry_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_PHB_SET_UPB_ENTRY_REQ_T_TYPE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 5},
    {.tlv_type = MIPC_PHB_SET_UPB_ENTRY_REQ_T_LINE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 248},
    {.tlv_type = MIPC_PHB_SET_UPB_ENTRY_REQ_T_ENCODE_METHOD, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 5},
};

mipc_tlv_checker_info_t mipc_phb_get_upb_anr_email_sne_entry_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_PHB_GET_UPB_ANR_EMAIL_SNE_ENTRY_REQ_T_TYPE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 5},
};

mipc_tlv_checker_info_t mipc_phb_get_upb_aas_gas_grp_list_entry_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_PHB_GET_UPB_AAS_GAS_GRP_LIST_ENTRY_REQ_T_TYPE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 5},
};

mipc_tlv_checker_info_t mipc_phb_get_phb_storage_info_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_PHB_GET_PHB_STORAGE_INFO_REQ_T_STORAGE_TYPE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 7},
};

mipc_tlv_checker_info_t mipc_phb_set_phb_mem_storage_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_PHB_SET_PHB_MEM_STORAGE_REQ_T_STORAGE_TYPE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 7},
    {.tlv_type = MIPC_PHB_SET_PHB_MEM_STORAGE_REQ_T_PASSWORD, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 50},
};

mipc_tlv_checker_info_t mipc_phb_get_phb_entry_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_PHB_GET_PHB_ENTRY_REQ_T_STORAGE_TYPE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 7},
};

mipc_tlv_checker_info_t mipc_phb_set_phb_entry_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_PHB_SET_PHB_ENTRY_REQ_T_TYPE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 7},
    {.tlv_type = MIPC_PHB_SET_PHB_ENTRY_REQ_T_ENTRY, .option = 0x400, .check_item.fixed.fixed_value = 512},
};

mipc_tlv_checker_info_t mipc_phb_get_phb_available_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_PHB_GET_PHB_AVAILABLE_REQ_T_TYPE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 5},
};

mipc_tlv_checker_info_t mipc_ecall_ivs_update_msd_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_ECALL_IVS_UPDATE_MSD_REQ_T_MSD_FORMAT, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_ECALL_IVS_UPDATE_MSD_REQ_T_MSD_DATA, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 140},
};

mipc_tlv_checker_info_t mipc_ecall_ivs_set_test_addr_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_ECALL_IVS_SET_TEST_ADDR_REQ_T_ADDR_TYPE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_ECALL_IVS_SET_TEST_ADDR_REQ_T_ADDRESS, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 180},
};

mipc_tlv_checker_info_t mipc_ecall_ivs_set_reconf_addr_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_ECALL_IVS_SET_RECONF_ADDR_REQ_T_ADDR_TYPE, .option = 0x8304, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_ECALL_IVS_SET_RECONF_ADDR_REQ_T_ADDRESS, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 180},
};

mipc_tlv_checker_info_t mipc_ecall_ivs_set_addr_pri_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_ECALL_IVS_SET_ADDR_PRI_REQ_T_FIRST_PRI, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 4},
    {.tlv_type = MIPC_ECALL_IVS_SET_ADDR_PRI_REQ_T_SECOND_PRI, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 4},
    {.tlv_type = MIPC_ECALL_IVS_SET_ADDR_PRI_REQ_T_THIRD_PRI, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 4},
    {.tlv_type = MIPC_ECALL_IVS_SET_ADDR_PRI_REQ_T_FOURTH_PRI, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 4},
};

mipc_tlv_checker_info_t mipc_ecall_ivs_set_ecall_mode_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_ECALL_IVS_SET_ECALL_MODE_REQ_T_ECALL_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
};

mipc_tlv_checker_info_t mipc_ecall_ivs_set_ecall_category_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_ECALL_IVS_SET_ECALL_CATEGORY_REQ_T_ECALL_CATEGORY, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
};

mipc_tlv_checker_info_t mipc_nw_congestion_cfg_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_CONGESTION_CFG_REQ_T_NW_CONGESTION_CFG, .option = 0x400, .check_item.fixed.fixed_value = 52},
    {.tlv_type = MIPC_NW_CONGESTION_CFG_REQ_T_NW_CONGESTION_CFG_V1, .option = 0x400, .check_item.fixed.fixed_value = 88},
};

mipc_tlv_checker_info_t mipc_nw_set_rx_mimo_cap_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_RX_MIMO_CAP_REQ_T_CTRL_OPERATION, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 3},
    {.tlv_type = MIPC_NW_SET_RX_MIMO_CAP_REQ_T_RELEASE_LEVEL, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
};

mipc_tlv_checker_info_t mipc_nw_set_mr_thresh_bound_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_MR_THRESH_BOUND_REQ_T_CONF_INFO, .option = 0x400, .check_item.fixed.fixed_value = 24},
};

mipc_tlv_checker_info_t mipc_nw_delete_ftai_entry_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_DELETE_FTAI_ENTRY_REQ_T_FTAI_DEL_ATTR, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
    {.tlv_type = MIPC_NW_DELETE_FTAI_ENTRY_REQ_T_TAI_LIST, .option = 0x400, .check_item.fixed.fixed_value = 10},
};

mipc_tlv_checker_info_t mipc_nw_serving_measuremnt_control_config_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SERVING_MEASUREMNT_CONTROL_CONFIG_REQ_T_RAT, .option = 0x8301, .check_item.min_max.min_value = 4, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_nw_clear_n1_s1_disabled_status_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_CLEAR_N1_S1_DISABLED_STATUS_REQ_T_PLMN_ID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 7},
    {.tlv_type = MIPC_NW_CLEAR_N1_S1_DISABLED_STATUS_REQ_T_RAT, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_NW_CLEAR_N1_S1_DISABLED_STATUS_REQ_T_ACTION, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 2},
};

mipc_tlv_checker_info_t mipc_nw_set_absolute_max_tx_power_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_ABSOLUTE_MAX_TX_POWER_REQ_T_BAND_POWER_TLV_ARRAY, .option = 0x400, .check_item.fixed.fixed_value = 8},
};

mipc_tlv_checker_info_t mipc_nw_set_cpe_mode_update_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_CPE_MODE_UPDATE_REQ_T_CPE_MODE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 127},
};

mipc_tlv_checker_info_t mipc_nw_set_max_eirp_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_MAX_EIRP_REQ_T_BAND_POWER_TLV_ARRAY, .option = 0x400, .check_item.fixed.fixed_value = 8},
};

mipc_tlv_checker_info_t mipc_nw_clear_nssai_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_CLEAR_NSSAI_REQ_T_CLEAR_NSSAI_OPTION, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 127},
};

mipc_tlv_checker_info_t mipc_nw_set_time_sync_pps_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_TIME_SYNC_PPS_REQ_T_START, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_nw_set_vehicle_status_req_tlv_checker_info[] = {
    {.tlv_type = MIPC_NW_SET_VEHICLE_STATUS_REQ_T_STATUS, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_sys_set_config_dipc_rsp_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_SET_CONFIG_DIPC_RSP_T_DATA, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 60000},
};

mipc_tlv_checker_info_t mipc_sys_get_config_dipc_rsp_tlv_checker_info[] = {
    {.tlv_type = MIPC_SYS_GET_CONFIG_DIPC_RSP_T_DATA, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 60000},
};

mipc_tlv_checker_info_t mipc_data_ifup_packet_route_rsp_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_IFUP_PACKET_ROUTE_RSP_T_APP_ID, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_internal_eipport_rsp_tlv_checker_info[] = {
    {.tlv_type = MIPC_INTERNAL_EIPPORT_RSP_T_ACTION, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_INTERNAL_EIPPORT_RSP_T_RESULT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_internal_multi_eipspi_free_rsp_tlv_checker_info[] = {
    {.tlv_type = MIPC_INTERNAL_MULTI_EIPSPI_FREE_RSP_T_SPI_LIST, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 50},
    {.tlv_type = MIPC_INTERNAL_MULTI_EIPSPI_FREE_RSP_T_STATUS_LIST, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 50},
};

mipc_tlv_checker_info_t mipc_sms_new_sms_rsp_tlv_checker_info[] = {
    {.tlv_type = MIPC_SMS_NEW_SMS_RSP_T_NEW_SMS_ACK, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_NEW_SMS_RSP_T_FORMAT, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_NEW_SMS_RSP_T_ERR_CLASS_C2K, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_NEW_SMS_RSP_T_ERR_CODE_C2K, .option = 0x8302, .check_item.min_max.min_value = 14, .check_item.min_max.max_value = 107},
    {.tlv_type = MIPC_SMS_NEW_SMS_RSP_T_ACK_PDU, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 256},
};

mipc_tlv_checker_info_t mipc_sms_new_status_report_rsp_tlv_checker_info[] = {
    {.tlv_type = MIPC_SMS_NEW_STATUS_REPORT_RSP_T_ACK_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_SMS_NEW_STATUS_REPORT_RSP_T_ACK_PDU, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 256},
};

mipc_tlv_checker_info_t mipc_call_approve_incoming_rsp_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_APPROVE_INCOMING_RSP_T_IS_APPROVE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_call_dc_bootstrap_cfg_qry_rsp_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_DC_BOOTSTRAP_CFG_QRY_RSP_T_QOS_HINT, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 64},
    {.tlv_type = MIPC_CALL_DC_BOOTSTRAP_CFG_QRY_RSP_T_DCMAP_LABEL, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 80},
    {.tlv_type = MIPC_CALL_DC_BOOTSTRAP_CFG_QRY_RSP_T_DCMAP_SUBPROTOCOL, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 10},
    {.tlv_type = MIPC_CALL_DC_BOOTSTRAP_CFG_QRY_RSP_T_LOCAL_TLS_ID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 256},
    {.tlv_type = MIPC_CALL_DC_BOOTSTRAP_CFG_QRY_RSP_T_LOCAL_FP_STR, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 192},
};

mipc_tlv_checker_info_t mipc_call_dc_create_rsp_tlv_checker_info[] = {
    {.tlv_type = MIPC_CALL_DC_CREATE_RSP_T_REMOTE_ADDR, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_CALL_DC_CREATE_RSP_T_LOCAL_ADDR, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_CALL_DC_CREATE_RSP_T_REMOTE_TLS_ID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 256},
    {.tlv_type = MIPC_CALL_DC_CREATE_RSP_T_LOCAL_TLS_ID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 256},
    {.tlv_type = MIPC_CALL_DC_CREATE_RSP_T_REMOTE_FP_STR, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 192},
    {.tlv_type = MIPC_CALL_DC_CREATE_RSP_T_LOCAL_FP_STR, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 192},
    {.tlv_type = MIPC_CALL_DC_CREATE_RSP_T_QOS_HINT, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 64},
    {.tlv_type = MIPC_CALL_DC_CREATE_RSP_T_DCMAP_LABEL, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 80},
    {.tlv_type = MIPC_CALL_DC_CREATE_RSP_T_DCMAP_SUBPROTOCOL, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 10},
};

mipc_tlv_checker_info_t mipc_ims_pdis_rsp_tlv_checker_info[] = {
    {.tlv_type = MIPC_IMS_PDIS_RSP_T_METHOD, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
};

mipc_tlv_checker_info_t mipc_wfc_ping_rsp_tlv_checker_info[] = {
    {.tlv_type = MIPC_WFC_PING_RSP_T_RAT, .option = 0x8301, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 255},
};

mipc_tlv_checker_info_t mipc_wfc_get_mac_rsp_tlv_checker_info[] = {
    {.tlv_type = MIPC_WFC_GET_MAC_RSP_T_IFNAME, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_WFC_GET_MAC_RSP_T_IP, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_WFC_GET_MAC_RSP_T_MAC, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
};

mipc_tlv_checker_info_t mipc_wfc_natt_keep_alive_rsp_tlv_checker_info[] = {
    {.tlv_type = MIPC_WFC_NATT_KEEP_ALIVE_RSP_T_IFNAME, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_WFC_NATT_KEEP_ALIVE_RSP_T_ENABLE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_WFC_NATT_KEEP_ALIVE_RSP_T_SRC_IP, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_WFC_NATT_KEEP_ALIVE_RSP_T_DST_IP, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
};

mipc_tlv_checker_info_t mipc_data_act_call_ntf_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_ACT_CALL_NTF_T_RESPONSE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 127},
};

mipc_tlv_checker_info_t mipc_data_act_packet_route_ntf_tlv_checker_info[] = {
    {.tlv_type = MIPC_DATA_ACT_PACKET_ROUTE_NTF_T_APP_ID, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
};

mipc_tlv_checker_info_t mipc_internal_inject_tst_ntf_tlv_checker_info[] = {
    {.tlv_type = MIPC_INTERNAL_INJECT_TST_NTF_T_MODULE, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 32},
    {.tlv_type = MIPC_INTERNAL_INJECT_TST_NTF_T_INJECT_STRING, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_wfc_wifi_signal_ntf_tlv_checker_info[] = {
    {.tlv_type = MIPC_WFC_WIFI_SIGNAL_NTF_T_IFNAME, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
};

mipc_tlv_checker_info_t mipc_wfc_epdg_screen_state_ntf_tlv_checker_info[] = {
    {.tlv_type = MIPC_WFC_EPDG_SCREEN_STATE_NTF_T_STATE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
};

mipc_tlv_checker_info_t mipc_wfc_wifi_info_ntf_tlv_checker_info[] = {
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_IFNAME, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 128},
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_ENABLE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_IPV4_GATEWAY, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 4},
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_IPV6, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_IPV6_GATEWAY, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_DNS, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 16},
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_IP_UPDATE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_WIFI_TYPE_STR, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 32},
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_WIFI_EXTEND_STATE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_CONN_STATE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_CONN_READY, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_APM_STATE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_ASSOCIATED, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 1},
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_SSID, .option = 0x300, .check_item.min_max.min_value = 1, .check_item.min_max.max_value = 33},
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_AP_MAC, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_WIFI_TYPE, .option = 0x8301, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 255},
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_UE_MAC, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 6},
    {.tlv_type = MIPC_WFC_WIFI_INFO_NTF_T_IPV4, .option = 0x200, .check_item.min_max.min_value = 0, .check_item.min_max.max_value = 4},
};

mipc_checker_info_t mipc_checker_info[] = {
    {MIPC_SYS_GET_INFO_REQ, "GET_INFO_REQ", 0, NULL},
    {MIPC_SYS_GET_INFO_CNF, "GET_INFO_CNF", 0, NULL},
    {MIPC_SYS_AT_REQ, "AT_REQ", 1, mipc_sys_at_req_tlv_checker_info},
    {MIPC_SYS_AT_CNF, "AT_CNF", 0, NULL},
    {MIPC_SYS_REBOOT_REQ, "REBOOT_REQ", 1, mipc_sys_reboot_req_tlv_checker_info},
    {MIPC_SYS_REBOOT_CNF, "REBOOT_CNF", 0, NULL},
    {MIPC_SYS_GET_MAPPING_REQ, "GET_MAPPING_REQ", 1, mipc_sys_get_mapping_req_tlv_checker_info},
    {MIPC_SYS_GET_MAPPING_CNF, "GET_MAPPING_CNF", 0, NULL},
    {MIPC_SYS_SET_MAPPING_REQ, "SET_MAPPING_REQ", 1, mipc_sys_set_mapping_req_tlv_checker_info},
    {MIPC_SYS_SET_MAPPING_CNF, "SET_MAPPING_CNF", 0, NULL},
    {MIPC_SYS_GET_THERMAL_SENSOR_NUM_REQ, "GET_THERMAL_SENSOR_NUM_REQ", 0, NULL},
    {MIPC_SYS_GET_THERMAL_SENSOR_NUM_CNF, "GET_THERMAL_SENSOR_NUM_CNF", 0, NULL},
    {MIPC_SYS_GET_THERMAL_SENSOR_INFO_REQ, "GET_THERMAL_SENSOR_INFO_REQ", 0, NULL},
    {MIPC_SYS_GET_THERMAL_SENSOR_INFO_CNF, "GET_THERMAL_SENSOR_INFO_CNF", 0, NULL},
    {MIPC_SYS_SET_THERMAL_SENSOR_REQ, "SET_THERMAL_SENSOR_REQ", 2, mipc_sys_set_thermal_sensor_req_tlv_checker_info},
    {MIPC_SYS_SET_THERMAL_SENSOR_CNF, "SET_THERMAL_SENSOR_CNF", 0, NULL},
    {MIPC_SYS_GET_THERMAL_SENSOR_REQ, "GET_THERMAL_SENSOR_REQ", 0, NULL},
    {MIPC_SYS_GET_THERMAL_SENSOR_CNF, "GET_THERMAL_SENSOR_CNF", 0, NULL},
    {MIPC_SYS_GET_THERMAL_ACTUATOR_NUM_REQ, "GET_THERMAL_ACTUATOR_NUM_REQ", 0, NULL},
    {MIPC_SYS_GET_THERMAL_ACTUATOR_NUM_CNF, "GET_THERMAL_ACTUATOR_NUM_CNF", 0, NULL},
    {MIPC_SYS_SET_THERMAL_ACTUATOR_REQ, "SET_THERMAL_ACTUATOR_REQ", 0, NULL},
    {MIPC_SYS_SET_THERMAL_ACTUATOR_CNF, "SET_THERMAL_ACTUATOR_CNF", 0, NULL},
    {MIPC_SYS_GET_THERMAL_ACTUATOR_INFO_REQ, "GET_THERMAL_ACTUATOR_INFO_REQ", 0, NULL},
    {MIPC_SYS_GET_THERMAL_ACTUATOR_INFO_CNF, "GET_THERMAL_ACTUATOR_INFO_CNF", 0, NULL},
    {MIPC_SYS_SET_CONFIG_REQ, "SET_CONFIG_REQ", 3, mipc_sys_set_config_req_tlv_checker_info},
    {MIPC_SYS_SET_CONFIG_CNF, "SET_CONFIG_CNF", 0, NULL},
    {MIPC_SYS_GET_CONFIG_REQ, "GET_CONFIG_REQ", 2, mipc_sys_get_config_req_tlv_checker_info},
    {MIPC_SYS_GET_CONFIG_CNF, "GET_CONFIG_CNF", 0, NULL},
    {MIPC_SYS_REG_CONFIG_REQ, "REG_CONFIG_REQ", 2, mipc_sys_reg_config_req_tlv_checker_info},
    {MIPC_SYS_REG_CONFIG_CNF, "REG_CONFIG_CNF", 0, NULL},
    {MIPC_SYS_SET_ADPCLK_REQ, "SET_ADPCLK_REQ", 1, mipc_sys_set_adpclk_req_tlv_checker_info},
    {MIPC_SYS_SET_ADPCLK_CNF, "SET_ADPCLK_CNF", 0, NULL},
    {MIPC_SYS_GET_ADPCLK_REQ, "GET_ADPCLK_REQ", 0, NULL},
    {MIPC_SYS_GET_ADPCLK_CNF, "GET_ADPCLK_CNF", 0, NULL},
    {MIPC_SYS_SET_MD_LOG_MODE_REQ, "SET_MD_LOG_MODE_REQ", 0, NULL},
    {MIPC_SYS_SET_MD_LOG_MODE_CNF, "SET_MD_LOG_MODE_CNF", 0, NULL},
    {MIPC_SYS_GET_MD_LOG_MODE_REQ, "GET_MD_LOG_MODE_REQ", 0, NULL},
    {MIPC_SYS_GET_MD_LOG_MODE_CNF, "GET_MD_LOG_MODE_CNF", 0, NULL},
    {MIPC_SYS_SET_MD_LOG_LEVEL_REQ, "SET_MD_LOG_LEVEL_REQ", 0, NULL},
    {MIPC_SYS_SET_MD_LOG_LEVEL_CNF, "SET_MD_LOG_LEVEL_CNF", 0, NULL},
    {MIPC_SYS_GET_MD_LOG_LEVEL_REQ, "GET_MD_LOG_LEVEL_REQ", 0, NULL},
    {MIPC_SYS_GET_MD_LOG_LEVEL_CNF, "GET_MD_LOG_LEVEL_CNF", 0, NULL},
    {MIPC_SYS_SET_MD_LOG_LOCATION_REQ, "SET_MD_LOG_LOCATION_REQ", 0, NULL},
    {MIPC_SYS_SET_MD_LOG_LOCATION_CNF, "SET_MD_LOG_LOCATION_CNF", 0, NULL},
    {MIPC_SYS_GET_MD_LOG_LOCATION_REQ, "GET_MD_LOG_LOCATION_REQ", 0, NULL},
    {MIPC_SYS_GET_MD_LOG_LOCATION_CNF, "GET_MD_LOG_LOCATION_CNF", 0, NULL},
    {MIPC_SYS_WRITE_NVRAM_REQ, "WRITE_NVRAM_REQ", 1, mipc_sys_write_nvram_req_tlv_checker_info},
    {MIPC_SYS_WRITE_NVRAM_CNF, "WRITE_NVRAM_CNF", 0, NULL},
    {MIPC_SYS_READ_NVRAM_REQ, "READ_NVRAM_REQ", 0, NULL},
    {MIPC_SYS_READ_NVRAM_CNF, "READ_NVRAM_CNF", 0, NULL},
    {MIPC_SYS_AUTH_REQ, "AUTH_REQ", 1, mipc_sys_auth_req_tlv_checker_info},
    {MIPC_SYS_AUTH_CNF, "AUTH_CNF", 0, NULL},
    {MIPC_SYS_SET_DAT_REQ, "SET_DAT_REQ", 0, NULL},
    {MIPC_SYS_SET_DAT_CNF, "SET_DAT_CNF", 0, NULL},
    {MIPC_SYS_GET_DAT_REQ, "GET_DAT_REQ", 0, NULL},
    {MIPC_SYS_GET_DAT_CNF, "GET_DAT_CNF", 0, NULL},
    {MIPC_SYS_MCF_REQ, "MCF_REQ", 5, mipc_sys_mcf_req_tlv_checker_info},
    {MIPC_SYS_MCF_CNF, "MCF_CNF", 0, NULL},
    {MIPC_SYS_SET_FCC_LOCK_REQ, "SET_FCC_LOCK_REQ", 0, NULL},
    {MIPC_SYS_SET_FCC_LOCK_CNF, "SET_FCC_LOCK_CNF", 0, NULL},
    {MIPC_SYS_SET_TIME_REQ, "SET_TIME_REQ", 0, NULL},
    {MIPC_SYS_SET_TIME_CNF, "SET_TIME_CNF", 0, NULL},
    {MIPC_SYS_GET_TIME_REQ, "GET_TIME_REQ", 0, NULL},
    {MIPC_SYS_GET_TIME_CNF, "GET_TIME_CNF", 0, NULL},
    {MIPC_SYS_SET_SAR_REQ, "SET_SAR_REQ", 1, mipc_sys_set_sar_req_tlv_checker_info},
    {MIPC_SYS_SET_SAR_CNF, "SET_SAR_CNF", 0, NULL},
    {MIPC_SYS_GET_SAR_REQ, "GET_SAR_REQ", 0, NULL},
    {MIPC_SYS_GET_SAR_CNF, "GET_SAR_CNF", 0, NULL},
    {MIPC_SYS_SET_POWER_SAVING_REQ, "SET_POWER_SAVING_REQ", 0, NULL},
    {MIPC_SYS_SET_POWER_SAVING_CNF, "SET_POWER_SAVING_CNF", 0, NULL},
    {MIPC_SYS_CONNECTIVITY_STATISTICS_REQ, "CONNECTIVITY_STATISTICS_REQ", 0, NULL},
    {MIPC_SYS_CONNECTIVITY_STATISTICS_CNF, "CONNECTIVITY_STATISTICS_CNF", 0, NULL},
    {MIPC_SYS_QUERY_SBP_REQ, "QUERY_SBP_REQ", 0, NULL},
    {MIPC_SYS_QUERY_SBP_CNF, "QUERY_SBP_CNF", 0, NULL},
    {MIPC_SYS_SET_TX_IND_INTERVAL_REQ, "SET_TX_IND_INTERVAL_REQ", 0, NULL},
    {MIPC_SYS_SET_TX_IND_INTERVAL_CNF, "SET_TX_IND_INTERVAL_CNF", 0, NULL},
    {MIPC_SYS_SET_GEO_LOCATION_REQ, "SET_GEO_LOCATION_REQ", 15, mipc_sys_set_geo_location_req_tlv_checker_info},
    {MIPC_SYS_SET_GEO_LOCATION_CNF, "SET_GEO_LOCATION_CNF", 0, NULL},
    {MIPC_SYS_SET_DSBP_REQ, "SET_DSBP_REQ", 1, mipc_sys_set_dsbp_req_tlv_checker_info},
    {MIPC_SYS_SET_DSBP_CNF, "SET_DSBP_CNF", 0, NULL},
    {MIPC_SYS_SEND_SAR_IND_REQ, "SEND_SAR_IND_REQ", 1, mipc_sys_send_sar_ind_req_tlv_checker_info},
    {MIPC_SYS_SEND_SAR_IND_CNF, "SEND_SAR_IND_CNF", 0, NULL},
    {MIPC_SYS_SILENT_REBOOT_REQ, "SILENT_REBOOT_REQ", 0, NULL},
    {MIPC_SYS_SILENT_REBOOT_CNF, "SILENT_REBOOT_CNF", 0, NULL},
    {MIPC_SYS_MULTI_SIM_CONFIG_REQ, "MULTI_SIM_CONFIG_REQ", 0, NULL},
    {MIPC_SYS_MULTI_SIM_CONFIG_CNF, "MULTI_SIM_CONFIG_CNF", 0, NULL},
    {MIPC_SYS_REBOOT_SET_REQ, "REBOOT_SET_REQ", 0, NULL},
    {MIPC_SYS_REBOOT_SET_CNF, "REBOOT_SET_CNF", 0, NULL},
    {MIPC_SYS_GET_THERMAL_ACTUATOR_REQ, "GET_THERMAL_ACTUATOR_REQ", 1, mipc_sys_get_thermal_actuator_req_tlv_checker_info},
    {MIPC_SYS_GET_THERMAL_ACTUATOR_CNF, "GET_THERMAL_ACTUATOR_CNF", 0, NULL},
    {MIPC_SYS_GET_THERMAL_SENSOR_RUNTIME_REQ, "GET_THERMAL_SENSOR_RUNTIME_REQ", 1, mipc_sys_get_thermal_sensor_runtime_req_tlv_checker_info},
    {MIPC_SYS_GET_THERMAL_SENSOR_RUNTIME_CNF, "GET_THERMAL_SENSOR_RUNTIME_CNF", 0, NULL},
    {MIPC_SYS_SET_THERMAL_RUNTIME_REQ, "SET_THERMAL_RUNTIME_REQ", 4, mipc_sys_set_thermal_runtime_req_tlv_checker_info},
    {MIPC_SYS_SET_THERMAL_RUNTIME_CNF, "SET_THERMAL_RUNTIME_CNF", 0, NULL},
    {MIPC_SYS_SET_OR_GET_SBP_INFO_REQ, "SET_OR_GET_SBP_INFO_REQ", 2, mipc_sys_set_or_get_sbp_info_req_tlv_checker_info},
    {MIPC_SYS_SET_OR_GET_SBP_INFO_CNF, "SET_OR_GET_SBP_INFO_CNF", 0, NULL},
    {MIPC_SYS_GET_ALL_THERMAL_INFO_REQ, "GET_ALL_THERMAL_INFO_REQ", 0, NULL},
    {MIPC_SYS_GET_ALL_THERMAL_INFO_CNF, "GET_ALL_THERMAL_INFO_CNF", 0, NULL},
    {MIPC_SYS_ECHO_REQ, "ECHO_REQ", 0, NULL},
    {MIPC_SYS_ECHO_CNF, "ECHO_CNF", 0, NULL},
    {MIPC_SYS_SET_MD_LOG_FLUSH_INTERVAL_REQ, "SET_MD_LOG_FLUSH_INTERVAL_REQ", 0, NULL},
    {MIPC_SYS_SET_MD_LOG_FLUSH_INTERVAL_CNF, "SET_MD_LOG_FLUSH_INTERVAL_CNF", 0, NULL},
    {MIPC_SYS_GET_MD_LOG_FLUSH_INTERVAL_REQ, "GET_MD_LOG_FLUSH_INTERVAL_REQ", 0, NULL},
    {MIPC_SYS_GET_MD_LOG_FLUSH_INTERVAL_CNF, "GET_MD_LOG_FLUSH_INTERVAL_CNF", 0, NULL},
    {MIPC_SYS_HBA_ESTABLISH_REQ, "HBA_ESTABLISH_REQ", 9, mipc_sys_hba_establish_req_tlv_checker_info},
    {MIPC_SYS_HBA_ESTABLISH_CNF, "HBA_ESTABLISH_CNF", 0, NULL},
    {MIPC_SYS_HBA_CTRL_REQ, "HBA_CTRL_REQ", 2, mipc_sys_hba_ctrl_req_tlv_checker_info},
    {MIPC_SYS_HBA_CTRL_CNF, "HBA_CTRL_CNF", 0, NULL},
    {MIPC_SYS_HBA_SEND_NOW_REQ, "HBA_SEND_NOW_REQ", 1, mipc_sys_hba_send_now_req_tlv_checker_info},
    {MIPC_SYS_HBA_SEND_NOW_CNF, "HBA_SEND_NOW_CNF", 0, NULL},
    {MIPC_SYS_HBA_RESUME_REQ, "HBA_RESUME_REQ", 1, mipc_sys_hba_resume_req_tlv_checker_info},
    {MIPC_SYS_HBA_RESUME_CNF, "HBA_RESUME_CNF", 0, NULL},
    {MIPC_SYS_MIA_START_SCENARIO_REQ, "MIA_START_SCENARIO_REQ", 1, mipc_sys_mia_start_scenario_req_tlv_checker_info},
    {MIPC_SYS_MIA_START_SCENARIO_CNF, "MIA_START_SCENARIO_CNF", 0, NULL},
    {MIPC_SYS_MIA_STOP_SCENARIO_REQ, "MIA_STOP_SCENARIO_REQ", 1, mipc_sys_mia_stop_scenario_req_tlv_checker_info},
    {MIPC_SYS_MIA_STOP_SCENARIO_CNF, "MIA_STOP_SCENARIO_CNF", 0, NULL},
    {MIPC_SYS_MIA_UPDATE_METRICS_REQ, "MIA_UPDATE_METRICS_REQ", 1, mipc_sys_mia_update_metrics_req_tlv_checker_info},
    {MIPC_SYS_MIA_UPDATE_METRICS_CNF, "MIA_UPDATE_METRICS_CNF", 0, NULL},
    {MIPC_SYS_TRIGGER_MD_LOG_FLUSH_REQ, "TRIGGER_MD_LOG_FLUSH_REQ", 0, NULL},
    {MIPC_SYS_TRIGGER_MD_LOG_FLUSH_CNF, "TRIGGER_MD_LOG_FLUSH_CNF", 0, NULL},
    {MIPC_SYS_GET_MD_LOG_FLUSH_STATUS_REQ, "GET_MD_LOG_FLUSH_STATUS_REQ", 0, NULL},
    {MIPC_SYS_GET_MD_LOG_FLUSH_STATUS_CNF, "GET_MD_LOG_FLUSH_STATUS_CNF", 0, NULL},
    {MIPC_SYS_GET_GNSS_COCLOCK_NVDATA_REQ, "GET_GNSS_COCLOCK_NVDATA_REQ", 0, NULL},
    {MIPC_SYS_GET_GNSS_COCLOCK_NVDATA_CNF, "GET_GNSS_COCLOCK_NVDATA_CNF", 0, NULL},
    {MIPC_SYS_GET_WAKEUP_REASON_REQ, "GET_WAKEUP_REASON_REQ", 0, NULL},
    {MIPC_SYS_GET_WAKEUP_REASON_CNF, "GET_WAKEUP_REASON_CNF", 0, NULL},
    {MIPC_SYS_SET_SLEEP_TIMER_REQ, "SET_SLEEP_TIMER_REQ", 0, NULL},
    {MIPC_SYS_SET_SLEEP_TIMER_CNF, "SET_SLEEP_TIMER_CNF", 0, NULL},
    {MIPC_SYS_GET_SLEEP_TIMER_REQ, "GET_SLEEP_TIMER_REQ", 0, NULL},
    {MIPC_SYS_GET_SLEEP_TIMER_CNF, "GET_SLEEP_TIMER_CNF", 0, NULL},
    {MIPC_SYS_SET_SLEEP_MODE_REQ, "SET_SLEEP_MODE_REQ", 1, mipc_sys_set_sleep_mode_req_tlv_checker_info},
    {MIPC_SYS_SET_SLEEP_MODE_CNF, "SET_SLEEP_MODE_CNF", 0, NULL},
    {MIPC_SYS_SEND_CONSYS_WIFI_REQ, "SEND_CONSYS_WIFI_REQ", 1, mipc_sys_send_consys_wifi_req_tlv_checker_info},
    {MIPC_SYS_SEND_CONSYS_WIFI_CNF, "SEND_CONSYS_WIFI_CNF", 0, NULL},
    {MIPC_SYS_SEND_CONSYS_BT_REQ, "SEND_CONSYS_BT_REQ", 1, mipc_sys_send_consys_bt_req_tlv_checker_info},
    {MIPC_SYS_SEND_CONSYS_BT_CNF, "SEND_CONSYS_BT_CNF", 0, NULL},
    {MIPC_APN_SET_IA_REQ, "SET_IA_REQ", 7, mipc_apn_set_ia_req_tlv_checker_info},
    {MIPC_APN_SET_IA_CNF, "SET_IA_CNF", 0, NULL},
    {MIPC_APN_GET_IA_REQ, "GET_IA_REQ", 0, NULL},
    {MIPC_APN_GET_IA_CNF, "GET_IA_CNF", 0, NULL},
    {MIPC_APN_ADD_PROFILE_REQ, "ADD_PROFILE_REQ", 10, mipc_apn_add_profile_req_tlv_checker_info},
    {MIPC_APN_ADD_PROFILE_CNF, "ADD_PROFILE_CNF", 0, NULL},
    {MIPC_APN_LIST_PROFILE_REQ, "LIST_PROFILE_REQ", 0, NULL},
    {MIPC_APN_LIST_PROFILE_CNF, "LIST_PROFILE_CNF", 0, NULL},
    {MIPC_APN_DEL_PROFILE_REQ, "DEL_PROFILE_REQ", 0, NULL},
    {MIPC_APN_DEL_PROFILE_CNF, "DEL_PROFILE_CNF", 0, NULL},
    {MIPC_APN_SET_PROFILE_STATUS_REQ, "SET_PROFILE_STATUS_REQ", 1, mipc_apn_set_profile_status_req_tlv_checker_info},
    {MIPC_APN_SET_PROFILE_STATUS_CNF, "SET_PROFILE_STATUS_CNF", 0, NULL},
    {MIPC_APN_LIST_MD_PROFILE_REQ, "LIST_MD_PROFILE_REQ", 0, NULL},
    {MIPC_APN_LIST_MD_PROFILE_CNF, "LIST_MD_PROFILE_CNF", 0, NULL},
    {MIPC_APN_SET_VZW_APN_REQ, "SET_VZW_APN_REQ", 4, mipc_apn_set_vzw_apn_req_tlv_checker_info},
    {MIPC_APN_SET_VZW_APN_CNF, "SET_VZW_APN_CNF", 0, NULL},
    {MIPC_APN_SET_VZW_APN_TIMER_REQ, "SET_VZW_APN_TIMER_REQ", 0, NULL},
    {MIPC_APN_SET_VZW_APN_TIMER_CNF, "SET_VZW_APN_TIMER_CNF", 0, NULL},
    {MIPC_APN_ADD_PROFILE_LIST_REQ, "ADD_PROFILE_LIST_REQ", 1, mipc_apn_add_profile_list_req_tlv_checker_info},
    {MIPC_APN_ADD_PROFILE_LIST_CNF, "ADD_PROFILE_LIST_CNF", 0, NULL},
    {MIPC_APN_SET_IA_MD_PREFER_REQ, "SET_IA_MD_PREFER_REQ", 1, mipc_apn_set_ia_md_prefer_req_tlv_checker_info},
    {MIPC_APN_SET_IA_MD_PREFER_CNF, "SET_IA_MD_PREFER_CNF", 0, NULL},
    {MIPC_DATA_ACT_CALL_REQ, "ACT_CALL_REQ", 13, mipc_data_act_call_req_tlv_checker_info},
    {MIPC_DATA_ACT_CALL_CNF, "ACT_CALL_CNF", 0, NULL},
    {MIPC_DATA_DEACT_CALL_REQ, "DEACT_CALL_REQ", 1, mipc_data_deact_call_req_tlv_checker_info},
    {MIPC_DATA_DEACT_CALL_CNF, "DEACT_CALL_CNF", 0, NULL},
    {MIPC_DATA_GET_CALL_REQ, "GET_CALL_REQ", 0, NULL},
    {MIPC_DATA_GET_CALL_CNF, "GET_CALL_CNF", 0, NULL},
    {MIPC_DATA_SET_PACKET_FILTER_REQ, "SET_PACKET_FILTER_REQ", 1, mipc_data_set_packet_filter_req_tlv_checker_info},
    {MIPC_DATA_SET_PACKET_FILTER_CNF, "SET_PACKET_FILTER_CNF", 0, NULL},
    {MIPC_DATA_GET_PACKET_FILTER_REQ, "GET_PACKET_FILTER_REQ", 0, NULL},
    {MIPC_DATA_GET_PACKET_FILTER_CNF, "GET_PACKET_FILTER_CNF", 0, NULL},
    {MIPC_DATA_GET_PCO_REQ, "GET_PCO_REQ", 2, mipc_data_get_pco_req_tlv_checker_info},
    {MIPC_DATA_GET_PCO_CNF, "GET_PCO_CNF", 0, NULL},
    {MIPC_DATA_SET_DATA_ALLOW_REQ, "SET_DATA_ALLOW_REQ", 0, NULL},
    {MIPC_DATA_SET_DATA_ALLOW_CNF, "SET_DATA_ALLOW_CNF", 0, NULL},
    {MIPC_DATA_GET_MD_DATA_CALL_LIST_REQ, "GET_MD_DATA_CALL_LIST_REQ", 0, NULL},
    {MIPC_DATA_GET_MD_DATA_CALL_LIST_CNF, "GET_MD_DATA_CALL_LIST_CNF", 0, NULL},
    {MIPC_DATA_SET_CONFIG_REQ, "SET_CONFIG_REQ", 6, mipc_data_set_config_req_tlv_checker_info},
    {MIPC_DATA_SET_CONFIG_CNF, "SET_CONFIG_CNF", 0, NULL},
    {MIPC_DATA_GET_CONFIG_REQ, "GET_CONFIG_REQ", 0, NULL},
    {MIPC_DATA_GET_CONFIG_CNF, "GET_CONFIG_CNF", 0, NULL},
    {MIPC_DATA_ABORT_CALL_REQ, "ABORT_CALL_REQ", 1, mipc_data_abort_call_req_tlv_checker_info},
    {MIPC_DATA_ABORT_CALL_CNF, "ABORT_CALL_CNF", 0, NULL},
    {MIPC_DATA_GET_CALL_INFO_REQ, "GET_CALL_INFO_REQ", 1, mipc_data_get_call_info_req_tlv_checker_info},
    {MIPC_DATA_GET_CALL_INFO_CNF, "GET_CALL_INFO_CNF", 0, NULL},
    {MIPC_DATA_GET_PDP_CID_REQ, "GET_PDP_CID_REQ", 0, NULL},
    {MIPC_DATA_GET_PDP_CID_CNF, "GET_PDP_CID_CNF", 0, NULL},
    {MIPC_DATA_RETRY_TIMER_REQ, "RETRY_TIMER_REQ", 2, mipc_data_retry_timer_req_tlv_checker_info},
    {MIPC_DATA_RETRY_TIMER_CNF, "RETRY_TIMER_CNF", 0, NULL},
    {MIPC_DATA_SET_LINK_CAPACITY_REPORTING_CRITERIA_REQ, "SET_LINK_CAPACITY_REPORTING_CRITERIA_REQ", 3, mipc_data_set_link_capacity_reporting_criteria_req_tlv_checker_info},
    {MIPC_DATA_SET_LINK_CAPACITY_REPORTING_CRITERIA_CNF, "SET_LINK_CAPACITY_REPORTING_CRITERIA_CNF", 0, NULL},
    {MIPC_DATA_GET_DEDICATE_BEARER_INFO_REQ, "GET_DEDICATE_BEARER_INFO_REQ", 0, NULL},
    {MIPC_DATA_GET_DEDICATE_BEARER_INFO_CNF, "GET_DEDICATE_BEARER_INFO_CNF", 0, NULL},
    {MIPC_DATA_GET_QOS_REQ, "GET_QOS_REQ", 0, NULL},
    {MIPC_DATA_GET_QOS_CNF, "GET_QOS_CNF", 0, NULL},
    {MIPC_DATA_GET_TFT_REQ, "GET_TFT_REQ", 0, NULL},
    {MIPC_DATA_GET_TFT_CNF, "GET_TFT_CNF", 0, NULL},
    {MIPC_DATA_SET_LGDCONT_REQ, "SET_LGDCONT_REQ", 3, mipc_data_set_lgdcont_req_tlv_checker_info},
    {MIPC_DATA_SET_LGDCONT_CNF, "SET_LGDCONT_CNF", 0, NULL},
    {MIPC_DATA_SET_NSSAI_REQ, "SET_NSSAI_REQ", 2, mipc_data_set_nssai_req_tlv_checker_info},
    {MIPC_DATA_SET_NSSAI_CNF, "SET_NSSAI_CNF", 0, NULL},
    {MIPC_DATA_KEEPALIVE_REQ, "KEEPALIVE_REQ", 1, mipc_data_keepalive_req_tlv_checker_info},
    {MIPC_DATA_KEEPALIVE_CNF, "KEEPALIVE_CNF", 0, NULL},
    {MIPC_DATA_GET_DSDA_STATE_REQ, "GET_DSDA_STATE_REQ", 0, NULL},
    {MIPC_DATA_GET_DSDA_STATE_CNF, "GET_DSDA_STATE_CNF", 0, NULL},
    {MIPC_DATA_GET_5GQOS_REQ, "GET_5GQOS_REQ", 0, NULL},
    {MIPC_DATA_GET_5GQOS_CNF, "GET_5GQOS_CNF", 0, NULL},
    {MIPC_DATA_SET_PSI_REQ, "SET_PSI_REQ", 2, mipc_data_set_psi_req_tlv_checker_info},
    {MIPC_DATA_SET_PSI_CNF, "SET_PSI_CNF", 0, NULL},
    {MIPC_DATA_GET_NSSAI_REQ, "GET_NSSAI_REQ", 2, mipc_data_get_nssai_req_tlv_checker_info},
    {MIPC_DATA_GET_NSSAI_CNF, "GET_NSSAI_CNF", 0, NULL},
    {MIPC_DATA_GET_URSP_ROUTE_PROFILE_REQ, "GET_URSP_ROUTE_PROFILE_REQ", 0, NULL},
    {MIPC_DATA_GET_URSP_ROUTE_PROFILE_CNF, "GET_URSP_ROUTE_PROFILE_CNF", 0, NULL},
    {MIPC_DATA_SET_URSP_PRECONF_UE_POLICY_REQ, "SET_URSP_PRECONF_UE_POLICY_REQ", 2, mipc_data_set_ursp_preconf_ue_policy_req_tlv_checker_info},
    {MIPC_DATA_SET_URSP_PRECONF_UE_POLICY_CNF, "SET_URSP_PRECONF_UE_POLICY_CNF", 0, NULL},
    {MIPC_DATA_GET_URSP_UE_POLICY_REQ, "GET_URSP_UE_POLICY_REQ", 1, mipc_data_get_ursp_ue_policy_req_tlv_checker_info},
    {MIPC_DATA_GET_URSP_UE_POLICY_CNF, "GET_URSP_UE_POLICY_CNF", 0, NULL},
    {MIPC_DATA_SET_RESERVED_IF_ID_REQ, "SET_RESERVED_IF_ID_REQ", 1, mipc_data_set_reserved_if_id_req_tlv_checker_info},
    {MIPC_DATA_SET_RESERVED_IF_ID_CNF, "SET_RESERVED_IF_ID_CNF", 0, NULL},
    {MIPC_DATA_SET_PCO_REQ, "SET_PCO_REQ", 2, mipc_data_set_pco_req_tlv_checker_info},
    {MIPC_DATA_SET_PCO_CNF, "SET_PCO_CNF", 0, NULL},
    {MIPC_DATA_MOD_CALL_REQ, "MOD_CALL_REQ", 3, mipc_data_mod_call_req_tlv_checker_info},
    {MIPC_DATA_MOD_CALL_CNF, "MOD_CALL_CNF", 0, NULL},
    {MIPC_DATA_SET_TSN_REQ, "SET_TSN_REQ", 1, mipc_data_set_tsn_req_tlv_checker_info},
    {MIPC_DATA_SET_TSN_CNF, "SET_TSN_CNF", 0, NULL},
    {MIPC_DATA_SET_IP_TUPLE_PACKET_FILTER_REQ, "SET_IP_TUPLE_PACKET_FILTER_REQ", 4, mipc_data_set_ip_tuple_packet_filter_req_tlv_checker_info},
    {MIPC_DATA_SET_IP_TUPLE_PACKET_FILTER_CNF, "SET_IP_TUPLE_PACKET_FILTER_CNF", 0, NULL},
    {MIPC_DATA_DEL_IP_TUPLE_PACKET_FILTER_REQ, "DEL_IP_TUPLE_PACKET_FILTER_REQ", 0, NULL},
    {MIPC_DATA_DEL_IP_TUPLE_PACKET_FILTER_CNF, "DEL_IP_TUPLE_PACKET_FILTER_CNF", 0, NULL},
    {MIPC_DATA_SET_IP_TUPLE_PACKET_FILTER_MODE_REQ, "SET_IP_TUPLE_PACKET_FILTER_MODE_REQ", 0, NULL},
    {MIPC_DATA_SET_IP_TUPLE_PACKET_FILTER_MODE_CNF, "SET_IP_TUPLE_PACKET_FILTER_MODE_CNF", 0, NULL},
    {MIPC_DATA_GET_IP_TUPLE_PACKET_FILTER_MODE_REQ, "GET_IP_TUPLE_PACKET_FILTER_MODE_REQ", 0, NULL},
    {MIPC_DATA_GET_IP_TUPLE_PACKET_FILTER_MODE_CNF, "GET_IP_TUPLE_PACKET_FILTER_MODE_CNF", 0, NULL},
    {MIPC_DATA_GET_URSP_UE_POLICY_PLMN_LIST_REQ, "GET_URSP_UE_POLICY_PLMN_LIST_REQ", 0, NULL},
    {MIPC_DATA_GET_URSP_UE_POLICY_PLMN_LIST_CNF, "GET_URSP_UE_POLICY_PLMN_LIST_CNF", 0, NULL},
    {MIPC_DATA_GET_TSN_REQ, "GET_TSN_REQ", 0, NULL},
    {MIPC_DATA_GET_TSN_CNF, "GET_TSN_CNF", 0, NULL},
    {MIPC_DATA_ACT_DEDICATED_CALL_REQ, "ACT_DEDICATED_CALL_REQ", 3, mipc_data_act_dedicated_call_req_tlv_checker_info},
    {MIPC_DATA_ACT_DEDICATED_CALL_CNF, "ACT_DEDICATED_CALL_CNF", 0, NULL},
    {MIPC_INTERNAL_OPEN_REQ, "OPEN_REQ", 2, mipc_internal_open_req_tlv_checker_info},
    {MIPC_INTERNAL_OPEN_CNF, "OPEN_CNF", 0, NULL},
    {MIPC_INTERNAL_CLOSE_REQ, "CLOSE_REQ", 0, NULL},
    {MIPC_INTERNAL_CLOSE_CNF, "CLOSE_CNF", 0, NULL},
    {MIPC_INTERNAL_TEST_REQ, "TEST_REQ", 0, NULL},
    {MIPC_INTERNAL_TEST_CNF, "TEST_CNF", 0, NULL},
    {MIPC_INTERNAL_REGISTER_IND_REQ, "REGISTER_IND_REQ", 0, NULL},
    {MIPC_INTERNAL_REGISTER_IND_CNF, "REGISTER_IND_CNF", 0, NULL},
    {MIPC_INTERNAL_UNREGISTER_IND_REQ, "UNREGISTER_IND_REQ", 1, mipc_internal_unregister_ind_req_tlv_checker_info},
    {MIPC_INTERNAL_UNREGISTER_IND_CNF, "UNREGISTER_IND_CNF", 0, NULL},
    {MIPC_INTERNAL_REGISTER_CMD_REQ, "REGISTER_CMD_REQ", 0, NULL},
    {MIPC_INTERNAL_REGISTER_CMD_CNF, "REGISTER_CMD_CNF", 0, NULL},
    {MIPC_INTERNAL_UNREGISTER_CMD_REQ, "UNREGISTER_CMD_REQ", 0, NULL},
    {MIPC_INTERNAL_UNREGISTER_CMD_CNF, "UNREGISTER_CMD_CNF", 0, NULL},
    {MIPC_INTERNAL_SET_FILTER_REQ, "SET_FILTER_REQ", 1, mipc_internal_set_filter_req_tlv_checker_info},
    {MIPC_INTERNAL_SET_FILTER_CNF, "SET_FILTER_CNF", 0, NULL},
    {MIPC_INTERNAL_RESET_FILTER_REQ, "RESET_FILTER_REQ", 1, mipc_internal_reset_filter_req_tlv_checker_info},
    {MIPC_INTERNAL_RESET_FILTER_CNF, "RESET_FILTER_CNF", 0, NULL},
    {MIPC_INTERNAL_EIF_REQ, "EIF_REQ", 4, mipc_internal_eif_req_tlv_checker_info},
    {MIPC_INTERNAL_EIF_CNF, "EIF_CNF", 0, NULL},
    {MIPC_INTERNAL_SET_LONG_STANDBY_MONITOR_REQ, "SET_LONG_STANDBY_MONITOR_REQ", 1, mipc_internal_set_long_standby_monitor_req_tlv_checker_info},
    {MIPC_INTERNAL_SET_LONG_STANDBY_MONITOR_CNF, "SET_LONG_STANDBY_MONITOR_CNF", 0, NULL},
    {MIPC_INTERNAL_SET_LONG_STANDBY_MONITOR_TIME_REQ, "SET_LONG_STANDBY_MONITOR_TIME_REQ", 0, NULL},
    {MIPC_INTERNAL_SET_LONG_STANDBY_MONITOR_TIME_CNF, "SET_LONG_STANDBY_MONITOR_TIME_CNF", 0, NULL},
    {MIPC_INTERNAL_SET_LONG_STANDBY_MONITOR_WARNING_RATIO_REQ, "SET_LONG_STANDBY_MONITOR_WARNING_RATIO_REQ", 0, NULL},
    {MIPC_INTERNAL_SET_LONG_STANDBY_MONITOR_WARNING_RATIO_CNF, "SET_LONG_STANDBY_MONITOR_WARNING_RATIO_CNF", 0, NULL},
    {MIPC_NW_GET_RADIO_STATE_REQ, "GET_RADIO_STATE_REQ", 0, NULL},
    {MIPC_NW_GET_RADIO_STATE_CNF, "GET_RADIO_STATE_CNF", 0, NULL},
    {MIPC_NW_SET_RADIO_STATE_REQ, "SET_RADIO_STATE_REQ", 3, mipc_nw_set_radio_state_req_tlv_checker_info},
    {MIPC_NW_SET_RADIO_STATE_CNF, "SET_RADIO_STATE_CNF", 0, NULL},
    {MIPC_NW_SET_REGISTER_STATE_REQ, "SET_REGISTER_STATE_REQ", 3, mipc_nw_set_register_state_req_tlv_checker_info},
    {MIPC_NW_SET_REGISTER_STATE_CNF, "SET_REGISTER_STATE_CNF", 0, NULL},
    {MIPC_NW_GET_REGISTER_STATE_REQ, "GET_REGISTER_STATE_REQ", 0, NULL},
    {MIPC_NW_GET_REGISTER_STATE_CNF, "GET_REGISTER_STATE_CNF", 0, NULL},
    {MIPC_NW_GET_PLMN_LIST_REQ, "GET_PLMN_LIST_REQ", 0, NULL},
    {MIPC_NW_GET_PLMN_LIST_CNF, "GET_PLMN_LIST_CNF", 0, NULL},
    {MIPC_NW_SET_PS_REQ, "SET_PS_REQ", 2, mipc_nw_set_ps_req_tlv_checker_info},
    {MIPC_NW_SET_PS_CNF, "SET_PS_CNF", 0, NULL},
    {MIPC_NW_GET_PS_REQ, "GET_PS_REQ", 0, NULL},
    {MIPC_NW_GET_PS_CNF, "GET_PS_CNF", 0, NULL},
    {MIPC_NW_SET_SIGNAL_REQ, "SET_SIGNAL_REQ", 1, mipc_nw_set_signal_req_tlv_checker_info},
    {MIPC_NW_SET_SIGNAL_CNF, "SET_SIGNAL_CNF", 0, NULL},
    {MIPC_NW_GET_SIGNAL_REQ, "GET_SIGNAL_REQ", 0, NULL},
    {MIPC_NW_GET_SIGNAL_CNF, "GET_SIGNAL_CNF", 0, NULL},
    {MIPC_NW_GET_PREFERRED_PROVIDER_REQ, "GET_PREFERRED_PROVIDER_REQ", 0, NULL},
    {MIPC_NW_GET_PREFERRED_PROVIDER_CNF, "GET_PREFERRED_PROVIDER_CNF", 0, NULL},
    {MIPC_NW_SET_PREFERRED_PROVIDER_REQ, "SET_PREFERRED_PROVIDER_REQ", 1, mipc_nw_set_preferred_provider_req_tlv_checker_info},
    {MIPC_NW_SET_PREFERRED_PROVIDER_CNF, "SET_PREFERRED_PROVIDER_CNF", 0, NULL},
    {MIPC_NW_SET_HOME_PROVIDER_REQ, "SET_HOME_PROVIDER_REQ", 1, mipc_nw_set_home_provider_req_tlv_checker_info},
    {MIPC_NW_SET_HOME_PROVIDER_CNF, "SET_HOME_PROVIDER_CNF", 0, NULL},
    {MIPC_NW_GET_HOME_PROVIDER_REQ, "GET_HOME_PROVIDER_REQ", 0, NULL},
    {MIPC_NW_GET_HOME_PROVIDER_CNF, "GET_HOME_PROVIDER_CNF", 0, NULL},
    {MIPC_NW_GET_IA_STATUS_REQ, "GET_IA_STATUS_REQ", 0, NULL},
    {MIPC_NW_GET_IA_STATUS_CNF, "GET_IA_STATUS_CNF", 0, NULL},
    {MIPC_NW_GET_NITZ_REQ, "GET_NITZ_REQ", 0, NULL},
    {MIPC_NW_GET_NITZ_CNF, "GET_NITZ_CNF", 0, NULL},
    {MIPC_NW_SET_IDLE_HINT_REQ, "SET_IDLE_HINT_REQ", 1, mipc_nw_set_idle_hint_req_tlv_checker_info},
    {MIPC_NW_SET_IDLE_HINT_CNF, "SET_IDLE_HINT_CNF", 0, NULL},
    {MIPC_NW_GET_IDLE_HINT_REQ, "GET_IDLE_HINT_REQ", 0, NULL},
    {MIPC_NW_GET_IDLE_HINT_CNF, "GET_IDLE_HINT_CNF", 0, NULL},
    {MIPC_NW_GET_BASE_STATIONS_REQ, "GET_BASE_STATIONS_REQ", 0, NULL},
    {MIPC_NW_GET_BASE_STATIONS_CNF, "GET_BASE_STATIONS_CNF", 0, NULL},
    {MIPC_NW_GET_LOCATION_INFO_REQ, "GET_LOCATION_INFO_REQ", 0, NULL},
    {MIPC_NW_GET_LOCATION_INFO_CNF, "GET_LOCATION_INFO_CNF", 0, NULL},
    {MIPC_NW_SET_RAT_REQ, "SET_RAT_REQ", 1, mipc_nw_set_rat_req_tlv_checker_info},
    {MIPC_NW_SET_RAT_CNF, "SET_RAT_CNF", 0, NULL},
    {MIPC_NW_GET_PROVIDER_NAME_REQ, "GET_PROVIDER_NAME_REQ", 1, mipc_nw_get_provider_name_req_tlv_checker_info},
    {MIPC_NW_GET_PROVIDER_NAME_CNF, "GET_PROVIDER_NAME_CNF", 0, NULL},
    {MIPC_NW_GET_RAT_REQ, "GET_RAT_REQ", 0, NULL},
    {MIPC_NW_GET_RAT_CNF, "GET_RAT_CNF", 0, NULL},
    {MIPC_NW_SET_NR_REQ, "SET_NR_REQ", 1, mipc_nw_set_nr_req_tlv_checker_info},
    {MIPC_NW_SET_NR_CNF, "SET_NR_CNF", 0, NULL},
    {MIPC_NW_GET_CS_REQ, "GET_CS_REQ", 0, NULL},
    {MIPC_NW_GET_CS_CNF, "GET_CS_CNF", 0, NULL},
    {MIPC_NW_GET_BAND_MODE_REQ, "GET_BAND_MODE_REQ", 1, mipc_nw_get_band_mode_req_tlv_checker_info},
    {MIPC_NW_GET_BAND_MODE_CNF, "GET_BAND_MODE_CNF", 0, NULL},
    {MIPC_NW_SET_BAND_MODE_REQ, "SET_BAND_MODE_REQ", 3, mipc_nw_set_band_mode_req_tlv_checker_info},
    {MIPC_NW_SET_BAND_MODE_CNF, "SET_BAND_MODE_CNF", 0, NULL},
    {MIPC_NW_SET_CHANNEL_LOCK_REQ, "SET_CHANNEL_LOCK_REQ", 2, mipc_nw_set_channel_lock_req_tlv_checker_info},
    {MIPC_NW_SET_CHANNEL_LOCK_CNF, "SET_CHANNEL_LOCK_CNF", 0, NULL},
    {MIPC_NW_GET_CHANNEL_LOCK_REQ, "GET_CHANNEL_LOCK_REQ", 0, NULL},
    {MIPC_NW_GET_CHANNEL_LOCK_CNF, "GET_CHANNEL_LOCK_CNF", 0, NULL},
    {MIPC_NW_SET_ABORT_PLMN_LIST_SEARCH_REQ, "SET_ABORT_PLMN_LIST_SEARCH_REQ", 0, NULL},
    {MIPC_NW_SET_ABORT_PLMN_LIST_SEARCH_CNF, "SET_ABORT_PLMN_LIST_SEARCH_CNF", 0, NULL},
    {MIPC_NW_GET_POL_CAPABILITY_REQ, "GET_POL_CAPABILITY_REQ", 0, NULL},
    {MIPC_NW_GET_POL_CAPABILITY_CNF, "GET_POL_CAPABILITY_CNF", 0, NULL},
    {MIPC_NW_SET_PREFER_RAT_REQ, "SET_PREFER_RAT_REQ", 1, mipc_nw_set_prefer_rat_req_tlv_checker_info},
    {MIPC_NW_SET_PREFER_RAT_CNF, "SET_PREFER_RAT_CNF", 0, NULL},
    {MIPC_NW_SET_LTE_CARRIER_AGGREGATION_SWITCH_REQ, "SET_LTE_CARRIER_AGGREGATION_SWITCH_REQ", 1, mipc_nw_set_lte_carrier_aggregation_switch_req_tlv_checker_info},
    {MIPC_NW_SET_LTE_CARRIER_AGGREGATION_SWITCH_CNF, "SET_LTE_CARRIER_AGGREGATION_SWITCH_CNF", 0, NULL},
    {MIPC_NW_SET_PS_CS_REGISTRATION_STATE_ROAMING_TYPE_REQ, "SET_PS_CS_REGISTRATION_STATE_ROAMING_TYPE_REQ", 1, mipc_nw_set_ps_cs_registration_state_roaming_type_req_tlv_checker_info},
    {MIPC_NW_SET_PS_CS_REGISTRATION_STATE_ROAMING_TYPE_CNF, "SET_PS_CS_REGISTRATION_STATE_ROAMING_TYPE_CNF", 0, NULL},
    {MIPC_NW_GET_LTE_CARRIER_AGGREGATION_SWITCH_REQ, "GET_LTE_CARRIER_AGGREGATION_SWITCH_REQ", 0, NULL},
    {MIPC_NW_GET_LTE_CARRIER_AGGREGATION_SWITCH_CNF, "GET_LTE_CARRIER_AGGREGATION_SWITCH_CNF", 0, NULL},
    {MIPC_NW_CELL_MEASUREMENT_REQ, "CELL_MEASUREMENT_REQ", 4, mipc_nw_cell_measurement_req_tlv_checker_info},
    {MIPC_NW_CELL_MEASUREMENT_CNF, "CELL_MEASUREMENT_CNF", 0, NULL},
    {MIPC_NW_CELL_BAND_WHITE_LIST_LOCK_REQ, "CELL_BAND_WHITE_LIST_LOCK_REQ", 7, mipc_nw_cell_band_white_list_lock_req_tlv_checker_info},
    {MIPC_NW_CELL_BAND_WHITE_LIST_LOCK_CNF, "CELL_BAND_WHITE_LIST_LOCK_CNF", 0, NULL},
    {MIPC_NW_GET_CELL_BAND_BANDWIDTH_REQ, "GET_CELL_BAND_BANDWIDTH_REQ", 0, NULL},
    {MIPC_NW_GET_CELL_BAND_BANDWIDTH_CNF, "GET_CELL_BAND_BANDWIDTH_CNF", 0, NULL},
    {MIPC_NW_GET_NR_REQ, "GET_NR_REQ", 0, NULL},
    {MIPC_NW_GET_NR_CNF, "GET_NR_CNF", 0, NULL},
    {MIPC_NW_GET_SRXLEV_REQ, "GET_SRXLEV_REQ", 0, NULL},
    {MIPC_NW_GET_SRXLEV_CNF, "GET_SRXLEV_CNF", 0, NULL},
    {MIPC_NW_SET_ROAMING_MODE_REQ, "SET_ROAMING_MODE_REQ", 0, NULL},
    {MIPC_NW_SET_ROAMING_MODE_CNF, "SET_ROAMING_MODE_CNF", 0, NULL},
    {MIPC_NW_GET_ROAMING_MODE_REQ, "GET_ROAMING_MODE_REQ", 0, NULL},
    {MIPC_NW_GET_ROAMING_MODE_CNF, "GET_ROAMING_MODE_CNF", 0, NULL},
    {MIPC_NW_SET_URC_ENABLE_REQ, "SET_URC_ENABLE_REQ", 1, mipc_nw_set_urc_enable_req_tlv_checker_info},
    {MIPC_NW_SET_URC_ENABLE_CNF, "SET_URC_ENABLE_CNF", 0, NULL},
    {MIPC_NW_GET_SUGGESTED_PLMN_LIST_REQ, "GET_SUGGESTED_PLMN_LIST_REQ", 0, NULL},
    {MIPC_NW_GET_SUGGESTED_PLMN_LIST_CNF, "GET_SUGGESTED_PLMN_LIST_CNF", 0, NULL},
    {MIPC_NW_SET_SEARCH_STORE_FREQUENCY_INFO_REQ, "SET_SEARCH_STORE_FREQUENCY_INFO_REQ", 2, mipc_nw_set_search_store_frequency_info_req_tlv_checker_info},
    {MIPC_NW_SET_SEARCH_STORE_FREQUENCY_INFO_CNF, "SET_SEARCH_STORE_FREQUENCY_INFO_CNF", 0, NULL},
    {MIPC_NW_SET_SELECT_FEMTOCELL_REQ, "SET_SELECT_FEMTOCELL_REQ", 2, mipc_nw_set_select_femtocell_req_tlv_checker_info},
    {MIPC_NW_SET_SELECT_FEMTOCELL_CNF, "SET_SELECT_FEMTOCELL_CNF", 0, NULL},
    {MIPC_NW_SET_CONFIG_A2_OFFSET_REQ, "SET_CONFIG_A2_OFFSET_REQ", 0, NULL},
    {MIPC_NW_SET_CONFIG_A2_OFFSET_CNF, "SET_CONFIG_A2_OFFSET_CNF", 0, NULL},
    {MIPC_NW_SET_CONFIG_B1_OFFSET_REQ, "SET_CONFIG_B1_OFFSET_REQ", 0, NULL},
    {MIPC_NW_SET_CONFIG_B1_OFFSET_CNF, "SET_CONFIG_B1_OFFSET_CNF", 0, NULL},
    {MIPC_NW_SET_REPORT_ANBR_REQ, "SET_REPORT_ANBR_REQ", 0, NULL},
    {MIPC_NW_SET_REPORT_ANBR_CNF, "SET_REPORT_ANBR_CNF", 0, NULL},
    {MIPC_NW_SET_NETWORK_EVENT_REQ, "SET_NETWORK_EVENT_REQ", 0, NULL},
    {MIPC_NW_SET_NETWORK_EVENT_CNF, "SET_NETWORK_EVENT_CNF", 0, NULL},
    {MIPC_NW_SET_ENABLE_CA_PLUS_FILTER_REQ, "SET_ENABLE_CA_PLUS_FILTER_REQ", 0, NULL},
    {MIPC_NW_SET_ENABLE_CA_PLUS_FILTER_CNF, "SET_ENABLE_CA_PLUS_FILTER_CNF", 0, NULL},
    {MIPC_NW_ABORT_FEMTOCELL_LIST_REQ, "ABORT_FEMTOCELL_LIST_REQ", 0, NULL},
    {MIPC_NW_ABORT_FEMTOCELL_LIST_CNF, "ABORT_FEMTOCELL_LIST_CNF", 0, NULL},
    {MIPC_NW_GET_FEMTOCELL_LIST_REQ, "GET_FEMTOCELL_LIST_REQ", 0, NULL},
    {MIPC_NW_GET_FEMTOCELL_LIST_CNF, "GET_FEMTOCELL_LIST_CNF", 0, NULL},
    {MIPC_NW_SET_PSEUDO_CELL_MODE_REQ, "SET_PSEUDO_CELL_MODE_REQ", 0, NULL},
    {MIPC_NW_SET_PSEUDO_CELL_MODE_CNF, "SET_PSEUDO_CELL_MODE_CNF", 0, NULL},
    {MIPC_NW_GET_PSEUDO_CELL_INFO_REQ, "GET_PSEUDO_CELL_INFO_REQ", 0, NULL},
    {MIPC_NW_GET_PSEUDO_CELL_INFO_CNF, "GET_PSEUDO_CELL_INFO_CNF", 0, NULL},
    {MIPC_NW_SET_ROAMING_ENABLE_REQ, "SET_ROAMING_ENABLE_REQ", 0, NULL},
    {MIPC_NW_SET_ROAMING_ENABLE_CNF, "SET_ROAMING_ENABLE_CNF", 0, NULL},
    {MIPC_NW_GET_ROAMING_ENABLE_REQ, "GET_ROAMING_ENABLE_REQ", 0, NULL},
    {MIPC_NW_GET_ROAMING_ENABLE_CNF, "GET_ROAMING_ENABLE_CNF", 0, NULL},
    {MIPC_NW_SET_FEMTOCELL_SYSTEM_SELECTION_MODE_REQ, "SET_FEMTOCELL_SYSTEM_SELECTION_MODE_REQ", 0, NULL},
    {MIPC_NW_SET_FEMTOCELL_SYSTEM_SELECTION_MODE_CNF, "SET_FEMTOCELL_SYSTEM_SELECTION_MODE_CNF", 0, NULL},
    {MIPC_NW_QUERY_FEMTOCELL_SYSTEM_SELECTION_MODE_REQ, "QUERY_FEMTOCELL_SYSTEM_SELECTION_MODE_REQ", 0, NULL},
    {MIPC_NW_QUERY_FEMTOCELL_SYSTEM_SELECTION_MODE_CNF, "QUERY_FEMTOCELL_SYSTEM_SELECTION_MODE_CNF", 0, NULL},
    {MIPC_NW_SET_NW_IND_REPORT_LEVEL_REQ, "SET_NW_IND_REPORT_LEVEL_REQ", 0, NULL},
    {MIPC_NW_SET_NW_IND_REPORT_LEVEL_CNF, "SET_NW_IND_REPORT_LEVEL_CNF", 0, NULL},
    {MIPC_NW_SET_DISABLE_2G_REQ, "SET_DISABLE_2G_REQ", 0, NULL},
    {MIPC_NW_SET_DISABLE_2G_CNF, "SET_DISABLE_2G_CNF", 0, NULL},
    {MIPC_NW_GET_DISABLE_2G_REQ, "GET_DISABLE_2G_REQ", 0, NULL},
    {MIPC_NW_GET_DISABLE_2G_CNF, "GET_DISABLE_2G_CNF", 0, NULL},
    {MIPC_NW_SET_SMART_RAT_SWITCH_REQ, "SET_SMART_RAT_SWITCH_REQ", 1, mipc_nw_set_smart_rat_switch_req_tlv_checker_info},
    {MIPC_NW_SET_SMART_RAT_SWITCH_CNF, "SET_SMART_RAT_SWITCH_CNF", 0, NULL},
    {MIPC_NW_GET_SMART_RAT_SWITCH_REQ, "GET_SMART_RAT_SWITCH_REQ", 1, mipc_nw_get_smart_rat_switch_req_tlv_checker_info},
    {MIPC_NW_GET_SMART_RAT_SWITCH_CNF, "GET_SMART_RAT_SWITCH_CNF", 0, NULL},
    {MIPC_NW_VSS_ANTENNA_CONF_REQ, "VSS_ANTENNA_CONF_REQ", 0, NULL},
    {MIPC_NW_VSS_ANTENNA_CONF_CNF, "VSS_ANTENNA_CONF_CNF", 0, NULL},
    {MIPC_NW_VSS_ANTENNA_INFO_REQ, "VSS_ANTENNA_INFO_REQ", 0, NULL},
    {MIPC_NW_VSS_ANTENNA_INFO_CNF, "VSS_ANTENNA_INFO_CNF", 0, NULL},
    {MIPC_NW_SET_RADIO_CAPABILITY_REQ, "SET_RADIO_CAPABILITY_REQ", 0, NULL},
    {MIPC_NW_SET_RADIO_CAPABILITY_CNF, "SET_RADIO_CAPABILITY_CNF", 0, NULL},
    {MIPC_NW_SET_CDMA_ROAMING_PREFERENCE_REQ, "SET_CDMA_ROAMING_PREFERENCE_REQ", 0, NULL},
    {MIPC_NW_SET_CDMA_ROAMING_PREFERENCE_CNF, "SET_CDMA_ROAMING_PREFERENCE_CNF", 0, NULL},
    {MIPC_NW_GET_CDMA_ROAMING_PREFERENCE_REQ, "GET_CDMA_ROAMING_PREFERENCE_REQ", 0, NULL},
    {MIPC_NW_GET_CDMA_ROAMING_PREFERENCE_CNF, "GET_CDMA_ROAMING_PREFERENCE_CNF", 0, NULL},
    {MIPC_NW_GET_BARRING_INFO_REQ, "GET_BARRING_INFO_REQ", 0, NULL},
    {MIPC_NW_GET_BARRING_INFO_CNF, "GET_BARRING_INFO_CNF", 0, NULL},
    {MIPC_NW_GET_EHRPD_INFO_REQ, "GET_EHRPD_INFO_REQ", 0, NULL},
    {MIPC_NW_GET_EHRPD_INFO_CNF, "GET_EHRPD_INFO_CNF", 0, NULL},
    {MIPC_NW_GET_EGMSS_REQ, "GET_EGMSS_REQ", 0, NULL},
    {MIPC_NW_GET_EGMSS_CNF, "GET_EGMSS_CNF", 0, NULL},
    {MIPC_NW_SET_CACHE_ENDC_CONNECT_MODE_REQ, "SET_CACHE_ENDC_CONNECT_MODE_REQ", 0, NULL},
    {MIPC_NW_SET_CACHE_ENDC_CONNECT_MODE_CNF, "SET_CACHE_ENDC_CONNECT_MODE_CNF", 0, NULL},
    {MIPC_NW_SET_PS_TEST_MODE_REQ, "SET_PS_TEST_MODE_REQ", 1, mipc_nw_set_ps_test_mode_req_tlv_checker_info},
    {MIPC_NW_SET_PS_TEST_MODE_CNF, "SET_PS_TEST_MODE_CNF", 0, NULL},
    {MIPC_NW_GET_PS_TEST_MODE_REQ, "GET_PS_TEST_MODE_REQ", 0, NULL},
    {MIPC_NW_GET_PS_TEST_MODE_CNF, "GET_PS_TEST_MODE_CNF", 0, NULL},
    {MIPC_NW_SET_SIGNAL_REPORT_CRITERIA_REQ, "SET_SIGNAL_REPORT_CRITERIA_REQ", 3, mipc_nw_set_signal_report_criteria_req_tlv_checker_info},
    {MIPC_NW_SET_SIGNAL_REPORT_CRITERIA_CNF, "SET_SIGNAL_REPORT_CRITERIA_CNF", 0, NULL},
    {MIPC_NW_GET_ECAINFO_REQ, "GET_ECAINFO_REQ", 0, NULL},
    {MIPC_NW_GET_ECAINFO_CNF, "GET_ECAINFO_CNF", 0, NULL},
    {MIPC_NW_GET_ACTIVITY_INFO_REQ, "GET_ACTIVITY_INFO_REQ", 0, NULL},
    {MIPC_NW_GET_ACTIVITY_INFO_CNF, "GET_ACTIVITY_INFO_CNF", 0, NULL},
    {MIPC_NW_SET_CA_REQ, "SET_CA_REQ", 2, mipc_nw_set_ca_req_tlv_checker_info},
    {MIPC_NW_SET_CA_CNF, "SET_CA_CNF", 0, NULL},
    {MIPC_NW_GET_LTE_RRC_STATE_REQ, "GET_LTE_RRC_STATE_REQ", 0, NULL},
    {MIPC_NW_GET_LTE_RRC_STATE_CNF, "GET_LTE_RRC_STATE_CNF", 0, NULL},
    {MIPC_NW_GET_LTE_1XRTT_CELL_LIST_REQ, "GET_LTE_1XRTT_CELL_LIST_REQ", 1, mipc_nw_get_lte_1xrtt_cell_list_req_tlv_checker_info},
    {MIPC_NW_GET_LTE_1XRTT_CELL_LIST_CNF, "GET_LTE_1XRTT_CELL_LIST_CNF", 0, NULL},
    {MIPC_NW_CLEAR_LTE_AVAILABLE_FILE_REQ, "CLEAR_LTE_AVAILABLE_FILE_REQ", 0, NULL},
    {MIPC_NW_CLEAR_LTE_AVAILABLE_FILE_CNF, "CLEAR_LTE_AVAILABLE_FILE_CNF", 0, NULL},
    {MIPC_NW_GET_CA_LINK_CAPABILITY_REQ, "GET_CA_LINK_CAPABILITY_REQ", 1, mipc_nw_get_ca_link_capability_req_tlv_checker_info},
    {MIPC_NW_GET_CA_LINK_CAPABILITY_CNF, "GET_CA_LINK_CAPABILITY_CNF", 0, NULL},
    {MIPC_NW_GET_CA_LINK_ENABLE_STATUS_REQ, "GET_CA_LINK_ENABLE_STATUS_REQ", 2, mipc_nw_get_ca_link_enable_status_req_tlv_checker_info},
    {MIPC_NW_GET_CA_LINK_ENABLE_STATUS_CNF, "GET_CA_LINK_ENABLE_STATUS_CNF", 0, NULL},
    {MIPC_NW_GET_TM9_ENABLE_STATUS_REQ, "GET_TM9_ENABLE_STATUS_REQ", 0, NULL},
    {MIPC_NW_GET_TM9_ENABLE_STATUS_CNF, "GET_TM9_ENABLE_STATUS_CNF", 0, NULL},
    {MIPC_NW_SET_TM9_ENABLE_STATUS_REQ, "SET_TM9_ENABLE_STATUS_REQ", 2, mipc_nw_set_tm9_enable_status_req_tlv_checker_info},
    {MIPC_NW_SET_TM9_ENABLE_STATUS_CNF, "SET_TM9_ENABLE_STATUS_CNF", 0, NULL},
    {MIPC_NW_GET_OMADM_CONF_REQ, "GET_OMADM_CONF_REQ", 1, mipc_nw_get_omadm_conf_req_tlv_checker_info},
    {MIPC_NW_GET_OMADM_CONF_CNF, "GET_OMADM_CONF_CNF", 0, NULL},
    {MIPC_NW_SET_OMADM_CONF_REQ, "SET_OMADM_CONF_REQ", 2, mipc_nw_set_omadm_conf_req_tlv_checker_info},
    {MIPC_NW_SET_OMADM_CONF_CNF, "SET_OMADM_CONF_CNF", 0, NULL},
    {MIPC_NW_GET_CA_BAND_MODE_REQ, "GET_CA_BAND_MODE_REQ", 0, NULL},
    {MIPC_NW_GET_CA_BAND_MODE_CNF, "GET_CA_BAND_MODE_CNF", 0, NULL},
    {MIPC_NW_SET_CA_LINK_ENABLE_STATUS_REQ, "SET_CA_LINK_ENABLE_STATUS_REQ", 2, mipc_nw_set_ca_link_enable_status_req_tlv_checker_info},
    {MIPC_NW_SET_CA_LINK_ENABLE_STATUS_CNF, "SET_CA_LINK_ENABLE_STATUS_CNF", 0, NULL},
    {MIPC_NW_GET_LTE_DATA_REQ, "GET_LTE_DATA_REQ", 0, NULL},
    {MIPC_NW_GET_LTE_DATA_CNF, "GET_LTE_DATA_CNF", 0, NULL},
    {MIPC_NW_SET_TUW_TIMER_LENGTH_REQ, "SET_TUW_TIMER_LENGTH_REQ", 1, mipc_nw_set_tuw_timer_length_req_tlv_checker_info},
    {MIPC_NW_SET_TUW_TIMER_LENGTH_CNF, "SET_TUW_TIMER_LENGTH_CNF", 0, NULL},
    {MIPC_NW_GET_TUW_TIMER_LENGTH_REQ, "GET_TUW_TIMER_LENGTH_REQ", 0, NULL},
    {MIPC_NW_GET_TUW_TIMER_LENGTH_CNF, "GET_TUW_TIMER_LENGTH_CNF", 0, NULL},
    {MIPC_NW_GET_5GUW_INFO_REQ, "GET_5GUW_INFO_REQ", 0, NULL},
    {MIPC_NW_GET_5GUW_INFO_CNF, "GET_5GUW_INFO_CNF", 0, NULL},
    {MIPC_NW_GET_NR_CA_BAND_REQ, "GET_NR_CA_BAND_REQ", 0, NULL},
    {MIPC_NW_GET_NR_CA_BAND_CNF, "GET_NR_CA_BAND_CNF", 0, NULL},
    {MIPC_NW_GET_NR_SCS_REQ, "GET_NR_SCS_REQ", 0, NULL},
    {MIPC_NW_GET_NR_SCS_CNF, "GET_NR_SCS_CNF", 0, NULL},
    {MIPC_NW_GET_PHYSICAL_CHANNEL_CONFIGS_REQ, "GET_PHYSICAL_CHANNEL_CONFIGS_REQ", 0, NULL},
    {MIPC_NW_GET_PHYSICAL_CHANNEL_CONFIGS_CNF, "GET_PHYSICAL_CHANNEL_CONFIGS_CNF", 0, NULL},
    {MIPC_NW_OS_ID_UPDATE_REQ, "OS_ID_UPDATE_REQ", 1, mipc_nw_os_id_update_req_tlv_checker_info},
    {MIPC_NW_OS_ID_UPDATE_CNF, "OS_ID_UPDATE_CNF", 0, NULL},
    {MIPC_NW_START_NETWORK_SCAN_REQ, "START_NETWORK_SCAN_REQ", 4, mipc_nw_start_network_scan_req_tlv_checker_info},
    {MIPC_NW_START_NETWORK_SCAN_CNF, "START_NETWORK_SCAN_CNF", 0, NULL},
    {MIPC_NW_STOP_NETWORK_SCAN_REQ, "STOP_NETWORK_SCAN_REQ", 0, NULL},
    {MIPC_NW_STOP_NETWORK_SCAN_CNF, "STOP_NETWORK_SCAN_CNF", 0, NULL},
    {MIPC_NW_GET_NETWORK_SCAN_SUPPORT_STATUS_REQ, "GET_NETWORK_SCAN_SUPPORT_STATUS_REQ", 0, NULL},
    {MIPC_NW_GET_NETWORK_SCAN_SUPPORT_STATUS_CNF, "GET_NETWORK_SCAN_SUPPORT_STATUS_CNF", 0, NULL},
    {MIPC_NW_SET_PREF_NSSAI_REQ, "SET_PREF_NSSAI_REQ", 2, mipc_nw_set_pref_nssai_req_tlv_checker_info},
    {MIPC_NW_SET_PREF_NSSAI_CNF, "SET_PREF_NSSAI_CNF", 0, NULL},
    {MIPC_NW_SET_DEFAULT_NSSAI_REQ, "SET_DEFAULT_NSSAI_REQ", 1, mipc_nw_set_default_nssai_req_tlv_checker_info},
    {MIPC_NW_SET_DEFAULT_NSSAI_CNF, "SET_DEFAULT_NSSAI_CNF", 0, NULL},
    {MIPC_NW_GET_NSSAI_REQ, "GET_NSSAI_REQ", 2, mipc_nw_get_nssai_req_tlv_checker_info},
    {MIPC_NW_GET_NSSAI_CNF, "GET_NSSAI_CNF", 0, NULL},
    {MIPC_NW_SET_5GUC_REQ, "SET_5GUC_REQ", 3, mipc_nw_set_5guc_req_tlv_checker_info},
    {MIPC_NW_SET_5GUC_CNF, "SET_5GUC_CNF", 0, NULL},
    {MIPC_NW_GET_5GUC_SETTING_REQ, "GET_5GUC_SETTING_REQ", 0, NULL},
    {MIPC_NW_GET_5GUC_SETTING_CNF, "GET_5GUC_SETTING_CNF", 0, NULL},
    {MIPC_NW_GET_5GUC_INFO_REQ, "GET_5GUC_INFO_REQ", 0, NULL},
    {MIPC_NW_GET_5GUC_INFO_CNF, "GET_5GUC_INFO_CNF", 0, NULL},
    {MIPC_NW_GET_FIRST_PLMN_REQ, "GET_FIRST_PLMN_REQ", 0, NULL},
    {MIPC_NW_GET_FIRST_PLMN_CNF, "GET_FIRST_PLMN_CNF", 0, NULL},
    {MIPC_NW_SET_UE_USAGE_SETTING_REQ, "SET_UE_USAGE_SETTING_REQ", 0, NULL},
    {MIPC_NW_SET_UE_USAGE_SETTING_CNF, "SET_UE_USAGE_SETTING_CNF", 0, NULL},
    {MIPC_NW_GET_UE_USAGE_SETTING_REQ, "GET_UE_USAGE_SETTING_REQ", 0, NULL},
    {MIPC_NW_GET_UE_USAGE_SETTING_CNF, "GET_UE_USAGE_SETTING_CNF", 0, NULL},
    {MIPC_NW_SET_CAG_STATUS_REQ, "SET_CAG_STATUS_REQ", 0, NULL},
    {MIPC_NW_SET_CAG_STATUS_CNF, "SET_CAG_STATUS_CNF", 0, NULL},
    {MIPC_NW_SET_CAG_SELECT_MODE_REQ, "SET_CAG_SELECT_MODE_REQ", 1, mipc_nw_set_cag_select_mode_req_tlv_checker_info},
    {MIPC_NW_SET_CAG_SELECT_MODE_CNF, "SET_CAG_SELECT_MODE_CNF", 0, NULL},
    {MIPC_NW_GET_CAG_LIST_REQ, "GET_CAG_LIST_REQ", 0, NULL},
    {MIPC_NW_GET_CAG_LIST_CNF, "GET_CAG_LIST_CNF", 0, NULL},
    {MIPC_NW_SET_ALLOWED_MCC_LIST_REQ, "SET_ALLOWED_MCC_LIST_REQ", 2, mipc_nw_set_allowed_mcc_list_req_tlv_checker_info},
    {MIPC_NW_SET_ALLOWED_MCC_LIST_CNF, "SET_ALLOWED_MCC_LIST_CNF", 0, NULL},
    {MIPC_NW_GET_LATEST_CA_MEASUREMENT_REQ, "GET_LATEST_CA_MEASUREMENT_REQ", 0, NULL},
    {MIPC_NW_GET_LATEST_CA_MEASUREMENT_CNF, "GET_LATEST_CA_MEASUREMENT_CNF", 0, NULL},
    {MIPC_SIM_PIN_PROTECT_REQ, "PIN_PROTECT_REQ", 3, mipc_sim_pin_protect_req_tlv_checker_info},
    {MIPC_SIM_PIN_PROTECT_CNF, "PIN_PROTECT_CNF", 0, NULL},
    {MIPC_SIM_CHANGE_PIN_REQ, "CHANGE_PIN_REQ", 3, mipc_sim_change_pin_req_tlv_checker_info},
    {MIPC_SIM_CHANGE_PIN_CNF, "CHANGE_PIN_CNF", 0, NULL},
    {MIPC_SIM_VERIFY_PIN_REQ, "VERIFY_PIN_REQ", 2, mipc_sim_verify_pin_req_tlv_checker_info},
    {MIPC_SIM_VERIFY_PIN_CNF, "VERIFY_PIN_CNF", 0, NULL},
    {MIPC_SIM_UNBLOCK_PIN_REQ, "UNBLOCK_PIN_REQ", 3, mipc_sim_unblock_pin_req_tlv_checker_info},
    {MIPC_SIM_UNBLOCK_PIN_CNF, "UNBLOCK_PIN_CNF", 0, NULL},
    {MIPC_SIM_GET_PIN_INFO_REQ, "GET_PIN_INFO_REQ", 0, NULL},
    {MIPC_SIM_GET_PIN_INFO_CNF, "GET_PIN_INFO_CNF", 0, NULL},
    {MIPC_SIM_GET_PIN_LIST_REQ, "GET_PIN_LIST_REQ", 0, NULL},
    {MIPC_SIM_GET_PIN_LIST_CNF, "GET_PIN_LIST_CNF", 0, NULL},
    {MIPC_SIM_STATE_REQ, "STATE_REQ", 0, NULL},
    {MIPC_SIM_STATE_CNF, "STATE_CNF", 0, NULL},
    {MIPC_SIM_STATUS_REQ, "STATUS_REQ", 0, NULL},
    {MIPC_SIM_STATUS_CNF, "STATUS_CNF", 0, NULL},
    {MIPC_SIM_ICCID_REQ, "ICCID_REQ", 0, NULL},
    {MIPC_SIM_ICCID_CNF, "ICCID_CNF", 0, NULL},
    {MIPC_SIM_IMSI_REQ, "IMSI_REQ", 1, mipc_sim_imsi_req_tlv_checker_info},
    {MIPC_SIM_IMSI_CNF, "IMSI_CNF", 0, NULL},
    {MIPC_SIM_MSISDN_REQ, "MSISDN_REQ", 0, NULL},
    {MIPC_SIM_MSISDN_CNF, "MSISDN_CNF", 0, NULL},
    {MIPC_SIM_GET_ATR_INFO_REQ, "GET_ATR_INFO_REQ", 0, NULL},
    {MIPC_SIM_GET_ATR_INFO_CNF, "GET_ATR_INFO_CNF", 0, NULL},
    {MIPC_SIM_OPEN_CHANNEL_REQ, "OPEN_CHANNEL_REQ", 1, mipc_sim_open_channel_req_tlv_checker_info},
    {MIPC_SIM_OPEN_CHANNEL_CNF, "OPEN_CHANNEL_CNF", 0, NULL},
    {MIPC_SIM_CLOSE_CHANNEL_REQ, "CLOSE_CHANNEL_REQ", 0, NULL},
    {MIPC_SIM_CLOSE_CHANNEL_CNF, "CLOSE_CHANNEL_CNF", 0, NULL},
    {MIPC_SIM_CHANNEL_RESTRICTED_ACCESS_REQ, "CHANNEL_RESTRICTED_ACCESS_REQ", 5, mipc_sim_channel_restricted_access_req_tlv_checker_info},
    {MIPC_SIM_CHANNEL_RESTRICTED_ACCESS_CNF, "CHANNEL_RESTRICTED_ACCESS_CNF", 0, NULL},
    {MIPC_SIM_CHANNEL_GENERIC_ACCESS_REQ, "CHANNEL_GENERIC_ACCESS_REQ", 2, mipc_sim_channel_generic_access_req_tlv_checker_info},
    {MIPC_SIM_CHANNEL_GENERIC_ACCESS_CNF, "CHANNEL_GENERIC_ACCESS_CNF", 0, NULL},
    {MIPC_SIM_LONG_APDU_ACCESS_REQ, "LONG_APDU_ACCESS_REQ", 4, mipc_sim_long_apdu_access_req_tlv_checker_info},
    {MIPC_SIM_LONG_APDU_ACCESS_CNF, "LONG_APDU_ACCESS_CNF", 0, NULL},
    {MIPC_SIM_APP_LIST_REQ, "APP_LIST_REQ", 0, NULL},
    {MIPC_SIM_APP_LIST_CNF, "APP_LIST_CNF", 0, NULL},
    {MIPC_SIM_FILE_STATUS_REQ, "FILE_STATUS_REQ", 2, mipc_sim_file_status_req_tlv_checker_info},
    {MIPC_SIM_FILE_STATUS_CNF, "FILE_STATUS_CNF", 0, NULL},
    {MIPC_SIM_GET_RESET_REQ, "GET_RESET_REQ", 0, NULL},
    {MIPC_SIM_GET_RESET_CNF, "GET_RESET_CNF", 0, NULL},
    {MIPC_SIM_SET_RESET_REQ, "SET_RESET_REQ", 1, mipc_sim_set_reset_req_tlv_checker_info},
    {MIPC_SIM_SET_RESET_CNF, "SET_RESET_CNF", 0, NULL},
    {MIPC_SIM_GET_TERMINAL_CAPABILITY_REQ, "GET_TERMINAL_CAPABILITY_REQ", 0, NULL},
    {MIPC_SIM_GET_TERMINAL_CAPABILITY_CNF, "GET_TERMINAL_CAPABILITY_CNF", 0, NULL},
    {MIPC_SIM_SET_TERMINAL_CAPABILITY_REQ, "SET_TERMINAL_CAPABILITY_REQ", 1, mipc_sim_set_terminal_capability_req_tlv_checker_info},
    {MIPC_SIM_SET_TERMINAL_CAPABILITY_CNF, "SET_TERMINAL_CAPABILITY_CNF", 0, NULL},
    {MIPC_SIM_SET_PIN_EX_REQ, "SET_PIN_EX_REQ", 5, mipc_sim_set_pin_ex_req_tlv_checker_info},
    {MIPC_SIM_SET_PIN_EX_CNF, "SET_PIN_EX_CNF", 0, NULL},
    {MIPC_SIM_GET_PIN_EX_REQ, "GET_PIN_EX_REQ", 1, mipc_sim_get_pin_ex_req_tlv_checker_info},
    {MIPC_SIM_GET_PIN_EX_CNF, "GET_PIN_EX_CNF", 0, NULL},
    {MIPC_SIM_GET_GSM_AUTH_REQ, "GET_GSM_AUTH_REQ", 3, mipc_sim_get_gsm_auth_req_tlv_checker_info},
    {MIPC_SIM_GET_GSM_AUTH_CNF, "GET_GSM_AUTH_CNF", 0, NULL},
    {MIPC_SIM_GET_EXT_AUTH_REQ, "GET_EXT_AUTH_REQ", 2, mipc_sim_get_ext_auth_req_tlv_checker_info},
    {MIPC_SIM_GET_EXT_AUTH_CNF, "GET_EXT_AUTH_CNF", 0, NULL},
    {MIPC_SIM_GET_FACILITY_REQ, "GET_FACILITY_REQ", 2, mipc_sim_get_facility_req_tlv_checker_info},
    {MIPC_SIM_GET_FACILITY_CNF, "GET_FACILITY_CNF", 0, NULL},
    {MIPC_SIM_SET_FACILITY_REQ, "SET_FACILITY_REQ", 3, mipc_sim_set_facility_req_tlv_checker_info},
    {MIPC_SIM_SET_FACILITY_CNF, "SET_FACILITY_CNF", 0, NULL},
    {MIPC_SIM_GET_EUICC_SLOTS_STATUS_REQ, "GET_EUICC_SLOTS_STATUS_REQ", 0, NULL},
    {MIPC_SIM_GET_EUICC_SLOTS_STATUS_CNF, "GET_EUICC_SLOTS_STATUS_CNF", 0, NULL},
    {MIPC_SIM_ACCESS_PROFILE_CONNECT_REQ, "ACCESS_PROFILE_CONNECT_REQ", 0, NULL},
    {MIPC_SIM_ACCESS_PROFILE_CONNECT_CNF, "ACCESS_PROFILE_CONNECT_CNF", 0, NULL},
    {MIPC_SIM_ACCESS_PROFILE_DISCONNECT_REQ, "ACCESS_PROFILE_DISCONNECT_REQ", 0, NULL},
    {MIPC_SIM_ACCESS_PROFILE_DISCONNECT_CNF, "ACCESS_PROFILE_DISCONNECT_CNF", 0, NULL},
    {MIPC_SIM_ACCESS_PROFILE_POWER_ON_REQ, "ACCESS_PROFILE_POWER_ON_REQ", 0, NULL},
    {MIPC_SIM_ACCESS_PROFILE_POWER_ON_CNF, "ACCESS_PROFILE_POWER_ON_CNF", 0, NULL},
    {MIPC_SIM_ACCESS_PROFILE_POWER_OFF_REQ, "ACCESS_PROFILE_POWER_OFF_REQ", 0, NULL},
    {MIPC_SIM_ACCESS_PROFILE_POWER_OFF_CNF, "ACCESS_PROFILE_POWER_OFF_CNF", 0, NULL},
    {MIPC_SIM_ACCESS_PROFILE_RESET_REQ, "ACCESS_PROFILE_RESET_REQ", 0, NULL},
    {MIPC_SIM_ACCESS_PROFILE_RESET_CNF, "ACCESS_PROFILE_RESET_CNF", 0, NULL},
    {MIPC_SIM_ACCESS_PROFILE_APDU_REQ, "ACCESS_PROFILE_APDU_REQ", 1, mipc_sim_access_profile_apdu_req_tlv_checker_info},
    {MIPC_SIM_ACCESS_PROFILE_APDU_CNF, "ACCESS_PROFILE_APDU_CNF", 0, NULL},
    {MIPC_SIM_SET_SIM_POWER_REQ, "SET_SIM_POWER_REQ", 0, NULL},
    {MIPC_SIM_SET_SIM_POWER_CNF, "SET_SIM_POWER_CNF", 0, NULL},
    {MIPC_SIM_SET_PHYSICAL_SLOTS_MAPPING_REQ, "SET_PHYSICAL_SLOTS_MAPPING_REQ", 1, mipc_sim_set_physical_slots_mapping_req_tlv_checker_info},
    {MIPC_SIM_SET_PHYSICAL_SLOTS_MAPPING_CNF, "SET_PHYSICAL_SLOTS_MAPPING_CNF", 0, NULL},
    {MIPC_SIM_EXTENDED_CHANNEL_GENERIC_ACCESS_REQ, "EXTENDED_CHANNEL_GENERIC_ACCESS_REQ", 1, mipc_sim_extended_channel_generic_access_req_tlv_checker_info},
    {MIPC_SIM_EXTENDED_CHANNEL_GENERIC_ACCESS_CNF, "EXTENDED_CHANNEL_GENERIC_ACCESS_CNF", 0, NULL},
    {MIPC_SIM_UICC_FILE_ACCESS_RECORD_REQ, "UICC_FILE_ACCESS_RECORD_REQ", 4, mipc_sim_uicc_file_access_record_req_tlv_checker_info},
    {MIPC_SIM_UICC_FILE_ACCESS_RECORD_CNF, "UICC_FILE_ACCESS_RECORD_CNF", 0, NULL},
    {MIPC_SIM_UICC_FILE_ACCESS_BINARY_REQ, "UICC_FILE_ACCESS_BINARY_REQ", 4, mipc_sim_uicc_file_access_binary_req_tlv_checker_info},
    {MIPC_SIM_UICC_FILE_ACCESS_BINARY_CNF, "UICC_FILE_ACCESS_BINARY_CNF", 0, NULL},
    {MIPC_SIM_GET_PHYSICAL_SLOTS_MAPPING_REQ, "GET_PHYSICAL_SLOTS_MAPPING_REQ", 0, NULL},
    {MIPC_SIM_GET_PHYSICAL_SLOTS_MAPPING_CNF, "GET_PHYSICAL_SLOTS_MAPPING_CNF", 0, NULL},
    {MIPC_SIM_GET_SIM_AUTH_REQ, "GET_SIM_AUTH_REQ", 2, mipc_sim_get_sim_auth_req_tlv_checker_info},
    {MIPC_SIM_GET_SIM_AUTH_CNF, "GET_SIM_AUTH_CNF", 0, NULL},
    {MIPC_SIM_SML_GET_ALLOWED_CARRIERS_REQ, "SML_GET_ALLOWED_CARRIERS_REQ", 0, NULL},
    {MIPC_SIM_SML_GET_ALLOWED_CARRIERS_CNF, "SML_GET_ALLOWED_CARRIERS_CNF", 0, NULL},
    {MIPC_SIM_SML_SET_ALLOWED_CARRIERS_REQ, "SML_SET_ALLOWED_CARRIERS_REQ", 3, mipc_sim_sml_set_allowed_carriers_req_tlv_checker_info},
    {MIPC_SIM_SML_SET_ALLOWED_CARRIERS_CNF, "SML_SET_ALLOWED_CARRIERS_CNF", 0, NULL},
    {MIPC_SIM_SML_ENTER_SIM_DEPERSONALIZATION_REQ, "SML_ENTER_SIM_DEPERSONALIZATION_REQ", 2, mipc_sim_sml_enter_sim_depersonalization_req_tlv_checker_info},
    {MIPC_SIM_SML_ENTER_SIM_DEPERSONALIZATION_CNF, "SML_ENTER_SIM_DEPERSONALIZATION_CNF", 0, NULL},
    {MIPC_SIM_SML_GET_LOCK_REQ, "SML_GET_LOCK_REQ", 1, mipc_sim_sml_get_lock_req_tlv_checker_info},
    {MIPC_SIM_SML_GET_LOCK_CNF, "SML_GET_LOCK_CNF", 0, NULL},
    {MIPC_SIM_SML_SET_LOCK_REQ, "SML_SET_LOCK_REQ", 6, mipc_sim_sml_set_lock_req_tlv_checker_info},
    {MIPC_SIM_SML_SET_LOCK_CNF, "SML_SET_LOCK_CNF", 0, NULL},
    {MIPC_SIM_SML_ENTER_DEVICE_DEPERSONALIZATION_REQ, "SML_ENTER_DEVICE_DEPERSONALIZATION_REQ", 1, mipc_sim_sml_enter_device_depersonalization_req_tlv_checker_info},
    {MIPC_SIM_SML_ENTER_DEVICE_DEPERSONALIZATION_CNF, "SML_ENTER_DEVICE_DEPERSONALIZATION_CNF", 0, NULL},
    {MIPC_SIM_SML_GET_DEV_LOCK_REQ, "SML_GET_DEV_LOCK_REQ", 0, NULL},
    {MIPC_SIM_SML_GET_DEV_LOCK_CNF, "SML_GET_DEV_LOCK_CNF", 0, NULL},
    {MIPC_SIM_SML_RSU_REQ, "SML_RSU_REQ", 3, mipc_sim_sml_rsu_req_tlv_checker_info},
    {MIPC_SIM_SML_RSU_CNF, "SML_RSU_CNF", 0, NULL},
    {MIPC_SIM_VSIM_SET_AKA_REQ, "VSIM_SET_AKA_REQ", 0, NULL},
    {MIPC_SIM_VSIM_SET_AKA_CNF, "VSIM_SET_AKA_CNF", 0, NULL},
    {MIPC_SIM_VSIM_ENABLE_REQ, "VSIM_ENABLE_REQ", 0, NULL},
    {MIPC_SIM_VSIM_ENABLE_CNF, "VSIM_ENABLE_CNF", 0, NULL},
    {MIPC_SIM_VSIM_DISABLE_REQ, "VSIM_DISABLE_REQ", 0, NULL},
    {MIPC_SIM_VSIM_DISABLE_CNF, "VSIM_DISABLE_CNF", 0, NULL},
    {MIPC_SIM_VSIM_PLUG_REQ, "VSIM_PLUG_REQ", 1, mipc_sim_vsim_plug_req_tlv_checker_info},
    {MIPC_SIM_VSIM_PLUG_CNF, "VSIM_PLUG_CNF", 0, NULL},
    {MIPC_SIM_VSIM_SET_TIMER_REQ, "VSIM_SET_TIMER_REQ", 1, mipc_sim_vsim_set_timer_req_tlv_checker_info},
    {MIPC_SIM_VSIM_SET_TIMER_CNF, "VSIM_SET_TIMER_CNF", 0, NULL},
    {MIPC_SIM_VSIM_RESET_REQ, "VSIM_RESET_REQ", 1, mipc_sim_vsim_reset_req_tlv_checker_info},
    {MIPC_SIM_VSIM_RESET_CNF, "VSIM_RESET_CNF", 0, NULL},
    {MIPC_SIM_VSIM_APDU_REQ, "VSIM_APDU_REQ", 1, mipc_sim_vsim_apdu_req_tlv_checker_info},
    {MIPC_SIM_VSIM_APDU_CNF, "VSIM_APDU_CNF", 0, NULL},
    {MIPC_SIM_VSIM_AUTH_REQ, "VSIM_AUTH_REQ", 1, mipc_sim_vsim_auth_req_tlv_checker_info},
    {MIPC_SIM_VSIM_AUTH_CNF, "VSIM_AUTH_CNF", 0, NULL},
    {MIPC_SIM_CDMA_SUBSCRIPTION_REQ, "CDMA_SUBSCRIPTION_REQ", 0, NULL},
    {MIPC_SIM_CDMA_SUBSCRIPTION_CNF, "CDMA_SUBSCRIPTION_CNF", 0, NULL},
    {MIPC_SIM_CDMA_GET_SUBSCRIPTION_SOURCE_REQ, "CDMA_GET_SUBSCRIPTION_SOURCE_REQ", 0, NULL},
    {MIPC_SIM_CDMA_GET_SUBSCRIPTION_SOURCE_CNF, "CDMA_GET_SUBSCRIPTION_SOURCE_CNF", 0, NULL},
    {MIPC_SIM_PIN_COUNT_QUERY_REQ, "PIN_COUNT_QUERY_REQ", 0, NULL},
    {MIPC_SIM_PIN_COUNT_QUERY_CNF, "PIN_COUNT_QUERY_CNF", 0, NULL},
    {MIPC_SIM_SML_GET_NETWORK_LOCK_REQ, "SML_GET_NETWORK_LOCK_REQ", 0, NULL},
    {MIPC_SIM_SML_GET_NETWORK_LOCK_CNF, "SML_GET_NETWORK_LOCK_CNF", 0, NULL},
    {MIPC_SIM_ESIM_SWITCH_SET_REQ, "ESIM_SWITCH_SET_REQ", 0, NULL},
    {MIPC_SIM_ESIM_SWITCH_SET_CNF, "ESIM_SWITCH_SET_CNF", 0, NULL},
    {MIPC_SIM_ESIM_SWITCH_GET_REQ, "ESIM_SWITCH_GET_REQ", 0, NULL},
    {MIPC_SIM_ESIM_SWITCH_GET_CNF, "ESIM_SWITCH_GET_CNF", 0, NULL},
    {MIPC_SIM_ESIM_EID_QUERY_REQ, "ESIM_EID_QUERY_REQ", 0, NULL},
    {MIPC_SIM_ESIM_EID_QUERY_CNF, "ESIM_EID_QUERY_CNF", 0, NULL},
    {MIPC_SIM_HOTSWAP_DISABLE_GET_REQ, "HOTSWAP_DISABLE_GET_REQ", 0, NULL},
    {MIPC_SIM_HOTSWAP_DISABLE_GET_CNF, "HOTSWAP_DISABLE_GET_CNF", 0, NULL},
    {MIPC_SIM_HOTSWAP_DISABLE_SET_REQ, "HOTSWAP_DISABLE_SET_REQ", 0, NULL},
    {MIPC_SIM_HOTSWAP_DISABLE_SET_CNF, "HOTSWAP_DISABLE_SET_CNF", 0, NULL},
    {MIPC_SIM_RECOVERY_DISABLE_GET_REQ, "RECOVERY_DISABLE_GET_REQ", 0, NULL},
    {MIPC_SIM_RECOVERY_DISABLE_GET_CNF, "RECOVERY_DISABLE_GET_CNF", 0, NULL},
    {MIPC_SIM_RECOVERY_DISABLE_SET_REQ, "RECOVERY_DISABLE_SET_REQ", 0, NULL},
    {MIPC_SIM_RECOVERY_DISABLE_SET_CNF, "RECOVERY_DISABLE_SET_CNF", 0, NULL},
    {MIPC_SMS_CFG_REQ, "CFG_REQ", 10, mipc_sms_cfg_req_tlv_checker_info},
    {MIPC_SMS_CFG_CNF, "CFG_CNF", 0, NULL},
    {MIPC_SMS_SEND_REQ, "SEND_REQ", 6, mipc_sms_send_req_tlv_checker_info},
    {MIPC_SMS_SEND_CNF, "SEND_CNF", 0, NULL},
    {MIPC_SMS_READ_REQ, "READ_REQ", 4, mipc_sms_read_req_tlv_checker_info},
    {MIPC_SMS_READ_CNF, "READ_CNF", 0, NULL},
    {MIPC_SMS_DELETE_REQ, "DELETE_REQ", 3, mipc_sms_delete_req_tlv_checker_info},
    {MIPC_SMS_DELETE_CNF, "DELETE_CNF", 0, NULL},
    {MIPC_SMS_GET_STORE_STATUS_REQ, "GET_STORE_STATUS_REQ", 2, mipc_sms_get_store_status_req_tlv_checker_info},
    {MIPC_SMS_GET_STORE_STATUS_CNF, "GET_STORE_STATUS_CNF", 0, NULL},
    {MIPC_SMS_WRITE_REQ, "WRITE_REQ", 5, mipc_sms_write_req_tlv_checker_info},
    {MIPC_SMS_WRITE_CNF, "WRITE_CNF", 0, NULL},
    {MIPC_SMS_CBM_CFG_REQ, "CBM_CFG_REQ", 5, mipc_sms_cbm_cfg_req_tlv_checker_info},
    {MIPC_SMS_CBM_CFG_CNF, "CBM_CFG_CNF", 0, NULL},
    {MIPC_SMS_SCBM_REQ, "SCBM_REQ", 1, mipc_sms_scbm_req_tlv_checker_info},
    {MIPC_SMS_SCBM_CNF, "SCBM_CNF", 0, NULL},
    {MIPC_SMS_DOMAIN_REQ, "DOMAIN_REQ", 4, mipc_sms_domain_req_tlv_checker_info},
    {MIPC_SMS_DOMAIN_CNF, "DOMAIN_CNF", 0, NULL},
    {MIPC_SS_SEND_USSD_REQ, "SEND_USSD_REQ", 2, mipc_ss_send_ussd_req_tlv_checker_info},
    {MIPC_SS_SEND_USSD_CNF, "SEND_USSD_CNF", 0, NULL},
    {MIPC_SS_CANCEL_USSD_REQ, "CANCEL_USSD_REQ", 0, NULL},
    {MIPC_SS_CANCEL_USSD_CNF, "CANCEL_USSD_CNF", 0, NULL},
    {MIPC_SS_SET_CLIR_REQ, "SET_CLIR_REQ", 0, NULL},
    {MIPC_SS_SET_CLIR_CNF, "SET_CLIR_CNF", 0, NULL},
    {MIPC_SS_GET_CLIR_REQ, "GET_CLIR_REQ", 0, NULL},
    {MIPC_SS_GET_CLIR_CNF, "GET_CLIR_CNF", 0, NULL},
    {MIPC_SS_SET_CALL_WAITING_REQ, "SET_CALL_WAITING_REQ", 2, mipc_ss_set_call_waiting_req_tlv_checker_info},
    {MIPC_SS_SET_CALL_WAITING_CNF, "SET_CALL_WAITING_CNF", 0, NULL},
    {MIPC_SS_QUERY_CALL_WAITING_REQ, "QUERY_CALL_WAITING_REQ", 1, mipc_ss_query_call_waiting_req_tlv_checker_info},
    {MIPC_SS_QUERY_CALL_WAITING_CNF, "QUERY_CALL_WAITING_CNF", 0, NULL},
    {MIPC_SS_SET_CALL_FORWARD_REQ, "SET_CALL_FORWARD_REQ", 6, mipc_ss_set_call_forward_req_tlv_checker_info},
    {MIPC_SS_SET_CALL_FORWARD_CNF, "SET_CALL_FORWARD_CNF", 0, NULL},
    {MIPC_SS_QUERY_CALL_FORWARD_REQ, "QUERY_CALL_FORWARD_REQ", 2, mipc_ss_query_call_forward_req_tlv_checker_info},
    {MIPC_SS_QUERY_CALL_FORWARD_CNF, "QUERY_CALL_FORWARD_CNF", 0, NULL},
    {MIPC_SS_SET_CALL_BARRING_REQ, "SET_CALL_BARRING_REQ", 4, mipc_ss_set_call_barring_req_tlv_checker_info},
    {MIPC_SS_SET_CALL_BARRING_CNF, "SET_CALL_BARRING_CNF", 0, NULL},
    {MIPC_SS_QUERY_CALL_BARRING_REQ, "QUERY_CALL_BARRING_REQ", 2, mipc_ss_query_call_barring_req_tlv_checker_info},
    {MIPC_SS_QUERY_CALL_BARRING_CNF, "QUERY_CALL_BARRING_CNF", 0, NULL},
    {MIPC_SS_CHANGE_BARRING_PASSWORD_REQ, "CHANGE_BARRING_PASSWORD_REQ", 4, mipc_ss_change_barring_password_req_tlv_checker_info},
    {MIPC_SS_CHANGE_BARRING_PASSWORD_CNF, "CHANGE_BARRING_PASSWORD_CNF", 0, NULL},
    {MIPC_SS_SET_SUPP_SVC_NOTIFICATION_REQ, "SET_SUPP_SVC_NOTIFICATION_REQ", 0, NULL},
    {MIPC_SS_SET_SUPP_SVC_NOTIFICATION_CNF, "SET_SUPP_SVC_NOTIFICATION_CNF", 0, NULL},
    {MIPC_SS_QUERY_CLIP_REQ, "QUERY_CLIP_REQ", 0, NULL},
    {MIPC_SS_QUERY_CLIP_CNF, "QUERY_CLIP_CNF", 0, NULL},
    {MIPC_SS_SET_CLIP_REQ, "SET_CLIP_REQ", 0, NULL},
    {MIPC_SS_SET_CLIP_CNF, "SET_CLIP_CNF", 0, NULL},
    {MIPC_SS_RUN_GBA_REQ, "RUN_GBA_REQ", 3, mipc_ss_run_gba_req_tlv_checker_info},
    {MIPC_SS_RUN_GBA_CNF, "RUN_GBA_CNF", 0, NULL},
    {MIPC_SS_GET_COLP_REQ, "GET_COLP_REQ", 0, NULL},
    {MIPC_SS_GET_COLP_CNF, "GET_COLP_CNF", 0, NULL},
    {MIPC_SS_SET_COLP_REQ, "SET_COLP_REQ", 0, NULL},
    {MIPC_SS_SET_COLP_CNF, "SET_COLP_CNF", 0, NULL},
    {MIPC_SS_GET_COLR_REQ, "GET_COLR_REQ", 0, NULL},
    {MIPC_SS_GET_COLR_CNF, "GET_COLR_CNF", 0, NULL},
    {MIPC_SS_SEND_CNAP_REQ, "SEND_CNAP_REQ", 0, NULL},
    {MIPC_SS_SEND_CNAP_CNF, "SEND_CNAP_CNF", 0, NULL},
    {MIPC_SS_SET_COLR_REQ, "SET_COLR_REQ", 0, NULL},
    {MIPC_SS_SET_COLR_CNF, "SET_COLR_CNF", 0, NULL},
    {MIPC_SS_SETUP_XCAP_USER_AGENT_REQ, "SETUP_XCAP_USER_AGENT_REQ", 1, mipc_ss_setup_xcap_user_agent_req_tlv_checker_info},
    {MIPC_SS_SETUP_XCAP_USER_AGENT_CNF, "SETUP_XCAP_USER_AGENT_CNF", 0, NULL},
    {MIPC_SS_SET_XCAP_CFG_REQ, "SET_XCAP_CFG_REQ", 2, mipc_ss_set_xcap_cfg_req_tlv_checker_info},
    {MIPC_SS_SET_XCAP_CFG_CNF, "SET_XCAP_CFG_CNF", 0, NULL},
    {MIPC_STK_SET_PAC_REQ, "SET_PAC_REQ", 1, mipc_stk_set_pac_req_tlv_checker_info},
    {MIPC_STK_SET_PAC_CNF, "SET_PAC_CNF", 0, NULL},
    {MIPC_STK_GET_PAC_REQ, "GET_PAC_REQ", 0, NULL},
    {MIPC_STK_GET_PAC_CNF, "GET_PAC_CNF", 0, NULL},
    {MIPC_STK_SEND_TERMINAL_RESPONSE_REQ, "SEND_TERMINAL_RESPONSE_REQ", 2, mipc_stk_send_terminal_response_req_tlv_checker_info},
    {MIPC_STK_SEND_TERMINAL_RESPONSE_CNF, "SEND_TERMINAL_RESPONSE_CNF", 0, NULL},
    {MIPC_STK_SEND_ENVELOPE_REQ, "SEND_ENVELOPE_REQ", 1, mipc_stk_send_envelope_req_tlv_checker_info},
    {MIPC_STK_SEND_ENVELOPE_CNF, "SEND_ENVELOPE_CNF", 0, NULL},
    {MIPC_STK_GET_ENVELOPE_INFO_REQ, "GET_ENVELOPE_INFO_REQ", 0, NULL},
    {MIPC_STK_GET_ENVELOPE_INFO_CNF, "GET_ENVELOPE_INFO_CNF", 0, NULL},
    {MIPC_STK_HANDLE_CALL_SETUP_FROM_SIM_REQ, "HANDLE_CALL_SETUP_FROM_SIM_REQ", 0, NULL},
    {MIPC_STK_HANDLE_CALL_SETUP_FROM_SIM_CNF, "HANDLE_CALL_SETUP_FROM_SIM_CNF", 0, NULL},
    {MIPC_STK_SEND_BIPCONF_REQ, "SEND_BIPCONF_REQ", 0, NULL},
    {MIPC_STK_SEND_BIPCONF_CNF, "SEND_BIPCONF_CNF", 0, NULL},
    {MIPC_CALL_DIAL_REQ, "DIAL_REQ", 8, mipc_call_dial_req_tlv_checker_info},
    {MIPC_CALL_DIAL_CNF, "DIAL_CNF", 0, NULL},
    {MIPC_CALL_SS_REQ, "SS_REQ", 3, mipc_call_ss_req_tlv_checker_info},
    {MIPC_CALL_SS_CNF, "SS_CNF", 0, NULL},
    {MIPC_CALL_HANGUP_REQ, "HANGUP_REQ", 2, mipc_call_hangup_req_tlv_checker_info},
    {MIPC_CALL_HANGUP_CNF, "HANGUP_CNF", 0, NULL},
    {MIPC_CALL_ANSWER_REQ, "ANSWER_REQ", 1, mipc_call_answer_req_tlv_checker_info},
    {MIPC_CALL_ANSWER_CNF, "ANSWER_CNF", 0, NULL},
    {MIPC_CALL_GET_CALL_STATUS_REQ, "GET_CALL_STATUS_REQ", 0, NULL},
    {MIPC_CALL_GET_CALL_STATUS_CNF, "GET_CALL_STATUS_CNF", 0, NULL},
    {MIPC_CALL_CONFERENCE_REQ, "CONFERENCE_REQ", 2, mipc_call_conference_req_tlv_checker_info},
    {MIPC_CALL_CONFERENCE_CNF, "CONFERENCE_CNF", 0, NULL},
    {MIPC_CALL_GET_CONFERENCE_INFO_REQ, "GET_CONFERENCE_INFO_REQ", 0, NULL},
    {MIPC_CALL_GET_CONFERENCE_INFO_CNF, "GET_CONFERENCE_INFO_CNF", 0, NULL},
    {MIPC_CALL_GET_FINISH_REASON_REQ, "GET_FINISH_REASON_REQ", 0, NULL},
    {MIPC_CALL_GET_FINISH_REASON_CNF, "GET_FINISH_REASON_CNF", 0, NULL},
    {MIPC_CALL_DTMF_REQ, "DTMF_REQ", 2, mipc_call_dtmf_req_tlv_checker_info},
    {MIPC_CALL_DTMF_CNF, "DTMF_CNF", 0, NULL},
    {MIPC_CALL_GET_ECC_LIST_REQ, "GET_ECC_LIST_REQ", 0, NULL},
    {MIPC_CALL_GET_ECC_LIST_CNF, "GET_ECC_LIST_CNF", 0, NULL},
    {MIPC_CALL_SET_ECC_LIST_REQ, "SET_ECC_LIST_REQ", 1, mipc_call_set_ecc_list_req_tlv_checker_info},
    {MIPC_CALL_SET_ECC_LIST_CNF, "SET_ECC_LIST_CNF", 0, NULL},
    {MIPC_CALL_SET_FLIGHT_MODE_ECC_SESSION_REQ, "SET_FLIGHT_MODE_ECC_SESSION_REQ", 0, NULL},
    {MIPC_CALL_SET_FLIGHT_MODE_ECC_SESSION_CNF, "SET_FLIGHT_MODE_ECC_SESSION_CNF", 0, NULL},
    {MIPC_CALL_SET_EXIT_ECBM_MODE_REQ, "SET_EXIT_ECBM_MODE_REQ", 0, NULL},
    {MIPC_CALL_SET_EXIT_ECBM_MODE_CNF, "SET_EXIT_ECBM_MODE_CNF", 0, NULL},
    {MIPC_CALL_IVS_ONEKEY_ECALL_REQ, "IVS_ONEKEY_ECALL_REQ", 4, mipc_call_ivs_onekey_ecall_req_tlv_checker_info},
    {MIPC_CALL_IVS_ONEKEY_ECALL_CNF, "IVS_ONEKEY_ECALL_CNF", 0, NULL},
    {MIPC_CALL_SET_SIP_HEADER_REQ, "SET_SIP_HEADER_REQ", 1, mipc_call_set_sip_header_req_tlv_checker_info},
    {MIPC_CALL_SET_SIP_HEADER_CNF, "SET_SIP_HEADER_CNF", 0, NULL},
    {MIPC_CALL_ENABLE_IMS_SIP_HEADER_REPORT_REQ, "ENABLE_IMS_SIP_HEADER_REPORT_REQ", 0, NULL},
    {MIPC_CALL_ENABLE_IMS_SIP_HEADER_REPORT_CNF, "ENABLE_IMS_SIP_HEADER_REPORT_CNF", 0, NULL},
    {MIPC_CALL_SET_CALL_ADDITIONAL_INFO_REQ, "SET_CALL_ADDITIONAL_INFO_REQ", 3, mipc_call_set_call_additional_info_req_tlv_checker_info},
    {MIPC_CALL_SET_CALL_ADDITIONAL_INFO_CNF, "SET_CALL_ADDITIONAL_INFO_CNF", 0, NULL},
    {MIPC_CALL_PEER_RTT_MODIFY_REQ, "PEER_RTT_MODIFY_REQ", 1, mipc_call_peer_rtt_modify_req_tlv_checker_info},
    {MIPC_CALL_PEER_RTT_MODIFY_CNF, "PEER_RTT_MODIFY_CNF", 0, NULL},
    {MIPC_CALL_LOCAL_RTT_MODIFY_REQ, "LOCAL_RTT_MODIFY_REQ", 1, mipc_call_local_rtt_modify_req_tlv_checker_info},
    {MIPC_CALL_LOCAL_RTT_MODIFY_CNF, "LOCAL_RTT_MODIFY_CNF", 0, NULL},
    {MIPC_CALL_RTT_TEXT_REQ, "RTT_TEXT_REQ", 1, mipc_call_rtt_text_req_tlv_checker_info},
    {MIPC_CALL_RTT_TEXT_CNF, "RTT_TEXT_CNF", 0, NULL},
    {MIPC_CALL_RTT_MODE_REQ, "RTT_MODE_REQ", 1, mipc_call_rtt_mode_req_tlv_checker_info},
    {MIPC_CALL_RTT_MODE_CNF, "RTT_MODE_CNF", 0, NULL},
    {MIPC_CALL_RTT_AUDIO_REQ, "RTT_AUDIO_REQ", 0, NULL},
    {MIPC_CALL_RTT_AUDIO_CNF, "RTT_AUDIO_CNF", 0, NULL},
    {MIPC_CALL_SET_RCS_STATE_AND_FEATURE_REQ, "SET_RCS_STATE_AND_FEATURE_REQ", 2, mipc_call_set_rcs_state_and_feature_req_tlv_checker_info},
    {MIPC_CALL_SET_RCS_STATE_AND_FEATURE_CNF, "SET_RCS_STATE_AND_FEATURE_CNF", 0, NULL},
    {MIPC_CALL_UPDATE_RCS_SESSION_INFO_REQ, "UPDATE_RCS_SESSION_INFO_REQ", 0, NULL},
    {MIPC_CALL_UPDATE_RCS_SESSION_INFO_CNF, "UPDATE_RCS_SESSION_INFO_CNF", 0, NULL},
    {MIPC_CALL_GET_VOICE_DOMAIN_PREFERENCE_REQ, "GET_VOICE_DOMAIN_PREFERENCE_REQ", 0, NULL},
    {MIPC_CALL_GET_VOICE_DOMAIN_PREFERENCE_CNF, "GET_VOICE_DOMAIN_PREFERENCE_CNF", 0, NULL},
    {MIPC_CALL_PULL_REQ, "PULL_REQ", 2, mipc_call_pull_req_tlv_checker_info},
    {MIPC_CALL_PULL_CNF, "PULL_CNF", 0, NULL},
    {MIPC_CALL_GET_TTY_MODE_REQ, "GET_TTY_MODE_REQ", 0, NULL},
    {MIPC_CALL_GET_TTY_MODE_CNF, "GET_TTY_MODE_CNF", 0, NULL},
    {MIPC_CALL_SET_IMS_CALL_MODE_REQ, "SET_IMS_CALL_MODE_REQ", 1, mipc_call_set_ims_call_mode_req_tlv_checker_info},
    {MIPC_CALL_SET_IMS_CALL_MODE_CNF, "SET_IMS_CALL_MODE_CNF", 0, NULL},
    {MIPC_CALL_SET_TTY_MODE_REQ, "SET_TTY_MODE_REQ", 1, mipc_call_set_tty_mode_req_tlv_checker_info},
    {MIPC_CALL_SET_TTY_MODE_CNF, "SET_TTY_MODE_CNF", 0, NULL},
    {MIPC_CALL_SET_VOICE_DOMAIN_PREFERENCE_REQ, "SET_VOICE_DOMAIN_PREFERENCE_REQ", 1, mipc_call_set_voice_domain_preference_req_tlv_checker_info},
    {MIPC_CALL_SET_VOICE_DOMAIN_PREFERENCE_CNF, "SET_VOICE_DOMAIN_PREFERENCE_CNF", 0, NULL},
    {MIPC_CALL_CONFERENCE_DIAL_REQ, "CONFERENCE_DIAL_REQ", 4, mipc_call_conference_dial_req_tlv_checker_info},
    {MIPC_CALL_CONFERENCE_DIAL_CNF, "CONFERENCE_DIAL_CNF", 0, NULL},
    {MIPC_CALL_SET_GWSD_MODE_REQ, "SET_GWSD_MODE_REQ", 2, mipc_call_set_gwsd_mode_req_tlv_checker_info},
    {MIPC_CALL_SET_GWSD_MODE_CNF, "SET_GWSD_MODE_CNF", 0, NULL},
    {MIPC_CALL_APPROVE_CSFB_REQ, "APPROVE_CSFB_REQ", 1, mipc_call_approve_csfb_req_tlv_checker_info},
    {MIPC_CALL_APPROVE_CSFB_CNF, "APPROVE_CSFB_CNF", 0, NULL},
    {MIPC_CALL_SET_GWSD_CALL_VALID_REQ, "SET_GWSD_CALL_VALID_REQ", 0, NULL},
    {MIPC_CALL_SET_GWSD_CALL_VALID_CNF, "SET_GWSD_CALL_VALID_CNF", 0, NULL},
    {MIPC_CALL_SET_GWSD_IGNORE_CALL_INTERVAL_REQ, "SET_GWSD_IGNORE_CALL_INTERVAL_REQ", 0, NULL},
    {MIPC_CALL_SET_GWSD_IGNORE_CALL_INTERVAL_CNF, "SET_GWSD_IGNORE_CALL_INTERVAL_CNF", 0, NULL},
    {MIPC_CALL_SET_GWSD_KA_PDCP_REQ, "SET_GWSD_KA_PDCP_REQ", 1, mipc_call_set_gwsd_ka_pdcp_req_tlv_checker_info},
    {MIPC_CALL_SET_GWSD_KA_PDCP_CNF, "SET_GWSD_KA_PDCP_CNF", 0, NULL},
    {MIPC_CALL_SET_GWSD_KA_IPDATA_REQ, "SET_GWSD_KA_IPDATA_REQ", 1, mipc_call_set_gwsd_ka_ipdata_req_tlv_checker_info},
    {MIPC_CALL_SET_GWSD_KA_IPDATA_CNF, "SET_GWSD_KA_IPDATA_CNF", 0, NULL},
    {MIPC_CALL_UIS_INFO_REQ, "UIS_INFO_REQ", 2, mipc_call_uis_info_req_tlv_checker_info},
    {MIPC_CALL_UIS_INFO_CNF, "UIS_INFO_CNF", 0, NULL},
    {MIPC_CALL_AUTO_ANSWER_REQ, "AUTO_ANSWER_REQ", 0, NULL},
    {MIPC_CALL_AUTO_ANSWER_CNF, "AUTO_ANSWER_CNF", 0, NULL},
    {MIPC_CALL_DATA_PREFER_SET_REQ, "DATA_PREFER_SET_REQ", 2, mipc_call_data_prefer_set_req_tlv_checker_info},
    {MIPC_CALL_DATA_PREFER_SET_CNF, "DATA_PREFER_SET_CNF", 0, NULL},
    {MIPC_CALL_ECC_REDIAL_APPROVE_REQ, "ECC_REDIAL_APPROVE_REQ", 0, NULL},
    {MIPC_CALL_ECC_REDIAL_APPROVE_CNF, "ECC_REDIAL_APPROVE_CNF", 0, NULL},
    {MIPC_CALL_DC_CREATE_SIP_REQ, "DC_CREATE_SIP_REQ", 9, mipc_call_dc_create_sip_req_tlv_checker_info},
    {MIPC_CALL_DC_CREATE_SIP_CNF, "DC_CREATE_SIP_CNF", 0, NULL},
    {MIPC_CALL_DC_MODIFY_SIP_REQ, "DC_MODIFY_SIP_REQ", 9, mipc_call_dc_modify_sip_req_tlv_checker_info},
    {MIPC_CALL_DC_MODIFY_SIP_CNF, "DC_MODIFY_SIP_CNF", 0, NULL},
    {MIPC_CALL_DC_CLOSE_SIP_REQ, "DC_CLOSE_SIP_REQ", 0, NULL},
    {MIPC_CALL_DC_CLOSE_SIP_CNF, "DC_CLOSE_SIP_CNF", 0, NULL},
    {MIPC_IMS_SET_CONFIG_REQ, "SET_CONFIG_REQ", 3, mipc_ims_set_config_req_tlv_checker_info},
    {MIPC_IMS_SET_CONFIG_CNF, "SET_CONFIG_CNF", 0, NULL},
    {MIPC_IMS_GET_CONFIG_REQ, "GET_CONFIG_REQ", 2, mipc_ims_get_config_req_tlv_checker_info},
    {MIPC_IMS_GET_CONFIG_CNF, "GET_CONFIG_CNF", 0, NULL},
    {MIPC_IMS_GET_STATE_REQ, "GET_STATE_REQ", 1, mipc_ims_get_state_req_tlv_checker_info},
    {MIPC_IMS_GET_STATE_CNF, "GET_STATE_CNF", 0, NULL},
    {MIPC_IMS_SET_PDIS_REQ, "SET_PDIS_REQ", 1, mipc_ims_set_pdis_req_tlv_checker_info},
    {MIPC_IMS_SET_PDIS_CNF, "SET_PDIS_CNF", 0, NULL},
    {MIPC_IMS_SET_NAPTR_REQ, "SET_NAPTR_REQ", 5, mipc_ims_set_naptr_req_tlv_checker_info},
    {MIPC_IMS_SET_NAPTR_CNF, "SET_NAPTR_CNF", 0, NULL},
    {MIPC_IMS_GET_NW_RPT_REQ, "GET_NW_RPT_REQ", 0, NULL},
    {MIPC_IMS_GET_NW_RPT_CNF, "GET_NW_RPT_CNF", 0, NULL},
    {MIPC_IMS_SET_TEST_MODE_REQ, "SET_TEST_MODE_REQ", 0, NULL},
    {MIPC_IMS_SET_TEST_MODE_CNF, "SET_TEST_MODE_CNF", 0, NULL},
    {MIPC_IMS_SET_EIREG_REQ, "SET_EIREG_REQ", 6, mipc_ims_set_eireg_req_tlv_checker_info},
    {MIPC_IMS_SET_EIREG_CNF, "SET_EIREG_CNF", 0, NULL},
    {MIPC_IMS_SET_SCM_REQ, "SET_SCM_REQ", 2, mipc_ims_set_scm_req_tlv_checker_info},
    {MIPC_IMS_SET_SCM_CNF, "SET_SCM_CNF", 0, NULL},
    {MIPC_IMS_SET_SERVICE_SESSION_REQ, "SET_SERVICE_SESSION_REQ", 2, mipc_ims_set_service_session_req_tlv_checker_info},
    {MIPC_IMS_SET_SERVICE_SESSION_CNF, "SET_SERVICE_SESSION_CNF", 0, NULL},
    {MIPC_IMS_SET_UAC_REQ, "SET_UAC_REQ", 2, mipc_ims_set_uac_req_tlv_checker_info},
    {MIPC_IMS_SET_UAC_CNF, "SET_UAC_CNF", 0, NULL},
    {MIPC_IMS_SET_EVODATA_REQ, "SET_EVODATA_REQ", 3, mipc_ims_set_evodata_req_tlv_checker_info},
    {MIPC_IMS_SET_EVODATA_CNF, "SET_EVODATA_CNF", 0, NULL},
    {MIPC_IMS_ENABLE_DATA_CHANNEL_REQ, "ENABLE_DATA_CHANNEL_REQ", 1, mipc_ims_enable_data_channel_req_tlv_checker_info},
    {MIPC_IMS_ENABLE_DATA_CHANNEL_CNF, "ENABLE_DATA_CHANNEL_CNF", 0, NULL},
    {MIPC_IMS_GET_PDN_REQ, "GET_PDN_REQ", 0, NULL},
    {MIPC_IMS_GET_PDN_CNF, "GET_PDN_CNF", 0, NULL},
    {MIPC_IMS_CFG_ADDITION_SERVICE_REQ, "CFG_ADDITION_SERVICE_REQ", 1, mipc_ims_cfg_addition_service_req_tlv_checker_info},
    {MIPC_IMS_CFG_ADDITION_SERVICE_CNF, "CFG_ADDITION_SERVICE_CNF", 0, NULL},
    {MIPC_WFC_REGISTER_CELL_SIGNAL_IND_REQ, "REGISTER_CELL_SIGNAL_IND_REQ", 2, mipc_wfc_register_cell_signal_ind_req_tlv_checker_info},
    {MIPC_WFC_REGISTER_CELL_SIGNAL_IND_CNF, "REGISTER_CELL_SIGNAL_IND_CNF", 0, NULL},
    {MIPC_WFC_CSSAC_REQ, "CSSAC_REQ", 0, NULL},
    {MIPC_WFC_CSSAC_CNF, "CSSAC_CNF", 0, NULL},
    {MIPC_WFC_SET_EMC_AID_REQ, "SET_EMC_AID_REQ", 1, mipc_wfc_set_emc_aid_req_tlv_checker_info},
    {MIPC_WFC_SET_EMC_AID_CNF, "SET_EMC_AID_CNF", 0, NULL},
    {MIPC_WFC_CFG_REQ, "CFG_REQ", 2, mipc_wfc_cfg_req_tlv_checker_info},
    {MIPC_WFC_CFG_CNF, "CFG_CNF", 0, NULL},
    {MIPC_WFC_SET_EAP_VERSION_REQ, "SET_EAP_VERSION_REQ", 0, NULL},
    {MIPC_WFC_SET_EAP_VERSION_CNF, "SET_EAP_VERSION_CNF", 0, NULL},
    {MIPC_PHB_SET_UPB_ENTRY_REQ, "SET_UPB_ENTRY_REQ", 3, mipc_phb_set_upb_entry_req_tlv_checker_info},
    {MIPC_PHB_SET_UPB_ENTRY_CNF, "SET_UPB_ENTRY_CNF", 0, NULL},
    {MIPC_PHB_GET_UPB_ANR_EMAIL_SNE_ENTRY_REQ, "GET_UPB_ANR_EMAIL_SNE_ENTRY_REQ", 1, mipc_phb_get_upb_anr_email_sne_entry_req_tlv_checker_info},
    {MIPC_PHB_GET_UPB_ANR_EMAIL_SNE_ENTRY_CNF, "GET_UPB_ANR_EMAIL_SNE_ENTRY_CNF", 0, NULL},
    {MIPC_PHB_GET_UPB_AAS_GAS_GRP_LIST_ENTRY_REQ, "GET_UPB_AAS_GAS_GRP_LIST_ENTRY_REQ", 1, mipc_phb_get_upb_aas_gas_grp_list_entry_req_tlv_checker_info},
    {MIPC_PHB_GET_UPB_AAS_GAS_GRP_LIST_ENTRY_CNF, "GET_UPB_AAS_GAS_GRP_LIST_ENTRY_CNF", 0, NULL},
    {MIPC_PHB_GET_PHB_STORAGE_INFO_REQ, "GET_PHB_STORAGE_INFO_REQ", 1, mipc_phb_get_phb_storage_info_req_tlv_checker_info},
    {MIPC_PHB_GET_PHB_STORAGE_INFO_CNF, "GET_PHB_STORAGE_INFO_CNF", 0, NULL},
    {MIPC_PHB_SET_PHB_MEM_STORAGE_REQ, "SET_PHB_MEM_STORAGE_REQ", 2, mipc_phb_set_phb_mem_storage_req_tlv_checker_info},
    {MIPC_PHB_SET_PHB_MEM_STORAGE_CNF, "SET_PHB_MEM_STORAGE_CNF", 0, NULL},
    {MIPC_PHB_GET_PHB_ENTRY_REQ, "GET_PHB_ENTRY_REQ", 1, mipc_phb_get_phb_entry_req_tlv_checker_info},
    {MIPC_PHB_GET_PHB_ENTRY_CNF, "GET_PHB_ENTRY_CNF", 0, NULL},
    {MIPC_PHB_SET_PHB_ENTRY_REQ, "SET_PHB_ENTRY_REQ", 2, mipc_phb_set_phb_entry_req_tlv_checker_info},
    {MIPC_PHB_SET_PHB_ENTRY_CNF, "SET_PHB_ENTRY_CNF", 0, NULL},
    {MIPC_PHB_GET_PHB_STRINGSLENGTH_REQ, "GET_PHB_STRINGSLENGTH_REQ", 0, NULL},
    {MIPC_PHB_GET_PHB_STRINGSLENGTH_CNF, "GET_PHB_STRINGSLENGTH_CNF", 0, NULL},
    {MIPC_PHB_GET_UPB_CAPABILITY_REQ, "GET_UPB_CAPABILITY_REQ", 0, NULL},
    {MIPC_PHB_GET_UPB_CAPABILITY_CNF, "GET_UPB_CAPABILITY_CNF", 0, NULL},
    {MIPC_PHB_GET_PHB_AVAILABLE_REQ, "GET_PHB_AVAILABLE_REQ", 1, mipc_phb_get_phb_available_req_tlv_checker_info},
    {MIPC_PHB_GET_PHB_AVAILABLE_CNF, "GET_PHB_AVAILABLE_CNF", 0, NULL},
    {MIPC_EMBMS_EMSLU_REQ, "EMSLU_REQ", 0, NULL},
    {MIPC_EMBMS_EMSLU_CNF, "EMSLU_CNF", 0, NULL},
    {MIPC_EMBMS_GET_SAI_LIST_REQ, "GET_SAI_LIST_REQ", 0, NULL},
    {MIPC_EMBMS_GET_SAI_LIST_CNF, "GET_SAI_LIST_CNF", 0, NULL},
    {MIPC_EMBMS_NOTIFY_HVOLTE_STATUS_REQ, "NOTIFY_HVOLTE_STATUS_REQ", 0, NULL},
    {MIPC_EMBMS_NOTIFY_HVOLTE_STATUS_CNF, "NOTIFY_HVOLTE_STATUS_CNF", 0, NULL},
    {MIPC_ECALL_IVS_UPDATE_MSD_REQ, "IVS_UPDATE_MSD_REQ", 2, mipc_ecall_ivs_update_msd_req_tlv_checker_info},
    {MIPC_ECALL_IVS_UPDATE_MSD_CNF, "IVS_UPDATE_MSD_CNF", 0, NULL},
    {MIPC_ECALL_IVS_RESET_REQ, "IVS_RESET_REQ", 0, NULL},
    {MIPC_ECALL_IVS_RESET_CNF, "IVS_RESET_CNF", 0, NULL},
    {MIPC_ECALL_IVS_SET_TEST_ADDR_REQ, "IVS_SET_TEST_ADDR_REQ", 2, mipc_ecall_ivs_set_test_addr_req_tlv_checker_info},
    {MIPC_ECALL_IVS_SET_TEST_ADDR_CNF, "IVS_SET_TEST_ADDR_CNF", 0, NULL},
    {MIPC_ECALL_IVS_SET_RECONF_ADDR_REQ, "IVS_SET_RECONF_ADDR_REQ", 2, mipc_ecall_ivs_set_reconf_addr_req_tlv_checker_info},
    {MIPC_ECALL_IVS_SET_RECONF_ADDR_CNF, "IVS_SET_RECONF_ADDR_CNF", 0, NULL},
    {MIPC_ECALL_IVS_SET_ADDR_PRI_REQ, "IVS_SET_ADDR_PRI_REQ", 4, mipc_ecall_ivs_set_addr_pri_req_tlv_checker_info},
    {MIPC_ECALL_IVS_SET_ADDR_PRI_CNF, "IVS_SET_ADDR_PRI_CNF", 0, NULL},
    {MIPC_ECALL_IVS_GET_SIM_INFO_REQ, "IVS_GET_SIM_INFO_REQ", 0, NULL},
    {MIPC_ECALL_IVS_GET_SIM_INFO_CNF, "IVS_GET_SIM_INFO_CNF", 0, NULL},
    {MIPC_ECALL_IVS_CONFIG_TIMER_REQ, "IVS_CONFIG_TIMER_REQ", 0, NULL},
    {MIPC_ECALL_IVS_CONFIG_TIMER_CNF, "IVS_CONFIG_TIMER_CNF", 0, NULL},
    {MIPC_ECALL_IVS_SET_ECALL_MODE_REQ, "IVS_SET_ECALL_MODE_REQ", 1, mipc_ecall_ivs_set_ecall_mode_req_tlv_checker_info},
    {MIPC_ECALL_IVS_SET_ECALL_MODE_CNF, "IVS_SET_ECALL_MODE_CNF", 0, NULL},
    {MIPC_ECALL_IVS_GET_ECALL_MODE_REQ, "IVS_GET_ECALL_MODE_REQ", 0, NULL},
    {MIPC_ECALL_IVS_GET_ECALL_MODE_CNF, "IVS_GET_ECALL_MODE_CNF", 0, NULL},
    {MIPC_ECALL_IVS_SET_ECALL_CATEGORY_REQ, "IVS_SET_ECALL_CATEGORY_REQ", 1, mipc_ecall_ivs_set_ecall_category_req_tlv_checker_info},
    {MIPC_ECALL_IVS_SET_ECALL_CATEGORY_CNF, "IVS_SET_ECALL_CATEGORY_CNF", 0, NULL},
    {MIPC_ECALL_IVS_GET_ECALL_CATEGORY_REQ, "IVS_GET_ECALL_CATEGORY_REQ", 0, NULL},
    {MIPC_ECALL_IVS_GET_ECALL_CATEGORY_CNF, "IVS_GET_ECALL_CATEGORY_CNF", 0, NULL},
    {MIPC_NW_GET_CURRENT_CELL_INFO_REQ, "GET_CURRENT_CELL_INFO_REQ", 0, NULL},
    {MIPC_NW_GET_CURRENT_CELL_INFO_CNF, "GET_CURRENT_CELL_INFO_CNF", 0, NULL},
    {MIPC_NW_CONGESTION_CFG_REQ, "CONGESTION_CFG_REQ", 2, mipc_nw_congestion_cfg_req_tlv_checker_info},
    {MIPC_NW_CONGESTION_CFG_CNF, "CONGESTION_CFG_CNF", 0, NULL},
    {MIPC_NW_GET_ENWCFGINFO_REQ, "GET_ENWCFGINFO_REQ", 0, NULL},
    {MIPC_NW_GET_ENWCFGINFO_CNF, "GET_ENWCFGINFO_CNF", 0, NULL},
    {MIPC_NW_SET_BARRING_RLF_CONFIG_REQ, "SET_BARRING_RLF_CONFIG_REQ", 0, NULL},
    {MIPC_NW_SET_BARRING_RLF_CONFIG_CNF, "SET_BARRING_RLF_CONFIG_CNF", 0, NULL},
    {MIPC_NW_SET_RX_MIMO_CAP_REQ, "SET_RX_MIMO_CAP_REQ", 2, mipc_nw_set_rx_mimo_cap_req_tlv_checker_info},
    {MIPC_NW_SET_RX_MIMO_CAP_CNF, "SET_RX_MIMO_CAP_CNF", 0, NULL},
    {MIPC_NW_SET_MR_THRESH_BOUND_REQ, "SET_MR_THRESH_BOUND_REQ", 1, mipc_nw_set_mr_thresh_bound_req_tlv_checker_info},
    {MIPC_NW_SET_MR_THRESH_BOUND_CNF, "SET_MR_THRESH_BOUND_CNF", 0, NULL},
    {MIPC_NW_DELETE_FTAI_ENTRY_REQ, "DELETE_FTAI_ENTRY_REQ", 2, mipc_nw_delete_ftai_entry_req_tlv_checker_info},
    {MIPC_NW_DELETE_FTAI_ENTRY_CNF, "DELETE_FTAI_ENTRY_CNF", 0, NULL},
    {MIPC_NW_SET_NO_SERVICE_TIMER_REQ, "SET_NO_SERVICE_TIMER_REQ", 0, NULL},
    {MIPC_NW_SET_NO_SERVICE_TIMER_CNF, "SET_NO_SERVICE_TIMER_CNF", 0, NULL},
    {MIPC_NW_GET_NO_SERVICE_TIMER_REQ, "GET_NO_SERVICE_TIMER_REQ", 0, NULL},
    {MIPC_NW_GET_NO_SERVICE_TIMER_CNF, "GET_NO_SERVICE_TIMER_CNF", 0, NULL},
    {MIPC_NW_SERVING_MEASUREMNT_CONTROL_CONFIG_REQ, "SERVING_MEASUREMNT_CONTROL_CONFIG_REQ", 1, mipc_nw_serving_measuremnt_control_config_req_tlv_checker_info},
    {MIPC_NW_SERVING_MEASUREMNT_CONTROL_CONFIG_CNF, "SERVING_MEASUREMNT_CONTROL_CONFIG_CNF", 0, NULL},
    {MIPC_NW_CLEAR_N1_S1_DISABLED_STATUS_REQ, "CLEAR_N1_S1_DISABLED_STATUS_REQ", 3, mipc_nw_clear_n1_s1_disabled_status_req_tlv_checker_info},
    {MIPC_NW_CLEAR_N1_S1_DISABLED_STATUS_CNF, "CLEAR_N1_S1_DISABLED_STATUS_CNF", 0, NULL},
    {MIPC_NW_SET_DYNAMIC_ANT_BIAS_REQ, "SET_DYNAMIC_ANT_BIAS_REQ", 0, NULL},
    {MIPC_NW_SET_DYNAMIC_ANT_BIAS_CNF, "SET_DYNAMIC_ANT_BIAS_CNF", 0, NULL},
    {MIPC_NW_SET_ABSOLUTE_MAX_TX_POWER_REQ, "SET_ABSOLUTE_MAX_TX_POWER_REQ", 1, mipc_nw_set_absolute_max_tx_power_req_tlv_checker_info},
    {MIPC_NW_SET_ABSOLUTE_MAX_TX_POWER_CNF, "SET_ABSOLUTE_MAX_TX_POWER_CNF", 0, NULL},
    {MIPC_NW_GET_CSCON_STATE_REQ, "GET_CSCON_STATE_REQ", 0, NULL},
    {MIPC_NW_GET_CSCON_STATE_CNF, "GET_CSCON_STATE_CNF", 0, NULL},
    {MIPC_NW_SET_CPE_MODE_UPDATE_REQ, "SET_CPE_MODE_UPDATE_REQ", 1, mipc_nw_set_cpe_mode_update_req_tlv_checker_info},
    {MIPC_NW_SET_CPE_MODE_UPDATE_CNF, "SET_CPE_MODE_UPDATE_CNF", 0, NULL},
    {MIPC_NW_SET_MAX_EIRP_REQ, "SET_MAX_EIRP_REQ", 1, mipc_nw_set_max_eirp_req_tlv_checker_info},
    {MIPC_NW_SET_MAX_EIRP_CNF, "SET_MAX_EIRP_CNF", 0, NULL},
    {MIPC_NW_CLEAR_NSSAI_REQ, "CLEAR_NSSAI_REQ", 1, mipc_nw_clear_nssai_req_tlv_checker_info},
    {MIPC_NW_CLEAR_NSSAI_CNF, "CLEAR_NSSAI_CNF", 0, NULL},
    {MIPC_NW_LOCAL_REL_WITH_REDIR_TO_LTE_REQ, "LOCAL_REL_WITH_REDIR_TO_LTE_REQ", 0, NULL},
    {MIPC_NW_LOCAL_REL_WITH_REDIR_TO_LTE_CNF, "LOCAL_REL_WITH_REDIR_TO_LTE_CNF", 0, NULL},
    {MIPC_NW_SET_TIME_SYNC_PPS_REQ, "SET_TIME_SYNC_PPS_REQ", 1, mipc_nw_set_time_sync_pps_req_tlv_checker_info},
    {MIPC_NW_SET_TIME_SYNC_PPS_CNF, "SET_TIME_SYNC_PPS_CNF", 0, NULL},
    {MIPC_NW_SET_VEHICLE_STATUS_REQ, "SET_VEHICLE_STATUS_REQ", 1, mipc_nw_set_vehicle_status_req_tlv_checker_info},
    {MIPC_NW_SET_VEHICLE_STATUS_CNF, "SET_VEHICLE_STATUS_CNF", 0, NULL},
    {MIPC_SYS_GET_SLEEP_MODE_REQ, "GET_SLEEP_MODE_REQ", 0, NULL},
    {MIPC_SYS_GET_SLEEP_MODE_CNF, "GET_SLEEP_MODE_CNF", 0, NULL},
    {MIPC_SYS_AT_IND, "AT_IND", 0, NULL},
    {MIPC_SYS_THERMAL_SENSOR_IND, "THERMAL_SENSOR_IND", 0, NULL},
    {MIPC_SYS_CONFIG_IND, "CONFIG_IND", 0, NULL},
    {MIPC_SYS_ADPCLK_IND, "ADPCLK_IND", 0, NULL},
    {MIPC_SYS_MCF_IND, "MCF_IND", 0, NULL},
    {MIPC_SYS_SBP_IND, "SBP_IND", 0, NULL},
    {MIPC_SYS_EL2_IP_UL_IND, "EL2_IP_UL_IND", 0, NULL},
    {MIPC_SYS_EL2_IP_DL_IND, "EL2_IP_DL_IND", 0, NULL},
    {MIPC_SYS_EL2_MAC_UL_IND, "EL2_MAC_UL_IND", 0, NULL},
    {MIPC_SYS_EL2_MAC_DL_IND, "EL2_MAC_DL_IND", 0, NULL},
    {MIPC_SYS_EL2_PDCP_UL_IND, "EL2_PDCP_UL_IND", 0, NULL},
    {MIPC_SYS_EL2_PDCP_DL_IND, "EL2_PDCP_DL_IND", 0, NULL},
    {MIPC_SYS_NL2_MAC_UL_IND, "NL2_MAC_UL_IND", 0, NULL},
    {MIPC_SYS_NL2_MAC_DL_IND, "NL2_MAC_DL_IND", 0, NULL},
    {MIPC_SYS_NL2_PDCP_UL_IND, "NL2_PDCP_UL_IND", 0, NULL},
    {MIPC_SYS_NL2_PDCP_DL_IND, "NL2_PDCP_DL_IND", 0, NULL},
    {MIPC_SYS_GEO_LOCATION_IND, "GEO_LOCATION_IND", 0, NULL},
    {MIPC_SYS_MD_INIT_IND, "MD_INIT_IND", 0, NULL},
    {MIPC_SYS_WARNING_IND, "WARNING_IND", 0, NULL},
    {MIPC_SYS_NV_SIG_ERR_IND, "NV_SIG_ERR_IND", 0, NULL},
    {MIPC_SYS_VODATA_STATISTICS_IND, "VODATA_STATISTICS_IND", 0, NULL},
    {MIPC_SYS_THERMAL_ACTUATOR_IND, "THERMAL_ACTUATOR_IND", 0, NULL},
    {MIPC_SYS_HBA_TIMEOUT_IND, "HBA_TIMEOUT_IND", 0, NULL},
    {MIPC_SYS_HBA_HW_FILTER_SRC_STATE_IND, "HBA_HW_FILTER_SRC_STATE_IND", 0, NULL},
    {MIPC_SYS_TIMER_EXPIRE_IND, "TIMER_EXPIRE_IND", 0, NULL},
    {MIPC_APN_IA_CFG_IND, "IA_CFG_IND", 0, NULL},
    {MIPC_APN_PROFILE_CFG_IND, "PROFILE_CFG_IND", 0, NULL},
    {MIPC_APN_VZW_CHG_IND, "VZW_CHG_IND", 0, NULL},
    {MIPC_DATA_ACT_CALL_IND, "ACT_CALL_IND", 0, NULL},
    {MIPC_DATA_DEACT_CALL_IND, "DEACT_CALL_IND", 0, NULL},
    {MIPC_DATA_MOD_CALL_IND, "MOD_CALL_IND", 0, NULL},
    {MIPC_DATA_MOD_PCO_IND, "MOD_PCO_IND", 0, NULL},
    {MIPC_DATA_WWAN_ACT_CALL_IND, "WWAN_ACT_CALL_IND", 0, NULL},
    {MIPC_DATA_WWAN_DEACT_CALL_IND, "WWAN_DEACT_CALL_IND", 0, NULL},
    {MIPC_DATA_CALL_ACT_REG_IND, "CALL_ACT_REG_IND", 0, NULL},
    {MIPC_DATA_CALL_DEACT_REG_IND, "CALL_DEACT_REG_IND", 0, NULL},
    {MIPC_DATA_CALL_MOD_REG_IND, "CALL_MOD_REG_IND", 0, NULL},
    {MIPC_DATA_MD_ACT_CALL_IND, "MD_ACT_CALL_IND", 0, NULL},
    {MIPC_DATA_MD_DEACT_CALL_IND, "MD_DEACT_CALL_IND", 0, NULL},
    {MIPC_DATA_IWLAN_PRIORITY_LIST_IND, "IWLAN_PRIORITY_LIST_IND", 0, NULL},
    {MIPC_DATA_LINK_CAPACITY_ESTIMATE_IND, "LINK_CAPACITY_ESTIMATE_IND", 0, NULL},
    {MIPC_DATA_NW_LIMIT_IND, "NW_LIMIT_IND", 0, NULL},
    {MIPC_DATA_TIMER_IND, "TIMER_IND", 0, NULL},
    {MIPC_DATA_KEEPALIVE_STATUS_IND, "KEEPALIVE_STATUS_IND", 0, NULL},
    {MIPC_DATA_MOBILE_DATA_USAGE_IND, "MOBILE_DATA_USAGE_IND", 0, NULL},
    {MIPC_DATA_NETWORK_REJECT_CAUSE_IND, "NETWORK_REJECT_CAUSE_IND", 0, NULL},
    {MIPC_DATA_DSDA_STATE_IND, "DSDA_STATE_IND", 0, NULL},
    {MIPC_DATA_UMTS_PS_STATE_IND, "UMTS_PS_STATE_IND", 0, NULL},
    {MIPC_DATA_RETRY_TIMER_IND, "RETRY_TIMER_IND", 0, NULL},
    {MIPC_DATA_URSP_REEVAL_IND, "URSP_REEVAL_IND", 0, NULL},
    {MIPC_DATA_URSP_UE_POLICY_CHG_IND, "URSP_UE_POLICY_CHG_IND", 0, NULL},
    {MIPC_DATA_PDN_NW_CAUSE_IND, "PDN_NW_CAUSE_IND", 0, NULL},
    {MIPC_DATA_TSN_TIME_IND, "TSN_TIME_IND", 0, NULL},
    {MIPC_DATA_IP_PACKET_RECV_IND, "IP_PACKET_RECV_IND", 0, NULL},
    {MIPC_DATA_RELEASE_CONFLICT_IF_IND, "RELEASE_CONFLICT_IF_IND", 0, NULL},
    {MIPC_INTERNAL_TEST_IND, "TEST_IND", 0, NULL},
    {MIPC_INTERNAL_EIF_IND, "EIF_IND", 0, NULL},
    {MIPC_INTERNAL_HO_IND, "HO_IND", 0, NULL},
    {MIPC_INTERNAL_MIPC_PENDING_IND, "MIPC_PENDING_IND", 0, NULL},
    {MIPC_NW_REGISTER_IND, "REGISTER_IND", 0, NULL},
    {MIPC_NW_SIGNAL_IND, "SIGNAL_IND", 0, NULL},
    {MIPC_NW_PS_IND, "PS_IND", 0, NULL},
    {MIPC_NW_RADIO_IND, "RADIO_IND", 0, NULL},
    {MIPC_NW_IA_IND, "IA_IND", 0, NULL},
    {MIPC_NW_NITZ_IND, "NITZ_IND", 0, NULL},
    {MIPC_NW_LOCATION_INFO_IND, "LOCATION_INFO_IND", 0, NULL},
    {MIPC_NW_CS_IND, "CS_IND", 0, NULL},
    {MIPC_NW_CSCON_IND, "CSCON_IND", 0, NULL},
    {MIPC_NW_PREFERRED_PROVIDER_IND, "PREFERRED_PROVIDER_IND", 0, NULL},
    {MIPC_NW_CAINFO_IND, "CAINFO_IND", 0, NULL},
    {MIPC_NW_EONS_IND, "EONS_IND", 0, NULL},
    {MIPC_NW_CIEV_IND, "CIEV_IND", 0, NULL},
    {MIPC_NW_EGMSS_IND, "EGMSS_IND", 0, NULL},
    {MIPC_NW_PSBEARER_IND, "PSBEARER_IND", 0, NULL},
    {MIPC_NW_ECELL_IND, "ECELL_IND", 0, NULL},
    {MIPC_NW_ANBR_IND, "ANBR_IND", 0, NULL},
    {MIPC_NW_IRAT_IND, "IRAT_IND", 0, NULL},
    {MIPC_NW_EREGINFO_IND, "EREGINFO_IND", 0, NULL},
    {MIPC_NW_EMODCFG_IND, "EMODCFG_IND", 0, NULL},
    {MIPC_NW_EPCELLINFO_IND, "EPCELLINFO_IND", 0, NULL},
    {MIPC_NW_PSEUDO_CELL_IND, "PSEUDO_CELL_IND", 0, NULL},
    {MIPC_NW_NETWORK_INFO_IND, "NETWORK_INFO_IND", 0, NULL},
    {MIPC_NW_MCCMNC_IND, "MCCMNC_IND", 0, NULL},
    {MIPC_NW_PHYSICAL_CHANNEL_CONFIGS_IND, "PHYSICAL_CHANNEL_CONFIGS_IND", 0, NULL},
    {MIPC_NW_OTACMSG_IND, "OTACMSG_IND", 0, NULL},
    {MIPC_NW_BARRING_INFO_IND, "BARRING_INFO_IND", 0, NULL},
    {MIPC_NW_RADIO_CAPABILITY_IND, "RADIO_CAPABILITY_IND", 0, NULL},
    {MIPC_NW_CURRENT_RAT_IND, "CURRENT_RAT_IND", 0, NULL},
    {MIPC_NW_CAMP_STATE_IND, "CAMP_STATE_IND", 0, NULL},
    {MIPC_NW_NR_SWITCH_IND, "NR_SWITCH_IND", 0, NULL},
    {MIPC_NW_FEMTOCELL_INFO_IND, "FEMTOCELL_INFO_IND", 0, NULL},
    {MIPC_NW_ETXPWR_IND, "ETXPWR_IND", 0, NULL},
    {MIPC_NW_ETXPWRSTUS_IND, "ETXPWRSTUS_IND", 0, NULL},
    {MIPC_NW_IWLAN_IND, "IWLAN_IND", 0, NULL},
    {MIPC_NW_CH_INFO_IND, "CH_INFO_IND", 0, NULL},
    {MIPC_NW_NRUW_INFO_IND, "NRUW_INFO_IND", 0, NULL},
    {MIPC_NW_NR_CA_BAND_IND, "NR_CA_BAND_IND", 0, NULL},
    {MIPC_NW_NR_SCS_IND, "NR_SCS_IND", 0, NULL},
    {MIPC_NW_NETWORK_SCAN_IND, "NETWORK_SCAN_IND", 0, NULL},
    {MIPC_NW_CA_INFO_IND, "CA_INFO_IND", 0, NULL},
    {MIPC_NW_NRUC_INFO_IND, "NRUC_INFO_IND", 0, NULL},
    {MIPC_NW_FIRST_PLMN_IND, "FIRST_PLMN_IND", 0, NULL},
    {MIPC_NW_ENWREJAC_IND, "ENWREJAC_IND", 0, NULL},
    {MIPC_NW_BASEMENT_DETECTION_IND, "BASEMENT_DETECTION_IND", 0, NULL},
    {MIPC_NW_ENWCFGINFO_IND, "ENWCFGINFO_IND", 0, NULL},
    {MIPC_NW_EMSR_IND, "EMSR_IND", 0, NULL},
    {MIPC_NW_LCM_HOPPING_IND, "LCM_HOPPING_IND", 0, NULL},
    {MIPC_NW_NRRC_RRC_RELEASE_UAI_IND, "NRRC_RRC_RELEASE_UAI_IND", 0, NULL},
    {MIPC_NW_NRRC_POWERSAVING_UAI_IND, "NRRC_POWERSAVING_UAI_IND", 0, NULL},
    {MIPC_NW_NRRC_OVERHEATINGASSISTANCE_UAI_IND, "NRRC_OVERHEATINGASSISTANCE_UAI_IND", 0, NULL},
    {MIPC_NW_NRRC_TIMER_STATUS_IND, "NRRC_TIMER_STATUS_IND", 0, NULL},
    {MIPC_NW_ACTUAL_REGISTER_IND, "ACTUAL_REGISTER_IND", 0, NULL},
    {MIPC_NW_HANDOVER_INFO_IND, "HANDOVER_INFO_IND", 0, NULL},
    {MIPC_NW_EVENT_REPORT_IND, "EVENT_REPORT_IND", 0, NULL},
    {MIPC_NW_SERVING_CELL_EVENT_IND, "SERVING_CELL_EVENT_IND", 0, NULL},
    {MIPC_NW_CONFIGURED_BAND_UNSUPPORT_IND, "CONFIGURED_BAND_UNSUPPORT_IND", 0, NULL},
    {MIPC_NW_REGISTER_CONFIG_IND, "REGISTER_CONFIG_IND", 0, NULL},
    {MIPC_SIM_STATE_IND, "STATE_IND", 0, NULL},
    {MIPC_SIM_STATUS_IND, "STATUS_IND", 0, NULL},
    {MIPC_SIM_EUICC_SLOTS_STATUS_IND, "EUICC_SLOTS_STATUS_IND", 0, NULL},
    {MIPC_SIM_ICCID_IND, "ICCID_IND", 0, NULL},
    {MIPC_SIM_STATUS_CHANGE_WITH_CAUSE_IND, "STATUS_CHANGE_WITH_CAUSE_IND", 0, NULL},
    {MIPC_SIM_PHYSICAL_SLOTS_MAPPING_DONE_IND, "PHYSICAL_SLOTS_MAPPING_DONE_IND", 0, NULL},
    {MIPC_SIM_CSIM_IMSI_CHANGE_IND, "CSIM_IMSI_CHANGE_IND", 0, NULL},
    {MIPC_SIM_SML_STATUS_IND, "SML_STATUS_IND", 0, NULL},
    {MIPC_SIM_SML_RSU_IND, "SML_RSU_IND", 0, NULL},
    {MIPC_SIM_VSIM_RESET_IND, "VSIM_RESET_IND", 0, NULL},
    {MIPC_SIM_VSIM_APDU_IND, "VSIM_APDU_IND", 0, NULL},
    {MIPC_SIM_VSIM_EVENT_IND, "VSIM_EVENT_IND", 0, NULL},
    {MIPC_SIM_SIMAPP_IND, "SIMAPP_IND", 0, NULL},
    {MIPC_SIM_TEST_SIM_IND, "TEST_SIM_IND", 0, NULL},
    {MIPC_SIM_CT3G_IND, "CT3G_IND", 0, NULL},
    {MIPC_SIM_CARD_TYPE_IND, "CARD_TYPE_IND", 0, NULL},
    {MIPC_SIM_SIMIND_IND, "SIMIND_IND", 0, NULL},
    {MIPC_SIM_STATUS_CHANGED_IND, "STATUS_CHANGED_IND", 0, NULL},
    {MIPC_SIM_ESLOTESIM_STATE_IND, "ESLOTESIM_STATE_IND", 0, NULL},
    {MIPC_SIM_UICC_CHANNEL_STATUS_CHANGE_IND, "UICC_CHANNEL_STATUS_CHANGE_IND", 0, NULL},
    {MIPC_SMS_CFG_IND, "CFG_IND", 0, NULL},
    {MIPC_SMS_NEW_SMS_IND, "NEW_SMS_IND", 0, NULL},
    {MIPC_SMS_STORE_STATUS_IND, "STORE_STATUS_IND", 0, NULL},
    {MIPC_SMS_NEW_STATUS_REPORT_IND, "NEW_STATUS_REPORT_IND", 0, NULL},
    {MIPC_SMS_NEW_CBM_IND, "NEW_CBM_IND", 0, NULL},
    {MIPC_SMS_SCBM_IND, "SCBM_IND", 0, NULL},
    {MIPC_SMS_EXT_INFO_IND, "EXT_INFO_IND", 0, NULL},
    {MIPC_SMS_DUP_NEW_SMS_IND, "DUP_NEW_SMS_IND", 0, NULL},
    {MIPC_SS_USSD_IND, "USSD_IND", 0, NULL},
    {MIPC_SS_ECMCCSS_IND, "ECMCCSS_IND", 0, NULL},
    {MIPC_SS_CFU_IND, "CFU_IND", 0, NULL},
    {MIPC_SS_XCAP_RCN_IND, "XCAP_RCN_IND", 0, NULL},
    {MIPC_SS_IMS_XUI_IND, "IMS_XUI_IND", 0, NULL},
    {MIPC_STK_PAC_IND, "PAC_IND", 0, NULL},
    {MIPC_STK_SIM_REFRESH_IND, "SIM_REFRESH_IND", 0, NULL},
    {MIPC_STK_BIP_EVENT_NOTIFY_IND, "BIP_EVENT_NOTIFY_IND", 0, NULL},
    {MIPC_CALL_STATUS_IND, "STATUS_IND", 0, NULL},
    {MIPC_CALL_EVENT_IND, "EVENT_IND", 0, NULL},
    {MIPC_CALL_MODE_IND, "MODE_IND", 0, NULL},
    {MIPC_CALL_SIP_IND, "SIP_IND", 0, NULL},
    {MIPC_CALL_CONFERENCE_IND, "CONFERENCE_IND", 0, NULL},
    {MIPC_CALL_IMS_EVENT_PACKAGE_IND, "IMS_EVENT_PACKAGE_IND", 0, NULL},
    {MIPC_CALL_ECC_LIST_CHANGE_IND, "ECC_LIST_CHANGE_IND", 0, NULL},
    {MIPC_CALL_SS_IND, "SS_IND", 0, NULL},
    {MIPC_CALL_ECBM_CHANGE_IND, "ECBM_CHANGE_IND", 0, NULL},
    {MIPC_CALL_CRSS_IND, "CRSS_IND", 0, NULL},
    {MIPC_CALL_ECT_IND, "ECT_IND", 0, NULL},
    {MIPC_CALL_CIPHER_IND, "CIPHER_IND", 0, NULL},
    {MIPC_CALL_RTT_AUDIO_IND, "RTT_AUDIO_IND", 0, NULL},
    {MIPC_CALL_RTT_CAPABILITY_IND, "RTT_CAPABILITY_IND", 0, NULL},
    {MIPC_CALL_LOCAL_RTT_MODIFY_RESULT_IND, "LOCAL_RTT_MODIFY_RESULT_IND", 0, NULL},
    {MIPC_CALL_PEER_RTT_MODIFY_RESULT_IND, "PEER_RTT_MODIFY_RESULT_IND", 0, NULL},
    {MIPC_CALL_RTT_TEXT_RECEIVE_IND, "RTT_TEXT_RECEIVE_IND", 0, NULL},
    {MIPC_CALL_RCS_DIGITS_LINE_IND, "RCS_DIGITS_LINE_IND", 0, NULL},
    {MIPC_CALL_DISPLAY_AND_SIGNALS_INFO_IND, "DISPLAY_AND_SIGNALS_INFO_IND", 0, NULL},
    {MIPC_CALL_EXTENDED_DISPLAY_INFO_IND, "EXTENDED_DISPLAY_INFO_IND", 0, NULL},
    {MIPC_CALL_LINE_CONTROL_INFO_IND, "LINE_CONTROL_INFO_IND", 0, NULL},
    {MIPC_CALL_REDIRECTING_NUMBER_INFO_IND, "REDIRECTING_NUMBER_INFO_IND", 0, NULL},
    {MIPC_CALL_GWSD_EVENT_IND, "GWSD_EVENT_IND", 0, NULL},
    {MIPC_CALL_ECONF_IND, "ECONF_IND", 0, NULL},
    {MIPC_CALL_IMS_SIP_HEADER_IND, "IMS_SIP_HEADER_IND", 0, NULL},
    {MIPC_CALL_ECC_REDIAL_IND, "ECC_REDIAL_IND", 0, NULL},
    {MIPC_CALL_EMERGENCY_BEARER_SUPPORT_IND, "EMERGENCY_BEARER_SUPPORT_IND", 0, NULL},
    {MIPC_CALL_UIS_INFO_IND, "UIS_INFO_IND", 0, NULL},
    {MIPC_CALL_CALL_ADDITIONAL_INFO_IND, "CALL_ADDITIONAL_INFO_IND", 0, NULL},
    {MIPC_CALL_MT_SIP_INVITE_IND, "MT_SIP_INVITE_IND", 0, NULL},
    {MIPC_CALL_RECV_DTMF_IND, "RECV_DTMF_IND", 0, NULL},
    {MIPC_CALL_DC_CONFIG_INIT_IND, "DC_CONFIG_INIT_IND", 0, NULL},
    {MIPC_CALL_DC_CONFIG_UPDATE_IND, "DC_CONFIG_UPDATE_IND", 0, NULL},
    {MIPC_CALL_DC_CONFIG_DEINIT_IND, "DC_CONFIG_DEINIT_IND", 0, NULL},
    {MIPC_IMS_CONFIG_IND, "CONFIG_IND", 0, NULL},
    {MIPC_IMS_STATE_IND, "STATE_IND", 0, NULL},
    {MIPC_IMS_SUPPORT_ECC_IND, "SUPPORT_ECC_IND", 0, NULL},
    {MIPC_IMS_PDN_IND, "PDN_IND", 0, NULL},
    {MIPC_IMS_NAPTR_IND, "NAPTR_IND", 0, NULL},
    {MIPC_IMS_REG_IND, "REG_IND", 0, NULL},
    {MIPC_IMS_SIP_REG_INFO_IND, "SIP_REG_INFO_IND", 0, NULL},
    {MIPC_IMS_VOPS_IND, "VOPS_IND", 0, NULL},
    {MIPC_IMS_REG_REMAIN_TIME_IND, "REG_REMAIN_TIME_IND", 0, NULL},
    {MIPC_IMS_UI_IND, "UI_IND", 0, NULL},
    {MIPC_WFC_CELL_SIGNAL_IND, "CELL_SIGNAL_IND", 0, NULL},
    {MIPC_WFC_WIFI_PDN_COUNT_IND, "WIFI_PDN_COUNT_IND", 0, NULL},
    {MIPC_WFC_PDN_HO_IND, "PDN_HO_IND", 0, NULL},
    {MIPC_WFC_ROVE_OUT_IND, "ROVE_OUT_IND", 0, NULL},
    {MIPC_WFC_SSAC_IND, "SSAC_IND", 0, NULL},
    {MIPC_WFC_WIFI_PDN_ERR_IND, "WIFI_PDN_ERR_IND", 0, NULL},
    {MIPC_WFC_WIFI_PDN_OOS_IND, "WIFI_PDN_OOS_IND", 0, NULL},
    {MIPC_WFC_WFC_IND, "WFC_IND", 0, NULL},
    {MIPC_PHB_READY_STATE_IND, "READY_STATE_IND", 0, NULL},
    {MIPC_EMBMS_EMSRV_IND, "EMSRV_IND", 0, NULL},
    {MIPC_EMBMS_EMSLUI_IND, "EMSLUI_IND", 0, NULL},
    {MIPC_EMBMS_EMSAILNF_IND, "EMSAILNF_IND", 0, NULL},
    {MIPC_EMBMS_EMSESS_IND, "EMSESS_IND", 0, NULL},
    {MIPC_EMBMS_EHVOLTE_IND, "EHVOLTE_IND", 0, NULL},
    {MIPC_ECALL_STATUS_IND, "STATUS_IND", 0, NULL},
    {MIPC_SYS_REBOOT_CMD, "REBOOT_CMD", 0, NULL},
    {MIPC_SYS_REBOOT_RSP, "REBOOT_RSP", 0, NULL},
    {MIPC_SYS_SET_CONFIG_DIPC_CMD, "SET_CONFIG_DIPC_CMD", 0, NULL},
    {MIPC_SYS_SET_CONFIG_DIPC_RSP, "SET_CONFIG_DIPC_RSP", 1, mipc_sys_set_config_dipc_rsp_tlv_checker_info},
    {MIPC_SYS_CONFIG_NEEDED_TO_UPDATE_CMD, "CONFIG_NEEDED_TO_UPDATE_CMD", 0, NULL},
    {MIPC_SYS_CONFIG_NEEDED_TO_UPDATE_RSP, "CONFIG_NEEDED_TO_UPDATE_RSP", 0, NULL},
    {MIPC_SYS_GET_CONFIG_DIPC_CMD, "GET_CONFIG_DIPC_CMD", 0, NULL},
    {MIPC_SYS_GET_CONFIG_DIPC_RSP, "GET_CONFIG_DIPC_RSP", 1, mipc_sys_get_config_dipc_rsp_tlv_checker_info},
    {MIPC_SYS_SET_TIME_CMD, "SET_TIME_CMD", 0, NULL},
    {MIPC_SYS_SET_TIME_RSP, "SET_TIME_RSP", 0, NULL},
    {MIPC_SYS_SEND_CONSYS_WIFI_CMD, "SEND_CONSYS_WIFI_CMD", 0, NULL},
    {MIPC_SYS_SEND_CONSYS_WIFI_RSP, "SEND_CONSYS_WIFI_RSP", 0, NULL},
    {MIPC_SYS_SEND_CONSYS_BT_CMD, "SEND_CONSYS_BT_CMD", 0, NULL},
    {MIPC_SYS_SEND_CONSYS_BT_RSP, "SEND_CONSYS_BT_RSP", 0, NULL},
    {MIPC_DATA_IFUP_PACKET_ROUTE_CMD, "IFUP_PACKET_ROUTE_CMD", 0, NULL},
    {MIPC_DATA_IFUP_PACKET_ROUTE_RSP, "IFUP_PACKET_ROUTE_RSP", 1, mipc_data_ifup_packet_route_rsp_tlv_checker_info},
    {MIPC_INTERNAL_TEST_CMD, "TEST_CMD", 0, NULL},
    {MIPC_INTERNAL_TEST_RSP, "TEST_RSP", 0, NULL},
    {MIPC_INTERNAL_EIPPORT_CMD, "EIPPORT_CMD", 0, NULL},
    {MIPC_INTERNAL_EIPPORT_RSP, "EIPPORT_RSP", 2, mipc_internal_eipport_rsp_tlv_checker_info},
    {MIPC_INTERNAL_EIPSPI_CMD, "EIPSPI_CMD", 0, NULL},
    {MIPC_INTERNAL_EIPSPI_RSP, "EIPSPI_RSP", 0, NULL},
    {MIPC_INTERNAL_MULTI_EIPSPI_FREE_CMD, "MULTI_EIPSPI_FREE_CMD", 0, NULL},
    {MIPC_INTERNAL_MULTI_EIPSPI_FREE_RSP, "MULTI_EIPSPI_FREE_RSP", 2, mipc_internal_multi_eipspi_free_rsp_tlv_checker_info},
    {MIPC_SMS_NEW_SMS_CMD, "NEW_SMS_CMD", 0, NULL},
    {MIPC_SMS_NEW_SMS_RSP, "NEW_SMS_RSP", 5, mipc_sms_new_sms_rsp_tlv_checker_info},
    {MIPC_SMS_NEW_STATUS_REPORT_CMD, "NEW_STATUS_REPORT_CMD", 0, NULL},
    {MIPC_SMS_NEW_STATUS_REPORT_RSP, "NEW_STATUS_REPORT_RSP", 2, mipc_sms_new_status_report_rsp_tlv_checker_info},
    {MIPC_CALL_APPROVE_INCOMING_CMD, "APPROVE_INCOMING_CMD", 0, NULL},
    {MIPC_CALL_APPROVE_INCOMING_RSP, "APPROVE_INCOMING_RSP", 1, mipc_call_approve_incoming_rsp_tlv_checker_info},
    {MIPC_CALL_DC_BOOTSTRAP_CFG_QRY_CMD, "DC_BOOTSTRAP_CFG_QRY_CMD", 0, NULL},
    {MIPC_CALL_DC_BOOTSTRAP_CFG_QRY_RSP, "DC_BOOTSTRAP_CFG_QRY_RSP", 5, mipc_call_dc_bootstrap_cfg_qry_rsp_tlv_checker_info},
    {MIPC_CALL_DC_CREATE_CMD, "DC_CREATE_CMD", 0, NULL},
    {MIPC_CALL_DC_CREATE_RSP, "DC_CREATE_RSP", 9, mipc_call_dc_create_rsp_tlv_checker_info},
    {MIPC_IMS_PDIS_CMD, "PDIS_CMD", 0, NULL},
    {MIPC_IMS_PDIS_RSP, "PDIS_RSP", 1, mipc_ims_pdis_rsp_tlv_checker_info},
    {MIPC_WFC_PING_CMD, "PING_CMD", 0, NULL},
    {MIPC_WFC_PING_RSP, "PING_RSP", 1, mipc_wfc_ping_rsp_tlv_checker_info},
    {MIPC_WFC_GET_MAC_CMD, "GET_MAC_CMD", 0, NULL},
    {MIPC_WFC_GET_MAC_RSP, "GET_MAC_RSP", 3, mipc_wfc_get_mac_rsp_tlv_checker_info},
    {MIPC_WFC_NATT_KEEP_ALIVE_CMD, "NATT_KEEP_ALIVE_CMD", 0, NULL},
    {MIPC_WFC_NATT_KEEP_ALIVE_RSP, "NATT_KEEP_ALIVE_RSP", 4, mipc_wfc_natt_keep_alive_rsp_tlv_checker_info},
    {MIPC_WFC_REGISTER_WIFI_SIGNAL_NTF_CMD, "REGISTER_WIFI_SIGNAL_NTF_CMD", 0, NULL},
    {MIPC_WFC_REGISTER_WIFI_SIGNAL_NTF_RSP, "REGISTER_WIFI_SIGNAL_NTF_RSP", 0, NULL},
    {MIPC_WFC_SET_DL_FILTER_CMD, "SET_DL_FILTER_CMD", 0, NULL},
    {MIPC_WFC_SET_DL_FILTER_RSP, "SET_DL_FILTER_RSP", 0, NULL},
    {MIPC_WFC_DEL_DL_FILTER_CMD, "DEL_DL_FILTER_CMD", 0, NULL},
    {MIPC_WFC_DEL_DL_FILTER_RSP, "DEL_DL_FILTER_RSP", 0, NULL},
    {MIPC_DATA_ACT_CALL_NTF, "ACT_CALL_NTF", 1, mipc_data_act_call_ntf_tlv_checker_info},
    {MIPC_DATA_ACT_PACKET_ROUTE_NTF, "ACT_PACKET_ROUTE_NTF", 1, mipc_data_act_packet_route_ntf_tlv_checker_info},
    {MIPC_INTERNAL_TEST_NTF, "TEST_NTF", 0, NULL},
    {MIPC_INTERNAL_INJECT_TST_NTF, "INJECT_TST_NTF", 2, mipc_internal_inject_tst_ntf_tlv_checker_info},
    {MIPC_WFC_WIFI_SIGNAL_NTF, "WIFI_SIGNAL_NTF", 1, mipc_wfc_wifi_signal_ntf_tlv_checker_info},
    {MIPC_WFC_EPDG_SCREEN_STATE_NTF, "EPDG_SCREEN_STATE_NTF", 1, mipc_wfc_epdg_screen_state_ntf_tlv_checker_info},
    {MIPC_WFC_WIFI_INFO_NTF, "WIFI_INFO_NTF", 18, mipc_wfc_wifi_info_ntf_tlv_checker_info},
};

#endif
