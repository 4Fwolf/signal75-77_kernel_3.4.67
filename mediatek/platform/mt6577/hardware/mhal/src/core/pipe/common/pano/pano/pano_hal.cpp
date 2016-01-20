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
#define LOG_TAG "mHalPano"

#include "../inc/MediaLog.h"
#include "../inc/MediaAssert.h"

#include "AppPano.h"
#include "pano_hal.h"
#include "pano_hal_base.h"

/*******************************************************************************
*
********************************************************************************/
static CAMERA_PANO_PROCESS_STRUCT stPanoParam;
static CAMERA_PANO_PROCESS_STRUCT *pstPanoParam = &stPanoParam;
static CAMERA_PANO_RESULT_STRUCT stPanoResult;
static CAMERA_PANO_RESULT_STRUCT *pstPanoResult = &stPanoResult;


static halPANOBase *pHalPANO = NULL;

/*******************************************************************************
*
********************************************************************************/
halPANOBase*
halPANO::
getInstance()
{
    MHAL_LOG("[halPANO] getInstance \n");
    if (pHalPANO == NULL) {
        pHalPANO = new halPANO();
    }
    return pHalPANO;
}

/*******************************************************************************
*
********************************************************************************/
void   
halPANO::
destroyInstance() 
{
    if (pHalPANO) {
        delete pHalPANO;
    }
    pHalPANO = NULL;
}

/*******************************************************************************
*                                            
********************************************************************************/
halPANO::halPANO()
{
    m_pMTKPanoObj = NULL;    
}


halPANO::~halPANO()
{    
}

/*******************************************************************************
*
********************************************************************************/
kal_int32 halPANO::mHalPanoCalcStitch(
)
{
    kal_int32 err = S_Pano_OK;
    PANO_STATE_ENUM state;

    MHAL_LOG("[mHalPanoCalcStitch] \n");

    m_pMTKPanoObj->camera_pano_add_image(pstPanoParam);

    state = PANO_ADD_IMAGE_STATE;
    while (state != PANO_READY_STATE) {
        state = m_pMTKPanoObj->camera_pano_process();
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
kal_int32 halPANO::mHalPanoDoStitch(
)
{
    kal_int32 err = S_Pano_OK;
    PANO_STATE_ENUM state;

    MHAL_LOG("[mHalPanoDoStitch] \n");

    m_pMTKPanoObj->camera_pano_stitch(pstPanoParam);

    state = PANO_ADD_IMAGE_STATE;
    while (state != PANO_READY_STATE) {
        state = m_pMTKPanoObj->camera_pano_process();
    }

    pstPanoResult = m_pMTKPanoObj->camera_pano_get_result();

    return err;
}

/*******************************************************************************
*
********************************************************************************/
kal_int32 halPANO::mHalPanoSetParam(
    kal_int32 param0,
    kal_int32 param1,
    kal_int32 param2,
    kal_int32 param3
)
{
    kal_int32 err = S_Pano_OK;

    switch (param0) {
    case HAL_PANO_RESET_PARAM:
        memset(pstPanoParam, 0, sizeof(CAMERA_PANO_PROCESS_STRUCT));
        pstPanoParam->snapshot_number = 2;
        pstPanoParam->pano_blend_scale = 8;
        pstPanoParam->jpeg_dst_type = IMG_COLOR_FMT_RGB565;
        memset(pstPanoResult, 0, sizeof(CAMERA_PANO_RESULT_STRUCT));
        break;
    case HAL_PANO_SET_DST_DIR:
        pstPanoParam->pano_direction = (PANO_DIRECTION_ENUM) param1;
        break;
    case HAL_PANO_SET_DST_BUF:
        pstPanoParam->extmem_start_address = (kal_uint32) param1;
        pstPanoParam->extmem_size = (kal_uint32) param2;
        break;
    case HAL_PANO_SET_SRC_DIM:
        pstPanoParam->source_width = (kal_uint32) param1;
        pstPanoParam->source_height = (kal_uint32) param2;
        break;
    case HAL_PANO_SET_SRC_BUF:
        pstPanoParam->jpeg_src_buffer_addr[param1] = (kal_uint8 *) param2;
        break;
    default:
        MHAL_ASSERT(0, "Err");
        break;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
kal_int32 halPANO::mHalPanoGetParam(
    kal_int32 param0,
    kal_int32 *param1,
    kal_int32 *param2,
    kal_int32 *param3
)
{
    kal_int32 err = S_Pano_OK;

    switch (param0) {
    case HAL_PANO_GET_RESULT:
        *param1 = (kal_int32) pstPanoResult->jpeg_src_buffer_ptr;
        *param2 = (kal_int32) pstPanoResult->target_width;
        *param3 = (kal_int32) pstPanoResult->target_height;
        break;
    default:
        MHAL_ASSERT(0, "Err");
        break;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
kal_int32
halPANO::mHalPanoInit(
)
{
    kal_int32 err = S_Pano_OK;

    MHAL_LOG("[mHalPanoInit] \n");

    if (m_pMTKPanoObj) {
        err = E_Pano_ERR;
        MHAL_LOG("[mHalPanoInit] Err, Init has been called \n");
    }
    m_pMTKPanoObj = new AppPano();
    MHAL_ASSERT(m_pMTKPanoObj != NULL, "Err");

    memset(pstPanoParam, 0, sizeof(CAMERA_PANO_PROCESS_STRUCT));

    return err;
}

/*******************************************************************************
*
********************************************************************************/
kal_int32
halPANO::mHalPanoUninit(
)
{
    MHAL_LOG("[mHalPanoUninit] \n");

    if (m_pMTKPanoObj) {
        m_pMTKPanoObj->camera_pano_exit();
        delete m_pMTKPanoObj;
    }
    m_pMTKPanoObj = NULL;

    return ERROR_NONE;
}


