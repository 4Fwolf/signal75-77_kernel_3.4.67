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

#define LOG_TAG "MHalCamBase/MHalCamUtils"
//
#include <utils/RWLock.h>
#include <utils/PropertyMap.h>
#include <cutils/xlog.h>

#include "CamUtilsProperty.h"

/******************************************************************************
*
*******************************************************************************/
#define CAM_LOGV(fmt, arg...)       XLOGV(fmt"\r\n", ##arg)
#define CAM_LOGD(fmt, arg...)       XLOGD(fmt"\r\n", ##arg)
#define CAM_LOGI(fmt, arg...)       XLOGI(fmt"\r\n", ##arg)
#define CAM_LOGW(fmt, arg...)       XLOGW(fmt"\r\n", ##arg)
#define CAM_LOGE(fmt, arg...)       XLOGE(fmt" (%s){#%d:%s}""\r\n", ##arg, __FUNCTION__, __LINE__, __FILE__)

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[CamPropertyMap::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[CamPropertyMap::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[CamPropertyMap::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[CamPropertyMap::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[CamPropertyMap::%s] "fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)

namespace android {
namespace MHalCamUtils {
namespace Property {
/******************************************************************************
*
*******************************************************************************/
namespace
{
    RWLock              gRWLock;
    PropertyMap         gPropertyMap;
};


/******************************************************************************
 *  Clears the all property map.
 *  
 *  return
 *      N/A
 *
 ******************************************************************************/
void
clear()
{
    RWLock::AutoWLock _l(gRWLock);
    gPropertyMap.clear();
    MY_LOGD("pid/tid=%d/%d", ::getpid(), ::gettid());
}


/******************************************************************************
 *  Set a property.
 *  Replaces the property with the same key if it is already present.
 *  
 *  return
 *      N/A
 *
 ******************************************************************************/
void
set(String8 const& key, String8 const& value)
{
    RWLock::AutoWLock _l(gRWLock);
    gPropertyMap.addProperty(key, value);
    MY_LOGD("pid/tid=%d/%d (%s)=(%s)", ::getpid(), ::gettid(), key.string(), value.string());
}

void
set(const char* key, const char* value)
{
    set( String8(key), String8(value));
}
/******************************************************************************
 *  Test if a specified key exists or not.
 *
 *  return
 *      Returns true if the property map contains the specified key.
 *
 ******************************************************************************/
bool
hasKey(String8 const& key)
{
    RWLock::AutoRLock _l(gRWLock);
    return  gPropertyMap.hasProperty(key);
}


/******************************************************************************
 *  Gets the value of a property and parses it.
 *
 *  return
 *      Returns true and sets outValue if the key was found and its value was 
 *      parsed successfully.
 *      Otherwise returns false and does not modify outValue.
 *
 ******************************************************************************/
bool
get(const String8& key, String8& outValue, const String8& def)
{
    RWLock::AutoRLock _l(gRWLock);
    if(  gPropertyMap.tryGetProperty(key, outValue) ){
        MY_LOGD("(String8)pid/tid=%d/%d (%s)=(%s)", ::getpid(), ::gettid(), key.string(), outValue.string());
        return true;
    }else{
        outValue = def;
        MY_LOGD("(String8)pid/tid=%d/%d (%s)=(%s)", ::getpid(), ::gettid(), key.string(), outValue.string());
        return false;
    }
}

bool
get(char const* key, char* outValue, char const* def)
{
    bool ret;
    String8 s8val;
    if( def ){
        ret = get( String8(key), s8val, String8(def) );
    }else{
        ret = get( String8(key), s8val );
    }
    memcpy( outValue, s8val.string(), (s8val.length() + 1) * sizeof( char ) );
    MY_LOGD("get (char)pid/tid=%d/%d (%s)=(%s)", ::getpid(), ::gettid(), key, outValue);
    return ret;
}

};  // namespace Property
};  // namespace MHalCamUtils
};  // namespace android

