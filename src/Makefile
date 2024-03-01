# Target module name
TARGET := qca-nss-phy

QCA_NSS_PHY_INCLUDES = \
		   -I$(SUBDIR)/../include

ccflags-y += $(QCA_NSS_PHY_INCLUDES)

# Kernel module compilation - part 2
ifneq ($(KERNELRELEASE),)

# Kernel module part of makefile
include $(SUBDIR)/target.mk

else
# Kernel Makefile targets - part 1
KERNEL_MAKEOPTS := -C $(LINUX_DIR) \
			M=$(SUBDIR) \
			ARCH="$(ARCH)"
all:
	make $(KERNEL_MAKEOPTS)  modules

clean:
	make -C $(LINUX_DIR) M=$(SUBDIR) clean
endif
