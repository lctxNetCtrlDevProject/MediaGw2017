/*
========================================================
** Description: vlan数据的控制，主要是与即设网的通信
** 
========================================================
*/
#include "PUBLIC.h"

//UDP_SOCKET js_udp_socket;	/*路由信息交互接口*/
//UDP_SOCKET js_data_udp_socket;/*既设网数据端口*/

int	init_vlan_bus(const void *data)
{
#if 0
	int broadcastFlag = 1;
	//js_udp_socket.local_ip = get_config_var_str(config_cfg,"JS_LOCAL_IP");
	js_udp_socket.local_ip = "0.0.0.0";
	js_udp_socket.local_port = get_config_var_val(config_cfg,"JS_LOCAL_PORT");
	js_udp_socket.remote_ip = get_config_var_str(config_cfg,"JS_REMOTE_IP");
	js_udp_socket.remote_port = get_config_var_val(config_cfg,"JS_REMOTE_PORT");
	strcpy(js_udp_socket.name,"js");
	if(socket_new_with_bindaddr(&js_udp_socket) < 0)
		return (-1);

	setsockopt(js_udp_socket.udp_handle,SOL_SOCKET,SO_BROADCAST,
		&broadcastFlag,
		sizeof(broadcastFlag));


	js_data_udp_socket.local_ip = "0.0.0.0";
	js_data_udp_socket.local_port = get_config_var_val(config_cfg,"JS_LOCAL_PORT")+1;
	js_data_udp_socket.remote_ip = get_config_var_str(config_cfg,"JS_REMOTE_IP");
	js_data_udp_socket.remote_port = get_config_var_val(config_cfg,"JS_REMOTE_PORT")+1;
	strcpy(js_data_udp_socket.name,"js_data");
	if(socket_new_with_bindaddr(&js_data_udp_socket)<0)
		return (-1);
#endif

	return 0;
}

int do_broadcast_test(cmd_tbl_t * cmdtp,int argc,char * argv [ ])
{
	struct sockaddr_in sin;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr("192.168.2.255");
	sin.sin_port = 20034;
	
	//sendto(js_udp_socket.udp_handle,"987654321",9, 0,(struct sockaddr*)&sin,js_udp_socket.addr_len);

	return 0;
}


static int js_route_parse(const char * pack,size_t len,struct sockaddr_in src_addr)
{
	ST_HF_MGW_JS_PRO_HEADER * header = (ST_HF_MGW_JS_PRO_HEADER *)pack;
	if(header->protocol == 0x01 && header->len+sizeof(ST_HF_MGW_JS_PRO_HEADER) == len)
	{
		switch(header->id)
		{
			case MGW_JS_HELLO_ID:
			{
				/*路由探测协议*/
				ST_HF_MGW_JS_PRO * pro = (ST_HF_MGW_JS_PRO *)pack;
				ST_HF_MGW_JS_HELLO* hello = (ST_HF_MGW_JS_HELLO*)pro->body;
				char str_dep[16];
				struct MGW_INTERFACE src_if;

				inc_log(LOG_DEBUG,"Recv Hello Message!\n");
				bcd2phone((char *)str_dep, (char *)hello->dep_num,4);
				inc_log(LOG_DEBUG,"dep:%s\n",str_dep);

				src_if.ip_addr = mgw_if_fetch_ip("eth1");
				src_if.ip_gw = src_if.ip_addr;
				src_if.ip_mask = mgw_fetch_mask(mgw_if_fetch_mask("eth1"));

				//if(mgw_add_js_node(hello->hf_addr,src_if,hello->ip_addr,32,hello->cost,str_dep) != (-1))
				{
					//mgw_insert_js_rt(hello->hf_addr,hello->hf_addr,GET_HF_MASK(hello->hf_addr),hello->cost,ROUTE_TYPE_JS,str_dep);
				}
			}
			break;

			case MGW_JS_HF_ROUTE_ID:
				{
					ST_HF_MGW_JS_PRO * pro = (ST_HF_MGW_JS_PRO *)pack;
					ST_HF_MGW_JS_HF_ROUTE_MSG * msg = (ST_HF_MGW_JS_HF_ROUTE_MSG *)pro->body;
					int i;
					char str_dep[16];
					
					ST_HF_MGW_DCU_PRO route_pro;
					struct sockaddr_in addr_dst;

					inc_log(LOG_DEBUG,"Recv JS HF Route Msg!\n");
					
					/*上报数据交换板专网路由*/
					route_pro.header.baowen_type = BAOWEN_TYPE_MGW_DCU;
					route_pro.header.dst_addr = 0x10;
					route_pro.header.src_addr = 0xb0;
					route_pro.header.info_type = INT_FRAME_TYPE_CMD_ROUTE;
					

					for(i=0;i<msg->route_count;i++)
					{
						bcd2phone((char *)str_dep, (char *)msg->route[i].dep_num,4);
						//mgw_insert_js_rt(msg->route[i].dst_hf_addr,msg->hf_addr, \
							//GET_HF_MASK(msg->route[i].dst_hf_addr),msg->route[i].cost,ROUTE_TYPE_JS,str_dep);
					}
					

					{
						/*发送到既设网路由应答信息*/
						ST_HF_MGW_JS_PRO js_pro;
						ST_HF_MGW_JS_HF_ROUTE_ANS * js_route_ans;

						js_pro.header.protocol = 0x01;
						js_pro.header.id = MGW_JS_HF_ROUTE_ANS_ID;
						js_pro.header.len = sizeof(ST_HF_MGW_JS_HF_ROUTE_ANS);

						js_route_ans = (ST_HF_MGW_JS_HF_ROUTE_ANS *)js_pro.body;
						js_route_ans->hf_addr = g_hf_addr;

						addr_dst.sin_family = AF_INET;
						addr_dst.sin_port = htons(20034);
						addr_dst.sin_addr = src_addr.sin_addr;

						//sendto(js_udp_socket.udp_handle,&js_pro,sizeof(ST_HF_MGW_JS_PRO_HEADER)+sizeof(ST_HF_MGW_JS_HF_ROUTE_ANS), 0,(struct sockaddr*)&addr_dst,js_udp_socket.addr_len);
					}
				}
				break;
		}
	}
	else if(header->protocol == MGW_JS_HF_TEST_REQ_ID){
		ST_HF_MGW_JS_TEST_REQ * test_req = (ST_HF_MGW_JS_TEST_REQ *)pack;
		ST_HF_MGW_JS_TEST_ANS test_ans;
		struct sockaddr_in addr_dst;
		
		if(len != sizeof(ST_HF_MGW_JS_TEST_REQ))
		{
			VERBOSE_OUT(LOG_SYS,"Recv Test Request with wrong len\n");
			return 0;
		}
		
		if(test_req->checksum != cal_checksum(test_req->data,32))
		{
			VERBOSE_OUT(LOG_SYS,"Recv Test Request with wrong checksum\n");
			return 0;
		}

		VERBOSE_OUT(LOG_SYS,"Recv Test Request and check correct\n");

		test_ans.id = MGW_JS_HF_TEST_ANS_ID;
		memcpy(test_ans.data,test_req->data,32);
		test_ans.checksum = test_req->checksum;
		test_ans.seqno  = test_req->seqno;

		addr_dst.sin_family = AF_INET;
		addr_dst.sin_addr = src_addr.sin_addr;
		addr_dst.sin_port = htons(20034);

		//sendto(js_udp_socket.udp_handle,&test_ans,sizeof(ST_HF_MGW_JS_TEST_ANS), 0,(struct sockaddr*)&addr_dst,js_udp_socket.addr_len);
		
	}else
		inc_log(LOG_DEBUG,"Recv Bad JS Packet!\n");
	return 0;
}

