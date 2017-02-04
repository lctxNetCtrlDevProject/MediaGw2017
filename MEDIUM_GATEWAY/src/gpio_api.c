//header: gpio_api.c
#ifndef __GPIO_API_C__
#define __GPIO_API_C__

#if defined(__cplusplus) 
extern "C" 
{ 
#endif

#include "PUBLIC.h"
//#include <math.h>


#define GPIO_DEV_NAME "/dev/gpio_drv"



struct vlan_config 
{
	int vlan_id;
	int vlan_val;
};


#define MAIN_VLAN_MAX 		6
#define MAIN_VLAN_PORT 	27


struct vlan_config vlan_cfg[MAIN_VLAN_MAX];

int vlan_reg_val[MAIN_VLAN_PORT] = {0};

int gpio_fd = 0;

uint8 pm_cfg[CTL_POWER_BUFF] = {0};
int32 g_pm_val = 0;

const char *pm_str[CTL_POWER_BUFF] = 
{
	"CP_WIRE_PM",
	"CP_RADIO_PM",		
	"CP_RAY_PM",
	"CP_WARNET_PM",	
	"CP_DINET_PM",	
};

int Gpio_Reset_Get(int chan)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((chan >= CTL_RESET_BCM53242_BIT)&&(chan < CTL_RESET_BUFF))
	{
		val[0] = chan;
		ret = ioctl(gpio_fd, IOCTL_CMD_RESET_GET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return val[0];	
}

int Gpio_Reset_Set(int chan, int status)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((chan >= CTL_RESET_BCM53242_BIT)&&(chan < CTL_RESET_BUFF))
	{
		val[0] = chan;
		val[1] = status;
		ret = ioctl(gpio_fd, IOCTL_CMD_RESET_SET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return ret;
}

int Gpio_WorkMode_Get(int chan)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((chan >= CTL_WORKMODE_BIT)&&(chan < CTL_WORKMODE_BUFF))
	{
		val[0] = chan;
		ret = ioctl(gpio_fd, IOCTL_CMD_WORKMODE_GET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return val[0];	
}

int Gpio_WorkMode_Set(int chan, int status)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((chan >= CTL_WORKMODE_BIT)&&(chan < CTL_WORKMODE_BUFF))
	{
		val[0] = chan;
		val[1] = status;
		ret = ioctl(gpio_fd, IOCTL_CMD_WORKMODE_SET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return ret;
}


int Gpio_Power_Get(int chan)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((chan >= CTL_POWER_WIRE_BIT)&&(chan < CTL_POWER_BUFF))
	{
		val[0] = chan;
		ret = ioctl(gpio_fd, IOCTL_CMD_POWER_GET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return val[0];	
}

/* status 0: off, 1: on */
int Gpio_Power_Set(int chan, int status)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((chan >= CTL_POWER_WIRE_BIT)&&(chan < CTL_POWER_BUFF))
	{
		val[0] = chan;
		val[1] = status;
		ret = ioctl(gpio_fd, IOCTL_CMD_POWER_SET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return ret;
}

int Gpio_Hook_Get(int chan)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((chan >= CTL_HOOK_PTT_BIT)&&(chan < CTL_HOOK_BUFF))
	{
		val[0] = chan;
		ret = ioctl(gpio_fd, IOCTL_CMD_HOOK_GET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return val[0];	
}


int Gpio_Slot_Get(int chan)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((chan >= CTL_SLOT_QINWU_BIT)&&(chan < CTL_SLOT_BUFF))
	{
		val[0] = chan;
		ret = ioctl(gpio_fd, IOCTL_CMD_SLOT_GET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return val[0];		
}

int Gpio_Slot_Set(int chan, int status)
{
	unsigned int val[2] = {0};
	int ret = 0;

	//LOG("%s: chan %d, status %d\r\n", __func__, chan, status);
	
	if((chan >= CTL_SLOT_QINWU_BIT)&&(chan < CTL_SLOT_BUFF))
	{
		val[0] = chan;
		val[1] = status;
		ret = ioctl(gpio_fd, IOCTL_CMD_SLOT_SET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return ret;	
}

int Gpio_Uart_Get(int chan)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((chan >= CTL_UART0_VAL)&&(chan < CTL_UART_BUFF))
	{
		val[0] = chan;
		ret = ioctl(gpio_fd, IOCTL_CMD_UART_GET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return val[0];	
}

int Gpio_Uart_Set(int chan, int status)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((chan >= CTL_UART0_VAL)&&(chan < CTL_UART_BUFF))
	{
		val[0] = chan;
		val[1] = status;
		ret = ioctl(gpio_fd, IOCTL_CMD_UART_SET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}


	return ret;	
}

int Gpio_Ring_Get(int chan)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((chan >= TRUNK_OFFSET)&&(chan < (PHONE_OFFSET + PHONE_NUM_MAX)))
	{
		val[0] = chan;
		ret = ioctl(gpio_fd, IOCTL_CMD_RING_GET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return val[0];	
}

int Gpio_Ring_Set(int chan, int status)
{
	unsigned int val[2] = {0};
	int ret = 0;

	LOG("%s: chan %d, status %d\r\n", __func__, chan, status);
	
	if((chan >= TRUNK_OFFSET)&&(chan < (PHONE_OFFSET + PHONE_NUM_MAX)))
	{
		val[0] = chan;
		val[1] = status;
		ret = ioctl(gpio_fd, IOCTL_CMD_RING_SET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return ret;	
}

int Gpio_Trunk_Get(int chan)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((chan >= TRUNK_OFFSET)&&(chan < (TRUNK_OFFSET + TRUNK_NUM_MAX)))
	{
		val[0] = chan;
		ret = ioctl(gpio_fd, IOCTL_CMD_RING_GET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return val[0];	
}

int Gpio_Trunk_Set(int chan, int status)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((chan >= TRUNK_OFFSET)&&(chan < (TRUNK_OFFSET + TRUNK_NUM_MAX)))
	{
		val[0] = chan;
		val[1] = status;
		ret = ioctl(gpio_fd, IOCTL_CMD_RING_SET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return ret;		
}

int Gpio_Mode_Get(int chan)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((chan >= CTL_MODE_AC491CLK_BIT)&&(chan < CTL_MODE_BUFF))
	{
		val[0] = chan;
		ret = ioctl(gpio_fd, IOCTL_CMD_MODE_GET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return val[0];	
}

int Gpio_Mode_Set(int chan, int status)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((chan >= CTL_MODE_AC491CLK_BIT)&&(chan < CTL_MODE_BUFF))
	{
		val[0] = chan;
		val[1] = status;
		ret = ioctl(gpio_fd, IOCTL_CMD_MODE_SET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return ret;
}

int Gpio_Ver_Get(int chan, int status)
{

	

	return DRV_OK;	
}


int Gport_Speed_Get(int speed)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((speed >= CTL_GINTF_KUOZHAN_2M)&&(speed < CTL_GINTF_WORKMODE_BUTT))
	{
		val[0] = speed;
		ret = ioctl(gpio_fd, IOCTL_CMD_GINTF_GET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return val[0];	
}

int Gport_Speed_Set(int speed)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((speed >= CTL_GINTF_KUOZHAN_2M)&&(speed < CTL_GINTF_WORKMODE_BUTT))
	{
		val[0] = speed;
		ret = ioctl(gpio_fd, IOCTL_CMD_GINTF_SET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return ret;
}

int Gpio_Gain_Set(int gain)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((gain >= CTL_GAIN_PAOFANG)&&(gain < CTL_GAIN_BUTT))
	{
		val[0] = gain;
		ret = ioctl(gpio_fd, IOCTL_CMD_GAIN_SET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return ret;
}

int Gpio_Qinwu_Gain_Set(int gain)
{
	unsigned int val[2] = {0};
	int ret = 0;
	
	if((gain >= CTL_GAIN_MIN24DB)&&(gain < CTL_GAIN_SET_BUTT))
	{
		val[0] = gain;
		ret = ioctl(gpio_fd, IOCTL_CMD_QINWU_GAIN_SET, &val[0]);
	}
	else
	{
		return DRV_ERR;
	}

	return ret;
}



/* ½»»»Ð¾Æ¬¼Ä´æÆ÷¶Á²âÊÔº¯Êý */
unsigned short switch_Reg_Read(unsigned int page, unsigned int reg)
{
	int ret = 0;
	struct switch_struct switch_val;
	
	memset(&switch_val, 0x00, sizeof(switch_val));
	
	switch_val.page = page;
	switch_val.reg = reg;

	ret = ioctl(gpio_fd, IOCTL_CMD_SWITCH_GET, &switch_val);
	
	LOG("Read reg 0x%x,val 0x%x\r\n", reg, switch_val.val);

	return switch_val.val;
}

int switch_vlan_get(unsigned int reg)
{
	int ret = 0;
	struct switch_struct switch_val;
	
	memset(&switch_val, 0x00, sizeof(switch_val));
	
	switch_val.page = 0x33;
	switch_val.reg = reg * 8;

	ret = ioctl(gpio_fd, IOCTL_CMD_SWITCH_VLAN_GET, &switch_val);

	printf("switch_val->reg = %d, switch_val->val = %d\r\n", reg, switch_val.val);

	return ret;
}

int switch_vlan_set(unsigned int reg, unsigned int val)
{
	int ret = 0;
	struct switch_struct switch_val;
	
	memset(&switch_val, 0x00, sizeof(switch_val));
	
	switch_val.page = 0x33;
	switch_val.reg = reg;
	switch_val.val = val;

	ret = ioctl(gpio_fd, IOCTL_CMD_SWITCH_VLAN_SET, &switch_val);
	
	return ret;
}

int main_vlan_init(void)
{
	char buf[100] = {0};
	char *vlan_id = NULL;
	int i = 0, j = 0;
	int val = 0;
	
	for(i = 0; i < MAIN_VLAN_MAX; i++)
	{
		if((WORK_MODE_PAOBING == Param_update.workmode)||(WORK_MODE_ZHUANGJIA == Param_update.workmode))
		{
			if(3 == i)
			{
				continue;
			}
		}
		else if(WORK_MODE_JILIAN == Param_update.workmode)
		{
			if((0 == i)||(1 == i)||(2 == i))
			{
				continue;
			}
		}
		
		snprintf(&buf[0], sizeof(buf), "vlan%d",  i);
		vlan_id = find_config_var(config_cfg, buf);
		if(vlan_id)
		{
			val = atoi(vlan_id);			
			if(0 != val)
			{
				vlan_cfg[i].vlan_id = val;
			}
		}
	}

	for(i = 0; i < MAIN_VLAN_PORT; i++)
	{
		for(j = 0; j < MAIN_VLAN_MAX; j++)
		{
			if((0 != vlan_cfg[j].vlan_id) &&(vlan_cfg[j].vlan_id & (1 << i)))
			{
				vlan_reg_val[i] |=  vlan_cfg[j].vlan_id;
			}
		}

		switch_vlan_set(i* 8, vlan_reg_val[i]);		
	}

	return DRV_OK;
}


int Gpio_pm_setting(int32 val)
{
	int i = 0;
	ST_PM_POWERCTRL setting; 

	memset(&setting, 0x00, sizeof(setting));
	
	for(i = 0; i < CTL_POWER_BUFF; i++)
	{
		setting.pmCmdId = i;
		setting.OffOnFlg = !((val >> i)&0x01);
		ioctl(gpio_fd, IOCTL_CMD_POWER_SET, &setting);	
		usleep(100 * 1000);
	}

	g_pm_val =  val;
	
	return DRV_OK;
}

static int Gpio_pm_init(void)
{
	uint8 i = 0;	
	printf("%s: \r\n", __func__);

	memset(pm_cfg, 0xFF, sizeof(pm_cfg));
	
	pm_cfg[CTL_POWER_WIRE_BIT] = get_config_var_val(mgw_cfg, "CP_WIRE_PM");
	pm_cfg[CTL_POWER_RADIO_BIT] = get_config_var_val(mgw_cfg, "CP_RADIO_PM");
	pm_cfg[CTL_POWER_RAY_BIT] = get_config_var_val(mgw_cfg, "CP_RAY_PM");
	pm_cfg[CTL_POWER_WARNET_BIT] = get_config_var_val(mgw_cfg, "CP_WARNET_PM");
	pm_cfg[CTL_POWER_DINET_BIT] = get_config_var_val(mgw_cfg, "CP_DINET_PM");


	for(i = 0; i < CTL_POWER_BUFF; i++)
	{
		g_pm_val |= (pm_cfg[i] << i);
	}
	
	Gpio_pm_setting(g_pm_val);

	return DRV_OK;
}

int Gpio_init(void)
{
	int val = -1;
	int gain = -1;
	gpio_fd = open(GPIO_DEV_NAME, O_RDWR);
	if (gpio_fd < 0)
	{		
		ERR("open device gpio %s failed\n",GPIO_DEV_NAME);
		return DRV_ERR;
	}

	if(WORK_MODE_PAOBING == Param_update.workmode)
	{
		Gpio_WorkMode_Set(CTL_WORKMODE_BIT, 0);
	}
	else if((WORK_MODE_ZHUANGJIA == Param_update.workmode)\
		||(WORK_MODE_ADAPTER == Param_update.workmode))
	{
		Gpio_WorkMode_Set(CTL_WORKMODE_BIT, 1);
	}
	else
	{
		Gpio_WorkMode_Set(CTL_WORKMODE_BIT, 1);
	}
	
	Gpio_Mode_Set(CTL_MODE_QINWU_ON_BIT, 1);

#if 0
	Gpio_Reset_Set(CTL_RESET_BCM53242_BIT, 1);
	usleep(100 * 1000);
	Gpio_Reset_Set(CTL_RESET_BCM53242_BIT, 0);
#endif

	main_vlan_init();

	Gpio_pm_init();

	val = get_config_var_val(mgw_cfg, "gport_speed");
	switch(Param_update.workmode)
	{
		case WORK_MODE_PAOBING:
			Gport_Speed_Set(CTL_GINTF_DULIKAISHE);
	        Gpio_Gain_Set(CTL_GAIN_PAOFANG);
	        gain = get_config_var_val(mgw_cfg, "gain_paofang");
	        Gpio_Qinwu_Gain_Set(gain);
			break;
		case WORK_MODE_ZHUANGJIA:
	        Gpio_Gain_Set(CTL_GAIN_ZHUANGJIA);
	        gain = get_config_var_val(mgw_cfg, "gain_zhuangjia");
	        Gpio_Qinwu_Gain_Set(gain);
	        break;
		case WORK_MODE_JILIAN:
		case WORK_MODE_SEAT:
			break;
		case WORK_MODE_ADAPTER:
			Gport_Speed_Set(val);
			Gpio_Gain_Set(CTL_GAIN_ADAPTER);
	        gain = get_config_var_val(mgw_cfg, "gain_adapter");
	        Gpio_Qinwu_Gain_Set(gain);
			break;
		default:
			break;
	}	

	/* ¶ÁÐ´Strap reg:Page 0 Addr 50£¬²é¿´Ó²¼þÅäÖÃ¹Ü½ÅµÄÖµ*/
	//switch_Reg_Read(0x0, 0x50);
	
	LOG("Gpio init is ok!\n");

	return DRV_OK;	
}

int Gpio_close(void)
{
	close(gpio_fd);

	return DRV_OK;	
}

#if defined(__cplusplus) 
} 
#endif 

#endif // __GPIO_API_H__


