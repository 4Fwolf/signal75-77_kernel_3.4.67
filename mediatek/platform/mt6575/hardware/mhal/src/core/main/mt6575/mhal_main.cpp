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

//#define LOG_TAG "mHalMain"
#include <utils/Errors.h>
#include <utils/Timers.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>


#include "MediaHal.h"
#include "MediaLog.h"
#include "MediaAssert.h"
//
#include "mhal_main.h"
#include "mhal_jpeg.h"
#include "scenario_imagetransform.h" //For bitblt

#include "mtkfb.h"

//#include "multimediaFactoryTest.h"



#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/resource.h>

#ifdef _WIN32
#define LOGE(x)
#define LOGD(x)
#else
#include <cutils/xlog.h>
#undef LOG_TAG
#define LOG_TAG "mHalMain"
#define LOGD(fmt, arg...)  XLOGD(fmt, ##arg)
#define LOGE(fmt, arg...)  XLOGE(fmt, ##arg)

#endif


MINT32 mHalFBConfigImmediateUpdate(MINT32 enable);


/******************************************************************************
*
*******************************************************************************/
void*
mHalOpen()
{
    return  new mHalOpenInfo;
}


/******************************************************************************
*
*******************************************************************************/
void
mHalClose(void* fd)
{
    delete  reinterpret_cast<mHalOpenInfo*>(fd);
}


/******************************************************************************
*
*******************************************************************************/
int
mHalIoctl(
    void*           fd, 
    unsigned int    uCtrlCode, 
    void*           pvInBuf, 
    unsigned int    uInBufSize, 
    void*           pvOutBuf, 
    unsigned int    uOutBufSize, 
    unsigned int*   puBytesReturned
)
{
    MINT32 err = MHAL_NO_ERROR;
    //
    if  (uCtrlCode & MHAL_IOCTL_CAMERA_GROUP_MASK) {
        err = mHalCamIoctl(fd, uCtrlCode, pvInBuf, uInBufSize, pvOutBuf, uOutBufSize, puBytesReturned);
        return  err;
    }
    //
    //
    return  mHalIoCtrl(uCtrlCode, pvInBuf, uInBufSize, pvOutBuf, uOutBufSize, puBytesReturned);
}


/*******************************************************************************
*
********************************************************************************/
MINT32
mHalIoCtrl(
    MUINT32 a_u4CtrlCode,
    MVOID *a_pInBuffer,
    MUINT32 a_u4InBufSize,
    MVOID *a_pOutBuffer,
    MUINT32 a_u4OutBufSize,
    MUINT32 *pBytesReturned
)
{
    MINT32 err = MHAL_NO_ERROR;

    //MHAL_LOG("[mHalIoCtrl] %s, 0x%x \n\n", __TIME__, a_u4CtrlCode);

    if (a_u4CtrlCode & MHAL_IOCTL_VIDEO_GROUP_MASK) {
        //err = mHalVideoIoCtrl(a_u4CtrlCode, a_pInBuffer, a_u4InBufSize, a_pOutBuffer, a_u4OutBufSize, pBytesReturned);
        MHAL_LOG("No Implement !!!");
        return err;
    }

    switch (a_u4CtrlCode) {
    case MHAL_IOCTL_LOCK_RESOURCE:
        break;

    case MHAL_IOCTL_UNLOCK_RESOURCE:
        break;

    case MHAL_IOCTL_JPEG_DEC_SET_READ_FUNC:
        MHAL_LOG("No Implement !!!");
        break;

    case MHAL_IOCTL_JPEG_DEC_START:
        //err = mHalJpgDecStart((MHAL_JPEG_DEC_START_IN*)a_pInBuffer);
        MHAL_LOG("No Implement !!!");
        break;

    case MHAL_IOCTL_JPEG_DEC_GET_INFO:
        //err = mHalJpgDecGetInfo((MHAL_JPEG_DEC_INFO_OUT*)a_pOutBuffer);
        MHAL_LOG("No Implement !!!");
        break;

    case MHAL_IOCTL_JPEG_DEC_PARSER:
        //err = mHalJpgDecParser((unsigned char*)a_pInBuffer, a_u4InBufSize);
        MHAL_LOG("No Implement !!!");
        break;        

    case MHAL_IOCTL_JPEG_ENC:
        err = mHalJpgEncStart((MHAL_JPEG_ENC_START_IN*) a_pInBuffer);
        //MHAL_LOG("No Implement !!!");
        break;

    case MHAL_IOCTL_BITBLT:
        err = mHalBitblt(a_pInBuffer);
        break;

    case MHAL_IOCTL_DIRECT_BITBLT_PREPARE:
        //err= mHalDirectBitbltPrepare(a_pInBuffer);
        MHAL_LOG("No Implement !!!");
        break;

    case MHAL_IOCTL_DIRECT_BITBLT_END:
        //err= mHalDirectBitbltEnd(a_pInBuffer);
        MHAL_LOG("No Implement !!!");
        break;

    case MHAL_IOCTL_FB_CONFIG_IMEDIATE_UPDATE: 
        err = mHalFBConfigImmediateUpdate(*(MINT32 *) a_pInBuffer);
        break;

   case MHAL_IOCTL_FACTORY:
        //err= mHalFactory(a_pInBuffer);
        break;
         
    default:
        MHAL_ASSERT(0, "Err, unknown a_u4CtrlCode");
        break;
    }

    return err;
}

