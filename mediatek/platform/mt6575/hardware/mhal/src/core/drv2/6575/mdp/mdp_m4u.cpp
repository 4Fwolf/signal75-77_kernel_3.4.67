/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
#define LOG_TAG "MDP"

#include "mdp_drv.h"
#include "mdp_m4u.h"

//mutex
#include <pthread.h>



/*/////////////////////////////////////////////////////////////////////////////
    Macro Re-Definition
  /////////////////////////////////////////////////////////////////////////////*/

#ifdef  MDP_FLAG_1_SUPPORT_M4U


static MTKM4UDrv* p_m4udrv_ = NULL;


static pthread_mutex_t mutex_m4udrv = PTHREAD_MUTEX_INITIALIZER;

static void ENTER_CRITICAL_M4UDRV( void )
{
    pthread_mutex_lock ( &mutex_m4udrv );
    //MDP_INFO("M4UDRV MUTEX LOCK\n");
}

static void EXIT_CRITICAL_M4UDRV( void )
{
    //MDP_INFO("M4UDRV MUTEX UN-LOCK\n");
    pthread_mutex_unlock ( &mutex_m4udrv );
}


int MdpM4u_Init( void )
{
    int ret = 0;
    
    ENTER_CRITICAL_M4UDRV();

    if( p_m4udrv_ == NULL )
    {
    p_m4udrv_ = new MTKM4UDrv;

    if( p_m4udrv_ == NULL )
            ret = -1;
    }

    EXIT_CRITICAL_M4UDRV();

    return ret;
}

int MdpM4u_Release( void )
{
    ENTER_CRITICAL_M4UDRV();
    
    if( p_m4udrv_ != NULL )
        delete p_m4udrv_;
    p_m4udrv_ = NULL;

    EXIT_CRITICAL_M4UDRV();

    return 0;
}


int MdpM4u_Start(   M4U_MODULE_ID_ENUM module_id, M4U_PORT_ID_ENUM port_id,
                    unsigned long va_addr, unsigned long va_size, 
                    unsigned long* p_mva_addr , int* p_b_alloc_mva ,
                    unsigned long  m4u_handle )
{
    M4U_MODULE_ID_ENUM  query_module_id = M4U_CLNTMOD_UNKNOWN;
    M4U_STATUS_ENUM status;
    int          ret_val = 0;
    unsigned int entryCount = 0;
    
    #if defined(MDP_FLAG_PROFILING)
            MdpDrv_Watch _MdpDrvWatch;
            static unsigned long tt0;//total time;
            static unsigned long fc0;//frame count;
            static unsigned long avt0;//avg_time_elapse;
    #endif

    
    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_DRV );
    
    if( ( p_m4udrv_ == NULL) || ( p_b_alloc_mva == NULL ))
    {
        MDP_ERROR("p_m4udrv_(0x%08X) or p_b_is_cached_mva(0x%08X) is NULL.(m4u_handle = 0x%08X)\n", 
            (unsigned int)p_m4udrv_, (unsigned int)p_b_alloc_mva , (unsigned int)m4u_handle );
        return -1;
    }

    *p_b_alloc_mva = 0;

    switch( module_id )
    {
        case M4U_CLNTMOD_RDMA0:
        case M4U_CLNTMOD_RDMA1:
        #if defined(MDP_FLAG_2_M4U_SUPPORT_QUERY)
        case M4U_CLNTMOD_RDMA_GENERAL:
        #endif
            entryCount = 0;
            #if defined(MDP_FLAG_2_M4U_SUPPORT_QUERY)
            query_module_id = M4U_CLNTMOD_RDMA_GENERAL;
            #endif
            break;
        case M4U_CLNTMOD_ROT1:
        case M4U_CLNTMOD_ROT2:
        case M4U_CLNTMOD_ROT4:
        case M4U_CLNTMOD_ROT0:
        case M4U_CLNTMOD_ROT3:
        #if defined(MDP_FLAG_2_M4U_SUPPORT_QUERY)
        case M4U_CLNTMOD_ROT_GENERAL:
        #endif
            entryCount = 0;//1; /*Temparary value*/
            #if defined(MDP_FLAG_2_M4U_SUPPORT_QUERY)
            query_module_id = M4U_CLNTMOD_ROT_GENERAL;
            #endif
            break;
        default:
            MDP_ERROR("Unknown m4u module id:%d (m4u_handle = 0x%08X)\n", module_id , (unsigned int)m4u_handle );
            return -1;
            break;
    }
        
    *p_mva_addr = NULL;


    //1) Query MVA first
