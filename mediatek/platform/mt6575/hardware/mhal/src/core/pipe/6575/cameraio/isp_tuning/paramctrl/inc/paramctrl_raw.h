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
#ifndef _PARAM_CTRL_RAW_H_
#define _PARAM_CTRL_RAW_H_

/*******************************************************************************
*
*******************************************************************************/
#include "camera_custom_nvram.h"
#include "ispmgr_mt6575.h"
#include "paramctrl_comm.h"
#include "sysram_drv_mgr.h"
#include "camera_custom_if.h"
#include "pcamgr_mt6575.h"
#include "lsc_mgr.h"


namespace NSIspTuning
{


/*******************************************************************************
* Parameter Control - RAW Sensor
*******************************************************************************/
class ParamctrlRAW : public ParamctrlTemplate<ESensorType_RAW>
{
public:
    typedef ParamctrlTemplate<ESensorType_RAW>  Parent_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    static ParamctrlRAW*    createInstance(ESensorRole_T const eSensorRole);
    virtual void            destroyInstance();

protected:  ////    Ctor/Dtor.
    ParamctrlRAW(
        ESensorRole_T const eSensorRole, 
        NVRAM_CAMERA_ISP_PARAM_STRUCT*const pNvram_Isp, 
        NVRAM_CAMERA_DEFECT_STRUCT*const    pNvram_Defect, 
        NVRAM_CAMERA_SHADING_STRUCT*const   pNvram_Shading
    );
    ~ParamctrlRAW();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Attributes

    virtual MBOOL       isToInvokeOfflineCapture() const;

    virtual MERROR_ENUM setIso(MUINT32 const u4ISOValue);

    virtual MERROR_ENUM setCCT(MINT32 const i4CCT);
    virtual MERROR_ENUM setCCTIndex_CCM (
                MINT32 const i4CCT, 
                MINT32 const i4FluorescentIndex
            );
    virtual MERROR_ENUM setCCTIndex_Shading(
                MINT32 const i4CCT,
                MINT32 const i4DaylightFluorescentIndex,
                MINT32 const i4DaylightProb,
                MINT32 const i4DaylightFluorescentProb,
                MINT32 const i4SceneLV);
    virtual MERROR_ENUM setIndex_Shading(MINT32 const i4IDX);
    virtual MERROR_ENUM getIndex_Shading(MVOID*const pCmdArg);

    virtual MERROR_ENUM setZoomRatio(MUINT32 const u4ZoomRatio_x100);
    virtual MERROR_ENUM setSceneLightValue(MINT32 const i4SceneLV_x10);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Operations.
    virtual MERROR_ENUM construct();
    virtual MERROR_ENUM uninit();

protected:  ////    Invoked only by init().
    virtual MERROR_ENUM do_init();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Exif.
    virtual MERROR_ENUM queryExifDebugInfo(
        NSIspExifDebug::IspExifDebugInfo_T& rExifDebugInfo
    ) const;

protected:  ////

    MERROR_ENUM         saveDebugInfo();

    mutable NSIspExifDebug::IspDebugInfo    m_stIspDebugInfo;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation: Frameless
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    All
    virtual MBOOL   prepareHw_Frameless_All();
    virtual MBOOL   applyToHw_Frameless_All();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////    Module.
    MBOOL   prepare_Frameless_Pca();
    MBOOL   prepare_Frameless_Shading();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation: Per-Frame
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Invoked only by validatePerFrame().
    virtual MERROR_ENUM do_validatePerFrame();

protected:  ////    All
    MBOOL   prepareHw_PerFrame_All();
    MBOOL   applyToHw_PerFrame_All();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////    Module.
    MVOID   prepareHw_PerFrame_Default();

