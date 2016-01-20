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
#ifndef _CAM_PORT_H_
#define _CAM_PORT_H_


#include <mhal/inc/camera/graphics.h>


#include <utils/Vector.h>
using namespace android;


/*******************************************************************************
* 
*******************************************************************************/
namespace NSCamera
{


/**
 * 
 * 
 */
struct ImgInfo
{
public:     //// fields.
    typedef EPixelFormat EDataFmt_t;
    EDataFmt_t  eImgFmt;        //  Image Data Format
    MUINT32     u4ImgWidth;     //  Image Width
    MUINT32     u4ImgHeight;    //  Image Height
    MUINT32     u4ImgStride;    //  Image Stride;

public:     //// constructors.
    ImgInfo(
        EDataFmt_t const _eImgFmt = NSCamera::ePIXEL_FORMAT_BAYER10, 
        MUINT32 const _u4ImgWidth = 0, 
        MUINT32 const _u4ImgHeight = 0,
        MUINT32 const _u4ImgStride = 0
    )
        : eImgFmt(_eImgFmt)
        , u4ImgWidth(_u4ImgWidth)
        , u4ImgHeight(_u4ImgHeight)
        , u4ImgStride(_u4ImgStride)
    {}
};


/**
 * 
 * 
 */
struct BufInfo
{
public:     //// fields.
    MUINT32     u4BufVA;    //  Vir Address
    MUINT32     u4BufPA;    //  Phy Address
    MUINT32     u4BufSize;  //  Per buffer size
    MUINT32     u4BufCnt;   //  buffer count

public:     //// constructors.
    BufInfo(
        MUINT32 const _u4BufVA = 0, 
        MUINT32 const _u4BufPA = 0, 
        MUINT32 const _u4BufSize = 0, 
        MUINT32 const _u4BufCnt  = 1
    )
        : u4BufVA(_u4BufVA)
        , u4BufPA(_u4BufPA)
        , u4BufSize(_u4BufSize)
        , u4BufCnt(_u4BufCnt)
    {}
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamera


/*******************************************************************************
* 
*******************************************************************************/
namespace NSCamera
{
////////////////////////////////////////////////////////////////////////////////
struct PortInfo;


/**
 * 
 * 
 */
class PortVect : public Vector<PortInfo const*>
{
};


/**
 * 
 * 
 */
enum EPortID
{
    ePortID_Memory, 
    ePortID_Sensor, 
    ePortID_Jpeg, 
    ePortID_Postview, 
    ePortID_Bypass = 0x80000000
};


/**
 * 
 * 
 */
struct PortInfo : public ImgInfo, public BufInfo
{
public:     //// fields.
    EPortID     ePortID;

public:     //// constructors.
    PortInfo(
        EPortID const _ePortID
    )
        : ImgInfo()
        , BufInfo()
        , ePortID(_ePortID)
    {}

    PortInfo(
        ImgInfo const& rImgInfo, 
        BufInfo const& rBufInfo, 
        EPortID const _ePortID = ePortID_Memory
    )
        : ImgInfo(rImgInfo)
        , BufInfo(rBufInfo)
        , ePortID(_ePortID)
    {}

    PortInfo(
        ImgInfo const& rImgInfo, 
        EPortID const _ePortID = ePortID_Memory
    )
        : ImgInfo(rImgInfo)
        , BufInfo()
        , ePortID(_ePortID)
    {}

    PortInfo(
        BufInfo const& rBufInfo, 
        EPortID const _ePortID = ePortID_Memory
    )
        : ImgInfo()
        , BufInfo(rBufInfo)
        , ePortID(_ePortID)
    {}

