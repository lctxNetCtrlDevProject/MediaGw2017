/*
===========================================================
** Description: Э��Ľ�����ת�����ɰ�
===========================================================
*/
#include "PUBLIC.h"


extern UDP_SOCKET gIpcSemSocket;
extern int32 time_cnt_100ms;

#define	MAXNETMASK	4

#define HF_MGW_SCU_IPC_SEM_SLOT_OFFSET 7

/* Modified by lishibing 20140723, Z043 */
/* ý�����ذ���ձ��Ĺ��ˣ�ֻ�ܽ�������Ϊ8,24,24 */
const unsigned long ADDRMASK[MAXNETMASK] = {0xff000000,0xffffff00,0xffffff00,0xffffff00};

/* ý�����ذ���ձ���IP���ˣ�ֻ�ܽ�������Ϊ10.x.x.x,192.168.1.x,192.168.254.x�� udp����*/
const unsigned long ADDRINDEX[MAXNETMASK] = {0x0a000000,0xc0a80100,0xc0a8FE00, 0xc0a80900};

const unsigned char	JSCOST[4] = {EN_TYPE_JS_COST_FIBER,EN_TYPE_JS_COST_JUNZONG,EN_TYPE_JS_COST_WEIBO,EN_TYPE_JS_COST_SATELLITIC};

const unsigned char DIRECTCALL_RADIO_INDEX[] = {0x81,0x82,0x83,0x84,0x86,0x87,0x88,0x89};

/* Added by lishibing 20140723 , Z043 */
uint32 g_uiShakeHandFlg = HF_MGW_SCU_LINK_STATUS_FAILED;

/* 50��������Ϣ15���ֽ�ͷ����2���ֽڳ����ֶ� */
unsigned char herader50[MSG_HEADER_LEN + 2] = 
{
	0x00,0x01,0x0f,0x80,0x00,0x04,0x0a,0x00,0x00,0x06,0x01,0x0a,0x00,0x00,0x01,
	0x00,0x00
};

int32 Socket_Send(int32 bsockfd,struct sockaddr_in *pt, uint8 *ptr,int32 len)
{	
	struct sockaddr_in *des_addr_p;
	uint8 *p;
	int32 ret;
	
	if ((0 == bsockfd)||(NULL == pt)||(NULL == ptr)||(len == 0))
	{
		printf("Point is NULL.fd = %d, len = %d\r\n", bsockfd, len);
		return DRV_ERR;
	}	
	
	des_addr_p = pt;
	p = ptr;
	ret = sendto(bsockfd, p, len, 0, (struct sockaddr *)des_addr_p, sizeof(struct sockaddr_in));

	if (DRV_ERR == ret)
	{
		printf("%s_%d ,send Fail, errno=%d\r\n",__func__,__LINE__,errno);
		printf("send socket udp error(%s) port %d!\n",inet_ntoa(des_addr_p->sin_addr), des_addr_p->sin_port);		
		//printf("send socket udp error!\n");	
		
		return DRV_ERR;
	}

	//printf("send to %s, len = %d\r\n", inet_ntoa(des_addr_p->sin_addr), len);

	return DRV_OK;
}

int32 Board_Sem_SendTo_Inc(uint8 *buf, int32 len)
{
#if 0
	int i = 0;

	printf("send to 50: ");
	for(i = 0; i < len; i++)
	{
		printf("%x:", buf[i]);
	}
	printf("\r\n");
#endif

	Socket_Send(gIpcSemSocket.udp_handle, (struct sockaddr_in*)&gIpcSemSocket.addr_them, buf, len);
		
	Dbg_Socket_SendToPc(MONITOR_I_50_SEM_BIT, buf, len);
	
	return DRV_OK;
}

int32 Board_Data_SendTo_Inc_Addr(uint8 *buf, int32 len, 	struct sockaddr_in addr)
{
	int i = 0;
	printf("%s: \r\n", __func__);
	for(i = 0;i< len; i++)
	{
		printf("%x:", buf[i]);
	}
	printf("\r\n");
	
	 Socket_Send(gIpcDatSocket.udp_handle, &addr, buf, len);
		
	return DRV_OK;
}

int32 Board_Data_SendTo_Inc(uint8 *buf, int32 len)
{
	Socket_Send(gIpcDatSocket.udp_handle, (struct sockaddr_in*)&gIpcDatSocket.addr_them, buf, len);
	
	return DRV_OK;
}

int Send_XIWEI_RTP_Packet(int channel,char * Buffer,size_t len)
{
	Socket_Send(terminal_group[channel].rtp_socket.udp_handle, \
		(struct sockaddr_in*)&terminal_group[channel].rtp_socket.addr_them, (uint8 *)Buffer, len);

	return DRV_OK;
}

int32 Hf_SeatSem_Socket_Send(uint8 *buf, uint32 len)
{
	int32 retval = 0;

	
	buf[9] += 12;
	
	retval = Socket_Send(gRpcSeatSocket, &Terminal_I_IP, buf, len);
					
	return DRV_OK;
}

int32 Hf_IVoc_Socket_Send(uint8 *buf, uint32 len)
{
	int32 retval = 0;
	
	retval = Socket_Send(gRpcVoiceSocket, &Terminal_I_IPV, buf, len);
					
	return DRV_OK;
}


int32 Dbg_Socket_Send(uint8 *buf, int32 len)
{
	int32 retval = 0;

	retval = Socket_Send(gRpcDbgSocket, &SocketT_DbgIP, buf, len);
	if(DRV_OK != retval)
	{
		//printf("%s failed \r\n", __func__);
		printf("failed\n");
		return DRV_ERR;
	}

	return DRV_OK;
}


int mgw_check_MsgLen(unsigned short sMsgBodyLen, unsigned short sMsgDefLen)
{
	if (sMsgBodyLen != sMsgDefLen)
	{
		VERBOSE_OUT(LOG_SYS,"Invalid msg length,msg body len %d,define len %d!\n", 
					sMsgBodyLen, sMsgDefLen);
		return DRV_ERR;
	}

	return DRV_OK;
}

int string_cmp(const char *s1, const char *s2)
{
	return  strcmp(s1, s2);
}

int string_ncmp(const char *s1, const char *s2, int count)
{
	return  strncmp(s1, s2, count);
}

char *string_dup(const char *s1)
{
	return  strdup(s1);
}

int	find_net(struct sockaddr_in * addr)
{
	int i,net_no;
	for(i=0;i<MAXNETMASK;i++)
	{
		net_no = addr->sin_addr.s_addr & ADDRMASK[i];
		if(net_no == ADDRINDEX[i])
			return i;
	}
	return IDLEW;
}

unsigned char mgw_fetch_mask(unsigned long mask)
{
	int i;
	unsigned long tmp_mask;
	for(i=0;i<32;i++){
		tmp_mask = htonl(1<<(31-i));
		if(!(mask & tmp_mask))
			return i;
	}
	return i;
}

unsigned long mgw_equal_if(unsigned long ip_addr)
{
	unsigned long tmp_ip_addr = 0;
	unsigned long tmp_ip_mask;
	char if_name[16]="";
	int i;

	if(!ip_addr || ip_addr == 0xffffffff)
	{
		return DRV_ERR;
	}
	
	for(i=0;i<5;i++)
	{
		if(i==0)
			//strcpy(if_name,"eth0:1");
            strncpy((char *)if_name,"eth0:1", sizeof(if_name));
		else if(i==1)
			//strcpy(if_name,"eth1");
             strncpy((char *)((char *)if_name),"eth1", sizeof(if_name));
		else
		{
			snprintf(if_name, sizeof(if_name), "eth1:%d",i-1);
		}
		tmp_ip_addr = mgw_if_fetch_ip(if_name);
		tmp_ip_mask = mgw_if_fetch_mask(if_name);
		if(ip_addr == tmp_ip_addr)
			return (i+1);
	}
	return DRV_OK;
}

int mgw_test_ip(unsigned long ip_addr)
{
	unsigned long tmp_ip_addr = 0;
	unsigned long tmp_ip_mask;
	unsigned long tmp_ip_broadcast;
	char if_name[16]="";
	
	strncpy((char *)if_name,"eth0:1", sizeof(if_name));
		
	tmp_ip_addr = mgw_if_fetch_ip(if_name);
	tmp_ip_mask = mgw_if_fetch_mask(if_name);
	tmp_ip_broadcast = mgw_if_fetch_boardaddr(if_name);

	if(tmp_ip_broadcast == ip_addr)
		return DRV_ERR;
	
	if((ip_addr&tmp_ip_mask) != (tmp_ip_addr&tmp_ip_mask))
		return DRV_ERR;
	
	return DRV_OK;

}

char * mgw_find_if(unsigned long ip_addr)
{
	unsigned long tmp_ip_addr = 0;
	unsigned long tmp_ip_mask;
	static char if_name[16]="";
	int i;
	
	for(i=0;i<4;i++)
	{
		if(i==0)
		{
			//strcpy(if_name,"eth1");
			strncpy((char *)if_name,"eth1", sizeof(if_name));
        }
		else
		{
			//sprintf(if_name,"eth1:%d",i);
			snprintf((char *)if_name, sizeof(if_name), "eth1:%d",i);
		}
		
		tmp_ip_addr = mgw_if_fetch_ip(if_name);
		tmp_ip_mask = mgw_if_fetch_mask(if_name);
		if((ip_addr & tmp_ip_mask) == (tmp_ip_addr & tmp_ip_mask))
			return if_name;
	}

	//strcpy(if_name,"");
	strncpy((char *)if_name,"", sizeof(if_name));
	
	return if_name;
}

struct in_addr mgw_cal_net(struct in_addr addr,int logmask)
{
	struct in_addr tmp_in_addr;
	tmp_in_addr.s_addr = 0;
	if (logmask){
		tmp_in_addr.s_addr = addr.s_addr & htonl(~((1<<(32-logmask))-1));
	}
	return tmp_in_addr;
}

int find_tunnel_type_by_cost(unsigned char cost)
{
	int i;
	for(i=0;i<sizeof(JSCOST);i++)
		if(cost == JSCOST[i])
			return (i+1);
	return 1;
}

//inline void phone2bcd(char *dest,const char *src,size_t count)
void phone2bcd(char *dest, char *src,size_t count)
{
	size_t valid_len = strlen(src);
	size_t i;

	if(!src)
		return;

	if(strlen(src) > count*2)
	{
		VERBOSE_OUT(LOG_SYS,"Format_PHONENUM Param ERROR!\n");
		return;
	}

	for(i = 0;i < valid_len; i++)
	{
		if(i%2)
		{
			*dest++ += ((*src - '0')<< 4);
			src++;
		}
		else
		{
			*dest = *src - '0';
			src++;
		}
	}
	
	if(i%2)
	{
		*dest += 0xf0;
		dest++;
	}
	
	i++;
	i >>= 1;
	
	for(; i < count; i++)
	{
		*dest++ = 0xff;
	}
	
	return;
}


int bcd2phone(char *dest, char *src, size_t count)
{
	size_t i;

	if(!src)
		return -1;

	for(i = 0;i < count; i++)
	{
		if(i%2)
		{
			*dest = ((*src & 0xF0) >> 4) + '0';
			src++;
		}
		else
		{
			*dest = (*src&0x0f) + '0';
		}
		
		if(*dest > '9')
		{
			break;
		}
		
		dest++;
	}

	*dest = '\0';
	
	return i;
}

int32 Sip_BoardPhToSipPh(char *pfrom, int32 len, char *pout)
{
	uint8 i = 0;
	uint8 bDat = 0;
	uint8 bNumCt = 0;
	

	for (i=0; i< len; i++)
	{
		bDat = *(pfrom+i);
		if ((bDat & 0x0f) != 0x0f)
		{
			if (bNumCt < (MAXNUMLEN-1))
			{
				*pout++ = (bDat & 0x0f) + '0';
				bNumCt++;
			}
			else
			{
				*pout++ = '\0';		
				break;
			}
		}
		else
		{
			*pout++ = '\0';		
			break;
		}

		bDat >>= 4;
		if ((bDat & 0x0f) != 0x0f)
		{
			if (bNumCt < (MAXNUMLEN-1))
			{
				*pout++ = (bDat & 0x0f) + '0';
				bNumCt++;
			}
			else
			{
				*pout++ = '\0';			
				break;
			}
		}
		else
		{
			*pout++ = '\0';		
			break;
		}
	}

	return bNumCt;
}


unsigned short cal_checksum(BYTE* src,size_t count)
{
	DWORD	checksum = 0;
	while(count--)
		checksum += (*src++);
	return checksum & 0xffff;
}

int32 Check_Control_Board_Timeout(const void *data)
{
	if(FALSE == Param_update.control_online)
	{
		return 1;
	}

	if((time_cnt_100ms - Param_update.control_timecnt) > 120)
	{
		LOG("control_online board offline timecnt %d, %d\r\n", \
			time_cnt_100ms, Param_update.control_timecnt);
		
		Param_update.control_online  = FALSE;

		if(FALSE == Param_update.ac491_init)
		{
			Ac491_Init();
		}
	}	

	return 1;
}

/*=============================���п���==================================*/
int mgw_ext_call_notify(TERMINAL_BASE* terext)
{
	unsigned short tmp_len;
	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_CALL_REQ call_req;
	
	/*1.Packet user call req*/
	memset(&st_hf_mgw_ext_pro, 0x00, sizeof(st_hf_mgw_ext_pro));
	memset(&call_req, 0xFF, sizeof(call_req));		
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
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;

	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*����У��������ֽ�*/

	/*3.Sent to Host*/
	ext_udp_socket.addr_them.sin_addr.s_addr= inet_addr(terext->rtp_socket.remote_ip);
	inc_log(LOG_DEBUG,"send to %s:%d call request!\n",terext->rtp_socket.remote_ip, \
		ext_udp_socket.addr_them.sin_port);

	return sendto(ext_udp_socket.udp_handle,&st_hf_mgw_ext_pro,tmp_len, \
		0,(struct sockaddr*)&ext_udp_socket.addr_them,ext_udp_socket.addr_len);
}

int mgw_ext_callack_notify(TERMINAL_BASE* terext)
{
	unsigned short tmp_len;

	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_CALLACK_REQ callack_req;
	/*1.Packet user callack req*/
	memset(&st_hf_mgw_ext_pro, 0x00, sizeof(st_hf_mgw_ext_pro));
	memset(&callack_req, 0x00, sizeof(callack_req));		
	callack_req.id = EN_TYPE_CALL_ANS;
	callack_req.msg_len = sizeof(ST_HF_MGW_EXT_CALLACK_REQ)-2;
	callack_req.slot = terext->bslot_us;
	callack_req.call_type = terext->bCallType;
	if(terext->bCommand == MC_RT_TONE)
		callack_req.call_statu = EN_JIEXU_SUCCESS;
	else if(terext->bCommand == MC_REJECT)
		callack_req.call_statu = EN_JIEXU_FAILURE;
	else if(terext->bCommand == MC_TALK)
		callack_req.call_statu = EN_JIEXU_SUCCESS;

	
	/*2.Packet MGW_EXT_PRO*/
	st_hf_mgw_ext_pro.header.sync_head = ntohs(0x55aa);
	st_hf_mgw_ext_pro.header.type = BAOWEN_TYPE_CALLCMD;
	st_hf_mgw_ext_pro.header.reserve = ntohs(0x0000);
	tmp_len = sizeof(ST_HF_MGW_EXT_CALLACK_REQ);
	st_hf_mgw_ext_pro.header.len = ntohs(tmp_len);
	memcpy(st_hf_mgw_ext_pro.body,&callack_req,tmp_len);
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;

	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*����У��������ֽ�*/

	/*3.Sent to Host*/
	ext_udp_socket.addr_them.sin_addr.s_addr= inet_addr(terext->rtp_socket.remote_ip);
	return sendto(ext_udp_socket.udp_handle,&st_hf_mgw_ext_pro,tmp_len, \
		0,(struct sockaddr*)&ext_udp_socket.addr_them,ext_udp_socket.addr_len);

}

int mgw_ext_connect_notify(TERMINAL_BASE* terext)
{
	unsigned short tmp_len;

	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_CONNECT_REQ connect_req;
	
	/*1.Packet user connect req*/
	memset(&st_hf_mgw_ext_pro, 0x00, sizeof(st_hf_mgw_ext_pro));
	memset(&connect_req, 0xFF, sizeof(connect_req));	
	connect_req.id = EN_TYPE_CONNECT_REQ;
	connect_req.msg_len = sizeof(ST_HF_MGW_EXT_CONNECT_REQ)-2;
	connect_req.slot = terext->bslot_us;
	connect_req.encoder = ENCODER_G711;
	connect_req.call_type = terext->bCallType;
	connect_req.rtp_port = get_random_port();
	terext->rtp_socket.local_port = connect_req.rtp_port;
	connect_req.phone_len = terext->phone_queue_g.bPhoneQueueHead;
	phone2bcd((char *)connect_req.caller_num, \
		(char *)terext->phone_queue_g.bPhoneQueue, connect_req.phone_len);

	/*2.Packet MGW_EXT_PRO*/
	st_hf_mgw_ext_pro.header.sync_head = ntohs(0x55aa);
	st_hf_mgw_ext_pro.header.type = BAOWEN_TYPE_CALLCMD;
	st_hf_mgw_ext_pro.header.reserve = ntohs(0x0000);
	tmp_len = sizeof(ST_HF_MGW_EXT_CONNECT_REQ);
	st_hf_mgw_ext_pro.header.len = ntohs(tmp_len);
	memcpy(st_hf_mgw_ext_pro.body,&connect_req,tmp_len);
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;

	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*����У��������ֽ�*/

	/*4.Sent to Host*/
	ext_udp_socket.addr_them.sin_addr.s_addr= inet_addr(terext->rtp_socket.remote_ip);
	return sendto(ext_udp_socket.udp_handle,&st_hf_mgw_ext_pro,tmp_len, \
		0,(struct sockaddr*)&ext_udp_socket.addr_them,ext_udp_socket.addr_len);


}

int mgw_ext_release_notify(TERMINAL_BASE* terext)
{
	unsigned short tmp_len;

	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_RELEASE_REQ release_req;
	
	/*1.Packet user connect req*/
	memset(&st_hf_mgw_ext_pro, 0x00, sizeof(st_hf_mgw_ext_pro));
	memset(&release_req, 0x00, sizeof(release_req));	
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
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;

	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*����У��������ֽ�*/

	/*3.Sent to Host*/
	ext_udp_socket.addr_them.sin_addr.s_addr= inet_addr(terext->rtp_socket.remote_ip);
	return sendto(ext_udp_socket.udp_handle,&st_hf_mgw_ext_pro,tmp_len, \
		0,(struct sockaddr*)&ext_udp_socket.addr_them,ext_udp_socket.addr_len);
}

int mgw_ext_release_cnf_notify(TERMINAL_BASE* terext)
{
	unsigned short tmp_len;

	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_RELEASE_CNF release_cnf;
	
	/*1.Packet user connect req*/
	memset(&st_hf_mgw_ext_pro, 0x00, sizeof(st_hf_mgw_ext_pro));
	memset(&release_cnf, 0x00, sizeof(release_cnf));
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
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;

	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*����У��������ֽ�*/

	/*3.Sent to Host*/
	ext_udp_socket.addr_them.sin_addr.s_addr= inet_addr(terext->rtp_socket.remote_ip);
	return sendto(ext_udp_socket.udp_handle,&st_hf_mgw_ext_pro,tmp_len, \
		0,(struct sockaddr*)&ext_udp_socket.addr_them,ext_udp_socket.addr_len);
}


int mgw_ext_buzzer_ctl(TERMINAL_BASE* terext)
{
	unsigned short tmp_len;
	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_BUZZER_CTL buzzer_ctl;
	memset(&st_hf_mgw_ext_pro, 0x00, sizeof(st_hf_mgw_ext_pro));
	memset(&buzzer_ctl, 0x00, sizeof(buzzer_ctl));	
	
	buzzer_ctl.id = EN_TYPE_BUZZER_CTL;
	buzzer_ctl.msg_len = sizeof(ST_HF_MGW_EXT_BUZZER_CTL)-2;
	buzzer_ctl.slot = terext->bslot_us;
	buzzer_ctl.buzzer_status = terext->buzzer_status;
	st_hf_mgw_ext_pro.header.sync_head = ntohs(0x55aa);
	st_hf_mgw_ext_pro.header.type = BAOWEN_TYPE_CALLCMD;
	st_hf_mgw_ext_pro.header.reserve = ntohs(0x0000);
	tmp_len = sizeof(ST_HF_MGW_EXT_BUZZER_CTL);
	st_hf_mgw_ext_pro.header.len = ntohs(tmp_len);
	memcpy(st_hf_mgw_ext_pro.body,&buzzer_ctl,tmp_len);
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;
	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*����У��������ֽ�*/
	ext_udp_socket.addr_them.sin_addr.s_addr= inet_addr(terext->rtp_socket.remote_ip);
	return sendto(ext_udp_socket.udp_handle,&st_hf_mgw_ext_pro,tmp_len, \
		0,(struct sockaddr*)&ext_udp_socket.addr_them,ext_udp_socket.addr_len);
}
int mgw_ext_assign_user_number(const void * data)
{
	/*assign user number to terminal*/
	unsigned short tmp_len;
	int i;

	TERMINAL_BASE * terext = (TERMINAL_BASE *)data;

	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_USER_NUM_REQ assign_user_num_req;

	/*Packet assign_user_num request*/
	memset(&st_hf_mgw_ext_pro, 0x00, sizeof(st_hf_mgw_ext_pro));
	memset(&assign_user_num_req, 0x00, sizeof(assign_user_num_req));	
	
	assign_user_num_req.id = EN_TYPE_USER_NUM_REQ;
	assign_user_num_req.mem_count = 0;
	assign_user_num_req.op_code = 1;	/*������ʽ:0--ɾ��,1--����*/
	assign_user_num_req.msg_len = 2;


	for(i=0;i<SIP_OFFSET;i++)
	{
		if(terminal_group[i].bPortType == PORT_TYPE_CISHI || \
			terminal_group[i].bPortType == PORT_TYPE_XIWEI || \
			terminal_group[i].bPortType == PORT_TYPE_TRK)
		{
			assign_user_num_req.terminal[assign_user_num_req.mem_count].mem_slot = terminal_group[i].bslot_us;
			phone2bcd((char *)assign_user_num_req.terminal[assign_user_num_req.mem_count].callernum, \
				(char *)terminal_group[i].name,MAXPHONENUMBYTE);
			assign_user_num_req.terminal[assign_user_num_req.mem_count].conf = terext->conference_enable;
			assign_user_num_req.terminal[assign_user_num_req.mem_count].prior = terext->priority;
			assign_user_num_req.terminal[assign_user_num_req.mem_count].encoder = ENCODER_G711;
			assign_user_num_req.mem_count++;
			assign_user_num_req.msg_len += sizeof(assign_user_num_req.terminal[0]);
		}
	}


	/*2.Packet MGW_EXT_PRO*/
	st_hf_mgw_ext_pro.header.sync_head = ntohs(0x55aa);
	st_hf_mgw_ext_pro.header.type = BAOWEN_TYPE_CALLCFG;
	st_hf_mgw_ext_pro.header.reserve = ntohs(0x0000);
	tmp_len = assign_user_num_req.msg_len + 2;
	st_hf_mgw_ext_pro.header.len = ntohs(tmp_len);
	memcpy(st_hf_mgw_ext_pro.body,&assign_user_num_req,tmp_len);
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;

	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*����У��������ֽ�*/

	/*3.Sent to Host*/
	ext_udp_socket.addr_them.sin_addr.s_addr= inet_addr(terext->rtp_socket.remote_ip);
	
	sendto(ext_udp_socket.udp_handle,(char*)&st_hf_mgw_ext_pro,tmp_len, \
		0,(struct sockaddr*)&ext_udp_socket.addr_them,ext_udp_socket.addr_len);	

	return 1;
}

/*====================================mgw_scu==========================================*/
int mgw_sw_phone_call_request(TERMINAL_BASE* terext)
{
	ST_HF_MGW_PHONE_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_CALL_REQ call_req;
	int tmp_len;
	char *tmp_dup_str;

	printf("%s terext->bslot_us = %d, %d\r\n", __func__, terext->bslot_us, terext->bCallType);

	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));
	memset(&call_req, 0xFF, sizeof(call_req));	
	
	/*1.packet call request*/
	call_req.id = MGW_SCU_CALL_ID;
	call_req.len = sizeof(ST_HF_MGW_SCU_CALL_REQ) - 2;
	call_req.slot = terext->bslot_us;
	call_req.call_type = terext->bCallType;
	
	tmp_dup_str = strdup((char *)terext->phone_queue_g.bPhoneQueue);
	if(terext->bCallType == EN_CALL_TRUNK)
	{
		call_req.call_type = EN_SEL_CALL;
	}else if(terext->bCallType == EN_FORCE_CALL){/*ǿ��*/
		call_req.call_type = EN_SEL_CALL;
		terext->phone_queue_g.bPhoneQueueHead += 4;
		//sprintf(terext->phone_queue_g.bPhoneQueue,"8012%s",tmp_dup_str);
		snprintf((char *)terext->phone_queue_g.bPhoneQueue, MAXPHONENUM, "8012%s",tmp_dup_str);
	}else if(terext->bCallType == EN_FORCE_RELEASE_CALL){/*ǿ��*/
		call_req.call_type = EN_SEL_CALL;
		terext->phone_queue_g.bPhoneQueueHead += 4;
		snprintf((char *)terext->phone_queue_g.bPhoneQueue, MAXPHONENUM, "8013%s",tmp_dup_str);
	}else if(terext->bCallType == EN_CALL_Z){
		call_req.call_type = EN_SEL_CALL;
		call_req.slot = terext->bresv;
	}else if(terext->bCallType == EN_CALL_RADIO){
		call_req.call_type = EN_SEL_CALL;
		call_req.slot = terext->bresv;		
	}
	else 
	{
		call_req.call_type = EN_SEL_CALL;	
	}
	
	free(tmp_dup_str);
	
	call_req.encoder = 0x04;	/*Ĭ�ϱ����ʽ*/
	call_req.phone_len = terext->phone_queue_g.bPhoneQueueHead;
	phone2bcd((char *)call_req.callee_num,\
		(char *)terext->phone_queue_g.bPhoneQueue,call_req.phone_len);

	tmp_len = sizeof(ST_HF_MGW_SCU_CALL_REQ);

	/*2.packet complete protocol*/
	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_CALL_REQ);
	memcpy(st_hf_mgw_scu_pro.body,&call_req,sizeof(ST_HF_MGW_SCU_CALL_REQ));

	tmp_len += sizeof(ST_HF_MGW_PHONE_PRO_HEADER);

	/*3.send packet*/
	return Sw_Phone_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}


