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
#define LOG_TAG "pipe/CamIO" 

#include <cutils/properties.h>

#include <mhal/inc/camera/graphics.h>
#include <pipe_types.h>
#include <cam_types.h>
#include <cam_port.h>
using namespace NSCamera;
#include <mcam_log.h>
#include <mcam_mem.h>
#include "isp_hal.h"
#include "mdp_hal.h"
#include <res_mgr_hal.h>
#include <camera_custom_if.h>
#ifdef  MTK_NATIVE_3D_SUPPORT
    //
    #include <mcam_profile.h>       // For CamTimer
    //
    #include <sys/prctl.h>
    #include <sys/time.h>
    #include <sys/resource.h>
    #include <utils/threads.h>
    //
    #include "camera_custom_n3d.h"  // For N3D Customer Parameters in Customer Folder.
    #include "n3d_hal_base.h"
    #include "n3d_hal.h"
    //
    #define N3D_FACTOR   (110)
#endif

//
#include "CameraIO75.h"

#include "CameraProfile.h"

#define MDP_WORK_BUFFER_CNT 3

static int MCAM_LOG_DBG = 0;  

#define NEW_MDP_EIS 1 
#define ROUND_TO_2X(x) ((x ) & (~0x1))    

//FIXME, it is better not to use this. 
static CameraIO75 *gCameraIO75Obj = NULL; 

/*******************************************************************************
*
*******************************************************************************/
ICameraIO*
ICameraIO::
createInstance()
{
    static CameraIO75 singleton; 
    //FIXME, it is better not be used
    gCameraIO75Obj = &singleton; 
    return &singleton; 
}

/*******************************************************************************
*
*******************************************************************************/
MVOID
CameraIO75::
destroyInstance()
{
}

/*******************************************************************************
*
*******************************************************************************/
CameraIO75::
CameraIO75()
    :mZoomRectSrc()
    ,mZoomRectDst() 
    ,mZoomVal(100)
    ,mCurPipeEngine(ePipeNone)
    ,mPreviewPort(0)
    ,mpResMgr(NULL)
    ,mpISPHalObj(NULL)
    ,mpMDPHalObj(NULL)
    ,mMDPWorkingBuf(NULL)
#ifdef  MTK_NATIVE_3D_SUPPORT
    ,mMDPWorkingBufTg1(NULL)
    ,mMDPWorkingBufTg2(NULL)
    ,mpN3DObj(NULL)
    ,mTg1CropRect()
    ,mTg2CropRect()
    ,mpInDataN3D(NULL)
    ,mpRunDataN3D(NULL)
    ,mpOutDataN3D(NULL)
    ,mpTg1Offset(NULL)
    ,mpTg2Offset(NULL)
    ,mpTg1Addr(0)
    ,mpTg2Addr(0)
    ,meCameraMode(eCAMERA_MODE_DEFAULT)
#endif
    ,meSensorMode(eSENSOR_CAPTURE_MODE)
    ,mpCameraIOCB(NULL)
{
    MCAM_DBG("[CameraIO75] E\n"); 
    ::memset(&mMDPParam, 0, sizeof(halIDPParam_t)); 
    ::memset(&mISPParam, 0, sizeof(halISPIFParam_t)); 
}

/*******************************************************************************
*
*******************************************************************************/
CameraIO75::
~CameraIO75()
{
    MCAM_DBG("[~CameraIO75] E\n"); 
    mpISPHalObj = NULL; 
    mpMDPHalObj = NULL;     
    if (mMDPWorkingBuf) {
        delete mMDPWorkingBuf; 
        mMDPWorkingBuf = NULL; 
    }

#ifdef  MTK_NATIVE_3D_SUPPORT
    if (mMDPWorkingBufTg1) {
        delete mMDPWorkingBufTg1; 
        mMDPWorkingBufTg1 = NULL; 
    }
    if (mMDPWorkingBufTg2) {
        delete mMDPWorkingBufTg2; 
        mMDPWorkingBufTg2 = NULL; 
    }
#endif

    //FIXME, should remove it 
    gCameraIO75Obj = NULL; 
    mpCameraIOCB = NULL; 
}

