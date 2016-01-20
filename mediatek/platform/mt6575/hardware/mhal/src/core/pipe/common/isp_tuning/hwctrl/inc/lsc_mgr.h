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
#ifndef _LSC_MGR_H_
#define _LSC_MGR_H_

//#if defined(LOG_TAG)
//#undef (LOG_TAG)
//#endif

//#define LOG_TAG "NSIspTuning::Lsc_mgr"

#define LOG_MSG(fmt, arg...)    LOGD("-%s()" fmt, __FUNCTION__, ##arg) //XLOGD("[%s]"fmt, __FUNCTION__, ##arg) //#define STNR_LOG(fmt, arg...)	 LOGD(STNR_HAL_TAG fmt, ##arg)


#if defined(MTK_M4U_SUPPORT)
#define LSC_TBL_M4U 1
#include "m4u_lib.h"		// For MT6575 M4UDrv.
#include <linux/cache.h>
#else
#define LSC_TBL_M4U 0
#endif 

#define RAW_LSC_TBL_NUM 2
namespace NSIspTuning
{
typedef struct {
    MUINT32 u4VirAddr;			///< Virtual address that passed from Middle-ware.
    MUINT32 u4UseM4u;			///< A flag indicates using M4U or not.
    MUINT32 u4CamPortMva;	      ///< MVA that using GMC2 Defect port.
    MUINT32 u4BufSize;		     ///< Size of this buffer.
} RawLscM4uInfo_t; 

/*******************************************************************************
* PCA Manager
*******************************************************************************/
class LscMgr
{
private:
    enum
    {
        LUT_PREVIEW = 0, 
        LUT_CAPTURE, 
        LUT_BINNING, 
        NUM_OF_LUTS_Mode
    };

    enum
    {
        LUT_LOW = 0, 
        LUT_MIDDLE, 
        LUT_HIGH, 
        NUM_OF_LUTS
    };

    enum
    {
        LUT_SIZE_Preview_M4u = MAX_SHADING_Preview_SIZE*3*4, // 3 table load at the same time (bytes)
        LUT_SIZE_Capture_M4u = MAX_SHADING_Capture_SIZE*3*4, //MAX_SHADING_Capture_SIZE*4,       // 1 table (bytes)
    };

    enum
    {
        LUT_SIZE_Preview = MAX_SHADING_Preview_SIZE*3*4, // 3 table load at the same time (bytes)
        LUT_SIZE_Capture = MAX_SHADING_Preview_SIZE*3*4, //MAX_SHADING_Capture_SIZE*4,       // 1 table (bytes)
    };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Change Count.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////
    template <typename T>
    inline
    MVOID
    setIfChange(T& dst, T const src)
    {
        if  ( src != dst )
        {
            dst = src;
            if (m_u4ChangeCount < 65536) //just avoid reset to zero
              m_u4ChangeCount++;
            else
            	m_u4ChangeCount = 1;
        }
    }

public:  ////
    inline MUINT32  getSettingChangeCount() const { return m_u4ChangeCount; }
    inline MVOID    resetSettingChangeCount() { m_u4ChangeCount = 0; }

private:    ////    Data Members.
    //  Nonzero indicates any member has changed; otherwise zero.
    MUINT32                 m_u4ChangeCount;


public:  ////    
//enable M4U or not
    inline
    MVOID
    LscM4uEn(MBOOL const mbEn)
    {
            MY_LOG(" LscM4uEn = %d\n",mbEn);    
        m_bM4uEn = mbEn;
    }

    inline
    MBOOL
    LscGetM4u() const
    {
            MY_LOG(" LscGetM4u %d\n",m_bM4uEn);    
        return m_bM4uEn;
    }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Index
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    inline
    MUINT32
    getIdx() const
    {
        return m_u4Idx;
    }

    inline
    MBOOL
    setIdx(MUINT32 const u4Idx)
    {
        if  ( NUM_OF_LUTS <= u4Idx )
        {
            MY_LOG("!!! WRONG Shading idx= %d\n",u4Idx);            
            return  MFALSE;
        }
        setIfChange(m_u4Idx, u4Idx);
        return  MTRUE;
    }

private:    ////    Common.

    //  LSC index
    MUINT32         m_u4Idx;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Mode
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    inline
    MUINT32
    getMode() const
    {
        return m_u4Mode;
    }

    inline
    MBOOL
    setMode(MUINT32 const u4Mode)
    {
        if  ( NUM_OF_LUTS_Mode <= u4Mode )
            return  MFALSE;
        setIfChange(m_u4Mode, u4Mode);
        return  MTRUE;
    }

private:    ////    Common.

    //  LSC index
    MUINT32         m_u4Mode;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Address
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations.

