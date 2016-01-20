/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <stdio.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <fcntl.h>
#include <sys/mman.h>


#include "jpeg_hal.h"
#include "jpeg_dec_hal.h"

#include "mdp_path.h"
#include "mdp_element.h"
#include "mdp_datatypes.h"

#include <cutils/xlog.h>



#ifdef USE_PMEM
#include <cutils/pmem.h>
#include <cutils/memutil.h>
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif

//#define JPEG_PROFILING

#define LOG_TAG "JpgDecHal"

#define JPG_LOG(fmt, arg...)    XLOGW(fmt, ##arg)
#define JPG_DBG(fmt, arg...)    XLOGD(fmt, ##arg)
//#define JPG_DBG(fmt, arg...)

#define JPG_MDP_MAX_ELEMENT 3
#define JPEG_DEC_BS_PRE_FETCH_SIZE 256

class JpgDecMdpPath : public MdpPath_I
{
private:
    //MDP Drv operation
    int              m_mdp_element_count;
    MDPELEMENT_I**   p_mdp_element_list;
    
public:
    JpgDecMdpPath():m_mdp_element_count(0) { MdpDrvInit(); }
        
    virtual ~JpgDecMdpPath() { MdpDrvRelease(); }

protected:/*[MdpPath_I]*/
    virtual MDPELEMENT_I**  mdp_element_list()  {   return p_mdp_element_list; }
    virtual int             mdp_element_count() {   return m_mdp_element_count; }  
    virtual const char*     name_str(){     return  "JpgDecMdpPath";  }

public:
    void Config(MDPELEMENT_I** pList, int count) {
        p_mdp_element_list = pList;
        m_mdp_element_count = count;
    }
};

static bool fail_return(const char msg[]) {
#if 1
    JPG_LOG("[JPEG Decoder] - %s", msg);
#endif
    return false;   // must always return false
}

JpgDecHal::JpgDecHal()
{
    JPG_DBG("JpgDecHal::JpgDecHal");

#ifdef USE_PMEM
    fSrcPmemVA = NULL;
    fDstPmemVA = NULL;
#else
    pM4uDrv = new MTKM4UDrv();
    fSrcMVA = 0;
    if (pM4uDrv == NULL) {
        JPG_LOG("new MTKM4UDrv() fail \n");
    }
#endif
    islock = false;
    isDither = false;
    isRangeDecode = false;

    fOutWidth = fOutHeight = 0;
    fJpgWidth = fJpgHeight = 0;
    fSrcAddr = fDstAddr = NULL;
    fProcHandler = NULL;
}
 
JpgDecHal::~JpgDecHal()
{
    JPG_DBG("JpgDecHal::~JpgDecHal");

    unlock();

#ifdef USE_PMEM
        if(fSrcPmemVA != NULL) {
            pmem_free(fSrcPmemVA, fSrcSize, fSrcPmemFD);
            fSrcPmemVA = NULL;
        }
        if(fDstPmemVA != NULL) {
            pmem_free(fDstPmemVA, fDstSize, fDstPmemFD);
            fDstPmemVA = NULL;
        }
#else
        if (pM4uDrv) {
            if(fSrcMVA != 0) {
                pM4uDrv->m4u_invalid_tlb_range(M4U_CLNTMOD_JPEG_DEC, fSrcMVA, fSrcMVA + fSrcSize + JPEG_DEC_BS_PRE_FETCH_SIZE  - 1);
                pM4uDrv->m4u_dealloc_mva(M4U_CLNTMOD_JPEG_DEC, (unsigned int)fSrcAddr , fSrcSize + JPEG_DEC_BS_PRE_FETCH_SIZE, fSrcMVA);
                fSrcMVA = 0;
            }
            delete pM4uDrv;
        }
        pM4uDrv = NULL;
#endif   

	
}

void JpgDecHal::setRangeDecode(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom)
{
    isRangeDecode = true;
    fLeft = left;
    fTop = top;
    fRight = right;
    fBottom = bottom;
}

//-----------------------------------------------------------------------------
// Use BRZ->CAM->CRZ->IPP->ROTDMA0
//-----------------------------------------------------------------------------
bool JpgDecHal::lock()
{
    if(JPEG_DEC_STATUS_OK != jpegDecLockDecoder(&decID)) {
        return fail_return("Jpeg decoder resource is busy");
    }

    islock = true;
    return true;
}

