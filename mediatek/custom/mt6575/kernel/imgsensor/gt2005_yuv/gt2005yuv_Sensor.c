
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "kd_camera_feature.h"

#include "gt2005yuv_Sensor.h"
#include "gt2005yuv_Camera_Sensor_para.h"
#include "gt2005yuv_CameraCustomized.h"

#define GT2005YUV_DEBUG
#ifdef GT2005YUV_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif

#define __GT_CAP_DB__

static GT2005_SENSOR_INFO_ST GT2005_sensor;

extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId, bool isRegAddr16bit);
extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId, bool isRegAddr16bit);


#define GT2005_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para ,1,GT2005_WRITE_ID, true)


kal_uint16 GT2005_read_cmos_sensor(kal_uint32 addr)
{
kal_uint16 get_byte=0;
    iReadReg((u16) addr ,(u8*)&get_byte,GT2005_WRITE_ID, true);
    return get_byte;
}


/*******************************************************************************
* // Adapter for Winmo typedef 
********************************************************************************/
#define WINMO_USE 0

#define Sleep(ms) mdelay(ms)
#define RETAILMSG(x,...)
#define TEXT


/*******************************************************************************
* // End Adapter for Winmo typedef 
********************************************************************************/


#define	GT2005_LIMIT_EXPOSURE_LINES				(1253)
#define	GT2005_VIDEO_NORMALMODE_30FRAME_RATE       (30)
#define	GT2005_VIDEO_NORMALMODE_FRAME_RATE         (15)
#define	GT2005_VIDEO_NIGHTMODE_FRAME_RATE          (7.5)
#define BANDING50_30HZ
/* Global Valuable */

static kal_uint32 zoom_factor = 0; 
static kal_uint8 GT2005_exposure_line_h = 0, GT2005_exposure_line_l = 0,GT2005_extra_exposure_line_h = 0, GT2005_extra_exposure_line_l = 0;

static kal_bool GT2005_gPVmode = KAL_TRUE; //PV size or Full size
static kal_bool GT2005_VEDIO_encode_mode = KAL_FALSE; //Picture(Jpeg) or Video(Mpeg4)
static kal_bool GT2005_sensor_cap_state = KAL_FALSE; //Preview or Capture

static kal_uint16 GT2005_dummy_pixels=0, GT2005_dummy_lines=0;

static kal_uint16 GT2005_exposure_lines=0, GT2005_extra_exposure_lines = 0;


static kal_int8 GT2005_DELAY_AFTER_PREVIEW = -1;

static kal_uint8 GT2005_Banding_setting = AE_FLICKER_MODE_50HZ;  //Wonder add

/****** OVT 6-18******/
static kal_uint16 GT2005_Capture_Max_Gain16= 6*16;
static kal_uint16 GT2005_Capture_Gain16=0 ;    
static kal_uint16 GT2005_Capture_Shutter=0;
static kal_uint16 GT2005_Capture_Extra_Lines=0;

static kal_uint16  GT2005_PV_Dummy_Pixels =0, GT2005_Capture_Dummy_Pixels =0, GT2005_Capture_Dummy_Lines =0;
static kal_uint16  GT2005_PV_Gain16 = 0;
static kal_uint16  GT2005_PV_Shutter = 0;
static kal_uint16  GT2005_PV_Extra_Lines = 0;

kal_uint16 GT2005_sensor_gain_base=0,GT2005_FAC_SENSOR_REG=0,GT2005_iGT2005_Mode=0,GT2005_max_exposure_lines=0;
kal_uint32 GT2005_capture_pclk_in_M=520,GT2005_preview_pclk_in_M=390,GT2005_PV_dummy_pixels=0,GT2005_PV_dummy_lines=0,GT2005_isp_master_clock=0;


static kal_uint32  GT2005_sensor_pclk=390;
static kal_bool GT2005_AWB_ENABLE = KAL_TRUE; 
static kal_bool GT2005_AE_ENABLE = KAL_TRUE; 
kal_bool CapState = KAL_FALSE; 
kal_uint32  Shutter = 1;

static kal_uint32 Capture_Shutter = 0; 
static kal_uint32 Capture_Gain = 0; 


UINT8 GT2005_PixelClockDivider=0;

MSDK_SENSOR_CONFIG_STRUCT GT2005SensorConfigData;



kal_uint16 GT2005_read_GT2005_gain(void)
{
	printk("%s\n", __func__);
	return 0;//liuhui add
}  /* GT2005_read_GT2005_gain */


kal_uint16 GT2005_read_shutter(void)
{kal_uint16 temp_reg1, temp_reg2;
	
	/* Backup the preview mode last shutter & sensor gain. */
	temp_reg1 = GT2005_read_cmos_sensor(0x0012);
	temp_reg2 = GT2005_read_cmos_sensor(0x0013);
	GT2005_sensor.pv_shutter = (temp_reg1 << 8) | (temp_reg2 & 0xFF);
	
	/* Backup the preview mode last shutter & sensor gain. */
	GT2005_sensor.pv_extra_shutter = 0;
	
	return GT2005_sensor.pv_shutter + GT2005_sensor.pv_extra_shutter;
}    /* GT2005_read_shutter */

void GT2005_write_GT2005_gain(kal_uint16 gain)
{    
	printk("%s\n", __func__);
	
}  /* GT2005_write_GT2005_gain */

static void GT2005_write_shutter(kal_uint16 shutter)
{
		kal_uint16 AGain_shutter,DGain_shutter;

	AGain_shutter = (GT2005_read_cmos_sensor(0x0014)<<8 )|( GT2005_read_cmos_sensor(0x0015));
	DGain_shutter = (GT2005_read_cmos_sensor(0x0016)<<8 )|( GT2005_read_cmos_sensor(0x0017));

	GT2005_write_cmos_sensor(0x0305 , shutter&0xff);           
	GT2005_write_cmos_sensor(0x0304 , (shutter>>8)&0xff); 

	GT2005_write_cmos_sensor(0x0307 , AGain_shutter&0xff);      
	GT2005_write_cmos_sensor(0x0306 , (AGain_shutter>>8)&0xff); //AG

	GT2005_write_cmos_sensor(0x0308,  (DGain_shutter>>2)&0xff);   //DG

	GT2005_write_cmos_sensor(0x0300 , 0x01); //close ALC
}


void GT2005_Computer_AECAGC(kal_uint16 preview_clk_in_M, kal_uint16 capture_clk_in_M)
{
	printk("%s\n", __func__);
    kal_uint16 PV_Line_Width;
    kal_uint16 Capture_Line_Width;
    kal_uint16 Capture_Maximum_Shutter;
    kal_uint16 Capture_Exposure;
    kal_uint16 Capture_Gain16;
    kal_uint32 Capture_Banding_Filter;
    kal_uint32 Gain_Exposure=0;

    PV_Line_Width = GT2005_PV_PERIOD_PIXEL_NUMS + GT2005_PV_Dummy_Pixels;   

    Capture_Line_Width = GT2005_FULL_PERIOD_PIXEL_NUMS + GT2005_Capture_Dummy_Pixels;
    Capture_Maximum_Shutter = GT2005_FULL_EXPOSURE_LIMITATION + GT2005_Capture_Dummy_Lines;

    if (GT2005_Banding_setting == AE_FLICKER_MODE_50HZ)
#if WINMO_USE        
        Capture_Banding_Filter = (kal_uint32)(capture_clk_in_M*1000000/100/(2*Capture_Line_Width)+0.5);
#else 
        Capture_Banding_Filter = (kal_uint32)(capture_clk_in_M*100000/100/(2*Capture_Line_Width));
#endif 
    else
#if WINMO_USE
        Capture_Banding_Filter = (kal_uint16)(capture_clk_in_M*1000000/120/(2*Capture_Line_Width)+0.5);
#else 
        Capture_Banding_Filter = (kal_uint32)(capture_clk_in_M*100000/120/(2*Capture_Line_Width) );
#endif 

    /*   Gain_Exposure = GT2005_PV_Gain16*(GT2005_PV_Shutter+GT2005_PV_Extra_Lines)*PV_Line_Width/g_Preview_PCLK_Frequency/Capture_Line_Width*g_Capture_PCLK_Frequency
    ;*/
    GT2005_PV_Gain16 = GT2005_read_GT2005_gain();
    Gain_Exposure = 1 * GT2005_PV_Gain16;  //For GT2005

    Gain_Exposure *=(GT2005_PV_Shutter+GT2005_PV_Extra_Lines);
    Gain_Exposure *=PV_Line_Width;  //970
    //   Gain_Exposure /=g_Preview_PCLK_Frequency;
    Gain_Exposure /=Capture_Line_Width;//1940
    Gain_Exposure = Gain_Exposure*capture_clk_in_M/preview_clk_in_M;// for clock   

    //redistribute gain and exposure
    if (Gain_Exposure < (kal_uint32)(Capture_Banding_Filter * 16))     // Exposure < 1/100/120
    {
      		 if(Gain_Exposure<16)
		   {
	            Gain_Exposure = Gain_Exposure*4;     
	            Capture_Exposure = 1;
	            Capture_Gain16 = (Gain_Exposure*2 + 1)/Capture_Exposure/2/4;
                 }
       	 else
        {
            Capture_Exposure = Gain_Exposure /16;
            Capture_Gain16 = (Gain_Exposure*2 + 1)/Capture_Exposure/2;
        }
    }
    else 
    {
        if (Gain_Exposure >(kal_uint32)( Capture_Maximum_Shutter * 16)) // Exposure > Capture_Maximum_Shutter
        {
           
            Capture_Exposure = Capture_Maximum_Shutter/Capture_Banding_Filter*Capture_Banding_Filter;
            Capture_Gain16 = (Gain_Exposure*2 + 1)/Capture_Exposure/2;
            if (Capture_Gain16 > GT2005_Capture_Max_Gain16) 
            {
                // gain reach maximum, insert extra line
                Capture_Exposure = (kal_uint16)(Gain_Exposure*11 /10 /GT2005_Capture_Max_Gain16);
                
                // Exposure = n/100/120
                Capture_Exposure = Capture_Exposure/Capture_Banding_Filter * Capture_Banding_Filter;
                Capture_Gain16 = ((Gain_Exposure *4)/ Capture_Exposure+3)/4;
            }
        }
        else  // 1/100 < Exposure < Capture_Maximum_Shutter, Exposure = n/100/120
        {
            Capture_Exposure = Gain_Exposure/16/Capture_Banding_Filter;
            Capture_Exposure = Capture_Exposure * Capture_Banding_Filter;
            Capture_Gain16 = (Gain_Exposure*2 +1) / Capture_Exposure/2;
        }
    }
    
    GT2005_Capture_Gain16 = Capture_Gain16;
    GT2005_Capture_Extra_Lines = (Capture_Exposure > Capture_Maximum_Shutter)?
            (Capture_Exposure - Capture_Maximum_Shutter/Capture_Banding_Filter*Capture_Banding_Filter):0;     
    
    GT2005_Capture_Shutter = Capture_Exposure - GT2005_Capture_Extra_Lines;
}

void GT2005_set_isp_driving_current(kal_uint8 current)
{
	printk("%s\n", __func__);
}


static void GT2005_set_AE_mode(kal_bool AE_enable)
{
	printk("%s\n", __func__);
	
	kal_uint16 Shutter, AGain, DGain;

	if (AE_enable)
	{
		GT2005_write_cmos_sensor(0x0300, 0x81);
	}
	else
	{
		/* when disable ae, ae will use 0x304~308 as shutter and gain.
		so need to read 0x12~17 and apply to 0x304~308 to avoid ae changed.
		*/
		GT2005_write_cmos_sensor(0x0300, 0xC1); /* fix ae */
		Shutter = (GT2005_read_cmos_sensor(0x0012) << 8)|GT2005_read_cmos_sensor(0x0013);
		AGain = (GT2005_read_cmos_sensor(0x0014) << 8)|GT2005_read_cmos_sensor(0x0015);
		DGain = (GT2005_read_cmos_sensor(0x0016) << 8)|GT2005_read_cmos_sensor(0x0017);
		GT2005_write_cmos_sensor(0x0304, Shutter >> 8);
		GT2005_write_cmos_sensor(0x0305, Shutter);
		GT2005_write_cmos_sensor(0x0306, AGain >> 8);
		GT2005_write_cmos_sensor(0x0307, AGain);
		GT2005_write_cmos_sensor(0x0308, DGain >> 2);
		GT2005_write_cmos_sensor(0x0300, 0x01); /* disable ae */
	}
}


