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
#define LOG_TAG "NSIspTuning::ParamctrlRAW"
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
#include "nvram_drv_mgr.h"
#include "nvram_drv.h"
#include "eeprom_drv.h"
#include "sensor_drv.h"
#include "camera_custom_msdk.h"
#include "camera_custom_eeprom.h"
#include "ShadingTblTransform.h"

//
using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ParamctrlRAW*
ParamctrlRAW::
createInstance(ESensorRole_T const eSensorRole)
{
    ParamctrlRAW* pParamctrlRAW = NULL;
    NVRAM_CAMERA_ISP_PARAM_STRUCT*  pNvram_Isp      = NULL;
    NVRAM_CAMERA_DEFECT_STRUCT*     pNvram_Defect   = NULL;
    NVRAM_CAMERA_SHADING_STRUCT*    pNvram_Shading  = NULL;
    NVRAM_CAMERA_SHADING_STRUCT    lNvram_Shading ;    
    NVRAM_CAMERA_ISP_PARAM_STRUCT  lNvram_Isp;

    EepromDrvBase *m_pEepromDrvObj = EepromDrvBase::createInstance();        
    SensorDrv*const pSensorDrv = SensorDrv::createInstance(SENSOR_MAIN|SENSOR_SUB);
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType=DUAL_CAMERA_SENSOR_MAX;
    GET_SENSOR_CALIBRATION_DATA_STRUCT GetSensorCalData;
    UINT32 m_u4SensorID= 0xFFFFFF;
    NvramDrvBase *m_pNvramDrvObj = NvramDrvBase::createInstance();
    UINT32 result=0,i=0;   
    UINT32 error=0;  
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT rSensorResolution;
    memset(&rSensorResolution, 0, sizeof(ACDK_SENSOR_RESOLUTION_INFO_STRUCT));
  
    if  ( MERR_OK != NvramDrvMgr::getInstance().init(ESensorType_RAW, eSensorRole) )
    {
        goto lbExit;
    }

    NvramDrvMgr::getInstance().getRefBuf(pNvram_Isp);
    NvramDrvMgr::getInstance().getRefBuf(pNvram_Defect);
    NvramDrvMgr::getInstance().getRefBuf(pNvram_Shading);

    GetSensorCalData.pCameraPara = pNvram_Isp;
    GetSensorCalData.pCameraShading = pNvram_Shading;
    GetSensorCalData.pCameraDefect = pNvram_Defect;

    if(eSensorRole==ESensorRole_Main)
    {
        a_eSensorType = DUAL_CAMERA_MAIN_SENSOR;
        m_u4SensorID = pSensorDrv->getMainSensorID();
    }
    else if(eSensorRole==ESensorRole_Sub)
    {    
        a_eSensorType = DUAL_CAMERA_SUB_SENSOR;     
        m_u4SensorID = pSensorDrv->getSubSensorID();
    }
    if(m_u4SensorID != pNvram_Shading->Shading.SensorId)
    {       
        MY_LOG("!!!!\n\n\n FAIL m_u4SensorID 0x%x != pNvram_Shading->Shading.SensorId 0x%x",m_u4SensorID,pNvram_Shading->Shading.SensorId);
        m_u4SensorID = pNvram_Shading->Shading.SensorId;
    }
    pSensorDrv->getResolution(&rSensorResolution);
    GetSensorCalData.pCameraPara->ISPComm.CommReg[EEPROM_INFO_IN_PV_WIDTH] = rSensorResolution.SensorPreviewWidth;
    GetSensorCalData.pCameraPara->ISPComm.CommReg[EEPROM_INFO_IN_PV_HEIGHT] = rSensorResolution.SensorPreviewHeight;
    GetSensorCalData.pCameraPara->ISPComm.CommReg[EEPROM_INFO_IN_CAP_WIDTH] = rSensorResolution.SensorFullWidth;
    GetSensorCalData.pCameraPara->ISPComm.CommReg[EEPROM_INFO_IN_CAP_HEIGHT] = rSensorResolution.SensorFullHeight;
    
    result |= m_pEepromDrvObj->GetEepromCalData((unsigned long)a_eSensorType, m_u4SensorID, CAMERA_EEPROM_DATA_SHADING_TABLE, (void *)&GetSensorCalData);
    result |= m_pEepromDrvObj->GetEepromCalData((unsigned long)a_eSensorType, m_u4SensorID, CAMERA_EEPROM_DATA_DEFECT_TABLE, (void *)&GetSensorCalData);

    if((!(result&EEPROM_ERR_NO_SHADING) ||!(result&EEPROM_ERR_NO_DEFECT)) && (!(result & EEPROM_ERR_NOT_WRITENVRAM)))
    {        
        if(pNvram_Isp->ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] != CAL_DATA_LOAD)
        {
            if (result&EEPROM_ERR_MTKOTP_SHADING)
            {
                MY_LOG("Run MTK 1-to-3");
                ParamctrlRAW::LscTblDump(m_u4SensorID, &pNvram_Shading->Shading.PreviewTable[0][0], &pNvram_Shading->Shading.CaptureTable[0][0], "lscOrg");
                ParamctrlRAW::DoLsc1to3(m_u4SensorID, pNvram_Isp, pNvram_Shading);
                ParamctrlRAW::LscTblDump(m_u4SensorID, &pNvram_Shading->Shading.PreviewTable[0][0], &pNvram_Shading->Shading.CaptureTable[0][0], "lsc123");
            }
            
            pNvram_Isp->ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] = CAL_DATA_LOAD;
            error = m_pNvramDrvObj->writeNvram(a_eSensorType, m_u4SensorID, CAMERA_NVRAM_DATA_ISP, (void *)pNvram_Isp, sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
            if(!(result&EEPROM_ERR_NO_SHADING))        
            {
                error = m_pNvramDrvObj->writeNvram(a_eSensorType, m_u4SensorID, CAMERA_NVRAM_DATA_SHADING, (void *)pNvram_Shading, sizeof(NVRAM_CAMERA_SHADING_STRUCT));
            }
            if(!(result&EEPROM_ERR_NO_DEFECT))        
            {
                error = m_pNvramDrvObj->writeNvram(a_eSensorType, m_u4SensorID, CAMERA_NVRAM_DATA_DEFECT, (void *)pNvram_Defect, sizeof(NVRAM_CAMERA_SHADING_STRUCT));
            }   
        }
        else
        {
            MY_LOG("BYPASS to NVRAM :  EEPROM has data but already save to NVRAM!");
        }
    }
    else
    {
        MY_LOG("No NEED To WRITENVRAM");       
    }
   
    if  ( ! pNvram_Isp || ! pNvram_Defect || ! pNvram_Shading )
    {
        MY_ERR(
            "[createInstance] "
            "(pNvram_Isp, pNvram_Defect, pNvram_Shading)=(%p, %p, %p)"
            , pNvram_Isp, pNvram_Defect, pNvram_Shading
        );
        goto lbExit;
    }

    //  Here, pIspParam must be legal.
    pParamctrlRAW = new ParamctrlRAW(
        eSensorRole, pNvram_Isp, pNvram_Defect, pNvram_Shading
    );

	MY_LOG("Check Sensor Shading type=%x", pNvram_Isp->ISPComm.CommReg[EEPROM_INFO_IN_COMM_SHADING_TYPE] );
	if (pNvram_Isp->ISPComm.CommReg[EEPROM_INFO_IN_COMM_SHADING_TYPE] == CAL_SHADING_TYPE_SENSOR)
	{
		MY_LOG("Set Sensor Shading");
		pSensorDrv->sendCommand(CMD_SENSOR_SET_CALIBRATION_DATA, (MUINT32*)(pNvram_Shading->Shading.SensorCalTable));
	}	

