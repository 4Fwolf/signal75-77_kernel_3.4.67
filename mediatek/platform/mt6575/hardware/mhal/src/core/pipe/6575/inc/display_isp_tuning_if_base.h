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
#ifndef _DISPLAY_ISP_TUNING_IF_BASE_H_
#define _DISPLAY_ISP_TUNING_IF_BASE_H_

namespace NSDisplayIspTuning
{

typedef struct
{
    MUINT8  uUpScaleCoeff;  //  [5 bits; 1~19] Up sample coeff. choose > 12 may get undesirable result, '8' is recommended.
    MUINT8  uDnScaleCoeff;  //  [5 bits; 1~19] Down sample coeff. '15' is recommended.
    MUINT8  uEEHCoeff;      //  [4 bits] The strength for horizontal edge.
    MUINT8  uEEVCoeff;      //  [4 bits] The strength for vertial edge.
} PRZ_T;

/*******************************************************************************
*
*******************************************************************************/
class DisplayIspTuningIFBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Ctor/Dtor.
    DisplayIspTuningIFBase() {}
    virtual ~DisplayIspTuningIFBase() {}

private:
    DisplayIspTuningIFBase(const DisplayIspTuningIFBase&);
    DisplayIspTuningIFBase& operator=(const DisplayIspTuningIFBase&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static DisplayIspTuningIFBase*    createInstance();
    virtual void    destroyInstance() = 0;
    virtual MINT32  init() = 0;
    virtual MINT32  deinit() = 0;
    virtual const PRZ_T& getPRZParam() = 0;
    virtual MINT32 loadISPParam() = 0;
    virtual MINT32 unloadISPParam() = 0;
    virtual MBOOL checkISPParamEffectiveness() = 0; //  TRUE: at least one of NR2/PCA/YCCGO is enabled
                                                    // FALSE: NR2/PCA/YCCGO are all disabled
    virtual MINT32 setISPParamIndex(MUINT32 u4PcaSkinLutIdx, MUINT32 u4PcaGrassLutIdx, MUINT32 u4PcaSkyLutIdx, MUINT32 u4YCCGOIdx, MUINT32 u4PRZIdx) = 0;
    virtual MINT32 getISPParamIndex(MUINT32& u4PcaSkinLutIdx, MUINT32& u4PcaGrassLutIdx, MUINT32& u4PcaSkyLutIdx, MUINT32& u4YCCGOIdx, MUINT32& u4PRZIdx) = 0;
};


};  //  namespace NSDisplayIsp
#endif // _DISPLAY_ISP_TUNING_IF_BASE_H_

