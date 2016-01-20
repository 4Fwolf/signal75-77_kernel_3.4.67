/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "AppFDVT"

#include "AppFDVT.h"
#include "fdvt_drv.h"
#include "mt6575_fdvt_learning_data.h"
#include "camera_custom_if.h"
#include "fd_sw.h"

#include <cutils/xlog.h>
#include <math.h>
#include <linux/cache.h>
#include <mhal/inc/camera/faces.h>

#define LOGD(fmt, arg...)   XLOGD(fmt, ##arg)
#define LOGE(fmt, arg...)   XLOGE(fmt, ##arg)

//Please fix me Sava
#if defined(MTK_M4U_SUPPORT)
//#undef MTK_M4U_SUPPORT
#endif

#if defined(MTK_M4U_SUPPORT)
#include "m4u_lib.h"
static MTKM4UDrv * pM4UDrv = NULL;
#endif 

#define SVM_OPITION (1)
#define SVM_NUM (1)
#define RED 0xF800
#define ORANGE 0xFCA5
#define BLACK 0x8041
#define DIR_NUM_DET_PRE_FRAME      3
#define DIRECTION_SUPPORT          3 
#define ROTATION_SETTING           0
#define FDVT_PARA_NUM              256
#define FDVT_BUFF_NUM              512
#define g_rs_num 11

#define ByteAlignment  8
#define FD_RESULT_MAX_SIZE (512 * 2)	// 511 faces, 2 words/face
#define TC_RESULT_MAX_SIZE (512 * 4)	// 511 faces, 4 words/face
#define RS_CONFIG_MAX_SIZE (128 * 2)	
#define FD_CONFIG_MAX_SIZE (64 * 4)	
#define RS_RESULT_MAX_SIZE (IMAGE_WIDTH*IMAGE_HEIGHT * 3 * 2) 
#define FDVT_TOTAL_BUFFER_SIZE (FD_RESULT_MAX_SIZE+TC_RESULT_MAX_SIZE+RS_CONFIG_MAX_SIZE+FD_CONFIG_MAX_SIZE+RS_RESULT_MAX_SIZE+40) //40byte for Alignment
#define MAX_FACE_CANDIDATE_NUM 512

// Priority function parameter
#define FDE_FACESELECTIONMODE 1     /* 0 : Prior to size.        */
                                    /* 1 : Prior to center.      */
                                    /* 2 : Size/Center weighting.*/
#define FDE_FACESELECTIONAREAX0	40   /* Definition of center      */
#define FDE_FACESELECTIONAREAY0 30   /* area to select centeral   */
#define FDE_FACESELECTIONAREAX1 120  /* face.                     */
#define FDE_FACESELECTIONAREAY1 90  
#define FDE_FACESELECTIONWEIGHT	128  /* Weight for size or center.*/
                                     /* This value is for size.   */
                                     /* Range is 0 - 256.         */
                                     /* 256 means weight = 1.0.   */
                                     /* Center weight is          */
                                     /* 256 - FdeFaceSelectionWeight */
#define RGB_DIFFERENCE_THRESHOLD 30
#define TRACKING_TIMES_LIMITATION 5
#define BOX_VIBRATION 1		    //  Range >= 0 , Box variance > Box vibration update box position                                     
#define SIZE_SMOOTH_FACTOR  80
#define SMOOTH_POSITION_THRESHOLD    0.80
#define TRIGGER_POSITION_THRESHOLD   0.03

#define LFD_POSE_EXT       1
#define LFD_SIZE_EXT       1
#define LFD_SHRINK_INDEX   3
#define LFD_PATTERN_TYPE   0

#if SVM_OPITION
int LFD_SKIP_MODE   =  0; //FD3.1 Skip 1
int GFD_SKIP_MODE  =  1;
#else
int LFD_SKIP_MODE  =  0;//FD3.0 Skip 0
//#define GFD_SKIP_MODE      1 
#endif

struct  rs_info         RS_INFO[g_rs_num];
struct  fd_info         GFD_INFO;   // full search active window
struct  tracking_info   LFD_INFO[FDVT_BUFF_NUM];   // local search active window
struct  result          FD_RESULT[FDVT_BUFF_NUM];
struct  tc_result       TC_RESULT[FDVT_BUFF_NUM]; 
struct  result          TEMP_FD_RESULT[FDVT_BUFF_NUM];
struct  tc_result       TEMP_TC_RESULT[FDVT_BUFF_NUM];    
int                     LFD_CHECK_ARRAY[MAX_TRACKING_FACE_NUM+1];
//int                     face_label_valid[FDVT_BUFF_NUM];
int                     face_label_valid[MAX_TRACKING_FACE_NUM+1];
int                     FULL_DET_DIR_DISPLAY_FLAG[13];
MUINT32                  fdvtreg_adr[FDVT_PARA_NUM];
MUINT32                  fdvtreg_value[FDVT_PARA_NUM];
struct  result          FDVTResult_buffer[MAX_FACE_NUM];
struct  result          FDVTResult2AP[MAX_FACE_NUM];

int AF_Face_Indicator[MAX_FACE_CANDIDATE_NUM] ;   
int Box_posi_x0[MAX_FACE_CANDIDATE_NUM];
int Box_posi_y0[MAX_FACE_CANDIDATE_NUM];	// Position of the face candidates
int Box_posi_x1[MAX_FACE_CANDIDATE_NUM];
int Box_posi_y1[MAX_FACE_CANDIDATE_NUM];	// Position of the face candidates
int Box_pdiff_x0[MAX_FACE_CANDIDATE_NUM];
int Box_pdiff_y0[MAX_FACE_CANDIDATE_NUM];

int AVG_R_VALUE[MAX_TRACKING_FACE_NUM+1] ;
int AVG_G_VALUE[MAX_TRACKING_FACE_NUM+1] ;
int AVG_B_VALUE[MAX_TRACKING_FACE_NUM+1] ;
int display_flag[MAX_TRACKING_FACE_NUM+1];
int Box_display_position_update_flag[MAX_TRACKING_FACE_NUM+1];
int execute_skin_color_track[MAX_TRACKING_FACE_NUM+1];
int Continuous_LFD_tracking_count[MAX_TRACKING_FACE_NUM+1];
int Non_LFD_tracking_count[MAX_TRACKING_FACE_NUM+1];
int Face_tracked_num_count[MAX_TRACKING_FACE_NUM+1];
int Face_reliabiliy_value[MAX_TRACKING_FACE_NUM+1];
float AVG_DIV_RG[MAX_TRACKING_FACE_NUM+1];
    
int GFD_NUM = 0 ;
int LFD_NUM = 0 ;
int SD_NUM = 0 ;
int FrameCount;
int g_CountTH = 5;
int Color_compensate_face_number = 0 ;
int Rotation_search=1;
int Current_Direction ;
int Current_Feature_Index ; 
int frame_with_only_box_tracking_face_count ;
int LFD_WINDOW_NUM;

void FD_Direction_Selection(int Direction_Support_Number,
			    int Rotate_CW,					//0: CR -> CCR	1: CCR -> CR
			    int *Current_Direction,
			    int *Current_Feature
			   );    
long long FD_DIR_TRANS_FW2HW(int FULL_DET_DIR_DISPLAY_FLAG[],
                             int Current_Feature_Index_in_one_dir_mode,
                             int fatch_quene_num_per_frame
                            );
int POSE_TRANS_RHW2FW(int pos);
int POSE_TRANS_RFW2HW(int pos);
void POSE_TRANS_HW2FW(struct result *FD_RESULT);
void POSE_TRANS_FW2HW(struct result *FD_RESULT);
void FDRegisterConfig(void* src_image_data);
void InitialDRAM();
void FDGetHWResult(FDVT_OPERATION_MODE_ENUM fd_state);
void GFD_LFD_INDEX_CHECK(void);
void TRACKING_COMPENSATION(
                            int CenterR,
                            int CenterG,
                            int CenterB,
                            int LeftR,
                            int LeftG,
                            int LeftB,
                            int RightR,
                            int RightG,
                            int RightB,
                            int *x0,
                            int *y0,
                            int *x1,
                            int *y1,
                            int *FaceType,
                            int FacePose,
                            float face_div_rg,
                            int face_r_value,
                            int face_g_value,
                            int face_b_value,
                            int *execute_skin_color_track,
                            int *face_statistical_value_differ,
                            int *BOX_COLOR_TRACK_POSITION);                            
void Face_Overlap_Detection(int max_tracking_number, int display_flag[],
			    int reliability_value[], struct result *FD_RESULT);
void FACE_PRIORITY_SORTING(int image_width, int image_height,
			   int max_selected_face_number,
			   int AutoFocusFaceLabel[],
			   int display_flag[], struct result *FD_RESULT);			    
void Smoother(int max_tracking_number, int display_x0[], int display_y0[],
		 	   int display_x1[], int display_y1[], int diffx[], int diffy[],
		 	   int Box_display_position_update_flag[],
		 	   int size_smooth_factor, float smooth_position_thrd,     // 0-1, higher means smoother & long latency
                	   float trigger_position_thrd,    // 0-1, higher means less sensitive
                	   int display_flag[], struct result *FD_RESULT);

//********************************************************************//
//***** Face Tracking Function BinChang 20120419 *****//
BOOL Mean_Shift(UINT8* RGB_Candidate, unsigned short int width, unsigned short int height, FACEDETECT_RECT *pRect, short int *num_face, unsigned char m_flag[], 
				float *V, float axis1, float axis2);

//#define Ratio      (1024)
#define Ratio      (1536)
static BOOL g_Track_retvalue = FALSE;
static FACEDETECT_RECT pRect[10];
static short int pwTotalRect = 0;
static float axis1;
static float axis2;
static float m_pV[4];
static unsigned char m_flag[10];
static unsigned short int g_CountIndex = 0;
static unsigned char TRACK_PASS = 0;

static unsigned short int num = 1024;
static unsigned short int Histogram_Target[1024];
static unsigned short int Histogram_Candidate[1024];
static unsigned char Tracking_flag;

static unsigned char g_COUNT_TH = 40;
//*******************************************************************//

MUINT32 *hw_fdvt_buffer = NULL;

/*
MUINT32 *hw_fd_result_data = NULL;
MUINT32 *hw_tc_result_data = NULL;
MUINT32 *rs_config_data = NULL;
MUINT32 *fd_config_data = NULL;
MUINT32 *hw_rs_result_data = NULL;
*/              							  
MUINT32 *hw_fd_result_datava = NULL;
MUINT32 *hw_tc_result_datava = NULL;
MUINT32 *rs_config_datava = NULL;
MUINT32 *fd_config_datava = NULL;
MUINT32 *hw_rs_result_datava = NULL;

MUINT32 *hw_fd_result_datapa = NULL;
MUINT32 *hw_tc_result_datapa = NULL;
MUINT32 *rs_config_datapa = NULL;
MUINT32 *fd_config_datapa = NULL;
MUINT32 *hw_rs_result_datapa = NULL;

MUINT32 *learning_dataph=NULL;
MUINT32 *learning_data0ph=NULL;
MUINT32 *learning_data1ph=NULL;
MUINT32 *learning_data2ph=NULL;
MUINT32 *learning_data3ph=NULL;
MUINT32 *learning_data4ph=NULL;
MUINT32 *learning_data5ph=NULL;
MUINT32 *learning_data6ph=NULL;
MUINT32 *learning_data7ph=NULL;

MUINT32 *learning_datava=NULL;
MUINT32 *learning_data0va=NULL;
MUINT32 *learning_data1va=NULL;
MUINT32 *learning_data2va=NULL;
MUINT32 *learning_data3va=NULL;
MUINT32 *learning_data4va=NULL;
MUINT32 *learning_data5va=NULL;
MUINT32 *learning_data6va=NULL;
MUINT32 *learning_data7va=NULL;

MUINT32 LearningSize=0;

int fdrt_fd;
int tcrt_fd;  
int rscf_fd;
int fdcf_fd; 
int rsrt_fd;
    
int *g_wDisplay_flag = NULL;
//*************************************************************//
//**********************FD 3. 1 ******************************//
#if SVM_OPITION
int gGfdPatternType = GFD_SKIP_MODE;
/*
struct fd_post_init_param
{
    int svm_option;
};
struct fd_post_param
{
    int real_rs_loop;
    int dest_wd[31];
	int dest_ht[31];
    unsigned char *pyramid_image_addr[31];
};
*/
fd_post_param Param;
fd_post_init_param InitParam;
int TOTAL_NUM = 0;
int COUNT_INDEX =0;

#endif
//*************************************************************//    
    
MUINT32 rs_config_table[RS_CONFIG_MAX_SIZE] __attribute__ ((aligned(8) ))= {
0x00000000, 0x01000000, 0x8501E140, 0x00022007, 0x00000000, 0x00000000, 
//0x00000000, 0x01000000, 0x8501E140, 0x00002007, 0x00000000, 0x00000000, 
0x01000000, 0x02000000, 0x0401E140, 0x00012806, 0x00000000, 0x00000000, 
0x02000000, 0x03000000, 0xCB358100, 0x00012804, 0x00000000, 0x00000000, 
0x03000000, 0x04000000, 0xD29132CD, 0x00012803, 0x00000000, 0x00000000, 
0x04000000, 0x05000000, 0x0A0CF4A4, 0x00012803, 0x00000000, 0x00000000, 
0x05000000, 0x06000000, 0x69A4C283, 0x00012802, 0x00000000, 0x00000000, 
0x06000000, 0x07000000, 0xE9509A69, 0x00012801, 0x00000000, 0x00000000, 
0x07000000, 0x08000000, 0x890C7A54, 0x00012801, 0x00000000, 0x00000000, 
0x08000000, 0x09000000, 0x38D46243, 0x00012801, 0x00000000, 0x00000000, 
0x09000000, 0x0A000000, 0xF8A84E35, 0x00012800, 0x00000000, 0x00000000, 
0x0A000000, 0x0B000000, 0xC8843E2A, 0x00012800, 0x00000000, 0x00000000};

MUINT32 fd_config_table[FD_CONFIG_MAX_SIZE] __attribute__ ((aligned(8) ))= {
0x0B000000, 0x00023221, 0x1806D2A0, 0x0000000C, 
0x0A000000, 0x00023E2A, 0x12088EE6, 0x00000009, 
0x09000000, 0x00024E35, 0x100AABEB, 0x00000008, 
0x08000000, 0x00026243, 0x0C0D6989, 0x00000006, 
0x07000000, 0x00027A54, 0x0A10C7A1, 0x00000005, 
0x06000000, 0x00029A69, 0x0814E61A, 0x00000004, 
0x05000000, 0x0002C283, 0x061A24E2, 0x00000003, 
0x04000000, 0x0002F4A4, 0x0620C3E8, 0x00000003, 
0x03000000, 0x000332CD, 0x0428E320, 0x00000002, 
0x02000000, 0x00038100, 0x04332280, 0x00000002, 
0x01000000, 0x0003E140, 0x04400200, 0x00000002};

#define REG_FDVT_GFD_NUM_RESULT_MASK				0x000001ff

#if SVM_OPITION
#define REG_FDVT_LFD_NUM_RESULT_MASK				0x01ff0000
#else
#define REG_FDVT_LFD_NUM_RESULT_MASK				0x000f0000
#endif

//#define REG_RMAP                                                0x05230401   //FD3.0
#define REG_RMAP                                                0x01230101    //FD3.1

#ifdef SmileDetect
int SD_LFD_WINDOW_NUM;     
struct  tracking_info   SD_LFD_INFO[FDVT_BUFF_NUM];
void SDRegisterConfig(void* src_image_data);
#endif
 

//#define Debug
//#define LOGON
#ifdef Debug
#define MEDIA_PATH "//sdcard"
int Count=0;
#endif

static MTKDetection *pAppFDVT = NULL;

/*******************************************************************************
*
********************************************************************************/
MTKDetection*
AppFDVT::
getInstance()
{
    LOGD("[halFDVT] getInstance \n");
    if (pAppFDVT == NULL) {
        pAppFDVT = new AppFDVT(); 
    }
    return pAppFDVT;
}

/*******************************************************************************
*
********************************************************************************/
void   
AppFDVT::
destroyInstance() 
{
    if (pAppFDVT) {
        delete pAppFDVT;
    }
    pAppFDVT = NULL;
}

/*******************************************************************************
*
********************************************************************************/
AppFDVT::AppFDVT(): 
    MTKDetection()
{ 
    #ifdef LOGON
    LOGD("constructor\n");
    #endif   
    halFDVT_OpenDriver();
    InitialDRAM();
}

/*******************************************************************************
*
********************************************************************************/
AppFDVT::~AppFDVT()
{
    #ifdef LOGON
    LOGD("destructor\n");   
    #endif
    
    #if defined(MTK_M4U_SUPPORT)
        //
       if (mWorkingBuffer.useM4U) {
            LOGD("[stop] free M4U fdvt mWorkingBuffer memory memory \n"); 
            freeM4UMemory(mWorkingBuffer.virtAddr, mWorkingBuffer.size,mWorkingBuffer.M4UVa); 
            mWorkingBuffer.useM4U = 0; 
        }
        
        if (mLearningData.useM4U) {
            LOGD("[stop] free M4U fdvt mLearningData memory \n"); 
            freeM4UMemory(mLearningData.virtAddr, mLearningData.size,mLearningData.M4UVa); 
            mLearningData.useM4U = 0; 
        }
        
        if (pM4UDrv) {
            delete pM4UDrv;
            pM4UDrv = NULL; 
            memset (&mWorkingBuffer, 0,  sizeof(FDVTM4UInfo)); 
            memset (&mLearningData, 0,  sizeof(FDVTM4UInfo)); 
        }   
    free(hw_fdvt_buffer);
    free(learning_datava);
    #else
    if (NULL != hw_fdvt_buffer)  {
       pmem_free(hw_fdvt_buffer, (FDVT_TOTAL_BUFFER_SIZE+ByteAlignment), fdrt_fd);        
    }
    
    if (NULL != learning_datava)  {
       pmem_free(learning_datava, (LearningSize+ByteAlignment), tcrt_fd);        
    }
    
    #endif
    
    
    
    hw_fdvt_buffer=NULL; 
    hw_fd_result_datava = NULL;
    hw_tc_result_datava = NULL;
    rs_config_datava = NULL;
    fd_config_datava = NULL;
    hw_rs_result_datava = NULL;

    hw_fd_result_datapa = NULL;
    hw_tc_result_datapa = NULL;
    rs_config_datapa = NULL;
    fd_config_datapa = NULL;
    hw_rs_result_datapa = NULL;
    
    learning_dataph=NULL;
    learning_data0ph=NULL;
    learning_data1ph=NULL;
    learning_data2ph=NULL;
    learning_data3ph=NULL;
    learning_data4ph=NULL;
    learning_data5ph=NULL;
    learning_data6ph=NULL;
    learning_data7ph=NULL;
    
    learning_datava=NULL;
    learning_data0va=NULL;
    learning_data1va=NULL;
    learning_data2va=NULL;
    learning_data3va=NULL;
    learning_data4va=NULL;
    learning_data5va=NULL;
    learning_data6va=NULL;
    learning_data7va=NULL;

    halFDVT_Uninit();
}


void AppFDVT::FDVTInit(kal_uint32 *tuning_data)
{
	LOGD("MTKFD_v3.10 SVM AppFDVT\n");
	
    #ifdef LOGON
    LOGD("FDVTInit IN \n");
    #endif 
    
    for (int m = 0 ; m < MAX_TRACKING_FACE_NUM ; m ++)
    {
        FD_RESULT[m].face_index = MAX_FACE_NUM - 1 ; 
        FD_RESULT[m].type = 3 ;
        FD_RESULT[m].x0 = 0 ;
        FD_RESULT[m].y0 = 0 ;
        FD_RESULT[m].x1 = 0 ;
        FD_RESULT[m].y1 = 0 ;
        FD_RESULT[m].fcv = 0 ;
        FD_RESULT[m].rip_dir = 0 ;
        FD_RESULT[m].rop_dir = 0 ;
        FD_RESULT[m].size_index = 0 ;
    }
    for (int m = 0 ; m < MAX_FACE_CANDIDATE_NUM ; m ++)
    {
        Box_posi_x0[m] = 0;
        Box_posi_y0[m] = 0;	// Position of the face candidates
        Box_posi_x1[m] = 0;
        Box_posi_y1[m] = 0;	// Position of the face candidates
        Box_pdiff_x0[m] = 0;
        Box_pdiff_y0[m] = 0;
    }
    GFD_NUM = 0 ;
    LFD_NUM = 0 ;
    SD_NUM = 0 ;
    FrameCount = 0;
    Color_compensate_face_number = 0 ;
    g_CountIndex = 0;
 
    //BinChang 2011/11/30 Get SD Threshold
    g_CountTH  = NSCamCustom::get_SD_threshold(); 
 
//*************************************************************//
#if SVM_OPITION
    InitParam.svm_option = SVM_NUM;
    FdPostInit(InitParam);
#endif
//*************************************************************//
  
   Tracking_flag = 0;
   for(int i =0; i<10;i++)
       m_flag[i]=0;
  
    #ifdef LOGON
    LOGD("FDVTInit OUT \n");
    #endif
}

