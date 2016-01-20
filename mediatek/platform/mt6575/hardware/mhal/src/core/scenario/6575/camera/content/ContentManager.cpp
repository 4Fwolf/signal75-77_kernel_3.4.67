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
#define LOG_TAG "CamContent"
//
#include <utils/Errors.h>
#include <utils/threads.h>
//
#include <scenario_types.h>
//
#include "my_log.h"
//
#include "IContentManager.h"


using namespace android;
using namespace NSContent;


/*******************************************************************************
*
********************************************************************************/
class ContentMgr : public IContentManager
{
protected:  ////    Constructor/Destructor.
    ContentMgr();
    virtual ~ContentMgr();

public:     ////    Interfaces.
    //
    static ContentMgr&  getInstance();
    //
    virtual MBOOL   queryContent(ECtnID const eCtnID, MVOID* pBuf);

    //  FIXME: should not be here.
    virtual MBOOL   updateContent(ECtnID const eCtnID, MVOID const* pBuf);

protected:  ////
    mutable Mutex   mLockContent;
    MBOOL           lockContent()   { return mLockContent.lock() == 0; }
    MVOID           unlockContent() { return mLockContent.unlock(); }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  EIS Global Motion Vector.
//      FIXME: should not be here.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    MINT32          mai4GmvX[1];
    MINT32          mai4GmvY[1];
    virtual MBOOL   queryContent_EisGmv(MVOID* pBuf);
    virtual MBOOL   updateContent_EisGmv(MVOID const* pBuf);
    
};


/*******************************************************************************
*
********************************************************************************/
IContentManager&
IContentManager::
getInstance()
{
    return  ContentMgr::getInstance();
}



/*******************************************************************************
*
********************************************************************************/
ContentMgr&
ContentMgr::
getInstance()
{
    static  ContentMgr inst;
    return  inst;
}


/*******************************************************************************
*
********************************************************************************/
ContentMgr::
ContentMgr()
    : mLockContent()
{
}


/*******************************************************************************
*
********************************************************************************/
ContentMgr::
~ContentMgr()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ContentMgr::
queryContent(ECtnID const eCtnID, MVOID* pBuf)
{
    MBOOL ret = MFALSE;
    //
    if  ( ! pBuf )
    {
        MY_LOGE("[ContentMgr::queryContent]<ID:%d> NULL pBuf", eCtnID);
        return  MFALSE;
    }
    //
    if  ( ! lockContent() )
    {
        MY_LOGE("[ContentMgr::queryContent]<ID:%d> lockContent fail", eCtnID);
        return  MFALSE;
    }
    //
    switch  ( eCtnID )
    {
    case eCtnID_EisGmv:
        ret = queryContent_EisGmv(pBuf);
        break;
    default:
        MY_LOGW("[ContentMgr::queryContent] bad content id:%d", eCtnID);
        break;
    }

    unlockContent();
    return  ret;
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
ContentMgr::
updateContent(ECtnID const eCtnID, MVOID const* pBuf)
{
    MBOOL ret = MFALSE;
    //
    if  ( ! pBuf )
    {
        MY_LOGE("[ContentMgr::updateContent]<ID:%d> NULL pBuf", eCtnID);
        return  MFALSE;
    }
    //
    if  ( ! lockContent() )
    {
        MY_LOGE("[ContentMgr::updateContent]<ID:%d> lockContent fail", eCtnID);
        return  MFALSE;
    }

    switch  ( eCtnID )
    {
    case eCtnID_EisGmv:
        ret = updateContent_EisGmv(pBuf);
        break;
    default:
        MY_LOGW("[ContentMgr::updateContent] bad content id:%d", eCtnID);
        break;
    }

    unlockContent();
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ContentMgr::
queryContent_EisGmv(MVOID* pBuf)
{
    Ctn_EisGmv*const pCtn = reinterpret_cast<Ctn_EisGmv*>(pBuf);
    //
    pCtn->u4Count = 1;
    pCtn->pi4GmvX[0] = mai4GmvX[0];
    pCtn->pi4GmvY[0] = mai4GmvY[0];
    //
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ContentMgr::
updateContent_EisGmv(MVOID const* pBuf)
{
    //  FIXME: should use another structure.
    Ctn_EisGmv const*const pCtn = reinterpret_cast<Ctn_EisGmv const*>(pBuf);
    //
    mai4GmvX[0] = pCtn->pi4GmvX[0];
    mai4GmvY[0] = pCtn->pi4GmvY[0];
    //
    return  MTRUE;
}

