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

#include <mdp_service.h>

#include <binder/BinderService.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

//Priority
#include <sys/resource.h>
#include <linux/rtpm_prio.h>
#include <sys/capability.h>

//AID_ROOT,AID_SYSTEM
#include <private/android_filesystem_config.h>

//prctl
#include <linux/prctl.h>





#include "mdp_datatypes.h"
#include "mdp_drv.h"


#undef LOG_TAG
#define LOG_TAG "MDP" 

static int ChangePriority( void )
{
    int                 current_scheduler;
    int                 current_prio;
    int                 current_min_prio, current_max_prio;
    int                 current_nice;
    int                 rr_min_prio, rr_max_prio;
    struct sched_param  sched_p;
    int                 ret = 0;
    int                 ret_val = 0;


    /*1.Get Information*/
    current_scheduler = sched_getscheduler( 0 /*pid_t pid*/);

    sched_getparam( 0 /*pid_t pid*/, &sched_p );
    current_prio = sched_p.sched_priority;

    current_min_prio = sched_get_priority_min( current_scheduler );
    current_max_prio = sched_get_priority_max( current_scheduler );

    current_nice = getpriority( PRIO_PROCESS, 0 /*int who*/);
    

    rr_min_prio = sched_get_priority_min( SCHED_RR );
    rr_max_prio = sched_get_priority_max( SCHED_RR );
    
    //MDP_INFO_PERSIST("[Original Priority Info]---------------------------\n");
    //MDP_INFO_PERSIST("Current ( scheduler , priority , min , max , nice ) = ( %d , %d , %d , %d , %d )\n", 
    //    current_scheduler , current_prio , current_min_prio , current_max_prio , current_nice );
    //MDP_INFO_PERSIST("RR priority ( min , max ) = ( %d, %d )\n", rr_min_prio, rr_max_prio );

    /*2.Setting*/
    /*Use the same priority as surfaceflinger*/
    sched_p.sched_priority = RTPM_PRIO_SURFACEFLINGER | MT_ALLOW_RT_PRIO_BIT /*MTK trick*/;
    ret = sched_setscheduler(0, SCHED_RR, &sched_p);

    if( ret != 0 )
    {
        //MDP_ERROR("sched_setscheduler failed.:%s\n", strerror(errno));
        ret_val = -1;
    }

    /*3.Check Information*/
    current_scheduler = sched_getscheduler( 0 /*pid_t pid*/);

    sched_getparam( 0 /*pid_t pid*/, &sched_p );
    current_prio = sched_p.sched_priority;

    current_min_prio = sched_get_priority_min( current_scheduler );
    current_max_prio = sched_get_priority_max( current_scheduler );

    current_nice = getpriority( PRIO_PROCESS, 0 /*int who*/);
    

    rr_min_prio = sched_get_priority_min( SCHED_RR );
    rr_max_prio = sched_get_priority_max( SCHED_RR );
    
    //MDP_INFO_PERSIST("[Altered Priority Info]---------------------------\n");
    //MDP_INFO_PERSIST("Current ( scheduler , priority , min , max , nice ) = ( %d , %d , %d , %d , %d )\n", 
    //    current_scheduler , current_prio , current_min_prio , current_max_prio , current_nice );
    //MDP_INFO_PERSIST("RR priority ( min , max ) = ( %d, %d )\n", rr_min_prio, rr_max_prio );
    
    
   


    #if 0
    // set policy and priority
    int policy = SCHED_OTHER;
    int priority = 0;
    struct sched_param sched_p;
    sched_getparam(0, &sched_p);
    sched_p.sched_priority = 0;

    sched_setscheduler(0, policy, &sched_p);

    setpriority(PRIO_PROCESS, 0, priority);
    #endif
    
    return ret_val;
}