int mgw_sw_phone_callack_request(TERMINAL_BASE* terext)
{
	ST_HF_MGW_PHONE_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_CALLACK_REQ callack_req;
	int tmp_len;

	printf("%s terext->bslot_us = %d\r\n", __func__, terext->bslot_us);

	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));
	memset(&callack_req, 0x00, sizeof(callack_req));
	
	callack_req.id = MGW_SCU_CALLACK_ID;
	callack_req.len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ) - 2;
	callack_req.slot = terext->bslot_us;
	callack_req.call_type = terext->bCallType;
	
	if(terext->bCallType == EN_CALL_TRUNK)
	{
		callack_req.call_type = EN_SEL_CALL;	
	}
	
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
	//return Board_Sem_SendTo_Inc((uint8*)&st_hf_mgw_scu_pro, tmp_len);
	return Sw_Phone_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}

int mgw_sw_phone_connect_request(TERMINAL_BASE* terext)
{
	ST_HF_MGW_PHONE_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_CONNECT_REQ connect_req;
	int tmp_len;

	printf("%s terext->bslot_us = %d\r\n", __func__, terext->bslot_us);

	memset(&connect_req, 0xFF, sizeof(connect_req));
	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));
	
	connect_req.id = MGW_SCU_CONNECT_ID;
	connect_req.len = sizeof(ST_HF_MGW_SCU_CONNECT_REQ) - 2;
	connect_req.slot = terext->bslot_us;
	connect_req.call_type = terext->bCallType;

	if(terext->bCallType == EN_CALL_TRUNK)
		connect_req.call_type = EN_SEL_CALL;

	connect_req.encoder = 0x04;
	connect_req.phone_len = terext->phone_queue_g.bPhoneQueueHead;
	phone2bcd((char *)connect_req.caller_num,\
		(char *)terext->phone_queue_g.bPhoneQueue,connect_req.phone_len);

	tmp_len = sizeof(ST_HF_MGW_SCU_CONNECT_REQ);
	
	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_CONNECT_REQ);
	memcpy(st_hf_mgw_scu_pro.body,&connect_req,sizeof(ST_HF_MGW_SCU_CONNECT_REQ));

	tmp_len += sizeof(ST_HF_MGW_PHONE_PRO_HEADER);
	
	/*3.send packet*/
	//return Board_Sem_SendTo_Inc((uint8*)&st_hf_mgw_scu_pro, tmp_len);
	return Sw_Phone_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}

int mgw_sw_phone_release_request(TERMINAL_BASE* terext)
{
	ST_HF_MGW_PHONE_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_RELEASE_REQ release_req;
	int tmp_len;

	/**/
	printf("%s terext->bslot_us = %d\r\n", __func__, terext->bslot_us);

	memset(&release_req, 0x00, sizeof(release_req));
	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));
	
	release_req.id = MGW_SCU_RELEASE_ID;
	release_req.len = sizeof(ST_HF_MGW_SCU_RELEASE_REQ)-2;
	release_req.slot = terext->bslot_us;
	release_req.call_type = terext->bCallType;

	if(terext->bCallType == EN_CALL_TRUNK)
		release_req.call_type = EN_SEL_CALL;
	else if(terext->bCallType == EN_CALL_Z){
		/*fix radio direct call*/
		release_req.call_type = EN_SEL_CALL;
		release_req.slot = terext->bresv;
	}else if(terext->bCallType == EN_CALL_RADIO){
		/*fix radio direct call*/
		release_req.call_type = EN_SEL_CALL;
		release_req.slot = terext->bresv;
	}
	
	release_req.rel_reason = terext->rel_reason;

	tmp_len = sizeof(ST_HF_MGW_SCU_RELEASE_REQ);
	
	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_RELEASE_REQ);
	memcpy(st_hf_mgw_scu_pro.body,&release_req,sizeof(ST_HF_MGW_SCU_RELEASE_REQ));

	tmp_len += sizeof(ST_HF_MGW_PHONE_PRO_HEADER);

	/*3.send packet*/
	//return Board_Sem_SendTo_Inc((uint8*)&st_hf_mgw_scu_pro, tmp_len);
	return Sw_Phone_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}

int mgw_sw_phone_release_cnf_request(TERMINAL_BASE* terext)
{
	ST_HF_MGW_PHONE_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_RELEASE_CNF_REQ release_cnf_req;
	int tmp_len;

	printf("%s terext->bslot_us = %d\r\n", __func__, terext->bslot_us);

	memset(&release_cnf_req, 0x00, sizeof(release_cnf_req));
	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));
	
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

	//return Board_Sem_SendTo_Inc((uint8*)&st_hf_mgw_scu_pro, tmp_len);
	return Sw_Phone_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}

int mgw_sw_phone_ptt_request(TERMINAL_BASE * terext)
{
	ST_HF_MGW_PHONE_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_PTT_MSG ptt_msg;
	int tmp_len;

    memset(&ptt_msg, 0x00, sizeof(ptt_msg));
	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro)); 
    
	ptt_msg.id = MGW_SCU_PTT_ID;
	ptt_msg.len = 2;
	ptt_msg.slot = terext->bslot_us;

	/*fix radio hf direct talk 2012-04-13*/
	if(terext->bCallType == EN_CALL_Z)
		ptt_msg.slot = terext->bresv;

	if(terext->bCallType == EN_CALL_RADIO)
		ptt_msg.slot = terext->bresv;

	phone2bcd((char *)ptt_msg.peer_num, \
		(char *)terext->name, MAXPHONENUMBYTE);
	if(terext->bPTTStatus == MGW_PTT_DOWN)
		ptt_msg.ptt_status = PTT_STATE_SEND;
	else if(terext->bPTTStatus == MGW_PTT_UP)
		ptt_msg.ptt_status = PTT_STATE_RECV;

	tmp_len = 4;

	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = tmp_len;
	memcpy(st_hf_mgw_scu_pro.body,&ptt_msg, tmp_len);

	tmp_len += sizeof(ST_HF_MGW_PHONE_PRO_HEADER);

	return Sw_Phone_Data_Send_by_IP((uint8 *)&st_hf_mgw_scu_pro ,tmp_len, terext->ipaddr);
}

/*=============================���п���==================================*/
int mgw_local_seat_call_request(TERMINAL_BASE* terext)
{
	unsigned short tmp_len;
	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_CALL_REQ call_req;
	
	/*1.Packet user call req*/
    memset(&call_req, 0xFF, sizeof(call_req));
	memset(&st_hf_mgw_ext_pro, 0x00, sizeof(st_hf_mgw_ext_pro));
	
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
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;

	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*����У��������ֽ�*/

#if 0
	/*3.Sent to Host*/
	ext_udp_socket.addr_them.sin_addr.s_addr= inet_addr(terext->rtp_socket.remote_ip);
	inc_log(LOG_DEBUG,"send to %s:%d call request!\n",terext->rtp_socket.remote_ip, \
		ext_udp_socket.addr_them.sin_port);

	return sendto(ext_udp_socket.udp_handle,&st_hf_mgw_ext_pro,tmp_len, \
		0,(struct sockaddr*)&ext_udp_socket.addr_them,ext_udp_socket.addr_len);
#endif

	return Hf_SeatSem_Socket_Send((uint8 *)&st_hf_mgw_ext_pro, tmp_len);

}

int mgw_local_seat_callack_request(TERMINAL_BASE* terext)
{
	unsigned short tmp_len;

	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_CALLACK_REQ callack_req;
	
	/*1.Packet user callack req*/
    memset(&callack_req, 0x00, sizeof(callack_req));
	memset(&st_hf_mgw_ext_pro, 0x00, sizeof(st_hf_mgw_ext_pro));	
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
	else if(terext->bCommand == MC_RING)
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
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;

	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*����У��������ֽ�*/

#if 0
	/*3.Sent to Host*/
	ext_udp_socket.addr_them.sin_addr.s_addr= inet_addr(terext->rtp_socket.remote_ip);
	return sendto(ext_udp_socket.udp_handle,&st_hf_mgw_ext_pro,tmp_len, \
		0,(struct sockaddr*)&ext_udp_socket.addr_them,ext_udp_socket.addr_len);
#endif

	return Hf_SeatSem_Socket_Send((uint8 *)&st_hf_mgw_ext_pro, tmp_len);
}

int mgw_local_seat_connect_request(TERMINAL_BASE* terext)
{
	unsigned short tmp_len;

	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_CONNECT_REQ connect_req;
	
	/*1.Packet user connect req*/
    memset(&connect_req, 0xFF, sizeof(connect_req));
	memset(&st_hf_mgw_ext_pro, 0x00, sizeof(st_hf_mgw_ext_pro));	
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
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;

	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*����У��������ֽ�*/

#if 0
	/*4.Sent to Host*/
	ext_udp_socket.addr_them.sin_addr.s_addr= inet_addr(terext->rtp_socket.remote_ip);
	return sendto(ext_udp_socket.udp_handle,&st_hf_mgw_ext_pro,tmp_len, \
		0,(struct sockaddr*)&ext_udp_socket.addr_them,ext_udp_socket.addr_len);
#endif

	return Hf_SeatSem_Socket_Send((uint8 *)&st_hf_mgw_ext_pro, tmp_len);
}

int mgw_local_seat_release_request(TERMINAL_BASE* terext)
{
	unsigned short tmp_len;

	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_RELEASE_REQ release_req;
	
	/*1.Packet user connect req*/
    memset(&release_req, 0x00, sizeof(release_req));
	memset(&st_hf_mgw_ext_pro, 0x00, sizeof(st_hf_mgw_ext_pro));
	
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
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;

	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*����У��������ֽ�*/

#if 0
	/*3.Sent to Host*/
	ext_udp_socket.addr_them.sin_addr.s_addr= inet_addr(terext->rtp_socket.remote_ip);
	return sendto(ext_udp_socket.udp_handle,&st_hf_mgw_ext_pro,tmp_len, \
		0,(struct sockaddr*)&ext_udp_socket.addr_them,ext_udp_socket.addr_len);
#endif

	return Hf_SeatSem_Socket_Send((uint8 *)&st_hf_mgw_ext_pro, tmp_len);
}

int mgw_local_seat_release_cnf_request(TERMINAL_BASE* terext)
{
	unsigned short tmp_len;

	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_RELEASE_CNF release_cnf;
	
	/*1.Packet user connect req*/
    memset(&release_cnf, 0x00, sizeof(release_cnf));
	memset(&st_hf_mgw_ext_pro, 0x00, sizeof(st_hf_mgw_ext_pro));	
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
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;

	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*����У��������ֽ�*/

#if 0
	/*3.Sent to Host*/
	ext_udp_socket.addr_them.sin_addr.s_addr= inet_addr(terext->rtp_socket.remote_ip);
	return sendto(ext_udp_socket.udp_handle,&st_hf_mgw_ext_pro,tmp_len, \
		0,(struct sockaddr*)&ext_udp_socket.addr_them,ext_udp_socket.addr_len);
#endif

	return Hf_SeatSem_Socket_Send((uint8 *)&st_hf_mgw_ext_pro, tmp_len);
}

int mgw_local_seat_ptt_request(TERMINAL_BASE* terext)
{
	unsigned short tmp_len;

	ST_HF_MGW_EXT_PRO st_hf_mgw_ext_pro;
	ST_HF_MGW_EXT_PTT_MSG ppt_msg;
	
	/*1.Packet user connect req*/
    memset(&ppt_msg, 0x00, sizeof(ppt_msg));
	memset(&st_hf_mgw_ext_pro, 0x00, sizeof(st_hf_mgw_ext_pro));	
	ppt_msg.id = EN_TYPE_PTT;
	ppt_msg.msg_len = sizeof(ST_HF_MGW_EXT_RELEASE_CNF)-2;
	ppt_msg.slot = terext->bslot_us;

	if(terext->bPTTStatus == MGW_PTT_DOWN)
		ppt_msg.ptt_value = PTT_STATE_SEND;
	else if(terext->bPTTStatus == MGW_PTT_UP)
		ppt_msg.ptt_value = PTT_STATE_RECV;
	
	/*2.Packet MGW_EXT_PRO*/
	st_hf_mgw_ext_pro.header.sync_head = ntohs(0x55aa);
	st_hf_mgw_ext_pro.header.type = BAOWEN_TYPE_CALLCMD;
	st_hf_mgw_ext_pro.header.reserve = ntohs(0x0000);
	tmp_len = sizeof(ST_HF_MGW_EXT_RELEASE_CNF);
	st_hf_mgw_ext_pro.header.len = ntohs(tmp_len);
	memcpy(st_hf_mgw_ext_pro.body,&ppt_msg,tmp_len);
	st_hf_mgw_ext_pro.checksum = cal_checksum((BYTE*)&st_hf_mgw_ext_pro,tmp_len+sizeof(ST_HF_MGW_EXT_PRO_HEADER));
	*(unsigned short*)&st_hf_mgw_ext_pro.body[tmp_len] = st_hf_mgw_ext_pro.checksum;

	tmp_len += sizeof(ST_HF_MGW_EXT_PRO_HEADER) + 2;/*����У��������ֽ�*/

#if 0
	/*3.Sent to Host*/
	ext_udp_socket.addr_them.sin_addr.s_addr= inet_addr(terext->rtp_socket.remote_ip);
	return sendto(ext_udp_socket.udp_handle,&st_hf_mgw_ext_pro,tmp_len, \
		0,(struct sockaddr*)&ext_udp_socket.addr_them,ext_udp_socket.addr_len);
#endif

	return Hf_SeatSem_Socket_Send((uint8 *)&st_hf_mgw_ext_pro, tmp_len);
}


/*====================================mgw_scu==========================================*/
int mgw_scu_call_request(TERMINAL_BASE* terext)
{
	ST_HF_MGW_SCU_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_CALL_REQ call_req;
	int tmp_len;
	char *tmp_dup_str;
	

	printf("%s terext->bslot_us = %d\r\n", __func__, terext->bslot_us);

	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));
	memset(&call_req, 0xFF, sizeof(call_req));	
	
	/*1.packet call request*/
	call_req.id = MGW_SCU_CALL_ID;
	call_req.len = sizeof(ST_HF_MGW_SCU_CALL_REQ) - 2;
	call_req.slot = terext->bslot_us;
	call_req.call_type = terext->bCallType;
	
	tmp_dup_str = strdup((char *)terext->phone_queue_g.bPhoneQueue);
	if(terext->bCallType == EN_CALL_TRUNK)
	{
		call_req.call_type = EN_SEL_CALL;
	}else if(terext->bCallType == EN_FORCE_CALL){/*ǿ��*/
		call_req.call_type = EN_SEL_CALL;
		terext->phone_queue_g.bPhoneQueueHead += 4;
		snprintf((char *)terext->phone_queue_g.bPhoneQueue,MAXPHONENUM, "8012%s",tmp_dup_str);
	}else if(terext->bCallType == EN_FORCE_RELEASE_CALL){/*ǿ��*/
		call_req.call_type = EN_SEL_CALL;
		terext->phone_queue_g.bPhoneQueueHead += 4;
		snprintf((char *)terext->phone_queue_g.bPhoneQueue, MAXPHONENUM, "8013%s",tmp_dup_str);
	}else if(terext->bCallType == EN_CALL_Z){
		call_req.call_type = EN_SEL_CALL;
		call_req.slot = terext->bresv;
	}else if(terext->bCallType == EN_CALL_RADIO){
		call_req.call_type = EN_SEL_CALL;
		call_req.slot = terext->bresv;		
	}
	free(tmp_dup_str);
	
	call_req.encoder = 0x04;	/*Ĭ�ϱ����ʽ*/

	call_req.phone_len = terext->phone_queue_g.bPhoneQueueHead;
	
	phone2bcd((char *)call_req.callee_num,\
		(char *)terext->phone_queue_g.bPhoneQueue,call_req.phone_len);

	tmp_len = sizeof(ST_HF_MGW_SCU_CALL_REQ);

#if 0
	int j = 0;
    for(j = (call_req.phone_len + 1)/2; j < 10; j++)
    {
	    call_req.callee_num[j] = 0xFF;
	}
#endif	
		
	/*2.packet complete protocol*/
	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.version = HF_MGW_SCU_PRO_VER;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_CALL_REQ);
	memcpy(st_hf_mgw_scu_pro.body,&call_req,sizeof(ST_HF_MGW_SCU_CALL_REQ));

	tmp_len += sizeof(ST_HF_MGW_SCU_PRO_HEADER);

	/*3.send packet*/
	return Board_Sem_SendTo_Inc((uint8 *)&st_hf_mgw_scu_pro ,tmp_len);
}


int mgw_scu_callack_request(TERMINAL_BASE* terext)
{
	ST_HF_MGW_SCU_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_CALLACK_REQ callack_req;
	int tmp_len;

	printf("%s terext->bslot_us = %d\r\n", __func__, terext->bslot_us);

	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));
	memset(&callack_req, 0x00, sizeof(callack_req));
	callack_req.id = MGW_SCU_CALLACK_ID;
	callack_req.len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ) - 2;
	callack_req.slot = terext->bslot_us;
	callack_req.call_type = terext->bCallType;
	
	if(terext->bCallType == EN_CALL_TRUNK)
		callack_req.call_type = EN_SEL_CALL;	

	if(terext->bCommand == MC_RINGING || terext->bCommand == MC_RING)
		callack_req.call_status = EN_JIEXU_SUCCESS;
	else if(terext->bCommand == MC_REJECT)
		callack_req.call_status = EN_JIEXU_FAILURE;
	else
		callack_req.call_status = EN_JIEXU_FAILURE;

	tmp_len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ);
	
	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.version = HF_MGW_SCU_PRO_VER;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ);
	memcpy(st_hf_mgw_scu_pro.body,&callack_req,sizeof(ST_HF_MGW_SCU_CALLACK_REQ));

	tmp_len += sizeof(ST_HF_MGW_SCU_PRO_HEADER);
	
	/*3.send packet*/
	return Board_Sem_SendTo_Inc((uint8*)&st_hf_mgw_scu_pro, tmp_len);
}

int mgw_scu_connect_request(TERMINAL_BASE* terext)
{
	ST_HF_MGW_SCU_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_CONNECT_REQ connect_req;
	int tmp_len;

	printf("%s terext->bslot_us = %d\r\n", __func__, terext->bslot_us);

	memset(&connect_req, 0xFF, sizeof(connect_req));	
	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));
	connect_req.id = MGW_SCU_CONNECT_ID;
	connect_req.len = sizeof(ST_HF_MGW_SCU_CONNECT_REQ) - 2;
	connect_req.slot = terext->bslot_us;
	connect_req.call_type = terext->bCallType;

	if(terext->bCallType == EN_CALL_TRUNK)
		connect_req.call_type = EN_SEL_CALL;

	connect_req.encoder = 0x04;
	connect_req.phone_len = terext->phone_queue_g.bPhoneQueueHead;
	phone2bcd((char *)connect_req.caller_num,\
		(char *)terext->phone_queue_g.bPhoneQueue,connect_req.phone_len);

	tmp_len = sizeof(ST_HF_MGW_SCU_CONNECT_REQ);
	
	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.version = HF_MGW_SCU_PRO_VER;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_CONNECT_REQ);
	memcpy(st_hf_mgw_scu_pro.body,&connect_req,sizeof(ST_HF_MGW_SCU_CONNECT_REQ));

	tmp_len += sizeof(ST_HF_MGW_SCU_PRO_HEADER);
	
	/*3.send packet*/
	return Board_Sem_SendTo_Inc((uint8*)&st_hf_mgw_scu_pro, tmp_len);
}

int mgw_scu_release_request(TERMINAL_BASE* terext)
{
	ST_HF_MGW_SCU_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_RELEASE_REQ release_req;
	int tmp_len;

	/**/
	printf("%s terext->bslot_us = %d\r\n", __func__, terext->bslot_us);
	
	memset(&release_req, 0x00, sizeof(release_req));	
	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));	
	release_req.id = MGW_SCU_RELEASE_ID;
	release_req.len = sizeof(ST_HF_MGW_SCU_RELEASE_REQ)-2;
	release_req.slot = terext->bslot_us;
	release_req.call_type = terext->bCallType;

	if(terext->bCallType == EN_CALL_TRUNK)
		release_req.call_type = EN_SEL_CALL;
	else if(terext->bCallType == EN_CALL_Z){
		/*fix radio direct call*/
		release_req.call_type = EN_SEL_CALL;
		release_req.slot = terext->bresv;
	}else if(terext->bCallType == EN_CALL_RADIO){
		/*fix radio direct call*/
		release_req.call_type = EN_SEL_CALL;
		release_req.slot = terext->bresv;
	}
	
	release_req.rel_reason = terext->rel_reason;

	tmp_len = sizeof(ST_HF_MGW_SCU_RELEASE_REQ);
	
	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.version = HF_MGW_SCU_PRO_VER;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_RELEASE_REQ);
	memcpy(st_hf_mgw_scu_pro.body,&release_req,sizeof(ST_HF_MGW_SCU_RELEASE_REQ));

	tmp_len += sizeof(ST_HF_MGW_SCU_PRO_HEADER);

	/*3.send packet*/
	return Board_Sem_SendTo_Inc((uint8*)&st_hf_mgw_scu_pro, tmp_len);
}

int mgw_scu_release_cnf_request(TERMINAL_BASE* terext)
{
	ST_HF_MGW_SCU_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_RELEASE_CNF_REQ release_cnf_req;
	int tmp_len;

	printf("%s terext->bslot_us = %d\r\n", __func__, terext->bslot_us);
	
	memset(&release_cnf_req, 0x00, sizeof(release_cnf_req));	
	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));	
	release_cnf_req.id = MGW_SCU_RELEASE_CNF_ID;
	release_cnf_req.len = sizeof(ST_HF_MGW_SCU_RELEASE_CNF_REQ)-2;
	release_cnf_req.slot = terext->bslot_us;
	release_cnf_req.call_type = terext->bCallType;

	tmp_len = sizeof(ST_HF_MGW_SCU_RELEASE_CNF_REQ);

	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.version = HF_MGW_SCU_PRO_VER;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_RELEASE_CNF_REQ);
	memcpy(st_hf_mgw_scu_pro.body,&release_cnf_req,sizeof(ST_HF_MGW_SCU_RELEASE_CNF_REQ));

	tmp_len += sizeof(ST_HF_MGW_SCU_PRO_HEADER);

	return Board_Sem_SendTo_Inc((uint8*)&st_hf_mgw_scu_pro, tmp_len);
}

int mgw_scu_ptt_request(TERMINAL_BASE * terext)
{
	ST_HF_MGW_SCU_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_PTT_MSG ptt_msg;
	int tmp_len;


	memset(&ptt_msg, 0x00, sizeof(ptt_msg));	
	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));
	
	ptt_msg.id = MGW_SCU_PTT_ID;
	ptt_msg.len = sizeof(ST_HF_MGW_SCU_PTT_MSG)-2;
	ptt_msg.slot = terext->bslot_us;

	/*fix radio hf direct talk 2012-04-13*/
	if(terext->bCallType == EN_CALL_Z)
		ptt_msg.slot = terext->bresv;

	if(terext->bCallType == EN_CALL_RADIO)
		ptt_msg.slot = terext->bresv;

	phone2bcd((char *)ptt_msg.peer_num, \
		(char *)terext->name, MAXPHONENUMBYTE);
	if(terext->bPTTStatus == MGW_PTT_DOWN)
		ptt_msg.ptt_status = PTT_STATE_SEND;
	else if(terext->bPTTStatus == MGW_PTT_UP)
		ptt_msg.ptt_status = PTT_STATE_RECV;

	tmp_len = sizeof(ST_HF_MGW_SCU_PTT_MSG);

	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.version = HF_MGW_SCU_PRO_VER;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_PTT_MSG);
	memcpy(st_hf_mgw_scu_pro.body,&ptt_msg,sizeof(ST_HF_MGW_SCU_PTT_MSG));

	tmp_len += sizeof(ST_HF_MGW_SCU_PRO_HEADER);

	return Board_Sem_SendTo_Inc((uint8*)&st_hf_mgw_scu_pro, tmp_len);
}


/* �ڷ�ģʽ��PRN��ս������ר�ߵĺ���Ӧ�� ����������Ϊ0x01 */
int mgw_scu_pf_zwzhuanxian_callack(TERMINAL_BASE* terext)
{
	ST_HF_MGW_SCU_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_CALLACK_REQ callack_req;
	int tmp_len;

	printf("lsb:pf zw %s terext->bslot_us = %d\r\n", __func__, terext->bslot_us);
	
	callack_req.id = MGW_SCU_CALLACK_ID;
	callack_req.len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ) - 2;
	callack_req.slot = terext->bslot_us;
	callack_req.call_type = 0x01;
	callack_req.call_status = EN_JIEXU_SUCCESS;
	
	tmp_len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ);
	
	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.version = HF_MGW_SCU_PRO_VER;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ);
	memcpy(st_hf_mgw_scu_pro.body,&callack_req,sizeof(ST_HF_MGW_SCU_CALLACK_REQ));

	tmp_len += sizeof(ST_HF_MGW_SCU_PRO_HEADER);
	
	/*3.send packet*/
	return Board_Sem_SendTo_Inc((uint8*)&st_hf_mgw_scu_pro, tmp_len);
}


int mgw_scu_zhuanxian_callack(TERMINAL_BASE* terext)
{
	ST_HF_MGW_SCU_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_CALLACK_REQ callack_req;
	int tmp_len;

	printf("%s terext->bslot_us = %d\r\n", __func__, terext->bslot_us);

	memset(&callack_req, 0x00, sizeof(callack_req));	
	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));	
	callack_req.id = MGW_SCU_CALLACK_ID;
	callack_req.len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ) - 2;
	callack_req.slot = terext->bslot_us;
	callack_req.call_type = 0x04;
	callack_req.call_status = EN_JIEXU_SUCCESS;


	tmp_len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ);
	
	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.version = HF_MGW_SCU_PRO_VER;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ);
	memcpy(st_hf_mgw_scu_pro.body,&callack_req,sizeof(ST_HF_MGW_SCU_CALLACK_REQ));

	tmp_len += sizeof(ST_HF_MGW_SCU_PRO_HEADER);
	
	/*3.send packet*/
	return Board_Sem_SendTo_Inc((uint8*)&st_hf_mgw_scu_pro, tmp_len);
}