#if defined(MDP_FLAG_2_M4U_SUPPORT_QUERY)
    //status = p_m4udrv_->m4u_query_mva( query_module_id, va_addr, va_size, (unsigned int*) p_mva_addr);
    status = p_m4udrv_->m4u_query_mva( query_module_id, (void*)m4u_handle, va_addr, va_size, (unsigned int*) p_mva_addr);
    
    MDP_INFO_MEM("m4u_query_mva:m4u_q_id=%d m4u_id=%d por_id=%d va_addr=0x%08X va_size=0x%08X mva_addr=0x%08X mav_end_addr=0x%08X (m4u_handle = 0x%08X)\n",
               query_module_id,module_id, port_id, 
               (unsigned int)va_addr, (unsigned int)va_size, 
               (unsigned int)*p_mva_addr, (unsigned int)(*p_mva_addr + va_size - 1),
               (unsigned int)m4u_handle );

    #if defined(MDP_FLAG_2_M4U_SUPPORT_PREALLOC) /*Enable for new MVA prealloc mechinasm*/
    if( status != M4U_STATUS_OK ){
        MDP_ERROR("m4u_query_mva error:auto allocate mva failed?\n");
        MDP_ERROR("m4u_id=%d por_id=%d va_addr=0x%08X va_size=0x%08X mva_addr=0x%08X mav_end_addr=0x%08X (m4u_handle = 0x%08X)\n",
                   module_id, port_id, (unsigned int)va_addr, (unsigned int)va_size, (unsigned int)*p_mva_addr, (unsigned int)(*p_mva_addr + va_size - 1) , (unsigned int)m4u_handle );
        ret_val = -1;
        return ret_val;
    }
    #endif
#endif


    //2) if no, allocate & insert tlb
    if( *p_mva_addr == NULL )
    {

    #if defined (MDP_FLAG_2_M4U_SUPPORT_QUERY) && defined(MDP_FLAG_2_M4U_SUPPORT_PREALLOC)
    
        /*MVA Pre-Alloc Mechanism*/
        if( ( module_id == M4U_CLNTMOD_RDMA_GENERAL)  ||  ( module_id == M4U_CLNTMOD_ROT_GENERAL)  )
        {
            #if 1
            status = p_m4udrv_->m4u_register_buffer(    module_id,                  // Module ID
                                                        va_addr,                    // buffer virtual start address
                                                        va_size,                    // buffer size
                                                        (unsigned int*) p_mva_addr);// return MVA address
            #endif
            if( status != M4U_STATUS_OK ){
                MDP_ERROR("m4u_register_buffer error\n");
                MDP_ERROR("m4u_id=%d por_id=%d va_addr=0x%08X va_size=0x%08X mva_addr=0x%08X mav_end_addr=0x%08X (m4u_handle = 0x%08X)\n",
                           module_id, port_id, (unsigned int)va_addr, (unsigned int)va_size, (unsigned int)*p_mva_addr, (unsigned int)(*p_mva_addr + va_size - 1) , (unsigned int)m4u_handle );
                ret_val = -1;
                return ret_val;
            }
            
        }
        else
            
    #endif
        {

        #if 0
        status = p_m4udrv_->m4u_alloc_mva(  module_id,     // Module ID
                                            va_addr,              // buffer virtual start address
                                            va_size,              // buffer size
                                            (unsigned int*) p_mva_addr);             // return MVA address
        #else
        
        status = p_m4udrv_->m4u_alloc_mva(  module_id,     // Module ID
                                            (void*)m4u_handle,     // m4u handle
                                            va_addr,              // buffer virtual start address
                                            va_size,              // buffer size
                                            (unsigned int*) p_mva_addr);             // return MVA address
        
        #endif
        
        if( status != M4U_STATUS_OK ){
            MDP_ERROR("m4u_alloc_mva error\n");
            MDP_ERROR("m4u_id=%d por_id=%d va_addr=0x%08X va_size=0x%08X mva_addr=0x%08X mav_end_addr=0x%08X (m4u_handle = 0x%08X)\n",
                       module_id, port_id, (unsigned int)va_addr, (unsigned int)va_size, (unsigned int)*p_mva_addr, (unsigned int)(*p_mva_addr + va_size - 1) , (unsigned int)m4u_handle );
            ret_val = -1;
            return ret_val;
        }
        }

        
        //Track
        MdpDrv_MvaLoopMemTrack( MLM_CLIENT_ELEMENT, MLMT_ACTION_ALLOC, va_size );


        //Cached flag
        *p_b_alloc_mva = 1;

    }
    else
    {
        //Cached flag
        *p_b_alloc_mva = 0;
    }

