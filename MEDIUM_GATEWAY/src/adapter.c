
#ifndef __PHONE_C__
#define __PHONE_C__

#if defined(__cplusplus) 
extern "C" 
{ 
#endif

#include "PUBLIC.h"

#define RINGTONE_BY_REMOTE_CTL
#undef RINGTONE_BY_REMOTE_CTL

static struct sockaddr_in g_XiWeiDataSockAddr;
static struct sockaddr_in g_XiWeiVocSockAddr;

static struct sockaddr_in g_AdptphDataSockAddr;
static struct sockaddr_in g_AdptphVocSockAddr;

static int32 g_XiWeiDatSockFd = 0;
static int32 g_XiWeiVocSockFd = 0;

static int32 g_AdpPhoneDatSockFd = 0;
static int32 g_AdpPhoneVocSockFd = 0;

static int32 g_SeatDatSockFd = 0;

extern uint32 time_cnt_100ms; //计算成员是否掉线时间
extern int Ac491DrvInit(void);

//static int32 adapter_phone_cnt_talk = 0;
static uint8 kuozhan_voc_add_temp_buf[ZJCT_VOC_PCM_20MS_LEN_320] = {0};
CONF_PARAM kuozhan_conf[KUOZHAN_CONF_NUM_MAX];


static int32 kuozhan_release_request(TERMINAL_BASE* terext);

static int32 XiWei_Data_Send_by_IP(uint8 *buf, int32 len, uint32 ipaddr)
{
	struct sockaddr_in SendIP;

	memset(&SendIP, 0x00, sizeof(SendIP));

	SendIP.sin_family = AF_INET;
	SendIP.sin_port = htons(40011);
	SendIP.sin_addr.s_addr = htonl(ipaddr);

	Socket_Send(g_XiWeiDatSockFd, &SendIP, buf, len);
	
	return DRV_OK;
}

static int32 XiWei_Voc_Send_by_IP(uint8 *buf, int32 len, uint32 ipaddr)
{
	struct sockaddr_in SendIP;

	memset(&SendIP, 0x00, sizeof(SendIP));

	SendIP.sin_family = AF_INET;
	SendIP.sin_port = htons(40012);
	SendIP.sin_addr.s_addr = htonl(ipaddr);

	Socket_Send(g_XiWeiVocSockFd, &SendIP, buf, len);
	
	return DRV_OK;
}

static int32 Adp_Phone_Data_Send_by_IP(uint8 *buf, int32 len, uint32 ipaddr)
{
	struct sockaddr_in SendIP;

	memset(&SendIP, 0x00, sizeof(SendIP));
	
	SendIP.sin_family = AF_INET;
	SendIP.sin_port = htons(40005);
	SendIP.sin_addr.s_addr = htonl(ipaddr);

	Socket_Send(g_AdpPhoneDatSockFd, &SendIP, buf, len);
	
	return DRV_OK;
}

static int32 Adp_Phone_Voc_Send_by_IP(uint8 *buf, int32 len, uint32 ipaddr)
{
	struct sockaddr_in SendIP;

	memset(&SendIP, 0x00, sizeof(SendIP));

	SendIP.sin_family = AF_INET;
	SendIP.sin_port = htons(40006);
	SendIP.sin_addr.s_addr = htonl(ipaddr);

	Socket_Send(g_AdpPhoneVocSockFd, &SendIP, buf, len);
	
	return DRV_OK;
}

static int32 Seat_Data_Send_by_IP(uint8 *buf, int32 len, uint32 ipaddr)
{
	struct sockaddr_in SendIP;

	memset(&SendIP, 0x00, sizeof(SendIP));

	SendIP.sin_family = AF_INET;
	SendIP.sin_port = htons(30000);
	SendIP.sin_addr.s_addr = htonl(ipaddr);

	Socket_Send(g_SeatDatSockFd, &SendIP, buf, len);
	
	return DRV_OK;
}

int32 Check_834_KuoZhan_XiWei_Timeout(const void *data)
{
	uint8 i = 0;

	for(i = KUOZHAN_PHONE_OFFSET; i < KUOZHAN_ADATPER_OFFSET; i++)
	{
		if(1 == terminal_group[i].sw_phone_online)
		{
			if((Param_update.timecnt_100ms - terminal_group[i].timecnt_100ms ) > 70)
			{
				if((MGW_SW_PHONE_INIT_STATE != terminal_group[i].bPortState)\
					&&(MGW_SW_PHONE_IDLE_STATE != terminal_group[i].bPortState))
				{
					kuozhan_release_request(&terminal_group[i]);
				}
				
				terminal_group[i].sw_phone_flg = 0;
				terminal_group[i].sw_phone_834_flg = 0;
				terminal_group[i].sw_phone_online = 0;

				if((i >= KUOZHAN_PHONE_OFFSET) && (i < KUOZHAN_XIWEI_OFFSET))
				{
					terminal_group[i].bPortType = PORT_TYPE_KUOZHAN_PHONE;
					terminal_group[i].bPortState = MGW_PHONE_INIT_STATE;
				}
				else
				{
					terminal_group[i].bPortState = MGW_SW_PHONE_INIT_STATE;
				}
				
				terminal_group[i].bPortState1 = 0;
				terminal_group[i].ipaddr = 0;
				terminal_group[i].talk_flg = 0;

				printf("sw phone %d offline\r\n", i - KUOZHAN_PHONE_OFFSET);
			}
		}
	}

	return 1;
}


static int32 Phone_Ac491_Voc_Process(uint8 *buf, uint32 len, uint8 id)
{	
	uint8 sendbuf[RTP_TXBUFF_SIZE];
	struct TRTPHeader *pRTPHeader;	
	
	static uint32 seq[KUOZHAN_USER_NUM_MAX] = {0};
	static uint32 s_timeslot[KUOZHAN_USER_NUM_MAX] = {0};
		
	if (ZJCT_VOC_G711_20MS_LEN_160 != len)
	{
		ERR("20ms G711 voice len (%d)!=160 \n", len);
		return DRV_ERR;
	}

	pRTPHeader = (struct TRTPHeader *)&sendbuf[0]; 
	memcpy(&sendbuf[sizeof(struct TRTPHeader)],  buf , ZJCT_VOC_G711_20MS_LEN_160);
	
	pRTPHeader->RTPVer = 2;
	pRTPHeader->RTPPadding = 0;
	pRTPHeader->RTPExt = 0;
	pRTPHeader->RTPCSRCCnt = 0;
	pRTPHeader->RTPMarker = 0;
	pRTPHeader->RTPPT = 0x08; 
	
	pRTPHeader->RTPSSRC = 0x11111111; 
	pRTPHeader->RTPSeqNum = seq[id];	
	seq[id]++;
	pRTPHeader->RTPTimeStamp = s_timeslot[id];
	s_timeslot[id] += 160;
	pRTPHeader->RTPSeqNum = ntohs(pRTPHeader->RTPSeqNum);		
	pRTPHeader->RTPSSRC = ntohl(pRTPHeader->RTPSSRC);

	if (0 != ac491SendData(id/NUMBER_OF_CHANNELS, id%NUMBER_OF_CHANNELS, \
		sendbuf,  RTP_TXBUFF_SIZE,  PACK_TYPE_RTP))
	{
		ERR("%s: ac491SendData  failed\r\n", __func__);
	}
	
	return DRV_OK;
}

static int32 Xiwei_Ac491_Voc_Process(uint8 *buf, int32 len, uint8 id)
{
	ST_PHONE_SEM Send;

	memset(&Send, 0x00, sizeof(Send));

	Send.Head.ProType = 0x01;
	Send.Head.SemType = ZJCT_VOC_MSG;
	Send.Head.Len = 160 + 3;
	Send.Data[0] = terminal_group[id].bslot_us;
	Send.Data[1] = terminal_group[terminal_group[id].connect_port].bslot_us;	
	Send.Data[2] = 160;
	memcpy(&Send.Data[3], buf, 160);

	XiWei_Voc_Send_by_IP((uint8 *)&Send, \
		Send.Head.Len +  sizeof(ST_PHONE_SEM_HEAD), terminal_group[id].ipaddr);

	return DRV_OK;
}

static int32 Adp_Ac491_Voc_Process(uint8 *buf, int32 len, uint8 id)
{
	ST_ADP_PHONE_SEM Send;

	memset(&Send, 0x00, sizeof(Send));

	Send.Head.ProType = 0x01;
	Send.Head.Edition = 0x01;
	Send.Head.SemType = ZJCT_VOC_MSG;
	Send.Head.Len = 160 + 4;
	Send.Data[0] = terminal_group[id].bslot_us;
	Send.Data[1] = terminal_group[terminal_group[id].connect_port].bslot_us;
	Send.Data[2] = 0;
	Send.Data[3] = 160;
	memcpy(&Send.Data[4], buf, 160);

	Adp_Phone_Voc_Send_by_IP((uint8 *)&Send, \
		Send.Head.Len +  sizeof(ST_ADP_PHONE_SEM_HEAD), terminal_group[id].ipaddr);

	return DRV_OK;
}


static int32 kuozhan_get_320_byte_from_mem(uint8 *buf, uint32 len, uint8 port)
{
	memcpy(buf, &kuozhan_conf[terminal_group[port].conf_id].mem_voc[port][kuozhan_conf[terminal_group[port].conf_id].mem_get[port]][0],\
			ZJCT_VOC_PCM_20MS_LEN_320);	
	kuozhan_conf[terminal_group[port].conf_id].mem_get[port] = (kuozhan_conf[terminal_group[port].conf_id].mem_get[port] + 1) % VOICE_TIME_1min;

	return ZJCT_VOC_PCM_20MS_LEN_320;
}

static int32 kuozhan_voc_add(uint8 *sum_buf, uint8 *buf, uint32 len)
{

	uint32 i = 0;
	int32 temp_sum = 0;

	memcpy(&kuozhan_voc_add_temp_buf[0], buf, len);
	
	for(i = 0; i < len; i+= 2)
	{

		//voc_swap(&voc_add_temp_buf [i], 2);
#if 1		
		temp_sum = *((int16 *)(sum_buf + i)) ;
		temp_sum  += (*((int16 *)(&kuozhan_voc_add_temp_buf[i])) >> 1);
		 if(temp_sum > 32760)
		 {
			*((int16 *)(sum_buf + i)) = 32760;
			ERR("too big %d\r\n", temp_sum);
		 }
		else if(temp_sum < -32760)
		 {
			*((int16 *)(sum_buf + i)) = -32760;
			ERR("too small %d\r\n", temp_sum);
		 }
		 else
		{
			*((int16 *)(sum_buf + i))  = temp_sum & 0xffff ;
		}
#else

		*((int16 *)(sum_buf + i))  = *((int16 *)(&voc_add_temp_buf[i])) ;

#endif
	}

	return DRV_OK;
}


int32 Kuozhan_Ac491_Conf_Voc_Process(const void *data)
{
	int32 retval = 0;
	uint32 sum_len = ZJCT_VOC_PCM_20MS_LEN_320;
	uint16 abuf_len = 0;
	uint8 alawbuf[ZJCT_VOC_G711_20MS_LEN_160] = {0};
	uint8 temp_buf[ZJCT_VOC_PCM_20MS_LEN_320] = {0};
	uint8 sum_buf[ZJCT_VOC_PCM_20MS_LEN_320] = {0};	
	
	int32 conf_id = 0, port = 0;
	int32 add_flg = 0;

#if 1	

	for(conf_id = 0; conf_id < KUOZHAN_CONF_NUM_MAX; conf_id++)
	{
		add_flg = 0;
		memset(sum_buf, 0x00, sizeof(sum_buf));
		
		if(1 == kuozhan_conf[conf_id].talk_flg)
		{
			for(port = 0; port < KUOZHAN_CONF_MEM_MAX; port++)
			{
				if((conf_id == terminal_group[port].conf_id)\
					&&(MC_TALK == terminal_group[port].bCommand))
				{
					if(kuozhan_conf[terminal_group[port].conf_id].mem_save[port]  \
						!= kuozhan_conf[terminal_group[port].conf_id].mem_get[port])
					{
						memset(temp_buf, 0x00, sizeof(temp_buf));
						retval = kuozhan_get_320_byte_from_mem(temp_buf, sum_len, port);
						retval = kuozhan_voc_add(sum_buf, temp_buf, sum_len);
						add_flg = 1;
					}
				}
			}
		}

		if(1 == add_flg)
		{
			memset(alawbuf, 0x00, sizeof(alawbuf));
			abuf_len = Pcm_Voice_Linear2Alaw(sum_buf, sum_len, alawbuf);
			for(port = 0; port < KUOZHAN_CONF_MEM_MAX; port++)
			{
				if((conf_id == terminal_group[port].conf_id)\
					&&(MC_TALK == terminal_group[port].bCommand))
				{
					switch(terminal_group[port].bPortType)
					{
						case PORT_TYPE_QINWU:
							if(MC_TALK == terminal_group[port].bCommand)
							{
								Phone_Ac491_Voc_Process(alawbuf , 160, port);
							}
							break;	
						case PORT_TYPE_TRK:
							if(MC_TALK == terminal_group[port].bCommand)
							{
								Phone_Ac491_Voc_Process(alawbuf , 160, port);
							}
							break;				
						case PORT_TYPE_KUOZHAN_PHONE:
							if(MC_TALK == terminal_group[port].bCommand)
							{
								Phone_Ac491_Voc_Process(alawbuf, 160, port);
							}
							break;
						case PORT_TYPE_KUOZHAN_XIWEI:
							if(MC_TALK == terminal_group[port].bCommand)
							{
								Xiwei_Ac491_Voc_Process(alawbuf, 160, port);
							}									
							break;
						case PORT_TYPE_ADP_PHONE:
							if(MC_TALK == terminal_group[port].bCommand)
							{
								Adp_Ac491_Voc_Process(alawbuf, 160, port);
							}									
							break;									
						default:
							break;
					}
				}
			}
		}		
	}
#else
		if(MC_TALK == terminal_group[0].bCommand)
		{
			if(kuozhan_conf[terminal_group[0].conf_id].mem_save[0]  \
				!= kuozhan_conf[terminal_group[0].conf_id].mem_get[0])
			{
				memset(temp_buf, 0x00, sizeof(temp_buf));
				retval = kuozhan_get_320_byte_from_mem(temp_buf, sum_len, port);
				retval = kuozhan_voc_add(sum_buf, temp_buf, sum_len);
				memset(alawbuf, 0x00, sizeof(alawbuf));
				abuf_len = Pcm_Voice_Linear2Alaw(sum_buf, sum_len, alawbuf);	

				Phone_Ac491_Voc_Process(alawbuf , 160, port);
			}
		}

#endif

	return 1;
}

static int32 Kuozhan_Ac491_Single_Voc_Process(uint8 *buf, int32 len, uint8 port)
{
	int connect_port = 0;

	connect_port =  terminal_group[port].connect_port;
	if((DRV_ERR == connect_port)||(IDLE == connect_port)\
		||(0xFF == connect_port))
	{
		printf("%s error port = %d\r\n", __func__, connect_port);
		return DRV_ERR;
	}
	
	switch(terminal_group[connect_port].bPortType)
	{
		case PORT_TYPE_QINWU:
			if(MC_TALK == terminal_group[connect_port].bCommand)
			{
				Phone_Ac491_Voc_Process(buf , 160, connect_port);
			}
			break;	
		case PORT_TYPE_TRK:
			if(MC_TALK == terminal_group[connect_port].bCommand)
			{
				Phone_Ac491_Voc_Process(buf , 160, connect_port);
			}
			break;				
		case PORT_TYPE_KUOZHAN_PHONE:
			if(MC_TALK == terminal_group[connect_port].bCommand)
			{
				Phone_Ac491_Voc_Process(buf, 160, connect_port);
			}
			break;
		case PORT_TYPE_KUOZHAN_XIWEI:
			if(MC_TALK == terminal_group[connect_port].bCommand)
			{
				Xiwei_Ac491_Voc_Process(buf, 160, connect_port);
			}									
			break;
		case PORT_TYPE_ADP_PHONE:
			if(MC_TALK == terminal_group[connect_port].bCommand)
			{
				Adp_Ac491_Voc_Process(buf, 160, connect_port);
			}									
			break;									
		default:
			break;
	}

	return DRV_OK;
}

static int32 Kuozhan_Ac491_Conf_Voc_Save(uint8 *buf, int32 len, uint8 port)
{
	int32 retval = 0;
	uint8 temp_buf[330] = {0};
	
	retval = Pcm_Voice_Alaw2Linear(buf, len, temp_buf);

	memcpy(&kuozhan_conf[terminal_group[port].conf_id].mem_voc[port][kuozhan_conf[terminal_group[port].conf_id].mem_save[port]][0],\
			temp_buf, ZJCT_VOC_PCM_20MS_LEN_320);	
	kuozhan_conf[terminal_group[port].conf_id].mem_save[port] = (kuozhan_conf[terminal_group[port].conf_id].mem_save[port] + 1) % VOICE_TIME_1min;

	return DRV_OK;
}

int32 Kuozhan_Ac491_Voc_Process(uint8 *buf, int32 len, uint8 port)
{
	if(len != ZJCT_VOC_G711_20MS_LEN_160)
	{
		ERR("%s: len error %d\r\n", __func__, len);
		return DRV_ERR;
	}
	
	switch(terminal_group[port].bCallType)
	{
		case EN_SEL_CALL:
			Kuozhan_Ac491_Single_Voc_Process(buf, len, port);
			break;
		case EN_PLAN_CONF:
			Kuozhan_Ac491_Conf_Voc_Save(buf, len, port);
			break;
		default:
			break;
	}
		
	return DRV_OK;
}

static int32 find_port_by_phone_number(TERMINAL_BASE* terext)
{
	int i = 0; 
	
	printf("%s\r\n", terext->phone_queue_g.bPhoneQueue);
	
	for(i = 0;i < KUOZHAN_USER_NUM_MAX; i++)
	{
		if(0 == memcmp(terminal_group[i].name, \
			terext->phone_queue_g.bPhoneQueue, terminal_group[i].user_num_len))
		{
			return i;
		}
	}

	return IDLE;
}

static int32 find_port_by_conf_number(TERMINAL_BASE* terext)
{
	int i = 0, j = 0; 
	
	printf("%s\r\n", terext->phone_queue_g.bPhoneQueue);
	
	for(i = 0;i < KUOZHAN_CONF_NUM_MAX; i++)
	{
		if(0 == memcmp(kuozhan_conf[i].conf_name, \
			terext->phone_queue_g.bPhoneQueue, kuozhan_conf[i].conf_num_len))
		{
			break;
		}
	}

	if(KUOZHAN_CONF_NUM_MAX != i)
	{
		for(j = 0;j < KUOZHAN_CONF_MEM_MAX; j++)
		{
			if(0 == memcmp(kuozhan_conf[i].conf_phone[j], \
				terext->name, terext->user_num_len))
			{
				break;
			}		
		}

		if(j == KUOZHAN_CONF_MEM_MAX)
		{
			return MC_NUMERROR;
		}
	}
	else
	{
		return MC_NONUM;
	}

	return i;
}

static int32 find_port_by_number(char *number)
{
	int i = 0; 
	
	printf("%s\r\n", number);
	
	for(i = 0;i < KUOZHAN_USER_NUM_MAX; i++)
	{
		if(0 == memcmp(terminal_group[i].name, \
			number, terminal_group[i].user_num_len))
		{
			return i;
		}
	}

	return IDLE;
}

static int32 xiwei_call_request(TERMINAL_BASE* terext)
{
	ST_HF_MGW_PHONE_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_CALL_REQ call_req;
	int tmp_len;
	//char *tmp_dup_str;

	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));
	memset(&call_req, 0x00, sizeof(call_req));	
	
	/*1.packet call request*/
	call_req.id = MGW_SCU_CALL_ID;
	call_req.len = sizeof(ST_HF_MGW_SCU_CALL_REQ) - 2;
	call_req.slot = terext->bslot_us;
	call_req.call_type = terext->bCallType;	
	call_req.encoder = 0x04;	/*默认编码格式*/
	call_req.phone_len = terext->phone_queue_g.bPhoneQueueHead;
	phone2bcd((char *)call_req.callee_num, (char *)terext->phone_queue_g.bPhoneQueue,\
		call_req.phone_len);

	tmp_len = sizeof(ST_HF_MGW_SCU_CALL_REQ);

	call_req.callee_num[4] = 0xFF;
	call_req.callee_num[5] = 0xFF;
	call_req.callee_num[6] = 0xFF;
	call_req.callee_num[7] = 0xFF;
	call_req.callee_num[8] = 0xFF;
	call_req.callee_num[9] = 0xFF;
		
	/*2.packet complete protocol*/
	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_CALL_REQ);
	memcpy(st_hf_mgw_scu_pro.body,&call_req,sizeof(ST_HF_MGW_SCU_CALL_REQ));

	tmp_len += sizeof(ST_HF_MGW_PHONE_PRO_HEADER);

	/*3.send packet*/
	return XiWei_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}


static int32 xiwei_callack_request(TERMINAL_BASE* terext)
{
	ST_HF_MGW_PHONE_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_CALLACK_REQ callack_req;
	int tmp_len;
    memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));
	memset(&callack_req, 0x00, sizeof(callack_req));

	callack_req.id = MGW_SCU_CALLACK_ID;
	callack_req.len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ) - 2;
	callack_req.slot = terext->bslot_us;
	callack_req.call_type = terext->bCallType;

	if(terext->bCommand == MC_RT_TONE)
	{		
		callack_req.call_status = EN_JIEXU_SUCCESS;
	}
	else if(terext->bCommand == MC_REJECT)
	{
		callack_req.call_status = EN_JIEXU_FAILURE;
	}
	else if(terext->bCommand == MC_TALK)
	{
		callack_req.call_status = EN_JIEXU_SUCCESS;
	}
	else if(terext->bCommand == MC_NONUM)
	{
		callack_req.call_status = EN_JIEXU_NONUM;
	}
	else if(terext->bCommand == MC_NUMERROR)
	{
		callack_req.call_status = EN_JIEXU_NUMERROR;
	}
			
	tmp_len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ);
	
	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ);
	memcpy(st_hf_mgw_scu_pro.body,&callack_req,sizeof(ST_HF_MGW_SCU_CALLACK_REQ));

	tmp_len += sizeof(ST_HF_MGW_PHONE_PRO_HEADER);
	
	/*3.send packet*/
	return XiWei_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}

static int32 xiwei_connect_request(TERMINAL_BASE* terext)
{
	ST_HF_MGW_PHONE_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_CONNECT_REQ connect_req;
	int tmp_len;

	connect_req.id = MGW_SCU_CONNECT_ID;
	connect_req.len = sizeof(ST_HF_MGW_SCU_CONNECT_REQ) - 2;
	connect_req.slot = terext->bslot_us;
	connect_req.call_type = terext->bCallType;
	connect_req.encoder = 0x04;
	connect_req.phone_len = terext->phone_queue_g.bPhoneQueueHead;
	phone2bcd((char *)connect_req.caller_num,(char *)terext->phone_queue_g.bPhoneQueue,\
		connect_req.phone_len);

	tmp_len = sizeof(ST_HF_MGW_SCU_CONNECT_REQ);
	
	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_CONNECT_REQ);
	memcpy(st_hf_mgw_scu_pro.body,&connect_req,sizeof(ST_HF_MGW_SCU_CONNECT_REQ));

	tmp_len += sizeof(ST_HF_MGW_PHONE_PRO_HEADER);
	
	/*3.send packet*/
	return XiWei_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}

static int32 xiwei_release_request(TERMINAL_BASE* terext)
{
	ST_HF_MGW_PHONE_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_RELEASE_REQ release_req;
	int tmp_len;

	/**/
	printf("%s terext->bslot_us = %d\r\n", __func__, terext->bslot_us);
	
	release_req.id = MGW_SCU_RELEASE_ID;
	release_req.len = sizeof(ST_HF_MGW_SCU_RELEASE_REQ)-2;
	release_req.slot = terext->bslot_us;
	release_req.call_type = terext->bCallType;

	release_req.rel_reason = terext->rel_reason;

	tmp_len = sizeof(ST_HF_MGW_SCU_RELEASE_REQ);
	
	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_RELEASE_REQ);
	memcpy(st_hf_mgw_scu_pro.body,&release_req,sizeof(ST_HF_MGW_SCU_RELEASE_REQ));

	tmp_len += sizeof(ST_HF_MGW_PHONE_PRO_HEADER);

	/*3.send packet*/
	return XiWei_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}

static int32 xiwei_release_cnf_request(TERMINAL_BASE* terext)
{
	ST_HF_MGW_PHONE_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_RELEASE_CNF_REQ release_cnf_req;
	int tmp_len;

	printf("%s terext->bslot_us = %d\r\n", __func__, terext->bslot_us);
	
	release_cnf_req.id = MGW_SCU_RELEASE_CNF_ID;
	release_cnf_req.len = sizeof(ST_HF_MGW_SCU_RELEASE_CNF_REQ)-2;
	release_cnf_req.slot = terext->bslot_us;
	release_cnf_req.call_type = terext->bCallType;

	tmp_len = sizeof(ST_HF_MGW_SCU_RELEASE_CNF_REQ);

	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_RELEASE_CNF_REQ);
	memcpy(st_hf_mgw_scu_pro.body,&release_cnf_req,sizeof(ST_HF_MGW_SCU_RELEASE_CNF_REQ));

	tmp_len += sizeof(ST_HF_MGW_PHONE_PRO_HEADER);

	return XiWei_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}