    inline
    MVOID*
    getPhyTBA() const
    {
        return  m_pvPhyTBA;
    }

    inline
    MVOID
    savePhyTBA(MVOID*const pTBA)
    {
        setIfChange(m_pvPhyTBA, pTBA);        
    }

    inline
    MVOID*
    getVirTBA() const
    {
        return  m_pvVirTBA;
    }

    inline
    MVOID
    saveVirTBA(MVOID*const pTBA)
    {
        setIfChange(m_pvVirTBA, pTBA);
    }

private:    ////    Address.
    //  Physical Base address of LSC table.
    MVOID*                  m_pvPhyTBA;
    //  Virtual Base address of LSC table.
    MVOID*                  m_pvVirTBA;
//+++++++++++
/******************************************************************************/
    inline
    MBOOL
    ZsdConfigUpdate()
    {
        MBOOL fgRet = MFALSE;  
        if(!LscGetM4u())
        {    
            MUINT32 mu32PreBlkHeight = 
                m_rIspLscCfg[LUT_PREVIEW].shading_ctrl2.bits.SDBLK_HEIGHT;// : 12; //[11:0]
            MUINT32 mu32PreBlkYnum = 
                m_rIspLscCfg[LUT_PREVIEW].shading_ctrl2.bits.SDBLK_YNUM;       //: 4; //[15:12]
            MUINT32 mu32PreBlkWidth = 
                m_rIspLscCfg[LUT_PREVIEW].shading_ctrl2.bits.SDBLK_WIDTH;      //: 12; //[27:16]
            MUINT32 mu32PreBlkXnum = 
                m_rIspLscCfg[LUT_PREVIEW].shading_ctrl2.bits.SDBLK_XNUM;       //: 4; //[31:28]
            
            MUINT32 mu32PreLHeight = 
                m_rIspLscCfg[LUT_PREVIEW].shading_last_blk.bits.SDBLK_LHEIGHT;    // : 12; //[11:0]
            MUINT32 mu32PreLWidth  = 
                m_rIspLscCfg[LUT_PREVIEW].shading_last_blk.bits.SDBLK_LWIDTH;    //     : 12; //[27:16]
    
            MUINT32 mu32CapBlkHeight = 
                m_rIspLscCfg[LUT_CAPTURE].shading_ctrl2.bits.SDBLK_HEIGHT;// : 12; //[11:0]
            MUINT32 mu32CapBlkYnum = 
                m_rIspLscCfg[LUT_CAPTURE].shading_ctrl2.bits.SDBLK_YNUM;       //: 4; //[15:12]
            MUINT32 mu32CapBlkWidth = 
                m_rIspLscCfg[LUT_CAPTURE].shading_ctrl2.bits.SDBLK_WIDTH;      //: 12; //[27:16]
            MUINT32 mu32CapBlkXnum = 
                m_rIspLscCfg[LUT_CAPTURE].shading_ctrl2.bits.SDBLK_XNUM;       //: 4; //[31:28]
    
            MUINT32 mu32CapLHeight = 
                m_rIspLscCfg[LUT_CAPTURE].shading_last_blk.bits.SDBLK_LHEIGHT;    // : 12; //[11:0]
            MUINT32 mu32CapLWidth  = 
                m_rIspLscCfg[LUT_CAPTURE].shading_last_blk.bits.SDBLK_LWIDTH;    //     : 12; //[27:16]
    
            MUINT32 mu32CapResXsize =  mu32CapBlkWidth*mu32CapBlkXnum+mu32CapLWidth;
            MUINT32 mu32CapResYsize =  mu32CapBlkHeight*mu32CapBlkYnum+mu32CapLHeight;        
    
            MUINT32 mu32PreResXsize =  mu32PreBlkWidth*mu32PreBlkXnum+mu32PreLWidth;
            MUINT32 mu32PreResYsize =  mu32PreBlkHeight*mu32PreBlkYnum+mu32PreLHeight;        
    
            m_rIspLscCfg[LUT_CAPTURE].shading_ctrl2.bits.SDBLK_YNUM = 
                m_rIspLscCfg[LUT_PREVIEW].shading_ctrl2.bits.SDBLK_YNUM;       //: 4; //[15:12]
                
            m_rIspLscCfg[LUT_CAPTURE].shading_ctrl2.bits.SDBLK_XNUM = 
                m_rIspLscCfg[LUT_PREVIEW].shading_ctrl2.bits.SDBLK_XNUM;       //: 4; //[15:12]        
    
            m_rIspLscCfg[LUT_CAPTURE].shading_ctrl2.bits.SDBLK_WIDTH = 
                (mu32CapResXsize*m_rIspLscCfg[LUT_PREVIEW].shading_ctrl2.bits.SDBLK_WIDTH/mu32PreResXsize+1)&0xffE;
            
            m_rIspLscCfg[LUT_CAPTURE].shading_ctrl2.bits.SDBLK_HEIGHT = 
                (mu32CapResYsize*m_rIspLscCfg[LUT_PREVIEW].shading_ctrl2.bits.SDBLK_HEIGHT/mu32PreResYsize+1)&0xffE;       
    
            m_rIspLscCfg[LUT_CAPTURE].shading_last_blk.bits.SDBLK_LWIDTH = 
                mu32CapResXsize - 
                m_rIspLscCfg[LUT_CAPTURE].shading_ctrl2.bits.SDBLK_WIDTH*
                m_rIspLscCfg[LUT_CAPTURE].shading_ctrl2.bits.SDBLK_XNUM;
            
            m_rIspLscCfg[LUT_CAPTURE].shading_last_blk.bits.SDBLK_LHEIGHT =
                mu32CapResYsize - 
                m_rIspLscCfg[LUT_CAPTURE].shading_ctrl2.bits.SDBLK_HEIGHT*
                m_rIspLscCfg[LUT_CAPTURE].shading_ctrl2.bits.SDBLK_YNUM;        
        }


        fgRet = MTRUE;
        return  fgRet;


}

//+++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LUT
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    get
    inline
    MUINT32*
    getLut() const
    {
      MY_LOG(
                "[LscMgr] "
                "getLut m_u4Mode %d m_u4Idx%d \n"
                ,m_u4Mode
                ,m_u4Idx
                );         
        switch (m_u4Mode)
        {
        case LUT_PREVIEW:      
            return  &m_rIspShadingLut.PreviewTable[0][0]; // preview load all table
        case LUT_CAPTURE:
            if(LscGetM4u())
            {
                return  &m_rIspShadingLut.CaptureTable[0][0]; // preview load all table
            }
            else
            {
                return  &m_rIspShadingLut.PreviewTable[0][0]; // preview load all table
            }
        	  //return  &m_rIspShadingLut.CaptureTable[m_u4Idx][0];//capture only load one table 
        case LUT_BINNING:    
        default:
            break;
        }
        return  NULL;
    }

