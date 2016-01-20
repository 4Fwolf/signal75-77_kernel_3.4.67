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
#ifndef _ISP_DRV_MGR_H_
#define _ISP_DRV_MGR_H_

#include <isp_drv.h>

namespace NSIspTuning
{


/*******************************************************************************
* ISP Driver Manager
*******************************************************************************/
class IspDrvMgr
{
public:
    typedef enum MERROR_ENUM
    {
        MERR_OK         = 0, 
        MERR_UNKNOWN    = 0x80000000, // Unknown error
        MERR_BAD_ISP_DRV, 
        MERR_BAD_ISP_ADDR
    } MERROR_ENUM_T;

    typedef IspDrv::reg_t   reg_t;

public:     ////    Interfaces.
    virtual volatile void*  getIspReg() const = 0;
    virtual MBOOL           readRegs(reg_t*const pRegs, MUINT32 const count) = 0;
    virtual MBOOL           writeRegs(reg_t*const pRegs, MUINT32 const count) = 0;
    virtual MERROR_ENUM_T   init() = 0;
    virtual MERROR_ENUM_T   uninit() = 0;
    virtual MERROR_ENUM_T   reinit() = 0;

public:     ////
    static IspDrvMgr&   getInstance();

protected:
    virtual ~IspDrvMgr() {}
};


/*******************************************************************************
* 
*******************************************************************************/


};  //  NSIspTuning
#endif // _ISP_DRV_MGR_H_

