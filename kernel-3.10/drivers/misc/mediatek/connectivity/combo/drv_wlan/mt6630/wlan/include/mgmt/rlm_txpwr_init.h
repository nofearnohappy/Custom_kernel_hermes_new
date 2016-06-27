/*
** $Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/include/mgmt/rlm_txpwr_init.h#1 $
*/

/*! \file   "rlm_txpwr_init.h"
    \brief
*/

/*******************************************************************************
* Copyright (c) 2007 MediaTek Inc.
*
* All rights reserved. Copying, compilation, modification, distribution
* or any other use whatsoever of this material is strictly prohibited
* except in accordance with a Software License Agreement with
* MediaTek Inc.
********************************************************************************
*/

/*******************************************************************************
* LEGAL DISCLAIMER
*
* BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
* AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK
* SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
* PROVIDED TO BUYER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY
* DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE
* ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY
* WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK
* SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY
* WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE
* FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION OR TO
* CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
* BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
* LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL
* BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
* ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
* BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
* WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT
* OF LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING
* THEREOF AND RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN
* FRANCISCO, CA, UNDER THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE
* (ICC).
********************************************************************************
*/


#ifndef _RLM_TXPWR_INIT_H
#define _RLM_TXPWR_INIT_H


/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
/*Support Tx Power Range : 63~ -64 (unit : 0.5dBm)*/

#if CFG_SUPPORT_PWR_LIMIT_COUNTRY 

COUNTRY_CHANNEL_POWER_LIMIT g_rRlmCountryPowerLimitTable[] = {
  { 
    {'T','W'},0,0,{0,0,0,0},
		{
			/*2.4G*/	  //Central Ch			  CCK,			20M,			 40M,			80M,			  160M,	     ucFlag								  
			CHANNEL_PWR_LIMIT(1,	 MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),				   
			CHANNEL_PWR_LIMIT(3,	 MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
			CHANNEL_PWR_LIMIT(9,	 MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
			CHANNEL_PWR_LIMIT(11,	 MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
			/*FCC- Spec : Band UNII-1*/
			CHANNEL_PWR_LIMIT(36,	 MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
			CHANNEL_PWR_LIMIT(38,	 MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
			CHANNEL_PWR_LIMIT(42,	 MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
			/*FCC- Spec : Band UNII-2A*/
			CHANNEL_PWR_LIMIT(58,	 MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
			CHANNEL_PWR_LIMIT(62,	 MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
			CHANNEL_PWR_LIMIT(64,	 MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
			/*FCC- Spec : Band UNII-2C*/
			CHANNEL_PWR_LIMIT(100,	 MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
			CHANNEL_PWR_LIMIT(102,	 MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
			CHANNEL_PWR_LIMIT(106,	 MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
			/*FCC- Spec : Band UNII-3*/
			CHANNEL_PWR_LIMIT(155,	 MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
			CHANNEL_PWR_LIMIT(159,	 MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
			CHANNEL_PWR_LIMIT(165,	 MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
			/*Used to check the end of channel power limit*/
			CHANNEL_PWR_LIMIT(ENDCH,	        0,			  0,			0,			  0,		    0,   0)
			}
  },/*end of TW*/
  {
    {'C','N'},0,0,{0,0,0,0},
     	{
     		/*2.4G*/      //Central Ch            CCK,                  20M,                  40M,                   80M,                  160M,          ucFlag                                  
     		CHANNEL_PWR_LIMIT(1,     MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),                 
     		CHANNEL_PWR_LIMIT(3,     MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
     		CHANNEL_PWR_LIMIT(9,     MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
     		CHANNEL_PWR_LIMIT(11,    MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
     		/*FCC- Spec : Band UNII-1*/
	 		CHANNEL_PWR_LIMIT(36,    MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
     		CHANNEL_PWR_LIMIT(38,    MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
     		CHANNEL_PWR_LIMIT(42,    MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
     		/*FCC- Spec : Band UNII-2A*/
	 		CHANNEL_PWR_LIMIT(58,    MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
     		CHANNEL_PWR_LIMIT(62,    MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
     		CHANNEL_PWR_LIMIT(64,    MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
     		/*FCC- Spec : Band UNII-2C*/
	 		/*FCC- Spec : Band UNII-3*/
     		CHANNEL_PWR_LIMIT(155,   MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
     		CHANNEL_PWR_LIMIT(159,   MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
     		CHANNEL_PWR_LIMIT(165,   MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER, MAX_TX_POWER,   0),
     		/*Used to check the end of channel power limit*/
     		CHANNEL_PWR_LIMIT(ENDCH,            0,            0,            0,            0,            0,   0)
     	}
  },/*end of CN*/
  {
  	/*Used to check the end of country entry*/
   	{0,0},0,0,{0,0,0,0},
     	{
     		/*Used to check the end of channel power limit*/
     		CHANNEL_PWR_LIMIT(ENDCH,            0,            0,            0,            0,            0,   0)
     	}
  }/*end of CountryTable*/
};

#endif

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/


#endif /* _RLM_TXPWR_INIT_H */


