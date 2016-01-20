/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
#define LOG_TAG "FLICKER"

#include <stdlib.h>
#include <stdio.h>
#include <utils/threads.h>
#include <cutils/xlog.h>
#include "pipe_types.h"
#include "Flicker_type.h"
#include "FlickerDetection.h"
#include "sequential_testing.h"
#include "isp_sysram_drv.h"
#include "sensor_drv.h"
#include "isp_drv.h"
#include "isp_reg.h"
#include "camera_custom_if.h"
#include "kd_camera_feature.h"
#include "flicker_hal.h"
#if defined(MTK_M4U_SUPPORT)    // use m4u to get sensor ouput from ISP.
#include "m4u_lib.h"
#include <linux/cache.h>
#endif

#include "camera_custom_flicker.h"




#define FLICKER_DEBUG

#define FLICKER_MAX_LENG  2048
#define MHAL_FLICKER_WORKING_BUF_SIZE (2048*4*3)    // flicker support max size
#define FLICKER_SUPPORT_MAX_SIZE (2500*4*3)

#ifdef FLICKER_DEBUG
#define FLICKER_HAL_TAG             "[FLICKER Hal] "
#define FLICKER_LOG(fmt, arg...)    XLOGD(FLICKER_HAL_TAG fmt, ##arg)
#define FLICKER_ERR(fmt, arg...)    XLOGE(FLICKER_HAL_TAG "Err: %5d: "fmt, __LINE__, ##arg)
#else
#define FLICKER_LOG(a,...)
#define FLICKER_ERR(a,...)
#endif

#if defined(MTK_M4U_SUPPORT)    // use m4u to get sensor ouput from ISP.
#define MTK_FLK_M4U_EN 1
#else
#define MTK_FLK_M4U_EN 0
#endif

#if MTK_FLK_M4U_EN
typedef struct {
    MUINT32 u4VirAddr;			///< Virtual address that passed from Middle-ware.
    MUINT32 u4UseM4u;			///< A flag indicates using M4U or not.
    MUINT32 u4DefectPortMva;	      ///< MVA that using GMC2 Defect port.
    MUINT32 u4BufSize;		     ///< Size of this buffer.
} RawFlkM4uInfo_t;
static MTKM4UDrv *pM4UDrv = NULL;
static RawFlkM4uInfo_t stRawFlkM4uInfo;
static M4U_PORT_STRUCT M4uPort;
#endif

extern MUINT32 g_u4PreviewShutterValue;


