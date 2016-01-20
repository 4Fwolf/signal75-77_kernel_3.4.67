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
#ifndef RES_MGR_DRV_H
#define RES_MGR_DRV_H
//----------------------------------------------------------------------------
#include "drv_types.h"
//----------------------------------------------------------------------------
#define RES_MGR_DRV_LOG_MSG     0x00000001
#define RES_MGR_DRV_LOG_WRN     0x00000002
#define RES_MGR_DRV_LOG_ERR     0x00000004
//----------------------------------------------------------------------------
#define RES_MGR_DRV_RES_ISP     0x00000001
//----------------------------------------------------------------------------

typedef struct
{
    MUINT32     ResMask;
    MUINT32     PermanentMask;
    MUINT32     Timeout;
}RES_MGR_DRV_LOCK_STRUCT;

typedef struct
{
    MUINT32     ResLockMask;
    MUINT32     PermanentMask;
}RES_MGR_DRV_CEHCK_STRUCT;

typedef enum
{
    RES_MGR_DRV_MODE_NONE,
    RES_MGR_DRV_MODE_PREVIEW_OFF,
    RES_MGR_DRV_MODE_PREVIEW_ON,
    RES_MGR_DRV_MODE_CAPTURE_OFF,
    RES_MGR_DRV_MODE_CAPTURE_ON,
    RES_MGR_DRV_MODE_VIDEO_REC_OFF,
    RES_MGR_DRV_MODE_VIDEO_REC_ON,
}RES_MGR_DRV_MODE_ENUM;
//----------------------------------------------------------------------------
class ResMgrDrv
{
    protected:
        virtual ~ResMgrDrv() {};
    //
    public:
        static ResMgrDrv* CreateInstance(void);
        virtual void    DestroyInstance(void) = 0;
        virtual MBOOL   Init(void) = 0;
        virtual MBOOL   Uninit(void) = 0;
        virtual MBOOL   LockRes(RES_MGR_DRV_LOCK_STRUCT* pLockInfo) = 0;
        virtual MBOOL   UnlockRes(MUINT32 ResMask) = 0;
        virtual MBOOL   CheckRes(RES_MGR_DRV_CEHCK_STRUCT* pCheckInfo) = 0;
        virtual MBOOL   SetSysramSwitchbank(MBOOL En) = 0;
        virtual MBOOL   CloseTvout(MBOOL En) = 0;
        virtual MBOOL   SetBWC(RES_MGR_DRV_MODE_ENUM Mode) = 0;
        virtual MBOOL   CloseHdmi(MBOOL En) = 0;
        virtual MBOOL   ForceHdmiFullscreen(MBOOL En) = 0;
        virtual MBOOL   EnableLog(MUINT32 LogMask) = 0;
        virtual MBOOL   DisableLog(MUINT32 LogMask) = 0;
};
//----------------------------------------------------------------------------
#endif

