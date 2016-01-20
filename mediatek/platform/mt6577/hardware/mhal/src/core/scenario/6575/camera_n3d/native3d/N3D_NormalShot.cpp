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
#define LOG_TAG "N3D_NormalShot"
//
#include <utils/Errors.h>
#include <utils/threads.h>
//
#include <mhal/inc/camera.h>
//
#include <cam_types.h>
#include <cam_port.h>
#include <ICameraIO.h>
#include "shot_log.h"
#include "shot_profile.h"
#include "IShot.h"
#include "Shotbase.h"
#include "NormalShot.h"
#include <mhal_misc.h>
#include <jpeg_hal.h>
#include <isp_hal.h>
#include <mdp_hal.h>
#include <aaa_hal_base.h>
#include <camera_custom_if.h>
//
#include "N3D_NormalShot.h"
//-----------------------------------------------------------------------------
#define LOG_MSG(fmt, arg...)    XLOGD(""fmt, ##arg)
#define LOG_WRN(fmt, arg...)    XLOGW("Warning(%5d):"fmt, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    XLOGE("Err(%5d):"fmt, __LINE__, ##arg)
//-----------------------------------------------------------------------------
#define MHAL_CAM_THUMB_ADDR_OFFSET              (64 * 1024)
#define MHAL_CAM_JPEG_ADDR_OFFSET               (128 * 1024)
//-----------------------------------------------------------------------------
#define CAPTURE_BUFFER_CNT              1
//-----------------------------------------------------------------------------


