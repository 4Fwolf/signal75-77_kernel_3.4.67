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
#ifndef _CCT_IMP_H_
#define _CCT_IMP_H_

#include "nvram_drv.h"
class NvramDrvBase;
class SensorDrv;
class IspDrv;
class Hal3ABase;
class IspHal;

/*******************************************************************************
*
********************************************************************************/
class CctCtrl
{
public:    
    static CctCtrl* createInstance(CAMERA_DUAL_CAMERA_SENSOR_ENUM const eSensorEnum, IspHal*const pIspHal);
    void            destroyInstance();

protected:
    CctCtrl(
        CAMERA_DUAL_CAMERA_SENSOR_ENUM const eSensorEnum, 
        MUINT32 const       u4SensorID, 
        NvramDrvBase*const  pNvramDrv, 
        SensorDrv*const     pSensorDrv, 
        IspDrv*const        pIspDrv, 
        IspHal*const        pIspHal, 
        NSNvram::BufIF<NVRAM_CAMERA_ISP_PARAM_STRUCT>*const pBufIf_ISP, 
        NSNvram::BufIF<NVRAM_CAMERA_SHADING_STRUCT>*const   pBufIf_SD, 
        NSNvram::BufIF<NVRAM_CAMERA_DEFECT_STRUCT>*const    pBufIf_DF, 
        NVRAM_CAMERA_ISP_PARAM_STRUCT*const pBuf_ISP, 
        NVRAM_CAMERA_SHADING_STRUCT*const   pBuf_SD, 
        NVRAM_CAMERA_DEFECT_STRUCT*const    pBuf_DF
    );
    ~CctCtrl();

public:
    virtual MINT32  cctFeatureCtrl_isp  (MUINT32 const a_u4Ioctl, MUINT8*const puParaIn, MUINT32 const u4ParaInLen, MUINT8*const puParaOut, MUINT32 const u4ParaOutLen, MUINT32*const pu4RealParaOutLen);
    virtual MINT32  cctFeatureCtrl_nvram(MUINT32 const a_u4Ioctl, MUINT8*const puParaIn, MUINT32 const u4ParaInLen, MUINT8*const puParaOut, MUINT32 const u4ParaOutLen, MUINT32*const pu4RealParaOutLen);

protected:
    template <MUINT32 ctl_code>
    MINT32 doCctFeatureCtrl(
        MUINT8*const puParaIn, 
        MUINT32 const u4ParaInLen, 
        MUINT8* const puParaOut, 
        MUINT32 const u4ParaOutLen, 
        MUINT32*const pu4RealParaOutLen
    );

#define IMP_CCT_CTRL(ctl_cocde) \
    template <> \
    MINT32 CctCtrl::doCctFeatureCtrl<ctl_cocde>( \
        MUINT8* const puParaIn, \
        MUINT32 const u4ParaInLen, \
        MUINT8* const puParaOut, \
        MUINT32 const u4ParaOutLen, \
        MUINT32*const pu4RealParaOutLen \
    )

private:    //// ISP feature control
    MINT32 CCTOPReadIspReg(MVOID *pCCTIspRegInfoIn, MVOID *pCCTIspRegInfoOut, MUINT32 *pRealParaOutLen);
    MINT32 CCTOPWriteIspReg(MVOID *pCCTIspRegInfoIn);
    MBOOL  updateIspRegs(MUINT32 const u4Category = 0xFFFFFFFF, MUINT32 const u4Index = 0xFFFFFFFF);

    MVOID   dumpIspReg(MUINT32 const u4Addr) const;

    MINT32  setIspOnOff(MUINT32 const u4Category, MBOOL const fgOn);
    MINT32  getIspOnOff(MUINT32 const u4Category, MBOOL& rfgOn) const;

    MVOID   setIspOnOff_OB(MBOOL const fgOn);
    MVOID   setIspOnOff_DP(MBOOL const fgOn);
    MVOID   setIspOnOff_NR1(MBOOL const fgOn);
    MVOID   setIspOnOff_NR2(MBOOL const fgOn);
    MVOID   setIspOnOff_EE(MBOOL const fgOn);
    MVOID   setIspOnOff_Saturation(MBOOL const fgOn);
    MVOID   setIspOnOff_Contrast(MBOOL const fgOn);
    MVOID   setIspOnOff_Hue(MBOOL const fgOn);

    MBOOL   getIspOnOff_OB() const;
    MBOOL   getIspOnOff_DP() const;
    MBOOL   getIspOnOff_NR1() const;
    MBOOL   getIspOnOff_NR2() const;
    MBOOL   getIspOnOff_EE() const;
    MBOOL   getIspOnOff_Saturation() const;
    MBOOL   getIspOnOff_Contrast() const;
    MBOOL   getIspOnOff_Hue() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    
    CAMERA_DUAL_CAMERA_SENSOR_ENUM const m_eSensorEnum;
    MUINT32 const   m_u4SensorID;

    NvramDrvBase*   m_pNvramDrv;
    SensorDrv*      m_pSensorDrv;
    IspDrv*         m_pIspDrv;

