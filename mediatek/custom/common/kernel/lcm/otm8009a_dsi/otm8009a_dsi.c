#include <linux/string.h>
#include "lcm_drv.h"
#if defined(BUILD_UBOOT)
#else
#include <linux/kernel.h>
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH                              (480)
#define FRAME_HEIGHT                             (800)

#define REGFLAG_DELAY               0XFE
#define REGFLAG_END_OF_TABLE        0xFFF   // END OF REGISTERS MARKER

#define LCM_ID                      0x8009
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)                                                                    (lcm_util.set_reset_pin((v)))

#define UDELAY(n)                                                                                         (lcm_util.udelay(n))
#define MDELAY(n)                                                                                         (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)                lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                                                                lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)                                        lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg                                                                                        lcm_util.dsi_read_reg()
#define read_reg_v2(cmd,buffer,buffer_size)                 lcm_util.dsi_dcs_read_lcm_reg_v2(cmd,buffer,buffer_size)      
      

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {
       
        /*
        Note :

        Data ID will depends on the following rule.
       
                count of parameters > 1        => Data ID = 0x39
                count of parameters = 1        => Data ID = 0x15
                count of parameters = 0        => Data ID = 0x05

        Structure Format :

        {DCS command, count of parameters, {parameter list}}
        {REGFLAG_DELAY, milliseconds of time, {}},

        ...

        Setting ending by predefined flag
       
        {REGFLAG_END_OF_TABLE, 0x00, {}}
        */

{0x00,1,{0x00}},{0xff,3,{0x80,0x09,0x01}},

{0x00,1,{0x80}},{0xff,2,{0x80,0x09}},

{0x00,1,{0x03}},{0xff,1,{0x01}},

{0x00,1,{0xb4}},{0xc0,1,{0x10}},

{0x00,1,{0x89}},{0xc4,1,{0x08}},

{0x00,1,{0xa3}},{0xc0,1,{0x00}},

{0x00,1,{0x82}},{0xc5,1,{0xa3}},

{0x00,1,{0x90}},{0xc5,2,{0xd6,0x87}},

{0x00,1,{0x00}},{0xd8,2,{0x74,0x72}},

{0x00,1,{0x00}},{0xd9,1,{0x50}},

{0x00,1,{0x00}},{0xe1,16,{0x09,0x0c,0x12,0x0e,0x08,0x19,0x0c,0x0b,0x01,0x05,0x03,0x07,0x0e,0x26,0x23,0x1b}},

{0x00,1,{0x00}},{0xe2,16,{0x09,0x0c,0x12,0x0e,0x08,0x19,0x0c,0x0b,0x01,0x05,0x03,0x07,0x0e,0x26,0x23,0x1b}},

{0x00,1,{0x81}},{0xc1,1,{0x66}},

{0x00,1,{0xa1}},{0xc1,1,{0x88}},

{0x00,1,{0x81}},{0xc4,1,{0x83}},

{0x00,1,{0x92}},{0xc5,1,{0x01}},

{0x00,1,{0xb1}},{0xc5,1,{0xa9}},

{0x00,1,{0x80}},{0xce,12,{0x85,0x03,0x00,0x84,0x03,0x00,0x83,0x03,0x00,0x82,0x03,0x00}},

{0x00,1,{0xa0}},{0xce,14,{0x38,0x02,0x03,0x21,0x00,0x00,0x00,0x38,0x01,0x03,0x22,0x00,0x00,0x00}},

{0x00,1,{0xb0}},{0xce,14,{0x38,0x00,0x03,0x23,0x00,0x00,0x00,0x30,0x00,0x03,0x24,0x00,0x00,0x00}},

{0x00,1,{0xc0}},{0xce,14,{0x30,0x01,0x03,0x25,0x00,0x00,0x00,0x30,0x02,0x03,0x26,0x00,0x00,0x00}},

{0x00,1,{0xd0}},{0xce,14,{0x30,0x03,0x03,0x27,0x00,0x00,0x00,0x30,0x04,0x03,0x28,0x00,0x00,0x00}},

{0x00,1,{0xc0}},{0xcf,10,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xd0}},{0xcf,1,{0x00}},

{0x00,1,{0xc0}},{0xcb,15,{0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xd0}},{0xcb,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04}},

