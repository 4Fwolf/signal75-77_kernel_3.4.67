/*
 * MD218A voice coil motor driver
 *
 *
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include "MT9P017AF.h"
#include "../camera/kd_camera_hw.h"

#define LENS_I2C_BUSNUM 1
static struct i2c_board_info __initdata kd_lens_dev={ I2C_BOARD_INFO("MT9P017AF", 0x18)};


#define MT9P017AF_DRVNAME "MT9P017AF"
#define MT9P017AF_VCM_WRITE_ID           0x18

#define MT9P017AF_DEBUG
#ifdef MT9P017AF_DEBUG
#define MT9P017AFDB printk
#else
#define MT9P017AFDB(x,...)
#endif

static spinlock_t g_MT9P017AF_SpinLock;

static struct i2c_client * g_pstMT9P017AF_I2Cclient = NULL;

static dev_t g_MT9P017AF_devno;
static struct cdev * g_pMT9P017AF_CharDrv = NULL;
static struct class *actuator_class = NULL;

static int  g_s4MT9P017AF_Opened = 0;
static long g_i4MotorStatus = 0;
static long g_i4Dir = 0;
static long g_i4Position = 0;
static unsigned long g_u4MT9P017AF_INF = 0;
static unsigned long g_u4MT9P017AF_MACRO = 1023;
static unsigned long g_u4TargetPosition = 0;
static unsigned long g_u4CurrPosition   = 0;


extern int mt_set_gpio_mode(unsigned long u4Pin, unsigned long u4Mode);
extern int mt_set_gpio_out(unsigned long u4Pin, unsigned long u4PinOut);
extern int mt_set_gpio_dir(unsigned long u4Pin, unsigned long u4Dir);

extern void MT9P017_write_cmos_sensor(kal_uint32 addr, kal_uint32 para);
extern kal_uint16 MT9P017_read_cmos_sensor(kal_uint32 addr);

static int s4MT9P017AF_ReadReg(unsigned short * a_pu2Result)
{
    *a_pu2Result = MT9P017_read_cmos_sensor(0x30f2)<<2;

MT9P017AFDB("s4MT9P017AF_ReadReg =0x%x \n", *a_pu2Result);

    return 0;
}

static int s4MT9P017AF_WriteReg(u16 a_u2Data)
{
    a_u2Data = a_u2Data >>2;
	
MT9P017AFDB("s4MT9P017AF_WriteReg2 =0x%x \n", a_u2Data);
    MT9P017_write_cmos_sensor(0x30f2,a_u2Data);

    return 0;
}

inline static int getMT9P017AFInfo(__user stMT9P017AF_MotorInfo * pstMotorInfo)
{
    stMT9P017AF_MotorInfo stMotorInfo;
    stMotorInfo.u4MacroPosition   = g_u4MT9P017AF_MACRO;
    stMotorInfo.u4InfPosition     = g_u4MT9P017AF_INF;
    stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
	if (g_i4MotorStatus == 1)	{stMotorInfo.bIsMotorMoving = 1;}
	else						{stMotorInfo.bIsMotorMoving = 0;}

	if (g_s4MT9P017AF_Opened >= 1)	{stMotorInfo.bIsMotorOpen = 1;}
	else						{stMotorInfo.bIsMotorOpen = 0;}

    if(copy_to_user(pstMotorInfo , &stMotorInfo , sizeof(stMT9P017AF_MotorInfo)))
    {
        MT9P017AFDB("[MT9P017AF] copy to user failed when getting motor information \n");
    }

    return 0;
}

inline static int moveMT9P017AF(unsigned long a_u4Position)
{
    int ret = 0;
    
    if((a_u4Position > g_u4MT9P017AF_MACRO) || (a_u4Position < g_u4MT9P017AF_INF))
    {
        MT9P017AFDB("[MT9P017AF] out of range \n");
        return -EINVAL;
    }

	if (g_s4MT9P017AF_Opened == 1)
	{
		unsigned short InitPos;
	    ret = s4MT9P017AF_ReadReg(&InitPos);
	    
	    spin_lock(&g_MT9P017AF_SpinLock);
		if(ret == 0)
		{
			MT9P017AFDB("[MT9P017AF] Init Pos %6d \n", InitPos);
			g_u4CurrPosition = (unsigned long)InitPos;
		}
		else
		{		
			g_u4CurrPosition = 0;
		}
		g_s4MT9P017AF_Opened = 2;
		spin_unlock(&g_MT9P017AF_SpinLock);
	}

	if      (g_u4CurrPosition < a_u4Position)
	{
		spin_lock(&g_MT9P017AF_SpinLock);	
		g_i4Dir = 1;
		spin_unlock(&g_MT9P017AF_SpinLock);	
	}
	else if (g_u4CurrPosition > a_u4Position)
	{
		spin_lock(&g_MT9P017AF_SpinLock);	
		g_i4Dir = -1;
		spin_unlock(&g_MT9P017AF_SpinLock);			
	}
	else										{return 0;}

    spin_lock(&g_MT9P017AF_SpinLock);
	g_i4Position = (long)g_u4CurrPosition;
	g_u4TargetPosition = a_u4Position;
    spin_unlock(&g_MT9P017AF_SpinLock);	

	if (g_i4Dir == 1)
	{	
	    spin_lock(&g_MT9P017AF_SpinLock);
		g_i4MotorStatus = 0;
		spin_unlock(&g_MT9P017AF_SpinLock);	
		
		if(s4MT9P017AF_WriteReg((unsigned short)g_u4TargetPosition) == 0)
		{
			spin_lock(&g_MT9P017AF_SpinLock);		
			g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
			spin_unlock(&g_MT9P017AF_SpinLock);				
		}
		else
		{
			MT9P017AFDB("[MT9P017AF] set I2C failed when moving the motor \n");			
			spin_lock(&g_MT9P017AF_SpinLock);
			g_i4MotorStatus = -1;
			spin_unlock(&g_MT9P017AF_SpinLock);				
		}
	}
	else if (g_i4Dir == -1)
	{
		spin_lock(&g_MT9P017AF_SpinLock);	
		g_i4MotorStatus = 0;		
		spin_unlock(&g_MT9P017AF_SpinLock);	
		
		if(s4MT9P017AF_WriteReg((unsigned short)g_u4TargetPosition) == 0)
		{
			spin_lock(&g_MT9P017AF_SpinLock);		
			g_u4CurrPosition = (unsigned long)g_u4TargetPosition;			
			spin_unlock(&g_MT9P017AF_SpinLock);	
		}
		else
		{
			MT9P017AFDB("[MT9P017AF] set I2C failed when moving the motor \n");
			spin_lock(&g_MT9P017AF_SpinLock);			
			g_i4MotorStatus = -1;			
			spin_unlock(&g_MT9P017AF_SpinLock);	
		}
	}

    return 0;
}

inline static int setMT9P017AFInf(unsigned long a_u4Position)
{
    spin_lock(&g_MT9P017AF_SpinLock);
	g_u4MT9P017AF_INF = a_u4Position;
    spin_unlock(&g_MT9P017AF_SpinLock);	
	return 0;
}

inline static int setMT9P017AFMacro(unsigned long a_u4Position)
{
    spin_lock(&g_MT9P017AF_SpinLock);
	g_u4MT9P017AF_MACRO = a_u4Position;
    spin_unlock(&g_MT9P017AF_SpinLock);	
	return 0;	
}

////////////////////////////////////////////////////////////////
static long MT9P017AF_Ioctl(
struct file * a_pstFile,
unsigned int a_u4Command,
unsigned long a_u4Param)
{
    long i4RetValue = 0;

    switch(a_u4Command)
    {
        case MT9P017AFIOC_G_MOTORINFO :
            i4RetValue = getMT9P017AFInfo((__user stMT9P017AF_MotorInfo *)(a_u4Param));
        break;

        case MT9P017AFIOC_T_MOVETO :
            i4RetValue = moveMT9P017AF(a_u4Param);
        break;
 
 		case MT9P017AFIOC_T_SETINFPOS :
			 i4RetValue = setMT9P017AFInf(a_u4Param);
		break;

 		case MT9P017AFIOC_T_SETMACROPOS :
			 i4RetValue = setMT9P017AFMacro(a_u4Param);
		break;
		
        default :
      	     MT9P017AFDB("[MT9P017AF] No CMD \n");
            i4RetValue = -EPERM;
        break;
    }

    return i4RetValue;
}

//Main jobs:
// 1.check for device-specified errors, device not ready.
// 2.Initialize the device if it is opened for the first time.
// 3.Update f_op pointer.
// 4.Fill data structures into private_data
//CAM_RESET
static int MT9P017AF_Open(struct inode * a_pstInode, struct file * a_pstFile)
{
    spin_lock(&g_MT9P017AF_SpinLock);

    if(g_s4MT9P017AF_Opened)
    {
        spin_unlock(&g_MT9P017AF_SpinLock);
        MT9P017AFDB("[MT9P017AF] the device is opened \n");
        return -EBUSY;
    }

    g_s4MT9P017AF_Opened = 1;
		
    spin_unlock(&g_MT9P017AF_SpinLock);

    return 0;
}

//Main jobs:
// 1.Deallocate anything that "open" allocated in private_data.
// 2.Shut down the device on last close.
// 3.Only called once on last time.
// Q1 : Try release multiple times.
static int MT9P017AF_Release(struct inode * a_pstInode, struct file * a_pstFile)
{
	unsigned int cnt = 0;

	if (g_s4MT9P017AF_Opened)
	{
		moveMT9P017AF(g_u4MT9P017AF_INF);

		while(g_i4MotorStatus)
		{
			msleep(1);
			cnt++;
			if (cnt>1000)	{break;}
		}
		
    	spin_lock(&g_MT9P017AF_SpinLock);

	    g_s4MT9P017AF_Opened = 0;

    	spin_unlock(&g_MT9P017AF_SpinLock);

	}

    return 0;
}

static const struct file_operations g_stMT9P017AF_fops = 
{
    .owner = THIS_MODULE,
    .open = MT9P017AF_Open,
    .release = MT9P017AF_Release,
    .unlocked_ioctl = MT9P017AF_Ioctl
};

inline static int Register_MT9P017AF_CharDrv(void)
{
    struct device* vcm_device = NULL;

    //Allocate char driver no.
    if( alloc_chrdev_region(&g_MT9P017AF_devno, 0, 1,MT9P017AF_DRVNAME) )
    {
        MT9P017AFDB("[MT9P017AF] Allocate device no failed\n");

        return -EAGAIN;
    }

    //Allocate driver
    g_pMT9P017AF_CharDrv = cdev_alloc();

    if(NULL == g_pMT9P017AF_CharDrv)
    {
        unregister_chrdev_region(g_MT9P017AF_devno, 1);

        MT9P017AFDB("[MT9P017AF] Allocate mem for kobject failed\n");

        return -ENOMEM;
    }

    //Attatch file operation.
    cdev_init(g_pMT9P017AF_CharDrv, &g_stMT9P017AF_fops);

    g_pMT9P017AF_CharDrv->owner = THIS_MODULE;

    //Add to system
    if(cdev_add(g_pMT9P017AF_CharDrv, g_MT9P017AF_devno, 1))
    {
        MT9P017AFDB("[MT9P017AF] Attatch file operation failed\n");

        unregister_chrdev_region(g_MT9P017AF_devno, 1);

        return -EAGAIN;
    }

    actuator_class = class_create(THIS_MODULE, "actuatordrv");
    if (IS_ERR(actuator_class)) {
        int ret = PTR_ERR(actuator_class);
        MT9P017AFDB("Unable to create class, err = %d\n", ret);
        return ret;            
    }

    vcm_device = device_create(actuator_class, NULL, g_MT9P017AF_devno, NULL, MT9P017AF_DRVNAME);

    if(NULL == vcm_device)
    {
        return -EIO;
    }
    
    return 0;
}

inline static void Unregister_MT9P017AF_CharDrv(void)
{
    //Release char driver
    cdev_del(g_pMT9P017AF_CharDrv);

    unregister_chrdev_region(g_MT9P017AF_devno, 1);
    
    device_destroy(actuator_class, g_MT9P017AF_devno);

    class_destroy(actuator_class);
}

//////////////////////////////////////////////////////////////////////

static int MT9P017AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int MT9P017AF_i2c_remove(struct i2c_client *client);
static const struct i2c_device_id MT9P017AF_i2c_id[] = {{MT9P017AF_DRVNAME,0},{}};   
struct i2c_driver MT9P017AF_i2c_driver = {                       
    .probe = MT9P017AF_i2c_probe,                                   
    .remove = MT9P017AF_i2c_remove,                           
    .driver.name = MT9P017AF_DRVNAME,                 
    .id_table = MT9P017AF_i2c_id,                             
};  

#if 0 
static int MT9P017AF_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {         
    strcpy(info->type, MT9P017AF_DRVNAME);                                                         
    return 0;                                                                                       
}      
#endif 
static int MT9P017AF_i2c_remove(struct i2c_client *client) {
    return 0;
}

/* Kirby: add new-style driver {*/
static int MT9P017AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int i4RetValue = 0;

    MT9P017AFDB("[MT9P017AF] Attach I2C \n");

    /* Kirby: add new-style driver { */
    g_pstMT9P017AF_I2Cclient = client;
    
    g_pstMT9P017AF_I2Cclient->addr = g_pstMT9P017AF_I2Cclient->addr >> 1;
    
    //Register char driver
    i4RetValue = Register_MT9P017AF_CharDrv();

    if(i4RetValue){

        MT9P017AFDB("[MT9P017AF] register char device failed!\n");

        return i4RetValue;
    }

    spin_lock_init(&g_MT9P017AF_SpinLock);

    MT9P017AFDB("[MT9P017AF] Attached!! \n");

    return 0;
}

