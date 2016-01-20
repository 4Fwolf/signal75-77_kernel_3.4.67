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
#define LOG_TAG "mHalCam"

#include <cutils/xlog.h>
#include <stdlib.h>

#include <mhal/inc/camera.h>
#include <cam_types.h>

#include <camexif/CamExif.h>
#include <jpeg_hal.h>
#include "MpoEncoder.h"
#include "CamJpg.h"
#include "aaa_hal_base.h"


/*******************************************************************************
*
*******************************************************************************/
#define MY_DBG(fmt, arg...)     XLOGD("[CamJpg]"fmt, ##arg)
#define MY_ERR(fmt, arg...)     XLOGE("[CamJpg]<%s:#%d>"fmt, __FILE__, __LINE__, ##arg)

/*******************************************************************************
*
********************************************************************************/

CamJpg::~CamJpg()
{
    mpHal3AObj = NULL;
    mpMhalCam = NULL;
    mpmhalCamParam = NULL;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 
CamJpg::doJpg(
    MBOOL isExif, 
    FrameParam  psrc, 
    FrameParam  pdst,
    MUINT32 shiftSrc,
    MUINT32 shiftDst
)
{
    MUINT32 JpgSize = 0;

    MY_DBG("[pSrc] shiftSize: %d, virtAddr: 0x%x, frmSize: %d, buffSize: %d", shiftSrc, psrc.virtAddr, psrc.frmSize, psrc.bufSize);
    MY_DBG("[pDst] shiftSize: %d, virtAddr: 0x%x, frmSize: %d, buffSize: %d", shiftDst, pdst.virtAddr, pdst.frmSize, pdst.bufSize);

    pdst.virtAddr += shiftDst;
    psrc.virtAddr += shiftSrc;

    if(!isExif)
    {
        encodeJpg(&psrc, &pdst, 1);
        JpgSize = pdst.frmSize;
    }
    else
    {
        // (1) Get Exif information
        MUINT32 ExifHeaderSize=0;
        CamExif camexif(meSensorType, NSCamera::eDevId_ImgSensor0);
        camexif.init(
                CamExifParam(mpmhalCamParam->camExifParam, mpmhalCamParam->cam3AParam, 100), 
                mpHal3AObj
        );
        camexif.makeExifApp1(160, 120, 0, (MUINT8*)pdst.virtAddr, &ExifHeaderSize);
        camexif.uninit();
        MY_DBG("ExifHeaderSize1: 0x%x \n", ExifHeaderSize);

        // (2) Encode main JPEG
        FrameParam tdst(pdst); 
        tdst.virtAddr += tdst.bufSize; // be aware of not frmSize
        encodeJpg(&psrc, &tdst, 0);

        MUINT8 *Jsrc, *Jdst;
        Jsrc = (MUINT8*)tdst.virtAddr;
        Jdst = (MUINT8*)pdst.virtAddr + ExifHeaderSize;
        memcpy(Jdst, Jsrc, tdst.frmSize);
        JpgSize = ExifHeaderSize + tdst.frmSize; // can be small: ideally, ExifHeaderSize+tdst.frmSize
    }

    return JpgSize;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
CamJpg::createMPO(MPImageInfo * pMPImageInfo, MUINT32 num, char* file, MUINT32 MPOType)
{
    MINT32 err = MHAL_NO_ERROR;
    MBOOL ok;
    MpoEncoder* mpoEncoder = new MpoEncoder();
    if (mpoEncoder) {
        ok = mpoEncoder->setJpegSources(TYPE_Disparity, pMPImageInfo, num);

        if (!ok) {
            MY_ERR("  mpoEncoder->setJpegSources fail \n");
            err = MHAL_UNKNOWN_ERROR;
            goto mHalCamMAVMakeMPO_EXIT;
        }

        ok = mpoEncoder->encode(file, MPOType);

        if (!ok) {
            MY_ERR("  mpoEncoder->encode fail \n");
            err = MHAL_UNKNOWN_ERROR;
            goto mHalCamMAVMakeMPO_EXIT;
        }
        
        MY_DBG("[mHalCamMAVMakeMPO] Done, %s \n", file);
    }
    else
    {
        MY_DBG("new MpoEncoder() fail");
        return MHAL_UNKNOWN_ERROR;
    }
    
mHalCamMAVMakeMPO_EXIT:
    delete mpoEncoder;
    delete [] pMPImageInfo;  

    return err;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamJpg::
encodeJpg(mhalCamFrame_t*const psrc, mhalCamFrame_t*const pdst, MBOOL const isSOI)
{
    MY_DBG("[encodeJpg]");

    MBOOL   ret = MFALSE;
    //
    MUINT32 u4JpgSize = 0;
    //
    JpgEncHal jpgEnc;

    jpgEnc.setSrcWidth(psrc->w);
    jpgEnc.setSrcHeight(psrc->h);
    jpgEnc.setDstWidth(pdst->w);
    jpgEnc.setDstHeight(pdst->h);
    jpgEnc.setQuality(85);
    jpgEnc.setSrcFormat((JpgEncHal::SrcFormat)psrc->frmFormat);
    jpgEnc.setEncFormat(JpgEncHal::kYUV_422_Format);
    #if defined(MTK_M4U_SUPPORT) 
    jpgEnc.setSrcAddr((void *) psrc->virtAddr);
    jpgEnc.setDstAddr((void *) pdst->virtAddr);
    #else
    jpgEnc.setSrcAddr((void *) psrc->phyAddr);
    jpgEnc.setDstAddr((void *) pdst->phyAddr);
    #endif
    jpgEnc.setDstSize(pdst->bufSize);
    jpgEnc.enableSOI(isSOI);

    //FIXME, 
    //jpgEnc.setWaitResTime(500);
    //
    if  ( ! jpgEnc.lock() ) {
        MY_ERR("[encodeJpg] jpgEnc.lock() failed");
        goto lbExit;
    }
    //
    if ( ! jpgEnc.start(&u4JpgSize) ) {
        MY_ERR("[encodeJpg] jpgEnc.start() failed");
    }

    jpgEnc.unlock();

    ret = MTRUE;

lbExit:

    pdst->frmSize = u4JpgSize;

    return  ret;
}