    inline
    MUINT32
    getLutSize() const
    {
      MY_LOG(
                "[LscMgr] "
                "getLut m_u4Mode %d\n"
                ,m_u4Mode
                );         
    
        switch (m_u4Mode)
        {
        	case LUT_PREVIEW:
        	    if(LscGetM4u())
        	    {
        	        return LUT_SIZE_Preview_M4u;
        	    }
        	    else
        	    {
        	        return LUT_SIZE_Preview;
        	    }
     		case LUT_CAPTURE:
        	    if(LscGetM4u())
        	    {
        	        return LUT_SIZE_Capture_M4u;
        	    }
        	    else
        	    {
        	        return LUT_SIZE_Capture;
        	    }
		case LUT_BINNING:     
        	    if(LscGetM4u())
        	    {
        	        return LUT_SIZE_Preview_M4u;
        	    }
        	    else
        	    {
        	        return LUT_SIZE_Preview;
        	    }
        	default:
        	    break;
        }
        return NULL;
    }

private:    ////    Reference.
    //  Reference to rIspNvram.Shading
    typedef ISP_NVRAM_SHADING_T LSCParameter[3];
    LSCParameter&        m_rIspLscCfg;
    ISP_SHADING_STRUCT&          m_rIspShadingLut;
//+++++++++++++++++++++
//
//+++++++++++++++++++++++

//++++++++++++
//M4U
//++++++++++++
#if 1
private:
    MTKM4UDrv *pM4UDrv;//=NULL;//new MTKM4UDrv();
    RawLscM4uInfo_t stRawLscM4uInfo[RAW_LSC_TBL_NUM];
    M4U_MODULE_ID_ENUM eM4U_ID;//=M4U_CLNTMOD_CAM;
    M4U_PORT_STRUCT M4uPort;
    MBOOL m_bM4uEn;//=MTRUE;
#endif
private:
//#if LSC_TBL_M4U    

//#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    load
    inline
    MVOID
    loadLut()
    {
        m_u4ChangeCount = 0;
        loadLutToSysram();
        if(!LscGetM4u())
        {
            ZsdConfigUpdate(); //120420
        }
        
        //return  SetTBAToISP();
    }

