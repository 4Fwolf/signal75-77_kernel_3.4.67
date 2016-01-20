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

#include "MyFacebeauty.h"
#include <linux/cache.h>  
#include <mhal_misc.h>
#include <camera_custom_if.h>

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
InitialAlgorithm(MUINT32 srcWidth, MUINT32 srcHeight, MINT32 gBlurLevel, MINT32 FBTargetColor)
{
    MY_INFO("[InitialAlgorithm] In srcWidth %d srcHeight %d",srcWidth,srcHeight);
    MTKPipeFaceBeautyEnvInfo FaceBeautyEnvInfo;    
    MTKPipeFaceBeautyTuningPara FaceBeautyTuningInfo;
    
    ::memset(&FaceBeautyEnvInfo,0,sizeof(MTKPipeFaceBeautyEnvInfo));

    FaceBeautyEnvInfo.SrcImgDSWidth = (srcWidth>>1);
    FaceBeautyEnvInfo.SrcImgDSHeight = (srcHeight>>1);
    FaceBeautyEnvInfo.SrcImgWidth = srcWidth;
    FaceBeautyEnvInfo.SrcImgHeight = srcHeight;
    FaceBeautyEnvInfo.FDWidth = FBFDWidth;
    FaceBeautyEnvInfo.FDHeight = FBFDHeight; 
    FaceBeautyEnvInfo.WorkingBufAddr = (MUINT32)mpWorkingBuferr->getVirtAddr();
    FaceBeautyEnvInfo.WorkingBufSize = FBWorkingBufferSize;
    FaceBeautyEnvInfo.SrcImgFormat = MTKPIPEFACEBEAUTY_IMAGE_YUV422;
    
    FaceBeautyEnvInfo.pTuningPara = &FaceBeautyTuningInfo;
    FaceBeautyEnvInfo.pTuningPara->WorkMode = FBWorkMode;
    FaceBeautyEnvInfo.pTuningPara->SkinToneEn = FBSkinToneEn;
    FaceBeautyEnvInfo.pTuningPara->BlurLevel = gBlurLevel;
    FaceBeautyEnvInfo.pTuningPara->AlphaBackground = FBAlphaBackground;
    FaceBeautyEnvInfo.pTuningPara->ZoomRatio = FBZoomRatio;
    FaceBeautyEnvInfo.pTuningPara->TargetColor = FBTargetColor;
    
    mpFb->mHalFacebeautifyInit(&FaceBeautyEnvInfo); 
    MY_INFO("[InitialAlgorithm] Out");
    return  MTRUE;
}
/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
createSource(MUINT32 const u4Mode)
{
#if (FB_PROFILE_CAPTURE)
    MyDbgTimer DbgTmr("FB createSource");
#endif

    MBOOL  ret = MTRUE;
    mHalBltParam_t bltParam;

    if  ( getControlStep() )
    {
        if(meSensorType==eSensorType_RAW)
            ret = createFullFrame2Pass();
        else
            ret = createFullFrame();
        if(mShotParam.u4ZoomRatio!=100)
        {
            MY_INFO("[createSource] Do crop and resize");
            MDPZoomCrop((void*)mpSreResize->getVirtAddr(), mu4W_yuv, mu4H_yuv, (void*)mpSource->getVirtAddr(), mu4W_yuv, mu4H_yuv);
        }
        if(mShotParam.uShotFileName!=NULL)
        {
            MY_INFO("[createSource] File name %s",mShotParam.uShotFileName);
            createJpgImage(mpSource->getVirtAddr(),1);
        }
        else
            MY_INFO("[createSource] File name NULL");
    }
    else
    {
        
    }

#if (FB_PROFILE_CAPTURE)
    DbgTmr.print("FBProfiling - createSource:: Done");
#endif
    return  ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
createFullFrame2Pass()
{
    using namespace NSCamera;
    MY_DBG("[createFullFrame2Pass] in");
    MBOOL  ret = MTRUE;
    MINT32 err = 0;    
    PortInfo InPortInfo;
    PortInfo BpsPort;
    PortInfo OutPortInfo;
    InPortInfo= PortInfo(ImgInfo(ePIXEL_FORMAT_BAYER10, mu4W_raw, mu4H_raw),BufInfo(NULL, NULL, 0, 0));
    OutPortInfo= PortInfo(ImgInfo(ePIXEL_FORMAT_BAYER10, mu4W_raw, mu4H_raw),BufInfo(mpBlurImg->getVirtAddr(), mpBlurImg->getVirtAddr(), mu4SourceSize)); 
    
    halISPIFParam_t ISPParam;  
    halIDPParam_t   MDPParam;
     
    configISPParam(&ISPParam, InPortInfo, OutPortInfo);
    ret =   setIspMode(ISP_CAM_MODE_CAPTURE_PASS1)
        &&  0==(err = mpHal3A->setCaptureParam(0))
        &&  0==(err = mpIspHal->setConf(&ISPParam));
    if  ( (err = mpIspHal->start()) )
    {
        MY_ERR("[do_Isp] mpIspHal->start() - ec(%x)", err);
        ret = false;
        goto lbExit;
    }    
    if  ( (err = mpIspHal->waitDone(ISP_HAL_IRQ_EXPDONE)) )
    {
        MY_ERR("[do_Isp] mpIspHal->waitDone() - ec(%x)", err);
        ret = false;
        goto lbExit;
    }
    MY_INFO("[createFullFrame2Pass] pass one done");
    
    //char szFileName[100];
    //::sprintf(szFileName, "/mnt/sdcard2/DCIM/Camera/%s_%dx%d.raw", "BayerA", mu4W_yuv,mu4H_yuv);
    //mHalMiscDumpToFile(szFileName, (UINT8*)mpBlurImg->getVirtAddr(), mu4SourceSize);
    //MY_INFO("[createFullFrame] Save File done");
    
    InPortInfo= PortInfo(ImgInfo(ePIXEL_FORMAT_BAYER10, mu4W_raw, mu4H_raw),BufInfo(mpBlurImg->getVirtAddr(), mpBlurImg->getVirtAddr(), mu4SourceSize)); 
    BpsPort= PortInfo(ImgInfo(ePIXEL_FORMAT_YUY2, mu4W_yuv, mu4H_yuv));
    mu4W_yuv=mu4W_yuv-(mu4W_yuv%4);
    if(mShotParam.u4ZoomRatio!=100)
         OutPortInfo= PortInfo(ImgInfo(ePIXEL_FORMAT_YUY2, mu4W_yuv, mu4H_yuv),BufInfo(mpSreResize->getVirtAddr(), mpSreResize->getVirtAddr(), mu4SourceSize)); 
    else
         OutPortInfo= PortInfo(ImgInfo(ePIXEL_FORMAT_YUY2, mu4W_yuv, mu4H_yuv),BufInfo(mpSource->getVirtAddr(), mpSource->getVirtAddr(), mu4SourceSize));
      
    configISPParam(&ISPParam, InPortInfo, BpsPort);
    configMDPParam(&MDPParam,BpsPort, OutPortInfo);
    
    ret =   setIspMode(ISP_CAM_MODE_CAPTURE_PASS2)
        &&  0==(err = mpHal3A->setCaptureParam(1))
        &&  0==(err = mpMdpHal->setConf(&MDPParam))
        &&  0==(err = mpIspHal->setConf(&ISPParam))
            ;
    if  ( ! ret )
    {
        MY_ERR("[createFullFrame2Pass] Config ISP/3A - ec(%x)", err);
        ret = false;
        goto lbExit;
    }

    //  (4)
    ret = do_Isp();
    if  ( ! ret )
    {
        MY_ERR("[createFullFrame2Pass] waitDone");
        ret = false;
        goto lbExit;
    }
    MY_INFO("[createFullFrame2Pass] pass two done");
    
lbExit:
    MY_DBG("[createFullFrame2Pass] out");
    return  ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
createFullFrame()
{
    using namespace NSCamera;

    MBOOL  ret = MTRUE;
    MINT32 err = 0;

    /*
        <> <Single-Frame>
        <> Sensor In -> Bayer Raw Out + LRZ Lightmap
        <> OB/NR1/Shading: on
    */
    MY_INFO("[createFullFrame]");


    PortInfo InPortInfo;        
    PortInfo OutPortInfo;       
    //  (0) Config In/Out Ports.

    InPortInfo = PortInfo(ImgInfo(ePIXEL_FORMAT_YUY2, mu4W_yuv, mu4H_yuv));
    PortInfo BpsPort= PortInfo(ImgInfo(ePIXEL_FORMAT_YUY2, mu4W_yuv, mu4H_yuv));
    mu4W_yuv=mu4W_yuv-(mu4W_yuv%4);    
    
    //OutPortInfo format didn't ref
    if(mShotParam.u4ZoomRatio!=100)
         OutPortInfo= PortInfo(ImgInfo(ePIXEL_FORMAT_YV12, mu4W_yuv, mu4H_yuv),BufInfo(mpSreResize->getVirtAddr(), mpSreResize->getPhyAddr(), mu4SourceSize)); 
    else
         OutPortInfo= PortInfo(ImgInfo(ePIXEL_FORMAT_YV12, mu4W_yuv, mu4H_yuv),BufInfo(mpSource->getVirtAddr(), mpSource->getPhyAddr(), mu4SourceSize));
    
    //  (1) Config ISPParam
    halISPIFParam_t ISPParam;
    halIDPParam_t   MDPParam;
    configISPParam(&ISPParam, InPortInfo, BpsPort);
    configMDPParam(&MDPParam,BpsPort, OutPortInfo);
    
    //  (2) Config ISP/3A
    ret =   setIspMode(ISP_CAM_MODE_CAPTURE_FLY)
        &&  0==(err = mpHal3A->setCaptureParam(0))
        &&  0==(err = mpMdpHal->setConf(&MDPParam))
        &&  0==(err = mpIspHal->setConf(&ISPParam))
            ;
    if  ( ! ret )
    {
        MY_ERR("[createFullFrame] Config ISP/3A - ec(%x)", err);
        goto lbExit;
    }


    //  (4)
    ret = do_Isp();
    if  ( ! ret )
    {
        MY_ERR("[createFullFrame] waitDone");
        goto lbExit;
    }
    MY_INFO("[createFullFrame] ISP done");
    
    #ifdef DebugMode 
    char szFileName[100];
    ::sprintf(szFileName, "/mnt/sdcard2/DCIM/Camera/%s_%dx%d.yuv", "FB", mu4W_yuv,mu4H_yuv);
    mHalMiscDumpToFile(szFileName, (UINT8*)mpSource->getVirtAddr(), mu4SourceSize);
    MY_INFO("[createFullFrame] Save File done");
    #endif
lbExit:
    return  ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
requestBufs(MUINT32 const u4Mode)
{
    MBOOL   fgRet = MFALSE;
    
    queryIspBayerResolution(mu4W_raw, mu4H_raw);
    queryIspYuvResolution(mu4W_yuv, mu4H_yuv);
    //  (1)  
    mu4SourceSize=mu4W_yuv*mu4H_yuv*2;
    //mu4SourceAddr = reinterpret_cast<MUINT32>(::memalign(L1_CACHE_BYTES, mu4SourceSize));
    mpSource = new mHalCamMemPool("FBSourceBuf", mu4SourceSize, 0);    
    mpBlurImg = new mHalCamMemPool("FBBlurImg", mu4SourceSize , 0);  
    mpSreResize = new mHalCamMemPool("FBResizeBuf", mu4SourceSize , 0);
    mpAlphamap = new mHalCamMemPool("FBAlphamap", mu4SourceSize , 0);    
    FBWorkingBufferSize=((mu4W_yuv>>2) << 1)*((mu4H_yuv>>2) << 1)*WorkingBufferCun+WorkingBuferrsize;
    mpWorkingBuferr = new mHalCamMemPool("FBWorkingBuf", FBWorkingBufferSize , 0);
    
    fgRet = MTRUE;
lbExit:
    if  ( ! fgRet )
    {
        releaseBufs();
    }
    return  fgRet;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
releaseBufs()
{
    //  (1) mu4RawLenAddrs v.s. mu4RawLenSize

    if  (mpSource)
    {
        delete mpSource; 
        mpSource = NULL; 
    }
    if  (mpSreResize)
    {
        delete mpSreResize; 
        mpSreResize = NULL; 
    }

    if  (mpBlurImg)
    {
        delete mpBlurImg; 
        mpBlurImg = NULL; 
    }
    
    if  (mpAlphamap)
    {
        delete mpAlphamap; 
        mpAlphamap = NULL; 
    }
        
    if  (mpWorkingBuferr)
    {
        delete mpWorkingBuferr; 
        mpWorkingBuferr = NULL; 
    }
    
    if  (mShotParam.u4JPEGOrientation)
    {
       freeRotatedQuickViewBuf(mrRotatedQVBuf);    
    }
    //  (3)
    //    mu4JpgEncOutAddr = 0;

    return  MTRUE;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
setIspMode(MUINT32 const u4Mode)
{
    MINT32 err = 0;

    err = mpIspHal->sendCommand(ISP_CMD_SET_CAM_MODE, u4Mode);
    if  (err)
    {
        MY_ERR("[setIspMode] ec(%x)", err);
    }

    return  (0<=err);
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
do_Isp(MBOOL const fgDoEis /*= MFALSE*/, MUINT32 const u4Idx /*= 0*/)
{
    MINT32 err = 0;

    if  ( (err = mpMdpHal->start()) )
    {
        MY_ERR("[do_Isp] mpIspHal->start() - ec(%x)", err);
        goto lbExit;
    }
    
    if  ( (err = mpIspHal->start()) )
    {
        MY_ERR("[do_Isp] mpIspHal->start() - ec(%x)", err);
        goto lbExit;
    }
    
    if  ( (err = mpMdpHal->waitDone(0x01)) )
    {
        MY_ERR("[do_Isp] mpIspHal->waitDone() - ec(%x)", err);
        goto lbExit;
    }
    /*
    if  ( (err = mpIspHal->waitDone(ISP_HAL_IRQ_ISPDONE)) )
    {
        MY_ERR("[do_Isp] mpIspHal->waitDone() - ec(%x)", err);
        goto lbExit;
    }
    */
lbExit:
	  MY_INFO("[do_Isp] Stop ISP");
    err = mpIspHal->stop();
    err = mpMdpHal->stop();
    MY_INFO("[do_Isp] Stop MDP");
    return  (0<=err);
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
configISPParam(MVOID*const pParam, PortInfo const& rInPort, PortInfo const& rOutPort) const
{
    halISPIFParam_t*const pISPParam = reinterpret_cast<halISPIFParam_t*>(pParam);

    MBOOL   ret = MFALSE;
    //
    if  ( ! pISPParam )
    {
        MY_ERR("[configISPParam] NULL pISPParam");
        goto lbExit;
    }

    //
    ::memset(pISPParam, 0, sizeof(halISPIFParam_t));

    //  (1) Input Port.
    if  ( 0 != rInPort.u4BufSize )
    {   //  Input Image from Memory.
        pISPParam->u4IsSrcDram = 1;
        pISPParam->u4InPhyAddr = rInPort.u4BufPA;
    }
    else
    {   //  Input Image from Sensor
        pISPParam->u4IsSrcDram = 0;
        pISPParam->u4InPhyAddr = 0;
    }
    pISPParam->u4SrcW = rInPort.u4ImgWidth;
    pISPParam->u4SrcH = rInPort.u4ImgHeight;


    //  (2) Output Port.
    if  ( 0 != rOutPort.u4BufSize )
    {   //  Output to Memory.
        pISPParam->u4IsDestDram = 1;
        pISPParam->u4OutPhyAddr = rOutPort.u4BufPA; 
    }
    else
    {   //  Output to the next module.
        pISPParam->u4IsDestDram = 0;
        pISPParam->u4OutPhyAddr = 0;
    }
    
    pISPParam->u4IsDestBayer = 0;

    //  (3) Others.
    //  TODO: divide into in/out mem sizes
    pISPParam->u4MemSize =
        ( rInPort.u4BufSize*rInPort.u4BufCnt > rOutPort.u4BufSize*rOutPort.u4BufCnt )
        ? rInPort.u4BufSize*rInPort.u4BufCnt : rOutPort.u4BufSize*rOutPort.u4BufCnt;

    MY_INFO("[configISPParam] u4IsDestBayer %d u4MemSize %d u4IsSrcDram 0x%x u4InPhyAddr 0x%x u4IsDestDram 0x%x u4OutPhyAddr 0x%x", pISPParam->u4IsDestBayer, pISPParam->u4MemSize, pISPParam->u4IsSrcDram, pISPParam->u4InPhyAddr, pISPParam->u4IsDestDram, pISPParam->u4OutPhyAddr);
    MY_INFO("[configISPParam] u4SrcW %d, u4SrcH%d", pISPParam->u4SrcW , pISPParam->u4SrcH);
    ret = MTRUE;
lbExit:
    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
configMDPParam(MVOID*const pParam, PortInfo const& rInPort, PortInfo const& rOutPort) const
{
    halIDPParam_t*const pMDPParam = reinterpret_cast<halIDPParam_t*>(pParam);

    if  ( ! pMDPParam )
    {
        MY_ERR("[configMDPParam] NULL pMDPParam");
        return  MFALSE;
    }

    rect_t rcSrc, rcDst, rcCrop;
    ::memset(&rcSrc, 0, sizeof(rect_t));
    ::memset(&rcDst, 0, sizeof(rect_t));
    ::memset(&rcCrop,0, sizeof(rect_t));
    rcSrc.w = rInPort.u4ImgWidth;
    rcSrc.h = rInPort.u4ImgHeight;
    rcDst.w = rOutPort.u4ImgWidth;
    rcDst.h = rOutPort.u4ImgHeight;
    mpMdpHal->calCropRect(rcSrc, rcDst, &rcCrop, mShotParam.u4ZoomRatio);
    MY_DBG("[configMDPParam] src:(w,h)=(%d,%d), dst:(w,h)=(%d,%d) u4ZoomRatio %d", rcSrc.w, rcSrc.h, rcDst.w, rcDst.h,mShotParam.u4ZoomRatio);
    MY_DBG("[configMDPParam] crop:(x,y,w,h)=(%d,%d,%d,%d)", rcCrop.x, rcCrop.y, rcCrop.w, rcCrop.h);
 
    // For capture mode, SrcW/H is input, YuvW/H is jpeg, RgbW/H is for internal YUV
    // JPEG Config 
    ::memset(pMDPParam, 0, sizeof(halIDPParam_t));
    pMDPParam->mode = MDP_MODE_CAP_JPG;          // For capture mode
    pMDPParam->Capture.b_jpg_path_disen = 1;
    
    pMDPParam->Capture.src_img_size.w = rInPort.u4ImgWidth;
    pMDPParam->Capture.src_img_size.h = rInPort.u4ImgHeight;
    pMDPParam->Capture.src_img_roi.x = 0;
    pMDPParam->Capture.src_img_roi.y = 0;
    pMDPParam->Capture.src_img_roi.w = rInPort.u4ImgWidth;
    pMDPParam->Capture.src_img_roi.h = rInPort.u4ImgHeight;
    /*
    pMDPParam->Capture.jpg_img_size.w = rOutPort.u4ImgWidth;
    pMDPParam->Capture.jpg_img_size.h = rOutPort.u4ImgHeight;
    pMDPParam->Capture.jpg_yuv_color_format = 422;   //integer : 411, 422, 444, 400, 410 etc...
    pMDPParam->Capture.jpg_buffer_addr = rOutPort.u4BufPA;
    pMDPParam->Capture.jpg_buffer_size = rOutPort.u4BufSize * rOutPort.u4BufCnt;
    pMDPParam->Capture.jpg_quality = mShotParam.u4JpgQValue;    // 39~90
    pMDPParam->Capture.jpg_b_add_soi = 0;                       // 1:Add EXIF 0:none
    */
    // QV Config 
    pMDPParam->Capture.b_qv_path_en = 0;
    /*
    pMDPParam->Capture.qv_path_sel =  0;           //0:auto select quick view path 1:QV_path_1 2:QV_path_2
#if defined(MTK_M4U_SUPPORT)
    pMDPParam->Capture.qv_yuv_img_addr.y = mShotParam.frmQv.virtAddr;
#else
    pMDPParam->Capture.qv_yuv_img_addr.y = mShotParam.frmQv.phyAddr;
#endif
    pMDPParam->Capture.qv_img_size.w = mShotParam.frmQv.w;
    pMDPParam->Capture.qv_img_size.h = mShotParam.frmQv.h;
    pMDPParam->Capture.qv_color_format = RGB888;
    pMDPParam->Capture.qv_flip = 0;
    pMDPParam->Capture.qv_rotate = 0;
    */
    
    pMDPParam->Capture.b_ff_path_en =  1;           //0:auto select quick view path 1:QV_path_1 2:QV_path_2
#if defined(MTK_M4U_SUPPORT)
    pMDPParam->Capture.ff_yuv_img_addr.y = rOutPort.u4BufVA;
#else                  
    pMDPParam->Capture.ff_yuv_img_addr.y = rOutPort.u4BufVA;
#endif                 
    pMDPParam->Capture.ff_img_size.w = rOutPort.u4ImgWidth;
    pMDPParam->Capture.ff_img_size.h = rOutPort.u4ImgHeight;
    pMDPParam->Capture.ff_color_format = YV16_Planar;
    pMDPParam->Capture.ff_flip = 0;
    pMDPParam->Capture.ff_rotate = 0;
       
    //
    /*
    using namespace NSCamCustom;
    TuningParam_CRZ_T const& crz_param = getParam_CRZ_Capture();
    pMDPParam->Capture.resz_coeff.crz_up_scale_coeff = crz_param.uUpScaleCoeff;
    pMDPParam->Capture.resz_coeff.crz_dn_scale_coeff = crz_param.uDnScaleCoeff;
    //
    TuningParam_PRZ_T const& prz_param = getParam_PRZ_QuickView();
    pMDPParam->Capture.resz_coeff.prz0_up_scale_coeff= prz_param.uUpScaleCoeff;
    pMDPParam->Capture.resz_coeff.prz0_dn_scale_coeff= prz_param.uDnScaleCoeff;
    pMDPParam->Capture.resz_coeff.prz0_ee_h_str      = prz_param.uEEHCoeff;
    pMDPParam->Capture.resz_coeff.prz0_ee_v_str      = prz_param.uEEVCoeff;
    */
    return  MTRUE;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
MDPZoomCrop(void* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, void* desAdr, MUINT32 desWidth, MUINT32 desHeight) const
{
	  mHalBltParam_t bltParam;
	  
    MY_INFO("[MDPZoomCrop] srcAdr 0x%x srcWidth %d srcHeight %d desAdr 0x%x desWidth %d desHeight %d ",(MUINT32)srcAdr,srcWidth,srcHeight,(MUINT32)desAdr,desWidth,desHeight);
    
    rect_t rcSrc, rcDst, rcCrop;
    ::memset(&rcSrc, 0, sizeof(rect_t));
    ::memset(&rcDst, 0, sizeof(rect_t));
    ::memset(&rcCrop,0, sizeof(rect_t));
    rcSrc.w = srcWidth;
    rcSrc.h = srcHeight;
    rcDst.w = desWidth;
    rcDst.h = desHeight;
    mpMdpHal->calCropRect(rcSrc, rcDst, &rcCrop, mShotParam.u4ZoomRatio);
    MY_DBG("[MDPZoomCrop] src:(w,h)=(%d,%d), dst:(w,h)=(%d,%d) u4ZoomRatio %d", rcSrc.w, rcSrc.h, rcDst.w, rcDst.h,mShotParam.u4ZoomRatio);
    MY_DBG("[MDPZoomCrop] crop:(x,y,w,h)=(%d,%d,%d,%d)", rcCrop.x, rcCrop.y, rcCrop.w, rcCrop.h);
    bltParam.srcX = rcCrop.x;
    bltParam.srcY = rcCrop.y;
    bltParam.srcW = rcCrop.w;
    bltParam.srcWStride = rcSrc.w;
    bltParam.srcH = rcCrop.h;
    bltParam.srcHStride = rcSrc.h;
    bltParam.srcAddr = (MUINT32)srcAdr;
    bltParam.srcFormat = MHAL_FORMAT_YUV_422_PL;
    
    bltParam.dstW = rcDst.w;
    bltParam.dstH = rcDst.h;
    bltParam.dstAddr = (MUINT32)desAdr;
    bltParam.favor_flags = ITFF_USE_CRZ;
    bltParam.dstFormat = MHAL_FORMAT_YUV_422_PL;
    
    bltParam.pitch = rcDst.w;
    
    bltParam.orientation = 0; //TODO: test 5
    bltParam.doImageProcess = 0;
    mHalMdp_BitBlt((void *)&bltParam);

    return  MTRUE;
}

/*******************************************************************************
*
*******************************************************************************/
#ifdef DebugMode
int count=0;
#endif
MBOOL
Mhal_facebeauty::
MDPResize(void* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, void* desAdr, MUINT32 desWidth, MUINT32 desHeight) const
{
	  mHalBltParam_t bltParam;
	  
    MY_INFO("[MDPResize] srcAdr 0x%x srcWidth %d srcHeight %d desAdr 0x%x desWidth %d desHeight %d ",(MUINT32)srcAdr,srcWidth,srcHeight,(MUINT32)desAdr,desWidth,desHeight);
    ::memset(&bltParam, 0, sizeof(mHalBltParam_t));
    bltParam.srcX = 0;
    bltParam.srcY = 0;
    bltParam.srcW = srcWidth;
    bltParam.srcWStride = srcWidth;
    bltParam.srcH = srcHeight;
    bltParam.srcHStride = srcHeight;
    bltParam.srcAddr = (MUINT32)srcAdr;
    bltParam.srcFormat = MHAL_FORMAT_YUV_422_PL;
    
    bltParam.dstW = desWidth;
    bltParam.dstH = desHeight;
    bltParam.dstAddr = (MUINT32)desAdr;
    bltParam.favor_flags = ITFF_USE_CRZ;
    //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
    bltParam.dstFormat = MHAL_FORMAT_YUV_422_PL;

    bltParam.pitch = desWidth;
    bltParam.resz_up_scale_coeff = 1;    //0:linear interpolation 1:most blur 19:sharpest 8:recommeneded >12:undesirable
    bltParam.resz_dn_scale_coeff = 7;       //0:linear interpolation 1:most blur 19:sharpest 15:recommeneded 
    bltParam.resz_ee_h_str = 0;          //down scale only/0~15
    bltParam.resz_ee_v_str = 0;          //down scale only/0~15

    bltParam.orientation = 0; //TODO: test 5
    bltParam.doImageProcess = 0;
    mHalMdp_BitBlt((void *)&bltParam);
    
    //Sava Test  
    #ifdef DebugMode
    if(count==0)
    {
       char szFileName[100];
       ::sprintf(szFileName, "/mnt/sdcard2/DCIM/Camera/%s_%dx%d.raw", "FBResize", desWidth, desHeight);
       mHalMiscDumpToFile(szFileName, (UINT8*)desAdr, desWidth*desHeight*2);
       MY_INFO("[MDPResize] Save File done desAdr 0x%x",desAdr);
    }
    count++;
    #endif
    return  MTRUE;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
MDPRotate(void* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, void* desAdr, MUINT32 desWidth, MUINT32 desHeight) const
{
    mHalBltParam_t bltParam;
	  
    MY_INFO("[MDPRotate] srcAdr 0x%x srcWidth %d srcHeight %d desAdr 0x%x desWidth %d desHeight %d ",(MUINT32)srcAdr,srcWidth,srcHeight,(MUINT32)desAdr,desWidth,desHeight);
    ::memset(&bltParam, 0, sizeof(mHalBltParam_t));
    bltParam.srcX = 0;
    bltParam.srcY = 0;
    bltParam.srcW = srcWidth;
    bltParam.srcWStride = srcWidth;
    bltParam.srcH = srcHeight;
    bltParam.srcHStride = srcHeight;
    bltParam.srcAddr = (MUINT32)srcAdr;
    bltParam.srcFormat = MHAL_FORMAT_YUY2;
    
    bltParam.dstW = desWidth;
    bltParam.dstH = desHeight;
    bltParam.dstAddr = (MUINT32)desAdr;
    bltParam.favor_flags = ITFF_USE_CRZ;
    //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
    bltParam.dstFormat = MHAL_FORMAT_YUY2;

    bltParam.pitch = desWidth;
    bltParam.resz_up_scale_coeff = 1;    //0:linear interpolation 1:most blur 19:sharpest 8:recommeneded >12:undesirable
    bltParam.resz_dn_scale_coeff = 7;       //0:linear interpolation 1:most blur 19:sharpest 15:recommeneded 
    bltParam.resz_ee_h_str = 0;          //down scale only/0~15
    bltParam.resz_ee_v_str = 0;          //down scale only/0~15

    bltParam.orientation = mShotParam.u4JPEGOrientation; //TODO: test 5
    bltParam.doImageProcess = 0;
    Mdp_BitbltSlice((void *)&bltParam);
    
    //Sava Test  
    #ifdef DebugMode
    if(count==0)
    {
       char szFileName[100];
       ::sprintf(szFileName, "/mnt/sdcard2/DCIM/Camera/%s_%dx%d.raw", "FBRotate", desWidth, desHeight);
       mHalMiscDumpToFile(szFileName, (UINT8*)desAdr, desWidth*desHeight*2);
       MY_INFO("[MDPResize] Save File done desAdr 0x%x",desAdr);
    }
    count++;
    #endif
    return  MTRUE;
}
/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
MDPImgTrasn(void* srcAdr,MUINT32 srcWidth, MUINT32 srcHeight,void* desAdr,MHAL_BITBLT_FORMAT_ENUM srcformat, MHAL_BITBLT_FORMAT_ENUM desformat) const
{
    mHalBltParam_t bltParam;

    MY_INFO("[MDPImgTrasn] srcAdr 0x%x srcWidth %d srcHeight %d ",(MUINT32)srcAdr,srcWidth,srcHeight);
    ::memset(&bltParam, 0, sizeof(mHalBltParam_t));
    bltParam.srcX = 0;
    bltParam.srcY = 0;
    bltParam.srcW = srcWidth;
    bltParam.srcWStride = srcWidth;
    bltParam.srcH = srcHeight;
    bltParam.srcHStride = srcHeight;
    bltParam.srcAddr = (MUINT32)srcAdr;
    bltParam.srcFormat = srcformat;
    
    bltParam.dstW = srcWidth;
    bltParam.dstH = srcHeight;
    bltParam.dstAddr = (MUINT32)desAdr; 
    bltParam.favor_flags = ITFF_USE_CRZ;
    //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
    bltParam.dstFormat = desformat;

    bltParam.pitch = srcWidth;

    bltParam.resz_up_scale_coeff = 1;    //0:linear interpolation 1:most blur 19:sharpest 8:recommeneded >12:undesirable
    bltParam.resz_dn_scale_coeff = 7;       //0:linear interpolation 1:most blur 19:sharpest 15:recommeneded 
    bltParam.resz_ee_h_str = 0;          //down scale only/0~15
    bltParam.resz_ee_v_str = 0;          //down scale only/0~15
    
    bltParam.orientation = 0; //TODO: test 5
    bltParam.doImageProcess = 0;
    mHalMdp_BitBlt((void *)&bltParam);
    
    return  MTRUE;
}
   
 
MBOOL
Mhal_facebeauty::
MDPPCA(void* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, void* desAdr, MUINT32 desWidth, MUINT32 desHeight,void* FaceBeautyResultInfo) 
{
	  using namespace NSCamCustom;
	  mHalBltParam_t bltParam;
	  MBOOL   ret = MFALSE;
	  
	  MTKPipeFaceBeautyResultInfo* tmp=(MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo;
	  MY_INFO("[MDPPCA] PCA Y 0x%x S 0x%x H 0x%x \n",tmp->PCAYTable,tmp->PCASTable,tmp->PCAHTable);
	  MUINT32* ytable=(MUINT32*)tmp->PCAYTable;
	  MUINT32* stable=(MUINT32*)tmp->PCASTable;
	  MUINT32* htable=(MUINT32*)tmp->PCAHTable;
	  MY_INFO("[MDPPCA] PCA Y %d S %d H %d \n",*ytable,*stable,*htable);
	  FB_PCA_LUT_T &PCALUT = getFBPCALut();
    MY_INFO("[MDPPCA] PCALUT adr 0x%x  \n", (MUINT32)&PCALUT);
	  
    
	  for(int i=0;i<NSCamCustom::PCA_BIN_NUM;i++)
	  {	  	  
	      //PCALUT.lut[i].y_gain = *(ytable+i);
	      //PCALUT.lut[i].sat_gain = *(stable+i);
	      //PCALUT.lut[i].hue_shift = *(htable+i);	
	      PCALUT.lut[i].y_gain = 0;      
	      PCALUT.lut[i].sat_gain = 0;      
	      PCALUT.lut[i].hue_shift = *(htable+i);	      
	      
	      //MY_INFO("[MDPPCA] i %d PCA Y %d S %d H %d \n",i,PCALUT.lut[i].y_gain ,PCALUT.lut[i].sat_gain,PCALUT.lut[i].hue_shift);      
	  }	  
	  ret = setIspMode(ISP_CAM_MODE_FB_POSTPROCESS_PCA_ONLY);
	  
	  if  ( ! ret )
    {
        MY_ERR("[MDPPCA] Config ISPSet - ec(%x)", ret);
        goto lbExit;
    }
	  
    MY_INFO("[MDPPCA] srcAdr 0x%x srcWidth %d srcHeight %d desAdr 0x%x desWidth %d desHeight %d ",(MUINT32)srcAdr,srcWidth,srcHeight,(MUINT32)desAdr,desWidth,desHeight);
    ::memset(&bltParam, 0, sizeof(mHalBltParam_t));
	  	    	
    bltParam.srcX = 0;
    bltParam.srcY = 0;
    bltParam.srcW = srcWidth;
    bltParam.srcWStride = srcWidth;
    bltParam.srcH = srcHeight;
    bltParam.srcHStride = srcHeight;
    bltParam.srcAddr = (MUINT32)srcAdr;
    bltParam.srcFormat = MHAL_FORMAT_YUV_422_PL;
    
    bltParam.dstW = desWidth;
    bltParam.dstH = desHeight;
    bltParam.dstAddr = (MUINT32)desAdr;
    bltParam.favor_flags = ITFF_USE_CRZ;
    //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
    bltParam.dstFormat = MHAL_FORMAT_YUY2;

    bltParam.pitch = desWidth;

    bltParam.orientation = 0; //TODO: test 5
    bltParam.doImageProcess = 3;
    MdpPathImageProcess((void *)&bltParam);
 
    MDPImgTrasn((void*) desAdr, mu4W_yuv, mu4H_yuv,srcAdr,MHAL_FORMAT_YUY2, MHAL_FORMAT_YUV_422_PL);  
           
    //Sava Test
    #ifdef DebugMode
    char szFileName[100];
    ::sprintf(szFileName, "/mnt/sdcard2/DCIM/Camera/%s_%dx%d.raw", "FBPCA", desWidth, desHeight);
    mHalMiscDumpToFile(szFileName, (UINT8*)srcAdr, desWidth*desHeight*2);
    MY_INFO("[MDPResize] Save File done desAdr 0x%x",srcAdr);
    #endif
 lbExit:     

    return  MTRUE;
}

MBOOL
Mhal_facebeauty::
createBlurImage(void* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, void* desAdr, MUINT32 desWidth, MUINT32 desHeight, MINT8 NRTimes) 
{
    mHalBltParam_t bltParam;
    MBOOL   ret = MFALSE;
    MY_INFO("[createBlurImage] srcAdr 0x%x srcWidth %d srcHeight %d desAdr 0x%x desWidth %d desHeight %d resize shift 0x%x",(MUINT32)srcAdr,srcWidth,srcHeight,(MUINT32)desAdr,desWidth,desHeight,((MUINT32)srcAdr+(srcWidth*srcHeight*2)));
    ::memset((void*)&bltParam, 0, sizeof(mHalBltParam_t));
	  
    #if (FB_PROFILE_CAPTURE)
    MyDbgTimer DbgTmr("FB createBlurImage");
    #endif

    // ------------- first NR	----------------//
    ret = setIspMode(ISP_CAM_MODE_FB_POSTPROCESS_NR2_ONLY);
    if  ( ! ret )
    {
        MY_ERR("[createBlurImage] Config ISPSet - ec(%x)", ret);
        goto lbExit;
    }

    bltParam.srcX = 0;
    bltParam.srcY = 0;
    bltParam.srcW = srcWidth;
    bltParam.srcWStride = srcWidth;
    bltParam.srcH = srcHeight;
    bltParam.srcHStride = srcHeight;
    bltParam.srcAddr = (MUINT32)srcAdr;
    bltParam.srcFormat = MHAL_FORMAT_YUV_422_PL;
    
    bltParam.dstW = ((srcWidth>>2)%2)?((srcWidth>>2)-1):srcWidth>>2;
    bltParam.dstH = ((srcHeight>>2)%2)?((srcHeight>>2)-1):srcHeight>>2;
    bltParam.dstAddr = ((MUINT32)srcAdr+(srcWidth*srcHeight*2));
    bltParam.favor_flags = ITFF_USE_CRZ;
    //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
    bltParam.dstFormat = MHAL_FORMAT_YUY2;

    bltParam.pitch = ((srcWidth>>2)%2)?((srcWidth>>2)-1):srcWidth>>2;

    bltParam.orientation = 0; //TODO: test 5
    bltParam.doImageProcess = 3;
    MdpPathImageProcess((void *)&bltParam);
    // ------------- first NR	----------------//
    #if (FB_PROFILE_CAPTURE)
    DbgTmr.print("FBProfiling - createBlurImage:: NR2 First");
    #endif
    
    #ifdef DebugMode
    char szFileName[100]; 
    #endif
    //::sprintf(szFileName, "sdcard/DCIM/Camera/%s_%dx%d.raw", "NR1Img", srcWidth, srcHeight);
    //mHalMiscDumpToFile(szFileName, (UINT8*)(srcAdr+(srcWidth*srcHeight*2)), srcWidth*srcHeight*2);
    //MY_INFO("[createBlurImage] Save File done desAdr 0x%x",(srcAdr+(srcWidth*srcHeight*2)));
	  // ------------- second NR	----------------//
	  
	  bltParam.srcX = 0;
    bltParam.srcY = 0;
    bltParam.srcW = ((srcWidth>>2)%2)?((srcWidth>>2)-1):srcWidth>>2;
    bltParam.srcWStride = ((srcWidth>>2)%2)?((srcWidth>>2)-1):srcWidth>>2;
    bltParam.srcH = ((srcHeight>>2)%2)?((srcHeight>>2)-1):srcHeight>>2;
    bltParam.srcHStride = ((srcHeight>>2)%2)?((srcHeight>>2)-1):srcHeight>>2;
    bltParam.srcAddr = ((MUINT32)srcAdr+(srcWidth*srcHeight*2));
    bltParam.srcFormat = MHAL_FORMAT_YUY2;
    
    bltParam.dstW = ((srcWidth>>2)%2)?((srcWidth>>2)-1):srcWidth>>2;
    bltParam.dstH = ((srcHeight>>2)%2)?((srcHeight>>2)-1):srcHeight>>2;
    bltParam.dstAddr = ((MUINT32)srcAdr+(srcWidth*srcHeight*4));
    bltParam.favor_flags = ITFF_USE_CRZ;
    //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
    bltParam.dstFormat = MHAL_FORMAT_YUY2;

    bltParam.pitch = ((srcWidth>>2)%2)?((srcWidth>>2)-1):srcWidth>>2;

    bltParam.orientation = 0; //TODO: test 5
    bltParam.doImageProcess = 3;
    MdpPathImageProcess((void *)&bltParam);
    // ------------- second NR	----------------//   
    #if (FB_PROFILE_CAPTURE)
    DbgTmr.print("FBProfiling - createBlurImage:: NR2 Second");
    #endif 
    //::sprintf(szFileName, "sdcard/DCIM/Camera/%s_%dx%d.raw", "NR2Img", srcWidth, srcHeight);
    //mHalMiscDumpToFile(szFileName, (UINT8*)(srcAdr+(srcWidth*srcHeight*4)), srcWidth*srcHeight*2);
    //MY_INFO("[createBlurImage] Save File done desAdr 0x%x",(srcAdr+(srcWidth*srcHeight*4)));
    
    // ------------- third NR	----------------//
    if(NRTimes>=3)
    {
	      bltParam.srcX = 0;
        bltParam.srcY = 0;
        bltParam.srcW = ((srcWidth>>2)%2)?((srcWidth>>2)-1):srcWidth>>2;
        bltParam.srcWStride = ((srcWidth>>2)%2)?((srcWidth>>2)-1):srcWidth>>2;
        bltParam.srcH = ((srcHeight>>2)%2)?((srcHeight>>2)-1):srcHeight>>2;
        bltParam.srcHStride = ((srcHeight>>2)%2)?((srcHeight>>2)-1):srcHeight>>2;
    bltParam.srcAddr = ((MUINT32)srcAdr+(srcWidth*srcHeight*4));
        bltParam.srcFormat = MHAL_FORMAT_YUY2;
        
        bltParam.dstW = ((srcWidth>>2)%2)?((srcWidth>>2)-1):srcWidth>>2;
        bltParam.dstH = ((srcHeight>>2)%2)?((srcHeight>>2)-1):srcHeight>>2;
    bltParam.dstAddr = ((MUINT32)srcAdr+(srcWidth*srcHeight*2));
        bltParam.favor_flags = ITFF_USE_CRZ;
        //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
        bltParam.dstFormat = MHAL_FORMAT_YUY2;
        
        bltParam.pitch = ((srcWidth>>2)%2)?((srcWidth>>2)-1):srcWidth>>2;
        
        bltParam.orientation = 0; //TODO: test 5
        bltParam.doImageProcess = 3;
        MdpPathImageProcess((void *)&bltParam);
        MY_INFO("[createBlurImage] NRTimes 3 Done");
        // ------------- third NR	----------------//   
        #if (FB_PROFILE_CAPTURE)
        DbgTmr.print("FBProfiling - createBlurImage:: NR2 Third");
        #endif 
        if(NRTimes>=4)
        {
	          // ------------- forth NR	----------------//
	          
	          bltParam.srcX = 0;
            bltParam.srcY = 0;
            bltParam.srcW = ((srcWidth>>2)%2)?((srcWidth>>2)-1):srcWidth>>2;
            bltParam.srcWStride = ((srcWidth>>2)%2)?((srcWidth>>2)-1):srcWidth>>2;
            bltParam.srcH = ((srcHeight>>2)%2)?((srcHeight>>2)-1):srcHeight>>2;
            bltParam.srcHStride = ((srcHeight>>2)%2)?((srcHeight>>2)-1):srcHeight>>2;
            bltParam.srcAddr = ((MUINT32)srcAdr+(srcWidth*srcHeight*2));
            bltParam.srcFormat = MHAL_FORMAT_YUY2;
            
            bltParam.dstW = ((srcWidth>>2)%2)?((srcWidth>>2)-1):srcWidth>>2;
            bltParam.dstH = ((srcHeight>>2)%2)?((srcHeight>>2)-1):srcHeight>>2;
            bltParam.dstAddr = ((MUINT32)srcAdr+(srcWidth*srcHeight*4));
            bltParam.favor_flags = ITFF_USE_CRZ;
            //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
            bltParam.dstFormat = MHAL_FORMAT_YUY2;
            
            bltParam.pitch = ((srcWidth>>2)%2)?((srcWidth>>2)-1):srcWidth>>2;
            
            bltParam.orientation = 0; //TODO: test 5
            bltParam.doImageProcess = 3;
            MdpPathImageProcess((void *)&bltParam);
            MY_INFO("[createBlurImage] NRTimes 4 Done");
            // ------------- forth NR	----------------//   
            #if (FB_PROFILE_CAPTURE)
            DbgTmr.print("FBProfiling - createBlurImage:: NR2 forth");
            #endif 
        }
    }   	  
	  // ------------- Resize to full	----------------//
	  bltParam.srcX = 0;
    bltParam.srcY = 0;
    bltParam.srcW = ((srcWidth>>2)%2)?((srcWidth>>2)-1):srcWidth>>2;
    bltParam.srcWStride = ((srcWidth>>2)%2)?((srcWidth>>2)-1):srcWidth>>2;
    bltParam.srcH = ((srcHeight>>2)%2)?((srcHeight>>2)-1):srcHeight>>2;
    bltParam.srcHStride = ((srcHeight>>2)%2)?((srcHeight>>2)-1):srcHeight>>2;
    if(NRTimes%2)
    {
    	  bltParam.srcAddr = (MUINT32)srcAdr+(srcWidth*srcHeight*2);
    	  MY_INFO("[createBlurImage] odd NRTimes %d ", NRTimes);
    }
    else
    {
        bltParam.srcAddr = (MUINT32)srcAdr+(srcWidth*srcHeight*4);
        MY_INFO("[createBlurImage] even NRTimes %d ", NRTimes);
    }
    bltParam.srcFormat = MHAL_FORMAT_YUY2;
    
    bltParam.dstW = desWidth;
    bltParam.dstH = desHeight;
    bltParam.dstAddr = (MUINT32)desAdr;
    bltParam.favor_flags = ITFF_USE_CRZ;
    //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
    bltParam.dstFormat = MHAL_FORMAT_YUV_422_PL;

    bltParam.pitch = desWidth;

    bltParam.orientation = 0; //TODO: test 5
    bltParam.doImageProcess = 0;
    
    bltParam.resz_up_scale_coeff = 1;    //0:linear interpolation 1:most blur 19:sharpest 8:recommeneded >12:undesirable
    bltParam.resz_dn_scale_coeff = 1;       //0:linear interpolation 1:most blur 19:sharpest 15:recommeneded 
    bltParam.resz_ee_h_str = 0;          //down scale only/0~15
    bltParam.resz_ee_v_str = 0;          //down scale only/0~15
    
    mHalMdp_BitBlt((void *)&bltParam);
    
    // ------------- Resize to full	----------------//    
    #if (FB_PROFILE_CAPTURE)
    DbgTmr.print("FBProfiling - createBlurImage:: Done");
    #endif
    //Sava Test   
    #ifdef DebugMode
    ::sprintf(szFileName, "/mnt/sdcard2/DCIM/Camera/%s_%dx%d.raw", "BlurImg", desWidth, desHeight);
    mHalMiscDumpToFile(szFileName, (UINT8*)desAdr, desWidth*desHeight*2);
    MY_INFO("[createBlurImage] Save File done desAdr 0x%x",desAdr);
    #endif
 lbExit:   
    return  MTRUE;
}

MBOOL   
Mhal_facebeauty::
createAlphaMap(void* ImgSrcAddr, void* ImgDsAddr,void* ImgBlurAddr,void* FDResult,void* FDInfo,void* FaceBeautyResultInfo) const
{
	  MINT32 err = 0;
    err=mpFb->mHalCreateAlphaMap(ImgSrcAddr,ImgDsAddr,ImgBlurAddr,FDResult,FDInfo,FaceBeautyResultInfo);   
    
    //Sava test
    #ifdef DebugMode
    MTKPipeFaceBeautyResultInfo* tmp=(MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo;
    char szFileName[100];
    ::sprintf(szFileName, "/mnt/sdcard2/DCIM/Camera/%s_%dx%d.raw", "TextureMap", ((mu4W_yuv>>2) << 1), ((mu4H_yuv>>2) << 1));
    mHalMiscDumpToFile(szFileName, (UINT8*)tmp->AlphaMapDsAddr, ((mu4W_yuv>>2) << 1)*((mu4H_yuv>>2) << 1)*2);
    MY_INFO("[createAlphaMap] Save File done desAdr 0x%x",tmp->AlphaMapDsAddr);
    ::sprintf(szFileName, "/mnt/sdcard2/DCIM/Camera/%s_%dx%d.raw", "ColorMap", ((mu4W_yuv>>2) << 1), ((mu4H_yuv>>2) << 1));
    mHalMiscDumpToFile(szFileName, (UINT8*)tmp->AlphaMapColorDsAddr, ((mu4W_yuv>>2) << 1)*((mu4H_yuv>>2) << 1)*2);
    MY_INFO("[createAlphaMap] Save File done desAdr 0x%x",tmp->AlphaMapColorDsAddr);
    #endif
    
    if(err)
    {
    	 MY_ERR("[createAlphaMap] mHalCreateAlphaMap fail");
    	 return  MFALSE;
    }
    else    	 
       return  MTRUE;    
}

MBOOL   
Mhal_facebeauty::
createBlendedImage(void* tmpDsAddr,void* BlenddesAdr,void* BlendYSdesAdr,void* TextAlphaMapAddr,void* ColorAlphaMapAddr, MUINT32 AlphaWidth, MUINT32 AlphaHeight,void* AlphaMapAddr,void* FaceBeautyResultInfo) const
{
	  MINT32 err = 0;  
	  MTKPipeFaceBeautyResultInfo* tmp=(MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo;  
	  MINT32 DsCrzWidth = (tmp->AlphaMapDsCrzWidth>>1)<<1;  //for Alignment 
	  MINT32 DsCrzHeight = (tmp->AlphaMapDsCrzHeight>>1)<<1;  //for Alignment 
	  
	  //Sava test    
	  #ifdef DebugMode
    char szFileName[100];
    #endif
    
    #if (FB_PROFILE_CAPTURE)
    MyDbgTimer DbgTmr("FB createBlendedImage");
    #endif
    
	  MY_INFO("[createBlendedImage] resize w %d h %d",DsCrzWidth,DsCrzHeight);
    MDPResize(TextAlphaMapAddr, AlphaWidth, AlphaHeight, tmpDsAddr, DsCrzWidth, DsCrzHeight);
    #if (FB_PROFILE_CAPTURE)
    DbgTmr.print("FBProfiling - createBlendedImage:: TextAlphaMap Reiszer one");
    #endif
    MDPResize(tmpDsAddr, DsCrzWidth, DsCrzHeight, AlphaMapAddr, mu4W_yuv, mu4H_yuv);
    #if (FB_PROFILE_CAPTURE)
    DbgTmr.print("FBProfiling - createBlendedImage:: TextAlphaMap Reiszer two");
    #endif
    err=mpFb->mHalBLENDTEXTURE(BlenddesAdr,BlendYSdesAdr,AlphaMapAddr,FaceBeautyResultInfo);   
    MY_INFO("[createBlendedImage] mHalBLENDTEXTURE done");
    
    #if (FB_PROFILE_CAPTURE)
    DbgTmr.print("FBProfiling - createBlendedImage:: mHalBLENDTEXTURE algroithm");
    #endif
    //------------------ Sava test ----------------//
    //AlphaMap      
    #ifdef DebugMode
    ::sprintf(szFileName, "/mnt/sdcard2/DCIM/Camera/%s_%dx%d.raw", "AlphaMap", mu4W_yuv, mu4H_yuv);
    mHalMiscDumpToFile(szFileName, (UINT8*)AlphaMapAddr, (mu4W_yuv*mu4H_yuv)*2);
    MY_INFO("[createBlendedImage] Save File done desAdr 0x%x",AlphaMapAddr);
    #endif
    //------------------ Sava test ----------------//        
        
    //create AlphaColor map
    MDPResize(ColorAlphaMapAddr, AlphaWidth, AlphaHeight, tmpDsAddr, DsCrzWidth, DsCrzHeight);
    #if (FB_PROFILE_CAPTURE)
    DbgTmr.print("FBProfiling - createBlendedImage:: AlphaColor Reiszer one");
    #endif
    MDPResize(tmpDsAddr, DsCrzWidth, DsCrzHeight, AlphaMapAddr, mu4W_yuv, mu4H_yuv);
    #if (FB_PROFILE_CAPTURE)
    DbgTmr.print("FBProfiling - createBlendedImage:: AlphaColor Reiszer two");
    #endif
    
    //ColorAlphaMap
    //------------------ Sava test ----------------//       
    #ifdef DebugMode
    ::sprintf(szFileName, "/mnt/sdcard2/DCIM/Camera/%s_%dx%d.raw", "ColorAlphaMap", mu4W_yuv, mu4H_yuv);
    mHalMiscDumpToFile(szFileName, (UINT8*)AlphaMapAddr, (mu4W_yuv*mu4H_yuv)*2);
    MY_INFO("[createBlendedImage] Save File done desAdr 0x%x",AlphaMapAddr);   
    //BlandImg
    ::sprintf(szFileName, "/mnt/sdcard2/DCIM/Camera/%s_%dx%d.raw", "BlendedImage", mu4W_yuv, mu4H_yuv);
    mHalMiscDumpToFile(szFileName, (UINT8*)tmp->BlendTextureImgAddr, (mu4W_yuv*mu4H_yuv)*2);
    MY_INFO("[createBlendedImage] Save File done desAdr 0x%x",tmp->BlendTextureImgAddr);
    //BlandYSImg
    ::sprintf(szFileName, "/mnt/sdcard2/DCIM/Camera/%s_%dx%d.raw", "BlendedYSImage", mu4W_yuv, mu4H_yuv);
    mHalMiscDumpToFile(szFileName, (UINT8*)tmp->BlendTextureAndYSImgAddr, (mu4W_yuv*mu4H_yuv)*2);
    MY_INFO("[createBlendedImage] Save File done desAdr 0x%x",tmp->BlendTextureImgAddr);
    #endif
    //------------------ Sava test ----------------//
    if(err)
    {
    	 MY_ERR("[createAlphaMap] createBlendedImage fail");
    	 return  MFALSE;
    }
    else    	 
       return  MTRUE;      
}

MBOOL   
Mhal_facebeauty::
createFinalImage(void* FinaldesAddr,void* PCAImgAddr,void* ColorAlphaMapAddr,void* FaceBeautyResultInfo) const
{
	  MINT32 err = 0; 	  
	  err=mpFb->mHalBLENDCOLOR(FinaldesAddr,ColorAlphaMapAddr,PCAImgAddr,FaceBeautyResultInfo);
	  
	  //------------------ Sava test ----------------//
    //char szFileName[100];
    //MTKPipeFaceBeautyResultInfo* tmp=(MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo; 
    //::sprintf(szFileName, "/mnt/sdcard2/DCIM/Camera/%s_%dx%d.raw", "Final", mu4W_yuv, mu4H_yuv);
    //mHalMiscDumpToFile(szFileName, (UINT8*)tmp->BlendColorImgAddr, mu4W_yuv*mu4H_yuv*2);
    //MY_INFO("[createFinalImage] Save File done desAdr 0x%x",tmp->BlendColorImgAddr);
    //------------------ Sava test ----------------//
    
    if(err)
    {
    	 MY_ERR("[createAlphaMap] createFinalImage fail %d",err);
    	 return  MFALSE;
    }
    else    	 
       return  MTRUE;   
}
/*/////////////////////////////////////////////////////////////////////////////
    RDMA0->CAM->CRZ->IPP->PRZ0->RGB0
  /////////////////////////////////////////////////////////////////////////////*/

static int PqCust_ConfigPre( void* p_custdata )
{
    return 0;
}
static int PqCust_ConfigPost( void* p_custdata )
{
    return 0;
}
static int PqCust_EnablePre( void* p_custdata )
{
    return 0;
}
static int PqCust_EnablePost( void* p_custdata )
{
    return 0;
}
static int PqCust_WaitPre( void* p_custdata )
{
    return 0;
}
static int PqCust_WaitPost( void* p_custdata )
{
    return 0;
}
static int PqCust_DisablePre( void* p_custdata )
{
    return 0;
}
static int PqCust_DisablePost( void* p_custdata )
{
    return 0;
}

static int PqCust_CamConfig( void* p_custdata )
{
    return 0;
}
static int PqCust_CamEnable( void* p_custdata )
{
    IspHal* pIspDisp = (IspHal*) p_custdata;

    if( pIspDisp == NULL )
    {
        MDP_ERROR("pIspDisp is NULL\n");
        return MDP_ERROR_CODE_FAIL;
    }

    if(pIspDisp->start())
    {
        MDP_ERROR("isp_disp_run start fail");
        return MDP_ERROR_CODE_FAIL;
    }

    return 0;
}
static int PqCust_CamWait( void* p_custdata )
{
    return 0;
}
static int PqCust_CamDisable( void* p_custdata )
{
    IspHal* pIspDisp = (IspHal*) p_custdata;

    if( pIspDisp == NULL )
    {
        MDP_ERROR("pIspDisp is NULL\n");
        return MDP_ERROR_CODE_FAIL;
    }
    
    //Close isp
    pIspDisp->stop();
    
    return 0;
}
MdpColorFormat MhalColorFormatToMdp( MHAL_BITBLT_FORMAT_ENUM mhal_bitblt_color_format )
{
    switch( mhal_bitblt_color_format )
    {
        case MHAL_FORMAT_RGB_565 :
            return RGB565;
        case MHAL_FORMAT_BGR_565 :
            return BGR565;
        case MHAL_FORMAT_RGB_888 :
            return RGB888;
        case MHAL_FORMAT_BGR_888 :
            return BGR888;
        case MHAL_FORMAT_ARGB_8888 :
            return ARGB8888;
        case MHAL_FORMAT_ABGR_8888 :
            return ABGR8888;
        case MHAL_FORMAT_BGRA_8888:
            return BGRA8888;
        case MHAL_FORMAT_RGBA_8888:
            return RGBA8888;
        case MHAL_FORMAT_YUV_420 :
            return YV12_Planar; /*Original YV12*/
            //return ANDROID_YV12; /*Android YV12 for Test*/
        case MHAL_FORMAT_YUV_420_SP :
            return NV21;
        case MHAL_FORMAT_MTK_YUV :
            return YUV420_4x4BLK;
        case MHAL_FORMAT_YUY2 :
            return YUY2_Pack;
        case MHAL_FORMAT_UYVY :
            return UYVY_Pack;
        case MHAL_FORMAT_Y800:
            return Y800;
        case MHAL_FORMAT_YUV_422_PL://422 Planar,i.e YV16 Planar
            return YV16_Planar;
        case MHAL_FORMAT_ANDROID_YV12:   //Androdi YV12.YVU stride all 16 pixel align
            return ANDROID_YV12;
        case MHAL_FORMAT_IMG_YV12:       //Imagination YV12.YVU stride all 32 pixel align
            return IMG_YV12;
           
            

            
        default :
     	     MDP_ERROR("Unsupport mhal bitblt color format 0x%x" , mhal_bitblt_color_format );
             return (MdpColorFormat)-1;
        break;
    }

    return (MdpColorFormat)-1;
    
}
int 
Mhal_facebeauty::
MdpPathImageProcess(void* parameter)
{
    int ret_val = MDP_ERROR_CODE_OK;
    mHalBltParam_t* param = (mHalBltParam_t*)parameter;
    MdpPathImageTransformParameter p_parameter; 
    
    ISP_HAL_CONFIG_PQ_STRUCT pConfig;
    pConfig.InputSize.Width = param->srcWStride;
    pConfig.InputSize.Height = param->srcHStride;    
    mpIspHal->setConfPQ(&pConfig);
    
    MdpDrvInit();
    
    //------------------------------------para init -------------------------------------------//
    p_parameter.src_color_format = MhalColorFormatToMdp( (MHAL_BITBLT_FORMAT_ENUM)param->srcFormat );

    p_parameter.src_img_size.w = param->srcWStride;
    p_parameter.src_img_size.h = param->srcHStride;

    p_parameter.src_img_roi.x = param->srcX;
    p_parameter.src_img_roi.y = param->srcY;
    p_parameter.src_img_roi.w = param->srcW;
    p_parameter.src_img_roi.h = param->srcH;

    p_parameter.src_yuv_img_addr.y = param->srcAddr;
            
    /*Dest 0 Param Conversion*/
    p_parameter.dst_color_format = MhalColorFormatToMdp( (MHAL_BITBLT_FORMAT_ENUM)param->dstFormat );

    p_parameter.dst_img_size.w = param->dstW;
    p_parameter.dst_img_size.h = param->dstH;//TODO:actually UNKNOWN from mhal interface
    
    p_parameter.dst_img_roi.x = 0; //there is no roi offeset info from mhal interface, dst starting address is the start address
    p_parameter.dst_img_roi.y = 0; //of roi region. thus roi offset is always 0
    p_parameter.dst_img_roi.w = param->dstW;
    p_parameter.dst_img_roi.h = param->dstH;
    
    p_parameter.dst_yuv_img_addr.y = param->dstAddr;        
    p_parameter.dst_rotate_angle = param->orientation;
    
    //------------------------------------para init -------------------------------------------//
    
    //MDP Drv operation
    int             mdp_element_count = 5;
    MDPELEMENT_I*   mdp_element_list[5];
    
    //MDP Elements    
    RDMA0           me_rdma0;
    CRZ             me_crz;
    IPP             me_ipp;
    PRZ0            me_prz0;
    RGBROT0         me_rgbrot0;

    MdpCustTriggerHw_t  pq_cust_func_tbl;

    //PRZ_T PRZ_PARAM;

    MDP_SHOWFUNCTION();

    /*-----------------------------------------------------------------------------
        Initial Cust HW Trigger Function
      -----------------------------------------------------------------------------*/
    pq_cust_func_tbl.cb_ConfigPre   =   PqCust_ConfigPre;
    pq_cust_func_tbl.cb_ConfigPost  =   PqCust_ConfigPost;

    pq_cust_func_tbl.cb_EnablePre   =   PqCust_EnablePre;
    pq_cust_func_tbl.cb_EnablePost  =   PqCust_EnablePost;
    
    pq_cust_func_tbl.cb_WaitPre     =   PqCust_WaitPre;
    pq_cust_func_tbl.cb_WaitPost    =   PqCust_WaitPost;

    pq_cust_func_tbl.cb_DisablePre  =   PqCust_DisablePre;
    pq_cust_func_tbl.cb_DisablePost =   PqCust_DisablePost;

    pq_cust_func_tbl.mdp_module_after_cam = MID_CRZ;

    pq_cust_func_tbl.cb_CamConfig   = PqCust_CamConfig;
    pq_cust_func_tbl.cb_CamEnable   = PqCust_CamEnable;
    pq_cust_func_tbl.cb_CamWait     = PqCust_CamWait;
    pq_cust_func_tbl.cb_CamDisable  = PqCust_CamDisable;

    //memset(&me_rdma0, 0, sizeof(RDMA0));
    //memset(&me_crz, 0, sizeof(CRZ));
    //memset(&me_ipp, 0, sizeof(IPP));
    //memset(&me_prz0, 0, sizeof(PRZ0));
    //memset(&me_rgbrot0, 0, sizeof(RGBROT0));

    //CPU loading issue, 720P content performance might be not achieved if PQ is on.
    //960x540 = 518400
    if((UYVY_Pack == p_parameter.dst_color_format) && (518400 < (p_parameter.src_img_roi.w * p_parameter.src_img_roi.h)))
    {
        return MDP_ERROR_CODE_LOCK_RESOURCE_FAIL;
    }

    /*Resource List*/
    mdp_element_list[0] = (MDPELEMENT_I*)&me_rdma0;
    mdp_element_list[1] = (MDPELEMENT_I*)&me_crz;
    mdp_element_list[2] = (MDPELEMENT_I*)&me_ipp;
    if(1280 < p_parameter.dst_img_roi.w)
    {
        mdp_element_list[3] = (MDPELEMENT_I*)&me_rgbrot0;
        mdp_element_count = 4;
    }
    else
    {
        mdp_element_list[3] = (MDPELEMENT_I*)&me_prz0;
        mdp_element_list[4] = (MDPELEMENT_I*)&me_rgbrot0;
        mdp_element_count = 5;
    }

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Setup mdp elements parameter
      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /***RDMA0***//*Crop*/
    // RDMA_I
    me_rdma0.uInBufferCnt = 1;
    me_rdma0.src_img_yuv_addr[0] = p_parameter.src_yuv_img_addr;                                 
    me_rdma0.src_img_size = p_parameter.src_img_size;
    me_rdma0.src_img_roi  = p_parameter.src_img_roi;
    me_rdma0.src_color_format = p_parameter.src_color_format;
    me_rdma0.bHWTrigger = 0;
    me_rdma0.bContinuous = 0;
    me_rdma0.bCamIn = 0;
    me_rdma0.bEISEn = 0;
    me_rdma0.u4EISCON = 0;//EIS setting

    // RDMA0
    me_rdma0.to_cam = 1;
    me_rdma0.to_ovl = 0;
    me_rdma0.to_mout = 0;
    me_rdma0.trigger_src = 0;
   
    // CRZ
    me_crz.src_sel = 2; // 0:RDMA0 , 1:BRZ, 2:Camera
    me_crz.src_img_size.w = me_rdma0.src_img_roi.w;
    me_crz.src_img_size.h = me_rdma0.src_img_roi.h;
    me_crz.src_img_roi.x = 0;
    me_crz.src_img_roi.y = 0;
    me_crz.src_img_roi.w = me_crz.src_img_size.w;
    me_crz.src_img_roi.h = me_crz.src_img_size.h;
    
    me_crz.dst_img_size.w = p_parameter.dst_img_roi.w;
    me_crz.dst_img_size.h = p_parameter.dst_img_roi.h;

    // IPP    
    me_ipp.bBypass = 1;
    me_ipp.src_sel = 1; //0 : OVL, 1:CRZ

    //PRZ0 max input dimension : 1280
    if(1280 < p_parameter.dst_img_roi.w)
    {
        me_ipp.to_prz0 = 0;
        me_ipp.to_rgb_rot0 = 1;
    }
    else
    {
        me_ipp.to_prz0 = 1;
        me_ipp.to_rgb_rot0 = 0;
    }

    /***PRZ0***//*Rescale*/
    //RESZ_I
    me_prz0.src_img_size.w = me_crz.dst_img_size.w;
    me_prz0.src_img_size.h = me_crz.dst_img_size.h;
    me_prz0.src_img_roi.x = 0;
    me_prz0.src_img_roi.y = 0;
    me_prz0.src_img_roi.w = me_prz0.src_img_size.w;
    me_prz0.src_img_roi.h = me_prz0.src_img_size.h;
    
    me_prz0.dst_img_size.w = me_prz0.src_img_size.w; /*Currently dst roi is not used*/
    me_prz0.dst_img_size.h = me_prz0.src_img_size.h;
    
    me_prz0.uUpScaleCoeff = 8;
    me_prz0.uDnScaleCoeff = 15;
    
    me_prz0.bContinuous = 0;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
    me_prz0.bCamIn = 0;
    me_prz0.bBypass = 0;

    //PRZ0
    me_prz0.src_sel = 1;//0-MOUT,1-IPP_MOUT,2-CAM,3-BRZ_MOUT

    /*output sel*/
    me_prz0.to_vdo_rot0 = 0;
    me_prz0.to_rgb_rot0 = 1;
    me_prz0.to_vrz0= 0;
    
    me_prz0.bCamIn= 0;

    /***RGBROT0***/
    //ROTDMA_I
    me_rgbrot0.dst_img_yuv_addr[0] = p_parameter.dst_yuv_img_addr;

    me_rgbrot0.src_img_roi.x = 0;    /*For ROTDMA, src img rect =src roi rect, we don't use roi functionality*/
    me_rgbrot0.src_img_roi.y = 0;
    me_rgbrot0.src_img_roi.w = me_prz0.dst_img_size.w;
    me_rgbrot0.src_img_roi.h = me_prz0.dst_img_size.h;
    
    me_rgbrot0.dst_img_size = p_parameter.dst_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
    me_rgbrot0.dst_color_format = p_parameter.dst_color_format;/*YUY2_Pack*/
    
    me_rgbrot0.uOutBufferCnt = 1;
    me_rgbrot0.bContinuous = 0;// 1:Continuous mode does not allow time sharing.
    me_rgbrot0.bFlip = p_parameter.dst_rotate_angle >> 2;
    me_rgbrot0.bRotate = (0x3 & p_parameter.dst_rotate_angle);//0:0, 1:90, 2:180, 3:270
#if defined MDP_DRIVER_DVT_SUPPORT
    me_rgbrot0.bDithering = p_parameter.dst_dither_en;
#else
    me_rgbrot0.bDithering = 0;
#endif
    me_rgbrot0.bCamIn = 0;// 1: real time path, ex : camera input
    me_rgbrot0.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable

    //RGBROT0
    if(1280 < p_parameter.dst_img_roi.w)
    {
        me_rgbrot0.src_sel= 1;    //0-PRZ0_MOUT ,1-IPP_MOUT
    }
    else
    {
        me_rgbrot0.src_sel= 0;    //0-PRZ0_MOUT ,1-IPP_MOUT
    }

    //Init isp tuning if


    //PRZ_PARAM = pISPTuning->getPRZParam();

    /*coefficient table for resizing.1:most blur 19:sharpest 0:linear interpolation rather than cubic*/
    //    me_prz0.uEEHCoeff = PRZ_PARAM.uEEHCoeff;
    //    me_prz0.uEEVCoeff = PRZ_PARAM.uEEVCoeff;
    me_prz0.uEEHCoeff = 0;
    me_prz0.uEEVCoeff = 0;
    me_prz0.uUpScaleCoeff = 0;
    me_prz0.uDnScaleCoeff = 0;


    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Trigger HW
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    
    ret_val = _MdpPathTriggerHw( mdp_element_list , mdp_element_count ,
                                 &pq_cust_func_tbl, (void*)mpIspHal );


    return ret_val;    
}

