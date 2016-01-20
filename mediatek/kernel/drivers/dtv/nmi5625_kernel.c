#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/gpio.h>
#include <linux/fs.h>

//#include "cust_matv.h"
#include "nm5625_kernel.h"
#include "mach/mt6575_gpio.h"
#include "nmi_gpio_i2c.h"

//<2012/9/6-NJRTC0000-Cokychen, [8317][DRV]Add for nmi5625 DTV driver
#include <mach/mt_devs.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_pm_ldo.h>
//>2012/9/6-NJRTC0000-Cokychen
//
//add for spi
#include <linux/slab.h>
#include "nmi326.h"
#include "nmi326_spi_drv.h"
//end for spi

MODULE_AUTHOR("nmi");
MODULE_DESCRIPTION("nmi TV 5625 driver");
MODULE_LICENSE("GPL");



/**************************************************************
	
	Global Defines & Variable

**************************************************************/

struct nmi_5625_dev {
	struct i2c_client *i2c_client_atv;

	struct mutex mu;
	struct class *tv_class;
	dev_t devn;
	struct	cdev cdv;
	//add for spi
	
	unsigned char* rwBuf;
	//end for spi
};

static int already_init = 0;
static struct nmi_5625_dev nd;

/**************************************************************
	
	Declareation:

**************************************************************/
//<2012/9/6-NJRTC0000-Cokychen, [8317][DRV]Add for nmi5625 DTV driver
#define NMI_POWER_VDDIO_PIN            gpio_nmi_power_vddio_pin //0xFF  //40        //gpio48  for VDDIO(2.8V)
#define NMI_POWER_VCORE_PIN            gpio_nmi_power_vcore_pin //0xFF //75  //   VCORE_1.2  

#define NMI_RESET_PIN               gpio_nmi_reset_pin //0xFF //30  

/*************************************************************
 *When entry DTV, GPIO124 = 0 && GPIO125 = 1, RF & Audio switch to DTV;
 *When exit DTV, GPIO124 = 1, GPIO125 = 0, RF & Audio  switch to FM;
 *************************************************************/
#define DTV_FM_AUDIO_SWITCH_PIN  gpio_dtv_fm_audio_switch_pin
#define DTV_FM_RF_SWITCH_PIN   gpio_dtv_fm_rf_switch_pin
//>2012/9/6-NJRTC0000-Cokychen

/**************************************************************
	
	V4L2: IOCTL functions from v4l2_int_ioctl_desc

**************************************************************/

/**************************************************************
	
	file operation:

**************************************************************/
#define SPI_RW_BUF 188*50*2

static int nmi5625_open(struct inode *inode, struct file *file)
{
	int ret = 0;

	func_enter();
	if (!already_init) {
		ret = -ENODEV;
		goto _fail_;
	}

	file->private_data = (void *)&nd;
	nd.rwBuf = kmalloc(SPI_RW_BUF, GFP_KERNEL);
	if ( nd.rwBuf == NULL )
	{
		dPrint(N_ERR,"dmb_open() nd.rwBuf : kmalloc failed\n");
		return -1;
	}

	//<2012/9/6-NJRTC0000-Cokychen, [8317][DRV]Add for nmi5625 DTV driver
	mt_set_gpio_out(DTV_FM_AUDIO_SWITCH_PIN, 1);
	mt_set_gpio_out(DTV_FM_RF_SWITCH_PIN, 0);
	//>2012/9/6-NJRTC0000-Cokychen
 
_fail_:

	func_exit();
	return ret;
}