void AppFDVT::FDVTMain(kal_uint32 image_buffer_address, kal_uint32 image_buffer_address1, FDVT_OPERATION_MODE_ENUM fd_state)
{       
    int m=0;
    int Start_fatch_quene_num ;     
    int fatch_quene_num_per_frame ;    
    //int Current_Feature_Index ; 
/*    
    int AVG_R_VALUE[512] ;
    int AVG_G_VALUE[512] ;
    int AVG_B_VALUE[512] ;
    int display_flag[512];
    int Box_display_position_update_flag[512];
    int execute_skin_color_track[512];
    int Continuous_LFD_tracking_count[512];
    int Non_LFD_tracking_count[512];
    int Face_tracked_num_count[512];
    int Face_reliabiliy_value[512];
    float AVG_DIV_RG[512];
*/    
    int TC_x0, TC_y0, TC_x1, TC_y1, TC_type, TC_execute_skin_color_track, TC_Face_reliabiliy_value ;
    int Box_tracking_face_number ;
    int Wait2ProcessTC_number, track_success ;
    int BOX_COLOR_TRACK_POSITION   ;
    
    MUINT32  *rs_dst_datava;
    MUINT32  *rs_cfg;
    MUINT32  *fd_cfg;
    MUINT32  rs_dst_addr;
    MUINT8   rs_num=g_rs_num;
    MUINT16  h,w;
    MUINT8   i;   
    MUINT32  srcbuf;
                     
    #ifdef LOGON
    LOGD("FDVTMain IN \n");
    LOGD("image_buffer_address adr %x %d \n",(MUINT32)image_buffer_address,fd_state);
    #endif
    if (fd_state == FDVT_GFD_MODE)
    {	

        //**************************************************************************************************************//
        //**************    Face Tracking   BinChang 20120419   **********************************************//        
        if(Tracking_flag == 1)
        {
 	     g_Track_retvalue = Mean_Shift( (UINT8*) image_buffer_address1, IMAGE_WIDTH, IMAGE_HEIGHT, pRect, &pwTotalRect, m_flag, m_pV, 
				                          axis1, axis2);
			
	     if((g_Track_retvalue == 1)  && (g_CountIndex<=g_COUNT_TH) )
	     {	
		GFD_NUM = pwTotalRect;
		LFD_NUM = 0;
               	Color_compensate_face_number = 0;
               	FD_RESULT[0].type = 0;
               	g_wDisplay_flag[0] = 1 ;
               	AF_Face_Indicator[0] = 1;
               	FD_RESULT[0].af_face_indicator=1;
               	          
               	FD_RESULT[0].x0 = pRect[0].wLeft;
               	FD_RESULT[0].y0 = pRect[0].wTop;
		FD_RESULT[0].x1 = FD_RESULT[0].x0 + pRect[0].wWidth;
		FD_RESULT[0].y1 = FD_RESULT[0].y0 + pRect[0].wHeight;
		     	
		memcpy((void*)FDVTResult_buffer, (void*)FD_RESULT, sizeof(result)*MAX_FACE_NUM);
		     	
		g_CountIndex++;
		TRACK_PASS = 0;
		     	
                    return;
               }
               
               else
               {
               	GFD_NUM = 0;
               	LFD_NUM = 0;
               	Color_compensate_face_number = 0;
               	FD_RESULT[0].type = 3;
               	g_wDisplay_flag[0] = 0 ;
               	Tracking_flag = 0;
               	g_CountIndex = 0;
               	AF_Face_Indicator[0] = 0;
               	FD_RESULT[0].af_face_indicator=0;
               	memcpy((void*)FDVTResult_buffer, (void*)FD_RESULT, sizeof(result)*MAX_FACE_NUM);
               	
               	//g_Track_retvalue = 0;               		
               }     
        }
        
       if(g_Track_retvalue==1)
       {		
              TRACK_PASS ++;
              if(TRACK_PASS >=3)
              {
              	    TRACK_PASS = 0;
              	    g_Track_retvalue = 0;
              }
       }	
        //**************************************************************************************************************//
        
        if ((LFD_NUM + GFD_NUM + Color_compensate_face_number) == 0)
        {               
            Rotation_search = 1 ;
        }               
        else if ((LFD_NUM + GFD_NUM) == 0)
        {               
            frame_with_only_box_tracking_face_count ++ ;
        
            if (frame_with_only_box_tracking_face_count > FRAME_NUM_WITHOUT_FACE_TO_DO_ROTATION_SEARCH)
            {           
                Rotation_search = 1 ;
            }           
            else        
            {           
                Rotation_search = 0 ;
            }           
        }               
        else            
        {               
            Rotation_search = 0 ;
            frame_with_only_box_tracking_face_count = 0 ;
        }              
        
#if SVM_OPITION        
        if ((LFD_NUM + GFD_NUM + Color_compensate_face_number) == 0)
        {
             GFD_SKIP_MODE = 1;
             LFD_SKIP_MODE = 0;
        }     
        else
        {	
        	   GFD_SKIP_MODE = 4;
        	   LFD_SKIP_MODE = 0;
        }
        if(gGfdPatternType>GFD_SKIP_MODE)
        {
            gGfdPatternType = GFD_SKIP_MODE;
        }      
        //LOGD("GFD_SKIP_MODE: %d, ",GFD_SKIP_MODE);
#endif        
        
        if( (Rotation_search == 1) && (g_Track_retvalue ==0))
        {
            FD_Direction_Selection(DIRECTION_SUPPORT,ROTATION_SETTING,&Current_Direction,&Current_Feature_Index);
        }
        GFD_INFO.x0 = (int)(IMAGE_WIDTH  * GFD_BOUNDARY_OFF_RATIO) ;
        GFD_INFO.y0 = (int)(IMAGE_HEIGHT * GFD_BOUNDARY_OFF_RATIO) ;
        GFD_INFO.x1 = IMAGE_WIDTH  - GFD_INFO.x0                   ;
        GFD_INFO.y1 = IMAGE_HEIGHT - GFD_INFO.y0                   ;
        
        GFD_INFO.pose    = 
                        (long long) FD_DIR_TRANS_FW2HW( FULL_DET_DIR_DISPLAY_FLAG,
                                                        Current_Feature_Index,
                                                        DIR_NUM_DET_PRE_FRAME
                                                        ) ;
        #ifdef LOGON
        LOGD("FD_DIR_TRANS_FW2HW OUT \n");
        #endif
        POSE_TRANS_FW2HW( FD_RESULT ) ;
        #ifdef LOGON
        LOGD("POSE_TRANS_FW2HW OUT \n");
        #endif        
        LFD_WINDOW_NUM = 0 ;
        
        for (m = 0 ; m < MAX_TRACKING_FACE_NUM ; m ++)
        {               
            LFD_INFO[m].valid       = 0 ;
            LFD_INFO[m].face_index  = 0 ;  
            LFD_INFO[m].x0          = 0 ;  
            LFD_INFO[m].y0          = 0 ;
            LFD_INFO[m].x1          = 0 ;
            LFD_INFO[m].y1          = 0 ;
            LFD_INFO[m].size_index  = 0 ;
            LFD_INFO[m].rip_dir     = 0 ;
            LFD_INFO[m].rop_dir     = 0 ;
            
            LFD_CHECK_ARRAY[m] = 0 ;
            
            if (FD_RESULT[m].type == 0 ||
                FD_RESULT[m].type == 1 ||
                FD_RESULT[m].type == 2 ) // Face candidate exist
            {                            
                LFD_INFO[LFD_WINDOW_NUM].valid      = 1 ;
                LFD_INFO[LFD_WINDOW_NUM].face_index = FD_RESULT[m].face_index ;  
                LFD_INFO[LFD_WINDOW_NUM].x0         = FD_RESULT[m].x0 ;  
                LFD_INFO[LFD_WINDOW_NUM].y0         = FD_RESULT[m].y0 ;
                LFD_INFO[LFD_WINDOW_NUM].x1         = FD_RESULT[m].x1 ;
                LFD_INFO[LFD_WINDOW_NUM].y1         = FD_RESULT[m].y1 ;
                LFD_INFO[LFD_WINDOW_NUM].size_index = FD_RESULT[m].size_index ;
                LFD_INFO[LFD_WINDOW_NUM].rip_dir    = FD_RESULT[m].rip_dir ;
                LFD_INFO[LFD_WINDOW_NUM].rop_dir    = FD_RESULT[m].rop_dir ;
                
                LFD_CHECK_ARRAY[LFD_WINDOW_NUM] = 1 ;
                
                LFD_WINDOW_NUM ++ ;
            }
            
            FD_RESULT[m].face_index = MAX_FACE_NUM - 1 ; 
            FD_RESULT[m].type = 3 ;
            FD_RESULT[m].x0 = 0 ;
            FD_RESULT[m].y0 = 0 ;
            FD_RESULT[m].x1 = 0 ;
            FD_RESULT[m].y1 = 0 ;
            FD_RESULT[m].fcv = 0 ;
            FD_RESULT[m].rip_dir = 0 ;
            FD_RESULT[m].rop_dir = 0 ;
            FD_RESULT[m].size_index = 0 ;
            
            TEMP_FD_RESULT[m].face_index = MAX_FACE_NUM - 1 ; 
            TEMP_FD_RESULT[m].type = 3 ;
            TEMP_FD_RESULT[m].x0 = 0 ;
            TEMP_FD_RESULT[m].y0 = 0 ;
            TEMP_FD_RESULT[m].x1 = 0 ;
            TEMP_FD_RESULT[m].y1 = 0 ;
            TEMP_FD_RESULT[m].fcv = 0 ;
            TEMP_FD_RESULT[m].rop_dir = 0 ;
            TEMP_FD_RESULT[m].rip_dir = 0 ;
            TEMP_FD_RESULT[m].size_index = 0 ;
            
            TC_RESULT[m].face_index = MAX_FACE_NUM - 1 ;
            TC_RESULT[m].type       = 3 ;
            TC_RESULT[m].LeftR      = 0 ;
            TC_RESULT[m].LeftG      = 0 ;
            TC_RESULT[m].LeftB      = 0 ;
            TC_RESULT[m].CenterR    = 0 ;
            TC_RESULT[m].CenterG    = 0 ;
            TC_RESULT[m].CenterB    = 0 ;
            TC_RESULT[m].RightR     = 0 ;
            TC_RESULT[m].RightG     = 0 ;
            TC_RESULT[m].RightB     = 0 ;
            
            TEMP_TC_RESULT[m].face_index = MAX_FACE_NUM - 1 ;
            TEMP_TC_RESULT[m].type       = 3 ;
            TEMP_TC_RESULT[m].LeftR      = 0 ;
            TEMP_TC_RESULT[m].LeftG      = 0 ;
            TEMP_TC_RESULT[m].LeftB      = 0 ;
            TEMP_TC_RESULT[m].CenterR    = 0 ;
            TEMP_TC_RESULT[m].CenterG    = 0 ;
            TEMP_TC_RESULT[m].CenterB    = 0 ;
            TEMP_TC_RESULT[m].RightR     = 0 ;
            TEMP_TC_RESULT[m].RightG     = 0 ;
            TEMP_TC_RESULT[m].RightB     = 0 ;
            face_label_valid[m] = 1 ;
            
        }

        for(m=MAX_TRACKING_FACE_NUM;m<FDVT_BUFF_NUM;m++)
        {
            FD_RESULT[m].face_index = FDVT_BUFF_NUM - 1 ; 
            FD_RESULT[m].type = 3 ;
            FD_RESULT[m].x0 = 0 ;
            FD_RESULT[m].y0 = 0 ;
            FD_RESULT[m].x1 = 0 ;
            FD_RESULT[m].y1 = 0 ;
            FD_RESULT[m].fcv = 0 ;
            FD_RESULT[m].rip_dir = 0 ;
            FD_RESULT[m].rop_dir = 0 ;
            FD_RESULT[m].size_index = 0 ;            
            
        }

        rs_cfg  = rs_config_datava;
        fd_cfg  = fd_config_datava;
        rs_dst_addr = (MUINT32 ) hw_rs_result_datapa;
        srcbuf=image_buffer_address;
        
        for (i = 0; i < rs_num; i++)
        {
        	// rs src addr
        	if (i==0)
        	{
        		// 1st rs src addr = input rgb565 addr
        		rs_cfg[6*i] = srcbuf;
        	}
        	else
        	{
        		// Nth rs src addr = (N-1)th rs dst addr
        		rs_cfg[6*i] = rs_cfg[6*(i-1)+1];
        	}
        	// rs dst addr
        	rs_cfg[6*i+1] = rs_dst_addr;
        	// fd src addr (inverse order)
        	fd_cfg[4*(rs_num-i-1)] = rs_dst_addr;
        
        	w = ((rs_cfg[6*i+2] >> 18) & 0x1FF);
        	h = (((rs_cfg[6*i+3]) & 0xF)<<5)|((rs_cfg[6*i+2] >> 27) & 0x1F);
        	rs_dst_addr += w*h;
        	// round to 8 bytes aligh
        	if ((rs_dst_addr & 0x7) != 0)
        	{
        		rs_dst_addr = (rs_dst_addr & ~0x7) + 0x8;
        	}
        	
        	//*******************************************************//
        	//***********BinChang 20110908 FD 3.1**********//
#if SVM_OPITION
        	RS_INFO[i].width = w;
        	RS_INFO[i].height = h;
        	RS_INFO[i].scale_factor = 1.25;
        	if(i==0)
        	{
        		 RS_INFO[0].rs_addrva = (MUINT32 ) hw_rs_result_datava;
        	}
        	else
        	{
        		RS_INFO[i].rs_addrva = RS_INFO[i-1].rs_addrva + RS_INFO[i-1].width*RS_INFO[i-1].height;
        		if((RS_INFO[i].rs_addrva & 0x7) != 0)
        		    RS_INFO[i].rs_addrva = (RS_INFO[i].rs_addrva & ~0x7) + 0x8;
        	}
#endif        	
        	//*******************************************************//
        	
        }
        #ifdef LOGON
        for(int k=0;k<11;k++)
            LOGD("rs_dst_addr0 0x%x rs_dst_addr1 0x%x rs_dst_addr2 0x%x rs_dst_addr3 0x%x rs_dst_addr4 0x%x rs_dst_addr5 0x%x \n",rs_cfg[k*6],rs_cfg[k*6+1],rs_cfg[k*6+2],rs_cfg[k*6+3],rs_cfg[k*6+4],rs_cfg[k*6+5]); 
        #endif
        FDRegisterConfig((void*)image_buffer_address);
        #if defined (MTK_M4U_SUPPORT)        
        SyncM4UCache(mWorkingBuffer);
        SyncM4UCache(mLearningData);
        #endif
        halFDVT_StartHW((void*)image_buffer_address);
        
        FDGetHWResult(fd_state);
        
#if (!SVM_OPITION)
       //LOGD("FD Engine1: GFD_NUM: %d, LFD_NUM: %d",GFD_NUM, LFD_NUM);
       int GFD_N = 0;
       int LFD_N = 0;
       for(m=0;m<FDVT_BUFF_NUM;m++)
       {
       	      if(FD_RESULT[m].type == 0)
       	      	GFD_N++;
                if(FD_RESULT[m].type == 1)
       	      	LFD_N++;
       }
       //LOGD("FD Engine2: TYPE0: %d, TYPE1: %d",GFD_N, LFD_N);
#endif        
        
       //*******************************************************//
       //***********BinChang 20110908 FD 3.1**********//
#if SVM_OPITION       
       rs_cfg[3] = (rs_cfg[3] & 0xFFFDFFFF); // Clear IGMA_EN
       
       fdvtreg_adr[0]=0x0004;
       fdvtreg_value[0]=0x00000001;
       halFDVT_PARA_SET(fdvtreg_adr,fdvtreg_value,1,FDVT_GFD_MODE);
       halFDVT_StartHW((void*)image_buffer_address);
       halFDVT_Wait_IRQ();
       
       rs_cfg[3] = (rs_cfg[3] | 0x00020000); // Set IGMA_EN
       
       //*Set Param for SVM*//
       Param.real_rs_loop = rs_num;
       for(i=0;i< rs_num;i++)
       {
       	  Param.dest_wd[i] = RS_INFO[i].width;
       	  Param.dest_ht[i] = RS_INFO[i].height;
       	  Param.pyramid_image_addr[i]  =  (unsigned char *) RS_INFO[i].rs_addrva;
       	  //LOGD("param.pyramid_address[%d]: %d,  RS_INFO[%d].rs_addrva: %d \n", i, param.pyramid_image_addr[i], i,  RS_INFO[i].rs_addrva);
       }
       
       //***********SVM**********//
       TOTAL_NUM = GFD_NUM + LFD_NUM;
       //LOGD("FD Engine1: GFD_NUM: %d, LFD_NUM: %d,  TOTAL_NUM: %d",GFD_NUM, LFD_NUM, TOTAL_NUM);
       
       int GFD_N = 0;
       int LFD_N = 0;
       for(m=0;m<FDVT_BUFF_NUM;m++)
       {
       	      if(FD_RESULT[m].type == 0)
       	      	GFD_N++;
                if(FD_RESULT[m].type == 1)
       	      	LFD_N++;
       }
       //LOGD("FD Engine2: TYPE0: %d, TYPE1: %d,  TOTAL_NUM: %d",GFD_N, LFD_N, TOTAL_NUM);
        
       if(TOTAL_NUM > 0)
       {
             FdPostProc(&TOTAL_NUM , FD_RESULT, Param);
       }
       
       //*********Sorting********//
       COUNT_INDEX=0;
       GFD_NUM = 0;
       LFD_NUM = 0;
       
       for(m=0;m<FDVT_BUFF_NUM;m++)
       {
       	  if(FD_RESULT[m].type == 0 || FD_RESULT[m].type == 1 || FD_RESULT[m].type == 2)
       	  {
       	      if(FD_RESULT[m].type == 0)
       	      	GFD_NUM++;
                if(FD_RESULT[m].type == 1)
       	      	LFD_NUM++;
       
       	      FD_RESULT[COUNT_INDEX].af_face_indicator = FD_RESULT[m].af_face_indicator;
       	      FD_RESULT[COUNT_INDEX].face_index = FD_RESULT[m].face_index;
       	      FD_RESULT[COUNT_INDEX].type = FD_RESULT[m].type;
       	      FD_RESULT[COUNT_INDEX].x0 = FD_RESULT[m].x0;
       	      FD_RESULT[COUNT_INDEX].y0 = FD_RESULT[m].y0;
       	      FD_RESULT[COUNT_INDEX].x1 = FD_RESULT[m].x1;
       	      FD_RESULT[COUNT_INDEX].y1 = FD_RESULT[m].y1;
       	      FD_RESULT[COUNT_INDEX].fcv = FD_RESULT[m].fcv;
       	      FD_RESULT[COUNT_INDEX].rip_dir = FD_RESULT[m].rip_dir;
       	      FD_RESULT[COUNT_INDEX].rop_dir = FD_RESULT[m].rop_dir;
       	      FD_RESULT[COUNT_INDEX].size_index = FD_RESULT[m].size_index;
       	      //FD_RESULT[COUNT_INDEX].face_num = FD_RESULT[m].face_num;
       	      COUNT_INDEX++;
       	  }
       }
       
       LOGD("After Sorting GFD_NUM: %d,  LFD_NUM: %d",GFD_NUM, LFD_NUM);
       
       for(m=COUNT_INDEX;m<FDVT_BUFF_NUM;m++)
       {
       	      FD_RESULT[m].type = 3;
       	      FD_RESULT[m].af_face_indicator = 0;
       	      FD_RESULT[m].face_index = MAX_FACE_NUM - 1;
       	      FD_RESULT[m].x0 = 0;
       	      FD_RESULT[m].y0 = 0;
       	      FD_RESULT[m].x1 = 0;
       	      FD_RESULT[m].y1 = 0;
       	      FD_RESULT[m].fcv = 0;
       	      FD_RESULT[m].rip_dir = 0;
       	      FD_RESULT[m].rop_dir = 0;
       	      FD_RESULT[m].size_index = 0;
       }
       
#endif
       //*******************************************************//
        
     //**************************************************************************************************************//
     //**************    Face Tracking   BinChang 20120419   **********************************************//	     
     //Calculate Target Weight Histogram
     unsigned char* RGB_Target_img;
     unsigned short int length;
     

#if 0
     //if( (GFD_NUM == 1) && (LFD_NUM == 0) &&  (Tracking_flag ==0) && (FD_RESULT[0].x1 - FD_RESULT[0].x0 <=160))
     if( (GFD_NUM == 1) && (LFD_NUM == 0) &&  (Tracking_flag ==0) && (FD_RESULT[0].x1 - FD_RESULT[0].x0 <=160) &&(FD_RESULT[0].x1 - FD_RESULT[0].x0 >=24) &&
     	(FD_RESULT[0].y1 - FD_RESULT[0].y0 <=160) && (FD_RESULT[0].y1 - FD_RESULT[0].y0 >=24))
     {
          g_wDisplay_flag = display_flag;
     	RGB_Target_img = (UINT8*) image_buffer_address1 + (FD_RESULT[0].y0  * IMAGE_WIDTH + FD_RESULT[0].x0) * 2; //RGB565 2bytes/pixel
     	length = FD_RESULT[0].x1 - FD_RESULT[0].x0;
     	Compute_Weight_Histogram(RGB_Target_img, Histogram_Target, IMAGE_WIDTH, length);
     	
          pRect[0].wWidth = FD_RESULT[0].x1 - FD_RESULT[0].x0;
          pRect[0].wHeight = FD_RESULT[0].y1 - FD_RESULT[0].y0;
          pRect[0].wTop = FD_RESULT[0].y0;
          pRect[0].wLeft = FD_RESULT[0].x0;
     	pwTotalRect = GFD_NUM;
          m_flag[0] = 1;
     	Tracking_flag = 1;
          g_wDisplay_flag[0] = 1 ;
          AF_Face_Indicator[0] = 1;
          FD_RESULT[0].af_face_indicator=1;
          
          for(int t=GFD_NUM;t<MAX_TRACKING_FACE_NUM;t++)
          {
              g_wDisplay_flag[t] = 0;
              m_flag[t] = 0;
          }
          
          POSE_TRANS_HW2FW( FD_RESULT ) ;
          
          memcpy((void*)FDVTResult_buffer, (void*)FD_RESULT, sizeof(result)*MAX_FACE_NUM); 
/*     	
     	 LOGD("Face Tracking: pRect[0].wWidth: %d,  pRect[0].wHeight : %d, pRect[0].wTop: %d, pRect[0].wLeft : %d ",
     	                pRect[0].wWidth, pRect[0].wHeight, pRect[0].wTop, pRect[0].wLeft );
     	
           LOGD("FD_RESULT[0].x0: %d, FD_RESULT[0].y0: %d, FD_RESULT[0].x1: %d, FD_RESULT[0].y1: %d",
		     	            FD_RESULT[0].x0, FD_RESULT[0].y0, FD_RESULT[0].x1, FD_RESULT[0].y1);
*/		     	            
     	return;
     }

#endif     
     //**************************************************************************************************************//
        
	//----------------------------------------------------------//
	//    Face candidate merge (Face label check function)      //
	//----------------------------------------------------------//
	#ifdef Debug
	if(Count==6)
	{
		for(i=0;i<11;i++)
	    {
	       //UCHAR szFileName[100];
	       char szFileName[100];
            sprintf(szFileName, "%s//Resize_Index%d_%d.raw","//sdcard", Count, i);
	       
	       FILE * pRawFp = fopen(szFileName, "wb");
            if (NULL == pRawFp )
            {
                LOGD("Can't open file to save RAW Image\n"); 
                while(1);
            }
                 
            //int i4WriteCnt = fwrite((void *)hw_rs_result_datava,2, (320 * 240 * 1),pRawFp);
            int i4WriteCnt = fwrite((void *)RS_INFO[i].rs_addrva,1, (RS_INFO[i].width*RS_INFO[i].height),pRawFp);
            fflush(pRawFp); 
            fclose(pRawFp);   
        }
	}
	Count++;
	#endif
	GFD_LFD_INDEX_CHECK() ;
	//------------------------------------------------------------------------------------//
	//    Tracking result check, enter tracking compensation without tracking success     //
	//------------------------------------------------------------------------------------//
	Wait2ProcessTC_number = 0 ;

	for (m = 0 ; m < MAX_TRACKING_FACE_NUM ; m ++)
	{
		if (LFD_CHECK_ARRAY[m] == 1)
		{
			track_success = 0 ;

			for (i = 0 ; i < (GFD_NUM+LFD_NUM) ; i ++)
			{
				if (FD_RESULT[i].face_index == LFD_INFO[m].face_index && 
				    FD_RESULT[i].type == 1)
				{
					track_success = 1 ;
					break ; 
				}
			}

			if (track_success == 0)
			{
				FD_RESULT[GFD_NUM + LFD_NUM + Wait2ProcessTC_number].face_index = LFD_INFO[m].face_index ; 
				FD_RESULT[GFD_NUM + LFD_NUM + Wait2ProcessTC_number].type = 2 ;
				FD_RESULT[GFD_NUM + LFD_NUM + Wait2ProcessTC_number].x0 = LFD_INFO[m].x0 ;
				FD_RESULT[GFD_NUM + LFD_NUM + Wait2ProcessTC_number].y0 = LFD_INFO[m].y0 ;
				FD_RESULT[GFD_NUM + LFD_NUM + Wait2ProcessTC_number].x1 = LFD_INFO[m].x1 ;
				FD_RESULT[GFD_NUM + LFD_NUM + Wait2ProcessTC_number].y1 = LFD_INFO[m].y1 ;
				FD_RESULT[GFD_NUM + LFD_NUM + Wait2ProcessTC_number].fcv = 0 ;
				FD_RESULT[GFD_NUM + LFD_NUM + Wait2ProcessTC_number].rip_dir = LFD_INFO[m].rip_dir ;
				FD_RESULT[GFD_NUM + LFD_NUM + Wait2ProcessTC_number].rop_dir = LFD_INFO[m].rop_dir ;
				FD_RESULT[GFD_NUM + LFD_NUM + Wait2ProcessTC_number].size_index = LFD_INFO[m].size_index ;

				Wait2ProcessTC_number ++ ;
			}                        
		}
	}

	//------------------------------------//
	//    Feature pose transformation     //
	//------------------------------------//

	GFD_INFO.pose = (long long)Current_Feature_Index ;

	POSE_TRANS_HW2FW( FD_RESULT ) ;

	//--------------------------------------------------------//
	//    ReGenerate face label index for new coming face     //
	//--------------------------------------------------------//

	for (m = 0 ; m < MAX_TRACKING_FACE_NUM ; m ++)
	{
		if (FD_RESULT[m].type == 1 || FD_RESULT[m].type == 2) // FD
		{
			face_label_valid[FD_RESULT[m].face_index] = 0 ;
		}
	}

	for (m = 0 ; m < MAX_TRACKING_FACE_NUM ; m ++)
	{
		if (FD_RESULT[m].type == 0) // FD
		{
			for (i = 0 ; i < MAX_TRACKING_FACE_NUM ; i ++)
			{
				if (face_label_valid[i] == 1)
				{
					FD_RESULT[m].face_index = i ;
					TC_RESULT[m].face_index = i ;
					face_label_valid[i] = 0     ;
					break ;
				}
				if(i==MAX_TRACKING_FACE_NUM-1)
					LOGD("face indexing error\n");
			}
		}
	}

	//-------------------------------------------//
	//    Regenerate FD_RESULT and TC_RESULT     //
	//-------------------------------------------//

	for (m = 0 ; m < MAX_TRACKING_FACE_NUM ; m ++)
	{
		TEMP_FD_RESULT[ FD_RESULT[m].face_index ].face_index = FD_RESULT[m].face_index ;
		TEMP_FD_RESULT[ FD_RESULT[m].face_index ].type       = FD_RESULT[m].type ;
		TEMP_FD_RESULT[ FD_RESULT[m].face_index ].x0         = FD_RESULT[m].x0 ;
		TEMP_FD_RESULT[ FD_RESULT[m].face_index ].y0         = FD_RESULT[m].y0 ;
		TEMP_FD_RESULT[ FD_RESULT[m].face_index ].x1         = FD_RESULT[m].x1 ;
		TEMP_FD_RESULT[ FD_RESULT[m].face_index ].y1         = FD_RESULT[m].y1 ;
		TEMP_FD_RESULT[ FD_RESULT[m].face_index ].fcv        = FD_RESULT[m].fcv ;
		TEMP_FD_RESULT[ FD_RESULT[m].face_index ].rip_dir    = FD_RESULT[m].rip_dir ;
		TEMP_FD_RESULT[ FD_RESULT[m].face_index ].rop_dir    = FD_RESULT[m].rop_dir ;
		TEMP_FD_RESULT[ FD_RESULT[m].face_index ].size_index    = FD_RESULT[m].size_index ;

		TEMP_TC_RESULT[ TC_RESULT[m].face_index ].face_index = TC_RESULT[m].face_index ;
		TEMP_TC_RESULT[ TC_RESULT[m].face_index ].type       = TC_RESULT[m].type ;
		TEMP_TC_RESULT[ TC_RESULT[m].face_index ].LeftR      = TC_RESULT[m].LeftR ;
		TEMP_TC_RESULT[ TC_RESULT[m].face_index ].LeftG      = TC_RESULT[m].LeftG ;
		TEMP_TC_RESULT[ TC_RESULT[m].face_index ].LeftB      = TC_RESULT[m].LeftB ;
		TEMP_TC_RESULT[ TC_RESULT[m].face_index ].CenterR    = TC_RESULT[m].CenterR ;
		TEMP_TC_RESULT[ TC_RESULT[m].face_index ].CenterG    = TC_RESULT[m].CenterG ;
		TEMP_TC_RESULT[ TC_RESULT[m].face_index ].CenterB    = TC_RESULT[m].CenterB ;
		TEMP_TC_RESULT[ TC_RESULT[m].face_index ].RightR     = TC_RESULT[m].RightR ;
		TEMP_TC_RESULT[ TC_RESULT[m].face_index ].RightG     = TC_RESULT[m].RightG ;
		TEMP_TC_RESULT[ TC_RESULT[m].face_index ].RightB     = TC_RESULT[m].RightB ;
	}

	for (m = 0 ; m < MAX_TRACKING_FACE_NUM ; m ++)
	{
		FD_RESULT[m].face_index = TEMP_FD_RESULT[m].face_index ;
		FD_RESULT[m].type       = TEMP_FD_RESULT[m].type ;
		FD_RESULT[m].x0         = TEMP_FD_RESULT[m].x0 ;
		FD_RESULT[m].y0         = TEMP_FD_RESULT[m].y0 ;
		FD_RESULT[m].x1         = TEMP_FD_RESULT[m].x1 ;
		FD_RESULT[m].y1         = TEMP_FD_RESULT[m].y1 ;
		FD_RESULT[m].fcv        = TEMP_FD_RESULT[m].fcv ;
		FD_RESULT[m].rip_dir    = TEMP_FD_RESULT[m].rip_dir ;
		FD_RESULT[m].rop_dir    = TEMP_FD_RESULT[m].rop_dir ;
		FD_RESULT[m].size_index    = TEMP_FD_RESULT[m].size_index ;

		TC_RESULT[m].face_index = TEMP_TC_RESULT[m].face_index ;
		TC_RESULT[m].type       = TEMP_TC_RESULT[m].type ;
		TC_RESULT[m].LeftR      = TEMP_TC_RESULT[m].LeftR ;
		TC_RESULT[m].LeftG      = TEMP_TC_RESULT[m].LeftG ;
		TC_RESULT[m].LeftB      = TEMP_TC_RESULT[m].LeftB ;
		TC_RESULT[m].CenterR    = TEMP_TC_RESULT[m].CenterR ;
		TC_RESULT[m].CenterG    = TEMP_TC_RESULT[m].CenterG ;
		TC_RESULT[m].CenterB    = TEMP_TC_RESULT[m].CenterB ;
		TC_RESULT[m].RightR     = TEMP_TC_RESULT[m].RightR ;
		TC_RESULT[m].RightG     = TEMP_TC_RESULT[m].RightG ;
		TC_RESULT[m].RightB     = TEMP_TC_RESULT[m].RightB ;
	}

	for (m = 0 ; m < MAX_TRACKING_FACE_NUM ; m ++)
	{

		if (FD_RESULT[m].type == 0 || FD_RESULT[m].type == 1) // GFD or LFD_pass
		{
			display_flag[m] = 1 ;
		}
		else    // LFD_fail
		{
			display_flag[m] = 0 ;
		}

		if (FD_RESULT[m].type == 1) // LFD_pass
		{
			(Continuous_LFD_tracking_count[m]) ++;
		}

		if (FD_RESULT[m].type == 2) // LFD_fail
		{
			execute_skin_color_track[m] = 1 ;
		}
	}

	Box_tracking_face_number = 0 ;

	for (m = 0 ; m < MAX_TRACKING_FACE_NUM ; m ++)
	{
		if (Continuous_LFD_tracking_count[m] <= 2 && execute_skin_color_track[m] == 1)
		{
			Continuous_LFD_tracking_count[m] = 0 ;
			execute_skin_color_track[m]      = 0 ;
		}

		// // remove the redundant condition
		// if (Continuous_LFD_tracking_count[m] <= 2)
		// {
		// 	execute_skin_color_track[m] = 0 ;
		// }

		if (execute_skin_color_track[m] == 1)
		{
			(Non_LFD_tracking_count[m]) ++ ;
		}
		else
		{
			Non_LFD_tracking_count[m] = 0 ;
		}

		if (Non_LFD_tracking_count[m] >= TRACKING_TIMES_LIMITATION)
		{
			execute_skin_color_track[m] = 0 ;
			display_flag[m] = 0 ;

			FD_RESULT[m].x0   = 0 ;
			FD_RESULT[m].y0   = 0 ;
			FD_RESULT[m].x1   = 0 ;
			FD_RESULT[m].y1   = 0 ;
			FD_RESULT[m].type = 3 ;

			AVG_DIV_RG[m] = 0.0 ;
			AVG_R_VALUE[m] = 0 ;
			AVG_G_VALUE[m] = 0 ;
			AVG_B_VALUE[m] = 0 ;
			Face_tracked_num_count[m] = 0 ;
			Face_reliabiliy_value[m] = 0 ;
		}


		if (FD_RESULT[m].type == 0) // FD
		{   
			AVG_DIV_RG[m] = (float)TC_RESULT[m].CenterR / (float)TC_RESULT[m].CenterG ;	
			AVG_R_VALUE[m] = TC_RESULT[m].CenterR ;		
			AVG_G_VALUE[m] = TC_RESULT[m].CenterG ;			
			AVG_B_VALUE[m] = TC_RESULT[m].CenterB ;		
			Face_tracked_num_count[m] = 1 ;	
		}
		else if (FD_RESULT[m].type == 1) // LFD
		{
			AVG_DIV_RG[m] = (float)((AVG_DIV_RG[m] + ((float)TC_RESULT[m].CenterR / (float)TC_RESULT[m].CenterG)) / 2.0);	
			AVG_R_VALUE[m] = (AVG_R_VALUE[m] + TC_RESULT[m].CenterR) >> 1 ;		
			AVG_G_VALUE[m] = (AVG_G_VALUE[m] + TC_RESULT[m].CenterG) >> 1 ;			
			AVG_B_VALUE[m] = (AVG_B_VALUE[m] + TC_RESULT[m].CenterB) >> 1 ;	

		// Vibration avoidance part
		if (( abs(FD_RESULT[m].x0 - Box_posi_x0[m]) <= BOX_VIBRATION ) &&
		( abs(FD_RESULT[m].y0 - Box_posi_y0[m]) <= BOX_VIBRATION ) &&
		( abs(FD_RESULT[m].x1 - Box_posi_x1[m]) <= BOX_VIBRATION ) &&
		( abs(FD_RESULT[m].y1 - Box_posi_y1[m]) <= BOX_VIBRATION )
		)
		{
			Box_display_position_update_flag[m] = 0 ;
		}
		}
		else if (FD_RESULT[m].type == 2) // TC
		{
			if (execute_skin_color_track[m] == 0)
			{
				display_flag[m] = 0 ;
				AVG_DIV_RG[m] = 0.0 ;
				AVG_R_VALUE[m] = 0 ;
				AVG_G_VALUE[m] = 0 ;
				AVG_B_VALUE[m] = 0 ;
				Face_reliabiliy_value[m] = 0 ;
				Face_tracked_num_count[m] = 0 ;

				FD_RESULT[m].x0 = 0 ;
				FD_RESULT[m].y0 = 0 ;
				FD_RESULT[m].x1 = 0 ;
				FD_RESULT[m].y1 = 0 ;
				FD_RESULT[m].type = 3 ;
				FD_RESULT[m].face_index = MAX_FACE_NUM - 1 ;
				FD_RESULT[m].size_index = 0 ;
				FD_RESULT[m].fcv = 0 ;
				FD_RESULT[m].rip_dir = 0 ;
				FD_RESULT[m].rop_dir = 0 ;

				TC_RESULT[m].CenterR = 0 ;
				TC_RESULT[m].CenterG = 0 ;
				TC_RESULT[m].CenterB = 0 ;
				TC_RESULT[m].LeftR = 0 ;
				TC_RESULT[m].LeftG = 0 ;
				TC_RESULT[m].LeftB = 0 ;
				TC_RESULT[m].RightR = 0 ;
				TC_RESULT[m].RightG = 0 ;
				TC_RESULT[m].RightB = 0 ;
			}
			else
			{                            
				TC_x0   = FD_RESULT[m].x0 ;
				TC_y0   = FD_RESULT[m].y0 ;
				TC_x1   = FD_RESULT[m].x1 ;
				TC_y1   = FD_RESULT[m].y1 ;
				TC_type = FD_RESULT[m].type ;

				TRACKING_COMPENSATION(
									TC_RESULT[m].CenterR,
									TC_RESULT[m].CenterG,
									TC_RESULT[m].CenterB,
									TC_RESULT[m].LeftR,
									TC_RESULT[m].LeftG,
									TC_RESULT[m].LeftB,
									TC_RESULT[m].RightR,
									TC_RESULT[m].RightG,
									TC_RESULT[m].RightB,
									&TC_x0,
									&TC_y0,
									&TC_x1,
									&TC_y1,
									&TC_type,
									FD_RESULT[m].rip_dir,
									AVG_DIV_RG[m],
									AVG_R_VALUE[m],
									AVG_G_VALUE[m],
									AVG_B_VALUE[m],
									&TC_execute_skin_color_track,
									&TC_Face_reliabiliy_value,
									&BOX_COLOR_TRACK_POSITION) ;

				FD_RESULT[m].x0   = TC_x0 ;
				FD_RESULT[m].y0   = TC_y0 ;
				FD_RESULT[m].x1   = TC_x1 ;
				FD_RESULT[m].y1   = TC_y1 ;
				FD_RESULT[m].type = TC_type ;

				execute_skin_color_track[m] = TC_execute_skin_color_track ;
				Face_reliabiliy_value[m]    = TC_Face_reliabiliy_value ;

				if (FD_RESULT[m].type == 2)
				{
					display_flag[m] = 1 ;
					Box_tracking_face_number ++ ;
				}

				if (FD_RESULT[m].type == 3)
				{
					display_flag[m] = 0 ;
					AVG_DIV_RG[m] = 0.0 ;
					AVG_R_VALUE[m] = 0 ;
					AVG_G_VALUE[m] = 0 ;
					AVG_B_VALUE[m] = 0 ;
					Face_tracked_num_count[m] = 0 ;
					Face_reliabiliy_value[m] = 0 ;
					FD_RESULT[m].x0 = 0 ;
					FD_RESULT[m].y0 = 0 ;
					FD_RESULT[m].x1 = 0 ;
					FD_RESULT[m].y1 = 0 ;
				}
			}
		}
		else
		{
			FD_RESULT[m].x0 = 0 ;
			FD_RESULT[m].y0 = 0 ;
			FD_RESULT[m].x1 = 0 ;
			FD_RESULT[m].y1 = 0 ;

			FD_RESULT[m].type = 3 ;
			FD_RESULT[m].face_index = MAX_FACE_NUM - 1 ;
			FD_RESULT[m].size_index = 0 ;
			FD_RESULT[m].fcv = 0 ;
			FD_RESULT[m].rip_dir = 0 ;
			FD_RESULT[m].rop_dir = 0 ;

			TC_RESULT[m].CenterR = 0 ;
			TC_RESULT[m].CenterG = 0 ;
			TC_RESULT[m].CenterB = 0 ;
			TC_RESULT[m].LeftR = 0 ;
			TC_RESULT[m].LeftG = 0 ;
			TC_RESULT[m].LeftB = 0 ;
			TC_RESULT[m].RightR = 0 ;
			TC_RESULT[m].RightG = 0 ;
			TC_RESULT[m].RightB = 0 ;

			AVG_DIV_RG[m] = 0.0 ;
			AVG_R_VALUE[m] = 0 ;
			AVG_G_VALUE[m] = 0 ;
			AVG_B_VALUE[m] = 0 ;
			Face_reliabiliy_value[m] = 0 ;
			display_flag[m] = 0 ;
		}
	}

	Color_compensate_face_number = Box_tracking_face_number ;
				
	for(m = 0 ; m < MAX_TRACKING_FACE_NUM ; m ++)
	{
		if (display_flag[m] == 0)
		{
			Continuous_LFD_tracking_count[m] = 0 ;
		}
	}

	Face_Overlap_Detection(MAX_TRACKING_FACE_NUM, display_flag,
							Face_reliabiliy_value, FD_RESULT) ;	 
				
	if ((GFD_NUM + LFD_NUM + Color_compensate_face_number) >= 1 )
	{
		FACE_PRIORITY_SORTING(  IMAGE_WIDTH, IMAGE_HEIGHT,
								MAX_TRACKING_FACE_NUM, AF_Face_Indicator,
								display_flag, FD_RESULT) ;
	}
	else
	{
		for (m = 0 ; m < MAX_TRACKING_FACE_NUM ; m ++)
		{
			AF_Face_Indicator[m] = 0 ;
		}
	}
    
	Smoother(MAX_TRACKING_FACE_NUM, Box_posi_x0, Box_posi_y0, Box_posi_x1,
				Box_posi_y1, Box_pdiff_x0, Box_pdiff_y0, Box_display_position_update_flag,
                      	SIZE_SMOOTH_FACTOR, SMOOTH_POSITION_THRESHOLD,
                    	TRIGGER_POSITION_THRESHOLD, display_flag, FD_RESULT) ;			

        //LOGD("Smoother TRIGGER_POSITION_THRESHOLD %f \n",TRIGGER_POSITION_THRESHOLD);
	//----------------------------------------------------//
	//                    Box draw part                   //
	//----------------------------------------------------//
		
	//FDE_Result_Output( &fde_out, &bmp_in_320x240,//&curr_frame,
	//				MAX_TRACKING_FACE_NUM, display_flag,
	//				Continuous_LFD_tracking_count, UNSTABLE_NUMBER,
	//				AF_Face_Indicator, TrackingFaceType,
	//				GFD_INFO.pose, //Current_Feature_Index,
	//				OMNI_DIRECTION_QUEUE,
	//				FULL_DET_DIR_DISPLAY_FLAG,
	//				Box_posi_x0, Box_posi_y0,Box_posi_x1,Box_posi_y1,	
	//				GFD_SKIP_MODE,GFD_PATTERN_TYPE,RIP_SUPPORT,0,FD_RESULT) ;

	for (m = 0 ; m < 13 ; m ++)
	{
		FULL_DET_DIR_DISPLAY_FLAG[m] = 0 ;
	}
	
    //*****************Binchang 20110728 remove fd_result_struct*****************************//
	for (m = 0 ; m < MAX_FACE_NUM ; m ++)
	{
		FD_RESULT[m].af_face_indicator=(FDBOOL)AF_Face_Indicator[m];
	}
	//******************************************************************************************************//
	
	memcpy((void*)FDVTResult_buffer, (void*)FD_RESULT, sizeof(result)*MAX_FACE_NUM);   
	
	g_wDisplay_flag = display_flag;
	
    }   
    #ifdef SmileDetect    
    else if (fd_state == FDVT_SD_MODE)
    {
        int tmpsd01;
        int sdcenterx=0;
        int sdcentery=0;
        int sdhalfwidth     ;  
        int SD_TARGET_NUM   ;
        int CountTH;
        
        //Only for main face
        //SD_TARGET_NUM = LFD_NUM + GFD_NUM  ;
        if((GFD_NUM==0) && (LFD_NUM==0))
            return;
        
        SD_TARGET_NUM = (GFD_NUM+LFD_NUM);
        //------------------------------------------------------------------------------
        //	Generate LFD target list.
	//------------------------------------------------------------------------------
        SD_LFD_WINDOW_NUM = 0  ;
        for( i = 0 ; i < SD_TARGET_NUM ; i++ ) {
            SD_LFD_INFO[SD_LFD_WINDOW_NUM].valid       = 1  ;
            SD_LFD_INFO[SD_LFD_WINDOW_NUM].face_index  = SD_LFD_WINDOW_NUM  ;
            
            if ( FD_RESULT[i].x1 - FD_RESULT[i].x0 < 32 ) // Skip small face.
                continue  ;
            
            if(AF_Face_Indicator[i] ==0) //Skip Non-focus window
            	 continue  ;

            tmpsd01 = FD_RESULT[i].size_index + 3       ;   // chris 0929, change from +4 to +3
            tmpsd01 = ( tmpsd01 < 0 ) ? 0 : tmpsd01     ;
            tmpsd01 = ( tmpsd01 >= g_rs_num ) ? g_rs_num-1 : tmpsd01  ;
            SD_LFD_INFO[SD_LFD_WINDOW_NUM].size_index = tmpsd01         ;
            SD_LFD_INFO[SD_LFD_WINDOW_NUM].rip_dir = POSE_TRANS_RHW2FW(FD_RESULT[ i ].rip_dir)      ;
            SD_LFD_INFO[SD_LFD_WINDOW_NUM].rop_dir = POSE_TRANS_RHW2FW(FD_RESULT[ i ].rop_dir)      ;
            //LOGD("FDVTMain Smile FD_RESULT[ i ].rip_dir 0x%d SD_LFD_INFO[SD_LFD_WINDOW_NUM].rip_dir 0x%d\n",FD_RESULT[ i ].rip_dir,SD_LFD_INFO[SD_LFD_WINDOW_NUM].rip_dir);
            // 20091029Hori : Estimate search region according to face pose.
            switch ( SD_LFD_INFO[SD_LFD_WINDOW_NUM].rip_dir ) {            
                // In case of RIP000.
                case  0 :
                case  1 :
                case 11 :
                    sdcenterx   = ( FD_RESULT[i].x0 + FD_RESULT[i].x1     ) / 2		;
                    sdcentery   = ( FD_RESULT[i].y0 + FD_RESULT[i].y1 * 2 ) / 3		;
                break  ;
                
                // In case of RIP090.
                case  2 :
                case  3 :
                case  4 :
                    sdcenterx   = ( FD_RESULT[i].x1 + FD_RESULT[i].x0 * 2 ) / 3		;
                    sdcentery   = ( FD_RESULT[i].y0 + FD_RESULT[i].y1     ) / 2		;
                break  ;
                
                // In case of RIP180.
                case  5 :
                case  6 :
                case  7 :
                    sdcenterx   = ( FD_RESULT[i].x0 + FD_RESULT[i].x1     ) / 2		;
                    sdcentery   = ( FD_RESULT[i].y1 + FD_RESULT[i].y0 * 2 ) / 3		;
                break  ;
                
                // In case of RIP270.
                case  8 :
                case  9 :
                case 10 :
                    sdcenterx   = ( FD_RESULT[i].x0 + FD_RESULT[i].x1 * 2 ) / 3		;
                    sdcentery   = ( FD_RESULT[i].y0 + FD_RESULT[i].y1     ) / 2		;
                break  ;
            
            }
            //int sfactor=fd_config_datava[(SD_LFD_INFO[SD_LFD_WINDOW_NUM].size_index*4)+2]&0x0FFF;
            //LOGD("FDVTMain sfactor 0x%x\n",sfactor);
            sdhalfwidth = ((fd_config_datava[(SD_LFD_INFO[SD_LFD_WINDOW_NUM].size_index*4)+2]&0x0FFF) * 12 ) >> 9  ;
            
            SD_LFD_INFO[SD_LFD_WINDOW_NUM].x0 = sdcenterx - sdhalfwidth                     ;
            SD_LFD_INFO[SD_LFD_WINDOW_NUM].x1 = sdcenterx + sdhalfwidth                     ;
            
            SD_LFD_INFO[SD_LFD_WINDOW_NUM].y0 = sdcentery - sdhalfwidth                     ;
            SD_LFD_INFO[SD_LFD_WINDOW_NUM].y1 = sdcentery + sdhalfwidth                     ;
            
            SD_LFD_INFO[SD_LFD_WINDOW_NUM].y0 = SD_LFD_INFO[SD_LFD_WINDOW_NUM].y0>IMAGE_HEIGHT? IMAGE_HEIGHT - sdhalfwidth*2 : SD_LFD_INFO[SD_LFD_WINDOW_NUM].y0    ;
            SD_LFD_INFO[SD_LFD_WINDOW_NUM].y1 = SD_LFD_INFO[SD_LFD_WINDOW_NUM].y1>IMAGE_HEIGHT? IMAGE_HEIGHT : SD_LFD_INFO[SD_LFD_WINDOW_NUM].y1                                        ;
            
            //SD_LFD_INFO[SD_LFD_WINDOW_NUM].rip_dir = POSE_TRANS_RHW2FW(FD_RESULT[ i ].rip_dir)    ;
                        
            SD_LFD_WINDOW_NUM ++  ;
        
        }
        memcpy(learning_data0va,learning_data0_sd_24x24,sizeof(unsigned int)*FNUM_BSDLEARNING_DATA0);
        memcpy(learning_data1va,learning_data1_sd_24x24,sizeof(unsigned int)*FNUM_BSDLEARNING_DATA1);
        memcpy(learning_data2va,learning_data2_sd_24x24,sizeof(unsigned int)*FNUM_BSDLEARNING_DATA2);
        memcpy(learning_data3va,learning_data3_sd_24x24,sizeof(unsigned int)*FNUM_BSDLEARNING_DATA3);
        memcpy(learning_data4va,learning_data4_sd_24x24,sizeof(unsigned int)*FNUM_BSDLEARNING_DATA4);
        memcpy(learning_data5va,learning_data5_sd_24x24,sizeof(unsigned int)*FNUM_BSDLEARNING_DATA5);
        memcpy(learning_data6va,learning_data6_sd_24x24,sizeof(unsigned int)*FNUM_BSDLEARNING_DATA6);
        memcpy(learning_data7va,learning_data7_sd_24x24,sizeof(unsigned int)*FNUM_BSDLEARNING_DATA7);
        SDRegisterConfig((void*)image_buffer_address);
        #if defined (MTK_M4U_SUPPORT)        
        SyncM4UCache(mWorkingBuffer);
        SyncM4UCache(mLearningData);
        #endif
        halFDVT_StartHW((void*)image_buffer_address);
        FDGetHWResult(fd_state);
        
        //20110511 BinChang Do Second Pass SD
        if(SD_NUM ==0)
        {
            memcpy(learning_data0va,learning_data0_sd_24x24,sizeof(unsigned int)*FNUM_BSDLEARNING_DATA0);
            memcpy(learning_data1va,learning_data1_sd_24x24,sizeof(unsigned int)*FNUM_BSDLEARNING_DATA1);
            memcpy(learning_data2va,learning_data5_sd_24x24,sizeof(unsigned int)*FNUM_BSDLEARNING_DATA5);
            memcpy(learning_data3va,learning_data6_sd_24x24,sizeof(unsigned int)*FNUM_BSDLEARNING_DATA6);
            memcpy(learning_data4va,learning_data7_sd_24x24,sizeof(unsigned int)*FNUM_BSDLEARNING_DATA7);
            memcpy(learning_data5va,learning_data2_sd_24x24,sizeof(unsigned int)*FNUM_BSDLEARNING_DATA2);
            memcpy(learning_data6va,learning_data3_sd_24x24,sizeof(unsigned int)*FNUM_BSDLEARNING_DATA3);
            memcpy(learning_data7va,learning_data4_sd_24x24,sizeof(unsigned int)*FNUM_BSDLEARNING_DATA4);
            //SDRegisterConfig((void*)image_buffer_address);
            #if defined (MTK_M4U_SUPPORT)
            SyncM4UCache(mWorkingBuffer);
            SyncM4UCache(mLearningData);
            #endif
            halFDVT_StartHW((void*)image_buffer_address);
            FDGetHWResult(fd_state);
        }

        memcpy(learning_data0va,learning_data0,sizeof(unsigned int)*FNUM_LEARNING_DATA0);
        memcpy(learning_data1va,learning_data1,sizeof(unsigned int)*FNUM_LEARNING_DATA1);
        memcpy(learning_data2va,learning_data2,sizeof(unsigned int)*FNUM_LEARNING_DATA2);
        memcpy(learning_data3va,learning_data3,sizeof(unsigned int)*FNUM_LEARNING_DATA3);
        memcpy(learning_data4va,learning_data4,sizeof(unsigned int)*FNUM_LEARNING_DATA4);
        memcpy(learning_data5va,learning_data5,sizeof(unsigned int)*FNUM_LEARNING_DATA5);
        memcpy(learning_data6va,learning_data6,sizeof(unsigned int)*FNUM_LEARNING_DATA6);
        memcpy(learning_data7va,learning_data7,sizeof(unsigned int)*FNUM_LEARNING_DATA7); 
        
        if((GFD_NUM+LFD_NUM) <3)
        	     CountTH = g_CountTH;
        else
        	     CountTH = 1;
        
        //Smooth Smile Detection
        if(SD_NUM !=0)
        {
        	   FrameCount++;
        	   if( FrameCount  < CountTH)
        	       SD_NUM=0;
        	   else
        	   	  FrameCount = 0;
        }	
        else
        	   FrameCount = 0;
        
        LOGD("SD_NUM:%d,  FrameCount:%d, CountTH:%d\n", SD_NUM, FrameCount, CountTH);
        
    }
    #endif
    
    #ifdef LOGON
    LOGD("FDVTMain OUT \n");
    #endif
}