static int ChangeUser( void )
{
    int ret_val = 0;
    uid_t   uid;

    uid = getuid();
    
    if ( AID_ROOT == uid ) 
    {
        //MDP_INFO_PERSIST("mdpserver(uid = %d) is in root user, adjust caps for its thread", (int)uid);
        
        if ( -1 == prctl(PR_SET_KEEPCAPS, 1, 0, 0) ) 
        {
            //MDP_ERROR("    prctl failed: %s", strerror(errno));

            ret_val = -1;
        } 
        else 
        {
            __user_cap_header_struct hdr;
            __user_cap_data_struct data;

            {
                //Get information
                hdr.version = _LINUX_CAPABILITY_VERSION;    // set caps
                hdr.pid = 0;

                if (-1 == capget(&hdr, &data)) {
                    //MDP_ERROR("    cap setting failed, %s", strerror(errno));
                }

                //MDP_INFO_PERSIST("Capability Org( uid, effective, permitted, inheritable ) = ( %d, 0x%08X, 0x%08X, 0x%08X )\n",
                //    getuid(), data.effective, data.permitted, data.inheritable );
            }


            hdr.version = _LINUX_CAPABILITY_VERSION;    // set caps
            hdr.pid = 0;
            data.effective = ((1 << CAP_SYS_NICE) | (1 << CAP_SETUID) | (1 << CAP_SETGID));
            data.permitted = ((1 << CAP_SYS_NICE) | (1 << CAP_SETUID) | (1 << CAP_SETGID));
            data.inheritable = 0xffffffff;
            if (-1 == capset(&hdr, &data)) {
                //MDP_ERROR("    cap setting failed, %s", strerror(errno));
            }

            setgid(AID_SYSTEM);
            setuid(AID_SYSTEM);         // change user to system

            {
                //Get information
                hdr.version = _LINUX_CAPABILITY_VERSION;    // set caps
                hdr.pid = 0;

                if (-1 == capget(&hdr, &data)) {
                    //MDP_ERROR("    cap setting failed, %s", strerror(errno));
                }

                //MDP_INFO_PERSIST("Capability After( uid, effective, permitted, inheritable ) = ( %d, 0x%08X, 0x%08X, 0x%08X )\n",
                //    getuid(), data.effective, data.permitted, data.inheritable );
            }

            hdr.version = _LINUX_CAPABILITY_VERSION;    // set caps again
            hdr.pid = 0;
            data.effective = (1 << CAP_SYS_NICE);
            data.permitted = (1 << CAP_SYS_NICE);
            data.inheritable = 0xffffffff;
            if (-1 == capset(&hdr, &data)) {
                //MDP_ERROR("    cap re-setting failed, %s", strerror(errno));
            }

            {
                //Get information
                hdr.version = _LINUX_CAPABILITY_VERSION;    // set caps
                hdr.pid = 0;

                if (-1 == capget(&hdr, &data)) {
                    //MDP_ERROR("    cap setting failed, %s", strerror(errno));
                }

                //MDP_INFO_PERSIST("Capability Final( uid, effective, permitted, inheritable ) = ( %d, 0x%08X, 0x%08X, 0x%08X )\n",
                //    getuid(), data.effective, data.permitted, data.inheritable );
            }
            
        }
    } 
    else 
    {
        //MDP_ERROR("mdpserver is not in root (uid = %d)", (int)uid );

        ret_val = -1;
    } 

    return ret_val;
    
}



int main(int argc, char** argv)
{
    //android::sp<android::ProcessState> proc(android::ProcessState::self());
    //android::sp<android::IServiceManager> sm = android::defaultServiceManager();
    //MDP_INFO_PERSIST("MDP service start...");

    //Change Priority to RR
    ChangePriority();

    //Change User to System
    ChangeUser();
    
    //android::sp<android::ProcessState> proc(android::ProcessState::self());
    android::MdpService::publishAndJoinThreadPool();

    //android::ProcessState::self()->startThreadPool();
    //android::IPCThreadState::self()->joinThreadPool();

    //MDP_INFO_PERSIST("MDP service finish...");
    return 0;
}
