#include <linux/string.h>
#ifdef BUILD_UBOOT
#include <asm/arch/mt6575_gpio.h>
#else
#include <mach/mt6575_gpio.h>
#endif
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define LSA0_GPIO_PIN (GPIO_DISP_LSA0_PIN)
#define LSCE_GPIO_PIN (GPIO_DISP_LSCE_PIN)
#define LSCK_GPIO_PIN (GPIO_DISP_LSCK_PIN)
#define LSDA_GPIO_PIN (GPIO_DISP_LSDA_PIN)

//chenhaojun
#define SET_GPIO_INPUT(n)  (lcm_util.set_gpio_dir((n), (0)))
#define SET_GPIO_OUTPUT(n)  (lcm_util.set_gpio_dir((n), (1)))

#define FRAME_WIDTH  (480)
#define FRAME_HEIGHT (800)

#define LCM_ID 0x8369
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))
#define SET_GPIO_OUT(n, v)  (lcm_util.set_gpio_out((n), (v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define SET_LSCE_LOW   SET_GPIO_OUT(LSCE_GPIO_PIN, 0)
#define SET_LSCE_HIGH  SET_GPIO_OUT(LSCE_GPIO_PIN, 1)
#define SET_LSCK_LOW   SET_GPIO_OUT(LSCK_GPIO_PIN, 0)
#define SET_LSCK_HIGH  SET_GPIO_OUT(LSCK_GPIO_PIN, 1)
#define SET_LSDA_LOW   SET_GPIO_OUT(LSDA_GPIO_PIN, 0)
#define SET_LSDA_HIGH  SET_GPIO_OUT(LSDA_GPIO_PIN, 1)

#define SET_LA0_INPUT  SET_GPIO_INPUT(LSA0_GPIO_PIN)
#define SET_LA0_OUTPUT  SET_GPIO_OUTPUT(LSA0_GPIO_PIN)

#define GET_LSA0_BIT mt_get_gpio_in(LSA0_GPIO_PIN)
#define CTRL_ID  (0 << 8)
#define DATA_ID  (1 << 8)

static __inline void send_ctrl_cmd(unsigned char cmd)
{
    unsigned char i;

    SET_LSCE_HIGH;
    UDELAY(1);
    SET_LSCK_HIGH;
	UDELAY(1);
    SET_LSDA_HIGH;
    UDELAY(1);
	
    SET_LSCE_LOW;
    UDELAY(1);

	SET_LSCK_LOW;
	UDELAY(1);
    SET_LSDA_LOW;//A0=0
    UDELAY(1);
    SET_LSCK_HIGH;
    UDELAY(1);

    for (i = 0; i < 8; ++ i)
    {
        SET_LSCK_LOW;
        if (cmd & (1 << 7)) {
            SET_LSDA_HIGH;
        } else {
            SET_LSDA_LOW;
        }
        UDELAY(1);
        SET_LSCK_HIGH;
        UDELAY(1);
        cmd <<= 1;
    }

    SET_LSDA_HIGH;
    SET_LSCE_HIGH;
}

static __inline void send_data_cmd(unsigned char data)
{
    unsigned char i;

    SET_LSCE_HIGH;
    UDELAY(1);
    SET_LSCK_HIGH;
	UDELAY(1);
    SET_LSDA_HIGH;
    UDELAY(1);
	
    SET_LSCE_LOW;
    UDELAY(1);

	SET_LSCK_LOW;
	UDELAY(1);
    SET_LSDA_HIGH;//A0=1
    UDELAY(1);
    SET_LSCK_HIGH;
    UDELAY(1);

    for (i = 0; i < 8; ++ i)
    {
        SET_LSCK_LOW;
        if (data & (1 << 7)) {
            SET_LSDA_HIGH;
        } else {
            SET_LSDA_LOW;
        }
        UDELAY(1);
        SET_LSCK_HIGH;
        UDELAY(1);
        data <<= 1;
    }

    SET_LSDA_HIGH;
    SET_LSCE_HIGH;
}

static __inline void set_lcm_register(unsigned char regIndex,
                                      unsigned char regData)
{
    send_ctrl_cmd(regIndex);
    send_data_cmd(regData);
}

