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
#define LOG_TAG "mHalAutorama"

#include "../inc/MediaLog.h"
#include "../inc/MediaAssert.h"

#include "AppAutorama.h"
#include "autorama_hal.h"
#include "autorama_hal_base.h"

/*******************************************************************************
*
********************************************************************************/
static halAUTORAMABase *pHalAUTORAMA = NULL;
MUINT8 skipcnt=0;
/*******************************************************************************
*
********************************************************************************/
halAUTORAMABase*
halAUTORAMA::
getInstance()
{
    MHAL_LOG("[halAUTORAMA] getInstance \n");
    if (pHalAUTORAMA == NULL) {
        pHalAUTORAMA = new halAUTORAMA();
    }
    return pHalAUTORAMA;
}

/*******************************************************************************
*
********************************************************************************/
void   
halAUTORAMA::
destroyInstance() 
{
    if (pHalAUTORAMA) {
        delete pHalAUTORAMA;
    }
    pHalAUTORAMA = NULL;
}

/*******************************************************************************
*                                            
********************************************************************************/
halAUTORAMA::halAUTORAMA()
{
    m_pMTKAutoramaObj = NULL;  
    m_pMTKMotionObj = NULL;  
}

/*******************************************************************************
*                                            
********************************************************************************/
halAUTORAMA::~halAUTORAMA()
{    
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halAUTORAMA::mHalAutoramaInit(MTKPipeAutoramaEnvInfo AutoramaInitInData
)
{
    MTKAutoramaEnvInfo AutoramaEnvInfo;
    MTKMotionEnvInfo MyMotionEnvInfo;
    MTKMotionTuningPara MyMotionTuningPara;
    MRESULT Retcode = S_AUTORAMA_OK;
    
    MHAL_LOG("[mHalAutoramaInit] \n");
    
    skipcnt=0;
    
    if (m_pMTKAutoramaObj) {
        Retcode = E_AUTORAMA_ERR;
        MHAL_LOG("[mHalAutoramaInit] Err, Init has been called \n");
    }    

    /*  Create MTKPano Interface  */
    m_pMTKAutoramaObj = MTKAutorama::createInstance(DRV_AUTORAMA_OBJ_SW);
    MHAL_ASSERT(m_pMTKAutoramaObj != NULL, "Err");
    /*  Pano Driver Init  */
    /*  Set Pano Driver Environment Parameters */
    AutoramaEnvInfo.SrcImgWidth = AutoramaInitInData.SrcImgWidth;
    AutoramaEnvInfo.SrcImgHeight = AutoramaInitInData.SrcImgHeight;
    AutoramaEnvInfo.MaxPanoImgWidth = AutoramaInitInData.MaxPanoImgWidth;
    AutoramaEnvInfo.WorkingBufAddr = (MUINT32)AutoramaInitInData.WorkingBufAddr;
    AutoramaEnvInfo.WorkingBufSize = AutoramaInitInData.WorkingBufSize;
    AutoramaEnvInfo.MaxSnapshotNumber = AutoramaInitInData.MaxSnapshotNumber;
    AutoramaEnvInfo.FixAE = AutoramaInitInData.FixAE;
    AutoramaEnvInfo.FocalLength = AutoramaInitInData.FocalLength;
    AutoramaEnvInfo.GPUWarp = AutoramaInitInData.GPUWarp;
    AutoramaEnvInfo.SrcImgFormat = MTKAUTORAMA_IMAGE_YV12;
    //AutoramaEnvInfo.StitchDirection = MTKAUTORAMA_DIR_RIGHT;
    Retcode = m_pMTKAutoramaObj->AutoramaInit(&AutoramaEnvInfo, 0);
    
    if (m_pMTKMotionObj) 
        MHAL_LOG("[mHalAutoramaInit] m_pMTKMotionObj Init has been called \n");    
    else
        m_pMTKMotionObj = MTKMotion::createInstance(DRV_MOTION_OBJ_PANO);
    MyMotionEnvInfo.WorkingBuffAddr = (MUINT32)(AutoramaInitInData.WorkingBufAddr+(AutoramaInitInData.WorkingBufSize));
    MyMotionEnvInfo.pTuningPara = &MyMotionTuningPara;
    MyMotionEnvInfo.SrcImgWidth = MOTION_MAX_IN_WIDTH;
    MyMotionEnvInfo.SrcImgHeight = MOTION_MAX_IN_HEIGHT;
    MyMotionEnvInfo.WorkingBuffSize = MOTION_WORKING_BUFFER_SIZE;
    MyMotionEnvInfo.pTuningPara->OverlapRatio = OVERLAP_RATIO;
    m_pMTKMotionObj->MotionInit(&MyMotionEnvInfo, NULL);
    MHAL_ASSERT(m_pMTKMotionObj != NULL, "Err");
      
    return Retcode;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halAUTORAMA::mHalAutoramaUninit(
)
{
    MHAL_LOG("[mHalAutoramaUninit] \n");
    if (m_pMTKMotionObj) {
        m_pMTKMotionObj->MotionExit();
        m_pMTKMotionObj->destroyInstance();
    }
    m_pMTKMotionObj = NULL;
    
    if (m_pMTKAutoramaObj) {
        m_pMTKAutoramaObj->AutoramaExit();
        m_pMTKAutoramaObj->destroyInstance();
    }
    m_pMTKAutoramaObj = NULL;

    return S_AUTORAMA_OK;
}
/*******************************************************************************
*
********************************************************************************/
MINT32 halAUTORAMA::mHalAutoramaCalcStitch(void* SrcImg,MINT32 gEv,MTKPIPEAUTORAMA_DIRECTION_ENUM DIRECTION
)
{
    MINT32 Retcode = S_AUTORAMA_OK;

    MHAL_LOG("[mHalAutoramaCalcStitch] \n");
    
    MTKAutoramaProcInfo AutoramaProcInfo;
    
    AutoramaProcInfo.AutoramaCtrlEnum = MTKAUTORAMA_CTRL_ADD_IMAGE;    
    AutoramaProcInfo.SrcImgAddr = (MUINT32)SrcImg;
    AutoramaProcInfo.EV = gEv;
    AutoramaProcInfo.StitchDirection=(MTKAUTORAMA_DIRECTION_ENUM)DIRECTION;
    Retcode = m_pMTKAutoramaObj->AutoramaFeatureCtrl(MTKAUTORAMA_FEATURE_SET_PROC_INFO, &AutoramaProcInfo, 0);
    if(Retcode!=S_AUTORAMA_OK)
    {
    	  MHAL_LOG("[mHalAutoramaCalcStitch] MTKAUTORAMA_FEATURE_SET_PROC_INFO Fail \n");
    	  return Retcode;
    }
    Retcode = m_pMTKAutoramaObj->AutoramaMain();
    if(Retcode!=S_AUTORAMA_OK)
    {
    	  MHAL_LOG("[mHalAutoramaCalcStitch] AutoramaMain Fail\n");
    }
    return Retcode;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 halAUTORAMA::mHalAutoramaDoStitch(
)
{
    MINT32 Retcode = S_AUTORAMA_OK;
    MTKAutoramaProcInfo AutoramaProcInfo;
    
    MHAL_LOG("[mHalAutoramaDoStitch] \n");

    AutoramaProcInfo.AutoramaCtrlEnum = MTKAUTORAMA_CTRL_STITCH;
    Retcode = m_pMTKAutoramaObj->AutoramaFeatureCtrl(MTKAUTORAMA_FEATURE_SET_PROC_INFO, &AutoramaProcInfo, 0);
    if(Retcode!=S_AUTORAMA_OK)
    {
    	  MHAL_LOG("[mHalAutoramaDoStitch] MTKAUTORAMA_FEATURE_SET_PROC_INFO Fail \n");
    	  return Retcode;
    }
     /* Stitching the images stored in Pano Driver */
    Retcode = m_pMTKAutoramaObj->AutoramaMain();
    if(Retcode!=S_AUTORAMA_OK)
    {
    	  MHAL_LOG("[mHalAutoramaCalcStitch] AutoramaMain Fail\n");
    }
    return Retcode;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 halAUTORAMA::mHalAutoramaGetResult(
MTKPipeAutoramaResultInfo* ResultInfo 
)
{
    MINT32 Retcode = S_AUTORAMA_OK;
    MTKAutoramaResultInfo AutoramaResultInfo;
    Retcode = m_pMTKAutoramaObj->AutoramaFeatureCtrl(MTKAUTORAMA_FEATURE_GET_RESULT, 0, &AutoramaResultInfo);
    if(Retcode!=S_AUTORAMA_OK)
    {
    	  MHAL_LOG("[mHalAutoramaGetResult] MTKAUTORAMA_FEATURE_GET_RESULT Fail\n");
    }
    MHAL_LOG("[mHalAutoramaGetResult] ImgWidth %d ImgHeight %d ImgBufferAddr 0x%x\n",AutoramaResultInfo.ImgWidth,AutoramaResultInfo.ImgHeight,AutoramaResultInfo.ImgBufferAddr);
    ResultInfo->ImgWidth=AutoramaResultInfo.ImgWidth;
    ResultInfo->ImgHeight=AutoramaResultInfo.ImgHeight;
    ResultInfo->ImgBufferAddr=AutoramaResultInfo.ImgBufferAddr;
    return Retcode;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halAUTORAMA::mHalAutoramaDoMotion(MUINT32* ImgSrc,MUINT32* MotionResult
)
{
	  MINT32 err = S_AUTORAMA_OK;
	  MTKMotionProcInfo MotionInfo;
	  skipcnt++;
	  if(skipcnt<3)
	  {
	  	 //MHAL_LOG("[mHalAutoramaDoMotion] return skipcnt %d \n",skipcnt);	
	  	 return err;
	  }
	  else 
	  	 skipcnt=3;
	  
    //MHAL_LOG("[mHalAutoramaDoMotion] skipcnt %d \n",skipcnt);	
    if (!m_pMTKMotionObj) {
        err = E_AUTORAMA_ERR;
        MHAL_LOG("[mHalAutoramaDoMotion] Err, Init has been called \n");
    }
    MotionInfo.ImgAddr = (MUINT32)ImgSrc;
    //MHAL_LOG("[mHalAutoramaDoMotion] ImgAddr 0x%x\n",MotionInfo.ImgAddr);
    m_pMTKMotionObj->MotionFeatureCtrl(MTKMOTION_FEATURE_SET_PROC_INFO, &MotionInfo, NULL);
    m_pMTKMotionObj->MotionMain();    
    m_pMTKMotionObj->MotionFeatureCtrl(MTKMOTION_FEATURE_GET_RESULT, NULL, MotionResult);
    return err;
}


