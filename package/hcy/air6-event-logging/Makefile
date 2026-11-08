include $(TOPDIR)/rules.mk

PKG_NAME:=air6-event-logging
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
QL_LIBDIR:=$(TOPDIR)/package/vendor/air6-modem-chain-only/files/usr/lib
MIPC_INC:=$(STAGING_DIR)/usr/include/mipc

include $(INCLUDE_DIR)/package.mk

define Package/air6-event-logging
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Quectel modem event logging for Air6
  DEPENDS:=+air6-modem-chain-only +libjson-c +libuci
endef

define Package/air6-event-logging/description
 Saves modem logs when configured events occur. Disabled until
 system.md_start=1 is set; event list lives in /etc/event_trigger.json.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) \
		-I$(PKG_BUILD_DIR) -I$(MIPC_INC) -I$(MIPC_INC)/msg -I$(MIPC_INC)/common \
		-o $(PKG_BUILD_DIR)/event_trigger_logging \
		$(PKG_BUILD_DIR)/event_trigger_logging.c \
		$(TARGET_LDFLAGS) -L$(STAGING_DIR)/usr/lib -L$(QL_LIBDIR) \
		-Wl,-rpath-link,$(STAGING_DIR)/usr/lib -Wl,-rpath-link,$(QL_LIBDIR) \
		-lmipc_msg -lql_mipc -ljson-c -luci
endef

define Package/air6-event-logging/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d $(1)/usr/share/air6-event-logging
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/event_trigger_logging $(1)/usr/bin/event_trigger_logging
	$(INSTALL_BIN) ./files/event_trigger_logging.init $(1)/etc/init.d/event_trigger_logging
	$(INSTALL_CONF) ./files/event_trigger.json $(1)/etc/event_trigger.json
	$(INSTALL_DATA) ./files/readme $(1)/usr/share/air6-event-logging/readme
endef

$(eval $(call BuildPackage,air6-event-logging))
