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
#define LOG_TAG "NSIspTuning::ParamctrlRAW_per_frame"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#include "debug.h"
#include "paramctrl_raw.h"
//
using namespace android;
using namespace NSIspTuning;


MBOOL
ParamctrlRAW::
applyToHw_PerFrame_All()
{
    return  MTRUE
        &&  ISP_MGR_CCLIP_T  ::getInstance().apply()
        &&  ISP_MGR_GAIN_CTRL_T ::getInstance().apply()
        &&  ISP_MGR_RGB2YCC_YOFST_T ::getInstance().apply()
        &&  ISP_MGR_SHADING_T::getInstance().apply()
        &&  ISP_MGR_OB_T     ::getInstance().apply()
        &&  ISP_MGR_DM_T     ::getInstance().apply()
        &&  ISP_MGR_GAMMA_T  ::getInstance().apply()
        &&  ISP_MGR_NR1_T    ::getInstance().apply()
        &&  ISP_MGR_NR2_T    ::getInstance().apply()
        &&  ISP_MGR_EDGE_T   ::getInstance().apply()
        &&  ISP_MGR_EE_T     ::getInstance().apply()
        &&  ISP_MGR_YCCGO_T  ::getInstance().apply()
        &&  ISP_MGR_CCM_T    ::getInstance().apply()
        &&  ISP_MGR_PCA_T    ::getInstance().apply()
        ;
}


MBOOL
ParamctrlRAW::
prepareHw_PerFrame_All()
{
    MBOOL fgRet = MTRUE;

    //  (1) reset: read register setting to ispmgr
    fgRet = MTRUE
        //&&  ISP_MGR_SHADING_T::getInstance().reset()
        &&  ISP_MGR_CCLIP_T  ::getInstance().reset()
        &&  ISP_MGR_GAIN_CTRL_T ::getInstance().reset()
        &&  ISP_MGR_RGB2YCC_YOFST_T ::getInstance().reset()
        &&  ISP_MGR_OB_T     ::getInstance().reset()
        &&  ISP_MGR_DM_T     ::getInstance().reset()
        &&  ISP_MGR_GAMMA_T  ::getInstance().reset()
        &&  ISP_MGR_NR1_T    ::getInstance().reset()
        &&  ISP_MGR_NR2_T    ::getInstance().reset()
        &&  ISP_MGR_EDGE_T   ::getInstance().reset()
        &&  ISP_MGR_EE_T     ::getInstance().reset()
        &&  ISP_MGR_YCCGO_T  ::getInstance().reset()
        &&  ISP_MGR_CCM_T    ::getInstance().reset()
        &&  ISP_MGR_PCA_T    ::getInstance().reset()
            ;
    if  ( ! fgRet )
    {
        goto lbExit;
    }

    //  (2) default
    prepareHw_PerFrame_Default();

    //  (3) prepare something and fill buffers.
    fgRet = MTRUE
        &&  prepareHw_PerFrame_ColorClip()
        &&  prepareHw_PerFrame_GainCtrl()
        &&  prepareHw_PerFrame_RGB2YCC_YOfst()
        &&  prepareHw_PerFrame_Shading()
        &&  prepareHw_PerFrame_OB()
        &&  prepareHw_PerFrame_DM()
        &&  prepareHw_PerFrame_GAMMA()
        &&  prepareHw_PerFrame_NR1()
        &&  prepareHw_PerFrame_NR2()
        &&  prepareHw_PerFrame_EE()
        &&  prepareHw_PerFrame_YCCGO()
        &&  prepareHw_PerFrame_CCM()
        &&  prepareHw_PerFrame_PCA()
            ;
    if  ( ! fgRet )
    {
        goto lbExit;
    }

lbExit:
    return  fgRet;
}


