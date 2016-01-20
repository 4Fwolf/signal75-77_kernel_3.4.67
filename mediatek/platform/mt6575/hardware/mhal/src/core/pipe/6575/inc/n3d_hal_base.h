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

#ifndef _N3D_HAL_BASE_H_
#define _N3D_HAL_BASE_H_

typedef enum
{
   	SCENARIO_IMAGE_PREVIEW,
    SCENARIO_IMAGE_CAPTURE,
    SCENARIO_IMAGE_PLAYBACK,
    SCENARIO_VIDEO_RECORD,
    SCENARIO_VIDEO_PLAYBACK 
     
} SCENARIO_ENUM;

typedef enum
{
	SOURCE_FORMAT_RGBA,
	SOURCE_FORMAT_YUYV
	
}SOURCE_FORMAT_ENUM;

typedef struct IMG_INFO_S
{
    MUINT32 width;
    MUINT32 height;

} IMG_INFO_T; 

typedef struct IN_DATA_N3D_S
{
    IMG_INFO_T originalImg;
    IMG_INFO_T targetImg;
    SCENARIO_ENUM eScenario;
    SOURCE_FORMAT_ENUM eFormat;
    MUINT32 u4Stride;
    
} IN_DATA_N3D_T;

typedef struct RUN_DATA_N3D_S
{
    MUINT32 u4BufAddrL;
    MUINT32 u4BufAddrR;

} RUN_DATA_N3D_T;

typedef struct OUT_DATA_N3D_S
{
    IMG_INFO_T offsetL;
    IMG_INFO_T offsetR;
 
} OUT_DATA_N3D_T;


/*******************************************************************************
*
********************************************************************************/
class N3DHalBase {
public:
    static N3DHalBase* createInstance();
    virtual ~N3DHalBase() {};
    virtual void      destroyInstance()  { delete this; };    

public:
    virtual MINT32 N3DInit(IN_DATA_N3D_T InData) = 0;
    virtual MINT32 N3DRun(RUN_DATA_N3D_T RunData, OUT_DATA_N3D_T &OutData) = 0;
    virtual MINT32 N3DReset() = 0;
};



class N3DNone:  public N3DHalBase
{
public:
    N3DNone() {}; 
    virtual ~N3DNone() {}; 

private:
    virtual void      destroyInstance()  { delete this; };	

public: 
    virtual MINT32 N3DInit(IN_DATA_N3D_T InData) {return 0;};
    virtual MINT32 N3DRun(RUN_DATA_N3D_T RunData, OUT_DATA_N3D_T &OutData) {return 0;};
    virtual MINT32 N3DReset() {return 0;};
}; 


#endif

