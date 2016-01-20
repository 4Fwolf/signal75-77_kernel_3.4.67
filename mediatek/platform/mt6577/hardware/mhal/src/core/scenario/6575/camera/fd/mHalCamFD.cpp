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
#define LOG_TAG "mHalCamFD"

#include "mHalCamFD.h"

/******************************************************************************
*
******************************************************************************/
//!++ ICS FD
// called from mHalCamUnInit
MVOID 
mHalCamFD::
destroyInstance(){

    FD_LOGD("+");

    ///
    mCamDestroy = true;

    ///
    if (mpFDHalObj) {
        mpFDHalObj->destroyInstance();
        mpFDHalObj = NULL;
    }

    delete this;
	
    FD_LOGD("-");    
}


/******************************************************************************
*
******************************************************************************/
//!++ ICS FD
// called from mHalCamInit
MVOID 
mHalCamFD::
createInstance(HalFDObject_e type) {
	
    mpFDHalObj = halFDBase::createInstance(type); 
}

/******************************************************************************
*
******************************************************************************/
//!++ ICS FD
// called from preview_stop
MINT32
mHalCamFD::
Uninit() {

    FD_LOGD("+");

    MINT32 err = MHAL_NO_ERROR;
    ///
    err = mpFDHalObj->halFDUninit();
    if (err < 0) 
    {
       FD_LOGE("mHalFDUninit err");
    }   
   
    FD_LOGD("-");

    return err;
}  


/******************************************************************************
*
******************************************************************************/
//!++  ICS FD
// called from preview_start
MINT32
mHalCamFD::
Init(MBOOL fdRotate, MINT32 bufCnt, mHalCamMemPool *FDWorkingBuf) {

    FD_LOGD("+");
    MINT32 err = 0;
    
    // (1) [FD_HAL init]
    // needs rotation information; 
    // Once it is rotated, W and H should be switched

    mRotated = fdRotate;
    MINT32 fdH = mHalCamFD::FD_HEIGHT;
    MINT32 fdW = mHalCamFD::FD_WIDTH;
    
    if (mRotated) {
        err = mpFDHalObj->halFDInit(fdH, fdW);
    }
    else {
        err = mpFDHalObj->halFDInit(fdW, fdH);    
    }
    if (err < 0) {
        FD_LOGE("configFDPort] halFDInit fail"); 
        return err; 
    }

    // (2) 
    mFDWorkingBuf = FDWorkingBuf;
    #if defined (MTK_M4U_SUPPORT)
    err = mpFDHalObj->halFDM4URegister((MUINT8 *)mFDWorkingBuf->getVirtAddr(), fdH * fdW * 2, bufCnt);
    #endif


    // (3) enable SD: 1 
    mpFDHalObj->halSetDetectPara(1);

    return err;
}


/*******************************************************************************
*
********************************************************************************/
//!++ Revised: ICS FD
MVOID
mHalCamFD::waitFDdone()
{
    FD_LOGD("+");
    
    mFDthread.lock();
    
    while (mFDisRunning) {
        mWaitFD.wait(mFDthread);
    }

    mFDthread.unlock();

    FD_LOGD("-");
}


/*******************************************************************************
*
********************************************************************************/
//!++ Revised: ICS FD
MVOID
mHalCamFD::lock()
{
    FD_LOGD("+");
    mFDthread.lock();
    mFDisRunning = true;
    mFDthread.unlock();
    FD_LOGD("-");    
}


/*******************************************************************************
*
********************************************************************************/
//!++ Revised: ICS FD
MVOID
mHalCamFD::unlock()
{
    FD_LOGD("+");
    mFDthread.lock();

    mFDisRunning = false;
    mWaitFD.broadcast();
    
    mFDthread.unlock();    
    FD_LOGD("-");
}


/*******************************************************************************
*
********************************************************************************/
MVOID
mHalCamFD::setContentProvider(MVOID *mem1, MVOID *mem2){
	mfaceResult = mem1;
	mfaceInfo = mem2;
}

/*******************************************************************************
*
********************************************************************************/
//!++ Revised: ICS FD
MINT32
mHalCamFD::setFaceResult(MINT32 &isNewFaceIn, MINT32 &isNewSmileIn)
{
    FD_LOGD("+");
    
    MINT32 err = MHAL_NO_ERROR;
    isNewFaceIn = 0;
    isNewSmileIn = 0;
	
    RWLock::AutoWLock _l(FDLock);

    /* get FD/SD result */
    if (getFDstate()) {
        isNewFaceIn = mpFDHalObj->halFDGetFaceResult(reinterpret_cast<camera_face_metadata_m*>(mfaceResult), 0);
        mpFDHalObj->halFDGetFaceInfo(reinterpret_cast<mtk_face_info_m*>(mfaceInfo));
        if (getSDstate()) {
            isNewSmileIn = mpFDHalObj->halSDGetSmileResult();
        }  
    }

    FD_LOGD("-");
    return err;
}


/*******************************************************************************
*
********************************************************************************/
//!++ Revised: ICS FD
MVOID*
mHalCamFD::getFaceResult(){

    RWLock::AutoRLock _l(FDLock);
    return mfaceResult; 
}

/*******************************************************************************
*
********************************************************************************/
//!++ Revised: ICS FD
MVOID
mHalCamFD::dump() {

    if (mfaceResult == NULL || mfaceInfo == NULL) {
        return;
    }
    camera_face_metadata_m *result = reinterpret_cast<camera_face_metadata_m*>(mfaceResult);
	mtk_face_info_m *info = reinterpret_cast<mtk_face_info_m *>(mfaceInfo);
	
    FD_LOGD("# face detected: %d", result->number_of_faces);
    for(MINT32 i = 0; i < result->number_of_faces; i++)
    {
        FD_LOGD("# %d: [%d, %d, %d, %d]", i, result->faces->rect[0],
                                             result->faces->rect[1],
                                             result->faces->rect[2],
                                             result->faces->rect[3]);
		
		FD_LOGD("%d %d", info->rip_dir, info->rop_dir);
    }
}

