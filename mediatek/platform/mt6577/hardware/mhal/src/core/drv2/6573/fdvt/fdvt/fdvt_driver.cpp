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
 /*
** $Log: fdvt_hal.cpp $
 *
*/
#define LOG_TAG "mHalFDVT"
#include <utils/Errors.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/Log.h>

#include "fdvt_drv.h"

/*******************************************************************************
*
********************************************************************************/
//-------------------------------------------//
//  Global face detection related parameter  //
//-------------------------------------------//
#define FDVT_DEV_NAME    "/dev/mt6573-fdvt"
static MINT32 fdFDVT = -1;

  
/*******************************************************************************
*                                            
********************************************************************************/
                                          
MINT32 
halFDVT_OpenDriver()
{
    MINT32 err = S_Detection_OK;
     
    //open isp driver
    if (fdFDVT == -1) {
        fdFDVT = open(FDVT_DEV_NAME, O_RDWR);
        if (fdFDVT < 0) {
            LOGD("error opening %s: %s", FDVT_DEV_NAME, strerror(errno));
            return E_Detection_Driver_Fail;
        }
    }
       
    //halFDVT_PARA_SET(&mFDVTReg);   
    //ioctl(fdFDVT,MT6573FDVT_INIT_SETPARA_CMD,&mFDVTReg);
    ioctl(fdFDVT,MT6573FDVT_INIT_SETPARA_CMD);
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
halFDVT_StartHW(void* srcbuf)
{
    int ret = 0;

    //MHAL_LOG("FDVTRS START in ");      
    ret=ioctl(fdFDVT,MT6573FDVTIOC_STARTFD_CMD);
    //MHAL_LOG("FDVTRS START out ");     
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT_Uninit(
)
{    
    //MHAL_LOG("halFDVT_Uninit in "); 
    if (fdFDVT > 0) {
        close(fdFDVT);
        fdFDVT = -1;
    }
    //MHAL_LOG("halFDVT_Uninit out ");
    return S_Detection_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT_Wait_IRQ(
)
{
    MUINT32 irqSts;
    int ret = 0;
    
    ret=ioctl(fdFDVT,MT6573FDVTIOC_G_WAITIRQ,&irqSts);
        
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT_PARA_SET(MUINT32* Adr,MUINT32* value,int num,FDVT_OPERATION_MODE_ENUM fd_state)
{   
    MINT32 ret = S_Detection_OK;        
    MT6573FDVTRegIO mFDVTReg;
    mFDVTReg.pAddr=Adr;
    mFDVTReg.pData=value;
    mFDVTReg.u4Count=num;  
    if (fd_state == FDVT_GFD_MODE)
       ret=ioctl(fdFDVT,MT6573FDVTIOC_T_SET_FDCONF_CMD,&mFDVTReg); 
    else if (fd_state == FDVT_SD_MODE)
       ret=ioctl(fdFDVT,MT6573FDVTIOC_T_SET_SDCONF_CMD,&mFDVTReg); 
    else
       ret=ioctl(fdFDVT,MT6573FDVTIOC_T_SET_FDCONF_CMD,&mFDVTReg); 
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDGetFaceResult(MUINT32* Adr,MUINT32* value,int num,MUINT32 &result)
{
    MINT32 ret = S_Detection_OK; 
    //MHAL_ASSERT(bufSize >= sizeof(fdvt_result_struct) * 16, "bufSize is too small");
    MT6573FDVTRegIO mFDVTReg;
    mFDVTReg.pAddr=Adr;
    mFDVTReg.pData=value;
    mFDVTReg.u4Count=num;
    ret=ioctl(fdFDVT,MT6573FDVTIOC_G_READ_FDREG_CMD,&mFDVTReg); 
    result=mFDVTReg.pData[0];
    return ret;
}

/*******************************************************************************
*
********************************************************************************/

MINT32
halSetDetectPara(MUINT8 Para)
{
    MINT32 err = S_Detection_OK;
    
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halSDGetSmileResult(
    MUINT8 *pbuf
)
{
    MINT32 SmileCnt = 0;
       
    //SmileCnt = m_pMTKFDObj->FDVTGetSDResult((MUINT32) pbuf);
    
    return SmileCnt;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halDumpReg(    
)
{
    ioctl(fdFDVT,MT6573FDVTIOC_T_DUMPREG);   
    return 0;
}