static int32 xiwei_ptt_request(TERMINAL_BASE * terext)
{
	ST_HF_MGW_PHONE_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_PTT_MSG ptt_msg;
	int tmp_len;
	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));
	memset(&ptt_msg, 0x00, sizeof(ptt_msg));

	if(terminal_group[terext ->connect_port].ptt_status \
		== terminal_group[terext ->connect_port].ptt_status_save)
	{
		return DRV_OK;
	}
	else
	{
		terminal_group[terext ->connect_port].ptt_status_save \
			= terminal_group[terext ->connect_port].ptt_status;
	}
	
	ptt_msg.id = MGW_SCU_PTT_ID;
	ptt_msg.len = 2;
	ptt_msg.slot = terext->bslot_us;

	phone2bcd((char *)ptt_msg.peer_num, (char *)terext->name, MAXPHONENUMBYTE);

	if(terminal_group[terext ->connect_port].ptt_status == MGW_PTT_DOWN)
	{
		ptt_msg.ptt_status = PTT_STATE_SEND;
	}
	else if(terminal_group[terext ->connect_port].ptt_status == MGW_PTT_UP)
	{	
		ptt_msg.ptt_status = PTT_STATE_RECV;
	}
	
	tmp_len = 4;

	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = tmp_len;
	memcpy(st_hf_mgw_scu_pro.body,&ptt_msg, tmp_len);

	tmp_len += sizeof(ST_HF_MGW_PHONE_PRO_HEADER);

	return XiWei_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}

static int32 adp_phone_call_request(TERMINAL_BASE* terext)
{
	ST_ADP_PHONE_SEM st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_CALL_REQ call_req;
	int tmp_len;
	//char *tmp_dup_str;

	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));
	memset(&call_req, 0x00, sizeof(call_req));	
	
	/*1.packet call request*/
	call_req.id = EN_TYPE_CALL_REQ;
	call_req.len = sizeof(ST_HF_MGW_SCU_CALL_REQ) - 2;
	call_req.slot = terext->bslot_us;
	call_req.call_type = terext->bCallType;
	
	call_req.encoder = 0x04;	/*默认编码格式*/
	call_req.phone_len = terext->phone_queue_g.bPhoneQueueHead;
	phone2bcd((char *)call_req.callee_num, (char *)terext->phone_queue_g.bPhoneQueue,\
		call_req.phone_len);

	tmp_len = sizeof(ST_HF_MGW_SCU_CALL_REQ);

	call_req.callee_num[4] = 0xFF;
	call_req.callee_num[5] = 0xFF;
	call_req.callee_num[6] = 0xFF;
	call_req.callee_num[7] = 0xFF;
	call_req.callee_num[8] = 0xFF;
	call_req.callee_num[9] = 0xFF;
		
	/*2.packet complete protocol*/
	st_hf_mgw_scu_pro.Head.ProType = 0X01;
	st_hf_mgw_scu_pro.Head.Edition = 0X01;
	st_hf_mgw_scu_pro.Head.SemType = ZJCT_SEM_MSG;
	st_hf_mgw_scu_pro.Head.Len = sizeof(ST_HF_MGW_SCU_CALL_REQ);
	memcpy(st_hf_mgw_scu_pro.Data,&call_req,sizeof(ST_HF_MGW_SCU_CALL_REQ));

	tmp_len += sizeof(ST_ADP_PHONE_SEM_HEAD);

	/*3.send packet*/
	return Adp_Phone_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}


static int32 adp_phone_callack_request(TERMINAL_BASE* terext)
{
	ST_ADP_PHONE_SEM st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_CALLACK_REQ callack_req;
	int tmp_len;
	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));
	memset(&callack_req, 0x00, sizeof(callack_req));
	
	callack_req.id = EN_TYPE_CALL_ANS;
	callack_req.len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ) - 2;
	callack_req.slot = terext->bslot_us;
	callack_req.call_type = terext->bCallType;

	if(terext->bCommand == MC_RT_TONE)
	{		
		callack_req.call_status = EN_JIEXU_SUCCESS;
	}
	else if(terext->bCommand == MC_REJECT)
	{
		callack_req.call_status = EN_JIEXU_FAILURE;
	}
	else if(terext->bCommand == MC_TALK)
	{
		callack_req.call_status = EN_JIEXU_SUCCESS;
	}
	else if(terext->bCommand == MC_NONUM)
	{
		callack_req.call_status = EN_JIEXU_NONUM;
	}
	else if(terext->bCommand == MC_NUMERROR)
	{
		callack_req.call_status = EN_JIEXU_NUMERROR;
	}
			
	tmp_len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ);
	
	st_hf_mgw_scu_pro.Head.ProType = 0X01;
	st_hf_mgw_scu_pro.Head.Edition = 0X01;
	st_hf_mgw_scu_pro.Head.SemType = ZJCT_SEM_MSG;
	st_hf_mgw_scu_pro.Head.Len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ);
	memcpy(st_hf_mgw_scu_pro.Data,&callack_req,sizeof(ST_HF_MGW_SCU_CALLACK_REQ));

	tmp_len += sizeof(ST_ADP_PHONE_SEM_HEAD);
	
	/*3.send packet*/
	return Adp_Phone_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}

static int32 adp_phone_connect_request(TERMINAL_BASE* terext)
{
	ST_ADP_PHONE_SEM st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_CONNECT_REQ connect_req;
	int tmp_len;

	connect_req.id = EN_TYPE_CONNECT_REQ;
	connect_req.len = sizeof(ST_HF_MGW_SCU_CONNECT_REQ) - 2;
	connect_req.slot = terext->bslot_us;
	connect_req.call_type = terext->bCallType;

	connect_req.encoder = 0x04;
	connect_req.phone_len = terext->phone_queue_g.bPhoneQueueHead;
	phone2bcd((char *)connect_req.caller_num,(char *)terext->phone_queue_g.bPhoneQueue,\
		connect_req.phone_len);

	tmp_len = sizeof(ST_HF_MGW_SCU_CONNECT_REQ);
	
	st_hf_mgw_scu_pro.Head.ProType = 0X01;
	st_hf_mgw_scu_pro.Head.Edition = 0X01;
	st_hf_mgw_scu_pro.Head.SemType = ZJCT_SEM_MSG;
	st_hf_mgw_scu_pro.Head.Len = sizeof(ST_HF_MGW_SCU_CONNECT_REQ);
	memcpy(st_hf_mgw_scu_pro.Data,&connect_req,sizeof(ST_HF_MGW_SCU_CONNECT_REQ));

	tmp_len += sizeof(ST_ADP_PHONE_SEM_HEAD);
	
	/*3.send packet*/
	return  Adp_Phone_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}

static int32 adp_phone_release_request(TERMINAL_BASE* terext)
{
	ST_ADP_PHONE_SEM st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_RELEASE_REQ release_req;
	int tmp_len;
	/**/
	printf("%s terext->bslot_us = %d\r\n", __func__, terext->bslot_us);
	
	release_req.id = EN_TYPE_RELEASE_REQ;
	release_req.len = sizeof(ST_HF_MGW_SCU_RELEASE_REQ)-2;
	release_req.slot = terext->bslot_us;
	release_req.call_type = terext->bCallType;
	release_req.rel_reason = terext->rel_reason;

	tmp_len = sizeof(ST_HF_MGW_SCU_RELEASE_REQ);
	
	st_hf_mgw_scu_pro.Head.ProType = 0X01;
	st_hf_mgw_scu_pro.Head.Edition = 0X01;
	st_hf_mgw_scu_pro.Head.SemType = ZJCT_SEM_MSG;
	st_hf_mgw_scu_pro.Head.Len = sizeof(ST_HF_MGW_SCU_RELEASE_REQ);
	memcpy(st_hf_mgw_scu_pro.Data,&release_req,sizeof(ST_HF_MGW_SCU_RELEASE_REQ));

	tmp_len += sizeof(ST_ADP_PHONE_SEM_HEAD);

	/*3.send packet*/
	return Adp_Phone_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}

static int32 adp_phone_release_cnf_request(TERMINAL_BASE* terext)
{
	ST_ADP_PHONE_SEM st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_RELEASE_CNF_REQ release_cnf_req;
	int tmp_len;
	printf("%s terext->bslot_us = %d\r\n", __func__, terext->bslot_us);
	
	release_cnf_req.id = EN_TYPE_RELEASE_ANS;
	release_cnf_req.len = sizeof(ST_HF_MGW_SCU_RELEASE_CNF_REQ)-2;

	release_cnf_req.slot = terext->bslot_us;
	release_cnf_req.call_type = terext->bCallType;

	tmp_len = sizeof(ST_HF_MGW_SCU_RELEASE_CNF_REQ);

	st_hf_mgw_scu_pro.Head.ProType = 0X01;
	st_hf_mgw_scu_pro.Head.Edition = 0X01;
	st_hf_mgw_scu_pro.Head.SemType = ZJCT_SEM_MSG;
	st_hf_mgw_scu_pro.Head.Len = sizeof(ST_HF_MGW_SCU_RELEASE_CNF_REQ);
	memcpy(st_hf_mgw_scu_pro.Data,&release_cnf_req,sizeof(ST_HF_MGW_SCU_RELEASE_CNF_REQ));

	tmp_len += sizeof(ST_ADP_PHONE_SEM_HEAD);

	return Adp_Phone_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}

static int32 adp_phone_ptt_request(TERMINAL_BASE * terext)
{
	ST_ADP_PHONE_SEM st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_PTT_MSG ptt_msg;
	int tmp_len;
	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));
	memset(&ptt_msg, 0x00, sizeof(ptt_msg));

	if(terminal_group[terext ->connect_port].ptt_status \
		== terminal_group[terext ->connect_port].ptt_status_save)
	{
		return DRV_OK;
	}
	else
	{
		terminal_group[terext ->connect_port].ptt_status_save \
			= terminal_group[terext ->connect_port].ptt_status;
	}
	
	ptt_msg.id = EN_TYPE_PTT;
	ptt_msg.len = 2;
	ptt_msg.slot = terext->bslot_us;

	if(terminal_group[terext ->connect_port].ptt_status == MGW_PTT_DOWN)
	{
		ptt_msg.ptt_status = PTT_STATE_SEND;
	}
	else if(terminal_group[terext ->connect_port].ptt_status == MGW_PTT_UP)
	{	
		ptt_msg.ptt_status = PTT_STATE_RECV;
	}		

	tmp_len = 4;

	st_hf_mgw_scu_pro.Head.ProType = 0X01;
	st_hf_mgw_scu_pro.Head.Edition = 0X01;
	st_hf_mgw_scu_pro.Head.SemType = ZJCT_SEM_MSG;
	st_hf_mgw_scu_pro.Head.Len = tmp_len;
	memcpy(st_hf_mgw_scu_pro.Data,&ptt_msg, tmp_len);

	tmp_len += sizeof(ST_ADP_PHONE_SEM_HEAD);

	return Adp_Phone_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}


static int32 adp_phone_transphone_request(TERMINAL_BASE* terext)
{
	ST_ADP_PHONE_SEM st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_NUM_TRANS_MSG num_trans_msg;
	
	int tmp_len;

	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));
	memset(&num_trans_msg, 0x00, sizeof(num_trans_msg));	
	
	/*1.packet call request*/
	num_trans_msg.id = EN_TYPE_NUM_TRANS;
	num_trans_msg.len = sizeof(ST_HF_MGW_SCU_NUM_TRANS_MSG) - 2;
	num_trans_msg.slot = terext->bslot_us;
	num_trans_msg.call_type = terext->bCallType;

	memset(num_trans_msg.trans_phone, 0xFF, 10);

	num_trans_msg.phone_len = terext->phone_queue_s.bPhoneQueueHead;
	phone2bcd((char *)num_trans_msg.trans_phone, (char *)terext->phone_queue_s.bPhoneQueue,\
		num_trans_msg.phone_len);

	tmp_len = sizeof(ST_HF_MGW_SCU_NUM_TRANS_MSG);

	/*2.packet complete protocol*/
	st_hf_mgw_scu_pro.Head.ProType = 0X01;
	st_hf_mgw_scu_pro.Head.Edition = 0X01;
	st_hf_mgw_scu_pro.Head.SemType = ZJCT_SEM_MSG;
	st_hf_mgw_scu_pro.Head.Len = sizeof(ST_HF_MGW_SCU_NUM_TRANS_MSG);
	memcpy(st_hf_mgw_scu_pro.Data,&num_trans_msg,sizeof(ST_HF_MGW_SCU_NUM_TRANS_MSG));

	tmp_len += sizeof(ST_ADP_PHONE_SEM_HEAD);

	/*3.send packet*/
	return Adp_Phone_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}

/*=============================呼叫控制==================================*/
static int32 seat_ext_call_request(TERMINAL_BASE* terext)
{
	unsigned short tmp_len;
	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_CALL_REQ call_req;
	
	/*1.Packet user call req*/
	call_req.id = EN_TYPE_CALL_REQ;
	call_req.msg_len = sizeof(ST_HF_MGW_EXT_CALL_REQ)-2;
	call_req.slot = terext->bslot_us;
	call_req.encoder = ENCODER_G711;
	call_req.call_type = terext->bCallType;
	call_req.phone_len = terext->phone_queue_g.bPhoneQueueHead;
	phone2bcd((char *)call_req.callee_num, \
		(char *)terext->phone_queue_g.bPhoneQueue,call_req.phone_len);

	/*2.Packet MGW_EXT_PRO*/
	st_hf_mgw_ext_pro.header.sync_head = ntohs(0x55aa);
	st_hf_mgw_ext_pro.header.type = BAOWEN_TYPE_CALLCMD;
	st_hf_mgw_ext_pro.header.reserve = ntohs(0x0000);
	tmp_len = sizeof(ST_HF_MGW_EXT_CALL_REQ);
	st_hf_mgw_ext_pro.header.len = ntohs(tmp_len);
	memcpy(st_hf_mgw_ext_pro.body,&call_req,tmp_len);
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+
	sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;

	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*增加校验的两个字节*/

	/*3.Sent to Host*/
	return Seat_Data_Send_by_IP((uint8 *)&st_hf_mgw_ext_pro, tmp_len, terext->ipaddr);
}

static int32 seat_ext_callack_request(TERMINAL_BASE* terext)
{
	unsigned short tmp_len;

	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_CALLACK_REQ callack_req;
	/*1.Packet user callack req*/
	callack_req.id = EN_TYPE_CALL_ANS;
	callack_req.msg_len = sizeof(ST_HF_MGW_EXT_CALLACK_REQ)-2;
	callack_req.slot = terext->bslot_us;
	callack_req.call_type = terext->bCallType;
	
	if(terext->bCommand == MC_RT_TONE)
	{
		callack_req.call_statu = EN_JIEXU_SUCCESS;
	}
	else if(terext->bCommand == MC_REJECT)
	{
		callack_req.call_statu = EN_JIEXU_FAILURE;
	}
	else if(terext->bCommand == MC_TALK)
	{
		callack_req.call_statu = EN_JIEXU_SUCCESS;
	}
	
	/*2.Packet MGW_EXT_PRO*/
	st_hf_mgw_ext_pro.header.sync_head = ntohs(0x55aa);
	st_hf_mgw_ext_pro.header.type = BAOWEN_TYPE_CALLCMD;
	st_hf_mgw_ext_pro.header.reserve = ntohs(0x0000);
	tmp_len = sizeof(ST_HF_MGW_EXT_CALLACK_REQ);
	st_hf_mgw_ext_pro.header.len = ntohs(tmp_len);
	memcpy(st_hf_mgw_ext_pro.body,&callack_req,tmp_len);
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+
	sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;

	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*增加校验的两个字节*/

	/*3.Sent to Host*/
	return Seat_Data_Send_by_IP((uint8 *)&st_hf_mgw_ext_pro, tmp_len, terext->ipaddr);	

}

static int32 seat_ext_connect_request(TERMINAL_BASE* terext)
{
	unsigned short tmp_len;

	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_CONNECT_REQ connect_req;
	
	/*1.Packet user connect req*/
	connect_req.id = EN_TYPE_CONNECT_REQ;
	connect_req.msg_len = sizeof(ST_HF_MGW_EXT_CONNECT_REQ)-2;
	connect_req.slot = terext->bslot_us;
	connect_req.encoder = ENCODER_G711;
	connect_req.call_type = terext->bCallType;
	connect_req.rtp_port = get_random_port();
	terext->rtp_socket.local_port = connect_req.rtp_port;
	connect_req.phone_len = terext->phone_queue_g.bPhoneQueueHead;
	phone2bcd((char *)connect_req.caller_num, \
		(char *)terext->phone_queue_g.bPhoneQueue,connect_req.phone_len);

	/*2.Packet MGW_EXT_PRO*/
	st_hf_mgw_ext_pro.header.sync_head = ntohs(0x55aa);
	st_hf_mgw_ext_pro.header.type = BAOWEN_TYPE_CALLCMD;
	st_hf_mgw_ext_pro.header.reserve = ntohs(0x0000);
	tmp_len = sizeof(ST_HF_MGW_EXT_CONNECT_REQ);
	st_hf_mgw_ext_pro.header.len = ntohs(tmp_len);
	memcpy(st_hf_mgw_ext_pro.body,&connect_req,tmp_len);
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+
	sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;

	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*增加校验的两个字节*/

	/*4.Sent to Host*/
	return Seat_Data_Send_by_IP((uint8 *)&st_hf_mgw_ext_pro, tmp_len, terext->ipaddr);
}

static int32 seat_ext_release_request(TERMINAL_BASE* terext)
{
	unsigned short tmp_len;

	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_RELEASE_REQ release_req;
	
	/*1.Packet user connect req*/
	release_req.id = EN_TYPE_RELEASE_REQ;
	release_req.msg_len = sizeof(ST_HF_MGW_EXT_RELEASE_REQ)-2;
	release_req.slot = terext->bslot_us;
	release_req.call_type = terext->bCallType;
	release_req.rel_type = terext->rel_reason;

	/*2.Packet MGW_EXT_PRO*/
	st_hf_mgw_ext_pro.header.sync_head = ntohs(0x55aa);
	st_hf_mgw_ext_pro.header.type = BAOWEN_TYPE_CALLCMD;
	st_hf_mgw_ext_pro.header.reserve = ntohs(0x0000);
	tmp_len = sizeof(ST_HF_MGW_EXT_RELEASE_REQ);
	st_hf_mgw_ext_pro.header.len = ntohs(tmp_len);
	memcpy(st_hf_mgw_ext_pro.body,&release_req,tmp_len);
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+
		sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;

	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*增加校验的两个字节*/

	/*3.Sent to Host*/
	return Seat_Data_Send_by_IP((uint8 *)&st_hf_mgw_ext_pro, tmp_len, terext->ipaddr);
}

static int32 seat_ext_release_cnf_request(TERMINAL_BASE* terext)
{
	unsigned short tmp_len;

	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_RELEASE_CNF release_cnf;
	
	/*1.Packet user connect req*/
	release_cnf.id = EN_TYPE_RELEASE_ANS;
	release_cnf.msg_len = sizeof(ST_HF_MGW_EXT_RELEASE_CNF)-2;
	release_cnf.slot = terext->bslot_us;
	release_cnf.call_type = terext->bCallType;

	/*2.Packet MGW_EXT_PRO*/
	st_hf_mgw_ext_pro.header.sync_head = ntohs(0x55aa);
	st_hf_mgw_ext_pro.header.type = BAOWEN_TYPE_CALLCMD;
	st_hf_mgw_ext_pro.header.reserve = ntohs(0x0000);
	tmp_len = sizeof(ST_HF_MGW_EXT_RELEASE_CNF);
	st_hf_mgw_ext_pro.header.len = ntohs(tmp_len);
	memcpy(st_hf_mgw_ext_pro.body,&release_cnf,tmp_len);
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+
	sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;

	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*增加校验的两个字节*/

	/*3.Sent to Host*/
	return Seat_Data_Send_by_IP((uint8 *)&st_hf_mgw_ext_pro, tmp_len, terext->ipaddr);
}


static int32 kuozhan_single_call_request(TERMINAL_BASE* terext)
{
	int wTemPort = 0;
	int speed = -1;
	
	speed = get_config_var_val(mgw_cfg, "gport_speed");
	
	wTemPort =  find_port_by_phone_number(terext);
	if(IDLE == wTemPort)
	{
		printf("%s error port = %d\r\n", __func__, wTemPort);
		terext->bCommand = MC_NONUM;
		return DRV_ERR;
	}
	else
	{
		printf("%s: port = %d\r\n", __func__, wTemPort);
	}

	if(0 == terminal_group[wTemPort].busy_flg)
	{
		switch(terminal_group[wTemPort].bPortType)
		{
			case PORT_TYPE_QINWU:
#if 0			
				if((PORT_TYPE_ADP_PHONE == terext->bPortType)\
					&&(speed == CTL_GINTF_KUOZHAN_128K)\
					&&(0 != adapter_phone_cnt_talk))
				{
					terext->bCommand = MC_REJECT;
					printf("%s: port = %d, speed = %d, talk_cnt = %d\r\n", \
						__func__, terext->wPort, speed, adapter_phone_cnt_talk);
					break;
				}
				else
				{
					adapter_phone_cnt_talk++;
				}
#endif				
				terminal_group[wTemPort].bCommand = MC_RING;
				terminal_group[wTemPort].bCallType = EN_SEL_CALL;
				memcpy(terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
					terext->name, terext->user_num_len);
				terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = terext->user_num_len;
				terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;		
				terminal_group[wTemPort].connect_port = terext->wPort;
				terminal_group[wTemPort].busy_flg = 1;		

				terext->connect_port = wTemPort;
				terext->bCommand = MC_RT_TONE;		
				break;
			case PORT_TYPE_TRK:
				terminal_group[wTemPort].bCommand = MC_USETRK;
				terminal_group[wTemPort].bCallType = EN_SEL_CALL;

				memcpy(terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
					terext->name, terext->user_num_len);
				terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = terext->user_num_len;
				terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;	

				terminal_group[wTemPort].connect_port = terext->wPort;
				terminal_group[wTemPort].busy_flg = 1;	
				
				terext->connect_port = wTemPort;
				terext->bCommand = MC_RT_TONE;				
				break;				
			case PORT_TYPE_KUOZHAN_PHONE:
#if 0			
				if((PORT_TYPE_ADP_PHONE == terext->bPortType)\
					&&(speed == CTL_GINTF_KUOZHAN_128K)\
					&&(0 != adapter_phone_cnt_talk))
				{
					terext->bCommand = MC_REJECT;
					printf("%s: port = %d, speed = %d, talk_cnt = %d\r\n", \
						__func__, terext->wPort, speed, adapter_phone_cnt_talk);
					break;
				}
				else
				{
					adapter_phone_cnt_talk++;
				}
#endif				
				terminal_group[wTemPort].bCommand = MC_RING;
				terminal_group[wTemPort].bCallType = EN_SEL_CALL;
				memcpy(terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
					terext->name, terext->user_num_len);
				terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = terext->user_num_len;
				terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;	
				terminal_group[wTemPort].connect_port = terext->wPort;
				terminal_group[wTemPort].busy_flg = 1;		
				
				terext->connect_port = wTemPort;
				terext->bCommand = MC_RT_TONE;	
				break;
			case PORT_TYPE_KUOZHAN_XIWEI:
				if(0 == terminal_group[wTemPort].sw_phone_online)
				{
					terext->bCommand = MC_REJECT;
					break;
				}
#if 0
				if((PORT_TYPE_ADP_PHONE == terext->bPortType)\
					&&(speed == CTL_GINTF_KUOZHAN_128K)\
					&&(0 != adapter_phone_cnt_talk))
				{
					terext->bCommand = MC_REJECT;
					printf("%s: port = %d, speed = %d, talk_cnt = %d\r\n", \
						__func__, terext->wPort, speed, adapter_phone_cnt_talk);
					break;
				}
				else
				{
					adapter_phone_cnt_talk++;
				}				
#endif				
				terminal_group[wTemPort].bCommand = MC_RING;
				terminal_group[wTemPort].bCallType = EN_SEL_CALL;
				memcpy(terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
					terext->name, terext->user_num_len);
				terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = terext->user_num_len;
				terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;		
				terminal_group[wTemPort].connect_port = terext->wPort;
				terminal_group[wTemPort].busy_flg = 1;	
				
				terext->connect_port = wTemPort;
				terext->bCommand = MC_RT_TONE;			
				break;
			case PORT_TYPE_ADP_PHONE:
				if(0 == terminal_group[wTemPort].adp_phone_online)
				{
					terext->bCommand = MC_REJECT;
					break;
				}
#if 0
				if((speed == CTL_GINTF_KUOZHAN_128K)\
					&&(0 != adapter_phone_cnt_talk))
				{
					terext->bCommand = MC_REJECT;
					printf("%s: port = %d, speed = %d, talk_cnt = %d\r\n", \
						__func__, terext->wPort, speed, adapter_phone_cnt_talk);
					break;
				}
				else
				{
					adapter_phone_cnt_talk++;
				}
#endif
				terminal_group[wTemPort].bCommand = MC_RING;
				terminal_group[wTemPort].bCallType = EN_SEL_CALL;
				
				memcpy(terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
					terext->name, terext->user_num_len);
				terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = terext->user_num_len;
				terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;	

				terminal_group[wTemPort].connect_port = terext->wPort;
				terminal_group[wTemPort].busy_flg = 1;		
				
				terext->connect_port = wTemPort;
				terext->bCommand = MC_RT_TONE;					
				break;				
			default:
				break;
		}
	}
	else if(1 == terminal_group[wTemPort].busy_flg)
	{
		switch(terext->bPortType)
		{
			case PORT_TYPE_QINWU:
				terext->bCommand = MC_REJECT;
				break;
			case PORT_TYPE_TRK:
				terext->bCommand = MC_REJECT;
				break;				
			case PORT_TYPE_KUOZHAN_PHONE:
				terext->bCommand = MC_REJECT;
				break;
			case PORT_TYPE_KUOZHAN_XIWEI:
				terext->bCommand = MC_REJECT;
				break;
			case PORT_TYPE_ADP_PHONE:
				terext->bCommand = MC_REJECT;
				break;				
			default:
				break;
		}
	}

	return DRV_OK;
}