bool JpgDecHal::unlock()
{    
    if(islock)
    {
      
        jpegDecUnlockDecoder(decID);
        islock = false;
    }
    
    return true;
}

bool JpgDecHal::checkParam()
{
    if(fJpgWidth == 0 || fJpgHeight == 0)
    {
        JPG_LOG("Invalide JPEG width/height %u/%u", fJpgWidth, fJpgHeight);
        return false;
    }
    
    if(fOutWidth < 3 || fOutHeight < 3)
    {
        JPG_LOG("Invalide width/height %u/%u [Range 3~4095]", fOutWidth, fOutHeight);
        return false;
    }

    if(fOutWidth > 4095 || fOutHeight > 4095)
    {
        JPG_LOG("Invalide width/height %u/%u [Range 3~4095]", fOutWidth, fOutHeight);
        return false;
    }

    if(fSrcAddr == NULL || fDstAddr == NULL) 
    {
        return fail_return("Invalide Address");
    }

    return true;
}


bool JpgDecHal::parse()
{
    if(fSrcAddr == NULL)
    {
        return fail_return("source address is null");
    }

    JPEG_FILE_INFO_IN info;   
    if(JPEG_DEC_STATUS_OK != jpegDecSetSourceFile(decID, fSrcAddr, fSrcSize))
    {
        return false;
    }

    if(JPEG_DEC_STATUS_OK != jpegDecGetFileInfo(decID, &info))
    {
        return false; 
    }

    fJpgWidth = info.width;
    fJpgHeight = info.height;
        
    return true;
}

