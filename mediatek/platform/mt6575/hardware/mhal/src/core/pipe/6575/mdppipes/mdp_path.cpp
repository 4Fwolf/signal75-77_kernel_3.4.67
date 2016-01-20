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

#include "mdp_path.h"

/*/////////////////////////////////////////////////////////////////////////////
    Macro Re-Definition
  /////////////////////////////////////////////////////////////////////////////*/
//#define WAIT_INT_TIMEOUT_MS (10000) /*For debug usage 10s*/
#define WAIT_INT_TIMEOUT_MS (3000)



/*-----------------------------------------------------------------------------
    MACRO Definition
  -----------------------------------------------------------------------------*/
#if 0
#define PRINT_ELEMENT_INVOLVED( _title_str_, _mdplist_, _mdpcount_, _mask_, _b_from_head_ )\
{\
    int  _i;    char _temp_str[512];\
    _temp_str[0] = '\0';\
    strcat( _temp_str, _title_str_ );\
    if( _b_from_head_ ) {\
        for( _i = 0; _i < _mdpcount_; _i++ )    {\
            if( ( _mask_ !=0  )    &&   ( ( _mask_ & _mdplist_[_i]->id() ) == 0 )    ){\
                    continue;       }\
            strcat( _temp_str,"->");\
            strcat( _temp_str,_mdplist_[_i]->name_str());\
        }\
    } else {\
        for( _i = (_mdpcount_-1) ; _i >= 0 ; _i-- )    {\
            if( ( _mask_ !=0  )    &&   ( ( _mask_ & _mdplist_[_i]->id() ) == 0 )    ){\
                    continue;       }\
            strcat( _temp_str,"->");\
            strcat( _temp_str,_mdplist_[_i]->name_str());\
        }\
    }\
    MDP_INFO("%s\n",_temp_str);\
}

#else
#define PRINT_ELEMENT_INVOLVED( _title_str_, _mdplist_, _mdpcount_, _mask_, _b_from_head_ )
#endif


/*-----------------------------------------------------------------------------
    Functions
  -----------------------------------------------------------------------------*/

int MdpPath_I::Start( void* pParam )
{
    int retVal;

    
#if defined(MDP_FLAG_PROFILING)
    MdpDrv_Watch _MdpDrvWatch;
    static unsigned long tt0,tt1,tt2,tt3;//total time;
    static unsigned long fc0,fc1,fc2,fc3;//frame count;
    static unsigned long avt0,avt1,avt2,avt3;//avg_time_elapse;
    char                 title_str[128];
#endif

    MDP_INFO("{%s}::Start"DEBUG_STR_BEGIN"\n", this->name_str() );


    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_PATH );
    retVal = this->_StartPre( pParam );
    if( retVal < 0 )
    {
        return retVal;
    }
    #if defined(MDP_FLAG_PROFILING)
    sprintf( title_str, "%s::StartPre", this->name_str() );
    #endif
    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_PATH, title_str, &tt0, &fc0, &avt0, 30 );
    
    
    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_PATH );
    retVal = this->_Start( pParam );
    if( retVal < 0 )
    {
        return retVal;
    }
    #if defined(MDP_FLAG_PROFILING)
    sprintf( title_str, "%s::Start", this->name_str() );
    #endif
    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_PATH, title_str, &tt1, &fc1, &avt1, 30 );
    

    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_PATH );
    retVal = this->_StartPost( pParam );
    if( retVal < 0 )
    {
        return retVal;
    }
    #if defined(MDP_FLAG_PROFILING)
    sprintf( title_str, "%s::StartPost", this->name_str() );
    #endif
    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_PATH, title_str, &tt2, &fc2, &avt2, 30 );


    MDP_INFO(DEBUG_STR_END"%s::Start""\n", this->name_str() );
    
    return 0;
}

unsigned long   MdpPath_I::ResourceIdMaskGet( void )
{
    
    int             ret_val = MDP_ERROR_CODE_OK;
    int             i;
    MDPELEMENT_I**  mdplist;
    int             mdpcount;
    unsigned long   resource_id_mask = 0;

    mdplist  =  this->mdp_element_list();
    mdpcount =  this->mdp_element_count();
    
    for( i = 0; i < mdpcount; i++ )
    {
        resource_id_mask |= mdplist[i]->id();
    }

    return resource_id_mask;
    
}


