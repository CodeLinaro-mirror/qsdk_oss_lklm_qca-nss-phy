###################################################
# Makefile for the QCA NSS PHY EXT driver
###################################################
TARGET := qca-nss-phy
obj-m += $(TARGET).o

$(TARGET)-objs := \
	nss_phy_common.o	\
	nss_phy_linux_wrapper.o	\
	nss_phy_linux.o	\
	qca807x_phy.o	\
	nss_phy_c45_common.o	\
	qca81xx_phy.o	\
	qca808x_phy.o	\
	qca8084_pinctrl.o	\
	qca803x_phy.o	\
	qca833x_phy.o

ifeq ($(nss-phy-ptp), y)
$(TARGET)-objs += nss_phy_ptp.o nss_phy_ptp_api.o
ccflags-y += -DNSS_PHY_PTP
endif
