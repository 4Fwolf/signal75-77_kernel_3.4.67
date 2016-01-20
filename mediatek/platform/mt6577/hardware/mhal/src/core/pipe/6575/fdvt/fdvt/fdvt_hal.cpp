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
#include "MediaHal.h"
#include <mhal/inc/camera/faces.h>
#include "MediaLog.h"
#include "MediaAssert.h"
#include "fdvt_hal.h"

#include "MTKDetection.h"

//Please fix me Sava
#if defined(MTK_M4U_SUPPORT)
//#undef MTK_M4U_SUPPORT
#endif

#if defined(MTK_M4U_SUPPORT)
#include "m4u_lib.h"
#endif 

/*******************************************************************************
*
********************************************************************************/
//-------------------------------------------//
//  Global face detection related parameter  //
//-------------------------------------------//
#define EnableSDFlag                m_DetectPara & 0x01
//
static halFDBase *pHalFD = NULL;

#if defined (MTK_M4U_SUPPORT)
static MTKM4UDrv * pM4UDrv = NULL;
#endif

MUINT32 g_Lcm_rotate = 0;
MUINT32 g_Sensor_rotate = 0;
MUINT32 g_CameraTYPE = 0;

/*******************************************************************************
*
********************************************************************************/
halFDBase*
halFDVT::
getInstance()
{
    MHAL_LOG("[halFDVT] getInstance \n");
    if (pHalFD == NULL) {
        pHalFD = new halFDVT();
    }
    return pHalFD;
}

/*******************************************************************************
*
********************************************************************************/
void   
halFDVT::
destroyInstance() 
{
    if (pHalFD) {
        delete pHalFD;
    }
    pHalFD = NULL;
}

/*******************************************************************************
*                                            
********************************************************************************/
halFDVT::halFDVT()
{
    m_pMTKFDVTObj = NULL;

    m_FDW = 0; 
    m_FDH = 0; 
    m_DispW = 0; 
    m_DispH = 0; 
    m_DispX = 0; 
    m_DispY = 0;
    m_DispRoate = 0;
    m_DetectPara = 0;  
    
    m_pMTKFDVTObj = MTKDetection::createInstance(DRV_FD_OBJ_HW);
    //m_pMTKFDVTObj = new AppFDVT();    
    MHAL_ASSERT(m_pMTKFDVTObj != NULL, "m_pMTKFDVTObj is NULL");   
}


halFDVT::~halFDVT()
{
    m_FDW = 0; 
    m_FDH = 0; 
    m_DispW = 0; 
    m_DispH = 0; 
    m_DispX = 0; 
    m_DispY = 0;
    m_DispRoate = 0;
    m_DetectPara = 0;   
    
    if (m_pMTKFDVTObj) {
        m_pMTKFDVTObj->destroyInstance();
    }
    m_pMTKFDVTObj = NULL;      
}


