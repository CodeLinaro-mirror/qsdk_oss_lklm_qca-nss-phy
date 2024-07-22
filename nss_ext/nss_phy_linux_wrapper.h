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

#ifndef _NSS_PHY_LINUX_WRAPPER_H_
#define _NSS_PHY_LINUX_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

#include <linux/phy.h>

struct nss_phy_device {
	struct phy_device *phydev;
};

#define nss_phy_err(nss_phydev, format, args...)	\
	(phydev_err(nss_phydev->phydev, format, ##args))
#define nss_phy_info(nss_phydev, format, args...)	\
	(phydev_info(nss_phydev->phydev, format, ##args))
#define nss_phy_warn(nss_phydev, format, args...)	\
	(phydev_warn(nss_phydev->phydev, format, ##args))
#define nss_phy_dbg(nss_phydev, format, args...)	\
	(phydev_dbg(nss_phydev->phydev, format, ##args))
#define nss_phy_pr_info		pr_info

#define NSS_PHY_EOPNOTSUPP		EOPNOTSUPP
#define NSS_PHY_EINVAL		EINVAL
#define NSS_PHY_ENOSPC		ENOSPC

#define nss_phy_interface_t phy_interface_t
#define NSS_NSS_PHY_INTERFACE_MODE_NA		\
	PHY_INTERFACE_MODE_NA
#define NSS_PHY_INTERFACE_MODE_SGMII		\
	PHY_INTERFACE_MODE_SGMII
#define NSS_PHY_INTERFACE_MODE_PSGMII		\
	PHY_INTERFACE_MODE_PSGMII
#define NSS_PHY_INTERFACE_MODE_QSGMII		\
	PHY_INTERFACE_MODE_QSGMII
#define NSS_PHY_INTERFACE_MODE_100BASEX		\
	PHY_INTERFACE_MODE_100BASEX
#define NSS_PHY_INTERFACE_MODE_1000BASEX		\
	PHY_INTERFACE_MODE_1000BASEX
#define NSS_PHY_INTERFACE_MODE_2500BASEX		\
	PHY_INTERFACE_MODE_2500BASEX
#define NSS_PHY_INTERFACE_MODE_USXGMII		\
	PHY_INTERFACE_MODE_USXGMII
#define NSS_PHY_INTERFACE_MODE_QUSGMII		\
	PHY_INTERFACE_MODE_QUSGMII
#define NSS_PHY_INTERFACE_MODE_MAX		\
	PHY_INTERFACE_MODE_MAX

#define NSS_PHY_SPEED_10		SPEED_10
#define NSS_PHY_SPEED_100		SPEED_100
#define NSS_PHY_SPEED_1000		SPEED_1000
#define NSS_PHY_SPEED_2500		SPEED_2500
#define NSS_PHY_SPEED_5000		SPEED_5000
#define NSS_PHY_SPEED_10000		SPEED_10000
#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _NSS_PHY_LINUX_WRAPPER_H_ */
