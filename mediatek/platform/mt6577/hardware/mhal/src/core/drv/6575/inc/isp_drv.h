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
#ifndef ISP_DRV_H
#define ISP_DRV_H
//----------------------------------------------------------------------------
#include "drv_types.h"
//----------------------------------------------------------------------------
typedef enum
{
    ISP_DRV_OUTPUT_FORMAT_BAYER,
    ISP_DRV_OUTPUT_FORMAT_YUV444,
    ISP_DRV_OUTPUT_FORMAT_YUV422
}ISP_DRV_OUTPUT_FORMAT_ENUM;
//
typedef enum
{
    ISP_DRV_INPUT_FORMAT_BAYER,
    ISP_DRV_INPUT_FORMAT_YUV422,
    ISP_DRV_INPUT_FORMAT_YCBCR,
    ISP_DRV_INPUT_FORMAT_RGB565
}ISP_DRV_INPUT_FORMAT_ENUM;
//
typedef enum
{
    //No parameter set or get
    ISP_DRV_CMD_RESET_BUF       = 0x1001,
    ISP_DRV_CMD_CLEAR_INT_STATUS,            
    ISP_DRV_CMD_RESET_VD_COUNT,
    //Set by parameter
    ISP_DRV_CMD_SET_PIXEL_TYPE  = 0x2001,
    ISP_DRV_CMD_SET_KDBG_FLAG,
    ISP_DRV_CMD_SET_CAPTURE_DELAY,
    ISP_DRV_CMD_SET_VD_PROC,               
    ISP_DRV_CMD_SET_PQ_PARAM,
    //Get by parameter
    ISP_DRV_CMD_GET_ADDR        = 0x3001,
    ISP_DRV_CMD_GET_LINE_ID,
    ISP_DRV_CMD_GET_CAPTURE_DELAY,          
    ISP_DRV_CMD_GET_VD_COUNT,
    ISP_DRV_CMD_GET_PQ_PARAM,
    ISP_DRV_CMD_DRV_MAX         = 0xFFFF
}ISP_DRV_CMD_ENUM;
//
#define ISP_DRV_CAM_PLL_48_GROUP    (1)
#define ISP_DRV_CAM_PLL_52_GROUP    (2)
//
#define ISP_DRV_IRQ_EXPDONE         (1)
#define ISP_DRV_IRQ_IDLE            (1<<3)
#define ISP_DRV_IRQ_ISPDONE         (1<<4)
#define ISP_DRV_IRQ_AFDONE          (1<<6)
#define ISP_DRV_IRQ_VSYNC           (1<<10)
#define ISP_DRV_IRQ_CLEAR_ALL       (1<<30)
#define ISP_DRV_IRQ_CLEAR_WAIT      (1<<31)
//
#define ISP_DRV_IRQ_TIMEOUT_DEFAULT     (3000) //ms
//
#define ISP_DRV_LOG_MSG     0x00000001
#define ISP_DRV_LOG_WRN     0x00000002
#define ISP_DRV_LOG_ERR     0x00000004
//
typedef struct
{
    MUINT32 Mode;
    MUINT32 Timeout; //ms
}ISP_DRV_WAIT_IRQ_STRUCT;
//
typedef struct
{
    MUINT32 u4SkinToneIndex;
    MUINT32 u4GrassToneIndex;
    MUINT32 u4SkyToneIndex;
    MUINT32 u4YCCGOIndex;
    MUINT32 u4EEIndex;
    MUINT32 u4Reset;//don't call it in constructor
}ISP_DRV_PQIndex_STRUCT;
//
typedef void (*pIspDrvCallback)(MBOOL);
//----------------------------------------------------------------------------
//For backward compatible, please do not use anymore!
#define OUTPUT_TYPE_FMT_YUV422      (ISP_DRV_OUTPUT_FORMAT_YUV422)
#define CMD_GET_ISP_ADDR            (ISP_DRV_CMD_GET_ADDR)
#define CMD_GET_LINE_ID             (ISP_DRV_CMD_GET_LINE_ID)
//----------------------------------------------------------------------------
class IspDrv
{
    protected:
        virtual ~IspDrv() {};
        
