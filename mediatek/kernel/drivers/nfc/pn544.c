/*
 * Copyright (C) 2010 Trusted Logic S.A.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/*****************************************************************************
** Include
******************************************************************************/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
//#include <linux/dma-mapping.h>
#include <linux/pn544.h>

#include <mach/mt_devs.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/eint.h>
#include <mach/mt_pm_ldo.h>

#include <cust_gpio_usage.h>
#include <cust_eint.h>

/*****************************************************************************
** Define
******************************************************************************/
  #if defined( NFC_PRINT )
    #undef    NFC_PRINT
  #endif
  #if defined( BUILD_LK ) || defined( BUILD_UBOOT )
    #define   NFC_PRINT             printf
  #else
    #define   NFC_PRINT             printk
  #endif /** End.. (BUILD_LK) **/

  #if defined( NFC_MSG_ENABLE )
    #undef    NFC_MSG_ENABLE
  #endif
  //#define   NFC_MSG_ENABLE

  #if defined( NFC_DEBUG_ENABLE )
    #undef    NFC_DEBUG_ENABLE
  #endif
  //#define   NFC_DEBUG_ENABLE

  #if defined( NFC_MSG )
    #undef    NFC_MSG
  #endif
  #if defined( NFC_MSG_ENABLE )
    #define   NFC_MSG(srt,arg...)       NFC_PRINT(srt,##arg)
  #else
    #define   NFC_MSG(srt,arg...)       {/*Do Nothing*/}
  #endif

  #if defined( NFC_DBG )
    #undef    NFC_DBG
  #endif
  #if defined( NFC_DEBUG_ENABLE )
    #define   NFC_DBG(srt,arg...)       NFC_PRINT(srt,##arg)
  #else
    #define   NFC_DBG(srt,arg...)       {/*Do Nothing*/}
  #endif

//<2012/12/11 Yuting Shih. Protection:add for SIM SWP PIN define.
#if !defined( GPIO_SIM_SWP_PIN )
  #if defined( HAWK40_EP0 ) || defined( HAWK40_EP1 ) || defined( HAWK40_EP2 )
    #define   GPIO_SIM_SWP_PIN      GPIO97
  #else /* Hawk35 */
    #define   GPIO_SIM_SWP_PIN      GPIO110
  #endif /* End.. (ARIMA_PROJECT_HAWK40) */
#endif /* End.. !(GPIO_SIM_SWP_PIN) */

#if !defined( GPIO_SIM_SWP_PIN_M_GPIO )
  #if defined( HAWK40_EP0 ) || defined( HAWK40_EP1 ) || defined( HAWK40_EP2 )
    #define   GPIO_SIM_SWP_PIN_M_GPIO      GPIO_MODE_00
  #else
    #define   GPIO_SIM_SWP_PIN_M_GPIO      GPIO_MODE_00
  #endif /* End.. (ARIMA_PROJECT_HAWK40) */
#endif /* End.. !(GPIO_SIM_SWP_PIN_M_GPIO) */
//>2012/12/11 Yuting Shih.

    #define   NFC_DEV_NAME      "pn544"
    #define   I2C_ID_NAME       "pn544"
    #define   MAX_BUFFER_SIZE   512

  #if defined( I2C_DMA_USAGE )
    #undef    I2C_DMA_USAGE
  #endif
    #define   I2C_DMA_USAGE_N

  typedef struct st_pn544_device
  {
      wait_queue_head_t read_wq;
      struct mutex      read_mutex;
      struct i2c_client *client;
      struct miscdevice pn544_device;
      unsigned int      ven_gpio;
      unsigned int      firm_gpio;  //sysrstb_gpio;
      unsigned int      irq_gpio;   /* Chip inform Host */
      bool              irq_enabled;
      spinlock_t        irq_enabled_lock;
  } PN544_DEV;

/*****************************************************************************
** GLobal Variable
******************************************************************************/
static PN544_DEV  *g_pn544_dev = NULL;

////<2014/05/06-Yuting Shih. Modified for Android KK
//#if 1 //defined( CUST_EINT_ALS_TYPE )
//extern void mt_eint_mask(unsigned int eint_num);
//extern void mt_eint_unmask(unsigned int eint_num);
//extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
//extern void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol);
//extern unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
//extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
//extern void mt65xx_eint_registration(unsigned int eint_num, unsigned int is_deb_en, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
//extern void mt_eint_print_status(void);
//#else
//extern void mt65xx_eint_mask(unsigned int line);
//extern void mt65xx_eint_unmask(unsigned int line);
//extern void mt65xx_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
//extern void mt65xx_eint_set_polarity(unsigned int eint_num, unsigned int pol);
//extern unsigned int mt65xx_eint_set_sens(unsigned int eint_num, unsigned int sens);
//extern void mt65xx_eint_registration(unsigned int eint_num, unsigned int is_deb_en, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
//#endif
////>2014/05/06-Yuting Shih.