void AppFDVT::FDVTReset(void)
{
    #ifdef LOGON
    LOGD("FDVTReset IN \n");
    #endif
       
    for (int m = 0 ; m < MAX_TRACKING_FACE_NUM ; m ++)
    {
        FD_RESULT[m].face_index = MAX_FACE_NUM - 1 ; 
        FD_RESULT[m].type = 3 ;
        FD_RESULT[m].x0 = 0 ;
        FD_RESULT[m].y0 = 0 ;
        FD_RESULT[m].x1 = 0 ;
        FD_RESULT[m].y1 = 0 ;
        FD_RESULT[m].fcv = 0 ;
        FD_RESULT[m].rip_dir = 0 ;
        FD_RESULT[m].rop_dir = 0 ;
        FD_RESULT[m].size_index = 0 ;
    }
    for (int m = 0 ; m < MAX_FACE_CANDIDATE_NUM ; m ++)
    {
        Box_posi_x0[m] = 0;
        Box_posi_y0[m] = 0;	// Position of the face candidates
        Box_posi_x1[m] = 0;
        Box_posi_y1[m] = 0;	// Position of the face candidates
        Box_pdiff_x0[m] = 0;
        Box_pdiff_y0[m] = 0;   
    }
    #ifdef LOGON
    LOGD("FDVTReset OUT \n");
    #endif

}

kal_uint32 AppFDVT::FDVTGetResultSize(void)
{
    //return sizeof(fdvt_result_struct);
    return 0;
}


void AppFDVT::FDVTGetFDInfo(kal_uint32 a_FD_Info_Result)
{
	MINT32 i, INDEX_NUM;
	
	mtk_face_info_m* FD_Info_Result = (mtk_face_info_m*)a_FD_Info_Result;
	
	INDEX_NUM = 0;
	
	if(g_wDisplay_flag != NULL)
	{	
		for (i = 0 ; i < (MAX_TRACKING_FACE_NUM); i++) 
	     {
	         if(g_wDisplay_flag[i] == 0)
	         {
	             continue;
	         }
	         
	         FD_Info_Result[INDEX_NUM].rop_dir =  FDVTResult_buffer[i].rop_dir;
	         FD_Info_Result[INDEX_NUM].rip_dir =  FDVTResult_buffer[i].rip_dir;
	         //FD_Info_Result[INDEX_NUM].rop_dir =  POSE_TRANS_RFW2HW(FDVTResult_buffer[i].rop_dir);
	         //FD_Info_Result[INDEX_NUM].rip_dir =  POSE_TRANS_RFW2HW(FDVTResult_buffer[i].rip_dir);
	         INDEX_NUM++;
	     }
     }

    for (i = INDEX_NUM ; i < (MAX_TRACKING_FACE_NUM); i++)
    {
    	    FD_Info_Result[i].rop_dir = 0;
    	    FD_Info_Result[i].rip_dir = 0;
    }	
	
	
}

