/*****************************************************************************
 Copyright(c) 2010 NMI Inc. All Rights Reserved
 
 File name : nmi_hw.h
 
 Description : NM625 host interface
 
 History : 
 ----------------------------------------------------------------------
 2010/05/17 	ssw		initial
*******************************************************************************/

#ifndef __NMI_HW_H__
#define __NMI_HW_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/ioctl.h>

#define SUPPORT_GPIO_INTERRUPT // johnny 1215 interrupt
//#define NMI326_HW_CHIP_ID_CHECK
//#define NM_DEBUG_ON

#ifdef SUPPORT_GPIO_INTERRUPT

#include <linux/fs.h>

//#define SUPPORT_SIGIO
 #ifndef SUPPORT_SIGIO
#define SUPPORT_POLL
 #endif

 #ifdef SUPPORT_POLL
 #include <linux/poll.h>  
 #include <linux/sched.h>
 #endif

#endif
#define NMI326_HW_CHIP_ID_CHECK 1

#define MAX_OPEN_NUM 		8

#define IOCTL_MAGIC	't'
#define NMI326_IOCTL_BUF_SIZE   256

typedef struct {
	unsigned char dev_addr;
	unsigned short size;
	unsigned char buf[NMI326_IOCTL_BUF_SIZE];
} ioctl_info;

#define IOCTL_MAXNR			26	//johnny

#define IOCTL_DMB_RESET			_IOW( IOCTL_MAGIC, 0, ioctl_info)
#define IOCTL_DMB_PROBE			_IO( IOCTL_MAGIC, 1 )
#define IOCTL_DMB_INIT		 	_IO( IOCTL_MAGIC, 2 )
#define IOCTL_DMB_DEINIT	 	_IO( IOCTL_MAGIC, 3 )

#define IOCTL_DMB_BYTE_READ 		_IOWR( IOCTL_MAGIC, 4, ioctl_info )
#define IOCTL_DMB_WORD_READ 		_IOWR( IOCTL_MAGIC, 5, ioctl_info )
#define IOCTL_DMB_LONG_READ 		_IOWR( IOCTL_MAGIC, 6, ioctl_info )
#define IOCTL_DMB_BULK_READ 		_IOWR( IOCTL_MAGIC, 7, ioctl_info )

#define IOCTL_DMB_BYTE_WRITE 		_IOW( IOCTL_MAGIC, 8, ioctl_info )
#define IOCTL_DMB_WORD_WRITE 		_IOW( IOCTL_MAGIC, 9, ioctl_info )
#define IOCTL_DMB_LONG_WRITE 		_IOW( IOCTL_MAGIC, 10, ioctl_info )
#define IOCTL_DMB_BULK_WRITE 		_IOW( IOCTL_MAGIC, 11, ioctl_info )

#define IOCTL_DMB_TUNER_READ	 	_IOWR( IOCTL_MAGIC, 12, ioctl_info )
#define IOCTL_DMB_TUNER_WRITE	 	_IOW( IOCTL_MAGIC, 13, ioctl_info )

#define IOCTL_DMB_TUNER_SET_FREQ 	_IOW( IOCTL_MAGIC, 14, ioctl_info )
#define IOCTL_DMB_TUNER_SELECT	 	_IOW( IOCTL_MAGIC, 15, ioctl_info )
#define IOCTL_DMB_TUNER_DESELECT 	_IO( IOCTL_MAGIC, 16 )
#define IOCTL_DMB_TUNER_GET_RSSI 	_IOWR( IOCTL_MAGIC, 17, ioctl_info )

#define IOCTL_DMB_HOSTIF_SELECT 	_IOW( IOCTL_MAGIC, 18, ioctl_info )
#define IOCTL_DMB_HOSTIF_DESELECT 	_IO( IOCTL_MAGIC, 19 )

#define IOCTL_DMB_POWER_ON		_IO( IOCTL_MAGIC, 20 )
#define IOCTL_DMB_POWER_OFF		_IO( IOCTL_MAGIC, 21 )

#define IOCTL_DMB_INTERRUPT_DONE _IO( IOCTL_MAGIC, 22 ) //johnny 1215
#define IOCTL_DMB_INTERRUPT_REGISTER _IO( IOCTL_MAGIC, 23 ) //bonnie
#define IOCTL_DMB_INTERRUPT_ENABLE _IO( IOCTL_MAGIC, 24 )//bonnie
#define IOCTL_DMB_INTERRUPT_DISABLE _IO( IOCTL_MAGIC, 25 )//bonnie

// gpio base address
#define PIO_BASE_ADDRESS	(0x01c20800)
#define PIO_RANGE_SIZE		(0x400)
#define GPIO_ENABLE
#define SYSCONFIG_GPIO_ENABLE

#define PIO_INT_STAT_OFFSET          (0x214)
#define PIO_INT_CTRL_OFFSET          (0x210)

#define PIO_PN_DAT_OFFSET(n)         ((n)*0x24 + 0x10) 
//#define PIOI_DATA                    (0x130)
#define PIOH_DATA                    (0x10c)
#define PIOI_CFG3_OFFSET             (0x12c)

#define PRESS_DOWN	(1)
#define FREE_UP		(0)

#define IRQ_EINT0	(0)
#define IRQ_EINT1	(1)
#define IRQ_EINT2	(2)
#define IRQ_EINT3	(3)
#define IRQ_EINT4	(4)
#define IRQ_EINT5	(5)
#define IRQ_EINT6	(6)
#define IRQ_EINT7	(7)
#define IRQ_EINT8	(8)
#define IRQ_EINT9	(9)
#define IRQ_EINT10	(10)
#define IRQ_EINT11	(11)
#define IRQ_EINT12	(12)
#define IRQ_EINT13	(13)
#define IRQ_EINT14	(14)
#define IRQ_EINT15	(15)
#define IRQ_EINT16	(16)
#define IRQ_EINT17	(17)
#define IRQ_EINT18	(18)
#define IRQ_EINT19	(19)
#define IRQ_EINT20	(20)
#define IRQ_EINT21	(21)
#define IRQ_EINT22	(22)
#define IRQ_EINT23	(23)
#define IRQ_EINT24	(24)
#define IRQ_EINT25	(25)
#define IRQ_EINT26	(26)
#define IRQ_EINT27	(27)
#define IRQ_EINT28	(28)
#define IRQ_EINT29	(29)
#define IRQ_EINT30	(30)
#define IRQ_EINT31	(31)

#define PIO_INT_CFG0_OFFSET 	(0x200)
#define PIO_INT_CFG1_OFFSET 	(0x204)
#define PIO_INT_CFG2_OFFSET 	(0x208)
#define PIO_INT_CFG3_OFFSET 	(0x20c)

/*
typedef enum{
     PIO_INT_CFG0_OFFSET = 0x200,
     PIO_INT_CFG1_OFFSET = 0x204,
     PIO_INT_CFG2_OFFSET = 0x208,
     PIO_INT_CFG3_OFFSET = 0x20c
} int_cfg_offset;
*/

typedef enum{
	POSITIVE_EDGE = 0x0,
	NEGATIVE_EDGE = 0x1,
	HIGH_LEVEL = 0x2,
	LOW_LEVEL = 0x3,
	DOUBLE_EDGE = 0x4
} ext_int_mode;

#ifdef __cplusplus
}
#endif

#endif // __NMI_HW_H__