static int32 kuozhan_single_callack_request(TERMINAL_BASE* terext)
{
	int wTemPort = 0;

	wTemPort =  terext ->connect_port;
	if(IDLE == wTemPort)
	{
		printf("%s error port = %d\r\n", __func__, wTemPort);
		return DRV_ERR;
	}

	if(1 == terminal_group[wTemPort].busy_flg)
	{
		switch(terminal_group[wTemPort].bPortType)
		{
			case PORT_TYPE_QINWU:		
				break;
			case PORT_TYPE_TRK:
				break;				
			case PORT_TYPE_KUOZHAN_PHONE:
				break;
			case PORT_TYPE_KUOZHAN_XIWEI:
				break;
			case PORT_TYPE_ADP_PHONE:
				break;				
			default:
				break;
		}
	}

	return DRV_OK;
}

static int32 kuozhan_single_connect_request(TERMINAL_BASE* terext)
{
	int wTemPort = 0;

	wTemPort =  terext ->connect_port;
	if(IDLE == wTemPort)
	{
		printf("%s error port = %d\r\n", __func__, wTemPort);
		return DRV_ERR;
	}

	if(1 == terminal_group[wTemPort].busy_flg)
	{
		switch(terminal_group[wTemPort].bPortType)
		{
			case PORT_TYPE_QINWU:
				terminal_group[wTemPort].bCommand = MC_TALK;
				break;	
			case PORT_TYPE_TRK:
				terminal_group[wTemPort].bCommand = MC_TALK;
				break;					
			case PORT_TYPE_KUOZHAN_PHONE:
				terminal_group[wTemPort].bCommand = MC_TALK;
				break;
			case PORT_TYPE_KUOZHAN_XIWEI:
				terminal_group[wTemPort].bCommand = MC_TALK;
				break;
			case PORT_TYPE_ADP_PHONE:
				terminal_group[wTemPort].bCommand = MC_TALK;
				break;				
			default:
				break;
		}
	}

	return DRV_OK;
}

static int32 kuozhan_single_release_request(TERMINAL_BASE* terext)
{
	int wTemPort = 0;

	wTemPort =  terext ->connect_port;
	if(IDLE == wTemPort)
	{
		printf("%s error port = %d\r\n", __func__, wTemPort);
		return DRV_ERR;
	}

	if(1 == terminal_group[wTemPort].busy_flg)
	{
		switch(terminal_group[wTemPort].bPortType)
		{
			case PORT_TYPE_QINWU:
				terminal_group[wTemPort].bCommand = MC_HANG;
#if 0				
				if(PORT_TYPE_ADP_PHONE == terext->bPortType)
				{
					if(0 != adapter_phone_cnt_talk)
					{
						adapter_phone_cnt_talk--;
					}
				}
#endif
				break;
			case PORT_TYPE_TRK:
				terminal_group[wTemPort].bCommand = MC_HANG;
#if 0				
				if(PORT_TYPE_ADP_PHONE == terext->bPortType)
				{
					if(0 != adapter_phone_cnt_talk)
					{
						adapter_phone_cnt_talk--;
					}
				}
#endif				
				break;				
			case PORT_TYPE_KUOZHAN_PHONE:
				terminal_group[wTemPort].bCommand = MC_HANG;
#if 0				
				if(PORT_TYPE_ADP_PHONE == terext->bPortType)
				{
					if(0 != adapter_phone_cnt_talk)
					{
						adapter_phone_cnt_talk--;
					}
				}
#endif				
				break;
			case PORT_TYPE_KUOZHAN_XIWEI:
				terminal_group[wTemPort].bCommand = MC_HANG;
#if 0				
				if(PORT_TYPE_ADP_PHONE == terext->bPortType)
				{
					if(0 != adapter_phone_cnt_talk)
					{
						adapter_phone_cnt_talk--;
					}
				}
#endif				
				break;
			case PORT_TYPE_ADP_PHONE:
				terminal_group[wTemPort].bCommand = MC_HANG;
#if 0				
				if(PORT_TYPE_ADP_PHONE == terext->bPortType)
				{
					if(0 != adapter_phone_cnt_talk)
					{
						adapter_phone_cnt_talk--;
					}
				}
#endif				
				break;					
			default:
				break;
		}
	}

	return DRV_OK;
}

static int32 kuozhan_single_release_cnf_request(TERMINAL_BASE* terext)
{
	int wTemPort = 0;

	wTemPort =  terext ->connect_port;
	if(IDLE == wTemPort)
	{
		printf("%s error port = %d\r\n", __func__, wTemPort);
		return DRV_ERR;
	}

	switch(terminal_group[wTemPort].bPortType)
	{
		case PORT_TYPE_QINWU:
			break;	
		case PORT_TYPE_TRK:
			break;			
		case PORT_TYPE_KUOZHAN_PHONE:
			break;
		case PORT_TYPE_KUOZHAN_XIWEI:
			break;
		case PORT_TYPE_ADP_PHONE:
			break;			
		default:
			break;
	}
	
	return DRV_OK;
}