    inline
    MVOID
    loadLutToSysram()
    {   //  VA <- LUT
      MY_LOG(
                "[LscMgr] "
                "loadLutToSysram m_u4Mode %d, m_pvVirTBA 0x%x \n"
                ,m_u4Mode
                ,reinterpret_cast<MUINT32>(m_pvVirTBA)
                );      
        if  (m_pvVirTBA == NULL)
        {
        	MY_LOG(
        	  "[LscMgr] "
             "Err :: load shading table to NULL address (m_pvVirTBA, 0x%x) \n"
             ,reinterpret_cast<MUINT32>(m_pvVirTBA)
        	);
        	return;
        }
        switch (m_u4Mode)
        {
        	case LUT_PREVIEW:
                            if(LscGetM4u())
                            {
                                ::memcpy(m_pvVirTBA, getLut(), LUT_SIZE_Preview_M4u);
                                RawLscTblM4uFlushCurrTbl(LUT_PREVIEW);
                            }
                            else
                            {
                                ::memcpy(m_pvVirTBA, getLut(), LUT_SIZE_Preview);
                            }   				
   				break;
     		case LUT_CAPTURE:
     		              if(LscGetM4u())
     		              {
     		                  ::memcpy(m_pvVirTBA, getLut(), LUT_SIZE_Capture_M4u);
     		                  RawLscTblM4uFlushCurrTbl(LUT_CAPTURE);
     		              }
     		              else
     		              {
               				::memcpy(m_pvVirTBA, getLut(), LUT_SIZE_Capture);
     		              }
   				break;
   			case LUT_BINNING:     
   			default:
   				break;
        }
        
        return;
    }

    inline
    MBOOL
    SetTBAToISP()
    {   //  ISP <- PA
        switch (m_u4Mode)
        {
        	case LUT_PREVIEW:
        		m_rIspLscCfg[m_u4Mode].shading_read_addr.bits.SHADING_RADDR = 
        		    reinterpret_cast<MUINT32>(m_pvPhyTBA) + MAX_SHADING_Preview_SIZE*m_u4Idx*4;
         MY_LOG(
                "[LscMgr]SetTBAToISP:LUT_PREVIEW "
                "m_rIspLscCfg[m_u4Mode].shading_read_addr.bits.SHADING_RADDR, 0x%8x\n"
                ,m_rIspLscCfg[m_u4Mode].shading_read_addr.bits.SHADING_RADDR
                );   
     			return  ISP_MGR_SHADING_T::getInstance().directSetTBA(
     				reinterpret_cast<MUINT32>(m_pvPhyTBA) + MAX_SHADING_Preview_SIZE*m_u4Idx*4
     				);
     		case LUT_CAPTURE:
     		    if(LscGetM4u())
     		    {
     		        m_rIspLscCfg[m_u4Mode].shading_read_addr.bits.SHADING_RADDR = 
     		            reinterpret_cast<MUINT32>(m_pvPhyTBA) + MAX_SHADING_Capture_SIZE*m_u4Idx*4;
     		        MY_LOG(
     		            "[LscMgr]SetTBAToISP:LUT_CAPTURE "
     		            "m_rIspLscCfg[m_u4Mode].shading_read_addr.bits.SHADING_RADDR, 0x%8x\n"
     		            ,m_rIspLscCfg[m_u4Mode].shading_read_addr.bits.SHADING_RADDR
     		            );   
     		        return  ISP_MGR_SHADING_T::getInstance().directSetTBA(
     		            reinterpret_cast<MUINT32>(m_pvPhyTBA) + MAX_SHADING_Capture_SIZE*m_u4Idx*4
     		            );
     		    }
     		    else
     		    {
     		        m_rIspLscCfg[m_u4Mode].shading_read_addr.bits.SHADING_RADDR = 
     		            reinterpret_cast<MUINT32>(m_pvPhyTBA) + MAX_SHADING_Preview_SIZE*m_u4Idx*4;
     		        MY_LOG(
     		            "[LscMgr]SetTBAToISP:LUT_PREVIEW "
     		            "m_rIspLscCfg[m_u4Mode].shading_read_addr.bits.SHADING_RADDR, 0x%8x\n"
     		            ,m_rIspLscCfg[m_u4Mode].shading_read_addr.bits.SHADING_RADDR
     		            );   
     		        return  ISP_MGR_SHADING_T::getInstance().directSetTBA(
     		            reinterpret_cast<MUINT32>(m_pvPhyTBA) + MAX_SHADING_Preview_SIZE*m_u4Idx*4
     		            );
     		    }	
     		case LUT_BINNING:     
              default:
                    break;
        }    
        return MFALSE; //Bining mode not support now
    }

    inline
    MVOID
    loadConfig()
    {
        prepareIspHWBuf( m_rIspLscCfg[m_u4Mode] );
    }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    

