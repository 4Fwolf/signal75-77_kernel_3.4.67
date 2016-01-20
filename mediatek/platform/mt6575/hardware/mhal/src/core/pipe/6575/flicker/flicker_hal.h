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

#ifndef _FLICKER_HAL_H_
#define _FLICKER_HAL_H_

#include "flicker_hal_base.h"
#include "FlickerDetection.h"
#include "sequential_testing.h"
using namespace android;
/*
typedef struct _flicker_threshold_t {
    MHAL_INT16 i2ThresC0;
    MHAL_INT16 i2ThresC1;
    MHAL_INT16 i2ThresA0;
    MHAL_INT16 i2ThresA1;
    MHAL_INT16 i2ThresF0;
    MHAL_INT16 i2ThresF1;
    MHAL_INT16 i2ThresF2;
} flicker_threshold_t;
*/


/*******************************************************************************
*
********************************************************************************/
class FlickerHal : public FlickerHalBase {
public:
    static FlickerHalBase* getInstance();
    virtual MVOID      destroyInstance();

private:
    FlickerHal();
    virtual ~FlickerHal();
    virtual MVOID  setFlickerThresholdParams(NSCamCustom::FlickerThresholdSetting_T  *strFlickerThres);
    virtual MINT32 sysram_alloc(NSIspSysram::EUser_T const eUsr, MUINT32 const u4BytesToAlloc, MVOID*& rPA, MVOID*& rVA);
    virtual MINT32 sysram_free(NSIspSysram::EUser_T const eUsr);
    virtual MINT32 setFlickerDrv(unsigned long flicker_mem_mode, unsigned long flicker_en);
    virtual MINT32 setFlickerWinConfig(unsigned long flicker_interval_x, unsigned long flicker_interval_y);
    virtual MINT32 setFlickerGMCConfig(unsigned long flicker_GMC_address);

public:
	virtual void setSensorInfo(CAMERA_DUAL_CAMERA_SENSOR_ENUM eDeviceId, MINT32 bZsdOn);
    virtual MINT32 init();
    virtual MINT32 uninit();
    virtual MINT32  enableFlickerDetection(MBOOL bEnableFlicker);
    virtual MINT32 analyzeFlickerFrequency(MINT32 i4LMVcnt, MINT32 *i4LMV_x, MINT32 *i4LMV_y, MINT32 *i4vAFstat);
    virtual MINT32 setWindowInfo(MUINT32 a_u4Width, MUINT32 a_u4Height);
    virtual MINT32 getFlickerStatus(MINT32 *a_flickerStatus);

private:
    volatile int mUsers;
    mutable Mutex mLock;
    NSIspSysram::IspSysramDrv* m_pFlickerSysram;
    SensorDrv *m_pSensorDrv;
    IspDrv *m_pIspDrv;
    isp_reg_t *m_pIspRegMap;

    MBOOL  m_bFlickerEnable;
    MBOOL m_bFlickerEnablebit;
    MUINT32 m_u4FlickerWidth;
    MUINT32 m_u4FlickerHeight;
    MUINT32 m_u4SensorPixelClkFreq;
    MINT32 m_u4FreqFrame;
    MINT32 m_u4Freq000;
    MINT32 m_u4Freq100;
    MINT32 m_u4Freq120;
    MINT32 m_vAMDF[8];
    MUINT32 m_u4PixelsInLine;
    FLICKER_STATUS m_flickerStatus ;
    LMV_STATUS m_EIS_LMV_Flag;
    MINT32 *m_pVectorAddress1;
    MINT32 *m_pVectorAddress2;
    MINT8 m_iDataBank;
    MINT32 *m_pVectorData1;
    MINT32 *m_pVectorData2;
    MINT32 m_u4FlickerFreq;

    // add new interface by Maggie
    flkEISVector	m_EISvector;
    flkSensorInfo	m_sensorInfo;
    flkAEInfo		m_AEInfo;
};

#endif

