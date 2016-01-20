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

#define LOG_TAG "MDP"

#include "mdp_element.h"
#include "mdp_sysram.h"



#ifdef MDP_FLAG_0_PLATFORM_LINUX
//open syscall
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//close syscall
#include <unistd.h>
//mmap syscall
#include <sys/mman.h>
//mutex
#include <pthread.h>
//Priority
#include <sys/resource.h>
//strerror
#include <string.h>
//errno
#include <errno.h>





    #ifdef MDP_FLAG_1_SUPPORT_M4U
    //For M4U
    #include    "mdp_m4u.h"
    #endif /*MDP_FLAG_1_SUPPORT_M4U*/


#endif /*MDP_FLAG_0_PLATFORM_LINUX*/





/*/////////////////////////////////////////////////////////////////////////////
    Definition
  /////////////////////////////////////////////////////////////////////////////*/
#define MDP_DMA_RESET_TIMEOUT   (50000)   //must be less than 50us -> 1G needs 50000times


/*/////////////////////////////////////////////////////////////////////////////
    Priority Boost support function
  /////////////////////////////////////////////////////////////////////////////*/
pthread_mutex_t mutex_g_mdp_nvstack = PTHREAD_MUTEX_INITIALIZER;

static void ENTER_CRITICAL_NVSTACK( void )
{
    pthread_mutex_lock ( &mutex_g_mdp_nvstack );
    //MDP_INFO("BE_REF MUTEX LOCK\n");
}

static void EXIT_CRITICAL_NVSTACK( void )
{
    //MDP_INFO("BE_REF MUTEX UN-LOCK\n");
    pthread_mutex_unlock ( &mutex_g_mdp_nvstack );
}


class NiceValueStack
{
private:
    #define   NICE_VALUE_STACK_SIZE   (20)
    int stack_[NICE_VALUE_STACK_SIZE];
    int top_;

public:
    NiceValueStack():top_(0){};

    int Push( int nicevalue );
    int Pop( int* p_nicevalue );
};


int NiceValueStack::Push( int nicevalue )
{
    int ret = 0;

    ENTER_CRITICAL_NVSTACK();
    if(top_ > (NICE_VALUE_STACK_SIZE-2) )
    {
        MDP_ERROR("nice value stack full\n");
        ret = -1;
    }
    else
    {
        top_++;
        stack_[top_]=nicevalue;
        ret = 0;
    }
    EXIT_CRITICAL_NVSTACK();

    return ret;
}

int NiceValueStack::Pop( int* p_nicevalue )
{
    int ret = 0;

    if( p_nicevalue == NULL ){
        MDP_ERROR("p_nicevalue = NULL\n");
        return -1;
    }


    ENTER_CRITICAL_NVSTACK();
    if( top_ <= 0 )
    {
        MDP_ERROR("nice value stack empty\n");
        ret = -1;
    }
    else
    {
        *p_nicevalue=stack_[top_];
        top_--;
    }
    EXIT_CRITICAL_NVSTACK();

    return ret;

}

static NiceValueStack  g_nvstack;


static int PriorityBoost( void )
{
    int nv;


    //MDP_DEBUG("PriorityBoost()\n");

    if( SCHED_NORMAL == sched_getscheduler(0) )
    {
        nv = getpriority(PRIO_PROCESS , 0);
        //MDP_DEBUG("\tgetpriority = %d\n",nv);

        g_nvstack.Push(nv);

        setpriority(PRIO_PROCESS/*which*/, 0/*who:0 is self*/, PRIO_MIN);
        //MDP_DEBUG("\tsetpriority = %d\n",PRIO_MIN);

    }


    return 0;

}

static int PriorityRestore( void )
{
    int nv;

    //MDP_DEBUG("PriorutyRestore()\n");

    if( SCHED_NORMAL == sched_getscheduler(0) )
    {

        g_nvstack.Pop(&nv);

        setpriority(PRIO_PROCESS, 0, nv);
        //MDP_DEBUG("\tsetpriority = %d\n",nv);

    }


    return 0;
}


/*/////////////////////////////////////////////////////////////////////////////
    Const Definition
  /////////////////////////////////////////////////////////////////////////////*/

/*These are the elements that use descriptor queue*/
#define ELEMENT_WITH_DESCRIPTOR_MASK (\
    MID_R_DMA0 | MID_R_DMA1 | \
    MID_RGB_ROT0 | MID_RGB_ROT1 | MID_RGB_ROT2 |\
    MID_VDO_ROT0 | MID_VDO_ROT1 )



/*/////////////////////////////////////////////////////////////////////////////
    MDP Begin-End Function
  /////////////////////////////////////////////////////////////////////////////*/

static int g_mdp_begin_end_pair_ref_count = 0; //MDP begin()-end() pair reference count


#ifdef MDP_FLAG_0_PLATFORM_LINUX

pthread_mutex_t mutex_g_mdp_begin_end_pair_ref_count = PTHREAD_MUTEX_INITIALIZER;

static void ENTER_CRITICAL_BE_REF( void )
{
    pthread_mutex_lock ( &mutex_g_mdp_begin_end_pair_ref_count );
    //MDP_INFO("BE_REF MUTEX LOCK\n");
}

static void EXIT_CRITICAL_BE_REF( void )
{
    //MDP_INFO("BE_REF MUTEX UN-LOCK\n");
    pthread_mutex_unlock ( &mutex_g_mdp_begin_end_pair_ref_count );
}

#endif



int MdpBegin( MDPELEMENT_I* mdp_element_list[] , int mdp_element_count , const char* mdp_path_name )    //Begin a series Mdp Element operation
{
    int i;
    int ret_val = MDP_ERROR_CODE_OK;
    int mdp_resource_mask = 0;
    unsigned long new_reg_base;
    int b_time_shared;
    unsigned long time_out_ms;


#if defined(MDP_FLAG_PROFILING)
        MdpDrv_Watch _MdpDrvWatch;
        static unsigned long tt0,tt1,tt2,tt3;//total time;
        static unsigned long fc0,fc1,fc2,fc3;//frame count;
        static unsigned long avt0,avt1,avt2,avt3;//avg_time_elapse;
#endif

    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_ELMNT );

    for( i = 0; i < mdp_element_count; i++ )
    {
        if( mdp_element_list[i] == NULL )
        {
            if( mdp_path_name != NULL )            {
                MDP_ERROR("{%s} mdp element %d is null\n", mdp_path_name, i );
            } else        {
                MDP_ERROR("{null path name }mdp element %d is null\n",i );
            }
            return MDP_ERROR_CODE_FAIL;
        }

        mdp_resource_mask |= mdp_element_list[i]->id();
    }

    /*-----------------------------------------------------------------------------*/
    //0.Distinguish specific mdppath
    /*-----------------------------------------------------------------------------*/
    /*-----------------------------------------------------------------------------
        Default Path (SF Image Transform)
      -----------------------------------------------------------------------------*/

    b_time_shared   = 1;       //Default time share
    time_out_ms     = 20;      //Image transform or other path wait for 20ms

    if( mdp_path_name != NULL )
    {
        /*-----------------------------------------------------------------------------
            Jpeg Decode Relate Path
          -----------------------------------------------------------------------------*/
        if( ( strcmp( mdp_path_name, "JpgDecMdpPath" ) == 0 ) )
        {
            time_out_ms = 300;      //HW Jpeg decode wait for 300ms
        }
        /*-----------------------------------------------------------------------------
            Camera Related Path
          -----------------------------------------------------------------------------*/
        else if(( strcmp( mdp_path_name, "MdpPathCameraPreviewToMemory" ) == 0 ) ||
                //( strcmp( mdp_path_name, "MdpPathDisplayFromMemory" ) == 0 ) ||   /*In SW trigger mode, this path can be share.TODO:here is no idea if sw trigger be enable*/
                ( strcmp( mdp_path_name, "MdpPathCameraPreviewZeroShutter" ) == 0 ) ||
                ( strcmp( mdp_path_name, "MdpPathCamera2PreviewToMemory" ) == 0 ) ||
                ( strcmp( mdp_path_name, "MdpPathJpgEncodeScale" ) == 0 ) ||
                ( strcmp( mdp_path_name, "MdpPathN3d2ndPass" ) == 0 ) ||
                ( strcmp( mdp_path_name, "MdpPathDummyBrz" ) == 0 ) /*This dummy path is lock as well as preview/capture path*/
                )
        {
            b_time_shared = 0;
            time_out_ms = 6000;    //Camera paths wait for a longer time,lock timeout more than mdp timeout
        }
        else if( ( strcmp( mdp_path_name, "MdpPathDisplayFromMemory" ) == 0 ) )    /*In SW trigger mode, this path can be share.TODO:here is no idea if sw trigger be enable*/
        {
            time_out_ms = 6000;    //MdpPathDisplayFromMemory is one part of camera, wait for a longer time
        }

    }

    //Priority Boost
    PriorityBoost();

    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_ELMNT, "strcmp", &tt0, &fc0, &avt0, 30 );



#ifdef MDP_FLAG_0_PLATFORM_LINUX

    /*-----------------------------------------------------------------------------*/
    //1.Lock Resource, if fail !!!return IMMEDIATEDLY!!!
    /*-----------------------------------------------------------------------------*/
    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_ELMNT );

    if( MdpDrvLockResource( mdp_resource_mask, b_time_shared, time_out_ms, mdp_path_name ) != 0 )
    {
        ret_val = MDP_ERROR_CODE_LOCK_RESOURCE_FAIL;

        //Priority Restore
        PriorityRestore();

        MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_ELMNT, "Lock Resource", &tt1, &fc1, &avt1, 30 );
        return ret_val;
    }

    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_ELMNT, "Lock Resource", &tt1, &fc1, &avt1, 30 );


    /*-----------------------------------------------------------------------------*/
    //2.MMAP MMSYS1
    /*-----------------------------------------------------------------------------*/
    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_ELMNT );

    ENTER_CRITICAL_BE_REF(); /*Critical Section Enter--------------------------------------------*/
    {

        //2.1 Increase reference count after lock success
        g_mdp_begin_end_pair_ref_count++;


        //2.2 Remap global mmsys1 reg if this is first reference
        if( g_mdp_begin_end_pair_ref_count == 1 )
        {
            new_reg_base = (unsigned long)mmap(   (void *)NULL ,
                                                    (size_t)MDPELEMENT_I::mmsys1_reg_range(),
                                                    (PROT_READ | PROT_WRITE) ,
                                                    MAP_SHARED ,
                                                    MdpDrvFd() ,
                                                    (off_t)MDPELEMENT_I::mmsys1_reg_base_addr_pa()
                                                    );
            if( new_reg_base == 0 )
            {
                ret_val = MDP_ERROR_CODE_FAIL;
            }

            MDPELEMENT_I::Remap_mmsys1_reg_base_addr( new_reg_base );

        }
    }
    EXIT_CRITICAL_BE_REF(); /*Critical Section Exit--------------------------------------------*/

    if( ret_val != MDP_ERROR_CODE_OK )
    {
        MDP_ERROR("mmsys1 mmap fail\n");
        MdpEnd( mdp_element_list, mdp_element_count );
        return ret_val;
    }


    /*-----------------------------------------------------------------------------*/
    //3.MMAP MDP Modules
    /*-----------------------------------------------------------------------------*/
    //3.1 Remap other register range
    for( i = 0; i < mdp_element_count; i++ )
    {


        new_reg_base = (unsigned long)mmap(   (void *)NULL ,
                                                (size_t)mdp_element_list[i]->reg_range(),
                                                (PROT_READ | PROT_WRITE) ,
                                                MAP_SHARED ,
                                                MdpDrvFd() ,
                                                (off_t)mdp_element_list[i]->reg_base_addr_pa()
                                                );
        if( new_reg_base == 0 )
            ret_val = MDP_ERROR_CODE_FAIL;

        mdp_element_list[i]->Remap_reg_base_addr( new_reg_base );

        //MDP_INFO("<%s> remap reg addr 0x%x --> 0x%x \n",      mdp_element_list[i]->name_str(),
        //                                                        (unsigned int)mdp_element_list[i]->reg_base_addr_pa(),
        //                                                        (unsigned int)mdp_element_list[i]->reg_base_addr() );


    }


    if( ret_val != MDP_ERROR_CODE_OK )
    {
        MDP_ERROR("mdp element mmap fail\n");
        MdpEnd( mdp_element_list, mdp_element_count );
        return ret_val;
    }


    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_ELMNT, "mmap", &tt2, &fc2, &avt2, 30 );


    /*-----------------------------------------------------------------------------*/
    //4.Descriptor memory allocate
    /*-----------------------------------------------------------------------------*/
    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_ELMNT );
    for( i = 0; i < mdp_element_count; i++ )
    {
        unsigned long desc_base = 0xFFFFFFFF;

        /*If not a descriptor-supported element just skip*/
        if( ( mdp_element_list[i]->id() & ELEMENT_WITH_DESCRIPTOR_MASK ) == 0 )
            continue;

        #if 1

        desc_base = MdpSysram_Alloc( mdp_element_list[i]->id(), 0xC0, MDPSYSRAM_SUBCAT_DESCRIPTOR /*1*/ );

        #else /*Test Code*/

        #define Base_Bank2_Temp              (0xC2000000 + 0x18000 + 0x8000 )

        switch( mdp_element_list[i]->id() )
        {
            case MID_R_DMA0:
                desc_base = Base_Bank2_Temp + 0xC0 * 0;
                break;
            case MID_R_DMA1:
                desc_base = Base_Bank2_Temp + 0xC0 * 1;
                break;
            case MID_RGB_ROT0:
                desc_base = Base_Bank2_Temp + 0xC0 * 2;
                break;
            case MID_RGB_ROT1:
                desc_base = Base_Bank2_Temp + 0xC0 * 3;
                break;
            case MID_RGB_ROT2:
                desc_base = Base_Bank2_Temp + 0xC0 * 4;
                break;
            case MID_VDO_ROT0:
                desc_base = Base_Bank2_Temp + 0xC0 * 5;
                break;
            case MID_VDO_ROT1:
                desc_base = Base_Bank2_Temp + 0xC0 * 6;
                break;
        }

        #endif


        if( desc_base == 0 )
        {
            mdp_element_list[i]->DumpDebugInfo();
            ret_val = MDP_ERROR_CODE_FAIL;
        }

        mdp_element_list[i]->Remap_descript_base_addr( desc_base );


    }


    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_ELMNT, "desc sysram alloc", &tt3, &fc3, &avt3, 30 );

    if( ret_val != MDP_ERROR_CODE_OK )
    {
        MDP_ERROR("mdp descriptor memory allocate fail\n");
        MdpEnd( mdp_element_list, mdp_element_count );
        return ret_val;
    }


#endif /*MDP_FLAG_0_PLATFORM_LINUX*/



    return ret_val;

}



int MdpEnd( MDPELEMENT_I* mdp_element_list[] , int mdp_element_count )    //Begin a series Mdp Element operation

{

    #if defined(MDP_FLAG_PROFILING)
            MdpDrv_Watch _MdpDrvWatch;
            static unsigned long tt0,tt1,tt2,tt3;//total time;
            static unsigned long fc0,fc1,fc2,fc3;//frame count;
            static unsigned long avt0,avt1,avt2,avt3;//avg_time_elapse;
    #endif

    int i;
    int mdp_resource_mask = 0;
    unsigned long new_reg_base;

    for( i = 0; i < mdp_element_count; i++ )
    {
        mdp_resource_mask |= mdp_element_list[i]->id();
    }



    //MDP_INFO("mdp_resource_mask = %x\n",mdp_resource_mask );

#ifdef MDP_FLAG_0_PLATFORM_LINUX

    /*-----------------------------------------------------------------------------*/
    //4.Descriptor memory free
    /*-----------------------------------------------------------------------------*/
    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_ELMNT );

    for( i = 0; i < mdp_element_count; i++ )
    {
        unsigned long desc_base;

        /*If not a descriptor-supported element just skip*/
        if( ( mdp_element_list[i]->id() & ELEMENT_WITH_DESCRIPTOR_MASK ) == 0 )
            continue;

        if( mdp_element_list[i]->descript_base_addr() != NULL )
        {

            if( MdpSysram_Free( mdp_element_list[i]->id(),
                                mdp_element_list[i]->descript_base_addr(),
                                MDPSYSRAM_SUBCAT_DESCRIPTOR /*1*/ ) < 0 )
            {
                MDP_ERROR("<%s> free descriptor memory failed\n", mdp_element_list[i]->name_str());
            }

            mdp_element_list[i]->Remap_descript_base_addr( 0 );
        }
    }

    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_ELMNT, "desc free", &tt0, &fc0, &avt0, 30 );

    /*-----------------------------------------------------------------------------*/
    //3.MUNMAP MDP Modules
    /*-----------------------------------------------------------------------------*/
    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_ELMNT );
    for( i = 0; i < mdp_element_count; i++ )
    {
        if( munmap( (void*) mdp_element_list[i]->reg_base_addr(), (size_t) mdp_element_list[i]->reg_range() ) != 0 )
        {
            MDP_ERROR("<%s> un-map failed\n", mdp_element_list[i]->name_str() );
        }

        mdp_element_list[i]->Remap_reg_base_addr( 0 );
    }


    /*-----------------------------------------------------------------------------*/
    //2.MUNMAP MMSYS1
    /*-----------------------------------------------------------------------------*/
    ENTER_CRITICAL_BE_REF(); /*Critical Section Enter--------------------------------------------*/
    {
        if( g_mdp_begin_end_pair_ref_count >= 1 )
        {
            //2.1 Descrease reference count
            g_mdp_begin_end_pair_ref_count--; //TODO:Re-entrant issue?

            //2.2 unmap global mmsys1 reg if this is last reference
            if( g_mdp_begin_end_pair_ref_count == 0 )
            {
                if( munmap( (void*) MDPELEMENT_I::mmsys1_reg_base_addr(), (size_t) MDPELEMENT_I::mmsys1_reg_range() ) != 0 )
                {
                    MDP_ERROR("<mmsys1> un-map failed\n");
                }

                MDPELEMENT_I::Remap_mmsys1_reg_base_addr( 0 );
            }
        }
    }
    EXIT_CRITICAL_BE_REF(); /*Critical Section Exit--------------------------------------------*/

    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_ELMNT, "unmap", &tt1, &fc1, &avt1, 30 );




    /*-----------------------------------------------------------------------------*/
    //1.Un-Lock Resource
    /*-----------------------------------------------------------------------------*/
    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_ELMNT );

    if( MdpDrvUnLockResource( mdp_resource_mask ) != 0 )
    {
        //Priority Restore
        PriorityRestore();
        return MDP_ERROR_CODE_FAIL;
    }
    //Priority Restore
    PriorityRestore();

    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_ELMNT, "unlock", &tt2, &fc2, &avt2, 30 );

#endif /*MDP_FLAG_0_PLATFORM_LINUX*/


    return MDP_ERROR_CODE_OK;
}



/*/////////////////////////////////////////////////////////////////////////////
    MDPELEMENT_I
  /////////////////////////////////////////////////////////////////////////////*/
//unsigned long MDPELEMENT_I::m_mmsys1_reg_base_addr = MT6575_MMSYS1_BASE; /*Use physical base address as default address*/
unsigned long MDPELEMENT_I::m_mmsys1_reg_base_addr = 0; /*Use physical base address as default address*/



unsigned long MDPELEMENT_I::dec_id( void )//Decimal ID number
{
    unsigned long dec_id = 0;
    unsigned long id = this->id();  //virtual

    while( id )
    {
        dec_id++;
        id = id >> 1;
    }

    return (dec_id-1);
}


int MDPELEMENT_I::Config( void )
{
    int retval;

    //MDP_PRINTF("<%2d> Config Pa=0x%x Va=0x%x\n",(int)this->dec_id(), (unsigned int)this->reg_base_addr_pa(), (unsigned int)reg_base_addr() );

    retval = this->_ConfigPre();   //virtual
    if( retval != 0 )   return retval;

    retval = this->_Config();       //virtual
    if( retval != 0 )   return retval;

    retval = this->_ConfigPost();  //virtual
    if( retval != 0 )   return retval;

    return 0;
}

int MDPELEMENT_I::Enable( void )
{
    int retval;

    //MDP_PRINTF("<%2d> Enable\n",(int)this->dec_id());

    /*Clear Int Done Flag in Kernel space*/
    MdpDrvClearIntDone( this->id() );

    retval = this->_EnablePre();   //virtual
    if( retval != 0 )   return retval;

    retval = this->_Enable();       //virtual
    if( retval != 0 )   return retval;

    retval = this->_EnablePost();  //virtual
    if( retval != 0 )   return retval;

    return 0;

}

int MDPELEMENT_I::Disable( void )
{

    int retval;

    //MDP_PRINTF("<%2d> Disable\n",(int)this->dec_id());

    retval = this->_DisablePre();   //virtual
    if( retval != 0 )   return retval;

    retval = this->_Disable();       //virtual
    if( retval != 0 )   return retval;

    retval = this->_DisablePost();  //virtual
    if( retval != 0 )   return retval;

    return 0;

}

//Return 0: Done -1:timeout
int MDPELEMENT_I::WaitIntDone( unsigned long timeout_ms )
{
    /*Skip module that is bypassed!*/
    if( this->is_bypass() )
        return 0;


    return MdpDrvWaitIntDone( this->id(), timeout_ms );
}


/*Special Register Read/Write function*/
void            MDPELEMENT_I::RegisterWrite32_Mmsys1( unsigned long offset, unsigned long value )
{
    unsigned long reg_base;

    reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();

    MDP_INFO("Write <MMSYS1> Register 0x%08X = 0x%08X\n", (unsigned int)(reg_base + offset) , (unsigned int)value);

    *( (volatile unsigned long*)( reg_base + offset ) ) = value;
}

unsigned long   MDPELEMENT_I::RegisterRead32_Mmsys1( unsigned long offset )
{
    unsigned long reg_base;
    unsigned long reg_val;

    reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();

    reg_val = *( (volatile unsigned long*)( reg_base + offset ) );

    MDP_INFO("Read <MMSYS1> Register 0x%08X = 0x%08X\n", (unsigned int)(reg_base + offset) , (unsigned int)reg_val);

    return reg_val;
}

void            MDPELEMENT_I::RegisterWrite32( unsigned long offset, unsigned long value )
{
    unsigned long reg_base;

    reg_base = this->reg_base_addr();

    if( reg_base == 0 ){
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return;
    }

    MDP_INFO("Write <%s> Register 0x%08X = 0x%08X\n", this->name_str(), (unsigned int)(reg_base + offset) , (unsigned int)value);

    *( (volatile unsigned long*)( reg_base + offset ) ) = value;
}

unsigned long   MDPELEMENT_I::RegisterRead32( unsigned long offset )
{
    unsigned long reg_base;
    unsigned long reg_val;

    reg_base = this->reg_base_addr();

    if( reg_base == 0 ){
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return 0;
    }

    reg_val = *( (volatile unsigned long*)( reg_base + offset ) );

    MDP_INFO("Read <%s> Register 0x%08X = 0x%08X\n", this->name_str(), (unsigned int)(reg_base + offset) , (unsigned int)reg_val);

    return reg_val;
}


int MDPELEMENT_I::DumpRegister( int mode )
{
    unsigned long addr, addr_start, addr_end;
    int i,j;

    switch( mode )
    {
        case 1://Dump system register
            addr_start = this->mmsys1_reg_base_addr();
            addr_end = addr_start + this->mmsys1_reg_range();
            addr_start &= ~(0xF);   //16 bytes align
            break;

        case 0://Dump mdp element register
        default:
            addr_start = this->reg_base_addr();
            addr_end = addr_start + this->reg_range();
            addr_start &= ~(0xF);   //16 bytes align
            break;
    }


    if( addr_start == 0 ){
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }




    MDP_PRINTF("\n");
    MDP_PRINTF("................................................\n");
    if( mode == 1 )     {
        MDP_PRINTF("<%s>:\n","MMSYS1" );
    }else               {
        MDP_PRINTF("<%s>:\n",this->name_str() );   }
    MDP_PRINTF("Address     3 2 1 0  7 6 5 4  B A 9 8  F E D C\n");
              /*0xF0030000 xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx*/

    for( addr = addr_start; addr < addr_end; addr+=16 )
    {
    MDP_PRINTF("0x%08X %08X %08X %08X %08X\n",(unsigned int)addr,
        (unsigned int)*((unsigned long*)(addr)),
        (unsigned int)*((unsigned long*)(addr+4)),
        (unsigned int)*((unsigned long*)(addr+8)),
        (unsigned int)*((unsigned long*)(addr+12))
        );
    }
    MDP_PRINTF("\n");
    if( mode == 0 )
    {
        this->DumpRegisterCustom( mode ); //virtual
    }else
    {
        Mmsys1DumpRegisterCustom();
    }

    MDP_PRINTF("................................................\n");
    MDP_PRINTF("\n");


    return 1;

}

int MDPELEMENT_I::Mmsys1DumpRegisterCustom( void )
{
    int i;
    unsigned long reg_base;
    unsigned long reg_val;
    static const char*  mdp_req0_desc[] =
                    {
                        /*bit  0*/ "BRZ_MOUT -> PRZ1",
                        /*bit  1*/ "VRZ1 -> VDO_ROT1_ISEL",
                        /*bit  2*/ "VRZ0_XOR_OUT -> VDO_ROT1_ISEL",
                        /*bit  3*/ "IPP_MOUT -> VRZ_ISEL",
                        /*bit  4*/ "PRZ0_MOUT -> VRZ_ISEL",
                        /*bit  5*/ "MOUT -> PRZ0_ISEL",
                        /*bit  6*/ "IPP_MOUT -> PRZ0_ISEL",
                        /*bit  7*/ "BRZ_MOUT -> PRZ0_ISEL",
                        /*bit  8*/ "PRZ0_MOUT -> VDO_ROT0_ISEL",
                        /*bit  9*/ "IPP_MOUT -> VDO_ROT0_ISEL",
                        /*bit 10*/ "MOUT -> JPEG_DMA_ISEL",
                        /*bit 11*/ "MOUT -> VRZ_ISEL",
                        /*bit 12*/ "CRZ_XOR_OUT -> OVL_DMA_ISEL",
                        /*bit 13*/ "OVL_DMA_MIMO -> VRZ_ISEL",
                        /*bit 14*/ "OVL_DMA_MIMO -> MOUT_ISEL",
                        /*bit 15*/ "OVL_DMA_MIMO -> IPP_ISEL",
                        /*bit 16*/ "CRZ_XOR_OUT -> IPP_ISEL",
                        /*bit 17*/ "R_DMA0_MOUT -> MOUT_ISEL",
                        /*bit 18*/ "R_DMA0_MOUT -> OVL_DMA_ISEL",
                        /*bit 19*/ "R_DMA0_MOUT -> CAM_ISEL",
                        /*bit 20*/ "LCD -> TV_ROT",
                        /*bit 21*/ "PRZ1_XOR_OUT -> LCD",
                        /*bit 22*/ "PRZ1_XOR_OUT -> RGB_ROT2",
                        /*bit 23*/ "PRZ1 -> PRZ1_XOR_OUT",
                        /*bit 24*/ "PRZ1_ISEL -> PRZ1",
                        /*bit 25*/ "R_DMA1 -> PRZ1_ISEL",
                        /*bit 26*/ "VDO_ROT1 -> R_DMA0",
                        /*bit 27*/ "VDO_ROT1_ISEL -> VDO_ROT1",
                        /*bit 28*/ "RGB_ROT1 -> R_DMA0",
                        /*bit 29*/ "VRZ_XOR_OUT -> RGB_ROT1",
                        /*bit 30*/ "VRZ -> VRZ_XOR_OUT",
                        /*bit 31*/ "VRZ_ISEL -> VRZ"
                    };

    static const char*  mdp_req1_desc[] =
                    {
                        /*bit  0*/ "RGB_ROT0 -> R_DMA0",
                        /*bit  1*/ "PRZ0_MOUT -> RGB_ROT0",
                        /*bit  2*/ "PRZ0 -> PRZ0_MOUT",
                        /*bit  3*/ "PRZ0_ISEL -> PRZ0",
                        /*bit  4*/ "VDO_ROT0 -> R_DMA0",
                        /*bit  5*/ "VDO_ROT0_ISEL -> VDO_ROT0",
                        /*bit  6*/ "MOUT_ISEL -> MOUT",
                        /*bit  7*/ "OVL_DMA -> OVL_DMA_MIMO",
                        /*bit  8*/ "OVL_DMA_ISEL -> OVL_DMA",
                        /*bit  9*/ "RESERVED",
                        /*bit 10*/ "RESERVED",
                        /*bit 11*/ "RESERVED",
                        /*bit 12*/ "RESERVED",
                        /*bit 13*/ "IPP -> IPP_MOUT",
                        /*bit 14*/ "IPP_ISEL -> IPP",
                        /*bit 15*/ "CAM -> EIS",
                        /*bit 16*/ "RESERVED",
                        /*bit 17*/ "RESERVED",
                        /*bit 18*/ "CRZ -> EIS",
                        /*bit 19*/ "CRZ -> CRZ_XOR_OUT",
                        /*bit 20*/ "CAM_ISEL -> CAM",
                        /*bit 21*/ "IPP_MOUT -> JPEG_DMA_ISEL",
                        /*bit 22*/ "JPEG_DMA_ISEL -> JPEG_DMA",
                        /*bit 23*/ "BRZ_MOUT -> VRZ_ISEL",
                        /*bit 24*/ "BRZ_MOUT -> CAM_ISEL",
                        /*bit 25*/ "BRZ -> BRZ_MOUT",
                        /*bit 26*/ "JPEG_DMA -> JPEG_ENC",
                        /*bit 27*/ "R_DMA0_TRIG_SEL -> R_DMA0",
                        /*bit 28*/ "R_DMA0 -> R_DMA0_MOUT",
                        /*bit 29*/ "CAM -> PRZ0_ISEL",
                        /*bit 30*/ "CAM -> CRZ"
                    };

    static const char*  mmsys1_cg_con1_desc[] =
                    {
                        /*bit  0:*/ "VRZ1",
                        /*bit  1:*/ "CSI2",
                        /*bit  2:*/ "FD",
                        /*bit  3:*/ "RESZ_LB",
                        /*bit  4:*/ "TV_ROT",
                        /*bit  5:*/ "LCD",
                        /*bit  6:*/ "RGB_ROT2",
                        /*bit  7:*/ "PRZ1",
                        /*bit  8:*/ "R_DMA1",
                        /*bit  9:*/ "VDO_ROT1",
                        /*bit 10:*/ "RGB_ROT1",
                        /*bit 11:*/ "VRZ",
                        /*bit 12:*/ "RGB_ROT0",
                        /*bit 13:*/ "PRZ0_MOUT",
                        /*bit 14:*/ "PRZ0",
                        /*bit 15:*/ "VDO_ROT0",
                        /*bit 16:*/ "MOUT",
                        /*bit 17:*/ "OVL_DMA_MIMO",
                        /*bit 18:*/ "OVL_DMA_BPS",
                        /*bit 19:*/ "OVL_DMA",
                        /*bit 20:*/ "IPP_MOUT",
                        /*bit 21:*/ "IPP",
                        /*bit 22:*/ "EIS",
                        /*bit 23:*/ "CRZ",
                        /*bit 24:*/ "JPEG_DMA",
                        /*bit 25:*/ "BRZ_MOUT",
                        /*bit 26:*/ "BRZ",
                        /*bit 27:*/ "JPEG_DEC",
                        /*bit 28:*/ "JPEG_ENC",
                        /*bit 29:*/ "R_DMA0_MOUT",
                        /*bit 30:*/ "R_DMA0",
                        /*bit 31:*/ "CAM"
                    };

    static int req0_count = sizeof(mdp_req0_desc)/sizeof(char *);
    static int req1_count = sizeof(mdp_req1_desc)/sizeof(char *);
    static int cgcon1_count = sizeof(mmsys1_cg_con1_desc)/sizeof(char *);



    reg_base = this->mmsys1_reg_base_addr();

    //MT6575_MMSYS_MDP_REQ0
    reg_val = MT6575_MMSYS_MDP_REQ0(reg_base);
    MDP_PRINTF("MT6575_MMSYS_MDP_REQ0:\n");
    for( i = 0; i < req0_count; i++ )
    {
        if( reg_val & ( 0x1 << i ) )
        {
            MDP_PRINTF("\tbit %d : %s\n", i, mdp_req0_desc[i] );
        }
    }
    MDP_PRINTF("\n");

    //MT6575_MMSYS_MDP_REQ1
    reg_val = MT6575_MMSYS_MDP_REQ1(reg_base);
    MDP_PRINTF("MT6575_MMSYS_MDP_REQ1:\n");
    for( i = 0; i < req1_count; i++ )
    {
        if( reg_val & ( 0x1 << i ) )
        {
            MDP_PRINTF("\tbit %d : %s\n", i, mdp_req1_desc[i] );
        }
    }
    MDP_PRINTF("\n");

    //MT6575_MMSYS_CG_CON1
    reg_val = MT6575_MMSYS_CG_CON1(reg_base);
    MDP_PRINTF("MT6575_MMSYS_CG_CON1:\n");
    for( i = 0; i < cgcon1_count; i++ )
    {
        if( (reg_val & ( 0x1 << i )) == 0 )
        {
            MDP_PRINTF("\tbit %d : %s\n", i, mmsys1_cg_con1_desc[i] );
        }
    }
    MDP_PRINTF("\n");


    //MT6575_MMSYS_CAM_ISEL
    reg_val = MT6575_MMSYS_CAM_ISEL(reg_base);
    MDP_PRINTF("CAM_ISEL::SEL=%s\n",reg_val&(0x1)?"BRZ":"RDMA0");
    MDP_PRINTF("CAM_ISEL::BYPASS=%d\n",reg_val&(0x1<<16)?1:0);


    //MT6575_MMSYS_R_DMA0_TRIG_SEL
    reg_val = MT6575_MMSYS_R_DMA0_TRIG_SEL(reg_base);
    reg_val &= 0x03;
    MDP_PRINTF("RDMA0_TRIGER_SRC=");
    switch( reg_val )
    {
    case 0:
        MDP_PRINTF("<MID_VDO_ROT0>\n");
        break;
    case 1:
        MDP_PRINTF("<MID_RGB_ROT0>\n");
        break;
    case 2:
        MDP_PRINTF("<MID_RGB_ROT1>\n");
        break;
    case 3:
        MDP_PRINTF("<MID_VDO_ROT1>\n");
        break;
    default:
        MDP_PRINTF("<?>\n");
        break;
    }


    return 0;



}

