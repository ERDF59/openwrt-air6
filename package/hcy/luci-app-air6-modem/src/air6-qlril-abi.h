/* SPDX-License-Identifier: GPL-2.0-only */
/* QuecOpen T830 ABI declarations; no vendor implementation. */
#pragma once
#include <stdint.h>
#define QL_NW_MAX_GSM_CELL_INFO_NUM 10
#define QL_NW_MAX_UMTS_CELL_INFO_NUM 10
#define QL_NW_MAX_LTE_CELL_INFO_NUM 20
#define QL_NW_MAX_NR_CELL_INFO_NUM 20

typedef enum
{
    NW_IPC_MODE_DEFAULT=0x00,
    NW_IPC_MODE_UBUS
}NW_IPC_MODE;

typedef enum QL_NW_ROAM_STATE_TYPE_ENUM
{
    QL_NW_ROAM_STATE_OFF  = 0,
    QL_NW_ROAM_STATE_ON   = 1
}QL_NW_ROAM_STATE_TYPE_E;

typedef enum QL_NW_RADIO_TECH_TYPE_ENUM
{
    QL_NW_RADIO_TECH_TD_SCDMA = 1,
    QL_NW_RADIO_TECH_GSM      = 2,
    QL_NW_RADIO_TECH_HSPAP    = 3,
    QL_NW_RADIO_TECH_LTE      = 4,
    QL_NW_RADIO_TECH_EHRPD    = 5,
    QL_NW_RADIO_TECH_EVDO_B   = 6,
    QL_NW_RADIO_TECH_HSPA     = 7,
    QL_NW_RADIO_TECH_HSUPA    = 8,
    QL_NW_RADIO_TECH_HSDPA    = 9,
    QL_NW_RADIO_TECH_EVDO_A   = 10,
    QL_NW_RADIO_TECH_EVDO_0   = 11,
    QL_NW_RADIO_TECH_1xRTT    = 12,
    QL_NW_RADIO_TECH_IS95B    = 13,
    QL_NW_RADIO_TECH_IS95A    = 14,
    QL_NW_RADIO_TECH_UMTS     = 15,
    QL_NW_RADIO_TECH_EDGE     = 16,
    QL_NW_RADIO_TECH_GPRS     = 17,
    QL_NW_RADIO_TECH_NR5G     = 18,
    QL_NW_RADIO_TECH_NSA5G    = 19,
    QL_NW_RADIO_TECH_NONE     = 20
}QL_NW_RADIO_TECH_TYPE_E;

typedef enum QL_NW_TECH_DOMAIN_TYPE_ENUM
{
    QL_NW_TECH_DOMAIN_NONE    = 0,
    QL_NW_TECH_DOMAIN_3GPP    = 1,
    QL_NW_TECH_DOMAIN_3GPP2   = 2,
}QL_NW_TECH_DOMAIN_TYPE_E;

