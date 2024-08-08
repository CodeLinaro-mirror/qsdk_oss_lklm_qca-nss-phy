/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/phy.h>

#define QCA8111_PHY		0x004dd1c0

static struct phy_driver qca81xx_phy_driver[] = {
{
	PHY_ID_MATCH_EXACT(QCA8111_PHY),
	.name = "Qualcomm QCA81xx",
	.flags = PHY_POLL_CABLE_TEST,
	.aneg_done = genphy_c45_aneg_done,
	.suspend = genphy_c45_pma_suspend,
	.resume = genphy_c45_pma_resume,
},
};

module_phy_driver(qca81xx_phy_driver);
MODULE_DESCRIPTION("QCA81XX PHY Driver");
MODULE_LICENSE("Dual BSD/GPL");
