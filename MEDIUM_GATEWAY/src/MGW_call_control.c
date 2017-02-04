/*
==================================================================
!\brief
* 实现呼叫控制和处理,中继控制
==================================================================
*/

#include "PUBLIC.h"
#include "AC49xDrv_Drv.h"

/*
** @brief
*/
TERMINAL_BASE	terminal_group[KUOZHAN_USER_NUM_MAX];
FENJI_BASE		fenji_group[FEIJI_NUM_MAX]={{"",FENJI_UNSET},{"",FENJI_UNSET}};
ST_SIPAG_INTF_FUNC st_sipag_intf_func;
Local_Param Param_update;

int g_mgwdrv;
extern uint32 time_cnt_100ms; //计算成员是否掉线时间

/* 4个电台K口PRN接入战网时的炮防专线号码和专线创建状态，数组下标分别对应电台K口1-4 */
NETWARE_ZHUANXIAN_BASE zhuanxian_group[ZW_ZHUANXIAN_NUM];
char jtjno[2] = {0x09,0x01};//用于保存集团军号,默认为0901


extern int g_ac491_dev_fd;
extern int ac491_thread_flg;

int32 Ac491_Init(void)
{
	printf("%s:\r\n", __func__);
	Param_update.ac491_init = TRUE;
	
	switch(Param_update.workmode)
	{
		case WORK_MODE_PAOBING:
			Gpio_WorkMode_Set(CTL_WORKMODE_BIT, 0);
			break;
		case WORK_MODE_ZHUANGJIA:
			Gpio_WorkMode_Set(CTL_WORKMODE_BIT, 1);
			break;
		case WORK_MODE_JILIAN:
			break;
		case WORK_MODE_SEAT:
			break;
		case WORK_MODE_ADAPTER:
			Gpio_WorkMode_Set(CTL_WORKMODE_BIT, 1);
			break;
		default:
			Gpio_WorkMode_Set(CTL_WORKMODE_BIT, 1);
			break;
	}	

	ac491_thread_flg = 1;
	close(g_ac491_dev_fd);
	usleep(500 * 1000);
	ac491_thread_flg = 0;
	
	Ac491DrvInit();/*AC491 test */
	usleep(50 * 1000);	
	Param_update.ac491_init = FALSE;
	
	return DRV_OK;
}

int init_terminal(const void *data)
{
	BYTE i;
	char name[10] = {0};
	int32 id = 0;
//	struct sched_context * con = (struct sched_context *)data;

	memset(terminal_group, 0x00, sizeof(terminal_group));
	memset(zhuanxian_group, 0x00, sizeof(zhuanxian_group));
	memset(fenji_group, 0x00, sizeof(fenji_group));
	
	/* 专线状态变量初始化 */
	for(i = 0; i < (ZW_ZHUANXIAN_NUM); i++)
	{
		SET_ZX_CREATE_TERMINALID(i, TERMINAL_ID_INVALID); //终端标识初始化成无效值
	}

	/*the thread in ac491 module take most of cpu resource*/
	Param_update.ac491_init = TRUE;
	Ac491DrvInit();
	Param_update.ac491_init = FALSE;
	Param_update.num_len = 0x07;
	
	g_mgwdrv = open("/dev/mgwdrv",O_RDWR);
	if(g_mgwdrv < 0)
	{
		VERBOSE_OUT(LOG_SYS,"Open MGW Driver Error:%s\n",strerror(errno));

		return 0;
	}

	for(i = 0; i < ZW_ZHUANXIAN_NUM; i++)
	{
		//sprintf(name,"K%d", i);
		snprintf(name, sizeof(name), "K%d", i);
		id = get_config_var_val(mgw_cfg, name);
		if((id >= 0)&&(id <= USER_OPEN_NUM_MAX))
		{
			terminal_group[id].pre_zhuanxian_flg = NETWARE_ZXCREATE_OK;
		}
	}
	
	/*Qinwu init */
	for(i = QINWU_OFFSET; i < (QINWU_OFFSET + QINWU_NUM_MAX); i++)
	{
		if(WORK_MODE_PAOBING == Param_update.workmode)
		{
			terminal_group[i].bPortType = PORT_TYPE_QINWU;
		}
		else if(WORK_MODE_ZHUANGJIA == Param_update.workmode)
		{
			terminal_group[i].bPortType = PORT_TYPE_HEADSET;
		}
		
		terminal_group[i].wPort = i;
		terminal_group[i].bHookStatus = IDLE;
		terminal_group[i].bPortState = MGW_XIWEI_INIT_STATE;
		terminal_group[i].bPortState1 = 0;
		terminal_group[i].bslot_us = QINWU_BASE_SLOT + i - QINWU_OFFSET;
		terminal_group[i].bresv =  IDLE;		
	}

	/*Trunk init */
	for(i = TRUNK_OFFSET; i < (TRUNK_OFFSET + TRUNK_NUM_MAX); i++)
	{
		terminal_group[i].bPortType = PORT_TYPE_TRK;
		terminal_group[i].wPort = i;
		terminal_group[i].bHookStatus = IDLE;
		terminal_group[i].bPortState = MGW_TRK_INIT_STATE;
		terminal_group[i].bPortState1 = 0;
		terminal_group[i].bslot_us = TRUNK_BASE_SLOT + i - TRUNK_OFFSET;
		terminal_group[i].bresv =  IDLE;		
	}
	
	/*Phone init */
	for(i = PHONE_OFFSET; i < (PHONE_OFFSET + PHONE_NUM_MAX); i++)
	{
		if((WORK_MODE_PAOBING == Param_update.workmode)||(WORK_MODE_ZHUANGJIA == Param_update.workmode))
		{
			terminal_group[i].bPortType = PORT_TYPE_PHONE;
		}
		else if(WORK_MODE_JILIAN == Param_update.workmode)
		{
			terminal_group[i].bPortType = PORT_TYPE_LOCAL_XIWEI;
		}
		
		terminal_group[i].wPort = i;
		terminal_group[i].bHookStatus = IDLE;
		terminal_group[i].bPortState = MGW_PHONE_INIT_STATE;
		terminal_group[i].bPortState1 = 0;
		terminal_group[i].bslot_us = PHONE_BASE_SLOT+ i - PHONE_OFFSET;
		terminal_group[i].bresv =  IDLE;		
	}

	/*XiWei init */
	for(i = XIWEI_OFFSET; i < (XIWEI_OFFSET + XIWEI_NUM_MAX); i++)
	{
		terminal_group[i].bPortType = PORT_TYPE_XIWEI;
		terminal_group[i].wPort = i;
		terminal_group[i].bHookStatus = IDLE;
		terminal_group[i].bPortState = MGW_XIWEI_INIT_STATE;
		terminal_group[i].bPortState1 = 0;
		terminal_group[i].bslot_us = XIWEI_BASE_SLOT +i - XIWEI_OFFSET;
		terminal_group[i].bresv =  IDLE;		
	}	

	/*Sip init */
	for(i = SIP_OFFSET; i < (SIP_OFFSET + SIP_NUM_MAX); i++)
	{
		terminal_group[i].bPortType = PORT_TYPE_SIP;
		terminal_group[i].wPort = i;
		terminal_group[i].bHookStatus = IDLE;
		terminal_group[i].bPortState = MGW_SIP_INIT_STATE;
		terminal_group[i].bPortState1 = 0;
		terminal_group[i].bslot_us = SIP_BASE_SLOT + i - SIP_OFFSET;
		terminal_group[i].bresv =  IDLE;		
	}

	struct category* iterator_cat = find_config_category(config_cfg,"SEAT");
	struct variable* iterator_var;
	for(iterator_var=iterator_cat->variable_list->root;iterator_var;iterator_var=iterator_var->next)
	{
		int	port_index = 0;
		port_index = atoi(iterator_var->name);
		if(0 != port_index)
		{
			/*QIN WU process*/
			//terminal_group[port_index].bslot_us = (BYTE)atoi(iterator_var->name) + 1;
		}
		
		terminal_group[port_index].rtp_socket.remote_ip = iterator_var->value;
		terminal_group[port_index].rtp_socket.local_ip = find_config_var(mgw_cfg,"EXTEND_LOCAL_IP");
	}

#if 0
	char * fenji_num = NULL;
	fenji_num = find_config_var(mgw_cfg,"FENJI0");
	if(fenji_num)
	{
		//strcpy(fenji_group[0].name,fenji_num);
		strncpy((char *)fenji_group[0].name,fenji_num, sizeof(fenji_group[0].name));
		fenji_group[0].set_state = FENJI_SET;
	}
	
	fenji_num = find_config_var(mgw_cfg,"FENJI1");
	if(fenji_num)
	{
		//strcpy(fenji_group[1].name,fenji_num);
		strncpy((char *)fenji_group[1].name,fenji_num, sizeof(fenji_group[1].name));
		fenji_group[1].set_state = FENJI_SET;
	}
#endif

	return 0;
}

int do_sendtone(cmd_tbl_t *cmdtp, int argc, char *argv[])
{
    if((0 == argc)||(NULL == argv))
    {
        ERR("%s: param error\r\n", __func__);
        return DRV_ERR;
    }
	if(argc != 3){
		printf("Invalid Parameter!\n");
		printf("usage: %s",cmdtp->usage);
		return 0;
	}

	if(argc == 3 && strcasecmp(argv[2],"DIAL") == 0)
		Send_TONE(atoi(argv[1]),TONE_DIAL);
	else if(argc == 3 && strcasecmp(argv[2],"BUSY") == 0)
		Send_TONE(atoi(argv[1]),TONE_BUSY);
	else if(argc == 3 && strcasecmp(argv[2],"STOP") == 0)
		Send_TONE(atoi(argv[1]),TONE_STOP);
	else if(argc == 3 && strcasecmp(argv[2],"SILENT") == 0)
		Send_TONE(atoi(argv[1]),TONE_SILENT);
	else if(argc == 3 && strcasecmp(argv[2],"RING") == 0)
		Send_TONE(atoi(argv[1]),TONE_RING);	
	else
		VERBOSE_OUT(LOG_SYS,"Unknow Tone %s to %s\n",argv[2],argv[1]);
	
	return 0;
}


int send_dtmf_string(const void * data)
{
	TERMINAL_BASE * ter = (TERMINAL_BASE *)data;

	if(ter->phone_queue_s.bPhoneQueueTail < ter->phone_queue_s.bPhoneQueueHead){
		if(ter->bCommand == MC_TALK){
			Send_DTMF(ter->wPort,ter->phone_queue_s.bPhoneQueue[ter->phone_queue_s.bPhoneQueueTail]);
			DEBUG_OUT("Send Digit %c to trk%d\n", \
				ter->phone_queue_s.bPhoneQueue[ter->phone_queue_s.bPhoneQueueTail], \
				ter->wPort);
		}else{
			Send_DTMF(ter->wPort,ter->phone_queue_s.bPhoneQueue[ter->phone_queue_s.bPhoneQueueTail]);
			DEBUG_OUT("Send Digit %c to trk%d\n", \
				ter->phone_queue_s.bPhoneQueue[ter->phone_queue_s.bPhoneQueueTail], \
				ter->wPort);
		}
		ter->phone_queue_s.bPhoneQueueTail++;
		return 1;
	}
	else
	{
		ter->phone_queue_s.bPhoneQueueHead = 0;
		ter->phone_queue_s.bPhoneQueueTail = 0;
		if((WORK_MODE_PAOBING == Param_update.workmode)\
			||(WORK_MODE_ZHUANGJIA == Param_update.workmode))
		{
			if(ter->bCommand == MC_TALK)
			{
				sw2st_bus(ter->wPort);		
			}
		}
		return 0;
	}
	return 0;
}


int do_senddtmf(cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	unsigned char	tmpport;

    if((0 == argc)||(NULL == argv))
    {
        ERR("%s: param error\r\n", __func__);
        return DRV_ERR;
    }

	//unsigned char 	index;
	if(argc != 3){
		printf("Invalid Parameter!\n");
		printf("usage: %s",cmdtp->usage);
		return 0;
	}

	tmpport = atoi(argv[1]);
	if(tmpport >= USER_NUM_MAX){
		printf("Invalid Paramter port:%d!\n",tmpport);
		return 0;
	}
	
	//strcpy((char *)terminal_group[tmpport].phone_queue_s.bPhoneQueue, argv[2]);
	strncpy((char *)terminal_group[tmpport].phone_queue_s.bPhoneQueue, \
	    argv[2], MAXPHONENUM);
	terminal_group[tmpport].phone_queue_s.bPhoneQueueHead = strlen(argv[2]);
	terminal_group[tmpport].phone_queue_s.bPhoneQueueTail = 0;
	inc_sched_add(con,500,send_dtmf_string,&terminal_group[tmpport]);
	return 0;
}

int do_trk_action(cmd_tbl_t * cmdtp,int argc,char * argv [ ])
{
	char trk_port;

    if((0 == argc)||(NULL == argv))
    {
        ERR("%s: param error\r\n", __func__);
        return DRV_ERR;
    }
    
	if(argc != 3){
		printf("Invalid Parameter!\n");
		printf("usage: %s",cmdtp->usage);
		return 0;
	}

	trk_port = atoi(argv[2]);

	if(trk_port>1 || trk_port<0){
		printf("Invalid Parameter!\n");
		printf("usage: %s",cmdtp->usage);		
		return 0;
	}
	
	if(!strcasecmp(argv[1],"use"))
		use_trk(&terminal_group[trk_port+1]);
	else if(!strcasecmp(argv[1],"free"))
		free_trk(&terminal_group[trk_port+1]);

	return 0;

}

int do_listen_slot(cmd_tbl_t * cmdtp,int argc,char * argv [ ])
{
	int port;
	static char local_ip[24];
	static char remote_ip[24];
    if((0 == argc)||(NULL == argv))
    {
        ERR("%s: param error\r\n", __func__);
        return DRV_ERR;
    }
    
	if(argc > 3 || argc < 2){
		printf("Invalid Parameter!\n");
		printf("usage: %s",cmdtp->usage);
		return 0;
	}
	if(argc == 2)
		port = atoi(argv[1]);
	
	if(argc == 3)
	{
		port = atoi(argv[1]);
		if(!strcmp(argv[2],"stop")){
			terminal_group[port].bCommand = MC_IDLE;
			rtp_io_remove(terminal_group[port].rtp_io_id);
			close(terminal_group[port].rtp_socket.udp_handle);
		}
		else
			DEBUG_OUT("param 2 must be stop\n");
		return 0;
	}
	terminal_group[port].bCommand = MC_LISTEN;
	terminal_group[port].bslot_us = port;
	DEBUG_OUT("Listen AC491 Slot:%d\n",port);
	//strcpy(local_ip,"10.0.0.3");
	//strcpy(remote_ip,"10.0.0.41");
	strncpy((char *)local_ip,"10.0.0.3", sizeof(local_ip));
	strncpy((char *)remote_ip,"10.0.0.41", sizeof(remote_ip));
	terminal_group[port].rtp_socket.local_ip = local_ip;
	terminal_group[port].rtp_socket.local_port = htons(30008);
	terminal_group[port].rtp_socket.remote_ip = remote_ip;
	terminal_group[port].rtp_socket.remote_port = htons(30008);
	/*
	if(socket_new_with_bindaddr(&terminal_group[port].rtp_socket)<0)
		VERBOSE_OUT(LOG_SYS,"Create listen Socket 0 Error:%s",strerror(errno));
	else
		terminal_group[port].rtp_io_id = rtp_io_add(terminal_group[port].rtp_socket.udp_handle,&terminal_group[port]);
	*/
	return 0;
}

int do_hear_slot(cmd_tbl_t * cmdtp,int argc,char * argv [ ])
{
    if((0 == argc)||(NULL == argv))
    {
        ERR("%s: param error\r\n", __func__);
        return DRV_ERR;
    }
	if(argc == 2 && !strcmp(argv[argc-1],"stop")){
		printf("Hear NetSide Voice Stop!\n");
		terminal_group[0].bCommand = MC_IDLE;
	}
	else if(argc == 1){
		printf("Hear NetSide Voice and Send to slot 0\n");
		terminal_group[0].bCommand = MC_HEAR;
	}else{
		printf("Invalid Parameter!\n");
		printf("usage: %s",cmdtp->usage);
		return 0;	
	}
	return 0;	
}


int close_channel(const void * data)
{
	int i;
	data = data;
	
	ST_HF_MGW_SCU_PRO pro;
	ST_HF_MGW_SCU_CLOSE_REQ * req;

	pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	pro.header.version = HF_MGW_SCU_PRO_VER;
	pro.header.info_type = INFO_TYPE_PARARM_SET;
	pro.header.len = sizeof(ST_HF_MGW_SCU_CLOSE_REQ);

	req = (ST_HF_MGW_SCU_CLOSE_REQ *)pro.body;
	req->id = MGW_SCU_CLOSE_REQ_ID;
	req->len = SIP_OFFSET+1;
	req->mem_count = SIP_OFFSET;
	for(i=0;i<SIP_OFFSET;i++)
		req->terminal[i] = terminal_group[i].bslot_us;
	VERBOSE_OUT(LOG_SYS,"send close msg to scu!\n");

	//sendto(scu_udp_socket.udp_handle,&pro,sizeof(ST_HF_MGW_SCU_PRO_HEADER)+req->len+2, 0,(struct sockaddr*)&scu_udp_socket.addr_them,scu_udp_socket.addr_len);

	/*Socket_Send(gIpcSemSocket.udp_handle, (struct sockaddr_in*)&gIpcSemSocket.addr_them , 
				  &pro,sizeof(ST_HF_MGW_SCU_PRO_HEADER)+req->len+2);*/
	return 0;
}


/*
** common trunk port control function
* trk port use port bit define:
* 0---use trk
* 1---free trk
* hook statu reg bit define:
* 1---in trk/hook off(cishi)
* 0---hook on(cishi)
*/
int	use_trk(TERMINAL_BASE* trkext)
{
	Gpio_Ring_Set(trkext->wPort, 0);

	return 0;
}