{0x00,1,{0xe0}},{0xcb,10,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0x80}},{0xcc,10,{0x00,0x00,0x00,0x00,0x0c,0x0a,0x10,0x0e,0x03,0x04}},

{0x00,1,{0x90}},{0xcc,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0b}},

{0x00,1,{0xa0}},{0xcc,15,{0x09,0x0f,0x0d,0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xb0}},{0xcc,10,{0x00,0x00,0x00,0x00,0x0d,0x0f,0x09,0x0b,0x02,0x01}},

{0x00,1,{0xc0}},{0xcc,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0e}},

{0x00,1,{0xd0}},{0xcc,15,{0x10,0x0a,0x0c,0x04,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0x00}},{0xff,3,{0xff,0xff,0xff}},

{0x3a,1,{0x77}},
#ifdef DISPLAY_DIRECTION_0_MODE
DCS_SHORT_1P(0x36,0x00);// Display Direction 0
DCS_SHORT_1P(0x35,0x00);// TE( Fmark ) Signal On
DCS_LONG_2P(0x44,0x01,0x22);// TE( Fmark ) Signal Output Position
#endif

#ifdef DISPLAY_DIRECTION_180_MODE
DCS_SHORT_1P(0x36,0xD0);// Display Direction 180
DCS_SHORT_1P(0x35,0x00);// TE( Fmark ) Signal On
DCS_LONG_2P(0x44,0x01,0xFF);// TE( Fmark ) Signal Output Position
#endif

#ifdef LCD_BACKLIGHT_CONTROL_MODE
DCS_SHORT_1P(0x51,0xFF);// Backlight Level Control
DCS_SHORT_1P(0x53,0x2C);// Backlight On
DCS_SHORT_1P(0x55,0x00);// CABC Function Off
#endif


{0x11,0,{}},

{REGFLAG_DELAY, 50, {}},

{0x29,0,{}},

{REGFLAG_DELAY, 200, {}},

