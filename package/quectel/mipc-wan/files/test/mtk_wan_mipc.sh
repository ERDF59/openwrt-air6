#!/bin/sh

. /usr/share/libubox/jshn.sh

json_load "$(mipc_wan_cli --apn_provision_by_sim)"
json_get_var apn apn
[ -z "$apn" ] && return

json_get_var auth_type auth_type
json_get_var user user
json_get_var password password
json_get_var protocol protocol
json_get_var iaapn iaapn
json_get_var rattype rattype
json_get_var plmn plmn

# Remove ethernet UCI settings
uci -q delete network.wan.ipaddr
uci -q delete network.wan.netmask
uci -q delete network.wan.gateway

uci set network.wan.proto='mipc'
uci set network.wan.device='ccmni'
uci set network.wan6.device='ccmni'
uci set network.wan.apn="$apn"

[ -z "$auth_type" ] || uci set network.wan.auth="$auth_type"
[ -z "$user" ] || uci set network.wan.username="$user"
[ -z "$password" ] || uci set network.wan.password="$password"
[ -z "$protocol" ] || uci set network.wan.iptype="$protocol"
[ -z "$iapan" ] || uci set network.wan.iaapn="$iaapn"
[ -z "$rattype" ] || uci set network.wan.rattype="$rattype"
[ -z "$plmn" ] || uci set network.wan.plmn="$plmn"

uci commit network

/etc/init.d/network restart > /dev/null 2>&1

echo -e "\nConfiguration Done\nPleaes start to run\n"
uci show network.wan
