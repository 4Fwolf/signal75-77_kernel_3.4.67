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
#define LOG_TAG "IspDrv"
//
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
//
#include "isp_drv_imp.h"
#include "camera_isp.h"
#include "mtgpio.h"
//----------------------------------------------------------------------------
static MUINT32 IspDrvLogMask = ISP_DRV_LOG_MSG|ISP_DRV_LOG_WRN|ISP_DRV_LOG_ERR;
//----------------------------------------------------------------------------
#define LOG_MSG(fmt, arg...)\
    if(IspDrvLogMask & ISP_DRV_LOG_MSG) \
    { \
        XLOGD("[%s]"fmt, __FUNCTION__, ##arg); \
    }
#define LOG_WRN(fmt, arg...) \
    if(IspDrvLogMask & ISP_DRV_LOG_WRN) \
    { \
        XLOGW("[%s]Warning(%5d):"fmt, __FUNCTION__, __LINE__, ##arg); \
    }
#define LOG_ERR(fmt, arg...) \
    if(IspDrvLogMask & ISP_DRV_LOG_ERR) \
    { \
        XLOGE("[%s]Err(%5d):"fmt, __FUNCTION__, __LINE__, ##arg); \
    }
//----------------------------------------------------------------------------
IspDrv* IspDrv::createInstance(void)
{
    return IspDrvImp::getInstance();
}
//----------------------------------------------------------------------------
IspDrv* IspDrvImp::getInstance(void)
{
    //LOG_MSG("");
    static IspDrvImp singleton;
    return &singleton;
}
//----------------------------------------------------------------------------
void IspDrvImp::destroyInstance(void) 
{
}
//----------------------------------------------------------------------------
IspDrvImp::IspDrvImp():IspDrv()
{
    //LOG_MSG("");
    //
    mUsers = 0;
    mfd = -1;
    mpIspHwRegAddr = NULL;
}
//----------------------------------------------------------------------------
IspDrvImp::~IspDrvImp()
{
    //LOG_MSG("");
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::init(void)
{
    LOG_MSG("mUsers(%d)", mUsers);
    MINT32 pll_base_hw = 0xC0007000;
    MINT32 cam_ap_config = 0xC0001000;
    MINT32 mmsys1_cg_clr0 = 0xC2080000;
    MINT32 mipiRx_config = 0xC20A3000;
    MINT32 gpio_base_addr = 0xC0005000;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUsers > 0)
    {
        LOG_MSG("Has inited");
        android_atomic_inc(&mUsers);
        return 0;
    }
    // Open isp driver
    mfd = open(ISP_DRV_DEV_NAME, O_RDWR);
    if(mfd < 0)
    {
        LOG_ERR("error open kernel driver, errno(%d):%s", errno, strerror(errno));
        return -1;
    }
    //mmap isp reg
    mpIspHwRegAddr = (MUINT32 *) mmap(0, ISP_BASE_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, ISP_BASE_HW);
    if(mpIspHwRegAddr == MAP_FAILED)
    {
        LOG_ERR("mmap err, errno(%d):%s", errno, strerror(errno));
        return -2;
    }
    // mmap pll reg
    mpPLLHwRegAddr = (MUINT32 *) mmap(0, ISP_DRV_CAM_PLL_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, pll_base_hw);
    if(mpPLLHwRegAddr == MAP_FAILED)
    {
        LOG_ERR("mmap err, errno(%d):%s", errno, strerror(errno));
        return -3;
    }
    // mmap scam reg
    #if (ISP_DRV_SCAM_ENABLE)
    mpScamHwRegAddr = (MUINT32 *) mmap(0, ISP_DRV_SCAM_BASE_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, ISP_DRV_SCAM_BASE_HW);
    if(mpScamHwRegAddr == MAP_FAILED)
     {
        LOG_ERR("mmap err, errno(%d):%s", errno, strerror(errno));
        return -6;
    }
    #endif
    // mmap ap config reg
    mpCAMAPConRegAddr = (MUINT32 *) mmap(0, ISP_DRV_CAM_APCONFIG_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, cam_ap_config);
    if(mpCAMAPConRegAddr == MAP_FAILED)
    {
        LOG_ERR("mmap err, errno(%d):%s", errno, strerror(errno));
        return -4;
    }
    // mmap csi2 clear gating reg
    mpCAMMMSYSRegAddr = (MUINT32 *) mmap(0, ISP_DRV_CAM_MMSYS_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, mmsys1_cg_clr0);
    if(mpCAMMMSYSRegAddr == MAP_FAILED)
    {
        LOG_ERR("mmap err, errno(%d):%s", errno, strerror(errno));
        return -5;
    }
    // mipi rx address
    mpCSI2MIPIRegAddr = (MUINT32 *) mmap(0, ISP_DRV_CAM_MIPIRX_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, mipiRx_config);
    if(mpCSI2MIPIRegAddr == MAP_FAILED)
    {
        LOG_ERR("mmap err, errno(%d):%s", errno, strerror(errno));
        return -5;
    }    
    //gpio 
    mpGpioHwRegAddr = (MUINT32 *) mmap(0, ISP_DRV_CAM_GPIO_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, gpio_base_addr);
    if(mpGpioHwRegAddr == MAP_FAILED)
    {
        LOG_ERR("mmap err, errno(%d):%s", errno, strerror(errno));
        return -7;
    }
    //
    mpIPllCon0RegAddr = mpPLLHwRegAddr + (0x140 /4);
    mpIPllCon1RegAddr = mpPLLHwRegAddr + (0x144 /4);
    mpIPllCon2RegAddr = mpPLLHwRegAddr + (0x148 /4);
    mpIPllCon3RegAddr = mpPLLHwRegAddr + (0x14C /4);
    mpMIPIAPConRegAddr = mpCAMAPConRegAddr + (0x890 / 4);
    mpCAMIODrvRegAddr = mpCAMAPConRegAddr + (0x824 / 4);    
    mpCAM2IODrvRegAddr = mpCAMAPConRegAddr + (0x830 / 4);   // added for N3D
    mpCSI2ClearGateRegAddr = mpCAMMMSYSRegAddr + (0x24/4);
    mpCSI2RxRegAddr = mpCSI2MIPIRegAddr + (0x800/4);
    mpMIPIPLLRegAddr = mpPLLHwRegAddr + (0x44 /4);
    // Disable 3A functions
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    ISP_REG(pisp, CAM_CTRL1) = 0x80000100;
    ISP_REG(pisp, CAM_CTRL2) = 0x00000000;
    ISP_REG(pisp, CAM_RAWGAIN0) = 0x00D000D0;
    ISP_REG(pisp, CAM_RAWGAIN1) = 0x00D000D0;
    // Fill default gamma table
    ISP_REG(pisp, CAM_GMA_REG1) = 0x46321e09;
    ISP_REG(pisp, CAM_GMA_REG2) = 0x8c7c6b59;
    ISP_REG(pisp, CAM_GMA_REG3) = 0xbfb4a79a;
    ISP_REG(pisp, CAM_GMA_REG4) = 0xe6ddd3ca;
    ISP_REG(pisp, CAM_GMA_REG5) = 0xf8f7f2ed;
    //
    ISP_BITS(pisp, CAM2_DROP_HSUB, H_SUB_EN) = 0;
    ISP_BITS(pisp, CAM2_DROP_VSUB, V_SUB_EN) = 0;
    //
    ISP_REG(pisp, CAM_PHSCNT) = 0x01010011;
    ISP_REG(pisp, CAM_TG2_PH_CNT) = 0x00000000;
    ISP_REG(pisp, CAM_PATH) = 0x30;
    //
    ISP_BITS(pisp, CAM_INTEN, VSYNC_INT_EN) = 1; //always enable VD for bridge 3D application.
    // Disable Camera PLL
    (*mpIPllCon0RegAddr) |= 0x01; //Power Down
    (*mpIPllCon1RegAddr) &= (~0x30);
    // Open mipi power driver
    #if ISP_DRV_CAM_MIPI_API
    mfd1 = open(ISP_DRV_DEV_NAME_MIPI, O_RDWR);
    //
    if(mfd1 < 0)
    {
        LOG_ERR("error open kernel driver, errno(%d):%s", errno, strerror(errno));
        return -1;
    }
    #endif
    // For FPGA
    ISP_BITS(pisp, CAM_CSMODE, RST) = 1;
    //
    mpCallback = NULL;
    // 
    android_atomic_inc(&mUsers);
    //
    return 0;
}


/*******************************************************
* Functionality : TG2 initialize
*******************************************************/
MINT32 IspDrvImp::initTG2(
    MUINT32 grabWidth, 
    MUINT32 grabHeight)
{
    LOG_MSG("E : grabWidth=%u, grabHeight=%u",grabWidth, grabHeight);
    
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    
    // === View finder mode control. CAM_BASE+0x0C0C ===
    
    ISP_BITS(pisp, CAM_TG2_VF_CON, VFDATA_EN   ) = 0;	// Take Picture Request
    ISP_BITS(pisp, CAM_TG2_VF_CON, SINGLE_MODE ) = 0;	// 0(continuous mode, PV), 1(single mode, CAP). Single Mode

    // === Sensor mode configure. CAM_BASE+0x0C08. ===

    setTG2SensorModeCfg(0, 0);
    
    // === CTL_MOD_EN. CAM_BASE+0x0D04 ===
    
    ISP_BITS(pisp, CAM_CTL_MOD_EN, TG2_EN   ) = 1; // TG2 enable     
    ISP_BITS(pisp, CAM_CTL_MOD_EN, C24D_2_EN) = 1; // C24D enabble at TG2 path    
    ISP_BITS(pisp, CAM_CTL_MOD_EN, FIFO_2_EN) = 1; // FUFO 2 enable

    // === ISP input/output format. CAM_BASE+0x0D08 ===
    
    ISP_BITS(pisp, CAM_CTL_FMT_SEL, TG2_FMT) = 3;  // 3(YUV422 8 bit)/5(RGB565 format)/7(JPEG format). TG2 format.

    // === ISP YUV/RGB SWAP. CAM_BASE+0x0D0C ===
    
    ISP_BITS(pisp, CAM_CTL_SWAP, TG2_SW) = 0;   // bit0 is YC swap, bit1 is UV or RB swap. For YUV format: 0(UYVY)/1(YUYV)/2(VYUY)/3(YVYU). For RGB format: 0,1(RGB)/2,3(BGR)

    // === ISP selection. CAM_BASE+0x0D10 ===

    //crz
    //ISP_BITS(pisp, CAM_CTL_SEL, CRZ_DISCONN) = 1;    // 0(connrect CRZ)/1(CRZ disconnection, only for debug). CRZ disconnection.                    
    //ISP_BITS(pisp, CAM_CTL_SEL, PRZ_DISCONN) = 0;    // 0(connrect CRZ)/1(CRZ disconnection, only for debug). PRZ disconnection.                    
    //ISP_BITS(pisp, CAM_CTL_SEL, CRZ_SEL         ) = 1;    // 0(CRZ select TG1)/1(CRZ select TG2). CRZ input selection.                                   

    // prz
    ISP_BITS(pisp, CAM_CTL_SEL, CRZ_DISCONN) = 1;    // 0(connrect CRZ)/1(CRZ disconnection, only for debug). CRZ disconnection.                    
    ISP_BITS(pisp, CAM_CTL_SEL, PRZ_DISCONN) = 0;    // 0(connrect CRZ)/1(CRZ disconnection, only for debug). PRZ disconnection.                    
    ISP_BITS(pisp, CAM_CTL_SEL, CRZ_SEL    ) = 0;    // 0(CRZ select TG1)/1(CRZ select TG2). CRZ input selection.                                   

    ISP_BITS(pisp, CAM_CTL_SEL, COLOR2_SEL ) = 1;    // 0(choose TG output , c24d is duplicate), 1(choose Color output, c24d is bilinear). Color sel.
    ISP_BITS(pisp, CAM_CTL_SEL, PCA_SEL    ) = 0;    // 0(PCA at TG1 path), 1(PCA at TG2 path). PCA sel

    // === Camera to RZ Contro Register. CAM_BASE+0x0D14 ===
    
    ISP_BITS(pisp, CAM_CTL_CRZ, CAMCRZ_INIT_EN    ) = 1;    // Camera to CRZ frame initialization scheme enable. In frame initialization, camera will block signal to CRZ and assert initial signal to CRZ when pixel dropped
    ISP_BITS(pisp, CAM_CTL_CRZ, CAMCRZ_INIT_PERIOD) = 10;   // Camera to CRZ initialization signal active period
    ISP_BITS(pisp, CAM_CTL_CRZ, CAMPRZ_INIT_EN    ) = 1;    // Camera to CRZ frame initialization scheme enable. In frame initialization, camera will block signal to CRZ and assert initial signal to CRZ when pixel dropped
    ISP_BITS(pisp, CAM_CTL_CRZ, CAMPRZ_INIT_PERIOD) = 10;   // Camera to CRZ initialization signal active period

    // === ISP Interrupt enable. CAM_BASE+0x0D20 ===

    ISP_BITS(pisp, CAM_CTL_INT_EN, CRZ_OVRUN_EN   ) = 1;    // CRZ over run enable.
    ISP_BITS(pisp, CAM_CTL_INT_EN, COLOR2_DON_EN  ) = 1;    // Color 2 DONE interrupt enable.
    ISP_BITS(pisp, CAM_CTL_INT_EN, TG2_GRAB_ERR_EN) = 1;    // TG2 ERR enable.
    ISP_BITS(pisp, CAM_CTL_INT_EN, EXPDON2_EN     ) = 1;    // Exposusre2 done interrupt enable.
    ISP_BITS(pisp, CAM_CTL_INT_EN, TG2_EN2        ) = 0;    // TG2 interrupt2 enable.
    ISP_BITS(pisp, CAM_CTL_INT_EN, TG2_EN1        ) = 0;    // TG2 interrupt1 enable.
    ISP_BITS(pisp, CAM_CTL_INT_EN, VS2_EN         ) = 1;    // Vsync2 interrupt enable.

    // === TG phase counter register. CAM_BASE+0x0C00 ===

    // have some same thing in setTG2PhaseCounter
    ISP_BITS(pisp, CAM_TG2_PH_CNT, PCEN        ) = 1;   // TG phase counter enable control
    ISP_BITS(pisp, CAM_TG2_PH_CNT, ADCLK_EN    ) = 1;   // Enable sensor master clock (mclk) output to sensor. Note that to set sensor master clock driving setting,  
    ISP_BITS(pisp, CAM_TG2_PH_CNT, CAM_PCLK_INV) = 0;   // For SCAM must set to 0. // Pixel clock inverse in CAM.
    ISP_BITS(pisp, CAM_TG2_PH_CNT, EXT_PWRDN   ) = 0;   // sensor power down.
    ISP_BITS(pisp, CAM_TG2_PH_CNT, EXT_RST     ) = 1;   // sensor reset.
    ISP_BITS(pisp, CAM_TG2_PH_CNT, TGCLK_SEL   ) = 0;   // 0(isp_clk)/1(cam_pll)/2(3rd clock source). Sensor master clock selection.

    // === TG Sensor clock divider. CAM_BASE+0x0C04 ===

    // have some same thing in setTG2PhaseCounter
    ISP_BITS(pisp, CAM_TG2_SEN_CK, CLKCNT) = 1;    // Sensor master clock frequency divider control. Sensor master clock will be ISP_clock/CLKCNT, where CLKCNT >=1
    ISP_BITS(pisp, CAM_TG2_SEN_CK, CLKRS ) = 1;    // Sensor master clock rising edge control
    ISP_BITS(pisp, CAM_TG2_SEN_CK, CLKFL ) = 1;    // Sensor master clock falling edge control

    // === SENSOR GRAB PIXEL Start/End. CAM_BASE+0x0C10 ===

    ISP_BITS(pisp, CAM_TG2_SEN_GRAB_PXL, PXL_E) = 2 * grabWidth;   // Grab end pixel clock number (first pixel start from 0).
    ISP_BITS(pisp, CAM_TG2_SEN_GRAB_PXL, PXL_S) = 0;               // Grab start pixel clock number (first pixel start from 0).

    // === SENSOR GRAB LINE Start/End. CAM_BASE+0x0C14 ===
    
    ISP_BITS(pisp, CAM_TG2_SEN_GRAB_LIN, LIN_E) = grabHeight;  // Grab end line number (first line start from 0).
    ISP_BITS(pisp, CAM_TG2_SEN_GRAB_LIN, LIN_S) = 0;           // Grab start line number (first line start from 0).


    // === TG path configure. CAM_BASE+0x0C18 ===

    ISP_BITS(pisp, CAM_TG2_PATH_CFG, YUV_U2S_DIS) = 1; // YUV sensor unsigned to signed disable.
    ISP_BITS(pisp, CAM_TG2_PATH_CFG, SEN_IN_LSB ) = 2; // 0(datain[11:0] as tg data input)/1(datain[9:0] as tg data input)/2(datain[7:0] as tg data input). Sensor input data bit order selection, used in ISP YUV422/RGB888/RGB565/Bayer format. Data input in ISP is 12-bit, datain[11:0].

    // === TG interrupt 1 setting. CAM_BASE+0x0C20 ===

    ISP_BITS(pisp, CAM_TG2_INT1, TG_INT1_PXLNO ) = grabWidth  >> 2; // TG interrupt1 pixel number.
    ISP_BITS(pisp, CAM_TG2_INT1, TG_INT1_LINENO) = grabHeight >> 2; // TG interrupt1 line number.
    
    // === TG interrupt 2 setting. CAM_BASE+0x0C24 ===
    
    ISP_BITS(pisp, CAM_TG2_INT2, TG_INT2_PXLNO ) = grabWidth  >> 1; // TG interrupt2 pixel number.
    ISP_BITS(pisp, CAM_TG2_INT2, TG_INT2_LINENO) = grabHeight >> 1; // TG interrupt2 line number.

    // === TG error control. CAM_BASE+0x0C34 ===
    
    ISP_BITS(pisp, CAM_TG2_ERR_CTL, REZ_OVRUN_FLIMIT_EN) = 1;   // Camera to CRZ interface overrun frame count limit enable.
    ISP_BITS(pisp, CAM_TG2_ERR_CTL, REZ_OVRUN_FLIMIT_NO) = 14;  // Camer to CRZ interface overrun frame count limit number, max value=14.
    ISP_BITS(pisp, CAM_TG2_ERR_CTL, GRAB_ERR_EN        ) = 1;   // TG grab error frame limit enable.
    ISP_BITS(pisp, CAM_TG2_ERR_CTL, GRAB_ERR_FLIMIT_EN ) = 1;   // TG grab error frame limit enable. only for single mode.
    ISP_BITS(pisp, CAM_TG2_ERR_CTL, GRAB_ERR_FLIMIT_NO ) = 14;  // TG grab error frame limit number. only for single mode, max value=14.

    // === Test model control register. CAM_BASE+0x0C60 ===

    ISP_BITS(pisp, CAM_TG2_TM_CTL, TM_VSYNC) = 1;   // VSYNC high duration in line unit (TM_DUMMYPXL + PIXEL).
    ISP_BITS(pisp, CAM_TG2_TM_CTL, TM_PAT  ) = 0;   // 0(white)/1(yellow)/2(cyan)/3(green)/4(magenta)/5(red)/6(blue)/7(black)/8(horizontal gray level (Unit 1))/9(horizontal gray level (Unit 4))/10(horizontal gray level (Take 1024 pixel as one period) (only bayer))/11(vertical gray level (Unit 1))/12(static horizontal color bar)/13(static vertical color bar (only bayer))/14(R,G,B,W flash every two frame (only bayer))/15(Dynamic horizontal colorbar (only bayer). Test model decision.
    ISP_BITS(pisp, CAM_TG2_TM_CTL, TM_EN   ) = 0;   // 0(disable)/1(enable). Test model enable.
    
    ISP_BITS(pisp, CAM_TG2_SEN_MODE, CMOS_EN) = 1;	// Sensor process enable.
    
    return 0;
}



//----------------------------------------------------------------------------
MINT32 IspDrvImp::uninit(void)
{
    LOG_MSG("mUsers(%d)", mUsers);
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUsers <= 0)
    {
        // No more users
        return 0;
    }
    // More than one user
    android_atomic_dec(&mUsers);
    //
    if(mUsers == 0)
    {
        // Last user
        ISP_REG(pisp, CAM_INTEN) = 0;
        setCSI2(0, 0, 0, 0, 0, 0, 0, 0); // disable CSI2        
        munmap(mpIspHwRegAddr, ISP_BASE_RANGE);
        mpIspHwRegAddr = NULL;
        // Disable Camera PLL
        (*mpIPllCon0RegAddr) |= 0x01; //Power Down
        munmap(mpPLLHwRegAddr, ISP_DRV_CAM_PLL_RANGE);
        mpPLLHwRegAddr = NULL;
        //
        #if (ISP_DRV_SCAM_ENABLE)
        munmap(mpScamHwRegAddr, ISP_DRV_SCAM_BASE_RANGE);
        mpScamHwRegAddr = NULL;
        #endif
        //
        munmap(mpCAMAPConRegAddr, ISP_DRV_CAM_APCONFIG_RANGE);
        mpCAMAPConRegAddr = NULL;
        munmap(mpCAMMMSYSRegAddr, ISP_DRV_CAM_MMSYS_RANGE);
        mpCAMMMSYSRegAddr = NULL;
        munmap(mpCSI2MIPIRegAddr, ISP_DRV_CAM_MIPIRX_RANGE);
        mpCSI2MIPIRegAddr = NULL;
        munmap(mpGpioHwRegAddr, ISP_DRV_CAM_GPIO_RANGE);
        mpGpioHwRegAddr = NULL;
        //
        if(mfd > 0)
        {
            close(mfd);
            mfd = -1;
        }
        //
        #if ISP_DRV_CAM_MIPI_API
        if(mfd1 > 0) {
            close(mfd1);
            mfd1 = -1;
        }
        #endif
    }
    else
    {
        LOG_MSG("Still users(%d)",mUsers);
    }
    //
    return 0;
}


/***************************************************************************
* Functionality : TG2 uninitialize
***************************************************************************/
MINT32 IspDrvImp::uninitTG2(void)
{
    LOG_MSG("Uninitialize TG2");

    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    ISP_BITS(pisp, CAM_CTL_MOD_EN, TG2_EN) = 0;  // TG2 enable
    ISP_BITS(pisp, CAM_TG2_PH_CNT, PCEN)   = 0;  // TG phase counter enable control    
    ISP_BITS(pisp, CAM_TG2_SEN_MODE, CMOS_EN     ) = 0;	// Sensor process enable.
    
    return 0;
}


//----------------------------------------------------------------------------
MINT32 IspDrvImp::initPQ(void)
{
    LOG_MSG("mUsers(%d)", mUsers);
    //
    MINT32 pll_base_hw = 0xC0007000;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUsers > 0)
    {
        LOG_MSG("Has inited");
        android_atomic_inc(&mUsers);
        return 0;
    }
    // Open isp driver
    mfd = open(ISP_DRV_DEV_NAME, O_RDWR);
    if(mfd < 0)
    {
        LOG_ERR("error open kernel driver, errno(%d):%s", errno, strerror(errno));
        return -1;
    }
    // mmap isp reg
    mpIspHwRegAddr = (MUINT32 *) mmap(0, ISP_BASE_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, ISP_BASE_HW);
    if(mpIspHwRegAddr == MAP_FAILED)
    {
        LOG_ERR("mmap err, errno(%d):%s", errno, strerror(errno));
        return -2;
    }
    // mmap pll reg
    mpPLLHwRegAddr = (MUINT32 *) mmap(0, ISP_DRV_CAM_PLL_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, pll_base_hw);
    if(mpPLLHwRegAddr == MAP_FAILED)
    {
        LOG_ERR("mmap err, errno(%d):%s", errno, strerror(errno));
        return -3;
    }
    //
    mpIPllCon0RegAddr = mpPLLHwRegAddr + (0x140 /4);
    mpIPllCon1RegAddr = mpPLLHwRegAddr + (0x144 /4);
    // Disable 3A functions
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    ISP_REG(pisp, CAM_CTRL1) = 0x80000100;
    ISP_REG(pisp, CAM_CTRL2) = 0x00000000;
    ISP_REG(pisp, CAM_RAWGAIN0) = 0x00D000D0;
    ISP_REG(pisp, CAM_RAWGAIN1) = 0x00D000D0;
    // Fill default gamma table
    ISP_REG(pisp, CAM_GMA_REG1) = 0x46321e09;
    ISP_REG(pisp, CAM_GMA_REG2) = 0x8c7c6b59;
    ISP_REG(pisp, CAM_GMA_REG3) = 0xbfb4a79a;
    ISP_REG(pisp, CAM_GMA_REG4) = 0xe6ddd3ca;
    ISP_REG(pisp, CAM_GMA_REG5) = 0xf8f7f2ed;
    //
    ISP_BITS(pisp, CAM2_DROP_HSUB, H_SUB_EN) = 0;
    ISP_BITS(pisp, CAM2_DROP_VSUB, V_SUB_EN) = 0;
    //
    ISP_REG(pisp, CAM_PHSCNT) = 0x01010011;
    ISP_REG(pisp, CAM_TG2_PH_CNT) = 0x00000000;
    ISP_REG(pisp, CAM_PATH) = 0x30;
    // Disable Camera PLL
    (*mpIPllCon0RegAddr) |= 0x01; //Power Down
    (*mpIPllCon1RegAddr) &= (~0x30);
    // For FPGA
    ISP_BITS(pisp, CAM_CSMODE, RST) = 1;
    // 
    android_atomic_inc(&mUsers);
    //
    return 0;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::uninitPQ(void)
{
    LOG_MSG("mUsers(%d)", mUsers);
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUsers <= 0)
    {
        // No more users
        return 0;
    }
    // More than one user
    android_atomic_dec(&mUsers);
    //
    if(mUsers == 0)
    {
        // Last user
        munmap(mpIspHwRegAddr, ISP_BASE_RANGE);
        mpIspHwRegAddr = NULL;
        // Disable Camera PLL
        (*mpIPllCon0RegAddr) |= 0x01; //Power Down
        munmap(mpPLLHwRegAddr, ISP_DRV_CAM_PLL_RANGE);
        mpPLLHwRegAddr = NULL;
        //
        if(mfd > 0)
        {
            close(mfd);
            mfd = -1;
        }
    }
    else
    {
        LOG_MSG("Still users(%d)",mUsers);
    }
    //
    return 0;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::setTgPhaseCounter(
    MUINT32     pcEn,
    MUINT32     mclkSel,
    MUINT32     clkCnt,
    MUINT32     clkPol,
    MUINT32     clkFallEdge,
    MUINT32     clkRiseEdge,
    MBOOL const isPadPClkInv /*= false*/)
{
    MINT32 ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    //
    LOG_MSG("pcEn(%ld), mclkSel(%ld), clkCnt(%ld), clkPol(%ld), clkFallEdge(%ld), clkRiseEdge(%ld), isPadPClkInv(%d)",
            pcEn,mclkSel,clkCnt,clkPol,clkFallEdge,clkRiseEdge,isPadPClkInv);
    // Enable Camera PLL first
    if(mclkSel == ISP_DRV_CAM_PLL_48_GROUP)
    {
        //120.25MHz
        (*mpIPllCon0RegAddr) = 0x2441;
        (*mpIPllCon1RegAddr) |= 0x20;
        (*mpIPllCon0RegAddr) &= (~0x1);
    }
    else
    if(mclkSel == ISP_DRV_CAM_PLL_52_GROUP)
    {
        //52MHz
        (*mpIPllCon0RegAddr) = 0x1F41;
        (*mpIPllCon1RegAddr) |= 0x20;
        (*mpIPllCon0RegAddr) &= (~0x1);
    }
    //
    ISP_BITS(pisp, CAM_PHSCNT, PAD_PCLK_INV) = isPadPClkInv ? 1 : 0;
    ISP_BITS(pisp, CAM_PHSCNT, CLKRS) = clkRiseEdge;
    ISP_BITS(pisp, CAM_PHSCNT, CLKFL) = clkFallEdge;
    ISP_BITS(pisp, CAM_PHSCNT, CLKCNT) = clkCnt - 1;
    ISP_BITS(pisp, CAM_PHSCNT, CLKFL) = clkCnt >> 1;
    ISP_BITS(pisp, CAM_PHSCNT, CLKFL_POL) = (clkCnt & 0x1) ? 1 : 0;
    // PXCLK_IN must be 1, spec error
    ISP_BITS(pisp, CAM_PHSCNT, PXCLK_IN) = 1;
    ISP_BITS(pisp, CAM_PHSCNT, CLKPOL) = clkPol;
    // mclkSel, 0: 122.88MHz, (others: Camera PLL) 1: 120.3MHz, 2: 52MHz
    ISP_BITS(pisp, CAM_PHSCNT, TGCLK_SEL) = mclkSel > 0 ? 1 : 0;
    ISP_BITS(pisp, CAM_PHSCNT, CLKEN) = 1;
    ISP_BITS(pisp, CAM_PHSCNT, PCEN) = pcEn;
    //New bit for MT6575
    ISP_BITS(pisp, CAM_PHSCNT, ASYNC_INF_SEL) = 1;
    // Wait 1ms for PLL stable
    usleep(1000);
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::setTg2PhaseCounter(
    MUINT32     pcEn,
    MUINT32     mclkSel,
    MUINT32     clkCnt,
    MUINT32     clkPol,
    MUINT32     clkFallEdge,
    MUINT32     clkRiseEdge)
{
    MINT32 ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    //
    LOG_MSG("pcEn(%ld), mclkSel(%ld), clkCnt(%ld), clkPol(%ld), clkFallEdge(%ld), clkRiseEdge(%ld), isPadPClkInv(%d)",
            pcEn,mclkSel,clkCnt,clkPol,clkFallEdge,clkRiseEdge);
    // Enable Camera PLL first
    if(mclkSel == ISP_DRV_CAM_PLL_48_GROUP)
    {
        //120.25MHz
        (*mpIPllCon0RegAddr) = 0x2441;
        (*mpIPllCon1RegAddr) |= 0x20;
        (*mpIPllCon0RegAddr) &= (~0x1);
    }
    else
    if(mclkSel == ISP_DRV_CAM_PLL_52_GROUP)
    {
        //52MHz
        (*mpIPllCon0RegAddr) = 0x1F41;
        (*mpIPllCon1RegAddr) |= 0x20;
        (*mpIPllCon0RegAddr) &= (~0x1);
    }
    // TG phase counter register (0x0C00)
    ISP_BITS(pisp, CAM_TG2_PH_CNT, PCEN        ) = pcEn;    // TG phase counter enable control
    ISP_BITS(pisp, CAM_TG2_PH_CNT, ADCLK_EN    ) = 1;    // Enable sensor master clock (mclk) output to sensor. Note that to set sensor master clock driving setting, 
    ISP_BITS(pisp, CAM_TG2_PH_CNT, CLKPOL      ) = clkPol;    // Sensor master clock polarity control
    ISP_BITS(pisp, CAM_TG2_PH_CNT, CAM_PCLK_INV) = 0;    // Pixel clock inverse in CAM.
    ISP_BITS(pisp, CAM_TG2_PH_CNT, PAD_PCLK_INV) = 0;    // Pixel clock inverse in PAD side
    ISP_BITS(pisp, CAM_TG2_PH_CNT, EXT_PWRDN   ) = 0;    // sensor power down
    ISP_BITS(pisp, CAM_TG2_PH_CNT, EXT_RST     ) = 1;    // sensor reset
    ISP_BITS(pisp, CAM_TG2_PH_CNT, CLKFL_POL   ) = (clkCnt & 0x1) ? 0 : 1;    // Sensor clock falling edge polarity
    ISP_BITS(pisp, CAM_TG2_PH_CNT, TGCLK_SEL   ) = mclkSel > 0 ? 1 : 0;   // 1;    // Sensor master clock selection (0: isp_clk, 1: cam_pll, 2: 3rd clock source) // mclkSel, 0: 122.88MHz, (others: Camera PLL) 1: 120.3MHz, 2: 52MHz
    // TG Sensor clock divider (0x0C04)
    ISP_BITS(pisp, CAM_TG2_SEN_CK, CLKCNT) = clkCnt-1;       // Sensor master clock falling edge control
    ISP_BITS(pisp, CAM_TG2_SEN_CK, CLKRS ) = clkRiseEdge;  // Sensor master clock rising edge control
    ISP_BITS(pisp, CAM_TG2_SEN_CK, CLKFL ) = clkCnt >> 1;  // "Sensor master clock frequency divider controlSensor master clock will be ISP_clock/CLKCNT, where CLKCNT >=1."
    //Wait 1ms for PLL stable 
    usleep(1000);
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::setIOConfig(MUINT32 ulCfg)
{
    MINT32 fd, ret;
    MUINT32 registerArry[14] = {19,21,22,26,27,30,32,34,35,38,40,41,44,45};
    
    //set pinmux for TG2 parallel sensor
    #if ISP_DRV_USE_CAM2
    if(ulCfg != 0)
    {
        fd = open("dev/mtgpio", O_RDONLY);
        if(fd < 0)
        {
            LOG_ERR("error open kernel driver dev/mtgpio, errno(%d):%s", errno, strerror(errno));            
        }

        //====== config gpio mode for CAM2 ======

        //0xC30, GPIO19, CM2PCLK [14:12], mode 3
        //0xC40, GPIO21, CM2VREF [5:3], mode 3
        //0xC40, GPIO22, CM2DAT5 [8:6], mode 3
        //0xC50, GPIO26, CM2DAT7 [5:3], mode 3
        //0xC50, GPIO27, CM2DAT4 [8:6], mode 3
        //0xC60, GPIO30, CM2HREF [2:0], mode 3
        //0xC60, GPIO32, CM2DAT6 [8:6], mode 3
        //0xC60, GPIO34, CM2RST [14:12], mode 3
        //0xC70, GPIO35, CM2DAT2 [2:0], mode 3
        //0xC70, GPIO38, CM2PDN [11:9], mode 3
        //0xC80, GPIO40, CM2DAT3 [2:0], mode 3
        //0xC80, GPIO41, CM2MCLK [5:3], mode 3
        //0xC80, GPIO44, CM2DAT0 [14:12], mode 3
        //0xC90, GPIO45, CM2DAT1 [2:0], mode 3
        
        for(int i = 0; i < 14; ++i)
        {
            ret = ioctl(fd, GPIO_IOCTMODE3, registerArry[i]);  
            if(ret != 0)
            {
                LOG_ERR("GPIO%d mode3 setting fail",registerArry[i]);
                break;
            }
            else
            {
                LOG_MSG("GPIO%d mode3 setting success",registerArry[i]);
            }
        }        

        //====== config pull sell ======

        // disable GPIO19/21/22/26/27/30 [3]/[5]/[6]/[10]/[11]/[14]
        // disable GPIO32//35//40//44/45 [0]/[3]/[8]/[12]/[13]
        
        for(int i = 0; i < 14; ++i)
        {
            if(i == 7 || i == 9 || i == 11)
            {
                continue;
            }
            else
            {
                ret = ioctl(fd, GPIO_IOCSPULLDISABLE, registerArry[i]);  
                if(ret != 0)
                {
                    LOG_ERR("GPIO%d pull disable setting fail",registerArry[i]);
                    break;
                }
                else
                {
                    LOG_MSG("GPIO%d pull disable setting success",registerArry[i]);
                }
            }
        }              

        //====== Close GPIO Driver ======
        
        if(close(fd) < 0)
        {
            LOG_ERR("error clsoe kernel driver dev/mtgpio, errno(%d):%s", errno, strerror(errno));            
        }
        
        #if 0
        //config gpio mode for CAM2
        mpTg2IOPinMuxCfgAddr = mpGpioHwRegAddr + (0xC30/4); //GPIO19 CM2PCLK [14:12]
        (*mpTg2IOPinMuxCfgAddr) &= (~0x7000);
        (*mpTg2IOPinMuxCfgAddr) |= 0x3000;
        mpTg2IOPinMuxCfgAddr = mpGpioHwRegAddr + (0xC40/4); //GPIO21/22 CM2VREF/CM2DAT5 [5:3]/[8:6]
        (*mpTg2IOPinMuxCfgAddr) &= (~0x1F8);
        (*mpTg2IOPinMuxCfgAddr) |= 0xD8;
        mpTg2IOPinMuxCfgAddr = mpGpioHwRegAddr + (0xC50/4); //GPIO26/27 CM2DAT7/CM2DAT4 [5:3]/[8:6]
        (*mpTg2IOPinMuxCfgAddr) &= (~0x1F8);
        (*mpTg2IOPinMuxCfgAddr) |= 0xD8;
        mpTg2IOPinMuxCfgAddr = mpGpioHwRegAddr + (0xC60/4); //GPIO30/32/34 CM2HREF/CM2DAT6/CM2RST [2:0]/[8:6]/[14:12]
        (*mpTg2IOPinMuxCfgAddr) &= (~0x71C7);
        (*mpTg2IOPinMuxCfgAddr) |= 0x30C3;
        mpTg2IOPinMuxCfgAddr = mpGpioHwRegAddr + (0xC70/4); //GPIO35/38 CM2DAT2/CM2PDN [2:0]/[11:9]
        (*mpTg2IOPinMuxCfgAddr) &= (~0xE07);
        (*mpTg2IOPinMuxCfgAddr) |= 0x603;
        mpTg2IOPinMuxCfgAddr = mpGpioHwRegAddr + (0xC80/4); //GPIO40/41/44 CM2DAT3/CM2MCLK/CM2DAT0 [2:0]/[5:3]/[14:12]
        (*mpTg2IOPinMuxCfgAddr) &= (~0x703F);
        (*mpTg2IOPinMuxCfgAddr) |= 0x301B;
        mpTg2IOPinMuxCfgAddr = mpGpioHwRegAddr + (0xC90/4); //GPIO45 CM2DAT1 [2:0]
        (*mpTg2IOPinMuxCfgAddr) &= (~0x7);
        (*mpTg2IOPinMuxCfgAddr) |= 0x03;
        //config pull sel
        //en
        mpTg2IOPinMuxCfgAddr = mpGpioHwRegAddr + (0x210/4); //GPIO19/21/22/26/27/30 [3]/[5]/[6]/[10]/[11]/[14]
        (*mpTg2IOPinMuxCfgAddr) &= (~0x4C68);//disable
        mpTg2IOPinMuxCfgAddr = mpGpioHwRegAddr + (0x220/4); //GPIO32//35//40//44/45 [0]/[3]/[8]/[12]/[13]
        (*mpTg2IOPinMuxCfgAddr) &= (~0x3109);//disable
        //sel
        //mpTg2IOPinMuxCfgAddr = mpGpioHwRegAddr + (0x410/4); //GPIO19/21/22/26/27/30 [3]/[5]/[6]/[10]/[11]/[14]
        //mpTg2IOPinMuxCfgAddr = mpGpioHwRegAddr + (0x420/4); //GPIO32/34/35/38/40/41/44/45 [0]/[2]/[3]/[6]/[8]/[9]/[12]/[13]
        #endif
        
    }
    #endif
    //
    return 0;
}//
//----------------------------------------------------------------------------
MINT32 IspDrvImp::setTgGrabRange(
    MUINT32     pixelStart,
    MUINT32     pixelEnd,
    MUINT32     lineStart,
    MUINT32     lineEnd)
{
    MINT32 ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    //
    LOG_MSG("pixelStart(%ld), pixelEnd(%ld), lineStart(%ld), lineEnd(%ld)",pixelStart,pixelEnd,lineStart,lineEnd);
    // TG Grab Win Setting
    ISP_BITS(pisp, CAM_GRABCOL, PIXEL_END) = pixelEnd;
    ISP_BITS(pisp, CAM_GRABCOL, PIXEL_START) = pixelStart;
    ISP_BITS(pisp, CAM_GRABROW, LINE_END) = lineEnd;
    ISP_BITS(pisp, CAM_GRABROW, LINE_START) = lineStart;
    //
    return ret;
}

/*******************************************************************************
* Functionality : set grab range of TG2
********************************************************************************/
int IspDrvImp::setTg2GrabRange(
    MUINT32 pixelStart, 
    MUINT32 pixelEnd, 
    MUINT32 lineStart, 
    MUINT32 lineEnd)
{
    int ret = 0;

    LOG_MSG("E : pixelStart=%u, pixelEnd=%u, lineStart=%u, lineEnd=%u,",pixelStart, pixelEnd, lineStart, lineEnd);
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    
    // TG2 Grab Win Setting 
    ISP_BITS(pisp, CAM_TG2_SEN_GRAB_PXL, PXL_E) = pixelEnd;
    ISP_BITS(pisp, CAM_TG2_SEN_GRAB_PXL, PXL_S) = pixelStart;
    ISP_BITS(pisp, CAM_TG2_SEN_GRAB_LIN, LIN_E) = lineEnd;
    ISP_BITS(pisp, CAM_TG2_SEN_GRAB_LIN, LIN_S) = lineStart;
    
    return ret;
}


//----------------------------------------------------------------------------
MINT32 IspDrvImp::setSensorModeCfg(
    MUINT32     hsPol,
    MUINT32     vsPol)
{
    MINT32 ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    //
    LOG_MSG("hsPol(%ld), vsPol(%ld)",hsPol,vsPol);
    // Sensor Mode Config
    ISP_BITS(pisp, CAM_CSMODE, HSPOL) = hsPol;
    ISP_BITS(pisp, CAM_CSMODE, VSPOL) = vsPol;
    ISP_BITS(pisp, CAM_CSMODE, EN) = 1;
    //
    return ret;
}


/*******************************************************
* Functionality : set sensor Hsync /Vsync input polarity
*******************************************************/
MINT32 IspDrvImp::setTG2SensorModeCfg(
    MUINT32 hsPol, 
    MUINT32 vsPol)
{
    MINT32 ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    
    LOG_MSG("hsPol(%u), vsPol(%u)",hsPol,vsPol);

    // === Sensor Mode Config===
    
    ISP_BITS(pisp, CAM_TG2_SEN_MODE, HSPOL       ) = hsPol;	// Sensor Hsync input polarity.
    ISP_BITS(pisp, CAM_TG2_SEN_MODE, VSPOL       ) = vsPol;	// Sensor Vsync input polarity.
    ISP_BITS(pisp, CAM_TG2_SEN_MODE, SOT_CLR_MODE) = 1;	// 0(choose SOT_MODE selection)/1(sot position before data 1T, only for rgb, yuv). SOT RGB/YUV MODE.
    ISP_BITS(pisp, CAM_TG2_SEN_MODE, SOT_MODE    ) = 1;	// 0(sot position is at SOT_CNT)/1(sot position is sof_1d). SOT MODE.
    #ifdef DBL_BUS  // Using double bus.
    ISP_BITS(pisp, CAM_TG2_SEN_MODE, DBL_DATA_BUS) = 1;	// Double the asynchronous fifo data bus. For yuv422 or rgb565 or jpeg mode, this bit could be set to 1. For raw/rgb888 mode, this bit must be 0.
    #endif  // DBL_BUS
    //ISP_BITS(pisp, CAM_TG2_SEN_MODE, CMOS_EN     ) = 0;	// Sensor process enable.
  
    return ret;
}


//----------------------------------------------------------------------------
MINT32 IspDrvImp::setResultWin(
    MUINT32     horiStart,
    MUINT32     horiEnd,
    MUINT32     vertStart,
    MUINT32     vertEnd,
    MUINT32     enable)
{
    MINT32 ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    //
    LOG_MSG("horiStart(%ld), horiStart(%ld), vertStart(%ld), vertEnd(%ld), enable(%ld)",horiStart,horiEnd,vertStart,vertEnd,enable);
    //
    ISP_BITS(pisp, CAM_RWINH_SEL, RWINH_END) = horiEnd;
    ISP_BITS(pisp, CAM_RWINH_SEL, RWINH_START) = horiStart;
    ISP_BITS(pisp, CAM_RWINV_SEL, RWINV_END) = vertEnd;
    ISP_BITS(pisp, CAM_RWINV_SEL, RWINV_START) = vertStart;
    ISP_BITS(pisp, CAM_RWINV_SEL, RWIN_EN) = enable;
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::setMemIOAddr(
    MUINT32     inAddr,
    MUINT32     outAddr)
{
    MINT32 ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    //
    LOG_MSG("inAddr(0x%08lX), outAddr(0x%08lX)",inAddr,outAddr);
    //
    ISP_BITS(pisp, CAM_INADDR, CAM_INADDR) = inAddr;
    ISP_BITS(pisp, CAM_OUTADDR, CAM_OUTADDR) = outAddr;
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::setCamModulePath(
    MUINT32     inPathSel,
    MUINT32     inTypeSel,
    MUINT32     swapY,
    MUINT32     swapCbCr,
    MUINT32     outPathType,
    MUINT32     outPathEn,
    MUINT32     inDataFormat)
{
    MINT32 ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    MINT32 isBayer10 = 1;    //0:8-bit ,  1:10-bit 
    //
    LOG_MSG("inPathSel(%ld), inTypeSel(%ld), swapY(%ld), swapCbCr(%ld), outPathType(%ld), outPathEn(%ld), inDataFormat(%ld)",
            inPathSel,inTypeSel,swapY,swapCbCr,outPathType,outPathEn,inDataFormat);
    // Enable debug mode counter
    ISP_BITS(pisp, CAM_PATH, CNTON) = 1;
    ISP_BITS(pisp, CAM_PATH, CNTMODE) = 1;
    ISP_BITS(pisp, CAM_PATH, INTYPE_SEL) = inTypeSel;
    //
    ISP_BITS(pisp, CAM_PATH, INPATH_SEL) = inPathSel;
    // [7:0] as raw data input  (Senosr raw out should set to 0)
    ISP_BITS(pisp, CAM_PATH, INDATA_FORMAT) = inDataFormat;
    ISP_BITS(pisp, CAM_PATH, SWAP_Y) = swapY;
    ISP_BITS(pisp, CAM_PATH, SWAP_CBCR) = swapCbCr;
    // accoding to MT6516 setting
    ISP_BITS(pisp, CAM_PATH, INPATH_RATE) = 0;
    // dump 10-bit bayer data
    ISP_BITS(pisp, CAM_PATH, BAYER10_IN) = outPathType == ISP_DRV_OUTPUT_FORMAT_BAYER ? (isBayer10) : 0;
    //
    ISP_BITS(pisp, CAM_PATH, OUTPATH_TYPE) = outPathType;
    ISP_BITS(pisp, CAM_PATH, BAYER10_OUT) = outPathType == ISP_DRV_OUTPUT_FORMAT_BAYER ? (isBayer10) : 0;
    //
    ISP_BITS(pisp, CAM_PATH, OUTPATH_EN) = outPathEn;
    //
    ISP_BITS(pisp, CAM_MEMIN, MEMIN_OUTS_EN) = 1;
    ISP_BITS(pisp, CAM_MEMIN, MEMIN_BURST16) = 1;
    //
    if(outPathEn == 0)
    {
        // To CRZ
        ISP_BITS(pisp, CAM_MEMIN, CRZ_RESZLB_LB_CONTROL) = 1;
        ISP_BITS(pisp, CAM_PATH, REZ_DISCONN) = 0;
        ISP_BITS(pisp, CAM_MEMOUT, BAYEROUT_DM_EN) = 0;
    }
    else
    {
        // To DRAM
        ISP_BITS(pisp, CAM_PATH, REZ_DISCONN) = 1;
        ISP_BITS(pisp, CAM_MEMIN, CRZ_RESZLB_LB_CONTROL) = 0;
        ISP_BITS(pisp, CAM_MEMOUT, BAYEROUT_DM_EN) = 1;
        //
        if(outPathType == ISP_DRV_OUTPUT_FORMAT_YUV444)
        {
            ISP_REG(pisp, CAM_PATH2) = 1;
        }
        else
        if(outPathType == ISP_DRV_OUTPUT_FORMAT_YUV422)
        {
            ISP_REG(pisp, CAM_PATH2) = 0;
        }
    }
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::setViewFinderMode(
    MUINT32     spMode,
    MUINT32     spDelay)
{
    MINT32 ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    //
    LOG_MSG("spMode(%ld), spDelay(%ld)",spMode,spDelay);
    //
    ISP_BITS(pisp, CAM_VFCON, SP_MODE) = spMode;
    ISP_BITS(pisp, CAM_VFCON, SP_DELAY) = spDelay;
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::setTG2ViewFinderMode(
    MUINT32     spMode,
    MUINT32     spDelay) 
{
    MINT32 ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    //
     LOG_MSG("spMode(%ld), spDelay(%ld)",spMode,spDelay);
    //
    ISP_BITS(pisp, CAM_TG2_VF_CON, SINGLE_MODE) = spMode;
    ISP_BITS(pisp, CAM_TG2_VF_CON, SP_DELAY) = spDelay;
    //
    return ret;
}

//----------------------------------------------------------------------------
MINT32 IspDrvImp::control(MUINT32 isEn)
{
    MINT32 ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    //
    LOG_MSG("isEn(%ld)", isEn);
    //
    if(mpCallback != NULL)
    {
        LOG_MSG("Callback(%d)",isEn);
        if(isEn)
        {
            (*mpCallback)(true);
        }
        else
        {
            (*mpCallback)(false);
            mpCallback = NULL;
        }
        LOG_MSG("Callback End");
    }
    // Enable/Disable ISR
    ISP_BITS(pisp, CAM_INTEN, EXPDONE_INT_EN) = isEn;
    ISP_BITS(pisp, CAM_INTEN, REZOVRUN_INT_EN) = isEn;
    ISP_BITS(pisp, CAM_INTEN, GMCOVRUN_INT_EN) = isEn;
    ISP_BITS(pisp, CAM_INTEN, IDLE_INT_EN) = isEn;
    ISP_BITS(pisp, CAM_INTEN, ISPDONE_INT_EN) = isEn;
    ISP_BITS(pisp, CAM_INTEN, TG1_INT_EN) = isEn;
    ISP_BITS(pisp, CAM_INTEN, TG2_INT_EN) = isEn;
    ISP_BITS(pisp, CAM_INTEN, REV1) = 0x2; //enable AF done for AF application.
    //ISP_BITS(pisp, CAM_INTEN, VSYNC_INT_EN) = isEn;
    ioctl(mfd, MT_ISP_IOC_T_ENABLE_VD_PROC, (MBOOL*)&isEn);
    // Enable/Disable
    ISP_BITS(pisp, CAM_VFCON, TAKE_PIC) = isEn;
    //enable CAMCRZ_INIT_EN
    ISP_BITS(pisp, CAMCRZ_CTRL, CAMCRZ_INIT_EN) = 1;
    ISP_BITS(pisp, CAMCRZ_CTRL, CAMCRZ_INIT_PERIOD) = 3;
    //
    return ret;
}

//----------------------------------------------------------------------------
MINT32 IspDrvImp::controlTG2(MUINT32 isEn)
{
    MINT32 ret = 0;
    
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("isEn(%u)", isEn);
    if (isEn) {
        LOG_MSG("HW_RST");
        ISP_BITS(pisp, CAM_CTL_SW_CTL, HW_RST) = 1;
        ISP_BITS(pisp, CAM_CTL_SW_CTL, HW_RST) = 0;
    }
    
    //ISP_BITS(pisp, CAM_TG2_SEN_MODE, CMOS_EN) = 1;   // Sensor process enable.    
    ISP_BITS(pisp, CAM_TG2_VF_CON, VFDATA_EN) = isEn;	// Take Picture Request     

    return ret;
}

//----------------------------------------------------------------------------
MINT32 IspDrvImp::setGammaEn(MUINT32 isEn)
{
    MINT32 ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    //
    LOG_MSG("isEn(%ld)", isEn);
    //
    ISP_BITS(pisp, CAM_CPSCON2, BYPGM) = (isEn == 1 ? 0 : 1);
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::waitIrq(ISP_DRV_WAIT_IRQ_STRUCT* pWaitIrq)
{
    MINT32 ret = 0;
    mt_isp_wait_irq_t IspWaitIrq;
    //
    IspWaitIrq.mode = pWaitIrq->Mode;
    IspWaitIrq.timeout = pWaitIrq->Timeout;
    //
    //LOG_MSG("mode(0x%08lX)", IspWaitIrq.mode);
    //
    ret = ioctl(mfd, MT_ISP_IOC_T_WAIT_IRQ, &IspWaitIrq);
    if(ret < 0)
    {
        LOG_ERR("MT_ISP_IOC_T_WAIT_IRQ err");
    }
    return ret;    
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::reset(void)
{
    MINT32 ret;
    //
    LOG_MSG("");
    //
    ret = ioctl(mfd, MT_ISP_IOC_T_RESET, NULL);
    if(ret < 0)
    {
        LOG_ERR("MT_ISP_IOC_T_RESET err");
    }
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::sendCommand(
    MINT32      cmd,
    MINT32      arg1,
    MINT32      arg2,
    MINT32      arg3)
{
    MINT32 ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    mt_isp_PQIndex_t PQIndex;
    ISP_DRV_PQIndex_STRUCT * pDrvPQIndex;
    //
    switch(cmd)
    {
        case ISP_DRV_CMD_RESET_BUF:
        {
            LOG_MSG("ISP_DRV_CMD_RESET_BUF");
            ret = ioctl(mfd, MT_ISP_IOC_T_RESET_BUF);
            if(ret < 0)
            {
                LOG_ERR("ISP_DRV_CMD_RESET_BUF err(%d)", ret);
            }
            break;
        }
        case ISP_DRV_CMD_CLEAR_INT_STATUS : 
        {
            MUINT32 cleanISPVDSignal;
            cleanISPVDSignal = ISP_REG(pisp, CAM_INTSTA);           
            break;  
        }
        case ISP_DRV_CMD_RESET_VD_COUNT:
        {
            ret = ioctl(mfd, MT_ISP_IOC_T_RESET_VD_COUNT, NULL);
            if(ret < 0)
            {
                LOG_ERR("MT_ISP_IOC_T_RESET_VD_COUNT err");
            }          
            break;
        }
        //
        case ISP_DRV_CMD_SET_PIXEL_TYPE:
        {
            LOG_MSG("ISP_DRV_CMD_SET_PIXEL_TYPE(%d)", arg1);
            ISP_BITS(pisp, CAM_CTRL1, GPID) = arg1 & 0x01;
            ISP_BITS(pisp, CAM_CTRL1, GLID) = (arg1 >> 1) & 0x01;
            LOG_MSG("CAM_CTRL1(0x%08x)", ISP_REG(pisp, CAM_CTRL1));
            break;
        }
        case ISP_DRV_CMD_SET_KDBG_FLAG:
        {
            LOG_MSG("ISP_DRV_CMD_SET_KDBG_FLAG(%d)", arg1);
            ret = ioctl(mfd, MT_ISP_IOC_T_DBG_FLAG, &arg1);
            if(ret < 0)
            {
                LOG_ERR("ISP_DRV_CMD_SET_KDBG_FLAG err");
            }
            break;
        }
        case ISP_DRV_CMD_SET_CAPTURE_DELAY:
        {
            ISP_BITS(pisp, CAM_VFCON, SP_DELAY) = arg1;
            break;
        }
        case ISP_DRV_CMD_SET_VD_PROC : 
        {
            ioctl(mfd, MT_ISP_IOC_T_ENABLE_VD_PROC, (MBOOL*)arg1);          
            break;
        }
        //
        case ISP_DRV_CMD_GET_ADDR:
        {
            //LOG_MSG("ISP_DRV_CMD_GET_ADDR(0x%x)", (MINT32) mpIspHwRegAddr);
            *(MINT32 *) arg1 = (MINT32) mpIspHwRegAddr;
            break;
        }
        case ISP_DRV_CMD_GET_LINE_ID:
        {
            MUINT32 const u4LineID = ISP_BITS(pisp, CAM_CTRL1, GLID);
            *reinterpret_cast<MUINT32*>(arg1) = u4LineID;
            LOG_MSG("<ISP_DRV_CMD_GET_LINE_ID> u4LineID(%d)", u4LineID);
            break;
        }
        case ISP_DRV_CMD_GET_CAPTURE_DELAY : 
        {
            *(MINT32 *) arg1 = (MINT32) ISP_BITS(pisp, CAM_VFCON, SP_DELAY); // get ISP capture delay value            
            break;
        }
        case ISP_DRV_CMD_GET_VD_COUNT:
        {
            ret = ioctl(mfd, MT_ISP_IOC_G_GET_VD_COUNT, (MUINT32*)arg1);
            if(ret < 0)
            {
                LOG_ERR("MT_ISP_IOC_G_GET_VD_COUNT err");
                *(MUINT32*)arg1 = 0;
            }        
            break;
        }
        case ISP_DRV_CMD_SET_PQ_PARAM:
        {
            pDrvPQIndex = (ISP_DRV_PQIndex_STRUCT *)arg1;
            PQIndex.u4SkinToneIndex = pDrvPQIndex->u4SkinToneIndex;
            PQIndex.u4GrassToneIndex = pDrvPQIndex->u4GrassToneIndex;
            PQIndex.u4SkyToneIndex = pDrvPQIndex->u4SkyToneIndex;
            PQIndex.u4YCCGOIndex = pDrvPQIndex->u4YCCGOIndex;
            PQIndex.u4EEIndex = pDrvPQIndex->u4EEIndex;

            if(-1 == mfd)
            {
                LOG_ERR("SET_PQ_PARAM failed because no fd");
                break;
            }

            if(1 == pDrvPQIndex->u4Reset)
            {
                ret = ioctl(mfd, MT_ISP_IOC_S_SET_PQ_PARAM, &PQIndex);
            }
            else
            {
                ret = ioctl(mfd, MT_ISP_IOC_S_SET_PQ_PARAM_INIT, &PQIndex);
            }

            if(ret < 0)
            {
                LOG_ERR("SET_PQ_PARAM err:%d" , ret);
                *(MUINT32*)arg1 = 0;
            }
            break;
        }
        case ISP_DRV_CMD_GET_PQ_PARAM:
        {
            pDrvPQIndex = (ISP_DRV_PQIndex_STRUCT *)arg1;
            if(-1 == mfd)
            {
                LOG_ERR("SET_PQ_PARAM failed because no fd");
                break;
            }
            ret = ioctl(mfd, MT_ISP_IOC_S_GET_PQ_PARAM, &PQIndex);
            if(ret < 0)
            {
                LOG_ERR("MT_ISP_IOC_S_GET_PQ_PARAM err");
                *(MUINT32*)arg1 = 0;
            }
            pDrvPQIndex->u4SkinToneIndex = PQIndex.u4SkinToneIndex;
            pDrvPQIndex->u4GrassToneIndex = PQIndex.u4GrassToneIndex;
            pDrvPQIndex->u4SkyToneIndex = PQIndex.u4SkyToneIndex;
            pDrvPQIndex->u4YCCGOIndex = PQIndex.u4YCCGOIndex;
            pDrvPQIndex->u4EEIndex = PQIndex.u4EEIndex;
            break;
        }
        default:
        {
            LOG_MSG("Unknow cmd(0x%X)",cmd);
            ret = -1;
            break;
        }
    }
    //
    return ret;
}
//----------------------------------------------------------------------------
MUINT32 IspDrvImp::readReg(MUINT32 addr)
{
    MINT32 ret;
    reg_t reg[2];
    MINT32 val = 0xFFFFFFFF;
    //
    //LOG_MSG("addr(0x%08x)", (MINT32) addr);
    //
    reg[0].addr = addr;
    reg[0].val = val;
    //
    ret = readRegs(reg, 1);
    if(ret < 0)
    {
        //do nothing
    }
    else
    {
        val = reg[0].val;
    }
    //
    return val;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::writeReg(
    MUINT32     addr,
    MUINT32     val)
{
    MINT32 ret;
    reg_t reg[2];
    //
    //LOG_MSG("addr(0x%08x), val(0x%08x)", (MINT32) addr, (MINT32) val);
    //
    reg[0].addr = addr;
    reg[0].val = val;
    //
    ret = writeRegs(reg, 1);
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::readRegs(
    reg_t*      pregs,
    MINT32      count)
{
    MINT32 ret;
    mt_isp_reg_io_t reg_io;
    //
    //LOG_MSG("");
    //
    reg_io.data = (MINT32) pregs;
    reg_io.count = count;
    //
    ret = ioctl(mfd, MT_ISP_IOC_G_READ_REG, &reg_io);
    if(ret < 0)
    {
        LOG_ERR("MT_ISP_IOC_G_READ_REG err");
    }
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::writeRegs(
    reg_t*      pregs,
    MINT32      count)
{
    MINT32 ret;
    mt_isp_reg_io_t reg_io;
    //
    //LOG_MSG("");
    //
    reg_io.data = (MINT32) pregs;
    reg_io.count = count;
    //
    ret = ioctl(mfd, MT_ISP_IOC_S_WRITE_REG, &reg_io);
    if(ret < 0)
    {
        LOG_ERR("MT_ISP_IOC_S_WRITE_REG err");
    }
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::holdReg(MBOOL isHold)
{
    MINT32 ret;
    MINT32 hold = isHold;
    //
    //LOG_MSG("");
    //
    ret = ioctl(mfd, MT_ISP_IOC_T_HOLD_REG, &hold);
    if(ret < 0)
    {
        LOG_ERR("MT_ISP_IOC_T_HOLD_REG err");
    }
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::dumpReg(void)
{
    MINT32 ret;
    //
    LOG_MSG("");
    //
    ret = ioctl(mfd, MT_ISP_IOC_T_DUMP_REG, NULL);
    if(ret < 0)
    {
        LOG_ERR("MT_ISP_IOC_T_DUMP_REG err");
    }
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::initCSI2(MUINT32 csi2_en)
{
    MINT32 ret = 0, bufferSize = 20;;
    char csi2Cmd[bufferSize];
    //
    if(mpCAMAPConRegAddr != NULL)
    {
        LOG_MSG("csi2_en(%d)", (MINT32) csi2_en);
        if(csi2_en == 0)
        {
            // disable mipi ap config and csi2 clear gating
            //TODO FIXME 
            //*(mpCSI2RxRegAddr) = 0x00000000;
            //*(mpCSI2RxRegAddr + (0x18/4)) = 0x00000000;     
            #if ISP_DRV_CAM_MIPI_API 
            if(mfd1 > 0)
            {
                strcpy(csi2Cmd, "disable 1");
                write(mfd1, csi2Cmd, strlen(csi2Cmd));
                LOG_MSG("csi2Cmd(%s), size(%d)", csi2Cmd, strlen(csi2Cmd));               
            }
            #endif           
        }
        else
        {
            // enable mipi ap and clear gating
            *(mpCSI2ClearGateRegAddr) |= 0x00000002; 
            //
            #if ISP_DRV_CAM_MIPI_API
            strcpy(csi2Cmd, "enable 1");     
            write(mfd1, csi2Cmd, strlen(csi2Cmd)); //enable camera mipi
            LOG_MSG("csi2Cmd(%s), size(%d)", csi2Cmd, strlen(csi2Cmd));
            #else
            *(mpMIPIAPConRegAddr) |= 0x00000100;            // 0xC0001890   RX_EN
            *(mpMIPIPLLRegAddr) |= 0x00000010;              // 0xC0007044   ANA_26M_EN
            // config mipi rx
            *(mpCSI2RxRegAddr) |= 0x00000002;               // 0xC20A3800   MIPITX_CON0
            *(mpCSI2RxRegAddr + (0x18/4)) |= 0x00000003;    // 0xC20A3818   MIPITX_CON6
            *(mpCSI2RxRegAddr + (0x54/4)) |= 0x00004008;    // 0xC20A3854   MIPIRX_CON5
            *(mpCSI2RxRegAddr + (0x44/4)) |= 0x00000666;    // 0xC20A3844   MIPIRX_CON1
            *(mpCSI2RxRegAddr + (0x50/4)) |= 0x00000020;    // 0xC20A3844   MIPIRX_CON4
            #endif
        }
        LOG_MSG("mpCSI2RxRegAddr(0x%0lx)", *(mpCSI2RxRegAddr)); 
    }
    return ret;   
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::setCSI2(
    MUINT32     dataTermDelay,
    MUINT32     dataSettleDelay,
    MUINT32     clkTermDelay,
    MUINT32     vsyncType,
    MUINT32     dlane1_en,
    MUINT32     csi2_en,
    MUINT32     dataheaderOrder,
    MUINT32     dataFlow)
{
    MINT32 ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;    
    //
    if(csi2_en == 1)
    {
        // enable CSI2
        LOG_MSG("DataTermDelay(%d) ,SettleDelay(%d), ClkTermDelay(%d), VsyncType(%d), dlane1_en(%d), CSI2 enable(%d), HeaderOrder(%d), DataFlow(%d)", 
                (MINT32) dataTermDelay, (MINT32) dataSettleDelay, (MINT32) clkTermDelay, (MINT32) vsyncType, (MINT32) dlane1_en, (MINT32) csi2_en, (MINT32)dataheaderOrder, (MINT32)dataFlow);
        //
        ISP_BITS(pisp, CSI2_CTRL, CSI2_EN) = 0;    // disable CSI2 first
        ISP_BITS(pisp, CSI2_DELAY, LP2HS_CLK_TERM_DELAY) = clkTermDelay;
        ISP_BITS(pisp, CSI2_DELAY, LP2HS_DATA_TERM_DELAY) = dataTermDelay;
        ISP_BITS(pisp, CSI2_DELAY, LP2HS_DATA_SETTLE_DELAY) = dataSettleDelay;
        ISP_BITS(pisp, CSI2_INTEN, CRC_ERR_IRQ_EN) = 1;
        ISP_BITS(pisp, CSI2_INTEN, ECC_ERR_IRQ_EN) = 1;
        ISP_BITS(pisp, CSI2_INTEN, ECC_CORRECT_IRQ_EN) = 1;
        ISP_BITS(pisp, CSI2_INTSTA, CSI2_IRQ_CLR_SEL) = 1;     // write clear
        //ISP_BITS(pisp, CSI2_DBG, LNC_HSRXDB_EN) = 1;
        ISP_REG(pisp, CSI2_CTRL) =(dataFlow<<17) | (vsyncType <<13) | (1 << 10) | (dataheaderOrder<<5) | (1<<4) | (dlane1_en<<1) | (csi2_en<<0);    
    }
    else
    {
        // disable CSI2
        ISP_BITS(pisp, CSI2_CTRL, CSI2_EN) = 0; 
    }
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::setIODrivingCurrent(MUINT32 ioDrivingCurrent)
{
    MINT32 ret = 0;
    if(mpCAMIODrvRegAddr != NULL)
    {
        *(mpCAMIODrvRegAddr) &= 0xFFFF0FFF;
        *(mpCAMIODrvRegAddr) |= (ioDrivingCurrent<<12);   // CLK  CAM1  CAM
    }
    LOG_MSG("[setIODrivingCurrent]:%d 0x%08x\n", (MINT32) ioDrivingCurrent, (MINT32) (*(mpCAMIODrvRegAddr)));

    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::setTG2IODrivingCurrent(MUINT32 ioDrivingCurrent)
{
    MINT32 ret = 0;

    if(mpCAM2IODrvRegAddr != NULL)
    {
        *(mpCAM2IODrvRegAddr) &= 0xF0FFFFFF;
        *(mpCAM2IODrvRegAddr) |= (ioDrivingCurrent<<24);   // CAM2 MCLK
    }
    LOG_MSG("[setTG2IODrivingCurrent]:%d 0x%08x\n", (MINT32) ioDrivingCurrent, (MINT32) (*(mpCAM2IODrvRegAddr)));

    return ret;
}

//----------------------------------------------------------------------------
MINT32 IspDrvImp::setMCLKEn(MUINT32 isEn)
{
    MINT32 ret = 0;    
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    //
    if(isEn == 0)
    {
        ISP_BITS(pisp, CAM_PHSCNT, CLKEN) = 0;
    }
    else
    {
        ISP_BITS(pisp, CAM_PHSCNT, CLKEN) = 1;
    }  
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspDrvImp::setCamClock(void)
{
    MINT32 ret;
    //
    LOG_MSG("");
    //
    ret = ioctl(mfd, MT_ISP_IOC_T_ENABLE_CAM_CLOCK, NULL);
    if(ret < 0)
    {
        LOG_ERR("MT_ISP_IOC_T_ENABLE_CAM_CLOCK err");
    }
    //
    return ret;
}
//----------------------------------------------------------------------------
void IspDrvImp::setSubsampleWidth(
    MBOOL       Enable,
    MUINT32     Input,
    MUINT32     Output)
{
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    //
    LOG_MSG("Enable(%d), Input(%ld), Output(%ld)",Enable,Input,Output);
    //
    ISP_BITS(pisp, CAM2_DROP_HSUB, H_SUB_EN) = Enable;
    //
    if(Enable)
    {
        ISP_BITS(pisp, CAM2_DROP_HSUB, H_SUB_IN) = Input;
        ISP_BITS(pisp, CAM2_DROP_HSUB, H_SUB_OUT) = Output;
    }
}
//----------------------------------------------------------------------------
void IspDrvImp::setSubsampleHeight(
    MBOOL       Enable,
    MUINT32     Input,
    MUINT32     Output)
{
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    //
    LOG_MSG("Enable(%d), Input(%ld), Output(%ld)",Enable,Input,Output);
    //
    ISP_BITS(pisp, CAM2_DROP_VSUB, V_SUB_EN) = Enable;
    //
    if(Enable)
    {
        ISP_BITS(pisp, CAM2_DROP_VSUB, V_SUB_IN) = Input;
        ISP_BITS(pisp, CAM2_DROP_VSUB, V_SUB_OUT) = Output;
    }
}
//----------------------------------------------------------------------------
void IspDrvImp::regCallback(pIspDrvCallback pFunc)
{
    if(pFunc == NULL)
    {
        LOG_ERR("pFunc is NULL");
        return;
    }
    //
    mpCallback = pFunc;
}
//----------------------------------------------------------------------------
void IspDrvImp::setLogMask(MUINT32 LogMask)
{
    LOG_MSG("LogMask(0x%08X)",LogMask);
    IspDrvLogMask = LogMask; 
}
//----------------------------------------------------------------------------

