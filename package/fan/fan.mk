FAN_VERSION = 1.0
FAN_SITE = $(TOPDIR)/package/fan
FAN_SITE_METHOD = local
FAN_LICENSE = GPLv3+

define FAN_BUILD_CMDS
	CC=$(TARGET_CC) $(MAKE) -C $(@D)
endef

define FAN_INSTALL_TARGET_CMDS
	prefix=$(TARGET_DIR)  $(MAKE) -C $(@D) install
endef

$(eval $(generic-package))
