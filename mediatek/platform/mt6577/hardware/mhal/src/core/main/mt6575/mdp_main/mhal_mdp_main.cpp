/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
#define LOG_TAG "MDP"

#include "MediaHal.h"
#include "MediaTypes.h"                 // For MHAL_ERROR_ENUM
#include "mhal_interface.h"             // For Mt6575_mHalBitblt
#ifdef MDP_FLAG_1_SUPPORT_MHALJPEG
#include "mhal_jpeg.h"                  // For MHAL JPEG
#endif


#include <mdp_service.h>
#include <cutils/properties.h>

#define FIELD_CPY( _to, _from, _field ) \
    _to->_field = _from->_field;


static int _RegisterLoopMem_ConvertFromMhal(    mHalMVALOOPMEM_CLIENT*          p_mhal_client_id,   /*in*/
                                                mHalRegisterLoopMemory_t*       p_mhal_param,       /*in*/
                                                mHalRegisterLoopMemoryObj_t*    p_mhal_out_obj,     /*in*/
                                                MVALOOPMEM_CLIENT*              p_client_id,        /*out*/
                                                RegisterLoopMemory_t*           p_param,            /*out*/
                                                RegisterLoopMemoryObj_t*        p_out_obj           /*out*/
                                            )
{

    if( ( p_mhal_client_id != NULL ) && ( p_client_id != NULL ) )
    {
        switch( *p_mhal_client_id )
        {
        case MHAL_MLM_CLIENT_SFTEX:        //Surface flinger Texture
            *p_client_id = MLM_CLIENT_SFTEX;
            break;
        case MHAL_MLM_CLIENT_MTKOVERLAY:
            *p_client_id = MLM_CLIENT_MTKOVERLAY;
            break;
        case MHAL_MLM_CLIENT_PV2ND:        //Preview 2nd path
            *p_client_id = MLM_CLIENT_PV2ND;
            break;
        case MHAL_MLM_CLIENT_ELEMENT:      //MDP Element
            *p_client_id = MLM_CLIENT_ELEMENT;
            break;
        case MHAL_MLM_CLIENT_PVCPY:        //Camera preview path extra-copy
            *p_client_id = MLM_CLIENT_PVCPY;
            break;
        case MHAL_MLM_CLIENT_GRALLOC:      //Gralloc Buffer Allocator
            *p_client_id = MLM_CLIENT_GRALLOC;
            break;
        case MHAL_MLM_CLIENT_MAX:           //MAX (for test)
            *p_client_id = MLM_CLIENT_MAX;
            break;
        default:
            MDP_ERROR("mHalMVALOOPMEM_CLIENT id not found.%d\n", *p_mhal_client_id );
            return -1;
        }
    }


    if( ( p_mhal_param != NULL ) && ( p_param != NULL ) )
    {
        FIELD_CPY( p_param, (REGLOOPMEM_TYPE)p_mhal_param, mem_type );
        FIELD_CPY( p_param, p_mhal_param, addr );
        FIELD_CPY( p_param, p_mhal_param, buffer_size );
        FIELD_CPY( p_param, p_mhal_param, mhal_color );
        FIELD_CPY( p_param, p_mhal_param, img_size.w );
        FIELD_CPY( p_param, p_mhal_param, img_size.h );
        FIELD_CPY( p_param, p_mhal_param, img_roi.x );
        FIELD_CPY( p_param, p_mhal_param, img_roi.y );
        FIELD_CPY( p_param, p_mhal_param, img_roi.w );
        FIELD_CPY( p_param, p_mhal_param, img_roi.h );
        FIELD_CPY( p_param, p_mhal_param, rotate ); //0:0 1:90 2:180 3:270.rotate always 0 when used by RDMA(input memory)

        //TODO:this is not a good mapping
        //memcpy( p_param, p_mhal_param , sizeof( RegisterLoopMemory_t ) );
    }
    
    if( ( p_mhal_out_obj != NULL ) && ( p_out_obj != NULL ) )
    {

        FIELD_CPY( p_out_obj, p_mhal_out_obj, mdp_id );

        FIELD_CPY( p_out_obj, p_mhal_out_obj, calc_addr[0].y );
        FIELD_CPY( p_out_obj, p_mhal_out_obj, calc_addr[0].u );
        FIELD_CPY( p_out_obj, p_mhal_out_obj, calc_addr[0].v );
        FIELD_CPY( p_out_obj, p_mhal_out_obj, calc_addr[0].y_buffer_size );
        FIELD_CPY( p_out_obj, p_mhal_out_obj, calc_addr[0].u_buffer_size );
        FIELD_CPY( p_out_obj, p_mhal_out_obj, calc_addr[0].v_buffer_size );
        
        FIELD_CPY( p_out_obj, p_mhal_out_obj, adapt_addr[0].y );
        FIELD_CPY( p_out_obj, p_mhal_out_obj, adapt_addr[0].u );
        FIELD_CPY( p_out_obj, p_mhal_out_obj, adapt_addr[0].v );
        FIELD_CPY( p_out_obj, p_mhal_out_obj, adapt_addr[0].y_buffer_size );
        FIELD_CPY( p_out_obj, p_mhal_out_obj, adapt_addr[0].u_buffer_size );
        FIELD_CPY( p_out_obj, p_mhal_out_obj, adapt_addr[0].v_buffer_size );

        FIELD_CPY( p_out_obj, p_mhal_out_obj, adapt_m4u_flag_bit );
        FIELD_CPY( p_out_obj, p_mhal_out_obj, alloc_mva_flag_bit );
        
        //TODO:this is not a good mapping
        //memcpy( p_out_obj, p_mhal_out_obj , sizeof( RegisterLoopMemoryObj_t ) );
    }

    return 0;
    
    
}


