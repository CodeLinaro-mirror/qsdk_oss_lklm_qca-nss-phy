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

#ifndef _NSS_PHY_H_
#define _NSS_PHY_H_

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */
#include "nss_phy_lib.h"

#define QCA_PHY_ID		0x004dd000
#define QCA_PHY_MASK		0xfffff000
#define QCA_PHY_MATCH(phy_id)		((phy_id & QCA_PHY_MASK) == QCA_PHY_ID)
#define QCA8075_PHY		0x004dd0b1
#define QCA8072_PHY		0x004dd0b2
#define QCA807X_MASK		0xfffffff0

struct nss_phy_ops {
	int (*hibernation_set)(struct nss_phy_device *nss_phydev, bool enable);
	int (*hibernation_get)(struct nss_phy_device *nss_phydev, bool *enable);
	int (*powersave_set)(struct nss_phy_device *nss_phydev, bool enable);
	int (*powersave_get)(struct nss_phy_device *nss_phydev, bool *enable);
};

struct nss_phy_ops *nss_phy_ops_get(void);
#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _NSS_PHY_H_ */