void AppFDVT::FDVTGetICSResult(kal_uint32 a_FD_ICS_Result, kal_uint32 a_FD_Results, MUINT32 Width,MUINT32 Height, MUINT32 LCM, MUINT32 Sensor, MUINT32 Camera_TYPE, MUINT32 Draw_TYPE)
{
	
	MINT32 i, WINDOW_NUM;
	MINT32 Bufx0,Bufx1,Bufy0,Bufy1;
	
	camera_face_metadata_m* FD_ICS_Result = (camera_face_metadata_m*)a_FD_ICS_Result;
	result* FD_Results=(result*)a_FD_Results;  
	
	//LOGD("LCM:%d,  Sensor:%d,   Camera_TYPE:%d, Draw_TYPE:%d, \n", LCM, Sensor, Camera_TYPE, Draw_TYPE);
	
	//***************************************************************************//
	//** Draw Type: 0 MDP for FD -->  MDP for Preview (Sensor) **//
	//***************************************************************************//
	//** Draw Type: 1 MDP for FD -->  Display                                   **//
	//***************************************************************************//
	
	WINDOW_NUM = 0;
	
	if(g_wDisplay_flag != NULL)
	{
	    for (i = 0 ; i < (MAX_TRACKING_FACE_NUM); i++) 
	    {
	         if(g_wDisplay_flag[i] == 0)
	         {
	             continue;  
	         }
	         
		   switch(Draw_TYPE)
		   {
		   	   case 0:
		   	   default:
		   	   if(Width>=Height)
		   	   {
		        	   Bufx0 = (FD_Results[i].x0*Width)/IMAGE_WIDTH;
		        	   Bufx1 = (FD_Results[i].x1*Width)/IMAGE_WIDTH;
		        	   Bufy0 = (FD_Results[i].y0*Height)/IMAGE_HEIGHT;
		        	   Bufy1 = (FD_Results[i].y1*Height)/IMAGE_HEIGHT;
		   	   }
		   	   else
		   	   {
		        	   Bufx0 = (FD_Results[i].y0*Width)/IMAGE_HEIGHT;
		        	   Bufx1 = (FD_Results[i].y1*Width)/IMAGE_HEIGHT;
		        	   Bufy0 = Height - 1 - ((FD_Results[i].x1*Height)/IMAGE_WIDTH);
		        	   Bufy1 = Height - 1 - ((FD_Results[i].x0*Height)/IMAGE_WIDTH);
		   	    }
		        break;
		  }
		  
		  //轉換到 (-1000,-1000) ~ (+1000, +1000) 座標系
		  FD_ICS_Result->faces[WINDOW_NUM].rect[0] = Bufx0*2000/Width - 1000;
		  FD_ICS_Result->faces[WINDOW_NUM].rect[1] = Bufy0*2000/Height - 1000;
		  FD_ICS_Result->faces[WINDOW_NUM].rect[2] = Bufx1*2000/Width - 1000;
		  FD_ICS_Result->faces[WINDOW_NUM].rect[3] = Bufy1*2000/Height - 1000;
		  
		  if(AF_Face_Indicator[i]!=0)
	           FD_ICS_Result->faces[WINDOW_NUM].score = 100;
	       else
	         	 FD_ICS_Result->faces[WINDOW_NUM].score = 90;
	       
		  FD_ICS_Result->faces[WINDOW_NUM].id = 0; 
	       FD_ICS_Result->faces[WINDOW_NUM].left_eye[0] = -2000;
	       FD_ICS_Result->faces[WINDOW_NUM].left_eye[1] = -2000;
	       FD_ICS_Result->faces[WINDOW_NUM].right_eye[0] = -2000;
	       FD_ICS_Result->faces[WINDOW_NUM].right_eye[1] = -2000;
	       FD_ICS_Result->faces[WINDOW_NUM].mouth[0] = -2000;
	       FD_ICS_Result->faces[WINDOW_NUM].mouth[1] = -2000;
		  //
		  WINDOW_NUM ++;
	    }
    }
    
    //FD_ICS_Result->number_of_faces = FD_Results[0].face_num;
    FD_ICS_Result->number_of_faces = WINDOW_NUM;
    
    //LOGD("FD_NUM1:%d,  FD_NUM2:%d  \n", FD_Results[0].face_num, WINDOW_NUM);
    
    for (i = WINDOW_NUM ; i < (MAX_TRACKING_FACE_NUM); i++) 
    {
        FD_ICS_Result->faces[i].rect[0] = 0;
        FD_ICS_Result->faces[i].rect[1] = 0;
        FD_ICS_Result->faces[i].rect[2] = 0;
        FD_ICS_Result->faces[i].rect[3] = 0;
        FD_ICS_Result->faces[i].score = 0;
        FD_ICS_Result->faces[i].id = 0;
        FD_ICS_Result->faces[i].left_eye[0] = -2000;
        FD_ICS_Result->faces[i].left_eye[1] = -2000;
        FD_ICS_Result->faces[i].right_eye[0] = -2000;
        FD_ICS_Result->faces[i].right_eye[1] = -2000;
        FD_ICS_Result->faces[i].mouth[0] = -2000;
        FD_ICS_Result->faces[i].mouth[1] = -2000;
     }
    
}

kal_uint8 AppFDVT::FDVTGetResult(kal_uint32 FD_result_Adr)
{
    #ifdef LOGON
    LOGD("FDVTGetResult IN \n");
    #endif
    #if 0
    result fd_result[3];
    result tmp;
     
    fd_result[0].x0 = 5;
    fd_result[0].y0 = 10;
    fd_result[0].x1 = 35;
    fd_result[0].y1 = 40;

    fd_result[1].x0 = 140;
    fd_result[1].y0 = 110;
    fd_result[1].x1 = 170;
    fd_result[1].y1 = 140;

    fd_result[2].x0 = 280;
    fd_result[2].y0 = 200;
    fd_result[2].x1 = 310;
    fd_result[2].y1 = 230;

    memset((void*)FDVTResult_buffer, 0, sizeof(result) * MAX_FACE_NUM);
    memcpy((void*)FDVTResult_buffer, (void*)fd_result, sizeof(result)*3);
    return 3;
    #else
    memset((void*)FD_result_Adr, 0, sizeof(result) * MAX_FACE_NUM);
    
    //*****************Binchang 20110728 remove fd_result_struct*****************************//
    result* tmp=(result*)FD_result_Adr;        
    memcpy((void*)tmp, (void*)FDVTResult_buffer, sizeof(result)*MAX_FACE_NUM);
    memcpy((void*)FDVTResult2AP, (void*)FDVTResult_buffer, sizeof(result)*MAX_FACE_NUM);
    
    for(int i=0;i<MAX_FACE_NUM;i++)
    {
        tmp[i].face_num=LFD_NUM + GFD_NUM+Color_compensate_face_number;
        FDVTResult2AP[i].face_num = tmp[i].face_num;
        //LOGD(" i %d x0 %d y0 %d x1 %d y1 %d af %d \n",i,tmp[i].face_display_pos_x0,tmp[i].face_display_pos_y0,tmp[i].face_display_pos_x1,tmp[i].face_display_pos_y1,tmp[i].af_face_indicator);
    }
    
    //halDumpReg();
         
    LOGD("FDVTGet FaceNum: %d \n", LFD_NUM + GFD_NUM+Color_compensate_face_number);      
         
    //return (LFD_NUM + GFD_NUM);
   //BinChang 20110315 Add TC Number
    if((LFD_NUM + GFD_NUM+Color_compensate_face_number) > 0)
    	   return  1;
    else
    	   return  0;    
    
    #endif
    
    #ifdef LOGON
    LOGD("FDVTGetResult OUT \n");
    #endif
} 
#ifdef SmileDetect
kal_uint8 AppFDVT::FDVTGetSDResult(kal_uint32 FD_result_Adr)
{   
    //*****************Binchang 20111207 Modify Result Structure for ICS4.0*****************************//
    if(SD_NUM!=0)
    {
        SD_NUM=0;
        return 1;
    }
    else
    {
        return 0;
    }
}
#endif

void AppFDVT::FDVTDrawFaceRect(MUINT32 image_buffer_address,MUINT32 Width,MUINT32 Height,MUINT32 OffsetW,MUINT32 OffsetH,MUINT8 orientation)
{
    
    kal_uint16 *display_buffer_ptr = (kal_uint16*)image_buffer_address;
    result *fd_result_data = (result *)FDVTResult2AP;
    result tmp;
    MUINT32 FDwidth;
    MUINT32 FDHeight;
    
    MUINT32 Bufx0,Bufx1,Bufy0,Bufy1;
    MUINT8 Cnt = 0;
    MUINT32 iWidth;
    MUINT32 iHeight;
    MUINT8 ucSensorFlag, ucLcmFlag, ucSensorType;
    MUINT32 UpdateX0,UpdateX1, UpdateY0, UpdateY1;
    
    #ifdef LOGON
    LOGD("FDVTDrawFaceRect in\n");   
    #endif
    //LOGE("image_buffer_address 0x%x ImageW %d ImageH %d OffsetW %d OffsetH %d Img %d\n",(MUINT32)image_buffer_address,Width,Height,OffsetW,OffsetH,IMAGE_HEIGHT);
    
    iWidth=Width;
    iHeight=Height;
    ucSensorFlag    = (orientation & 0xE0) >> 5;
    ucLcmFlag = (orientation & 0x1C) >> 2;
    ucSensorType = orientation & 0x03;

    //ucLcmFlag = 1;
    
    //LOGD("[FDdraw] GFD_NUM %d LFD_NUM = %d Color_compensate_face_number = %d  \n",GFD_NUM,LFD_NUM,Color_compensate_face_number); 
    //LOGD("[FDdraw]  FD_W = %d, FD_H = %d, Disp_W = %d,  Disp_H=%d, Sen_Flag=%d, LCM_Flag=%d, Sen_Type=%d, \n",IMAGE_WIDTH,IMAGE_HEIGHT,iWidth,iHeight,ucSensorFlag, ucLcmFlag, ucSensorType); 
        
    memset((void*)image_buffer_address,0,Width*Height*2);
    //for (int i = 0 ; i < (LFD_NUM + GFD_NUM+Color_compensate_face_number); i++) {
    
    //BinChang 20120314
     if(g_wDisplay_flag != NULL)
     {
	    	for (int i = 0 ; i < (MAX_TRACKING_FACE_NUM); i++) {
	    	  if(g_wDisplay_flag[i] == 0)
	    		continue;
	    		
	    	tmp=fd_result_data[i];
	    	/*
	    	if (tmp.display_flag == FALSE ) {
	            continue;
	        }*/
	        Cnt++;
	        
	        //BinChang20110411 Separate Front & Back Cam.        
	        //LOGD("[FDdraw] i=%d,  x0 = %d, y0 = %d, x1 = %d, y1 = %d, OffsetW = %d, OffsetH = %d, \n",i,  tmp.x0, tmp.y0,tmp.x1,tmp.y1, OffsetW, OffsetH);
	        
	        //************************************************************************************************//
	        //**************       ucLcmFlag:        73Demo/JxxPhone = 1  (F/B)        ***************//
	        //**************                                      73Tablet / 73Cxx = 0       (F/B)        ***************//
	        //************************************************************************************************//
	        //*****      ucSensorFlag:  73DemoPhone:    Front Cam = 0, Back Cam = 1     ****//
	        //*****                                     73JxxPhone:      Front Cam = 3, Back Cam = 1     ****//
	        //********                                      73Tablet :   Front Cam = 3, Back Cam = 1      ********//
	        //********                                      73Cxx    :   Front Cam = 1, Back Cam = 0      ********//
	        //************************************************************************************************//
	        //************************************************************************************************//
	        
	        if(ucSensorType==0)  // Main-Back Sensor
	        {
	            switch(ucLcmFlag)
	            {
	                case 0:
	                case 2:
	                if( (ucSensorFlag ==0) ||  (ucSensorFlag ==3)) 
	                {	                    
	                    Bufx0=iWidth - 1 - (tmp.x1*iWidth)/IMAGE_WIDTH;
	                    Bufx1=iWidth - 1 - (tmp.x0*iWidth)/IMAGE_WIDTH;
	                    Bufy0=iHeight - 1 - (tmp.y1*iHeight)/IMAGE_HEIGHT;
	                    Bufy1=iHeight - 1 - (tmp.y0*iHeight)/IMAGE_HEIGHT;
	                }
	                else if( (ucSensorFlag ==1) ||  (ucSensorFlag ==2))
	                {
	                   Bufx0=(tmp.x0*iWidth)/IMAGE_WIDTH;
	                   Bufx1=(tmp.x1*iWidth)/IMAGE_WIDTH;
	                   Bufy0=(tmp.y0*iHeight)/IMAGE_HEIGHT;
	                   Bufy1=(tmp.y1*iHeight)/IMAGE_HEIGHT;
	                }
	                else
	                {
	                   Bufx0=(tmp.x0*iWidth)/IMAGE_WIDTH;
	                   Bufx1=(tmp.x1*iWidth)/IMAGE_WIDTH;
	                   Bufy0=(tmp.y0*iHeight)/IMAGE_HEIGHT;
	                   Bufy1=(tmp.y1*iHeight)/IMAGE_HEIGHT;
	                }
	                
	                FDwidth=(Bufx1-Bufx0)+1;
	                FDHeight=(Bufy1-Bufy0)+1;
	                //if((FDwidth!=(Bufy1-Bufy0))&&(FDwidth>(Bufy1-Bufy0)))
	                //    FDwidth=(Bufy1-Bufy0);
	                
	                Bufx0+=OffsetW;
	                Bufy0+=OffsetH;
	                Bufx1+=OffsetW;
	                Bufy1+=OffsetH;
	                if(Bufx1>(iWidth-OffsetW) || Bufy1>(iHeight-OffsetH) || (Bufx1*Bufy1)>(iWidth*iHeight)) 
	                {
	                    LOGE("[FDdraw] 0 FD position invalid i %d x0 %d x1 %d y0 %d y1 %d\n",i,Bufx0,Bufx1,Bufy0,Bufy1); 
	                    FDwidth=0;
	                    Bufx0=0;
	                    Bufx1=0;
	                    Bufy0=0; 
	                    Bufy1=0;     
	                }
	                break;       	   
	                case 1:
	                case 3:
	                if( (ucSensorFlag ==0) ||  (ucSensorFlag ==3))
	                {
	                    Bufx0= (tmp.y0*iWidth)/IMAGE_HEIGHT;
	                    Bufx1= (tmp.y1*iWidth)/IMAGE_HEIGHT;
	                    Bufy0= iHeight - 1 - ((tmp.x1*iHeight)/IMAGE_WIDTH);
	                    Bufy1= iHeight - 1 - ((tmp.x0*iHeight)/IMAGE_WIDTH);
	                }
	                else if( (ucSensorFlag ==1) ||  (ucSensorFlag ==2))
	                {
	                	Bufx1= iWidth - 1 - ((tmp.y0*iWidth)/IMAGE_HEIGHT);
	                    Bufx0= iWidth - 1 - ((tmp.y1*iWidth)/IMAGE_HEIGHT);
	                    Bufy0=(tmp.x0*iHeight)/IMAGE_WIDTH;
	                    Bufy1=(tmp.x1*iHeight)/IMAGE_WIDTH;
	                }
	                else
	                {
	                   Bufx1= iWidth - 1 - ((tmp.y0*iWidth)/IMAGE_HEIGHT);
	                   Bufx0= iWidth - 1 - ((tmp.y1*iWidth)/IMAGE_HEIGHT);
	                   Bufy0=(tmp.x0*iHeight)/IMAGE_WIDTH;
	                   Bufy1=(tmp.x1*iHeight)/IMAGE_WIDTH;
	                }
	                //LOGE("After zoom x0 = %d y0 = %d x1 = %d y1=%d \n",Bufx0,Bufy0,Bufx1,Bufy1);
	                     
	                FDwidth=(Bufx1-Bufx0)+1;
	                FDHeight=(Bufy1-Bufy0)+1;            
	                //if((FDwidth!=(Bufy1-Bufy0))&&(FDwidth>(Bufy1-Bufy0)))
	                //    FDwidth=(Bufy1-Bufy0);
	                
	                Bufx0+=OffsetH;
	                Bufy0+=OffsetW;
	                Bufx1+=OffsetH;
	                Bufy1+=OffsetW;
	                if(Bufx1>(iWidth-OffsetH) || Bufy1>(iHeight-OffsetW) || (Bufx1*Bufy1)>(iWidth*iHeight)) 
	                {
	                    LOGE("[FDdraw] 1 FD position invalid i %d x0 %d x1 %d y0 %d y1 %d\n",i,Bufx0,Bufx1,Bufy0,Bufy1); 
	                    FDwidth=0;
	                    Bufx0=0;
	                    Bufx1=0;
	                    Bufy0=0; 
	                    Bufy1=0;   
	                        
	                }
	                break;
	                default:
	                Bufx0=(tmp.x0*iWidth)/IMAGE_WIDTH;
	                Bufx1=(tmp.x1*iWidth)/IMAGE_WIDTH;
	                Bufy0=(tmp.y0*iHeight)/IMAGE_HEIGHT;    
	                Bufy1=(tmp.y1*iHeight)/IMAGE_HEIGHT;         
	                FDwidth=(Bufx1-Bufx0)+1;
	                FDHeight=(Bufy1-Bufy0)+1;
	                //if((FDwidth!=(Bufy1-Bufy0))&&(FDwidth>(Bufy1-Bufy0)))
	                //    FDwidth=(Bufy1-Bufy0);
	                
	                Bufx0+=OffsetW;
	                Bufy0+=OffsetH;
	                Bufx1+=OffsetW;
	                Bufy1+=OffsetH;
	                if(Bufx1>(iWidth-OffsetW) || Bufy1>(iHeight-OffsetH) || (Bufx1*Bufy1)>(iWidth*iHeight)) 
	                {
	                    LOGE("[FDdraw] 2 FD position invalid i %d x0 %d x1 %d y0 %d y1 %d\n",i,Bufx0,Bufx1,Bufy0,Bufy1); 
	                    FDwidth=0;
	                    Bufx0=0;
	                    Bufx1=0; 
	                    Bufy0=0; 
	                    Bufy1=0;    
	                }
	                break;
	           }
	        }
	        
	        else  // Sub-Front Sensor (ucSensorType=1)
	        {
	             switch(ucLcmFlag)
	            {
	                case 0:
	                case 2:
	                if( (ucSensorFlag ==0) ||  (ucSensorFlag ==3)) 
	                {	
	                    Bufx0=iWidth - 1 - (tmp.x1*iWidth)/IMAGE_WIDTH;
	                    Bufx1=iWidth - 1 - (tmp.x0*iWidth)/IMAGE_WIDTH;
	                    Bufy0=(tmp.y0*iHeight)/IMAGE_HEIGHT;
	                    Bufy1=(tmp.y1*iHeight)/IMAGE_HEIGHT;
	                }
	                else if( (ucSensorFlag ==1) ||  (ucSensorFlag ==2))
	                {
	                   Bufx0=(tmp.x0*iWidth)/IMAGE_WIDTH;
	                   Bufx1=(tmp.x1*iWidth)/IMAGE_WIDTH;
	                   Bufy0=iHeight - 1 - (tmp.y1*iHeight)/IMAGE_HEIGHT;
	                   Bufy1=iHeight - 1 - (tmp.y0*iHeight)/IMAGE_HEIGHT;
	                }
	                else
	                {
	                   Bufx0=(tmp.x0*iWidth)/IMAGE_WIDTH;
	                   Bufx1=(tmp.x1*iWidth)/IMAGE_WIDTH;
	                   Bufy0=(tmp.y0*iHeight)/IMAGE_HEIGHT;
	                   Bufy1=(tmp.y1*iHeight)/IMAGE_HEIGHT;
	                }
	                
	                FDwidth=(Bufx1-Bufx0)+1;
	                FDHeight=(Bufy1-Bufy0)+1;
	                //if((FDwidth!=(Bufy1-Bufy0))&&(FDwidth>(Bufy1-Bufy0)))
	                //    FDwidth=(Bufy1-Bufy0);
	                
	                Bufx0+=OffsetW;
	                Bufy0+=OffsetH;
	                Bufx1+=OffsetW;
	                Bufy1+=OffsetH;
	                if(Bufx1>(iWidth-OffsetW) || Bufy1>(iHeight-OffsetH) || (Bufx1*Bufy1)>(iWidth*iHeight)) 
	                {
	                    LOGE("[FDdraw] 0 FD position invalid i %d x0 %d x1 %d y0 %d y1 %d\n",i,Bufx0,Bufx1,Bufy0,Bufy1); 
	                    FDwidth=0;
	                    Bufx0=0;
	                    Bufx1=0;
	                    Bufy0=0; 
	                    Bufy1=0;     
	                }       
	                       	
	                break;       	   
	                case 1:
	                case 3:
	                if( (ucSensorFlag ==0) ||  (ucSensorFlag ==3))
	                {
	                	Bufx1= iWidth - 1 - ((tmp.y0*iWidth)/IMAGE_HEIGHT);
	                    Bufx0= iWidth - 1 - ((tmp.y1*iWidth)/IMAGE_HEIGHT);
	                    Bufy0= iHeight - 1 - ((tmp.x1*iHeight)/IMAGE_WIDTH);
	                    Bufy1= iHeight - 1 - ((tmp.x0*iHeight)/IMAGE_WIDTH);
	                }
	                else if( (ucSensorFlag ==1) ||  (ucSensorFlag ==2))
	                {
	                	Bufx0=(tmp.y0*iWidth)/IMAGE_HEIGHT;
	                    Bufx1=(tmp.y1*iWidth)/IMAGE_HEIGHT;
	                    Bufy0=(tmp.x0*iHeight)/IMAGE_WIDTH;
	                    Bufy1=(tmp.x1*iHeight)/IMAGE_WIDTH;
	                }
	    
	                else
	                {
	                   Bufx1= iWidth - 1 - ((tmp.y0*iWidth)/IMAGE_HEIGHT);
	                   Bufx0= iWidth - 1 - ((tmp.y1*iWidth)/IMAGE_HEIGHT);
	                   Bufy0=(tmp.x0*iHeight)/IMAGE_WIDTH;
	                   Bufy1=(tmp.x1*iHeight)/IMAGE_WIDTH;
	                }
	                //LOGE("After zoom x0 = %d y0 = %d x1 = %d y1=%d \n",Bufx0,Bufy0,Bufx1,Bufy1);
	                     
	                FDwidth=(Bufx1-Bufx0)+1;
	                FDHeight=(Bufy1-Bufy0)+1;            
	                //if((FDwidth!=(Bufy1-Bufy0))&&(FDwidth>(Bufy1-Bufy0)))
	                //    FDwidth=(Bufy1-Bufy0);
	                
	                Bufx0+=OffsetH;
	                Bufy0+=OffsetW;
	                Bufx1+=OffsetH;
	                Bufy1+=OffsetW;
	                if(Bufx1>(iWidth-OffsetH) || Bufy1>(iHeight-OffsetW) || (Bufx1*Bufy1)>(iWidth*iHeight)) 
	                {
	                    LOGE("[FDdraw] 1 FD position invalid i %d x0 %d x1 %d y0 %d y1 %d\n",i,Bufx0,Bufx1,Bufy0,Bufy1); 
	                    FDwidth=0;
	                    Bufx0=0;
	                    Bufx1=0;
	                    Bufy0=0; 
	                    Bufy1=0;   
	                        
	                }
	                break;
	                default:
	                Bufx0=(tmp.x0*iWidth)/IMAGE_WIDTH;
	                Bufx1=(tmp.x1*iWidth)/IMAGE_WIDTH;
	                Bufy0=(tmp.y0*iHeight)/IMAGE_HEIGHT;    
	                Bufy1=(tmp.y1*iHeight)/IMAGE_HEIGHT;         
	                FDwidth=(Bufx1-Bufx0)+1;
	                FDHeight=(Bufy1-Bufy0)+1;
	                //if((FDwidth!=(Bufy1-Bufy0))&&(FDwidth>(Bufy1-Bufy0)))
	                //    FDwidth=(Bufy1-Bufy0);
	                
	                Bufx0+=OffsetW;
	                Bufy0+=OffsetH;
	                Bufx1+=OffsetW;
	                Bufy1+=OffsetH;
	                if(Bufx1>(iWidth-OffsetW) || Bufy1>(iHeight-OffsetH) || (Bufx1*Bufy1)>(iWidth*iHeight)) 
	                {
	                    LOGE("[FDdraw] 2 FD position invalid i %d x0 %d x1 %d y0 %d y1 %d\n",i,Bufx0,Bufx1,Bufy0,Bufy1); 
	                    FDwidth=0;
	                    Bufx0=0;
	                    Bufx1=0; 
	                    Bufy0=0; 
	                    Bufy1=0;    
	                }
	                break;
	           } 
	        }
	        
	        //LOGE("[FDdraw] iWidth: %d, iHeight: %d,  IMAGE_WIDTH: %d,  IMAGE_HEIGHT: %d,\n",iWidth,iHeight,IMAGE_WIDTH,IMAGE_HEIGHT);
	        //LOGE("[FDdraw] FD position i %d, x0 %d, y0 %d, x1 %d, y1 %d, w %d, h %d,\n",i,Bufx0,Bufy0,Bufx1,Bufy1,FDwidth, FDHeight);
	        //LOGE("[FDdraw] Color %x\n",*(display_buffer_ptr+(((iHeight>>2)*iWidth)+(iWidth>>2))));
	        
	        //**************************************************************************************//
	        //****************BinChang 2011/07/22 Boundary Protection****************//
	        //**************************************************************************************//
	        if((MINT32)Bufx0 < 0)
	        	 Bufx0 =0 ;
	        if(Bufx1 >= iWidth)
	           Bufx1 =  iWidth-1;
	        if((MINT32)Bufy0 < 0)
	        	 Bufy0 =0 ;
	        if(Bufy1 >= iHeight)
	        	 Bufy1 =  iHeight-1;
	        
	        if( ((Bufx0 ==0) && (Bufx1 ==0)) || (Bufx0 > Bufx1) )
	            FDwidth=0;	
	        else
	        	  FDwidth=(Bufx1-Bufx0)+1;
	        	   
	        if( ((Bufy0 ==0) && (Bufy1 ==0)) || (Bufy0 > Bufy1) )
	        	  FDHeight = 0; 
	        else
	        	  FDHeight=(Bufy1-Bufy0)+1;
	        //**************************************************************************************//
	        //**************************************************************************************//        
	        
	            for(MUINT32 k=0; k<FDwidth; k++)
	            {
	                *(display_buffer_ptr + Bufy0 * iWidth + Bufx0 + k) = ORANGE;
	                *(display_buffer_ptr + Bufy1 * iWidth + Bufx0 + k) = ORANGE;
	                
	                //**************************************************************************************//
	                //****************BinChang 2011/07/18 Boundary Protection****************//
	                //**************************************************************************************//
	                if(((MINT32)Bufy0-1)>=0)
	                   *(display_buffer_ptr + (Bufy0-1) * iWidth + Bufx0 + k) = BLACK;
	                if(((MINT32)Bufy1-1)>=0)
	                   *(display_buffer_ptr + (Bufy1-1) * iWidth + Bufx0 + k) = ORANGE;
	                if(((MINT32)Bufy0+1)< (MINT32)iHeight)
	                   *(display_buffer_ptr + (Bufy0+1) * iWidth + Bufx0 + k) = ORANGE;
	                if(((MINT32)Bufy1+1)< (MINT32)iHeight)
	                   *(display_buffer_ptr + (Bufy1+1) * iWidth + Bufx0 + k) = BLACK;
	                if(((MINT32)Bufy0+2)< (MINT32)iHeight)
	                   *(display_buffer_ptr + (Bufy0+2) * iWidth + Bufx0 + k) = BLACK;
	                if(((MINT32)Bufy1-2)>=0)
	                   *(display_buffer_ptr + (Bufy1-2) * iWidth + Bufx0 + k) = BLACK;
	            }
	
	            for(MUINT32 k=0; k<FDHeight; k++)
	            {
	                *(display_buffer_ptr + (Bufy0 + k) * iWidth + Bufx0) = ORANGE;
	                *(display_buffer_ptr + (Bufy0 + k) * iWidth + Bufx1) = ORANGE;
	
	                //**************************************************************************************//
	                //****************BinChang 2011/07/18 Boundary Protection****************//
	                //**************************************************************************************//
	                if(((MINT32)Bufx0-1)>=0)
	                    *(display_buffer_ptr + (Bufy0 + k) * iWidth + (Bufx0-1)) = BLACK;
	                if(((MINT32)Bufx1-1)>=0)
	                    *(display_buffer_ptr + (Bufy0 + k) * iWidth + (Bufx1-1)) = ORANGE;
	                if(((MINT32)Bufx0+1)<(MINT32)iWidth)
	                    *(display_buffer_ptr + (Bufy0 + k) * iWidth + (Bufx0+1)) = ORANGE;
	                if(((MINT32)Bufx1+1)<(MINT32)iWidth)
	                   *(display_buffer_ptr + (Bufy0 + k) * iWidth + (Bufx1+1)) = BLACK;
	                if(((MINT32)Bufx0+2)<(MINT32)iWidth)
	                   *(display_buffer_ptr + (Bufy0 + k) * iWidth + (Bufx0+2)) = BLACK;
	                if(((MINT32)Bufx1-2)>=0)
	                    *(display_buffer_ptr + (Bufy0 + k) * iWidth + (Bufx1-2)) = BLACK;
	            }
	    }
   }
    
    #ifdef LOGON
    LOGD("FDVTDrawFaceRect out\n");   
    #endif
}