#if defined(MDP_FLAG_2_M4U_SUPPORT_QUERY)
    if( ( module_id != M4U_CLNTMOD_RDMA_GENERAL)  &&  ( module_id != M4U_CLNTMOD_ROT_GENERAL)  )
#endif
    {
        //1) manual insert MVA start page address
        status = p_m4udrv_->m4u_manual_insert_entry(    module_id, 
                                                        *p_mva_addr,   // MVA address
                                                        false );    // lock the entry for circuling access
        if( status != M4U_STATUS_OK ){
            MDP_ERROR("m4u_manual_insert_entry error\n");
            
            MDP_ERROR("m4u_id=%d por_id=%d va_addr=0x%08X va_size=0x%08X mva_addr=0x%08X mav_end_addr=0x%08X (m4u_handle = 0x%08X)\n",
                       module_id, port_id, (unsigned int)va_addr, (unsigned int)va_size, (unsigned int)*p_mva_addr, (unsigned int)(*p_mva_addr + va_size - 1) , (unsigned int)m4u_handle );

            ret_val = -1;
        }

        //2) insert TLB uni-update range
        status = p_m4udrv_->m4u_insert_tlb_range(   module_id, 
                                                    *p_mva_addr,                // range start MVA
                                                    *p_mva_addr + va_size - 1,  // range end MVA
                                                    RT_RANGE_HIGH_PRIORITY,
                                                    entryCount );
        if( status != M4U_STATUS_OK ){
            MDP_ERROR("m4u_insert_tlb_range error\n");
            
            MDP_ERROR("m4u_id=%d por_id=%d va_addr=0x%08X va_size=0x%08X mva_addr=0x%08X mav_end_addr=0x%08X (m4u_handle = 0x%08X)\n",
                       module_id, port_id, (unsigned int)va_addr, (unsigned int)va_size, (unsigned int)*p_mva_addr, (unsigned int)(*p_mva_addr + va_size - 1) , (unsigned int)m4u_handle);

            ret_val = -1;
        }
        

        //3) config port 
        M4U_PORT_STRUCT M4uPort;
        M4uPort.ePortID = port_id;
        M4uPort.Virtuality = 1;						   
        M4uPort.Security = 0;
        M4uPort.Distance = 1;
        M4uPort.Direction = 0;
        status = p_m4udrv_->m4u_config_port(&M4uPort);

        if( status != M4U_STATUS_OK ){
            MDP_ERROR("m4u_config_port error\n");
            
            MDP_ERROR("m4u_id=%d por_id=%d va_addr=0x%08X va_size=0x%08X mva_addr=0x%08X mav_end_addr=0x%08X (m4u_handle = 0x%08X)\n",
                       module_id, port_id, (unsigned int)va_addr, (unsigned int)va_size, (unsigned int)*p_mva_addr, (unsigned int)(*p_mva_addr + va_size - 1) , (unsigned int)m4u_handle );
            ret_val = -1;
            }
    }


    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_DRV, "m4u start", &tt0, &fc0, &avt0, 30 );


    return ret_val;    
}