static void GT2005_set_AWB_mode(kal_bool AWB_enable)
{
	printk("%s\n", __func__);
    kal_uint8 temp_AWB_reg = 0;

    //return ;

    if (AWB_enable == KAL_TRUE)
    {
        //enable Auto WB
 
      	GT2005_write_cmos_sensor(0x031a,0x81);   // select Auto WB
	GT2005_write_cmos_sensor(0x0320,0x24);//  WBGRMAX[7:0];
	GT2005_write_cmos_sensor(0x0321,0x14);//  WBGRMIN[7:0];
	GT2005_write_cmos_sensor(0x0322,0x1A);//  WBGBMAX[7:0];
	GT2005_write_cmos_sensor(0x0323,0x24);//  WBGBMIN[7:0];
	GT2005_write_cmos_sensor(0x0441,0x63);//  PWBGAINR[7:0];
	GT2005_write_cmos_sensor(0x0442,0x00);//  PWBGAINGR[7:0];
	GT2005_write_cmos_sensor(0x0443,0x00);//  PWBGAINGB[7:0];
	GT2005_write_cmos_sensor(0x0444,0x24);//  PWBGAINB[7:0];
    }
    else
    {
        //turn off AWB
        temp_AWB_reg = GT2005_read_cmos_sensor(0x031a);
        GT2005_write_cmos_sensor(0x031a, temp_AWB_reg & ~0x80);        
    }
}


/*************************************************************************
* FUNCTION
*	GT2005_night_mode
*
* DESCRIPTION
*	This function night mode of GT2005.
*
* PARAMETERS
*	none
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
BOOL GT2005_set_param_banding(UINT16 para); 
void GT2005_night_mode(kal_bool enable)
{
		printk("%s\n", __func__);
	//kal_uint8 night = GT2005_read_cmos_sensor(0x3014); //bit[3], 0: disable, 1:enable
		if (GT2005_sensor_cap_state == KAL_FALSE) 
		{
			if (enable) 
			{
			 
				if (GT2005_VEDIO_encode_mode == KAL_TRUE) 
				{
					/* MJPEG or MPEG4 Apps */
				
					GT2005_write_cmos_sensor(0x0312,0xe8); //Enable night mode 
				}
				else 
				{
					/* Camera mode only */                
			
					GT2005_write_cmos_sensor(0x0312,0xe8); //Enable night mode
				}	   
			}
			else 
			{
				/* when enter normal mode (disable night mode) without light, the AE vibrate */
				if (GT2005_VEDIO_encode_mode == KAL_TRUE) 
				{
					/* MJPEG or MPEG4 Apps */
		       		GT2005_write_cmos_sensor(0x0312,0x98); //Disable night mode
				}
				else 
				{
					GT2005_write_cmos_sensor(0x0312,0x98); //Disable night mode
				}	
		}
	}
       // GT2005_set_param_banding(GT2005_Banding_setting); 
}	/* GT2005_night_mode */



/*****************************************************************************/
/* Windows Mobile Sensor Interface */
/*****************************************************************************/
static void GT2005_LSC(void)
{
#if 1
GT2005_write_cmos_sensor(0x040c,0xe0);
GT2005_write_cmos_sensor(0x040d,0x00);
GT2005_write_cmos_sensor(0x040e,0x00);
GT2005_write_cmos_sensor(0x040f,0x00);
GT2005_write_cmos_sensor(0x0410,0x00);
GT2005_write_cmos_sensor(0x0411,0x00);
GT2005_write_cmos_sensor(0x0412,0x03);
GT2005_write_cmos_sensor(0x0413,0x00);
GT2005_write_cmos_sensor(0x0414,0x04);
GT2005_write_cmos_sensor(0x0415,0x04);
GT2005_write_cmos_sensor(0x0416,0x01);
GT2005_write_cmos_sensor(0x0417,0x02);
GT2005_write_cmos_sensor(0x0418,0x01);
GT2005_write_cmos_sensor(0x0419,0x01);
GT2005_write_cmos_sensor(0x041a,0x01);
GT2005_write_cmos_sensor(0x041b,0x02);
GT2005_write_cmos_sensor(0x041c,0x00);
GT2005_write_cmos_sensor(0x041d,0x00);
GT2005_write_cmos_sensor(0x041e,0x00);
GT2005_write_cmos_sensor(0x041f,0x05);
GT2005_write_cmos_sensor(0x0420,0x3b);
GT2005_write_cmos_sensor(0x0421,0x3b);
GT2005_write_cmos_sensor(0x0422,0x34);
GT2005_write_cmos_sensor(0x0423,0x35);
GT2005_write_cmos_sensor(0x0424,0x3b);
GT2005_write_cmos_sensor(0x0425,0x3d);
GT2005_write_cmos_sensor(0x0426,0x3b);
GT2005_write_cmos_sensor(0x0427,0x37);
GT2005_write_cmos_sensor(0x0428,0x1e);
GT2005_write_cmos_sensor(0x0429,0x1d);
GT2005_write_cmos_sensor(0x042a,0x1e);
GT2005_write_cmos_sensor(0x042b,0x29);
GT2005_write_cmos_sensor(0x042c,0x1a);
GT2005_write_cmos_sensor(0x042d,0x1a);
GT2005_write_cmos_sensor(0x042e,0x19);
GT2005_write_cmos_sensor(0x042f,0x10);
GT2005_write_cmos_sensor(0x0430,0x51);
GT2005_write_cmos_sensor(0x0431,0x51);
GT2005_write_cmos_sensor(0x0432,0x28);
GT2005_write_cmos_sensor(0x0433,0x4d);
GT2005_write_cmos_sensor(0x0434,0x00);
GT2005_write_cmos_sensor(0x0435,0x00);
GT2005_write_cmos_sensor(0x0436,0x00);
GT2005_write_cmos_sensor(0x0437,0x00);
GT2005_write_cmos_sensor(0x0438,0x18);
GT2005_write_cmos_sensor(0x0439,0x18);
GT2005_write_cmos_sensor(0x043a,0x00);
GT2005_write_cmos_sensor(0x043b,0x57);
GT2005_write_cmos_sensor(0x043c,0x28);
GT2005_write_cmos_sensor(0x043d,0x28);
GT2005_write_cmos_sensor(0x043e,0x36);
GT2005_write_cmos_sensor(0x043f,0x00);

/*
	GT2005_write_cmos_sensor(0x040c,0xe0);
	GT2005_write_cmos_sensor(0x040d,0x00);
	GT2005_write_cmos_sensor(0x040e,0x00);
	GT2005_write_cmos_sensor(0x040f,0x00);
	GT2005_write_cmos_sensor(0x0410,0x09);
	GT2005_write_cmos_sensor(0x0411,0x09);
	GT2005_write_cmos_sensor(0x0412,0x06);
	GT2005_write_cmos_sensor(0x0413,0x0e);
	GT2005_write_cmos_sensor(0x0414,0x0a);
	GT2005_write_cmos_sensor(0x0415,0x0a);
	GT2005_write_cmos_sensor(0x0416,0x06);
	GT2005_write_cmos_sensor(0x0417,0x06);
	GT2005_write_cmos_sensor(0x0418,0x09);
	GT2005_write_cmos_sensor(0x0419,0x09);
	GT2005_write_cmos_sensor(0x041a,0x0a);
	GT2005_write_cmos_sensor(0x041b,0x08);
	GT2005_write_cmos_sensor(0x041c,0x0b);
	GT2005_write_cmos_sensor(0x041d,0x0b);
	GT2005_write_cmos_sensor(0x041e,0x0d);
	GT2005_write_cmos_sensor(0x041f,0x13);
	GT2005_write_cmos_sensor(0x0420,0x3c);
	GT2005_write_cmos_sensor(0x0421,0x3c);
	GT2005_write_cmos_sensor(0x0422,0x38);
	GT2005_write_cmos_sensor(0x0423,0x36);
	GT2005_write_cmos_sensor(0x0424,0x41);
	GT2005_write_cmos_sensor(0x0425,0x41);
	GT2005_write_cmos_sensor(0x0426,0x3f);
	GT2005_write_cmos_sensor(0x0427,0x38);
	GT2005_write_cmos_sensor(0x0428,0x20);
	GT2005_write_cmos_sensor(0x0429,0x20);
	GT2005_write_cmos_sensor(0x042a,0x20);
	GT2005_write_cmos_sensor(0x042b,0x21);
	GT2005_write_cmos_sensor(0x042c,0x23);
	GT2005_write_cmos_sensor(0x042d,0x23);
	GT2005_write_cmos_sensor(0x042e,0x20);
	GT2005_write_cmos_sensor(0x042f,0x16);
	GT2005_write_cmos_sensor(0x0430,0x28);
	GT2005_write_cmos_sensor(0x0431,0x28);
	GT2005_write_cmos_sensor(0x0432,0x00);
	GT2005_write_cmos_sensor(0x0433,0x2c);
	GT2005_write_cmos_sensor(0x0434,0x2c);
	GT2005_write_cmos_sensor(0x0435,0x2c);
	GT2005_write_cmos_sensor(0x0436,0x27);
	GT2005_write_cmos_sensor(0x0437,0x00);
	GT2005_write_cmos_sensor(0x0438,0x00);
	GT2005_write_cmos_sensor(0x0439,0x00);
	GT2005_write_cmos_sensor(0x043a,0x00);
	GT2005_write_cmos_sensor(0x043b,0x00);
	GT2005_write_cmos_sensor(0x043c,0x36);
	GT2005_write_cmos_sensor(0x043d,0x3c);
	GT2005_write_cmos_sensor(0x043e,0x3c);
	GT2005_write_cmos_sensor(0x043f,0x00);
	*/
#else
	GT2005_write_cmos_sensor(0x040C,0xE0);
	GT2005_write_cmos_sensor(0x040D,0x00);
	GT2005_write_cmos_sensor(0x040E,0x00);
	GT2005_write_cmos_sensor(0x040F,0x00);
	GT2005_write_cmos_sensor(0x0410,0x14);
	GT2005_write_cmos_sensor(0x0411,0x14);
	GT2005_write_cmos_sensor(0x0412,0x12);
	GT2005_write_cmos_sensor(0x0413,0x18);
	GT2005_write_cmos_sensor(0x0414,0x0E);
	GT2005_write_cmos_sensor(0x0415,0x0E);
	GT2005_write_cmos_sensor(0x0416,0x0A);
	GT2005_write_cmos_sensor(0x0417,0x0B);
	GT2005_write_cmos_sensor(0x0418,0x0C);
	GT2005_write_cmos_sensor(0x0419,0x0C);
	GT2005_write_cmos_sensor(0x041A,0x0A);
	GT2005_write_cmos_sensor(0x041B,0x08);
	GT2005_write_cmos_sensor(0x041C,0x00);
	GT2005_write_cmos_sensor(0x041D,0x00);
	GT2005_write_cmos_sensor(0x041E,0x01);
	GT2005_write_cmos_sensor(0x041F,0x08);
	GT2005_write_cmos_sensor(0x0420,0x3C);
	GT2005_write_cmos_sensor(0x0421,0x3C);
	GT2005_write_cmos_sensor(0x0422,0x3E);
	GT2005_write_cmos_sensor(0x0423,0x33);
	GT2005_write_cmos_sensor(0x0424,0x40);
	GT2005_write_cmos_sensor(0x0425,0x2D);
	GT2005_write_cmos_sensor(0x0426,0x35);
	GT2005_write_cmos_sensor(0x0427,0x2E);
	GT2005_write_cmos_sensor(0x0428,0x1D);
	GT2005_write_cmos_sensor(0x0429,0x1D);
	GT2005_write_cmos_sensor(0x042A,0x1E);
	GT2005_write_cmos_sensor(0x042B,0x1A);
	GT2005_write_cmos_sensor(0x042C,0x25);
	GT2005_write_cmos_sensor(0x042D,0x1F);
	GT2005_write_cmos_sensor(0x042E,0x1E);
	GT2005_write_cmos_sensor(0x042F,0x18);
	GT2005_write_cmos_sensor(0x0430,0x47);
	GT2005_write_cmos_sensor(0x0431,0x45);
	GT2005_write_cmos_sensor(0x0432,0x47);
	GT2005_write_cmos_sensor(0x0433,0x4B);
	GT2005_write_cmos_sensor(0x0434,0x51);
	GT2005_write_cmos_sensor(0x0435,0x51);
	GT2005_write_cmos_sensor(0x0436,0x43);
	GT2005_write_cmos_sensor(0x0437,0x3D);
	GT2005_write_cmos_sensor(0x0438,0x28);
	GT2005_write_cmos_sensor(0x0439,0x2B);
	GT2005_write_cmos_sensor(0x043A,0x38);
	GT2005_write_cmos_sensor(0x043B,0x22);
	GT2005_write_cmos_sensor(0x043C,0x3B);
	GT2005_write_cmos_sensor(0x043D,0x3B);
	GT2005_write_cmos_sensor(0x043E,0x31);
	GT2005_write_cmos_sensor(0x043F,0x32);
#endif
}

