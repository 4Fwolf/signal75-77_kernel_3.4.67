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
#ifndef _EIS_DRV_BASE_H_
#define _EIS_DRV_BASE_H_

#include "eis_reg.h"

/*******************************************************************************
*
********************************************************************************/
class EisDrvBase {
public:
    static EisDrvBase* createInstance();
    virtual void   destroyInstance() = 0;

protected:
    virtual ~EisDrvBase() {};
    
public:
    virtual void enableEIS(bool a_bEnable) = 0;    
    virtual bool isEISenable() = 0;    
    virtual void setSrcSel(bool a_bSrcSel) = 0;
    virtual void setSRAM_PD(bool a_bSRAM_PD) = 0;    
    virtual void setSRAM_RB(bool a_bSRAM_RB) = 0;
    virtual void setKneeClip(MINT32 a_i4Knee, MINT32 a_i4Clip) = 0;
    virtual void setHRP(MINT32 a_i4Num) = 0;
    virtual void setVFilter(MINT32 a_i4Gain, MINT32 a_i4DS) = 0;
    virtual void setHFilter(MINT32 a_i4Gain, MINT32 a_i4DS, MINT32 a_i4GainFIR, MINT32 a_i4FIR) = 0;
    virtual void setLMV_TH(MINT32 a_i4H_Center, MINT32 a_i4H_Surr, MINT32 a_i4V_Center, MINT32 a_i4V_Surr) = 0;       
    virtual void setFLoffset(MINT32 a_i4FL_H, MINT32 a_i4FL_V) = 0;
    virtual void setFLoffsetMax(MINT32 a_i4FL_Max_H, MINT32 a_i4FL_Max_V) = 0;    
    virtual void setMB_H(MINT32 a_i4MBoffset_H, MINT32 a_i4MBinterval_H) = 0;
    virtual void setMB_V(MINT32 a_i4MBoffset_V, MINT32 a_i4MBinterval_V) = 0;    
    virtual void setIRQ_WR_CLR(bool a_bIRQ_WR_CLR) = 0;
    virtual void setIRQ_EN(bool a_bIRQ_EN) = 0;
    virtual void set0024H(bool a_bPipe_Mode, MINT32 a_i4Img_Width, MINT32 a_i4Img_Height) = 0;
    virtual void resetEIS(bool a_bReset) = 0;
    virtual void setStatAddr(MINT32 a_PA, MINT32 a_VA) = 0;       
    virtual void getFLoffsetMax(MINT32 &a_i4FL_Max_H, MINT32 &a_i4FL_Max_V) = 0;    
    virtual bool getIRQ_State() = 0;
    virtual MINT32 getHW_GMV(MINT32 &a_i4GMV_X, MINT32 &a_i4GMV_Y) = 0;    
    virtual MINT32 getOriStat(eis_ori_stat_t &a_EIS_Ori_Stat) = 0;
    virtual MINT32 getStat(eis_stat_t &a_EIS_Stat) = 0;
    virtual void getDIVinfo(MINT32 &a_i4DIV_H, MINT32 &a_i4DIV_V) = 0;


    // for auto config
    virtual MINT32 AutoConfig(MINT32 a_i4Path, MINT32 a_i4Width, MINT32 a_i4Height) = 0;    
    virtual MINT32 dumpReg() = 0;
    
};

#endif // _EIS_DRV_BASE_H_