static int32 kuozhan_single_transphone_request(TERMINAL_BASE* terext)
{
	int wTemPort = 0;

	wTemPort =  terext ->connect_port;
	if(IDLE == wTemPort)
	{
		printf("%s error port = %d\r\n", __func__, wTemPort);
		return DRV_ERR;
	}

	switch(terminal_group[wTemPort].bPortType)
	{
		case PORT_TYPE_QINWU:
			break;	
		case PORT_TYPE_TRK:
			memcpy(terminal_group[wTemPort].phone_queue_s.bPhoneQueue, \
				terext->phone_queue_s.bPhoneQueue, terext->phone_queue_s.bPhoneQueueHead);
			terminal_group[wTemPort].phone_queue_s.bPhoneQueueHead \
				= terext->phone_queue_s.bPhoneQueueHead;
			terminal_group[wTemPort].phone_queue_s.bPhoneQueueTail = 0;	
			terminal_group[wTemPort].phone_queue_s.bFinish = 1;
			break;			
		case PORT_TYPE_KUOZHAN_PHONE:
			break;
		case PORT_TYPE_KUOZHAN_XIWEI:
			break;
		case PORT_TYPE_ADP_PHONE:
			memcpy(terminal_group[wTemPort].phone_queue_s.bPhoneQueue, \
				terext->phone_queue_s.bPhoneQueue, terext->phone_queue_s.bPhoneQueueHead);
			terminal_group[wTemPort].phone_queue_s.bPhoneQueueHead \
				= terext->phone_queue_s.bPhoneQueueHead;
			terminal_group[wTemPort].phone_queue_s.bPhoneQueueTail = 0;	
			terminal_group[wTemPort].phone_queue_s.bFinish = 1;		
			break;			
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 kuozhan_single_ptt_request(TERMINAL_BASE * terext)
{
	int wTemPort = 0;

	wTemPort =  terext ->connect_port;
	if(IDLE == wTemPort)
	{
		printf("%s error port = %d\r\n", __func__, wTemPort);
		return DRV_ERR;
	}

	switch(terminal_group[wTemPort].bPortType)
	{
		case PORT_TYPE_QINWU:
			break;
		case PORT_TYPE_TRK:
			if(PORT_TYPE_KUOZHAN_XIWEI == terext->bPortType)
			{
				if(terext ->ptt_status == terext ->ptt_status_save)
				{
					return DRV_OK;
				}
				else
				{
					terext ->ptt_status_save = terext ->ptt_status;
				}
				
				if(terext ->ptt_status == MGW_PTT_DOWN)
				{
					terminal_group[wTemPort].phone_queue_s.bPhoneQueue[0] = '2';
					terminal_group[wTemPort].phone_queue_s.bPhoneQueue[0] = '*';
					terminal_group[wTemPort].phone_queue_s.bPhoneQueueHead  = 2;
					terminal_group[wTemPort].phone_queue_s.bPhoneQueueTail = 0;	
					terminal_group[wTemPort].phone_queue_s.bFinish = 1;	
				}
				else if(terext ->ptt_status == MGW_PTT_UP)
				{	
					terminal_group[wTemPort].phone_queue_s.bPhoneQueue[0] = '3';
					terminal_group[wTemPort].phone_queue_s.bPhoneQueue[0] = '#';
					terminal_group[wTemPort].phone_queue_s.bPhoneQueueHead  = 2;
					terminal_group[wTemPort].phone_queue_s.bPhoneQueueTail = 0;	
					terminal_group[wTemPort].phone_queue_s.bFinish = 1;	
				}
			}
			break;
		case PORT_TYPE_KUOZHAN_PHONE:
			break;
		case PORT_TYPE_KUOZHAN_XIWEI:
			if(MGW_PTT_UP == terminal_group[wTemPort].ptt_status)
			{
				xiwei_ptt_request(&terminal_group[wTemPort]);
			}
		case PORT_TYPE_ADP_PHONE:
			if(MGW_PTT_UP == terminal_group[wTemPort].ptt_status)
			{
				adp_phone_ptt_request(&terminal_group[wTemPort]);
			}
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 kuozhan_conf_call_request(TERMINAL_BASE* terext)
{
	int i = 0, id = 0;
	int wTemPort = 0;
	int speed = -1;
	speed = get_config_var_val(mgw_cfg, "gport_speed");

	id =  find_port_by_conf_number(terext);
	if(MC_NONUM == id)
	{
		printf("%s error conf id = %d\r\n", __func__, id);
		terext->bCommand = MC_NONUM;
		return DRV_ERR;
	}
	else if(MC_NUMERROR == id)
	{
		printf("%s error conf no member %s\r\n", __func__, terext->name);
		terext->bCommand = MC_REJECT;
		return DRV_ERR;
	}
	else
	{
		printf("%s: conf id = %d\r\n", __func__, id);
	}

	terext->conf_id = id;
	terext->connect_port_cnt = 0x00;
	terext->connect_port = 0xFF;
	
	for(i = 0; i < kuozhan_conf[id].conf_cnt; i++)
	{
		wTemPort =  find_port_by_number(kuozhan_conf[id].conf_phone[i]);

		if(0 == terminal_group[wTemPort].busy_flg)
		{
			switch(terminal_group[wTemPort].bPortType)
			{
				case PORT_TYPE_QINWU:
#if 0				
					if((PORT_TYPE_ADP_PHONE == terext->bPortType)\
						&&(speed == CTL_GINTF_KUOZHAN_128K)\
						&&(0 != adapter_phone_cnt_talk))
					{
						printf("%s: port = %d, speed = %d, talk_cnt = %d\r\n", \
							__func__, terext->wPort, speed, adapter_phone_cnt_talk);
						break;
					}
					else
					{
						adapter_phone_cnt_talk++;
					}
#endif					
					terminal_group[wTemPort].bCommand = MC_RING;
					terminal_group[wTemPort].bCallType = EN_PLAN_CONF;
					memcpy(terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
						terext->name, terext->user_num_len);
					terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = terext->user_num_len;
					terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;		
					terminal_group[wTemPort].connect_port = terext->wPort;
					terminal_group[wTemPort].busy_flg = 1;	
					terminal_group[wTemPort].conf_id = id;	

					terext->connect_port_flg[wTemPort] = 1;	
					terext->connect_port_array[terext->connect_port_cnt] = wTemPort;
					terext->connect_port_cnt++;						
					break;
				case PORT_TYPE_TRK:
					terminal_group[wTemPort].bCommand = MC_USETRK;
					terminal_group[wTemPort].bCallType = EN_PLAN_CONF;

					memcpy(terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
						terext->name, terext->user_num_len);
					terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = terext->user_num_len;
					terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;	

					terminal_group[wTemPort].connect_port = terext->wPort;
					terminal_group[wTemPort].busy_flg = 1;	
					terminal_group[wTemPort].conf_id = id;	

					terext->connect_port_flg[wTemPort] = 1;	
					terext->connect_port_array[terext->connect_port_cnt] = wTemPort;
					terext->connect_port_cnt++;			
					break;				
				case PORT_TYPE_KUOZHAN_PHONE:
#if 0				
					if((PORT_TYPE_ADP_PHONE == terext->bPortType)\
						&&(speed == CTL_GINTF_KUOZHAN_128K)\
						&&(0 != adapter_phone_cnt_talk))
					{
						printf("%s: port = %d, speed = %d, talk_cnt = %d\r\n", \
							__func__, terext->wPort, speed, adapter_phone_cnt_talk);
						break;
					}
					else
					{
						adapter_phone_cnt_talk++;
					}
#endif					
					terminal_group[wTemPort].bCommand = MC_RING;
					terminal_group[wTemPort].bCallType = EN_PLAN_CONF;
					memcpy(terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
						terext->name, terext->user_num_len);
					terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = terext->user_num_len;
					terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;	
					terminal_group[wTemPort].connect_port = terext->wPort;
					terminal_group[wTemPort].busy_flg = 1;	
					terminal_group[wTemPort].conf_id = id;	

					terext->connect_port_flg[wTemPort] = 1;	
					terext->connect_port_array[terext->connect_port_cnt] = wTemPort;
					terext->connect_port_cnt++;	
					break;
				case PORT_TYPE_KUOZHAN_XIWEI:
					if(0 == terminal_group[wTemPort].sw_phone_online)
					{
						break;
					}
#if 0
					if((PORT_TYPE_ADP_PHONE == terext->bPortType)\
						&&(speed == CTL_GINTF_KUOZHAN_128K)\
						&&(0 != adapter_phone_cnt_talk))
					{
						printf("%s: port = %d, speed = %d, talk_cnt = %d\r\n", \
							__func__, terext->wPort, speed, adapter_phone_cnt_talk);
						break;
					}
					else
					{
						adapter_phone_cnt_talk++;
					}				
#endif					
					terminal_group[wTemPort].bCommand = MC_RING;
					terminal_group[wTemPort].bCallType = EN_PLAN_CONF;
					memcpy(terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
						terext->name, terext->user_num_len);
					terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = terext->user_num_len;
					terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;		
					terminal_group[wTemPort].connect_port = terext->wPort;
					terminal_group[wTemPort].busy_flg = 1;	
					terminal_group[wTemPort].conf_id = id;	

					terext->connect_port_flg[wTemPort] = 1;	
					terext->connect_port_array[terext->connect_port_cnt] = wTemPort;
					terext->connect_port_cnt++;	
					break;
				case PORT_TYPE_ADP_PHONE:
					if(0 == terminal_group[wTemPort].adp_phone_online)
					{
						break;
					}
#if 0
					if((speed == CTL_GINTF_KUOZHAN_128K)\
						&&(0 != adapter_phone_cnt_talk))
					{
						printf("%s: port = %d, speed = %d, talk_cnt = %d\r\n", \
							__func__, terext->wPort, speed, adapter_phone_cnt_talk);
						break;
					}
					else
					{
						adapter_phone_cnt_talk++;
					}
#endif
					terminal_group[wTemPort].bCommand = MC_RING;
					terminal_group[wTemPort].bCallType = EN_PLAN_CONF;
					
					memcpy(terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
						terext->name, terext->user_num_len);
					terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = terext->user_num_len;
					terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;	

					terminal_group[wTemPort].connect_port = terext->wPort;
					terminal_group[wTemPort].busy_flg = 1;	
					terminal_group[wTemPort].conf_id = id;	

					terext->connect_port_flg[wTemPort] = 1;	
					terext->connect_port_array[terext->connect_port_cnt] = wTemPort;
					terext->connect_port_cnt++;			
					break;				
				default:
					break;
			}
		}
		else if(1 == terminal_group[wTemPort].busy_flg)
		{
			switch(terext->bPortType)
			{
				case PORT_TYPE_QINWU:
					break;
				case PORT_TYPE_TRK:
					break;				
				case PORT_TYPE_KUOZHAN_PHONE:
					break;
				case PORT_TYPE_KUOZHAN_XIWEI:
					break;
				case PORT_TYPE_ADP_PHONE:
					break;				
				default:
					break;
			}
		}
	}

	if(0x00 == terext->connect_port_cnt)
	{
		terext->bCommand = MC_REJECT;
	}
	else
	{
		terext->bCommand = MC_RT_TONE;
	}

	return DRV_OK;
}


static int32 kuozhan_conf_callack_request(TERMINAL_BASE* terext)
{
	int wTemPort = 0;

	wTemPort =  terext ->connect_port;
	if(IDLE == wTemPort)
	{
		printf("%s error port = %d\r\n", __func__, wTemPort);
		return DRV_ERR;
	}

	switch(terminal_group[wTemPort].bPortType)
	{
		case PORT_TYPE_QINWU:		
			break;
		case PORT_TYPE_TRK:
			break;				
		case PORT_TYPE_KUOZHAN_PHONE:
			break;
		case PORT_TYPE_KUOZHAN_XIWEI:
			break;
		case PORT_TYPE_ADP_PHONE:
			break;				
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 kuozhan_conf_connect_request(TERMINAL_BASE* terext)
{
	int wTemPort = 0;

	wTemPort =  terext ->connect_port;
	if(IDLE == wTemPort)
	{
		printf("%s error port = %d\r\n", __func__, wTemPort);
		return DRV_ERR;
	}

	switch(terminal_group[wTemPort].bPortType)
	{
		case PORT_TYPE_QINWU:
			kuozhan_conf[terext->conf_id].talk_flg = 1;
			terminal_group[wTemPort].bCommand = MC_TALK;
			break;	
		case PORT_TYPE_TRK:
			kuozhan_conf[terext->conf_id].talk_flg = 1;
			terminal_group[wTemPort].bCommand = MC_TALK;
			break;					
		case PORT_TYPE_KUOZHAN_PHONE:
			kuozhan_conf[terext->conf_id].talk_flg = 1;
			terminal_group[wTemPort].bCommand = MC_TALK;
			break;
		case PORT_TYPE_KUOZHAN_XIWEI:
			kuozhan_conf[terext->conf_id].talk_flg = 1;
			terminal_group[wTemPort].bCommand = MC_TALK;
			break;
		case PORT_TYPE_ADP_PHONE:
			kuozhan_conf[terext->conf_id].talk_flg = 1;
			terminal_group[wTemPort].bCommand = MC_TALK;
			break;				
		default:
			break;
	}

	return DRV_OK;
}

static int32 kuozhan_conf_release_request(TERMINAL_BASE* terext)
{
	int wTemPort = 0;
	int i = 0;
	int flg = 0;

	wTemPort =  terext ->connect_port;
	if(IDLE == wTemPort)
	{
		printf("%s error port = %d\r\n", __func__, wTemPort);
		return DRV_ERR;
	}

	if(0xFF == wTemPort)
	{
		kuozhan_conf[terext->conf_id].talk_flg = 0;
		kuozhan_conf[terext->conf_id].mem_save[terext->wPort] = 0x00;
		kuozhan_conf[terext->conf_id].mem_get[terext->wPort] = 0x00;
		
		for(i = 0; i < KUOZHAN_CONF_MEM_MAX; i++)
		{
			flg = terext->connect_port_flg[i];
			if(0 == flg)
			{
				continue;
			}
			
			switch(terminal_group[i].bPortType)
			{
				case PORT_TYPE_QINWU:
					terminal_group[i].bCommand = MC_HANG;
					kuozhan_conf[terext->conf_id].mem_save[i] = 0x00;
					kuozhan_conf[terext->conf_id].mem_get[i] = 0x00;
					terext->connect_port_flg[i] = 0x00;
#if 0					
					if(PORT_TYPE_ADP_PHONE == terext->bPortType)
					{
						if(0 != adapter_phone_cnt_talk)
						{
							adapter_phone_cnt_talk--;
						}
					}
#endif
					break;
				case PORT_TYPE_TRK:
					terminal_group[i].bCommand = MC_HANG;
					kuozhan_conf[terext->conf_id].mem_save[i] = 0x00;
					kuozhan_conf[terext->conf_id].mem_get[i] = 0x00;					
					terext->connect_port_flg[i] = 0x00;
#if 0					
					if(PORT_TYPE_ADP_PHONE == terext->bPortType)
					{
						if(0 != adapter_phone_cnt_talk)
						{
							adapter_phone_cnt_talk--;
						}
					}				
#endif
					break;				
				case PORT_TYPE_KUOZHAN_PHONE:
					terminal_group[i].bCommand = MC_HANG;
					kuozhan_conf[terext->conf_id].mem_save[i] = 0x00;
					kuozhan_conf[terext->conf_id].mem_get[i] = 0x00;					
					terext->connect_port_flg[i] = 0x00;
#if 0					
					if(PORT_TYPE_ADP_PHONE == terext->bPortType)
					{
						if(0 != adapter_phone_cnt_talk)
						{
							adapter_phone_cnt_talk--;
						}
					}				
#endif
					break;
				case PORT_TYPE_KUOZHAN_XIWEI:
					terminal_group[i].bCommand = MC_HANG;
					kuozhan_conf[terext->conf_id].mem_save[i] = 0x00;
					kuozhan_conf[terext->conf_id].mem_get[i] = 0x00;					
					terext->connect_port_flg[i] = 0x00;
#if 0					
					if(PORT_TYPE_ADP_PHONE == terext->bPortType)
					{
						if(0 != adapter_phone_cnt_talk)
						{
							adapter_phone_cnt_talk--;
						}
					}				
#endif
					break;
				case PORT_TYPE_ADP_PHONE:
					terminal_group[i].bCommand = MC_HANG;
					kuozhan_conf[terext->conf_id].mem_save[i] = 0x00;
					kuozhan_conf[terext->conf_id].mem_get[i] = 0x00;					
					terext->connect_port_flg[i] = 0x00;
#if 0					
					if(0 != adapter_phone_cnt_talk)
					{
						adapter_phone_cnt_talk--;
					}				
#endif
					break;					
				default:
					break;
			}
		}
	}
	else
	{
		switch(terminal_group[wTemPort].bPortType)
		{
			case PORT_TYPE_QINWU:
				terminal_group[wTemPort].connect_port_flg[terext ->wPort] = 0x00;
				terminal_group[wTemPort].connect_port_cnt--;	

				kuozhan_conf[terext->conf_id].mem_save[terext->wPort] = 0x00;
				kuozhan_conf[terext->conf_id].mem_get[terext->wPort] = 0x00;
#if 0				
				if(PORT_TYPE_ADP_PHONE == terext->bPortType)
				{
					if(0 != adapter_phone_cnt_talk)
					{
						adapter_phone_cnt_talk--;
					}
				}
#endif
				break;
			case PORT_TYPE_TRK:
				terminal_group[wTemPort].connect_port_flg[terext ->wPort] = 0x00;
				terminal_group[wTemPort].connect_port_cnt--;	

				kuozhan_conf[terext->conf_id].mem_save[terext->wPort] = 0x00;
				kuozhan_conf[terext->conf_id].mem_get[terext->wPort] = 0x00;
#if 0				
				if(PORT_TYPE_ADP_PHONE == terext->bPortType)
				{
					if(0 != adapter_phone_cnt_talk)
					{
						adapter_phone_cnt_talk--;
					}
				}
#endif				
				break;				
			case PORT_TYPE_KUOZHAN_PHONE:
				terminal_group[wTemPort].connect_port_flg[terext ->wPort] = 0x00;
				terminal_group[wTemPort].connect_port_cnt--;	

				kuozhan_conf[terext->conf_id].mem_save[terext->wPort] = 0x00;
				kuozhan_conf[terext->conf_id].mem_get[terext->wPort] = 0x00;				
#if 0				
				if(PORT_TYPE_ADP_PHONE == terext->bPortType)
				{
					if(0 != adapter_phone_cnt_talk)
					{
						adapter_phone_cnt_talk--;
					}
				}
#endif				
				break;
			case PORT_TYPE_KUOZHAN_XIWEI:
				terminal_group[wTemPort].connect_port_flg[terext ->wPort] = 0x00;
				terminal_group[wTemPort].connect_port_cnt--;	

				kuozhan_conf[terext->conf_id].mem_save[terext->wPort] = 0x00;
				kuozhan_conf[terext->conf_id].mem_get[terext->wPort] = 0x00;				
#if 0				
				if(PORT_TYPE_ADP_PHONE == terext->bPortType)
				{
					if(0 != adapter_phone_cnt_talk)
					{
						adapter_phone_cnt_talk--;
					}
				}
#endif			
				break;
			case PORT_TYPE_ADP_PHONE:		
				terminal_group[wTemPort].connect_port_flg[terext ->wPort] = 0x00;
				terminal_group[wTemPort].connect_port_cnt--;					

				kuozhan_conf[terext->conf_id].mem_save[terext->wPort] = 0x00;
				kuozhan_conf[terext->conf_id].mem_get[terext->wPort] = 0x00;
#if 0				
				if(PORT_TYPE_ADP_PHONE == terext->bPortType)
				{
					if(0 != adapter_phone_cnt_talk)
					{
						adapter_phone_cnt_talk--;
					}
				}
#endif				
				break;					
			default:
				break;
		}

		if(0x00 == terminal_group[wTemPort].connect_port_cnt)
		{
			terminal_group[wTemPort].bCommand = MC_HANG;
			kuozhan_conf[terext->conf_id].talk_flg = 0;
			kuozhan_conf[terext->conf_id].mem_save[wTemPort] = 0x00;
			kuozhan_conf[terext->conf_id].mem_get[wTemPort] = 0x00;			
		}
	}

	return DRV_OK;
}

static int32 kuozhan_conf_release_cnf_request(TERMINAL_BASE* terext)
{
	int wTemPort = 0;

	wTemPort =  terext ->connect_port;
	if(IDLE == wTemPort)
	{
		printf("%s error port = %d\r\n", __func__, wTemPort);
		return DRV_ERR;
	}

	switch(terminal_group[wTemPort].bPortType)
	{
		case PORT_TYPE_QINWU:
			break;	
		case PORT_TYPE_TRK:
			break;			
		case PORT_TYPE_KUOZHAN_PHONE:
			break;
		case PORT_TYPE_KUOZHAN_XIWEI:
			break;
		case PORT_TYPE_ADP_PHONE:
			break;			
		default:
			break;
	}
	
	return DRV_OK;
}


static int32 kuozhan_conf_transphone_request(TERMINAL_BASE* terext)
{

	return DRV_OK;
}

static int32 kuozhan_conf_ptt_request(TERMINAL_BASE * terext)
{
	int32 wTemPort = 0;
	int32 i = 0;
	int32 flg = 0;

	wTemPort =  terext ->connect_port;
	if(IDLE == wTemPort)
	{
		printf("%s error port = %d\r\n", __func__, wTemPort);
		return DRV_ERR;
	}

	if(0xFF == wTemPort)
	{
		wTemPort =  terext ->wPort;
	}
	else
	{
		wTemPort =  terext ->connect_port;
	}


	for(i = 0; i < KUOZHAN_CONF_MEM_MAX; i++)
	{
		flg = terminal_group[wTemPort].connect_port_flg[i];
		if(0 == flg)
		{
			continue;
		}

		if(MC_TALK != terminal_group[i].bCommand)
		{
			continue;
		}
		
		switch(terminal_group[i].bPortType)
		{
			case PORT_TYPE_QINWU:
				break;
			case PORT_TYPE_KUOZHAN_PHONE:
				break;
			case PORT_TYPE_KUOZHAN_XIWEI:
				if(MGW_PTT_UP == terminal_group[i].ptt_status)
				{
					//xiwei_ptt_request(&terminal_group[i]);
				}
			case PORT_TYPE_ADP_PHONE:
				if(MGW_PTT_UP == terminal_group[i].ptt_status)
				{
					//adp_phone_ptt_request(&terminal_group[i]);
				}
				break;
			default:
				break;
		}
	}

	return DRV_OK;
}


static int32 kuozhan_call_request(TERMINAL_BASE* terext)
{
	switch(terext->bCallType)
	{
		case 0x01:
			kuozhan_single_call_request(terext);
			break;
		case 0x02:
			kuozhan_conf_call_request(terext);
			break;
		default:
			ERR("%s: calltype error %d\r\n", __func__, terext->bCallType);
			break;
	}
	
	return DRV_OK;
}


static int32 kuozhan_callack_request(TERMINAL_BASE* terext)
{
	switch(terext->bCallType)
	{
		case EN_SEL_CALL:
			kuozhan_single_callack_request(terext);
			break;
		case EN_PLAN_CONF:
			kuozhan_conf_callack_request(terext);
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 kuozhan_connect_request(TERMINAL_BASE* terext)
{
	switch(terext->bCallType)
	{
		case EN_SEL_CALL:
			kuozhan_single_connect_request(terext);
			break;
		case EN_PLAN_CONF:
			kuozhan_conf_connect_request(terext);
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 kuozhan_release_request(TERMINAL_BASE* terext)
{
	switch(terext->bCallType)
	{
		case EN_SEL_CALL:
			kuozhan_single_release_request(terext);
			break;
		case EN_PLAN_CONF:
			kuozhan_conf_release_request(terext);
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 kuozhan_release_cnf_request(TERMINAL_BASE* terext)
{
	switch(terext->bCallType)
	{
		case EN_SEL_CALL:
			kuozhan_single_release_cnf_request(terext);
			break;
		case EN_PLAN_CONF:
			kuozhan_conf_release_cnf_request(terext);
			break;
		default:
			break;
	}
	
	return DRV_OK;
}


static int32 kuozhan_transphone_request(TERMINAL_BASE* terext)
{
	switch(terext->bCallType)
	{
		case EN_SEL_CALL:
			kuozhan_single_transphone_request(terext);
			break;
		case EN_PLAN_CONF:
			kuozhan_conf_transphone_request(terext);
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 kuozhan_ptt_request(TERMINAL_BASE * terext)
{
	switch(terext->bCallType)
	{
		case EN_SEL_CALL:
			kuozhan_single_ptt_request(terext);
			break;
		case EN_PLAN_CONF:
			kuozhan_conf_ptt_request(terext);
			break;
		default:
			break;
	}
	
	return DRV_OK;
}


static int32 KuoZhan_Seat_INIT_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 1:
		case 0:
			xiweiext->bPortState = MGW_SW_PHONE_IDLE_STATE;
			xiweiext->bPortState1 = 0;
			xiweiext->wConnectPort = IDLEW;
			xiweiext->wPortDelay = IDLEW;
			xiweiext->bCommand = MC_IDLE;
			xiweiext->bHookStatus = HOOK_ON;	/*note: different from trk port*/
			xiweiext->phone_queue_g.bFinish = 0;
			xiweiext->phone_queue_g.bPhoneQueueHead = 0;
			memset(xiweiext->phone_queue_g.bPhoneQueue,0,MAXPHONENUM);
			xiweiext->phone_queue_s.bFinish = 0;
			xiweiext->phone_queue_s.bPhoneQueueHead = 0;			
			memset(xiweiext->phone_queue_s.bPhoneQueue,0,MAXPHONENUM);			
			xiweiext->bPTTStatus = MGW_PTT_INIT;
			xiweiext->bresv = IDLE;				/*go to init state in radio direct call*/
			xiweiext->rel_reason = MGW_RELEASE_REASON_NORMAL;
			xiweiext->busy_flg = 0;
			xiweiext->connect_port = IDLE;
			xiweiext->ptt_status = MGW_PTT_UP;
			xiweiext->ptt_status_save = MGW_PTT_UP;
			xiweiext->phone_num_trans_flg = 0;
			xiweiext->conf_id = IDLE;
			xiweiext->connect_port_cnt = IDLE;
			memset(xiweiext->connect_port_array, IDLE, KUOZHAN_CONF_MEM_MAX);
			memset(xiweiext->connect_port_flg, 0x00, KUOZHAN_CONF_MEM_MAX);			

#if 0
			xiweiext->bCallType = EN_PLAN_CONF;
			xiweiext->bCommand = MC_TALK;
			xiweiext->conf_id = 0;
#endif			
			sw2ac491(xiweiext->wPort);
			printf("user port %d init\r\n", xiweiext->wPort);
			break;
		default:
			break;
	}

	return DRV_OK;
}

static int32 KuoZhan_Seat_IDLE_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			if(xiweiext->bHookStatus == HOOK_OFF)
			{
				xiweiext->busy_flg = 1;
				xiweiext->bPortState = MGW_SW_PHONE_DIAL_STATE;
				xiweiext->bPortState1 = 0;				
			}

			if(xiweiext->bCommand == MC_RING)
			{
				xiweiext->bPortState = MGW_SW_PHONE_RING_STATE;
				xiweiext->bPortState1 = 0;				
			}

			if(MC_HANG_ACTIVE == xiweiext->bCommand)
			{
				kuozhan_release_request(xiweiext);
				seat_ext_release_cnf_request(xiweiext);	
				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;				
			}
			break;
		default:
			xiweiext->bPortState1 = 0;
			break;
	}
	
	return DRV_OK;
}
static int32 KuoZhan_Seat_DIAL_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			xiweiext->bPortState1 = 1;
			break;
		case 1:
			if(xiweiext->phone_queue_g.bFinish)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d send to scu call request!\r\n", \
					xiweiext->wPort - KUOZHAN_QINWU_OFFSET);
				kuozhan_call_request(xiweiext);
				xiweiext->wPortDelay = 150;
			}
			xiweiext->bPortState1 = 2;
			break;
		case 2:
			if(xiweiext->bCommand == MC_RT_TONE)
			{
				seat_ext_callack_request(xiweiext);
				
				VERBOSE_OUT(LOG_SYS,"send swphone%d callack!\r\n", \
					xiweiext->wPort - KUOZHAN_QINWU_OFFSET);
				xiweiext->bPortState = MGW_SW_PHONE_RINGTONE_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(xiweiext->bCommand == MC_REJECT)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d recv call reject ack !\n", \
					xiweiext->wPort - KUOZHAN_QINWU_OFFSET);
				seat_ext_callack_request(xiweiext);
				kuozhan_release_request(xiweiext);
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
			}

			if(xiweiext->bCommand == MC_NONUM)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d recv call no num ack !\n", \
					xiweiext->wPort - KUOZHAN_QINWU_OFFSET);
				seat_ext_callack_request(xiweiext);
				kuozhan_release_request(xiweiext);			
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
			}

			if(xiweiext->bCommand == MC_NUMERROR)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d recv call no num ack !\n", \
					xiweiext->wPort - KUOZHAN_QINWU_OFFSET);
				seat_ext_callack_request(xiweiext);
				kuozhan_release_request(xiweiext);				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
			}

			if(xiweiext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d Recv Hangup Msg\n",\
					xiweiext->wPort - KUOZHAN_QINWU_OFFSET);

				seat_ext_release_request(xiweiext);
				kuozhan_release_cnf_request(xiweiext);
				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}			

			if(xiweiext->bHookStatus == HOOK_ON || xiweiext->wPortDelay == 0)
			{
				VERBOSE_OUT(LOG_SYS,"swphone %d hangup! OR Time Out\n",\
					xiweiext->wPort - KUOZHAN_QINWU_OFFSET);
				if(xiweiext->wPortDelay==0)
				{
					xiweiext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				}
				
				kuozhan_release_request(xiweiext);
				seat_ext_release_cnf_request(xiweiext);
				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
			}
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Seat_RINGTONE_Process(TERMINAL_BASE* xiweiext)
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
				seat_ext_connect_request(xiweiext);
				VERBOSE_OUT(LOG_SYS,"swphone%d send to swphone connnect!\r\n", \
					xiweiext->wPort - KUOZHAN_QINWU_OFFSET);
				xiweiext->bPortState = MGW_SW_PHONE_TALK_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(xiweiext->bHookStatus == HOOK_ON)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d Send Hangup msg \n",\
					xiweiext->wPort - KUOZHAN_QINWU_OFFSET);
				kuozhan_release_request(xiweiext);
				seat_ext_release_cnf_request(xiweiext);
				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;				
			}

			if(xiweiext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d Recv Hangup Msg!\r\n",\
					xiweiext->wPort - KUOZHAN_QINWU_OFFSET);

				kuozhan_release_cnf_request(xiweiext);
				seat_ext_release_request(xiweiext);
				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}
			
			if(xiweiext->wPortDelay == 0)
			{
				xiweiext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				kuozhan_release_request(xiweiext);
				seat_ext_release_request(xiweiext);				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Seat_RING_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			xiweiext->wPortDelay = 600;
			xiweiext->bPortState1 = 1;
			seat_ext_call_request(xiweiext);
			break;
		case 1:
			if(MC_RINGING == xiweiext->bCommand)
			{
				kuozhan_callack_request(xiweiext);
				xiweiext->bPortState1 = 2;
				xiweiext->wPortDelay = 600;
			}

			if(xiweiext->bCommand == MC_REJECT)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d recv call reject ack !\n", \
					xiweiext->wPort - KUOZHAN_QINWU_OFFSET);
				kuozhan_release_request(xiweiext);
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
			}			
			
			if(xiweiext->bCommand == MC_HANG || xiweiext->wPortDelay == 0)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d Recv Hangup Msg! OR Time Out\n",\
					xiweiext->wPort - KUOZHAN_QINWU_OFFSET);
				kuozhan_release_cnf_request(xiweiext);
				seat_ext_release_request(xiweiext);			
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(MC_HANG_ACTIVE == xiweiext->bCommand)
			{
				kuozhan_release_request(xiweiext);
				seat_ext_release_cnf_request(xiweiext);	
				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;				
			}
			break;
		case 2:
			if(xiweiext->bHookStatus == HOOK_OFF)
			{
				kuozhan_connect_request(xiweiext);
				xiweiext->bPortState = MGW_SW_PHONE_TALK_STATE;
				xiweiext->bPortState1 = 0;
				xiweiext->bCommand = MC_TALK;
			}

			if(xiweiext->bCommand == MC_HANG || xiweiext->wPortDelay == 0)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d Recv Hangup Msg! OR Time Out\n",\
					xiweiext->wPort - KUOZHAN_QINWU_OFFSET);
				kuozhan_release_cnf_request(xiweiext);
				seat_ext_release_request(xiweiext);			
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(MC_HANG_ACTIVE == xiweiext->bCommand)
			{
				kuozhan_release_request(xiweiext);
				seat_ext_release_cnf_request(xiweiext);	
				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;				
			}
			break;			
		default:
			break;			
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Seat_TALK_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			xiweiext->wPortDelay = 0;
			
			if(xiweiext->bHookStatus == HOOK_ON)
			{
				kuozhan_release_request(xiweiext);
				seat_ext_release_cnf_request(xiweiext);
				xiweiext->bPortState1 = 1;
			}

			if(xiweiext->bCommand == MC_TRANS)
			{
				//mgw_scu_calltrans_request(xiweiext);
				//mgw_sw_phone_release_request(xiweiext);
				//kuozhan_release_request(xiweiext);
				//xiweiext->bPortState1 = 1;
			}

			if(xiweiext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"xiwei rcv Handup in talk\r\n");
				kuozhan_release_cnf_request(xiweiext);
				seat_ext_release_request(xiweiext);
				xiweiext->bPortState1 = 1;
			}

			if(xiweiext->bPTTStatus == MGW_PTT_DOWN || xiweiext->bPTTStatus == MGW_PTT_UP)
			{
				VERBOSE_OUT(LOG_SYS,"Send PTT %s to SCU\n", \
					(xiweiext->bPTTStatus == MGW_PTT_DOWN)?"Down":"Up");

				xiweiext->ptt_status = xiweiext->bPTTStatus;
				kuozhan_ptt_request(xiweiext);
				xiweiext->bPTTStatus = MGW_PTT_HOLD;
			}
			break;
		case 1:
			if(xiweiext->wPortDelay == 0)
			{
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Trk_INIT_Process(TERMINAL_BASE* trkext)
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
			trkext->phone_queue_g.bFinish = 0;
			trkext->phone_queue_g.bPhoneQueueHead = 0;
			memset(trkext->phone_queue_g.bPhoneQueue,0,MAXPHONENUM);
			trkext->phone_queue_s.bFinish = 0;
			trkext->phone_queue_s.bPhoneQueueHead = 0;			
			memset(trkext->phone_queue_s.bPhoneQueue,0,MAXPHONENUM);			
			trkext->bPTTStatus = MGW_PTT_INIT;
			trkext->bresv = IDLE;				/*go to init state in radio direct call*/
			trkext->rel_reason = MGW_RELEASE_REASON_NORMAL;
			trkext->busy_flg = 0;
			trkext->connect_port = IDLE;
			trkext->ptt_status = MGW_PTT_UP;
			trkext->ptt_status_save = MGW_PTT_UP;
			trkext->phone_num_trans_flg = 0;
			trkext->conf_id = IDLE;
			trkext->connect_port_cnt = IDLE;
			memset(trkext->connect_port_array, IDLE, KUOZHAN_CONF_MEM_MAX);
			memset(trkext->connect_port_flg, 0x00, KUOZHAN_CONF_MEM_MAX);			
			printf("user port %d init\r\n", trkext->wPort);
			sw2ac491(trkext->wPort);
			break;
		default:	
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Trk_IDLE_Process(TERMINAL_BASE* trkext)
{
	switch(trkext->bPortState1)
	{
		case 0:
			if(trkext->bHookStatus == HOOK_OFF)
			{
				trkext->busy_flg = 1;
				trkext->bPortState = MGW_TRK_IN_DIAL_STATE;
				VERBOSE_OUT(LOG_SYS,"trk %d is hook_off\n",\
					trkext->wPort - KUOZHAN_TRUNK_OFFSET);
				trkext->bPortState1 = 0;
			}
			
			if(trkext->bCommand == MC_USETRK)
			{
				trkext->bPortState = MGW_TRK_OUT_DIAL_STATE;
				trkext->bPortState1 = 0;
				VERBOSE_OUT(LOG_SYS,"USE TRK...\n");
			}

			if(trkext->bCommand == MC_INACTIVE)
			{
				trkext->bPortState1 = 2;
			}
			break;
		case 2:
			if(trkext->bCommand == MC_ACTIVE)
			{
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 0;
			}
			break;
		default:
			trkext->bPortState1 = 0;
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Trk_IN_DIAL_Process(TERMINAL_BASE* trkext)
{
	int tmp_port = 0;
	switch(trkext->bPortState1)
	{
		case 0:
			use_trk(trkext);
			trkext->bCommand = MC_DIAL;
			trkext->bToneType = TONE_DIAL;
			Send_TONE(trkext->wPort,trkext->bToneType);
			trkext->wPortDelay = 150;
			trkext->bPortState1 = 1;
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
				kuozhan_release_request(trkext);
			}

			if(trkext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"trk %d recv hangup!\n", \
					trkext->wPort - KUOZHAN_TRUNK_OFFSET);
				trkext->bToneType = TONE_STOP;
				Send_TONE(trkext->wPort,trkext->bToneType);
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 0;	
			}			

			if(trkext->wPortDelay < 100 && !trkext->phone_queue_g.bPhoneQueueHead)
			{
			#if 0
				if(0 != trkext->fenji)
				{
					memcpy(trkext->phone_queue_g.bPhoneQueue, \
						terminal_group[trkext->fenji].name, terminal_group[trkext->fenji].user_num_len);
					trkext->phone_queue_g.bPhoneQueueHead = terminal_group[trkext->fenji].user_num_len;
					trkext->phone_queue_g.bFinish = 1;
					VERBOSE_OUT(LOG_SYS,"TRK %d Call fenji %s\n",\
						trkext->wPort - KUOZHAN_TRUNK_OFFSET, terminal_group[trkext->fenji].name);
				}
			#else
				if(0 != trkext->zhenru_slot)
				{
					tmp_port = find_port_by_simple_slot(trkext->zhenru_slot);
            		if(tmp_port < 0)
            		{
            			ERR("%s find port %d error\r\n", __func__, trkext->zhenru_slot);
            			return DRV_ERR;
            		}
					memcpy(trkext->phone_queue_g.bPhoneQueue, \
						terminal_group[tmp_port].name, terminal_group[tmp_port].user_num_len);
					trkext->phone_queue_g.bPhoneQueueHead = terminal_group[tmp_port].user_num_len;
					trkext->phone_queue_g.bFinish = 1;
					VERBOSE_OUT(LOG_SYS,"TRK %d Call fenji %s\n",\
						trkext->wPort - KUOZHAN_TRUNK_OFFSET, terminal_group[tmp_port].name);
				}			
			#endif
			}

			if(trkext->phone_queue_g.bPhoneQueueHead)
			{
				trkext->bToneType = TONE_STOP;
				Send_TONE(trkext->wPort,trkext->bToneType);
			}

			if(trkext->phone_queue_g.bFinish == 1)
			{
				trkext->bCallType = EN_SEL_CALL;
				kuozhan_call_request(trkext);
				VERBOSE_OUT(LOG_SYS,"TRK %d Send Call %s\n",\
					trkext->wPort - KUOZHAN_TRUNK_OFFSET, trkext->phone_queue_g.bPhoneQueue);
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
				kuozhan_release_request(trkext);
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 0;
			}

			if(trkext->bCommand == MC_RT_TONE)
			{
				trkext->bPortState = MGW_TRK_IN_RINGTONG_STATE;
				trkext->bPortState1 = 0;
				VERBOSE_OUT(LOG_SYS,"TRK in %d goto ringtone \r\n",\
					trkext->wPort - KUOZHAN_TRUNK_OFFSET);				
			}
			break;
		case 3:
			break;
		default:
			break;
	}
	
	return DRV_OK;
}


static int32 KuoZhan_Trk_IN_RINGTONG_Process(TERMINAL_BASE* trkext)
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
				trkext->bPortState1 = 0;
				trkext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				kuozhan_release_request(trkext);
			}
			
			if(trkext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"trk %d recv hangup!\n", trkext->wPort - KUOZHAN_TRUNK_OFFSET);
				trkext->bToneType = TONE_STOP;
				Send_TONE(trkext->wPort,trkext->bToneType);
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 0;	
				trkext->rel_reason = MGW_RELEASE_REASON_NORMAL;
				kuozhan_release_request(trkext);				
			}
			
			if(trkext->bCommand == MC_TALK)
			{
				VERBOSE_OUT(LOG_SYS,"TRK in %d goto talk \r\n",\
					trkext->wPort - KUOZHAN_TRUNK_OFFSET);				
				trkext->bToneType = TONE_STOP;
				Send_TONE(trkext->wPort,trkext->bToneType);
				trkext->bPortState = MGW_TRK_IN_TALK_STATE;
				trkext->bPortState1 = 0;
			}
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Trk_IN_TALK_Process(TERMINAL_BASE* trkext)
{
	switch(trkext->bPortState1)
	{
		case 0:
			if(trkext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"phone %d recv hangup!\n", trkext->wPort - KUOZHAN_TRUNK_OFFSET);
				trkext->wPortDelay = 10;
				trkext->bPortState1 = 1;
			}

			if(trkext->phone_queue_s.bFinish)
			{
				trkext->bPortState1 = 2;
				trkext->wPortDelay = 10;
			}			
			break;
		case 1:
			if(trkext->wPortDelay == 0)
			{
				trkext->rel_reason = MGW_RELEASE_REASON_NORMAL;
				kuozhan_release_request(trkext);
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 0;
			}
			break;
		case 2:
			if(trkext->wPortDelay == 0)
			{
				inc_sched_add(con,150,send_dtmf_string,trkext);
				VERBOSE_OUT(LOG_SYS,"Send dtmf string: %s, len %d\n",\
					trkext->phone_queue_s.bPhoneQueue, trkext->phone_queue_s.bPhoneQueueHead);
				trkext->phone_queue_s.bFinish = 0;
				trkext->bPortState1 = 0;
			}

			if(trkext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"trk %d is hangup!\n", trkext->wPort - KUOZHAN_TRUNK_OFFSET);
				trkext->wPortDelay = 10;
				trkext->bPortState1 = 1;
			}
			break;		
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Trk_OUT_DIAL_Process(TERMINAL_BASE* trkext)
{
	switch(trkext->bPortState1)
	{
		case 0:
			trkext->wPortDelay = 20;
			trkext->bPortState1 = 1;
			trkext->bCommand = MC_RING;
			kuozhan_callack_request(trkext);
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
				kuozhan_release_cnf_request(trkext);
			}
			break;
		case 2:
			if(trkext->wPortDelay == 0)
			{
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 0;
				trkext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				kuozhan_release_request(trkext);
			}

			kuozhan_connect_request(trkext);
			use_trk(trkext);

			trkext->bPortState = MGW_TRK_OUT_TALK_STATE;
			trkext->bPortState1 = 0;
			trkext->bCommand = MC_TALK;
			VERBOSE_OUT(LOG_SYS,"trk goto talk...\n");
			break;
		default:
			break;
	}
	
	return DRV_OK;
}


static int32 KuoZhan_Trk_OUT_RINGTONE_Process(TERMINAL_BASE* trkext)
{
	switch(trkext->bPortState1)
	{
		case 0:
			terminal_group[trkext->wConnectPort].bCommand = MC_RT_TONE;
			trkext->wPortDelay = 50;
			trkext->bPortState1 = 1;
			kuozhan_connect_request(trkext);
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
	
	return DRV_OK;
}

static int32 KuoZhan_Trk_OUT_TALK_Process(TERMINAL_BASE* trkext)
{
	switch(trkext->bPortState1)
	{
		case 0:
			if(trkext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"trk %d is hangup!\n", trkext->wPort - KUOZHAN_TRUNK_OFFSET);
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
				kuozhan_release_request(trkext);
				trkext->bPortState = MGW_TRK_INIT_STATE;
				trkext->bPortState1 = 0;
			}
			break;
		case 2:
			if(trkext->wPortDelay == 0)
			{
				inc_sched_add(con,150,send_dtmf_string,trkext);
				VERBOSE_OUT(LOG_SYS,"Send dtmf string: %s, len %d\n",\
					trkext->phone_queue_s.bPhoneQueue, trkext->phone_queue_s.bPhoneQueueHead);
				trkext->phone_queue_s.bFinish = 0;
				trkext->bPortState1 = 0;
			}

			if(trkext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"trk %d is hangup!\n", trkext->wPort - KUOZHAN_TRUNK_OFFSET);
				trkext->wPortDelay = 10;
				trkext->bPortState1 = 1;
			}
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Phone_INIT_Process(TERMINAL_BASE* phoneext)
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
			phoneext->phone_queue_s.bFinish = 0;
			phoneext->phone_queue_s.bPhoneQueueHead = 0;			
			memset(phoneext->phone_queue_s.bPhoneQueue,0,MAXPHONENUM);
			phoneext->bPTTStatus = MGW_PTT_INIT;
			phoneext->rel_reason = MGW_RELEASE_REASON_NORMAL;
			phoneext->busy_flg = 0;
			phoneext->connect_port = IDLE;
			phoneext->ptt_status = MGW_PTT_UP;
			phoneext->ptt_status_save = MGW_PTT_UP;
			phoneext->phone_num_trans_flg = 0;
			phoneext->conf_id = IDLE;
			phoneext->connect_port_cnt = IDLE;
			memset(phoneext->connect_port_array, IDLE, KUOZHAN_CONF_MEM_MAX);
			memset(phoneext->connect_port_flg, 0x00, KUOZHAN_CONF_MEM_MAX);					
			sw2ac491(phoneext->wPort);
			Gpio_Ring_Set(phoneext->wPort, 0);
			printf("user port %d init\r\n", phoneext->wPort);
			break;
		default:
			break;		
	}
	
	return DRV_OK;
}


static int32 KuoZhan_Phone_IDLE_Process(TERMINAL_BASE* phoneext)
{
	switch(phoneext->bPortState1)
	{
		case 0:
			if(phoneext->bHookStatus == HOOK_OFF)
			{
				phoneext->busy_flg = 1;
				phoneext->bPortState = MGW_PHONE_DIAL_STATE;
				VERBOSE_OUT(LOG_SYS,"phone %d is hook_off\n", phoneext->wPort - KUOZHAN_PHONE_OFFSET);
				phoneext->bPortState1 = 0;
			}
			
			if(phoneext->bCommand == MC_RING)
			{
				phoneext->bPortState = MGW_PHONE_RING_STATE;
				phoneext->bPortState1 = 0;
			}

			break;
		default:
			phoneext->bPortState1 = 0;
			break;
	}
	
	return DRV_OK;
}


static int32 KuoZhan_Phone_DIAL_Process(TERMINAL_BASE* phoneext)
{
	int dial_flg = 1;
	switch(phoneext->bPortState1)
	{
		case 0:
			phoneext->bToneType = TONE_DIAL;
			Send_TONE(phoneext->wPort,phoneext->bToneType);
			VERBOSE_OUT(LOG_SYS,"Send phone %d Tone Dial\n",phoneext->wPort - KUOZHAN_PHONE_OFFSET);
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
				VERBOSE_OUT(LOG_SYS,"phone %d Dial Time is Up\n", phoneext->wPort - KUOZHAN_PHONE_OFFSET);
			}

			if((phoneext->wPortDelay == 0)&&(phoneext->bToneType == TONE_BUSY))
			{
				phoneext->bToneType = TONE_STOP;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				VERBOSE_OUT(LOG_SYS,"phone %d busy Time is Up\n", phoneext->wPort - KUOZHAN_PHONE_OFFSET);
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
					VERBOSE_OUT(LOG_SYS,"phone %d Send Call %s\n", \
						phoneext->wPort  - KUOZHAN_PHONE_OFFSET, phoneext->phone_queue_g.bPhoneQueue);
					//mgw_scu_call_request(phoneext);
					kuozhan_call_request(phoneext);
					phoneext->wPortDelay = 150;
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
					kuozhan_callack_request(phoneext);
				}
			}
			
			if(phoneext->bHookStatus == HOOK_ON)
			{
				phoneext->bToneType = TONE_STOP;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				
				VERBOSE_OUT(LOG_SYS,"phone %d hangup!\n",phoneext->wPort - KUOZHAN_PHONE_OFFSET);

				kuozhan_release_request(phoneext);
				
				phoneext->bPortState = MGW_PHONE_INIT_STATE;
				phoneext->bPortState1 = 0;
			}
			break;
		case 2:
			if(phoneext->wPortDelay == 0)
			{
				phoneext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				kuozhan_release_request(phoneext);
				
				phoneext->wPortDelay = 300;
				phoneext->bToneType = TONE_BUSY;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				phoneext->bPortState1 = 3;					
			}

			if(phoneext->bCommand == MC_RT_TONE)
			{
				phoneext->bPortState = MGW_PHONE_RINGTONE_STATE;
				phoneext->bPortState1 = 0;

				VERBOSE_OUT(LOG_SYS,"phone%d goto ringtone!\n",phoneext->wPort - KUOZHAN_PHONE_OFFSET);
			}

			if(phoneext->bCommand == MC_REJECT)
			{
				phoneext->wPortDelay = 300;
				phoneext->bToneType = TONE_BUSY;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				phoneext->bPortState1 = 3;
			}

			if(phoneext->bCommand == MC_NONUM)
			{			
				phoneext->wPortDelay = 300;
				phoneext->bToneType = TONE_BUSY;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				phoneext->bPortState1 = 3;
				VERBOSE_OUT(LOG_SYS,"phone%d recv call no num ack !\n", \
					phoneext->wPort - KUOZHAN_PHONE_OFFSET);				
			}	

			if(phoneext->bCommand == MC_NUMERROR)
			{
				phoneext->wPortDelay = 300;
				phoneext->bToneType = TONE_BUSY;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				phoneext->bPortState1 = 3;
				VERBOSE_OUT(LOG_SYS,"phone%d recv call num error ack !\n", \
					phoneext->wPort - KUOZHAN_PHONE_OFFSET);				
			}			

			if(phoneext->bHookStatus == HOOK_ON)
			{	
				phoneext->bToneType = TONE_STOP;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				kuozhan_release_request(phoneext);
				phoneext->bPortState = MGW_PHONE_INIT_STATE;
				phoneext->bPortState1 = 0;

				VERBOSE_OUT(LOG_SYS,"phone %d hangup in dial!\n",\
					phoneext->wPort - KUOZHAN_PHONE_OFFSET);				
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
				
				VERBOSE_OUT(LOG_SYS,"phone %d hangup!\n",phoneext->wPort - KUOZHAN_PHONE_OFFSET);
				kuozhan_release_request(phoneext);
				
				phoneext->bPortState = MGW_PHONE_INIT_STATE;
				phoneext->bPortState1 = 0;
			}					
			break;
		default:
			phoneext->bPortState1 = 0;
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Phone_RINGTONE_Process(TERMINAL_BASE* phoneext)
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
				phoneext->wPortDelay = 300;
				phoneext->bToneType = TONE_BUSY;
				Send_TONE(phoneext->wPort, phoneext->bToneType);
				phoneext->bPortState1 = 2;
			}

			if(phoneext->bHookStatus == HOOK_ON)
			{	
				VERBOSE_OUT(LOG_SYS,"phone %d hangup in ringtone!\n",phoneext->wPort - KUOZHAN_PHONE_OFFSET);
				kuozhan_release_request(phoneext);
				phoneext->bPortState = MGW_PHONE_INIT_STATE;
				phoneext->bPortState1 = 0;
			}
				
			if(phoneext->bCommand == MC_TALK)
			{
				phoneext->bToneType = TONE_STOP;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				phoneext->bPortState = MGW_PHONE_TALK_STATE;
				phoneext->bPortState1 = 0;
				VERBOSE_OUT(LOG_SYS,"phone%d goto talk!\n",phoneext->wPort - KUOZHAN_PHONE_OFFSET);
			}
			break;
		case 2:
			if(phoneext->wPortDelay == 0)
			{
				phoneext->bToneType = TONE_STOP;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
			}

			if(phoneext->bHookStatus == HOOK_ON)
			{
				phoneext->bToneType = TONE_STOP;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				VERBOSE_OUT(LOG_SYS,"phone %d hangup!\n",phoneext->wPort - KUOZHAN_PHONE_OFFSET);
				kuozhan_release_request(phoneext);
				phoneext->bPortState = MGW_PHONE_INIT_STATE;
				phoneext->bPortState1 = 0;
			}
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Phone_RING_Process(TERMINAL_BASE* phoneext)
{
	switch(phoneext->bPortState1)
	{
		case 0:
			Gpio_Ring_Set(phoneext->wPort, 1);
			phoneext->wPortDelay = 600;
			phoneext->bPortState1 = 1;
			kuozhan_callack_request(phoneext);
			break;
		case 1:
			if(phoneext->bCommand == MC_HANG)
			{
				Gpio_Ring_Set(phoneext->wPort, 0);
				phoneext->bPortState = MGW_PHONE_INIT_STATE;
				phoneext->bPortState1 = 0;
				VERBOSE_OUT(LOG_SYS," zhujiao hangup!\n");
			}

			if(phoneext->bCommand == MC_REJECT)
			{
				VERBOSE_OUT(LOG_SYS,"phone%d  recv call reject ack !\n", \
					phoneext->wPort - KUOZHAN_PHONE_OFFSET);
				kuozhan_release_request(phoneext);
				phoneext->bPortState = MGW_PHONE_INIT_STATE;
				phoneext->bPortState1 = 0;	
			}			
			
			if(phoneext->bHookStatus == HOOK_OFF)
			{
				Gpio_Ring_Set(phoneext->wPort, 0);
				phoneext->bPortState = MGW_PHONE_TALK_STATE;
				phoneext->bPortState1 = 0;
				kuozhan_connect_request(phoneext);
				VERBOSE_OUT(LOG_SYS,"phone%d goto talk in ringing!\n",phoneext->wPort - KUOZHAN_PHONE_OFFSET);
				phoneext->bCommand = MC_TALK;
			}

			if(phoneext->wPortDelay == 0)
			{
				Gpio_Ring_Set(phoneext->wPort, 0);
				kuozhan_release_request(phoneext);
				phoneext->bPortState = MGW_PHONE_INIT_STATE;
				phoneext->bPortState1 = 0;
				VERBOSE_OUT(LOG_SYS,"ring time up!\n");
			}
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Phone_TALK_Process(TERMINAL_BASE* phoneext)
{
	switch(phoneext->bPortState1)
	{
		case 0:
			if(phoneext->bHookStatus == HOOK_ON)
			{
				VERBOSE_OUT(LOG_SYS,"phone %d hangup in talk!\n", phoneext->wPort -KUOZHAN_PHONE_OFFSET);
				kuozhan_release_request(phoneext);
				phoneext->bPortState = MGW_PHONE_INIT_STATE;
				phoneext->bPortState1 = 1;
			}

			if(phoneext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"phone %d recv hangup!\n", phoneext->wPort - KUOZHAN_PHONE_OFFSET);
				phoneext->wPortDelay = 300;
				phoneext->bToneType = TONE_BUSY;
				Send_TONE(phoneext->wPort, phoneext->bToneType);
				phoneext->bPortState1 = 1;
			}

			if(phoneext->bPTTStatus == MGW_PTT_DOWN || phoneext->bPTTStatus == MGW_PTT_UP){
				VERBOSE_OUT(LOG_SYS,"Send PTT %s to SCU\n", \
					(phoneext->bPTTStatus == MGW_PTT_DOWN)?"Down":"Up");

				phoneext->ptt_status = phoneext->bPTTStatus;
				kuozhan_ptt_request(phoneext);
				phoneext->bPTTStatus = MGW_PTT_HOLD;
			}	
			break;
		case 1:
			if(phoneext->wPortDelay == 0)
			{
				phoneext->bToneType = TONE_STOP;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
			}

			if(phoneext->bHookStatus == HOOK_ON)
			{
				phoneext->bToneType = TONE_STOP;
				Send_TONE(phoneext->wPort,phoneext->bToneType);
				VERBOSE_OUT(LOG_SYS,"phone %d hangup!\n",phoneext->wPort - KUOZHAN_PHONE_OFFSET);
				kuozhan_release_request(phoneext);
				phoneext->bPortState = MGW_PHONE_INIT_STATE;
				phoneext->bPortState1 = 0;
			}
			else if((phoneext->bHookStatus == HOOK_OFF))
			{
				if(phoneext->bCommand == MC_RING)
				{
					phoneext->bCommand = MC_REJECT;
					kuozhan_callack_request(phoneext);
				}
			}
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_XiWei_INIT_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 1:
		case 0:
			xiweiext->bPortState = MGW_SW_PHONE_IDLE_STATE;
			xiweiext->bPortState1 = 0;
			xiweiext->wConnectPort = IDLEW;
			xiweiext->wPortDelay = IDLEW;
			xiweiext->bCommand = MC_IDLE;
			xiweiext->bHookStatus = HOOK_ON;	/*note: different from trk port*/
			xiweiext->phone_queue_g.bFinish = 0;
			xiweiext->phone_queue_g.bPhoneQueueHead = 0;
			memset(xiweiext->phone_queue_g.bPhoneQueue,0,MAXPHONENUM);
			xiweiext->phone_queue_s.bFinish = 0;
			xiweiext->phone_queue_s.bPhoneQueueHead = 0;			
			memset(xiweiext->phone_queue_s.bPhoneQueue,0,MAXPHONENUM);			
			xiweiext->bPTTStatus = MGW_PTT_INIT;
			xiweiext->bresv = IDLE;				/*go to init state in radio direct call*/
			xiweiext->rel_reason = MGW_RELEASE_REASON_NORMAL;
			xiweiext->busy_flg = 0;
			xiweiext->connect_port = IDLE;
			xiweiext->ptt_status = MGW_PTT_UP;
			xiweiext->ptt_status_save = MGW_PTT_UP;
			xiweiext->phone_num_trans_flg = 0;
			xiweiext->conf_id = IDLE;
			xiweiext->connect_port_cnt = IDLE;
			memset(xiweiext->connect_port_array, IDLE, KUOZHAN_CONF_MEM_MAX);
			memset(xiweiext->connect_port_flg, 0x00, KUOZHAN_CONF_MEM_MAX);				
			printf("user port %d init\r\n", xiweiext->wPort);
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_XiWei_IDLE_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			if(xiweiext->bHookStatus == HOOK_OFF)
			{
				xiweiext->busy_flg = 1;
				xiweiext->bPortState = MGW_SW_PHONE_DIAL_STATE;
				xiweiext->bPortState1 = 0;
				VERBOSE_OUT(LOG_SYS,"swphone%d idle to dail!\r\n", \
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);				
			}

			if(xiweiext->bCommand == MC_RING)
			{
				xiweiext->bPortState = MGW_SW_PHONE_RING_STATE;
				xiweiext->bPortState1 = 0;
				VERBOSE_OUT(LOG_SYS,"swphone%d idle to ring!\r\n", \
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);				
			}

			if(MC_HANG_ACTIVE == xiweiext->bCommand)
			{
				kuozhan_release_request(xiweiext);
				xiwei_release_cnf_request(xiweiext);	
				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;				
			}	
			break;
		default:
			xiweiext->bPortState1 = 0;
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_XiWei_DIAL_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			xiweiext->bPortState1 = 1;
			break;
		case 1:
			if(xiweiext->phone_queue_g.bFinish)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d send to scu call request!\r\n", \
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);
				kuozhan_call_request(xiweiext);
				xiweiext->wPortDelay = 150;
			}
			xiweiext->bPortState1 = 2;
			break;
		case 2:
			if(xiweiext->bCommand == MC_RT_TONE)
			{
				xiwei_callack_request(xiweiext);
				VERBOSE_OUT(LOG_SYS,"send swphone%d callack!\r\n", \
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);
				xiweiext->bPortState = MGW_SW_PHONE_RINGTONE_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(xiweiext->bCommand == MC_REJECT)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d recv call reject ack !\n", \
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);
				xiwei_callack_request(xiweiext);
				kuozhan_release_request(xiweiext);
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
			}

			if(xiweiext->bCommand == MC_NONUM)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d recv call no num ack !\n", \
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);
				xiwei_callack_request(xiweiext);
				kuozhan_release_request(xiweiext);			
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
			}

			if(xiweiext->bCommand == MC_NUMERROR)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d recv call no num ack !\n", \
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);
				xiwei_callack_request(xiweiext);
				kuozhan_release_request(xiweiext);				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
			}

			if(xiweiext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d Recv Hangup Msg\n",\
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);

				xiwei_release_request(xiweiext);
				kuozhan_release_cnf_request(xiweiext);
				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}			

			if(xiweiext->bHookStatus == HOOK_ON || xiweiext->wPortDelay == 0)
			{
				VERBOSE_OUT(LOG_SYS,"swphone %d hangup! OR Time Out\n",\
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);
				if(xiweiext->wPortDelay==0)
				{
					xiweiext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				}
				
				kuozhan_release_request(xiweiext);
				xiwei_release_cnf_request(xiweiext);
				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
			}
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_XiWei_RINGTONE_Process(TERMINAL_BASE* xiweiext)
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
				xiwei_connect_request(xiweiext);
				VERBOSE_OUT(LOG_SYS,"swphone%d send to swphone connnect!\r\n", \
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);
				xiweiext->bPortState = MGW_SW_PHONE_TALK_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(xiweiext->bHookStatus == HOOK_ON)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d Send Hangup msg \n",\
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);
				kuozhan_release_request(xiweiext);
				xiwei_release_cnf_request(xiweiext);
				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;				
			}

			if(xiweiext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d Recv Hangup Msg!\r\n",\
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);

				kuozhan_release_cnf_request(xiweiext);
				xiwei_release_request(xiweiext);
				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}
			
			if(xiweiext->wPortDelay == 0)
			{
				xiweiext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				kuozhan_release_request(xiweiext);
				xiwei_release_request(xiweiext);				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_XiWei_RING_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			xiweiext->wPortDelay = 600;
			xiweiext->bPortState1 = 1;
			xiwei_call_request(xiweiext);
			break;
		case 1:
			if(MC_RINGING == xiweiext->bCommand)
			{
				kuozhan_callack_request(xiweiext);
				xiweiext->wPortDelay = 600;
				xiweiext->bPortState1 = 2;
				VERBOSE_OUT(LOG_SYS,"swphone%d ring to ringing!\r\n", \
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);				
			}

			if(xiweiext->bCommand == MC_REJECT)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d  recv call reject ack !\n", \
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);
				kuozhan_release_request(xiweiext);
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
			}			
			
			if(xiweiext->bCommand == MC_HANG || xiweiext->wPortDelay == 0)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d Recv Hangup Msg! OR Time Out\n",\
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);
				kuozhan_release_cnf_request(xiweiext);
				xiwei_release_request(xiweiext);			
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(MC_HANG_ACTIVE == xiweiext->bCommand)
			{
				kuozhan_release_request(xiweiext);
				xiwei_release_cnf_request(xiweiext);	
				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;				
			}
			break;
		case 2:
			
			if(xiweiext->bHookStatus == HOOK_OFF)
			{
				kuozhan_connect_request(xiweiext);

				VERBOSE_OUT(LOG_SYS,"swphone%d ringing to talk!\r\n", \
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);
				
				xiweiext->bPortState = MGW_SW_PHONE_TALK_STATE;
				xiweiext->bPortState1 = 0;
				xiweiext->bCommand = MC_TALK;
			}

			if(xiweiext->bCommand == MC_HANG || xiweiext->wPortDelay == 0)
			{
				VERBOSE_OUT(LOG_SYS,"swphone%d Recv Hangup Msg! OR Time Out\n",\
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);
				kuozhan_release_cnf_request(xiweiext);
				xiwei_release_request(xiweiext);			
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(MC_HANG_ACTIVE == xiweiext->bCommand)
			{
				kuozhan_release_request(xiweiext);
				xiwei_release_cnf_request(xiweiext);	
				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;				
			}
			break;			
		default:
			break;			
	}
	
	return DRV_OK;
}

