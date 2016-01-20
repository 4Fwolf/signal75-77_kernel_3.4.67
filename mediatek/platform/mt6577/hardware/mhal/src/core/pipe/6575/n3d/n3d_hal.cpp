/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <utils/threads.h>
#include <cutils/xlog.h>

#include "MTKGia.h"
#include "n3d_hal.h"


/*******************************************************************************
*
********************************************************************************/
#define N3D_DEBUG

#ifdef N3D_DEBUG
#define N3D_HAL_TAG             "[N3D_hal]"
#define N3D_LOG(fmt, arg...)    XLOGD(N3D_HAL_TAG fmt, ##arg)
#define N3D_ERR(fmt, arg...)    XLOGE(N3D_HAL_TAG "Err: %5d: "fmt, __LINE__, ##arg)
#else
#define N3D_LOG(a,...)
#define N3D_ERR(a,...)
#endif

/*******************************************************************************
*
********************************************************************************/
N3DHalBase* N3DHalBase::createInstance()
{
    return N3DHal::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
N3DHalBase* N3DHal::getInstance()
{
    N3D_LOG("getInstance \n");
    static N3DHal singleton;

    if (singleton.init() != 0)  {
        N3D_LOG("singleton.init() fail \n");        
        return NULL;
    }    
        
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void N3DHal::destroyInstance() 
{    
    uninit();
}

/*******************************************************************************
*
********************************************************************************/
N3DHal::N3DHal():N3DHalBase()
{
    mUsers = 0;
    m_pGiaApp = NULL;
    m_i4InitCnt = 0;
    m_i4FileCnt = 0;

    for (MINT32 i=0; i<LEARN_DATA_LENGTH; i++)
    {
        m_u4LearningData[i] = 0;
    }
}

/*******************************************************************************
*
********************************************************************************/
N3DHal::~N3DHal()
{
    m_i4FileCnt = 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 N3DHal::init()
{
    N3D_LOG("init - mUsers: %d \n", mUsers);

    Mutex::Autolock lock(mLock);

    if (mUsers > 0) {
        N3D_LOG("%d has created \n", mUsers);
        android_atomic_inc(&mUsers);
        return 0;
    }

    // ---------------------------------------------------
    m_pGiaApp = MTKGia::createInstance();

    if (!m_pGiaApp) {
        N3D_ERR("GiaApp::createInstance fail \n");
        goto create_fail_exit;
    }
    
    android_atomic_inc(&mUsers);

    return 0;

create_fail_exit:	

    if (m_pGiaApp) {
        m_pGiaApp->destroyInstance();
        m_pGiaApp = NULL;
    }

    return 0;
}

/*******************************************************************************
*
********************************************************************************/
void N3DHal::uninit()
{    
    N3D_LOG("uninit - mUsers: %d \n", mUsers);

    Mutex::Autolock lock(mLock);

    if (mUsers <= 0) {
        // No more users
        return;
    }
    // More than one user
    android_atomic_dec(&mUsers);
    //
    if (mUsers == 0) {

        if (m_pGiaApp) {
            m_pGiaApp->destroyInstance();
            m_pGiaApp = NULL;
        }        
    }
    else {
        N3D_LOG("Still %d users \n", mUsers);
    }

    return;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 N3DHal::N3DInit(IN_DATA_N3D_T InData)
{
    Mutex::Autolock lock(mLock);

    if (m_i4InitCnt > 0) {
        N3D_LOG("%d has inited \n", m_i4InitCnt);
        return S_GIA_OK;
    }

    if (m_pGiaApp == NULL) {
        N3D_ERR("[N3DInit] m_pGiaApp Null \n");
        return E_GIA_NULL_OBJECT;
    }

    MINT32 ret;
    GIA_SET_ENV_INFO_STRUCT InitInData;

    InitInData.source_image_width      = InData.originalImg.width;
    InitInData.source_image_height     = InData.originalImg.height;
    InitInData.crop_image_width        = InData.targetImg.width;
    InitInData.crop_image_height       = InData.targetImg.height;
    InitInData.scenario                = (GIA_SCENARIO_ENUM)InData.eScenario;
    InitInData.learning_data           = m_u4LearningData;

    InitInData.tuning_para.enable_tuning    = 0; // use default value
    InitInData.format                       = (GIA_SOURCE_FORMAT_ENUM)InData.eFormat;
    InitInData.stride                       = InData.originalImg.width; // xxxx TBD

    if(InData.eScenario == SCENARIO_VIDEO_PLAYBACK)
        InitInData.stride *= 2;

    ret = m_pGiaApp->GiaInit(&InitInData);

    if (ret != S_GIA_OK)
    {
        N3D_ERR("N3D init fail\n");
        return ret;
    }

    /*********************** Set Working buffer ********************/
    MUINT32 WorkBufSize;
    GIA_SET_WORK_BUF_INFO_STRUCT WorkBufInfo;

    m_pGiaApp->GiaFeatureCtrl(GIA_FEATURE_GET_WORK_BUF_INFO, NULL, &WorkBufSize);
    
    m_pWorkBuf = new char [WorkBufSize];
    
    if(m_pWorkBuf == NULL)
    {
        N3D_ERR("alloc working buffer fail\n");
        return E_GIA_NOT_ENOUGH_MEM;
    }
      
    WorkBufInfo.ext_mem_size = WorkBufSize;
    WorkBufInfo.ext_mem_start_addr = (MUINT32)m_pWorkBuf;
    ret = m_pGiaApp->GiaFeatureCtrl(GIA_FEATURE_SET_WORK_BUF_INFO, &WorkBufInfo, NULL);

    if (ret != S_GIA_OK)
    {
        N3D_ERR("set working buffer fail\n");
        return ret;
    }

    /*********************** init parameter ********************/
    //m_i4FileCnt = 0;

    
    android_atomic_inc(&m_i4InitCnt);        
        
    return S_GIA_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 N3DHal::N3DRun(RUN_DATA_N3D_T RunData, OUT_DATA_N3D_T &OutData)
{
    if (m_i4InitCnt <= 0) {
        N3D_ERR("not yet init\n");
        return E_GIA_ERR;
    }

    if (m_pGiaApp == NULL) {
        N3D_ERR("[N3DRun] m_pGiaApp Null \n");
        return E_GIA_NULL_OBJECT;
    }
    
    MINT32 ret;
    GIA_SET_PROC_INFO_STRUCT GiaProcInfo;
    GIA_RESULT_STRUCT GiaResult;

    GiaProcInfo.source_image_left_addr = RunData.u4BufAddrL;
    GiaProcInfo.source_image_right_addr = RunData.u4BufAddrR;

    ret = m_pGiaApp->GiaFeatureCtrl(GIA_FEATURE_SET_PROC_INFO,&GiaProcInfo,NULL);
    
    if (ret != S_GIA_OK)
    {
        N3D_ERR("set proc info fail\n");
        return ret;
    }

    ret = m_pGiaApp->GiaMain();

    if (ret != S_GIA_OK)
    {
        N3D_ERR("run fail\n");
        return ret;
    }

    /********************** Get log *************************/
    //if (m_i4FileCnt < 100)
/*    {
        ret = m_pGiaApp->GiaFeatureCtrl(GIA_FEATURE_SAVE_LOG, &m_i4FileCnt ,NULL);
        m_i4FileCnt++;
    }*/

    if (ret != S_GIA_OK)
    {
        N3D_ERR("get log fail\n");
        return ret;
    }

    /********************** Get Result ************************/    
    ret = m_pGiaApp->GiaFeatureCtrl(GIA_FEATURE_GET_RESULT,NULL,&GiaResult);

    if (ret != S_GIA_OK)
    {
        N3D_ERR("get result fail\n");
        return ret;
    }
    
    OutData.offsetL.width  = GiaResult.left_offset_x;
    OutData.offsetL.height = GiaResult.left_offset_y;
    OutData.offsetR.width  = GiaResult.right_offset_x;
    OutData.offsetR.height = GiaResult.right_offset_y;  

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 N3DHal::N3DReset()
{
    Mutex::Autolock lock(mLock);

    if (m_i4InitCnt <= 0) {
        N3D_LOG("reset already\n");
        return S_GIA_OK;
    }

    if (m_pGiaApp == NULL) {
        N3D_ERR("[N3DReset] m_pGiaApp Null \n");
        return E_GIA_NULL_OBJECT;
    }

    MINT32 ret;

    ret = m_pGiaApp->GiaReset();    

    if (ret != S_GIA_OK)
    {
        N3D_ERR("N3D reset fail\n");
        return ret;
    }

    free(m_pWorkBuf);
    
    android_atomic_dec(&m_i4InitCnt);
     
    return ret;    
}

