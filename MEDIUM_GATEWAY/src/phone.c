
#ifndef __PHONE_C__
#define __PHONE_C__

#if defined(__cplusplus) 
extern "C" 
{ 
#endif

#include "PUBLIC.h"
#include "phone.h"

static struct sockaddr_in g_AdptDataSockAddr;
static struct sockaddr_in g_AdptVocSockAddr;
static struct sockaddr_in g_AdptMngSockAddr;

static struct sockaddr_in Radio_SeatMngIP;	

int32 g_SwPhoneDatSockFd = 0;
int32 g_SwPhoneVocSockFd = 0;
int32 g_SwPhoneMngSockFd = 0;


int32 Sw_Phone_Data_Send_by_IP(uint8 *buf, int32 len, uint32 ipaddr)
{
	struct sockaddr_in SendIP;

	memset(&SendIP, 0x00, sizeof(SendIP));

	SendIP.sin_family = AF_INET;
	SendIP.sin_port = htons(40011);
	SendIP.sin_addr.s_addr = htonl(ipaddr);

	Socket_Send(g_SwPhoneDatSockFd, &SendIP, buf, len);
	
	return DRV_OK;
}

int32 Sw_Phone_Voc_Send_by_IP(uint8 *buf, int32 len, uint32 ipaddr)
{
	struct sockaddr_in SendIP;

	memset(&SendIP, 0x00, sizeof(SendIP));

	SendIP.sin_family = AF_INET;
	SendIP.sin_port = htons(40012);
	SendIP.sin_addr.s_addr = htonl(ipaddr);

	Socket_Send(g_SwPhoneVocSockFd, &SendIP, buf, len);
	
	return DRV_OK;
}

int32 Sw_Phone_Mng_Send_by_IP(uint8 *buf, int32 len, uint32 ipaddr)
{
	struct sockaddr_in SendIP;

	memset(&SendIP, 0x00, sizeof(SendIP));

	if(0x00 == Radio_SeatMngIP.sin_addr.s_addr)
	{
		return DRV_ERR;
	}	

	SendIP.sin_family = AF_INET;
	SendIP.sin_port = htons(40013);
	SendIP.sin_addr.s_addr = htonl(Radio_SeatMngIP.sin_addr.s_addr);

	Socket_Send(g_SwPhoneMngSockFd, &SendIP, buf, len);
	
	return DRV_OK;
}

int32 Ac491_Voc_Process(uint8 *buf, int32 len, uint8 id)
{
	ST_PHONE_SEM Send;

	memset(&Send, 0x00, sizeof(Send));

	Send.Head.ProType = 0x01;
	Send.Head.SemType = ZJCT_VOC_MSG;
	Send.Head.Len = 160 + 3;
	Send.Data[0] = 0x00;
	Send.Data[1] = terminal_group[id].bslot_us;
	Send.Data[2] = 160;
	memcpy(&Send.Data[3], buf + 12, 160);

	Sw_Phone_Voc_Send_by_IP((uint8 *)&Send, \
		Send.Head.Len +  sizeof(ST_PHONE_SEM_HEAD), terminal_group[id].ipaddr);

	return DRV_OK;
}

int32 Check_834_Sw_Phone_Timeout(const void *data)
{
	uint8 i = 0;
	data = data;

	for(i = 0; i < USER_OPEN_NUM_MAX; i++)
	{
		if(1 == terminal_group[i].sw_phone_flg)
		{
			if((Param_update.timecnt_100ms - terminal_group[i].timecnt_100ms ) > 70)
			{

				printf("sw phone %d  type %s offline\r\n", i, \
						terminal_group[i].sw_phone_834_flg? "834":"28");				

				if((MGW_SW_PHONE_INIT_STATE != terminal_group[i].bPortState)\
					&&(MGW_SW_PHONE_IDLE_STATE != terminal_group[i].bPortState))
				{
					terminal_group[i].bCommand = MC_IDLE;
					terminal_group[i].bHookStatus = HOOK_ON;	
					if(NETWARE_ZXCREATE_NOTOK == terminal_group[i].zhuanxian_flg)
					{
						mgw_scu_release_request(&terminal_group[i]);
					}
					else if(NETWARE_ZXCREATE_OK == terminal_group[i].zhuanxian_flg)
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
					}
				}				
		
				terminal_group[i].sw_phone_flg = 0;
				terminal_group[i].sw_phone_834_flg = 0;
				
				terminal_group[i].bPortType = PORT_TYPE_PHONE;
				terminal_group[i].bPortState = MGW_PHONE_INIT_STATE;
				terminal_group[i].bPortState1 = 0;
				terminal_group[i].ipaddr = 0;
				terminal_group[i].talk_flg = 0;
			}
		}
	}

	return 1;
}