typedef enum QL_NW_DENY_REASON_TYPE_ENUM
{
    QL_NW_IMSI_UNKNOWN_HLR_DENY_REASON                  = 1,
    QL_NW_ILLEGAL_MS_DENY_REASON                        = 2,
    QL_NW_IMSI_UNKNOWN_VLR_DENY_REASON                  = 3,
    QL_NW_IMEI_NOT_ACCEPTED_DENY_REASON                 = 4,
    QL_NW_ILLEGAL_ME_DENY_REASON                        = 5,
    QL_NW_PLMN_NOT_ALLOWED_DENY_REASON                  = 6,
    QL_NW_LA_NOT_ALLOWED_DENY_REASON                    = 7,
    QL_NW_ROAMING_NOT_ALLOWED_LA_DENY_REASON            = 8,
    QL_NW_NO_SUITABLE_CELLS_LA_DENY_REASON              = 9,
    QL_NW_NETWORK_FAILURE_DENY_REASON                   = 10,
    QL_NW_MAC_FAILURE_DENY_REASON                       = 11,
    QL_NW_SYNCH_FAILURE_DENY_REASON                     = 12,
    QL_NW_CONGESTION_DENY_REASON                        = 13,
    QL_NW_GSM_AUTHENTICATION_UNACCEPTABLE_DENY_REASON   = 14,
    QL_NW_NOT_AUTHORIZED_CSG_DENY_REASON                = 15,
    QL_NW_SERVICE_OPTION_NOT_SUPPORTED_DENY_REASON      = 16,
    QL_NW_REQ_SERVICE_OPTION_NOT_SUBSCRIBED_DENY_REASON = 17,
    QL_NW_CALL_CANNOT_BE_IDENTIFIED_DENY_REASON         = 18,
    QL_NW_SEMANTICALLY_INCORRECT_MSG_DENY_REASON        = 19,
    QL_NW_INVALID_MANDATORY_INFO_DENY_REASON            = 20,
    QL_NW_MSG_TYPE_NON_EXISTENT_DENY_REASON             = 21,
    QL_NW_INFO_ELEMENT_NON_EXISTENT_DENY_REASON         = 22,
    QL_NW_CONDITIONAL_IE_ERR_DENY_REASON                = 23,
    QL_NW_MSG_INCOMPATIBLE_PROTOCOL_STATE_DENY_REASON   = 24,
    QL_NW_PROTOCOL_ERROR_DENY_REASON                    = 25,
}QL_NW_DENY_REASON_TYPE_E;

typedef enum QL_NW_SERVICE_TYPE_ENUM
{
    QL_NW_SERVICE_NONE						= 0,
    QL_NW_SERVICE_LIMITED					= 1,
    QL_NW_SERVICE_FULL						= 2,
	QL_NW_SERVICE_SEARCHING					= 3,
	QL_NW_SERVICE_DENIED					= 4,
	QL_NW_SERVICE_UNKNOWN					= 5,
	QL_NW_SERVICE_ROAMING					= 6,
	QL_NW_SERVICE_HOME_SMS_ONLY				= 7,
	QL_NW_SERVICE_ROAMING_SMS_ONLY 			= 8,
	QL_NW_SERVICE_HOME_CSFB_NOT_PREF		= 9,
	QL_NW_SERVICE_ROAMING_CSFB_NOT_PREF		= 10,
}QL_NW_SERVICE_TYPE_E;

typedef struct
{
    uint32_t    cid;
    char        plmn[7];
    uint16_t    lac;
    uint16_t    arfcn;
    uint8_t     bsic;
    int16_t     rssi;
}ql_nw_gsm_info_t;

typedef struct
{
    uint32_t cid;
    uint32_t lcid;
    char plmn[7];
    uint16_t lac;
    uint16_t uarfcn;
    uint16_t psc;
    int16_t  rssi;
}ql_nw_umts_info_t;

typedef struct
{
    uint32_t cid;
    uint32_t eNodeB_id;
    char plmn[7];
    uint16_t tac;
    uint16_t pci;
    uint16_t earfcn;
    int16_t  rssi;
    int32_t  rsrp;
    int32_t  rsrq;
    int32_t  sinr;
    uint32_t ta;
}ql_nw_lte_info_t;

typedef struct
{
    uint64_t cid;
    char plmn[7];
    uint32_t tac;
    uint32_t pci;
    uint32_t nr_arfcn;
    int16_t  rssi;
    int32_t  rsrp;
    int32_t  rsrq;
    int32_t  sinr;
    uint32_t ta;
}ql_nw_nr_info_t;

typedef struct
{
   uint16_t sid;
   uint16_t nid;
   uint16_t bid;
   uint16_t refpn;
   uint32_t base_lat;
   uint32_t base_long;
   int16_t  rssi;
}ql_nw_cdma_info_t;

