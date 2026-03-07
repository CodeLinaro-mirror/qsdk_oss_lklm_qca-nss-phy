/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#include <linux/module.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/phy.h>
#include "mdio-ahb.h"

/* Private data for the MDIO-AHB bus */
struct mdio_ahb_priv {
	void __iomem *base;
	size_t size;
	struct device *dev;
};

/* MMD to AHB address mapping offsets based on hardware specification
 * Each MMD region is 256KB (0x40000 bytes) apart in the address space
 * Registers within each region are 32-bit aligned (4 bytes per register)
 */
#define AHB_PHY_MMD_REGION_SIZE			0x40000  /* 256KB per MMD region */
#define AHB_PHY_MMD1_BASE			0x00000  /* MMD1 (PMAPMD) base */
#define AHB_PHY_MMD3_BASE			0x40000  /* MMD3 (PCS) base */
#define AHB_PHY_MMD7_BASE			0x80000  /* MMD7 (AN) base */
#define AHB_PHY_MMD31_BASE			0xC0000  /* MMD31 (Vendor) base */
#define AHB_PHY_REG_ALIGNMENT_SHIFT		2        /* 32-bit alignment (4 bytes) */

/**
 * mmd_to_ahb_addr_convert() - Convert MMD device address to AHB offset
 * @bus: MDIO bus
 * @phy_addr: PHY address
 * @devad: MDIO MMD device address
 * @reg: Register offset within the MMD
 * @ahb_addr: Output parameter for the calculated AHB address
 *
 * This function maps MDIO MMD device addresses to their corresponding
 * AHB base addresses according to the hardware specification.
 *
 * Return: 0 on success, -EOPNOTSUPP for unsupported operations, -EINVAL for out of bounds
 */
static int mmd_to_ahb_addr_convert(struct mii_bus *bus, int phy_addr, int devad, u32 reg, u32 *ahb_addr)
{
	struct mdio_ahb_priv *priv = NULL;
	u32 ahb_base_addr = 0, offset = 0;

	if (!bus || !bus->priv)
		return -EINVAL;

	priv = bus->priv;

	/* Only support the specific PHY address for built-in PHY */
	if (phy_addr != MDIO_AHB_PHY_ADDR)
		return -EOPNOTSUPP;

	switch (devad) {
	case MDIO_MMD_PMAPMD:
		ahb_base_addr = AHB_PHY_MMD1_BASE;
		break;
	case MDIO_MMD_PCS:
		ahb_base_addr = AHB_PHY_MMD3_BASE;
		break;
	case MDIO_MMD_AN:
		ahb_base_addr = AHB_PHY_MMD7_BASE;
		break;
	case MDIO_MMD_VEND2:
		ahb_base_addr = AHB_PHY_MMD31_BASE;
		break;
	default:
		/* Unsupported MMD device */
		return -EOPNOTSUPP;
	}

	/* Calculate register offset with 32-bit alignment */
	offset = (reg & 0xFFFF) << AHB_PHY_REG_ALIGNMENT_SHIFT;

	/* Bounds checking: ensure no overflow and access is within mapped region */
	if (offset >= priv->size ||
	    ahb_base_addr >= priv->size ||
	    priv->size - ahb_base_addr < offset ||
	    priv->size - ahb_base_addr - offset < sizeof(u32))
		return -EINVAL;

	/* Calculate final AHB address */
	*ahb_addr = ahb_base_addr + offset;

	return 0;
}

/**
 * mdio_ahb_read_c45() - Read from PHY register via AHB interface
 * @bus: MDIO bus
 * @phy_addr: PHY address
 * @devad: MDIO MMD device address
 * @reg: Register offset
 *
 * Return: Register value on success, negative error code on failure
 */
static int mdio_ahb_read_c45(struct mii_bus *bus, int phy_addr, int devad, int reg)
{
	struct mdio_ahb_priv *priv;
	u32 ahb_addr;
	u32 val;
	int ret;

	if (!bus || !bus->priv)
		return -EINVAL;

	priv = bus->priv;

	if (!priv->base)
		return -EINVAL;

	ret = mmd_to_ahb_addr_convert(bus, phy_addr, devad, reg, &ahb_addr);
	/* Return 0xFFFF for unsupported PHY addresses/MMD devices to indicate no device present */
	if (ret == -EOPNOTSUPP)
		return 0xFFFF;
	/* Return error for other failures (e.g., out of bounds access) */
	if (ret < 0)
		return ret;

	/* Read 32-bit value from AHB-mapped register */
	val = readl(priv->base + ahb_addr);

	/* MDIO registers are 16-bit, return the lower 16 bits */
	return val & 0xFFFF;
}