MINT32
halFDVT::halFDInit(
    MUINT32 fdW,
    MUINT32 fdH
)
{
    MINT32 err = MHAL_NO_ERROR;
    MUINT32* Cdata=NULL;
    MHAL_LOG("[mHalFDInit] Start \n");
    m_FDW = fdW;
    m_FDH = fdH;
    m_DispW = 0;
    m_DispH = 0;
    m_DispX = 0;
    m_DispY = 0;
    m_DispRoate = 0;    
    
    #if defined(MTK_M4U_SUPPORT)
    pM4UDrv = new MTKM4UDrv();
    if (pM4UDrv == NULL)  {
        MHAL_LOG("Init FDVT Null M4U driver \n"); 
        return -1;
    }
    else
    	  MHAL_LOG("Init FDVT M4U driver ok\n");
    
    pM4UDrv->m4u_reset_mva_release_tlb(M4U_CLNTMOD_FD_INPUT);
    
    memset (&mIspReadPort, 0,  sizeof(FDVTM4UInfo)); 
    //memset (&mIspWritePort, 0,  sizeof(FDVTM4UInfo)); 
    #endif 
      
    MHAL_ASSERT(m_pMTKFDVTObj != NULL, "m_pMTKFDVTObj is NULL");
    m_pMTKFDVTObj->FDVTInit(Cdata);
    return err;
}
//#define SaveFile
#ifdef SaveFile
int framecunt=0;
#endif
/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT::halFDDo(
    MUINT32 imgBufAddr
)
{  
    //FDVT_OPERATION_MODE_ENUM fdvt_state = FDVT_IDLE_MODE;
    MUINT32 m_imgBufAddr=imgBufAddr;
    #if SaveFile
    framecunt++;    
    if(framecunt>200)
    {
        MHAL_LOG("  Sava Image \n"); 
        UCHAR szFileName[100];
                sprintf(szFileName, "%s//%04d_%04d.raw","//sdcard", 160, 120);    
                
                printf(" Save File Name:%s\n", szFileName);
                printf(" Save RAW Image \n");
                          
                FILE *pRawFp = fopen(szFileName, "wb");
                
                if (NULL == pRawFp )
                {
                    printf("Can't open file to save RAW Image\n"); 
                    while(1);
                }
                
                MINT32 i4WriteCnt = fwrite((void *)m_imgBufAddr,2, (160 * 120 * 1),pRawFp);
                fflush(pRawFp); 
                fclose(pRawFp);
                printf(" Save RAW Image Done\n");
                while(1);
    }
    #endif
    
 #if defined (MTK_M4U_SUPPORT)   
//    MHAL_LOG("[halFDDo] imgBufAddr B 0x%x \n",imgBufAddr);
    m_imgBufAddr=vir2m4u(imgBufAddr);
//    MHAL_LOG("[halFDDo] imgBufAddr A 0x%x \n",m_imgBufAddr);
 #endif
    
    m_pMTKFDVTObj->FDVTMain(m_imgBufAddr, imgBufAddr, FDVT_GFD_MODE);
    #ifdef SmileDetect  
    if(EnableSDFlag)
    {
        m_pMTKFDVTObj->FDVTMain((MUINT32)m_imgBufAddr, imgBufAddr, FDVT_SD_MODE); 
    }
    #endif
   
    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT::halFDUninit(
)
{  
    MHAL_LOG("[halFDUninit] IN \n");
    m_pMTKFDVTObj->FDVTReset();
    #if defined(MTK_M4U_SUPPORT)
        //
        if (mIspReadPort.useM4U) {
            MHAL_LOG("[stop] free M4U ISP read port memory \n"); 
            freeM4UMemory(mIspReadPort.virtAddr, mIspReadPort.size); 
            mIspReadPort.useM4U = 0; 
        }
        if (pM4UDrv) {
            delete pM4UDrv;
            pM4UDrv = NULL; 
            memset (&mIspReadPort, 0,  sizeof(FDVTM4UInfo)); 
            //memset (&mIspWritePort, 0,  sizeof(FDVTM4UInfo)); 
        }     
        
        /*
        if (mIspWritePort.useM4U) {
            MHAL_LOG("[stop] free M4U ISP write port memory \n"); 
            freeM4UMemory(mIspWritePort.virtAddr, mIspWritePort.size); 
            mIspWritePort.useM4U = 0; 
        }*/

    #endif

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT::halFDDrawFaceRect(
    MUINT8 *pbuf
)
{
    MINT32 err = MHAL_NO_ERROR;
    
    m_pMTKFDVTObj->FDVTDrawFaceRect((MUINT32) pbuf,m_DispW,m_DispH,m_DispX,m_DispY,m_DispRoate);
    
    return err;
}

MINT32
halFDVT::halSDDrawFaceRect(
    MUINT8 *pbuf
)
{
    MINT32 err = MHAL_NO_ERROR;
    
    m_pMTKFDVTObj->FDVTSDDrawFaceRect((MUINT32) pbuf,m_DispW,m_DispH,m_DispX,m_DispY,m_DispRoate);
    
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT::halFDGetFaceInfo(
    mtk_face_info_m *fd_info_result
)
{
	m_pMTKFDVTObj->FDVTGetFDInfo((MUINT32) fd_info_result);
	MHAL_LOG("[halFDDo] ROP: %d,  RIP:%d, \n",fd_info_result[0].rop_dir, fd_info_result[0].rip_dir);
	
	return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT::halFDGetFaceResult(
    camera_face_metadata_m *fd_result,
    MUINT32 DrawMode
)
{
    MINT32 faceCnt = 0;
    MUINT8 pbuf[1024];   
    MUINT8 i;
    
    //MHAL_ASSERT(bufSize >= sizeof(fdvt_result_struct) * 16, "bufSize is too small");
    faceCnt=m_pMTKFDVTObj->FDVTGetResult((MUINT32) pbuf);
    
    m_pMTKFDVTObj->FDVTGetICSResult((MUINT32) fd_result, (MUINT32) pbuf, m_DispW, m_DispH, g_Lcm_rotate, g_Sensor_rotate, g_CameraTYPE, DrawMode);
    
    //*******************************************************************************************************//
    //For ICS FD transformation
    //*******************************************************************************************************//
    
     MHAL_LOG("[halFDDo] Face Number: %d, X0:%d, Y0:%d, X1:%d, Y1:%d, \n",fd_result->number_of_faces, fd_result->faces[0].rect[0],
                                                                                                                                                  fd_result->faces[0].rect[1], fd_result->faces[0].rect[2], fd_result->faces[0].rect[3]);

/*
     MUINT8 pbuf1[1024];
     m_pMTKFDVTObj->FDVTGetFDInfo((MUINT32) pbuf1);
     
    mtk_face_info_m*  pbuf2 =( mtk_face_info_m*) pbuf1;
     
     MHAL_LOG("[halFDDo] ROP: %d,  RIP: %d, \n",pbuf2[0].rop_dir, pbuf2[0].rip_dir);
*/     
     
/*    
    result* fd_result_drv = (result *) pbuf;
    fd_result->number_of_faces = fd_result_drv[0].face_num;
    
    MHAL_LOG("[halFDDo] Face Number: %d, X0:%d, Y0:%d, X1:%d, Y1:%d  \n",fd_result->number_of_faces, fd_result_drv[0].x0,
                                                                                                                                                        fd_result_drv[0].y0, fd_result_drv[0].x1, fd_result_drv[0].y1);
    
    for(i=0;i<MAX_FACE_NUM;i++)
    {
        fd_result->faces[i].rect[0] = fd_result_drv[i].x0;
        fd_result->faces[i].rect[1] = fd_result_drv[i].y0;
        fd_result->faces[i].rect[2] = fd_result_drv[i].x1;
        fd_result->faces[i].rect[3] = fd_result_drv[i].y1;
        
        if(i<fd_result->number_of_faces)
        {
            if(fd_result_drv[i].af_face_indicator!=0)
                 fd_result->faces[i].score = 100;
            else
         	       fd_result->faces[i].score = 90;
        }
        else
        	 fd_result->faces[i].score = 0;	
        
        fd_result->faces[i].id = 0; 
        fd_result->faces[i].left_eye[0] = -2000;
        fd_result->faces[i].left_eye[1] = -2000;
        fd_result->faces[i].right_eye[0] = -2000;
        fd_result->faces[i].right_eye[1] = -2000;
        fd_result->faces[i].mouth[0] = -2000;
        fd_result->faces[i].mouth[1] = -2000;
    }
*/    
    

    //*******************************************************************************************************//
    
    
    return faceCnt;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT::mHalFDSetDispInfo(
    MUINT32 x,
    MUINT32 y,
    MUINT32 w,
    MUINT32 h,
    MUINT32 Lcm_rotate,
    MUINT32 Sensor_rotate,
    MINT32 CameraTYPE
)
{
    MINT32 err = MHAL_NO_ERROR;

    // x,y is offset from left-top corner
    // w,h is preview frame width and height seen on LCD
    m_DispX = x;
    m_DispY = y;
    m_DispW = w;
    m_DispH = h;
    m_DispRoate = (Sensor_rotate << 5) | (Lcm_rotate <<2 ) | (CameraTYPE);
    
    g_Lcm_rotate = Lcm_rotate;
    g_Sensor_rotate = Sensor_rotate;
    g_CameraTYPE = CameraTYPE;
    
    
    //LOGE("[FDdraw_SetDisp] Lcm_rotate %d Sensor_rotate %d m_DispRoate %d \n",Lcm_rotate,Sensor_rotate,m_DispRoate); 

    return err;
}


/*******************************************************************************
*
********************************************************************************/

MINT32
halFDVT::halSetDetectPara(MUINT8 Para)
{
    MINT32 err = MHAL_NO_ERROR;
    
    m_DetectPara = Para;
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFDVT::halSDGetSmileResult()
{
    MINT32 SmileCnt = 0;
    MUINT8 pbuf1[8];
       
    SmileCnt = m_pMTKFDVTObj->FDVTGetSDResult((MUINT32) pbuf1);
    
    return SmileCnt;
}

/*******************************************************************************
*
********************************************************************************/
#if defined (MTK_M4U_SUPPORT)

MINT32 halFDVT::halFDM4URegister(MUINT8 *pbuf,MUINT32 BufSize,MUINT8 BufCunt)
{
	  mIspReadPort.useM4U = 1; 
    mIspReadPort.virtAddr = (MUINT32)pbuf; 
    mIspReadPort.size = BufSize*BufCunt; 
    allocM4UMemory(mIspReadPort.virtAddr, mIspReadPort.size, &mIspReadPort.M4UVa); 
    m_RegisterBuff=(MUINT32)pbuf;
    m_BuffCount=BufCunt;
    MHAL_LOG("[halFDM4URegister] m_RegisterBuff 0x%x virtAddr = 0x%x, mva = 0x%x, size %d m_BuffCount %d \n",m_RegisterBuff,(MUINT32)pbuf,(MUINT32)mIspReadPort.M4UVa,BufSize,m_BuffCount); 
    return 0;
}

MINT32 halFDVT::allocM4UMemory(MUINT32 virtAddr, MUINT32 size, MUINT32 *m4uVa)
{
    if (pM4UDrv == NULL)  {
        MHAL_LOG("Null M4U driver \n"); 
        return -1;
    }
    MINT32 ret = 0;  

    MHAL_LOG("[allocM4UMemory] E \n"); 
    MHAL_LOG("[allocM4UMemory] virtAddr = 0x%x, size = %d \n", virtAddr, size); 
    M4U_MODULE_ID_ENUM eM4U_ID = M4U_CLNTMOD_FD_INPUT;
    M4U_PORT_STRUCT port;
    ret=pM4UDrv->m4u_alloc_mva(eM4U_ID , virtAddr,  size , m4uVa);
    if(ret!=0)
    	 MHAL_LOG("[freeM4UMemory] m4u_invalid_tlb_range fail \n"); 
    pM4UDrv->m4u_manual_insert_entry(eM4U_ID , *m4uVa, true);
    pM4UDrv->m4u_insert_tlb_range(eM4U_ID, *m4uVa,  *m4uVa + size-1 , RT_RANGE_HIGH_PRIORITY, 0);
    MHAL_LOG("[allocM4UMemory] m4uVa = 0x%x \n", *m4uVa); 

    return ret; 
}


/*******************************************************************************
*
********************************************************************************/
MINT32 halFDVT::freeM4UMemory(MUINT32 m4uVa, MUINT32 size)
{
    if (pM4UDrv == NULL)  {
        MHAL_LOG("Null M4U driver \n"); 
        return -1;
    } 
    MINT32 ret = 0; 
    MHAL_LOG("[freeM4UMemory] E \n"); 
    MHAL_LOG("[freeM4UMemory] m4uVa = 0x%x, size = %d \n", m4uVa, size); 
    ret = pM4UDrv->m4u_invalid_tlb_range( M4U_CLNTMOD_FD_INPUT, mIspReadPort.M4UVa , (mIspReadPort.M4UVa+mIspReadPort.size));
    if(ret!=0)
    	 MHAL_LOG("[freeM4UMemory] m4u_invalid_tlb_range fail \n"); 
    ret = pM4UDrv->m4u_dealloc_mva( M4U_CLNTMOD_FD_INPUT, m4uVa , size, mIspReadPort.M4UVa);
    return ret; 
}

MUINT32 halFDVT::vir2m4u(MUINT32 imgBufAddr)
{
	 for(MUINT32 i=0;i<m_BuffCount;i++)
	 {
	 	   MHAL_LOG("[vir2m4u] m_BuffCount %d m_FDW %d m_FDH %d imgBufAddr 0x%x VA 0x%x \n", m_BuffCount, m_FDW,m_FDH,imgBufAddr,(m_RegisterBuff+(i*m_FDW*m_FDH*2))); 
	     if(imgBufAddr==(m_RegisterBuff+(i*m_FDW*m_FDH*2)))
	     	{
	     		 MHAL_LOG("[vir2m4u] in imgBufAddr 0x%x mva 0x%x \n", imgBufAddr,((MUINT32)mIspReadPort.M4UVa)+(i*m_FDW*m_FDH*2)); 
	         return (MUINT32)(((MUINT32)mIspReadPort.M4UVa)+(i*m_FDW*m_FDH*2));
	      }
	 }
	 MHAL_LOG("[vir2m4u] out imgBufAddr 0x%x \n", imgBufAddr); 
	 return 0;
}

#endif 
