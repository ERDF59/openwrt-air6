#!/bin/sh
# Generic MT6990 platform upgrade hooks (air6 images are written via fastboot
# or to the inactive slot; no vendor-specific sysupgrade handling).
. /lib/functions.sh

platform_check_image() {
	echo "SoftBank Air 6: LuCI sysupgrade is not supported." >&2
	echo "Flash the signed boot and rootfs with the MTK SP Flash Tool (or USB)." >&2
	return 1
}

platform_do_upgrade() {
	platform_check_image "$1" || return 1
}
