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
#ifndef _DBG_CAM_EXIF_H_
#define _DBG_CAM_EXIF_H_


/*******************************************************************************
*
********************************************************************************/
#include <scenario_types.h>
#include <cam_types.h>

/*******************************************************************************
*
********************************************************************************/
struct CamDbgParam
{
    MUINT32    u4ShotMode;
    
    CamDbgParam(MUINT32 _u4ShotMode = 0) : u4ShotMode(_u4ShotMode)
    {
    }
};

/*******************************************************************************
*
********************************************************************************/
class IspHal;
struct CAM_EXIF_DEBUG_INFO_T;

/*******************************************************************************
* Debug Camera Exif
********************************************************************************/
class DbgCamExif : public CamExif
{
public:     ////    Constructor/Destructor
    virtual ~DbgCamExif() {}
    DbgCamExif(ESensorType_t const eSensorType, EDeviceId_t const eDeviceId);

public:     ////    Interfaces.

    virtual MBOOL   init(CamExifParam const& rCamExifParam, Hal3ABase*const pHal3A, CamDbgParam const& rCamDbgParam);
    virtual MBOOL   uninit();

    virtual
    MBOOL
    appendDebugExif(
        MUINT8* const puAppnBuf,        //  [O] Pointer to APPn Buffer
        MUINT32*const pu4AppnSize       //  [O] Pointer to APPn Size
    );

    void getCamDebugInfo(CAM_EXIF_DEBUG_INFO_T &a_rCamExifDebugInfo);

protected:  ////    Data Members.
    IspHal*         mpIspHal;

private:
    CamDbgParam             mCamDbgParam;

};


#endif  //  _DBG_CAM_EXIF_H_

