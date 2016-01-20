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
#include "mhal_misc.h"
#include "jpeg_hal.h"
#include "../3dlib/CamJpg.h"
#include <camera_custom_if.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <semaphore.h>
#include <linux/rtpm_prio.h>

static camera_face_mtk FBFaceInfo[15];
static camera_face_metadata_mtk  FBmetadata;
static mtk_face_info MTKFaceInfo[15];
#ifdef BanchMark
    MUINT32 capturecount = 0;
#endif
#define SmallFaceWidthThreshold 40
#define BigFaceWidthThreshold 60
Mhal_facebeauty*     mpFbObj;

//Thread
static MVOID* FBCapture(void *arg); 
static MVOID* FBSaveFile(void *arg);
static pthread_t threadFB; 
static pthread_t threadSaveFile;
static sem_t semFBthread; 

MBOOL  WaitSaveFileDone();

/*******************************************************************************
*
*******************************************************************************/
IMhal_facebeauty*
IMhal_facebeauty::
createInstance(ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
{
    using namespace NSCamera;
    switch  (eSensorType)
    {
    case eSensorType_RAW:
        {
        static Mhal_facebeauty  instMhal_facebeauty(eSensorType, eDeviceId);
        static IMhal_facebeauty instIMhal_facebeauty(&instMhal_facebeauty);
        return &instIMhal_facebeauty;
        }
        break;
    case eSensorType_YUV:
    {
        static Mhal_facebeauty  instMhal_facebeauty(eSensorType, eDeviceId);
        static IMhal_facebeauty instIMhal_facebeauty(&instMhal_facebeauty);
        return &instIMhal_facebeauty;
    }
    default:
        MY_WARN("[createInstance] YUV is unsupported!");
        break;
    }
    return  NULL;
}


/*******************************************************************************
*
*******************************************************************************/
MVOID
IMhal_facebeauty::
destroyInstance()
{
}


/*******************************************************************************
*
*******************************************************************************/
IMhal_facebeauty::
IMhal_facebeauty(ShotBase*const pShot)
    : IShot()
    , mrShotBase(*pShot)
{
}

    
/*******************************************************************************
*
*******************************************************************************/
MBOOL
IMhal_facebeauty::
init(ShotParam const& rShotParam, Hal3ABase*const pHal3A, MUINT32 const rMetadata, MUINT32 const rFaceInfo)
{  
	  //::memcpy(&FBmetadata, (void*)rMetadata, sizeof(camera_face_metadata_mtk));
	  FBmetadata.faces=(camera_face_m *)FBFaceInfo;
	  camera_face_metadata_mtk *tmp	=(camera_face_metadata_mtk *)rMetadata; 
	  mtk_face_info* mtktmp = (mtk_face_info *)rFaceInfo;
	  FBmetadata.number_of_faces=tmp->number_of_faces;                 
	  memcpy( FBmetadata.faces,tmp->faces,sizeof(camera_face_mtk)*15); 
	  MY_INFO("[facebeauty init] FBFaceInfo adr 0x%x FBFaceInfo num %d",(MUINT32)FBmetadata.faces,FBmetadata.number_of_faces);
	  for(int i=0;i<FBmetadata.number_of_faces;i++)	
	  {     	  	  
	      FBmetadata.faces[i].rect[0] = ((FBmetadata.faces[i].rect[0] + 1000) * FBFDWidth) / 2000;	
	      FBmetadata.faces[i].rect[1] = ((FBmetadata.faces[i].rect[1] + 1000) * FBFDHeight) / 2000;	
	      FBmetadata.faces[i].rect[2] = ((FBmetadata.faces[i].rect[2] + 1000) * FBFDWidth) / 2000;	
	      FBmetadata.faces[i].rect[3] = ((FBmetadata.faces[i].rect[3] + 1000) * FBFDHeight) / 2000;		      	
	      MTKFaceInfo[i].rop_dir = mtktmp[i].rop_dir;
	      MTKFaceInfo[i].rip_dir = mtktmp[i].rip_dir;     	
	      MY_INFO("[facebeauty init] After FBFaceInfo num %d left %d top %d right %d button %d pose %d",i,FBmetadata.faces[i].rect[0],FBmetadata.faces[i].rect[1],FBmetadata.faces[i].rect[2],FBmetadata.faces[i].rect[3],MTKFaceInfo[i].rip_dir);	
    }
    return  mrShotBase.init(rShotParam, pHal3A);
}

/*******************************************************************************
*
*******************************************************************************/
MVOID
Mhal_facebeauty::
destroyInstance()
{
}


/*******************************************************************************
*
*******************************************************************************/
Mhal_facebeauty::
Mhal_facebeauty(ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
    : ShotBase("FBShot", eSensorType, eDeviceId)
    //
    , mu4W_raw(0)
    , mu4H_raw(0)
    , mu4W_yuv(0)
    , mu4H_yuv(0)
    , meSensorType(eSensorType)
    //
{

}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
init(ShotParam const& rShotParam, Hal3ABase*const pHal3A)
{
    MBOOL   ret = MFALSE;
    MINT32  ec = 0;


    ret = ShotBase::init(rShotParam, pHal3A);
    if  ( ! ret )
    {
        goto lbExit;
    }

    mpFb = halFACEBEAUTIFYBase::createInstance(HAL_FACEBEAUTY_OBJ_SW);
    mpFbObj = this;
    if  ( ! mpFb )
    {
        MY_ERR("[init] NULL mpFb");
        goto lbExit;
    }      
    
    ret = MTRUE;
lbExit:
    if  ( ! ret )
    {
        uninit();
    }
    MY_DBG("[init] rc(%d)", ret);
    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
uninit()
{
    MBOOL ret = MTRUE;

    if  (mpFb)
    {
        mpFb->destroyInstance();
        mpFb = NULL;
    }

    mu4W_raw = mu4H_raw = mu4W_yuv = mu4H_yuv = 0;

    ret = ShotBase::uninit();

    MY_DBG("[uninit] rc(%d)", ret);
    return  ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
capture()
{
#if (FB_PROFILE_CAPTURE)
    MyDbgTimer DbgTmr("FB capture");
#endif
    mu4Step=1;
    MBOOL   ret = MFALSE;
    
    sem_init(&semFBthread, 0, 0);
    pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_OTHER, RTPM_PRIO_CAMERA_COMPRESS};
    pthread_create(&threadFB, &attr, FBCapture, NULL);  
    
    sem_wait(&semFBthread);
        
    ret =
        
        //  ()  Shutter Callback
        invokeShutterCB()
        #if (FB_PROFILE_CAPTURE)
        &&	DbgTmr.print("FBProfiling - capture:: invokeShutterCB")
        #endif
            //{ B } Jpeg 
        &&  createJpgImage((MUINT32)msFaceBeautyResultInfo.BlendColorImgAddr,0)
        #if (FB_PROFILE_CAPTURE)
        &&	DbgTmr.print("FBProfiling - capture:: createJpgImage")
        #endif
            //  reConfigISP
        &&  reConfigISP()
        #if (FB_PROFILE_CAPTURE)
        &&	DbgTmr.print("FBProfiling - capture:: reConfigISP")
        #endif
            //  ()  Wait Shutter Callback Done
        &&  waitShutterCBDone()
            ;
    #if (FB_PROFILE_CAPTURE)
	  DbgTmr.print("FBProfiling - capture:: waitShutterCBDone");
	  #endif
	  //------------------ Sava test ----------------//
	  #ifdef BanchMark
	  char szFileName[100];    
	  MUINT32 FDInfo[100]={0};
	  MUINT32* htable=(MUINT32*)msFaceBeautyResultInfo.PCAHTable; 
	  int i=0;  
	  for(i=0;i<FBmetadata.number_of_faces;i++)
	  {
	  	 FDInfo[i*4]   = FBmetadata.faces[i].rect[0];
	  	 FDInfo[i*4+1] = FBmetadata.faces[i].rect[1];
	  	 FDInfo[i*4+2] = FBmetadata.faces[i].rect[2]-FBmetadata.faces[i].rect[0];
	  	 FDInfo[i*4+3] = MTKFaceInfo[i].rip_dir;
	  	 MY_INFO("[FACEINFO] x %d y %d w %d",FBmetadata.faces[i].rect[0],FBmetadata.faces[i].rect[1],FBmetadata.faces[i].rect[2]);
	  }
    ::sprintf(szFileName, "sdcard/DCIM/Camera/%s_H_%d_%d.txt", "FDinfo", *htable,capturecount);
    mHalMiscDumpToFile(szFileName, (MUINT8*)&FDInfo, 100 * 4);
    MY_INFO("[FACEINFO] Save File done");
    #endif
    //------------------ Sava test ----------------//
         
    if  ( ! ret )
    {
        goto lbExit;
    }

    ret = MTRUE;
lbExit:
    releaseBufs();
    pthread_join(threadFB, NULL);
    
    //  Force to handle done even if there is any error before.
    handleCaptureDone();
    #if (FB_PROFILE_CAPTURE)
    DbgTmr.print("FBProfiling - capture:: handleCaptureDone");
    #endif
    
#if (FB_PROFILE_CAPTURE)
    DbgTmr.print("FBProfiling:: Done");
#endif
    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
createJpgImage(MUINT32 srcadr,MBOOL dumpOrg)
{    
    #if (FB_PROFILE_CAPTURE)
    MyDbgTimer DbgTmr("FB Jpg Encode");
    #endif
    
    MBOOL   ret = MFALSE;
    MINT32  err = -1;
    
    int u4Orientation = mShotParam.u4JPEGOrientation;
    MY_DBG("[createJpgImage] u4Orientation %d \n",u4Orientation);
    BufInfo bufInfo; 
    if(u4Orientation)
    {
        mrRotatedQVBuf.u4BufSize = mShotParam.frmQv.w*mShotParam.frmQv.h * 3 / 2; //for I420
        allocateRotatedQuickViewBuf(mrRotatedQVBuf);
    }
    bufInfo.u4BufVA = mShotParam.frmQv.virtAddr;   
    bufInfo.u4BufPA = mShotParam.frmQv.phyAddr; 
    MY_DBG("[MDPJPGENC] bufInfo VA 0x%x bufInfo PA 0x%x\n",bufInfo.u4BufVA,bufInfo.u4BufPA); 
    MDPJPGENC(srcadr,bufInfo,MHAL_FORMAT_YUV_422_PL,0);
    #if (FB_PROFILE_CAPTURE)
    DbgTmr.print("FBProfiling:: Source encode");
    #endif
    if(u4Orientation)
    {
        MDPImgTrasn((void*)srcadr,mu4W_yuv, mu4H_yuv,(void*)mpSreResize->getVirtAddr(),MHAL_FORMAT_YUV_422_PL, MHAL_FORMAT_YUY2); // using mpSreResize for temp format transfer
        #if (FB_PROFILE_CAPTURE)
        DbgTmr.print("FBProfiling:: Source format chane");
        #endif
        if(u4Orientation==2)          
            MDPRotate((void*)mpSreResize->getVirtAddr(), mu4W_yuv, mu4H_yuv,(void*)mpBlurImg->getVirtAddr(), mu4W_yuv, mu4H_yuv); // using mpBlurImg for temp source rotate
        else
            MDPRotate((void*)mpSreResize->getVirtAddr(), mu4W_yuv, mu4H_yuv,(void*)mpBlurImg->getVirtAddr(), mu4H_yuv, mu4W_yuv); // using mpBlurImg for temp source rotate
        #if (FB_PROFILE_CAPTURE)
        DbgTmr.print("FBProfiling:: rotate");
        #endif
        bufInfo = getBufInfo_quickViewEnc();    
        MDPJPGENC(mpBlurImg->getVirtAddr(),bufInfo,MHAL_FORMAT_YUY2,u4Orientation);
        #if (FB_PROFILE_CAPTURE)
        DbgTmr.print("FBProfiling:: rotate endocde");
        #endif
    }
    if(dumpOrg)
    {
    	  MUINT32 u4JpgPictureSize = 0;
    	  #ifdef BanchMark
    	      char szFileName[100];   	      
    	      ShotBase::flattenJpgPicBuf(getJpgEncDoneSize(), 0, u4JpgPictureSize);
              ::sprintf(szFileName, "%s_%d.jpg", "sdcard/DCIM/Camera/FBOrignal", capturecount);   
              capturecount++;	
    	      mHalMiscDumpToFile(szFileName, (UINT8*)mShotParam.frmJpg.virtAddr, u4JpgPictureSize);
    	  #else  
    	      ShotBase::flattenJpgPicBuf(getJpgEncDoneSize(), 0, u4JpgPictureSize);  	  
            //mHalMiscDumpToFile((char*)mShotParam.uShotFileName, (UINT8*)mShotParam.frmJpg.virtAddr, u4JpgPictureSize);
              ShotBase::invokeCallback(MHAL_CAM_CB_FB_ORI_DATA, (MUINT8*)(mShotParam.frmJpg.virtAddr), u4JpgPictureSize);
        #endif
    }
    #if (FB_PROFILE_CAPTURE)
        DbgTmr.print("FBProfiling:: endocde done");
    #endif
lbExit:
    
    ret = MTRUE;
    return  ret;
}
/*******************************************************************************
* JPG encode 
*******************************************************************************/
MBOOL
Mhal_facebeauty::
MDPJPGENC(MUINT32 srcadr,BufInfo bufInfo,MHAL_BITBLT_FORMAT_ENUM srcformat,int u4Orientation)
{
    //  (1) Config ISPParam / MDPParam
    MBOOL   ret = MFALSE;
    MINT32  err = -1;
    
    BufInfo jpgbufInfo; 
    jpgbufInfo = getBufInfo_JpgEnc(); 
    
    MY_DBG("[MDPJPGENC] jpgbufInfo 0x%x w %d h %d \n",jpgbufInfo.u4BufVA,mShotParam.frmJpg.w,mShotParam.frmJpg.h );
    halIDPParam_t   pMDPParam;
    MdpYuvAddr  jpgbuff;

    // For capture mode, SrcW/H is input, YuvW/H is jpeg, RgbW/H is for internal YUV
    // JPEG Config 
    ::memset((void*)&pMDPParam, 0, sizeof(halIDPParam_t));
    ::memset((void*)&jpgbuff, 0, sizeof(MdpYuvAddr));   
       
    rect_t rcSrc, rcDst, rcCrop;
    ::memset(&rcSrc, 0, sizeof(rect_t));
    ::memset(&rcDst, 0, sizeof(rect_t));
    ::memset(&rcCrop,0, sizeof(rect_t));
    if(u4Orientation==0||u4Orientation==2)
    {
        rcSrc.w = mu4W_yuv;
        rcSrc.h = mu4H_yuv;
        rcDst.w = mShotParam.frmJpg.w;
        rcDst.h = mShotParam.frmJpg.h;
    }   
    else
    {
        rcSrc.w = mu4H_yuv;
        rcSrc.h = mu4W_yuv;
        rcDst.w = mShotParam.frmJpg.h;
        rcDst.h = mShotParam.frmJpg.w;
    }    
    mpMdpHal->calCropRect(rcSrc, rcDst, &rcCrop, 100);
              
    MY_DBG("[MDPJPGENC] src:(w,h)=(%d,%d), dst:(w,h)=(%d,%d)", rcSrc.w, rcSrc.h, rcDst.w, rcDst.h);
    MY_DBG("[MDPJPGENC] crop:(x,y,w,h)=(%d,%d,%d,%d)", rcCrop.x, rcCrop.y, rcCrop.w, rcCrop.h);                	
    	
    pMDPParam.mode = MDP_MODE_CAP_JPG;          // For capture mode
    jpgbuff.y=srcadr;
    pMDPParam.Capture.b_jpg_path_disen = 0;
    pMDPParam.Capture.pseudo_src_enable = 1;
    pMDPParam.Capture.pseudo_src_yuv_img_addr = jpgbuff;
    pMDPParam.Capture.pseudo_src_color_format = MhalColorFormatToMdp(srcformat);
    pMDPParam.Capture.src_img_roi.x = rcCrop.x;
    pMDPParam.Capture.src_img_roi.y = rcCrop.y;
    pMDPParam.Capture.src_img_roi.w = rcCrop.w;
    pMDPParam.Capture.src_img_roi.h = rcCrop.h;
    if(u4Orientation==0||u4Orientation==2)
    {
        pMDPParam.Capture.src_img_size.w = mu4W_yuv;
        pMDPParam.Capture.src_img_size.h = mu4H_yuv;           
    }
    else
    {
        pMDPParam.Capture.src_img_size.w = mu4H_yuv;
        pMDPParam.Capture.src_img_size.h = mu4W_yuv; 
    }        
    
    pMDPParam.Capture.jpg_yuv_color_format = 422;   //integer : 411, 422, 444, 400, 410 etc...
    pMDPParam.Capture.jpg_buffer_addr = jpgbufInfo.u4BufVA;//VA
    pMDPParam.Capture.jpg_buffer_size = jpgbufInfo.u4BufSize * jpgbufInfo.u4BufCnt;
    pMDPParam.Capture.jpg_quality = mShotParam.u4JpgQValue;    // 39~90
    pMDPParam.Capture.jpg_b_add_soi = 0;                       // 1:Add EXIF 0:none
    if(u4Orientation==0||u4Orientation==2)
    {
        pMDPParam.Capture.jpg_img_size.w = mShotParam.frmJpg.w;
        pMDPParam.Capture.jpg_img_size.h = mShotParam.frmJpg.h;
    }
    else
    {
        pMDPParam.Capture.jpg_img_size.w = mShotParam.frmJpg.h;
        pMDPParam.Capture.jpg_img_size.h = mShotParam.frmJpg.w;
    }
    
    pMDPParam.Capture.b_qv_path_en = 1;
    pMDPParam.Capture.qv_path_sel =  0;  //0:auto select quick view path 1:QV_path_1 2:QV_path_2
    pMDPParam.Capture.qv_yuv_img_addr.y = bufInfo.u4BufVA; //VA    
    pMDPParam.Capture.qv_color_format = YUV420_Planar;
    pMDPParam.Capture.qv_flip = 0;
    pMDPParam.Capture.qv_rotate = 0;  
     
    pMDPParam.Capture.b_ff_path_en =  0;
    if(u4Orientation==0||u4Orientation==2)
    {    
        pMDPParam.Capture.qv_img_size.w = mShotParam.frmQv.w;
        pMDPParam.Capture.qv_img_size.h = mShotParam.frmQv.h;
    }
    else
    {    
        pMDPParam.Capture.qv_img_size.w = mShotParam.frmQv.h;
        pMDPParam.Capture.qv_img_size.h = mShotParam.frmQv.w;
    }    
    
    //  (2) Apply settings.
    ret = 0==(err = mpMdpHal->setConf(&pMDPParam));
    if  (!ret)
    {
        MY_ERR("[MDPJPGENC] Config LCE - ec(%x) \n", err);
        goto failExit;
    }

    //  (3) Start
    ret = 0==(err = mpMdpHal->start());
    if  (!ret)
    {
        MY_ERR("[MDPJPGENC] Start - ec(%x) \n", err);
        goto failExit;
    }

    //  (4) Wait Done
    ret =   waitJpgCaptureDone();
    if  (!ret)
    {
        MY_ERR("[MDPJPGENC] waitJpgCaptureDone fail \n");
        goto failExit;
    }
    ret = 0==(err = mpMdpHal->stop());
    if  (!ret)
    {
        MY_ERR("[MDPJPGENC] stop - ec(%x) \n", err);
        goto failExit;
    }
    return  ret;
failExit:    
    ret = MTRUE;
    return  ret;
}
/*******************************************************************************
* PQ path will reset ISP para, so we should reconfig ISP
*******************************************************************************/
MBOOL
Mhal_facebeauty::
reConfigISP()
{
    MY_DBG("[reConfigISP]");
    MBOOL   ret = MFALSE;  
    halISPIFParam_t pISPParam;                           //
    //
    ::memset((void*)&pISPParam, 0, sizeof(halISPIFParam_t));
    
    BufInfo const bufInfo = getBufInfo_Raw();
    
    pISPParam.u4IsSrcDram = 0;
    pISPParam.u4InPhyAddr = 0;
    pISPParam.u4SrcW = mu4W_raw;
    pISPParam.u4SrcH = mu4H_raw;

    pISPParam.u4IsDestDram = 0;
    pISPParam.u4OutPhyAddr = 0;

    pISPParam.u4IsDestBayer = 1;

    //  (3) Others.
    //  TODO: divide into in/out mem sizes
    pISPParam.u4MemSize = bufInfo.u4BufSize*bufInfo.u4BufCnt;
    
    mpIspHal->setConf(&pISPParam);
    
    MY_INFO("[configISPParam] u4SrcW %d, u4SrcH%d", pISPParam.u4SrcW , pISPParam.u4SrcH);
    ret = MTRUE;
lbExit:

    return  ret;
}

MBOOL 
Mhal_facebeauty::
doCapture() 
{    
    #if (FB_PROFILE_CAPTURE)
    MyDbgTimer DbgTmr("FB docapture");
    #endif
    MBOOL ret = MFALSE;
    MINT8 TargetColor = NSCamCustom::get_FB_ColorTarget();
    MINT8 BlurLevel = NSCamCustom::get_FB_BlurLevel();
    int MaxFace = 0;    
    if(BlurLevel>4)    	
        BlurLevel = 4; 
    if(BlurLevel<1)    	
        BlurLevel = 1;    
    
    for(int i=0;i<FBmetadata.number_of_faces;i++)	
	  {
	      if(MaxFace<(FBmetadata.faces[i].rect[2]-FBmetadata.faces[i].rect[0]))
	           MaxFace = FBmetadata.faces[i].rect[2]-FBmetadata.faces[i].rect[0];
    }    
    MY_INFO("[FACEINFO] MaxFace %d",MaxFace);
    if(MaxFace>=BigFaceWidthThreshold)
    {
        MaxFace = NSCamCustom::get_FB_NRTime();
        if(MaxFace>6)    	
            MaxFace = 6; 
        if(MaxFace<1)    	
            MaxFace = 1;
    }
    else if(MaxFace>=SmallFaceWidthThreshold)
        MaxFace = 4;
    else
        MaxFace = 2; 
    MY_INFO("[FACEINFO] NR time %d",MaxFace);
    ret =     
        //  ()  Request Buffers.
        requestBufs(getControlStep())
        //capture source
        &&  createSource(getControlStep())
        #if (FB_PROFILE_CAPTURE)
        &&	DbgTmr.print("FBProfiling - capture:: createSource")
        #endif
            //Initial Algorithm        
        &&  InitialAlgorithm(mu4W_yuv, mu4H_yuv, BlurLevel, TargetColor)
        #if (FB_PROFILE_CAPTURE)
        &&	DbgTmr.print("FBProfiling - capture:: InitialAlgorithm")
        #endif
            //resize
        &&  MDPResize((void*) mpSource->getVirtAddr(), mu4W_yuv, mu4H_yuv, (void*) mpSreResize->getVirtAddr(), ((mu4W_yuv>>2) << 1), ((mu4H_yuv>>2) << 1))
        #if (FB_PROFILE_CAPTURE)
        &&	DbgTmr.print("FBProfiling - capture:: MDPResize")
        #endif
            //BlurImage
        &&  createBlurImage((void*) mpSreResize->getVirtAddr(), ((mu4W_yuv>>2) << 1), ((mu4H_yuv>>2) << 1), (void*) mpBlurImg->getVirtAddr(), mu4W_yuv, mu4H_yuv, MaxFace)
        #if (FB_PROFILE_CAPTURE)
        &&	DbgTmr.print("FBProfiling - capture:: createBlurImage")
        #endif
        &&  WaitSaveFileDone()
            //AlphaMap
        &&  createAlphaMap((void*) mpSource->getVirtAddr(), (void*) mpSreResize->getVirtAddr(),(void*) mpBlurImg->getVirtAddr(),(void*) &FBmetadata, (void*) &MTKFaceInfo ,(void*) &msFaceBeautyResultInfo)       
        #if (FB_PROFILE_CAPTURE)
        &&	DbgTmr.print("FBProfiling - capture:: createAlphaMap")
        #endif
            //TextAlphaMap
        &&  createBlendedImage((void*) mpSreResize->getVirtAddr(),(void*) mpSource->getVirtAddr(),(void*) mpBlurImg->getVirtAddr(),(void*) msFaceBeautyResultInfo.AlphaMapDsAddr,(void*) msFaceBeautyResultInfo.AlphaMapColorDsAddr, ((mu4W_yuv>>2) << 1), ((mu4H_yuv>>2) << 1),(void*) mpAlphamap->getVirtAddr(),(void*) &msFaceBeautyResultInfo)
        #if (FB_PROFILE_CAPTURE)
        &&	DbgTmr.print("FBProfiling - capture:: createBlendedImage")
        #endif
            //DoPCA
        &&  MDPPCA((void*) mpBlurImg->getVirtAddr(), mu4W_yuv, mu4H_yuv, (void*) mpSreResize->getVirtAddr(), mu4W_yuv, mu4H_yuv, (void*) &msFaceBeautyResultInfo)    
        #if (FB_PROFILE_CAPTURE)
        &&	DbgTmr.print("FBProfiling - capture:: MDPPCA")
        #endif
            //Final image
        &&  createFinalImage((void*) mpSource->getVirtAddr(),(void*) mpBlurImg->getVirtAddr(), (void*) mpAlphamap->getVirtAddr(), (void*) &msFaceBeautyResultInfo)
        #if (FB_PROFILE_CAPTURE)
        &&	DbgTmr.print("FBProfiling - capture:: createFinalImage")
        #endif
        ;
    if  ( ! ret )
    {
        MY_INFO("[FBCapture] Capture fail");
    }
    sem_post(&semFBthread);
    
    return ret;
}

MVOID* FBCapture(void *arg)
{
    ::prctl(PR_SET_NAME, "FaceBeautythread", 0, 0, 0);
    mpFbObj->doCapture();
    return NULL;
}

MBOOL   
Mhal_facebeauty::
SaveOrgFile()
{
    MUINT32 u4JpgPictureSize = 0;
    
    ShotBase::flattenJpgPicBuf(getJpgEncDoneSize(), 0, u4JpgPictureSize); 
    mHalMiscDumpToFile((char*)mShotParam.uShotFileName, (UINT8*)mShotParam.frmJpg.virtAddr, u4JpgPictureSize); 
    return NULL;
}

MVOID* FBSaveFile(void *arg)
{
    MY_INFO("[FBSaveFile] Save File thread IN");
    ::prctl(PR_SET_NAME, "FBSaveFilethread", 0, 0, 0);
    mpFbObj->SaveOrgFile();
    return NULL;
}

MBOOL   
WaitSaveFileDone()
{
    MY_INFO("[WaitSaveFileDone] File Done IN");
    pthread_join(threadSaveFile, NULL);
    MY_INFO("[WaitSaveFileDone] File Done OUT");
    return  MTRUE; 
}