static int MT9P017AF_probe(struct platform_device *pdev)
{
    return i2c_add_driver(&MT9P017AF_i2c_driver);
}

static int MT9P017AF_remove(struct platform_device *pdev)
{
    i2c_del_driver(&MT9P017AF_i2c_driver);
    return 0;
}

static int MT9P017AF_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    return 0;
}

static int MT9P017AF_resume(struct platform_device *pdev)
{
    return 0;
}

// platform structure
static struct platform_driver g_stMT9P017AF_Driver = {
    .probe		= MT9P017AF_probe,
    .remove	= MT9P017AF_remove,
    .suspend	= MT9P017AF_suspend,
    .resume	= MT9P017AF_resume,
    .driver		= {
        .name	= "lens_actuator",
        .owner	= THIS_MODULE,
    }
};

static int __init MT9P017AF_i2C_init(void)
{
	i2c_register_board_info(LENS_I2C_BUSNUM, &kd_lens_dev, 1);
	
    if(platform_driver_register(&g_stMT9P017AF_Driver)){
        MT9P017AFDB("failed to register MT9P017AF driver\n");
        return -ENODEV;
    }

    return 0;
}

static void __exit MT9P017AF_i2C_exit(void)
{
	platform_driver_unregister(&g_stMT9P017AF_Driver);
}

module_init(MT9P017AF_i2C_init);
module_exit(MT9P017AF_i2C_exit);

MODULE_DESCRIPTION("MT9P017AF lens module driver");
MODULE_AUTHOR("Gipi Lin <Gipi.Lin@Mediatek.com>");
MODULE_LICENSE("GPL");