static int _RegisterLoopMem_ConvertFromMdp(     mHalMVALOOPMEM_CLIENT*          p_mhal_client_id,   /*out*/
                                                mHalRegisterLoopMemory_t*       p_mhal_param,       /*out*/
                                                mHalRegisterLoopMemoryObj_t*    p_mhal_out_obj,     /*out*/
                                                MVALOOPMEM_CLIENT*              p_client_id,        /*in*/
                                                RegisterLoopMemory_t*           p_param,            /*in*/
                                                RegisterLoopMemoryObj_t*        p_out_obj           /*in*/
                                            )
{

    if( ( p_mhal_client_id != NULL ) && ( p_client_id != NULL ) )
    {
        switch( *p_client_id )
        {
        case MLM_CLIENT_SFTEX:        //Surface flinger Texture
            *p_mhal_client_id = MHAL_MLM_CLIENT_SFTEX;
            break;
        case MLM_CLIENT_MTKOVERLAY:
            *p_mhal_client_id = MHAL_MLM_CLIENT_MTKOVERLAY;
            break;
        case MLM_CLIENT_PV2ND:        //Preview 2nd path
            *p_mhal_client_id = MHAL_MLM_CLIENT_PV2ND;
            break;
        case MLM_CLIENT_ELEMENT:      //MDP Element
            *p_mhal_client_id = MHAL_MLM_CLIENT_ELEMENT;
            break;
        case MLM_CLIENT_PVCPY:        //Camera preview path extra-copy
            *p_mhal_client_id = MHAL_MLM_CLIENT_PVCPY;
            break;
        case MLM_CLIENT_GRALLOC:      //Gralloc Buffer Allocator
            *p_mhal_client_id = MHAL_MLM_CLIENT_GRALLOC;
        case MLM_CLIENT_MAX:           //MAX (for test)
            *p_mhal_client_id = MHAL_MLM_CLIENT_MAX;
            break;
        default:
            MDP_ERROR("MVALOOPMEM_CLIENT id not found.%d\n", *p_client_id );
            return -1;
        }
    }


    if( ( p_mhal_param != NULL ) && ( p_param != NULL ) )
    {
        
        FIELD_CPY( p_mhal_param, (mHalREGLOOPMEM_TYPE)p_param,  mem_type );
        FIELD_CPY( p_mhal_param, p_param,  addr );
        FIELD_CPY( p_mhal_param, p_param,  buffer_size );
        FIELD_CPY( p_mhal_param, p_param,  mhal_color );
        FIELD_CPY( p_mhal_param, p_param,  img_size.w );
        FIELD_CPY( p_mhal_param, p_param,  img_size.h );
        FIELD_CPY( p_mhal_param, p_param,  img_roi.x );
        FIELD_CPY( p_mhal_param, p_param,  img_roi.y );
        FIELD_CPY( p_mhal_param, p_param,  img_roi.w );
        FIELD_CPY( p_mhal_param, p_param,  img_roi.h );
        FIELD_CPY( p_mhal_param, p_param,  rotate ); //0:0 1:90 2:180 3:270.rotate always 0 when used by RDMA(input memory)

        //TODO:this is not a good mapping
        //memcpy( p_mhal_param, p_param,  , sizeof( mHalRegisterLoopMemory_t ) );
    }
    
    if( ( p_mhal_out_obj != NULL ) && ( p_out_obj != NULL ) )
    {
        
        FIELD_CPY( p_mhal_out_obj, p_out_obj,  mdp_id );

        FIELD_CPY( p_mhal_out_obj, p_out_obj,  calc_addr[0].y );
        FIELD_CPY( p_mhal_out_obj, p_out_obj,  calc_addr[0].u );
        FIELD_CPY( p_mhal_out_obj, p_out_obj,  calc_addr[0].v );
        FIELD_CPY( p_mhal_out_obj, p_out_obj,  calc_addr[0].y_buffer_size );
        FIELD_CPY( p_mhal_out_obj, p_out_obj,  calc_addr[0].u_buffer_size );
        FIELD_CPY( p_mhal_out_obj, p_out_obj,  calc_addr[0].v_buffer_size );
        
        FIELD_CPY( p_mhal_out_obj, p_out_obj,  adapt_addr[0].y );
        FIELD_CPY( p_mhal_out_obj, p_out_obj,  adapt_addr[0].u );
        FIELD_CPY( p_mhal_out_obj, p_out_obj,  adapt_addr[0].v );
        FIELD_CPY( p_mhal_out_obj, p_out_obj,  adapt_addr[0].y_buffer_size );
        FIELD_CPY( p_mhal_out_obj, p_out_obj,  adapt_addr[0].u_buffer_size );
        FIELD_CPY( p_mhal_out_obj, p_out_obj,  adapt_addr[0].v_buffer_size );

        FIELD_CPY( p_mhal_out_obj, p_out_obj,  adapt_m4u_flag_bit );
        FIELD_CPY( p_mhal_out_obj, p_out_obj,  alloc_mva_flag_bit );

        //TODO:this is not a good mapping
        //memcpy( p_mhal_out_obj, p_out_obj,  , sizeof( mHalRegisterLoopMemoryObj_t ) );
    }

    return 0;
    
    
}


