/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   Sensor.c
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   Image sensor driver function
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <asm/system.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "ov8835mipiraw_Sensor_Semco_Kantatsu.h"
#include "ov8835mipiraw_Camera_Sensor_para_Semco_Kantatsu.h"
#include "ov8835mipiraw_CameraCustomized_Semco_Kantatsu.h"

#ifdef OV8835MIPI_Semco_Kantatsu_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif
//#define ACDK
extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);

//#define OV8835MIPI_Semco_Kantatsu_PREVIEW_CLK 138666667  //65000000
//#define OV8835MIPI_Semco_Kantatsu_CAPTURE_CLK 134333333  //117000000  //69333333
//#define OV8835MIPI_Semco_Kantatsu_DEBUG_SETTING
#ifdef OV8835MIPI_Semco_Kantatsu_DEBUG_SETTING
#define OV8835MIPI_Semco_Kantatsu_PREVIEW_CLK 138666667  //65000000
#define OV8835MIPI_Semco_Kantatsu_CAPTURE_CLK 134333333  //117000000  //69333333
#define OV8835MIPI_Semco_Kantatsu_ZSD_PRE_CLK 134333333 //117000000  
#else
#define OV8835MIPI_Semco_Kantatsu_PREVIEW_CLK 143000000  //65000000
#define OV8835MIPI_Semco_Kantatsu_CAPTURE_CLK 143000000  //117000000  //69333333
#define OV8835MIPI_Semco_Kantatsu_ZSD_PRE_CLK 143000000 //117000000
#endif


#define OV8835MIPI_SEMCO_KANTATSU_SENSOR_ID			  	0x8830
//#define OV8835MIPI_Semco_Kantatsu_ZSD_PRE_CLK 134333333 //117000000

//<2012/11/28 alberthsiao-17860,camera quality tuning for Hawk40
kal_int16 preview_flag_semco_kantatsu=0;
unsigned long OTP_DelayTime_semco_kantatsu=10;  // spec. is 10sec,3sec is work,test by norman
kal_uint16 OTP_index_semco_kantatsu=6;//used=0~3,no_calibrate=5,init=6
kal_int16 OTP_AWB_flag_semco_kantatsu=false;

MSDK_SCENARIO_ID_ENUM OV8835MIPIRAW_Semco_Kantatsu_CurrentScenarioId = ACDK_SCENARIO_ID_CAMERA_PREVIEW;
static OV8835MIPI_Semco_Kantatsu_sensor_struct OV8835MIPI_Semco_Kantatsu_sensor =
{
  .eng =
  {
    .reg = CAMERA_SENSOR_REG_DEFAULT_VALUE,
    .cct = CAMERA_SENSOR_CCT_DEFAULT_VALUE,
  },
  .eng_info =
  {
    .SensorId = 128,
    .SensorType = CMOS_SENSOR,
    .SensorOutputDataFormat = OV8835MIPI_Semco_Kantatsu_COLOR_FORMAT,
  },
  .shutter = 0x20,  
  .gain = 0x20,
  .pv_pclk = OV8835MIPI_Semco_Kantatsu_PREVIEW_CLK,
  .cap_pclk = OV8835MIPI_Semco_Kantatsu_CAPTURE_CLK,
  .frame_height = OV8835MIPI_Semco_Kantatsu_PV_PERIOD_LINE_NUMS,
  .line_length = OV8835MIPI_Semco_Kantatsu_PV_PERIOD_PIXEL_NUMS,
  .is_zsd = KAL_FALSE, //for zsd
  .dummy_pixels = 0,
  .dummy_lines = 0,  //for zsd
  .is_autofliker = KAL_FALSE,
};


static DEFINE_SPINLOCK(ov8835mipi_semco_kantatsudrv_lock);

kal_uint16 OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte=0;
    char puSendCmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
	iReadRegI2C(puSendCmd , 2, (u8*)&get_byte,1,OV8835MIPI_Semco_Kantatsu_sensor.write_id);
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
	//SENSORDB("OV8835MIPI_Semco_Kantatsu_read_cmos_sensor, addr:%x;get_byte:%x \n",addr,get_byte);
#endif		
    return get_byte;
}


kal_uint16 OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
    //kal_uint16 reg_tmp;
	
    char puSendCmd[3] = {(char)(addr >> 8) , (char)(addr & 0xFF) ,(char)(para & 0xFF)};
	
	iWriteRegI2C(puSendCmd , 3,OV8835MIPI_Semco_Kantatsu_sensor.write_id);

	//SENSORDB("OV8835MIPI_Semco_Kantatsu_write_cmos_sensor, addr:%x;get_byte:%x \n",addr,para);

	//for(i=0;i<0x100;i++)
	//	{
			
	//	}
	
	//reg_tmp = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(addr);

	//SENSORDB("OV8835MIPI_Semco_Kantatsu_read_cmos_sensor, addr:%x;get_byte:%x \n",addr,reg_tmp);
	return 0;
}

//MEIMEI ADD OTP 10-8
#if defined(OV8835MIPI_Semco_Kantatsu_USE_OTP)
//index:index of otp group.(0,1,2,3)
//return:	0:group index is empty.
//		1.group index has invalid data
//		2.group index has valid data

kal_uint16 ov8835mipi_semco_kantatsucheck_otp_flag(kal_uint16 index)
{
    kal_uint16 temp,flag;
    kal_uint32 address;

    //select bank 
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc0+1+index*7);
    //load otp to buffer
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
    msleep(OTP_DelayTime_semco_kantatsu);

    //disable otp read
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

    //read flag
    address = 0x3d00;
    flag = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    //printk("meimei_OV8835MIPI_Semco_Kantatsu_check_otp_flag=%d\r\n",flag);

    if(NULL == flag)
    {
        //SENSORDB("[OV8835MIPI_Semco_Kantatsu_check_otp]index[%x]read flag[%x][0]\n",index,flag);
        return 0;
    }
    else if((!(flag&0x80)) && (flag&0x7f))
    {
        //SENSORDB("[OV8835MIPI_Semco_Kantatsu_check_otp]index[%x]read flag[%x][1]\n",index,flag);
        return 1;
    }
    return 0;
}


//index:index of otp group.(0,1,2)
//return:	0.group index is empty.
//		1.group index has valid data
static void  ov8835mipi_semco_kantatsucheck_otp_CRC(kal_uint16 index)
{
    kal_uint16 CRC0,CRC1;
    kal_uint32 CRC,address;

    //select bank 
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc0+7+index*7);
    //load otp to buffer
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
    msleep(OTP_DelayTime_semco_kantatsu);
    //disable otp read
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);
    
    address = 0x3d0e;
    CRC0 = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    address++;
    CRC1 = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);

    CRC = CRC0 | (CRC1<<8);

    SENSORDB("[OV8835MIPI_Semco_Kantatsu_read_otp_CRC]index[%x]CRC[%x]\n",index,CRC);
}

//index:index of otp group.(0,1,2,3)
//return value:	0 check success
//				1 no 	OTP
kal_uint16 ov8835mipi_semco_kantatsucheck_otp_Index(void)
{
    kal_int16 temp,i;

    if(OTP_index_semco_kantatsu==6)
    {
        for(i = 3; i >= 0; i--)
        {
            temp = ov8835mipi_semco_kantatsucheck_otp_flag(i);
            if(1 == temp)
            {
                OTP_index_semco_kantatsu = (kal_uint16)i;
                return 0;
            }
        }
        if( temp == 0)
        {
            //SENSORDB("[ov8835mipi_semco_kantatsucheck_otp_Index]no valid wb OTP data!\r\n");
            OTP_index_semco_kantatsu=5;
        }
        return 1;
    }
    if(OTP_index_semco_kantatsu==5)
        return 1;
    else
        return 0;
}

//index:index of otp group.(0,1,2)
//return: 0
kal_uint16 ov8835mipi_semco_kantatsuread_otp_wb(kal_uint16 index, struct OV8835MIPI_Semco_Kantatsu_otp_struct *otp)
{
    kal_uint16 temp;
    kal_uint32 address;

    //select bank 
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc0+1+index*7);
    //load otp to buffer
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
    msleep(OTP_DelayTime_semco_kantatsu);
    //disable otp read
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);
    
    address = 0x3d09;
    otp->lens_id = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    //SENSORDB("[OV8835MIPI_Semco_Kantatsu_read_otp_wb]address[%x]lens_id[%x]\n",address,otp->lens_id);
    
    address++;
    otp->module_integrator_id = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    //SENSORDB("[OV8835MIPI_Semco_Kantatsu_read_otp_wb]address[%x]module_integrator_id[%x]\n",address,otp->module_integrator_id);

    //select bank 
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc0+3+index*7);
    //load otp to buffer
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
    msleep(OTP_DelayTime_semco_kantatsu);
    //disable otp read
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);
    
    address = 0x3d00;
    otp->Unit_AWB_R  = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    //SENSORDB("[OV8835MIPI_Semco_Kantatsu_read_otp_wb]address[%x]Unit_AWB_R[%x]\n",address,otp->Unit_AWB_R);

    address++;
    otp->Unit_AWB_Gr  = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    //SENSORDB("[OV8835MIPI_Semco_Kantatsu_read_otp_wb]address[%x]Unit_AWB_Gr[%x]\n",address,otp->Unit_AWB_Gr);

    address++;
    otp->Unit_AWB_Gb  = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    //SENSORDB("[OV8835MIPI_Semco_Kantatsu_read_otp_wb]address[%x]Unit_AWB_Gb[%x]\n",address,otp->Unit_AWB_Gb);

    address++;
    otp->Unit_AWB_B  = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    //SENSORDB("[OV8835MIPI_Semco_Kantatsu_read_otp_wb]address[%x]>Unit_AWB_[%x]\n",address,otp->Unit_AWB_B);

    address++;
    otp->Gloden_AWB_R  = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    //SENSORDB("[OV8835MIPI_Semco_Kantatsu_read_otp_wb]address[%x]Gloden_AWB_R][%x]\n",address,otp->Gloden_AWB_R);

    address++;
    otp->Gloden_AWB_Gr  = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    //SENSORDB("[OV8835MIPI_Semco_Kantatsu_read_otp_wb]address[%x]Gloden_AWB_Gr][%x]\n",address,otp->Gloden_AWB_Gr);

    address++;
    otp->Gloden_AWB_Gb  = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    //SENSORDB("[OV8835MIPI_Semco_Kantatsu_read_otp_wb]address[%x]Gloden_AWB_Gb[%x]\n",address,otp->Gloden_AWB_Gb);

    address++;
    otp->Gloden_AWB_B  = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    //SENSORDB("[OV8835MIPI_Semco_Kantatsu_read_otp_wb]address[%x]Gloden_AWB_B[%x]\n",address,otp->Gloden_AWB_B);
    SENSORDB("ov8835mipi_semco_kantatsuread_otp_wb Unit(%d,%d,%d,%d) Gloden(%d,%d,%d,%d)\r\n",otp->Unit_AWB_R,otp->Unit_AWB_Gr,otp->Unit_AWB_Gb,otp->Unit_AWB_B,
        otp->Gloden_AWB_R,otp->Gloden_AWB_Gr,otp->Gloden_AWB_Gb,otp->Gloden_AWB_B);
    if((otp->Unit_AWB_R==0)||(otp->Unit_AWB_Gr==0)||(otp->Unit_AWB_Gb==0)||(otp->Unit_AWB_B==0)||
        (otp->Gloden_AWB_R==0)||(otp->Gloden_AWB_Gr==0)||(otp->Gloden_AWB_Gb==0)||(otp->Gloden_AWB_B==0))
        return 1;
    else
        return 0;
}

kal_uint16 ov8835mipi_semco_kantatsuread_otp_lenc(kal_uint16 index,struct OV8835MIPI_Semco_Kantatsu_otp_struct *otp)
{
    kal_uint16 bank,temp1,temp2,i;
    kal_uint32 address;

    //select bank 
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc0+4+index*7);
    //load otp to buffer
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
    msleep(OTP_DelayTime_semco_kantatsu);
    //disable otp read
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);
    //read lenc
    address = 0x3d00;
    for(i = 0; i < 16; i++)
    {	
        otp->lenc[i] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+i);
        //SENSORDB("[OV8835MIPI_Semco_Kantatsu_read_otp_lenc]address[%x]otp->lenc[%d][%x]\n",address+i,i,otp->lenc[i]);
    }

    //select bank 
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc0+5+index*7);
    //load otp to buffer
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
    msleep(OTP_DelayTime_semco_kantatsu);
    //disable otp read
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);
    //read lenc
    address = 0x3d00;
    for(i = 0; i < 16; i++)
    {	
        otp->lenc[16+i] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+i);
        //SENSORDB("[OV8835MIPI_Semco_Kantatsu_read_otp_lenc]address[%x]otp->lenc[%d][%x]\n",address+i,16+i,otp->lenc[16+i]);
    }

    //select bank 
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc0+6+index*7);
    //load otp to buffer
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
    msleep(OTP_DelayTime_semco_kantatsu);
    //disable otp read
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);
    //read lenc
    address = 0x3d00;
    for(i = 0; i < 16; i++)
    {	
        otp->lenc[32+i] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+i);
        //SENSORDB("[OV8835MIPI_Semco_Kantatsu_read_otp_lenc]address[%x]otp->lenc[%d][%x]\n",address+i,32+i,otp->lenc[32+i]);
    }

    //select bank 
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc0+7+index*7);
    //load otp to buffer
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
    msleep(OTP_DelayTime_semco_kantatsu);
    //disable otp read
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);
    //read lenc
    address = 0x3d00;
    for(i = 0; i < 14; i++)
    {	
        otp->lenc[48+i] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+i);
        //SENSORDB("[OV8835MIPI_Semco_Kantatsu_read_otp_lenc]address[%x]otp->lenc[%d][%x]\n",address+i,48+i,otp->lenc[48+i]);
    }
    
    return 0;
}
kal_uint16 ov8835mipi_semco_kantatsuread_otp_blc(struct OV8835MIPI_Semco_Kantatsu_otp_struct *otp)
{
    kal_uint16 temp,i;
    kal_uint32 address;

    //select bank 0
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc0+31);
    //load otp to buffer
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
    msleep(OTP_DelayTime_semco_kantatsu);

    //disable otp read
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

    address = 0x3d0a;
    otp->user_data[0] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);

    SENSORDB("[ov8835mipi_semco_kantatsuread_otp_blc]address[%x]user_data[0][%x]\n",address,otp->user_data[0]);

    //disable otp read
    //OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

    return 0;
}

kal_uint16 ov8835mipi_semco_kantatsuread_otp_af(kal_uint16 index,struct OV8835MIPI_Semco_Kantatsu_otp_struct *otp)
{
    kal_uint16 bank,temp1,temp2,i;
    kal_uint32 address;

    //select bank 
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc0+2+index*7);
    //load otp to buffer
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
    msleep(OTP_DelayTime_semco_kantatsu);
    //disable otp read
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);
    //read lenc
    
    address = 0x3d0c;
    otp->user_data[1] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    address++;
    otp->user_data[1] = (otp->user_data[1]<<8) | OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    SENSORDB("[ov8835mipi_semco_kantatsuread_otp_af]otp->user_data[1][%x]\n",otp->user_data[1]);

    address++;
    otp->user_data[2] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    address++;
    otp->user_data[2] = (otp->user_data[2]<<8) | OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    SENSORDB("[ov8835mipi_semco_kantatsuread_otp_af]otp->user_data[2][%x]\n",otp->user_data[2]);
    return 0;
}

//R_gain: red gain of sensor AWB, 0x400 = 1
//G_gain: green gain of sensor AWB, 0x400 = 1
//B_gain: blue gain of sensor AWB, 0x400 = 1
//reutrn 0
kal_uint16 ov8835mipi_semco_kantatsuupdate_wb_gain(kal_uint32 R_gain, kal_uint32 G_gain, kal_uint32 B_gain)
{
    //SENSORDB("[ov8835mipi_semco_kantatsuupdate_wb_gain]R_gain[%x]G_gain[%x]B_gain[%x]\n",R_gain,G_gain,B_gain);

    if(R_gain > 0x400)
    {
        OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3400,R_gain >> 8);
        OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3401,(R_gain&0x00ff));
    }
    if(G_gain > 0x400)
    {
        OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3402,G_gain >> 8);
        OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3403,(G_gain&0x00ff));
    }
    if(B_gain >0x400)
    {
        OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3404,B_gain >> 8);
        OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3405,(B_gain&0x00ff));
    }
    return 0;
}


//return 0
kal_uint16 ov8835mipi_semco_kantatsuupdate_lenc(struct OV8835MIPI_Semco_Kantatsu_otp_struct *otp)
{
    kal_uint16 i, temp;

    for(i = 0; i < 62; i++)
    {
        OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5800+i,otp->lenc[i]);
        //SENSORDB("[OV8835MIPI_Semco_Kantatsu_update_lenc]otp->lenc[%x][%x]\n",0x5800+i,OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x5800+i));
    }
    return 0;
}

kal_uint16 ov8835mipi_semco_kantatsuupdate_blc(struct OV8835MIPI_Semco_Kantatsu_otp_struct *otp)
{
        OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4006,otp->user_data[0]);

        //SENSORDB("[OV8835MIPI_Semco_Kantatsu_update_blc]otp->blc[%d][%x]\n",otp->user_data[0]);
        return 0;
}

//R/G and B/G ratio of typical camera module is defined here

kal_uint32 RG_Ratio_typical_semco_kantatsu;
kal_uint32 BG_Ratio_typical_semco_kantatsu ;


//call this function after OV8835MIPI initialization
//return value:	0 update success
//				1 no 	OTP