/*****************************************************************************
**
******************************************************************************/
static void pn544_disable_irq(PN544_DEV *pn544_dev)
{
unsigned long flags;

    spin_lock_irqsave( &pn544_dev->irq_enabled_lock, flags );
    if( pn544_dev->irq_enabled )
    {
    //disable_irq_nosync( pn544_dev->client->irq );
    //<2014/05/06-Yuting Shih. Modified for Android KK
    #if 1 //defined( CUST_EINT_ALS_TYPE )
      mt_eint_mask( CUST_EINT_NFC_NUM );
    #else
      mt65xx_eint_mask( CUST_EINT_NFC_NUM );
    #endif
    //>2014/05/06-Yuting Shih.
      pn544_dev->irq_enabled = false;
    }
    spin_unlock_irqrestore( &pn544_dev->irq_enabled_lock, flags );
}

/*****************************************************************************
**
******************************************************************************/
#if 1 //defined( ARIMA_PROJECT_HAWK40 )
static void pn544_dev_irq_handler(void)
{
PN544_DEV * pn544_dev = g_pn544_dev;

    if( !pn544_dev )
    {
      printk("pn544_dev null.\n");
      return;
    }

    if( !mt_get_gpio_in( GPIO_NFC_EINT_PIN ))
    {
      printk("pn544_dev no ENIT.\n");
      return;
    }

    pn544_disable_irq( pn544_dev );
  /* Wake up waiting readers */
    wake_up( &pn544_dev->read_wq );
}
#else
static irqreturn_t pn544_dev_irq_handler(int irq, void *dev_id)
{
PN544_DEV *pn544_dev = dev_id;

  //if( !gpio_get_value( pn544_dev->irq_gpio )) {
    if( !mt_get_gpio_in( GPIO_NFC_EINT_PIN ))
    {
      return IRQ_HANDLED;
    }

    pn544_disable_irq( pn544_dev );
  /* Wake up waiting readers */
    wake_up( &pn544_dev->read_wq );

  return IRQ_HANDLED;
}
#endif /* End.. (ARIMA_PROJECT_HAWK40) */

/*****************************************************************************
**
******************************************************************************/
static ssize_t pn544_dev_read(struct file *filp, char __user *buf,
    size_t count, loff_t *offset)
{
PN544_DEV *pn544_dev = filp->private_data;
int   ret, i;
unsigned short  addr;
char  tmp[MAX_BUFFER_SIZE];

    if( count > MAX_BUFFER_SIZE )
      count = MAX_BUFFER_SIZE;

    printk( KERN_DEBUG "%s : reading %zu bytes.\n", __func__, count );

    mutex_lock( &pn544_dev->read_mutex );

  //if( !gpio_get_value(pn544_dev->irq_gpio)) {
    if( !mt_get_gpio_in( GPIO_NFC_EINT_PIN ))
    {
      if( filp->f_flags & O_NONBLOCK )
      {
        ret = -EAGAIN;
        goto fail;
      }

      pn544_dev->irq_enabled = true;
    //enable_irq( pn544_dev->client->irq );
  //<2014/05/06-Yuting Shih. Modified for Android KK.
  #if 1 //defined( CUST_EINT_ALS_TYPE )
      mt_eint_unmask( CUST_EINT_NFC_NUM );
  #else
      mt65xx_eint_unmask( CUST_EINT_NFC_NUM );
   #endif
  //>2014/05/06-Yuting Shih.
    //ret = wait_event_interruptible( pn544_dev->read_wq,
    //            gpio_get_value( pn544_dev->irq_gpio ));
      ret = wait_event_interruptible( pn544_dev->read_wq,
                  mt_get_gpio_in( GPIO_NFC_EINT_PIN ));

    //pn544_disable_irq( pn544_dev );
    //<2014/05/06-Yuting Shih. Modified for Android KK.
    #if 1 //defined( CUST_EINT_ALS_TYPE )
      mt_eint_mask( CUST_EINT_NFC_NUM );
    #else
      mt65xx_eint_mask( CUST_EINT_NFC_NUM );
    #endif
    //>2014/05/06-Yuting Shih.
      pn544_dev->irq_enabled = false;

      if( ret )
        goto fail;
    }

//<2013/01/16-SunnyHu. Modify to prevent cardmode abnormal. This is root cause
  //msleep( 10 );//wait for long package write into chip ack
//>2013/01/16-SunnyHu.
    addr = pn544_dev->client->addr;
#if defined( I2C_DMA_USAGE )
    pn544_dev->client->addr     &= I2C_MASK_FLAG;
  //pn544_dev->client->addr     |= I2C_DMA_FLAG;
    pn544_dev->client->ext_flag |= I2C_DMA_FLAG;
  //pn544_dev->client->ext_flag |= I2C_DIRECTION_FLAG;
    pn544_dev->client->ext_flag |= I2C_A_FILTER_MSG;
#endif
  //pn544_dev->client->timing = 400;
  /* Read data */
    ret = i2c_master_recv( pn544_dev->client, tmp, count );

    pn544_dev->client->addr = addr;
    mutex_unlock( &pn544_dev->read_mutex );
    if( ret < 0 )
    {
      pr_err("%s: i2c_master_recv returned %d\n", __func__, ret );
      return ret;
    }
    if( ret > count )
    {
      pr_err("%s: received too many bytes from i2c (%d)\n", __func__, ret);
      return -EIO;
    }
    if( copy_to_user( buf, tmp, ret ))
    {
      pr_warning("%s : failed to copy to user space\n", __func__ );
      return -EFAULT;
    }

    printk( KERN_DEBUG "IFD->PC:" );
    for( i = 0; i < ret; i++ )
    {
      printk( KERN_DEBUG " %02X", tmp[i] );
    }
    printk( KERN_DEBUG "\n");

    return ret;

fail:
    mutex_unlock( &pn544_dev->read_mutex );
    return ret;
} /* End.. pn544_dev_read() */