static MINT32 pre_1_LMVx[16];
static MINT32 pre_1_LMVy[16];
static MINT32 pre_2_LMVx[16];
static MINT32 pre_2_LMVy[16];
static MINT32 flicker_frame_count = 0;
/*
*
*/
/*******************************************************************************
*
********************************************************************************/
FlickerHalBase* FlickerHalBase::createInstance()
{
    return FlickerHal::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
FlickerHalBase* FlickerHal::getInstance()
{
    FLICKER_LOG("[FlickerHal] getInstance \n");
    static FlickerHal singleton;

    if (singleton.init() != 0)  {
        FLICKER_LOG("singleton.init() fail \n");
        return NULL;
    }

    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void FlickerHal::destroyInstance()
{
	uninit();
}

/*******************************************************************************
*
********************************************************************************/
FlickerHal::FlickerHal()
{
MINT8 i;

    FLICKER_LOG("FlickerHal() \n");

    m_bFlickerEnable = MFALSE;
    m_bFlickerEnablebit = MFALSE;
    m_u4SensorPixelClkFreq = 0;
    m_u4FreqFrame = 0;
    m_u4Freq000 = 0;
    m_u4Freq100 = 0;
    m_u4Freq120 = 0;
    m_flickerStatus = INCONCLUSIVE;
    m_EIS_LMV_Flag = SMALL_MOTION;
    m_pFlickerSysram = NULL;
    m_pSensorDrv = NULL;
    m_pIspDrv = NULL;
    m_iDataBank = 0;
    m_u4FlickerFreq = HAL_FLICKER_AUTO_50HZ;
    m_u4FlickerWidth = 0;
    m_u4FlickerHeight = 0;


#if MTK_FLK_M4U_EN
//    pM4UDrv = NULL;    	// TBD need to do free m4u memory first
#endif

    for(i=0; i<8; i++) {
    	m_vAMDF[i] = 0;
    }
}

/*******************************************************************************
*
********************************************************************************/
FlickerHal::~FlickerHal()
{
    FLICKER_LOG("~FlickerHal() \n");
}

/*******************************************************************************
*
********************************************************************************/
void FlickerHal::setSensorInfo(CAMERA_DUAL_CAMERA_SENSOR_ENUM eDeviceId, MINT32 bZsdOn)
{
	MINT32* tab1;
	MINT32* tab2;
	int tabDim1;
	int tabDim2;

	tab1 = cust_getFlickerTable1(eDeviceId , bZsdOn, &tabDim1); //flicker_table1[9][500] --> tabDim1=500
	tab2 = cust_getFlickerTable2(eDeviceId , bZsdOn, &tabDim2);

	flicker_setTable(tab1, tab2, tabDim1);

}

MINT32 FlickerHal::init()
{
    MINT32 err = 0;
    NSCamCustom::FlickerThresholdSetting_T strFlickerThreshold;

    FLICKER_LOG("init - mUsers: %d \n", mUsers);

    Mutex::Autolock lock(mLock);


    for(int i=0;i<16;i++)
    {
        pre_1_LMVx[i] = 0;
        pre_1_LMVy[i] = 0;
        pre_2_LMVx[i] = 0;
        pre_2_LMVy[i] = 0;
    }

	if (m_u4FlickerFreq == HAL_FLICKER_AUTO_50HZ) {
		set_flicker_state(Hz50);               // defined in "sequential_testing.cpp", have to call these two functions in intialization or every time we change the flicker table
		FLICKER_LOG("Initialize flicker state: %d\n",Hz50);
		}
	else {
		set_flicker_state(Hz60);			   // defined in "sequential_testing.cpp", have to call these two functions in intialization or every time we change the flicker table
		FLICKER_LOG("Initialize flicker state: %d\n",Hz60);
	}
	reset_flicker_queue();

    if (mUsers > 0) {
        FLICKER_ERR("%d has created \n", mUsers);
        android_atomic_inc(&mUsers);
        return 0;
    }

    //sensor driver
    m_pSensorDrv = SensorDrv::createInstance(SENSOR_MAIN);
    if (!m_pSensorDrv) {
        FLICKER_ERR("Flicker::createInstance SensorDrv fail \n");
        goto create_fail_exit;
    }

    err = m_pSensorDrv->sendCommand(CMD_SENSOR_GET_PIXEL_CLOCK_FREQ, &m_u4SensorPixelClkFreq, NULL, NULL);
    if(err != 0) {
    	FLICKER_ERR("No get pixel clock frequency. \n");
    }
    FLICKER_LOG("init - m_u4SensorPixelClkFreq: %d \n", m_u4SensorPixelClkFreq);

    err = m_pSensorDrv->sendCommand(CMD_SENSOR_GET_PIXEL_PERIOD, &m_u4PixelsInLine, NULL, NULL);
    if(err != 0) {
    	FLICKER_ERR("No get pixels per line. \n");
    }

    m_u4PixelsInLine &= 0x0000FFFF;
    FLICKER_LOG("init - m_u4PixelsInLine: %d \n", m_u4PixelsInLine);

    // Create isp driver
    m_pIspDrv = IspDrv::createInstance();
    if (!m_pIspDrv) {
        FLICKER_ERR("Flicker::createInstance IspDrv fail \n");
        goto create_fail_exit;
    }

    if (m_pIspDrv) {
        m_pIspDrv->init();
	 m_pIspDrv->sendCommand(CMD_GET_ISP_ADDR, (MINT32)&m_pIspRegMap);
    }

    android_atomic_inc(&mUsers);

    m_pVectorAddress1 = NULL;

    strFlickerThreshold = NSCamCustom::getFlickerThresPara();
    setFlickerThresholdParams(&strFlickerThreshold);


    static int init1st=1;
    static MINT32* pMem1=0;
    static MINT32* pMem2=0;
    if(init1st==1)
    {
        m_pVectorData1 = (MINT32*)malloc(MHAL_FLICKER_WORKING_BUF_SIZE);
        if(m_pVectorData1 == NULL)
        {
            FLICKER_ERR("memory1 is not enough");
            return -2;
        }

        m_pVectorData2 = (MINT32*)malloc(MHAL_FLICKER_WORKING_BUF_SIZE);
        if(m_pVectorData2 == NULL)
        {
            FLICKER_ERR("memory2 is not enough");
            return -3;
        }

        pMem1 = m_pVectorData1;
        pMem2 = m_pVectorData2;
        init1st=0;
    }
    else
    {
        m_pVectorData1 = pMem1;
        m_pVectorData2 = pMem2;
    }

    m_u4FlickerWidth = 0;
    m_u4FlickerHeight = 0;

#if MTK_FLK_M4U_EN
    FLICKER_LOG("M4U enable sizeof(stRawFlkM4uInfo) = %d\n",sizeof(stRawFlkM4uInfo));
//    if(!pM4UDrv) {
    memset(&stRawFlkM4uInfo,0x00, sizeof(stRawFlkM4uInfo));
    pM4UDrv = new MTKM4UDrv();
    if (!pM4UDrv) {
        FLICKER_ERR("pM4UDrv new fail.\n");
        goto create_fail_exit;
    } else {
        FLICKER_LOG("pM4UDrv new OK.p:0x%8x &p:0x%8x\n", (MUINT32)pM4UDrv, (MUINT32)&pM4UDrv);
        if(!stRawFlkM4uInfo.u4VirAddr) {
            stRawFlkM4uInfo.u4VirAddr = (MUINT32)memalign(L1_CACHE_BYTES, FLICKER_SUPPORT_MAX_SIZE);
            memset((MUINT8*)stRawFlkM4uInfo.u4VirAddr,0x00, FLICKER_SUPPORT_MAX_SIZE);
            FLICKER_LOG("malloc(stRawFlkM4uInfo.u4VirAddr0x%8x, size%d\n", stRawFlkM4uInfo.u4VirAddr, FLICKER_SUPPORT_MAX_SIZE);
            stRawFlkM4uInfo.u4BufSize = FLICKER_SUPPORT_MAX_SIZE;
            err = pM4UDrv->m4u_alloc_mva( M4U_CLNTMOD_CAM, stRawFlkM4uInfo.u4VirAddr, stRawFlkM4uInfo.u4BufSize, &stRawFlkM4uInfo.u4DefectPortMva);
            if(err) {
                goto create_fail_exit;
            }
            FLICKER_LOG("Flicker [m4u_insert_tlb_range] u4CamPortMva,END = 0x%8x, 0x%8x \n", stRawFlkM4uInfo.u4DefectPortMva, stRawFlkM4uInfo.u4DefectPortMva + stRawFlkM4uInfo.u4BufSize-1);
            err = pM4UDrv->m4u_insert_tlb_range( M4U_CLNTMOD_CAM, stRawFlkM4uInfo.u4DefectPortMva, stRawFlkM4uInfo.u4DefectPortMva + stRawFlkM4uInfo.u4BufSize -1, RT_RANGE_HIGH_PRIORITY, 1);
            if(err != M4U_STATUS_OK) {
                FLICKER_ERR("m4u_insert_tlb_range fail");
                goto create_fail_exit;
            }

            M4uPort.ePortID = M4U_PORT_CAM;
            M4uPort.Virtuality = 1;	   	// 0: means use physical address directly. 1: means enable M4U for this port.
            M4uPort.Security = 0;		// Security zone support, dose not used now, set to 0.
            M4uPort.Distance = 1;		// Prefetch distance in page unit. Set to 1 for normal sequential memory access, means will prefetch next one page when processing current page.
            M4uPort.Direction = 0;		// 0: means access from low address to high address. 1: means access from high address to low address.
            pM4UDrv->m4u_config_port(&M4uPort);
        } else {
            FLICKER_LOG("already ! malloc(stRawFlkM4uInfo.u4VirAddr0x%8x, size%d\n", stRawFlkM4uInfo.u4VirAddr, FLICKER_SUPPORT_MAX_SIZE);
        }
    }
//    }else {
//        FLICKER_LOG("M4U address exist 0x%x 0x%x\n", *pM4UDrv, pM4UDrv);
//    }
#endif

    return err;

create_fail_exit:

    if (m_pSensorDrv) {
        m_pSensorDrv->destroyInstance();
        m_pSensorDrv = NULL;
    }

    if (m_pIspDrv) {
        m_pIspDrv->destroyInstance();
        m_pIspDrv = NULL;
    }

#if MTK_FLK_M4U_EN
    if(pM4UDrv) {
        delete pM4UDrv;
        pM4UDrv = NULL;
    }
#endif
    return -1;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 FlickerHal::uninit()
{
    MINT32 err = 0;
    MINT32 i4PollingTime = 10, i4Index;
    MINT32 i4FlickerStatus;

    FLICKER_LOG("uninit - mUsers: %d \n", mUsers);

    Mutex::Autolock lock(mLock);

    if (mUsers <= 0) {
        // No more users
        return 0;
    }

    // More than one user
    android_atomic_dec(&mUsers);
    //
    if (mUsers == 0) {
    	 if (m_pSensorDrv) {
            m_pSensorDrv->destroyInstance();
            m_pSensorDrv = NULL;
        }

        if (m_pIspDrv) {
        	m_pIspDrv->uninit();
        	m_pIspDrv->destroyInstance();
        	m_pIspDrv = NULL;
        }

        /*
        if(m_pVectorData1 != NULL)
        {
            free(m_pVectorData1);
            m_pVectorData1 = NULL;
        }

        if(m_pVectorData2 != NULL)
        {
            free(m_pVectorData2);
            m_pVectorData2 = NULL;
        }*/

#if MTK_FLK_M4U_EN
    if(pM4UDrv == NULL) {
        FLICKER_LOG("Null M4U driver");
        return -1;
    }

    {
	    int  i4FlickerStatus;
	    i4FlickerStatus = ISP_REG(m_pIspRegMap, CAM_FLK_CON);
	    FLICKER_LOG("flicker status = %d ln=%d\n", i4FlickerStatus, __LINE__);
	}
    // Wait the flicker GMC done
    i4PollingTime = 10;
    for(i4Index=0; i4Index < 100; i4Index++) {   //total time 42 + 4*90 = 402ms
        i4FlickerStatus = ISP_REG(m_pIspRegMap, CAM_FLK_CON);
//        if(((i4FlickerStatus & 0x0400) != 0) || ((i4FlickerStatus & 0x08000000) == 0)) {   // the flicker stop  bit10 flk gmc ok and bit27 double buffer value of flk_en
        if((i4FlickerStatus & 0x08000000) == 0) {   // the flicker stop  bit10 flk gmc ok and bit27 double buffer value of flk_en
            break;
        }
        //Wait 1ms for PLL stable
        usleep(i4PollingTime*1000);  // sleep time 10ms, 8ms, 6ms, 4ms, 4ms, 4ms.....
        i4PollingTime -= 2;
        if(i4PollingTime < 4) {
            i4PollingTime = 4;
        }
    }

    if(i4Index == 100) {
        FLICKER_ERR("Wait flicker stop timeout 2 :0x%x \n", i4FlickerStatus);
        ISP_REG(m_pIspRegMap, CAM_FLK_CON) = (0x01 << 5);
        i4FlickerStatus = ISP_REG(m_pIspRegMap, CAM_FLK_CON);
        FLICKER_ERR("Wait flicker stop timeout 1 :0x%x \n", i4FlickerStatus);
        ISP_REG(m_pIspRegMap, CAM_FLK_CON) = (0x04 << 5);
        i4FlickerStatus = ISP_REG(m_pIspRegMap, CAM_FLK_CON);
        FLICKER_ERR("Wait flicker stop timeout 4 :0x%x \n", i4FlickerStatus);
    }

    //
    FLICKER_LOG("Flicker [m4u_invalid_tlb_range] m4uMVa = 0x%x, size = %d \n", stRawFlkM4uInfo.u4DefectPortMva, (stRawFlkM4uInfo.u4DefectPortMva+stRawFlkM4uInfo.u4BufSize-1));
    err = pM4UDrv->m4u_invalid_tlb_range( M4U_CLNTMOD_CAM, stRawFlkM4uInfo.u4DefectPortMva, stRawFlkM4uInfo.u4DefectPortMva + stRawFlkM4uInfo.u4BufSize - 1);
    if(err!=0) {
    	 FLICKER_LOG("Flicker [freeM4UMemory] m4u_invalid_tlb_range fail \n");
    }
    FLICKER_LOG("Flicker [m4u_dealloc_mva] m4uMVa = 0x%x, size = %d \n", stRawFlkM4uInfo.u4DefectPortMva, stRawFlkM4uInfo.u4BufSize);
    pM4UDrv->m4u_dealloc_mva( M4U_CLNTMOD_CAM, stRawFlkM4uInfo.u4VirAddr, stRawFlkM4uInfo.u4BufSize, stRawFlkM4uInfo.u4DefectPortMva);
    if(err!=0) {
    	 FLICKER_LOG("Flicker [freeM4UMemory] m4u_dealloc_mva fail \n");
    }

    if(stRawFlkM4uInfo.u4VirAddr) {
        free((void*)stRawFlkM4uInfo.u4VirAddr);  //from AppFD.cpp
        FLICKER_LOG("~ free u4VirAddr(0x%8x)!!!\n",stRawFlkM4uInfo.u4VirAddr);
        stRawFlkM4uInfo.u4VirAddr = NULL;
    }

    if(pM4UDrv) {
        delete pM4UDrv;
        pM4UDrv = NULL;
    }
#endif

    }
    else {
        FLICKER_LOG("Still %d users \n", mUsers);
    }

    {
	    int  i4FlickerStatus;
	    i4FlickerStatus = ISP_REG(m_pIspRegMap, CAM_FLK_CON);
	    FLICKER_LOG("flicker status = %d ln=%d\n", i4FlickerStatus, __LINE__);
	}

    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 FlickerHal::sysram_alloc(
    NSIspSysram::EUser_T const eUsr,
    MUINT32 const u4BytesToAlloc,
    MVOID*& rPA,
    MVOID*& rVA
)
{
    MINT32 err = 0;

    //  (1) Make sure the sysram driver is open.
    if (!m_pFlickerSysram)
    {
        err = -1;
        goto lbExit;
    }

    //  (2) Try to alloc the sysram.
    {
        MUINT32 u4BytesAllocated = 0;
        NSIspSysram::MERROR_ENUM_T err_sysram = m_pFlickerSysram->alloc(eUsr, u4BytesToAlloc, u4BytesAllocated, 3000);   // timeout time 1 sec
        if  ( NSIspSysram::MERR_OK != err_sysram ||  u4BytesToAlloc != u4BytesAllocated || NULL == m_pFlickerSysram->getPhyAddr(eUsr))
        {   //  Free it and re-alloc.
            sysram_free(eUsr);
            err_sysram = m_pFlickerSysram->alloc( eUsr, u4BytesToAlloc, u4BytesAllocated);
            //  Again, check to see whether it is successful or not.
            if  ( NSIspSysram::MERR_OK != err_sysram ||  u4BytesToAlloc != u4BytesAllocated || NULL == m_pFlickerSysram->getPhyAddr(eUsr))
            {
                FLICKER_ERR("[sysram alloc] fail: (err_sysram, eUsr, u4BytesToAlloc, u4BytesAllocated, PA)=(%X %d, 0x%08X, 0x%08X, %p)"
                    , err_sysram, eUsr, u4BytesToAlloc, u4BytesAllocated, m_pFlickerSysram->getPhyAddr(eUsr));
                err = -1;
                goto lbExit;
            }
        }
    }

    //  (3) Here, alloc must be successful.
    rPA = m_pFlickerSysram->getPhyAddr(eUsr);
    rVA = m_pFlickerSysram->getVirAddr(eUsr);
    err = 0;

    FLICKER_LOG("[sysram alloc] (eUsr, u4BytesToAlloc, rPA, rVA)=(%d, 0x%08X, %p, %p)", eUsr, u4BytesToAlloc, rPA, rVA);

lbExit:
    if (err != 0)
    {
        sysram_free(eUsr);
    }

    return err;
}


MINT32 FlickerHal::sysram_free(NSIspSysram::EUser_T const eUsr)
{
    FLICKER_LOG("[sysram free] eUsr=%d m_pFlickerSysram:0x%08x", (NSIspSysram::EUser_T)eUsr, (MINT32) m_pFlickerSysram);
    if(m_pFlickerSysram) {
       m_pFlickerSysram->free(eUsr);
    }
    return  0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 FlickerHal::setFlickerDrv(
    unsigned long flicker_mem_mode,
    unsigned long flicker_en)
{
    int ret = 0;
    cam_flk_con_t rFlkCtrlReg; // 0x00E0

    memset((void *)&rFlkCtrlReg, 0, sizeof(cam_flk_con_t));

    rFlkCtrlReg.u.bits.REV0 = 0x02;   // work around for the FLK_DONE_STATUS
    rFlkCtrlReg.u.bits.FLK_MEM_MODE = flicker_mem_mode;

    if(flicker_en == 1) {  // enable flicker
        rFlkCtrlReg.u.bits.FLK_EN = flicker_en;
#if MTK_FLK_M4U_EN
        // flush dram each 2 frames, explain from Xiang Xu
        // flushA,B,HW write A，CPU read A,HW write B，CPU read B，從flushAB到HW write 完A之間， 保證CPU不 readA， 從flushAB到HW write完B之間， 保證CPU不 read B 的話，就沒關系
        pM4UDrv->m4u_cache_sync(M4U_CLNTMOD_CAM , M4U_CACHE_FLUSH_BEFORE_HW_WRITE_MEM ,stRawFlkM4uInfo.u4VirAddr, stRawFlkM4uInfo.u4BufSize);
#endif
    } else {   // disable flicker
//        FLICKER_LOG("[setFlickerDrv]:flicker_mem_mode:%d flicker_en:%d\n", (int) flicker_mem_mode, (int) flicker_en);
        rFlkCtrlReg.u.bits.FLK_EN = flicker_en;
    }

    ISP_REG(m_pIspRegMap, CAM_FLK_CON) = rFlkCtrlReg.u.reg;

//    FLICKER_LOG("[setFlickerDrv]:flicker_mem_mode:%d flicker_en:%d Regbit:0x%08x\n", (int) flicker_mem_mode, (int) flicker_en, (int) ISP_REG(m_pIspRegMap, CAM_FLK_CON));

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 FlickerHal::setFlickerWinConfig(
    unsigned long flicker_interval_x,
    unsigned long flicker_interval_y)
{
    int ret = 0;
    cam_flk_intvl_t rFlkWinIntvl; // 0x00E4

    memset((void *)&rFlkWinIntvl, 0, sizeof(cam_flk_intvl_t));

    rFlkWinIntvl.u.bits.INTVL_X = flicker_interval_x;
    rFlkWinIntvl.u.bits.INTVL_Y = flicker_interval_y;

    ISP_REG(m_pIspRegMap, CAM_FLK_INTVL) = rFlkWinIntvl.u.reg;
    FLICKER_LOG("[setFlickerConfig]:flicker_interval:0x%08x\n", (int) ISP_REG(m_pIspRegMap, CAM_FLK_INTVL));

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 FlickerHal::setFlickerGMCConfig(
    unsigned long flicker_GMC_address)
{
    int ret = 0;

    ISP_REG(m_pIspRegMap, CAM_FLK_GADDR) = flicker_GMC_address;

//    FLICKER_LOG("[setFlickerConfig]:flicker_GMC_address:0x%08x 0x%08x\n", (int) ISP_REG(pisp, CAM_FLK_GADDR), (int) flicker_GMC_address);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 FlickerHal::analyzeFlickerFrequency(MINT32 i4LMVcnt, MINT32 *i4LMV_x, MINT32 *i4LMV_y, MINT32 *i4vAFstatisic)
{
		FLICKER_LOG("analyzeFlickerFrequency");
//MINT32 i4LMV_x[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, i4LMV_y[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
MINT32 i4vAFstat[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
MINT32 i4FlickerStatus;
MINT32 i;
MINT32 *i4vAFInfo = NULL;
MINT32 i4DataLen = 0;
MUINT32 u4Height;

//    FLICKER_LOG("analyzeFlickerFrequency:%d\n", m_bFlickerEnable);

    if(m_bFlickerEnable) {
        i4FlickerStatus = ISP_REG(m_pIspRegMap, CAM_FLK_CON);





        if(i4FlickerStatus & 0x60000000) {
            FLICKER_ERR("[Flicker] The Flicker status error 0x%08x\n", i4FlickerStatus);
        }

//        FLICKER_LOG("[Flicker] i4FlickerStatus:0x%08x Bank:%d\n", i4FlickerStatus, m_iDataBank);

//        if((i4FlickerStatus & 0x0800) && ((i4FlickerStatus & 0x04000000)==0)) { // check the flicker done status


        if(i4FlickerStatus & 0x0800)
            flicker_frame_count = 0;
        else
            flicker_frame_count++;

        if(flicker_frame_count == 1)
        {
        	  for(int i=0;i<16;i++)
            {
                pre_1_LMVx[i] = i4LMV_x[i];
                pre_1_LMVy[i] = i4LMV_y[i];
            }
        }
        else if(flicker_frame_count == 2)
        {
        		for(int i=0;i<16;i++)
            {
                pre_2_LMVx[i] = i4LMV_x[i];
                pre_2_LMVy[i] = i4LMV_y[i];
            }
        }
        FLICKER_LOG("flicker_frame_count is %d\n", flicker_frame_count);

        if(i4FlickerStatus & 0x0800) { // check the flicker done status
           // get the AF statistic information
           if(i4vAFstatisic == NULL) {
           	i4vAFInfo = &i4vAFstat[0];
           } else {
              i4vAFInfo = i4vAFstatisic;
           }
           // get the EIS LMV information

           // flicker detection firmware
           m_pVectorAddress2 = (MINT32 *) (8*(((MUINT32)m_pVectorAddress1 + 6*m_u4FlickerHeight)/8));

           if((i4FlickerStatus & 0x04000000) == 0){
               setFlickerDrv(1, m_bFlickerEnable);    // Save the column vector and difference
           } else {
               m_bFlickerEnablebit = MTRUE;
           }


//           FLICKER_LOG("[Flicker]Vector address:0x%08x, 0x%08x LMV_x:%d LMV_y:%d %d %d\n", (MUINT32) m_pVectorAddress1, (MUINT32) m_pVectorAddress2, i4LMV_x, i4LMV_y, m_u4FlickerWidth, m_u4FlickerHeight);
           if(m_u4FlickerHeight > FLICKER_MAX_LENG) {
               i4DataLen = 3*FLICKER_MAX_LENG /2 ;
               u4Height = FLICKER_MAX_LENG;
           } else {
               i4DataLen = 3*m_u4FlickerHeight /2 ;
               u4Height = m_u4FlickerHeight;
           }
 #if 0   // 6 frame run one time flicker algorithm
           if(m_iDataBank == 0) {
               for(i=0; i<i4DataLen; i++) {
                   m_pVectorData1[2*i+0] = m_pVectorAddress1[i] &0x0000FFFF;
                   m_pVectorData1[2*i+1] = (m_pVectorAddress1[i] &0xFFFF0000)>>16;
               }
               m_iDataBank = 1;
               return MHAL_NO_ERROR;
           } else {
               for(i=0; i<i4DataLen; i++) {
                   m_pVectorData2[2*i+0] = m_pVectorAddress1[i] &0x0000FFFF;
                   m_pVectorData2[2*i+1] = (m_pVectorAddress1[i] &0xFFFF0000)>>16;
               }
               m_iDataBank = 0;
           }
#else
           if(m_iDataBank == 0) {
               for(i=0; i<i4DataLen; i++) {
                   m_pVectorData2[2*i+0] = m_pVectorAddress1[i] &0x0000FFFF;
                   m_pVectorData2[2*i+1] = (m_pVectorAddress1[i] &0xFFFF0000)>>16;
               }
               m_iDataBank = 1;
               return 0;
           } else {
               for(i=0; i<i4DataLen; i++) {
                   m_pVectorData1[2*i+0] = m_pVectorData2[2*i+0] ;
                   m_pVectorData1[2*i+1] = m_pVectorData2[2*i+1];
                   m_pVectorData2[2*i+0] = m_pVectorAddress1[i] &0x0000FFFF;
                   m_pVectorData2[2*i+1] = (m_pVectorAddress1[i] &0xFFFF0000)>>16;
               }
           }
#endif
			m_sensorInfo.pixelClock = m_u4SensorPixelClkFreq;
			m_sensorInfo.fullLineWidth = m_u4PixelsInLine;
			m_AEInfo.previewShutterValue = g_u4PreviewShutterValue;
			for(i=0; i<16; i++)
			{
				m_EISvector.vx[i] = (i4LMV_x[i] + pre_1_LMVx[i] + pre_2_LMVx[i])/16;
				m_EISvector.vy[i] = (i4LMV_y[i] + pre_1_LMVy[i] + pre_1_LMVy[i])/16;
			}




    	   int bFlickerWHSet=0;
    	   if(m_u4FlickerWidth> 0 && m_u4FlickerHeight>0)
    	   		bFlickerWHSet=1;

           //if(((g_u4PreviewShutterValue < 75000) && (m_u4FlickerFreq == HAL_FLICKER_AUTO_60HZ)) || ((g_u4PreviewShutterValue < 80000) && (m_u4FlickerFreq == HAL_FLICKER_AUTO_50HZ))) {
           if(    bFlickerWHSet==1 &&
           		 g_u4PreviewShutterValue < 74000 &&
           	     ( (g_u4PreviewShutterValue > 8200 && m_u4FlickerFreq == HAL_FLICKER_AUTO_60HZ ) || (g_u4PreviewShutterValue > 9800 && m_u4FlickerFreq == HAL_FLICKER_AUTO_50HZ ) ) )
           	 {
//               m_flickerStatus = detectFlicker_SW(m_u4FlickerWidth, u4Height, i4LMV_x, i4LMV_y, 13*256, &m_EIS_LMV_Flag, i4vAFInfo, m_pVectorData1, m_pVectorData2, m_u4SensorPixelClkFreq, m_u4PixelsInLine, m_vAMDF, m_flickerStatus, &m_u4FreqFrame, &m_u4Freq000, &m_u4Freq100, &m_u4Freq120, m_u4FlickerFreq);
                 MINT32 win_wd = ((m_u4FlickerWidth / 3)>>1)<<1;
                 MINT32 win_ht = ((u4Height / 3)>>1)<<1;


                 FLICKER_LOG("Eric i4LMVcnt is %d\n", i4LMVcnt);
                 FLICKER_LOG("Eric m_EISvector.vx[0]:%d,m_EISvector.vy[0]:%d\n",  m_EISvector.vx[0],  m_EISvector.vy[0]);
                 setLMVcnt(i4LMVcnt);
                 if(getFlkPause() ==0)
                 {

                 	FLICKER_LOG("detectFlicker_SW+ w,h=%d %d", win_wd, win_ht);
                 m_flickerStatus = detectFlicker_SW(m_pVectorData1, m_pVectorData2, 3, 3, win_wd, win_ht, m_u4FlickerFreq, m_sensorInfo, m_EISvector, m_AEInfo, i4vAFInfo);
                 	FLICKER_LOG("detectFlicker_SW-");

    if(m_flickerStatus == FK120)
    	 FLICKER_LOG("Eric m_u4FlickerFreq = HAL_FLICKER_AUTO_60HZ\n");
    else if(m_flickerStatus == FK100)
        FLICKER_LOG("Eric m_u4FlickerFreq = HAL_FLICKER_AUTO_50HZ\n");
    else
    	  FLICKER_LOG("Eric m_u4FlickerFreq = HAL_FLICKER_AUTO_Error\n");
                 }
				// if the decision is to change
				if (m_u4FlickerFreq == HAL_FLICKER_AUTO_50HZ && m_flickerStatus == FK120 )
				{
                   m_u4FlickerFreq = HAL_FLICKER_AUTO_60HZ;
				   set_flicker_state(Hz60);	// defined in "sequential_testing.cpp", have to call these two functions every time we change the flicker table
				   reset_flicker_queue();


				}
				else if (m_u4FlickerFreq == HAL_FLICKER_AUTO_60HZ && m_flickerStatus == FK100 )
				{
                   m_u4FlickerFreq = HAL_FLICKER_AUTO_50HZ;
				   set_flicker_state(Hz50);	// defined in "sequential_testing.cpp", have to call these two functions every time we change the flicker table
				   reset_flicker_queue();
				   FLICKER_LOG("Eric m_u4FlickerFreq = HAL_FLICKER_AUTO_50HZ\n");
				}

               //if(m_flickerStatus == FK100) {
               //	   m_u4FlickerFreq = HAL_FLICKER_AUTO_50HZ;
               //} else if (m_flickerStatus == FK120) {
               //    m_u4FlickerFreq = HAL_FLICKER_AUTO_60HZ;
               //}
           } else if ( g_u4PreviewShutterValue >= 74000 ) {
               FLICKER_LOG("The exposure time too long, skip flicker:%d\n", g_u4PreviewShutterValue);
           } else {
               FLICKER_LOG("The exposure time too short, skip flicker:%d\n", g_u4PreviewShutterValue);
           }

           FLICKER_LOG("Status:%d, %d\n",m_u4FlickerFreq,g_u4PreviewShutterValue);

//           FLICKER_LOG("Status:%d,%d,%d,%d,%d,%d, %d, %d\n", m_EIS_LMV_Flag, m_flickerStatus, m_u4FreqFrame, m_u4Freq000, m_u4Freq100, m_u4Freq120, m_u4FlickerFreq,g_u4PreviewShutterValue);
//           FLICKER_LOG("AMDF : %d,%d,%d,%d,%d,%d,%d,%d, LMV:%d %d\n", m_vAMDF[0], m_vAMDF[1], m_vAMDF[2], m_vAMDF[3], m_vAMDF[4], m_vAMDF[5], m_vAMDF[6], m_vAMDF[7], i4LMV_x[0],  i4LMV_y[0]);

       // output result to log files
           FLICKER_LOG("AF vector : %d,%d,%d,%d,%d,%d,%d,%d,%d\n", i4vAFInfo[0], i4vAFInfo[1], i4vAFInfo[2], i4vAFInfo[3], i4vAFInfo[4], i4vAFInfo[5], i4vAFInfo[6], i4vAFInfo[7], i4vAFInfo[8]);
      //     FLICKER_LOG("CAM_FLK_CON:0x%08x CAM_FLK_INTVL:0x%08x CAM_FLK_GADDR:0x%08x\n", (int) ISP_REG(m_pIspRegMap, CAM_FLK_CON), (int) ISP_REG(m_pIspRegMap, CAM_FLK_INTVL), (int) ISP_REG(m_pIspRegMap, CAM_FLK_GADDR));
      //     FLICKER_LOG("CAM_AFWIN0:0x%08x CAM_AFWIN1:0x%08x CAM_AFWIN2:0x%08x\n", (int) ISP_REG(m_pIspRegMap, CAM_AFWIN0), (int) ISP_REG(m_pIspRegMap, CAM_AFWIN1), (int) ISP_REG(m_pIspRegMap, CAM_AFWIN2));
      //     FLICKER_LOG("CAM_AFWIN3:0x%08x CAM_AFWIN4:0x%08x CAM_AFWIN5:0x%08x\n", (int) ISP_REG(m_pIspRegMap, CAM_AFWIN3), (int) ISP_REG(m_pIspRegMap, CAM_AFWIN4), (int) ISP_REG(m_pIspRegMap, CAM_AFWIN5));
      //     FLICKER_LOG("CAM_AFWIN6:0x%08x CAM_AFWIN7:0x%08x CAM_AFWIN8:0x%08x\n", (int) ISP_REG(m_pIspRegMap, CAM_AFWIN6), (int) ISP_REG(m_pIspRegMap, CAM_AFWIN7), (int) ISP_REG(m_pIspRegMap, CAM_AFWIN8));
      //     FLICKER_LOG("LMV_x:%d,LMV_y:%d\n",  i4LMV_x[0],  i4LMV_y[0]);
      //     FLICKER_LOG("%d,%d,%d,%d,%d,%d,%d,%d, \n",  i4LMV_x[0],  i4LMV_y[0],  i4LMV_x[1],  i4LMV_y[1],  i4LMV_x[2],  i4LMV_y[2],  i4LMV_x[3],  i4LMV_y[3]);
      //     FLICKER_LOG("%d,%d,%d,%d,%d,%d,%d,%d, \n",  i4LMV_x[4],  i4LMV_y[4],  i4LMV_x[5],  i4LMV_y[5],  i4LMV_x[6],  i4LMV_y[6],  i4LMV_x[7],  i4LMV_y[7]);
      //     FLICKER_LOG("%d,%d,%d,%d,%d,%d,%d,%d, \n",  i4LMV_x[8],  i4LMV_y[8],  i4LMV_x[9],  i4LMV_y[9],  i4LMV_x[10], i4LMV_y[10], i4LMV_x[11], i4LMV_y[11]);
      //     FLICKER_LOG("%d,%d,%d,%d,%d,%d,%d,%d \n", i4LMV_x[12], i4LMV_y[12], i4LMV_x[13], i4LMV_y[13], i4LMV_x[14], i4LMV_y[14], i4LMV_x[15], i4LMV_y[15]);
        }else {
//            FLICKER_LOG("[Flicker] Wait the next frame:0x%08x \n", i4FlickerStatus);
//            FLICKER_LOG("CAM_FLK_CON:0x%08x CAM_FLK_INTVL:0x%08x CAM_FLK_GADDR:0x%08x\n", (int) ISP_REG(m_pIspRegMap, CAM_FLK_CON), (int) ISP_REG(m_pIspRegMap, CAM_FLK_INTVL), (int) ISP_REG(m_pIspRegMap, CAM_FLK_GADDR));
            if(m_bFlickerEnablebit) {
               setFlickerDrv(1, m_bFlickerEnable);    // Save the column vector and difference
               m_bFlickerEnablebit = MFALSE;
               FLICKER_LOG("[Flicker] i4FlickerStatus:0x%08x Bank:%d Enablebit:%d\n", i4FlickerStatus, m_iDataBank, m_bFlickerEnablebit);
            } else if((i4FlickerStatus & 0x0600) == 0x0400){
               setFlickerDrv(1, 0);    // Disable the auto flicker
               m_bFlickerEnablebit = MTRUE;
               FLICKER_LOG("[Flicker] Disable the auto flicker\n");
            }
			// update the window configure if the window different
            setWindowInfo(m_u4FlickerWidth, m_u4FlickerHeight);
        }
    }
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MVOID FlickerHal::setFlickerThresholdParams(NSCamCustom::FlickerThresholdSetting_T *strFlickerThres)
{
    MINT32 threc[2] = {0, 0}, threa[3] = {0, 0, 0}, thref[3] = {0, 0, 0};

    threc[0] = strFlickerThres->u4FlickerPoss1;
    threc[1] = strFlickerThres->u4FlickerPoss2;
    threa[0] = strFlickerThres->u4FlickerFreq1;
    threa[1] = strFlickerThres->u4FlickerFreq2;
    threa[2] = strFlickerThres->u4FlickerFreq2;
    thref[0] = strFlickerThres->u4ConfidenceLevel1;
    thref[1] = strFlickerThres->u4ConfidenceLevel2;
    thref[2] = strFlickerThres->u4ConfidenceLevel3;

//   FLICKER_LOG("threc:%d,%d threa:%d,%d thref:%d,%d,%d, \n", threc[0], threc[1], threa[0], threa[1], thref[0], thref[1], thref[2]);

    setThreshold(threc, threa, thref);
}

/*******************************************************************************
*
********************************************************************************/
MINT32 FlickerHal::enableFlickerDetection(MBOOL bEnableFlicker)
{
    MVOID * rPhyAddress = NULL;
    MVOID * rVirAddress = NULL;
    MINT32 i4FlickerStatus;
    MINT32 i4PollingTime = 10, i4Index;
    MINT32 ret = 0;	// 0: no error.

    m_bFlickerEnable = bEnableFlicker;
    if(m_bFlickerEnable && m_pIspDrv) {
        i4FlickerStatus = ISP_REG(m_pIspRegMap, CAM_FLK_CON);
        if(i4FlickerStatus & 0x01) {
            FLICKER_LOG("flicker enable already 0x%8x\n",(MUINT32)i4FlickerStatus);
            return ret;
        }

#if MTK_FLK_M4U_EN
        if(stRawFlkM4uInfo.u4VirAddr!=0) {
            rPhyAddress = (MVOID*)stRawFlkM4uInfo.u4DefectPortMva;
            rVirAddress = (MVOID*)stRawFlkM4uInfo.u4VirAddr;
            FLICKER_LOG("rPhyAddress 0x%8x\n",(MUINT32)rPhyAddress);
            FLICKER_LOG("rVirAddress 0x%8x\n",(MUINT32)rVirAddress);
            FLICKER_LOG("[allocM4UMemory] m4uVa = 0x%x \n", stRawFlkM4uInfo.u4VirAddr);
            setFlickerGMCConfig((MUINT32)rPhyAddress);
            m_pVectorAddress1 = (MINT32 *) (8*(((MUINT32)rVirAddress + 7)/8));
            m_iDataBank = 0;
        } else {
            FLICKER_LOG("!!!UNABLE to update pPhyAddr,pVirAddr!!!\n");
        }
#else
        if (m_pFlickerSysram == NULL) {
            FLICKER_LOG("Create sysram buffer\n");

            // ---------------------------------------------------
            //  Create isp sysram driver.
            m_pFlickerSysram = NSIspSysram::IspSysramDrv::createInstance();

            if (!m_pFlickerSysram) {
                FLICKER_ERR("Flicker::createInstance IspSysramDrv fail \n");
                return ret;
            }

            sysram_alloc(NSIspSysram::EUsr_Flicker, 29832, rPhyAddress, rVirAddress);    // 12* (8M heigh)
            setFlickerGMCConfig((MUINT32)rPhyAddress);
            m_pVectorAddress1 = (MINT32 *) (8*(((MUINT32)rVirAddress + 7)/8));
            m_iDataBank = 0;
        }
#endif

        setFlickerDrv(1, m_bFlickerEnable);    // Save the column vector and difference
    } else {
        i4FlickerStatus = ISP_REG(m_pIspRegMap, CAM_FLK_CON);
        setFlickerDrv(1, m_bFlickerEnable);    // disable theflicker
#if MTK_FLK_M4U_EN
#else
        if (m_pFlickerSysram) {
        	  for(i4Index=0; i4Index < 100; i4Index++) {   //total time 42 + 4*90 = 402ms
                    i4FlickerStatus = ISP_REG(m_pIspRegMap, CAM_FLK_CON);
       	      if(((i4FlickerStatus & 0x0400) != 0) || ((i4FlickerStatus & 0x08000000) == 0)) {   // the flicker stop  bit10 flk gmc ok and bit27 double buffer value of flk_en
       	          break;
       	      }
       	      //Wait 1ms for PLL stable
                    usleep(i4PollingTime*1000);  // sleep time 10ms, 8ms, 6ms, 4ms, 4ms, 4ms.....
       	      i4PollingTime -= 2;
       	      if(i4PollingTime < 4) {
                        i4PollingTime = 4;
       	      }
        	  }
        	  if(i4Index == 100) {
                    FLICKER_ERR("Wait flicker stop timeout \n");
        	  }
            // free sysram buffer
//            FLICKER_LOG("Free sysram buffer3: 0x%08x\n", i4FlickerStatus);
            sysram_free(NSIspSysram::EUsr_Flicker);

            m_pFlickerSysram->destroyInstance();
            m_pFlickerSysram = NULL;
        }
#endif
    }
//    FLICKER_LOG("Flicker enable:%d\n", bEnableFlicker);
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 FlickerHal::setWindowInfo(
    MUINT32 a_u4Width,
    MUINT32 a_u4Height
)
{
    MUINT32 u4Height;
    MUINT32 u4Width;
    MUINT32 u4PixelEnd, u4PixelStart, u4LineEnd, u4LineStart;
    MINT32 i4FlickerStatus = 0;

    FLICKER_LOG("[setWindowInfo] width:%d %d height:%d %d\n", a_u4Width, m_u4FlickerWidth, a_u4Height, m_u4FlickerHeight);

    if ((a_u4Width != 0) && (a_u4Height != 0)) {
#if 0
    	 if((m_u4FlickerWidth != a_u4Width) || (m_u4FlickerHeight != a_u4Height)) {
            m_u4FlickerWidth = a_u4Width;
            m_u4FlickerHeight = a_u4Height;
#else
        u4PixelEnd = ISP_BITS(m_pIspRegMap, CAM_GRABCOL, PIXEL_END);
        u4PixelStart =  ISP_BITS(m_pIspRegMap, CAM_GRABCOL, PIXEL_START);
        u4LineEnd = ISP_BITS(m_pIspRegMap, CAM_GRABROW, LINE_END);
        u4LineStart =  ISP_BITS(m_pIspRegMap, CAM_GRABROW, LINE_START);
        u4Width = u4PixelEnd - u4PixelStart + 1 - 4;
        u4Height = u4LineEnd - u4LineStart + 1 -6;

        FLICKER_LOG("[setWindowInfo] %d %d %d %d %d %d",
        	(int)u4PixelEnd, (int)u4PixelStart, (int)u4LineEnd, (int)u4LineStart, (int)u4Width, (int)u4Height);

    	 if((m_u4FlickerWidth != u4Width) || (m_u4FlickerHeight != u4Height)) {
            m_u4FlickerWidth = u4Width;
            m_u4FlickerHeight = u4Height;
            FLICKER_LOG("[setWindowInfo] width:%d %d height:%d %d\n", u4Width, m_u4FlickerWidth, u4Height, m_u4FlickerHeight);
#endif
            if(m_u4FlickerHeight > FLICKER_MAX_LENG){
                u4Height = FLICKER_MAX_LENG;
            } else {
                u4Height = m_u4FlickerHeight;
            }

            FLICKER_LOG("[setWindowInfo] %d %d", (int)u4Height, (int)FLICKER_MAX_LENG);

            i4FlickerStatus = ISP_REG(m_pIspRegMap, CAM_FLK_CON);
            if(i4FlickerStatus & 0x01) {
                FLICKER_LOG("[setWindowInfo] Flicker enable already 0x%8x\n",(MUINT32)i4FlickerStatus);
            } else {
            	FLICKER_LOG("[setWindowInfo] %d %d\n",(MUINT32)m_u4FlickerWidth, (int)u4Height);

            setFlickerWinConfig((MUINT32)(m_u4FlickerWidth/3), (MUINT32)(u4Height/3));
            m_pVectorAddress2 = (MINT32 *) (8*(((MUINT32)m_pVectorAddress1 + 2*3*m_u4FlickerHeight + 7)/8));
            }
    	 }
        return 0;
    }

    return -1;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 FlickerHal::getFlickerStatus(MINT32 *a_flickerStatus)
{
    if(m_bFlickerEnable == MFALSE) {
    	*a_flickerStatus = HAL_FLICKER_AUTO_OFF;
//    	FLICKER_LOG("[getFlickerStatus] Flicker disable\n");
    } else {
        *a_flickerStatus = m_u4FlickerFreq;
    }

    return 0;
}

