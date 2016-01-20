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
#ifndef _FEATURE_HAL_BRIDGE_H_
#define _FEATURE_HAL_BRIDGE_H_


namespace NSFeature
{


/*******************************************************************************
*
*******************************************************************************/
class ControlBase;


/*******************************************************************************
*
*******************************************************************************/
class FeatureHalBridge : public FeatureHal
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    
    virtual MBOOL setSceneMode(SID_T const sid);
    virtual MBOOL queryFeatures(
        MVOID* const  pOutBuf, 
        MUINT32 const u4OutBufNum, 
        MUINT32&      ru4NumReturned
    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor/Init/Deinit.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    FeatureHalBridge(FeatureHalBridge const&);
    //  Copy-assignment operator is disallowed.
    FeatureHalBridge& operator=(FeatureHalBridge const&);

protected:  ////    
    FeatureHalBridge(ControlBase*const pCtrl);
    MBOOL   Init();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Feature Info.
    virtual FInfoIF*        GetFInfo(FID_T const fid) = 0;
    virtual FInfoIF*        GetFInfo(FID_T const fid, SID_T const sid) = 0;

public:     ////    Scene Info.
    inline  FInfoIF*        GetFInfo_Scene() { return GetFInfo(FID_SCENE_MODE); }
    virtual SID_T const*    SceneBegin() = 0;
    virtual SID_T const*    SceneEnd() = 0;
    virtual MBOOL           IsSceneSupport(SID_T const sid) const = 0;
    virtual void            UpdateScenesInfo() = 0;
    void                    DumpScenesInfo();

public:     ////    Camera Info.
    virtual ECamRole_T      GetCamRole()   const = 0;

protected:  ////    Data Members.
    ControlBase&    m_rCtrl;    //  Reference to control context.
};


};  //  namespace NSFeature


#endif // _FEATURE_HAL_BRIDGE_H_