static void GT2005_Gamma_Select(UINT16 curve)
{
	switch (curve)
	{
		case 5:
			// gamma curve 5
			GT2005_write_cmos_sensor(0x0461 , 0x00);
			GT2005_write_cmos_sensor(0x0462 , 0x00);
			GT2005_write_cmos_sensor(0x0463 , 0x00);
			GT2005_write_cmos_sensor(0x0464 , 0x00);
			GT2005_write_cmos_sensor(0x0465 , 0x15);
			GT2005_write_cmos_sensor(0x0466 , 0x33);
			GT2005_write_cmos_sensor(0x0467 , 0x61);
			GT2005_write_cmos_sensor(0x0468 , 0x56);
			GT2005_write_cmos_sensor(0x0469 , 0x30);
			GT2005_write_cmos_sensor(0x046A , 0x22);
			GT2005_write_cmos_sensor(0x046B , 0x3E);
			GT2005_write_cmos_sensor(0x046C , 0x2E);
			GT2005_write_cmos_sensor(0x046D , 0x2B);
			GT2005_write_cmos_sensor(0x046E , 0x28);
			GT2005_write_cmos_sensor(0x046F , 0x3C);
			GT2005_write_cmos_sensor(0x0470 , 0x38);
			GT2005_write_cmos_sensor(0x0471 , 0x2F);
			GT2005_write_cmos_sensor(0x0472 , 0x2A);
			GT2005_write_cmos_sensor(0x0473 , 0x3C);
			GT2005_write_cmos_sensor(0x0474 , 0x34);
			GT2005_write_cmos_sensor(0x0475 , 0x31);
			GT2005_write_cmos_sensor(0x0476 , 0x31);
			GT2005_write_cmos_sensor(0x0477 , 0x39);
			break;

		case 4:    
			// gamma curve 5
			GT2005_write_cmos_sensor(0x0461 , 0x00);
			GT2005_write_cmos_sensor(0x0462 , 0x00);
			GT2005_write_cmos_sensor(0x0463 , 0x00);
			GT2005_write_cmos_sensor(0x0464 , 0x00);
			GT2005_write_cmos_sensor(0x0465 , 0x0d);
			GT2005_write_cmos_sensor(0x0466 , 0x23);
			GT2005_write_cmos_sensor(0x0467 , 0x5a);
			GT2005_write_cmos_sensor(0x0468 , 0x56);
			GT2005_write_cmos_sensor(0x0469 , 0x33);
			GT2005_write_cmos_sensor(0x046A , 0x2b);
			GT2005_write_cmos_sensor(0x046B , 0x3a);
			GT2005_write_cmos_sensor(0x046C , 0x32);
			GT2005_write_cmos_sensor(0x046D , 0x2B);
			GT2005_write_cmos_sensor(0x046E , 0x28);
			GT2005_write_cmos_sensor(0x046F , 0x3C);
			GT2005_write_cmos_sensor(0x0470 , 0x3a);
			GT2005_write_cmos_sensor(0x0471 , 0x38);
			GT2005_write_cmos_sensor(0x0472 , 0x31);
			GT2005_write_cmos_sensor(0x0473 , 0x3a);
			GT2005_write_cmos_sensor(0x0474 , 0x39);
			GT2005_write_cmos_sensor(0x0475 , 0x33);
			GT2005_write_cmos_sensor(0x0476 , 0x33);
			GT2005_write_cmos_sensor(0x0477 , 0x3a);
			break;

		default:
			break;
	}
}
/*************************************************************************
* FUNCTION
*	GT2005Open
*
* DESCRIPTION
*	This function initialize the registers of CMOS sensor
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
UINT32 GT2005Open(void)
{
	printk("%s\n", __func__);
	volatile signed char i;
	kal_uint16 sensor_id=0;

	zoom_factor = 0; 
    	Sleep(10);

	for(i=0;i<3;i++)
	{
		sensor_id = (GT2005_read_cmos_sensor(0x0000) << 8) | GT2005_read_cmos_sensor(0x0001);
		printk("%s sensor_id=%d\n", __func__, sensor_id);
		if(sensor_id != GT2005_SENSOR_ID)
		{
			return ERROR_SENSOR_CONNECT_FAIL;
		}
	}	
	    RETAILMSG(1, (TEXT("GT2005 Sensor Read ID OK \r\n")));
	    GT2005_write_cmos_sensor(0x0101, 0x00); 
	    GT2005_write_cmos_sensor(0x0102, 0x00); 
	    GT2005_write_cmos_sensor(0x0103, 0x00); 

	/* Hcount&Vcount */
	    GT2005_write_cmos_sensor(0x0105, 0x00); 
	    GT2005_write_cmos_sensor(0x0106, 0xF0); 
	    GT2005_write_cmos_sensor(0x0107, 0x00); 
	    GT2005_write_cmos_sensor(0x0108, 0x1C); 

	/* Binning&Resoultion */
	    GT2005_write_cmos_sensor(0x0109, 0x01); 
	    GT2005_write_cmos_sensor(0x010A, 0x00); //00_1018
	    GT2005_write_cmos_sensor(0x010B, 0x00); 
	    GT2005_write_cmos_sensor(0x010C, 0x00); 
	    GT2005_write_cmos_sensor(0x010D, 0x08); 
	    GT2005_write_cmos_sensor(0x010E, 0x00); 
	    GT2005_write_cmos_sensor(0x010F, 0x08); 
	    GT2005_write_cmos_sensor(0x0110, 0x06); 
	    GT2005_write_cmos_sensor(0x0111, 0x40); 
	    GT2005_write_cmos_sensor(0x0112, 0x04); 
	    GT2005_write_cmos_sensor(0x0113, 0xB0); 

	/* YUV Mode */
	    GT2005_write_cmos_sensor(0x0114, 0x00); //0x00;yuyv

	/* Picture Effect */
	    GT2005_write_cmos_sensor(0x0115, 0x00); 

	/* PLL&Frame Rate */
	    GT2005_write_cmos_sensor(0x0116, 0x02); 
	    GT2005_write_cmos_sensor(0x0117, 0x00); 
	    GT2005_write_cmos_sensor(0x0118, 0x63); //60_1018
	    GT2005_write_cmos_sensor(0x0119, 0x01); 
	    GT2005_write_cmos_sensor(0x011A, 0x04); 
	    GT2005_write_cmos_sensor(0x011B, 0x00); 

	/* DCLK Polarity */
	    GT2005_write_cmos_sensor(0x011C, 0x00); //00

	/* Do not change */
	    GT2005_write_cmos_sensor(0x011D, 0x02); 
	    GT2005_write_cmos_sensor(0x011E, 0x00); 
	    GT2005_write_cmos_sensor(0x011F, 0x00); 
	    GT2005_write_cmos_sensor(0x0120, 0x1C); 
	    GT2005_write_cmos_sensor(0x0121, 0x00); //00
	    GT2005_write_cmos_sensor(0x0122, 0x04); 
	    GT2005_write_cmos_sensor(0x0123, 0x00); 
	    GT2005_write_cmos_sensor(0x0124, 0x00); 
	    GT2005_write_cmos_sensor(0x0125, 0x00); 
	    GT2005_write_cmos_sensor(0x0126, 0x00); 
	    GT2005_write_cmos_sensor(0x0127, 0x00); 
	    GT2005_write_cmos_sensor(0x0128, 0x00); 

	/* Contrast */
	    GT2005_write_cmos_sensor(0x0200, 0x30); //00 3a_1018 /0x20_1024
	    GT2005_write_cmos_sensor(0x0201, 0xf8); // 2b_1018 /0xfc_1024
	    GT2005_write_cmos_sensor(0x0202, 0x58); // 4c_1019 /0x50_1024
	    GT2005_write_cmos_sensor(0x0203, 0x00); 
	    GT2005_write_cmos_sensor(0x0204, 0x0C); 
	    GT2005_write_cmos_sensor(0x0205, 0x09); 
	    GT2005_write_cmos_sensor(0x0206, 0x0A); 
	    GT2005_write_cmos_sensor(0x0207, 0x40); //20
	    GT2005_write_cmos_sensor(0x0208, 0x00); 
	    GT2005_write_cmos_sensor(0x0209, 0x2A); 
	    GT2005_write_cmos_sensor(0x020A, 0x01); 
	    GT2005_write_cmos_sensor(0x020B, 0x68); //28_1018 /0x48_1024
	    GT2005_write_cmos_sensor(0x020C, 0x84);  //44 77_1018 /0x74_1024
	    GT2005_write_cmos_sensor(0x020D, 0xC8); 
	    GT2005_write_cmos_sensor(0x020E, 0xBC); 
	    GT2005_write_cmos_sensor(0x020F, 0x08); 
	    GT2005_write_cmos_sensor(0x0210, 0xe6); //16
	    GT2005_write_cmos_sensor(0x0211, 0x00); 
	    GT2005_write_cmos_sensor(0x0212, 0x20); 
	    GT2005_write_cmos_sensor(0x0213, 0x81); 
	    GT2005_write_cmos_sensor(0x0214, 0x15); 
	    GT2005_write_cmos_sensor(0x0215, 0x00); 
	    GT2005_write_cmos_sensor(0x0216, 0x00); 
	    GT2005_write_cmos_sensor(0x0217, 0x00); 
	    GT2005_write_cmos_sensor(0x0218, 0x44); 
	    GT2005_write_cmos_sensor(0x0219, 0x30); 
	    GT2005_write_cmos_sensor(0x021A, 0x03); 
	    GT2005_write_cmos_sensor(0x021B, 0x28); 
	    GT2005_write_cmos_sensor(0x021C, 0x02); 
	    GT2005_write_cmos_sensor(0x021D, 0x60); 
	    GT2005_write_cmos_sensor(0x021E, 0x00); 
	    GT2005_write_cmos_sensor(0x021F, 0x00); 
	    GT2005_write_cmos_sensor(0x0220, 0x00); 
	    GT2005_write_cmos_sensor(0x0221, 0x00); 
	    GT2005_write_cmos_sensor(0x0222, 0x00); 
	    GT2005_write_cmos_sensor(0x0223, 0x00); 
	    GT2005_write_cmos_sensor(0x0224, 0x00); 
	    GT2005_write_cmos_sensor(0x0225, 0x00); 
	    GT2005_write_cmos_sensor(0x0226, 0x00); 
	    GT2005_write_cmos_sensor(0x0227, 0x00); 
	    GT2005_write_cmos_sensor(0x0228, 0x00); 
	    GT2005_write_cmos_sensor(0x0229, 0x00); 
	    GT2005_write_cmos_sensor(0x022A, 0x00); 
	    GT2005_write_cmos_sensor(0x022B, 0x00); 
	    GT2005_write_cmos_sensor(0x022C, 0x00); 
	    GT2005_write_cmos_sensor(0x022D, 0x00); 
	    GT2005_write_cmos_sensor(0x022E, 0x00); 
	    GT2005_write_cmos_sensor(0x022F, 0x00); 
	    GT2005_write_cmos_sensor(0x0230, 0x00); 
	    GT2005_write_cmos_sensor(0x0231, 0x00); 
	    GT2005_write_cmos_sensor(0x0232, 0x00); 
	    GT2005_write_cmos_sensor(0x0233, 0x00); 
	    GT2005_write_cmos_sensor(0x0234, 0x00); 
	    GT2005_write_cmos_sensor(0x0235, 0x00); 
	    GT2005_write_cmos_sensor(0x0236, 0x00); 
	    GT2005_write_cmos_sensor(0x0237, 0x00); 
	    GT2005_write_cmos_sensor(0x0238, 0x00); 
	    GT2005_write_cmos_sensor(0x0239, 0x00); 
	    GT2005_write_cmos_sensor(0x023A, 0x00); 
	    GT2005_write_cmos_sensor(0x023B, 0x00); 
	    GT2005_write_cmos_sensor(0x023C, 0x00); 
	    GT2005_write_cmos_sensor(0x023D, 0x00); 
	    GT2005_write_cmos_sensor(0x023E, 0x00); 
	    GT2005_write_cmos_sensor(0x023F, 0x00); 
	    GT2005_write_cmos_sensor(0x0240, 0x00); 
	    GT2005_write_cmos_sensor(0x0241, 0x00); 
	    GT2005_write_cmos_sensor(0x0242, 0x00); 
	    GT2005_write_cmos_sensor(0x0243, 0x00); 
	    GT2005_write_cmos_sensor(0x0244, 0x00); 
	    GT2005_write_cmos_sensor(0x0245, 0x00); 
	    GT2005_write_cmos_sensor(0x0246, 0x00); 
	    GT2005_write_cmos_sensor(0x0247, 0x00); 
	    GT2005_write_cmos_sensor(0x0248, 0x00); 
	    GT2005_write_cmos_sensor(0x0249, 0x00); 
	    GT2005_write_cmos_sensor(0x024A, 0x00); 
	    GT2005_write_cmos_sensor(0x024B, 0x00); 
	    GT2005_write_cmos_sensor(0x024C, 0x00); 
	    GT2005_write_cmos_sensor(0x024D, 0x00); 
	    GT2005_write_cmos_sensor(0x024E, 0x00); 
	    GT2005_write_cmos_sensor(0x024F, 0x00); 
	    GT2005_write_cmos_sensor(0x0250, 0x01); 
	    GT2005_write_cmos_sensor(0x0251, 0x00); 
	    GT2005_write_cmos_sensor(0x0252, 0x00); 
	    GT2005_write_cmos_sensor(0x0253, 0x00); 
	    GT2005_write_cmos_sensor(0x0254, 0x00); 
	    GT2005_write_cmos_sensor(0x0255, 0x00); 
	    GT2005_write_cmos_sensor(0x0256, 0x00); 
	    GT2005_write_cmos_sensor(0x0257, 0x00); 
	    GT2005_write_cmos_sensor(0x0258, 0x00); 
	    GT2005_write_cmos_sensor(0x0259, 0x00); 
	    GT2005_write_cmos_sensor(0x025A, 0x00); 
	    GT2005_write_cmos_sensor(0x025B, 0x00); 
	    GT2005_write_cmos_sensor(0x025C, 0x00); 
	    GT2005_write_cmos_sensor(0x025D, 0x00); 
	    GT2005_write_cmos_sensor(0x025E, 0x00); 
	    GT2005_write_cmos_sensor(0x025F, 0x00); 
	    GT2005_write_cmos_sensor(0x0260, 0x00); 
	    GT2005_write_cmos_sensor(0x0261, 0x00); 
	    GT2005_write_cmos_sensor(0x0262, 0x00); 
	    GT2005_write_cmos_sensor(0x0263, 0x00); 
	    GT2005_write_cmos_sensor(0x0264, 0x00); 
	    GT2005_write_cmos_sensor(0x0265, 0x00); 
	    GT2005_write_cmos_sensor(0x0266, 0x00); 
	    GT2005_write_cmos_sensor(0x0267, 0x00); 
	    GT2005_write_cmos_sensor(0x0268, 0x92); //8f /0xfc_1024
	    GT2005_write_cmos_sensor(0x0269, 0xA3); //a3
	    GT2005_write_cmos_sensor(0x026A, 0xaf); //b4_1019
	    GT2005_write_cmos_sensor(0x026B, 0x90); 
	    GT2005_write_cmos_sensor(0x026C, 0x00); 
	    GT2005_write_cmos_sensor(0x026D, 0xD0); 
	    GT2005_write_cmos_sensor(0x026E, 0x60); 
	    GT2005_write_cmos_sensor(0x026F, 0xA0); 
	    GT2005_write_cmos_sensor(0x0270, 0x40); 
	    GT2005_write_cmos_sensor(0x0300, 0x81); 
	    GT2005_write_cmos_sensor(0x0301, 0x80); //c0_1018 0xbb_1024
	    GT2005_write_cmos_sensor(0x0302, 0x0D); 
	    GT2005_write_cmos_sensor(0x0303, 0x07); 
	    GT2005_write_cmos_sensor(0x0304, 0x03); 
	    GT2005_write_cmos_sensor(0x0305, 0x83); 
	    GT2005_write_cmos_sensor(0x0306, 0x00); 
	    GT2005_write_cmos_sensor(0x0307, 0x22); 
	    GT2005_write_cmos_sensor(0x0308, 0x00); 
	    GT2005_write_cmos_sensor(0x0309, 0x55); 
	    GT2005_write_cmos_sensor(0x030A, 0x55); 
	    GT2005_write_cmos_sensor(0x030B, 0x55); 
	    GT2005_write_cmos_sensor(0x030C, 0x54); 
	    GT2005_write_cmos_sensor(0x030D, 0x1F); 
	    GT2005_write_cmos_sensor(0x030E, 0x0A); 
	    GT2005_write_cmos_sensor(0x030F, 0x10); 
	    GT2005_write_cmos_sensor(0x0310, 0x04); 
	    GT2005_write_cmos_sensor(0x0311, 0xFF); 
	    GT2005_write_cmos_sensor(0x0312, 0x98); 
	    GT2005_write_cmos_sensor(0x0313, 0x38); //35
	    GT2005_write_cmos_sensor(0x0314, 0xba); //a4
	    GT2005_write_cmos_sensor(0x0315, 0x16); 
	    GT2005_write_cmos_sensor(0x0316, 0x26); 
	    GT2005_write_cmos_sensor(0x0317, 0x02); 
	    GT2005_write_cmos_sensor(0x0318, 0x08); 
	    GT2005_write_cmos_sensor(0x0319, 0x0C); 
	    GT2005_write_cmos_sensor(0x031A, 0x81); //AWB:auto
	    GT2005_write_cmos_sensor(0x031B, 0x00); 
	    GT2005_write_cmos_sensor(0x031C, 0x3D); 
	    GT2005_write_cmos_sensor(0x031D, 0x00); 
	    GT2005_write_cmos_sensor(0x031E, 0xFD); 
	    GT2005_write_cmos_sensor(0x031F, 0x00); 
