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
#ifndef _DISPLAY_ISP_TUNING_IF_H_
#define _DISPLAY_ISP_TUNING_IF_H_

#include "display_isp_tuning_if_base.h"

#define LOGD(fmt, arg...)    XLOGD(fmt, ##arg)
namespace NSDisplayIspTuning
{

/*******************************************************************************
*
*******************************************************************************/
class DisplayIspTuningIF : public DisplayIspTuningIFBase
{
    enum
    {
        PCA_LUT_SIZE = sizeof(DISPLAY_ISP_PCA_BIN_T)*PCA_BIN_NUM
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Ctor/Dtor.
    DisplayIspTuningIF();
    virtual ~DisplayIspTuningIF();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static DisplayIspTuningIFBase* getInstance();
    virtual void    destroyInstance();
    virtual MINT32  init();
    virtual MINT32  deinit();
    virtual const PRZ_T& getPRZParam();
    virtual MINT32 loadISPParam();
    virtual MINT32 unloadISPParam();
    virtual MBOOL checkISPParamEffectiveness();
    virtual MINT32 setISPParamIndex(MUINT32 u4PcaSkinLutIdx, MUINT32 u4PcaGrassLutIdx, MUINT32 u4PcaSkyLutIdx, MUINT32 u4YCCGOIdx, MUINT32 u4PRZIdx);
    virtual MINT32 getISPParamIndex(MUINT32& u4PcaSkinLutIdx, MUINT32& u4PcaGrassLutIdx, MUINT32& u4PcaSkyLutIdx, MUINT32& u4YCCGOIdx, MUINT32& u4PRZIdx);

    MINT32 allocSysram(NSIspSysram::EUser_T const eUsr, MUINT32 const u4BytesToAlloc, MVOID*& rPA, MVOID*& rVA);
    MINT32 freeSysram(NSIspSysram::EUser_T const eUsr);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PCA LUT
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    inline
    MBOOL
    isPCAEnabled() const
    {
        return  (0 != m_rParam.rPcaCfg[m_rParam.rIndex.PcaCfg].ctrl.bits.EN);
    }

    inline
    MVOID
    disablePCA() const
    {
        if (m_pIspReg)
            ISP_BITS(m_pIspReg, CAM_PCA_CON, EN) = 0;
    }

    inline
    MUINT32
    getPCALutSize() const
    {
        return PCA_LUT_SIZE;
    }

    inline
    MVOID
    mergePCALut()
    {
        MINT32 i;
        // merge skin color
//        if (m_bIsPcaSkinLutIdxChanged) {
            for (i = PCA_SKIN_BIN_START; i < PCA_SKIN_BIN_START + PCA_SKIN_BIN_NUM; i++) {
                m_rPcaLut.lut[i] = m_rParam.rPcaSkinLut[m_rParam.rIndex.PcaSkinLut].lut[i - PCA_SKIN_BIN_START];
//                LOGD("m_rPcaLut.lut[%d].hue_shit = %d\n", i, m_rPcaLut.lut[i].hue_shift);
            }

            m_bIsPcaSkinLutIdxChanged = MFALSE;
//        }

        // merge grass color
//        if (m_bIsPcaGrassLutIdxChanged) {
            for (i = PCA_GRASS_BIN_START; i < PCA_GRASS_BIN_START + PCA_GRASS_BIN_NUM; i++) {
                m_rPcaLut.lut[i] = m_rParam.rPcaGrassLut[m_rParam.rIndex.PcaGrassLut].lut[i - PCA_GRASS_BIN_START];
//                LOGD("m_rPcaLut.lut[%d].hue_shit = %d\n", i, m_rPcaLut.lut[i].hue_shift);
            }

            m_bIsPcaGrassLutIdxChanged = MFALSE;
//        }

        // merge sky color
//        if (m_bIsPcaSkyLutIdxChanged) {
            for (i = PCA_SKY_BIN_START; i < PCA_BIN_NUM; i++) {
                m_rPcaLut.lut[i] = m_rParam.rPcaSkyLut[m_rParam.rIndex.PcaSkyLut].lut[i - PCA_SKY_BIN_START];
//                LOGD("m_rPcaLut.lut[%d].hue_shit = %d\n", i, m_rPcaLut.lut[i].hue_shift);
            }
            for (i = 0; i < PCA_SKY_BIN_NUM - (PCA_BIN_NUM - PCA_SKY_BIN_START); i++) {
                m_rPcaLut.lut[i] = m_rParam.rPcaSkyLut[m_rParam.rIndex.PcaSkyLut].lut[i + (PCA_BIN_NUM - PCA_SKY_BIN_START)];
//                LOGD("m_rPcaLut.lut[%d].hue_shit = %d\n", i, m_rPcaLut.lut[i].hue_shift);
            }

            m_bIsPcaSkyLutIdxChanged = MFALSE;
//        }
    }

    inline
    MVOID
    loadPCALutToSysram()
    {   //  VA <- PCA LUT
        ::memcpy(m_PcaVA, &m_rPcaLut.lut[0], PCA_LUT_SIZE);
    }

    inline
    MVOID
    loadSysramLutToISP()
    {
        ISP_REG(m_pIspReg, CAM_PCA_TBA) = reinterpret_cast<MUINT32>(m_PcaPA);
    }

