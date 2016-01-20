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
//#define LOG_TAG "NSIspTuning::ParamctrlRAW_frameless"
#if defined(LOG_TAG)
#undef (LOG_TAG)
#endif
#define LOG_TAG "RAW_frameless_cpp"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#include "debug.h"
#include "paramctrl_raw.h"
//
#include "m4u_lib.h"		// For MT6575 M4UDrv.
#include <linux/cache.h>
using namespace android;
using namespace NSIspTuning;

#define LOG_MSG(fmt, arg...)    LOGD("-%s()" fmt, __FUNCTION__, ##arg) //XLOGD("[%s]"fmt, __FUNCTION__, ##arg) //#define STNR_LOG(fmt, arg...)	 LOGD(STNR_HAL_TAG fmt, ##arg)

#if defined(MTK_M4U_SUPPORT)
#define RAW_LSC_TBL_M4U_EN 1
//#include "m4u_lib.h"		// For MT6575 M4UDrv.
#else
#define RAW_LSC_TBL_M4U_EN 0
#endif 


#define RAW_LSC_TBL_PV 0
#define RAW_LSC_TBL_CP 1

MBOOL
ParamctrlRAW::
applyToHw_Frameless_All()
{
    return  MTRUE
        &&  ISP_MGR_SHADING_T::getInstance().apply() //apply ispmgr_mt6575 to isp register
        ;
}


MBOOL
ParamctrlRAW::
prepareHw_Frameless_All()
{
    MBOOL fgRet = MTRUE;

    //  (1) prepare something and fill buffers.
    fgRet = MTRUE
        &&  prepare_Frameless_Pca()
        &&  prepare_Frameless_Shading()
            ;
    if  ( ! fgRet )
    {
        goto lbExit;
    }

lbExit:
    return  fgRet;
}


MBOOL
ParamctrlRAW::
prepare_Frameless_Pca()
{
    MBOOL fgRet = MFALSE;

    m_pIPcaMgr = IPcaMgr::getInstance(m_IspCamInfo.eCamMode, m_rIspParam.ISPPca);

    //  (1) Check to see whether PCA is enabled.
    if  ( ! m_pIPcaMgr->isEnable() )
    {
        m_SysramMgr.free(NSIspSysram::EUsr_PCA);
        fgRet = MTRUE;
        goto lbExit;
    }

    //  (2) Preset PCA (disable PCA at the moment).
    if  ( ! m_pIPcaMgr->preset() )
    {
        goto lbExit;
    }

    //  (3) Now, free all resources.
    m_pIPcaMgr->savePhyTBA(NULL);
    m_pIPcaMgr->saveVirTBA(NULL);
    m_SysramMgr.free(NSIspSysram::EUsr_PCA);

    fgRet = MTRUE;
lbExit:
    return  fgRet;
}


