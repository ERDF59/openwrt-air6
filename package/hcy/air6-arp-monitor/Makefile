include $(TOPDIR)/rules.mk

PKG_NAME:=air6-arp-monitor
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/air6-arp-monitor
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Quectel ARP monitor for Air6
  DEPENDS:=+air6-modem-chain-only
endef

define Package/air6-arp-monitor/description
 Keeps LAN ARP/neighbour entries from being cleared. Active only when
 /mnt/vendor/nvcfg/allow_arp_sync exists, matching the vendor behaviour.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -I$(PKG_BUILD_DIR)/inc \
		-o $(PKG_BUILD_DIR)/arp_monitor $(PKG_BUILD_DIR)/arp_monitor.c \
		$(TARGET_LDFLAGS)
endef

define Package/air6-arp-monitor/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/arp_monitor $(1)/usr/bin/arp_monitor
	$(INSTALL_BIN) ./files/arp_monitor.init $(1)/etc/init.d/arp_monitor
endef

$(eval $(call BuildPackage,air6-arp-monitor))
