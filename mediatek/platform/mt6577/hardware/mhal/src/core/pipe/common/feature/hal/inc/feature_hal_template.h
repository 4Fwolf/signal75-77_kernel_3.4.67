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
#ifndef _FEATURE_HAL_TEMPLATE_H_
#define _FEATURE_HAL_TEMPLATE_H_


namespace NSFeature
{


/*******************************************************************************
*
*******************************************************************************/
template <ECamRole_T _RoleId, MUINT32 _SceneNum>
class FeatureHalTemplate : public FeatureHalBridge
{
public:     ////

    static ECamRole_T const ECamRole = _RoleId;
    enum
    {
        ESceneNum   = _SceneNum
    };

private:    ////    Disallowed.
    //  Copy-assignment operator is disallowed.
    FeatureHalTemplate& operator=(FeatureHalTemplate const&);

private:    ////    Ctor/Dtor/Init/Deinit.
    FeatureHalTemplate();

public:     ////
    static FeatureHal*  getInstance();

public:     ////    Interfaces.
    virtual void        destroyInstance();

public:     ////    Attributes.
    virtual FInfoIF*    GetFInfo(FID_T const fid);
    virtual FInfoIF*    GetFInfo(FID_T const fid, SID_T const sid);

    virtual ECamRole_T  GetCamRole()   const { return ECamRole; }

    virtual
    SID_T const*
    SceneBegin()
    {
        return  m_pFInfo_Scene->GetTable();
    }

    virtual
    SID_T const*
    SceneEnd()
    {
        return  SceneBegin() + m_pFInfo_Scene->GetCount();
    }

    virtual
    MBOOL
    IsSceneSupport(SID_T const sid) const
    {
        return sm_afgSceneSupport[sid];
    }

    virtual
    void
    UpdateScenesInfo()
    {
        ::memset(sm_afgSceneSupport, 0, sizeof(sm_afgSceneSupport));
        SID_T const* pSid = SceneBegin();
        SID_T const* const pSEnd = SceneEnd();
        for (; pSEnd != pSid; pSid++)
        {
            sm_afgSceneSupport[*pSid] = MTRUE;
        }
        DumpScenesInfo();
    }

private:    ////    Scene-Mode Information.
    FInfoIF*        m_pFInfo_Scene;

private:    ////
    static MBOOL    sm_afgSceneSupport[];

};


};  //  namespace NSFeature


#endif // _FEATURE_HAL_TEMPLATE_H_

