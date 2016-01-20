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
/*
** $Log: mcam_mem.h $
 *
*/

#ifndef _MCAM_MEM_H_ 
#define _MCAM_MEM_H_

//
#include <stdlib.h>
#include <linux/cache.h>
#include "mcam_log.h"

/*******************************************************************************
*
********************************************************************************/
struct mHalCamMemPool {
    mHalCamMemPool(const char *poolName, int poolSize, int isPmem)
    : mVirtAddr(0), mPhyAddr(0), mPoolName(poolName), mPoolSize(poolSize), mPmemFd(0), 
      mAlignPoolSize(0)
    {
        const size_t pagesize = getpagesize();
        mAlignPoolSize = (mPoolSize + L1_CACHE_BYTES - 1) & ~(L1_CACHE_BYTES -1);   
        
        #if 0  //@carson: no pmem.h
        if (isPmem) {
            mVirtAddr = (unsigned long)(unsigned char *)pmem_alloc(mAlignPoolSize, &mPmemFd);
            mPhyAddr = (unsigned long) pmem_get_phys(mPmemFd);
            mPoolName = poolName; 
        }
        #endif
        //else {
            mVirtAddr = (unsigned long)(unsigned char *) memalign(L1_CACHE_BYTES, mAlignPoolSize); 
            mPoolName = poolName; 
            mPhyAddr = 0; 
            mPmemFd = -1; 
        //}
        MCAM_DBG("[mHalCamMemPool] poolName = %s , phyAddr = 0x%lx, virtAddr = 0x%lx, size = %d, PmemFD = %d\n", 
                                         mPoolName, mPhyAddr, mVirtAddr , mPoolSize, mPmemFd);                                                                                                         
    }; 

    //
    virtual ~mHalCamMemPool() {
        MCAM_DBG("[~mHalCamMemPool] poolName = %s , phyAddr = 0x%lx, virtAddr = 0x%lx, size = %d, PmemFD = %d\n", 
                                         mPoolName, mPhyAddr, mVirtAddr, mPoolSize, mPmemFd);     
        //
        if (mPmemFd == -1) { 
            if (mVirtAddr != 0) {
                free((unsigned char *)mVirtAddr); 
            }
        }
        #if 0 //carson no pmem.h
        else {
            if (mVirtAddr != 0) {
                pmem_free((unsigned char*) mVirtAddr, mAlignPoolSize, mPmemFd);
            }
        }
        #endif
        mPmemFd = -1;
        mVirtAddr = mPhyAddr = 0; 
    };
    
    //
    const char* getPoolName() {
        return mPoolName; 
    }

    //
    unsigned int getPoolSize() {
        return mPoolSize; 
    }

    //
    unsigned long getVirtAddr() {
        return mVirtAddr; 
    }

    //
    unsigned long getPhyAddr() {
        return mPhyAddr; 
    }

private:
     //
    unsigned long mVirtAddr; 
    unsigned long mPhyAddr; 
    const char *mPoolName;     
    unsigned int mPoolSize; 
    int mPmemFd;    
    //
    unsigned long mAlignPoolSize; 
    
}; 

#endif 
