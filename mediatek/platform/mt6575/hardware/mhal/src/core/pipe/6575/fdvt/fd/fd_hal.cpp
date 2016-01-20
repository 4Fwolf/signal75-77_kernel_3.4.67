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
** $Log: fd_hal.cpp $
 *
*/
#define LOG_TAG "mHalFD"
#include <utils/Errors.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "MediaHal.h"
#include <mhal/inc/camera/faces.h>
#include "MediaLog.h"
#include "MediaAssert.h"
#include "fd_hal.h"
#include "AppFD.h"

/*******************************************************************************
*
********************************************************************************/
//-------------------------------------------//
//  Global face detection related parameter  //
//-------------------------------------------//
#define BOX_VIBRATION  3
#define MINI_DET_SIZE_INDEX_00     1    // value 0 - 8
#define MINI_DET_SIZE_INDEX_30     1    // value 0 - 8
#define FRAME_DETECT_DIVISION      12   // value must > 0
#define FD_TRACKING_FACE_NUM       9
#define TCS_TIMES_TO_CLOSE         30
#define DIRECTION_SUPPORT          2    // Range 1 - 4
#define RIP_SUPPORT                3    // 1 : 0 degree only  3: -30 ~ +30 degree
#define RGB_DIFFERENCE_THRESHOLD   30
#define FDE_FACESELECTIONMODE      1    /* 0 : Prior to size.        */
                                        /* 1 : Prior to center.      */
                                        /* 2 : Size/Center weighting.*/  
#define FDE_FACESELECTIONWEIGHT	   128  /* Weight for size or center.*/
                                        /* This value is for size.   */
                                        /* Range is 0 - 256.         */
                                        /* 256 means weight = 1.0.   */
                                        /* Center weight is          */
                                        /* 256 - FdeFaceSelectionWeight */                                     
#define ROTATION_SETTING           1    // 0: CR -> CCR	1: CCR -> CR
#define HORI_TOP_CYCLE             1    /* 1- 10, detection cycles in top direction */
#define HORI_BOT_CYCLE             1    /* 1- 10, detection cycles in bottom direction */
#define HORI_LFT_CYCLE             1    /* 1- 10, detection cycles in left direction */
#define HORI_RHT_CYCLE             1    /* 1- 10, detection cycles in right direction */

#ifdef SmileDetect
#define SD_MAX_DETECT_NUM_PER_ROUND 2	/* 1-9, max smiles for detecting in a fd process  */
#define SD_TOLERANCE_NUM            8   /* 1-, stability level  */
#define SD_MAIN_FACE_MUST_FLAG      1   /* 0/1, if the main face smile needs to be detected  */
#define SD_REQUIRED_SMILE_NUM       1   /* 1-9, number of smiles needs to be detected  */

#define EnableSDFlag                m_DetectPara & 0x01
#endif
/*******************************************************************************
*
********************************************************************************/
static halFDBase *pHalFD = NULL;

/*******************************************************************************
*
********************************************************************************/
halFDBase*
halFD::
getInstance()
{
    MHAL_LOG("[halFDTmp] getInstance \n");
    if (pHalFD == NULL) {
        pHalFD = new halFD();
    }
    return pHalFD;
}

/*******************************************************************************
*
********************************************************************************/
void   
halFD::
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
halFD::halFD()
{
     m_pMTKFDObj = NULL;

    m_FDW = 0; 
    m_FDH = 0; 
    m_DispW = 0; 
    m_DispH = 0; 
    m_DispX = 0; 
    m_DispY = 0;
    m_DispRoate = 0;
    m_DetectPara = 0;     
}


halFD::~halFD()
{
     m_pMTKFDObj = NULL;

    m_FDW = 0; 
    m_FDH = 0; 
    m_DispW = 0; 
    m_DispH = 0; 
    m_DispX = 0; 
    m_DispY = 0;
    m_DispRoate = 0;
    m_DetectPara = 0;     
}