static int _Ipc_GetM4uHandle( unsigned long *p_m4u_handle, unsigned long *p_adj )
{
    return MdpDrv_GetTaskStruct( p_m4u_handle, p_adj );
}

static int _Ipc_PutM4uHandle( unsigned long adj )
{
    return MdpDrv_PutTaskStruct( adj );
}






signed int mHalMdp_BitBlt( void *a_pInBuffer )
{
    
    #if 0
    
    return mHalMdpIpc_BitBlt( (mHalBltParam_t*) a_pInBuffer );

    #else

    return mHalMdpLocal_BitBlt( (mHalBltParam_t*) a_pInBuffer );
    
    #endif
}

#define RETRY_COUNT 20
signed int mHalPostProc(    mHalBltParam_t* p_parameter , 
                            unsigned long bIsRemote , 
                            android::sp<android::IMdpService> mdpService )
{
    void * pWorkingBuff = NULL;
    mHalBltParam_t tempParam;
    unsigned long u4ResidualWidth;
    unsigned long u4Index = 0;
    unsigned long u4Index2 = 0;
    unsigned long u4BPP = 0;
    unsigned long u4InitOffset = 0;
    unsigned long u4Stride = 0;
    unsigned long u4ChunkSize = 0;
    unsigned long u4Case = 0;
    unsigned long u4BuffSize = 0;
    int           ret_val = 0;

//Debug
    unsigned long u4Src , u4Dst;

    if(((1 & p_parameter->srcW) || (16 > p_parameter->srcW) || (16 > p_parameter->srcH)) && 
    (p_parameter->srcAddr == p_parameter->dstAddr) && (p_parameter->dstFormat < MHAL_FORMAT_RGBA_8888))
    {
        switch(p_parameter->dstFormat)
        {
            case MHAL_FORMAT_RGB_888 :
            case MHAL_FORMAT_BGR_888 :
                u4BPP = 3;
            break;
            case MHAL_FORMAT_RGB_565 :
            case MHAL_FORMAT_BGR_565 :
                u4BPP = 2;
            break;
            case MHAL_FORMAT_ABGR_8888 :
            case MHAL_FORMAT_ARGB_8888 :
            case MHAL_FORMAT_BGRA_8888 :
            case MHAL_FORMAT_RGBA_8888 :
                u4BPP = 4;
            break;
            default :
                if(0x10 & p_parameter->doImageProcess)
                {
                    MDP_ERROR("PostProc format does not match:%d , Skip this frame :w:%d , h:%d" , p_parameter->dstFormat , p_parameter->srcW , p_parameter->srcH);
                    return MDP_ERROR_CODE_FAIL;
                }
                else
                {
                    MDP_ERROR("PostProc format does not match:%d , run normal bitblt :w:%d , h:%d" , p_parameter->dstFormat , p_parameter->srcW , p_parameter->srcH);
                    p_parameter->doImageProcess = 0;
                    goto _Normal_MdpPipeImageTransform_Func;
                }
            break;
        }

        memcpy(&tempParam , p_parameter , sizeof(mHalBltParam_t));

        //allocates 16 x image height memory space , suppose ARGB8888
        if(16 > p_parameter->srcW)
        {
            if(16 > p_parameter->srcH)
            {
                //(w < 16, h < 16) + (1 & w , w < 16, h < 16) case
                //16x16 box
                u4BuffSize = (u4BPP<<8);
                u4Case = 3;

            }
            else
            {
                //(w < 16 , h > 16 , 1 & w) + (w < 16 , h > 16 , !(1 & w)) case
                //H residule : residulex16
                u4BuffSize = (p_parameter->srcH << 4)*u4BPP;
                u4Case = 1;
            }
        }
        else
        {
            if(16 > p_parameter->srcH)
            {
                //(h < 16) + (h < 16 , 1 & w) case
                //V residule : 16xresidule
                u4BuffSize = (1 & p_parameter->srcW ? ((p_parameter->srcW+1) << 4) : ((p_parameter->srcW) << 4))*u4BPP;
                u4Case = 2;
            }
            else
            {
                // 1 & w case
                //H residule : residulex16
                u4BuffSize = (p_parameter->srcH << 4)*u4BPP;
                u4Case = 1; 
            }
        }

        pWorkingBuff = malloc(u4BuffSize);
        if(NULL == pWorkingBuff)
        {
            if(0x10 & p_parameter->doImageProcess)
            {
                MDP_ERROR("No memory space to do odd width post process , skip this frame :w:%d , h:%d" , p_parameter->srcW , p_parameter->srcH);
                return MDP_ERROR_CODE_FAIL;
            }
            else
            {
                MDP_ERROR("No memory space to do odd width post process , run normal path :w:%d , h:%d" , p_parameter->srcW , p_parameter->srcH);
                p_parameter->doImageProcess = 0;
                goto _Normal_MdpPipeImageTransform_Func;
            }
        }
        memset(pWorkingBuff , 0 , u4BuffSize);

        //process large part first
        tempParam.srcW = ((p_parameter->srcW >> 4) << 4);
        tempParam.dstW = tempParam.srcW;
        u4ResidualWidth = (p_parameter->srcW - tempParam.srcW);

        if((0 < tempParam.srcW) && (16 < tempParam.srcH))
        {
            if(bIsRemote)
            {
                ret_val = mdpService->BitBlt( &tempParam );
            }
            else
            {
                ret_val = Mt6575_mHalBitblt( &tempParam );
            }
      
            if(MDP_ERROR_CODE_OK != ret_val)
            {
                free(pWorkingBuff);
                if(0x10 & p_parameter->doImageProcess)
                {
                    MDP_DEBUG("Post proc failed in making main part , skip this frame :w:%d , h:%d" , p_parameter->srcW , p_parameter->srcH);
                    return MDP_ERROR_CODE_FAIL;
                }
                else
                {
                    MDP_DEBUG("Post proc failed in making main part , run normal path :w:%d , h:%d" , p_parameter->srcW , p_parameter->srcH);
                    p_parameter->doImageProcess = 0;
                    goto _Normal_MdpPipeImageTransform_Func;
                }
            }
        }

        //Copy out
        //Use memcpy to avoid no DMA resource.
        if(1 == u4Case)
        {
            //H band
            u4InitOffset = (tempParam.srcW + tempParam.srcX + tempParam.srcY*tempParam.srcWStride)*u4BPP;
            u4Stride = (u4BPP << 4);//16 pixels
            u4ChunkSize = (u4ResidualWidth*u4BPP);
        }
        else if(2 == u4Case)
        {
            //V band
            u4InitOffset = (tempParam.srcX + tempParam.srcY*tempParam.srcWStride)*u4BPP;
            u4Stride = (1 & p_parameter->srcW ? (p_parameter->srcW+1) : (p_parameter->srcW))*u4BPP;
            u4ChunkSize = (p_parameter->srcW*u4BPP);
        }
        else
        {
            // 16x16 box
            u4InitOffset = (tempParam.srcX + tempParam.srcY*tempParam.srcWStride)*u4BPP;
            u4Stride = (u4BPP << 4);
            u4ChunkSize = (p_parameter->srcW*u4BPP);
        }

        for(u4Index = 0 ; u4Index < p_parameter->srcH ; u4Index += 1)
        {
            memcpy((void *)((char *)pWorkingBuff + (u4Index*u4Stride)) , (void *)((char *)p_parameter->srcAddr + u4InitOffset + u4Index*p_parameter->srcWStride*u4BPP) , u4ChunkSize);
        }

        //Padding in H for YUV422 process and sharpness
        for(u4Index = 0 ; u4Index < p_parameter->srcH ; u4Index += 1)
        {
            for(u4Index2 = u4ChunkSize ; u4Index2 < u4Stride; u4Index2 += u4BPP)
            {
                memcpy((void *)((char *)pWorkingBuff + u4Index*u4Stride + u4Index2) , 
                    (void *)((char *)pWorkingBuff + u4ChunkSize + u4Index*u4Stride - u4BPP) ,
                    (u4BPP));
            }
        }

        //Padding in V for sharpness
        if(16 > p_parameter->srcH)
        {
            for(u4Index = p_parameter->srcH ; u4Index < 16 ; u4Index += 1)
            {
                memcpy((void *)((char *)pWorkingBuff + u4Index*u4Stride) ,
                    (void *)((char *)pWorkingBuff + (p_parameter->srcH-1)*u4Stride) ,
             	      u4Stride );
            }
        }

        //process residule part
        //to make sure it always success, we use memcpy.
        tempParam.srcX = 0;
        tempParam.srcY = 0;
        tempParam.srcW = (1 & u4Case ? 16 : p_parameter->srcW);// 0x1 or 0x3
        tempParam.srcW = ((1 & tempParam.srcW) ? (tempParam.srcW + 1) : tempParam.srcW);//Align to even
        tempParam.srcH = (2 & u4Case ? 16 : p_parameter->srcH);// 0x2 or 0x3
        tempParam.srcWStride = tempParam.srcW;
        tempParam.srcHStride = tempParam.srcH;

        tempParam.dstW = tempParam.srcW;
        tempParam.dstH = tempParam.srcH;
        tempParam.pitch = tempParam.srcW;

        tempParam.srcAddr = (unsigned long)pWorkingBuff;
        tempParam.dstAddr = (unsigned long)pWorkingBuff;

//Debug - S
#if 0
{
FILE *fp;
char filename [60];
sprintf(filename , "/dump/PartInn%d_%d_%d_%d_%d.raw" , g_Cnt , tempParam.dst_img_size.w , tempParam.dst_img_roi.h , u4ChunkSize , p_parameter->src_img_roi.h);
fp = fopen( filename, "wb");
unsigned long size = (tempParam.dst_img_size.w*tempParam.dst_img_roi.h*u4BPP);

if(  fp == NULL )
{
    MDP_ERROR("Open file failed.:%s\n", strerror(errno));
    goto _EXIT_MdpPipeImageTransform_Func;
}

if( fwrite( (void*)pWorkingBuff , 1 , size , fp) < size )
{
    MDP_ERROR("write file failed.:%s\n", strerror(errno));
}
 
fclose(fp);
MDP_ERROR("Part dump %d : pitch:%d , vp:%d , w:%d , h : %d" , g_Cnt , tempParam.dst_img_size.w , tempParam.dst_img_roi.h , u4ChunkSize , p_parameter->src_img_roi.h);
}
#endif
//Debug - E

        u4Index = 0;

        if(bIsRemote)
        {
            ret_val = mdpService->BitBlt( &tempParam );
            
            while((MDP_ERROR_CODE_OK != ret_val) && (RETRY_COUNT > u4Index))
            {
                ret_val = mdpService->BitBlt( &tempParam );
                u4Index += 1;
                MDP_DEBUG("mdpservice bitblt failed, retry :%lu" , u4Index);
            }
        }
        else
        {
            ret_val = Mt6575_mHalBitblt( &tempParam );

            while((MDP_ERROR_CODE_OK != ret_val) && (RETRY_COUNT > u4Index))
            {
                ret_val = Mt6575_mHalBitblt( &tempParam );
                u4Index += 1;
                MDP_DEBUG("mhalbitblt failed, retry :%lu" , u4Index);
            }
        }

        if(MDP_ERROR_CODE_OK != ret_val)
        {
            free(pWorkingBuff);
            if(0x10 & p_parameter->doImageProcess)
            {
                MDP_DEBUG("Post proc failed in making residual part , skip this frame :w:%d , h:%d" , p_parameter->srcW , p_parameter->srcH);
                return MDP_ERROR_CODE_FAIL;
            }
            else
            {
                MDP_DEBUG("Post proc failed in making residual part , run normal path :w:%d , h:%d" , p_parameter->srcW , p_parameter->srcH);
                p_parameter->doImageProcess = 0;
                goto _Normal_MdpPipeImageTransform_Func;
            }
        }

        //Copy in
        //Use memcpy to avoid no DMA resource.
        for(u4Index = 0 ; u4Index < p_parameter->srcH ; u4Index += 1)
        {
            memcpy((void *)((char *)p_parameter->srcAddr + u4InitOffset + u4Index*p_parameter->srcWStride*u4BPP) , (void *)((char *)pWorkingBuff + (u4Index*u4Stride)) , u4ChunkSize);
        }

//Debug - S
#if 0
{
FILE *fp;
char filename [60];
sprintf(filename , "/dump/PartOut%d_%d_%d_%d_%d.raw" , g_Cnt , tempParam.dst_img_size.w , tempParam.dst_img_roi.h , u4ChunkSize , p_parameter->src_img_roi.h);
fp = fopen( filename, "wb");
unsigned long size = (tempParam.dst_img_size.w*tempParam.dst_img_roi.h*u4BPP);

if(  fp == NULL )
{
    MDP_ERROR("Open file failed.:%s\n", strerror(errno));
    goto _EXIT_MdpPipeImageTransform_Func;
}

if( fwrite( (void*)pWorkingBuff , 1 , size , fp) < size )
{
    MDP_ERROR("write file failed.:%s\n", strerror(errno));
}
 
fclose(fp);
MDP_ERROR("Part dump %d : pitch:%d , vp:%d , w:%d , h : %d" , g_Cnt , tempParam.dst_img_size.w , tempParam.dst_img_roi.h , u4ChunkSize , p_parameter->src_img_roi.h);
g_Cnt += 1;
}
#endif
//Debug - E

        //free memory
        free(pWorkingBuff);
    }
    else
    {
        if(bIsRemote)
        {
            ret_val = mdpService->BitBlt( p_parameter );
        }
        else
        {
            ret_val = Mt6575_mHalBitblt( p_parameter );
        }

        if(MDP_ERROR_CODE_OK != ret_val)
        {
            free(pWorkingBuff);
            if(0x10 & p_parameter->doImageProcess)
            {
                MDP_DEBUG("Post proc failed , skip this frame :w:%d , h:%d" , p_parameter->srcW , p_parameter->srcH);
                return MDP_ERROR_CODE_FAIL;
            }
            else
            {
                MDP_DEBUG("Post proc failed , go normal transform :w:%d , h:%d" , p_parameter->srcW , p_parameter->srcH);
                p_parameter->doImageProcess = 0;
                goto _Normal_MdpPipeImageTransform_Func;
            }
        }

    }

    return 0;

_Normal_MdpPipeImageTransform_Func:
    if(bIsRemote)
    {
        ret_val = mdpService->BitBlt( p_parameter );
    }
    else
    {
        ret_val = Mt6575_mHalBitblt( p_parameter );
    }

    return ret_val;

}

