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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// BMQFile.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  BMQFile.cpp
//! \brief

#define LOG_TAG "BMQFile"
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "../inc/AcdkTypes.h"
#include "../inc/BMQFile.h"
#include "../inc/AcdkLog.h"

/////////////////////////////////////////////////////////////////////////////
// Compiler optimization switches
/////////////////////////////////////////////////////////////////////////////
// O0 - Minimum (best debug view)
// O1 - Most (good debug view, good code)
// O2 - All (poor debug view, best code)
// Otime - Optimize for time
// Ospace - Optimize for space
//
/////////////////////////////////////////////////////////////////////////////

const BMQFileHeader DefaultHeader = 
{
    0x4d42,                                                   //'BM'
//    0x5242,                                                 //'BR'
    sizeof(BMQFileHeader)+(BMQ_IMAGE_XSIZE*BMQ_IMAGE_YSIZE*BMQ_BYTES_PER_PIXEL),  // File Size
    0, 0,                                                   // (not used)
    sizeof(BMQFileHeader),                                  // Header Size = 54
    40,                                                     // infor header size
    BMQ_IMAGE_XSIZE, BMQ_IMAGE_YSIZE,                       // X,Y sizes
    1,                                                      // Planes
    BMQ_BITS_PER_PIXEL,                                     // bits per pixel
    0,                                                      // a_1
    (BMQ_IMAGE_XSIZE*BMQ_IMAGE_YSIZE*BMQ_BYTES_PER_PIXEL),  // a_2 = data length
    0x80000000,                                             // a_3 = 1, origin=upper left corner
    0,                                                      // a_4 (not used)
    0,                                                      // a_5
    1024                                                     // a_6 = 12 bits depth
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// BMQFile Class - Base class of the BMQFile object
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//      BMQFile() - BMQFile object constructor/destructor.
//
////////////////////////////////////////////////////////////////////
BMQFile::BMQFile(UCHAR *pName)
{
    // init file header with default setup
    memcpy((CHAR*)&m_stHeader,(CHAR*)&DefaultHeader,sizeof(BMQFileHeader));

    m_puFileName = pName; 
}

BMQFile::~BMQFile()
{
    m_puFileName = NULL; 
}

BOOL BMQFile::bOpenFile()
{

    m_pFile = fopen(m_puFileName, "wb"); 

    if (m_pFile == NULL)
    {
        ACDK_LOGE("Open the file:%s fail \n", m_puFileName); 
        return FALSE; 
    }
    return TRUE; 
}


BOOL BMQFile::bCloseFile()
{
    if (m_pFile != NULL)
    {
        ACDK_LOGD("bCloseFile () - close file \n"); 
        fclose(m_pFile); 
        return TRUE; 
    }
    return FALSE; 
}
/////////////////////////////////////////////////////////////////////////////////////////
//
// buildFile():  build a Wave file.
//
//  Note:   all proper header entries need to set before calling this function. otherwise, the
//          default setup will be used.
//
//  Return: TRUE = success, FALSE = fail.
//////////////////////////////////////////////////////////////////////////////////////////
BOOL BMQFile::buildFile (   UCHAR* pDataBuf,            // pointer to raw data
                            UINT32 ulXSize,              // X length of image
                            UINT32 ulYSize)              // Y length of Image
{
    INT32 ulSize;

    if (!bOpenFile())

    {
        ACDK_LOGE("buildFiel() - Open the file:%s fail \n", m_puFileName); 
        return FALSE; 
    }

    setXYLength(ulXSize, ulYSize); 

    ulSize = getDataLength(); 
    
    setXYLength(ulXSize,ulYSize);
    ulSize = getDataLength();

    ACDK_LOGD("buildFile() - Write Header \n");
    if(!writeHeader())
    {
        ACDK_LOGE("buildFiiel() - fail to write heade\nr" );
        bCloseFile();
        return FALSE;
    }

    ACDK_LOGD("buildFile() - Write Data \n"); 
    if(writeData(pDataBuf,ulSize)!=ulSize)
    {
        ACDK_LOGE("buildFile() -  fail to write data\n" );
        bCloseFile(); 
        return FALSE;
    }

    bCloseFile(); 
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// writeHeader():  build and write BMQ file header.
//
// Return: TRUE = success, FALSE = fail.
//
//////////////////////////////////////////////////////////////////////////////////////////
BOOL BMQFile::writeHeader()
{   
    INT32 size = sizeof(m_stHeader);

    fseek(m_pFile, 0, SEEK_SET); 
        
    return ((INT32)fwrite((UCHAR*)&m_stHeader, 1, size, m_pFile)==size)?TRUE:FALSE;

}

//////////////////////////////////////////////////////////////////////////////////////////
//
// writeData():  write BMQ file data.
//
// Note: this function can only be called when the file handler points to data area.
// Return: number of bytes written
//
//////////////////////////////////////////////////////////////////////////////////////////
INT32 BMQFile::writeData(UCHAR* pData, UINT32 ulSize)
{        
    return ((INT32)fwrite((UCHAR*)pData, 1, ulSize, m_pFile));
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// decodeFile():  decode BMQ file and return data size.
//
//  Return: TRUE = success, FALSE = fail.
//////////////////////////////////////////////////////////////////////////////////////////
BOOL BMQFile::decodeFile (  UINT32&  ulDataSize)             // size of wave data
{
    ulDataSize = getDataLength();
    return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////////////
//
// readHeader():  read and validate BMQ file header.
//
// Return: TRUE = success, FALSE = fail.
//
//////////////////////////////////////////////////////////////////////////////////////////
BOOL BMQFile::readHeader()
{
    return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////////////
//
// readData():  read BMQ file data.
//
// Note: this function can only be called when the file handler points to data area.
// Return: number of bytes read
//
//////////////////////////////////////////////////////////////////////////////////////////
INT32 BMQFile::readData(UCHAR* pData,UINT32 ulSize)
{
    return 0; 
}


//////////////////////////////////////////////////////////////////////////////////////////
//
// setXYLength():  set X,Y dimension for BMQ image
//
// Return: none
//
//////////////////////////////////////////////////////////////////////////////////////////
VOID BMQFile::setXYLength(UINT32 ulXLength, UINT32 ulYLength)
{
    m_stHeader.ulXLength = ulXLength;
    m_stHeader.ulYLength = ulYLength;
    m_stHeader.a_2 = ulXLength*ulYLength*BMQ_BYTES_PER_PIXEL;
    m_stHeader.ulFileSize = m_stHeader.a_2+54;
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// getDataLength():  get BMQ data length.
//
// Return: wav data length in bytes
//
//////////////////////////////////////////////////////////////////////////////////////////
VOID BMQFile::getXYLength(UINT32& ulXLength, UINT32& ulYLength)
{
    ulXLength = m_stHeader.ulXLength;
    ulYLength = m_stHeader.ulYLength;
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// getDataLength():  get BMQ data length.
//
// Return: wav data length in bytes
//
//////////////////////////////////////////////////////////////////////////////////////////
UINT32 BMQFile::getDataLength()
{
    return (m_stHeader.ulXLength*m_stHeader.ulYLength*BMQ_BYTES_PER_PIXEL);
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// setBitsPerPixel():  set bits per pixel for BMQ image
//
// Return: none
//
//////////////////////////////////////////////////////////////////////////////////////////
VOID BMQFile::setBitsPerPixel(UINT16 uBits)
{
    m_stHeader.uBitPerPixel = uBits;
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// getBitsPerPixel():  get bits per pixel for BMQ image
//
// Return: none
//
//////////////////////////////////////////////////////////////////////////////////////////
UINT16 BMQFile::getBitsPerPixel()
{
    return m_stHeader.uBitPerPixel;
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// setBitsDepth():  set effective bits depth, 10bit=1024, 12 bits= 4096
//
// Return: none
//
//////////////////////////////////////////////////////////////////////////////////////////
VOID BMQFile::setBitsDepth(UINT32 ulDepth)
{
    m_stHeader.a_6 = ((UINT32)1 << ulDepth);
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// getBitsDepth():  get bits per pixel for BMQ image
//
// Return: none
//
//////////////////////////////////////////////////////////////////////////////////////////
UINT32 BMQFile::getBitsDepth()
{
    UINT32 u4BitDepth = 0; 
    UINT8 i = 0; 
    for (i = 0; i < 32; i++)
    {
        if ((m_stHeader.a_6 >> i ) & (UINT32)0x1)
        {
            break; 
        }        
    }
    u4BitDepth = i; 
    return u4BitDepth;
}