/*****************************************************************************
**
******************************************************************************/
static ssize_t pn544_dev_write(struct file *filp, const char __user *buf,
    size_t count, loff_t *offset)
{
PN544_DEV * pn544_dev;
int   ret, i;
//<2012/10/25-yaotsulin. HAWK NFC firmware update C3 1.26
int   remain_count;
//>2012/10/25-yaotsulin.
//<2013/01/16-SunnyHu. Modify to prevent cardmode abnormal.
/* Modify for sending data buffer size > 255 */
int   send_count, sendOutCnt;
char  * ptmp;
//>2013/01/16-SunnyHu.
unsigned short  addr;
char  tmp[MAX_BUFFER_SIZE];

    pn544_dev = filp->private_data;
    if( count > MAX_BUFFER_SIZE )
      count = MAX_BUFFER_SIZE;

    if( copy_from_user( tmp, buf, count ))
    {
      pr_err("%s : failed to copy from user space\n", __func__ );
      return -EFAULT;
    }

    printk( KERN_DEBUG "%s : writing %zu bytes.\n", __func__, count );

//<2013/01/16-SunnyHu. Modify to prevent cardmode abnormal.
/* Modify for sending data buffer size > 255 */
    remain_count = count;
    ptmp = tmp;
    ret = 0;

    addr = pn544_dev->client->addr;
#if defined( I2C_DMA_USAGE )
    pn544_dev->client->addr     &= I2C_MASK_FLAG;
  //pn544_dev->client->addr     |= I2C_DMA_FLAG;
    pn544_dev->client->ext_flag |= I2C_DMA_FLAG;
  //pn544_dev->client->ext_flag |= I2C_DIRECTION_FLAG;
    pn544_dev->client->ext_flag |= I2C_A_FILTER_MSG;
#endif
  //pn544_dev->client->timing = 400;
    while( remain_count > 0 )
    {
      if( remain_count >= 255 )
        send_count = 255;
      else
        send_count = remain_count;

      sendOutCnt = i2c_master_send( pn544_dev->client, ptmp, send_count );
    //printk( KERN_DEBUG "%s : sendOutCnt = %d\n", __func__, sendOutCnt);
    /* Add retry by NXP suggestion,0719 */
      if( sendOutCnt == -EREMOTEIO )
      {
        usleep_range( 6000, 10000 ); //msleep(10);
        sendOutCnt = i2c_master_send( pn544_dev->client, ptmp, send_count );
        printk( KERN_DEBUG "[PN544_DEBUG] I2C Writer Retry - %d", sendOutCnt );
      }

    //printk( KERN_DEBUG "%s : sendOutCnt = %d, send_count = %d\n", __func__, sendOutCnt, send_count );
      if( sendOutCnt != send_count )
      {
        ret = sendOutCnt;
        break;
      }

      ret += sendOutCnt;
      remain_count -= sendOutCnt;
      ptmp += sendOutCnt;
    //printk( KERN_DEBUG "%s : ret = %d, remain_count = %d\n", __func__, ret, remain_count );

      if( remain_count <= 0 )
        break;
    } /* End.. while(remain_count>0) */

    pn544_dev->client->addr = addr;
    printk( KERN_DEBUG "%s : complete, result = %d\n", __func__, ret);
//>2013/01/16-SunnyHu.

    return ret;
} /* End.. pn544_dev_write() */

/*****************************************************************************
**
******************************************************************************/
static int pn544_dev_open(struct inode *inode, struct file *filp)
{
PN544_DEV *pn544_dev = container_of( filp->private_data,
                            PN544_DEV,
                            pn544_device );

    filp->private_data = pn544_dev;

    pr_debug("%s : %d,%d\n", __func__, imajor( inode ), iminor( inode ));

    return  0;
} /* End.. pn544_dev_open() */

