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
#ifndef _ISP_SYSRAM_DRV_H_
#define _ISP_SYSRAM_DRV_H_

namespace NSIspSysram
{


/*******************************************************************************
* 
*******************************************************************************/
typedef enum MERROR_ENUM
{
    MERR_OK         = 0, 
    MERR_UNKNOWN    = 0x80000000,   // Unknown error
    MERR_UNSUPPORT, 
    MERR_BAD_SYSRAM_DRV, 
    MERR_BAD_PARAM, 
    MERR_BAD_FORMAT, 
    MERR_HAS_ALLOCATED_BY_MYSELF, 
    MERR_HAS_ALLOCATED_BY_OTHERS, 
    MERR_NOT_ALLOCATED, 
    MERR_NO_MEM, 
    MERR_BUSY, 
    MERR_OVER_REF,                  // Over reference.
    MERR_UNDER_REF,                 // Under reference.
} MERROR_ENUM_T;


/*******************************************************************************
*
********************************************************************************/
typedef enum EUser
{
    EUsr_Begin      = 0, 
    EUsr_Flicker    = EUsr_Begin, 
    EUsr_PCA, 
    EUsr_EIS, 
    EUsr_LCE0, 
    EUsr_LCE1, 
    EUsr_Shading_Preview,
    EUsr_Shading_Capture,
    EUsr_Defect, 
    //..................................
    ENumOfUsr, 
    EUsr_None       = -1
} EUser_T;


/*******************************************************************************
*
********************************************************************************/
class IspSysramDrv
{
public:     ////
    static IspSysramDrv*    createInstance();
    virtual void            destroyInstance() = 0;

public:     ////    Interfaces.

    virtual MERROR_ENUM     init() = 0;
    virtual MERROR_ENUM     uninit() = 0;

    virtual MERROR_ENUM     alloc(
        EUser_T const eUsr, 
        MUINT32 const u4BytesToAllocate, 
        MUINT32&      ru4BytesAllocated
    ) = 0;

    virtual MERROR_ENUM     alloc(
        EUser_T const eUsr, 
        MUINT32 const u4BytesToAllocate, 
        MUINT32&      ru4BytesAllocated,
        MUINT32 const TimeoutMS
    ) = 0;

    virtual MERROR_ENUM     free(
        EUser_T const eUsr
    ) = 0;

    virtual MVOID*          getPhyAddr(EUser_T const eUsr) = 0;
    virtual MVOID*          getVirAddr(EUser_T const eUsr) = 0;

protected:  ////
    virtual ~IspSysramDrv() {};
};


/*******************************************************************************
*
********************************************************************************/
};  //  NSIspSysram
#endif // _ISP_SYSRAM_DRV_H_