int MdpPath_I::WaitBusy( unsigned long rsc_to_wait_mask )
{
    int ret = 0;
    unsigned long   time_elapse;
    struct timeval  tv0,tv1;
    struct timezone tz0,tz1;
    
    #if defined(MDP_FLAG_PROFILING)
        MdpDrv_Watch _MdpDrvWatch;
        static unsigned long tt0,tt1,tt2,tt3;//total time;
        static unsigned long fc0,fc1,fc2,fc3;//frame count;
        static unsigned long avt0,avt1,avt2,avt3;//avg_time_elapse;
        char                 title_str[128];
    #endif
    

    
    MDP_INFO("{%s}::WaitBusy"DEBUG_STR_BEGIN"\n", this->name_str() );

    
    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_PATH );
    gettimeofday( &tv0, &tz0 );

    if( this->_WaitBusyPre( rsc_to_wait_mask ) < 0 )
        ret = -1;
    if( this->_WaitBusy( rsc_to_wait_mask ) < 0 )
        ret = -1;
    if( this->_WaitBusyPost( rsc_to_wait_mask ) < 0 )
        ret = -1;

    gettimeofday( &tv1, &tz1 );
    time_elapse = (tv1.tv_sec - tv0.tv_sec)*1000000 + (tv1.tv_usec - tv0.tv_usec);
    

    #if defined(MDP_FLAG_PROFILING)
    sprintf( title_str, "%s::WaitBusy", this->name_str() );
    #endif
    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_PATH , title_str, &tt0, &fc0, &avt0, 30 );

    MDP_INFO(DEBUG_STR_END"%s::WaitBusy"" time_elapse:%luus\n", this->name_str(), time_elapse );

    return ret;
}

int MdpPath_I::QueueGetFreeListOrWaitBusy( unsigned long mdp_id,unsigned long *p_StartIndex, unsigned long *p_Count  )
{
    int ret = 0;


    #if 0
    int repeat_count = 3;

    do
    {
        repeat_count--;

        //WaitBusy( mdp_id );

        QueueGetFreeList( mdp_id, p_StartIndex, p_Count  );

        if( *p_Count != 0 ) break;

        if( WaitBusy( mdp_id ) < 0 )
        {
            ret = -1;
        }
        
    } while( repeat_count );
    #else

    if( WaitBusy( mdp_id ) < 0 )
    {
        ret = -1;
    }

    QueueGetFreeList( mdp_id, p_StartIndex, p_Count  );
        
    #endif

    if( *p_Count == 0 )
    {
            MDP_ERROR("<0x%08X> wait 3 times,no free queue.!\n", (unsigned int)mdp_id );
            ret = -1;
    }


    

    return ret;
}


