#!/bin/sh

file="/tmp/netdiag.log"
if [ -d "/data/debuglog" ] ; then
    file="/data/debuglog/netdiag.log"
fi

run_cmd() {
    echo -e "\n----[$@]---- \n" >> $file
    eval "$@" >> $file
}

echo "" > $file

# Dump current date time
run_cmd "date"
# Show versoin & system running time
run_cmd "cat /etc/vendor_info"
run_cmd "cat /etc/openwrt_release"
run_cmd "uname -a"
run_cmd "uptime"

# Check WAN network availability
run_cmd "ping -c 4 8.8.8.8"
run_cmd "ping -c 4 2001:4860:4860::8888"

# Dump OpenWRT UCI configuration
run_cmd "cat /etc/config/network"
run_cmd "cat /etc/config/system"
run_cmd "cat /etc/config/dhcp"
run_cmd "cat /etc/config/firewall"

run_cmd "ubus call service list"
run_cmd "ubus call network.interface dump"
run_cmd "cat /tmp/dhcp.leases"
run_cmd "cat /tmp/resolv.conf"
run_cmd "fw4 print"
run_cmd "nft list ruleset"
run_cmd "ip route list table all"
run_cmd "ip -6 route list table all"
run_cmd "ip link"
run_cmd "ip addr"
run_cmd "ip -6 addr"
run_cmd "ip link"
run_cmd "ifconfig"
run_cmd "brctl show"
run_cmd "cat /proc/net/nf_conntrack"

# Modem
run_cmd "cat /sys/kernel/ccci/boot"
run_cmd "cat /proc/ccmni/*"

# Wi-Fi
run_cmd "lspci"
run_cmd "iwconfig 2>&1 > /dev/null"
run_cmd "cat /etc/wireless/l1profile.dat"
[ -f /etc/wireless/mediatek/mt7990.1.dat ] && run_cmd "cat /etc/wireless/mediatek/mt7990.1.dat"
[ -f /etc/wireless/mediatek/mt7990.2.dat ] && run_cmd "cat /etc/wireless/mediatek/mt7990.2.dat"
[ -f /etc/wireless/mediatek/mt7992.1.dat ] && run_cmd "cat /etc/wireless/mediatek/mt7992.1.dat"
[ -f /etc/wireless/mediatek/mt7992.2.dat ] && run_cmd "cat /etc/wireless/mediatek/mt7992.2.dat"

# Ethernet
run_cmd "ethtool eth0"
run_cmd "ethtool eth1"
run_cmd "ethtool -S eth0"
run_cmd "ethtool -S eth1"
run_cmd "cat /proc/mtketh/esw_cnt"
run_cmd "cat /proc/mtketh/esw_cnt"

# Hareware NAT
run_cmd "hwnat -g"
run_cmd "hwnat -f"

echo "adb pull $file"