// IICuX add
// ***************************
#define GPIO_DISP_BL_EN_PIN GPIO_LCDBL_EN_PIN

static  __inline void backlight_ctrl(unsigned char cmd)
{
	if(cmd)
	{
	   mt_set_gpio_mode(GPIO_DISP_BL_EN_PIN, 0);
            mt_set_gpio_dir(GPIO_DISP_BL_EN_PIN, GPIO_DIR_OUT);
            //mt_set_gpio_out(GPIO_DISP_BL_EN_PIN, GPIO_OUT_ZERO);
	   mt_set_gpio_out(GPIO_DISP_BL_EN_PIN, GPIO_OUT_ONE);
	   UDELAY(100);
	}
	else
	{
	   mt_set_gpio_mode(GPIO_DISP_BL_EN_PIN, 0);
            mt_set_gpio_dir(GPIO_DISP_BL_EN_PIN, GPIO_DIR_OUT);
            mt_set_gpio_out(GPIO_DISP_BL_EN_PIN, GPIO_OUT_ZERO);
	   UDELAY(100);
	}   
return;     
}
//**********************************

static void init_lcm_registers(void)
{
/*
customer:AUX[V900]
module id:90-24045-3065A[dijing]
LCM IC:HX8369-A
glass:HSD3.97
resolution:wvga(480*800)
*/
	send_ctrl_cmd(0xB9);  // SET password
	send_data_cmd(0xFF);  
	send_data_cmd(0x83);  
	send_data_cmd(0x69);

	send_ctrl_cmd(0xB1);  //Set Power
	send_data_cmd(0x85);
	send_data_cmd(0x00);
	send_data_cmd(0x34);
	send_data_cmd(0x0A);
	send_data_cmd(0x00);
	send_data_cmd(0x0F);
	send_data_cmd(0x0F);
	send_data_cmd(0x2A);
	send_data_cmd(0x32);
	send_data_cmd(0x3F);
	send_data_cmd(0x3F);
	send_data_cmd(0x01);
	send_data_cmd(0x23);
	send_data_cmd(0x01);
	send_data_cmd(0xE6);
	send_data_cmd(0xE6);
	send_data_cmd(0xE6);
	send_data_cmd(0xE6);
	send_data_cmd(0xE6);

	send_ctrl_cmd(0xB2);  // SET Display  480x800
	send_data_cmd(0x00);  
	send_data_cmd(0x20);  
	send_data_cmd(0x03);  
	send_data_cmd(0x03);  
	send_data_cmd(0x70);  
	send_data_cmd(0x00);  
	send_data_cmd(0xFF);  
	send_data_cmd(0x06);  
	send_data_cmd(0x00);  
	send_data_cmd(0x00);  
	send_data_cmd(0x00);  
	send_data_cmd(0x03);  
	send_data_cmd(0x03);  
	send_data_cmd(0x00);  
	send_data_cmd(0x01);  

	send_ctrl_cmd(0xB4);  // SET Display  column inversion
	send_data_cmd(0x00);  
	send_data_cmd(0x18);  
	send_data_cmd(0x80);  
	send_data_cmd(0x06);  
	send_data_cmd(0x02);  

	send_ctrl_cmd(0xB6);  // SET VCOM
	send_data_cmd(0x3A);  
	send_data_cmd(0x3A);  

	send_ctrl_cmd(0xD5);
	send_data_cmd(0x00);
	send_data_cmd(0x02);
	send_data_cmd(0x03);
	send_data_cmd(0x00);
	send_data_cmd(0x01);
	send_data_cmd(0x03);
	send_data_cmd(0x28);
	send_data_cmd(0x70);
	send_data_cmd(0x01);
	send_data_cmd(0x03);
	send_data_cmd(0x00);
	send_data_cmd(0x00);
	send_data_cmd(0x40);
	send_data_cmd(0x06);
	send_data_cmd(0x51);
	send_data_cmd(0x07);
	send_data_cmd(0x00);
	send_data_cmd(0x00);
	send_data_cmd(0x41);
	send_data_cmd(0x06);
	send_data_cmd(0x50);
	send_data_cmd(0x07);
	send_data_cmd(0x07);
	send_data_cmd(0x0F);
	send_data_cmd(0x04);
	send_data_cmd(0x00);

	send_ctrl_cmd(0xE0); // Set Gamma
	send_data_cmd(0x00);  
	send_data_cmd(0x13);  
	send_data_cmd(0x19);  
	send_data_cmd(0x38);  
	send_data_cmd(0x3D);  
	send_data_cmd(0x3F);  
	send_data_cmd(0x28);  
	send_data_cmd(0x46);  
	send_data_cmd(0x07);  
	send_data_cmd(0x0D);  
	send_data_cmd(0x0E);  
	send_data_cmd(0x12);  
	send_data_cmd(0x15);  
	send_data_cmd(0x12);  
	send_data_cmd(0x14);  
	send_data_cmd(0x0F);  
	send_data_cmd(0x17);  
	send_data_cmd(0x00);  
	send_data_cmd(0x13);  
	send_data_cmd(0x19);  
	send_data_cmd(0x38);  
	send_data_cmd(0x3D);  
	send_data_cmd(0x3F);  
	send_data_cmd(0x28);  
	send_data_cmd(0x46);  
	send_data_cmd(0x07);  
	send_data_cmd(0x0D);  
	send_data_cmd(0x0E);  
	send_data_cmd(0x12);  
	send_data_cmd(0x15);  
	send_data_cmd(0x12);  
	send_data_cmd(0x14);  
	send_data_cmd(0x0F);  
	send_data_cmd(0x17);  

	send_ctrl_cmd(0xC1); // Set DGC
	send_data_cmd(0x01);  
	send_data_cmd(0x04);  
	send_data_cmd(0x13);  
	send_data_cmd(0x1A);  
	send_data_cmd(0x20);  
	send_data_cmd(0x27);  
	send_data_cmd(0x2C);  
	send_data_cmd(0x32);  
	send_data_cmd(0x36);  
	send_data_cmd(0x3F);  
	send_data_cmd(0x47);  
	send_data_cmd(0x50);  
	send_data_cmd(0x59);  
	send_data_cmd(0x60);  
	send_data_cmd(0x68);  
	send_data_cmd(0x71);  
	send_data_cmd(0x7B);  
	send_data_cmd(0x82);  
	send_data_cmd(0x89);  
	send_data_cmd(0x91);  
	send_data_cmd(0x98);  
	send_data_cmd(0xA0);  
	send_data_cmd(0xA8);  
	send_data_cmd(0xB0);  
	send_data_cmd(0xB8);  
	send_data_cmd(0xC1);  
	send_data_cmd(0xC9);  
	send_data_cmd(0xD0);  
	send_data_cmd(0xD7);  
	send_data_cmd(0xE0);  
	send_data_cmd(0xE7);  
	send_data_cmd(0xEF);  
	send_data_cmd(0xF7);  
	send_data_cmd(0xFE);  
	send_data_cmd(0xCF);  
	send_data_cmd(0x52);  
	send_data_cmd(0x34);  
	send_data_cmd(0xF8);  
	send_data_cmd(0x51);  
	send_data_cmd(0xF5);  
	send_data_cmd(0x9D);  
	send_data_cmd(0x75);  
	send_data_cmd(0x00);  
	send_data_cmd(0x04);  
	send_data_cmd(0x13);  
	send_data_cmd(0x1A);  
	send_data_cmd(0x20);  
	send_data_cmd(0x27);  
	send_data_cmd(0x2C);  
	send_data_cmd(0x32);  
	send_data_cmd(0x36);  
	send_data_cmd(0x3F);  
	send_data_cmd(0x47);  
	send_data_cmd(0x50);  
	send_data_cmd(0x59);  
	send_data_cmd(0x60);  
	send_data_cmd(0x68);  
	send_data_cmd(0x71);  
	send_data_cmd(0x7B);  
	send_data_cmd(0x82);  
	send_data_cmd(0x89);  
	send_data_cmd(0x91);  
	send_data_cmd(0x98);  
	send_data_cmd(0xA0);  
	send_data_cmd(0xA8);  
	send_data_cmd(0xB0);  
	send_data_cmd(0xB8);  
	send_data_cmd(0xC1); 
	send_data_cmd(0xC9);  
	send_data_cmd(0xD0);  
	send_data_cmd(0xD7);  
	send_data_cmd(0xE0);  
	send_data_cmd(0xE7);  
	send_data_cmd(0xEF);  
	send_data_cmd(0xF7);  
	send_data_cmd(0xFE);  
	send_data_cmd(0xCF);  
	send_data_cmd(0x52);  
	send_data_cmd(0x34);  
	send_data_cmd(0xF8);  
	send_data_cmd(0x51);  
	send_data_cmd(0xF5);  
	send_data_cmd(0x9D);  
	send_data_cmd(0x75);  
	send_data_cmd(0x00);  
	send_data_cmd(0x04);  
	send_data_cmd(0x13);  
	send_data_cmd(0x1A);  
	send_data_cmd(0x20);  
	send_data_cmd(0x27);  
	send_data_cmd(0x2C);  
	send_data_cmd(0x32);  
	send_data_cmd(0x36);  
	send_data_cmd(0x3F);  
	send_data_cmd(0x47);  
	send_data_cmd(0x50);  
	send_data_cmd(0x59);  
	send_data_cmd(0x60);  
	send_data_cmd(0x68);  
	send_data_cmd(0x71);  
	send_data_cmd(0x7B);  
	send_data_cmd(0x82); 
	send_data_cmd(0x89);  
	send_data_cmd(0x91);  
	send_data_cmd(0x98);  
	send_data_cmd(0xA0);  
	send_data_cmd(0xA8);  
	send_data_cmd(0xB0);  
	send_data_cmd(0xB8);  
	send_data_cmd(0xC1);  
	send_data_cmd(0xC9);  
	send_data_cmd(0xD0);  
	send_data_cmd(0xD7);  
	send_data_cmd(0xE0);  
	send_data_cmd(0xE7);  
	send_data_cmd(0xEF);  
	send_data_cmd(0xF7);  
	send_data_cmd(0xFE);  
	send_data_cmd(0xCF);  
	send_data_cmd(0x52);  
	send_data_cmd(0x34);  
	send_data_cmd(0xF8);  
	send_data_cmd(0x51);  
	send_data_cmd(0xF5);  
	send_data_cmd(0x9D);  
	send_data_cmd(0x75);  
	send_data_cmd(0x00);

	send_ctrl_cmd(0x3A);  // set Interface Pixel Format
	send_data_cmd(0x07);   // 0x07=24 Bit/Pixel; 0x06=18 Bit/Pixel; 0x05=16 Bit/Pixel

	send_ctrl_cmd(0x51);//write display brightness
	send_data_cmd(0xff);//set brightness 0x00-0xff
	MDELAY(50);

	send_ctrl_cmd(0x53);//write ctrl display
	send_data_cmd(0x24);
	MDELAY(50);

	send_ctrl_cmd(0x55);
	send_data_cmd(0x02);//still picture
	MDELAY(50);

	send_ctrl_cmd(0x5e);//write CABC minumum brightness
	send_data_cmd(0x70);//
	MDELAY(50);

	send_ctrl_cmd(0x11); 	
	MDELAY(200);

         send_ctrl_cmd(0x29);
	MDELAY(20);
}


