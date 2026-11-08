include $(TOPDIR)/rules.mk

PKG_NAME:=mipc-wan
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/mipc-wan
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Control utility for MTK/Quectel MIPC modems
	DEPENDS:=+mipc +libubus +libubox +libuci +libjson-c +libpthread
endef

define Package/mipc-wan/description
	mipc_wan_cli is a command line tool for MTK/Quectel MIPC modems.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

MIPC_INC:=-I$(STAGING_DIR)/usr/include/mipc -I$(STAGING_DIR)/usr/include/mipc/common -I$(STAGING_DIR)/usr/include/mipc/msg -I$(STAGING_DIR)/usr/include/mipc/external

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(MIPC_INC) -I$(PKG_BUILD_DIR) -I$(PKG_BUILD_DIR)/inc -D_LINUX -D_GNU_SOURCE -Wall $(PKG_BUILD_DIR)/mipc-wan-cli.c $(PKG_BUILD_DIR)/mipc-wan-netlink.c $(PKG_BUILD_DIR)/at_stub.c -o $(PKG_BUILD_DIR)/mipc_wan_cli $(TARGET_LDFLAGS) -L$(STAGING_DIR)/usr/lib -Wl,-rpath-link=$(STAGING_DIR)/usr/lib -lmipc_msg -lubus -lubox -luci -ljson-c -lpthread -ldl
endef

define Package/mipc-wan/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mipc_wan_cli $(1)/usr/bin/
endef

$(eval $(call BuildPackage,mipc-wan))