    inline
    MBOOL
    isEnable() const
    {
                MY_LOG(
                      "[LscMgr] -isEnable[m_u4Mode=%d]"
                      "  --> %d\n"
                      ,m_u4Mode,m_rIspLscCfg[m_u4Mode].shading_ctrl1.bits.SHADING_EN
                      );           
        return  (0 != m_rIspLscCfg[m_u4Mode].shading_ctrl1.bits.SHADING_EN);
    }

    inline
    MBOOL
    enableLscWoVariable(MBOOL const fgEnable)
    {
        MBOOL fgRet = MFALSE;
        MBOOL OrgShadingEn = MFALSE;
            MY_LOG(
                      "[LscMgr] -enableLsc(enableLscWoVariable)"
                      "  --> %d\n"
                      ,fgEnable
                      );                      

        //  (1) Check to see whether to change the enable state.
        OrgShadingEn = m_rIspLscCfg[m_u4Mode].shading_ctrl1.bits.SHADING_EN;
            MY_LOG(
                      "[LscMgr] -OrgShadingEn"
                      "  --> %d\n"
                      ,OrgShadingEn
                      );          
        //  (2) Change the state of NVRAM data
        if  ( fgEnable )//  Disable --> Enable
        {
            m_rIspLscCfg[m_u4Mode].shading_ctrl1.bits.SHADING_EN  = 1;
        }
        else            //  Enable --> Disable
        {
            m_rIspLscCfg[m_u4Mode].shading_ctrl1.bits.SHADING_EN = 0;
        }
        //  (3) Apply to ISP.
        fgRet = ISP_MGR_SHADING_T::getInstance().reset();
        if  ( fgRet )
        {
            ISP_MGR_SHADING_T::getInstance().put(m_rIspLscCfg[m_u4Mode]); //put to ispmgr_mt6573
            fgRet = ISP_MGR_SHADING_T::getInstance().apply();//Apply to ISP.
            m_rIspLscCfg[m_u4Mode].shading_ctrl1.bits.SHADING_EN=OrgShadingEn;            
        }
              MY_LOG(
                      "[LscMgr] -m_rIspLscCfg[%d].shading_ctrl1.bits.SHADING_EN = %d"
                      "  --> %d\n"
                      ,m_u4Mode,m_rIspLscCfg[m_u4Mode].shading_ctrl1.bits.SHADING_EN,fgEnable
                      );         

 //   lbExit:
        return  fgRet;
    }