static int32 KuoZhan_XiWei_TALK_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			xiweiext->wPortDelay = 0;
			
			if(xiweiext->bHookStatus == HOOK_ON)
			{
				kuozhan_release_request(xiweiext);
				xiwei_release_cnf_request(xiweiext);
				xiweiext->bPortState1 = 1;
			}

			if(xiweiext->bCommand == MC_TRANS)
			{
				//mgw_scu_calltrans_request(xiweiext);
				//mgw_sw_phone_release_request(xiweiext);
				//kuozhan_release_request(xiweiext);
				//xiweiext->bPortState1 = 1;
			}

			if(1 == xiweiext->phone_queue_s.bFinish)
			{
				kuozhan_transphone_request(xiweiext);
				xiweiext->phone_queue_s.bFinish = 0;
			}

			if(xiweiext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"xiwei rcv Handup in talk\r\n");
				kuozhan_release_cnf_request(xiweiext);
				xiwei_release_request(xiweiext);
				xiweiext->bPortState1 = 1;
			}

			if(xiweiext->bPTTStatus == MGW_PTT_DOWN || xiweiext->bPTTStatus == MGW_PTT_UP)
			{
				VERBOSE_OUT(LOG_SYS,"Send PTT %s to SCU\n", \
					(xiweiext->bPTTStatus == MGW_PTT_DOWN)?"Down":"Up");

				xiweiext->ptt_status = xiweiext->bPTTStatus;
				kuozhan_ptt_request(xiweiext);
				xiweiext->bPTTStatus = MGW_PTT_HOLD;
			}
			break;
		case 1:
			if(xiweiext->wPortDelay == 0)
			{
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Adp_PHONE_INIT_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 1:
		case 0:
			xiweiext->bPortState = MGW_ADP_PHONE_IDLE_STATE;
			xiweiext->bPortState1 = 0;
			xiweiext->wConnectPort = IDLEW;
			xiweiext->wPortDelay = IDLEW;
			xiweiext->bCommand = MC_IDLE;
			xiweiext->bHookStatus = HOOK_ON;	/*note: different from trk port*/
			xiweiext->phone_queue_g.bFinish = 0;
			xiweiext->phone_queue_g.bPhoneQueueHead = 0;
			memset(xiweiext->phone_queue_g.bPhoneQueue,0,MAXPHONENUM);
			xiweiext->phone_queue_s.bFinish = 0;
			xiweiext->phone_queue_s.bPhoneQueueHead = 0;			
			memset(xiweiext->phone_queue_s.bPhoneQueue,0,MAXPHONENUM);
			xiweiext->bPTTStatus = MGW_PTT_INIT;
			xiweiext->bresv = IDLE;				/*go to init state in radio direct call*/
			xiweiext->rel_reason = MGW_RELEASE_REASON_NORMAL;
			xiweiext->busy_flg = 0;
			xiweiext->connect_port = IDLE;
			xiweiext->ptt_status = MGW_PTT_UP;
			xiweiext->ptt_status_save = MGW_PTT_UP;	
			xiweiext->phone_num_trans_flg = 0;
			xiweiext->conf_id = IDLE;
			xiweiext->connect_port_cnt = IDLE;
			memset(xiweiext->connect_port_array, IDLE, KUOZHAN_CONF_MEM_MAX);
			memset(xiweiext->connect_port_flg, 0x00, KUOZHAN_CONF_MEM_MAX);			
			printf("user port %d init\r\n", xiweiext->wPort);
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Adp_PHONE_IDLE_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			if(xiweiext->bHookStatus == HOOK_OFF)
			{
				xiweiext->busy_flg = 1;
				xiweiext->bPortState = MGW_ADP_PHONE_DIAL_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(xiweiext->bCommand == MC_RING)
			{
				xiweiext->bPortState = MGW_ADP_PHONE_RING_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(MC_HANG_ACTIVE == xiweiext->bCommand)
			{
				kuozhan_release_request(xiweiext);
				adp_phone_release_cnf_request(xiweiext);	
				
				xiweiext->bPortState = MGW_SW_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;				
			}	
			break;
		default:
			xiweiext->bPortState1 = 0;
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Adp_PHONE_DIAL_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			xiweiext->bPortState1 = 1;
			break;
		case 1:
			if(xiweiext->phone_queue_g.bFinish)
			{
				VERBOSE_OUT(LOG_SYS,"adpphone%d send to scu call request!\r\n", \
					xiweiext->wPort - KUOZHAN_ADATPER_OFFSET);
				kuozhan_call_request(xiweiext);
				xiweiext->wPortDelay = 150;
			}
			xiweiext->bPortState1 = 2;
			break;
		case 2:
			if(xiweiext->bCommand == MC_RT_TONE)
			{
				adp_phone_callack_request(xiweiext);
				VERBOSE_OUT(LOG_SYS,"adpphone%d send to adpphone callack!\r\n", \
					xiweiext->wPort - KUOZHAN_ADATPER_OFFSET);
				xiweiext->bPortState = MGW_ADP_PHONE_RINGTONE_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(xiweiext->bCommand == MC_REJECT)
			{
				VERBOSE_OUT(LOG_SYS,"adpphone%d recv call reject ack !\n", \
					xiweiext->wPort - KUOZHAN_ADATPER_OFFSET);
				adp_phone_callack_request(xiweiext);
				kuozhan_release_request(xiweiext);
				xiweiext->bPortState = MGW_ADP_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
			}

			if(xiweiext->bCommand == MC_NONUM)
			{
				VERBOSE_OUT(LOG_SYS,"adpphone%d recv call no num ack !\n", \
					xiweiext->wPort - KUOZHAN_ADATPER_OFFSET);
				adp_phone_callack_request(xiweiext);
				kuozhan_release_request(xiweiext);
				xiweiext->bPortState = MGW_ADP_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
			}

			if(xiweiext->bCommand == MC_NUMERROR)
			{
				VERBOSE_OUT(LOG_SYS,"adpphone%d recv call no num ack !\n", \
					xiweiext->wPort - KUOZHAN_ADATPER_OFFSET);
				adp_phone_callack_request(xiweiext);
				kuozhan_release_request(xiweiext);
				xiweiext->bPortState = MGW_ADP_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
			}

			if(xiweiext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"adpphone%d Recv Hangup Msg\n",\
					xiweiext->wPort - KUOZHAN_ADATPER_OFFSET);
				kuozhan_release_cnf_request(xiweiext);
				adp_phone_release_request(xiweiext);
				xiweiext->bPortState = MGW_ADP_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}			

			if(xiweiext->bHookStatus == HOOK_ON || xiweiext->wPortDelay == 0)
			{
				VERBOSE_OUT(LOG_SYS,"adpphone %d hangup! OR Time Out\n",\
					xiweiext->wPort - KUOZHAN_ADATPER_OFFSET);
				if(xiweiext->wPortDelay==0)
				{
					xiweiext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				}
				
				kuozhan_release_request(xiweiext);
				adp_phone_release_cnf_request(xiweiext);
				xiweiext->bPortState = MGW_ADP_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
			}
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Adp_PHONE_RINGTONE_Process(TERMINAL_BASE* xiweiext)
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
				adp_phone_connect_request(xiweiext);
				VERBOSE_OUT(LOG_SYS,"adpphone%d send to swphone connnect!\r\n", \
					xiweiext->wPort - KUOZHAN_ADATPER_OFFSET);
				xiweiext->bPortState = MGW_ADP_PHONE_TALK_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(xiweiext->bHookStatus == HOOK_ON)
			{
				VERBOSE_OUT(LOG_SYS,"adpphone%d Send Hangup msg \n",\
					xiweiext->wPort - KUOZHAN_ADATPER_OFFSET);
				kuozhan_release_request(xiweiext);
				adp_phone_release_cnf_request(xiweiext);
				xiweiext->bPortState = MGW_ADP_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;				
			}

			if(xiweiext->bCommand == MC_HANG)
			{
				VERBOSE_OUT(LOG_SYS,"adpphone%d Recv Hangup Msg! OR Time Out\n",\
					xiweiext->wPort - KUOZHAN_ADATPER_OFFSET);
				kuozhan_release_cnf_request(xiweiext);
				adp_phone_release_request(xiweiext);
				xiweiext->bPortState = MGW_ADP_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}
			
			if(xiweiext->wPortDelay == 0)
			{
				xiweiext->rel_reason = MGW_RELEASE_REASON_TIMEOUT;
				kuozhan_release_request(xiweiext);
				adp_phone_release_request(xiweiext);
				xiweiext->bPortState = MGW_ADP_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}
			break;
		default:
			break;
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Adp_PHONE_RING_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			xiweiext->wPortDelay = 600;
			xiweiext->bPortState1 = 1;
			adp_phone_call_request(xiweiext);
			break;
		case 1:
			if(MC_RINGING == xiweiext->bCommand)
			{
				kuozhan_callack_request(xiweiext);
				xiweiext->wPortDelay = 600;
				xiweiext->bPortState1 = 2;				
			}

			if(xiweiext->bCommand == MC_REJECT)
			{
				VERBOSE_OUT(LOG_SYS,"xiwei%d  recv call reject ack !\n", \
					xiweiext->wPort - KUOZHAN_PHONE_OFFSET);
				kuozhan_release_request(xiweiext);
				xiweiext->bPortState = MGW_ADP_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;	
			}			

			if(xiweiext->bCommand == MC_HANG || xiweiext->wPortDelay == 0)
			{
				VERBOSE_OUT(LOG_SYS,"xiwei%d Recv Hangup Msg! OR Time Out\n",\
					xiweiext->wPort - KUOZHAN_ADATPER_OFFSET);
				kuozhan_release_cnf_request(xiweiext);
				adp_phone_release_request(xiweiext);
				xiweiext->bPortState = MGW_ADP_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(MC_HANG_ACTIVE == xiweiext->bCommand)
			{
				kuozhan_release_request(xiweiext);
				adp_phone_release_cnf_request(xiweiext);/*释放证实*/
				xiweiext->bPortState = MGW_ADP_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;				
			}
			break;
		case 2:
			if(xiweiext->bHookStatus == HOOK_OFF)
			{
				kuozhan_connect_request(xiweiext);

				VERBOSE_OUT(LOG_SYS,"adpphone%d send to scu connnect!\r\n", \
					xiweiext->wPort - KUOZHAN_ADATPER_OFFSET);
				
				xiweiext->bPortState = MGW_ADP_PHONE_TALK_STATE;
				xiweiext->bPortState1 = 0;
				xiweiext->bCommand = MC_TALK;
			}

			if(xiweiext->bCommand == MC_HANG || xiweiext->wPortDelay == 0)
			{
				VERBOSE_OUT(LOG_SYS,"xiwei%d Recv Hangup Msg! OR Time Out\n",\
					xiweiext->wPort - KUOZHAN_ADATPER_OFFSET);
				kuozhan_release_cnf_request(xiweiext);
				adp_phone_release_request(xiweiext);
				xiweiext->bPortState = MGW_ADP_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;
			}

			if(MC_HANG_ACTIVE == xiweiext->bCommand)
			{
				kuozhan_release_request(xiweiext);
				adp_phone_release_cnf_request(xiweiext);/*释放证实*/
				xiweiext->bPortState = MGW_ADP_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 0;				
			}
			break;			
		default:
			break;			
	}
	
	return DRV_OK;
}

static int32 KuoZhan_Adp_PHONE_TALK_Process(TERMINAL_BASE* xiweiext)
{
	switch(xiweiext->bPortState1)
	{
		case 0:
			xiweiext->wPortDelay = 0;
			
			if(xiweiext->bHookStatus == HOOK_ON)
			{
				kuozhan_release_request(xiweiext);
				adp_phone_release_cnf_request(xiweiext);/*释放证实*/
				xiweiext->bPortState1 = 1;
			}

			if(xiweiext->bCommand == MC_TRANS)
			{
				//mgw_scu_calltrans_request(xiweiext);
				//adp_phone_release_request(xiweiext);
				//xiweiext->bPortState1 = 1;
			}

			if(1 == xiweiext->phone_queue_s.bFinish)
			{
				adp_phone_transphone_request(xiweiext);
				xiweiext->phone_queue_s.bPhoneQueueHead = 0;
				xiweiext->phone_queue_s.bPhoneQueueTail = 0;
				xiweiext->phone_queue_s.bFinish = 0;
			}

			if(xiweiext->bCommand == MC_HANG)
			{
				kuozhan_release_cnf_request(xiweiext);
				adp_phone_release_request(xiweiext);
				xiweiext->bPortState1 = 1;
			}

			if(xiweiext->bPTTStatus == MGW_PTT_DOWN || xiweiext->bPTTStatus == MGW_PTT_UP)
			{
				VERBOSE_OUT(LOG_SYS,"Send PTT %s to SCU\n", \
					(xiweiext->bPTTStatus == MGW_PTT_DOWN)?"Down":"Up");

				xiweiext->ptt_status = xiweiext->bPTTStatus;
				kuozhan_ptt_request(xiweiext);
				xiweiext->bPTTStatus = MGW_PTT_HOLD;

			}
			break;
		case 1:
			if(xiweiext->wPortDelay == 0)
			{
				xiweiext->bPortState = MGW_ADP_PHONE_INIT_STATE;
				xiweiext->bPortState1 = 1;
			}
			break;
		default:
			break;			
	}
	
	return DRV_OK;
}

static int32 kuozhan_update_hookstatus(void)
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
					ptt_status = temp_val;

					if(1 == temp_val)
					{
						terminal_group[i].bPTTStatus = MGW_PTT_UP;
					}
					else if(0 == temp_val)
					{
						terminal_group[i].bPTTStatus = MGW_PTT_DOWN;
					}
					

					LOG("qinwu ppt %s \r\n", terminal_group[i].bPTTStatus ? "Up" : "Down");
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
					if(PORT_TYPE_KUOZHAN_PHONE == terminal_group[i].bPortType)
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

int32 kuozhan_terminal_call_control(const void *data)
{
	int i;
	for(i = 0;i < KUOZHAN_USER_NUM_MAX; i++)
	{
		switch(terminal_group[i].bPortType)
		{
			case PORT_TYPE_QINWU:
				switch(terminal_group[i].bPortState)
				{
					case MGW_SW_PHONE_INIT_STATE:
						KuoZhan_Seat_INIT_Process(&terminal_group[i]);
						break;
					case MGW_SW_PHONE_IDLE_STATE:
						KuoZhan_Seat_IDLE_Process(&terminal_group[i]);
						break;
					case MGW_SW_PHONE_DIAL_STATE:
						KuoZhan_Seat_DIAL_Process(&terminal_group[i]);
						break;
					case MGW_SW_PHONE_RINGTONE_STATE:
						KuoZhan_Seat_RINGTONE_Process(&terminal_group[i]);
						break;
					case MGW_SW_PHONE_RING_STATE:
						KuoZhan_Seat_RING_Process(&terminal_group[i]);
						break;
					case MGW_SW_PHONE_TALK_STATE:
						KuoZhan_Seat_TALK_Process(&terminal_group[i]);
						break;
					default:
						KuoZhan_Seat_INIT_Process(&terminal_group[i]);
						break;
				}
				break;
			case PORT_TYPE_TRK:
				switch(terminal_group[i].bPortState)
				{
					case MGW_TRK_INIT_STATE:
						KuoZhan_Trk_INIT_Process(&terminal_group[i]);
						break;
					case MGW_TRK_IDLE_STATE:
						KuoZhan_Trk_IDLE_Process(&terminal_group[i]);
						break;
					case MGW_TRK_IN_DIAL_STATE:
						KuoZhan_Trk_IN_DIAL_Process(&terminal_group[i]);
						break;
					case MGW_TRK_IN_RINGTONG_STATE:
						KuoZhan_Trk_IN_RINGTONG_Process(&terminal_group[i]);
						break;
					case MGW_TRK_IN_TALK_STATE:
						KuoZhan_Trk_IN_TALK_Process(&terminal_group[i]);
						break;
					case MGW_TRK_OUT_DIAL_STATE:
						KuoZhan_Trk_OUT_DIAL_Process(&terminal_group[i]);
						break;
					case MGW_TRK_OUT_RINGTONE_STATE:
						KuoZhan_Trk_OUT_RINGTONE_Process(&terminal_group[i]);
						break;
					case MGW_TRK_OUT_TALK_STATE:
						KuoZhan_Trk_OUT_TALK_Process(&terminal_group[i]);
						break;
					default:
						terminal_group[i].bPortState = MGW_TRK_INIT_STATE;
						break;
				}			
				break;
			case PORT_TYPE_KUOZHAN_PHONE:
				switch(terminal_group[i].bPortState)
				{
					case MGW_PHONE_INIT_STATE:
						KuoZhan_Phone_INIT_Process(&terminal_group[i]);
						break;
					case MGW_PHONE_IDLE_STATE:
						KuoZhan_Phone_IDLE_Process(&terminal_group[i]);
						break;
					case MGW_PHONE_DIAL_STATE:
						KuoZhan_Phone_DIAL_Process(&terminal_group[i]);
						break;
					case MGW_PHONE_RINGTONE_STATE:
						KuoZhan_Phone_RINGTONE_Process(&terminal_group[i]);
						break;
					case MGW_PHONE_TALK_STATE:
						KuoZhan_Phone_TALK_Process(&terminal_group[i]);
						break;
					case MGW_PHONE_RING_STATE:
						KuoZhan_Phone_RING_Process(&terminal_group[i]);
						break;
					default:
						KuoZhan_Phone_INIT_Process(&terminal_group[i]);
						break;
				}
				break;
			case PORT_TYPE_KUOZHAN_XIWEI:	
				switch(terminal_group[i].bPortState)
				{
					case MGW_SW_PHONE_INIT_STATE:
						KuoZhan_XiWei_INIT_Process(&terminal_group[i]);
						break;
					case MGW_SW_PHONE_IDLE_STATE:
						KuoZhan_XiWei_IDLE_Process(&terminal_group[i]);
						break;
					case MGW_SW_PHONE_DIAL_STATE:
						KuoZhan_XiWei_DIAL_Process(&terminal_group[i]);
						break;
					case MGW_SW_PHONE_RINGTONE_STATE:
						KuoZhan_XiWei_RINGTONE_Process(&terminal_group[i]);
						break;
					case MGW_SW_PHONE_RING_STATE:
						KuoZhan_XiWei_RING_Process(&terminal_group[i]);
						break;
					case MGW_SW_PHONE_TALK_STATE:
						KuoZhan_XiWei_TALK_Process(&terminal_group[i]);
						break;
					default:
						KuoZhan_XiWei_INIT_Process(&terminal_group[i]);
						break;
				}
				break;
			case PORT_TYPE_ADP_PHONE:
				switch(terminal_group[i].bPortState)
				{
					case MGW_ADP_PHONE_INIT_STATE:
						KuoZhan_Adp_PHONE_INIT_Process(&terminal_group[i]);
						break;
					case MGW_ADP_PHONE_IDLE_STATE:
						KuoZhan_Adp_PHONE_IDLE_Process(&terminal_group[i]);
						break;
					case MGW_ADP_PHONE_DIAL_STATE:
						KuoZhan_Adp_PHONE_DIAL_Process(&terminal_group[i]);
						break;
					case MGW_ADP_PHONE_RINGTONE_STATE:
						KuoZhan_Adp_PHONE_RINGTONE_Process(&terminal_group[i]);
						break;
					case MGW_ADP_PHONE_RING_STATE:
						KuoZhan_Adp_PHONE_RING_Process(&terminal_group[i]);
						break;
					case MGW_ADP_PHONE_TALK_STATE:
						KuoZhan_Adp_PHONE_TALK_Process(&terminal_group[i]);
						break;
					default:
						KuoZhan_Adp_PHONE_INIT_Process(&terminal_group[i]);
						break;
				}
				break;
			default:
				break;
		}
		
		if(terminal_group[i].wPortDelay != 0 && terminal_group[i].wPortDelay != IDLEW)
		{
			terminal_group[i].wPortDelay--;
		}
	}
	
	kuozhan_update_hookstatus();
	time_cnt_100ms++;
	Param_update.timecnt_100ms++;
	
	return 1;
}

static int32 XiWei_Sem_Process(uint8 *buf, int32 len, uint32 ipaddr)
{
	ST_PHONE_SEM *Rec = (ST_PHONE_SEM *)buf;	

	switch(Rec->Data[0])
	{
		case EN_TYPE_CALL_REQ:
		{
			int wTemPort;
			wTemPort = find_port_by_simple_ip(ipaddr);
			if((wTemPort < KUOZHAN_PHONE_OFFSET)||(wTemPort >= KUOZHAN_ADATPER_OFFSET))
			{
				VERBOSE_OUT(LOG_SYS,"UnRegister swphone User!\n");
				return DRV_ERR;
			}

			if(Rec->Data[5] != terminal_group[wTemPort].user_num_len)
			{
				VERBOSE_OUT(LOG_SYS,"adpphone User %d phone len %d error\r\n",  \
					wTemPort - KUOZHAN_PHONE_OFFSET, Rec->Data[5]);
				break;
			}			
			
			terminal_group[wTemPort].bCallType = Rec->Data[3];
			if(terminal_group[wTemPort].bCallType >= MAX_CALL_JIEXU_TYPE)
			{
				VERBOSE_OUT(LOG_SYS,"adpphone User %d call type %d error\r\n",  \
					wTemPort - KUOZHAN_PHONE_OFFSET, Rec->Data[3]);
				return DRV_ERR;
			}

			bcd2phone((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
				(char *)&Rec->Data[6], Rec->Data[5]);
				 
			terminal_group[wTemPort].phone_queue_g.bFinish = 1;
			terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;
			terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = Rec->Data[5];
		
			terminal_group[wTemPort].bHookStatus = HOOK_OFF;			
		}
		break;
		case EN_TYPE_NUM_TRANS:
		{
			int wTemPort ;
			wTemPort = find_port_by_simple_ip(ipaddr);
			if((wTemPort < KUOZHAN_PHONE_OFFSET)||(wTemPort >= KUOZHAN_ADATPER_OFFSET))
			{
				VERBOSE_OUT(LOG_SYS,"UnRegister sw phone User!\n");
				return DRV_ERR;
			}
			
			bcd2phone((char *)terminal_group[wTemPort].phone_queue_s.bPhoneQueue, \
				(char *)&Rec->Data[5], Rec->Data[4]);

			terminal_group[wTemPort].phone_queue_s.bPhoneQueueHead = Rec->Data[4];
			terminal_group[wTemPort].phone_queue_s.bPhoneQueueTail = 0;			
			terminal_group[wTemPort].phone_queue_s.bFinish = 1;
				
			VERBOSE_OUT(LOG_SYS,"Recv sw phone %d Call  %s!\n",wTemPort - KUOZHAN_PHONE_OFFSET, \
				(char *)terminal_group[wTemPort].phone_queue_s.bPhoneQueue);
		}
		break;
		case EN_TYPE_CALL_ANS:
		{
			int wTemPort;
			wTemPort = find_port_by_simple_ip(ipaddr);
			if((wTemPort < KUOZHAN_PHONE_OFFSET)||(wTemPort >= KUOZHAN_ADATPER_OFFSET))
			{
				VERBOSE_OUT(LOG_SYS,"UnRegister sw phone User!\n");
				return DRV_ERR;
			}
			
			VERBOSE_OUT(LOG_SYS,"Recv sw phone %d Call Ans!\n",wTemPort - KUOZHAN_PHONE_OFFSET);
			
			//terminal_group[wTemPort].bCallType = Rec->Data[3];

			if(Rec->Data[4] == EN_JIEXU_FAILURE)
			{
				terminal_group[wTemPort].bCommand = MC_REJECT;
				ERR("%s: jiexu failed 0x%x\r\n", __func__, Rec->Data[4]);
			}
			else if(Rec->Data[4] == EN_JIEXU_SUCCESS)
			{	
				terminal_group[wTemPort].bCommand = MC_RINGING;
			}
			else
			{	
				terminal_group[wTemPort].bCommand = MC_REJECT;
				ERR("%s: jiexu failed 0x%x\r\n", __func__, Rec->Data[4]);
			}
		}
			break;
		case EN_TYPE_CONNECT_REQ:
		{
			int wTemPort;
			wTemPort = find_port_by_simple_ip(ipaddr);
			if((wTemPort < KUOZHAN_PHONE_OFFSET)||(wTemPort >= KUOZHAN_ADATPER_OFFSET))
			{
				VERBOSE_OUT(LOG_SYS,"UnRegister sw phone User!\n");
				return DRV_ERR;
			}
			VERBOSE_OUT(LOG_SYS,"Recv sw phone %d Connect request!\n",wTemPort - KUOZHAN_PHONE_OFFSET);
			terminal_group[wTemPort].bHookStatus = HOOK_OFF;
			
			//terminal_group[wTemPort].bCallType = Rec->Data[3];		
			terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;
			terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = Rec->Data[5];
			bcd2phone((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
				(char *)&Rec->Data[6], Rec->Data[5]);						
		}
			break;
		case EN_TYPE_RELEASE_REQ:
		{
			int wTemPort;
			wTemPort = find_port_by_simple_ip(ipaddr);
			if((wTemPort < KUOZHAN_PHONE_OFFSET)||(wTemPort >= KUOZHAN_ADATPER_OFFSET))
			{
				VERBOSE_OUT(LOG_SYS,"UnRegister sw phone User!\n");
				return DRV_ERR;
			}

			terminal_group[wTemPort].bCommand = MC_HANG_ACTIVE;
			terminal_group[wTemPort].bHookStatus = HOOK_ON;
			//terminal_group[wTemPort].bCallType =  Rec->Data[3];
			terminal_group[wTemPort].rel_reason =  Rec->Data[4];
			VERBOSE_OUT(LOG_SYS,"Recv sw phone %d Release Request!\n",wTemPort - KUOZHAN_PHONE_OFFSET);
		}
			break;
		case EN_TYPE_RELEASE_ANS:
			break;		
		case EN_TYPE_PTT:
		{
			int wTemPort;
			wTemPort = find_port_by_simple_ip(ipaddr);
			if((wTemPort < KUOZHAN_PHONE_OFFSET)||(wTemPort >= KUOZHAN_ADATPER_OFFSET))
			{
				VERBOSE_OUT(LOG_SYS,"UnRegister sw phone User!\n");
				return DRV_ERR;
			}
			
			if(Rec->Data[3] == PTT_STATE_SEND)
			{
				terminal_group[wTemPort].bPTTStatus = MGW_PTT_DOWN;
			}
			else if(Rec->Data[3] == PTT_STATE_RECV)
			{
				terminal_group[wTemPort].bPTTStatus = MGW_PTT_UP;
			}
		}
			break;
		default:
			DEBUG_OUT("Unknow call msg id\n");
			break;
	}
			
	return DRV_OK;
}

static int32 XiWei_User_Info_Inject(uint8 port_id, uint32 ipaddr)
{
	uint8 timeslot = 0;
	int tmp_port;
	ST_PHONE_SEM Send;
	int32 send_len = 0;
	int i = 0;
	int cnt = 1;

	timeslot = terminal_group[port_id].bslot_us;

	LOG("%s: \r\n", __func__);		

	memset(&Send, 0x00, sizeof(Send));
	
	if((port_id < KUOZHAN_PHONE_OFFSET)||(port_id >= KUOZHAN_ADATPER_OFFSET))
	{
		ERR("%s: timeslot error %d\r\n", __func__, timeslot);	
		return DRV_ERR;
	}

	if(0x00 != terminal_group[port_id].ans)
	{
		ERR("%s: timeslot error %d\r\n", __func__, terminal_group[port_id].bslot_us);	
		return DRV_ERR;
	}

	if(1 == terminal_group[port_id].sw_phone_834_flg)
	{
		cnt = 8;
	}

	Send.Head.ProType = 0x01;
	Send.Head.SemType = ZJCT_PARAM_MSG;

	Send.Data[0] = ZJCT_USER_INJECT_MSG;
	Send.Data[1] = 1 + 16 * cnt;
	Send.Data[2] = cnt;

	for(i = 0; i < cnt ; i++)
	{
		switch(i)
		{
			case 0:
				timeslot = terminal_group[port_id].bslot_us;
				tmp_port = terminal_group[port_id].bslot_us -1;
				break;
			case 1:
				timeslot = 2;
				tmp_port = timeslot -1;
				break;
			case 2:
				timeslot = 3;
				tmp_port = timeslot -1;				
				break;
			case 3:
				timeslot = 36;
				tmp_port = timeslot -1;				
				break;
			case 4:
				timeslot = 37;
				tmp_port = timeslot -1;				
				break;
			case 5:
				timeslot = 38;
				tmp_port = timeslot -1;				
				break;
			case 6:
				timeslot = 39;
				tmp_port = timeslot -1;				
				break;	
			case 7:
				timeslot = 40;
				tmp_port = timeslot -1;				
				break;	
			default:
				break;
		}

		Send.Data[3 + 16 * i] = terminal_group[tmp_port].bslot_us;
		
		Send.Data[4+ 16 * i] = terminal_group[tmp_port].ans;
		Send.Data[5+ 16 * i] = terminal_group[tmp_port].user_num_len;

		memcpy(&Send.Data[6+ 16 * i], terminal_group[tmp_port].user_num, 4);

		Send.Data[16+ 16 * i] = 0x00;
		Send.Data[17+ 16 * i] = 0x01;
		Send.Data[18+ 16 * i] = 0x04;	

		
	}
	

	Send.Head.Len = Send.Data[1] + 2;

	send_len = sizeof(ST_PHONE_SEM_HEAD) + Send.Head.Len;
	
	XiWei_Data_Send_by_IP((uint8 *)&Send, send_len, ipaddr);	

	return DRV_OK;
}

static int32 XiWei_Data_Thread_Process(uint8 *buf, int32 len, uint32 ipaddr)
{
	ST_PHONE_SEM *Rec = (ST_PHONE_SEM *)buf;
	ST_PHONE_SEM Reply;
	int32 tmp_port = 0;

	memset(&Reply, 0x00, sizeof(Reply));	

	switch(Rec->Head.SemType)
	{
		case ZJCT_MNG_MSG:
			if((ZJCT_MNG_LINK_MSG == Rec->Data[0])&&(0x08 == Rec->Data[1]))
			{
				Reply.Head.ProType = 0x01;
				Reply.Head.SemType = ZJCT_MNG_MSG;
				Reply.Head.Len = 0x0a;
				Reply.Data[0] = ZJCT_MNG_LINK_ACK_MSG;
				Reply.Data[1] = 0x08;
				Reply.Data[2] = (Param_update.Media_Id32 >> 24) & 0xFF;
				Reply.Data[3] = (Param_update.Media_Id32 >> 16) & 0xFF;
				Reply.Data[4] = (Param_update.Media_Id32 >>  8) & 0xFF;
				Reply.Data[5] = (Param_update.Media_Id32 >>  0) & 0xFF;				
				Reply.Data[6] = 0x02;	
				Reply.Data[7] = 0x00;
				Reply.Data[8] = Rec->Data[8];		

				tmp_port = find_port_by_simple_slot(Rec->Data[9]);
				if(tmp_port < 0)
				{
				    	ERR("%s: port error %d\r\n", __func__, tmp_port);
                   	 		return DRV_ERR;
				}
				
				if(0 == terminal_group[tmp_port].sw_phone_online)
				{
					if((tmp_port >= KUOZHAN_PHONE_OFFSET) && (tmp_port < KUOZHAN_XIWEI_OFFSET))
					{
						if(0 == terminal_group[tmp_port].sw_phone_flg)
						{
							if((PORT_TYPE_KUOZHAN_PHONE == terminal_group[tmp_port].bPortType)\
								&&((MGW_PHONE_INIT_STATE == terminal_group[tmp_port].bPortState)\
								||(MGW_PHONE_IDLE_STATE == terminal_group[tmp_port].bPortState)))
							{
								terminal_group[tmp_port].bPortType = PORT_TYPE_KUOZHAN_XIWEI;
								terminal_group[tmp_port].bPortState = MGW_SW_PHONE_INIT_STATE;
								terminal_group[tmp_port].bPortState1 = 0;
								
								terminal_group[tmp_port].sw_phone_flg = 1;
								terminal_group[tmp_port].sw_phone_online = 1;
								terminal_group[tmp_port].ipaddr = ipaddr;
								if(0x70 == Rec->Data[8])
								{
									terminal_group[tmp_port].sw_phone_834_flg = 1;
								}
								
								Reply.Data[9] = 0x00;
								terminal_group[tmp_port].timecnt_100ms = Param_update.timecnt_100ms;

								printf("sw phone %d ipaddr = 0x%x type %s online\r\n", \
									tmp_port - KUOZHAN_PHONE_OFFSET, ipaddr, \
										terminal_group[tmp_port].sw_phone_834_flg? "834":"28");
							}
							else
							{
								Reply.Data[9] = 0xFF;
							}
						}
					}
					else if((tmp_port >= KUOZHAN_XIWEI_OFFSET) && (tmp_port < KUOZHAN_ADATPER_OFFSET))
					{
						terminal_group[tmp_port].bPortState = MGW_SW_PHONE_INIT_STATE;
						terminal_group[tmp_port].bPortState1 = 0;
						
						terminal_group[tmp_port].sw_phone_flg = 1;
						terminal_group[tmp_port].sw_phone_online = 1;
						terminal_group[tmp_port].ipaddr = ipaddr;
						if(0x70 == Rec->Data[8])
						{
							terminal_group[tmp_port].sw_phone_834_flg = 1;
						}
						Reply.Data[9] = 0x00;

						terminal_group[tmp_port].timecnt_100ms = Param_update.timecnt_100ms;
						printf("sw phone %d ipaddr = 0x%x type %s online\r\n", \
							tmp_port - KUOZHAN_PHONE_OFFSET, ipaddr, \
								terminal_group[tmp_port].sw_phone_834_flg? "834":"28");						
					}
					else
					{
						Reply.Data[9] = 0xFF;
					}
				}
				else
				{
					if(ipaddr == terminal_group[tmp_port].ipaddr)
					{
						Reply.Data[9] = 0x00;
						terminal_group[tmp_port].timecnt_100ms = Param_update.timecnt_100ms;
					}
					else
					{
						Reply.Data[9] = 0x01;
					}
				}
				
				XiWei_Data_Send_by_IP((uint8 *)&Reply, Reply.Head.Len +  sizeof(ST_PHONE_SEM_HEAD), ipaddr);
			}
			break;
		case ZJCT_PARAM_MSG:
			tmp_port = find_port_by_simple_slot(Rec->Data[3]);
			if(tmp_port < 0)
    		{
    			ERR("%s find port %d error\r\n", __func__, Rec->Data[3]);
    			return DRV_ERR;
    		}
			if((ipaddr == terminal_group[tmp_port].ipaddr)&&(ZJCT_OPEN_MSG == Rec->Data[0]))
			{
				if(1 == terminal_group[tmp_port].sw_phone_flg)
				{
					XiWei_User_Info_Inject(tmp_port, ipaddr);
				}
			}
			break;
		case ZJCT_SEM_MSG:
			tmp_port = find_port_by_simple_slot(Rec->Data[2]);
    		if(tmp_port < 0)
    		{
    			ERR("%s find port %d error\r\n", __func__, Rec->Data[2]);
    			return DRV_ERR;
    		}
			if((ipaddr == terminal_group[tmp_port].ipaddr)&&(1 == terminal_group[tmp_port].sw_phone_flg))
			{
				XiWei_Sem_Process(buf, len, ipaddr);
			}			
			break;
		case ZJCT_DATA_MSG:
			break;
		default: 
			break;
	}

	return DRV_OK;
}

void XiWei_Voc_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 buf[MAX_SOCKET_LEN];
	int32 Rec_Len;
	uint32 len;
	int32 wTemPort = 0;

	len = sizeof(remote_addr);
	
	LOG("%s: start\r\n", __func__);
	
	while (1)
	{
		memset(buf, 0, sizeof(buf));
			
		if((Rec_Len = recvfrom(g_XiWeiVocSockFd, buf, MAX_SOCKET_LEN, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{
			wTemPort = find_port_by_simple_ip(remote_addr.sin_addr.s_addr);
	
			if((wTemPort < KUOZHAN_PHONE_OFFSET)||(wTemPort >= KUOZHAN_ADATPER_OFFSET))
			{
				VERBOSE_OUT(LOG_SYS,"%s error %d\r\n", __func__, wTemPort);
				continue;
			}

			if(MC_TALK == terminal_group[wTemPort].bCommand)
			{
				Kuozhan_Ac491_Voc_Process(buf + 6, 160, wTemPort);
			}
		}
	}
}

static int32 XiWei_Voc_Socket_Init(void)
{
	struct sockaddr_in my_addr = {0};

	memset(&my_addr, 0x00, sizeof(my_addr));
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(40012);
	my_addr.sin_addr.s_addr = htonl(inet_addr(get_config_var_str(mgw_cfg,"EXTEND_LOCAL_IP")));

	if ((g_XiWeiVocSockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("%s: create socket\r\n", __func__);
		return DRV_ERR;
	}

	if (DRV_ERR == bind(g_XiWeiVocSockFd, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("%s: Bind socket failed\r\n", __func__);
		close(g_XiWeiVocSockFd);
		return DRV_ERR;
	}	

	return DRV_OK;
}

static int32 XiWei_Voc_Socket_Close(void)
{
	if (g_XiWeiVocSockFd)
	{
		close(g_XiWeiVocSockFd);
	}
	
	return DRV_OK;
}

void XiWei_Data_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 buf[MAX_SOCKET_LEN];
	int32 Rec_Len;
	uint32 len;

	len = sizeof(remote_addr);
	
	LOG("%s: start\r\n", __func__);
	
	while (1)
	{
		memset(buf, 0, sizeof(buf));
			
		if((Rec_Len = recvfrom(g_XiWeiDatSockFd, buf, MAX_SOCKET_LEN, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{
			XiWei_Data_Thread_Process(buf, Rec_Len, remote_addr.sin_addr.s_addr);			
		}
	}
}

static int32 XiWei_Data_Socket_Init(void)
{
	struct sockaddr_in my_addr = {0};

	memset(&my_addr, 0x00, sizeof(my_addr));
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(40011);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((g_XiWeiDatSockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("%s: create socket\r\n", __func__);
		return DRV_ERR;
	}

	if (DRV_ERR == bind(g_XiWeiDatSockFd, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("%s: Bind socket failed\r\n", __func__);
		close(g_XiWeiDatSockFd);
		return DRV_ERR;
	}	

	return DRV_OK;
}

static int32 XiWei_Data_Socket_Close(void)
{
	if (g_XiWeiDatSockFd)
	{
		close(g_XiWeiDatSockFd);
	}
	
	return DRV_OK;
}

static int32 KuoZhan_XiWei_init(void)
{
	memset(&g_XiWeiDataSockAddr, 0x00, sizeof(g_XiWeiDataSockAddr));
	memset(&g_XiWeiVocSockAddr, 0x00, sizeof(g_XiWeiVocSockAddr));		

	if(DRV_OK != XiWei_Data_Socket_Init())
	{
		ERR("XiWei_Data_Socket_Init failed \r\n");		
		return DRV_ERR;		
	}

	if(DRV_OK != XiWei_Voc_Socket_Init())
	{
		ERR("XiWei_Voc_Socket_Init failed \r\n");		
		return DRV_ERR;		
	}
	
	return DRV_OK;
}

static int32 KuoZhan_XiWei_Close(void)
{
	if(DRV_OK != XiWei_Data_Socket_Close())
	{
		ERR("XiWei_Data_Socket_Close failed \r\n");		
		return DRV_ERR;		
	}

	if(DRV_OK != XiWei_Voc_Socket_Close())
	{
		ERR("XiWei_Voc_Socket_Close failed \r\n");		
		return DRV_ERR;		
	}
	
	return DRV_OK;
}



int32 Check_834_AdpPhone_Timeout(const void *data)
{
	int i = 0; 

	for(i = 0; i < KUOZHAN_ADATPER_NUM_MAX; i++)
	{
		if(1 == terminal_group[KUOZHAN_ADATPER_OFFSET + i].adp_phone_online)
		{
			if((Param_update.timecnt_100ms - terminal_group[KUOZHAN_ADATPER_OFFSET + i].timecnt_100ms ) > 160)
			{
				if((MGW_ADP_PHONE_INIT_STATE != terminal_group[i].bPortState)\
					&&(MGW_ADP_PHONE_IDLE_STATE != terminal_group[i].bPortState))
				{
					kuozhan_release_request(&terminal_group[KUOZHAN_ADATPER_OFFSET + i]);
				}
				
				terminal_group[KUOZHAN_ADATPER_OFFSET + i].adp_phone_online = 0;
				terminal_group[KUOZHAN_ADATPER_OFFSET + i].ipaddr = 0;
				terminal_group[KUOZHAN_ADATPER_OFFSET + i].bPortState = MGW_ADP_PHONE_INIT_STATE;

				printf("adapter phone %d offline\r\n", i);
			}
		}
	}
	return 1;
}

static int32 Adp_Phone_Sem_Process(uint8 *buf, int32 len, uint32 ipaddr)
{
	ST_ADP_PHONE_SEM *Rec = (ST_ADP_PHONE_SEM *)buf;	

	switch(Rec->Data[0])
	{
		case EN_TYPE_CALL_REQ:
		{
			int wTemPort;
			wTemPort = find_port_by_simple_slot(Rec->Data[2]);
			if((wTemPort < KUOZHAN_ADATPER_OFFSET) || (wTemPort >= KUOZHAN_USER_NUM_MAX))
			{
				VERBOSE_OUT(LOG_SYS,"UnRegister adpphone User!\n");
				return DRV_ERR;
			}

			if(Rec->Data[5] != terminal_group[wTemPort].user_num_len)
			{
				VERBOSE_OUT(LOG_SYS,"adpphone User %d phone len error %d\r\n",  \
					wTemPort - KUOZHAN_ADATPER_OFFSET, Rec->Data[5]);
				break;
			}			

			terminal_group[wTemPort].bCallType = Rec->Data[3];
			if(terminal_group[wTemPort].bCallType >= MAX_CALL_JIEXU_TYPE)
			{
				VERBOSE_OUT(LOG_SYS,"adpphone User %d phone call type error %d \r\n",  \
					wTemPort - KUOZHAN_ADATPER_OFFSET, Rec->Data[3]);
				return DRV_ERR;
			}
						
			bcd2phone((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
				       (char *)&Rec->Data[6], Rec->Data[5]);
				 
			terminal_group[wTemPort].phone_queue_g.bFinish = 1;
			terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;
			terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = Rec->Data[5];
			terminal_group[wTemPort].bHookStatus = HOOK_OFF;
		}
		break;
		case EN_TYPE_NUM_TRANS:
		{
		
		}
		break;
		case EN_TYPE_CALL_ANS:
		{
			int wTemPort;
			wTemPort = find_port_by_simple_slot(Rec->Data[2]);
			if((wTemPort < KUOZHAN_ADATPER_OFFSET) || (wTemPort >= KUOZHAN_USER_NUM_MAX))
			{
				VERBOSE_OUT(LOG_SYS,"UnRegister adp phone User!\n");
				return DRV_ERR;
			}
			
			//terminal_group[wTemPort].bCallType = Rec->Data[3];

			if(Rec->Data[4] == EN_JIEXU_FAILURE)
			{
				terminal_group[wTemPort].bCommand = MC_REJECT;
			}
			else if(Rec->Data[4] == EN_JIEXU_SUCCESS)
			{	
				terminal_group[wTemPort].bCommand = MC_RINGING;
			}
			else
			{
				terminal_group[wTemPort].bCommand = MC_REJECT;
			}
		}
			break;
		case EN_TYPE_CONNECT_REQ:
		{
			int wTemPort;
			wTemPort = find_port_by_simple_slot(Rec->Data[2]);
			if((wTemPort < KUOZHAN_ADATPER_OFFSET) || (wTemPort >= KUOZHAN_USER_NUM_MAX))
			{
				VERBOSE_OUT(LOG_SYS,"UnRegister adp phone User!\n");
				return DRV_ERR;
			}
			VERBOSE_OUT(LOG_SYS,"Recv Adp phone %d Connect request!\n",wTemPort);

			terminal_group[wTemPort].bHookStatus = HOOK_OFF;
			//terminal_group[wTemPort].bCallType = Rec->Data[3];
			terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;
			terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = Rec->Data[5];
			bcd2phone((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
				(char *)&Rec->Data[6], Rec->Data[5]);						
		}
			break;
		case EN_TYPE_RELEASE_REQ:
		{
			int wTemPort;
			wTemPort = find_port_by_simple_slot(Rec->Data[2]);
			if((wTemPort < KUOZHAN_ADATPER_OFFSET) || (wTemPort >= KUOZHAN_USER_NUM_MAX))
			{
				VERBOSE_OUT(LOG_SYS,"UnRegister Adp phone User!\n");
				return DRV_ERR;
			}

			terminal_group[wTemPort].bCommand = MC_HANG_ACTIVE;
			terminal_group[wTemPort].bHookStatus = HOOK_ON;
			//terminal_group[wTemPort].bCallType =  Rec->Data[3];
			terminal_group[wTemPort].rel_reason =  Rec->Data[4];
			VERBOSE_OUT(LOG_SYS,"Recv Adp phone %d Release Request!\n",wTemPort);
		}
			break;
		case EN_TYPE_RELEASE_ANS:
			break;		
		case EN_TYPE_PTT:
		{
			int wTemPort;
			wTemPort = find_port_by_simple_slot(Rec->Data[2]);
			if((wTemPort < KUOZHAN_ADATPER_OFFSET) || (wTemPort >= KUOZHAN_USER_NUM_MAX))
			{
				VERBOSE_OUT(LOG_SYS,"UnRegister adp phone User!\n");
				return DRV_ERR;
			}

			if(Rec->Data[3] == PTT_STATE_SEND)
			{
				terminal_group[wTemPort].bPTTStatus = MGW_PTT_DOWN;
			}
			else if(Rec->Data[3] == PTT_STATE_RECV)
			{
				terminal_group[wTemPort].bPTTStatus = MGW_PTT_UP;
			}
		}
			break;
		default:
			break;
	}
			
	return DRV_OK;
}


static int32 Adp_Phone_User_Info_Inject(uint8 *buf, uint8 port_id, uint32 ipaddr)
{
	uint8 timeslot = 0;
	ST_ADP_PHONE_SEM Send;
	ST_ADP_PHONE_SEM *Rec = (ST_ADP_PHONE_SEM *)buf;
	int32 send_len = 0;
	uint8 mem_num = 0;
	uint8 i = 0;
	int wTemPort = 0;

	timeslot = terminal_group[port_id].bslot_us;
	
	LOG("%s: \r\n", __func__);		

	memset(&Send, 0x00, sizeof(Send));

	terminal_group[port_id].member_num = Rec->Data[2];
	mem_num = terminal_group[port_id].member_num;

	#if 0
	if((timeslot < ADP_XIWEI_FIRST_ID)||(timeslot > ADP_XIWEI_END_ID))
	{
		ERR("%s: timeslot error %d\r\n", __func__, timeslot);	
		return DRV_ERR;
	}
    #endif
    
	if(0x00 != terminal_group[port_id].ans)
	{
		ERR("%s: timeslot error %d\r\n", __func__, terminal_group[port_id].bslot_us);	
		return DRV_ERR;
	}

    	Send.Head.ProType = 0x01;
	Send.Head.Edition = 0x01;
	Send.Head.SemType = ZJCT_PARAM_MSG;
	Send.Data[0] = ZJCT_USER_INJECT_MSG;
	Send.Data[1] = 16 * mem_num + 1;
    	Send.Data[2] = mem_num;
	for(i = 0; i < mem_num; i++)
	{
		wTemPort = find_port_by_simple_slot(Rec->Data[i + 3]);	
		if(DRV_ERR == wTemPort)
		{
			printf("%s port %d error\r\n", __func__, Rec->Data[i + 3]);
			return DRV_ERR;
		}
		
		Send.Data[i * 16 + 3] = Rec->Data[i + 3];
		
		Send.Data[i * 16 + 4] = terminal_group[wTemPort].ans;
		Send.Data[i * 16 + 5] = terminal_group[wTemPort].user_num_len;		
		memcpy(&Send.Data[i * 16 + 6], terminal_group[wTemPort].user_num, 4);

		Send.Data[i * 16 + 16] = 0x00;//terminal_group[port_id].conference_enable;
		Send.Data[i * 16 + 17] = 0x01;//terminal_group[port_id].priority;
		Send.Data[i * 16 + 18] = 0x04;//terminal_group[port_id].encoder;	
	}
	
	Send.Head.Len = Send.Data[1] + 2;

	send_len = sizeof(ST_ADP_PHONE_SEM_HEAD) + Send.Head.Len;
	
	Adp_Phone_Data_Send_by_IP((uint8 *)&Send, send_len, ipaddr);	

	return DRV_OK;
}

static int32 Adp_Phone_User_Info_Inject_all(void)
{
	ST_ADP_PHONE_SEM Send;

	int32 send_len = 0;
	uint8 mem_num = 0;
	uint8 i = 0;
	int wTemPort = 0;

	LOG("%s: \r\n", __func__);		

	memset(&Send, 0x00, sizeof(Send));

	mem_num = 10;
	wTemPort = 0;
    	Send.Head.ProType = 0x01;
	Send.Head.Edition = 0x01;
	Send.Head.SemType = ZJCT_PARAM_MSG;
	Send.Data[0] = ZJCT_USER_INJECT_MSG;
	Send.Data[1] = 16 * mem_num + 1;
    	Send.Data[2] = mem_num;
	for(i = 0; i < mem_num; i++, wTemPort++)
	{
		Send.Data[i * 16 + 3] = terminal_group[wTemPort].bslot_us;
		
		Send.Data[i * 16 + 4] = terminal_group[wTemPort].ans;
		Send.Data[i * 16 + 5] = terminal_group[wTemPort].user_num_len;		
		memcpy(&Send.Data[i * 16 + 6], terminal_group[wTemPort].user_num, 4);

		Send.Data[i * 16 + 16] = 0x00;//terminal_group[port_id].conference_enable;
		Send.Data[i * 16 + 17] = 0x01;//terminal_group[port_id].priority;
		Send.Data[i * 16 + 18] = 0x04;//terminal_group[port_id].encoder;	
	}
	
	Send.Head.Len = Send.Data[1] + 2;
	send_len = sizeof(ST_ADP_PHONE_SEM_HEAD) + Send.Head.Len;
	Adp_Phone_Data_Send_by_IP((uint8 *)&Send, send_len, \
		terminal_group[KUOZHAN_ADATPER_OFFSET].ipaddr);	

	mem_num = 10;
	wTemPort = 10;
    	Send.Head.ProType = 0x01;
	Send.Head.Edition = 0x01;
	Send.Head.SemType = ZJCT_PARAM_MSG;
	Send.Data[0] = ZJCT_USER_INJECT_MSG;
	Send.Data[1] = 16 * mem_num + 1;
    	Send.Data[2] = mem_num;
	for(i = 0; i < mem_num; i++, wTemPort++)
	{
		Send.Data[i * 16 + 3] = terminal_group[wTemPort].bslot_us;
		
		Send.Data[i * 16 + 4] = terminal_group[wTemPort].ans;
		Send.Data[i * 16 + 5] = terminal_group[wTemPort].user_num_len;		
		memcpy(&Send.Data[i * 16 + 6], terminal_group[wTemPort].user_num, 4);

		Send.Data[i * 16 + 16] = 0x00;//terminal_group[port_id].conference_enable;
		Send.Data[i * 16 + 17] = 0x01;//terminal_group[port_id].priority;
		Send.Data[i * 16 + 18] = 0x04;//terminal_group[port_id].encoder;	
	}
	
	Send.Head.Len = Send.Data[1] + 2;
	send_len = sizeof(ST_ADP_PHONE_SEM_HEAD) + Send.Head.Len;
	Adp_Phone_Data_Send_by_IP((uint8 *)&Send, send_len, \
		terminal_group[KUOZHAN_ADATPER_OFFSET].ipaddr);		

	mem_num = 10;
	wTemPort = 20;
    	Send.Head.ProType = 0x01;
	Send.Head.Edition = 0x01;
	Send.Head.SemType = ZJCT_PARAM_MSG;
	Send.Data[0] = ZJCT_USER_INJECT_MSG;
	Send.Data[1] = 16 * mem_num + 1;
    	Send.Data[2] = mem_num;
	for(i = 0; i < mem_num; i++, wTemPort++)
	{
		Send.Data[i * 16 + 3] = terminal_group[wTemPort].bslot_us;
		
		Send.Data[i * 16 + 4] = terminal_group[wTemPort].ans;
		Send.Data[i * 16 + 5] = terminal_group[wTemPort].user_num_len;		
		memcpy(&Send.Data[i * 16 + 6], terminal_group[wTemPort].user_num, 4);

		Send.Data[i * 16 + 16] = 0x00;//terminal_group[port_id].conference_enable;
		Send.Data[i * 16 + 17] = 0x01;//terminal_group[port_id].priority;
		Send.Data[i * 16 + 18] = 0x04;//terminal_group[port_id].encoder;	
	}
	
	Send.Head.Len = Send.Data[1] + 2;
	send_len = sizeof(ST_ADP_PHONE_SEM_HEAD) + Send.Head.Len;
	Adp_Phone_Data_Send_by_IP((uint8 *)&Send, send_len, \
		terminal_group[KUOZHAN_ADATPER_OFFSET].ipaddr);		

	mem_num = 10;
	wTemPort = 30;
    	Send.Head.ProType = 0x01;
	Send.Head.Edition = 0x01;
	Send.Head.SemType = ZJCT_PARAM_MSG;
	Send.Data[0] = ZJCT_USER_INJECT_MSG;
	Send.Data[1] = 16 * mem_num + 1;
    	Send.Data[2] = mem_num;
	for(i = 0; i < mem_num; i++, wTemPort++)
	{
		Send.Data[i * 16 + 3] = terminal_group[wTemPort].bslot_us;
		
		Send.Data[i * 16 + 4] = terminal_group[wTemPort].ans;
		Send.Data[i * 16 + 5] = terminal_group[wTemPort].user_num_len;		
		memcpy(&Send.Data[i * 16 + 6], terminal_group[wTemPort].user_num, 4);

		Send.Data[i * 16 + 16] = 0x00;//terminal_group[port_id].conference_enable;
		Send.Data[i * 16 + 17] = 0x01;//terminal_group[port_id].priority;
		Send.Data[i * 16 + 18] = 0x04;//terminal_group[port_id].encoder;	
	}
	
	Send.Head.Len = Send.Data[1] + 2;
	send_len = sizeof(ST_ADP_PHONE_SEM_HEAD) + Send.Head.Len;
	Adp_Phone_Data_Send_by_IP((uint8 *)&Send, send_len, \
		terminal_group[KUOZHAN_ADATPER_OFFSET].ipaddr);			

	return DRV_OK;
}

int32 Adp_Phone_Zhenru_Fenji_Inject_all(void)
{
	ST_ADP_PHONE_SEM Send;

	int32 send_len = 0;
	uint8 mem_num = 0;
	uint8 i = 0;
	int wTemPort = 0;
	int id = 0;	

	LOG("%s: \r\n", __func__);		

	memset(&Send, 0x00, sizeof(Send));

	mem_num = 10;

    	Send.Head.ProType = 0x01;
	Send.Head.Edition = 0x01;
	Send.Head.SemType = ZJCT_PARAM_MSG;
	Send.Data[0] = ZJCT_ZHENRU_FENJI_REG_ACK;
	Send.Data[1] = 13 * mem_num + 1;
   	Send.Data[2] = mem_num;
   	
	for(i = 0; i < mem_num; i++)
	{
		switch(i)
		{
			case 0:
				id = 31;
				break;
			case 1:
				id = 32;
				break;
			case 2:
				id = 33;
				break;
			case 3:
				id = 34;
				break;
			case 4:
				id = 35;
				break;
			case 5:
				id = 36;
				break;
			case 6:
				id = 37;
				break;
			case 7:
				id = 38;
				break;
			case 8:
				id = 39;
				break;
			case 9:
				id = 40;
				break;
			default:
				break;								
		}		


		Send.Data[i * 13 + 3] = id;
		wTemPort = find_port_by_simple_slot(id);
		if(wTemPort < 0)
		{
			ERR("%s find port %d error\r\n", __func__, id);
			return DRV_ERR;
		}
		
		if(0 == terminal_group[wTemPort].zhenru_slot)
		{
			Send.Data[i * 13 + 4] = 0xFF;
		}
		else
		{
			Send.Data[i * 13 + 4] = 0x00;	
			Send.Data[i * 13 + 5] = terminal_group[wTemPort].zhenru_slot;
			wTemPort = find_port_by_simple_slot(terminal_group[wTemPort].zhenru_slot);
    		if(wTemPort < 0)
    		{
    			ERR("%s find port %d error\r\n", __func__, Send.Data[i * 13 + 5]);
    			return DRV_ERR;
    		}
	
			memcpy(&Send.Data[i * 13 + 6], terminal_group[wTemPort].user_num, 4);			
		}
	}
	
	Send.Head.Len = Send.Data[1] + 2;
	send_len = sizeof(ST_ADP_PHONE_SEM_HEAD) + Send.Head.Len;
	Adp_Phone_Data_Send_by_IP((uint8 *)&Send, send_len, \
		terminal_group[KUOZHAN_ADATPER_OFFSET].ipaddr);	

	return DRV_OK;
}

static int32 Adp_Phone_Data_Thread_Process(uint8 *buf, int32 len, uint32 ipaddr)
{    
	ST_ADP_PHONE_SEM *Rec = (ST_ADP_PHONE_SEM *)buf;
	ST_ADP_PHONE_SEM Reply;
	int32 tmp_port = 0;
	int i = 0;

	memset(&Reply, 0x00, sizeof(Reply));
	
	switch(Rec->Head.SemType)
	{    
		case ZJCT_MNG_MSG:		    
			if((ZJCT_MNG_LINK_MSG == Rec->Data[0])&&(0x08 == Rec->Data[1]))
			{
			    Reply.Head.ProType = 0x01;
				Reply.Head.Edition = 0x01;
				Reply.Head.SemType = ZJCT_MNG_MSG;
				Reply.Head.Len = 0x0a;
				Reply.Data[0] = ZJCT_MNG_LINK_ACK_MSG;
				Reply.Data[1] = 0x08;
				Reply.Data[2] = (Param_update.Media_Id32 >> 24) & 0xFF;
				Reply.Data[3] = (Param_update.Media_Id32 >> 16) & 0xFF;
				Reply.Data[4] = (Param_update.Media_Id32 >>  8) & 0xFF;
				Reply.Data[5] = (Param_update.Media_Id32 >>  0) & 0xFF;				
				Reply.Data[6] = 0x14;	
				Reply.Data[7] = 0x01;
				Reply.Data[8] = 0x60;	

				tmp_port = find_port_by_simple_slot(Rec->Data[9]);
				if(tmp_port != KUOZHAN_ADATPER_OFFSET)
				{
					ERR("%s mng link port %d error\r\n", __func__, tmp_port);
					return DRV_ERR;
				}
				
				if(0 == terminal_group[tmp_port].adp_phone_online)
				{
					printf("adp phone %d ipaddr = 0x%x online\r\n", tmp_port, ipaddr);
					
					for(i = 0; i < KUOZHAN_ADATPER_NUM_MAX; i++)
					{
						terminal_group[tmp_port + i].ipaddr = ipaddr;
						terminal_group[tmp_port + i].adp_phone_online = 1;
						terminal_group[tmp_port + i].bPortState = MGW_ADP_PHONE_INIT_STATE;
						terminal_group[tmp_port + i].timecnt_100ms = Param_update.timecnt_100ms;	
					}
				
					Reply.Data[9] = 0x00;
				}
				else
				{
					if(ipaddr == terminal_group[tmp_port].ipaddr)
					{
						Reply.Data[9] = 0x00;

						for(i = 0; i < KUOZHAN_ADATPER_NUM_MAX; i++)
						{
							terminal_group[tmp_port + i].timecnt_100ms = Param_update.timecnt_100ms;	
						}
					}
					else
					{
						Reply.Data[9] = 0x01;
					}
				}
				
				Adp_Phone_Data_Send_by_IP((uint8 *)&Reply, Reply.Head.Len +  sizeof(ST_ADP_PHONE_SEM_HEAD), ipaddr);
			}
			break;
		case ZJCT_PARAM_MSG:
			tmp_port = find_port_by_simple_slot(Rec->Data[3]);
			if(tmp_port != KUOZHAN_ADATPER_OFFSET)
			{
				ERR("%s open user  port %d error\r\n", __func__, tmp_port);
				return DRV_ERR;
			}	
			
			if((ipaddr == terminal_group[tmp_port].ipaddr)\
				&&(1 == terminal_group[tmp_port].adp_phone_online))
			{
				switch(Rec->Data[0])
				{
					case ZJCT_OPEN_MSG:
						Adp_Phone_User_Info_Inject(buf, tmp_port, ipaddr);
						Adp_Phone_User_Info_Inject_all();
						break;
					case ZJCT_ZHENRU_FENJI_REG:
						Adp_Phone_Zhenru_Fenji_Inject_all();
						break;
					default:
						break;
				}
			}
			break;
		case ZJCT_SEM_MSG:
			tmp_port = find_port_by_simple_slot(Rec->Data[2]);	
    		if(tmp_port < 0)
    		{
    			ERR("%s find port %d error\r\n", __func__, Rec->Data[2]);
    			return DRV_ERR;
    		}
			if((ipaddr == terminal_group[tmp_port].ipaddr)&&(1 == terminal_group[tmp_port].adp_phone_online))
			{
				Adp_Phone_Sem_Process(buf, len, ipaddr);
			}			
			break;
		default: 
			break;
	}

	return DRV_OK;
}

void Adp_Phone_Voc_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 buf[MAX_SOCKET_LEN];
	int32 Rec_Len;
	uint32 len;
	int32 wTemPort = 0;
	
	len = sizeof(remote_addr);
	
	LOG("%s: start\r\n", __func__);
	
	while (1)
	{
		memset(buf, 0, sizeof(buf));
			
		if((Rec_Len = recvfrom(g_AdpPhoneVocSockFd, buf, MAX_SOCKET_LEN, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{
			wTemPort = find_port_by_simple_slot(buf[6]);	

			if((wTemPort < KUOZHAN_ADATPER_OFFSET) || (wTemPort >=KUOZHAN_USER_NUM_MAX))
			{
				VERBOSE_OUT(LOG_SYS,"%s error %d\r\n", __func__, wTemPort);
				continue;
			}			
			
			if(MC_TALK == terminal_group[wTemPort].bCommand)
			{
				Kuozhan_Ac491_Voc_Process(buf + 9, 160, wTemPort);
			}
		}
	}
}

static int32 Adp_Phone_Voc_Socket_Init(void)
{
	struct sockaddr_in my_addr = {0};

	memset(&my_addr, 0x00, sizeof(my_addr));
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(40006);
	my_addr.sin_addr.s_addr = htonl(inet_addr("192.168.9.3"));

	if ((g_AdpPhoneVocSockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("%s: create socket\r\n", __func__);
		return DRV_ERR;
	}

	if (DRV_ERR == bind(g_AdpPhoneVocSockFd, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("%s: Bind socket failed\r\n", __func__);
		close(g_AdpPhoneVocSockFd);
		return DRV_ERR;
	}	

	return DRV_OK;
}

static int32 Adp_Phone_Voc_Socket_Close(void)
{
	if (g_AdpPhoneVocSockFd)
	{
		close(g_AdpPhoneVocSockFd);
	}
	
	return DRV_OK;
}

void Adp_Phone_Data_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 buf[MAX_SOCKET_LEN];
	int32 Rec_Len;
	uint32 len;

	len = sizeof(remote_addr);
	
	LOG("%s: start\r\n", __func__);
	
	while (1)
	{
		memset(buf, 0, sizeof(buf));

		if((Rec_Len = recvfrom(g_AdpPhoneDatSockFd, buf, MAX_SOCKET_LEN, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{
			Adp_Phone_Data_Thread_Process(buf, Rec_Len, remote_addr.sin_addr.s_addr);			
		}
	}
}

static int32 Adp_Phone_Data_Socket_Init(void)
{
	struct sockaddr_in my_addr = {0};

	memset(&my_addr, 0x00, sizeof(my_addr));
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(40005);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((g_AdpPhoneDatSockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("%s: create socket\r\n", __func__);
		return DRV_ERR;
	}

	if (DRV_ERR == bind(g_AdpPhoneDatSockFd, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("%s: Bind socket failed\r\n", __func__);
		close(g_AdpPhoneDatSockFd);
		return DRV_ERR;
	}	

	return DRV_OK;
}

static int32 Adp_Phone_Data_Socket_Close(void)
{
	if (g_AdpPhoneDatSockFd)
	{
		close(g_AdpPhoneDatSockFd);
	}
	
	return DRV_OK;
}


static int32 Adp_Socket_Phone_init(void)
{
	memset(&g_AdptphDataSockAddr, 0x00, sizeof(g_AdptphDataSockAddr));
	memset(&g_AdptphVocSockAddr, 0x00, sizeof(g_AdptphVocSockAddr));		

	if(DRV_OK != Adp_Phone_Data_Socket_Init())
	{
		ERR("Adp_Phone_Data_Socket_Init failed \r\n");		
		return DRV_ERR;		
	}

	if(DRV_OK != Adp_Phone_Voc_Socket_Init())
	{
		ERR("Adp_Phone_Voc_Socket_Init failed \r\n");		
		return DRV_ERR;		
	}

	return DRV_OK;
}

static int32 Adp_Socket_Phone_Close(void)
{
	if(DRV_OK != Adp_Phone_Data_Socket_Close())
	{
		ERR("Adp_Phone_Data_Socket_Close failed \r\n");		
		return DRV_ERR;		
	}

	if(DRV_OK != Adp_Phone_Voc_Socket_Close())
	{
		ERR("Adp_Phone_Voc_Socket_Close failed \r\n");		
		return DRV_ERR;		
	}

	return DRV_OK;
}

static int32 Seat_Data_Thread_Process(uint8 *buf, int32 len, uint32 addr_src)
{    
	/*1.进行校验，检查*/

	ST_HF_MGW_EXT_PRO* pro = (ST_HF_MGW_EXT_PRO*)buf;
	
	if(pro->header.sync_head != 0x55aa)
	{
		return (-1);
	}

	/* 必需特别注意，使用结构体代替协议需进行字节对齐，在此应使用4字节对齐*/
	pro->checksum = *(unsigned short*)&pro->body[pro->header.len];
	if(!MGW_EQUAL(pro->checksum,cal_checksum((BYTE*)pro,len-2)))
	{
		DEBUG_OUT("Checksum Error! %.4x : %.4x\n",pro->checksum, cal_checksum((BYTE*)pro,len-2));
		return DRV_ERR;
	}
	
	switch(pro->header.type)
	{
		case BAOWEN_TYPE_CALLCFG:	/*呼叫配置报文*/
			switch(pro->body[0])
			{
				case EN_TYPE_USER_NUM_ANS:
					{
						int port;
						ST_HF_MGW_EXT_USER_NUM_ANS * ans = (ST_HF_MGW_EXT_USER_NUM_ANS *)pro->body;
						//if(ans->ans_code == 0x00)
						if(1)
						{
							port = find_port_by_slot(ans->mem_slot);
							if(port<0){
								VERBOSE_OUT(LOG_SYS,"Unkonw slot %d in user num ans!\n",ans->mem_slot);
								break;
							}else
								inc_sched_del(con,terminal_group[port].sched_assign_id);
						}else
							inc_log(LOG_DEBUG,"Recv User num error ans!\n");
					}
					break;
				default:
					VERBOSE_OUT(LOG_SYS,"Unkonw call cfg baowen id:%d!\n",pro->body[0]);
					break;
			}
			break;
		case BAOWEN_TYPE_CALLCMD:	/*语音呼叫信令报文*/
			switch(((ST_HF_MGW_EXT_CALL_REQ*)pro->body)->id)
			{
				case EN_TYPE_CALL_REQ:
					{
						int wTemPort;
						wTemPort = find_port_by_simple_ip(addr_src);
						if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
						{
							VERBOSE_OUT(LOG_SYS,"UnRegister XIWEI User!\n");

							/*在没有找到网络用户的情况下，应向话音板发送呼叫应答，携带错误状态信息*/
							return DRV_ERR;
						}
						
						if(((ST_HF_MGW_EXT_CALL_REQ*)pro->body)->phone_len != terminal_group[wTemPort].user_num_len)
						{
							VERBOSE_OUT(LOG_SYS,"seat User %d phone len error\r\n",  \
								wTemPort);
							break;
						}	
						
						terminal_group[wTemPort].bCallType = ((ST_HF_MGW_EXT_CALL_REQ*)pro->body)->call_type;
						if(terminal_group[wTemPort].bCallType >= MAX_CALL_JIEXU_TYPE)
						{
							VERBOSE_OUT(LOG_SYS,"Recv XIWEI%d Call Request error !,type:%d\n",wTemPort, \
								terminal_group[wTemPort].bCallType);						
							return DRV_ERR;
						}
						
						terminal_group[wTemPort].phone_queue_g.bFinish = 1;
						terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;
						terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = ((ST_HF_MGW_EXT_CALL_REQ*)pro->body)->phone_len;
						bcd2phone((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
							(char *)((ST_HF_MGW_EXT_CALL_REQ*)pro->body)->callee_num,\
								((ST_HF_MGW_EXT_CALL_REQ*)pro->body)->phone_len);

						terminal_group[wTemPort].bHookStatus = HOOK_OFF;

						VERBOSE_OUT(LOG_SYS,"Recv XIWEI%d Call Request!,type:%d\n",wTemPort, \
							terminal_group[wTemPort].bCallType);
					}
					break;
				case EN_TYPE_NUM_TRANS:
					{
					
					}
					break;
				case EN_TYPE_CALL_ANS:
					{
						int wTemPort;
						wTemPort = find_port_by_simple_ip(addr_src);
						if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
						{
							VERBOSE_OUT(LOG_SYS,"UnRegister XIWEI User!\n");
							return DRV_ERR;
						}
						VERBOSE_OUT(LOG_SYS,"Recv XIWEI %d Call Ans!\n",wTemPort);
						
						//terminal_group[wTemPort].bCallType = ((ST_HF_MGW_EXT_CALLACK_REQ*)pro->body)->call_type;

						if(((ST_HF_MGW_EXT_CALLACK_REQ*)pro->body)->call_statu == EN_JIEXU_FAILURE)
							terminal_group[wTemPort].bCommand = MC_REJECT;
						else if(((ST_HF_MGW_EXT_CALLACK_REQ*)pro->body)->call_statu == EN_JIEXU_SUCCESS)
							terminal_group[wTemPort].bCommand = MC_RINGING;
						else
							terminal_group[wTemPort].bCommand = MC_REJECT;
					}
					break;
				case EN_TYPE_CONNECT_REQ:
				{
					int wTemPort;
					wTemPort = find_port_by_simple_ip(addr_src);
					if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
					{
						VERBOSE_OUT(LOG_SYS,"UnRegister XIWEI User!\n");
						return DRV_ERR;
					}
					VERBOSE_OUT(LOG_SYS,"Recv XIWEI %d Connect request!\n",wTemPort);
					terminal_group[wTemPort].bHookStatus = HOOK_OFF;
					//terminal_group[wTemPort].bCallType = ((ST_HF_MGW_EXT_CONNECT_REQ*)pro->body)->call_type;
					terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;
					terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = ((ST_HF_MGW_EXT_CALL_REQ*)pro->body)->phone_len;
					bcd2phone((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
						(char *)((ST_HF_MGW_EXT_CALL_REQ*)pro->body)->callee_num,\
						((ST_HF_MGW_EXT_CALL_REQ*)pro->body)->phone_len);
				}
					break;
				case EN_TYPE_RELEASE_REQ:
				{
					int wTemPort;
					wTemPort = find_port_by_simple_ip(addr_src);
					if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
					{
						VERBOSE_OUT(LOG_SYS,"UnRegister XIWEI User!\n");
						return DRV_ERR;
					}

					terminal_group[wTemPort].bCommand = MC_HANG_ACTIVE;
					terminal_group[wTemPort].bHookStatus = HOOK_ON;
					//terminal_group[wTemPort].bCallType = ((ST_HF_MGW_EXT_RELEASE_REQ*)pro->body)->call_type;
					terminal_group[wTemPort].rel_reason = ((ST_HF_MGW_EXT_RELEASE_REQ*)pro->body)->rel_type;
					VERBOSE_OUT(LOG_SYS,"Recv XIWEI %d Release Request!\n",wTemPort);
				}
					break;
				case EN_TYPE_PTT:
				{
					int wTemPort;
					ST_HF_MGW_EXT_PTT_MSG * msg = (ST_HF_MGW_EXT_PTT_MSG *)pro->body;
					wTemPort = find_port_by_simple_ip(addr_src);
					if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
					{
						VERBOSE_OUT(LOG_SYS,"UnRegister XIWEI User!\n");
						return DRV_ERR;
					}
					if(msg->ptt_value == PTT_STATE_SEND)
					{
						terminal_group[wTemPort].bPTTStatus = MGW_PTT_DOWN;
					}
					else if(msg->ptt_value == PTT_STATE_RECV)
					{
						terminal_group[wTemPort].bPTTStatus = MGW_PTT_UP;
					}
				}
					break;
				case EN_TYPE_RELEASE_ANS:
					break;
				default:
					DEBUG_OUT("Unknow call msg id\n");
					break;
			}
			break;
		default:
			DEBUG_OUT("Recv Unknow BAOWEN \n");
			break;
	}

	return DRV_OK;
}


void Seat_Data_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 buf[MAX_SOCKET_LEN];
	int32 Rec_Len;
	uint32 len;

	len = sizeof(remote_addr);
	
	LOG("%s: start\r\n", __func__);
	
	while (1)
	{
		memset(buf, 0, sizeof(buf));

		if((Rec_Len = recvfrom(g_SeatDatSockFd, buf, MAX_SOCKET_LEN, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{
#if 0		
			int i = 0;
			printf("%s\r\n", __func__);
			for(i = 0; i < Rec_Len; i++)
			{
				printf("%x:", buf[i]);
			}
			printf("\r\n");
#endif		
			Seat_Data_Thread_Process(buf, Rec_Len, remote_addr.sin_addr.s_addr);			
		}
	}
}

static int32 Seat_Data_Socket_Init(void)
{
	struct sockaddr_in my_addr = {0};

	memset(&my_addr, 0x00, sizeof(my_addr));
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(50718);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((g_SeatDatSockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("%s: create socket\r\n", __func__);
		return DRV_ERR;
	}

	if (DRV_ERR == bind(g_SeatDatSockFd, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("%s: Bind socket failed\r\n", __func__);
		close(g_SeatDatSockFd);
		return DRV_ERR;
	}	

	return DRV_OK;
}

static int32 Seat_Data_Socket_Close(void)
{
	if (g_SeatDatSockFd)
	{
		close(g_SeatDatSockFd);
	}
	
	return DRV_OK;
}

int32 kuozhan_socket_init(const void *data)
{
	KuoZhan_XiWei_init();
	Adp_Socket_Phone_init();
	Seat_Data_Socket_Init();
	
	return DRV_OK;
}

int32 kuozhan_socket_Close(void)
{
	KuoZhan_XiWei_Close();
	Adp_Socket_Phone_Close();
	Seat_Data_Socket_Close();
	
	return DRV_OK;
}

static int32 huozhan_phone_num_init(unsigned chan)
{
	snprintf(terminal_group[chan].name, 10, "%d", 801 + chan);	
	terminal_group[chan].ans = 0x00;

	phone2bcd((char *)terminal_group[chan].user_num, \
		(char *)terminal_group[chan].name, terminal_group[chan].user_num_len);

	printf("%s: chan %d %s\r\n", __func__, chan, terminal_group[chan].name);
	return DRV_OK;
}

static int32 huozhan_conf_num_init(void)
{
	int i = 0; 
	char name[10] = {0};
	int val = 0;

	
	kuozhan_conf[0].conf_num_len = 3;
	kuozhan_conf[0].conf_cnt = get_config_cat_var_val(mgw_cfg, PHONE, "conf_cnt");
	snprintf(kuozhan_conf[0].conf_name, 10, \
		"%d", get_config_cat_var_val(mgw_cfg, PHONE, "conf_id"));	
	phone2bcd((char *)kuozhan_conf[0].conf_user_name, \
		(char *)kuozhan_conf[0].conf_name, kuozhan_conf[0].conf_num_len);

	if(kuozhan_conf[0].conf_cnt > KUOZHAN_CONF_MEM_MAX_USE)
	{
		kuozhan_conf[0].conf_cnt = KUOZHAN_CONF_MEM_MAX_USE;
	}

	for(i = 0; i < kuozhan_conf[0].conf_cnt; i++)
	{
		snprintf(name, 10, "phone%d", i);
		val = get_config_cat_var_val(mgw_cfg, PHONE, name);
		snprintf(kuozhan_conf[0].conf_phone[i], 10, "%d", val);
		if(0 != val)
		{
			kuozhan_conf[0].conf_phone_flg[i] = 1;
		}
	}
	
	return DRV_OK;
}

static int32 zhenru_feiji_init(void)
{
	int i = 0; 
	int id = 0;
	char name[10] = {0};
	int tmp_port = 0;

	for(i = 0; i < 12; i++)
	{
		switch(i)
		{
			case 0:
				id = 2;
				break;
			case 1:
				id = 3;
				break;
			case 2:
				id = 31;
				break;
			case 3:
				id = 32;
				break;
			case 4:
				id = 33;
				break;
			case 5:
				id = 34;
				break;
			case 6:
				id = 35;
				break;
			case 7:
				id = 36;
				break;
			case 8:
				id = 37;
				break;
			case 9:
				id = 38;
				break;
			case 10:
				id = 39;
				break;
			case 11:
				id = 40;
				break;
			default:
				break;									
		}

		snprintf(name, sizeof(name), "FENJI%d", i);
		tmp_port = find_port_by_simple_slot(id);
		if(tmp_port < 0)
		{
			ERR("%s find port %d error\r\n", __func__, id);
			return DRV_ERR;
		}
		
		terminal_group[tmp_port].zhenru_slot = get_config_var_val(mgw_cfg, name);		
	}
	
	return DRV_OK;
}


int32 kuozhan_init_terminal(const void *data)
{
	uint8 i = 0;
	
	memset(terminal_group, 0x00, sizeof(terminal_group));
	memset(kuozhan_conf, 0x00, sizeof(kuozhan_conf));


	Ac491DrvInit();

	g_mgwdrv = open("/dev/mgwdrv",O_RDWR);
	if(g_mgwdrv < 0)
	{
		VERBOSE_OUT(LOG_SYS,"Open MGW Driver Error:%s\n",strerror(errno));

		return 0;
	}

	Param_update.num_len = 0x03;

	/*Qinwu init */
	for(i = KUOZHAN_QINWU_OFFSET; i < (KUOZHAN_QINWU_OFFSET + KUOZHAN_QINWU_NUM_MAX); i++)
	{
		terminal_group[i].bPortType = PORT_TYPE_QINWU;
		terminal_group[i].wPort = i;
		terminal_group[i].bHookStatus = IDLE;
		terminal_group[i].bPortState = MGW_XIWEI_INIT_STATE;
		terminal_group[i].bPortState1 = 0;
		terminal_group[i].bslot_us = KUOZHAN_QINWU_BASE_SLOT + i - KUOZHAN_QINWU_OFFSET;
		terminal_group[i].bresv =  IDLE;		
		terminal_group[i].user_num_len =  3;
		terminal_group[i].ipaddr = htonl(inet_addr(get_config_var_str(config_cfg,"0")));

		huozhan_phone_num_init(i);
	}
	
	/*Trunk init */
	for(i = KUOZHAN_TRUNK_OFFSET; i < (KUOZHAN_TRUNK_OFFSET + KUOZHAN_TRUNK_NUM_MAX); i++)
	{
		terminal_group[i].bPortType = PORT_TYPE_TRK;
		terminal_group[i].wPort = i;
		terminal_group[i].bHookStatus = IDLE;
		terminal_group[i].bPortState = MGW_TRK_INIT_STATE;
		terminal_group[i].bPortState1 = 0;
		terminal_group[i].bslot_us = KUOZHAN_TRUNK_BASE_SLOT + i - KUOZHAN_TRUNK_OFFSET;
		terminal_group[i].bresv =  IDLE;	
		terminal_group[i].user_num_len =  3;
		//terminal_group[i].fenji = KUOZHAN_FENJI_SLOT_OFFSET + i - KUOZHAN_TRUNK_OFFSET;
		
		huozhan_phone_num_init(i);
	}
	
	/*Phone init */
	for(i = KUOZHAN_PHONE_OFFSET; i < (KUOZHAN_PHONE_OFFSET + KUOZHAN_PHONE_NUM_MAX); i++)
	{
		terminal_group[i].bPortType = PORT_TYPE_KUOZHAN_PHONE;
		terminal_group[i].wPort = i;
		terminal_group[i].bHookStatus = IDLE;
		terminal_group[i].bPortState = MGW_PHONE_INIT_STATE;
		terminal_group[i].bPortState1 = 0;
		terminal_group[i].bslot_us = KUOZHAN_PHONE_BASE_SLOT+ i - KUOZHAN_PHONE_OFFSET;
		terminal_group[i].bresv =  IDLE;
		terminal_group[i].user_num_len =  3;
		
		huozhan_phone_num_init(i);
	}

	/*XiWei init */
	for(i = KUOZHAN_XIWEI_OFFSET; i < (KUOZHAN_XIWEI_OFFSET + KUOZHAN_XIWEI_NUM_MAX); i++)
	{
		terminal_group[i].bPortType = PORT_TYPE_KUOZHAN_XIWEI;
		terminal_group[i].wPort = i;
		terminal_group[i].bHookStatus = IDLE;
		terminal_group[i].bPortState = MGW_XIWEI_INIT_STATE;
		terminal_group[i].bPortState1 = 0;
		terminal_group[i].bslot_us = KUOZHAN_XIWEI_BASE_SLOT +i - KUOZHAN_XIWEI_OFFSET;
		terminal_group[i].bresv =  IDLE;	
		terminal_group[i].user_num_len =  3;	
		
		huozhan_phone_num_init(i);
	}	

	/*adapter init */
	for(i = KUOZHAN_ADATPER_OFFSET; i < (KUOZHAN_ADATPER_OFFSET + KUOZHAN_ADATPER_NUM_MAX); i++)
	{
		terminal_group[i].bPortType = PORT_TYPE_ADP_PHONE;
		terminal_group[i].wPort = i;
		terminal_group[i].bHookStatus = IDLE;
		terminal_group[i].bPortState = MGW_ADP_PHONE_INIT_STATE;
		terminal_group[i].bPortState1 = 0;
		terminal_group[i].bslot_us = KUOZHAN_ADATPER_BASE_SLOT + i - KUOZHAN_ADATPER_OFFSET;
		terminal_group[i].bresv =  IDLE;	
		terminal_group[i].user_num_len =  3;
		
		huozhan_phone_num_init(i);
	}

	huozhan_conf_num_init();
	zhenru_feiji_init();

	return DRV_OK;
}

#if defined(__cplusplus) 
} 
#endif 

#endif // __FSK_C__


