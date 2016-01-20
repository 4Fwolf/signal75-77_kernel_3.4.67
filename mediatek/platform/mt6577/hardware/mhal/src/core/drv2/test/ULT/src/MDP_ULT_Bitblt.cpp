/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#include <stdlib.h>
#include <cutils/log.h>
#include <cutils/pmem.h>
#include <cutils/memutil.h>
#include "MediaHal.h"
#include "jpeg_dec_hal.h"
#include "MT6573MDPDrv.h"
#include "scenario_imagetransform.h"


#include "MDP_ULT.h"
#include "MDP_ULT_Bitblt.h"

#include <SkImageEncoder.h>
#include <SkBitmap.h>


#if 1
#define ulog(...) \
        do { \
            LOGD(__VA_ARGS__); \
        } while (0)
#else
#define ulog printf
#endif


#undef LOG_TAG
#define LOG_TAG "mdp_ult"


#define ULT_ASSERT(x) if(!(x)){ulog("__assert fail, file:%s, line:%d", __FILE__, __LINE__);}


#define BITBLT_ULT_USE_PMEM

/***********************************************
                .so resource
 ***********************************************/

#if defined MDPULT_RES_RGB565_16X16
extern unsigned char rgb565_16x16[];
#endif

#if defined MDPULT_RES_RGB565_399x199
extern unsigned char rgb565_399x199[];
#endif

#if defined MDPULT_RES_RGB565_80x80
extern unsigned char rgb565_80x80[];
#endif

#if defined MDPULT_RES_RGB888_80x80
extern unsigned char rgb888_80x80[];
#endif

#if defined MDPULT_RES_RGB565_256x256
extern unsigned char rgb565_256x256[];
#endif

#if defined MDPULT_RES_ARGB8888_80x80
extern unsigned char argb8888_a80_80x80[];
#endif

#if defined MDPULT_RES_MTKYUV_352x288
extern unsigned char yuv420_block_planar_352x288[];
#endif

#if defined MDPULT_RES_YUV420_240x416
extern unsigned char yuv420_scanline_planar_240x416[];
#endif

#if defined MDPULT_RES_NV21_176x220
extern unsigned char nv21_176x220[];
#endif

#if defined MDPULT_RES_YUYV422_104x60
extern unsigned char yuyv422_104x60[];
#endif




typedef struct {
	bool clipEn;
	int startX;
	int startY;
    int clipW;
	int clipH;
	bool pitchEn;
	int pitch;
	MHAL_BITBLT_ROT_ENUM rot;
} stBitBltParam;



/***********************************************
               internal function 
 ***********************************************/


static int saveFile(unsigned char* buf,int bufSize,
	   				int width, int height, 
					char* filename, int fmt, bool onlyRaw)
{
	if (buf == NULL) return 0;
	//if (fmt != MHAL_FORMAT_RGB_565 && fmt != MHAL_FORMAT_ARGB_8888 || onlyRaw)
	if ((fmt != MHAL_FORMAT_RGB_565 && 
		 fmt != MHAL_FORMAT_RGB_888 &&
	     fmt != MHAL_FORMAT_ARGB_8888) || onlyRaw)
	{
		//ulog("can not save as png! unsupport format\n");
    	ulog("[ULT]saving file as RAW in %s ... ", filename);
		strcat(filename, ".bin");
		FILE* fp;
		int index;
		fp = fopen(filename, "w");
    	for(index = 0 ; index < bufSize ; index++) {
        	fprintf(fp, "%c", buf[index]);
    	}
    	fclose(fp);
		return 0;
	}

    SkBitmap bmp;              // use Skia to encode raw pixel data into image file
   	SkBitmap bmp_argb; 
    ulog("[ULT]saving file as PNG in %s ... ", filename);
	strcat(filename, ".png");
    
	//bmp.setConfig(SkBitmap::kARGB_8888_Config, sc.getWidth(), sc.getHeight());
	if (fmt == MHAL_FORMAT_RGB_565) {
		bmp.setConfig(SkBitmap::kRGB_565_Config, width, height);
    	bmp.setPixels((void*)buf);


		if (!bmp.copyTo(&bmp_argb, SkBitmap::kARGB_8888_Config)) {
			ulog("[ULT]save as png error!! @ tanslate to argb\n");
		}		
		
		SkImageEncoder::EncodeFile(filename, bmp_argb, 
				SkImageEncoder::kPNG_Type, SkImageEncoder::kDefaultQuality);
	} else if (fmt == MHAL_FORMAT_ARGB_8888){
		bmp.setConfig(SkBitmap::kARGB_8888_Config, width, height);
    	bmp.setPixels((void*)buf);

		SkImageEncoder::EncodeFile(filename, bmp, 
				SkImageEncoder::kPNG_Type, SkImageEncoder::kDefaultQuality);
	} else if (fmt == MHAL_FORMAT_RGB_888) {
		unsigned char* tmpARGB = (unsigned char*)malloc(width * height * 4);
		memset(tmpARGB, 0xff, width * height * 4);

		for (int i=0; i < width*height; i++) {
			tmpARGB[4*i + 2] = buf[3*i + 0];	
			tmpARGB[4*i + 1] = buf[3*i + 1];	
			tmpARGB[4*i + 0] = buf[3*i + 2];	
		}
		bmp.setConfig(SkBitmap::kARGB_8888_Config, width, height);
    	bmp.setPixels((void*)tmpARGB);

		SkImageEncoder::EncodeFile(filename, bmp, 
				SkImageEncoder::kPNG_Type, SkImageEncoder::kDefaultQuality);

		if (tmpARGB) free(tmpARGB);	
	}
   

    ulog("[ULT]done !\n\n");
    return 0;
}