kal_uint16 ov8835mipi_semco_kantatsuupdate_wb_register_from_otp(void)
{
    kal_uint16 temp, i;
    struct OV8835MIPI_Semco_Kantatsu_otp_struct current_otp;
    kal_uint32 R_gain, B_gain, G_gain, G_gain_R,G_gain_B;
    kal_uint32 rg_ratio,bg_ratio;

    //SENSORDB("OV8835MIPI_Semco_Kantatsu_update_wb_register_from_otp\n");
    //printk("meimei_OV8835MIPI_Semco_Kantatsu_update_wb_register_from_otp");

    //update white balance setting from OTP
    //check first wb OTP with valid OTP

    if(ov8835mipi_semco_kantatsucheck_otp_Index())    return 1;

    if(ov8835mipi_semco_kantatsuread_otp_wb(OTP_index_semco_kantatsu,&current_otp))    return 1;
    	
    RG_Ratio_typical_semco_kantatsu = (1000*current_otp.Gloden_AWB_R)/current_otp.Gloden_AWB_Gr;
    BG_Ratio_typical_semco_kantatsu = (1000*current_otp.Gloden_AWB_B)/current_otp.Gloden_AWB_Gr;
    rg_ratio= (1000*current_otp.Unit_AWB_R)/current_otp.Unit_AWB_Gr;
    bg_ratio= (1000*current_otp.Unit_AWB_B)/current_otp.Unit_AWB_Gr;

    //SENSORDB("[OTP_AWB]Gloden_AWB_R=%d\n",current_otp.Gloden_AWB_R);
    //SENSORDB("[OTP_AWB]Unit_AWB_Gr=%d\n",current_otp.Unit_AWB_Gr);
    //SENSORDB("[OTP_AWB]RG_Ratio_typical_semco_kantatsu=%d\n",RG_Ratio_typical_semco_kantatsu);

    //SENSORDB("[OTP_AWB]BG_Ratio_typical_semco_kantatsu=%d\n",BG_Ratio_typical_semco_kantatsu);
    //SENSORDB("[OTP_AWB]rg_ratio=%d\n",rg_ratio);
    //SENSORDB("[OTP_AWB]bg_ratio=%d\n",bg_ratio);

    //calculate gain
    //0x400 = 1x gain
    if(bg_ratio < BG_Ratio_typical_semco_kantatsu)
    {
        if(rg_ratio < RG_Ratio_typical_semco_kantatsu)
        {
            //current_opt.bg_ratio < BG_Ratio_typical_semco_kantatsu &&
            //cuttent_otp.rg < RG_Ratio_typical_semco_kantatsu
            G_gain = 0x400;
            B_gain = (0x400 * BG_Ratio_typical_semco_kantatsu) / bg_ratio;
            R_gain = (0x400 * RG_Ratio_typical_semco_kantatsu )/ rg_ratio;
        }
        else
        {
            //current_otp.bg_ratio < BG_Ratio_typical_semco_kantatsu &&
            //current_otp.rg_ratio >= RG_Ratio_typical_semco_kantatsu
            R_gain = 0x400;
            G_gain = (0x400 * rg_ratio) / RG_Ratio_typical_semco_kantatsu;
            B_gain = (G_gain * BG_Ratio_typical_semco_kantatsu )/ bg_ratio;
        }
    }
    else
    {
        if(rg_ratio < RG_Ratio_typical_semco_kantatsu)
        {
            //current_otp.bg_ratio >= BG_Ratio_typical_semco_kantatsu &&
            //current_otp.rg_ratio < RG_Ratio_typical_semco_kantatsu
            B_gain = 0x400;
            G_gain = (0x400 * bg_ratio )/ BG_Ratio_typical_semco_kantatsu;
            R_gain = (G_gain * RG_Ratio_typical_semco_kantatsu) / rg_ratio;
        }
        else
        {
            //current_otp.bg_ratio >= BG_Ratio_typical_semco_kantatsu &&
            //current_otp.rg_ratio >= RG_Ratio_typical_semco_kantatsu
            G_gain_B = 0x400*bg_ratio / BG_Ratio_typical_semco_kantatsu;
            G_gain_R = 0x400*rg_ratio / RG_Ratio_typical_semco_kantatsu;

            if(G_gain_B > G_gain_R)
            {
                B_gain = 0x400;
                G_gain = G_gain_B;
                R_gain = G_gain * RG_Ratio_typical_semco_kantatsu / rg_ratio;
            }
            else
            {
                R_gain = 0x400;
                G_gain = G_gain_R;
                B_gain = G_gain * BG_Ratio_typical_semco_kantatsu / bg_ratio;
            }
        }
    }
    //write sensor wb gain to register
    ov8835mipi_semco_kantatsuupdate_wb_gain(R_gain,G_gain,B_gain);

    //success
    return 0;
}

//call this function after OV8835MIPI initialization
//return value:	0 update success
//				1 no otp
//<2012/12/10 Clarklin, Reduce load OTP time, about 0.12s
struct OV8835MIPI_Semco_Kantatsu_otp_struct g_current_otp_semco_kantatsu;

kal_uint8 g_OTPReadSuccess_Semco_Kantatsu = KAL_FALSE;
//>2012/12/10 Clarklin


kal_uint16 ov8835mipi_semco_kantatsuupdate_lenc_register_from_otp(void)
{
    kal_uint16 temp,i;
   

    if(ov8835mipi_semco_kantatsucheck_otp_Index()) return 1;
//<2012/12/10 Clarklin, Reduce load OTP time, about 0.12s
	if (g_OTPReadSuccess_Semco_Kantatsu != KAL_TRUE)
		{
    ov8835mipi_semco_kantatsuread_otp_lenc(OTP_index_semco_kantatsu,&g_current_otp_semco_kantatsu);
	g_OTPReadSuccess_Semco_Kantatsu = KAL_TRUE;
		}
    ov8835mipi_semco_kantatsuupdate_lenc(&g_current_otp_semco_kantatsu);
//>2012/12/10 Clarklin

    //at last should enable the shading enable register
//<2012/11/07-16438-alberthsiao,Fix OV8835 dead pixel issue for Hawk40
    temp = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x5000);
//>2012/11/07-16438-alberthsiao
    temp |= 0x80;
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5000,temp);

    //success
    return 0;
}

kal_uint16 ov8835mipi_semco_kantatsuupdate_blc_register_from_otp(void)
{
    struct OV8835MIPI_Semco_Kantatsu_otp_struct current_otp;

    ov8835mipi_semco_kantatsuread_otp_blc(&current_otp);

    ov8835mipi_semco_kantatsuupdate_blc(&current_otp);
    //success
    return 0;
}

kal_uint16 ov8835mipi_semco_kantatsuupdate_af_from_otp(void)
{
    kal_uint16 temp,i;
    struct OV8835MIPI_Semco_Kantatsu_otp_struct current_otp;

    if(ov8835mipi_semco_kantatsucheck_otp_Index()) return 1;
    
    ov8835mipi_semco_kantatsuread_otp_af(OTP_index_semco_kantatsu,&current_otp);

    //success
    return 0;
}
void ov8835mipi_semco_kantatsuread_all_register_from_otp(void)
{
    kal_uint16 i,j,address;

    for(i=0;i<32;i++)
    {
        //select bank 
        OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,(kal_uint32)(0xc0+i));
        //load otp to buffer
        OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,(kal_uint32)(0x01));
        msleep(10);
        //disable otp read
        OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,(kal_uint32)(0x00));

        address = 0x3d00;
        for(j=0;j<16;j++)
            SENSORDB("[ov8835mipi_semco_kantatsuread_all_register_from_otp][%02d][%02d]-%02x\n",i,j,OV8835MIPI_Semco_Kantatsu_read_cmos_sensor((kal_uint32)(0x3d00+j)));
    }
}
#endif

//MEIMEI END



#if 0
#ifdef OV8835MIPI_Semco_Kantatsu_USE_OTP

//index:index of otp group.(0,1,2)
//return:	0:group index is empty.
//		1.group index has invalid data
//		2.group index has valid data

kal_uint16 ov8835mipi_semco_kantatsucheck_otp_wb(kal_uint16 index)
{
	kal_uint16 temp,flag;
	kal_uint32 address;

    if(index < 2)
    	{
    		//select bank 0
    		OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc0);
			//load otp to buffer
			OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
			msleep(10);

			//disable otp read
			OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

			//read flag
			address = 0x3d05+index*9;
			flag = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    	}
	else
		{
			//select bank 1
    		OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc1);
			//load otp to buffer
			OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
			msleep(10);

			//disable otp read
			OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

			//read flag
        address = 0x3d07;
			flag = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
		}
	//disable otp read
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

	if(NULL == flag)
		{
			
			SENSORDB("[ov8835mipi_semco_kantatsucheck_otp_wb]index[%x]read flag[%x][0]\n",index,flag);
			return 0;
			
		}
	else if(!(flag&0x80) && (flag&0x7f))
		{
			SENSORDB("[ov8835mipi_semco_kantatsucheck_otp_wb]index[%x]read flag[%x][2]\n",index,flag);
			return 2;
		}
	else
		{
			SENSORDB("[ov8835mipi_semco_kantatsucheck_otp_wb]index[%x]read flag[%x][1]\n",index,flag);
		    return 1;
		}
	
}

//index:index of otp group.(0,1,2)
//return:	0.group index is empty.
//		1.group index has invalid data
//		2.group index has valid data

kal_uint16 ov8835mipi_semco_kantatsucheck_otp_lenc(kal_uint16 index)
{
   kal_uint16 temp,flag,i;
   kal_uint32 address;

   //select bank :index*4+2
   OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc2+index*4);

   //read otp
   OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
   msleep(10);

   //disable otp read
   OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

   //read flag
   address = 0x3d00; 
   flag = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
   flag = flag & 0xc0;

   //disable otp read
   OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

   //clear otp buffer
   address = 0x3d00;
   for(i = 0;i<16; i++)
   	{
   		OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(address+i,0x00);
   	}

   if(NULL == flag)
   	{
   		SENSORDB("[ov8835mipi_semco_kantatsucheck_otp_lenc]index[%x]read flag[%x][0]\n",index,flag);
   	    return 0;
   	}
   else if(0x40 == flag)
   	{
   		SENSORDB("[ov8835mipi_semco_kantatsucheck_otp_lenc]index[%x]read flag[%x][2]\n",index,flag);
   	    return 2;
   	}
   else
   	{
   		SENSORDB("[ov8835mipi_semco_kantatsucheck_otp_lenc]index[%x]read flag[%x][1]\n",index,flag);
		return 1;
   	}
}

kal_uint16 ov8835mipi_semco_kantatsucheck_otp_blc(kal_uint16 index)
{
	kal_uint16 temp,flag;
	kal_uint32 address;
	
    	{
			
			//select bank 31: 0x1f
    		OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc0+0x1f);
			//load otp to buffer
			OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
			msleep(10);

			//disable otp read
			OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

			//read flag
			address = 0x3d01;
			flag = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
    	}

	//disable otp read
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

	if(NULL == flag)
		{
			
			SENSORDB("[ov8835mipi_semco_kantatsucheck_otp_blc] read flag[%x]\n",index,flag);
			return 0;			
		}
	else
		{
			SENSORDB("[ov8835mipi_semco_kantatsucheck_otp_blc] read flag[%x]\n",index,flag);
			return 1;
		}	
}

//for otp_af
struct ov8835mipi_semco_kantatsuotp_af_struct 
{
	kal_uint16 group1_data;
	kal_uint16 group1_far_h8;
	kal_uint16 group1_far_l8;
	kal_uint16 group1_near_h8;
	kal_uint16 group1_near_l8;

	kal_uint16 group2_data;
	kal_uint16 group2_far_h8;
	kal_uint16 group2_far_l8;
	kal_uint16 group2_near_h8;
	kal_uint16 group2_near_l8;

	kal_uint16 group3_data;
	kal_uint16 group3_far_h8;
	kal_uint16 group3_far_l8;
	kal_uint16 group3_near_h8;
	kal_uint16 group3_near_l8;
};

struct ov8835mipi_semco_kantatsuotp_af_struct ov8835mipi_semco_kantatsuread_otp_af(void)
{
	struct ov8835mipi_semco_kantatsuotp_af_struct otp;
	kal_uint16 i;
	kal_uint32 address;

	//select bank 15
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xcf);
	//load otp to buffer
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
	msleep(10);
				
	//disable otp read
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

	otp.group1_data = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3d00);
	otp.group1_far_h8 = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3d01);
	otp.group1_far_l8 = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3d02);
	otp.group1_near_h8 = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3d03);
	otp.group1_near_l8 = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3d04);

	otp.group2_data = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3d05);
	otp.group2_far_h8 = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3d06);
	otp.group2_far_l8 = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3d07);
	otp.group2_near_h8 = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3d08);
	otp.group2_near_l8 = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3d09);

	otp.group3_data = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3d0a);
	otp.group3_far_h8 = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3d0b);
	otp.group3_far_l8 = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3d0c);
	otp.group3_near_h8 = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3d0d);
	otp.group3_near_l8 = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3d0e);


	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_af]address[%x]group1_data[%x]\n",0x3d00,otp.group1_data);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_af]address[%x]group1_far_h8[%x]\n",0x3d01,otp.group1_far_h8);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_af]address[%x]group1_far_l8[%x]\n",0x3d02,otp.group1_far_l8);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_af]address[%x]group1_near_h8[%x]\n",0x3d03,otp.group1_near_h8);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_af]address[%x]group1_near_l8[%x]\n",0x3d04,otp.group1_near_l8);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_af]address[%x]group2_data[%x]\n",0x3d05,otp.group2_data);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_af]address[%x]group2_far_h8[%x]\n",0x3d06,otp.group2_far_h8);	
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_af]address[%x]group2_far_l8[%x]\n",0x3d07,otp.group2_far_l8);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_af]address[%x]group2_near_h8[%x]\n",0x3d08,otp.group2_near_h8);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_af]address[%x]group2_near_h8[%x]\n",0x3d09,otp.group2_near_l8);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_af]address[%x]group2_near_h8[%x]\n",0x3d0a,otp.group3_data);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_af]address[%x]group2_near_h8[%x]\n",0x3d0b,otp.group3_far_h8);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_af]address[%x]group2_near_h8[%x]\n",0x3d0c,otp.group3_far_l8);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_af]address[%x]group2_near_h8[%x]\n",0x3d0d,otp.group3_near_h8);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_af]address[%x]group2_near_h8[%x]\n",0x3d0e,otp.group3_near_l8);
	//disable otp read
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);
	
	//clear otp buffer
	address = 0x3d00;
	for(i =0; i<32; i++)
		{
			OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d00,0x00);
		}

	return otp;
}
//end otp_af

//index:index of otp group.(0,1,2)
//return: 0
kal_uint16 ov8835mipi_semco_kantatsuread_otp_wb(kal_uint16 index, struct ov8835mipi_semco_kantatsuotp_struct *otp)
{
	kal_uint16 temp,i;
	kal_uint32 address;

	switch(index)
		{
			case 0:
				
				//select bank 0
				OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc0);
				//load otp to buffer
				OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
				msleep(10);
				
				//disable otp read
				OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

                address = 0x3d05;
                otp->module_integrator_id = (OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address)&0x7f);
				otp->lens_id = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+1);
				otp->rg_ratio = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+2);
				otp->bg_ratio = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+3);
				otp->user_data[0] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+4);
				otp->user_data[1] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+5);
				otp->user_data[2] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+6);
				otp->user_data[3] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+7);
				otp->user_data[4] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+8);
				break;
			case 1:
				//select bank 0
				OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc0);
				//load otp to buffer
				OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
				msleep(10);
				
				//disable otp read
                OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

                address = 0x3d0e;
                otp->module_integrator_id = (OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address)&0x7f);
                otp->lens_id = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+1);

				//select bank 1
				OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc1);
				//load otp to buffer
				OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
				msleep(10);
				
				//disable otp read
				OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

				address = 0x3d00;
				otp->rg_ratio = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
				otp->bg_ratio = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+1);
				otp->user_data[0] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+2);
				otp->user_data[1] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+3);
				otp->user_data[2] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+4);
				otp->user_data[3] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+5);
				otp->user_data[4] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+6);
				break;
			case 2:
				//select bank 1
				OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc1);
				//load otp to buffer
				OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
				msleep(10);
				
                //disable otp read
                OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

                address = 0x3d07;
                otp->module_integrator_id = (OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address)&0x7f);
                otp->lens_id = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+1);
                otp->rg_ratio = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+2);
				otp->bg_ratio = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+3);
				otp->user_data[0] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+4);
				otp->user_data[1] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+5);
				otp->user_data[2] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+6);
				otp->user_data[3] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+7);
				otp->user_data[4] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+8);
				break;
			default:
				break;
				
		}

	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]module_integrator_id[%x]\n",address,otp->module_integrator_id);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]lens_id[%x]\n",address,otp->lens_id);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]rg_ratio[%x]\n",address,otp->rg_ratio);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]bg_ratio[%x]\n",address,otp->bg_ratio);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]user_data[0][%x]\n",address,otp->user_data[0]);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]user_data[1][%x]\n",address,otp->user_data[1]);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]user_data[2][%x]\n",address,otp->user_data[2]);	
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]user_data[3][%x]\n",address,otp->user_data[3]);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]user_data[4][%x]\n",address,otp->user_data[4]);

	//disable otp read
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

        //clear otp buffer
        address = 0x3d00;
        for(i =0; i<16; i++)
        {
            OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(address+i, 0x00);
        }

        return 0;
	
}

kal_uint16 ov8835mipi_semco_kantatsuread_otp_lenc(kal_uint16 index,struct ov8835mipi_semco_kantatsuotp_struct *otp)
{
	kal_uint16 bank,temp,i;
	kal_uint32 address;

    //select bank: index*4 +2;
    bank = index*4+2;
	temp = 0xc0|bank;
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,temp);

	//read otp
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
	msleep(10);

	//disabe otp read
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

	
	address = 0x3d01;
	for(i = 0; i < 15; i++)
		{
			otp->lenc[i] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
			address++;
		}

	//select bank+1
    bank++;
	temp = 0xc0|bank;
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,temp);

	//read otp
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
	msleep(10);

	//disabe otp read
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);


        address = 0x3d00;
        for(i = 15; i < 31; i++)
        {
                otp->lenc[i] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
                address++;
        }

	//select bank+2
    bank++;
	temp = 0xc0|bank;
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,temp);

	//read otp
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
	msleep(10);

	//disabe otp read
        OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);


        address = 0x3d00;
        for(i = 31; i < 47; i++)
        {
                otp->lenc[i] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
                address++;
        }

	//select bank+3
    bank++;
	temp = 0xc0|bank;
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,temp);

	//read otp
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
	msleep(10);

	//disabe otp read
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);


        address = 0x3d00;
        for(i = 47; i < 62; i++)
        {
            otp->lenc[i] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
            address++;
        }

	//disable otp read
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

	
	//clear otp buffer
		address = 0x3d00;
		for(i =0; i<32; i++)
			{
				OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d00,0x00);
			}
	return 0;
}

kal_uint16 ov8835mipi_semco_kantatsuread_otp_blc(kal_uint16 index, struct ov8835mipi_semco_kantatsuotp_struct *otp)
{
	kal_uint16 temp,i;
	kal_uint32 address;


		{

				
				//select bank 0
				OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84,0xc0);
				//load otp to buffer
				OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x01);
				msleep(10);
				
				//disable otp read
				OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

                address = 0x3d0a;
             //   otp->module_integrator_id = (OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address)&0x7f);
			//	otp->lens_id = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+1);
			//	otp->rg_ratio = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+2);
			//	otp->bg_ratio = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+3);
				otp->user_data[0] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address);
			//	otp->user_data[1] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+5);
			//	otp->user_data[2] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+6);
			//	otp->user_data[3] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+7);
			//	otp->user_data[4] = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(address+8);
				
		}

	/*SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]module_integrator_id[%x]\n",address,otp->module_integrator_id);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]lens_id[%x]\n",address,otp->lens_id);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]rg_ratio[%x]\n",address,otp->rg_ratio);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]bg_ratio[%x]\n",address,otp->bg_ratio);*/
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]user_data[0][%x]\n",address,otp->user_data[0]);
	/*SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]user_data[1][%x]\n",address,otp->user_data[1]);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]user_data[2][%x]\n",address,otp->user_data[2]);	
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]user_data[3][%x]\n",address,otp->user_data[3]);
	SENSORDB("[ov8835mipi_semco_kantatsuread_otp_wb]address[%x]user_data[4][%x]\n",address,otp->user_data[4]);*/

	//disable otp read
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81,0x00);

        return 0;
	
}


//R_gain: red gain of sensor AWB, 0x400 = 1
//G_gain: green gain of sensor AWB, 0x400 = 1
//B_gain: blue gain of sensor AWB, 0x400 = 1
//reutrn 0
kal_uint16 ov8835mipi_semco_kantatsuupdate_wb_gain(kal_uint32 R_gain, kal_uint32 G_gain, kal_uint32 B_gain)
{

    SENSORDB("[ov8835mipi_semco_kantatsuupdate_wb_gain]R_gain[%x]G_gain[%x]B_gain[%x]\n",R_gain,G_gain,B_gain);

	if(R_gain > 0x400)
		{
			OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5186,R_gain >> 8);
			OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5187,(R_gain&0x00ff));
		}
	if(G_gain > 0x400)
		{
			OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5188,G_gain >> 8);
			OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5189,(G_gain&0x00ff));
		}
	if(B_gain >0x400)
		{
			OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x518a,B_gain >> 8);
			OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x518b,(B_gain&0x00ff));
		}
	return 0;
}