    inline
    MBOOL
    isPCALutLoadBusy() const
    {
        return  (0 != ISP_BITS(m_pIspReg, CAM_PCA_CON, PCA_BUSY));
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISP Parameter
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    inline
    MBOOL
    isNR2Enabled() const
    {
        return  ((0 != m_rParam.rNR2[m_rParam.rIndex.NR2].ctrl.bits.ENY) || (0 != m_rParam.rNR2[m_rParam.rIndex.NR2].ctrl.bits.ENC));
    }

    inline
    MBOOL
    isYCCGOEnabled() const
    {
        return  (0 != m_rParam.rYCCGO[m_rParam.rIndex.YCCGO].ctrl.val);
    }

    inline
    MVOID
    loadEEParam()
    {
        ISP_REG(m_pIspReg, CAM_EE_CTRL) = 0x00000000; // disable EE
    }

    inline
    MVOID
    loadNR2Param()
    {
        ISP_REG(m_pIspReg, CAM_NR2_CON) = m_rParam.rNR2[m_rParam.rIndex.NR2].ctrl.val;
        ISP_REG(m_pIspReg, CAM_NR2_CFG_C1) = m_rParam.rNR2[m_rParam.rIndex.NR2].cfg1.val;
        ISP_REG(m_pIspReg, CAM_NR2_CFG2) = m_rParam.rNR2[m_rParam.rIndex.NR2].cfg2.val;
        ISP_REG(m_pIspReg, CAM_NR2_CFG3) = m_rParam.rNR2[m_rParam.rIndex.NR2].cfg3.val;
        ISP_REG(m_pIspReg, CAM_NR2_CFG4) = m_rParam.rNR2[m_rParam.rIndex.NR2].cfg4.val;
        ISP_REG(m_pIspReg, CAM_NR2_CFG_C2) = m_rParam.rNR2[m_rParam.rIndex.NR2].luma.val;
        ISP_REG(m_pIspReg, CAM_NR2_CFG_L1) = m_rParam.rNR2[m_rParam.rIndex.NR2].lce_gain.val;
        ISP_REG(m_pIspReg, CAM_NR2_CFG_L2) = m_rParam.rNR2[m_rParam.rIndex.NR2].lce_gain_div.val;
        ISP_REG(m_pIspReg, CAM_NR2_CFG_N1) = m_rParam.rNR2[m_rParam.rIndex.NR2].mode1_cfg1.val;
        ISP_REG(m_pIspReg, CAM_NR2_CFG_N2) = m_rParam.rNR2[m_rParam.rIndex.NR2].mode1_cfg2.val;
        ISP_REG(m_pIspReg, CAM_NR2_CFG_N3) = m_rParam.rNR2[m_rParam.rIndex.NR2].mode1_cfg3.val;
    }

    inline
    MVOID
    loadPcaParam()
    {
        ISP_REG(m_pIspReg, CAM_PCA_CON) = m_rParam.rPcaCfg[m_rParam.rIndex.PcaCfg].ctrl.val;
        ISP_REG(m_pIspReg, CAM_PCA_GMC) = m_rParam.rPcaCfg[m_rParam.rIndex.PcaCfg].gmc.val;
    }

    inline
    MVOID
    loadYCCGOParam()
    {
        ISP_REG(m_pIspReg, CAM_YCCGO_CON) = m_rParam.rYCCGO[m_rParam.rIndex.YCCGO].ctrl.val;
        ISP_REG(m_pIspReg, CAM_YCCGO_CFG1) = m_rParam.rYCCGO[m_rParam.rIndex.YCCGO].cfg1.val;
        ISP_REG(m_pIspReg, CAM_YCCGO_CFG2) = m_rParam.rYCCGO[m_rParam.rIndex.YCCGO].cfg2.val;
        ISP_REG(m_pIspReg, CAM_YCCGO_CFG3) = m_rParam.rYCCGO[m_rParam.rIndex.YCCGO].cfg3.val;
        ISP_REG(m_pIspReg, CAM_YCCGO_CFG4) = m_rParam.rYCCGO[m_rParam.rIndex.YCCGO].cfg4.val;
        ISP_REG(m_pIspReg, CAM_YCCGO_CFG5) = m_rParam.rYCCGO[m_rParam.rIndex.YCCGO].cfg5.val;
        ISP_REG(m_pIspReg, CAM_YCCGO_CFG6) = m_rParam.rYCCGO[m_rParam.rIndex.YCCGO].cfg6.val;
        ISP_REG(m_pIspReg, CAM_YCCGO_CFG7) = m_rParam.rYCCGO[m_rParam.rIndex.YCCGO].cfg7.val;
        ISP_REG(m_pIspReg, CAM_YCCGO_CFG8) = m_rParam.rYCCGO[m_rParam.rIndex.YCCGO].cfg8.val;
        ISP_REG(m_pIspReg, CAM_YCCGO_CFG9) = m_rParam.rYCCGO[m_rParam.rIndex.YCCGO].cfg9.val;
    }

private:
    DISPLAY_ISP_T              m_rParam;
    DISPLAY_ISP_PCA_LUT_T      m_rPcaLut;
    IspDrv*                    m_pIspDrv;
    isp_reg_t*                 m_pIspReg;
    NSIspSysram::IspSysramDrv* m_pSysramDrv;
    MVOID*                     m_PcaPA;
    MVOID*                     m_PcaVA;
    MBOOL                      m_bIsPcaSkinLutIdxChanged;
    MBOOL                      m_bIsPcaGrassLutIdxChanged;
    MBOOL                      m_bIsPcaSkyLutIdxChanged;
    volatile MINT32            m_Users;
    mutable android::Mutex     m_Lock;
};

};  //  namespace NSDisplayIspTuning
#endif // _DISPLAY_ISP_TUNING_IF_H_

