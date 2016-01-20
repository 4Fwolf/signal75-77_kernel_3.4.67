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
/*
** $Log: fd_hal_base.h $
 *
*/

#ifndef _FACEBEAUTIFY_HAL_BASE_H_
#define _FACEBEAUTIFY_HAL_BASE_H_

#include "pipe_types.h"

/*******************************************************************************
*
********************************************************************************/
#define WorkingBufferCun 4
#define WorkingBuferrsize (1024*82)
#define FBWorkMode  0
#define FBSkinToneEn  1
#define FBAlphaBackground  0
#define FBZoomRatio  2
#define FBFDWidth  320 //if image is vertical  FD width and Height must be exchange
#define FBFDHeight  240 //if image is vertical  FD width and Height must be exchange


typedef enum HalFACEBEAUTIFYObject_s {
    HAL_FACEBEAUTY_OBJ_NONE = 0,
    HAL_FACEBEAUTY_OBJ_SW,
    HAL_FACEBEAUTY_OBJ_SW_NEON,    
    HAL_FACEBEAUTY_OBJ_UNKNOWN = 0xFF,
} HalFACEBEAUTIFYObject_e;

typedef enum
{
    MTKPIPEFACEBEAUTY_IMAGE_YUV422,                 // input image format
    MTKPIPEFACEBEAUTY_IMAGE_MAX
} MTKPIPEFACEBEAUTY_IMAGE_FORMAT_ENUM;

typedef enum
{
    MTKPIPEFACEBEAUTY_CTRL_IDLE,
    MTKPIPEFACEBEAUTY_CTRL_ALPHA_MAP,             // 1. generate the alpha texture map and alpha color map 2. generate the alpha maps destinate size
    MTKPIPEFACEBEAUTY_CTRL_BLEND_TEXTURE_IMG,     // 1. blend the original image and blur image by alpha texture map. 2. generate the YSH table for PCA
    MTKPIPEFACEBEAUTY_CTRL_BLEND_COLOR_IMG,       // 1. blend the smooth image and PCA image by alpha color map.
    MTKPIPEFACEBEAUTY_CTRL_MAX
} MTKPIPEFACEBEAUTY_CTRL_ENUM;                    // specify in set proc info 

struct MTKPipeFaceBeautyTuningPara
{
    MINT32 WorkMode;                             //0:Extract skin mask + apply wrinkle removal, 1:Extract skin mask only    
    MINT32 SkinToneEn;                           //0:close skin tone adjustment; 1:open skin tone adjustment
    MINT32 BlurLevel;                            //0~4 (weak~strongest)
    MINT32 AlphaBackground;                      //0~255 (non-smooth to strongest smooth)
    MINT32 ZoomRatio;                            //zoom ratio of down-sampled image
    MINT32 TargetColor;                          //0: red, 1: white, 2: natural
};

struct MTKPipeFaceBeautyEnvInfo
{
    MUINT16  SrcImgDSWidth;                      // input DS image width
    MUINT16  SrcImgDSHeight;                     // input DS image height
    MUINT16  SrcImgWidth;                        // input image width
    MUINT16  SrcImgHeight;                       // input image height
    MUINT16  FDWidth;                            // FD width
    MUINT16  FDHeight;                           // FD height
    MTKPIPEFACEBEAUTY_IMAGE_FORMAT_ENUM SrcImgFormat;// source image format
    MUINT32  WorkingBufAddr;                     // working buffer
    MUINT32  WorkingBufSize;                     // working buffer size
    MTKPipeFaceBeautyTuningPara *pTuningPara;        // tuning parameters
};

