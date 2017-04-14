/*
==========================================================
**	Description:总线收发，也即本地网络数据的收发
**	从数据交换板送来的数据
==========================================================
*/
#include "PUBLIC.h"

/*端口及IP信息由配置文件确定*/
#define		RTP_START		30000
#define		RTP_END		50000 
#define 		SNMP_AGENT_834_PORT	50002




UDP_SOCKET gIpcSemSocket;	/*scu--socket*/

//UDP_SOCKET scu_js_udp_socket;

UDP_SOCKET gIpcDatSocket;	/*dcu--socket*/
UDP_SOCKET ext_udp_socket;
UDP_SOCKET ext_rtp_socket;

int 	gRpcDbgSocket = 0;
int 	gRpcDisSocket = 0;
int		gRpcDisSocket_zhuangjia = 0;
int		gRpcDisSocket_zhuangjia_from = 0;
int32	gRpcSeatSocket = 0;
int32	gRpcVoiceSocket = 0;
int 	gRpcSeatMngSocket = 0;
int 	gRpcSeatMng_default_Socket = 0;
int 	gRpcBoardMngSocket_716 = 0;
int 	gRpcBoardMngSocket_716_Radio = 0;
int 	gRpcBoardMngSocket_716_Ray = 0;
int 	gRpcBoardMngSocket_50 = 0;
int 	gRpcBoardMngSocket_50_before = 0;

struct sockaddr_in SocketT_DbgIP;	
struct sockaddr_in SocketT_DisPlayIP;	
struct sockaddr_in SocketT_SeatMngIP;	
struct sockaddr_in SocketT_default_SeatMngIP;
struct sockaddr_in SocketT_834BoardIP;
struct sockaddr_in	SocketT_834SnmpAgent;
struct sockaddr_in SocketT_DisPlayIP_zhuangjia;
struct sockaddr_in SocketT_DisPlayIP_716;

struct sockaddr_in SocketT_BoardMngIP_716;	
struct sockaddr_in SocketT_BoardMngIP_716_Radio;	
struct sockaddr_in SocketT_BoardMngIP_716_Ray;	
struct sockaddr_in SocketT_BoardMngIP_50;	
struct sockaddr_in SocketT_BoardMngIP_50_before;	

struct sockaddr_in Terminal_I_IP;	//指控I型信令集成IP
struct sockaddr_in Terminal_I_IPV;	//指控I型话音集成IP

/*=================rtp variable====================*/
static unsigned short mgwrtpend,mgwrtpstart;
static struct io_context	*rtp_ioc = NULL;


char	flag_dump = DUMP_CLOSE;
static char rawdata[EXTMAXDATASIZE];

int32 Board_Mng_SendTo_Display(uint8 *buf, int32 len)
{
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

	Socket_Send(gRpcDisSocket_zhuangjia, &SocketT_DisPlayIP_zhuangjia,buf, len);
		
	
	return DRV_OK;
}

int32 Board_Mng_SendTo_834(uint8 *buf, int32 len)
{
	int i = 0;

	if(1)//flag_dump == DUMP_OPEN)
	{
		printf("%s\r\n", __func__);
		for(i = 0; i < len; i++)
		{
			printf("%02x:", buf[i]);
		}
		printf("\r\n");
	}

	Socket_Send(gRpcSeatMngSocket, (struct sockaddr_in*)&SocketT_834BoardIP, buf, len);
		
	//Dbg_Socket_SendToPc(MONITOR_I_50_SEM_BIT, buf, len);
	
	return DRV_OK;
}

int32 Board_Mng_SendTo_SnmpAgent(uint8 *buf, int32 len)
{
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

	Socket_Send(gRpcSeatMngSocket, (struct sockaddr_in*)&SocketT_834SnmpAgent, buf, len);
	printf("%s_%d ,send to , addr=%s, port=%d, len=%d\r\n",__func__,__LINE__,inet_ntoa(SocketT_834SnmpAgent.sin_addr),ntohs(SocketT_834SnmpAgent.sin_port),len);	
	//Dbg_Socket_SendToPc(MONITOR_I_50_SEM_BIT, buf, len);
	
	return DRV_OK;
}


int32 Board_Mng_SendTo_716(uint8 *buf, int32 len)
{
	int i = 0;

	if(1)//flag_dump == DUMP_OPEN)
	{
		printf("%s\r\n", __func__);
		for(i = 0; i < len; i++)
		{
			printf("%02x:", buf[i]);
		}
		printf("\r\n");
	}

	Socket_Send(gRpcBoardMngSocket_716, (struct sockaddr_in*)&SocketT_BoardMngIP_716, buf, len);
		
	//Dbg_Socket_SendToPc(MONITOR_I_50_SEM_BIT, buf, len);
	
	return DRV_OK;
}

int32 Board_Mng_SendTo_50(uint8 *buf, int32 len)
{
	int i = 0;
	int ret = -1;
	
	if(flag_dump == DUMP_OPEN)
	{
		printf("%s\r\n", __func__);
		for(i = 0; i < len; i++)
		{
			printf("%02x:", buf[i]);
		}
		printf("\r\n");
	}

	ret = Socket_Send(gRpcBoardMngSocket_50, (struct sockaddr_in*)&SocketT_BoardMngIP_50, buf, len);
	if(DRV_ERR == ret)
	{
		printf("%s send error\r\n", __func__);
		return DRV_ERR;
	}
	
	//Dbg_Socket_SendToPc(MONITOR_I_50_SEM_BIT, buf, len);
	
	return DRV_OK;
}

int32 Board_Mng_SendTo_716_Ray(uint8 *buf, int32 len)
{
	if(1)//flag_dump == DUMP_OPEN)
	{
		int i = 0;
		printf("%s\r\n", __func__);
		for(i = 0; i < len; i++)
		{
			printf("%02x:", buf[i]);
		}
		printf("\r\n");
	}

	Socket_Send(gRpcBoardMngSocket_716_Ray, (struct sockaddr_in*)&SocketT_BoardMngIP_716_Ray, buf, len);
	
	return DRV_OK;
}

int32 Board_Mng_SendTo_716_Radio(uint8 *buf, int32 len)
{
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

	Socket_Send(gRpcBoardMngSocket_716_Radio, (struct sockaddr_in*)&SocketT_BoardMngIP_716_Radio, buf, len);
		
	//Dbg_Socket_SendToPc(MONITOR_I_50_SEM_BIT, buf, len);
	
	return DRV_OK;
}

int32 RpcSeatMng_SendTo_Seat(uint8 *buf, int32 len)
{
	int i = 0;
	int ret = 0;
	
	if(flag_dump == DUMP_OPEN)
	{
		printf("%s\r\n", __func__);
		for(i = 0; i < len; i++)
		{
			printf("%02x:", buf[i]);
		}
		printf("\r\n");
	}

	ret = Socket_Send(gRpcSeatMngSocket, (struct sockaddr_in*)&SocketT_SeatMngIP, buf, len);
	if(DRV_ERR == ret)
	{
		printf("%s send error\r\n", __func__);
		return DRV_ERR;
	}
	//Dbg_Socket_SendToPc(MONITOR_I_50_SEM_BIT, buf, len);
	
	return DRV_OK;
}

int32 RpcSeatMng_Default_SendTo_Seat(uint8 *buf, int32 len)
{
	int i = 0;
	int ret = 0;
	
	if(flag_dump == DUMP_OPEN)
	{
		printf("%s\r\n", __func__);
		for(i = 0; i < len; i++)
		{
			printf("%02x:", buf[i]);
		}
		printf("\r\n");
	}
	printf("\r\n ---------%s_%d----------- =%d \r\n",__func__,__LINE__,gRpcSeatMng_default_Socket);

	ret = Socket_Send(gRpcSeatMng_default_Socket, (struct sockaddr_in*)&SocketT_default_SeatMngIP, buf, len);
	if(DRV_ERR == ret)
	{
		printf("%s send error\r\n", __func__);
		return DRV_ERR;
	}
	
	return DRV_OK;
}