static int nmi5625_release(struct inode * inode, struct file * file)
{
	int ret = 0;
	struct nmi_5625_dev *d = file->private_data;

	/**
		nothing to do
	**/
	func_enter();

	//<2012/9/6-NJRTC0000-Cokychen, [8317][DRV]Add for nmi5625 DTV driver
	mt_set_gpio_out(DTV_FM_AUDIO_SWITCH_PIN, 0);
	mt_set_gpio_out(DTV_FM_RF_SWITCH_PIN, 1);
	//>2012/9/6-NJRTC0000-Cokychen	

	func_exit();
	return ret;
}
static u8 i2cBuf[32];
bool g_bIsAtvStart  =	false;
static int nmi5625_ioctl(/*struct inode *inode, */struct file *file,
		    unsigned int cmd, unsigned long arg)
{
	struct nmi_5625_dev *d = file->private_data;
	int ret = 0;
	#define NMI_I2C_RW_LENGTH	256
	//func_enter();

	switch ((cmd&0xffff0000)) {
		case NM5625_PWR_2P8_CTL:
			dPrint(N_TRACE,"NM5625_PWR_2P8_CTL, power %s\n", (arg==1)?"on":"off");
			if (arg == 1) {	/* on */
                           mt_set_gpio_out(NMI_POWER_VDDIO_PIN, 1);
			       //<2012/9/6-NJRTC0000-Cokychen, [8317][DRV]Add for nmi5625 DTV driver
			       hwPowerOn( MT65XX_POWER_LDO_VCAM_IO, VOL_1800,"nmiatv" );
			       //>2012/9/6-NJRTC0000-Cokychen
             			} 
			else	{
				mt_set_gpio_out(NMI_POWER_VDDIO_PIN, 0);
				//<2012/9/6-NJRTC0000-Cokychen, [8317][DRV]Add for nmi5625 DTV driver
				hwPowerDown( MT65XX_POWER_LDO_VCAM_IO, "nmiatv" );		
				//>2012/9/6-NJRTC0000-Cokychen
				}

			break;
		case NM5625_PWR_1P2_CTL:
			dPrint(N_TRACE,"NM5625_PWR_1P2_CTL, power %s\n", (arg==1)?"on":"off");
			if (arg == 1) {	/* on */
				mt_set_gpio_out(NMI_POWER_VCORE_PIN, 1);
				g_bIsAtvStart  = true;
        			//<2012/9/6-NJRTC0000-Cokychen, [8317][DRV]Add for nmi5625 DTV driver
			       //hwPowerOn( MT65XX_POWER_LDO_VCAM_IO, VOL_1800,"nmiatv" );
			       //>2012/9/6-NJRTC0000-Cokychen
				} 
			else {
				mt_set_gpio_out(NMI_POWER_VCORE_PIN, 0);
				g_bIsAtvStart = false;
		 		//<2012/9/6-NJRTC0000-Cokychen, [8317][DRV]Add for nmi5625 DTV driver
				//hwPowerDown( MT65XX_POWER_LDO_VCAM_IO, "nmiatv" );		
				//>2012/9/6-NJRTC0000-Cokychen
				}	
			break;
			
		case NM5625_ATV_RESET_CTL:
			dPrint(N_TRACE,"NM5625_ATV_RESET_CTL, reset %s\n", (arg==1)?"high":"low");
           	if (arg == 1) {
						mt_set_gpio_out(NMI_RESET_PIN, 1);
									} 
				else {
						mt_set_gpio_out(NMI_RESET_PIN, 0);

						}
			break;
			
			
		case NM5625_ATV_I2C_READ:
			{
				u8 *kbuf = &i2cBuf[0];
				int size = cmd&0xffff;	/* Note: I used the lower 16 bits for size */	
				int len = size;
				dPrint(N_TRACE,"NM5625_ATV_I2C_READ\n");
				mutex_lock(&d->mu);
				while(len) {
					int sz;
					if (len > NMI_I2C_RW_LENGTH)
						sz = NMI_I2C_RW_LENGTH;
					else
						sz = len;
					ret = i2c_master_recv(d->i2c_client_atv, kbuf, sz); 
					if (ret < 0) {
						dPrint(N_ERR, "nmi: failed i2c read...(%d)\n", ret);
						//kfree(kbuf);
						mutex_unlock(&d->mu);
						goto _fail_;
					}
					kbuf += NMI_I2C_RW_LENGTH;
					len -= sz;
				}
				//nmi_i2c_read(0x60,kbuf,size);

				if (copy_to_user(arg, i2cBuf, size) ) {
					dPrint(N_ERR, "nmi: failed copy to user...\n");
					ret = -EFAULT;
					//kfree(kbuf);
					mutex_unlock(&d->mu);
					goto _fail_;
				}
				//kfree(kbuf);
				mutex_unlock(&d->mu);
			}
			break;
			case NM5625_ATV_SPI_READ:
			{
				u8 *kbuf = &i2cBuf[0];
				int size = cmd&0xffff;	/* Note: I used the lower 16 bits for size */	
				int len = size;
				dPrint(N_TRACE,"NM5625_ATV_SPI_READ\n");
				mutex_lock(&d->mu);
				/*while(len) {
					int sz;
					if (len > NMI_I2C_RW_LENGTH)
						sz = NMI_I2C_RW_LENGTH;
					else
						sz = len;
					ret = i2c_master_recv(d->i2c_client_atv, kbuf, sz); 
					if (ret < 0) {
						dPrint(N_ERR, "nmi: failed i2c read...(%d)\n", ret);
						//kfree(kbuf);
						mutex_unlock(&d->mu);
						goto _fail_;
					}
					kbuf += NMI_I2C_RW_LENGTH;
					len -= sz;
				}*/
				//nmi326_spi_read(d->rwBuf,size);
				//nmi_i2c_read(0x60,kbuf,size);
	ret = nmi326_spi_read(d->rwBuf, size);
	if (ret < 0) {
		dPrint(N_ERR,"dmb_read() : nmi326_spi_read failed(ret:%d)\n", ret); 
		goto _fail_;
	}
	/* move data from kernel area to user area */
	if (copy_to_user (arg, d->rwBuf, size)) {
		dPrint(N_ERR,"dmb_read() : copy_to_user failed\n"); 
		goto _fail_;
	}

			/*	if (copy_to_user(arg, i2cBuf, size) ) {
					dPrint(N_ERR, "nmi: failed copy to user...\n");
					ret = -EFAULT;
					//kfree(kbuf);
					mutex_unlock(&d->mu);
					goto _fail_;
				}*/
				//kfree(kbuf);
				mutex_unlock(&d->mu);
			}
			break;
		case NM5625_ATV_I2C_WRITE:
			{
				u8 *kbuf = &i2cBuf[0];
				int size = cmd&0xffff;	/* Note: I used the lower 16 bits for size */
				int len = size;
				dPrint(N_TRACE,"NM5625_ATV_I2C_WRITE\n");
				if (copy_from_user(kbuf, arg, size)) {					
					dPrint(N_ERR, "nmi: failed copy from user...\n");
					ret = -EFAULT;
					goto _fail_;
				}
				mutex_lock(&d->mu);
				while(len){
					int sz;
					if (len > NMI_I2C_RW_LENGTH)
						sz = NMI_I2C_RW_LENGTH;
					else
						sz = len;
					ret = i2c_master_send(d->i2c_client_atv, kbuf, sz);
					if (ret < 0) {
						dPrint(N_ERR, "nmi: failed i2c write...(%d)\n", ret);
						//kfree(kbuf);
						mutex_unlock(&d->mu);
						goto _fail_;
					}
					kbuf += NMI_I2C_RW_LENGTH;
					len -= sz;
				}
			//	nmi_i2c_write(0x60,kbuf,size);
				
				mutex_unlock(&d->mu);
			}
			break;
			case NM5625_ATV_SPI_WRITE:
			{
				u8 *kbuf = &i2cBuf[0];
				int size = cmd&0xffff;	/* Note: I used the lower 16 bits for size */
				int len = size;
				dPrint(N_TRACE,"NM5625_ATV_SPI_WRITE\n");
			/*	if (copy_from_user(kbuf, arg, size)) {					
					dPrint(N_ERR, "nmi: failed copy from user...\n");
					ret = -EFAULT;
					goto _fail_;
				}*/
				mutex_lock(&d->mu);
						/* move data from user area to kernel  area */
		if(copy_from_user(d->rwBuf,arg, size)) {
			dPrint(N_ERR,"dmb_write() : copy_from_user failed\n"); 
			ret = -EFAULT;
			goto _fail_;
		}

		ret = nmi326_spi_write(d->rwBuf, size);
		if (ret < 0) {
			dPrint(N_ERR,"dmb_write() : nmi326_spi_write failed(ret:%d)\n", ret); 
			goto _fail_;
		}
			/*	while(len){
					int sz;
					if (len > NMI_I2C_RW_LENGTH)
						sz = NMI_I2C_RW_LENGTH;
					else
						sz = len;
					ret = i2c_master_send(d->i2c_client_atv, kbuf, sz);
					if (ret < 0) {
						dPrint(N_ERR, "nmi: failed i2c write...(%d)\n", ret);
						//kfree(kbuf);
						mutex_unlock(&d->mu);
						goto _fail_;
					}
					kbuf += NMI_I2C_RW_LENGTH;
					len -= sz;
				}*/
			//	nmi_i2c_write(0x60,kbuf,size);
				
				mutex_unlock(&d->mu);
			}
			break;
		default:
			break;
	}

_fail_:
	//func_exit();
	dPrint(N_TRACE, "nmi_ioctl return value...(%d)\n", ret);
	return ret; 
}