MINT32
halFD::halFDInit(
    MUINT32 fdW,
    MUINT32 fdH
)
{
    MINT32 err = MHAL_NO_ERROR;
    camera_fd_tuning_struct fd_tuning_data;
    
    //MHAL_LOG("[mHalFDInit] Start \n");
    m_FDW = fdW;
    m_FDH = fdH;
    m_DispW = 0;
    m_DispH = 0;
    m_DispX = 0;
    m_DispY = 0;
    m_DispRoate = 0;

    m_pMTKFDObj = new AppFD();
    MHAL_ASSERT(m_pMTKFDObj != NULL, "m_pMTKFDObj is NULL");

    fd_tuning_data.box_vibration_tolerance = BOX_VIBRATION; /* 0 - 6 */        
    fd_tuning_data.min_detect_face_size_index_00 = MINI_DET_SIZE_INDEX_00; /* 0 - 8 */
    fd_tuning_data.min_detect_face_size_index_30 = MINI_DET_SIZE_INDEX_30; /* 0 - 8 */
    fd_tuning_data.frame_detect_division = FRAME_DETECT_DIVISION; /* 1 - 20 */    
    fd_tuning_data.max_tracking_face_num = FD_TRACKING_FACE_NUM; /* 1 - 9 */    
    fd_tuning_data.error_box_closing_time = TCS_TIMES_TO_CLOSE; /* 15 - 110 */    
    fd_tuning_data.phone_rotation_mode = ROTATION_SETTING; /* H_CR_CCR_MODE or H_CCR_CR_MODE */    
    fd_tuning_data.support_direction_num = DIRECTION_SUPPORT; /* 1 - 4 */    
    fd_tuning_data.support_rip_num = RIP_SUPPORT; /* RIP_00 or RIP_00_30 */    
    fd_tuning_data.color_check_threshold = RGB_DIFFERENCE_THRESHOLD;	 /* 30 - 60 */    
    fd_tuning_data.priority_mode = FDE_FACESELECTIONMODE; /* PRIOR_TO_SIZE, PRIOR_TO_CENTER, PRIOR_TO_BOTH_WEIGHT */    
    fd_tuning_data.priority_weight = FDE_FACESELECTIONWEIGHT; /* 0 - 256, the value is for size*/    
    fd_tuning_data.hori_top_det_cycle = HORI_TOP_CYCLE; /* 1- 10, detection cycles in top direction */
    fd_tuning_data.hori_bot_det_cycle = HORI_BOT_CYCLE; /* 1- 10, detection cycles in bottom direction */
    fd_tuning_data.hori_lft_det_cycle = HORI_LFT_CYCLE; /* 1- 10, detection cycles in left direction */
    fd_tuning_data.hori_rgt_det_cycle = HORI_RHT_CYCLE; /* 1- 10, detection cycles in right direction */
    
    #ifdef SmileDetect
    fd_tuning_data.sd_max_detect_smile_per_round = SD_MAX_DETECT_NUM_PER_ROUND;	
    fd_tuning_data.sd_tolerance_count = SD_TOLERANCE_NUM;
    fd_tuning_data.sd_main_face_must_flag = (kal_bool)SD_MAIN_FACE_MUST_FLAG;
    fd_tuning_data.sd_smile_required_num = SD_REQUIRED_SMILE_NUM;
    #endif
    
    m_pMTKFDObj->FDVTInit((kal_uint32*)&fd_tuning_data);    
       
    //MHAL_LOG("[mHalFDInit] End \n");

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
halFD::halFDDo(
    MUINT32 imgBufAddr
)
{
    FDVT_OPERATION_MODE_ENUM fdvt_state = FDVT_IDLE_MODE;
    //fdvt_result_struct fdvt_result;
    //fdvt_result_struct *pfdvt_result_data = &fdvt_result;
    //MUINT32 faceCnt = 0;
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
                
                MINT32 i4WriteCnt = fwrite((void *)imgBufAddr,2, (160 * 120 * 1),pRawFp);
                fflush(pRawFp); 
                fclose(pRawFp);
                printf(" Save RAW Image Done\n");
                while(1);
    }
    #endif
    //MHAL_LOG("  1 imgBufAddr 0x%x \n", imgBufAddr);    
    fdvt_state = FDVT_LFD_MODE;
    m_pMTKFDObj->FDVTMain((kal_uint32)imgBufAddr, (kal_uint32)imgBufAddr, fdvt_state);
    //MHAL_LOG("  2 imgBufAddr 0x%x \n", imgBufAddr);
    fdvt_state = FDVT_GFD_MODE;
    m_pMTKFDObj->FDVTMain((kal_uint32)imgBufAddr, (kal_uint32)imgBufAddr, fdvt_state);
    #ifdef SmileDetect  
    if(EnableSDFlag)
    {
        fdvt_state = FDVT_SD_MODE;
        m_pMTKFDObj->FDVTMain((kal_uint32)imgBufAddr, (kal_uint32)imgBufAddr, fdvt_state); 
    }
    #endif
    //MHAL_LOG("  3 imgBufAddr 0x%x \n", imgBufAddr);                
    //m_pMTKFDObj->FDVTGetResult(pfdvt_result_data);
    /*
    faceCnt = 0;
    for (MUINT32 i = 0 ; i < MAX_FACE_SEL_NUM; i++) {
        if (pfdvt_result_data->display_flag[i] == KAL_FALSE ) {
            continue;
        }
        faceCnt++;

    }
    
    if (faceCnt > 0) {
        MHAL_LOG("  faceCnt %d \n", faceCnt);    
    }
    */
    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFD::halFDUninit(
)
{
    if (m_pMTKFDObj) {
        delete m_pMTKFDObj;
    }
    m_pMTKFDObj = NULL;

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFD::halFDDrawFaceRect(
    MUINT8 *pbuf
)
{
    MINT32 err = MHAL_NO_ERROR;
    
    //memset(plrVirtBuf, 0, lrMemSize);
    m_pMTKFDObj->FDVTDrawFaceRect((MUINT32) pbuf,m_DispW,m_DispH,m_DispX,m_DispY,m_DispRoate);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFD::halFDGetFaceResult(
    camera_face_metadata_m *fd_result,
    MUINT32 DrawMode
)
{
    MINT32 faceCnt = 0;
    MUINT8 pbuf[1024];
    
    //MHAL_ASSERT(bufSize >= sizeof(fdvt_result_struct) * 16, "bufSize is too small");
    
    faceCnt = m_pMTKFDObj->FDVTGetResult((MUINT32) pbuf);

    #if 0
    if (faceCnt > 0) {
        fdvt_result_struct *presult = (fdvt_result_struct *) pbuf;
        for (int i = 0; i < 9; i++) {
            MHAL_LOG("[mHalFDGetFaceResult] %d, %d, %d, %d, %d, %d \n", 
                presult->af_face_indicator,
                presult->display_flag,
                presult->face_display_pos_x0,
                presult->face_display_pos_y0,
                presult->face_display_pos_x1,
                presult->face_display_pos_y1);
            presult++;    
        }
    }
    #endif
    
    return faceCnt;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFD::halFDSetDispInfo(
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

    return err;
}

#ifdef SmileDetect
/*******************************************************************************
*
********************************************************************************/

MINT32
halFD::halSetDetectPara(MUINT8 Para)
{
    MINT32 err = MHAL_NO_ERROR;
    
    m_DetectPara = Para;
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFD::halSDGetSmileResult( )
{
    MINT32 SmileCnt = 0;
    MUINT8 pbuf1[8];
       
    SmileCnt = m_pMTKFDObj->FDVTGetSDResult((MUINT32) pbuf1);
    
    return SmileCnt;
}
#endif