#if 1
	    GT2005_write_cmos_sensor(0x0320, 0xE1); 
	    GT2005_write_cmos_sensor(0x0321, 0x1A); 
	    GT2005_write_cmos_sensor(0x0322, 0xDE); 
	    GT2005_write_cmos_sensor(0x0323, 0x11); 
	    GT2005_write_cmos_sensor(0x0324, 0x1A); 
	    GT2005_write_cmos_sensor(0x0325, 0xEE); 
	    GT2005_write_cmos_sensor(0x0326, 0x50); 
	    GT2005_write_cmos_sensor(0x0327, 0x18); 
	    GT2005_write_cmos_sensor(0x0328, 0x25); 
	    GT2005_write_cmos_sensor(0x0329, 0x37); 
	    GT2005_write_cmos_sensor(0x032A, 0x24); 
	    GT2005_write_cmos_sensor(0x032B, 0x32); 
	    GT2005_write_cmos_sensor(0x032C, 0xA9); 
	    GT2005_write_cmos_sensor(0x032D, 0x32); 
	    GT2005_write_cmos_sensor(0x032E, 0xFF); 
	    GT2005_write_cmos_sensor(0x032F, 0x7F); 
	    GT2005_write_cmos_sensor(0x0330, 0xBA); 
	    GT2005_write_cmos_sensor(0x0331, 0x7F); 
	    GT2005_write_cmos_sensor(0x0332, 0x7F); 
	    GT2005_write_cmos_sensor(0x0333, 0x14); 
	    GT2005_write_cmos_sensor(0x0334, 0x81); 
	    GT2005_write_cmos_sensor(0x0335, 0x14); 
	    GT2005_write_cmos_sensor(0x0336, 0xFF); 
	    GT2005_write_cmos_sensor(0x0337, 0x20); 
#else	//add 1019
	    GT2005_write_cmos_sensor(0x0320, 0xE1); 
	    GT2005_write_cmos_sensor(0x0321, 0x1A); 
	    GT2005_write_cmos_sensor(0x0322, 0xDE); 
	    GT2005_write_cmos_sensor(0x0323, 0x11); 
	    GT2005_write_cmos_sensor(0x0324, 0x29); 
	    GT2005_write_cmos_sensor(0x0325, 0xEE); 
	    GT2005_write_cmos_sensor(0x0326, 0x2f); 
	    GT2005_write_cmos_sensor(0x0327, 0x17); 
	    GT2005_write_cmos_sensor(0x0328, 0x25); 
	    GT2005_write_cmos_sensor(0x0329, 0x21); 
	    GT2005_write_cmos_sensor(0x032A, 0x20); 
	    GT2005_write_cmos_sensor(0x032B, 0x19); 
	    GT2005_write_cmos_sensor(0x032C, 0xed); 
	    GT2005_write_cmos_sensor(0x032D, 0x16); 
	    GT2005_write_cmos_sensor(0x032E, 0xF6); 
	    GT2005_write_cmos_sensor(0x032F, 0x1c); 
	    GT2005_write_cmos_sensor(0x0330, 0x3a); 
	    GT2005_write_cmos_sensor(0x0331, 0x23); 
	    GT2005_write_cmos_sensor(0x0332, 0x6c); 
	    GT2005_write_cmos_sensor(0x0333, 0x24); 
	    GT2005_write_cmos_sensor(0x0334, 0xa6); 
	    GT2005_write_cmos_sensor(0x0335, 0x18); 
	    GT2005_write_cmos_sensor(0x0336, 0xFF); 
	    GT2005_write_cmos_sensor(0x0337, 0x20);
#endif
	    GT2005_write_cmos_sensor(0x0338, 0x46); 
	    GT2005_write_cmos_sensor(0x0339, 0x04); 
	    GT2005_write_cmos_sensor(0x033A, 0x04); 
	    GT2005_write_cmos_sensor(0x033B, 0x00); 
	    GT2005_write_cmos_sensor(0x033C, 0x00); 
	    GT2005_write_cmos_sensor(0x033D, 0x00); 
	    GT2005_write_cmos_sensor(0x033E, 0x03); 
	    GT2005_write_cmos_sensor(0x033F, 0x28); 
	    GT2005_write_cmos_sensor(0x0340, 0x02); 
	    GT2005_write_cmos_sensor(0x0341, 0x60); 
	    GT2005_write_cmos_sensor(0x0342, 0x91); 
	    GT2005_write_cmos_sensor(0x0343, 0xAE); 
	    GT2005_write_cmos_sensor(0x0344, 0x3C); 
	    GT2005_write_cmos_sensor(0x0400, 0xE8); 
	    GT2005_write_cmos_sensor(0x0401, 0x40); 
	    GT2005_write_cmos_sensor(0x0402, 0x00); 
	    GT2005_write_cmos_sensor(0x0403, 0x00); 
	    GT2005_write_cmos_sensor(0x0404, 0xF8); 
	    GT2005_write_cmos_sensor(0x0405, 0x08); 
	    GT2005_write_cmos_sensor(0x0406, 0x08); 
	    GT2005_write_cmos_sensor(0x0407, 0x89); 
	    GT2005_write_cmos_sensor(0x0408, 0x44); 
	    GT2005_write_cmos_sensor(0x0409, 0x1F); 
	    GT2005_write_cmos_sensor(0x040A, 0x40); //00_1018
	    GT2005_write_cmos_sensor(0x040B, 0x44); //f6_1018 
#if 0
		GT2005_write_cmos_sensor(0x040C, 0xE0); 
		GT2005_write_cmos_sensor(0x040D, 0x00); 
		GT2005_write_cmos_sensor(0x040E, 0x00); 
		GT2005_write_cmos_sensor(0x040F, 0x00); 
		GT2005_write_cmos_sensor(0x0410, 0x09); 
		GT2005_write_cmos_sensor(0x0411, 0x09); 
		GT2005_write_cmos_sensor(0x0412, 0x0A); 
		GT2005_write_cmos_sensor(0x0413, 0x0E); 
		GT2005_write_cmos_sensor(0x0414, 0x08); 
		GT2005_write_cmos_sensor(0x0415, 0x08); 
		GT2005_write_cmos_sensor(0x0416, 0x07); 
		GT2005_write_cmos_sensor(0x0417, 0x00); 
		GT2005_write_cmos_sensor(0x0418, 0x07); 
		GT2005_write_cmos_sensor(0x0419, 0x07); 
		GT2005_write_cmos_sensor(0x041A, 0x09); 
		GT2005_write_cmos_sensor(0x041B, 0x0C); 
		GT2005_write_cmos_sensor(0x041C, 0x07); 
		GT2005_write_cmos_sensor(0x041D, 0x07); 
		GT2005_write_cmos_sensor(0x041E, 0x06); 
		GT2005_write_cmos_sensor(0x041F, 0x07); 
		GT2005_write_cmos_sensor(0x0420, 0x3F); 
		GT2005_write_cmos_sensor(0x0421, 0x3F); 
		GT2005_write_cmos_sensor(0x0422, 0x42); 
		GT2005_write_cmos_sensor(0x0423, 0x35); 
		GT2005_write_cmos_sensor(0x0424, 0x4F); 
		GT2005_write_cmos_sensor(0x0425, 0x50); 
		GT2005_write_cmos_sensor(0x0426, 0x54); 
		GT2005_write_cmos_sensor(0x0427, 0x46); 
		GT2005_write_cmos_sensor(0x0428, 0x26); 
		GT2005_write_cmos_sensor(0x0429, 0x26); 
		GT2005_write_cmos_sensor(0x042A, 0x2C); 
		GT2005_write_cmos_sensor(0x042B, 0x21); 
		GT2005_write_cmos_sensor(0x042C, 0x25); 
		GT2005_write_cmos_sensor(0x042D, 0x25); 
		GT2005_write_cmos_sensor(0x042E, 0x28); 
		GT2005_write_cmos_sensor(0x042F, 0x20); 
		GT2005_write_cmos_sensor(0x0430, 0x3E); 
		GT2005_write_cmos_sensor(0x0431, 0x3E); 
		GT2005_write_cmos_sensor(0x0432, 0x2E); 
		GT2005_write_cmos_sensor(0x0433, 0x25); 
		GT2005_write_cmos_sensor(0x0434, 0x4E); 
		GT2005_write_cmos_sensor(0x0435, 0x32); 
		GT2005_write_cmos_sensor(0x0436, 0x38); 
		GT2005_write_cmos_sensor(0x0437, 0x50); 
		GT2005_write_cmos_sensor(0x0438, 0x2B); 
		GT2005_write_cmos_sensor(0x0439, 0x2B); 
		GT2005_write_cmos_sensor(0x043A, 0x38); 
		GT2005_write_cmos_sensor(0x043B, 0x22); 
		GT2005_write_cmos_sensor(0x043C, 0x3B); 
		GT2005_write_cmos_sensor(0x043D, 0x3B); 
		GT2005_write_cmos_sensor(0x043E, 0x31); 
		GT2005_write_cmos_sensor(0x043F, 0x37); 
		
