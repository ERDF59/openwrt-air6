#!/bin/sh
# stage2 copies the verifier and its runtime before detaching the installed root.
RAMFS_COPY_BIN="$RAMFS_COPY_BIN /usr/bin/head /usr/bin/ucode /usr/bin/usign /usr/bin/sha256sum /usr/sbin/air6-ui-slotctl /usr/lib/ucode/fs.so"
RAMFS_COPY_DATA="$RAMFS_COPY_DATA /usr/share/air6-cpe/update.uc /etc/air6-cpe/update.pub"