typedef struct
{
    QL_NW_RADIO_TECH_TYPE_E serving_rat;
    uint8_t                 gsm_info_valid;
    uint8_t                 gsm_info_len;
    ql_nw_gsm_info_t        gsm_info[QL_NW_MAX_GSM_CELL_INFO_NUM];
    uint8_t                 umts_info_valid;
    uint8_t                 umts_info_len;
    ql_nw_umts_info_t       umts_info[QL_NW_MAX_UMTS_CELL_INFO_NUM];
    uint8_t                 lte_info_valid;
    uint8_t                 lte_info_len;
    ql_nw_lte_info_t        lte_info[QL_NW_MAX_LTE_CELL_INFO_NUM];
    uint8_t                 cdma_info_valid;
    ql_nw_cdma_info_t       cdma_info;
    uint8_t                 nr_info_valid;
    uint8_t                 nr_info_len;
    ql_nw_nr_info_t         nr_info[QL_NW_MAX_NR_CELL_INFO_NUM];
}ql_nw_cell_info_t;

typedef struct
{
    QL_NW_TECH_DOMAIN_TYPE_E tech_domain;
    QL_NW_RADIO_TECH_TYPE_E  radio_tech;
    QL_NW_ROAM_STATE_TYPE_E  roaming;
    QL_NW_DENY_REASON_TYPE_E deny_reason;
    QL_NW_SERVICE_TYPE_E     reg_state;
    char                     mcc[3+1];
    char                     mnc[3+1];
    uint8_t                  forbidden;
    uint64_t                 cid;
    uint32_t                 lac;
    uint16_t                 psc;
    uint32_t                 tac;
    uint8_t                  inPRL;
    uint8_t                  css;
    uint16_t                 sid;
    uint16_t                 nid;
    uint16_t                 bsid;
}ql_nw_reg_status_info_t;

typedef struct
{
    uint16_t	lte_band;
	uint16_t	nr_band;
	char		lte_dl_bandwidth[8];
	char		lte_ul_bandwidth[8];
	char		nr_dl_bandwidth[8];
	char		nr_ul_bandwidth[8];
    uint8_t     lte_cqi;
    uint8_t     nr_cqi;
    uint32_t    lte_ul_earfcn;
    uint32_t    lte_dl_earfcn;
    uint32_t    nr_ul_arfcn;
    uint32_t    nr_dl_arfcn;
	uint32_t	nr_point_a_arfcn;
	uint32_t	nr_subcarrier_spacing;
    uint16_t    wcdma_band;
    char        net_ambr_dl[15];
    char        net_ambr_ul[15];
    char        ims_ambr_dl[15];
    char        ims_ambr_ul[15];
}ql_nw_get_band_info_t;

typedef struct
{
    char long_eons[64 + 1];
    char short_eons[64 + 1];
    char mcc[3 + 1];
    char mnc[3 + 1];
}ql_nw_mobile_operator_name_info_t;

typedef struct
{
    char     pcc_type[10];
    uint16_t pcc_band;
    uint32_t pcc_pci;
    uint64_t pcc_arfcn;
    uint16_t pcc_dl_bandwidth;
    uint16_t pcc_ul_bandwidth;
    uint16_t pcc_dl_mimo;
    uint16_t pcc_ul_mimo;
    uint16_t pcc_dl_modulation;
    uint16_t pcc_ul_modulation;
    uint16_t pcc_dl_rb;
    uint16_t pcc_ul_rb;
    uint16_t pcc_dl_mcs;
    uint16_t pcc_ul_mcs;
    int16_t  pcc_pucch_tx_pwr;
    uint16_t pcc_lte_dl_tm;
    uint16_t pcc_lte_ul_tm;
    int16_t  pcc_pusch_tx_pwr;
    uint16_t pcc_dl_tput;
    uint16_t pcc_ul_tput;
    uint16_t pcc_dl_bwp_bandwidth;
    uint16_t pcc_ul_bwp_bandwidth;
}pcc_ca_info_table;