/*
	src name format: rgb565_16x16
 */
static MHAL_BITBLT_FORMAT_ENUM getFormat(const char* srcName)
{
	if (srcName == NULL)
		// Todo: write error msg to logfile 
		return MHAL_FORMAT_ERROR;

    if (0 == strncmp(srcName, "rgb565", 6))
		return MHAL_FORMAT_RGB_565;
	
	if (0 == strncmp(srcName, "rgb888", 6))
		return MHAL_FORMAT_RGB_888;
	
	if (0 == strncmp(srcName, "argb8888", 7))
		return MHAL_FORMAT_ARGB_8888;
	
	if (0 == strncmp(srcName, "mtkyuv", 6))
		return MHAL_FORMAT_MTK_YUV;
	
	if (0 == strncmp(srcName, "yuv420", 6))
		return MHAL_FORMAT_YUV_420;
	
	if (0 == strncmp(srcName, "nv21", 4))
		return MHAL_FORMAT_YUV_420_SP;

	if (0 == strncmp(srcName, "yuyv422", 7))
		return MHAL_FORMAT_YUY2;
	
	return MHAL_FORMAT_ERROR;
}

/*
	src name format: rgb565_16x16
 */
static int getWidth(const char* srcName)
{
	if (srcName == NULL)
		// Todo: write error msg to logfile 
		return -1;
	
	const char* pos = srcName;
	char partName[10];
	int i = 0;
	int width;

	while (pos[i++] != '\0');
	while (i != 0 && pos[--i] != '_');
	if (*pos != '_' && i == 0) {
		// Todo: write error msg to logfile 
		return -2;
	}

	strcpy(partName, &pos[++i]);
	width = atoi(partName);
	//LOGE("Width: %d\n", width);
	
	return width;
}



/*
	src name format: rgb565_16x16
 */
static int getHeight(const char* srcName)
{
	if (srcName == NULL)
		// Todo: write error msg to logfile 
		return -1;
	
	const char* pos = srcName;
	char partName[10];
	int i = 0;
	int height;

	while (pos[i++] != '\0');
	while (i != 0 && pos[--i] != 'x');
	if (*pos != 'x' && i == 0) {
		// Todo: write error msg to logfile 
		return -2;
	}

	strcpy(partName, &pos[++i]);
	height = atoi(partName);
	//LOGE("height: %d\n", height);
	
	return height;
}


static int getSize(int width, int height, int fmt)
{
	switch (fmt) {
		case MHAL_FORMAT_RGB_565:
		case MHAL_FORMAT_YUY2:
			return (width * height * 2);

		case MHAL_FORMAT_RGB_888:
			return (width * height * 3);
		
		case MHAL_FORMAT_ARGB_8888:
			return (width * height * 4);

		case MHAL_FORMAT_YUV_420:
		case MHAL_FORMAT_YUV_420_SP:
		case MHAL_FORMAT_MTK_YUV:
			return (width * height * 3 / 2);
	}
	return 0;

}