    PortInfo(
        EDataFmt_t const _eImgFmt = ePIXEL_FORMAT_BAYER10, 
        MUINT32 const _u4ImgWidth = 0, 
        MUINT32 const _u4ImgHeight = 0, 
        MUINT32 const _u4BufVA = 0, 
        MUINT32 const _u4BufPA = 0, 
        MUINT32 const _u4BufSize = 0, 
        MUINT32 const _u4BufCnt  = 1, 
        EPortID const _ePortID = ePortID_Memory
    )
        : ImgInfo(_eImgFmt, _u4ImgWidth, _u4ImgHeight)
        , BufInfo(_u4BufVA, _u4BufPA, _u4BufSize, _u4BufCnt)
        , ePortID(_ePortID)
    {}
};


/**
 * 
 * 
 */
struct MemoryPortInfo : public PortInfo
{
public:     //// fields.
    MUINT32     u4Rotation;     //Rotation, 0, 90, 180, 270
    MUINT32     u4Flip;         //Flip,  0, 1:H_FLIP, 2:V_FLIP
    MUINT32     u4zoomRatio;    //Zoom ratio x 100
	MBOOL		fgDumpYuvData;	//0: isp dump raw data for 1st capture pass,  1: mdp dump yuv data for 1st capture pass 

public:     //// constructors.
    MemoryPortInfo(
        EPortID const _ePortID = ePortID_Memory
    )
        : PortInfo(_ePortID)
        , u4Rotation(0)
        , u4Flip(0)
        , u4zoomRatio(100)
        , fgDumpYuvData(MFALSE)
    {}

    MemoryPortInfo(
        ImgInfo const& rImgInfo, 
        BufInfo const& rBufInfo, 
        MUINT32 const _u4Rotation = 0, 
        MUINT32 const _u4Flip = 0, 
        MUINT32 const _u4zoomRatio = 100, 
        EPortID const _ePortID = ePortID_Memory
    )
        : PortInfo(rImgInfo, rBufInfo, _ePortID)
        , u4Rotation(_u4Rotation)
        , u4Flip(_u4Flip)
        , u4zoomRatio(_u4zoomRatio)
    {}
};


/**
 * 
 * 
 */
struct SensorPortInfo : public PortInfo
{
public:     //// fields.
    MBOOL       fgBypassSensorDelay;    //  bypass sensor delay if ture;
    MBOOL       fgBypassSensorScenaio;

public:     //// constructors.
    SensorPortInfo()
        : PortInfo(ePortID_Sensor)
        , fgBypassSensorDelay(MFALSE)
        , fgBypassSensorScenaio(MFALSE)
    {}

    SensorPortInfo(
        ImgInfo const& rImgInfo, 
        BufInfo const& rBufInfo, 
        MBOOL const _fgBypassSensorDelay = MFALSE, 
        MBOOL const _fgBypassSensorScenaio = MFALSE
    )
        : PortInfo(rImgInfo, rBufInfo, ePortID_Sensor)
        , fgBypassSensorDelay(_fgBypassSensorDelay)
        , fgBypassSensorScenaio(_fgBypassSensorScenaio)
    {}
};


/**
 * 
 * 
 */
struct JpegPortInfo : public MemoryPortInfo
{
public:     //// fields.
    MUINT32     u4Quality;
    MBOOL       fgIsSOI;
    MBOOL       fgIsDither;

public:     //// constructors.
    JpegPortInfo()
        : MemoryPortInfo(ePortID_Jpeg)
        , u4Quality(0)
        , fgIsSOI(MFALSE)
        , fgIsDither(MFALSE)
    {}

    JpegPortInfo(
        ImgInfo const& rImgInfo, 
        BufInfo const& rBufInfo, 
        MUINT32 const _u4Rotation = 0, 
        MUINT32 const _u4Flip = 0, 
        MUINT32 const _u4zoomRatio = 100, 
        MUINT32 const _u4Quality = 0, 
        MBOOL const _fgIsSOI = MFALSE, 
        MBOOL const _fgIsDither = MFALSE
    )
        : MemoryPortInfo(rImgInfo, rBufInfo, _u4Rotation, _u4Flip, _u4zoomRatio, ePortID_Jpeg)
        , u4Quality(_u4Quality)
        , fgIsSOI(_fgIsSOI)
        , fgIsDither(_fgIsDither)
    {}
};


/**
 * 
 * 
 */
struct PostviewPortInfo : public MemoryPortInfo
{
public:     //// fields.

public:     //// constructors.
    PostviewPortInfo()
        : MemoryPortInfo(ePortID_Postview)
    {}

    PostviewPortInfo(
        ImgInfo const& rImgInfo, 
        BufInfo const& rBufInfo, 
        MUINT32 const _u4Rotation = 0, 
        MUINT32 const _u4Flip = 0, 
        MUINT32 const _u4zoomRatio = 100
    )
        : MemoryPortInfo(rImgInfo, rBufInfo, _u4Rotation, _u4Flip, _u4zoomRatio, ePortID_Postview)
    {}
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamera
#endif  //  _CAM_PORT_H_