void AppFDVT::FDVTSDDrawFaceRect(MUINT32 image_buffer_address,MUINT32 Width,MUINT32 Height,MUINT32 OffsetW,MUINT32 OffsetH,MUINT8 orientation)
{
    
    kal_uint16 *display_buffer_ptr = (kal_uint16*)image_buffer_address;
    result *fd_result_data = (result *)FDVTResult2AP;
    result tmp;
    MUINT32 FDwidth;
    MUINT32 FDHeight;
    
    MUINT32 Bufx0,Bufx1,Bufy0,Bufy1;
    MUINT8 Cnt = 0;
    MUINT32 iWidth;
    MUINT32 iHeight;
    MUINT8 ucSensorFlag, ucLcmFlag, ucSensorType;
    MUINT32 UpdateX0,UpdateX1, UpdateY0, UpdateY1;
    
    #ifdef LOGON
    LOGD("FDVTDrawFaceRect in\n");   
    #endif
    //LOGE("image_buffer_address 0x%x ImageW %d ImageH %d OffsetW %d OffsetH %d Img %d\n",(MUINT32)image_buffer_address,Width,Height,OffsetW,OffsetH,IMAGE_HEIGHT);
    //LOGE("[FDdraw] GFD_NUM %d LFD_NUM = %d Color_compensate_face_number = %d  \n",GFD_NUM,LFD_NUM,Color_compensate_face_number); 
    
    iWidth=Width;
    iHeight=Height;
    ucSensorFlag    = (orientation & 0xE0) >> 5;
    ucLcmFlag = (orientation & 0x1C) >> 2;
    ucSensorType = orientation & 0x03;
    
        
    memset((void*)image_buffer_address,0,Width*Height*2);
    //for (int i = 0 ; i < (LFD_NUM + GFD_NUM+Color_compensate_face_number); i++) {
    
    //BinChang 20110316 & 20110417
    	for (int i = 0 ; i < (MAX_TRACKING_FACE_NUM); i++) {
    	if( (g_wDisplay_flag[i] == 0) || (AF_Face_Indicator[i] ==0) )
    		continue;
    		
    	tmp=fd_result_data[i];
    	/*
    	if (tmp.display_flag == FALSE ) {
            continue;
        }*/
        Cnt++;
        
        //************************************************************************************************//
        //**************       ucLcmFlag:        73Demo/JxxPhone = 1  (F/B)        ***************//
        //**************                                      73Tablet / 73Cxx = 0       (F/B)        ***************//
        //************************************************************************************************//
        //*****      ucSensorFlag:  73DemoPhone:    Front Cam = 0, Back Cam = 1     ****//
        //*****                                     73JxxPhone:      Front Cam = 3, Back Cam = 1     ****//
        //********                                      73Tablet :   Front Cam = 3, Back Cam = 1      ********//
        //********                                      73Cxx    :   Front Cam = 1, Back Cam = 0      ********//
        //************************************************************************************************//
        //************************************************************************************************//
        
        if(ucSensorType==0)  // Main-Back Sensor
        {
            switch(ucLcmFlag)
            {
                case 0:
                case 2:
                if( (ucSensorFlag ==0) ||  (ucSensorFlag ==3)) 
                {	                    
                    Bufx0=iWidth - 1 - (tmp.x1*iWidth)/IMAGE_WIDTH;
                    Bufx1=iWidth - 1 - (tmp.x0*iWidth)/IMAGE_WIDTH;
                    Bufy0=iHeight - 1 - (tmp.y1*iHeight)/IMAGE_HEIGHT;
                    Bufy1=iHeight - 1 - (tmp.y0*iHeight)/IMAGE_HEIGHT;
                }
                else if( (ucSensorFlag ==1) ||  (ucSensorFlag ==2))
                {
                   Bufx0=(tmp.x0*iWidth)/IMAGE_WIDTH;
                   Bufx1=(tmp.x1*iWidth)/IMAGE_WIDTH;
                   Bufy0=(tmp.y0*iHeight)/IMAGE_HEIGHT;
                   Bufy1=(tmp.y1*iHeight)/IMAGE_HEIGHT;
                }
                else
                {
                   Bufx0=(tmp.x0*iWidth)/IMAGE_WIDTH;
                   Bufx1=(tmp.x1*iWidth)/IMAGE_WIDTH;
                   Bufy0=(tmp.y0*iHeight)/IMAGE_HEIGHT;
                   Bufy1=(tmp.y1*iHeight)/IMAGE_HEIGHT;
                }
                
                FDwidth=(Bufx1-Bufx0)+1;
                FDHeight=(Bufy1-Bufy0)+1;
                //if((FDwidth!=(Bufy1-Bufy0))&&(FDwidth>(Bufy1-Bufy0)))
                //    FDwidth=(Bufy1-Bufy0);
                
                Bufx0+=OffsetW;
                Bufy0+=OffsetH;
                Bufx1+=OffsetW;
                Bufy1+=OffsetH;
                if(Bufx1>(iWidth-OffsetW) || Bufy1>(iHeight-OffsetH) || (Bufx1*Bufy1)>(iWidth*iHeight)) 
                {
                    LOGE("[FDdraw] 0 FD position invalid i %d x0 %d x1 %d y0 %d y1 %d\n",i,Bufx0,Bufx1,Bufy0,Bufy1); 
                    FDwidth=0;
                    Bufx0=0;
                    Bufx1=0;
                    Bufy0=0; 
                    Bufy1=0;     
                }
                break;       	   
                case 1:
                case 3:
                if( (ucSensorFlag ==0) ||  (ucSensorFlag ==3))
                {
                    Bufx0= (tmp.y0*iWidth)/IMAGE_HEIGHT;
                    Bufx1= (tmp.y1*iWidth)/IMAGE_HEIGHT;
                    Bufy0= iHeight - 1 - ((tmp.x1*iHeight)/IMAGE_WIDTH);
                    Bufy1= iHeight - 1 - ((tmp.x0*iHeight)/IMAGE_WIDTH);
                }
                else if( (ucSensorFlag ==1) ||  (ucSensorFlag ==2))
                {
                	Bufx1= iWidth - 1 - ((tmp.y0*iWidth)/IMAGE_HEIGHT);
                    Bufx0= iWidth - 1 - ((tmp.y1*iWidth)/IMAGE_HEIGHT);
                    Bufy0=(tmp.x0*iHeight)/IMAGE_WIDTH;
                    Bufy1=(tmp.x1*iHeight)/IMAGE_WIDTH;
                }
                else
                {
                   Bufx1= iWidth - 1 - ((tmp.y0*iWidth)/IMAGE_HEIGHT);
                   Bufx0= iWidth - 1 - ((tmp.y1*iWidth)/IMAGE_HEIGHT);
                   Bufy0=(tmp.x0*iHeight)/IMAGE_WIDTH;
                   Bufy1=(tmp.x1*iHeight)/IMAGE_WIDTH;
                }
                //LOGE("After zoom x0 = %d y0 = %d x1 = %d y1=%d \n",Bufx0,Bufy0,Bufx1,Bufy1);
                     
                FDwidth=(Bufx1-Bufx0)+1;
                FDHeight=(Bufy1-Bufy0)+1;            
                //if((FDwidth!=(Bufy1-Bufy0))&&(FDwidth>(Bufy1-Bufy0)))
                //    FDwidth=(Bufy1-Bufy0);
                
                Bufx0+=OffsetH;
                Bufy0+=OffsetW;
                Bufx1+=OffsetH;
                Bufy1+=OffsetW;
                if(Bufx1>(iWidth-OffsetH) || Bufy1>(iHeight-OffsetW) || (Bufx1*Bufy1)>(iWidth*iHeight)) 
                {
                    LOGE("[FDdraw] 1 FD position invalid i %d x0 %d x1 %d y0 %d y1 %d\n",i,Bufx0,Bufx1,Bufy0,Bufy1); 
                    FDwidth=0;
                    Bufx0=0;
                    Bufx1=0;
                    Bufy0=0; 
                    Bufy1=0;   
                        
                }
                break;
                default:
                Bufx0=(tmp.x0*iWidth)/IMAGE_WIDTH;
                Bufx1=(tmp.x1*iWidth)/IMAGE_WIDTH;
                Bufy0=(tmp.y0*iHeight)/IMAGE_HEIGHT;    
                Bufy1=(tmp.y1*iHeight)/IMAGE_HEIGHT;         
                FDwidth=(Bufx1-Bufx0)+1;
                FDHeight=(Bufy1-Bufy0)+1;
                //if((FDwidth!=(Bufy1-Bufy0))&&(FDwidth>(Bufy1-Bufy0)))
                //    FDwidth=(Bufy1-Bufy0);
                
                Bufx0+=OffsetW;
                Bufy0+=OffsetH;
                Bufx1+=OffsetW;
                Bufy1+=OffsetH;
                if(Bufx1>(iWidth-OffsetW) || Bufy1>(iHeight-OffsetH) || (Bufx1*Bufy1)>(iWidth*iHeight)) 
                {
                    LOGE("[FDdraw] 2 FD position invalid i %d x0 %d x1 %d y0 %d y1 %d\n",i,Bufx0,Bufx1,Bufy0,Bufy1); 
                    FDwidth=0;
                    Bufx0=0;
                    Bufx1=0; 
                    Bufy0=0; 
                    Bufy1=0;    
                }
                break;
           }
        }
        
        else  // Sub-Front Sensor (ucSensorType=1)
        {
             switch(ucLcmFlag)
            {
                case 0:
                case 2:
                if( (ucSensorFlag ==0) ||  (ucSensorFlag ==3)) 
                {	
                    Bufx0=iWidth - 1 - (tmp.x1*iWidth)/IMAGE_WIDTH;
                    Bufx1=iWidth - 1 - (tmp.x0*iWidth)/IMAGE_WIDTH;
                    Bufy0=(tmp.y0*iHeight)/IMAGE_HEIGHT;
                    Bufy1=(tmp.y1*iHeight)/IMAGE_HEIGHT;
                }
                else if( (ucSensorFlag ==1) ||  (ucSensorFlag ==2))
                {
                   Bufx0=(tmp.x0*iWidth)/IMAGE_WIDTH;
                   Bufx1=(tmp.x1*iWidth)/IMAGE_WIDTH;
                   Bufy0=iHeight - 1 - (tmp.y1*iHeight)/IMAGE_HEIGHT;
                   Bufy1=iHeight - 1 - (tmp.y0*iHeight)/IMAGE_HEIGHT;
                }
                else
                {
                   Bufx0=(tmp.x0*iWidth)/IMAGE_WIDTH;
                   Bufx1=(tmp.x1*iWidth)/IMAGE_WIDTH;
                   Bufy0=(tmp.y0*iHeight)/IMAGE_HEIGHT;
                   Bufy1=(tmp.y1*iHeight)/IMAGE_HEIGHT;
                }
                
                FDwidth=(Bufx1-Bufx0)+1;
                FDHeight=(Bufy1-Bufy0)+1;
                //if((FDwidth!=(Bufy1-Bufy0))&&(FDwidth>(Bufy1-Bufy0)))
                //    FDwidth=(Bufy1-Bufy0);
                
                Bufx0+=OffsetW;
                Bufy0+=OffsetH;
                Bufx1+=OffsetW;
                Bufy1+=OffsetH;
                if(Bufx1>(iWidth-OffsetW) || Bufy1>(iHeight-OffsetH) || (Bufx1*Bufy1)>(iWidth*iHeight)) 
                {
                    LOGE("[FDdraw] 0 FD position invalid i %d x0 %d x1 %d y0 %d y1 %d\n",i,Bufx0,Bufx1,Bufy0,Bufy1); 
                    FDwidth=0;
                    Bufx0=0;
                    Bufx1=0;
                    Bufy0=0; 
                    Bufy1=0;     
                }       
                       	
                break;       	   
                case 1:
                case 3:
                if( (ucSensorFlag ==0) ||  (ucSensorFlag ==3))
                {
                	Bufx1= iWidth - 1 - ((tmp.y0*iWidth)/IMAGE_HEIGHT);
                    Bufx0= iWidth - 1 - ((tmp.y1*iWidth)/IMAGE_HEIGHT);
                    Bufy0= iHeight - 1 - ((tmp.x1*iHeight)/IMAGE_WIDTH);
                    Bufy1= iHeight - 1 - ((tmp.x0*iHeight)/IMAGE_WIDTH);
                }
                else if( (ucSensorFlag ==1) ||  (ucSensorFlag ==2))
                {
                	Bufx0=(tmp.y0*iWidth)/IMAGE_HEIGHT;
                    Bufx1=(tmp.y1*iWidth)/IMAGE_HEIGHT;
                    Bufy0=(tmp.x0*iHeight)/IMAGE_WIDTH;
                    Bufy1=(tmp.x1*iHeight)/IMAGE_WIDTH;
                }
    
                else
                {
                   Bufx1= iWidth - 1 - ((tmp.y0*iWidth)/IMAGE_HEIGHT);
                   Bufx0= iWidth - 1 - ((tmp.y1*iWidth)/IMAGE_HEIGHT);
                   Bufy0=(tmp.x0*iHeight)/IMAGE_WIDTH;
                   Bufy1=(tmp.x1*iHeight)/IMAGE_WIDTH;
                }
                //LOGE("After zoom x0 = %d y0 = %d x1 = %d y1=%d \n",Bufx0,Bufy0,Bufx1,Bufy1);
                     
                FDwidth=(Bufx1-Bufx0)+1;
                FDHeight=(Bufy1-Bufy0)+1;            
                //if((FDwidth!=(Bufy1-Bufy0))&&(FDwidth>(Bufy1-Bufy0)))
                //    FDwidth=(Bufy1-Bufy0);
                
                Bufx0+=OffsetH;
                Bufy0+=OffsetW;
                Bufx1+=OffsetH;
                Bufy1+=OffsetW;
                if(Bufx1>(iWidth-OffsetH) || Bufy1>(iHeight-OffsetW) || (Bufx1*Bufy1)>(iWidth*iHeight)) 
                {
                    LOGE("[FDdraw] 1 FD position invalid i %d x0 %d x1 %d y0 %d y1 %d\n",i,Bufx0,Bufx1,Bufy0,Bufy1); 
                    FDwidth=0;
                    Bufx0=0;
                    Bufx1=0;
                    Bufy0=0; 
                    Bufy1=0;   
                        
                }
                break;
                default:
                Bufx0=(tmp.x0*iWidth)/IMAGE_WIDTH;
                Bufx1=(tmp.x1*iWidth)/IMAGE_WIDTH;
                Bufy0=(tmp.y0*iHeight)/IMAGE_HEIGHT;    
                Bufy1=(tmp.y1*iHeight)/IMAGE_HEIGHT;         
                FDwidth=(Bufx1-Bufx0)+1;
                FDHeight=(Bufy1-Bufy0)+1;
                //if((FDwidth!=(Bufy1-Bufy0))&&(FDwidth>(Bufy1-Bufy0)))
                //    FDwidth=(Bufy1-Bufy0);
                
                Bufx0+=OffsetW;
                Bufy0+=OffsetH;
                Bufx1+=OffsetW;
                Bufy1+=OffsetH;
                if(Bufx1>(iWidth-OffsetW) || Bufy1>(iHeight-OffsetH) || (Bufx1*Bufy1)>(iWidth*iHeight)) 
                {
                    LOGE("[FDdraw] 2 FD position invalid i %d x0 %d x1 %d y0 %d y1 %d\n",i,Bufx0,Bufx1,Bufy0,Bufy1); 
                    FDwidth=0;
                    Bufx0=0;
                    Bufx1=0; 
                    Bufy0=0; 
                    Bufy1=0;    
                }
                break;
           } 
        }
        
        //LOGE("[FDdraw] iWidth: %d, iHeight: %d,  IMAGE_WIDTH: %d,  IMAGE_HEIGHT: %d,\n",iWidth,iHeight,IMAGE_WIDTH,IMAGE_HEIGHT);
        //LOGE("[FDdraw] FD position i %d, x0 %d, y0 %d, x1 %d, y1 %d, w %d, h %d,\n",i,Bufx0,Bufy0,Bufx1,Bufy1,FDwidth, FDHeight);
        //LOGE("[FDdraw] Color %x\n",*(display_buffer_ptr+(((iHeight>>2)*iWidth)+(iWidth>>2))));
        
        //**************************************************************************************//
        //****************BinChang 2011/07/22 Boundary Protection****************//
        //**************************************************************************************//
        if((MINT32)Bufx0 < 0)
        	 Bufx0 =0 ;
        if(Bufx1 >= iWidth)
           Bufx1 =  iWidth-1;
        if((MINT32)Bufy0 < 0)
        	 Bufy0 =0 ;
        if(Bufy1 >= iHeight)
        	 Bufy1 =  iHeight-1;
        
        if( ((Bufx0 ==0) && (Bufx1 ==0)) || (Bufx0 > Bufx1) )
            FDwidth=0;	
        else
        	  FDwidth=(Bufx1-Bufx0)+1;
        	   
        if( ((Bufy0 ==0) && (Bufy1 ==0)) || (Bufy0 > Bufy1) )
        	  FDHeight = 0; 
        else
        	  FDHeight=(Bufy1-Bufy0)+1;
        //**************************************************************************************//
        //**************************************************************************************//        
        
            for(MUINT32 k=0; k<FDwidth; k++)
            {
                *(display_buffer_ptr + Bufy0 * iWidth + Bufx0 + k) = ORANGE;
                *(display_buffer_ptr + Bufy1 * iWidth + Bufx0 + k) = ORANGE;
                
                //**************************************************************************************//
                //****************BinChang 2011/07/18 Boundary Protection****************//
                //**************************************************************************************//
                if(((MINT32)Bufy0-1)>=0)
                   *(display_buffer_ptr + (Bufy0-1) * iWidth + Bufx0 + k) = BLACK;
                if(((MINT32)Bufy1-1)>=0)
                   *(display_buffer_ptr + (Bufy1-1) * iWidth + Bufx0 + k) = ORANGE;
                if(((MINT32)Bufy0+1)< (MINT32)iHeight)
                   *(display_buffer_ptr + (Bufy0+1) * iWidth + Bufx0 + k) = ORANGE;
                if(((MINT32)Bufy1+1)< (MINT32)iHeight)
                   *(display_buffer_ptr + (Bufy1+1) * iWidth + Bufx0 + k) = BLACK;
                if(((MINT32)Bufy0+2)< (MINT32)iHeight)
                   *(display_buffer_ptr + (Bufy0+2) * iWidth + Bufx0 + k) = BLACK;
                if(((MINT32)Bufy1-2)>=0)
                   *(display_buffer_ptr + (Bufy1-2) * iWidth + Bufx0 + k) = BLACK;
            }

            for(MUINT32 k=0; k<FDHeight; k++)
            {
                *(display_buffer_ptr + (Bufy0 + k) * iWidth + Bufx0) = ORANGE;
                *(display_buffer_ptr + (Bufy0 + k) * iWidth + Bufx1) = ORANGE;

                //**************************************************************************************//
                //****************BinChang 2011/07/18 Boundary Protection****************//
                //**************************************************************************************//
                if(((MINT32)Bufx0-1)>=0)
                    *(display_buffer_ptr + (Bufy0 + k) * iWidth + (Bufx0-1)) = BLACK;
                if(((MINT32)Bufx1-1)>=0)
                    *(display_buffer_ptr + (Bufy0 + k) * iWidth + (Bufx1-1)) = ORANGE;
                if(((MINT32)Bufx0+1)<(MINT32)iWidth)
                    *(display_buffer_ptr + (Bufy0 + k) * iWidth + (Bufx0+1)) = ORANGE;
                if(((MINT32)Bufx1+1)<(MINT32)iWidth)
                   *(display_buffer_ptr + (Bufy0 + k) * iWidth + (Bufx1+1)) = BLACK;
                if(((MINT32)Bufx0+2)<(MINT32)iWidth)
                   *(display_buffer_ptr + (Bufy0 + k) * iWidth + (Bufx0+2)) = BLACK;
                if(((MINT32)Bufx1-2)>=0)
                    *(display_buffer_ptr + (Bufy0 + k) * iWidth + (Bufx1-2)) = BLACK;
            }
    }
    
    #ifdef LOGON
    LOGD("FDVTDrawFaceRect out\n");   
    #endif
}