MVOID
ParamctrlRAW::
prepareHw_PerFrame_Default()
{
    //  (1) Edge Gamma
    ISP_NVRAM_EDGE_GAMMA_T  IspEdgeGamma;
    m_pIspTuningCustom->prepare_edge_gamma(IspEdgeGamma);
    putIspHWBuf( IspEdgeGamma );

    //  (2) Color Effect
    ISP_EFFECT_T IspEffect;

    setDefault_ISP_XXX(IspEffect.yccgo.ctrl);
    setDefault_ISP_XXX(IspEffect.yccgo.cfg1);
    setDefault_ISP_XXX(IspEffect.yccgo.cfg2);
    setDefault_ISP_XXX(IspEffect.yccgo.cfg3);
    setDefault_ISP_XXX(IspEffect.yccgo.cfg4);
    setDefault_ISP_XXX(IspEffect.yccgo.cfg5);
    setDefault_ISP_XXX(IspEffect.yccgo.cfg6);
    setDefault_ISP_XXX(IspEffect.yccgo.cfg7);
    setDefault_ISP_XXX(IspEffect.yccgo.cfg8);
    setDefault_ISP_XXX(IspEffect.yccgo.cfg9);

    setDefault_ISP_XXX(IspEffect.edge.ed_ctrl);
    setDefault_ISP_XXX(IspEffect.edge.ed_inter1);
    setDefault_ISP_XXX(IspEffect.edge.ed_inter2);
    setDefault_ISP_XXX(IspEffect.edge.edgcore);
    setDefault_ISP_XXX(IspEffect.edge.edggain1);
    setDefault_ISP_XXX(IspEffect.edge.edggain2);
    setDefault_ISP_XXX(IspEffect.edge.edgthre);
    setDefault_ISP_XXX(IspEffect.edge.edgvcon);
    setDefault_ISP_XXX(IspEffect.edge.cpscon2);
    setDefault_ISP_XXX(IspEffect.edge.ee_ctrl);
    setDefault_ISP_XXX(IspEffect.edge.ed_ctrl1);
    setDefault_ISP_XXX(IspEffect.edge.ed_ctrl2);
    setDefault_ISP_XXX(IspEffect.edge.ed_ctrl3);
    setDefault_ISP_XXX(IspEffect.edge.ed_ctrl4);
    setDefault_ISP_XXX(IspEffect.edge.ed_ctrl5);

    setDefault_ISP_XXX(IspEffect.ccm.ccm1);
    setDefault_ISP_XXX(IspEffect.ccm.ccm2);
    setDefault_ISP_XXX(IspEffect.ccm.ccm3);
    setDefault_ISP_XXX(IspEffect.ccm.ccm4);
    setDefault_ISP_XXX(IspEffect.ccm.ccm5);

    putIspHWBuf(IspEffect.yccgo);
    putIspHWBuf(IspEffect.edge);
    putIspHWBuf(IspEffect.ccm);
}


MVOID
ParamctrlRAW::
prepareHw_PerFrame_by_OpMode(
    MVOID (*pf_prepareIspHWBuf_enableXXX)(MBOOL const fgEnable), 
    MBOOL const fgIsForceSet_XXX /*= MFALSE*/, 
    MBOOL const fgForceEnable_XXX /*= MFALSE*/
)
{
    //  Enable/Disable.
    switch (getOperMode())
    {
    case EOperMode_PureRaw:
        (*pf_prepareIspHWBuf_enableXXX)(MFALSE);
        break;
    case EOperMode_Meta:
        if  ( fgIsForceSet_XXX )
        {
            (*pf_prepareIspHWBuf_enableXXX)(fgForceEnable_XXX);
        }
        break;
    case EOperMode_Normal:
    default:
        break;
    }
}

MBOOL
ParamctrlRAW::
prepareHw_PerFrame_ColorClip()
{
    ISP_NVRAM_CCLIP_T color_clip;
    ISP_MGR_CCLIP_T::getInstance().get(color_clip);

    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_ColorClip(m_IspCamInfo, color_clip);
    }

    prepareIspHWBuf( color_clip );

    return  MTRUE;
}