int MdpPath_I::QueueGetFreeList( unsigned long mdp_id,unsigned long *p_StartIndex, unsigned long *p_Count  )
{
    
    int             ret_val = MDP_ERROR_CODE_OK;
    int             i;
    MDPELEMENT_I**  mdplist;
    int             mdpcount;
    int             b_mdp_id_found = 0;

    mdplist  =  this->mdp_element_list();
    mdpcount =  this->mdp_element_count();

    *p_StartIndex   = 0;
    *p_Count        = 0;

    //Get FreeList (From tail to head to get somewhat performance improvement)
    for( i = mdpcount-1; i >= 0; i-- )
    {
        unsigned long id;

        id = mdplist[i]->id();

        if( (mdplist[i]->id() != mdp_id) && (mdplist[i]->id() != (mdp_id & 0x7FFFFFFF)) )
            continue;

        if( id & ( MID_RGB_ROT0 | MID_RGB_ROT1 | MID_RGB_ROT2 | MID_VDO_ROT0 | MID_VDO_ROT1 | MID_RGB_ROT0_EX) )
        {
            ROTDMA_I* pRotDma;

            b_mdp_id_found = 1;

            pRotDma = (ROTDMA_I*)mdplist[i];
            ret_val = pRotDma->DescQueueGetFreeList( p_StartIndex, p_Count );
            
            //MDP_INFO("[QUEUE]Dequeue <%s> ROT get free list. Start = %d Count = %d\n",pRotDma->name_str(),(int)*p_StartIndex, (int)*p_Count );

            break;
        }

        if( id & ( MID_R_DMA0 | MID_R_DMA1 ) )
        {
            RDMA_I* pRDma;

             b_mdp_id_found = 1;

            pRDma = (RDMA_I*)mdplist[i];
            ret_val = pRDma->DescQueueGetFreeList( p_StartIndex, p_Count );
            
            //MDP_INFO("[QUEUE]Dequeue <%s> RDMA get free list. Start = %d Count = %d\n",pRDma->name_str(),(int)*p_StartIndex, (int)*p_Count );

            break;
        }

        break;

    }

     if( b_mdp_id_found == 0 ){
        MDP_ERROR("MDP ID (0x%08X) is not found in the resource list of this path!\n",(unsigned int) mdp_id );
     }
        


    return ret_val;
    
    
}


int MdpPath_I::QueueRefill( unsigned long resource_to_refill_mask )
{
    
    int             ret_val = MDP_ERROR_CODE_OK;
    int             i;
    MDPELEMENT_I**  mdplist;
    int             mdpcount;

    mdplist  =  this->mdp_element_list();
    mdpcount =  this->mdp_element_count();
    

    //Refill
    for( i = mdpcount-1; i >= 0; i-- )
    {
        unsigned long id;

        id = mdplist[i]->id();

        if( (id & resource_to_refill_mask) == 0 && (id & resource_to_refill_mask & 0x7FFFFFFF) == 0)
            continue;

        //MDP_INFO("<%s> \n",mdplist[i]->name_str() );
        // for ZSD
        if ( (id & MID_RGB_ROT0_EX) && ((resource_to_refill_mask & MID_RGB_ROT0_EX) == MID_RGB_ROT0_EX))
        {
            ROTDMA_I* pRotDma;

            pRotDma = (ROTDMA_I*)mdplist[i];
            pRotDma->DescQueueRefillEx();


        }

        if( id & ( MID_RGB_ROT0 | MID_RGB_ROT1 | MID_RGB_ROT2 | MID_VDO_ROT0 | MID_VDO_ROT1 ) &&
            ((resource_to_refill_mask & MID_RGB_ROT0_EX) != MID_RGB_ROT0_EX))
        {
            ROTDMA_I* pRotDma;

            pRotDma = (ROTDMA_I*)mdplist[i];
            pRotDma->DescQueueRefill();

            
            
        }

        if( id & ( MID_R_DMA0 | MID_R_DMA1 ) )
        {
            RDMA_I* pRDma;

            pRDma = (RDMA_I*)mdplist[i];
            pRDma->DescQueueRefill();

            
        }

    }

    return ret_val;

    
}


int MdpPath_I::DumpRegister( void* pParam )
{
    
    int     i;
    MDPELEMENT_I**  mdplist;
    int             mdpcount;

    mdplist  =  this->mdp_element_list();
    mdpcount =  this->mdp_element_count();
    
    MDP_INFO("%s::DumpRegister"DEBUG_STR_BEGIN"\n", this->name_str() );

    MDP_PRINTF("DumpRegister - %s\n", this->name_str() );
    for( i = 0; i < mdpcount; i++ )
    {
        if( i == 0 )//Use 1st mdp element to dump mmsys1 system register
            mdplist[i]->DumpRegister(1);
            
        mdplist[i]->DumpRegister(0);
    }

    /*Dump software infor*/
    for( i = 0; i < mdpcount; i++ )
    {
        mdplist[i]->DumpDebugInfo();
    }

    
    MDP_INFO(DEBUG_STR_END"%s::DumpRegister""\n", this->name_str() );

    return 0;
    
}