lbExit:
    NvramDrvMgr::getInstance().uninit();
    return  pParamctrlRAW;
}

void 
ParamctrlRAW::LscTblDump(MUINT32 u4SensorId, UINT32* pTblPv, UINT32* pTblCap, const char* filename)
{
    char strfile[128];
    FILE* fpdebug;

    MY_LOG("[LscTblDump] \n");

    sprintf(strfile, "/sdcard/lsc1to3data/0x%08x_%s.log", u4SensorId, filename);

    fpdebug = fopen(strfile, "w");
    
    if ( fpdebug == NULL )
    {
        MY_LOG("Can't open :%s\n", filename);
        return;
    } 

    UINT32 ct = 0;
    UINT32* Addr = pTblPv;
    fprintf(fpdebug, "Preview:\n");
    for (ct = 0; ct < 3; ct++)
    {
        UINT32* AddrEnd = (UINT32*) Addr + MAX_SHADING_Preview_SIZE;

        fprintf(fpdebug, "    {\n");
        while (Addr < AddrEnd)
        {
            UINT32 a, b, c, d;
            a = *Addr++;
            b = *Addr++;
            c = *Addr++;
            d = *Addr++;
            fprintf(fpdebug, "        0x%08x,0x%08x,0x%08x,0x%08x,\n", a, b, c, d);
        }
        fprintf(fpdebug, "    }, // ct%d\n", ct);
    }
    fprintf(fpdebug, "},\n");

    Addr = pTblCap;
    fprintf(fpdebug, "Capture:\n");
    for (ct = 0; ct < 3; ct++)
    {
        UINT32* AddrEnd = (UINT32*) Addr + MAX_SHADING_Capture_SIZE;

        fprintf(fpdebug, "    {\n");
        while (Addr < AddrEnd)
        {
            UINT32 a, b, c, d;
            a = *Addr++;
            b = *Addr++;
            c = *Addr++;
            d = *Addr++;
            fprintf(fpdebug, "        0x%08x,0x%08x,0x%08x,0x%08x,\n", a, b, c, d);
        }
        fprintf(fpdebug, "    }, // ct%d\n", ct);
    }
    fprintf(fpdebug, "},\n");

    fclose(fpdebug);
}

