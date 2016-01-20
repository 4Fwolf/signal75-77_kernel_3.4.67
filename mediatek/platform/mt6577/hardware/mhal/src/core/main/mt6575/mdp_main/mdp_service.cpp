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
#include "mdp_datatypes.h"
#include "mdp_drv.h"

#ifdef MDP_FLAG_1_SUPPORT_MHALJPEG
#include "mhal_jpeg.h" 
#endif

#include <sys/mman.h>

#ifdef MDP_FLAG_1_SUPPORT_M4U
#include "m4u_lib.h"
#endif

#undef LOG_TAG
#define LOG_TAG "MdpService" 

#define JPEG_DEC_BS_PRE_FETCH_SIZE 256

namespace android {

int MdpService::decodeJpg(MHAL_JPEG_DEC_START_IN* inParams, void* procHandler)
{
    #ifdef MDP_FLAG_1_SUPPORT_MHALJPEG
    return mHalJpgDecStart(inParams, procHandler);
    #else
    return 0;
    #endif
}

int MdpService::parseJpg(unsigned char* addr, unsigned int size, int fd)
{
    #ifdef MDP_FLAG_1_SUPPORT_MHALJPEG
    return mHalJpgDecParser(addr, size);
    #else
    return 0;
    #endif
}

int MdpService::getJpgInfo(MHAL_JPEG_DEC_INFO_OUT* outParams)
{
    #ifdef MDP_FLAG_1_SUPPORT_MHALJPEG
    return mHalJpgDecGetInfo(outParams);
    #else
    return 0;
    #endif
}

int MdpService::BitBlt( mHalBltParam_t* bltParam )
{
    MDP_INFO_PERSIST("[MDP][IPC]BitBlt:Server Process\n");
    return mHalMdpLocal_BitBlt( bltParam );
}


static void* getProcHandler()
{
    void *handler = NULL;

    #ifdef MDP_FLAG_1_SUPPORT_M4U
#if 0
    MTKM4UDrv* pDrv = new MTKM4UDrv();

    handler = pDrv->m4u_get_task_struct();

    MDP_INFO_PERSIST("get proc handler %p", handler);
    delete pDrv;
#endif    
    #endif
    
    return handler;
}

status_t BnMdpService::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) 
{
    MHAL_JPEG_DEC_START_IN inParams;
    MHAL_JPEG_DEC_INFO_OUT outParams;
    unsigned char *srcAddr;
    int addr, size, fd;
    int enable;
    int ret;
    int proc_handler;
	unsigned int map_size ;

    MDP_INFO_PERSIST("receieve the command code %d", code);
    
    switch(code) 
    {
        case JPEG_DECODE:
            data.readInt32(&enable);
            if(enable == 0)
            {
                ret = decodeJpg(NULL, NULL);
            }
            else
            {
                data.read(&inParams, sizeof(MHAL_JPEG_DEC_START_IN));
                data.readInt32(&proc_handler);
                fd = data.readFileDescriptor();
                map_size = inParams.srcLength + JPEG_DEC_BS_PRE_FETCH_SIZE;
                srcAddr = (unsigned char *)mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                if(-1 == (long)srcAddr)
                {
                    MDP_ERROR(" mmap src addr error");
                    reply->writeInt32(JPEG_ERROR_INVALID_MEMORY);
                    break;
                }
                inParams.srcBuffer = srcAddr;
                ret = decodeJpg(&inParams, (void *)proc_handler);
                munmap(srcAddr, map_size);
            }
            //MDP_INFO_PERSIST("receieve the command code JPEG_DECODE w:%d h:%d ", inParams.dstWidth, inParams.dstHeight);

            reply->writeInt32(ret);
            break;
            
        case JPEG_DECODE_PARSE: 
            //mHalJpgDecParser(unsigned char* srcAddr, unsigned int srcSize);
            
            data.readInt32(&addr);
            data.readInt32(&size);
            fd = data.readFileDescriptor();

            srcAddr = (unsigned char *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if(-1 == (long)srcAddr)
            {
                MDP_ERROR(" mmap src addr error");
                reply->writeInt32(JPEG_ERROR_INVALID_MEMORY);
                break;
            }
            
            ret = parseJpg(srcAddr, size, fd);
            //MDP_INFO_PERSIST("parser ret : %d", ret);
            reply->writeInt32(ret);
            munmap(srcAddr, size);
            
            break;
            
        case JPEG_DECODE_INFO:
            ret = getJpgInfo(&outParams);
            reply->writeInt32(outParams.srcWidth);
            reply->writeInt32(outParams.srcHeight);
            reply->writeInt32(ret);
            //MDP_INFO_PERSIST("receieve the command code JPEG_DECODE_INFO %d %d (%d)", outParams.srcWidth, outParams.srcHeight, ret);
            break;

        case MDP_BITBLT:
            {
                mHalBltParam_t  bltParam;
                int             ret;

                MDP_INFO_PERSIST("receieve the command code MDP_BITBLT");
                
                data.read(&bltParam, sizeof(mHalBltParam_t));
                ret = BitBlt( &bltParam );
                reply->writeInt32(ret);
            }
            break;
            
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }

    return 0;
}


static bool checkProcessRunable()
{
    char acBuf[256];
    sprintf(acBuf, "/proc/%d/cmdline", getpid());
    FILE *fp = fopen(acBuf, "r");
    if (fp)
    {
        fread(acBuf, 1, sizeof(acBuf), fp);
        fclose(fp);
        if(strncmp(acBuf, "com.android.cts", 15) == 0)
        {
            return false;
        }
        else if (strncmp(acBuf, "system_server", 13) == 0)
        {
            return false;
        }
    }

    return true;
}

// client : proxy mdp class
class BpMdpService : public BpInterface<IMdpService> 
{
public:
    BpMdpService(const sp<IBinder>& impl) : BpInterface<IMdpService>(impl) {}

    virtual int decodeJpg(MHAL_JPEG_DEC_START_IN* inParams, void* procHandler);
    virtual int parseJpg(unsigned char* addr, unsigned int size, int fd);
    virtual int getJpgInfo(MHAL_JPEG_DEC_INFO_OUT* outParams);
    virtual int BitBlt( mHalBltParam_t* bltParam );

};

int BpMdpService::decodeJpg(MHAL_JPEG_DEC_START_IN* inParams, void* procHandler)
{
    Parcel data, reply;
    void *handler = NULL;
    unsigned int oom_adj ;

    #ifdef MDP_FLAG_1_SUPPORT_M4U

    MTKM4UDrv* pDrv = new MTKM4UDrv();
    
    #endif
    
    if(inParams == NULL) 
    {
        data.writeInt32(0);
    }
    else 
    {
        data.writeInt32(1);
        data.write(inParams, sizeof(MHAL_JPEG_DEC_START_IN));
#if 0        
        data.writeInt32((int)getProcHandler());
#else


    #ifdef MDP_FLAG_1_SUPPORT_M4U

    if( M4U_STATUS_OK != pDrv->m4u_get_task_struct(&handler, &oom_adj))
      MDP_INFO_PERSIST("get proc handler fail %p, adj %d", handler, oom_adj);
      
    MDP_INFO_PERSIST("get proc handler %p", handler);

    #endif

    data.writeInt32((int)handler);
    
#endif
        
        data.writeFileDescriptor(inParams->srcFD);
    }
    remote()->transact(JPEG_DECODE, data, &reply);

    #ifdef MDP_FLAG_1_SUPPORT_M4U

    if(inParams != NULL)
      if( M4U_STATUS_OK != pDrv->m4u_put_task_struct(oom_adj))
        MDP_INFO_PERSIST("restore proc oom_adj fail %p", oom_adj);
    
    delete pDrv;

    #endif
    
    int err; 
    reply.readInt32(&err);
    
    MDP_INFO_PERSIST("BpMdpService::decodeJpg reply:%d", err);
    
    return err;
}

int BpMdpService::parseJpg(unsigned char* addr, unsigned int size, int fd)
{
    Parcel data, reply;

    if(!checkProcessRunable()) return MHAL_INVALID_DRIVER;
    
    MDP_INFO_PERSIST("BpMdpService::parseJpg addr:%p, size:%d, fd:%d", addr, size, fd);
    data.writeInt32((int)addr);
    data.writeInt32((int)size);
    data.writeFileDescriptor(fd);
    
    remote()->transact(JPEG_DECODE_PARSE, data, &reply);
    
    int err; 
    reply.readInt32(&err);

    MDP_INFO_PERSIST("BpMdpService::parseJpg reply:%d", err);
    return err;
}

int BpMdpService::getJpgInfo(MHAL_JPEG_DEC_INFO_OUT* outParams)
{
    Parcel data, reply;
    int ret;
    //data.write(outParams, sizeof(MHAL_JPEG_DEC_INFO_OUT));
    remote()->transact(JPEG_DECODE_INFO, data, &reply);
    reply.readInt32(&ret);
    outParams->srcWidth = (unsigned int) ret;
    reply.readInt32(&ret);
    outParams->srcHeight= (unsigned int) ret;
    reply.readInt32(&ret);
    
    MDP_INFO_PERSIST("BpMdpService::getJpgInfo %d %d (%d)", outParams->srcWidth, outParams->srcHeight, ret);
    return ret;
}

int BpMdpService::BitBlt( mHalBltParam_t* bltParam )
{
    Parcel  data, reply;
    int     err; 
    
    if(bltParam == NULL) 
    {
        return -1;
    }
    else 
    {
        data.write(bltParam, sizeof(mHalBltParam_t));
    }
    remote()->transact(MDP_BITBLT, data, &reply);
    
    reply.readInt32(&err);

    MDP_INFO_PERSIST("[MDP][IPC]BitBlt:Client return %d\n",err);
    
    return err;
    
}


IMPLEMENT_META_INTERFACE(MdpService, "MdpService");


};
