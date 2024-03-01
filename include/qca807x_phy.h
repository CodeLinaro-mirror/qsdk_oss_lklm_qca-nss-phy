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

#ifndef _QCA807X_PHY_H_
#define _QCA807X_PHY_H_

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */
/*mmd registers*/
#define QCA807X_PHY_MMD3_ADDR_8023AZ_TIMER_CTRL		0x804e
#define QCA807X_PHY_MMD3_ADDR_CLD_CTRL5		0x8005
#define QCA807X_PHY_MMD3_ADDR_CLD_CTRL3		0x8003

/*mmd registers field*/
#define QCA807X_PHY_MMD3_DAC_AT_BIAS_EN		0x4000
#define QCA807X_PHY_MMD3_VD_HALF_BIAS_EN		0x4000
#define QCA807X_PHY_MMD3_AFE_TX_FULL_AMPLITUDE_EN		0x8000

int qca807x_phy_powersave_set(struct nss_phy_device *nss_phy_device,
	bool enable);
int qca807x_phy_powersave_get(struct nss_phy_device *nss_phy_device,
	bool *enable);
#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _QCA807X_PHY_H_ */