/*****************************************************************************
**
******************************************************************************/
static long pn544_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
PN544_DEV * pn544_dev = filp->private_data;

  switch( cmd )
  {
    case PN544_SET_PWR:
    {
      if( arg == 2 )
      {
      /* power on with firmware download (requires hw reset) */
    //<2012/11/26-MaoyiChou. Removed VCAM_IO reason is not related.
      //hwPowerOn( MT65XX_POWER_LDO_VCAM_IO, VOL_1800, NFC_DEV_NAME );
    //>2012/11/26-MaoyiChou.
    //<2012/10/25-yaotsulin-15725. HAWK NFC firmware update C3 1.26
        printk( KERN_DEBUG "%s power on with firmware\n", __func__ );
        mt_set_gpio_out( GPIO_NFC_VEN_PIN, GPIO_OUT_ZERO );
      //printk( KERN_DEBUG "pn544-----VEN ZERO------\n");
        mt_set_gpio_out( GPIO_NFC_FIRM_PIN, GPIO_OUT_ONE );
      //printk( KERN_DEBUG "pn544-----FIRM ONE------\n");
        msleep( 50 );
        mt_set_gpio_out( GPIO_NFC_VEN_PIN, GPIO_OUT_ONE );
      //mt_set_gpio_out( GPIO_NFC_FIRM_PIN, GPIO_OUT_ZERO );
      //printk( KERN_DEBUG "pn544-----VEN ONE------\n");
        msleep(70);
        mt_set_gpio_out( GPIO_NFC_VEN_PIN, GPIO_OUT_ZERO );
      //mt_set_gpio_out( GPIO_NFC_FIRM_PIN, GPIO_OUT_ONE );
      //printk( KERN_DEBUG "pn544-----VEN ZERO------\n");
    //>2012/10/25-yaotsulin-15725.
        msleep(50);
      }
      else if( arg == 1 )
      {
      /* power on */
        printk( KERN_DEBUG "%s power on\n", __func__);

    //<2012/11/26-MaoyiChou. Removed VCAM_IO reason is not related.
      //hwPowerOn( MT65XX_POWER_LDO_VCAM_IO, VOL_1800, NFC_DEV_NAME );
    //>2012/11/26-MaoyiChou.
        mt_set_gpio_out( GPIO_NFC_FIRM_PIN, GPIO_OUT_ZERO );
        mt_set_gpio_out( GPIO_NFC_VEN_PIN, GPIO_OUT_ZERO );
    //<2012/10/25 yaotsulin-15725, Hawk NFC firmware update C3 1.26
        msleep( 50 );
    //>2012/10/25 yaotsulin-15725
      }
      else if( arg == 0 )
      {
      /* power off */
        printk( KERN_DEBUG "%s power off\n", __func__ );

    //<2012/11/26-MaoyiChou. Removed VCAM_IO reason is not related.
      //hwPowerDown( MT65XX_POWER_LDO_VCAM_IO, NFC_DEV_NAME );
    //>2012/11/26-MaoyiChou.
        mt_set_gpio_out( GPIO_NFC_FIRM_PIN, GPIO_OUT_ZERO );
        mt_set_gpio_out( GPIO_NFC_VEN_PIN, GPIO_OUT_ONE );
    //<2012/10/25-yaotsulin-15725. HAWK NFC firmware update C3 1.26
        msleep( 70 );
    //>2012/10/25 yaotsulin-15725
      }
      else
      {
        printk( KERN_DEBUG "%s bad arg %u\n", __func__, arg );
        return -EINVAL;
      }
    } break;

    default:
    {
      printk( KERN_DEBUG "%s bad ioctl %u\n", __func__, cmd );
      return -EINVAL;
    }
  }

  return 0;
} /* End.. pn544_dev_ioctl() */

/*****************************************************************************
**
******************************************************************************/
static const struct file_operations pn544_dev_fops = {
    .owner    = THIS_MODULE,
    .llseek   = no_llseek,
    .read     = pn544_dev_read,
    .write    = pn544_dev_write,
    .open     = pn544_dev_open,
    .unlocked_ioctl = pn544_dev_ioctl,
};

//<2012/08/16-yaotsulin-12963, Implement NFC SWP switch function for dual SIM
/*****************************************************************************
**
******************************************************************************/
static ssize_t attr_get_swp_sim(struct device *dev, struct device_attribute *attr, char *buf)
{
//int err;
u8  value;

    printk( KERN_DEBUG "SWP switch SIM function attr_get_swp_sim 'get_swp_sim'.\n");
  /************************************************
  ** [NFC] chage swp switch pin
  **===============================================
  ** 0: SIM1, 1: SIM2
  *************************************************/
    value = mt_get_gpio_out( GPIO_SIM_SWP_PIN );

  #if defined( HAWK40_EP1 ) || defined( HAWK40_EP2 )
    return sprintf( buf, "%x\n", !value );  /* Hawk40 EP1 */
  #elif defined( HAWK40_EP0 )
    return sprintf( buf, "%x\n", value );   /* Hawk40 EP0 */
  #else
    return sprintf( buf, "%x\n", value );
  #endif
} /* End.. attr_get_swp_sim() */