static int js_data_parse(const char * pack,size_t len)
{
	int i;
	ST_HF_CCC_CCU_PRO_HEADER * header = (ST_HF_CCC_CCU_PRO_HEADER *)pack;
	
	if(header->protocol_type == 0x90){
		struct sockaddr_in sin;
		int res;
		ST_HF_MGW_DCU_PRO_HEADER * dcu_pro_header = (ST_HF_MGW_DCU_PRO_HEADER *)pack;
		sin.sin_family = AF_INET;
		inc_log(LOG_DEBUG,"recv js xinling msg!\n");
		if(dcu_pro_header->dst_addr == 0x10 && \
			dcu_pro_header->src_addr == 0xb0 && \
			dcu_pro_header->info_type == INT_FRAME_TYPE_CMD_XINLING){/*类型校验*/
			if(dcu_pro_header->data_len == len-sizeof(ST_HF_MGW_DCU_PRO_HEADER)){/*长度校验*/
				inc_log(LOG_DEBUG,"recv js xinling msg and parse!\n");
				sin.sin_addr.s_addr = inet_addr(HF_MGW_SCU_SCUDATA_IP);
				sin.sin_port = HF_MGW_SCU_SCUDATA_PORT; 

				res = Board_Data_SendTo_Inc_Addr((uint8 *)&pack, len, sin);
				if(res<0)
					VERBOSE_OUT(LOG_SYS,"Send Error:%s\n",strerror(errno));
				return 0;
			}
		}
	}
	
	for(i=0;i<header->dst_count;i++)
	{
		inc_log(LOG_DEBUG,"Send Data to hf %.8x\n", (unsigned int)header->dst_addr[i]);
		//transmit_packet(header->dst_addr[i],pack,len);
	}
	return 0;
}

int	js_route_proc(int *id, int fd, short events, void *cbdata)
{
	int res;
	UDP_SOCKET *sck = (UDP_SOCKET*)cbdata;
	
	res = recvfrom(fd,sck->rawdata,sizeof(sck->rawdata), \
		0,(struct sockaddr*)&sck->addr_us,&sck->addr_len);
	
	if(res<0 && errno != EAGAIN)
	{
		VERBOSE_OUT(LOG_SYS,"Recv Error:%s\n",strerror(errno));
		return 0;
	}

	js_route_parse(sck->rawdata,res,sck->addr_us);
	
	return 1;
}

int	js_data_proc(int *id, int fd, short events, void *cbdata)
{
	int res;
	UDP_SOCKET *sck = (UDP_SOCKET*)cbdata;
	
	res = recvfrom(fd,sck->rawdata,sizeof(sck->rawdata), \
		0,(struct sockaddr*)&sck->addr_us,&sck->addr_len);
	
	if(res < 0)
	{
		VERBOSE_OUT(LOG_SYS,"Recv Error:%s\n",strerror(errno));
		return 0;
	}

	js_data_parse(sck->rawdata,res);
	
	return 1;
}