int MdpPath_I::End( void* pParam )
{
    int ret = 0;
    
#if defined(MDP_FLAG_PROFILING)
    MdpDrv_Watch _MdpDrvWatch;
    static unsigned long tt0,tt1,tt2,tt3;//total time;
    static unsigned long fc0,fc1,fc2,fc3;//frame count;
    static unsigned long avt0,avt1,avt2,avt3;//avg_time_elapse;
    char                 title_str[128];
#endif
    
    MDP_INFO("{%s}::End"DEBUG_STR_BEGIN"\n", this->name_str() );

    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_PATH );
    
    if( this->_EndPre( pParam ) < 0 )
        ret = -1;
    if( this->_End( pParam ) < 0 )
        ret = -1;
    if( this->_EndPost( pParam ) < 0 )
        ret = -1;

    #if defined(MDP_FLAG_PROFILING)
    sprintf( title_str, "%s::End", this->name_str() );
    #endif
    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_PATH , title_str, &tt0, &fc0, &avt0, 30 );

    MDP_INFO(DEBUG_STR_END"%s::End""\n", this->name_str() );

    return ret;
}




int MdpPath_I::_Start( void* pParam )
{
    int             ret_val = MDP_ERROR_CODE_OK;
    int             i;
    MDPELEMENT_I**  mdplist;
    int             mdpcount;

    mdplist  =  this->mdp_element_list();
    mdpcount =  this->mdp_element_count();


    if( b_is_mdp_begin_ != 0 )
    {
        MDP_ERROR("MDP Path restart!\n");
        return -1;
    }
    
    
    //0.Begin
    ret_val = MdpBegin( mdplist , mdpcount , this->name_str());//Begin a series Mdp Element operation

    /*MdpBegin will handle error status internally, no need to do resource release when fail*/
    if( ret_val < 0 )
    {
        return ret_val;
    }

    b_is_mdp_begin_ = 1;



    //1.Config (From head to tail)
    PRINT_ELEMENT_INVOLVED("[Config]:", mdplist, mdpcount, 0 , 1 );
    for( i = 0; i < mdpcount; i++ )
    {
        MDP_INFO("<%s> Config:\n",mdplist[i]->name_str());
        if( ( ret_val = mdplist[i]->Config() ) < 0 )
        {
            return ret_val;
        }
    }


    //2.0. Before Trigger(after config), give user a chance to r/w register
    if( callbackfunc_before_start_ != 0 )
        callbackfunc_before_start_( pParam );

    //2.0. Before Trigger, flush cache all (only on MT6577 platform)
    MdpDrv_CacheFlushAll();


    //2.Enable (From tail to head)
    PRINT_ELEMENT_INVOLVED("[Enable]:", mdplist, mdpcount, 0, 0 );
    for( i = (mdpcount-1) ; i >= 0 ; i-- )
    {
        //MDP_INFO("<%s> Enable.\n",mdplist[i]->name_str() );
        if( ( ret_val = mdplist[i]->Enable() ) < 0 )
            return ret_val;
    }

    return ret_val;

    
}