//return 0
kal_uint16 ov8835mipi_semco_kantatsuupdate_lenc(struct ov8835mipi_semco_kantatsuotp_struct *otp)
{
        kal_uint16 i, temp;
        //lenc g
        for(i = 0; i < 62; i++)
        {
                OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5800+i,otp->lenc[i]);

                SENSORDB("[ov8835mipi_semco_kantatsuupdate_lenc]otp->lenc[%d][%x]\n",i,otp->lenc[i]);
        }

        return 0;
}

kal_uint16 ov8835mipi_semco_kantatsuupdate_blc(struct ov8835mipi_semco_kantatsuotp_struct *otp)
{
        kal_uint16 i, temp;
        OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4006,otp->user_data[0]);

        SENSORDB("[ov8835mipi_semco_kantatsuupdate_Blc]otp->blc[%d][%x]\n",i,otp->user_data[0]);
        return 0;
}


//R/G and B/G ratio of typical camera module is defined here

kal_uint32 RG_Ratio_typical_semco_kantatsu = RG_TYPICAL;
kal_uint32 BG_Ratio_typical_semco_kantatsu = BG_TYPICAL;

//call this function after OV5650 initialization
//return value:	0 update success
//				1 no 	OTP

kal_uint16 ov8835mipi_semco_kantatsuupdate_wb_register_from_otp(void)
{
	kal_uint16 temp, i, otp_index;
	struct ov8835mipi_semco_kantatsuotp_struct current_otp;
	kal_uint32 R_gain, B_gain, G_gain, G_gain_R,G_gain_B;

	SENSORDB("ov8835mipi_semco_kantatsuupdate_wb_register_from_otp\n");


	//check first wb OTP with valid OTP
	for(i = 0; i < 3; i++)
		{
			temp = ov8835mipi_semco_kantatsucheck_otp_wb(i);
			if(temp == 2)
				{
					otp_index = i;
					break;
				}
		}
	if( 3 == i)
		{
		 	SENSORDB("[ov8835mipi_semco_kantatsuupdate_wb_register_from_otp]no valid wb OTP data!\r\n");
			return 1;
		}
	ov8835mipi_semco_kantatsuread_otp_wb(otp_index,&current_otp);

	//calculate gain
	//0x400 = 1x gain
	if(current_otp.bg_ratio < BG_Ratio_typical_semco_kantatsu)
		{
			if(current_otp.rg_ratio < RG_Ratio_typical_semco_kantatsu)
				{
					//current_opt.bg_ratio < BG_Ratio_typical_semco_kantatsu &&
					//cuttent_otp.rg < RG_Ratio_typical_semco_kantatsu

					G_gain = 0x400;
					B_gain = 0x400 * BG_Ratio_typical_semco_kantatsu / current_otp.bg_ratio;
					R_gain = 0x400 * RG_Ratio_typical_semco_kantatsu / current_otp.rg_ratio;
				}
			else
				{
					//current_otp.bg_ratio < BG_Ratio_typical_semco_kantatsu &&
			        //current_otp.rg_ratio >= RG_Ratio_typical_semco_kantatsu
			        R_gain = 0x400;
					G_gain = 0x400 * current_otp.rg_ratio / RG_Ratio_typical_semco_kantatsu;
					B_gain = G_gain * BG_Ratio_typical_semco_kantatsu / current_otp.bg_ratio;
					
			        
				}
		}
	else
		{
			if(current_otp.rg_ratio < RG_Ratio_typical_semco_kantatsu)
				{
					//current_otp.bg_ratio >= BG_Ratio_typical_semco_kantatsu &&
			        //current_otp.rg_ratio < RG_Ratio_typical_semco_kantatsu
			        B_gain = 0x400;
					G_gain = 0x400 * current_otp.bg_ratio / BG_Ratio_typical_semco_kantatsu;
					R_gain = G_gain * RG_Ratio_typical_semco_kantatsu / current_otp.rg_ratio;
					
				}
			else
				{
					//current_otp.bg_ratio >= BG_Ratio_typical_semco_kantatsu &&
			        //current_otp.rg_ratio >= RG_Ratio_typical_semco_kantatsu
			        G_gain_B = 0x400*current_otp.bg_ratio / BG_Ratio_typical_semco_kantatsu;
				    G_gain_R = 0x400*current_otp.rg_ratio / RG_Ratio_typical_semco_kantatsu;
					
					if(G_gain_B > G_gain_R)
						{
							B_gain = 0x400;
							G_gain = G_gain_B;
							R_gain = G_gain * RG_Ratio_typical_semco_kantatsu / current_otp.rg_ratio;
						}
					else

						{
							R_gain = 0x400;
							G_gain = G_gain_R;
							B_gain = G_gain * BG_Ratio_typical_semco_kantatsu / current_otp.bg_ratio;
						}
			        
				}
			
		}
	//write sensor wb gain to register
	ov8835mipi_semco_kantatsuupdate_wb_gain(R_gain,G_gain,B_gain);

	//success
	return 0;
}

//call this function after ov5650 initialization
//return value:	0 update success
//				1 no otp

kal_uint16 ov8835mipi_semco_kantatsuupdate_lenc_register_from_otp(void)
{
	kal_uint16 temp,i,otp_index;
    struct ov8835mipi_semco_kantatsuotp_struct current_otp;

	for(i = 0; i < 3; i++)
		{
			temp = ov8835mipi_semco_kantatsucheck_otp_lenc(i);
			if(2 == temp)
				{
					otp_index = i;
					break;
				}
		}
	if(3 == i)
		{
		 	SENSORDB("[ov8835mipi_semco_kantatsuupdate_lenc_register_from_otp]no valid wb OTP data!\r\n");
			return 1;
		}
	ov8835mipi_semco_kantatsuread_otp_lenc(otp_index,&current_otp);

	ov8835mipi_semco_kantatsuupdate_lenc(&current_otp);

	//success
	return 0;
}

kal_uint16 ov8835mipi_semco_kantatsuupdate_blc_register_from_otp(void)
{
	kal_uint16 temp,i,otp_index;
    struct ov8835mipi_semco_kantatsuotp_struct current_otp;

	temp = ov8835mipi_semco_kantatsucheck_otp_blc(i);

	if(temp == 0)//not update
	{
	 	SENSORDB("[ov8835mipi_semco_kantatsuupdate_blc_register_from_otp]no valid blc OTP data!\r\n");
		return 1;
	}
	else//update
	{
		ov8835mipi_semco_kantatsuread_otp_blc(otp_index,&current_otp);
		ov8835mipi_semco_kantatsuupdate_blc(&current_otp);
	 	SENSORDB("[ov8835mipi_semco_kantatsuupdate_blc_register_from_otp] valid blc OTP data!\r\n");
		return 0;
	}
}

#endif
#endif


void OV8835MIPI_Semco_Kantatsu_Write_Shutter(kal_uint16 ishutter)
{

kal_uint16 extra_shutter = 0;
kal_uint16 frame_length = 0;
kal_uint16 realtime_fp = 0;
kal_uint32 pclk = 0;

unsigned long flags;
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
	SENSORDB("OV8835MIPI_Semco_Kantatsu_write_shutter:%x \n",ishutter);
#endif
   if (!ishutter) ishutter = 1; /* avoid 0 */
   

   if (OV8835MIPI_Semco_Kantatsu_sensor.pv_mode)
	{
	  
	  frame_length = OV8835MIPI_Semco_Kantatsu_PV_PERIOD_LINE_NUMS + OV8835MIPI_Semco_Kantatsu_sensor.dummy_lines;
	}
	else
	{
	  
	  frame_length = OV8835MIPI_Semco_Kantatsu_FULL_PERIOD_LINE_NUMS + OV8835MIPI_Semco_Kantatsu_sensor.dummy_lines;
	}


	if(ishutter > (frame_length -14))
		{
		    
			
			extra_shutter = ishutter - frame_length  +14;

			SENSORDB("[shutter > frame_length] frame_height:%x extra_shutter:%x \n",frame_length,extra_shutter);

			
		}
	else  
		{
			extra_shutter = 0;
		}

	if (OV8835MIPI_Semco_Kantatsu_sensor.pv_mode)
	{
	  //line_length = OV8835MIPI_Semco_Kantatsu_PV_PERIOD_PIXEL_NUMS + iPixels;
	  spin_lock_irqsave(&ov8835mipi_semco_kantatsudrv_lock,flags);
	  OV8835MIPI_Semco_Kantatsu_sensor.frame_height = OV8835MIPI_Semco_Kantatsu_PV_PERIOD_LINE_NUMS + OV8835MIPI_Semco_Kantatsu_sensor.dummy_lines + extra_shutter;
	  spin_unlock_irqrestore(&ov8835mipi_semco_kantatsudrv_lock,flags);
	}
	else
	{
	  //line_length = OV8835MIPI_Semco_Kantatsu_FULL_PERIOD_PIXEL_NUMS + iPixels;
	  spin_lock_irqsave(&ov8835mipi_semco_kantatsudrv_lock,flags);
	  OV8835MIPI_Semco_Kantatsu_sensor.frame_height = OV8835MIPI_Semco_Kantatsu_FULL_PERIOD_LINE_NUMS + OV8835MIPI_Semco_Kantatsu_sensor.dummy_lines + extra_shutter;
	  spin_unlock_irqrestore(&ov8835mipi_semco_kantatsudrv_lock,flags);
	}
	SENSORDB("OV8835MIPI_Semco_Kantatsu_sensor.frame_height:%x \n",OV8835MIPI_Semco_Kantatsu_sensor.frame_height);

	//		SENSORDB("OV8835MIPI_Semco_Kantatsu_sensor.is_autofliker:%x \n",OV8835MIPI_Semco_Kantatsu_sensor.is_autofliker);
#if 1
	if(OV8835MIPI_Semco_Kantatsu_sensor.is_autofliker == KAL_TRUE)
		{
		  		if(OV8835MIPI_Semco_Kantatsu_sensor.is_zsd == KAL_TRUE)
		  			{
		  				pclk = OV8835MIPI_Semco_Kantatsu_ZSD_PRE_CLK;
		  			}
				 else
				 	{
				 		pclk = OV8835MIPI_Semco_Kantatsu_PREVIEW_CLK;
				 	}
					
				realtime_fp = pclk *10 / (OV8835MIPI_Semco_Kantatsu_sensor.line_length * OV8835MIPI_Semco_Kantatsu_sensor.frame_height);
			    SENSORDB("[OV8835MIPI_Semco_Kantatsu_Write_Shutter]pv_clk:%d\n",pclk);
				SENSORDB("[OV8835MIPI_Semco_Kantatsu_Write_Shutter]line_length:%d\n",OV8835MIPI_Semco_Kantatsu_sensor.line_length);
				SENSORDB("[OV8835MIPI_Semco_Kantatsu_Write_Shutter]frame_height:%d\n",OV8835MIPI_Semco_Kantatsu_sensor.frame_height);
			    SENSORDB("[OV8835MIPI_Semco_Kantatsu_Write_Shutter]framerate(10base):%d\n",realtime_fp);

				if((realtime_fp >= 297)&&(realtime_fp <= 303))
					{
						realtime_fp = 296;
						spin_lock_irqsave(&ov8835mipi_semco_kantatsudrv_lock,flags);
						OV8835MIPI_Semco_Kantatsu_sensor.frame_height = pclk *10 / (OV8835MIPI_Semco_Kantatsu_sensor.line_length * realtime_fp);
						spin_unlock_irqrestore(&ov8835mipi_semco_kantatsudrv_lock,flags);

						SENSORDB("[autofliker realtime_fp=30,extern heights slowdown to 29.6fps][height:%d]",OV8835MIPI_Semco_Kantatsu_sensor.frame_height);
					}
				else if((realtime_fp >= 147)&&(realtime_fp <= 153))
					{
						realtime_fp = 146;
						spin_lock_irqsave(&ov8835mipi_semco_kantatsudrv_lock,flags);
						OV8835MIPI_Semco_Kantatsu_sensor.frame_height = pclk *10 / (OV8835MIPI_Semco_Kantatsu_sensor.line_length * realtime_fp);
						spin_unlock_irqrestore(&ov8835mipi_semco_kantatsudrv_lock,flags);
						
						SENSORDB("[autofliker realtime_fp=15,extern heights slowdown to 14.6fps][height:%d]",OV8835MIPI_Semco_Kantatsu_sensor.frame_height);
					}
				//OV8835MIPI_Semco_Kantatsu_sensor.frame_height = OV8835MIPI_Semco_Kantatsu_sensor.frame_height +(OV8835MIPI_Semco_Kantatsu_sensor.frame_height>>7);

		}
#endif





    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380e, (OV8835MIPI_Semco_Kantatsu_sensor.frame_height>>8)&0xFF);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380f, (OV8835MIPI_Semco_Kantatsu_sensor.frame_height)&0xFF);

	
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3500, (ishutter >> 12) & 0xF);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3501, (ishutter >> 4) & 0xFF);	
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3502, (ishutter << 4) & 0xFF);

}

static void OV8835MIPI_Semco_Kantatsu_Set_Dummy(const kal_uint16 iPixels, const kal_uint16 iLines)
{
	kal_uint16 line_length, frame_height;
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
	SENSORDB("OV8835MIPI_Semco_Kantatsu_Set_Dummy:iPixels:%x; iLines:%x \n",iPixels,iLines);
#endif
//return;
	if (OV8835MIPI_Semco_Kantatsu_sensor.pv_mode)
	{
	  line_length = OV8835MIPI_Semco_Kantatsu_PV_PERIOD_PIXEL_NUMS + iPixels;
	  frame_height = OV8835MIPI_Semco_Kantatsu_PV_PERIOD_LINE_NUMS + iLines;
	}
	else
	{
	  line_length = OV8835MIPI_Semco_Kantatsu_FULL_PERIOD_PIXEL_NUMS + iPixels;
	  frame_height = OV8835MIPI_Semco_Kantatsu_FULL_PERIOD_LINE_NUMS + iLines;
	}

	if(frame_height < (OV8835MIPI_Semco_Kantatsu_sensor.shutter+14))
	{
		frame_height = OV8835MIPI_Semco_Kantatsu_sensor.shutter +14;
	}
	
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
	SENSORDB("line_length:%x; frame_height:%x \n",line_length,frame_height);
#endif

//	if ((line_length >= 0x1FFF)||(frame_height >= 0xFFF))
//	{
//#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
//		SENSORDB("Warnning: line length or frame height is overflow!!!!!!!!  \n");
//#endif
//		return ;
//	}


//	if((line_length == OV8835MIPI_Semco_Kantatsu_sensor.line_length)&&(frame_height == OV8835MIPI_Semco_Kantatsu_sensor.frame_height))
//		return ;
	spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
	OV8835MIPI_Semco_Kantatsu_sensor.line_length = line_length;
	OV8835MIPI_Semco_Kantatsu_sensor.frame_height = frame_height;
	spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);

	spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
	OV8835MIPI_Semco_Kantatsu_sensor.dummy_pixels = iPixels;
	OV8835MIPI_Semco_Kantatsu_sensor.dummy_lines = iLines;
	spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);

	SENSORDB("line_length:%x; frame_height:%x \n",line_length,frame_height);
	
    /*  Add dummy pixels: */
    /* 0x380c [0:4], 0x380d defines the PCLKs in one line of OV8835MIPI  */  
    /* Add dummy lines:*/
    /* 0x380e [0:1], 0x380f defines total lines in one frame of OV8835MIPI */
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380c, line_length >> 8);
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380d, line_length & 0xFF);
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380e, frame_height >> 8);
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380f, frame_height & 0xFF);
	
}   /*  OV8835MIPI_Semco_Kantatsu_Set_Dummy    */


/*************************************************************************
* FUNCTION
*	OV8835MIPI_Semco_Kantatsu_SetShutter
*
* DESCRIPTION
*	This function set e-shutter of OV8835MIPI to change exposure time.
*
* PARAMETERS
*   iShutter : exposured lines
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/


void set_OV8835MIPI_Semco_Kantatsu_shutter(kal_uint16 iShutter)
{

	unsigned long flags;
	
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
	SENSORDB("set_OV8835MIPI_Semco_Kantatsu_shutter:%x \n",iShutter);
#endif

    if((OV8835MIPI_Semco_Kantatsu_sensor.pv_mode == KAL_FALSE)&&(OV8835MIPI_Semco_Kantatsu_sensor.is_zsd == KAL_FALSE))
    	{
    	   SENSORDB("[set_OV8835MIPI_Semco_Kantatsu_shutter]now is in 1/4size cap mode\n");
		//   iShutter = 2*iShutter;
    	   //return;
    	}
	else if((OV8835MIPI_Semco_Kantatsu_sensor.is_zsd == KAL_TRUE)&&(OV8835MIPI_Semco_Kantatsu_sensor.is_zsd_cap == KAL_TRUE))
		{
			SENSORDB("[set_OV8835MIPI_Semco_Kantatsu_shutter]now is in zsd cap mode\n");

			//SENSORDB("[set_OV8835MIPI_Semco_Kantatsu_shutter]0x3500:%x\n",OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3500));
			//SENSORDB("[set_OV8835MIPI_Semco_Kantatsu_shutter]0x3500:%x\n",OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3501));
			//SENSORDB("[set_OV8835MIPI_Semco_Kantatsu_shutter]0x3500:%x\n",OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3502));
			//return;
		}

//<2012/12/10 Clarklin, Reduce load OTP time, about 0.12s
/*	if(OV8835MIPI_Semco_Kantatsu_sensor.shutter == iShutter)
		{
			SENSORDB("[set_OV8835MIPI_Semco_Kantatsu_shutter]shutter is the same with previous, skip\n");
			return;
		}*/
//>2012/12/10 Clarklin

	spin_lock_irqsave(&ov8835mipi_semco_kantatsudrv_lock,flags);
	OV8835MIPI_Semco_Kantatsu_sensor.shutter = iShutter;
	spin_unlock_irqrestore(&ov8835mipi_semco_kantatsudrv_lock,flags);
	
    OV8835MIPI_Semco_Kantatsu_Write_Shutter(iShutter);

}   /*  Set_OV8835MIPI_Semco_Kantatsu_Shutter */

 kal_uint16 OV8835MIPI_Semco_Kantatsu_Gain2Reg(const kal_uint16 iGain)
{
    kal_uint16 iReg = 0x00;

	//iReg = ((iGain / BASEGAIN) << 4) + ((iGain % BASEGAIN) * 16 / BASEGAIN);
	iReg = iGain *16 / BASEGAIN;

	iReg = iReg & 0xFF;
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
	SENSORDB("OV8835MIPI_Semco_Kantatsu_Gain2Reg:iGain:%x; iReg:%x \n",iGain,iReg);
#endif
    return iReg;
}


