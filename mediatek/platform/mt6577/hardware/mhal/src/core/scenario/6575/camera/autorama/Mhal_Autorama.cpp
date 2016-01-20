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
#define LOG_TAG "mHalCam"

#include <cutils/xlog.h>
#include <stdlib.h>
#include <semaphore.h>
#include <linux/rtpm_prio.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/resource.h>

#include "Mhal_Autorama.h"
#include "mhal_misc.h"
#include "jpeg_hal.h"
#include "aaa_hal_base.h"
#include "mhal_cam.h"
#include "ICameraIO.h"

#include "CamJpg.h"


/*******************************************************************************
*
********************************************************************************/

static sem_t CompressSem;
static pthread_t AUTORAMAFuncThread;
static mhal_Autorama *mhalAutoramaObj;

/*******************************************************************************
*
********************************************************************************/
mhal_Autorama*
mhal_Autorama::
createInstance(NSCamera::ESensorType const eSensorType)
{
    return new mhal_Autorama(eSensorType);
}


/*******************************************************************************
*
********************************************************************************/
mhal_Autorama::mhal_Autorama(NSCamera::ESensorType const eSensorType)
    : mhal_featureBase("AUTORAMA")
    , meSensorType(eSensorType)
    , mpAUTORAMAWorkingBuf(NULL)
    , mJPGFrameAddr(0)
    , mAUTORAMAFrameIdx(0)
    , mergeCnt(1)
{

    MY_DBG("mhal_Autorama constructor");
    ::memset(&mAUTORAMAFrame, 0, sizeof(mhalCamFrame_t)); 
    mhalAutoramaObj = NULL;
    mpAUTORAMAObj = halAUTORAMABase::createInstance(HAL_AUTORAMA_OBJ_AUTO); 
    setState(MHAL_CAM_FEATURE_STATE_IDLE);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
mhal_Autorama::
destroyInstance()
{
    MY_DBG("~mhal_Autorama()");

    delete this;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 
mhal_Autorama::
init(
    mHalCam* const pMhalCam, 
    Hal3ABase* const pHal3AObj, 
    ICameraIO* const pCameraIOObj,
    mHalCamMemPool* const pFDWorkingBuf,
    MINT32 sOrientation
)
{
    //
    if  ( ! pMhalCam )
    {
        MY_ERR("[mHalCamInitMAV] pMhalCam == NULL");
        return  -1;
    }
    mpMhalCam = pMhalCam;

    //
    if  ( ! pHal3AObj )
    {
        MY_ERR("[mHalCamInitMAV] pHal3AObj == NULL");
        return  -1;
    }
    mpHal3AObj = pHal3AObj;   

    //
    if  ( ! pCameraIOObj )
    {
        MY_ERR("[mHalCamInitMAV] pCameraIOObj == NULL");
        return  -1;
    }
    mpCameraIOObj = pCameraIOObj;

    //
    if  ( ! pFDWorkingBuf )
    {
        MY_ERR("[mHalCamInitMAV] pFDWorkingBuf == NULL");
        return  -1;
    }
	
    //
    mFDWorkingBuf = pFDWorkingBuf;
    sensorOrientation = sOrientation;
    mhalAutoramaObj = this;
	
    sem_init(&CompressSem, 0, 0);

    return MHAL_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
mhal_Autorama::
setParameters(
    mhalCamParam_t *pmhalCamParam
)
{
    MINT32 err = MHAL_NO_ERROR;
    strcpy((char *) outputFile, (char *)pmhalCamParam->u1FileName);
    frmYuv_virtAddr = pmhalCamParam->frmYuv.virtAddr;
    frmYuv_size = pmhalCamParam->frmYuv.frmSize;
    AutoramaNum = pmhalCamParam->u4BusrtNo;

    mAUTORAMAFrame.w = pmhalCamParam->frmYuv.w;
    mAUTORAMAFrame.h = pmhalCamParam->frmYuv.h;
    mAUTORAMAFrame.frmSize = mAUTORAMAFrame.w * mAUTORAMAFrame.h * 3 / 2;  // YV12 now
    mAUTORAMAFrame.frmCount = AutoramaNum;    // Maximum is 10
    mAUTORAMAFrame.bufSize = mAUTORAMAFrame.frmSize * mAUTORAMAFrame.frmCount;
    mAUTORAMAFrame.virtAddr = (MUINT32) malloc(mAUTORAMAFrame.bufSize);
    if (mAUTORAMAFrame.virtAddr == 0)
    {
        MY_ERR("mAUTORAMAFrame.virtAddr == 0");
        return  MHAL_INVALID_MEMORY;
    }
    
    mStitchDir = MTKPIPEAUTORAMA_DIR_NO;
    mAUTORAMAFrameIdx = 0;
    mpmhalCamParam = pmhalCamParam;

    return err;
} 

/*******************************************************************************
*
********************************************************************************/
MINT32 
mhal_Autorama::
mHalCamFeatureInit(
    mhalCamParam_t *pmhalCamParam
)
{
    MY_DBG("[mHalCamAUTORAMAInit]");
    MINT32 err = MHAL_NO_ERROR;
    
    // (1) Set global variable
    err = setParameters(pmhalCamParam);
    if ( err != MHAL_NO_ERROR) return err;
    MY_DBG("[mHalCamAUTORAMAInit] num: %d", AutoramaNum);
    MY_DBG("[mHalCamAUTORAMAInit] %d/%d", pmhalCamParam->frmYuv.w, pmhalCamParam->frmYuv.h);

    
    // (2) Create working buffer
    MINT32 initBufSize = pmhalCamParam->frmYuv.w * pmhalCamParam->frmYuv.h * 4 * 10;
    MINT32 motionBufSize = MOTION_PIPE_WORKING_BUFFER_SIZE;

    mpAUTORAMAWorkingBuf = (MINT8 *) malloc(initBufSize + motionBufSize);
    if (mpAUTORAMAWorkingBuf == NULL) {
       MY_ERR("[mHalCamAUTORAMAInit] Err, mpAUTORAMAWorkingBuf == NULL");
       //mHalCamFeatureUninit();
       return MHAL_INVALID_MEMORY;
    }

    MTKPipeAutoramaEnvInfo mAutoramaInitInData; 
    mAutoramaInitInData.SrcImgWidth = pmhalCamParam->frmYuv.w ;
    mAutoramaInitInData.SrcImgHeight = pmhalCamParam->frmYuv.h;
    mAutoramaInitInData.MaxPanoImgWidth = AUTORAMA_MAX_OUT_WIDTH;
    mAutoramaInitInData.WorkingBufAddr = (MUINT32)mpAUTORAMAWorkingBuf;
    mAutoramaInitInData.WorkingBufSize = initBufSize;
    mAutoramaInitInData.MaxSnapshotNumber = AutoramaNum;
    mAutoramaInitInData.FixAE = meSensorType;
    mAutoramaInitInData.FocalLength = 750;
    mAutoramaInitInData.GPUWarp = 0;      

    MUINT32 jpegBufsize = ((mAUTORAMAFrame.bufSize) + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
    mJPGFrameAddr = (MUINT32)memalign (L1_CACHE_BYTES, jpegBufsize);
    MY_DBG("L1_CACHE_BYTES: %d, mJPEGFrameAddr: %x, jpegBufSize: %d", L1_CACHE_BYTES, mJPGFrameAddr, jpegBufsize);
    if ( (mJPGFrameAddr & (L1_CACHE_BYTES-1)) > 0) {
        MY_ERR("[mHalCamAUTORAMAInit] Err,mJPGFrameAddr is not 32-bytes aligned --> return error");
        return MHAL_INVALID_MEMORY;
    }

    err = mpAUTORAMAObj->mHalAutoramaInit(mAutoramaInitInData);
    if ( err < 0) {
        MY_ERR("  mpAUTORAMAObj->mHalAutoramaInit Err \n");
        return err;
    }
	
    setState(MHAL_CAM_FEATURE_STATE_CAPTURE);

    return MHAL_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MVOID 
mhal_Autorama::
mHalCamFeatureUninit()
{
    MY_DBG("[mHalCamAUTORAMAUninit]");
    //
    /* Mutex at 2 plases: Uninit() and Proc() */    
    Mutex::Autolock lock(mLock);
    setState(MHAL_CAM_FEATURE_STATE_UNINIT);
    //
    mpMhalCam->mHalCam3ACtrl(1, 1, 1); // go back to default setting

    if ( mpAUTORAMAObj ) {
        mpAUTORAMAObj->mHalAutoramaUninit();
        mpAUTORAMAObj = NULL;
    }

    if (mpAUTORAMAWorkingBuf) {
        free(mpAUTORAMAWorkingBuf);
        mpAUTORAMAWorkingBuf = NULL;
    }

    if (mAUTORAMAFrame.virtAddr) {
        free((UINT8 *) mAUTORAMAFrame.virtAddr);
        mAUTORAMAFrame.virtAddr = 0;
    }

    if (mJPGFrameAddr) {
        free((UINT8 *) mJPGFrameAddr);
        mJPGFrameAddr = 0;
    }
}


/******************************************************************************
*
*******************************************************************************/
MINT32
mhal_Autorama::
getPriority(int policy)
{
    int priority;
    policy = sched_getscheduler(0);
    if (policy == SCHED_OTHER) {
        // a conventional process has only the static priority
        priority = getpriority(PRIO_PROCESS, 0);
    } else {
        // a real-time process has both the static priority and real-time priority.
        struct sched_param sched_p;
        sched_getparam(0, &sched_p);
        priority = sched_p.sched_priority;
    }

    return priority;
}

/******************************************************************************
*
*******************************************************************************/
MVOID 
mhal_Autorama::
setPriority(int policy, int priority)
{
    struct sched_param sched_p;

    sched_getparam(0, &sched_p);
    if (policy == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        sched_setscheduler(0, policy, &sched_p);
        setpriority(PRIO_PROCESS, 0, priority); //  Note: "priority" is nice value.
        XLOGD("[setPriority] tid(%d) policy(SCHED_OTHER:%d) priority(%d)", gettid(), policy, priority);
    } else {
        sched_p.sched_priority = priority;      //  Note: "priority" is real-time priority.
        sched_setscheduler(0, policy, &sched_p);
        XLOGD("[setPriority] tid(%d) policy(Real-Time:%d) priority(%d)", gettid(), policy, priority);
    }
}


/*******************************************************************************
*
********************************************************************************/
MVOID*
mhal_Autorama::
MergeThread(void *arg)
{
    MINT32 err = MHAL_NO_ERROR;

    // set thread name
    ::prctl(PR_SET_NAME, "AUTORAMAthread", 0, 0, 0);
	// set thread nice priority
    setPriority(SCHED_OTHER, -20);
    MINT32 priority = getPriority(SCHED_OTHER);
    XLOGD("[getPriority] %d Schedule NORMAL priority %d!\n", gettid(), priority);

    // (1) calcStitch
    for(MUINT32 i = 0; i < mhalAutoramaObj->mAUTORAMAFrameIdx; i++)
    {
    	XLOGD(" mHalCamAUTORAMAMerge mStitchDir %d EV %d", mhalAutoramaObj->mStitchDir, mhalAutoramaObj->mpAUTORAMAObj->gImgEV[i]);        
        err = mhalAutoramaObj->mpAUTORAMAObj->mHalAutoramaCalcStitch(
                            (void*)(mhalAutoramaObj->mAUTORAMAFrame.virtAddr + 
                            (mhalAutoramaObj->mAUTORAMAFrame.frmSize * i)), 
                            mhalAutoramaObj->mpAUTORAMAObj->gImgEV[i], 
                            mhalAutoramaObj->mStitchDir);
		
        if ( err != MHAL_NO_ERROR) {
			*reinterpret_cast<int*>(arg) = err;
            return NULL;
        }
    }

    // (2) DoStitch
    XLOGD(" mHalAutoramaDoStitch");
    err = mhalAutoramaObj->mpAUTORAMAObj->mHalAutoramaDoStitch();
    if ( err != MHAL_NO_ERROR) {
		*reinterpret_cast<int*>(arg) = err;
        return NULL;
    }

    // (3) GetResult
    XLOGD(" mHalAutoramaGetResult");
    err = mhalAutoramaObj->mpAUTORAMAObj->mHalAutoramaGetResult(&mhalAutoramaObj->ResultInfo);
    if ( err != MHAL_NO_ERROR) {
		*reinterpret_cast<int*>(arg) = err;
        return NULL;
    }
	
    XLOGD(" ImgWidth %d ImgHeight %d ImgBufferAddr 0x%x", mhalAutoramaObj->ResultInfo.ImgWidth, mhalAutoramaObj->ResultInfo.ImgHeight, mhalAutoramaObj->ResultInfo.ImgBufferAddr);
	return NULL;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mhal_Autorama::
mHalCamFeatureMerge()
{
    MINT32 err = MHAL_NO_ERROR;
    // (1) 
    // AP thead has possibility to do merge even when specified number has not collected yet
    // AP thread and preview thread could compete for 'merge'
    android_atomic_dec(&mergeCnt);
    if (mergeCnt < 0) {
        MY_DBG("mergeCnt < 0");
        return err;
    }
    MY_DBG("thread: %d, [mHalCamAUTORAMAMerge] %d, %s", gettid(), mAUTORAMAFrameIdx, outputFile);
    setState(MHAL_CAM_FEATURE_STATE_MERGE);

    // (2)
    // create normal priority thread for merge   
    pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_OTHER, RTPM_PRIO_CAMERA_COMPRESS};
    pthread_create(&AUTORAMAFuncThread, &attr, MergeThread, &err);
    pthread_join(AUTORAMAFuncThread, NULL);
	MY_DBG("get thread back");	
	if (err != MHAL_NO_ERROR) {
        return err;
    }

    // (3)
    // to guarantee compression is done after merge
    // notify AP that merge is done
    sem_post(&CompressSem);

    #if 0
    char sourceFiles[80];
    for(int i = 0; i < mAUTORAMAFrameIdx; i++)
    {
        sprintf(sourceFiles, "%s%d.raw", "/sdcard/DCIM/Camera/AUTORAMA",i);
        mHalMiscDumpToFile(sourceFiles, (MUINT8 *)(mAUTORAMAFrame.virtAddr + (mAUTORAMAFrame.frmSize * i)), mAUTORAMAFrame.frmSize);
    }
    #endif 
    
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32   
mhal_Autorama::
mHalCamFeatureCompress()
{
    MY_DBG("[mHalCamAUTORAMACompress]");
    MINT32 err = MHAL_NO_ERROR;

    // (1) 
    // guarantee compression is excuted after merge done
    sem_wait(&CompressSem);
    MY_DBG("got AutoramaSem object");

    // (2)
    // compress to jpeg file
    CamJpg camjpg(mpHal3AObj, mpMhalCam, mpmhalCamParam, meSensorType);

    FrameParam src(
        ResultInfo.ImgWidth,
        ResultInfo.ImgHeight,
        ResultInfo.ImgWidth*ResultInfo.ImgHeight * 3 / 2,
        0, //don't care
        JpgEncHal::kYV12_Planar_Format,
        ResultInfo.ImgBufferAddr
    );
        
    FrameParam dst(
        ResultInfo.ImgWidth,
        ResultInfo.ImgHeight,
        0, // don't care
        mAUTORAMAFrame.frmSize,
        0, // don't care
        mJPGFrameAddr
    );
       
    #if 0
    char tmpName[80];
    sprintf(tmpName, "%s.raw", "/sdcard/DCIM/Camera/src");
    mHalMiscDumpToFile((char *) tmpName, (MUINT8 *) src.virtAddr, src.frmSize);
    #endif


    MUINT32 jpgSize = camjpg.doJpg(1, src, dst, 0, 0);
    
    MY_DBG("jpeg size: %d", jpgSize);
    //mHalMiscDumpToFile((char *) outputFile, (MUINT8 *) mJPGFrameAddr, jpgSize);
    captureDoneCallback(MHAL_CAM_CB_PANO_ORI_DATA
                                , 2
                                , (int32_t)mJPGFrameAddr
                                , jpgSize
                                );
    MINT32 cb[2]= {0, 50};
    mpMhalCam->mHalCamCBHandle(MHAL_CAM_CB_AUTORAMA, (void*)&cb);   
                              
    setState(MHAL_CAM_FEATURE_STATE_MERGE_DONE);
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 
mhal_Autorama::
ISShot(MVOID *arg1, MBOOL &shot)
{
    MINT32 err = MHAL_NO_ERROR;
    ICameraIO::BuffInfo_t pBuffInfo; 

    // Get FD frame    
    MY_DBG("start to get Preview");        
    err = mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_FD_PORT, &pBuffInfo);  
    if ( err != MHAL_NO_ERROR ) {
        MY_WARN("getPreviewFrame error");
        return err;
    }
    
    err = mpAUTORAMAObj->mHalAutoramaDoMotion((MUINT32 *)(mFDWorkingBuf->getVirtAddr()+ pBuffInfo.hwIndex * 320 * 240 *2), (MUINT32 *) arg1);
    if ( err != MHAL_NO_ERROR ) {
        MY_WARN("Do motion error");
        return err;
    }    
    
    err = mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_FD_PORT, &pBuffInfo);      
    if ( err != MHAL_NO_ERROR ) {
        MY_WARN("releasePreviewFrame error");
        return err;
    }  
    
    #if 0
    mHalMiscDumpToFile((char*) "/sdcard/DCIM/Camera/fd.raw", (UINT8*)mFDWorkingBuf->getVirtAddr(), mFDWorkingBuf->getPoolSize()); 
    #endif 
 
    MY_DBG("  mHalAutoramaDoMotion: %d ", ((AutoramaMotionResult*)arg1)->ReadyToShot);
    shot = (((AutoramaMotionResult*)arg1)->ReadyToShot) > 0 ? true : false;
    
    return err;
}



/*******************************************************************************
*
********************************************************************************/
MVOID
mhal_Autorama::
updateDir(MINT32 sensor, MTKPIPEAUTORAMA_DIRECTION_ENUM &dir)
{
    if(sensor == 0)
    {
     switch(dir)
     {
         case MTKPIPEAUTORAMA_DIR_RIGHT:
             dir = MTKPIPEAUTORAMA_DIR_DOWN;
         break;
         case MTKPIPEAUTORAMA_DIR_LEFT:
             dir = MTKPIPEAUTORAMA_DIR_UP;
         break;
         case MTKPIPEAUTORAMA_DIR_UP:
             dir = MTKPIPEAUTORAMA_DIR_LEFT;
         break;
         case MTKPIPEAUTORAMA_DIR_DOWN:
             dir = MTKPIPEAUTORAMA_DIR_RIGHT;
         break;
         default:
             dir = MTKPIPEAUTORAMA_DIR_NO;
         break;
     }            
    }
    else if(sensor == 180)
    {
     switch(dir)
     {
         case MTKPIPEAUTORAMA_DIR_RIGHT:
             dir = MTKPIPEAUTORAMA_DIR_UP;
         break;
         case MTKPIPEAUTORAMA_DIR_LEFT:
             dir = MTKPIPEAUTORAMA_DIR_DOWN;
         break;
         case MTKPIPEAUTORAMA_DIR_UP:
             dir = MTKPIPEAUTORAMA_DIR_RIGHT;
         break;
         case MTKPIPEAUTORAMA_DIR_DOWN:
             dir = MTKPIPEAUTORAMA_DIR_LEFT;
         break;
         default:
             dir = MTKPIPEAUTORAMA_DIR_NO;
         break;
     }            
    }
    else if(sensor == 270)
    {
     switch(dir)
     {
         case MTKPIPEAUTORAMA_DIR_RIGHT:
             dir = MTKPIPEAUTORAMA_DIR_LEFT;
         break;
         case MTKPIPEAUTORAMA_DIR_LEFT:
             dir = MTKPIPEAUTORAMA_DIR_RIGHT;
         break;
         case MTKPIPEAUTORAMA_DIR_UP:
             dir = MTKPIPEAUTORAMA_DIR_DOWN;
         break;
         case MTKPIPEAUTORAMA_DIR_DOWN:
             dir = MTKPIPEAUTORAMA_DIR_UP;
         break;
         default:
             dir = MTKPIPEAUTORAMA_DIR_NO;
         break;
     }            
    }    
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mhal_Autorama::
mHalCamFeatureProc(
    MUINT32 frameIndex
)
{
    MINT32 err = MHAL_NO_ERROR;

    /* Mutex at 2 plases: Uninit() and Proc() */
    Mutex::Autolock lock(mLock);
    //   
    if (getState() == MHAL_CAM_FEATURE_STATE_CAPTURE && mAUTORAMAFrameIdx < mAUTORAMAFrame.frmCount) {

        // Shot or not  
        AutoramaMotionResult autoramaResult;
        ::memset(&autoramaResult, 0, sizeof(AutoramaMotionResult));
        autoramaResult.Direction = MTKPIPEAUTORAMA_DIR_NO;        
        MBOOL isShot = false;
        err = ISShot(&autoramaResult, isShot);
        
        mStitchDir = (MTKPIPEAUTORAMA_DIRECTION_ENUM)autoramaResult.Direction;
        //updateDir(sensorOrientation, mStitchDir);

        MINT32 MV[3] = {autoramaResult.MV_X, autoramaResult.MV_Y, mStitchDir};
        mpMhalCam->mHalCamCBHandle(MHAL_CAM_CB_AUTORAMAMV, (void*)&MV);

        if (mAUTORAMAFrameIdx == 0) {
           isShot = true;
           mpMhalCam->mHalCam3ACtrl(1, 0, 1);
           mpHal3AObj->setAutoExposureLock(0);
        }
        if (isShot) {
            MY_DBG("autorama got one frame: %d", mAUTORAMAFrameIdx);
            mStitchDir = (MTKPIPEAUTORAMA_DIRECTION_ENUM)autoramaResult.Direction; 
            mpHal3AObj->getAEPlineEV(&mpAUTORAMAObj->gImgEV[mAUTORAMAFrameIdx]);           

            // Currently format: YV12
            MUINT8 *pbufIn = (MUINT8 *) (frmYuv_virtAddr + (frmYuv_size* frameIndex));
            MUINT8 *pbufOut = (MUINT8 *) (mAUTORAMAFrame.virtAddr + (mAUTORAMAFrame.frmSize * mAUTORAMAFrameIdx));
            memcpy(pbufOut, pbufIn, mAUTORAMAFrame.frmSize);

            #if 0
            char sourceFiles[80];
            sprintf(sourceFiles, "%s%d.raw", "/sdcard/DCIM/Camera/AUTORAMAgetFrame", mAUTORAMAFrameIdx);
            mHalMiscDumpToFile(sourceFiles, (MUINT8 *)(mAUTORAMAFrame.virtAddr + (mAUTORAMAFrame.frmSize * mAUTORAMAFrameIdx)), mAUTORAMAFrame.frmSize);
            #endif    
            
            mAUTORAMAFrameIdx++;

            // Do Merge
            if (mAUTORAMAFrameIdx == mAUTORAMAFrame.frmCount)
            {                
                err = mHalCamFeatureMerge(); 
                if ( err != MHAL_NO_ERROR) {
                    return err;
                }
            }
            // Call back to upper layer
            MINT32 cb[2]= {1, 50};
            mpMhalCam->mHalCamCBHandle(MHAL_CAM_CB_AUTORAMA, (void*)&cb);               
        }
    }
    else {
        MY_DBG("frame is enough, no more frame is needed");
    }
    
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
mhal_Autorama::
captureDoneCallback(int32_t message, int32_t id, int32_t bufferAddr, int32_t bufferSize)
{
    MY_DBG("+");
    bool ret = true;

    // for debug
	  //char value[PROPERTY_VALUE_MAX] = {'\0'};
	  //property_get("mediatek.previewfeature.dump", value, "1");
	  //bool dump = atoi(value);
    //if(dump) {
    //    mHalMiscDumpToFile("/sdcard/PanoramaFinal.jpg", (MUINT8*)bufferAddr, (MUINT32)bufferSize);
    //}
    
    mpMhalCam->mHalCamCBHandle(message, (void*)bufferAddr, bufferSize); 

    MY_DBG("-");
    return ret;
}

