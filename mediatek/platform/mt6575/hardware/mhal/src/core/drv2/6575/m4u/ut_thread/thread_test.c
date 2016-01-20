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

#include<stdio.h>
#include<pthread.h>
#include <sys/prctl.h>

void* SubThread1(void* pdata);

void* FunThread1(void* pdata)
{
	int i,j;
	int policy;
	struct sched_param param;
	prctl(PR_SET_NAME, (unsigned long)"FunThread1", 0, 0, 0);
	pthread_getschedparam(pthread_self(),&policy,&param);
	
	if(policy==SCHED_OTHER) printf("FunThread1 SCHED_OTHER\n");
	
	if(policy==SCHED_RR) printf("FunThread1 SCHED_RR\n");
	
	if(policy==SCHED_FIFO) printf("FunThread1 SCHED_FIFO\n");

	nice(100);

	pthread_attr_t attr;
	pthread_t ppid;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&ppid, &attr, SubThread1, NULL);
    pthread_attr_destroy(&attr);

    
	
	for(i=1;i<50;i++)
	{
		for(j=1;j<50000;j++)
		{
		}
		printf("thread 1\n");
	}
	pthread_join(ppid,NULL);
	//sleep(30);
	printf("Thread1 exit\n");
	
	return NULL;
}

void* FunThread2(void* pdata)
{
	int i;
	// for(i=1;i<5000;i++)
	prctl(PR_SET_NAME, (unsigned long)"FunThread2", 0, 0, 0);
	sched_yield();
	sched_yield();
	printf("Thread2 exit\n");
	return NULL;
}

void* FunThread3(void* pdata) 
{
	int i;
	// for(i=1;i<5000;i++)
	prctl(PR_SET_NAME, (unsigned long)"FunThread3", 0, 0, 0);
	sched_yield();
	sched_yield();
	printf("Thread3 exit\n");
	return NULL;
}

void* SubThread1(void* pdata) 
{
	int i,j;
	int policy;
	// for(i=1;i<5000;i++)
	struct sched_param sched_p;
    // Change the scheduling policy to SCHED_RR
    prctl(PR_SET_NAME, (unsigned long)"SubThread1", 0, 0, 0);
    sched_getparam(0, &sched_p);
    //sched_p.sched_priority = 0;

    if (0 != sched_setscheduler(0, SCHED_RR, &sched_p)) {
        printf("sched_setscheduler fail...\n");
    }
    else {
        //setpriority(0, 0, -2);
        nice(-16);
        sched_p.sched_priority = 1;
        sched_getparam(0, &sched_p);
        printf("sched_setscheduler ok..., priority:%d\n", sched_p.sched_priority);
    } 

    nice(-16);

    struct sched_param param;
	pthread_getschedparam(pthread_self(),&policy,&param);
	
	if(policy==SCHED_OTHER) printf("SubThread1 SCHED_OTHER\n");
	
	if(policy==SCHED_RR) printf("SSubThread1 CHED_RR\n");
	
	if(policy==SCHED_FIFO) printf("SubThread1 SCHED_FIFO\n");
    
	for(i=1;i<50;i++)
	{
		for(j=1;j<50000;j++)
		{
		}
		printf("SubThread 1\n");
	}
	sleep(30);
	printf("SubThread1 exit\n");
	return NULL;
}


int main()
{
	int i;
	
	i=getuid();
	
	if(i==0)
		printf("The current user is root\n");
	else
		printf("The current user is not root\n");

    nice(-20);
	
	pthread_t ppid1,ppid2,ppid3;
	struct sched_param param;
	param.sched_priority=1;
	pthread_attr_t attr,attr2;
	pthread_attr_init(&attr);
	pthread_attr_init(&attr2);
	pthread_attr_setschedpolicy(&attr2,SCHED_OTHERS);
	param.sched_priority = 90;
	pthread_attr_setschedparam(&attr2,&param);
	pthread_create(&ppid1,&attr2,&FunThread1,NULL);
    pthread_create(&ppid2,&attr,&FunThread2,NULL);
	pthread_create(&ppid3,&attr,&FunThread3,NULL);
	pthread_join(ppid1,NULL);
	pthread_join(ppid2,NULL);
	pthread_join(ppid3,NULL);
	pthread_attr_destroy(&attr);
	pthread_attr_destroy(&attr2);
	return 0;
}