/*****************************************************************************
**
******************************************************************************/
static ssize_t attr_set_swp_sim(struct device *dev, struct device_attribute *attr, char *buf)
{
//int   err;
u8    set_value;
u8    value;

    printk( KERN_DEBUG "SWP switch SIM function attr_set_swp_sim 'set_swp_sim'.\n");

    if( buf[0] == '0' )
    {
    #if defined( HAWK40_EP1 ) || defined( HAWK40_EP2 )
      set_value = 1;  /* Hawk40 EP1 */
    #elif defined( HAWK40_EP0 )
      set_value = 0;  /* Hawk40 EP0 */
    #else
      set_value = 0;
    #endif
    }
    else if( buf[0] == '1' )
    {
    #if defined( HAWK40_EP1 ) || defined( HAWK40_EP2 )
      set_value = 0;  /* Hawk40 EP1 */
    #elif defined( HAWK40_EP0 )
      set_value = 1;  /* Hawk40 EP0 */
    #else
      set_value = 1;
    #endif
    }
    else
    {
      return sprintf( buf, "setting error\n");
    }

  /************************************************
  ** [NFC] chage swp switch pin
  **===============================================
  ** 0: SIM1, 1: SIM2
  *************************************************/
    value = mt_get_gpio_out( GPIO_SIM_SWP_PIN );
    if( set_value == value )
      return sprintf( buf, "setting non change\n");

    if( set_value )
    {
      mt_set_gpio_mode( GPIO_SIM_SWP_PIN, GPIO_SIM_SWP_PIN_M_GPIO);
      mt_set_gpio_dir( GPIO_SIM_SWP_PIN, GPIO_DIR_OUT );
      mt_set_gpio_out( GPIO_SIM_SWP_PIN, GPIO_OUT_ONE );

      printk( KERN_DEBUG "'set_swp_sim' SIM1.\n");
    }
    else
    {
      mt_set_gpio_mode( GPIO_SIM_SWP_PIN, GPIO_SIM_SWP_PIN_M_GPIO );
      mt_set_gpio_dir( GPIO_SIM_SWP_PIN, GPIO_DIR_OUT );
      mt_set_gpio_out( GPIO_SIM_SWP_PIN, GPIO_OUT_ZERO );

      printk( KERN_DEBUG "'set_swp_sim' SIM2.\n");
    }
    return sprintf( buf, "%x\n", set_value );
} /* End.. attr_set_swp_sim() */

//<2012/12/03-SunnyHu. Add for switch sim1/sim2 fail because permission.
/*****************************************************************************
**
******************************************************************************/
static ssize_t attr_set_swp_sim_0(struct device *dev, struct device_attribute *attr, char *buf)
{
//int err;
u8  set_value;
u8  value;

    printk( KERN_DEBUG "SWP switch SIM function attr_set_swp_sim_0 'set_swp_sim_0'.\n");

  #if defined( HAWK40_EP1 ) || defined( HAWK40_EP2 )
    set_value = 1;  /* Hawk40 EP1 */
  #elif defined( HAWK40_EP0 )
    set_value = 0;  /* Hawk40 EP0 */
  #else
    set_value = 0;
  #endif

  /************************************************
  ** [NFC] chage swp switch pin
  **===============================================
  ** 0: SIM1, 1: SIM2
  *************************************************/
    value = mt_get_gpio_out( GPIO_SIM_SWP_PIN );

    if( set_value == value )
      return sprintf( buf, "setting non change\n");

    if( set_value )
    {
      mt_set_gpio_mode( GPIO_SIM_SWP_PIN, GPIO_SIM_SWP_PIN_M_GPIO );
      mt_set_gpio_dir( GPIO_SIM_SWP_PIN, GPIO_DIR_OUT );
      mt_set_gpio_out( GPIO_SIM_SWP_PIN, GPIO_OUT_ONE );

      printk( KERN_DEBUG "'set_swp_sim_0' SIM1.\n");
    }
    else
    {
      mt_set_gpio_mode( GPIO_SIM_SWP_PIN, GPIO_SIM_SWP_PIN_M_GPIO );
      mt_set_gpio_dir( GPIO_SIM_SWP_PIN, GPIO_DIR_OUT );
      mt_set_gpio_out( GPIO_SIM_SWP_PIN, GPIO_OUT_ZERO );

      printk( KERN_DEBUG "'set_swp_sim_1' SIM2.\n");
    }
    return sprintf( buf, "%x\n", set_value );
} /* End.. attr_set_swp_sim_0() */

/*****************************************************************************
**
******************************************************************************/
static ssize_t attr_set_swp_sim_1(struct device *dev, struct device_attribute *attr, char *buf)
{
//int err;
u8  set_value;
u8  value;

    printk( KERN_DEBUG "SWP switch SIM function attr_set_swp_sim_1 'set_swp_sim_1'.\n");

  #if defined( HAWK40_EP1 ) || defined( HAWK40_EP2 )
    set_value = 0;  /* Hawk40 EP1 */
  #elif defined( HAWK40_EP0 )
    set_value = 1;  /* Hawk40 EP0 */
  #else
    set_value = 1;
  #endif

  /************************************************
  ** [NFC] chage swp switch pin
  **===============================================
  ** 0: SIM1, 1: SIM2
  *************************************************/
    value = mt_get_gpio_out( GPIO_SIM_SWP_PIN );

    if( set_value == value )
      return sprintf( buf, "setting non change\n");

    if( set_value )
    {
      mt_set_gpio_mode( GPIO_SIM_SWP_PIN, GPIO_SIM_SWP_PIN_M_GPIO );
      mt_set_gpio_dir( GPIO_SIM_SWP_PIN, GPIO_DIR_OUT );
      mt_set_gpio_out( GPIO_SIM_SWP_PIN, GPIO_OUT_ONE );

      printk( KERN_DEBUG "'set_swp_sim_1' SIM1.\n");
    }
    else
    {
      mt_set_gpio_mode( GPIO_SIM_SWP_PIN, GPIO_SIM_SWP_PIN_M_GPIO );
      mt_set_gpio_dir( GPIO_SIM_SWP_PIN, GPIO_DIR_OUT );
      mt_set_gpio_out( GPIO_SIM_SWP_PIN, GPIO_OUT_ZERO );

      printk( KERN_DEBUG "'set_swp_sim_1' SIM2.\n");
    }
    return sprintf(buf, "%x\n", set_value);
} /* End.. attr_set_swp_sim_1() */
//>2012/12/03-SunnyHu.

