#
# LuCI 5G modem management for Air6 (MT6990 / Quectel RG620T)
#
# 移植自 FiberHome AIR6 的 luci-app-5gmodem（同一颗 MT6990 集成模组），
# 适配本仓库的 QuecOpen 固件：
#   * 接口名 modem5g -> wan（本固件用 ql_datacall proto 起 wan）
#   * board_ok 用 mipc_wan_cli 是否存在判断（本固件没有 fiberhome 的 board_name）
#   * 少了 AIR6 私有的 nw_get_signal / show_register_status / sample_nw，
#     信号与注册改走 atcid 的 AT；锁机页去掉（无 sample_nw）
#
# 用 package.mk 而不是 luci.mk：本包要装两个交叉编译出来的小程序，
# luci.mk 的 install 段会自动覆盖自定义 install，装不进去。
#
include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-air6-modem
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_LICENSE:=Apache-2.0
PKG_MAINTAINER:=Air6 bring-up
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  TITLE:=LuCI 5G modem management for Air6 (MT6990 / RG620T)
  DEPENDS:=+rpcd +libuci +libubox +libubus +libblobmsg-json
  PKGARCH:=all
endef

define Package/$(PKG_NAME)/description
Cellular status, SMS, dial-up control and an AT terminal for the built-in
MT6990 modem. Talks to the vendor atcid (127.0.0.1:17171) for AT commands and
to mipc_wan_cli for the modem state that AT cannot report.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) \
		-o $(PKG_BUILD_DIR)/air6-modem-at $(PKG_BUILD_DIR)/air6-at.c
	if [ -e $(STAGING_DIR)/usr/lib/libqlril.so ]; then \
		$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) \
		-L$(STAGING_DIR)/usr/lib -Wl,-rpath-link,$(STAGING_DIR)/usr/lib \
		-o $(PKG_BUILD_DIR)/air6-modem-sms-send $(PKG_BUILD_DIR)/air6-sms-send.c -lqlril; \
	fi
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/air6-modem-at $(1)/usr/bin/air6-modem-at
	[ ! -f $(PKG_BUILD_DIR)/air6-modem-sms-send ] || \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/air6-modem-sms-send $(1)/usr/bin/air6-modem-sms-send

	$(INSTALL_DIR) $(1)/usr/share/rpcd/ucode
	$(INSTALL_DATA) ./root/usr/share/rpcd/ucode/luci.air6modem \
		$(1)/usr/share/rpcd/ucode/luci.air6modem

	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(INSTALL_DATA) ./root/usr/share/rpcd/acl.d/luci-app-air6-modem.json \
		$(1)/usr/share/rpcd/acl.d/luci-app-air6-modem.json

	$(INSTALL_DIR) $(1)/usr/share/luci/menu.d
	$(INSTALL_DATA) ./root/usr/share/luci/menu.d/luci-app-air6-modem.json \
		$(1)/usr/share/luci/menu.d/luci-app-air6-modem.json

	$(INSTALL_DIR) $(1)/etc/init.d $(1)/etc/config
	$(INSTALL_BIN) ./root/etc/init.d/air6-modem-status $(1)/etc/init.d/air6-modem-status
	$(INSTALL_CONF) ./root/etc/config/air6_modem_status $(1)/etc/config/air6_modem_status

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./root/usr/bin/air6-modem-status $(1)/usr/bin/air6-modem-status
	$(INSTALL_BIN) ./root/usr/bin/air6-modem-lock $(1)/usr/bin/air6-modem-lock
	$(INSTALL_BIN) ./root/usr/bin/air6-simslot $(1)/usr/bin/air6-simslot
	$(INSTALL_BIN) ./root/etc/init.d/air6-simslot $(1)/etc/init.d/air6-simslot
	$(INSTALL_CONF) ./root/etc/config/air6_simslot $(1)/etc/config/air6_simslot
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./root/etc/uci-defaults/99-air6-simslot $(1)/etc/uci-defaults/99-air6-simslot

	$(INSTALL_DIR) $(1)/www/luci-static/resources/air6modem
	$(INSTALL_DATA) ./htdocs/luci-static/resources/air6modem/common_v2.js \
		$(1)/www/luci-static/resources/air6modem/common_v2.js
	$(INSTALL_DATA) ./htdocs/luci-static/resources/air6modem/i18n.js \
		$(1)/www/luci-static/resources/air6modem/i18n.js

	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) ./files/usr/lib/lua/luci/i18n/luci-app-air6-modem.en.lmo \
		$(1)/usr/lib/lua/luci/i18n/luci-app-air6-modem.en.lmo

	$(INSTALL_DIR) $(1)/www/luci-static/resources/view/air6modem
	$(INSTALL_DATA) ./htdocs/luci-static/resources/view/air6modem/overview_v10.js \
		$(1)/www/luci-static/resources/view/air6modem/overview_v10.js
	$(INSTALL_DATA) ./htdocs/luci-static/resources/view/air6modem/bands_v3.js \
		$(1)/www/luci-static/resources/view/air6modem/bands_v3.js
	$(INSTALL_DATA) ./htdocs/luci-static/resources/view/air6modem/network_v5.js \
		$(1)/www/luci-static/resources/view/air6modem/network_v5.js
	$(INSTALL_DATA) ./htdocs/luci-static/resources/view/air6modem/sms_v5.js \
		$(1)/www/luci-static/resources/view/air6modem/sms_v5.js
	$(INSTALL_DATA) ./htdocs/luci-static/resources/view/air6modem/terminal_v5.js \
		$(1)/www/luci-static/resources/view/air6modem/terminal_v5.js
	$(INSTALL_DATA) ./htdocs/luci-static/resources/view/air6modem/accel_v1.js \
		$(1)/www/luci-static/resources/view/air6modem/accel_v1.js
	$(INSTALL_DATA) ./htdocs/luci-static/resources/air6modem/unlock_v3.js \
		$(1)/www/luci-static/resources/air6modem/unlock_v3.js
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
[ -n "$${IPKG_INSTROOT}" ] || {
	/etc/init.d/rpcd restart >/dev/null 2>&1
	/etc/init.d/air6-modem-status enable >/dev/null 2>&1
	/etc/init.d/air6-modem-status restart >/dev/null 2>&1
	rm -f /tmp/luci-indexcache
	rm -rf /tmp/luci-modulecache
}
exit 0
endef

define Package/$(PKG_NAME)/conffiles
/etc/config/air6_modem_status
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