int free_trk(TERMINAL_BASE* trkext)
{
	Gpio_Ring_Set(trkext->wPort, 1);
	
	return 0;
}


int ring_cishi_on(TERMINAL_BASE* cishiext)
{
    cishiext->bPTTStatus = MGW_PTT_INIT;
	return 0;
}

int ring_cishi_off(TERMINAL_BASE* cishiext)
{
    cishiext->bPTTStatus = MGW_PTT_INIT;
	return 0;
}

int update_hookstatus(void)
{
	static int val_save =0;
	static int ptt_status = 0;
	static int change_cnt = 0;
	 int val = 0, i = 0, temp_val = 0;
	 
	val =Gpio_Hook_Get(0);

	if(val != val_save)
	{
		change_cnt++;
		
		if(0x00 == (change_cnt%3))
		{
			change_cnt = 0;
			
			val_save = val;
			
			printf("%s: val_save = 0x%x\r\n", __func__, val_save);

			for(i = CTL_HOOK_PTT_BIT; i <= CTL_HOOK_PTT_BIT; i++)
			{
				temp_val = (val_save >> i) & 0x01;	
				if(ptt_status != temp_val)
				{
					//terminal_group[i].bPTTStatus = temp_val;
					ptt_status = temp_val;

					if(1 == temp_val)
					{
						terminal_group[i].bPTTStatus = MGW_PTT_UP;
					}
					else if(0 == temp_val)
					{
						terminal_group[i].bPTTStatus = MGW_PTT_DOWN;
					}
					

					LOG("ppt %s \r\n", terminal_group[i].bPTTStatus ? "Up" : "Down");
				}
				else
				{
					//terminal_group[i].bPTTStatus = MGW_PTT_HOLD;
					;
				}
			}		

			for(i = CTL_HOOK_TRUNK0_BIT; i <= CTL_HOOK_TRUNK1_BIT; i++)
			{
				temp_val = (val_save >> i) & 0x01;	
				if(terminal_group[i].trk_ring != temp_val)
				{
					terminal_group[i].trk_ring = temp_val;
				
					if((1 == terminal_group[i].trk_ring)&&(HOOK_OFF != terminal_group[i].bHookStatus))
					{
						if(PORT_TYPE_TRK == terminal_group[i].bPortType)
						{
							terminal_group[i].bHookStatus = HOOK_OFF;
							printf("trunk %d Ring \r\n", i - CTL_HOOK_TRUNK0_BIT);
						}
					}
				}
			}		
		
			for(i = CTL_HOOK_PHONE0_BIT; i <= CTL_HOOK_PHONE11_BIT; i++)
			{
				temp_val = (val_save >> i) & 0x01;	
				if(temp_val != terminal_group[i].bHookStatus)
				{
					if(PORT_TYPE_PHONE == terminal_group[i].bPortType)
					{
						if(temp_val)
						{
							terminal_group[i].bHookStatus = HOOK_ON;
							printf("phone %d hookon\r\n", i - PHONE_OFFSET);
						}
						else
						{
							terminal_group[i].bHookStatus = HOOK_OFF;
							printf("phone %d hookoff\r\n", i - PHONE_OFFSET);
						}
					}
				}
			}
		}
	}
	
	return 0;
}


/*
! brief
* for some reason,there must have a switch that
* selete local user voice to ac491(pcm) or st_bus
* Talk state: sw2st_bus(ch)
* Idle state: sw2ac491(ch)
*
* Parameters
* ch - 0~3
*
* return value:
* 0 - success
* other - fail
*/
int	sw2ac491(unsigned char ch)
{
	Gpio_Slot_Set(ch, 0);

	return 0;
}

int	sw2st_bus(unsigned char ch)
{
	Gpio_Slot_Set(ch, 1);
	
	return 0;
}

static int32 sw2net(unsigned char ch)
{
	Gpio_Slot_Set(ch, 2);

	return 0;
}

#ifdef ZHUANXIAN_VOICE_CTRL	
static int32 sw2_phone_talk(unsigned char ch)
{
	Gpio_Slot_Set(ch, 3);

	return 0;
}
#endif

/*====================================mgw_sip========================================*/
int find_port_by_name(const char * name)
{
	int i;
	for(i=0;i<USER_NUM_MAX;i++)
	{
		if(!strcmp(terminal_group[i].name,name)){
			return i;
		}
	}
	
	return DRV_ERR;
}

int find_port_by_simple_ip(uint32 ipaddr)
{
	int i;
	for(i = 0;i < KUOZHAN_USER_NUM_MAX;i++)
	{
		if(terminal_group[i].ipaddr == ipaddr)
		{
			return i;
		}
	}
	
	return DRV_ERR;
}

int find_port_by_simple_slot(unsigned char slot)
{
	int i;
	
	for(i = 0;i < KUOZHAN_USER_NUM_MAX;i++)
	{	
		if(terminal_group[i].bslot_us == slot)
		{
			return i;
		}
	}
	
	return DRV_ERR;
}

int find_port_by_name_sip(const char * name)
{
	int i;
	for(i=SIP_OFFSET;i<USER_NUM_MAX;i++)
	{
		if(!strcmp(terminal_group[i].name,name) && terminal_group[i].bCommand == MC_DIAL){
			return i;
		}
	}
	return DRV_ERR;
}


int find_port_by_CallID(unsigned short callid)
{
	int i;
	for(i=0;i<USER_NUM_MAX;i++)
	{
		if(terminal_group[i].bPortType == PORT_TYPE_SIP && \
			terminal_group[i].call_id == callid){
			return i;
		}
	}
	return DRV_ERR;
}

/*从IP<->时隙表中对应关系*/
/*
* return value: (-1)---not found
* 0-16---find slot
*/
int find_port_by_ip(struct sockaddr_in *addr)
{
	int i;
	for(i=0;i<USER_NUM_MAX;i++)
	{
		if(terminal_group[i].bPortType == PORT_TYPE_QINWU || 
		  terminal_group[i].bPortType == PORT_TYPE_XIWEI)
		{
			//printf("terminal_group[%d].rtp_socket.remote_ip = %s", i, terminal_group[i].rtp_socket.remote_ip);
			if(NULL == terminal_group[i].rtp_socket.remote_ip)
			{
				return DRV_ERR;
			}
			
			if(addr->sin_addr.s_addr == inet_addr(terminal_group[i].rtp_socket.remote_ip))
			{
				return i;
			}
		}
	}
	
	return DRV_ERR;
}

int find_port_by_slot(unsigned char slot)
{
	int i;
	
	for(i=0;i<USER_NUM_MAX;i++)
	{
		if(terminal_group[i].bslot_us == slot)
		{
			return i;
		}
	}
	
	/*fix radio direct call -- swf 20120419*/
	for(i=0;i<USER_NUM_MAX;i++)
	{
		if(((terminal_group[i].bCallType == EN_CALL_Z) ||
		   (terminal_group[i].bCallType == EN_CALL_RADIO)) &&
			terminal_group[i].bresv == slot)
		{
			return i;
		}
	}
	
	return DRV_ERR;
}

int find_local_port_by_slot(unsigned char slot)
{
	int val = 0;

	val = slot - 13;
	if(val > 0)
	{
		return val;
	}
		
	return DRV_ERR;
}

int get_port_sip_idle(void)
{
	int i;
	for(i=0;i<USER_NUM_MAX;i++)
	{
		if(terminal_group[i].bPortType == PORT_TYPE_SIP && \
			terminal_group[i].bCommand == MC_IDLE){
			return i;
		}
	}
	return DRV_ERR;

}

int find_js_node_by_callee(const char* num,const char* num1)
{
	if(!num || !num1)
		return 1;
	DEBUG_OUT("find_js_node_by_callee: %s <-> %s\n",num,num1);
	return strncmp(num,num1,4);
}

int	parse_phonenum(TERMINAL_BASE * ext)
{
	/*
	* 号码分析:
	* 852+xxx : 呼叫预约会议
	* 834+xxxx: 组呼
	*/
	if(!strncmp((char *)ext->phone_queue_g.bPhoneQueue,"852",3)){
		ext->bCallType = EN_PLAN_CONF;
		VERBOSE_OUT(LOG_SYS,"ext %d Call Conference!\n",ext->wPort);
		return 0;
	}
	/*
	else if(!strncmp((char *)ext->phone_queue_g.bPhoneQueue,"834",3))
	{
		ext->bCallType = EN_GROUP_CALL;
		VERBOSE_OUT(LOG_SYS,"ext %d Call Group!\n",ext->wPort);
		return 0;
	}
	*/
	else if(!strncmp((char *)ext->phone_queue_g.bPhoneQueue,"881",3))
	{
		ext->bCallType = EN_PLAN_CONF;
		VERBOSE_OUT(LOG_SYS,"ext %d Call Group!\n",ext->wPort);
		return 0;
	}	
	else
	{
		ext->bCallType = EN_SEL_CALL;
	}
	
	return 0;
}



int	append_phone_number(BYTE ch,BYTE number)
{
	/*
	** @brief
	* Add num to PhoneQueue
	* Set finish flag
	* @note
	* the format is string
	*
	* return value: 0--success -1--fail
	*/
	BYTE bHead = 0;

	if(ch >= USER_NUM_MAX)
	{
		VERBOSE_OUT(LOG_SYS,"UnConfig Channel %d\n",ch);
		return (-1);
	}

	if(number>0x0b)
	{
		VERBOSE_OUT(LOG_SYS,"chan%d Unknow number %d!\n", ch, number);
		return (-1);
	}

    //LOG("channel = %d ,number = %d\r\n", ch,number);

	if((PORT_TYPE_TRK != terminal_group[ch].bPortType)\
		&&(PORT_TYPE_PHONE != terminal_group[ch].bPortType)\
		&&(PORT_TYPE_KUOZHAN_PHONE != terminal_group[ch].bPortType))
	{
		VERBOSE_OUT(LOG_SYS,"chan%d  bPortType error %d!\n", ch, terminal_group[ch].bPortType);
		return (-1);
	}

	if(MC_TALK == terminal_group[ch].bCommand)
	{
		if(number == 0x0a)
		{
			terminal_group[ch].bPTTStatus = MGW_PTT_DOWN;
			LOG("phone%d PPT down\r\n", ch);
		}
		else if(number == 0x0b)
		{
			terminal_group[ch].bPTTStatus = MGW_PTT_UP;
			LOG("phone%d PPT up\r\n", ch);
		}	

		return 0;
	}
	else
	{
		LOG(" chan %d PPT %s \r\n", ch, 0x0a == number ? "Down":"Up" );
	}
		
	if(terminal_group[ch].bCommand != MC_DIAL)
	{
		LOG("%s: chan%d num = %d bCommand = %d\r\n", __func__, ch, number, terminal_group[ch].bCommand);
		return 0;
	}
	
	bHead = terminal_group[ch].phone_queue_g.bPhoneQueueHead;
	terminal_group[ch].phone_queue_g.bPhoneQueue[bHead] = number + '0';
	bHead++;
	terminal_group[ch].phone_queue_g.bPhoneQueueHead = bHead;

	parse_phonenum(&terminal_group[ch]);

	if(terminal_group[ch].bCallType == EN_PLAN_CONF && bHead >= 6){
		char * tmpnum;
		tmpnum = strdup((char *)terminal_group[ch].phone_queue_g.bPhoneQueue);

		if(!strncmp((char *)terminal_group[ch].phone_queue_g.bPhoneQueue,"881",3))
		{
			//strcpy(terminal_group[ch].phone_queue_g.bPhoneQueue,tmpnum);
			strncpy((char *)terminal_group[ch].phone_queue_g.bPhoneQueue,tmpnum, MAXPHONENUM);
			terminal_group[ch].phone_queue_g.bFinish = 1;
			terminal_group[ch].phone_queue_g.bPhoneQueueHead = 6;
		}
		else
		{		
			//strcpy(terminal_group[ch].phone_queue_g.bPhoneQueue,tmpnum + 3);
			strncpy((char *)terminal_group[ch].phone_queue_g.bPhoneQueue,tmpnum + 3, MAXPHONENUM);
			terminal_group[ch].phone_queue_g.bFinish = 1;
			terminal_group[ch].phone_queue_g.bPhoneQueueHead = 3;
		}
		
		free(tmpnum);
	}
	else if(terminal_group[ch].bCallType == EN_GROUP_CALL && bHead >= 3)
	{
		terminal_group[ch].phone_queue_g.bFinish = 1;
	}	
	else if((!strncmp((char *)terminal_group[ch].phone_queue_g.bPhoneQueue,"9",1))&& (bHead >= 3))
	{
		terminal_group[ch].phone_queue_g.bFinish = 1;
	}	
	else if(!strncmp((char *)terminal_group[ch].phone_queue_g.bPhoneQueue,"8",1))
	{
		switch(Param_update.workmode)
		{
			case WORK_MODE_PAOBING:
			case WORK_MODE_ZHUANGJIA:
				if((!strncmp((char *)terminal_group[ch].phone_queue_g.bPhoneQueue,"82",2))&& (bHead >= 5))
				{
					terminal_group[ch].phone_queue_g.bFinish = 1;
				}			
				break;
			case WORK_MODE_JILIAN:
			case WORK_MODE_SEAT:
				break;
			case WORK_MODE_ADAPTER:
				 if((!strncmp((char *)terminal_group[ch].phone_queue_g.bPhoneQueue,"8",1))&& (bHead >= 3)\
					&&(terminal_group[ch].bCallType == EN_SEL_CALL))
					{
						terminal_group[ch].phone_queue_g.bFinish = 1;
					}
				break;
			default:
				break;
		}	
	
		
	}	
	else if(bHead >= Param_update.num_len)
	{
		terminal_group[ch].phone_queue_g.bFinish = 1;
	}
	
	return 0;
}


int detect_tone(BYTE ch,BYTE type)
{
	/*
	** @brief
	* detect call progress tone
	* 6--dial tone,8--busy tone
	* @note
	* return value: 0--success -1--fail
	*/
	if(ch >= USER_NUM_MAX)
	{
		VERBOSE_OUT(LOG_SYS,"UnConfig Channel %d\n",ch);
		return (-1);
	}

	if(type == 8)
	{
		//if(terminal_group[ch].bPortType == PORT_TYPE_TRK && terminal_group[ch].bCommand == MC_TALK)
		if(terminal_group[ch].bPortType == PORT_TYPE_TRK)
		{
			LOG("%s: chan %d  Handup\r\n", __func__, ch);
			terminal_group[ch].bCommand = MC_HANG;
		}
	}
	return 0;
}

char* get_fenji(void)
{
	/*@brief
	** get fenji number
	*/
	int i;

	for(i=0; i<FEIJI_NUM_MAX; i++)
	{
		if(fenji_group[i].set_state == FENJI_SET)
		{
			return fenji_group[i].name;
		}
	}
	
	return NULL;
}

void	MGW_TRK_INIT_Process(TERMINAL_BASE* trkext)
{
	switch(trkext->bPortState1)
	{
		case 1:
		case 0:
			free_trk(trkext);
			trkext->bPortState = MGW_TRK_IDLE_STATE;
			trkext->bPortState1 = 0;
			trkext->wConnectPort = IDLEW;
			trkext->wPortDelay = IDLEW;
			trkext->bCommand = MC_IDLE;
			trkext->bHookStatus = IDLE;
			memset(&trkext->phone_queue_g,0,sizeof(PHONE_QUEUE));
			memset(&trkext->phone_queue_s,0,sizeof(PHONE_QUEUE));
			trkext->zhuanxian_status = ZHUANXIAN_IDLE;
			DEBUG_OUT("trk%d in init state\n",trkext->wPort);
			sw2ac491(trkext->wPort);
		break;
	}
}


void	MGW_TRK_IDLE_Process(TERMINAL_BASE* trkext)
{
	switch(trkext->bPortState1)
	{
		case 0:
			if(trkext->bHookStatus == HOOK_OFF)
			{
				trkext->bPortState = MGW_TRK_IN_DIAL_STATE;
				VERBOSE_OUT(LOG_SYS,"trk %d is hook_off\n",trkext->wPort);
				trkext->bPortState1 = 0;
			}
			
			if(trkext->bCommand == MC_USETRK)
			{
				trkext->bPortState = MGW_TRK_OUT_DIAL_STATE;
				trkext->bPortState1 = 0;
				VERBOSE_OUT(LOG_SYS,"USE TRK...\n");
			}

			if(trkext->bCommand == MC_INACTIVE)
				trkext->bPortState1 = 2;
		break;

		case 2:
			if(trkext->bCommand == MC_ACTIVE){
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 0;
			}
		break;

		default:
			trkext->bPortState1 = 0;
		break;

	}
}

