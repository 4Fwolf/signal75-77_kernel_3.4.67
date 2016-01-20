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
#include <utils/Errors.h>
#include <cutils/xlog.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
#include "MediaTypes.h"

#include "eis_drv.h"
#include "camera_eis.h"

#define EIS_DRV_TAG             "[EISdrv]"
#define EIS_LOG(fmt, arg...)    XLOGD(EIS_DRV_TAG fmt, ##arg)
#define EIS_ERR(fmt, arg...)    XLOGE(EIS_DRV_TAG "Err: %5d:, "fmt, __LINE__, ##arg)

#define IMG_HEIGHT_THRES_1   490
#define IMG_HEIGHT_THRES_2   900

#define IMG_WIDTH_THRES_1    400
#define IMG_WIDTH_THRES_2    600
#define IMG_WIDTH_THRES_3    800
#define IMG_WIDTH_THRES_4    1200

typedef struct EIS_REG_S {
    unsigned int val[21];    // register's value
} EIS_REG_T;


/*******************************************************************************
*
********************************************************************************/
EisDrvBase* 
EisDrvBase::createInstance()
{
    return EisDrv::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
EisDrvBase*
EisDrv::
getInstance()
{
    EIS_LOG("getInstance \n");
    static EisDrv singleton;
    
    if (singleton.init() != 0)  {
        EIS_LOG("singleton.init() fail \n");        
        return NULL;
    }
    
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void   
EisDrv::
destroyInstance() 
{
	uninit();
}

/*******************************************************************************
*
********************************************************************************/
EisDrv::EisDrv() : EisDrvBase()
{    
    mUsers = 0;
    mfd = 0;
    mpEis = NULL;

    m_i4FL_Max_H = 0;
    m_i4FL_Max_V = 0; 
    m_i4DIV_H = 0;
    m_i4DIV_V = 0;
    
}

/*******************************************************************************
*
********************************************************************************/
EisDrv::~EisDrv()
{

}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrv::init()
{
    EIS_LOG("init - mUsers: %d \n", mUsers);
    
    Mutex::Autolock lock(mLock);

    if (mUsers > 0) {
        EIS_LOG("%d has inited \n", mUsers);
        android_atomic_inc(&mUsers);
        return 0;
    }
    
    // Open eis driver
    mfd = open("/dev/camera-eis", O_RDWR);
    if (mfd < 0) {
        EIS_ERR("error open kernel driver, %d, %s", errno, strerror(errno));
        return -1;
    }

    // mmap eis reg
    mpEis = (unsigned long *) mmap(0, EIS_BASE_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, EIS_BASE_HW);
    if (mpEis == MAP_FAILED) {
        EIS_ERR("mmap err, %d, %s \n", errno, strerror(errno));
        return -2;
    } 

    android_atomic_inc(&mUsers);

    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrv::uninit()
{
    EIS_LOG("uninit - mUsers: %d \n", mUsers);

    Mutex::Autolock lock(mLock);

    if (mUsers <= 0) {
        // No more users
        return 0;
    }
    // More than one user
    android_atomic_dec(&mUsers);

    if (mUsers == 0) {
        // Last user
        munmap(mpEis, EIS_BASE_RANGE);
        mpEis = NULL;

        if (mfd > 0) {
            close(mfd);
            mfd = -1;
        }
    }
    else {
        EIS_LOG("Still %d users \n", mUsers);
    }

    return 0;
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::enableEIS(bool a_bEnable)
{
    MINT32 ret = 0;

    EIS_LOG("[enableEIS] %d\n", a_bEnable);

    eis_reg_t *pEis = (eis_reg_t *) mpEis;

	if (a_bEnable == 0)	 {setIRQ_EN(0);}
		
    ret = ioctl(mfd, MT_EIS_IOC_T_CLK_ONOFF, a_bEnable);

    if (ret != 0)
    {
        EIS_ERR("CLK ONOFF error\n");
    }
    else
    {
        EIS_BITS(pEis, EIS_PREP_ME_CTRL, CE) = a_bEnable;
    }
}

/*******************************************************************************
*
********************************************************************************/
bool EisDrv::isEISenable()
{
    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    return EIS_BITS(pEis, EIS_PREP_ME_CTRL, CE);
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::setSrcSel(bool a_bSrcSel)
{
    EIS_LOG("[set EIS source: 0 CAM, 1 CRZ] %d\n", a_bSrcSel);

    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    EIS_BITS(pEis, EIS_PREP_ME_CTRL, EIS_SRC_SEL) = a_bSrcSel;
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::setSRAM_PD(bool a_bSRAM_PD)
{
    EIS_LOG("[set EIS SRAM PD: 1 Power down] %d\n", a_bSRAM_PD);

    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    EIS_BITS(pEis, EIS_PREP_ME_CTRL, EIS_SRAM_PD) = a_bSRAM_PD;    
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::setSRAM_RB(bool a_bSRAM_RB)
{
    EIS_LOG("[set EIS SRAM RB: 0 Reset] %d\n", a_bSRAM_RB);

    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    EIS_BITS(pEis, EIS_PREP_ME_CTRL, EIS_SRAM_RB) = a_bSRAM_RB;
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::setKneeClip(MINT32 a_i4Knee, MINT32 a_i4Clip)
{
    EIS_LOG("[setKneeClip 0~15] Knee: %d, Clip: %d\n", a_i4Knee, a_i4Clip);

    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    if (a_i4Knee > 15)  {a_i4Knee = 15;}
    if (a_i4Knee < 0)   {a_i4Knee = 0;}    
    if (a_i4Clip > 15)  {a_i4Clip = 15;}
    if (a_i4Clip < 0)   {a_i4Clip = 0;}    

    EIS_BITS(pEis, EIS_PREP_ME_CTRL, ME_AD_KNEE) = a_i4Knee;
    EIS_BITS(pEis, EIS_PREP_ME_CTRL, ME_AD_CLIP) = a_i4Clip;
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::setHRP(MINT32 a_i4Num)
{
    EIS_LOG("[setHRP num 1~8] %d\n", a_i4Num);

    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    if (a_i4Num > 8)  {a_i4Num = 8;}
    if (a_i4Num < 1)  {a_i4Num = 1;}    

    EIS_BITS(pEis, EIS_PREP_ME_CTRL, ME_NUM_HRP) = a_i4Num;
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::setVFilter(MINT32 a_i4Gain, MINT32 a_i4DS)
{
    EIS_LOG("[setVFilter] G: %d, DS: %d\n", a_i4Gain, a_i4DS);
    // Gain         [0,3: 1/8]   [1: 1/16]  [2: 1/32]
    // DeSample [0,3: 1->1] [1: 1->2] [2: 1->4]
    
    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    if (a_i4Gain > 3)   {a_i4Gain = 3;}
    if (a_i4Gain < 0)   {a_i4Gain = 0;}    
    if (a_i4DS > 3)     {a_i4DS = 3;}
    if (a_i4DS < 0)     {a_i4DS = 0;}    

    if ((a_i4DS == 0) || (a_i4DS == 3))  {        
        m_i4DIV_V = 1;
    }
    else if (a_i4DS == 1)  {
        m_i4DIV_V = 2;
    }
    else  {
        m_i4DIV_V = 4;
    }

    EIS_BITS(pEis, EIS_PREP_ME_CTRL, PREP_GAIN_IIR_V) = a_i4Gain;
    EIS_BITS(pEis, EIS_PREP_ME_CTRL, PREP_DS_IIR_V)   = a_i4DS;
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::setHFilter(MINT32 a_i4Gain, MINT32 a_i4DS, MINT32 a_i4GainFIR, MINT32 a_i4FIR)
{
    EIS_LOG("[setHFilter] G: %d, DS: %d, GFIR: %d, FIR: %d\n", a_i4Gain, a_i4DS, a_i4GainFIR, a_i4FIR);
    // Gain         [0: 1]                [1: 2]
    // DeSample [0: 1->2]          [1: 1->4]
    // GainFIR    [0: 1/2]             [1: 1]
    // FIR           [0: 1-z^-16]     [1: 1-z^-32]
    
    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    if (a_i4Gain > 1)      {a_i4Gain = 1;}
    if (a_i4Gain < 0)      {a_i4Gain = 0;}    
    if (a_i4DS > 1)        {a_i4DS = 1;}
    if (a_i4DS < 0)        {a_i4DS = 0;}    
    if (a_i4GainFIR > 1)   {a_i4GainFIR = 1;}
    if (a_i4GainFIR < 0)   {a_i4GainFIR = 0;} 
    if (a_i4FIR > 1)       {a_i4FIR = 1;}
    if (a_i4FIR < 0)       {a_i4FIR = 0;}     

    if (a_i4DS == 0)  {        
        m_i4DIV_H = 2;
    }
    else  {
        m_i4DIV_H = 4;
    }

    EIS_BITS(pEis, EIS_PREP_ME_CTRL, PREP_GAIN_H)    = a_i4Gain;
    EIS_BITS(pEis, EIS_PREP_ME_CTRL, PREP_DS_IIR_H)  = a_i4DS;
    EIS_BITS(pEis, EIS_PREP_ME_CTRL, PREP_GAIN_FIR_H)= a_i4GainFIR;
    EIS_BITS(pEis, EIS_PREP_ME_CTRL, PREP_FIR_H)     = a_i4FIR;        
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::setLMV_TH(MINT32 a_i4H_Center, MINT32 a_i4H_Surr, MINT32 a_i4V_Center, MINT32 a_i4V_Surr)
{
    EIS_LOG("[setLMV_TH] HC: %d, HS: %d, VC: %d, VS: %d\n", a_i4H_Center, a_i4H_Surr, a_i4V_Center, a_i4V_Surr);
   
    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    if (a_i4H_Center < 0)    {a_i4H_Center = 0;}
    if (a_i4H_Center > 255)  {a_i4H_Center = 255;}    
    if (a_i4H_Surr < 0)      {a_i4H_Surr   = 0;}
    if (a_i4H_Surr > 255)    {a_i4H_Surr   = 255;}    
    if (a_i4V_Center < 0)    {a_i4V_Center = 0;}
    if (a_i4V_Center > 255)  {a_i4V_Center = 255;}    
    if (a_i4V_Surr < 0)      {a_i4V_Surr   = 0;}
    if (a_i4V_Surr > 255)    {a_i4V_Surr   = 255;}    
    
    EIS_BITS(pEis, EIS_LMV_TH, LMV_TH_X_CENTER) = a_i4H_Center;
    EIS_BITS(pEis, EIS_LMV_TH, LMV_TH_X_SURROUND) = a_i4H_Surr;
    EIS_BITS(pEis, EIS_LMV_TH, LMV_TH_Y_CENTER) = a_i4V_Center;
    EIS_BITS(pEis, EIS_LMV_TH, LMV_TH_Y_SURROUND) = a_i4V_Surr;
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::setFLoffset(MINT32 a_i4FL_H, MINT32 a_i4FL_V)
{
    EIS_LOG("[setFLoffset (double buffer)] H: %d, V: %d\n", a_i4FL_H, a_i4FL_V);
   
    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    if (a_i4FL_H > m_i4FL_Max_H)   {a_i4FL_H = m_i4FL_Max_H;}
    if (a_i4FL_H < -m_i4FL_Max_H)  {a_i4FL_H = -m_i4FL_Max_H;}    
    if (a_i4FL_V > m_i4FL_Max_V)   {a_i4FL_V = m_i4FL_Max_V;}
    if (a_i4FL_V < -m_i4FL_Max_V)  {a_i4FL_V = -m_i4FL_Max_V;}    
    
    EIS_BITS(pEis, EIS_FL_OFFSET, FL_OFFSET_H) = a_i4FL_H;
    EIS_BITS(pEis, EIS_FL_OFFSET, FL_OFFSET_V) = a_i4FL_V;
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::setFLoffsetMax(MINT32 a_i4FL_Max_H, MINT32 a_i4FL_Max_V)
{
    EIS_LOG("[setFLoffsetMax] H: %d, V: %d\n", a_i4FL_Max_H, a_i4FL_Max_V);

    m_i4FL_Max_H = a_i4FL_Max_H;
    m_i4FL_Max_V = a_i4FL_Max_V;
    
    if (m_i4FL_Max_H > 23)   {m_i4FL_Max_H = 23;}     
    if (m_i4FL_Max_H < 0)    {m_i4FL_Max_H = 0;} 
    if (m_i4FL_Max_V > 23)   {m_i4FL_Max_V = 23;} 
    if (m_i4FL_Max_V < 0)    {m_i4FL_Max_V = 0;}     
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::getFLoffsetMax(MINT32 &a_i4FL_Max_H, MINT32 &a_i4FL_Max_V)
{
    a_i4FL_Max_H = m_i4FL_Max_H;
    a_i4FL_Max_V = m_i4FL_Max_V;

    EIS_LOG("[getFLoffsetMax] H: %d, V: %d\n", a_i4FL_Max_H, a_i4FL_Max_V);    
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::setMB_H(MINT32 a_i4MBoffset_H, MINT32 a_i4MBinterval_H)
{
    EIS_LOG("[setMB_H] os: %d, interval: %d\n", a_i4MBoffset_H, a_i4MBinterval_H);
   
    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    if (a_i4MBoffset_H   > 4095)   {a_i4MBoffset_H   = 4095;}
    if (a_i4MBoffset_H   < 0)      {a_i4MBoffset_H   = 0;}    
    if (a_i4MBinterval_H > 4095)   {a_i4MBinterval_H = 4095;}
    if (a_i4MBinterval_H < 0)      {a_i4MBinterval_H = 0;}    
    
    EIS_BITS(pEis, EIS_MB_OFFSET, MB_OFFSET_H) = a_i4MBoffset_H;
    EIS_BITS(pEis, EIS_MB_INTERVAL, MB_INTERVAL_H) = a_i4MBinterval_H;    
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::setMB_V(MINT32 a_i4MBoffset_V, MINT32 a_i4MBinterval_V)
{
    EIS_LOG("[setMB_V] os: %d, interval: %d\n", a_i4MBoffset_V, a_i4MBinterval_V);
   
    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    if (a_i4MBoffset_V   > 4095)   {a_i4MBoffset_V   = 4095;}
    if (a_i4MBoffset_V   < 0)      {a_i4MBoffset_V   = 0;}    
    if (a_i4MBinterval_V > 4095)   {a_i4MBinterval_V = 4095;}
    if (a_i4MBinterval_V < 0)      {a_i4MBinterval_V = 0;}    

    EIS_BITS(pEis, EIS_MB_OFFSET, MB_OFFSET_V) = a_i4MBoffset_V;   
    EIS_BITS(pEis, EIS_MB_INTERVAL, MB_INTERVAL_V) = a_i4MBinterval_V;
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::setIRQ_WR_CLR(bool a_bIRQ_WR_CLR)
{
    EIS_LOG("[set EIS IRQ write clear: 1 write clear] %d\n", a_bIRQ_WR_CLR);

    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    EIS_BITS(pEis, EIS_DBG_CTRL, EIS_IRQ_WR_CLR) = a_bIRQ_WR_CLR;    
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::setIRQ_EN(bool a_bIRQ_EN)
{
    EIS_LOG("[set EIS IRQ enable: 1 enable] %d\n", a_bIRQ_EN);

    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    EIS_BITS(pEis, EIS_DBG_CTRL, EIS_IRQ_ENABLE) = a_bIRQ_EN;    
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::set0024H(bool a_bPipe_Mode, MINT32 a_i4Img_Width, MINT32 a_i4Img_Height)
{
    EIS_LOG("[set EIS 0024H: pipe mode] %d\n", a_bPipe_Mode);

    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    // bit 31 : pipe mode
    // bit 16~28: Width
    // bit 0~12  : Height
    EIS_REG(pEis, EIS_RESERVED1) = (a_bPipe_Mode<<31) | (a_i4Img_Width<<16) | (a_i4Img_Height);
}

/*******************************************************************************
*
********************************************************************************/
bool EisDrv::getIRQ_State()
{
    MINT32 ret = 0;
    MINT32 vlu = 0;

    EIS_REG_T eis_reg;
        
    ret = ioctl(mfd, MT_EIS_IOC_T_DUMP_REG, &eis_reg);
    if (ret < 0) {
        EIS_ERR("MT_EIS_IOC_T_DUMP_REG err \n");
        return 0;
    }
    else   {
        vlu = (eis_reg.val[7] & 0x00100000)>>20;

        return vlu;
    }
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::resetEIS(bool a_bReset)
{
    EIS_LOG("[software reset EIS: 0 enable] %d\n", a_bReset);

    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    EIS_BITS(pEis, EIS_DBG_CTRL, EIS_SWRST_B) = a_bReset;    
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::setStatAddr(MINT32 a_PA, MINT32 a_VA)
{
    EIS_LOG("[set EIS statistic base address] PA %x, VA %x\n", a_PA, a_VA);

    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    EIS_BITS(pEis, EIS_BASE_ADDR, ADDR) = a_PA;

    m_pEIS_Stat = (eis_ori_stat_t *)a_VA;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrv::dumpReg()
{
    MINT32 ret = 0;

    EIS_REG_T eis_reg;
        
    EIS_LOG("[dumpReg] \n");

    ret = ioctl(mfd, MT_EIS_IOC_T_DUMP_REG, &eis_reg);
    if (ret < 0) {
        EIS_ERR("MT_EIS_IOC_T_DUMP_REG err \n");
    }
    else   {

        EIS_LOG("[CE] %d [SRC] %d [KNEE] %2d [CLIP] %2d [RP] %2d\n"
                               , (eis_reg.val[0]>>31)&0x1
                               , (eis_reg.val[0]>>30)&0x1
                               , (eis_reg.val[0]>>16)&0xf
                               , (eis_reg.val[0]>>12)&0xf
                               , (eis_reg.val[0]>>8)&0xf);

        EIS_LOG("[V_Gain] %d [V_DS] %d\n"
                               , (eis_reg.val[0]>>6)&0x3
                               , (eis_reg.val[0]>>4)&0x3);

        EIS_LOG("[H_Gain] %d [H_DS] %d [H_FIRG] %d, [H_FIR] %d\n"
                               , (eis_reg.val[0]>>3)&0x1
                               , (eis_reg.val[0]>>2)&0x1
                               , (eis_reg.val[0]>>1)&0x1
                               , (eis_reg.val[0]>>0)&0x1);
        
        EIS_LOG("[TH_XC] %5d, [TH_XS] %5d, [TH_YC] %5d, [TH_YS] %5d\n"
                               , (eis_reg.val[1]>>24)&0xff
                               , (eis_reg.val[1]>>16)&0xff
                               , (eis_reg.val[1]>>8)&0xff
                               , (eis_reg.val[1]>>0)&0xff);
        
        EIS_LOG("[FL_X] %5d, [FL_Y] %5d\n"
                               , (eis_reg.val[2]>>16)&0xfff
                               , (eis_reg.val[2]>>0)&0xfff);

        EIS_LOG("[OS_X] %5d, [IN_X] %5d\n"
                               , (eis_reg.val[3]>>16)&0xfff
                               , (eis_reg.val[4]>>16)&0xfff);

        EIS_LOG("[OS_Y] %5d, [IN_Y] %5d\n"
                               , (eis_reg.val[3]>>0)&0xfff
                               , (eis_reg.val[4]>>0)&0xfff);

        EIS_LOG("[GMV_X] %5d, [GMV_Y] %5d\n"
                               , (eis_reg.val[5]>>16)&0xfff
                               , (eis_reg.val[5]>>0)&0xfff);
        
        EIS_LOG("[DBG_OUT] %x \n", eis_reg.val[6]);
        EIS_LOG("[DBG_CTL] %x \n", eis_reg.val[7]);
        EIS_LOG("[BAS_ADR] %x \n", eis_reg.val[10]);

        //EIS_LOG("[70096004] %x \n", eis_reg.val[1]);        
        //EIS_LOG("[70096020] %x \n", eis_reg.val[8]);
        //EIS_LOG("[70096024] %x \n", eis_reg.val[9]);


        EIS_LOG("[CRZ src size] %x \n", eis_reg.val[12]);
        EIS_LOG("[CRZ tar size] %x \n", eis_reg.val[14]);
        //EIS_LOG("[%x] %x \n", eis_reg.val[15], eis_reg.val[16]);
        //EIS_LOG("[%x] %x \n", eis_reg.val[17], eis_reg.val[18]);
        //EIS_LOG("[%x] %x \n", eis_reg.val[19], eis_reg.val[20]);
        
        EIS_LOG("[FL_H_Max] %d [FL_V_Max] %d\n", m_i4FL_Max_H, m_i4FL_Max_V);
        
        EIS_LOG("[DIV_H] %d [DIV_V] %d\n", m_i4DIV_H, m_i4DIV_V);
    }
    
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrv::AutoConfig(MINT32 a_i4Path, MINT32 a_i4Width, MINT32 a_i4Height)
{
    MINT32 i4Offset_H, i4Offset_V, i4Interval, i4RP;
    MINT32 i4Div_H, i4Div_V;

    eis_reg_t *pEis = (eis_reg_t *) mpEis;

    EIS_LOG("[AutoConfig] Path : %d, W: %d, H: %d\n", a_i4Path, a_i4Width, a_i4Height);

    enableEIS(0);

    // reset EIS
    //resetEIS(0);
    //wait
    //resetEIS(1);
    
    setSrcSel((bool)a_i4Path);
    setSRAM_PD(0);

    // reset SRAM
    //setSRAM_RB(0);
    //wait    
    //setSRAM_RB(1);
    
    setKneeClip(1,2);
    setFLoffset(0,0);

    // ------------------ V ---------------------
    // 160 = 4x(8x5)
    if      (a_i4Height < 160)                           {return -1;}
    else if (a_i4Height < max(320,IMG_HEIGHT_THRES_1))   {i4Div_V = 1;}
    else if (a_i4Height < max(640,IMG_HEIGHT_THRES_2))   {i4Div_V = 2;}
    else                                                 {i4Div_V = 4;}

    // assume to place 4 window equally according to image width.
    // |Gap|--40--|Gap|--40--|Gap|--40--|Gap|--40--|Gap|    
    // Gap formula: original image width -> subsample -> minus 40x4 -> div by 5 gap.
    // interval = 40 + gap.
    // if Gap trucate, MB_offset will not equal 8+Gap, we need to calculate by :
    // original Img width -> subsample -> minus 40 & 3xinterval -> div by 2
    
    i4Interval = 40 + (a_i4Height/i4Div_V - 160)/7;
    i4Offset_V = 8 + ((a_i4Height/i4Div_V - 40 - 3*i4Interval)>>1);

    setVFilter(1,(i4Div_V>>1));        
    setMB_V(i4Offset_V,i4Interval);

    // ------------------ H ---------------------
    if      (a_i4Width < 128)   { 
        return -1;
    }
    else if (a_i4Width < 192)   { 
        i4RP = 1;
        i4Div_H = 2;
    }    
    else if (a_i4Width < max(320,IMG_WIDTH_THRES_1))   {
        i4RP = 2;
        i4Div_H = 2;
    }    
    else if (a_i4Width < max(448,IMG_WIDTH_THRES_2))   {
        i4RP = 4;                
        i4Div_H = 2;
    }
    else if (a_i4Width < max(576,IMG_WIDTH_THRES_3))   {
        i4RP = 6;                
        i4Div_H = 2;
    }
    else if (a_i4Width < max(704,IMG_WIDTH_THRES_4))   {
        i4RP = 8;                
        i4Div_H = 2;
    }
    else   {        
        i4RP = 8;                
        i4Div_H = 4;
    }

    i4Interval = 8*(i4RP+1) + (a_i4Width/i4Div_H - 32*(i4RP+1))/5;
    i4Offset_H = 8 + ((a_i4Width/i4Div_H - 8*(i4RP+1) - 3*i4Interval)>>1);

    setHRP(i4RP);
    setHFilter(0,(i4Div_H>>2),0,0);
    setMB_H(i4Offset_H,i4Interval);

    // ------------- FL offset Max ----------------
    // MB_offset_H = 11 + FL_offset_H
    // MB_offset_V =  9 + FL_offset_V    
    setFLoffsetMax(i4Offset_H-11,i4Offset_V-9);

    setLMV_TH(0,0,0,0);
    setIRQ_WR_CLR(0);
    setIRQ_EN(1);
    
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrv::getHW_GMV(MINT32 &a_i4GMV_X, MINT32 &a_i4GMV_Y)
{
    MINT32 a, b;    
    MINT32 cnt = 0, sum = 0;
    
    //EIS_LOG("[getHW_GMV]\n");

    do {
        if (getIRQ_State())
        {                
            a = Complement2(EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD2, GMV_X), LMV_DIGIT) * m_i4DIV_H;
            b = Complement2(EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD2, GMV_Y), LMV_DIGIT) * m_i4DIV_V;           
            
            //EIS_LOG("[GMV X]%5d [GMV Y]%5d\n", a, b);            
        
            return 0;
        }
        cnt++;

        for (MINT32 i=0; i<1000; i++)
        {
            sum++;
        }        
    } while (cnt < 1000);

    EIS_LOG("[getHW_GMV fail]\n");
    return -1;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrv::getOriStat(eis_ori_stat_t &a_EIS_Ori_Stat)    
{
    MINT32 cnt = 0, sum = 0;
    
    do {
        if (getIRQ_State())
        {                            
            a_EIS_Ori_Stat.EIS_STAT_AVGSAD0 = m_pEIS_Stat->EIS_STAT_AVGSAD0;            
            a_EIS_Ori_Stat.EIS_STAT_AVGSAD1 = m_pEIS_Stat->EIS_STAT_AVGSAD1;            
            a_EIS_Ori_Stat.EIS_STAT_AVGSAD2 = m_pEIS_Stat->EIS_STAT_AVGSAD2;            
            a_EIS_Ori_Stat.EIS_STAT_NEWTRUST[0] = m_pEIS_Stat->EIS_STAT_NEWTRUST[0];
            a_EIS_Ori_Stat.EIS_STAT_NEWTRUST[1] = m_pEIS_Stat->EIS_STAT_NEWTRUST[1];
            a_EIS_Ori_Stat.EIS_STAT_NEWTRUST[2] = m_pEIS_Stat->EIS_STAT_NEWTRUST[2];
            a_EIS_Ori_Stat.EIS_STAT_NEWTRUST[3] = m_pEIS_Stat->EIS_STAT_NEWTRUST[3];
            
            for (MINT32 i=0; i<EIS_WIN_NUM; i++)
            {
                a_EIS_Ori_Stat.EIS_STAT_MB[i] = m_pEIS_Stat->EIS_STAT_MB[i];                
            }
        
            return 0;
        }
        cnt++;

        for (MINT32 i=0; i<1000; i++)
        {
            sum++;
        }        
    } while (cnt < 1000);

    EIS_LOG("[getOriStat fail]\n");
    return -1;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrv::getStat(eis_stat_t &a_EIS_Stat)
{
    MINT32 mod;    
    MINT32 cnt = 0, sum = 0;
    
    //EIS_LOG("[getStat]\n");

    do {
        if (getIRQ_State())
        {                
            for (MINT32 i=0; i<EIS_WIN_NUM; i++)
            {
                a_EIS_Stat.i4LMV_X[i] = Complement2(EIS_BITS(m_pEIS_Stat, EIS_STAT_MB[i], LMV_X), LMV_DIGIT) * m_i4DIV_H;
                a_EIS_Stat.i4LMV_X2[i] = Complement2(EIS_BITS(m_pEIS_Stat, EIS_STAT_MB[i], LMV_X_2ND), LMV2_DIGIT) * m_i4DIV_H;            
                a_EIS_Stat.i4LMV_Y[i] = Complement2(EIS_BITS(m_pEIS_Stat, EIS_STAT_MB[i], LMV_Y), LMV_DIGIT) * m_i4DIV_V;
                a_EIS_Stat.i4LMV_Y2[i] = Complement2(EIS_BITS(m_pEIS_Stat, EIS_STAT_MB[i], LMV_Y_2ND), LMV2_DIGIT) * m_i4DIV_V;                        

                mod = i%4;

                if (mod == 0)
                {
                    a_EIS_Stat.i4NewTrust_X[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_NEWTRUST[i>>2], NEW_TRUST_X0);
                    a_EIS_Stat.i4NewTrust_Y[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_NEWTRUST[i>>2], NEW_TRUST_Y0);
                }
                else if (mod == 1)
                {
                    a_EIS_Stat.i4NewTrust_X[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_NEWTRUST[i>>2], NEW_TRUST_X1);
                    a_EIS_Stat.i4NewTrust_Y[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_NEWTRUST[i>>2], NEW_TRUST_Y1);
                }
                else if (mod == 2)
                {
                    a_EIS_Stat.i4NewTrust_X[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_NEWTRUST[i>>2], NEW_TRUST_X2);
                    a_EIS_Stat.i4NewTrust_Y[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_NEWTRUST[i>>2], NEW_TRUST_Y2);
                }
                else
                {
                    a_EIS_Stat.i4NewTrust_X[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_NEWTRUST[i>>2], NEW_TRUST_X3);
                    a_EIS_Stat.i4NewTrust_Y[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_NEWTRUST[i>>2], NEW_TRUST_Y3);
                }
                
                a_EIS_Stat.i4SAD[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_MB[i], SAD);
                a_EIS_Stat.i4SAD2[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_MB[i], SAD_2ND); 

                if      (i == 0)    {a_EIS_Stat.i4AVG[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD0, AVGSAD00);}
                else if (i == 1)    {a_EIS_Stat.i4AVG[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD0, AVGSAD01);}
                else if (i == 2)    {a_EIS_Stat.i4AVG[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD0, AVGSAD02);}
                else if (i == 3)    {a_EIS_Stat.i4AVG[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD0, AVGSAD03);}
                else if (i == 4)    {a_EIS_Stat.i4AVG[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD0, AVGSAD10);}
                else if (i == 5)    {a_EIS_Stat.i4AVG[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD0, AVGSAD11);}                
                else if (i == 6)    {a_EIS_Stat.i4AVG[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD0, AVGSAD12);}                
                else if (i == 7)    {a_EIS_Stat.i4AVG[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD1, AVGSAD13);}                
                else if (i == 8)    {a_EIS_Stat.i4AVG[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD1, AVGSAD20);}                
                else if (i == 9)    {a_EIS_Stat.i4AVG[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD1, AVGSAD21);}                
                else if (i == 10)   {a_EIS_Stat.i4AVG[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD1, AVGSAD22);}                                
                else if (i == 11)   {a_EIS_Stat.i4AVG[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD1, AVGSAD23);}                
                else if (i == 12)   {a_EIS_Stat.i4AVG[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD1, AVGSAD30);}                
                else if (i == 13)   {a_EIS_Stat.i4AVG[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD1, AVGSAD31);}                
                else if (i == 14)   {a_EIS_Stat.i4AVG[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD2, AVGSAD32);}
                else                {a_EIS_Stat.i4AVG[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_AVGSAD2, AVGSAD33);}                

                a_EIS_Stat.i4Trust_X[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_MB[i], TRUST_X);
                a_EIS_Stat.i4Trust_Y[i] = EIS_BITS(m_pEIS_Stat, EIS_STAT_MB[i], TRUST_Y);
                
            }
        
            return 0;
        }
        cnt++;

        for (MINT32 i=0; i<1000; i++)
        {
            sum++;
        }        
    } while (cnt < 1000);

    EIS_LOG("[getStat fail]\n");
    return -1;
}

/*******************************************************************************
*
********************************************************************************/
void EisDrv::getDIVinfo(MINT32 &a_i4DIV_H, MINT32 &a_i4DIV_V)
{
    a_i4DIV_H = m_i4DIV_H;
    a_i4DIV_V = m_i4DIV_V;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrv::max(MINT32 a, MINT32 b)
{
    if (a >= b) {return a ;}
    else        {return b ;}
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrv::Complement2(MUINT32 Vlu, MUINT32 Digit)
{
    MINT32 Result;

    if (((Vlu>>(Digit-1)) & 0x1) == 1)    // negative
    {
        Result = 0 - (MINT32)((~Vlu + 1) & ((1<<Digit)-1));        
    }
    else
    {
        Result = (MINT32)(Vlu & ((1<<Digit)-1));
    }

    return Result;
}