MBOOL
ParamctrlRAW::
prepare_Frameless_Shading()
{
    MBOOL fgRet = MTRUE;
    UINT8 Shading_Parameter_IDX;
    UINT8 u8LscIdx=0;
    MINT32 ret = 0;	// 0: no error.
    UINT32 lu32ErrCode=0;
#if RAW_LSC_TBL_M4U_EN  
    m_LscMgr.LscM4uEn(MTRUE); //inform Lsc_mgr.h to use capture table blocks even in ZSD
    if(!m_LscMgr.RawLscTblM4uInit())
    {    
        LOG_MSG("pM4UDrv new fail.\n");
        lu32ErrCode = 1;
        goto lbExit_fail;
    }
#else
    m_LscMgr.LscM4uEn(MFALSE);
    LOG_MSG("RAW_LSC_TBL_SYSRAM\n");
#endif

    if ((m_IspRegMgr.getIdx_Shading()!=0)&&(m_IspRegMgr.getIdx_Shading()!=1))
    {
        MY_LOG(
                    "[prepare_Frameless_Shading] "
                    " Shading Parameter Index 2, Disable Shading\n"
                );    
        m_LscMgr.setMode(2); // Set binnig mode, should disable shading 
        m_LscMgr.loadConfig(); // set NVRAM data to ispmgr_mt6575
        fgRet = m_LscMgr.enableLsc(0);// disable shading
        goto lbExit;
    }
        

    MY_LOG(
                "[prepare_Frameless_Shading] "
                " (CamMode,SensorMode,operation mode, CCT) = (%d,%d,%d,%d)\n"
                , m_IspCamInfo.eCamMode //   ECamMode_Video          = 0,     ECamMode_Online_Preview = ECamMode_Video,     ECamMode_Online_Capture, 
                , getSensorMode()
                , getOperMode()
                , m_IspCamInfo.eIdx_Shading_CCT
            );

    // (1) free allocated SRAM when mode change
    //      The very 1st time when camera start-up, free EUsr_Shading_Capture will cause error.
    //      Ignore this error code from SRAM driver is ok.
    switch (m_IspCamInfo.eCamMode)
    {
         case ECamMode_Online_Preview:
         case ECamMode_Video:
        #if !RAW_LSC_TBL_M4U_EN            	
             m_SysramMgr.free(NSIspSysram::EUsr_Shading_Capture);
        #endif
             break;
         case ECamMode_Online_Capture:
         case ECamMode_Online_Capture_ZSD:
#ifdef MTK_ZSD_AF_ENHANCE
         case ECamMode_Online_Preview_ZSD:
#endif
         case ECamMode_Offline_Capture_Pass1:
         case ECamMode_HDR_Cap_Pass1_SF:
         case ECamMode_HDR_Cap_Pass1_MF2:
             switch (getSensorMode())
             {
                 case ESensorMode_Preview : //engineer mode preivew raw capture
                 #if !RAW_LSC_TBL_M4U_EN            	
        	     m_SysramMgr.free(NSIspSysram::EUsr_Shading_Capture);        	     
                  #endif
        	     break;        	     
      		 case ESensorMode_Capture :
      		 default :
        #if !RAW_LSC_TBL_M4U_EN         
        	     m_SysramMgr.free(NSIspSysram::EUsr_Shading_Preview);
        #endif
        	     break;
             }
             break;
         case ECamMode_Offline_Capture_Pass2:
         case ECamMode_HDR_Cap_Pass1_MF1:
         case ECamMode_HDR_Cap_Pass2:
         default:
              break;
    }	    

    //  (2) Here, make sure that sysram is available.
    //  re-allocate is acceptable, but only new allocate will reserve
    {
        MVOID* pPhyAddr = NULL;
        MVOID* pVirAddr = NULL;        
        MERROR_ENUM err = MERR_OK;
        switch (m_IspCamInfo.eCamMode)
        {
            case ECamMode_Online_Preview:
            case ECamMode_Video:
        	m_LscMgr.setMode(0); // Set preview mode for  LscMgr get table size
    #if RAW_LSC_TBL_M4U_EN    
            u8LscIdx = RAW_LSC_TBL_PV;    
            if(!m_LscMgr.RawLscTblM4uAlloc(u8LscIdx,m_LscMgr.getLutSize())) 
            {
                lu32ErrCode = 2;
                goto lbExit_fail;
            }
    #else            	
            	err = m_SysramMgr.autoAlloc(
            	    NSIspSysram::EUsr_Shading_Preview, m_LscMgr.getLutSize(), pPhyAddr, pVirAddr
            	);
    #endif
        	break;
            case ECamMode_Online_Capture_ZSD: //120417            	
#ifdef MTK_ZSD_AF_ENHANCE          	
            case ECamMode_Online_Preview_ZSD:
#endif
            case ECamMode_Online_Capture:
            case ECamMode_Offline_Capture_Pass1:
            case ECamMode_HDR_Cap_Pass1_SF:
            case ECamMode_HDR_Cap_Pass1_MF2:
            switch (getSensorMode())
            {
               case ESensorMode_Preview : //engineer mode preivew raw capture, allocate preview buffer
                   m_LscMgr.setMode(0); // Set preview mode for  LscMgr get table size
                   #if RAW_LSC_TBL_M4U_EN 
                   u8LscIdx = RAW_LSC_TBL_PV;
                   if(!m_LscMgr.RawLscTblM4uAlloc(u8LscIdx,m_LscMgr.getLutSize()))
                   {
                       lu32ErrCode = 3;
                       goto lbExit_fail;
                   }
                   #else
                   err = m_SysramMgr.autoAlloc(
                   NSIspSysram::EUsr_Shading_Preview, m_LscMgr.getLutSize(), pPhyAddr, pVirAddr
                   );
                   #endif
                   break;
               case ESensorMode_Capture :
               default :
                   m_LscMgr.setMode(1); // Set capture mode for  LscMgr get table size
                   #if RAW_LSC_TBL_M4U_EN            	
                   u8LscIdx = RAW_LSC_TBL_CP;
                   if(!m_LscMgr.RawLscTblM4uAlloc(u8LscIdx,m_LscMgr.getLutSize()))
                   {
                       lu32ErrCode = 4;
                       goto lbExit_fail;
                   }
                   #else		        
                   err = m_SysramMgr.autoAlloc(
                   NSIspSysram::EUsr_Shading_Capture, m_LscMgr.getLutSize(), pPhyAddr, pVirAddr
                   );
                   #endif
                   break;
            }        		
            break;
            case ECamMode_Offline_Capture_Pass2:
            case ECamMode_HDR_Cap_Pass1_MF1:
            case ECamMode_HDR_Cap_Pass2:
        	switch (getSensorMode())
        	{
        	    case ESensorMode_Preview : 
		        m_LscMgr.setMode(0); // Set preview mode for  configure shading parameters
        		break;
      		    case ESensorMode_Capture :
      		    default :
		        m_LscMgr.setMode(1); // Set capture  mode for  configure shading parameters
        		break;
        	 }        		
        	 break;
            default:
        	 break;
        }	        
        if  ( MERR_OK != err )
        {
            fgRet = MFALSE;
            goto lbExit;
        }
    #if RAW_LSC_TBL_M4U_EN      
        m_LscMgr.RawLscTblM4uMemInfoShow(u8LscIdx);
        if(m_LscMgr.getMode()!=u8LscIdx)
        {
            LOG_MSG("WRONG !!! u8LscIdx,m_LscMgr.getMode() 0x%8x,0x%8x\n",u8LscIdx,m_LscMgr.getMode());      
            u8LscIdx = m_LscMgr.getMode();
            LOG_MSG("Force to lscmode !!!--> "
            	"u8LscIdx,m_LscMgr.getMode() 0x%8x,0x%8x\n",u8LscIdx,m_LscMgr.getMode());                
        }
/*        if(m_LscMgr.RawLscTblM4uSetPhyVirAddr(m_LscMgr.getMode(),pPhyAddr,pVirAddr)==MFALSE)  
        {
            LOG_MSG("!!!WRONG UNABLE to update pPhyAddr,pVirAddr!!!\n");      
            lu32ErrCode = 5;
            goto lbExit_fail;          
        }*/
        pPhyAddr =  (MVOID*)m_LscMgr.RawLscTblM4uGetPhyAddr(m_LscMgr.getMode());
        pVirAddr =  (MVOID*)m_LscMgr.RawLscTblM4uGetVirAddr(m_LscMgr.getMode());        
        LOG_MSG("pPhyAddr 0x%8x\n",(MUINT32)pPhyAddr);    
        LOG_MSG("pVirAddr 0x%8x\n",(MUINT32)pVirAddr);      
        #endif
        MY_LOG(
            "[prepare_Frameless_Shading] "
            "Shading table address  Phy 0x%x Vir 0x%x\n"
            ,reinterpret_cast<MUINT32>(pPhyAddr) 
            ,reinterpret_cast<MUINT32>(pVirAddr)
        );       
        

        m_LscMgr.savePhyTBA(pPhyAddr);
        m_LscMgr.saveVirTBA(pVirAddr);
    }

    //  (3) prepare shading table and parameter for normal operation
    //       meta mode will be setted at "prepareHw_PerFrame_Shading"
    //       1. write shading parameter to register
    //       2. Load Shading table to SRAM
    if (getOperMode() == EOperMode_Meta)
    {
         // capture doesn't call "prepareHw_PerFrame_Shading", must set shading parameters here  
         if (m_LscMgr.isEnable()!=0)
        {
            MY_LOG(
                                "[prepare_Frameless_Shading] "
                                "m_LscMgr.isEnable()!=0 : "
                                "m_LscMgr.loadLut();    "
                                "fgRet = m_LscMgr.SetTBAToISP() \n "                                
                             );	            

            m_LscMgr.loadLut(); //load table   
            #if RAW_LSC_TBL_M4U_EN      
//            m_LscMgr.RawLscTblM4uFlushCurrTbl(m_LscMgr.getMode()) ;
            #endif
            fgRet = m_LscMgr.SetTBAToISP(); //preview table address will modify base on m_u4Idx
            if (fgRet != MTRUE) // update table address
            {
                MY_LOG(
                                "[prepare_Frameless_Shading] "
                                "Set table address fail %d \n"
                                ,fgRet
                             );	    	
            }
        }
         else
         {
                 MY_LOG(
                                "[prepare_Frameless_Shading] "
                                "m_LscMgr.isEnable()==0\n "
                             );	            
         }
	m_LscMgr.loadConfig(); // set NVRAM data to ispmgr_mt6575

        //shouldn't chage any parameter unless user change it in meta mode.
        //but 2nd pass is an exception, must disable lsc.
        if(m_IspCamInfo.eCamMode == ECamMode_Offline_Capture_Pass2)
        {
                MY_LOG(
                                "[prepare_Frameless_Shading] "
                                "ECamMode_Offline_Capture_Pass2 m_LscMgr.enableLscWoVariable(0) \n"
                             );	        
	    //fgRet = m_LscMgr.enableLsc(0);// disable shading        
	    fgRet = m_LscMgr.enableLscWoVariable(0);// disblae shading but reserve original setting
        }
#if 0        
        else
        {
            if((m_IspRegMgr.getIdx_Shading()!=2)&& (m_LscMgr.isEnable()==0))
            {
                fgRet = m_LscMgr.enableLsc(1);
                MY_LOG(
                	"[prepare_Frameless_Shading] "
                                "FORCE to EN LSC for factory mode 2nd capture"
                                "\n "                                
                             );	
                m_LscMgr.loadLut(); //load table   
                #if RAW_LSC_TBL_M4U_EN      
//                m_LscMgr.RawLscTblM4uFlushCurrTbl(m_LscMgr.getMode()) ;
                #endif
                fgRet = m_LscMgr.SetTBAToISP(); //preview table address will modify base on m_u4Idx
                if (fgRet != MTRUE) // update table address
                {
                    MY_LOG(
                    	"[prepare_Frameless_Shading] "
                    	"Set table address fail %d \n"
                    	,fgRet
                    	);	    	
                }                          
            }
        }
#endif        
        LOGD(
                      "[prepare_Frameless_Shading] "
                      "NVRAM data change"
                      " (CamMode,SensorMode, Operation_Mode) = (%d,%d,%d)\n"
                      "Lsc_mode, Lsc_idx (%d,%d) \n"
                      , m_IspCamInfo.eCamMode //   ECamMode_Video          = 0,     ECamMode_Online_Preview = ECamMode_Video,     ECamMode_Online_Capture, 
                      , getSensorMode()
                      , getOperMode()
                      , m_LscMgr.getMode()
                      , m_LscMgr.getIdx()
                      );            
    }
    else
    {
    if( (m_IspCamInfo.eCamMode == ECamMode_Offline_Capture_Pass2)
    ||  (m_IspCamInfo.eCamMode == ECamMode_HDR_Cap_Pass1_MF1)
    ||  (m_IspCamInfo.eCamMode == ECamMode_HDR_Cap_Pass2)
    ||	getOperMode()==EOperMode_PureRaw)
    {
	     m_LscMgr.setIdx(0);  // 
	                                   // capture must modify index base on  color temperature from AWB
	                                   // preview always load 3 color temperature table
	    fgRet = m_LscMgr.SetTBAToISP(); //preview table address will modify base on m_u4Idx
	    if (fgRet != MTRUE) // update table address
	    {
		      MY_LOG(
		                "[prepare_Frameless_Shading] "
		                "Set table address fail %d \n"
		                ,fgRet
		            );	    	
	    }
	    m_LscMgr.loadConfig(); // set NVRAM data to ispmgr_mt6575
                MY_LOG(
                                "[prepare_Frameless_Shading] "
                                "ECamMode_Offline_Capture_Pass2/HDR m_LscMgr.enableLsc(0) \n"
                             );	        	    
	    fgRet = m_LscMgr.enableLsc(0);// disable shading
//            fgRet = m_LscMgr.enableLscWoVariable(0);// disblae shading but reserve original setting
    }
    else
    {
           #if 0
           if (m_IspCamInfo.eIdx_Shading_CCT < eIDX_Shading_CCT_D65)
           {
	         m_LscMgr.setIdx(1);  // CWF and A Light both using 2nd talbe
	                                   // capture must modify index base on  color temperature from AWB
	                                   // preview always load 3 color temperature table
           }
           else
           {
	         m_LscMgr.setIdx(2);  // D65  using 3rd table
	                                   // capture must modify index base on  color temperature from AWB
	                                   // preview always load 3 color temperature table
           }
           #else
           m_LscMgr.setIdx(m_IspCamInfo.eIdx_Shading_CCT);
           #endif
	    m_LscMgr.loadLut(); //load table   	    
    #if RAW_LSC_TBL_M4U_EN      	    
//            m_LscMgr.RawLscTblM4uFlushCurrTbl(m_LscMgr.getMode()) ;
    #endif
	    fgRet = m_LscMgr.SetTBAToISP(); //preview table address will modify base on m_u4Idx
	    if (fgRet != MTRUE) // update table address
	    {
		      MY_LOG(
		                "[prepare_Frameless_Shading] "
		                "Set table address fail %d \n"
		                ,fgRet
		            );	    	
	    }
	    m_LscMgr.loadConfig(); // set NVRAM data to ispmgr_mt6575
                MY_LOG(
                                "[prepare_Frameless_Shading] "
                                "Others m_LscMgr.enableLsc(1) \n"
                             );	     	    
	    fgRet = m_LscMgr.enableLsc(1);// enable shading	    
    }

ISP_NVRAM_SHADING_T debug;
ISP_MGR_SHADING_T::getInstance().get(debug);
      MY_LOG(
                    "[prepare_Frameless_Shading] "
                "Shading param 0x%x,0x%x ,0x%x ,0x%x ,0x%x \n"
                , debug.shading_ctrl1.val
                , debug.shading_ctrl2.val
                , debug.shading_read_addr.val
                , debug.shading_last_blk.val
                , debug.shading_ratio_cfg.val
            );
    }
                                   
lbExit:

      MY_LOG(
                "[prepare_Frameless_Shading] "
                " (lsc_idx, lsc_mode, fgret) = (%d, %d, %d)\n"
                ,m_LscMgr.getIdx()
                ,m_LscMgr.getMode()
                , fgRet
            );

    return  fgRet;

lbExit_fail:
	
	fgRet = MFALSE;	
      MY_LOG(
                    "[prepare_Frameless_Shading] "
                "!!! WRONG !!! lu32ErrCode 0x%x \n"
                , lu32ErrCode
            );

	#if RAW_LSC_TBL_M4U_EN	
       m_LscMgr.RawLscTblM4uUnInit();
       #endif
	return fgRet;

    
}

