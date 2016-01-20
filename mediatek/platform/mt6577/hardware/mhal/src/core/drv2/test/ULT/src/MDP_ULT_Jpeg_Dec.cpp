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

#include "MDP_ULT_Jpeg_Dec.h"


#include <stdio.h>
#include <stdlib.h>
#include <cutils/log.h>
#include <cutils/pmem.h>
#include <cutils/memutil.h>
#include "jpeg_hal.h"
#include "MediaHal.h"


#include "MDP_ULT.h"

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

//#define JPG_DEC_ULT_USE_PMEM
#define JPEG_DEC_ULT_MHAL_IF


#define ULT_ASSERT(x) if(!(x)){ulog("__assert fail, file:%s, line:%d\n", __FILE__, __LINE__);}
#define Alignment_Up(x, y) ((x+y-1) & ~(y-1)) 


#if defined MDPULT_RES_RGB565_16X16
extern unsigned char rgb565_16x16[];
#endif

#if 1
static int saveFile(unsigned char* buf,int bufSize,
	   				int width, int height, 
					char* filename, int fmt, bool onlyRaw)
{
	if (buf == NULL) return 0;

	if ((fmt != JpgDecHal::kRGB_565_Format && 
		fmt != JpgDecHal::kARGB_8888_Format &&
	   	fmt != JpgDecHal::kRGB_888_Format) ||
		   	onlyRaw)
	{
		//ulog("can not save as png! unsupport format\n");
    	ulog("[UT]saving file as RAW in %s ... ", filename);
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
    ulog("[UT]saving file as PNG in %s ... ", filename);
	strcat(filename, ".png");
    
	//bmp.setConfig(SkBitmap::kARGB_8888_Config, sc.getWidth(), sc.getHeight());
	if (fmt == JpgDecHal::kRGB_565_Format) {
		bmp.setConfig(SkBitmap::kRGB_565_Config, width, height);
    	bmp.setPixels((void*)buf);


		if (!bmp.copyTo(&bmp_argb, SkBitmap::kARGB_8888_Config)) {
			ulog("[UT]save as png error!! @ tanslate to argb\n");
		}		
		
		SkImageEncoder::EncodeFile(filename, bmp_argb, 
				SkImageEncoder::kPNG_Type, SkImageEncoder::kDefaultQuality);
	} else if (fmt == JpgDecHal::kARGB_8888_Format){
		bmp.setConfig(SkBitmap::kARGB_8888_Config, width, height);
    	bmp.setPixels((void*)buf);

		SkImageEncoder::EncodeFile(filename, bmp, 
				SkImageEncoder::kPNG_Type, SkImageEncoder::kDefaultQuality);
	} else if (fmt == JpgDecHal::kRGB_888_Format) {
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
   
    ulog("done !\n\n");
    return 0;
}
#endif


static JpgDecHal::ColorFormat getDecFormat(const char* name)
{

	if (0 == strncmp(name, "rgb565", 6))
		return JpgDecHal::kRGB_565_Format;
	
	if (0 == strncmp(name, "rgb888", 6))
		return JpgDecHal::kRGB_888_Format;
	
	if (0 == strncmp(name, "argb8888", 7))
		return JpgDecHal::kARGB_8888_Format;

	ulog("[ULT]unsupport format\n");
	return JpgDecHal::kRGB_565_Format;
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


static unsigned int getFileSize(const char *filename) 
{      
	unsigned int size;     
  	FILE* fp = fopen( filename, "rb" ); 
    if (fp==NULL) {
    	ulog("[ULT]ERROR: Open file %s failed.\n", filename);  
        return 0;     
    }
   
   	fseek(fp, SEEK_SET, SEEK_END);  
    size=ftell(fp);   
    fclose(fp);   
    return size;  
} 


static unsigned int getDstSize(unsigned int width, unsigned int height, int fmt)
{
	unsigned int size;
	switch (fmt) {
		case JpgDecHal::kRGB_565_Format:
			size = width * height * 2;
			break;
		case JpgDecHal::kRGB_888_Format:
			size = width * height * 3;
			break;
		case JpgDecHal::kARGB_8888_Format:
			size = width * height * 4;
			break;
		default:
			size = 0;
			break;
	}
	return size;
}

static bool onDecodeHW(uint8_t* srcBuffer, uint32_t srcSize, MHAL_JPEG_DEC_START_IN* param)
{
    //int width, height;
    MHAL_JPEG_DEC_INFO_OUT outInfo;
	MHAL_JPEG_DEC_START_IN inParams;
    ULT_ASSERT(param != NULL);

	memcpy(&inParams, param, sizeof(MHAL_JPEG_DEC_START_IN));

    //inParams.dstFormat = JPEG_OUT_FORMAT_RGB565;

    if (MHAL_NO_ERROR != mHalIoCtrl(MHAL_IOCTL_JPEG_DEC_PARSER, (void *)srcBuffer, srcSize, 
                                    NULL, 0, NULL))
    {
		ulog("[ULT]parser file error\n");
        return false;
    }
    
    // get file dimension
    if (MHAL_NO_ERROR != mHalIoCtrl(MHAL_IOCTL_JPEG_DEC_GET_INFO, NULL, 0, 
                                   (void *)&outInfo, sizeof(outInfo), NULL))
    {
		ulog("[ULT]get info error\n");
        return false;
    }

    //inParams.dstFormat = JPEG_OUT_FORMAT_RGB565;
    //inParams.dstWidth = dstW;
    //inParams.dstHeight = dstH;
    //inParams.dstVirAddr = (UINT8*) dstBuffer;
    //inParams.dstPhysAddr = NULL;

    //inParams.doDithering = 0;

    // start decode
    if (MHAL_NO_ERROR != mHalIoCtrl(MHAL_IOCTL_JPEG_DEC_START, 
                                   (void *)&inParams, sizeof(inParams), 
                                   NULL, 0, NULL))
    {
        ulog("JPEG HW not support this image\n");
        return false;
    }

    return true;
}

static void _testJpegDecTestCase(const char* src,  const char* dst, void* buf, UINT32 param, int ID)
{
	ULT_ASSERT(src != NULL);
	ULT_ASSERT(dst != NULL);

	const char* srcName = src;
	const char* dstName = dst;
		
	unsigned int srcW, srcH;
	unsigned int dstW, dstH;
   	JpgDecHal::ColorFormat dstFormat;

	unsigned int src_size;
    unsigned int dst_size;
    unsigned char *src_va = NULL;
    unsigned char *dst_va = NULL;
    int src_ID;
    int dst_ID;
    unsigned long src_pa;
    unsigned long dst_pa;
	bool ret = false;


	printf("[JPGDEC] ID:%02d (%s -> %s) ", ID, srcName, dstName);
	ulog("[JPGDEC] ID:%02d (%s -> %s)\n", ID, srcName, dstName);


	srcW = getWidth(srcName);
	srcH = getHeight(srcName);

#if !defined (JPEG_DEC_ULT_MHAL_IF)
    JpgDecHal* jpgDecoder = new JpgDecHal();
#else
	MHAL_JPEG_DEC_START_IN decParam;
#endif


#if 1
	{
		FILE* fp;
		unsigned int index = 0;
		char filename[100];
    	sprintf(filename, SOURCE_PATH "jpeg/%s.jpg" ,  srcName);

		src_size = getFileSize(filename);
		if (src_size == 0) {
			ulog("[ULT]file size is 0\n");
			printf("\t..Failed\n");
			return;
		}

		ulog("%s %d\n", filename, src_size);		

	    // 1. ALLOCATE MEM FOR SRC
#if defined JPG_DEC_ULT_USE_PMEM
		src_va = (unsigned char *) pmem_alloc(src_size , &src_ID);
    	src_va = (unsigned char *) pmem_alloc_sync(src_size , &src_ID);
   	 	src_pa = (unsigned long)pmem_get_phys(src_ID);
#else
		src_va = (unsigned char *) malloc(src_size);
#endif
		
	
		ulog("[ULT] src va :0x%x \n", (UINT32)src_va );
		
    
		if (src_va == NULL) {
        	ulog("[UT]Can not allocate memory\n");
			printf("\t..Failed\n");
       		return; 
    	}

   		fp = fopen(filename, "rb");
   		for(index = 0 ; index < src_size ; index++) {
       		fscanf(fp, "%c", &src_va[index]);
   		}
   		fclose(fp);
		
	}
#endif

//save source image
#if 0
	{
		FILE* fp;
		unsigned int index = 0;
		char filename[100];
		char resultname[100];
        sprintf(filename, RESULT_PATH "jpg_%02d.jpg", ID);			

    	fp = fopen(filename, "w");
    	for(index = 0 ; index < src_size ; index++) {
        	fprintf(fp, "%c", src_va[index]);
    	}
    	fclose(fp);

	}
#endif

	dstW = getWidth(dstName);
	dstH = getHeight(dstName);
	dstFormat = getDecFormat(dstName);



    // 2. ALLOCATE PMEM FOR DST 
	dst_size = getDstSize(dstW, dstH, dstFormat);

#if defined JPG_DEC_ULT_USE_PMEM
    dst_va = (unsigned char *) pmem_alloc(dst_size , &dst_ID);
    dst_va = (unsigned char *) pmem_alloc_sync(dst_size , &dst_ID);
    dst_pa = (unsigned long)pmem_get_phys(dst_ID);
#else
    dst_va = (unsigned char *)malloc(dst_size); 
#endif


	ulog("[ULT] dst va :0x%x size:%d\n", (UINT32)dst_va, dst_size);

	if (dst_va == NULL) {
        ulog("[UT]Can not allocate memory\n");
		printf("\t..Failed\n");
       	return; 
    }

    memset(dst_va , 255 , dst_size);

#if !defined(JPEG_DEC_ULT_MHAL_IF)

	// 3. Decode JPEG File

    if(!jpgDecoder->lock())
    {
        ulog("can't lock resource\n");
		printf("\t..Failed\n");
    	goto End;
	}

    //jpgDecoder->setSrcAddr((unsigned char*)src_pa);
    jpgDecoder->setSrcAddr(src_va);
    jpgDecoder->setSrcSize(src_size);



    if (!jpgDecoder->parse()) {
		ulog("[ULT]Jpeg decoder parse error\n");
		printf("\t..Failed\n");
		goto End;
	}

	srcW = jpgDecoder->getJpgWidth();
    srcH = jpgDecoder->getJpgHeight();

    ulog("Jpge Dec src(%d %d), dst(%d, %d %d)\n", srcW, srcH, dstFormat, dstW, dstH);

    jpgDecoder->setOutWidth(dstW);
    jpgDecoder->setOutHeight(dstH);
    jpgDecoder->setOutFormat(dstFormat);
    

    jpgDecoder->setDstAddr(dst_va);
    jpgDecoder->setDstSize(dst_size);

    if (!jpgDecoder->start()) {
        ulog("decode failed~~~\n");
		printf("\t..Failed\n");
    } else {
		ret = true;
	}



    jpgDecoder->unlock();
    
#else

    decParam.dstFormat = (JPEG_OUT_FORMAT_ENUM)dstFormat;
    //decParam.dstFormat = JPEG_OUT_FORMAT_RGB565;
    
	decParam.dstWidth = dstW;
    decParam.dstHeight = dstH;
    decParam.dstVirAddr = (UINT8*) dst_va;
    decParam.dstPhysAddr = NULL;
    decParam.doDithering = 0;
	
	if (!onDecodeHW(src_va, src_size, &decParam)) {
		ulog("[ULT]decode failed!!\n");
		printf("\t..Failed\n");
	} else {
		ret = true;
	}
#endif


	if (ret == true) printf("\t.Successful\n");
End:

//Whatever the result is, save ds tbuffer to file. Because we will compare it with Golden sample.
//save output image
#if 1
	{
		FILE* fp;
		unsigned int index = 0;
		char filename[100];
		char resultname[100];
#if 0
        sprintf(filename, RESULT_PATH "jpg_%02d_(%d_%d)_(%d-%d_%d).bin",
			 ID,  srcW, srcH, dstFormat, dstW, dstH);			

    	fp = fopen(filename, "w");
		if (dst_va != NULL) {
    		for(index = 0 ; index < dst_size ; index++) {
        		fprintf(fp, "%c", dst_va[index]);
    		}
		}
    	fclose(fp);
#endif

		sprintf(resultname, RESULT_PATH "jpgdec_%02d_(%d_%d)_(%d-%d_%d)",
			 ID,  srcW, srcH, dstFormat, dstW, dstH);			
			 

		saveFile(dst_va, dst_size, dstW, dstH, resultname, dstFormat, false);
	}
#endif

	
#if !defined (JPEG_DEC_ULT_MHAL_IF)
    delete jpgDecoder;
#endif

#if defined JPG_DEC_ULT_USE_PMEM
    if (src_va) pmem_free(src_va , src_size , src_ID);
    if (dst_va) pmem_free(dst_va , dst_size , dst_ID);
#else
    if (src_va) free(src_va);
    if (dst_va) free(dst_va);
#endif

}	


bool TestMdpJpegDec()
{
#if defined TEST_JPG_DEC_DIFF_SRC_SIZE_32_32
	_testJpegDecTestCase("JPG_32x32-1", "rgb565_256x256", 
						NULL, NULL, 
						TEST_JPG_DEC_DIFF_SRC_SIZE_32_32);
#endif

#if defined TEST_JPG_DEC_DIFF_SRC_SIZE_399_199
	_testJpegDecTestCase("JPG_399x199-1", "argb8888_256x256", 
						NULL, NULL, 
						TEST_JPG_DEC_DIFF_SRC_SIZE_399_199);
#endif

#if defined TEST_JPG_DEC_DIFF_SRC_SIZE_1600_1200
	_testJpegDecTestCase("JPG_1600x1200", "rgb888_256x256", 
						NULL, NULL, 
						TEST_JPG_DEC_DIFF_SRC_SIZE_1600_1200);
#endif

#if defined TEST_JPG_DEC_DIFF_SRC_SIZE_2592_1944
	_testJpegDecTestCase("JPG_2592x1944", "rgb565_256x256", 
						NULL, NULL, 
						TEST_JPG_DEC_DIFF_SRC_SIZE_2592_1944);
#endif

#if defined TEST_JPG_DEC_DIFF_SRC_SIZE_4096_4096
	_testJpegDecTestCase("JPG_4096x4096", "rgb565_256x256", 
						NULL, NULL, 
						TEST_JPG_DEC_DIFF_SRC_SIZE_4096_4096);
#endif

#if defined TEST_JPG_DEC_DIFF_SRC_SIZE_4096_8184	
	_testJpegDecTestCase("JPG_4096x8184", "rgb565_256x256", 
						NULL, NULL, 
						TEST_JPG_DEC_DIFF_SRC_SIZE_4096_8184);
#endif


#if defined TEST_JPG_DEC_DIFF_DST_SIZE_16_16
	_testJpegDecTestCase("JPG_480x800", "rgb565_16x16", 
						NULL, NULL, 
						TEST_JPG_DEC_DIFF_DST_SIZE_16_16);
#endif

#if defined TEST_JPG_DEC_DIFF_DST_SIZE_399_199
	_testJpegDecTestCase("JPG_480x800", "rgb565_399x199", 
						NULL, NULL, 
						TEST_JPG_DEC_DIFF_DST_SIZE_399_199);
#endif

#if defined TEST_JPG_DEC_DIFF_DST_SIZE_928_928
	_testJpegDecTestCase("JPG_480x800", "rgb565_928x928", 
						NULL, NULL, 
						TEST_JPG_DEC_DIFF_DST_SIZE_928_928);
#endif

#if defined TEST_JPG_DEC_DIFF_DST_FORMAT_RGB565
	_testJpegDecTestCase("JPG_480x800", "rgb565_256x256", 
						NULL, NULL, 
						TEST_JPG_DEC_DIFF_DST_FORMAT_RGB565);
#endif

#if defined TEST_JPG_DEC_DIFF_DST_FORMAT_RGB888
	_testJpegDecTestCase("JPG_480x800", "rgb888_256x256", 
						NULL, NULL, 
						TEST_JPG_DEC_DIFF_DST_FORMAT_RGB888);
#endif

#if defined TEST_JPG_DEC_DIFF_DST_FORMAT_ARGB8888
	_testJpegDecTestCase("JPG_480x800", "argb8888_256x256", 
						NULL, NULL, 
						TEST_JPG_DEC_DIFF_DST_FORMAT_ARGB8888);
#endif

	return true;

}