static int _testMdpBitBlt(mHalBltParam_t* bltParam, unsigned char* buf, int ID)
{

    mHalBltParam_t stParam;
	memset(&stParam, 0, sizeof(mHalBltParam_t));
	memcpy(&stParam, bltParam, sizeof(mHalBltParam_t));


    unsigned int src_size;
    unsigned int dst_size;
    unsigned char *src_va;
    unsigned char *dst_va;
    int src_ID;
    int dst_ID;
	unsigned int src_size_align;
	unsigned int dst_size_align;
	unsigned char *src_va_align;
	unsigned char *dst_va_align;
	bool ret = false;

    unsigned int mode         = MHAL_MODE_BITBLT;

	//src_size = stParam.srcW * stParam.srcH * 2;
	src_size = getSize(stParam.srcWStride, stParam.srcHStride, stParam.srcFormat);

	src_size_align = src_size;	
	if (stParam.srcFormat == MHAL_FORMAT_RGB_888) {
		src_size_align += 12;	
	}

    //PMEM
    src_va = (unsigned char *) pmem_alloc(src_size_align , &src_ID);
    src_va = (unsigned char *) pmem_alloc_sync(src_size_align , &src_ID);
    
	if(src_va == NULL)
    {
        ulog("[ULT]Can not allocate memory\n");
		printf("..Failed\n");
    	goto End;
	}

	src_va_align = src_va;
	
	if (stParam.srcFormat == MHAL_FORMAT_RGB_888) {
		for (int i = 0; i < 12; i++) {
			src_va_align++;
			if ((unsigned int)src_va_align % 12 == 0) break;
		}
		ULT_ASSERT((unsigned int)src_va_align % 12 == 0);
	}

	memcpy((void*)src_va_align, buf, src_size);

// save source image
#if 0
	{
		FILE* fp;
		unsigned int index = 0;
		char filename[100];
		char resultname[100];
#if 1
        sprintf(filename, RESULT_PATH "%02d_(%d-%d_%d_%d_%d_%d_%d).bin",
			 ID, stParam.srcFormat, stParam.srcX, stParam.srcY, stParam.srcW, stParam.srcH, 
			 stParam.srcWStride, stParam.srcHStride);		
    	fp = fopen(filename, "w");
    	for(index = 0 ; index < src_size ; index++) {
        	fprintf(fp, "%c", src_va[index]);
    	}
    	fclose(fp);
#endif
		//ulog("[UT]write src to input file(%d)\n", src_size);
	
		sprintf(resultname, RESULT_PATH "src_%02d_(%d-%d_%d_%d_%d_%d_%d)",
			 ID, stParam.srcFormat, stParam.srcX, stParam.srcY, stParam.srcW, stParam.srcH, 
			 stParam.srcWStride, stParam.srcHStride);		
	
		saveFile(src_va, src_size, stParam.srcW, stParam.srcH, resultname, stParam.srcFormat, false);
	}

#endif


	dst_size = getSize(stParam.dstW, stParam.dstH, stParam.dstFormat) ;
    dst_va = (unsigned char *) pmem_alloc(dst_size , &dst_ID);
    dst_va = (unsigned char *) pmem_alloc_sync(dst_size , &dst_ID);

	if(dst_va == NULL)
    {
        ulog("[ULT]Can not allocate memory\n");
		printf("\tFailed\n");
    	goto End;
	}

	//ulog("[UT]Dest VA is : %d\n" , dst_va);
    memset(dst_va , 255 , dst_size);


    stParam.srcAddr = (MUINT32)src_va_align;//TODO
    stParam.dstAddr = (MUINT32)dst_va;
    ulog("[ULT]bitblt: src: (0x%x, %d) (%d, %d, %d, %d, %d, %d) dst: (0x%x, %d) (%d, %d, %d) rot: %d\n\n", 
             stParam.srcAddr, stParam.srcFormat, 
             stParam.srcX, stParam.srcY, stParam.srcW, stParam.srcH, stParam.srcWStride, stParam.srcHStride,
             stParam.dstAddr, stParam.dstFormat,
             stParam.dstW, stParam.dstH, stParam.pitch,
             stParam.orientation
     );
	//ulog("[UT]Start Bit blt!!\n");

#if 0
    MHalLockParam_t resource;
    resource.mode         = MHAL_MODE_BITBLT;
    resource.waitMilliSec = 0;
    resource.waitMode     = MHAL_MODE_ALL;
    if (MHAL_NO_ERROR != mHalIoCtrl(MHAL_IOCTL_LOCK_RESOURCE, &resource, sizeof(resource), NULL, 0, NULL)) {
        ulog("[ULT] IDP resource is locked.\n");
		printf("\t..Failed\n");
		goto End;
    }

    if (MHAL_NO_ERROR != mHalIoCtrl(MHAL_IOCTL_BITBLT, &stParam, sizeof(stParam), NULL, 0, NULL)) {
        LOGE("IDP_bitblt() can't do bitblt operation\n");
		printf("\t..Failed\n");
	} else {
		ret = true;
	}


	if(MHAL_NO_ERROR != mHalIoCtrl(MHAL_IOCTL_UNLOCK_RESOURCE, &mode, sizeof(mode), NULL, 0, NULL)) {
        LOGE("[OverlayMgr] can't unlock IDP resource\n");
    }
#else


    if(MHAL_NO_ERROR != mHalBitblt((void *)&stParam))
    {
		printf("\t..Failed\n");
    } else {
		ret = true;
    }


#endif

	if (ret == true) printf("\t.Successful\n");

End:

//Whatever the result is, save dst buffer to file. Because we will compare it with Golden sample.
//save output image
#if 1
	{
		FILE* fp;
		unsigned int index = 0;
		char filename[100];
		char resultname[100];
#if 0
        sprintf(filename, RESULT_PATH "%02d_(%d-%d_%d_%d_%d_%d_%d)_(%d-%d_%d_%d)_%d.bin",
			 ID, stParam.srcFormat, stParam.srcX, stParam.srcY, stParam.srcW, stParam.srcH, 
			 stParam.srcWStride, stParam.srcHStride,
             stParam.dstFormat, stParam.dstW, stParam.dstH, stParam.pitch,
             stParam.orientation);			

    	fp = fopen(filename, "w");
		if (dst_va != NULL) {
    		for(index = 0 ; index < dst_size ; index++) {
        		fprintf(fp, "%c", dst_va[index]);
    		}
		}
    	fclose(fp);
#endif

		sprintf(resultname, RESULT_PATH "bitblt_%02d_(%d-%d_%d_%d_%d_%d_%d)_(%d-%d_%d_%d)_%d",
			 ID, stParam.srcFormat, stParam.srcX, stParam.srcY, stParam.srcW, stParam.srcH, 
			 stParam.srcWStride, stParam.srcHStride,
             stParam.dstFormat, stParam.dstW, stParam.dstH, stParam.pitch,
             stParam.orientation);

	//	saveFile(dst_va, dst_size, stParam.dstW, stParam.dstH, resultname, stParam.dstFormat, false);
		saveFile(dst_va, dst_size, stParam.pitch, stParam.dstH, resultname, stParam.dstFormat, false);
	}
#endif

    pmem_free(src_va , src_size_align , src_ID);
    pmem_free(dst_va , dst_size , dst_ID);
    
	return 0;
}