int MDPELEMENT_I::DumpDebugInfo( void )
{
    MDP_PRINTF("================================================\n");
    MDP_PRINTF("<%s>:\n",this->name_str() );
    MDP_PRINTF("================================================\n");

    MDP_DUMP_VAR_H(m_mmsys1_reg_base_addr);
    MDP_DUMP_VAR_H(m_reg_base_addr);
    MDP_DUMP_VAR_H(m_descript_base_addr);

    MdpDrv_DumpCallStack(NULL);

    return 0;
}




/*/////////////////////////////////////////////////////////////////////////////
    RDMA_I
  /////////////////////////////////////////////////////////////////////////////*/
static int CheckImgDimensionAlignment(  MdpColorFormat color,
                                        unsigned long src_w, unsigned long roi_w, unsigned long roi_x,
                                        unsigned long src_h, unsigned long roi_h, unsigned long roi_y )
{
    int ret_val = 0;
    int align_factor_power_of_2 = 0; //0:1 align 1:2 align 2:4 align
    int b_vertical_compress = 0;


    switch( color )
    {
    /***RGB***//*No limit*/
    case RGB888 :
    case BGR888 :
    case RGB565 :
    case BGR565 :
    case ARGB8888 :
    case ABGR8888 :
    case RGBA8888 :
    case BGRA8888 :

    /***YUV800***//*No limit*/
    case Y800 ://Y plan only,only ROTDMA0

        return 0;
        break;


    /***YUV422***//*2x limit*/
    case UYVY_Pack ://UYVY,
    case YUY2_Pack ://YUYV
    case YV16_Planar ://YUV422, 2x1 subsampled U/V planes
        align_factor_power_of_2 = 1; // 2 align
        break;

    /***YUV420***//*2x limit*/
    case YV12_Planar ://YUV420, 2x2 subsampled U/V planes,only ROTDMA0
    case NV12 ://YUV420, 2x2 subsampled , interleaved U/V plane,only ROTDMA0
    case NV21 ://YUV420, 2x2 subsampled , interleaved V/U plane,only ROTDMA0
    case ANDROID_YV12://YUV420, 2x2 subsampled U/V planes , 16x stride
    case IMG_YV12:
        b_vertical_compress = 1;
        align_factor_power_of_2 = 1; // 2 align
        break;


    /***YUV411***//*4x limit*/
    case Y411_Pack :
        align_factor_power_of_2 = 2; // 4 align
        break;


    default :
        MDP_ERROR("Config RDMA failed -- unsupport format : %d !!" , color );
        return -1;
        break;
    }


    if( MDP_IS_ALIGN( src_w, align_factor_power_of_2 ) == 0 )
    {
        MDP_ERROR("src_w : %lu not align %d ,color:%d\n",src_w, 0x1<<align_factor_power_of_2,color );
        ret_val = -1;
    }

    if( MDP_IS_ALIGN( roi_w, align_factor_power_of_2 ) == 0 )
    {
        MDP_ERROR("roi_w : %lu not align %d ,color:%d\n",roi_w, 0x1<<align_factor_power_of_2,color  );
        ret_val = -1;
    }


    #if 1
    if( MDP_IS_ALIGN( roi_x, align_factor_power_of_2 ) == 0 )
    {
        MDP_ERROR("roi_x : %lu not align %d ,color:%d\n",roi_x, 0x1<<align_factor_power_of_2,color  );
        ret_val = -1;
    }
    #endif

    if( b_vertical_compress )
    {

        if( MDP_IS_ALIGN( src_h, align_factor_power_of_2 ) == 0 )
        {
            MDP_ERROR("src_h : %lu not align %d ,color:%d\n",src_h, 0x1<<align_factor_power_of_2,color );
            ret_val = -1;
        }

        if( MDP_IS_ALIGN( roi_h, align_factor_power_of_2 ) == 0 )
        {
            MDP_ERROR("roi_h : %lu not align %d ,color:%d\n",roi_h, 0x1<<align_factor_power_of_2,color  );
            ret_val = -1;
        }

        #if 1
        if( MDP_IS_ALIGN( roi_y, align_factor_power_of_2 ) == 0 )
        {
            MDP_ERROR("roi_y : %lu not align %d ,color:%d\n",roi_y, 0x1<<align_factor_power_of_2,color  );
            ret_val = -1;
        }
        #endif

    }


    /*-----------------------------------------------------------------------------
        Other color format
      -----------------------------------------------------------------------------*/
    //Android YV12, stride should be 16x align
    if( color == ANDROID_YV12 )  {
            if( MDP_IS_ALIGN( src_w, 4 ) == 0 )    { //Check if 16x align width
                MDP_ERROR("Width of ANDROID_YV12must be 16x. src_w = %lu\n",src_w );
                ret_val = -1;
            }
    }


    return ret_val;

}


int RDMA_I::_Config( void )
{
    int i;
    unsigned long pBase = 0;
    unsigned long u4DescriptAddr = 0;
    //unsigned long u4BytePerPixel = 0;
    unsigned long u4Index = 0;
    unsigned long u4YOffset = 0;
    unsigned long u4UVOffset = 0;
    MdpDrvColorInfo_t  ci;
    //unsigned long u4yh = 0 , u4yv = 0 , u4uvh = 0, u4uvv = 0;
    //unsigned long u4IsGenericYUV = 0;

    unsigned long uv_stride;


    /*-----------------------------------------------------------------------------
        Parameter sanity check
      -----------------------------------------------------------------------------*/
    if( (0 == src_img_size.w ) || (0 == src_img_size.h ) ||
        (0 == src_img_roi.w  ) || (0 == src_img_roi.h  ) )
    {
        MDP_ERROR("Config RDMA failed -- src or crop win is 0 srcw:%lu,srch:%lu,cropw:%lu,croph:%lu" ,
        	        src_img_size.w , src_img_size.h , src_img_roi.w , src_img_roi.h );
        return -1;
    }

    if( ((src_img_roi.x + src_img_roi.w) > src_img_size.w) ||
    	((src_img_roi.y + src_img_roi.h) > src_img_size.h))
    {
        MDP_ERROR("Config RDMA failed -- crop win exceeds src win srcw:%lu,srch:%lu,cropw:%lu,croph:%lu,cropXoff:%lu,cropYoff:%lu" ,
        	        src_img_size.w , src_img_size.h , src_img_roi.w , src_img_roi.h , src_img_roi.x , src_img_roi.y);
        return -1;
    }

    if(( uInBufferCnt > MT6575RDMA_MAX_RINGBUFFER_CNT) || (0 == uInBufferCnt))
    {
        MDP_ERROR("Config RDMA failed -- input buffer count exceeds range %lu" , uInBufferCnt);
        return -1;
    }



    if( CheckImgDimensionAlignment( src_color_format,
                                    src_img_size.w, src_img_roi.w, src_img_roi.x,
                                    src_img_size.h, src_img_roi.h, src_img_roi.y ) < 0 )
    {
        MDP_ERROR("Alignment check failed\n");
        return -1;
    }

    if( src_img_yuv_addr[0].y == NULL )
    {
        MDP_ERROR("address is NULL. src_img_yuv_addr.y = 0x%08X\n", (unsigned int)src_img_yuv_addr[0].y );
        DumpDebugInfo();
        return -1;
    }


    /*-----------------------------------------------------------------------------
        Calculate buffer size and uv address
      -----------------------------------------------------------------------------*/
    if( MdpDrv_CalImgBufferArraySizeAndFillIn(
                        src_color_format,
                        src_img_yuv_addr,
                        uInBufferCnt,
                        src_img_size,
                        src_img_roi,
                        0, /*rotate*/
                        &src_img_buffer_size_total_ ) < 0 )
    {
        MDP_ERROR("<%s> Calc memory array size error.\n", this->name_str() );
        return -1;
    }


    /*-----------------------------------------------------------------------------
        Adapt source image address
      -----------------------------------------------------------------------------*/

    if( MdpDrv_AdaptMdpYuvAddrArray(    this->id(),
                                        src_img_yuv_addr, uInBufferCnt, //Original user input address
                                        src_img_yuv_adapt_addr,         //Output adapted address
                                        &adapt_m4u_flag_bit_,           //If address has been adapted with m4u mva, corresponding bit will set to 1
                                        &alloc_mva_flag_bit_ ) != 0 )   //Corresponding bit will set to 1 if the mva is new allocated
    {
        MDP_ERROR("<%s> Adapt Memory error.\n", this->name_str() );
        DumpDebugInfo();
        return -1;
    }


    /*-----------------------------------------------------------------------------
        Get Color format information
      -----------------------------------------------------------------------------*/
    if( MdpDrvColorFormatInfoGet( src_color_format, &ci  ) < 0 )
    {
        MDP_ERROR("Config RDMA failed -- unsupport format : %d !!" , src_color_format );
        return -1;
    }


    u4YOffset = ( (unsigned long)src_img_size.w * src_img_roi.y + src_img_roi.x )  * ci.byte_per_pixel;

    if( ci.b_is_generic_yuv )
    {
        uv_stride = src_img_size.w >> (ci.uvh ? (ci.uvh - 1) : 0); //uv_stride = y_stride/2^(uv__sample_period-1)
        MDP_ROUND_UP( uv_stride, ci.uv_stride_align );//Round uv stride to align
        u4UVOffset = (  (uv_stride * ( src_img_roi.y >> (ci.uvv?(ci.uvv-1):0) ) )   +  ( src_img_roi.x >> (ci.uvh?(ci.uvh-1):0) )  ) * ci.byte_per_pixel;

    }else
    {
        uv_stride = 0;
    }


    /*-----------------------------------------------------------------------------
        Get register base address and descript base address
      -----------------------------------------------------------------------------*/

    pBase           = this->reg_base_addr();      //virtual
    u4DescriptAddr  = this->descript_base_addr(); //virtual

    if( pBase == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }



   /*-----------------------------------------------------------------------------
       R/W register
     -----------------------------------------------------------------------------*/
    if( Reset() < 0 ){
        return -1;
    }

    MT6575_RDMA_CON(pBase) = ((1 << 31) | ((unsigned long)bEISEn << 28) | (0xf << 12) | (1 << 7));

    MT6575_RDMA_CON(pBase) +=   ci.in_format +( ci.swap << 8 ) +
                                (ci.yh << 16) + (ci.uvh << 18) + (ci.uvh << 20) + (ci.yv << 22) + (ci.uvv << 24) + (ci.uvv << 26) +
                                ((( ci.uv_stride_align == 0 )? 0:1) << 29 );    //UV_PITCH_SPLIT


    //Enable all interrupts
    MT6575_RDMA_IRQ_FLAG(pBase) = (0x23 << 16);

    //Clear interrupts
    MT6575_RDMA_IRQ_FLAG_CLR(pBase) = 0x23;

    MT6575_RDMA_CFG(pBase) = ((bCamIn << 31) | (bHWTrigger << 4) | bContinuous); //Enable FRAME_SYNC_EN(31) / HW_RIGGER_EN(4) / AUTO_LOOP(0)


    //Descript mode
#ifdef MDP_FLAG_1_SUPPORT_DESCRIPTOR

    MT6575_RDMA_CFG(pBase) =    (bCamIn << 31) |        /*Frame Sync*/
                                (0x7 << 16) |           /*YUV Address*/
                                (0 << 15) |             /*Q_EMPTY_DROP. 1:Enable 0:Disable (For RDMA, Use 0)*/
                                ((uInBufferCnt-1) << 8) |   /*QUEUE_DEPTH*/
                                (1 << 7) |              /*MODE*/
                                (bHWTrigger << 4) |     /*HW_TRIGGER*/
                                0; //bContinuous;            /*Disable auto loop when in descriptor mode*/
#else
    MT6575_RDMA_CFG(pBase) = ((bCamIn << 31) | (bHWTrigger << 4) | bContinuous); //Enable FRAME_SYNC_EN(31) / HW_RIGGER_EN(4) / AUTO_LOOP(0)
#endif

    MT6575_RDMA_IN_SEL(pBase) = 0;

    MT6575_RDMA_QUEUE_BASE(pBase) = u4DescriptAddr;

    MT6575_RDMA_Y_SRC_STR_ADDR(pBase) = src_img_yuv_adapt_addr[0].y + u4YOffset;

    MT6575_RDMA_U_SRC_STR_ADDR(pBase)	 = src_img_yuv_adapt_addr[0].u  + u4UVOffset;

    MT6575_RDMA_V_SRC_STR_ADDR(pBase) = src_img_yuv_adapt_addr[0].v + u4UVOffset;

#ifdef MDP_FLAG_1_SUPPORT_DESCRIPTOR
    for( u4Index = 0 ; u4Index < uInBufferCnt ; u4Index += 1)
    {
        unsigned long time_out;

        /*Backup descriptor value, for latter queue/dequeue used*/
        desc_src_img_yuv_addr_[u4Index].y = src_img_yuv_adapt_addr[u4Index].y + u4YOffset;
        desc_src_img_yuv_addr_[u4Index].u = src_img_yuv_adapt_addr[u4Index].u + u4UVOffset;
        desc_src_img_yuv_addr_[u4Index].v = src_img_yuv_adapt_addr[u4Index].v + u4UVOffset;

        /*Fill descriptor command*/
        if( DescWriteWaitBusy( pBase ) < 0 ){
            MDP_ERROR("Config RDMA failed -- put command 1 queue timeout\n");
            return -1;
        }
        MT6575_RDMA_QUEUE_DATA(pBase) = desc_src_img_yuv_addr_[u4Index].y;


        if( DescWriteWaitBusy( pBase ) < 0 ){
            MDP_ERROR("Config RDMA failed -- put command 2 queue timeout\n");
            return -1;
        }
        MT6575_RDMA_QUEUE_DATA(pBase) = desc_src_img_yuv_addr_[u4Index].u;


        if( DescWriteWaitBusy( pBase ) < 0 ){
            MDP_ERROR("Config RDMA failed -- put command 3 queue timeout\n");
            return -1;
        }
        MT6575_RDMA_QUEUE_DATA(pBase) = desc_src_img_yuv_addr_[u4Index].v;

    }

    /*Reset SW write index , this "0" is actually a overflow value of uInBufferCnt.*/
    desc_sw_write_index_ = 0;
#endif

    MT6575_RDMA_SRC_SIZE(pBase) = src_img_size.w + ((unsigned long)src_img_size.h << 16);

    MT6575_RDMA_SRC_SIZE_IN_BYTE(pBase) = ( (unsigned long)src_img_size.w * ci.byte_per_pixel ) & 0x0003FFFF;
    if( ci.uv_stride_align != 0 ) {
        MT6575_RDMA_SRC_SIZE_IN_BYTE(pBase) |= ( ( uv_stride * ci.byte_per_pixel * 2 ) << 18 ); /* x2 is specific to hw*/
    }



    MT6575_RDMA_CLIP_SIZE(pBase) = src_img_roi.w + ((unsigned long)src_img_roi.h << 16);

    MT6575_RDMA_CLIP_SIZE_IN_BYTE(pBase) = (unsigned long)src_img_roi.w * ci.byte_per_pixel;

    MT6575_RDMA_SLOW_DOWN(pBase) = 0;

    if( bEISEn )
    {
        //Allocate sysram
        sysram_.size = ((src_img_roi.w*18 + 2) >> 2);
        sysram_.address = 0;
        if( sysram_.size != 0 )
        {
            sysram_.address = MdpSysram_Alloc( this->id(), sysram_.size , MDPSYSRAM_SUBCAT_NORMAL/*0*/ );

            if( sysram_.address == 0 )
            {
                sysram_.size == 0;
                MDP_ERROR("SYSRAM allocate failed!\n");
                DumpDebugInfo();
                return -1;
            }
        }

        MT6575_RDMA_EIS_STR(pBase) = sysram_.address;

        MT6575_RDMA_EIS_CON(pBase) = u4EISCON;
    }


    return 0;
}