typedef struct
{
    char     scc_type[10];
    uint8_t  scc_state;
    uint8_t  scc_ul_configured;
    uint16_t scc_band;
    uint32_t scc_pci;
    uint64_t scc_arfcn;
    uint16_t scc_dl_bandwidth;
    uint16_t scc_ul_bandwidth;
    uint16_t scc_dl_mimo;
    uint16_t scc_ul_mimo;
    uint16_t scc_dl_modulation;
    uint16_t scc_ul_modulation;
    uint16_t scc_dl_rb;
    uint16_t scc_ul_rb;
    uint16_t scc_dl_mcs;
    uint16_t scc_ul_mcs;
    int16_t  scc_pucch_tx_pwr;
    uint16_t scc_lte_dl_tm;
    uint16_t scc_lte_ul_tm;
    int16_t  scc_pusch_tx_pwr;
    uint16_t scc_dl_tput;
    uint16_t scc_ul_tput;
    uint16_t scc_dl_bwp_bandwidth;
    uint16_t scc_ul_bwp_bandwidth;
}scc_ca_info_table;

typedef struct
{
    uint8_t lte_pcc_num;
    pcc_ca_info_table lte_pcc_ca_info;
    uint8_t lte_scc_num;
    scc_ca_info_table lte_scc_ca_info[8];
    uint8_t nr_pcc_num;
    pcc_ca_info_table nr_pcc_ca_info;
    uint8_t nr_scc_num;
    scc_ca_info_table nr_scc_ca_info[8];
}ql_nw_get_ca_info_resp_t;

typedef struct
{
    char cc_type[10];
    uint8_t path_loss;
    uint8_t rank_indicator;
    int16_t serv_rsrp;
    int16_t serv_rsrq;
    int16_t serv_snr;
    int16_t serv_rssi;
}lte_meas_info_table;

typedef struct
{
    char cc_type[10];
    uint8_t path_loss;
    uint8_t rank_indicator;
    int16_t brsrp;
    int16_t serv_rsrp;
    int16_t serv_rsrq;
    int16_t ssb_best_beam_id;
    int16_t ssb_snr_avg;
    int16_t ssb_rsrp_avg;
    int16_t trs_snr_avg;
    int16_t trs_rsrp_avg;
    int16_t serv_snr;
    int16_t serv_rssi;
}nr_meas_info_table;

typedef struct
{
    uint8_t lte_pcc_num;
    lte_meas_info_table lte_pcc_info;
    uint8_t lte_scc_num;
    lte_meas_info_table lte_scc_info[8];
    uint8_t nr_pcc_num;
    nr_meas_info_table nr_pcc_info;
    uint8_t nr_scc_num;
    nr_meas_info_table nr_scc_info[8];
}ql_nw_get_meas_info_resp_t;

typedef struct
{
    uint8_t    endc_available;
    uint8_t    dcnr_restricted;
}ql_nw_get_endc_status_req_t;
int ql_nw_init(NW_IPC_MODE mode);
int ql_nw_release();
int ql_nw_get_mobile_operator_name(ql_nw_mobile_operator_name_info_t *p_info);
int ql_nw_get_cell_info(ql_nw_cell_info_t *p_info);
int ql_nw_get_data_reg_status(ql_nw_reg_status_info_t *p_info);
int ql_nw_get_band_info(ql_nw_get_band_info_t *p_info);
int ql_nw_get_ca_info(ql_nw_get_ca_info_resp_t *resp_info);
int ql_nw_get_meas_info( ql_nw_get_meas_info_resp_t *resp_info);
int ql_nw_get_endc_status(ql_nw_get_endc_status_req_t *resp_info);

typedef struct {
 int get_band_option;
 uint32_t umts_band_class;
 uint32_t lte_band_class[8];
 uint32_t nr_band_class[32];
} ql_nw_get_band_mode_info_t;
extern int ql_nw_get_band_mode(ql_nw_get_band_mode_info_t *);