signed int mHalMdpIpc_BitBlt( mHalBltParam_t* bltParam )
{
    
#if defined(MDP_FLAG_PROFILING)
    MdpDrv_Watch _MdpDrvWatch;
    static unsigned long total_time = 0;
    static unsigned long frame_count = 0;
    static unsigned long avg_time_elapse = 0;
#endif
    unsigned long m4u_handle = 0;
    unsigned long adj = 0;


    int ret = MHAL_NO_ERROR;
    

    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_IT );

    
    android::sp<android::IServiceManager> sm = android::defaultServiceManager();
    //android::sp<android::IBinder> binder = sm->getService(android::String16("media.mdp_service"));
    android::sp<android::IBinder> binder = sm->checkService(android::String16("media.mdp_service"));
    android::sp<android::IMdpService> mdpService = android::interface_cast<android::IMdpService>(binder);

    if( bltParam == NULL )
    {
        MDP_ERROR("bltParam is NULL\n");
        return -1;
    }

    if( (mdpService.get() == NULL) || (sm.get() == NULL) || (binder.get() == NULL) )
    {
        MDP_ERROR(  "pointer is NULL. sm is 0x%08X.binder is 0x%08X.mdpService is 0x%08X\n",
                    (unsigned int)sm.get(), (unsigned int)binder.get(), (unsigned int)mdpService.get() );
        return -1;
    }


    /*Fill task pointer here!! & Set ADJ = 0 to prevent LMK               */
    _Ipc_GetM4uHandle( &m4u_handle, &adj );
    bltParam->m4u_handle = m4u_handle;

    if( m4u_handle == 0 )   MDP_ERROR("m4u_handle = 0x%08X\n", (unsigned int)m4u_handle );
    //if( adj == 0 )          MDP_ERROR("adj = 0x%08X\n", (unsigned int)adj );
    


    MDP_INFO_PERSIST("[IPC]BitBlt:Client (m4u_handle = 0x%08X doImageProcess = %d)\n",
                      (unsigned int)bltParam->m4u_handle , (int)bltParam->doImageProcess );

    if( bltParam->doImageProcess ){
        ret = mHalPostProc( bltParam, 1, mdpService );
    } else {
        ret = mdpService->BitBlt( bltParam );
    }

    /*Restore ADJ */
    _Ipc_PutM4uHandle( adj );
    

    

    
    MDPDRV_WATCHSTOP( MDPDRV_WATCH_LEVEL_IT, "mHalMdpIpc_BitBlt", &total_time, &frame_count, &avg_time_elapse, 30 );

    return ret;
}


