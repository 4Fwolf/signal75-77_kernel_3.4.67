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

#include "FDProvider.h"
#include "mHalCamFD.h"  //only for log
/*******************************************************************************
*
********************************************************************************/
camera_face_metadata_m*  
mHalFDProvider::
getFDResult()
{
     RWLock::AutoRLock _l(FDLock);
     return mFDResult; 
}


/*******************************************************************************
*
********************************************************************************/
mtk_face_info_m*  
mHalFDProvider::
getFaceInfo()
{
     RWLock::AutoRLock _l(FDLock);
     return mInfo; 
}


/*******************************************************************************
*
********************************************************************************/
MVOID
mHalFDProvider::
createInstance(MINT32 maxNum)
{
 
    FD_LOGD("max number of face %d", maxNum);
    if ( !mFDResult )
    {
        camera_face_m *faces;
        mFDResult = (camera_face_metadata_m *)malloc(sizeof(camera_face_metadata_m));
        faces = ( camera_face_m* )malloc(sizeof(camera_face_m) * maxNum);
        memset(faces, 0, sizeof(camera_face_m)*maxNum);
        mFDResult->faces = faces;
        mFDResult->number_of_faces = 0;
    }
    if ( !mInfo )
    {
        mInfo = (mtk_face_info_m *)malloc(sizeof(mtk_face_info_m) * maxNum);
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID
mHalFDProvider::
destroyInstance()
{
    if ( mFDResult != NULL ) {
        if (mFDResult->faces != NULL) {
            free(mFDResult->faces);
            mFDResult->faces = NULL;
        }
        free(mFDResult);
        mFDResult = NULL;
    }
    if ( mInfo != NULL ) {
        free(mInfo);
        mInfo = NULL;
    }

	delete this;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
mHalFDProvider::
dump()
{
    FD_LOGD("# face detected: %d", mFDResult->number_of_faces);
    for(MINT32 i = 0; i < mFDResult->number_of_faces; i++)
    {
    	FD_LOGD("# %d: [%d, %d, %d, %d]", i, mFDResult->faces->rect[0],
    										 mFDResult->faces->rect[1],
    										 mFDResult->faces->rect[2],
    										 mFDResult->faces->rect[3]);
    }
}