void	MGW_TRK_IN_DIAL_Process(TERMINAL_BASE* trkext)
{
	switch(trkext->bPortState1)
	{
		case 0:
			use_trk(trkext);
			trkext->bCommand = MC_DIAL;
			trkext->bToneType = TONE_DIAL;
			Send_TONE(trkext->wPort,trkext->bToneType);
			trkext->wPortDelay = 150;
			trkext->bPortState1 = 1;
			VERBOSE_OUT(LOG_SYS,"trk %d in use,Start Get NUM\n",trkext->wPort);
		break;

		case 1:
			/*
			** 使用中继端口时，发现有不收号的问题，将
			*  收到的信噪比改为DTMF_SNR__12DB,探测为最高灵敏度
			*/
			if(trkext->wPortDelay == 0)
			{
				trkext->bToneType = TONE_STOP;
				Send_TONE(trkext->wPort,trkext->bToneType);
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 0;
				mgw_scu_release_request(trkext);
			}

			if(trkext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"trk %d recv hangup!\n", trkext->wPort);
				trkext->bToneType = TONE_STOP;
				Send_TONE(trkext->wPort,trkext->bToneType);
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 0;	
			}
			#if 0

			/*若10秒内用户没有拨号操作则振入分机*/
			if(trkext->wPortDelay < 50 && !trkext->phone_queue_g.bPhoneQueueHead)
			{
			
				char * fenji = get_fenji();
				if(NULL != fenji){
					//strcpy(trkext->phone_queue_g.bPhoneQueue,fenji);
					strncpy((char *)trkext->phone_queue_g.bPhoneQueue,fenji, MAXPHONENUM);
					trkext->phone_queue_g.bPhoneQueueHead = strlen(fenji);
					trkext->phone_queue_g.bFinish = 1;
					VERBOSE_OUT(LOG_SYS,"TRK %d Call fenji %s\n",trkext->wPort,fenji);
				}
			    else
			
				{
					VERBOSE_OUT(LOG_SYS,"TRK %d get fenji fail\n",trkext->wPort);
					trkext->bToneType = TONE_STOP;
					Send_TONE(trkext->wPort,trkext->bToneType);
					trkext->bPortState = MGW_TRK_INIT_STATE;
					trkext->bPortState1 = 0;
					mgw_scu_release_request(trkext);					
				}	
			}
			#endif

			//if(trkext->phone_queue_g.bPhoneQueueHead == 1)
			if(trkext->phone_queue_g.bPhoneQueueHead)
			{
				trkext->bToneType = TONE_STOP;
				Send_TONE(trkext->wPort,trkext->bToneType);
			}

			if(trkext->phone_queue_g.bFinish == 1)
			{
				trkext->bCallType = EN_SEL_CALL;
				mgw_scu_call_request(trkext);
				VERBOSE_OUT(LOG_SYS,"TRK %d Send Call %s\n",trkext->wPort,trkext->phone_queue_g.bPhoneQueue);
				trkext->phone_queue_g.bFinish = 0;
				trkext->phone_queue_g.bPhoneQueueHead = 0;
				memset(trkext->phone_queue_g.bPhoneQueue,0,MAXPHONENUM);
				trkext->wPortDelay = 600;
				trkext->bPortState1 = 2;
			}
		break;

		case 2:
			if(trkext->wPortDelay == 0 || trkext->bCommand == MC_REJECT || trkext->bCommand == MC_HANG)
			{
				if(trkext->wPortDelay == 0)
					trkext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				mgw_scu_release_request(trkext);
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 0;
			}

			if(trkext->bCommand == MC_RT_TONE)
			{
				trkext->bPortState = MGW_TRK_IN_RINGTONG_STATE;
				trkext->bPortState1 = 0;
			}
		break;
	    default:
		break;

	}
}


void	MGW_TRK_IN_RINGTONG_Process(TERMINAL_BASE* trkext)
{
	switch(trkext->bPortState1)
	{
		case 0:
			trkext->wPortDelay = 600;
			trkext->bToneType = TONE_RING;
			Send_TONE(trkext->wPort,trkext->bToneType);
			trkext->bPortState1 = 1;
		break;

		case 1:
			if(trkext->wPortDelay == 0)
			{
				trkext->bToneType = TONE_STOP;
				Send_TONE(trkext->wPort,trkext->bToneType);
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 1;
				trkext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				mgw_scu_release_request(trkext);
			}
			
			if(trkext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"trk %d recv hangup!\n", trkext->wPort);
				trkext->bToneType = TONE_STOP;
				Send_TONE(trkext->wPort,trkext->bToneType);
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 0;	
				trkext->rel_reason = MGW_RELEASE_REASON_NORMAL;
				mgw_scu_release_request(trkext);				
			}
			
			if(trkext->bCommand == MC_TALK)
			{
				trkext->bToneType = TONE_STOP;
				Send_TONE(trkext->wPort,trkext->bToneType);
				trkext->bPortState = MGW_TRK_IN_TALK_STATE;
				trkext->bPortState1 = 0;
				sw2st_bus(trkext->wPort);
			}
		break;
	    default:
	    break;
	}
}

void	MGW_TRK_IN_TALK_Process(TERMINAL_BASE* trkext)
{
	switch(trkext->bPortState1)
	{
		case 0:
			if(trkext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"trk %d Rcv hangup!\n", trkext->wPort - TRUNK_OFFSET);
				trkext->wPortDelay = 10;
				trkext->bPortState1 = 1;
			}

			if(trkext->phone_queue_s.bFinish)
			{
				trkext->bPortState1 = 2;
				trkext->wPortDelay = 15;
			}	
			break;
		case 1:
			if(trkext->wPortDelay == 0)
			{
				trkext->rel_reason = MGW_RELEASE_REASON_NORMAL;
				mgw_scu_release_request(trkext);
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 1;
			}
			break;
		case 2:
			if(trkext->wPortDelay == 0)
			{
				sw2ac491(trkext->wPort);
				inc_sched_add(con,150,send_dtmf_string,trkext);
				VERBOSE_OUT(LOG_SYS,"Send dtmf string: %s\n",trkext->phone_queue_s.bPhoneQueue);
				trkext->phone_queue_s.bFinish = 0;
				trkext->bPortState1 = 0;
			}
			
			if(trkext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"trk %d rcv hangup!\n", trkext->wPort - TRUNK_OFFSET);
				trkext->wPortDelay = 10;
				trkext->bPortState1 = 1;
			}			
			break;			
	    default:
	   		break;
	}
}

void	MGW_TRK_OUT_DIAL_Process(TERMINAL_BASE* trkext)
{
#if 0
	switch(trkext->bPortState1)
	{
		case 0:
			trkext->wPortDelay = 150;
			trkext->bPortState1 = 1;
			/*Send callack to host*/
			trkext->bCommand = MC_RING;
			mgw_scu_callack_request(trkext);
		break;

		case 1:
			if(trkext->wPortDelay == 0)
			{
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 0;
				trkext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				mgw_scu_release_request(trkext);
			}
#if 0			
			if(trkext->phone_queue_g.bFinish == 1)
			{
				/*进行简单的号码分析*/
				//parse_num();
				/*发送号码,进入回铃音状态*/
				//Send_DTMF(trkext->wPort,trkext->phone_queue_g.bPhoneQueue);
				trkext->bPortState = MGW_TRK_OUT_RINGTONE_STATE;
				trkext->bPortState1 = 0;
			}	
#else
			mgw_scu_connect_request(trkext);

			trkext->bPortState = MGW_TRK_OUT_TALK_STATE;
			trkext->bPortState1 = 0;
			sw2st_bus(trkext->wPort);
			trkext->bCommand = MC_TALK;
			VERBOSE_OUT(LOG_SYS,"trk goto talk...\n");
#endif
		break;

	}
#endif

	switch(trkext->bPortState1)
	{
		case 0:
			trkext->wPortDelay = 20;
			trkext->bPortState1 = 1;
			/*Send callack to host*/
			trkext->bCommand = MC_RING;
			mgw_scu_callack_request(trkext);
		break;
		case 1:
			if(trkext->wPortDelay == 0)
			{
				trkext->wPortDelay = 150;
				trkext->bPortState1 = 2;
			}

			if(trkext->bCommand == MC_HANG)
			{
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 0;
				trkext->rel_reason = MGW_RELEASE_REASON_NORMAL;
				mgw_scu_release_request(trkext);
			}
			
		break;
		case 2:
			if(trkext->wPortDelay == 0)
			{
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 0;
				trkext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				mgw_scu_release_request(trkext);
			}
#if 0			
			if(trkext->phone_queue_g.bFinish == 1)
			{
				/*进行简单的号码分析*/
				//parse_num();
				/*发送号码,进入回铃音状态*/
				//Send_DTMF(trkext->wPort,trkext->phone_queue_g.bPhoneQueue);
				trkext->bPortState = MGW_TRK_OUT_RINGTONE_STATE;
				trkext->bPortState1 = 0;
			}	
#else
			mgw_scu_connect_request(trkext);
			/* add by guoxiaorong by DTMF phone, buf not for software phone */
			use_trk(trkext);

			trkext->bPortState = MGW_TRK_OUT_TALK_STATE;
			trkext->bPortState1 = 0;
			sw2st_bus(trkext->wPort);
			trkext->bCommand = MC_TALK;
			VERBOSE_OUT(LOG_SYS,"trk goto talk...\n");
#endif
		break;
	    default:
	    break;
	}
	
}


void	MGW_TRK_OUT_RINGTONE_Process(TERMINAL_BASE* trkext)
{
	switch(trkext->bPortState1)
	{
		case 0:
			terminal_group[trkext->wConnectPort].bCommand = MC_RT_TONE;
			trkext->wPortDelay = 50;
			trkext->bPortState1 = 1;
			/*send connect to host*/
			mgw_scu_connect_request(trkext);
		break;

		case 1:
			if(trkext->wPortDelay == 0)
			{
				trkext->bPortState = MGW_TRK_OUT_TALK_STATE;
				trkext->bPortState1 = 0;
				terminal_group[trkext->wConnectPort].bCommand = MC_TALK;

			}
		break;
	    default:
	    break;
	}
}

void	MGW_TRK_OUT_TALK_Process(TERMINAL_BASE* trkext)
{
#if 0
	switch(trkext->bPortState1)
	{
		case 0:
			if(terminal_group[trkext->wConnectPort].bHookStatus == HOOK_ON)
			{
				trkext->bToneType = TONE_BUSY;
				Send_TONE(trkext->wPort,trkext->bToneType);
				trkext->wPortDelay = 600;
				trkext->bPortState1 = 1;
			}
			
			/*忙音检测*/
			if(trkext->bCommand == MC_HANG)
			{
				DEBUG_OUT("trk %d is hangup!\n");
				trkext->wPortDelay = 10;
				trkext->bPortState1 = 1;
			}
		break;

		case 1:
			if(trkext->wPortDelay == 0)
			{
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 1;
			}
		break;

	}
#else
	switch(trkext->bPortState1)
	{
		case 0:
			if(trkext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"trk %d is hangup!\n", trkext->wPort - TRUNK_OFFSET);
				trkext->wPortDelay = 10;
				trkext->bPortState1 = 1;
			}
					
			if(trkext->phone_queue_s.bFinish)
			{
				trkext->bPortState1 = 2;
				trkext->wPortDelay = 15;
			}	
			break;
		case 1:
			if(trkext->wPortDelay == 0)
			{
				mgw_scu_release_request(trkext);
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 0;
			}
			break;
		case 2:
			if(trkext->wPortDelay == 0)
			{
				sw2ac491(trkext->wPort);
				inc_sched_add(con,150,send_dtmf_string,trkext);
				VERBOSE_OUT(LOG_SYS,"Send dtmf string: %s\n",trkext->phone_queue_s.bPhoneQueue);
				trkext->phone_queue_s.bFinish = 0;
				trkext->bPortState1 = 0;
			}
			
			if(trkext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"trk %d is hangup!\n", trkext->wPort - TRUNK_OFFSET);
				trkext->wPortDelay = 10;
				trkext->bPortState1 = 1;
			}			
			break;
	    	default:
	    		break;		
	}
#endif
}


void	MGW_CISHI_INIT_Process(TERMINAL_BASE* cishiext)
{
	switch(cishiext->bPortState1)
	{
		case 1:
		case 0:
			cishiext->bPortState = MGW_CISHI_IDLE_STATE;
			cishiext->bPortState1 = 0;
			cishiext->wConnectPort = IDLEW;
			cishiext->wPortDelay = IDLEW;
			cishiext->bCommand = MC_IDLE;
			cishiext->bHookStatus = HOOK_ON;	/*note: different from trk port*/
			cishiext->phone_queue_g.bFinish = 0;
			cishiext->phone_queue_g.bPhoneQueueHead = 0;
			memset(cishiext->phone_queue_g.bPhoneQueue,0,MAXPHONENUM);
			memset(cishiext->phone_queue_s.bPhoneQueue,0,MAXPHONENUM);
			cishiext->bPTTStatus = MGW_PTT_INIT;
			cishiext->rel_reason = MGW_RELEASE_REASON_NORMAL;
			cishiext->zhuanxian_status = ZHUANXIAN_IDLE;
			//sw2ac491(cishiext->wPort-LOCALTER_OFFSET);
		break;
	    default:
		break;
	}
}

void	MGW_CISHI_IDLE_Process(TERMINAL_BASE* cishiext)
{
	switch(cishiext->bPortState1)
	{
		case 0:
			if(cishiext->bHookStatus == HOOK_OFF)
			{
				cishiext->bPortState = MGW_CISHI_DIAL_STATE;
				VERBOSE_OUT(LOG_SYS,"cishi %d is hook_off\n",cishiext->wPort);
				cishiext->bPortState1 = 0;
			}
			
			if(cishiext->bCommand == MC_RING)
			{
				cishiext->bPortState = MGW_CISHI_RING_STATE;
				cishiext->bPortState1 = 0;
			}

			if(cishiext->bCommand == MC_INACTIVE)
				cishiext->bPortState1 = 2;
		break;

		case 2:
			{
				if(cishiext->bCommand == MC_ACTIVE){
					cishiext->bPortState = MGW_CISHI_INIT_STATE;
					cishiext->bPortState1 = 0;
				}
			}
			break;

		default:
			cishiext->bPortState1 = 0;
		break;

	}
}


void	MGW_CISHI_DIAL_Process(TERMINAL_BASE* cishiext)
{
	switch(cishiext->bPortState1)
	{
			case 0:
				cishiext->bToneType = TONE_DIAL;
				Send_TONE(cishiext->wPort,cishiext->bToneType);
				VERBOSE_OUT(LOG_SYS,"Send CISHI %d Tone Dial\n",cishiext->wPort);
				cishiext->wPortDelay = 150;
				cishiext->bPortState1 = 1;
				cishiext->bCommand = MC_DIAL;
			break;
	
			case 1:
				if(cishiext->wPortDelay == 0)
				{
					cishiext->bToneType = TONE_STOP;
					Send_TONE(cishiext->wPort,cishiext->bToneType);
					VERBOSE_OUT(LOG_SYS,"CISHI %d Dial Time is Up\n",cishiext->wPort);
					cishiext->bPortState = MGW_CISHI_INIT_STATE;
					cishiext->bPortState1 = 0;
				}
	
				if(cishiext->phone_queue_g.bPhoneQueueHead == 1)
				{
					cishiext->bToneType = TONE_STOP;
					Send_TONE(cishiext->wPort,cishiext->bToneType);
				}
	
				if(cishiext->phone_queue_g.bFinish)
				{
					/*磁石话机的呼叫为普通呼叫*/
					VERBOSE_OUT(LOG_SYS,"cishi %d Send Call %s\n",cishiext->wPort,cishiext->phone_queue_g.bPhoneQueue);
					mgw_scu_call_request(cishiext);
					cishiext->wPortDelay = 600;
					cishiext->phone_queue_g.bFinish = 0;
					cishiext->phone_queue_g.bPhoneQueueHead = 0;
					memset(cishiext->phone_queue_g.bPhoneQueue,0,MAXPHONENUM);
					cishiext->bPortState1 = 2;
				}

				if(cishiext->bHookStatus == HOOK_ON)
				{
					cishiext->bToneType = TONE_STOP;
					Send_TONE(cishiext->wPort,cishiext->bToneType);
					
					VERBOSE_OUT(LOG_SYS,"cishi %d hangup!\n",cishiext->wPort);
					cishiext->phone_queue_g.bFinish = 0;
					cishiext->phone_queue_g.bPhoneQueueHead = 0;
					//strcpy(cishiext->phone_queue_g.bPhoneQueue,"");
					strncpy((char *)cishiext->phone_queue_g.bPhoneQueue,"", MAXPHONENUM);

					mgw_scu_release_request(cishiext);
					
					cishiext->bPortState = MGW_CISHI_INIT_STATE;
					cishiext->bPortState1 = 0;
				}
			break;
	
			case 2:
				if(cishiext->wPortDelay == 0)
				{
					cishiext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
					mgw_scu_release_request(cishiext);
					cishiext->bPortState = MGW_CISHI_INIT_STATE;
					cishiext->bPortState1 = 0;
				}

				if(cishiext->bHookStatus == HOOK_ON)
				{	
					VERBOSE_OUT(LOG_SYS,"cishi %d hangup in dial!\n",cishiext->wPort);
					mgw_scu_release_request(cishiext);
					cishiext->bPortState = MGW_CISHI_INIT_STATE;
					cishiext->bPortState1 = 0;
				}

	
				if(cishiext->bCommand == MC_RT_TONE)
				{
					cishiext->bPortState = MGW_CISHI_RINGTONE_STATE;
					cishiext->bPortState1 = 0;
				}

				if(cishiext->bCommand == MC_REJECT)
				{
					cishiext->bToneType = TONE_BUSY;
					Send_TONE(cishiext->wPort,cishiext->bToneType);
					mgw_scu_release_request(cishiext);
					cishiext->bPortState = MGW_CISHI_INIT_STATE;
					cishiext->bPortState1 = 0;
				}
			break;
	        default:
	        break;
		}
}

void	MGW_CISHI_RINGTONE_Process(TERMINAL_BASE* cishiext)
{
	switch(cishiext->bPortState1)
	{
			case 0:
				cishiext->wPortDelay = 600;
				cishiext->bToneType = TONE_RING;
				Send_TONE(cishiext->wPort,cishiext->bToneType);
				cishiext->bPortState1 = 1;
			break;
	
			case 1:
				if(cishiext->wPortDelay == 0 || cishiext->bCommand == MC_HANG)
				{
					cishiext->bToneType = TONE_STOP;
					Send_TONE(cishiext->wPort,cishiext->bToneType);
					cishiext->bPortState = MGW_CISHI_INIT_STATE;
					cishiext->bPortState1 = 1;
				}
	
				if(cishiext->bCommand == MC_TALK)
				{
					cishiext->bToneType = TONE_STOP;
					Send_TONE(cishiext->wPort,cishiext->bToneType);
					cishiext->bPortState = MGW_CISHI_TALK_STATE;
					cishiext->bPortState1 = 0;
					VERBOSE_OUT(LOG_SYS,"cishi%d goto talk!\n",cishiext->wPort);
					//sw2st_bus(cishiext->wPort-LOCALTER_OFFSET);
				}

			break;
	        default:
	        break;	
		}

}