kal_uint16 OV8835MIPI_Semco_Kantatsu_SetGain(kal_uint16 iGain)
{
   kal_uint16 iReg;
   unsigned long flags;
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
   SENSORDB("OV8835MIPI_Semco_Kantatsu_SetGain:%x;\n",iGain);
#endif
//<2012/12/10 Clarklin, Reduce load OTP time, about 0.12s
/*   if(OV8835MIPI_Semco_Kantatsu_sensor.gain == iGain)
   	{
   		SENSORDB("[OV8835MIPI_Semco_Kantatsu_SetGain]:gain is the same with previous,skip\n");
	 	return 0;
   	}*/
//>2012/12/10 Clarklin

   spin_lock_irqsave(&ov8835mipi_semco_kantatsudrv_lock,flags);
   OV8835MIPI_Semco_Kantatsu_sensor.gain = iGain;
   spin_unlock_irqrestore(&ov8835mipi_semco_kantatsudrv_lock,flags);

  iReg = OV8835MIPI_Semco_Kantatsu_Gain2Reg(iGain);
   
	  if (iReg < 0x10) //MINI gain is 0x10	 16 = 1x
	   {
		   iReg = 0x10;
	   }
   
	  else if(iReg > 0xFF) //max gain is 0xFF
		   {
			   iReg = 0xFF;
		   }
		  
	   //OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x350a, (iReg>>8)&0xFF);
	   OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x350b, iReg&0xFF);//only use 0x350b for gain control
	  return iGain;
}




/*************************************************************************
* FUNCTION
*	OV8835MIPI_Semco_Kantatsu_SetGain
*
* DESCRIPTION
*	This function is to set global gain to sensor.
*
* PARAMETERS
*   iGain : sensor global gain(base: 0x40)
*
* RETURNS
*	the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/

#if 0
void OV8835MIPI_Semco_Kantatsu_set_isp_driving_current(kal_uint16 current)
{
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
   SENSORDB("OV8835MIPI_Semco_Kantatsu_set_isp_driving_current:current:%x;\n",current);
#endif
  //iowrite32((0x2 << 12)|(0<<28)|(0x8880888), 0xF0001500);
}
#endif

/*************************************************************************
* FUNCTION
*	OV8835MIPI_Semco_Kantatsu_NightMode
*
* DESCRIPTION
*	This function night mode of OV8835MIPI.
*
* PARAMETERS
*	bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void OV8835MIPI_Semco_Kantatsu_night_mode(kal_bool enable)
{
	const kal_uint16 dummy_pixel = OV8835MIPI_Semco_Kantatsu_sensor.line_length - OV8835MIPI_Semco_Kantatsu_PV_PERIOD_PIXEL_NUMS;
	const kal_uint16 pv_min_fps =  enable ? OV8835MIPI_Semco_Kantatsu_sensor.night_fps : OV8835MIPI_Semco_Kantatsu_sensor.normal_fps;
	kal_uint16 dummy_line = OV8835MIPI_Semco_Kantatsu_sensor.frame_height - OV8835MIPI_Semco_Kantatsu_PV_PERIOD_LINE_NUMS;
	kal_uint16 max_exposure_lines;
	
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
   SENSORDB("OV8835MIPI_Semco_Kantatsu_night_mode:enable:%x;\n",enable);
#endif

	if (!OV8835MIPI_Semco_Kantatsu_sensor.video_mode) return;
	max_exposure_lines = OV8835MIPI_Semco_Kantatsu_sensor.pv_pclk * OV8835MIPI_Semco_Kantatsu_FPS(1) / (pv_min_fps * OV8835MIPI_Semco_Kantatsu_sensor.line_length);
	if (max_exposure_lines > OV8835MIPI_Semco_Kantatsu_sensor.frame_height) /* fix max frame rate, AE table will fix min frame rate */
	{
	  dummy_line = max_exposure_lines - OV8835MIPI_Semco_Kantatsu_PV_PERIOD_LINE_NUMS;
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
	 SENSORDB("dummy_line:%x;\n",dummy_line);
#endif
	  OV8835MIPI_Semco_Kantatsu_Set_Dummy(dummy_pixel, dummy_line);
	}

}   /*  OV8835MIPI_Semco_Kantatsu_NightMode    */


/* write camera_para to sensor register */
static void OV8835MIPI_Semco_Kantatsu_camera_para_to_sensor(void)
{
  kal_uint32 i;
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
	 SENSORDB("OV8835MIPI_Semco_Kantatsu_camera_para_to_sensor\n");
#endif
  for (i = 0; 0xFFFFFFFF != OV8835MIPI_Semco_Kantatsu_sensor.eng.reg[i].Addr; i++)
  {
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(OV8835MIPI_Semco_Kantatsu_sensor.eng.reg[i].Addr, OV8835MIPI_Semco_Kantatsu_sensor.eng.reg[i].Para);
  }
  for (i = OV8835MIPI_Semco_Kantatsu_FACTORY_START_ADDR; 0xFFFFFFFF != OV8835MIPI_Semco_Kantatsu_sensor.eng.reg[i].Addr; i++)
  {
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(OV8835MIPI_Semco_Kantatsu_sensor.eng.reg[i].Addr, OV8835MIPI_Semco_Kantatsu_sensor.eng.reg[i].Para);
  }
  OV8835MIPI_Semco_Kantatsu_SetGain(OV8835MIPI_Semco_Kantatsu_sensor.gain); /* update gain */
}

/* update camera_para from sensor register */
static void OV8835MIPI_Semco_Kantatsu_sensor_to_camera_para(void)
{
  kal_uint32 i;
  kal_uint32 temp_data;
  
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
   SENSORDB("OV8835MIPI_Semco_Kantatsu_sensor_to_camera_para\n");
#endif
  for (i = 0; 0xFFFFFFFF != OV8835MIPI_Semco_Kantatsu_sensor.eng.reg[i].Addr; i++)
  {
  	temp_data = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(OV8835MIPI_Semco_Kantatsu_sensor.eng.reg[i].Addr);

	spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
    OV8835MIPI_Semco_Kantatsu_sensor.eng.reg[i].Para = temp_data;
	spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
	
  }
  for (i = OV8835MIPI_Semco_Kantatsu_FACTORY_START_ADDR; 0xFFFFFFFF != OV8835MIPI_Semco_Kantatsu_sensor.eng.reg[i].Addr; i++)
  {
  	temp_data = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(OV8835MIPI_Semco_Kantatsu_sensor.eng.reg[i].Addr);
	
	spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
    OV8835MIPI_Semco_Kantatsu_sensor.eng.reg[i].Para = temp_data;
	spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
  }
}

/* ------------------------ Engineer mode ------------------------ */
inline static void OV8835MIPI_Semco_Kantatsu_get_sensor_group_count(kal_int32 *sensor_count_ptr)
{
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
   SENSORDB("OV8835MIPI_Semco_Kantatsu_get_sensor_group_count\n");
#endif
  *sensor_count_ptr = OV8835MIPI_Semco_Kantatsu_GROUP_TOTAL_NUMS;
}

inline static void OV8835MIPI_Semco_Kantatsu_get_sensor_group_info(MSDK_SENSOR_GROUP_INFO_STRUCT *para)
{
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
   SENSORDB("OV8835MIPI_Semco_Kantatsu_get_sensor_group_info\n");
#endif
  switch (para->GroupIdx)
  {
  case OV8835MIPI_Semco_Kantatsu_PRE_GAIN:
    sprintf(para->GroupNamePtr, "CCT");
    para->ItemCount = 5;
    break;
  case OV8835MIPI_Semco_Kantatsu_CMMCLK_CURRENT:
    sprintf(para->GroupNamePtr, "CMMCLK Current");
    para->ItemCount = 1;
    break;
  case OV8835MIPI_Semco_Kantatsu_FRAME_RATE_LIMITATION:
    sprintf(para->GroupNamePtr, "Frame Rate Limitation");
    para->ItemCount = 2;
    break;
  case OV8835MIPI_Semco_Kantatsu_REGISTER_EDITOR:
    sprintf(para->GroupNamePtr, "Register Editor");
    para->ItemCount = 2;
    break;
  default:
    ASSERT(0);
  }
}

inline static void OV8835MIPI_Semco_Kantatsu_get_sensor_item_info(MSDK_SENSOR_ITEM_INFO_STRUCT *para)
{

  const static kal_char *cct_item_name[] = {"SENSOR_BASEGAIN", "Pregain-R", "Pregain-Gr", "Pregain-Gb", "Pregain-B"};
  const static kal_char *editer_item_name[] = {"REG addr", "REG value"};
  
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
	 SENSORDB("OV8835MIPI_Semco_Kantatsu_get_sensor_item_info\n");
#endif
  switch (para->GroupIdx)
  {
  case OV8835MIPI_Semco_Kantatsu_PRE_GAIN:
    switch (para->ItemIdx)
    {
    case OV8835MIPI_Semco_Kantatsu_SENSOR_BASEGAIN:
    case OV8835MIPI_Semco_Kantatsu_PRE_GAIN_R_INDEX:
    case OV8835MIPI_Semco_Kantatsu_PRE_GAIN_Gr_INDEX:
    case OV8835MIPI_Semco_Kantatsu_PRE_GAIN_Gb_INDEX:
    case OV8835MIPI_Semco_Kantatsu_PRE_GAIN_B_INDEX:
      break;
    default:
      ASSERT(0);
    }
    sprintf(para->ItemNamePtr, cct_item_name[para->ItemIdx - OV8835MIPI_Semco_Kantatsu_SENSOR_BASEGAIN]);
    para->ItemValue = OV8835MIPI_Semco_Kantatsu_sensor.eng.cct[para->ItemIdx].Para * 1000 / BASEGAIN;
    para->IsTrueFalse = para->IsReadOnly = para->IsNeedRestart = KAL_FALSE;
    para->Min = OV8835MIPI_Semco_Kantatsu_MIN_ANALOG_GAIN * 1000;
    para->Max = OV8835MIPI_Semco_Kantatsu_MAX_ANALOG_GAIN * 1000;
    break;
  case OV8835MIPI_Semco_Kantatsu_CMMCLK_CURRENT:
    switch (para->ItemIdx)
    {
    case 0:
      sprintf(para->ItemNamePtr, "Drv Cur[2,4,6,8]mA");
      switch (OV8835MIPI_Semco_Kantatsu_sensor.eng.reg[OV8835MIPI_Semco_Kantatsu_CMMCLK_CURRENT_INDEX].Para)
      {
      case ISP_DRIVING_2MA:
        para->ItemValue = 2;
        break;
      case ISP_DRIVING_4MA:
        para->ItemValue = 4;
        break;
      case ISP_DRIVING_6MA:
        para->ItemValue = 6;
        break;
      case ISP_DRIVING_8MA:
        para->ItemValue = 8;
        break;
      default:
        ASSERT(0);
      }
      para->IsTrueFalse = para->IsReadOnly = KAL_FALSE;
      para->IsNeedRestart = KAL_TRUE;
      para->Min = 2;
      para->Max = 8;
      break;
    default:
      ASSERT(0);
    }
    break;
  case OV8835MIPI_Semco_Kantatsu_FRAME_RATE_LIMITATION:
    switch (para->ItemIdx)
    {
    case 0:
      sprintf(para->ItemNamePtr, "Max Exposure Lines");
      para->ItemValue = 5998;
      break;
    case 1:
      sprintf(para->ItemNamePtr, "Min Frame Rate");
      para->ItemValue = 5;
      break;
    default:
      ASSERT(0);
    }
    para->IsTrueFalse = para->IsNeedRestart = KAL_FALSE;
    para->IsReadOnly = KAL_TRUE;
    para->Min = para->Max = 0;
    break;
  case OV8835MIPI_Semco_Kantatsu_REGISTER_EDITOR:
    switch (para->ItemIdx)
    {
    case 0:
    case 1:
      sprintf(para->ItemNamePtr, editer_item_name[para->ItemIdx]);
      para->ItemValue = 0;
      para->IsTrueFalse = para->IsReadOnly = para->IsNeedRestart = KAL_FALSE;
      para->Min = 0;
      para->Max = (para->ItemIdx == 0 ? 0xFFFF : 0xFF);
      break;
    default:
      ASSERT(0);
    }
    break;
  default:
    ASSERT(0);
  }
}

inline static kal_bool OV8835MIPI_Semco_Kantatsu_set_sensor_item_info(MSDK_SENSOR_ITEM_INFO_STRUCT *para)
{
  kal_uint16 temp_para;
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
   SENSORDB("OV8835MIPI_Semco_Kantatsu_set_sensor_item_info\n");
#endif
  switch (para->GroupIdx)
  {
  case OV8835MIPI_Semco_Kantatsu_PRE_GAIN:
    switch (para->ItemIdx)
    {
    case OV8835MIPI_Semco_Kantatsu_SENSOR_BASEGAIN:
    case OV8835MIPI_Semco_Kantatsu_PRE_GAIN_R_INDEX:
    case OV8835MIPI_Semco_Kantatsu_PRE_GAIN_Gr_INDEX:
    case OV8835MIPI_Semco_Kantatsu_PRE_GAIN_Gb_INDEX:
    case OV8835MIPI_Semco_Kantatsu_PRE_GAIN_B_INDEX:
	  spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
      OV8835MIPI_Semco_Kantatsu_sensor.eng.cct[para->ItemIdx].Para = para->ItemValue * BASEGAIN / 1000;
	  spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
	  
      OV8835MIPI_Semco_Kantatsu_SetGain(OV8835MIPI_Semco_Kantatsu_sensor.gain); /* update gain */
      break;
    default:
      ASSERT(0);
    }
    break;
  case OV8835MIPI_Semco_Kantatsu_CMMCLK_CURRENT:
    switch (para->ItemIdx)
    {
    case 0:
      switch (para->ItemValue)
      {
      case 2:
        temp_para = ISP_DRIVING_2MA;
        break;
      case 3:
      case 4:
        temp_para = ISP_DRIVING_4MA;
        break;
      case 5:
      case 6:
        temp_para = ISP_DRIVING_6MA;
        break;
      default:
        temp_para = ISP_DRIVING_8MA;
        break;
      }
      //OV8835MIPI_Semco_Kantatsu_set_isp_driving_current((kal_uint16)temp_para);
	  spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
      OV8835MIPI_Semco_Kantatsu_sensor.eng.reg[OV8835MIPI_Semco_Kantatsu_CMMCLK_CURRENT_INDEX].Para = temp_para;
	  spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
      break;
    default:
      ASSERT(0);
    }
    break;
  case OV8835MIPI_Semco_Kantatsu_FRAME_RATE_LIMITATION:
    ASSERT(0);
    break;
  case OV8835MIPI_Semco_Kantatsu_REGISTER_EDITOR:
    switch (para->ItemIdx)
    {
      static kal_uint32 fac_sensor_reg;
    case 0:
      if (para->ItemValue < 0 || para->ItemValue > 0xFFFF) return KAL_FALSE;
      fac_sensor_reg = para->ItemValue;
      break;
    case 1:
      if (para->ItemValue < 0 || para->ItemValue > 0xFF) return KAL_FALSE;
      OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(fac_sensor_reg, para->ItemValue);
      break;
    default:
      ASSERT(0);
    }
    break;
  default:
    ASSERT(0);
  }
  return KAL_TRUE;
}




void OV8835MIPI_Semco_Kantatsu_globle_setting(void)
{
	//OV8835MIPI_Semco_Kantatsu_Global_setting
	//Base_on_OV8835MIPI_Semco_Kantatsu_APP_R1.11
	//2012_2_1
	//Tony Li
	//;;;;;;;;;;;;;Any modify please inform to OV FAE;;;;;;;;;;;;;;;

//#ifdef OV8835MIPI_Semco_Kantatsu_DEBUG_SETTING
#if 0
	//Slave_ID=0x6c;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
	//Slave_ID=0x6c;         


	SENSORDB("OV8830MIPI_globle_setting  start \n");                                                                                                  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0100, 0x00); //; software standby                                                                                     
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0103, 0x01); //; software reset                                                                                       
	//delay(5,m0xs));                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0102, 0x01);                                                                                                          
	//;system, 0xco);ntrol                                                                                                     
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3001, 0x2a); //;drive 2x, d1_frex_in_disable                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3002, 0x88); //; vsync, strobe output enable, href, pclk, frex, SIOD output disable                                   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3005, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x301b, 0xb4); //; sclk_bist20, sclk_snr_sync on                                                                        
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x301d, 0x02); //; frex_mask_arb on                                                                                     
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3021, 0x20); //; internal regulator on                                                                                
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3022, 0x00); //; pad_pixelvdd_sel = 0                                                                                 
	 //  0x    , 0x  );                                                                                                          
	//;0xxpos,u0xre);/Gain                                                                                                     
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3503, 0x07); //; AGC manual, AEC manual                                                                               
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3504, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3505, 0x30);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3506, 0x00); //; ShortExposure[10:16], for inter line HDR mode                                                        
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3508, 0x80); //; ShortExposure[15:8]                                                                                  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3509, 0x10); //; ShortExposure[7:0]                                                                                   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x350a, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x350b, 0x38);                                                                                                          
	//;0xnalo,g0x  );                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3600, 0x78);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3601, 0x0a); //0x02-> 0x0a from tony                                                                                                         
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3602, 0x9c); //0x1c->0x9c from tony                                                                                                         
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3604, 0x38);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3620, 0x64);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3621, 0xb5);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3622, 0x03);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3625, 0x64);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3630, 0x55);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3631, 0xd2);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3632, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3633, 0x34);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3634, 0x03);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3660, 0x80);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3662, 0x10);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3665, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3666, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3667, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x366a, 0x80);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x366c, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x366d, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x366e, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x366f, 0x20);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3680, 0xe0);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3681, 0x00);                                                                                                          
	//;0x sen,s0xor); control                                                                                                  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3701, 0x14);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3702, 0xbf);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3703, 0x8c);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3704, 0x78);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3705, 0x02);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370a, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370b, 0x20);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370c, 0x0c);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370d, 0x11);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370e, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370f, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3710, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x371c, 0x01);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x371f, 0x0c);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3721, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3724, 0x10);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3726, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x372a, 0x01);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3730, 0x18);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3738, 0x22);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3739, 0x08);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x373a, 0x51);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x373b, 0x02);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x373c, 0x20);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x373f, 0x02);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3740, 0x42);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3741, 0x02);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3742, 0x18);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3743, 0x01);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3744, 0x02);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3747, 0x10);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x374c, 0x04);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3751, 0xf0);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3752, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3753, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3754, 0xc0);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3755, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3756, 0x1a);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3758, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3759, 0x0f);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x375c, 0x04);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3767, 0x01);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x376b, 0x44);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3774, 0x10);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3776, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x377f, 0x08);                                                                                                          
	//;0x PSR,A0xM );                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3780, 0x22);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3781, 0x0c);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3784, 0x2c);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3785, 0x1e);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x378f, 0xf5);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3791, 0xb0);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3795, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3796, 0x64);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3797, 0x11);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3798, 0x30);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3799, 0x41);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x379a, 0x07);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x379b, 0xb0);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x379c, 0x0c);                                                                                                          
	//;0x Fre,x0x c);ontrol                                                                                                    
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37c5, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37c6, 0xa0);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37c7, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37c9, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37ca, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37cb, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37cc, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37cd, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37ce, 0x01);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37cf, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37d1, 0x01);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37de, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37df, 0x00);                                                                                                          
	//;0x tim,i0xng);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3823, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3824, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3825, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3826, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3827, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x382a, 0x04);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3a06, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3a07, 0xf8);                                                                                                          
	//;0xstro,b0xe );control                                                                                                   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3b00, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3b02, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3b03, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3b04, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3b05, 0x00);                                                                                                          
	//;0x OTP, 0x  );                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d00, 0x00); //; OTP buffer                                                                                           
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d01, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d02, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d03, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d04, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d05, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d06, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d07, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d08, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d09, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0a, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0b, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0c, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0d, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0e, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0f, 0x00); //; OTP buffer                                                                                           
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d80, 0x00); //; OTP program                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81, 0x00); //; OTP load                                                                                             
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84, 0x00); //; OTP program enable, manual memory bank disable, bank sel 0                                           
	//;0x BLC, 0x  );                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4000, 0x18);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4001, 0x04); //; BLC start line 4                                                                                     
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4002, 0x45); //; BLC auto, reset frame number = 5                                                                     
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4004, 0x02); //; 2 lines for BLC                                                                                      
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4006, 0x16); //; DC BLC coefficient                                                                                   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4008, 0x20); //; First part BLC calculation start line address = 2, DC BLC on                                         
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4009, 0x10); //; BLC target                                                                                           
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4101, 0x12);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4104, 0x5b);                                                                                                          
	//;0xform,a0xt );control                                                                                                   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4307, 0x30); //; embed_line_st = 3                                                                                    
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4315, 0x00); //; Vsync trigger signal to Vsync output delay[15:8]                                                     
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4511, 0x05);                                                                                                          
	//;0xMIPI, 0xco);ntrol                                                                                                     
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4805, 0x01);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4806, 0x00);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x481f, 0x36);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4831, 0x6c);                                                                                                          
	//;0x LVD,S0x  );                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4a00, 0xaa); //; SYNC code enable when only 1 lane, Bit swap, SAV first enable                                        
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4a03, 0x01); //; Dummy data0[7:0]                                                                                     
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4a05, 0x08); //; Dummy data1[7:0]                                                                                     
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4a0a, 0x88);                                                                                                          
	//;0x ISP, 0x  );    