static int32 Sw_Phone_Sem_Process(uint8 *buf, int32 len, uint32 ipaddr)
{
	ST_PHONE_SEM *Rec = (ST_PHONE_SEM *)buf;	

#if 0
	int i = 0; 
	printf("%s: \r\n", __func__);
	for(i = 0; i < len; i++)
	{
		printf("%x:", buf[i]);
	}
	printf("\r\n");
#endif

	switch(Rec->Data[0])
	{
		case EN_TYPE_CALL_REQ:
		{
			int wTemPort;
			wTemPort = find_port_by_simple_ip(ipaddr);

			/* Added by lishibing 20150921，端口4~15，判断有不一致*/
			if((wTemPort < 3) || (wTemPort > USER_OPEN_NUM_MAX))
			{
				VERBOSE_OUT(LOG_SYS,"UnRegister swphone User!\n");

				/*在没有找到网络用户的情况下，应向话音板发送呼叫应答，携带错误状态信息*/
				return DRV_ERR;
			}
			
			terminal_group[wTemPort].bCallType = Rec->Data[3];
			if(terminal_group[wTemPort].bCallType >= MAX_CALL_JIEXU_TYPE)
				terminal_group[wTemPort].bCallType = EN_SEL_CALL;

			bcd2phone((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
				(char *)&Rec->Data[6], Rec->Data[5]);
				 
			terminal_group[wTemPort].phone_queue_g.bFinish = 1;
			terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;
			terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = Rec->Data[5];
		
			terminal_group[wTemPort].bHookStatus = HOOK_OFF;

			if(1 == terminal_group[wTemPort].zhuanxian_flg)
			{
				mgw_sw_phone_connect_request(&terminal_group[wTemPort]);
				terminal_group[wTemPort].talk_flg = 1;
			}
			
			VERBOSE_OUT(LOG_SYS,"Recv Sw Phone %d Call Request!,type:%d, bPortState = 0x%x, bPortState1 = %d\n",wTemPort, \
				terminal_group[wTemPort].bCallType, terminal_group[wTemPort].bPortState,
			terminal_group[wTemPort].bPortState1);			
		}
		break;
		case EN_TYPE_NUM_TRANS:
		{
#if 0		
			int wTemPort ;
			wTemPort = find_port_by_simple_ip(ipaddr);
			if((wTemPort < 3) || (wTemPort > USER_OPEN_NUM_MAX))
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
#endif
			int wTemPort ;
			int wConnectPort;
			wTemPort = find_port_by_simple_ip(ipaddr);
			if((wTemPort < 3) || (wTemPort > USER_OPEN_NUM_MAX))
			{
				VERBOSE_OUT(LOG_SYS,"UnRegister sw phone User!\n");
				return DRV_ERR;
			}
			
			wConnectPort = terminal_group[wTemPort].wConnectPort;
			bcd2phone((char *)terminal_group[wConnectPort].phone_queue_s.bPhoneQueue, \
					(char *)&Rec->Data[5], Rec->Data[4]);
			
			terminal_group[wConnectPort].phone_queue_s.bPhoneQueueHead =  Rec->Data[4];
			terminal_group[wConnectPort].phone_queue_s.bPhoneQueueTail = 0;			
			terminal_group[wConnectPort].phone_queue_s.bFinish = 1;
				
			VERBOSE_OUT(LOG_SYS,"Recv sw phone %d Call  %s!\n",wTemPort, \
				(char *)terminal_group[wConnectPort].phone_queue_s.bPhoneQueue);

		}		
		break;
		case EN_TYPE_CALL_ANS:
		{
			int wTemPort;

			/* Added by lishibing 20150921，端口4~15，判断有不一致*/
			wTemPort = find_port_by_simple_ip(ipaddr);
			if((wTemPort < 3) || (wTemPort > USER_OPEN_NUM_MAX))
			{
				VERBOSE_OUT(LOG_SYS,"UnRegister sw phone User!\n");
				return DRV_ERR;
			}
			VERBOSE_OUT(LOG_SYS,"Recv sw phone %d Call Ans!\n",wTemPort);
			
			terminal_group[wTemPort].bCallType = Rec->Data[3];

			if(Rec->Data[4] == EN_JIEXU_FAILURE)
				terminal_group[wTemPort].bCommand = MC_REJECT;
			else if(Rec->Data[4] == EN_JIEXU_SUCCESS)
				terminal_group[wTemPort].bCommand = MC_RINGING;
			else
				terminal_group[wTemPort].bCommand = MC_REJECT;
			
			mgw_scu_callack_request(&terminal_group[wTemPort]);
		}
		break;
		case EN_TYPE_CONNECT_REQ:
		{
			int wTemPort;

			/* Added by lishibing 20150921，端口4~15，判断有不一致*/
			wTemPort = find_port_by_simple_ip(ipaddr);
			if((wTemPort < 3) || (wTemPort > USER_OPEN_NUM_MAX))
			{
				VERBOSE_OUT(LOG_SYS,"UnRegister sw phone User!\n");
				return DRV_ERR;
			}
			VERBOSE_OUT(LOG_SYS,"Recv sw phone %d Connect request!\n",wTemPort);
#if 0			
			if(((ST_HF_MGW_EXT_CONNECT_REQ*)pro->body)->rtp_port){
				terminal_group[wTemPort].rtp_socket.remote_port = ((ST_HF_MGW_EXT_CONNECT_REQ*)pro->body)->rtp_port;
				terminal_group[wTemPort].rtp_socket.local_port = ((ST_HF_MGW_EXT_CONNECT_REQ*)pro->body)->rtp_port;
			}else{
				terminal_group[wTemPort].rtp_socket.remote_port = get_random_port();
				terminal_group[wTemPort].rtp_socket.local_port = get_random_port();
			}
			VERBOSE_OUT(LOG_SYS,"XIWEI%d port:%d\n",wTemPort,terminal_group[wTemPort].rtp_socket.remote_port);
#endif
			terminal_group[wTemPort].bHookStatus = HOOK_OFF;
			terminal_group[wTemPort].bCallType = Rec->Data[3];
			terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;
			terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = Rec->Data[5];
			bcd2phone((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
				(char *)&Rec->Data[6], Rec->Data[5]);						
		}
		break;
		case EN_TYPE_RELEASE_REQ:
		{
			int wTemPort;

			/* Added by lishibing 20150921，端口4~15，判断有不一致*/
			wTemPort = find_port_by_simple_ip(ipaddr);
			if((wTemPort < 3) || (wTemPort > USER_OPEN_NUM_MAX))
			{
				VERBOSE_OUT(LOG_SYS,"UnRegister sw phone User!\n");
				return DRV_ERR;
			}

			terminal_group[wTemPort].bCommand = MC_HANG_ACTIVE;
			terminal_group[wTemPort].bHookStatus = HOOK_ON;
			/*for fix radio direct call--20120420*/
			//terminal_group[wTemPort].bCallType = ((ST_HF_MGW_EXT_RELEASE_REQ*)pro->body)->call_type;
			terminal_group[wTemPort].rel_reason =  Rec->Data[4];
			VERBOSE_OUT(LOG_SYS,"Recv sw phone %d Release Request!\n",wTemPort);
			
			//mgw_scu_release_request(&terminal_group[wTemPort]);

			if(1 == terminal_group[wTemPort].zhuanxian_flg)
			{
				terminal_group[wTemPort].talk_flg = 0;
			}
		}
		break;
		case EN_TYPE_RELEASE_ANS:
		break;		
		case EN_TYPE_PTT:
		{
			int wTemPort;

			/* Added by lishibing 20150921，端口4~15，判断有不一致*/
			wTemPort = find_port_by_simple_ip(ipaddr);
			if((wTemPort < 3) || (wTemPort > USER_OPEN_NUM_MAX))
			{
				VERBOSE_OUT(LOG_SYS,"UnRegister sw phone User!\n");
				return DRV_ERR;
			}
			if(Rec->Data[3] == PTT_STATE_SEND){
				terminal_group[wTemPort].bPTTStatus = MGW_PTT_DOWN;
				//mgw_scu_ptt_request(&terminal_group[wTemPort]);
			}else if(Rec->Data[3] == PTT_STATE_RECV){
				terminal_group[wTemPort].bPTTStatus = MGW_PTT_UP;
				//mgw_scu_ptt_request(&terminal_group[wTemPort]);
			}
		}
		break;

		default:
		DEBUG_OUT("Unknow call msg id\n");
		break;
	}
			
	return DRV_OK;
}


int32 Sw_Phone_User_Info_Inject(uint8 port_id, uint32 ipaddr)
{
	uint8 timeslot = 0;
	ST_PHONE_SEM Send;
	int32 send_len = 0;
	int i = 0;
	int cnt = 1;	
	int tmp_port = 0;

	timeslot = terminal_group[port_id].bslot_us;

	LOG("%s: \r\n", __func__);		

	memset(&Send, 0xFF, sizeof(Send));

	/* Added by lishibing 20150921，端口4~15，判断有不一致*/
	if((timeslot <= 3)||(timeslot > 15))
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
		cnt = 3;
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
			default:
				break;
		}

		Send.Data[3 + 16 * i] = terminal_group[tmp_port].bslot_us;
		
		Send.Data[4+ 16 * i] = terminal_group[tmp_port].ans;
		Send.Data[5+ 16 * i] = terminal_group[tmp_port].user_num_len;

		memcpy(&Send.Data[6+ 16 * i], terminal_group[tmp_port].user_num,\
		        (terminal_group[tmp_port].user_num_len + 1)/2);

		Send.Data[16+ 16 * i] = 0x00;
		Send.Data[17+ 16 * i] = 0x01;
		Send.Data[18+ 16 * i] = 0x04;	
	}
	

	Send.Head.Len = Send.Data[1] + 2;
	send_len = sizeof(ST_PHONE_SEM_HEAD) + Send.Head.Len;

	Sw_Phone_Data_Send_by_IP((uint8 *)&Send, send_len, ipaddr);	
	
	return DRV_OK;
}

