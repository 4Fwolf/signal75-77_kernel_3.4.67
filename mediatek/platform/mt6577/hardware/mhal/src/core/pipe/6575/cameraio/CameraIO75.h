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
#ifndef _CAMERAIO75_H_
#define _CAMERAIO75_H_

#include "ICameraIO.h"
class IspHal;
class MdpHal;
class ResMgrHal;

#ifdef  MTK_NATIVE_3D_SUPPORT
    class N3DHalBase;
    struct IMG_INFO_S;
#endif
/*******************************************************************************
*
********************************************************************************/
class CameraIO75  : public ICameraIO
{  
    //
    enum ePipeEngine
    {
        ePipeNone               = 0x0,
        ePipeISP                = 0x1,
        ePipeMDP                = 0x2, 
        ePipeJPEG               = 0x4, 
    };      

public:  ////    Interfaces
    //
    virtual MVOID destroyInstance();    
    //
    CameraIO75(); 
    virtual ~CameraIO75();     
    //
    virtual MINT32 init(IspHal* const pIspHalObj);
    //
    virtual MINT32 uninit();    
    //
    virtual MINT32 start(); 
    //
    virtual MINT32 stop(); 
    //
    // ++
    // N3D: Real-time priority, DISP > VIDEO > FD 
    virtual MINT32 getN3DPreviewFrame(ePREVIEW_PORT_ID const ePortID,
                                   BuffInfo_t* buffInfo
                                  );
    //
    // N3D: Preview Pipe connection
    virtual MINT32 setN3DPreviewPipe(MUINT32 const eEnablePortID, 
                                  MemInfo_t* const pInMemInfo,                   //I: Mem Info (Image is from mem) 
                                  PreviewPipeCfg_t* const pPreviewPipeCfg,       //I: Prv Pipe Config
                                  MemInfo_t* const pDispMemInfo,                 //O: Disp Mem Info 
                                  MemInfo_t* const pVideoMemInfo                 //O: Video Mem Info 
                                  );
    //
    virtual MINT32 releaseN3DPreviewFrame(ePREVIEW_PORT_ID const ePortID,
                                           BuffInfo_t* const buffInfo
                                          ); 
    //
    virtual MBOOL setN3DCapturePipe(MUINT32 const eEnablePortID, 
                                  MemInfo_t* const pInMemInfo,                   //I: Mem Info (Image is from mem) 
                                  PreviewPipeCfg_t* const pPreviewPipeCfg,       //I: Prv Pipe Config
                                  MemInfo_t* const pDispMemInfo,                 //O: Disp Mem Info 
                                  MemInfo_t* const pVideoMemInfo                 //O: Video Mem Info 
                                  );
    //
    virtual MINT32 dropN3DPreviewFrame(); 
    //
#ifdef  MTK_NATIVE_3D_SUPPORT
    virtual MINT32 setCameraMode(eCameraMode const mode) {meCameraMode = mode; return 0; }
#endif
    // --
    //
    // Real-time priority, DISP > VIDEO > FD 
    virtual MINT32 getPreviewFrame(ePREVIEW_PORT_ID const ePortID,
                                   BuffInfo_t* buffInfo
                                  );   
    //
    virtual MINT32 releasePreviewFrame(ePREVIEW_PORT_ID const ePortID,
                                       BuffInfo_t* const buffInfo
                                      ); 
    //
    // Preview Pipe connection
    // 1. Sensor --> ISP --> MDP --> Mem 
    // 2. MEM --> ISP --> MDP --> Mem 
    virtual MINT32 setPreviewPipe(MUINT32 const eEnablePortID, 
                                  MemInfo_t* const pInMemInfo,                   //I: Mem Info (Image is from mem) 
                                  PreviewPipeCfg_t* const pPreviewPipeCfg,       //I: Prv Pipe Config
                                  MemInfo_t* const pDispMemInfo,                 //O: Disp Mem Info 
                                  MemInfo_t* const pVideoMemInfo,                //O: Video Mem Info 
                                  MemInfo_t* const pFDMemInfo                    //O: FD Mem Info                       
                                  );

    //
    // Zero Shutter Pipe connection
    // 1. Sensor --> ISP --> MDP --> Mem 
    // 2. MEM --> ISP --> MDP --> Mem 
    virtual MINT32 setZSDPreviewPipe(MUINT32 const eEnablePortID, 
                                     MemInfo_t* const pInMemInfo,                   //I: Mem Info (Image is from mem) 
                                     PreviewPipeCfg_t* const pPreviewPipeCfg,       //I: Prv Pipe Config
                                     MemInfo_t* const pDispMemInfo,                 //O: Disp Mem Info 
                                     MemInfo_t* const pVideoMemInfo,                //O: Video Mem Info 
                                     MemInfo_t* const pFDMemInfo,                   //O: FD Mem Info                       
                                     MUINT32 const pSkipPrev,                       //I: Skip sensor mode switch 
                                     MUINT32 const pZSDPass                         //I: 1:old pass, 2:new pass
                                     );
	
