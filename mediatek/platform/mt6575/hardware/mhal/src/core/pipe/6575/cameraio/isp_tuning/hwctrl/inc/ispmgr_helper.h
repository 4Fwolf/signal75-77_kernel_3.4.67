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
#ifndef _ISP_MGR_HELPER_H_
#define _ISP_MGR_HELPER_H_


namespace NSIspTuning
{


/*******************************************************************************
* 
*******************************************************************************/
template <class T>
struct ISPType2Type;

#define BIND_ISP_TYPES(FROM_T, TO_T)\
    template <> struct ISPType2Type<FROM_T> { typedef TO_T type; }


/*******************************************************************************
* 
*******************************************************************************/
BIND_ISP_TYPES( ISP_NVRAM_CCLIP_T,         ISP_MGR_CCLIP_T );
BIND_ISP_TYPES( ISP_NVRAM_GAIN_CTRL_T,   ISP_MGR_GAIN_CTRL_T );
BIND_ISP_TYPES( ISP_NVRAM_GAMMA_ECO_T,     ISP_MGR_EE_T);
BIND_ISP_TYPES( ISP_NVRAM_RGB2YCC_YOFST_T, ISP_MGR_RGB2YCC_YOFST_T);
BIND_ISP_TYPES( ISP_NVRAM_SHADING_T,    ISP_MGR_SHADING_T);
BIND_ISP_TYPES( ISP_NVRAM_OB_T,         ISP_MGR_OB_T    );
BIND_ISP_TYPES( ISP_NVRAM_DEMOSAIC_T,   ISP_MGR_DM_T    );
BIND_ISP_TYPES( ISP_NVRAM_DP_T,         ISP_MGR_NR1_T   );
BIND_ISP_TYPES( ISP_NVRAM_NR1_T,        ISP_MGR_NR1_T   );
BIND_ISP_TYPES( ISP_NVRAM_NR2_T,        ISP_MGR_NR2_T   );
BIND_ISP_TYPES( ISP_NVRAM_EE_T,         ISP_MGR_EE_T    );
BIND_ISP_TYPES( ISP_NVRAM_EDGE_GAMMA_T, ISP_MGR_EE_T    );
BIND_ISP_TYPES( ISP_NVRAM_SATURATION_T, ISP_MGR_YCCGO_T );
BIND_ISP_TYPES( ISP_NVRAM_CONTRAST_T,   ISP_MGR_YCCGO_T );
BIND_ISP_TYPES( ISP_NVRAM_HUE_T,        ISP_MGR_YCCGO_T );
BIND_ISP_TYPES( ISP_NVRAM_CCM_T,        ISP_MGR_CCM_T   );
BIND_ISP_TYPES( ISP_NVRAM_GAMMA_T,      ISP_MGR_GAMMA_T );
BIND_ISP_TYPES( ISP_NVRAM_PCA_T,        ISP_MGR_PCA_T   );
/*******************************************************************************
* 
*******************************************************************************/
BIND_ISP_TYPES( ISP_EFFECT_YCCGO_T,     ISP_MGR_YCCGO_T );
BIND_ISP_TYPES( ISP_EFFECT_CCM_T,       ISP_MGR_CCM_T   );


/*******************************************************************************
* 
*******************************************************************************/
template <class ISP_XXX_T>
MVOID
getIspHWBuf(ISP_XXX_T& rParam)
{
    typedef typename ISPType2Type<ISP_XXX_T>::type ISP_MGR_T;
    ISP_MGR_T::getInstance().get(rParam);
}


template <class ISP_XXX_T>
MVOID
putIspHWBuf(ISP_XXX_T const& rParam)
{
    typedef typename ISPType2Type<ISP_XXX_T>::type ISP_MGR_T;
    ISP_MGR_T::getInstance().put(rParam);
}


#define prepareIspHWBuf putIspHWBuf


//  Specialization: EFFECT EDGE -> REG: DM + EDGE + EE
template <>
MVOID
getIspHWBuf<ISP_EFFECT_EDGE_T>(ISP_EFFECT_EDGE_T& rParam);


//  Specialization: EFFECT EDGE -> REG: DM + EDGE + EE
template <>
MVOID
putIspHWBuf<ISP_EFFECT_EDGE_T>(ISP_EFFECT_EDGE_T const& rParam);


/*******************************************************************************
* 
*******************************************************************************/
inline
MVOID
prepareIspHWBuf_enableShading(MBOOL const fgEnable)
{
    ISP_MGR_SHADING_T::getInstance().setEnableShading(fgEnable);
}


inline
MVOID
prepareIspHWBuf_enableNR1_DP(MBOOL const fgEnable)
{
    ISP_MGR_NR1_T::getInstance().setEnableDP(fgEnable);
}


inline
MVOID
prepareIspHWBuf_enableNR1_CT(MBOOL const fgEnable)
{
    ISP_MGR_NR1_T::getInstance().setEnableCT(fgEnable);
}


inline
MVOID
prepareIspHWBuf_enableNR1_NR(MBOOL const fgEnable)
{
    ISP_MGR_NR1_T::getInstance().setEnableNR(fgEnable);
}


inline
MVOID
prepareIspHWBuf_enableGamma(MBOOL const fgEnable)
{
    ISP_MGR_EE_T::getInstance().setEnableGamma(fgEnable);
}


/*******************************************************************************
* 
*******************************************************************************/


};  //  NSIspTuning
#endif // _ISP_MGR_HELPER_H_

