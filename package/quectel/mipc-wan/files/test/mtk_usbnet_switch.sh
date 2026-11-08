#!/bin/sh

usage()
{
	echo "Usage: mtk_usbnet_switch.sh [protocol/stop]"
	echo "protocol : rndis"
	echo "           ecm"
	echo "           ncm"
	echo :stop
}

if [ ! "$#" -eq 1 ]; then
	usage
	exit 0
fi

init_script_path="/etc/init.d"
usb_init_script="$init_script_path/usb.init"
brName="br-lan"
linkPathPrefix="ln -sf /config/usb_gadget/g1/functions"
linkPath="$linkPathPrefix/"

find_exist_usbNet_if()
{
	curIfName=`ifconfig |grep rndis|awk '{print $1}'`
	if [ "$curIfName" != "" ]; then
		echo "$curIfName"
		return
	fi

	curIfName=`ifconfig |grep ecm|awk '{print $1}'`
	if [ "$curIfName" != "" ]; then
		echo "$curIfName"
		return
	fi

	curIfName=`ifconfig |grep ncm|awk '{print $1}'`
	if [ "$curIfName" != "" ]; then
		echo "$curIfName"
 		return
	fi

	if [ "$curIfName" = "" ]; then
		echo "no exist usb net interface"
 	fi
}

curIfName=$( find_exist_usbNet_if )

usbnet_if_down()
{
	if [ "$curIfName" != "" ]; then
		ifconfig "$curIfName" down
		brctl delif "$brName" "$curIfName"
	fi
}

usbnet_if_up()
{
	initUsbIf=`ifconfig -a |grep $protocol|awk '{print $1}'`
	if [ "$initUsbIf" != "" ]; then
		ifconfig "$initUsbIf" up
		brctl addif "$brName" "$initUsbIf"
	fi
}

comment_unuse_link()
{
#comment all first
	rndisLinkLine=`grep -n "#$linkPathPrefix/rndis" "$usb_init_script"|awk -F":" '{print $1}'`
	if [ "$rndisLinkLine" = "" ]; then
		tmpLine=`grep -n "$linkPathPrefix/rndis" "$usb_init_script"|awk -F":" '{print $1}'`
		sed -i ''"$tmpLine"'s/ln/#ln/g' "$usb_init_script"
	fi

	ecmLinkLine=`grep -n "#$linkPathPrefix/ecm" "$usb_init_script"|awk -F":" '{print $1}'`
	if [ "$ecmLinkLine" = "" ]; then
		tmpLine=`grep -n "$linkPathPrefix/ecm" "$usb_init_script"|awk -F":" '{print $1}'`
		sed -i ''"$tmpLine"'s/ln/#ln/g' "$usb_init_script"
	fi

	ncmLinkLine=`grep -n "#$linkPathPrefix/ncm" "$usb_init_script"|awk -F":" '{print $1}'`
	if [ "$ncmLinkLine" = "" ]; then
		tmpLine=`grep -n "$linkPathPrefix/ncm" "$usb_init_script"|awk -F":" '{print $1}'`
		sed -i ''"$tmpLine"'s/ln/#ln/g' "$usb_init_script"
	fi

#uncomment usb net link command
	tmpLine=`grep -n "$linkPathPrefix/$protocol" "$usb_init_script"|awk -F":" '{print $1}'`
	sed -i ''"$tmpLine"'s/#ln/ln/g' "$usb_init_script"
}

usbnet_restart()
{
	$usb_init_script stop
	comment_unuse_link
	usbnet_if_down
	$usb_init_script start
	echo "waiting for USBnet interface ready..."
	sleep 3
	usbnet_if_up
}

case ${1} in
	"rndis")
		protocol="rndis"
		usbnet_restart
	;;
	"ecm")
		protocol="ecm"
		usbnet_restart
	;;
	"ncm")
 		protocol="ncm"
		usbnet_restart
	;;
	"stop")
		echo "stop usb net ..."
		usbnet_if_down
		$usb_init_script stop
	;;
	*)
		usage
	;;
esac