static void config_gpio(void)
{
    const unsigned int USED_GPIOS[] = 
    {
        LSCE_GPIO_PIN,
        LSCK_GPIO_PIN,
        LSDA_GPIO_PIN
    };

    unsigned int i;

    lcm_util.set_gpio_mode(LSA0_GPIO_PIN, GPIO_DISP_LSA0_PIN_M_GPIO);
    lcm_util.set_gpio_mode(LSCE_GPIO_PIN, GPIO_DISP_LSCE_PIN_M_GPIO);
    lcm_util.set_gpio_mode(LSCK_GPIO_PIN, GPIO_DISP_LSCK_PIN_M_GPIO);
    lcm_util.set_gpio_mode(LSDA_GPIO_PIN, GPIO_DISP_LSDA_PIN_M_GPIO);

    for (i = 0; i < ARY_SIZE(USED_GPIOS); ++ i)
    {
        lcm_util.set_gpio_dir(USED_GPIOS[i], 1);               // GPIO out
        lcm_util.set_gpio_pull_enable(USED_GPIOS[i], 0);
    }

    // Swithc LSA0 pin to GPIO mode to avoid data contention,
    // since A0 is connected to LCM's SPI SDO pin
    //
    lcm_util.set_gpio_dir(LSA0_GPIO_PIN, 0);                   // GPIO in
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));

    params->type   = LCM_TYPE_DPI;
    params->ctrl   = LCM_CTRL_GPIO;
    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;
    
    params->dpi.mipi_pll_clk_ref  = 0;
    params->dpi.mipi_pll_clk_div1 = 40;
    params->dpi.mipi_pll_clk_div2 = 10;
    params->dpi.dpi_clk_div       = 2;
    params->dpi.dpi_clk_duty      = 1;

    params->dpi.clk_pol           = LCM_POLARITY_FALLING;
    params->dpi.de_pol            = LCM_POLARITY_RISING;
    params->dpi.vsync_pol         = LCM_POLARITY_FALLING;
    params->dpi.hsync_pol         = LCM_POLARITY_FALLING;

    params->dpi.hsync_pulse_width = 5;
    params->dpi.hsync_back_porch  = 5;
    params->dpi.hsync_front_porch = 5;
    params->dpi.vsync_pulse_width = 5; //4
    params->dpi.vsync_back_porch  = 5;
    params->dpi.vsync_front_porch = 8;
   
    params->dpi.format            = LCM_DPI_FORMAT_RGB888;
    params->dpi.rgb_order         = LCM_COLOR_ORDER_RGB;
    params->dpi.is_serial_output  = 0;

    params->dpi.intermediat_buffer_num = 2;

    params->dpi.io_driving_current = LCM_DRIVING_CURRENT_6575_4MA;
}