int MdpM4u_End( M4U_MODULE_ID_ENUM module_id, M4U_PORT_ID_ENUM port_id,
                unsigned long va_addr, unsigned long va_size, 
                unsigned long mva_addr , int b_alloc_mva ,
                unsigned long  m4u_handle )
{
    
    #if defined(MDP_FLAG_PROFILING)
            MdpDrv_Watch _MdpDrvWatch;
            static unsigned long tt0;//total time;
            static unsigned long fc0;//frame count;
            static unsigned long avt0;//avg_time_elapse;
    #endif

    int ret_val = 0;
    M4U_STATUS_ENUM status = M4U_STATUS_OK;


    
    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_DRV );
    
    if( p_m4udrv_ == NULL )
        return -1;

    
#if defined (MDP_FLAG_2_M4U_SUPPORT_QUERY) && defined(MDP_FLAG_2_M4U_SUPPORT_PREALLOC)
    /*In pre-alloc, the mav address maybe 0*/
    if( mva_addr == NULL ) 
    {
        if( ( module_id == M4U_CLNTMOD_RDMA_GENERAL)  ||  ( module_id == M4U_CLNTMOD_ROT_GENERAL)  )
        {
            if(         module_id == M4U_CLNTMOD_RDMA_GENERAL ){
                //status = p_m4udrv_->m4u_query_mva( M4U_CLNTMOD_RDMA_GENERAL, va_addr, va_size, (unsigned int*) &mva_addr);
                status = p_m4udrv_->m4u_query_mva( M4U_CLNTMOD_RDMA_GENERAL, (void*)m4u_handle, va_addr, va_size, (unsigned int*) &mva_addr);
            }else if(   module_id == M4U_CLNTMOD_ROT_GENERAL ){
                //status = p_m4udrv_->m4u_query_mva( M4U_CLNTMOD_ROT_GENERAL, va_addr, va_size, (unsigned int*) &mva_addr);
                status = p_m4udrv_->m4u_query_mva( M4U_CLNTMOD_ROT_GENERAL, (void*)m4u_handle, va_addr, va_size, (unsigned int*) &mva_addr);
            }

            /*
            MDP_INFO_MEM("m4u_query_mva:m4u_q_id=%d m4u_id=%d por_id=%d va_addr=0x%08X va_size=0x%08X mva_addr=0x%08X mav_end_addr=0x%08X\n",
                       query_module_id,module_id, port_id, 
                       (unsigned int)va_addr, (unsigned int)va_size, 
                       (unsigned int)*p_mva_addr, (unsigned int)(*p_mva_addr + va_size - 1));
            */
            
            if( status != M4U_STATUS_OK ){
                MDP_ERROR("m4u_query_mva error:query before dealloc failed?\n");
                MDP_ERROR("m4u_id=%d por_id=%d va_addr=0x%08X va_size=0x%08X mva_addr=0x%08X mav_end_addr=0x%08X (m4u_handle = 0x%08X)\n",
                           module_id, port_id, (unsigned int)va_addr, (unsigned int)va_size, (unsigned int)mva_addr, (unsigned int)(mva_addr + va_size - 1) , (unsigned int)m4u_handle );
                ret_val = -1;
                return ret_val;
            }

            //Debug
            if( mva_addr == NULL )
            {
                MDP_ERROR("m4u addr = 0 after query!\n");
            }
            
        }
    }
#endif

        //Debug
        if( mva_addr == NULL )
        {
            MDP_ERROR("m4u addr = 0\n");
            return -1; //Temp solution
            
        }
    

#if defined(MDP_FLAG_2_M4U_SUPPORT_QUERY)
    if( ( module_id != M4U_CLNTMOD_RDMA_GENERAL)  &&  ( module_id != M4U_CLNTMOD_ROT_GENERAL)  )
