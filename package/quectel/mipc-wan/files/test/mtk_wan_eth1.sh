#!/bin/sh

uci set network.wan.device='eth1'
uci set network.wan6.device='eth1'

# Remove MIPC settings
uci -q delete network.wan.apn
uci -q delete network.wan.iptype
uci -q delete network.wan.plmn
uci -q delete network.wan.ipaddr
uci -q delete network.wan.netmask
uci -q delete network.wan.gateway

if [ $# -eq 0 ]
then
	uci set network.wan.proto='dhcp'
else
	ipaddr=$1
	netmask=24
	if [ $# -eq 2 ]
	then
		netmask=$2
	fi
	eval "$(ipcalc.sh $ipaddr $netmask)"

	gw=$(echo $ipaddr | awk -F"." '{print $1"."$2"."$3"."254}')

	uci set network.wan.proto='static'
	uci set network.wan.ipaddr="$ipaddr"
	uci set network.wan.netmask="$NETMASK"
	uci set network.wan.gateway="$gw"
fi

# Remove eth1 from LAN setion
#lan_ifnames=$(uci get network.lan.device)
lan_ifnames=$(uci get network.@device[0].ports)
lan_ifnames=`echo ${lan_ifnames} | sed -e s/eth1// -e s/^[[:space:]]*//`
uci set network.@device[0].ports="$lan_ifnames"

uci commit
/etc/init.d/network restart > /dev/null 2>&1

echo -e "\nConfiguration Done\nPleaes start to run\n"
uci show network.wan