int socket_new(UDP_SOCKET *sck)
{
#if 0
	const int reuseFlag = 1;

	sck->udp_handle = socket(AF_INET,SOCK_DGRAM,0);
	if(sck->udp_handle < 0)
	{
		ERR("Error on socket():%s\n",strerror(errno));
		return -1;
	}
	
	sck->addr_len = sizeof(struct sockaddr_in);
	
	bzero(&sck->addr_us,sck->addr_len);
	bzero(&sck->addr_them,sck->addr_len);

	sck->addr_us.sin_family = AF_INET;
	sck->addr_us.sin_port = htons(sck->local_port);
	sck->addr_us.sin_addr.s_addr = INADDR_ANY;

	sck->addr_them.sin_family = AF_INET;
	sck->addr_them.sin_port = htons(sck->remote_port);
	sck->addr_them.sin_addr.s_addr = inet_addr(sck->remote_ip);

	setsockopt(sck->udp_handle,SOL_SOCKET,SO_BROADCAST,
					(const char*)&reuseFlag,
					sizeof(reuseFlag));
	//setsockopt(sck->udp_handle, SOL_SOCKET, SO_REUSEADDR,
	//			   (const char*)&reuseFlag,
	//			   sizeof(reuseFlag));
	
	/*bind addr*/
	#if 0
	if(bind(sck->udp_handle,(struct sockaddr*)&sck->addr_us,sck->addr_len) != 0)
	{
		/*some error happen*/
		VERBOSE_OUT(LOG_SYS,"Bind Error:%s\n",strerror(errno));
		return -1;
	}
	#endif
	return sck->udp_handle;
#endif	

	return DRV_OK;
}


int socket_new_with_bindaddr(UDP_SOCKET *sck)
{
	const int reuseFlag = 1;
	sck->udp_handle = socket(AF_INET,SOCK_DGRAM,0);
	if(sck->udp_handle < 0)
	{
		ERR("Error on socket create:%s\n",sck->local_ip);
		return DRV_ERR;
	}

	sck->addr_len = sizeof(struct sockaddr_in);
	
	bzero(&sck->addr_us,sck->addr_len);
	bzero(&sck->addr_them,sck->addr_len);

	sck->addr_them.sin_family = AF_INET;
	sck->addr_them.sin_port = htons(sck->remote_port);
	sck->addr_them.sin_addr.s_addr = inet_addr(sck->remote_ip);

	sck->addr_us.sin_family = AF_INET;
	sck->addr_us.sin_port = htons(sck->local_port);
	sck->addr_us.sin_addr.s_addr = inet_addr(sck->local_ip);

	/*设置socket选项*/
	setsockopt(sck->udp_handle, SOL_SOCKET, SO_REUSEADDR,
				   (const char*)&reuseFlag,
				   sizeof(reuseFlag));
	setsockopt(sck->udp_handle,SOL_SOCKET,SO_BROADCAST,
					(const char*)&reuseFlag,
					sizeof(reuseFlag));

	/*bind addr*/
	if(bind(sck->udp_handle,(struct sockaddr*)&sck->addr_us,sck->addr_len) != 0)
	{
		/*some error happen*/
		if(errno != EADDRINUSE)
		{
			ERR("Socket Bind Error:%s\n", sck->local_ip);
			return DRV_ERR;
		}
	}

	return sck->udp_handle;
}


int socket_close(UDP_SOCKET *sck)
{
	if(!sck->udp_handle)
	{
		ERR("Invalid udp handle!\n");
		return DRV_ERR;
	}

	close(sck->udp_handle);
	sck->udp_handle = 0;

	return DRV_OK;
}

int socket_recreate(UDP_SOCKET *sck)
{
	/*销毁原socket相关资源*/
	inc_io_remove(sck->ioc,sck->ioc_id);
	socket_close(sck);

	if(!strcasecmp(sck->name,"ext"))
	{
		sck->local_ip = find_config_var(mgw_cfg,"EXTEND_LOCAL_IP");
		sck->local_port = atoi(find_config_var(config_cfg,"EXTEND_LOCAL_PORT"));
		sck->remote_ip = find_config_var(config_cfg,"EXTEND_REMOTE_IP");
		sck->remote_port = atoi(find_config_var(config_cfg,"EXTEND_REMOTE_PORT"));
		if(socket_new_with_bindaddr(sck)<0)
		{
			return DRV_ERR;
		}
		sck->ioc_id = inc_io_add(sck->ioc,sck->udp_handle,lan_bus_ctrl,POLLIN,sck);
	}

#if 0
	else if(!strcasecmp(sck->name,"js"))
	{
		//sck->local_ip = find_config_var(cfg,"JS_LOCAL_IP");
		sck->local_ip = "0.0.0.0";
		sck->local_port = atoi(find_config_var(mgw_cfg,"JS_LOCAL_PORT"));
		sck->remote_ip = find_config_var(mgw_cfg,"JS_REMOTE_IP");
		sck->remote_port = atoi(find_config_var(mgw_cfg,"JS_REMOTE_PORT"));	
		if(socket_new_with_bindaddr(sck)<0)
			return -1;			
		sck->ioc_id = inc_io_add(sck->ioc,sck->udp_handle,lan_bus_ctrl,POLLIN,sck);
	}
	else if(!strcasecmp(sck->name,"js_data"))
	{
		//sck->local_ip = find_config_var(mgw_cfg,"JS_LOCAL_IP");
		sck->local_ip = "0.0.0.0";
		sck->local_port = atoi(find_config_var(mgw_cfg,"JS_LOCAL_PORT"))+1;
		sck->remote_ip = find_config_var(mgw_cfg,"JS_REMOTE_IP");
		sck->remote_port = atoi(find_config_var(mgw_cfg,"JS_REMOTE_PORT"))+1;	
		if(socket_new_with_bindaddr(sck)<0)
			return -1;		
		sck->ioc_id = inc_io_add(sck->ioc,sck->udp_handle,lan_bus_ctrl,POLLIN,sck);
	}
#endif
	else if(!strcasecmp(sck->name,"extrtp"))
	{
		sck->local_ip = get_config_var_str(mgw_cfg,"EXTEND_LOCAL_IP");
		sck->local_port = get_config_var_val(config_cfg,"RTP_START");
		sck->remote_ip = get_config_var_str(config_cfg,"EXTEND_REMOTE_IP");
		sck->remote_port = get_config_var_val(config_cfg,"RTP_START");
		if(socket_new(sck)<0)
		{
			return DRV_ERR;
		}
	}
	
	return DRV_OK;
}


