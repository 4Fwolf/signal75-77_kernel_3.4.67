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
//#include "MediaHal.h"
//#include "jpeg_dec_hal.h"
//#include "MT6573MDPDrv.h"
#include "jpeg_hal.h"
//#include "scenario_imagetransform.h"


#include "MDP_ULT.h"
#include "MDP_ULT_Jpeg_Enc.h"

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

#define ULT_USE_PMEM


#define ULT_ASSERT(x) if(!(x)){ulog("__assert fail, file:%s, line:%d\n", __FILE__, __LINE__);}




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

#if defined MDPULT_RES_RGB565_256x256
extern unsigned char rgb565_256x256[];
#endif

#if defined MDPULT_RES_RGB565_405x405
extern unsigned char rgb565_405x405[];
#endif

#if defined MDPULT_RES_RGB888_80x80
extern unsigned char rgb888_80x80[];
#endif

#if defined MDPULT_RES_ARGB8888_80x80
extern unsigned char argb8888_a80_80x80[];
#endif



#define Alignment_Up(x, y) ((x+y-1) & ~(y-1)) 


/*
	src name format: rgb565_16x16
 */
static JpgEncHal::SrcFormat getSrcFormat(const char* srcName)
{
	if (srcName == NULL)
		// Todo: write error msg to logfile 
		return JpgEncHal::kSrcFormatCount;

    if (0 == strncmp(srcName, "rgb565", 6))
		return JpgEncHal::kRGB_565_Format;
	
	if (0 == strncmp(srcName, "rgb888", 6))
		return JpgEncHal::kRGB_888_Format;
	
	if (0 == strncmp(srcName, "argb8888", 7))
		return JpgEncHal::kARGB_8888_Format;
	
	return JpgEncHal::kSrcFormatCount;
}

