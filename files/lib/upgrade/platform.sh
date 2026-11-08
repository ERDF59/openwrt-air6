#!/bin/sh
# Only the authenticated AIR6 UI can authorize a signed boot/rootfs bundle.
platform_check_image() {
    [ "$1" = /tmp/air6-update/upload.tar ] || return 74
    local id
    id="$(jsonfilter -i /tmp/air6-update/upload.json -e '@.id')"
    /usr/bin/ucode /usr/share/air6-cpe/update.uc check "$id" || return 74
    return 0
}
platform_do_upgrade() {
    echo "Use AIR6 System update to choose a slot." >&2
    return 1
}
