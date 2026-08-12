/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */


#ifndef _QCE1204_PHY_H_
#define _QCE1204_PHY_H_

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */
#define QCE1204_PHY_MMD7_CDT_WITH_AUTONEG	0x200
#define QCE1204_PHY_ALL_PAIRS_NORMAL		0x1111
#define QCE1204_PHY_MMD3_CLD_CTRL		0x8002
#define QCE1204_PHY_CLD_FORCE_EN		0x100
#define QCE1204_PHY_MMD3_CLD_RESULT		0x808c
#define QCE1204_PHY_CLD_CABLE_LENGTH		0xff

/* PPM offset clock frequency divisors for QCE1204 (Huntington) and IPQ52XX (Hermosa) */
#define QCE1204_PPM_DIV_1G		(512ULL << 18)
#define QCE1204_PPM_DIV_2_5G		(640ULL << 15)

int qce1204_phy_ops_init(struct nss_phy_ops *ops);
#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _QCE1204_PHY_H_ */