MINT32 mtk_AdjustPrio(MHAL_CHAR *name)
{

#ifdef CONFIG_PRIORITY_BY_PROC
    MINT32 fd;

    struct priority_data data;
    struct sched_param sched_set;

    strcpy(data.name, name);

    fd = open("/dev/priority", O_RDONLY, 0);

    if(fd == -1)
    {
        MHAL_LOG("Error: Cannot open the /dev/priority \n");
        return -1;
    }

    ioctl(fd, NULL, (int) &data);

    MHAL_LOG("Schedule policy : %d\n", data.policy);
    MHAL_LOG("Schedule priority : %d\n", data.priority);

    if(data.policy == SCHED_NORMAL)
    {
        int prio_process = 0;
        pid_t pid = gettid();

        sched_set.sched_priority = 0;
        MHAL_LOG("Schedule NORMAL !\n");

        if(0 != sched_setscheduler(pid, SCHED_NORMAL, &sched_set))
        {
            MHAL_LOG("Error: Set scheduler failed \n");
            return -1;
        }

        if(data.priority < 20 || data.priority >= -20)
        {
            setpriority(PRIO_PROCESS, pid, data.priority);
            MHAL_LOG("Set %d Schedule NORMAL priority %d!\n", pid, data.priority);
            prio_process = getpriority(PRIO_PROCESS, pid);
            MHAL_LOG("Get %d Schedule NORMAL priority %d!\n", pid, prio_process);
        }
        else
        {
            MHAL_LOG("Error: Priority exceed the region \n");
            close(fd);
            return -1;
        }
    }
    else if(data.policy == SCHED_FIFO || data.policy == SCHED_RR)
    {
        pid_t pid = gettid();

        if(data.priority > 99 || data.priority < 0)
        {
            MHAL_LOG("Error: Priority exceed the region \n");
            close(fd);
            return -1;
        }

        sched_set.sched_priority = data.priority;
        MHAL_LOG("Set %d Schedule Real-Time !\n", pid);
        if(0 != sched_setscheduler(pid, data.policy, &sched_set))
        {
            MHAL_LOG("Error: Set scheduler failed \n");
            close(fd);
            return -1;
        }
    }
    else
    {
        close(fd);
        return -1;
    }

    close(fd);
    MHAL_LOG("1. mtk_AdjustPrio for proc setting %s\n", name);
#else
    MHAL_LOG("2. mtk_AdjustPrio for header file %s\n", name);
#endif
    return 0;
}


MINT32 mHalFBConfigImmediateUpdate(MINT32 enable) 
{
    int fd = open("/dev/graphics/fb0", O_RDWR, 0);
    
    LOGD("[mHalFBConfigImmediateUpdate] %d \n", enable);
    
    if (fd >= 0) {
        unsigned int value = (enable ? 1 : 0);
        if (ioctl(fd, MTKFB_CONFIG_IMMEDIATE_UPDATE, value) == -1) {
            LOGE("[%s] ioctl failed, errno: %d", __func__, errno);
            return -1;
        }
        close(fd);
    } else {
        LOGE("[%s] open mtkfb failed. errno: %d", __func__, errno);
        return -1;
    }
    return 0;
}