void	MGW_CISHI_TALK_Process(TERMINAL_BASE* cishiext)
{
	switch(cishiext->bPortState1)
	{
		case 0:
			if(cishiext->bHookStatus == HOOK_ON)
			{
				VERBOSE_OUT(LOG_SYS,"cishi %d hangup in talk!\n",cishiext->wPort);
				mgw_scu_release_request(cishiext);
				cishiext->bPortState = MGW_CISHI_INIT_STATE;
				cishiext->bPortState1 = 1;
			}

			if(cishiext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"cishi %d recv hangup!\n",cishiext->wPort);
				cishiext->wPortDelay = 600;
				cishiext->bToneType = TONE_BUSY;
				Send_TONE(cishiext->wPort,cishiext->bToneType);
				cishiext->bPortState1 = 1;
			}

			if(cishiext->bPTTStatus == MGW_PTT_DOWN || cishiext->bPTTStatus == MGW_PTT_UP){
				VERBOSE_OUT(LOG_SYS,"Send PTT %s to SCU\n", \
					(cishiext->bPTTStatus == MGW_PTT_DOWN)?"Down":"Up");
				mgw_scu_ptt_request(cishiext);
				cishiext->bPTTStatus = MGW_PTT_HOLD;
			}
			
		break;

		case 1:
			if(cishiext->wPortDelay == 0)
			{
				cishiext->bToneType = TONE_STOP;
				Send_TONE(cishiext->wPort,cishiext->bToneType);
				/*发送释放请求*/
				cishiext->bPortState = MGW_CISHI_INIT_STATE;
				cishiext->bPortState1 = 1;
			}

			if(cishiext->bHookStatus == HOOK_ON)
			{
				mgw_scu_release_request(cishiext);
				cishiext->bToneType = TONE_STOP;
				Send_TONE(cishiext->wPort,cishiext->bToneType);
				VERBOSE_OUT(LOG_SYS,"cishi%d hangup!\n",cishiext->wPort);
				cishiext->bPortState = MGW_CISHI_INIT_STATE;
				cishiext->bPortState1 = 1;
			}
		break;
	    default:
		break;
	}
}

void	MGW_CISHI_RING_Process(TERMINAL_BASE* cishiext)
{
	switch(cishiext->bPortState1)
	{
		case 0:
			ring_cishi_on(cishiext);
			cishiext->wPortDelay = 50;
			cishiext->bPortState1 = 1;
			mgw_scu_callack_request(cishiext);
		break;

		case 1:
			if(cishiext->wPortDelay == 0)
			{
				ring_cishi_off(cishiext);
				cishiext->bPortState1 = 2;
				cishiext->wPortDelay = 600;
			}
		break;

		case 2:
			if(cishiext->bHookStatus == HOOK_OFF)
			{
				cishiext->bPortState = MGW_CISHI_TALK_STATE;
				cishiext->bPortState1 = 0;
				mgw_scu_connect_request(cishiext);
				VERBOSE_OUT(LOG_SYS,"cishi%d goto talk in ringing!\n",cishiext->wPort);
				cishiext->bCommand = MC_TALK;
				//sw2st_bus(cishiext->wPort-LOCALTER_OFFSET);
			}

			if(cishiext->wPortDelay == 0)
			{
				cishiext->bPortState = MGW_CISHI_INIT_STATE;
				cishiext->bPortState1 = 1;
				VERBOSE_OUT(LOG_SYS,"ring time up!\n");
			}
		break;

		default:
		break;
	}
}


void	MGW_Phone_INIT_Process(TERMINAL_BASE* phoneext)
{
	switch(phoneext->bPortState1)
	{
		case 1:
		case 0:
			phoneext->bPortState = MGW_PHONE_IDLE_STATE;
			phoneext->bPortState1 = 0;
			phoneext->wConnectPort = IDLEW;
			phoneext->wPortDelay = IDLEW;
			phoneext->bCommand = MC_IDLE;
			phoneext->bHookStatus = HOOK_ON;	/*note: different from trk port*/
			phoneext->phone_queue_g.bFinish = 0;
			phoneext->phone_queue_g.bPhoneQueueHead = 0;
			memset(phoneext->phone_queue_g.bPhoneQueue,0,MAXPHONENUM);
			memset(phoneext->phone_queue_s.bPhoneQueue,0,MAXPHONENUM);
			phoneext->bPTTStatus = MGW_PTT_INIT;
			phoneext->rel_reason = MGW_RELEASE_REASON_NORMAL;
			phoneext->zhuanxian_status = ZHUANXIAN_IDLE;
			Gpio_Ring_Set(phoneext->wPort, 0);
			sw2ac491(phoneext->wPort);
		break;
	    default:
		break;
	}
}

void	MGW_Phone_IDLE_Process(TERMINAL_BASE* phoneext)
{
	switch(phoneext->bPortState1)
	{
		case 0:
			if(phoneext->bHookStatus == HOOK_OFF)
			{
				phoneext->bPortState = MGW_PHONE_DIAL_STATE;
				VERBOSE_OUT(LOG_SYS,"phone %d is hook_off\n", phoneext->wPort - PHONE_OFFSET);
				phoneext->bPortState1 = 0;
			}
			
			if(phoneext->bCommand == MC_RING)
			{
				phoneext->bPortState = MGW_PHONE_RING_STATE;
				phoneext->bPortState1 = 0;
			}

			if(phoneext->bCommand == MC_INACTIVE)
			{
				phoneext->bPortState1 = 2;
			}
			break;

		case 2:
			if(phoneext->bCommand == MC_ACTIVE)
			{
				phoneext->bPortState = MGW_PHONE_INIT_STATE;
				phoneext->bPortState1 = 0;
			}
			break;

		default:
			phoneext->bPortState1 = 0;
			break;
	}
}


void	MGW_Phone_DIAL_Process(TERMINAL_BASE* phoneext)
{
	int dial_flg = 1;
	switch(phoneext->bPortState1)
	{
			case 0:
				phoneext->bToneType = TONE_DIAL;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				VERBOSE_OUT(LOG_SYS,"Send phone %d Tone Dial\n",phoneext->wPort - PHONE_OFFSET);
				phoneext->wPortDelay = 150;
				phoneext->bPortState1 = 1;
				phoneext->bCommand = MC_DIAL;
			break;
	
			case 1:
				if((phoneext->wPortDelay == 0)&&(phoneext->bToneType == TONE_DIAL))
				{
					dial_flg = 0;
					phoneext->bToneType = TONE_BUSY;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					phoneext->wPortDelay = 150;
					VERBOSE_OUT(LOG_SYS,"phone %d Dial Time is Up\n", phoneext->wPort);
					//phoneext->bPortState = MGW_PHONE_INIT_STATE;
					//phoneext->bPortState1 = 0;
				}

				if((phoneext->wPortDelay == 0)&&(phoneext->bToneType == TONE_BUSY))
				{
					phoneext->bToneType = TONE_STOP;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					VERBOSE_OUT(LOG_SYS,"phone %d busy Time is Up\n", phoneext->wPort);
					//phoneext->bPortState = MGW_PHONE_INIT_STATE;
					//phoneext->bPortState1 = 0;
				}				

				if(1 == dial_flg)
				{
					if(phoneext->phone_queue_g.bPhoneQueueHead == 1)
					{
						phoneext->bToneType = TONE_STOP;
						Send_TONE(phoneext->wPort,phoneext->bToneType);
					}
		
					if(phoneext->phone_queue_g.bFinish)
					{
						VERBOSE_OUT(LOG_SYS,"phone %d Send Call %s\n",phoneext->wPort,phoneext->phone_queue_g.bPhoneQueue);
						mgw_scu_call_request(phoneext);
						phoneext->wPortDelay = 600;
						phoneext->phone_queue_g.bFinish = 0;
						phoneext->phone_queue_g.bPhoneQueueHead = 0;
						memset(phoneext->phone_queue_g.bPhoneQueue,0,MAXPHONENUM);
						phoneext->bPortState1 = 2;
					}
				}

				if((phoneext->bHookStatus == HOOK_OFF))
				{
					if(phoneext->bCommand == MC_RING)
					{
						phoneext->bCommand = MC_REJECT;
						mgw_scu_callack_request(phoneext);
					}
				}
				
				if(phoneext->bHookStatus == HOOK_ON)
				{
					phoneext->bToneType = TONE_STOP;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					
					VERBOSE_OUT(LOG_SYS,"phone %d hangup!\n",phoneext->wPort);
					phoneext->phone_queue_g.bFinish = 0;
					phoneext->phone_queue_g.bPhoneQueueHead = 0;
					//strcpy(phoneext->phone_queue_g.bPhoneQueue,"");
					strncpy((char *)phoneext->phone_queue_g.bPhoneQueue,"", MAXPHONENUM);

					mgw_scu_release_request(phoneext);
					
					phoneext->bPortState = MGW_PHONE_INIT_STATE;
					phoneext->bPortState1 = 0;
				}
			break;
	
			case 2:
				if(phoneext->wPortDelay == 0)
				{
					//phoneext->bToneType = TONE_BUSY;
					//Send_TONE(phoneext->wPort,phoneext->bToneType);
					
					phoneext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
					mgw_scu_release_request(phoneext);
					phoneext->bPortState = MGW_PHONE_INIT_STATE;
					phoneext->bPortState1 = 0;
				}

				if(phoneext->bHookStatus == HOOK_ON)
				{	
					VERBOSE_OUT(LOG_SYS,"phone %d hangup in dial!\n",phoneext->wPort);
					mgw_scu_release_request(phoneext);
					phoneext->bPortState = MGW_PHONE_INIT_STATE;
					phoneext->bPortState1 = 0;
				}

	
				if(phoneext->bCommand == MC_RT_TONE)
				{
					phoneext->bPortState = MGW_PHONE_RINGTONE_STATE;
					phoneext->bPortState1 = 0;

					VERBOSE_OUT(LOG_SYS,"phone%d goto ringtone!\n",phoneext->wPort);
				}

				/* radio 直呼功能 guoxiaorong add 20140730*/
				if(phoneext->bCommand == MC_TALK)
				{
					phoneext->bToneType = TONE_STOP;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					phoneext->bPortState = MGW_PHONE_TALK_STATE;
					phoneext->bPortState1 = 0;
					VERBOSE_OUT(LOG_SYS,"phone%d goto talk!\n",phoneext->wPort);
					sw2st_bus(phoneext->wPort);
				}
				
				//printf("phoneext->bCommand = %d\r\n", phoneext->bCommand);
				if(phoneext->bCommand == MC_REJECT)
				{
					phoneext->wPortDelay = 150;
					phoneext->bToneType = TONE_BUSY;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					//mgw_scu_release_request(phoneext);
					//phoneext->bPortState = MGW_PHONE_INIT_STATE;
					phoneext->bPortState1 = 3;
				}

				if(phoneext->bCommand == MC_NONUM)
				{
					phoneext->wPortDelay = 150;
					phoneext->bToneType = TONE_BUSY;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					//mgw_scu_release_request(phoneext);
					//phoneext->bPortState = MGW_PHONE_INIT_STATE;
					phoneext->bPortState1 = 3;
				}				

				if(phoneext->bHookStatus == HOOK_ON)
				{
					phoneext->bToneType = TONE_STOP;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					
					VERBOSE_OUT(LOG_SYS,"phone %d hangup!\n",phoneext->wPort);
					phoneext->phone_queue_g.bFinish = 0;
					phoneext->phone_queue_g.bPhoneQueueHead = 0;
					//strcpy(phoneext->phone_queue_g.bPhoneQueue,"");
					strncpy((char *)phoneext->phone_queue_g.bPhoneQueue,"", MAXPHONENUM);

					mgw_scu_release_request(phoneext);
					
					phoneext->bPortState = MGW_PHONE_INIT_STATE;
					phoneext->bPortState1 = 0;
				}				
				
			break;
			case 3:
					if(0 == phoneext->wPortDelay)
					{
						phoneext->bToneType = TONE_STOP;
						Send_TONE(phoneext->wPort,phoneext->bToneType);
					}

					if(phoneext->bHookStatus == HOOK_ON)
					{
						phoneext->bToneType = TONE_STOP;
						Send_TONE(phoneext->wPort,phoneext->bToneType);
						
						VERBOSE_OUT(LOG_SYS,"phone %d hangup!\n",phoneext->wPort);
						phoneext->phone_queue_g.bFinish = 0;
						phoneext->phone_queue_g.bPhoneQueueHead = 0;
						//strcpy(phoneext->phone_queue_g.bPhoneQueue,"");
						strncpy((char *)phoneext->phone_queue_g.bPhoneQueue,"", MAXPHONENUM);

						mgw_scu_release_request(phoneext);
						
						phoneext->bPortState = MGW_PHONE_INIT_STATE;
						phoneext->bPortState1 = 0;
					}					
			break;
    	    default:
			break;
		}
}

void	MGW_Phone_RINGTONE_Process(TERMINAL_BASE* phoneext)
{
	switch(phoneext->bPortState1)
	{
			case 0:
				phoneext->wPortDelay = 600;
				phoneext->bToneType = TONE_RING;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				phoneext->bPortState1 = 1;
			break;
	
			case 1:
				if(phoneext->wPortDelay == 0 || phoneext->bCommand == MC_HANG)
				{
#if 0				
					phoneext->bToneType = TONE_STOP;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					phoneext->bPortState = MGW_PHONE_INIT_STATE;
					phoneext->bPortState1 = 1;
#endif

					sw2ac491(phoneext->wPort);
					phoneext->wPortDelay = 300;
					phoneext->bToneType = TONE_BUSY;
					Send_TONE(phoneext->wPort, phoneext->bToneType);
					phoneext->bPortState1 = 2;
				}

				if(phoneext->bHookStatus == HOOK_ON)
				{	
					VERBOSE_OUT(LOG_SYS,"phone %d hangup in ringtone!\n",phoneext->wPort);
					mgw_scu_release_request(phoneext);
					phoneext->bPortState = MGW_PHONE_INIT_STATE;
					phoneext->bPortState1 = 0;
				}
					
				if(phoneext->bCommand == MC_TALK)
				{
					phoneext->bToneType = TONE_STOP;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					phoneext->bPortState = MGW_PHONE_TALK_STATE;
					phoneext->bPortState1 = 0;
					VERBOSE_OUT(LOG_SYS,"phone%d goto talk!\n",phoneext->wPort);
					sw2st_bus(phoneext->wPort);
				}

			break;
			case 2:
				if(phoneext->wPortDelay == 0)
				{
					phoneext->bToneType = TONE_STOP;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					/*发送释放请求*/
					//mgw_scu_release_request(phoneext);
					//phoneext->bPortState = MGW_PHONE_INIT_STATE;
					//phoneext->bPortState1 = 1;
				}

				if(phoneext->bHookStatus == HOOK_ON)
				{
					mgw_scu_release_request(phoneext);
					phoneext->bToneType = TONE_STOP;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					VERBOSE_OUT(LOG_SYS,"phone%d hangup!\n",phoneext->wPort);
					phoneext->bPortState = MGW_PHONE_INIT_STATE;
					phoneext->bPortState1 = 1;
				}

			break;			
		    default:
	        break;
	
		}

}

void	MGW_Phone_TALK_Process(TERMINAL_BASE* phoneext)
{
	switch(phoneext->bPortState1)
	{
		case 0:
			if(phoneext->bHookStatus == HOOK_ON)
			{
				VERBOSE_OUT(LOG_SYS,"phone %d hangup in talk!\n",phoneext->wPort);
				mgw_scu_release_request(phoneext);
				phoneext->bPortState = MGW_PHONE_INIT_STATE;
				phoneext->bPortState1 = 1;
			}

			if(phoneext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"phone %d recv hangup!\n",phoneext->wPort);
				sw2ac491(phoneext->wPort);
				phoneext->wPortDelay = 300;
				phoneext->bToneType = TONE_BUSY;
				Send_TONE(phoneext->wPort, phoneext->bToneType);
				phoneext->bPortState1 = 1;
			}

			if(phoneext->bPTTStatus == MGW_PTT_DOWN || phoneext->bPTTStatus == MGW_PTT_UP){
				VERBOSE_OUT(LOG_SYS,"Send PTT %s to SCU\n", \
					(phoneext->bPTTStatus == MGW_PTT_DOWN)?"Down":"Up");
				mgw_scu_ptt_request(phoneext);
				phoneext->bPTTStatus = MGW_PTT_HOLD;
			}
			
		break;

		case 1:
			if(phoneext->wPortDelay == 0)
			{
				phoneext->bToneType = TONE_STOP;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				/*发送释放请求*/
				//mgw_scu_release_request(phoneext);
				//phoneext->bPortState = MGW_PHONE_INIT_STATE;
				//phoneext->bPortState1 = 1;
			}

			if(phoneext->bHookStatus == HOOK_ON)
			{
				mgw_scu_release_request(phoneext);
				phoneext->bToneType = TONE_STOP;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				VERBOSE_OUT(LOG_SYS,"phone%d hangup!\n",phoneext->wPort);
				phoneext->bPortState = MGW_PHONE_INIT_STATE;
				phoneext->bPortState1 = 1;
			}
			else if((phoneext->bHookStatus == HOOK_OFF))
			{
				if(phoneext->bCommand == MC_RING)
				{
					phoneext->bCommand = MC_REJECT;
					mgw_scu_callack_request(phoneext);
				}
			}

		break;
   	    default:
	    break;		
	}
}

