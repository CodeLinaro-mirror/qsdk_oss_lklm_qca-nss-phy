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
/*Debug registers*/
#define QCE1204_PHY_DEBUG_AFE_DAC8_DP		0x2f80
#define QCE1204_PHY_DEBUG_AFE_DAC9_DP		0x3080

int qce1204_phy_ops_init(struct nss_phy_ops *ops);
#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _QCE1204_PHY_H_ */