int mgw_scu_zhuanxian_connect_request(TERMINAL_BASE* terext)
{
	ST_HF_MGW_SCU_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_CONNECT_REQ connect_req;
	int tmp_len;

	printf("%s terext->bslot_us = %d\r\n", __func__, terext->bslot_us);

	memset(&connect_req, 0x00, sizeof(connect_req));	
	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));	
	connect_req.id = MGW_SCU_CONNECT_ID;
	connect_req.len = sizeof(ST_HF_MGW_SCU_CONNECT_REQ) - 2;
	connect_req.slot = terext->bslot_us;
	connect_req.call_type = 0x04;
	connect_req.encoder = 0x04;
	
	tmp_len = sizeof(ST_HF_MGW_SCU_CONNECT_REQ);
	
	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.version = HF_MGW_SCU_PRO_VER;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_CONNECT_REQ);
	memcpy(st_hf_mgw_scu_pro.body,&connect_req,sizeof(ST_HF_MGW_SCU_CONNECT_REQ));

	tmp_len += sizeof(ST_HF_MGW_SCU_PRO_HEADER);
	
	/*3.send packet*/
	return Board_Sem_SendTo_Inc((uint8*)&st_hf_mgw_scu_pro, tmp_len);
}


int mgw_scu_zhuanxian_msg(TERMINAL_BASE * terext)
{
	ST_HF_MGW_SCU_PRO msg;

	printf("%s\r\n", __func__);	
	
	memset(&msg, 0x00, sizeof(msg));

	/*2.packet complete protocol*/
	msg.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	msg.header.version = HF_MGW_SCU_PRO_VER;
	msg.header.info_type = INFO_TYPE_CALL_CTL;
	msg.header.len = 0x04;

	msg.body[0] = MGW_SCU_CALL_TARNS_ID;
	msg.body[1] = 0x02;
	msg.body[2] = terext->bslot_us;	

	if(HOOK_OFF == terext->bHookStatus)
	{
		msg.body[3] = 0x01;
	}
	else
	{
		msg.body[3] = 0x00;
	}

	/*3.send packet*/
	return Board_Sem_SendTo_Inc((uint8 *)&msg \
		,msg.header.len + sizeof(ST_HF_MGW_SCU_PRO_HEADER));
}


int mgw_scu_calltrans_request(TERMINAL_BASE* terext)
{
	ST_HF_MGW_SCU_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_CALL_TRANS_MSG calltrans_msg;
	int tmp_len;
	
	/*1.packet call request*/
	memset(&calltrans_msg, 0x00, sizeof(calltrans_msg));	
	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));
	
	calltrans_msg.id = MGW_SCU_CALL_TARNS_ID;
	calltrans_msg.len = sizeof(ST_HF_MGW_SCU_CALL_TRANS_MSG) - 2;
	calltrans_msg.slot = terext->bslot_us;

	calltrans_msg.phone_len = terext->phone_queue_g.bPhoneQueueHead;
	phone2bcd((char *)calltrans_msg.trans_phone,\
		(char *)terext->phone_queue_g.bPhoneQueue,calltrans_msg.phone_len);

	tmp_len = sizeof(ST_HF_MGW_SCU_CALL_TRANS_MSG);
	
	/*2.packet complete protocol*/
	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.version = HF_MGW_SCU_PRO_VER;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_CALL_TRANS_MSG);
	memcpy(st_hf_mgw_scu_pro.body,&calltrans_msg,sizeof(ST_HF_MGW_SCU_CALL_TRANS_MSG));

	tmp_len += sizeof(ST_HF_MGW_SCU_PRO_HEADER);

	/*3.send packet*/
	return Board_Sem_SendTo_Inc((uint8*)&st_hf_mgw_scu_pro, tmp_len);

}


int mgw_scu_listen_request(TERMINAL_BASE* terext,char operate)
{
	ST_HF_MGW_SCU_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_LISTEN_MSG listen_msg;
	int tmp_len;
	
	/*1.packet call request*/
	memset(&listen_msg, 0x00, sizeof(listen_msg));	
	memset(&st_hf_mgw_scu_pro, 0x00, sizeof(st_hf_mgw_scu_pro));	
	listen_msg.id = MGW_SCU_LISTEN_ID;
	listen_msg.len = sizeof(ST_HF_MGW_SCU_LISTEN_MSG) - 2;
	listen_msg.slot = terext->bslot_us;
	listen_msg.operate = operate;

	tmp_len = sizeof(ST_HF_MGW_SCU_CALL_TRANS_MSG);
	
	/*2.packet complete protocol*/
	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.version = HF_MGW_SCU_PRO_VER;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_LISTEN_MSG);
	memcpy(st_hf_mgw_scu_pro.body,&listen_msg,sizeof(ST_HF_MGW_SCU_LISTEN_MSG));

	tmp_len += sizeof(ST_HF_MGW_SCU_PRO_HEADER);

	/*3.send packet*/
	return Board_Sem_SendTo_Inc((uint8*)&st_hf_mgw_scu_pro, tmp_len);
}


