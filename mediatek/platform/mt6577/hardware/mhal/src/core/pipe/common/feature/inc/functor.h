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
#ifndef _FUNCTOR_H_
#define _FUNCTOR_H_


namespace NSFeature
{


/*******************************************************************************
*
*   Functor of Target Feature ID.
*
*   @param _Impl_T Type of functor implementation.
*   An implementation type must define enum FID and the functor with prototype:
*   MBOOL operator()(
*       FInfoIF*const pFInfo_Tgt, FID const fid, SID const sid
*   ) const
*
*******************************************************************************/
template <class _Impl_T>
class Functor_OneFID_TgtDo
{
private:
    enum
    {
        EFid = _Impl_T::FID, 
        EIsSceneDep = Fid2Type<EFid>::Info::isSceneDep
    };

    //  Scene-Indpe. Feature.
    MBOOL op(FeatureHalBridge& rFHBridge, Int2Type<false>) const
    {
        FInfoIF* pFInfo_Tgt = NULL;
        pFInfo_Tgt = rFHBridge.GetFInfo(EFid);
        return  m_Impl(pFInfo_Tgt, EFid);
    }

    //  Scene-Dpe. Feature.
    MBOOL op(FeatureHalBridge& rFHBridge, Int2Type<true>) const
    {
        FInfoIF* pFInfo_Tgt = NULL;
        //  Just loop supported scene modes.
        SID_T const* pSid = rFHBridge.SceneBegin();
        SID_T const* const pSEnd = rFHBridge.SceneEnd();
        SID_T sid = 0;
        for (; pSEnd != pSid; pSid++)
        {
            sid = *pSid;
            pFInfo_Tgt = rFHBridge.GetFInfo(EFid, sid);
            if  ( ! m_Impl(pFInfo_Tgt, EFid, sid) )
                return  MFALSE;
        }
        return  MTRUE;
    }

public:
    MBOOL operator()(FeatureHalBridge& rFHBridge) const
    {
        return  op(rFHBridge, Int2Type<EIsSceneDep>());
    }

    Functor_OneFID_TgtDo()
        : m_Impl()
    {}

    Functor_OneFID_TgtDo(_Impl_T _impl)
        : m_Impl(_impl)
    {}

private:
    _Impl_T     m_Impl;
};


/*******************************************************************************
*
*   Initialize Scene-Indep. Features.
*
*   @param _FIDBegin_SI the first scene-indep. feature id to begin initializing.
*   @param _FIDEnd_SI the end of scene-indep. feature id to initialize.
*   @param _GetFInfo_Src_T the functor type to get the FInfo of source features.
*
*******************************************************************************/
template <
    MUINT32 _FIDBegin_SI, 
    MUINT32 _FIDEnd_SI, 
    class   _GetFInfo_Src_T
>
MBOOL
InitFeatures_SI(
    FeatureHalBridge& rFHBridge, _GetFInfo_Src_T pfGetFInfo_Source
)
{
    enum {isValidRange=_FIDBegin_SI<=_FIDEnd_SI};
    STATIC_CHECK(isValidRange, fid_range_INVALID);
    enum {isSceneIndep=(FID_BEGIN_SI<=_FIDBegin_SI && _FIDEnd_SI<=FID_END_SI)};
    STATIC_CHECK(isSceneIndep, range_NOT_scene_indep);

    MBOOL fgRet = MFALSE;

    // target feature
    FInfoIF* pFInfo_Tgt = NULL;
    // source feature (misc/raw/yuv/custom)
    FInfoIF* pFInfo_Src = NULL;

    if  ( ! pfGetFInfo_Source )
        goto lbExit;

    //  Scene-Indep. Features.
    for (FID_T fid = _FIDBegin_SI; fid < _FIDEnd_SI; fid++)
    {
        //  Source feature.
        pFInfo_Src = pfGetFInfo_Source(fid);
        if  ( ! pFInfo_Src )
            continue;
        //  Target feature.
        pFInfo_Tgt = rFHBridge.GetFInfo(fid);
        if  ( ! pFInfo_Tgt )
            continue;

        //  Copy. (Target <- Source)
        if  ( ! pFInfo_Tgt->Copy(pFInfo_Src) )
        {
            MY_LOG("[Target <- Source] Fail to Copy: (fid)=(%d)\n", fid);
            goto lbExit;
        }
#if ENABLE_MY_LOG
        MY_LOG("-Init. (fid)=(%d)\n", fid);
#endif
    }

    fgRet = MTRUE;

lbExit:
    return  fgRet;
}


/*******************************************************************************
*
*   Initialize Scene-Dep. Features.
*
*   @param _FIDBegin_SD the first scene-dep. feature id to begin initializing.
*   @param _FIDEnd_SD the end of scene-dep. feature id to initialize.
*   @param _GetFInfo_Src_T the functor type to get the FInfo of source features.
*
*******************************************************************************/
template <
    MUINT32 _FIDBegin_SD, 
    MUINT32 _FIDEnd_SD, 
    class   _GetFInfo_Src_T
>
MBOOL
InitFeatures_SD(
    FeatureHalBridge& rFHBridge, _GetFInfo_Src_T pfGetFInfo_Source
)
{
    enum {isValidRange=_FIDBegin_SD<=_FIDEnd_SD};
    STATIC_CHECK(isValidRange, fid_range_INVALID);
    enum {isSceneDep=(FID_BEGIN_SD<=_FIDBegin_SD && _FIDEnd_SD<=FID_END_SD)};
    STATIC_CHECK(isSceneDep, range_NOT_scene_dep);

    MBOOL fgRet = MFALSE;

    // target feature
    FInfoIF* pFInfo_Tgt = NULL;
    // source feature (misc/raw/yuv/custom)
    FInfoIF* pFInfo_Src = NULL;

    if  ( ! pfGetFInfo_Source )
        goto lbExit;

    //  Scene-Dep. Features.
    for (FID_T fid = _FIDBegin_SD; fid < _FIDEnd_SD; fid++)
    {
        //  Just loop supported scene modes.
        SID_T const*        pSid = rFHBridge.SceneBegin();
        SID_T const* const pSEnd = rFHBridge.SceneEnd();
        for (; pSEnd != pSid; pSid++)
        {
            SID_T const sid = *pSid;

            //  Source feature.
            pFInfo_Src = pfGetFInfo_Source(fid, sid);
            if  ( ! pFInfo_Src )
                continue;
            //  Target feature.
            pFInfo_Tgt = rFHBridge.GetFInfo(fid, sid);
            if  ( ! pFInfo_Tgt )
                continue;

            //  Copy. (Target <- Source)
            if  ( ! pFInfo_Tgt->Copy(pFInfo_Src) )
            {
                MY_LOG("[Target <- Source] Fail to Copy: "
                       "(fid,sid)=(%d,%d)\n", fid, sid);
                goto lbExit;
            }
        }
#if ENABLE_MY_LOG
        MY_LOG("-Init. (fid)=(%d)\n", fid);
#endif
    }

    fgRet = MTRUE;

lbExit:
    return  fgRet;
}


};  //  namespace NSFeature

#endif // _FUNCTOR_H_