#ifdef OV8835MIPI_Semco_Kantatsu_USE_OTP
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5000, 0x86); //; LENC off, BPC on, WPC on   0x06->0x86 turn on lenc  
#else 
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5000, 0x06); //; LENC off, BPC on, WPC on     
#endif
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5001, 0x01); //; MWB on                                                                                               
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5002, 0x80); //; scale on                                                                                             
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5003, 0x20);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5013, 0x00); //; LSB disable                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5046, 0x4a); //; ISP SOF sel - VSYNC                                                                                  
	//;0x DPC, 0x  );                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5780, 0x1c);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5786, 0x20);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5787, 0x10);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5788, 0x18);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x578a, 0x04);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x578b, 0x02);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x578c, 0x02);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x578e, 0x06);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x578f, 0x02);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5790, 0x02);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5791, 0xff);                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5a08, 0x02); //; window control                                                                                       
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5e00, 0x00); //; color bar off                                                                                        
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5e10, 0x0c);      
#ifdef OV8835MIPI_Semco_Kantatsu_USE_OTP
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5000, 0x86); //; lenc off, bpc on, wpc on    0x06->0x86 turn on lenc  
#else
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5000, 0x06); //; lenc off, bpc on, wpc on   
#endif
	//;0x MWB, 0x  );                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5001, 0x01); //; MWB on                                                                                               
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3400, 0x04); //; red gain h                                                                                           
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3401, 0x00); //; red gain l                                                                                           
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3402, 0x04); //; green gain h                                                                                         
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3403, 0x00); //; green gain l                                                                                         
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3404, 0x04); //; blue gain h                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3405, 0x00); //; blue gain l                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3406, 0x01); //; MWB on                                                                                               
	//;0x BLC, 0x  );                                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4000, 0x18); //; BLC on, BLC window as BLC, BLC ratio from register                                                   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4002, 0x45); //; Format change trigger off, auto on, reset frame number = 5                                           
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4005, 0x18); //; no black line output, blc_man_1_en, BLC after reset, then stop, BLC triggered by gain change         
	//Daniel
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x404f, 0xaf);	//																	   
	//Daniel//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4009, 0x10); //; BLC target                                                                                           
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3503, 0x07); //; AEC manual, AGC manual                                                                               
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3500, 0x00); //; Exposure[19:16]                                                                                      
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3501, 0x29); //; Exposure[15:8]                                                                                       
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3502, 0x00); //; Exposure[7:0]                                                                                        
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x350b, 0x78); //; Gain[7:0]                                                                                            
	//;0x MIP,I0x d);ata rate = 640Mbps                                                                                        
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b3, 0x50); //; pll1_multiplier                                                                                      
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b4, 0x03); //; pll1_prediv                                                                                          
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b5, 0x04); //; pll1_op_pix_div                                                                                      
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b6, 0x01); //; pll1_op_sys_div                                                                                      
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4837, 0x0c); //;0d ; MIPI global timing; //tony      
	
	SENSORDB("OV8830MIPI_globle_setting  end \n");
#else
	//@@OV8835_init
	
	SENSORDB("OV8835MIPI_Semco_Kantatsu_globle_setting  start \n");
#if 0//first test
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0103, 0x01);  //; software reset
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0100, 0x00);  //; software standby

	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0102, 0x01);

	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3001, 0x2a);  //;drive 2x, d1_frex_in_disable
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3002, 0x88);  //; vsync, strobe output enable, href, pclk, frex, SIOD output disable 
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3005, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3011, 0x41);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3015, 0x08);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x301b, 0xb4);  //; sclk_bist20, sclk_snr_sync on		
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x301d, 0x02);  //; frex_mask_arb on	
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3021, 0x00);  //; internal regulator on
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3022, 0x02);  //; pad_pixelvdd_sel = 0		
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3081, 0x02);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3083, 0x01);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3090, 0x03);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3091, 0x23);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3094, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3092, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3093, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3098, 0x04);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3099, 0x10);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x309a, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x309b, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30a2, 0x01);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b0, 0x05);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b2, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b3, 0x64);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b4, 0x03);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b5, 0x04);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b6, 0x01);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3104, 0xa1);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3106, 0x01);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3400, 0x04);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3401, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3402, 0x04);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3403, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3404, 0x04);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3405, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3406, 0x01);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3500, 0x00);


	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3503, 0x07);  //; AGC manual, AEC manual 	 
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3504, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3505, 0x30);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3506, 0x00);  //; ShortExposure[10:16], for inter line HDR mode 
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3508, 0x80);  //; ShortExposure[15:8]  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3509, 0x10);  //; ShortExposure[7:0] 
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x350a, 0x00);
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x350b, 0x38);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x350b, 0x78);//for first enter sensor

	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3600, 0x98);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3601, 0x02);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3602, 0x7c);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3604, 0x38);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3612, 0x80);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3620, 0x41);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3621, 0xa4);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3622, 0x0b);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3625, 0x44);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3630, 0x55);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3631, 0xf2);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3632, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3633, 0x34);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3634, 0x03);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x364d, 0x0d);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x364f, 0x60);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3662, 0x10);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3665, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3666, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3667, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x366a, 0x80);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x366c, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x366d, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x366e, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x366f, 0x20);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3680, 0xb5);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3681, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3701, 0x14);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3702, 0x50);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3703, 0x8c);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3704, 0x68);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3705, 0x02);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370a, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370b, 0x20);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370c, 0x0c);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370d, 0x11);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370e, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370f, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3710, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x371c, 0x01);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x371f, 0x0c);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3721, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3724, 0x10);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3726, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x372a, 0x01);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3730, 0x18);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3738, 0x22);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3739, 0xd0);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x373a, 0x50);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x373b, 0x02);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x373c, 0x20);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x373f, 0x02);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3740, 0x42);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3741, 0x02);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3742, 0x18);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3743, 0x01);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3744, 0x02);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3747, 0x10);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x374c, 0x04);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3751, 0xf0);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3752, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3753, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3754, 0xc0);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3755, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3756, 0x1a);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3758, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3759, 0x0f);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x375c, 0x04);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3767, 0x01);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x376b, 0x44);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3774, 0x10);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3776, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x377f, 0x08);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3780, 0x22);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3781, 0xcc);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3784, 0x2c);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3785, 0x08);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3786, 0x16);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x378f, 0xf5);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3791, 0xb0);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3795, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3796, 0x94);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3797, 0x11);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3798, 0x30);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3799, 0x41);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x379a, 0x07);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x379b, 0xb0);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x379c, 0x0c);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37c5, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37c6, 0xa0);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37c7, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37c9, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37ca, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37cb, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37cc, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37cd, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37ce, 0x01);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37cf, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37d1, 0x01);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37de, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37df, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3810, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3811, 0x04);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3812, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3813, 0x04);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3823, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3824, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3825, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3826, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3827, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x382a, 0x04);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3a06, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3a07, 0xf8);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3b00, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3b02, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3b03, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3b04, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3b05, 0x00);

	//OTP
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d00, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d01, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d02, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d03, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d04, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d05, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d06, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d07, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d08, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d09, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0a, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0b, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0c, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0d, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0e, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0f, 0x00);  //; OTP buffer
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d80, 0x00);  //; OTP program	
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81, 0x00);  //; OTP load		
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84, 0x00);  //; OTP program enable, manual memory bank disable, bank sel 0 			

	//BLC
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4000, 0x18);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4001, 0x04);  //; BLC start line 4		
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4002, 0x45);  //; BLC auto, reset frame number = 5  
//	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4005, 0x18);////for pre and cap diff
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4005, 0x18);////for pre and cap diff
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4006, 0x20);  //; DC BLC coefficient  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4006, 0x1f);  //; DC BLC coefficient   //for pre and cap diff
//	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4008, 0x24);  //; First part BLC calculation start line address = 2, DC BLC on	
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4008, 0x20);  //; First part BLC calculation start line address = 2, DC BLC on	//for pre and cap diff
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4009, 0x10);  //; BLC target
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4100, 0x17);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4101, 0x03);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4102, 0x04);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4103, 0x03);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4104, 0x5a);
//add for pre and cap diff
OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x364f, 0xcf);           
OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3680, 0xe5);           
OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3702, 0xbf);           
OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3704, 0x78);           
OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3739, 0x08);           
OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x373a, 0x51);           
OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3781, 0x0c);           
OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3785, 0x1e);           
OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3796, 0x64);           
OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4008, 0x20);                             
OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4009, 0x10);                             
OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4100, 0x10);                             
OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4101, 0x12);                             
OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4102, 0x24);                             
OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4103, 0x00);                             
OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4104, 0x5b); 

OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4006, 0x1f);
//

	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4307, 0x30);  //; embed_line_st = 3	  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4315, 0x00); //; Vsync trigger signal to Vsync output delay[15:8]	
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4511, 0x05);

	//MIPI control
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4805, 0x21);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4806, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x481f, 0x36);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4831, 0x6c);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4837, 0x0a);

	//LVD
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4a00, 0xaa);  //; SYNC code enable when only 1 lane, Bit swap, SAV first enable
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4a03, 0x01); //; Dummy data0[7:0] 
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4a05, 0x08); //; Dummy data1[7:0]		
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4a0a, 0x88);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4d03, 0xbb);

	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5000, 0x06);


	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5001, 0x01); //; MWB on
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5002, 0x80); //; scale on
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5003, 0x20);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5013, 0x00); //; LSB disable   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5046, 0x4a); //; ISP SOF sel - VSYNC

	//DPC
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5780, 0x1c);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5786, 0x20);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5787, 0x10);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5788, 0x18);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x578a, 0x04);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x578b, 0x02);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x578c, 0x02);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x578e, 0x06);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x578f, 0x02);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5790, 0x02);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5791, 0xff);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5a08, 0x02);  //; window control 	 
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5e00, 0x00);  //; color bar off	
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5e10, 0x0c);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0100, 0x01);
#endif
	//@@OV8835_init_Lisa_15fps

	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0103, 0x01);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0100, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0102, 0x01);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3001, 0x2a);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3002, 0x88);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3005, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3011, 0x41);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3015, 0x08);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x301b, 0xb4);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x301d, 0x02);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3021, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3022, 0x02);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3081, 0x02);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3083, 0x01);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3090, 0x03);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3091, 0x23);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3094, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3092, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3093, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3098, 0x02);//0x04);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3099, 0x13);//0x10);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x309a, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x309b, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30a2, 0x01);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b0, 0x05);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b2, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b3, 0x64);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b4, 0x03);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b5, 0x04);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b6, 0x01);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3104, 0xa1);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3106, 0x01);//
    if(OTP_AWB_flag_semco_kantatsu==false)
    {
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3400, 0x04);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3401, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3402, 0x04);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3403, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3404, 0x04);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3405, 0x00);//
    }
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3406, 0x01);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3500, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3503, 0x07);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3504, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3505, 0x30);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3506, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3508, 0x80);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3509, 0x10);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x350a, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x350b, 0x10);//;38
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3600, 0x98);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3601, 0x02);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3602, 0x7c);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3604, 0x38);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3612, 0x80);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3620, 0x41);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3621, 0xa4);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3622, 0x0f);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3625, 0x44);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3630, 0x55);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3631, 0xf2);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3632, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3633, 0x34);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3634, 0x03);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x364d, 0x0d);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x364f, 0xCF);//;60
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3662, 0x10);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3665, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3666, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3667, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x366a, 0x80);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x366c, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x366d, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x366e, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x366f, 0x20);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3680, 0xE5);//;b5
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3681, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3701, 0x14);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3702, 0xBF);//;50
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3703, 0x8c);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3704, 0x78);//;68
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3705, 0x02);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370a, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370b, 0x20);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370c, 0x0c);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370d, 0x11);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370e, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x370f, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3710, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x371c, 0x01);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x371f, 0x0c);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3721, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3724, 0x10);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3726, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x372a, 0x01);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3730, 0x18);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3738, 0x22);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3739, 0x08);//;d0
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x373a, 0x51);//;50
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x373b, 0x02);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x373c, 0x20);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x373f, 0x02);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3740, 0x42);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3741, 0x02);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3742, 0x18);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3743, 0x01);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3744, 0x02);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3747, 0x10);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x374c, 0x04);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3751, 0xf0);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3752, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3753, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3754, 0xc0);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3755, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3756, 0x1a);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3758, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3759, 0x0f);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x375c, 0x04);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3767, 0x01);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x376b, 0x44);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3774, 0x10);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3776, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x377f, 0x08);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3780, 0x22);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3781, 0x0c);//;cc
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3784, 0x2c);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3785, 0x1E);//;08
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3786, 0x16);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x378f, 0xf5);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3791, 0xb0);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3795, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3796, 0x64);//;94
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3797, 0x11);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3798, 0x30);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3799, 0x41);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x379a, 0x07);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x379b, 0xb0);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x379c, 0x0c);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37c5, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37c6, 0xa0);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37c7, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37c9, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37ca, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37cb, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37cc, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37cd, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37ce, 0x01);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37cf, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37d1, 0x01);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37de, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x37df, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3810, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3811, 0x04);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3812, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3813, 0x04);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3823, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3824, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3825, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3826, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3827, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x382a, 0x04);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3a06, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3a07, 0xf8);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3b00, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3b02, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3b03, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3b04, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3b05, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d00, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d01, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d02, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d03, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d04, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d05, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d06, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d07, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d08, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d09, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0a, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0b, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0c, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0d, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0e, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d0f, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d80, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4000, 0x18);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4001, 0x04);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4002, 0xc5);//0x45);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4005, 0x18);//
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4006, 0x22);//;20
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4006, 0x1f);//;20
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4008, 0x20);//;24
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4009, 0x10);//;10
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4100, 0x10);//;17
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4101, 0x12);//;03
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4102, 0x24);//;04
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4103, 0x00);//;03
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4104, 0x5B);//;5a
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4307, 0x30);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4315, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4511, 0x05);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4805, 0x21);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4806, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x481f, 0x36);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4831, 0x6c);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4837, 0x0a);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4a00, 0xaa);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4a03, 0x01);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4a05, 0x08);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4a0a, 0x88);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4d03, 0xbb);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5000, 0x06);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5001, 0x01);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5002, 0x80);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5003, 0x20);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5013, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5046, 0x4a);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5780, 0x1c);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5786, 0x20);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5787, 0x10);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5788, 0x18);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x578a, 0x04);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x578b, 0x02);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x578c, 0x02);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x578e, 0x06);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x578f, 0x02);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5790, 0x02);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5791, 0xff);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5a08, 0x02);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5e00, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x5e10, 0x0c);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0100, 0x01);//


	SENSORDB("OV8835MIPI_Semco_Kantatsu_globle_setting  end \n");

#endif



                                                                   		
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
OV8835MIPI_Semco_Kantatsu_1632_1224_2Lane_30fps_Mclk26M_setting(void)
{

	//#ifdef OV8835MIPI_Semco_Kantatsu_DEBUG_SETTING		
	#if 0
//	;//OV8835MIPI_Semco_Kantatsu_1600*1200_setting_2lanes_640Mbps/lane_30fps			  
//	;//Base_on_OV8835MIPI_Semco_Kantatsu_APP_R1.11 									  
//	;//2012_2_1 													  
//	;//Tony Li														  
//	;;;;;;;;;;;;;Any modify please inform to OV FAE;;;;;;;;;;;;;;;	  




	SENSORDB("OV8830MIPI_1632_1224_2Lane_30fps_Mclk26M_setting start \n");



	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0100, 0x00);  // ; software standby						  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3708, 0xe2);  // ; sensor control							  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3709, 0x03);  // ; sensor control							  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3800, 0x00);  // ; HS = 8									  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3801, 0x08);  // ; HS										  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3802, 0x00);  // ; VS = 8									  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3803, 0x08);  // ; VS										  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3804, 0x0c);  // ; HE = 3287								  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3805, 0xd7);  // ; HE										  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3806, 0x09);  // ; VE = 2471								  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3807, 0xa7);  // ; VE										  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3808, 0x06);  // ; HO = 1632								  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3809, 0x60);  // ; HO										  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380a, 0x04);  // ; VO = 1224								  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380b, 0xc8);  // ; VO										  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380c, 0x0e);  // ; HTS = 3608								  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380d, 0x18);  // ; HTS 									  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380e, 0x04);  // ; VTS = 1260								  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380f, 0xec);  // ; VTS 									  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3810, 0x00);  // ; H OFFSET = 4							  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3811, 0x04);  // ; H OFFSET								  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3812, 0x00);  // ; V OFFSET = 4							  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3813, 0x04);  // ; V OFFSET								  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3814, 0x31);  // ; X INC									  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3815, 0x31);  // ; Y INC									  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3820, 0x11);  //											  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3821, 0x0f);  //											  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3a04, 0x04);  //											  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3a05, 0xc9);  //			
	
	//Daniel
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4005, 0x18);  // Daniel: For solving color flushing issue											  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x404f, 0xaf);	//																	   
	//Daniel//