/*=======================================================================*/
static int	seat_sem_protocol_parse
(
	ST_HF_MGW_EXT_PRO* pro,
	size_t count,
	struct sockaddr_in *addr_src
)
{
	
	/*1.����У�飬���*/
	if(pro->header.sync_head != 0x55aa)
	{
		return (-1);
	}
    #if 0
	int i = 0;
	uint8 *tmp_buf = (ST_HF_MGW_EXT_PRO*)pro;

    printf("%s: len = %d\r\n", __func__, count);
	for(i = 0; i < count; i++)
	{
		printf("%x:", tmp_buf[i]);
	}
	printf("\r\n");
    #endif

	/* �����ر�ע�⣬ʹ�ýṹ�����Э��������ֽڶ��룬�ڴ�Ӧʹ��4�ֽڶ���*/
	pro->checksum = *(unsigned short*)&pro->body[pro->header.len];
	if(!MGW_EQUAL(pro->checksum,cal_checksum((BYTE*)pro,count-2)))
	{
		DEBUG_OUT("Checksum Error! %.4x : %.4x\n",pro->checksum, cal_checksum((BYTE*)pro,count-2));
		return DRV_ERR;
	}

	/*2.Э�����,װ��*/
	{
		/*
		** �����Ӧ��Э���ֶ�
		* �Ժ���Э����г����ж�
		*/
		switch(pro->header.type)
		{
			case BAOWEN_TYPE_CALLCFG:	/*�������ñ���*/
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

			case BAOWEN_TYPE_CALLCMD:	/*�������������*/
				switch(((ST_HF_MGW_EXT_CALL_REQ*)pro->body)->id)
				{
					case EN_TYPE_CALL_REQ:
						{
							int wTemPort;
							wTemPort = find_port_by_ip(addr_src);
							if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
							{
								VERBOSE_OUT(LOG_SYS,"UnRegister XIWEI User!\n");

								/*��û���ҵ������û�������£�Ӧ�����巢�ͺ���Ӧ��Я������״̬��Ϣ*/
								return DRV_ERR;
							}
							
							terminal_group[wTemPort].bCallType = ((ST_HF_MGW_EXT_CALL_REQ*)pro->body)->call_type;
							if(terminal_group[wTemPort].bCallType >= MAX_CALL_JIEXU_TYPE)
								terminal_group[wTemPort].bCallType = EN_SEL_CALL;
							if(terminal_group[wTemPort].bCallType == EN_CALL_FORWARD_TO && terminal_group[wTemPort].bCommand != MC_TALK){
								VERBOSE_OUT(LOG_SYS,"Recv call Forward but not in talk state\n");
								return DRV_ERR;
							}
							if(terminal_group[wTemPort].bCallType == EN_CALL_FORWARD_TO && terminal_group[wTemPort].bCommand == MC_TALK)
								terminal_group[wTemPort].bCommand = MC_TRANS;
							
							terminal_group[wTemPort].phone_queue_g.bFinish = 1;
							terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;
							terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = ((ST_HF_MGW_EXT_CALL_REQ*)pro->body)->phone_len;
							bcd2phone((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
								(char *)((ST_HF_MGW_EXT_CALL_REQ*)pro->body)->callee_num,\
									((ST_HF_MGW_EXT_CALL_REQ*)pro->body)->phone_len);

							/*if the call type is znet then we should save the slot*/
							if(terminal_group[wTemPort].bCallType == EN_CALL_Z){
								int tmp_radio_index = 0;
								tmp_radio_index = atoi((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue);
								if(tmp_radio_index<1 || tmp_radio_index>8) 
									return DRV_ERR;
								terminal_group[wTemPort].bresv = DIRECTCALL_RADIO_INDEX[tmp_radio_index-1];
								terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;
								terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = strlen(terminal_group[wTemPort].name);
								//strcpy(terminal_group[wTemPort].phone_queue_g.bPhoneQueue,terminal_group[wTemPort].name);
                                				strncpy((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue,terminal_group[wTemPort].name, MAXPHONENUM);
							}

							if(terminal_group[wTemPort].bCallType == EN_CALL_RADIO){
								int tmp_radio_index = 0;
								tmp_radio_index = atoi((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue);
								if(tmp_radio_index<0 || tmp_radio_index>7) /*fix z024 radio direct call -- swf 20140107*/
									return DRV_ERR;
								terminal_group[wTemPort].bresv = DIRECTCALL_RADIO_INDEX[tmp_radio_index];
								terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;
								terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = strlen(terminal_group[wTemPort].name);
								//strcpy(terminal_group[wTemPort].phone_queue_g.bPhoneQueue,terminal_group[wTemPort].name);
                               				strncpy((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue,terminal_group[wTemPort].name, MAXPHONENUM);
							}

							terminal_group[wTemPort].bHookStatus = HOOK_OFF;

							VERBOSE_OUT(LOG_SYS,"Recv XIWEI%d Call Request!,type:%d\n",wTemPort, \
								terminal_group[wTemPort].bCallType);
						}
					break;

					case EN_TYPE_NUM_TRANS:
					{
#if 0					
						ST_HF_MGW_EXT_USER_NUMTRANS_MSG * msg;
						int wTemPort;
						int wConnectPort;
						wTemPort = find_port_by_ip(addr_src);
						if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
						{
							VERBOSE_OUT(LOG_SYS,"UnRegister XIWEI User!\n");
							return DRV_ERR;
						}
						msg = (ST_HF_MGW_EXT_USER_NUMTRANS_MSG *)pro->body;
						if(terminal_group[wTemPort].wConnectPort < USER_NUM_MAX){
							wConnectPort = terminal_group[wTemPort].wConnectPort;	
							bcd2phone((char *)terminal_group[wConnectPort].phone_queue_s.bPhoneQueue, \
								(char *)msg->trans_phone,msg->phone_len);
							terminal_group[wConnectPort].phone_queue_s.bPhoneQueueHead = msg->phone_len;
							terminal_group[wConnectPort].phone_queue_s.bPhoneQueueTail = 0;
							terminal_group[wConnectPort].phone_queue_s.bFinish = 1;
							VERBOSE_OUT(LOG_SYS,"Recv XIWEI%d Num Trans: %s\n",wConnectPort, \
								terminal_group[wConnectPort].phone_queue_s.bPhoneQueue);
						}else
							VERBOSE_OUT(LOG_SYS,"Can't find Connect User!\n");
#else
						int wTemPort ;
						int wConnectPort;
						ST_HF_MGW_EXT_USER_NUMTRANS_MSG * msg;
						wTemPort = find_port_by_ip(addr_src);
						if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
						{
							VERBOSE_OUT(LOG_SYS,"UnRegister sw phone User!\n");
							return DRV_ERR;
						}

						msg = (ST_HF_MGW_EXT_USER_NUMTRANS_MSG *)pro->body;
						wConnectPort = terminal_group[wTemPort].wConnectPort;
						bcd2phone((char *)terminal_group[wConnectPort].phone_queue_s.bPhoneQueue, \
								(char *)msg->trans_phone,msg->phone_len);
						
						terminal_group[wConnectPort].phone_queue_s.bPhoneQueueHead = msg->phone_len;
						terminal_group[wConnectPort].phone_queue_s.bPhoneQueueTail = 0;			
						terminal_group[wConnectPort].phone_queue_s.bFinish = 1;
							
						VERBOSE_OUT(LOG_SYS,"Recv sw phone %d Call  %s!\n",wTemPort - KUOZHAN_PHONE_OFFSET, \
							(char *)terminal_group[wConnectPort].phone_queue_s.bPhoneQueue);
#endif
					}
					break;

					case EN_TYPE_CALL_ANS:
						{
							int wTemPort;
							wTemPort = find_port_by_ip(addr_src);
							if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
							{
								VERBOSE_OUT(LOG_SYS,"UnRegister XIWEI User!\n");
								return DRV_ERR;
							}
							VERBOSE_OUT(LOG_SYS,"Recv XIWEI %d Call Ans!\n",wTemPort);
							
							terminal_group[wTemPort].bCallType = ((ST_HF_MGW_EXT_CALLACK_REQ*)pro->body)->call_type;

							if(((ST_HF_MGW_EXT_CALLACK_REQ*)pro->body)->call_statu == EN_JIEXU_FAILURE)
								terminal_group[wTemPort].bCommand = MC_REJECT;
							else if(((ST_HF_MGW_EXT_CALLACK_REQ*)pro->body)->call_statu == EN_JIEXU_SUCCESS)
								terminal_group[wTemPort].bCommand = MC_RINGING;
							else
								terminal_group[wTemPort].bCommand = MC_REJECT;
							
							mgw_scu_callack_request(&terminal_group[wTemPort]);
						}
					break;

					case EN_TYPE_CONNECT_REQ:
					{
						int wTemPort;
						wTemPort = find_port_by_ip(addr_src);
						if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
						{
							VERBOSE_OUT(LOG_SYS,"UnRegister XIWEI User!\n");
							return DRV_ERR;
						}
						VERBOSE_OUT(LOG_SYS,"Recv XIWEI %d Connect request!\n",wTemPort);
						if(((ST_HF_MGW_EXT_CONNECT_REQ*)pro->body)->rtp_port){
							terminal_group[wTemPort].rtp_socket.remote_port = ((ST_HF_MGW_EXT_CONNECT_REQ*)pro->body)->rtp_port;
							terminal_group[wTemPort].rtp_socket.local_port = ((ST_HF_MGW_EXT_CONNECT_REQ*)pro->body)->rtp_port;
						}else{
							terminal_group[wTemPort].rtp_socket.remote_port = get_random_port();
							terminal_group[wTemPort].rtp_socket.local_port = get_random_port();
						}
						VERBOSE_OUT(LOG_SYS,"XIWEI%d port:%d\n",wTemPort,terminal_group[wTemPort].rtp_socket.remote_port);
						terminal_group[wTemPort].bHookStatus = HOOK_OFF;
						terminal_group[wTemPort].bCallType = ((ST_HF_MGW_EXT_CONNECT_REQ*)pro->body)->call_type;
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
						wTemPort = find_port_by_ip(addr_src);
						if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
						{
							VERBOSE_OUT(LOG_SYS,"UnRegister XIWEI User!\n");
							return DRV_ERR;
						}

						terminal_group[wTemPort].bCommand = MC_HANG_ACTIVE;
						terminal_group[wTemPort].bHookStatus = HOOK_ON;
						/*for fix radio direct call--20120420*/
						//terminal_group[wTemPort].bCallType = ((ST_HF_MGW_EXT_RELEASE_REQ*)pro->body)->call_type;
						terminal_group[wTemPort].rel_reason = ((ST_HF_MGW_EXT_RELEASE_REQ*)pro->body)->rel_type;
						VERBOSE_OUT(LOG_SYS,"Recv XIWEI %d Release Request!\n",wTemPort);
						
						mgw_scu_release_request(terminal_group);
					}
					break;

					case EN_TYPE_PTT:
					{
						int wTemPort;
						ST_HF_MGW_EXT_PTT_MSG * msg = (ST_HF_MGW_EXT_PTT_MSG *)pro->body;
						wTemPort = find_port_by_ip(addr_src);
						if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
						{
							VERBOSE_OUT(LOG_SYS,"UnRegister XIWEI User!\n");
							return DRV_ERR;
						}
						if(msg->ptt_value == PTT_STATE_SEND){
							terminal_group[wTemPort].bPTTStatus = MGW_PTT_DOWN;
							mgw_scu_ptt_request(&terminal_group[wTemPort]);
						}else if(msg->ptt_value == PTT_STATE_RECV){
							terminal_group[wTemPort].bPTTStatus = MGW_PTT_UP;
							mgw_scu_ptt_request(&terminal_group[wTemPort]);
						}
					}
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
	}
	
	return DRV_OK;
}

static int Ipc_dat_protocol_parse(ST_HF_MGW_DCU_PRO* pro,size_t count,struct sockaddr_in *addr_src)
{
	struct sockaddr_in addr_dcu;
	int i = 0;

#if 0
	int i = 0;
	uint8 *buf = (uint8 *)pro;
	printf("%s: \r\n", __func__);
	for(i=0;i<count;i++)
	{
		printf("%x:", buf[i]);
	}
	printf("\r\n");
#endif	
	
	if(pro->header.data_len != count-sizeof(ST_HF_MGW_DCU_PRO_HEADER))
	{
		return DRV_ERR;
	}

	if(pro->header.baowen_type == BAOWEN_TYPE_MGW_DCU)
	{
		switch(pro->header.info_type)
		{
			case INT_FRAME_TYPE_CMD_CFG:
			{
				switch(pro->body[0])
				{
					case MGW_DCU_ADDR_ID:
						{
							ST_HF_MGW_DCU_ADDR_MSG * msg = (ST_HF_MGW_DCU_ADDR_MSG *)pro->body;
							//struct JS_NODE * tmp_node;
							inc_log(LOG_DEBUG,"recv hf addr : %.8x\n", (unsigned int)msg->hf_addr);

							g_hf_addr = msg->hf_addr;
						}
						break;	

					case MGW_DCU_RADIO_FLOW_CTRL1_ID:
					case MGW_DCU_RADIO_FLOW_CTRL_ID:
						{
							/*͸��������Ϣ֡*/
							ST_HF_MGW_FLOW_CONTROL_MSG flowctl;
							struct sockaddr_in brdaddr;

							flowctl.id = 0xf0f1;
							flowctl.len = 7;

							/*pro->body+3---ȥ��Э��ǰ��3���ֽڰ�����Ϣ�����볤����Ϣ*/
							memcpy(flowctl.data,pro->body+3,flowctl.len);
							
							brdaddr.sin_family = AF_INET;
							brdaddr.sin_addr.s_addr = mgw_if_fetch_boardaddr("eth0:1");
							brdaddr.sin_port = 30001;
							
							inc_log(LOG_DEBUG,"recv flow control && send boardcast!\n");
							sendto(ext_udp_socket.udp_handle,&flowctl,4+flowctl.len, \
									0,(struct sockaddr*)&brdaddr,ext_udp_socket.addr_len);
						}
						break;
					default:
						break;
					}
				}
				break;

			case INT_FRAME_TYPE_CMD_ROUTE:
				{
					/*���ܵ�·��Ӧ����Ϣ*/
					switch(pro->body[0])
					{
						case MGW_DCU_ROUTE_FIND_ID:
							/*·�ɲ�ѯ��Ϣ*/
							{
								ST_HF_MGW_DCU_PRO route_pro;
								ST_HF_MGW_DCU_ROUTE_MSG * msg;
								struct JS_ROUTE_TABLE_T * tmp_js_route_table_t;
								int i = 0;

								VERBOSE_OUT(LOG_SYS,"Recv Route find req!\n");

								route_pro.header.baowen_type = BAOWEN_TYPE_MGW_DCU;
								route_pro.header.dst_addr = 0x10;
								route_pro.header.src_addr = 0xb0;
								route_pro.header.info_type = INT_FRAME_TYPE_CMD_ROUTE;

								msg = (ST_HF_MGW_DCU_ROUTE_MSG *)route_pro.body;
								msg->id = MGW_DCU_ROUTE_ID;
								msg->route_index = 1;
								INC_LIST_TRAVERSE(&js_route_user_table,tmp_js_route_table_t,list)
								{
									msg->route[i].hf_addr = tmp_js_route_table_t->mask;
									msg->route[i].next_hop = tmp_js_route_table_t->mask;
									msg->route[i].module_addr = 0xb0;
									msg->route[i].cost = 100;
									i++;
								}
								msg->len = 11*i+4;
								route_pro.header.data_len = msg->len+3;
								msg->route_count = i;

								VERBOSE_OUT(LOG_SYS,"Send %d Route msg!\n",i);
								
								memcpy(&addr_dcu,addr_src,sizeof(struct sockaddr_in));
								addr_dcu.sin_port = htons(9050);
								
								Board_Data_SendTo_Inc_Addr((uint8 *)&route_pro, \
									sizeof(ST_HF_MGW_DCU_PRO_HEADER)+msg->len+3, addr_dcu);
							}
							break;

						case MGW_DCU_ROUTE_ANS_ID:
							break;

						case MGW_DCU_ADD_IP_ROUTE_ANS_ID:
							{
								ST_HF_MGW_DCU_ADD_IP_ROUTE_ANS * ans = (ST_HF_MGW_DCU_ADD_IP_ROUTE_ANS *)pro->body;
								int i;
								for(i=0;i<ans->route_count;i++)
								{
									if(ans->route[i].status == EN_TYPE_ROUTE_ANS_SUCCESS)
										VERBOSE_OUT(LOG_SYS,"Recv add %x route ans success!\n", \
										 ans->route[i].ip_addr);
									else if(ans->route[i].status == EN_TYPE_ROUTE_ANS_FAILURE)
										VERBOSE_OUT(LOG_SYS,"Recv add %x route ans failure!\n", \
										 ans->route[i].ip_addr);
								}
							}
							break;

						case MGW_DCU_DEL_IP_ROUTE_ANS_ID:
							{
								ST_HF_MGW_DCU_DEL_IP_ROUTE_ANS * ans = (ST_HF_MGW_DCU_DEL_IP_ROUTE_ANS *)pro->body;
								int i;
								for(i=0;i<ans->route_count;i++)
								{
									if(ans->route[i].status == EN_TYPE_ROUTE_ANS_SUCCESS)
										VERBOSE_OUT(LOG_SYS,"Recv del %x route ans success!\n", \
										 ans->route[i].ip_addr);
									else if(ans->route[i].status == EN_TYPE_ROUTE_ANS_FAILURE)
										VERBOSE_OUT(LOG_SYS,"Recv del %x route ans failure!\n", \
										 ans->route[i].ip_addr);
								}
							}

							break;
						
						case MGW_DCU_HF_ROUTE_QUERY_ANS_ID:
							
							break;
				        default:
						    break;
					}
				}
				break;

			case INT_FRAME_TYPE_DATA:
				{
					int i;
					ST_HF_CCC_CCU_PRO_HEADER * ccc_ccu_head = (ST_HF_CCC_CCU_PRO_HEADER *)pro->body;
					for(i=0;i<ccc_ccu_head->dst_count;i++)
					{
						inc_log(LOG_DEBUG,"Send Data to %.8x\n", (unsigned int)ccc_ccu_head->dst_addr[i]);
						//transmit_packet(ccc_ccu_head->dst_addr[i],(char *)pro->body,count-sizeof(ST_HF_MGW_DCU_PRO_HEADER));
					}
					
				}
				break;

			case INT_FRAME_TYPE_CMD_XINLING:
				{
					/*ý�����ذ�͸��ǿ�塢ǿ������*/
					ST_HF_MGW_JS_ADD_XINLING_MSG_HEADER * xinling = (ST_HF_MGW_JS_ADD_XINLING_MSG_HEADER *)pro->body;
					/*ֱ���޸ı������ݣ�����������͸��������Զ�*/
					pro->header.dst_addr = 0x10;
					pro->header.src_addr = 0xb0;
					inc_log(LOG_DEBUG,"Send xinling to %.8x\n", (unsigned int)xinling->dst);
					//transmit_packet(xinling->dst, (char *)pro,count);
				}
				break;
			default:
				break;
		}
	}

	
	/*�ж�Ŀ�ĵ�ַ�Ƿ���ý�����ذ��ַ*/
	if(pro->header.dst_addr == 0xb0)
	{
		ST_HF_CCU_MGW_PRO *st_hf_ccu_mgw_pro = (ST_HF_CCU_MGW_PRO *)pro->body;

		/*�ӱ��ĳ����ж� swf fix 2011/11/02*/
		if(st_hf_ccu_mgw_pro->header.len != pro->header.data_len-3)
			return DRV_ERR;

		switch(st_hf_ccu_mgw_pro->header.info_type)
		{
			case INFO_TYPE_CCU_MGW_IP_CONFIG_REQ_ID:
				{
					ST_BOARD_DATA Send;
					ST_HF_CCU_MGW_IP_CONFIG_REQ *ip_config = (ST_HF_CCU_MGW_IP_CONFIG_REQ *)st_hf_ccu_mgw_pro->body;
					//ST_HF_CCU_MGW_IP_CONFIG_ANS *ip_config_ans = (ST_HF_CCU_MGW_IP_CONFIG_ANS *)st_hf_ccu_mgw_pro->body;
					//int tmp_len;
					//char cmd_tmp[33];
					struct in_addr tmp_addr;
					//char str_tmp_addr[16];
					int tmp_val = 0;
					
					if(ip_config->eth_index == 0)
					{
						tmp_addr.s_addr = ip_config->ip_addr;

						if(mgw_equal_if(ip_config->ip_addr)>1){
							tmp_val = 0x01;
							goto ERR_OUT1;
						}

						if(ip_config->ip_addr == 0 || ip_config->ip_addr == 0xffffffff){
							tmp_val = 0xff;
							goto ERR_OUT1;
						}

						if(ip_config->ip_mask == 0 || ip_config->ip_mask == 0xffffffff){
							tmp_val = 0xff;
							goto ERR_OUT1;
						}						

						tmp_addr.s_addr = ip_config->ip_addr;
						change_config_cat_var(mgw_cfg, "ETH", "EXTEND_LOCAL_IP", inc_inet_ntoa(tmp_addr));
						tmp_addr.s_addr = ip_config->ip_mask;
						change_config_cat_var(mgw_cfg, "ETH", "EXTEND_LOCAL_NETMASK", inc_inet_ntoa(tmp_addr));

						//rewrite_config(mgw_cfg);
						Param_update.cfg_wr_flg = 1;

ERR_OUT1:

						memset(&Send, 0x00, sizeof(Send));
						Send.Head.PackType = 0x90;				
						Send.Head.EquDestAddr8 = 0x10;
						Send.Head.EquSourceAddr8 = 0xb0;
						Send.Head.FrameType = 0x01;
						i = 0;
						Send.Data[i++] = INFO_TYPE_CCU_MGW_IP_CONFIG_ANS_ID;
						Send.Data[i++] = 0x00;
						Send.Data[i++] = 0x01;
					
						Send.Data[i++] = tmp_val;
						Send.Head.DataLen = i;						

						Board_Data_SendTo_Inc((uint8 *)&Send, Send.Head.DataLen + sizeof(ST_BOARD_DATA_HEAD));						
						
					}
				}
				break;
			case INFO_TYPE_CCU_MGW_DETECT_ETH_REQ_ID:
				{
#if 0				
					printf("11111111111111\r\n");
					/*��ѯ���ڲ���������Ϣ*/
					int tmp_len;
					ST_HF_CCU_MGW_DETECT_ETH_ANS * detect_eth_ans = (ST_HF_CCU_MGW_DETECT_ETH_ANS *)st_hf_ccu_mgw_pro->body;

					detect_eth_ans->eth_local[0].eth_index = 0;
					//detect_eth_ans->eth_local[0].ip_addr = mgw_if_fetch_ip("eth0");
					//detect_eth_ans->eth_local[0].ip_mask = mgw_if_fetch_mask("eth0");
					//detect_eth_ans->eth_local[0].udp_port = htons(ext_udp_socket.local_port);
					detect_eth_ans->eth_local[0].ip_addr = 0x0a000003;
					detect_eth_ans->eth_local[0].ip_mask = 0xffffff00;
					detect_eth_ans->eth_local[0].udp_port = htons(30000);					
					
					/*����Э��ͷ��Ϣ*/
					st_hf_ccu_mgw_pro->header.info_type = INFO_TYPE_CCU_MGW_DETECT_ETH_ANS_ID;
					st_hf_ccu_mgw_pro->header.len = 11;
					/*����Ӧ����Ϣ*/

					memcpy(&addr_dcu,addr_src,sizeof(struct sockaddr_in));
					addr_dcu.sin_port = htons(9050);

					pro->header.dst_addr = 0x10;
					pro->header.src_addr = 0xb0;
					pro->header.data_len = sizeof(ST_HF_CCU_MGW_PRO_HEADER) + st_hf_ccu_mgw_pro->header.len;

					tmp_len = sizeof(ST_HF_MGW_DCU_PRO_HEADER) + pro->header.data_len;
					Board_Data_SendTo_Inc_Addr((uint8 *)&pro, tmp_len, addr_dcu);
#else
					ST_BOARD_DATA Send;	
					memset(&Send, 0x00, sizeof(Send));
					Send.Head.PackType = 0x90;				
					Send.Head.EquDestAddr8 = 0x10;
					Send.Head.EquSourceAddr8 = 0xb0;
					Send.Head.FrameType = 0x01;

				//-------------------��Ӧ-------------------------
					i = 0;
					Send.Data[i++] = INFO_TYPE_CCU_MGW_DETECT_ETH_ANS_ID;
					Send.Data[i++] = (11>>8)&0x0ff;//len hi
					Send.Data[i++] =  (11) & 0x0ff;//len low	
				
					Send.Data[i++] = 0x00;

					Send.Data[i++] = ((htonl(inet_addr(get_config_var_str(mgw_cfg,"EXTEND_LOCAL_IP")))) >> 24)&0x0ff;
					Send.Data[i++] = ((htonl(inet_addr(get_config_var_str(mgw_cfg,"EXTEND_LOCAL_IP")))) >> 16)&0x0ff;
					Send.Data[i++] = ((htonl(inet_addr(get_config_var_str(mgw_cfg,"EXTEND_LOCAL_IP")))) >> 8)&0x0ff;
					Send.Data[i++] =  (htonl(inet_addr(get_config_var_str(mgw_cfg,"EXTEND_LOCAL_IP")))) & 0x0ff;
					Send.Data[i++] = ((htonl(inet_addr(get_config_var_str(mgw_cfg,"EXTEND_LOCAL_NETMASK")))) >> 24)&0x0ff;
					Send.Data[i++] = ((htonl(inet_addr(get_config_var_str(mgw_cfg,"EXTEND_LOCAL_NETMASK")))) >> 16)&0x0ff;
					Send.Data[i++] = ((htonl(inet_addr(get_config_var_str(mgw_cfg,"EXTEND_LOCAL_NETMASK")))) >> 8)&0x0ff;
					Send.Data[i++] =  (htonl(inet_addr(get_config_var_str(mgw_cfg,"EXTEND_LOCAL_NETMASK")))) & 0x0ff;
					Send.Data[i++] = (30000 >> 8)&0x0ff;
					Send.Data[i++] =  30000 & 0x0ff;
					Send.Head.DataLen = i;						

					Board_Data_SendTo_Inc((uint8 *)&Send, Send.Head.DataLen + sizeof(ST_BOARD_DATA_HEAD));
#endif
				}
				break;
				
#if 0
			case INFO_TYPE_CCU_MGW_ADD_FENJI_REQ_ID:
				{
					int tmp_len;
					//int res;
					ST_HF_CCU_MGW_ADD_FENJI_REQ * add_fenji_req = (ST_HF_CCU_MGW_ADD_FENJI_REQ *)st_hf_ccu_mgw_pro->body;
					ST_HF_CCU_MGW_ADD_FENJI_ANS * add_fenji_ans = (ST_HF_CCU_MGW_ADD_FENJI_ANS *)st_hf_ccu_mgw_pro->body;

					VERBOSE_OUT(LOG_SYS,"Recv Add fenji req:index - %d,fenji - %ld\n",add_fenji_req->index,add_fenji_req->fenji_num);
					if(add_fenji_req->index < FEIJI_NUM_MAX){
						//sprintf(fenji_group[add_fenji_req->index].name,"%ld",add_fenji_req->fenji_num);
                        snprintf(fenji_group[add_fenji_req->index].name, \
                            sizeof(fenji_group[add_fenji_req->index].name), "%ld",add_fenji_req->fenji_num);
						fenji_group[add_fenji_req->index].set_state = FENJI_SET;
						if(add_fenji_req->index == 0)
							change_config_var(mgw_cfg,"FENJI0",fenji_group[add_fenji_req->index].name);
						else if(add_fenji_req->index == 1)
							change_config_var(mgw_cfg,"FENJI1",fenji_group[add_fenji_req->index].name);
					}
					
					rewrite_config(mgw_cfg);
					
					add_fenji_ans->status = 0x00;
					
					/*����Э��ͷ��Ϣ*/
					st_hf_ccu_mgw_pro->header.info_type= INFO_TYPE_CCU_MGW_ADD_FENJI_ANS_ID;
					st_hf_ccu_mgw_pro->header.len = sizeof(ST_HF_CCU_MGW_ADD_FENJI_ANS);

					memcpy(&addr_dcu,addr_src,sizeof(struct sockaddr_in));
					addr_dcu.sin_port = htons(9050);

					pro->header.dst_addr = 0x10;
					pro->header.src_addr = 0xb0;
					pro->header.data_len = sizeof(ST_HF_CCU_MGW_PRO_HEADER)+sizeof(ST_HF_CCU_MGW_ADD_FENJI_ANS);

					tmp_len = sizeof(ST_HF_MGW_DCU_PRO_HEADER) + pro->header.data_len;
					Board_Data_SendTo_Inc_Addr((uint8 *)&pro, tmp_len, addr_dcu);
				}
				break;

			case INFO_TYPE_CCU_MGW_SUB_FENJI_REQ_ID:
				{
					int tmp_len;
					ST_HF_CCU_MGW_SUB_FENJI_REQ * sub_fenji_req = (ST_HF_CCU_MGW_SUB_FENJI_REQ *)st_hf_ccu_mgw_pro->body;
					ST_HF_CCU_MGW_SUB_FENJI_ANS * sub_fenji_ans = (ST_HF_CCU_MGW_SUB_FENJI_ANS *)st_hf_ccu_mgw_pro->body;

					VERBOSE_OUT(LOG_SYS,"Recv SUB fenji req:index - %d,fenji - %ld\n",sub_fenji_req->index,sub_fenji_req->fenji_num);
					
					if(sub_fenji_req->index < FEIJI_NUM_MAX){
						fenji_group[sub_fenji_req->index].set_state = FENJI_UNSET;
						
						if(sub_fenji_req->index == 0)
							change_config_var(mgw_cfg,"FENJI0","");
						else if(sub_fenji_req->index == 1)
							change_config_var(mgw_cfg,"FENJI1","");
					}
					rewrite_config(mgw_cfg);
					sub_fenji_ans->status = 0x00;
					
					/*����Э��ͷ��Ϣ*/
					st_hf_ccu_mgw_pro->header.info_type= INFO_TYPE_CCU_MGW_SUB_FENJI_ANS_ID;
					st_hf_ccu_mgw_pro->header.len = sizeof(ST_HF_CCU_MGW_SUB_FENJI_ANS);

					memcpy(&addr_dcu,addr_src,sizeof(struct sockaddr_in));
					addr_dcu.sin_port = htons(9050);

					pro->header.dst_addr = 0x10;
					pro->header.src_addr = 0xb0;
					pro->header.data_len = sizeof(ST_HF_CCU_MGW_PRO_HEADER)+sizeof(ST_HF_CCU_MGW_SUB_FENJI_ANS);

					tmp_len = sizeof(ST_HF_MGW_DCU_PRO_HEADER) + pro->header.data_len;
					Board_Data_SendTo_Inc_Addr((uint8 *)&pro, tmp_len, addr_dcu);
				}				
				break;

			case INFO_TYPE_CCU_MGW_DETECT_FENJI_REQ_ID:
				{
					int tmp_len;
					ST_HF_CCU_MGW_DETECT_FENJI_ANS * detect_fenji_ans = (ST_HF_CCU_MGW_DETECT_FENJI_ANS *)st_hf_ccu_mgw_pro->body;

					
					detect_fenji_ans->fenji1 = (fenji_group[0].set_state==FENJI_SET) ? atoi(fenji_group[0].name) : 0;
					detect_fenji_ans->fenji2 = (fenji_group[1].set_state==FENJI_SET) ? atoi(fenji_group[1].name) : 0;
					/*����Э��ͷ��Ϣ*/
					st_hf_ccu_mgw_pro->header.info_type= INFO_TYPE_CCU_MGW_DETECT_FENJI_ANS_ID;
					st_hf_ccu_mgw_pro->header.len = sizeof(ST_HF_CCU_MGW_DETECT_FENJI_ANS);

					memcpy(&addr_dcu,addr_src,sizeof(struct sockaddr_in));
					addr_dcu.sin_port = htons(9050);

					pro->header.dst_addr = 0x10;
					pro->header.src_addr = 0xb0;
					pro->header.data_len = sizeof(ST_HF_CCU_MGW_PRO_HEADER)+sizeof(ST_HF_CCU_MGW_DETECT_FENJI_ANS);

					tmp_len = sizeof(ST_HF_MGW_DCU_PRO_HEADER) + pro->header.data_len;
				}
				break;

			case INFO_TYPE_CCU_MGW_ADD_IP_SLOT_REQ_ID:
				{
					int tmp_len;
					char tmp_str[33];
					int port_index;
					struct in_addr tmp_addr;
					struct sockaddr_in tmp_sck;
					ST_HF_CCU_MGW_ADD_IP_SLOT_REQ * add_ip_slot_req = (ST_HF_CCU_MGW_ADD_IP_SLOT_REQ *)st_hf_ccu_mgw_pro->body;
					ST_HF_CCU_MGW_ADD_IP_SLOT_ANS * add_ip_slot_ans = (ST_HF_CCU_MGW_ADD_IP_SLOT_ANS *)st_hf_ccu_mgw_pro->body;

					port_index = find_port_by_slot(add_ip_slot_req->slot);

					tmp_addr.s_addr = add_ip_slot_req->ip_addr;
					tmp_sck.sin_addr = tmp_addr;

					if(port_index != (-1) \
						&& terminal_group[port_index].bPortType==PORT_TYPE_XIWEI \
						&& mgw_test_ip(add_ip_slot_req->ip_addr)==0 \
						&& find_port_by_ip(&tmp_sck)==(-1)){
						
						snprintf(tmp_str, sizeof(tmp_str), "%d",add_ip_slot_req->slot);

						change_config_var(config_cfg,tmp_str,inc_inet_ntoa(tmp_addr));
						rewrite_config(config_cfg);
						add_ip_slot_ans->status = 0x00;
					}else
						add_ip_slot_ans->status = 0xff;

					
					/*����Э��ͷ��Ϣ*/
					st_hf_ccu_mgw_pro->header.info_type= INFO_TYPE_CCU_MGW_ADD_IP_SLOT_ANS_ID;
					st_hf_ccu_mgw_pro->header.len = sizeof(ST_HF_CCU_MGW_ADD_IP_SLOT_ANS);

					memcpy(&addr_dcu,addr_src,sizeof(struct sockaddr_in));
					addr_dcu.sin_port = htons(9050);

					pro->header.dst_addr = 0x10;
					pro->header.src_addr = 0xb0;
					pro->header.data_len = sizeof(ST_HF_CCU_MGW_PRO_HEADER)+sizeof(ST_HF_CCU_MGW_ADD_IP_SLOT_ANS);

					tmp_len = sizeof(ST_HF_MGW_DCU_PRO_HEADER) + pro->header.data_len;
					Board_Data_SendTo_Inc_Addr((uint8 *)&pro, tmp_len, addr_dcu);
				}
				break;

			case INFO_TYPE_CCU_MGW_MODIFY_IP_SLOT_REQ_ID:
				{
					int tmp_len;
					char tmp_str[33];
					int port_index;
					struct in_addr tmp_addr;
					struct sockaddr_in tmp_sck;
					ST_HF_CCU_MGW_MODIFY_IP_SLOT_REQ * modify_ip_slot_req = (ST_HF_CCU_MGW_MODIFY_IP_SLOT_REQ *)st_hf_ccu_mgw_pro->body;
					ST_HF_CCU_MGW_MODIFY_IP_SLOT_ANS * modify_ip_slot_ans = (ST_HF_CCU_MGW_MODIFY_IP_SLOT_ANS *)st_hf_ccu_mgw_pro->body;

					port_index = find_port_by_slot(modify_ip_slot_req->slot);

					tmp_addr.s_addr = modify_ip_slot_req->ip_addr;
					tmp_sck.sin_addr = tmp_addr;
					
					if(port_index != (-1) \
						&& terminal_group[port_index].bPortType==PORT_TYPE_XIWEI \
						&& mgw_test_ip(modify_ip_slot_req->ip_addr)==0 \
						&& find_port_by_ip(&tmp_sck)==(-1)){
						
						snprintf(tmp_str, sizeof(tmp_str), "%d",modify_ip_slot_req->slot);

						change_config_var(config_cfg,tmp_str,inc_inet_ntoa(tmp_addr));
						rewrite_config(config_cfg);
						modify_ip_slot_ans->status = 0x00;
					}else
						modify_ip_slot_ans->status = 0xff;


					/*����Э��ͷ��Ϣ*/
					st_hf_ccu_mgw_pro->header.info_type= INFO_TYPE_CCU_MGW_MODIFY_IP_SLOT_ANS_ID;
					st_hf_ccu_mgw_pro->header.len = sizeof(ST_HF_CCU_MGW_MODIFY_IP_SLOT_ANS);

					memcpy(&addr_dcu,addr_src,sizeof(struct sockaddr_in));
					addr_dcu.sin_port = htons(9050);

					pro->header.dst_addr = 0x10;
					pro->header.src_addr = 0xb0;
					pro->header.data_len = sizeof(ST_HF_CCU_MGW_PRO_HEADER)+sizeof(ST_HF_CCU_MGW_MODIFY_IP_SLOT_ANS);

					tmp_len = sizeof(ST_HF_MGW_DCU_PRO_HEADER) + pro->header.data_len;
					Board_Data_SendTo_Inc_Addr((uint8 *)&pro, tmp_len, addr_dcu);
				}				
				break;

			case INFO_TYPE_CCU_MGW_SUB_IP_SLOT_REQ_ID:
				{
					int tmp_len;
					char tmp_str[33];
					int port_index;
					struct in_addr tmp_addr;
					ST_HF_CCU_MGW_SUB_IP_SLOT_REQ * sub_ip_slot_req = (ST_HF_CCU_MGW_SUB_IP_SLOT_REQ *)st_hf_ccu_mgw_pro->body;
					ST_HF_CCU_MGW_SUB_IP_SLOT_ANS * sub_ip_slot_ans = (ST_HF_CCU_MGW_SUB_IP_SLOT_ANS *)st_hf_ccu_mgw_pro->body;

					port_index = find_port_by_slot(sub_ip_slot_req->slot);
					
					if(port_index != (-1) && terminal_group[port_index].bPortType == PORT_TYPE_XIWEI){
						snprintf(tmp_str, sizeof(tmp_str), "%d",sub_ip_slot_req->slot);
						tmp_addr.s_addr = 0x00000000;

						change_config_var(config_cfg,tmp_str,inc_inet_ntoa(tmp_addr));
						rewrite_config(config_cfg);
						sub_ip_slot_ans->status = 0x00;
					}else
						sub_ip_slot_ans->status = 0xff;

					/*����Э��ͷ��Ϣ*/
					st_hf_ccu_mgw_pro->header.info_type= INFO_TYPE_CCU_MGW_SUB_IP_SLOT_ANS_ID;
					st_hf_ccu_mgw_pro->header.len = sizeof(ST_HF_CCU_MGW_SUB_IP_SLOT_ANS);

					memcpy(&addr_dcu,addr_src,sizeof(struct sockaddr_in));
					addr_dcu.sin_port = htons(9050);

					pro->header.dst_addr = 0x10;
					pro->header.src_addr = 0xb0;
					pro->header.data_len = sizeof(ST_HF_CCU_MGW_PRO_HEADER)+sizeof(ST_HF_CCU_MGW_SUB_IP_SLOT_ANS);

					tmp_len = sizeof(ST_HF_MGW_DCU_PRO_HEADER) + pro->header.data_len;
					Board_Data_SendTo_Inc_Addr((uint8 *)&pro, tmp_len, addr_dcu);
				}
				break;
			case INFO_TYPE_CCU_MGW_DETECT_IP_SLOT_REQ_ID:
				{
					int tmp_len;
					int i;
					ST_HF_CCU_MGW_DETECT_IP_SLOT_ANS * ip_slot_ans = (ST_HF_CCU_MGW_DETECT_IP_SLOT_ANS *)st_hf_ccu_mgw_pro->body;
					
					ip_slot_ans->terminal_count = XIWEI_NUM_MAX;
					for(i = XIWEI_OFFSET;i<XIWEI_OFFSET+XIWEI_NUM_MAX;i++)
					{
						if(terminal_group[i].bPortType != PORT_TYPE_XIWEI){
							ip_slot_ans->ter_local[i-XIWEI_OFFSET].slot = 0;
							ip_slot_ans->ter_local[i-XIWEI_OFFSET].ip_addr = 0;							
							continue;
						}
						ip_slot_ans->ter_local[i-XIWEI_OFFSET].slot = terminal_group[i].bslot_us;
						ip_slot_ans->ter_local[i-XIWEI_OFFSET].ip_addr = inet_addr(terminal_group[i].rtp_socket.remote_ip);
					}
					st_hf_ccu_mgw_pro->header.info_type = INFO_TYPE_CCU_MGW_DETECT_IP_SLOT_ANS_ID;
					st_hf_ccu_mgw_pro->header.len = sizeof(ST_HF_CCU_MGW_DETECT_IP_SLOT_ANS);

					memcpy(&addr_dcu,addr_src,sizeof(struct sockaddr_in));
					addr_dcu.sin_port = htons(9050);

					pro->header.dst_addr = 0x10;
					pro->header.src_addr = 0xb0;
					pro->header.data_len = sizeof(ST_HF_CCU_MGW_PRO_HEADER)+sizeof(ST_HF_CCU_MGW_DETECT_IP_SLOT_ANS);

					tmp_len = sizeof(ST_HF_MGW_DCU_PRO_HEADER) + pro->header.data_len;
					Board_Data_SendTo_Inc_Addr((uint8 *)&pro, tmp_len, addr_dcu);
				}
				break;

			case INFO_TYPE_CCU_MGW_SELF_CHECK_REQ_ID:
				{
					int tmp_len;
					ST_HF_CCU_MGW_SELF_CHECK_ANS * self_check_ans = (ST_HF_CCU_MGW_SELF_CHECK_ANS *)st_hf_ccu_mgw_pro->body;
					self_check_ans->err_count = 0;
					self_check_ans->err_info = 0;
					self_check_ans->err_index = 0;
					self_check_ans->err_type = 0xff;

					st_hf_ccu_mgw_pro->header.info_type = INFO_TYPE_CCU_MGW_SELF_CHECK_ANS_ID;
					st_hf_ccu_mgw_pro->header.len = sizeof(ST_HF_CCU_MGW_SELF_CHECK_ANS);

					memcpy(&addr_dcu,addr_src,sizeof(struct sockaddr_in));
					addr_dcu.sin_port = htons(9050);

					pro->header.dst_addr = 0x10;
					pro->header.src_addr = 0xb0;
					pro->header.data_len = sizeof(ST_HF_CCU_MGW_PRO_HEADER)+sizeof(ST_HF_CCU_MGW_SELF_CHECK_ANS);

					tmp_len = sizeof(ST_HF_MGW_DCU_PRO_HEADER) + pro->header.data_len;
					Board_Data_SendTo_Inc_Addr((uint8 *)&pro, tmp_len, addr_dcu);
				}
				break;

			case INFO_TYPE_CCU_MGW_SELF_RESET_REQ_ID:
				{
					int tmp_len;
					ST_HF_CCU_MGW_SELF_RESET_ANS * self_reset_ans = (ST_HF_CCU_MGW_SELF_RESET_ANS *)st_hf_ccu_mgw_pro->body;
					self_reset_ans->status = 0x00;
					mgw_sys_exec("cp -f /data/bak/mgw.conf /data");
					mgw_sys_exec("cp -f /data/bak/extension.conf /data");

					st_hf_ccu_mgw_pro->header.info_type = INFO_TYPE_CCU_MGW_SELF_RESET_ANS_ID;
					st_hf_ccu_mgw_pro->header.len = sizeof(ST_HF_CCU_MGW_SELF_RESET_ANS);

					memcpy(&addr_dcu,addr_src,sizeof(struct sockaddr_in));
					addr_dcu.sin_port = htons(9050);

					pro->header.dst_addr = 0x10;
					pro->header.src_addr = 0xb0;
					pro->header.data_len = sizeof(ST_HF_CCU_MGW_PRO_HEADER)+sizeof(ST_HF_CCU_MGW_SELF_RESET_ANS);

					tmp_len = sizeof(ST_HF_MGW_DCU_PRO_HEADER)+pro->header.data_len;
					Board_Data_SendTo_Inc_Addr((uint8 *)&pro, tmp_len, addr_dcu);
				}
				break;

			case INFO_TYPE_CCU_MGW_ADD_TUNNEL_REQ_ID:
				
				break;
				
			case INFO_TYPE_CCU_MGW_SUB_TUNNEL_REQ_ID:
				
				break;

			case INFO_TYPE_CCU_MGW_DETECT_TUNNEL_REQ_ID:
				
				break;

			case INFO_TYPE_CCU_MGW_ADD_DNAT_REQ_ID:
				
				break;

			case INFO_TYPE_CCU_MGW_SUB_DNAT_REQ_ID:
				/* ȡ��DNAT */
				break;

			case INFO_TYPE_CCU_MGW_SUB_DNAT_PORT_REQ_ID:
				/* ȡ��DNAT */
				break;

			case INFO_TYPE_CCU_MGW_DETECT_DNAT_PORT_REQ_ID:
				
				break;

			case INFO_TYPE_CCU_MGW_DETECT_DNAT_IP_REQ_ID:
				
				break;
#endif
			default:
				//VERBOSE_OUT(LOG_SYS,"Unknow CCU_MGW_PROTOCOL ID %.2x!\n",st_hf_ccu_mgw_pro->header.info_type);
				break;
		}
	}

	return 0;
}


int mgw_scu_ZhuanXian_callack(TERMINAL_BASE* terext)
{
	ST_HF_MGW_SCU_PRO st_hf_mgw_scu_pro;
	ST_HF_MGW_SCU_CALLACK_REQ callack_req;
	int tmp_len;

	printf("%s terext->bslot_us = %d\r\n", __func__, terext->bslot_us);
	
	callack_req.id = MGW_SCU_CALLACK_ID;
	callack_req.len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ) - 2;

	#if 0
	callack_req.slot = terext->bslot_us;
	callack_req.call_type = terext->bCallType;
	#else
	callack_req.slot = 4;//�涨ʱ϶4
	callack_req.call_type = 0x01; //ѡ��
	#endif
	
	if(terext->bCallType == EN_CALL_TRUNK)
		callack_req.call_type = EN_SEL_CALL;	

	if(terext->bCommand == MC_RINGING || terext->bCommand == MC_RING)
		callack_req.call_status = EN_JIEXU_SUCCESS;
	else if(terext->bCommand == MC_REJECT)
		callack_req.call_status = EN_JIEXU_FAILURE;
	else
		callack_req.call_status = EN_JIEXU_FAILURE;

	tmp_len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ);
	
	st_hf_mgw_scu_pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	st_hf_mgw_scu_pro.header.version = HF_MGW_SCU_PRO_VER;
	st_hf_mgw_scu_pro.header.info_type = INFO_TYPE_CALL_CTL;
	st_hf_mgw_scu_pro.header.len = sizeof(ST_HF_MGW_SCU_CALLACK_REQ);
	memcpy(st_hf_mgw_scu_pro.body,&callack_req,sizeof(ST_HF_MGW_SCU_CALLACK_REQ));

	tmp_len += sizeof(ST_HF_MGW_SCU_PRO_HEADER);
	
	/*3.send packet*/
	return Board_Sem_SendTo_Inc((uint8*)&st_hf_mgw_scu_pro, tmp_len);
}

/* ý�����ذ��뻰����Э����� 
 1��Modified by lishibing20150923:
 ��Z043����״̬��<������̫���������ն��û���������v003> 7.3.2.6-7Ϊר��ר����ע���ע������Z043�ڷ�ģʽ��ר��ͨ��
 ��̨PRNģʽ����ս��������ʱ,Z018��Z024�����õ�ֱ̨����ʽ������ϯλ���������������8210~8213��50�ӷֱ��ͨ
 ս������ʱ϶�� ����ͨ�����ķ�ʽ��ȱ����:������ͬʱ��ͨ��̨����һ�˲���8210���Է������塣Z043�ڻ�����������������
 ר������(0x8802)����������͸�50����50��͸����716���ר��ͨ�����̡�����ں�������������ս��ר�����������Ϣ��
*/

static  int32 mgw_scu_open_user_msg(void)
{
	ST_HF_MGW_SCU_PRO pro;
	memset(&pro, 0x00, sizeof(pro));
	ST_HF_MGW_SCU_OPEN_MSG *open_msg = (ST_HF_MGW_SCU_OPEN_MSG *)pro.body;

	int tmp_len,i;

	LOG("%s: \r\n", __func__);
		
	pro.header.protocol_type = 0x01;
	pro.header.version = 0x01;
	pro.header.info_type = INFO_TYPE_PARARM_SET;
					
	open_msg->id = MGW_SCU_OPEN_ID;	
	open_msg->mem_count = 10;
	open_msg->len = (open_msg->mem_count * 1) + 1; 
	
	for(i=0;i < open_msg->mem_count;i++)
	{
		open_msg->terminal[i] = terminal_group[i].bslot_us;
	}

	pro.header.len =  open_msg->len + 2;
	tmp_len = sizeof(ST_HF_MGW_SCU_PRO_HEADER) + pro.header.len;
	Board_Sem_SendTo_Inc((uint8*)&pro, tmp_len);

	return DRV_OK;
}

static int Ipc_sem_protocol_parse
(
	ST_HF_MGW_SCU_PRO* pro,
	size_t count)
{									
	if(pro->header.protocol_type != HF_MGW_SCU_PRO_TYPE)
	{
		return DRV_ERR;
	}
	
	if(pro->header.len != (count - sizeof(ST_HF_MGW_SCU_PRO_HEADER)))
	{
		return DRV_ERR;
	}
	
	if(pro->header.len != (pro->body[1]+2))/* �ӳ����ж� */
	{
		return DRV_ERR;
	}

	usleep(100*1000);

	Dbg_Socket_SendToPc(MONITOR_50_I_SEM_BIT, (uint8 *)pro, count);

#if 0
	uint8 *buf = (uint8 *)pro;

	int i = 0;

	printf("recv from 50");	
	for(i = 0; i < count; i++)
	{
		printf("%x:", buf[i]);
	}
	printf("\r\n");	
#endif

	switch(pro->header.info_type)
	{
		case INFO_TYPE_MANAGE:
			if(pro->body[0] == MGW_SCU_LINK_ID)
			{
				ST_HF_MGW_SCU_LINK_MSG *link_msg = (ST_HF_MGW_SCU_LINK_MSG *)pro->body;

				if(FALSE == Param_update.control_online)
				{
					LOG("control_online board online \r\n");
					Param_update.control_online = TRUE;
					Param_update.control_timecnt = time_cnt_100ms;
					if(FALSE == Param_update.ac491_init)
					{
						Ac491_Init();
					}					
				}
				else if(TRUE == Param_update.control_online)
				{
					Param_update.control_timecnt = time_cnt_100ms;
				}

				//LOG("Recv shake hand msg %d!\r\n", link_msg->link_status);
				if(link_msg->link_status == HF_MGW_SCU_LINK_STATUS_FAILED)
				{
					link_msg->link_status = HF_MGW_SCU_LINK_STATUS_OK;

					g_uiShakeHandFlg = HF_MGW_SCU_LINK_STATUS_FAILED; //��¼ʧ��

					/* �ط�������Ϣ,������ Board_Sem_SendTo_Inc() */
					//Board_Sem_SendTo_Inc((uint8*)pro, count);
				}
				else if(link_msg->link_status == HF_MGW_SCU_LINK_STATUS_OK)
				{
					/* �ط�������Ϣ */
					//Board_Sem_SendTo_Inc((uint8*)pro, count);

					/* �����ؼ�¼���ϴ����ֳɹ���־Ϊ"���粻����"���ڱ����յ�
					   ������Ϣ�󣬷��û���ͨ��Ϣ
					*/
					if(HF_MGW_SCU_LINK_STATUS_FAILED == g_uiShakeHandFlg )
					{
						ST_HF_MGW_SCU_OPEN_MSG *open_msg = (ST_HF_MGW_SCU_OPEN_MSG *)pro->body;
						int tmp_len,i;
						
						pro->header.info_type = INFO_TYPE_PARARM_SET;					
						open_msg->id = MGW_SCU_OPEN_ID;	

						/* 
						ע��:��Э��涨��7.3.2.3�û���Ϣע����Ϣ����Ϣ����Ϊ1���ֽڣ����255��
						��ÿ��ע����Ϣ��ÿ���û�ռ16���ֽڣ���˷�����Ϣ����14���û���Ϣ��
						(һ�ο�ͨ����15���û���������Ϣ��ʾ���ȴ���)
						
						���ȿ�ͨ10�����ٿ�ͨ5���� 
						*/
						open_msg->mem_count = 10;

						/* ������Ϣ��ÿ���û���Ӧ���ն˱�ʶռ1���ֽ� */
						open_msg->len = (open_msg->mem_count * 1)+1; 
						
						for(i=0;i < open_msg->mem_count;i++)
						{
							/* 50��ʶ���ն˱�ʶ��1��ʼ��834��¼�ն˱�ʶ��0��ʼ(�������±��Ӧ) */
							open_msg->terminal[i] = terminal_group[i].bslot_us;
						}
						tmp_len = sizeof(ST_HF_MGW_SCU_PRO_HEADER) + open_msg->len + 2;
						pro->header.len = tmp_len - sizeof(ST_HF_MGW_SCU_PRO_HEADER);
						
						Board_Sem_SendTo_Inc((uint8*)pro, tmp_len);

						open_msg->mem_count = 6;
						open_msg->len = open_msg->mem_count*1+1;
						
						for(i=0;i<open_msg->mem_count;i++)
						{
							open_msg->terminal[i] = terminal_group[10+i].bslot_us;
						}
						tmp_len = sizeof(ST_HF_MGW_SCU_PRO_HEADER) + open_msg->len + 2;
						pro->header.len = tmp_len-sizeof(ST_HF_MGW_SCU_PRO_HEADER);

						Board_Sem_SendTo_Inc((uint8*)pro, tmp_len);
						
						g_uiShakeHandFlg = HF_MGW_SCU_LINK_STATUS_OK;

						VERBOSE_OUT(LOG_SYS,"Send user open msg OK!\r\n");
					}
				}
			}
			break;
		case INFO_TYPE_PARARM_SET:
			switch(pro->body[0])
			{
            
				case MGW_SCU_CLOSE_ANS_ID:
					VERBOSE_OUT(LOG_SYS,"recv close ans msg!\n");
					break;
				case MGW_SCU_CONFERENCE_ASSIGN_REQ_ID:
					{
						//VERBOSE_OUT(LOG_SYS,"recv CONFERENCE_ASSIGN msg!\n");
					}
					break;
				case MGW_SCU_USER_ASSIGN_REQ_ID:
					{
						/*���ܵ��û�ע����Ϣ�󣬽���Ϣ���͵����û��ն�*/
                         #if 0
                        	int j = 0;

                        	printf("receive from 50: ");
                        	for(j = 0; j < pro->header.len; j++)
                        	{
                        		printf("%x:", pro->body[j]);
                        	}
                        	printf("\r\n");
                        #endif

						int i;
						ST_HF_MGW_SCU_PRO user_assign;
						ST_HF_MGW_SCU_USER_ASSIGN_ANS * ans = NULL;
						VERBOSE_OUT(LOG_SYS,"recv user assign reg msg!\n");
						ST_HF_MGW_SCU_USER_ASSIGN_REQ * req = (ST_HF_MGW_SCU_USER_ASSIGN_REQ *)pro->body;
						VERBOSE_OUT(LOG_SYS,"user assign mem_count:%d\n",req->mem_count);

						if((req->mem_count * sizeof(req->terminal[0]) + 1) != req->len)
						{
							VERBOSE_OUT(LOG_SYS,"recv user assign reg msg len error:%d, %d!\n",\
											(req->mem_count * sizeof(req->terminal[0]) + 1), req->len);
							break;
						}

						ans = (ST_HF_MGW_SCU_USER_ASSIGN_ANS *)user_assign.body;
						ans->id = MGW_SCU_USER_ASSIGN_ANS_ID;

						/* Ӧ����Ϣ��ÿ���ն�ռ2���ֽ� */
						ans->len = req->mem_count *2+1;
						ans->mem_count = req->mem_count;
						
						for(i=0;i<req->mem_count;i++)
						{
							ans->terminal[i].terminal_id = req->terminal[i].terminal_id;
							ans->terminal[i].ans = req->terminal[i].ans;
						}

						user_assign.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
						user_assign.header.version = HF_MGW_SCU_PRO_VER;
						user_assign.header.info_type = INFO_TYPE_PARARM_SET;
						user_assign.header.len = 2+ans->len;

						/*=====assign user num to each terminal=====*/
						{
							int tmp_port;
							for(i=0;i < req->mem_count;i++)
							{
								/* 50���ն˱�ʶ��1��ʼ��834��1��ʼ */
								tmp_port = find_port_by_slot(req->terminal[i].terminal_id);
								if(DRV_ERR == tmp_port)
								{
									VERBOSE_OUT(LOG_SYS,"Can't find port by slot:%d\n", \
													req->terminal[i].terminal_id);
									continue;
								}

								/* �ɹ�Ӧ�� */
								if(req->terminal[i].ans == 0x00)
								{
									bcd2phone((char *)terminal_group[tmp_port].name, \
												(char *)req->terminal[i].user_num, MAXPHONENUM);

									if((req->terminal[i].num_len > 0) \
									    && (req->terminal[i].num_len != Param_update.num_len))
									{
										Param_update.num_len = req->terminal[i].num_len;
										if(Param_update.num_len > 0)
										{
											VERBOSE_OUT(LOG_SYS,"user phone num len:%d\n", Param_update.num_len);
										}    
									}
		
									//VERBOSE_OUT(LOG_SYS," phone num %s\n", (char *)req->terminal[i].user_num);
												
									if(0 == tmp_port)
									{
										memcpy(fenji_group[0].name, terminal_group[tmp_port].name, Param_update.num_len);
										fenji_group[0].set_state = FENJI_SET; 
									}
									else if(3 == tmp_port)
									{
										memcpy(fenji_group[0].name, terminal_group[tmp_port].name, Param_update.num_len);
										fenji_group[0].set_state = FENJI_SET; 
									}
									

									//DEBUG_OUT("term%d - %s\n",tmp_port, terminal_group[tmp_port].name);

								#if 0
									if(string_ncmp(terminal_group[tmp_port].name,"8000",4) && 
									  string_ncmp(terminal_group[tmp_port].name,"0000",4) && 
									  string_ncmp(get_config_var_str(config_cfg,"DEP"),terminal_group[tmp_port].name,4)
									  )
									{
										//char *tmp_dep = strdup(terminal_group[tmp_port].name);
										char *tmp_dep = string_dup((char *)terminal_group[tmp_port].name);
										tmp_dep[4] = '\0';
										change_config_var(config_cfg,"DEP",tmp_dep);
										VERBOSE_OUT(LOG_SYS,"Change local dep to %s\n",tmp_dep);
										rewrite_config(config_cfg);
										free(tmp_dep);
									}
								#endif

								#if 0
									/*TRUNK CISHI*/
									if(tmp_port == LOCALTER_OFFSET+1 || tmp_port == LOCALTER_OFFSET+2)
									{
										if((0 == string_cmp(terminal_group[tmp_port].name, "8000000")) \
											|| (0 == string_cmp(terminal_group[tmp_port].name, "8000001")))
										{
											terminal_group[tmp_port].bPortType = PORT_TYPE_TRK;
										}
										else
										{
											terminal_group[tmp_port].bPortType = PORT_TYPE_CISHI;
										}
									}
								#endif

									terminal_group[tmp_port].ans = req->terminal[i].ans;
									terminal_group[tmp_port].user_num_len = req->terminal[i].num_len;

									memcpy(terminal_group[tmp_port].user_num, \
												(char *)req->terminal[i].user_num, (req->terminal[i].num_len + 1)/2);

									char b_ph[MAXNUMLEN] = {0};
									Sip_BoardPhToSipPh((char *)terminal_group[tmp_port].user_num, (req->terminal[i].num_len + 1)/2, b_ph);
									VERBOSE_OUT(LOG_SYS,"seat%d, phonenum %s\r\n", tmp_port, b_ph);												
									
									terminal_group[tmp_port].conference_enable = req->terminal[i].conference_enable;
									
									if(terminal_group[tmp_port].conference_enable > 1)
									{
										terminal_group[tmp_port].conference_enable = 1;
									}
										
									terminal_group[tmp_port].priority = req->terminal[i].priority;
									if(terminal_group[tmp_port].priority == 0 || terminal_group[tmp_port].priority > 2)
									{
										terminal_group[tmp_port].priority = 1;
									}

									terminal_group[tmp_port].encoder = req->terminal[i].encoder;

									if(terminal_group[tmp_port].bCommand == MC_INACTIVE)
									{
										terminal_group[tmp_port].bCommand = MC_ACTIVE;
										inc_sched_add(con,10+i*10,mgw_scu_open,&terminal_group[tmp_port]);
									}
									
									if(terminal_group[tmp_port].bPortType == PORT_TYPE_XIWEI)
									{
										terminal_group[tmp_port].sched_assign_id = inc_sched_add(con,30000,\
												mgw_ext_assign_user_number,&terminal_group[tmp_port]);
									}

									if(1 == terminal_group[tmp_port].sw_phone_flg)
									{
										Sw_Phone_User_Info_Inject(tmp_port, terminal_group[tmp_port].ipaddr);
									}
								}
								else
								{	
									if(0 == tmp_port)
									{
										memset(fenji_group[0].name, 0x00, 10);
										fenji_group[0].set_state = FENJI_UNSET; 
									}
									else if(3 == tmp_port)
									{
										memset(fenji_group[0].name, 0x00, 10);
										fenji_group[0].set_state = FENJI_UNSET; 
									}
									
									DEBUG_OUT("term%d - failed!\n",tmp_port);
								}
							}
							
							/* Added by lishibing 20151005,�����ڷ�ģʽ��PRN K��ר���Զ�ע��*/
							if(WORK_MODE_PAOBING == Param_update.workmode)
							{
								Board_Mng_ZhuanXianInit();
							}
						}
						
						Board_Sem_SendTo_Inc((uint8*)&user_assign, \
												(sizeof(ST_HF_MGW_SCU_PRO_HEADER)+user_assign.header.len));
					}
					break;

				case MGW_SCU_USER_FREE_REQ_ID:
					{
						int i;
						int port;
						ST_HF_MGW_SCU_PRO	user_free;
						ST_HF_MGW_SCU_USER_FREE_REQ * req = (ST_HF_MGW_SCU_USER_FREE_REQ *)pro->body;
						ST_HF_MGW_SCU_USER_FREE_ANS * ans = NULL;

						ans = (ST_HF_MGW_SCU_USER_FREE_ANS *)user_free.body;
						ans->id = MGW_SCU_USER_FREE_ANS_ID;
						ans->len = req->mem_count*sizeof(ans->terminal[0]);
						ans->mem_count = req->mem_count;

						VERBOSE_OUT(LOG_SYS,"Recv User Free Msg!\n");
						
						for(i=0;i<req->mem_count;i++)
						{
							ans->terminal[i].terminal_id = req->terminal[i].terminal_id;
							VERBOSE_OUT(LOG_SYS,"Recv Scu User_Free %d Msg!\n",ans->terminal[i].terminal_id);

							port = find_port_by_slot(ans->terminal[i].terminal_id);
							if(port < 0)
							{						
								ans->terminal[i].ans = 0xff;
								VERBOSE_OUT(LOG_SYS,"Can't Find terminal id %d\n",ans->terminal[i].terminal_id);
							}
							else if(terminal_group[port].bCommand != MC_IDLE)
							{
								ans->terminal[i].ans = 0xff;
								VERBOSE_OUT(LOG_SYS,"Can't Free terminal %d cause in use\n",port);
							}
							else
							{
								ans->terminal[i].ans = 0x00;
								terminal_group[port].bCommand = MC_INACTIVE;
							}
						}

						user_free.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
						user_free.header.version = HF_MGW_SCU_PRO_VER;
						user_free.header.info_type = INFO_TYPE_PARARM_SET;
						user_free.header.len = 3+ans->len;

						Board_Sem_SendTo_Inc((uint8*)&user_free, \
													sizeof(ST_HF_MGW_SCU_PRO_HEADER)+ user_free.header.len);
					}
					break;
				case MGW_SCU_ZHUANXIAN_REQ_ID:
#if 0		
					ST_HF_MGW_SCU_PRO msg;	
					int tmp_port = 0;
					
					memset(&msg, 0x00, sizeof(msg));
					
					msg.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
					msg.header.version = HF_MGW_SCU_PRO_VER;
					msg.header.info_type = INFO_TYPE_PARARM_SET;
					msg.header.len = 0x04;

					msg.body[0] = MGW_SCU_ZHUANXIAN_REQ_ACK_ID;
					msg.body[1] = 0x02;
					msg.body[2] = pro->body[2];
						
					if((13 == pro->body[1])&&(pro->body[3] <= 3))
					{
						tmp_port = find_port_by_slot(pro->body[2]);	
						terminal_group[tmp_port].zhuanxian_type = pro->body[3];
						terminal_group[tmp_port].zhuanxian_flg = 1;
						memcpy(terminal_group[tmp_port].zhuanxian_num, pro->body[5], pro->body[4]);

						terminal_group[tmp_port].zhuanxian_status = ZHUANXIAN_INIT;
	
						msg.body[3] = 0x00;
						printf("zhuanxian reg chan %d, type %d, num %s\r\n", tmp_port, \
							terminal_group[tmp_port].zhuanxian_type, terminal_group[tmp_port].zhuanxian_num);
					}
					else
					{
						msg.body[3] = 0xff;
					}

					Board_Sem_SendTo_Inc((uint8*)&msg, \
											sizeof(ST_HF_MGW_SCU_PRO_HEADER)+ msg.header.len);	
#endif												
					break;
				case MGW_SCU_ZHUANXIAN_FREE_ID:	
#if 0
					//int tmp_port = 0;
					memset(&msg, 0x00, sizeof(msg));
					
					msg.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
					msg.header.version = HF_MGW_SCU_PRO_VER;
					msg.header.info_type = INFO_TYPE_PARARM_SET;
					msg.header.len = 0x04;

					msg.body[0] = MGW_SCU_ZHUANXIAN_REQ_ACK_ID;
					msg.body[1] = 0x02;
					msg.body[2] = pro->body[2];
						
					if(1 == pro->body[1])
					{
						tmp_port = find_port_by_slot(pro->body[2]);	
						terminal_group[tmp_port].zhuanxian_type = 0;
						terminal_group[tmp_port].zhuanxian_flg = NETWARE_ZXCREATE_NOTOK;
						memset(terminal_group[tmp_port].zhuanxian_num, 0x00, 15);

						if(terminal_group[tmp_port].bPortType == PORT_TYPE_PHONE)
						{
							terminal_group[tmp_port].bPortState = MGW_PHONE_INIT_STATE;
						}
						else if(terminal_group[tmp_port].bPortType == PORT_TYPE_XIWEI)
						{
							terminal_group[tmp_port].bPortState = MGW_XIWEI_INIT_STATE;
						}

						printf("zhuanxian free chan %d\r\n", tmp_port);
	
						msg.body[3] = 0x00;
					}
					else
					{
						msg.body[3] = 0xff;
					}

					Board_Sem_SendTo_Inc((uint8*)&msg, \
												sizeof(ST_HF_MGW_SCU_PRO_HEADER)+ msg.header.len);	
#endif												
					break;						
				default:
					break;
			}
			break;
		case INFO_TYPE_CALL_CTL:
			switch(pro->body[0])
			{
				case MGW_SCU_CALL_ID:
					{
						int wTemPort;
						ST_HF_MGW_SCU_CALL_EVENT *call_event = (ST_HF_MGW_SCU_CALL_EVENT *)pro->body;

						wTemPort = find_port_by_slot(call_event->slot);
						//LOG("lsb: rev 50 call req slot %d \r\n", wTemPort);
						
						if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
						{
							VERBOSE_OUT(LOG_SYS,"CAN'T FIND PORT\n");
							break;
						}
						else
						{
							VERBOSE_OUT(LOG_SYS,"FIND %d,RECV SCU CALL %d \n",call_event->slot,wTemPort);
						}

						if(terminal_group[wTemPort].bPortType == PORT_TYPE_HEADSET)
						{
							terminal_group[wTemPort].bCommand = MC_RING;
							break;
						}
						else if(terminal_group[wTemPort].bPortType != PORT_TYPE_TRK)
						{
							if(WORK_MODE_PAOBING == Param_update.workmode)
							{
								if(NETWARE_ZXCREATE_OK == terminal_group[wTemPort].pre_zhuanxian_flg)
								{
									printf("lsb: Get port %d ZW zhuanxian request.\r\n", wTemPort);
									terminal_group[wTemPort].zhuanxian_status = ZHUANXIAN_INIT;
									terminal_group[wTemPort].zhuanxian_flg = NETWARE_ZXCREATE_OK;
									if(PORT_TYPE_PHONE == terminal_group[wTemPort].bPortType)
									{
										terminal_group[wTemPort].bPortState = MGW_PHONE_INIT_STATE;
									}
									else if(PORT_TYPE_SW_PHONE == terminal_group[wTemPort].bPortType)
									{
										terminal_group[wTemPort].bPortState = MGW_SW_PHONE_INIT_STATE;
									}
								}
								else 
								{
									terminal_group[wTemPort].bCommand = MC_RING;
								}
							}
							else if(WORK_MODE_ZHUANGJIA == Param_update.workmode)
							{
								terminal_group[wTemPort].bCommand = MC_RING;
							}

							terminal_group[wTemPort].bCallType = call_event->call_type;
							bcd2phone((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
								(char *)call_event->callee_num,call_event->phone_len);
							terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = call_event->phone_len;
							terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;							
						}
						else
						{
							int wConnectPort;
							terminal_group[wTemPort].bCommand = MC_USETRK;
							terminal_group[wTemPort].bCallType = call_event->call_type;
							bcd2phone((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
								(char *)call_event->callee_num,call_event->phone_len);
							
							wConnectPort = find_port_by_name((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue);
							if(wConnectPort<0)
							{
								VERBOSE_OUT(LOG_SYS,"Can't find connect Port\n");
							}
							else
							{
								terminal_group[wTemPort].wConnectPort = wConnectPort;
								terminal_group[wConnectPort].wConnectPort = wTemPort;
								VERBOSE_OUT(LOG_SYS,"Find Connect between %d<-->%d\n",wTemPort,wConnectPort);
							}
							VERBOSE_OUT(LOG_SYS,"RECV USETRK MSG\n");
						}
					}
					break;
				case MGW_SCU_CALLACK_ID:
					{
						int wTemPort;
						ST_HF_MGW_SCU_CALLACK_EVENT *callack_event = (ST_HF_MGW_SCU_CALLACK_EVENT *)pro->body;
						wTemPort = find_port_by_slot(callack_event->slot);
						
						LOG("rev call ack slot %d \r\n", wTemPort);
						
						if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
						{
							VERBOSE_OUT(LOG_SYS,"CAN'T FIND PORT\n");
							break;
						}

						if(terminal_group[wTemPort].bPortType == PORT_TYPE_HEADSET)
						{
							terminal_group[wTemPort].bCommand = MC_RT_TONE;
							break;
						}

						//printf("call status = 0x%x\r\n", callack_event->call_status);
						if(callack_event->call_status == EN_JIEXU_SUCCESS)
						{
							terminal_group[wTemPort].bCommand = MC_RT_TONE;
							
							/*fix radio hf call direct --swf 20140107*/
							if((terminal_group[wTemPort].bCallType == EN_CALL_Z) || 
							  (terminal_group[wTemPort].bCallType == EN_CALL_RADIO))
							{
								VERBOSE_OUT(LOG_SYS,"term%d direct call hf",wTemPort);
								terminal_group[wTemPort].bCommand = MC_TALK;
							}
						}
						else if(callack_event->call_status == EN_JIEXU_FAILURE)
						{
							terminal_group[wTemPort].bCommand = MC_REJECT;
						}
						else if(callack_event->call_status == EN_JIEXU_NONUM)
						{
							terminal_group[wTemPort].bCommand = MC_NONUM;
						}	
						else if(callack_event->call_status == EN_JIEXU_NUMERROR)
						{
							terminal_group[wTemPort].bCommand = MC_NUMERROR;
						}							
						
					}
					break;
				case MGW_SCU_CONNECT_ID:
					{
						int wTemPort;
						ST_HF_MGW_SCU_CONNECT_EVENT *connect_event = (ST_HF_MGW_SCU_CONNECT_EVENT *)pro->body;

						wTemPort = find_port_by_slot(connect_event->slot);					
						//wTemPort = connect_event->slot - AC491_TIMESLOT_MAP;
						LOG("rev call connnet slot %d \r\n",wTemPort);

						if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
						{
							VERBOSE_OUT(LOG_SYS,"CAN'T FIND PORT\n");
							break;
						}

						if(WORK_MODE_ZHUANGJIA == Param_update.workmode)
						{
							/* ר��*/
							if(0x04 == connect_event->call_type)
							{
								terminal_group[wTemPort].bCallType = 0x04;
								terminal_group[wTemPort].zhuanxian_flg = NETWARE_ZXCREATE_OK;
								printf("zhuanxian reg slot %d, reg \r\n", connect_event->slot);

								if(PORT_TYPE_PHONE == terminal_group[wTemPort].bPortType)
								{
									terminal_group[wTemPort].bPortState = MGW_PHONE_INIT_STATE;
								}
								else if(PORT_TYPE_SW_PHONE == terminal_group[wTemPort].bPortType)
								{
									terminal_group[wTemPort].bPortState = MGW_SW_PHONE_INIT_STATE;
								}
								
								break;
							}
						}

						if(terminal_group[wTemPort].bPortType == PORT_TYPE_HEADSET)
						{
							terminal_group[wTemPort].bCommand = MC_TALK;
							break;
						}

						terminal_group[wTemPort].bCallType = connect_event->call_type;

						terminal_group[wTemPort].bCommand = MC_TALK;
						bcd2phone((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
							(char *)connect_event->caller_num,connect_event->phone_len);
						terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = connect_event->phone_len;
						terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;	
						
					}
					break;
				case MGW_SCU_RELEASE_ID:
					{
						int wTemPort;
						ST_HF_MGW_SCU_RELEASE_EVENT *rel_event = (ST_HF_MGW_SCU_RELEASE_EVENT *)pro->body;

						wTemPort = find_port_by_slot(rel_event->slot);					
						
						LOG("rev release req slot %d \r\n", wTemPort);
										
						if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
						{
							VERBOSE_OUT(LOG_SYS,"CAN'T FIND PORT\n");
							break;
						}

						if(NETWARE_ZXCREATE_OK == terminal_group[wTemPort].zhuanxian_flg)
						{
							terminal_group[wTemPort].bCallType = 0x00;
							terminal_group[wTemPort].zhuanxian_type = 0;
							memset(terminal_group[wTemPort].zhuanxian_num, 0x00, 15);
							
							//if(WORK_MODE_ZHUANGJIA == Param_update.workmode)
							{
								terminal_group[wTemPort].zhuanxian_flg = NETWARE_ZXCREATE_NOTOK;
							}

							if(terminal_group[wTemPort].bPortType == PORT_TYPE_PHONE)
							{
								terminal_group[wTemPort].bPortState = MGW_PHONE_INIT_STATE;
							}
							else if(terminal_group[wTemPort].bPortType == PORT_TYPE_SW_PHONE)
							{
								terminal_group[wTemPort].bPortState = MGW_SW_PHONE_INIT_STATE;
							}

							printf("zhuanxian free slot %d\r\n", rel_event->slot);

							break;
						}

						if(terminal_group[wTemPort].bPortType == PORT_TYPE_HEADSET)
						{
							terminal_group[wTemPort].bCommand = MC_HANG;
							break;
						}

						terminal_group[wTemPort].bCommand = MC_HANG;

						terminal_group[wTemPort].bCallType = rel_event->call_type;
						terminal_group[wTemPort].rel_reason = rel_event->rel_reason;
						
						if(rel_event->rel_reason == 0xff || rel_event->rel_reason == 0x00)
						{
							terminal_group[wTemPort].rel_reason = MGW_RELEASE_REASON_DEVICEFAULT;
						}
						
					}
					break;
				case MGW_SCU_RELEASE_CNF_ID:
					{
						int wTemPort;
						ST_HF_MGW_SCU_RELEASE_CNF_EVENT *rel_cnf_event = (ST_HF_MGW_SCU_RELEASE_CNF_EVENT *)pro->body;

						wTemPort = find_port_by_slot(rel_cnf_event->slot);
						
						LOG("rev release req cnf slot %d \r\n", wTemPort);
												
						if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
						{
							VERBOSE_OUT(LOG_SYS,"CAN'T FIND PORT\n");
							break;
						}

						if(terminal_group[wTemPort].bPortType == PORT_TYPE_HEADSET)
						{
							terminal_group[wTemPort].bCommand = MC_HANG;
							break;
						}						
						//mgw_ext_release_cnf_notify(&terminal_group[wTemPort]);
					}
					break;
				case MGW_SCU_PTT_ID:
					{
						int wTemPort;

						wTemPort = find_port_by_slot(pro->body[2]);
						if(wTemPort < 0)
						{
                            			return DRV_ERR;
						}

						if(PORT_TYPE_TRK == terminal_group[wTemPort].bPortType)
						{
							if((terminal_group[wTemPort].wConnectPort < 0) || (terminal_group[wTemPort].wConnectPort >= USER_NUM_MAX))
							{
								ERR("connect port error %d\r\n", terminal_group[wTemPort].wConnectPort);
								return DRV_ERR;
							}
							
							if(PORT_TYPE_SW_PHONE == terminal_group[terminal_group[wTemPort].wConnectPort].bPortType)
							{			
								if(0x00 == pro->body[3]) /*up*/
								{
									terminal_group[wTemPort].phone_queue_s.bPhoneQueue[0] = '3';
									terminal_group[wTemPort].phone_queue_s.bPhoneQueue[1] = '#';
									terminal_group[wTemPort].phone_queue_s.bPhoneQueueHead  = 2;
									terminal_group[wTemPort].phone_queue_s.bPhoneQueueTail = 0;	
									terminal_group[wTemPort].phone_queue_s.bFinish = 1;	
								}
								else if(0x01 == pro->body[3]) /*down*/
								{
									terminal_group[wTemPort].phone_queue_s.bPhoneQueue[0] = '2';
									terminal_group[wTemPort].phone_queue_s.bPhoneQueue[1] = '*';
									terminal_group[wTemPort].phone_queue_s.bPhoneQueueHead  = 2;
									terminal_group[wTemPort].phone_queue_s.bPhoneQueueTail = 0;	
									terminal_group[wTemPort].phone_queue_s.bFinish = 1;	
								}
							}
						}	

						if(NETWARE_ZXCREATE_OK == terminal_group[wTemPort].zhuanxian_flg)
						{
							if(0x00 == pro->body[3])
							{
								terminal_group[wTemPort].zhuanxian_romote_ppt = MGW_PTT_UP;
								if(terminal_group[wTemPort].bPortType == PORT_TYPE_PHONE)
								{
									Gpio_Ring_Set(terminal_group[wTemPort].wPort, 0);
								}
							}
							else if(0x01 == pro->body[3])
							{
								terminal_group[wTemPort].zhuanxian_romote_ppt = MGW_PTT_DOWN;
								if(terminal_group[wTemPort].bPortType == PORT_TYPE_PHONE)
								{
									Gpio_Ring_Set(terminal_group[wTemPort].wPort, 1);
								}
							}
						}
					}				
					break;
				case MGW_SCU_NUM_TRANS_ID:
					{
						int wTemPort;
						ST_HF_MGW_SCU_NUM_TRANS_MSG * num_trans = (ST_HF_MGW_SCU_NUM_TRANS_MSG *)pro->body;

						wTemPort = find_port_by_slot(num_trans->slot);
						
						LOG("rev number trans msg slot %d \r\n", wTemPort);
											
						if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
						{
							VERBOSE_OUT(LOG_SYS,"CAN'T FIND PORT\n");
							break;
						}
						bcd2phone((char *)terminal_group[wTemPort].phone_queue_s.bPhoneQueue, \
							(char *)num_trans->trans_phone,num_trans->phone_len);
						terminal_group[wTemPort].phone_queue_s.bPhoneQueueHead = num_trans->phone_len;
						terminal_group[wTemPort].phone_queue_s.bPhoneQueueTail = 0;
						terminal_group[wTemPort].phone_queue_s.bFinish = 1;
						VERBOSE_OUT(LOG_SYS,"Recv TRK%d Num Trans: %s\n",wTemPort, \
							terminal_group[wTemPort].phone_queue_s.bPhoneQueue);						
					}
					break;
				case MGW_SCU_CALL_TARNS_ID:
					{
						int wTemPort = 0;
						if(2 == pro->body[1])
						{
							wTemPort = find_port_by_slot(pro->body[2]);
		            				if(wTemPort < 0)
		            				{
		                               		ERR("%s: port error %d\r\n", __func__, wTemPort);
		                                		return DRV_ERR;
		            				}		
            				
							if(NETWARE_ZXCREATE_OK == terminal_group[wTemPort].zhuanxian_flg)
							{
								if(0 == pro->body[3])/*�һ�*/
								{
									printf("zhuanxian hangup\r\n");
									if((terminal_group[wTemPort].bPortType == PORT_TYPE_SW_PHONE)\
										&&(terminal_group[wTemPort].bCommand != MC_HANG)\
										&&(1 == terminal_group[wTemPort].talk_flg))
									{
										mgw_sw_phone_release_request(&terminal_group[wTemPort]);
									}
									
									terminal_group[wTemPort].bCommand = MC_HANG;
								}
								else if(1 == pro->body[3])/*ժ��*/
								{
									printf("zhuanxian handoff %d, %d, %d\r\n", terminal_group[wTemPort].bPortType, \
										terminal_group[wTemPort].bCommand, terminal_group[wTemPort].talk_flg);
									if((terminal_group[wTemPort].bPortType == PORT_TYPE_SW_PHONE)\
										&&(terminal_group[wTemPort].bCommand != MC_TALK)\
										&&(0 == terminal_group[wTemPort].talk_flg))
									{
										mgw_sw_phone_call_request(&terminal_group[wTemPort]);
									}
									
									terminal_group[wTemPort].bCommand = MC_TALK;
								}
							}

						}
					}
					break;
				case MGW_SCU_ZW_ZHUANXIAN_ID:
					{
#if 0					
						int wTemPort = 0;
						printf("lsb: Get ZW Zhuanxian hook msg(%d)\r\n",  pro->body[3]);

						if(2 == pro->body[1]) //����Ϊ2
						{
							wTemPort = find_port_by_slot(pro->body[2]);
							if(NETWARE_ZXCREATE_OK == terminal_group[wTemPort].zhuanxian_flg)
							{
								if(0 == pro->body[3])/*�һ�*/
								{
									printf("lsb: %d zhuanxian hangup\r\n", wTemPort);
									terminal_group[wTemPort].bCommand = MC_HANG;
								}
								else if(1 == pro->body[3])/* ժ�� */
								{
									printf("lsb: %d zhuanxian hangoff\r\n", wTemPort);
									terminal_group[wTemPort].bCommand = MC_TALK;
								}
							}

						}
#endif						
					}
				}
				break;		
		default:
			VERBOSE_OUT(LOG_SYS,"Unknow Protocol Info_type(0x%x)!\n", pro->header.info_type);
			break;
		}

	return 0;
}

void protocol_parse(void *pro,size_t count,struct sockaddr_in *addr_src)
{
	/* �ӵ�ַ�з���Э��Ľ������� */
#if 0
	int i = 0;
	uint8 *tmp = (uint8 *)pro;
	printf("%s:\r\n", __func__);

	for(i = 0; i < count; i++)
	{
		printf("%x:", tmp[i]);
	}
	printf("\r\n");
#endif
	
	switch(find_net(addr_src))
	{
		case 0:		/* ϯλͨ��: 10.x.x.x */
			seat_sem_protocol_parse(pro,count,addr_src);

			//hf_mgw_scu_js_protocol_parse(pro,count,addr_src);
			break;
		case 2:		/* �������ͨ��IP:192.168.9.x,��仰��ͨ��IP: 192.168.254.x */
			Ipc_dat_protocol_parse(pro,count,addr_src);
			Ipc_sem_protocol_parse(pro,count);
			seat_sem_protocol_parse(pro,count,addr_src);

			//hf_mgw_scu_js_protocol_parse(pro,count,addr_src);
			break;
		case 3:
			Ipc_dat_protocol_parse(pro,count,addr_src);
			break;
		default:	/*δ֪��ַ��*/
			/*
			VERBOSE_OUT(LOG_SYS,"Unknow net from %s:%d!\n",inc_inet_ntoa(addr_src->sin_addr), \
				addr_src->sin_port);
			*/
			seat_sem_protocol_parse(pro,count,addr_src);	
			break;
	}

	return;
}


int do_show_route(cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	struct JS_ROUTE_TABLE_T * tmp_js_route_table_t;
    
    if((0 == argc)||(NULL == argv)||(NULL == cmdtp))
    {
        ERR("%s: param error\r\n", __func__);
        return DRV_ERR;
    }
    
	DEBUG_OUT("list:\n");
	INCOBJ_CONTAINER_TRAVERSE(&js_user_list,1,do{ \
		INCOBJ_RDLOCK(iterator); \
		DEBUG_OUT("%.8x | %.8x--->",iterator->hf_addr,iterator->js_ip_addr); \
		INCOBJ_UNLOCK(iterator); \
		}while(0));
	DEBUG_OUT("nul\n");
	
#if 1
	DEBUG_OUT("table:\n");

	INC_LIST_TRAVERSE_SAFE_BEGIN(&js_route_user_table,tmp_js_route_table_t,list)
	{
		typeof(tmp_js_route_table_t->head) iterator = NULL;
		typeof(tmp_js_route_table_t->head) next = NULL;
		DEBUG_OUT("mask: %.8x: \n",tmp_js_route_table_t->mask);
		INCOBJ_CONTAINER_RDLOCK(tmp_js_route_table_t);
		next = tmp_js_route_table_t->head;
		//while(iterator = next){
		for(;NULL != next; iterator = next)
		{
			next = iterator->next[0];
			DEBUG_OUT("dst:%.8x,next:%.8x,cost:%.4x,dep:%s\n",
				iterator->dst_hf_addr,iterator->next_hop,iterator->cost,iterator->dep_num);
		}
		INCOBJ_CONTAINER_UNLOCK(tmp_js_route_table_t);
	}
	INC_LIST_TRAVERSE_SAFE_END
#endif
	return 0;
}


int do_submit_del_route(cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	ST_HF_MGW_DCU_PRO route_pro;
	ST_HF_MGW_DCU_ROUTE_MSG * msg;
	struct JS_ROUTE_TABLE_T * tmp_js_route_table_t;
	int i = 0;
	struct sockaddr_in addr_dst;
	
	route_pro.header.baowen_type = BAOWEN_TYPE_MGW_DCU;
	route_pro.header.dst_addr = 0x10;
	route_pro.header.src_addr = 0xb0;
	route_pro.header.info_type = INT_FRAME_TYPE_CMD_ROUTE;
	
	
	msg = (ST_HF_MGW_DCU_ROUTE_MSG *)route_pro.body;
	msg->id = MGW_DCU_ROUTE_ID;
	msg->route_index = 1;
	INC_LIST_TRAVERSE(&js_route_user_table,tmp_js_route_table_t,list)
	{
		msg->route[i].hf_addr = tmp_js_route_table_t->mask;
		msg->route[i].next_hop = tmp_js_route_table_t->mask;
		msg->route[i].module_addr = 0xb0;
		msg->route[i].cost = 100;
		i++;
	}
	msg->len = 11*i+4;
	msg->route_count = 0;

	route_pro.header.data_len = msg->len+3;
	
	VERBOSE_OUT(LOG_SYS,"Send %d Del Route msg!\n",i);

	addr_dst.sin_family = AF_INET;
	addr_dst.sin_port = htons(HF_MGW_SCU_SCUDATA_PORT);
	addr_dst.sin_addr.s_addr = inet_addr(HF_MGW_SCU_SCUDATA_IP);
	
	Board_Data_SendTo_Inc_Addr((uint8 *)&route_pro, sizeof(ST_HF_MGW_DCU_PRO_HEADER)+msg->len+3, \
		addr_dst);

	return 0;

}

int do_submit_ip_route(cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	ST_HF_MGW_DCU_PRO ip_route_pro;
	ST_HF_MGW_DCU_ADD_IP_ROUTE_MSG * msg;
	int i = 1;
	struct sockaddr_in addr_dst;

	if(argc != 3){
		printf("Invalid Parameter!\n");
		printf("usage: %s",cmdtp->usage);
		return 0;
	}

	ip_route_pro.header.baowen_type = BAOWEN_TYPE_MGW_DCU;
	ip_route_pro.header.dst_addr = 0x10;
	ip_route_pro.header.src_addr = 0xb0;
	ip_route_pro.header.info_type = INT_FRAME_TYPE_CMD_ROUTE;

	msg = (ST_HF_MGW_DCU_ADD_IP_ROUTE_MSG *)ip_route_pro.body;
	msg->id = MGW_DCU_ADD_IP_ROUTE_ID;

	msg->route[0].ip_addr = inet_addr(argv[1]);
	msg->route[0].mask = strtoul(argv[2],NULL,10);
	
	msg->len = 5*i+2;
	msg->route_count = i;

	ip_route_pro.header.data_len = msg->len+3;

	VERBOSE_OUT(LOG_SYS,"Send %d IP Route msg!\n",i);

	addr_dst.sin_family = AF_INET;
	addr_dst.sin_port = htons(HF_MGW_SCU_SCUDATA_PORT);
	addr_dst.sin_addr.s_addr = inet_addr(HF_MGW_SCU_SCUDATA_IP);

	Board_Data_SendTo_Inc_Addr((uint8 *)&ip_route_pro, sizeof(ST_HF_MGW_DCU_PRO_HEADER)+msg->len+3, addr_dst);	

	return 0;
}

int do_submit_del_ip_route(cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	ST_HF_MGW_DCU_PRO ip_route_pro;
	ST_HF_MGW_DCU_ADD_IP_ROUTE_MSG * msg;
	int i = 1;
	struct sockaddr_in addr_dst;

	if(argc != 3){
		printf("Invalid Parameter!\n");
		printf("usage: %s",cmdtp->usage);
		return 0;
	}

	ip_route_pro.header.baowen_type = BAOWEN_TYPE_MGW_DCU;
	ip_route_pro.header.dst_addr = 0x10;
	ip_route_pro.header.src_addr = 0xb0;
	ip_route_pro.header.info_type = INT_FRAME_TYPE_CMD_ROUTE;

	msg = (ST_HF_MGW_DCU_ADD_IP_ROUTE_MSG *)ip_route_pro.body;
	msg->id = MGW_DCU_DEL_IP_ROUTE_ID;

	msg->route[0].ip_addr = inet_addr(argv[1]);
	msg->route[0].mask = strtoul(argv[2],NULL,10);
	
	msg->len = 5*i+2;
	msg->route_count = i;

	ip_route_pro.header.data_len = msg->len+3;

	VERBOSE_OUT(LOG_SYS,"Send %d del IP Route msg!\n",i);

	addr_dst.sin_family = AF_INET;
	addr_dst.sin_port = htons(HF_MGW_SCU_SCUDATA_PORT);
	addr_dst.sin_addr.s_addr = inet_addr(HF_MGW_SCU_SCUDATA_IP);
	
	Board_Data_SendTo_Inc_Addr((uint8 *)&ip_route_pro, sizeof(ST_HF_MGW_DCU_PRO_HEADER)+msg->len+3, addr_dst);
		
	return 0;
}

int do_query_hf_route(cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	ST_HF_MGW_DCU_PRO route_pro;
	ST_HF_MGW_DCU_HF_ROUTE_QUERY_MSG * msg;
	struct sockaddr_in addr_dst;
	
	route_pro.header.baowen_type = BAOWEN_TYPE_MGW_DCU;
	route_pro.header.dst_addr = 0x10;
	route_pro.header.src_addr = 0xb0;
	route_pro.header.info_type = INT_FRAME_TYPE_CMD_ROUTE;
	
	msg = (ST_HF_MGW_DCU_HF_ROUTE_QUERY_MSG *)route_pro.body;
	msg->id = MGW_DCU_HF_ROUTE_QUERY_ID;
	msg->len = 0;

	route_pro.header.data_len = msg->len+3;
	
	//inc_log(LOG_DEBUG,"Send query hf Route msg!\n");

	addr_dst.sin_family = AF_INET;
	addr_dst.sin_port = htons(HF_MGW_SCU_SCUDATA_PORT);
	addr_dst.sin_addr.s_addr = inet_addr(HF_MGW_SCU_SCUDATA_IP);
	
	Board_Data_SendTo_Inc_Addr((uint8 *)&route_pro, sizeof(ST_HF_MGW_DCU_PRO_HEADER)+msg->len+3, addr_dst);
	
	return 0;	
}

/* Z018ý�����ذ���50�����ݽ�����֮������ֱ��� */
void mgw_dcu_link(void)
{
	ST_HF_MGW_DCU_PRO pro;
	ST_HF_MGW_DCU_LINK_MSG* msg;
	struct sockaddr_in addr;
	
	pro.header.baowen_type = BAOWEN_TYPE_MGW_DCU;
	pro.header.dst_addr = 0x10;
	pro.header.src_addr = 0xb0;
	pro.header.info_type = 0x01;
	pro.header.data_len = sizeof(ST_HF_MGW_DCU_LINK_MSG);

	msg = (ST_HF_MGW_DCU_LINK_MSG*)pro.body;
	msg->id = MGW_DCU_LINK_ID;
	msg->len = 2;
	msg->content[0] = 0xb0;
	msg->content[1] = 0x34;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(HF_MGW_SCU_SCUDATA_IP);
	addr.sin_port = htons(HF_MGW_SCU_SCUDATA_PORT);

	Board_Data_SendTo_Inc_Addr((uint8 *)&pro, sizeof(ST_HF_MGW_DCU_PRO_HEADER)+sizeof(ST_HF_MGW_DCU_LINK_MSG), addr);
		
}

int mgw_scu_open(const void * data)
{
	ST_HF_MGW_SCU_PRO pro;
	ST_HF_MGW_SCU_OPEN_MSG * msg;
	int tmp_len;
	TERMINAL_BASE * ter = (TERMINAL_BASE *)data;

	msg = (ST_HF_MGW_SCU_OPEN_MSG *)pro.body;
	pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	pro.header.version = HF_MGW_SCU_PRO_VER;
	pro.header.info_type = INFO_TYPE_PARARM_SET;
	pro.header.len = sizeof(ST_HF_MGW_SCU_OPEN_MSG);

	msg->id = MGW_SCU_OPEN_ID;
	msg->mem_count = 1;
	msg->len = 1*msg->mem_count+1;
	msg->terminal[0] = ter->bslot_us;

	VERBOSE_OUT(LOG_SYS,"ReOpen terminal :%d - slot:%d\n",ter->wPort,ter->bslot_us);

	tmp_len = sizeof(ST_HF_MGW_SCU_PRO_HEADER) + msg->len + 2;

	Board_Sem_SendTo_Inc((uint8*)&pro, tmp_len);
	
	return 0;
}

/* Z043 ý�����ذ���50���ڷ�ͨ�ſ��ư�֮������ֱ��� */
int mgw_scu_ShakeHand(void)
{
	ST_HF_MGW_SCU_PRO pro;
	ST_HF_MGW_SCU_LINK_MSG *link_msg = NULL;
	int tmp_len;

	pro.header.protocol_type = HF_MGW_SCU_PRO_TYPE;
	pro.header.version = HF_MGW_SCU_PRO_VER;
	pro.header.info_type = INFO_TYPE_MANAGE;
	pro.header.len = sizeof(ST_HF_MGW_SCU_LINK_MSG);

	link_msg = (ST_HF_MGW_SCU_LINK_MSG *)pro.body;
	link_msg->id = MGW_SCU_LINK_ID;
	link_msg->len = 1;
	link_msg->link_status = HF_MGW_SCU_LINK_STATUS_FAILED;

	//VERBOSE_OUT(LOG_SYS,"Send shake hand Msg");

	tmp_len = sizeof(ST_HF_MGW_SCU_PRO_HEADER) + link_msg->len + 2;

	Socket_Send(gIpcSemSocket.udp_handle, (struct sockaddr_in*)&gIpcSemSocket.addr_them, \
				(uint8*)&pro, tmp_len);
		
	return 0;
}


int32 Board_Mng_SendTo_Dis(uint8 *buf, int32 len)
{
	struct sockaddr_in my_addr;
	int ret = 0;

	int i = 0;

	if(flag_dump == DUMP_OPEN)
	{
		printf("%s\r\n", __func__);
		for(i = 0; i < len; i++)
		{
			printf("%02x:", buf[i]);
		}
		printf("\r\n");	
	}

	memset(&my_addr, 0x00, sizeof(my_addr));

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htonl(50001);
	my_addr.sin_addr.s_addr = SocketT_DisPlayIP.sin_addr.s_addr;

	ret = Socket_Send(gRpcDisSocket, (struct sockaddr_in*)&my_addr, buf, len);
	if(DRV_ERR == ret)
	{
		printf("%s send error\r\n", __func__);
		return DRV_ERR;
	}
			
	return DRV_OK;
}


/* ҵ��ӿڰ�����԰����ֱ��ģ�Z043��ʱ���� */
int32 Board_Display_Connect(void)
{
#ifdef Z024_Z018_VER
	ST_HF_MGW_DCU_PRO send;

	memset(&send, 0x00, sizeof(send));

	send.header.baowen_type = MSG_DISPLAY_TYPE;
	send.header.dst_addr = MSG_DISPLAY_ADDR;
	send.header.src_addr = MSG_YWB834_ADDR;
	send.header.info_type = MSG_TYPE_BOARD_CONNECT;
	send.header.data_len = 0x0C;
	send.body[0] = MSG_YWB834_ADDR;
	send.body[1] = 0x34;
	
	send.body[2] = 0x01,
	send.body[3] = 0x00,
	send.body[4] = 0x00,	

	send.body[5] = 0x01,
	send.body[6] = 0x00,
	send.body[7] = 0x00,		
	

	Board_Mng_SendTo_Dis((uint8 *)&send, \
		send.header.data_len + sizeof(ST_HF_MGW_DCU_PRO_HEADER));
#else
	ST_SEAT_MNG_MSG send;

	memset(&send, 0x00, sizeof(send));

	send.header.protocol_version = Z043_PROTOCAL_TYPE;
	send.header.dst_addr = BAOWEN_ADDR_TYPE_834_DIS_BOARD;		
	send.header.src_addr = BAOWEN_ADDR_TYPE_834_BOARD;		
	send.header.msg_type = BAOWEN_MSG_TYPE_CMD;	
	send.header.reserve = 0x0;
	send.header.data_len = 0x08;

	send.body[0] = BAOWEN_ADDR_TYPE_834_BOARD;
	send.body[1] = 0x34;
	
	send.body[2] = 0x01,
	send.body[3] = 0x00,
	send.body[4] = 0x00,	

	send.body[5] = 0x01,
	send.body[6] = 0x00,
	send.body[7] = 0x00,			

	Board_Mng_SendTo_Dis((uint8 *)&send, send.header.data_len + sizeof(ST_SEAT_MNG_HEADER));

#endif
			
	return DRV_OK;
}

void workModSet(int8 setMode){
	/* ����ģʽ���浽834���� */
	if((WORK_MODE_PAOBING == setMode) || (WORK_MODE_ZHUANGJIA == setMode) ||	\
					(WORK_MODE_ADAPTER == setMode)){
	change_config_var_val(mgw_cfg, "workmode", setMode);
	//rewrite_config(mgw_cfg);
	Param_update.cfg_wr_flg = 1;

	VERBOSE_OUT(LOG_SYS,"Set 834 board workmode to 0x%xx\r\n", setMode);
	}
}

/* ����834����Ĳ��� */
int32 Board_Mng_834Proc(uint8 *buf, int32 len)
{
	ST_SEAT_MNG_MSG send;

	ST_SEAT_MNG_MSG *msg = (ST_SEAT_MNG_MSG *)buf;
	int8 send_flg = 0; //�����Ƿ�����Ӧ��Ϣ
	int32 i = 0;
	int8 setMode;
	int tmp_port = 0;
	int id = 0;
	char name[10] = {0};	
	char name_tmp[10] = {0};	
	int val = 0;
	
	/* ������Ӧ��Ϣ */
	memset(&send, 0x00, sizeof(send));

	send.header.protocol_version = 0x01;
	send.header.dst_addr = BAOWEN_ADDR_TYPE_834_PC;
	send.header.src_addr = BAOWEN_ADDR_TYPE_834_BOARD;

	send.header.msg_type = BAOWEN_MSG_TYPE_CMD;
	send.header.reserve = 0x00;

	switch(msg->body[0])
	{
		case MSG_834_HANDLE_MSG:
			if(4 == msg->header.data_len)
			{
				i = 0;
				send.body[i++] = MSG_834_HANDLE_MSG_ACK;			
				send.body[i++] = 0x00;
				send.body[i++] = 0x01; //����
				send.body[i++] = 0x00; 
				send.header.data_len = i;//�����ܳ���
				send_flg = 1;
			}			
			break;	
		case MSG_834_ZHENRUFENJI_ADD:
			if ((WORK_MODE_PAOBING == Param_update.workmode)\
				||(WORK_MODE_ZHUANGJIA == Param_update.workmode))
			{
				return DRV_OK;
			}
			
			if(15 == msg->header.data_len)
			{
				i = 0;
				send.body[i++] = MSG_834_ZHENRUFENJI_ADD_ACK;			
				send.body[i++] = 0x00;
				send.body[i++] = 0x01; //����

				if(msg->body[3] > KUOZHAN_USER_NUM_MAX)
				{
					send.body[i++] = 0xFF; 
				}	
				else if(((msg->body[3] >= KUOZHAN_ADATPER_BASE_SLOT)&&(msg->body[3] <= KUOZHAN_USER_NUM_MAX))\
					&&(0 == terminal_group[KUOZHAN_ADATPER_OFFSET].adp_phone_online))
				{
					send.body[i++] = 0xFF; 
				}
				else
				{	
					tmp_port = find_port_by_simple_slot(msg->body[3]);
					if(tmp_port < 0)
            		{
            			ERR("%s find port %d error\r\n", __func__, msg->body[3]);
            			return DRV_ERR;
            		}
		            		
					if((0 != terminal_group[tmp_port].zhenru_slot)||(msg->body[4] < QINWU_BASE_SLOT)\
						||(msg->body[4] > KUOZHAN_USER_NUM_MAX))					
					{
						send.body[i++] = 0xFF; 
					}
					else
					{
						int id = 0;
						char name[10] = {0};
						switch(msg->body[3])
						{
							case 2:
								id = 0;
								break;
							case 3:
								id = 1;
								break;
							case 31:
								id = 2;
								break;								
                            case 32:
                                id = 3;
                                break;
                            case 33:
                                id = 4;
                                break;
							case 34:
								id = 5;
								break;
							case 35:
								id = 6;
								break;
							case 36:
								id = 7;
								break;
							case 37:
								id = 8;
								break;
							case 38:
								id = 9;
								break;
							case 39:
								id = 10;
								break;
							case 40:
								id = 11;
								break;
							default:
								break;								
						}

						snprintf(name, sizeof(name), "FENJI%d", id);
						change_config_var_val(mgw_cfg, name, msg->body[4]);
						//rewrite_config(mgw_cfg);
						Param_update.cfg_wr_flg = 1;
						terminal_group[tmp_port].zhenru_slot =  msg->body[4];
						send.body[i++] = 0x00; 
						if((0 != id )&&(1 != id))
						{
							Adp_Phone_Zhenru_Fenji_Inject_all();
						}
					}
				}
				
				send.header.data_len = i;//�����ܳ���
				send_flg = 1;
			}			
			break;
		case MSG_834_ZHENRUFENJI_DEL:
			if ((WORK_MODE_PAOBING == Param_update.workmode)\
				||(WORK_MODE_ZHUANGJIA == Param_update.workmode))
			{
				return DRV_OK;
			}
			
			if(15 == msg->header.data_len)
			{
				i = 0;
				send.body[i++] = MSG_834_ZHENRUFENJI_DEL_ACK;			
				send.body[i++] = 0x00;
				send.body[i++] = 0x01; //����

				if(msg->body[3] > KUOZHAN_USER_NUM_MAX)
				{
					send.body[i++] = 0xFF; 
				}	
				else if(((msg->body[3] >= KUOZHAN_ADATPER_BASE_SLOT)&&(msg->body[3] <= KUOZHAN_USER_NUM_MAX))\
					&&(0 == terminal_group[KUOZHAN_ADATPER_OFFSET].adp_phone_online))
				{
					send.body[i++] = 0xFF; 
				}	
				else
				{	
					tmp_port = find_port_by_simple_slot(msg->body[3]);
                	if(tmp_port < 0)
                	{
                		ERR("%s find port %d error\r\n", __func__, msg->body[3]);
                		return DRV_ERR;
                	}
		                	
                	if((msg->body[4] < QINWU_BASE_SLOT)\
						||(msg->body[4] >= KUOZHAN_USER_NUM_MAX))					
					{
						send.body[i++] = 0xFF; 
					}
					else
					{
						switch(msg->body[3])
						{
							case 2:
								id = 0;
								break;
							case 3:
								id = 1;
								break;
							case 31:
								id = 2;
								break;	
                            case 32:
                                id = 3;
                                break;
                            case 33:
                                id = 4;
                                break;
							case 34:
								id = 5;
								break;
							case 35:
								id = 6;
								break;
							case 36:
								id = 7;
								break;
							case 37:
								id = 8;
								break;
							case 38:
								id = 9;
								break;
							case 39:
								id = 10;
								break;
							case 40:
								id = 11;
								break;
							default:
								break;								
						}

						snprintf(name, sizeof(name), "FENJI%d", id);
						change_config_var_val(mgw_cfg, name, 0);
						//rewrite_config(mgw_cfg);
						Param_update.cfg_wr_flg = 1;
						terminal_group[tmp_port].zhenru_slot = 0;
						send.body[i++] = 0x00; 
						if((0 != id )&&(1 != id))
						{
							Adp_Phone_Zhenru_Fenji_Inject_all();
						}
					}
				}
				
				send.header.data_len = i;//�����ܳ���
				send_flg = 1;
			}
			break;
		case MSG_834_ZHENRUFENJI_GET:
			if ((WORK_MODE_PAOBING == Param_update.workmode)\
				||(WORK_MODE_ZHUANGJIA == Param_update.workmode))
			{
				return DRV_OK;
			}
			if(3 == msg->header.data_len)
			{
				int trunk_cnt = 2;
			
				/* ������Ӧ��Ϣ */
				if(0 == terminal_group[KUOZHAN_ADATPER_OFFSET].adp_phone_online)
				{
					trunk_cnt = 2;
				}
				else if(1 == terminal_group[KUOZHAN_ADATPER_OFFSET].adp_phone_online)
				{
					trunk_cnt = 12;
				}
				
				i = 0;
				send.body[0] = MSG_834_ZHENRUFENJI_GET_ACK;			
				send.body[1] = 0x00;
				send.body[2] =  12 * trunk_cnt;

				for(i = 0; i < trunk_cnt; i++)
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
					
					send.body[3 + i * 12] = id;
					tmp_port = find_port_by_simple_slot(id);
					if((tmp_port > 0)&&(tmp_port < KUOZHAN_USER_NUM_MAX)&&(0 != terminal_group[tmp_port].zhenru_slot))
					{
						send.body[3 + i * 12 + 1] = terminal_group[tmp_port].zhenru_slot;
						phone2bcd((char *)&send.body[3 + i * 12 + 2], \
						(char *)terminal_group[terminal_group[tmp_port].zhenru_slot].name, \
								terminal_group[terminal_group[tmp_port].zhenru_slot].user_num_len);
						tmp_port = find_port_by_simple_slot(terminal_group[tmp_port].zhenru_slot);
		        	    		if(tmp_port < 0)
		                		{
		                			ERR("%s find port by zhenru_slot %d error\r\n", __func__, send.body[3 + i * 12 + 1]);
		                			return DRV_ERR;
		                		}
		                		
						memcpy(&send.body[3 + i * 12 + 2], terminal_group[tmp_port].user_num, 4);			
					}
				}

				send.header.data_len = send.body[2] + 3;//�����ܳ���
				send_flg = 1;
			}
			break;	
		case MSG_834_GPORT_MODE_SET:
			if(6 == msg->header.data_len)
			{
				/* ������Ӧ��Ϣ */
				i = 0;
				send.body[i++] = MSG_834_GPORT_MODE_SET_ACK;			
				send.body[i++] = 0x00;
				send.body[i++] = 0x01; //����
				
				if(msg->body[5] < CTL_GINTF_WORKMODE_BUTT)
				{
					Gport_Speed_Set(msg->body[5]);
					change_config_var_val(mgw_cfg, "gport_speed", msg->body[5]);
					//rewrite_config(mgw_cfg);	
					Param_update.cfg_wr_flg = 1;
					send.body[i++] = 0x00; 
				}
				else
				{
					send.body[i++] = 0xff; 
				}				
				
				send.header.data_len = i;//�����ܳ���
				send_flg = 1;
			}
			break;				
		case MSG_834_GPORT_MODE_GET:
			if(4 == msg->header.data_len)
			{
				/* ������Ӧ��Ϣ */
				i = 0;
				send.body[i++] = MSG_834_GPORT_MODE_GET_ACK;			
				send.body[i++] = 0x00;
				send.body[i++] = 0x03; //����
				send.body[i++] = 0x06; 
				send.body[i++] = 0x00; 
				send.body[i++] = get_config_var_val(mgw_cfg, "gport_speed");
				send.header.data_len = i;//�����ܳ���
				send_flg = 1;
			}		
			
			break;				
		case MSG_834_WORKMODE_SET:

			i = 0;
			send.body[i++] = MSG_834_WORKMODE_SET_ACK;
			send.body[i++] = 0x00;
			send.body[i++] = 0x01; //����

			setMode = msg->body[3];
			if((4 == msg->header.data_len) &&  \
				((WORK_MODE_PAOBING == setMode) || (WORK_MODE_ZHUANGJIA == setMode) ||  \
				(WORK_MODE_ADAPTER == setMode)))
			{
				workModSet(setMode);		

				send.body[i++] = 0x00;	
			}
			else
			{
				send.body[i++] = 0xFF;
			}

			send.header.data_len = i;//�����ܳ���
			send_flg = 1;
			break;
		case MSG_834_WORKMODE_GET:
			if(4 == msg->header.data_len)
			{
				/* ������Ӧ��Ϣ */
				i = 0;
				send.body[i++] = MSG_834_WORKMODE_GET_ACK;			
				send.body[i++] = 0x00;
				send.body[i++] = 0x01; //����
				send.body[i++] = get_config_var_val(mgw_cfg, "workmode");
				
				send.header.data_len = i;//�����ܳ���

				send_flg = 1;
			}					
			break;					
		case MSG_834_GROUP_ID_ADD:
			if ((WORK_MODE_PAOBING == Param_update.workmode)\
				||(WORK_MODE_ZHUANGJIA == Param_update.workmode))
			{
				return DRV_OK;
			}
			
			//if(3 == msg->header.data_len)
			{
				i = 0;
				send.body[0] = MSG_834_GROUP_ID_ADD_ACK;			
				send.body[1] = 0x00;
				send.body[2] =  1;

				for(i = 0; i < KUOZHAN_CONF_MEM_MAX_USE; i++)
				{
					snprintf(name, sizeof(name), "phone%d", i);
					change_config_var_val(mgw_cfg, name, 0);
					kuozhan_conf[0].conf_phone_flg[i] = 0;
						
				}

				for(i = 0; i < msg->body[5]; i++)
				{
					snprintf(name, sizeof(name), "phone%d", i);
					bcd2phone((char *)name_tmp, (char *)&msg->body[6 + i * 10], 3);					
					change_config_var(mgw_cfg, name, name_tmp);
					kuozhan_conf[0].conf_phone_flg[i] = 1;
					val = get_config_cat_var_val(mgw_cfg, PHONE, name);
					snprintf(kuozhan_conf[0].conf_phone[i], 10, "%d", val);
				}				

				//rewrite_config(mgw_cfg);
				Param_update.cfg_wr_flg = 1;
				
				send.body[3] =  0x00;
				send.header.data_len = send.body[2] + 3;//�����ܳ���
				send_flg = 1;
			}		
			break;
		case MSG_834_GROUP_ID_DEL:
			if ((WORK_MODE_PAOBING == Param_update.workmode)\
				||(WORK_MODE_ZHUANGJIA == Param_update.workmode))
			{
				return DRV_OK;
			}
			
			if(5 == msg->header.data_len)
			{
				i = 0;
				send.body[0] = MSG_834_GROUP_ID_DEL_ACK;			
				send.body[1] = 0x00;
				send.body[2] =  1;

				for(i = 0; i < KUOZHAN_CONF_MEM_MAX_USE; i++)
				{
					snprintf(name, sizeof(name), "phone%d", i);
					change_config_var_val(mgw_cfg, name, 0);
					kuozhan_conf[0].conf_phone_flg[i] = 0;	
				}

				//rewrite_config(mgw_cfg);
				Param_update.cfg_wr_flg = 1;
				
				send.body[3] =  0x00;
				send.header.data_len = send.body[2] + 3;//�����ܳ���
				send_flg = 1;
			}		
			break;		
		case MSG_834_GROUP_ID_GET:
			if ((WORK_MODE_PAOBING == Param_update.workmode)\
				||(WORK_MODE_ZHUANGJIA == Param_update.workmode))
			{
				return DRV_OK;
			}
			if(3 == msg->header.data_len)
			{
				i = 0;
				int actual_cnt = 0;
				send.body[0] = MSG_834_GROUP_ID_GET_ACK;			
				send.body[1] = 0x00;
				send.body[3] =  1;
				memcpy(&send.body[4], kuozhan_conf[0].conf_user_name, 2);	
				
				for(i = 0; i < KUOZHAN_CONF_MEM_MAX_USE; i++)
				{
					if(1 == kuozhan_conf[0].conf_phone_flg[i])
					{
						actual_cnt++;
						phone2bcd(name, &kuozhan_conf[0].conf_phone[i][0], 3);
						memcpy(&send.body[7 + i *10], name, 2);
					}
				}

				send.body[2] =  11 * actual_cnt;
				send.body[6] =  actual_cnt;
				send.header.data_len = send.body[2] + 3;//�����ܳ���
				send_flg = 1;
			}		
			break;				
		case MSG_834_GROUP_MEM_ADD:
			break;				
		case MSG_834_GROUP_MEM_DEL:
			break;	
		default:
			break;
	}

	if(1 == send_flg)
	{
		send.header.dst_addr = BAOWEN_ADDR_TYPE_834_PC;
		RpcSeatMng_SendTo_Seat((uint8 *)&send, sizeof(ST_SEAT_MNG_HEADER) + send.header.data_len);
		if(1 == Param_update.ip_addr_default_flg)
		{
			RpcSeatMng_Default_SendTo_Seat((uint8 *)&send, sizeof(ST_SEAT_MNG_HEADER) + send.header.data_len);
		}
		
		/* �ڷ�ģʽ��,������834��������ѯ����ģʽ��K��ģʽ���͸���ʾ�� */
		send.header.dst_addr = BAOWEN_ADDR_TYPE_834_DIS_BOARD;
		Board_Mng_SendTo_Dis((uint8 *)&send, sizeof(ST_SEAT_MNG_HEADER) + send.header.data_len);			

		/* Forward Pkt to 834 Net snmp agent*/
		send.header.dst_addr = BAOWEN_ADDR_TYPE_834_SNMP_AGENT;
		Board_Mng_SendTo_SnmpAgent((uint8 *)&send, sizeof(ST_SEAT_MNG_HEADER) + send.header.data_len);			
	}

	return DRV_OK;
}

/* 
���������������50�����������Ϣ��ת����<ר����ս�������豸����ӿ�Э��v010 20140805>����50�����塣 
*/
int32 Board_Mng_Sendto50Proc(uint8 *buf, int32 len)
{
	MNG_50_MSG send;//ת����50������Ϣ
	
	ST_SEAT_MNG_MSG *msg = (ST_SEAT_MNG_MSG *)buf;
	uint32 tmplen = 0;
	int8 send_flg = 0; //�����Ƿ���

	/* ��Ϣ���е�2���ֽ������ֲ������� */
	VERBOSE_OUT(LOG_SYS,"Set 50 board para(0x%x) to module 0x%02x from mng 0x%x\r\n", \
	             msg->body[1], msg->body[0], msg->header.src_addr);

	memset(&send, 0x00, sizeof(send));
	
	/* ���ת����Ϣ��ͷ�� */
	memcpy(&send.Head, herader50, sizeof(herader50));

	/* �Ϸ��Լ��ֻ�жϸ�����Ϣ�ĳ��� */
	tmplen = msg->header.data_len;

#if 0	
	switch(msg->body[1]) //��������
	{
		case MSG_50_K_MODE_SET:		
			if (7 == tmplen ) //50����Ϣ�е����ĳ���ΪĿ�ĵ�ַ�����Ժ�������ֶε��ֽ���
			{			
				send_flg = 1;
			}			
			break;
		case MSG_50_K_MODE_GET:
			if (6 == tmplen)
			{
				send_flg = 1;
			}
			break;

		default:
			break;					
	}
#else

	send_flg = 1;

#endif

	if(1 == send_flg)
	{
		send.Head.MsgLen = tmplen;
		memcpy(send.Data, msg->body, tmplen);
		Board_Mng_SendTo_50((uint8 *)&send, sizeof(MNG_50_MSG_HEAD) + send.Head.MsgLen);
	}

	return DRV_OK;
}

/*  ����50������������Ϣ�����ʽΪ<ר����ս�������豸����ӿ�Э��v010 20140805>��
ת���� <�����豸����ӿ�Э��__20150414>�������԰��PC�������
*/
int32 Board_Mng_RxFrom50Proc(uint8 *buf, int32 len)
{
	ST_SEAT_MNG_MSG  send;//50������Ϣ
	MNG_50_MSG *rcv = (MNG_50_MSG *)buf;
	uint32 tmplen = 0;

	memset(&send, 0x00, sizeof(send));

	/* �ж���Ϣͷ�е�Э�����ͺ��������ͺϷ��� */
	if ((rcv->Head.ProType != 0x00) || (rcv->Head.PackType != 0x80) || (rcv->Head.MsgLen >= MAXSCUPROSIZE))
	{
		ERR("Get invalid msg from 50 msg=0x%x, type=0x%x,len=%d.\r\n", \
		      rcv->Head.ProType, rcv->Head.PackType, rcv->Head.MsgLen );

		return DRV_ERR;
	}
	
	/* ���ת����Ϣ��ͷ�� */
	send.header.protocol_version = 0x01;

	/* �ر�ע��:��Ӧ��Ϣת����PC���ܺͼ��԰�ʱ�����������Ϣ�е�Ŀ�ĵ�ַ�����ﲻ�̶ܹ����0x10
	  ( ��50�����弯�ɶ��ģ�飬������Ϣ�в�ͬģ���Ŀ�ĵ�ַ��ͬ),��Ҫ���50����Ӧ��Ϣ�е�Ŀ�ĵ�ַ��  
	 */
	//send.header.src_addr = rcv->Data[0];
	send.header.src_addr = BAOWEN_ADDR_TYPE_50_BOARD;
	send.header.msg_type = BAOWEN_MSG_TYPE_CMD;
	send.header.reserve = 0x00;

	/* ת�����������ʱ�������е�Ŀ�ĵ�ַ�����Ժ���ֶζ���Ҫת�� */
	tmplen = rcv->Head.MsgLen; 
	send.header.data_len = tmplen;
	memcpy(send.body, rcv->Data, tmplen);

	/* ���͸�PC������� */
	send.header.dst_addr = BAOWEN_ADDR_TYPE_834_PC;
	RpcSeatMng_SendTo_Seat((uint8 *)&send, sizeof(ST_SEAT_MNG_HEADER) + send.header.data_len);
	if(1 == Param_update.ip_addr_default_flg)
	{
		RpcSeatMng_Default_SendTo_Seat((uint8 *)&send, sizeof(ST_SEAT_MNG_HEADER) + send.header.data_len);
	}
	/* �޸�Ŀ�ĵ�ַ�������԰� */
	send.header.dst_addr = BAOWEN_ADDR_TYPE_834_DIS_BOARD;
	Board_Mng_SendTo_Dis((uint8 *)&send, sizeof(ST_SEAT_MNG_HEADER) + send.header.data_len);	
	
	return DRV_OK;
}

/* 
���������ڴ�������PC��������ͼ��԰����õĲ�����
�ڷ�ģʽ�£���ʾ�����ò���ת���� <ר����ս�������豸����ӿ�Э��v010 20140805>����50�� 
*/
int32 Board_Mng_Process(uint8 *buf, int32 len)
{
	ST_RAY_MNG_MSG ray_msg;
	ST_HF_MGW_DCU_PRO msg;//���͸�716����Ϣ
	
	ST_SEAT_MNG_MSG *rcv = (ST_SEAT_MNG_MSG *)buf;
	
	if((BAOWEN_ADDR_TYPE_834_PC != rcv->header.src_addr) \
		&& (BAOWEN_ADDR_TYPE_834_DIS_BOARD != rcv->header.src_addr)
		&& (BAOWEN_ADDR_TYPE_30_PC != rcv->header.src_addr)
		&& (BAOWEN_ADDR_TYPE_834_SNMP_AGENT!= rcv->header.src_addr))
	{
		ERR("%s: drop packet src addr 0x%x\r\n", __func__, rcv->header.src_addr);
		return DRV_ERR;
	}
	
	memset(&ray_msg, 0x00, sizeof(ray_msg));
	memset(&msg, 0x00, sizeof(msg));

	/* ���ݾ���״̬�������ܣ�ֱ�Ӵ���0xD1 */
	#ifdef MNG_FOR_JINBIAO 	
	if(buf[0] == 0xd1)
	{
		Board_Mng_SendTo_716_Ray(buf, len);
	}
	#endif

	switch(rcv->header.dst_addr)
	{
		case BAOWEN_ADDR_TYPE_834_BOARD:
			/* 834ģʽ������Ϣ����ͬ50��ģʽ���ݣ�ֻ�Ǽ��Է���ʱ���ò�ͬ��Ŀ�ĵ�ַ */
			Board_Mng_834Proc(buf, len);
			break;
		case BAOWEN_ADDR_TYPE_716_BOARD:
			/* ��716�������ú�����Ŀ��50���Ľ���Э�飬����ģ��50����������Ϣ����716 */
			msg.header.baowen_type = 0xA0;
			msg.header.dst_addr = BAOWEN_ADDR_TYPE_716_BOARD;
			msg.header.src_addr = BAOWEN_ADDR_TYPE_50_BOARD;
			msg.header.info_type = BAOWEN_MSG_TYPE_CMD;
			msg.header.data_len = rcv->header.data_len;

			memcpy(msg.body, rcv->body, rcv->header.data_len);

			/* �ڷ�ģʽ�£������������ս����Ԫ�Ĳ�����834��Э��<ר����ս�������豸����ӿ�Э��v010 20140805>
			  ת����50��,50��͸����716 
			*/
			if (WORK_MODE_PAOBING == Param_update.workmode)
			{
			#ifdef MNG_FOR_JINBIAO 	
				/* ����������ר����Ϣ */
				Board_Mng_ZhuanXianMsg50Proc(buf, len);
		
				/* �յ�����״̬�����������ս��������Ϣֱ��ת����50��͸��
				*/
				Board_Mng_SendTo_50(buf, len);			
			#else
				/* ����������ר����Ϣ */
				Board_Mng_ZhuanXianMsgProc(buf, len);
		
				/* ֱ��ת��ϯλ��װ�������������Ϣ���������ʹ��װ��������� */
			#endif
			}

			/* װ�׺��ڷ�ģʽ��ս����Ϣ������716 */
			Board_Mng_SendTo_716((uint8 *)&msg, \
				sizeof(ST_HF_MGW_DCU_PRO_HEADER) + msg.header.data_len);
			break;
		case BAOWEN_ADDR_TYPE_716_RAY_BOARD:
			memset(&ray_msg, 0x00, sizeof(ray_msg));
			
			ray_msg.header.msg_type = 0xD1;
			ray_msg.header.msg_flg = rcv->body[0];
			ray_msg.header.board_local = 0x0A;
			ray_msg.header.board_id= 0x00;
			ray_msg.header.data_len = *((uint16 *)&(rcv->body[1]));

			if(0x00 != ray_msg.header.data_len)
			{
				memcpy(ray_msg.body, (uint8 *)&(rcv->body[3]), ray_msg.header.data_len);
			}

			Board_Mng_SendTo_716_Ray((uint8 *)&ray_msg, \
				sizeof(ST_RAY_MNG_HEADER) + ray_msg.header.data_len);
			
			break;
		/* 50��ר������Ŀ�ĵ�ַ��ģ��������ͬ */
		case BAOWEN_ADDR_TYPE_50_BOARD:
		//case BAOWEN_ADDR_TYPE_50_WIRELESS:
		//case BAOWEN_ADDR_TYPE_50_WIRE:
		//case BAOWEN_ADDR_TYPE_50_MAIN:
			if(WORK_MODE_PAOBING == Param_update.workmode)
			{
				Board_Mng_Sendto50Proc(buf, len);
			}
			break;
		case BAOWEN_ADDR_TYPE_834_DIS_BOARD:
			/* ���������ʱ�����ü��Բ��� */
			break;
		case BAOWEN_ADDR_TYPE_834_PC:
			/* ���������ʱ���������ܲ��� */
			break;	
		default:
			break;
	}

	return DRV_OK;
}


int mgw_5s_proc(const void * data)
{
	mgw_scu_ShakeHand();
	//Board_Display_Connect();
	
	/* deleted by lishibing 20140723, Z043 */
#if 0
	static int i = 0;	
	mgw_dcu_link();
	mgw_js_route_msg();

	if((i%2) == 0)
		do_query_hf_route(NULL,0,0,NULL);

	i++;		
#endif
	
	return 1;
}


/*******************************************************************************************************
	Added by lishibing 20150928, Z043 �ڷ�ģʽ�µ�̨��PRN��ս����834ҵ��ӿڰ���Ҫ��������������͸�716�ĵ�̨K��
   ר��ע��/ע����Ӧ��Ϣ��(1)��ȡ���������е�ר�ߺ��룬�����б���������Ƿ�Ϊר��716ר�ߴ���ʱ���͵ĺ���������������������
   (2)����������ע����Ϣ��ÿ���ϵ�ʱ�Զ�����ע��ר�߶�����
********************************************************************************************************/
	
/* 
�ڷ�ģʽ�£��������������ս����Ԫ�Ĳ�����Ӧ��Ϣ��834ֱ�ӷ���716��Э���ʽ
<�����豸����ӿ�Э��__20150414>��
�����յ���ע�ἰ��Ӧ:
01 80 90 01 00 00 13 42 00 10 08 01 0d 01 00 00 01 00 01 09 01 21 01 00 f1 00 
01 90 80 01 00 00 0c 42 00 09 08 04 06 01 00 00 00 00 01 

ע������Ӧ:
01 80 90 01 00 00 13 42 00 10 08 03 06 01 00 00 01 00 01 00 00 f0 ff ff ff 00 
01 90 80 01 00 00 0c 42 00 09 08 06 06 01 00 00 00 00 01 
 */
int Board_Mng_ZhuanXianMsgProc(uint8 *buf, int32 len)
{
	ST_SEAT_MNG_MSG msg;
	MNG_ZW_ZHUANXIAN_MSG *strZxCmd = NULL;
	
	int8 kport = 0;
	int8 idex = 0;

	if (len > sizeof(ST_SEAT_MNG_MSG))
	{
		ERR("Msg invalid len(%d)!\r\n", len);
	 	return DRV_ERR;
	}
	memcpy(&msg, buf, len);

	if (msg.header.dst_addr != EN_PORT_716NETWARE )
	{
		ERR("Msg invalid  Dst addr=(0x%x) !\r\n", msg.header.dst_addr);
	 	return DRV_ERR;
	}

	/* ������Ϣ��ת����ר����Ϣ  */
	strZxCmd = (MNG_ZW_ZHUANXIAN_MSG *)&msg.body;
	
	/* ��ָ��ר�߲���ע�� */
	if (0x42 == msg.body[0] && (0x08 == strZxCmd->CmdCode))
	{	
		//716��Ϣ����
		//printf("lsb:Get msg to 716 type =0x%x, cmd =0x%x, OP =0x%x\r\n", strZxCmd->InfoType, strZxCmd->CmdCode, strZxCmd->OpMode);

		/* ��ȡK��ͨ���� */
		kport = strZxCmd->CallerChanId;
		if ((kport > ZW_ZHUANXIAN_NUM)) //K�ڱ��1-4
		{
			return DRV_ERR;
		}
		idex = kport - 1;//K�ڱ��1-4,�����±��0��ʼ
			
		if(EN_OP_ZWZHUANXIAN_REGIST == strZxCmd->OpMode) //ע��
		{
            usleep(1000 * 1000);		
		}
		else if(EN_OP_ZWZHUANXIAN_UNREGIST == strZxCmd->OpMode) //ע��
		{										
			usleep(50 * 1000);	
		}		
	}

	return DRV_OK;
}

/* 
�ڷ�ģʽ�µ�̨��PRN��������ս������K��ר�߷�ʽ���л���ͨ�š�������Ҫʹ��50��һ�廯���ܻ�834������
ע��һ���ڷ��û��ĺ��뵽716��Ȼ��ʹ��834������ע��K��ר�ߣ�716��K��ר�ߴ��������з��ͺ��������834(ͨ��50��͸��)
��834�������嵫��ظ�����Ӧ������������716�����ר�ߴ����ɹ���
Ϊ����ר�ߴ������̵ĺ��������������ĺ�������834����ݺ���������ʱ϶��������Ӧ�ն˵�ר�߱�ʶ��
����834��ר�߱�ʶ���������:
(1)���һ:�û���ʹ�ù�����ͨ��834������ע���ע��ר�ߣ�����������ר�߱�ʶ��
(2)�����:�豸�������ϵ��豣�����ǰ��ר�߱�ʶ��

�������1: 
������һ�����յ����ܵ�K��ר��ע���ע����Ϣ���������ն˵�ר�߱�־������716ע���ע�����ɹ�������£�����834��
716��ר�߲�һ�µ���������ֻ������Ӧ��Ϣ������ר�߱�־������Ӧ��Ϣ��û��ͨ���ź͵绰���룬�Ҳ��ܱ�֤ע��/ע������
����Ӧ��Ӧ��Ϣһһƥ��(�����Ӧ��Ϣû�������ظ�)�����ң���Ӧ��Ϣ�д��ڶ��ֲ��ɹ����(1-ʧ�ܣ�2-ע������
4-����ͨ�����ظ���5-���к����ظ�)�������Ӧ4/5��˵��716�Ѿ�������Ӧר�ߣ���Ȼ��Ҫ��ѯ���ܵõ�716��ר�߽����
��������������ר����Ϣ��ͨ���ź͵绰���뵽�����ļ����ϵ��Զ�ע�ᡣ
�÷���ȱ��:(1)�������Ӧ��Ϣ������Ӧ��Ϣ�޺����ͨ��������Ӧ����ʱʱ����bug��(2)����������ļ���

����2:
������һ���յ�ע���ע����Ϣ���õȴ�Ӧ��״̬����ʱ���߳��и���__NetWare_ZxCreate_Status__״̬��
���Ͳ�ѯ���󣬸��ݲ�ѯ��Ӧ����ר�߱�־��
��������������ר����Ϣ��ͨ���ź͵绰���뵽�����ļ����ϵ��Զ�ע�ᡣ
ȱ��:������״̬���ͱ��������ļ���

����3:
������һ��834�����������ʹ��ָ��ר�߷�ʽ����ĳ��ͨ����ע��/ע����Ϣ���������Ͳ�ѯ��Ϣ�������������Ӧ��Ϣ
����ר�߱�ʶ��
�����������ϵ籾����Զ�����4����ѯ��Ϣ�������������Ӧ��Ϣ����ר�߱�ʶ��

��������÷���3

buf�����ݸ�ʽΪ:716���巵�ص�Ӧ����Ϣ����ʽΪ:
��ѯӦ��: 
a0:10:80:01:00:13:42:00:10:08:05:0d:01:00:00:01:00:01:09:01:21:01:00:f1:00: //ͨ��1��ר��
Board_716_Mng_RxThread
a0:10:80:01:00:13:42:00:10:08:05:0d:01:00:00:01:00:01:09:01:21:01:00:f4:00:

ע��Ӧ��: a0:10:80:01:00:0c:42:00:09:08:06:06:01:00:00:00:00:01:
ע��Ӧ��: a0:10:80:01:00:0c:42:00:09:08:04:06:01:00:00:00:00:01:
*/
int Board_Mng_ZhuanXianAckMsgProc(uint8 *buf, int32 len)
{
	ST_HF_MGW_DCU_PRO *rxMsg = NULL;
	
	char tmpPhoneNum[8] = {0};
	char name[8] = {0};
	
	int tmp_port = 0; //�ն˱�ʶ
	int8 kport = 0;   //K��ͨ����
	int8 AckFlg = 0;
	int8 indx = 0;

	rxMsg = (ST_HF_MGW_DCU_PRO *)buf;
	if (rxMsg->header.dst_addr != EN_PORT_50MAINCTRL)
	{
		ERR("716 Msg invalid Dst addr=(0x%x) !\r\n", rxMsg->header.dst_addr);
	 	return DRV_ERR;
	}
	
	/* ר����Ϣ,��Ϊָ����ר����Ϣ */
	if ((0x42 != rxMsg->body[0]) || (0x08 != rxMsg->body[3]))
	{
		//���ﲻ��ר����Ϣֱ�ӷ��أ����ܴ�ӡ
	 	return DRV_ERR;
	}
	
	switch(rxMsg->body[4]) //��Ϣ����
	{
		case EN_OP_ZWZHUANXIAN_QUERY_ACK: //��ѯӦ��
		{
			MNG_ZW_ZHUANXIAN_MSG ZxCmd; 

			memcpy(&ZxCmd, rxMsg->body, sizeof(ZxCmd));
			
			/* ��ȡK��ͨ���� */
			kport = ZxCmd.CallerChanId;
			if (((0x00 == ZxCmd.CallerFlgSel)&&(kport > ZW_ZHUANXIAN_NUM))\
				||(((19 != ZxCmd.CallerFlgSel)&&(20 != ZxCmd.CallerFlgSel))&&(17 == kport))) //K�ڱ��1-4
			{
				ERR("Z043 support k port number zhuji %d, port %d error\r\n", ZxCmd.CallerFlgSel, kport);								
				return DRV_ERR;
			}
			else
			{
				if(0x00 == ZxCmd.CallerFlgSel)
				{
					indx = kport - 1;//�����±��0��ʼ
				}
				else
				{
					indx = 4 + ZxCmd.CallerFlgSel - 19;//�����±��0��ʼZxCmd.CallerFlgSel
				}
			}

			/* ��ѯӦ����Ϣ�У������Ӧͨ����ר�ߣ���Ӧ���־��0-ר�߲����ڡ�1-����ר�� */
			AckFlg = ZxCmd.AckFlgSel;
			if (0x01 == AckFlg) //����ר�ߴ��ڣ���Ϊ����ר��
			{
				/* ������Ϣ�еĺ�������ն˱�ʶ����Ϣ�к���ΪBCD��,1210001Ϊ0x210100f1�� */
				bcd2phone((char *)tmpPhoneNum, (char *)ZxCmd.BJPhoneNum, 8);
				
				printf("lsb:Get msg phone num(%x:%x:%x:%x:) to %s.\r\n", \
						ZxCmd.BJPhoneNum[0],ZxCmd.BJPhoneNum[1], \
						ZxCmd.BJPhoneNum[2],ZxCmd.BJPhoneNum[3], tmpPhoneNum);
						
				tmp_port = find_port_by_name(tmpPhoneNum);
				if(DRV_ERR == tmp_port)
				{

					ERR("zhuanxin ack port cannot find phone num(%x:%x:%x:%x:)\r\n",ZxCmd.BJPhoneNum[0],ZxCmd.BJPhoneNum[1], \
						ZxCmd.BJPhoneNum[2],ZxCmd.BJPhoneNum[3]);
					
					return DRV_ERR;
				}

				/* ע��ʱ��Ҫʹ���Ƿ�ע���ר�ߵ�״̬�ͳɹ�ע����ר�ߵ��ն�ID */
				SET_ZX_CREATE_TERMINALID(indx, tmp_port);
				//sprintf(name,"K%d",indx);
				snprintf(name, sizeof(name), "K%d", indx);
				change_config_var_val(mgw_cfg, name, tmp_port);
				//rewrite_config(mgw_cfg);	
				Param_update.cfg_wr_flg = 1;
				
				/* �����ն˱�ʶ��Ӧ��ר�ߴ�����־Ӧ���յ�ע��ɹ���Ӧ��  */
				terminal_group[tmp_port].pre_zhuanxian_flg = NETWARE_ZXCREATE_OK;

				printf("lsb:Set terminal %d k port %d zhuanxian flag status %d!\r\n", \
					tmp_port, indx, ZxCmd.ZXianStatus);
			}
			else if (0x00 == AckFlg)
			{
				MNG_ZWZX_UNREGISTER_MSG unRegAck;//ע��Ӧ����ע��������Ϣ�ֶ���ͬ
				memcpy(&unRegAck, rxMsg->body, sizeof(unRegAck));
			
			
				/* �����ն˱�ʶ��Ӧ��ר�ߴ�����־Ӧ���յ�ע��ɹ���Ӧ��  */
				//if (NETWARE_ZXCREATE_REGREQ == GET_ZX_CREATE_STATUS(indx))
				{
					tmp_port = GET_ZX_CREATE_TERMINALID(indx);
					if (tmp_port < USER_NUM_MAX)
					{
						SET_ZX_CREATE_TERMINALID(indx, TERMINAL_ID_INVALID);
						snprintf(name, sizeof(name), "K%d",indx);
						change_config_var_val(mgw_cfg, name, 255);
						//rewrite_config(mgw_cfg);	
						Param_update.cfg_wr_flg = 1;
						
						terminal_group[tmp_port].pre_zhuanxian_flg = NETWARE_ZXCREATE_NOTOK;
						if(PORT_TYPE_PHONE == terminal_group[tmp_port].bPortType)
						{
							terminal_group[tmp_port].bPortState = MGW_PHONE_INIT_STATE;
						}
						printf("lsb:Clear terminal %d k port %d zhuanxian flag success!\r\n", tmp_port, indx);
					}
				}
			}
		}
		
		break;	
		default:
			break;	
	}

	return DRV_OK;
}


/* ���ݵ��籣�浽�����ļ��еı�־�Զ�ע��ר��
kPortId: ��̨�ڱ�ţ�1-4
phonenum : ר�ߺ���


Board_716_Mng_RxThread
a0:10:80:01:00:13:42:00:10:08:05:0d:01:00:00:01:00:01:09:01:21:01:00:f4:00:
Board_Mng_SendTo_Dis
01:d0:80:01:00:00:13:42:00:10:08:05:0d:01:00:00:01:00:01:09:01:21:01:00:f4:00:
*/
int Board_Mng_RegisterZhuanXian(uint8 kPortId, char *phonenum)
{
	ST_HF_MGW_DCU_PRO Regmsg;
	MNG_ZW_ZHUANXIAN_MSG ZxCmd; 
	int len = 0;

	if ((NULL == phonenum) || (kPortId >= ZW_ZHUANXIAN_NUM))
	{
		VERBOSE_OUT(LOG_SYS,"Register zhuanxian invalid k port=%d.\r\n", kPortId);
		return DRV_ERR;
	}

	memset(&Regmsg, 0x0, sizeof(Regmsg));
	memset(&ZxCmd, 0x0, sizeof(MNG_ZW_ZHUANXIAN_MSG));
	
	ZxCmd.InfoType = 0x42;
	ZxCmd.CmdLen = 0x0010; //ע�������16���ֽ�
	ZxCmd.CmdCode = 0x08;//ָ��ר����
	ZxCmd.OpMode = EN_OP_ZWZHUANXIAN_REGIST;
	ZxCmd.CmdMsgLen = 0x0d; //ע���������ĳ���13���ֽ�
	ZxCmd.OpTypeSel = 0x01;
	ZxCmd.IndxNum = 0x00;
	ZxCmd.AckFlgSel = 0x01;
	ZxCmd.CallerFlgSel = 0x00;
	ZxCmd.CallerChanId = kPortId;
	ZxCmd.JtjId[0] = jtjno[0]; //��̬��ȡ���ž���
	ZxCmd.JtjId[1] = jtjno[1]; 
	memcpy(ZxCmd.BJPhoneNum, phonenum, 4);
	ZxCmd.ZXianStatus = 0x00;	

	memcpy(Regmsg.body, &ZxCmd, sizeof(MNG_ZW_ZHUANXIAN_MSG));

	/* �������ע�����Ϊ1210001��ͨ��1�����ž���Ϊ0901�����ı���Ϊ:
	01 80 90 01 00 00 13 42 00 10 08 01 0d 01 00 00 01 00 01 09 01 21 01 00 f1 00 
	ע����ӦΪ
	01 90 80 01 00 00 0c 42 00 09 08 04 06 01 00 00 00 00 01 
	*/
	/* ��716�������ú�����Ŀ��50���Ľ���Э�飬����ģ��50����������Ϣ����716 */
	Regmsg.header.baowen_type = 0xA0;
	Regmsg.header.dst_addr = BAOWEN_ADDR_TYPE_716_BOARD;
	Regmsg.header.src_addr = BAOWEN_ADDR_TYPE_50_BOARD;
	Regmsg.header.info_type = BAOWEN_MSG_TYPE_CMD;
	Regmsg.header.data_len = sizeof(MNG_ZW_ZHUANXIAN_MSG);

	/* ������Ϣͷ+��Ϣ�壬������Ϣ�峤�ȵ���ר��ע����Ϣ�ṹ��ĳ��� */
	len = sizeof(ST_HF_MGW_DCU_PRO_HEADER) + Regmsg.header.data_len;	
	Board_Mng_SendTo_716((uint8 *)&Regmsg, len);

	
	printf("lsb:Auto register kport %d netware zhuanxian %02x:%02x:%02x:%02x",kPortId, \
			ZxCmd.BJPhoneNum[0],ZxCmd.BJPhoneNum[1],ZxCmd.BJPhoneNum[2],ZxCmd.BJPhoneNum[3]);
	
	return DRV_OK;
}


/* ע��ר�ߣ�ֻ��Ҫͨ���� */
int Board_Mng_UnRegisterZhuanXian(uint8 kPortId)
{
	ST_HF_MGW_DCU_PRO Regmsg;
	MNG_ZWZX_UNREGISTER_MSG UnRegCmd; 
	int len = 0;

	if (kPortId >= ZW_ZHUANXIAN_NUM)
	{
		VERBOSE_OUT(LOG_SYS,"UnRegister zhuanxian invalid k port=%d.\r\n", kPortId);
		return DRV_ERR;
	}
	
	memset(&Regmsg, 0x0, sizeof(ST_HF_MGW_DCU_PRO));
	memset(&UnRegCmd, 0x0, sizeof(MNG_ZWZX_UNREGISTER_MSG));
	
	UnRegCmd.InfoType = 0x42;
	UnRegCmd.CmdLen = 0x000C; //�����13���ֽ�
	UnRegCmd.CmdCode = 0x08;//ָ��ר����
	UnRegCmd.OpMode = EN_OP_ZWZHUANXIAN_UNREGIST;
	UnRegCmd.CmdMsgLen = 0x06; //�����ĳ���6���ֽ�
	UnRegCmd.OpTypeSel = 0x01;
	UnRegCmd.IndxNum = 0x00;
	UnRegCmd.AckFlgSel = 0x01;
	UnRegCmd.CallerFlgSel = 0x00;
	UnRegCmd.CallerChanId = kPortId;

	/* �������ע�����Ϊ1210001��ͨ��1�����ž���Ϊ0923�����ı���Ϊ:
	01 80 90 01 00 00 13 42 00 10 08 01 0d 01 00 00 01 00 01 09 23 21 01 00 f1 00
	*/
	/* ��716�������ú�����Ŀ��50���Ľ���Э�飬����ģ��50����������Ϣ����716 */
	Regmsg.header.baowen_type = 0xA0;
	Regmsg.header.dst_addr = BAOWEN_ADDR_TYPE_716_BOARD;
	Regmsg.header.src_addr = BAOWEN_ADDR_TYPE_50_BOARD;
	Regmsg.header.info_type = BAOWEN_MSG_TYPE_CMD;
	Regmsg.header.data_len = sizeof(UnRegCmd);
		
	len = sizeof(ST_HF_MGW_DCU_PRO_HEADER) + Regmsg.header.data_len;
	Board_Mng_SendTo_716((uint8 *)&Regmsg, len);

	return DRV_OK;
}

/*ע������ר��
�������������ע������:
01 80 90 01 00 00 0c 42 00 09 08 03 06 01 00 00 01 00 01 
�յ���ע��Ӧ��
01 90 80 01 00 00 0c 42 00 09 08 06 06 01 00 00 00 00 01
*/
int Board_Mng_UnRegister_All_ZhuanXian(void)
{
   	ST_HF_MGW_DCU_PRO Regmsg;
	MNG_ZWZX_UNREGISTER_MSG UnRegAll;
	
	int len = 0;
	
	memset(&Regmsg, 0x0, sizeof(ST_HF_MGW_DCU_PRO));
	memset(&UnRegAll, 0x0, sizeof(UnRegAll));
	
	UnRegAll.InfoType = 0x42;
	UnRegAll.CmdLen = 0x000C; //�����13���ֽ�
	UnRegAll.CmdCode = 0x09;//����ר��
	UnRegAll.OpMode = EN_OP_ZWZHUANXIAN_UNREGIST;
	UnRegAll.CmdMsgLen = 0x06;
	UnRegAll.OpTypeSel = 0x01;
	UnRegAll.IndxNum = 0x00;
	UnRegAll.AckFlgSel = 0x00;
	UnRegAll.CallerFlgSel = 0x00;
	UnRegAll.CallerChanId = 0x00;

	/* �������ע��������������ı���Ϊ
	01 80 90 01 00 00 13 42 00 10 09 03 03 01 00 00 00 00 00 00 00 f0 ff ff ff 00
	*/
	/* ��716�������ú�����Ŀ��50���Ľ���Э�飬����ģ��50����������Ϣ����716 */
	Regmsg.header.baowen_type = 0xA0;
	Regmsg.header.dst_addr = BAOWEN_ADDR_TYPE_716_BOARD;
	Regmsg.header.src_addr = BAOWEN_ADDR_TYPE_50_BOARD;
	Regmsg.header.info_type = BAOWEN_MSG_TYPE_CMD;
	Regmsg.header.data_len = sizeof(UnRegAll);

	/* ��Ϣͷ+��Ϣ�壬������Ϣ�峤�ȵ���ר��ע����Ϣ�ṹ��ĳ��� */
	len = sizeof(ST_HF_MGW_DCU_PRO_HEADER) + Regmsg.header.data_len;
	Board_Mng_SendTo_716((uint8 *)&Regmsg, len);
	
	return DRV_OK;
}


/* 
�����������ҵ��ӿڰ�Ĳ�ѯָ��ר��(ͨ��1)�ı���Ϊ
01 80 90 01 00 00 0c 42 00 09 08 02 06 01 00 00 01 00 01 //����
01 90 80 01 00 00 0c 42 00 09 08 05 06 01 00 00 00 00 01 //ͨ��1��ר��ʱ�Ĳ�ѯ��Ӧ

01 90 80 01 00 00 13 42 00 10 08 05 0d 01 00 00 01 00 01 09 01 21 01 00 f4 00  //ͨ��1��ר��ʱ�Ĳ�ѯ��Ӧ

ҵ��ӿڰ巢�͸�716�Ĳ�ѯ������ϢΪ:
a0:80:10:01:00:0c:42:00:09:08:02:06:01:00:00:01:00:01:
ҵ��ӿڰ��յ�716����Ӧ��ϢΪ:Board_716_Mng_RxThread
a0:10:80:01:00:13:42:00:10:08:05:0d:01:00:00:01:00:01:09:01:21:01:00:f4:00:
*/
int Board_Mng_Query_SpecificZhuanXian(uint8 kPortId)
{
	ST_HF_MGW_DCU_PRO Regmsg;
	MNG_ZWZX_QUERY_MSG ZxQuery;
	int len = 0;

	printf("%s port =%d\r\n", __func__, kPortId);
	
	memset(&Regmsg, 0x0, sizeof(ST_HF_MGW_DCU_PRO));
	memset(&ZxQuery, 0x0, sizeof(ZxQuery));
	
	ZxQuery.InfoType = 0x42;
	ZxQuery.CmdLen = 0x000C; //��ѯ�����13���ֽ�
	ZxQuery.CmdCode = 0x08; //ָ��ר��
	ZxQuery.OpMode = EN_OP_ZWZHUANXIAN_QUERY;
	ZxQuery.CmdMsgLen = 0x06; //��ѯ�������ĳ���6���ֽ�
	ZxQuery.OpTypeSel = 0x01;
	ZxQuery.IndxNum = 0x00;
	ZxQuery.AckFlgSel = 0x00;
	ZxQuery.CallerFlgSel = 0x00;
	ZxQuery.CallerChanId = kPortId;

	/* ��716�������ú�����Ŀ��50���Ľ���Э�飬����ģ��50����������Ϣ����716 */
	Regmsg.header.baowen_type = 0xA0;
	Regmsg.header.dst_addr = BAOWEN_ADDR_TYPE_716_BOARD;
	Regmsg.header.src_addr = BAOWEN_ADDR_TYPE_50_BOARD;
	Regmsg.header.info_type = BAOWEN_MSG_TYPE_CMD;
	Regmsg.header.data_len = sizeof(ZxQuery);

	len = sizeof(ST_HF_MGW_DCU_PRO_HEADER) + Regmsg.header.data_len;
	Board_Mng_SendTo_716((uint8 *)&Regmsg, len);
	
	return DRV_OK;
}

/* 
���������ѯ������������ı���Ϊ
01 80 90 01 00 00 13 (42 00 10 09 02 03 01 00 00 00 00 00 00 00 f0 ff ff ff 00) 
����Ϣ����Ӧ��Ϣ�е�������Ϊ0�����������������+1�Ĳ�ѯ��Ϣ���ܵõ���һ��ר����Ӧ��
����ѭ�����ܲ�ѯ����ר�߽����
*/
int Board_Mng_Query_All_ZhuanXian(uint16 IdNum)
{
	ST_HF_MGW_DCU_PRO Regmsg;
	MNG_ZWZX_QUERY_MSG QueryCmd;
	
	//uint8 BJPhoneNumTmp[4]={0xf0,0xff,0xff,0xff};
	int len = 0;
	
	memset(&Regmsg, 0x0, sizeof(ST_HF_MGW_DCU_PRO));
	memset(&QueryCmd, 0x0, sizeof(QueryCmd));
	
	QueryCmd.InfoType = 0x42;
	QueryCmd.CmdLen = 0x000C; //��ѯ�����13���ֽ�
	QueryCmd.CmdCode = 0x09; //����ר��
	QueryCmd.OpMode = EN_OP_ZWZHUANXIAN_QUERY;
	QueryCmd.CmdMsgLen = 0x03; //ע���������ĳ���13���ֽ�
	QueryCmd.OpTypeSel = 0x01;
	QueryCmd.IndxNum = IdNum;
	QueryCmd.AckFlgSel = 0x00;
	QueryCmd.CallerFlgSel = 0x00;
	QueryCmd.CallerChanId = 0x00;

	/* ��716�������ú�����Ŀ��50���Ľ���Э�飬����ģ��50����������Ϣ����716 */
	Regmsg.header.baowen_type = 0xA0;
	Regmsg.header.dst_addr = BAOWEN_ADDR_TYPE_716_BOARD;
	Regmsg.header.src_addr = BAOWEN_ADDR_TYPE_50_BOARD;
	Regmsg.header.info_type = BAOWEN_MSG_TYPE_CMD;
	Regmsg.header.data_len = sizeof(QueryCmd);
	
	len = sizeof(ST_HF_MGW_DCU_PRO_HEADER) + Regmsg.header.data_len;
	Board_Mng_SendTo_716((uint8 *)&Regmsg, len);
	
	return DRV_OK;
}




/* 
ÿ���ϵ�ʱ�������Զ�����ָ��ר�ߵĲ�ѯ���󣬸��ݲ�ѯ��Ӧ������ñ��ص�ר�߱�ʶ��
*/
int Board_Mng_ZhuanXianInit(void)
{
	static uint8 bFirst = 0;	
	int i = 0;

	/* ���û�����ע����Ϣ�����֧�е��ã��û�����ע���2�Σ���1��ֻ�ܻ�ȡ10�����룬
	   ��2�λ�ȡ6�����롣���ѯר�ߵ���Ӧ��Ϣ�У�����ݺ�������ר�߱�ʶ���������ȴ�16������
	   �·���ɺ�ŷ�ר�߲�ѯ��Ϣ��
	*/	
	if (1 == bFirst)
	{
		for(i = 1;i <= ZW_ZHUANXIAN_NUM;i++)
		{		
			//Board_Mng_Query_SpecificZhuanXian(i);
		}

		 bFirst = 0;//�����־
	}

	bFirst += 1;//��1�ε��ü�1����֤��2�η���ѯ����

	return DRV_OK;
}



