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
#define LOG_TAG "FeatureHalBridge_query"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG           (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#include "debug.h"
//
#include "camera_feature.h"
#include "feature_hal.h"
#include "feature_hal_bridge.h"
#include "control_base.h"

using namespace NSFeature;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Legacy Interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace
{


static
MBOOL
FInfo2LegacyStruct(FInfoIF*const pFInfo, CamFeatureStruct*const pCFS)
{
    FID_T const fid = pFInfo->GetFID();

    pCFS->u4FeatureID = fid;

    pCFS->u4FIDSupportFlag  = pFInfo->GetCount() > 0;
    pCFS->u4SubItemTotalNum = pFInfo->GetCount();

    pCFS->u4DefaultSelection = pFInfo->GetDefault();
    pCFS->pu4SubItemAllSupport= (MUINT32*)pFInfo->GetTable();

    return  MTRUE;
}


static
void
DumpLegacyFeatures(CamFeatureStruct aCFS[], MUINT32 const u4Count)
{
#if ENABLE_MY_LOG
    for (MUINT32 f = 0; f < u4Count; f++)
    {
        CamFeatureStruct& r = aCFS[f];
        MUINT32 u4Mask = 0;
        for (MUINT32 i = 0; i < r.u4SubItemTotalNum; i++)
        {
#if 0
            MY_LOG("    %d", r.pu4SubItemAllSupport[i]);
#endif
            u4Mask |= (1<<r.pu4SubItemAllSupport[i]);
        }
        MY_LOG(
            "[Legacy FID:%-2d](support,num,default)=(%d,%-2d,%-2d)"
            " MASK:[0x%08X]\n"
            , r.u4FeatureID, r.u4FIDSupportFlag
            , r.u4SubItemTotalNum, r.u4DefaultSelection
            , u4Mask
        );
    }
#endif  //  ENABLE_MY_LOG
}


};  //  namespace


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation of class FeatureHalBridge
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
FeatureHalBridge::
queryFeatures(
    MVOID* const  pOutBuf, 
    MUINT32 const u4OutBufNum, 
    MUINT32&      ru4NumReturned
)
{
    CamFeatureStruct*paCFS = reinterpret_cast<CamFeatureStruct*>(pOutBuf);
    MUINT32     u4Count = 0;

    FInfoIF*    pFInfo = NULL; // target feature

    FID_T fid = 0;
    SID_T sid = 0;

    MY_LOG("[+queryFeatures]\n");

    if  ( ! pOutBuf || 0 == u4OutBufNum )
    {
        MY_LOG("[queryFeatures][Invalid Parameters]:"
               "(pOutBuf, u4OutBufNum)=(%p,%d)\n", 
                 pOutBuf, u4OutBufNum);
        goto lbExit;
    }


    //  Scene-Indep. Features.
    MY_LOG("[+queryFeatures][SI]");
    for (fid = FID_BEGIN_SI; fid < FID_END_SI; fid++)
    {
        //  Target feature.
        pFInfo = GetFInfo(fid);
        if  ( ! pFInfo )
            continue;

        FInfo2LegacyStruct(pFInfo, paCFS + u4Count);
        u4Count++;
#if 0
        pFInfo->Dump();
#endif
        if  ( u4OutBufNum <= u4Count )
        {
            goto lbExit;
        }
    }

    //  Scene-Dep. Features.
    sid = m_rCtrl.GetSceneMode();
    MY_LOG("[+queryFeatures][SD]sid=%d", sid);
    for (fid = FID_BEGIN_SD; fid < FID_END_SD; fid++)
    {
        //  Target feature.
        pFInfo = GetFInfo(fid, sid);
        if  ( ! pFInfo )
            continue;

        FInfo2LegacyStruct(pFInfo, paCFS + u4Count);
        u4Count++;
#if 0
        pFInfo->Dump();
#endif
        if  ( u4OutBufNum <= u4Count )
        {
            goto lbExit;
        }
    }


    if  ( u4OutBufNum > u4Count )
    {
        ::memset(
            paCFS + u4Count, 
            0, 
            sizeof(CamFeatureStruct)*(u4OutBufNum-u4Count)
        );
        MY_LOG(
            "[queryFeatures] u4OutBufNum:%d > u4Count:%d \n"
            , u4OutBufNum, u4Count
        );
    }


lbExit:

    ru4NumReturned = u4Count;

    DumpLegacyFeatures(paCFS, u4Count);

    MY_LOG("[-queryFeatures][RoleId:%d] Count(%d)\n", GetCamRole(), u4Count);
    //  Return success if any feature is written into the out buffer.
    return  ( 0 < u4Count );
}