/**
 * mdio_ahb_write_c45() - Write to PHY register via AHB interface
 * @bus: MDIO bus
 * @phy_addr: PHY address
 * @devad: MDIO MMD device address
 * @reg: Register offset
 * @val: Value to write
 *
 * Return: 0 on success, negative error code on failure
 */
static int mdio_ahb_write_c45(struct mii_bus *bus, int phy_addr, int devad, int reg, u16 val)
{
	struct mdio_ahb_priv *priv;
	u32 ahb_addr;
	int ret;

	if (!bus || !bus->priv)
		return -EINVAL;

	priv = bus->priv;

	if (!priv->base)
		return -EINVAL;

	ret = mmd_to_ahb_addr_convert(bus, phy_addr, devad, reg, &ahb_addr);
	/* If the return value is -EOPNOTSUPP, do not treat it as an error.
	 * Return success (0) to avoid breaking normal kernel flow.
	 */
	if (ret == -EOPNOTSUPP)
		return 0;
	/* Return error for other failures (e.g., out of bounds access) */
	if (ret < 0)
		return ret;

	/* Write 16-bit value to a 32-bit AHB-mapped register */
	writel(val, priv->base + ahb_addr);

	return 0;
}

/**
 * mdio_ahb_iounmap() - Unmap the AHB register space
 * @bus: The MDIO-AHB bus to clean up
 *
 * This function unmaps the I/O memory region associated with the MDIO-AHB bus.
 */
static void mdio_ahb_iounmap(struct mii_bus *bus)
{
	struct mdio_ahb_priv *priv;

	if (!bus || !bus->priv)
		return;

	priv = bus->priv;
	if (priv->base) {
		iounmap(priv->base);
		priv->base = NULL;
	}
}

/**
 * mdio_ahb_bus_register() - Allocate and register an MDIO-AHB bus
 * @base: Physical base address of the AHB PHY register space
 * @size: Size of the AHB PHY register space
 * @parent: Parent device for the MDIO bus
 *
 * This function allocates, initializes and registers an MDIO bus that
 * translates MDIO C45 operations to AHB memory-mapped register accesses.
 *
 * Return: Pointer to registered mii_bus on success, ERR_PTR on failure
 */
static struct mii_bus *mdio_ahb_bus_register(phys_addr_t base, size_t size, struct device *parent)
{
	struct mii_bus *bus;
	struct mdio_ahb_priv *priv;
	int ret;

	if (!size || !base)
		return ERR_PTR(-EINVAL);

	/* Allocate MDIO bus with private data */
	bus = mdiobus_alloc_size(sizeof(*priv));
	if (!bus)
		return ERR_PTR(-ENOMEM);

	priv = bus->priv;

	/* Map the AHB register space */
	priv->base = ioremap(base, size);
	if (!priv->base) {
		ret = -ENOMEM;
		goto err_free_bus;
	}

	priv->size = size;
	priv->dev = parent;

	/* Configure MDIO bus */
	bus->name = MDIO_AHB_BUS_NAME;
	bus->read_c45 = mdio_ahb_read_c45;
	bus->write_c45 = mdio_ahb_write_c45;
	bus->parent = parent;

	snprintf(bus->id, MII_BUS_ID_SIZE, MDIO_AHB_BUS_NAME);

	/* Register MDIO bus */
	ret = mdiobus_register(bus);
	if (ret) {
		goto err_iounmap;
	}

	return bus;

err_iounmap:
	mdio_ahb_iounmap(bus);
err_free_bus:
	mdiobus_free(bus);
	return ERR_PTR(ret);
}

/* IPQ52xx PHY SYS clock/reset registers */
#define IPQ52XX_PHY_SYS_CLK_REG					0x182A004
#define IPQ52XX_PHY_SYS_CLK_EN					BIT(0)

#define GCC_PCNOC_BFDCD_CMD_RCGR				0x1831004
#define GCC_MDIO_GEPHY_AHB_CBCR					0x1817098