static int32 Sw_Phone_Data_Thread_Process(uint8 *buf, int32 len, uint32 ipaddr)
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
				//Reply.Data[8] = 0x80;	
				//Reply.Data[9] = 0x80;	
				Reply.Data[8] = Rec->Data[8];	

				tmp_port = find_port_by_simple_slot(Rec->Data[9]);
				if(tmp_port < 0)
				{
		                    ERR("%s: port error %d\r\n", __func__, tmp_port);
		                    return DRV_ERR;
				}

				if(0 == terminal_group[tmp_port].sw_phone_flg)
				{
					//printf("port %d , port state %d\r\n", tmp_port, terminal_group[tmp_port].bPortState);
					
					if((PORT_TYPE_PHONE == terminal_group[tmp_port].bPortType)&&((MGW_PHONE_INIT_STATE == terminal_group[tmp_port].bPortState)\
						||(MGW_PHONE_IDLE_STATE == terminal_group[tmp_port].bPortState)))
					{
						terminal_group[tmp_port].bPortType = PORT_TYPE_SW_PHONE;
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
							tmp_port, ipaddr, \
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
						Reply.Data[7] = terminal_group[tmp_port].zhuanxian_flg;
						Reply.Data[9] = 0x00;
						terminal_group[tmp_port].timecnt_100ms = Param_update.timecnt_100ms;
						//Sw_Phone_User_Info_Inject(tmp_port, ipaddr);
					}
					else
					{
						Reply.Data[9] = 0x01;
					}
				}
				
				Sw_Phone_Data_Send_by_IP((uint8 *)&Reply, Reply.Head.Len +  sizeof(ST_PHONE_SEM_HEAD), ipaddr);
			}
			break;
		case ZJCT_PARAM_MSG:
			tmp_port = find_port_by_simple_slot(Rec->Data[3]);
			if(tmp_port < 0)
			{
				ERR("%s: port error %d\r\n", __func__, tmp_port);
				return DRV_ERR;
			}	
			
			if((ipaddr == terminal_group[tmp_port].ipaddr)&&(ZJCT_OPEN_MSG == Rec->Data[0]))
			{
				if(1 == terminal_group[tmp_port].sw_phone_flg)
				{
					Sw_Phone_User_Info_Inject(tmp_port, ipaddr);
				}
			}
			break;
		case ZJCT_SEM_MSG:
			tmp_port = find_port_by_simple_slot(Rec->Data[2]);	
			if(tmp_port < 0)
			{
				ERR("%s: port error %d\r\n", __func__, tmp_port);
				return DRV_ERR;
			}	
			
			if((ipaddr == terminal_group[tmp_port].ipaddr)&&(1 == terminal_group[tmp_port].sw_phone_flg))
			{
				Sw_Phone_Sem_Process(buf, len, ipaddr);
			}			
			break;
		case ZJCT_DATA_MSG:
			break;
		default: 
			break;
	}

	return DRV_OK;
}