#endif
	    GT2005_write_cmos_sensor(0x0440, 0x00); 
	    GT2005_write_cmos_sensor(0x0441, 0x63); //5c_1018
	    GT2005_write_cmos_sensor(0x0442, 0x00); 
	    GT2005_write_cmos_sensor(0x0443, 0x00); 
	    GT2005_write_cmos_sensor(0x0444, 0x24); //17_1018
	    GT2005_write_cmos_sensor(0x0445, 0x00); 
	    GT2005_write_cmos_sensor(0x0446, 0x00); 
	    GT2005_write_cmos_sensor(0x0447, 0x00); 
	    GT2005_write_cmos_sensor(0x0448, 0x00); 
	    GT2005_write_cmos_sensor(0x0449, 0x00); 
	    GT2005_write_cmos_sensor(0x044A, 0x00); 
	    GT2005_write_cmos_sensor(0x044D, 0xE0); 
	    GT2005_write_cmos_sensor(0x044E, 0x05); 
	    GT2005_write_cmos_sensor(0x044F, 0x07); 
	    GT2005_write_cmos_sensor(0x0450, 0x00); 
	    GT2005_write_cmos_sensor(0x0451, 0x00); 
	    GT2005_write_cmos_sensor(0x0452, 0x00); 
	    GT2005_write_cmos_sensor(0x0453, 0x00); 
	    GT2005_write_cmos_sensor(0x0454, 0x00); 
	    GT2005_write_cmos_sensor(0x0455, 0x00); 
	    GT2005_write_cmos_sensor(0x0456, 0x00); 
	    GT2005_write_cmos_sensor(0x0457, 0x00); 
	    GT2005_write_cmos_sensor(0x0458, 0x00); 
	    GT2005_write_cmos_sensor(0x0459, 0x00); 
	    GT2005_write_cmos_sensor(0x045A, 0x00); 
	    GT2005_write_cmos_sensor(0x045B, 0x00); 
	    GT2005_write_cmos_sensor(0x045C, 0x00); 
	    GT2005_write_cmos_sensor(0x045D, 0x00); 
	    GT2005_write_cmos_sensor(0x045E, 0x00); 
	    GT2005_write_cmos_sensor(0x045F, 0x00);
	    GT2005_write_cmos_sensor(0x0460, 0x80); 
#if 0
		// gamma curve 5
		GT2005_write_cmos_sensor(0x0461 , 0x00);
		GT2005_write_cmos_sensor(0x0462 , 0x00);
		GT2005_write_cmos_sensor(0x0463 , 0x00);
		GT2005_write_cmos_sensor(0x0464 , 0x00);
		GT2005_write_cmos_sensor(0x0465 , 0x15);
		GT2005_write_cmos_sensor(0x0466 , 0x33);
		GT2005_write_cmos_sensor(0x0467 , 0x61);
		GT2005_write_cmos_sensor(0x0468 , 0x56);
		GT2005_write_cmos_sensor(0x0469 , 0x30);
		GT2005_write_cmos_sensor(0x046A , 0x22);
		GT2005_write_cmos_sensor(0x046B , 0x3E);
		GT2005_write_cmos_sensor(0x046C , 0x2E);
		GT2005_write_cmos_sensor(0x046D , 0x2B);
		GT2005_write_cmos_sensor(0x046E , 0x28);
		GT2005_write_cmos_sensor(0x046F , 0x3C);
		GT2005_write_cmos_sensor(0x0470 , 0x38);
		GT2005_write_cmos_sensor(0x0471 , 0x2F);
		GT2005_write_cmos_sensor(0x0472 , 0x2A);
		GT2005_write_cmos_sensor(0x0473 , 0x3C);
		GT2005_write_cmos_sensor(0x0474 , 0x34);
		GT2005_write_cmos_sensor(0x0475 , 0x31);
		GT2005_write_cmos_sensor(0x0476 , 0x31);
		GT2005_write_cmos_sensor(0x0477 , 0x39);
#endif 
#if 0	//1018
	    GT2005_write_cmos_sensor(0x0600, 0x00); 
	    GT2005_write_cmos_sensor(0x0601, 0x24); 
	    GT2005_write_cmos_sensor(0x0602, 0x45); 
	    GT2005_write_cmos_sensor(0x0603, 0x0E); 
	    GT2005_write_cmos_sensor(0x0604, 0x14); 
	    GT2005_write_cmos_sensor(0x0605, 0x2F); 
	    GT2005_write_cmos_sensor(0x0606, 0x01); 
	    GT2005_write_cmos_sensor(0x0607, 0x0E); 
	    GT2005_write_cmos_sensor(0x0608, 0x0E); 
	    GT2005_write_cmos_sensor(0x0609, 0x37); 
	    GT2005_write_cmos_sensor(0x060A, 0x18); 
	    GT2005_write_cmos_sensor(0x060B, 0xA0); 
	    GT2005_write_cmos_sensor(0x060C, 0x20); 
	    GT2005_write_cmos_sensor(0x060D, 0x07); 
	    GT2005_write_cmos_sensor(0x060E, 0x47); 
	    GT2005_write_cmos_sensor(0x060F, 0x90); 
	    GT2005_write_cmos_sensor(0x0610, 0x06); 
	    GT2005_write_cmos_sensor(0x0611, 0x0C); 
	    GT2005_write_cmos_sensor(0x0612, 0x28); 
	    GT2005_write_cmos_sensor(0x0613, 0x13); 
	    GT2005_write_cmos_sensor(0x0614, 0x0B); 
	    GT2005_write_cmos_sensor(0x0615, 0x10); 
	    GT2005_write_cmos_sensor(0x0616, 0x14); 
	    GT2005_write_cmos_sensor(0x0617, 0x19); 
	    GT2005_write_cmos_sensor(0x0618, 0x52); 
	    GT2005_write_cmos_sensor(0x0619, 0xA0); 
	    GT2005_write_cmos_sensor(0x061A, 0x11); 
	    GT2005_write_cmos_sensor(0x061B, 0x33); 
	    GT2005_write_cmos_sensor(0x061C, 0x56); 
	    GT2005_write_cmos_sensor(0x061D, 0x20); 
	    GT2005_write_cmos_sensor(0x061E, 0x28); 
	    GT2005_write_cmos_sensor(0x061F, 0x2B); 
	    GT2005_write_cmos_sensor(0x0620, 0x22); 
	    GT2005_write_cmos_sensor(0x0621, 0x11); 
	    GT2005_write_cmos_sensor(0x0622, 0x75); 
	    GT2005_write_cmos_sensor(0x0623, 0x49); 
	    GT2005_write_cmos_sensor(0x0624, 0x6E); 
	    GT2005_write_cmos_sensor(0x0625, 0x80); 
	    GT2005_write_cmos_sensor(0x0626, 0x02); 
	    GT2005_write_cmos_sensor(0x0627, 0x0C); 
	    GT2005_write_cmos_sensor(0x0628, 0x51); 
	    GT2005_write_cmos_sensor(0x0629, 0x25); 
	    GT2005_write_cmos_sensor(0x062A, 0x01); 
	    GT2005_write_cmos_sensor(0x062B, 0x3D); 
	    GT2005_write_cmos_sensor(0x062C, 0x04); 
	    GT2005_write_cmos_sensor(0x062D, 0x01); 
	    GT2005_write_cmos_sensor(0x062E, 0x0C); 
	    GT2005_write_cmos_sensor(0x062F, 0x2C); 
	    GT2005_write_cmos_sensor(0x0630, 0x0D); 
	    GT2005_write_cmos_sensor(0x0631, 0x14); 
	    GT2005_write_cmos_sensor(0x0632, 0x12); 
	    GT2005_write_cmos_sensor(0x0633, 0x34); 
	    GT2005_write_cmos_sensor(0x0634, 0x00); 
	    GT2005_write_cmos_sensor(0x0635, 0x00); 
	    GT2005_write_cmos_sensor(0x0636, 0x00); 
	    GT2005_write_cmos_sensor(0x0637, 0xB1); 
	    GT2005_write_cmos_sensor(0x0638, 0x22); 
	    GT2005_write_cmos_sensor(0x0639, 0x32); 
	    GT2005_write_cmos_sensor(0x063A, 0x0E); 
	    GT2005_write_cmos_sensor(0x063B, 0x18); 
	    GT2005_write_cmos_sensor(0x063C, 0x88); 
	    GT2005_write_cmos_sensor(0x0640, 0xB2); 
	    GT2005_write_cmos_sensor(0x0641, 0xC0); 
	    GT2005_write_cmos_sensor(0x0642, 0x01); 
	    GT2005_write_cmos_sensor(0x0643, 0x26); 
	    GT2005_write_cmos_sensor(0x0644, 0x13); 
	    GT2005_write_cmos_sensor(0x0645, 0x88); 
	    GT2005_write_cmos_sensor(0x0646, 0x64); 
	    GT2005_write_cmos_sensor(0x0647, 0x00); 
	    GT2005_write_cmos_sensor(0x0681, 0x1B); 
	    GT2005_write_cmos_sensor(0x0682, 0xA0); 
	    GT2005_write_cmos_sensor(0x0683, 0x28); 
	    GT2005_write_cmos_sensor(0x0684, 0x00); 
	    GT2005_write_cmos_sensor(0x0685, 0xB0); 
	    GT2005_write_cmos_sensor(0x0686, 0x6F); 
	    GT2005_write_cmos_sensor(0x0687, 0x33); 
	    GT2005_write_cmos_sensor(0x0688, 0x1F); 
	    GT2005_write_cmos_sensor(0x0689, 0x44); 
	    GT2005_write_cmos_sensor(0x068A, 0xA8); 
	    GT2005_write_cmos_sensor(0x068B, 0x44); 
	    GT2005_write_cmos_sensor(0x068C, 0x08); 
	    GT2005_write_cmos_sensor(0x068D, 0x08); 
	    GT2005_write_cmos_sensor(0x068E, 0x00); 
	    GT2005_write_cmos_sensor(0x068F, 0x00); 
	    GT2005_write_cmos_sensor(0x0690, 0x01); 
	    GT2005_write_cmos_sensor(0x0691, 0x00); 
	    GT2005_write_cmos_sensor(0x0692, 0x01); 
	    GT2005_write_cmos_sensor(0x0693, 0x00); 
	    GT2005_write_cmos_sensor(0x0694, 0x00); 
	    GT2005_write_cmos_sensor(0x0695, 0x00); 
	    GT2005_write_cmos_sensor(0x0696, 0x00); 
	    GT2005_write_cmos_sensor(0x0697, 0x00); 
	    GT2005_write_cmos_sensor(0x0698, 0x2A); 
	    GT2005_write_cmos_sensor(0x0699, 0x80); 
	    GT2005_write_cmos_sensor(0x069A, 0x1F); 
	    GT2005_write_cmos_sensor(0x069B, 0x00); 
	    GT2005_write_cmos_sensor(0x069C, 0x03); 
	    GT2005_write_cmos_sensor(0x069D, 0x09); 
	    GT2005_write_cmos_sensor(0x069E, 0x03); 
	    GT2005_write_cmos_sensor(0x069F, 0x0F); 
	    GT2005_write_cmos_sensor(0x06A0, 0x22); 
	    GT2005_write_cmos_sensor(0x06A1, 0x98); 
	    GT2005_write_cmos_sensor(0x06A2, 0x22); 
	    GT2005_write_cmos_sensor(0x06A3, 0xA4); 
	    GT2005_write_cmos_sensor(0x06A4, 0x00); 
	    GT2005_write_cmos_sensor(0x06A5, 0x2F); 
	    GT2005_write_cmos_sensor(0x06A6, 0x00); 
	    GT2005_write_cmos_sensor(0x06A7, 0x2F); 
	    GT2005_write_cmos_sensor(0x0F00, 0x00); 
	    GT2005_write_cmos_sensor(0x0F01, 0x00); 