//	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4005, 0x1a);  //											  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4512, 0x01);  // ; vertical average	  0x00 for sum					  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3011, 0x21);  // ; MIPI 2 lane, MIPI enable				  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3015, 0xc8);  // ; MIPI 2 lane on, select MIPI 			  
//	;PL0xL	 , 0x  );  //											  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3081, 0x02);  //											  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3083, 0x01);  //											  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3093, 0x00);  // ; pll2_sel5								  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3098, 0x03);  // ; pll3_prediv 							  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3099, 0x1d);  // ;1e; pll3_multiplier//tony				  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x309a, 0x00);  // ; pll3_divs								  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x309b, 0x01);  // ;00; pll3_div//tony						  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30a2, 0x01);  //											  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b0, 0x05);  //											  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b2, 0x00);  //											  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b5, 0x04);  // ; pll1_op_pix_div 						  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b6, 0x02);  // ; pll1_op_sys_div 						  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3104, 0xa1);  // ; sclk_sw2pll 							  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3106, 0x01);  //											  
//	; S0xCLK ,=0x 1);  //38.66Mhz									  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3090, 0x03);  // ; pll2_prediv 							  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3091, 0x20);  // ;23; pll2_multiplier//tony				  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3092, 0x01);  // ; pll2_divs								  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3093, 0x02);  // ;00; pll2_seld5//tony 					  
//	   0x	 , 0x  );  //											  

    //2012-02-27
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b6, 0x01);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3093, 0x01);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b3, 0x3c);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4837, 0x0f);
    //2012-02-27
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0100, 0x01);  // ; wake up

	SENSORDB("OV8830MIPI_1632_1224_2Lane_30fps_Mclk26M_setting end \n");
	
	#else
	//@@Ov8835_1632x1224_2lane_30fps_143MSclk_693Mbps

	//@@Ov8835_1632x1224_2lane_30fps_143MSclk_693Mbps
	
	SENSORDB("OV8835MIPI_Semco_Kantatsu_1632_1224_2Lane_30fps_Mclk26M_setting start \n");
	
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0100, 0x00);
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3501, 0x50);
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3502, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3507, 0x08);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3660, 0x80);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3708, 0xe6);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3709, 0x43);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3800, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3801, 0x08);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3802, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3803, 0x08);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3804, 0x0c);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3805, 0xd7);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3806, 0x09);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3807, 0xa7);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3808, 0x06);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3809, 0x60);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380a, 0x04);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380b, 0xc8);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380c, 0x0e);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380d, 0x18);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380e, 0x05);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380f, 0x28);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3814, 0x31);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3815, 0x31);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3820, 0x11);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3821, 0x0f);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3a04, 0x04);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3a05, 0xc9);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4004, 0x02);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x404f, 0xa0);
#ifdef OV8835_BINNING_SUM
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4512, 0x00);
#else
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4512, 0x01);
#endif

	
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x    , 0x  );
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x    , 0x  ); 										;PLL
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3011, 0x21); //; MIPI 2 lane, MIPI enable
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3015, 0xc8); //; MIPI 2 lane on, select MIPI
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3090, 0x03); //; PLL
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3091, 0x21); //;23 ; PLL
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3091, 0x18); //;23 ; PLL//0x18 104M
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3092, 0x01); //;00 ; PLL _SCLK_143M
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3093, 0x00); //; PLL
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3094, 0x00); //;
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x    , 0x  );
//	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b3, 0x50); //; 64 ; PLL _PLL_MIPICLK_693MBps
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b3, 0x3c); //; 64 ; PLL _PLL_MIPICLK_693MBps
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b4, 0x03); //; PLL
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b5, 0x04); //; PLL
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b6, 0x01); //; PLL
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4837, 0x0f);//0x0c); //; MIPI pclk period
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x    , 0x  );
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0100, 0x01);

	SENSORDB("OV8835MIPI_Semco_Kantatsu_1632_1224_2Lane_30fps_Mclk26M_setting end \n");


	#endif
	
}

OV8835MIPI_Semco_Kantatsu_3264_2448_2Lane_15fps_Mclk26M_setting(void)
{
	
//	;//OV8835MIPI_Semco_Kantatsu_3264*2448_setting_2lanes_690Mbps/lane_15fps									   
//	;//Base_on_OV8835MIPI_Semco_Kantatsu_APP_R1.11 															   
//	;//2012_2_1 																			   
//	;//Tony Li																				   
//	;;;;;;;;;;;;;Any modify please inform to OV FAE;;;;;;;;;;;;;;;	


#if 0//ifdef OV8835MIPI_Semco_Kantatsu_DEBUG_SETTING//1//OV8835MIPI_Semco_Kantatsu_DEBUG_SETTING
	SENSORDB("OV8830MIPI_3264_2448_2Lane_15fps_Mclk26M_setting start \n");
																							   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0100, 0x00);	// ; software standby												   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3708, 0xe2);	// ; sensor control 												   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3709, 0x03);	// ; sensor control 												   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3800, 0x00);	// ; HS = 12														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3801, 0x0c);	// ; HS 															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3802, 0x00);	// ; VS = 12														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3803, 0x0c);	// ; VS 															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3804, 0x0c);	// ; HE = 3283														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3805, 0xd3);	// ; HE 															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3806, 0x09);	// ; VE = 2467														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3807, 0xa3);	// ; VE 															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3808, 0x0c);	// ; HO = 3264														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3809, 0xc0);	// ; HO 															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380a, 0x09);	// ; VO = 2448														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380b, 0x90);	// ; VO 															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380c, 0x0e);	// ; HTS = 3608 													   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380d, 0x18);	// ; HTS															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380e, 0x09);	// ; VTS = 2484 													   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380f, 0xb4);	// ; VTS															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3810, 0x00);	// ; H OFFSET = 4													   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3811, 0x04);	// ; H OFFSET														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3812, 0x00);	// ; V OFFSET = 4													   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3813, 0x04);	// ; V OFFSET														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3814, 0x11);	// ; X INC															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3815, 0x11);	// ; Y INC															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3820, 0x10);	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3821, 0x0e);	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3a04, 0x09);	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3a05, 0xa9);	//																	   
//	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4005, 0x1a);	//																	   
	//Daniel
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4005, 0x18);  // Daniel: For solving color flushing issue											  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x404f, 0xaf);	//																	   
	//Daniel//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4512, 0x01);	// ; vertical average												   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3011, 0x21);	// ; MIPI 2 lane, MIPI enable										   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3015, 0xc8);	// ; MIPI 2 lane on, select MIPI									   
//	;PL0xL	 , 0x  );	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3081, 0x02);	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3083, 0x01);	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3093, 0x00);	// ; pll2_sel5														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3098, 0x03);	// ; pll3_prediv													   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3099, 0x1c);	// ;1e ; pll3_multiplier//Tony										   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x309a, 0x00);	// ; pll3_divs														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x309b, 0x01);	// ;00 ; pll3_div//Tony 											   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30a2, 0x01);	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b0, 0x05);	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b2, 0x00);	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b3, 0x48);	//Tony_for 13fps
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b5, 0x04);	// ; pll1_op_pix_div												   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b6, 0x01);	// ; pll1_op_sys_div												   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3104, 0xa1);	// ; sclk_sw2pll													   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3106, 0x01);	//																	   
//	; S0xCLK ,=0x 1);	//36Mhz 															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3090, 0x03);	// ; pll2_prediv													   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3091, 0x1f);	// ;22 ; pll2_multiplier//Tony										   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3092, 0x01);	// ; pll2_divs														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3093, 0x01);	// ;00 ; pll2_seld5//Tony	
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4837, 0x0d);	// tony_for 13fps	

//	   0x	 , 0x  );	//	

	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0100, 0x01);	// ; wake up	
	
	SENSORDB("OV8830MIPI_3264_2448_2Lane_15fps_Mclk26M_setting end \n");
#else
	//@@Ov8835_3264x2448_2lane_15fps_143MSclk_693Mbps

	SENSORDB("OV8835MIPI_Semco_Kantatsu_3264_2448_2Lane_15fps_Mclk26M_setting start \n");

	//@@Ov8835_3264x2448_2lane_15fps_143MSclk_693Mbps
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0100, 0x00);
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3501, 0xA0);
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3502, 0xC0);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3507, 0x10);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3660, 0x80);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3708, 0xe3);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3709, 0x43);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3800, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3801, 0x0c);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3802, 0x00);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3803, 0x0c);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3804, 0x0c);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3805, 0xd3);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3806, 0x09);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3807, 0xa3);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3808, 0x0c);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3809, 0xc0);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380a, 0x09);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380b, 0x90);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380c, 0x0e);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380d, 0x18);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380e, 0x0A);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380f, 0x52);//0x1A);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3814, 0x11);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3815, 0x11);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3820, 0x10);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3821, 0x0e);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3a04, 0x09);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3a05, 0xa9);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4004, 0x08);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x404f, 0xA0);
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4512, 0x01);
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x    , 0x  );
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x    , 0x  );              ;PLL
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3011, 0x21); //; MIPI 2 lane, MIPI enable
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3015, 0xc8); //; MIPI 2 lane on, select MIPI
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3090, 0x03); //; PLL
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3091, 0x21); //;23 ; PLL
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3091, 0x18); //;23 ; PLL //capture clk
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3091, 0x18); //;23 ; PLL
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3092, 0x01); //; PLL _SCLK_143M
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3093, 0x00); //; PLL
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3094, 0x00); //;
	//OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x    , 0x  );
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b3, 0x50);//0x4d); //; 64 ; PLL _PLL_MIPICLK_800MBps//50:693  //4d:667
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b4, 0x03); //; PLL
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b5, 0x04); //; PLL
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b6, 0x01); //; PLL
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4837, 0x0c); //; MIPI pclk period
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0100, 0x01);
	//                                   , 0x


	SENSORDB("OV8835MIPI_Semco_Kantatsu_3264_2448_2Lane_15fps_Mclk26M_setting end \n");

#endif
	//SENSORDB("OV8835MIPI_Semco_Kantatsu_3264_2448_2Lane_15fps_Mclk26M_setting end \n");
	
}


void OV8835MIPI_Semco_Kantatsu_3264_2448_2Lane_13fps_Mclk26M_setting(void)
{
	
//	;//OV8835MIPI_Semco_Kantatsu_3264*2448_setting_2lanes_690Mbps/lane_15fps									   
//	;//Base_on_OV8835MIPI_Semco_Kantatsu_APP_R1.11 															   
//	;//2012_2_1 																			   
//	;//Tony Li																				   
//	;;;;;;;;;;;;;Any modify please inform to OV FAE;;;;;;;;;;;;;;;	

	SENSORDB("OV8835MIPI_Semco_Kantatsu_3264_2448_2Lane_13fps_Mclk26M_setting start \n");

#if 0
#ifdef OV8835MIPI_Semco_Kantatsu_DEBUG_SETTING
																							   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0100, 0x00);	// ; software standby												   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3708, 0xe2);	// ; sensor control 												   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3709, 0x03);	// ; sensor control 												   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3800, 0x00);	// ; HS = 12														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3801, 0x0c);	// ; HS 															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3802, 0x00);	// ; VS = 12														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3803, 0x0c);	// ; VS 															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3804, 0x0c);	// ; HE = 3283														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3805, 0xd3);	// ; HE 															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3806, 0x09);	// ; VE = 2467														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3807, 0xa3);	// ; VE 															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3808, 0x0c);	// ; HO = 3264														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3809, 0xc0);	// ; HO 															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380a, 0x09);	// ; VO = 2448														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380b, 0x90);	// ; VO 															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380c, 0x0e);	// ; HTS = 3608 													   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380d, 0x18);	// ; HTS															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380e, 0x09);	// ; VTS = 2484 													   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380f, 0xb4);	// ; VTS															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3810, 0x00);	// ; H OFFSET = 4													   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3811, 0x04);	// ; H OFFSET														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3812, 0x00);	// ; V OFFSET = 4													   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3813, 0x04);	// ; V OFFSET														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3814, 0x11);	// ; X INC															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3815, 0x11);	// ; Y INC															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3820, 0x10);	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3821, 0x0e);	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3a04, 0x09);	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3a05, 0xa9);	//																	   
//	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4005, 0x1a);	//																	   
	//Daniel
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4005, 0x18);  // Daniel: For solving color flushing issue											  
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x404f, 0xaf);	//																	   
	//Daniel//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4512, 0x01);	// ; vertical average												   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3011, 0x21);	// ; MIPI 2 lane, MIPI enable										   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3015, 0xc8);	// ; MIPI 2 lane on, select MIPI									   
//	;PL0xL	 , 0x  );	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3081, 0x02);	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3083, 0x01);	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3093, 0x00);	// ; pll2_sel5														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3098, 0x03);	// ; pll3_prediv													   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3099, 0x1c);	// ;1e ; pll3_multiplier//Tony										   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x309a, 0x00);	// ; pll3_divs														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x309b, 0x01);	// ;00 ; pll3_div//Tony 											   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30a2, 0x01);	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b0, 0x05);	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b2, 0x00);	//																	   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b3, 0x40);	//Tony_for 13fps
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b5, 0x04);	// ; pll1_op_pix_div												   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x30b6, 0x01);	// ; pll1_op_sys_div												   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3104, 0xa1);	// ; sclk_sw2pll													   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3106, 0x01);	//																	   
//	; S0xCLK ,=0x 1);	//36Mhz 															   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3090, 0x03);	// ; pll2_prediv													   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3091, 0x1b);	// ;22 ; pll2_multiplier//Tony										   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3092, 0x01);	// ; pll2_divs														   
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3093, 0x01);	// ;00 ; pll2_seld5//Tony	
    OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x4837, 0x0e);	// tony_for 13fps	

	
//	   0x	 , 0x  );	//	



	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0100, 0x01);	// ; wake up	


#else

#endif
#endif
	SENSORDB("OV8835MIPI_Semco_Kantatsu_3264_2448_2Lane_13fps_Mclk26M_setting end \n");
	
}

UINT32 OV8835MIPI_Semco_Kantatsu_Open(void)
{
    kal_uint16 sensor_id=0,ret; 


	//added by mandrave
   int i;
   const kal_uint16 sccb_writeid[] = {OV8835MIPI_Semco_Kantatsu_SLAVE_WRITE_ID_1,OV8835MIPI_Semco_Kantatsu_SLAVE_WRITE_ID_2};

   SENSORDB("OV8835MIPI_Semco_Kantatsu_Open \n");

   spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
   OV8835MIPI_Semco_Kantatsu_sensor.is_zsd = KAL_FALSE;  //for zsd full size preview
   OV8835MIPI_Semco_Kantatsu_sensor.is_zsd_cap = KAL_FALSE;
   OV8835MIPI_Semco_Kantatsu_sensor.is_autofliker = KAL_FALSE; //for autofliker.
   OV8835MIPI_Semco_Kantatsu_sensor.pv_mode = KAL_TRUE;
   spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
   
  for(i = 0; i <(sizeof(sccb_writeid)/sizeof(sccb_writeid[0])); i++)
  	{
  		spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
  	   OV8835MIPI_Semco_Kantatsu_sensor.write_id = sccb_writeid[i];
	   OV8835MIPI_Semco_Kantatsu_sensor.read_id = (sccb_writeid[i]|0x01);
	   spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);

	   sensor_id=((OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x300A) << 8) | OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x300B));	
	   
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
		SENSORDB("OV8835MIPI_Semco_Kantatsu_Open, sensor_id:%x \n",sensor_id);
#endif
		if(OV8835MIPI_SEMCO_KANTATSU_SENSOR_ID == sensor_id)
			{
				SENSORDB("OV8835MIPI slave write id:%x \n",OV8835MIPI_Semco_Kantatsu_sensor.write_id);
				break;
			}
  	}
  
	// check if sensor ID correct		
	if (sensor_id != OV8835MIPI_SEMCO_KANTATSU_SENSOR_ID) 
		{	
			//sensor_id = 0xFFFFFFFF;
//			SENSORDB("sensor_id = 0x%x, OV8835MIPI_SEMCO_KANTATSU_SENSOR_ID = 0x%x \n", sensor_id, OV8835MIPI_SEMCO_KANTATSU_SENSOR_ID);
			return ERROR_SENSOR_CONNECT_FAIL;
		}

	OV8835MIPI_Semco_Kantatsu_globle_setting();
spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
	OV8835MIPI_Semco_Kantatsu_sensor.shutter = 0x500;//init shutter
//<2012/12/10 Clarklin, Reduce load OTP time, about 0.12s
	OV8835MIPI_Semco_Kantatsu_sensor.gain = 0x20;//init gain
	preview_flag_semco_kantatsu = 0;
//>2012/12/10 Clarklin
	spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);


	SENSORDB("test for bootimage \n");
	
   #ifdef OV8835MIPI_Semco_Kantatsu_USE_OTP
   /*
   printk("meimei_OV8835MIPI_Semco_Kantatsu_update_wb_register_from_otp();\n");

    ret = OV8835MIPI_Semco_Kantatsu_update_wb_register_from_otp();
    if(1 == ret)
    {
        printk("meimei_OV8835MIPI_Semco_Kantatsu_update_wb_register_from_otp invalid\n");
    }
    else if(0 == ret)
    {
        printk("meimei_OV8835MIPI_Semco_Kantatsu_update_wb_register_from_otp success\n");
    }
    */
    //printk("meimei_OV8835MIPI_Semco_Kantatsu_update_lenc_register_from_otp();\n");
    ret = ov8835mipi_semco_kantatsuupdate_lenc_register_from_otp();
    if(1 == ret)
    {
        SENSORDB("meimei_OV8835MIPI_Semco_Kantatsu_update_lenc_register_from_otp invalid\n");
    }
    else if(0 == ret)
    {
        //printk("meimei_OV8835MIPI_Semco_Kantatsu_update_lenc_register_from_otp success\n");
    }

    ret = ov8835mipi_semco_kantatsuupdate_wb_register_from_otp();
    if(1 == ret)
    {
        OTP_AWB_flag_semco_kantatsu=false;
        SENSORDB("meimei_OV8835MIPI_Semco_Kantatsu_update_wb_register_from_otp invalid\n");
    }
    else if(0 == ret)
    {
        OTP_AWB_flag_semco_kantatsu=true;
        //printk("meimei_OV8830MIPI_update_wb_register_from_otp success\n");
    }
    ov8835mipi_semco_kantatsuupdate_blc_register_from_otp();
    ov8835mipi_semco_kantatsuupdate_af_from_otp();
#endif
   return ERROR_NONE;
}   /* OV8835MIPI_Semco_Kantatsu_Open  */

kal_uint16 OV8835mipi_Semco_Kantatsu_read_otp_Module_Lens_ID(void)
{
	kal_uint16 Module_ID = 0, Lens_ID = 0;
//	kal_uint32 address;
//	kal_uint16 index;

	if(ov8835mipi_semco_kantatsucheck_otp_Index())
//<2013/01/29-ClarkLin-Set default camera is Semco_Kantatsu
	{
		SENSORDB("NO OTP data, load default camera driver !! \n");
		return KAL_TRUE;
	}
//>2013/01/29-ClarkLin
	//select bank 
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d84, 0xc0+1+OTP_index_semco_kantatsu*7);
	//load otp to buffer
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81, 0x01);
	msleep(OTP_DelayTime_semco_kantatsu);
	//disable otp read
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x3d81, 0x00);
	//read lenc

//	address = 0x3d09;
	Lens_ID = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3d09);
//	address = 0x3d0a;
	Module_ID = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3d0a);
SENSORDB("Module_ID = 0x%x, Lens_ID = 0x%x \n", Module_ID, Lens_ID);

if ((Module_ID == 0x18) && (Lens_ID == 0x0B))	// semco + kantatsu
	return KAL_TRUE;
