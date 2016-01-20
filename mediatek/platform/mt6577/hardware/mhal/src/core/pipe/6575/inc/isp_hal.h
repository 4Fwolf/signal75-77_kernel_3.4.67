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
#ifndef ISP_HAL_H
#define ISP_HAL_H
//----------------------------------------------------------------------------
#include "pipe_types.h"
//----------------------------------------------------------------------------
#define ISP_HAL_IRQ_EXPDONE             (1)
#define ISP_HAL_IRQ_IDLE                (1<<3)
#define ISP_HAL_IRQ_ISPDONE             (1<<4)
#define ISP_HAL_IRQ_AFDONE              (1<<6)
#define ISP_HAL_IRQ_VSYNC               (1<<10)
#define ISP_HAL_IRQ_CLEAR_ALL           (1<<30)
#define ISP_HAL_IRQ_CLEAR_WAIT          (1<<31)
//
#define ISP_HAL_IRQ_TIMEOUT_DEFAULT     (3000) //ms
//----------------------------------------------------------------------------
typedef enum
{
    ISP_HAL_SCEN_CAM_PRV,
    ISP_HAL_SCEN_CAM_CAP
}ISP_HAL_SCEN_ENUM;

typedef struct
{
    MBOOL   Enable;
    MUINT32 Input;
    MUINT32 Output;
}ISP_HAL_SUBSAMPLE_STRUCT;

