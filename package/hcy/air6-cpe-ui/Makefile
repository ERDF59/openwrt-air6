include $(TOPDIR)/rules.mk

PKG_NAME:=air6-cpe-ui
PKG_VERSION:=1.0.0
PKG_RELEASE:=7

PKG_LICENSE:=MIT
PKG_MAINTAINER:=Air6

include $(INCLUDE_DIR)/package.mk

define Build/Compile
endef

define Package/air6-cpe-ui
  SECTION:=luci
  CATEGORY:=LuCI
  TITLE:=Air6 CPE web interface
  DEPENDS:=+luci-base +luci-app-air6-modem +air6-modem-chain-only +uhttpd +cgi-io +rpcd +rpcd-mod-ucode +rpcd-mod-iwinfo +usign
  PKGARCH:=aarch64_cortex-a55_neon-vfpv4
endef

define Package/air6-cpe-ui/description
  Standalone CPE-style web UI for SoftBank Air 6. Serves the dashboard at /
  while LuCI remains available at /cgi-bin/luci.
endef

define Package/air6-cpe-ui/conffiles
/etc/air6-cpe/home.json
endef

define Package/air6-cpe-ui/install
	$(INSTALL_DIR) $(1)/lib/upgrade
	$(INSTALL_DATA) ./files/lib/upgrade/air6-ui.sh $(1)/lib/upgrade/air6-ui.sh
	$(INSTALL_DIR) $(1)/www/cpe
	$(INSTALL_DATA) ./files/www/cpe.html $(1)/www/cpe.html
	$(INSTALL_DATA) ./files/www/cpe/app.css $(1)/www/cpe/app.css
	$(INSTALL_DATA) ./files/www/cpe/app.js $(1)/www/cpe/app.js
	$(INSTALL_DATA) ./files/www/cpe/vue.global.prod.js $(1)/www/cpe/vue.global.prod.js
	$(INSTALL_DATA) ./files/www/cpe/gridstack-all.js $(1)/www/cpe/gridstack-all.js
	$(INSTALL_DATA) ./files/www/cpe/gridstack.min.css $(1)/www/cpe/gridstack.min.css
	$(INSTALL_DATA) ./files/www/cpe/logo.svg $(1)/www/cpe/logo.svg
	$(INSTALL_DIR) $(1)/usr/share/air6-cpe $(1)/etc/air6-cpe $(1)/usr/sbin
	$(INSTALL_DATA) ./files/usr/share/air6-cpe/update.uc $(1)/usr/share/air6-cpe/update.uc
	$(INSTALL_DATA) ./files/etc/air6-cpe/update.pub $(1)/etc/air6-cpe/update.pub
	$(INSTALL_DATA) ./files/etc/air6-cpe/home.json $(1)/etc/air6-cpe/home.json
	$(INSTALL_BIN) ./files/usr/sbin/air6-ui-slotctl $(1)/usr/sbin/air6-ui-slotctl
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/usr/sbin/air6-ui-band-job $(1)/usr/sbin/air6-ui-band-job
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/etc/uci-defaults/90-air6-cpe-ui $(1)/etc/uci-defaults/90-air6-cpe-ui
	$(INSTALL_DIR) $(1)/usr/share/rpcd/ucode
	$(INSTALL_BIN) ./files/usr/share/rpcd/ucode/air6.cpe $(1)/usr/share/rpcd/ucode/air6.cpe
	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(INSTALL_DATA) ./files/usr/share/rpcd/acl.d/air6-cpe.json $(1)/usr/share/rpcd/acl.d/air6-cpe.json
endef

define Package/air6-cpe-ui/postinst
#!/bin/sh
[ -n "$${IPKG_INSTROOT}" ] || {
	/etc/init.d/rpcd restart >/dev/null 2>&1
	/etc/init.d/uhttpd restart >/dev/null 2>&1
}
endef

$(eval $(call BuildPackage,air6-cpe-ui))
