GPIOOO_VERSION = 1.0
GPIOOO_SITE = $(TOPDIR)/package/gpiooo
GPIOOO_SITE_METHOD = local
GPIOOO_LICENSE = GPLv3+

GPIOOO_DEPENDENCIES = linux

define GPIOOO_BUILD_CMDS
$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D)
endef

define GPIOOO_INSTALL_TARGET_CMDS
$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D) modules_install
endef

$(eval $(generic-package))