int MdpPath_I::_WaitBusy( unsigned long rsc_to_wait_mask  )
{
    int     i;
    int     ret = 0;
    const int           kWAIT_COUNT_DOWN = 0xFFFFFF;
    unsigned long   desc_read_pointer;
    MDPELEMENT_I**  mdplist;
    int             mdpcount;

    mdplist  =  this->mdp_element_list();
    mdpcount =  this->mdp_element_count();
    


    //3.Check busy (From tail to head)
    PRINT_ELEMENT_INVOLVED("[CheckBusy]:", mdplist, mdpcount, rsc_to_wait_mask,1 );
    for( i = (mdpcount-1) ; i >= 0 ; i-- )
    {
        int wait_count_down = kWAIT_COUNT_DOWN;

        //MDP_INFO("Waiting <%s> \n",mdplist[i]->name_str());

        if( ( rsc_to_wait_mask !=0  )    &&  
            ( ( rsc_to_wait_mask & mdplist[i]->id() ) == 0 )    )
        {
            continue;
        }

    #if defined(MDP_FLAG_1_SUPPORT_INT)

        if( ret == -1 )
        {
            if( mdplist[i]->WaitIntDone( 1 ) < 0 )
            {
                MDP_ERROR("Waiting <%s> Time Out (zero wait)\n",mdplist[i]->name_str());
                ret = -1;
            }
        }
        else
        {
            if( mdplist[i]->WaitIntDone( WAIT_INT_TIMEOUT_MS ) < 0 )
            {
                MDP_ERROR("Waiting <%s> Time Out\n",mdplist[i]->name_str());
                ret = -1;
            }
        }
    #else
        while( mdplist[i]->CheckBusy(&desc_read_pointer) )
        {
            wait_count_down--;

            if( wait_count_down == 0 )
            {
                ret = -1;
                wait_count_down = kWAIT_COUNT_DOWN;
                MDP_ERROR("Waiting <%s> Time Out\n",mdplist[i]->name_str());
                break; //Stop busy waiting
            }
        }
    #endif

    }



    if( ret == -1 )
    {
        MDP_INFO("Start to dump register...\n");
        DumpRegister( NULL );
        MDP_INFO("Dump register finish.\n");

        MDP_ERROR("Time Out, Hard Reset\n");

        for( i = (mdpcount-1) ; i >= 0 ; i-- )
        {
            mdplist[i]->HardReset();
        }
        
    }


    /*ONLY FOR TEST*/
    #if 0
    MDP_DEBUG("test dump register...\n");
    DumpRegister( NULL );
    MDP_DEBUG("test dump register finish.\n");
    sleep(1);
    #endif

    return ret;


}




int MdpPath_I::_End( void* pParam )
{

    int     ret_val = MDP_ERROR_CODE_OK;
    int     ret_val_temp = MDP_ERROR_CODE_OK;
    int     i;
    MDPELEMENT_I**  mdplist;
    int             mdpcount;

    mdplist  =  this->mdp_element_list();
    mdpcount =  this->mdp_element_count();

    
    if( b_is_mdp_begin_ != 1 )
        return 0;

    
    //3.Disable (From head to tail)
    PRINT_ELEMENT_INVOLVED("[Disable]:", mdplist, mdpcount, 0, 1 );
    for( i = 0; i < mdpcount; i++ )
    {
        //MDP_INFO("<%s> Disable.\n",mdplist[i]->name_str() );
        if( ( ret_val_temp = mdplist[i]->Disable() ) < 0 )
            ret_val = ret_val_temp;
    }


    if( (ret_val_temp = MdpEnd( mdplist, mdpcount )) < 0 )
    {
        ret_val = ret_val_temp;
    }

    b_is_mdp_begin_ = 0;


    return ret_val;
    
}





/*/////////////////////////////////////////////////////////////////////////////
    Lagecy C version
  /////////////////////////////////////////////////////////////////////////////*/

int _MdpPathTriggerHw( MDPELEMENT_I* mdp_element_list[] , int mdp_element_count,  
                       MdpCustTriggerHw_t* p_cust_func /*= NULL*/, void* p_custdata /*= NULL*/ )