{0x2c,0,{}},
{REGFLAG_DELAY, 120, {}},

        // Note
        // Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.
        // Setting ending by predefined flag
       
        {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_set_window[] = {
        {0x2A,        4,        {0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
        {0x2B,        4,        {0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
        {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
        {0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
        {0x29, 1, {0x00}},
        {REGFLAG_DELAY, 20, {}},
        {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
        // Display off sequence
        {0x28, 1, {0x00}},

    // Sleep Mode On
        {0x10, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

        {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_backlight_level_setting[] = {
        {0x51, 1, {0xFF}},
        {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_backlight_mode_setting[] = {
        {0x55, 1, {0x1}},
        {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void init_lcm_registers(void)
{
        unsigned int data_array[16];

//{0x00,1,{0x00}},{0xff,3,{0x80,0x09,0x01}},
    data_array[0]=0x00001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00043902;
        data_array[1]=0x010980ff;
    dsi_set_cmdq(data_array, 2, 1);
   
//{0x00,1,{0x80}},{0xff,2,{0x80,0x09}},
    data_array[0]=0x80001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00033902;
        data_array[1]=0x000980ff;
    dsi_set_cmdq(data_array, 2, 1);
   
//{0x00,1,{0x03}},{0xff,1,{0x01}},
    data_array[0]=0x03001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x01ff1500;
    dsi_set_cmdq(data_array, 1, 1);
   
//{0x00,1,{0xb4}},{0xc0,1,{0x10}},
    data_array[0]=0xb4001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x20c01500;
    dsi_set_cmdq(data_array, 1, 1);
   
//{0x00,1,{0x89}},{0xc4,1,{0x08}},
    data_array[0]=0x89001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x08c41500;
    dsi_set_cmdq(data_array, 1, 1);
   
//{0x00,1,{0xa3}},{0xc0,1,{0x00}},
    data_array[0]=0xa3001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00c01500;
    dsi_set_cmdq(data_array, 1, 1);
   
//{0x00,1,{0x82}},{0xc5,1,{0xa3}},
    data_array[0]=0x82001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0xa3c51500;
    dsi_set_cmdq(data_array, 1, 1);
   
//{0x00,1,{0x90}},{0xc5,2,{0xd6,0x87}},
    data_array[0]=0x90001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00033902;
        data_array[1]=0x0087d6c5;
    dsi_set_cmdq(data_array, 2, 1);
   
//{0x00,1,{0x00}},{0xd8,2,{0x74,0x72}},
    data_array[0]=0x00001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00033902;
        data_array[1]=0x007274d8;
    dsi_set_cmdq(data_array, 2, 1);
   
//{0x00,1,{0x00}},{0xd9,1,{0x50}},
    data_array[0]=0x00001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x50d91500;
    dsi_set_cmdq(data_array, 1, 1);
   
//{0x00,1,{0x00}},{0xe1,16,{0x09,0x0c,0x12,0x0e,0x08,0x19,0x0c,0x0b,0x01,0x05,0x03,0x07,0x0e,0x26,0x23,0x1b}},
    data_array[0]=0x00001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00113902;
    data_array[1]=0x120c09e1;
    data_array[2]=0x0c19080e;
    data_array[3]=0x0305010b;
    data_array[4]=0x23260e07;
    data_array[5]=0x0000001b;
    dsi_set_cmdq(data_array, 6, 1);


//{0x00,1,{0x00}},{0xe2,16,{0x09,0x0c,0x12,0x0e,0x08,0x19,0x0c,0x0b,0x01,0x05,0x03,0x07,0x0e,0x26,0x23,0x1b}},
    data_array[0]=0x00001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00113902;
    data_array[1]=0x120c09e2;
    data_array[2]=0x0c19080e;
    data_array[3]=0x0305010b;
    data_array[4]=0x23260e07;
    data_array[5]=0x0000001b;
    dsi_set_cmdq(data_array, 6, 1);


//{0x00,1,{0x81}},{0xc1,1,{0x66}},
    data_array[0]=0x81001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x66c11500;
    dsi_set_cmdq(data_array, 1, 1);
   
//{0x00,1,{0xa1}},{0xc1,1,{0x88}},
    data_array[0]=0xa1001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x88c11500;
    dsi_set_cmdq(data_array, 1, 1);
   
//{0x00,1,{0x81}},{0xc4,1,{0x83}},
    data_array[0]=0x81001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x83c41500;
    dsi_set_cmdq(data_array, 1, 1);
   
//{0x00,1,{0x92}},{0xc5,1,{0x01}},
    data_array[0]=0x92001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x01c51500;
    dsi_set_cmdq(data_array, 1, 1);
   
//{0x00,1,{0xb1}},{0xc5,1,{0xa9}},
    data_array[0]=0xb1001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0xa9c51500;
    dsi_set_cmdq(data_array, 1, 1);
   
//{0x00,1,{0x80}},{0xce,12,{0x85,0x03,0x00,0x84,0x03,0x00,0x83,0x03,0x00,0x82,0x03,0x00}},
    data_array[0]=0x80001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x000d3902;
    data_array[1]=0x000385ce;
    data_array[2]=0x83000384;
    data_array[3]=0x03820003;
    data_array[4]=0x00000000;
    dsi_set_cmdq(data_array, 5, 1);

//{0x00,1,{0xa0}},{0xce,14,{0x38,0x02,0x03,0x21,0x00,0x00,0x00,0x38,0x01,0x03,0x22,0x00,0x00,0x00}},
    data_array[0]=0xa0001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x000d3902;
    data_array[1]=0x030238ce;
    data_array[2]=0x00000021;
    data_array[3]=0x22030138;
    data_array[4]=0x00000000;
    dsi_set_cmdq(data_array, 5, 1);
   
   
//{0x00,1,{0xb0}},{0xce,14,{0x38,0x00,0x03,0x23,0x00,0x00,0x00,0x30,0x00,0x03,0x24,0x00,0x00,0x00}},
    data_array[0]=0xb0001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x000f3902;
    data_array[1]=0x030038ce;
    data_array[2]=0x00000023;
    data_array[3]=0x24030030;
    data_array[4]=0x23000000;
    dsi_set_cmdq(data_array, 5, 1);
   
//{0x00,1,{0xc0}},{0xce,14,{0x30,0x01,0x03,0x25,0x00,0x00,0x00,0x30,0x02,0x03,0x26,0x00,0x00,0x00}},
    data_array[0]= 0xc0001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x000f3902;
    data_array[1]=0x030130ce;
    data_array[2]=0x00000025;
    data_array[3]=0x26030230;
    data_array[4]=0x23000000;
    dsi_set_cmdq(data_array, 5, 1);
   
//{0x00,1,{0xd0}},{0xce,14,{0x30,0x03,0x03,0x27,0x00,0x00,0x00,0x30,0x04,0x03,0x28,0x00,0x00,0x00}},
    data_array[0]= 0xd0001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x000f3902;
    data_array[1]=0x030330ce;
    data_array[2]=0x00000027;
    data_array[3]=0x28030430;
    data_array[4]=0x23000000;
    dsi_set_cmdq(data_array, 5, 1);
   
//{0x00,1,{0xc0}},{0xcf,10,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
    data_array[0]= 0xc0001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x000b3902;
    data_array[1]=0x000000cf;
    data_array[2]=0x00000000;
    data_array[3]=0x28000000;
    dsi_set_cmdq(data_array, 4, 1);


//{0x00,1,{0xd0}},{0xcf,1,{0x00}},
    data_array[0]=0xd0001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00cf1500;
    dsi_set_cmdq(data_array, 1, 1);
   
//{0x00,1,{0xc0}},{0xcb,15,{0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00}},
    data_array[0]=0xc0001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00103902;
    data_array[1]=0x000000cb;
    data_array[2]=0x04040400;
    data_array[3]=0x00040404;
    data_array[4]=0x00000000;
    dsi_set_cmdq(data_array, 5, 1);

//{0x00,1,{0xd0}},{0xcb,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04}},
    data_array[0]=0xd0001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00103902;
    data_array[1]=0x000000cb;
    data_array[2]=0x00000000;
    data_array[3]=0x04040000;
    data_array[4]=0x04040404;
    dsi_set_cmdq(data_array, 5, 1);
//{0x00,1,{0xe0}},{0xcb,10,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
    data_array[0]=0xe0001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x000b3902;
    data_array[1]=0x000000cb;
    data_array[2]=0x00000000;
    data_array[3]=0x04000000;
    dsi_set_cmdq(data_array, 4, 1);
  
//{0x00,1,{0x80}},{0xcc,10,{0x00,0x00,0x00,0x00,0x0c,0x0a,0x10,0x0e,0x03,0x04}},
    data_array[0]=0x80001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x000b3902;
    data_array[1]=0x000000cc;
    data_array[2]=0x100a0c00;
    data_array[3]=0x0404030e;
    dsi_set_cmdq(data_array, 4, 1);
  
//{0x00,1,{0x90}},{0xcc,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0b}},
    data_array[0]=0x90001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00103902;
    data_array[1]=0x000000cc;
    data_array[2]=0x00000000;
    data_array[3]=0x00000000;
    data_array[4]=0x0b000000;
    dsi_set_cmdq(data_array, 5, 1);

//{0x00,1,{0xa0}},{0xcc,15,{0x09,0x0f,0x0d,0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
    data_array[0]=0xa0001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00103902;
    data_array[1]=0x0d0f09cc;
    data_array[2]= 0x00000201;
    data_array[3]=0x00000000;
    data_array[4]=0x00000000;
    dsi_set_cmdq(data_array, 5, 1);

//{0x00,1,{0xb0}},{0xcc,10,{0x00,0x00,0x00,0x00,0x0d,0x0f,0x09,0x0b,0x02,0x01}},
    data_array[0]=0xb0001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x000b3902;
    data_array[1]=0x000000cc;
    data_array[2]=0x090f0d00;
    data_array[3]=0x0001020b;
    dsi_set_cmdq(data_array, 4, 1);
  
//{0x00,1,{0xc0}},{0xcc,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0e}},
    data_array[0]=0xc0001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00103902;
    data_array[1]=0x000000cc;
    data_array[2]= 0x00000000;
    data_array[3]=0x00000000;
    data_array[4]=0x00000000;
    dsi_set_cmdq(data_array, 5, 1);
  
//{0x00,1,{0xd0}},{0xcc,15,{0x10,0x0a,0x0c,0x04,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
    data_array[0]=0xd0001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00103902;
    data_array[1]=0x0c0a10cc;
    data_array[2]= 0x00000304;
    data_array[3]=0x00000000;
    data_array[4]=0x00000000;
    dsi_set_cmdq(data_array, 5, 1);
  
//{0x00,1,{0x00}},{0xff,3,{0xff,0xff,0xff}},
    data_array[0]=0x00001500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00043902;
    data_array[1]=0xffffffff;
    dsi_set_cmdq(data_array, 2, 1);

//{0x3a,1,{0x77}},
    data_array[0]=0x773a1500;
    dsi_set_cmdq(data_array, 1, 1);

#ifdef DISPLAY_DIRECTION_0_MODE
//DCS_SHORT_1P(0x36,0x00);// Display Direction 0
//DCS_SHORT_1P(0x35,0x00);// TE( Fmark ) Signal On
//DCS_LONG_2P(0x44,0x01,0x22);// TE( Fmark ) Signal Output Position
    data_array[0]=0x00361500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00351500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00033902;
    data_array[0]=0x00220144;
    dsi_set_cmdq(data_array, 2, 1);
#endif

    data_array[0]=0x00361500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00351500;
    dsi_set_cmdq(data_array, 1, 1);


#ifdef DISPLAY_DIRECTION_180_MODE
//DCS_SHORT_1P(0x36,0xD0);// Display Direction 180
//DCS_SHORT_1P(0x35,0x00);// TE( Fmark ) Signal On
//DCS_LONG_2P(0x44,0x01,0xFF);// TE( Fmark ) Signal Output Position
    data_array[0]=0xd0361500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00351500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00033902;
    data_array[0]=0x00ff0144;
    dsi_set_cmdq(data_array, 2, 1);
#endif

#ifdef LCD_BACKLIGHT_CONTROL_MODE
//DCS_SHORT_1P(0x51,0xFF);// Backlight Level Control
//DCS_SHORT_1P(0x53,0x2C);// Backlight On
//DCS_SHORT_1P(0x55,0x00);// CABC Function Off
    data_array[0]=0xff511500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x2c531500;
    dsi_set_cmdq(data_array, 1, 1);
    data_array[0]=0x00551500;
    dsi_set_cmdq(data_array, 1, 1);
#endif


}
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
        unsigned int i;

    for(i = 0; i < count; i++) {
               
        unsigned cmd;
        cmd = table[i].cmd;
               
        switch (cmd) {
                       
            case REGFLAG_DELAY :
                 MDELAY(table[i].count);
                 break;
                               
            case REGFLAG_END_OF_TABLE :
                 break;
                               
            default:
                 dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
        }
    }
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
       
                params->type   = LCM_TYPE_DSI;

                params->width  = FRAME_WIDTH;
                params->height = FRAME_HEIGHT;

                // enable tearing-free
                params->dbi.te_mode                                 = LCM_DBI_TE_MODE_VSYNC_ONLY;
                params->dbi.te_edge_polarity                = LCM_POLARITY_RISING;
                params->dsi.mode                                           = CMD_MODE;

                // DSI
                /* Command mode setting */
                params->dsi.LANE_NUM                                = LCM_TWO_LANE;
                //The following defined the fomat for data coming from LCD engine.
                params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
                params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
                params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
                params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

                // Highly depends on LCD driver capability.
                params->dsi.packet_size                            = 256;

                // Video mode setting               
                params->dsi.PS                      = LCM_PACKED_PS_24BIT_RGB888;

                params->dsi.word_count                  = 480*3;       
                params->dsi.vertical_sync_active        = 2;
                params->dsi.vertical_backporch                = 2;
                params->dsi.vertical_frontporch                = 2;
                params->dsi.vertical_active_line        = 800;
       
                params->dsi.line_byte                                = 2180;                // 2256 = 752*3
                params->dsi.horizontal_sync_active_byte  = 26;
                params->dsi.horizontal_backporch_byte    = 206;
                params->dsi.horizontal_frontporch_byte   = 206;       
                params->dsi.rgb_byte                                         = (480*3+6);       
       
                params->dsi.horizontal_sync_active_word_count = 20;       
                params->dsi.horizontal_backporch_word_count   = 200;
                params->dsi.horizontal_frontporch_word_count  = 200;

                // Bit rate calculation
                params->dsi.pll_div1   = 38;                // fref=26MHz, fvco=fref*(div1+1)        (div1=0~63, fvco=500MHZ~1GHz)
                params->dsi.pll_div2   = 1;                        // div2=0~15: fout=fvo/(2*div2)
               
      
}


static void lcm_init(void)
{
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(10 );
    SET_RESET_PIN(1);
    MDELAY(10 );

        //push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
    init_lcm_registers();
}


static void lcm_suspend(void)
{
        unsigned int data_array[16];
       
        data_array[0]=0x00280500;
        dsi_set_cmdq(&data_array, 1, 1);
        //MDELAY(50);
       
        data_array[0]=0x00100500;
        dsi_set_cmdq(&data_array, 1, 1);       
        MDELAY(150);
        //push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_resume(void)
{
        unsigned int data_array[16];

        data_array[0]=0x00110500;
        dsi_set_cmdq(&data_array, 1, 1);
        MDELAY(150);
       
        data_array[0]=0x00290500;
        dsi_set_cmdq(&data_array, 1, 1);               
        //push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
        unsigned int x0 = x;
        unsigned int y0 = y;
        unsigned int x1 = x0 + width - 1;
        unsigned int y1 = y0 + height - 1;

        unsigned char x0_MSB = ((x0>>8)&0xFF);
        unsigned char x0_LSB = (x0&0xFF);
        unsigned char x1_MSB = ((x1>>8)&0xFF);
        unsigned char x1_LSB = (x1&0xFF);
        unsigned char y0_MSB = ((y0>>8)&0xFF);
        unsigned char y0_LSB = (y0&0xFF);
        unsigned char y1_MSB = ((y1>>8)&0xFF);
        unsigned char y1_LSB = (y1&0xFF);

        unsigned int data_array[16];

        data_array[0]= 0x00053902;
        data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
        data_array[2]= (x1_LSB);
        data_array[3]= 0x00053902;
        data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
        data_array[5]= (y1_LSB);
        data_array[6]= 0x002c3909;
        dsi_set_cmdq(&data_array, 7, 0);
}


static void lcm_setbacklight(unsigned int level)
{
        unsigned int default_level = 0;
        unsigned int mapped_level = 0;

        //for LGE backlight IC mapping table
        if(level > 255)
                level = 255;

        if(level >0)
                mapped_level = default_level+(level)*(255-default_level)/(255);
        else
                mapped_level=0;

        // Refresh value of backlight level.
        lcm_backlight_level_setting[0].para_list[0] = mapped_level;

        push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}

//static void lcm_setbacklight_mode(unsigned int mode)
//{
//        lcm_backlight_mode_setting[0].para_list[0] = mode;
//        push_table(lcm_backlight_mode_setting, sizeof(lcm_backlight_mode_setting) / sizeof(struct LCM_setting_table), 1);
//}

static void lcm_setpwm(unsigned int divider)
{
        // TBD
}


static unsigned int lcm_getpwm(unsigned int divider)
{
        // ref freq = 15MHz, B0h setting 0x80, so 80.6% * freq is pwm_clk;
        // pwm_clk / 255 / 2(lcm_setpwm() 6th params) = pwm_duration = 23706
        unsigned int pwm_clk = 23706 / (1<<divider);       
        return pwm_clk;
}


static unsigned int otm8009a_cmp_id(void)
{
        unsigned int id=0;
        unsigned char buffer[2];
        unsigned int array[16];   

        SET_RESET_PIN(1);
        SET_RESET_PIN(0);
        MDELAY(25);
        SET_RESET_PIN(1);
        MDELAY(50);

        array[0]=0x00043902;
        array[1]=0x010980ff;
        array[2]=0x80001500;
        array[3]=0x00033902;
        array[4]=0x010980ff;
        dsi_set_cmdq(array, 5, 1);
        MDELAY(10);

        array[0] = 0x00023700;// set return byte number
        dsi_set_cmdq(array, 1, 1);

        array[0] = 0x02001500;
        dsi_set_cmdq(array, 1, 1);

        read_reg_v2(0xD2, &buffer, 2);

        id = buffer[0]<<8 |buffer[1];

        return (LCM_ID == id)?1:0;

}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER otm8009a_dsi_lcm_drv =
{
    .name                        = "otm8009a_dsi",
        .set_util_funcs = lcm_set_util_funcs,
        .get_params     = lcm_get_params,
        .init           = lcm_init,
        .suspend        = lcm_suspend,
        .resume         = lcm_resume,
        .update         = lcm_update,
        .set_backlight        = lcm_setbacklight,
//        .set_backlight_mode = lcm_setbacklight_mode,
        //.set_pwm        = lcm_setpwm,
        //.get_pwm        = lcm_getpwm  
        .compare_id    = otm8009a_cmp_id,
};