int
ParamctrlRAW::DoLsc1to3(MUINT32 u4SensorId, NVRAM_CAMERA_ISP_PARAM_STRUCT* pNvramIsp, NVRAM_CAMERA_SHADING_STRUCT* pNvramShading)
{
    SHADIND_ALIGN_CONF  align_conf;

    MINT32 i4SdblkXnum, i4SdblkYnum;
    MINT32 i4Width, i4Height, i4SdblkWidth, i4SdblkHeight, i4lWidth, i4lHeight;
    MUINT16 i;
    MUINT8 bBayer;

    EEPROM_DATA_STRUCT cal_data;

    MUINT32 retCal = GetCameraCalData(u4SensorId, (MUINT32*)&cal_data);
    if ((EEPROM_ERR_NO_SHADING & retCal) || (retCal == 0xff))
    {
        MY_LOG("[DoLsc1to3] u4SensorId(0x%08x) retCal(0x%08x) EEPROM_ERR_NO_SHADING\n", u4SensorId, retCal);
        return MFALSE;
    }
    else
    {
        MY_LOG("[DoLsc1to3] u4SensorId(0x%08x) retCal(0x%08x) EEPROM read OK\n", u4SensorId, retCal);
    }

    EEPROM_SHADING_STRUCT& rEepromShading = cal_data.Shading;   
    const EEPROM_SHADING_REG_STRUCT& CapReg = rEepromShading.CapRegister;
    
    i4SdblkWidth  = (CapReg.shading_ctrl2&0x0FFF0000) >> 16;    
    i4SdblkHeight = (CapReg.shading_ctrl2&0x0FFF);
    i4SdblkXnum   = (CapReg.shading_ctrl2&0xF0000000) >> 28;
    i4SdblkYnum   = (CapReg.shading_ctrl2&0x0000F000) >> 12;  
    i4lWidth      = (CapReg.shading_last_blk&0x0FFF0000) >> 16;
    i4lHeight     = (CapReg.shading_last_blk&0x0FFF);    
    i4Width       = i4SdblkXnum*i4SdblkWidth + i4lWidth;
    i4Height      = i4SdblkYnum*i4SdblkHeight + i4lHeight;
    bBayer        = rEepromShading.ColorOrder;

    
    MY_LOG("W(%d) H(%d) XNUM(%d) Y_NUM(%d) SDBLK_W(%d) SDBLK_H(%d) SDBLK_LW(%d) SDBLK_LH(%d)\n",
        i4Width, i4Height, i4SdblkXnum, i4SdblkYnum, i4SdblkWidth, i4SdblkHeight, i4lWidth, i4lHeight);
    MY_LOG("pEerom_data->ColorOrder = %d\n", bBayer); 

    char *gWorkinBuffer = new char[SHADIND_FUNC_WORKING_BUFFER_SIZE]; // 139,116 (int) = 556,464 (bytes)
    if (!gWorkinBuffer)
    {
        MY_LOG("[DoLsc1to3] gWorkinBuffer is NULL");
        return EEPROM_MEMORY_ALLOCATE_ERROR;
    }

    align_conf.working_buff_addr = (void*)gWorkinBuffer;
    align_conf.working_buff_size = SHADIND_FUNC_WORKING_BUFFER_SIZE;
            
    UINT32 u4GainTblSize = (i4SdblkXnum+2)*(i4SdblkYnum+2)*2;
    UINT32* pUnitTbl = new UINT32[u4GainTblSize];
    UINT32* pGoldenTbl = new UINT32[MAX_SHADING_Capture_SIZE]; //new UINT32[u4GainTblSize];
    UINT32* pBackupTbl[3];
    
    align_conf.cali.img_width   = i4Width*2;
    align_conf.cali.img_height  = i4Height*2;
    align_conf.cali.offset_x    = 0;
    align_conf.cali.offset_y    = 0;
    align_conf.cali.crop_width  = i4Width*2;
    align_conf.cali.crop_height = i4Height*2;
    align_conf.cali.bayer       = (BAYER_ID_T)BAYER_B;
    align_conf.cali.grid_x      = (int)i4SdblkXnum+2;
    align_conf.cali.grid_y      = (int)i4SdblkYnum+2;
    align_conf.cali.lwidth      = i4lWidth;
    align_conf.cali.lheight     = i4lHeight;
    align_conf.cali.ratio_idx   = 0;
    align_conf.cali.grgb_same   = SHADING_GRGB_SAME_NO;
    align_conf.cali.table       = (UINT32*)pUnitTbl;
    align_conf.cali.data_type   = SHADING_TYPE_GAIN;
    
    align_conf.golden.img_width   = i4Width*2;
    align_conf.golden.img_height  = i4Height*2;
    align_conf.golden.offset_x    = 0;
    align_conf.golden.offset_y    = 0;
    align_conf.golden.crop_width  = i4Width*2;
    align_conf.golden.crop_height = i4Height*2;
    align_conf.golden.bayer       = (BAYER_ID_T)BAYER_B;
    align_conf.golden.grid_x      = (int)i4SdblkXnum+2;
    align_conf.golden.grid_y      = (int)i4SdblkYnum+2;
    align_conf.golden.lwidth      = i4lWidth;
    align_conf.golden.lheight     = i4lHeight;
    align_conf.golden.ratio_idx   = 0;
    align_conf.golden.grgb_same   = SHADING_GRGB_SAME_NO;
    align_conf.golden.table       = (UINT32*)pGoldenTbl;
    align_conf.golden.data_type   = SHADING_TYPE_GAIN;

    memcpy(pUnitTbl, rEepromShading.SensorCalTable, u4GainTblSize*sizeof(UINT32));
    memcpy(pGoldenTbl, pNvramShading->Shading.SensorCalTable, u4GainTblSize*sizeof(UINT32));

    MY_LOG("[DoLsc1to3] Check Unit GainTbl\n");
    for (MUINT32 idx = 0; idx < u4GainTblSize; idx += 4)
    {
        MY_LOG("0x%08x    0x%08x    0x%08x    0x%08x\n", 
            *(pUnitTbl+idx), *(pUnitTbl+idx+1), *(pUnitTbl+idx+2), *(pUnitTbl+idx+3));
    }
    UINT8* pu8Tbl = reinterpret_cast<UINT8*>(&pUnitTbl[0]);
    for (MUINT32 idx = 0; idx < u4GainTblSize*4; idx += 16)
    {
        MY_LOG("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,\n",
            *(pu8Tbl+idx), *(pu8Tbl+idx+1), *(pu8Tbl+idx+2), *(pu8Tbl+idx+3),
            *(pu8Tbl+idx+4), *(pu8Tbl+idx+5), *(pu8Tbl+idx+6), *(pu8Tbl+idx+7),
            *(pu8Tbl+idx+8), *(pu8Tbl+idx+9), *(pu8Tbl+idx+10), *(pu8Tbl+idx+11),
            *(pu8Tbl+idx+12), *(pu8Tbl+idx+13), *(pu8Tbl+idx+14), *(pu8Tbl+idx+15));
    }
    MY_LOG("[DoLsc1to3] Check Golden GainTbl\n");
    for (MUINT32 idx = 0; idx < u4GainTblSize; idx += 4)
    {
        MY_LOG("0x%08x    0x%08x    0x%08x    0x%08x\n",
            *(pGoldenTbl+idx), *(pGoldenTbl+idx+1), *(pGoldenTbl+idx+2), *(pGoldenTbl+idx+3));
    }
    pu8Tbl = reinterpret_cast<UINT8*>(&pGoldenTbl[0]);
    for (MUINT32 idx = 0; idx < u4GainTblSize*4; idx += 16)
    {
        MY_LOG("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,\n",
            *(pu8Tbl+idx), *(pu8Tbl+idx+1), *(pu8Tbl+idx+2), *(pu8Tbl+idx+3),
            *(pu8Tbl+idx+4), *(pu8Tbl+idx+5), *(pu8Tbl+idx+6), *(pu8Tbl+idx+7),
            *(pu8Tbl+idx+8), *(pu8Tbl+idx+9), *(pu8Tbl+idx+10), *(pu8Tbl+idx+11),
            *(pu8Tbl+idx+12), *(pu8Tbl+idx+13), *(pu8Tbl+idx+14), *(pu8Tbl+idx+15));
    }

    // for capture
    i4SdblkWidth  = pNvramIsp->ISPRegs.Shading[1].shading_ctrl2.bits.SDBLK_WIDTH;
    i4SdblkHeight = pNvramIsp->ISPRegs.Shading[1].shading_ctrl2.bits.SDBLK_HEIGHT;
    i4SdblkXnum   = pNvramIsp->ISPRegs.Shading[1].shading_ctrl2.bits.SDBLK_XNUM;
    i4SdblkYnum   = pNvramIsp->ISPRegs.Shading[1].shading_ctrl2.bits.SDBLK_YNUM;
    i4lWidth      = pNvramIsp->ISPRegs.Shading[1].shading_last_blk.bits.SDBLK_LWIDTH;
    i4lHeight     = pNvramIsp->ISPRegs.Shading[1].shading_last_blk.bits.SDBLK_LHEIGHT;
    i4Width       = i4SdblkXnum*i4SdblkWidth + i4lWidth;
    i4Height      = i4SdblkYnum*i4SdblkHeight + i4lHeight;

    MY_LOG("W(%d) H(%d) XNUM(%d) Y_NUM(%d) SDBLK_W(%d) SDBLK_H(%d) SDBLK_LW(%d) SDBLK_LH(%d)\n",
        i4Width, i4Height, i4SdblkXnum, i4SdblkYnum, i4SdblkWidth, i4SdblkHeight, i4lWidth, i4lHeight);

    align_conf.input.img_width   = i4Width*2;
    align_conf.input.img_height  = i4Height*2;
    align_conf.input.offset_x    = 0;
    align_conf.input.offset_y    = 0;
    align_conf.input.crop_width  = i4Width*2;
    align_conf.input.crop_height = i4Height*2;
    align_conf.input.bayer       = BAYER_B;
    align_conf.input.grid_x      = (int)i4SdblkXnum+2;
    align_conf.input.grid_y      = (int)i4SdblkYnum+2;
    align_conf.input.lwidth      = i4lWidth;
    align_conf.input.lheight     = i4lHeight;
    align_conf.input.ratio_idx   = 0;
    align_conf.input.grgb_same   = SHADING_GRGB_SAME_NO;
    align_conf.input.data_type   = SHADING_TYPE_COEFF;
    
    align_conf.output.img_width   = i4Width*2;
    align_conf.output.img_height  = i4Height*2;
    align_conf.output.offset_x    = 0;
    align_conf.output.offset_y    = 0;
    align_conf.output.crop_width  = i4Width*2;
    align_conf.output.crop_height = i4Height*2;
    align_conf.output.bayer       = BAYER_B;
    align_conf.output.grid_x      = (int)i4SdblkXnum+2;
    align_conf.output.grid_y      = (int)i4SdblkYnum+2;
    align_conf.output.lwidth      = i4lWidth;
    align_conf.output.lheight     = i4lHeight;
    align_conf.output.ratio_idx   = 0;
    align_conf.output.grgb_same   = SHADING_GRGB_SAME_NO;
    align_conf.output.data_type   = SHADING_TYPE_COEFF;

    for (i = 0; i < 3; i++)
    {
        memcpy(pUnitTbl, rEepromShading.SensorCalTable, u4GainTblSize*sizeof(UINT32));
        memcpy(pGoldenTbl, pNvramShading->Shading.SensorCalTable, u4GainTblSize*sizeof(UINT32));
        align_conf.input.table = (UINT32*)pNvramShading->Shading.CaptureTable[i];
        align_conf.output.table = (UINT32*)pNvramShading->Shading.CaptureTable[i];
        pBackupTbl[i] = new UINT32[MAX_SHADING_Capture_SIZE]; 
        memcpy(pBackupTbl[i] , pNvramShading->Shading.CaptureTable[i], MAX_SHADING_Capture_SIZE*sizeof(UINT32));
        

        LSC_RESULT ret = S_LSC_CONVERT_OK;
        ret = shading_align_golden(align_conf);

        if (S_LSC_CONVERT_OK != ret)
        {
            MY_LOG("[DoLsc1to3] shading_align_golden error, ct_idx(%d)\n", i);
            for(int loop=0; loop<=i; loop++)
            {
                memcpy(pNvramShading->Shading.CaptureTable[loop],pBackupTbl[loop] , MAX_SHADING_Capture_SIZE*sizeof(UINT32));
                delete [] pBackupTbl[loop];
            }            
            return MFALSE;
        } 
        else
        {
            MY_LOG("[DoLsc1to3] align ct_idx %d done!", i);
        }
    }

    // for preview
    i4SdblkWidth  = pNvramIsp->ISPRegs.Shading[0].shading_ctrl2.bits.SDBLK_WIDTH;
    i4SdblkHeight = pNvramIsp->ISPRegs.Shading[0].shading_ctrl2.bits.SDBLK_HEIGHT;
    i4SdblkXnum   = pNvramIsp->ISPRegs.Shading[0].shading_ctrl2.bits.SDBLK_XNUM;
    i4SdblkYnum   = pNvramIsp->ISPRegs.Shading[0].shading_ctrl2.bits.SDBLK_YNUM;
    i4lWidth      = pNvramIsp->ISPRegs.Shading[0].shading_last_blk.bits.SDBLK_LWIDTH;
    i4lHeight     = pNvramIsp->ISPRegs.Shading[0].shading_last_blk.bits.SDBLK_LHEIGHT;
    i4Width       = i4SdblkXnum*i4SdblkWidth + i4lWidth;
    i4Height      = i4SdblkYnum*i4SdblkHeight + i4lHeight;

    MY_LOG("W(%d) H(%d) XNUM(%d) Y_NUM(%d) SDBLK_W(%d) SDBLK_H(%d) SDBLK_LW(%d) SDBLK_LH(%d)\n",
        i4Width, i4Height, i4SdblkXnum, i4SdblkYnum, i4SdblkWidth, i4SdblkHeight, i4lWidth, i4lHeight);

#if 0
    // use preview cct and align to golden
    align_conf.input.img_width   = i4Width*2;
    align_conf.input.img_height  = i4Height*2;
    align_conf.input.offset_x    = 0;
    align_conf.input.offset_y    = 0;
    align_conf.input.crop_width  = i4Width*2;
    align_conf.input.crop_height = i4Height*2;
    align_conf.input.bayer       = BAYER_B;
    align_conf.input.grid_x      = (int)i4SdblkXnum+2;
    align_conf.input.grid_y      = (int)i4SdblkYnum+2;
    align_conf.input.lwidth      = i4lWidth;
    align_conf.input.lheight     = i4lHeight;
    align_conf.input.ratio_idx   = 0;
    align_conf.input.grgb_same   = SHADING_GRGB_SAME_NO;
    align_conf.input.data_type   = SHADING_TYPE_COEFF;

    align_conf.output.img_width   = i4Width*2;
    align_conf.output.img_height  = i4Height*2;
    align_conf.output.offset_x    = 0;
    align_conf.output.offset_y    = 0;
    align_conf.output.crop_width  = i4Width*2;
    align_conf.output.crop_height = i4Height*2;
    align_conf.output.bayer       = BAYER_B;
    align_conf.output.grid_x      = (int)i4SdblkXnum+2;
    align_conf.output.grid_y      = (int)i4SdblkYnum+2;
    align_conf.output.lwidth      = i4lWidth;
    align_conf.output.lheight     = i4lHeight;
    align_conf.output.ratio_idx   = 0;
    align_conf.output.grgb_same   = SHADING_GRGB_SAME_NO;
    align_conf.output.data_type   = SHADING_TYPE_COEFF;
    for (i = 0; i < 3; i++)
    {
        memcpy(pUnitTbl, rEepromShading.SensorCalTable, u4GainTblSize*sizeof(UINT32));
        memcpy(pGoldenTbl, pNvramShading->Shading.SensorCalTable, u4GainTblSize*sizeof(UINT32));
        align_conf.input.table = (UINT32*)pNvramShading->Shading.PreviewTable[i];
        align_conf.output.table = (UINT32*)pNvramShading->Shading.PreviewTable[i];

        LSC_RESULT ret = S_LSC_CONVERT_OK;
        ret = shading_align_golden(align_conf);

        if (S_LSC_CONVERT_OK != ret)
        {
            MY_LOG("[DoLsc1to3] shading_align_golden error, ct_idx(%d)\n", i);
        } 
        else
        {
            MY_LOG("[DoLsc1to3] align ct_idx %d done!", i);
        }
    }    
#else
    // use capture 1 to 3 and transform to preview
    SHADIND_TRFM_CONF trfm;
    trfm.working_buff_addr = gWorkinBuffer;
    trfm.working_buff_size = SHADIND_FUNC_WORKING_BUFFER_SIZE;
    trfm.afn = SHADING_AFN_R0D;

    trfm.input = align_conf.input;
    trfm.output.img_width   = i4Width*2;
    trfm.output.img_height  = i4Height*2;
    trfm.output.offset_x    = 0;
    trfm.output.offset_y    = 0;
    trfm.output.crop_width  = i4Width*2;
    trfm.output.crop_height = i4Height*2;
    trfm.output.bayer       = BAYER_B;
    trfm.output.grid_x      = (int)i4SdblkXnum+2;
    trfm.output.grid_y      = (int)i4SdblkYnum+2;
    trfm.output.lwidth      = i4lWidth;
    trfm.output.lheight     = i4lHeight;
    trfm.output.ratio_idx   = 0;
    trfm.output.grgb_same   = SHADING_GRGB_SAME_NO;
    trfm.output.data_type   = SHADING_TYPE_COEFF;

    for (i = 0; i < 3; i++)
    {
        memcpy(pGoldenTbl, pNvramShading->Shading.CaptureTable[i], MAX_SHADING_Capture_SIZE*sizeof(UINT32));
        trfm.input.table = pGoldenTbl;
        trfm.output.table = (UINT32*)pNvramShading->Shading.PreviewTable[i];

        
        memset(pBackupTbl[i],0x0,MAX_SHADING_Capture_SIZE*sizeof(UINT32));
        memcpy(pBackupTbl[i] , pNvramShading->Shading.PreviewTable[i], MAX_SHADING_Capture_SIZE*sizeof(UINT32));

        LSC_RESULT ret = S_LSC_CONVERT_OK;
        ret = shading_transform(trfm);
        
        if (S_LSC_CONVERT_OK != ret)
        {
            MY_LOG("[DoLsc1to3] shading_transform error, ct_idx(%d)\n", i);
            for(int loop=0; loop<=i; loop++)
            {
                memcpy(pNvramShading->Shading.PreviewTable[loop],pBackupTbl[loop] , MAX_SHADING_Capture_SIZE*sizeof(UINT32));
                delete [] pBackupTbl[loop];
            }            
            return MFALSE;
        } 
        else
        {
            MY_LOG("[DoLsc1to3] align ct_idx %d done!", i);
        }
    }
#endif

    for(int loop=0; loop<3; loop++)
    {
        delete [] pBackupTbl[loop];
    }

    
    delete [] pUnitTbl;
    delete [] pGoldenTbl;
    delete [] gWorkinBuffer;

    return EEPROM_NO_ERROR;
}