/*******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::
init(
    IspHal* const pIspHalObj
)
{
    MINT32 err = 0; 
    MCAM_DBG("[init] E \n"); 
    //
#ifdef  MTK_NATIVE_3D_SUPPORT
    mpN3DObj = N3DHalBase::createInstance();
    if  ( ! mpN3DObj ) {
        MCAM_ERR("[init] N3DHalBase::createInstance()");
        return  -1;
    }
    mpInDataN3D = new IN_DATA_N3D_T;
    mpRunDataN3D = new RUN_DATA_N3D_T;
    mpOutDataN3D = new OUT_DATA_N3D_T;
#endif
    //
    mpResMgr = ResMgrHal::CreateInstance();
    if  ( ! mpResMgr ) {
        MCAM_ERR("[init] ResMgrHal::CreateInstance()");
        return  -1;
    }
    if  ( ! mpResMgr->Init() ) {
        MCAM_ERR("[init] mpResMgr->Init()");
        mpResMgr->DestroyInstance();
        mpResMgr = NULL;
        return  -1;
    }
    //
    if (NULL == pIspHalObj) {
        MCAM_ERR("[init] Null pIspHal Obj \n"); 
        return -1; 
    }
    mpISPHalObj = pIspHalObj; 
    mpMDPHalObj = MdpHal::createInstance();
    err = mpMDPHalObj->init();
    if (err < 0) {
        MCAM_ERR("[init] mpMdpHalObj->init err (0x%x)\n", err); 
        return err;
    }
    ::memset(&mISPParam, 0, sizeof(halISPIFParam_t)); 
    ::memset(&mMDPParam, 0, sizeof(halIDPParam_t)); 

    char value[PROPERTY_VALUE_MAX] = {'\0'}; 
    property_get("debug.camera.debug", value, "0"); 
    MCAM_LOG_DBG = atoi(value); 
    return 0;
}

/*******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::
uninit()
{
    MINT32 err = 0; 
    MCAM_DBG("[uninit] E \n"); 
    ::memset(&mISPParam, 0, sizeof(halISPIFParam_t)); 
    ::memset(&mMDPParam, 0, sizeof(halIDPParam_t)); 
    //
    err = mpMDPHalObj->uninit();
    if (err < 0) {
        MCAM_ERR("[uninit] mpMdpHalObj->uninit err (0x%x)\n", err); 
        return err;
    }
    mpMDPHalObj->destroyInstance();
    mpMDPHalObj = NULL;
    //
    if  ( mpResMgr ) {
        mpResMgr->Uninit();
        mpResMgr->DestroyInstance();
        mpResMgr = NULL;
    }
    //
#ifdef  MTK_NATIVE_3D_SUPPORT
    if  ( mpN3DObj ) {
        mpN3DObj->destroyInstance();
        mpN3DObj = NULL;
    }

    delete mpInDataN3D;
    delete mpRunDataN3D;
    delete mpOutDataN3D;
#endif

    return 0;
}    

/*******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::
start()
{
    AutoCPTLog cptlog(Event_CamIO_start);
    MCAM_DBG("[start], mCurPipeEngine = 0x%x \n", mCurPipeEngine); 
    MINT32 err = 0;
    //
    //lock pipe
    if  ( ! lockPipe(MTRUE) )
    {
        MCAM_ERR("[start] lockPipe(1) fail");
        return  -1;
    }
    //
    if (mCurPipeEngine & ePipeMDP) {
        CPTLogStr(Event_CamIO_start, CPTFlagSeparator, "config Mdp");
        err = mpMDPHalObj->setConf(&mMDPParam);
        if (err < 0) {
            return err;
        }
    }
    //
    if (mCurPipeEngine & ePipeISP) {
        CPTLogStr(Event_CamIO_start, CPTFlagSeparator, "config Isp");
        err = mpISPHalObj->setConf(&mISPParam);
        if (err < 0) {
            return err;
        }
    }

    if (mCurPipeEngine & ePipeMDP) {
        CPTLogStr(Event_CamIO_start, CPTFlagSeparator, "start Mdp");
        err = mpMDPHalObj->start();
        if (err < 0) {
            MCAM_ERR("[start] mdp start fail \n"); 
            return err;
        }
    }
    //
    if (mCurPipeEngine & ePipeISP) {
        CPTLogStr(Event_CamIO_start, CPTFlagSeparator, "start Isp");
        err = mpISPHalObj->start();
        if (err < 0) {
            MCAM_ERR("[start] ISP start fail \n"); 
            return err;
        }        
    }
    return 0;
} 

/*******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::
stop()
{
    MCAM_DBG("[stop], mCurPipeEngine = 0x%x \n", mCurPipeEngine); 
    MINT32 err = 0; 
    //
    if (mCurPipeEngine & ePipeISP) {
        err = mpISPHalObj->stop();
        if (err < 0) {
            MCAM_ERR("[stop] ISP stop fail \n"); 
            return err;
        }        
    }
    //
    if (mCurPipeEngine & ePipeMDP) {
        err = mpMDPHalObj->stop();
        if (err < 0) {
            MCAM_ERR("[stop] mdp stop fail \n"); 
            return err;
        }
    }
    //
    //unlock pipe
    lockPipe(MFALSE);
    //
    if (mMDPWorkingBuf) {
        delete mMDPWorkingBuf; 
        mMDPWorkingBuf = NULL; 
    } 
#ifdef  MTK_NATIVE_3D_SUPPORT
    mpN3DObj->N3DReset();
    if (mMDPWorkingBufTg1) {
        delete mMDPWorkingBufTg1; 
        mMDPWorkingBufTg1 = NULL; 
    }
    if (mMDPWorkingBufTg2) {
        delete mMDPWorkingBufTg2;
        mMDPWorkingBufTg2 = NULL; 
    }
#endif
    return err;
} 


/*******************************************************************************
*
*******************************************************************************/
MBOOL
CameraIO75::
lockPipe(MBOOL const isOn)
{
    MBOOL ret = MTRUE;

    //
    //  ISP -> MEM
    if  ( 0==(mCurPipeEngine & ePipeMDP) &&  0!=(mCurPipeEngine & ePipeISP) )
    {
        ret = ret && lockPipe_Isp2Mem(isOn);
        if  ( ! ret )
        {
            MCAM_ERR("[lockPipe] lockPipe_Isp2Mem(%d) fail", isOn);
        }
    }

    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
CameraIO75::
lockPipe_Isp2Mem(MBOOL const isOn)
{
    MCAM_DBG("[lockPipe_Isp2Mem] on/off(%d)", isOn);
    RES_MGR_HAL_MODE_STRUCT ModeInfo;
    ModeInfo.Dev    = RES_MGR_HAL_DEV_CAM;  //  FIXME: could be ATV?
    ModeInfo.Mode   = RES_MGR_HAL_MODE_CAPTURE_ON;
    ModeInfo.ModeSub= isOn ? RES_MGR_HAL_MODE_SUB_ISP2MEM : RES_MGR_HAL_MODE_SUB_NONE;

    MBOOL ret = mpResMgr->SetMode(&ModeInfo);
    if  ( ! ret ) 
    {
        MCAM_ERR("[lockPipe_Isp2Mem] on/off(%d): mpResMgr->SetMode()", isOn);
    }
    return  ret;
}


/*******************************************************************************
* Real-time priority, DISP > VIDEO > FD 
*******************************************************************************/
MINT32 
CameraIO75::
getPreviewFrame(
    ePREVIEW_PORT_ID const ePortID,
    BuffInfo_t* buffInfo
)
{
    MCAM_DBG1("[getPreviewFrame] port id = %d\n", ePortID); 
    MINT32 err = 0; 
    MINT32 i = 0;
    MINT32 mEsdCount=0;
    halMdpBufInfo_t mdpBufInfo; 
    ::memset(&mdpBufInfo, 0 , sizeof(halMdpBufInfo_t)); 
    if (mPreviewPort & ePortID & ePREVIEW_DISP_PORT) {
       if (mMDPParam.working_buffer_addr != 0) {
            err = mpMDPHalObj->dequeueBuff(PREVIEW_WORKING_BUFFER, &mdpBufInfo);
            if (err != 0) {
                MCAM_DBG("[getPreviewFrame] PREVIEW_WORKING_BUFFER: err:%d \n",err);
                mpISPHalObj->dumpReg(); 
#if 1 //for ESD
                MCAM_DBG("[ESD]+ \n");
                //while (0 != mpISPHalObj->waitDone(ISP_HAL_IRQ_VSYNC,200)){
                while(1){
                    mEsdCount++;
                    MCAM_DBG("[ESD][ISP_HAL_IRQ_VSYNC]Check Times: %d\n",mEsdCount);
                    if (mEsdCount>=2){
                        startESD();                    
                        err = mpMDPHalObj->dequeueBuff(PREVIEW_WORKING_BUFFER, &mdpBufInfo);
                        MCAM_DBG("[ESD]ESD Done. Working buff: err:%d \n",err);
                        break;
                     }
                }
                mEsdCount = 0;
                MCAM_DBG("[ESD]- \n");
#endif
            }
        }
        if (err == 0){
            //one pass preview path only output working buffer, do not dequeue/release disp port 
            if(mMDPParam.en_one_pass_preview_path == 0){
                err = mpMDPHalObj->dequeueBuff(DISP_PORT, &mdpBufInfo); 
                if (err != 0) {
                    MCAM_DBG("[getPreviewFrame] DISP_PORT: err:%d \n",err);
                    mpISPHalObj->dumpReg();
                }
            }
        }
    }
    else if (mPreviewPort & ePortID & ePREVIEW_VIDEO_PORT) {
        err = mpMDPHalObj->dequeueBuff(VDOENC_PORT, &mdpBufInfo); 
        if (err != 0) {
            mpISPHalObj->dumpReg();
        }
    }
    else if (mPreviewPort & ePortID & ePREVIEW_FD_PORT) { 
        err = mpMDPHalObj->dequeueBuff(FD_PORT, &mdpBufInfo); 
    }
    else if ((mPreviewPort & ePREVIEW_DISP_PORT) && (ePortID & ePREVIEW_DISP_PORT_EX)) {
        err = mpMDPHalObj->dequeueBuff(DISP_PORT, &mdpBufInfo);
        if (err != 0) {
            mpISPHalObj->dumpReg();
        }
    }
    else if ((mPreviewPort & ePREVIEW_DISP_PORT) && (ePortID & ePREVIEW_ZSD_PORT))
    {
        err = mpMDPHalObj->dequeueBuff(ZSD_PORT, &mdpBufInfo);
        if (err != 0) {
            mpISPHalObj->dumpReg();
        }
    }
    buffInfo->hwIndex = mdpBufInfo.hwIndex; 
    buffInfo->fillCnt = mdpBufInfo.fillCnt; 
    buffInfo->bufAddr = 0;      //TODO 
    buffInfo->timeStampS = mdpBufInfo.timeStamp.timeStampS[mdpBufInfo.hwIndex]; 
    buffInfo->timeStampUs = mdpBufInfo.timeStamp.timeStampUs[mdpBufInfo.hwIndex]; 
    return err; 
}   

/*******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::
releasePreviewFrame (
    ePREVIEW_PORT_ID const ePortID,
    BuffInfo_t* const buffInfo
)
{
    if (NULL == mpMDPHalObj) {
        MCAM_ERR("[releasePreviewFrame] Null MDP Obj \n"); 
        return -1; 
    }
    MCAM_DBG1("[releasePreviewFrame] portID = %d \n", ePortID); 
    if (mPreviewPort & ePortID & ePREVIEW_DISP_PORT) {
        // working buffer should sync with DISP port 
        //if (mMDPWorkingBuf != NULL) {
        if (mMDPParam.working_buffer_addr != 0) {
            mpMDPHalObj->enqueueBuff(PREVIEW_WORKING_BUFFER); 
        }
        //one pass preview path only output working buffer, do not dequeue/release disp port 
        if(mMDPParam.en_one_pass_preview_path == 0){
            mpMDPHalObj->enqueueBuff(DISP_PORT); 
        }
    }
    else if (mPreviewPort & ePortID & ePREVIEW_VIDEO_PORT) {
        mpMDPHalObj->enqueueBuff(VDOENC_PORT); 
    }
    else if (mPreviewPort & ePortID & ePREVIEW_FD_PORT) {
        mpMDPHalObj->enqueueBuff(FD_PORT); 
    }
    else if ((mPreviewPort & ePREVIEW_DISP_PORT) && (ePortID & ePREVIEW_DISP_PORT_EX)) {
        mpMDPHalObj->enqueueBuff(DISP_PORT);
        mpMDPHalObj->sendCommand(CMD_TRIGGER_LAST_FRAME_DISPLAY);
    }
    else if ((mPreviewPort & ePREVIEW_DISP_PORT) && (ePortID & ePREVIEW_ZSD_PORT))
    {
        //if (mMDPWorkingBuf != NULL) {
        if (mMDPParam.working_buffer_addr != NULL) {
            mpMDPHalObj->enqueueBuff(ZSD_PORT);
        }

    }
    else {
        MCAM_ERR("[releasePreviewFrame] not enable port id = %d\n", ePortID); 
    }
    return 0; 
} 

/*******************************************************************************
*    // Preview Pipe connection
*    // 1. Sensor --> ISP --> MDP --> Mem 
*    // 2. MEM --> ISP --> MDP --> Mem (TODO)
*******************************************************************************/
MINT32 
CameraIO75::
setPreviewPipe (
    MUINT32 eEnablePortID, 
    MemInfo_t* const pInMemInfo,                    //I: Mem Info 
    PreviewPipeCfg_t* const pPreviewPipeCfg ,       //I: Prv Pipe Config
    MemInfo_t* const pDispMemInfo,                  //O: Disp Mem Info 
    MemInfo_t* const pVideoMemInfo,                 //O: Video Mem Info 
    MemInfo_t* const pFDMemInfo                     //O: FD Mem Info                                                  
)
{
    MCAM_DBG("[setPreviewPipe] E portID = %d\n", eEnablePortID); 
    //
    ::memset(&mISPParam, 0, sizeof(halISPIFParam_t)); 
    ::memset(&mMDPParam, 0, sizeof(halIDPParam_t)); 
    // connect ISP -> MDP 
    MUINT32 sensorW = 0, sensorH = 0; 
    MINT32 err = 0; 
    if (NULL == pInMemInfo) { //Sensor --> ISP --> MDP
        getSensorRawSize(sensorW, sensorH); 
        //
        ImgCfg_t ispImgCfg; 
        ::memset(&mISPParam, 0, sizeof(halISPIFParam_t)); 
        mISPParam.u4SrcW = sensorW;
        mISPParam.u4SrcH = sensorH;
        mISPParam.u4IsContinous = 1;
    }
    //
    mISPParam.SubsampleWidth.Enable = MFALSE;
    mISPParam.SubsampleHeight.Enable = MFALSE;
    mISPParam.u4IsBypassSensorDelay = 1;    
    mISPParam.Scen = ISP_HAL_SCEN_CAM_PRV;
    //
    MUINT32 ispW = 0, ispH = 0; 
    mMDPParam.debug_preview_single_frame_enable = mISPParam.u4IsContinous ? 0 : 1;
    getIspDestResolution(ispW, ispH); 
    mMDPParam.src_size_w = ispW; 
    mMDPParam.src_size_h = ispH; 
    
    if(eSENSOR_CAPTURE_MODE == meSensorMode && 1920 == pPreviewPipeCfg->pDispConfig->imgWidth && 1088 == pPreviewPipeCfg->pDispConfig->imgHeight)
    {
        
        mISPParam.u4IsZsd = 1;

        MCAM_DBG("[setPreviewPipe] use mdp one pass preview path \n"); 

        // Working Buf config 
        // Working path is Sensor -> ISP -> MDP(CRZ) --> MEM.
        Rect_t rSrc, rDst, rCrop; 
        rSrc.w = ispW;
        rSrc.h = ispH;   
        rDst.w = pPreviewPipeCfg->pDispConfig->imgWidth;
        rDst.h = pPreviewPipeCfg->pDispConfig->imgHeight; 
        //
        calCropRect(rSrc, rDst, &rCrop, 100);
        mMDPParam.src_roi.x = rCrop.x; 
        mMDPParam.src_roi.y = rCrop.y; 
        mMDPParam.src_roi.w = rCrop.w; 
        mMDPParam.src_roi.h = rCrop.h;     
        //
        mMDPParam.en_one_pass_preview_path = 1;
        //
#if defined (MTK_M4U_SUPPORT)
        mMDPParam.working_buffer_addr = pDispMemInfo->virtAddr; 
#else 
        mMDPParam.working_buffer_addr = pDispMemInfo->phyAddr; 
#endif 
        //
        mMDPParam.working_buffer_count = pDispMemInfo->bufCnt;
        mMDPParam.working_color_format = (MdpColorFormat)mapMDPColorFormat(pPreviewPipeCfg->pDispConfig->imgFmt);
        mMDPParam.working_img_size.w = pPreviewPipeCfg->pDispConfig->imgWidth; 
        mMDPParam.working_img_size.h = pPreviewPipeCfg->pDispConfig->imgHeight; 
        mMDPParam.working_rotate = pPreviewPipeCfg->pDispConfig->rotation;
        mMDPParam.working_flip = pPreviewPipeCfg->pDispConfig->flip;

        MCAM_DBG("[setPreviewPipe] w = %d, h = %d, ftm = 0x%x, rotate = %d, flip = %d\n", 
             mMDPParam.working_img_size.w,  mMDPParam.working_img_size.h, mMDPParam.working_color_format,  mMDPParam.working_rotate, mMDPParam.working_flip );

        MCAM_DBG("[setPreviewPipe] mem w = %d, h = %d, virtAddr = 0x%x, phyAddr = 0x%x, bufCnt = %d, bufSize = 0x%x\n",
             pDispMemInfo->virtAddr, pDispMemInfo->phyAddr, pDispMemInfo->bufCnt, pDispMemInfo->bufSize); 

        NSCamCustom::TuningParam_CRZ_T const& crz_param = NSCamCustom::getParam_CRZ_Preview();
        mMDPParam.prv_resz_coeff.crz_up_scale_coeff = crz_param.uUpScaleCoeff;
        mMDPParam.prv_resz_coeff.crz_dn_scale_coeff = crz_param.uDnScaleCoeff;
        MCAM_DBG("[setPreviewPipe]<ePREVIEW_DISP_PORT> CRZ:(up,dn)=(%d,%d)", mMDPParam.prv_resz_coeff.crz_up_scale_coeff, mMDPParam.prv_resz_coeff.crz_dn_scale_coeff);
    }
    else
    {
        // Working Buf config 
        // Working path is Sensor -> ISP -> MDP(CRZ) --> MEM.
        Rect_t rSrc, rDst, rCrop; 
        rSrc.w = ispW;
        rSrc.h = ispH;   
        rDst.w = pPreviewPipeCfg->pInputConfig->imgWidth;
        rDst.h = pPreviewPipeCfg->pInputConfig->imgHeight; 
        //
        calCropRect(rSrc, rDst, &rCrop, 100);
        mMDPParam.src_roi.x = rCrop.x; 
        mMDPParam.src_roi.y = rCrop.y; 
        mMDPParam.src_roi.w = rCrop.w; 
        mMDPParam.src_roi.h = rCrop.h;     
        //
        // resizer quality tuning
        if (eEnablePortID & ePREVIEW_VIDEO_PORT) {
            NSCamCustom::TuningParam_CRZ_T const& crz_param = NSCamCustom::getParam_CRZ_Video();
            mMDPParam.prv_resz_coeff.crz_up_scale_coeff = crz_param.uUpScaleCoeff;
            mMDPParam.prv_resz_coeff.crz_dn_scale_coeff = crz_param.uDnScaleCoeff;
            //
            mMDPParam.prv_resz_coeff.prz1_up_scale_coeff = 8;
            mMDPParam.prv_resz_coeff.prz1_dn_scale_coeff = 15;
            mMDPParam.prv_resz_coeff.prz1_ee_h_str = 0;
            mMDPParam.prv_resz_coeff.prz1_ee_v_str = 0;
            //
            MCAM_DBG("[setPreviewPipe]<ePREVIEW_VIDEO_PORT> CRZ:(up,dn)=(%d,%d)", mMDPParam.prv_resz_coeff.crz_up_scale_coeff, mMDPParam.prv_resz_coeff.crz_dn_scale_coeff);
        }
        else if (eEnablePortID & ePREVIEW_DISP_PORT) {
            NSCamCustom::TuningParam_CRZ_T const& crz_param = NSCamCustom::getParam_CRZ_Preview();
            mMDPParam.prv_resz_coeff.crz_up_scale_coeff = crz_param.uUpScaleCoeff;
            mMDPParam.prv_resz_coeff.crz_dn_scale_coeff = crz_param.uDnScaleCoeff;
            //
            mMDPParam.prv_resz_coeff.prz0_up_scale_coeff = 8;
            mMDPParam.prv_resz_coeff.prz0_dn_scale_coeff = 15;
            mMDPParam.prv_resz_coeff.prz0_ee_h_str = 0;
            mMDPParam.prv_resz_coeff.prz0_ee_v_str = 0;
            //
            MCAM_DBG("[setPreviewPipe]<ePREVIEW_DISP_PORT> CRZ:(up,dn)=(%d,%d)", mMDPParam.prv_resz_coeff.crz_up_scale_coeff, mMDPParam.prv_resz_coeff.crz_dn_scale_coeff);
        }
        //
        // Disp Port 
        if (eEnablePortID & ePREVIEW_DISP_PORT) {
            if (NULL == pPreviewPipeCfg->pDispConfig || NULL == pDispMemInfo) {
                MCAM_ERR("[setZSDPreviewPipe] Null display port config  \n"); 
                return -1; 
            }
            configPrvMDPPort(ePREVIEW_DISP_PORT, pPreviewPipeCfg->pDispConfig, pDispMemInfo); 
        }
        // Video Port 
        if (eEnablePortID & ePREVIEW_VIDEO_PORT) {
            if (NULL == pPreviewPipeCfg->pVideoConfig || NULL == pVideoMemInfo) {
                MCAM_ERR("[setZSDPreviewPipe] Null video port config   \n"); 
                return -1; 
            }        
            configPrvMDPPort(ePREVIEW_VIDEO_PORT, pPreviewPipeCfg->pVideoConfig, pVideoMemInfo); 
        }
        // FD Port 
        if (eEnablePortID & ePREVIEW_FD_PORT) {
            if (NULL == pPreviewPipeCfg->pFDConfig || NULL == pFDMemInfo) {
                MCAM_ERR("[setZSDPreviewPipe] Null fd port config   \n"); 
                return -1; 
            }        
            configPrvMDPPort(ePREVIEW_FD_PORT, pPreviewPipeCfg->pFDConfig, pFDMemInfo); 
        }
        
        MUINT32 workBufW = pPreviewPipeCfg->pInputConfig->imgWidth; 
        MUINT32 workBufH = pPreviewPipeCfg->pInputConfig->imgHeight; 
        MCAM_DBG("[setPreviewPipe] workBuf (W, H) = (%d, %d)\n",workBufW, workBufH);    
        #if defined(MTK_M4U_SUPPORT)
        mMDPWorkingBuf = new mHalCamMemPool("MDPWorkBuf", workBufW * workBufH * 2 * MDP_WORK_BUFFER_CNT, 0); 
        mMDPParam.working_buffer_addr = mMDPWorkingBuf->getVirtAddr(); 
        #else 
        mMDPWorkingBuf = new mHalCamMemPool("MDPWorkBuf", workBufW * workBufH * 2 * MDP_WORK_BUFFER_CNT, 0); 
        mMDPParam.working_buffer_addr = mMDPWorkingBuf->getPhyAddr(); 
        #endif 
        if (0x0 == mMDPWorkingBuf->getVirtAddr()) {
            MCAM_ERR("[setPreviewPipe] No memory for MDP working buffer\n"); 
            return -1; 
        }
        mMDPParam.working_buffer_count = MDP_WORK_BUFFER_CNT; 
        mZoomRectSrc.w = workBufW; 
        mZoomRectSrc.h = workBufH; 
        mZoomRectDst.w = workBufW; 
        mZoomRectDst.h = workBufH; 
#if NEW_MDP_EIS
        mMDPParam.working_rotate = pPreviewPipeCfg->pInputConfig->rotation;
        mMDPParam.working_color_format = YUY2_Pack; 
        mMDPParam.working_img_size.w = workBufW; 
        mMDPParam.working_img_size.h = workBufH; 
        mMDPParam.working_img_roi.x = 0; 
        mMDPParam.working_img_roi.y = 0; 
        mMDPParam.working_img_roi.w = workBufW; //ispW; // pPreviewPipeCfg->pDispConfig->imgWidth;
        mMDPParam.working_img_roi.h =  workBufH; //ispH; //pPreviewPipeCfg->pDispConfig->imgHeight; 
        mMDPParam.en_sw_trigger = 1;   //enable EIS trigger; 
        // if the callback is null the MDP will not start EIS function, 
        // it can save the SYSRAM use 
        if (mpCameraIOCB != NULL) {
            mMDPParam.CB_EIS_INFO_GET_FUNC = eisGetInfoCallback;  
        }
#endif 
    }
    mCurPipeEngine = ePipeISP | ePipeMDP; 
    mPreviewPort = eEnablePortID; 
    MCAM_DBG("[setPreviewPipe] X \n"); 
    return err; 
}
/*******************************************************************************
*    // Last Preview Frame
*    // Output full size frame to working buffer
*    //
*******************************************************************************/
MINT32
CameraIO75::
setZSDLastPreviewFrame (
    MUINT32 const pDisWidth,            //I: Display size
    MUINT32 const pDisHeight,           //I: 
    MUINT32 const pZoomVal              //I: zoom value
)
{
    MUINT32 err;
    MUINT32 sensorW = 0, sensorH = 0;

    MCAM_DBG("[setZSDLastPreviewFrame]E, zoomVal=%d,W:%d,H:%d\n", pZoomVal,pDisWidth,pDisHeight);
    Rect_t rSrc;
    rect_t rCrop;
    Rect_t Crop;
    Rect_t Dest(0,0,pDisWidth,pDisHeight);
    ::memset(&rCrop, 0, sizeof(rect_t));

    err = mpISPHalObj->sendCommand(ISP_CMD_GET_SENSOR_FULL_RANGE, (int)&sensorW, (int)&sensorH);

    rSrc.w = sensorW;
    rSrc.h = sensorH;
    calCropRect(rSrc, Dest, &Crop, pZoomVal);
    rCrop.x = Crop.x;
    rCrop.y = Crop.y;
    rCrop.w = Crop.w;
    rCrop.h = Crop.h;

    err = mpMDPHalObj->sendCommand(CMD_SET_ZSD_LAST_PREVIEW_FRAME, (int) &rCrop);
    MCAM_DBG("[setZSDLastPreviewFrame] X\n");

    return err;
}

/*******************************************************************************
*    // Last Preview Frame
*    // Output full size frame to working buffer
*    //
*******************************************************************************/
MINT32
CameraIO75::
setZSDPreviewFrame (
    MUINT32 const pDisWidth,            //I: Display size
    MUINT32 const pDisHeight,           //I:
    MUINT32 const pZoomVal              //I: zoom value
)
{
    MUINT32 err;
    MUINT32 sensorW = 0, sensorH = 0;

    MCAM_DBG("[setZSDPreviewFrame]E, zoomVal=%d,W:%d,H:%d\n", pZoomVal,pDisWidth,pDisHeight);
    Rect_t rSrc;
    rect_t rCrop;
    Rect_t Crop;
    Rect_t Dest(0,0,pDisWidth,pDisHeight);
    ::memset(&rCrop, 0, sizeof(rect_t));

    err = mpISPHalObj->sendCommand(ISP_CMD_GET_SENSOR_FULL_RANGE, (int)&sensorW, (int)&sensorH);

    rSrc.w = sensorW;//sensorW;
    rSrc.h = sensorH;//sensorH;
    calCropRect(rSrc, Dest, &Crop, pZoomVal);
    rCrop.x = Crop.x;
    rCrop.y = Crop.y;
    rCrop.w = Crop.w;
    rCrop.h = Crop.h;

    err = mpMDPHalObj->sendCommand(CMD_SET_ZSD_PREVIEW_FRAME, (int) &rCrop);
    MCAM_DBG("[setZSDPreviewFrame] X\n");

    return err;
}

/*******************************************************************************
 *   // Zero Shutter Pipe connection
 *   // 1. Sensor --> ISP --> MDP --> Mem 
 *   // 2. MEM --> ISP --> MDP --> Mem 
*******************************************************************************/    //
MINT32 
CameraIO75::
setZSDPreviewPipe(
    MUINT32 const eEnablePortID, 
    MemInfo_t* const pInMemInfo,                   //I: Mem Info (Image is from mem) 
    PreviewPipeCfg_t* const pPreviewPipeCfg,       //I: Prv Pipe Config
    MemInfo_t* const pDispMemInfo,                 //O: Disp Mem Info 
    MemInfo_t* const pVideoMemInfo,                //O: video Mem Info
    MemInfo_t* const pFDMemInfo,                   //O: FD Mem Info                       
    MUINT32 const pSkipPrev,                       //I: Skip sensor mode switch 
    MUINT32 const pZSDPass                         //I: 1:old pass, 2:new pass
)
{
    MUINT32 sensorW =0, sensorH =0, u4PaddW =0, u4PaddH =0;
    MINT32 err =0; 
    halIspSensorType_e sensorType = ISP_SENSOR_TYPE_UNKNOWN; 

    ::memset(&mISPParam, 0, sizeof(halISPIFParam_t)); 
    ::memset(&mMDPParam, 0, sizeof(halIDPParam_t)); 

    if(pZSDPass == 0x1){
    
        MCAM_DBG("[setZSDPreviewPipe] E portID = %d\n", eEnablePortID); 
    // Get sensor size
    mpISPHalObj->sendCommand(ISP_CMD_GET_SENSOR_TYPE, (int) &sensorType);
    if (sensorType == ISP_SENSOR_TYPE_RAW) {
        mpISPHalObj->sendCommand(ISP_CMD_GET_RAW_DUMMY_RANGE, (int)&u4PaddW, (int)&u4PaddH);
    }
#if (1) //zsd preview pipe
    mpISPHalObj->sendCommand(ISP_CMD_GET_SENSOR_FULL_RANGE, (int)&sensorW, (int)&sensorH); 
#else
    mpISPHalObj->sendCommand(ISP_CMD_GET_SENSOR_PRV_RANGE, (int)&sensorW, (int)&sensorH); 
#endif

    if (NULL == pInMemInfo) {
        // Set ISP src size = sensor output size   
        mISPParam.u4SrcW = sensorW+u4PaddW;
        mISPParam.u4SrcH = sensorH+u4PaddH;
        mISPParam.u4IsContinous = 1;
        mISPParam.u4IsZsd = 1;
        MCAM_DBG("[setZSDPreviewPipe]ISP src: W:%d,H:%d\n", mISPParam.u4SrcW, mISPParam.u4SrcH); 
    }
    if (pSkipPrev){
         mISPParam.u4IsBypassSensorScenario = 1;
    }
    mISPParam.u4IsBypassSensorDelay = 1;
    
    // Set MDP size = isp output size
    mMDPParam.debug_preview_single_frame_enable = mISPParam.u4IsContinous ? 0 : 1;
    mMDPParam.en_zero_shutter_path = 1;
    //~!!this size must = mpd pass1 output size, mdp can't do resize in pass1(prz1)
    mMDPParam.src_size_w = sensorW;
    mMDPParam.src_size_h = sensorH;
    mMDPParam.src_roi.x = 0; 
    mMDPParam.src_roi.y = 0; 
    mMDPParam.src_roi.w = sensorW; 
    mMDPParam.src_roi.h = sensorH;       
    MCAM_DBG("[setZSDPreviewPipe]MDP src: W:%d,H:%d\n", mMDPParam.src_size_w, mMDPParam.src_size_h); 
    
    // Disp Port 
    if (eEnablePortID & ePREVIEW_DISP_PORT) {
        if (NULL == pPreviewPipeCfg->pDispConfig || NULL == pDispMemInfo) {
            MCAM_ERR("[setZSDPreviewPipe] Null display port config  \n"); 
            return -1; 
        }
        configPrvMDPPort(ePREVIEW_DISP_PORT, pPreviewPipeCfg->pDispConfig, pDispMemInfo); 
    }
    // ZSD use Video Port 
    if (eEnablePortID & ePREVIEW_VIDEO_PORT) {
        if (NULL ==  pPreviewPipeCfg->pVideoConfig || NULL == pVideoMemInfo) {
            MCAM_ERR("[setZSDPreviewPipe] Null zsd port config  \n"); 
            return -1; 
        }
        configPrvMDPPort(ePREVIEW_VIDEO_PORT, pPreviewPipeCfg->pVideoConfig, pVideoMemInfo); 
    }
    // FD Port 
    if (eEnablePortID & ePREVIEW_FD_PORT) {
        if (NULL == pPreviewPipeCfg->pFDConfig || NULL == pFDMemInfo) {
            MCAM_ERR("[setZSDPreviewPipe] Null fd port config \n"); 
            return -1; 
        }        
        configPrvMDPPort(ePREVIEW_FD_PORT, pPreviewPipeCfg->pFDConfig, pFDMemInfo); 
    }

    mCurPipeEngine = ePipeISP | ePipeMDP; 
    mPreviewPort = eEnablePortID; 
    MCAM_DBG("[setZSDPreviewPipe] X \n"); 
    return err; 
    }
    else{
        MCAM_DBG("[setZSDPreviewPipe.N]E, portID = %d\n", eEnablePortID);
        
        // Get sensor size
        mpISPHalObj->sendCommand(ISP_CMD_GET_SENSOR_TYPE, (int) &sensorType);
        if (sensorType == ISP_SENSOR_TYPE_RAW) {
            mpISPHalObj->sendCommand(ISP_CMD_GET_RAW_DUMMY_RANGE, (int)&u4PaddW, (int)&u4PaddH);
        }
    #if (1) //zsd preview pipe
        mpISPHalObj->sendCommand(ISP_CMD_GET_SENSOR_FULL_RANGE, (int)&sensorW, (int)&sensorH);
    #else
        mpISPHalObj->sendCommand(ISP_CMD_GET_SENSOR_PRV_RANGE, (int)&sensorW, (int)&sensorH);
    #endif
    
        if (NULL == pInMemInfo) {
            // Set ISP src size = sensor output size
            mISPParam.u4SrcW = sensorW+u4PaddW;
            mISPParam.u4SrcH = sensorH+u4PaddH;
            mISPParam.u4IsContinous = 1;
            mISPParam.u4IsZsd = 1;
            MCAM_DBG("[setZSDPreviewPipe.N]ISP src: W:%d,H:%d\n", mISPParam.u4SrcW, mISPParam.u4SrcH);
        }
        if (pSkipPrev){
             mISPParam.u4IsBypassSensorScenario = 1;
        }
        mISPParam.u4IsBypassSensorDelay = 1;
        mMDPParam.debug_preview_single_frame_enable = mISPParam.u4IsContinous ? 0 : 1;
        mMDPParam.en_zero_shutter_path = 2; /// 1:1st ZSD 2: 2nd ZSD
    
        mMDPParam.src_size_w = sensorW;
        mMDPParam.src_size_h = sensorH;
        // Working Buf config
        // Working path is Sensor -> ISP -> MDP(CRZ) --> MEM.
        Rect_t rSrc, rDst, rCrop;
        rSrc.w = sensorW;
        rSrc.h = sensorH;
        rDst.w = pPreviewPipeCfg->pInputConfig->imgWidth;
        rDst.h = pPreviewPipeCfg->pInputConfig->imgHeight;
    
        //
        calCropRect(rSrc, rDst, &rCrop, 100);
        mMDPParam.src_roi.x = rCrop.x;
        mMDPParam.src_roi.y = rCrop.y;
        mMDPParam.src_roi.w = rCrop.w;
        mMDPParam.src_roi.h = rCrop.h;
        //
        // resizer quality tuning
        if (eEnablePortID & ePREVIEW_VIDEO_PORT) {
            NSCamCustom::TuningParam_CRZ_T const& crz_param = NSCamCustom::getParam_CRZ_Video();
            mMDPParam.prv_resz_coeff.crz_up_scale_coeff = crz_param.uUpScaleCoeff;
            mMDPParam.prv_resz_coeff.crz_dn_scale_coeff = crz_param.uDnScaleCoeff;
            //
            mMDPParam.prv_resz_coeff.prz1_up_scale_coeff = 8;
            mMDPParam.prv_resz_coeff.prz1_dn_scale_coeff = 15;
            mMDPParam.prv_resz_coeff.prz1_ee_h_str = 0;
            mMDPParam.prv_resz_coeff.prz1_ee_v_str = 0;
            //
            MCAM_DBG("[setZSDPreviewPipe.N]<ePREVIEW_VIDEO_PORT> CRZ:(up,dn)=(%d,%d)", mMDPParam.prv_resz_coeff.crz_up_scale_coeff, mMDPParam.prv_resz_coeff.crz_dn_scale_coeff);
        }
        else if (eEnablePortID & ePREVIEW_DISP_PORT) {
            NSCamCustom::TuningParam_CRZ_T const& crz_param = NSCamCustom::getParam_CRZ_Preview();
            mMDPParam.prv_resz_coeff.crz_up_scale_coeff = crz_param.uUpScaleCoeff;
            mMDPParam.prv_resz_coeff.crz_dn_scale_coeff = crz_param.uDnScaleCoeff;
            //
            mMDPParam.prv_resz_coeff.prz0_up_scale_coeff = 8;
            mMDPParam.prv_resz_coeff.prz0_dn_scale_coeff = 15;
            mMDPParam.prv_resz_coeff.prz0_ee_h_str = 0;
            mMDPParam.prv_resz_coeff.prz0_ee_v_str = 0;
            //
            MCAM_DBG("[setZSDPreviewPipe.N]<ePREVIEW_DISP_PORT> CRZ:(up,dn)=(%d,%d)", mMDPParam.prv_resz_coeff.crz_up_scale_coeff, mMDPParam.prv_resz_coeff.crz_dn_scale_coeff);
        }
        //
        // Disp Port
        if (eEnablePortID & ePREVIEW_DISP_PORT) {
            if (NULL == pPreviewPipeCfg->pDispConfig || NULL == pDispMemInfo) {
                MCAM_ERR("[setZSDPreviewPipe.N] Null display port config  \n");
                return -1;
            }
            configPrvMDPPort(ePREVIEW_DISP_PORT, pPreviewPipeCfg->pDispConfig, pDispMemInfo);
            //! keep the width/height for zoom src
            //mZoomSrcW = pPreviewPipeCfg->pDispConfig->imgWidth;
            //mZoomSrcH = pPreviewPipeCfg->pDispConfig->imgHeight;
        }
        // Video Port
        if (eEnablePortID & ePREVIEW_VIDEO_PORT) {
            if (NULL == pPreviewPipeCfg->pVideoConfig || NULL == pVideoMemInfo) {
                MCAM_ERR("[setZSDPreviewPipe.N] Null video port config   \n");
                return -1;
            }
            configPrvMDPPort(ePREVIEW_VIDEO_PORT, pPreviewPipeCfg->pVideoConfig, pVideoMemInfo);
            //mZoomSrcW = mZoomSrcW > pPreviewPipeCfg->pVideoConfig->imgWidth ? mZoomSrcW: pPreviewPipeCfg->pVideoConfig->imgWidth;
           // mZoomSrcH = mZoomSrcH > pPreviewPipeCfg->pVideoConfig->imgHeight ? mZoomSrcH: pPreviewPipeCfg->pVideoConfig->imgHeight;
        }
        // FD Port
        if (eEnablePortID & ePREVIEW_FD_PORT) {
            if (NULL == pPreviewPipeCfg->pFDConfig || NULL == pFDMemInfo) {
                MCAM_ERR("[setZSDPreviewPipe.N] Null fd port config   \n");
                return -1;
            }
            configPrvMDPPort(ePREVIEW_FD_PORT, pPreviewPipeCfg->pFDConfig, pFDMemInfo);
        }
    
        MUINT32 workBufW = pPreviewPipeCfg->pInputConfig->imgWidth;
        MUINT32 workBufH = pPreviewPipeCfg->pInputConfig->imgHeight;
    
        MCAM_DBG("[setZSDPreviewPipe.N] workBuf (W %d, H %d)\n",workBufW, workBufH);
        #if defined(MTK_M4U_SUPPORT)
        mMDPWorkingBuf = new mHalCamMemPool("MDPWorkBuf", workBufW * workBufH * 2 * MDP_WORK_BUFFER_CNT, 0);
        mMDPParam.working_buffer_addr = mMDPWorkingBuf->getVirtAddr();
        //mMDPParam.working_buffer_addr = pVideoMemInfo->virtAddr;
        mMDPParam.working_buffer_addr_last_preview_ = pVideoMemInfo->virtAddr;
        mMDPParam.working_buffer_count_zsd = pVideoMemInfo->bufCnt;
        #else
        //mMDPWorkingBuf = new mHalCamMemPool("MDPWorkBuf", workBufW * workBufH * 2 * MDP_WORK_BUFFER_CNT, 0);
        //mMDPParam.working_buffer_addr = mMDPWorkingBuf->getPhyAddr();
        mMDPParam.working_buffer_addr = pVideoMemInfo->phyAddr;
        #endif
        //if (0x0 == mMDPWorkingBuf->getVirtAddr()) {
        if (0 == mMDPParam.working_buffer_addr) {
            MCAM_ERR("[setZSDPreviewPipe.N] No memory for MDP working buffer\n");
            return -1;
        }
        mMDPParam.working_buffer_count = MDP_WORK_BUFFER_CNT;
        mZoomRectSrc.w = workBufW; 
        mZoomRectSrc.h = workBufH; 
        mZoomRectDst.w = workBufW; 
        mZoomRectDst.h = workBufH; 
    
    #if NEW_MDP_EIS
        mMDPParam.working_rotate = pPreviewPipeCfg->pInputConfig->rotation;
        mMDPParam.working_color_format = YUV422_Pack;//YUY2_Pack;//RGB565;//YUY2_Pack;
        mMDPParam.working_img_size.w = workBufW;
        mMDPParam.working_img_size.h = workBufH;
        mMDPParam.working_img_roi.x = 0;
        mMDPParam.working_img_roi.y = 0;
        mMDPParam.working_img_roi.w = workBufW; //ispW; // pPreviewPipeCfg->pDispConfig->imgWidth;
        mMDPParam.working_img_roi.h =  workBufH; //ispH; //pPreviewPipeCfg->pDispConfig->imgHeight;
        mMDPParam.en_sw_trigger = 1;   //enable EIS trigger;
        // if the callback is null the MDP will not start EIS function,
        // it can save the SYSRAM use
        if (mpCameraIOCB != NULL) {
            mMDPParam.CB_EIS_INFO_GET_FUNC = eisGetInfoCallback;
        }
    #endif
        mCurPipeEngine = ePipeISP | ePipeMDP;
        mPreviewPort = eEnablePortID;
        MCAM_DBG("[setZSDPreviewPipe.N] X \n");
        return err;

    }
}


/*******************************************************************************
 * Capture Pipe
 *
 *
 * Case A
 *
 *   [Sensor Port   ]
 *   [MEM Port:Bayer] --> ISP --> [MEM Port:Bayer]
 *   [MEM Port:YUY2 ]             [MEM Port:YUY2 ]
 *
 *
 * Case B
 *
 *   [Sensor Port   ]                                            [Jpeg Port      ]
 *   [MEM Port:Bayer] --> ISP --> [Bypass Port:YUY2] --> MDP --> [Postview Port  ]
 *   [MEM Port:YUY2 ]                                            [MEM Port:others]
 *
*******************************************************************************/
MBOOL
CameraIO75::
setCapturePipe(PortVect const& rvpInPorts, PortVect const& rvpOutPorts)
{
    MBOOL   ret = MFALSE;

    //  (1) Reset
    mCurPipeEngine = 0; 
    ::memset(&mISPParam, 0, sizeof(halISPIFParam_t)); 
    ::memset(&mMDPParam, 0, sizeof(halIDPParam_t));
    //
    mISPParam.Scen = ISP_HAL_SCEN_CAM_CAP;
    //  (2) Check arguments.
    if  (
            rvpInPorts.isEmpty()  || ! rvpInPorts[0]
        ||  rvpOutPorts.isEmpty() || ! rvpOutPorts[0]
        )
    {
        goto lbExit;
    }

    //  (3) Check output formats.
    switch  ( rvpOutPorts[0]->eImgFmt )
    {
    case ePIXEL_FORMAT_BAYER8:
    case ePIXEL_FORMAT_BAYER10:
    case ePIXEL_FORMAT_BAYER12:
    case ePIXEL_FORMAT_YV12:
    case ePIXEL_FORMAT_YUY2:   //TODO, can it output from isp
    // * Case A
    // *   [Sensor Port   ]
    // *   [MEM Port:Bayer] --> ISP --> [MEM Port:Bayer]
    // *   [MEM Port:YUY2 ]             [MEM Port:YUY2 ]
        if  ( MemoryPortInfo const*const pMemPort = reinterpret_cast<MemoryPortInfo const*>(rvpOutPorts[0]))
        {
            if (pMemPort->fgDumpYuvData)
            {   
                MCAM_DBG("MDP dump yuv data for 1st capture pass");
                PortInfo const InPort = *rvpInPorts[0];
                PortInfo const OutPort = *rvpOutPorts[0];
                PortInfo BpsPort(ePortID_Bypass);
                //
                ret = queryImgInfoForIspYuvOut(InPort.ePortID, InPort, BpsPort);  //depend on input port & format.
                MCAM_DBG("[setCapturePipe] In Port(id:%d):(fmt, w, h)=(0x%x, %d, %d)", InPort.ePortID, InPort.eImgFmt, InPort.u4ImgWidth, InPort.u4ImgHeight);
                MCAM_DBG("[setCapturePipe] Bypass Port:(fmt, w, h)=(0x%x, %d, %d)", BpsPort.eImgFmt, BpsPort.u4ImgWidth, BpsPort.u4ImgHeight);
                MCAM_DBG("[setCapturePipe] Out Port(id:%d):(fmt, w, h)=(0x%x, %d, %d)", OutPort.ePortID, OutPort.eImgFmt, OutPort.u4ImgWidth, OutPort.u4ImgHeight);
                //
                PortVect vpBypasPorts;
                vpBypasPorts.push(&BpsPort);

                ret =   ret
                    &&  configISPParam(rvpInPorts, vpBypasPorts)
                    &&  configMDPParam(vpBypasPorts, rvpOutPorts)
                        ;
                if  ( ret )
                {
                    mCurPipeEngine = ePipeISP | ePipeMDP; 
                }
                
            }
            else
            {
                MCAM_DBG("ISP dump raw data for 1st capture pass");
                ret = configISPParam(rvpInPorts, rvpOutPorts);
                
                if  ( ret )
                {
                    mCurPipeEngine = ePipeISP; 
                }
            }
        }
        else
        {
            ret = MFALSE;
            MCAM_ERR("reinterpret_cast MemoryPortInfo fail");
        }
        break;

    default:
    // * Case B
    // *   [Sensor Port   ]                                            [Jpeg Port      ]
    // *   [MEM Port:Bayer] --> ISP --> [Bypass Port:YUY2] --> MDP --> [Postview Port  ]
    // *   [MEM Port:YUY2 ]                                            [MEM Port:others]
        {
            PortInfo const InPort = *rvpInPorts[0];
            PortInfo const OutPort = *rvpOutPorts[0];
            PortInfo BpsPort(ePortID_Bypass);
            //
            ret = queryImgInfoForIspYuvOut(InPort.ePortID, InPort, BpsPort);  //depend on input port & format.
            MCAM_DBG("[setCapturePipe] In Port(id:%d):(fmt, w, h)=(0x%x, %d, %d)", InPort.ePortID, InPort.eImgFmt, InPort.u4ImgWidth, InPort.u4ImgHeight);
            MCAM_DBG("[setCapturePipe] Bypass Port:(fmt, w, h)=(0x%x, %d, %d)", BpsPort.eImgFmt, BpsPort.u4ImgWidth, BpsPort.u4ImgHeight);
            MCAM_DBG("[setCapturePipe] Out Port(id:%d):(fmt, w, h)=(0x%x, %d, %d)", OutPort.ePortID, OutPort.eImgFmt, OutPort.u4ImgWidth, OutPort.u4ImgHeight);
            //
            PortVect vpBypasPorts;
            vpBypasPorts.push(&BpsPort);

            ret =   ret
                &&  configISPParam(rvpInPorts, vpBypasPorts)
                &&  configMDPParam(vpBypasPorts, rvpOutPorts)
                    ;
            if  ( ret )
            {
                mCurPipeEngine = ePipeISP | ePipeMDP; 
            }
        }
        break;
    }

lbExit:
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
CameraIO75::
queryImgInfoForIspYuvOut(EPortID const eInPortID, ImgInfo const InImgInfo, ImgInfo& rOutImgInfo)
{
    MBOOL   ret = MFALSE;
    MINT32  err = 0;
    //
    MUINT32 u4ImgWidth = 0;
    MUINT32 u4ImgHeight= 0;
    //
    MUINT32 u4PadWidth = 0;
    MUINT32 u4PadHeight= 0;
    //
    switch  (eInPortID)
    {
    case ePortID_Sensor:
        switch  (meSensorMode)
        {
        case eSENSOR_PREVIEW_MODE:
            err = mpISPHalObj->sendCommand(ISP_CMD_GET_SENSOR_PRV_RANGE, (MINT32)&u4ImgWidth, (MINT32)&u4ImgHeight);
            break;
        case eSENSOR_CAPTURE_MODE:
            err = mpISPHalObj->sendCommand(ISP_CMD_GET_SENSOR_FULL_RANGE, (MINT32)&u4ImgWidth, (MINT32)&u4ImgHeight);
            break;
        default:
            MCAM_ERR("[queryImgInfoForIspYuvOut] unknown sensor mode(%d)", meSensorMode);
            err = -1;
            break;
        }
        if  ( 0 != err )
        {
            MCAM_ERR("[queryImgInfoForIspYuvOut] fail to get isp sensor input - (meSensorMode, err)=(%d, %x)", meSensorMode, err);
            goto lbExit;
        }
        break;

    case ePortID_Memory:
        if  (0 == InImgInfo.u4ImgWidth || 0 == InImgInfo.u4ImgHeight)
        {
            MCAM_ERR("[queryImgInfoForIspYuvOut] bad isp memory input size:(format, w, h)=(%x, %d, %d)", InImgInfo.eImgFmt, InImgInfo.u4ImgWidth, InImgInfo.u4ImgHeight);
            goto lbExit;
        }
        //
        switch  (InImgInfo.eImgFmt)
        {
        case ePIXEL_FORMAT_BAYER8:
        case ePIXEL_FORMAT_BAYER10:
        case ePIXEL_FORMAT_BAYER12:
            err = mpISPHalObj->sendCommand(ISP_CMD_GET_RAW_DUMMY_RANGE, (MINT32)&u4PadWidth, (MINT32)&u4PadHeight);
            if  ( 0 != err)
            {
                MCAM_ERR("[queryImgInfoForIspYuvOut] ISP_CMD_GET_RAW_DUMMY_RANGE - err(%x)", err);
                goto lbExit;
            }
            u4ImgWidth = InImgInfo.u4ImgWidth  - u4PadWidth;
            u4ImgHeight= InImgInfo.u4ImgHeight - u4PadHeight;
            break;

        case ePIXEL_FORMAT_YUY2:
            u4ImgWidth = InImgInfo.u4ImgWidth;
            u4ImgHeight= InImgInfo.u4ImgHeight;
            break;

        default:
            MCAM_ERR("[queryImgInfoForIspYuvOut] unsupported isp memory input format(%x)", InImgInfo.eImgFmt);
            goto lbExit;
        }
        break;

    default:
        MCAM_ERR("[queryImgInfoForIspYuvOut] bad eInPortID(%d)", eInPortID);
        goto lbExit;
    }

    rOutImgInfo = ImgInfo(ePIXEL_FORMAT_YUY2, u4ImgWidth, u4ImgHeight);
    //
    ret = MTRUE;
lbExit:
    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::
getSensorRawSize(
    MUINT32 &width, 
    MUINT32 &height
)
{
    MINT32 err = 0; 
    MUINT32 sensorW = 0, sensorH = 0; 
    MUINT32 u4PaddW = 0, u4PaddH = 0;

    halIspSensorType_e sensorType = ISP_SENSOR_TYPE_RAW; 
    //
    err = mpISPHalObj->sendCommand(ISP_CMD_GET_SENSOR_TYPE, (int) &sensorType);
    if (err < 0) {
        MCAM_ERR("[getSensorRawSize] ISP_CMD_GET_SENSOR_TYPE err (0x%x)", err); 
        return err;
    }
    //
    if (sensorType == ISP_SENSOR_TYPE_RAW) {
        //
        err = mpISPHalObj->sendCommand(ISP_CMD_GET_RAW_DUMMY_RANGE, (int)&u4PaddW, (int)&u4PaddH);
        if  (err) {
            MCAM_ERR("[getSensorRawSize] ISP_CMD_GET_RAW_DUMMY_RANGE err(0x%x)", err);
            return err; 
        }
    }
    err = getIspDestResolution(sensorW, sensorH); 
 
    width = sensorW + u4PaddW; 
    height = sensorH + u4PaddH;     
    return err; 
}

/*******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::dropPreviewFrame()
{
    AutoCPTLog cptlog(Event_CamIO_dropPreviewFrame);
    MCAM_DBG("[dropPreviewFrame] E \n"); 
    BuffInfo_t buffInfo; 
    // Display port 
    if (mPreviewPort & ePREVIEW_DISP_PORT) {
        getPreviewFrame(ePREVIEW_DISP_PORT, &buffInfo); 
        MCAM_DBG("[dropPreviewFrame] disp cnt:%ld\n", buffInfo.fillCnt); 
        releasePreviewFrame(ePREVIEW_DISP_PORT, &buffInfo); 
        // flush the remain frame 
        int i4RemainBuf = buffInfo.fillCnt - 1; 
        for (int i = 0; i < i4RemainBuf; i++) {
            getPreviewFrame(ePREVIEW_DISP_PORT, &buffInfo); 
            MCAM_DBG("[dropPreviewFrame] disp cnt:%ld\n", buffInfo.fillCnt); 
            releasePreviewFrame(ePREVIEW_DISP_PORT, &buffInfo);             
        }
    }
    // Video port 
    if (mPreviewPort & ePREVIEW_VIDEO_PORT) {
        getPreviewFrame(ePREVIEW_VIDEO_PORT, &buffInfo);
        MCAM_DBG("[dropPreviewFrame] video cnt:%ld\n", buffInfo.fillCnt);         
        releasePreviewFrame(ePREVIEW_VIDEO_PORT, &buffInfo); 
        // flush the remain frame 
        int i4RemainBuf = buffInfo.fillCnt - 1; 
        for (int i = 0; i < i4RemainBuf; i++) {
            getPreviewFrame(ePREVIEW_VIDEO_PORT, &buffInfo); 
            MCAM_DBG("[dropPreviewFrame] video cnt:%ld\n", buffInfo.fillCnt); 
            releasePreviewFrame(ePREVIEW_VIDEO_PORT, &buffInfo);             
        }
    }
    //
/*   
    if (mPreviewPort & ePREVIEW_FD_PORT) {
        getPreviewFrame(ePREVIEW_DISP_PORT, buffInfo); 
        releasePreviewFrame(ePREVIEW_DISP_PORT, buffInfo); 
    }
*/
    return 0;
}

/*******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::
setPreviewZoom(
    MUINT32 const zoomVal,               //I: zoom value
    Rect_t* const pDest,                 //I: dest rect                                     
    Rect_t* pCrop                        //O: crop rect 
)
{
    MINT32 err = 0; 
    MCAM_DBG("[setPreviewZoom]zoomVal = %d\n", zoomVal); 
    if (NULL == pDest  || NULL == pCrop) {
        MCAM_ERR("[setPreviewZoom] pDest or pCrop argument is NULL\n"); 
        return -1; 
    }
    MUINT32 sensorW = 0, sensorH = 0; 
    Rect_t rSrc;
    rect_t rCrop; 
    ::memset(&rCrop, 0, sizeof(rect_t)); 
    //
    MCAM_DBG("srcW = %d, srcH = %d, dstW = %d, dstH = %d\n", 
                        mZoomRectSrc.w, mZoomRectSrc.h, pDest->w, pDest->h); 
    calCropRect(mZoomRectSrc, *pDest, pCrop, zoomVal);
    rCrop.x = pCrop->x; 
    rCrop.y = pCrop->y; 
    rCrop.w = pCrop->w; 
    rCrop.h = pCrop->h; 
    //
    err = mpMDPHalObj->sendCommand(CMD_SET_ZOOM_RATIO, (int) &rCrop);
    if (err != 0) {
        MCAM_ERR("[setPreviewZoom] CMD_SET_ZOOM_RATIO err:%x\n", err); 
    }
    // for upper layer, the src w, h is from sensor 
    getIspDestResolution(sensorW, sensorH); 
    rSrc.w = sensorW; 
    rSrc.h = sensorH; 
    calCropRect(rSrc, *pDest, pCrop, zoomVal); 
    //
    mZoomVal = zoomVal; 
    memcpy(&mZoomRectDst, pDest, sizeof(Rect_t)); 

    return err; 
} 

/*******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::
setZSDPreviewZoom(
    MUINT32 const zoomVal,               //I: zoom value
    Rect_t* const pDest,                 //I: dest rect                                     
    Rect_t* pCrop,                       //O: crop rect 
    MUINT32 const pZsdpass               //I: zsd pass
)
{
    MINT32 err = 0; 
    MCAM_DBG("[setZSDPreviewZoom]zoomVal = %d\n", zoomVal); 
    if (NULL == pDest  || NULL == pCrop) {
        MCAM_ERR("[setPreviewZoom] pDest or pCrop argument is NULL\n"); 
        return -1; 
    }
    MUINT32 sensorW = 0, sensorH = 0; 
    Rect_t rSrc; 
    rect_t rCrop;
    ::memset(&rCrop, 0, sizeof(rect_t)); 
    int zsdtestw, zsdtesth;
    
    if (pZsdpass == 0x1){
        //
        #if (1) //zsd preview pipe
        mpISPHalObj->sendCommand(ISP_CMD_GET_SENSOR_FULL_RANGE, (int)&sensorW, (int)&sensorH); 
        #else
        mpISPHalObj->sendCommand(ISP_CMD_GET_SENSOR_PRV_RANGE, (int)&sensorW, (int)&sensorH); 
        #endif
        rSrc.w = sensorW;
        rSrc.h = sensorH;
        //
        MCAM_DBG("[setZSDPreviewZoom]srcW = %d, srcH = %d, dstW = %d, dstH = %d\n", 
                            rSrc.w, rSrc.h, pDest->w, pDest->h); 
        calCropRect(rSrc, *pDest, pCrop, zoomVal);
        rCrop.x = pCrop->x; 
        rCrop.y = pCrop->y; 
        rCrop.w = pCrop->w; 
        rCrop.h = pCrop->h; 
        err = mpMDPHalObj->sendCommand(CMD_SET_ZOOM_RATIO, (int) &rCrop,(int)&zsdtestw,(int)&zsdtesth);
        MCAM_DBG("[setZSDPreviewZoom]rCrop.x = %d, rCrop.y = %d, rCrop.w = %d, rCrop.h = %d,zsdtestw = %d,zsdtesth=%d\n", 
                            rCrop.x, rCrop.y, rCrop.w, rCrop.h,zsdtestw,zsdtesth); 
        pCrop->w = zsdtestw;
        pCrop->h = zsdtesth;

        mZoomVal = zoomVal; 
        memcpy(&mZoomRectDst, pDest, sizeof(Rect_t)); 

        if (err != 0) {
            MCAM_ERR("[setPreviewZoom] CMD_SET_ZOOM_RATIO err:%x\n", err); 
        }
        return err; 
    }
    else{
         // MDP new pass
        MCAM_DBG("[setZSDPreviewZoom.N]srcW = %d, srcH = %d, dstW = %d, dstH = %d\n", 
                            mZoomRectSrc.w, mZoomRectSrc.h, pDest->w, pDest->h); 
        calCropRect(mZoomRectSrc, *pDest, pCrop, zoomVal);
        rCrop.x = pCrop->x; 
        rCrop.y = pCrop->y; 
        rCrop.w = pCrop->w; 
        rCrop.h = pCrop->h; 
        //
        err = mpMDPHalObj->sendCommand(CMD_SET_ZOOM_RATIO, (int) &rCrop);
        if (err != 0) {
            MCAM_ERR("[setZSDPreviewZoom.N] CMD_SET_ZOOM_RATIO err:%x\n", err); 
        }
        // for upper layer, the src w, h is from sensor 
        #if (1) //zsd preview pipe
            mpISPHalObj->sendCommand(ISP_CMD_GET_SENSOR_FULL_RANGE, (int)&sensorW, (int)&sensorH); 
        #else
            mpISPHalObj->sendCommand(ISP_CMD_GET_SENSOR_PRV_RANGE, (int)&sensorW, (int)&sensorH); 
        #endif
        
        rSrc.w = sensorW; 
        rSrc.h = sensorH; 
        calCropRect(rSrc, *pDest, pCrop, zoomVal); 
        //
        mZoomVal = zoomVal; 
        memcpy(&mZoomRectDst, pDest, sizeof(Rect_t)); 
        if (err != 0) {
            MCAM_ERR("[setZSDPreviewZoom.N] CMD_SET_ZOOM_RATIO err:%x\n", err); 
        }
        return err; 
    }
} 

/*******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::
waitCaptureDone()
{
    AutoCPTLog cptlog(Event_CamIO_waitCaptureDone);
    MCAM_DBG("[waitCaptureDone] E \n"); 
    MINT32 err = 0; 
    //if MDP pipe enable, only wait for MDP 
    if (mCurPipeEngine & ePipeMDP) {
        err = mpMDPHalObj->waitDone(0x01);
    }
    else if (mCurPipeEngine & ePipeISP) {
        err = mpISPHalObj->waitDone(ISP_HAL_IRQ_EXPDONE);
    }
    MCAM_DBG("[waitCaptureDone][Perf][Shutter Delay]End\n"); 
    return err; 
}

/*******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::
getJPEGSize(MUINT32 &jpegSize)
{
    return mpMDPHalObj->sendCommand(CMD_GET_JPEG_FILE_SIZE, (int) &jpegSize);
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
CameraIO75::
configISPParam(PortVect const& rvpInPorts, PortVect const& rvpOutPorts)
{
    char pipeConfig[64] = {'\0'};
    //
    MBOOL   ret = MFALSE;
    //
    MUINT32 u4InBufSize = 0;
    MUINT32 u4OutBufSize = 0;

    //  (0) Reset
//    ::memset(&mISPParam, 0, sizeof(mISPParam));

    //  (1) Input Ports.
    for (size_t i = 0; i < rvpInPorts.size(); i++)
    {
        PortInfo const*const pInPort = rvpInPorts[i];
        switch  ( pInPort->ePortID )
        {
        case ePortID_Sensor:
            ::sprintf(pipeConfig, "Sensor --> ISP");
            mISPParam.u4InPhyAddr = 0;
            mISPParam.u4IsSrcDram = 0;
            //  Note: dynamic_cast is not supported without rtti.
            if  ( SensorPortInfo const*const pSensorPort = reinterpret_cast<SensorPortInfo const*>(pInPort) )
            {
                mISPParam.u4IsBypassSensorDelay = pSensorPort->fgBypassSensorDelay;
                mISPParam.u4IsBypassSensorScenario = pSensorPort->fgBypassSensorScenaio;
            }
            break;
        case ePortID_Memory:
            ::sprintf(pipeConfig, "Mem --> ISP");
            #if (MTK_M4U_SUPPORT)
            mISPParam.u4InPhyAddr = pInPort->u4BufVA;
            #else
            mISPParam.u4InPhyAddr = pInPort->u4BufPA;
            #endif
            u4InBufSize = pInPort->u4BufSize * pInPort->u4BufCnt;
            mISPParam.u4IsSrcDram = 1;
            mISPParam.u4IsBypassSensorDelay = 1; 

            //
            // FIXME: Why destination format depends on input? Who knows
            mISPParam.u4IsDestBayer = (pInPort->eImgFmt == ePIXEL_FORMAT_BAYER8 || 
                                       pInPort->eImgFmt == ePIXEL_FORMAT_BAYER10 ||
                                       pInPort->eImgFmt == ePIXEL_FORMAT_BAYER12) ? 1 : 0; 

            break;
        default:
            MCAM_ERR("[configISPParam] bad input port %d with ID(%d)", i, pInPort->ePortID);
            goto lbExit;
        }
        mISPParam.u4SrcW = pInPort->u4ImgWidth;
        mISPParam.u4SrcH = pInPort->u4ImgHeight;

        //  TODO: now we just support 1 isp (memory) input.
        break;
    }

    //  (2) Output Ports.
    for (size_t i = 0; i < rvpOutPorts.size(); i++)
    {
        PortInfo const*const pOutPort = rvpOutPorts[i];
        if  ( ePortID_Memory == pOutPort->ePortID )
        {
            ::sprintf(pipeConfig, "%s --> Mem", pipeConfig);

            #if (MTK_M4U_SUPPORT)
            mISPParam.u4OutPhyAddr = pOutPort->u4BufVA;
            #else
            mISPParam.u4OutPhyAddr = pOutPort->u4BufPA;
            #endif
            u4OutBufSize = pOutPort->u4BufSize * pOutPort->u4BufCnt;
            mISPParam.u4IsDestDram = 1;
            mISPParam.u4IsDestBayer = (pOutPort->eImgFmt == ePIXEL_FORMAT_BAYER8  ||
                                       pOutPort->eImgFmt == ePIXEL_FORMAT_BAYER10 ||
                                       pOutPort->eImgFmt == ePIXEL_FORMAT_BAYER12) ? 1 : 0;

            //  TODO: now we just support 1 isp memory output.
            break;
        }
    }

    //  (3) Others.
    //  TODO: separate in/out sizes.
    mISPParam.u4MemSize = (u4InBufSize >= u4OutBufSize) ? u4InBufSize : u4OutBufSize;

    //
    MCAM_DBG("[configISPParam] Pipe Config = %s\n", pipeConfig);
    ret = MTRUE;
lbExit:
    return ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
CameraIO75::
configMDPParam(PortVect const& rvpInPorts, PortVect const& rvpOutPorts)
{
    MCAM_DBG("[configMDPParam]");
    //
    MBOOL   ret = MFALSE;

    Rect_t rcSrc;

    //  (0) Reset
//    ::memset(&mMDPParam, 0, sizeof(mMDPParam));

    //  (1) Input Ports.
    for (size_t i = 0; i < rvpInPorts.size(); i++)
    {
        PortInfo const*const pInPort = rvpInPorts[i];

        switch  ( pInPort->ePortID )
        {
        case ePortID_Bypass:  //  [in] -> ISP -> [bypass] -> MDP -> [out]
        case ePortID_Memory:  //  [memory] -> MDP -> [out]
            rcSrc.w = pInPort->u4ImgWidth;
            rcSrc.h = pInPort->u4ImgHeight;
            break;
        default:
            MCAM_ERR("[configMDPParam] bad input port %d with ID(%d)", i, pInPort->ePortID);
            goto lbExit;
        }

        //  TODO: now we just support 1 mdp (memory) input.
        break;
    }

    //  (2) Output Ports.
    for (size_t i = 0; i < rvpOutPorts.size(); i++)
    {
        PortInfo const*const pOutPort = rvpOutPorts[i];
        switch  ( pOutPort->ePortID )
        {
        case ePortID_Jpeg:
            MCAM_DBG("[configMDPParam] Jpeg");
            MCAM_ASSERT(ePIXEL_FORMAT_JPEG == pOutPort->eImgFmt, "Jpeg port without ePIXEL_FORMAT_JPEG format");
            //  Note: dynamic_cast is not supported without rtti.
            if  ( JpegPortInfo const*const pJpegPort = reinterpret_cast<JpegPortInfo const*>(pOutPort) )
            {
                Rect_t rcDst, rcCrop;
                rcDst.w = pJpegPort->u4ImgWidth;
                rcDst.h = pJpegPort->u4ImgHeight;
                calCropRect(rcSrc, rcDst, &rcCrop, pJpegPort->u4zoomRatio);
                //
                mMDPParam.mode = MDP_MODE_CAP_JPG;          // For capture mode
                //
                mMDPParam.Capture.src_img_size.w = rcSrc.w;
                mMDPParam.Capture.src_img_size.h = rcSrc.h;
                mMDPParam.Capture.src_img_roi.x = rcCrop.x;
                mMDPParam.Capture.src_img_roi.y = rcCrop.y;
                mMDPParam.Capture.src_img_roi.w = rcCrop.w;
                mMDPParam.Capture.src_img_roi.h = rcCrop.h;
                //
                mMDPParam.Capture.jpg_img_size.w = pJpegPort->u4ImgWidth;
                mMDPParam.Capture.jpg_img_size.h = pJpegPort->u4ImgHeight;
                mMDPParam.Capture.jpg_yuv_color_format = 422;   //integer : 411, 422, 444, 400, 410 etc...
                #if defined(MTK_M4U_SUPPORT)
                mMDPParam.Capture.jpg_buffer_addr = pJpegPort->u4BufVA;
                #else 
                mMDPParam.Capture.jpg_buffer_addr = pJpegPort->u4BufPA;
                #endif 
                mMDPParam.Capture.jpg_buffer_size = pJpegPort->u4BufSize * pJpegPort->u4BufCnt;
                mMDPParam.Capture.jpg_quality   = pJpegPort->u4Quality;   //39~90
                mMDPParam.Capture.jpg_b_add_soi = pJpegPort->fgIsSOI;   // 1:Add EXIF 0:none
                //
                NSCamCustom::TuningParam_CRZ_T const& crz_param = NSCamCustom::getParam_CRZ_Capture();
                mMDPParam.Capture.resz_coeff.crz_up_scale_coeff = crz_param.uUpScaleCoeff;
                mMDPParam.Capture.resz_coeff.crz_dn_scale_coeff = crz_param.uDnScaleCoeff;
                //
                MCAM_DBG("[configMDPParam]<ePortID_Jpeg> CRZ:(up,dn)=(%d,%d)", mMDPParam.Capture.resz_coeff.crz_up_scale_coeff, mMDPParam.Capture.resz_coeff.crz_dn_scale_coeff);
            }
            break;

        case ePortID_Postview:
            MCAM_DBG("[configMDPParam] Postview");
            //  Note: dynamic_cast is not supported without rtti.
            if  ( PostviewPortInfo const*const pQVPort = reinterpret_cast<PostviewPortInfo const*>(pOutPort) )
            {
                mMDPParam.Capture.b_qv_path_en = 1;
                mMDPParam.Capture.qv_path_sel =  0; //  0:auto select quick view path 1:QV_path_1 2:QV_path_2
                #if defined(MTK_M4U_SUPPORT)
                mMDPParam.Capture.qv_yuv_img_addr.y = pQVPort->u4BufVA;
                #else 
                mMDPParam.Capture.qv_yuv_img_addr.y = pQVPort->u4BufPA;
                #endif 
                mMDPParam.Capture.qv_img_size.w = pQVPort->u4ImgWidth; 
                mMDPParam.Capture.qv_img_size.h = pQVPort->u4ImgHeight;
                mMDPParam.Capture.qv_color_format = (MdpColorFormat)mapMDPColorFormat(pQVPort->eImgFmt);
                mMDPParam.Capture.qv_flip       = pQVPort->u4Flip; 
                mMDPParam.Capture.qv_rotate     = pQVPort->u4Rotation; 
                //
                NSCamCustom::TuningParam_PRZ_T const& prz_param = NSCamCustom::getParam_PRZ_QuickView();
                mMDPParam.Capture.resz_coeff.prz0_up_scale_coeff = prz_param.uUpScaleCoeff;
                mMDPParam.Capture.resz_coeff.prz0_dn_scale_coeff = prz_param.uDnScaleCoeff;
                mMDPParam.Capture.resz_coeff.prz0_ee_h_str       = prz_param.uEEHCoeff;
                mMDPParam.Capture.resz_coeff.prz0_ee_v_str       = prz_param.uEEVCoeff;
                //
                MCAM_DBG(
                    "[configMDPParam]<ePortID_Postview> PRZ0:(up,dn,ee_h,ee_v)=(%d,%d,%d,%d)"
                    , prz_param.uUpScaleCoeff, prz_param.uDnScaleCoeff, prz_param.uEEHCoeff, prz_param.uEEVCoeff
                );
            }
            break;

        case ePortID_Memory:  
        	MCAM_DBG("[configMDPParam] Memory");
            if  ( MemoryPortInfo const*const pMemPort = reinterpret_cast<MemoryPortInfo const*>(pOutPort) )
            {
                Rect_t rcDst, rcCrop;
                if(pMemPort->u4Rotation == 1 || pMemPort->u4Rotation == 3)
                {
                    rcDst.w = pMemPort->u4ImgHeight;
                    rcDst.h = pMemPort->u4ImgWidth;       
                    MCAM_DBG("Rotation = %d, switch Dst w/h \n", pMemPort->u4Rotation);
                }
                else
                {
                    rcDst.w = pMemPort->u4ImgWidth;
                    rcDst.h = pMemPort->u4ImgHeight;
                }
                calCropRect(rcSrc, rcDst, &rcCrop, pMemPort->u4zoomRatio);
                //
                mMDPParam.mode = MDP_MODE_CAP_JPG;  
                //
                mMDPParam.Capture.src_img_size.w = rcSrc.w;
                mMDPParam.Capture.src_img_size.h = rcSrc.h;
                mMDPParam.Capture.src_img_roi.x = rcCrop.x;
                mMDPParam.Capture.src_img_roi.y = rcCrop.y;
                mMDPParam.Capture.src_img_roi.w = rcCrop.w;
                mMDPParam.Capture.src_img_roi.h = rcCrop.h;

                mMDPParam.Capture.b_jpg_path_disen = 1;
                mMDPParam.Capture.b_ff_path_en = 1;
                
                #if defined(MTK_M4U_SUPPORT)
                mMDPParam.Capture.ff_yuv_img_addr.y = pMemPort->u4BufVA;
                #else 
                mMDPParam.Capture.ff_yuv_img_addr.y = pMemPort->u4BufPA;
                #endif 
                if(pMemPort->u4Rotation == 1 || pMemPort->u4Rotation == 3)
                {
                    mMDPParam.Capture.ff_img_size.w = pMemPort->u4ImgHeight;
                    mMDPParam.Capture.ff_img_size.h = pMemPort->u4ImgWidth;                 
                    MCAM_DBG("Rotation = %d, switch ff_img_size w/h \n", pMemPort->u4Rotation);
                }
                else
                {
                    mMDPParam.Capture.ff_img_size.w = pMemPort->u4ImgWidth;
                    mMDPParam.Capture.ff_img_size.h = pMemPort->u4ImgHeight;
                }

                mMDPParam.Capture.ff_color_format = (MdpColorFormat)mapMDPColorFormat(ePIXEL_FORMAT_UYVY); // MDP dump UYVY data, becase ISP set SWAP_Y=0 in the 2nd pass
                mMDPParam.Capture.ff_flip = pMemPort->u4Flip;
                mMDPParam.Capture.ff_rotate = pMemPort->u4Rotation; 

                
                NSCamCustom::TuningParam_CRZ_T const& crz_param = NSCamCustom::getParam_CRZ_Capture();
                mMDPParam.Capture.resz_coeff.crz_up_scale_coeff = crz_param.uUpScaleCoeff;
                mMDPParam.Capture.resz_coeff.crz_dn_scale_coeff = crz_param.uDnScaleCoeff;
                //
                MCAM_DBG("[configMDPParam]<ePortID_Memory> CRZ:(up,dn)=(%d,%d)", mMDPParam.Capture.resz_coeff.crz_up_scale_coeff, mMDPParam.Capture.resz_coeff.crz_dn_scale_coeff);
            
            }
            break;
        default:
            MCAM_ERR("[configMDPParam] bad output port %d with ID(%d)", i, pOutPort->ePortID);
            goto lbExit;
        }
    }

    ret = MTRUE;
lbExit:
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::
configPrvMDPPort (
    ePREVIEW_PORT_ID const portID, 
    ImgCfg_t const *config, 
    MemInfo_t const *memInfo
) 
{
    MCAM_DBG("[configMDPPort] E \n"); 
    if (NULL == mpMDPHalObj || NULL == config || NULL == memInfo) {
        MCAM_ERR ("[configMDPPort] mpMDPHalObj or config or memInfo is null \n"); 
        return -1; 
    }
    //
    MCAM_DBG("[configMDPPort] portID = %d\n", portID); 
    MCAM_DBG("[configMDPPort] w = %d, h = %d, ftm = 0x%x, rotate = %d, flip = %d\n", 
              config->imgWidth, config->imgHeight, config->imgFmt, 
              config->rotation, config->flip); 

    MCAM_DBG("[configMDPPort] mem virtAddr = 0x%x, phyAddr = 0x%x, bufCnt = %d, bufSize = 0x%x\n",
              memInfo->virtAddr, memInfo->phyAddr, memInfo->bufCnt, memInfo->bufSize); 

    mHalOutPort *pMDPOutPort = NULL; 
    switch (portID) {
        case ePREVIEW_DISP_PORT:
            mMDPParam.en_dst_port0 = 1; 
            pMDPOutPort = &mMDPParam.dst_port0; 
            break; 
        case ePREVIEW_VIDEO_PORT:
            mMDPParam.en_dst_port1 = 1; 
            pMDPOutPort = &mMDPParam.dst_port1; 
            break; 
        case ePREVIEW_FD_PORT:
            mMDPParam.en_dst_port2 = 1; 
            pMDPOutPort = &mMDPParam.dst_port2; 
            break; 
        default:
            MCAM_ERR("[configMDPPort] error Port \n"); 
            return -1; 
            break; 
    }

    pMDPOutPort->color_format = (MdpColorFormat)mapMDPColorFormat(config->imgFmt);
    pMDPOutPort->size_w = config->imgWidth; 
    pMDPOutPort->size_h = config->imgHeight; 
    pMDPOutPort->flip = config->flip; 
    pMDPOutPort->rotate = config->rotation;   

    #if defined (MTK_M4U_SUPPORT)
    pMDPOutPort->buffer_addr = memInfo->virtAddr; 
    #else 
    pMDPOutPort->buffer_addr = memInfo->phyAddr; 
    #endif 
    pMDPOutPort->buffer_count = memInfo->bufCnt; 

    MCAM_DBG("[configMDPPort] X \n"); 
    return 0; 
}

/*******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::
getIspDestResolution(
    MUINT32 &width, 
    MUINT32 &height
)
{
    MINT32 err = 0; 
    MINT32 sensorW = 0, sensorH = 0; 
    //       
    if (meSensorMode == eSENSOR_PREVIEW_MODE) {
        err = mpISPHalObj->sendCommand(ISP_CMD_GET_SENSOR_PRV_RANGE, (int)&sensorW, (int)&sensorH); 
    }
    else if (meSensorMode == eSENSOR_CAPTURE_MODE) {
        err = mpISPHalObj->sendCommand(ISP_CMD_GET_SENSOR_FULL_RANGE, (int)&sensorW, (int)&sensorH); 
    }
    else {
        MCAM_ERR("[getSensorResolution] unknown sensor mode = %d\n", meSensorMode);     
        err = -1; 
    }    
    width = sensorW; 
    height = sensorH; 
    return err; 
}

/******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::calCropRect(
    Rect_t rSrc,
    Rect_t rDst,
    Rect_t *prCrop,
    MUINT32 zoomRatio
)
{
    Rect_t rCrop;
    MUINT32 u4ZoomRatio = zoomRatio; 
    //
    if (u4ZoomRatio > 800) {
        MCAM_WARN("[calCropRegion] Zoom Raio reach max value %d" , u4ZoomRatio);
        u4ZoomRatio = 800;
    }

    MCAM_DBG("[calCropRegion] src: %d/%d, dst: %d/%d, zoom: %d \n", 
        rSrc.w, rSrc.h, rDst.w, rDst.h, zoomRatio); 

    // srcW/srcH < dstW/dstH 
    if (rSrc.w * rDst.h < rDst.w * rSrc.h) {
        rCrop.w = rSrc.w; 
        rCrop.h = rSrc.w * rDst.h / rDst.w;     
    }
    else if (rSrc.w * rDst.h > rDst.w * rSrc.h) { //srcW/srcH > dstW/dstH
        rCrop.w = rSrc.h * rDst.w / rDst.h; 
        rCrop.h = rSrc.h; 
    }
    else {
        rCrop.w = rSrc.w; 
        rCrop.h = rSrc.h; 
    }
    // 
    //
    rCrop.w =  ROUND_TO_2X(rCrop.w * 100 / zoomRatio);   
    rCrop.h =  ROUND_TO_2X(rCrop.h * 100 / zoomRatio);
    //
    rCrop.x = ROUND_TO_2X((rSrc.w - rCrop.w) / 2);
    rCrop.y = ROUND_TO_2X((rSrc.h - rCrop.h) / 2);
    //
    *prCrop = rCrop;
    MCAM_DBG("[calCropRegion] Crop region: %d, %d, %d, %d \n", rCrop.x, rCrop.y, rCrop.w, rCrop.h); 
    
    return 0;
}

/*******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::
eisGetInfoCallback(unsigned long *eis_x, unsigned long *eis_y)
{
    MINT32 eisX = 0,  eisY = 0; 
    //FIXME, it is better to use cookie instead of global object.
    if (gCameraIO75Obj != NULL) {
        gCameraIO75Obj->eisCB(&eisX, &eisY);
    }
    *eis_x = eisX; 
    *eis_y = eisY;     
    MCAM_DBG1("[eisCallback] X eisX:%ld, eisY:%ld\n", *eis_x, *eis_y); 
    return 0; 
 }

/*******************************************************************************
*
*******************************************************************************/
MINT32
CameraIO75::
eisCB(MINT32 *eisX, MINT32 *eisY)
{
    MINT32 err = 0; 
    if (mpCameraIOCB != NULL) {
        err = mpCameraIOCB(eCAMIO_CB_EIS, (MINT32 )eisX, (MINT32)eisY, mCallbackCookie); 
        // align eisX to 2x for YUV 420 
        int floatPart = 0, intPart = 0; 
        intPart = *eisX >> 8; 
        floatPart = *eisX & 0xff; 
        /*
        if (intPart & 0x1) {
            intPart++;
            floatPart = 0; 
        }*/
        *eisX = intPart << 8 | floatPart;         
    }
    else {
        *eisX = 0; 
        *eisY = 0; 
    }
    return err; 
}

/*******************************************************************************
*
*******************************************************************************/
MVOID 
CameraIO75::
setCallbacks(CALLBACKFUNC_CAMERAIO pCameraIOCB, MVOID *user)
{
    MCAM_DBG("[setCallbacks] cb:0x%x, cookie:0x%x\n", (MUINT32) pCameraIOCB, (MUINT32)user); 
    if (NULL == pCameraIOCB) {
        mpCameraIOCB = NULL; 
        mCallbackCookie = NULL; 
    }
    else {
        mpCameraIOCB = pCameraIOCB; 
        mCallbackCookie = user;
    }
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL 
CameraIO75::
setZoomCropSrc(MUINT32 const a_u4StartX, MUINT32 a_u4StartY, MUINT32 const a_u4Width, MUINT32 const a_u4Height)
{
    //
    MCAM_DBG("[setZoomCropSrc] (x, y, w, h) = (%d, %d, %d, %d)\n", a_u4StartX, 
                       a_u4StartY, a_u4Width, a_u4Height); 
    mZoomRectSrc.w = a_u4Width; 
    mZoomRectSrc.h = a_u4Height; 
    
    // 
    Rect_t rCrop; 
    rect_t rMDPCrop; 
    calCropRect(mZoomRectSrc, mZoomRectDst, &rCrop, mZoomVal);
   
    rMDPCrop.x = rCrop.x; 
    rMDPCrop.y = rCrop.y; 
    rMDPCrop.w = rCrop.w; 
    rMDPCrop.h = rCrop.h; 
    //
    mpMDPHalObj->sendCommand(CMD_SET_ZOOM_RATIO, (int) &rMDPCrop);

    return MTRUE; 
}

/*******************************************************************************
*
*******************************************************************************/
MUINT32 
CameraIO75::
mapMDPColorFormat(
    EPixelFormat const pixelFmt
)
{
    MUINT32 colorFmt = 0;
    switch (pixelFmt)
    {
        case ePIXEL_FORMAT_I420:
            colorFmt = YUV420_Planar;
            break;
        case ePIXEL_FORMAT_YV12:
            colorFmt = ANDROID_YV12;
            break;
        case ePIXEL_FORMAT_NV21:
            colorFmt = NV21; 
            break; 
        case ePIXEL_FORMAT_NV21_BLK:
            break; 
        case ePIXEL_FORMAT_NV12:
            colorFmt = NV12;
            break; 
        case ePIXEL_FORMAT_NV12_BLK:
            break; 
        case ePIXEL_FORMAT_YUY2:
            colorFmt = YUY2_Pack; 
            break; 
        case ePIXEL_FORMAT_UYVY:
            colorFmt = UYVY_Pack; 
            break; 
        case ePIXEL_FORMAT_RGB565:
            colorFmt = RGB565; 
            break; 
        case ePIXEL_FORMAT_RGB888:
            colorFmt = RGB888; 
            break; 
        case ePIXEL_FORMAT_RGBA8888:
            //  ePIXEL_FORMAT_RGBA8888 -> R:LSB, A:MSB
            //  ABGR8888 in MDP driver -> A:MSB, R:LSB
            colorFmt = ABGR8888;
            break;
        case ePIXEL_FORMAT_ARGB8888:
            //  ePIXEL_FORMAT_ARGB8888 -> A:LSB, B:MSB
            //  BGRA8888 in MDP driver -> B:MSB, A:LSB
            colorFmt = BGRA8888;
            break;
        case ePIXEL_FORMAT_Y800:
            colorFmt = Y800;
            break;
        default:
            MCAM_WARN("not MDP ColorFormat, 0x%x\n", pixelFmt); 
            break;
    }
    return colorFmt;
}  


/*******************************************************************************
* Real-time priority, DISP > VIDEO
*******************************************************************************/

MINT32 
CameraIO75::
getN3DPreviewFrame(
    ePREVIEW_PORT_ID const ePortID,
    BuffInfo_t* buffInfo
)
{
    MCAM_DBG("[getN3DPreviewFrame] port id = %d\n", ePortID); 
    MINT32 err = 0; 

#ifdef  MTK_NATIVE_3D_SUPPORT
    MUINT32 debugTimeLog = 0;
    MUINT32 debugDataLog = 0;
    char value[PROPERTY_VALUE_MAX] = {'\0'}; 
    property_get("debug.camera.n3d.time", value, "0");
    debugTimeLog = atoi(value); 
    property_get("debug.camera.n3d.data", value, "0");
    debugDataLog = atoi(value); 

    MINT32 i = 0;
    MUINT32 outBufAddr;
    halMdpBufInfo_t mdpBufInfo;
    halMdpBufInfo_t mdpBufInfoTg1;
    halMdpBufInfo_t mdpBufInfoTg2;
    ::memset(&mdpBufInfo, 0 , sizeof(halMdpBufInfo_t)); 
    ::memset(&mdpBufInfoTg1, 0 , sizeof(halMdpBufInfo_t)); 
    ::memset(&mdpBufInfoTg2, 0 , sizeof(halMdpBufInfo_t)); 

    Rect_t tg1roi, tg2roi;
    ::memset(&tg1roi, 0 , sizeof(Rect_t)); 
    ::memset(&tg2roi, 0 , sizeof(Rect_t)); 

    if (mPreviewPort & ePortID & ePREVIEW_DISP_PORT) {
        // work buffer should sync with DISP port 
        // (1) DQ tg1 + tg2
        CamTimer camTmrPass1("debugN3DTime", MHAL_CAM_GENERIC_PROFILE_OPT);
        //        
        if (mMDPParam.work_tg1.working_buffer_addr != 0) {
            err = mpMDPHalObj->dequeueBuff(N3D_PREVIEW_WORKING_BUFFER_TG1, &mdpBufInfoTg1);
            if (err != 0) {
                MCAM_ERR("[getN3DPreviewFrame] N3D_PREVIEW_WORKING_BUFFER_TG1 err:%x\n", err); 
                mpISPHalObj->dumpReg(); 
            }
        }
        //
        if (mMDPParam.work_tg2.working_buffer_addr != 0) {
            err = mpMDPHalObj->dequeueBuff(N3D_PREVIEW_WORKING_BUFFER_TG2, &mdpBufInfoTg2);
            if (err != 0) {
                MCAM_ERR("[getN3DPreviewFrame] N3D_PREVIEW_WORKING_BUFFER_TG2 err:%x\n", err); 
                mpISPHalObj->dumpReg(); 
            }
        }
        if (debugTimeLog) {
            camTmrPass1.printTime("1--Pass1");
        }

        // (2) get address
        err = mpMDPHalObj->sendCommand(CMD_GET_BUFFER_ADDRESS, N3D_PREVIEW_WORKING_BUFFER_TG1, mdpBufInfoTg1.hwIndex, (int) mpTg1Addr);
        if (err != 0) {
            MCAM_ERR("[getN3DPreviewFrame] CMD_GET_BUFFER_ADDRESS TG1 err:%x\n", err); 
        }
        err = mpMDPHalObj->sendCommand(CMD_GET_BUFFER_ADDRESS, N3D_PREVIEW_WORKING_BUFFER_TG2, mdpBufInfoTg2.hwIndex, (int) mpTg2Addr);
        if (err != 0) {
            MCAM_ERR("[getN3DPreviewFrame] CMD_GET_BUFFER_ADDRESS TG2 err:%x\n", err); 
        }
/*        MCAM_DBG("[getN3DPreviewFrame] mpTg1Addr(0x%x), mpTg2Addr(0x%x) \n", *mpTg1Addr, *mpTg2Addr); 
        MCAM_DBG("[getN3DPreviewFrame] Tg1.idx/Cnt(%d/%d), Tg2.idx/Cnt(%d/%d) \n", 
                mdpBufInfoTg1.hwIndex, mdpBufInfoTg1.fillCnt, mdpBufInfoTg2.hwIndex, mdpBufInfoTg2.fillCnt);*/

        // (3) Algorithm
        CamTimer camTmrAlgo("debugN3DTime", MHAL_CAM_GENERIC_PROFILE_OPT);

        mpN3DObj->N3DRun(*mpRunDataN3D, *mpOutDataN3D);

        //0:LR 1:RL (L:tg1, R:tg2)
        // tg1 
        tg1roi.x = mpTg1Offset->width + mTg1CropRect.x;
        tg1roi.y = mpTg1Offset->height + mTg1CropRect.y;
        // tg2
        tg2roi.x = mpTg2Offset->width + mTg2CropRect.x;
        tg2roi.y = mpTg2Offset->height + mTg2CropRect.y;
        //
        tg1roi.w = mTg1CropRect.w;
        tg1roi.h = mTg1CropRect.h;
        tg2roi.w = mTg2CropRect.w;
        tg2roi.h = mTg2CropRect.h;
        //
        if (debugTimeLog) {
            camTmrAlgo.printTime("2--Algorithm"); 
        }
        if (debugDataLog) {
            MCAM_DBG("[debugN3DData] -------------------------------------------------------");
            MCAM_DBG("mpTg1Offset(w,h)=(%d, %d); mTg1CropRect=(%d, %d, %d, %d)", 
                mpTg1Offset->width, mpTg1Offset->height, mTg1CropRect.x, mTg1CropRect.y, mTg1CropRect.w, mTg1CropRect.h);
            MCAM_DBG("mpTg2Offset(w,h)=(%d, %d); mTg2CropRect=(%d, %d, %d, %d)",
                mpTg2Offset->width, mpTg2Offset->height, mTg2CropRect.x, mTg2CropRect.y, mTg2CropRect.w, mTg2CropRect.h);
            MCAM_DBG("tg1roi = (%d, %d, %d, %d)", tg1roi.x, tg1roi.y, tg1roi.w, tg1roi.h);
            MCAM_DBG("tg2roi = (%d, %d, %d, %d)", tg2roi.x, tg2roi.y, tg2roi.w, tg2roi.h);
            MCAM_DBG("----------------------------------------------------------------------");
        }
        
        // (4) Manual Triger
        CamTimer camTmrTrigger("debugN3DTime", MHAL_CAM_GENERIC_PROFILE_OPT);
        
        err = mpMDPHalObj->sendCommand(CMD_N3D_MANUAL_TRIGGER_2ND_PASS, (int) &tg1roi, (int) &tg2roi, NULL);
        if (err != 0) {
            MCAM_ERR("[getN3DPreviewFrame] CMD_N3D_MANUAL_TRIGGER_2ND_PASS err:%x\n", err); 
        }

        // (5) DQ disp      
        err = mpMDPHalObj->dequeueBuff(N3D_OUTPUT_PORT, &mdpBufInfo); 
        if (err != 0) {
            MCAM_ERR("[getN3DPreviewFrame] N3D_OUTPUT_PORT err:%x\n", err); 
            mpISPHalObj->dumpReg();
        }
        if (debugTimeLog) {
            camTmrTrigger.printTime("3--ManualTriggerPass2");
        }

        // EQ tg1
        if (mMDPParam.work_tg1.working_buffer_addr != 0) {
            mpMDPHalObj->enqueueBuff(N3D_PREVIEW_WORKING_BUFFER_TG1); 
        }

        // EQ tg2
        if (mMDPParam.work_tg2.working_buffer_addr != 0) {
            mpMDPHalObj->enqueueBuff(N3D_PREVIEW_WORKING_BUFFER_TG2); 
        }
            
    }
    else if (mPreviewPort & ePortID & ePREVIEW_VIDEO_PORT) {
        err = mpMDPHalObj->dequeueBuff(VDOENC_PORT, &mdpBufInfo); 
        if (err != 0) {
            mpISPHalObj->dumpReg();
        }
    }
    

    err = mpMDPHalObj->sendCommand(CMD_GET_BUFFER_ADDRESS, N3D_OUTPUT_PORT, mdpBufInfo.hwIndex, (int) &outBufAddr);
    if (err != 0) {
        MCAM_ERR("[getN3DPreviewFrame] CMD_GET_BUFFER_ADDRESS  N3D_OUTPUT_PORT  err:%x\n", err); 
    }


    buffInfo->hwIndex = mdpBufInfo.hwIndex; 
    buffInfo->fillCnt = mdpBufInfo.fillCnt; 
    buffInfo->bufAddr = outBufAddr;
    buffInfo->timeStampS = mdpBufInfo.timeStamp.timeStampS[mdpBufInfo.hwIndex]; 
    buffInfo->timeStampUs = mdpBufInfo.timeStamp.timeStampUs[mdpBufInfo.hwIndex]; 

#endif
    return err; 
}

/*******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::
releaseN3DPreviewFrame (
    ePREVIEW_PORT_ID const ePortID,
    BuffInfo_t* const buffInfo
)
{
    //MCAM_DBG("[releaseN3DPreviewFrame] E \n"); 

#ifdef  MTK_NATIVE_3D_SUPPORT

    if (NULL == mpMDPHalObj) {
        MCAM_ERR("[releaseN3DPreviewFrame] Null MDP Obj \n"); 
        return -1; 
    }
    MCAM_DBG("[releaseN3DPreviewFrame] portID = %d \n", ePortID); 
    if (mPreviewPort & ePortID & ePREVIEW_DISP_PORT) {
        // already enqueue tg1 & tg2 working buffer
        mpMDPHalObj->enqueueBuff(N3D_OUTPUT_PORT); 
    }
    else if (mPreviewPort & ePortID & ePREVIEW_VIDEO_PORT) {
        mpMDPHalObj->enqueueBuff(VDOENC_PORT); 
    }
    else {
        MCAM_ERR("[releaseN3DPreviewFrame] not enable port id = %d\n", ePortID); 
    }

    //MCAM_DBG("[releaseN3DPreviewFrame] X \n"); 

#endif

    return 0; 
} 

/*******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::dropN3DPreviewFrame()
{
    //MCAM_DBG("[dropN3DPreviewFrame] E \n"); 

#ifdef  MTK_NATIVE_3D_SUPPORT

    BuffInfo_t buffInfo; 
    // Display port 
    if (mPreviewPort & ePREVIEW_DISP_PORT) {
        getN3DPreviewFrame(ePREVIEW_DISP_PORT, &buffInfo); 
        MCAM_DBG("[dropN3DPreviewFrame] disp cnt:%ld\n", buffInfo.fillCnt); 
        releaseN3DPreviewFrame(ePREVIEW_DISP_PORT, &buffInfo); 
        // flush the remain frame 
        int i4RemainBuf = buffInfo.fillCnt - 1; 
        for (int i = 0; i < i4RemainBuf; i++) {
            getN3DPreviewFrame(ePREVIEW_DISP_PORT, &buffInfo); 
            MCAM_DBG("[dropN3DPreviewFrame] disp cnt:%ld\n", buffInfo.fillCnt); 
            releaseN3DPreviewFrame(ePREVIEW_DISP_PORT, &buffInfo);             
        }
    }
    // Video port 
    if (mPreviewPort & ePREVIEW_VIDEO_PORT) {
        getN3DPreviewFrame(ePREVIEW_VIDEO_PORT, &buffInfo);
        MCAM_DBG("[dropN3DPreviewFrame] video cnt:%ld\n", buffInfo.fillCnt);         
        releaseN3DPreviewFrame(ePREVIEW_VIDEO_PORT, &buffInfo); 
        // flush the remain frame 
        int i4RemainBuf = buffInfo.fillCnt - 1; 
        for (int i = 0; i < i4RemainBuf; i++) {
            getN3DPreviewFrame(ePREVIEW_VIDEO_PORT, &buffInfo); 
            MCAM_DBG("[dropN3DPreviewFrame] video cnt:%ld\n", buffInfo.fillCnt); 
            releaseN3DPreviewFrame(ePREVIEW_VIDEO_PORT, &buffInfo);             
        }
    }
    //MCAM_DBG("[dropN3DPreviewFrame] X \n"); 
    //
#endif

    return 0;
}

/*******************************************************************************
*    // Preview Pipe connection
*    // 1. Sensor --> ISP --> MDP --> Mem 
*    // 2. MEM --> ISP --> MDP --> Mem (TODO)
*******************************************************************************/

MINT32 
CameraIO75::
setN3DPreviewPipe (
    MUINT32 eEnablePortID, 
    MemInfo_t* const pInMemInfo,                    //I: Mem Info 
    PreviewPipeCfg_t* const pPreviewPipeCfg ,       //I: Prv Pipe Config
    MemInfo_t* const pDispMemInfo,                  //O: Disp Mem Info 
    MemInfo_t* const pVideoMemInfo                  //O: Video Mem Info 
)
{
    MCAM_DBG("[setN3DPreviewPipe] E portID = %d\n", eEnablePortID); 
    //
    ::memset(&mISPParam, 0, sizeof(halISPIFParam_t)); 
    ::memset(&mMDPParam, 0, sizeof(halIDPParam_t)); 
    // connect ISP -> MDP 
    MUINT32 sensorW = 0, sensorH = 0; 
    MINT32 err = 0; 

#ifdef  MTK_NATIVE_3D_SUPPORT

    if (NULL == pInMemInfo) { //Sensor --> ISP --> MDP
        getSensorRawSize(sensorW, sensorH); 
        //
        ImgCfg_t ispImgCfg; 
        ::memset(&mISPParam, 0, sizeof(halISPIFParam_t)); 
        mISPParam.u4SrcW = sensorW;
        mISPParam.u4SrcH = sensorH;
        mISPParam.u4IsContinous = 1;
    }
    //
    mISPParam.SubsampleWidth.Enable = MFALSE;
    mISPParam.SubsampleHeight.Enable = MFALSE;
    mISPParam.u4IsBypassSensorDelay = 1;    
    //
    MUINT32 ispW = 0, ispH = 0; 
    mMDPParam.debug_preview_single_frame_enable = mISPParam.u4IsContinous ? 0 : 1;
    getIspDestResolution(ispW, ispH); 
    mMDPParam.src_size_w = ispW; 
    mMDPParam.src_size_h = ispH; 

    mMDPParam.src_tg2_size_w = ispW;
    mMDPParam.src_tg2_size_h = ispH; 

        
    // Working Buf config 
    // Working path is Sensor -> ISP -> MDP(CRZ) --> MEM.
    Rect_t rSrc, rDst, rCrop; 
    rSrc.w = ispW;
    rSrc.h = ispH;   
    rDst.w = pPreviewPipeCfg->pInputConfig->imgWidth;
    rDst.h = pPreviewPipeCfg->pInputConfig->imgHeight; 
    //
    calCropRect(rSrc, rDst, &rCrop, 100);
    mMDPParam.src_roi.x = rCrop.x; 
    mMDPParam.src_roi.y = rCrop.y; 
    mMDPParam.src_roi.w = rCrop.w; 
    mMDPParam.src_roi.h = rCrop.h;

    mMDPParam.src_tg2_roi.x = rCrop.x;
    mMDPParam.src_tg2_roi.y = rCrop.y; 
    mMDPParam.src_tg2_roi.w = rCrop.w;
    mMDPParam.src_tg2_roi.h = rCrop.h;

    //
    // resizer quality tuning
    if (eEnablePortID & ePREVIEW_DISP_PORT) {
        NSCamCustom::TuningParam_CRZ_T const& crz_param = NSCamCustom::getParam_CRZ_Preview();
        mMDPParam.prv_resz_coeff.crz_up_scale_coeff = crz_param.uUpScaleCoeff;
        mMDPParam.prv_resz_coeff.crz_dn_scale_coeff = crz_param.uDnScaleCoeff;
        //
        mMDPParam.prv_resz_coeff.prz0_up_scale_coeff = 8;
        mMDPParam.prv_resz_coeff.prz0_dn_scale_coeff = 15;
        mMDPParam.prv_resz_coeff.prz0_ee_h_str = 0;
        mMDPParam.prv_resz_coeff.prz0_ee_v_str = 0;
        //
        mpInDataN3D->eScenario = SCENARIO_IMAGE_PREVIEW;
        //
        MCAM_DBG("[setN3DPreviewPipe]<ePREVIEW_DISP_PORT> CRZ:(up,dn)=(%d,%d)", mMDPParam.prv_resz_coeff.crz_up_scale_coeff, mMDPParam.prv_resz_coeff.crz_dn_scale_coeff);
    }
    else {
        
        MCAM_ERR("[setN3DPreviewPipe] eEnablePortID error"); 
    }
    //
    // Disp Port 
    if (eEnablePortID & ePREVIEW_DISP_PORT) {
        if (NULL == pPreviewPipeCfg->pDispConfig || NULL == pDispMemInfo) {
            MCAM_ERR("[setN3DPreviewPipe] Null display port config  "); 
            return -1; 
        }
        configPrvMDPPort(ePREVIEW_DISP_PORT, pPreviewPipeCfg->pDispConfig, pDispMemInfo); 
    }
    // Video Port 
    if (eEnablePortID & ePREVIEW_VIDEO_PORT) {
        if (NULL == pPreviewPipeCfg->pVideoConfig || NULL == pVideoMemInfo) {
            MCAM_ERR("[setZSDPreviewPipe] Null video port config   \n"); 
            return -1; 
        }        
        configPrvMDPPort(ePREVIEW_VIDEO_PORT, pPreviewPipeCfg->pVideoConfig, pVideoMemInfo); 
    }
    //

    // set algorithm scenario
    if (meCameraMode == eCAMERA_MODE_PREVIEW) {
        mpInDataN3D->eScenario = SCENARIO_IMAGE_PREVIEW;
    } else if (meCameraMode == eCAMERA_MODE_VIDEO) {
        mpInDataN3D->eScenario = SCENARIO_VIDEO_RECORD;
    } else {
        MCAM_WARN("[setN3DPreviewPipe] meCameraMode(%d)", meCameraMode);
    }
    //
    // get customer sensor position
    customSensorPos_N3D_t const& custom_n3d = getSensorPosN3D();
    mMDPParam.n3d_working_buff_layout = custom_n3d.uSensorPos; //0:LR 1:RL (L:tg1, R:tg2)
    MCAM_DBG("custom_n3d sensor position (%d)", custom_n3d.uSensorPos);
    //
    if (mMDPParam.n3d_working_buff_layout == 1) {
        // 1: tg2-tg1 -- tg1 is right
        mpTg1Addr   = &mpRunDataN3D->u4BufAddrR;
        mpTg2Addr   = &mpRunDataN3D->u4BufAddrL;
        mpTg1Offset = &mpOutDataN3D->offsetR;
        mpTg2Offset = &mpOutDataN3D->offsetL;
    } else { 
        // 0: tg1-tg2 -- tg1 is left
        mpTg1Addr   = &mpRunDataN3D->u4BufAddrL;
        mpTg2Addr   = &mpRunDataN3D->u4BufAddrR;        
        mpTg1Offset = &mpOutDataN3D->offsetL;
        mpTg2Offset = &mpOutDataN3D->offsetR;
    }
    //    
    // modify workBuf W and H
    MUINT32 workBufW = ROUND_TO_2X(pPreviewPipeCfg->pInputConfig->imgWidth * N3D_FACTOR/200);
    MUINT32 workBufH = ROUND_TO_2X(pPreviewPipeCfg->pInputConfig->imgHeight * N3D_FACTOR/100);
    //
    mpInDataN3D->originalImg.width     = workBufW;
    mpInDataN3D->originalImg.height    = workBufH;
    mpInDataN3D->targetImg.width       = pPreviewPipeCfg->pInputConfig->imgWidth/2;
    mpInDataN3D->targetImg.height      = pPreviewPipeCfg->pInputConfig->imgHeight;
    mpInDataN3D->eFormat               = SOURCE_FORMAT_YUYV;
    MCAM_DBG("[setN3DPreviewPipe inDataN3D Parameters] ");
    MCAM_DBG("mpInDataN3D.eScenario(%d), mpInDataN3D.originalImg(%dx%d), mpInDataN3D.targetImg(%dx%d)", 
        mpInDataN3D->eScenario,
        mpInDataN3D->originalImg.width, mpInDataN3D->originalImg.height,
        mpInDataN3D->targetImg.width, mpInDataN3D->targetImg.height);
    //
    mpN3DObj->N3DInit(*mpInDataN3D);
    //
    mTg1CropRect.x = 0;
    mTg1CropRect.y = 0;
    mTg1CropRect.w = mpInDataN3D->targetImg.width;
    mTg1CropRect.h = mpInDataN3D->targetImg.height;
    //
    mTg2CropRect.x = 0;
    mTg2CropRect.y = 0;    
    mTg2CropRect.w = mpInDataN3D->targetImg.width;
    mTg2CropRect.h = mpInDataN3D->targetImg.height;
    //
    MCAM_DBG("[setN3DPreviewPipe] tg1_crop_size(x=%d, y=%d, w=%d, h=%d)", mTg1CropRect.x, mTg1CropRect.y, mTg1CropRect.w, mTg1CropRect.h);
    MCAM_DBG("[setN3DPreviewPipe] tg2_crop_size(x=%d, y=%d, w=%d, h=%d)", mTg2CropRect.x, mTg2CropRect.y, mTg2CropRect.w, mTg2CropRect.h); 
    //
    mMDPParam.en_n3d_preview_path = 1; //1: enable native 3d preview path

    #if defined(MTK_M4U_SUPPORT)
    mMDPWorkingBufTg1 = new mHalCamMemPool("MDPWorkBufTg1", workBufW * workBufH * 2 * MDP_WORK_BUFFER_CNT, 0); 
    mMDPParam.work_tg1.working_buffer_addr = mMDPWorkingBufTg1->getVirtAddr();
    mMDPWorkingBufTg2 = new mHalCamMemPool("MDPWorkBufTg2", workBufW * workBufH * 2 * MDP_WORK_BUFFER_CNT, 0); 
    mMDPParam.work_tg2.working_buffer_addr = mMDPWorkingBufTg2->getVirtAddr();
    #else 
    mMDPWorkingBufTg1 = new mHalCamMemPool("MDPWorkBufTg1", workBufW * workBufH * 2 * MDP_WORK_BUFFER_CNT, 0); 
    mMDPParam.work_tg1.working_buffer_addr = mMDPWorkingBufTg1->getPhyAddr(); 
    mMDPWorkingBufTg2 = new mHalCamMemPool("MDPWorkBufTg2", workBufW * workBufH * 2 * MDP_WORK_BUFFER_CNT, 0); 
    mMDPParam.work_tg2.working_buffer_addr = mMDPWorkingBufTg2->getPhyAddr(); 
    #endif 
    if ((0x0 == mMDPWorkingBufTg1->getVirtAddr()) && (0x0 == mMDPWorkingBufTg2->getVirtAddr())) {
        MCAM_ERR("[setN3DPreviewPipe] No memory for MDP working buffer\n"); 
        return -1; 
    }
    mMDPParam.work_tg1.working_buffer_count = MDP_WORK_BUFFER_CNT; 
    mMDPParam.work_tg2.working_buffer_count = MDP_WORK_BUFFER_CNT;

    mZoomRectSrc.w = workBufW; 
    mZoomRectSrc.h = workBufH; 
    mZoomRectDst.w = workBufW; 
    mZoomRectDst.h = workBufH; 
#if NEW_MDP_EIS
    // tg1
    mMDPParam.work_tg1.working_rotate = pPreviewPipeCfg->pInputConfig->rotation;
    mMDPParam.work_tg1.working_color_format = YUV422_2_Pack;
    mMDPParam.work_tg1.working_img_size.w = workBufW;
    mMDPParam.work_tg1.working_img_size.h = workBufH;
    mMDPParam.work_tg1.working_img_roi.x = 0;
    mMDPParam.work_tg1.working_img_roi.y = 0;
    mMDPParam.work_tg1.working_img_roi.w = workBufW;
    mMDPParam.work_tg1.working_img_roi.h =  workBufH;
    // tg2
    mMDPParam.work_tg2.working_rotate = pPreviewPipeCfg->pInputConfig->rotation;
    mMDPParam.work_tg2.working_color_format = YUV422_2_Pack;
    mMDPParam.work_tg2.working_img_size.w = workBufW;
    mMDPParam.work_tg2.working_img_size.h = workBufH;
    mMDPParam.work_tg2.working_img_roi.x = 0;
    mMDPParam.work_tg2.working_img_roi.y = 0;
    mMDPParam.work_tg2.working_img_roi.w = workBufW;
    mMDPParam.work_tg2.working_img_roi.h =  workBufH;
    //
    mMDPParam.en_sw_trigger = 1;   //enable EIS trigger; 
    // if the callback is null the MDP will not start EIS function, 
    // it can save the SYSRAM use 
    if (mpCameraIOCB != NULL) {
        mMDPParam.CB_EIS_INFO_GET_FUNC = eisGetInfoCallback;  
    }
#endif 
    mCurPipeEngine = ePipeISP | ePipeMDP; 
    mPreviewPort = eEnablePortID; 
    MCAM_DBG("[setN3DPreviewPipe] X \n"); 

#endif
    
    return err; 
}

/*******************************************************************************
 * Capture Pipe
 *
 *      just like preview pipe
 *
*******************************************************************************/

MBOOL 
CameraIO75::
setN3DCapturePipe (
    MUINT32 eEnablePortID, 
    MemInfo_t* const pInMemInfo,                    //I: Mem Info 
    PreviewPipeCfg_t* const pPreviewPipeCfg ,       //I: Prv Pipe Config
    MemInfo_t* const pDispMemInfo,                  //O: Disp Mem Info 
    MemInfo_t* const pVideoMemInfo                  //O: Video Mem Info 
)
{
    MCAM_DBG("[setN3DCapturePipe] E portID = %d\n", eEnablePortID); 
    //
    ::memset(&mISPParam, 0, sizeof(halISPIFParam_t)); 
    ::memset(&mMDPParam, 0, sizeof(halIDPParam_t)); 
    // connect ISP -> MDP 
    MUINT32 sensorW = 0, sensorH = 0; 
    MINT32 err = 0; 

#ifdef  MTK_NATIVE_3D_SUPPORT

    if (NULL == pInMemInfo) { //Sensor --> ISP --> MDP
        getSensorRawSize(sensorW, sensorH); 
        //
        ImgCfg_t ispImgCfg; 
        ::memset(&mISPParam, 0, sizeof(halISPIFParam_t)); 
        mISPParam.u4SrcW = sensorW;
        mISPParam.u4SrcH = sensorH;
        mISPParam.u4IsContinous = 0;

        MCAM_DBG("[setN3DCapturePipe] sensorW, sensorH = %d, %d\n", sensorW, sensorH); 
    }
    //
    mISPParam.SubsampleWidth.Enable = MFALSE;
    mISPParam.SubsampleHeight.Enable = MFALSE;
    mISPParam.u4IsBypassSensorDelay = 1;    
    //
    MUINT32 imgWidth = 0, imgHeight =0;
    MUINT32 ispW = 0, ispH = 0; 
    mMDPParam.debug_preview_single_frame_enable = 0;//mISPParam.u4IsContinous ? 0 : 1;
    getIspDestResolution(ispW, ispH); 
    mMDPParam.src_size_w = ispW; 
    mMDPParam.src_size_h = ispH; 
    mMDPParam.src_tg2_size_w = ispW;
    mMDPParam.src_tg2_size_h = ispH; 
    //
    imgWidth = pPreviewPipeCfg->pInputConfig->imgWidth;
    imgHeight = pPreviewPipeCfg->pInputConfig->imgHeight;
    //

        // Working Buf config 
        // Working path is Sensor -> ISP -> MDP(CRZ) --> MEM.
        Rect_t rSrc, rDst, rCrop; 
        rSrc.w = ispW;
        rSrc.h = ispH;   
        rDst.w = imgWidth;
        rDst.h = imgHeight;
        //
        calCropRect(rSrc, rDst, &rCrop, 100);

        mMDPParam.src_roi.x = 0; 
        mMDPParam.src_roi.y = 0; 
        mMDPParam.src_roi.w = rSrc.w; 
        mMDPParam.src_roi.h = rSrc.h;

        mMDPParam.src_tg2_roi.x = 0;
        mMDPParam.src_tg2_roi.y = 0; 
        mMDPParam.src_tg2_roi.w = rSrc.w;
        mMDPParam.src_tg2_roi.h = rSrc.h;
        //
        // resizer quality tuning
        if (eEnablePortID & ePREVIEW_DISP_PORT) {
            NSCamCustom::TuningParam_CRZ_T const& crz_param = NSCamCustom::getParam_CRZ_Preview();
            mMDPParam.prv_resz_coeff.crz_up_scale_coeff = crz_param.uUpScaleCoeff;
            mMDPParam.prv_resz_coeff.crz_dn_scale_coeff = crz_param.uDnScaleCoeff;
            //
            mMDPParam.prv_resz_coeff.prz0_up_scale_coeff = 8;
            mMDPParam.prv_resz_coeff.prz0_dn_scale_coeff = 15;
            mMDPParam.prv_resz_coeff.prz0_ee_h_str = 0;
            mMDPParam.prv_resz_coeff.prz0_ee_v_str = 0;
            //
            mpInDataN3D->eScenario = SCENARIO_IMAGE_CAPTURE;
            //
            MCAM_DBG("[setN3DCapturePipe]<ePREVIEW_DISP_PORT> CRZ:(up,dn)=(%d,%d)", mMDPParam.prv_resz_coeff.crz_up_scale_coeff, mMDPParam.prv_resz_coeff.crz_dn_scale_coeff);
        }
        else {
            MCAM_ERR("[setN3DCapturePipe] eEnablePortID error"); 
        }
        //
        // Disp Port 
        if (eEnablePortID & ePREVIEW_DISP_PORT) {
            if (NULL == pPreviewPipeCfg->pDispConfig || NULL == pDispMemInfo) {
                MCAM_ERR("[setN3DCapturePipe] Null display port config  "); 
                return -1; 
            }
            configPrvMDPPort(ePREVIEW_DISP_PORT, pPreviewPipeCfg->pDispConfig, pDispMemInfo); 
        }
        //
        // modify workBuf W and H
        // get customer sensor position
        customSensorPos_N3D_t const& custom_n3d = getSensorPosN3D();
        mMDPParam.n3d_working_buff_layout = custom_n3d.uSensorPos; //0:LR 1:RL (L:tg1, R:tg2)
    MCAM_DBG("custom_n3d sensor position = %d", custom_n3d.uSensorPos);
    //
    MUINT32 workBufW = rSrc.w;
    MUINT32 workBufH = rSrc.h;
    // Init data for N3D algorithm
        mpInDataN3D->originalImg.width  = rCrop.w;
        mpInDataN3D->originalImg.height = rCrop.h;
        mpInDataN3D->targetImg.width    = ROUND_TO_2X(rCrop.w * 100 / N3D_FACTOR);  //  xxxx
        mpInDataN3D->targetImg.height   = ROUND_TO_2X(rCrop.h * 100 / N3D_FACTOR);
        mpInDataN3D->eFormat            = SOURCE_FORMAT_YUYV;
    MCAM_DBG("[setN3DCapturePipe inDataN3D Parameters]");
    MCAM_DBG("mpInDataN3D.eScenario(%d), mpInDataN3D.originalImg(%dx%d), mpInDataN3D.targetImg(%dx%d)", 
        mpInDataN3D->eScenario, 
        mpInDataN3D->originalImg.width, mpInDataN3D->originalImg.height,
        mpInDataN3D->targetImg.width, mpInDataN3D->targetImg.height);
    //
    mpN3DObj->N3DInit(*mpInDataN3D);
    //tg1
        mTg1CropRect.x = rCrop.x;
        mTg1CropRect.y = rCrop.y;
        mTg1CropRect.w = mpInDataN3D->targetImg.width;
        mTg1CropRect.h = mpInDataN3D->targetImg.height;
    //tg2
        mTg2CropRect.x = rCrop.x;
        mTg2CropRect.y = rCrop.y;    
        mTg2CropRect.w = mpInDataN3D->targetImg.width;
        mTg2CropRect.h = mpInDataN3D->targetImg.height;
    //
    MCAM_DBG("[setN3DCapturePipe] tg1_crop_size(x=%d, y=%d, w=%d, h=%d)", mTg1CropRect.x, mTg1CropRect.y, mTg1CropRect.w, mTg1CropRect.h);
    MCAM_DBG("[setN3DCapturePipe] tg2_crop_size(x=%d, y=%d, w=%d, h=%d)", mTg2CropRect.x, mTg2CropRect.y, mTg2CropRect.w, mTg2CropRect.h); 
    //
    mMDPParam.en_n3d_preview_path = 1; //1: enable native 3d preview path
    //
    #if defined(MTK_M4U_SUPPORT)
        mMDPWorkingBufTg1 = new mHalCamMemPool("MDPWorkBufTg1", workBufW * workBufH * 2 * MDP_WORK_BUFFER_CNT, 0); 
        mMDPParam.work_tg1.working_buffer_addr = mMDPWorkingBufTg1->getVirtAddr();
        mMDPWorkingBufTg2 = new mHalCamMemPool("MDPWorkBufTg2", workBufW * workBufH * 2 * MDP_WORK_BUFFER_CNT, 0); 
        mMDPParam.work_tg2.working_buffer_addr = mMDPWorkingBufTg2->getVirtAddr();
    #else 
        mMDPWorkingBufTg1 = new mHalCamMemPool("MDPWorkBufTg1", workBufW * workBufH * 2 * MDP_WORK_BUFFER_CNT, 0); 
        mMDPParam.work_tg1.working_buffer_addr = mMDPWorkingBufTg1->getPhyAddr(); 
        mMDPWorkingBufTg2 = new mHalCamMemPool("MDPWorkBufTg2", workBufW * workBufH * 2 * MDP_WORK_BUFFER_CNT, 0); 
        mMDPParam.work_tg2.working_buffer_addr = mMDPWorkingBufTg2->getPhyAddr(); 
    #endif 
        if ((0x0 == mMDPWorkingBufTg1->getVirtAddr()) && (0x0 == mMDPWorkingBufTg2->getVirtAddr())) {
            MCAM_ERR("[setN3DCapturePipe] No memory for MDP working buffer\n"); 
            return -1; 
        }
        mMDPParam.work_tg1.working_buffer_count = MDP_WORK_BUFFER_CNT; 
        mMDPParam.work_tg2.working_buffer_count = MDP_WORK_BUFFER_CNT;
    
        mZoomRectSrc.w = workBufW; 
        mZoomRectSrc.h = workBufH; 
        mZoomRectDst.w = workBufW; 
        mZoomRectDst.h = workBufH; 
#if NEW_MDP_EIS
        // tg1
        mMDPParam.work_tg1.working_rotate = pPreviewPipeCfg->pInputConfig->rotation;
        mMDPParam.work_tg1.working_color_format = YUV422_2_Pack;
        mMDPParam.work_tg1.working_img_size.w = workBufW;
        mMDPParam.work_tg1.working_img_size.h = workBufH;
        mMDPParam.work_tg1.working_img_roi.x = 0;
        mMDPParam.work_tg1.working_img_roi.y = 0;
        mMDPParam.work_tg1.working_img_roi.w = workBufW;
        mMDPParam.work_tg1.working_img_roi.h =  workBufH;
        // tg2
        mMDPParam.work_tg2.working_rotate = pPreviewPipeCfg->pInputConfig->rotation;
        mMDPParam.work_tg2.working_color_format = YUV422_2_Pack;
        mMDPParam.work_tg2.working_img_size.w = workBufW;
        mMDPParam.work_tg2.working_img_size.h = workBufH;
        mMDPParam.work_tg2.working_img_roi.x = 0;
        mMDPParam.work_tg2.working_img_roi.y = 0;
        mMDPParam.work_tg2.working_img_roi.w = workBufW;
        mMDPParam.work_tg2.working_img_roi.h = workBufH;
        //
        mMDPParam.en_sw_trigger = 1;   //enable EIS trigger; 
        // if the callback is null the MDP will not start EIS function, 
        // it can save the SYSRAM use 
        if (mpCameraIOCB != NULL) {
            mMDPParam.CB_EIS_INFO_GET_FUNC = eisGetInfoCallback;  
        }
#endif 
        mCurPipeEngine = ePipeISP | ePipeMDP; 
        mPreviewPort = eEnablePortID; 
        MCAM_DBG("[setN3DCapturePipe] X \n"); 


#endif
        
        return err; 

}

/*******************************************************************************
*
*******************************************************************************/
MINT32 
CameraIO75::
startESD()
{
    MCAM_DBG("[startESD]mCurPipeEngine = 0x%x \n", mCurPipeEngine); 
    MINT32 err = 0;
    //
    //lock pipe
    if  ( ! lockPipe(MTRUE) )
    {
        MCAM_ERR("[start] lockPipe(1) fail");
        return  -1;
    }
    if (mCurPipeEngine & ePipeISP) {
        err = mpISPHalObj->stop();
        if (err < 0) {
            MCAM_ERR("[start] isp stop fail \n"); 
            return -1;
        }
    }

    if (mCurPipeEngine & ePipeMDP) {
        err = mpMDPHalObj->stop();
        if (err < 0) {
            MCAM_ERR("[start] mdp stop fail \n"); 
            return -1;
        }
    }
    MCAM_DBG("[startESD]Restart sensor\n"); 
    mpISPHalObj->sendCommand(ISP_HAL_CMD_SET_SENSOR_RESTART);
    
    if (mCurPipeEngine & ePipeISP) {
        err = mpISPHalObj->setConf(&mISPParam);
        if (err < 0) {
            MCAM_ERR("[start] isp setconf fail \n"); 
            return -1;
        }
    }
    if (mCurPipeEngine & ePipeMDP) {
        err = mpMDPHalObj->setConf(&mMDPParam);
        if (err < 0) {
            MCAM_ERR("[start] mdp setconf fail \n"); 
            return -1;
        }
    }
    if (mCurPipeEngine & ePipeMDP) {
        err = mpMDPHalObj->start();
        if (err < 0) {
            MCAM_ERR("[start] mdp start fail \n"); 
            return -1;
        }
    }    
    if (mCurPipeEngine & ePipeISP) {
        err = mpISPHalObj->start();
        if (err < 0) {
            MCAM_ERR("[start] isp start fail \n"); 
            return -1;
        }
    }
    return 0;
} 