static JpgEncHal::EncFormat getEncFormat(const char* encName)
{
	if (encName == NULL)
		// Todo: write error msg to logfile 
		return JpgEncHal::kEncFormatCount;

	if (0 == strncmp(encName, "yuv444", 6))
		return JpgEncHal::kYUV_444_Format;
	
	if (0 == strncmp(encName, "yuv422", 6))
		return JpgEncHal::kYUV_422_Format;
	
	if (0 == strncmp(encName, "yuv411", 6))
		return JpgEncHal::kYUV_411_Format;
	
	if (0 == strncmp(encName, "yuv420", 6))
		return JpgEncHal::kYUV_420_Format;

	if (0 == strncmp(encName, "yuv400", 6))
		return JpgEncHal::kYUV_400_Format;
	
	return JpgEncHal::kEncFormatCount;
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



static unsigned int getSrcSize(unsigned int width, unsigned int height, int fmt)
{
	unsigned int size;
	switch (fmt) {
		case JpgEncHal::kRGB_565_Format:
			size = width * height * 2;
			break;
		case JpgEncHal::kRGB_888_Format:
			size = width * height * 3;
			break;
		case JpgEncHal::kARGB_8888_Format:
			size = width * height * 4;
			break;
		default:
			size = 0;
			break;
	}

	return size;
}

static unsigned int getDstSize(unsigned int width, unsigned int height, int fmt)
{
	unsigned int size;
	switch (fmt) {
		case JpgEncHal::kYUV_444_Format:
			size = width * height * 3;
			break;
		case JpgEncHal::kYUV_422_Format:
			size = width * height * 2;
			break;
		case JpgEncHal::kYUV_411_Format:
			size = width * height * 3 / 2;
			break;
		case JpgEncHal::kYUV_420_Format:
			size = width * height * 3 / 2;
			break;
		case JpgEncHal::kYUV_400_Format:
			size = width * height ;
			break;
		default:
			size = 0;
			break;
	}

	return size;
}

static unsigned int getSrcBPP(int fmt)
{
	unsigned int bpp;
	switch (fmt) {
		case JpgEncHal::kRGB_565_Format:
			bpp = 2;
			break;
		case JpgEncHal::kRGB_888_Format:
			bpp = 3;
			break;
		case JpgEncHal::kARGB_8888_Format:
			bpp = 4;
			break;
		default:
			LOGE("[ULT] un support format");
			bpp = 0;
			break;
	}

	return bpp;
}


static void _testJpegEncTestCase(const char* src,  const char* dst, unsigned char buf[], UINT32 param, int ID)
{
	ULT_ASSERT(src != NULL);
	ULT_ASSERT(dst != NULL);

	const char* srcName = src;
	const char* dstName = dst;
		
	unsigned int srcW, srcH;
	unsigned int dstW, dstH;
	JpgEncHal::SrcFormat srcFormat;
   	JpgEncHal::EncFormat dstFormat;
	unsigned int enc_size=0;


	unsigned int src_size;
    unsigned int dst_size;
    unsigned int src_real_size;
    unsigned int dst_real_size;
    unsigned char *src_va = NULL;
    unsigned char *dst_va = NULL;
    unsigned char *src_va_real = NULL;
    unsigned char *dst_va_real = NULL;
    int src_ID;
    int dst_ID;
    unsigned long src_pa;
    unsigned long dst_pa;
    unsigned long src_pa_real;
    unsigned long dst_pa_real;

	unsigned int src_align = 48;
	unsigned int dst_align = 48;
	unsigned int quality;
	unsigned int bpp;
	bool ret = false;

	quality = param;
	ULT_ASSERT(quality <= 100 && quality >= 0);

	printf("[JPGENC] ID:%02d (%s -> %s) ", ID, srcName, dstName);
	ulog("[JPGENC] ID:%02d (%s -> %s)\n", ID, srcName, dstName);


    JpgEncHal* jpgEncoder = NULL;


	srcW = getWidth(srcName);
	srcH = getHeight(srcName);
	srcFormat = getSrcFormat(srcName);

	dstW = getWidth(dstName);
	dstH = getHeight(dstName);
	dstFormat = getEncFormat(dstName);


	bpp = getSrcBPP(srcFormat);

	// RGB888, alignment 3X
	if (bpp == 3) {
		src_align = 48;
	} else {
		src_align = bpp;
	}

	src_real_size = getSrcSize(srcW, srcH, srcFormat) ;
    src_size = src_real_size + src_align;

    // 1. ALLOCATE MEM FOR SRC
#if defined ULT_USE_PMEM	
	src_va = (unsigned char *) pmem_alloc(src_size , &src_ID);
    src_va = (unsigned char *) pmem_alloc_sync(src_size , &src_ID);
    src_pa = (unsigned long)pmem_get_phys(src_ID);
#else
	src_va = (unsigned char*)malloc(src_size);
#endif
	
	ulog("[ULT] src va:0x%x \n", src_va );

	if (bpp == 3) {
		src_va_real = src_va;

		for (int i = 0; i < src_align ; i++) {
			src_va_real++;
			if ((unsigned int)src_va_real % src_align == 0) break;
		}
	} else {
		src_va_real = (unsigned char*)Alignment_Up((UINT32)src_va, bpp);
	}


	ULT_ASSERT((unsigned int)src_va_real % src_align == 0);

	ulog("[ULT] src va align:0x%x \n", src_va_real );
    
	if(src_va_real == NULL)
    {
        ulog("[UT]Can not allocate memory\n");
		printf("\t..Failed\n");
       	goto End; 
    }

	memcpy((void*)src_va_real, buf, src_real_size);
		
	
	
    // 2. ALLOCATE PMEM FOR DST 
	dst_real_size = getDstSize(dstW, dstH, dstFormat);
	dst_size = dst_real_size;

#if defined ULT_USE_PMEM	
    dst_va = (unsigned char *) pmem_alloc(dst_size , &dst_ID);
    dst_va = (unsigned char *) pmem_alloc_sync(dst_size , &dst_ID);
    dst_pa = (unsigned long)pmem_get_phys(dst_ID);
#else
	dst_va = (unsigned char*)malloc(dst_size);
#endif


	dst_va_real = dst_va;
	dst_pa_real = dst_pa;


	ulog("[ULT] dst va align:0x%x\n", dst_va_real );

	if(dst_va_real == NULL)
    {
        ulog("[UT]Can not allocate memory\n");
		printf("\t..Failed\n");
       	goto End; 
    }

	//ulog("[UT]Dest VA is : %d\n" , dst_va);
    memset(dst_va_real , 255 , dst_real_size);

    ulog("[UT]JPEG Enc: src: (0x%x, %d, %d, %d) dst: (0x%x, %d, %d, %d) \n\n", 
             src_va_real, srcFormat, srcW, srcH,
             dst_va_real, dstFormat, dstW, dstH 
     );

    jpgEncoder = new JpgEncHal();



    jpgEncoder->setSrcWidth(srcW);
    jpgEncoder->setSrcHeight(srcH);
    jpgEncoder->setDstWidth(dstW);
    jpgEncoder->setDstHeight(dstH);
    jpgEncoder->setEncFormat(dstFormat);
    jpgEncoder->setSrcFormat(srcFormat);
    jpgEncoder->setQuality(quality);

    jpgEncoder->setSrcAddr((void *)src_va_real);
    jpgEncoder->setDstAddr((void *)dst_va_real);
    jpgEncoder->setDstSize(dst_size);

    if(!jpgEncoder->lock())
    {
        ulog("can't lock resource");
		printf("\t..Failed\n");
        goto End;
    }

    if(!jpgEncoder->start(&enc_size))
    {
        ulog("[ULT] jpeg encode failed~~~\n");
		printf("\t..Failed\n");
    } else {
		ret = true;
	}
    
    jpgEncoder->unlock();

	ULT_ASSERT(enc_size <= dst_real_size);


	if (ret == true) printf("\t.Successful\n");
End:
#if 1
	{
		FILE* fp;
		unsigned int index = 0;
		char filename[100];
    	sprintf(filename, RESULT_PATH "jpgenc_%02d_(%d-%d_%d)_(%d-%d_%d).jpg",
			 ID, srcFormat, srcW, srcH, 
         	dstFormat, dstW, dstH);			

   		fp = fopen(filename, "w");
		if (dst_va_real != NULL) {
   			for(index = 0 ; index < enc_size ; index++) {
       			fprintf(fp, "%c", dst_va_real[index]);
   			}
		}
   		fclose(fp);
	}
#endif


	
    if (jpgEncoder) delete jpgEncoder;
#if defined ULT_USE_PMEM
    if (src_va) pmem_free(src_va , src_size , src_ID);
    if (dst_va) pmem_free(dst_va , dst_size , dst_ID);
#else
    if (src_va) free(src_va);
    if (dst_va) free(dst_va);
#endif

}	


bool TestMdpJpegEnc()
{

#if defined TEST_JPG_ENC_DIFF_SRC_SIZE_16_16
	_testJpegEncTestCase("rgb565_16x16", "yuv422_256x256", 
						rgb565_16x16, 95, 
						TEST_JPG_ENC_DIFF_SRC_SIZE_16_16);
#endif

#if defined TEST_JPG_ENC_DIFF_SRC_SIZE_399_199 
	_testJpegEncTestCase("rgb565_399x199", "yuv422_1024x768", 
						rgb565_399x199, 95, 
						TEST_JPG_ENC_DIFF_SRC_SIZE_399_199);
#endif

#if defined TEST_JPG_ENC_DIFF_SRC_SIZE_2048_2048

#endif

#if defined TEST_JPG_ENC_DIFF_ENC_SIZE_16_16 			
	_testJpegEncTestCase("rgb565_256x256", "yuv422_32x32", 
						rgb565_256x256, 95, 
						TEST_JPG_ENC_DIFF_ENC_SIZE_16_16);
#endif

#if defined TEST_JPG_ENC_DIFF_ENC_SIZE_399_199
	_testJpegEncTestCase("rgb565_256x256", "yuv422_399x199", 
						rgb565_256x256, 95, 
						TEST_JPG_ENC_DIFF_ENC_SIZE_399_199);
#endif

#if defined TEST_JPG_ENC_DIFF_ENC_SIZE_256_8191
	_testJpegEncTestCase("rgb565_256x256", "yuv411_256x8191", 
						rgb565_256x256, 0, 
						TEST_JPG_ENC_DIFF_ENC_SIZE_256_8191);
#endif

#if defined TEST_JPG_ENC_DIFF_SRC_FORMAT_RGB565
	_testJpegEncTestCase("rgb565_80x80", "yuv422_80x80", 
						rgb565_80x80, 95, 
						TEST_JPG_ENC_DIFF_SRC_FORMAT_RGB565);
#endif

#if defined TEST_JPG_ENC_DIFF_SRC_FORMAT_RGB888
	_testJpegEncTestCase("rgb888_80x80", "yuv422_80x80", 
						rgb888_80x80, 95, 
						TEST_JPG_ENC_DIFF_SRC_FORMAT_RGB888);
#endif	

#if defined TEST_JPG_ENC_DIFF_SRC_FORMAT_ARGB8888
	_testJpegEncTestCase("argb888_80x80", "yuv422_80x80", 
						argb8888_a80_80x80, 95, 
						TEST_JPG_ENC_DIFF_SRC_FORMAT_ARGB8888);
#endif	


#if defined TEST_JPG_ENC_DIFF_ENC_FORMAT_YUV422
	_testJpegEncTestCase("rgb565_80x80", "yuv422_80x80", 
						rgb565_80x80, 95, 
						TEST_JPG_ENC_DIFF_ENC_FORMAT_YUV422);
#endif	

#if defined TEST_JPG_ENC_DIFF_ENC_FORMAT_YUV411
	_testJpegEncTestCase("rgb565_80x80", "yuv411_80x80", 
						rgb565_80x80, 95, 
						TEST_JPG_ENC_DIFF_ENC_FORMAT_YUV411);
#endif	

#if defined TEST_JPG_ENC_DIFF_ENC_FORMAT_YUV420
	_testJpegEncTestCase("rgb565_80x80", "yuv420_80x80", 
						rgb565_80x80, 95, 
						TEST_JPG_ENC_DIFF_ENC_FORMAT_YUV420);
#endif	

#if defined TEST_JPG_ENC_DIFF_ENC_FORMAT_YUV400
	_testJpegEncTestCase("rgb565_80x80", "yuv400_128x128", 
						rgb565_80x80, 95, 
						TEST_JPG_ENC_DIFF_ENC_FORMAT_YUV400);
#endif	
	return true;

}