MBOOL
ParamctrlRAW::
prepareHw_PerFrame_GainCtrl()
{
/*
    ISP_NVRAM_GAIN_CTRL_T gainCtrl;
    ISP_MGR_GAIN_CTRL_T::getInstance().get(gainCtrl);

    switch (m_IspCamInfo.eCamMode)
    {
    case ECamMode_HDR_Cap_Pass1_MF1:    //  HDR Pass1: Multi Frame Stage1
        // Modify ISP GAIN_COMP and PGAIN here
        gainCtrl.cam_ctrl1.bits.GAIN_COMP = 0x80;
        gainCtrl.cam_ctrl1.bits.PGAIN_INT = 3;
        gainCtrl.cam_ctrl1.bits.PGAIN_FRAC = 0x7F;
        break;

    default:
        // Resume ISP GAIN_COMP and PGAIN to 1X
        gainCtrl.cam_ctrl1.bits.GAIN_COMP = 0x80;
        gainCtrl.cam_ctrl1.bits.PGAIN_INT = 1;
        gainCtrl.cam_ctrl1.bits.PGAIN_FRAC = 0;
        break;
    }

    prepareIspHWBuf( gainCtrl );
*/
    return  MTRUE;
}

MBOOL
ParamctrlRAW::
prepareHw_PerFrame_RGB2YCC_YOfst()
{
    ISP_NVRAM_RGB2YCC_YOFST_T rgb2ycc_yofst;
    ISP_MGR_RGB2YCC_YOFST_T::getInstance().get(rgb2ycc_yofst);

    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_RGB2YCC_YOfst(m_IspCamInfo, rgb2ycc_yofst);
    }

    prepareIspHWBuf( rgb2ycc_yofst );

    return  MTRUE;
}

MBOOL
ParamctrlRAW::
prepareHw_PerFrame_OB()
{
    //  Load it to ISP manager buffer.
    ISP_NVRAM_OB_T ob;

    switch (m_IspCamInfo.eCamMode)
    {
    //  NORMAL
    case ECamMode_Online_Preview:
    case ECamMode_Video:
    case ECamMode_Online_Capture:
    case ECamMode_Online_Capture_ZSD:
#ifdef MTK_ZSD_AF_ENHANCE
    case ECamMode_Online_Preview_ZSD:
#endif
    case ECamMode_Offline_Capture_Pass1:
    //  HDR
    case ECamMode_HDR_Cap_Pass1_SF:     //  HDR Pass1: Single Frame
    case ECamMode_HDR_Cap_Pass1_MF1:    //  HDR Pass1: Multi Frame Stage1
        ob = m_IspRegMgr.getOB();

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_OB(m_IspCamInfo, ob);
        }
        break;

    default:
        //  Neen't apply OB since it has been applied in pass1.
        ::memset(ob.set, 0, sizeof(ob));
        break;
    }
    prepareIspHWBuf( ob );

    return  MTRUE;
}


MBOOL
ParamctrlRAW::
prepareHw_PerFrame_DM()
{
    //  (1) Save the current DM to local.
    ISP_NVRAM_DEMOSAIC_T dm = m_IspRegMgr.getDM();

    //  (2) Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_DM(m_IspCamInfo, dm);
    }

    //  (3) Load it to ISP manager buffer.
    prepareIspHWBuf(dm);
    return  MTRUE;
}


MBOOL
ParamctrlRAW::
prepareHw_PerFrame_GAMMA()
{
    //  (1) Load it to ISP manager buffer.
    prepareIspHWBuf( m_IspRegMgr.getGamma() );

    //  (2) Enable/Disable Gamma.
    switch (getOperMode())
    {
    case EOperMode_Normal:
        prepareIspHWBuf_enableGamma(MTRUE);
        break;
    case EOperMode_Meta:
        prepareIspHWBuf_enableGamma(getEnable_Meta_Gamma());
        break;
    case EOperMode_PureRaw:
    default:
        break;
    }

    return  MTRUE;
}


