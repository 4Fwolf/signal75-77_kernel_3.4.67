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
#ifndef _ICAMERAIO_H_
#define _ICAMERAIO_H_

class IspHal; 
class MdpHal; 
namespace NSCamera {
    class PortVect;
};

typedef MINT32 (*CALLBACKFUNC_CAMERAIO)(MINT32 msgType, MINT32 arg1, MINT32 arg2, MVOID *user);
/*******************************************************************************
*
********************************************************************************/
class ICameraIO 
{
public: 
    //
    typedef NSCamera::EPixelFormat EPixelFormat;
    //
    enum ePREVIEW_PORT_ID
    {
        ePREVIEW_DISP_PORT      = 0x1,
        ePREVIEW_VIDEO_PORT     = 0x2, 
        ePREVIEW_FD_PORT        = 0x4,
        ePREVIEW_DISP_PORT_EX   = 0x8,
        ePREVIEW_ZSD_PORT       = 0x10,
    }; 
    //
    enum eSensorMode {
        eSENSOR_CAPTURE_MODE         = 0x0,      //default
        eSENSOR_PREVIEW_MODE         = 0x1, 
    }; 
    //
    //++
    enum eCameraMode {
        eCAMERA_MODE_DEFAULT         = 0x0,      //default
        eCAMERA_MODE_PREVIEW         = 0x1, 
        eCAMERA_MODE_VIDEO           = 0x2, 
        eCAMERA_MODE_VT              = 0x3, 
    }; 
    //--
    //
    enum eCallbackType {
        eCAMIO_CB_EIS    = 0x1, 
    }; 
    //
    struct ImgCfg_t {
        MUINT32 imgWidth;             //Image Width
        MUINT32 imgHeight;            //Image Height
        EPixelFormat imgFmt;          //Image Data Format 
        MUINT32 zoomRatio;            //Zoom Ratio 
        MUINT32 rotation;             //Rotation, 0, 90, 180, 270
        MUINT32 flip;                 //Flip,  0, 1:H_FLIP, 2:V_FLIP 
        MUINT32 upScaleCoef;          //up scale Coefficient 
        MUINT32 downScaleCoef;        //down scale coeffcient 
    }; 
    // 
    struct JpegCfg_t {
        MUINT32 quality; 
        MUINT32 isSOI; 
        MUINT32 isDither;         
    }; 
    //
    struct PreviewPipeCfg_t {
        ImgCfg_t* pInputConfig; 
        ImgCfg_t* pDispConfig; 
        ImgCfg_t* pVideoConfig; 
        ImgCfg_t* pFDConfig;
    };    
    //
    struct MemInfo_t {
        MUINT32 virtAddr; 
        MUINT32 phyAddr; 
        MUINT32 bufCnt; 
        MUINT32 bufSize; 
        //
        MemInfo_t(MUINT32 const _virtAddr = 0,           
                  MUINT32 const _phyAddr = 0, 
                  MUINT32 const _bufCnt = 0, 
                  MUINT32 const _bufSize = 0
                 )
                  : virtAddr(_virtAddr), phyAddr(_phyAddr),
                    bufCnt(_bufCnt), bufSize(_bufSize)
        {}
    };   
    //
    struct BuffInfo_t {
        MUINT32 hwIndex; 
        MUINT32 fillCnt; 
        MUINT32 bufAddr; 
        MUINT32 timeStampS; 
        MUINT32 timeStampUs; 
        BuffInfo_t(MUINT32 _hwIndex =0, 
                   MUINT32 _fillCnt = 0, 
                   MUINT32 _bufAddr = 0, 
                   MUINT32 _timeStampS = 0, 
                   MUINT32 _timeStampUs = 0
                  ) 
                  :hwIndex(_hwIndex), fillCnt(_fillCnt), 
                  bufAddr(_bufAddr), timeStampS(_timeStampS), 
                  timeStampUs(_timeStampUs) {}
                        
    };
    //
    struct Rect_t {
        MUINT32 x; 
        MUINT32 y; 
        MUINT32 w;  
        MUINT32 h; 
        Rect_t (MUINT32 _x = 0, MUINT32 _y = 0, MUINT32 _w = 0, MUINT32 _h = 0)
               :x(_x), y(_y), w(_w), h(_h){}
    }; 
 

public:  ////    Interfaces
    //
    static ICameraIO* createInstance();
    virtual MVOID destroyInstance() = 0;    
    //
    ICameraIO() {}; 
    virtual ~ICameraIO() {};     
    //
    virtual MINT32 init(IspHal* const pIspHalObj) = 0;
    //
    virtual MINT32 uninit() = 0;    
    //
    virtual MINT32 start() = 0; 
    //
    virtual MINT32 stop() = 0; 
    //
    // ++
    // N3D: Real-time priority, DISP > VIDEO > FD 
    virtual MINT32 getN3DPreviewFrame(ePREVIEW_PORT_ID const ePortID,
                                   BuffInfo_t* buffInfo
                                  ) = 0;
    //
    virtual MINT32 releaseN3DPreviewFrame(ePREVIEW_PORT_ID const ePortID,
                                       BuffInfo_t* const buffInfo
                                      ) = 0; 
    //
    // N3D: Preview Pipe connection
    // 1. Sensor --> ISP --> MDP --> Mem 
    // 2. MEM --> ISP --> MDP --> Mem 
    virtual MINT32 setN3DPreviewPipe(MUINT32 const eEnablePortID, 
                                  MemInfo_t* const pInMemInfo,                   //I: Mem Info (Image is from mem) 
                                  PreviewPipeCfg_t* const pPreviewPipeCfg,       //I: Prv Pipe Config
                                  MemInfo_t* const pDispMemInfo,                 //O: Disp Mem Info 
                                  MemInfo_t* const pVideoMemInfo                 //O: Video Mem Info 
                                  ) = 0;
    //
    virtual MBOOL setN3DCapturePipe(MUINT32 const eEnablePortID, 
                                  MemInfo_t* const pInMemInfo,                   //I: Mem Info (Image is from mem) 
                                  PreviewPipeCfg_t* const pPreviewPipeCfg,       //I: Prv Pipe Config
                                  MemInfo_t* const pDispMemInfo,                 //O: Disp Mem Info 
                                  MemInfo_t* const pVideoMemInfo                 //O: Video Mem Info 
                                  ) = 0;
    //
    virtual MINT32 dropN3DPreviewFrame() = 0; 
    //
#ifdef  MTK_NATIVE_3D_SUPPORT
    virtual MINT32 setCameraMode(eCameraMode const mode) = 0; 
#endif
    //--
    // Real-time priority, DISP > VIDEO > FD 
    virtual MINT32 getPreviewFrame(ePREVIEW_PORT_ID const ePortID,
                                   BuffInfo_t* buffInfo
                                  ) = 0;   
    //
    virtual MINT32 releasePreviewFrame(ePREVIEW_PORT_ID const ePortID,
                                       BuffInfo_t* const buffInfo
                                      ) = 0 ; 
    //
    // Preview Pipe connection
    // 1. Sensor --> ISP --> MDP --> Mem 
    // 2. MEM --> ISP --> MDP --> Mem 
    virtual MINT32 setPreviewPipe(MUINT32 const eEnablePortID, 
                                  MemInfo_t* const pInMemInfo,                   //I: Mem Info (Image is from mem) 
                                  PreviewPipeCfg_t* const pPreviewPipeCfg,       //I: Prv Pipe Config
                                  MemInfo_t* const pDispMemInfo,                 //O: Disp Mem Info 
                                  MemInfo_t* const pVideoMemInfo,                //O: Video Mem Info 
                                  MemInfo_t* const pFDMemInfo                     //O: FD Mem Info                       
                                  ) = 0;

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
                                     ) = 0;

    virtual MINT32 setZSDLastPreviewFrame(
                                     MUINT32 const pDisWidth,                       //I: Display size
                                     MUINT32 const pDisHeight,                      //I: 
                                     MUINT32 const zoomVal                          //I: Zoom value
                                     ) = 0;

    virtual MINT32  setZSDPreviewFrame (
            MUINT32 const pDisWidth,            //I: Display size
            MUINT32 const pDisHeight,           //I:
            MUINT32 const pZoomVal              //I: zoom value
    ) = 0;
    //
    virtual MBOOL setCapturePipe(NSCamera::PortVect const& rvpInPorts, NSCamera::PortVect const& rvpOutPorts) = 0;
    //
    virtual MINT32 setSensorMode(eSensorMode const mode) = 0; 
    //
    virtual MINT32 dropPreviewFrame() = 0; 
    //
    virtual MINT32 setPreviewZoom(MUINT32 const zoomVal,               //I: zoom value
                                  Rect_t* const pDest,                 //I: dest rect                                     
                                  Rect_t* pCrop                        //O: crop rect 
                                 ) = 0; 
    //
    virtual MINT32 setZSDPreviewZoom(MUINT32 const zoomVal,               //I: zoom value
                                     Rect_t* const pDest,                 //I: dest rect                                     
                                     Rect_t* pCrop,                       //O: crop rect 
                                     MUINT32 const pZsdpass               //I: zsd pass
                                     ) = 0; 
    //
    virtual MINT32 getJPEGSize(MUINT32 &jpegSize) = 0; 
    //
    virtual MINT32 getSensorRawSize(MUINT32 &width, MUINT32 &height) = 0; 
    //
    virtual MINT32 waitCaptureDone() = 0; 
    //
    virtual MVOID setCallbacks(CALLBACKFUNC_CAMERAIO pCameraIOCB, MVOID *user) = 0;     
    // 
    virtual MBOOL setZoomCropSrc(MUINT32 const a_u4StartX, MUINT32 a_u4StartY, MUINT32 const a_u4Width, MUINT32 const a_u4Height) = 0;     
    
protected:
    
};

#endif // _ICAMERAIO_H_