#endif
	    GT2005_write_cmos_sensor(0x0104, 0x03); //0x03;H,V are low
	    GT2005_write_cmos_sensor(0x0100, 0x01); 
	    GT2005_write_cmos_sensor(0x0102, 0x02); 
		 GT2005_LSC();
		GT2005_Gamma_Select(5);
	    Sleep(500);
    	    printk("%s, open end\n", __func__);
	    return ERROR_NONE;
} /* GT2005_YUV_sensor_initial_setting */

//==========================================================

void GT2005_H_V_Switch(kal_uint8 direction)
{
	printk("%s\n", __func__);
	switch(direction) 
	{
		case 1:  // normal
	    GT2005_write_cmos_sensor(0x0101, 0x00);
		  break;
		case 2:  // IMAGE_H_MIRROR
	    GT2005_write_cmos_sensor(0x0101, 0x01);
		  break;		  
		case 3:  // IMAGE_V_MIRROR
	    GT2005_write_cmos_sensor(0x0101, 0x02);
  	  break;	
		case 4:  // IMAGE_HV_MIRROR
	    GT2005_write_cmos_sensor(0x0101, 0x03);
		  break;		    
	} 
}
/*************************************************************************
* FUNCTION
*	GT2005_GetPeriod
*
* DESCRIPTION
*	This function return the image width and height of image sensor.
*
* PARAMETERS
*	*pixel_number : address pointer of pixel numbers in one period of HSYNC
*  *line_number : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void GT2005_GetPeriod(kal_uint16 *pixel_number, kal_uint16 *line_number)
{
	printk("%s\n", __func__);
	*pixel_number=GT2005_PV_PERIOD_PIXEL_NUMS;	/* pixel numbers in one period of HSYNC */
	*line_number=GT2005_PV_PERIOD_LINE_NUMS;		/* line numbers in one period of VSYNC */
}	/* GT2005_GetPeriod */




/*************************************************************************
* FUNCTION
*	GT2005Close
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
UINT32 GT2005GetSensorID(UINT32 *sensorID) 
{
	volatile signed char i;
	kal_uint16 sensor_id=0;
	
	for(i=0;i<3;i++)
	{
		*sensorID = (GT2005_read_cmos_sensor(0x0000) << 8) | GT2005_read_cmos_sensor(0x0001);
		printk("%s sensor_id=%d\n", __func__, *sensorID);
		if(*sensorID != GT2005_SENSOR_ID)
		{
			*sensorID = 0xFFFFFFFF;
			return ERROR_SENSOR_CONNECT_FAIL;
		}
	}
	
	return ERROR_NONE;
}
UINT32 GT2005Close(void)
{
	printk("%s\n", __func__);
//	CISModulePowerOn(FALSE);


	return ERROR_NONE;
}	/* GT2005Close() */

/*************************************************************************
* FUNCTION
*	GT2005Preview
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
UINT32 GT2005Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{

	printk("%s\n", __func__);
	kal_uint8 iTemp, temp_AE_reg, temp_AWB_reg;
    kal_uint16 iDummyPixels = 0, iDummyLines = 0, iStartX = 0, iStartY = 0;
     
    GT2005_sensor_cap_state = KAL_FALSE;
	GT2005_write_cmos_sensor(0x0300 , 0x81);  
	GT2005_write_cmos_sensor(0x0109 , 0x00);
	GT2005_write_cmos_sensor(0x010A , 0x00);	//00_1018
	GT2005_write_cmos_sensor(0x010B , 0x0F);					
	GT2005_write_cmos_sensor(0x0110 , 0x03);					
	GT2005_write_cmos_sensor(0x0111 , 0x20);					
	GT2005_write_cmos_sensor(0x0112 , 0x02);					
	GT2005_write_cmos_sensor(0x0113 , 0x58); 
	

	if (CapState)
    {
        //GT2005_write_shutter(Shutter);
    }
	GT2005_set_AE_mode(KAL_TRUE);
	CapState = KAL_FALSE;
    image_window->GrabStartX = 1;//iStartX;
    image_window->GrabStartY = 1;//iStartY;
    image_window->ExposureWindowWidth = GT2005_IMAGE_SENSOR_PV_WIDTH ;
    image_window->ExposureWindowHeight = GT2005_IMAGE_SENSOR_PV_HEIGHT;
    
    GT2005_DELAY_AFTER_PREVIEW = 1;

	// copy sensor_config_data
	memcpy(&GT2005SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
  	return ERROR_NONE;
}	/* GT2005Preview() */

UINT32 GT2005Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{

	printk("%s\n", __func__);
    volatile kal_uint32 shutter = GT2005_exposure_lines, temp_reg;
    kal_uint8 temp_AE_reg, temp;
    kal_uint16 AE_setting_delay = 0;

    GT2005_sensor_cap_state = KAL_TRUE;
 
	   GT2005_write_cmos_sensor(0x0300, 0xC1); 

    //Shutter = GT2005_read_shutter();
	
    if ((image_window->ImageTargetWidth<=GT2005_IMAGE_SENSOR_PV_WIDTH)&&
        (image_window->ImageTargetHeight<=GT2005_IMAGE_SENSOR_PV_HEIGHT))
    {    /* Less than PV Mode */ 


        GT2005_gPVmode=KAL_TRUE;

        GT2005_dummy_pixels = 0;
        GT2005_dummy_lines = 0;

        GT2005_capture_pclk_in_M = GT2005_preview_pclk_in_M;   //Don't change the clk
   
        
        
    image_window->GrabStartX = GT2005_PV_GRAB_START_X;//iStartX;
    image_window->GrabStartY = GT2005_PV_GRAB_START_Y;//iStartY;
    image_window->ExposureWindowWidth = GT2005_IMAGE_SENSOR_PV_WIDTH ;
    image_window->ExposureWindowHeight = GT2005_IMAGE_SENSOR_PV_HEIGHT;
		

    }
    else 
    {    
    /*
		if (CapState)
		{
			GT2005_write_shutter(Shutter);
		}
		else
		{
			shutter = Shutter;
			GT2005_write_shutter(shutter);	
		}
	*/
    	 /* 2M FULL Mode */
		//GT2005_write_cmos_sensor(0x0300 , 0xc1); 
		GT2005_write_cmos_sensor(0x0109 , 0x01);
		GT2005_write_cmos_sensor(0x010A , 0x00);	
		GT2005_write_cmos_sensor(0x010B , 0x00);					
		GT2005_write_cmos_sensor(0x0110 , 0x06);					
		GT2005_write_cmos_sensor(0x0111 , 0x40);					
		GT2005_write_cmos_sensor(0x0112 , 0x04);					
		GT2005_write_cmos_sensor(0x0113 , 0xb0);
		CapState = KAL_TRUE;
		GT2005_gPVmode = KAL_FALSE;

		GT2005_Capture_Dummy_Pixels = GT2005_dummy_pixels ;
		GT2005_Capture_Dummy_Lines = GT2005_dummy_lines;

Sleep(500);
		image_window->GrabStartX = 1;//iStartX;
		image_window->GrabStartY = 1;//iStartY;
		image_window->ExposureWindowWidth=GT2005_IMAGE_SENSOR_FULL_HEIGHT ;
		image_window->ExposureWindowHeight=GT2005_IMAGE_SENSOR_FULL_HEIGHT;
	}

  

    GT2005_DELAY_AFTER_PREVIEW = 4;  // 2

	// copy sensor_config_data
	memcpy(&GT2005SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
       
	//Sleep(500);
	return ERROR_NONE;
}	/* GT2005Capture() */

UINT32 GT2005GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
	printk("%s\n", __func__);
	pSensorResolution->SensorFullWidth=GT2005_FULL_GRAB_WIDTH;  //modify by yanxu
	pSensorResolution->SensorFullHeight=GT2005_FULL_GRAB_HEIGHT;
	pSensorResolution->SensorPreviewWidth=GT2005_PV_GRAB_WIDTH ;
	pSensorResolution->SensorPreviewHeight=GT2005_PV_GRAB_HEIGHT;

	//pSensorResolution->SensorVideoWidth=GT2005_PV_GRAB_WIDTH;
	//pSensorResolution->SensorVideoHeight=GT2005_PV_GRAB_HEIGHT;


	return ERROR_NONE;
}	/* GT2005GetResolution() */

UINT32 GT2005GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
					  MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
					  
{
	pSensorInfo->SensorPreviewResolutionX=GT2005_PV_GRAB_WIDTH;
	pSensorInfo->SensorPreviewResolutionY=GT2005_PV_GRAB_HEIGHT;
	pSensorInfo->SensorFullResolutionX=GT2005_FULL_GRAB_WIDTH;
	pSensorInfo->SensorFullResolutionY=GT2005_FULL_GRAB_HEIGHT;

	pSensorInfo->SensorCameraPreviewFrameRate=30;
	pSensorInfo->SensorVideoFrameRate=30;
	pSensorInfo->SensorStillCaptureFrameRate=10;
	pSensorInfo->SensorWebCamCaptureFrameRate=15;
	pSensorInfo->SensorResetActiveHigh=FALSE;
	pSensorInfo->SensorResetDelayCount=1;

	pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_UYVY;//SENSOR_OUTPUT_FORMAT_YUYV;
	pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;	
	pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;

	pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_HIGH;
	pSensorInfo->SensorInterruptDelayLines = 1;
	pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_PARALLEL;

	pSensorInfo->CaptureDelayFrame = 3; 
	pSensorInfo->PreviewDelayFrame = 5; 
	pSensorInfo->VideoDelayFrame = 20; 
	pSensorInfo->SensorMasterClockSwitch = 0; 
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;   	
	
	switch (ScenarioId)
	{
	
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		//case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
		pSensorInfo->SensorClockFreq=26;
		pSensorInfo->SensorClockDividCount=	3;
		pSensorInfo->SensorClockRisingCount= 0;
		pSensorInfo->SensorClockFallingCount= 2;
		pSensorInfo->SensorPixelClockCount= 3;
		pSensorInfo->SensorDataLatchCount= 2;
                /*ergate-004*/
		pSensorInfo->SensorGrabStartX = GT2005_PV_GRAB_START_X;//0; 
		pSensorInfo->SensorGrabStartY = GT2005_PV_GRAB_START_Y;//0; 
		printk("MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4");
		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		//case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
		pSensorInfo->SensorClockFreq=26;
		pSensorInfo->SensorClockDividCount=	3;
		pSensorInfo->SensorClockRisingCount= 0;
		pSensorInfo->SensorClockFallingCount= 2;
		pSensorInfo->SensorPixelClockCount= 3;
		pSensorInfo->SensorDataLatchCount= 2;
                /*ergate-004*/
		pSensorInfo->SensorGrabStartX = GT2005_FULL_GRAB_START_X;//0; 
		pSensorInfo->SensorGrabStartY = GT2005_FULL_GRAB_START_Y;//0;
		printk("MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM");
		break;
		default:
		pSensorInfo->SensorClockFreq=26;
		pSensorInfo->SensorClockDividCount=3;
		pSensorInfo->SensorClockRisingCount=0;
		pSensorInfo->SensorClockFallingCount=2;
		pSensorInfo->SensorPixelClockCount=3;
		pSensorInfo->SensorDataLatchCount=2;
                /*ergate-004*/
		pSensorInfo->SensorGrabStartX = GT2005_PV_GRAB_START_X;//0; 
		pSensorInfo->SensorGrabStartY = GT2005_PV_GRAB_START_Y;//0;
		printk("jinyunshan default");
		break;
	}
	//GT2005_PixelClockDivider=pSensorInfo->SensorPixelClockCount;
	memcpy(pSensorConfigData, &GT2005SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	return ERROR_NONE;
}	/* GT2005GetInfo() */

UINT32 GT2005Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	printk("%s\n", __func__);
	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		//case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
			GT2005Preview(pImageWindow, pSensorConfigData);
		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		//case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
			GT2005Capture(pImageWindow, pSensorConfigData);
		break;
		default:
		    break; 
	}
	return TRUE;
}	/* GT2005Control() */

