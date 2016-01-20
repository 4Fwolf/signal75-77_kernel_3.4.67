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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "mHalAsd"

#include <stdio.h>
#include <stdlib.h>

#include "../inc/MediaLog.h"
#include "../inc/MediaAssert.h"

#include "AppAsd.h"
#include "asd_hal.h"
#include "asd_hal_base.h"
#include <cutils/properties.h>

//#define   ASDdebug
#define   ASD_IMAGE_WIDTH  160
#define   ASD_IMAGE_HEIGHT 120  
#define   ASD_ORT          4
/*******************************************************************************
*
********************************************************************************/
static halASDBase *pHalASD = NULL;
static int mHalASDDumpOPT = 0;  
static int g_udCount;
/*******************************************************************************
*
********************************************************************************/
halASDBase*
halASD::
getInstance()
{
    MHAL_LOG("[halASD] getInstance \n");
    if (pHalASD == NULL) {
        pHalASD = new halASD();
    }
    return pHalASD;
}

/*******************************************************************************
*
********************************************************************************/
void   
halASD::
destroyInstance() 
{
    if (pHalASD) {
        delete pHalASD;
    }
    pHalASD = NULL;
}

/*******************************************************************************
*                                            
********************************************************************************/
halASD::halASD()
{
    m_pMTKAsdObj = NULL;  
    MHAL_LOG("[halASD] Reset g_udCount:%d \n", g_udCount);
    g_udCount=0;
}

/*******************************************************************************
*                                            
********************************************************************************/
halASD::~halASD()
{
}
 