N3D_NormalShot*
N3D_NormalShot::
createInstance(
    ESensorType_t const eSensorType,
    EDeviceId_t const   eDeviceId)
{
    using namespace NSCamera;
    //
    LOG_MSG("[createInstance]");
    //
    switch  (eSensorType)
    {
        case eSensorType_RAW:
        case eSensorType_YUV:
        {
            return new N3D_NormalShot("N3D_NormalShot", eSensorType, eDeviceId);
        }
        default:
        {
            LOG_ERR("[createInstance]unsupported sensor type:%d", eSensorType);
            break;
        }
    }
    //
    return  NULL;
}
//-----------------------------------------------------------------------------
MVOID
N3D_NormalShot::
destroyInstance(void)
{
    LOG_MSG("[destroyInstance]");
    delete  this;
}
//-----------------------------------------------------------------------------
N3D_NormalShot::
N3D_NormalShot(
    char const*const    szShotName,
    ESensorType_t const eSensorType,
    EDeviceId_t const   eDeviceId)
    : NormalShotN3D(szShotName, eSensorType, eDeviceId)
{
    LOG_MSG("[N3D_NormalShot]");
}
//-----------------------------------------------------------------------------
MBOOL
N3D_NormalShot::
init(
    mHalCamN3D* const   pMhalCam, 
    ShotParam const&    rShotParam,
    Hal3ABase*const     pHal3A,
    ICameraIO*const     pCameraIO,
    N3D_FILE_TYPE_ENUM   FileType)
{
    LOG_MSG("[init]");
    //
    if(!NormalShotN3D::init(rShotParam, pHal3A, pCameraIO))
    {
        return  MFALSE;
    }
    //
    mpMhalCam = pMhalCam;
    mFileType = FileType;
    //
    setConfigCapture();
    //
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
N3D_NormalShot::
setConfigCapture()
{
    MINT32 err = MHAL_NO_ERROR;
    ICameraIO::ImgCfg_t rInCfg; 
    ICameraIO::ImgCfg_t *pDispCfg = NULL, *pVdoCfg = NULL; 
    ICameraIO::MemInfo_t *pDispMemInfo = NULL, *pVdoMemInfo = NULL; 
    MUINT32 previewPort = 0; 
#ifdef  MTK_NATIVE_3D_SUPPORT
    MCAM_DBG("[setConfigCapture] E \n"); 

    CamTimer camTmr("setConfigCapture", MHAL_CAM_GENERIC_PROFILE_OPT);    

    // frmJpg.w should be divided by 2
    rInCfg.imgWidth = mShotParam.frmJpg.w/2;  // xxxx
    rInCfg.imgHeight = mShotParam.frmJpg.h;
    rInCfg.rotation = 0;
    
    //
    // Capture port config, 
    MCAM_DBG("Capture Port Init\n");
    pDispCfg = new ICameraIO::ImgCfg_t(); 
    pDispMemInfo = new ICameraIO::MemInfo_t();
    err = configCapPort(&mShotParam, pDispCfg, pDispMemInfo);     
    MCAM_ASSERT(err >= 0, "configCapPort err \n"); 
    previewPort |= ICameraIO::ePREVIEW_DISP_PORT; 
    //
    // 
    ICameraIO::PreviewPipeCfg_t* pPreviewPipeCfg  = new ICameraIO::PreviewPipeCfg_t(); 
    pPreviewPipeCfg->pInputConfig = &rInCfg; 
    pPreviewPipeCfg->pDispConfig = pDispCfg; 
    pPreviewPipeCfg->pVideoConfig = pVdoCfg; 
    //
    err = mpCameraIOObj->setSensorMode(ICameraIO::eSENSOR_PREVIEW_MODE);
    //
    err = mpCameraIOObj->setN3DCapturePipe(
                                    previewPort,         //enable port 
                                    NULL,                //image is from mem ?
                                    pPreviewPipeCfg,     //preview pipe config 
                                    pDispMemInfo,        //display out 
                                    pVdoMemInfo          //video out 
                                    ); 
    delete pPreviewPipeCfg; 
    //
    //
    if (pDispCfg) {
        delete pDispCfg; 
        delete pDispMemInfo; 
    }
    //
    if (pVdoCfg) {
        delete pVdoCfg; 
        delete pVdoMemInfo; 
    }
    camTmr.printTime(); 
    MCAM_DBG("[setConfigCapture] X \n"); 
#endif
    return err;

}

/******************************************************************************
*
*******************************************************************************/
MINT32 
N3D_NormalShot::
configCapPort(
    ShotParam* pShotParam, 
    ICameraIO::ImgCfg_t* pCapCfg, 
    ICameraIO::MemInfo_t* pCapMemCfg)
{
    MUINT32 upScaleCoef = 0;          //up scale Coefficient 
    MUINT32 downScaleCoef = 0;        //down scale coeffcient 

    MINT32 capW = pShotParam->frmJpg.w;
    MINT32 capH = pShotParam->frmJpg.h;

    // allocate buffer for capture frame
    MINT32 capBuf = capW * capH * CAPTURE_BUFFER_CNT * 3 / 2; // 1.5 --> I420
    MCAM_DBG("capBuf = %d", capBuf);

    mCapWorkingBuf = new mHalCamMemPool("CapWorkBuf", capBuf, 0); 
    if (mCapWorkingBuf == NULL){
        MCAM_DBG("[mHalCamCaptureInit] No memory for Capture buffer\n");
        return -1;
    }

    //
    #if (MHAL_CAM_ISP_TUNING)
    NSCamCustom::TuningParam_CRZ_T crz;
    crz = NSCamCustom::getParam_CRZ_Preview();
    upScaleCoef = crz.uUpScaleCoeff;
    downScaleCoef = crz.uDnScaleCoeff;
    #endif 
    // 
    pCapCfg->rotation = 0; 
#if 0
    //Marked in MT6575 for MDP limitation
    if (pmhalCamParam->u4CamMode == MHAL_CAM_MODE_VT) {
        // VT Mode, rotate the buffer to 0 degree 
        if ((mHalCamBase::mHalCamSensorInfo[mCameraId].orientation == 90) ||
            (mHalCamBase::mHalCamSensorInfo[mCameraId].orientation == 270)) {
            SWAP(pmhalCamParam->frmYuv.w, pmhalCamParam->frmYuv.h); 
            mmHalCamParam.frmYuv.w = pmhalCamParam->frmYuv.w;
            mmHalCamParam.frmYuv.h = pmhalCamParam->frmYuv.h;
            pDispCfg->rotation = 90; 
        }
    }
#endif
    //  //workaround
    pCapCfg->imgFmt = NSCamera::ePIXEL_FORMAT_I420;
    pCapCfg->imgWidth = capW; //pmhalCamParam->frmJpg.w; // xxxx 
    pCapCfg->imgHeight = capH; //pmhalCamParam->frmJpg.h; 
    pCapCfg->flip = 0; 
    pCapCfg->upScaleCoef = upScaleCoef; 
    pCapCfg->downScaleCoef = downScaleCoef; 
    //
    pCapMemCfg->virtAddr = mCapWorkingBuf->getVirtAddr(); //pmhalCamParam->frmJpg.virtAddr; 
    pCapMemCfg->phyAddr = mCapWorkingBuf->getPhyAddr(); //pmhalCamParam->frmJpg.phyAddr; 
    pCapMemCfg->bufCnt = CAPTURE_BUFFER_CNT; //pmhalCamParam->frmJpg.frmCount; 
    pCapMemCfg->bufSize = capBuf; //pmhalCamParam->frmJpg.w * pmhalCamParam->frmJpg.h * 3 / 2; 

    MCAM_DBG("pCapCfg->imgFmt = 0x%x", pCapCfg->imgFmt);

    return 0; 
}


//-----------------------------------------------------------------------------
MBOOL
N3D_NormalShot::
handleCaptureDone(MINT32 const i4ErrCode /*= 0*/)
{
    LOG_MSG("[handleCaptureDone] i4ErrCode(%d)", i4ErrCode);

    MBOOL ret = MTRUE;

#if 0   //  FIXME TODO
    if (i4ErrCode < 0) {
        //return error code to upper layer
        //mHalCamCbHandle(MHAL_CAM_CB_CAP_ERR, &i4ErrCode); 
    }
#endif

    // Process thumbnail
    MUINT32 u4ThumbSize = 0;
    if  ( ! makeQuickViewThumbnail(u4ThumbSize) )
    {
        LOG_ERR("[handleCaptureDone] makeQuickViewThumbnail failed");
    }

    // Raw callback, for quickview callback 
    // QV Image put in mHalCamParam.qv 
    invokeCallback(MHAL_CAM_CB_RAW, NULL);

    // Postview callback, donothing 
    invokeCallback(MHAL_CAM_CB_POSTVIEW, NULL);

    // Flatten Jpeg Picture Buffer for callback (including Exif)
    MUINT32 u4JpgPictureSize = 0;
    ret = flattenJpgPicBuf(getJpgEncDoneSize(), u4ThumbSize, u4JpgPictureSize);
    //
    switch(mFileType)
    {
        case N3D_FILE_TYPE_MPO:
        {
            break;
        }
        case N3D_FILE_TYPE_JPS:
        {
            #if 1
            LOG_MSG("[handleCaptureDone]uShotFileName(%s)", mShotParam.uShotFileName);
            ::mHalMiscDumpToFile(
                (char*)(mShotParam.uShotFileName),
                (MUINT8*)(mShotParam.frmJpg.virtAddr),
                u4JpgPictureSize);
            #endif
            break;
        }
        default:
        {
            LOG_ERR("[handleCaptureDone]Unknown mFileType(%d)",mFileType);
            break;
        }
    }
    //  Save file if needed.
    if  (mu4ShotDumpOPT)
    {
        ::mHalMiscDumpToFile((char*)"//sdcard//1.jpg", (MUINT8*)(mShotParam.frmJpg.virtAddr), u4JpgPictureSize);
    }

    // Jpeg callback, for JPEG bitstream callback to AP
    // Note: buffer may be freed during this callback.
    //
    LOG_MSG("[handleCaptureDone]uShotFileName(%s)", mShotParam.uShotFileName);
    LOG_MSG("[handleCaptureDone]u4JpgPictureSize(%d)",u4JpgPictureSize);
    invokeCallback(MHAL_CAM_CB_JPEG, &u4JpgPictureSize, u4JpgPictureSize);
    //
    LOG_MSG("[handleCaptureDone]End");
    //
    return  ret;
}
//-----------------------------------------------------------------------------
MBOOL
N3D_NormalShot::
capture(void)
{
    ShotProfileN3D profile("capture", getShotName());

    ICameraIO::BuffInfo_t rDispBufInfo;
    MINT32 err = 0;
    MBOOL  ret = MFALSE;
    LOG_MSG("[capture] E"); 

    //
#ifdef  MTK_NATIVE_3D_SUPPORT
    //
    //  (1) Invoke Shutter Callback
    invokeShutterCB();

    //
    err = mpCameraIOObj->start();
    if (err < 0) {
        LOG_ERR("[capture] mpCameraIOObj->start err (0x%x)", err); 
        goto lbExit; 
    }
    err = mpCameraIOObj->getN3DPreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo);
    LOG_MSG("[capture][getN3DPreviewFrame]  DISP hwIndex: %d, fillCnt: %d, timeS = %d, timeUs = %d\n",
                           rDispBufInfo.hwIndex, rDispBufInfo.fillCnt, rDispBufInfo.timeStampS, rDispBufInfo.timeStampUs);
    if (err < 0) {
        LOG_ERR("[capture] mpCameraIOObj->getN3DPreviewFrame  err (0x%x)", err); 
        goto lbExit; 
    }

    LOG_MSG("[capture] rDispBufInfo.bufAddr = 0x%x", rDispBufInfo.bufAddr); 
    LOG_MSG("[capture] frmJpg.virtAdd(0x%x), frmJpg.frmSize(%d), frmJpg.bufSize(%d)", 
            mShotParam.frmJpg.virtAddr, mShotParam.frmJpg.frmSize, mShotParam.frmJpg.bufSize);
    LOG_MSG("[capture] frmQv.frmFormat(0x%x), frmQv.frmSize(%d), frmQv.bufSize(%d)",
            (NSCamera::EPixelFormat)mShotParam.frmQv.frmFormat, mShotParam.frmQv.frmSize, mShotParam.frmQv.bufSize); 

    //::mHalMiscDumpToFile((char*) "/sdcard/DCIM/Camera3D/frm.raw", (UINT8*)mCapWorkingBuf->getVirtAddr(), mShotParam.frmYuv.frmSize); 

    err = mpCameraIOObj->stop(); 
    if (err < 0) {
        LOG_ERR("[capture] mpCameraIOObj->stop err (0x%x)", err); 
        goto lbExit; 
    }
    //  

    createJpgImage(mCapWorkingBuf->getVirtAddr(), YUV420_Planar, MTRUE);
    //
#endif


    ret = MTRUE;
lbExit:

    //  (5) Wait Shutter Callback Done.
    if  ( ! waitShutterCBDone() )
    {
        LOG_ERR("[capture] waitShutterCBDone() fail");
        ret = MFALSE;
    }

    //  (6) Force to handle done even if any error has happened before.
    if  ( ! handleCaptureDone() )
    {
        LOG_ERR("[capture] handleCaptureDone() fail");
        ret = MFALSE;
    }

    if  ( ! ret )
    {
        LOG_ERR("[capture] fail");
    }

    profile.print();

    LOG_MSG("[capture] X"); 
    return  ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
N3D_NormalShot::
createJpgImage(MUINT32 u4SrcAddr, MdpColorFormat emInputFormat, MBOOL fgShowQuickView)
{
    LOG_MSG("[createJpgImage] - E. u4SrcAddr: 0x%08X. emInputFormat: %d. fgShowQuickView: %d.", u4SrcAddr, emInputFormat, fgShowQuickView);

    using namespace NSCamera;

#if (HDR_PROFILE_CAPTURE)
    MyDbgTimer DbgTmr("createJpgImage");
#endif


    MBOOL	ret = MFALSE;
    MINT32	err = -1;

    //	(1) Config ISPParam / MDPParam
    BufInfo const bufInfo = getBufInfo_JpgEnc();
    LOG_MSG("[createJpgImage] u4BufVA 0x%x w %d h %d.",bufInfo.u4BufPA,mShotParam.frmJpg.w,mShotParam.frmJpg.h);
    halIDPParam_t pMDPParam;
    MdpYuvAddr jpgbuff;

    // For capture mode, SrcW/H is input, YuvW/H is jpeg, RgbW/H is for internal YUV
    // JPEG Config 
    ::memset((void*)&pMDPParam, 0, sizeof(halIDPParam_t));
    ::memset((void*)&jpgbuff, 0, sizeof(MdpYuvAddr));
	
    rect_t rcSrc, rcDst, rcCrop;
    ::memset(&rcSrc, 0, sizeof(rect_t));
    ::memset(&rcDst, 0, sizeof(rect_t));
    ::memset(&rcCrop,0, sizeof(rect_t));
    rcSrc.w = mShotParam.frmJpg.w;
    rcSrc.h = mShotParam.frmJpg.h;
    rcDst.w = mShotParam.frmJpg.w;
    rcDst.h = mShotParam.frmJpg.h;
    mpMdpHal->calCropRect(rcSrc, rcDst, &rcCrop, mShotParam.u4ZoomRatio);
    LOG_MSG("[createJpgImage] src:(w,h)=(%d,%d), dst:(w,h)=(%d,%d)", rcSrc.w, rcSrc.h, rcDst.w, rcDst.h);
    LOG_MSG("[createJpgImage] crop:(x,y,w,h)=(%d,%d,%d,%d)", rcCrop.x, rcCrop.y, rcCrop.w, rcCrop.h);

    pMDPParam.mode  = MDP_MODE_CAP_JPG;          // For capture mode
    jpgbuff.y       = u4SrcAddr;

    pMDPParam.Capture.pseudo_src_enable         = 1;
    pMDPParam.Capture.pseudo_src_yuv_img_addr   = jpgbuff;
    pMDPParam.Capture.pseudo_src_color_format   = emInputFormat;  // ===> N3D final result format (YUV420_Planar).
    pMDPParam.Capture.src_img_size.w            = mShotParam.frmJpg.w;
    pMDPParam.Capture.src_img_size.h            = mShotParam.frmJpg.h;
    pMDPParam.Capture.src_img_roi.x             = rcCrop.x;
    pMDPParam.Capture.src_img_roi.y             = rcCrop.y;
    pMDPParam.Capture.src_img_roi.w             = rcCrop.w;
    pMDPParam.Capture.src_img_roi.h             = rcCrop.h;

    pMDPParam.Capture.b_jpg_path_disen = 0;
    if (pMDPParam.Capture.b_jpg_path_disen == 0)
    {
        pMDPParam.Capture.jpg_img_size.w        = mShotParam.frmJpg.w;
        pMDPParam.Capture.jpg_img_size.h        = mShotParam.frmJpg.h;
        pMDPParam.Capture.jpg_yuv_color_format  = 422;	//integer : 411, 422, 444, 400, 410 etc...
        pMDPParam.Capture.jpg_buffer_addr       = bufInfo.u4BufPA;
        pMDPParam.Capture.jpg_buffer_size       = mShotParam.frmJpg.w * mShotParam.frmJpg.h * 2;
        pMDPParam.Capture.jpg_quality           = mShotParam.u4JpgQValue;   // 39~90
        pMDPParam.Capture.jpg_b_add_soi         = 0;                        // 1:Add EXIF 0:none
    }

   	pMDPParam.Capture.b_qv_path_en = fgShowQuickView;	// 0: disable QuickView. 1: enable QuickView.
//    pMDPParam.Capture.b_qv_path_en = 1;	// 0: disable QuickView. 1: enable QuickView.
	if (pMDPParam.Capture.b_qv_path_en == 1)
    {
        pMDPParam.Capture.qv_path_sel       =  0;  //0:auto select quick view path 1:QV_path_1 2:QV_path_2
        pMDPParam.Capture.qv_yuv_img_addr.y = mShotParam.frmQv.virtAddr;
        pMDPParam.Capture.qv_img_size.w     = mShotParam.frmQv.w;
        pMDPParam.Capture.qv_img_size.h     = mShotParam.frmQv.h;
        pMDPParam.Capture.qv_color_format   = YUV420_Planar;
        pMDPParam.Capture.qv_flip           = 0;
        pMDPParam.Capture.qv_rotate         = 0;
    }

    pMDPParam.Capture.b_ff_path_en =  0;

    //	(2) Apply settings.
    ret = 0==(err = mpMdpHal->setConf(&pMDPParam));
    if	(!ret)
    {
        LOG_ERR("[createJpgImage] mpMdpHal->setConf fail. ec(%x).", err);
        goto lbExit;
    }

    //	(3) Start
    ret = 0==(err = mpMdpHal->start());
    if	(!ret)
    {
        LOG_ERR("[createJpgImage] mpMdpHal->Start - ec(%x).", err);
        goto lbExit;
    }


    //	(4) Wait Done
    ret = waitJpgCaptureDone();
    if (!ret)
    {
        LOG_ERR("[createJpgImage] waitJpgCaptureDone fail.");
        goto lbExit;
    }



lbExit:

    //  (5) Stop & Reset.
    mpMdpHal->stop();
    ret = MTRUE;

#if (HDR_PROFILE_CAPTURE)
    DbgTmr.print();
#endif


    LOG_MSG("[createJpgImage] - X. ret: %d.", ret);

    return	ret;

}