/*******************************************************************************
* Initial DRAM                                           
********************************************************************************/
#if defined(MTK_M4U_SUPPORT)
void AppFDVT::InitialDRAM()
{
    pM4UDrv =new MTKM4UDrv();
    memset (&mWorkingBuffer, 0,  sizeof(FDVTM4UInfo)); 
    memset (&mLearningData, 0,  sizeof(FDVTM4UInfo));
    
    pM4UDrv->m4u_reset_mva_release_tlb(M4U_CLNTMOD_FD);
    
    hw_fdvt_buffer = (MUINT32 *)memalign(L1_CACHE_BYTES, (FDVT_TOTAL_BUFFER_SIZE + (L1_CACHE_BYTES)) & ~(L1_CACHE_BYTES -1)); 

    if (NULL == hw_fdvt_buffer)  {
        LOGD("Not enough memory for hw_fdvt_buffer buffer\n");
        return;         
    } 
    
    hw_fd_result_datava=hw_fdvt_buffer;
    hw_tc_result_datava=hw_fd_result_datava+FD_RESULT_MAX_SIZE;
    rs_config_datava=hw_tc_result_datava+TC_RESULT_MAX_SIZE;
    fd_config_datava=rs_config_datava+RS_CONFIG_MAX_SIZE;
    hw_rs_result_datava=fd_config_datava+FD_CONFIG_MAX_SIZE;   
            
    mWorkingBuffer.useM4U = 1; 
    mWorkingBuffer.virtAddr = (MUINT32)hw_fdvt_buffer; 
    mWorkingBuffer.size = (FDVT_TOTAL_BUFFER_SIZE + (L1_CACHE_BYTES)) & ~(L1_CACHE_BYTES -1); 
    allocM4UMemory(mWorkingBuffer.virtAddr, mWorkingBuffer.size, &mWorkingBuffer.M4UVa);       

    memcpy(rs_config_datava,rs_config_table,RS_CONFIG_MAX_SIZE*4);
    //LOGD("InitialDRAM Init rs_config_datava buffer\n"); 
    memcpy(fd_config_datava,fd_config_table,FD_CONFIG_MAX_SIZE*4);
    //LOGD("InitialDRAM Init fd_config_datava buffer\n"); 

    hw_fd_result_datapa=(MUINT32 *)mWorkingBuffer.M4UVa;
    
    hw_tc_result_datapa=hw_fd_result_datapa+FD_RESULT_MAX_SIZE;
    rs_config_datapa=hw_tc_result_datapa+TC_RESULT_MAX_SIZE;
    fd_config_datapa=rs_config_datapa+RS_CONFIG_MAX_SIZE;
    hw_rs_result_datapa=fd_config_datapa+FD_CONFIG_MAX_SIZE;
    
    LOGD("InitialDRAM hw_fd_result_datapa 0x%x hw_tc_result_datapa 0x%x rs_config_datapa 0x%x fd_config_datapa 0x%x\n",(MUINT32)hw_fd_result_datapa,(MUINT32)hw_tc_result_datapa,(MUINT32)rs_config_datapa,(MUINT32)fd_config_datapa);
    LOGD("InitialDRAM hw_fd_result_datava 0x%x hw_tc_result_datava 0x%x rs_config_datava 0x%x fd_config_datava 0x%x\n",(MUINT32)hw_fd_result_datava,(MUINT32)hw_tc_result_datava,(MUINT32)rs_config_datava,(MUINT32)fd_config_datava);
    
    LearningSize=(FNUM_LEARNING_DATA0+FNUM_LEARNING_DATA1+FNUM_LEARNING_DATA2+FNUM_LEARNING_DATA3+FNUM_LEARNING_DATA4+FNUM_LEARNING_DATA5+FNUM_LEARNING_DATA6+FNUM_LEARNING_DATA7)*sizeof(MUINT32);
    learning_datava = (MUINT32 *)memalign(L1_CACHE_BYTES, (LearningSize + (L1_CACHE_BYTES)) & ~(L1_CACHE_BYTES -1)); 

    if (NULL == learning_datava)  {
        LOGD("Not enough memory for hw_fdvt_buffer buffer\n");
        return;         
    } 
    learning_data0va=learning_datava;
    learning_data1va=learning_data0va+FNUM_LEARNING_DATA0;
    learning_data2va=learning_data1va+FNUM_LEARNING_DATA1;
    learning_data3va=learning_data2va+FNUM_LEARNING_DATA2;
    learning_data4va=learning_data3va+FNUM_LEARNING_DATA3;
    learning_data5va=learning_data4va+FNUM_LEARNING_DATA4;
    learning_data6va=learning_data5va+FNUM_LEARNING_DATA5;
    learning_data7va=learning_data6va+FNUM_LEARNING_DATA6;
                
    mLearningData.useM4U = 1; 
    mLearningData.virtAddr = (MUINT32)learning_datava; 
    mLearningData.size =(LearningSize + (L1_CACHE_BYTES)) & ~(L1_CACHE_BYTES -1);
    allocM4UMemory(mLearningData.virtAddr, mLearningData.size, &mLearningData.M4UVa); 
    
    learning_dataph=(MUINT32 *)mLearningData.M4UVa;
    learning_data0ph=learning_dataph;
    learning_data1ph=learning_data0ph+FNUM_LEARNING_DATA0;
    learning_data2ph=learning_data1ph+FNUM_LEARNING_DATA1;
    learning_data3ph=learning_data2ph+FNUM_LEARNING_DATA2;
    learning_data4ph=learning_data3ph+FNUM_LEARNING_DATA3;
    learning_data5ph=learning_data4ph+FNUM_LEARNING_DATA4;
    learning_data6ph=learning_data5ph+FNUM_LEARNING_DATA5;
    learning_data7ph=learning_data6ph+FNUM_LEARNING_DATA6;
 
    LOGD("InitialDRAM learning_data0ph 0x%x learning_data1ph 0x%x learning_data2ph 0x%x learning_data3ph 0x%x learning_data4ph 0x%x learning_data5ph 0x%x learning_data6ph 0x%x learning_data7ph 0x%x\n",(MUINT32)learning_data0ph,(MUINT32)learning_data1ph,(MUINT32)learning_data2ph,(MUINT32)learning_data3ph,(MUINT32)learning_data4ph,(MUINT32)learning_data5ph,(MUINT32)learning_data6ph,(MUINT32)learning_data7ph);
    LOGD("InitialDRAM learning_data0va 0x%x learning_data1va 0x%x learning_data2va 0x%x learning_data3va 0x%x learning_data4va 0x%x learning_data5va 0x%x learning_data6va 0x%x learning_data7va 0x%x\n",(MUINT32)learning_data0va,(MUINT32)learning_data1va,(MUINT32)learning_data2va,(MUINT32)learning_data3va,(MUINT32)learning_data4va,(MUINT32)learning_data5va,(MUINT32)learning_data6va,(MUINT32)learning_data7va);
    
    memcpy(learning_data0va,learning_data0,sizeof(unsigned int)*FNUM_LEARNING_DATA0);
    memcpy(learning_data1va,learning_data1,sizeof(unsigned int)*FNUM_LEARNING_DATA1);
    memcpy(learning_data2va,learning_data2,sizeof(unsigned int)*FNUM_LEARNING_DATA2);
    memcpy(learning_data3va,learning_data3,sizeof(unsigned int)*FNUM_LEARNING_DATA3);
    memcpy(learning_data4va,learning_data4,sizeof(unsigned int)*FNUM_LEARNING_DATA4);
    memcpy(learning_data5va,learning_data5,sizeof(unsigned int)*FNUM_LEARNING_DATA5);
    memcpy(learning_data6va,learning_data6,sizeof(unsigned int)*FNUM_LEARNING_DATA6);
    memcpy(learning_data7va,learning_data7,sizeof(unsigned int)*FNUM_LEARNING_DATA7); 
}
#else
void AppFDVT::InitialDRAM()
{    
    MUINT32 adr;
        
    hw_fdvt_buffer = (MUINT32 *)pmem_alloc_sync((FDVT_TOTAL_BUFFER_SIZE+ByteAlignment), &fdrt_fd);
    adr=(MUINT32)hw_fdvt_buffer;
    if((adr&0x0000000F)!=0)
    {    
        hw_fdvt_buffer = (MUINT32*)((MUINT32)(hw_fdvt_buffer + ByteAlignment) & (~(ByteAlignment-1)));
        LOGD("InitialDRAM hw_fdvt_buffer 0x%x\n",(MUINT32)hw_fdvt_buffer);
    }
    if (NULL == hw_fdvt_buffer)  {
        LOGD("Not enough memory for hw_fdvt_buffer buffer\n");
        return;         
    } 
    
    hw_fd_result_datava=hw_fdvt_buffer;
    hw_tc_result_datava=hw_fd_result_datava+FD_RESULT_MAX_SIZE;
    rs_config_datava=hw_tc_result_datava+TC_RESULT_MAX_SIZE;
    fd_config_datava=rs_config_datava+RS_CONFIG_MAX_SIZE;
    hw_rs_result_datava=fd_config_datava+FD_CONFIG_MAX_SIZE;    
    
    memcpy(rs_config_datava,rs_config_table,RS_CONFIG_MAX_SIZE*4);
    //LOGD("InitialDRAM Init rs_config_datava buffer\n"); 
    memcpy(fd_config_datava,fd_config_table,FD_CONFIG_MAX_SIZE*4);
    //LOGD("InitialDRAM Init fd_config_datava buffer\n"); 

    
    hw_fd_result_datapa = (MUINT32*)pmem_get_phys(fdrt_fd);
    
    hw_tc_result_datapa=hw_fd_result_datapa+FD_RESULT_MAX_SIZE;
    rs_config_datapa=hw_tc_result_datapa+TC_RESULT_MAX_SIZE;
    fd_config_datapa=rs_config_datapa+RS_CONFIG_MAX_SIZE;
    hw_rs_result_datapa=fd_config_datapa+FD_CONFIG_MAX_SIZE;
       
    LOGD("InitialDRAM hw_fd_result_datapa 0x%x hw_tc_result_datapa 0x%x rs_config_datapa 0x%x fd_config_datapa 0x%x\n",(MUINT32)hw_fd_result_datapa,(MUINT32)hw_tc_result_datapa,(MUINT32)rs_config_datapa,(MUINT32)fd_config_datapa);
    LOGD("InitialDRAM hw_fd_result_datava 0x%x hw_tc_result_datava 0x%x rs_config_datava 0x%x fd_config_datava 0x%x\n",(MUINT32)hw_fd_result_datava,(MUINT32)hw_tc_result_datava,(MUINT32)rs_config_datava,(MUINT32)fd_config_datava);
    
    LearningSize=(FNUM_LEARNING_DATA0+FNUM_LEARNING_DATA1+FNUM_LEARNING_DATA2+FNUM_LEARNING_DATA3+FNUM_LEARNING_DATA4+FNUM_LEARNING_DATA5+FNUM_LEARNING_DATA6+FNUM_LEARNING_DATA7)*sizeof(MUINT32);
    learning_datava = (MUINT32 *)pmem_alloc_sync((LearningSize+ByteAlignment), &tcrt_fd);
    adr=(MUINT32)learning_datava;
    if((adr&0x0000000F)!=0)
    {    
        learning_datava = (MUINT32*)((MUINT32)(learning_datava + ByteAlignment) & (~(ByteAlignment-1)));
        LOGD("InitialDRAM hw_fdvt_buffer 0x%x\n",(MUINT32)learning_datava);
    }
    if (NULL == learning_datava)  {
        LOGD("Not enough memory for hw_fdvt_buffer buffer\n");
        return;         
    } 
    learning_data0va=learning_datava;
    learning_data1va=learning_data0va+FNUM_LEARNING_DATA0;
    learning_data2va=learning_data1va+FNUM_LEARNING_DATA1;
    learning_data3va=learning_data2va+FNUM_LEARNING_DATA2;
    learning_data4va=learning_data3va+FNUM_LEARNING_DATA3;
    learning_data5va=learning_data4va+FNUM_LEARNING_DATA4;
    learning_data6va=learning_data5va+FNUM_LEARNING_DATA5;
    learning_data7va=learning_data6va+FNUM_LEARNING_DATA6;
                    
    learning_dataph = (MUINT32*)pmem_get_phys(tcrt_fd);
    
    learning_data0ph=learning_dataph;
    learning_data1ph=learning_data0ph+FNUM_LEARNING_DATA0;
    learning_data2ph=learning_data1ph+FNUM_LEARNING_DATA1;
    learning_data3ph=learning_data2ph+FNUM_LEARNING_DATA2;
    learning_data4ph=learning_data3ph+FNUM_LEARNING_DATA3;
    learning_data5ph=learning_data4ph+FNUM_LEARNING_DATA4;
    learning_data6ph=learning_data5ph+FNUM_LEARNING_DATA5;
    learning_data7ph=learning_data6ph+FNUM_LEARNING_DATA6;
 
    LOGD("InitialDRAM learning_data0ph 0x%x learning_data1ph 0x%x learning_data2ph 0x%x learning_data3ph 0x%x learning_data4ph 0x%x learning_data5ph 0x%x learning_data6ph 0x%x learning_data7ph 0x%x\n",(MUINT32)learning_data0ph,(MUINT32)learning_data1ph,(MUINT32)learning_data2ph,(MUINT32)learning_data3ph,(MUINT32)learning_data4ph,(MUINT32)learning_data5ph,(MUINT32)learning_data6ph,(MUINT32)learning_data7ph);
    LOGD("InitialDRAM learning_data0va 0x%x learning_data1va 0x%x learning_data2va 0x%x learning_data3va 0x%x learning_data4va 0x%x learning_data5va 0x%x learning_data6va 0x%x learning_data7va 0x%x\n",(MUINT32)learning_data0va,(MUINT32)learning_data1va,(MUINT32)learning_data2va,(MUINT32)learning_data3va,(MUINT32)learning_data4va,(MUINT32)learning_data5va,(MUINT32)learning_data6va,(MUINT32)learning_data7va);
    
    memcpy(learning_data0va,learning_data0,sizeof(unsigned int)*FNUM_LEARNING_DATA0);
    memcpy(learning_data1va,learning_data1,sizeof(unsigned int)*FNUM_LEARNING_DATA1);
    memcpy(learning_data2va,learning_data2,sizeof(unsigned int)*FNUM_LEARNING_DATA2);
    memcpy(learning_data3va,learning_data3,sizeof(unsigned int)*FNUM_LEARNING_DATA3);
    memcpy(learning_data4va,learning_data4,sizeof(unsigned int)*FNUM_LEARNING_DATA4);
    memcpy(learning_data5va,learning_data5,sizeof(unsigned int)*FNUM_LEARNING_DATA5);
    memcpy(learning_data6va,learning_data6,sizeof(unsigned int)*FNUM_LEARNING_DATA6);
    memcpy(learning_data7va,learning_data7,sizeof(unsigned int)*FNUM_LEARNING_DATA7); 
    
}
#endif

void FDRegisterConfig(void* src_image_data)
{
    int i, idx;
    int LFDCount=0;
    MUINT32 udFeature_RegTH = 0x3F000000; //Default
    
    for(i=0;i<15;i++){
         idx = 0x00a4+i*12+0;
         if(LFD_WINDOW_NUM>LFDCount)
         {
             //LOGD("GFD_NUM in :%d",GFD_NUM);
             fdvtreg_adr[i*3]=idx;
             fdvtreg_value[i*3]=
             (int)((LFD_INFO[i].valid     &0x0001)<< 28   )+ 
             (int)((LFD_INFO[i].size_index&0x001f)<< 20   )+ 
             (int)((LFD_INFO[i].rop_dir   &0x0007)<< 16   )+ 
             (int)((LFD_INFO[i].rip_dir   &0x000f)<< 12   )+ 
             (int)((LFD_INFO[i].face_index&0x01ff)<<  0   );
             fdvtreg_adr[(i*3)+1]=idx+4;
             fdvtreg_value[(i*3)+1]=
             (int)((LFD_INFO[i].y0        &0x00ff)<< 16   )+ 
             (int)((LFD_INFO[i].x0        &0x01ff)<<  0   );
             fdvtreg_adr[(i*3)+2]=idx+8;
             fdvtreg_value[(i*3)+2]=
             (int)((LFD_INFO[i].y1        &0x00ff)<< 16   )+ 
             (int)((LFD_INFO[i].x1        &0x01ff)<<  0   );       
         }
         else   
         {
             //LOGD("GFD_NUM out:%d",GFD_NUM);
             fdvtreg_adr[i*3]=idx;
             fdvtreg_value[i*3]=0;
             fdvtreg_adr[(i*3)+1]=idx+4;
             fdvtreg_value[(i*3)+1]=0;
             fdvtreg_adr[(i*3)+2]=idx+8;
             fdvtreg_value[(i*3)+2]=0;
         }
         LFDCount++;
         //LOGD("LFD_INFO x0:%x y0:%x x1:%x y1:%x \n",LFD_INFO[i].x0,LFD_INFO[i].y0,LFD_INFO[i].x1,LFD_INFO[i].y1);               
         //LOGD("LFD_INFO Adr:0x%x Val1:0x%x Val2:0x%x Val3:0x%x\n",fdvtreg_adr[i*3],fdvtreg_value[i*3],fdvtreg_value[(i*3)+1],fdvtreg_value[(i*3)+2]); 
    }
    i*=3;
    
    fdvtreg_adr[i]=0x008c;
    fdvtreg_value[i]=(int)((GFD_INFO.y0   &0x00ff)<< 16   )+
                     (int)((GFD_INFO.x0   &0x01ff)<<  0   );
    fdvtreg_adr[i+1]=0x0090;
    fdvtreg_value[i+1]=(int)((GFD_INFO.y1   &0x00ff)<< 16   )+
                     (int)((GFD_INFO.x1   &0x01ff)<<  0   );			  
    fdvtreg_adr[i+2]=0x0094;
    fdvtreg_value[i+2]=(int)((GFD_INFO.pose   &0xffffffff)<<  0   );
    fdvtreg_adr[i+3]=0x0098;
    fdvtreg_value[i+3]=(int)(( ((long long)GFD_INFO.pose>>32) &0x0fffffff)<<  0   );
    fdvtreg_adr[i+4]=0x0158;
    fdvtreg_value[i+4]=(int)((LFD_NUM   &0x000f)<< 16   )+
		       (int)((GFD_NUM   &0x01ff)<<  0   );
    fdvtreg_adr[i+5]=0x0160;
    fdvtreg_value[i+5]=(int)((IMAGE_WIDTH&0x01ff)<< 16   )+
                       (int)((IMAGE_HEIGHT&0x00ff)<<  0   );
    fdvtreg_adr[i+6]=0x0164;
    fdvtreg_value[i+6]=(kal_uint32)src_image_data;
    
    fdvtreg_adr[i+7]=0x000C;        
    fdvtreg_value[i+7]=(kal_uint32)rs_config_datapa;

    fdvtreg_adr[i+8]=0x0080;                       
    fdvtreg_value[i+8]=(kal_uint32)fd_config_datapa;
    
    fdvtreg_adr[i+9]=0x009C;                       
    fdvtreg_value[i+9]=(kal_uint32)hw_fd_result_datapa;

    fdvtreg_adr[i+10]=0x00A0;                       
    fdvtreg_value[i+10]=(kal_uint32)hw_tc_result_datapa;

    fdvtreg_adr[i+11]=0x0078;                       
    fdvtreg_value[i+11]=REG_RMAP;
    
    fdvtreg_adr[i+12]=0x0004;
    fdvtreg_value[i+12]=TC_ENABLE+FF_ENABLE+FD_ENABLE+RS_ENABLE;
    
    fdvtreg_adr[i+13]=0x0088;   
    fdvtreg_value[i+13]=(int)((LFD_POSE_EXT      &0x0007)<< 24   )+ 
			(int)((LFD_SIZE_EXT      &0x0003)<< 20   )+ 
			(int)((LFD_SHRINK_INDEX  &0x0007)<< 16   )+ 
			(int)((LFD_PATTERN_TYPE  &0x0001)<< 12   )+ 
			(int)((LFD_SKIP_MODE     &0x0001)<<  8   )+ 
			(int)((LFD_WINDOW_NUM    &0x000f)<<  0   );	
    //fdvtreg_adr[i+14]=0x0000;   
    //fdvtreg_value[i+14]=0;	        

    fdvtreg_adr[i+14]=0x0054;                                   
    fdvtreg_value[i+14]=(kal_uint32)learning_data0ph;

    fdvtreg_adr[i+15]=0x0058;                                   
    fdvtreg_value[i+15]=(kal_uint32)learning_data1ph;
    
    fdvtreg_adr[i+16]=0x005C;                                   
    fdvtreg_value[i+16]=(kal_uint32)learning_data2ph;
    
    fdvtreg_adr[i+17]=0x0060;                                   
    fdvtreg_value[i+17]=(kal_uint32)learning_data3ph;
    
    fdvtreg_adr[i+18]=0x0064;                                   
    fdvtreg_value[i+18]=(kal_uint32)learning_data4ph;
    
    fdvtreg_adr[i+19]=0x0068;                                   
    fdvtreg_value[i+19]=(kal_uint32)learning_data5ph;
    
    fdvtreg_adr[i+20]=0x006C;                                   
    fdvtreg_value[i+20]=(kal_uint32)learning_data6ph;
    
    fdvtreg_adr[i+21]=0x0070;                                   
    fdvtreg_value[i+21]=(kal_uint32)learning_data7ph;
     
    //halFDVT_PARA_SET(fdvtreg_adr,fdvtreg_value,i+22,FDVT_GFD_MODE);
    
    //************************************************************************************************//
    //*******************BinChang 20110516 Get Feature TH********************************//
    //**********Get FD threshold    Default: 5       0/1/2/3/4/  5  /6/7/8/9/10/*****************//
    //***                      loose  (High DR/FPR)  <----> strict  (Low DR/FPR)                      ***//
    //************************************************************************************************//
    //MINT8 cFeature_TH  = NSCamCustom::get_fdvt_threshold();
    //Please fix me Sava
    MINT8 cFeature_TH  = 5;
    //LOGD("cFeature_TH:%d",cFeature_TH);
    
    if( (cFeature_TH <0) || (cFeature_TH >10) )
    	   cFeature_TH = 5; //Default
    
     switch (cFeature_TH)
     {
            case  0 : // -120
                udFeature_RegTH = 0x3F888888;
                break ;
            case  1 : // -96
                udFeature_RegTH = 0x3FA0A0A0;
                break ;
            case  2 : // -72
                udFeature_RegTH = 0x3FB8B8B8;
                break ;
            case  3 : // -48
                udFeature_RegTH = 0x3FD0D0D0;
                break ;
            case  4 : // -24
                udFeature_RegTH = 0x3FE8E8E8;
                break ;
            case  5 : // 0
                udFeature_RegTH = 0x3F000000;
                break ;
            case  6 : // 24
                udFeature_RegTH = 0x3F181818;
                break ;
            case  7 : // 48
                udFeature_RegTH = 0x3F303030;
                break ;
            case  8 : // 72
                udFeature_RegTH = 0x3F484848;
                break ;
            case  9 : // 96
                udFeature_RegTH = 0x3F606060;
                break ;
            case  10 : // 120
                udFeature_RegTH = 0x3F787878;
                break ;
            
            default :
            	udFeature_RegTH = 0x3F000000 ;
                // printf(" Error !! Feature Threhold out of range in AppFDVT.cpp !!") ;
                break ;
        }
    
    //*******************************************************************************************//
    
    //LOGD("udFeature_RegTH:%d",udFeature_RegTH);
    
    fdvtreg_adr[i+22]=0x0074;
    fdvtreg_value[i+22]=0x3F000000;   //FD3.1 Setting
//    fdvtreg_value[i+22]=udFeature_RegTH;

#if (SVM_OPITION)
    fdvtreg_adr[i+23]=0x0084;
    fdvtreg_value[i+23]=(0x00002200)|((gGfdPatternType<<4)&0x70)|(GFD_SKIP_MODE & 0x07);
    gGfdPatternType++;
    if(gGfdPatternType>GFD_SKIP_MODE)
    {
        gGfdPatternType = 0;
    }
#else
    fdvtreg_adr[i+23]=0x0084;
    fdvtreg_value[i+23]=0x00002200;
#endif
    //LOGD("SVM=%d, Skip Mode Type = [%d %d %d %d %x]",SVM_OPITION,GFD_SKIP_MODE,gGfdPatternType,LFD_SKIP_MODE,LFD_PATTERN_TYPE,fdvtreg_value[i+23]);

    //*************************************************************//
#if SVM_OPITION
    fdvtreg_adr[i+24]=0x007C;
    fdvtreg_value[i+24]=0x01FF000B;
    halFDVT_PARA_SET(fdvtreg_adr,fdvtreg_value,i+25,FDVT_GFD_MODE);
#else
    halFDVT_PARA_SET(fdvtreg_adr,fdvtreg_value,i+24,FDVT_GFD_MODE);
#endif
    
}

void FD_Direction_Selection(int Direction_Support_Number,
			    int Rotate_CW,					//0: CR -> CCR	1: CCR -> CR
			    int *Current_Direction,
			    int *Current_Feature
			   )
{

    int Direction1_Feature_Sequence[5] = {1,2,3,4,5} ;		//    0 degree direction
    int Direction2_Feature_Sequence[5] = {7,5,9,3,11} ;		//  -90 degree direction
    int Direction3_Feature_Sequence[5] = {6,4,8,2,10} ;		//  +90 degree direction
    int Direction4_Feature_Sequence[5] = {12,10,11,8,9} ;	// +180 degree direction
	
    if(Direction_Support_Number == 1){
        *Current_Direction = 1 ;
    }
    else if (Direction_Support_Number == 2){
        if (Rotate_CW == 0)
        {
            if (*Current_Direction == 1)
                *Current_Direction = 3 ;
            else
                *Current_Direction = 1 ;
        }
        else
        {
            if (*Current_Direction == 1)
                *Current_Direction = 2 ;
            else
                *Current_Direction = 1 ;
        }
    }
    else if (Direction_Support_Number == 3){
        if (Rotate_CW == 0)
        {
            if (*Current_Direction == 1)
                *Current_Direction = 3 ;
            else if (*Current_Direction == 3)
                *Current_Direction = 2 ;
            else
                *Current_Direction = 1 ;
        }
        else
        {
            if (*Current_Direction == 1)
                *Current_Direction = 2 ;
            else if (*Current_Direction == 2)
                *Current_Direction = 3 ;
            else
                *Current_Direction = 1 ;
        }
    }
    else{
        if (Rotate_CW == 0){
            if (*Current_Direction == 1)
                *Current_Direction = 3 ;
            else if (*Current_Direction == 3)
                *Current_Direction = 2 ;
            else if (*Current_Direction == 2)
                *Current_Direction = 4 ;
            else
                *Current_Direction = 1 ;
        }
        else{
            if (*Current_Direction == 1)
                *Current_Direction = 2 ;
            else if (*Current_Direction == 2)
                *Current_Direction = 3 ;
            else if (*Current_Direction == 3)
                *Current_Direction = 4 ;
            else
                *Current_Direction = 1 ;
        }
    }
    
    switch (*Current_Direction)
    {                                                                                        
    case 1 :
        *Current_Feature = 1 ;
        break ;
    case 2 :
        *Current_Feature = 7 ;
        break ;
    case 3 :
        *Current_Feature = 6 ;
        break ;
    case 4 :
        *Current_Feature = 12 ;
        break ;
    default :
        printf ("Error !! Current Direction is not correct !! In SoftwareRelatedModule.cpp !") ;
        break ;
    }
}
                                                                                                           
