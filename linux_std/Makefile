obj-y := qca8084/ ptp/ clock/ macsec/ mdio_ahb/
#
# PHY objects
#
obj-m += qcom-lnx-phy.o

qcom-lnx-phy-objs := qca81xx/qca81xx.o qce1204/qce1204.o

ifdef CONFIG_HWMON
qcom-lnx-phy-objs += qca81xx/qca81xx_hwmon.o qce1204/qce1204_hwmon.o
endif