static const struct file_operations nmi5625_fops = {
	.owner		= THIS_MODULE,
 	.unlocked_ioctl			= nmi5625_ioctl,
	.open		= nmi5625_open,
	.release	= nmi5625_release,
};

/**************************************************************
	
	i2c:

**************************************************************/

static int nmi5625_remove(struct i2c_client *client)
{
	int ret = 0;

	func_enter();

	nd.i2c_client_atv = NULL;

	func_exit();
	
	return ret;
}

static int nmi5625_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {         
    strcpy(info->type, "nmiatv");                                                         
    return 0;                                                                                       
}        

static int nmi5625_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	struct device *dev;
	func_enter();

	if (!already_init) {
		memset(&nd, 0, sizeof(struct nmi_5625_dev));

		/**
			initialize mutex
		**/
		mutex_init(&nd.mu);

		/**
			register our driver
		**/
		if ((ret = alloc_chrdev_region (&nd.devn, 0, 1, "nmi")) < 0) {
			dPrint(N_ERR, "nmi: failed unable to get major...%d\n", ret);
			goto _fail_;
		}
		dPrint(N_INFO, "nmi:dynamic major(%d),minor(%d)\n", MAJOR(nd.devn), MINOR(nd.devn));

		cdev_init(&nd.cdv, &nmi5625_fops);
		nd.cdv.owner = THIS_MODULE;
		ret = cdev_add(&nd.cdv, nd.devn, 1);
		if (ret) {
			dPrint(N_ERR, "nmi: failed to add device...%d\n", ret);
			goto _fail_;
		}


		nd.tv_class = class_create(THIS_MODULE, "atv");
		if (IS_ERR(nd.tv_class)) {
			dPrint(N_ERR, "nmi: failed to create the atv class\n");
		}

		dev = device_create(nd.tv_class, NULL, nd.devn, NULL, "nmi");
		if (IS_ERR(dev)) {
			dPrint(N_ERR, "nmi: failed to create device\n");
		}
		/*User interface end */

		already_init = 1;
	}

		nd.i2c_client_atv = client;
		nd.i2c_client_atv->timing = 400;


