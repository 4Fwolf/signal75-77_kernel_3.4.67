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

#include <stdlib.h>
//
#include "Mhal_3dshot.h"
#include "mhal_cam.h"
#include "ICameraIO.h"

#include "mhal_misc.h"
#include "jpeg_hal.h"
#include "pano_hal_base.h"
#include "MpoEncoder.h"
#include "CamJpg.h"
#include "aaa_hal_base.h"

/*******************************************************************************
*
********************************************************************************/

static sem_t mergeDone;

/*******************************************************************************
*
********************************************************************************/
mhal_3dshot*
mhal_3dshot::
createInstance(NSCamera::ESensorType const eSensorType)
{
    return new mhal_3dshot(eSensorType);
}


/*******************************************************************************
*
********************************************************************************/
mhal_3dshot::mhal_3dshot(NSCamera::ESensorType const eSensorType)
    : mhal_featureBase("3DshotPrv")
    , meSensorType(eSensorType)
    , mp3dWorkingBuf(NULL)
    , mJPGFrameAddr(0)
    , m3dFrameIdx(0)
    , virAddrShift(0)
    , mergeCnt(1)
{

    MY_DBG("mhal_3dshot constructor");
    ::memset(&m3dFrame, 0, sizeof(mhalCamFrame_t));
    ::memset(&mp3dResult, 0, sizeof(MavPipeResultInfo));
    ::memset(&MyPano3DResultInfo, 0, sizeof(PipePano3DResultInfo));
    ::memset(&MyMavPipeResultInfo, 0, sizeof(MavPipeResultInfo));
    ::memset(&ImageInfo, 0, sizeof(MavPipeImageInfo));    
  
    mp3dshotObj = hal3DFBase::createInstance(HAL_PANO3D_OBJ_NORMAL); 
    setState(MHAL_CAM_FEATURE_STATE_IDLE);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
mhal_3dshot::
destroyInstance()
{
    MY_DBG("[~mhal_3dshot()]");

    //mHalCamAUTORAMAUninit();
    if (mp3dshotObj) {
        mp3dshotObj->destroyInstance();
        mp3dshotObj = NULL;
    }

    delete this;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 
mhal_3dshot::
init(
    mHalCam* const pMhalCam, 
    Hal3ABase* const pHal3AObj, 
    ICameraIO* const pCameraIOObj,
    mHalCamMemPool* const pFDWorkingBuf
)
{
    //
    if  ( ! pMhalCam )
    {
        MY_ERR("[mHalCamInit3dshot] pMhalCam == NULL");
        return  -1;
    }
    mpMhalCam = pMhalCam;

    //
    if  ( ! pHal3AObj )
    {
        MY_ERR("[mHalCamInit3dshot] pHal3AObj == NULL");
        return  -1;
    }
    mpHal3AObj = pHal3AObj;   

    //
    if  ( ! pCameraIOObj )
    {
        MY_ERR("[mHalCamInit3dshot] pCameraIOObj == NULL");
        return  -1;
    }
    mpCameraIOObj = pCameraIOObj;

    //
    if  ( ! pFDWorkingBuf )
    {
        MY_ERR("[mHalCamInit3dshot] pFDWorkingBuf == NULL");
        return  -1;
    }
    mFDWorkingBuf = pFDWorkingBuf;
    
    sem_init(&mergeDone, 0, 0);


    return MHAL_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
mhal_3dshot::
setParameters(
    mhalCamParam_t *pmhalCamParam
)
{
    MINT32 err = MHAL_NO_ERROR;
    strcpy((char *) outputFile, (char *)pmhalCamParam->u1FileName);
    //strcat(strtok((char *)outputFile,"."),".mpo");
    //m3dNum = 2;
    m3dNum=pmhalCamParam->u4BusrtNo;

    if ( !::strcmp(getFeatureName(), "3DshotPrv") ) {
        frmYuv_virtAddr = pmhalCamParam->frmYuv.virtAddr;
        frmYuv_size = pmhalCamParam->frmYuv.frmSize;
        m3dFrame.w = pmhalCamParam->frmYuv.w;
        m3dFrame.h = pmhalCamParam->frmYuv.h;
    }
    else if ( !::strcmp(getFeatureName(), "3DshotCap") ) {
        m3dFrame.w = pmhalCamParam->frmJpg.w;
        m3dFrame.h = pmhalCamParam->frmJpg.h;
    }
    
    m3dFrame.frmSize = m3dFrame.w * m3dFrame.h * 3 / 2;  // YV12 now
    m3dFrame.frmCount = m3dNum;    // fix at 2
    m3dFrame.bufSize = m3dFrame.frmSize * m3dFrame.frmCount;
    m3dFrame.virtAddr = (MUINT32) malloc(m3dFrame.bufSize);
    MY_DBG("[setParameters] frmSize %d frmCount %d bufSize %d",m3dFrame.frmSize,m3dFrame.frmCount,m3dFrame.bufSize);
    if (m3dFrame.virtAddr == 0)
    {
        MY_ERR("m3dFrame.virtAddr == 0");
        return MHAL_INVALID_MEMORY;
    }

    mStitchDir = MTKPIPEAUTORAMA_DIR_NO;
    mpmhalCamParam = pmhalCamParam;
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
mhal_3dshot::
mHalCamFeatureInit(
    mhalCamParam_t *pmhalCamParam
)
{
    MY_DBG("[mHalCamFeatureInit] E");
    MINT32 err = MHAL_NO_ERROR;
    // (1) Set global variable
    err = setParameters(pmhalCamParam);
    if ( err != MHAL_NO_ERROR) return err;
    
    // (2) Create working buffer
    MINT32 initBufSize = m3dFrame.w * m3dFrame.h * 4 * 10;
    MINT32 motionBufSize = 320 * 240 * 3;
    MINT32 warpBufSize = m3dFrame.w * m3dFrame.h* 2 + 2048;
    MINT32 pano3dBufSize = m3dFrame.w * m3dFrame.h * 4 * 25;
    
    mp3dWorkingBuf = (MINT8 *) memalign(L1_CACHE_BYTES,initBufSize + motionBufSize + warpBufSize + pano3dBufSize );
    if (mp3dWorkingBuf == NULL) {
       MY_ERR("[mHalCamFeatureInit] Err, mp3dWorkingBuf == NULL");
       //mHalCamFeatureUninit();
       return MHAL_INVALID_MEMORY;
    }

    MUINT32 jpegBufsize = ((m3dFrame.bufSize * 2) + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
    mJPGFrameAddr = (MUINT32)memalign (L1_CACHE_BYTES, jpegBufsize);
    MY_DBG("bufsize: %d, virtAddr: %x,  L1_bytes: %d", jpegBufsize, mJPGFrameAddr, L1_CACHE_BYTES);

    //mJPGFrameAddr = (MUINT32) malloc(m3dFrame.bufSize + m3dFrame.frmSize);
    if (mJPGFrameAddr == 0) {
        MY_ERR("[mHalCamFeatureInit] Err, m3dFrame.virtAddr == NULL");
        //mHalCamFeatureUninit();
        return MHAL_INVALID_MEMORY;
    }
    
    // (3) 3d init 
    err = mp3dshotObj->mHal3dfInit(mp3dWorkingBuf, 
                             mp3dWorkingBuf + initBufSize, 
                             mp3dWorkingBuf + initBufSize + motionBufSize, 
                             mp3dWorkingBuf + initBufSize + motionBufSize + warpBufSize);
                                 
    if ( err < 0) {
        MY_ERR("mp3dshotObj->mHal3dfInit Err");
        return err;
    }

    
    setState(MHAL_CAM_FEATURE_STATE_CAPTURE);
    
    MY_DBG("[mHalCamFeatureInit] X");
    return MHAL_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MVOID 
mhal_3dshot::
mHalCamFeatureUninit()
{
    MY_DBG("[mHalCamFeatureUninit] E");
    /* Mutex at 2 plases: Uninit() and Proc() */    
    Mutex::Autolock lock(mLock);
    setState(MHAL_CAM_FEATURE_STATE_UNINIT);
    //

    mpMhalCam->mHalCam3ACtrl(1, 1, 1);

    if ( mp3dshotObj ) {
        mp3dshotObj->mHal3dfUninit();
    }

    if ( mp3dWorkingBuf ) {
        free(mp3dWorkingBuf);
        mp3dWorkingBuf = NULL;
    }

    if ( m3dFrame.virtAddr ) {
        free((UINT8 *) m3dFrame.virtAddr);
        m3dFrame.virtAddr = 0;
    }

    if ( mJPGFrameAddr ) {
        free((UINT8 *) mJPGFrameAddr);
        mJPGFrameAddr = 0;
    }
    
    MY_DBG("[mHalCamFeatureUninit] X ");
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mhal_3dshot::
mHalCamFeatureMerge()
{
    MINT32 err = MHAL_NO_ERROR;
    MY_DBG("[mHalCamFeatureMerge] E");
    android_atomic_dec(&mergeCnt);
    if (mergeCnt < 0) {
        MY_DBG("mergeCnt < 0");
        return err;
    }
    setState(MHAL_CAM_FEATURE_STATE_MERGE); 
       
    MY_DBG("[mHalCamFeatureMerge] AddImg");
    err = mHalCamFeatureAddImg();
    if ( err != MHAL_NO_ERROR) return err;
                    
    // (1) Merge
    MY_DBG("[mHalCamFeatureMerge] Merge");
    err = mp3dshotObj->mHal3dfMerge((MUINT32*)&mp3dResult);
    if ( err != MHAL_NO_ERROR) return err;

    MUINT32 result;   
    err = mp3dshotObj->mHal3dfGetResult(result);
    if ( err != MHAL_NO_ERROR) return err;    
    MY_DBG("mHalMavGetResult result %d",result);
    
    if(mp3dResult.RetCode!=MHAL_NO_ERROR)
    {
        MY_DBG("[mHalCamFeatureMerge] Merge fail then change frame number to 2 for ori");
        m3dFrameIdx=2;
    }
    else
    {
        // (2) Warp
        MY_DBG("[mHalCamFeatureMerge] Warp");
        ImageInfo.ImgAddr = m3dFrame.virtAddr;
        err = mp3dshotObj->mHal3dfWarp((MavPipeImageInfo*)&ImageInfo,(MUINT32*)&mp3dResult, m3dFrameIdx);
        if ( err != MHAL_NO_ERROR) return err;
    }
    
    // (3) stitch
    MY_DBG("[mHalCamFeatureMerge] Stitch");
    MyMavPipeResultInfo.ImageInfo[0].Width = m3dFrame.w;
    MyMavPipeResultInfo.ImageInfo[0].Height = m3dFrame.h;
    MyMavPipeResultInfo.ImageInfo[0].ImgAddr = m3dFrame.virtAddr;  
    MyMavPipeResultInfo.ImageInfo[0].ClipY = mp3dResult.ImageInfo[0].ClipY;  

    for(MUINT32 i = 0; i < m3dFrameIdx; i++)
    {
       MY_DBG("[mHalCamFeatureMerge] i = %d GridX %d MinX %d MaxX %d", i,mp3dResult.ImageInfo[i].GridX,mp3dResult.ImageInfo[i].MinX,mp3dResult.ImageInfo[i].MaxX);
    	 MyMavPipeResultInfo.ImageInfo[i].GridX = mp3dResult.ImageInfo[i].GridX;
    	 MyMavPipeResultInfo.ImageInfo[i].MinX = mp3dResult.ImageInfo[i].MinX;
    	 MyMavPipeResultInfo.ImageInfo[i].MaxX = mp3dResult.ImageInfo[i].MaxX;
    } 
    MyMavPipeResultInfo.ClipHeight = mp3dResult.ClipHeight;
    err = mp3dshotObj->mHal3dfStitch((MUINT32 *)&MyMavPipeResultInfo, m3dFrameIdx);
    if ( err != MHAL_NO_ERROR) return err;
    MY_DBG("mHal3dfStitch done");    

    
    err = mp3dshotObj->mHal3dfGetStitchResult((void*) &MyPano3DResultInfo);
    if ( err != MHAL_NO_ERROR) return err;
    MY_DBG("PanoWidth %d  PanoHeight %d LeftPanoImageAddr 0x%x RightPanoImageAddr 0x%x", MyPano3DResultInfo.PanoWidth, MyPano3DResultInfo.PanoHeight, MyPano3DResultInfo.LeftPanoImageAddr, MyPano3DResultInfo.RightPanoImageAddr);

    #if 0
    //  temporary save 2 images to 2 files
    char tmpName[80];
    sprintf(tmpName, "%s%d.raw", "/mnt/sdcard/Pano", 1);
    mHalMiscDumpToFile((char *) tmpName, (MUINT8 *) MyPano3DResultInfo.LeftPanoImageAddr, MyPano3DResultInfo.PanoWidth*MyPano3DResultInfo.PanoHeight*3>>1);
    sprintf(tmpName, "%s%d.raw", "/mnt/sdcard/Pano", 2);
    mHalMiscDumpToFile((char *) tmpName, (MUINT8 *) MyPano3DResultInfo.RightPanoImageAddr, MyPano3DResultInfo.PanoWidth*MyPano3DResultInfo.PanoHeight*3>>1);     
    #endif 

    MY_DBG("width: %d, height: %d, diff(left,right): %d", MyPano3DResultInfo.PanoWidth, MyPano3DResultInfo.PanoHeight, MyPano3DResultInfo.RightPanoImageAddr- MyPano3DResultInfo.LeftPanoImageAddr);

    setState(MHAL_CAM_FEATURE_STATE_MERGE_DONE);
     
    sem_post(&mergeDone);

    MY_DBG("[mHalCamFeatureMerge] X");
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32   
mhal_3dshot::
mHalCamFeatureCompress()
{
    MINT32 err = MHAL_NO_ERROR;

    sem_wait(&mergeDone);
    MY_DBG("[mHalCamFeatureCompress] E");
    MY_DBG("width: %d, height: %d", MyPano3DResultInfo.PanoWidth, MyPano3DResultInfo.PanoHeight);
           
    CamJpg camjpg(mpHal3AObj, mpMhalCam, mpmhalCamParam, meSensorType);

    FrameParam src(
        MyPano3DResultInfo.PanoWidth,
        MyPano3DResultInfo.PanoHeight,
        MyPano3DResultInfo.PanoWidth*MyPano3DResultInfo.PanoHeight * 3 >> 1,
        0, //don't care
        JpgEncHal::kYV12_Planar_Format,
        MyPano3DResultInfo.LeftPanoImageAddr
    );
        
    FrameParam dst(
        MyPano3DResultInfo.PanoWidth,
        MyPano3DResultInfo.PanoHeight,
        0, // don't care
        m3dFrame.frmSize, //memory aligment. Instead of MyPano3DResultInfo.PanoWidth * MyPano3DResultInfo.PanoHeight * 3 >> 1,
        0, // don't care
        mJPGFrameAddr
    );
       
    #if 0
    char tmpName[80];
    sprintf(tmpName, "%s.raw", "/sdcard/DCIM/Camera/src");
    mHalMiscDumpToFile((char *) tmpName, (MUINT8 *) src.virtAddr, src.frmSize);
    #endif


    // (2) Compress Jpeg
    MPImageInfo * pMPImageInfo = new MPImageInfo[2];

    for (MUINT8 i = 0; i < 2; i++) {
        MUINT32 SRCshiftsize = i * MyPano3DResultInfo.PanoWidth * MyPano3DResultInfo.PanoHeight * 3 >> 1;
        MUINT32 DSTshiftsize = i * m3dFrame.frmSize;
        MUINT32 jpgSize = camjpg.doJpg(i == 0? 1 : 0, src, dst, SRCshiftsize, DSTshiftsize);
        MY_DBG("jpeg size: %d, SRCshiftsize: %d, DSTshiftsize: %d", jpgSize, SRCshiftsize, DSTshiftsize);
        //
    #if 0
        char filename[80];
        sprintf(filename, "/mnt/sdcard2/out%d.jpg", i);
        mHalMiscDumpToFile(filename, (MUINT8 *)(mJPGFrameAddr + DSTshiftsize), jpgSize);
    #endif 
        //
        pMPImageInfo[i].imageBuf = (char*)(mJPGFrameAddr + DSTshiftsize);
        pMPImageInfo[i].imageSize = jpgSize ;
        pMPImageInfo[i].sourceType = SOURCE_TYPE_BUF; 
        pMPImageInfo[i].filename = const_cast<char*>("stereo");
    }

    

    // (3) create MPO file
    err = camjpg.createMPO(pMPImageInfo, 2, (char*)outputFile, MTK_TYPE_Stereo);
    MY_DBG("outputFile: %s", outputFile);
    setState(MHAL_CAM_FEATURE_STATE_IDLE);

    //let AP know: merge done. 
    MINT32 cb[2];
    cb[0] = 0;
    cb[1] = 10;
    mpMhalCam->mHalCamCBHandle(MHAL_CAM_CB_AUTORAMA,(void*)&cb);    
    
    MY_DBG("[mHalCamFeatureCompress] X");
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
mhal_3dshot::
ISShot(MVOID *arg1, MBOOL &shot)
{
    if (!::strcmp(getFeatureName(), "3DshotPrv") && m3dFrameIdx == 0)
    {
        mpMhalCam->mHalCam3ACtrl(1, 0, 1);
        shot = true;
        return MHAL_NO_ERROR;
    }
        
    ICameraIO::BuffInfo_t *pBuffInfo = new ICameraIO::BuffInfo_t(); 

    ::memset(arg1, 0, sizeof(AutoramaMotionResult));
    ((AutoramaMotionResult*)arg1)->Direction = MTKPIPEAUTORAMA_DIR_NO;

    // Get FD frame           
    mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_FD_PORT, pBuffInfo);  
    #if 0
    mHalMiscDumpToFile((char*) "/sdcard/DCIM/Camera/fd.raw", (UINT8*)mFDWorkingBuf->getVirtAddr(), mFDWorkingBuf->getPoolSize()); 
    #endif     
    mp3dshotObj->mHal3dfDoMotion((MUINT32 *)mFDWorkingBuf->getVirtAddr(), (MUINT32 *) arg1);
    mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_FD_PORT, pBuffInfo);      
   
    MY_DBG("mHalDoMotion: %d", ((AutoramaMotionResult*)arg1)->ReadyToShot);

    delete pBuffInfo;
    shot = (((AutoramaMotionResult*)arg1)->ReadyToShot) > 0 ? true : false;
    return MHAL_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
mhal_3dshot::
mHalCamFeatureAddImg()
{
    MINT32 err = MHAL_NO_ERROR;
    MY_DBG("[mHalCamFeatureAddImg E");


// test if viraddr has correct data
#if 0
    char tmpName[80];
    for (MUINT32 i = 0; i < m3dFrameIdx; i++)
    {
        sprintf(tmpName, "%s%d.raw", "/sdcard/DCIM/Camera/src", i);
        mHalMiscDumpToFile((char *) tmpName, (MUINT8 *)(m3dFrame.virtAddr + (m3dFrame.frmSize * i)), m3dFrame.frmSize);
    }
#endif

  MY_DBG("[mHalCamFeatureAddImg m3dFrameIdx %d",m3dFrameIdx);
	for (MUINT32 i = 0; i < m3dFrameIdx; i++) {
        ImageInfo.ImgAddr = m3dFrame.virtAddr + (m3dFrame.frmSize * i);
        ImageInfo.Width = m3dFrame.w;
        ImageInfo.Height = m3dFrame.h;
        ImageInfo.ControlFlow = 0;
    
        ImageInfo.MotionValue[0] = mp3dResult.ImageInfo[i].MotionValue[0];
        ImageInfo.MotionValue[1] = mp3dResult.ImageInfo[i].MotionValue[1];
        ImageInfo.MotionValue[0] *= -2;
        ImageInfo.MotionValue[1] *= -2;

        mp3dResult.ImageInfo[i].Width = m3dFrame.w;
        mp3dResult.ImageInfo[i].Height = m3dFrame.h;
        mp3dResult.ImageInfo[i].ImgAddr = ImageInfo.ImgAddr;

        MY_DBG("ImgAddr 0x%x, Width %d, Height %d, Motion: %d %d", 
                 ImageInfo.ImgAddr, ImageInfo.Width, ImageInfo.Height,
                 ImageInfo.MotionValue[0], ImageInfo.MotionValue[1]);

        err = mp3dshotObj->mHal3dfAddImg((MavPipeImageInfo*)&ImageInfo);
        if ( err!= MHAL_NO_ERROR ) return err;
    }
    
    MY_DBG("[mHalCamFeatureAddImg X");

    return err;
}



/*******************************************************************************
*
********************************************************************************/
MVOID
mhal_3dshot::
setFrameBuf(
    MUINT8* memIn,
    MUINT32 size 
)
{
    // read from memory
    if ( memIn != NULL) 
    {
        ::memcpy((MUINT8*)(m3dFrame.virtAddr + virAddrShift), memIn, size);
        virAddrShift += size;

        if (m3dFrame.frmSize != size){
            MY_ERR("m3dFrame.frmSize ! = size");
        }

        m3dFrameIdx++;
    }
    // read from preview frame
    else
    {
        // Currently format: YV12
        MUINT8 *pbufIn = (MUINT8 *) (frmYuv_virtAddr + (frmYuv_size* size));  // size here means index
        MUINT8 *pbufOut = (MUINT8 *) (m3dFrame.virtAddr + (m3dFrame.frmSize * m3dFrameIdx));
        memcpy(pbufOut, pbufIn, m3dFrame.frmSize);
    }
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mhal_3dshot::
mHalCamFeatureProc(
    MUINT32 frameIndex
)
{
    MY_DBG("[mHalCamFeatureProc] E");

    MINT32 err = MHAL_NO_ERROR;

    /* Mutex at 2 plases: Uninit() and Proc() */
    Mutex::Autolock lock(mLock);
    //   
    if (getState() == MHAL_CAM_FEATURE_STATE_CAPTURE && m3dFrameIdx < m3dFrame.frmCount) {
        
        // Shot or not  
        AutoramaMotionResult AutoramaResult;
        MBOOL isShot = false;
        err = ISShot(&AutoramaResult, isShot);
        
        //
        mStitchDir = (MTKPIPEAUTORAMA_DIRECTION_ENUM)AutoramaResult.Direction;
        MINT32 MV[3];
        MV[0] = AutoramaResult.MV_X;
        MV[1] = AutoramaResult.MV_Y;
        MV[2] = mStitchDir;
        mpMhalCam->mHalCamCBHandle(MHAL_CAM_CB_AUTORAMAMV, (void*)&MV);

        if (isShot){
            if (!::strcmp(getFeatureName(), "3DshotCap")) {
                // let AP know --> shot 
            }
            else if (!::strcmp(getFeatureName(), "3DshotPrv")) {
                MY_DBG("3d got one frame: %d", m3dFrameIdx);          

                setFrameBuf(NULL, frameIndex);
                
                mp3dResult.ImageInfo[m3dFrameIdx].MotionValue[0] = MV[0];
                mp3dResult.ImageInfo[m3dFrameIdx].MotionValue[1] = MV[1];

                //
                #if 0
                char sourceFiles[80];
                sprintf(sourceFiles, "%s%d.raw", "/mnt/sdcard2/DCIM/3DPanoFrame", m3dFrameIdx);
                mHalMiscDumpToFile(sourceFiles, (MUINT8 *)(m3dFrame.virtAddr + (m3dFrame.frmSize * m3dFrameIdx)), m3dFrame.frmSize);
                #endif    
                               
                m3dFrameIdx++;
                
                // Call back to upper layer
                MINT32 cb[2];
                cb[0] = 1;
                cb[1] = 10;

                mpMhalCam->mHalCamCBHandle(MHAL_CAM_CB_AUTORAMA, (void*)&cb);               
                // Do Merge
                if (m3dFrameIdx == m3dFrame.frmCount)
                {                
                    setState(MHAL_CAM_FEATURE_STATE_MERGE);
                    //err = mHalCamFeatureAddImg();
                    //if ( err != MHAL_NO_ERROR) return err;
                    err = mHalCamFeatureMerge();
                    if ( err != MHAL_NO_ERROR) return err;

                } 
            }
        }
    }
    else {
        MY_DBG("frame is enough, no more frame is needed");
    }
    
    MY_DBG("[mHalCamFeatureProc] X");

    return err;
}