void	MGW_Phone_RING_Process(TERMINAL_BASE* phoneext)
{
	switch(phoneext->bPortState1)
	{
		case 0:
			Gpio_Ring_Set(phoneext->wPort, 1);
//			phoneext->wPortDelay = 50;
//			phoneext->bPortState1 = 1;
			phoneext->wPortDelay = 600;
			phoneext->bPortState1 = 2;

			mgw_scu_callack_request(phoneext);
		break;
#if 0
		case 1:
			if(phoneext->wPortDelay == 0)
			{
				//Gpio_Ring_Set(phoneext->wPort, 0);
				phoneext->bPortState1 = 2;
				phoneext->wPortDelay = 600;
			}
		break;
#endif
		case 2:
		
			if(phoneext->bCommand == MC_HANG)
			{
				Gpio_Ring_Set(phoneext->wPort, 0);
				phoneext->bPortState = MGW_PHONE_INIT_STATE;
				phoneext->bPortState1 = 1;
				mgw_scu_release_cnf_request(phoneext);
				VERBOSE_OUT(LOG_SYS," zhujiao hangup!\n");
			}
			
			if(phoneext->bHookStatus == HOOK_OFF)
			{
				Gpio_Ring_Set(phoneext->wPort, 0);
				phoneext->bPortState = MGW_PHONE_TALK_STATE;
				phoneext->bPortState1 = 0;
				mgw_scu_connect_request(phoneext);
				VERBOSE_OUT(LOG_SYS,"phone%d goto talk in ringing!\n",phoneext->wPort);
				phoneext->bCommand = MC_TALK;
				sw2st_bus(phoneext->wPort);
			}

			if(phoneext->wPortDelay == 0)
			{
				Gpio_Ring_Set(phoneext->wPort, 0);
				phoneext->bPortState = MGW_PHONE_INIT_STATE;
				phoneext->bPortState1 = 1;
				VERBOSE_OUT(LOG_SYS,"ring time up!\n");
			}
		break;

		default:
		break;
	}
}


void	MGW_XIWEI_INIT_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 1:
		case 0:
			xiweiext->bPortState = MGW_XIWEI_IDLE_STATE;
			xiweiext->bPortState1 = 0;
			xiweiext->wConnectPort = IDLEW;
			xiweiext->wPortDelay = IDLEW;
			xiweiext->bCommand = MC_IDLE;
			xiweiext->bHookStatus = HOOK_ON;	/*note: different from trk port*/
			xiweiext->bPTTStatus = MGW_PTT_INIT;
			xiweiext->bresv = IDLE;				/*go to init state in radio direct call*/
			xiweiext->rel_reason = MGW_RELEASE_REASON_NORMAL;
			xiweiext->zhuanxian_status = ZHUANXIAN_IDLE;
			if(xiweiext->bPortType == PORT_TYPE_QINWU)
			{
				sw2ac491(xiweiext->wPort);
			}
			
			break;
			default:
			break;
	}
}

void	MGW_XIWEI_IDLE_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			if(xiweiext->bHookStatus == HOOK_OFF)
			{
				xiweiext->bPortState = MGW_XIWEI_DIAL_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(xiweiext->bCommand == MC_RING)
			{
				xiweiext->bPortState = MGW_XIWEI_RING_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(xiweiext->bCommand == MC_INACTIVE)
				xiweiext->bPortState1 = 2;
			
			break;

		case 2:
			if(xiweiext->bCommand == MC_ACTIVE)
			{
				xiweiext->bPortState = MGW_XIWEI_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}
			break;

		default:
			xiweiext->bPortState1 = 0;
			break;
	}
}

void	MGW_XIWEI_DIAL_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			xiweiext->bPortState1 = 1;
			xiweiext->wPortDelay = 300;
			break;

		case 1:
			if(xiweiext->phone_queue_g.bFinish)
			{
				VERBOSE_OUT(LOG_SYS,"send to scu call request!\n");
				mgw_scu_call_request(xiweiext);
				DEBUG_OUT("xiweiext%d slot = %d\n",xiweiext->wPort,xiweiext->bslot_us);
			}
			xiweiext->bPortState1 = 2;
			break;

		case 2:
			if(xiweiext->bCommand == MC_RT_TONE)
			{
				mgw_ext_callack_notify(xiweiext);
				xiweiext->bPortState = MGW_XIWEI_RINGTONE_STATE;
				xiweiext->bPortState1 = 0;

				if(xiweiext->bPortType == PORT_TYPE_QINWU)
				{
					VERBOSE_OUT(LOG_SYS,"Qinwu recv call normal ack!\n");
					//xiweiext->bToneType = TONE_RING;
					//Send_TONE(xiweiext->wPort,xiweiext->bToneType);
				}
				
			}

			if(xiweiext->bCommand == MC_REJECT)
			{
				VERBOSE_OUT(LOG_SYS,"Qinwu recv call reject ack !\n");
				mgw_ext_callack_notify(xiweiext);
				mgw_scu_release_request(xiweiext);
				xiweiext->bPortState = MGW_XIWEI_INIT_STATE;
				xiweiext->bPortState1 = 0;	
				//xiweiext->wPortDelay == 100;
			}

			if(xiweiext->bHookStatus == HOOK_ON || xiweiext->wPortDelay == 0)
			{
				VERBOSE_OUT(LOG_SYS,"xiwei %d hangup! OR Time Out\n",xiweiext->wPort);
				if(xiweiext->wPortDelay==0)
				{
					xiweiext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				}
				
				mgw_scu_release_request(xiweiext);
				mgw_ext_release_cnf_notify(xiweiext);
				xiweiext->bPortState = MGW_XIWEI_INIT_STATE;
				xiweiext->bPortState1 = 0;	
				//xiweiext->wPortDelay == 100;
			}

			if(xiweiext->bCommand == MC_TALK)
			{
				VERBOSE_OUT(LOG_SYS,"XIWEI%d goto talk in dial state!\n",xiweiext->wPort);
				mgw_ext_callack_notify(xiweiext);
				/*键显板不能对快速的信令及时作出动作，故延时必要的间隔*/
				xiweiext->wPortDelay = 2;
				xiweiext->bPortState1 = 3;
			}
			break;
		case 3:
			if(xiweiext->wPortDelay == 0)
			{
				mgw_ext_connect_notify(xiweiext);
				xiweiext->bPortState = MGW_XIWEI_TALK_STATE;
				xiweiext->bPortState1 = 0;

				//xiweiext->rtp_socket.remote_port = xiweiext->rtp_socket.local_port;
				xiweiext->rtp_socket.remote_port = get_random_port();

				VERBOSE_OUT(LOG_SYS,"Recv Xiwei connect msg,bind %d!\n",xiweiext->rtp_socket.local_port);

				if(xiweiext->bPortType == PORT_TYPE_QINWU)
				{
					sw2st_bus(xiweiext->wPort);	
				}
				
				if(xiweiext->bPortType != PORT_TYPE_XIWEI)
					break;
				/*
				if(socket_new_with_bindaddr(&xiweiext->rtp_socket)<0)
					VERBOSE_OUT(LOG_SYS,"Create xiwei Socket 0 Error:%s",strerror(errno));
				else
					xiweiext->rtp_io_id = rtp_io_add(xiweiext->rtp_socket.udp_handle,xiweiext);
				*/
				if(socket_new(&xiweiext->rtp_socket)<0)
					VERBOSE_OUT(LOG_SYS,"Create xiwei Socket 0 Error:%s",strerror(errno));	
			}
			break;
			default:
	        break;	
	}
}

void	MGW_XIWEI_RINGTONE_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			xiweiext->wPortDelay = 600;
			xiweiext->bPortState1 = 1;
			break;

		case 1:
			if(xiweiext->bCommand == MC_TALK)
			{
				mgw_ext_connect_notify(xiweiext);
				xiweiext->bPortState = MGW_XIWEI_TALK_STATE;
				xiweiext->bPortState1 = 0;

				//xiweiext->rtp_socket.remote_port = xiweiext->rtp_socket.local_port;
				xiweiext->rtp_socket.remote_port = get_random_port();

				VERBOSE_OUT(LOG_SYS,"Recv Xiwei connect msg,bind %d!\n",xiweiext->rtp_socket.local_port);

				if(xiweiext->bPortType == PORT_TYPE_QINWU)
				{
					xiweiext->bToneType = TONE_STOP;
					Send_TONE(xiweiext->wPort,xiweiext->bToneType);
					
					sw2st_bus(xiweiext->wPort);
				}
				
				if(xiweiext->bPortType != PORT_TYPE_XIWEI)
				{
					break;
				}
				/*
				if(socket_new_with_bindaddr(&xiweiext->rtp_socket)<0)
					VERBOSE_OUT(LOG_SYS,"Create xiwei Socket 0 Error:%s",strerror(errno));
				else
					xiweiext->rtp_io_id = rtp_io_add(xiweiext->rtp_socket.udp_handle,xiweiext);
				*/
				if(socket_new(&xiweiext->rtp_socket)<0)
					VERBOSE_OUT(LOG_SYS,"Create xiwei Socket 0 Error:%s",strerror(errno));
			}

			if(xiweiext->bHookStatus == HOOK_ON)
			{
				if(xiweiext->bPortType == PORT_TYPE_QINWU)
				{
					xiweiext->bToneType = TONE_STOP;
					Send_TONE(xiweiext->wPort,xiweiext->bToneType);
				}			
				
				mgw_scu_release_request(xiweiext);
				mgw_ext_release_cnf_notify(xiweiext);
				xiweiext->bPortState = MGW_XIWEI_INIT_STATE;
				xiweiext->bPortState1 = 0;				
			}

			if(xiweiext->bCommand == MC_HANG)
			{
				if(xiweiext->bPortType == PORT_TYPE_QINWU)
				{
					xiweiext->bToneType = TONE_STOP;
					Send_TONE(xiweiext->wPort,xiweiext->bToneType);
				}
				
				VERBOSE_OUT(LOG_SYS,"xiwei%d Recv Hangup Msg! OR Time Out\n",xiweiext->wPort);
				mgw_scu_release_cnf_request(xiweiext);
				mgw_ext_release_notify(xiweiext);
				xiweiext->bPortState = MGW_XIWEI_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}
			
			if(xiweiext->wPortDelay == 0)
			{
				xiweiext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				mgw_scu_release_request(xiweiext);
				mgw_ext_release_notify(xiweiext);
				xiweiext->bPortState = MGW_XIWEI_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}
			break;
			default:
	        break;	
	}
}

void	MGW_XIWEI_RING_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			xiweiext->wPortDelay = 600;
			xiweiext->bPortState1 = 1;
			mgw_ext_call_notify(xiweiext);
			break;

		case 1:
			if(xiweiext->bHookStatus == HOOK_OFF)
			{
				mgw_scu_connect_request(xiweiext);
				
				xiweiext->rtp_socket.remote_port = xiweiext->rtp_socket.local_port;
				xiweiext->rtp_socket.remote_port = get_random_port();
				xiweiext->bPortState = MGW_XIWEI_TALK_STATE;
				xiweiext->bPortState1 = 0;
				xiweiext->bCommand = MC_TALK;

				VERBOSE_OUT(LOG_SYS,"Recv Xiwei connect msg,bind %d!\n",xiweiext->rtp_socket.local_port);

				if(xiweiext->bPortType == PORT_TYPE_QINWU)
				{
					sw2st_bus(xiweiext->wPort);	
					break;
				}
				if(xiweiext->bPortType != PORT_TYPE_XIWEI)/*判断为席位用户后才建立RTP通道*/
				{	
					break;
				}
				/*
				if(socket_new_with_bindaddr(&xiweiext->rtp_socket) < 0)
					VERBOSE_OUT(LOG_SYS,"Create xiwei 1 Socket Error:%s\n",strerror(errno));
				else
					xiweiext->rtp_io_id = rtp_io_add(xiweiext->rtp_socket.udp_handle,xiweiext);
				*/
				if(socket_new(&xiweiext->rtp_socket)<0)
					VERBOSE_OUT(LOG_SYS,"Create xiwei Socket 0 Error:%s",strerror(errno));				
			}

			if(xiweiext->bCommand == MC_HANG || xiweiext->wPortDelay == 0)
			{
				VERBOSE_OUT(LOG_SYS,"xiwei%d Recv Hangup Msg! OR Time Out\n",xiweiext->wPort);
				mgw_scu_release_cnf_request(xiweiext);
				mgw_ext_release_notify(xiweiext);
				xiweiext->bPortState = MGW_XIWEI_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(MC_HANG_ACTIVE == xiweiext->bCommand)
			{
				mgw_ext_release_notify(xiweiext);
				xiweiext->bPortState = MGW_XIWEI_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}
			
			break;
			default:
			break;
	}
}

void	MGW_XIWEI_TALK_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			if(xiweiext->bHookStatus == HOOK_ON)
			{
				xiweiext->wPortDelay = 0;
				mgw_scu_release_request(xiweiext);
				mgw_ext_release_cnf_notify(xiweiext);/*释放证实*/
				xiweiext->bPortState1 = 1;
			}

			if(xiweiext->bCommand == MC_TRANS)
			{
				mgw_scu_calltrans_request(xiweiext);
				mgw_ext_release_notify(xiweiext);
				xiweiext->bPortState1 = 1;
			}

			if(xiweiext->bCommand == MC_HANG)
			{
				if(xiweiext->bPortType == PORT_TYPE_QINWU)
				{
					//sw2ac491(xiweiext->wPort);
					//xiweiext->wPortDelay = 100;
					//xiweiext->bToneType = TONE_BUSY;
					//Send_TONE(xiweiext->wPort, xiweiext->bToneType);
					xiweiext->wPortDelay = 0;
				}
				
				mgw_scu_release_cnf_request(xiweiext);
				mgw_ext_release_notify(xiweiext);
				xiweiext->bPortState1 = 1;
			}

			if(xiweiext->bPTTStatus == MGW_PTT_DOWN || xiweiext->bPTTStatus == MGW_PTT_UP)
			{
				VERBOSE_OUT(LOG_SYS,"Send PTT %s to SCU\n", \
					(xiweiext->bPTTStatus == MGW_PTT_DOWN)?"Down":"Up");
				/*fix repeat ptt msg error -- swf 2011-10-03*/
				if(xiweiext->bPortType == PORT_TYPE_QINWU)
				{
					mgw_scu_ptt_request(xiweiext);
				}
				
				xiweiext->bPTTStatus = MGW_PTT_HOLD;
			}
			
			break;

		case 1:

			if(xiweiext->wPortDelay == 0)
			{
				xiweiext->bToneType = TONE_STOP;
				Send_TONE(xiweiext->wPort, xiweiext->bToneType);
					
				xiweiext->bPortState = MGW_XIWEI_INIT_STATE;
				xiweiext->bPortState1 = 1;
				rtp_io_remove(xiweiext->rtp_io_id);
				if(xiweiext->rtp_socket.udp_handle > 0)
					close(xiweiext->rtp_socket.udp_handle);
			}
			break;
			default:
	        break;			
	}
}


void	MGW_LOCAL_XIWEI_INIT_Process(TERMINAL_BASE* phoneext)
{
	switch(phoneext->bPortState1)
	{
		case 1:
		case 0:
			phoneext->bPortState = MGW_LOCAL_XIWEI_IDLE_STATE;
			phoneext->bPortState1 = 0;
			phoneext->wConnectPort = IDLEW;
			phoneext->wPortDelay = IDLEW;
			phoneext->bCommand = MC_IDLE;
			phoneext->bHookStatus = HOOK_ON;	/*note: different from trk port*/
			phoneext->phone_queue_g.bFinish = 0;
			phoneext->phone_queue_g.bPhoneQueueHead = 0;
			memset(phoneext->phone_queue_g.bPhoneQueue,0,MAXPHONENUM);
			memset(phoneext->phone_queue_s.bPhoneQueue,0,MAXPHONENUM);
			phoneext->bPTTStatus = MGW_PTT_INIT;
			phoneext->rel_reason = MGW_RELEASE_REASON_NORMAL;
			phoneext->zhuanxian_status = ZHUANXIAN_IDLE;
			sw2ac491(phoneext->wPort);
		break;
			default:
		break;
	}
}

void	MGW_LOCAL_XIWEI_IDLE_Process(TERMINAL_BASE* phoneext)
{
	switch(phoneext->bPortState1)
	{
		case 0:
			if(phoneext->bHookStatus == HOOK_OFF)
			{
				phoneext->bPortState = MGW_LOCAL_XIWEI_DIAL_STATE;
				VERBOSE_OUT(LOG_SYS,"phone %d is hook_off\n", phoneext->wPort - PHONE_OFFSET);
				phoneext->bPortState1 = 0;
			}
			
			if(phoneext->bCommand == MC_RING)
			{
				phoneext->bPortState = MGW_LOCAL_XIWEI_RING_STATE;
				phoneext->bPortState1 = 0;
			}

			if(phoneext->bCommand == MC_INACTIVE)
			{
				phoneext->bPortState1 = 2;
			}
			break;

		case 2:
			if(phoneext->bCommand == MC_ACTIVE)
			{
				phoneext->bPortState = MGW_LOCAL_XIWEI_INIT_STATE;
				phoneext->bPortState1 = 0;
			}
			break;

		default:
			phoneext->bPortState1 = 0;
			break;
	}
}