    virtual MINT32 setZSDLastPreviewFrame(
                                     MUINT32 const pDisWidth,                       //I: Display size
                                     MUINT32 const pDisHeight,                      //I: 
                                     MUINT32 const zoomVal                          //I: Zoom value
                                     );

    virtual MINT32  setZSDPreviewFrame (
            MUINT32 const pDisWidth,            //I: Display size
            MUINT32 const pDisHeight,           //I:
            MUINT32 const pZoomVal              //I: zoom value
    );
    //
    virtual MBOOL setCapturePipe(PortVect const& rvpInPorts, PortVect const& rvpOutPorts);
    //
    virtual MINT32 setSensorMode(eSensorMode const mode) {meSensorMode = mode; return 0; }
    //
    virtual MINT32 dropPreviewFrame(); 
    //
    virtual MINT32 setPreviewZoom(MUINT32 const zoomVal,               //I: zoom value
                                  Rect_t* const pDest,                 //I: dest rect                                     
                                  Rect_t* pCrop                        //O: crop rect 
                                  ); 
    //
    virtual MINT32 setZSDPreviewZoom(MUINT32 const zoomVal,            //I: zoom value
                                     Rect_t* const pDest,              //I: dest rect                                     
                                     Rect_t* pCrop,                    //O: crop rect 
                                     MUINT32 const pZsdpass            //I: zsd pass
                                     ); 
    //
    virtual MINT32 getJPEGSize(MUINT32 &jpegSize); 
    //
    virtual MINT32 getSensorRawSize(MUINT32 &width, MUINT32 &height); 
    //
    virtual MINT32 waitCaptureDone(); 
    //
    virtual MVOID setCallbacks(CALLBACKFUNC_CAMERAIO pCameraIOCB, MVOID *user); 
    //
    virtual MBOOL setZoomCropSrc(MUINT32 const a_u4StartX, MUINT32 a_u4StartY, MUINT32 const a_u4Width, MUINT32 const a_u4Height); 
    
    MINT32 eisCB(MINT32 *eisX, MINT32 *eisY) ;

protected:
    //
    static MINT32 eisGetInfoCallback(unsigned long *eis_x, unsigned long *eis_y); 

    //
    MINT32 configPrvMDPPort(ePREVIEW_PORT_ID const portID, 
                            ImgCfg_t const *config, 
                            MemInfo_t const *memInfo
                            ); 
    //
    MUINT32 mapMDPColorFormat(EPixelFormat const pixelFmt);  
    //
    MBOOL configISPParam(PortVect const& rvpInPorts, PortVect const& rvpOutPorts);
    //
    MBOOL configMDPParam(PortVect const& rvpInPorts, PortVect const& rvpOutPorts);
    //
    MINT32 getIspDestResolution(MUINT32 &width, MUINT32 &height);         
    //
    MBOOL   queryImgInfoForIspYuvOut(EPortID const eInPortID, ImgInfo const InImgInfo, ImgInfo& rOutImgInfo);
    //
    MINT32 calCropRect(Rect_t rSrc, Rect_t rDst, Rect_t *prCrop, MUINT32 zoomRatio);
    //
    MINT32 startESD();
protected:  ////    Resource Locking.
    MBOOL   lockPipe(MBOOL const isOn);
    MBOOL   lockPipe_Isp2Mem(MBOOL const isOn);

private:  
    Rect_t mZoomRectSrc; 
    Rect_t mZoomRectDst; 
    MUINT32 mZoomVal; 
    MUINT32 mCurPipeEngine; 
    MUINT32 mPreviewPort; 
    ResMgrHal*mpResMgr;
    IspHal *mpISPHalObj; 
    MdpHal *mpMDPHalObj; 
    halIDPParam_t mMDPParam; 
    halISPIFParam_t mISPParam; 
    mHalCamMemPool *mMDPWorkingBuf;     
#ifdef  MTK_NATIVE_3D_SUPPORT
    mHalCamMemPool *mMDPWorkingBufTg1;
    mHalCamMemPool *mMDPWorkingBufTg2;
    N3DHalBase *mpN3DObj;
    Rect_t mTg1CropRect;
    Rect_t mTg2CropRect;

    IN_DATA_N3D_T *mpInDataN3D;
    RUN_DATA_N3D_T *mpRunDataN3D;
    OUT_DATA_N3D_T *mpOutDataN3D;

    IMG_INFO_S *mpTg1Offset;
    IMG_INFO_S *mpTg2Offset;
    MUINT32 *mpTg1Addr;
    MUINT32 *mpTg2Addr;

    eCameraMode meCameraMode;
#endif
    eSensorMode meSensorMode; 
    CALLBACKFUNC_CAMERAIO mpCameraIOCB; 
    MVOID *mCallbackCookie; 
};

#endif // _CAMERAIO75_H_