long long FD_DIR_TRANS_FW2HW(int FULL_DET_DIR_DISPLAY_FLAG[],
                             int Current_Feature_Index_in_one_dir_mode,
                             int fatch_quene_num_per_frame
                            )
{
    long long hw_full_fd_direction ;
    long long Hex_direction_value = 0x001;
    int i, j ;

    hw_full_fd_direction = 0 ;


    if (fatch_quene_num_per_frame == 1)
    {
        switch (Current_Feature_Index_in_one_dir_mode)
        {
            case  1 :
                Hex_direction_value = 0x001 ; //  Dec_direction_value = 0 ;
                FULL_DET_DIR_DISPLAY_FLAG[1] = 1 ;
                break ;
            case  6 :
                Hex_direction_value = 0x008 ; //  Dec_direction_value = 3 ;
                FULL_DET_DIR_DISPLAY_FLAG[6] = 1 ;
                break ;
            case  7 :
                Hex_direction_value = 0x200 ; //  Dec_direction_value = 9 ;
                FULL_DET_DIR_DISPLAY_FLAG[7] = 1 ;
                break ;
            case  12 :
                Hex_direction_value = 0x040 ; //  Dec_direction_value = 6 ;
                FULL_DET_DIR_DISPLAY_FLAG[12] = 1 ;
                break ;
            default :                                                                              
                printf(" Error !! Wrong parameter in one-directional mode !!\n") ;
                break ;
        }   

        hw_full_fd_direction = Hex_direction_value ;
    }
    else if (fatch_quene_num_per_frame == 3)
    {
        switch (Current_Feature_Index_in_one_dir_mode)
        {
            case  1 :
                Hex_direction_value = 0x803 ; //  Dec_direction_value = 0 ;
                FULL_DET_DIR_DISPLAY_FLAG[1] = 1 ;
                FULL_DET_DIR_DISPLAY_FLAG[2] = 1 ;
                FULL_DET_DIR_DISPLAY_FLAG[3] = 1 ;
                break ;
            case  6 :
                Hex_direction_value = 0x01c ; //  Dec_direction_value = 3 ;
                FULL_DET_DIR_DISPLAY_FLAG[4] = 1 ;
                FULL_DET_DIR_DISPLAY_FLAG[6] = 1 ;
                FULL_DET_DIR_DISPLAY_FLAG[8] = 1 ;
                break ;
            case  7 :
                Hex_direction_value = 0x700 ; //  Dec_direction_value = 9 ;
                FULL_DET_DIR_DISPLAY_FLAG[5] = 1 ;
                FULL_DET_DIR_DISPLAY_FLAG[7] = 1 ;
                FULL_DET_DIR_DISPLAY_FLAG[9] = 1 ;
                break ;
            case  12 :
                Hex_direction_value = 0x0e0 ; //  Dec_direction_value = 6 ;
                FULL_DET_DIR_DISPLAY_FLAG[10] = 1 ;
                FULL_DET_DIR_DISPLAY_FLAG[11] = 1 ;
                FULL_DET_DIR_DISPLAY_FLAG[12] = 1 ;
                break ;
            default :                                                                              
                printf(" Error !! Wrong parameter in one-directional mode !!\n") ;
                break ;
        }   

        hw_full_fd_direction = Hex_direction_value ;
    }
    else if (fatch_quene_num_per_frame == 30)   // frontal + semi (ROP00)
    {
        switch (Current_Feature_Index_in_one_dir_mode)
        {
            case  1 :
                Hex_direction_value = (((long long)1<< 0) + 
                                       ((long long)1<<12) + 
                                       ((long long)1<<24) );
                FULL_DET_DIR_DISPLAY_FLAG[1] = 1 ;
                break ;
            case  6 :
                Hex_direction_value = (((long long)1<< 3) + 
                                       ((long long)1<<15) + 
                                       ((long long)1<<27) );
                FULL_DET_DIR_DISPLAY_FLAG[6] = 1 ;
                break ;
            case  7 :
                Hex_direction_value = (((long long)1<< 9) + 
                                       ((long long)1<<21) + 
                                       ((long long)1<<33) );
                FULL_DET_DIR_DISPLAY_FLAG[7] = 1 ;
                break ;
            case  12 :
                Hex_direction_value = (((long long)1<< 6) + 
                                       ((long long)1<<18) + 
                                       ((long long)1<<30) );
                FULL_DET_DIR_DISPLAY_FLAG[12] = 1 ;
                break ;
            default :                                                                              
                printf(" Error !! Wrong parameter in one-directional mode !!\n") ;
                break ;
        }   

        hw_full_fd_direction = Hex_direction_value ;
    }
    else if (fatch_quene_num_per_frame == 50)   // ROP00_RIP-30/0/+30 + Semi_RIP00 (5 poses)
    {
        switch (Current_Feature_Index_in_one_dir_mode)
        {
            case  1 :
                Hex_direction_value = (((long long)1<< 0) + 
                                       ((long long)1<< 1) + 
                                       ((long long)1<<11) + 
                                       ((long long)1<<12) + 
                                       ((long long)1<<24) );
                FULL_DET_DIR_DISPLAY_FLAG[1] = 1 ;
                FULL_DET_DIR_DISPLAY_FLAG[2] = 1 ;
                FULL_DET_DIR_DISPLAY_FLAG[3] = 1 ;
                break ;
            case  6 :
                Hex_direction_value = (((long long)1<<( 0+3)) + 
                                       ((long long)1<<( 1+3)) + 
                                       ((long long)1<<( 2  )) + 
                                       ((long long)1<<(12+3)) + 
                                       ((long long)1<<(24+3)) );
                FULL_DET_DIR_DISPLAY_FLAG[4] = 1 ;
                FULL_DET_DIR_DISPLAY_FLAG[6] = 1 ;
                FULL_DET_DIR_DISPLAY_FLAG[8] = 1 ;
                break ;
            case  7 :
                Hex_direction_value = (((long long)1<<( 0+9)) + 
                                       ((long long)1<<( 1+9)) + 
                                       ((long long)1<<( 11 )) + 
                                       ((long long)1<<(12+9)) + 
                                       ((long long)1<<(24+9)) );
                FULL_DET_DIR_DISPLAY_FLAG[5] = 1 ;
                FULL_DET_DIR_DISPLAY_FLAG[7] = 1 ;
                FULL_DET_DIR_DISPLAY_FLAG[9] = 1 ;
                break ;
            case  12 :
                Hex_direction_value = (((long long)1<<( 0+6)) + 
                                       ((long long)1<<( 1+6)) + 
                                       ((long long)1<<( 8  )) + 
                                       ((long long)1<<(12+6)) + 
                                       ((long long)1<<(24+6)) );
                FULL_DET_DIR_DISPLAY_FLAG[10] = 1 ;
                FULL_DET_DIR_DISPLAY_FLAG[11] = 1 ;
                FULL_DET_DIR_DISPLAY_FLAG[12] = 1 ;
                break ;
            default :                                                                              
                //printf(" Error !! Wrong parameter in one-directional mode !!\n") ;
                LOGD(" Error !! Wrong parameter in one-directional mode !!\n") ;
                break ;
        }   

        hw_full_fd_direction = Hex_direction_value ;
    }
    else
    {
        //printf (" parameter error ! fatch_quene_num_per_frame in one-directional tracking mode must be 1 or 3 !! \n") ;
        LOGD(" parameter error ! fatch_quene_num_per_frame in one-directional tracking mode must be 1 or 3 !! \n") ;
    }
   

    return hw_full_fd_direction ;
}

int POSE_TRANS_RFW2HW(int pos)
{ 
    switch (pos)
    {
        case  1 :
            return 0 ;
        case  2 :
            return 1 ;
        case  3 :
            return 11 ;
        case  4 :
            return 2 ;
        case  5 :
            return 10 ;
        case  6 :
            return 3 ;
        case  7 :
            return 9 ;
        case  8 :
            return 4 ;
        case  9 :
            return 8 ;
        case  10 :
            return 5 ;
        case  11 :
            return 7 ;
        case  12 :
            return 6 ;
        default :                                                                              
            //printf(" Error !! Feature pose out of range in main.cpp !!") ;
            LOGD(" Error !! Feature pose out of range in main.cpp !!");
            return 0 ;
    }     
}

void POSE_TRANS_FW2HW(struct result *FD_RESULT)
{
    int m ;

    for (m = 0 ; m < MAX_TRACKING_FACE_NUM ; m ++)
    {
        switch (FD_RESULT[m].rip_dir)
        {
            case  1 :
                FD_RESULT[m].rip_dir = 0 ;
                break ;
            case  2 :
                FD_RESULT[m].rip_dir = 1 ;
                break ;
            case  3 :
                FD_RESULT[m].rip_dir = 11 ;
                break ;
            case  4 :
                FD_RESULT[m].rip_dir = 2 ;
                break ;
            case  5 :
                FD_RESULT[m].rip_dir = 10 ;
                break ;
            case  6 :
                FD_RESULT[m].rip_dir = 3 ;
                break ;
            case  7 :
                FD_RESULT[m].rip_dir = 9 ;
                break ;
            case  8 :
                FD_RESULT[m].rip_dir = 4 ;
                break ;
            case  9 :
                FD_RESULT[m].rip_dir = 8 ;
                break ;
            case  10 :
                FD_RESULT[m].rip_dir = 5 ;
                break ;
            case  11 :
                FD_RESULT[m].rip_dir = 7 ;
                break ;
            case  12 :
                FD_RESULT[m].rip_dir = 6 ;
                break ;
            default :                                                                              
                // printf(" Error !! Feature pose out of range in main.cpp !!") ;
                FD_RESULT[m].rip_dir = 0 ;
                break ;
        }
    }
}
int POSE_TRANS_RHW2FW(int pos)
{ 
    switch (pos)
    {
        case  1 :
            return 0 ;
        case  2 :
            return 1 ;
        case  4 :
            return 2 ;
        case  6 :
            return 3 ;
        case  8 :
            return 4 ;
        case  10 :
            return 5 ;
        case  12 :
            return 6 ;
        case  11 :
            return 7 ;
        case  9 :
            return 8 ;
        case  7 :
            return 9 ;
        case  5 :
            return 10 ;
        case  3 :
            return 11 ;
        default :                                                                              
            //printf(" Error !! Feature pose out of range in main.cpp !!") ;
            LOGD(" Error !! Feature pose out of range in main.cpp !!") ;
            return 0 ;
    }     
}
void POSE_TRANS_HW2FW(struct result *FD_RESULT)
{
    int m ;

    for (m = 0 ; m < MAX_TRACKING_FACE_NUM ; m ++)
    {
        switch (FD_RESULT[m].rip_dir)
        {
            case  0 :
                FD_RESULT[m].rip_dir = 1 ;
                break ;
            case  1 :
                FD_RESULT[m].rip_dir = 2 ;
                break ;
            case  2 :
                FD_RESULT[m].rip_dir = 4 ;
                break ;
            case  3 :
                FD_RESULT[m].rip_dir = 6 ;
                break ;
            case  4 :
                FD_RESULT[m].rip_dir = 8 ;
                break ;
            case  5 :
                FD_RESULT[m].rip_dir = 10 ;
                break ;
            case  6 :
                FD_RESULT[m].rip_dir = 12 ;
                break ;
            case  7 :
                FD_RESULT[m].rip_dir = 11 ;
                break ;
            case  8 :
                FD_RESULT[m].rip_dir = 9 ;
                break ;
            case  9 :
                FD_RESULT[m].rip_dir = 7 ;
                break ;
            case  10 :
                FD_RESULT[m].rip_dir = 5 ;
                break ;
            case  11 :
                FD_RESULT[m].rip_dir = 3 ;
                break ;
            default :
                FD_RESULT[m].rip_dir = 1 ;
                //printf(" Error !! Feature pose out of range in main.cpp !!") ;
                LOGD(" Error !! Feature pose out of range in main.cpp !!") ;
                break ;
        }
    }    
    
}

//-------------------------------------------------------------------------------------------------------------------------
int max(int a, int b)
{
    if (a >= b)
    {
        return a ;
    }
    else
    {
        return b ;
    }
}
//-------------------------------------------------------------------------------------------------------------------------
int min(int a, int b)
{
    if (a <= b)
    {
        return a ;
    }
    else
    {
        return b ;
    }
}
//-------------------------------------------------------------------------------------------------------------------------
void GFD_LFD_INDEX_CHECK(void)
{
    int i, j ;
    int max_x, max_y, min_x, min_y ;
    int fd_result_width, act_window_width ;
    int x_overlap, y_overlap, overlap_area ;
    int act_window_overlap_percentage, fd_result_overlap_percentage ;
    int face_size_label_difference ;

    for (i = 0 ; i < (GFD_NUM + LFD_NUM) ; i++)
    {
        if (FD_RESULT[i].type == 0) // FD candidate
        {
            for (j = 0 ; j < LFD_WINDOW_NUM ; j++) // Tracking result of previous frame
            {
                fd_result_width = FD_RESULT[i].x1 - FD_RESULT[i].x0 ;
                act_window_width = LFD_INFO[j].x1 - LFD_INFO[j].x0 ;

                max_x = max(max( LFD_INFO[j].x0 , LFD_INFO[j].x1) , max( FD_RESULT[i].x0 , FD_RESULT[i].x1 ) ) ;
                min_x = min(min( LFD_INFO[j].x0 , LFD_INFO[j].x1) , min( FD_RESULT[i].x0 , FD_RESULT[i].x1 ) ) ;
                max_y = max(max( LFD_INFO[j].y0 , LFD_INFO[j].y1) , max( FD_RESULT[i].y0 , FD_RESULT[i].y1 ) ) ;
                min_y = min(min( LFD_INFO[j].y0 , LFD_INFO[j].y1) , min( FD_RESULT[i].y0 , FD_RESULT[i].y1 ) ) ;
                    
                x_overlap = fd_result_width + act_window_width - (max_x - min_x) ;
                y_overlap = fd_result_width + act_window_width - (max_y - min_y) ;

                if (x_overlap < 0)
                {
                    x_overlap = 0 ;
                }

                if (y_overlap < 0)
                {
                    y_overlap = 0 ;
                }

                overlap_area = x_overlap * y_overlap ;
    
                act_window_overlap_percentage = (overlap_area * 100) / (act_window_width * act_window_width) ;
                fd_result_overlap_percentage = (overlap_area * 100) / (fd_result_width * fd_result_width) ;
                face_size_label_difference = abs(LFD_INFO[j].size_index - FD_RESULT[i].size_index) ;

                if ( (fd_result_overlap_percentage > 70  || act_window_overlap_percentage > 70) &&
                     abs(fd_result_overlap_percentage - act_window_overlap_percentage) > 50 &&
                     face_size_label_difference > 2
                    )
                {
                    FD_RESULT[i].face_index = MAX_FACE_NUM - 1 ;
                    FD_RESULT[i].type = 3 ;
                    FD_RESULT[i].x0 = 0 ;
                    FD_RESULT[i].y0 = 0 ;
                    FD_RESULT[i].x1 = 0 ;
                    FD_RESULT[i].y1 = 0 ;
                    FD_RESULT[i].fcv = 0 ;
                    FD_RESULT[i].rip_dir = 0 ;
                    FD_RESULT[i].rop_dir = 0 ;
                    FD_RESULT[i].size_index = 0 ;
                    break ;
                }

                if ((fd_result_overlap_percentage > 50  && act_window_overlap_percentage > 50) ||
                    fd_result_overlap_percentage > 70 ||
                    act_window_overlap_percentage > 70
                    )
                {
                    if (face_size_label_difference <= 2)
                    {
                        FD_RESULT[i].type =  1 ;
                        FD_RESULT[i].face_index = LFD_INFO[j].face_index ;
                        TC_RESULT[i].face_index = LFD_INFO[j].face_index ;
                        break ;
                    }
                }
            }
        }

    }
}

void TRACKING_COMPENSATION(
                            int CenterR,
                            int CenterG,
                            int CenterB,
                            int LeftR,
                            int LeftG,
                            int LeftB,
                            int RightR,
                            int RightG,
                            int RightB,
                            int *x0,
                            int *y0,
                            int *x1,
                            int *y1,
                            int *FaceType,
                            int FacePose,
                            float face_div_rg,
                            int face_r_value,
                            int face_g_value,
                            int face_b_value,
                            int *execute_skin_color_track,
                            int *face_statistical_value_differ,
                            int *BOX_COLOR_TRACK_POSITION 
						)
{
	int box_width ;
	float differ ;
	int r_differ, g_differ, b_differ ;
	int r_differ_percent, g_differ_percent, b_differ_percent, differ_percent ;
	float div_rg_matrix[3] = {0};
	int curr_r_value[3] = {0};
	int curr_g_value[3] = {0};
	int curr_b_value[3] = {0};
	
	int n ;
	int current_min_differ ;
	int Total_differ ;
	int shift_length ;

	bool Non_face ;

	Non_face = false ;
	*BOX_COLOR_TRACK_POSITION = 0 ;

	
	for (n = 0 ; n < 3 ; n++)   // 0 : Center  1: Left-hand side  2: Right-hand side
	{
		if (n == 0) // Center
		{
		    curr_r_value[n]  = CenterR ;
		    curr_g_value[n]  = CenterG ;
		    curr_b_value[n]  = CenterB ;
		    div_rg_matrix[n] = (float)CenterR / (float)CenterG ;
		}
		else if (n == 1) // Left-hand side
		{
		    curr_r_value[n]  = LeftR ;
		    curr_g_value[n]  = LeftG ;
		    curr_b_value[n]  = LeftB ;
		    div_rg_matrix[n] = (float)LeftR / (float)LeftG ;
		}
		else // Right-hand side
		{
		    curr_r_value[n]  = RightR ;
		    curr_g_value[n]  = RightG ;
		    curr_b_value[n]  = RightB ;
		    div_rg_matrix[n] = (float)RightR / (float)RightG ;
		}
	}

	box_width = *x1 - *x0 ;
	shift_length = ( box_width * 171 ) >> 10 ; // shift_length = box_width / 6
	current_min_differ = 1000 ;

	for (n = 0 ; n < 3 ; n ++)
	{
		Total_differ = 0 ;

		if ((curr_r_value[n] > 230 && curr_g_value[n] > 230 && curr_b_value[n] > 230) ||
			 (curr_r_value[n] < 30 && curr_g_value[n] < 30 && curr_b_value[n] < 30) )
		{
			//Non_face = true ;
			//current_min_differ = 1000 ;
			continue ;
		}

		if (face_div_rg > div_rg_matrix[n])
		{
			differ = face_div_rg - div_rg_matrix[n] ;
		}
		else
		{
			differ = div_rg_matrix[n] - face_div_rg ;
		}

		if (curr_r_value[n] > face_r_value)
		{
			r_differ = curr_r_value[n] - face_r_value ;
		}
		else
		{
			r_differ = face_r_value - curr_r_value[n] ;
		}

		if (curr_g_value[n] > face_g_value)
		{
			g_differ = curr_g_value[n] - face_g_value ;
		}
		else
		{
			g_differ = face_g_value - curr_g_value[n] ;
		}

		if (curr_b_value[n] > face_b_value)
		{
			b_differ = curr_b_value[n] - face_b_value ;
		}
		else
		{
			b_differ = face_b_value - curr_b_value[n] ;
		}

		differ_percent = (int)(( differ * 1000.0 )/ face_div_rg) ;
		r_differ_percent = (int)(( r_differ * 1000.0 )/ face_r_value) ;
		g_differ_percent = (int)(( g_differ * 1000.0 )/ face_g_value) ;
		b_differ_percent = (int)(( b_differ * 1000.0 )/ face_b_value) ;

		Total_differ = r_differ_percent + g_differ_percent + differ_percent;

		if (Total_differ < current_min_differ && 
			r_differ < RGB_DIFFERENCE_THRESHOLD &&
			g_differ < RGB_DIFFERENCE_THRESHOLD &&
			b_differ < RGB_DIFFERENCE_THRESHOLD &&
			r_differ_percent < 200 &&
			g_differ_percent < 200 &&
			b_differ_percent < 350  
			)
		{
			current_min_differ = Total_differ ;
			*BOX_COLOR_TRACK_POSITION = n ;
		}
	}

	*face_statistical_value_differ = current_min_differ ;

	if (*BOX_COLOR_TRACK_POSITION == 1)
	{
		if (FacePose <= 3 || FacePose >= 10)
		{
			(*x0) -= shift_length ;
			(*x1) -= shift_length ;
		}
		else
		{
			(*y0) -= shift_length ;
			(*y1) -= shift_length ;
		}
	}

	if (*BOX_COLOR_TRACK_POSITION == 2)
	{	
		if (FacePose <= 3 || FacePose >= 10)
		{
			(*x0) += shift_length ;
			(*x1) += shift_length ;
		}
		else
		{
			(*y0) += shift_length ;
			(*y1) += shift_length ;
		}
	}			

	if ((*y0) < 0) { (*y0) = 0 ; }
	if ((*y1) >= IMAGE_HEIGHT) { (*y1) = IMAGE_HEIGHT - 1 ; }
	if ((*x0) < 0) { (*x0) = 0 ; }
	if ((*x1) >= IMAGE_WIDTH) { (*x1) = IMAGE_WIDTH - 1 ; }

	if ( current_min_differ < 700)
	{
		*FaceType = 2 ;
	}
	else
	{
        	*FaceType = 3 ;
	}
	*execute_skin_color_track = 0 ;
}

void Face_Overlap_Detection(int max_tracking_number, int display_flag[],
						  int reliability_value[], struct result *FD_RESULT)	                  
{
	int k, z ;
	int v_dis ;		// vertical distance
	int h_dis ;		// horizontal distance
	int touch_dis, vertical_touch_dis ;
	int face_center_x_k, face_center_y_k ;
	int face_center_x_z, face_center_y_z ;
	int face_width_k, face_width_z ;
	int face_height_k, face_height_z ;

	for (k = 0 ; k < max_tracking_number ; k ++)
	{
		if (display_flag[k] == 1)
		{
			for (z = (k + 1) ; z < max_tracking_number ; z ++ )
			{
				if (display_flag[z] == 1)
				{
					face_center_x_k = (FD_RESULT[k].x0 + FD_RESULT[k].x1) >> 1 ;
					face_center_y_k = (FD_RESULT[k].y0 + FD_RESULT[k].y1) >> 1 ;

					face_center_x_z = (FD_RESULT[z].x0 + FD_RESULT[z].x1) >> 1 ;
					face_center_y_z = (FD_RESULT[z].y0 + FD_RESULT[z].y1) >> 1 ;
					
					face_width_k = FD_RESULT[k].x1 - FD_RESULT[k].x0 ;
					face_width_z = FD_RESULT[z].x1 - FD_RESULT[z].x0 ;

					face_height_k = FD_RESULT[k].y1 - FD_RESULT[k].y0 ;
					face_height_z = FD_RESULT[z].y1 - FD_RESULT[z].y0 ;

					h_dis = abs(face_center_x_k - face_center_x_z) ;
					v_dis = abs(face_center_y_k - face_center_y_z) ;

					touch_dis = (face_width_k + face_width_z) >> 1 ;
					vertical_touch_dis = (face_height_k + face_height_z) >> 1 ;

					if (face_width_k >= face_width_z)
					{
						if ( (touch_dis - h_dis) >= ((face_width_z >> 2) + (face_width_z >> 4)) )
						{
							if (FD_RESULT[z].type == 2 && FD_RESULT[k].type == 2)
							{
								if(reliability_value[z] > reliability_value[k])
								{
									display_flag[z] = 0 ;
								}
								else
								{
									display_flag[k] = 0 ;
								}
							}
							else
							{
								if (FD_RESULT[z].type == 2)
								{
									if ( (vertical_touch_dis - v_dis) >= (face_height_z >> 1) )
									{
										display_flag[z] = 0 ;
									}
								}

								if (FD_RESULT[k].type == 2)
								{
									if ( (vertical_touch_dis - v_dis) >= (face_height_z >> 1) )
									{
										display_flag[k] = 0 ;
									}
								}
							}
						}
					}
					else
					{
						if ( (touch_dis - h_dis) >= ((face_width_k >> 2) + (face_width_k >> 4)) )	//	if ( (touch_dis - h_dis) >= (face_width_k >> 1))
						{
							if (FD_RESULT[z].type == 2 && FD_RESULT[k].type == 2)
							{
								if(reliability_value[z] > reliability_value[k])
								{
									display_flag[z] = 0 ;
								}
								else
								{
									display_flag[k] = 0 ;
								}
							}
							else
							{
								if (FD_RESULT[z].type == 2)
								{
									if ( (vertical_touch_dis - v_dis) >= (face_height_z >> 1) )
									{
										display_flag[z] = 0 ;
									}
								}

								if (FD_RESULT[k].type == 2)
								{
									if ( (vertical_touch_dis - v_dis) >= (face_height_z >> 1) )
									{
										display_flag[k] = 0 ;
									}
								}
							}
						}
					}

				}
			}
		}
	}

	for (k = 0 ; k < max_tracking_number ; k ++)
	{
		if (display_flag[k] == 0)
		{
            		FD_RESULT[k].face_index = max_tracking_number ;
            		FD_RESULT[k].type = 3 ;
			FD_RESULT[k].x0 = 0 ;
			FD_RESULT[k].y0 = 0 ;
			FD_RESULT[k].x1 = 0 ;
			FD_RESULT[k].y1 = 0 ;
	            //FD_RESULT[k].valid = 0 ;
	            FD_RESULT[k].fcv = 0 ;
	            FD_RESULT[k].rop_dir = 0 ;
	            FD_RESULT[k].rip_dir = 0 ;
	            FD_RESULT[k].size_index = 0 ;
		}
	}

}