void	MGW_LOCAL_XIWEI_DIAL_Process(TERMINAL_BASE* phoneext)
{
	int dial_flg = 1;
	switch(phoneext->bPortState1)
	{
			case 0:
				phoneext->bToneType = TONE_DIAL;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				VERBOSE_OUT(LOG_SYS,"Send phone %d Tone Dial\n",phoneext->wPort - PHONE_OFFSET);
				phoneext->wPortDelay = 150;
				phoneext->bPortState1 = 1;
				phoneext->bCommand = MC_DIAL;
			break;
	
			case 1:
				if((phoneext->wPortDelay == 0)&&(phoneext->bToneType == TONE_DIAL))
				{
					dial_flg = 0;
					phoneext->bToneType = TONE_BUSY;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					phoneext->wPortDelay = 150;
					VERBOSE_OUT(LOG_SYS,"phone %d Dial Time is Up\n", phoneext->wPort);
					//phoneext->bPortState = MGW_PHONE_INIT_STATE;
					//phoneext->bPortState1 = 0;
				}

				if((phoneext->wPortDelay == 0)&&(phoneext->bToneType == TONE_BUSY))
				{
					phoneext->bToneType = TONE_STOP;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					VERBOSE_OUT(LOG_SYS,"phone %d busy Time is Up\n", phoneext->wPort);
					//phoneext->bPortState = MGW_PHONE_INIT_STATE;
					//phoneext->bPortState1 = 0;
				}				

				if(1 == dial_flg)
				{
					if(phoneext->phone_queue_g.bPhoneQueueHead == 1)
					{
						phoneext->bToneType = TONE_STOP;
						Send_TONE(phoneext->wPort,phoneext->bToneType);
					}
		
					if(phoneext->phone_queue_g.bFinish)
					{
						VERBOSE_OUT(LOG_SYS,"phone %d Send Call %s\n",phoneext->wPort,phoneext->phone_queue_g.bPhoneQueue);
						//mgw_scu_call_request(phoneext);
						mgw_local_seat_call_request(phoneext);
						phoneext->wPortDelay = 600;
						phoneext->phone_queue_g.bFinish = 0;
						phoneext->phone_queue_g.bPhoneQueueHead = 0;
						memset(phoneext->phone_queue_g.bPhoneQueue,0,MAXPHONENUM);
						phoneext->bPortState1 = 2;
					}
				}

				if((phoneext->bHookStatus == HOOK_OFF))
				{
					if(phoneext->bCommand == MC_RING)
					{
						phoneext->bCommand = MC_REJECT;
						mgw_local_seat_callack_request(phoneext);
					}
				}
				
				if(phoneext->bHookStatus == HOOK_ON)
				{
					phoneext->bToneType = TONE_STOP;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					
					VERBOSE_OUT(LOG_SYS,"phone %d hangup!\n",phoneext->wPort);
					phoneext->phone_queue_g.bFinish = 0;
					phoneext->phone_queue_g.bPhoneQueueHead = 0;
					//strcpy(phoneext->phone_queue_g.bPhoneQueue,"");
					strncpy((char *)phoneext->phone_queue_g.bPhoneQueue,"", MAXPHONENUM);

					mgw_local_seat_release_request(phoneext);
					
					phoneext->bPortState = MGW_LOCAL_XIWEI_INIT_STATE;
					phoneext->bPortState1 = 0;
				}
			break;
	
			case 2:
				if(phoneext->wPortDelay == 0)
				{
					//phoneext->bToneType = TONE_BUSY;
					//Send_TONE(phoneext->wPort,phoneext->bToneType);
					
					phoneext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
					mgw_local_seat_release_request(phoneext);
					phoneext->bPortState = MGW_LOCAL_XIWEI_INIT_STATE;
					phoneext->bPortState1 = 0;
				}

				if(phoneext->bHookStatus == HOOK_ON)
				{	
					VERBOSE_OUT(LOG_SYS,"phone %d hangup in dial!\n",phoneext->wPort);
					mgw_local_seat_release_request(phoneext);
					phoneext->bPortState = MGW_LOCAL_XIWEI_INIT_STATE;
					phoneext->bPortState1 = 0;
				}

	
				if(phoneext->bCommand == MC_RT_TONE)
				{
					phoneext->bPortState = MGW_LOCAL_XIWEI_RINGTONE_STATE;
					phoneext->bPortState1 = 0;

					VERBOSE_OUT(LOG_SYS,"phone%d goto ringtone!\n",phoneext->wPort);
				}

				/* radio 直呼功能 guoxiaorong add 20140730*/
				if(phoneext->bCommand == MC_TALK)
				{
					phoneext->bToneType = TONE_STOP;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					phoneext->bPortState = MGW_LOCAL_XIWEI_TALK_STATE;
					phoneext->bPortState1 = 0;
					VERBOSE_OUT(LOG_SYS,"phone%d goto talk!\n",phoneext->wPort);
					//sw2st_bus(phoneext->wPort);
				}
				
				//printf("phoneext->bCommand = %d\r\n", phoneext->bCommand);
				if(phoneext->bCommand == MC_REJECT)
				{
					phoneext->wPortDelay = 150;
					phoneext->bToneType = TONE_BUSY;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					//mgw_scu_release_request(phoneext);
					//phoneext->bPortState = MGW_PHONE_INIT_STATE;
					phoneext->bPortState1 = 3;
				}

				if(phoneext->bHookStatus == HOOK_ON)
				{
					phoneext->bToneType = TONE_STOP;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					
					VERBOSE_OUT(LOG_SYS,"phone %d hangup!\n",phoneext->wPort);
					phoneext->phone_queue_g.bFinish = 0;
					phoneext->phone_queue_g.bPhoneQueueHead = 0;
					//strcpy(phoneext->phone_queue_g.bPhoneQueue,"");
					strncpy((char *)phoneext->phone_queue_g.bPhoneQueue,"", MAXPHONENUM);

					mgw_local_seat_release_request(phoneext);
					
					phoneext->bPortState = MGW_LOCAL_XIWEI_INIT_STATE;
					phoneext->bPortState1 = 0;
				}				
				
			break;
			case 3:
					if(0 == phoneext->wPortDelay)
					{
						phoneext->bToneType = TONE_STOP;
						Send_TONE(phoneext->wPort,phoneext->bToneType);
					}

					if(phoneext->bHookStatus == HOOK_ON)
					{
						phoneext->bToneType = TONE_STOP;
						Send_TONE(phoneext->wPort,phoneext->bToneType);
						
						VERBOSE_OUT(LOG_SYS,"phone %d hangup!\n",phoneext->wPort);
						phoneext->phone_queue_g.bFinish = 0;
						phoneext->phone_queue_g.bPhoneQueueHead = 0;
						//strcpy(phoneext->phone_queue_g.bPhoneQueue,"");
						strncpy((char *)phoneext->phone_queue_g.bPhoneQueue,"", MAXPHONENUM);

						mgw_local_seat_release_request(phoneext);
						
						phoneext->bPortState = MGW_LOCAL_XIWEI_INIT_STATE;
						phoneext->bPortState1 = 0;
					}					
			break;
			default:
	        break;			
		}
}

void	MGW_LOCAL_XIWEI_RINGTONE_Process(TERMINAL_BASE* phoneext)
{
	switch(phoneext->bPortState1)
	{
			case 0:
				phoneext->wPortDelay = 600;
				phoneext->bToneType = TONE_RING;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				phoneext->bPortState1 = 1;
			break;
	
			case 1:
				if(phoneext->wPortDelay == 0 || phoneext->bCommand == MC_HANG)
				{
					phoneext->bToneType = TONE_STOP;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					phoneext->bPortState = MGW_LOCAL_XIWEI_INIT_STATE;
					phoneext->bPortState1 = 1;
				}


				if(phoneext->bHookStatus == HOOK_ON)
				{	
					VERBOSE_OUT(LOG_SYS,"phone %d hangup in ringtone!\n",phoneext->wPort);
					mgw_local_seat_release_request(phoneext);
					phoneext->bPortState = MGW_LOCAL_XIWEI_INIT_STATE;
					phoneext->bPortState1 = 0;
				}
					
				if(phoneext->bCommand == MC_TALK)
				{
					phoneext->bToneType = TONE_STOP;
					Send_TONE(phoneext->wPort,phoneext->bToneType);
					phoneext->bPortState = MGW_LOCAL_XIWEI_TALK_STATE;
					phoneext->bPortState1 = 0;
					VERBOSE_OUT(LOG_SYS,"phone%d goto talk!\n",phoneext->wPort);
					//sw2st_bus(phoneext->wPort);
				}

			break;
			default:
	        break;			
	
		}

}

void	MGW_LOCAL_XIWEI_TALK_Process(TERMINAL_BASE* phoneext)
{
	switch(phoneext->bPortState1)
	{
		case 0:
			if(phoneext->bHookStatus == HOOK_ON)
			{
				VERBOSE_OUT(LOG_SYS,"phone %d hangup in talk!\n",phoneext->wPort);
				mgw_local_seat_release_request(phoneext);
				phoneext->bPortState = MGW_LOCAL_XIWEI_INIT_STATE;
				phoneext->bPortState1 = 1;
			}

			if(phoneext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"phone %d recv hangup!\n",phoneext->wPort);
				sw2ac491(phoneext->wPort);
				phoneext->wPortDelay = 300;
				phoneext->bToneType = TONE_BUSY;
				Send_TONE(phoneext->wPort, phoneext->bToneType);
				phoneext->bPortState1 = 1;
			}

			if(phoneext->bPTTStatus == MGW_PTT_DOWN || phoneext->bPTTStatus == MGW_PTT_UP){
				VERBOSE_OUT(LOG_SYS,"Send PTT %s to SCU\n", \
					(phoneext->bPTTStatus == MGW_PTT_DOWN)?"Down":"Up");
				mgw_local_seat_ptt_request(phoneext);
				phoneext->bPTTStatus = MGW_PTT_HOLD;
			}
			
		break;

		case 1:
			if(phoneext->wPortDelay == 0)
			{
				phoneext->bToneType = TONE_STOP;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				/*发送释放请求*/
				//mgw_scu_release_request(phoneext);
				//phoneext->bPortState = MGW_PHONE_INIT_STATE;
				//phoneext->bPortState1 = 1;
			}

			if(phoneext->bHookStatus == HOOK_ON)
			{
				mgw_local_seat_release_request(phoneext);
				phoneext->bToneType = TONE_STOP;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				VERBOSE_OUT(LOG_SYS,"phone%d hangup!\n",phoneext->wPort);
				phoneext->bPortState = MGW_LOCAL_XIWEI_INIT_STATE;
				phoneext->bPortState1 = 1;
			}
			else if((phoneext->bHookStatus == HOOK_OFF))
			{
				if(phoneext->bCommand == MC_RING)
				{
					phoneext->bCommand = MC_REJECT;
					mgw_local_seat_callack_request(phoneext);
				}
			}

		break;
			default:
	        break;		
	}
}

void	MGW_LOCAL_XIWEI_RING_Process(TERMINAL_BASE* phoneext)
{
	switch(phoneext->bPortState1)
	{
		case 0:
			Gpio_Ring_Set(phoneext->wPort, 1);
//			phoneext->wPortDelay = 50;
//			phoneext->bPortState1 = 1;
			phoneext->wPortDelay = 600;
			phoneext->bPortState1 = 2;

			mgw_local_seat_callack_request(phoneext);
		break;
#if 0
		case 1:
			if(phoneext->wPortDelay == 0)
			{
				//Gpio_Ring_Set(phoneext->wPort, 0);
				phoneext->bPortState1 = 2;
				phoneext->wPortDelay = 600;
			}
		break;
#endif
		case 2:
		
			if(phoneext->bCommand == MC_HANG)
			{
				Gpio_Ring_Set(phoneext->wPort, 0);
				phoneext->bPortState = MGW_LOCAL_XIWEI_INIT_STATE;
				phoneext->bPortState1 = 1;
				mgw_local_seat_release_cnf_request(phoneext);
				VERBOSE_OUT(LOG_SYS," zhujiao hangup!\n");
			}
			
			if(phoneext->bHookStatus == HOOK_OFF)
			{
				Gpio_Ring_Set(phoneext->wPort, 0);
				phoneext->bPortState = MGW_LOCAL_XIWEI_TALK_STATE;
				phoneext->bPortState1 = 0;
				mgw_local_seat_connect_request(phoneext);
				VERBOSE_OUT(LOG_SYS,"phone%d goto talk in ringing!\n",phoneext->wPort);
				phoneext->bCommand = MC_TALK;
				//sw2st_bus(phoneext->wPort);
			}

			if(phoneext->wPortDelay == 0)
			{
				Gpio_Ring_Set(phoneext->wPort, 0);
				phoneext->bPortState = MGW_LOCAL_XIWEI_INIT_STATE;
				phoneext->bPortState1 = 1;
				VERBOSE_OUT(LOG_SYS,"ring time up!\n");
			}
		break;

		default:
		break;
	}
}


static void MGW_HEADSET_INIT_Process(TERMINAL_BASE* headset)
{
	switch(headset->bPortState1)
	{
		case 1:
		case 0:
			headset->bPortState = MGW_HEADSET_IDLE_STATE;
			headset->bPortState1 = 0;
			headset->wConnectPort = IDLEW;
			headset->wPortDelay = IDLEW;
			headset->bCommand = MC_IDLE;
			headset->bHookStatus = HOOK_ON;	/*note: different from trk port*/
			headset->bPTTStatus = MGW_PTT_INIT;
			headset->bresv = IDLE;	
			headset->rel_reason = MGW_RELEASE_REASON_NORMAL;
			headset->zhuanxian_status = ZHUANXIAN_IDLE;
			/*go to init state in radio direct call*/
			if(headset->bPortType == PORT_TYPE_HEADSET)
			{
				sw2ac491(headset->wPort);
			}
			
			break;
			default:
	        break;			
	}
}

static void MGW_HEADSET_IDLE_Process(TERMINAL_BASE* headset)
{
	switch(headset->bPortState1)
	{
		case 0:
			if(headset->bCommand == MC_RING)
			{
#if 1			
				headset->bPortState = MGW_HEADSET_RING_STATE;
				headset->bPortState1 = 0;
				VERBOSE_OUT(LOG_SYS,"headset recv one normal call!\n");
				headset->buzzer_status = EN_BUZZER_CTL_ON;
				mgw_ext_buzzer_ctl(headset);
#else
				headset->bPortState = MGW_HEADSET_TALK_STATE;
				headset->bPortState1 = 0;

				sw2st_bus(headset->wPort);
				VERBOSE_OUT(LOG_SYS,"headset recv one normal call goto talk!\n");
#endif
			}
			else if(headset->bCommand == MC_RT_TONE)
			{
				headset->bPortState = MGW_HEADSET_RINGTONE_STATE;
				headset->bPortState1 = 0;

				VERBOSE_OUT(LOG_SYS,"headset recv call normal ack!\n");
				headset->bToneType = TONE_RING;
				Send_TONE(headset->wPort,headset->bToneType);
			}	
			break;
		default:
			headset->bPortState1 = 0;
			break;
	}
}

static void MGW_HEADSET_DIAL_Process(TERMINAL_BASE* headset)
{
	switch(headset->bPortState1)
	{
		case 0:
			headset->bPortState1 = 1;
			headset->wPortDelay = 300;
			break;

		case 1:
			if(headset->phone_queue_g.bFinish)
			{
				VERBOSE_OUT(LOG_SYS,"send to scu call request!\n");
				mgw_scu_call_request(headset);
				DEBUG_OUT("xiweiext%d slot = %d\n",headset->wPort,headset->bslot_us);
			}
			headset->bPortState1 = 2;
			break;

		case 2:
			if(headset->bCommand == MC_RT_TONE)
			{
				mgw_ext_callack_notify(headset);
				headset->bPortState = MGW_XIWEI_RINGTONE_STATE;
				headset->bPortState1 = 0;

				if(headset->bPortType == PORT_TYPE_QINWU)
				{
					VERBOSE_OUT(LOG_SYS,"Qinwu recv call normal ack!\n");
					//headset->bToneType = TONE_RING;
					//Send_TONE(headset->wPort,headset->bToneType);
				}
				
			}

			if(headset->bCommand == MC_REJECT)
			{
				VERBOSE_OUT(LOG_SYS,"Qinwu recv call reject ack !\n");
				mgw_ext_callack_notify(headset);
				mgw_scu_release_request(headset);
				headset->bPortState = MGW_XIWEI_INIT_STATE;
				headset->bPortState1 = 0;	
				//xiweiext->wPortDelay == 100;
			}

			if(headset->bHookStatus == HOOK_ON || headset->wPortDelay == 0)
			{
				VERBOSE_OUT(LOG_SYS,"xiwei %d hangup! OR Time Out\n",headset->wPort);
				if(headset->wPortDelay==0)
				{
					headset->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				}
				
				mgw_scu_release_request(headset);
				mgw_ext_release_cnf_notify(headset);
				headset->bPortState = MGW_XIWEI_INIT_STATE;
				headset->bPortState1 = 0;	
				//xiweiext->wPortDelay == 100;
			}

			if(headset->bCommand == MC_TALK)
			{
				VERBOSE_OUT(LOG_SYS,"XIWEI%d goto talk in dial state!\n",headset->wPort);
				mgw_ext_callack_notify(headset);
				/*键显板不能对快速的信令及时作出动作，故延时必要的间隔*/
				headset->wPortDelay = 2;
				headset->bPortState1 = 3;
			}
			break;
		case 3:
			if(headset->wPortDelay == 0)
			{
				mgw_ext_connect_notify(headset);
				headset->bPortState = MGW_XIWEI_TALK_STATE;
				headset->bPortState1 = 0;

				//xiweiext->rtp_socket.remote_port = xiweiext->rtp_socket.local_port;
				headset->rtp_socket.remote_port = get_random_port();

				VERBOSE_OUT(LOG_SYS,"Recv Xiwei connect msg,bind %d!\n",headset->rtp_socket.local_port);

				if(headset->bPortType == PORT_TYPE_QINWU)
				{
					sw2st_bus(headset->wPort);	
				}
				
				if(headset->bPortType != PORT_TYPE_XIWEI)
					break;
				/*
				if(socket_new_with_bindaddr(&xiweiext->rtp_socket)<0)
					VERBOSE_OUT(LOG_SYS,"Create xiwei Socket 0 Error:%s",strerror(errno));
				else
					xiweiext->rtp_io_id = rtp_io_add(xiweiext->rtp_socket.udp_handle,xiweiext);
				*/
				if(socket_new(&headset->rtp_socket)<0)
					VERBOSE_OUT(LOG_SYS,"Create xiwei Socket 0 Error:%s",strerror(errno));	
			}
			break;
			default:
	        break;			
	}
}

