STICK_VERSION = 1.0
STICK_SITE = $(TOPDIR)/package/stick
STICK_SITE_METHOD = local
STICK_LICENSE = GPLv3+

STICK_DEPENDENCIES = linux

define STICK_BUILD_CMDS
$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D)
endef

define STICK_INSTALL_TARGET_CMDS
$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D) modules_install
endef

$(eval $(generic-package))