static DEVICE_ATTR( get_swp_sim, S_IRUGO, attr_get_swp_sim, NULL );
//<2012/11/15-16993-xiaoleiding. [8317][CQ][BU2SC00138577][CTS]fix android.permission.cts.FileSystemPermissionTest#testAllFilesInSysAreNotWritable test failed
//static DEVICE_ATTR( set_swp_sim, 0666, attr_set_swp_sim, attr_set_swp_sim );
static DEVICE_ATTR( set_swp_sim, 0664, attr_set_swp_sim, attr_set_swp_sim );
//>2012/11/15-16993-xiaoleiding.

//<2012/12/03-SunnyHu. Add for switch sim1/sim2 fail because permission.
// use read function to set sim1/sim2
static DEVICE_ATTR( set_swp_sim_0, S_IRUGO, attr_set_swp_sim_0, NULL );
static DEVICE_ATTR( set_swp_sim_1, S_IRUGO, attr_set_swp_sim_1, NULL );
//>2012/12/03-SunnyHu.

//static struct device_attribute attributes[] = {
//  __ATTR(get_swp_sim, S_IRUGO, attr_get_swp_sim, NULL),
//  __ATTR(set_swp_sim, S_IRUGO, attr_set_swp_sim, NULL),
//};

/*****************************************************************************
**
******************************************************************************/
static struct kobject *android_nfc_kobj;

static int create_sysfs_interfaces(void)
{
int ret;

    android_nfc_kobj = kobject_create_and_add( "android_nfc", NULL );
    if( android_nfc_kobj == NULL )
    {
      printk( KERN_DEBUG "Subsystem register failed.\n");
      ret = -ENOMEM;
      return  ret;
    }

    ret = sysfs_create_file( android_nfc_kobj, &dev_attr_get_swp_sim.attr );
    if( ret )
    {
      printk( KERN_DEBUG "sysfs create file failed 1.\n");
      return  ret;
    }

    ret = sysfs_create_file( android_nfc_kobj, &dev_attr_set_swp_sim.attr );
    if( ret )
    {
      printk( KERN_DEBUG "sysfs create file failed 2.\n");
      return  ret;
    }

//<2012/12/03-SunnyHu. Add for switch sim1/sim2 fail because permission.
    ret = sysfs_create_file( android_nfc_kobj, &dev_attr_set_swp_sim_0.attr );
    if( ret )
    {
      printk( KERN_DEBUG "sysfs create file failed 3.\n");
      return  ret;
    }

    ret = sysfs_create_file( android_nfc_kobj, &dev_attr_set_swp_sim_1.attr );
    if( ret )
    {
      printk( KERN_DEBUG "sysfs create file failed 4.\n");
      return  ret;
    }
//>2012/12/03-SunnyHu.
    return  0;
/*
  int i;
  for (i = 0; i < ARRAY_SIZE(attributes); i++)
    if (device_create_file(dev, attributes + i))
        goto error;
    return 0;

error:
  for ( ; i >= 0; i--)
    device_remove_file(dev, attributes + i);
  dev_err(dev, "%s:Unable to create interface\n", __func__);
*/
} /* End.. create_sysfs_interfaces() */

/*****************************************************************************
**
******************************************************************************/
static int remove_sysfs_interfaces(void)
{
    sysfs_remove_file( android_nfc_kobj, &dev_attr_set_swp_sim.attr );
    sysfs_remove_file( android_nfc_kobj, &dev_attr_get_swp_sim.attr );
//<2012/12/03-SunnyHu. Add for switch sim1/sim2 fail because permission.
    sysfs_remove_file( android_nfc_kobj, &dev_attr_set_swp_sim_0.attr );
    sysfs_remove_file( android_nfc_kobj, &dev_attr_set_swp_sim_1.attr );
//>2012/12/03-SunnyHu.
    kobject_del( android_nfc_kobj );

/*
  int i;
  for (i = 0; i < ARRAY_SIZE(attributes); i++)
    device_remove_file(dev, attributes + i);
  return 0;
*/
} /* End.. remove_sysfs_interfaces() */
//>2012/08/16-yaotsulin-12963