    IspHal*         m_pIspHal;

private:    ////    NVRAM buffer I/F.
    NSNvram::BufIF<NVRAM_CAMERA_ISP_PARAM_STRUCT>&  m_rBufIf_ISP;//isp
    NSNvram::BufIF<NVRAM_CAMERA_SHADING_STRUCT>&    m_rBufIf_SD; //shading
    NSNvram::BufIF<NVRAM_CAMERA_DEFECT_STRUCT>&     m_rBufIf_DF; //defect

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  ISP
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
private:    ////    NVRAM buffer.

    NVRAM_CAMERA_ISP_PARAM_STRUCT&  m_rBuf_ISP;

    ISP_NVRAM_COMMON_STRUCT&        m_rISPComm;
    ISP_NVRAM_REGISTER_STRUCT&      m_rISPRegs;
    ISP_NVRAM_REG_INDEX_T&          m_rISPRegsIdx;
    ISP_NVRAM_PCA_STRUCT&           m_rISPPca;

private:    ////    OB.

    MBOOL   m_fgEnabled_OB;
    MUINT32 m_u4Backup_OB;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  Shading
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
private:    ////    NVRAM buffer.

    NVRAM_CAMERA_SHADING_STRUCT&    m_rBuf_SD;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  Defect
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
private:    ////    NVRAM buffer.

    NVRAM_CAMERA_DEFECT_STRUCT&     m_rBuf_DF;

};

/*******************************************************************************
*
********************************************************************************/
class CctImp : CCTIF
{
    friend class CCTIF;

protected:
    CctImp();
    virtual ~CctImp();

public:    

    virtual void destroyInstance();
    virtual MINT32  init(IspHal*const pIspHal, MINT32 sensorType);
    virtual MINT32  uninit();

public:

    virtual MINT32 nvramCCTFeatureControl (MUINT32 a_u4Ioctl, MUINT8 *puParaIn, MUINT32 u4ParaInLen, MUINT8 *puParaOut, MUINT32 u4ParaOutLen, MUINT32 *pu4RealParaOutLen);
    virtual MINT32 sensorCCTFeatureControl(MUINT32 a_u4Ioctl, MUINT8 *puParaIn, MUINT32 u4ParaInLen, MUINT8 *puParaOut, MUINT32 u4ParaOutLen, MUINT32 *pu4RealParaOutLen);
    virtual MINT32 aaaCCTFeatureControl(MUINT32 a_u4Ioctl, MUINT8 *puParaIn, MUINT32 u4ParaInLen, MUINT8 *puParaOut, MUINT32 u4ParaOutLen, MUINT32 *pu4RealParaOutLen);
    virtual MINT32 ispCCTFeatureControl(MUINT32 a_u4Ioctl, MUINT8 *puParaIn, MUINT32 u4ParaInLen, MUINT8 *puParaOut, MUINT32 u4ParaOutLen, MUINT32 *pu4RealParaOutLen);

private:
    // Sensor feature control
    MINT32 CCTOReadSensorReg(MVOID *puParaIn, MVOID *puParaOut, MUINT32 *pu4RealParaOutLen);
    MINT32 CCTOPWriteSensorReg(MVOID *puParaIn);
    MINT32 CCTOPQuerySensor(MVOID *a_pCCTSensorInfoOut, MUINT32 *pRealParaOutLen);
    MINT32 CCTOPGetSensorRes(MVOID *pCCTSensorResOut, MUINT32 *pRealParaOutLen);
    MINT32 CCTOPGetLSCSensorRes(MVOID *pCCTSensorResOut, MUINT32 *pRealParaOutLen);
    MINT32 CCTOPGetEngSensorGroupCount(MUINT32 *pGoupCntOut, MUINT32 *pRealParaOutLen);
    MINT32 CCTOPGetEngSensorGroupPara(MUINT32 groupIdx, MVOID *pGroupInfoOut, MUINT32 *pRealParaOutLen);    
    MINT32 CCTOPGetEngSensorPara(MVOID *pSensorItemInfoIn, MVOID *pSensorItemInfoOut, MUINT32 *pRealParaOutLen);
    MINT32 CCTOPSetEngSensorPara(MVOID *pSensorItemInfoIn);
    MINT32 CCTOPGetSensorPregain(MVOID *pSensorItemInfoIn, MVOID *pSensorItemInfoOut, MUINT32 *pRealParaOutLen); 
    MINT32 CCTOPSetSensorPregain(MVOID *pSensorItemInfoIn);
    MINT32 CCTOPGetSensorInfo(MVOID *puParaIn, MVOID *puParaOut, MUINT32 *pu4RealParaOutLen);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    

    NvramDrvBase*   m_pNvramDrvObj;
    SensorDrv*      m_pSensorDrvObj;
    IspDrv*         m_pIspDrvObj;
    Hal3ABase*      m_pHal3AObj;

    CctCtrl*        m_pCctCtrl;
};

#endif // _CCT_IMP_H_