#define CRZ_PATH 0
#define VRZ1_PATH 1
bool JpgDecHal::onStart()
{
    // MDP Class
    JpgDecMdpPath   mdp_path;
    MDPELEMENT_I*   mdp_element_list[5];
    int             mdp_element_count = 5;
    
    // MDP Elements 
    BRZ         me_brz;
    CRZ         me_crz;
    IPP         me_ipp;
    VRZ0        me_vrz0;
    RGBROT1EX   me_rgbrot1;

    VRZ1        me_vrz1;
    RGBROT2EX     me_rgbrot2;

    //PRZ0        me_prz;
    //RGBROT0     me_rgbrot0;

    unsigned long u4Path;
    int i4CheckVal;

    JPEG_FILE_INFO_IN info;
    if(JPEG_DEC_STATUS_OK != jpegDecGetFileInfo(decID, &info))
    {
        return false;
    }

    // Config BRZ
    me_brz.input_format = info.samplingFormat;
    if(info.samplingFormat == 422 && info.y_vSamplingFactor == 2)
    {
        me_brz.input_format = 421;
    }
    me_brz.shrink_factor = 0;
    me_brz.src_img_size.w = info.padded_width;
    me_brz.src_img_size.h = info.padded_height;
//    me_brz.to_vrz1 = 1;
//    me_brz.to_cam = 1;

    // Config CRZ
    me_crz.src_sel = 1; // 0:RDMA0 , 1:BRZ, 2:Camera
    me_crz.src_img_size.w = info.padded_width;
    me_crz.src_img_size.h = info.padded_height;
    me_crz.src_img_roi.x = 0;
    me_crz.src_img_roi.y = 0;
    me_crz.src_img_roi.w = info.width;
    me_crz.src_img_roi.h = info.height;
    me_crz.dst_img_size.w = fOutWidth;
    me_crz.dst_img_size.h = fOutHeight;

    // Config IPP
    me_ipp.bBypass = 1;
    me_ipp.src_sel = 1; //0 : OVL, 1:CRZ
    me_ipp.to_vrz0 = 1;

    // Config VRZ0
    me_vrz0.src_sel = 2;// 0-MOUT, 1-BRZ_MOUT, 2-IPP_MOUT, 3-OVL_DMA_MIMO, 4-PRZ0_MOUT
    me_vrz0.bBypass = 1;

    //Config VRZ1
    me_vrz1.src_sel = 1;// 0-R_DMA1, 1-BRZ_MOUT
    me_vrz1.bBypass = 0;
    me_vrz1.src_img_size.w = info.padded_width;
    me_vrz1.src_img_size.h = info.padded_height;
    me_vrz1.dst_img_size.w = fOutWidth * info.padded_width / fJpgWidth;
    me_vrz1.dst_img_size.h = fOutHeight * info.padded_height / fJpgHeight;
    me_vrz1.bContinuous = 0;
    me_vrz1.bCamIn = 0;

    // Config RGB_ROT1
    // me_rgbrot.src_sel = 0; //0-PRZ0_MOUT ,1-IPP_MOUT
    /*
    me_rgbrot1.bDithering = isDither ? 1 : 0;
    //me_rgbrot.src_img_size.w = fOutWidth;
    //me_rgbrot.src_img_size.h = fOutHeight;
    me_rgbrot1.src_img_roi.x = 0;
    me_rgbrot1.src_img_roi.y = 0;
    me_rgbrot1.src_img_roi.w = fOutWidth;
    me_rgbrot1.src_img_roi.h = fOutHeight;
    me_rgbrot1.dst_img_yuv_addr[0].y = fDstConfigAddr;
*/

    me_rgbrot1.bDithering = isDither ? 1 : 0;
    me_rgbrot1.src_img_size.w = fOutWidth;
    me_rgbrot1.src_img_size.h = fOutHeight;
    me_rgbrot1.src_img_roi.x = 0;
    me_rgbrot1.src_img_roi.y = 0;
    me_rgbrot1.src_img_roi.w = fOutWidth;
    me_rgbrot1.src_img_roi.h = fOutHeight;
    me_rgbrot1.dst_img_yuv_addr[0].y = fDstConfigAddr;
    me_rgbrot1.dst_img_yuv_addr[0].m4u_handle = (unsigned long)fProcHandler;
    me_rgbrot1.dst_img_size.w = fOutWidth;
    me_rgbrot1.dst_img_size.h = fOutHeight;
    me_rgbrot1.dst_img_roi.x = 0;
    me_rgbrot1.dst_img_roi.y = 0;
    me_rgbrot1.dst_img_roi.w = fOutWidth;
    me_rgbrot1.dst_img_roi.h = fOutHeight;
    me_rgbrot1.uOutBufferCnt = 1;

    if(isRangeDecode)
    {
        me_rgbrot1.src_img_roi.x = fLeft * fOutWidth / info.width;
        me_rgbrot1.src_img_roi.y = fTop * fOutWidth / info.width;
        me_rgbrot1.src_img_roi.w = (fRight - fLeft) * fOutWidth / info.width;
        me_rgbrot1.src_img_roi.h = (fBottom - fTop) * fOutWidth / info.width;

        me_rgbrot1.dst_img_size.w = me_rgbrot1.src_img_roi.w;
        me_rgbrot1.dst_img_size.h = me_rgbrot1.src_img_roi.h;
        me_rgbrot1.dst_img_roi.x = 0;
        me_rgbrot1.dst_img_roi.y = 0;
        me_rgbrot1.dst_img_roi.w = me_rgbrot1.src_img_roi.w;
        me_rgbrot1.dst_img_roi.h = me_rgbrot1.src_img_roi.h;
        JPG_DBG("configure out range addr:0x%x, (x,y):[%d %d] (w, h):[%d %d]  dd", me_rgbrot1.dst_img_yuv_addr[0].y,
                                                                        me_rgbrot1.src_img_roi.x, me_rgbrot1.src_img_roi.y,
                                                                        me_rgbrot1.src_img_roi.w, me_rgbrot1.src_img_roi.h);
    }


    switch (fOutFormat)
    {
        case kRGB_565_Format:
            me_rgbrot1.dst_color_format = RGB565;
            me_rgbrot2.dst_color_format = RGB565;
            break;

        case kRGB_888_Format:
            me_rgbrot1.dst_color_format = RGB888;
            me_rgbrot2.dst_color_format = RGB888;
            break;

        case kBGR_888_Format:
            me_rgbrot1.dst_color_format = BGR888;
            me_rgbrot2.dst_color_format = BGR888;
            break;

        case kARGB_8888_Format:
            me_rgbrot1.dst_color_format = ABGR8888;
            me_rgbrot2.dst_color_format = ABGR8888;
            break;
            
        case kABGR_8888_Format:
            me_rgbrot1.dst_color_format = ABGR8888;
            me_rgbrot2.dst_color_format = ABGR8888;
            break;

        case kUYVY_Pack_Format:
            me_rgbrot1.dst_color_format = UYVY_Pack;
            me_rgbrot2.dst_color_format = UYVY_Pack;
            break;

        case kYUY2_Pack_Format:
            me_rgbrot1.dst_color_format = YUY2_Pack;
            me_rgbrot2.dst_color_format = YUY2_Pack;
            break;
            
        default :
            break;
    }

    // Config RGB_ROT2
    me_rgbrot2.bDithering = isDither ? 1 : 0;
    me_rgbrot2.src_img_size.w = me_vrz1.dst_img_size.w;
    me_rgbrot2.src_img_size.h = me_vrz1.dst_img_size.h;
    me_rgbrot2.src_img_roi.x = 0;
    me_rgbrot2.src_img_roi.y = 0;
    me_rgbrot2.src_img_roi.w = fOutWidth;
    me_rgbrot2.src_img_roi.h = fOutHeight;
    me_rgbrot2.dst_img_yuv_addr[0].y = fDstConfigAddr;
    me_rgbrot2.dst_img_yuv_addr[0].m4u_handle = (unsigned long)fProcHandler;
    me_rgbrot2.dst_img_size.w = fOutWidth;
    me_rgbrot2.dst_img_size.h = fOutHeight;
    me_rgbrot2.dst_img_roi.x = 0;
    me_rgbrot2.dst_img_roi.y = 0;
    me_rgbrot2.dst_img_roi.w = fOutWidth;
    me_rgbrot2.dst_img_roi.h = fOutHeight;
    me_rgbrot2.uOutBufferCnt = 1;

#if 0
    // Config PRZ
    me_prz.src_sel = 3; // 0-MOUT,1-IPP_MOUT,2-CAM,3-BRZ_MOUT
    me_prz.to_rgb_rot0 = 1;
    me_prz.src_img_size.w = info.padded_width;
    me_prz.src_img_size.h = info.padded_height;
    me_prz.src_img_roi.x = 0;
    me_prz.src_img_roi.y = 0;
    me_prz.src_img_roi.w = info.width;
    me_prz.src_img_roi.h = info.height;
    me_prz.dst_img_size.w = fOutWidth;
    me_prz.dst_img_size.h = fOutHeight;

    // Config RGB_ROT0
    me_rgbrot0.src_sel = 0; //0-PRZ0_MOUT ,1-IPP_MOUT
    me_rgbrot0.bDithering = isDither ? 1 : 0;
    me_rgbrot0.src_img_roi.x = 0;
    me_rgbrot0.src_img_roi.y = 0;
    me_rgbrot0.src_img_roi.w = fOutWidth;
    me_rgbrot0.src_img_roi.h = fOutHeight;
    me_rgbrot0.dst_img_yuv_addr[0].y = fDstConfigAddr;
    me_rgbrot0.dst_img_size.w = fOutWidth;
    me_rgbrot0.dst_img_size.h = fOutHeight;
    me_rgbrot0.uOutBufferCnt = 1;
    
    switch (fOutFormat)
    {
        case kRGB_565_Format:
            me_rgbrot0.dst_color_format = RGB565;
            break;

        case kRGB_888_Format:
            me_rgbrot0.dst_color_format = RGB888;
            break;

        case kBGR_888_Format:
            me_rgbrot0.dst_color_format = BGR888;
            break;

        case kARGB_8888_Format:
            me_rgbrot0.dst_color_format = ARGB8888;
            break;
            
        case kABGR_8888_Format:
            me_rgbrot0.dst_color_format = ABGR8888;
            break;

        case kYUY2_Pack_Format:
            me_rgbrot0.dst_color_format = YUY2_Pack;
            break;

        case kUYVY_Pack_Format:
            me_rgbrot0.dst_color_format = UYVY_Pack;
            break;
            
        default :
            break;
    }
#endif

    //Select jpeg decode path : scale up or large image ( > 640x480) 
    if(isRangeDecode || (fOutWidth > info.padded_width) || (fOutWidth > 640))
    {
        u4Path = CRZ_PATH;
        me_brz.to_cam = 1;
        mdp_element_list[0] = (MDPELEMENT_I*)&me_brz;
        mdp_element_list[1] = (MDPELEMENT_I*)&me_crz;
        mdp_element_list[2] = (MDPELEMENT_I*)&me_ipp;
        mdp_element_list[3] = (MDPELEMENT_I*)&me_vrz0;
        mdp_element_list[4] = (MDPELEMENT_I*)&me_rgbrot1;
        mdp_element_count = 5;
    }
    else
    {
        u4Path = VRZ1_PATH;
        me_brz.to_vrz1 = 1;
        mdp_element_list[0] = (MDPELEMENT_I*)&me_brz;
        mdp_element_list[1] = (MDPELEMENT_I*)&me_vrz1;
        mdp_element_list[2] = (MDPELEMENT_I*)&me_rgbrot2;
        mdp_element_count = 3;
    }

    mdp_path.Config(mdp_element_list, mdp_element_count);
    
    if(JPEG_DEC_STATUS_OK != jpegDecSetSourceSize(decID, fSrcConfigAddr, fSrcSize)) 
    {
        return fail_return("JPEG Configure Driver fail");    
    }

    // Trigger MDP
    i4CheckVal = mdp_path.Start(NULL);

    if(0 != i4CheckVal)
    {
        if(!isRangeDecode && (MDP_ERROR_CODE_LOCK_RESOURCE_FAIL == i4CheckVal) && (CRZ_PATH == u4Path) && ((fOutWidth < 960)))
        {
            mdp_path.End(NULL);

            //960 is VRZ1 sysram output limitation.
            //Let's try another path
            JPG_LOG("No resource, try another path!!");
            u4Path = VRZ1_PATH;
            me_brz.to_cam = 0;
            me_brz.to_vrz1 = 1;
            mdp_element_list[0] = (MDPELEMENT_I*)&me_brz;
            mdp_element_list[1] = (MDPELEMENT_I*)&me_vrz1;
            mdp_element_list[2] = (MDPELEMENT_I*)&me_rgbrot2;
            mdp_element_count = 3;
            mdp_path.Config(mdp_element_list, mdp_element_count);            
            i4CheckVal = mdp_path.Start(NULL);
        }

        mdp_path.End(NULL);

        if(0 != i4CheckVal)
        {
            return fail_return("MDP Start Error");
        }
    }

    // Trigger & Wait JPEG Decoder
    // Set Timeout accroding to image pixels
    JPEG_DEC_RESULT_ENUM result;
    long time_out;
    time_out = (fJpgWidth * fJpgHeight * 100) / (1024 * 1024);
    if(time_out > 3000) time_out = 3000;
    if(time_out < 100) time_out = 100;

    if(JPEG_DEC_STATUS_OK != jpegDecStart(decID, time_out, &result)) 
    {
        mdp_path.End(NULL);
        return fail_return("JPEG Decoder Start fail");
    }
    
    
    JPG_DBG("jpeg decoder result:%d ", result);

    // Wait MDP
    if(JPEG_DEC_RESULT_DONE == result)
    {
        if(0 != mdp_path.WaitBusy(NULL) )
        {
            //mdp_path.DumpRegister(NULL);
            mdp_path.End(NULL);
            return fail_return("Wait MDP Done Error");
        }
    }
    
    mdp_path.End(NULL);
    
    if(JPEG_DEC_RESULT_DONE != result)
    {
        return fail_return("JPEG Decode Fail");
    }

    return true;

}