void RpcDbg_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 abuf[600];	
	int32 Rec_Len;
	uint32 len;
		
	printf("%s:start\r\n", __func__);
	
	len = sizeof(remote_addr);
	
	while (1)
	{
		if((Rec_Len = recvfrom(gRpcDbgSocket, abuf, 600, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{

			if (SocketT_DbgIP.sin_addr.s_addr != remote_addr.sin_addr.s_addr)
			{
				SocketT_DbgIP.sin_addr.s_addr = remote_addr.sin_addr.s_addr;
			}

			if (SocketT_DbgIP.sin_port != remote_addr.sin_port)
			{
				SocketT_DbgIP.sin_port = remote_addr.sin_port;
			}

			DBG("%s: rec\r\n", __func__);
			
			DbgPara_Process((unsigned char *)abuf, Rec_Len);
		}
	}
}


int32 RpcDbg_Socket_init(void)
{
	struct sockaddr_in my_addr;

	/* don't set remote socket, dynaic config  */
	memset(&my_addr, 0x00, sizeof(my_addr));
	memset(&SocketT_DbgIP, 0x00, sizeof(SocketT_DbgIP));

	/* local socket setting  */
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htonl(33333);
	
	if((WORK_MODE_PAOBING == Param_update.workmode)||(WORK_MODE_ZHUANGJIA == Param_update.workmode))
	{
		my_addr.sin_addr.s_addr = htonl(inet_addr(get_config_var_str(mgw_cfg,"EXTEND_LOCAL_IP")));
	}
	else if(WORK_MODE_JILIAN == Param_update.workmode)
	{
		my_addr.sin_addr.s_addr = htonl(inet_addr(get_config_var_str(config_cfg,"EXTEND_LOCAL_SEAT_IP")));
	}
	
	if ((gRpcDbgSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("cann't create Rpc-Dbg socket.\n");
		return DRV_ERR;
	}

	if (DRV_ERR == bind(gRpcDbgSocket, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("Bind Rpc-Dbg socket failed.\n");
		close(gRpcDbgSocket);
		return DRV_ERR;
	}
			
	return DRV_OK;
}

/* Added by lishibing 20150923,炮兵模式调试新增代码
业务板统一完成键显板和席位网管消息的转发。
(1)炮兵模式下: 业务板10.0.0.3与键显板网管，席位网管交互，并通过192.168.254.3 与炮兵模式下的50所单板沟通网管消息；
(1)装甲模式下: 业务板10.0.0.3与键显板网管，席位网管交互，并通过192.168.254.3 与716单板沟通网管消息沟通网管消息；
*/
void RpcDisplayMng_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 abuf[600];	
	int32 Rec_Len;
	uint32 len;
	int i;
		
	printf("%s:start\r\n", __func__);
	
	len = sizeof(remote_addr);
	
	while (1)
	{
		if((Rec_Len = recvfrom(gRpcDisSocket, abuf, 600, 0,(struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{	

#if 0
			if (SocketT_DisPlayIP.sin_addr.s_addr != remote_addr.sin_addr.s_addr)
			{
				SocketT_DisPlayIP.sin_addr.s_addr = remote_addr.sin_addr.s_addr;
			}

			if (SocketT_DisPlayIP.sin_port != remote_addr.sin_port)
			{
				//SocketT_DisPlayIP.sin_port = remote_addr.sin_port;
			}
#endif

			if(flag_dump == DUMP_OPEN)
			{
				printf("%s\r\n", __func__);
				for(i = 0; i < Rec_Len; i++)
				{
					printf("%02x:", abuf[i]);
				}
				printf("\r\n");	
			}
	
			Board_Mng_Process(abuf, Rec_Len);
		}
	}
}

int32 snmpAgentMng_Socket_init(void){
	SocketT_834SnmpAgent.sin_family = AF_INET;
	SocketT_834SnmpAgent.sin_addr.s_addr = htonl(inet_addr("127.0.0.1"));
	SocketT_834SnmpAgent.sin_port = htonl(SNMP_AGENT_834_PORT);
}

int32 RpcDisplayMng_Socket_init(void)
{
	struct sockaddr_in my_addr;

	/* don't set remote socket, dynaic config  */
	memset(&my_addr, 0x00, sizeof(my_addr));

	/* local socket setting  */
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htonl(50001);

	SocketT_DisPlayIP.sin_family = AF_INET;
	SocketT_DisPlayIP.sin_addr.s_addr = htonl(inet_addr(get_config_var_str(config_cfg,"DEFAULT_DISPLAY_IP")));
	SocketT_DisPlayIP.sin_port = htonl(50001);

	/* Modified by lishibing 20151017 14:00，Bug Fix: 当设备处于席位扩展模式时，
	键显板不能设置模式。增加席位扩展模式
	*/
	if((WORK_MODE_PAOBING == Param_update.workmode) ||
	   (WORK_MODE_ZHUANGJIA == Param_update.workmode)  ||
	   (WORK_MODE_ADAPTER == Param_update.workmode))
	{
		my_addr.sin_addr.s_addr = htonl(inet_addr(get_config_var_str(config_cfg,"DEFAULT_LOCAL_IP")));
	}	
	else if(WORK_MODE_JILIAN == Param_update.workmode)	
	{
		my_addr.sin_addr.s_addr = htonl(inet_addr(get_config_var_str(config_cfg,"DEFAULT_SEAT_LOCAL_IP")));
	}
	
	if ((gRpcDisSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("cann't create Rpc-display socket.\n");
		return DRV_ERR;
	}

	if (DRV_ERR == bind(gRpcDisSocket, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("Bind Rpc-display socket failed.\n");
		close(gRpcDisSocket);
		return DRV_ERR;
	}

	return DRV_OK;
}

int32 RpcDisplayMng_Socket_Close(void)
{
	if(gRpcDisSocket)
	{
		close(gRpcDisSocket);
	}

	return DRV_OK;
}


void RpcDisplayMng_Zhuangjia_From_dis_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 abuf[600];	
	int32 Rec_Len;
	uint32 len;
	int i;
		
	printf("%s:start\r\n", __func__);
	
	len = sizeof(remote_addr);
	
	while (1)
	{
		memset(abuf, 0x00, sizeof(abuf));		

		if((Rec_Len = recvfrom(gRpcDisSocket_zhuangjia, abuf, 600, 0,(struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{	
			if(flag_dump == DUMP_OPEN)
			{
				printf("%s : ipaddr = 0x%x\r\n", __func__, remote_addr.sin_addr.s_addr);
				for(i = 0; i < Rec_Len; i++)
				{
					printf("%02x:", abuf[i]);
				}
				printf("\r\n");	
			}
			

			Socket_Send(gRpcDisSocket_zhuangjia_from, &SocketT_DisPlayIP_716, abuf, Rec_Len);

		}
	}
}

int32 RpcDisplayMng_Zhuangjia_From_dis_Socket_init(void)
{
	struct sockaddr_in my_addr;

	/* don't set remote socket, dynaic config  */
	memset(&my_addr, 0x00, sizeof(my_addr));


	SocketT_DisPlayIP_716.sin_family = AF_INET;
	SocketT_DisPlayIP_716.sin_addr.s_addr = htonl(inet_addr("192.168.254.1"));
	SocketT_DisPlayIP_716.sin_port = htonl(50721);	

	/* local socket setting  */
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htonl(50721);
	my_addr.sin_addr.s_addr = htonl(inet_addr("192.168.254.4"));

	if ((gRpcDisSocket_zhuangjia = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("cann't create Rpc-display socket.\n");
		return DRV_ERR;
	}

	if (DRV_ERR == bind(gRpcDisSocket_zhuangjia, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("Bind Rpc-display socket failed.\n");
		close(gRpcDisSocket_zhuangjia);
		return DRV_ERR;
	}

	return DRV_OK;
}

int32 RpcDisplayMng_Zhuangjia_from_dis_Socket_Close(void)
{
	if(gRpcDisSocket_zhuangjia)
	{
		close(gRpcDisSocket_zhuangjia);
	}

	return DRV_OK;
}

void RpcDisplayMng_Zhuangjia_from_716_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 abuf[600];	
	int32 Rec_Len;
	uint32 len;
	int i;
		
	printf("%s:start\r\n", __func__);
	
	len = sizeof(remote_addr);
	
	while (1)
	{
		memset(abuf, 0x00, sizeof(abuf));		

		if((Rec_Len = recvfrom(gRpcDisSocket_zhuangjia_from, abuf, 600, 0,(struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{	
			if(flag_dump == DUMP_OPEN)
			{
				printf("%s : ipaddr = 0x%x\r\n", __func__, remote_addr.sin_addr.s_addr);
				for(i = 0; i < Rec_Len; i++)
				{
					printf("%02x:", abuf[i]);
				}
				printf("\r\n");	
			}
			
			Socket_Send(gRpcDisSocket_zhuangjia, &SocketT_DisPlayIP_zhuangjia, abuf, Rec_Len);
		}
	}
}

int32 RpcDisplayMng_from_716_Socket_init(void)
{
	struct sockaddr_in my_addr;

	/* don't set remote socket, dynaic config  */
	memset(&my_addr, 0x00, sizeof(my_addr));

	/*键显板的IP和端口号*/
	SocketT_DisPlayIP_zhuangjia.sin_family = AF_INET;
	SocketT_DisPlayIP_zhuangjia.sin_addr.s_addr = htonl(inet_addr(get_config_var_str(config_cfg,"DEFAULT_DISPLAY_IP")));
	SocketT_DisPlayIP_zhuangjia.sin_port = htonl(50721);


	/* local socket setting  */
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htonl(50721);
	my_addr.sin_addr.s_addr = htonl(inet_addr(DISPLAY_BRD_MNG_LOCAL_IP));

	if ((gRpcDisSocket_zhuangjia_from = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("cann't create Rpc-display socket.\n");
		return DRV_ERR;
	}

	if (DRV_ERR == bind(gRpcDisSocket_zhuangjia_from, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("Bind Rpc-display socket failed.\n");
		close(gRpcDisSocket_zhuangjia_from);
		return DRV_ERR;
	}

	return DRV_OK;
}

int32 RpcDisplayMng_Zhuangjia_Socket_from_716_Close(void)
{
	if(gRpcDisSocket_zhuangjia_from)
	{
		close(gRpcDisSocket_zhuangjia_from);
	}

	return DRV_OK;
}


/* 
使用linli开发的网管软件V1版本，发送消息按<参数设备管理接口协议__20150414> 
(该协议实际是716与50所之间在其他项目定制的，716提供给我厂使用)，
其中战网参数目的地址为716厂战网控制单元(0x80)，业务接口板收到该目的的报文，处理方法为:
1)在装甲模式下直接发送给716单板。716单板返回的响应目的地址总为50所单板(因为在其他项目中，716只发给50所)，834收到该项目需
透传给PC网管软件。
2)在炮防模式下需转换成<专网、战网参数设备管理接口协议v010 20140805>发送给50所，50所再转发。
*/
void RpcSeatMng_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 abuf[MAX_SOCKET_LEN] = {0};
	int Rec_Len;
	unsigned int len;
	int i = 0;	
	ST_SEAT_MNG_MSG *pMsg = (ST_SEAT_MNG_MSG *)abuf;


	printf("%s start !\r\n", __func__);
	
	len = sizeof(remote_addr);
	while (1)
	{
		if((Rec_Len = recvfrom(gRpcSeatMngSocket, abuf, MAX_SOCKET_LEN, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{
			if (SocketT_SeatMngIP.sin_addr.s_addr != remote_addr.sin_addr.s_addr)
			{
				SocketT_SeatMngIP.sin_addr.s_addr = remote_addr.sin_addr.s_addr;
			}

			if (SocketT_SeatMngIP.sin_port != remote_addr.sin_port)
			{
				//g_ZjctSeatMngSockAddr.sin_port = remote_addr.sin_port;
			}

			if(flag_dump == DUMP_OPEN)
			{
				printf("%s\r\n", __func__);
				for(i = 0; i < Rec_Len; i++)
				{
					printf("%02x:", abuf[i]);
				}
				printf("\r\n");	
			}

			Board_Mng_Process(abuf, Rec_Len);

			signalQueryEvent(pMsg->body[0]);
		}
	}
}

int32 RpcSeatMng_Socket_init(void)
{
	//struct sockaddr_in my_addr;

	/* don't set remote socket, dynaic config  */
	//memset(&my_addr, 0x00, sizeof(my_addr));

	SocketT_SeatMngIP.sin_family = AF_INET;
	SocketT_SeatMngIP.sin_addr.s_addr = htonl(inet_addr("10.0.0.7"));
	SocketT_SeatMngIP.sin_port = htonl(30006);

		/* local socket setting  */
	SocketT_834BoardIP.sin_family = AF_INET;
	SocketT_834BoardIP.sin_port = htonl(30006);
	//SocketT_834BoardIP.sin_addr.s_addr = htonl(inet_a
	SocketT_834BoardIP.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if ((gRpcSeatMngSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("cann't create Rpc-display socket.\n");
		return DRV_ERR;
	}

	if (DRV_ERR == bind(gRpcSeatMngSocket, (struct sockaddr *)&SocketT_834BoardIP, sizeof(SocketT_834BoardIP)))
	{
		ERR("Bind %s\n", __func__);
		close(gRpcSeatMngSocket);
		return DRV_ERR;
	}

	return DRV_OK;
}

int32 RpcSeatMng_Socket_Close(void)
{
	if(gRpcSeatMngSocket)
	{
		close(gRpcSeatMngSocket);
	}

	return DRV_OK;
}


void RpcSeatMng_Default_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 abuf[MAX_SOCKET_LEN] = {0};
	int Rec_Len;
	unsigned int len;
	int i = 0;	

	printf("%s start !\r\n", __func__);
	
	len = sizeof(remote_addr);
	while (1)
	{
		if((Rec_Len = recvfrom(gRpcSeatMng_default_Socket, abuf, MAX_SOCKET_LEN, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{
			if (SocketT_default_SeatMngIP.sin_addr.s_addr != remote_addr.sin_addr.s_addr)
			{
				SocketT_default_SeatMngIP.sin_addr.s_addr = remote_addr.sin_addr.s_addr;
			}

			if (SocketT_default_SeatMngIP.sin_port != remote_addr.sin_port)
			{
				//g_ZjctSeatMngSockAddr.sin_port = remote_addr.sin_port;
			}

			if(flag_dump == DUMP_OPEN)
			{
				printf("%s\r\n", __func__);
				for(i = 0; i < Rec_Len; i++)
				{
					printf("%02x:", abuf[i]);
				}
				printf("\r\n");	
			}

			Board_Mng_Process(abuf, Rec_Len);		
		}
	}
}

int32 RpcSeatMng_Default_Socket_init(void)
{
	struct sockaddr_in my_addr;

	/* don't set remote socket, dynaic config  */
	memset(&my_addr, 0x00, sizeof(my_addr));

	SocketT_default_SeatMngIP.sin_family = AF_INET;
	SocketT_default_SeatMngIP.sin_addr.s_addr = htonl(inet_addr("10.0.0.7"));
	SocketT_default_SeatMngIP.sin_port = htonl(30006);

		/* local socket setting  */
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htonl(30006);
	my_addr.sin_addr.s_addr = htonl(inet_addr(get_config_var_str(config_cfg,"EXTEND_DEFAULT_IP")));
	
	if ((gRpcSeatMng_default_Socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("cann't create Rpc-display socket.\n");
		return DRV_ERR;
	}

	if (DRV_ERR == bind(gRpcSeatMng_default_Socket, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("Bind %s\n", __func__);
		close(gRpcSeatMng_default_Socket);
		return DRV_ERR;
	}

	return DRV_OK;
}

int32 RpcSeatMng_Default_Socket_Close(void)
{
	if(gRpcSeatMng_default_Socket)
	{
		close(gRpcSeatMng_default_Socket);
	}

	return DRV_OK;
}

void Board_716_Mng_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 abuf[MAX_SOCKET_LEN] = {0};
	int Rec_Len;
	unsigned int len;
	int i = 0;

	ST_SEAT_MNG_MSG msg;
	ST_HF_MGW_DCU_PRO Send;

	printf("%s start !\r\n", __func__);
	
	len = sizeof(remote_addr);
	while (1)
	{
		if((Rec_Len = recvfrom(gRpcBoardMngSocket_716, abuf, MAX_SOCKET_LEN, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{
		
			if(1)//flag_dump == DUMP_OPEN)
			{
				printf("%s\r\n", __func__);
				for(i = 0; i < Rec_Len; i++)
				{
					printf("%02x:", abuf[i]);
				}
				printf("\r\n");	
			}

			/* 
			V1: 炮防模式下,使用新网管(针对装甲新开发的网管)设置战网单元参数。
			*/
			ST_HF_MGW_DCU_PRO *rcv = (ST_HF_MGW_DCU_PRO *)abuf;
			memset(&msg, 0x00, sizeof(msg));
			memset(&Send, 0x00, sizeof(Send));
			
			switch(rcv->header.dst_addr)
			{
				case BAOWEN_ADDR_TYPE_834_BOARD:
					break;				
				case BAOWEN_ADDR_TYPE_834_DIS_BOARD:
					break;
				case BAOWEN_ADDR_TYPE_834_PC:
					break;				
				case BAOWEN_ADDR_TYPE_716_BOARD:
					break;
				case BAOWEN_ADDR_TYPE_716_RAY_BOARD:
					break;				
				case BAOWEN_ADDR_TYPE_50_BOARD:
					msg.header.protocol_version = 0x01;
					msg.header.src_addr = BAOWEN_ADDR_TYPE_716_BOARD;
					msg.header.msg_type = BAOWEN_MSG_TYPE_CMD;
					msg.header.reserve = 0x00;
					msg.header.data_len = rcv->header.data_len;
					memcpy(msg.body, rcv->body, rcv->header.data_len);

					/* 发送给席位 */
					msg.header.dst_addr = BAOWEN_ADDR_TYPE_834_PC;
					RpcSeatMng_SendTo_Seat((uint8 *)&msg, sizeof(ST_SEAT_MNG_HEADER) + msg.header.data_len);
					
					/* 炮防模式下,键显由834开发，查询整机模式和K口模式发送给显示板 */
					msg.header.dst_addr = BAOWEN_ADDR_TYPE_834_DIS_BOARD;
					Board_Mng_SendTo_Dis((uint8 *)&msg, sizeof(ST_SEAT_MNG_HEADER) + msg.header.data_len);

					/* Hook:解析专线注册、注销等消息，用于834保存专线标志 */
					if ((0x42 == msg.body[0])\
						&&(WORK_MODE_PAOBING == Param_update.workmode))
					{
						Board_Mng_ZhuanXianAckMsgProc(abuf, Rec_Len);
					}

					/* Forward Pkt to 834 Net snmp agent*/
					msg.header.dst_addr = BAOWEN_ADDR_TYPE_834_SNMP_AGENT;
					Board_Mng_SendTo_SnmpAgent((uint8 *)&msg, sizeof(ST_SEAT_MNG_HEADER) + msg.header.data_len);

					/* Deleted by lishibing 20151011，装甲和炮防模式下战网参数设置响应不需要发送给50所 */
					#if 0 
					/* 响应转发给50所 */
					Send.header.baowen_type = 0xA0;
					Send.header.dst_addr = BAOWEN_ADDR_TYPE_50_BOARD;
					Send.header.src_addr = BAOWEN_ADDR_TYPE_716_BOARD;
					Send.header.info_type = BAOWEN_MSG_TYPE_CMD;
					Send.header.data_len = rcv->header.data_len;

					memcpy(msg.body, rcv->body, rcv->header.data_len);
					Board_Mng_SendTo_50((uint8 *)&msg, sizeof(ST_HF_MGW_DCU_PRO_HEADER) + msg.header.data_len);	
					#endif

					signalQueryEvent(rcv->body[0]);

					break;	
				default:
					break;
			}			
		}
	}
}

int32 Board_716_Mng_Socket_init(void)
{
	struct sockaddr_in my_addr;

	/* don't set remote socket, dynaic config  */
	memset(&my_addr, 0x00, sizeof(my_addr));


	SocketT_BoardMngIP_716.sin_family = AF_INET;
	SocketT_BoardMngIP_716.sin_addr.s_addr = htonl(inet_addr("192.168.254.1"));
	SocketT_BoardMngIP_716.sin_port = htons(6060);

	/* local socket setting  */
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htonl(6050);	
	my_addr.sin_addr.s_addr = htonl(inet_addr(get_config_var_str(config_cfg,"DEFAULT_LOCAL_IP")));	
	
	if ((gRpcBoardMngSocket_716 = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("cann't create Rpc-display socket.\n");
		return DRV_ERR;
	}

	if (DRV_ERR == bind(gRpcBoardMngSocket_716, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("Bind %s\n", __func__);
		close(gRpcBoardMngSocket_716);
		return DRV_ERR;
	}

	return DRV_OK;
}

int32 Board_716_Mng_Socket_Close(void)
{
	if(gRpcBoardMngSocket_716)
	{
		close(gRpcBoardMngSocket_716);
	}

	return DRV_OK;
}


void Board_716_Radio_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 abuf[MAX_SOCKET_LEN] = {0};
	int Rec_Len;
	unsigned int len;

	printf("%s start !\r\n", __func__);
	
	len = sizeof(remote_addr);
	while (1)
	{
		if((Rec_Len = recvfrom(gRpcBoardMngSocket_716_Radio, abuf, MAX_SOCKET_LEN, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{
			if(WORK_MODE_ZHUANGJIA == Param_update.workmode)
			{
				Sw_Phone_Mng_Send_by_IP(abuf, Rec_Len, 0);
			}
		}
	}
}

int32 Board_716_Radio_Socket_init(void)
{
	struct sockaddr_in my_addr;

	/* don't set remote socket, dynaic config  */
	memset(&my_addr, 0x00, sizeof(my_addr));


	SocketT_BoardMngIP_716_Radio.sin_family = AF_INET;
	SocketT_BoardMngIP_716_Radio.sin_addr.s_addr = htonl(inet_addr("192.168.254.1"));
	SocketT_BoardMngIP_716_Radio.sin_port = htonl(50725);

	/* local socket setting  */
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htonl(50725);	
	my_addr.sin_addr.s_addr = htonl(inet_addr(get_config_var_str(config_cfg,"DEFAULT_LOCAL_IP")));	
	
	if ((gRpcBoardMngSocket_716_Radio = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("cann't create 716_Radio_Socket socket.\n");
		return DRV_ERR;
	}

	if (DRV_ERR == bind(gRpcBoardMngSocket_716_Radio, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("Bind %s\n", __func__);
		close(gRpcBoardMngSocket_716_Radio);
		return DRV_ERR;
	}

	return DRV_OK;
}

int32 Board_716_Radio_Socket_Close(void)
{
	if(gRpcBoardMngSocket_716_Radio)
	{
		close(gRpcBoardMngSocket_716_Radio);
	}

	return DRV_OK;
}


void Board_716_Ray_Mng_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 abuf[MAX_SOCKET_LEN] = {0};
	int Rec_Len;
	unsigned int len;

	ST_SEAT_MNG_MSG msg;

	printf("%s start !\r\n", __func__);
	
	len = sizeof(remote_addr);
	while (1)
	{
		if((Rec_Len = recvfrom(gRpcBoardMngSocket_716_Ray, abuf, MAX_SOCKET_LEN, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{
#if 1	
			int i = 0;
			printf("%s\r\n", __func__);
			for(i = 0; i < Rec_Len; i++)
			{
				printf("%x:", abuf[i]);
			}
			printf("\r\n");
#endif			
			ST_RAY_MNG_MSG *rcv = (ST_RAY_MNG_MSG *)abuf;
			memset(&msg, 0x00, sizeof(msg));

			if(abuf[0] == 0xd1)
			{
				RpcSeatMng_SendTo_Seat(abuf, Rec_Len);
				
			}
				
			switch(rcv->header.msg_type)
			{
				case 0xd1:
					msg.header.protocol_version = 0x01;
					msg.header.dst_addr = BAOWEN_ADDR_TYPE_834_PC;
					msg.header.src_addr = BAOWEN_ADDR_TYPE_716_RAY_BOARD;
					msg.header.msg_type = BAOWEN_MSG_TYPE_CMD;
					msg.header.reserve = 0x00;
					msg.body[0] = rcv->header.msg_flg;
					msg.header.data_len = rcv->header.data_len + 3;
					
					if(0x00 != rcv->header.data_len)
					{
						*((uint16 *)&(msg.body[1])) = rcv->header.data_len;
						memcpy((uint8 *)&(msg.body[3]), rcv->body, rcv->header.data_len);
					}

					RpcSeatMng_SendTo_Seat((uint8 *)&msg, sizeof(ST_SEAT_MNG_HEADER) + msg.header.data_len);
					
					signalQueryEvent(rcv->header.msg_flg);
					
					break;				
				default:
					break;

			}		
		}
	}
}

int32 Board_716_Ray_Mng_Socket_init(void)
{
	struct sockaddr_in my_addr;

	/* don't set remote socket, dynaic config  */
	memset(&my_addr, 0x00, sizeof(my_addr));

	SocketT_BoardMngIP_716_Ray.sin_family = AF_INET;
	SocketT_BoardMngIP_716_Ray.sin_addr.s_addr = htonl(inet_addr("192.168.254.1"));
	SocketT_BoardMngIP_716_Ray.sin_port = htonl(50722);

	/* local socket setting  */
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htonl(50722);	
	my_addr.sin_addr.s_addr = htonl(inet_addr(get_config_var_str(config_cfg,"DEFAULT_LOCAL_IP")));	
	
	if ((gRpcBoardMngSocket_716_Ray = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("cann't create Rpc-display socket.\n");
		return DRV_ERR;
	}

	if (DRV_ERR == bind(gRpcBoardMngSocket_716_Ray, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("Bind %s\n", __func__);
		close(gRpcBoardMngSocket_716_Ray);
		return DRV_ERR;
	}

	return DRV_OK;
}

int32 Board_716_Ray_Mng_Socket_Close(void)
{
	if(gRpcBoardMngSocket_716_Ray)
	{
		close(gRpcBoardMngSocket_716_Ray);
	}

	return DRV_OK;
}


void Board_50_Mng_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 abuf[MAX_SOCKET_LEN] = {0};
	int Rec_Len;
	unsigned int len;
	int i = 0;	

	ST_HF_MGW_DCU_PRO msg;
	MNG_50_MSG *rcv = (MNG_50_MSG *)abuf;

	printf("%s start !\r\n", __func__);
	
	len = sizeof(remote_addr);
	while (1)
	{
		if((Rec_Len = recvfrom(gRpcBoardMngSocket_50, abuf, MAX_SOCKET_LEN, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{
			memset(&msg, 0x00, sizeof(msg));
		
			if(1)//flag_dump == DUMP_OPEN)
			{
				printf("%s\r\n", __func__);
				for(i = 0; i < Rec_Len; i++)
				{
					printf("%02x:", abuf[i]);
				}
				printf("\r\n");	
			}

			/* 炮防模式下，网管软件设置战网单元的参数响应消息由50所透传给834，协议格式
			<专网、战网参数设备管理接口协议v010 20140805>，834需转发给席位网管软件。
			 */
			if (WORK_MODE_PAOBING == Param_update.workmode)
			{
			#ifdef MNG_FOR_JINBIAO
				/* 解析专线注册和注销响应消息 */
				Board_Mng_ZhuanXianAckMsg50Proc(abuf, Rec_Len);

				/* 转发给PC竞标网管软件 */
				RpcSeatMng_SendTo_Seat(abuf, Rec_Len);
				
			#else
				/* 解析专线注册和注销响应消息 */
				Board_Mng_ZhuanXianAckMsgProc(abuf, Rec_Len);

				/* 转发给PC新网管软件和炮防键显板软件 */
				Board_Mng_RxFrom50Proc(abuf, Rec_Len);
			#endif
			}		
			else
			{
				printf("Invalid work mode(%d).\r\n", Param_update.workmode);
			}

			signalQueryEvent(*(uint16 *)(&rcv->Data[1]));
		}
	}
}

int32 Board_50_Mng_Socket_init(void)
{
	struct sockaddr_in my_addr;

	/* don't set remote socket, dynaic config  */
	memset(&my_addr, 0x00, sizeof(my_addr));


	SocketT_BoardMngIP_50.sin_family = AF_INET;
	SocketT_BoardMngIP_50.sin_addr.s_addr = htonl(inet_addr("192.168.254.2"));
	SocketT_BoardMngIP_50.sin_port = htonl(SEAT_MNG_PORT);

	/* local socket setting  */
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htonl(SEAT_MNG_PORT);	
	my_addr.sin_addr.s_addr = htonl(inet_addr(DISPLAY_BRD_MNG_LOCAL_IP));	
	
	if ((gRpcBoardMngSocket_50 = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("cann't create 50-Mng socket.\n");
		return DRV_ERR;
	}

	if (DRV_ERR == bind(gRpcBoardMngSocket_50, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("Bind %s\n", __func__);
		close(gRpcBoardMngSocket_50);
		return DRV_ERR;
	}

	return DRV_OK;
}

int32 Board_50_Mng_Socket_Close(void)
{
	if(gRpcBoardMngSocket_50)
	{
		close(gRpcBoardMngSocket_50);
	}

	return DRV_OK;
}


void Board_50_Mng_before_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 abuf[MAX_SOCKET_LEN] = {0};
	int Rec_Len;
	unsigned int len;
	int i = 0;	

	ST_HF_MGW_DCU_PRO msg;

	printf("%s start !\r\n", __func__);
	
	len = sizeof(remote_addr);
	while (1)
	{
		if((Rec_Len = recvfrom(gRpcBoardMngSocket_50_before, abuf, MAX_SOCKET_LEN, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{
			memset(&msg, 0x00, sizeof(msg));
		
			//if(flag_dump == DUMP_OPEN)
			{
				printf("%s\r\n", __func__);
				for(i = 0; i < Rec_Len; i++)
				{
					printf("%02x:", abuf[i]);
				}
				printf("\r\n");	
			}
		}
	}
}

int32 Board_50_Mng_before_Socket_init(void)
{
	struct sockaddr_in my_addr;

	/* don't set remote socket, dynaic config  */
	memset(&my_addr, 0x00, sizeof(my_addr));


	SocketT_BoardMngIP_50_before.sin_family = AF_INET;
	SocketT_BoardMngIP_50_before.sin_addr.s_addr = htonl(inet_addr("192.168.9.1"));
	SocketT_BoardMngIP_50_before.sin_port = htonl(9050);

	/* local socket setting  */
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htonl(9034);	
	my_addr.sin_addr.s_addr = htonl(inet_addr("192.168.9.3"));	
	
	if ((gRpcBoardMngSocket_50_before = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ERR("cann't create 50-Mng socket.\n");
		return DRV_ERR;
	}

	if (DRV_ERR == bind(gRpcBoardMngSocket_50_before, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		ERR("Bind %s\n", __func__);
		close(gRpcBoardMngSocket_50_before);
		return DRV_ERR;
	}

	return DRV_OK;
}

int32 Board_50_Mng_before_Socket_Close(void)
{
	if(gRpcBoardMngSocket_50_before)
	{
		close(gRpcBoardMngSocket_50_before);
	}

	return DRV_OK;
}


//=======================================
/************************************************************************
外部网口数据接收
************************************************************************/
int RpcSeat_Socket_init(void)
{
	struct sockaddr_in my_addr = {0};
		
	my_addr.sin_family = AF_INET;//接收来自PC机的信息
	my_addr.sin_port = htons(30000);
	my_addr.sin_addr.s_addr = htonl(inet_addr(get_config_var_str(config_cfg,"EXTEND_LOCAL_SEAT_IP")));
 	
	if ((gRpcSeatSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("can't create Rpc-seat socket.\r\n");
		return DRV_ERR;
	}

	if (DRV_ERR == bind(gRpcSeatSocket, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		printf("Bind Rpc-seat socket failed.\r\n");
		close(gRpcSeatSocket);
		return DRV_ERR;
	}

	
	return DRV_OK;
}


static int RpcSeat_Sem_Process(uint8 *buf, int32 len)
{
	ST_HF_MGW_EXT_PRO *rcv = (ST_HF_MGW_EXT_PRO *)buf;
	int wTemPort = 0;	

	switch(rcv->body[0])
	{
		case MGW_SCU_CALL_ID:
			{
				wTemPort = find_local_port_by_slot(rcv->body[2]);
				LOG("rev call req slot %d \r\n", wTemPort);
				
				if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
				{
					VERBOSE_OUT(LOG_SYS,"CAN'T FIND PORT\n");
					break;
				}
				else
				{
					VERBOSE_OUT(LOG_SYS,"FIND %d,RECV SCU CALL %d \n",rcv->body[2],wTemPort);
				}

				if(terminal_group[wTemPort].bPortType == PORT_TYPE_LOCAL_XIWEI)
				{
					terminal_group[wTemPort].bCommand = MC_RING;
					terminal_group[wTemPort].bCallType = rcv->body[3];
					bcd2phone((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
						(char *)&rcv->body[6], rcv->body[5]);
					terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = rcv->body[5];
					terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;
				}
			}
			
			break;
		case MGW_SCU_CALLACK_ID:
			{
				wTemPort = find_local_port_by_slot(rcv->body[2]);
				
				LOG("rev call ack slot %d \r\n", wTemPort);
				
				if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
				{
					VERBOSE_OUT(LOG_SYS,"CAN'T FIND PORT\n");
					break;
				}
				
				if(rcv->body[4] == EN_JIEXU_SUCCESS)
				{
					terminal_group[wTemPort].bCommand = MC_RT_TONE;
				}
				else if(rcv->body[4] == EN_JIEXU_FAILURE)
				{
					terminal_group[wTemPort].bCommand = MC_REJECT;
				}
			}
			break;

		case MGW_SCU_CONNECT_ID:
			{
				wTemPort = find_local_port_by_slot(rcv->body[2]);
				
				LOG("rev call connnet slot %d \r\n",wTemPort);

				if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
				{
					VERBOSE_OUT(LOG_SYS,"CAN'T FIND PORT\n");
					break;
				}
				
				terminal_group[wTemPort].bCallType = rcv->body[3];

				terminal_group[wTemPort].bCommand = MC_TALK;
				bcd2phone((char *)terminal_group[wTemPort].phone_queue_g.bPhoneQueue, \
					(char *)&rcv->body[6], rcv->body[5]);
				terminal_group[wTemPort].phone_queue_g.bPhoneQueueHead = rcv->body[5];
				terminal_group[wTemPort].phone_queue_g.bPhoneQueueTail = 0;						
			}
			break;

		case MGW_SCU_RELEASE_ID:
			{
				wTemPort = find_local_port_by_slot(rcv->body[2]);
				
				LOG("rev release req slot %d \r\n", wTemPort);
								
				if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
				{
					VERBOSE_OUT(LOG_SYS,"CAN'T FIND PORT\n");
					break;
				}

				terminal_group[wTemPort].bCommand = MC_HANG;

				terminal_group[wTemPort].bCallType = rcv->body[3];
				terminal_group[wTemPort].rel_reason = rcv->body[4];
				
				if(rcv->body[4] == 0xff || rcv->body[4] == 0x00)
				{
					terminal_group[wTemPort].rel_reason = MGW_RELEASE_REASON_DEVICEFAULT;
				}
			}
			break;

		case MGW_SCU_RELEASE_CNF_ID:
			{
				wTemPort = find_local_port_by_slot(rcv->body[2]);
				
				LOG("rev release req cnf slot %d \r\n", wTemPort);
										
				if((wTemPort < 0) || (wTemPort >= USER_NUM_MAX))
				{
					VERBOSE_OUT(LOG_SYS,"CAN'T FIND PORT\n");
					break;
				}
				
			}
			break;

		case MGW_SCU_PTT_ID:				
			break;
		case MGW_SCU_NUM_TRANS_ID:
			break;
		case MGW_SCU_CALL_TARNS_ID:
			break;
		default:
			break;
	}
		
	return DRV_OK;
}

static int RpcSeat_Process(uint8 *buf, int32 len)
{
	ST_HF_MGW_EXT_PRO *rcv = (ST_HF_MGW_EXT_PRO *)buf;

	switch(rcv->header.type)
	{
		case BAOWEN_TYPE_CALLCFG:
		
			break;
		case BAOWEN_TYPE_CALLCMD:
			RpcSeat_Sem_Process(buf, len);
			break;
		default:
			break;
	}

	return DRV_OK;
}

void RpcSeat_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 abuf[MAX_SOCKET_LEN] = {0};
	int Rec_Len;
	unsigned int len;
	
	printf("%s start !\r\n", __func__);
	
	len = sizeof(remote_addr);
	while (1)
	{
		if((Rec_Len = recvfrom(gRpcSeatSocket, abuf, MAX_SOCKET_LEN, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{
			RpcSeat_Process(abuf, Rec_Len);
		}
	}
}

int RpcSeat_Socket_Close(void)
{	
	
	if (gRpcSeatSocket)
	{
		close(gRpcSeatSocket);
		return DRV_OK;
	}	
	
	return DRV_OK;
}

/************************************************************************
话音数据接收线程
************************************************************************/
int RpcVoice_Socket_init(void)
{
	struct sockaddr_in my_addr = {0};
	
	/* don't set remote socket, dynaic config  */

	/* local socket setting  */
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(30008);
	my_addr.sin_addr.s_addr = htonl(inet_addr(get_config_var_str(config_cfg,"EXTEND_LOCAL_SEAT_IP")));
	
	if ((gRpcVoiceSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("cann't create Rpc-voice socket.\r\n");
		return DRV_ERR;
	}

	if (DRV_ERR == bind(gRpcVoiceSocket, (struct sockaddr *)&my_addr, sizeof(my_addr)))
	{
		printf("Bind Rpc-voice socket failed.\r\n");
		close(gRpcVoiceSocket);
		return DRV_ERR;
	}
		
	return DRV_OK;
}

void RpcVoice_RxThread(void)
{
	struct sockaddr_in remote_addr;
	uint8 abuf[MAX_SOCKET_LEN] = {0};
	int Rec_Len;
	unsigned int len;

	printf("%s start !\r\n", __func__);
	
	len = sizeof(remote_addr);
	while (1)
	{

		if((Rec_Len = recvfrom(gRpcVoiceSocket, abuf, MAX_SOCKET_LEN, 0,
		   (struct sockaddr *)&remote_addr, &len)) != DRV_ERR)
		{
		
			if((WORK_MODE_JILIAN == Param_update.workmode) && (terminal_group[3].bPortType == PORT_TYPE_LOCAL_XIWEI )\
				&& (terminal_group[3].bCommand == MC_TALK))

			{
				if(172 == Rec_Len)
				{
					ac491SendData(0, 3, abuf, Rec_Len, PACK_TYPE_RTP);	
				}
				
				continue;
			}		
		}
	}
}

int RpcVoice_Socket_Close(void)
{
	if (gRpcVoiceSocket)
	{
		close(gRpcVoiceSocket);
	}
	return DRV_OK;
}


int init_lan_bus(const void *data)
{

	if((WORK_MODE_PAOBING == Param_update.workmode)||(WORK_MODE_ZHUANGJIA == Param_update.workmode))
	{	
		//	int broadcastFlag = 1;
		
		/*mgw.conf必须存在，若该文件不存在，则程序不能正常启动*/
		gIpcSemSocket.local_ip = get_config_var_str(config_cfg,"DEFAULT_LOCAL_IP");
		gIpcSemSocket.local_port = get_config_var_val(config_cfg,"SCU_LOCAL_PORT");

		if(WORK_MODE_PAOBING == Param_update.workmode)
		{
			gIpcSemSocket.remote_ip = get_config_var_str(config_cfg,"DEFAULT_REMOTE_IP2");
		}
		else if(WORK_MODE_ZHUANGJIA == Param_update.workmode)
		{
			gIpcSemSocket.remote_ip = get_config_var_str(config_cfg,"DEFAULT_REMOTE_IP1");
		}
		
		gIpcSemSocket.remote_port = get_config_var_val(config_cfg,"DEFAULT_REMOTE_PORT");
		strncpy((char *)gIpcSemSocket.name,"scu", 4);
		if(socket_new_with_bindaddr(&gIpcSemSocket)<0)
		{
			return DRV_ERR;
		}
		
	#if 0
		scu_js_udp_socket.local_ip = get_config_var_str(config_cfg,"DEFAULT_LOCAL_IP");
		scu_js_udp_socket.local_port = 40001;
		scu_js_udp_socket.remote_ip = get_config_var_str(config_cfg,"DEFAULT_REMOTE_IP");
		scu_js_udp_socket.remote_port = 40001;
		strcpy(scu_js_udp_socket.name,"scu_js");
		if(socket_new_with_bindaddr(&scu_js_udp_socket)<0)
			return (-1);
	#endif

		gIpcDatSocket.local_ip = get_config_var_str(config_cfg,"DCU_LOCAL_DATA_IP");
		gIpcDatSocket.local_port = get_config_var_val(config_cfg,"DCU_LOCAL_DATA_PORT");
		if(WORK_MODE_PAOBING == Param_update.workmode)
		{
			gIpcDatSocket.remote_ip = get_config_var_str(config_cfg,"DCU_REMOTE_DATA_IP");
		}
		else if(WORK_MODE_ZHUANGJIA == Param_update.workmode)
		{
			gIpcDatSocket.remote_ip = get_config_var_str(config_cfg,"DCU_REMOTE_DATA_IP");
		}
		
		gIpcDatSocket.remote_port = get_config_var_val(config_cfg,"DCU_REMOTE_DATA_PORT");
		strncpy((char *)gIpcDatSocket.name,"dcu", 4);
		if(socket_new_with_bindaddr(&gIpcDatSocket)<0)
		{
			return DRV_ERR;
		}

		ext_udp_socket.local_ip = get_config_var_str(mgw_cfg,"EXTEND_LOCAL_IP");
		ext_udp_socket.local_port = get_config_var_val(config_cfg,"EXTEND_LOCAL_PORT");
		ext_udp_socket.remote_ip = get_config_var_str(config_cfg,"EXTEND_REMOTE_IP");
		ext_udp_socket.remote_port = get_config_var_val(config_cfg,"EXTEND_REMOTE_PORT");
		strncpy((char *)ext_udp_socket.name,"ext", 4);
		if(socket_new_with_bindaddr(&ext_udp_socket)<0)
		{
			return DRV_ERR;
		}

		ext_rtp_socket.local_ip = get_config_var_str(mgw_cfg,"EXTEND_LOCAL_IP");
		ext_rtp_socket.local_port = get_config_var_val(config_cfg,"RTP_START");
		ext_rtp_socket.remote_ip = get_config_var_str(config_cfg,"EXTEND_REMOTE_IP");
		ext_rtp_socket.remote_port = get_config_var_val(config_cfg,"RTP_START");
		strncpy((char *)ext_rtp_socket.name,"extrtp", 7);
		if(socket_new_with_bindaddr(&ext_rtp_socket)<0)
		{
			return DRV_ERR;
		}

		Sw_Phone_init();
		
		mgwrtpstart = get_config_var_val(config_cfg,"RTP_START");
		mgwrtpend = get_config_var_val(config_cfg,"RTP_END");		

	}	
	else if(WORK_MODE_JILIAN == Param_update.workmode)
	{
		/* I 型信令*/	
		Terminal_I_IP.sin_family = AF_INET;
		Terminal_I_IP.sin_port = htons(30000);
		Terminal_I_IP.sin_addr.s_addr = htonl(inet_addr(get_config_var_str(config_cfg,"EXTEND_REMOTE_SEAT_IP")));

		/* I 型话音*/
		Terminal_I_IPV.sin_family = AF_INET;
		Terminal_I_IPV.sin_port = htons(30008);
		Terminal_I_IPV.sin_addr.s_addr = htonl(inet_addr(get_config_var_str(config_cfg,"EXTEND_REMOTE_SEAT_IP")));
	
		RpcSeat_Socket_init();
		RpcVoice_Socket_init();

		RpcDbg_Socket_init();
		RpcDisplayMng_Socket_init();
		
		mgwrtpstart = get_config_var_val(config_cfg,"RTP_START");
		mgwrtpend = get_config_var_val(config_cfg,"RTP_END");		
	}
	else if(WORK_MODE_SEAT == Param_update.workmode)
	{
		Zjct_init();
	}
	else if(WORK_MODE_ADAPTER == Param_update.workmode)
	{
		gIpcDatSocket.local_ip = get_config_var_str(config_cfg,"DCU_LOCAL_DATA_IP");
		gIpcDatSocket.local_port = get_config_var_val(config_cfg,"DCU_LOCAL_DATA_PORT");

		gIpcDatSocket.remote_ip = get_config_var_str(config_cfg,"DCU_REMOTE_DATA_IP");
		gIpcDatSocket.remote_port = get_config_var_val(config_cfg,"DCU_REMOTE_DATA_PORT");
		strncpy((char *)gIpcDatSocket.name,"dcu", 4);
		if(socket_new_with_bindaddr(&gIpcDatSocket)<0)
		{
			return DRV_ERR;
		}
	}
	
	return 0;
}



int do_buf_dump (cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	if(argc != 2){
		printf("Invalid Parameter!\n");
		printf("usage: %s",cmdtp->usage);
		return 0;
	}

	if(strcasecmp(argv[1],"open") == 0)
		flag_dump = DUMP_OPEN;
	else if(strcasecmp(argv[1],"close") == 0)
		flag_dump = DUMP_CLOSE;

	return 0;
}

inline void dump_buff(const unsigned char* buf,int count)
{
	int i;
	
	if(flag_dump == DUMP_CLOSE)
		return;
	
	for(i=0;i<count;i++)
	{
		if((i%8) == 0)
			DEBUG_OUT("\n%.2x : ",i);
		DEBUG_OUT("%.2x ",buf[i]);
	}
	DEBUG_OUT("\n");
	return;
}

int	lan_bus_ctrl(int *id, int fd, short events, void *cbdata)
{
	int res;
	UDP_SOCKET *sck = (UDP_SOCKET*)cbdata;
	
	res = recvfrom(fd,sck->rawdata,sizeof(sck->rawdata), \
		0,(struct sockaddr*)&sck->addr_us,&sck->addr_len);
	
	if(res < 0)
	{
		ERR("Recv Error:%s\n",strerror(errno));
		return 0;
	}
	/*dump data in buff with hex format*/
	//VERBOSE_OUT(LOG_SYS,"recvfrom %s:%d\n",inet_ntoa(sck->addr_us.sin_addr),sck->addr_us.sin_port);
	dump_buff((unsigned char *)sck->rawdata,res);

	protocol_parse(sck->rawdata,res,(struct sockaddr_in*)&sck->addr_us);
	
	return 1;
}


int *rtp_io_add(int fd, void *data)
{
	return inc_io_add(rtp_ioc,fd,mgw_rtp_read,POLLIN,data);
}

int rtp_io_remove(int *_id)
{
	//return inc_io_remove(rtp_ioc,_id);
	return DRV_OK;
}


void	*do_rtp_monitor(void *arg)
{
#if 0
	if(!(rtp_ioc = io_context_create()))
	{
		VERBOSE_OUT(LOG_SYS,"Create IO Context Error!\n");
		return NULL;
	}
#endif

	VERBOSE_OUT(LOG_SYS,"do_rtp_monitor is running...\n");
	
	//ext_rtp_socket.ioc = rtp_ioc;
	//ext_rtp_socket.ioc_id = inc_io_add(rtp_ioc,ext_rtp_socket.udp_handle,mgw_rtp_read,POLLIN,&ext_rtp_socket);
	
	while(1)
	{
		//inc_io_wait(rtp_ioc,5);
		mgw_rtp_read(NULL,ext_rtp_socket.udp_handle,POLLIN,&ext_rtp_socket);
	}
}


int	mgw_rtp_read(int *id, int fd, short events, void *cbdata)
{
	int res;
	int port;
	struct sockaddr_in sin;
	int len = sizeof(struct sockaddr_in);
	//UDP_SOCKET * sck;

	res = recvfrom(fd,rawdata,sizeof(rawdata), \
		0,(struct sockaddr *)&sin,(socklen_t *)&len);
	
	if(res<0)
	{
		ERR("Rtp read recv error:%s\n",strerror(errno));
		return 0;
		
	}

	port = find_port_by_ip(&sin);
	if(port<0)
	{
		ERR("Rtp read can't find port\n");
		return 0;
	}

	/*
	* 发送RTP数据到相应的RTP通道，
	* RTP通道的通道号为rtp_ter.wPort;
	*/
	if(terminal_group[port].bCommand == MC_TALK)
	{
		//Recv_XIWEI_RTP_Packet(rtp_ter->wPort + MAXLOCALTER,rtp_ter->rtp_socket.rawdata,res);
		Recv_XIWEI_RTP_Packet(terminal_group[port].bslot_us,rawdata,res);
	}
	else if(terminal_group[port].bCommand == MC_LISTEN)
	{
		Recv_XIWEI_RTP_Packet(terminal_group[port].bslot_us,rawdata,res);
	}

	return 1;
}

int do_io_dump (cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	inc_io_dump(rtp_ioc);
	return 0;
}


unsigned short get_random_port(void)
{
	return (mgwrtpend == mgwrtpstart) ? mgwrtpstart : (random() % (mgwrtpend - mgwrtpstart)) + mgwrtpstart;
}

#if 0
void	*lan_bus_control(void *arg)
{
	int res;

	if(init_lan_bus(NULL) < 0)
		return NULL;
	while(1)
	{
		res = recvfrom(gIpcSemSocket.udp_handle, gIpcSemSocket.rawdata,sizeof(gIpcSemSocket.rawdata), \
			0,(struct sockaddr*)&gIpcSemSocket.addr_us,&gIpcSemSocket.addr_len);
		
		if(res<0 && errno != EAGAIN)
		{
			VERBOSE_OUT(LOG_SYS,"Recv Error:%s\n",strerror(errno));
			break;
		}
		
		DEBUG_OUT("recv %d bytes from[%s:%d]--->%s",res,inc_inet_ntoa(gIpcSemSocket.addr_us.sin_addr), \
			ntohs(gIpcSemSocket.addr_us.sin_port),gIpcSemSocket.rawdata);

		res = sendto(gIpcSemSocket.udp_handle,gIpcSemSocket.rawdata,strlen(gIpcSemSocket.rawdata), \
			0,(struct sockaddr*)&gIpcSemSocket.addr_them,gIpcSemSocket.addr_len);

		if(res<0)
		{
			VERBOSE_OUT(LOG_SYS,"Send to lan Error:%s\n",strerror(errno));
			break;
		}
#if 1
		/*send to vlan 0*/
		res = sendto(vgIpcSemSocket[0].udp_handle,gIpcSemSocket.rawdata,strlen(gIpcSemSocket.rawdata), \
			0,(struct sockaddr*)&vgIpcSemSocket[0].addr_them,vgIpcSemSocket[0].addr_len);

		if(res<0)
		{
			VERBOSE_OUT(LOG_SYS,"Send to vlan 0 Error:%s\n",strerror(errno));
			break;
		}
		
		/*send to vlan 1*/
		res = sendto(vgIpcSemSocket[1].udp_handle,gIpcSemSocket.rawdata,strlen(gIpcSemSocket.rawdata), \
			0,(struct sockaddr*)&vgIpcSemSocket[1].addr_them,vgIpcSemSocket[1].addr_len);

		if(res<0)
		{
			VERBOSE_OUT(LOG_SYS,"Send to vlan 1 Error:%s\n",strerror(errno));
			break;
		}
#endif		
	}
	close(gIpcSemSocket.udp_handle);
	return NULL;
}
#endif

