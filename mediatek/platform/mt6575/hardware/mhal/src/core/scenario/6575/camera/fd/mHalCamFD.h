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
#ifndef _MHAL_CAM_FD_H
#define _MHAL_CAM_FD_H
	 
#include <utils/threads.h>
#include <cutils/xlog.h>
#include <mcam_mem.h>
#include <mhal/inc/camera.h>
#include "fd_hal_base.h"

using namespace android;

/*******************************************************************************
*
********************************************************************************/
#define FD_LOGV(fmt, arg...)    XLOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define FD_LOGD(fmt, arg...)    XLOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define FD_LOGI(fmt, arg...)    XLOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define FD_LOGW(fmt, arg...)    XLOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define FD_LOGE(fmt, arg...)    XLOGE("[%s] "fmt, __FUNCTION__, ##arg)

/*******************************************************************************
*
********************************************************************************/
class mHalCamFD {

public:
    mHalCamFD() 
            : mfaceResult(NULL)
            , mfaceInfo(NULL)
            , mFDon (0)
            , mSDon (0)
            , mRotated(0)
            , mFDisRunning(false)
            , mCamDestroy(false)
            {}    
	 
    virtual ~mHalCamFD() {}
	 
    ////
    MVOID                     createInstance(HalFDObject_e type);	 
    MVOID                     destroyInstance();
    MINT32                    Init(MINT32 fdRotate, MINT32 bufCnt, mHalCamMemPool *FDWorkingBuf);
    MINT32                    Uninit();
    MVOID                     setCamDestroy() { mCamDestroy = true; }
    ////
    MVOID*                    getFaceResult();
    MINT32                    setFaceResult(MINT32 &isNewFaceIn, MINT32 &isNewSmileIn);
    MVOID                     setContentProvider(MVOID *mem1, MVOID *mem2);
    ///
    mHalCamMemPool*           getFDBuf() const { return mFDWorkingBuf;} // suppose no mutex is needed
    MBOOL                     getFDstate() const { return mFDon; }
    MVOID                     setFDstate(MBOOL s) { mFDon = s; }
    MINT32                    bufRotate() const { return mRotated; }
    MVOID                     setCallback(mHalCamObserver cb) { mcb = cb;}
    MVOID                     handleCallback(MINT32 type, void* data) {
                                  mcb.notify(type, data);
    					      }

/// T.B.D
    MVOID                     setDispInfo(MUINT32 x, MUINT32 y, MUINT32 w, 
								  MUINT32 h, MUINT32 rr, MUINT32 r, MUINT32 s)
                              {   
                                  mpFDHalObj->mHalFDSetDispInfo(x, y, w, h, rr, r, s); 
                              }
    MVOID                     dump();
    /// thread control 
    MVOID                     waitFDdone();  
    MVOID                     lock();
    MVOID                     unlock();
    
    /// SD
    MBOOL                     getSDstate() const { return mSDon; }
    MVOID                     setSDstate(MBOOL s) { mSDon = s; } 
    
    
    ///
    static const ssize_t FD_WIDTH = 320;
    static const ssize_t FD_HEIGHT = 240;
    static const ssize_t MAX_FACE_NUM = 15;
	
	 
private:
    MVOID                     *mfaceResult;
    MVOID                     *mfaceInfo;        
    MBOOL                     mFDon;
    MBOOL                     mSDon;
    MINT32                    mRotated;
    mHalCamObserver           mcb;
    
    /// for read/write FD result
    RWLock                    FDLock; 
    
    /// for FD proc thread control 
    mutable Mutex             mFDthread; 
    Condition                 mWaitFD;	 
    MBOOL                     mFDisRunning;
    mHalCamMemPool*           mFDWorkingBuf;
	 ////
 public:
    halFDBase*                mpFDHalObj; 
    MBOOL                     mCamDestroy;

};


#endif