typedef struct
{
    MUINT32 u4InPhyAddr;     // For DRAM-IN-OUT
    MUINT32 u4OutPhyAddr;    // For DRAM-IN-OUT
    MUINT32 u4SrcW;          // For input sensor width 
    MUINT32 u4SrcH;          // For input sensor height 
    MUINT32 u4IsContinous;
    MUINT32 u4IsSrcDram;
    MUINT32 u4IsDestDram;
    MUINT32 u4IsDestBayer;   // 1: Bayer, 0: YUV422
    MUINT32 u4MemSize; 
    MUINT32 u4IsBypassSensorScenario;
    MUINT32 u4IsBypassSensorDelay;
    MUINT32 u4IsZsd;
    ISP_HAL_SUBSAMPLE_STRUCT SubsampleWidth;
    ISP_HAL_SUBSAMPLE_STRUCT SubsampleHeight;
    ISP_HAL_SCEN_ENUM Scen; //Only vaild when preview and capture size are the same.
}ISP_HAL_CONFIG_STRUCT;
//
typedef struct
{
    MUINT32 u4Width; 
    MUINT32 u4Height;
    MUINT32 u4BitDepth; 
    MUINT32 u4IsPacked; 
    MUINT32 u4Size;
    MUINT32 u1Order;
}ISP_HAL_RAW_INFO_STRUCT; 
//
typedef enum
{
    //No parameter set or get
    ISP_HAL_CMD_RESET                       = 0x1001,
    //Set by parameter
    ISP_HAL_CMD_SET_SENSOR_DEV              = 0x2001,
    ISP_HAL_CMD_SET_SENSOR_GAIN,
    ISP_HAL_CMD_SET_SENSOR_EXP,
    ISP_HAL_CMD_SET_CAM_MODE,
    ISP_HAL_CMD_SET_SCENE_MODE,
    ISP_HAL_CMD_SET_ISO,
    ISP_HAL_CMD_SET_FLUORESCENT_CCT,
    ISP_HAL_CMD_SET_SCENE_LIGHT_VALUE,
    ISP_HAL_CMD_SET_VALIDATE_FRAME,
    ISP_HAL_CMD_SET_OPERATION_MODE,
    ISP_HAL_CMD_SET_EFFECT,
    ISP_HAL_CMD_SET_ZOOM_RATIO,
    ISP_HAL_CMD_SET_BRIGHTNESS,
    ISP_HAL_CMD_SET_CONTRAST,
    ISP_HAL_CMD_SET_EDGE,
    ISP_HAL_CMD_SET_HUE,
    ISP_HAL_CMD_SET_SATURATION,
    ISP_HAL_CMD_SET_TUNING_CMD, 
    ISP_HAL_CMD_SET_OFFLINE_CAPTURE,
    ISP_HAL_CMD_SET_LOCK_REG,
    ISP_HAL_CMD_SET_SHADING_IDX,
    ISP_HAL_CMD_SET_CAP_DELAY_FRAME,
    ISP_HAL_CMD_SET_FREE_M4U_FLAG,
    ISP_HAL_CMD_SET_SENSOR_WAIT,
    //Get by parameter
    ISP_HAL_CMD_GET_SENSOR_PRV_RANGE        = 0x3001,
    ISP_HAL_CMD_GET_SENSOR_FULL_RANGE,
    ISP_HAL_CMD_GET_RAW_DUMMY_RANGE,
    ISP_HAL_CMD_GET_SENSOR_NUM,
    ISP_HAL_CMD_GET_SENSOR_TYPE,
    ISP_HAL_CMD_GET_RAW_INFO,
    ISP_HAL_CMD_GET_EXIF_DEBUG_INFO,
    ISP_HAL_CMD_GET_SHADING_IDX,
    ISP_HAL_CMD_GET_ATV_DISP_DELAY,    
    ISP_HAL_CMD_GET_SENSOR_DELAY_FRAME_CNT,
    ISP_HAL_CMD_GET_SENSOR_3D_CAP_TYPE,
    ISP_HAL_CMD_GET_VD_COUNT,
    ISP_HAL_CMD_SET_SENSOR_RESTART,
    ISP_HAL_CMD_MAX                         = 0xFFFF
}ISP_HAL_CMD_ENUM;
//
typedef enum
{
    ISP_HAL_SENSOR_DEV_NONE = 0x0,
    ISP_HAL_SENSOR_DEV_MAIN = 0x1,
    ISP_HAL_SENSOR_DEV_SUB  = 0x2,
    ISP_HAL_SENSOR_DEV_ATV  = 0x4,
    ISP_HAL_SENSOR_DEV_MAIN_2 = 0x8,    
}ISP_HAL_SENSOR_DEV_ENUM;
//
typedef enum
{
    ISP_HAL_SENSOR_TYPE_RAW, 
    ISP_HAL_SENSOR_TYPE_YUV, 
    ISP_HAL_SENSOR_TYPE_RGB565, 
    ISP_HAL_SENSOR_TYPE_UNKNOWN = 0xFF,
}ISP_HAL_SENSOR_TYPE_ENUM;
//
typedef enum
{
    ISP_HAL_SENSOR_WAIT_SHUTTER_GAIN,
    ISP_HAL_SENSOR_WAIT_AMOUNT
}ISP_HAL_SENSOR_WAIT_ENUM;
//
typedef enum
{
    ISP_HAL_CAM_MODE_PREVIEW,                   //  Camera Preview / VT
    ISP_HAL_CAM_MODE_VIDEO,                     //  Video Preview / Recording
    ISP_HAL_CAM_MODE_CAPTURE_FLY,
    ISP_HAL_CAM_MODE_CAPTURE_FLY_ZSD,
#ifdef MTK_ZSD_AF_ENHANCE
    ISP_HAL_CAM_MODE_PREVIEW_FLY_ZSD,
#endif
    ISP_HAL_CAM_MODE_CAPTURE_PASS1,
    ISP_HAL_CAM_MODE_CAPTURE_PASS2,
    ISP_HAL_CAM_MODE_CAPTURE_HDR_PASS1_SF,      //  HDR Pass1 Single Frame
    ISP_HAL_CAM_MODE_CAPTURE_HDR_PASS1_MF1,     //  HDR Pass1 Multi Frame Stage1
    ISP_HAL_CAM_MODE_CAPTURE_HDR_PASS1_MF2,     //  HDR Pass1 Multi Frame Stage2
    ISP_HAL_CAM_MODE_CAPTURE_HDR_PASS2,         //  HDR Pass2
    ISP_HAL_CAM_MODE_YUV2JPG_SCALADO,           //  YUV -> JPG (Scalado)
    ISP_HAL_CAM_MODE_YUV2JPG_ZSD,               //  YUV -> JPG (ZSD)
    ISP_HAL_CAM_MODE_FB_POSTPROCESS_NR2_ONLY,   // Post-processing for face beautifier: NR2 only
    ISP_HAL_CAM_MODE_FB_POSTPROCESS_PCA_ONLY,   // Post-processing for face beautifier: PCA only
    ISP_HAL_CAM_MODE_UNKNOWN = 0xFF,
}ISP_HAL_CAM_MODE_ENUM;
//
typedef enum
{
    ISP_HAL_OPER_MODE_NORMAL,
    ISP_HAL_OPER_MODE_PURE_RAW,
    ISP_HAL_OPER_MODE_META,
    ISP_HAL_OPER_MODE_CUSTOM,
}ISP_HAL_OPER_MODE_ENUM;;
//
typedef enum
{
  ISP_HAL_CAP_3D_TYPE_NONE = 0,
  ISP_HAL_CAP_3D_TYPE_FRAME_SEQUENTIAL,
  ISP_HAL_CAP_3D_TYPE_SIDE_BY_SIDE,
  ISP_HAL_CAP_3D_TYPE_TOP_BOTTOM
}ISP_HAL_CAP_3D_TYPE_ENUM;
//
typedef enum
{
    ISP_HAL_DELAY_FRAME_PREVIEW,
    ISP_HAL_DELAY_FRAME_VIDEO,
    ISP_HAL_DELAY_FRAME_CAPTURE
}ISP_HAL_DELAY_FRAME_ENUM;
//
typedef struct
{
    MUINT32 Mode;
    MUINT32 Timeout; //ms
}ISP_HAL_WAIT_IRQ_STRUCT;

