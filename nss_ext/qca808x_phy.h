/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _QCA808X_PHY_H_
#define _QCA808X_PHY_H_

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */
/*PHY debug registers*/
#define QCA8084_PHY_DEBUG_CONTROL_REGISTER0		0x1f
#define QCA8084_PHY_DEBUG_AFE25_CMN_6_MII		0x380
#define QCA8084_PHY_DEBUG_AFE25_CMN_2_MII		0x180

/*PHY MMD1 registers*/
#define QCA808X_PHY_MMD1_FIFO_REST_REG		0x9072

/*PHY MMD7 registers*/
#define QCA808X_PHY_MMD7_LED0_CTRL		0x8078
#define QCA808X_PHY_MMD7_LED1_CTRL		0x8074
#define QCA808X_PHY_MMD7_LED2_CTRL		0x8076
#define QCA808X_PHY_MMD7_LED0_FORCE_CTRL		0x8079
#define QCA808X_PHY_MMD7_LED1_FORCE_CTRL		0x8075
#define QCA808X_PHY_MMD7_LED2_FORCE_CTRL		0x8077

/*PHY debug registers field*/
#define QCA8084_PHY_DEBUG_1588_P2_EN		0x2
#define QCA8084_PHY_DEBUG_AFE25_PLL_EN		0x8000
#define QCA8084_PHY_DEBUG_AFE25_LDO_EN		0x2000

/*PHY MMD1 registers field*/
#define QCA808X_PHY_MMD1_FIFO_RESET		0
#define QCA808X_PHY_MMD1_FIFO_RELEASE		0x800

/*PHY MMD7 registers field*/
#define QCA808X_PHY_MMD7_LINK_2500M_LIGHT_EN		0x8000

/*PHY soc address*/
#define QCA8084_SOC_EPHY_CFG		0xC90F018
#define QCA8084_SOC_EPHY3_ADDR_BOFFSET		15
#define QCA8084_SOC_EPHY2_ADDR_BOFFSET		10
#define QCA8084_SOC_EPHY1_ADDR_BOFFSET		5
#define QCA8084_SOC_EPHY0_ADDR_BOFFSET		0

enum qca808x_phy_addr_offset {
	QCA808X_SERDES_ADDR_OFFSET = 1,
};

int qca808x_phy_ops_init(struct nss_phy_ops *ops);
int qca8084_phy_fixup(struct nss_phy_device *nss_phydev);
#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _qca808x_PHY_H_ */