/* [TC] YUV sensor */	
#if WINMO_USE
void GT2005Query(PMSDK_FEATURE_INFO_STRUCT pSensorFeatureInfo)
{
	MSDK_FEATURE_TYPE_RANGE_STRUCT *pFeatureRange;
	MSDK_FEATURE_TYPE_MULTI_SELECTION_STRUCT *pFeatureMultiSelection;
	switch (pSensorFeatureInfo->FeatureId)
	{
		case ISP_FEATURE_DSC_MODE:
			pSensorFeatureInfo->FeatureType = MSDK_FEATURE_TYPE_MULTI_SELECTION;
			pSensorFeatureInfo->FeatureSupported = (UINT8)(MSDK_SET_GET_FEATURE_SUPPORTED|MSDK_QUERY_CAMERA_SUPPORTED);
			pFeatureMultiSelection = (PMSDK_FEATURE_TYPE_MULTI_SELECTION_STRUCT)(&pSensorFeatureInfo->FeatureInformation.FeatureMultiSelection);
			pFeatureMultiSelection->TotalSelection = CAM_NO_OF_SCENE_MODE_MAX;
			pFeatureMultiSelection->DefaultSelection = CAM_AUTO_DSC_MODE;
			pFeatureMultiSelection->SupportedSelection = 
				(CAMERA_FEATURE_SUPPORT(CAM_AUTO_DSC_MODE)|
				CAMERA_FEATURE_SUPPORT(CAM_NIGHTSCENE_MODE));			
		break;
		case ISP_FEATURE_WHITEBALANCE:
			pSensorFeatureInfo->FeatureType = MSDK_FEATURE_TYPE_MULTI_SELECTION;
			pSensorFeatureInfo->FeatureSupported = (UINT8)(MSDK_SET_GET_FEATURE_SUPPORTED|MSDK_QUERY_CAMERA_VIDEO_SUPPORTED);
			pFeatureMultiSelection = (PMSDK_FEATURE_TYPE_MULTI_SELECTION_STRUCT)(&pSensorFeatureInfo->FeatureInformation.FeatureMultiSelection);
			pFeatureMultiSelection->TotalSelection = CAM_NO_OF_WB;
			pFeatureMultiSelection->DefaultSelection = CAM_WB_AUTO;
			pFeatureMultiSelection->SupportedSelection = 
				(CAMERA_FEATURE_SUPPORT(CAM_WB_AUTO)|
				CAMERA_FEATURE_SUPPORT(CAM_WB_CLOUD)|
				CAMERA_FEATURE_SUPPORT(CAM_WB_DAYLIGHT)|
				CAMERA_FEATURE_SUPPORT(CAM_WB_INCANDESCENCE)|
				CAMERA_FEATURE_SUPPORT(CAM_WB_TUNGSTEN)|
				CAMERA_FEATURE_SUPPORT(CAM_WB_FLUORESCENT));
		break;
		case ISP_FEATURE_IMAGE_EFFECT:
			pSensorFeatureInfo->FeatureType = MSDK_FEATURE_TYPE_MULTI_SELECTION;
			pSensorFeatureInfo->FeatureSupported = (UINT8)(MSDK_SET_GET_FEATURE_SUPPORTED|MSDK_QUERY_CAMERA_VIDEO_SUPPORTED);
			pFeatureMultiSelection = (PMSDK_FEATURE_TYPE_MULTI_SELECTION_STRUCT)(&pSensorFeatureInfo->FeatureInformation.FeatureMultiSelection);
			pFeatureMultiSelection->TotalSelection = CAM_NO_OF_EFFECT_ENC;
			pFeatureMultiSelection->DefaultSelection = CAM_EFFECT_ENC_NORMAL;
			pFeatureMultiSelection->SupportedSelection = 
				(CAMERA_FEATURE_SUPPORT(CAM_EFFECT_ENC_NORMAL)|
				CAMERA_FEATURE_SUPPORT(CAM_EFFECT_ENC_GRAYSCALE)|
				CAMERA_FEATURE_SUPPORT(CAM_EFFECT_ENC_COLORINV)|
				CAMERA_FEATURE_SUPPORT(CAM_EFFECT_ENC_SEPIAGREEN)|
				CAMERA_FEATURE_SUPPORT(CAM_EFFECT_ENC_SEPIABLUE)|
				CAMERA_FEATURE_SUPPORT(CAM_EFFECT_ENC_SEPIA));	
		break;
		case ISP_FEATURE_AE_METERING_MODE:
			pSensorFeatureInfo->FeatureSupported = MSDK_FEATURE_NOT_SUPPORTED;
		break;
		case ISP_FEATURE_BRIGHTNESS:
			pSensorFeatureInfo->FeatureType = MSDK_FEATURE_TYPE_RANGE;
			pSensorFeatureInfo->FeatureSupported = (UINT8)(MSDK_SET_GET_FEATURE_SUPPORTED|MSDK_QUERY_CAMERA_VIDEO_SUPPORTED);
			pFeatureRange = (PMSDK_FEATURE_TYPE_RANGE_STRUCT)(&pSensorFeatureInfo->FeatureInformation.FeatureRange);
			pFeatureRange->MinValue = CAM_EV_NEG_4_3;
			pFeatureRange->MaxValue = CAM_EV_POS_4_3;
			pFeatureRange->StepValue = CAMERA_FEATURE_ID_EV_STEP;
			pFeatureRange->DefaultValue = CAM_EV_ZERO;
		break;
		case ISP_FEATURE_BANDING_FREQ:
			pSensorFeatureInfo->FeatureType = MSDK_FEATURE_TYPE_MULTI_SELECTION;
			pSensorFeatureInfo->FeatureSupported = (UINT8)(MSDK_SET_GET_FEATURE_SUPPORTED|MSDK_QUERY_CAMERA_VIDEO_SUPPORTED);
			pFeatureMultiSelection = (PMSDK_FEATURE_TYPE_MULTI_SELECTION_STRUCT)(&pSensorFeatureInfo->FeatureInformation.FeatureMultiSelection);
			pFeatureMultiSelection->TotalSelection = CAM_NO_OF_BANDING;
			pFeatureMultiSelection->DefaultSelection = CAM_BANDING_50HZ;
			pFeatureMultiSelection->SupportedSelection = 
				(CAMERA_FEATURE_SUPPORT(CAM_BANDING_50HZ)|
				CAMERA_FEATURE_SUPPORT(CAM_BANDING_60HZ));
		break;
		case ISP_FEATURE_AF_OPERATION:
			pSensorFeatureInfo->FeatureSupported = MSDK_FEATURE_NOT_SUPPORTED;
		break;
		case ISP_FEATURE_AF_RANGE_CONTROL:
			pSensorFeatureInfo->FeatureSupported = MSDK_FEATURE_NOT_SUPPORTED;
		break;
		case ISP_FEATURE_FLASH:
			pSensorFeatureInfo->FeatureSupported = MSDK_FEATURE_NOT_SUPPORTED;			
		break;
		case ISP_FEATURE_VIDEO_SCENE_MODE:
			pSensorFeatureInfo->FeatureType = MSDK_FEATURE_TYPE_MULTI_SELECTION;
			pSensorFeatureInfo->FeatureSupported = (UINT8)(MSDK_SET_GET_FEATURE_SUPPORTED|MSDK_QUERY_VIDEO_SUPPORTED);
			pFeatureMultiSelection = (PMSDK_FEATURE_TYPE_MULTI_SELECTION_STRUCT)(&pSensorFeatureInfo->FeatureInformation.FeatureMultiSelection);
			pFeatureMultiSelection->TotalSelection = CAM_NO_OF_SCENE_MODE_MAX;
			pFeatureMultiSelection->DefaultSelection = CAM_VIDEO_AUTO_MODE;
			pFeatureMultiSelection->SupportedSelection = 
				(CAMERA_FEATURE_SUPPORT(CAM_VIDEO_AUTO_MODE)|
				CAMERA_FEATURE_SUPPORT(CAM_VIDEO_NIGHT_MODE));
		break;
		case ISP_FEATURE_ISO:
			pSensorFeatureInfo->FeatureSupported = MSDK_FEATURE_NOT_SUPPORTED;			
		break;
		default:
			pSensorFeatureInfo->FeatureSupported = MSDK_FEATURE_NOT_SUPPORTED;			
		break;
	}
}
#endif 

BOOL GT2005_set_param_wb(UINT16 para)
{
	printk("%s\n", __func__);
    kal_uint8  temp_reg;

    temp_reg=GT2005_read_cmos_sensor(0x3306);

    switch (para)
    {
        case AWB_MODE_OFF:
            GT2005_AWB_ENABLE = KAL_FALSE; 
            GT2005_set_AWB_mode(GT2005_AWB_ENABLE);
            break;                     
        case AWB_MODE_AUTO:
            GT2005_AWB_ENABLE = KAL_TRUE;  //
            GT2005_set_AWB_mode(GT2005_AWB_ENABLE);
			GT2005_write_cmos_sensor(0x031a,0x81);   // select Auto WB
			GT2005_write_cmos_sensor(0x0320,0x24);//  WBGRMAX[7:0];
			GT2005_write_cmos_sensor(0x0321,0x14);//  WBGRMIN[7:0];
			GT2005_write_cmos_sensor(0x0322,0x1A);//  WBGBMAX[7:0];
			GT2005_write_cmos_sensor(0x0323,0x24);//  WBGBMIN[7:0];
			GT2005_write_cmos_sensor(0x0441,0x63);//  PWBGAINR[7:0];
			GT2005_write_cmos_sensor(0x0442,0x00);//  PWBGAINGR[7:0];
			GT2005_write_cmos_sensor(0x0443,0x00);//  PWBGAINGB[7:0];
			GT2005_write_cmos_sensor(0x0444,0x24);//  PWBGAINB[7:0];
            break;

        case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy
			GT2005_write_cmos_sensor(0x0320 ,0x02);//  WBGRMAX[7:0];
			GT2005_write_cmos_sensor(0x0321 ,0x02);//  WBGRMIN[7:0];
			GT2005_write_cmos_sensor(0x0322 ,0x02);//  WBGBMAX[7:0];
			GT2005_write_cmos_sensor(0x0323 ,0x02);//  WBGBMIN[7:0];
			GT2005_write_cmos_sensor(0x0441 ,0x72);//  PWBGAINR[7:0];
			GT2005_write_cmos_sensor(0x0442 ,0x00);//  PWBGAINGR[7:0];
			GT2005_write_cmos_sensor(0x0443 ,0x00);//  PWBGAINGB[7:0];
			GT2005_write_cmos_sensor(0x0444 ,0x0D);//  PWBGAINB[7:0];
            break;

        case AWB_MODE_DAYLIGHT: //sunny
			GT2005_write_cmos_sensor(0x0320 ,0x02);//  WBGRMAX[7:0];
			GT2005_write_cmos_sensor(0x0321 ,0x02);//  WBGRMIN[7:0];
			GT2005_write_cmos_sensor(0x0322 ,0x02);//  WBGBMAX[7:0];
			GT2005_write_cmos_sensor(0x0323 ,0x02);//  WBGBMIN[7:0];
			GT2005_write_cmos_sensor(0x0441 ,0x60);//  PWBGAINR[7:0];
			GT2005_write_cmos_sensor(0x0442 ,0x00);//  PWBGAINGR[7:0];
			GT2005_write_cmos_sensor(0x0443 ,0x00);//  PWBGAINGB[7:0];
			GT2005_write_cmos_sensor(0x0444 ,0x14);//  PWBGAINB[7:0];
            break;

        case AWB_MODE_INCANDESCENT: //office
			GT2005_write_cmos_sensor(0x0320 , 0x02);
			GT2005_write_cmos_sensor(0x0321 , 0x02);
			GT2005_write_cmos_sensor(0x0322 , 0x02);
			GT2005_write_cmos_sensor(0x0323 , 0x02);
			GT2005_write_cmos_sensor(0x0441 , 0x50);
			GT2005_write_cmos_sensor(0x0442 , 0x00);
			GT2005_write_cmos_sensor(0x0443 , 0x00);
			GT2005_write_cmos_sensor(0x0444 , 0x30);
            break;

        case AWB_MODE_TUNGSTEN: //home
			GT2005_write_cmos_sensor(0x0320 ,0x02);//  WBGRMAX[7:0];
			GT2005_write_cmos_sensor(0x0321 ,0x02);//  WBGRMIN[7:0];
			GT2005_write_cmos_sensor(0x0322 ,0x02);//  WBGBMAX[7:0];
			GT2005_write_cmos_sensor(0x0323 ,0x02);//  WBGBMIN[7:0];
			GT2005_write_cmos_sensor(0x0441 ,0x0B);//  PWBGAINR[7:0];
			GT2005_write_cmos_sensor(0x0442 ,0x00);//  PWBGAINGR[7:0];
			GT2005_write_cmos_sensor(0x0443 ,0x00);//  PWBGAINGB[7:0];
			GT2005_write_cmos_sensor(0x0444 ,0x5E);//  PWBGAINB[7:0];
            break;

        case AWB_MODE_FLUORESCENT:
      			GT2005_write_cmos_sensor(0x0320 ,0x02);//  WBGRMAX[7:0];
			GT2005_write_cmos_sensor(0x0321 ,0x02);//  WBGRMIN[7:0];
			GT2005_write_cmos_sensor(0x0322 ,0x02);//  WBGBMAX[7:0];
			GT2005_write_cmos_sensor(0x0323 ,0x02);//  WBGBMIN[7:0];
			GT2005_write_cmos_sensor(0x0441 ,0x43);//  PWBGAINR[7:0];
			GT2005_write_cmos_sensor(0x0442 ,0x00);//  PWBGAINGR[7:0];
			GT2005_write_cmos_sensor(0x0443 ,0x00);//  PWBGAINGB[7:0];
			GT2005_write_cmos_sensor(0x0444 ,0x4B);//  PWBGAINB[7:0];
            break;


        default:
            return FALSE;
    }

    return TRUE;
} /* GT2005_set_param_wb */