else
	return KAL_FALSE;
}
/*************************************************************************
* FUNCTION
*   OV5642GetSensorID
*
* DESCRIPTION
*   This function get the sensor ID 
*
* PARAMETERS
*   *sensorID : return the sensor ID 
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV8835MIPI_Semco_Kantatsu_GetSensorID(UINT32 *sensorID) 
{
  //added by mandrave
   int i;
   const kal_uint16 sccb_writeid[] = {OV8835MIPI_Semco_Kantatsu_SLAVE_WRITE_ID_1,OV8835MIPI_Semco_Kantatsu_SLAVE_WRITE_ID_2};
 

  for(i = 0; i <(sizeof(sccb_writeid)/sizeof(sccb_writeid[0])); i++)
  	{
  		spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
  	   OV8835MIPI_Semco_Kantatsu_sensor.write_id = sccb_writeid[i];
	   OV8835MIPI_Semco_Kantatsu_sensor.read_id = (sccb_writeid[i]|0x01);
	   spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);

	   *sensorID=((OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x300A) << 8) | OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x300B));	
	   
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
		SENSORDB("sensor_id:%x \n",*sensorID);
#endif
		if(OV8835MIPI_SEMCO_KANTATSU_SENSOR_ID == *sensorID)
			{
				SENSORDB("OV8835MIPI slave write id:%x \n",OV8835MIPI_Semco_Kantatsu_sensor.write_id);
				break;
			}
  	}
  
	// check if sensor ID correct		
	if (*sensorID != OV8835MIPI_SEMCO_KANTATSU_SENSOR_ID) 
		{	
			*sensorID = 0xFFFFFFFF;
			return ERROR_SENSOR_CONNECT_FAIL;
		}
	
	SENSORDB("Sensor Read ID OK!\n");

#ifdef OTP_COMBINATION_Semco_Kantatsu
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0103, 0x01);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0100, 0x00);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0102, 0x01);//
	OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x0100, 0x01);//

	if(!OV8835mipi_Semco_Kantatsu_read_otp_Module_Lens_ID())
	{
		*sensorID = 0xFFFFFFFF;
		SENSORDB("OTP fail \n");
		return ERROR_SENSOR_CONNECT_FAIL;
	}
	else
		SENSORDB("SEMCO + KANTATSU OK!!\n");

#endif
   return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*	OV8835MIPI_Semco_Kantatsu_Close
*
* DESCRIPTION
*	This function is to turn off sensor module power.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV8835MIPI_Semco_Kantatsu_Close(void)
{
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
   SENSORDB("OV8835MIPI_Semco_Kantatsu_Close\n");
#endif
  //CISModulePowerOn(FALSE);
//	DRV_I2CClose(OV8835MIPIhDrvI2C);
	return ERROR_NONE;
}   /* OV8835MIPI_Semco_Kantatsu_Close */

/*************************************************************************
* FUNCTION
* OV8835MIPI_Semco_Kantatsu_Preview
*
* DESCRIPTION
*	This function start the sensor preview.
*
* PARAMETERS
*	*image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV8835MIPI_Semco_Kantatsu_Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	//kal_uint16 dummy_line;
	kal_uint16 ret;
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
	SENSORDB("OV8835MIPI_Semco_Kantatsu_Preview setting\n");
#endif
	//OV8835MIPI_Semco_Kantatsu_Sensor_1M();
	//OV8835MIPI_Semco_Kantatsu_globle_setting();

    spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
	OV8835MIPI_Semco_Kantatsu_sensor.pv_mode = KAL_TRUE;
	spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);

	OV8835MIPI_Semco_Kantatsu_1632_1224_2Lane_30fps_Mclk26M_setting();
#if 0//norman@20121015
    #ifdef OV8835MIPI_Semco_Kantatsu_USE_OTP
			   
		ret = ov8835mipi_semco_kantatsuupdate_wb_register_from_otp();
				if(1 == ret)
					{
				         SENSORDB("ov8835mipi_semco_kantatsuupdate_wb_register_from_otp invalid\n");
					}
				else if(0 == ret)
					{
						 SENSORDB("ov8835mipi_semco_kantatsuupdate_wb_register_from_otp success\n");
					}
			   
		ret = ov8835mipi_semco_kantatsuupdate_lenc_register_from_otp();
				 if(1 == ret)
					{
						 SENSORDB("ov8835mipi_semco_kantatsuupdate_lenc_register_from_otp invalid\n");
					}
				 else if(0 == ret)
					{
						 SENSORDB("ov8835mipi_semco_kantatsuupdate_lenc_register_from_otp success\n");
					}
		ret = ov8835mipi_semco_kantatsuupdate_blc_register_from_otp();
				 if(1 == ret)
					{
						 SENSORDB("ov8835mipi_semco_kantatsuupdate_blc_register_from_otp invalid\n");
					}
				 else if(0 == ret)
					{
						 SENSORDB("ov8835mipi_semco_kantatsuupdate_blc_register_from_otp success\n");
					}
	#endif
#endif//norman@20121015
    //msleep(30);
	
	//OV8835MIPI_Semco_Kantatsu_set_mirror(sensor_config_data->SensorImageMirror);
	switch (sensor_config_data->SensorOperationMode)
	{
	  case MSDK_SENSOR_OPERATION_MODE_VIDEO: 
	  	spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
		OV8835MIPI_Semco_Kantatsu_sensor.video_mode = KAL_TRUE;		
		OV8835MIPI_Semco_Kantatsu_sensor.normal_fps = OV8835MIPI_Semco_Kantatsu_FPS(30);
		OV8835MIPI_Semco_Kantatsu_sensor.night_fps = OV8835MIPI_Semco_Kantatsu_FPS(15);
		spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
		//dummy_line = 0;
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
		SENSORDB("Video mode \n");
#endif
	   break;
	  default: /* ISP_PREVIEW_MODE */
	  	spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
		OV8835MIPI_Semco_Kantatsu_sensor.video_mode = KAL_FALSE;
		spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
		//dummy_line = 0;
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
		SENSORDB("Camera preview mode \n");
#endif
	  break;
	}

	spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
	OV8835MIPI_Semco_Kantatsu_sensor.dummy_pixels = 0;
	OV8835MIPI_Semco_Kantatsu_sensor.dummy_lines = 0;
	spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
	
    OV8835MIPI_Semco_Kantatsu_Write_Shutter(OV8835MIPI_Semco_Kantatsu_sensor.shutter);
	OV8835MIPI_Semco_Kantatsu_Set_Dummy(OV8835MIPI_Semco_Kantatsu_sensor.dummy_pixels, OV8835MIPI_Semco_Kantatsu_sensor.dummy_lines); /* modify dummy_pixel must gen AE table again */
	
	mdelay(40);
	return ERROR_NONE;
	
}   /*  OV8835MIPI_Semco_Kantatsu_Preview   */

/*************************************************************************
* FUNCTION
*	OV8835MIPI_Semco_Kantatsu_Capture
*
* DESCRIPTION
*	This function setup the CMOS sensor in capture MY_OUTPUT mode
*
* PARAMETERS
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 OV8835MIPI_Semco_Kantatsu_Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
						  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	const kal_uint16 pv_line_length = OV8835MIPI_Semco_Kantatsu_sensor.line_length;
	const kal_uint32 pv_pclk = OV8835MIPI_Semco_Kantatsu_sensor.pv_pclk;
	const kal_uint32 cap_pclk = OV8835MIPI_Semco_Kantatsu_sensor.cap_pclk;
	kal_uint32 shutter = OV8835MIPI_Semco_Kantatsu_sensor.shutter;
	kal_uint16 dummy_pixel;
	//kal_uint32 temp;
	kal_uint16 ret;
   //kal_uint16 temp,temp1,temp2,temp3,temp4,temp5;
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
	SENSORDB("OV8835MIPI_Semco_Kantatsu_Capture setting start \n");
#endif
	//if((OV8835MIPI_Semco_Kantatsu_sensor.pv_mode == KAL_TRUE)||(OV8835MIPI_Semco_Kantatsu_sensor.is_zsd == KAL_TRUE))
	//if(OV8835MIPI_Semco_Kantatsu_sensor.pv_mode == KAL_TRUE)
	{
		// 
		spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
		OV8835MIPI_Semco_Kantatsu_sensor.video_mode = KAL_FALSE;
		OV8835MIPI_Semco_Kantatsu_sensor.is_autofliker = KAL_FALSE;
		spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
		
		if(OV8835MIPI_Semco_Kantatsu_sensor.is_zsd == KAL_TRUE)
			{
				spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
				OV8835MIPI_Semco_Kantatsu_sensor.pv_mode = KAL_FALSE;
				spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);

				//OV8835MIPI_Semco_Kantatsu_3264_2448_2Lane_13fps_Mclk26M_setting();
				OV8835MIPI_Semco_Kantatsu_3264_2448_2Lane_15fps_Mclk26M_setting();

				spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
				OV8835MIPI_Semco_Kantatsu_sensor.dummy_pixels = 0;
				OV8835MIPI_Semco_Kantatsu_sensor.dummy_lines = 0;
				spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);

				#if 0//def OV8835MIPI_Semco_Kantatsu_USE_OTP			   
					ret = ov8835mipi_semco_kantatsuupdate_wb_register_from_otp();
				   if(1 == ret)
					   {
						   SENSORDB("ov8835mipi_semco_kantatsuupdate_wb_register_from_otp invalid\n");
					   }
				   else if(0 == ret)
					   {
						   SENSORDB("ov8835mipi_semco_kantatsuupdate_wb_register_from_otp success\n");
					   }
			   
				   ret = ov8835mipi_semco_kantatsuupdate_lenc_register_from_otp();
				   if(1 == ret)
					   {
						   SENSORDB("ov8835mipi_semco_kantatsuupdate_lenc_register_from_otp invalid\n");
					   }
				   else if(0 == ret)
					   {
						   SENSORDB("ov8835mipi_semco_kantatsuupdate_lenc_register_from_otp success\n");
					   }
				#endif
				
//<2013/01/29-ClarkLin-modify flicker sensitive
				OV8835MIPI_Semco_Kantatsu_Set_Dummy(OV8835MIPI_Semco_Kantatsu_sensor.dummy_pixels, OV8835MIPI_Semco_Kantatsu_sensor.dummy_lines+10);
//>2013/01/29-ClarkLin
			   
			}
		else
			{
				spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
				OV8835MIPI_Semco_Kantatsu_sensor.pv_mode = KAL_FALSE;
				spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
				
				//if(OV8835MIPI_Semco_Kantatsu_sensor.pv_mode == KAL_TRUE)
				OV8835MIPI_Semco_Kantatsu_3264_2448_2Lane_15fps_Mclk26M_setting();

				spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
				OV8835MIPI_Semco_Kantatsu_sensor.dummy_pixels = 0;
				OV8835MIPI_Semco_Kantatsu_sensor.dummy_lines = 0;
				spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);

				//cap_fps = OV8835MIPI_Semco_Kantatsu_FPS(8);
				SENSORDB("OV8835MIPI_Semco_Kantatsu_FPS 15 \n");

			    //dummy_pixel=0;
				OV8835MIPI_Semco_Kantatsu_Set_Dummy(OV8835MIPI_Semco_Kantatsu_sensor.dummy_pixels, OV8835MIPI_Semco_Kantatsu_sensor.dummy_lines);

			
						//dummy_pixel = OV8835MIPI_Semco_Kantatsu_sensor.cap_pclk * OV8835MIPI_Semco_Kantatsu_FPS(1) / (OV8835MIPI_Semco_Kantatsu_FULL_PERIOD_LINE_NUMS * cap_fps);
						//dummy_pixel = dummy_pixel < OV8835MIPI_Semco_Kantatsu_FULL_PERIOD_PIXEL_NUMS ? 0 : dummy_pixel - OV8835MIPI_Semco_Kantatsu_FULL_PERIOD_PIXEL_NUMS;

						//OV8835MIPI_Semco_Kantatsu_Set_Dummy(dummy_pixel, 0);
						
				/* shutter translation */
				//shutter = shutter * pv_line_length / OV8835MIPI_Semco_Kantatsu_sensor.line_length;
				SENSORDB("pv shutter %d\n",shutter);
				SENSORDB("cap pclk %d\n",cap_pclk);
				SENSORDB("pv pclk %d\n",pv_pclk);
				SENSORDB("pv line length %d\n",pv_line_length);
				SENSORDB("cap line length %d\n",OV8835MIPI_Semco_Kantatsu_sensor.line_length);

				//shutter = shutter * (cap_pclk / pv_pclk);
				//SENSORDB("pv shutter %d\n",shutter);
				//shutter = shutter * pv_line_length / OV8835MIPI_Semco_Kantatsu_sensor.line_length;
				//SENSORDB("pv shutter %d\n",shutter);
				shutter = ((cap_pclk / 1000) * shutter) / (pv_pclk / 1000);
				SENSORDB("pv shutter %d\n",shutter);
				#ifdef OV8835_BINNING_SUM
				shutter = 2*(shutter * pv_line_length) /OV8835MIPI_Semco_Kantatsu_sensor.line_length*94/100;//*101/107;
				#else
				shutter = (shutter * pv_line_length) /OV8835MIPI_Semco_Kantatsu_sensor.line_length;
				#endif
				SENSORDB("cp shutter %d\n",shutter);
				
				//shutter *= 2;
				//shutter = ( shutter * cap_pclk * pv_line_length) / (pv_pclk * OV8835MIPI_Semco_Kantatsu_sensor.line_length);
				OV8835MIPI_Semco_Kantatsu_Write_Shutter(shutter);
				//set_OV8835MIPI_Semco_Kantatsu_shutter(shutter);
			}
		
		//OV8835MIPI_Semco_Kantatsu_Set_Dummy(OV8835MIPI_Semco_Kantatsu_sensor.dummy_pixels, OV8835MIPI_Semco_Kantatsu_sensor.dummy_lines);
			
		//mdelay(80);
	}
	mdelay(50);

#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
		SENSORDB("OV8835MIPI_Semco_Kantatsu_Capture end\n");
#endif
/*
    temp=OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3400);
    temp1=OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3401);
    temp2=OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3402);
    temp3=OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3403);
    temp4=OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3404);
    temp5=OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(0x3405);
    SENSORDB("[ov8835mipi_semco_kantatsuread_wb_gain]R_gain[%x]G_gain[%x]B_gain[%x]\n",(temp<<8)|temp1,(temp2<<8)|temp3,(temp4<<8)|temp5);
*/
	return ERROR_NONE;
}   /* OV8835MIPI_Semco_Kantatsu_Capture() */

UINT32 OV8835MIPI_Semco_Kantatsu_GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
		SENSORDB("OV8835MIPI_Semco_Kantatsu_GetResolution \n");
#endif		
//#ifdef ACDK
//	pSensorResolution->SensorFullWidth=OV8835MIPI_Semco_Kantatsu_IMAGE_SENSOR_PV_WIDTH;
//	pSensorResolution->SensorFullHeight=OV8835MIPI_Semco_Kantatsu_IMAGE_SENSOR_PV_HEIGHT;
//#else
	pSensorResolution->SensorFullWidth=OV8835MIPI_Semco_Kantatsu_IMAGE_SENSOR_FULL_WIDTH;
	pSensorResolution->SensorFullHeight=OV8835MIPI_Semco_Kantatsu_IMAGE_SENSOR_FULL_HEIGHT;
//#endif

	pSensorResolution->SensorPreviewWidth=OV8835MIPI_Semco_Kantatsu_IMAGE_SENSOR_PV_WIDTH;
	pSensorResolution->SensorPreviewHeight=OV8835MIPI_Semco_Kantatsu_IMAGE_SENSOR_PV_HEIGHT;
	return ERROR_NONE;
}	/* OV8835MIPI_Semco_Kantatsu_GetResolution() */

UINT32 OV8835MIPI_Semco_Kantatsu_GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
					  MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
	SENSORDB("OV8835MIPIGetInfo£¬FeatureId:%d\n",ScenarioId);
#endif
#if 1
	switch(ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			pSensorInfo->SensorPreviewResolutionX=OV8835MIPI_Semco_Kantatsu_IMAGE_SENSOR_FULL_WIDTH;
			pSensorInfo->SensorPreviewResolutionY=OV8835MIPI_Semco_Kantatsu_IMAGE_SENSOR_FULL_HEIGHT;
			pSensorInfo->SensorCameraPreviewFrameRate = 15;
		break;
		default:
  	        pSensorInfo->SensorPreviewResolutionX=OV8835MIPI_Semco_Kantatsu_IMAGE_SENSOR_PV_WIDTH;
			pSensorInfo->SensorPreviewResolutionY=OV8835MIPI_Semco_Kantatsu_IMAGE_SENSOR_PV_HEIGHT;
			pSensorInfo->SensorCameraPreviewFrameRate = 30;
		break;
	}

	//pSensorInfo->SensorPreviewResolutionX=OV8835MIPI_Semco_Kantatsu_IMAGE_SENSOR_PV_WIDTH;
	//pSensorInfo->SensorPreviewResolutionY=OV8835MIPI_Semco_Kantatsu_IMAGE_SENSOR_PV_HEIGHT;
	pSensorInfo->SensorFullResolutionX=OV8835MIPI_Semco_Kantatsu_IMAGE_SENSOR_FULL_WIDTH;
	pSensorInfo->SensorFullResolutionY=OV8835MIPI_Semco_Kantatsu_IMAGE_SENSOR_FULL_HEIGHT;

	//pSensorInfo->SensorCameraPreviewFrameRate=30;
	pSensorInfo->SensorVideoFrameRate=30;
	pSensorInfo->SensorStillCaptureFrameRate=10;
	pSensorInfo->SensorWebCamCaptureFrameRate=15;
	pSensorInfo->SensorResetActiveHigh=FALSE; //low active
	pSensorInfo->SensorResetDelayCount=5; 
#endif
	pSensorInfo->SensorOutputDataFormat=OV8835MIPI_Semco_Kantatsu_COLOR_FORMAT;
	pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;	
	pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
#if 1
	pSensorInfo->SensorInterruptDelayLines = 4;
	
	//#ifdef MIPI_INTERFACE
   		pSensorInfo->SensroInterfaceType        = SENSOR_INTERFACE_TYPE_MIPI;
   	//#else
   	//	pSensorInfo->SensroInterfaceType		= SENSOR_INTERFACE_TYPE_PARALLEL;
   	//#endif

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxWidth=CAM_SIZE_1M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxHeight=CAM_SIZE_1M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].BinningEnable=TRUE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxWidth=CAM_SIZE_1M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxHeight=CAM_SIZE_1M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].BinningEnable=TRUE;
#endif
	pSensorInfo->CaptureDelayFrame = 2; 
	//pSensorInfo->PreviewDelayFrame = 3; 
	pSensorInfo->PreviewDelayFrame = 2; 
	pSensorInfo->VideoDelayFrame = 1; 	

	pSensorInfo->SensorMasterClockSwitch = 0; 
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_6MA;
    pSensorInfo->AEShutDelayFrame = 0;		   /* The frame of setting shutter default 0 for TG int */
	pSensorInfo->AESensorGainDelayFrame = 0;	   /* The frame of setting sensor gain */
	pSensorInfo->AEISPGainDelayFrame = 2;    
	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
			pSensorInfo->SensorClockFreq=26;
			pSensorInfo->SensorClockDividCount=	3;
			pSensorInfo->SensorClockRisingCount= 0;
			pSensorInfo->SensorClockFallingCount= 2;
			pSensorInfo->SensorPixelClockCount= 3;
			pSensorInfo->SensorDataLatchCount= 2;
			pSensorInfo->SensorGrabStartX = OV8835MIPI_Semco_Kantatsu_PV_X_START; 
			pSensorInfo->SensorGrabStartY = OV8835MIPI_Semco_Kantatsu_PV_Y_START; 
			
				
	            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;			
	            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
		        pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
		        pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
	            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
	            pSensorInfo->SensorPacketECCOrder = 1;
	        	


		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			pSensorInfo->SensorClockFreq=26;
			pSensorInfo->SensorClockDividCount= 3;
			pSensorInfo->SensorClockRisingCount=0;
			pSensorInfo->SensorClockFallingCount=2;
			pSensorInfo->SensorPixelClockCount=3;
			pSensorInfo->SensorDataLatchCount=2;
			pSensorInfo->SensorGrabStartX = OV8835MIPI_Semco_Kantatsu_FULL_X_START; 
			pSensorInfo->SensorGrabStartY = OV8835MIPI_Semco_Kantatsu_FULL_Y_START; 
			
			   			
	            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;			
	            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
		        pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
		        pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
	            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
	            pSensorInfo->SensorPacketECCOrder = 1;
	      

		break;
		default:
			pSensorInfo->SensorClockFreq=26;
			pSensorInfo->SensorClockDividCount=3;
			pSensorInfo->SensorClockRisingCount=0;
			pSensorInfo->SensorClockFallingCount=2;		
			pSensorInfo->SensorPixelClockCount=3;
			pSensorInfo->SensorDataLatchCount=2;
			pSensorInfo->SensorGrabStartX = OV8835MIPI_Semco_Kantatsu_PV_X_START; 
			pSensorInfo->SensorGrabStartY = OV8835MIPI_Semco_Kantatsu_PV_Y_START; 
		break;
	}