void
ParamctrlRAW::
destroyInstance()
{
    delete this;
}


ParamctrlRAW::
ParamctrlRAW(
    ESensorRole_T const eSensorRole, 
    NVRAM_CAMERA_ISP_PARAM_STRUCT*const pNvram_Isp, 
    NVRAM_CAMERA_DEFECT_STRUCT*const    pNvram_Defect, 
    NVRAM_CAMERA_SHADING_STRUCT*const   pNvram_Shading
)
    : Parent_t(eSensorRole, &m_IspCamInfo)
    //
    , m_stIspDebugInfo()
    //
    , m_IspCamInfo()
    , m_pIspTuningCustom(NULL)
    , m_SysramMgr()
    //
    //  ISP Tuning Parameters.
    , m_rIspParam(*pNvram_Isp)
    ////ISP Common Parameters.
    , m_rIspComm(m_rIspParam.ISPComm)
    ////ISP Register Parameters.
    , m_IspRegMgr(&m_rIspParam.ISPRegs)
    ////ISP PCA Parameters.
    , m_pIPcaMgr(NULL)
    //
    //  Defect Parameters.
    , m_rDefectParam(pNvram_Defect->Defect)
    //
    //  Shading Parameters.
    //, m_rShadingParam(pNvram_Shading->Shading)
    ,m_LscMgr(&m_rIspParam.ISPRegs, pNvram_Shading->Shading)
{
}