signed int mHalMdpLocal_BitBlt( mHalBltParam_t* bltParam )
{
    
#if defined(MDP_FLAG_PROFILING)
            MdpDrv_Watch _MdpDrvWatch;
            static unsigned long total_time = 0;
            static unsigned long frame_count = 0;
            static unsigned long avg_time_elapse = 0;
#endif

    int ret = MHAL_NO_ERROR;

    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_IT );


    if( bltParam->doImageProcess ){
        ret = mHalPostProc( bltParam, 0, NULL );
    } else {
        ret = Mt6575_mHalBitblt( bltParam );
    }

    
    MDPDRV_WATCHSTOP( MDPDRV_WATCH_LEVEL_IT, "mHalMdpLocal_BitBlt", &total_time, &frame_count, &avg_time_elapse, 30 );

    return ret;
}



signed int mHalMdp_BitBltEx( mHalBltParamEx_t* p_param )
{
    int ret = MHAL_NO_ERROR;
    
    if( Mdp_BitBltEx( p_param ) != 0 )
    {
        ret = MHAL_UNKNOWN_ERROR;
    }

    return ret;
}


signed int mHalMdp_BitbltSlice( void *a_pInBuffer )
{
    
    int ret = MHAL_NO_ERROR;
    
    if( Mdp_BitbltSlice( a_pInBuffer ) != 0 )
    {
        ret = MHAL_UNKNOWN_ERROR;
    }

    return ret;
}