static void _testBitBltTestCase(const char* src,  const char* dst, unsigned char buf[], stBitBltParam* param, int ID)
{
		ULT_ASSERT(src != NULL);
		ULT_ASSERT(dst != NULL);

		//ulog("%d, %d, %d\n", sizeof(buf), sizeof(buf[0]), sizeof(buf)/sizeof(buf[0]));

		const char* srcName = src;
		const char* dstName = dst;
		mHalBltParam_t stParam;
		memset(&stParam, 0, sizeof(stParam));


		printf("[BITBLT] ID:%02d (%s -> %s) ", ID, srcName, dstName);
		ulog("[BITBLT] ID:%d (%s -> %s)\n", ID, srcName, dstName);


		//stParam.srcX;
		//stParam.srcY;
		//stParam.srcAddr;
		//stParam.dstAddr;
		stParam.srcW = getWidth(srcName);
		stParam.srcWStride = getWidth(srcName);
		stParam.srcH = getHeight(srcName);
		stParam.srcHStride = getHeight(srcName);
		stParam.srcFormat = getFormat(srcName);

		stParam.dstW = getWidth(dstName);
		stParam.dstH = getHeight(dstName);
		stParam.dstFormat = getFormat(dstName);
		stParam.pitch = getWidth(dstName);

		stParam.orientation = MHAL_BITBLT_ROT_0;

		if (param != NULL) {
			stParam.orientation = param->rot;
			if (param->clipEn) {
				stParam.srcX = param->startX;
				stParam.srcY = param->startY;
				stParam.srcW = param->clipW;
				stParam.srcH = param->clipH;
			}

			if (param->pitchEn) {
				stParam.pitch = param->pitch;
			}
		}

		_testMdpBitBlt(&stParam, buf, ID);
}