static void lcm_init(void)
{
    config_gpio();

    SET_RESET_PIN(1);
    MDELAY(50);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(150);

    init_lcm_registers();
    //backlight
    backlight_ctrl(1);
}

// IICuX add
//****************************
static void lcm_update(unsigned int x, unsigned int y,
		unsigned int width, unsigned int height)
{
	unsigned short x0, y0, x1, y1;
	unsigned short h_X_start,l_X_start,h_X_end,l_X_end,h_Y_start,l_Y_start,h_Y_end,l_Y_end;

	x0 = (unsigned short)x;
	y0 = (unsigned short)y;
	x1 = (unsigned short)x+width-1;
	y1 = (unsigned short)y+height-1;

	h_X_start=((x0&0xFF00)>>8);
	l_X_start=(x0&0x00FF);
	h_X_end=((x1&0xFF00)>>8);
	l_X_end=(x1&0x00FF);

	h_Y_start=((y0&0xFF00)>>8);
	l_Y_start=(y0&0x00FF);
	h_Y_end=((y1&0xFF00)>>8);
	l_Y_end=(y1&0x00FF);

	send_ctrl_cmd(0x2A);
	send_data_cmd(h_X_start); 
	send_data_cmd(l_X_start); 
	send_data_cmd(h_X_end); 
	send_data_cmd(l_X_end); 

	send_ctrl_cmd(0x2B);
	send_data_cmd(h_Y_start); 
	send_data_cmd(l_Y_start); 
	send_data_cmd(h_Y_end); 
	send_data_cmd(l_Y_end); 

	send_ctrl_cmd(0x29); 

	send_ctrl_cmd(0x2C);
}

static void lcm_suspend(void)
{
    //backlight
    printk("Backlight OFF\n");
    backlight_ctrl(0);
    
    send_ctrl_cmd(0x28);
    MDELAY(100);
    send_ctrl_cmd(0x10);
    MDELAY(100);
}


static void lcm_resume(void)
{
    printk("Backlight ON\n");
    backlight_ctrl(1);

    send_ctrl_cmd(0x11);
    MDELAY(100);
    send_ctrl_cmd(0x29);
    MDELAY(100);
}

void lcm_setbacklight(unsigned int level)
{
    backlight_ctrl(1);
}

static unsigned int lcm_compare_id(void)
{

	
    return 1;

}
//****************************

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hx8369a_lcm_drv = 
{
    .name			= "hx8369a",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
         // Add by IICuX
	.update         = lcm_update,
         //.set_backlight  = lcm_setbacklight,
         //*************
};
