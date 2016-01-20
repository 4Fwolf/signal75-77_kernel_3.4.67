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
#define LOG_TAG "mHalCamN3D"
//
#include <utils/Errors.h>
#include <utils/threads.h>
//
#include <mhal/inc/MediaHal.h>
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
#include "CameraProfile.h"

#include "mhal_interface.h"    //for mdp bitblt 


using namespace NSCamera;


/*******************************************************************************
*
********************************************************************************/
NormalShotN3D*
NormalShotN3D::
createInstance(char const*const szShotName, ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
{
    return  new NormalShotN3D(szShotName, eSensorType, eDeviceId);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
NormalShotN3D::
destroyInstance()
{
    delete  this;
}


/*******************************************************************************
*
********************************************************************************/
NormalShotN3D::
NormalShotN3D(char const*const szShotName, ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
    : ShotBaseN3D(szShotName, eSensorType, eDeviceId)
    , mpCameraIOObj(NULL)
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalShotN3D::
init(ShotParam const& rShotParam, Hal3ABase*const pHal3A, ICameraIO*const pCameraIO)
{
    MBOOL   ret = MFALSE;

    if  ( ! ShotBaseN3D::init(rShotParam, pHal3A) )
    {
        goto lbExit;
    }

    if  ( ! pCameraIO )
    {
        goto lbExit;
    }
    mpCameraIOObj = pCameraIO;

    ret = MTRUE;
lbExit:
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalShotN3D::
uninit()
{
    mpCameraIOObj = NULL;
    return  ShotBaseN3D::uninit();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalShotN3D::
capture()
{
    AutoCPTLog cptlog(Event_NShot_capture);
    ShotProfileN3D profile("capture", getShotName());

    MBOOL   ret = MFALSE;
    //
    //
    //  (1) Invoke Shutter Callback
    invokeShutterCB();


    // (*2) for YUV sensor bandwidth leak
    SensorPortInfo   sensorInPort;
    JpegPortInfo     jpegOutPort;

    querySensorPort(sensorInPort);
    queryJpegPort(jpegOutPort);
    //

    //  (2) Determine whether or not to offline capture.
    MBOOL fgIsOfflineCapMode = MFALSE;
    if  (
            (mShotParam.u4ZoomRatio != 100)
        ||  (mShotParam.u4IsDumpRaw == 1)
        ||  (mShotParam.u4Scalado == 1)
        ||  (mShotParam.u4Rotate != 0)
        ||  (jpegOutPort.u4ImgWidth > sensorInPort.u4ImgWidth)
        )
    {
        MY_LOGI("[capture] jpegOutPort.w %d > sensorInPort.w %d", jpegOutPort.u4ImgWidth, sensorInPort.u4ImgWidth);
        fgIsOfflineCapMode = MTRUE;
    }
    else
    {
        fgIsOfflineCapMode = MFALSE;
        mpIspHal->sendCommand(ISP_CMD_DECIDE_OFFLINE_CAPTURE, reinterpret_cast<int>(&fgIsOfflineCapMode));
    }
    MY_LOGI("[capture] fgIsOfflineCapMode:0x%x", fgIsOfflineCapMode);

    //  (3) Create Image
    //
    if  (mShotParam.camZsdParam.u4ZsdAddr)
    {
        PortVect vpMemPorts, vpOutPorts;
        MY_LOGI("[capture]ZSD, E \n");

        MemoryPortInfo   memoryPort;
        JpegPortInfo     jpegPort;
        PostviewPortInfo qvPort;

        ret =   queryInputMemPort(memoryPort)
            &&  queryJpegPort(jpegPort)
            &&  queryPostviewPort(qvPort);

        vpMemPorts.push(&memoryPort);
        vpOutPorts.push(&jpegPort);
        vpOutPorts.push(&qvPort);

        ret =   ret
            &&  createJpgImage(vpMemPorts, vpOutPorts);
        MY_LOGI("[capture]ZSD, X \n");
    }
    else
    {
        if  ( fgIsOfflineCapMode )
        {   //  Offline Capture
            //
            //  [Sensor Port] --> ISP --> [Memory Port] --> MDP --> [Jpeg Port]
            //                                                      [QV Port]
            //
            PortVect vpInPorts, vpMemPorts, vpOutPorts;
            //
            SensorPortInfo   sensorPort;
            MemoryPortInfo   memoryPort;
            JpegPortInfo     jpegPort;
            PostviewPortInfo qvPort;
            //
            ret =   querySensorPort(sensorPort)
                &&  queryRawMemPort(memoryPort)
                &&  queryJpegPort(jpegPort)
                &&  queryPostviewPort(qvPort)
                    ;
            //
            vpInPorts.push(&sensorPort);
            //
            vpMemPorts.push(&memoryPort);
            //
            vpOutPorts.push(&jpegPort);
            vpOutPorts.push(&qvPort);
            //
            ret =   ret
                &&  createRawImage(vpInPorts, vpMemPorts)
                &&  createJpgImage(vpMemPorts, vpOutPorts)
                    ;
        }
        else
        {   //  Online Capture
            //
            //  [Sensor Port] --> ISP --> --> MDP --> [Jpeg Port]
            //                                        [QV Port]
            //
            PortVect vpInPorts, vpOutPorts;
            //
            SensorPortInfo   sensorPort;
            JpegPortInfo     jpegPort;
            PostviewPortInfo qvPort;
            //
            ret =   querySensorPort(sensorPort)
                &&  queryJpegPort(jpegPort)
                &&  queryPostviewPort(qvPort)
                    ;
            //
            vpInPorts.push(&sensorPort);
            //
            vpOutPorts.push(&jpegPort);
            vpOutPorts.push(&qvPort);
            //
            ret =   ret
                &&  createJpgImage(vpInPorts, vpOutPorts)
                    ;
        }
    }


    profile.print();


    ret =   ret
    //  (4) Handle Image Ready.
        &&  handleImageReady()
            ;
    if  ( ! ret )
    {
        goto lbExit;
    }


    ret = MTRUE;
lbExit:

    //  (5) Wait Shutter Callback Done.
    if  ( ! waitShutterCBDone() )
    {
        MY_LOGE("[capture] waitShutterCBDone() fail");
        ret = MFALSE;
    }

    //  (6) Force to handle done even if any error has happened before.
    if  ( ! handleCaptureDone() )
    {
        MY_LOGE("[capture] handleCaptureDone() fail");
        ret = MFALSE;
    }

    if  ( ! ret )
    {
        MY_LOGE("[capture] fail");
    }

    profile.print();
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalShotN3D::
configPipe(PortVect const& rvpInPorts, PortVect const& rvpOutPorts)
{
    MINT32  err = 0;

    //  (1) Set Sensor Mode.
    //
    err = mpCameraIOObj->setSensorMode(
        mShotParam.u4CapPreFlag
        ? ICameraIO::eSENSOR_PREVIEW_MODE
        : ICameraIO::eSENSOR_CAPTURE_MODE
    );
    if  ( 0 > err )
    {
        MY_LOGE("[configPipe] mpCameraIOObj->setSensorMode err(%x)", err);
        goto lbExit;
    }

    //  (2) Set capture Pipe
    if ( ! mpCameraIOObj->setCapturePipe(rvpInPorts, rvpOutPorts) )
    {
        MY_LOGE("[configPipe] mpCameraIOObj->setCapturePipe");
        err = -1;
        goto lbExit;
    }

    err = 0;
lbExit:
    return  (0==err);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalShotN3D::
createImage(PortVect const& rvpInPorts, PortVect const& rvpOutPorts)
{
    ShotProfileN3D profile("createImage", getShotName());
    //
    MINT32  err = 0;
    //
    MUINT32 u4IspMode = 0;
    MUINT32 u4AaaMode = 0;
    //
    //  (0) Check arguments.
    PortInfo const* pInPortMain = NULL; //  pointer to main input port
    PortInfo const* pOutPortMain= NULL; //  pointer to main output port
    if  (
            rvpInPorts.isEmpty()  || ! rvpInPorts[0]
        ||  rvpOutPorts.isEmpty() || ! rvpOutPorts[0]
        )
    {
        MY_LOGE("[createImage] bad arguments: in:(%d, 0x%p) out:(%d, 0x%p)", rvpInPorts.size(), rvpInPorts[0], rvpOutPorts.size(), rvpOutPorts[0]);
        goto lbExit;
    }
    pInPortMain  = rvpInPorts[0];
    pOutPortMain = rvpOutPorts[0];
    MY_LOGI("[createImage] main format:(in, out)=(0x%x, 0x%x)", pInPortMain->eImgFmt, pOutPortMain->eImgFmt);


    //  (1) Determine Isp Mode and Aaa Mode
    if  ( MBOOL const fgIsSensorIn = (ePortID_Sensor == pInPortMain->ePortID) )
    {
        switch  ( pOutPortMain->eImgFmt )
        {
        //  Sensor -> ISP -> Mem (BayerOut)
        case ePIXEL_FORMAT_BAYER8:
        case ePIXEL_FORMAT_BAYER10:
        case ePIXEL_FORMAT_BAYER12:
            u4IspMode = ISP_CAM_MODE_CAPTURE_PASS1;
            u4AaaMode = 0;
            break;

        //  Sensor -> ISP -> MDP -> JPEG
        case ePIXEL_FORMAT_JPEG:
        //  Sensor -> ISP -> (MDP) -> Mem
        default:
            u4IspMode = ISP_CAM_MODE_CAPTURE_FLY;
            u4AaaMode = 0;
            break;
        }
    }
    else
    {
        switch  ( pOutPortMain->eImgFmt )
        {
        case ePIXEL_FORMAT_JPEG:
            //  Mem -> (ISP) -> MDP -> JPEG
            if  (1 == mShotParam.u4Scalado)
            {
                u4IspMode = ISP_CAM_MODE_YUV2JPG_SCALADO;
                u4AaaMode = 1;
            }
            else if (pInPortMain->eImgFmt == ePIXEL_FORMAT_YUY2)
            {
                u4IspMode = ISP_CAM_MODE_YUV2JPG_ZSD;
                u4AaaMode = 1;
            }
            else
            {
                u4IspMode = ISP_CAM_MODE_CAPTURE_PASS2;
                u4AaaMode = 1;
            }
            break;
        default:
            //  Mem -> (ISP) -> (MDP) -> Mem
            MY_LOGE("[createImage] unsupported: Mem -> (ISP) -> (MDP) -> Mem");
            err = -1;
            goto lbExit;
        }
    }
    MY_LOGI("[createImage] (u4IspMode, u4AaaMode)=(%d, %d)", u4IspMode, u4AaaMode);

    //  (2) Configure Isp Mode and Aaa Mode
    //
    err = mpIspHal->sendCommand(ISP_CMD_SET_OPERATION_MODE, mShotParam.u4CamIspMode);
    if (err < 0) {
        MY_LOGE("[createImage] mpIspHal->sendCommand(ISP_CMD_SET_OPERATION_MODE, %d) - err(%x)", mShotParam.u4CamIspMode, err);
        goto lbExit;
    }
    //
    err = mpIspHal->sendCommand(ISP_CMD_SET_CAM_MODE, u4IspMode);
    if (err < 0) {
        MY_LOGE("[createImage] mpIspHal->sendCommand(ISP_CMD_SET_CAM_MODE, %d) - err(%x)", u4IspMode, err);
        goto lbExit;
    }
    //
    if ((mShotParam.camZsdParam.u4ZsdEVShot==1)||(mShotParam.camZsdParam.u4ZsdEnable==0)){
        err = mpHal3A->setCaptureParam(u4AaaMode);
        if (err < 0) {
            MY_LOGE("[createImage] mpHal3A->setCaptureParam(%d) - err (%x)", u4AaaMode, err);
            goto lbExit;
        }
    }
    profile.print();

    //  (3) Configure Pipe.
    if  ( ! configPipe(rvpInPorts, rvpOutPorts) )
    {
        err = -1;
        goto lbExit;
    }
    //
    err = mpCameraIOObj->start();
    if (err < 0) {
        MY_LOGE("[createImage] mpCameraIOObj->start err (0x%x)", err); 
        goto lbExit; 
    }
    //
    err = mpCameraIOObj->waitCaptureDone();
    if (err < 0) {
        MY_LOGE("[createImage] mpCameraIOObj->waitCaptureDone err (0x%x)", err); 
        mpIspHal->dumpReg(); 
        mpCameraIOObj->stop();
        goto lbExit; 
    }
    //
    err = mpCameraIOObj->stop(); 
    if (err < 0) {
        MY_LOGE("[createImage] mpCameraIOObj->stop err (0x%x)", err); 
        goto lbExit; 
    }

lbExit:
    profile.print();
    return  (0==err);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalShotN3D::
createRawImage(PortVect const& rvpInPorts, PortVect const& rvpOutPorts)
{
    AutoCPTLog cptlog(Event_NShot_createRawImage);
    ShotProfileN3D profile("createRawImage", getShotName());

    MBOOL   ret = MFALSE;

    //
    if  ( ! createImage(rvpInPorts, rvpOutPorts) )
    {
        MY_LOGE("[createRawImage] createImage() fails");
        goto lbExit;
    }


    if  (1 == mShotParam.u4Scalado)
    {
        invokeCallback(MHAL_CAM_CB_SCALADO, (MVOID*)rvpOutPorts[0]->u4BufVA);
    }


    //  Save file if needed.
    if  ( mu4ShotDumpOPT || 1 == mShotParam.u4IsDumpRaw )
    {
        if(1 == mShotParam.u4DumpYuvData)
        {
            ::mHalMiscDumpToFile((char *)mShotParam.u1FileName, (MUINT8*)rvpOutPorts[0]->u4BufVA, mShotParam.frmJpg.w*mShotParam.frmJpg.h*2);   
        }
        else
        {
            halISPRawImageInfo_t ispRawInfo;
            MINT32 err = mpIspHal->sendCommand(ISP_CMD_GET_RAW_INFO, (int)&ispRawInfo);
            if (err < 0) {
                MY_LOGE("[createRawImage] mpIspHal->sendCommand(ISP_CMD_GET_RAW_INFO) - err(%x)", err);
                goto lbExit;
            }
            else {
                ::mHalMiscDumpToFile((char *)mShotParam.u1FileName, (MUINT8*)rvpOutPorts[0]->u4BufVA, ispRawInfo.u4Size);
            }
        }
    }


    ret = MTRUE;
lbExit:
    profile.print();
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
NormalShotN3D::
createJpgImage(PortVect const& rvpInPorts, PortVect const& rvpOutPorts)
{
    AutoCPTLog cptlog(Event_NShot_createJpgImage);
    ShotProfileN3D profile("createJpgImage", getShotName());

    MBOOL   ret = MFALSE;
    MINT32  err = 0;
    //
    MUINT32 u4JpegSize = 0;

    //
    if  ( ! createImage(rvpInPorts, rvpOutPorts) )
    {
        MY_LOGE("[createJpgImage] createImage() fails");
        goto lbExit;
    }

    //
    err = mpCameraIOObj->getJPEGSize(u4JpegSize);
    if (err < 0) {
        MY_LOGE("[createJpgImage] mpCameraIOObj->getJPEGSize err(0x%x)", err);
        goto lbExit;
    }
    setJpgEncDoneSize(u4JpegSize);
    MY_LOGD("[createJpgImage] jpeg size = %d", u4JpegSize);


    //  Save file if needed.
    if  (mu4ShotDumpOPT)
    {
        ::mHalMiscDumpToFile((char*)"//sdcard//qv.raw", (MUINT8*)(mShotParam.frmQv.virtAddr), mShotParam.frmQv.frmSize);
    }


    ret = MTRUE;
lbExit:
    profile.print();
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
NormalShotN3D::
querySensorPort(SensorPortInfo& rPort)
{
    MBOOL   ret = MFALSE;

    ImgInfo imgInfo;
    switch  (meSensorType)
    {
    case eSensorType_RAW:
        imgInfo.eImgFmt = ePIXEL_FORMAT_BAYER10;  //  FIXME: depends on raw sensor type.
        ret = queryIspBayerResolution(imgInfo.u4ImgWidth, imgInfo.u4ImgHeight);
        break;
    case eSensorType_YUV:
    default:
        imgInfo.eImgFmt = ePIXEL_FORMAT_YUY2;     //  FIXME: depends on yuv sensor type.
        ret = queryIspYuvResolution(imgInfo.u4ImgWidth, imgInfo.u4ImgHeight);
        break;
    }

    if  (!ret)
    {
        MY_LOGE("[querySensorPort] meSensorType(%d)", meSensorType);
        goto lbExit;
    }


    rPort = SensorPortInfo(
        imgInfo, 
        BufInfo(NULL, NULL, 0, 0)   //  no mem due to sensor in.
    );

    ret = MTRUE;
lbExit:
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
NormalShotN3D::
queryJpegPort(JpegPortInfo& rPort)
{
    rPort = JpegPortInfo(
        ImgInfo(ePIXEL_FORMAT_JPEG, mShotParam.frmJpg.w, mShotParam.frmJpg.h), 
        getBufInfo_JpgEnc()
    );
    rPort.u4zoomRatio = mShotParam.u4ZoomRatio;
    rPort.u4Quality   = mShotParam.u4JpgQValue;

    return  MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
NormalShotN3D::
queryPostviewPort(PostviewPortInfo& rPort)
{
    mhalCamFrame_t const& frmQv = mShotParam.frmQv;
    rPort = PostviewPortInfo(
        ImgInfo((EPixelFormat)frmQv.frmFormat, frmQv.w, frmQv.h), 
        BufInfo(frmQv.virtAddr, frmQv.phyAddr, frmQv.bufSize, frmQv.frmCount)
    );
    rPort.u4zoomRatio = mShotParam.u4ZoomRatio;
    return  MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
NormalShotN3D::
queryRawMemPort(MemoryPortInfo& rPort)
{
    MBOOL   ret = MFALSE;

    ImgInfo imgInfo;

    if  ( eSensorType_YUV == meSensorType || 1 == mShotParam.u4Scalado || 1 == mShotParam.u4DumpYuvData )
    {
        //  Yuv/Raw Sensor -> Mem (YUV) -> JPEG
        imgInfo.eImgFmt = ePIXEL_FORMAT_YUY2;
        ret = queryIspYuvResolution(imgInfo.u4ImgWidth, imgInfo.u4ImgHeight);
    }
    else
    {
        //  Raw Sensor -> Mem (Bayer) -> JPEG
        imgInfo.eImgFmt = ePIXEL_FORMAT_BAYER10;
        ret = queryIspBayerResolution(imgInfo.u4ImgWidth, imgInfo.u4ImgHeight);
    }

    if  (!ret)
    {
        MY_LOGE("[queryRawMemPort] meSensorType(%d)", meSensorType);
        goto lbExit;
    }


    rPort = MemoryPortInfo(imgInfo, getBufInfo_Raw());

    if(mShotParam.u4DumpYuvData == 1)
    {
    
        rPort.u4ImgWidth = mShotParam.frmJpg.w;
        rPort.u4ImgHeight = mShotParam.frmJpg.h;
        rPort.fgDumpYuvData = MTRUE; 
        rPort.u4zoomRatio = 100;
    }
    else
    {
        rPort.fgDumpYuvData = MFALSE;
        rPort.u4zoomRatio =  mShotParam.u4ZoomRatio;
    }

    rPort.u4Rotation = mShotParam.u4Rotate;

    ret = MTRUE;
lbExit:
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
NormalShotN3D::
queryInputMemPort(MemoryPortInfo& rPort)
{
    MBOOL   ret = MFALSE;

    mhalZsdParam_t const& rCamZsdParam = mShotParam.camZsdParam;
    ImgInfo const imgInfo(ePIXEL_FORMAT_YUY2, rCamZsdParam.u4ZsdWidth, rCamZsdParam.u4ZsdHeight);

    rPort = MemoryPortInfo(imgInfo, BufInfo(rCamZsdParam.u4ZsdAddr, NULL, imgInfo.u4ImgWidth*imgInfo.u4ImgHeight*2, 1));
    rPort.u4zoomRatio = mShotParam.u4ZoomRatio;
    MY_LOGI("[queryInputMemPort:zsd]W(%d),H(%d),Zoom(%d)", imgInfo.u4ImgWidth,imgInfo.u4ImgHeight,rPort.u4zoomRatio);

    ret = MTRUE;
lbExit:
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
NormalShotN3D::
handleImageReady()
{
    return  MTRUE;
}