void FACE_PRIORITY_SORTING(int image_width, int image_height,
							int max_selected_face_number,
							int AutoFocusFaceLabel[],
							int display_flag[], struct result *FD_RESULT)
{
	int i, j ;
	int FaceSelectionPriorityArray[MAX_TRACKING_FACE_NUM+1] ;
	int FaceSelectionIndexArray[MAX_TRACKING_FACE_NUM+1] ;
	int rect_x0, rect_y0, rect_x1, rect_y1 ;
	int rect_center_xpos, rect_center_ypos ;
	int dist_x_work, dist_y_work ;
	int dist_to_center ;
	int size ;
	int priority_value ;
	float dist_to_center_norm ;
	float size_norm ;
	int min_index ;
	int temp ;

	/*----------------------------------------------------------------------------*/
	/*                              Initialization.                               */
	/*----------------------------------------------------------------------------*/
                                                 
	for ( i = 0 ; i < max_selected_face_number; i ++ )
	{                                           
		FaceSelectionPriorityArray [ i ] = 0  ;
		FaceSelectionIndexArray    [ i ] = i  ;
	}                                           
                                                 
	/*----------------------------------------------------------------------------*/
	/*                           Check input condition.                           */
	/*----------------------------------------------------------------------------*/
                                                 
	if ( max_selected_face_number <= 1 )
	{
		return ;                         
	}       
    

	/*----------------------------------------------------------------------------*/
	/*                              Define priority.                              */
	/*----------------------------------------------------------------------------*/
                                                 
	for ( i = 0 ; i < max_selected_face_number ; i ++ )
	{         
	                                         
		rect_x0 = FD_RESULT[ i ].x0 ;
		rect_y0 = FD_RESULT[ i ].y0 ;
		rect_x1 = FD_RESULT[ i ].x1 ;
		rect_y1 = FD_RESULT[ i ].y1 ;
		                                         
		rect_center_xpos = ( rect_x1 + rect_x0 ) >> 1;
		rect_center_ypos = ( rect_y1 + rect_y0 ) >> 1;
		                                         
		dist_x_work = ( (image_width >> 1) - rect_center_xpos );
		dist_y_work = ( (image_height >> 1) - rect_center_ypos );
		                                         
		if ( dist_x_work < 0 ) dist_x_work *= -1;
		if ( dist_y_work < 0 ) dist_y_work *= -1;
		                                         
		dist_to_center = ( image_width + image_height ) - ( dist_x_work + dist_y_work );
		                                         
		size = rect_x1 - rect_x0;              
		                                         
		if ( FDE_FACESELECTIONMODE == 0 )         /* Prior to size.            */
		{
			priority_value = size ;             
		}                                       
		else if ( FDE_FACESELECTIONMODE == 1 )    /* Prior to center.          */
		{
			priority_value = dist_to_center ;
		}                                       
		else                                      /* Size/Center weighting.    */
		{                                       
			// Must be fixed point calculation.  
			dist_to_center_norm = (float)dist_to_center / (float)( image_width + image_height );
			size_norm           = (float)size  / (float)  image_height;
			                                     /* Calculate normalized      */
			                                     /* score.                    */
			                                     
			if ( rect_center_xpos >= FDE_FACESELECTIONAREAX0 &&
			     rect_center_xpos <= FDE_FACESELECTIONAREAX1 &&
			     rect_center_ypos >= FDE_FACESELECTIONAREAY0 &&
			     rect_center_ypos <= FDE_FACESELECTIONAREAY1 )
			 {                                   /* Inside of center          */
			                                     /* rectangle.                */
				priority_value = (int)( dist_to_center_norm * ( 256 - FDE_FACESELECTIONWEIGHT ) ) +
				                     (int)( size_norm * FDE_FACESELECTIONWEIGHT );
			 }                                   
			else                                 
			 {                                   /* Outsize of center         */
			                                     /* rectangle.                */
				priority_value = (int)( size_norm * FDE_FACESELECTIONWEIGHT );
			                                     
			 }                                   
		                                         
		}                                       
		                                         
		FaceSelectionPriorityArray[ i ] = priority_value;
	                                         
	}                                           

	for (i = 0; i < (max_selected_face_number-1); i++) 
	{
		min_index = 0; 

		for (j = 1; j <= (max_selected_face_number-i-1); j++) 
		{
			if (FaceSelectionPriorityArray[j] < FaceSelectionPriorityArray[min_index]) 
			{
				min_index = j; 
			}
		}

		temp = FaceSelectionPriorityArray[min_index];
		FaceSelectionPriorityArray[min_index] = FaceSelectionPriorityArray[max_selected_face_number-i-1];
		FaceSelectionPriorityArray[max_selected_face_number-i-1] = temp;

		temp = FaceSelectionIndexArray[min_index];
		FaceSelectionIndexArray[min_index] = FaceSelectionIndexArray[max_selected_face_number-i-1];
		FaceSelectionIndexArray[max_selected_face_number-i-1] = temp;
	}

	for (i = 0; i < max_selected_face_number ; i++)
	{
		AutoFocusFaceLabel[i] = 0 ;
		if (i == FaceSelectionIndexArray[0])
		{
			AutoFocusFaceLabel[i] = 1 ;
		}

		if (i >= 512)
		{
		        LOGD("max_selected_face_number out off rang %d \n",i);
			//display_flag[FaceSelectionIndexArray[i]] = 0 ;
			//FD_RESULT[FaceSelectionIndexArray[i]].x0 = 0 ;
			//FD_RESULT[FaceSelectionIndexArray[i]].y0 = 0 ;
			//FD_RESULT[FaceSelectionIndexArray[i]].x1 = 0 ;
			//FD_RESULT[FaceSelectionIndexArray[i]].y1 = 0 ;
		}
	}

}

void Smoother(int max_tracking_number, int display_x0[], int display_y0[],
		 	int display_x1[], int display_y1[], int diffx[], int diffy[],
		 	int Box_display_position_update_flag[],
		 	int size_smooth_factor, float smooth_position_thrd,     // 0-1, higher means smoother & long latency
                	float trigger_position_thrd,    // 0-1, higher means less sensitive
                	int display_flag[], struct result *FD_RESULT)
{
    int k ;
    int History_box_wide, Current_box_wide, New_box_wide ;
    int New_center_x, New_center_y ;
    
    //BinChang 20110314
    int wBoundaryProt_x, wBoundaryProt_y;    
    
//    float   smooth_position_thrd    ;
//    float   trigger_position_thrd   ;
    //LOGD("[Smoother] IN");
    for (k = 0 ; k < max_tracking_number ; k ++)
    {
        //LOGD("[Smoother] k=%d Box_display_position_update_flag %d\n",k,Box_display_position_update_flag[k]);
        //LOGD("[Smoother] Before k %d x0 = %d y0 = %d x1 = %d y1=%d \n",k,FD_RESULT[k].x0,FD_RESULT[k].y0,FD_RESULT[k].x1,FD_RESULT[k].y1); 		    
                
        if((FD_RESULT[k].x1==0)&&(FD_RESULT[k].y1==0))     
        {
            display_x0[k]=0;
            display_y0[k]=0;
            display_x1[k]=0;
            display_y1[k]=0;
            continue;            
        }    
	if (Box_display_position_update_flag[k] == 1)
	{

            History_box_wide = display_x1[k] - display_x0[k] ;
            Current_box_wide = FD_RESULT[k].x1 - FD_RESULT[k].x0 ;
            New_box_wide = ((size_smooth_factor * History_box_wide) + ((100 - size_smooth_factor) * Current_box_wide)) / 100 ;

            // New_center_x = (FD_RESULT[k].x0 + FD_RESULT[k].x1) >> 1 ;
            // New_center_y = (FD_RESULT[k].y0 + FD_RESULT[k].y1) >> 1 ;
            
            if((display_x0[k]==0) || (FD_RESULT[k].type == 0)){
                New_center_x = (FD_RESULT[k].x0 + FD_RESULT[k].x1) >> 1 ;
                New_center_y = (FD_RESULT[k].y0 + FD_RESULT[k].y1) >> 1 ;
                
                diffx[k] = 0    ;      
                diffy[k] = 0    ;   

                New_box_wide = Current_box_wide ;
            }              
            else{
                diffx[k] = (int)( (1-smooth_position_thrd)*( ((FD_RESULT[k].x0+FD_RESULT[k].x1)>>1) - ((display_x0[k]+display_x1[k])>>1) ) + smooth_position_thrd*diffx[k] ) ;
                diffy[k] = (int)( (1-smooth_position_thrd)*( ((FD_RESULT[k].y0+FD_RESULT[k].y1)>>1) - ((display_y0[k]+display_y1[k])>>1) ) + smooth_position_thrd*diffy[k] ) ;   

                New_center_x = ((display_x1[k]+display_x0[k])>>1) + diffx[k]    ;
                New_center_y = ((display_y1[k]+display_y0[k])>>1) + diffy[k]    ;
                //LOGD("[Smoother2]\n");
            }
            //LOGD("[Smoother] FD_RESULT[k].type %d \n",FD_RESULT[k].type);
            if (FD_RESULT[k].type == 0||FD_RESULT[k].type == 2)
            {
                //LOGD("[Smoother3]\n");
                display_x0[k] = FD_RESULT[k].x0 ;
		display_y0[k] = FD_RESULT[k].y0 ;
		display_x1[k] = FD_RESULT[k].x1 ;
		display_y1[k] = FD_RESULT[k].y1 ;
            }
            else
            {
                if( (fabs(float(New_center_x-((display_x0[k]+display_x1[k])>>1))/(float)(display_x1[k]-display_x0[k]))>trigger_position_thrd) ||
                    (fabs(float(New_center_y-((display_y0[k]+display_y1[k])>>1))/(float)(display_y1[k]-display_y0[k]))>trigger_position_thrd) ){
                    	
                    	
                	//BinChang 20110314 Coordiate Protection: Start
                	wBoundaryProt_x  = New_center_x - (New_box_wide >> 1) ;
                	wBoundaryProt_y  = New_center_y - (New_box_wide >> 1) ;	
                     if(wBoundaryProt_x <0)
                		New_center_x -=wBoundaryProt_x;
                    if(wBoundaryProt_y <0)
                    	New_center_y -=wBoundaryProt_y;
                 	
                 	wBoundaryProt_x  = New_center_x + (New_box_wide >> 1) ;
                	wBoundaryProt_y  = New_center_y + (New_box_wide >> 1) ;
                	if(wBoundaryProt_x >IMAGE_WIDTH)
                		New_center_x -= (wBoundaryProt_x -  IMAGE_WIDTH);
                    if(wBoundaryProt_y >IMAGE_HEIGHT)
                    	New_center_y -= (wBoundaryProt_y - IMAGE_HEIGHT);
                	//BinChang 20110314 Coordiate Protection: End                    	
                    	
                    	
                    display_x0[k] = New_center_x - (New_box_wide >> 1) ;
		    display_x1[k] = New_center_x + (New_box_wide >> 1) ;
		    display_y0[k] = New_center_y - (New_box_wide >> 1) ;
		    display_y1[k] = New_center_y + (New_box_wide >> 1) ;
                }
                //LOGD("[Smoother] Before disx0 = %d disy0 = %d disx1 = %d disy1=%d \n",display_x0[k],display_y0[k],display_x1[k],display_y1[k]); 		    
                
                FD_RESULT[k].x0=display_x0[k];
                FD_RESULT[k].y0=display_y0[k];
                FD_RESULT[k].x1=display_x1[k];
                FD_RESULT[k].y1=display_y1[k];
                //LOGD("[Smoother] After x0 = %d y0 = %d x1 = %d y1=%d \n",FD_RESULT[k].x0,FD_RESULT[k].y0,FD_RESULT[k].x1,FD_RESULT[k].y1); 		    		                    
            }

       }
       /*
       if (display_flag[k] == 0)
       {
       	    display_x0[k] = 0 ;
       	    display_y0[k] = 0 ;
            display_x1[k] = 0 ;
       	    display_y1[k] = 0 ;
       }*/
       
       Box_display_position_update_flag[k] = 1 ;
   }
   //LOGD("[Smoother] OUT");
}

void FDGetHWResult(FDVT_OPERATION_MODE_ENUM fd_state)
{
	int i;
	MUINT32 reg_fdvt_result;
	struct result *fd_rst;
	struct tc_result *tc_rst;
	int total_fd_num;
	int total_tc_num;
	MUINT32 reg_val;
	
	halFDVT_Wait_IRQ();
	
	if(fd_state == FDVT_GFD_MODE)
	{
            fd_rst = FD_RESULT;
            tc_rst = TC_RESULT;
            fdvtreg_adr[0]=FDVT_RESULTNUM;
            halFDGetFaceResult(fdvtreg_adr,fdvtreg_value,1,reg_fdvt_result);
            
            GFD_NUM = (reg_fdvt_result & REG_FDVT_GFD_NUM_RESULT_MASK);
            LFD_NUM = ((reg_fdvt_result & REG_FDVT_LFD_NUM_RESULT_MASK)>>16);
            LOGD("HW_Result: GFD_NUM 0x%x LFD_NUM 0x%x \n",GFD_NUM,LFD_NUM);
            fdvtreg_adr[0]=FDVT_RESULT;
            halFDGetFaceResult(fdvtreg_adr,fdvtreg_value,1,reg_fdvt_result);
            for(i=0;i<15;i++)
            {
            	LFD_INFO[i].valid = ((reg_fdvt_result >> (16+i)) & 0x1);
            }
            
            total_fd_num = GFD_NUM + LFD_NUM;
            
            for(i=0;i<total_fd_num;i++)
            {
                reg_val = hw_fd_result_datava[2*i];
            	fd_rst[i].x1 = ((reg_val>>27)&0x01f);
            	fd_rst[i].y0 = ((reg_val>>19)&0x0ff);
            	fd_rst[i].x0 = ((reg_val>>10)&0x1ff);
            	fd_rst[i].type = ((reg_val>>9)&0x001);
            	fd_rst[i].face_index = ((reg_val>>0)&0x1ff);
            	reg_val = hw_fd_result_datava[2*i+1];
            	fd_rst[i].size_index = ((reg_val>>27)&0x01f);
            	fd_rst[i].rop_dir = ((reg_val>>24)&0x007);
            	fd_rst[i].rip_dir = ((reg_val>>20)&0x0f);
            	fd_rst[i].fcv = ((reg_val>>12)&0x0ff);
            	fd_rst[i].y1 = ((reg_val>>4)&0x0ff);
            	fd_rst[i].x1 |= (((reg_val>>0)&0xf)<<5);
            	//LOGD("GFD_NUM 0x%x LFD_NUM 0x%x fd_rst[i].x0 %d fd_rst[i].y0 %d\n",GFD_NUM,LFD_NUM,fd_rst[i].x0,fd_rst[i].y0);
            	
            }
            
            total_tc_num = GFD_NUM + LFD_WINDOW_NUM;
            //LOGD("GFD_NUM %d LFD_WINDOW_NUM %d total_tc_num %d\n",total_tc_num);
            for(i=0;i<total_tc_num;i++)
            {
            	reg_val = hw_tc_result_datava[4*i];
            	tc_rst[i].LeftB = ((reg_val>>24)&0x0ff);
            	tc_rst[i].CenterR = ((reg_val>>16)&0x0ff);
            	tc_rst[i].CenterG = ((reg_val>>8)&0x0ff);
            	tc_rst[i].CenterB = ((reg_val>>0)&0x0ff);
            	
            	reg_val = hw_tc_result_datava[4*i+1];
            	tc_rst[i].face_index = ((reg_val>>17)&0x1ff);  
            	tc_rst[i].type = ((reg_val>>16)&0x001);              
            	tc_rst[i].LeftR = ((reg_val>>8)&0x0ff);
            	tc_rst[i].LeftG = ((reg_val>>0)&0x0ff);
            	
            	reg_val = hw_tc_result_datava[4*i+2];
            	tc_rst[i].RightR = ((reg_val>>16)&0x0ff);                   
            	tc_rst[i].RightG = ((reg_val>>8)&0x0ff);                    
            	tc_rst[i].RightB = ((reg_val>>0)&0x0ff);
            }                                        
	}
	else
	{
	    fdvtreg_adr[0]=FDVT_RESULTNUM;
	    halFDGetFaceResult(fdvtreg_adr,fdvtreg_value,1,reg_fdvt_result);                        
            SD_NUM = ((reg_fdvt_result & REG_FDVT_LFD_NUM_RESULT_MASK)>>16);
	}
}
#ifdef SmileDetect
void SDRegisterConfig(void* src_image_data)
{
    int i, idx;
    
    for(i=0;i<15;i++){
         idx = 0x00a4+i*12+0;
         fdvtreg_adr[i*3]=idx;
         fdvtreg_value[i*3]=
         (int)((SD_LFD_INFO[i].valid     &0x0001)<< 28   )+ 
         (int)((SD_LFD_INFO[i].size_index&0x001f)<< 20   )+ 
         (int)((SD_LFD_INFO[i].rop_dir   &0x0007)<< 16   )+ 
         (int)((SD_LFD_INFO[i].rip_dir   &0x000f)<< 12   )+ 
         (int)((SD_LFD_INFO[i].face_index&0x01ff)<<  0   );
         fdvtreg_adr[(i*3+1)]=idx+4;
         fdvtreg_value[(i*3+1)]=
         (int)((SD_LFD_INFO[i].y0        &0x00ff)<< 16   )+ 
         (int)((SD_LFD_INFO[i].x0        &0x01ff)<<  0   );
         fdvtreg_adr[(i*3+2)]=idx+8;
         fdvtreg_value[(i*3+2)]=
         (int)((SD_LFD_INFO[i].y1        &0x00ff)<< 16   )+ 
         (int)((SD_LFD_INFO[i].x1        &0x01ff)<<  0   );                      
    }
    i*=3;
    fdvtreg_adr[i]=0x0078;                       
    fdvtreg_value[i]=(REG_RMAP&0xFFFFFFFE);
    
    fdvtreg_adr[i+1]=0x0088;
    //fdvtreg_value[i+1]= (0x1<<20)+SD_LFD_WINDOW_NUM;
    fdvtreg_value[i+1]= (0x4<<24) +(0x1<<20)+SD_LFD_WINDOW_NUM;
    
    fdvtreg_adr[i+2]=0x0004;
    fdvtreg_value[i+2]=FF_ENABLE+FD_ENABLE;
    
    fdvtreg_adr[i+3]=0x0054;
    fdvtreg_value[i+3]=(kal_uint32)learning_data0ph;

    fdvtreg_adr[i+4]=0x0058;
    fdvtreg_value[i+4]=(kal_uint32)learning_data1ph;
    
    fdvtreg_adr[i+5]=0x005C;
    fdvtreg_value[i+5]=(kal_uint32)learning_data2ph;
    
    fdvtreg_adr[i+6]=0x0060;
    fdvtreg_value[i+6]=(kal_uint32)learning_data3ph;
    
    fdvtreg_adr[i+7]=0x0064;
    fdvtreg_value[i+7]=(kal_uint32)learning_data4ph;
    
    fdvtreg_adr[i+8]=0x0068;
    fdvtreg_value[i+8]=(kal_uint32)learning_data5ph;
    
    fdvtreg_adr[i+9]=0x006C;
    fdvtreg_value[i+9]=(kal_uint32)learning_data6ph;
    
    fdvtreg_adr[i+10]=0x0070;
    fdvtreg_value[i+10]=(kal_uint32)learning_data7ph;
        
    //halFDVT_PARA_SET(fdvtreg_adr,fdvtreg_value,i+11,FDVT_SD_MODE);
    
    fdvtreg_adr[i+11]=0x0074;
    fdvtreg_value[i+11]=0x3F000000;
       
    halFDVT_PARA_SET(fdvtreg_adr,fdvtreg_value,i+12,FDVT_SD_MODE);
}
#endif

#if defined (MTK_M4U_SUPPORT)
void AppFDVT::SyncM4UCache(FDVTM4UInfo m4uinfo)
{
    pM4UDrv->m4u_cache_sync(M4U_CLNTMOD_FD , M4U_CACHE_FLUSH_BEFORE_HW_READ_MEM , m4uinfo.virtAddr, m4uinfo.size);
}

MINT32 AppFDVT::allocM4UMemory(MUINT32 virtAddr, MUINT32 size, MUINT32 *m4uVa)
{
    if (pM4UDrv == NULL)  {
        LOGD("Null M4U driver \n"); 
        return -1;
    }
    MINT32 ret = 0;  

    LOGD("[allocM4UMemory] virtAddr = 0x%x, size = %d \n", virtAddr, size); 
    M4U_MODULE_ID_ENUM eM4U_ID = M4U_CLNTMOD_FD;
    M4U_PORT_STRUCT port;
    ret=pM4UDrv->m4u_alloc_mva(eM4U_ID , virtAddr,  size , m4uVa);
    if(ret!=0)
    	 LOGD("[freeM4UMemory] m4u_alloc_mva fail \n"); 
    pM4UDrv->m4u_manual_insert_entry(eM4U_ID , *m4uVa, true);
    pM4UDrv->m4u_insert_tlb_range(eM4U_ID, *m4uVa,  *m4uVa + size-1 , RT_RANGE_HIGH_PRIORITY,0);
    LOGD("[allocM4UMemory] m4uVa = 0x%x \n", *m4uVa); 

    //port.ePortID = M4U_PORT_JPG_DEC_FDVT;
    port.ePortID = M4U_PORT_FD0;
    port.Virtuality = 1;
    port.Security = 0;
    port.Distance = 1;
    port.Direction = 0;
    ret = pM4UDrv->m4u_config_port(&port);
    
    //port.ePortID = M4U_PORT_FDVT_OUT1; 
    port.ePortID = M4U_PORT_FD2;
    port.Virtuality = 1;
    port.Security = 0;
    port.Distance = 1;
    port.Direction = 0;
    ret = pM4UDrv->m4u_config_port(&port);
    
    //port.ePortID = M4U_PORT_FDVT_OUT2;
    port.ePortID = M4U_PORT_FD1;
    port.Virtuality = 1;
    port.Security = 0;
    port.Distance = 1;
    port.Direction = 0;
    ret = pM4UDrv->m4u_config_port(&port);
    
    return ret; 
}


/*******************************************************************************
*
********************************************************************************/
MINT32 AppFDVT::freeM4UMemory(MUINT32 m4uVa, MUINT32 size,MUINT32 m4uMVa)
{
    if (pM4UDrv == NULL)  {
        LOGD("Null M4U driver \n"); 
        return -1;
    } 
    MINT32 ret = 0; 
    LOGD("[freeM4UMemory] E \n"); 
    LOGD("[freeM4UMemory] m4uVa = 0x%x, size = %d \n", m4uVa, size); 
    ret = pM4UDrv->m4u_invalid_tlb_range( M4U_CLNTMOD_FD, m4uMVa , (m4uMVa+size));
    if(ret!=0)
    	 LOGD("[freeM4UMemory] m4u_invalid_tlb_range fail \n"); 
    ret = pM4UDrv->m4u_dealloc_mva( M4U_CLNTMOD_FD, m4uVa , size, m4uMVa);

    return ret; 
}
#endif


BOOL Mean_Shift(UINT8* RGB_Candidate, unsigned short int width, unsigned short int height, FACEDETECT_RECT *pRect, short int *num_face, unsigned char m_flag[], 
				float *V, float axisX, float axisY)
{
	unsigned char *RGB_Target_t;
	unsigned short int half_length, temp1=0;
	unsigned int seeker_center;//, weight_length;
	unsigned short int Bha_Coef_current, Bha_Coef_next, similarity[3];
	int Y1, Y1_X, Y1_Y, temp2=0, Location[3];
	unsigned char scale;
	int time=0;
	int i,m;
	INT16 length,top,left;
	unsigned short int face_sum = 0,TH, TH_Scale;
	UINT8 Loop_Count;

//**************************************************************************
    char str_dstfile[128], str_dstfile_1[128], str_dstfile_2[128];
    FILE *out_file, *out_file1;
    int write_size;

//**************************************************************************
//Step 2

     if((*num_face <0) || (*num_face >2))
     {
     	*num_face = 0;
		return FALSE;
	}
     
	for(m=0;m<*num_face;m++)
	{
		Loop_Count = 0;

		if(m_flag[m]==1)
		{
		scale=0;

		length = pRect[m].wWidth;
		left   = pRect[m].wLeft ;
		top    = pRect[m].wTop  ;
		
		if(length %2 == 0)
			length++;
        
		half_length = length>>1;

		seeker_center = (top* width + half_length * width + left + half_length) * 2;

		//weight histogram accumulator
		if( ((seeker_center - ( half_length*width+half_length)*2) >=0) && (length >=24))
                RGB_Target_t = RGB_Candidate + seeker_center - ( half_length*width+half_length)*2;
          else
          {
          	*num_face = 0;
		      return FALSE;
          }
          	      
		//Candidate Histogram
		
		Compute_Weight_Histogram(RGB_Target_t, Histogram_Candidate, width,length);

	    //**************************************************************************
	    //Step 3: weight histogram similarity 
		Bha_Coef_current = 0;

		for(i=0;i<num;i++)
			Bha_Coef_current = Bha_Coef_current + abs(Histogram_Target[i] - Histogram_Candidate[i]);

        //******************  For testing case  ***********************
MS_LOOP:
        //*************************************************************
		Compute_new_cordinate(RGB_Target_t, Histogram_Target, Histogram_Candidate, width, length, &Y1_X, &Y1_Y, V, axisX, axisY);
		
		//Check Boundary
          top = top + Y1_X;
          left = left + Y1_Y;
          
	     if( (top < 0) || (left < 0) || ( (top + length) > height ) ||  ( (left + length) > width ) )
	     {
              Y1_X = 0;
              Y1_Y = 0;
           }

	    //**************************************************************************
	    //Step 6:  Similar to Step 2 

		//Part 1: Find Histogram_Candidate

		if(Y1_X==0 && Y1_Y==0)
		{
			Y1 = seeker_center;
			Bha_Coef_next = Bha_Coef_current;
		}
		else
		{
			Y1 = seeker_center + (Y1_X *width+Y1_Y)*2;


		      if(  ((Y1 - (half_length*width+half_length)*2) >=0) && (length >=24))
                      RGB_Target_t = RGB_Candidate + seeker_center - ( half_length*width+half_length)*2;
                else
                {
          	      *num_face = 0;
		            return FALSE;
                }

			//RGB_Target_t = RGB_Candidate + Y1 - (half_length*width+half_length)*2;
			Compute_Weight_Histogram(RGB_Target_t, Histogram_Candidate, width, length);

			//Part 2: Find Bha_Coef_next
			Bha_Coef_next = 0;

			for(i=0;i<num;i++)
				Bha_Coef_next = Bha_Coef_next + abs(Histogram_Target[i] - Histogram_Candidate[i]);

			//**************************************************************************
			//Step 7

				if(Bha_Coef_next > Bha_Coef_current)
				{
					Y1_X = Y1_X/2;
					Y1_Y = Y1_Y/2;
					Y1 =  seeker_center + (Y1_X *width + Y1_Y) * 2;
				}
			
			//**************************************************************************
			//Step 8
				if(  (abs(Y1_X) + abs(Y1_Y) ) > 0 )
				{
					Loop_Count++;
					seeker_center = Y1;
					RGB_Target_t = RGB_Candidate + seeker_center - ( half_length*width+half_length)*2;
					Bha_Coef_current = Bha_Coef_next;
					if(Loop_Count <= 10)
						goto MS_LOOP;	
					else	
						Loop_Count = 0;
				}
		}
	    //**************************************************************************

	    Y1 = Y1 / 2;
		temp2 = Y1/width;
		if(Bha_Coef_next <= Ratio)
		{
			pRect[m].wWidth  = length;
			pRect[m].wHeight = length;
			pRect[m].wTop    = temp2 - half_length;
			pRect[m].wLeft   = Y1 - temp2 * width - half_length;
			
			if( (pRect[m].wTop < 0) || (pRect[m].wLeft < 0) || ( (pRect[m].wTop + length) > height ) ||  ( (pRect[m].wLeft + length) > width ) )
			{
			     pRect[m].wWidth  = 0;
			     pRect[m].wHeight = 0;
			     pRect[m].wTop    = 0;
			     pRect[m].wLeft   = 0;
			     m_flag[m]=0;
			}
			else
			{
			     face_sum++;
			     m_flag[m]=1;
			}
			
		
		}
		else
		{
			pRect[m].wWidth  = 0;
			pRect[m].wHeight = 0;
			pRect[m].wTop    = 0;
			pRect[m].wLeft   = 0;
			m_flag[m]=0;
		}
    }
}	// for face count

	//if(face_sum > 0)
	if(face_sum == 1)
	{
		//*num_face = face_sum;
		*num_face = 1;
		return TRUE;
	}
	else
	{
		*num_face = 0;
		return FALSE;
	}
}