BOOL GT2005_set_param_effect(UINT16 para)
{
	printk("%s\n", __func__);
   BOOL  ret = TRUE;
   //UINT8  temp_reg;
   //temp_reg=GT2005_read_cmos_sensor(0x3391);
    switch (para)
    {
        case MEFFECT_OFF:

		GT2005_write_cmos_sensor(0x0115,0x00);
	break;

        case MEFFECT_SEPIA:
	     	GT2005_write_cmos_sensor(0x0115,0x0a);
			GT2005_write_cmos_sensor(0x026e,0x60);
			GT2005_write_cmos_sensor(0x026f,0xa0);
            break;

        case MEFFECT_NEGATIVE:
			GT2005_write_cmos_sensor(0x0115,0x09);
            break;

        case MEFFECT_SEPIAGREEN:
			GT2005_write_cmos_sensor(0x0115,0x0a);
			GT2005_write_cmos_sensor(0x026e,0x20);
			GT2005_write_cmos_sensor(0x026f,0x00);
            break;

        case MEFFECT_SEPIABLUE:
			GT2005_write_cmos_sensor(0x0115,0x0a);
			GT2005_write_cmos_sensor(0x026e,0xfb);
			GT2005_write_cmos_sensor(0x026f,0x00);
            break;
	case MEFFECT_MONO: //B&W
			GT2005_write_cmos_sensor(0x0115,0x06);
		break;

        default:
            ret = FALSE;
    }

    return ret;

} /* GT2005_set_param_effect */
BOOL GT2005_set_param_banding(UINT16 para)
{
	printk("%s\n", __func__);
    kal_uint8 banding;

    switch (para)
    {
        case AE_FLICKER_MODE_50HZ:
			
           GT2005_Banding_setting = AE_FLICKER_MODE_50HZ;

           GT2005_preview_pclk_in_M = 335;
           if (GT2005_VEDIO_encode_mode)
           {
			GT2005_write_cmos_sensor(0x0315, 0x16);                  			
           }
           else
           {
           
			GT2005_write_cmos_sensor(0x0315, 0x16);                  			
           }
            break;

        case AE_FLICKER_MODE_60HZ:			
            GT2005_preview_pclk_in_M = 390;
            GT2005_Banding_setting = AE_FLICKER_MODE_60HZ;			

		GT2005_write_cmos_sensor(0x0315, 0x56);                  			
            break;

          default:
              return FALSE;
    }

    return TRUE;
} /* GT2005_set_param_banding */

BOOL GT2005_set_param_exposure(UINT16 para)
{
	printk("%s\n", __func__);
    kal_uint8  temp_reg;

    //temp_reg=GT2005_read_cmos_sensor(0x0300);

    switch (para)
    {
        case AE_EV_COMP_n13:
			GT2005_write_cmos_sensor(0x0201, 0x85);
            break;

        case AE_EV_COMP_n10:    
			GT2005_write_cmos_sensor(0x0201, 0xa4);
            break;

        case AE_EV_COMP_n07:
			GT2005_write_cmos_sensor(0x0201, 0xca);
		break;
        case AE_EV_COMP_n03:
			GT2005_write_cmos_sensor(0x0201, 0xf0);
            break;

        case AE_EV_COMP_00:
			GT2005_write_cmos_sensor(0x0201, 0x02);//00 betty 04_1018 /0xfc_1024
            break;

        case AE_EV_COMP_03:
			GT2005_write_cmos_sensor(0x0201, 0x40);
            break;

        case AE_EV_COMP_07:
			GT2005_write_cmos_sensor(0x0201, 0x50);
            break;

        case AE_EV_COMP_10:
			GT2005_write_cmos_sensor(0x0201, 0x60);
            break;

        case AE_EV_COMP_13:
			GT2005_write_cmos_sensor(0x0201, 0x70);
            break;

        default:
            return FALSE;
    }

    return TRUE;
} /* GT2005_set_param_exposure */



UINT32 GT2005YUVSensorSetting(FEATURE_ID iCmd, UINT32 iPara)
{
	printk("%s\n", __func__);
//   if( GT2005_sensor_cap_state == KAL_TRUE)
//	   return TRUE;

	switch (iCmd) {
	case FID_SCENE_MODE:	    
//	    printk("Set Scene Mode:%d\n", iPara); 
	    if (iPara == SCENE_MODE_OFF)
	    {
	        GT2005_night_mode(0); 
	    }
	    else if (iPara == SCENE_MODE_NIGHTSCENE)
	    {
               GT2005_night_mode(1); 
	    }	    
	    break; 	    
	case FID_AWB_MODE:
//	    printk("Set AWB Mode:%d\n", iPara); 	    
           GT2005_set_param_wb(iPara);
	break;
	case FID_COLOR_EFFECT:
//	    printk("Set Color Effect:%d\n", iPara); 	    	    
           GT2005_set_param_effect(iPara);
	break;
	case FID_AE_EV:
#if WINMO_USE	    
	case ISP_FEATURE_EXPOSURE:
#endif 	    
//           printk("Set EV:%d\n", iPara); 	    	    
           GT2005_set_param_exposure(iPara);
	break;
	case FID_AE_FLICKER:
//           printk("Set Flicker:%d\n", iPara); 	    	    	    
           GT2005_set_param_banding(iPara);
	break;
        case FID_AE_SCENE_MODE: 
            if (iPara == AE_MODE_OFF) {
                GT2005_AE_ENABLE = KAL_FALSE; 
            }
            else {
                GT2005_AE_ENABLE = KAL_TRUE; 
	    }
            GT2005_set_AE_mode(GT2005_AE_ENABLE);
            break; 
	case FID_ZOOM_FACTOR:
	    zoom_factor = iPara; 
        break; 
	default:
	break;
	}
	return TRUE;
}   /* GT2005YUVSensorSetting */

UINT32 GT2005YUVSetVideoMode(UINT16 u2FrameRate)
{
	printk("%s\n", __func__);
    kal_uint8 iTemp;
    /* to fix VSYNC, to fix frame rate */
    //printk("Set YUV Video Mode \n");  
//    iTemp = GT2005_read_cmos_sensor(0x3014);
//    GT2005_write_cmos_sensor(0x3014, iTemp & 0xf7); //Disable night mode

    if (u2FrameRate == 30)
    {
 //       GT2005_write_cmos_sensor(0x302d, 0x00);
  //      GT2005_write_cmos_sensor(0x302e, 0x00);
    }
    else if (u2FrameRate == 15)       
    {
 //       GT2005_write_cmos_sensor(0x300e, 0x34);
//        GT2005_write_cmos_sensor(0x302A, GT2005_VIDEO_15FPS_FRAME_LENGTH>>8);  /*  15fps*/
//        GT2005_write_cmos_sensor(0x302B, GT2005_VIDEO_15FPS_FRAME_LENGTH&0xFF);
                
        // clear extra exposure line
 //       GT2005_write_cmos_sensor(0x302d, 0x00);
//        GT2005_write_cmos_sensor(0x302e, 0x00);   
    }
    else 
    {
        printk("Wrong frame rate setting \n");
    }
    GT2005_VEDIO_encode_mode = KAL_TRUE; 
        
    return TRUE;
}

UINT32 GT2005FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
							 UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
	printk("%s\n", __func__);

	UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
	UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
	UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
	UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
	MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;

#if WINMO_USE	
	PMSDK_FEATURE_INFO_STRUCT pSensorFeatureInfo=(PMSDK_FEATURE_INFO_STRUCT) pFeaturePara;
#endif 

	switch (FeatureId)
	{
		case SENSOR_FEATURE_GET_RESOLUTION:
			*pFeatureReturnPara16++=GT2005_IMAGE_SENSOR_FULL_WIDTH;
			*pFeatureReturnPara16=GT2005_IMAGE_SENSOR_FULL_HEIGHT;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_GET_PERIOD:
			*pFeatureReturnPara16++=GT2005_PV_PERIOD_PIXEL_NUMS+GT2005_PV_dummy_pixels;
			*pFeatureReturnPara16=GT2005_PV_PERIOD_LINE_NUMS+GT2005_PV_dummy_lines;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
			*pFeatureReturnPara32 = GT2005_sensor_pclk/10;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_SET_ESHUTTER:
		break;
		case SENSOR_FEATURE_SET_NIGHTMODE:
			GT2005_night_mode((BOOL) *pFeatureData16);
		break;
		case SENSOR_FEATURE_SET_GAIN:
		case SENSOR_FEATURE_SET_FLASHLIGHT:
		break;
		case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			GT2005_isp_master_clock=*pFeatureData32;
		break;
		case SENSOR_FEATURE_SET_REGISTER:
			GT2005_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
		break;
		case SENSOR_FEATURE_GET_REGISTER:
			pSensorRegData->RegData = GT2005_read_cmos_sensor(pSensorRegData->RegAddr);
		break;
		case SENSOR_FEATURE_GET_CONFIG_PARA:
			memcpy(pSensorConfigData, &GT2005SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
			*pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
		break;
		case SENSOR_FEATURE_SET_CCT_REGISTER:
		case SENSOR_FEATURE_GET_CCT_REGISTER:
		case SENSOR_FEATURE_SET_ENG_REGISTER:
		case SENSOR_FEATURE_GET_ENG_REGISTER:
		case SENSOR_FEATURE_GET_REGISTER_DEFAULT:

		case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
		case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
		case SENSOR_FEATURE_GET_GROUP_INFO:
		case SENSOR_FEATURE_GET_ITEM_INFO:
		case SENSOR_FEATURE_SET_ITEM_INFO:
		case SENSOR_FEATURE_GET_ENG_INFO:
		break;
		case SENSOR_FEATURE_GET_GROUP_COUNT:
                        *pFeatureReturnPara32++=0;
                        *pFeatureParaLen=4;	    
		    break; 
		case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
			// get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
			// if EEPROM does not exist in camera module.
			*pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_SET_YUV_CMD:
//		       printk("GT2005 YUV sensor Setting:%d, %d \n", *pFeatureData32,  *(pFeatureData32+1));
			GT2005YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
		break;
#if WINMO_USE		    		
		case SENSOR_FEATURE_QUERY:
			GT2005Query(pSensorFeatureInfo);
			*pFeatureParaLen = sizeof(MSDK_FEATURE_INFO_STRUCT);
		break;		
		case SENSOR_FEATURE_SET_YUV_CAPTURE_RAW_SUPPORT:
			/* update yuv capture raw support flag by *pFeatureData16 */
		break;		
#endif 			
		case SENSOR_FEATURE_SET_VIDEO_MODE:
		       GT2005YUVSetVideoMode(*pFeatureData16);
		       break; 
	case SENSOR_FEATURE_CHECK_SENSOR_ID:
		GT2005GetSensorID(pFeatureData32); 
	break; 				   
		default:
			break;			
	}

	return ERROR_NONE;
}	/* GT2005FeatureControl() */

SENSOR_FUNCTION_STRUCT	SensorFuncGT2005=
{
	GT2005Open,
	GT2005GetInfo,
	GT2005GetResolution,
	GT2005FeatureControl,
	GT2005Control,
	GT2005Close
};

UINT32 GT2005_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncGT2005;

	return ERROR_NONE;
}	/* SensorInit() */