{
    /*-----------------------------------------------------------------------------
        MACRO Definition
      -----------------------------------------------------------------------------*/
    #define _CUSTTRIGGER_CB( _cb_ , _b_exit_ ) \
        if( p_cust_func != NULL ){ \
            if( p_cust_func->_cb_ != NULL ){ \
                int temp_ret_val;\
                MDP_INFO("p_cust_func->%s()\n",#_cb_);\
                if( (temp_ret_val = p_cust_func->_cb_( p_custdata )) < 0 ){ \
                    ret_val = temp_ret_val;\
                    MDP_ERROR("p_cust_func->%s failed.\n",#_cb_); \
                    if( (_b_exit_==1) ) goto _MdpPathTriggerHw_EXIT; \
                } \
            } \
        }

    #define _CUSTTRIGGER_CAM_CB( _cb_ , _b_exit_ , _curr_mdp_id_ ) \
        if( p_cust_func != NULL ){ \
            if( p_cust_func->mdp_module_after_cam == _curr_mdp_id_ ){\
                if( p_cust_func->_cb_ != NULL ){ \
                    int temp_ret_val;\
                    MDP_INFO("p_cust_func->%s()\n",#_cb_);\
                    if( (temp_ret_val = p_cust_func->_cb_( p_custdata )) < 0 ){ \
                        ret_val = temp_ret_val;\
                        MDP_ERROR("p_cust_func->%s failed.\n",#_cb_); \
                        if( (_b_exit_==1) ) goto _MdpPathTriggerHw_EXIT; \
                    } \
                } \
            }\
        }
   
   /*-----------------------------------------------------------------------------*/
    int             ret_val = MDP_ERROR_CODE_OK;
    unsigned long   desc_read_pointer;
    const int       kWAIT_COUNT_DOWN = 0xFFFFF;
    int i,j;

    
#if defined(MDP_FLAG_PROFILING)
    MdpDrv_Watch _MdpDrvWatch;
    static unsigned long tt0,tt1,tt2,tt3;//total time;
    static unsigned long fc0,fc1,fc2,fc3;//frame count;
    static unsigned long avt0,avt1,avt2,avt3;//avg_time_elapse;
#endif
    
    /*.............................................................................*/
    //0.Begin
    
    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_PATH );

    ret_val = MdpBegin( mdp_element_list , mdp_element_count , NULL );    //Begin a series Mdp Element operation

    if( ret_val < 0 ){
        return ret_val;
    }
    
    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_PATH, "Begin", &tt0, &fc0, &avt0, 30 );

    /*.............................................................................*/

    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_PATH );


    //1.Config (From head to tail)
    /*CB*/_CUSTTRIGGER_CB(cb_ConfigPre, 1 );

    PRINT_ELEMENT_INVOLVED("[Config]:", mdp_element_list, mdp_element_count, 0, 1 );
    for( i = 0; i < mdp_element_count; i++ )
    {
        /*CB*/_CUSTTRIGGER_CAM_CB( cb_CamConfig, 1, mdp_element_list[i]->id() );
        
        MDP_INFO("<%s> Config:\n",mdp_element_list[i]->name_str());
        if( (ret_val = mdp_element_list[i]->Config() ) < 0 ){
            goto _MdpPathTriggerHw_EXIT;
        }
    }
    /*CB*/_CUSTTRIGGER_CB(cb_ConfigPost, 1);
    
    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_PATH , "Config", &tt1, &fc1, &avt1, 30 );

    /*.............................................................................*/


    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_PATH );

    
    //2.0. Before Trigger, flush cache all (only on MT6577 platform)
    MdpDrv_CacheFlushAll();

    //2.Enable (From tail to head)
    /*CB*/_CUSTTRIGGER_CB(cb_EnablePre, 1);
    
    PRINT_ELEMENT_INVOLVED("[Enable]:", mdp_element_list, mdp_element_count, 0, 0 );
    for( i = (mdp_element_count-1) ; i >= 0 ; i-- )
    {
        MDP_INFO("<%s> Enable.\n",mdp_element_list[i]->name_str() );
        if( (ret_val = mdp_element_list[i]->Enable()) < 0 ){
            goto _MdpPathTriggerHw_EXIT;
        }

        /*CB*/_CUSTTRIGGER_CAM_CB( cb_CamEnable, 1, mdp_element_list[i]->id() );
        }
        
    /*CB*/_CUSTTRIGGER_CB(cb_EnablePost, 1);
           
    /*.............................................................................*/


    //3.Check busy (From tail to head)
    /*CB*/_CUSTTRIGGER_CB(cb_WaitPre, 0);

    PRINT_ELEMENT_INVOLVED("CheckBusy:", mdp_element_list, mdp_element_count, 0, 1 );
    for( i = (mdp_element_count-1) ; i >= 0 ; i-- )
    {
        int wait_count_down = kWAIT_COUNT_DOWN;

        MDP_INFO("Waiting <%s> \n",mdp_element_list[i]->name_str());

        
    #if defined(MDP_FLAG_1_SUPPORT_INT)
        if( ret_val == MDP_ERROR_CODE_FAIL )
        {
            if( mdp_element_list[i]->WaitIntDone( 1 ) < 0 )
            {
                MDP_ERROR("Waiting <%s> Time Out (zero wait)\n",mdp_element_list[i]->name_str());
                ret_val = MDP_ERROR_CODE_FAIL;
            }
        } 
        else
        {
            if( mdp_element_list[i]->WaitIntDone( WAIT_INT_TIMEOUT_MS ) < 0 )
            {
                MDP_ERROR("Waiting <%s> Time Out\n",mdp_element_list[i]->name_str());
                ret_val = MDP_ERROR_CODE_FAIL;
            }
        }
    #else
           
        while( mdp_element_list[i]->CheckBusy(&desc_read_pointer) )
        {
            wait_count_down--;

            if( wait_count_down == 0 )
            {
                ret_val = MDP_ERROR_CODE_FAIL;
                wait_count_down = kWAIT_COUNT_DOWN;
                MDP_ERROR("Waiting <%s> Time Out\n",mdp_element_list[i]->name_str());
                //MDP_INFO("Start to dump register...\n");
                //_MdpPathDumpRegister( mdp_element_list, mdp_element_count );
                //MDP_INFO("Dump registerFinish.\n");
                break; //Stop busy waiting
            }
        }
    #endif /*MDP_FLAG_1_SUPPORT_INT*/

        /*CB*/_CUSTTRIGGER_CAM_CB( cb_CamWait, 0, mdp_element_list[i]->id() ); //Call only if current mdp id match the specific id
    
    }

    /*CB*/_CUSTTRIGGER_CB(cb_WaitPost, 0);
    

    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_PATH, "Trigger-Done", &tt2, &fc2, &avt2, 30 );
    
    /*.............................................................................*/

    /*Debug*/
    if( ret_val < 0 )
    {
        MDP_INFO("Start to dump register...\n");
        _MdpPathDumpRegister( mdp_element_list, mdp_element_count );
        MDP_INFO("Dump registerFinish.\n");

        
        MDP_ERROR("HwTime Out, Hard Reset\n");

        for( i = (mdp_element_count-1) ; i >= 0 ; i-- )
        {
            mdp_element_list[i]->HardReset();
    }
    }

    #if 0
    //Clouds test
    {
        
        MDP_ERROR("HwTime Out, Hard Reset\n");

        for( i = (mdp_element_count-1) ; i >= 0 ; i-- )
        {
            mdp_element_list[i]->HardReset();
        }
    }
    #endif

    
