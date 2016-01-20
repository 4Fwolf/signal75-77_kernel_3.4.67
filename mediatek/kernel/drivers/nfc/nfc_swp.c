
/*****************************************************************************
** Include Header file
******************************************************************************/
  #include  <linux/module.h>
  #include  <linux/kernel.h>
  #include  <linux/init.h>
  #include  <linux/major.h>
  #include  <linux/device.h>
  #include  <linux/gpio.h>
  #include  <linux/platform_device.h>
//#include  <linux/miscdevice.h>
  #include  <linux/pn544.h>

  #include  <mach/mt_devs.h>
  #include  <mach/mt_typedefs.h>
  #include  <mach/mt_gpio.h>
//#include  <mach/mt_pm_ldo.h>

/*****************************************************************************
** Macro-define
******************************************************************************/
#if !defined( GPIO_SIM_SWP_PIN )
  #if defined( HAWK40_EP0 ) || defined( HAWK40_EP1 ) || defined( HAWK40_EP2 )
    #define   GPIO_SIM_SWP_PIN      GPIO97
  #else /* Hawk35 */
    #define   GPIO_SIM_SWP_PIN      GPIO110
  #endif
#endif /* End.. !(GPIO_SIM_SWP_PIN) */

#if !defined( GPIO_SIM_SWP_PIN_M_GPIO )
    #define   GPIO_SIM_SWP_PIN_M_GPIO      GPIO_MODE_00
#endif /* End.. !(GPIO_SIM_SWP_PIN_M_GPIO) */

    #define   DEVICE_NAME         "nfc_swp"
  //#define   PATH_NFC_SWP_SIM    "/data/nfc/sim"

/*****************************************************************************
** 
******************************************************************************/
static struct class * nfc_swp_class = NULL;

static int  driver_major  = 0;
static int  nfc_entry = 0;
static int  sim_n = NFC_SWP_SIM1; //NFC_SWP_SIM2;

/*****************************************************************************
** 
******************************************************************************/
static int nfc_sim_swp( void )
{
//struct file * fp = NULL;

  printk("[NFC SWP] %s entry.\n", __func__ );
//  fp = filp_open( PATH_NFC_SWP_SIM, O_RDONLY, 0 /* S_IRUSR */);
//  if( IS_ERR( fp ))
//  {
//    sim_n = NFC_SWP_SIM1;
//  }
//  else
//  {
//    sim_n = NFC_SWP_SIM2;
//    filp_close( fp, NULL );
//  }

  mt_set_gpio_mode(GPIO_SIM_SWP_PIN, GPIO_SIM_SWP_PIN_M_GPIO );
  mt_set_gpio_dir( GPIO_SIM_SWP_PIN, GPIO_DIR_OUT );
  if( NFC_SWP_SIM2 == sim_n )  /* Set SIM2 */
  {
    mt_set_gpio_out( GPIO_SIM_SWP_PIN, GPIO_OUT_ZERO );
    printk("[NFC SWP] %s: SWP SIM2.\n", __func__ );
  }
  else
  {
    mt_set_gpio_out( GPIO_SIM_SWP_PIN, GPIO_OUT_ONE );
    printk("[NFC SWP] %s: SWP SIM1.\n", __func__ );
  }

  return  0;
} /* End.. nfc_sim_swp() */

/*****************************************************************************
** 
******************************************************************************/
static int nfc_swp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
//struct pn544_dev *pn544_dev = filp->private_data;

  switch( cmd )
  {
    case PN544_SIM_SWP:
    {
      sim_n = (unsigned int)arg;
      nfc_sim_swp();
      nfc_entry = 1;
    } break;

    default:
      printk("%s bad ioctl %u\n", __func__, cmd);
      return -EINVAL;
  }

  return  0;
} /* End.. nfc_swp_ioctl() */

/*****************************************************************************
** 
******************************************************************************/
static ssize_t nfc_swp_read(struct file *file, char __user *buf,
        size_t count, loff_t *ppos)
{
  return  0;
} /* End.. nfc_swp_write() */

/*****************************************************************************
** 
******************************************************************************/
static ssize_t nfc_swp_write(struct file *file, const char __user *buf,
        size_t count, loff_t *ppos)
{
  return  0;
} /* End.. nfc_swp_write() */

/*****************************************************************************
** 
******************************************************************************/
static int nfc_swp_open(struct inode *inode, struct file *file)
{
  /*
  * This could (should?) be enforced by the permissions on /dev/nfc_swp.
  */

  return  0;
} /* End.. nfc_swp_ioctl() */

/*****************************************************************************
** 
******************************************************************************/
static const struct file_operations nfc_swp_fops = {
  .owner    = THIS_MODULE,
  .read     = nfc_swp_read,
  .write    = nfc_swp_write,
  .open     = nfc_swp_open,
  .unlocked_ioctl  = nfc_swp_ioctl,
};

static int __init nfc_swp_init(void)
{
  printk( KERN_INFO "[NFC SWP] %s ......\n", __func__ );
  driver_major = register_chrdev( 0, DEVICE_NAME, &nfc_swp_fops );
  if( driver_major < 0 )
  {
    printk( KERN_ERR "[NFC SWP] Register nfc swp failed\n");
    return  -EFAULT;
  }
  else
  {
    printk( KERN_DEBUG "Mknod /dev/%s c %d 0\n", DEVICE_NAME, driver_major );
  }

  nfc_swp_class = class_create( THIS_MODULE, DEVICE_NAME );
  if( IS_ERR( nfc_swp_class ))
    return  -EFAULT;

  device_create( nfc_swp_class, NULL, MKDEV( driver_major, 0 ), NULL, DEVICE_NAME );

  return 0;
}

static void __exit nfc_swp_exit(void)
{
  unregister_chrdev( driver_major, DEVICE_NAME );
  device_destroy( nfc_swp_class, MKDEV( driver_major, 0 ));
  class_destroy( nfc_swp_class );

  printk( KERN_INFO "[NFC SWP] %s: Removed driver...\n", __func__ );
}

module_init(nfc_swp_init);
module_exit(nfc_swp_exit);

MODULE_DESCRIPTION("NFC SIM SWP");
MODULE_AUTHOR("Yuting Shih");
MODULE_LICENSE("GPL");
