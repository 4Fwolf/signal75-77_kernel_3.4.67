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
#ifndef ISP_DRV_IMP_H
#define ISP_DRV_IMP_H
//----------------------------------------------------------------------------
#include "isp_drv.h"
#include <cutils/xlog.h>
#include "isp_reg.h"
//----------------------------------------------------------------------------
using namespace android;
//----------------------------------------------------------------------------
#define ISP_DRV_CAM_MIPI_API        (1)
#define ISP_DRV_SCAM_ENABLE         (1)
#define ISP_DRV_USE_CAM2            (1)
//
#define ISP_DRV_DEV_NAME            "/dev/camera-isp"
#define ISP_DRV_DEV_NAME_MIPI       "/proc/clkmgr/mipi_test"
#define ISP_DRV_CAM_PLL_RANGE       (0x300)
#define ISP_DRV_SCAM_BASE_HW        (ISP_BASE_HW + 0x00008000)
#define ISP_DRV_SCAM_BASE_RANGE     (0x150) // Last SCAM register is 0x140, allocate more for safety.
#define ISP_DRV_CAM_APCONFIG_RANGE  (0x1000)
#define ISP_DRV_CAM_MMSYS_RANGE     (0x100)
#define ISP_DRV_CAM_MIPIRX_RANGE    (0x1000)
#define ISP_DRV_CAM_MIPIPLL_RANGE   (0x100)
#define ISP_DRV_CAM_GPIO_RANGE      (0x1000)
//----------------------------------------------------------------------------
class IspDrvImp : public IspDrv
{
    private:
        IspDrvImp();
        virtual ~IspDrvImp();
    //
    public:
        static IspDrv* getInstance(void);
        virtual void destroyInstance(void);
        virtual MINT32 init(void);
        virtual MINT32 initTG2(
            MUINT32    grabWidth, 
            MUINT32    grabHeight);  
        virtual MINT32 uninit(void);
        virtual MINT32 uninitTG2(void); 
        virtual MINT32 initPQ(void);
        virtual MINT32 uninitPQ(void);
        virtual MINT32 reset(void);
        virtual MUINT32 readReg(MUINT32 addr);
        virtual MINT32 writeReg(
            MUINT32     addr,
            MUINT32     val);
        virtual MINT32 readRegs(
            reg_t*      pregs, 
            MINT32      count);
        virtual MINT32 writeRegs(
            reg_t*      pregs, 
            MINT32      count);
        virtual MINT32 holdReg(MBOOL isHold);
        virtual MINT32 dumpReg(void);
        virtual MINT32 setTgPhaseCounter(
            MUINT32     pcEn,
            MUINT32     mclkSel,
            MUINT32     clkCnt,
            MUINT32     clkPol,
            MUINT32     clkFallEdge,
            MUINT32     clkRiseEdge,
            MBOOL const isPadPClkInv /*= false*/);
        virtual MINT32 setTg2PhaseCounter(
            MUINT32     pcEn,
            MUINT32     mclkSel,
            MUINT32     clkCnt,
            MUINT32     clkPol,
            MUINT32     clkFallEdge,
            MUINT32     clkRiseEdge);
        virtual MINT32 setIOConfig(MUINT32 ulCfg);
        virtual MINT32 setTgGrabRange(
            MUINT32     pixelStart,
            MUINT32     pixelEnd,
            MUINT32     lineStart,
            MUINT32     lineEnd);
        virtual MINT32 setTg2GrabRange(
            MUINT32     pixelStart,
            MUINT32     pixelEnd,
            MUINT32     lineStart,
            MUINT32     lineEnd);   
        virtual MINT32 setSensorModeCfg(
            MUINT32     hsPol,
            MUINT32     vsPol);
        virtual MINT32 setTG2SensorModeCfg(
            MUINT32     hsPol, 
            MUINT32     vsPol);   
        virtual MINT32 setResultWin(
            MUINT32     horiStart,
            MUINT32     horiEnd,
            MUINT32     vertStart,
            MUINT32     vertEnd,
            MUINT32     enable);
        virtual MINT32 setMemIOAddr(
            MUINT32     inAddr,
            MUINT32     outAddr);
        virtual MINT32 setViewFinderMode(
            MUINT32     spMode,
            MUINT32     spDelay);
        virtual MINT32 setTG2ViewFinderMode(
            MUINT32     spMode,
            MUINT32     spDelay);   
        virtual MINT32 setCamModulePath(
            MUINT32     inPathSel,
            MUINT32     inTypeSel,
            MUINT32     swapY,
            MUINT32     swapCbCr, 
            MUINT32     outPathType,
            MUINT32     outPathEn,
            MUINT32     inDataFormat);
        virtual MINT32 control(MUINT32 isEn);
        virtual MINT32 controlTG2(MUINT32 isEn);  
        virtual MINT32 setGammaEn(MUINT32 isEn);
        virtual MINT32 waitIrq(ISP_DRV_WAIT_IRQ_STRUCT* pWaitIrq);
        virtual MINT32 sendCommand(
            MINT32      cmd,
            MINT32      arg1 = 0,
            MINT32      arg2 = 0,
            MINT32      arg3 = 0);
        virtual MINT32 initCSI2(MUINT32 csi2_en);
        virtual MINT32 setCSI2(
            MUINT32     dataTermDelay, 
            MUINT32     dataSettleDelay, 
            MUINT32     clkTermDelay, 
            MUINT32     vsyncType, 
            MUINT32     dlane1_en, 
            MUINT32     csi2_en,
            MUINT32     dataheaderOrder,
            MUINT32     dataFlow);
        virtual MINT32 setIODrivingCurrent(MUINT32 ioDrivingCurrent);
        virtual MINT32 setTG2IODrivingCurrent(MUINT32 ioDrivingCurrent);
        virtual MINT32 setMCLKEn(MUINT32 isEn);
        virtual MINT32 setCamClock(void);
        virtual void setSubsampleWidth(
            MBOOL       Enable,
            MUINT32     Input,
            MUINT32     Output);
        virtual void setSubsampleHeight(
            MBOOL       Enable,
            MUINT32     Input,
            MUINT32     Output);
        virtual void regCallback(pIspDrvCallback pFunc);
        virtual void setLogMask(MUINT32 LogMask);
        //
    private:
        //
        volatile MINT32 mUsers;
        mutable Mutex mLock;
        MINT32 mfd;
        MINT32 mfd1;
        MUINT32 *mpIspHwRegAddr;
        MUINT32 *mpScamHwRegAddr; // Add for SCAM, can be deleted if SCAM is not needed.
        MUINT32 *mpPLLHwRegAddr;
        MUINT32 *mpIPllCon0RegAddr;
        MUINT32 *mpIPllCon1RegAddr;
        MUINT32 *mpIPllCon2RegAddr;
        MUINT32 *mpIPllCon3RegAddr;
        MUINT32 *mpCAMAPConRegAddr;    
        MUINT32 *mpCAMIODrvRegAddr;
        MUINT32 *mpCAM2IODrvRegAddr;    //add for N3D
        MUINT32 *mpMIPIAPConRegAddr;
        MUINT32 *mpCAMMMSYSRegAddr;
        MUINT32 *mpCSI2ClearGateRegAddr;
        MUINT32 *mpCSI2MIPIRegAddr;    
        MUINT32 *mpCSI2RxRegAddr;    
        MUINT32 *mpMIPIPLLRegAddr;    
        MUINT32 *mpGpioHwRegAddr;
        MUINT32 *mpTg2IOPinMuxCfgAddr;
        pIspDrvCallback mpCallback;
};
//----------------------------------------------------------------------------
#endif