static int32 Sw_Phone_Voc_Process(uint8 *buf, uint32 len, uint8 id)
{	
	uint8 sendbuf[RTP_TXBUFF_SIZE];
	struct TRTPHeader *pRTPHeader;	
	
	static uint32 seq[36] = {0};
	static uint32 s_timeslot[36] = {0};
		
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
	pRTPHeader->RTPPT = 0x08; //lishibing ,G711时必须用0x12,PCM必须为0x08
	
	pRTPHeader->RTPSSRC = 0x11111111; //lishibing ,G711时必须用0x23482900;PCM必须为0x11111111.
	pRTPHeader->RTPSeqNum = seq[id];	
	seq[id]++;
	pRTPHeader->RTPTimeStamp = s_timeslot[id];
	s_timeslot[id] += 160;
	pRTPHeader->RTPSeqNum = ntohs(pRTPHeader->RTPSeqNum);		
	pRTPHeader->RTPSSRC = ntohl(pRTPHeader->RTPSSRC);

	if (0 != ac491SendData(id/NUMBER_OF_CHANNELS, id%NUMBER_OF_CHANNELS, sendbuf,  RTP_TXBUFF_SIZE,  PACK_TYPE_RTP))
	{
		ERR("%s: ac491SendData  failed\r\n", __func__);
	}
	
	return DRV_OK;
}