_MdpPathTriggerHw_EXIT:

    
    /*.............................................................................*/
    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_PATH );

    //4.Disable (From head to tail)
    /*CB*/_CUSTTRIGGER_CB(cb_DisablePre, 0);
    
    PRINT_ELEMENT_INVOLVED("[Disable]:", mdp_element_list, mdp_element_count, 0, 1 );
    for( i = 0; i < mdp_element_count; i++ )
    {
        /*CB*/_CUSTTRIGGER_CAM_CB( cb_CamDisable, 0, mdp_element_list[i]->id() );
        
        MDP_INFO("<%s> Disable.\n",mdp_element_list[i]->name_str() );
        mdp_element_list[i]->Disable();
    }

    /*CB*/_CUSTTRIGGER_CB(cb_DisablePost, 0);

    /*.............................................................................*/

    MdpEnd( mdp_element_list , mdp_element_count ); //End a series Mdp Element

    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_PATH, "Finish", &tt3, &fc3, &avt3, 30 );


    return ret_val;
}

int _MdpPathDumpRegister( MDPELEMENT_I* mdp_element_list[] , int mdp_element_count  )
{
    int j;

    MDP_PRINTF("DumpRegister - Legacy C version path\n" );
    
    for( j = 0; j < mdp_element_count; j++ )
    {
        if( j == 0 )//Use 1st mdp element to dump mmsys1 system register
            mdp_element_list[j]->DumpRegister(1);
            
        mdp_element_list[j]->DumpRegister(0);
    }

    
    /*Dump software infor*/
    for( j = 0; j < mdp_element_count; j++ )
    {
        mdp_element_list[j]->DumpDebugInfo();
    }

    return 0;
}