int mHalMdp_RegisterLoopMemory( mHalMVALOOPMEM_CLIENT client_id, mHalRegisterLoopMemory_t* p_param, mHalRegisterLoopMemoryObj_t* p_out_obj )
{
    int ret_val;
    MVALOOPMEM_CLIENT       mdp_client_id;
    RegisterLoopMemory_t    mdp_param;
    RegisterLoopMemoryObj_t mdp_out_obj;

    
    ret_val =  _RegisterLoopMem_ConvertFromMhal(    &client_id,         //mHalMVALOOPMEM_CLIENT*          p_mhal_client_id,   /*in*/
                                                    p_param,            //mHalRegisterLoopMemory_t*       p_mhal_param,       /*in*/
                                                    p_out_obj,          //mHalRegisterLoopMemoryObj_t*    p_mhal_out_obj,     /*in*/
                                                    &mdp_client_id,     //MVALOOPMEM_CLIENT*              p_client_id,        /*out*/
                                                    &mdp_param,         //RegisterLoopMemory_t*           p_param,            /*out*/
                                                    &mdp_out_obj );     //RegisterLoopMemoryObj_t*        p_out_obj           /*out*/
    if( ret_val < 0 )  {
        MDP_ERROR("convert param from mhal error\n");
        return -1;
    }
    
    
    ret_val = Mdp_RegisterLoopMemory( mdp_client_id, &mdp_param, &mdp_out_obj );

    
    if( ret_val < 0 )  {
        MDP_ERROR("mdp register loop memory error\n");
        return -1;
    }

    
    ret_val =  _RegisterLoopMem_ConvertFromMdp (    NULL,           //mHalMVALOOPMEM_CLIENT*          p_mhal_client_id,   /*out*/
                                                    NULL,           //mHalRegisterLoopMemory_t*       p_mhal_param,       /*out*/
                                                    p_out_obj,      //mHalRegisterLoopMemoryObj_t*    p_mhal_out_obj,     /*out*/
                                                    NULL,           //MVALOOPMEM_CLIENT*              p_client_id,        /*in*/
                                                    NULL,           //RegisterLoopMemory_t*           p_param,            /*in*/
                                                    &mdp_out_obj ); //RegisterLoopMemoryObj_t*        p_out_obj           /*in*/
    if( ret_val < 0 )  {
        MDP_ERROR("convert param from mdp error\n");
        return -1;
    }

    return 0;
    
}