    public:
        static IspDrv* createInstance(void);
        virtual void   destroyInstance(void) = 0;        
        virtual MINT32 init(void) = 0;
        virtual MINT32 initTG2(
            MUINT32   grabWidth,
            MUINT32   grabHeight) = 0;   
        virtual MINT32 uninit(void) = 0;
        virtual MINT32 uninitTG2(void) = 0; 
        virtual MINT32 initPQ(void) = 0;
        virtual MINT32 uninitPQ(void) = 0;
        virtual MINT32 reset(void) = 0;
        typedef struct reg_s {
            MUINT32 addr;
            MUINT32 val;
        } reg_t;
        //
        virtual MUINT32 readReg(MUINT32 addr) = 0;
        virtual MINT32 writeReg(
            MUINT32     addr,
            MUINT32     val) = 0;
        virtual MINT32 readRegs(
            reg_t*      pregs,
            MINT32      count) = 0;
        virtual MINT32 writeRegs(
            reg_t*      pregs,
            MINT32      count) = 0;
        virtual MINT32 holdReg(MBOOL isHold) = 0;
        virtual MINT32 dumpReg(void) = 0;
        virtual MINT32 setTgPhaseCounter(
            MUINT32     pcEn,
            MUINT32     mclkSel,
            MUINT32     clkCnt,
            MUINT32     clkPol,
            MUINT32     clkFallEdge,
            MUINT32     clkRiseEdge,
            MBOOL const      isPadPClkInv = false) = 0;
        virtual MINT32 setTg2PhaseCounter(
            MUINT32     pcEn,
            MUINT32     mclkSel,
            MUINT32     clkCnt,
            MUINT32     clkPol,
            MUINT32     clkFallEdge,
            MUINT32     clkRiseEdge) = 0;
        virtual MINT32 setIOConfig(MUINT32 ulCfg) = 0;
        virtual MINT32 setTgGrabRange(
            MUINT32     pixelStart,
            MUINT32     pixelEnd,
            MUINT32     lineStart,
            MUINT32     lineEnd) = 0;
        virtual MINT32 setTg2GrabRange(
            MUINT32     pixelStart,
            MUINT32     pixelEnd,
            MUINT32     lineStart,
            MUINT32     lineEnd) = 0;  
        virtual MINT32 setSensorModeCfg(
            MUINT32     hsPol,
            MUINT32     vsPol) = 0;
        virtual MINT32 setTG2SensorModeCfg(
            MUINT32     hsPol, 
            MUINT32     vsPol) = 0; 
        virtual MINT32 setResultWin(
            MUINT32     horiStart,
            MUINT32     horiEnd,
            MUINT32     vertStart,
            MUINT32     vertEnd,
            MUINT32     enable) = 0;
        virtual MINT32 setMemIOAddr(
            MUINT32     inAddr,
            MUINT32     outAddr) = 0;
        virtual MINT32 setViewFinderMode(
            MUINT32     spMode,
            MUINT32     spDelay) = 0;
        virtual MINT32 setTG2ViewFinderMode(
            MUINT32     spMode,
            MUINT32     spDelay) = 0; 
        virtual MINT32 setCamModulePath(
            MUINT32     inPathSel,
            MUINT32     inTypeSel,
            MUINT32     swapY,
            MUINT32     swapCbCr, 
            MUINT32     outPathType,
            MUINT32     outPathEn,
            MUINT32     inDataFormat) = 0;
        virtual MINT32 control(MUINT32 isEn) = 0;
        virtual MINT32 controlTG2(MUINT32 isEn) = 0;   
        virtual MINT32 setGammaEn(MUINT32 isEn) = 0;
        virtual MINT32 waitIrq(ISP_DRV_WAIT_IRQ_STRUCT* pWaitIrq) = 0;
        virtual MINT32 sendCommand(
            MINT32      cmd,
            MINT32      arg1 = 0,
            MINT32      arg2 = 0,
            MINT32      arg3 = 0) = 0;
        static MINT32 getFeatureProvider(void* const pOutBuf);
        virtual MINT32 initCSI2(MUINT32 csi2_en) = 0;
        virtual MINT32 setCSI2(
            MUINT32     dataTermDelay, 
            MUINT32     dataSettleDelay, 
            MUINT32     clkTermDelay, 
            MUINT32     vsyncType, 
            MUINT32     dlane1_en, 
            MUINT32     csi2_en,
            MUINT32     dataheaderOrder,
            MUINT32     dataFlow) = 0;
        virtual MINT32 setIODrivingCurrent(MUINT32 ioDrivingCurrent) = 0;
        virtual MINT32 setTG2IODrivingCurrent(MUINT32 ioDrivingCurrent) = 0;
        virtual MINT32 setMCLKEn(MUINT32 isEn) = 0;
        virtual MINT32 setCamClock(void) = 0;
        virtual void setSubsampleWidth(
            MBOOL       Enable,
            MUINT32     Input,
            MUINT32     Output) = 0;
        virtual void setSubsampleHeight(
            MBOOL       Enable,
            MUINT32     Input,
            MUINT32     Output) = 0;
        virtual void regCallback(pIspDrvCallback pFunc) = 0;
        virtual void setLogMask(MUINT32 LogMask) = 0;
};
//----------------------------------------------------------------------------
#endif

