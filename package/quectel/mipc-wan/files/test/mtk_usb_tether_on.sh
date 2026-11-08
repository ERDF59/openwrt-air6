#!/bin/sh

get_device_idx() {
    local i=0
    while true; do
        result=$(uci -q get network.@device[$i].name)
        [ -z "$result" ] && return

        if [ "$result" == "br-lan" ]; then
            let device_id=$i
            echo "device_id is $device_id"
            break
        fi

        let i++
    done
}

get_device_idx
# Add rndis0 into LAN section
lan_ifnames=$(uci get network.@device[$device_id].ports)
if [[ "$lan_ifnames" == *"rndis0"* ]];
then
    echo "'rndis0' existed, no need to up again"
    exit 0
fi
uci add_list network.@device[$device_id].ports=rndis0
uci add_list network.@device[$device_id].ports=ecm0
uci commit
sbin/wifi down
ifdown lan
ifup lan
sbin/wifi up

# add wifi to br-lan
for wifi in $(iwpriv | grep Available | grep -Eo '^[a-zA-Z0-9]+');
do
    if [ $(ifconfig | grep -c $wifi ) == 1 ] ; then
        brctl addif br-lan $wifi
    fi
done

echo -e "\nConfigure usb tethering on done\n"
uci show network.@device[$device_id]
