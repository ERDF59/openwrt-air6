# SPDX-License-Identifier: GPL-2.0-only
#
# MediaTek MT6990 / T830 — SoftBank Air 6 (A775)

define Build/air6-android-boot
	python3 $(TOPDIR)/scripts/mkbootimg_v2.py \
		--meta $(TOPDIR)/target/linux/mediatek/image/air6-boot-v2.json \
		--kernel $(KDIR)/Image \
		--dtb $(KDIR)/image-mt6990-softbank-air6.dtb \
		--out $@
endef

define Build/air6-copy-rootfs
	cp -f $@ $(BIN_DIR)/rootfs.img
endef

define Device/softbank_air6
  DEVICE_VENDOR := SoftBank
  DEVICE_MODEL := Air 6
  SOC := mt6990
  KERNEL_LOADADDR := 0x80080000
  KERNEL_ENTRY := 0x80080000
  DEVICE_DTS := mt6990-softbank-air6
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := kmod-cfg80211 libf2fs mkf2fs f2fsck
  IMAGES += boot.img rootfs.img
  IMAGE/boot.img := air6-android-boot
  IMAGE/rootfs.img := append-rootfs | air6-copy-rootfs
endef
TARGET_DEVICES += softbank_air6