bool TestMdpBitBlt()
{


	stBitBltParam bitBltParam;



#if defined TEST_BIT_BLT_DIFF_SRC_SIZE_16_16
	_testBitBltTestCase("rgb565_16x16",	"rgb565_256x256", 
						rgb565_16x16, NULL, 
						TEST_BIT_BLT_DIFF_SRC_SIZE_16_16);
#endif

#if defined TEST_BIT_BLT_DIFF_SRC_SIZE_399_199
	_testBitBltTestCase("rgb565_399x199", "rgb565_256x256", 
						rgb565_399x199, NULL,
						TEST_BIT_BLT_DIFF_SRC_SIZE_399_199);
#endif


#if defined TEST_BIT_BLT_DIFF_DST_SIZE_13_13
	_testBitBltTestCase("rgb565_80x80", "rgb565_13x13",
		   				rgb565_80x80, NULL,
						TEST_BIT_BLT_DIFF_DST_SIZE_13_13);
#endif

#if defined TEST_BIT_BLT_DIFF_DST_SIZE_720_480
	_testBitBltTestCase("rgb565_80x80", "rgb565_720x480",
		   				rgb565_80x80, NULL,
						TEST_BIT_BLT_DIFF_DST_SIZE_720_480);
#endif


#if defined TEST_BIT_BLT_DIFF_SRC_FORMAT_RGB565
	_testBitBltTestCase("rgb565_80x80", "rgb565_80x80", 
						rgb565_80x80, NULL,
						TEST_BIT_BLT_DIFF_SRC_FORMAT_RGB565);
#endif

#if defined TEST_BIT_BLT_DIFF_SRC_FORMAT_RGB888
	_testBitBltTestCase("rgb888_80x80", "rgb565_256x256",
		   				rgb888_80x80, NULL,
						TEST_BIT_BLT_DIFF_SRC_FORMAT_RGB888);
#endif

#if defined TEST_BIT_BLT_DIFF_SRC_FORMAT_ARGB8888
	_testBitBltTestCase("argb8888_80x80", "rgb565_256x256",
		   				argb8888_a80_80x80, NULL,
						TEST_BIT_BLT_DIFF_SRC_FORMAT_ARGB8888);
#endif

#if defined TEST_BIT_BLT_DIFF_SRC_FORMAT_MTKYUV
	_testBitBltTestCase("mtkyuv_352x288", "rgb565_256x256",
		   				yuv420_block_planar_352x288, NULL,
						TEST_BIT_BLT_DIFF_SRC_FORMAT_MTKYUV);
#endif

#if defined TEST_BIT_BLT_DIFF_SRC_FORMAT_YUV420
	_testBitBltTestCase("yuv420_240x416", "rgb565_256x256",
		   				yuv420_scanline_planar_240x416, NULL,
						TEST_BIT_BLT_DIFF_SRC_FORMAT_YUV420);
#endif

#if defined TEST_BIT_BLT_DIFF_SRC_FORMAT_NV21
	_testBitBltTestCase("nv21_176x220", "rgb565_256x256",
		   				nv21_176x220, NULL,
						TEST_BIT_BLT_DIFF_SRC_FORMAT_NV21);
#endif

#if defined TEST_BIT_BLT_DIFF_SRC_FORMAT_YUYV422
	_testBitBltTestCase("yuyv422_104x60", "rgb565_256x256",
		   				yuyv422_104x60, NULL,
						TEST_BIT_BLT_DIFF_SRC_FORMAT_YUYV422);
#endif



#if defined TEST_BIT_BLT_DIFF_DST_FORMAT_RGB565
	_testBitBltTestCase("mtkyuv_352x288", "rgb565_352x288",
		   				yuv420_block_planar_352x288, NULL,
						TEST_BIT_BLT_DIFF_DST_FORMAT_RGB565);
#endif

#if defined TEST_BIT_BLT_DIFF_DST_FORMAT_RGB888
	_testBitBltTestCase("mtkyuv_352x288", "rgb888_80x80",
		   				yuv420_block_planar_352x288, NULL,
						TEST_BIT_BLT_DIFF_DST_FORMAT_RGB888);
#endif

#if defined TEST_BIT_BLT_DIFF_DST_FORMAT_ARGB8888
	_testBitBltTestCase("mtkyuv_352x288", "argb8888_80x80",
		   				yuv420_block_planar_352x288, NULL,
						TEST_BIT_BLT_DIFF_DST_FORMAT_ARGB8888);
#endif

#if defined TEST_BIT_BLT_DIFF_DST_FORMAT_MTKYUV
	_testBitBltTestCase("rgb565_80x80", "mtkyuv_256x256",
		   				rgb565_80x80, NULL,
						TEST_BIT_BLT_DIFF_DST_FORMAT_MTKYUV);
#endif

#if defined TEST_BIT_BLT_DIFF_DST_FORMAT_YUV420
	_testBitBltTestCase("rgb565_80x80", "yuv420_256x256",
		   				rgb565_80x80, NULL,
						TEST_BIT_BLT_DIFF_DST_FORMAT_YUV420);
#endif

#if defined TEST_BIT_BLT_DIFF_DST_FORMAT_NV21
	_testBitBltTestCase("rgb565_80x80", "nv21_256x256",
		  				rgb565_80x80, NULL,
						TEST_BIT_BLT_DIFF_DST_FORMAT_NV21);
#endif

#if defined TEST_BIT_BLT_DIFF_DST_FORMAT_YUYV422
	_testBitBltTestCase("rgb565_80x80", "yuyv422_256x256",
		   				rgb565_80x80, NULL,
						TEST_BIT_BLT_DIFF_DST_FORMAT_YUYV422);
#endif

#if defined TEST_BIT_BLT_DIFF_ROTATE_90
	memset(&bitBltParam, 0, sizeof(bitBltParam));
	bitBltParam.rot = MHAL_BITBLT_ROT_90;
    
	_testBitBltTestCase("mtkyuv_352x288", "rgb565_256x256",
		   				yuv420_block_planar_352x288, &bitBltParam,
						TEST_BIT_BLT_DIFF_ROTATE_90);
#endif

#if defined TEST_BIT_BLT_DIFF_ROTATE_180
	memset(&bitBltParam, 0, sizeof(bitBltParam));
	bitBltParam.rot = MHAL_BITBLT_ROT_180;
    
	_testBitBltTestCase("mtkyuv_352x288", "rgb565_256x256",
		   				yuv420_block_planar_352x288, &bitBltParam,
						TEST_BIT_BLT_DIFF_ROTATE_180);
#endif

#if defined TEST_BIT_BLT_DIFF_ROTATE_270
	memset(&bitBltParam, 0, sizeof(bitBltParam));
	bitBltParam.rot = MHAL_BITBLT_ROT_270;
    
	_testBitBltTestCase("mtkyuv_352x288", "rgb565_256x256",
		   				yuv420_block_planar_352x288, &bitBltParam,
						TEST_BIT_BLT_DIFF_ROTATE_270);
#endif

#if defined TEST_BIT_BLT_DIFF_PITCH
	memset(&bitBltParam, 0, sizeof(bitBltParam));
	bitBltParam.rot = MHAL_BITBLT_ROT_0;
	bitBltParam.pitchEn = true;
	bitBltParam.pitch = 211;
    
	_testBitBltTestCase("mtkyuv_352x288", "rgb565_256x256",
		   				yuv420_block_planar_352x288, &bitBltParam,
						TEST_BIT_BLT_DIFF_PITCH);
#endif

#if defined TEST_BIT_BLT_DIFF_CLIP
	memset(&bitBltParam, 0, sizeof(bitBltParam));
	bitBltParam.rot = MHAL_BITBLT_ROT_0;
	//bitBltParam.rot = MHAL_BITBLT_ROT_0 | MHAL_BITBLT_FLIP_H;
	bitBltParam.clipEn = true;
	bitBltParam.startX = 64;
	bitBltParam.startY = 64;
	bitBltParam.clipW   = 128;
	bitBltParam.clipH   = 128;
	
    
	_testBitBltTestCase("rgb565_256x256", "rgb565_480x640",
		   				rgb565_256x256, &bitBltParam,
						TEST_BIT_BLT_DIFF_CLIP);
#endif


	return true;
}