struct MTKPipeFaceBeautyProcInfo
{
    MTKPIPEFACEBEAUTY_CTRL_ENUM FaceBeautyCtrlEnum;  // control the process in fb_core
    MUINT8* SrcImgDsAddr;                        // 1/4 size original image addr
    MUINT8* SrcImgAddr;                          // full size original image addr
    MUINT8* SrcImgBlurAddr;                      // full size blur image addr
    MINT32  FDLeftTopPointX1[15];                    // start x position of face in FD image (320x240)
    MINT32  FDLeftTopPointY1[15];                    // start y position of face in FD image (320x240)
    MINT32  FDBoxSize[15];		                     // size of face in FD image (320x240)
    MINT32  FDPose[15];                              // Direction of face (0: 0 degree, 1: 15 degrees, 2: 30 degrees, and the like
    MINT32  FaceCount;                           // Number of face in current image
    MUINT8*  AlphaMap;                           // full size alpha texture map
    MUINT8*  TexBlendResultAddr;
    MUINT8*  TexBlendAndYSResultAddr;
    MUINT8*  AlphaMapColor;                      // full size alpha color map
    MUINT8*  PCAImgAddr;                         // full size PCAed image addr
    MUINT8*  ColorBlendResultAddr;
};

struct MTKPipeFaceBeautyResultInfo
{
    MUINT32 AlphaMapDsAddr;                      // DS alpha texture map addr
    MUINT32 AlphaMapColorDsAddr;                 // DS alpha color map addr
    MUINT32 AlphaMapDsCrzWidth;                  // down sample the DS alpha texture map to this width then resize to full size
    MUINT32 AlphaMapDsCrzHeight;                 // down sample the DS alpha texture map to this height then resize to full size
    MUINT32 AlphaMapColorDsCrzWidth;             // down sample the DS alpha color map to this width then resize to full size
    MUINT32 AlphaMapColorDsCrzHeight;            // down sample the DS alpha color map to this height then resize to full size
    MINT32  AngleRange[2];                       // PCA Hue angle range
    MUINT32 PCAYTable;                           // PCA Y table
    MUINT32 PCASTable;                           // PCA Sat table
    MUINT32 PCAHTable;                           // PCA Hue table
    MUINT8* BlendTextureImgAddr;                 // result addr for blending the original and blur image with alpha texture map
    MUINT8* BlendTextureAndYSImgAddr;            // result addr for blending the original and blur image with alpha texture map    
    MUINT8* BlendColorImgAddr;                   // result addr for blending the smooth and PCAed image with alpha color map
};

/*******************************************************************************
*
********************************************************************************/
class halFACEBEAUTIFYBase {
public:
    //
    MINT32 gImgEV[9];
    static halFACEBEAUTIFYBase* createInstance(HalFACEBEAUTIFYObject_e eobject);
    virtual void      destroyInstance() = 0;
    virtual ~halFACEBEAUTIFYBase() {};
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFacebeautifyInit () -
    //! \brief init facebeautify 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFacebeautifyInit(void* FaceBeautyEnvInfo) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFacebeautifyUninit () -
    //! \brief Facebeautify uninit 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFacebeautifyUninit() {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCreateAlphaMap () -
    //! \brief Create Alpha Map
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalCreateAlphaMap(void* ImgSrcAddr,void* ImgDsAddr,void* ImgBlurAddr,void* FDResult,void* FDInfo,void* FaceBeautyResultInfo) {return 0;}
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalBLENDTEXTURE () -
    //! \brief create blend texture info
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalBLENDTEXTURE(void* BlendResultAdr,void* BlendYSResultAdr,void* AplhaMapBuffer,void* FaceBeautyResultInfo){return 0;}     
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalBLENDCOLOR () -
    //! \brief create blend color info
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalBLENDCOLOR(void* FinalAdr,void* AplhaMapColorBuffer,void* PCABuffer,void* FaceBeautyResultInfo){return 0;}  
    
};

class halFACEBEAUTIFYTmp : public halFACEBEAUTIFYBase {
public:
    //
    static halFACEBEAUTIFYBase* getInstance();
    virtual void destroyInstance();
    //
    halFACEBEAUTIFYTmp() {}; 
    virtual ~halFACEBEAUTIFYTmp() {};
};

#endif

