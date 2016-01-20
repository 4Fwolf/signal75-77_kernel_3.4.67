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
#ifndef N3D_NORMAL_SHOT_H
#define N3D_NORMAL_SHOT_H
//-----------------------------------------------------------------------------
#include "N3D.h"
#include "mcam_mem.h"
#include "isp_hal.h"
#include "mdp_hal.h"
#include "mhal_interface.h"

//-----------------------------------------------------------------------------
class IShotN3D;
class ShotBaseN3D;
class NormalShotN3D;
class ICameraIO;
class mHalCamN3D;
using NSCamera::SensorPortInfo;
using namespace NSCamera;
//-----------------------------------------------------------------------------
class N3D_NormalShot : public NormalShotN3D
{
    //Constructor/Destructor.
    protected:
        N3D_NormalShot(
            char const*const    szShotName,
            ESensorType_t const eSensorType,
            EDeviceId_t const   eDeviceId);
    //
    public:
        static N3D_NormalShot*   createInstance(
            ESensorType_t const eSensorType,
            EDeviceId_t const   eDeviceId);
        virtual MVOID   destroyInstance(void);
        virtual MBOOL   init(
            mHalCamN3D* const      pMhalCam,
            ShotParam const&    rShotParam,
            Hal3ABase*const     pHal3A,
            ICameraIO*const     pCameraIO,
            N3D_FILE_TYPE_ENUM   FileType);
        virtual MBOOL   capture(void);
    //
    private:
        virtual MBOOL handleCaptureDone(MINT32 const i4ErrCode = 0);
        //
        virtual MBOOL createJpgImage(MUINT32 u4SrcAddr, MdpColorFormat emInputFormat, MBOOL fgShowQuickView);
        //
        virtual MINT32 configCapPort(ShotParam* pShotParam, 
                             ICameraIO::ImgCfg_t* pCapCfg, 
                             ICameraIO::MemInfo_t* pCapMemCfg
                             ); 
        virtual MBOOL setConfigCapture(); 
        //
        mHalCamMemPool*             mCapWorkingBuf;
        mHalCamN3D*                 mpMhalCam;
        N3D_FILE_TYPE_ENUM          mFileType;
        ISP_HAL_CAP_3D_TYPE_ENUM    m3DCapType;
        //
        
};
//-----------------------------------------------------------------------------
#endif  //N3D_NORMAL_SHOT_H