ParamctrlRAW::
~ParamctrlRAW()
{
}


MERROR_ENUM
ParamctrlRAW::
construct()
{
    MERROR_ENUM err = MERR_UNKNOWN;

    //  (1) Invoke parent's construct().
    err = Parent_t::construct();
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

    err = MERR_OK;

lbExit:
    MY_LOG("[-construct]err(%X)", err);
    return  err;
}


MERROR_ENUM
ParamctrlRAW::
do_init()
{
    MERROR_ENUM err = MERR_UNKNOWN;

    //  (1) Create custom instance.
    m_pIspTuningCustom = IspTuningCustom::createInstance(getSensorRole());
    if  ( ! m_pIspTuningCustom )
    {
        MY_ERR("[init][m_pIspTuningCustom==NULL](eSensorRole=%d)", getSensorRole());
        err = MERR_CUSTOM_NOT_READY;
        goto lbExit;
    }

    //  (3) Init sysram manager.
    err = m_SysramMgr.init();
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

    err = MERR_OK;

lbExit:
    MY_LOG("[-ParamctrlRAW::do_init]err(%X)", err);
    return  err;
}


MERROR_ENUM
ParamctrlRAW::
uninit()
{
    MY_LOG("[+uninit]");

    //  (1) Uninit sysram manager.
    m_SysramMgr.uninit();

    //  (2) Custom
    if  ( m_pIspTuningCustom )
    {
        m_pIspTuningCustom->destroyInstance();
        m_pIspTuningCustom = NULL;
    }

    return  Parent_t::uninit();
}