int mHalMdp_UnRegisterLoopMemory( mHalMVALOOPMEM_CLIENT client_id, mHalRegisterLoopMemoryObj_t* p_obj )
{
    
    int ret_val;
    MVALOOPMEM_CLIENT       mdp_client_id;
    RegisterLoopMemoryObj_t mdp_out_obj;
    
    ret_val =  _RegisterLoopMem_ConvertFromMhal(    &client_id,         //mHalMVALOOPMEM_CLIENT*          p_mhal_client_id,   /*in*/
                                                    NULL,               //mHalRegisterLoopMemory_t*       p_mhal_param,       /*in*/
                                                    p_obj,              //mHalRegisterLoopMemoryObj_t*    p_mhal_out_obj,     /*in*/
                                                    &mdp_client_id,     //MVALOOPMEM_CLIENT*              p_client_id,        /*out*/
                                                    NULL,               //RegisterLoopMemory_t*           p_param,            /*out*/
                                                    &mdp_out_obj );     //RegisterLoopMemoryObj_t*        p_out_obj           /*out*/
    if( ret_val < 0 )  {
        MDP_ERROR("convert param from mhal error\n");
        return -1;
    }

   
    return Mdp_UnRegisterLoopMemory( mdp_client_id, &mdp_out_obj );
    
}




