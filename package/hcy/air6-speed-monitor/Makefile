include $(TOPDIR)/rules.mk

PKG_NAME:=air6-speed-monitor
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
QL_LIBDIR:=$(TOPDIR)/package/vendor/air6-modem-chain-only/files/usr/lib

include $(INCLUDE_DIR)/package.mk

define Package/air6-speed-monitor
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Quectel per-client speed query for Air6
  DEPENDS:=+air6-modem-chain-only
endef

define Package/air6-speed-monitor/description
 Queries HNAT MIB counters and prints per-LAN-client tx/rx rates.
 Usage: speed_monitor -a [mac] | -A
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -I$(PKG_BUILD_DIR)/inc \
		-o $(PKG_BUILD_DIR)/speed_monitor $(PKG_BUILD_DIR)/speed_monitor.c \
		$(TARGET_LDFLAGS) -L$(QL_LIBDIR) -Wl,-rpath-link,$(QL_LIBDIR) \
		-lql_speed_monitor
endef

define Package/air6-speed-monitor/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/speed_monitor $(1)/usr/bin/speed_monitor
endef

$(eval $(call BuildPackage,air6-speed-monitor))