static void MGW_HEADSET_RINGTONE_Process(TERMINAL_BASE* headset)
{
	switch(headset->bPortState1)
	{
		case 0:
			if(headset->bCommand == MC_TALK)
			{
				headset->bPortState = MGW_HEADSET_TALK_STATE;
				headset->bPortState1 = 0;

				headset->bToneType = TONE_STOP;
				Send_TONE(headset->wPort,headset->bToneType);
				sw2st_bus(headset->wPort);
				VERBOSE_OUT(LOG_SYS,"headset ringtone goto talk!\n");
			}

			if(headset->bCommand == MC_HANG)
			{
				headset->wPortDelay = 0;
				headset->bPortState1 = 1;
				headset->bToneType = TONE_STOP;
				Send_TONE(headset->wPort,headset->bToneType);				
			}			
			break;
		case 1:
			if(headset->wPortDelay == 0)
			{		
				headset->bPortState = MGW_HEADSET_INIT_STATE;
				headset->bPortState1 = 0x00;
			}			
			break;
		default:
		    break;
			
	}
}

static void MGW_HEADSET_RING_Process(TERMINAL_BASE* headset)
{
	switch(headset->bPortState1)
	{
		case 0:
			if(headset->bCommand == MC_TALK)
			{
				headset->bPortState = MGW_HEADSET_TALK_STATE;
				headset->bPortState1 = 0;

				headset->buzzer_status = EN_BUZZER_CTL_OFF;
				mgw_ext_buzzer_ctl(headset);				
				sw2st_bus(headset->wPort);
				VERBOSE_OUT(LOG_SYS,"headset ring goto talk!\n");
			}
			
			if(headset->bCommand == MC_HANG)
			{
				headset->wPortDelay = 0;
				headset->bPortState1 = 1;
				headset->buzzer_status = EN_BUZZER_CTL_OFF;
				mgw_ext_buzzer_ctl(headset);				
			}			
			break;
		case 1:
			if(headset->wPortDelay == 0)
			{		
				headset->bPortState = MGW_HEADSET_INIT_STATE;
				headset->bPortState1 = 0x00;
			}			
			break;
			default:
			break;
	}
}

static void MGW_HEADSET_TALK_Process(TERMINAL_BASE* headset)
{
	switch(headset->bPortState1)
	{
		case 0:
			if(headset->bCommand == MC_HANG)
			{
				headset->wPortDelay = 0;
				headset->bPortState1 = 1;
			}

			//sw2st_bus(headset->wPort);

			if(headset->bPTTStatus == MGW_PTT_DOWN || headset->bPTTStatus == MGW_PTT_UP)
			{
				VERBOSE_OUT(LOG_SYS,"Send PTT %s to SCU\n", \
					(headset->bPTTStatus == MGW_PTT_DOWN)?"Down":"Up");
				/*fix repeat ptt msg error -- swf 2011-10-03*/
				if(headset->bPortType == PORT_TYPE_QINWU)
				{
					mgw_scu_ptt_request(headset);
				}
				
				headset->bPTTStatus = MGW_PTT_HOLD;
			}
			
			break;
		case 1:
			if(headset->wPortDelay == 0)
			{		
				headset->bPortState = MGW_HEADSET_INIT_STATE;
				headset->bPortState1 = 0x00;
			}
			break;
			default:
	        break;			
	}
}


static void MGW_SW_PHONE_INIT_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 1:
		case 0:
			//mgw_scu_release_request(xiweiext);
			xiweiext->bPortState = MGW_SW_PHONE_IDLE_STATE;
			xiweiext->bPortState1 = 0;
			xiweiext->wConnectPort = IDLEW;
			xiweiext->wPortDelay = IDLEW;
			xiweiext->bCommand = MC_IDLE;
			xiweiext->bHookStatus = HOOK_ON;	/*note: different from trk port*/
			xiweiext->bPTTStatus = MGW_PTT_INIT;
			xiweiext->bresv = IDLE;				/*go to init state in radio direct call*/
			xiweiext->rel_reason = MGW_RELEASE_REASON_NORMAL;
			xiweiext->zhuanxian_status = ZHUANXIAN_IDLE;
			if(xiweiext->bPortType == PORT_TYPE_SW_PHONE)
			{
				sw2ac491(xiweiext->wPort);
			}
			break;
			default:
			break;
	}
}

static void MGW_SW_PHONE_IDLE_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			if(xiweiext->bHookStatus == HOOK_OFF)
			{
				xiweiext->bPortState = MGW_SW_PHONE_DIAL_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(xiweiext->bCommand == MC_RING)
			{
				xiweiext->bPortState = MGW_SW_PHONE_RING_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(xiweiext->bCommand == MC_INACTIVE)
				xiweiext->bPortState1 = 2;
			
			break;

		case 2:
			if(xiweiext->bCommand == MC_ACTIVE)
			{
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}
			break;

		default:
			xiweiext->bPortState1 = 0;
			break;
	}
}

static void MGW_SW_PHONE_DIAL_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			xiweiext->bPortState1 = 1;
			xiweiext->wPortDelay = 300;
			break;
		case 1:
			if(xiweiext->phone_queue_g.bFinish)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d send to scu call request!\r\n", xiweiext->wPort);
				mgw_scu_call_request(xiweiext);
				//VERBOSE_OUT(LOG_SYS,"swphone%d slot = %d\n",xiweiext->wPort,xiweiext->bslot_us);
			}
			xiweiext->bPortState1 = 2;
			break;

		case 2:
			if(xiweiext->bCommand == MC_RT_TONE)
			{
				//xiweiext->wPortDelay = 0;
				mgw_sw_phone_callack_request(xiweiext);
				VERBOSE_OUT(LOG_SYS,"swphone%d send to swphone callack!\r\n", xiweiext->wPort);
				xiweiext->bPortState = MGW_SW_PHONE_RINGTONE_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(xiweiext->bCommand == MC_TALK)
			{
				//xiweiext->wPortDelay = 0;
				sw2net(xiweiext->wPort);
				
				mgw_sw_phone_connect_request(xiweiext);
				VERBOSE_OUT(LOG_SYS,"swphone%d  from dial  send to swphoneconnnect!\r\n", xiweiext->wPort);
				xiweiext->bPortState = MGW_SW_PHONE_TALK_STATE;
				xiweiext->bPortState1 = 0;
				xiweiext->rtp_socket.remote_port = get_random_port();
			}

			if(xiweiext->bCommand == MC_REJECT)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d recv call reject ack !\n", xiweiext->wPort);
				mgw_sw_phone_callack_request(xiweiext);
				mgw_scu_release_request(xiweiext);
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
				//xiweiext->wPortDelay == 100;
			}

			if(xiweiext->bCommand == MC_NONUM)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d recv call no num ack !\n", xiweiext->wPort);
				mgw_sw_phone_callack_request(xiweiext);
				mgw_scu_release_request(xiweiext);
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
				//xiweiext->wPortDelay == 100;
			}

			if(xiweiext->bCommand == MC_NUMERROR)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d recv call no num ack !\n", xiweiext->wPort);
				mgw_sw_phone_callack_request(xiweiext);
				mgw_scu_release_request(xiweiext);
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
				//xiweiext->wPortDelay == 100;
			}

			if(xiweiext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d Recv Hangup Msg\n",xiweiext->wPort);
				mgw_scu_release_cnf_request(xiweiext);
				mgw_sw_phone_release_request(xiweiext);
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}			

			if(xiweiext->bHookStatus == HOOK_ON || xiweiext->wPortDelay == 0)
			{
				VERBOSE_OUT(LOG_SYS,"swphone %d hangup! OR Time Out\n",xiweiext->wPort);
				if(xiweiext->wPortDelay==0)
				{
					xiweiext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				}
				
				mgw_scu_release_request(xiweiext);
				mgw_sw_phone_release_cnf_request(xiweiext);
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
				//xiweiext->wPortDelay == 100;
			}
			break;
			default:
	        break;			
	}
}

static void MGW_SW_PHONE_RINGTONE_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			xiweiext->wPortDelay = 600;
			xiweiext->bPortState1 = 1;
			break;

		case 1:
			if(xiweiext->bCommand == MC_TALK)
			{
				//xiweiext->wPortDelay = 0;
				sw2net(xiweiext->wPort);
				
				mgw_sw_phone_connect_request(xiweiext);
				VERBOSE_OUT(LOG_SYS,"swphone%d send to swphone connnect!\r\n", xiweiext->wPort);
				xiweiext->bPortState = MGW_SW_PHONE_TALK_STATE;
				xiweiext->bPortState1 = 0;

				//xiweiext->rtp_socket.remote_port = xiweiext->rtp_socket.local_port;
				xiweiext->rtp_socket.remote_port = get_random_port();

				//VERBOSE_OUT(LOG_SYS,"Recv Xiwei connect msg,bind %d!\n",xiweiext->rtp_socket.local_port);
			}

			if(xiweiext->bHookStatus == HOOK_ON)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d Send Hangup msg \n",xiweiext->wPort);
				mgw_scu_release_request(xiweiext);
				mgw_sw_phone_release_cnf_request(xiweiext);
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;				
			}

			if(xiweiext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d Recv Hangup Msg! OR Time Out\n",xiweiext->wPort);
				mgw_scu_release_cnf_request(xiweiext);
				mgw_sw_phone_release_request(xiweiext);
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}
			
			if(xiweiext->wPortDelay == 0)
			{
				xiweiext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				mgw_scu_release_request(xiweiext);
				mgw_sw_phone_release_request(xiweiext);
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}
			break;
			default:
	        break;			
	}
}

static void MGW_SW_PHONE_RING_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			xiweiext->wPortDelay = 600;
			xiweiext->bPortState1 = 1;
			mgw_sw_phone_call_request(xiweiext);
			break;
		case 1:
			if(xiweiext->bHookStatus == HOOK_OFF)
			{
				//xiweiext->wPortDelay = 0;
				sw2net(xiweiext->wPort);	
				mgw_scu_connect_request(xiweiext);

				VERBOSE_OUT(LOG_SYS,"swphone%d send to scu connnect!\r\n", xiweiext->wPort);
				
				xiweiext->rtp_socket.remote_port = xiweiext->rtp_socket.local_port;
				xiweiext->rtp_socket.remote_port = get_random_port();
				xiweiext->bPortState = MGW_SW_PHONE_TALK_STATE;
				xiweiext->bPortState1 = 0;
				xiweiext->bCommand = MC_TALK;

				//VERBOSE_OUT(LOG_SYS,"Recv Xiwei connect msg,bind %d!\n",xiweiext->rtp_socket.local_port);
			}

			if(xiweiext->bCommand == MC_HANG || xiweiext->wPortDelay == 0)
			{
				VERBOSE_OUT(LOG_SYS,"xiwei%d Recv Hangup Msg! OR Time Out\n",xiweiext->wPort);
				mgw_scu_release_cnf_request(xiweiext);
				mgw_sw_phone_release_request(xiweiext);
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(MC_HANG_ACTIVE == xiweiext->bCommand)
			{
				//mgw_sw_phone_release_request(xiweiext);
				//xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				//xiweiext->bPortState1 = 0;
				mgw_scu_release_request(xiweiext);
				mgw_sw_phone_release_cnf_request(xiweiext);/*释放证实*/
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;				
			}

			break;
			default:
	        break;			
	}
}

static void MGW_SW_PHONE_TALK_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			xiweiext->wPortDelay = 0;
			
			if(xiweiext->bHookStatus == HOOK_ON)
			{
				mgw_scu_release_request(xiweiext);
				mgw_sw_phone_release_cnf_request(xiweiext);/*释放证实*/
				xiweiext->bPortState1 = 1;
			}

			if(xiweiext->bCommand == MC_TRANS)
			{
				mgw_scu_calltrans_request(xiweiext);
				mgw_sw_phone_release_request(xiweiext);
				xiweiext->bPortState1 = 1;
			}

			if(xiweiext->bCommand == MC_HANG)
			{
				mgw_scu_release_cnf_request(xiweiext);
				mgw_sw_phone_release_request(xiweiext);
				xiweiext->bPortState1 = 1;
			}

			if(xiweiext->bPTTStatus == MGW_PTT_DOWN || xiweiext->bPTTStatus == MGW_PTT_UP)
			{
				VERBOSE_OUT(LOG_SYS,"Send PTT %s to SCU\n", \
					(xiweiext->bPTTStatus == MGW_PTT_DOWN)?"Down":"Up");
				/*fix repeat ptt msg error -- swf 2011-10-03*/
				if(xiweiext->bPortType == PORT_TYPE_SW_PHONE)
				{
					mgw_scu_ptt_request(xiweiext);
				}
				
				xiweiext->bPTTStatus = MGW_PTT_HOLD;
			}
			
			break;

		case 1:

			if(xiweiext->wPortDelay == 0)
			{
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 1;
			}
			break;
			default:
	        break;			
	}
}


