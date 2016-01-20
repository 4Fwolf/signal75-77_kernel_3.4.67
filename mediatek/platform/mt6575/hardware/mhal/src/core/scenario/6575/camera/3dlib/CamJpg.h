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
#ifndef _CAM_JPG_H_
#define _CAM_JPG_H_

/*******************************************************************************
*
********************************************************************************/
#include <utils/Errors.h>
#include <stdlib.h>
#include <utils/threads.h>
#include <cutils/properties.h>
#include "mpo_type.h"

/*******************************************************************************
*
********************************************************************************/
class MPImageInfo;
class mHalCam;
class Hal3ABase;

class FrameParam : public mhalCamFrame_s{
    public:
		FrameParam() { ::memset(this, 0, sizeof(mhalCamFrame_s));}

	    FrameParam(mhalCamFrame_s param)
			: mhalCamFrame_s(param)
	    {}

		FrameParam(
			MUINT32 width,
			MUINT32 height,
			MUINT32 fSize,
			MUINT32 bSize,
			MINT32 format,
			MUINT32 vAddr,
			MUINT32 pAddr = 0xFFFFFFFF
		)
		{
		    w = width;
		    h = height;
		    frmSize = fSize;
		    bufSize = bSize;
		    frmFormat = format;
	        virtAddr = vAddr;
		    phyAddr = pAddr;
		}
};  


/*******************************************************************************
*
********************************************************************************/
class CamJpg
{
    public:
		virtual ~CamJpg();
		
		CamJpg(
            Hal3ABase*      const pHal3AObj,
            mHalCam*        const pMhalCam,
            mhalCamParam_s* const pmhalCamParam,
            NSCamera::ESensorType const eSensorType
	    )
	        :mpHal3AObj(pHal3AObj)
		    ,mpMhalCam(pMhalCam)
		    ,mpmhalCamParam(pmhalCamParam)
		    ,meSensorType(eSensorType)
		{}
		    
	private:
		Hal3ABase*      mpHal3AObj;
		mHalCam*        mpMhalCam;
		mhalCamParam_s* mpmhalCamParam;
		NSCamera::ESensorType const meSensorType;
	

	public:
		MUINT32 doJpg(MBOOL isExif, FrameParam psrc, FrameParam pdst, MUINT32 shiftSrc, MUINT32 shiftDst);
		MINT32  createMPO(MPImageInfo * pMPImageInfo, MUINT32 num, char* file, MUINT32 MPOType = MTK_TYPE_MAV);

    protected:  ////    Utilities.
        MBOOL   encodeJpg(mhalCamFrame_t*const psrc, mhalCamFrame_t*const pdst, MBOOL const isSOI);
};


#endif
