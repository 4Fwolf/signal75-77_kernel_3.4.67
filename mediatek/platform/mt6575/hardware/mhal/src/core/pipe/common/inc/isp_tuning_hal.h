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
#ifndef _ISP_TUNING_HAL_H_
#define _ISP_TUNING_HAL_H_

#include "isp_tuning.h"     // in custom folder
#include "isp_exif_debug.h"


namespace NSIspTuning
{


typedef enum ESensorMode
{
    ESensorMode_Preview, 
    ESensorMode_Capture
} ESensorMode_T;

typedef enum ENCmd {
    ECmd_DecideOfflineCapture, 
    ECmd_SetDynamicTuning, 
    ECmd_GetDynamicTuning, 
    ECmd_SetDynamicCCM, 
    ECmd_GetDynamicCCM, 
    ECmd_SetForceCtrl_Gamma, 
    ECmd_GetForceCtrl_Gamma, 
    ECmd_ModShadingNVRAMData
} ENCmd_T;


typedef struct CmdArg {
    ENCmd_T eCmd;
    MVOID*  pInBuf;
    MUINT32 u4InBufSize;
    MVOID*  pOutBuf;
    MUINT32 u4OutBufSize;
    MUINT32 u4ActualOutSize;
} CmdArg_T;


/*******************************************************************************
*
*******************************************************************************/
class IspTuningHal
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Ctor/Dtor.
    virtual ~IspTuningHal() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations.
    static IspTuningHal*    createInstance(
        ESensorType_T const eSensorType, 
        ESensorRole_T const eSensorRole
    );
    virtual void    destroyInstance() = 0;
    virtual MINT32  construct() = 0;
    virtual MINT32  init() = 0;
    virtual MINT32  uninit() = 0;
    virtual MINT32  validate(bool const fgForce = false) = 0;
    virtual MINT32  validateFrameless() = 0;
    virtual MINT32  validatePerFrame(bool const fgForce = false) = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Attributes.
    virtual ESensorRole_T   getSensorRole() const = 0;

    virtual EOperMode_T getOperMode()   const = 0;
    virtual MINT32      setOperMode(
        EOperMode_T const eOperMode = EOperMode_Normal
    ) = 0;

    virtual MINT32  setSensorMode(ESensorMode_T const eSensorMode) = 0;
    virtual MINT32  setCamMode(ECamMode_T const eCamMode) = 0;
    virtual MINT32  setSceneMode(MUINT32 const u4Scene) = 0;
    virtual MINT32  setEffect(MUINT32 const u4Effect) = 0;

    virtual MINT32  updateIso(MUINT32 const u4ISOValue) = 0;
    virtual MINT32  updateFluorescentAndCCT (MVOID*const pCCT) = 0;
    virtual MINT32  setShadingIndex (MINT32 const i4IDX) = 0;
    virtual MINT32  getShadingIndex (MVOID*const pCmdArg) = 0;

    virtual MINT32  updateZoomRatio(MUINT32 const u4ZoomRatio_x100) = 0;
    virtual MINT32  updateSceneLightValue(MINT32 const i4SceneLV_x10) = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    ISP End-User-Defined Tuning Index.
    virtual MINT32  setIspUserIdx_Edge(MUINT32 const u4Index) = 0;
    virtual MINT32  setIspUserIdx_Hue(MUINT32 const u4Index) = 0;
    virtual MINT32  setIspUserIdx_Sat(MUINT32 const u4Index) = 0;
    virtual MINT32  setIspUserIdx_Bright(MUINT32 const u4Index) = 0;
    virtual MINT32  setIspUserIdx_Contrast(MUINT32 const u4Index) = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Exif.

    virtual MINT32  queryExifDebugInfo(
        NSIspExifDebug::IspExifDebugInfo_T& rExifDebugInfo
    ) const = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Commands.
    virtual MINT32  sendCommand(MVOID*const pCmdArg) = 0;

};


};  //  namespace NSIspTuning
#endif // _ISP_TUNING_HAL_H_