MINT32 halASD::mHalAsdInit(void* AAAData,void* working_buffer,MUINT8 SensorType)
{
	  g_udCount++;
	  MHAL_LOG("[halASD]  g_udCount++:%d \n", g_udCount);
	  AAA_ASD_PARAM* rASDInfo=(AAA_ASD_PARAM*)AAAData;
	  MINT32 Retcode = S_ASD_OK;
	  MUINT32* AFtable=(MUINT32*)malloc((rASDInfo->i4AFTableIdxNum + 1)*sizeof(MUINT32));
	  //********************************************************************************//
	  //********Binchang 20110810 Add ASD Debug Opition****************//
	  char value[PROPERTY_VALUE_MAX] = {'\0'}; 
       property_get("ASD.debug.dump", value, "0");
       mHalASDDumpOPT = atoi(value); 
       //********************************************************************************//
       //********************************************************************************//

	  gMyAsdInitInfo.pInfo = &gMyAsdEnvInfo;
	  gMyAsdInitInfo.pDeciderInfo = &gMyDeciderEnvInfo;
	  gMyAsdInitInfo.pDeciderTuningInfo = &gMyDeciderTuningInfo;
    
	  gMyAsdInitInfo.pInfo->ext_mem_start_addr=(MUINT32)working_buffer;	// working buffer, size: ASD_WORKING_BUFFER_SIZE
	  gMyAsdInitInfo.pInfo->image_width=ASD_IMAGE_WIDTH;						// source image width (160)
	  gMyAsdInitInfo.pInfo->image_height	=ASD_IMAGE_HEIGHT;					// source image height (120)
	  gMyAsdInitInfo.pInfo->asd_tuning_data.num_of_ort=ASD_ORT;					// num_of_ort, drfault = 4

//*********************************************************************************************************************************************//
//Please fix me Bin
    if(SensorType)
    {
    	  MHAL_LOG("[YUV Sensor] \n");
    	  gMyAsdInitInfo.pDeciderInfo->DeciderInfoVer = ASD_DECIDER_INFO_SRC_YUV;	// Smart Phone Info 
    }
    else
    {
    	  MHAL_LOG("[SP_RAW_Sensor] \n");
    	  gMyAsdInitInfo.pDeciderInfo->DeciderInfoVer = ASD_DECIDER_INFO_SRC_SP_RAW;	// Smart Phone Info
    }
//*********************************************************************************************************************************************//
/*
    if(SensorType==ISP_SENSOR_TYPE_YUV)
    {
    	   gMyAsdInitInfo.pDeciderInfo->DeciderInfoVer = ASD_DECIDER_INFO_SRC_YUV;	// Smart Phone Info
    }
    else
    {
    	  gMyAsdInitInfo.pDeciderInfo->DeciderInfoVer = ASD_DECIDER_INFO_SRC_SP_RAW;	// Smart Phone Info
    }
*/
//*********************************************************************************************************************************************//
    gMyAsdInitInfo.pDeciderInfo->RefAwbD65Rgain = rASDInfo->i4AWBRgain_D65_X128;					// reference 3A info, get from sensor or AWB data
    gMyAsdInitInfo.pDeciderInfo->RefAwbD65Bgain = rASDInfo->i4AWBBgain_D65_X128;					// reference 3A info, get from sensor or AWB data
    gMyAsdInitInfo.pDeciderInfo->RefAwbCwfRgain = rASDInfo->i4AWBRgain_CWF_X128;					// reference 3A info, get from sensor or AWB data
    gMyAsdInitInfo.pDeciderInfo->RefAwbCwfBgain = rASDInfo->i4AWBBgain_CWF_X128;					// reference 3A info, get from sensor or AWB data
    *AFtable=rASDInfo->i4AFTableMacroIdx;
    memcpy((AFtable+1),rASDInfo->pAFTable,(rASDInfo->i4AFTableIdxNum*sizeof(MUINT32)));
    
    MHAL_LOG("[mHalAsdInit] i4AFTableMacroIdx %d i4AFTableIdxNum %d i4AFTableOffset %d\n",rASDInfo->i4AFTableMacroIdx,rASDInfo->i4AFTableIdxNum,rASDInfo->i4AFTableOffset);
    
    for(int i=1;i<rASDInfo->i4AFTableIdxNum+1;i++)
    {
    	   *(AFtable+i) =  *(AFtable+i) + rASDInfo->i4AFTableOffset; 
        MHAL_LOG("[mHalAsdInit] After (Offset) AF table %d \n",*(AFtable+i));
    }
    
    //BinChang 20121228
    //gMyAsdInitInfo.pDeciderInfo->RefAfTbl = (void *)AFtable;					// reference 3A info, get from lens or AF data
    if(rASDInfo->i4AFTableMacroIdx != 0)
        gMyAsdInitInfo.pDeciderInfo->RefAfTbl = (void *)AFtable;
   else
        gMyAsdInitInfo.pDeciderInfo->RefAfTbl = NULL;
    
    
	  gMyAsdInitInfo.pDeciderTuningInfo = 0;						// use default value
	  
	  if (m_pMTKAsdObj) {
        Retcode = E_ASD_ERR;
        MHAL_LOG("[v] Err, Init has been called \n");
    }    

    /*  Create MTKPano Interface  */
    m_pMTKAsdObj = MTKAsd::createInstance(DRV_ASD_OBJ_SW);
    MHAL_ASSERT(m_pMTKAsdObj != NULL, "Err");
    
	  m_pMTKAsdObj->AsdInit(&gMyAsdInitInfo, 0);
	  
    if (AFtable) {
        free(AFtable);
    }
	  return Retcode;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 halASD::mHalAsdUnInit( )
{
    MHAL_LOG("[mHalAsdUnInit] \n");
    
    if(mHalASDDumpOPT)
    //if(1)
    {
    	  MHAL_LOG("[mHalAsdUnInit] Save log \n");
    	  m_pMTKAsdObj->AsdFeatureCtrl(ASD_FEATURE_SAVE_DECIDER_LOG_INFO, &g_udCount, 0);
    }
    if (m_pMTKAsdObj) {
        m_pMTKAsdObj->destroyInstance();
    }
    m_pMTKAsdObj = NULL;

    MHAL_LOG("[mHalAsdUnInit] OK\n");

    return S_ASD_OK;
}
#ifdef ASDdebug
UINT32 filecun=0;
#endif 
MINT32 halASD::mHalAsdDoSceneDet(void* src, MUINT16 imgw, MUINT16 imgh)
{
	  MINT32 Retcode = S_ASD_OK;
//	  MHAL_LOG("[mHalAsdDoSceneDet] \n");
	  MUINT16* img = (MUINT16*)src;
	  MUINT16 times;
	  MUINT32 count=0;
	  ASD_SCD_INFO_STRUCT gMyAsdInfo;
	  
	  if((imgw%ASD_IM_WIDTH!=0)||(imgh%ASD_IM_HEIGHT!=0)||((imgh%ASD_IM_HEIGHT)!=(imgw%ASD_IM_WIDTH)))
	  {
	  	 MHAL_LOG("[mHalAsdDoSceneDet] can not resize img\n");
	  	 Retcode=E_ASD_WRONG_CMD_PARAM;
	  	 return Retcode;
	  }
	  times=imgw/ASD_IM_WIDTH;
	  for(int i=0;i<imgh;i+=times)
	  {
	  	 for(int j=0;j<imgw;j+=times)
	  	 {
	  	    *(img+count)=*(img+(i*imgw)+j);
	  	    count++;
	  	 }
	  }
    #ifdef ASDdebug
	  UCHAR szFileName[100];            
    sprintf(szFileName, "//sdcard//ASDResize%04d_%04d_%04d.raw", 160, 120,filecun); 
	  filecun++;
	  FILE * pRawFp = fopen(szFileName, "wb");
      
    if (NULL == pRawFp )
    {
        LOGD("Can't open file to save RAW Image\n"); 
        while(1);
    }
      
    int i4WriteCnt = fwrite((void *)img,2, (160 * 120 * 1),pRawFp);
    fflush(pRawFp); 
    fclose(pRawFp);
    #endif  
    gMyAsdInfo.src_buffer_addr = (MUINT32)src;
//    MHAL_LOG("[mHalAsdDoSceneDet] src 0x%x \n",(MUINT32)src);
		m_pMTKAsdObj->AsdMain(ASD_PROC_MAIN, &gMyAsdInfo);

		return Retcode;
}

MINT32 halASD::mHalAsdDecider(void* AAAData,void* FDResult,mhal_ASD_DECIDER_UI_SCENE_TYPE_ENUM &Scene)
{
	  MINT32 Retcode = S_ASD_OK;
	  AAA_ASD_PARAM* rASDInfo=(AAA_ASD_PARAM*)AAAData;
	  //MUINT8 FDResultInfo[1024];
	 
//	  MHAL_LOG("[mHalAsdDecider] \n");
//	  result* FDReslutInfo=(result*)FDResult;
       camera_face_metadata_m* FDResultInfo=(camera_face_metadata_m*) FDResult;
       //camera_face_m *faces;
       //FDResultInfo->faces = faces;

	  //AAA_ASD_PARAM* ASDInfo=(AAA_ASD_PARAM*)AAAData;
	  ASD_DECIDER_RESULT_STRUCT MyDeciderResult;
	  ASD_SCD_RESULT_STRUCT gMyAsdResultInfo;
	  ASD_DECIDER_INFO_STRUCT gMyDeciderInfo;
		
		gMyDeciderInfo.InfoFd.FdFaceNum=FDResultInfo->number_of_faces;
		gMyDeciderInfo.InfoFd.FdMainFaceX0=(FDResultInfo->faces[0].rect[0] + 1000);  // for positive > 0
		gMyDeciderInfo.InfoFd.FdMainFaceY0=(FDResultInfo->faces[0].rect[1] + 1000);  // for positive > 0
		gMyDeciderInfo.InfoFd.FdMainFaceX1=(FDResultInfo->faces[0].rect[2] + 1000);  // for positive > 0
		gMyDeciderInfo.InfoFd.FdMainFaceY1=(FDResultInfo->faces[0].rect[3] + 1000);  // for positive > 0
		//gMyDeciderInfo.InfoFd.FdMainFacePose=(FDReslutInfo->rop_dir<<4)+ FDReslutInfo->rip_dir;
		gMyDeciderInfo.InfoFd.FdMainFacePose=0;
		gMyDeciderInfo.InfoFd.FdMainFaceLuma = 0;
		   
		gMyDeciderInfo.InfoAaa.AeEv = rASDInfo->i4AELv_x10;					// reference 3A info, get from sensor or AWB data
		gMyDeciderInfo.InfoAaa.AeFaceEnhanceEv = rASDInfo->i2AEFaceDiffIndex;      // reference 3A info, get Face AE enhance Ev value */    
    gMyDeciderInfo.InfoAaa.AeIsBacklit = rASDInfo->bAEBacklit;					// reference 3A info, get from sensor or AWB data
    gMyDeciderInfo.InfoAaa.AeIsStable = rASDInfo->bAEStable;					// reference 3A info, get from sensor or AWB data
    gMyDeciderInfo.InfoAaa.AwbCurRgain = rASDInfo->i4AWBRgain_X128;					// reference 3A info, get from sensor or AWB data
    gMyDeciderInfo.InfoAaa.AwbCurBgain = rASDInfo->i4AWBBgain_X128;					// reference 3A info, get from lens or AF data
	  gMyDeciderInfo.InfoAaa.AwbIsStable = rASDInfo->bAWBStable;
	  gMyDeciderInfo.InfoAaa.AfPosition = rASDInfo->i4AFPos;
	  gMyDeciderInfo.InfoAaa.AfIsStable = rASDInfo->bAFStable;
	  
	  //memcpy((void*)&gMyDeciderInfo.InfoAaa,AAAData, sizeof(AAA_ASD_PARAM));		  	  
	  m_pMTKAsdObj->AsdFeatureCtrl(ASD_FEATURE_GET_RESULT, 0, &gMyAsdResultInfo);
	  memcpy(&(gMyDeciderInfo.InfoScd),&gMyAsdResultInfo, sizeof(ASD_SCD_RESULT_STRUCT));		
    m_pMTKAsdObj->AsdMain(ASD_PROC_DECIDER, &gMyDeciderInfo);
    m_pMTKAsdObj->AsdFeatureCtrl(DECIDER_FEATURE_GET_RESULT, 0, &MyDeciderResult);
    MHAL_LOG("[mHalAsdDecider] detect Scene is %d, Face Num:%d \n",MyDeciderResult.DeciderUiScene, gMyDeciderInfo.InfoFd.FdFaceNum);
    
    //Scene=MyDeciderResult.DeciderUiScene;
    Scene=(mhal_ASD_DECIDER_UI_SCENE_TYPE_ENUM) MyDeciderResult.DeciderUiScene;
        
    //Scene=mhal_ASD_DECIDER_UI_AUTO;
    return Retcode;
}