MBOOL
ParamctrlRAW::
prepareHw_PerFrame_NR1()
{
    //  (1) Save the current NR1 to local.
    ISP_NVRAM_NR1_T nr1 = m_IspRegMgr.getNR1();
    ISP_NVRAM_DP_T dp = m_IspRegMgr.getDP();

    //  (2) Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_NR1(m_IspCamInfo, nr1);
        m_pIspTuningCustom->refine_DP(m_IspCamInfo, dp);
    }

    //  (3.1) Load NR1 to ISP manager buffer.
    prepareIspHWBuf( nr1 );
    
    //  (3.2) Load DP to ISP manager buffer.
    prepareIspHWBuf( dp );

    //  (4.1) Enable/Disable NR1 CT.
    prepareHw_PerFrame_by_OpMode(
        prepareIspHWBuf_enableNR1_CT
    );
    //  (4.2) Enable/Disable NR1 NR.
    prepareHw_PerFrame_by_OpMode(
        prepareIspHWBuf_enableNR1_NR
    );
    //  (4.3) Enable/Disable NR1 DP.
    prepareHw_PerFrame_by_OpMode(
        prepareIspHWBuf_enableNR1_DP
    );

    return  MTRUE;
}


MBOOL
ParamctrlRAW::
prepareHw_PerFrame_NR2()
{
     if ((m_IspCamInfo.eCamMode == ECamMode_YUV2JPG_Scalado) ||
         (m_IspCamInfo.eCamMode == ECamMode_FB_PostProcess_PCA_Only))
    {
        ISP_MGR_NR2_T::getInstance().setDisable();
    }
    else
    {
        //  (1) Save the current NR2 to local.
        ISP_NVRAM_NR2_T nr2 = m_IspRegMgr.getNR2();

        //  (2) Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_NR2(m_IspCamInfo, nr2);
        }

        //  (3) Load it to ISP manager buffer.
        prepareIspHWBuf(nr2);
    }

    return  MTRUE;
}


MBOOL
ParamctrlRAW::
prepareHw_PerFrame_EE()
{
    if ((m_IspCamInfo.eCamMode == ECamMode_YUV2JPG_Scalado)  ||
        (m_IspCamInfo.eCamMode == ECamMode_YUV2JPG_ZSD) ||
        (m_IspCamInfo.eCamMode == ECamMode_FB_PostProcess_NR2_Only) ||
        (m_IspCamInfo.eCamMode == ECamMode_FB_PostProcess_PCA_Only))
    {
        ISP_MGR_EDGE_T::getInstance().setDisable();
        ISP_MGR_EE_T::getInstance().setDisable();
    }
    else
    {
        //  (1) Save the current EE and GammaECO to local.
        ISP_NVRAM_EE_T ee = m_IspRegMgr.getEE();
        ISP_NVRAM_GAMMA_ECO_T gammaECO;
        gammaECO.gamma_eco_en.val = ISP_NVRAM_CPSCON2_T::DEFAULT;

        //  (2) Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_EE(m_IspCamInfo, ee);
            m_pIspTuningCustom->refine_GammaECO(m_IspCamInfo, gammaECO);
        }

        //  (3) Load it to ISP manager buffer.
        prepareIspHWBuf(ee);
        prepareIspHWBuf(gammaECO);
    }

    return  MTRUE;
}


MBOOL
ParamctrlRAW::
prepareHw_PerFrame_YCCGO()
{
    if ((m_IspCamInfo.eCamMode == ECamMode_YUV2JPG_Scalado)  ||
        (m_IspCamInfo.eCamMode == ECamMode_YUV2JPG_ZSD) ||
        (m_IspCamInfo.eCamMode == ECamMode_FB_PostProcess_NR2_Only) ||
        (m_IspCamInfo.eCamMode == ECamMode_FB_PostProcess_PCA_Only))
    {
        ISP_MGR_YCCGO_T::getInstance().setDisable();
    }
    else
    {
        //  (1) Save current saturation, contrast, and hue to local
        ISP_NVRAM_SATURATION_T saturation = m_IspRegMgr.getSaturation();
        ISP_NVRAM_CONTRAST_T contrast = m_IspRegMgr.getContrast();
        ISP_NVRAM_HUE_T hue = m_IspRegMgr.getHue();

        // (2) Invoke callback for customers to modify
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_Saturation(m_IspCamInfo, saturation);
            m_pIspTuningCustom->refine_Contrast(m_IspCamInfo, contrast);
            m_pIspTuningCustom->refine_Hue(m_IspCamInfo, hue);
        }

        // (3) Load it to ISP manager buffer
        prepareIspHWBuf(saturation);
        prepareIspHWBuf(contrast);
        prepareIspHWBuf(hue);
    }

    return  MTRUE;  
}