typedef struct
{
    MUINT32     Width;
    MUINT32     Height;
}ISP_HAL_IMG_SIZE_STRUCT;

typedef struct
{
    ISP_HAL_IMG_SIZE_STRUCT     InputSize;
}ISP_HAL_CONFIG_PQ_STRUCT;
//----------------------------------------------------------------------------
//For backward compatible, please do not use anymore!
#define halISPIFParam_t                         ISP_HAL_CONFIG_STRUCT
#define halISPRawImageInfo_t                    ISP_HAL_RAW_INFO_STRUCT
//
#define halIspCmd_e                             ISP_HAL_CMD_ENUM
#define ISP_CMD_SET_SENSOR_DEV                  (ISP_HAL_CMD_SET_SENSOR_DEV)
#define ISP_CMD_SET_SENSOR_GAIN                 (ISP_HAL_CMD_SET_SENSOR_GAIN)
#define ISP_CMD_SET_SENSOR_EXP                  (ISP_HAL_CMD_SET_SENSOR_EXP)
#define ISP_CMD_SET_CAM_MODE                    (ISP_HAL_CMD_SET_CAM_MODE)
#define ISP_CMD_SET_SCENE_MODE                  (ISP_HAL_CMD_SET_SCENE_MODE)
#define ISP_CMD_SET_ISO                         (ISP_HAL_CMD_SET_ISO)
#define ISP_CMD_SET_FLUORESCENT_CCT             (ISP_HAL_CMD_SET_FLUORESCENT_CCT)
#define ISP_CMD_SET_SCENE_LIGHT_VALUE           (ISP_HAL_CMD_SET_SCENE_LIGHT_VALUE)
#define ISP_CMD_VALIDATE_FRAME                  (ISP_HAL_CMD_SET_VALIDATE_FRAME)
#define ISP_CMD_SET_OPERATION_MODE              (ISP_HAL_CMD_SET_OPERATION_MODE)
#define ISP_CMD_SET_EFFECT                      (ISP_HAL_CMD_SET_EFFECT)
#define ISP_CMD_SET_ZOOM_RATIO                  (ISP_HAL_CMD_SET_ZOOM_RATIO)
#define ISP_CMD_SET_BRIGHTNESS                  (ISP_HAL_CMD_SET_BRIGHTNESS)
#define ISP_CMD_SET_CONTRAST                    (ISP_HAL_CMD_SET_CONTRAST)
#define ISP_CMD_SET_EDGE                        (ISP_HAL_CMD_SET_EDGE)
#define ISP_CMD_SET_HUE                         (ISP_HAL_CMD_SET_HUE)
#define ISP_CMD_SET_SATURATION                  (ISP_HAL_CMD_SET_SATURATION)
#define ISP_CMD_SEND_TUNING_CMD                 (ISP_HAL_CMD_SET_TUNING_CMD)
#define ISP_CMD_DECIDE_OFFLINE_CAPTURE          (ISP_HAL_CMD_SET_OFFLINE_CAPTURE)
#define ISP_CMD_LOCK_REG                        (ISP_HAL_CMD_SET_LOCK_REG)
#define ISP_CMD_SET_SHADING_IDX                 (ISP_HAL_CMD_SET_SHADING_IDX)
#define ISP_CMD_GET_SENSOR_PRV_RANGE            (ISP_HAL_CMD_GET_SENSOR_PRV_RANGE)
#define ISP_CMD_GET_SENSOR_FULL_RANGE           (ISP_HAL_CMD_GET_SENSOR_FULL_RANGE)
#define ISP_CMD_GET_RAW_DUMMY_RANGE             (ISP_HAL_CMD_GET_RAW_DUMMY_RANGE)
#define ISP_CMD_GET_SENSOR_NUM                  (ISP_HAL_CMD_GET_SENSOR_NUM)
#define ISP_CMD_GET_SENSOR_TYPE                 (ISP_HAL_CMD_GET_SENSOR_TYPE)
#define ISP_CMD_GET_RAW_INFO                    (ISP_HAL_CMD_GET_RAW_INFO)
#define ISP_CMD_GET_EXIF_DEBUG_INFO             (ISP_HAL_CMD_GET_EXIF_DEBUG_INFO)
#define ISP_CMD_GET_SHADING_IDX                 (ISP_HAL_CMD_GET_SHADING_IDX)
#define ISP_CMD_GET_ATV_DISP_DELAY              (ISP_HAL_CMD_GET_ATV_DISP_DELAY)   
#define ISP_CMD_GET_SENSOR_DELAY_FRAME_CNT      (ISP_HAL_CMD_GET_SENSOR_DELAY_FRAME_CNT)
#define ISP_CMD_SET_FREE_M4U_FLAG 				(ISP_HAL_CMD_SET_FREE_M4U_FLAG)
#define ISP_CMD_HAL_MAX                         (ISP_HAL_CMD_MAX)
//
#define halIspSensorDev_e                       ISP_HAL_SENSOR_DEV_ENUM
#define ISP_SENSOR_DEV_NONE                     (ISP_HAL_SENSOR_DEV_NONE)
#define ISP_SENSOR_DEV_MAIN                     (ISP_HAL_SENSOR_DEV_MAIN)
#define ISP_SENSOR_DEV_SUB                      (ISP_HAL_SENSOR_DEV_SUB)
#define ISP_SENSOR_DEV_ATV                      (ISP_HAL_SENSOR_DEV_ATV)
#define ISP_SENSOR_DEV_MAIN_2                   (ISP_HAL_SENSOR_DEV_MAIN_2) 
//
#define halIspSensorType_e                      ISP_HAL_SENSOR_TYPE_ENUM
#define ISP_SENSOR_TYPE_RAW                     (ISP_HAL_SENSOR_TYPE_RAW)
#define ISP_SENSOR_TYPE_YUV                     (ISP_HAL_SENSOR_TYPE_YUV)
#define ISP_SENSOR_TYPE_RGB565                  (ISP_HAL_SENSOR_TYPE_RGB565)
#define ISP_SENSOR_TYPE_UNKNOWN                 (ISP_HAL_SENSOR_TYPE_UNKNOWN)
//
#define halIspCamMode_e                         ISP_HAL_CAM_MODE_ENUM
#define ISP_CAM_MODE_PREVIEW                    (ISP_HAL_CAM_MODE_PREVIEW)
#define ISP_CAM_MODE_VIDEO                      (ISP_HAL_CAM_MODE_VIDEO)
#define ISP_CAM_MODE_CAPTURE_FLY                (ISP_HAL_CAM_MODE_CAPTURE_FLY)
#define ISP_CAM_MODE_CAPTURE_FLY_ZSD            (ISP_HAL_CAM_MODE_CAPTURE_FLY_ZSD)
#ifdef MTK_ZSD_AF_ENHANCE
#define ISP_CAM_MODE_PREVIEW_FLY_ZSD            (ISP_HAL_CAM_MODE_PREVIEW_FLY_ZSD)
#endif
#define ISP_CAM_MODE_CAPTURE_PASS1              (ISP_HAL_CAM_MODE_CAPTURE_PASS1)
#define ISP_CAM_MODE_CAPTURE_PASS2              (ISP_HAL_CAM_MODE_CAPTURE_PASS2)
#define ISP_CAM_MODE_CAPTURE_HDR_PASS1_SF       (ISP_HAL_CAM_MODE_CAPTURE_HDR_PASS1_SF)
#define ISP_CAM_MODE_CAPTURE_HDR_PASS1_MF1      (ISP_HAL_CAM_MODE_CAPTURE_HDR_PASS1_MF1)
#define ISP_CAM_MODE_CAPTURE_HDR_PASS1_MF2      (ISP_HAL_CAM_MODE_CAPTURE_HDR_PASS1_MF2)
#define ISP_CAM_MODE_CAPTURE_HDR_PASS2          (ISP_HAL_CAM_MODE_CAPTURE_HDR_PASS2)
#define ISP_CAM_MODE_YUV2JPG_SCALADO            (ISP_HAL_CAM_MODE_YUV2JPG_SCALADO)
#define ISP_CAM_MODE_YUV2JPG_ZSD                (ISP_HAL_CAM_MODE_YUV2JPG_ZSD)
#define ISP_CAM_MODE_FB_POSTPROCESS_NR2_ONLY    (ISP_HAL_CAM_MODE_FB_POSTPROCESS_NR2_ONLY)
#define ISP_CAM_MODE_FB_POSTPROCESS_PCA_ONLY    (ISP_HAL_CAM_MODE_FB_POSTPROCESS_PCA_ONLY)
#define ISP_CAM_MODE_UNKNOWN                    (ISP_HAL_CAM_MODE_UNKNOWN)
//
#define halIspOperMode_T                        ISP_HAL_OPER_MODE_ENUM
#define ISP_OPER_MODE_NORMAL                    (ISP_HAL_OPER_MODE_NORMAL)
#define ISP_OPER_MODE_PURE_RAW                  (ISP_HAL_OPER_MODE_PURE_RAW)
#define ISP_OPER_MODE_META                      (ISP_HAL_OPER_MODE_META)
#define ISP_OPER_MODE_CUSTOM                    (ISP_HAL_OPER_MODE_CUSTOM)
//----------------------------------------------------------------------------
class IspHal
{
    protected:
        virtual ~IspHal() {};
            
    public:
        static IspHal* createInstance(void);
        virtual void destroyInstance(void) = 0;
        virtual MINT32 searchSensor(void) = 0;
        virtual MINT32 init(void) = 0;
        virtual MINT32 uninit(void) = 0;
        virtual MINT32 start(void) = 0;
        virtual MINT32 stop(void) = 0;
        virtual MINT32 setConf(ISP_HAL_CONFIG_STRUCT *pConfig) = 0;
        virtual MINT32 setConfPQ(ISP_HAL_CONFIG_PQ_STRUCT* pConfig) = 0;
        virtual MINT32 waitDone(MINT32 mode,MINT32 Timeout=ISP_HAL_IRQ_TIMEOUT_DEFAULT) = 0;
        virtual MINT32 waitIrq(ISP_HAL_WAIT_IRQ_STRUCT* pWaitIrq) = 0;
        virtual MINT32 sendCommand(
            MINT32      cmd,
            MINT32      arg1 = 0,
            MINT32      arg2 = 0,
            MINT32      arg3 = 0) = 0;
        virtual MINT32 dumpReg(void) = 0;
};
//----------------------------------------------------------------------------
#endif