    inline
    MBOOL
    enableLsc(MBOOL const fgEnable)
    {
        MBOOL fgRet = MFALSE;
        MY_LOG(
            "[LscMgr] -enableLsc"
            "  --> %d\n"
            ,fgEnable
            );                      
        //  (1) Check to see whether to change the enable state.
        if  ( (0 != fgEnable) == (0 != isEnable()) )
        {   //  Do nothing if no change.
            fgRet = MTRUE;
            MY_LOG(
                      "[LscMgr] -enableLsc\n"
                      "Do nothing if no change %d\n"
                      ,fgEnable
                      );             
            {
                goto lbExit;
            }
        }

        //  (2) Change the state of NVRAM data
        if  ( fgEnable )//  Disable --> Enable
        {
            m_rIspLscCfg[m_u4Mode].shading_ctrl1.bits.SHADING_EN  = 1;
        }
        else            //  Enable --> Disable
        {
            m_rIspLscCfg[m_u4Mode].shading_ctrl1.bits.SHADING_EN = 0;
        }       

        //  (3) Apply to ISP.
        fgRet = ISP_MGR_SHADING_T::getInstance().reset();
        if  ( fgRet )
        {
            ISP_MGR_SHADING_T::getInstance().put(m_rIspLscCfg[m_u4Mode]); //put to ispmgr_mt6573
            fgRet = ISP_MGR_SHADING_T::getInstance().apply();//Apply to ISP.
        }

    lbExit:
        return  fgRet;
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//M4U functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline MINT32 RawLscfreeM4UMemory(MUINT32 m4uVa, MUINT32 size,MUINT32 m4uMVa)
{
    if (pM4UDrv == NULL)  {
        LOGD("RawLsc Null M4U driver \n"); 
        return -1;
    } 
    MINT32 ret = 0; 
    LOGD("RawLsc [m4u_invalid_tlb_range] m4uMVa = 0x%x, size = %d \n", m4uMVa, (m4uMVa+size-1)); 
    ret = pM4UDrv->m4u_invalid_tlb_range( M4U_CLNTMOD_CAM, m4uMVa , (m4uMVa+size-1));
    if(ret!=0)
    {
    	 LOGD("RawLsc [freeM4UMemory] m4u_invalid_tlb_range fail \n"); 
    }
    LOGD("RawLsc [m4u_dealloc_mva] m4uMVa = 0x%x, size = %d \n", m4uMVa, size); 
    ret = pM4UDrv->m4u_dealloc_mva( M4U_CLNTMOD_CAM, m4uVa , size, m4uMVa);

    return ret; 
}


    
    inline
    MBOOL 
    RawLscTblM4uMemInfoShow(MUINT32 const u8LscIdx)
{
        #if LSC_TBL_M4U	
        MY_LOG("[LscMgr]RawLscTblM4uMemInfoShow()>>\n");
        MY_LOG("[LscMgr][%d].u4VirAddr 0x%8x\n",u8LscIdx,stRawLscM4uInfo[u8LscIdx].u4VirAddr);       
        MY_LOG("[LscMgr][%d].u4UseM4u 0x%8x\n",u8LscIdx,stRawLscM4uInfo[u8LscIdx].u4UseM4u);       
        MY_LOG("[LscMgr][%d].u4CamPortMva 0x%8x\n",u8LscIdx,stRawLscM4uInfo[u8LscIdx].u4CamPortMva);    
        MY_LOG("[LscMgr][%d].u4BufSize 0x%8x\n",u8LscIdx,stRawLscM4uInfo[u8LscIdx].u4BufSize);   
        MY_LOG("[LscMgr]RawLscTblM4uMemInfoShow()<<\n");        
        #endif
        return MTRUE;
        
}
    inline
    MBOOL 
    RawLscTblM4uFlushCurrTbl(MUINT32 const u8LscIdx)
{    
    #if LSC_TBL_M4U	
    MY_LOG("[LscMgr]RawLscTblM4uFlushCurrTbl(eM4U_ID=%d)\n",eM4U_ID);       
    MY_LOG("[LscMgr](u8LscIdx=%d)\n",u8LscIdx);       
//    eM4U_ID = M4U_CLNTMOD_CAM;  //
    pM4UDrv->m4u_cache_sync(eM4U_ID , 
    	M4U_CACHE_FLUSH_BEFORE_HW_READ_MEM ,
    	stRawLscM4uInfo[u8LscIdx].u4VirAddr, 
    	stRawLscM4uInfo[u8LscIdx].u4BufSize);
    #endif
    return MTRUE;
}


    inline
    MBOOL 
    RawLscTblM4uSetPhyVirAddr(MUINT32 const u8LscIdx
	,MVOID* pPhyAddr ,MVOID* pVirAddr)
{
       MBOOL ret=MFALSE;
       #if LSC_TBL_M4U	
	if(stRawLscM4uInfo[u8LscIdx].u4VirAddr!=0)
      	{
            pPhyAddr = (MVOID*)stRawLscM4uInfo[u8LscIdx].u4CamPortMva;
            pVirAddr = (MVOID*)stRawLscM4uInfo[u8LscIdx].u4VirAddr;      
            MY_LOG("pPhyAddr 0x%8x\n",(MUINT32)pPhyAddr);    
            MY_LOG("pVirAddr 0x%8x\n",(MUINT32)pVirAddr);      
//            MY_LOG("[allocM4UMemory] m4uVa = 0x%x \n", stRawLscM4uInfo[u8LscIdx].u4CamPortMva);         
            ret = MTRUE;
       }
	#else
	ret = MFALSE;
	#endif
        return ret;
}


    inline
    MUINT32 
    RawLscTblM4uGetPhyAddr(MUINT32 const u8LscIdx)
{
       #if LSC_TBL_M4U	
	if(stRawLscM4uInfo[u8LscIdx].u4CamPortMva!=0)
      	{
            MY_LOG("[allocM4UMemory] u4CamPortMva = 0x%x \n", stRawLscM4uInfo[u8LscIdx].u4CamPortMva);         
       }
	else
	{
	
            MY_LOG("ERROR [allocM4UMemory] u4CamPortMva = 0x%x \n", stRawLscM4uInfo[u8LscIdx].u4CamPortMva); 
	}	
	#endif
	return stRawLscM4uInfo[u8LscIdx].u4CamPortMva;
}

    inline
    MUINT32 
    RawLscTblM4uGetVirAddr(MUINT32 const u8LscIdx)
{
       #if LSC_TBL_M4U	
	if(stRawLscM4uInfo[u8LscIdx].u4VirAddr!=0)
      	{
            MY_LOG("[allocM4UMemory] u4VirAddr = 0x%x \n", stRawLscM4uInfo[u8LscIdx].u4VirAddr);         
       }
	else
	{	
            MY_LOG("ERROR [allocM4UMemory] u4VirAddr = 0x%x \n", stRawLscM4uInfo[u8LscIdx].u4VirAddr); 
	}	
	#endif
	return stRawLscM4uInfo[u8LscIdx].u4VirAddr;
}

    inline
    MBOOL 
    RawLscTblM4uAlloc(MUINT32 const u8LscIdx,MUINT32 const u8LscLutSize)
{
    MBOOL mbret=MFALSE;
    MUINT32 ret;
    eM4U_ID = M4U_CLNTMOD_CAM;  //must add even LscMgr()
    
    #if LSC_TBL_M4U	
    if(!stRawLscM4uInfo[u8LscIdx].u4VirAddr)
    {
        stRawLscM4uInfo[u8LscIdx].u4VirAddr = (MUINT32)memalign(L1_CACHE_BYTES, 
            (u8LscLutSize + (L1_CACHE_BYTES)) & ~(L1_CACHE_BYTES -1)); 
        
        MY_LOG("malloc(stRawLscM4uInfo[%d].u4VirAddr0x%8x, size%d\n", u8LscIdx,
            stRawLscM4uInfo[u8LscIdx].u4VirAddr,u8LscLutSize);    
        //                memset((MUINT8*)stRawLscM4uInfo[u8LscIdx].u4VirAddr,0x00, u8LscLutSize);                            
        stRawLscM4uInfo[u8LscIdx].u4BufSize = u8LscLutSize;
        ret = pM4UDrv->m4u_alloc_mva(eM4U_ID , 
        	stRawLscM4uInfo[u8LscIdx].u4VirAddr,  
        	stRawLscM4uInfo[u8LscIdx].u4BufSize,
        	&stRawLscM4uInfo[u8LscIdx].u4CamPortMva);        
        if(!ret)
        {        
                LOGD("RawLsc [m4u_insert_tlb_range] u4CamPortMva,END = 0x%8x, 0x%8x \n", 
                    stRawLscM4uInfo[u8LscIdx].u4CamPortMva, 
                    stRawLscM4uInfo[u8LscIdx].u4CamPortMva + stRawLscM4uInfo[u8LscIdx].u4BufSize-1);

                pM4UDrv->m4u_insert_tlb_range(eM4U_ID, 
                    stRawLscM4uInfo[u8LscIdx].u4CamPortMva,  
                    stRawLscM4uInfo[u8LscIdx].u4CamPortMva + stRawLscM4uInfo[u8LscIdx].u4BufSize-1 , 
                    RT_RANGE_HIGH_PRIORITY,  1);
                
                M4uPort.ePortID = M4U_PORT_CAM;
                M4uPort.Virtuality = 1;	   	// 0: means use physical address directly. 1: means enable M4U for this port.
                M4uPort.Security = 0;		// Security zone support, dose not used now, set to 0.
                M4uPort.Distance = 1;		// Prefetch distance in page unit. Set to 1 for normal sequential memory access, means will prefetch next one page when processing current page.
                M4uPort.Direction = 0;		// 0: means access from low address to high address. 1: means access from high address to low address.
                pM4UDrv->m4u_config_port(&M4uPort);  
                mbret=MTRUE;
        }
    }
    else
    {                
        mbret=MTRUE;
        MY_LOG("already ! malloc(stRawLscM4uInfo[%d].u4VirAddr0x%8x, size%d\n", u8LscIdx,stRawLscM4uInfo[u8LscIdx].u4VirAddr,u8LscLutSize);    
    }
    #else
    mbret = MTRUE;
    #endif
    return mbret;

}
    inline
    MBOOL 
    RawLscTblM4uInit()
    {
       MBOOL ret=MFALSE;
    #if LSC_TBL_M4U    
        UINT32 u8LscIdx=0;
        MUINT32 u4BufSize[RAW_LSC_TBL_NUM]={LUT_SIZE_Preview_M4u,LUT_SIZE_Capture_M4u};	
        ret=MTRUE;
        if(!pM4UDrv)
        {    
            MY_LOG("new MTKM4UDrv()\n");
            MY_LOG("sizeof(stRawLscM4uInfo) = %d\n",sizeof(stRawLscM4uInfo));
            memset(&stRawLscM4uInfo,0x00, sizeof(RawLscM4uInfo_t)*RAW_LSC_TBL_NUM);   
            pM4UDrv = new MTKM4UDrv();
            if (!pM4UDrv)
            {
                MY_LOG("pM4UDrv new fail.\n");
                ret=MFALSE;
            }       
            for(u8LscIdx=0;u8LscIdx<RAW_LSC_TBL_NUM;u8LscIdx++)
            {
                if(!RawLscTblM4uAlloc(u8LscIdx,u4BufSize[u8LscIdx]))  
                {
                    MY_LOG("RawLscTblM4uAlloc(%d) FAILED\n",u8LscIdx);
                }    
                else
                {
                    RawLscTblM4uMemInfoShow(u8LscIdx);  
                }
             }
        }      
        else
        {
            MY_LOG("pM4UDrv = 0x%8x\n",(MUINT32)pM4UDrv);
        }
    #endif    
        return ret;
    }
    inline
    MBOOL 
    RawLscTblM4uUnInit() 
{
    #if LSC_TBL_M4U	
    UINT32 lu32ErrCode=0,ret=0;
    UINT32 i=0;
    MY_LOG("~ RawLscTblM4uUnInit()!!!\n");  
    for(i=0;i<RAW_LSC_TBL_NUM;i++)
    {
        RawLscfreeM4UMemory(stRawLscM4uInfo[i].u4VirAddr,
            stRawLscM4uInfo[i].u4BufSize,
            stRawLscM4uInfo[i].u4CamPortMva);
    }
    for(i=0;i<RAW_LSC_TBL_NUM;i++)
    {
        if(stRawLscM4uInfo[i].u4VirAddr)
        {
            free((void*)stRawLscM4uInfo[i].u4VirAddr);  //from AppFD.cpp
            MY_LOG("~ free u4VirAddr(0x%8x)!!!\n",stRawLscM4uInfo[i].u4VirAddr);  
            stRawLscM4uInfo[i].u4VirAddr = NULL;
        }        
    }    
    memset(&stRawLscM4uInfo,0x00, sizeof(RawLscM4uInfo_t)*RAW_LSC_TBL_NUM);   
    if (pM4UDrv)
    {
        delete pM4UDrv;
        pM4UDrv = NULL;
    }
    #endif	// LSC_TBL_M4U
    return MTRUE;
}
    public:     ////    
    LscMgr(ISP_NVRAM_REGISTER_STRUCT* const rIspNvram, ISP_SHADING_STRUCT& rIspShadingLut)
        : m_u4ChangeCount(1)     
        , m_u4Idx(1)
        , m_u4Mode(0)
        , m_pvPhyTBA(NULL)
        , m_pvVirTBA(NULL)
        , m_rIspLscCfg(rIspNvram->Shading)
        , m_rIspShadingLut(rIspShadingLut)
        , pM4UDrv(NULL)
        , eM4U_ID(M4U_CLNTMOD_CAM)
    {
        MY_LOG(
            "[LscMgr] "
            "pM4UDrv == 0x%x\n"
            "ENTER LscMgr() \n",(MUINT32)pM4UDrv);
        {
            ISP_NVRAM_SHADING_T debug;
            ISP_MGR_SHADING_T::getInstance().get(debug);
                  MY_LOG(
                                "[LscMgr]  "
                            "LscMgr() Shading param 0x%8x,0x%x ,0x%x ,0x%x ,0x%x \n"
                            , debug.shading_ctrl1.val
                            , debug.shading_ctrl2.val
                            , debug.shading_read_addr.val
                            , debug.shading_last_blk.val
                            , debug.shading_ratio_cfg.val
                        );      
        } 

        #if LSC_TBL_M4U  
        m_bM4uEn=MTRUE;
        LscM4uEn(MTRUE); //inform Lsc_mgr.h to use capture table blocks even in ZSD_BUFFER_CNT
        if(!RawLscTblM4uInit())
        {    
            MY_LOG("FATAL WRONG pM4UDrv new fail.\n");
        }        

        MY_LOG(
                "[LscMgr] "
                "ENTER LscMgr(pM4UDrv - 0x%8x) \n",(MUINT32)pM4UDrv);      
        #else
        m_bM4uEn=MFALSE;
        MY_LOG(
                "[LscMgr] "
                "ENTER LscMgr(No pM4UDrv) \n");      
        #endif        
    }



    ~LscMgr()
    {
        MY_LOG(
            "[LscMgr] "
            "EXIT ~LscMgr(pM4UDrv - 0x%8x) >>\n",
            (MUINT32)pM4UDrv
            );         
        enableLscWoVariable(MFALSE);
        {
            ISP_NVRAM_SHADING_T debug;
            ISP_MGR_SHADING_T::getInstance().get(debug);
                  MY_LOG(
                                "[LscMgr]  "
                            "~LscMgr() Shading param 0x%8x,0x%x ,0x%x ,0x%x ,0x%x \n"
                            , debug.shading_ctrl1.val
                            , debug.shading_ctrl2.val
                            , debug.shading_read_addr.val
                            , debug.shading_last_blk.val
                            , debug.shading_ratio_cfg.val
                        );      
        }
        MY_LOG(
            "[LscMgr] "
            "EXIT ~LscMgr(pM4UDrv - 0x%8x) <<\n",
            (MUINT32)pM4UDrv
            );       
        RawLscTblM4uUnInit();    
    }


};


/*******************************************************************************
* 
*******************************************************************************/
};  //  NSIspTuning
#endif // _LSC_MGR_H_