#endif
    {
        status = p_m4udrv_->m4u_invalid_tlb_range(module_id, mva_addr, mva_addr+va_size-1);
        if( status != M4U_STATUS_OK ){
            MDP_ERROR("m4u_invalid_tlb_range error\n");
            
            MDP_ERROR("m4u_id=%d por_id=%d va_addr=0x%08X va_size=0x%08X mva_addr=0x%08X mav_end_addr=0x%08X (m4u_handle = 0x%08X)\n",
                       module_id, port_id, (unsigned int)va_addr, (unsigned int)va_size, (unsigned int)mva_addr, (unsigned int)(mva_addr + va_size - 1) , (unsigned int)m4u_handle);
            ret_val = -1;
        }
    }

        
    if( b_alloc_mva == 1 )
    {
        status = p_m4udrv_->m4u_dealloc_mva(module_id, (void*)m4u_handle, va_addr, va_size, mva_addr);
        if( status != M4U_STATUS_OK ){
            MDP_ERROR("m4u_dealloc_mva error\n");
            
            MDP_ERROR("m4u_id=%d por_id=%d va_addr=0x%08X va_size=0x%08X mva_addr=0x%08X mav_end_addr=0x%08X (m4u_handle = 0x%08X)\n",
                       module_id, port_id, (unsigned int)va_addr, (unsigned int)va_size, (unsigned int)mva_addr, (unsigned int)(mva_addr + va_size - 1) , (unsigned int)m4u_handle);
            
            ret_val = -1;
        }
        
        //Track
        MdpDrv_MvaLoopMemTrack( MLM_CLIENT_ELEMENT, MLMT_ACTION_FREE, va_size );

    }
    
#if defined(MDP_FLAG_2_M4U_SUPPORT_QUERY)
    if( ( module_id != M4U_CLNTMOD_RDMA_GENERAL)  &&  ( module_id != M4U_CLNTMOD_ROT_GENERAL)  )
#endif
    {
        //config port                       
        M4U_PORT_STRUCT M4uPort;
        M4uPort.ePortID = port_id;
        M4uPort.Virtuality = 0;						   
        M4uPort.Security = 0;
        M4uPort.Distance = 1;
        M4uPort.Direction = 0;
        status = p_m4udrv_->m4u_config_port(&M4uPort);
        if( status != M4U_STATUS_OK ){
            MDP_ERROR("m4u_config_port error (m4u_handle = 0x%08X)\n", (unsigned int)m4u_handle);
            ret_val = -1;
            }
    }

    
    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_DRV, "m4u end", &tt0, &fc0, &avt0, 30 );

    return ret_val;
}

int MdpM4u_ZSD_Start(M4U_MODULE_ID_ENUM module_id,
                    unsigned long va_addr, unsigned long va_size,
                    unsigned long* p_mva_addr)
{
    M4U_STATUS_ENUM status;
    int ret_val = 0;

    status = p_m4udrv_->m4u_alloc_mva(  module_id,     // Module ID
                                        va_addr,              // buffer virtual start address
                                        va_size,              // buffer size
                                        (unsigned int*) p_mva_addr);             // return MVA address
    if( status != M4U_STATUS_OK )
    {
        MDP_ERROR("[ZSD] m4u_alloc_mva error\n");
        ret_val = -1;
        return ret_val;
    }

    //2) manual insert MVA start page address
    status = p_m4udrv_->m4u_manual_insert_entry(    module_id,
                                                    *p_mva_addr,   // MVA address
                                                    false );    // lock the entry for circuling access
    if( status != M4U_STATUS_OK ){
        MDP_ERROR("m4u_manual_insert_entry error\n");
        ret_val = -1;
    }

    //3) insert TLB uni-update range
    status = p_m4udrv_->m4u_insert_tlb_range(   module_id,
                                                *p_mva_addr,                // range start MVA
                                                *p_mva_addr + va_size - 1,  // range end MVA
                                                RT_RANGE_HIGH_PRIORITY,
                                                0 );

    MDP_INFO_PERSIST("[ZSD] MdpM4u_ZSD_Start: module_id=%d va=0x%x size=0x%x mva=0x%x\n", (int)module_id, (unsigned int)va_addr, (unsigned int)va_size, (unsigned int)*p_mva_addr);

    return ret_val;

}