static int mdio_ahb_clk_init(struct device *dev)
{
	void __iomem *pcnoc_bfdcd;
	void __iomem *mdio_gephy_ahb;
	u32 val;

	/* Map 8 bytes to cover CMD (offset 0) and CFG (offset 4) registers */
	pcnoc_bfdcd = ioremap(GCC_PCNOC_BFDCD_CMD_RCGR, 8);
	if (!pcnoc_bfdcd)
		return -ENOMEM;

	/* Configure RCG to 100MHz (0x10f) */
	writel(0x10f, pcnoc_bfdcd + 4);

	/* Update CMD register */
	val = readl(pcnoc_bfdcd);
	val |= BIT(0);
	writel(val, pcnoc_bfdcd);

	iounmap(pcnoc_bfdcd);

	mdio_gephy_ahb = ioremap(GCC_MDIO_GEPHY_AHB_CBCR, 4);
	if (!mdio_gephy_ahb)
		return -ENOMEM;

	/* Enable MDIO AHB clock branch */
	val = readl(mdio_gephy_ahb);
	val |= BIT(0);
	writel(val, mdio_gephy_ahb);

	iounmap(mdio_gephy_ahb);

	dev_info(dev, "MDIO AHB clock init successfully\n");
	return 0;
}

static int ipq52xx_phy_sys_clk_enable(void)
{
	void __iomem *reg;
	u32 val;

	reg = ioremap(IPQ52XX_PHY_SYS_CLK_REG, 4);
	if (!reg)
		return -ENOMEM;

	val = readl(reg);
	val |= IPQ52XX_PHY_SYS_CLK_EN;
	writel(val, reg);

	iounmap(reg);

	return 0;
}

/**
 * mdio_ahb_probe() - Platform driver probe function
 * @pdev: Platform device
 *
 * This function is called when the platform device is registered.
 * It allocates and registers the MDIO-AHB bus.
 *
 * Return: 0 on success, negative error code on failure
 */
static int mdio_ahb_probe(struct platform_device *pdev)
{
	struct mii_bus *bus;
	struct resource *res;
	phys_addr_t base_addr;
	resource_size_t reg_size;

	/* init the clock for mdio ahb bus */
	mdio_ahb_clk_init(&pdev->dev);
	/* will check  to enable it in clock driver */
	ipq52xx_phy_sys_clk_enable();
	/* Get memory resource from device tree */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Failed to get memory resource\n");
		return -ENODEV;
	}

	base_addr = res->start;
	reg_size = resource_size(res);

	dev_info(&pdev->dev, "Probing MDIO-AHB bus at 0x%llx, size 0x%llx\n",
		 (unsigned long long)base_addr, (unsigned long long)reg_size);

	/* Allocate and register MDIO-AHB bus */
	bus = mdio_ahb_bus_register(base_addr, reg_size, &pdev->dev);
	if (IS_ERR(bus)) {
		dev_err(&pdev->dev, "Failed to register MDIO-AHB bus: %ld\n", PTR_ERR(bus));
		return PTR_ERR(bus);
	}

	/* Store bus pointer in platform device data */
	platform_set_drvdata(pdev, bus);

	dev_info(&pdev->dev, "MDIO-AHB bus registered successfully\n");

	return 0;
}

/**
 * mdio_ahb_remove() - Platform driver remove function
 * @pdev: Platform device
 *
 * This function is called when the platform device is removed.
 * It unregisters and frees the MDIO-AHB bus.
 *
 * Return: 0 on success
 */
static int mdio_ahb_remove(struct platform_device *pdev)
{
	struct mii_bus *bus = platform_get_drvdata(pdev);

	if (!bus)
		return 0;

	/* Unregister and free the bus */
	mdiobus_unregister(bus);
	mdio_ahb_iounmap(bus);
	mdiobus_free(bus);

	dev_info(&pdev->dev, "MDIO-AHB bus removed\n");

	return 0;
}

/* Device tree match table */
static const struct of_device_id mdio_ahb_of_match[] = {
	{ .compatible = "qcom,mdio-ahb-ipq52xx" },
	{ }
};
MODULE_DEVICE_TABLE(of, mdio_ahb_of_match);

/* Platform driver structure */
static struct platform_driver mdio_ahb_driver = {
	.probe = mdio_ahb_probe,
	.remove = mdio_ahb_remove,
	.driver = {
		.name = "mdio-ahb",
		.of_match_table = mdio_ahb_of_match,
	},
};

module_platform_driver(mdio_ahb_driver);

MODULE_DESCRIPTION("mdio ahb bus driver");
MODULE_LICENSE("Dual BSD/GPL");