void Sw_Phone_Voc_RxThread(void)
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
			
		if((Rec_Len = recvfrom(g_SwPhoneVocSockFd, buf, MAX_SOCKET_LEN, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{
			wTemPort = find_port_by_simple_ip(remote_addr.sin_addr.s_addr);
			if((wTemPort < 3) || (wTemPort >= USER_OPEN_NUM_MAX))
			{
				VERBOSE_OUT(LOG_SYS,"%s error %d\r\n", __func__, wTemPort);
				continue;
			}

			if(MGW_SW_PHONE_TALK_STATE == terminal_group[wTemPort].bPortState)
			{
				Sw_Phone_Voc_Process(buf + 6, 160, wTemPort);
			}
		}
	}
}

static int32 Sw_Phone_Voc_Socket_Init(void)
{
	struct sockaddr_in my_addr = {0};

	memset(&my_addr, 0x00, sizeof(my_addr));
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(40012);
	my_addr.sin_addr.s_addr = htonl(inet_addr(get_config_var_str(mgw_cfg,"EXTEND_LOCAL_IP")));

	if ((g_SwPhoneVocSockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("%s: create socket\r\n", __func__);
		return DRV_ERR;
	}

	if (DRV_ERR == bind(g_SwPhoneVocSockFd, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("%s: Bind socket failed\r\n", __func__);
		close(g_SwPhoneVocSockFd);
		return DRV_ERR;
	}	

	return DRV_OK;
}

static int32 Sw_Phone_Voc_Socket_Close(void)
{
	if (g_SwPhoneVocSockFd)
	{
		close(g_SwPhoneVocSockFd);
	}
	
	return DRV_OK;
}

void Sw_Phone_Data_RxThread(void)
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
			
		if((Rec_Len = recvfrom(g_SwPhoneDatSockFd, buf, MAX_SOCKET_LEN, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{
			//Dbg_Socket_SendToPc(MONITOR_ADAPTER_I_BIT, buf, Rec_Len);
			
			Sw_Phone_Data_Thread_Process(buf, Rec_Len, remote_addr.sin_addr.s_addr);			
		}
	}
}

static int32 Sw_Phone_Data_Socket_Init(void)
{
	struct sockaddr_in my_addr = {0};

	memset(&my_addr, 0x00, sizeof(my_addr));
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(40011);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((g_SwPhoneDatSockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("%s: create socket\r\n", __func__);
		return DRV_ERR;
	}

	if (DRV_ERR == bind(g_SwPhoneDatSockFd, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("%s: Bind socket failed\r\n", __func__);
		close(g_SwPhoneDatSockFd);
		return DRV_ERR;
	}	

	//setsockopt(g_AdptDatSockFd, SOL_SOCKET, SO_BROADCAST, &broadcast_flag, sizeof(broadcast_flag));

	return DRV_OK;
}

static int32 Sw_Phone_Data_Socket_Close(void)
{
	if (g_SwPhoneDatSockFd)
	{
		close(g_SwPhoneDatSockFd);
	}
	
	return DRV_OK;
}


void Sw_Phone_Mng_RxThread(void)
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
			
		if((Rec_Len = recvfrom(g_SwPhoneMngSockFd, buf, MAX_SOCKET_LEN, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{
			if(WORK_MODE_ZHUANGJIA == Param_update.workmode)
			{
				if (Radio_SeatMngIP.sin_addr.s_addr != remote_addr.sin_addr.s_addr)
				{
					Radio_SeatMngIP.sin_addr.s_addr = remote_addr.sin_addr.s_addr;
				}
				
				Board_Mng_SendTo_716_Radio(buf, Rec_Len);
			}
		}
	}
}

static int32 Sw_Phone_Mng_Socket_Init(void)
{
	struct sockaddr_in my_addr = {0};

	memset(&my_addr, 0x00, sizeof(my_addr));
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(40013);
	my_addr.sin_addr.s_addr = htonl(inet_addr(get_config_var_str(mgw_cfg,"EXTEND_LOCAL_IP")));
	
	if ((g_SwPhoneMngSockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("%s: create socket\r\n", __func__);
		return DRV_ERR;
	}

	if (DRV_ERR == bind(g_SwPhoneMngSockFd, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("%s: Bind socket failed\r\n", __func__);
		close(g_SwPhoneMngSockFd);
		return DRV_ERR;
	}	

	//setsockopt(g_AdptDatSockFd, SOL_SOCKET, SO_BROADCAST, &broadcast_flag, sizeof(broadcast_flag));

	return DRV_OK;
}

static int32 Sw_Phone_Mng_Socket_Close(void)
{
	if (g_SwPhoneMngSockFd)
	{
		close(g_SwPhoneMngSockFd);
	}
	
	return DRV_OK;
}


int32 Sw_Phone_Socket_Close(void)
{
	Sw_Phone_Voc_Socket_Close();
	Sw_Phone_Data_Socket_Close();
	Sw_Phone_Mng_Socket_Close();
	
	return DRV_OK;
}

int32 Sw_Phone_init(void)
{
	memset(&g_AdptDataSockAddr, 0x00, sizeof(g_AdptDataSockAddr));
	memset(&g_AdptVocSockAddr, 0x00, sizeof(g_AdptVocSockAddr));	
	memset(&g_AdptMngSockAddr, 0x00, sizeof(g_AdptMngSockAddr));		

	if(DRV_OK != Sw_Phone_Data_Socket_Init())
	{
		ERR("Sw_Phone_Data_Socket_Init failed \r\n");		
		return DRV_ERR;		
	}

	if(DRV_OK != Sw_Phone_Voc_Socket_Init())
	{
		ERR("Sw_Phone_Voc_Socket_Init failed \r\n");		
		return DRV_ERR;		
	}

	if(DRV_OK != Sw_Phone_Mng_Socket_Init())
	{
		ERR("Sw_Phone_Mng_Socket_Init failed \r\n");		
		return DRV_ERR;		
	}	
	
	return DRV_OK;
}


#if defined(__cplusplus) 
} 
#endif 

#endif // __FSK_C__