/*****************************************************************************
**
******************************************************************************/
static int pn544_probe(struct i2c_client *client,
    const struct i2c_device_id *id)
{
struct pn544_i2c_platform_data  * platform_data = NULL;
PN544_DEV  * pn544_dev = NULL;
int   ret = 0;

#if defined( I2C_DMA_USAGE )
    client->addr     &= I2C_MASK_FLAG;
  //client->addr     |= I2C_DMA_FLAG;
    client->ext_flag |= I2C_DMA_FLAG;
  //client->ext_flag |= I2C_DIRECTION_FLAG;
    client->ext_flag |= I2C_A_FILTER_MSG;
#endif
  //client->timing    = 400; /* 400 KHz */

    platform_data = client->dev.platform_data;
    if( platform_data == NULL )
    {
      pr_err("%s : nfc probe fail\n", __func__ );
      return  -ENODEV;
    }

    printk("nfc probe step01 is ok\n");

    if( !i2c_check_functionality( client->adapter, I2C_FUNC_I2C ))
    {
      pr_err("%s : need I2C_FUNC_I2C\n", __func__ );
      return  -ENODEV;
    }

    printk("nfc probe step02 is ok\n");

  //ret = gpio_request( platform_data->irq_gpio, "nfc_int" );
  //if( ret )
  //  return  -ENODEV;
  //ret = gpio_request( platform_data->ven_gpio, "nfc_ven" );
  //if( ret )
  //  goto err_ven;
  //ret = gpio_request( platform_data->firm_gpio, "nfc_firm" );
  //if( ret )
  //  goto err_firm;

    printk("nfc probe step03 is ok\n");

    pn544_dev = kzalloc( sizeof( *pn544_dev ), GFP_KERNEL );
    if( pn544_dev == NULL )
    {
      dev_err(  &client->dev, "failed to allocate memory for module data\n");
      ret = -ENOMEM;
      goto err_exit;
    }

    printk( KERN_DEBUG "nfc probe step04 is ok\n");

    pn544_dev->irq_gpio   = platform_data->irq_gpio;
    pn544_dev->ven_gpio   = platform_data->ven_gpio;
    pn544_dev->firm_gpio  = platform_data->firm_gpio;
    pn544_dev->client     = client;

  /* init mutex and queues */
    init_waitqueue_head( &pn544_dev->read_wq );
    mutex_init( &pn544_dev->read_mutex );
    spin_lock_init( &pn544_dev->irq_enabled_lock );

    pn544_dev->pn544_device.minor = MISC_DYNAMIC_MINOR;
    pn544_dev->pn544_device.name  = NFC_DEV_NAME;
    pn544_dev->pn544_device.fops  = &pn544_dev_fops;

    ret = misc_register( &pn544_dev->pn544_device );
    if( ret )
    {
      pr_err("%s : misc_register failed\n", __FILE__ );
      goto  err_misc_register;
    }
    printk("nfc probe step05 is ok\n");

  /************************************************************************
  ** request irq.  the irq is set whenever the chip has data available
  ** for reading.  it is cleared when all data has been read.
  *************************************************************************/
  //hwPowerOn( MT65XX_POWER_LDO_VCAM_IO, VOL_1800, NFC_DEV_NAME );
#if 0 //defined(HAWK40_EP0) || defined(HAWK40_EP1) || defined( HAWK40_EP2 )
/* SIM default set SIM2 */
    mt_set_gpio_mode( GPIO_SIM_SWP_PIN, GPIO_SIM_SWP_PIN_M_GPIO );
    mt_set_gpio_dir( GPIO_SIM_SWP_PIN, GPIO_DIR_OUT );
    mt_set_gpio_out( GPIO_SIM_SWP_PIN, GPIO_OUT_ZERO );
#endif
    mt_set_gpio_mode( GPIO106, GPIO_MODE_00 );
    mt_set_gpio_dir( GPIO106, GPIO_DIR_OUT );
    mt_set_gpio_out( GPIO106, GPIO_OUT_ONE );

    mt_set_gpio_mode( GPIO_NFC_VEN_PIN, GPIO_MODE_00 );
    mt_set_gpio_dir( GPIO_NFC_VEN_PIN, GPIO_DIR_OUT );
    mt_set_gpio_out( GPIO_NFC_VEN_PIN, GPIO_OUT_ZERO );

    mt_set_gpio_mode( GPIO_NFC_FIRM_PIN, GPIO_MODE_00 );
    mt_set_gpio_dir( GPIO_NFC_FIRM_PIN, GPIO_DIR_OUT );
    mt_set_gpio_out( GPIO_NFC_FIRM_PIN, GPIO_OUT_ZERO );

    pr_info("%s : requesting IRQ %d\n", __func__, client->irq );

    pn544_dev->irq_enabled = true;
    g_pn544_dev = pn544_dev;

    mt_set_gpio_mode( GPIO_NFC_EINT_PIN, GPIO_NFC_EINT_PIN_M_EINT );
    mt_set_gpio_dir( GPIO_NFC_EINT_PIN, GPIO_DIR_IN );
    mt_set_gpio_pull_enable( GPIO_NFC_EINT_PIN, GPIO_PULL_ENABLE );
    mt_set_gpio_pull_select( GPIO_NFC_EINT_PIN, GPIO_PULL_UP );

//<2014/05/06-Yuting Shih. Modified for Android KK
#if 1 //defined( CUST_EINT_ALS_TYPE )
    mt_eint_set_sens( CUST_EINT_NFC_NUM, CUST_EINT_NFC_SENSITIVE );
    mt_eint_set_polarity( CUST_EINT_NFC_NUM, CUST_EINT_NFC_POLARITY );
    mt_eint_set_hw_debounce( CUST_EINT_NFC_NUM, CUST_EINT_NFC_DEBOUNCE_CN );
  //mt_eint_registration( CUST_EINT_NFC_NUM, CUST_EINT_ALS_TYPE, pn544_dev_irq_handler, 0 );
    mt65xx_eint_registration( CUST_EINT_NFC_NUM, CUST_EINT_NFC_DEBOUNCE_EN, CUST_EINT_NFC_POLARITY, pn544_dev_irq_handler, 0 );
    mt_eint_unmask( CUST_EINT_NFC_NUM );
#else
    mt65xx_eint_set_sens( CUST_EINT_NFC_NUM, CUST_EINT_NFC_SENSITIVE );
    mt65xx_eint_set_polarity( CUST_EINT_NFC_NUM, CUST_EINT_NFC_POLARITY );
    mt65xx_eint_set_hw_debounce( CUST_EINT_NFC_NUM, CUST_EINT_NFC_DEBOUNCE_CN );
    mt65xx_eint_registration( CUST_EINT_NFC_NUM, CUST_EINT_NFC_DEBOUNCE_EN, CUST_EINT_NFC_POLARITY, pn544_dev_irq_handler, 0 );
    mt65xx_eint_unmask( CUST_EINT_NFC_NUM );
#endif
//>2014/05/06-Yuting Shih.

  //ret = request_irq( client->irq, pn544_dev_irq_handler,
  //            IRQF_TRIGGER_HIGH, client->name, pn544_dev);
  //if( ret )
  //{
  //  printk( &client->dev, "request_irq failed\n");
  //  goto err_request_irq_failed;
  //}
    printk("nfc probe step06 is ok\n");

  //pn544_disable_irq( pn544_dev );
//<2014/05/06-Yuting Shih. Modified for Android KK.
  #if 1 //defined( CUST_EINT_ALS_TYPE )
    mt_eint_mask( CUST_EINT_NFC_NUM );
  #else
    mt65xx_eint_mask( CUST_EINT_NFC_NUM );
   #endif
//>2014/05/06-Yuting Shih.

    i2c_set_clientdata( client, pn544_dev );

    printk("nfc probe step07 is ok\n");

//<2012/08/16-yaotsulin-12963. Implement NFC SWP switch function for dual SIM
    ret = create_sysfs_interfaces();
    if( ret < 0 )
    {
      printk( KERN_DEBUG "sysfs create error.\n");
      goto err_sysfs;
    }
    printk( "nfc probe step08 is ok\n");
//>2012/08/16-yaotsulin-12963.

    return 0;

err_request_irq_failed:
    misc_deregister( &pn544_dev->pn544_device );
err_misc_register:
    mutex_destroy( &pn544_dev->read_mutex );
    kfree( pn544_dev );
//<2012/08/16 yaotsulin-12963, Implement NFC SWP switch function for dual SIM
err_sysfs:
    remove_sysfs_interfaces();
//>2012/08/16 yaotsulin-12963
err_exit:
  //gpio_free( platform_data->firm_gpio );
    mt_set_gpio_out( GPIO_NFC_VEN_PIN, GPIO_OUT_ZERO );
    mt_set_gpio_out( GPIO_NFC_FIRM_PIN, GPIO_OUT_ZERO );
    mt_set_gpio_out( GPIO106, GPIO_OUT_ONE );

    return  ret;
} /* End.. pn544_probe() */