MBOOL
ParamctrlRAW::
prepareHw_PerFrame_CCM()
{
    //  (1) CCM index depends on Color Temperature.
    if  ( isDynamicCCM() )
    {   //  Dynamic Tuning: Enable

        MUINT8 const u8CcmIdx = m_IspCamInfo.eIdx_CCM_CCT;

        if  ( ! m_IspRegMgr.setIdx_CCM(u8CcmIdx) )
        {
            MY_LOG(
                "[prepareHw_PerFrame_CCM] "
                "CCM idx(%d) out of range from map_CCM_CCT_index_to_nvram_index"
                , u8CcmIdx
            );
            return  MFALSE;
        }
    }

    //  (2) Save the current CCM to local.
    ISP_NVRAM_CCM_T ccm = m_IspRegMgr.getCCM();

    //  (3) Invoke callback for customers to modify.
    if  ( isDynamicCCM() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_CCM(m_IspCamInfo, ccm);
    }

    //  (4) Load it to ISP manager buffer.
    prepareIspHWBuf(ccm);

    return  MTRUE;
}


MBOOL
ParamctrlRAW::
prepareHw_PerFrame_PCA()
{
    MBOOL fgRet = MTRUE;
    MBOOL fgIsToLoadLut = MFALSE;   //  MTRUE indicates to load LUT.

    if ((m_IspCamInfo.eCamMode == ECamMode_YUV2JPG_Scalado)  ||
        (m_IspCamInfo.eCamMode == ECamMode_YUV2JPG_ZSD) ||
        (m_IspCamInfo.eCamMode == ECamMode_FB_PostProcess_NR2_Only))
    {
        ISP_MGR_PCA_T::getInstance().setDisable();
        goto lbExit;
    }

    //  (1) Check to see whether PCA is enabled?
    if  ( ! m_pIPcaMgr->isEnable() )
    {
        m_pIPcaMgr->loadConfig();
        goto lbExit;
    }

    //  (2) Now we must decide the PCA index.
    if  ( isDynamicCCM() )
    {
        //  Use CCM index as the PCA index if the dynamic tuning is enabled;
        //  Otherwise do nothing without modifying.
        //  Note: CCM index depends on Color Temperature.
        m_pIPcaMgr->setIdx(m_IspRegMgr.getIdx_CCM());
#if 0
        MY_LOG("[prepareHw_PerFrame_PCA] (pca idx)=(%d)", m_pIPcaMgr->getIdx());
#endif
    }

    //  (3) Check to see if it is needed to load LUT.
    switch  (getOperMode())
    {
    case EOperMode_Normal:
    case EOperMode_PureRaw:
        fgIsToLoadLut = m_pIPcaMgr->isChanged();   // Load if changed.
        break;
    default:
        fgIsToLoadLut = MTRUE;                  // Force to load.
        break;
    }

    //  (4) Load the LUT to ISP internal buffer if needed.
    if  ( fgIsToLoadLut )
    {
        MVOID* pPhyAddr = NULL;
        MVOID* pVirAddr = NULL;
        MERROR_ENUM err = m_SysramMgr.autoAlloc(
            NSIspSysram::EUsr_PCA, m_pIPcaMgr->getLutSize(), pPhyAddr, pVirAddr
        );
        if  ( MERR_OK != err )
        {
            //  Error handling: 
            //  Currently, we don't return failure. Instead, just skip this frame.
            MY_ERR("[ParamctrlRAW::prepareHw_PerFrame_PCA] fail to alloc sysram.");
            goto lbExit;
        }
        m_pIPcaMgr->savePhyTBA(pPhyAddr);
        m_pIPcaMgr->saveVirTBA(pVirAddr);
        fgRet = m_pIPcaMgr->loadLut();
        if  ( ! fgRet )
        {
            goto lbExit;
        }
    }

    //  (5) Load config (from nvram) to ISP manager buffer.
    m_pIPcaMgr->loadConfig();

#if 0
    //  (6) Free resources if loading done.
    //
    //  Currently, we don't want to repeatedly alloc/free the resources 
    //  in preview mode in order to prevent fragmenting.
    //
    if  ( ! m_pIPcaMgr->isLoading() && ! fgIsToLoadLut )
    {
        m_pIPcaMgr->savePhyTBA(NULL);
        m_pIPcaMgr->saveVirTBA(NULL);
        m_SysramMgr.free(NSIspSysram::EUsr_PCA);
    }
#endif

lbExit:
    return  fgRet;
}