_fail_:

	func_exit();
	return ret;
}

static const struct i2c_device_id nmi5625_id[] = {
	{"nmiatv", 0},
	{},
};

//<2012/9/6-NJRTC0000-Cokychen, [8317][DRV]Add for nmi5625 DTV driver
//static unsigned short force[] = {2, 0xc0, I2C_CLIENT_END, I2C_CLIENT_END};   
//static const unsigned short * const forces[] = { force, NULL };              
//static struct i2c_client_address_data addr_data = { .forces = forces,};
static struct i2c_board_info __initdata nmi5625_i2c_info = { I2C_BOARD_INFO("nmiatv", (0xc0>>1))};

static struct i2c_driver nmi5625_i2c_driver = {
	.driver = {
		  .owner = THIS_MODULE,
		  .name  = "nmiatv",
		  },
	.probe  = nmi5625_probe,
	.detect = nmi5625_detect,
	.remove = nmi5625_remove,
	.id_table = nmi5625_id,
	//.address_data =&addr_data,
};
//>2012/9/6-NJRTC0000-Cokychen


//<2012/11/15-17026-Cokychen, [8317][DTV] Add autodetect DTV flag to distinguis EP0/EP1 headset gpio setting
#if 0
bool g_bIsDtvBoard  =	 false;
static void nmi_power_up_privtae(void)
{
	mt_set_gpio_out(NMI_POWER_VDDIO_PIN, 1);
	hwPowerOn( MT65XX_POWER_LDO_VCAM_IO, VOL_1800,"nmiatv" );
	mt_set_gpio_out(NMI_POWER_VCORE_PIN, 1);

	mt_set_gpio_out(NMI_RESET_PIN, 1);
	mdelay( 10);
	mt_set_gpio_out(NMI_RESET_PIN, 0);
	mdelay( 10);
	mt_set_gpio_out(NMI_RESET_PIN, 1);
	mdelay( 10);
}
static void nmi_power_down_private(void)
{
	mt_set_gpio_out(NMI_POWER_VDDIO_PIN, 0);
	hwPowerDown( MT65XX_POWER_LDO_VCAM_IO, "nmiatv" );		
	mt_set_gpio_out(NMI_POWER_VCORE_PIN, 0);
	mt_set_gpio_out(NMI_RESET_PIN, 0);
}
void check_dtv_board()
{	
	int  RetValue = 0;
       u8 i2cTestBuf[32] = {0.};
	int retry;

	printk("check_dtv_board enter\n");
	nmi_power_up_privtae();

	if (nd.i2c_client_atv == NULL)
	{
		printk("check_dtv_board i2c not init\n");
		return;
	}
	
	retry = 20;
	do {
		RetValue = i2c_master_send(nd.i2c_client_atv, i2cTestBuf, 32);
		printk("check_dtv_board RetValue = %d\n", RetValue);

		if (RetValue == 32)
		{
			g_bIsDtvBoard = true;
			break;
		}
		else 
		{
			 g_bIsDtvBoard = false;
		}
					
	}while (retry--);

	printk("check_dtv_board g_bIsDtvBoard = %d\n",g_bIsDtvBoard);
	nmi_power_down_private();
	printk("check_dtv_board exit\n");
}

