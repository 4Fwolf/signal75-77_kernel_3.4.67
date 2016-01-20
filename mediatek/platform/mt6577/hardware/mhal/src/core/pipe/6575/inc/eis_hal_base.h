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

#ifndef _EIS_HAL_BASE_H_
#define _EIS_HAL_BASE_H_

#include "eis_reg.h"

/*******************************************************************************
*
********************************************************************************/
typedef struct _eis_config_t {

    MINT32 i4IspSize_X;
    MINT32 i4IspSize_Y;    
    MINT32 i4CrzSize_X;
    MINT32 i4CrzSize_Y;
    MINT32 i4TarSize_X;
    MINT32 i4TarSize_Y;    
    MBOOL  bForce_ISP_Path;
} eis_config_t;

typedef struct _eis_factor_t {

   
           
} eis_factor_t;


/*******************************************************************************
*
********************************************************************************/
class EisHalBase {
public:
    static EisHalBase* createInstance();
    virtual MVOID      destroyInstance() = 0;
    
protected:
    virtual ~EisHalBase() {};

public:
    virtual MVOID  enableEIS(eis_config_t a_sEisConfig, eis_factor_t &a_sEisFactor) = 0;    
    virtual MVOID  disableEIS() = 0;    

    // lsb 2 bit for quarter sub-pixel precision    
    // a_i4CMV_X = 15 (00001111), means 3.75 (decimal)
    // a_i4CMV_X =  5 (00000101), means 1.25 (decimal)
    // a_i4CMV_X = 22 (00010110), means 5.5 (decimal) 
    // a_i4CMV_X = 22 (00011100), means 7.0 (decimal)    
    virtual MINT32 doEIS(MINT32 &a_i4CMV_X, MINT32 &a_i4CMV_Y) = 0;

    // must run doEIS(CMV_X, XMV_Y) before calling following function
    virtual MVOID getHWGMV(MINT32 &a_i4GMV_X, MINT32 &a_i4GMV_Y) = 0;
    virtual MVOID getSWGMV(MINT32 &a_i4GMV_X, MINT32 &a_i4GMV_Y) = 0;
    virtual MVOID getEISStat(eis_stat_t &a_sEIS_Stat) = 0;

    
};



class EisNone:  public EisHalBase
{
public:
    EisNone() {}; 
private:
    virtual MVOID      destroyInstance()  { delete this; };	
    virtual ~EisNone() {}; 

public: 
    virtual MVOID  enableEIS(eis_config_t a_sEisConfig, eis_factor_t &a_sEisFactor) {};    
    virtual MVOID  disableEIS() {};    

    virtual MINT32 doEIS(MINT32 &a_i4CMV_X, MINT32 &a_i4CMV_Y) {a_i4CMV_X =0; a_i4CMV_Y = 0; return 0;};

    // must run doEIS(CMV_X, XMV_Y) before calling following function
    virtual MVOID getHWGMV(MINT32 &a_i4GMV_X, MINT32 &a_i4GMV_Y) {a_i4GMV_X =0; a_i4GMV_Y =0; };
    virtual MVOID getSWGMV(MINT32 &a_i4GMV_X, MINT32 &a_i4GMV_Y) {a_i4GMV_X = 0; a_i4GMV_Y = 0;  };
    virtual MVOID getEISStat(eis_stat_t &a_sEIS_Stat) {};	
}; 


#endif

