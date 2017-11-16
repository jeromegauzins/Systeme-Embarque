SERVO_VERSION = 1.0
SERVO_SITE = $(TOPDIR)/package/servo
SERVO_SITE_METHOD = local
SERVO_LICENSE = GPLv3+

SERVO_DEPENDENCIES = linux

define SERVO_BUILD_CMDS
$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D)
endef

define SERVO_INSTALL_TARGET_CMDS
$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D) modules_install
endef

$(eval $(generic-package))