    MVOID   prepareHw_PerFrame_by_OpMode(
                MVOID (*pf_prepareIspHWBuf_enableXXX)(MBOOL const fgEnable), 
                MBOOL const fgIsForceSet_XXX  = MFALSE, 
                MBOOL const fgForceEnable_XXX = MFALSE
            );
    MBOOL   prepareHw_PerFrame_ColorClip();
    MBOOL   prepareHw_PerFrame_GainCtrl();
    MBOOL   prepareHw_PerFrame_RGB2YCC_YOfst();
    MBOOL   prepareHw_PerFrame_Shading();
    MBOOL   prepareHw_PerFrame_OB();
    MBOOL   prepareHw_PerFrame_DM();
    MBOOL   prepareHw_PerFrame_GAMMA();
    MBOOL   prepareHw_PerFrame_NR1();
    MBOOL   prepareHw_PerFrame_NR2();
    MBOOL   prepareHw_PerFrame_EE();
    MBOOL   prepareHw_PerFrame_YCCGO();
    MBOOL   prepareHw_PerFrame_CCM();
    MBOOL   prepareHw_PerFrame_PCA();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  ISP End-user-define Tuning
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////    
    /*
        Will change param members.
    */
    MBOOL   prepareHw_PerFrame_IspUserIndex();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  Color Effect
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////
    virtual MERROR_ENUM setEffect(EIndex_Effect_T const eEffect);

protected:  ////    
    /*
        Won't change param members.
        Prepare settings to the ispmgr's buffer directly.
    */
    MBOOL   prepareHw_PerFrame_ColorEffect();

protected:  ////

    template <EIndex_Effect_T eEffect> MVOID prepareEffect();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    
    RAWIspCamInfo       m_IspCamInfo;
    IspTuningCustom*    m_pIspTuningCustom;
    IspSysramDrvMgr     m_SysramMgr;

//..............................................................................
//  ISP Tuning Parameters.
//..............................................................................
protected:  ////    ISP Tuning Parameters.

    //  Reference to a given buffer.
    NVRAM_CAMERA_ISP_PARAM_STRUCT&  m_rIspParam;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////    ISP Common Parameters.

    //  Reference to m_rIspParam.ISPComm
    ISP_NVRAM_COMMON_STRUCT&        m_rIspComm;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////    ISP Register Parameters.

    //  Reference to m_rIspParam.ISPRegs & m_rIspParam.ISPRegs.Idx
    class MyIspNvramRegMgr : public IspNvramRegMgr
    {
    public:
        MyIspNvramRegMgr(ISP_NVRAM_REGISTER_STRUCT*const pIspNvramRegs)
            : IspNvramRegMgr(pIspNvramRegs)
        {}

        MyIspNvramRegMgr& operator=(IndexMgr const& rIdxmgr)
        {
            setIdx_DM   (rIdxmgr.getIdx_DM());
            setIdx_DP   (rIdxmgr.getIdx_DP());
            setIdx_NR1  (rIdxmgr.getIdx_NR1());
            setIdx_NR2  (rIdxmgr.getIdx_NR2());
            setIdx_EE   (rIdxmgr.getIdx_EE());
            setIdx_Saturation(rIdxmgr.getIdx_Saturation());
            setIdx_Contrast(rIdxmgr.getIdx_Contrast());
            setIdx_Hue  (rIdxmgr.getIdx_Hue());
            setIdx_Gamma(rIdxmgr.getIdx_Gamma());
            return  (*this);
        }
    };
    MyIspNvramRegMgr                m_IspRegMgr;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////    ISP PCA Parameters.

    //  PCA Manager.
    IPcaMgr*                          m_pIPcaMgr;

//..............................................................................
//  ISP Defect Parameters.
//..............................................................................
protected:  ////    Defect Parameters.

    //  Reference to a given buffer.
    ISP_DEFECT_STRUCT&              m_rDefectParam;

//..............................................................................
//  ISP Shading Parameters.
//..............................................................................
protected:  ////    ISP Shading Manager.

    //  LSC Manager.
   LscMgr                          m_LscMgr;

    static int DoLsc1to3(MUINT32 u4SensorId, NVRAM_CAMERA_ISP_PARAM_STRUCT* pNvramIsp, NVRAM_CAMERA_SHADING_STRUCT* pNvramShading);
    static void LscTblDump(MUINT32 u4SensorId, UINT32* pTblPv, UINT32* pTblCap, const char* filename);

};


};  //  namespace NSIspTuning
#endif // _PARAM_CTRL_RAW_H_