MINT32
mHalJpeg(
    MUINT32 a_u4CtrlCode,
    MVOID *a_pInBuffer,
    MUINT32 a_u4InBufSize,
    MVOID *a_pOutBuffer,
    MUINT32 a_u4OutBufSize,
    MUINT32 *pBytesReturned
)
{
    
    MINT32 err = MHAL_NO_ERROR;
    MHAL_JPEG_DEC_SRC_IN *inParam;

#ifdef MDP_FLAG_1_SUPPORT_MHALJPEG

    android::sp<android::IServiceManager> sm = android::defaultServiceManager();
    //android::sp<android::IBinder> binder = sm->getService(android::String16("media.mdp_service"));
    android::sp<android::IBinder> binder = sm->checkService(android::String16("media.mdp_service"));
    android::sp<android::IMdpService> mdpService = android::interface_cast<android::IMdpService>(binder);

    if((mdpService.get() == NULL) || (sm.get() == NULL) || (binder.get() == NULL))
    {
        MDP_ERROR(  "pointer is NULL. sm is 0x%08X.binder is 0x%08X.mdpService is 0x%08X\n",
                    (unsigned int)sm.get(), (unsigned int)binder.get(), (unsigned int)mdpService.get() );
        return MHAL_INVALID_DRIVER;
    }
    
    switch (a_u4CtrlCode) {
        
    case MHAL_IOCTL_JPEG_DEC_START:
        //err = mHalJpgDecStart((MHAL_JPEG_DEC_START_IN*)a_pInBuffer);
        err = mdpService->decodeJpg((MHAL_JPEG_DEC_START_IN*)a_pInBuffer, NULL);
        break;

    case MHAL_IOCTL_JPEG_DEC_GET_INFO:
        //err = mHalJpgDecGetInfo((MHAL_JPEG_DEC_INFO_OUT*)a_pOutBuffer);
        err = mdpService->getJpgInfo((MHAL_JPEG_DEC_INFO_OUT*)a_pOutBuffer);
        break;

    case MHAL_IOCTL_JPEG_DEC_PARSER:
        //err = mHalJpgDecParser((unsigned char*)a_pInBuffer, a_u4InBufSize);
        inParam = (MHAL_JPEG_DEC_SRC_IN *)a_pInBuffer;
        err = mdpService->parseJpg(inParam->srcBuffer, inParam->srcLength, inParam->srcFD);
        break;   
        
    default:
        err = JPEG_ERROR_INVALID_CTRL_CODE;
        break;
    }

#endif

    return err;     
}
