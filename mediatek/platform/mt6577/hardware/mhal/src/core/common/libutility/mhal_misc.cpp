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

#define LOG_TAG "mHalMisc"
#include <utils/Errors.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <linux/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <cutils/xlog.h>
//#include "inc/MediaHal.h"
#include "mhal_misc.h"

#define LOGD(fmt, arg...)    XLOGD(fmt, ##arg)
#define LOGE(fmt, arg...)    XLOGE(fmt, ##arg)

//#define BUS_MONITOR
#ifdef BUS_MONITOR
#include "mt6516_busmonitor.h"
#endif
/*******************************************************************************
*
********************************************************************************/
#if (MHAL_PROFILING)
static struct timeval t1;
static struct timeval tAry[8];
#endif

/*******************************************************************************
*
********************************************************************************/
void 
mhalProfilingInit(
)
{
    #if (MHAL_PROFILING)
    gettimeofday(&t1, NULL);
    #endif
}

/*******************************************************************************
*
********************************************************************************/
unsigned int
mhalProfilingGetUs(
)
{
    unsigned int us = 0;
    
    #if (MHAL_PROFILING)
    struct timeval t2;

    gettimeofday(&t2, NULL);

    us = (t2.tv_sec - t1.tv_sec) * 1000 * 1000;
    if (t2.tv_usec >= t1.tv_usec) {
        us += (t2.tv_usec - t1.tv_usec);
    }
    else {
        us -= (t1.tv_usec - t2.tv_usec);
    }
    #endif

    return us;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int
mhalProfilingGetMs(
)
{
    unsigned int ms = 0;

    #if (MHAL_PROFILING)
    ms = (mhalProfilingGetUs() + 500) / 1000;
    #endif
 
    return ms;
}


/*******************************************************************************
*
********************************************************************************/
void 
mhalProfilingInitEx(
    unsigned int num
)
{
    #if (MHAL_PROFILING)
    gettimeofday(&tAry[num], NULL);
    #endif
}

/*******************************************************************************
*
********************************************************************************/
unsigned int
mhalProfilingGetUsEx(
    unsigned int num
)
{
    unsigned int us = 0;
    
    #if (MHAL_PROFILING)
    struct timeval t2;
    struct timeval *pt1;

    gettimeofday(&t2, NULL);
    pt1 = &tAry[num];

    us = (t2.tv_sec - pt1->tv_sec) * 1000 * 1000;
    if (t2.tv_usec >= pt1->tv_usec) {
        us += (t2.tv_usec - pt1->tv_usec);
    }
    else {
        us -= (pt1->tv_usec - t2.tv_usec);
    }
    #endif

    return us;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int
mhalProfilingGetMsEx(
    unsigned int num
)
{
    unsigned int ms = 0;

    #if (MHAL_PROFILING)
    ms = (mhalProfilingGetUsEx(num) + 500) / 1000;
    #endif
 
    return ms;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int mHalMiscDumpToFile(
    char *fname,
    unsigned char *pbuf, 
    unsigned int size
)
{
    int nw, cnt = 0;
    unsigned int written = 0;

    LOGD("opening file [%s]\n", fname);
    int fd = open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        LOGE("failed to create file [%s]: %s", fname, strerror(errno));
        return 0x80000000;
    }

    LOGD("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd, pbuf + written, size - written);
        if (nw < 0) {
            LOGE("failed to write to file [%s]: %s", fname, strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);

    return 0;
}

/*******************************************************************************
* Time profile
********************************************************************************/
#if 1
void TIMEPROFILELOG(LOG_CMD eCMD , unsigned long a_u4Val)
{
    static stLOG Log = {0,0,0,NULL};
    struct timeval tv;
    unsigned long u4Index,u4Index2;
    FILE * pWriteFile = NULL;

    switch(eCMD)
    {
        case RESET_TIMEPROFILE :
            Log.u4Counter = 0;
            Log.u4MaxTimeMark = 0;
        break;
        case ALLOC_TIMEPROFILEBUFF :
            if(NULL != Log.pPage){return;}
            Log.u4LogDepth = a_u4Val;
            Log.u4Counter = 0;
            Log.u4MaxTimeMark = 0;
            Log.pPage = (stLOGPAGE *)malloc(a_u4Val*sizeof(stLOGPAGE));
            if(NULL == Log.pPage){LOGE("Not enough memory for Time profile LOG\n");}
        break;
        case FREE_TIMEPROFILEBUFF :
            if(NULL == Log.pPage){return;}
            free(Log.pPage);
            Log.pPage = NULL;
            Log.u4Counter = 0;
            Log.u4LogDepth = 0;
            Log.u4MaxTimeMark = 0;
        break;
        case PUT_TIMEPROFILE:
            if(NULL == Log.pPage){LOGE("Allocate Log buffer first!!\n");return;}
            if(Log.u4Counter >= Log.u4LogDepth){return;}
            gettimeofday(&tv,NULL);
            Log.pPage[Log.u4Counter].u4Sec = (tv.tv_sec & 0xFF);
            Log.pPage[Log.u4Counter].u4USec = tv.tv_usec;
            Log.pPage[Log.u4Counter].u4TimeMark = a_u4Val;
            if(a_u4Val > Log.u4MaxTimeMark){Log.u4MaxTimeMark = a_u4Val;}
            Log.u4Counter += 1;
        break;
        case PRINT_TIMEPROFILE:
            if(NULL == Log.pPage){LOGE("Allocate Log buffer first!!\n");return;}

            pWriteFile = fopen("//sdcard//TimeLog.txt","w");
            if(NULL == pWriteFile){LOGE("Failed to open file!!\n");return;}

            fprintf(pWriteFile,"\nTime profile\n");
            for(u4Index2 = 0; u4Index2 < (Log.u4MaxTimeMark + 1) ; u4Index2 += 1)
            {
                fprintf(pWriteFile,"\nTime Mark : %lu\n",u4Index2);
                for(u4Index = 0; u4Index < Log.u4LogDepth;u4Index += 1)
                {
                    if(Log.pPage[u4Index].u4TimeMark == u4Index2)
                    {
                        fprintf(pWriteFile,"%lu,%lu,%lu\n" , Log.pPage[u4Index].u4Sec , Log.pPage[u4Index].u4USec , Log.pPage[u4Index].u4TimeMark);
                    }
                }
            }

            fclose(pWriteFile);
            sync();

        break;
        default :
        break;
    }
}
#endif

#ifdef BUS_MONITOR
/*******************************************************************************
*Band width profile
********************************************************************************/

void BusTrafficLog(unsigned long u4LogCnt , unsigned long u4LogIntInms ,  unsigned long u4IsEnd)
{
    static int id;
    static MT6516_BM_CONF Conf = {0,0,0,0,0,0,0};
    static stSlot * pBuff = NULL;
    FILE * pWriteFile = NULL;
    unsigned long u4Index = 0;

    if(u4IsEnd)
    {
LOGE("\n\n Stop!!\n\n");
        if(read(id,(char *)pBuff, Conf.u4TimeSlotCnt) < 0)
        {
            LOGE("Read error because : %s\n" , strerror(errno));
        }
        close(id);

        //Save to card
        pWriteFile = fopen("//sdcard//BusTrafficLog.txt","a");
        if(NULL == pWriteFile)
        {
            LOGE("\n\nFail to open the file\n\n");
            free(pBuff);
            pBuff = NULL;
        }
#if 0
        fprintf(pWriteFile , "Sec,usec,Bus cycle counter,Trans of all,Trans of selected,");
        fprintf(pWriteFile , "Access amount of all,Access amount of selected,");
        fprintf(pWriteFile , "Access cycle of all,Access cycle of selected\n");
        for(;u4Index < Conf.u4TimeSlotCnt ; u4Index += 1)
        {
            fprintf(pWriteFile , "%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",pBuff[u4Index].u4Sec,
                pBuff[u4Index].u4Usec,pBuff[u4Index].u4BCNT,pBuff[u4Index].u4TACT,
                pBuff[u4Index].u4TSCT,pBuff[u4Index].u4WACT,pBuff[u4Index].u4WSCT,
                pBuff[u4Index].u4BACT,pBuff[u4Index].u4BSCT);
        }
#else
        fprintf(pWriteFile , "Sec,usec,ACNT,ICNT,GCNT,UCNT,CPCACNT,DPCACNT,DBEACNT,DBYACNT,HANGMAX,MAX\n");
        for(;u4Index < Conf.u4TimeSlotCnt ; u4Index += 1)
        {
            fprintf(pWriteFile , "%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",pBuff[u4Index].u4Sec,
                pBuff[u4Index].u4Usec,pBuff[u4Index].u4BCNT,pBuff[u4Index].u4TACT,
                pBuff[u4Index].u4TSCT,pBuff[u4Index].u4WACT,pBuff[u4Index].u4WSCT,
                pBuff[u4Index].u4BACT,pBuff[u4Index].u4BSCT,pBuff[u4Index].u4DBYACNT,
                pBuff[u4Index].u4HangMax,pBuff[u4Index].u4MAX);
        }
        fprintf(pWriteFile , "\n\n");
#endif
        fclose(pWriteFile);
//        sync();
        free(pBuff);
        pBuff = NULL;
    }
    else
    {
LOGE("\n\n Start!!\n\n");
        id = open(BM_DEVDIR,O_RDWR);
        if(id < 0)
        {
            LOGE("\n\n open bus monitor driver failed\n\n");
            return;
        }

        if(Conf.u4Masters > (GMC1_MASTERCNT + GMC2_MASTERCNT))
        	return;

        pBuff = (stSlot *)malloc(u4LogCnt*sizeof(stSlot));
        if(NULL == pBuff)
        {
            LOGE("fail to allocate memory\n");
            close(id);
            return;
        }

        Conf.u4EMIorGMC = 1;
        Conf.u4ScanLoop = 0;
        Conf.u4RW = (MT6516_BM_READ | MT6516_BM_WRITE);
        Conf.u4TimeSlotCnt = u4LogCnt;
        Conf.u4TimeSlotIntInms = u4LogIntInms;
        Conf.u4PrintInUART = 0;
        if(write(id,&Conf,sizeof(MT6516_BM_CONF)) < 0)
        {
            LOGE("Write error because : %s\n" , strerror(errno));
            free(pBuff);
            pBuff = NULL;
            close(id);
            return;
        }
        Conf.u4Masters += 1;
    }
}
#endif