int RDMA_I::_Enable( void )
{
    unsigned long u4Base;

    /*-----------------------------------------------------------------------------
        Clean Cache before HW read
      -----------------------------------------------------------------------------*/
    if( MdpDrv_CacheSync(   this->id(),
                            MDPDRV_CACHE_SYNC_CLEAN_BEFORE_HW_READ_MEM,
                            src_img_yuv_addr[0].y , src_img_buffer_size_total_
                         ) < 0 )
    {
        MDP_WARNING("RDMA Cache sync error. address:0x%08X size:0x%08X\n", (unsigned int)src_img_yuv_addr[0].y, (unsigned int)src_img_buffer_size_total_ );
        //return -1;
    }


    /*-----------------------------------------------------------------------------
        Trigger HW
      -----------------------------------------------------------------------------*/
    u4Base = this->reg_base_addr();      //virtual

    if( u4Base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    MT6575_RDMA_STOP(u4Base) = 0;

    MT6575_RDMA_EN(u4Base) = 1;

    return 0;

}

int RDMA_I::_Disable( void )
{
    unsigned long pBase;

    pBase = this->reg_base_addr();      //virtual

    if( pBase == 0 )
    {
        return -1;
    }

//    MT6575_RDMA_QUEUE_RSTA(pBase) = 0;

    MT6575_RDMA_STOP(pBase) = 1;

    MT6575_RDMA_STOP(pBase) = 0;

    if( sysram_.size > 0 )
    {
        MdpSysram_Free( this->id(), sysram_.address , MDPSYSRAM_SUBCAT_NORMAL/*0*/ );
        sysram_.address = 0;
        sysram_.size = 0;
    }

    Reset();

    /*UnAdapt MdpYuvAddress*/
    MdpDrv_UnAdaptMdpYuvAddrArray( this->id(),
                                    src_img_yuv_addr, uInBufferCnt,     //Original user input address
                                    src_img_yuv_adapt_addr,             //Adapted address
                                    adapt_m4u_flag_bit_,                //Corresponding bit will set to 1 if address had been adapted with m4u mva
                                    alloc_mva_flag_bit_ );              //Corresponding bit will set to 1 if the mva is new allocated


    return 0;
}

int RDMA_I::CheckBusy( unsigned long* param  )
{
    unsigned long pBase;

    unsigned long u4Result;

    pBase = this->reg_base_addr();      //virtual


    if( pBase == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    //Check error status as well
    u4Result = MT6575_RDMA_IRQ_FLAG(pBase);

    //Clear interrupt
    MT6575_RDMA_IRQ_FLAG_CLR(pBase) = (0x23 & u4Result);

    u4Result &= 0x23;

    if(u4Result & 0x1){     /*MDP_INFO("Read done\n");*/}

    if(u4Result & 0x2){     MDP_ERROR("SW configuration error. Invalid descriptor mode\n");}

    if(u4Result & 0x20){    MDP_ERROR("SW configuration error. SW writing to descriptor queue when command queue writer is busy\n");}

//    return MT6575_RDMA_EN(pBase);
    return (u4Result & 0x1 ? 0 : 1);
}


int RDMA_I::DescQueueGetFreeList( unsigned long *p_StartIndex, unsigned long *p_Count ) /*Equel to De Queue Buffer in 73*/
{
    unsigned long   reg_base;
    unsigned long   reg_val;
    int             b_empty;
    unsigned long   hw_read_index; /*RPT*/

    reg_base = this->reg_base_addr();

    reg_val = MT6575_RDMA_QUEUE_RSTA( reg_base );


    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    b_empty         = ( reg_val & ( 0x1 << 2 ) ) >> 2;
    hw_read_index   = ( reg_val & ( 0xF << 8 ) ) >> 8;


    /*Queue is full*/
    if( !b_empty &&  ( hw_read_index == desc_sw_write_index_ ) )
    {
        *p_StartIndex = desc_sw_write_index_;
        *p_Count = 0;

        //MDP_INFO("1:b_empty=%d , hw_read_index=%d, desc_sw_write_index_=%d\n",(int)b_empty,(int)hw_read_index,(int)desc_sw_write_index_ );
    }
    /*Queue still has free space, return immediately and no wait*/
    else
    {

        *p_StartIndex = desc_sw_write_index_;
        *p_Count = ( hw_read_index > desc_sw_write_index_ ?
                        (hw_read_index - desc_sw_write_index_) :
                        (uInBufferCnt - desc_sw_write_index_ + hw_read_index)  );

        //MDP_INFO("2:b_empty=%d , hw_read_index=%d, desc_sw_write_index_=%d\n",(int)b_empty,(int)hw_read_index,(int)desc_sw_write_index_ );

    }

    MDP_INFO_QUEUE("[QUEUE]Dequeue <%s> get free list. Start = %d Count = %d\n",name_str(),(int)*p_StartIndex, (int)*p_Count );

    return 0;

}


int RDMA_I::DescQueueRefill( void ) /*Equel to En Queue Buffer in 73*/
{
    unsigned long reg_base;
    unsigned long rot_id;
    unsigned long reg_val;

    reg_base = this->reg_base_addr();
    rot_id   = this->id();


    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    if( DescWriteWaitBusy( reg_base ) < 0 )
    {
        MDP_ERROR("RDMA<%s> Desc Refill Error.-Y TimeOut\n",this->name_str());
        return -1;
    }
    /*EnQueue*/
    MT6575_RDMA_QUEUE_DATA(reg_base) = desc_src_img_yuv_addr_[desc_sw_write_index_].y;

    if( DescWriteWaitBusy( reg_base ) < 0 ){
        MDP_ERROR("RDMA<%s> Desc Refill Error.-U TimeOut\n",this->name_str());
        return -1;
    }
    /*EnQueue*/
    MT6575_RDMA_QUEUE_DATA(reg_base) = desc_src_img_yuv_addr_[desc_sw_write_index_].u;


    if( DescWriteWaitBusy( reg_base ) < 0 ){
        MDP_ERROR("RDMA<%s> Desc Refill Error.-V TimeOut\n",this->name_str());
        return -1;
    }
    /*EnQueue*/
    MT6575_RDMA_QUEUE_DATA(reg_base) = desc_src_img_yuv_addr_[desc_sw_write_index_].v;


    /*Update SW write index*/
    desc_sw_write_index_ = (desc_sw_write_index_+1) == uInBufferCnt ? 0 : (desc_sw_write_index_+1);

    MDP_INFO_QUEUE("[QUEUE]Enqueue <%s> Refill 1. Index = %d\n",this->name_str(), (int)desc_sw_write_index_ );



    return 0;


}


inline int RDMA_I::DescWriteWaitBusy( unsigned long reg_base )
{
    unsigned long u4TimeOut;

    u4TimeOut = 0;
    while(!(MT6575_RDMA_QUEUE_WSTA(reg_base) & 0x1))
    {
        u4TimeOut += 1;
        if(u4TimeOut  > 10000)
        {
            return -1;
        }
    }

    return 0;
}

int RDMA_I::Reset( void )
{
    volatile unsigned long time_out = MDP_DMA_RESET_TIMEOUT;//0xFFFFFFFF;
    unsigned long reg_base;
    volatile unsigned long time_delay;

    reg_base = this->reg_base_addr();
    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    MT6575_RDMA_RESET( reg_base ) = 0x2;

    while(0x2 == MT6575_RDMA_RESET( reg_base ))
    {
        time_out--;
        if( time_out == 0 ){
            MDP_ERROR("<%s> reset timeout, proceed hard reset!\n",this->name_str());

            /*Hard Reset*/
            MT6575_RDMA_RESET( reg_base ) = 0x1;
            for( time_delay = 0 ; time_delay < 50 ; time_delay += 1){;}//Delay 50 cycle
            MT6575_RDMA_RESET( reg_base ) = 0x0;

            return 0;
        }
    }

    return 0;
}

int RDMA_I::HardReset( void )
{
    unsigned long reg_base;
    volatile unsigned long time_delay;

    reg_base = this->reg_base_addr();
    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    /*Hard Reset*/
    MT6575_RDMA_RESET( reg_base ) = 0x1;
    for( time_delay = 0 ; time_delay < 50 ; time_delay += 1){;}//Delay 50 cycle
    MT6575_RDMA_RESET( reg_base ) = 0x0;

    MDP_WARNING("<%s> Hard Reset!\n",this->name_str());


    return 0;
}





int RDMA_I::DumpRegisterCustom( int mode )
{
    unsigned long reg_base;
    unsigned long reg_val;

    reg_base = this->reg_base_addr();


    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    //MT6575_RDMA_CFG
    reg_val = MT6575_RDMA_CFG(reg_base);
    MDP_PRINTF("AUTO_LOOP=%d\n",reg_val&(0x1)?1:0);
    MDP_PRINTF("FRAME_SYNC_EN=%d\n",reg_val&(0x1<<31)?1:0);
    MDP_PRINTF("HW_TRIGGER_EN=%d\n",reg_val&(0x1<<4)?1:0);

    //MT6575_RDMA_IRQ_FLAG
    reg_val = MT6575_RDMA_IRQ_FLAG(reg_base);
    MDP_PRINTF("FLAG0(Done)=%d\n",reg_val&(0x1)?1:0);
    MDP_PRINTF("FLAG1(SW err)=%d\n",reg_val&(0x1<<1)?1:0);
    MDP_PRINTF("FLAG5(SW err)=%d\n",reg_val&(0x1<<5)?1:0);


    //MT6575_RDMA_DEBUG_STATUS14
    reg_val = MT6575_RDMA_DEBUG_STATUS14(reg_base);
    MDP_PRINTF("RENG0_CUR_X(Y counter)=%d RENG1_CUR_X(U counter)=%d\n", (int)((reg_val>>16)&0x0000FFFF), (int)(reg_val&0x0000FFFF) );

    //MT6575_RDMA_DEBUG_STATUS15
    reg_val = MT6575_RDMA_DEBUG_STATUS15(reg_base);
    MDP_PRINTF("RENG2_CUR_X(V counter)=%d\n", (int)(reg_val&0x0000FFFF) );


    //MT6575_RDMA_SRC_SIZE
    reg_val = MT6575_RDMA_SRC_SIZE(reg_base);
    MDP_PRINTF("SRC_W=%d\n",(int)(reg_val&0x0000FFFF));
    MDP_PRINTF("SRC_H=%d\n",(int)((reg_val&0xFFFF0000)>>16));

    //MT6575_RDMA_SRC_SIZE_IN_BYTE
    reg_val = MT6575_RDMA_SRC_SIZE_IN_BYTE(reg_base);
    MDP_PRINTF("SRC_W_SIZE=%d\n",(int)(reg_val));

    //MT6575_RDMA_CLIP_SIZE
    reg_val = MT6575_RDMA_CLIP_SIZE(reg_base);
    MDP_PRINTF("CLIP_W=%d\n",(int)(reg_val&0x0000FFFF));
    MDP_PRINTF("CLIP_H=%d\n",(int)((reg_val&0xFFFF0000)>>16));

    //MT6575_RDMA_CLIP_SIZE_IN_BYTE
    reg_val = MT6575_RDMA_CLIP_SIZE_IN_BYTE(reg_base);
    MDP_PRINTF("CLIP_W_SIZE=%d\n",(int)(reg_val));


    //MT6575_ROT_DMA_QUEUE_RSTA
    reg_val = MT6575_RDMA_QUEUE_RSTA(reg_base);
    MDP_PRINTF("Descripter EMPTY=%d\n",(int)((reg_val & 0x4) >> 2));


    return 0;
}

int RDMA_I::DumpDebugInfo( void )
{
    int i;

    MDPELEMENT_I::DumpDebugInfo();


    MDP_DUMP_VAR_D(uInBufferCnt);
    for( i = 0; i < uInBufferCnt; i++ ) {
        MDP_PRINTF("%d/%d:\n",(int)i+1,(int)uInBufferCnt);
        MDP_DUMP_VAR_H(src_img_yuv_addr[i].y);
        MDP_DUMP_VAR_H(src_img_yuv_addr[i].u);
        MDP_DUMP_VAR_H(src_img_yuv_addr[i].v);
        MDP_DUMP_VAR_H(src_img_yuv_addr[i].y_buffer_size);
        MDP_DUMP_VAR_H(src_img_yuv_addr[i].u_buffer_size);
        MDP_DUMP_VAR_H(src_img_yuv_addr[i].v_buffer_size);
        MDP_PRINTF("\n");
    }
    MDP_DUMP_VAR_D(src_img_size.w);
    MDP_DUMP_VAR_D(src_img_size.h);

    MDP_DUMP_VAR_D(src_img_roi.x);
    MDP_DUMP_VAR_D(src_img_roi.y);
    MDP_DUMP_VAR_D(src_img_roi.w);
    MDP_DUMP_VAR_D(src_img_roi.h);

    MDP_DUMP_VAR_D(src_color_format);

    MDP_DUMP_VAR_D(bHWTrigger);
    MDP_DUMP_VAR_D(bContinuous);
    MDP_DUMP_VAR_D(bCamIn);

    MDP_DUMP_VAR_D(bEISEn);
    MDP_DUMP_VAR_H(u4EISCON);

    for( i = 0; i < uInBufferCnt; i++ ) {
        MDP_PRINTF("%d/%d:\n",(int)i,(int)uInBufferCnt);
        MDP_DUMP_VAR_H(src_img_yuv_adapt_addr[i].y);
        MDP_DUMP_VAR_H(src_img_yuv_adapt_addr[i].u);
        MDP_DUMP_VAR_H(src_img_yuv_adapt_addr[i].v);
        MDP_DUMP_VAR_H(src_img_yuv_adapt_addr[i].y_buffer_size);
        MDP_DUMP_VAR_H(src_img_yuv_adapt_addr[i].u_buffer_size);
        MDP_DUMP_VAR_H(src_img_yuv_adapt_addr[i].v_buffer_size);
    }
    MDP_DUMP_VAR_H(adapt_m4u_flag_bit_);
    MDP_DUMP_VAR_H(src_img_buffer_size_total_);

    MDP_DUMP_VAR_H(sysram_.address);
    MDP_DUMP_VAR_H(sysram_.size);

    for( i = 0; i < uInBufferCnt; i++ ) {
        MDP_PRINTF("%d/%d:\n",(int)i,(int)uInBufferCnt);
        MDP_DUMP_VAR_H(desc_src_img_yuv_addr_[i].y);
        MDP_DUMP_VAR_H(desc_src_img_yuv_addr_[i].u);
        MDP_DUMP_VAR_H(desc_src_img_yuv_addr_[i].v);
        MDP_DUMP_VAR_H(desc_src_img_yuv_addr_[i].y_buffer_size);
        MDP_DUMP_VAR_H(desc_src_img_yuv_addr_[i].u_buffer_size);
        MDP_DUMP_VAR_H(desc_src_img_yuv_addr_[i].v_buffer_size);
    }

    MDP_DUMP_VAR_D(desc_sw_write_index_);

    return 0;

}


int RDMA_I::CalcBufferSize( void )
{

    return 0;
}



/*/////////////////////////////////////////////////////////////////////////////
    RDMA0
  /////////////////////////////////////////////////////////////////////////////*/
int RDMA0::_ConfigPre( void )
{
    unsigned long mmsys1_reg_base;

    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();

    MT6575_MMSYS_R_DMA0_MOUT_CLR(mmsys1_reg_base) = 1;
    MT6575_MMSYS_R_DMA0_MOUT_CLR(mmsys1_reg_base) = 0;
    MT6575_MMSYS_R_DMA0_MOUT_SEL(mmsys1_reg_base) = ( to_cam << 0 | to_ovl << 1 | to_mout << 2 );
    MT6575_MMSYS_R_DMA0_TRIG_SEL(mmsys1_reg_base) = trigger_src;

    return 0;
}

int RDMA0::_ConfigPost( void )
{
    return 0;
}

int RDMA0::_EnablePre( void )
{
    unsigned long mmsys1_reg_base;

    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();
    MT6575_MMSYS_R_DMA0_MOUT_EN(mmsys1_reg_base) = 1;
    return 0;
}

int RDMA0::_EnablePost( void )
{
    return 0;
}

int RDMA0::_DisablePre( void )
{
    return 0;
}


int RDMA0::_DisablePost( void )
{
    unsigned long mmsys1_reg_base;

    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();

    if( mmsys1_reg_base == 0 ){
        return -1;
    }


    MT6575_MMSYS_R_DMA0_MOUT_EN(mmsys1_reg_base) = 0;
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    RDMA1
  /////////////////////////////////////////////////////////////////////////////*/
int RDMA1::_ConfigPre( void )
{
    return 0;
}

int RDMA1::_ConfigPost( void )
{
    return 0;
}

int RDMA1::_EnablePre( void )
{
    return 0;
}

int RDMA1::_EnablePost( void )
{
    return 0;
}

int RDMA1::_DisablePre( void )
{
    return 0;
}


int RDMA1::_DisablePost( void )
{
    /*
    unsigned long mmsys1_reg_base;

    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();
    MT6575_MMSYS_R_DMA0_MOUT_EN(mmsys1_reg_base) = 0;
    */
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    MOUT
  /////////////////////////////////////////////////////////////////////////////*/
int MOUT::_Config( void )
{
    unsigned long base_addr;

    base_addr = this->reg_base_addr(); //virtual


    if( base_addr == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    #if 1
    Reset();
    #else
    MT6575_MMSYS_MOUT_CLR(base_addr) = 1;

    MT6575_MMSYS_MOUT_CLR(base_addr) = 0;
    #endif

    MT6575_MMSYS_MOUT_ISEL(base_addr) = src_sel;

    MT6575_MMSYS_MOUT_SEL(base_addr) = (to_jpg_dma | (to_prz0 << 1) | (to_vrz0 << 2) | (to_prz1 << 3));

    MT6575_MMSYS_MOUT_CON(base_addr) = bCamIn;

    /*?:Why??*/
    if(0 == src_sel)
    {
        MT6575_MMSYS_R_DMA0_MOUT_SEL(base_addr) = 0x4;
    }

    return 0;
}

int MOUT::_Enable( void )
{
    unsigned long base_addr;

    base_addr = reg_base_addr(); //virtual

    if( base_addr == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    MT6575_MMSYS_MOUT_EN(base_addr) = 1;
    return 0;
}

int MOUT::_Disable( void )
{
    unsigned long base_addr;

    base_addr = reg_base_addr(); //virtual


    if( base_addr == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    MT6575_MMSYS_MOUT_EN(base_addr) = 0;

    Reset();


    return 0;
}

int MOUT::CheckBusy( unsigned long* param )
{
    return 0;
}

int MOUT::Reset( void )
{

    unsigned long base_addr;

    base_addr = reg_base_addr(); //virtual


    if( base_addr == 0 ){
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    MT6575_MMSYS_MOUT_CLR(base_addr) = 1;
    MT6575_MMSYS_MOUT_CLR(base_addr) = 0;

    return 0;
}

int MOUT::HardReset( void )
{
    return Reset(); //Warm reset only
}


int MOUT::DumpRegisterCustom( int mode )
{


    return 0;
}


/*/////////////////////////////////////////////////////////////////////////////
    OVL
  /////////////////////////////////////////////////////////////////////////////*/
int OVL::_Config( void )
{

    //Parameter sanity check
    unsigned long u4Index = 0;
    unsigned long u4TimeOut = 0;
    unsigned long base_addr;
    unsigned long mmsys1_reg_base;

    base_addr = this->reg_base_addr(); //virtual
    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();


    if( base_addr == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    if( Reset() < 0 ){
        return -1;
    }

    //Config MOUT
    MT6575_MMSYS_OVL_DMA_ISEL(mmsys1_reg_base) = bSource;
    if(0 == bSource)
    {
        MT6575_MMSYS_R_DMA0_MOUT_SEL(mmsys1_reg_base) = 0x2;
    }
    MT6575_MMSYS_OVL_DMA_MIMO_CON(mmsys1_reg_base) = bCamIn;
    MT6575_MMSYS_OVL_DMA_MIMO_OUTEN(mmsys1_reg_base) = (bToIPP | (bToVRZ << 1) | (bToMOUT << 2));
    MT6575_MMSYS_OVL_DMA_MIMO_OUTSEL(mmsys1_reg_base) =(bToIPP | (bToVRZ << 2) | (bToMOUT << 4));

    #if 0
    //Reset OVL and MOUT before bypass
    {
        MT6575_MMSYS_OVL_DMA_MIMO_CLR(mmsys1_reg_base) = 1;
        MT6575_MMSYS_OVL_DMA_MIMO_CLR(mmsys1_reg_base) = 0;
        MT6575_OVL_DMA_RESET(base_addr) = 0x2;

        //TODO : Add time out
        while(0x2 == MT6575_OVL_DMA_RESET(base_addr))
        {
            MDP_INFO("Waiting for OVL reset done\n");
        }

        MT6575_OVL_DMA_IRQ_FLAG(base_addr) = (0x33 << 16);
        MT6575_OVL_DMA_IRQ_FLAG_CLR(base_addr) = 0x33;
    }
    #endif


    MT6575_OVL_DMA_IN_SEL(base_addr) = (bBypass << 15);
    if(bBypass)
    {
        /*MT6575_OVL_DMA_IN_SEL(base_addr) = (bBypass << 15);*//*CLS:Not Here!*/
        MT6575_OVL_DMA_CFG(base_addr) = (bCamIn ? (1 << 31) : 0);
        return 0;
    }

    //Parameter check
    //Check parameter
    //Rule1.
    if((uMaskCnt > 16) || (uMaskCnt == 0))
    {
        MDP_ERROR("Config OVL failed -- mask count exceeds limit : %lu\n" , uMaskCnt);
        return -1;
    }

    for(u4Index = 0 ; u4Index < uMaskCnt ; u4Index += 1)
    {

        //Rule2. Mask source size should be lesser or equal to mask destinaiton size
        if((u2MaskSrcWidthInPixel[u4Index] > u2MaskDestWidthInPixel[u4Index]) ||
            (u2MaskSrcHeightInLine[u4Index] > u2MaskDestHeightInLine[u4Index]))
        {
            MDP_ERROR("Config OVL failed -- mask src should be lesser than dest srcw: %d,destw:%d,srch:%d,desth:%d\n" , u2MaskSrcWidthInPixel[u4Index] , u2MaskDestWidthInPixel[u4Index] ,
                u2MaskSrcHeightInLine[u4Index] , u2MaskDestHeightInLine[u4Index]);
            return -1;
        }


        //Rule3. Mask destinaiton size should be less than input image
        if((u2MaskDestWidthInPixel[u4Index] > u2SrcWidthInPixel) ||
            (u2MaskDestHeightInLine[u4Index] > u2SrcHeightInLine))
        {
            MDP_ERROR("Config OVL failed -- mask src should be larger than dest srcw: %d,destw:%d,srch:%d,desth:%d\n" , u2SrcWidthInPixel , u2MaskDestWidthInPixel[u4Index] ,
                u2SrcHeightInLine , u2MaskDestHeightInLine[u4Index]);
            return -1;
        }

        //Rule4. Base address should be 8 byte aligned
        if((u4HeaderBaseAddr[u4Index] & 0x7) || (u4MaskBaseAddr[u4Index] & 0x7))
        {
            MDP_ERROR("Config OVL failed -- header or mask base addr must be 8 bytes aligned : head : %lu, mask : %lu\n" , u4HeaderBaseAddr[u4Index] , u4MaskBaseAddr[u4Index]);
            return -1;
        }

        //Rule5. Src/Dest mask size should not be 0
        if((0 == u2MaskSrcWidthInPixel[u4Index]) || (0 == u2MaskSrcHeightInLine[u4Index]) ||
            (0 == u2MaskDestWidthInPixel[u4Index]) || (0 == u2MaskDestHeightInLine[u4Index]))
        {
            MDP_ERROR("Config OVL failed -- header or mask size cannot be 0\n");
            return -1;
        }

        if((u2MaskOffsetX[u4Index] > 32767) || (u2MaskOffsetY[u4Index] > 32767) ||
            (u2MaskSrcWidthInPixel[u4Index] > 32768) || (u2MaskSrcHeightInLine[u4Index] > 32768))
        {
            MDP_ERROR("Config OVL failed -mask size overflow\n");
            return -1;
        }
    }

    if((u2SrcWidthInPixel > 32768) || (u2SrcHeightInLine > 32768))
    {
        MDP_ERROR("Config OVL failed -source size overflow\n");
        return -1;
    }


    //Descriptor mode
#ifdef DESCRIPTMODE
    MT6575_OVL_DMA_CFG(base_addr) = ((bContinuous << 31) | (0x3 << 16) | (uMaskCnt << 8) | (1 << 7) | bContinuous);//TODO : decide which register should be en
#else
    MT6575_OVL_DMA_CFG(base_addr) = ((bContinuous << 31) | bContinuous);
#endif

    MT6575_OVL_DMA_QUEUE_BASE(base_addr) = 0x40043BC0;//0xF4043C80; /*CLS:What's this??*/

#ifdef DESCRIPTMODE
    for(u4Index = 0; u4Index < uMaskCnt ; u4Index += 1)
    {
        //Header base address
        MT6575_OVL_DMA_QUEUE_DATA(base_addr) = u4HeaderBaseAddr[u4Index];
        while(!(MT6575_OVL_DMA_QUEUE_WSTA(base_addr) & 0x1))
        {
            u4TimeOut += 1;
            if(u4TimeOut  > 10000)
            {
                MDP_ERROR("Write to OVL descriptor time out 1!!\n");
                return -1;
            }
        }
        u4TimeOut = 0;

        //Mask base address
        MT6575_OVL_DMA_QUEUE_DATA(base_addr) = u4MaskBaseAddr[u4Index];
        while(!(MT6575_OVL_DMA_QUEUE_WSTA(base_addr) & 0x1))
        {
            u4TimeOut += 1;
            if(u4TimeOut  > 10000)
            {
                MDP_ERROR("Write to OVL descriptor time out 2!!\n");
                return -1;
            }
        }
        u4TimeOut = 0;

        //mask offset
        MT6575_OVL_DMA_QUEUE_DATA(base_addr) = (u2MaskOffsetX[u4Index] | ((unsigned long)u2MaskOffsetY[u4Index] << 16));
        while(!(MT6575_OVL_DMA_QUEUE_WSTA(base_addr) & 0x1))
        {
            u4TimeOut += 1;
            if(u4TimeOut  > 10000)
            {
                MDP_ERROR("Write to OVL descriptor time out 3!!\n");
                return -1;
            }
        }
        u4TimeOut = 0;

        //mask source size
        MT6575_OVL_DMA_QUEUE_DATA(base_addr) = (u2MaskSrcWidthInPixel[0] | ((unsigned long)u2MaskSrcHeightInLine[0] << 16));
        while(!(MT6575_OVL_DMA_QUEUE_WSTA(base_addr) & 0x1))
        {
            u4TimeOut += 1;
            if(u4TimeOut  > 10000)
            {
                MDP_ERROR("Write to OVL descriptor time out 4!!\n");
                return -1;
            }
        }
        u4TimeOut = 0;

        //mask destination size
        MT6575_OVL_DMA_QUEUE_DATA(base_addr) = (u2MaskDestWidthInPixel[0] | ((unsigned long)u2MaskDestHeightInLine[0] << 16));
        while(!(MT6575_OVL_DMA_QUEUE_WSTA(base_addr) & 0x1))
        {
            u4TimeOut += 1;
            if(u4TimeOut  > 10000)
            {
                MDP_ERROR("Write to OVL descriptor time out 4!!\n");
                return -1;
            }
        }
        u4TimeOut = 0;

        //CON
        MT6575_OVL_DMA_QUEUE_DATA(base_addr) = ((u2MaskSrcHeightInLine[0] - (u2MaskDestHeightInLine[0]%u2MaskSrcHeightInLine[0]))%u2MaskSrcHeightInLine[0]) |
            ((unsigned long)uColorKey << 16);
        while(!(MT6575_OVL_DMA_QUEUE_WSTA(base_addr) & 0x1))
        {
            u4TimeOut += 1;
            if(u4TimeOut  > 10000)
            {
                MDP_ERROR("Write to OVL descriptor time out 4!!\n");
                return -1;
            }
        }
        u4TimeOut = 0;
    }
#endif

    MT6575_OVL_DMA_REPLACE_COLOR(base_addr) = (((unsigned long)uReplaceColorY << 16) | ((unsigned long)uReplaceColorU << 8) | uReplaceColorV);

    MT6575_OVL_DMA_HEADER_BASE(base_addr) = u4HeaderBaseAddr[0];

    MT6575_OVL_DMA_MASK_BASE(base_addr) = u4MaskBaseAddr[0];

    MT6575_OVL_DMA_SRC_SIZE(base_addr) = (u2SrcWidthInPixel | ((unsigned long)u2SrcHeightInLine << 16));

    MT6575_OVL_DMA_OFFSET(base_addr) = (u2MaskOffsetX[0] | ((unsigned long)u2MaskOffsetY[0] << 16));

    MT6575_OVL_DMA_MASK_SRC_SIZE(base_addr) = (u2MaskSrcWidthInPixel[0] | ((unsigned long)u2MaskSrcHeightInLine[0] << 16));

    MT6575_OVL_DMA_MASK_DST_SIZE(base_addr) = (u2MaskDestWidthInPixel[0] | ((unsigned long)u2MaskDestHeightInLine[0] << 16));

    MT6575_OVL_DMA_MASK_CFG(base_addr) = ((u2MaskSrcHeightInLine[0] - (u2MaskDestHeightInLine[0]%u2MaskSrcHeightInLine[0]))%u2MaskSrcHeightInLine[0]) |
        ((unsigned long)uColorKey << 16);

    MT6575_OVL_DMA_CON(base_addr) = (0x3 | (bIsCompressMask << 2) | (0x4 << 4) | (bCamIn ? (0x3 << 7) : 0));


    return 0;
}

int OVL::_Enable( void )
{
    unsigned long base_addr;
    unsigned long mmsys1_reg_base;

    base_addr = this->reg_base_addr(); //virtual
    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();


    if( base_addr == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    //Enable MOUT
    MT6575_MMSYS_OVL_DMA_MIMO_EN(mmsys1_reg_base) = 1;
    //Enable OVL DMA
    MT6575_OVL_DMA_STOP(base_addr) = 0;

    if(!(MT6575_OVL_DMA_IN_SEL(base_addr) & (1 << 15)))
    {
        MT6575_OVL_DMA_EN(base_addr) = 1;
    }


    return 0;
}

int OVL::_Disable( void )
{
    unsigned long base_addr;
    unsigned long mmsys1_reg_base;

    base_addr = this->reg_base_addr(); //virtual
    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();


    if( base_addr == 0 )
    {
        return -1;
    }

    MT6575_OVL_DMA_QUEUE_RSTA(base_addr) = 0;

    MT6575_OVL_DMA_STOP(base_addr) = 1;

    MT6575_OVL_DMA_STOP(base_addr) = 0;

    //Disable MOUT
    MT6575_MMSYS_OVL_DMA_MIMO_EN(mmsys1_reg_base) = 0;

    Reset();

    return 0;
}

int OVL::CheckBusy( unsigned long* param )
{
    unsigned long base_addr;
    unsigned long u4Result = 0;

    base_addr = this->reg_base_addr(); //virtual

    if( base_addr == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    //Check error status as well
    u4Result = MT6575_OVL_DMA_IRQ_FLAG(base_addr);

    MT6575_OVL_DMA_IRQ_FLAG_CLR(base_addr) = ( 0x33 & u4Result);

    if(u4Result & 0x1){/*MDP_INFO("engine finished the decriptor\n");*/}

    if(u4Result & 0x2){MDP_ERROR("SW configuration error. Invalid descriptor mode\n");}

    if(u4Result & 0x10){MDP_ERROR("SW configuration error. Erroneous mask source size and destination size set\n");}

    if(u4Result & 0x20){MDP_ERROR("SW configuration error. SW writing to descriptor queue when command queue writer is busy\n");}

    return MT6575_OVL_DMA_EN(base_addr);
}

int OVL::Reset( void )
{
    unsigned long time_out = 0xFFFFFF;
    unsigned long base_addr;
    unsigned long mmsys1_reg_base;

    base_addr = this->reg_base_addr(); //virtual
    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();

    if( base_addr == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    //Reset OVL and MOUT before bypass
    MT6575_MMSYS_OVL_DMA_MIMO_CLR(mmsys1_reg_base) = 1;
    MT6575_MMSYS_OVL_DMA_MIMO_CLR(mmsys1_reg_base) = 0;
    MT6575_OVL_DMA_RESET(base_addr) = 0x2;

    while(0x2 == MT6575_OVL_DMA_RESET(base_addr))
    {
        time_out--;

        if( time_out == 0 ){
            MDP_ERROR("Waiting for OVL reset done\n");
            break;
        }
    }

    MT6575_OVL_DMA_IRQ_FLAG(base_addr) = (0x33 << 16);
    MT6575_OVL_DMA_IRQ_FLAG_CLR(base_addr) = 0x33;

    return 0;
}

int OVL::HardReset( void )
{
    unsigned long base_addr;
    unsigned long mmsys1_reg_base;
    volatile unsigned long time_delay;

    base_addr = this->reg_base_addr(); //virtual
    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();

    if( base_addr == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    //Reset OVL and MOUT before bypass
    MT6575_MMSYS_OVL_DMA_MIMO_CLR(mmsys1_reg_base) = 1;
    MT6575_MMSYS_OVL_DMA_MIMO_CLR(mmsys1_reg_base) = 0;

        /*Hard Reset*/
    MT6575_OVL_DMA_RESET( base_addr ) = 0x1;
    for( time_delay = 0 ; time_delay < 50 ; time_delay += 1){;}//Delay 50 cycle
    MT6575_OVL_DMA_RESET( base_addr ) = 0x0;

    MDP_WARNING("<%s> Hard Reset!\n",this->name_str());

    return 0;
}


int OVL::DumpRegisterCustom( int mode )
{


    return 0;
}


/*/////////////////////////////////////////////////////////////////////////////
    IPP
  /////////////////////////////////////////////////////////////////////////////*/
int IPP::_Config( void )
{
    unsigned long u4Index = 0;
    unsigned long base_addr;
    unsigned long mmsys1_reg_base;

    base_addr = this->reg_base_addr(); //virtual
    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();


    if( base_addr == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    #if 1
    Reset();
    #else
    MT6575_MMSYS_IPP_MOUT_CLR(base_addr) = 1;
    MT6575_MMSYS_IPP_MOUT_CLR(base_addr) = 0;
    #endif

    MT6575_IMGPROC_EN(base_addr) = 0x0000;
    MT6575_IMGPROC_EN(base_addr) = (0x1000 | (bCamIn << 13));

    MT6575_IMGPROC_IPP_CFG(base_addr) = (((bEnContractBrightAdj|bEnY2R|bEnR2Y) << 4) | (bEnHueAdj << 5) | ((bEnSatAdj|bEnY2R|bEnR2Y) << 8) |
        (bEnColorize << 9) | ((bEnRGBReplace | bEnColorAdj | bEnColorInverse) ? (0x3 << 16) : 0) |
        (bEnColorAdj << 20) | (bEnColorInverse << 28) | (bEnRGBReplace << 30));

    MT6575_IMGPROC_R2Y_CFG(base_addr) = ((bEnRGBReplace | bEnColorAdj | bEnColorInverse) ? 0x3 : 0);

    if(bEnHueAdj)
    {
        MT6575_IMGPROC_HUE11(base_addr) = uHueRotMatrix[0];
        MT6575_IMGPROC_HUE12(base_addr) = uHueRotMatrix[1];
        MT6575_IMGPROC_HUE21(base_addr) = uHueRotMatrix[2];
        MT6575_IMGPROC_HUE22(base_addr) = uHueRotMatrix[3];
    }

    if(uSatAdj)
    {
        MT6575_IMGPROC_SAT(base_addr) = uSatAdj;
    }

    if(bEnContractBrightAdj)
    {
        MT6575_IMGPROC_BRIADJ1(base_addr) = uBriXAdj;
        MT6575_IMGPROC_BRIADJ2(base_addr) = uBriYAdj;
        MT6575_IMGPROC_CONADJ(base_addr) = uContrastAdj;
    }

    if(bEnColorize)
    {
        MT6575_IMGPROC_COLORIZEU(base_addr) = uColorizeU;
        MT6575_IMGPROC_COLORIZEV(base_addr) = uColorizeV;
    }

    if(bEnColorAdj)
    {
        if(NULL == puColorAdjCoeff)
        {
            MDP_ERROR("Config IPP failed -- color adj is null but enable color adj");
            return -1;
        }

        for(u4Index = 0 ; u4Index < 21 ; u4Index += 1)
        {
            MT6575_IMGPROC_COLOR1R_OFFX(base_addr + (u4Index << 2)) = puColorAdjCoeff->uColorAdj[u4Index];
        }
    }

    if(bEnRGBReplace)
    {
        MT6575_IMGPROC_IPP_RGB_DETECT(base_addr) = (((unsigned long)uRGBDetectR << 16) | ((unsigned long)uRGBDetectG << 8) | uRGBDetectB);
        MT6575_IMGPROC_IPP_RGB_REPLACE(base_addr) = (((unsigned long)uRGBReplaceR << 16) | ((unsigned long)uRGBReplaceG << 8) | uRGBReplaceB);
    }

    // patch for google cts
    if (bEnY2R)
    {
        //Y
        MT6575_IMGPROC_BRIADJ2(base_addr) = 16;
        MT6575_IMGPROC_CONADJ(base_addr)  = 37;
        MT6575_IMGPROC_BRIADJ1(base_addr) = 0;
        //UV
        MT6575_IMGPROC_SAT(base_addr)     = 36;
    }

    if (bEnR2Y)
    {
        //Y
        MT6575_IMGPROC_BRIADJ2(base_addr) = 0;
        MT6575_IMGPROC_CONADJ(base_addr)  = 28;
        MT6575_IMGPROC_BRIADJ1(base_addr) = 16;
        //UV
        MT6575_IMGPROC_SAT(base_addr)     = 28;
    }

    //Config MOUT
    MT6575_MMSYS_IPP_ISEL(mmsys1_reg_base) = src_sel;
    MT6575_MMSYS_IPP_MOUT_SEL(mmsys1_reg_base) = (to_jpg_dma | (to_vdo_rot0 << 1) | (to_prz0 << 2) | (to_vrz0 << 3) |(to_rgb_rot0<< 4));
    MT6575_MMSYS_IPP_MOUT_CON(mmsys1_reg_base) = bCamIn;

    return 0;
}

int IPP::_Enable( void )
{
    unsigned long base_addr;
    unsigned long mmsys1_reg_base;

    base_addr = this->reg_base_addr(); //virtual
    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();


    if( base_addr == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    //Enable MOUT
    MT6575_MMSYS_IPP_MOUT_EN( mmsys1_reg_base ) = 0x1;

    //Enable IPP
    MT6575_IMGPROC_EN( base_addr ) |= (1 == bBypass ? 0x1000 : 0x1001);

    return 0;
}

int IPP::_Disable( void )
{
    unsigned long base_addr;
    unsigned long mmsys1_reg_base;

    base_addr = this->reg_base_addr(); //virtual
    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();


    if( base_addr == 0 )
    {
        return -1;
    }

    MT6575_IMGPROC_EN( base_addr ) &= (~0x1);
    MT6575_MMSYS_IPP_MOUT_EN( mmsys1_reg_base ) = 0;

    Reset();


    return 0;
}

int IPP::CheckBusy( unsigned long* param )
{
    /*No busy check with IPP*/

    return 0;

    /*
    unsigned long base_addr;

    base_addr = this->reg_base_addr(); //virtual

    return ((MT6575_IMGPROC_EN(base_addr) & 0x1) > 0)?1:0;
    */
}

int IPP::Reset(void)
{
    unsigned long base_addr;

    base_addr = this->reg_base_addr(); //virtual

    if( base_addr == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    MT6575_MMSYS_IPP_MOUT_CLR(base_addr) = 1;
    MT6575_MMSYS_IPP_MOUT_CLR(base_addr) = 0;

    return 0;

}

int IPP::HardReset(void)
{
    return Reset();//Warm Reset Only
}


int IPP::DumpRegisterCustom( int mode )
{


    return 0;
}



/*/////////////////////////////////////////////////////////////////////////////
    RESZ_I
  /////////////////////////////////////////////////////////////////////////////*/

int RESZ_I::_Config( void )
{
    unsigned long u4WMin , u4LB;
    unsigned long a_pRegBase;


    a_pRegBase = this->reg_base_addr(); //virtual


    if( a_pRegBase== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    //Reset before bypass
    #if 1
    Reset();
    #else
    MT6575_RZ_CON(a_pRegBase) = (0x1 << 16);
    MT6575_RZ_CON(a_pRegBase) = 0;
    #endif

    if(1 == bBypass)
    {
        MT6575_RZ_CFG(a_pRegBase) = (1 << 15) + (bContinuous << 8) + (1 << 9) + (1 << 10) + ((bCamIn ? 1 : 0) << 31);
        //MDP_WARNING("%s Bypass!\n",this->name_str() );

        if( (src_img_size.w != dst_img_size.w) || (src_img_size.h != dst_img_size.h) )
        {
            MDP_ERROR("%s is bypass but use its resize function.(%lu,%lu)->(%lu,%lu)\n",this->name_str(), src_img_size.w, src_img_size.h, dst_img_size.w, dst_img_size.h );
            return -1;
        }

        return 0;
    }

    if((MID_PRZ0 == this->id() || MID_PRZ1 == this->id()) && ((0 < uEEHCoeff) || (0 < uEEVCoeff)) && ((src_img_roi.w < dst_img_size.w) || (src_img_roi.h < dst_img_size.h)))
    {
        MDP_ERROR("EE cannot be applied in scale up case srcROI.w:%lu,srcROI.h:%lu,dst.w:%lu,dst.h:%lu \n", src_img_roi.w , src_img_roi.h , dst_img_size.w, dst_img_size.h );
    }

    u4WMin = src_img_roi.w > dst_img_size.w ? dst_img_size.w : src_img_roi.w;
    u4WMin = (u4WMin + 1) & (~0x1);

    //Rule1. source/destination width/height limitation
    if((src_img_size.w > 8191) || (src_img_size.h > 8191) || (src_img_size.w < 3) || (src_img_size.h < 3) ||
        (dst_img_size.w > 8191) || (dst_img_size.h > 8191) || (dst_img_size.w < 3) || (dst_img_size.h < 3) ||
        (src_img_roi.w > 8191) || (src_img_roi.h > 8191) || (src_img_roi.w < 3) || (src_img_roi.h < 3))
    {
        MDP_ERROR("<%s> Config resizer failed -- dimension exceeds limit, srcw:%lu,srch:%lu,destw:%lu,desth:%lu,cropw:%lu,croph:%lu",
            this->name_str(),
            src_img_size.w , src_img_size.h , dst_img_size.w , dst_img_size.h , src_img_roi.w , src_img_roi.h);
        return -1;
    }

    if((MID_CRZ == this->id()) && (u4WMin > RESZ_CRZ_MAX_MINWIDTH))//virtual
    {
        MDP_ERROR("<%s> Config resizer failed -- CRZ dimension exceeds limit, srcw:%lu,srch:%lu,destw:%lu,desth:%lu,cropw:%lu,croph:%lu",
            this->name_str(),
            src_img_size.w , src_img_size.h , dst_img_size.w , dst_img_size.h , src_img_roi.w , src_img_roi.h);
        return -1;
    }

    if((((unsigned long)src_img_roi.w << 6) < dst_img_size.w) || (src_img_roi.w > ((unsigned long)dst_img_size.w << 11)))
    {
        MDP_ERROR("<%s> Config resizer failed -- Rescale ratio exceeds limit, srcw:%lu,srch:%lu,destw:%lu,desth:%lu,cropw:%lu,croph:%lu",
            this->name_str(),
            src_img_size.w , src_img_size.h , dst_img_size.w , dst_img_size.h , src_img_roi.w , src_img_roi.h);
        return -1;
    }

    if((MID_PRZ0 == this->id()) && (u4WMin > RESZ_PRZ0_MAX_MINWIDTH))//virtual
    {
       MDP_ERROR("<%s> Config resizer failed -- PRZ0 dimension exceeds limit, srcw:%lu,srch:%lu,destw:%lu,desth:%lu,cropw:%lu,croph:%lu",
            this->name_str(),
            src_img_size.w , src_img_size.h , dst_img_size.w , dst_img_size.h , src_img_roi.w , src_img_roi.h);
       return -1;
    }

    if((MID_PRZ1 == this->id()) && (u4WMin > RESZ_PRZ1_MAX_MINWIDTH))//virtual
    {
       MDP_ERROR("<%s> Config resizer failed -- PRZ1 dimension exceeds limit, srcw:%lu,srch:%lu,destw:%lu,desth:%lu,cropw:%lu,croph:%lu",
            this->name_str(),
            src_img_size.w , src_img_size.h , dst_img_size.w , dst_img_size.h , src_img_roi.w , src_img_roi.h);
       return -1;
    }

    //Rule2. Crop window should not exceeds source window
    if(((src_img_roi.x + src_img_roi.w) > src_img_size.w) ||
    	((src_img_roi.y + src_img_roi.h) > src_img_size.h))
    {
        MDP_ERROR("<%s> Config resizer failed -- dimension exceeds limit, srcw:%lu,srch:%lu,cropx:%lu,cropy:%lu,cropw:%lu,croph:%lu",
            this->name_str(),src_img_size.w , src_img_size.h , src_img_roi.x , src_img_roi.y , src_img_roi.w , src_img_roi.h);
        return -1;
    }
    /*
    //Reset
    MT6575_RZ_CON(a_pRegBase) = (0x1 << 16);
    MT6575_RZ_CON(a_pRegBase) = 0;
    */

    MT6575_RZ_CFG(a_pRegBase) = (1 << 6) + (bContinuous << 8) + (1 << 9) + (0x3 << 10) + ((bCamIn ? 1 : 0) << 31);

    /*Calculate LBMAX*/
    u4LB = CalcLBMAX( MdpSize( src_img_roi.w, src_img_roi.h ), MdpSize( dst_img_size.w, dst_img_size.h ) );

    if(MID_CRZ == this->id()) //virtual
    {
        //u4LB = (3328/u4WMin)*6;
        //u4LB = u4LB > 1023 ? 1023 : u4LB;
        MT6575_RZ_CFG(a_pRegBase) |= ((bCamIn ? 0 : 1) + (u4LB << 16) + (0x3 << 26) + (0x3 << 29));

        //test
        //MT6575_RZ_CFG(a_pRegBase) |= (1 + (u4LB << 16) + (0x3 << 26) + (0x3 << 29));
    }
    else
    {
        //u4LB = (864/u4WMin)*6;
        //u4LB = u4LB > 1023 ? 1023 : u4LB;
        MT6575_RZ_CFG(a_pRegBase) |= (u4LB << 16);
        if((0 < uEEHCoeff) || (0 < uEEVCoeff))
        {
            MT6575_RZ_CFG(a_pRegBase) |= (3 << 12);
        }
    }

    MT6575_RZ_ORIGSZ(a_pRegBase) = (src_img_size.w | ((unsigned long)src_img_size.h << 16));
    MT6575_RZ_SRCSZ(a_pRegBase) = (src_img_roi.w | ((unsigned long)src_img_roi.h << 16));
    MT6575_RZ_TARSZ(a_pRegBase) = (dst_img_size.w | ((unsigned long)dst_img_size.h << 16));

    MT6575_RZ_CROPLR(a_pRegBase) = (((unsigned long)src_img_roi.x << 16) | (src_img_roi.w + src_img_roi.x - 1));
    if((src_img_size.w > src_img_roi.w) || (src_img_size.h > src_img_roi.h))
    {
        MT6575_RZ_CROPLR(a_pRegBase) |= (1 << 31);
    }
    MT6575_RZ_CROPTB(a_pRegBase) = (((unsigned long)src_img_roi.y << 16) | (src_img_roi.h + src_img_roi.y - 1));

    MT6575_RZ_HRATIO(a_pRegBase) = (src_img_roi.w > dst_img_size.w) ?
        (((dst_img_size.w -1) << 20) + ((src_img_roi.w -1) >> 1))/(src_img_roi.w -1) :
        (((src_img_roi.w -1) << 20) + ((dst_img_size.w -1) >> 1))/(dst_img_size.w -1);
    MT6575_RZ_VRATIO(a_pRegBase) = (src_img_roi.h > dst_img_size.h) ?
        (((dst_img_size.h -1) << 20) + ((src_img_roi.h -1) >> 1))/(src_img_roi.h -1) :
        (((src_img_roi.h -1) << 20) + ((dst_img_size.h -1) >> 1))/(dst_img_size.h -1);

    MT6575_RZ_COEFF(a_pRegBase) = (((unsigned long)uUpScaleCoeff << 8) | uDnScaleCoeff);

    if((MID_PRZ0 == this->id()) || (MID_PRZ1 == this->id()))
    {
        MT6575_RZ_COEFF(a_pRegBase) |= (((unsigned long)uEEHCoeff << 20) | ((unsigned long)uEEVCoeff << 16));
    }



    if( MID_CRZ == this->id() )
    {
        /*Config CRZ ultra-high threshold*/
        if( bCamIn )
        {
            /*
                0x680 is the maxmum fifo downscale counter statics by the register 0xC0[31:16],
                ( upscale counter is in 0xC0[15:0] ).
                if BUSY_CNT_EN @ MT6575_RZ_BUSYU[30] is enable, 0xC4 will count the busy cycle
             */
            #if 0
            unsigned long BUSYD_HTH = ( 0x680*2/3 ) & 0x3FF;
            unsigned long BUSYD_LTH = ( 0x680*1/3 ) & 0x3FF;
            #else
            unsigned long BUSYD_HTH = ( 0x298*2/3 ) & 0x3FF;//( 0x180*2/3 ) & 0x3FF;//( 0x298*2/3 ) & 0x3FF;
            unsigned long BUSYD_LTH = ( 0x298*1/3 ) & 0x3FF;//( 0x180*1/3 ) & 0x3FF;//( 0x298*1/3 ) & 0x3FF;
            #endif
            const unsigned long BUSYU_EN = 0;   //Up scale ultra high
            const unsigned long BUSYD_EN = 0;   //Down scale ultra high
            const unsigned long BUSY_CNT_EN = 1;
            const unsigned long BUSY_ALL = 1;   //Enable BUSY_ALL can always pull busy high


            MT6575_RZ_BUSYU( a_pRegBase ) = ( BUSYU_EN << 31 ) | (BUSY_CNT_EN << 30 ) | (BUSY_ALL << 29 );

            MT6575_RZ_BUSYD( a_pRegBase ) = ( BUSYD_EN << 31 ) | (BUSYD_HTH << 16 ) | (BUSYD_LTH << 0 );
        }
        else
        {
            MT6575_RZ_BUSYU( a_pRegBase ) = 0;
            MT6575_RZ_BUSYD( a_pRegBase ) = 0;
        }
    }


    return 0;
}

int RESZ_I::_Enable( void )
{
    unsigned long a_pRegBase;

    a_pRegBase = this->reg_base_addr(); //virtual


    if( a_pRegBase== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    if((1 << 15) & MT6575_RZ_CFG(a_pRegBase)) //CLS Bypass enable
    {
        return 0;
    }
    MT6575_RZ_CON(a_pRegBase) = 0x1;

    return 0;
}

int RESZ_I::_Disable( void )
{
    unsigned long a_pRegBase;

    a_pRegBase = this->reg_base_addr(); //virtual


    if( a_pRegBase== 0 )
    {
        return -1;
    }


    //Disable RZ
    MT6575_RZ_CON(a_pRegBase) = 0;

    Reset();

    return 0;
}

int RESZ_I::CheckBusy( unsigned long* param  )
{
    unsigned long a_pRegBase;

    if( bContinuous )
        return 0;   //CRZ in continous mode will always busy, ignore busy check

    a_pRegBase = this->reg_base_addr(); //virtual

    if( a_pRegBase== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    switch( this->id() )
    {
        case MID_CRZ:
            return ((MT6575_RZ_STA(a_pRegBase) & 0x00000007) > 0)?1:0;
        break;
        case MID_PRZ1:
            return ((MT6575_RZ_STA(a_pRegBase) & 0x00000003) > 0)?1:0;
        break;
        case MID_PRZ0:
            return ((MT6575_RZ_STA(a_pRegBase) & 0x00000003) > 0)?1:0;
        break;
    }

    return 0;
}

int RESZ_I::Reset( void )
{
    unsigned long a_pRegBase;

    a_pRegBase = this->reg_base_addr(); //virtual


    if( a_pRegBase== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    MT6575_RZ_CON(a_pRegBase) = (0x1 << 16);
    MT6575_RZ_CON(a_pRegBase) = 0;

    return 0;
}

int RESZ_I::HardReset( void )
{
    return Reset(); //Warm Reset Only
}




int RESZ_I::DumpRegisterCustom( int mode )
{

    unsigned long reg_base;
    unsigned long reg_val;

    reg_base = this->reg_base_addr();


    if( reg_base== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    //MT6575_RZ_CFG
    reg_val = MT6575_RZ_CFG(reg_base);
    MDP_PRINTF("BYPASS=%u\n",(unsigned int)reg_val&(0x1<<15)?1:0);
    MDP_PRINTF("PCON(Cont. mode)=%u\n",(unsigned int)reg_val&(0x1<<8)?1:0);
    MDP_PRINTF("NOVS_MODE(Vsync)=%u\n",(unsigned int)reg_val&(0x1));

    //MT6575_RZ_STA
    reg_val = MT6575_RZ_STA(reg_base);
    MDP_PRINTF("BUSYO=%d BUSYI=%d BUSYC=%d\n", reg_val&(0x1<<0)?1:0 ,  reg_val&(0x1<<1)?1:0 , reg_val&(0x1<<2)?1:0);
    MDP_PRINTF("EIS_ERR(Output to EIS error)=%d\n", reg_val&(0x1<<3)?1:0 );
    MDP_PRINTF("ERR0(Input pixel not eneough)=%d\n", reg_val&(0x1<<4)?1:0 );
    MDP_PRINTF("ERR1(Output too slow)=%d\n", reg_val&(0x1<<5)?1:0 );
    MDP_PRINTF("ERR2(Init at output)=%d\n", reg_val&(0x1<<6)?1:0 );
    MDP_PRINTF("ERR3(Init at input)=%d (sensor too fast?frame sync?)\n", reg_val&(0x1<<7)?1:0 );

    //MT6575_RZ_INFO0
    reg_val = MT6575_RZ_INFO0(reg_base);
    MDP_PRINTF("INIF_XCNT=%d INIF_YCNT=%d\n", (int)reg_val&0x1FFF,(int)(reg_val>>16)&0x1FFF );

    //MT6575_RZ_INFO1
    reg_val = MT6575_RZ_INFO1(reg_base);
    MDP_PRINTF("OUT_XCNT=%d OUT_YCNT=%d\n", (int)reg_val&0x1FFF,(int)(reg_val>>16)&0x1FFF );


    //MT6575_RZ_SRCSZ
    reg_val = MT6575_RZ_SRCSZ(reg_base);
    MDP_PRINTF("SRC_W(or CROP_W)=%d\n",(int)(reg_val&0x0000FFFF));
    MDP_PRINTF("SRC_H(or CROP_H)=%d\n",(int)((reg_val&0xFFFF0000)>>16));


    //MT6575_RZ_TARSZ
    reg_val = MT6575_RZ_TARSZ(reg_base);
    MDP_PRINTF("DST_W=%d\n",(int)(reg_val&0x0000FFFF));
    MDP_PRINTF("DST_H=%d\n",(int)((reg_val&0xFFFF0000)>>16));


    //MT6575_RZ_ORIGSZ
    reg_val = MT6575_RZ_ORIGSZ(reg_base);
    MDP_PRINTF("ORG_W=%d\n",(int)(reg_val&0x0000FFFF));
    MDP_PRINTF("ORG_H=%d\n",(int)((reg_val&0xFFFF0000)>>16));


    //MT6575_RZ_CROPLR
    reg_val = MT6575_RZ_CROPLR(reg_base);
    MDP_PRINTF("CROP_EN=%d\n",(int)((reg_val&0x80000000)>>31));
    MDP_PRINTF("CROP_R=%d\n",(int)(reg_val&0x0000FFFF));
    MDP_PRINTF("CROP_L=%d\n",(int)((reg_val&0x1FFF0000)>>16));


    //MT6575_RZ_CROPTB
    reg_val = MT6575_RZ_CROPTB(reg_base);
    MDP_PRINTF("CROP_B=%d\n",(int)(reg_val&0x0000FFFF));
    MDP_PRINTF("CROP_T=%d\n",(int)((reg_val&0x1FFF0000)>>16));


    return 0;
}

int RESZ_I::DumpDebugInfo( void )
{
    MDPELEMENT_I::DumpDebugInfo();


    MDP_DUMP_VAR_D(src_img_size.w);
    MDP_DUMP_VAR_D(src_img_size.h);

    MDP_DUMP_VAR_D(src_img_roi.x);
    MDP_DUMP_VAR_D(src_img_roi.y);
    MDP_DUMP_VAR_D(src_img_roi.w);
    MDP_DUMP_VAR_D(src_img_roi.h);

    MDP_DUMP_VAR_D(dst_img_size.w);
    MDP_DUMP_VAR_D(dst_img_size.h);

    MDP_DUMP_VAR_H(uUpScaleCoeff);
    MDP_DUMP_VAR_H(uDnScaleCoeff);

    MDP_DUMP_VAR_H(uEEHCoeff);
    MDP_DUMP_VAR_H(uEEVCoeff);

    MDP_DUMP_VAR_D(bContinuous);
    MDP_DUMP_VAR_D(bCamIn);
    MDP_DUMP_VAR_D(bBypass);

    return 0;
}




int RESZ_I::ConfigZoom( MdpRect crop_size , unsigned long linked_mdp_id_set )
{

    unsigned long reg_base;
    unsigned long reg_val;

    unsigned long TarW = 0;
    unsigned long TarH = 0;
    unsigned long MinW = 0;
    unsigned long LBMAX = 0;

    MdpDrvConfigZoom_Param ZoomCfg;

    reg_base = this->reg_base_addr();

    if( reg_base == 0 ){
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    MDP_INFO("ZOOM = (%ld, %ld, %lu, %lu )\n", crop_size.x, crop_size.y, crop_size.w, crop_size.h );


    TarW = (MT6575_RZ_TARSZ(reg_base) & 0x1FFF);
    TarH = ((MT6575_RZ_TARSZ(reg_base) & 0x1FFF0000) >> 16);

    #if 0
    MinW = crop_size.w > TarW ? TarW : crop_size.w;
    MinW = (MinW + 1) & (~0x1);

    LBMAX = (4096/MinW)*6;//
    LBMAX = LBMAX > 1023 ? 1023 : LBMAX;
    #endif

    LBMAX = CalcLBMAX( MdpSize( crop_size.w, crop_size.h ), MdpSize( TarW, TarH ) );


    /*If CRZ is running*/
    if( 0x1 & MT6575_RZ_CON(reg_base) )
    {
        //Run time
        //        MDPLOG("Set zoom in run time");
        ZoomCfg.mdp_id = this->id();

        ZoomCfg.linked_mdp_id_set = linked_mdp_id_set;

        ZoomCfg.reg_CFG = MT6575_RZ_CFG(reg_base);
        ZoomCfg.reg_CFG &= (~(0x3FF << 16));    //CLS:Clear LBMAX
        ZoomCfg.reg_CFG |= (LBMAX << 16);       //CLS:Re-setting LBMAX

        ZoomCfg.reg_SRCSZ = (crop_size.w | (crop_size.h << 16)); //CLS:SRC = crop size

        ZoomCfg.reg_CROPLR = (0x1 << 31 | (crop_size.x << 16) | (crop_size.x + crop_size.w - 1));

        ZoomCfg.reg_CROPTB = ((crop_size.y << 16) | (crop_size.y + crop_size.h - 1));

        ZoomCfg.reg_HRATIO = crop_size.w > TarW ?
                ((((TarW - 1) << 20) + ((crop_size.w - 1) >> 1))/(crop_size.w - 1)) :
                ((((crop_size.w - 1) << 20) + ((TarW - 1) >> 1))/(TarW - 1));

        ZoomCfg.reg_VRATIO = crop_size.h > TarH ?
                ((((TarH - 1) << 20) + ((crop_size.h - 1) >> 1))/(crop_size.h - 1)) :
                ((((crop_size.h - 1) << 20) + ((TarH - 1) >> 1))/(TarH - 1));

        if( MdpDrv_ConfigZoom( &ZoomCfg ) < 0 )
        {
            return -1;
        }
    }
    else
    {
        //        MDPLOG("Set zoom in initial");
        //Initial
        MT6575_RZ_LOCK(reg_base) = 1;

        MT6575_RZ_CFG(reg_base) &= (~(0x3FF << 16));
        MT6575_RZ_CFG(reg_base) |= (LBMAX << 16);

        MT6575_RZ_SRCSZ(reg_base) = (crop_size.w | (crop_size.h << 16));

        MT6575_RZ_CROPLR(reg_base) = (0x1 << 31 | (crop_size.x << 16) | (crop_size.x + crop_size.w - 1));

        MT6575_RZ_CROPTB(reg_base) = ((crop_size.y << 16) | (crop_size.y + crop_size.h - 1));

        MT6575_RZ_HRATIO(reg_base) = crop_size.w > TarW ?
                            ((((TarW - 1) << 20) + ((crop_size.w - 1) >> 1))/(crop_size.w - 1)) :
                            ((((crop_size.w - 1) << 20) + ((TarW - 1) >> 1))/(TarW - 1));

        MT6575_RZ_VRATIO(reg_base) = crop_size.h > TarH ?
                            ((((TarH - 1) << 20) + ((crop_size.h - 1) >> 1))/(crop_size.h - 1)) :
                            ((((crop_size.h - 1) << 20) + ((TarH - 1) >> 1))/(TarH - 1));

        MT6575_RZ_LOCK(reg_base) = 0;
    }



    return 0;

}


int RESZ_I::ConfigZsdZoom_RegWrite( ReszZsdZoomParam param )
{

    unsigned long reg_base;
    unsigned long reg_val;

    if( is_bypass() )   return 0;

    reg_base = this->reg_base_addr();

    if( reg_base == 0 ){
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    MT6575_RZ_LOCK(reg_base) = 1;

    MT6575_RZ_CFG(reg_base) &= (~(0x3FF << 16));
    MT6575_RZ_CFG(reg_base) |= (param.LBMAX << 16);

    if( param.reg_ORIGSZ != 0xFFFFFFFF )
    {
        MT6575_RZ_ORIGSZ(reg_base)   = param.reg_ORIGSZ;
    }

    MT6575_RZ_SRCSZ(reg_base)   = param.reg_SRCSZ;
    MT6575_RZ_TARSZ(reg_base)   = param.reg_TARSZ;
    MT6575_RZ_CROPLR(reg_base)  = param.reg_CROPLR;
    MT6575_RZ_CROPTB(reg_base)  = param.reg_CROPTB;
    MT6575_RZ_HRATIO(reg_base)  = param.reg_HRATIO;
    MT6575_RZ_VRATIO(reg_base)  = param.reg_VRATIO;

    MT6575_RZ_LOCK(reg_base) = 0;

    return 0;



}



unsigned long RESZ_I::CalcLBMAX( MdpSize src_size, MdpSize dst_size )
{
    unsigned long LBMAX;
    unsigned long min_width;


    min_width = src_size.w > dst_size.w ? dst_size.w : src_size.w;
    min_width = (min_width + 1) & (~0x1);

    switch( this->id() ) //virtual
    {
    case MID_CRZ:
        LBMAX = (RESZ_CRZ_MAX_MINWIDTH/min_width)*6;
        LBMAX = LBMAX > 1023 ? 1023 : LBMAX;
        break;

    case MID_PRZ0:
        LBMAX = (RESZ_PRZ0_MAX_MINWIDTH/min_width)*6;
        LBMAX = LBMAX > 1023 ? 1023 : LBMAX;
        break;

    case MID_PRZ1:
        LBMAX = (RESZ_PRZ1_MAX_MINWIDTH/min_width)*6;
        LBMAX = LBMAX > 1023 ? 1023 : LBMAX;
        break;

    default:
        MDP_ERROR("Mismatch mdp id\n");
        return (unsigned long)-1;
        break;

    }

    return LBMAX;


}

unsigned long RESZ_I::CalcHRATIO( unsigned long src_w, unsigned long dst_w )
{
    unsigned long HRATIO;
    HRATIO = src_w > dst_w ?
            ((((dst_w - 1) << 20) + ((src_w - 1) >> 1))/(src_w - 1)) :
            ((((src_w - 1) << 20) + ((dst_w - 1) >> 1))/(dst_w - 1));
    return HRATIO;
}

unsigned long RESZ_I::CalcVRATIO( unsigned long src_h, unsigned long dst_h )
{
    unsigned long VRATIO;
    VRATIO = src_h > dst_h ?
            ((((dst_h - 1) << 20) + ((src_h - 1) >> 1))/(src_h - 1)) :
            ((((src_h - 1) << 20) + ((dst_h - 1) >> 1))/(dst_h - 1));

    return VRATIO;
}



/*/////////////////////////////////////////////////////////////////////////////
    CRZ
  /////////////////////////////////////////////////////////////////////////////*/
int CRZ::_ConfigPre( void )
{
    unsigned long mmsys1_reg_base;

    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();


    if( mmsys1_reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    MT6575_MMSYS_CAM_ISEL(mmsys1_reg_base) = ((0x1 & src_sel) | (0x2 & src_sel ? 0 : (0x1 << 16)));


    return 0;
}

int CRZ::_ConfigPost( void )
{
    return 0;
}

int CRZ::_EnablePre( void )
{

    return 0;
}

int CRZ::_EnablePost( void )
{
    return 0;
}

int CRZ::_DisablePre( void )
{

    unsigned long mmsys1_reg_base;

    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();


    if( mmsys1_reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    MT6575_MMSYS_CAM_ISEL(mmsys1_reg_base) = ( 0 | (0x1 << 16) ); //RDMA0 as default reset setting/bypass CAM

    return 0;
}

int CRZ::_DisablePost( void )
{

    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    PRZ0
  /////////////////////////////////////////////////////////////////////////////*/
int PRZ0::_ConfigPre( void )
{
    unsigned long mmsys1_reg_base;

    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();


    if( mmsys1_reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    /*src_sel0-MOUT, 1-IPP_MOUT,2-CAM(Sensor),3-BRZ_MOUT,6-CAM(RDMA0),10-CAM(BRZ)*/

    MT6575_MMSYS_PRZ0_ISEL(mmsys1_reg_base) = (src_sel&0x3);
    MT6575_MMSYS_PRZ0_MOUT_CLR(mmsys1_reg_base) = 1;
    MT6575_MMSYS_PRZ0_MOUT_CLR(mmsys1_reg_base) = 0;
    MT6575_MMSYS_PRZ0_MOUT_SEL(mmsys1_reg_base) = (to_vdo_rot0 | (to_rgb_rot0 << 1) | (to_vrz0 << 2));
    MT6575_MMSYS_PRZ0_MOUT_CON(mmsys1_reg_base) = bCamIn;

    if( (src_sel&0x3) == 2 )/*src = CAM*/
    {
        int b_BRZ = 0; //BRZ or RDMA0
        int b_BYPASCAM = 0;

        b_BRZ       = ( ((src_sel >> 2 ) & 0x3 ) == 2 )? 1: 0;
        b_BYPASCAM  = ( ((src_sel >> 2 ) & 0x3 ) != 0 )? 1: 0;

        MT6575_MMSYS_CAM_ISEL(mmsys1_reg_base) = ( b_BRZ  | ( b_BYPASCAM << 16) );
    }

    return 0;
}

int PRZ0::_ConfigPost( void )
{
    return 0;
}

int PRZ0::_EnablePre( void )
{
    unsigned long mmsys1_reg_base;

    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();

    //Enable MOUT
    MT6575_MMSYS_PRZ0_MOUT_EN(mmsys1_reg_base) = 1;


    return 0;
}

int PRZ0::_EnablePost( void )
{
    return 0;
}

int PRZ0::_DisablePre( void )
{
    return 0;
}

int PRZ0::_DisablePost( void )
{
    unsigned long mmsys1_reg_base;

    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();

    if( mmsys1_reg_base == 0 ){
        return -1;
    }

    //Disable MOUT
    MT6575_MMSYS_PRZ0_MOUT_EN(mmsys1_reg_base) = 0;

    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    PRZ1
  /////////////////////////////////////////////////////////////////////////////*/
int PRZ1::_ConfigPre( void )
{
    /*Nothing*/
    return 0;
}

int PRZ1::_ConfigPost( void )
{
    /*Nothing*/
    return 0;
}

int PRZ1::_EnablePre( void )
{
    /*Nothing*/
    return 0;
}

int PRZ1::_EnablePost( void )
{
    /*Nothing*/
    return 0;
}

int PRZ1::_DisablePre( void )
{
    /*Nothing*/
    return 0;
}

int PRZ1::_DisablePost( void )
{
    /*Nothing*/
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    VRZ_I
  /////////////////////////////////////////////////////////////////////////////*/
int VRZ_I::_Config( void )
{

    unsigned long uWMSZ = 4; /*VRZ working memory size. The line number for vertical interpolation,minimum value is 4*/
    unsigned long aRegBase;

    aRegBase = this->reg_base_addr(); //virtual


    if( aRegBase== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    /*Reset before bypass*/
    #if 1
    Reset();
    #else
    MT6575_VRZ_CON(aRegBase) = (1 << 16);
    MT6575_VRZ_CON(aRegBase) = 0;
    #endif

    if(1 == bBypass)
    {
        MT6575_VRZ_CFG(aRegBase) = (1 << 15);
        //MDP_WARNING("VRZ Bypass!\n");
        return 0;
    }

    if((2 > src_img_size.w) ||(2 > src_img_size.h) || (2 > dst_img_size.w) ||(2 > dst_img_size.h) ||
        (4095 < src_img_size.w) ||(4095 < src_img_size.h) || (4095 < dst_img_size.w) ||(4095 < dst_img_size.h))
    {
        MDP_ERROR("Config VRZ failed -- dimension exceeds, srcw:%lu, srch:%lu,destw:%lu,desth:%lu",
            src_img_size.w , src_img_size.h , dst_img_size.w , dst_img_size.h);
        return -1;
    }

    /*
    MT6575_VRZ_CON(aRegBase) = (1 << 16);
    MT6575_VRZ_CON(aRegBase) = 0;
    */

    MT6575_VRZ_CFG(aRegBase) = ((bContinuous << 8) | (1 << 10) | (bCamIn << 31));

    MT6575_VRZ_SRCSZ(aRegBase) = ((src_img_size.h << 16) | src_img_size.w);
    MT6575_VRZ_TARSZ(aRegBase) = ((dst_img_size.h << 16) | dst_img_size.w);

    MT6575_VRZ_HRATIO(aRegBase) = (dst_img_size.w < src_img_size.w ?
    	(((unsigned long)dst_img_size.w - 1) << 20)/(src_img_size.w - 1) :
    	(((unsigned long)src_img_size.w) << 20)/dst_img_size.w);

    MT6575_VRZ_VRATIO(aRegBase) = (dst_img_size.h < src_img_size.h ?
    	(((unsigned long)dst_img_size.h - 1) << 20)/(src_img_size.h - 1) :
    	(((unsigned long)src_img_size.h) << 20)/dst_img_size.h);

    MT6575_VRZ_HRES(aRegBase) = (src_img_size.w%dst_img_size.w);
    MT6575_VRZ_VRES(aRegBase) = (src_img_size.h%dst_img_size.h);

    MT6575_VRZ_FRFG(aRegBase) = ((uWMSZ << 16) | (1 << 1));

    /*u4Address = Alloc_Sysram( this->id() , 2*(((dst_img_size.w + 3) & ~0x3) * uWMSZ)); //virtual*/
    sysram_.size = 2*(((dst_img_size.w + 3) & ~0x3) * uWMSZ);
    sysram_.address = MdpSysram_Alloc( this->id(), sysram_.size , MDPSYSRAM_SUBCAT_NORMAL/*0*/ );


    if( 0 == sysram_.address )
    {
        sysram_.size = 0;
        MDP_ERROR("Config VRZ failed -- not enough internal sram");
        DumpDebugInfo();
        return -1;
    }

    MT6575_VRZ_PREMBASE(aRegBase) = sysram_.address;


    return 0;
}

int VRZ_I::_Enable( void )
{
    unsigned long a_pRegBase;

    a_pRegBase = this->reg_base_addr(); //virtual


    if( a_pRegBase== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    if(!(MT6575_VRZ_CFG(a_pRegBase) & (1 << 15)))
    {
        MT6575_VRZ_CON(a_pRegBase) = 0x1;
    }

    return 0;
}

int VRZ_I::_Disable( void )
{
    unsigned long a_pRegBase;

    #if 0
    this->_DisablePre(); //virtual
    #endif

    a_pRegBase = this->reg_base_addr(); //virtual


    if( a_pRegBase== 0 ){
        return -1;
    }



    MT6575_VRZ_CON(a_pRegBase) = 0x0;

    /*Free_Sysram( this->id() ); //virtual*/
    MdpSysram_Free( this->id(), sysram_.address , MDPSYSRAM_SUBCAT_NORMAL/*0*/ );
    sysram_.address = 0;
    sysram_.size = 0;

    Reset();

    #if 0
    this->_DisablePost(); //virtual
    #endif


    return 0;
}

int VRZ_I::CheckBusy( unsigned long* param  )
{
    return 0;
}

int VRZ_I::Reset( void )
{
    unsigned long aRegBase;

    aRegBase = this->reg_base_addr(); //virtual


    if( aRegBase== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    MT6575_VRZ_CON(aRegBase) = (1 << 16);
    MT6575_VRZ_CON(aRegBase) = 0;

    return 0;

}

int VRZ_I::HardReset( void )
{
    return Reset(); //Warm Reset Only
}



int VRZ_I::DumpRegisterCustom( int mode )
{
    unsigned long reg_base;
    unsigned long reg_val;

    reg_base = this->reg_base_addr();


    if( reg_base== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }
    //MT6575_VRZ_CFG
    reg_val = MT6575_VRZ_CFG(reg_base);
    MDP_PRINTF("Bypass=%u\n",(unsigned int)((reg_val>>15)&0x1));

    //MT6575_VRZ_INFO0
    reg_val = MT6575_VRZ_INFO0(reg_base);
    MDP_PRINTF("In Line Count=%u In Pixel Count=%u\n",(unsigned int)((reg_val>>16)&0xFFFF), (unsigned int)(reg_val&0xFFFF) );
    reg_val = MT6575_VRZ_INFO1(reg_base);
    MDP_PRINTF("Out Line Count=%u Out Pixel Count=%u\n",(unsigned int)((reg_val>>16)&0xFFFF), (unsigned int)(reg_val&0xFFFF) );


    return 0;
}

int VRZ_I::ConfigZsdZoom_RegWrite( VrzZsdZoomParam param )
{
    unsigned long reg_base;

    if( is_bypass() )   return 0;

    reg_base = this->reg_base_addr(); //virtual

    if( reg_base== 0 ){
        return -1;
    }

    MT6575_VRZ_SRCSZ(reg_base)  = param.reg_SRCSZ;
    MT6575_VRZ_TARSZ(reg_base)  = param.reg_TARSZ;
    MT6575_VRZ_HRATIO(reg_base) = param.reg_HRATIO;
    MT6575_VRZ_VRATIO(reg_base) = param.reg_VRATIO;
    MT6575_VRZ_HRES(reg_base)   = param.reg_HRES;
    MT6575_VRZ_VRES(reg_base)   = param.reg_VRES;

    return 0;

}

int VRZ_I::DumpDebugInfo( void )
{
    MDPELEMENT_I::DumpDebugInfo();


    MDP_DUMP_VAR_D(src_img_size.w);
    MDP_DUMP_VAR_D(src_img_size.h);

    MDP_DUMP_VAR_D(dst_img_size.w);
    MDP_DUMP_VAR_D(dst_img_size.h);

    MDP_DUMP_VAR_D(bContinuous);
    MDP_DUMP_VAR_D(bCamIn);
    MDP_DUMP_VAR_D(bBypass);

    MDP_DUMP_VAR_H(sysram_.address);
    MDP_DUMP_VAR_D(sysram_.size);


    return 0;
}




/*/////////////////////////////////////////////////////////////////////////////
    VRZ0
  /////////////////////////////////////////////////////////////////////////////*/
int VRZ0::_ConfigPre( void )
{
    unsigned long mmsys1_reg_base;

    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();

    MT6575_MMSYS_VRZ0_ISEL(mmsys1_reg_base) = src_sel;
    return 0;
}

int VRZ0::_ConfigPost( void )
{
    /*Nothing*/
    return 0;
}

int VRZ0::_EnablePre( void )
{
    /*Nothing*/
    return 0;
}

int VRZ0::_EnablePost( void )
{
    /*Nothing*/
    return 0;
}

int VRZ0::_DisablePre( void )
{
    /*Nothing*/
    return 0;
}

int VRZ0::_DisablePost( void )
{
    /*Nothing*/
    return 0;
}



/*/////////////////////////////////////////////////////////////////////////////
    VRZ1
  /////////////////////////////////////////////////////////////////////////////*/
int VRZ1::_ConfigPre( void )
{
    unsigned long mmsys1_reg_base;

    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();

    MT6575_MMSYS_VRZ1_ISEL(mmsys1_reg_base) = src_sel;
    return 0;
}

int VRZ1::_ConfigPost( void )
{
    /*Nothing*/
    return 0;
}

int VRZ1::_EnablePre( void )
{
    /*Nothing*/
    return 0;
}

int VRZ1::_EnablePost( void )
{
    /*Nothing*/
    return 0;
}

int VRZ1::_DisablePre( void )
{
    /*Nothing*/
    return 0;
}

int VRZ1::_DisablePost( void )
{
    /*Nothing*/
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    ROTDMA_I
  /////////////////////////////////////////////////////////////////////////////*/
typedef struct {
    unsigned long u4MainSize;
    unsigned long u4SubSize;
    unsigned long u4LB_S_IN_LINE;//Output : lib buffer size in line
    unsigned long u4Main_blk_w;//Output : main block width
    unsigned long u4Sub_blk_w;//Output : main block width
    unsigned long u4Main_buf_line_size;//Output : main buffer line size
    unsigned long u4Sub_buf_line_size;//Output : main buffer line size
    unsigned long u4BytePerPixel;
} stROTMDALineBuffSetting;

typedef struct {
    unsigned long clip_w; //Input : clip width
    MdpColorFormat eOutputColorFMT; //Input : color format
    unsigned long uvv;
    stROTMDALineBuffSetting * pLineBuffSetting;
} stLineBuffCalculation;

typedef struct
{
    unsigned long u4Alignment;// ex :32 means 32 byte aligned
    unsigned long u4MinBuffCnt;// minimum required buffer count
    unsigned long u4SingleBuffSize; // single buffer size
    unsigned long u4RecommandBuffCnt;// Recommand buffer count
} SYSRAM_INFO;

static void CheckAndAllocateLineBuffer(stLineBuffCalculation * a_stCfg , SYSRAM_INFO * a_pstSysram)
{
    unsigned long main_buf_line_size, main_blk_w , sub_buf_line_size , sub_blk_w;
    unsigned long clip_w_residual , adjusted_clip_w , Mainsize , SubSize , clip_w_real , YUVoutput;
    unsigned long main_lb_s_in_line , sub_lb_s_in_line;

    //Decide initial condition, and find the lower limitation
    switch(a_stCfg->eOutputColorFMT)
    {
        case RGB888 :
        case BGR888 :
            main_lb_s_in_line = 16;//8~24
            YUVoutput = 0;
        break;

        case RGB565 :
        case BGR565 :
            main_lb_s_in_line = 16;// 4~32
            YUVoutput = 0;
        break;

				case Y411_Pack ://YUV410, 4x4 sub sample U/V plane
        case UYVY_Pack ://UYVY
        case YUY2_Pack ://YUYV
            main_lb_s_in_line = 16;// 8~32
            YUVoutput = 0; //YUV is only mean planar genic YUV
        break;

        case ARGB8888 :
        case ABGR8888 :
        case RGBA8888 :
        case BGRA8888 :
            main_lb_s_in_line = 16;// 2~16
            YUVoutput = 0;
        break;

        case YV16_Planar ://YUV422, 2x1 subsampled U/V planes
        case YV12_Planar ://YUV420, 2x2 subsampled U/V planes
        case ANDROID_YV12:
        case IMG_YV12:
        case Y800 ://Y plan only
        case NV12 ://YUV420, 2x2 subsampled , interleaved U/V plane
        case NV21 ://YUV420, 2x2 subsampled , interleaved U/V plane
            main_lb_s_in_line = 32;// 8~64
            YUVoutput = 1;
        break;

        default:
            main_lb_s_in_line = 0;  /*meaningless*/
            YUVoutput = 0;          /*meaningless*/
            MDP_ERROR("Unsupport color format:%d\n", (int)a_stCfg->eOutputColorFMT );
        break;
    }

    //Calculate main buffer
    clip_w_residual = a_stCfg->clip_w % main_lb_s_in_line;

    adjusted_clip_w = a_stCfg->clip_w;

    if (clip_w_residual != 0)
    {
        adjusted_clip_w = a_stCfg->clip_w + (main_lb_s_in_line - clip_w_residual);
    }

    main_blk_w = adjusted_clip_w / main_lb_s_in_line;

    //FIFO mode
    main_buf_line_size = main_blk_w * (main_lb_s_in_line + 1);

    Mainsize = main_buf_line_size * (main_lb_s_in_line + 1);

    //Calculate sub buffer
    sub_lb_s_in_line = main_lb_s_in_line;

    if((UYVY_Pack == a_stCfg->eOutputColorFMT) || (YUY2_Pack == a_stCfg->eOutputColorFMT))
    {
        sub_lb_s_in_line = (sub_lb_s_in_line >> 1);
    }

    clip_w_real = a_stCfg->clip_w;
     if(!YUVoutput)
    {
        clip_w_real = (clip_w_real >> 1);
    }

    if((2 == a_stCfg->uvv) && (1 == YUVoutput))
    {
        clip_w_real = (clip_w_real >> 1);
    }

    if((3 == a_stCfg->uvv) && (1 == YUVoutput))
    {
        clip_w_real = (clip_w_real >> 2);
    }

    clip_w_residual = clip_w_real % sub_lb_s_in_line;

    adjusted_clip_w = clip_w_real;

    if(clip_w_residual)
    {
        adjusted_clip_w = clip_w_real + (sub_lb_s_in_line - clip_w_residual);
    }

    sub_blk_w = adjusted_clip_w / sub_lb_s_in_line;

    sub_buf_line_size = sub_blk_w * (sub_lb_s_in_line + 1);

    SubSize = 2 * sub_buf_line_size * (sub_lb_s_in_line + 1);

    if((2 == a_stCfg->uvv) && (1 == YUVoutput))
    {
        SubSize = (SubSize >> 1);
    }

    if((3 == a_stCfg->uvv) && (1 == YUVoutput))
    {
        SubSize = (SubSize >> 2);
    }

    Mainsize = ((Mainsize+0x7) & (~0x7));

    SubSize = ((SubSize+0x7) & (~0x7));

    a_pstSysram->u4SingleBuffSize = (Mainsize + SubSize);

    a_stCfg->pLineBuffSetting->u4MainSize = Mainsize;

    a_stCfg->pLineBuffSetting->u4SubSize = SubSize;

    a_stCfg->pLineBuffSetting->u4LB_S_IN_LINE = main_lb_s_in_line;

    a_stCfg->pLineBuffSetting->u4Main_blk_w = main_blk_w;

    a_stCfg->pLineBuffSetting->u4Main_buf_line_size = main_buf_line_size;

    a_stCfg->pLineBuffSetting->u4Sub_blk_w = sub_blk_w;

    a_stCfg->pLineBuffSetting->u4Sub_buf_line_size = sub_buf_line_size;


    //CLS:Debug
    #if 0
    MDP_INFO("CheckAndAllocateLineBuffer:\n");
    MDP_INFO("Mainsize = %u\n", (unsigned int)Mainsize );
    MDP_INFO("main_lb_s_in_line = %u\n", (unsigned int)main_lb_s_in_line );
    MDP_INFO("main_blk_w = %u\n", (unsigned int)main_blk_w );
    MDP_INFO("main_buf_line_size = %u\n", (unsigned int)main_buf_line_size );
    MDP_INFO("Subsize = %u\n", (unsigned int)SubSize );
    MDP_INFO("sub_lb_s_in_line = %u\n", (unsigned int)sub_lb_s_in_line );
    MDP_INFO("sub_blk_w = %u\n", (unsigned int)sub_blk_w );
    MDP_INFO("sub_buf_line_size = %u\n\n", (unsigned int)sub_buf_line_size );
    #endif


}


int ROTDMA_I::_Config( void )
{
    int i;
    unsigned long mmsys1_reg_base;
    unsigned long reg_base;
    unsigned long descript_base = 0;

    unsigned long u4Index = 0;
    //unsigned long u4BytePerPixel = 0;
   //unsigned long u4IsGenericYUV = 0;// 1 if planar YUV
    //unsigned long u4yh = 0;// horizontal y sample factor
    //unsigned long u4yv = 0;// vertical y sample factor
    //unsigned long u4uvh = 0;// horizontal uv sample factor
    //unsigned long u4uvv = 0;// vertical uv sample factor
    MdpDrvColorInfo_t  ci;
    unsigned long u4YFrame_Start_In_Byte = 0;
    unsigned long u4UVFrame_Start_In_Byte = 0;

    unsigned long y_stride, uv_stride;

    stROTMDALineBuffSetting LBSetting;
    stLineBuffCalculation stBuffCal;
    SYSRAM_INFO stSysram;
    unsigned long rot_id;




    rot_id = this->id(); //virtual
    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();
    reg_base = this->reg_base_addr();           //virtual
    descript_base = this->descript_base_addr(); //virtual


    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    if((uOutBufferCnt > MT6575ROTDMA_MAX_RINGBUFFER_CNT) || (0 == uOutBufferCnt))
    {
        MDP_ERROR("rotdma buffer count exceeds upper limit or 0 %lu\n" , uOutBufferCnt);
        return -1;
    }

    if((dst_color_format <= RGBA8888) && ((MID_VDO_ROT0 == rot_id) || (MID_VDO_ROT1 == rot_id)))
    {
        MDP_ERROR("VDO_ROT does not support RGB output\n");
        return -1;
    }

    if((dst_color_format >= YV16_Planar) && ((MID_RGB_ROT0 == rot_id) || (MID_RGB_ROT1 == rot_id) || (MID_RGB_ROT2 == rot_id)))
    {
        MDP_ERROR("RGB_ROT does not support planar output\n");
        return -1;
    }

    if((dst_color_format == RAW) && (MID_RGB_ROT0 != rot_id) && (MID_VDO_ROT0 != rot_id))
    {
        MDP_ERROR("Only RGBROT0 and VDOROT0 support raw format output\n");
        return -1;
    }

    if((dst_color_format != RGB565) && (dst_color_format != RGB565) && (MID_TV_ROT == rot_id))
    {
        MDP_ERROR("TV ROT only support RGB565 and UYVY format\n");
        return -1;
    }

    // YUV pack format, width must be 2xN
    if(((YUY2_Pack == dst_color_format) || (UYVY_Pack == dst_color_format)) && (src_img_roi.w & 0x1))
    {
        MDP_ERROR("Config ROTDMA failed -- YUV pack format, width must be 2N : %lu" , src_img_roi.w);
        return -1;
    }


    //Android YV12, stride should be 16x align
    if( dst_color_format == ANDROID_YV12 )  {
        if( (bRotate & 0x01) == 0 )     { //0,180
            if( MDP_IS_ALIGN( dst_img_size.w, 4 ) == 0 )    { //Check if 16x align width
                MDP_ERROR("Width of ANDROID_YV12must be 16x. dst_img_size.w = %lu\n", dst_img_size.w );
                return -1;
            }
        } else  {//90,270
            if( MDP_IS_ALIGN( dst_img_size.h, 4 ) == 0 )    { //Check if 16x align width
                MDP_ERROR("Width of ANDROID_YV12must be 16x. dst_img_size.h (after rotate) = %lu\n", dst_img_size.h );
                return -1;
            }
        }
    }


    if( dst_img_yuv_addr[0].y == NULL )
    {
        MDP_ERROR("address is NULL. dst_img_yuv_addr.y = 0x%08X\n", (unsigned int)dst_img_yuv_addr[0].y );
        DumpDebugInfo();
        return -1;
    }




    /*-----------------------------------------------------------------------------
        Calculate buffer size and uv address
      -----------------------------------------------------------------------------*/
    if( MdpDrv_CalImgBufferArraySizeAndFillIn(
                        dst_color_format,
                        dst_img_yuv_addr,
                        uOutBufferCnt,
                        dst_img_size,
                        src_img_roi,
                        ((bFlip << 2) | bRotate ), /*rotate*/
                        &dst_img_buffer_size_total_ ) < 0 )
    {
        MDP_ERROR("<%s> Calc memory array size error.\n", this->name_str() );
        return -1;
    }


    /*-----------------------------------------------------------------------------
        Adapt dest image address
      -----------------------------------------------------------------------------*/

    if( MdpDrv_AdaptMdpYuvAddrArray(    this->id(),
                                        dst_img_yuv_addr, uOutBufferCnt,//Original user input address
                                        dst_img_yuv_adapt_addr,         //Output adapted address
                                        &adapt_m4u_flag_bit_,           //If address has been adapted with m4u mva, corresponding bit will set to 1
                                        &alloc_mva_flag_bit_ ) != 0 )   //Corresponding bit will set to 1 if the mva is new allocated
    {
        MDP_ERROR("<%s> Adapt Memory error.\n", this->name_str() );
        DumpDebugInfo();
        return -1;
    }

    /*-----------------------------------------------------------------------------
        Get Color format information
      -----------------------------------------------------------------------------*/
    if( MdpDrvColorFormatInfoGet( dst_color_format, &ci  ) < 0 )
    {
        MDP_ERROR("Config ROTDMA failed!! -- Unknow color format (0x%x)!!\n",(unsigned int)dst_color_format);
        return -1;
    }


    /*-----------------------------------------------------------------------------
        Register R/W
      -----------------------------------------------------------------------------*/
    Reset();

    //Enable all interrupts
    MT6575_ROT_DMA_IRQ_FLAG(reg_base) = (0x33 << 16);

    //Clear interrupts
    MT6575_ROT_DMA_IRQ_FLAG_CLR(reg_base) = 0x33;

#ifdef MDP_FLAG_1_SUPPORT_DESCRIPTOR
    //Descript mode
    MT6575_ROT_DMA_CFG(reg_base) =  ((unsigned long)bCamIn << 31) +
                                    (((MID_VDO_ROT0 == rot_id) || (MID_VDO_ROT1 == rot_id) ? 0x7 : 0x1) << 17) + /*YUV or RGB SEG enable*/
                                    (1 << 15) +                 /*Q_EMPTY_DROP. 1:Enable 0:Disable (For RDMA, Use 0)*/
                                    ( (uOutBufferCnt-1) << 8) +    /*QUEUE_DEPTH. 0 start*/
                                    (1 << 7) +                  /*MODE. 1:Desc 2:Register*/
                                    0;//bContinuous;            /*Disable auto loop when in descriptor mode*/
    if( b_is_zero_shutter_encode_port_ == 0x1 )
    {
        /*VDO Rotator*/
        if( this->id() & ( MID_VDO_ROT0 | MID_VDO_ROT1 ) )
        {
            MT6575_ROT_DMA_CFG(reg_base) |= (0x0F << 20 ); /*Turn on desc : SEG4~SEG7*/
        }
        /*RGB Rotator*/
        else
        {
            MT6575_ROT_DMA_CFG(reg_base) |= (0x3F << 20 ); /*Turn on desc : SEG4~SEG9*/
        }
    }
    else if ( b_is_zero_shutter_encode_port_ == 0x2 )
    {
        MDP_INFO_QUEUE("CONFIG ROTDMA enable ZSD 2nd path\n");
        /*RGB Rotator*/
        if( this->id() & ( MID_RGB_ROT0 ) )
        {
            MT6575_ROT_DMA_CFG(reg_base) |= (0x3F << 20 ); /*Turn on desc : SEG4~SEG7*/
        }

    }
    MT6575_ROT_DMA_QUEUE_BASE(reg_base) = descript_base;//Queue address

#else
    //Register mode
    MT6575_ROT_DMA_CFG(reg_base) =  ((unsigned long)bCamIn << 31) +
                                    bContinuous;
#endif

    MT6575_ROT_DMA_IN_SEL(reg_base) = 0;


    if(bEnHWTriggerRDMA0)
    {
        MT6575_ROT_DMA_LCD(reg_base) = 0x7;
    }
    else
    {
        MT6575_ROT_DMA_LCD(reg_base) = 0;
    }

    #if 1
    MT6575_ROT_DMA_SLOW_DOWN(reg_base) = 0;
    #else
    MT6575_ROT_DMA_SLOW_DOWN(reg_base) = 0xFFFF0001;
    MDP_WARNING("ROT DMA slow down enable.:0x%08X\n", MT6575_ROT_DMA_SLOW_DOWN(reg_base));
    #endif

    MT6575_ROT_DMA_CON(reg_base) = ((1 << 31) | ((unsigned long)bFlip << 29) |
    	((unsigned long)bRotate << 27) | ((unsigned long)uAlpha << 8) | (0x3 << 4));

    MT6575_ROT_DMA_CON(reg_base) += ci.out_format +
                                    (ci.yh << 18) + (ci.uvh << 20) + (ci.yv << 22) + (ci.uvv << 24) +
                                    ( ( ( ci.uv_stride_align == 0 )? 0 : 1 ) << 26 );//bit 26:Y_UV_PITCH_SPLIT




    //Calculate line buffer size
    stSysram.u4Alignment = 8;
    stSysram.u4SingleBuffSize = 0;
    stSysram.u4MinBuffCnt = 1;
    stSysram.u4RecommandBuffCnt = 1;

    stBuffCal.pLineBuffSetting = &LBSetting;
    stBuffCal.pLineBuffSetting->u4BytePerPixel = ci.byte_per_pixel;
    stBuffCal.pLineBuffSetting->u4MainSize = 0;
    stBuffCal.pLineBuffSetting->u4SubSize  = 0;
    stBuffCal.pLineBuffSetting->u4LB_S_IN_LINE = 0;
    stBuffCal.pLineBuffSetting->u4Main_blk_w = 0;
    stBuffCal.pLineBuffSetting->u4Sub_blk_w = 0;
    stBuffCal.pLineBuffSetting->u4Main_buf_line_size = 0;
    stBuffCal.pLineBuffSetting->u4Sub_buf_line_size = 0;

    //TODO:Should exchang w and h??
    if(0x1 & bRotate)//90 or 270 degree case, needs line buffer
    {
        stBuffCal.clip_w = src_img_roi.w;
        stBuffCal.eOutputColorFMT = dst_color_format;
        stBuffCal.uvv = ci.uvv;

        CheckAndAllocateLineBuffer(&stBuffCal , &stSysram);
    }


    /*-----------------------------------------------------------------------------
        Frame Start Address Calculation/Y & UV stride calculation
      -----------------------------------------------------------------------------*/
    {
        CalcFrameStartAddress_In   param_in;
        CalcFrameStartAddress_Out  param_out;

        param_in.rotate = bRotate;                  /*rotation w/o flip. 0-3*/
        param_in.src_img_roi  = src_img_roi;        /*src roi image before rotation*/
        param_in.dst_img_size = dst_img_size;       /*stride before rotation*/
        param_in.dst_color_format = dst_color_format;
        param_in.b_is_generic_yuv = ci.b_is_generic_yuv;
        param_in.byte_per_pixel = ci.byte_per_pixel;
        /*y,u,v sampling period*/
        param_in.yv = ci.yv;
        param_in.yh = ci.yh;
        param_in.uvv = ci.uvv;
        param_in.uvh = ci.uvh;
        //Stride Align: 0:None 1:2 align 2:4 align 3:8 align 4:16 align
        param_in.y_stride_align = ci.y_stride_align;
        param_in.uv_stride_align= ci.uv_stride_align;



        CalcFrameStartAddress( &param_in, &param_out );

        y_stride  = param_out.y_stride;
        uv_stride = param_out.uv_stride;
        u4YFrame_Start_In_Byte = param_out.y_frame_start_in_byte;
        u4UVFrame_Start_In_Byte= param_out.uv_frame_start_in_byte;
    }

    /*-----------------------------------------------------------------------------

      -----------------------------------------------------------------------------*/

//This register is moved to read DMA, don't need to set here
//    MT6575_ROT_DMA_LCD_STR_ADDR(pBase) = ;

    MT6575_ROT_DMA_SRC_SIZE(reg_base) = src_img_roi.w + ((unsigned long)src_img_roi.h << 16);

    MT6575_ROT_DMA_CLIP_SIZE(reg_base) = src_img_roi.w + ((unsigned long)src_img_roi.h << 16);

    MT6575_ROT_DMA_CLIP_OFFSET(reg_base) = 0;


    MT6575_ROT_DMA_DST_SIZE(reg_base) = ( y_stride * ci.byte_per_pixel) & 0x0003FFFF ;

    if( ci.uv_stride_align != 0 )
    {
        MT6575_ROT_DMA_DST_SIZE(reg_base) |= ( ( uv_stride * ci.byte_per_pixel ) << 18 );
    }


    MT6575_ROT_DMA_CLIP_W_IN_BYTE(reg_base) = (unsigned long)src_img_roi.w * ci.byte_per_pixel;

    MT6575_ROT_DMA_CLIP_H_IN_BYTE(reg_base) = (unsigned long)src_img_roi.h * ci.byte_per_pixel;

    MT6575_ROT_DMA_Y_DST_STR_ADDR(reg_base) = dst_img_yuv_adapt_addr[0].y + u4YFrame_Start_In_Byte;

    MT6575_ROT_DMA_U_DST_STR_ADDR(reg_base) = dst_img_yuv_adapt_addr[0].u + u4UVFrame_Start_In_Byte;

    MT6575_ROT_DMA_V_DST_STR_ADDR(reg_base) = dst_img_yuv_adapt_addr[0].v + u4UVFrame_Start_In_Byte;

#ifdef MDP_FLAG_1_SUPPORT_DESCRIPTOR

    /*Backup descriptor value, for latter queue/dequeue used*/
    desc_src_img_roi_               = src_img_roi;
    desc_dst_img_size_              = dst_img_size;
    desc_is_generic_yuv_            = ci.b_is_generic_yuv;
    desc_byte_per_pixel_            = ci.byte_per_pixel;
    desc_yv_                        = ci.yv;
    desc_yh_                        = ci.yh;
    desc_uvv_                       = ci.uvv;
    desc_uvh_                       = ci.uvh;
    desc_y_stride_align_            = ci.y_stride_align;
    desc_uv_stride_align_           = ci.uv_stride_align;
    desc_y_stride_                  = y_stride;
    desc_uv_stride_                 = uv_stride;
    desc_y_frame_start_in_byte_     = u4YFrame_Start_In_Byte;
    desc_uv_frame_start_in_byte_    = u4UVFrame_Start_In_Byte;

    for(u4Index = 0; u4Index < uOutBufferCnt ; u4Index += 1)
    {
        _DescQueueRefillByIndex( u4Index, reg_base );

    }

    /*Reset SW write index , this "0" is actually a overflow value of uOutBufferCnt.*/
    desc_sw_write_index_ = 0;

#endif
    if( rot_id== MID_VDO_ROT1 )
    {
        MDP_INFO("Try to allocate SYSRAM Main:%lu Sub:%lu\n", stBuffCal.pLineBuffSetting->u4MainSize, stBuffCal.pLineBuffSetting->u4SubSize );
        //Allocate Main buffer sysram
        sysram_.size = stBuffCal.pLineBuffSetting->u4MainSize;
        sysram_.address = 0;
        if( sysram_.size != 0 )
        {
            sysram_.address = MdpSysram_Alloc( this->id(), sysram_.size , MDPSYSRAM_SUBCAT_NORMAL );

            if( sysram_.address == 0 )
            {
                sysram_.size == 0;
                MDP_ERROR("Main SYSRAM allocate failed!\n");
                DumpDebugInfo();
                return -1;
            }
        }

        //Allocate Sub buffer sysram
        sysram_sub_.size = stBuffCal.pLineBuffSetting->u4SubSize;
        sysram_sub_.address = 0;
        if( sysram_sub_.size != 0 )
        {
            sysram_sub_.address = MdpSysram_Alloc( this->id(), sysram_sub_.size , MDPSYSRAM_SUBCAT_SUBROTSYSRAM );

            if( sysram_sub_.address == 0 )
            {
                sysram_sub_.size == 0;
                MDP_ERROR("Sub SYSRAM allocate failed!\n");
                DumpDebugInfo();
                return -1;
            }
        }



        MT6575_ROT_DMA_BUF_ADDR0(reg_base) = sysram_.address;//u4MainBuffAddr;
        MT6575_ROT_DMA_BUF_ADDR1(reg_base) = MT6575_ROT_DMA_BUF_ADDR0(reg_base);
        MT6575_ROT_DMA_MAIN_BUFF_SIZE(reg_base) = (stBuffCal.pLineBuffSetting->u4Main_blk_w << 17) + stBuffCal.pLineBuffSetting->u4Main_buf_line_size;


        MT6575_ROT_DMA_BUF_ADDR2(reg_base) = sysram_sub_.address;
        MT6575_ROT_DMA_BUF_ADDR3(reg_base) = MT6575_ROT_DMA_BUF_ADDR2(reg_base);
        MT6575_ROT_DMA_SUB_BUFF_SIZE(reg_base) = (stBuffCal.pLineBuffSetting->u4Sub_blk_w << 17) + stBuffCal.pLineBuffSetting->u4Sub_buf_line_size;

    }
    else
    {
        MDP_INFO("Try to allocate SYSRAM Single:%lu\n", stSysram.u4SingleBuffSize );

        //Allocate sysram
        sysram_.size = stSysram.u4SingleBuffSize;
        sysram_.address = 0;
        if( sysram_.size != 0 )
        {
            sysram_.address = MdpSysram_Alloc( this->id(), sysram_.size , MDPSYSRAM_SUBCAT_NORMAL/*0*/ );

            if( sysram_.address == 0 )
            {
                sysram_.size == 0;
                MDP_ERROR("SYSRAM allocate failed!\n");
                DumpDebugInfo();
                return -1;
            }
        }



        MT6575_ROT_DMA_BUF_ADDR0(reg_base) = sysram_.address;//u4MainBuffAddr;
        MT6575_ROT_DMA_BUF_ADDR1(reg_base) = MT6575_ROT_DMA_BUF_ADDR0(reg_base);
        MT6575_ROT_DMA_MAIN_BUFF_SIZE(reg_base) = (stBuffCal.pLineBuffSetting->u4Main_blk_w << 17) + stBuffCal.pLineBuffSetting->u4Main_buf_line_size;


        MT6575_ROT_DMA_BUF_ADDR2(reg_base) = sysram_.address + stBuffCal.pLineBuffSetting->u4MainSize;
        MT6575_ROT_DMA_BUF_ADDR3(reg_base) = MT6575_ROT_DMA_BUF_ADDR2(reg_base);
        MT6575_ROT_DMA_SUB_BUFF_SIZE(reg_base) = (stBuffCal.pLineBuffSetting->u4Sub_blk_w << 17) + stBuffCal.pLineBuffSetting->u4Sub_buf_line_size;

    }



    MT6575_ROT_DMA_PERF(reg_base) = ((bCamIn ? (0x3 << 30) : 0) | (1 << 24) | (stBuffCal.pLineBuffSetting->u4LB_S_IN_LINE << 16)
        | ((stBuffCal.pLineBuffSetting->u4LB_S_IN_LINE * ci.byte_per_pixel ) << 8) | 0xF);//Use 0xF to enhance emi usage effeiciency


    if((MID_RGB_ROT0 == rot_id) || (MID_RGB_ROT1 == rot_id) || (MID_RGB_ROT2 == rot_id))
    {
        MT6575_ROT_DMA_DITHER(reg_base) = (bDithering ? 0x232003 : 0);
    }

	MT6575_ROT_DMA_LCD(reg_base) = (1 == bEnHWTriggerRDMA0 ? 0x13 : 0);

    if(bEnHWTriggerRDMA0)
    {
		switch(rot_id)
		{
			case MID_RGB_ROT0 :
				MT6575_MMSYS_R_DMA0_TRIG_SEL(mmsys1_reg_base) = 1;
			break;
			case MID_RGB_ROT1 :
				MT6575_MMSYS_R_DMA0_TRIG_SEL(mmsys1_reg_base) = 2;
			break;
			case MID_VDO_ROT0 :
				MT6575_MMSYS_R_DMA0_TRIG_SEL(mmsys1_reg_base) = 0;
			break;
			case MID_VDO_ROT1 :
				MT6575_MMSYS_R_DMA0_TRIG_SEL(mmsys1_reg_base) = 3;
			break;
			default :
				MDP_ERROR("unknown ROTDMA!!\n");
				return -1;
			break;
		}


    }

    return 0;
}

int ROTDMA_I::_Enable( void )
{

    /*-----------------------------------------------------------------------------
        Invalid Cache before HW write
      -----------------------------------------------------------------------------*/
    if( MdpDrv_CacheSync(   this->id(),
                            MDPDRV_CACHE_SYNC_INVALID_BEFORE_HW_WRITE_MEM,
                            dst_img_yuv_addr[0].y , dst_img_buffer_size_total_
                        ) < 0 )
    {
        MDP_WARNING("ROTDMA Cache sync error. address:0x%08X size:0x%08X\n", (unsigned int)dst_img_yuv_addr[0].y, (unsigned int)dst_img_buffer_size_total_ );
        //return -1;
    }


    /*-----------------------------------------------------------------------------
        Trigger HW
      -----------------------------------------------------------------------------*/
    unsigned long a_pRegBase;

    a_pRegBase = this->reg_base_addr(); //virtual


    if( a_pRegBase== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    MT6575_ROT_DMA_STOP(a_pRegBase) = 0;

    MT6575_ROT_DMA_EN(a_pRegBase) = 1;


    return 0;
}

int ROTDMA_I::_Disable( void )
{
    unsigned long a_pRegBase;

    a_pRegBase = this->reg_base_addr(); //virtual


    if( a_pRegBase== 0 ){
        return -1;
    }

#ifdef DESCRIPTMODE
    //    while(0x1 != MT6575_ROT_DMA_QUEUE_RSTA(u4Base));
#endif

    MT6575_ROT_DMA_STOP(a_pRegBase) = 1;
    MT6575_ROT_DMA_STOP(a_pRegBase) = 0;

    /*Free_Sysram( this->id() ); //virtual*/
    if( sysram_.size > 0 )
    {
        MdpSysram_Free( this->id(), sysram_.address , MDPSYSRAM_SUBCAT_NORMAL );
        sysram_.address = 0;
        sysram_.size = 0;
    }

    /*Free_Sysram( this->id() ); //virtual*/
    if( sysram_sub_.size > 0 )
    {
        MdpSysram_Free( this->id(), sysram_sub_.address , MDPSYSRAM_SUBCAT_SUBROTSYSRAM );
        sysram_sub_.address = 0;
        sysram_sub_.size = 0;
    }

    Reset();


    /*UnAdapt MdpYuvAddress*/
    MdpDrv_UnAdaptMdpYuvAddrArray(  this->id(),
                                    dst_img_yuv_addr, uOutBufferCnt,     //Original user input address
                                    dst_img_yuv_adapt_addr,             //Adapted address
                                    adapt_m4u_flag_bit_ ,               //Corresponding bit will set to 1 if address had been adapted with m4u mva
                                    alloc_mva_flag_bit_ );              //Corresponding bit will set to 1 if the mva is new allocated


    return 0;
}

int ROTDMA_I::CheckBusy( unsigned long* desc_read_pointer )
{
    static unsigned long    _lcd_too_slow_show_frequency = 0;
    unsigned long reg_base;

    unsigned long u4Result;

    unsigned long u4BuffCnt;


    reg_base = this->reg_base_addr(); //virtual


    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    //Check error status as well
    u4Result = MT6575_ROT_DMA_IRQ_FLAG(reg_base);

    //Clear interrupt
    MT6575_ROT_DMA_IRQ_FLAG_CLR(reg_base) = ( 0x33 & u4Result );

    u4Result &= 0xFF;

    if(u4Result & 0x1){/*MDP_INFO("engine finished the decriptor...\n");*/}

    if(u4Result & 0x2){     MDP_ERROR("SW configuration error. Invalid descriptor mode...\n");}

    if(u4Result & 0x10){    if( (_lcd_too_slow_show_frequency++%300) == 0 )  MDP_INFO("LCD too slow...(%lu)\n",_lcd_too_slow_show_frequency);  }

    if(u4Result & 0x20){    MDP_ERROR("SW configuration error. SW writing to descriptor queue when command queue writer is busy...\n");}

    u4BuffCnt = ((MT6575_ROT_DMA_CFG(reg_base) & 0xf0000) >> 16);

    *desc_read_pointer = ((MT6575_ROT_DMA_QUEUE_RSTA(reg_base) & 0xf00) >> 8);

    *desc_read_pointer = (0 == *desc_read_pointer ? u4BuffCnt : (*desc_read_pointer) - 1);

    return (u4Result & 0x1 ? 0 : 1);//MT6575_ROT_DMA_EN(pBase);
}


int ROTDMA_I::Reset( void )
{
    volatile unsigned long time_out = MDP_DMA_RESET_TIMEOUT;//0xFFFFFF;
    unsigned long reg_base;
    volatile unsigned long time_delay;

    reg_base = this->reg_base_addr();

    if( reg_base== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    MT6575_ROT_DMA_RESET(reg_base) = 0x2;

    while(0x2 == MT6575_ROT_DMA_RESET(reg_base))
    {
        time_out--;
        if( time_out == 0 ){
            MDP_ERROR("Config <%s> failed -- reset ROTDMA timeout,proceed hard reset\n",this->name_str());

            /*Hard Reset*/
            MT6575_ROT_DMA_RESET( reg_base ) = 0x1;
            for( time_delay = 0 ; time_delay < 50 ; time_delay += 1){;}//Delay 50 cycle
            MT6575_ROT_DMA_RESET( reg_base ) = 0x0;

            return 0;
        }
    }

    //New Rotater Reset Change for bug fix
        /*RGB ROT*/
    if( this->id() & ( MID_RGB_ROT0|MID_RGB_ROT1|MID_RGB_ROT2 ) )
    {
        MT6575_ROT_DMA_CON( reg_base ) = 0x08000003;
        MT6575_ROT_DMA_RESET(reg_base) = 0x2;
        MT6575_ROT_DMA_RESET(reg_base) = 0x2;
        MT6575_ROT_DMA_RESET(reg_base) = 0x2;
        MT6575_ROT_DMA_RESET(reg_base) = 0x2;
        MT6575_ROT_DMA_RESET(reg_base) = 0x2;
        MT6575_ROT_DMA_CON( reg_base ) = 0x08000007;
        MT6575_ROT_DMA_RESET(reg_base) = 0x2;
    }
    else/*VDO ROT*/
    {
        MT6575_ROT_DMA_CON( reg_base ) = 0x08000000;
        MT6575_ROT_DMA_RESET(reg_base) = 0x2;
    }


    return 0;
}

int ROTDMA_I::HardReset( void )
{
    unsigned long reg_base;
    volatile unsigned long time_delay;

    reg_base = this->reg_base_addr();

    if( reg_base== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    /*Hard Reset*/
    MT6575_ROT_DMA_RESET( reg_base ) = 0x1;
    for( time_delay = 0 ; time_delay < 50 ; time_delay += 1){;}//Delay 50 cycle
    MT6575_ROT_DMA_RESET( reg_base ) = 0x0;

    MDP_WARNING("<%s> Hard Reset!\n",this->name_str());


    return 0;
}


int ROTDMA_I::DumpRegisterCustom( int mode )
{

    unsigned long reg_base;
    unsigned long reg_val;

    reg_base = this->reg_base_addr();

    if( reg_base== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    //MT6575_ROT_DMA_CFG
    reg_val = MT6575_ROT_DMA_CFG(reg_base);
    MDP_PRINTF("AUTO_LOOP=%d\n",reg_val&(0x1)?1:0);
    MDP_PRINTF("FRAME_SYNC_EN=%d\n",reg_val&(0x1<<31)?1:0);


    //MT6575_ROT_DMA_IRQ_FLAG
    reg_val = MT6575_ROT_DMA_IRQ_FLAG(reg_base);
    MDP_PRINTF("FLAG0(Done)=%d\n",reg_val&(0x1)?1:0);
    MDP_PRINTF("FLAG1(SW err)=%d\n",reg_val&(0x1<<1)?1:0);
    MDP_PRINTF("FLAG4(LCD too slow)=%d\n",reg_val&(0x1<<4)?1:0);
    MDP_PRINTF("FLAG5(SW err)=%d\n",reg_val&(0x1<<5)?1:0);


    //MT6575_ROT_DMA_DBG_ST0
    reg_val = MT6575_ROT_DMA_DBG_ST0(reg_base);
    MDP_PRINTF("CLIP_CUR_X(counter)=%d CLIP_CUR_Y(counter)=%d\n",(int)(reg_val&0x0000FFFF) , (int)((reg_val>>16)&0x0000FFFF));


    //MT6575_ROT_DMA_LCD
    reg_val = MT6575_ROT_DMA_LCD(reg_base);
    MDP_PRINTF("ROT_DMA_LCD(HW Trigger)=0x%X\n",(int)reg_val);

//---
    //MT6575_ROT_DMA_SRC_SIZE
    reg_val = MT6575_ROT_DMA_SRC_SIZE(reg_base);
    MDP_PRINTF("SRC_W=%d\n",(int)(reg_val&0x0000FFFF));
    MDP_PRINTF("SRC_H=%d\n",(int)((reg_val&0xFFFF0000)>>16));

    //MT6575_ROT_DMA_CLIP_OFFSET
    reg_val = MT6575_ROT_DMA_CLIP_OFFSET(reg_base);
    MDP_PRINTF("CLIP_X=%d\n",(int)(reg_val&0x0000FFFF));
    MDP_PRINTF("CLIP_Y=%d\n",(int)((reg_val&0xFFFF0000)>>16));

    //MT6575_ROT_DMA_CLIP_SIZE
    reg_val = MT6575_ROT_DMA_CLIP_SIZE(reg_base);
    MDP_PRINTF("CLIP_W=%d\n",(int)(reg_val&0x0000FFFF));
    MDP_PRINTF("CLIP_H=%d\n",(int)((reg_val&0xFFFF0000)>>16));


   //MT6575_ROT_DMA_DST_SIZE
   reg_val = MT6575_ROT_DMA_DST_SIZE(reg_base);
   MDP_PRINTF("DST_W_SIZE=%d(Y) %d(UV)\n",(int)(reg_val & 0x0003FFFF ), (int)( reg_val >> 18) );

   //MT6575_ROT_DMA_CLIP_W_IN_BYTE
   reg_val = MT6575_ROT_DMA_CLIP_W_IN_BYTE(reg_base);
   MDP_PRINTF("CLIP_W_SIZE=%d\n",(int)(reg_val));


   //MT6575_ROT_DMA_CLIP_H_IN_BYTE
   reg_val = MT6575_ROT_DMA_CLIP_H_IN_BYTE(reg_base);
   MDP_PRINTF("CLIP_H_SIZE=%d\n",(int)(reg_val));


   //MT6575_ROT_DMA_QUEUE_RSTA
   reg_val = MT6575_ROT_DMA_QUEUE_RSTA(reg_base);
   MDP_PRINTF("Descripter EMPTY=%d\n",(int)((reg_val & 0x4) >> 2));




    return 0;
}


int ROTDMA_I::DumpDebugInfo( void )
{
    int i;

    MDPELEMENT_I::DumpDebugInfo();

    for( i = 0; i < uOutBufferCnt; i++ ) {
        MDP_PRINTF("%d/%d:\n",(int)i+1,(int)uOutBufferCnt);
        MDP_DUMP_VAR_H(dst_img_yuv_addr[i].y);
        MDP_DUMP_VAR_H(dst_img_yuv_addr[i].u);
        MDP_DUMP_VAR_H(dst_img_yuv_addr[i].v);
        MDP_DUMP_VAR_H(dst_img_yuv_addr[i].y_buffer_size);
        MDP_DUMP_VAR_H(dst_img_yuv_addr[i].u_buffer_size);
        MDP_DUMP_VAR_H(dst_img_yuv_addr[i].v_buffer_size);
        MDP_PRINTF("\n");
    }

    MDP_DUMP_VAR_D(src_img_roi.x);
    MDP_DUMP_VAR_D(src_img_roi.y);
    MDP_DUMP_VAR_D(src_img_roi.w);
    MDP_DUMP_VAR_D(src_img_roi.h);

    MDP_DUMP_VAR_D(dst_img_size.w);
    MDP_DUMP_VAR_D(dst_img_size.h);

    MDP_DUMP_VAR_D(dst_color_format);

    MDP_DUMP_VAR_D(uOutBufferCnt);
    MDP_DUMP_VAR_D(bContinuous);
    MDP_DUMP_VAR_D(bFlip);
    MDP_DUMP_VAR_D(bRotate);
    MDP_DUMP_VAR_D(bDithering);
    MDP_DUMP_VAR_D(uAlpha);
    MDP_DUMP_VAR_D(bCamIn);
    MDP_DUMP_VAR_D(bEnHWTriggerRDMA0);


    for( i = 0; i < uOutBufferCnt; i++ ) {
        MDP_PRINTF("%d/%d:\n",(int)i+1,(int)uOutBufferCnt);
        MDP_DUMP_VAR_H(dst_img_yuv_adapt_addr[i].y);
        MDP_DUMP_VAR_H(dst_img_yuv_adapt_addr[i].u);
        MDP_DUMP_VAR_H(dst_img_yuv_adapt_addr[i].v);
        MDP_DUMP_VAR_H(dst_img_yuv_adapt_addr[i].y_buffer_size);
        MDP_DUMP_VAR_H(dst_img_yuv_adapt_addr[i].u_buffer_size);
        MDP_DUMP_VAR_H(dst_img_yuv_adapt_addr[i].v_buffer_size);
        MDP_PRINTF("\n");
    }

    MDP_DUMP_VAR_H(adapt_m4u_flag_bit_);
    MDP_DUMP_VAR_H(dst_img_buffer_size_total_);

    MDP_DUMP_VAR_H(sysram_.address);
    MDP_DUMP_VAR_H(sysram_sub_.size);

    MDP_DUMP_VAR_D(desc_sw_write_index_);

    MDP_DUMP_VAR_D(b_is_zero_shutter_encode_port_); /*1:zero shutter enable*/

    MDP_DUMP_VAR_D(desc_src_img_roi_.x);
    MDP_DUMP_VAR_D(desc_src_img_roi_.y);
    MDP_DUMP_VAR_D(desc_src_img_roi_.w);
    MDP_DUMP_VAR_D(desc_src_img_roi_.h);

    MDP_DUMP_VAR_D(desc_dst_img_size_.w);
    MDP_DUMP_VAR_D(desc_dst_img_size_.h);

    MDP_DUMP_VAR_D(desc_is_generic_yuv_);
    MDP_DUMP_VAR_D(desc_byte_per_pixel_);
    MDP_DUMP_VAR_D(desc_yv_);
    MDP_DUMP_VAR_D(desc_yh_);
    MDP_DUMP_VAR_D(desc_uvv_);
    MDP_DUMP_VAR_D(desc_uvh_);
    MDP_DUMP_VAR_H(desc_y_stride_);
    MDP_DUMP_VAR_H(desc_uv_stride_);
    MDP_DUMP_VAR_H(desc_y_frame_start_in_byte_);
    MDP_DUMP_VAR_H(desc_uv_frame_start_in_byte_);

    return 0;
}


// last preview frame for zsd
int ROTDMA_I::AdpteLastPreviewFrameBuffer()
{
    /*-----------------------------------------------------------------------------
        Calculate buffer size and uv address
      -----------------------------------------------------------------------------*/
    int i =0;
    if( MdpDrv_CalImgBufferArraySizeAndFillIn(
                        dst_color_format,
                        dst_img_yuv_addr_last_frame_,
                        uOutBufferCntZSD,
                        dst_img_size_last_frame_,
                        MdpRect(0, 0, dst_img_size_last_frame_.w, dst_img_size_last_frame_.h),
                        ((bFlip << 2) | bRotate ), /*rotate*/
                        &dst_img_buffer_size_total_last_frame_) < 0 )
    {
        MDP_ERROR("<%s> Calc memory array size error.\n", this->name_str() );
        return -1;
    }

    for (i = 0; i < uOutBufferCntZSD; i++) {
        MDP_DEBUG("[ZSD] VA [%d] = 0x%x", i, (unsigned int)dst_img_yuv_addr_last_frame_[i].y);
    }

    /*-----------------------------------------------------------------------------
        Adapt dest image address
      -----------------------------------------------------------------------------*/
    for (i = 0; i < uOutBufferCntZSD; i++) {
        if (MdpDrv_AdaptM4uForZSD(dst_img_yuv_addr_last_frame_[i].y,
                                  dst_img_yuv_addr_last_frame_[0].y_buffer_size,
                                  &(dst_img_yuv_adapt_addr_last_frame_[i].y)))
        {
            MDP_ERROR("[ZSD] <%s> Adapt Memory error.\n", this->name_str() );
            return -1;
        }

        MDP_DEBUG("[ZSD] MVA [%d] = 0x%x", i,(unsigned int)dst_img_yuv_adapt_addr_last_frame_[i].y);
    }

    return 0;

}

int ROTDMA_I::UnAdpteLastPreviewFrameBuffer()
{

    int i = 0;
    for (i = 0; i < uOutBufferCntZSD; i++) {
        MdpDrv_UnAdaptM4uForZSD(dst_img_yuv_addr_last_frame_[i].y,
                                  dst_img_yuv_addr_last_frame_[0].y_buffer_size,
                                  dst_img_yuv_adapt_addr_last_frame_[i].y);
    }
    return 0;
}
int ROTDMA_I::DescQueueManualRefill(unsigned long dst_width, unsigned long dst_height)
{
    unsigned long   reg_base;
    unsigned long   reg_q_rsta;
    unsigned long   reg_q_wsta;
    unsigned long   reg_q_rd_base_pa;
    unsigned long   reg_q_rd_base_va;
    unsigned long   reg_q_base_pa;
    unsigned long   reg_q_base_va;
    int             b_empty;
    unsigned long   hw_read_index,hw_r_wrap;  /*RPT*/
    unsigned long   hw_write_index,hw_w_wrap; /*WPT*/
    unsigned long   desc_remain_count;

    unsigned long   new_reg_base;
    unsigned long   rd_rpt;
    unsigned long   rd_adv_cnt;
    int             ret = -1;

#define MM_SYSRAM_BASE_PA       (0xC2000000)

    reg_base = this->reg_base_addr();

    new_reg_base = MdpSysram_Remap();
    if( new_reg_base == 0 ) {
         MDP_ERROR("remap SYRAM descprioter error\n");
            return -1;
    }

    MT6575_ROT_DMA_LOCK( reg_base ) = 0x1;

    reg_q_rsta  = MT6575_ROT_DMA_QUEUE_RSTA( reg_base );
    reg_q_wsta = MT6575_ROT_DMA_QUEUE_WSTA( reg_base );
    reg_q_base_pa = MT6575_ROT_DMA_QUEUE_BASE( reg_base );
    reg_q_rd_base_pa = MT6575_ROT_DMA_RD_BASE( reg_base );
    //reg_q_base_va = SYSRAM_BASE + (reg_q_base_pa - MM_SYSRAM_BASE_PA);
    reg_q_rd_base_va = new_reg_base + (reg_q_rd_base_pa - MM_SYSRAM_BASE_PA);

    b_empty         = ( reg_q_rsta & ( 0x1 << 2 ) ) >> 2;
    hw_read_index   = ( reg_q_rsta & ( 0xF << 8 ) ) >> 8;
    hw_r_wrap       = ( reg_q_rsta & ( 0x1 << 12 ) ) >> 12;


    hw_write_index  = ( reg_q_wsta & ( 0xF << 8 ) ) >> 8;
    hw_w_wrap       = ( reg_q_wsta & ( 0x1 << 12 ) ) >> 12;


    if( b_empty== 1 ) {
        desc_remain_count = 0;
    } else {
        if( hw_r_wrap == hw_w_wrap )    {
            desc_remain_count = hw_write_index - hw_read_index;
        } else  {
            desc_remain_count = uOutBufferCnt - hw_read_index + hw_write_index;
        }
    }
    MDP_INFO_PERSIST("[QUEUE]<%s> 40:%08x, 48:%08x, 58:%08x\n", name_str(),(unsigned int)reg_q_rsta,(unsigned int)reg_q_rd_base_pa, (unsigned int)reg_q_wsta);
    MDP_INFO_PERSIST("[QUEUE]empty %d remain %d\n", b_empty, (int)desc_remain_count );

    //if (CheckBusy(&rd_rpt))
    {
        if (desc_remain_count >= 2)
        {
            for (rd_adv_cnt = desc_remain_count; rd_adv_cnt > 2; rd_adv_cnt--)
            {

                MDP_INFO_PERSIST("[QUEUE]advance RPT\n");
                MT6575_ROT_DMA_RPT_ADVANCE(reg_base) = 0x1;
            }

        }
        else
        {
            MDP_ERROR("ROTDMA Busy %d, remain %d desc, can not manual refill\n", CheckBusy(&rd_rpt), (int)desc_remain_count);
            goto End;
        }

    }/*
    else
    {
        if (desc_remain_count >= 2)
        {
            for (rd_adv_cnt = desc_remain_count; rd_adv_cnt > 2; rd_adv_cnt--)
            {
                MT6575_ROT_DMA_RPT_ADVANCE(reg_base) = 0x1;
            }

        }
        else
        {
            MDP_ERROR("ROTDMA is busy, remain %d desc, can not manual refill\n", desc_remain_count);
            goto End;

        }

    }*/

    // *((volatile unsigned long*)reg_q_rd_base_va) = dst_img_yuv_adapt_addr[0].y;

     *((volatile unsigned long*)reg_q_rd_base_va) = dst_img_yuv_adapt_addr_last_frame_[0].y;
    //MDP_INFO_MDPD("0x%x\n", dst_img_yuv_adapt_addr_last_frame_[0].y);

    *((volatile unsigned long*)(reg_q_rd_base_va + 4*1)) = dst_height<<16 | dst_width;
    //clip_w & clip_h
    *((volatile unsigned long*)(reg_q_rd_base_va + 4*2)) = dst_height<<16 | dst_width;
    //clip_x & clip_y
    *((volatile unsigned long*)(reg_q_rd_base_va + 4*3)) = 0;
    //dst_w_in_byte
    *((volatile unsigned long*)(reg_q_rd_base_va + 4*4)) = dst_width*2;
    //clip_w_in_byte
    *((volatile unsigned long*)(reg_q_rd_base_va + 4*5)) = dst_width*2;
    //clip_h_in_byte
    *((volatile unsigned long*)(reg_q_rd_base_va + 4*6)) = dst_height*2;
    ret = 0;

End:
    MT6575_ROT_DMA_LOCK( reg_base ) = 0x0;
    return ret;

}

int ROTDMA_I::QueryCurrentSwReadIndex()
{

    /*Update SW write index*/
    MDP_INFO_PERSIST("[QUEUE] [ZSD] <%s> Current Index = %d \n",
            this->name_str(), (int)desc_sw_write_index_);

    return desc_sw_write_index_;

}



int ROTDMA_I::DescQueueFullFill(RotdmaZsdZoomParam param )
{
    unsigned long   reg_base;
    unsigned long   reg_val;
    unsigned long   reg_val2;
    int             b_empty;
    unsigned long   hw_read_index,hw_r_wrap;  /*RPT*/
    unsigned long   hw_write_index,hw_w_wrap; /*WPT*/
    unsigned long   desc_remain_count;

    reg_base = this->reg_base_addr();


    if( reg_base== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    reg_val  = MT6575_ROT_DMA_QUEUE_RSTA( reg_base );
    reg_val2 = MT6575_ROT_DMA_QUEUE_WSTA( reg_base );


    b_empty         = ( reg_val & ( 0x1 << 2 ) ) >> 2;
    hw_read_index   = ( reg_val & ( 0xF << 8 ) ) >> 8;
    hw_r_wrap       = ( reg_val & ( 0x1 << 12 ) ) >> 12;

    hw_write_index  = ( reg_val2& ( 0xF << 8 ) ) >> 8;
    hw_w_wrap       = ( reg_val2 & ( 0x1 << 12 ) ) >> 12;

    if( b_empty== 1 ){
        desc_remain_count = 0;
    } else
    {
        if( hw_r_wrap == hw_w_wrap ){
            desc_remain_count = hw_write_index - hw_read_index;
        } else  {
            desc_remain_count = uOutBufferCnt - hw_read_index + hw_write_index;
        }
    }

    ConfigZsd_DescUpdate(param);

    MDP_DEBUG("[ZSD] Full fill desc, remian %d\n",(int)desc_remain_count );
    for (int i = (uOutBufferCnt - desc_remain_count); i > 0; i--)
    {

        MDP_DEBUG("[ZSD] Full fill desc, hw index %d, 0x%x\n",(int)desc_remain_count ,(unsigned int)dst_img_yuv_adapt_addr_last_frame_[i].y);
        _DescQueueRefillByIndexEx(hw_write_index, reg_base );
        hw_write_index = (hw_write_index == (unsigned long)(uOutBufferCnt -1)) ? 0 : (hw_write_index + 1) ;
        desc_sw_write_index_ = (desc_sw_write_index_+1) == uOutBufferCnt ? 0 : (desc_sw_write_index_+1);
    }

    return 0;
}
int ROTDMA_I::DescQueueGetFreeList( unsigned long *p_StartIndex, unsigned long *p_Count ) /*Equel to De Queue Buffer in 73*/
{
    unsigned long   reg_base;
    unsigned long   reg_val;
    unsigned long   reg_val2;
    int             b_empty;
    unsigned long   hw_read_index,hw_r_wrap;  /*RPT*/
    unsigned long   hw_write_index,hw_w_wrap; /*WPT*/
    unsigned long   desc_remain_count;

    reg_base = this->reg_base_addr();


    if( reg_base== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    reg_val  = MT6575_ROT_DMA_QUEUE_RSTA( reg_base );
    reg_val2 = MT6575_ROT_DMA_QUEUE_WSTA( reg_base );


    b_empty         = ( reg_val & ( 0x1 << 2 ) ) >> 2;
    hw_read_index   = ( reg_val & ( 0xF << 8 ) ) >> 8;
    hw_r_wrap       = ( reg_val & ( 0x1 << 12 ) ) >> 12;

    hw_write_index  = ( reg_val2& ( 0xF << 8 ) ) >> 8;
    hw_w_wrap       = ( reg_val2 & ( 0x1 << 12 ) ) >> 12;


    /*Queue is full*/
    if( !b_empty &&  ( hw_read_index == desc_sw_write_index_ ) )
    {
        *p_StartIndex = desc_sw_write_index_;
        *p_Count = 0;
    }
    /*Queue still has free space, return immediately and no wait*/
    else
    {

        *p_StartIndex = desc_sw_write_index_;
        *p_Count = ( hw_read_index > desc_sw_write_index_ ?
                        (hw_read_index - desc_sw_write_index_) :
                        (uOutBufferCnt - desc_sw_write_index_ + hw_read_index)  );
    }

    if( b_empty== 1 ){
        desc_remain_count = 0;
    } else
    {
        if( hw_r_wrap == hw_w_wrap ){
            desc_remain_count = hw_write_index - hw_read_index;
        } else  {
            desc_remain_count = uOutBufferCnt - hw_read_index + hw_write_index;
        }

        #if 0
        desc_remain_count = (hw_read_index >= hw_write_index) ?
                            ( uOutBufferCnt - hw_read_index + hw_write_index ) : ( hw_write_index - hw_read_index );
        #endif
    }

    if( *p_Count== 0 )
    {
        MDP_INFO_QUEUE("[QUEUE]Dequeue <%s> get free list. Start = %d Count = %d desc_remain = %d (r,w)=(%d,%d)(no free bufffer)\n",
                    name_str(),(int)*p_StartIndex, (int)*p_Count, (int)desc_remain_count, (int)hw_read_index, (int)hw_write_index );

    } else
    {
        MDP_INFO_QUEUE("[QUEUE]Dequeue <%s> get free list. Start = %d Count = %d desc_remain = %d (r,w)=(%d,%d)\n",
                    name_str(),(int)*p_StartIndex, (int)*p_Count, (int)desc_remain_count, (int)hw_read_index, (int)hw_write_index );
    }

    return 0;

}


int ROTDMA_I::DescQueueRefill( void ) /*Equel to En Queue Buffer in 73*/
{
    unsigned long   reg_base;
    unsigned long   reg_val;
    unsigned long   reg_val2;
    int             b_empty;
    unsigned long   hw_read_index,hw_r_wrap;  /*RPT*/
    unsigned long   hw_write_index,hw_w_wrap; /*WPT*/
    unsigned long   desc_remain_count;
    int             ret_val = 0;

    reg_base = this->reg_base_addr();



    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }



    //Fill descriptor
    ret_val = _DescQueueRefillByIndex( desc_sw_write_index_, reg_base );


    //Query  remain descriptor
    reg_val  = MT6575_ROT_DMA_QUEUE_RSTA( reg_base );
    reg_val2 = MT6575_ROT_DMA_QUEUE_WSTA( reg_base );

    b_empty         = ( reg_val & ( 0x1 << 2 ) ) >> 2;
    hw_read_index   = ( reg_val & ( 0xF << 8 ) ) >> 8;
    hw_r_wrap       = ( reg_val & ( 0x1 << 12 ) ) >> 12;

    hw_write_index  = ( reg_val2& ( 0xF << 8 ) ) >> 8;
    hw_w_wrap       = ( reg_val2 & ( 0x1 << 12 ) ) >> 12;


    if( b_empty== 1 ){
        desc_remain_count = 0;
    } else
    {
        if( hw_r_wrap == hw_w_wrap ){
            desc_remain_count = hw_write_index - hw_read_index;
        } else  {
            desc_remain_count = uOutBufferCnt - hw_read_index + hw_write_index;
        }

    #if 0
        desc_remain_count = (hw_read_index >= hw_write_index) ?
                            ( uOutBufferCnt - hw_read_index + hw_write_index ) : ( hw_write_index - hw_read_index );

    #endif
    }


    if( ret_val == 0 )
    {
        /*Update SW write index*/

        desc_sw_write_index_ = (desc_sw_write_index_+1) == uOutBufferCnt ? 0 : (desc_sw_write_index_+1);

        MDP_INFO_QUEUE("[QUEUE]Enqueue <%s> Refill 1. Index = %d desc_remain = %d (r,w)=(%d,%d)\n",
                this->name_str(), (int)desc_sw_write_index_, (int)desc_remain_count , (int)hw_read_index, (int)hw_write_index );
    } else
    {
        MDP_ERROR("[QUEUE]Enqueue <%s> failed. Index = %d desc_remain = %d (r,w)=(%d,%d)\n",
                this->name_str(), (int)desc_sw_write_index_ , (int)desc_remain_count,  (int)hw_read_index, (int)hw_write_index );
    }


    return ret_val;


}

int ROTDMA_I::DescQueueRefillEx( void ) /*Equel to En Queue Buffer in 73*/
{
    unsigned long   reg_base;
    unsigned long   reg_val;
    unsigned long   reg_val2;
    int             b_empty;
    unsigned long   hw_read_index,hw_r_wrap;  /*RPT*/
    unsigned long   hw_write_index,hw_w_wrap; /*WPT*/
    unsigned long   desc_remain_count;
    int             ret_val = 0;

    reg_base = this->reg_base_addr();

    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    MDP_DEBUG("[ZSD] RefillEX sw_index %d 0x%x\n", (int)desc_sw_write_index_,(unsigned int)dst_img_yuv_adapt_addr_last_frame_[desc_sw_write_index_].y );
    //Fill descriptor
    ret_val = _DescQueueRefillByIndexEx( desc_sw_write_index_, reg_base );


    //Query  remain descriptor
    reg_val  = MT6575_ROT_DMA_QUEUE_RSTA( reg_base );
    reg_val2 = MT6575_ROT_DMA_QUEUE_WSTA( reg_base );

    b_empty         = ( reg_val & ( 0x1 << 2 ) ) >> 2;
    hw_read_index   = ( reg_val & ( 0xF << 8 ) ) >> 8;
    hw_r_wrap       = ( reg_val & ( 0x1 << 12 ) ) >> 12;

    hw_write_index  = ( reg_val2& ( 0xF << 8 ) ) >> 8;
    hw_w_wrap       = ( reg_val2 & ( 0x1 << 12 ) ) >> 12;


    if( b_empty== 1 ){
        desc_remain_count = 0;
    } else
    {
        if( hw_r_wrap == hw_w_wrap ){
            desc_remain_count = hw_write_index - hw_read_index;
        } else  {
            desc_remain_count = uOutBufferCnt - hw_read_index + hw_write_index;
        }

    #if 0
        desc_remain_count = (hw_read_index >= hw_write_index) ?
                            ( uOutBufferCnt - hw_read_index + hw_write_index ) : ( hw_write_index - hw_read_index );

    #endif
    }


    if( ret_val == 0 )
    {
        /*Update SW write index*/

        desc_sw_write_index_ = (desc_sw_write_index_+1) == uOutBufferCnt ? 0 : (desc_sw_write_index_+1);

        MDP_INFO_QUEUE("[QUEUE]Enqueue <%s> Refill 1. Index = %d desc_remain = %d (r,w)=(%d,%d)\n",
                this->name_str(), (int)desc_sw_write_index_, (int)desc_remain_count , (int)hw_read_index, (int)hw_write_index );
    } else
    {
        MDP_ERROR("[QUEUE]Enqueue <%s> failed. Index = %d desc_remain = %d (r,w)=(%d,%d)\n",
                this->name_str(), (int)desc_sw_write_index_ , (int)desc_remain_count,  (int)hw_read_index, (int)hw_write_index );
    }


    return ret_val;


}

inline int ROTDMA_I::DescWriteWaitBusy( unsigned long reg_base )
{
    unsigned long u4TimeOut;

    u4TimeOut = 0;
    while(!(MT6575_ROT_DMA_QUEUE_WSTA(reg_base) & 0x1))
    {
        u4TimeOut += 1;
        //if(u4TimeOut  > 100000)
        if(u4TimeOut  > 0xFFFFFF)
        {
            return -1;
        }
    }

    return 0;
}

int ROTDMA_I::DescQueueWaitEmpty( void )
{
    unsigned long   reg_base;
    unsigned long   reg_val;
    int             b_empty;
    unsigned long   hw_read_index; /*RPT*/

    reg_base = this->reg_base_addr();

    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    do
    {
        reg_val = MT6575_ROT_DMA_QUEUE_RSTA( reg_base );
        b_empty         = ( reg_val & ( 0x1 << 2 ) ) >> 2;
        hw_read_index   = ( reg_val & ( 0xF << 8 ) ) >> 8;

    } while( b_empty == 0 );
    MDP_INFO_QUEUE("\t<%s> wait empty(%d), desc hw_read_index= %lu\n", this->name_str(), b_empty, hw_read_index );


    return 0;


}



int ROTDMA_I::_DescQueueRefillByIndex( unsigned long index , unsigned long reg_base )
{

    #define _CHECK_Q_BUSY_AND_RETURN_( _seg_name_ ) \
        if( DescWriteWaitBusy( reg_base ) < 0 ){\
            MDP_ERROR("%s put " _seg_name_ " command queue timeout\n",this->name_str());\
            return -1; }

    /*Fill descriptor command*/
    /*[SEG1]*/
    _CHECK_Q_BUSY_AND_RETURN_("[SEG1]");
    MT6575_ROT_DMA_QUEUE_DATA(reg_base) = dst_img_yuv_adapt_addr[index].y + desc_y_frame_start_in_byte_;

    if((MID_VDO_ROT0 == this->id()) || (MID_VDO_ROT1 == this->id()))
    {
        /*[SEG2]*/
        _CHECK_Q_BUSY_AND_RETURN_("[SEG2]");
        MT6575_ROT_DMA_QUEUE_DATA(reg_base) = dst_img_yuv_adapt_addr[index].u + desc_uv_frame_start_in_byte_;

        /*[SEG3]*/
        _CHECK_Q_BUSY_AND_RETURN_("[SEG3]");
        MT6575_ROT_DMA_QUEUE_DATA(reg_base) = dst_img_yuv_adapt_addr[index].v + desc_uv_frame_start_in_byte_;
    }

    if( b_is_zero_shutter_encode_port_ )
    {
        /*[SEG4]*/
        _CHECK_Q_BUSY_AND_RETURN_("[SEG4]");
        MT6575_ROT_DMA_QUEUE_DATA(reg_base) = desc_src_img_roi_.w + ((unsigned long)desc_src_img_roi_.h << 16);

        /*[SEG5]*/
        _CHECK_Q_BUSY_AND_RETURN_("[SEG5]");
        MT6575_ROT_DMA_QUEUE_DATA(reg_base) = desc_src_img_roi_.w + ((unsigned long)desc_src_img_roi_.h << 16);

        /*[SEG6]*/
        _CHECK_Q_BUSY_AND_RETURN_("[SEG6]");
        MT6575_ROT_DMA_QUEUE_DATA(reg_base) = 0;

        /*[SEG7]*/
        _CHECK_Q_BUSY_AND_RETURN_("[SEG7]");
        if( desc_uv_stride_align_ != 0 ){
            MT6575_ROT_DMA_QUEUE_DATA(reg_base)  =  ( ( desc_y_stride_ * desc_byte_per_pixel_ ) & 0x0003FFFF ) |
                                                    ( ( desc_uv_stride_ * desc_byte_per_pixel_ ) << 18 );

        }else  {
            MT6575_ROT_DMA_QUEUE_DATA(reg_base) = ( desc_y_stride_ * desc_byte_per_pixel_ ) & 0x0003FFFF ;
        }

        if( this->id() & ( MID_RGB_ROT0 | MID_RGB_ROT1 | MID_RGB_ROT2 ) )
        {

            /*[SEG8]*/
            _CHECK_Q_BUSY_AND_RETURN_("[SEG8]");
            MT6575_ROT_DMA_QUEUE_DATA(reg_base) = (unsigned long)desc_src_img_roi_.w * desc_byte_per_pixel_;

            /*[SEG9]*/
            _CHECK_Q_BUSY_AND_RETURN_("[SEG9]");
            MT6575_ROT_DMA_QUEUE_DATA(reg_base) = (unsigned long)desc_src_img_roi_.h * desc_byte_per_pixel_;
        }


    }

    return 0;

}

int ROTDMA_I::_DescQueueRefillByIndexEx( unsigned long index , unsigned long reg_base )
{

    #define _CHECK_Q_BUSY_AND_RETURN_( _seg_name_ ) \
        if( DescWriteWaitBusy( reg_base ) < 0 ){\
            MDP_ERROR("%s put " _seg_name_ " command queue timeout\n",this->name_str());\
            return -1; }

    /*Fill descriptor command*/
    /*[SEG1]*/
    _CHECK_Q_BUSY_AND_RETURN_("[SEG1]");
    MT6575_ROT_DMA_QUEUE_DATA(reg_base) = dst_img_yuv_adapt_addr_last_frame_[index].y;

    /*[SEG4]*/
    _CHECK_Q_BUSY_AND_RETURN_("[SEG4]");
    MT6575_ROT_DMA_QUEUE_DATA(reg_base) = desc_src_img_roi_.w + ((unsigned long)desc_src_img_roi_.h << 16);

    /*[SEG5]*/
    _CHECK_Q_BUSY_AND_RETURN_("[SEG5]");
    MT6575_ROT_DMA_QUEUE_DATA(reg_base) = desc_src_img_roi_.w + ((unsigned long)desc_src_img_roi_.h << 16);

    /*[SEG6]*/
    _CHECK_Q_BUSY_AND_RETURN_("[SEG6]");
    MT6575_ROT_DMA_QUEUE_DATA(reg_base) = 0;

    /*[SEG7]*/
    _CHECK_Q_BUSY_AND_RETURN_("[SEG7]");
    MT6575_ROT_DMA_QUEUE_DATA(reg_base) = ( desc_y_stride_ * desc_byte_per_pixel_ ) & 0x0003FFFF ;


    /*[SEG8]*/
    _CHECK_Q_BUSY_AND_RETURN_("[SEG8]");
    MT6575_ROT_DMA_QUEUE_DATA(reg_base) = (unsigned long)desc_src_img_roi_.w * desc_byte_per_pixel_;

    /*[SEG9]*/
    _CHECK_Q_BUSY_AND_RETURN_("[SEG9]");
    MT6575_ROT_DMA_QUEUE_DATA(reg_base) = (unsigned long)desc_src_img_roi_.h * desc_byte_per_pixel_;


    return 0;

}
int ROTDMA_I::ConfigZsd_DescUpdate( RotdmaZsdZoomParam param )
{

    //----
    //Do not recalculate buffer address,this may multibuffer address is not continous
    //----
    //DoWhenOutputSizeChange(param.desc_dst_img_size, param.desc_src_img_roi);


    desc_src_img_roi_ = param.desc_src_img_roi;    //image roi before rotate
    desc_dst_img_size_ = param.desc_dst_img_size;    //Size before rotate (image stride)
    desc_y_stride_ = param.desc_y_stride;
    desc_uv_stride_ = param.desc_uv_stride;
    desc_y_frame_start_in_byte_ = param.desc_y_frame_start_in_byte;
    desc_uv_frame_start_in_byte_ = param.desc_uv_frame_start_in_byte;

    return 0;
}

int ROTDMA_I::ConfigZsdZoom_DescUpdate( RotdmaZsdZoomParam param )
{

    //----
    //Do not recalculate buffer address,this may multibuffer address is not continous
    //----
    //DoWhenOutputSizeChange(param.desc_dst_img_size, param.desc_src_img_roi);

    desc_src_img_roi_ = param.desc_src_img_roi;    //image roi before rotate
    desc_dst_img_size_ = param.desc_dst_img_size;    //Size before rotate (image stride)
    desc_y_stride_ = param.desc_y_stride;
    desc_uv_stride_ = param.desc_uv_stride;
    desc_y_frame_start_in_byte_ = param.desc_y_frame_start_in_byte;
    desc_uv_frame_start_in_byte_ = param.desc_uv_frame_start_in_byte;

    return 0;
}


int ROTDMA_I::CalcFrameStartAddress( CalcFrameStartAddress_In* p_in, CalcFrameStartAddress_Out* p_out )
{

    unsigned long   rotate              =   p_in->rotate;             /*rotation w/o flip. 0-3*/
    MdpRect         src_img_roi         =   p_in->src_img_roi;        /*src roi image before rotation*/
    MdpSize         dst_img_size        =   p_in->dst_img_size;       /*stride before rotation*/
    MdpColorFormat  dst_color_format    =   p_in->dst_color_format;
    int             b_is_generic_yuv    =   p_in->b_is_generic_yuv;
    unsigned long   byte_per_pixel      =   p_in->byte_per_pixel;
    /*y,u,v sampling period*/
    unsigned long   yv      =   p_in->yv;
    unsigned long   yh      =   p_in->yh;
    unsigned long   uvv     =   p_in->uvv;
    unsigned long   uvh     =   p_in->uvh;


    p_out->y_stride = (rotate & 0x1)?dst_img_size.h : dst_img_size.w;

    if( b_is_generic_yuv )
    {
        p_out->uv_stride = p_out->y_stride >> (uvh ? (uvh - 1) : 0);

        MDP_ROUND_UP( p_out->uv_stride, p_in->uv_stride_align );//Round uv stride to 16x align

    } else
    {
        p_out->uv_stride = 0;
    }


    if((2 == rotate) || (3 == rotate))
    {
        if(b_is_generic_yuv)//Planar
        {
            /*Y*/
            p_out->y_frame_start_in_byte = ((unsigned long)p_out->y_stride * byte_per_pixel >> (yh ? (yh - 1) : 0));
            p_out->y_frame_start_in_byte *= (2 == rotate ?
                    (((unsigned long)src_img_roi.h >> (yv ? (yv - 1) : 0) ) - 1 ) :
                    (((unsigned long)src_img_roi.w >> (yv ? (yv - 1) : 0) ) - 1 ));
            /*UV*/
            p_out->uv_frame_start_in_byte = (unsigned long) p_out->uv_stride * byte_per_pixel;
            p_out->uv_frame_start_in_byte *= (2 == rotate ?
                    (((unsigned long)src_img_roi.h >> (uvv ? (uvv - 1) : 0) ) - 1 ) :
                    (((unsigned long)src_img_roi.w >> (uvv ? (uvv - 1) : 0) ) - 1 ));
        }
        else
        {
            /*
                For non generic YUV format:
                180 degree (both flip and non-flip) : FRM_S_IN_BYTE = DST_W_IN_BYTE * (CLIP_H-1)
                270 degree (both flip and non-flip) : FRM_S_IN_BYTE = DST_W_IN_BYTE * (CLIP_W-1)
            */
            p_out->y_frame_start_in_byte = (unsigned long)p_out->y_stride * byte_per_pixel;
            p_out->y_frame_start_in_byte *= (2 == rotate ? (unsigned long)(src_img_roi.h - 1) : (unsigned long)(src_img_roi.w - 1));
            p_out->uv_frame_start_in_byte = 0;
        }
    }
    else
    {
        p_out->y_frame_start_in_byte = 0;
        p_out->uv_frame_start_in_byte = 0;
    }


    /*Add ROI X,Y offset*/
    /*Clouds:
        I think this piece of code is meaningless, because src_img_roi.x & src_img_roi.y is always set to 0 by upper layer,
        the destination offset is control by the destination starting address which is calculated by upper layer
     */
    p_out->y_frame_start_in_byte += ((unsigned long) p_out->y_stride * src_img_roi.y + src_img_roi.x)*byte_per_pixel;
    if(b_is_generic_yuv)
    {
        p_out->uv_frame_start_in_byte += (((unsigned long) p_out->uv_stride * src_img_roi.y + src_img_roi.x)*byte_per_pixel >> ( uvv - 1));
    }


    return 0;
}

int ROTDMA_I::CalculateBufferSizeAndYuvOffset( MdpSize Dst_Img_Size, MdpRect Src_Img_Roi )
{
    int i;

    /*-----------------------------------------------------------------------------
        Calculate buffer size and uv address
      -----------------------------------------------------------------------------*/
    dst_img_buffer_size_total_ = 0;

    for ( i = 0 ; i < uOutBufferCnt; i++ )
    {
        MDP_INFO("Buffer %d/%d:\n", (int)i+1,(int)uOutBufferCnt );

        /*.............................................................................
            Use address[0] as the start address of these sequence of buffer.
            thus, address[1] = address[0] + address[0].size

            Skip the address[0], it is given by user.
          .............................................................................*/
        if( i != 0 )
        {
            dst_img_yuv_addr[i].y = dst_img_yuv_addr[i-1].y +
                                    dst_img_yuv_addr[i-1].y_buffer_size +
                                    dst_img_yuv_addr[i-1].u_buffer_size +
                                    dst_img_yuv_addr[i-1].v_buffer_size;
        }
        /*.............................................................................
            Here calculate the u,v address and buffer size base on y address
          .............................................................................*/
        if( MdpDrv_CalImgBufferSizeAndFillIn(
                        dst_color_format,
                        &(dst_img_yuv_addr[i]),
                        Dst_Img_Size,
                        Src_Img_Roi,
                        ((bFlip << 2) | bRotate ) ) != 0 )
        {
            return -1;
        }


        dst_img_buffer_size_total_ +=   dst_img_yuv_addr[i].y_buffer_size +
                                        dst_img_yuv_addr[i].u_buffer_size +
                                        dst_img_yuv_addr[i].v_buffer_size;
    }

    return 0;

}


int ROTDMA_I::DoWhenOutputSizeChange( MdpSize Dst_Img_Size, MdpRect Src_Img_Roi )
{

    MDP_INFO("\t<%s> Output size change. Buffer size and YUV offset recalculate...................\n",this->name_str());

    /*UnAdapt MdpYuvAddress*/
    MdpDrv_UnAdaptMdpYuvAddrArray(  this->id(),
                                    dst_img_yuv_addr, uOutBufferCnt,     //Original user input address
                                    dst_img_yuv_adapt_addr,              //Adapted address
                                    adapt_m4u_flag_bit_,                 //Corresponding bit will set to 1 if address had been adapted with m4u mva
                                    alloc_mva_flag_bit_ );              //Corresponding bit will set to 1 if the mva is new allocated

    /*recalculate output size*/
    #if 1

    if( MdpDrv_CalImgBufferArraySizeAndFillIn(
                        dst_color_format,
                        dst_img_yuv_addr,
                        uOutBufferCnt,
                        Dst_Img_Size,
                        Src_Img_Roi,
                        ((bFlip << 2) | bRotate ), /*rotate*/
                        &dst_img_buffer_size_total_ ) < 0 )
    {
        return -1;
    }
    #else
    CalculateBufferSizeAndYuvOffset(  Dst_Img_Size,  Src_Img_Roi );
    #endif



    /*-----------------------------------------------------------------------------
        Adapt dest image address
      -----------------------------------------------------------------------------*/
    if( MdpDrv_AdaptMdpYuvAddrArray(    this->id(),
                                        dst_img_yuv_addr, uOutBufferCnt,//Original user input address
                                        dst_img_yuv_adapt_addr,         //Output adapted address
                                        &adapt_m4u_flag_bit_,           //If address has been adapted with m4u mva, corresponding bit will set to 1
                                        &alloc_mva_flag_bit_ ) != 0 )
    {
        MDP_ERROR("<%s> Adapt Memory error.\n", this->name_str() );
        DumpDebugInfo();
        return -1;
    }
    MDP_INFO("\t..................................................................................\n");


    return 0;

}

int     ROTDMA_I::CalcBufferSize( void )
{

    unsigned long reg_base;
    unsigned long reg_val;

    reg_base = this->reg_base_addr();

    if( reg_base== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    /*******************************************************************************
        Wait For Andy's Code
     *******************************************************************************/

    return 0;

}


/*/////////////////////////////////////////////////////////////////////////////
    VDOROT0
  /////////////////////////////////////////////////////////////////////////////*/
int VDOROT0::_ConfigPre( void )
{
    unsigned long mmsys1_reg_base;

    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();

    MT6575_MMSYS_VDO_ROT0_ISEL( mmsys1_reg_base ) = src_sel;
    return 0;
}

int VDOROT0::_ConfigPost( void )
{
    /*Nothing*/
    return 0;
}

int VDOROT0::_EnablePre( void )
{
    /*Nothing*/
    return 0;
}

int VDOROT0::_EnablePost( void )
{
    /*Nothing*/
    return 0;
}

int VDOROT0::_DisablePre( void )
{
    /*Nothing*/
    return 0;
}

int VDOROT0::_DisablePost( void )
{
    /*Nothing*/
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    VDOROT1
  /////////////////////////////////////////////////////////////////////////////*/
int VDOROT1::_ConfigPre( void )
{
    unsigned long mmsys1_reg_base;

    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();

    MT6575_MMSYS_VDO_ROT1_ISEL(mmsys1_reg_base) = src_sel;
    return 0;
}

int VDOROT1::_ConfigPost( void )
{
    /*Nothing*/
    return 0;
}

int VDOROT1::_EnablePre( void )
{
    /*Nothing*/
    return 0;
}

int VDOROT1::_EnablePost( void )
{
    /*Nothing*/
    return 0;
}

int VDOROT1::_DisablePre( void )
{
    /*Nothing*/
    return 0;
}

int VDOROT1::_DisablePost( void )
{
    /*Nothing*/
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    RGBROT0
  /////////////////////////////////////////////////////////////////////////////*/
int RGBROT0::_ConfigPre( void )
{
    unsigned long mmsys1_reg_base;

    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();

    MT6575_MMSYS_RGB_ROT0_ISEL(mmsys1_reg_base) = src_sel;
    return 0;
}

int RGBROT0::_ConfigPost( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT0::_EnablePre( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT0::_EnablePost( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT0::_DisablePre( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT0::_DisablePost( void )
{
    /*Nothing*/
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    RGBROT1
  /////////////////////////////////////////////////////////////////////////////*/
int RGBROT1::_ConfigPre( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT1::_ConfigPost( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT1::_EnablePre( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT1::_EnablePost( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT1::_DisablePre( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT1::_DisablePost( void )
{
    /*Nothing*/
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    RGBROT2
  /////////////////////////////////////////////////////////////////////////////*/
int RGBROT2::_ConfigPre( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT2::_ConfigPost( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT2::_EnablePre( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT2::_EnablePost( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT2::_DisablePre( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT2::_DisablePost( void )
{
    /*Nothing*/
    return 0;
}


/*/////////////////////////////////////////////////////////////////////////////
    BRZ
  /////////////////////////////////////////////////////////////////////////////*/
int BRZ::_Config( void )
{

    unsigned long u4BufferCount = 1;    /*1: Single Buffer 2:Double Buffer*/
    unsigned long u4YLineBuffSize;
    unsigned long u4UVLineBuffSize;
    unsigned long u4YLineBuffCnt;
    int Hy,Vy,Hu,Vu,Hv,Vv;
    int bitHy,bitVy,bitHu,bitVu,bitHv,bitVv;

    unsigned long u4Index = 0;
    unsigned long base_addr;
    unsigned long mmsys1_reg_base;

    base_addr = this->reg_base_addr(); //virtual
    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();

    if( base_addr == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }



    if((src_img_size.h & 0x7) || (src_img_size.w & 0x7))// width and height should be 8N
    {
        MDP_ERROR("BRZ Source width height should be 8N w:%d h:%d!!\n" , (int)src_img_size.w , (int)src_img_size.h);
        return -1;
    }

    /*if((src_img_size.h > (8184 << 3)) || (src_img_size.w > (8184 << 3)))*//*Original*/
    if((src_img_size.h > (8184)) || (src_img_size.w > (8184))) /*CLS*/
    {
        MDP_ERROR("BRZ Source width height exceeds 8184*8 w:%d h:%d!!\n" , (int)src_img_size.w , (int)src_img_size.h);
        return -1;
    }

    /*CLS:uShrinkFactor only have 3 bits, the below expression is always true*/
    if((shrink_factor > 3))// shrink factor is less than 3
    {
        MDP_ERROR("BRZ Source width height exceeds 8191 w:%d h:%d!!\n" , (int)src_img_size.w , (int)src_img_size.h);
        return -1;
    }

    //Reset BRZ
    #if 1
    Reset();
    #else
    MT6575_BRZ_CON( base_addr ) = (1 << 16);
    MT6575_BRZ_CON( base_addr ) = 0;
    MT6575_MMSYS_BRZ_MOUT_CLR( mmsys1_reg_base ) = 1;
    MT6575_MMSYS_BRZ_MOUT_CLR( mmsys1_reg_base ) = 0;
    #endif

    #if 0
    MT6575_BRZ_INT( base_addr ) = 0x3;
    #else
    MT6575_BRZ_INT( base_addr ) = 0x0;
    #endif

    switch( input_format )
    {
        case 444 :
      /// YUV 4:4:4 (Hy=1, Vy=1), (Hu=1, Vu=1), (Hv=1, Vv=1)
            Hy=1;   Vy=1;   Hu=1;   Vu=1;   Hv=1;   Vv=1;
            bitHy=0;bitVy=0;bitHu=0;bitVu=0;bitHv=0;bitVv=0;
        break;
        case 422 :// 2x1
      /// YUV 4:2:2 (Hy=2, Vy=1), (Hu=1, Vu=1), (Hv=1, Vv=1)
            Hy=2;   Vy=1;   Hu=1;   Vu=1;   Hv=1;   Vv=1;
            bitHy=1;bitVy=0;bitHu=0;bitVu=0;bitHv=0;bitVv=0;
        break;
        case 421 :// 1x2
      /// YUV 4:2:2 (Hy=1, Vy=2), (Hu=1, Vu=1), (Hv=1, Vv=1)
            Hy=1;   Vy=2;   Hu=1;   Vu=1;   Hv=1;   Vv=1;
            bitHy=0;bitVy=1;bitHu=0;bitVu=0;bitHv=0;bitVv=0;
        break;
        case 420 : // 2x2
      /// YUV 4:2:0 (Hy=2, Vy=2), (Hu=1, Vu=1), (Hv=1, Vv=1)
            Hy=2;   Vy=2;   Hu=1;   Vu=1;   Hv=1;   Vv=1;
            bitHy=1;bitVy=1;bitHu=0;bitVu=0;bitHv=0;bitVv=0;
        break;
        case 411 : // 4x1
      /// YUV 4:1:1 (Hy=4, Vy=1), (Hu=1, Vu=1), (Hv=1, Vv=1)
            Hy=4;   Vy=1;   Hu=1;   Vu=1;   Hv=1;   Vv=1;
            bitHy=2;bitVy=0;bitHu=0;bitVu=0;bitHv=0;bitVv=0;
        break;
        case 400 ://
      /// YUV 4:0:0 (Hy=1, Vy=1), (Hu=0, Vu=0), (Hv=0, Vv=0)
            Hy=1;   Vy=1;   Hu=0;   Vu=0;   Hv=0;   Vv=0;
            bitHy=0;bitVy=0;bitHu=3;bitVu=3;bitHv=3;bitVv=3;
        break;

        default:
            MDP_ERROR("BRZ unsupported format\n");
            return -1;
    }

    //Calculate strip buffer size
    u4YLineBuffCnt =   u4BufferCount * ( Vy*8 >> shrink_factor );
    u4YLineBuffSize =  ( (src_img_size.w*Hy)/MDP_MAX3(Hy, Hu, Hv) >> shrink_factor) * u4YLineBuffCnt;
    u4UVLineBuffSize = ( (src_img_size.w*Hu)/MDP_MAX3(Hy, Hu, Hv) >> shrink_factor) * ( Vu*8 >> shrink_factor )
                       * u4BufferCount;


    MT6575_BRZ_BLKSCSFG( base_addr ) =  ( bitHy << 4 )  | ( bitVy << 6  ) |
                                        ( bitHu << 8 )  | ( bitVu << 10 ) |
                                        ( bitHv << 12 ) | ( bitVv << 14 ) |
                                        (unsigned long)shrink_factor;

    #define _ALIGN_TO_8B( _x_ ) \
    if( (_x_ & 0x7) > 0 ) {\
        unsigned long _old_size;\
        _old_size = _x_;\
        _x_ = (_x_ + 0x7)&0x7;\
        MDP_WARNING("BRZ sysram size 8-byte alignment. %s = 0x%08X => 0x%08X\n",#_x_, (unsigned int)_old_size ,(unsigned int)_x_ );\
    }

    _ALIGN_TO_8B(u4YLineBuffSize);
    _ALIGN_TO_8B(u4UVLineBuffSize);


    sysram_.size = (u4YLineBuffSize + 2*u4UVLineBuffSize);
    sysram_.address = MdpSysram_Alloc( this->id(), sysram_.size , MDPSYSRAM_SUBCAT_NORMAL/*0*/ );

    if( 0 == sysram_.address )
    {
        if( u4BufferCount > 1 )
        {
            u4YLineBuffCnt = (u4YLineBuffCnt >> 1);
            u4YLineBuffSize = (u4YLineBuffSize >> 1);
            u4UVLineBuffSize = (u4UVLineBuffSize >> 1);

            _ALIGN_TO_8B(u4YLineBuffSize);
            _ALIGN_TO_8B(u4UVLineBuffSize);

            sysram_.size = (u4YLineBuffSize + 2*u4UVLineBuffSize);
            sysram_.address = MdpSysram_Alloc( this->id(), sysram_.size , MDPSYSRAM_SUBCAT_NORMAL/*0*/ );
        }
        if(0 == sysram_.address)
        {
            sysram_.size = 0;
            MDP_ERROR("SYSRAM Request failed. BRZ request sysram :%d @ CSF=%d\n",(int)(u4YLineBuffSize + 2*u4UVLineBuffSize),(int)shrink_factor);
            DumpDebugInfo();
            return -1;
        }
    }

    if( ( sysram_.address & 0x7 ) > 0 )
    {
        MDP_ERROR("BRZ sysram_.address = 0x%08X is not 8-B aligned\n", (unsigned int)sysram_.address );
        return -1;
    }


    MT6575_BRZ_YLMBASE( base_addr ) = sysram_.address;

    MT6575_BRZ_ULMBASE( base_addr ) = (sysram_.address + u4YLineBuffSize);

    MT6575_BRZ_VLMBASE( base_addr ) = (sysram_.address + u4YLineBuffSize + u4UVLineBuffSize);

    MT6575_BRZ_YLBSIZE( base_addr ) = u4YLineBuffCnt;

    MT6575_BRZ_SRCSZ( base_addr ) = (((unsigned long)src_img_size.h << 16) + src_img_size.w);

    MT6575_MMSYS_BRZ_MOUT_SEL( mmsys1_reg_base ) = (to_vrz1 | (to_prz0 << 1) | (to_vrz0 << 2) | (to_cam << 3));

    return 0;

}

int BRZ::_Enable( void )
{
    unsigned long base_addr;
    unsigned long mmsys1_reg_base;

    base_addr = this->reg_base_addr(); //virtual
    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();

    if( base_addr == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    MT6575_MMSYS_BRZ_MOUT_EN( mmsys1_reg_base ) = 1;
    MT6575_BRZ_CON( base_addr ) = 1;

    return 0;
}

int BRZ::_Disable( void )
{
    unsigned long busy_wait = 0xFFFF;
    unsigned long base_addr;
    unsigned long mmsys1_reg_base;

    base_addr = this->reg_base_addr(); //virtual
    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();


    if( base_addr == 0 )
    {
        return -1;
    }


    MT6575_BRZ_CON( base_addr ) = 0;

    while(MT6575_BRZ_STA( base_addr ))
    {
        busy_wait--;

        if( busy_wait == 0 )
        {
            //MDP_WARNING("BRZ Busy wait time out!\n");
            break;
        }
    }

    MT6575_MMSYS_BRZ_MOUT_EN( mmsys1_reg_base ) = 0;

    Reset();

    /*Free_Sysram( MID_BRZ );*/
    MdpSysram_Free( this->id(), sysram_.address , MDPSYSRAM_SUBCAT_NORMAL/*0*/ );
    sysram_.address = 0;
    sysram_.size = 0;


    return 0;
}

int BRZ::CheckBusy( unsigned long* param )
{

    unsigned long base_addr;

    base_addr = this->reg_base_addr(); //virtual


    if( base_addr == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    return (MT6575_BRZ_STA( base_addr ) > 0 ? 1 : 0);

}

int BRZ::Reset( void )
{
    unsigned long base_addr;
    unsigned long mmsys1_reg_base;


    base_addr = this->reg_base_addr(); //virtual
    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();

    if( base_addr == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    MT6575_BRZ_CON( base_addr ) = (1 << 16);
    MT6575_BRZ_CON( base_addr ) = 0;
    MT6575_MMSYS_BRZ_MOUT_CLR( mmsys1_reg_base ) = 1;
    MT6575_MMSYS_BRZ_MOUT_CLR( mmsys1_reg_base ) = 0;

    return 0;
}

int BRZ::HardReset( void )
{
    return Reset(); //Warm reset only
}

int BRZ::DumpRegisterCustom( int mode )
{

    unsigned long reg_base;
    unsigned long reg_val;

    reg_base = this->reg_base_addr();


    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    //MT6575_BRZ_STA
    reg_val = MT6575_BRZ_STA(reg_base);
    MDP_PRINTF("BUSY:Coarse Shrink=%d\n",reg_val&(0x1<<0)?1:0);
    MDP_PRINTF("BUSY:Intra Block=%d\n",reg_val&(0x1<<1)?1:0);
    MDP_PRINTF("BUSY:Block to Raster Scan Out=%d\n",reg_val&(0x1<<2)?1:0);

    //MT6575_BRZ_SRCSZ
    reg_val = MT6575_BRZ_SRCSZ(reg_base);
    MDP_PRINTF("SRC_W=%d\n",(int)(reg_val&0x0000FFFF));
    MDP_PRINTF("SRC_H=%d\n",(int)((reg_val&0xFFFF0000)>>16));

    return 0;
}

int BRZ::DumpDebugInfo( void )
{
    int i;

    MDPELEMENT_I::DumpDebugInfo();

    MDP_DUMP_VAR_D(src_img_size.w);
    MDP_DUMP_VAR_D(src_img_size.h);

    MDP_DUMP_VAR_D(shrink_factor);
    MDP_DUMP_VAR_D(input_format);

    MDP_DUMP_VAR_D(to_vrz1);
    MDP_DUMP_VAR_D(to_prz0);
    MDP_DUMP_VAR_D(to_vrz0);
    MDP_DUMP_VAR_D(to_cam);



    return 0;
}


/*/////////////////////////////////////////////////////////////////////////////
    JPEGDMA
  /////////////////////////////////////////////////////////////////////////////*/
int JPEGDMA::_Config( void )
{
    #define CEIL(a,b) (((unsigned long)a + (unsigned long)b)&(~(unsigned long)b))
    unsigned long u4LineBuffAddr = 0;
    unsigned long u4YLineBuffSize = 0;
    unsigned long u4UVLineBuffSize = 0;

    unsigned long u4Index = 0;
    unsigned long base_addr;
    unsigned long mmsys1_reg_base;

    base_addr = this->reg_base_addr(); //virtual
    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();

    if( base_addr == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    /*In MT6575, no limitation of input size with 8x, JPEGDMA engine will handle this */
    if((1 > src_img_size.w) ||(1 > src_img_size.h) )
    {
        MDP_ERROR("Config JPGDMA failed -- image dimension is incorrect w:%d,h:%d" , (int)src_img_size.w , (int)src_img_size.h);
        return -1;
    }

    switch( output_format )
    {
        case 422 :
            u4YLineBuffSize = CEIL(CEIL(src_img_size.w , 15) , 63)*8;
            u4UVLineBuffSize = u4YLineBuffSize/2;
        break;
        case 420 :
            u4YLineBuffSize = CEIL(CEIL(src_img_size.w , 15) , 127)*16;
            u4UVLineBuffSize = CEIL(CEIL(src_img_size.w , 15)/2 , 63)*8;
        break;
        case 400 :
            u4YLineBuffSize = CEIL(CEIL(src_img_size.w , 7) , 63)*8;
            u4UVLineBuffSize = 0;
        break;
        case 411 :
            u4YLineBuffSize = CEIL(CEIL(src_img_size.w , 31) , 63)*8;
            u4UVLineBuffSize = CEIL(CEIL(src_img_size.w , 31)/4 , 63)*8;;
        break;
        default :
            MDP_ERROR("Config JPGDMA failed -- Unsupport format\n");
            return -1;
        break;
    }

    u4YLineBuffSize = ((u4YLineBuffSize*81) >> 6);
    u4UVLineBuffSize = ((u4UVLineBuffSize*81) >> 6);

    sysram_.size = (u4YLineBuffSize + 2*u4UVLineBuffSize);
    sysram_.address = MdpSysram_Alloc( this->id(), sysram_.size , MDPSYSRAM_SUBCAT_NORMAL/*0*/ );

    if( sysram_.address == 0 )
    {
        sysram_.size == 0;
        MDP_ERROR("SYSRAM allocate failed!\n");
        DumpDebugInfo();
        return -1;
    }


    #if 1
    Reset();
    #else
    MT6575_JPEG_DMA_RESET(base_addr) = 0x2;

    MT6575_JPEG_DMA_RESET(base_addr) = 0;
    #endif


    MT6575_JPEG_DMA_BUF_BASE_ADDR0(base_addr) = sysram_.address;

    MT6575_JPEG_DMA_BUF_BASE_ADDR1(base_addr) = sysram_.address;

    MT6575_JPEG_DMA_CON(base_addr) = ((1 << 8) | ((unsigned long)bCamIn << 7) | ((unsigned long)bContinuous << 3) | 1);

    MT6575_JPEG_DMA_SIZE(base_addr) = (((unsigned long)src_img_size.h << 16) | src_img_size.w);

    MT6575_MMSYS_JPEG_DMA_ISEL(mmsys1_reg_base) = src_sel;


    return 0;

}

int JPEGDMA::_Enable( void )
{
    unsigned long base_addr;

    base_addr = this->reg_base_addr(); //virtual


    if( base_addr == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    MT6575_JPEG_DMA_EN( base_addr ) = 1;

    return 0;
}

int JPEGDMA::_Disable( void )
{
    unsigned long base_addr;

    base_addr = this->reg_base_addr(); //virtual


    if( base_addr == 0 )
    {
        return -1;
    }

    MT6575_JPEG_DMA_STOP(base_addr) = 1;
    MT6575_JPEG_DMA_STOP(base_addr) = 0;

    Reset();

    /*Free_Sysram( MID_JPEG_DMA );*/
    MdpSysram_Free( this->id(), sysram_.address , MDPSYSRAM_SUBCAT_NORMAL/*0*/ );
    sysram_.address = 0;
    sysram_.size = 0;



    return 0;
}

int JPEGDMA::CheckBusy( unsigned long* param )
{

    unsigned long u4Result = 0;
    unsigned long base_addr;

    base_addr = this->reg_base_addr(); //virtual

    if( base_addr== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    u4Result = MT6575_JPEG_DMA_INTERRUPT( base_addr );

    if(u4Result & 0x1)
    {
        MT6575_JPEG_DMA_INTERRUPT( base_addr ) = 0;
    }

    return (u4Result > 0 ? 0 : 1);

}

int JPEGDMA::Reset( void )
{
    unsigned long reg_base;

    reg_base = this->reg_base_addr();

    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    MT6575_JPEG_DMA_RESET(reg_base) = 0x2;

    MT6575_JPEG_DMA_RESET(reg_base) = 0;

    return 0;
}

int JPEGDMA::HardReset( void )
{
    volatile unsigned long time_delay;
    unsigned long reg_base;

    reg_base = this->reg_base_addr();

    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

        /*Hard Reset*/
    MT6575_JPEG_DMA_RESET( reg_base ) = 0x1;
    for( time_delay = 0 ; time_delay < 50 ; time_delay += 1){;}//Delay 50 cycle
    MT6575_JPEG_DMA_RESET( reg_base ) = 0x0;

    MDP_WARNING("<%s> Hard Reset!\n",this->name_str());


    return 0;
}




int JPEGDMA::DumpRegisterCustom( int mode )
{
    unsigned long reg_base;
    unsigned long reg_val;

    reg_base = this->reg_base_addr();


    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    //MT6575_JPEG_DMA_CON
    reg_val = MT6575_JPEG_DMA_CON(reg_base);
    MDP_PRINTF("FIFO_MODE=%d\n",reg_val&(0x1<<8)?1:0);
    MDP_PRINTF("FRAME_SYNC_EN=%d\n",reg_val&(0x1<<7)?1:0);
    MDP_PRINTF("AUTO RSTR(Cont. mode)=%d\n",reg_val&(0x1<<3)?1:0);


    //MT6575_JPEG_DMA_SIZE
    reg_val = MT6575_JPEG_DMA_SIZE(reg_base);
    MDP_PRINTF("SRC_W=%d\n",(int)(reg_val&0x0000FFFF));
    MDP_PRINTF("SRC_H=%d\n",(int)((reg_val&0xFFFF0000)>>16));

    //MT6575_JPEG_DMA_DEBUG_STATUS0
    reg_val = MT6575_JPEG_DMA_DEBUG_STATUS0(reg_base);
    MDP_PRINTF("RX_XCNT=%d RX_YCNT=%d\n",(int)(reg_val&0x0000FFFF) , (int)((reg_val>>16)&0x0000FFFF));


    return 0;
}


int JPEGDMA::DumpDebugInfo( void )
{
    int i;

    MDPELEMENT_I::DumpDebugInfo();

    MDP_DUMP_VAR_D(src_img_size.w);
    MDP_DUMP_VAR_D(src_img_size.h);

    MDP_DUMP_VAR_D(output_format);
    MDP_DUMP_VAR_D(bContinuous);

    MDP_DUMP_VAR_D(bCamIn);
    MDP_DUMP_VAR_D(src_sel);
    MDP_DUMP_VAR_D(sysram_.address);
    MDP_DUMP_VAR_D(sysram_.size);



    return 0;
}



/*/////////////////////////////////////////////////////////////////////////////
    ROTDMAEX_I
  /////////////////////////////////////////////////////////////////////////////*/

int ROTDMAEX_I::_Config( void )
{
    int i;
    unsigned long mmsys1_reg_base;
    unsigned long reg_base;
    unsigned long descript_base = 0;

    unsigned long u4Index = 0;
    //unsigned long u4BytePerPixel = 0;
   //unsigned long u4IsGenericYUV = 0;// 1 if planar YUV
    //unsigned long u4yh = 0;// horizontal y sample factor
    //unsigned long u4yv = 0;// vertical y sample factor
    //unsigned long u4uvh = 0;// horizontal uv sample factor
    //unsigned long u4uvv = 0;// vertical uv sample factor
    MdpDrvColorInfo_t  ci;
    unsigned long u4YFrame_Start_In_Byte = 0;
    unsigned long u4UVFrame_Start_In_Byte = 0;

    unsigned long y_stride, uv_stride;

    stROTMDALineBuffSetting LBSetting;
    stLineBuffCalculation stBuffCal;
    SYSRAM_INFO stSysram;
    unsigned long rot_id;




    rot_id = this->id(); //virtual
    mmsys1_reg_base = MDPELEMENT_I::mmsys1_reg_base_addr();
    reg_base = this->reg_base_addr();           //virtual
    descript_base = this->descript_base_addr(); //virtual


    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    if((uOutBufferCnt > MT6575ROTDMA_MAX_RINGBUFFER_CNT) || (0 == uOutBufferCnt))
    {
        MDP_ERROR("rotdma buffer count exceeds upper limit or 0 %lu\n" , uOutBufferCnt);
        return -1;
    }

    if((dst_color_format <= RGBA8888) && ((MID_VDO_ROT0 == rot_id) || (MID_VDO_ROT1 == rot_id)))
    {
        MDP_ERROR("VDO_ROT does not support RGB output\n");
        return -1;
    }

    if((dst_color_format >= YV16_Planar) && ((MID_RGB_ROT0 == rot_id) || (MID_RGB_ROT1 == rot_id) || (MID_RGB_ROT2 == rot_id)))
    {
        MDP_ERROR("RGB_ROT does not support planar output\n");
        return -1;
    }

    if((dst_color_format == RAW) && (MID_RGB_ROT0 != rot_id) && (MID_VDO_ROT0 != rot_id))
    {
        MDP_ERROR("Only RGBROT0 and VDOROT0 support raw format output\n");
        return -1;
    }

    if((dst_color_format != RGB565) && (dst_color_format != RGB565) && (MID_TV_ROT == rot_id))
    {
        MDP_ERROR("TV ROT only support RGB565 and UYVY format\n");
        return -1;
    }

    // YUV pack format, width must be 2xN
    if(((YUY2_Pack == dst_color_format) || (UYVY_Pack == dst_color_format)) && (dst_img_roi.w & 0x1))
    {
        MDP_ERROR("Config ROTDMA failed -- YUV pack format, width must be 2N : %lu" , dst_img_roi.w);
        return -1;
    }


    //Android YV12, stride should be 16x align
    if( dst_color_format == ANDROID_YV12 )  {
        if( (bRotate & 0x01) == 0 )     { //0,180
            if( MDP_IS_ALIGN( dst_img_size.w, 4 ) == 0 )    { //Check if 16x align width
                MDP_ERROR("Width of ANDROID_YV12must be 16x. dst_img_size.w = %lu\n", dst_img_size.w );
                return -1;
            }
        } else  {//90,270
            if( MDP_IS_ALIGN( dst_img_size.h, 4 ) == 0 )    { //Check if 16x align width
                MDP_ERROR("Width of ANDROID_YV12must be 16x. dst_img_size.h (after rotate) = %lu\n", dst_img_size.h );
                return -1;
            }
        }
    }


    if( dst_img_yuv_addr[0].y == NULL )
    {
        MDP_ERROR("address is NULL. dst_img_yuv_addr[0].y = 0x%08X\n", (unsigned int)dst_img_yuv_addr[0].y );
        DumpDebugInfo();
        return -1;
    }




    /*-----------------------------------------------------------------------------
        Calculate buffer size and uv address
      -----------------------------------------------------------------------------*/
    if( MdpDrv_CalImgBufferArraySizeAndFillIn(
                        dst_color_format,
                        dst_img_yuv_addr,
                        uOutBufferCnt,
                        dst_img_size,
                        dst_img_roi,
                        ((bFlip << 2) | bRotate ), /*rotate*/
                        &dst_img_buffer_size_total_ ) < 0 )
    {
        MDP_ERROR("<%s> Calc memory array size error.\n", this->name_str() );
        return -1;
    }


    /*-----------------------------------------------------------------------------
        Adapt dest image address
      -----------------------------------------------------------------------------*/

    if( MdpDrv_AdaptMdpYuvAddrArray(    this->id(),
                                        dst_img_yuv_addr, uOutBufferCnt,//Original user input address
                                        dst_img_yuv_adapt_addr,         //Output adapted address
                                        &adapt_m4u_flag_bit_,           //If address has been adapted with m4u mva, corresponding bit will set to 1
                                        &alloc_mva_flag_bit_ ) != 0 )   //Corresponding bit will set to 1 if the mva is new allocated
    {
        MDP_ERROR("<%s> Adapt Memory error.\n", this->name_str() );
        DumpDebugInfo();
        return -1;
    }

    /*-----------------------------------------------------------------------------
        Get Color format information
      -----------------------------------------------------------------------------*/
    if( MdpDrvColorFormatInfoGet( dst_color_format, &ci  ) < 0 )
    {
        MDP_ERROR("Config ROTDMA failed!! -- Unknow color format (0x%x)!!\n",(unsigned int)dst_color_format);
        return -1;
    }


    /*-----------------------------------------------------------------------------
        Register R/W
      -----------------------------------------------------------------------------*/
    //Warm Reset
    Reset();

    //Enable all interrupts
    MT6575_ROT_DMA_IRQ_FLAG(reg_base) = (0x33 << 16);

    //Clear interrupts
    MT6575_ROT_DMA_IRQ_FLAG_CLR(reg_base) = 0x33;

#ifdef MDP_FLAG_1_SUPPORT_DESCRIPTOR
    //Descript mode
    MT6575_ROT_DMA_CFG(reg_base) =  ((unsigned long)bCamIn << 31) +
                                    (((MID_VDO_ROT0 == rot_id) || (MID_VDO_ROT1 == rot_id) ? 0x7 : 0x1) << 17) + /*YUV or RGB SEG enable*/
                                    (1 << 15) +                 /*Q_EMPTY_DROP. 1:Enable 0:Disable (For RDMA, Use 0)*/
                                    ( (uOutBufferCnt-1) << 8) +    /*QUEUE_DEPTH. 0 start*/
                                    (1 << 7) +                  /*MODE. 1:Desc 2:Register*/
                                    0;//bContinuous;            /*Disable auto loop when in descriptor mode*/
    if( b_is_zero_shutter_encode_port_ == 0x1 )
    {
        /*VDO Rotator*/
        if( this->id() & ( MID_VDO_ROT0 | MID_VDO_ROT1 ) )
        {
            MT6575_ROT_DMA_CFG(reg_base) |= (0x0F << 20 ); /*Turn on desc : SEG4~SEG7*/
        }
        /*RGB Rotator*/
        else
        {
            MT6575_ROT_DMA_CFG(reg_base) |= (0x3F << 20 ); /*Turn on desc : SEG4~SEG9*/
        }
    }

    MT6575_ROT_DMA_QUEUE_BASE(reg_base) = descript_base;//Queue address

#else
    //Register mode
    MT6575_ROT_DMA_CFG(reg_base) =  ((unsigned long)bCamIn << 31) +
                                    bContinuous;
#endif

    MT6575_ROT_DMA_IN_SEL(reg_base) = 0;


    if(bEnHWTriggerRDMA0)
    {
        MT6575_ROT_DMA_LCD(reg_base) = 0x7;
    }
    else
    {
        MT6575_ROT_DMA_LCD(reg_base) = 0;
    }

    #if 1
    MT6575_ROT_DMA_SLOW_DOWN(reg_base) = 0;
    #else
    MT6575_ROT_DMA_SLOW_DOWN(reg_base) = 0xFFFF0001;
    MDP_WARNING("ROT DMA slow down enable.:0x%08X\n", MT6575_ROT_DMA_SLOW_DOWN(reg_base));
    #endif

    MT6575_ROT_DMA_CON(reg_base) = ((1 << 31) | ((unsigned long)bFlip << 29) |
    	((unsigned long)bRotate << 27) | ((unsigned long)uAlpha << 8) | (0x3 << 4));

    MT6575_ROT_DMA_CON(reg_base) += ci.out_format +
                                    (ci.yh << 18) + (ci.uvh << 20) + (ci.yv << 22) + (ci.uvv << 24) +
                                    ( ( ( ci.uv_stride_align == 0 )? 0 : 1 ) << 26 );//bit 26:Y_UV_PITCH_SPLIT




    //Calculate line buffer size
    stSysram.u4Alignment = 8;
    stSysram.u4SingleBuffSize = 0;
    stSysram.u4MinBuffCnt = 1;
    stSysram.u4RecommandBuffCnt = 1;

    stBuffCal.pLineBuffSetting = &LBSetting;
    stBuffCal.pLineBuffSetting->u4BytePerPixel = ci.byte_per_pixel;
    stBuffCal.pLineBuffSetting->u4MainSize = 0;
    stBuffCal.pLineBuffSetting->u4SubSize  = 0;
    stBuffCal.pLineBuffSetting->u4LB_S_IN_LINE = 0;
    stBuffCal.pLineBuffSetting->u4Main_blk_w = 0;
    stBuffCal.pLineBuffSetting->u4Sub_blk_w = 0;
    stBuffCal.pLineBuffSetting->u4Main_buf_line_size = 0;
    stBuffCal.pLineBuffSetting->u4Sub_buf_line_size = 0;

    //TODO:Should exchang w and h??
    if(0x1 & bRotate)//90 or 270 degree case, needs line buffer
    {
        stBuffCal.clip_w = dst_img_roi.w;
        stBuffCal.eOutputColorFMT = dst_color_format;
        stBuffCal.uvv = ci.uvv;

        CheckAndAllocateLineBuffer(&stBuffCal , &stSysram);
    }


    /*-----------------------------------------------------------------------------
        Frame Start Address Calculation/Y & UV stride calculation
      -----------------------------------------------------------------------------*/
    {
        CalcFrameStartAddress_In   param_in;
        CalcFrameStartAddress_Out  param_out;

        param_in.rotate = bRotate;                  /*rotation w/o flip. 0-3*/
        param_in.dst_img_roi  = dst_img_roi;        /*src roi image before rotation*/
        param_in.dst_img_size = dst_img_size;       /*stride before rotation*/
        param_in.dst_color_format = dst_color_format;
        param_in.b_is_generic_yuv = ci.b_is_generic_yuv;
        param_in.byte_per_pixel = ci.byte_per_pixel;
        /*y,u,v sampling period*/
        param_in.yv = ci.yv;
        param_in.yh = ci.yh;
        param_in.uvv = ci.uvv;
        param_in.uvh = ci.uvh;
        //Stride Align: 0:None 1:2 align 2:4 align 3:8 align 4:16 align
        param_in.y_stride_align = ci.y_stride_align;
        param_in.uv_stride_align= ci.uv_stride_align;



        CalcFrameStartAddress( &param_in, &param_out );

        y_stride  = param_out.y_stride;
        uv_stride = param_out.uv_stride;
        u4YFrame_Start_In_Byte = param_out.y_frame_start_in_byte;
        u4UVFrame_Start_In_Byte= param_out.uv_frame_start_in_byte;
    }

    /*-----------------------------------------------------------------------------

      -----------------------------------------------------------------------------*/

//This register is moved to read DMA, don't need to set here
//    MT6575_ROT_DMA_LCD_STR_ADDR(pBase) = ;

    MT6575_ROT_DMA_SRC_SIZE(reg_base) = src_img_size.w + ((unsigned long)src_img_size.h << 16);

    MT6575_ROT_DMA_CLIP_SIZE(reg_base) = src_img_roi.w + ((unsigned long)src_img_roi.h << 16);

    MT6575_ROT_DMA_CLIP_OFFSET(reg_base) = src_img_roi.x + ((unsigned long)src_img_roi.y << 16);;


    MT6575_ROT_DMA_DST_SIZE(reg_base) = ( y_stride * ci.byte_per_pixel) & 0x0003FFFF ;

    if( ci.uv_stride_align != 0 )
    {
        MT6575_ROT_DMA_DST_SIZE(reg_base) |= ( ( uv_stride*ci.byte_per_pixel ) << 18 );
    }


    MT6575_ROT_DMA_CLIP_W_IN_BYTE(reg_base) = (unsigned long)src_img_roi.w*ci.byte_per_pixel;

    MT6575_ROT_DMA_CLIP_H_IN_BYTE(reg_base) = (unsigned long)src_img_roi.h*ci.byte_per_pixel;

    MT6575_ROT_DMA_Y_DST_STR_ADDR(reg_base) = dst_img_yuv_adapt_addr[0].y + u4YFrame_Start_In_Byte;

    MT6575_ROT_DMA_U_DST_STR_ADDR(reg_base) = dst_img_yuv_adapt_addr[0].u + u4UVFrame_Start_In_Byte;

    MT6575_ROT_DMA_V_DST_STR_ADDR(reg_base) = dst_img_yuv_adapt_addr[0].v + u4UVFrame_Start_In_Byte;

#ifdef MDP_FLAG_1_SUPPORT_DESCRIPTOR

    /*Backup descriptor value, for latter queue/dequeue used*/
    desc_src_img_size_              = src_img_size;
    desc_src_img_roi_               = src_img_roi;
    desc_dst_img_size_              = dst_img_size;
    desc_dst_img_roi_               = dst_img_roi;
    desc_is_generic_yuv_            = ci.b_is_generic_yuv;
    desc_byte_per_pixel_            = ci.byte_per_pixel;
    desc_yv_                        = ci.yv;
    desc_yh_                        = ci.yh;
    desc_uvv_                       = ci.uvv;
    desc_uvh_                       = ci.uvh;
    desc_y_stride_align_            = ci.y_stride_align;
    desc_uv_stride_align_           = ci.uv_stride_align;
    desc_y_stride_                  = y_stride;
    desc_uv_stride_                 = uv_stride;
    desc_y_frame_start_in_byte_     = u4YFrame_Start_In_Byte;
    desc_uv_frame_start_in_byte_    = u4UVFrame_Start_In_Byte;

    for(u4Index = 0; u4Index < uOutBufferCnt ; u4Index += 1)
    {
        _DescQueueRefillByIndex( u4Index, reg_base );

    }

    /*Reset SW write index , this "0" is actually a overflow value of uOutBufferCnt.*/
    desc_sw_write_index_ = 0;

#endif


    //Allocate sysram
    sysram_.size = stSysram.u4SingleBuffSize;
    sysram_.address = 0;
    if( sysram_.size != 0 )
    {
        sysram_.address = MdpSysram_Alloc( this->id(), sysram_.size , MDPSYSRAM_SUBCAT_NORMAL/*0*/ );

        if( sysram_.address == 0 )
        {
            sysram_.size == 0;
            MDP_ERROR("SYSRAM allocate failed!\n");
            DumpDebugInfo();
            return -1;
        }
    }



    MT6575_ROT_DMA_BUF_ADDR0(reg_base) = sysram_.address;//u4MainBuffAddr;

    MT6575_ROT_DMA_BUF_ADDR1(reg_base) = MT6575_ROT_DMA_BUF_ADDR0(reg_base);

    MT6575_ROT_DMA_MAIN_BUFF_SIZE(reg_base) = (stBuffCal.pLineBuffSetting->u4Main_blk_w << 17) + stBuffCal.pLineBuffSetting->u4Main_buf_line_size;

    MT6575_ROT_DMA_BUF_ADDR2(reg_base) = sysram_.address + stBuffCal.pLineBuffSetting->u4MainSize;

    MT6575_ROT_DMA_BUF_ADDR3(reg_base) = MT6575_ROT_DMA_BUF_ADDR2(reg_base);

    MT6575_ROT_DMA_SUB_BUFF_SIZE(reg_base) = (stBuffCal.pLineBuffSetting->u4Sub_blk_w << 17) + stBuffCal.pLineBuffSetting->u4Sub_buf_line_size;

    MT6575_ROT_DMA_PERF(reg_base) = ((bCamIn ? (0x3 << 30) : 0) | (1 << 24) | (stBuffCal.pLineBuffSetting->u4LB_S_IN_LINE << 16)
        | ((stBuffCal.pLineBuffSetting->u4LB_S_IN_LINE*ci.byte_per_pixel) << 8) | 0xF);//Use 0xF to enhance emi usage effeiciency

    if((MID_RGB_ROT0 == rot_id) || (MID_RGB_ROT1 == rot_id) || (MID_RGB_ROT2 == rot_id))
    {
        MT6575_ROT_DMA_DITHER(reg_base) = (bDithering ? 0x232003 : 0);
    }

	MT6575_ROT_DMA_LCD(reg_base) = (1 == bEnHWTriggerRDMA0 ? 0x13 : 0);

    if(bEnHWTriggerRDMA0)
    {
		switch(rot_id)
		{
			case MID_RGB_ROT0 :
				MT6575_MMSYS_R_DMA0_TRIG_SEL(mmsys1_reg_base) = 1;
			break;
			case MID_RGB_ROT1 :
				MT6575_MMSYS_R_DMA0_TRIG_SEL(mmsys1_reg_base) = 2;
			break;
			case MID_VDO_ROT0 :
				MT6575_MMSYS_R_DMA0_TRIG_SEL(mmsys1_reg_base) = 0;
			break;
			case MID_VDO_ROT1 :
				MT6575_MMSYS_R_DMA0_TRIG_SEL(mmsys1_reg_base) = 3;
			break;
			default :
				MDP_ERROR("unknown ROTDMA!!\n");
				return -1;
			break;
		}


    }

    return 0;
}

int ROTDMAEX_I::_Enable( void )
{

    /*-----------------------------------------------------------------------------
        Invalid Cache before HW write
      -----------------------------------------------------------------------------*/
    if( MdpDrv_CacheSync(   this->id(),
                            MDPDRV_CACHE_SYNC_INVALID_BEFORE_HW_WRITE_MEM,
                            dst_img_yuv_addr[0].y , dst_img_buffer_size_total_
                        ) < 0 )
    {

        MDP_WARNING("ROTDMAEX Cache sync error. address:0x%08X size:0x%08X\n", (unsigned int)dst_img_yuv_addr[0].y, (unsigned int)dst_img_buffer_size_total_ );
        //return -1;
    }


    /*-----------------------------------------------------------------------------
        Trigger HW
      -----------------------------------------------------------------------------*/
    unsigned long a_pRegBase;

    a_pRegBase = this->reg_base_addr(); //virtual


    if( a_pRegBase== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    MT6575_ROT_DMA_STOP(a_pRegBase) = 0;

    MT6575_ROT_DMA_EN(a_pRegBase) = 1;


    return 0;
}

int ROTDMAEX_I::_Disable( void )
{
    unsigned long a_pRegBase;

    a_pRegBase = this->reg_base_addr(); //virtual


    if( a_pRegBase== 0 ){
        return -1;
    }

#ifdef DESCRIPTMODE
    //    while(0x1 != MT6575_ROT_DMA_QUEUE_RSTA(u4Base));
#endif

    MT6575_ROT_DMA_STOP(a_pRegBase) = 1;
    MT6575_ROT_DMA_STOP(a_pRegBase) = 0;

    /*Free_Sysram( this->id() ); //virtual*/
    if( sysram_.size > 0 )
    {
        MdpSysram_Free( this->id(), sysram_.address , MDPSYSRAM_SUBCAT_NORMAL/*0*/ );
        sysram_.address = 0;
        sysram_.size = 0;
    }

    Reset();


    /*UnAdapt MdpYuvAddress*/
    MdpDrv_UnAdaptMdpYuvAddrArray(  this->id(),
                                    dst_img_yuv_addr, uOutBufferCnt,     //Original user input address
                                    dst_img_yuv_adapt_addr,             //Adapted address
                                    adapt_m4u_flag_bit_,                //Corresponding bit will set to 1 if address had been adapted with m4u mva
                                    alloc_mva_flag_bit_ );              //Corresponding bit will set to 1 if the mva is new allocated


    return 0;
}

int ROTDMAEX_I::CheckBusy( unsigned long* desc_read_pointer )
{
    static unsigned long    _lcd_too_slow_show_frequency = 0;
    unsigned long reg_base;

    unsigned long u4Result;

    unsigned long u4BuffCnt;


    reg_base = this->reg_base_addr(); //virtual


    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    //Check error status as well
    u4Result = MT6575_ROT_DMA_IRQ_FLAG(reg_base);

    //Clear interrupt
    MT6575_ROT_DMA_IRQ_FLAG_CLR(reg_base) = ( 0x33 & u4Result );

    u4Result &= 0xFF;

    if(u4Result & 0x1){/*MDP_INFO("engine finished the decriptor...\n");*/}

    if(u4Result & 0x2){MDP_INFO("SW configuration error. Invalid descriptor mode...\n");}

    if(u4Result & 0x10){    if( (_lcd_too_slow_show_frequency++%300) == 0 )  MDP_INFO("LCD too slow...(%lu)\n",_lcd_too_slow_show_frequency);  }

    if(u4Result & 0x20){MDP_INFO("SW configuration error. SW writing to descriptor queue when command queue writer is busy...\n");}

    u4BuffCnt = ((MT6575_ROT_DMA_CFG(reg_base) & 0xf0000) >> 16);

    *desc_read_pointer = ((MT6575_ROT_DMA_QUEUE_RSTA(reg_base) & 0xf00) >> 8);

    *desc_read_pointer = (0 == *desc_read_pointer ? u4BuffCnt : (*desc_read_pointer) - 1);

    return (u4Result & 0x1 ? 0 : 1);//MT6575_ROT_DMA_EN(pBase);
}


int ROTDMAEX_I::Reset( void )
{
    volatile unsigned long time_out = MDP_DMA_RESET_TIMEOUT;//0xFFFFFF;
    unsigned long reg_base;
    volatile unsigned long time_delay;

    reg_base = this->reg_base_addr();

    if( reg_base== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    MT6575_ROT_DMA_RESET(reg_base) = 0x2;

    while(0x2 == MT6575_ROT_DMA_RESET(reg_base))
    {
        time_out--;
        if( time_out == 0 ){
            MDP_ERROR("Config <%s> failed -- reset ROTDMAEX timeout,proceed hard reset\n",this->name_str());

            /*Hard Reset*/
            MT6575_ROT_DMA_RESET( reg_base ) = 0x1;
            for( time_delay = 0 ; time_delay < 50 ; time_delay += 1){;}//Delay 50 cycle
            MT6575_ROT_DMA_RESET( reg_base ) = 0x0;

            return 0;
        }
    }


    //New Rotater Reset Change for bug fix
        /*RGB ROT*/
    if( this->id() & ( MID_RGB_ROT0|MID_RGB_ROT1|MID_RGB_ROT2 ) )
    {
        MT6575_ROT_DMA_CON( reg_base ) = 0x08000003;
        MT6575_ROT_DMA_RESET(reg_base) = 0x2;
        MT6575_ROT_DMA_RESET(reg_base) = 0x2;
        MT6575_ROT_DMA_RESET(reg_base) = 0x2;
        MT6575_ROT_DMA_RESET(reg_base) = 0x2;
        MT6575_ROT_DMA_RESET(reg_base) = 0x2;
        MT6575_ROT_DMA_CON( reg_base ) = 0x08000007;
        MT6575_ROT_DMA_RESET(reg_base) = 0x2;
    }
    else/*VDO ROT*/
    {
        MT6575_ROT_DMA_CON( reg_base ) = 0x08000000;
        MT6575_ROT_DMA_RESET(reg_base) = 0x2;
    }

    return 0;
}

int ROTDMAEX_I::HardReset( void )
{
    unsigned long reg_base;
    volatile unsigned long time_delay;

    reg_base = this->reg_base_addr();

    if( reg_base== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    /*Hard Reset*/
    MT6575_ROT_DMA_RESET( reg_base ) = 0x1;
    for( time_delay = 0 ; time_delay < 50 ; time_delay += 1){;}//Delay 50 cycle
    MT6575_ROT_DMA_RESET( reg_base ) = 0x0;

    MDP_WARNING("<%s> Hard Reset!\n",this->name_str());


    return 0;
}



int ROTDMAEX_I::DumpRegisterCustom( int mode )
{

    unsigned long reg_base;
    unsigned long reg_val;

    reg_base = this->reg_base_addr();

    if( reg_base== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    //MT6575_ROT_DMA_CFG
    reg_val = MT6575_ROT_DMA_CFG(reg_base);
    MDP_PRINTF("AUTO_LOOP=%d\n",reg_val&(0x1)?1:0);
    MDP_PRINTF("FRAME_SYNC_EN=%d\n",reg_val&(0x1<<31)?1:0);


    //MT6575_ROT_DMA_IRQ_FLAG
    reg_val = MT6575_ROT_DMA_IRQ_FLAG(reg_base);
    MDP_PRINTF("FLAG0(Done)=%d\n",reg_val&(0x1)?1:0);
    MDP_PRINTF("FLAG1(SW err)=%d\n",reg_val&(0x1<<1)?1:0);
    MDP_PRINTF("FLAG4(LCD too slow)=%d\n",reg_val&(0x1<<4)?1:0);
    MDP_PRINTF("FLAG5(SW err)=%d\n",reg_val&(0x1<<5)?1:0);


    //MT6575_ROT_DMA_DBG_ST0
    reg_val = MT6575_ROT_DMA_DBG_ST0(reg_base);
    MDP_PRINTF("CLIP_CUR_X(counter)=%d CLIP_CUR_Y(counter)=%d\n",(int)(reg_val&0x0000FFFF) , (int)((reg_val>>16)&0x0000FFFF));


    //MT6575_ROT_DMA_LCD
    reg_val = MT6575_ROT_DMA_LCD(reg_base);
    MDP_PRINTF("ROT_DMA_LCD(HW Trigger)=0x%X\n",(int)reg_val);

//---
    //MT6575_ROT_DMA_SRC_SIZE
    reg_val = MT6575_ROT_DMA_SRC_SIZE(reg_base);
    MDP_PRINTF("SRC_W=%d\n",(int)(reg_val&0x0000FFFF));
    MDP_PRINTF("SRC_H=%d\n",(int)((reg_val&0xFFFF0000)>>16));

    //MT6575_ROT_DMA_CLIP_OFFSET
    reg_val = MT6575_ROT_DMA_CLIP_OFFSET(reg_base);
    MDP_PRINTF("CLIP_X=%d\n",(int)(reg_val&0x0000FFFF));
    MDP_PRINTF("CLIP_Y=%d\n",(int)((reg_val&0xFFFF0000)>>16));

    //MT6575_ROT_DMA_CLIP_SIZE
    reg_val = MT6575_ROT_DMA_CLIP_SIZE(reg_base);
    MDP_PRINTF("CLIP_W=%d\n",(int)(reg_val&0x0000FFFF));
    MDP_PRINTF("CLIP_H=%d\n",(int)((reg_val&0xFFFF0000)>>16));


   //MT6575_ROT_DMA_DST_SIZE
   reg_val = MT6575_ROT_DMA_DST_SIZE(reg_base);
   MDP_PRINTF("DST_W_SIZE=%d(Y) %d(UV)\n",(int)(reg_val & 0x0003FFFF ), (int)( reg_val >> 18) );

   //MT6575_ROT_DMA_CLIP_W_IN_BYTE
   reg_val = MT6575_ROT_DMA_CLIP_W_IN_BYTE(reg_base);
   MDP_PRINTF("CLIP_W_SIZE=%d\n",(int)(reg_val));


   //MT6575_ROT_DMA_CLIP_H_IN_BYTE
   reg_val = MT6575_ROT_DMA_CLIP_H_IN_BYTE(reg_base);
   MDP_PRINTF("CLIP_H_SIZE=%d\n",(int)(reg_val));


   //MT6575_ROT_DMA_QUEUE_RSTA
   reg_val = MT6575_ROT_DMA_QUEUE_RSTA(reg_base);
   MDP_PRINTF("Descripter EMPTY=%d\n",(int)((reg_val & 0x4) >> 2));




    return 0;
}



int ROTDMAEX_I::DumpDebugInfo( void )
{
    int i;

    MDPELEMENT_I::DumpDebugInfo();

    for( i = 0; i < uOutBufferCnt; i++ ) {
        MDP_PRINTF("%d/%d:\n",(int)i+1,(int)uOutBufferCnt);
        MDP_DUMP_VAR_H(dst_img_yuv_addr[i].y);
        MDP_DUMP_VAR_H(dst_img_yuv_addr[i].u);
        MDP_DUMP_VAR_H(dst_img_yuv_addr[i].v);
        MDP_DUMP_VAR_H(dst_img_yuv_addr[i].y_buffer_size);
        MDP_DUMP_VAR_H(dst_img_yuv_addr[i].u_buffer_size);
        MDP_DUMP_VAR_H(dst_img_yuv_addr[i].v_buffer_size);
        MDP_PRINTF("\n");
    }

    MDP_DUMP_VAR_D(src_img_roi.x);
    MDP_DUMP_VAR_D(src_img_roi.y);
    MDP_DUMP_VAR_D(src_img_roi.w);
    MDP_DUMP_VAR_D(src_img_roi.h);

    MDP_DUMP_VAR_D(dst_img_size.w);
    MDP_DUMP_VAR_D(dst_img_size.h);

    MDP_DUMP_VAR_D(dst_color_format);

    MDP_DUMP_VAR_D(uOutBufferCnt);
    MDP_DUMP_VAR_D(bContinuous);
    MDP_DUMP_VAR_D(bFlip);
    MDP_DUMP_VAR_D(bRotate);
    MDP_DUMP_VAR_D(bDithering);
    MDP_DUMP_VAR_D(uAlpha);
    MDP_DUMP_VAR_D(bCamIn);
    MDP_DUMP_VAR_D(bEnHWTriggerRDMA0);


    for( i = 0; i < uOutBufferCnt; i++ ) {
        MDP_PRINTF("%d/%d:\n",(int)i+1,(int)uOutBufferCnt);
        MDP_DUMP_VAR_H(dst_img_yuv_adapt_addr[i].y);
        MDP_DUMP_VAR_H(dst_img_yuv_adapt_addr[i].u);
        MDP_DUMP_VAR_H(dst_img_yuv_adapt_addr[i].v);
        MDP_DUMP_VAR_H(dst_img_yuv_adapt_addr[i].y_buffer_size);
        MDP_DUMP_VAR_H(dst_img_yuv_adapt_addr[i].u_buffer_size);
        MDP_DUMP_VAR_H(dst_img_yuv_adapt_addr[i].v_buffer_size);
        MDP_PRINTF("\n");
    }

    MDP_DUMP_VAR_H(adapt_m4u_flag_bit_);
    MDP_DUMP_VAR_H(dst_img_buffer_size_total_);

    MDP_DUMP_VAR_H(sysram_.address);

    MDP_DUMP_VAR_D(desc_sw_write_index_);

    MDP_DUMP_VAR_D(b_is_zero_shutter_encode_port_); /*1:zero shutter enable*/

    MDP_DUMP_VAR_D(desc_src_img_roi_.x);
    MDP_DUMP_VAR_D(desc_src_img_roi_.y);
    MDP_DUMP_VAR_D(desc_src_img_roi_.w);
    MDP_DUMP_VAR_D(desc_src_img_roi_.h);

    MDP_DUMP_VAR_D(desc_dst_img_size_.w);
    MDP_DUMP_VAR_D(desc_dst_img_size_.h);

    MDP_DUMP_VAR_D(desc_is_generic_yuv_);
    MDP_DUMP_VAR_D(desc_byte_per_pixel_);
    MDP_DUMP_VAR_D(desc_yv_);
    MDP_DUMP_VAR_D(desc_yh_);
    MDP_DUMP_VAR_D(desc_uvv_);
    MDP_DUMP_VAR_D(desc_uvh_);
    MDP_DUMP_VAR_H(desc_y_stride_);
    MDP_DUMP_VAR_H(desc_uv_stride_);
    MDP_DUMP_VAR_H(desc_y_frame_start_in_byte_);
    MDP_DUMP_VAR_H(desc_uv_frame_start_in_byte_);

    return 0;

}



int ROTDMAEX_I::DescQueueGetFreeList( unsigned long *p_StartIndex, unsigned long *p_Count ) /*Equel to De Queue Buffer in 73*/
{
    unsigned long   reg_base;
    unsigned long   reg_val;
    int             b_empty;
    unsigned long   hw_read_index; /*RPT*/

    reg_base = this->reg_base_addr();


    if( reg_base== 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    reg_val = MT6575_ROT_DMA_QUEUE_RSTA( reg_base );


    b_empty         = ( reg_val & ( 0x1 << 2 ) ) >> 2;
    hw_read_index   = ( reg_val & ( 0xF << 8 ) ) >> 8;


    /*Queue is full*/
    if( !b_empty &&  ( hw_read_index == desc_sw_write_index_ ) )
    {
        *p_StartIndex = desc_sw_write_index_;
        *p_Count = 0;
    }
    /*Queue still has free space, return immediately and no wait*/
    else
    {

        *p_StartIndex = desc_sw_write_index_;
        *p_Count = ( hw_read_index > desc_sw_write_index_ ?
                        (hw_read_index - desc_sw_write_index_) :
                        (uOutBufferCnt - desc_sw_write_index_ + hw_read_index)  );
    }

    return 0;

}


int ROTDMAEX_I::DescQueueRefill( void ) /*Equel to En Queue Buffer in 73*/
{
    unsigned long reg_base;
    unsigned long rot_id;
    unsigned long reg_val;
    int           ret_val = 0;

    reg_base = this->reg_base_addr();
    rot_id   = this->id();


    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }

    ret_val = _DescQueueRefillByIndex( desc_sw_write_index_, reg_base );


    if( ret_val == 0 )
    {
        /*Update SW write index*/

        desc_sw_write_index_ = (desc_sw_write_index_+1) == uOutBufferCnt ? 0 : (desc_sw_write_index_+1);

        MDP_INFO_QUEUE("[QUEUE]Enqueue <%s> Refill 1. Index = %d\n",this->name_str(), (int)desc_sw_write_index_ );
    } else
    {
        MDP_INFO_QUEUE("[QUEUE]Enqueue <%s> failed. Index = %d\n",this->name_str(), (int)desc_sw_write_index_ );
    }


    return ret_val;


}


inline int ROTDMAEX_I::DescWriteWaitBusy( unsigned long reg_base )
{
    unsigned long u4TimeOut;

    u4TimeOut = 0;
    while(!(MT6575_ROT_DMA_QUEUE_WSTA(reg_base) & 0x1))
    {
        u4TimeOut += 1;
        //if(u4TimeOut  > 100000)
        if(u4TimeOut  > 0xFFFFFF)
        {
            return -1;
        }
    }

    return 0;
}

int ROTDMAEX_I::DescQueueWaitEmpty( void )
{
    unsigned long   reg_base;
    unsigned long   reg_val;
    int             b_empty;
    unsigned long   hw_read_index; /*RPT*/

    reg_base = this->reg_base_addr();

    if( reg_base == 0 )
    {
        MDP_ERROR("<%s> No register memory map\n",this->name_str());
        return -1;
    }


    do
    {
        reg_val = MT6575_ROT_DMA_QUEUE_RSTA( reg_base );
        b_empty         = ( reg_val & ( 0x1 << 2 ) ) >> 2;
        hw_read_index   = ( reg_val & ( 0xF << 8 ) ) >> 8;

    } while( b_empty == 0 );
    MDP_INFO_QUEUE("\t<%s> wait empty(%d), desc hw_read_index= %lu\n", this->name_str(), b_empty, hw_read_index );


    return 0;


}



int ROTDMAEX_I::_DescQueueRefillByIndex( unsigned long index , unsigned long reg_base )
{

    #define _CHECK_Q_BUSY_AND_RETURN_( _seg_name_ ) \
        if( DescWriteWaitBusy( reg_base ) < 0 ){\
            MDP_ERROR("%s put " _seg_name_ " command queue timeout\n",this->name_str());\
            return -1; }

    /*Fill descriptor command*/
    /*[SEG1]*/
    _CHECK_Q_BUSY_AND_RETURN_("[SEG1]");
    MT6575_ROT_DMA_QUEUE_DATA(reg_base) = dst_img_yuv_adapt_addr[index].y + desc_y_frame_start_in_byte_;

    if((MID_VDO_ROT0 == this->id()) || (MID_VDO_ROT1 == this->id()))
    {
        /*[SEG2]*/
        _CHECK_Q_BUSY_AND_RETURN_("[SEG2]");
        MT6575_ROT_DMA_QUEUE_DATA(reg_base) = dst_img_yuv_adapt_addr[index].u + desc_uv_frame_start_in_byte_;

        /*[SEG3]*/
        _CHECK_Q_BUSY_AND_RETURN_("[SEG3]");
        MT6575_ROT_DMA_QUEUE_DATA(reg_base) = dst_img_yuv_adapt_addr[index].v + desc_uv_frame_start_in_byte_;
    }

    if( b_is_zero_shutter_encode_port_ )
    {
        /*[SEG4]*/
        _CHECK_Q_BUSY_AND_RETURN_("[SEG4]");
        MT6575_ROT_DMA_QUEUE_DATA(reg_base) = desc_src_img_size_.w + ((unsigned long)desc_src_img_size_.h << 16);

        /*[SEG5]*/
        _CHECK_Q_BUSY_AND_RETURN_("[SEG5]");
        MT6575_ROT_DMA_QUEUE_DATA(reg_base) = desc_src_img_roi_.w + ((unsigned long)desc_src_img_roi_.h << 16);

        /*[SEG6]*/
        _CHECK_Q_BUSY_AND_RETURN_("[SEG6]");
        MT6575_ROT_DMA_QUEUE_DATA(reg_base) = desc_src_img_roi_.x + ((unsigned long)desc_src_img_roi_.y << 16);;

        /*[SEG7]*/
        _CHECK_Q_BUSY_AND_RETURN_("[SEG7]");
        if( desc_uv_stride_align_ != 0 ){
            MT6575_ROT_DMA_QUEUE_DATA(reg_base)  =  ( ( desc_y_stride_ * desc_byte_per_pixel_ ) & 0x0003FFFF ) |
                                                    ( ( desc_uv_stride_ * desc_byte_per_pixel_ ) << 18 );

        }else  {
            MT6575_ROT_DMA_QUEUE_DATA(reg_base) = ( desc_y_stride_ * desc_byte_per_pixel_ ) & 0x0003FFFF ;
        }

        if( this->id() & ( MID_RGB_ROT0 | MID_RGB_ROT1 | MID_RGB_ROT2 ) )
        {

            /*[SEG8]*/
            _CHECK_Q_BUSY_AND_RETURN_("[SEG8]");
            MT6575_ROT_DMA_QUEUE_DATA(reg_base) = (unsigned long)desc_dst_img_roi_.w * desc_byte_per_pixel_;

            /*[SEG9]*/
            _CHECK_Q_BUSY_AND_RETURN_("[SEG9]");
            MT6575_ROT_DMA_QUEUE_DATA(reg_base) = (unsigned long)desc_dst_img_roi_.h * desc_byte_per_pixel_;
        }


    }

    return 0;

}



int ROTDMAEX_I::CalcFrameStartAddress( CalcFrameStartAddress_In* p_in, CalcFrameStartAddress_Out* p_out )
{

    unsigned long   rotate              =   p_in->rotate;             /*rotation w/o flip. 0-3*/
    MdpRect         dst_img_roi         =   p_in->dst_img_roi;        /*src roi image before rotation*/
    MdpSize         dst_img_size        =   p_in->dst_img_size;       /*stride before rotation*/
    MdpColorFormat  dst_color_format    =   p_in->dst_color_format;
    int             b_is_generic_yuv    =   p_in->b_is_generic_yuv;
    unsigned long   byte_per_pixel      =   p_in->byte_per_pixel;
    /*y,u,v sampling period*/
    unsigned long   yv      =   p_in->yv;
    unsigned long   yh      =   p_in->yh;
    unsigned long   uvv     =   p_in->uvv;
    unsigned long   uvh     =   p_in->uvh;


    p_out->y_stride = (rotate & 0x1)?dst_img_size.h : dst_img_size.w;

    /*UV*//*For Android YV12, stride should bi 16X*/
    if( b_is_generic_yuv )
    {
        p_out->uv_stride = p_out->y_stride >> (uvh ? (uvh - 1) : 0);
        MDP_ROUND_UP( p_out->uv_stride, p_in->uv_stride_align );//Round uv stride to 16x align
    } else
    {
        p_out->uv_stride = 0;
    }


    if((2 == rotate) || (3 == rotate))
    {
        if(b_is_generic_yuv)//Planar
        {
            /*Y*/
            p_out->y_frame_start_in_byte = ((unsigned long)p_out->y_stride * byte_per_pixel >> (yh ? (yh - 1) : 0));
            p_out->y_frame_start_in_byte *= (2 == rotate ?
                    (((unsigned long)dst_img_roi.h >> (yv ? (yv - 1) : 0) ) - 1 ) :
                    (((unsigned long)dst_img_roi.w >> (yv ? (yv - 1) : 0) ) - 1 ));
            /*UV*/
            p_out->uv_frame_start_in_byte = (unsigned long) p_out->uv_stride * byte_per_pixel;
            p_out->uv_frame_start_in_byte *= (2 == rotate ?
                    (((unsigned long)dst_img_roi.h >> (uvv ? (uvv - 1) : 0) ) - 1 ) :
                    (((unsigned long)dst_img_roi.w >> (uvv ? (uvv - 1) : 0) ) - 1 ));
        }
        else
        {
            /*
                For non generic YUV format:
                180 degree (both flip and non-flip) : FRM_S_IN_BYTE = DST_W_IN_BYTE * (CLIP_H-1)
                270 degree (both flip and non-flip) : FRM_S_IN_BYTE = DST_W_IN_BYTE * (CLIP_W-1)
            */
            p_out->y_frame_start_in_byte = (unsigned long)p_out->y_stride * byte_per_pixel;
            p_out->y_frame_start_in_byte *= (2 == rotate ? (unsigned long)(dst_img_roi.h - 1) : (unsigned long)(dst_img_roi.w - 1));
            p_out->uv_frame_start_in_byte = 0;
        }
    }
    else
    {
        p_out->y_frame_start_in_byte = 0;
        p_out->uv_frame_start_in_byte = 0;
    }


    /*Add ROI X,Y offset*/
    /*Clouds:
        I think this piece of code is meaningless, because dst_img_roi.x & dst_img_roi.y is always set to 0 by upper layer,
        the destination offset is control by the destination starting address which is calculated by upper layer
     */
    p_out->y_frame_start_in_byte += ((unsigned long) p_out->y_stride * dst_img_roi.y + dst_img_roi.x)*byte_per_pixel;
    if(b_is_generic_yuv)
    {
        p_out->uv_frame_start_in_byte += (((unsigned long) p_out->uv_stride * dst_img_roi.y + dst_img_roi.x)*byte_per_pixel >> ( uvv - 1));
    }


    return 0;
}

int ROTDMAEX_I::CalculateBufferSizeAndYuvOffset( MdpSize Dst_Img_Size, MdpRect Dst_Img_Roi )
{
    int i;

    /*-----------------------------------------------------------------------------
        Calculate buffer size and uv address
      -----------------------------------------------------------------------------*/
    dst_img_buffer_size_total_ = 0;

    for ( i = 0 ; i < uOutBufferCnt; i++ )
    {
        MDP_INFO("Buffer %d/%d:\n", (int)i+1,(int)uOutBufferCnt );

        /*.............................................................................
            Use address[0] as the start address of these sequence of buffer.
            thus, address[1] = address[0] + address[0].size

            Skip the address[0], it is given by user.
          .............................................................................*/
        if( i != 0 )
        {
            dst_img_yuv_addr[i].y = dst_img_yuv_addr[i-1].y +
                                    dst_img_yuv_addr[i-1].y_buffer_size +
                                    dst_img_yuv_addr[i-1].u_buffer_size +
                                    dst_img_yuv_addr[i-1].v_buffer_size;
        }
        /*.............................................................................
            Here calculate the u,v address and buffer size base on y address
          .............................................................................*/
        if( MdpDrv_CalImgBufferSizeAndFillIn(
                        dst_color_format,
                        &(dst_img_yuv_addr[i]),
                        Dst_Img_Size,
                        Dst_Img_Roi,
                        ((bFlip << 2) | bRotate ) ) != 0 )
        {
            return -1;
        }


        dst_img_buffer_size_total_ +=   dst_img_yuv_addr[i].y_buffer_size +
                                        dst_img_yuv_addr[i].u_buffer_size +
                                        dst_img_yuv_addr[i].v_buffer_size;
    }

    return 0;

}

/*/////////////////////////////////////////////////////////////////////////////
    RGBROT1EX
  /////////////////////////////////////////////////////////////////////////////*/
int RGBROT1EX::_ConfigPre( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT1EX::_ConfigPost( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT1EX::_EnablePre( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT1EX::_EnablePost( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT1EX::_DisablePre( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT1EX::_DisablePost( void )
{
    /*Nothing*/
    return 0;
}


/*/////////////////////////////////////////////////////////////////////////////
    RGBROT2EX
  /////////////////////////////////////////////////////////////////////////////*/
int RGBROT2EX::_ConfigPre( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT2EX::_ConfigPost( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT2EX::_EnablePre( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT2EX::_EnablePost( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT2EX::_DisablePre( void )
{
    /*Nothing*/
    return 0;
}

int RGBROT2EX::_DisablePost( void )
{
    /*Nothing*/
    return 0;
}