int	terminal_call_control(const void *data)
{
	int i;
	data = data;
	for(i=0;i<USER_NUM_MAX;i++)
	{
		if(terminal_group[i].bPortType == PORT_TYPE_TRK)
		{
			switch(terminal_group[i].bPortState)
			{
				case MGW_TRK_INIT_STATE:
					MGW_TRK_INIT_Process(&terminal_group[i]);
				break;

				case MGW_TRK_IDLE_STATE:
					MGW_TRK_IDLE_Process(&terminal_group[i]);
				break;

				case MGW_TRK_IN_DIAL_STATE:
					MGW_TRK_IN_DIAL_Process(&terminal_group[i]);
				break;

				case MGW_TRK_IN_RINGTONG_STATE:
					MGW_TRK_IN_RINGTONG_Process(&terminal_group[i]);
				break;

				case MGW_TRK_IN_TALK_STATE:
					MGW_TRK_IN_TALK_Process(&terminal_group[i]);
				break;

				case MGW_TRK_OUT_DIAL_STATE:
					MGW_TRK_OUT_DIAL_Process(&terminal_group[i]);
				break;

				case MGW_TRK_OUT_RINGTONE_STATE:
					MGW_TRK_OUT_RINGTONE_Process(&terminal_group[i]);
				break;

				case MGW_TRK_OUT_TALK_STATE:
					MGW_TRK_OUT_TALK_Process(&terminal_group[i]);
				break;

				default:
					terminal_group[i].bPortState = MGW_TRK_INIT_STATE;
				break;
			}	
		}
		else if(terminal_group[i].bPortType == PORT_TYPE_CISHI)
		{
			switch(terminal_group[i].bPortState)
			{
				case MGW_CISHI_INIT_STATE:
					MGW_CISHI_INIT_Process(&terminal_group[i]);
				break;

				case MGW_CISHI_IDLE_STATE:
					MGW_CISHI_IDLE_Process(&terminal_group[i]);
				break;

				case MGW_CISHI_DIAL_STATE:
					MGW_CISHI_DIAL_Process(&terminal_group[i]);
				break;

				case MGW_CISHI_RINGTONE_STATE:
					MGW_CISHI_RINGTONE_Process(&terminal_group[i]);
				break;

				case MGW_CISHI_TALK_STATE:
					MGW_CISHI_TALK_Process(&terminal_group[i]);
				break;

				case MGW_CISHI_RING_STATE:
					MGW_CISHI_RING_Process(&terminal_group[i]);
				break;

				default:
					MGW_CISHI_INIT_Process(&terminal_group[i]);
				break;
			}
		}
		else if(terminal_group[i].bPortType == PORT_TYPE_PHONE)
		{
			if(NETWARE_ZXCREATE_NOTOK == terminal_group[i].zhuanxian_flg)
			{
				switch(terminal_group[i].bPortState)
				{
					case MGW_PHONE_INIT_STATE:
						MGW_Phone_INIT_Process(&terminal_group[i]);
					break;

					case MGW_PHONE_IDLE_STATE:
						MGW_Phone_IDLE_Process(&terminal_group[i]);
					break;

					case MGW_PHONE_DIAL_STATE:
						MGW_Phone_DIAL_Process(&terminal_group[i]);
					break;

					case MGW_PHONE_RINGTONE_STATE:
						MGW_Phone_RINGTONE_Process(&terminal_group[i]);
					break;

					case MGW_PHONE_TALK_STATE:
						MGW_Phone_TALK_Process(&terminal_group[i]);
					break;

					case MGW_PHONE_RING_STATE:
						MGW_Phone_RING_Process(&terminal_group[i]);
					break;

					default:
						MGW_Phone_INIT_Process(&terminal_group[i]);
					break;
				}
			}
			else if(NETWARE_ZXCREATE_OK == terminal_group[i].zhuanxian_flg) //建立了专线
			{
#if 1			
				switch(terminal_group[i].bPortState)
				{
					case MGW_PHONE_INIT_STATE:
					
						if ((WORK_MODE_PAOBING == Param_update.workmode)\
							&&(ZHUANXIAN_INIT == terminal_group[i].zhuanxian_status))
						{				
							TERMINAL_BASE* phoneext = &terminal_group[i];

							mgw_scu_pf_zwzhuanxian_callack(phoneext);
							
							printf("lsb 11:ZW zhuanxian send connect request after call ack\r\n");
							
							mgw_scu_connect_request(phoneext);
							
							printf("lsb 22: ZW phone %d goto talk !\r\n", phoneext->wPort);

							terminal_group[i].zhuanxian_status = ZHUANXIAN_CALLTALK; //避免本循环反复进入本段代码发送连接请求

							terminal_group[i].bPTTStatus = MGW_PTT_INIT;
							//terminal_group[i].bPortState = MGW_PHONE_IDLE_STATE;
							printf("phone%d init goto idle!%d\r\n", i, terminal_group[i].bCommand);
						}
						else if(WORK_MODE_ZHUANGJIA == Param_update.workmode)
						{
							//terminal_group[i].bPortState = MGW_PHONE_IDLE_STATE;
						}
						
						terminal_group[i].bPortState = MGW_PHONE_IDLE_STATE;
						terminal_group[i].bHookStatus = HOOK_ON;	/*note: different from trk port*/
						terminal_group[i].bPTTStatus = MGW_PTT_INIT;
						Gpio_Ring_Set(terminal_group[i].wPort, 0);
						sw2ac491(terminal_group[i].wPort);
						break;
					case MGW_PHONE_IDLE_STATE:
						terminal_group[i].bPTTStatus = MGW_PTT_INIT;
						if(terminal_group[i].bHookStatus == HOOK_OFF)
						{					
							if (WORK_MODE_ZHUANGJIA == Param_update.workmode)
							{
								mgw_scu_zhuanxian_msg(&terminal_group[i]);
							}
							
							sw2st_bus(terminal_group[i].wPort);
							Gpio_Ring_Set(terminal_group[i].wPort, 0);

							terminal_group[i].bCommand = MC_TALK; 
							terminal_group[i].bPortState = MGW_PHONE_TALK_STATE;
							printf("phone%d init idle goto talk!%d\r\n", i, terminal_group[i].bCommand);
						}
						break;
					case MGW_PHONE_TALK_STATE:

						if(terminal_group[i].bPTTStatus == MGW_PTT_DOWN || terminal_group[i].bPTTStatus == MGW_PTT_UP)
						{
							VERBOSE_OUT(LOG_SYS,"Send PTT %s to SCU\n", \
								(terminal_group[i].bPTTStatus == MGW_PTT_DOWN)?"Down":"Up");
							mgw_scu_ptt_request(&terminal_group[i]);
							
							#ifdef ZHUANXIAN_VOICE_CTRL	
								if(MGW_PTT_DOWN == terminal_group[i].bPTTStatus)
								{
									sw2_phone_talk(terminal_group[i].wPort);
								}
								else if(MGW_PTT_UP == terminal_group[i].bPTTStatus)
								{
									sw2st_bus(terminal_group[i].wPort);
								}
							#endif	
							
							terminal_group[i].bPTTStatus = MGW_PTT_HOLD;
						}
						
						if(terminal_group[i].bHookStatus == HOOK_ON)
						{
							terminal_group[i].bPTTStatus = MGW_PTT_UP;
							if(MGW_PTT_UP == terminal_group[i].bPTTStatus)
							{
								mgw_scu_ptt_request(&terminal_group[i]);
								terminal_group[i].bPTTStatus = MGW_PTT_HOLD;
							}
							
							if (WORK_MODE_ZHUANGJIA == Param_update.workmode)
							{
								mgw_scu_zhuanxian_msg(&terminal_group[i]);
							}
							
							sw2ac491(terminal_group[i].wPort);

							terminal_group[i].bPortState = MGW_PHONE_INIT_STATE;
						}		
						
						break;
					case MGW_PHONE_RING_STATE:
						if(terminal_group[i].bHookStatus == HOOK_OFF)
						{					
							if (WORK_MODE_ZHUANGJIA == Param_update.workmode)
							{
								mgw_scu_zhuanxian_msg(&terminal_group[i]);
							}
							
							sw2st_bus(terminal_group[i].wPort);
							Gpio_Ring_Set(terminal_group[i].wPort, 0);
							
							terminal_group[i].bPortState = MGW_PHONE_TALK_STATE;
						}					
						break;
					default:
						break;
				}
			
#else
				/* 炮防模式下PRN入战网建立专线流程比装甲模式入战网的专线多一个处理流程 ,
				   炮防模式下:834收到50所转发的716专线呼叫请求需要回呼叫应答和连接请求，专线才能建立；
				   装甲模式下:834收到716直接发送过来的连接请求认为专线建立
				*/
				
				if (WORK_MODE_PAOBING == Param_update.workmode)
				{
					if (ZHUANXIAN_INIT == terminal_group[i].zhuanxian_status)
					{				
						TERMINAL_BASE* phoneext = &terminal_group[i];

						mgw_scu_pf_zwzhuanxian_callack(phoneext);
						
						printf("lsb 11:ZW zhuanxian send connect request after call ack\r\n");
						
						mgw_scu_connect_request(phoneext);
						
						printf("lsb 22: ZW phone %d goto talk !\r\n", phoneext->wPort);

						terminal_group[i].zhuanxian_status = ZHUANXIAN_CALLTALK; //避免本循环反复进入本段代码发送连接请求

						terminal_group[i].bPTTStatus = MGW_PTT_INIT;
					}
				}
			
				static int hookstatus1_flg = 0;
				static int hookstatus2_flg = 0;
				
				if(terminal_group[i].bHookStatus == HOOK_ON)
				{
					if((0 == hookstatus1_flg)&&(MGW_PTT_INIT != terminal_group[i].bPTTStatus))
					{
						terminal_group[i].bPTTStatus = MGW_PTT_UP;
						if(MGW_PTT_UP == terminal_group[i].bPTTStatus)
						{
							mgw_scu_ptt_request(&terminal_group[i]);
							terminal_group[i].bPTTStatus = MGW_PTT_INIT;
						}
						
						if (WORK_MODE_ZHUANGJIA == Param_update.workmode)
						{
							mgw_scu_zhuanxian_msg(&terminal_group[i]);
						}
						
						sw2ac491(terminal_group[i].wPort);
					}
					
					hookstatus1_flg = 1;
					hookstatus2_flg = 0;

				}
				else if(terminal_group[i].bHookStatus == HOOK_OFF)
				{					
					if(0 == hookstatus2_flg)
					{
						if (WORK_MODE_ZHUANGJIA == Param_update.workmode)
						{
							mgw_scu_zhuanxian_msg(&terminal_group[i]);
						}
						sw2st_bus(terminal_group[i].wPort);
						Gpio_Ring_Set(terminal_group[i].wPort, 0);
					}

					hookstatus1_flg = 0;
					hookstatus2_flg = 1;
					
					if(terminal_group[i].bPTTStatus == MGW_PTT_DOWN || terminal_group[i].bPTTStatus == MGW_PTT_UP)
					{
						VERBOSE_OUT(LOG_SYS,"Send PTT %s to SCU\n", \
							(terminal_group[i].bPTTStatus == MGW_PTT_DOWN)?"Down":"Up");
						mgw_scu_ptt_request(&terminal_group[i]);
						terminal_group[i].bPTTStatus = MGW_PTT_HOLD;
					}
				}
#endif				
			}
			
		}		
		else if((terminal_group[i].bPortType == PORT_TYPE_XIWEI) || (terminal_group[i].bPortType == PORT_TYPE_QINWU))
		{
			switch(terminal_group[i].bPortState)
			{
				case MGW_XIWEI_INIT_STATE:
					MGW_XIWEI_INIT_Process(&terminal_group[i]);
					break;

				case MGW_XIWEI_IDLE_STATE:
					MGW_XIWEI_IDLE_Process(&terminal_group[i]);
					break;

				case MGW_XIWEI_DIAL_STATE:
					MGW_XIWEI_DIAL_Process(&terminal_group[i]);
					break;

				case MGW_XIWEI_RINGTONE_STATE:
					MGW_XIWEI_RINGTONE_Process(&terminal_group[i]);
					break;

				case MGW_XIWEI_RING_STATE:
					MGW_XIWEI_RING_Process(&terminal_group[i]);
					break;

				case MGW_XIWEI_TALK_STATE:
					MGW_XIWEI_TALK_Process(&terminal_group[i]);
					break;

				default:
					MGW_XIWEI_INIT_Process(&terminal_group[i]);
					break;
			}
		}
		else if(terminal_group[i].bPortType == PORT_TYPE_LOCAL_XIWEI)
		{
			switch(terminal_group[i].bPortState)
			{
				case MGW_LOCAL_XIWEI_INIT_STATE:
					MGW_LOCAL_XIWEI_INIT_Process(&terminal_group[i]);
					break;

				case MGW_LOCAL_XIWEI_IDLE_STATE:
					MGW_LOCAL_XIWEI_IDLE_Process(&terminal_group[i]);
					break;

				case MGW_LOCAL_XIWEI_DIAL_STATE:
					MGW_LOCAL_XIWEI_DIAL_Process(&terminal_group[i]);
					break;

				case MGW_LOCAL_XIWEI_RINGTONE_STATE:
					MGW_LOCAL_XIWEI_RINGTONE_Process(&terminal_group[i]);
					break;

				case MGW_LOCAL_XIWEI_RING_STATE:
					MGW_LOCAL_XIWEI_RING_Process(&terminal_group[i]);
					break;

				case MGW_LOCAL_XIWEI_TALK_STATE:
					MGW_LOCAL_XIWEI_TALK_Process(&terminal_group[i]);
					break;

				default:
					MGW_LOCAL_XIWEI_INIT_Process(&terminal_group[i]);
					break;
			}
		}
		else if(terminal_group[i].bPortType == PORT_TYPE_HEADSET)
		{
			switch(terminal_group[i].bPortState)
			{
				case MGW_HEADSET_INIT_STATE:
					MGW_HEADSET_INIT_Process(&terminal_group[i]);
					break;

				case MGW_HEADSET_IDLE_STATE:
					MGW_HEADSET_IDLE_Process(&terminal_group[i]);
					break;

				case MGW_HEADSET_DIAL_STATE:
					MGW_HEADSET_DIAL_Process(&terminal_group[i]);
					break;

				case MGW_HEADSET_RINGTONE_STATE:
					MGW_HEADSET_RINGTONE_Process(&terminal_group[i]);
					break;

				case MGW_HEADSET_RING_STATE:
					MGW_HEADSET_RING_Process(&terminal_group[i]);
					break;

				case MGW_HEADSET_TALK_STATE:
					MGW_HEADSET_TALK_Process(&terminal_group[i]);
					break;

				default:
					MGW_HEADSET_INIT_Process(&terminal_group[i]);
					break;
			}
		}		
		else if(terminal_group[i].bPortType == PORT_TYPE_SW_PHONE)
		{
			if(NETWARE_ZXCREATE_NOTOK == terminal_group[i].zhuanxian_flg)
			{
				switch(terminal_group[i].bPortState)
				{
					case MGW_SW_PHONE_INIT_STATE:
						MGW_SW_PHONE_INIT_Process(&terminal_group[i]);
						break;

					case MGW_SW_PHONE_IDLE_STATE:
						MGW_SW_PHONE_IDLE_Process(&terminal_group[i]);
						break;

					case MGW_SW_PHONE_DIAL_STATE:
						MGW_SW_PHONE_DIAL_Process(&terminal_group[i]);
						break;

					case MGW_SW_PHONE_RINGTONE_STATE:
						MGW_SW_PHONE_RINGTONE_Process(&terminal_group[i]);
						break;

					case MGW_SW_PHONE_RING_STATE:
						MGW_SW_PHONE_RING_Process(&terminal_group[i]);
						break;

					case MGW_SW_PHONE_TALK_STATE:
						MGW_SW_PHONE_TALK_Process(&terminal_group[i]);
						break;

					default:
						MGW_SW_PHONE_INIT_Process(&terminal_group[i]);
						break;
				}
			}
			else if(NETWARE_ZXCREATE_OK == terminal_group[i].zhuanxian_flg)
			{	
#if 1
				switch(terminal_group[i].bPortState)
				{
					case MGW_SW_PHONE_INIT_STATE:
						if ((WORK_MODE_PAOBING == Param_update.workmode)\
							&&(ZHUANXIAN_INIT == terminal_group[i].zhuanxian_status))
						{				
							TERMINAL_BASE* phoneext = &terminal_group[i];

							mgw_scu_pf_zwzhuanxian_callack(phoneext);
							
							printf("lsb 11:ZW zhuanxian send connect request after call ack\r\n");
							
							mgw_scu_connect_request(phoneext);
							
							printf("lsb 22: ZW swphone %d goto talk !\r\n", phoneext->wPort);

							terminal_group[i].zhuanxian_status = ZHUANXIAN_CALLTALK; //避免本循环反复进入本段代码发送连接请求

							//terminal_group[i].bPTTStatus = MGW_PTT_INIT;

							//terminal_group[i].bPortState = MGW_SW_PHONE_IDLE_STATE;
							printf("swphone%d init goto idle!%d\r\n", i, terminal_group[i].bCommand);
						}
						else if ((WORK_MODE_PAOBING == Param_update.workmode)\
							&&(ZHUANXIAN_CALLTALK == terminal_group[i].zhuanxian_status))
						{
							//terminal_group[i].bPTTStatus = MGW_PTT_INIT;
							//terminal_group[i].bPortState = MGW_SW_PHONE_IDLE_STATE;
							printf("swphone%d init goto idle!%d\r\n", i, terminal_group[i].bCommand);
						}	
						else if(WORK_MODE_ZHUANGJIA == Param_update.workmode)
						{
							printf("swphone%d init goto idle!%d\r\n", i, terminal_group[i].bCommand);
							//terminal_group[i].bPortState = MGW_SW_PHONE_IDLE_STATE;
						}

						terminal_group[i].bPortState = MGW_SW_PHONE_IDLE_STATE;
						terminal_group[i].bPTTStatus = MGW_PTT_INIT;

						#ifdef ZHUANXIAN_VOICE_CTRL	
						terminal_group[i].zhuanxian_swphone_voc_send = 0;
						#endif

						sw2ac491(terminal_group[i].wPort);
						break;						
					case MGW_SW_PHONE_IDLE_STATE:
						terminal_group[i].bPTTStatus = MGW_PTT_INIT;
						if(terminal_group[i].bHookStatus == HOOK_OFF)
						{
							if (WORK_MODE_ZHUANGJIA == Param_update.workmode)
							{
								mgw_scu_zhuanxian_msg(&terminal_group[i]);
							}
							
							sw2net(terminal_group[i].wPort);
							
							terminal_group[i].bPortState = MGW_SW_PHONE_TALK_STATE;
							terminal_group[i].bCommand = MC_TALK; 
							LOG("swphone %d idle goto talk\r\n", i);
						}
						break;
					case MGW_SW_PHONE_TALK_STATE:
						if(terminal_group[i].bPTTStatus == MGW_PTT_DOWN || terminal_group[i].bPTTStatus == MGW_PTT_UP)
						{
							VERBOSE_OUT(LOG_SYS,"Send PTT %s to SCU\n", \
								(terminal_group[i].bPTTStatus == MGW_PTT_DOWN)?"Down":"Up");

							mgw_scu_ptt_request(&terminal_group[i]);

							#ifdef ZHUANXIAN_VOICE_CTRL	
							if(MGW_PTT_DOWN == terminal_group[i].bPTTStatus)
							{
								terminal_group[i].zhuanxian_swphone_voc_send = 0;
							}
							else if(MGW_PTT_UP == terminal_group[i].bPTTStatus)
							{
								terminal_group[i].zhuanxian_swphone_voc_send = 1;
							}
							#endif
							
							terminal_group[i].bPTTStatus = MGW_PTT_HOLD;
						}
						
						if(terminal_group[i].bHookStatus == HOOK_ON)
						{
							terminal_group[i].bPTTStatus = MGW_PTT_UP;
							if(MGW_PTT_UP == terminal_group[i].bPTTStatus)
							{
								mgw_scu_ptt_request(&terminal_group[i]);
								terminal_group[i].bPTTStatus = MGW_PTT_HOLD;
							}
							
							if (WORK_MODE_ZHUANGJIA == Param_update.workmode)
							{
								mgw_scu_zhuanxian_msg(&terminal_group[i]);
							}
							
							sw2ac491(terminal_group[i].wPort);

							terminal_group[i].bPortState = MGW_SW_PHONE_INIT_STATE;
							terminal_group[i].bCommand = MC_IDLE;
							LOG("swphone %d talk goto idle\r\n", i);
						}		
						
						break;
					case MGW_SW_PHONE_RING_STATE:
						if(terminal_group[i].bHookStatus == HOOK_OFF)
						{					
							if (WORK_MODE_ZHUANGJIA == Param_update.workmode)
							{
								mgw_scu_zhuanxian_msg(&terminal_group[i]);
							}
							
							sw2net(terminal_group[i].wPort);
							terminal_group[i].bPortState = MGW_SW_PHONE_TALK_STATE;
						}					
						break;
					default:
						break;
				}
			
#else
				static int sw_phone_hookstatus1_flg = 0;
				static int sw_phone_hookstatus2_flg = 0;
				
				if(terminal_group[i].bHookStatus == HOOK_ON)
				{
					terminal_group[i].bPTTStatus = MGW_PTT_UP;
					if(0 == sw_phone_hookstatus1_flg)
					{
						mgw_sw_phone_release_cnf_request(&terminal_group[i]);
						mgw_scu_ptt_request(&terminal_group[i]);
						if (WORK_MODE_ZHUANGJIA == Param_update.workmode)
						{
							mgw_scu_zhuanxian_msg(&terminal_group[i]);
						}

						sw2ac491(terminal_group[i].wPort);

						terminal_group[i].bPortState = MGW_SW_PHONE_INIT_STATE;
					}
					
					sw_phone_hookstatus1_flg = 1;
					sw_phone_hookstatus2_flg = 0;

				}
				else if(terminal_group[i].bHookStatus == HOOK_OFF)
				{					
					if(0 == sw_phone_hookstatus2_flg)
					{
						if (WORK_MODE_ZHUANGJIA == Param_update.workmode)
						{
							mgw_scu_zhuanxian_msg(&terminal_group[i]);
						}
						sw2net(terminal_group[i].wPort);
						terminal_group[i].bPortState = MGW_SW_PHONE_TALK_STATE;
					}

					sw_phone_hookstatus1_flg = 0;
					sw_phone_hookstatus2_flg = 1;
					
					if(terminal_group[i].bPTTStatus == MGW_PTT_DOWN || terminal_group[i].bPTTStatus == MGW_PTT_UP)
					{
						VERBOSE_OUT(LOG_SYS,"Send PTT %s to SCU\n", \
							(terminal_group[i].bPTTStatus == MGW_PTT_DOWN)?"Down":"Up");
						mgw_scu_ptt_request(&terminal_group[i]);
						terminal_group[i].bPTTStatus = MGW_PTT_HOLD;
					}
				}
#endif				
			}
			
		}		
		
		if(terminal_group[i].wPortDelay != 0 && terminal_group[i].wPortDelay != IDLEW)
			terminal_group[i].wPortDelay--;
	}
	
	update_hookstatus();
	time_cnt_100ms++;
	Param_update.timecnt_100ms++;
	
	return 1;
}