/*****************************************************************************
**
******************************************************************************/
static int pn544_remove(struct i2c_client *client)
{
PN544_DEV * pn544_dev;

    pn544_dev = i2c_get_clientdata( client );

  //free_irq( client->irq, pn544_dev );
    misc_deregister( &pn544_dev->pn544_device );
    mutex_destroy( &pn544_dev->read_mutex );
    gpio_free( pn544_dev->irq_gpio );
    gpio_free( pn544_dev->ven_gpio );
    gpio_free( pn544_dev->firm_gpio );

  //hwPowerDown( MT65XX_POWER_LDO_VCAM_IO, NFC_DEV_NAME );

    kfree( pn544_dev );

    return  0;
} /* End.. pn544_remove() */

/*****************************************************************************
**
******************************************************************************/
static const struct i2c_device_id pn544_id[] =
{
    { I2C_ID_NAME, 0 },
    { }
};

static struct i2c_driver pn544_driver = {
    .id_table = pn544_id,
    .probe    = pn544_probe,
    .remove   = pn544_remove,
    .driver   = {
      .owner  = THIS_MODULE,
      .name   = NFC_DEV_NAME,
    },
};

/*
 * module load/unload record keeping
 */
/*****************************************************************************
**
******************************************************************************/
static int __init pn544_dev_init(void)
{
    pr_info( "Loading pn544 driver\n" );

    return  i2c_add_driver( &pn544_driver );
}
module_init( pn544_dev_init );

/*****************************************************************************
**
******************************************************************************/
static void __exit pn544_dev_exit(void)
{
    pr_info("Unloading pn544 driver\n");
    i2c_del_driver( &pn544_driver );
}
module_exit( pn544_dev_exit );

MODULE_AUTHOR("Sylvain Fonteneau");
MODULE_DESCRIPTION("NFC PN544 driver");
MODULE_LICENSE("GPL");
