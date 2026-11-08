include $(TOPDIR)/rules.mk

PKG_NAME:=mipc
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/mipc
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=Quectel MIPC message library
	DEPENDS:=+libpthread
endef

define Package/mipc/description
	Quectel MIPC message library for MTK/Quectel CPE modems.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./include ./src ./Makefile.build ./Makefile.build.openwrt $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) -f Makefile.build CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS) -fPIC -Wno-error=implicit-function-declaration -Wno-error=incompatible-pointer-types" LDFLAGS="$(TARGET_LDFLAGS)" MIPC_CFLAGS="-DENABLE_MIPC_SUPPORT_CHECKER -DENABLE_TRM -DSLEEP_FOR_UNREGISTERED_PROCESS -DPORT_MAP_FILE=mipc_port.def -DWAIT_MD_READY" out/libmipc_msg.so
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/mipc
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/mipc/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/out/libmipc_msg.so $(1)/usr/lib/
endef

define Package/mipc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/out/libmipc_msg.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,mipc))