EXPORT_SYMBOL(check_dtv_board);
#endif
//>2012/11/15-17026-Cokychen

/**************************************************************
	
	Module:

**************************************************************/
static __init int nmi5625_init(void)
{
	int ret;
	func_enter();
	
	    //PWR Enable
    mt_set_gpio_mode(NMI_POWER_VDDIO_PIN,GPIO_MODE_00);
    mt_set_gpio_dir(NMI_POWER_VDDIO_PIN, GPIO_DIR_OUT);
    mt_set_gpio_pull_enable(NMI_POWER_VDDIO_PIN,true);
    mt_set_gpio_out(NMI_POWER_VDDIO_PIN, 0);

	//LDO_Enable
	    mt_set_gpio_mode(NMI_POWER_VCORE_PIN,GPIO_MODE_00);
    mt_set_gpio_dir(NMI_POWER_VCORE_PIN, GPIO_DIR_OUT);
    mt_set_gpio_pull_enable(NMI_POWER_VCORE_PIN,true);
    mt_set_gpio_out(NMI_POWER_VCORE_PIN, 0);
    
        //n_Reset
    mt_set_gpio_mode(NMI_RESET_PIN,GPIO_MODE_00);
    mt_set_gpio_dir(NMI_RESET_PIN, GPIO_DIR_OUT);
    mt_set_gpio_pull_enable(NMI_RESET_PIN,true);
    mt_set_gpio_out(NMI_RESET_PIN, 0);

    //i2c sda & scl
    //nmi_i2c_init();
    //add for spi
    nmi326_spi_init();
    //end for spi

    //<2012/9/6-NJRTC0000-Cokychen, [8317][DRV]Add for nmi5625 DTV driver
    i2c_register_board_info(1, &nmi5625_i2c_info, 1);
    //>2012/9/6-NJRTC0000-Cokychen

	ret = i2c_add_driver(&nmi5625_i2c_driver);
	if (ret < 0) {
		dPrint(N_ERR, "nmi: failed register i2c driver...(%d)\n", ret);
	}

	func_exit();

	return ret;
}

static __exit void nmi5625_clean(void)
{
	func_enter();

	i2c_del_driver(&nmi5625_i2c_driver);

	if (already_init) {
		device_destroy(nd.tv_class, nd.devn);
		cdev_del(&nd.cdv);
		unregister_chrdev_region(nd.devn, 1);
		already_init = 0;
	}
	//nmi_i2c_deinit();
    //add for spi
    nmi326_spi_exit();
    //end for spi
	func_exit();
}

module_init(nmi5625_init);
module_exit(nmi5625_clean);


