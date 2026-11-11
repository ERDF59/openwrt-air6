// SPDX-License-Identifier: GPL-2.0-only
/* A bounded, read-only QuecOpen RIL snapshot. The supervisor owns the deadline
 * and publishes completed JSON atomically; rpcd never waits for modem I/O. */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <glob.h>
#include <libubox/blobmsg_json.h>
#include "air6-qlril-abi.h"

static struct blob_buf out;
#define S(k,v) blobmsg_add_string(&out,k,v)
#define N(k,v) blobmsg_add_u32(&out,k,(uint32_t)(v))
#define B(k,v) blobmsg_add_u8(&out,k,!!(v))
static void textnum(const char *key,long long value) { char s[32];snprintf(s,sizeof(s),"%lld",value);S(key,s); }
static void num(const char *key,double value) { blobmsg_add_double(&out,key,value); }
static void fixedstr(const char *key,const char *value,size_t size) { char s[128];size_t n=strnlen(value,size);if(n>=sizeof(s))n=sizeof(s)-1;memcpy(s,value,n);s[n]=0;S(key,s); }
static const char *rat(int r) { switch(r){case 18:return "NR5G-SA";case 19:return "NR5G-NSA";case 4:return "LTE";case 20:return "NONE";default:return "UNKNOWN";} }
static double bandwidth(int nr,unsigned index) {
 static const double lte[]={1.4,3,5,10,15,20};
 static const double nr_bw[]={5,10,15,20,25,30,40,50,60,80,90,100,70,200,400};
 return nr ? (index<15?nr_bw[index]:0) : (index<6?lte[index]:0);
}
static void quality(const char *key,int value,int low,int high) {
 if(value>=low&&value<=high)num(key,value);else blobmsg_add_field(&out,BLOBMSG_TYPE_UNSPEC,key,NULL,0);
}
static unsigned carrier_band(const ql_nw_get_ca_info_resp_t *ca,int nr,unsigned pci,unsigned arfcn) {
 const pcc_ca_info_table *p=nr?&ca->nr_pcc_ca_info:&ca->lte_pcc_ca_info;
 unsigned count=nr?ca->nr_pcc_num:ca->lte_pcc_num;
 if(count&&p->pcc_pci==pci&&p->pcc_arfcn==arfcn)return p->pcc_band;
 const scc_ca_info_table *ss=nr?ca->nr_scc_ca_info:ca->lte_scc_ca_info;
 count=nr?ca->nr_scc_num:ca->lte_scc_num;
 for(unsigned i=0;i<count&&i<8;i++)if(ss[i].scc_pci==pci&&ss[i].scc_arfcn==arfcn)return ss[i].scc_band;
 return 0;
}
static void cell(int nr,int serving,uint64_t cid,unsigned pci,unsigned arfcn,unsigned tac,const char *plmn,int rsrp,int rsrq,int sinr,int rssi,unsigned band) {
 void *t=blobmsg_open_table(&out,NULL);
 S("rat",nr?"NR":"LTE");S("role",serving?"serving":"neighbor");
 /* A neighbour's band cannot be inferred unambiguously from ARFCN. */
 if(serving&&band)N("band",band);
 if(cid&&cid!=0x0fffffffULL&&cid!=0xffffffffULL)textnum("cellid",cid);
 N("pci",pci);N("arfcn",arfcn);if(tac&&tac!=0x0fffffffU&&tac!=0xffffffffU)N("tac",tac);fixedstr("plmn",plmn,7);
 quality("rsrp",rsrp,-156,-20);quality("rsrq",rsrq,-44,20);
 quality("sinr",sinr,-50,100);quality("rssi",rssi,-160,-1);
 blobmsg_close_table(&out,t);
}
static void carrier(int nr,int primary,const pcc_ca_info_table *p,const scc_ca_info_table *s) {
 unsigned band=primary?p->pcc_band:s->scc_band;
 if(!band)return;
 void *t=blobmsg_open_table(&out,NULL);
 S("rat",nr?"NR":"LTE");S("role",primary?"PCC":"SCC");N("band",band);
 N("pci",primary?p->pcc_pci:s->scc_pci);
 num("arfcn",primary?p->pcc_arfcn:s->scc_arfcn);
 /* PCC is a serving carrier. SCC activity is reported, never inferred. */
 if(!primary){N("state",s->scc_state);B("active",s->scc_state==2);B("ul_configured",s->scc_ul_configured);}
 double dl=bandwidth(nr,primary?p->pcc_dl_bandwidth:s->scc_dl_bandwidth);
 double ul=bandwidth(nr,primary?p->pcc_ul_bandwidth:s->scc_ul_bandwidth);
 if(dl)num("dl_bandwidth_mhz",dl);
 if(ul)num("ul_bandwidth_mhz",ul);
 N("dl_mimo",primary?p->pcc_dl_mimo:s->scc_dl_mimo);N("ul_mimo",primary?p->pcc_ul_mimo:s->scc_ul_mimo);
 N("dl_modulation_index",primary?p->pcc_dl_modulation:s->scc_dl_modulation);N("ul_modulation_index",primary?p->pcc_ul_modulation:s->scc_ul_modulation);
 N("dl_mcs",primary?p->pcc_dl_mcs:s->scc_dl_mcs);N("ul_mcs",primary?p->pcc_ul_mcs:s->scc_ul_mcs);
 N("dl_kbytes_per_second",primary?p->pcc_dl_tput:s->scc_dl_tput);N("ul_kbytes_per_second",primary?p->pcc_ul_tput:s->scc_ul_tput);
 blobmsg_close_table(&out,t);
}
static void measure(int nr,int primary,int index,const lte_meas_info_table *l,const nr_meas_info_table *n) {
 void *t=blobmsg_open_table(&out,NULL);
 S("rat",nr?"NR":"LTE");S("role",primary?"PCC":"SCC");N("index",index);
 /* Keep 3GPP report indices explicitly named; these are not dBm values. */
 N("rsrp_index",nr?n->serv_rsrp:l->serv_rsrp);N("rsrq_index",nr?n->serv_rsrq:l->serv_rsrq);N("snr_index",nr?n->serv_snr:l->serv_snr);
 quality("rssi_dbm",nr?n->serv_rssi:l->serv_rssi,-160,0);
 N("rank_indicator",nr?n->rank_indicator:l->rank_indicator);N("path_loss_db",nr?n->path_loss:l->path_loss);
 if(nr){quality("best_beam_rsrp_dbm",n->brsrp,-156,-20);if(n->ssb_best_beam_id>=0&&n->ssb_best_beam_id<=63)N("best_beam_id",n->ssb_best_beam_id);quality("ssb_snr_db",n->ssb_snr_avg,-50,100);quality("ssb_rsrp_dbm",n->ssb_rsrp_avg,-156,-20);}
 blobmsg_close_table(&out,t);
}
/* Kernel modem thermal sensors: avoid an AT temperature request. */
static void temperature(void) {
 glob_t paths={0};long maximum=-1;
 if(glob("/sys/class/thermal/thermal_zone*/type",0,NULL,&paths)==0){
  for(size_t i=0;i<paths.gl_pathc;i++){
   char type[64]={0},path[256];FILE *f=fopen(paths.gl_pathv[i],"r");
   if(!f)continue;
   if(!fgets(type,sizeof(type),f))type[0]=0;
   fclose(f);
   if(strncmp(type,"md",2)||!strstr(type,"-thermal"))continue;
   snprintf(path,sizeof(path),"%.*stemp",(int)(strlen(paths.gl_pathv[i])-4),paths.gl_pathv[i]);
   long value=-1;f=fopen(path,"r");if(!f)continue;int got=fscanf(f,"%ld",&value);fclose(f);
   if(got==1&&value>=0&&value<=150000&&value>maximum)maximum=value;
  }
 }
 globfree(&paths);
 if(maximum>=0){char v[32];snprintf(v,sizeof(v),"%.1f",maximum/1000.0);S("TEMP",v);S("temperature_source","kernel-modem-thermal-max");}
}
struct imsi_resp {char imsi[16];uint8_t mnc_len;char plmn[7];};
struct serial_resp {uint8_t imei_valid;char imei[33];uint8_t meid_valid;char meid[33];};
extern int ql_sim_init(int),ql_sim_release(void),ql_sim_get_imsi(int,struct imsi_resp*),ql_sim_get_iccid(int,char*,int);
extern int ql_dm_init(int),ql_dm_release(void),ql_dm_get_device_serial_numbers(int,struct serial_resp*),ql_dm_get_modem_state(char*,int);
_Static_assert(sizeof(ql_nw_nr_info_t)==48,"NR cell ABI");
_Static_assert(sizeof(struct imsi_resp)==24,"SIM ABI");
int main(void) {
 ql_nw_cell_info_t c={0};ql_nw_reg_status_info_t r={0};ql_nw_get_band_info_t b={0};ql_nw_mobile_operator_name_info_t o={0};
 ql_nw_get_ca_info_resp_t ca={0};ql_nw_get_meas_info_resp_t m={0};ql_nw_get_endc_status_req_t e={0};
 if(ql_nw_init(NW_IPC_MODE_UBUS))return 2;
 int rr=ql_nw_get_data_reg_status(&r),cr=ql_nw_get_cell_info(&c),br=ql_nw_get_band_info(&b),orr=ql_nw_get_mobile_operator_name(&o);
 int car=ql_nw_get_ca_info(&ca),mr=ql_nw_get_meas_info(&m),er=ql_nw_get_endc_status(&e);
 if(rr){ql_nw_release();return 3;}
 blob_buf_init(&out,0);S("source","qlril-ubus");N("UPDATED",time(NULL));S("RAT",rat(r.radio_tech));S("RAT_RAW",rat(r.radio_tech));
 int registered=r.reg_state==2||r.reg_state==6||r.reg_state==9||r.reg_state==10;
 S("REGISTRATION",registered?(r.roaming?"roaming":"registered"):r.reg_state==3?"searching":r.reg_state==4?"denied":"not registered");S("REGISTERED",registered?"YES":"NO");N("registration_state",r.reg_state);
 if(!orr)fixedstr("OPERATOR",o.long_eons,sizeof(o.long_eons));
 textnum("TAC",r.tac);textnum("CELLID",r.cid);
 unsigned band=!br?(r.radio_tech==18?b.nr_band:b.lte_band):0;
 if(band){char s[20];snprintf(s,sizeof(s),"%s%u",r.radio_tech==18?"n":"B",band);S("BAND",s);}
 ql_nw_get_band_mode_info_t bands={.get_band_option=0};
 int bands_result=ql_nw_get_band_mode(&bands);
 if(!bands_result){
  char lte[65],nr[257];
  for(int i=0;i<8;i++)snprintf(lte+i*8,9,"%08x",bands.lte_band_class[i]);
  for(int i=0;i<32;i++)snprintf(nr+i*8,9,"%08x",bands.nr_band_class[i]);
  void *t=blobmsg_open_table(&out,"band_lock");S("lte_mask",lte);S("nr_mask",nr);blobmsg_close_table(&out,t);
 }
 void *arr=blobmsg_open_array(&out,"cells");
 if(!cr){
  for(unsigned i=0;c.nr_info_valid&&i<c.nr_info_len&&i<20;i++){ql_nw_nr_info_t *n=&c.nr_info[i];unsigned cb=!car?carrier_band(&ca,1,n->pci,n->nr_arfcn):0;int serving=cb||(n->cid==r.cid&&r.cid!=0);cell(1,serving,n->cid,n->pci,n->nr_arfcn,n->tac,n->plmn,n->rsrp,n->rsrq,n->sinr,n->rssi,cb?cb:(!br&&serving?b.nr_band:0));}
  for(unsigned i=0;c.lte_info_valid&&i<c.lte_info_len&&i<20;i++){ql_nw_lte_info_t *l=&c.lte_info[i];unsigned cb=!car?carrier_band(&ca,0,l->pci,l->earfcn):0;int serving=cb||(l->cid==r.cid&&r.cid!=0);cell(0,serving,l->cid,l->pci,l->earfcn,l->tac,l->plmn,l->rsrp,l->rsrq,l->sinr/10,l->rssi,cb?cb:(!br&&serving?b.lte_band:0));}
 }
 blobmsg_close_array(&out,arr);
 if(!cr&&registered){
  for(unsigned i=0;c.nr_info_valid&&i<c.nr_info_len&&i<20;i++)if(c.nr_info[i].cid==r.cid&&r.radio_tech==18){ql_nw_nr_info_t *n=&c.nr_info[i];textnum("PCI",n->pci);textnum("ARFCN",n->nr_arfcn);textnum("RSRP",n->rsrp);textnum("RSRQ",n->rsrq);textnum("SINR",n->sinr);textnum("SIGNAL",n->rsrp);}
  for(unsigned i=0;c.lte_info_valid&&i<c.lte_info_len&&i<20;i++)if(c.lte_info[i].cid==r.cid&&r.radio_tech!=18){ql_nw_lte_info_t *l=&c.lte_info[i];textnum("PCI",l->pci);textnum("ARFCN",l->earfcn);textnum("RSRP",l->rsrp);textnum("RSRQ",l->rsrq);textnum("SINR",l->sinr/10);textnum("SIGNAL",l->rsrp);}
 }
 arr=blobmsg_open_array(&out,"carriers");
 if(!car){if(ca.lte_pcc_num)carrier(0,1,&ca.lte_pcc_ca_info,NULL);for(unsigned i=0;i<ca.lte_scc_num&&i<8;i++)carrier(0,0,NULL,&ca.lte_scc_ca_info[i]);if(ca.nr_pcc_num)carrier(1,1,&ca.nr_pcc_ca_info,NULL);for(unsigned i=0;i<ca.nr_scc_num&&i<8;i++)carrier(1,0,NULL,&ca.nr_scc_ca_info[i]);}
 blobmsg_close_array(&out,arr);
 void *t=blobmsg_open_table(&out,"ca");B("valid",!car);if(!car){unsigned la=0,na=0;for(unsigned i=0;i<ca.lte_scc_num&&i<8;i++)la+=ca.lte_scc_ca_info[i].scc_state==2;for(unsigned i=0;i<ca.nr_scc_num&&i<8;i++)na+=ca.nr_scc_ca_info[i].scc_state==2;B("lte_active",la>0);B("nr_active",na>0);N("lte_active_scc",la);N("nr_active_scc",na);N("lte_configured_scc",ca.lte_scc_num);N("nr_configured_scc",ca.nr_scc_num);}blobmsg_close_table(&out,t);
 t=blobmsg_open_table(&out,"endc");B("valid",!er);if(!er){B("available",e.endc_available);B("restricted",e.dcnr_restricted);}B("nsa_registered",r.radio_tech==19&&registered);S("note","EN-DC availability is not proof of an active NR secondary connection");blobmsg_close_table(&out,t);
 arr=blobmsg_open_array(&out,"measurements");
 if(!mr){if(m.lte_pcc_num)measure(0,1,0,&m.lte_pcc_info,NULL);for(unsigned i=0;i<m.lte_scc_num&&i<8;i++)measure(0,0,i,&m.lte_scc_info[i],NULL);if(m.nr_pcc_num)measure(1,1,0,NULL,&m.nr_pcc_info);for(unsigned i=0;i<m.nr_scc_num&&i<8;i++)measure(1,0,i,NULL,&m.nr_scc_info[i]);}
 blobmsg_close_array(&out,arr);
 t=blobmsg_open_table(&out,"api_errors");N("registration",rr);N("cells",cr);N("band",br);N("operator",orr);N("ca",car);N("measurements",mr);N("endc",er);N("band_lock",bands_result);blobmsg_close_table(&out,t);
 if(!ql_sim_init(1)){struct imsi_resp im={0};char iccid[32]={0};int ir=ql_sim_get_imsi(0xb01,&im);if(!ir)fixedstr("IMSI",im.imsi,sizeof(im.imsi));if(!ql_sim_get_iccid(0xb01,iccid,sizeof(iccid)))fixedstr("ICCID",iccid,sizeof(iccid));S("SIM",!ir?"READY":"unknown");ql_sim_release();}
 if(!ql_dm_init(1)){struct serial_resp sn={0};char radio[32]={0};if(!ql_dm_get_device_serial_numbers(0xb01,&sn)&&sn.imei_valid)fixedstr("IMEI",sn.imei,sizeof(sn.imei));if(!ql_dm_get_modem_state(radio,sizeof(radio)))fixedstr("RADIO",radio,sizeof(radio));ql_dm_release();}
 temperature();
 char *json=blobmsg_format_json(out.head,true);if(!json)return 4;puts(json);free(json);blob_buf_free(&out);ql_nw_release();return 0;
}
