//header: gpio_api.h
#ifndef __GPIO_API_H__
#define __GPIO_API_H__

#if defined(__cplusplus) 
extern "C" 
{ 
#endif

#include <linux/ioctl.h>

typedef enum __CTL_RESET__
{
	CTL_RESET_BCM53242_BIT 	= 0x00,
	CTL_RESET_BCM5482_BIT 	= 0x01,
	CTL_RESET_FPGA_BIT		= 0x05,
	CTL_RESET_BUFF,
}CTL_RESET;

typedef enum __CTL_WORKMODE__
{
	CTL_WORKMODE_BIT 	= 0x00,
	CTL_WORKMODE_BUFF,
}CTL_WORKMODE;

typedef enum __CTL_POWER__
{
	CTL_POWER_WIRE_BIT 		= 0x00,
	CTL_POWER_RADIO_BIT 		= 0x01,
	CTL_POWER_RAY_BIT		= 0x02,
	CTL_POWER_WARNET_BIT	= 0x03,
	CTL_POWER_DINET_BIT 		= 0x04,
	
	CTL_POWER_BUFF,
}CTL_POWER;


typedef enum __CTL_HOOK__
{
	CTL_HOOK_PTT_BIT = 0x00,	

	CTL_HOOK_TRUNK0_BIT = 0x01,
	CTL_HOOK_TRUNK1_BIT ,	
	
	CTL_HOOK_PHONE0_BIT = 0x03,
	CTL_HOOK_PHONE1_BIT ,
	CTL_HOOK_PHONE2_BIT ,
	CTL_HOOK_PHONE3_BIT ,	
	CTL_HOOK_PHONE4_BIT ,	
	CTL_HOOK_PHONE5_BIT ,	
	CTL_HOOK_PHONE6_BIT ,	
	CTL_HOOK_PHONE7_BIT ,	
	CTL_HOOK_PHONE8_BIT ,	
	CTL_HOOK_PHONE9_BIT ,	
	CTL_HOOK_PHONE10_BIT ,	
	CTL_HOOK_PHONE11_BIT ,	

	CTL_HOOK_BUFF,
}CTL_HOOK;

typedef enum __CTL_SLOT__
{
	CTL_SLOT_QINWU_BIT  = 0x00,

	CTL_SLOT_TRUNK0_BIT = 0x01,
	CTL_SLOT_TRUNK1_BIT ,	
	
	CTL_SLOT_PHONE0_BIT = 0x03,
	CTL_SLOT_PHONE1_BIT ,	
	CTL_SLOT_PHONE2_BIT ,	
	CTL_SLOT_PHONE3_BIT ,		
	CTL_SLOT_PHONE4_BIT ,	
	CTL_SLOT_PHONE5_BIT ,	
	CTL_SLOT_PHONE6_BIT ,	
	CTL_SLOT_PHONE7_BIT ,		
	CTL_SLOT_PHONE8_BIT ,	
	CTL_SLOT_PHONE9_BIT ,	
	CTL_SLOT_PHONE10_BIT ,	
	CTL_SLOT_PHONE11_BIT ,	

	CTL_SLOT_BUFF,
}CTL_SLOT;

typedef enum __CTL_UART__
{
	CTL_UART0_VAL		= 0x00,
	CTL_UART1_VAL 		= 0x01,
	CTL_UART2_VAL 		= 0x02,
	CTL_UART3_VAL 		= 0x03,	
	CTL_UART4_VAL 		= 0x04,
	CTL_UART5_VAL 		= 0x05,
	CTL_UART6_VAL 		= 0x06,
	CTL_UART7_VAL 		= 0x07,	
	CTL_UART8_VAL 		= 0x08,
	CTL_UART9_VAL 		= 0x09,		
	
	CTL_UART_BUFF,
}CTL_UART;

typedef enum __CTL_RING__
{
	CTL_RING_RESERVE_BIT  = 0x00,
	
	CTL_RING_TRUNK0_BIT 	= 0x01,
	CTL_RING_TRUNK1_BIT ,	
	
	CTL_RING_PHONE0_BIT 	= 0x03,
	CTL_RING_PHONE1_BIT ,	
	CTL_RING_PHONE2_BIT ,	
	CTL_RING_PHONE3_BIT ,		
	CTL_RING_PHONE4_BIT ,	
	CTL_RING_PHONE5_BIT ,	
	CTL_RING_PHONE6_BIT ,	
	CTL_RING_PHONE7_BIT ,	
	CTL_RING_PHONE8_BIT ,	
	CTL_RING_PHONE9_BIT ,	
	CTL_RING_PHONE10_BIT ,	
	CTL_RING_PHONE11_BIT ,	
	
	CTL_RING_BUFF,
}CTL_RING;

typedef enum __CTL_MODE__
{
	CTL_MODE_AC491CLK_BIT 		= 0x00,
	CTL_MODE_QINWU_ON_BIT 		= 0x01,
	CTL_MODE_QINWU_TEST_BIT 	= 0x02,
	CTL_MODE_QINWU_LOOP_BIT 	= 0x03,
	
	CTL_MODE_BUFF,
}CTL_MODE;

typedef enum __CTL_GINTF__
{
	CTL_GINTF_KUOZHAN_2M = 0x00,
	CTL_GINTF_KUOZHAN_1M = 0x01,
	CTL_GINTF_KUOZHAN_128K = 0x02,
	CTL_GINTF_DULIKAISHE = 0x03,
	
	CTL_GINTF_WORKMODE_BUTT
}CTL_GINTF_VAL;

typedef enum __CTL_GAIN_VAL__
{
	CTL_GAIN_PAOFANG = 0x00,
	CTL_GAIN_ZHUANGJIA = 0x01,
	CTL_GAIN_ADAPTER = 0x02,
	
	CTL_GAIN_BUTT
}CTL_GAIN_VAL;

typedef enum __CTL_GAIN__
{
	CTL_GAIN_MIN24DB = 0x00,
	CTL_GAIN_MIN21DB = 0x01,
	CTL_GAIN_MIN18DB = 0x02,
	CTL_GAIN_MIN15DB = 0x03,
	CTL_GAIN_MIN12DB = 0x04,
	CTL_GAIN_MIN9DB = 0x05,
	CTL_GAIN_MIN6DB = 0x06,
	CTL_GAIN_MIN3DB = 0x07,
	CTL_GAIN_0DB = 0x08,
	CTL_GAIN_PLUS3DB = 0x09,
	CTL_GAIN_PLUS6DB = 0x0A,
	CTL_GAIN_PLUS9DB = 0x0B,
	CTL_GAIN_PLUS12DB = 0x0C,
	CTL_GAIN_PLUS15DB = 0x0D,
	CTL_GAIN_PLUS18DB = 0x0E,
	
	CTL_GAIN_SET_BUTT
}CTL_GAIN_VAL_SET;

struct switch_struct {
	unsigned int page;
	unsigned int reg;
	unsigned int val; 
	unsigned short  regval[4];
};

/* 上下电控制*/
typedef struct pm_PowerCtrl{
   unsigned int pmCmdId;//取值:EN_PM_CTRL
   unsigned int OffOnFlg;//状态取值:FPGA_PM_ON, FPGA_PM_OFF
}ST_PM_POWERCTRL;


#define LKDEV_IOCTL_BASE 				'm'
#define LKDEV_IOR(nr, size) 				_IOR(LKDEV_IOCTL_BASE, nr, size)
#define LKDEV_IOW(nr, size) 				_IOW(LKDEV_IOCTL_BASE, nr, size)

#define IOCTL_CMD_RESET_GET 			LKDEV_IOR(0x00,	int)
#define IOCTL_CMD_RESET_SET 			LKDEV_IOW(0x01,int)
#define IOCTL_CMD_POWER_GET 			LKDEV_IOR(0x02, int)
#define IOCTL_CMD_POWER_SET			LKDEV_IOW(0x03,int)
#define IOCTL_CMD_HOOK_GET			LKDEV_IOR(0x04, int)
#define IOCTL_CMD_HOOK_SET 			LKDEV_IOW(0x05,int)
#define IOCTL_CMD_SLOT_GET 			LKDEV_IOR(0x06, int)
#define IOCTL_CMD_SLOT_SET 			LKDEV_IOW(0x07,int)
#define IOCTL_CMD_UART_GET 			LKDEV_IOR(0x08, int)
#define IOCTL_CMD_UART_SET 			LKDEV_IOW(0x09,int)
#define IOCTL_CMD_RING_GET 			LKDEV_IOR(0x0A, int)
#define IOCTL_CMD_RING_SET 			LKDEV_IOW(0x0B,int)
#define IOCTL_CMD_MODE_GET 			LKDEV_IOR(0x0C, int)
#define IOCTL_CMD_MODE_SET 			LKDEV_IOW(0x0D,int)
#define IOCTL_CMD_VER_GET 				LKDEV_IOR(0x0E, int)
#define IOCTL_CMD_VER_SET 				LKDEV_IOW(0x0F,int)
#define IOCTL_CMD_SWITCH_GET 			LKDEV_IOR(0x10, int)
#define IOCTL_CMD_SWITCH_SET 			LKDEV_IOW(0x11,int)
#define IOCTL_CMD_SWITCH_VLAN_GET 	LKDEV_IOR(0x12, int)
#define IOCTL_CMD_SWITCH_VLAN_SET 	LKDEV_IOW(0x13,int)
#define IOCTL_CMD_WORKMODE_GET 		LKDEV_IOR(0x14, int)
#define IOCTL_CMD_WORKMODE_SET 		LKDEV_IOW(0x15,int)
#define IOCTL_CMD_GINTF_GET 			LKDEV_IOR(0x16,int)
#define IOCTL_CMD_GINTF_SET 			LKDEV_IOW(0x17,int)
#define IOCTL_CMD_GAIN_SET 			LKDEV_IOW(0x18,int)
#define IOCTL_CMD_QINWU_GAIN_SET 	LKDEV_IOW(0x19,int)



extern int gpio_fd;
extern uint8 pm_cfg[CTL_POWER_BUFF];
extern int32 g_pm_val;
extern const char *pm_str[CTL_POWER_BUFF];

extern int Gpio_init(void);
extern int Gpio_close(void);
extern int Gpio_Reset_Get(int chan);
extern int Gpio_Reset_Set(int chan, int status);
extern int Gpio_Power_Get(int chan);
extern int Gpio_Power_Set(int chan, int status);
extern int Gpio_Hook_Get(int chan);
extern int Gpio_Slot_Get(int chan);
extern int Gpio_Slot_Set(int chan, int status);
extern int Gpio_Uart_Get(int chan);
extern int Gpio_Uart_Set(int chan, int status);
extern int Gpio_Ring_Get(int chan);
extern int Gpio_Ring_Set(int chan, int status);
extern int Gpio_Trunk_Get(int chan);
extern int Gpio_Trunk_Set(int chan, int status);
extern int Gpio_Mode_Get(int chan);
extern int Gpio_Mode_Set(int chan, int status);
extern int Gpio_Ver_Get(int chan, int status);
extern int switch_vlan_set(unsigned int reg, unsigned int val);
extern int switch_vlan_get(unsigned int reg);
extern int main_vlan_init(void);
extern int Gpio_pm_setting(int32 val);
extern int Gpio_WorkMode_Get(int chan);
extern int Gpio_WorkMode_Set(int chan, int status);
extern int Gport_Speed_Set(int speed);
extern int Gport_Speed_Get(int speed);

extern unsigned short switch_Reg_Read(unsigned int page, unsigned int reg);



#if defined(__cplusplus) 
} 
#endif 

#endif // __GPIO_API_H__

