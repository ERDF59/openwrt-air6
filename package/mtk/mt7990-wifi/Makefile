#
# MT7990 vendor Wi-Fi driver, built from source
#
include $(TOPDIR)/rules.mk

PKG_NAME:=kmod-mt7990-wifi
PKG_VERSION:=8.2.1.5
PKG_RELEASE:=1

PKG_LICENSE:=GPL-2.0
PKG_MAINTAINER:=Air6 bring-up

include $(INCLUDE_DIR)/package.mk

define Package/kmod-mt7990-wifi
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=MT7990 vendor Wi-Fi driver (mt_wifi/hwifi/warp)
  DEPENDS:=+kmod-mac80211 +kmod-mt6990-hwnat
endef

define Package/kmod-mt7990-wifi/description
 MediaTek MT7990 BE19000 vendor Wi-Fi driver stack built from the
 in-tree vendor sources (mt_wifi, wlan_hwifi, warp, mt_wifi_cmn,
 wifi_md_coex).
endef

# coex 必须最先构建：wifi 的 KBUILD_EXTRA_SYMBOLS 引用 coex/Module.symvers，
# 该文件由 coex 构建生成且被 .gitignore 排除，干净树上不存在。
define Build/Compile
	$(foreach p,coex cmn wifi warp hwifi, \
		python3 $(TOPDIR)/scripts/vendor-build-part.py $(p) || exit 1;)
endef

define Package/kmod-mt7990-wifi/install
	$(INSTALL_DIR) $(1)/usr/lib/air6-wifi/modules
	$(CP) $(TOPDIR)/package/mtk/mt7990-wifi/src/mt_wifi_cmn/mt_wifi_cmn.ko $(1)/usr/lib/air6-wifi/modules/
	$(CP) $(TOPDIR)/package/mtk/mt7990-wifi/src/wifi/mt_wifi_ap/mt_wifi.ko $(1)/usr/lib/air6-wifi/modules/
	$(CP) $(TOPDIR)/package/mtk/mt7990-wifi/src/wifi_md_coex/wifi_md_coex.ko $(1)/usr/lib/air6-wifi/modules/
	$(CP) $(TOPDIR)/package/mtk/mt7990-wifi/src/wifi/warp_driver/warp/mtk_warp.ko $(1)/usr/lib/air6-wifi/modules/
	$(CP) $(TOPDIR)/package/mtk/mt7990-wifi/src/wifi/wlan_hwifi/mtk_hwifi.ko $(1)/usr/lib/air6-wifi/modules/
	$(CP) $(TOPDIR)/package/mtk/mt7990-wifi/src/wifi/wlan_hwifi/mtk_pci.ko $(1)/usr/lib/air6-wifi/modules/
	$(CP) $(TOPDIR)/package/mtk/mt7990-wifi/src/wifi/wlan_hwifi/mtk_wed.ko $(1)/usr/lib/air6-wifi/modules/
	$(CP) $(TOPDIR)/package/mtk/mt7990-wifi/src/wifi/wlan_hwifi/connac_if.ko $(1)/usr/lib/air6-wifi/modules/
	$(CP) $(TOPDIR)/package/mtk/mt7990-wifi/src/wifi/wlan_hwifi/mt7990.ko $(1)/usr/lib/air6-wifi/modules/
	$(CP) $(TOPDIR)/package/mtk/mt7990-wifi/src/wifi/wlan_hwifi/mt7991.ko $(1)/usr/lib/air6-wifi/modules/
	$(TARGET_CROSS)strip --strip-debug $(1)/usr/lib/air6-wifi/modules/*.ko
endef

$(eval $(call BuildPackage,kmod-mt7990-wifi))