#if 0
	//OV8835MIPIPixelClockDivider=pSensorInfo->SensorPixelClockCount;
	memcpy(pSensorConfigData, &OV8835MIPISensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
#endif		
  return ERROR_NONE;
}	/* OV8835MIPI_Semco_Kantatsu_GetInfo() */
//>2012/11/28 alberthsiao-17860

UINT32 OV8835MIPI_Semco_Kantatsu_Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
	SENSORDB("OV8835MIPIControl£¬FeatureId:%d\n",ScenarioId);
#endif	

	spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
   OV8835MIPIRAW_Semco_Kantatsu_CurrentScenarioId = ScenarioId;
   spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);

	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
			
			OV8835MIPI_Semco_Kantatsu_Preview(pImageWindow, pSensorConfigData);
		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
			if(OV8835MIPI_Semco_Kantatsu_sensor.is_zsd == KAL_TRUE)
				{
					spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
					OV8835MIPI_Semco_Kantatsu_sensor.is_zsd_cap = KAL_TRUE;
					spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
					SENSORDB("OV8835MIPIControl£¬is_zsd_cap is true!\n");
				}
			else
				{
					spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
					OV8835MIPI_Semco_Kantatsu_sensor.is_zsd_cap = KAL_FALSE;
					spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
					SENSORDB("OV8835MIPIControl£¬is_zsd_cap is false!\n");
				}
			
			OV8835MIPI_Semco_Kantatsu_Capture(pImageWindow, pSensorConfigData);
			break;
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
			OV8835MIPI_Semco_Kantatsu_sensor.is_zsd = KAL_TRUE;  //for zsd full size preview
			OV8835MIPI_Semco_Kantatsu_sensor.is_zsd_cap = KAL_FALSE;
			spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
			OV8835MIPI_Semco_Kantatsu_Capture(pImageWindow, pSensorConfigData);
		break;		
        default:
            return ERROR_INVALID_SCENARIO_ID;
	}
	return TRUE;
}	/* OV8835MIPI_Semco_Kantatsu_Control() */

UINT32 OV8835MIPI_Semco_Kantatsu_SetAutoFlickerMode(kal_bool bEnable, UINT16 u2FrameRate)
{
	
	//kal_uint32 pv_max_frame_rate_lines = OV8835MIPI_Semco_Kantatsu_sensor.dummy_lines;
	
	SENSORDB("[OV8835MIPI_Semco_Kantatsu_SetAutoFlickerMode] frame rate(10base) = %d %d\n", bEnable, u2FrameRate);

	if(bEnable)
		{
		    
			spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
			OV8835MIPI_Semco_Kantatsu_sensor.is_autofliker = KAL_TRUE;
			spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);

			//if(OV8835MIPI_Semco_Kantatsu_sensor.video_mode = KAL_TRUE)
			//	{
			//		pv_max_frame_rate_lines = OV8835MIPI_Semco_Kantatsu_sensor.frame_height + (OV8835MIPI_Semco_Kantatsu_sensor.frame_height>>7);
			//		OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380e, (pv_max_frame_rate_lines>>8)&0xFF);
			//		OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380f, (pv_max_frame_rate_lines)&0xFF);
			//	}	
		}
	else
		{
			spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
			OV8835MIPI_Semco_Kantatsu_sensor.is_autofliker = KAL_FALSE;
			spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
			
			//if(OV8835MIPI_Semco_Kantatsu_sensor.video_mode = KAL_TRUE)
			//	{
			//		pv_max_frame_rate_lines = OV8835MIPI_Semco_Kantatsu_sensor.frame_height;
			//		OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380e, (pv_max_frame_rate_lines>>8)&0xFF);
			//		OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(0x380f, (pv_max_frame_rate_lines)&0xFF);
			//	}	
		}
	SENSORDB("[OV8835MIPI_Semco_Kantatsu_SetAutoFlickerMode]bEnable:%x \n",bEnable);
	return 0;
}


UINT32 OV8835MIPI_Semco_Kantatsu_SetVideoMode(UINT16 u2FrameRate)
{
	kal_int16 dummy_line;
    /* to fix VSYNC, to fix frame rate */
#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
	SENSORDB("OV8835MIPI_Semco_Kantatsu_SetVideoMode, u2FrameRate:%d\n",u2FrameRate);
#endif	


    if((30 == u2FrameRate)||(15 == u2FrameRate))
    	{
//<2012/12/10 Clarklin, Reduce load OTP time, about 0.12s
			if( OV8835MIPIRAW_Semco_Kantatsu_CurrentScenarioId == MSDK_SCENARIO_ID_CAMERA_ZSD)
				dummy_line = OV8835MIPI_Semco_Kantatsu_sensor.cap_pclk / u2FrameRate / OV8835MIPI_Semco_Kantatsu_FULL_PERIOD_PIXEL_NUMS - OV8835MIPI_Semco_Kantatsu_FULL_PERIOD_LINE_NUMS;
			else
//>2012/12/10 Clarklin
			dummy_line = OV8835MIPI_Semco_Kantatsu_sensor.pv_pclk / u2FrameRate / OV8835MIPI_Semco_Kantatsu_PV_PERIOD_PIXEL_NUMS - OV8835MIPI_Semco_Kantatsu_PV_PERIOD_LINE_NUMS;
			if (dummy_line < 0) dummy_line = 0;
         #ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
			SENSORDB("dummy_line %d\n", dummy_line);
         #endif
			OV8835MIPI_Semco_Kantatsu_Set_Dummy(0, dummy_line); /* modify dummy_pixel must gen AE table again */
		 
			//OV8835MIPI_Semco_Kantatsu_Set_Dummy(0, 0); /* modify dummy_pixel must gen AE table again */
		 	spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
			OV8835MIPI_Semco_Kantatsu_sensor.video_mode = KAL_TRUE;
			spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
    	}
	else if(0 == u2FrameRate)
		{
			spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
			OV8835MIPI_Semco_Kantatsu_sensor.video_mode = KAL_FALSE;
			spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
			
			SENSORDB("disable video mode\n");
		}
    return TRUE;
}


UINT32 OV8835MIPI_Semco_Kantatsu_FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
							 UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
	UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
	UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
	UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
	UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
	UINT32 OV8835MIPISensorRegNumber;
	UINT32 i;
	//PNVRAM_SENSOR_DATA_STRUCT pSensorDefaultData=(PNVRAM_SENSOR_DATA_STRUCT) pFeaturePara;
	//MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
	MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
	//MSDK_SENSOR_GROUP_INFO_STRUCT *pSensorGroupInfo=(MSDK_SENSOR_GROUP_INFO_STRUCT *) pFeaturePara;
	//MSDK_SENSOR_ITEM_INFO_STRUCT *pSensorItemInfo=(MSDK_SENSOR_ITEM_INFO_STRUCT *) pFeaturePara;
	//MSDK_SENSOR_ENG_INFO_STRUCT	*pSensorEngInfo=(MSDK_SENSOR_ENG_INFO_STRUCT *) pFeaturePara;

#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
	//SENSORDB("OV8835MIPIFeatureControl£¬FeatureId:%d\n",FeatureId); 
#endif		
	switch (FeatureId)
	{
		case SENSOR_FEATURE_GET_RESOLUTION:
			*pFeatureReturnPara16++=OV8835MIPI_Semco_Kantatsu_IMAGE_SENSOR_FULL_WIDTH;
			*pFeatureReturnPara16=OV8835MIPI_Semco_Kantatsu_IMAGE_SENSOR_FULL_HEIGHT;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_GET_PERIOD:	/* 3 */
			switch(OV8835MIPIRAW_Semco_Kantatsu_CurrentScenarioId)
				{
					case MSDK_SCENARIO_ID_CAMERA_ZSD:
						*pFeatureReturnPara16++= OV8835MIPI_Semco_Kantatsu_FULL_PERIOD_PIXEL_NUMS + OV8835MIPI_Semco_Kantatsu_sensor.dummy_pixels;
						*pFeatureReturnPara16= OV8835MIPI_Semco_Kantatsu_FULL_PERIOD_LINE_NUMS + OV8835MIPI_Semco_Kantatsu_sensor.dummy_lines;
						*pFeatureParaLen=4;
			            #ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
				          SENSORDB("SENSOR_FEATURE_GET_PERIOD£¬OV8835MIPI cap line length:%d\n",OV8835MIPI_Semco_Kantatsu_FULL_PERIOD_PIXEL_NUMS + OV8835MIPI_Semco_Kantatsu_sensor.dummy_pixels); 
			            #endif	
						break;
					default:
						*pFeatureReturnPara16++= OV8835MIPI_Semco_Kantatsu_PV_PERIOD_PIXEL_NUMS + OV8835MIPI_Semco_Kantatsu_sensor.dummy_pixels;
						*pFeatureReturnPara16= OV8835MIPI_Semco_Kantatsu_PV_PERIOD_LINE_NUMS + OV8835MIPI_Semco_Kantatsu_sensor.dummy_lines;
						*pFeatureParaLen=4;
			            #ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
				          SENSORDB("SENSOR_FEATURE_GET_PERIOD£¬OV8835MIPI pv line length:%d\n",OV8835MIPI_Semco_Kantatsu_PV_PERIOD_PIXEL_NUMS + OV8835MIPI_Semco_Kantatsu_sensor.dummy_pixels); 
			            #endif	
						break;
				}		
		break;
		case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:  /* 3 */
			switch(OV8835MIPIRAW_Semco_Kantatsu_CurrentScenarioId)
				{
				   case MSDK_SCENARIO_ID_CAMERA_ZSD:
						*pFeatureReturnPara32 = OV8835MIPI_Semco_Kantatsu_ZSD_PRE_CLK; //OV8835MIPI_Semco_Kantatsu_sensor.cap_pclk;
						*pFeatureParaLen=4;
						#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
				          SENSORDB("SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ£¬OV8835MIPI_Semco_Kantatsu_ZSD_PRE_CLK:%d\n",OV8835MIPI_Semco_Kantatsu_ZSD_PRE_CLK); 
			            #endif
						break;
					default:
						*pFeatureReturnPara32 = OV8835MIPI_Semco_Kantatsu_sensor.pv_pclk;
						*pFeatureParaLen=4;
						#ifdef OV8835MIPI_Semco_Kantatsu_DRIVER_TRACE
				          SENSORDB("SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ£¬OV8835MIPI_Semco_Kantatsu_sensor.pv_pclk:%d\n",OV8835MIPI_Semco_Kantatsu_sensor.pv_pclk); 
			            #endif
						break;
				}
		break;
		case SENSOR_FEATURE_SET_ESHUTTER:	/* 4 */
			set_OV8835MIPI_Semco_Kantatsu_shutter(*pFeatureData16);
		break;
		case SENSOR_FEATURE_SET_NIGHTMODE:
			//OV8835MIPI_Semco_Kantatsu_night_mode((BOOL) *pFeatureData16);
		break;
		case SENSOR_FEATURE_SET_GAIN:	/* 6 */
			OV8835MIPI_Semco_Kantatsu_SetGain((UINT16) *pFeatureData16);
		break;
		case SENSOR_FEATURE_SET_FLASHLIGHT:
		break;
		case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
		break;
		case SENSOR_FEATURE_SET_REGISTER:
		OV8835MIPI_Semco_Kantatsu_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
		break;
		case SENSOR_FEATURE_GET_REGISTER:
			pSensorRegData->RegData = OV8835MIPI_Semco_Kantatsu_read_cmos_sensor(pSensorRegData->RegAddr);
		break;
		case SENSOR_FEATURE_SET_CCT_REGISTER:
			//memcpy(&OV8835MIPI_Semco_Kantatsu_sensor.eng.cct, pFeaturePara, sizeof(OV8835MIPI_Semco_Kantatsu_sensor.eng.cct));
			OV8835MIPISensorRegNumber = OV8835MIPI_Semco_Kantatsu_FACTORY_END_ADDR;
			for (i=0;i<OV8835MIPISensorRegNumber;i++)
            {
                spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
                OV8835MIPI_Semco_Kantatsu_sensor.eng.cct[i].Addr=*pFeatureData32++;
                OV8835MIPI_Semco_Kantatsu_sensor.eng.cct[i].Para=*pFeatureData32++;
			    spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
            }
			
		break;
		case SENSOR_FEATURE_GET_CCT_REGISTER:	/* 12 */
			if (*pFeatureParaLen >= sizeof(OV8835MIPI_Semco_Kantatsu_sensor.eng.cct) + sizeof(kal_uint32))
			{
			  *((kal_uint32 *)pFeaturePara++) = sizeof(OV8835MIPI_Semco_Kantatsu_sensor.eng.cct);
			  memcpy(pFeaturePara, &OV8835MIPI_Semco_Kantatsu_sensor.eng.cct, sizeof(OV8835MIPI_Semco_Kantatsu_sensor.eng.cct));
			}
			break;
		case SENSOR_FEATURE_SET_ENG_REGISTER:
			//memcpy(&OV8835MIPI_Semco_Kantatsu_sensor.eng.reg, pFeaturePara, sizeof(OV8835MIPI_Semco_Kantatsu_sensor.eng.reg));
			OV8835MIPISensorRegNumber = OV8835MIPI_Semco_Kantatsu_ENGINEER_END;
			for (i=0;i<OV8835MIPISensorRegNumber;i++)
            {
                spin_lock(&ov8835mipi_semco_kantatsudrv_lock);
                OV8835MIPI_Semco_Kantatsu_sensor.eng.reg[i].Addr=*pFeatureData32++;
                OV8835MIPI_Semco_Kantatsu_sensor.eng.reg[i].Para=*pFeatureData32++;
			    spin_unlock(&ov8835mipi_semco_kantatsudrv_lock);
            }
			break;
		case SENSOR_FEATURE_GET_ENG_REGISTER:	/* 14 */
			if (*pFeatureParaLen >= sizeof(OV8835MIPI_Semco_Kantatsu_sensor.eng.reg) + sizeof(kal_uint32))
			{
			  *((kal_uint32 *)pFeaturePara++) = sizeof(OV8835MIPI_Semco_Kantatsu_sensor.eng.reg);
			  memcpy(pFeaturePara, &OV8835MIPI_Semco_Kantatsu_sensor.eng.reg, sizeof(OV8835MIPI_Semco_Kantatsu_sensor.eng.reg));
			}
		case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
			((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->Version = NVRAM_CAMERA_SENSOR_FILE_VERSION;
			((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->SensorId = OV8835MIPI_SEMCO_KANTATSU_SENSOR_ID;
			memcpy(((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->SensorEngReg, &OV8835MIPI_Semco_Kantatsu_sensor.eng.reg, sizeof(OV8835MIPI_Semco_Kantatsu_sensor.eng.reg));
			memcpy(((PNVRAM_SENSOR_DATA_STRUCT)pFeaturePara)->SensorCCTReg, &OV8835MIPI_Semco_Kantatsu_sensor.eng.cct, sizeof(OV8835MIPI_Semco_Kantatsu_sensor.eng.cct));
			*pFeatureParaLen = sizeof(NVRAM_SENSOR_DATA_STRUCT);
			break;
		case SENSOR_FEATURE_GET_CONFIG_PARA:
			memcpy(pFeaturePara, &OV8835MIPI_Semco_Kantatsu_sensor.cfg_data, sizeof(OV8835MIPI_Semco_Kantatsu_sensor.cfg_data));
			*pFeatureParaLen = sizeof(OV8835MIPI_Semco_Kantatsu_sensor.cfg_data);
			break;
		case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
		     OV8835MIPI_Semco_Kantatsu_camera_para_to_sensor();
		break;
		case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
			OV8835MIPI_Semco_Kantatsu_sensor_to_camera_para();
		break;							
		case SENSOR_FEATURE_GET_GROUP_COUNT:
			OV8835MIPI_Semco_Kantatsu_get_sensor_group_count((kal_uint32 *)pFeaturePara);
			*pFeatureParaLen = 4;
		break;										
		  OV8835MIPI_Semco_Kantatsu_get_sensor_group_info((MSDK_SENSOR_GROUP_INFO_STRUCT *)pFeaturePara);
		  *pFeatureParaLen = sizeof(MSDK_SENSOR_GROUP_INFO_STRUCT);
		  break;
		case SENSOR_FEATURE_GET_ITEM_INFO:
		  OV8835MIPI_Semco_Kantatsu_get_sensor_item_info((MSDK_SENSOR_ITEM_INFO_STRUCT *)pFeaturePara);
		  *pFeatureParaLen = sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
		  break;
		case SENSOR_FEATURE_SET_ITEM_INFO:
		  OV8835MIPI_Semco_Kantatsu_set_sensor_item_info((MSDK_SENSOR_ITEM_INFO_STRUCT *)pFeaturePara);
		  *pFeatureParaLen = sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
		  break;
		case SENSOR_FEATURE_GET_ENG_INFO:
     		memcpy(pFeaturePara, &OV8835MIPI_Semco_Kantatsu_sensor.eng_info, sizeof(OV8835MIPI_Semco_Kantatsu_sensor.eng_info));
     		*pFeatureParaLen = sizeof(OV8835MIPI_Semco_Kantatsu_sensor.eng_info);
     		break;
		case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
			// get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
			// if EEPROM does not exist in camera module.
			*pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_SET_VIDEO_MODE:
		       OV8835MIPI_Semco_Kantatsu_SetVideoMode(*pFeatureData16);
        break; 
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            OV8835MIPI_Semco_Kantatsu_GetSensorID(pFeatureReturnPara32); 
            break; 
		case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
			OV8835MIPI_Semco_Kantatsu_SetAutoFlickerMode((BOOL)*pFeatureData16,*(pFeatureData16+1));
			break;
		default:
			break;
	}
	return ERROR_NONE;
}	/* OV8835MIPI_Semco_Kantatsu_FeatureControl() */
SENSOR_FUNCTION_STRUCT	SensorFuncOV8835MIPI_Semco_Kantatsu=
{
	OV8835MIPI_Semco_Kantatsu_Open,
	OV8835MIPI_Semco_Kantatsu_GetInfo,
	OV8835MIPI_Semco_Kantatsu_GetResolution,
	OV8835MIPI_Semco_Kantatsu_FeatureControl,
	OV8835MIPI_Semco_Kantatsu_Control,
	OV8835MIPI_Semco_Kantatsu_Close
};

UINT32 OV8835_MIPI_RAW_SEMCO_KANTATSU_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
//   SENSORDB("OV8835_MIPI_RAW_SEMCO_KANTATSU_SensorInit");
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncOV8835MIPI_Semco_Kantatsu;

	return ERROR_NONE;
}	/* SensorInit() */



