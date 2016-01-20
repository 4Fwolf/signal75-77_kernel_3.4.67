#include <cust_leds.h>
#include <cust_leds_def.h>
#include <mach/mt_pwm.h>

#include <linux/kernel.h>
#include <mach/pmic_mt6329_hw_bank1.h> 
#include <mach/pmic_mt6329_sw_bank1.h> 
#include <mach/pmic_mt6329_hw.h>
#include <mach/pmic_mt6329_sw.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>

#include <mach/mt6577_gpio.h>
#include <linux/delay.h>
#include "cust_gpio_usage.h"

#include <linux/semaphore.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/version.h>

unsigned int brightness_mapping(unsigned int level)
{  
	if(level) {
		return level/32;
	}else {
		return level;
	}
}

#define LED_MAX_LEVEL 32
static int g_led_level = 0;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
    DECLARE_MUTEX(led_mutex);           
#else // (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36))        
    DEFINE_SEMAPHORE(led_mutex);
#endif
#if 0
  #define LED_DBG(fmt, arg...) \
	printk("[LED-LCM] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)
#else
  #define LED_DBG(fmt, arg...) do {} while (0)
#endif

unsigned int Cust_SetBacklight(int level, int div)
{
	int loop, led_level, pulse = 0;

	if (level < 0 || level > 255){
		LED_DBG("[LED]Error! LED level is out of range.");
		return -EFAULT;
	}
	if (down_interruptible(&led_mutex)){
		LED_DBG("LED is bussy.");
		return -EAGAIN;
	}
	mt_set_gpio_mode(GPIO_LCDBL_EN_PIN, GPIO_MODE_GPIO);
	mt_set_gpio_dir(GPIO_LCDBL_EN_PIN, GPIO_DIR_OUT);	
	if (level){
		/*
		 * Mapping soft level (0~255) to hard level (32~1)
		 * "1" is the top level for LED
		 */
		led_level = (255 - level) / 8 + 1;
		if (g_led_level != led_level){
			if (led_level > g_led_level)
				pulse = led_level - g_led_level;
			else
				pulse = LED_MAX_LEVEL - g_led_level + led_level ;

			//local_irq_disable();
			preempt_disable();
			for (loop = 0; loop < pulse; loop++){
				mt_set_gpio_out(GPIO_LCDBL_EN_PIN,0);
				udelay(1);	    
				mt_set_gpio_out(GPIO_LCDBL_EN_PIN,1);
				udelay(1);
			}
			preempt_enable();
			//local_irq_enable();
			g_led_level = led_level;
		}
	}
	else{
		LED_DBG("[LED]LED will shut down.");
		mt_set_gpio_out(GPIO_LCDBL_EN_PIN,0);
		msleep(5);
		g_led_level = 0;
	}
	up(&led_mutex);
    return 0;
}

static struct cust_mt65xx_led cust_led_list[MT65XX_LED_TYPE_TOTAL] = {
	{"red",               MT65XX_LED_MODE_PWM, PWM1,{0}},
	{"green",             MT65XX_LED_MODE_PWM, PWM2,{0}},
	{"blue",              MT65XX_LED_MODE_PWM, PWM3,{0}},
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1,{0}},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1,{0}},
	{"button-backlight",  MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_BUTTON,{0}},
	{"lcd-backlight",     MT65XX_LED_MODE_CUST_LCM, (int)Cust_SetBacklight,{0}},  // IICuX
	//{"lcd-backlight",     MT65XX_LED_MODE_PWM, PWM1,{0}}, //Clarc
	//{"lcd-backlight",     MT65XX_LED_MODE_PWM, PWM0,{1,CLK_DIV2,32,32}},
	//{"lcd-backlight",     MT65XX_LED_MODE_PWM, PWM1,{1,CLK_DIV8,5,5}},
	//{"lcd-backlight",     MT65XX_LED_MODE_PWM, PMW 1;2;3;4;5;6,{0}},
	//{"lcd-backlight",     MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_LCD_ISINK,{0}},
	//{"lcd-backlight",     MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_LCD_BOOST,{0}},
	//{"lcd-backlight",     MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_LCD,{0}},
         //{"torch",             MT65XX_LED_MODE_NONE, -1,{0}},
	//{"flash-light",       MT65XX_LED_MODE_GPIO, GPIO_CAMERA_FLASH_EN_PIN,{0}}, //kaka_12_0112 FlashLight control  for FTM use only
};


struct cust_mt65xx_led *get_cust_led_list(void)
{
   return cust_led_list;
}