MBOOL
ParamctrlRAW::
prepareHw_PerFrame_Shading()
{
/*
if  META mode, 
modify m_rIspRegs.Shading's enable it by refering to
 a flag of shading enable/disable.
*/
    MINT32 u4ShadingSettingChangeCount;
    MBOOL fgRet;

    if (getOperMode() == EOperMode_Meta)
    {
       // Shading Parameter Control when Meta mode
       if ( m_fgShadingNVRAMdataChange != 0)
       {
           if (m_LscMgr.isEnable()!=0)
           {
        	    m_LscMgr.loadLut(); //load table   
        	    fgRet = m_LscMgr.SetTBAToISP(); //preview table address will modify base on m_u4Idx
        	    if (fgRet != MTRUE) // update table address
        	    {
        		      MY_LOG(
        		                "[Raw_per_frame] "
        		                "Set table address fail %d \n"
        		                ,fgRet
        		            );	    	
        	    }
           }
	    m_LscMgr.loadConfig(); // set NVRAM data to ispmgr_mt6575
            
           m_fgShadingNVRAMdataChange = 0;// reset flag
           
            LOGD(
                      "[Raw_per_frame] "
                      "NVRAM data change"
                      " (CamMode,SensorMode, Operation_Mode) = (%d,%d,%d)\n"
                      "Lsc_mode, Lsc_idx (%d,%d) \n"
                      , m_IspCamInfo.eCamMode //   ECamMode_Video          = 0,     ECamMode_Online_Preview = ECamMode_Video,     ECamMode_Online_Capture, 
                      , getSensorMode()
                      , getOperMode()
                      , m_LscMgr.getMode()
                      , m_LscMgr.getIdx()
                      );            
        }    
    }
    else //normal operation
    {
        //  (1) Set shading table reference index base on CCT information
#if 0
        if (m_IspCamInfo.eIdx_Shading_CCT < eIDX_Shading_CCT_D65)
        {
            m_LscMgr.setIdx(1);  // CWF and A Light both using  2nd table
                                          // capture must modify index base on  color temperature from AWB
                                          // preview always load 3 color temperature table
        }
        else
        {
            m_LscMgr.setIdx(2);  // D65 using 3rd table
    	                                 // capture must modify index base on  color temperature from AWB
    	                                 // preview always load 3 color temperature table
        }    
#else
        m_LscMgr.setIdx(m_IspCamInfo.eIdx_Shading_CCT);
#endif
    
         //  (2) If index chage, modify shading table address
        u4ShadingSettingChangeCount = m_LscMgr.getSettingChangeCount();
        if (u4ShadingSettingChangeCount != 0)
        {
             MY_LOG(
                         "[Raw_per_frame] "
     		                "Shading table ref idx change to %d \n"
     		                ,m_LscMgr.getIdx()
     		            );	    	
     	    fgRet = m_LscMgr.SetTBAToISP(); //preview table address will modify base on m_u4Idx
     	    if (fgRet != MTRUE) // update table address
     	    {
     		      MY_LOG(
     		                "[Raw_per_frame] "
     		                "Set table address fail %d \n"
     		                ,fgRet
     		            );	    	
     	    }
     	    m_LscMgr.loadConfig(); // set NVRAM data to ispmgr_mt6575
     	    m_LscMgr.resetSettingChangeCount();
        }
    }

    return  MTRUE;
}

