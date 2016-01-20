#include <cust_vibrator.h>
#include <linux/types.h>

static struct vibrator_hw cust_vibrator_hw = {
	.vib_timer = 50,
	.vib_timer_min = 10, 
#ifdef CUST_VIBR_VOL
//	.vib_vol = 0x2,//1.5V
//	.vib_vol = 0x3,//2.0V
//	.vib_vol = 0x4,//2.5V
//	.vib_vol = 0x5,//2.8V
	.vib_vol = 0x6,//3.0V
//	.vib_vol = 0x7,//3.3V
#endif
};

struct vibrator_hw *get_cust_vibrator_hw(void)
{
#ifdef CUST_VIBR_VOL
//	cust_vibrator_hw.vib_vol = 0x2;//1.5V
//	cust_vibrator_hw.vib_vol = 0x3;//2.0V
//	cust_vibrator_hw.vib_vol = 0x4;//2.5V
//	cust_vibrator_hw.vib_vol = 0x5;//2.8V
	cust_vibrator_hw.vib_vol = 0x6;//3.0V
//	cust_vibrator_hw.vib_vol = 0x7;//3.3V
#endif
    return &cust_vibrator_hw;
}

