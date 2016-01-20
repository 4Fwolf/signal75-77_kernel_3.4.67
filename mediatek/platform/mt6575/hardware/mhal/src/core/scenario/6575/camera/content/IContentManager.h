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
#ifndef _I_CONTENT_MANAGER_H_
#define _I_CONTENT_MANAGER_H_


/*******************************************************************************
*
********************************************************************************/
namespace NSContent
{
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
* Content ID
********************************************************************************/
enum ECtnID
{
    eCtnID_EisGmv,  //  EIS Global Motion Vector.
    eCtnID_NUM
};


/*******************************************************************************
* Content: EIS Global Motion Vector.
********************************************************************************/
struct Ctn_EisGmv
{
    MUINT32     u4Version;  //I/O
    MUINT32     u4Count;    //I/O
    MINT32*     pi4GmvX;    //O
    MINT32*     pi4GmvY;    //O
};


/*******************************************************************************
*
********************************************************************************/
class IContentManager
{
protected:  ////    Constructor/Destructor.
    virtual ~IContentManager() {} 

public:     ////    Interfaces.
    //
    static IContentManager& getInstance();
    //
    virtual MBOOL   queryContent(ECtnID const eCtnID, MVOID* pBuf) = 0;

    //  FIXME: should not be here.
    virtual MBOOL   updateContent(ECtnID const eCtnID, MVOID const* pBuf) =0 ;

};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSContent
#endif  //  _I_CONTENT_MANAGER_H_