bool JpgDecHal::start()
{
    //Check param
    if(true != checkParam()) {
        return false;
    }
#ifdef JPEG_PROFILING
    struct timeval t1, t2;
#endif
    JPG_DBG("JpgDecHal::start -> config jpeg path");
    JPG_DBG("Decoder Src Addr:0x%x, width/height:[%u, %u], bf_size %x", (unsigned int)fSrcAddr, fJpgWidth, fJpgHeight, fSrcSize + JPEG_DEC_BS_PRE_FETCH_SIZE);
    JPG_DBG("Decoder Dst Addr:0x%x, width/height:[%u, %u], format:%u", (unsigned int)fDstAddr, fOutWidth, fOutHeight, fOutFormat);
    JPG_DBG("Decoder Dither:%d, RangeDecode:%d [%d %d %d %d]", isDither, isRangeDecode, fLeft, fTop, fRight, fBottom);

#ifdef USE_PMEM
    unsigned char *fSrcPmemVA;
    unsigned char *fDstPmemVA;
    int fSrcPmemFD;
    int fDstPmemFD;

    // Allocate Source
    fSrcPmemVA = (unsigned char *)pmem_alloc_sync(fSrcSize, &fSrcPmemFD);
    if(fSrcPmemVA == NULL) {
        return fail_return("Can not allocate PMEM\n");
    }
    fSrcConfigAddr = (UINT32)pmem_get_phys(fSrcPmemFD);
    memcpy((void*)fSrcPmemVA, fSrcAddr, fSrcSize);
    
    JPG_DBG("Allocate Source Pmem, va:0x%x, pa:0x%x, size:%u", fSrcPmemVA, fSrcConfigAddr, fSrcSize);

    
    fDstPmemVA= (unsigned char *)pmem_alloc_sync(fDstSize, &fDstPmemFD);
    if(fDstPmemVA == NULL) {
        return fail_return("Can not allocate PMEM\n");
    }
    fDstConfigAddr = (unsigned int)pmem_get_phys(fDstPmemFD);
    
    JPG_DBG("Allocate Destination Pmem, va:0x%x, pa:0x%x, size:%u", fDstPmemVA, fDstConfigAddr, fDstSize);
#else

#ifdef JPEG_PROFILING
    gettimeofday(&t1, NULL);
#endif
    if(M4U_STATUS_OK != pM4uDrv->m4u_reset_mva_release_tlb(M4U_CLNTMOD_JPEG_DEC))
    {
        fSrcMVA = 0;
        return fail_return("Can not reset m4u mva release tlb\n");        
    }
    
    if(M4U_STATUS_OK != pM4uDrv->m4u_alloc_mva(M4U_CLNTMOD_JPEG_DEC, (unsigned int)fSrcAddr , fSrcSize + JPEG_DEC_BS_PRE_FETCH_SIZE, &fSrcMVA))
    {
        fSrcMVA = 0;
        return fail_return("Can't allocate mva\n");       
    }

    if(M4U_STATUS_OK != pM4uDrv->m4u_insert_tlb_range(M4U_CLNTMOD_JPEG_DEC, fSrcMVA, fSrcMVA + fSrcSize + JPEG_DEC_BS_PRE_FETCH_SIZE - 1, RT_RANGE_HIGH_PRIORITY, 1))
    {
        pM4uDrv->m4u_dealloc_mva(M4U_CLNTMOD_JPEG_DEC, (unsigned int)fSrcAddr , fSrcSize + JPEG_DEC_BS_PRE_FETCH_SIZE, fSrcMVA);
        fSrcMVA = 0;
        return fail_return("Can't insert tlb range\n"); 
    }
            
    fSrcConfigAddr = fSrcMVA;
    
    M4U_PORT_STRUCT m4uPort;

    m4uPort.ePortID = M4U_PORT_JPEG_DEC;
    m4uPort.Virtuality = 1;
    m4uPort.Security = 0;
    m4uPort.Distance = 1;
    m4uPort.Direction = 0;

    if(M4U_STATUS_OK != pM4uDrv->m4u_config_port(&m4uPort))
    {
        return fail_return("Can config m4u port\n");         
    }

    pM4uDrv->m4u_cache_sync(M4U_CLNTMOD_JPEG_DEC, M4U_CACHE_FLUSH_BEFORE_HW_READ_MEM, (unsigned int)(void *)fSrcAddr, fSrcSize + JPEG_DEC_BS_PRE_FETCH_SIZE);

#ifdef JPEG_PROFILING
    gettimeofday(&t2, NULL);
    JPG_LOG("Jpeg HW config m4u time : %u", (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));
#endif

    fDstConfigAddr = (unsigned int)(void *)fDstAddr;
#endif


#ifdef JPEG_PROFILING
    gettimeofday(&t1, NULL);
#endif

    if(true == onStart()) {      
#ifdef USE_PMEM
        memcpy(fDstAddr, fDstPmemVA, fDstSize);
#endif
    } else {
        return false;
    }
    
#ifdef JPEG_PROFILING
    gettimeofday(&t2, NULL);
    JPG_LOG("Jpeg HW decode time : %u", (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));
#endif

    return true;
}

