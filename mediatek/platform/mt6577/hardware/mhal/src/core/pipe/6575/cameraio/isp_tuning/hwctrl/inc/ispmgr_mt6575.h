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
#ifndef _ISP_MGR_MT6575_H_
#define _ISP_MGR_MT6575_H_

/*******************************************************************************
*
*******************************************************************************/
#include "camera_custom_nvram_mt6575.h"
#include "ispif_mt6575.h"
using namespace NS_MT6575ISP_NVRAM;
using namespace NS_MT6575ISP_EFFECT;
#include "ispdrvmgr.h"
#include "isp_reg.h"


namespace NSIspTuning
{


/*******************************************************************************
* ISP Manager
*******************************************************************************/
typedef class MT6575ISP_MGR_BASE
{
protected:  ////
    typedef MT6575ISP_MGR_BASE  MyType;
    typedef IspDrvMgr::reg_t    reg_t;

protected:
    virtual ~MT6575ISP_MGR_BASE() {}
    MT6575ISP_MGR_BASE(MVOID*const pvRegs, MUINT32 const u4RegCount)
     : m_pvRegs(pvRegs)
     , m_u4RegCount(u4RegCount)
    {
    }

protected:
    MVOID*const     m_pvRegs;
    MUINT32 const   m_u4RegCount;

//==============================================================================
protected:  ////

#define REG_ADDR(field)\
    ((MUINT32)(&getIspReg()->field) - (MUINT32)getIspReg())

#define REG_FIELD(field)\
    (m_regs[EReg_##field])

#define FIELD_ADDR(field)\
    (REG_FIELD(field).addr)

#define FIELD_VAL(field)\
    (REG_FIELD(field).val)

#define FIELD_VAL_PTR(field)\
    ( & FIELD_VAL(field) )

#define INIT_REG_ADDR(field)\
    FIELD_ADDR(field) = REG_ADDR(field)

#define INIT_REG_ADDR2(field_dst, field_src)\
    FIELD_ADDR(field_dst) = REG_ADDR(field_src)

    inline
    isp_reg_t*
    getIspReg() const
    {
        return reinterpret_cast<isp_reg_t*> (
            IspDrvMgr::getInstance().getIspReg()
        );
    }

    inline
    MBOOL
    readRegs(reg_t*const pRegs, MUINT32 const count) const
    {
        return  IspDrvMgr::getInstance().readRegs(pRegs, count);
    }

    inline
    MBOOL
    writeRegs(reg_t*const pRegs, MUINT32 const count)
    {
        return  IspDrvMgr::getInstance().writeRegs(pRegs, count);
    }

    inline
    MBOOL
    writeReg(MUINT32 const u4Addr, MUINT32 const u4Val)
    {
        reg_t reg;
        reg.addr = u4Addr;
        reg.val  = u4Val;
        return  writeRegs(&reg, 1);
    }

public: ////    Interfaces.
    virtual
    MBOOL
    reset()
    {
        return  readRegs(static_cast<reg_t*>(m_pvRegs), m_u4RegCount);
    }

    virtual
    MBOOL
    apply()
    {
        return  writeRegs(static_cast<reg_t*>(m_pvRegs), m_u4RegCount);
    }

//..............................................................................
} ISP_MGR_BASE_T;

/*******************************************************************************
* Color Clip
*******************************************************************************/
typedef class MT6575ISP_MGR_CCLIP : public ISP_MGR_BASE_T
{
    typedef MT6575ISP_MGR_CCLIP    MyType;
private:
    enum
    {
        EReg_CAM_CCLIP_CON,
        EReg_CAM_CCLIP_GTC,
        EReg_CAM_CCLIP_ADC,
        EReg_CAM_CCLIP_BAC,
        ENumOfRegs
    };
    reg_t   m_regs[ENumOfRegs];

protected:
    MT6575ISP_MGR_CCLIP()
        : ISP_MGR_BASE_T(m_regs, ENumOfRegs)
    {
        INIT_REG_ADDR(CAM_CCLIP_CON); // 0x0CCH
        INIT_REG_ADDR(CAM_CCLIP_GTC); // 0x0D0H
        INIT_REG_ADDR(CAM_CCLIP_ADC); // 0x0D4H
        INIT_REG_ADDR(CAM_CCLIP_BAC); // 0x0D8H
    }

//==============================================================================
public: ////
    static MyType&  getInstance();

public: ////    Interfaces.
    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

} ISP_MGR_CCLIP_T;


/*******************************************************************************
* Gain Control
*******************************************************************************/
typedef class MT6575ISP_MGR_GAIN_CTRL : public ISP_MGR_BASE_T
{
    typedef MT6575ISP_MGR_GAIN_CTRL    MyType;
private:
    enum
    {
        EReg_CAM_CTRL1,
        ENumOfRegs
    };
    reg_t   m_regs[ENumOfRegs];

protected:
    MT6575ISP_MGR_GAIN_CTRL()
        : ISP_MGR_BASE_T(m_regs, ENumOfRegs)
    {
        INIT_REG_ADDR(CAM_CTRL1); // 0x040
    }

//==============================================================================
public: ////
    static MyType&  getInstance();

public: ////    Interfaces.
    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

} ISP_MGR_GAIN_CTRL_T;

/*******************************************************************************
* RGB2YCC Yoffset
*******************************************************************************/
typedef class MT6575ISP_MGR_RGB2YCC_YOFST : public ISP_MGR_BASE_T
{
    typedef MT6575ISP_MGR_RGB2YCC_YOFST    MyType;
private:
    enum
    {
        EReg_CAM_RGB2YCC_CON,
        ENumOfRegs
    };
    reg_t   m_regs[ENumOfRegs];

protected:
    MT6575ISP_MGR_RGB2YCC_YOFST()
        : ISP_MGR_BASE_T(m_regs, ENumOfRegs)
    {
        INIT_REG_ADDR(CAM_RGB2YCC_CON); // 0x0BC
    }

//==============================================================================
public: ////
    static MyType&  getInstance();

public: ////    Interfaces.
    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

} ISP_MGR_RGB2YCC_YOFST_T;


/*******************************************************************************
* Shading
*******************************************************************************/
typedef class MT6575ISP_MGR_SHADING : public ISP_MGR_BASE_T
{
    typedef MT6575ISP_MGR_SHADING   MyType;
private:
    enum
    {
        EReg_RAW_DATA_ACC_CFG, 
        EReg_RAW_ACC_WIN, 
        EReg_RAW_ACC_RESULT_B, 
        EReg_RAW_ACC_RESULT_GB, 
        EReg_RAW_ACC_RESULT_GR, 
        EReg_RAW_ACC_RESULT_R, 
        EReg_SHADING_CTRL1, 
        EReg_SHADING_CTRL2, 
        EReg_SHADING_READ_ADDR, 
        EReg_SHADING_LAST_BLK_CFG, 
        EReg_SHADING_RATIO_CFG, 
        ENumOfRegs
    };
    reg_t   m_regs[ENumOfRegs];

protected:
    MT6575ISP_MGR_SHADING()
        : ISP_MGR_BASE_T(m_regs, ENumOfRegs)
        ,m_u4ShadingParamChangeCount(0)
    {
        INIT_REG_ADDR2(RAW_DATA_ACC_CFG, CAM_RAWACC_REG);   // 0x01BC
        INIT_REG_ADDR2(RAW_ACC_WIN, CAM_RAWWIN_REG);        // 0x01C0
        INIT_REG_ADDR2(RAW_ACC_RESULT_B,  CAM_RAWSUM_B);    // 0x01C4
        INIT_REG_ADDR2(RAW_ACC_RESULT_GB, CAM_RAWSUM_GB);   // 0x01C8
        INIT_REG_ADDR2(RAW_ACC_RESULT_GR, CAM_RAWSUM_GR);   // 0x01C4
        INIT_REG_ADDR2(RAW_ACC_RESULT_R,  CAM_RAWSUM_R);    // 0x01D0
        INIT_REG_ADDR2(SHADING_CTRL1, CAM_SHADING1);        // 0x0214
        INIT_REG_ADDR2(SHADING_CTRL2, CAM_SHADING2);        // 0x0218
        INIT_REG_ADDR2(SHADING_READ_ADDR, CAM_SDRADDR);     // 0x021C
        INIT_REG_ADDR2(SHADING_LAST_BLK_CFG, CAM_SDLBLOCK); // 0x0220
        INIT_REG_ADDR2(SHADING_RATIO_CFG, CAM_SDRATIO);     // 0x0224
    }

//==============================================================================
public: ////
    static MyType&  getInstance();

public: ////    Interfaces.
    virtual MBOOL   apply();

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

public: ////    Set Enable/Disable
    MyType&     setEnableShading(MBOOL const fgEnable);

public: ////    Direct Set
    MBOOL       directSetTBA(MUINT32 const u4TBA);

private:    ////    Data Members.
    //  It means that any params have changed if > 0.
    MUINT32         m_u4ShadingParamChangeCount;    
} ISP_MGR_SHADING_T;


/*******************************************************************************
* OB
*******************************************************************************/
typedef class MT6575ISP_MGR_OB : public ISP_MGR_BASE_T
{
    typedef MT6575ISP_MGR_OB    MyType;
private:
    enum
    {
        EReg_COMP_OFFSET_ADJUST, 
        ENumOfRegs
    };
    reg_t   m_regs[ENumOfRegs];

protected:
    MT6575ISP_MGR_OB()
        : ISP_MGR_BASE_T(m_regs, ENumOfRegs)
    {
        INIT_REG_ADDR2(COMP_OFFSET_ADJUST, CAM_RGBOFF); // 0x090
    }

//==============================================================================
public: ////
    static MyType&  getInstance();

public: ////    Interfaces.
    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

} ISP_MGR_OB_T;


/*******************************************************************************
* DM
*******************************************************************************/
typedef class MT6575ISP_MGR_DM : public ISP_MGR_BASE_T
{
    typedef MT6575ISP_MGR_DM    MyType;
private:
    enum
    {
        EReg_COLOR_PROC_STAGE_CTRL, 
        EReg_INTERPOLATION_REG1, 
        EReg_INTERPOLATION_REG2, 
        ENumOfRegs
    };
    reg_t   m_regs[ENumOfRegs];

protected:
    MT6575ISP_MGR_DM()
        : ISP_MGR_BASE_T(m_regs, ENumOfRegs)
    {
        INIT_REG_ADDR2(COLOR_PROC_STAGE_CTRL, CAM_CPSCON1); // 0x070
        INIT_REG_ADDR2(INTERPOLATION_REG1, CAM_INTER1);     // 0x074
        INIT_REG_ADDR2(INTERPOLATION_REG2, CAM_INTER2);     // 0x078
    }

//==============================================================================
public: ////
    static MyType&  getInstance();

public: ////    Interfaces.
    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

} ISP_MGR_DM_T;


/*******************************************************************************
* NR1
*******************************************************************************/
typedef class MT6575ISP_MGR_NR1 : public ISP_MGR_BASE_T
{
    typedef MT6575ISP_MGR_NR1    MyType;
private:
    enum
    {
        EReg_NR1_CTRL, 
        EReg_NR1_DEFECT_PX_CFG1, 
        EReg_NR1_DEFECT_PX_CFG2, 
        EReg_NR1_DEFECT_PX_CFG3, 
        EReg_NR1_DEFECT_PX_CFG4, 
        EReg_NR1_CX_COMP_CFG, 
        EReg_NR1_NR_CFG1, 
        EReg_NR1_NR_CFG2, 
        EReg_NR1_NR_CFG3, 
        EReg_NR1_NR_CFG4, 
        EReg_NR1_NR_CFG5, 
        EReg_NR1_NR_CFG6, 
        EReg_NR1_NR_CFG7, 
        EReg_NR1_NR_CFG8, 
        EReg_NR1_NR_CFG9, 
        EReg_NR1_NR_CFG10, 
        EReg_NR1_NR_CFG11, 
        EReg_NR1_NR_CFG12, 
        ENumOfRegs
    };
    reg_t   m_regs[ENumOfRegs];

protected:
    MT6575ISP_MGR_NR1()
        : ISP_MGR_BASE_T(m_regs, ENumOfRegs)
    {
        INIT_REG_ADDR2(NR1_CTRL, CAM_NR1_CON);           // 0x0550
        INIT_REG_ADDR2(NR1_DEFECT_PX_CFG1, CAM_NR1_DP1); // 0x0554
        INIT_REG_ADDR2(NR1_DEFECT_PX_CFG2, CAM_NR1_DP2); // 0x0558
        INIT_REG_ADDR2(NR1_DEFECT_PX_CFG3, CAM_NR1_DP3); // 0x055C
        INIT_REG_ADDR2(NR1_DEFECT_PX_CFG4, CAM_NR1_DP4); // 0x0560
        INIT_REG_ADDR2(NR1_CX_COMP_CFG, CAM_NR1_CT);     // 0x0564
        INIT_REG_ADDR2(NR1_NR_CFG1, CAM_NR1_NR1);        // 0x0568
        INIT_REG_ADDR2(NR1_NR_CFG2, CAM_NR1_NR2);        // 0x056C
        INIT_REG_ADDR2(NR1_NR_CFG3, CAM_NR1_NR3);        // 0x0570
        INIT_REG_ADDR2(NR1_NR_CFG4, CAM_NR1_NR4);        // 0x0574
        INIT_REG_ADDR2(NR1_NR_CFG5, CAM_NR1_NR5);        // 0x0578
        INIT_REG_ADDR2(NR1_NR_CFG6, CAM_NR1_NR6);        // 0x057C
        INIT_REG_ADDR2(NR1_NR_CFG7, CAM_NR1_NR7);        // 0x0580
        INIT_REG_ADDR2(NR1_NR_CFG8, CAM_NR1_NR8);        // 0x0584
        INIT_REG_ADDR2(NR1_NR_CFG9, CAM_NR1_NR9);        // 0x0588
        INIT_REG_ADDR2(NR1_NR_CFG10, CAM_NR1_NR10);      // 0x058C
        INIT_REG_ADDR2(NR1_NR_CFG11, CAM_NR1_NR11);      // 0x0590
        INIT_REG_ADDR2(NR1_NR_CFG12, CAM_NR1_NR12);      // 0x0594
    }

//==============================================================================
public: ////
    static MyType&  getInstance();

public: ////    Interfaces.
    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

public: ////    Set Enable/Disable
    MyType&     setEnableDP(MBOOL const fgEnable);
    MyType&     setEnableCT(MBOOL const fgEnable);
    MyType&     setEnableNR(MBOOL const fgEnable);

} ISP_MGR_NR1_T;


/*******************************************************************************
* NR2
*******************************************************************************/
typedef class MT6575ISP_MGR_NR2 : public ISP_MGR_BASE_T
{
    typedef MT6575ISP_MGR_NR2    MyType;
private:
    enum
    {
        EReg_NR2_CTRL, 
        EReg_NR2_COMM_CFG1, 
        EReg_NR2_CFG2, 
        EReg_NR2_CFG3, 
        EReg_NR2_CFG4, 
        EReg_NR2_COMM_CFG2, 
        EReg_NR2_LCE_CFG1, 
        EReg_NR2_LCE_CFG2, 
        EReg_NR2_MODE1_CFG1, 
        EReg_NR2_MODE1_CFG2, 
        EReg_NR2_MODE1_CFG3, 
        ENumOfRegs
    };
    reg_t   m_regs[ENumOfRegs];

protected:
    MT6575ISP_MGR_NR2()
        : ISP_MGR_BASE_T(m_regs, ENumOfRegs)
    {
        INIT_REG_ADDR2(NR2_CTRL, CAM_NR2_CON);          // 0x0500
        INIT_REG_ADDR2(NR2_COMM_CFG1, CAM_NR2_CFG_C1);  // 0x0504
        INIT_REG_ADDR2(NR2_CFG2, CAM_NR2_CFG2);         // 0x0508
        INIT_REG_ADDR2(NR2_CFG3, CAM_NR2_CFG3);         // 0x050C
        INIT_REG_ADDR2(NR2_CFG4, CAM_NR2_CFG4);         // 0x0510
        INIT_REG_ADDR2(NR2_COMM_CFG2, CAM_NR2_CFG_C2);  // 0x0514
        INIT_REG_ADDR2(NR2_LCE_CFG1, CAM_NR2_CFG_L1);   // 0x0518
        INIT_REG_ADDR2(NR2_LCE_CFG2, CAM_NR2_CFG_L2);   // 0x051C
        INIT_REG_ADDR2(NR2_MODE1_CFG1, CAM_NR2_CFG_N1); // 0x0520
        INIT_REG_ADDR2(NR2_MODE1_CFG2, CAM_NR2_CFG_N2); // 0x0524
        INIT_REG_ADDR2(NR2_MODE1_CFG3, CAM_NR2_CFG_N3); // 0x0528
    }

//==============================================================================
public: ////
    static MyType&  getInstance();

public: ////    Interfaces.
    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

public: ////    Set Enable/Disable
    MyType&     setDisable();

} ISP_MGR_NR2_T;


/*******************************************************************************
* EDGE
*******************************************************************************/
typedef class MT6575ISP_MGR_EDGE : public ISP_MGR_BASE_T
{
    typedef MT6575ISP_MGR_EDGE     MyType;
private:
    enum
    {
        EReg_EDGE_CORE, 
        EReg_EDGE_GAIN_REG1, 
        EReg_EDGE_GAIN_REG2, 
        EReg_EDGE_THRESHOLD, 
        EReg_EDGE_VERT_CTRL, 
        ENumOfRegs
    };
    reg_t   m_regs[ENumOfRegs];

protected:
    MT6575ISP_MGR_EDGE()
        : ISP_MGR_BASE_T(m_regs, ENumOfRegs)
    {
        INIT_REG_ADDR2(EDGE_CORE, CAM_EDGCORE);       // 0x05B8
        INIT_REG_ADDR2(EDGE_GAIN_REG1, CAM_EDGGAIN1); // 0x05BC
        INIT_REG_ADDR2(EDGE_GAIN_REG2, CAM_EDGGAIN2); // 0x05C0
        INIT_REG_ADDR2(EDGE_THRESHOLD, CAM_EDGTHRE);  // 0x05C4
        INIT_REG_ADDR2(EDGE_VERT_CTRL, CAM_EDGVCON);  // 0x05C8
    }

//==============================================================================
public: ////
    static MyType&  getInstance();

public: ////    Interfaces.
    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

public: ////    Set Enable/Disable
    MyType&     setDisable();

} ISP_MGR_EDGE_T;


/*******************************************************************************
* EE
*******************************************************************************/
typedef class MT6575ISP_MGR_EE : public ISP_MGR_BASE_T
{
    typedef MT6575ISP_MGR_EE     MyType;
private:
    enum
    {
        EReg_COLOR_PROC_STAGE_CTRL2, 
        EReg_EDGE_ENHANCE_CTRL, 
        EReg_EDGE_DETECT_CTRL1,
        EReg_EDGE_DETECT_CTRL2,
        EReg_EDGE_DETECT_CTRL3,
        EReg_EDGE_DETECT_CTRL4,
        EReg_EDGE_DETECT_CTRL5,
        EReg_EDGE_GAMMA_CFG1, 
        EReg_EDGE_GAMMA_CFG2, 
        EReg_EDGE_GAMMA_CFG3, 
        ENumOfRegs
    };
    reg_t   m_regs[ENumOfRegs];

protected:
    MT6575ISP_MGR_EE()
        : ISP_MGR_BASE_T(m_regs, ENumOfRegs)
    {
        INIT_REG_ADDR2(COLOR_PROC_STAGE_CTRL2, CAM_CPSCON2); // 0x00AC
        INIT_REG_ADDR2(EDGE_ENHANCE_CTRL, CAM_EE_CTRL);      // 0x05A0
        INIT_REG_ADDR2(EDGE_DETECT_CTRL1, CAM_EDCTRL1);      // 0x05A4
        INIT_REG_ADDR2(EDGE_DETECT_CTRL2, CAM_EDCTRL2);      // 0x05A8
        INIT_REG_ADDR2(EDGE_DETECT_CTRL3, CAM_EDCTRL3);      // 0x05AC
        INIT_REG_ADDR2(EDGE_DETECT_CTRL4, CAM_EDCTRL4);      // 0x05B0
        INIT_REG_ADDR2(EDGE_DETECT_CTRL5, CAM_EDCTRL5);      // 0x05B4
        INIT_REG_ADDR2(EDGE_GAMMA_CFG1, CAM_EGAMMA1);        // 0x05CC
        INIT_REG_ADDR2(EDGE_GAMMA_CFG2, CAM_EGAMMA2);        // 0x05D0
        INIT_REG_ADDR2(EDGE_GAMMA_CFG3, CAM_EGAMMA3);        // 0x05D4
    }

//==============================================================================
public: ////
    static MyType&  getInstance();

public: ////    Interfaces.
    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

public: ////    Set Enable/Disable
    MyType&     setDisable();
    MyType&     setEnableGamma(MBOOL const fgEnable);

} ISP_MGR_EE_T;


/*******************************************************************************
* YCCGO
*******************************************************************************/
typedef class MT6575ISP_MGR_YCCGO : public ISP_MGR_BASE_T
{
    typedef MT6575ISP_MGR_YCCGO     MyType;
private:
    enum
    {
        EReg_YCCGO_CTRL, 
        EReg_YCCGO_CFG1, 
        EReg_YCCGO_CFG2, 
        EReg_YCCGO_CFG3, 
        EReg_YCCGO_CFG4, 
        EReg_YCCGO_CFG5, 
        EReg_YCCGO_CFG6, 
        EReg_YCCGO_CFG7, 
        EReg_YCCGO_CFG8, 
        EReg_YCCGO_CFG9, 
        ENumOfRegs
    };
    reg_t   m_regs[ENumOfRegs];

protected:
    MT6575ISP_MGR_YCCGO()
        : ISP_MGR_BASE_T(m_regs, ENumOfRegs)
    {
        INIT_REG_ADDR2(YCCGO_CTRL, CAM_YCCGO_CON);  // 0x0600
        INIT_REG_ADDR2(YCCGO_CFG1, CAM_YCCGO_CFG1); // 0x0604
        INIT_REG_ADDR2(YCCGO_CFG2, CAM_YCCGO_CFG2); // 0x0608
        INIT_REG_ADDR2(YCCGO_CFG3, CAM_YCCGO_CFG3); // 0x060C
        INIT_REG_ADDR2(YCCGO_CFG4, CAM_YCCGO_CFG4); // 0x0610
        INIT_REG_ADDR2(YCCGO_CFG5, CAM_YCCGO_CFG5); // 0x0614
        INIT_REG_ADDR2(YCCGO_CFG6, CAM_YCCGO_CFG6); // 0x0618
        INIT_REG_ADDR2(YCCGO_CFG7, CAM_YCCGO_CFG7); // 0x061C
        INIT_REG_ADDR2(YCCGO_CFG8, CAM_YCCGO_CFG8); // 0x0620
        INIT_REG_ADDR2(YCCGO_CFG9, CAM_YCCGO_CFG9); // 0x0624
    }

//==============================================================================
public: ////
    static MyType&  getInstance();

public: ////    Interfaces.
    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

public: ////    Set Enable/Disable
    MyType&     setDisable();

} ISP_MGR_YCCGO_T;


/*******************************************************************************
* CCM
*******************************************************************************/
typedef class MT6575ISP_MGR_CCM : public ISP_MGR_BASE_T
{
    typedef MT6575ISP_MGR_CCM       MyType;
private:
    enum
    {
        EReg_COLOR_MATRIX1, 
        EReg_COLOR_MATRIX2, 
        EReg_COLOR_MATRIX3, 
        EReg_COLOR_MATRIX4,
        EReg_COLOR_MATRIX5,
        ENumOfRegs
    };
    reg_t   m_regs[ENumOfRegs];

protected:
    MT6575ISP_MGR_CCM()
        : ISP_MGR_BASE_T(m_regs, ENumOfRegs)
    {
        INIT_REG_ADDR2(COLOR_MATRIX1, CAM_MATRIX1); // 0x0094
        INIT_REG_ADDR2(COLOR_MATRIX2, CAM_MATRIX2); // 0x0098
        INIT_REG_ADDR2(COLOR_MATRIX3, CAM_MATRIX3); // 0x009C
        INIT_REG_ADDR2(COLOR_MATRIX4, CAM_MATRIX4); // 0x00A0
        INIT_REG_ADDR2(COLOR_MATRIX5, CAM_MATRIX5); // 0x00A4
    }

//==============================================================================
public: ////
    static MyType&  getInstance();

public: ////    Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

} ISP_MGR_CCM_T;


/*******************************************************************************
* GAMMA
*******************************************************************************/
typedef class MT6575ISP_MGR_GAMMA : public ISP_MGR_BASE_T
{
    typedef MT6575ISP_MGR_GAMMA     MyType;
private:
    enum
    {
        EReg_GAMMA_REG1, 
        EReg_GAMMA_REG2, 
        EReg_GAMMA_REG3, 
        EReg_GAMMA_REG4, 
        EReg_GAMMA_REG5, 
        ENumOfRegs
    };
    reg_t   m_regs[ENumOfRegs];

protected:
    MT6575ISP_MGR_GAMMA()
        : ISP_MGR_BASE_T(m_regs, ENumOfRegs)
    {
        INIT_REG_ADDR2(GAMMA_REG1, CAM_GMA_REG1); // 0x01A8
        INIT_REG_ADDR2(GAMMA_REG2, CAM_GMA_REG2); // 0x01AC
        INIT_REG_ADDR2(GAMMA_REG3, CAM_GMA_REG3); // 0x01B0
        INIT_REG_ADDR2(GAMMA_REG4, CAM_GMA_REG4); // 0x01B4
        INIT_REG_ADDR2(GAMMA_REG5, CAM_GMA_REG5); // 0x01B8
    }

//==============================================================================
public: ////
    static MyType&  getInstance();

public: ////    Interfaces.
    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

} ISP_MGR_GAMMA_T;


/*******************************************************************************
* PCA
*******************************************************************************/
typedef class MT6575ISP_MGR_PCA : public ISP_MGR_BASE_T
{
    typedef MT6575ISP_MGR_PCA     MyType;
private:
    enum
    {
        EReg_PCA_CTRL, 
        EReg_PCA_GMC_CTRL, 
        ENumOfRegs
    };
    reg_t   m_regs[ENumOfRegs];

protected:
    MT6575ISP_MGR_PCA()
        : ISP_MGR_BASE_T(m_regs, ENumOfRegs)
    {
        INIT_REG_ADDR2(PCA_CTRL, CAM_PCA_CON);     // 0x0630
        INIT_REG_ADDR2(PCA_GMC_CTRL, CAM_PCA_GMC); // 0x0638
    }

//==============================================================================
public: ////
    static MyType&  getInstance();

public: ////    Interfaces.
    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

public: ////    Set Enable/Disable
    MyType&     setDisable();

public: ////    Direct Set
    MBOOL       directSetTBA(MUINT32 const u4TBA);

} ISP_MGR_PCA_T;


//------------------------------------------------------------------------------
};  //  NSIspTuning


/*******************************************************************************
* 
*******************************************************************************/
#include "ispmgr_helper.h"


/*******************************************************************************
* 
*******************************************************************************/


#endif // _ISP_MGR_MT6575_H_