int MdpM4u_ZSD_End(M4U_MODULE_ID_ENUM module_id,
                unsigned long va_addr, unsigned long va_size,
                unsigned long mva_addr )
{
    M4U_STATUS_ENUM status;
    int          ret_val = 0;
    status = p_m4udrv_->m4u_invalid_tlb_range(module_id, mva_addr, mva_addr+va_size-1);
    if( status != M4U_STATUS_OK ){
        MDP_ERROR("m4u_invalid_tlb_range error\n");
        ret_val = -1;
    }

    status = p_m4udrv_->m4u_dealloc_mva(module_id, va_addr, va_size, mva_addr);
    if( status != M4U_STATUS_OK ){
        MDP_ERROR("m4u_dealloc_mva error\n");
        ret_val = -1;
    }

    MDP_INFO_PERSIST("[ZSD] MdpM4u_ZSD_End: module_id=%d va=0x%x size=0x%x mva=0x%x\n", (int)module_id, (unsigned int)va_addr, (unsigned int)va_size,(unsigned int)mva_addr );
    return ret_val;
}


        


int MdpM4u_CacheSync(   M4U_MODULE_ID_ENUM  module_id, 
                        M4U_CACHE_SYNC_ENUM MeCacheSync,  
                        unsigned long       start_addr, 
                        unsigned long       size)
{
    
    #if defined(MDP_FLAG_PROFILING)
            MdpDrv_Watch _MdpDrvWatch;
            static unsigned long tt0;//total time;
            static unsigned long fc0;//frame count;
            static unsigned long avt0;//avg_time_elapse;
    #endif
    
    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_DRV );
    
    if( p_m4udrv_ == NULL )
        return -1;

    
    if(  p_m4udrv_->m4u_cache_sync( module_id , MeCacheSync , start_addr , size) != M4U_STATUS_OK )
        return -1;

    
    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_DRV, "flush cache", &tt0, &fc0, &avt0, 30 );

    return 0;
}

int MdpM4u_CacheFlushAll( void )
{
#if 1 /*defined(MT6577)*/
    
    #if defined(MDP_FLAG_PROFILING)
            MdpDrv_Watch _MdpDrvWatch;
            static unsigned long tt0;//total time;
            static unsigned long fc0;//frame count;
            static unsigned long avt0;//avg_time_elapse;
    #endif
    
    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_DRV );
    
    if( p_m4udrv_ == NULL )
        return -1;

    
    if(  p_m4udrv_->m4u_cache_flush_all( M4U_CLNTMOD_RDMA_GENERAL ) != M4U_STATUS_OK )
        return -1;

    
    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_DRV, "flush cache all", &tt0, &fc0, &avt0, 30 );
    

    return 0;

#else
    MDP_ERROR("Cache Flush All is not implemented!\n" );

    return -1;

#endif
}

int MdpM4u_GetTaskStruct( unsigned long *p_m4u_handle, unsigned long *p_adj  )
{
    
    MTKM4UDrv m4udrv;

    if( (p_m4u_handle == NULL) || (p_adj == NULL) )
    {
        MDP_ERROR("pointer is NULL. p_m4u_handle = 0x%08X p_adj = 0x%08X\n", (unsigned int)p_m4u_handle, p_adj );
        return -1;
    }

    
    
    if(  m4udrv.m4u_get_task_struct((void**) p_m4u_handle, (unsigned int *)p_adj) != M4U_STATUS_OK  )
    {
        MDP_ERROR("m4u_get_task_struct failed\n");
        return -1;
    }
        
    return 0;

}

int MdpM4u_PutTaskStruct( unsigned long adj  )
{
    
    MTKM4UDrv m4udrv;

    if(  m4udrv.m4u_put_task_struct( (unsigned int)adj ) != M4U_STATUS_OK  )
    {
        MDP_ERROR("m4u_put_task_struct failed\n");
        return -1;
    }
        
    return 0;
}



#endif /*MDP_FLAG_SUPPORT_M4U*/


