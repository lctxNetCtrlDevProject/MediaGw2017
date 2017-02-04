/*
** route control block
*/

#include "PUBLIC.h"

struct JS_LIST js_user_list;
struct JS_ROUTE_TABLE js_route_user_table = INC_LIST_HEAD_INIT_VALUE;
struct JS_NAT_TABLE js_nat_table = INC_LIST_HEAD_INIT_VALUE;

#if 0
static struct io_context	*nat_ioc = NULL;
static unsigned short nat_port_start,nat_port_end;
#endif

void send_route_msg(struct JS_NODE * node)
{
	struct sockaddr_in sin;

	ST_HF_MGW_JS_PRO pro;
	ST_HF_MGW_JS_HELLO * hello;

#if 1
	if(node->state == JS_NODE_STATE_ALIVE){
		node->state = JS_NODE_STATE_INACTIVE;
		node->inactive_time++;
	}else if(node->state == JS_NODE_STATE_INACTIVE){
		node->inactive_time++;
		if(node->inactive_time > 3)
		{
			node->state = JS_NODE_STATE_IDLE;
			node->inactive_time = 0;
		}
	}else if(node->state == JS_NODE_STATE_ACTIVE){
		node->inactive_time = 0;
		node->state = JS_NODE_STATE_INACTIVE;
	}else if(node->state == JS_NODE_STATE_IDLE){
		node->inactive_time = 0;
	}
#endif

	hello = (ST_HF_MGW_JS_HELLO *)pro.body;
	pro.header.protocol = 0x01;
	pro.header.id = MGW_JS_HELLO_ID;
	pro.header.len = sizeof(ST_HF_MGW_JS_HELLO);

	hello->hf_addr = g_hf_addr;
	hello->ip_addr = mgw_if_fetch_ip("eth1");
	hello->mask = 24;
	hello->cost = node->cost;
	phone2bcd((char *)hello->dep_num, (char *)get_config_var_str(config_cfg,"DEP"),2);

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = node->js_ip_addr;
	sin.sin_port = htons(20034);

	if(node->hf_addr == g_hf_addr)
		return;

	inc_log(LOG_DEBUG,"Send route msg to %s\n",inc_inet_ntoa(sin.sin_addr));
	
	//sendto(js_udp_socket.udp_handle,&pro,sizeof(ST_HF_MGW_JS_PRO_HEADER)+sizeof(ST_HF_MGW_JS_HELLO), 0,(struct sockaddr*)&sin,js_udp_socket.addr_len);

	return;
}


int mgw_js_route_msg(void)
{
	mgw_dcu_route_msg();

	INCOBJ_CONTAINER_TRAVERSE(&js_user_list,1,do{ \
		INCOBJ_RDLOCK(iterator); \
		send_route_msg(iterator); \
		INCOBJ_UNLOCK(iterator); \
		}while(0));

	return 0;
}

int send_dcu_route_msg(struct JS_ROUTE_NODE * node,unsigned char rt_index)
{
	ST_HF_MGW_DCU_PRO route_pro;
	ST_HF_MGW_DCU_ROUTE_MSG * msg;
	struct JS_NODE * neighbour_node;
	int i = 1;
	struct sockaddr_in addr_dst;

	if(node->type == ROUTE_TYPE_HF)
		return (-1);
	
	route_pro.header.baowen_type = BAOWEN_TYPE_MGW_DCU;
	route_pro.header.dst_addr = 0x10;
	route_pro.header.src_addr = 0xb0;
	route_pro.header.info_type = INT_FRAME_TYPE_CMD_ROUTE;
	
	
	msg = (ST_HF_MGW_DCU_ROUTE_MSG *)route_pro.body;
	msg->id = MGW_DCU_ROUTE_ID;
	msg->route_index = rt_index;
	
	{
		msg->route[0].hf_addr = node->dst_hf_addr;
		msg->route[0].next_hop = node->next_hop;
		msg->route[0].module_addr = 0xb0;
		msg->route[0].cost = node->cost;
	}
	msg->len = 11*i+4;
	msg->route_count = i;

#if 1
	neighbour_node = INCOBJ_CONTAINER_FIND_FULL(&js_user_list,node->next_hop,hf_addr,NULL,NULL,is_equal);
	if(neighbour_node)
	{		
		if(neighbour_node->state == JS_NODE_STATE_IDLE)
		{
			neighbour_node->inactive_time = 0;
			msg->route_count = 0;
			neighbour_node->state = JS_NODE_STATE_INIT;
			VERBOSE_OUT(LOG_SYS,"Node in js_user_list: %x goto JS_NODE_STATE_INIT\n",neighbour_node->hf_addr);
			
			route_pro.header.data_len = msg->len+3;
			
			inc_log(LOG_DEBUG,"Send to DCU Del %d JS Route msg!\n",i);
			
			addr_dst.sin_family = AF_INET;
			addr_dst.sin_port = htons(HF_MGW_SCU_SCUDATA_PORT);
			addr_dst.sin_addr.s_addr = inet_addr(HF_MGW_SCU_SCUDATA_IP);
			
			Board_Data_SendTo_Inc_Addr((uint8 *)&route_pro, sizeof(ST_HF_MGW_DCU_PRO_HEADER)+msg->len+3, addr_dst);
			return 1;
		}else if(neighbour_node->state == JS_NODE_STATE_INIT)
			return 1;
	}
#endif

	route_pro.header.data_len = msg->len+3;
	
	inc_log(LOG_DEBUG,"Send to DCU %d JS Route msg!\n",i);
	
	addr_dst.sin_family = AF_INET;
	addr_dst.sin_port = htons(HF_MGW_SCU_SCUDATA_PORT);
	addr_dst.sin_addr.s_addr = inet_addr(HF_MGW_SCU_SCUDATA_IP);
	
	Board_Data_SendTo_Inc_Addr((uint8 *)&route_pro, sizeof(ST_HF_MGW_DCU_PRO_HEADER)+msg->len+3, addr_dst);	

	return 0;
}

void mgw_dcu_route_msg(void)
{
	int rt_index = 1;
	struct JS_ROUTE_TABLE_T * tmp_js_route_table_t;
	
	INC_LIST_TRAVERSE_SAFE_BEGIN(&js_route_user_table,tmp_js_route_table_t,list)
	{
		INCOBJ_CONTAINER_TRAVERSE(tmp_js_route_table_t,1,do{ \
			INCOBJ_RDLOCK(iterator); \
			if(!send_dcu_route_msg(iterator,rt_index)) \
				rt_index++; \
			INCOBJ_UNLOCK(iterator); \
			}while(0));
	}
	INC_LIST_TRAVERSE_SAFE_END
}

int process_tunnel_cmd_str(const char* cmd_str)
{
	/*!brief
	** @the format of cmd_str like that:
	* ip route add xxx.xxx.xxx.xxx/xx via xxx.xxx.xxx.xxx dev eth1
	*/
	char * clone_cmd_str = inc_strdup(cmd_str);
	char * result;
	char * mask_str;	
	char gw_str[32]="";
	char ip_str[16];
	char * if_name;
	unsigned long dst_ip;
	unsigned char dst_mask;
	struct MGW_INTERFACE src_if;
	result = strpbrk(clone_cmd_str,"0123456789");
	if(!result){
		VERBOSE_OUT(LOG_SYS,"ERROR IN PARSE TUNNEL %s\n",clone_cmd_str);
		free(clone_cmd_str);
		return (-1);
	}

	strncpy((char *)ip_str,result,15);
	ip_str[strcspn(result,"/")] = '\0';
	dst_ip = inet_addr(ip_str);

	mask_str = strpbrk(clone_cmd_str,"/");
	if(!mask_str){
		free(clone_cmd_str);
		return (-1);
	}
	mask_str++;
	dst_mask = strtol(mask_str,NULL,10);

	sscanf(clone_cmd_str,"ip route add %*s via %s dev eth1",gw_str);
	src_if.ip_gw = inet_addr(gw_str);
	if_name=mgw_find_if(src_if.ip_gw);

	if(NULL != if_name)
	{
		src_if.ip_addr = mgw_if_fetch_ip(if_name);
		src_if.ip_mask= mgw_fetch_mask(mgw_if_fetch_mask(if_name));
	}else{
		VERBOSE_OUT(LOG_SYS,"can't find interface\n");
		src_if.ip_addr = inet_addr("0.0.0.0");
		src_if.ip_mask = inet_addr("255.255.255.0");
	}

	mgw_add_js_node(0,src_if,dst_ip,dst_mask,EN_TYPE_JS_COST_FIBER,NULL);

	free(clone_cmd_str);
	return 0;
}

int process_dnat_cmd_str(const char* cmd_str)
{
	/*!brief
	** @the format of cmd_str like that:
	* iptables -t nat -A PREROUTING -i eth1 --dport xxxxx -j DNAT --to xxx.xxx.xxx.xxx
	*/
	char * clone_cmd_str = inc_strdup(cmd_str);
	char * result;
	unsigned short dnat_port;
	unsigned long dnat_ip;
	result = strpbrk(clone_cmd_str,"0123456789");
	result += 1;
	result = strpbrk(result,"0123456789");
	if(!result){
		VERBOSE_OUT(LOG_SYS,"ERROR IN PARSE DNAT %s\n",clone_cmd_str);
		free(clone_cmd_str);
		return (-1);
	}

	dnat_port = atoi(result);
	result += strcspn(result," ");
	result = strpbrk(result,"0123456789");
	dnat_ip = inet_addr(result);
	mgw_add_nat_node(dnat_port,dnat_ip);

	free(clone_cmd_str);
	return 0;
}

int	init_mgw_rcb(const void * data)
{
	INCOBJ_CONTAINER_INIT(&js_user_list);

	/*Read Tunnel Configuration and DNAT Configuration from system config file*/	
	{
		struct category* iterator_cat = find_config_category(config_cfg,"TUNNEL");
		struct variable* iterator_var;
		for(iterator_var=iterator_cat->variable_list->root;iterator_var;iterator_var=iterator_var->next){
			if(strncasecmp(iterator_var->name,"tunnel",strlen("tunnel")) == 0)
				process_tunnel_cmd_str(iterator_var->value);
		}
			

		iterator_cat = find_config_category(config_cfg,"DNAT");
		for(iterator_var=iterator_cat->variable_list->root;iterator_var;iterator_var=iterator_var->next){
			if(strncasecmp(iterator_var->name,"dnat",strlen("dnat")) == 0)
				process_dnat_cmd_str(iterator_var->value);
		}
	}	
	return 0;
}


int mgw_add_js_node(unsigned long hf_addr,struct MGW_INTERFACE mgw_if,unsigned long ip_addr,unsigned char ip_mask,unsigned short cost,const char* dep)
{
	unsigned long		tmp_hf_addr;
	unsigned long		tmp_ip_addr;
	unsigned short	 	tmp_cost;
	unsigned short 	obj_cnt = 0;
	struct JS_NODE * tmp_js_node;
	unsigned long 	mask;

	tmp_hf_addr = hf_addr;
	tmp_ip_addr = ip_addr;
	tmp_cost = cost;

	INCOBJ_CONTAINER_TRAVERSE(&js_user_list, 1, obj_cnt++);
	if(obj_cnt>=MAX_TUNNEL_COUNT){
		inc_log(LOG_DEBUG,"add js node error cause js_user_list is full!\n");
		return (-1);
	}

	if(!mgw_equal_if(mgw_if.ip_addr))
		return (-1);

	if(!mgw_if.ip_mask || mgw_if.ip_mask>32)
		return (-1);
	
	mask = htonl(~((1<<(32-mgw_if.ip_mask))-1));
	if((mgw_if.ip_addr&mask) != (mgw_if.ip_gw&mask))
		return (-1);

	if(!tmp_ip_addr || tmp_ip_addr == 0xffffffff)
		return (-1);

	if(!ip_mask || ip_mask>32)
		return (-1);

	mask = htonl(~((1<<(32-ip_mask))-1));
	if((ip_addr & mask) == 0)
		return (-1);
	
	if(!(tmp_js_node=INCOBJ_CONTAINER_FIND_FULL(&js_user_list,tmp_ip_addr,js_ip_addr,NULL,NULL,is_equal)))
	{

		tmp_js_node = inc_calloc(1,sizeof(struct JS_NODE));
		INCOBJ_INIT(tmp_js_node);

		INCOBJ_WRLOCK(tmp_js_node);
		tmp_js_node->hf_addr = tmp_hf_addr;
		tmp_js_node->js_ip_addr = tmp_ip_addr;
		tmp_js_node->cost = tmp_cost;
		tmp_js_node->mgw_if = mgw_if;
		tmp_js_node->js_ip_mask = ip_mask;
		
		//if(tmp_js_node->dep_num && dep)
		//	bcd2phone(tmp_js_node->dep_num,dep,4);

		if(tmp_js_node->dep_num && dep)
		{
			strcpy((char *)tmp_js_node->dep_num,dep);
		}
		tmp_js_node->state = JS_NODE_STATE_ALIVE;
		tmp_js_node->inactive_time = 0;
		INCOBJ_UNLOCK(tmp_js_node);
		
		INCOBJ_CONTAINER_LINK(&js_user_list,tmp_js_node);
		#if 0
		if(!tmp_js_node->hf_addr || !tmp_js_node->dep_num || !dep)
			return (-1);
		#endif

	}else{
		inc_log(LOG_DEBUG,"ip_addr: %.8x already in list - modify\n", (unsigned int)tmp_ip_addr);

		
		if(tmp_js_node->cost != cost)
			tmp_js_node->cost = cost;
		
		if(!tmp_js_node->dep_num && dep)
		{
			strncpy((char *)tmp_js_node->dep_num,dep, 16);
        }
        
		if(tmp_js_node->js_ip_mask != ip_mask)
			tmp_js_node->js_ip_mask = ip_mask;

		tmp_js_node->mgw_if = mgw_if;

		if(tmp_js_node->dep_num && dep && strcmp((char *)tmp_js_node->dep_num,dep))
		{	
		    strncpy((char *)tmp_js_node->dep_num,dep, 16);
        }
        
		if(tmp_js_node->state == JS_NODE_STATE_INACTIVE)
			tmp_js_node->state = JS_NODE_STATE_ACTIVE;

		if(tmp_js_node->state == JS_NODE_STATE_INIT)
			tmp_js_node->state = JS_NODE_STATE_ALIVE;

		if(tmp_js_node->hf_addr != hf_addr){
			tmp_js_node->hf_addr = hf_addr;
			return 0;
		}

		return (-1);
	}

	return 0;
}

struct JS_NODE * mgw_find_js_node(unsigned long ip_addr)
{
	unsigned long tmp_ip_addr = ip_addr;
	struct JS_NODE * tmp_js_node = NULL;
	if(!ip_addr)
		return NULL;
	tmp_js_node = INCOBJ_CONTAINER_FIND_FULL(&js_user_list,tmp_ip_addr,js_ip_addr,NULL,NULL,is_equal);
	return tmp_js_node;
}

int mgw_sub_js_node(unsigned long ip_addr)
{
	unsigned long tmp_ip_addr = ip_addr;
	struct JS_NODE * tmp_js_node;
	struct JS_ROUTE_TABLE_T * tmp_js_route_table_t;
	struct JS_ROUTE_NODE * tmp_js_route_node = NULL;
	if(!ip_addr)
		return (-1);

	tmp_js_node = INCOBJ_CONTAINER_FIND_UNLINK_FULL(&js_user_list,tmp_ip_addr,js_ip_addr,NULL,NULL,is_equal);

	if(!tmp_js_node)
		return (-1);
	else{
		INC_LIST_TRAVERSE_SAFE_BEGIN(&js_route_user_table,tmp_js_route_table_t,list)
		{
			//tmp_js_route_node = INCOBJ_CONTAINER_FIND_FULL(tmp_js_route_table_t,tmp_js_node->hf_addr,next_hop,NULL,NULL,is_equal);
			INCOBJ_CONTAINER_TRAVERSE(tmp_js_route_table_t, !tmp_js_route_node, do { \
				inc_log(LOG_DEBUG,"swf: Lock %p\n",iterator); \
				if (!(is_equal(iterator->next_hop, tmp_js_node->hf_addr))) { \
					tmp_js_route_node = (iterator); \
				} \
				inc_log(LOG_DEBUG,"swf: UnLock %p\n",iterator); \
			} while (0));
			
			if(tmp_js_route_node){
				inc_log(LOG_DEBUG,"swf:Lock %p!\n",tmp_js_route_table_t);
				INCOBJ_CONTAINER_UNLINK(tmp_js_route_table_t,tmp_js_route_node);
				INC_LIST_REMOVE_CURRENT(&js_route_user_table,list);
				inc_log(LOG_DEBUG,"swf:UnLock %p!\n",tmp_js_route_table_t);
				//INCOBJ_UNREF(tmp_js_route_node,free);
				//free(tmp_js_route_node);
			}
			tmp_js_route_node = NULL;
		}
		INC_LIST_TRAVERSE_SAFE_END

		INCOBJ_UNREF(tmp_js_node,free);
	}
	
	return 0;
}

int do_add_js_node (cmd_tbl_t * cmdtp, int argc, char *argv[])
{
	/*!brief
	add js node
	command usage: addnode xxxxxxxx xxx.xxx.xxx.xxx
	*/
	unsigned long	tmp_hf_addr;
	unsigned long	tmp_ip_addr;
	struct MGW_INTERFACE src_if;
	
	if(argc != 3){
		printf("Invalid Parameter!\n");
		printf("usage: %s",cmdtp->usage);
		return 0;
	}

	tmp_hf_addr = strtoul(argv[1],NULL,16);
	tmp_ip_addr = inet_addr(argv[2]);

	src_if.ip_addr = mgw_if_fetch_ip("eth1");
	src_if.ip_gw = src_if.ip_addr;
	src_if.ip_mask = mgw_fetch_mask(mgw_if_fetch_mask("eth1"));
	
	if(mgw_add_js_node(tmp_hf_addr,src_if,tmp_ip_addr,32,EN_TYPE_JS_COST_JUNZONG,"1000") != (-1))
	{
		mgw_insert_js_rt(tmp_hf_addr,tmp_hf_addr,GET_HF_MASK(tmp_hf_addr),EN_TYPE_JS_COST_JUNZONG,ROUTE_TYPE_JS,NULL);
	}

	return 0;
}

int do_sub_js_node (cmd_tbl_t * cmdtp,  int argc, char *argv[])
{
	/*!brief
	add js node
	command usage: subnode xxx.xxx.xxx.xxx
	*/
	unsigned long	tmp_ip_addr;
	
	if(argc != 2){
		printf("Invalid Parameter!\n");
		printf("usage: %s",cmdtp->usage);
		return 0;
	}

	tmp_ip_addr = inet_addr(argv[1]);
	
	if(mgw_sub_js_node(tmp_ip_addr))
		printf("Sub js node %s error!\n",argv[1]);
	else
		printf("Sub js node %s success!\n",argv[1]);

	return 0;
}



int mgw_insert_js_rt(unsigned long dst_addr,unsigned long next_hop_addr,unsigned long mask,unsigned short cost,unsigned char type,const char * dep_num)
{
	unsigned long		tmp_dst_addr;
	unsigned long		tmp_next_hop_addr;
	unsigned long		tmp_mask;
	struct JS_ROUTE_NODE * tmp_js_route_node;
	struct JS_ROUTE_TABLE_T * tmp_js_route_table_t;

	tmp_dst_addr = dst_addr;
	tmp_next_hop_addr = next_hop_addr;
	tmp_mask = mask;

	tmp_js_route_node = inc_calloc(1,sizeof(struct JS_ROUTE_NODE));
	INCOBJ_INIT(tmp_js_route_node);

	INCOBJ_WRLOCK(tmp_js_route_node);
	tmp_js_route_node->dst_hf_addr = tmp_dst_addr;
	tmp_js_route_node->next_hop = tmp_next_hop_addr;
	tmp_js_route_node->cost = cost;
	tmp_js_route_node->inactive_time = 0;
	tmp_js_route_node->state = JS_ROUTE_NODE_STATE_ACTIVE;
	tmp_js_route_node->type = type;
	//sprintf(tmp_js_route_node->dep_num,"%.4x",(tmp_dst_addr&0xffff));
	if(dep_num)
	{
		//strncpy((char *)tmp_js_route_node->dep_num,dep_num, );
		strncpy((char *)tmp_js_route_node->dep_num,dep_num,16);
    }
	else
	{	
	    //sprintf((char *)tmp_js_route_node->dep_num,"%.4x", (tmp_dst_addr&0xffff));
		snprintf((char *)tmp_js_route_node->dep_num, 16,"%x", (unsigned int)tmp_dst_addr);
    }
    
	INCOBJ_UNLOCK(tmp_js_route_node);

	if(INC_LIST_EMPTY(&js_route_user_table))
	{
		tmp_js_route_table_t = inc_calloc(1,sizeof(struct JS_ROUTE_TABLE_T));
		INCOBJ_CONTAINER_INIT(tmp_js_route_table_t);		
		tmp_js_route_table_t->mask = tmp_mask;

		tmp_js_route_node->state = JS_ROUTE_NODE_STATE_ACTIVE;
		INCOBJ_CONTAINER_LINK(tmp_js_route_table_t,tmp_js_route_node);
		
		INC_LIST_LOCK(&js_route_user_table);
		INC_LIST_INSERT_HEAD(&js_route_user_table,tmp_js_route_table_t,list);		
		INC_LIST_UNLOCK(&js_route_user_table);
		
	}else{
		INC_LIST_TRAVERSE_SAFE_BEGIN(&js_route_user_table,tmp_js_route_table_t,list)
		{
			if(tmp_mask == tmp_js_route_table_t->mask){
				INCOBJ_UNREF(tmp_js_route_node,free);
				INCOBJ_CONTAINER_TRAVERSE(tmp_js_route_table_t,1,do{ \
					iterator->state = JS_ROUTE_NODE_STATE_ACTIVE; \
					iterator->inactive_time = 0; \
					iterator->next_hop = tmp_next_hop_addr; \
					iterator->cost = cost; \
					iterator->type = type;}while(0));
				return (-1);
			}
		}
		INC_LIST_TRAVERSE_SAFE_END		
		
		tmp_js_route_table_t = inc_calloc(1,sizeof(struct JS_ROUTE_TABLE_T));
		INCOBJ_CONTAINER_INIT(tmp_js_route_table_t);
		
		tmp_js_route_table_t->mask = tmp_mask;
		INCOBJ_CONTAINER_LINK(tmp_js_route_table_t,tmp_js_route_node);
		
		
		INC_LIST_LOCK(&js_route_user_table);
		INC_LIST_INSERT_HEAD(&js_route_user_table,tmp_js_route_table_t,list);
		INC_LIST_UNLOCK(&js_route_user_table);
	}

	return 0;
}

int do_insert_js_rt (cmd_tbl_t * cmdtp,int argc, char *argv[])
{
	/* !brief
	add node to js route table
	command usage: addroute xxxxxxxx xxxxxxxx xxxx
	*/
	unsigned long		tmp_dst_addr;
	unsigned long		tmp_next_hop_addr;
	unsigned long		tmp_cost;
	
	if(argc != 4){
		printf("Invalid Parameter!\n");
		printf("usage: %s",cmdtp->usage);
		return 0;
	}

	tmp_dst_addr = strtoul(argv[1],NULL,16);
	tmp_next_hop_addr = strtoul(argv[2],NULL,16);
	tmp_cost = strtoul(argv[3],NULL,16);

	mgw_insert_js_rt(tmp_dst_addr,tmp_next_hop_addr,GET_HF_MASK(tmp_dst_addr),tmp_cost,ROUTE_TYPE_JS,NULL);

	return 0;
}


int transmit_packet(unsigned long dst_addr,const char* buf,size_t len)
{
	struct JS_ROUTE_TABLE_T * tmp_table_t;
	struct JS_ROUTE_NODE * next_hf_hop = NULL;
	struct JS_NODE * next_hf_node;
	struct sockaddr_in sin;
	int res = 0 ;
	ST_HF_MGW_DCU_PRO pro;

	sin.sin_family = AF_INET;

	if((dst_addr&0xfffffff0) == g_hf_addr)
	{
		/*构造板间数据发送到DCU*/
		pro.header.baowen_type = BAOWEN_TYPE_MGW_DCU;
		pro.header.dst_addr = 0x10;
		pro.header.src_addr = 0xb0;
		pro.header.info_type = INT_FRAME_TYPE_DATA;
		pro.header.data_len = len;

		memcpy(pro.body,buf,len);

		inc_log(LOG_DEBUG,"Transfer Packet to Local HF Node!\n");
		
		sin.sin_addr.s_addr = inet_addr(HF_MGW_SCU_SCUDATA_IP);
		sin.sin_port = HF_MGW_SCU_SCUDATA_PORT; 
		
		Board_Data_SendTo_Inc_Addr((uint8 *)&pro, len+sizeof(ST_HF_MGW_DCU_PRO_HEADER), sin);	
		
		return 0;
	}

	INC_LIST_TRAVERSE(&js_route_user_table,tmp_table_t,list)
	{	
		//next_hf_hop = INCOBJ_CONTAINER_FIND_FULL(tmp_table_t,dst_addr, dst_hf_addr&tmp_table_t->mask,NULL,NULL,is_equal);

		next_hf_hop = INCOBJ_CONTAINER_FIND_FULL(tmp_table_t,(dst_addr&0xfffffff0), \
			dst_hf_addr,NULL,NULL,is_equal);

		if(next_hf_hop)
			break;
	}
	
	if(!next_hf_hop)
	{
		inc_log(LOG_DEBUG,"Can't Find next_hf_hop\n");
		return 0;
	}

	next_hf_node = INCOBJ_CONTAINER_FIND_FULL(&js_user_list,next_hf_hop->next_hop, \
		hf_addr,NULL,NULL,is_equal);

	if(!next_hf_node)
	{
		inc_log(LOG_DEBUG,"Find next_hop %.8x but not find node\n", (unsigned int)next_hf_hop->next_hop);
		pro.header.baowen_type = BAOWEN_TYPE_MGW_DCU;
		pro.header.dst_addr = 0x10;
		pro.header.src_addr = 0xb0;
		pro.header.info_type = INT_FRAME_TYPE_DATA;
		pro.header.data_len = len;

		memcpy(pro.body,buf,len);
		
		sin.sin_addr.s_addr = inet_addr(HF_MGW_SCU_SCUDATA_IP);
		sin.sin_port = HF_MGW_SCU_SCUDATA_PORT;

		inc_log(LOG_DEBUG,"Transfer Packet to Local HF Node!\n");
		
		Board_Data_SendTo_Inc_Addr((uint8 *)&pro, len+sizeof(ST_HF_MGW_DCU_PRO_HEADER), sin);
		
		return 0;
	}

	/*寻找到下一跳节点信息，转发数据包*/
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = next_hf_node->js_ip_addr;

	/*如果下一跳是本节点则转发数据包到数据交换板*/
	if(next_hf_node->hf_addr == g_hf_addr)
	{
		pro.header.baowen_type = BAOWEN_TYPE_MGW_DCU;
		pro.header.dst_addr = 0x10;
		pro.header.src_addr = 0xb0;
		pro.header.info_type = INT_FRAME_TYPE_DATA;
		pro.header.data_len = len;

		memcpy(pro.body,buf,len);
		
		sin.sin_addr.s_addr = inet_addr(HF_MGW_SCU_SCUDATA_IP);
		sin.sin_port = HF_MGW_SCU_SCUDATA_PORT;

		inc_log(LOG_DEBUG,"Transfer Packet to Local HF Node!\n");

		Board_Data_SendTo_Inc_Addr((uint8 *)&pro, len+sizeof(ST_HF_MGW_DCU_PRO_HEADER), sin);
		
		if(res<0)
			inc_log(LOG_DEBUG,"Transfer Packet to Local HF Error\n");
		
		return 0;
		
	}else{
		sin.sin_addr.s_addr = next_hf_node->js_ip_addr;
		sin.sin_port = 20035;
		
		inc_log(LOG_DEBUG,"Transfer Packet to %s!\n",inc_inet_ntoa(sin.sin_addr));
		
		//res = sendto(js_data_udp_socket.udp_handle,buf,len, 0,(struct sockaddr*)&sin,js_data_udp_socket.addr_len);	
	}

	return res;
}

int mgw_add_nat_node(unsigned short port,unsigned long ip_addr)
{
	unsigned long		tmp_ip_addr;
	unsigned short		tmp_port;
	unsigned short		ip_cnt=0,port_cnt=0;
	struct JS_NAT_NODE * tmp_js_nat_node;
	struct JS_NAT_TABLE_T * tmp_js_nat_table_t;

	tmp_ip_addr = ip_addr;
	tmp_port = port;

	if(!tmp_ip_addr || tmp_ip_addr == 0xffffffff)
		return (-1);
	
	if(tmp_port<=1024)
		return (-1);

	tmp_js_nat_node = inc_calloc(1,sizeof(struct JS_NAT_NODE));
	INCOBJ_INIT(tmp_js_nat_node);

	INCOBJ_WRLOCK(tmp_js_nat_node);
	tmp_js_nat_node->dst_ip_addr= tmp_ip_addr;
	INCOBJ_UNLOCK(tmp_js_nat_node);

	if(INC_LIST_EMPTY(&js_nat_table))
	{
		tmp_js_nat_table_t = inc_calloc(1,sizeof(struct JS_NAT_TABLE_T));
		INCOBJ_CONTAINER_INIT(tmp_js_nat_table_t);		
		tmp_js_nat_table_t->port = tmp_port;
		INCOBJ_CONTAINER_LINK(tmp_js_nat_table_t,tmp_js_nat_node);
		
		INC_LIST_LOCK(&js_nat_table);
		INC_LIST_INSERT_HEAD(&js_nat_table,tmp_js_nat_table_t,list);		
		INC_LIST_UNLOCK(&js_nat_table);
		
	}else{

		INC_LIST_TRAVERSE_SAFE_BEGIN(&js_nat_table,tmp_js_nat_table_t,list)
		{
			if(port == tmp_js_nat_table_t->port){
				if(INCOBJ_CONTAINER_FIND_FULL(tmp_js_nat_table_t,tmp_ip_addr,dst_ip_addr,NULL,NULL,is_equal))
					return (-1);
				
				INCOBJ_CONTAINER_TRAVERSE(tmp_js_nat_table_t, 1, ip_cnt++);
				if(ip_cnt<MAX_DNAT_IP_COUNT)
					INCOBJ_CONTAINER_LINK(tmp_js_nat_table_t,tmp_js_nat_node);
				else{
					inc_log(LOG_DEBUG,"add nat node error cause ip is too many!\n");
					return (-1);
				}
				return 0;
			}
		}
		INC_LIST_TRAVERSE_SAFE_END
		

		INC_LIST_LOCK(&js_nat_table);
		INC_LIST_TRAVERSE(&js_nat_table,tmp_js_nat_table_t,list)
			port_cnt++;
		if(port_cnt>=MAX_DNAT_PORT_COUNT){
			inc_log(LOG_DEBUG,"add nat node error cause port is too many!\n");
			INCOBJ_UNREF(tmp_js_nat_node,free);
			INC_LIST_UNLOCK(&js_nat_table);
			return (-1);
		}else{
			tmp_js_nat_table_t = inc_calloc(1,sizeof(struct JS_ROUTE_TABLE_T));
			INCOBJ_CONTAINER_INIT(tmp_js_nat_table_t);
			
			tmp_js_nat_table_t->port = tmp_port;
			INCOBJ_CONTAINER_LINK(tmp_js_nat_table_t,tmp_js_nat_node);		
			INC_LIST_INSERT_HEAD(&js_nat_table,tmp_js_nat_table_t,list);
		}
		INC_LIST_UNLOCK(&js_nat_table);
	}
	return 0;
}

int mgw_sub_nat_node(unsigned short port,unsigned long ip_addr)
{
	unsigned long		tmp_ip_addr;
	unsigned short		tmp_port;
	struct JS_NAT_NODE * tmp_js_nat_node;
	struct JS_NAT_TABLE_T * tmp_js_nat_table_t;

	if(INC_LIST_EMPTY(&js_nat_table))
		return (-1);

	tmp_ip_addr = ip_addr;
	tmp_port = port;
	
	INC_LIST_TRAVERSE_SAFE_BEGIN(&js_nat_table,tmp_js_nat_table_t,list)
	{
		if(port == tmp_js_nat_table_t->port){
			tmp_js_nat_node = INCOBJ_CONTAINER_FIND_UNLINK_FULL(tmp_js_nat_table_t,ip_addr,dst_ip_addr,NULL,NULL,is_equal);
			if(tmp_js_nat_node)
				INCOBJ_UNREF(tmp_js_nat_node,free);
			return 0;
		}
	}
	INC_LIST_TRAVERSE_SAFE_END

	return (-1);
}

int mgw_sub_nat_port(unsigned short port)
{
	unsigned short		tmp_port;
	struct JS_NAT_TABLE_T * tmp_js_nat_table_t;

	if(INC_LIST_EMPTY(&js_nat_table))
		return (-1);

	tmp_port = port;
	
	INC_LIST_TRAVERSE_SAFE_BEGIN(&js_nat_table,tmp_js_nat_table_t,list)
	{
		if(port == tmp_js_nat_table_t->port){
			INC_LIST_REMOVE_CURRENT(&js_nat_table, list);
			return 0;
		}
	}
	INC_LIST_TRAVERSE_SAFE_END

	return (-1);
}

int do_nat_atction (cmd_tbl_t * cmdtp,  int argc, char *argv[])
{
	struct JS_NAT_TABLE_T * tmp_js_nat_table_t;
	unsigned short tmp_port;
	unsigned long tmp_ip_addr;

	if(argc == 1)
	{
		INC_LIST_TRAVERSE_SAFE_BEGIN(&js_nat_table,tmp_js_nat_table_t,list)
		{
			typeof(tmp_js_nat_table_t->head) iterator = NULL;
			typeof(tmp_js_nat_table_t->head) next = NULL;
			DEBUG_OUT("port %d: \n",tmp_js_nat_table_t->port);
			INCOBJ_CONTAINER_RDLOCK(tmp_js_nat_table_t);
			next = tmp_js_nat_table_t->head;
			//while(iterator = next){
			for(; NULL != next; iterator = next)
			{
				next = iterator->next[0];
				printf("dst:%.8x\n",iterator->dst_ip_addr);
			}
			INCOBJ_CONTAINER_UNLOCK(tmp_js_nat_table_t);
		}
		INC_LIST_TRAVERSE_SAFE_END

		return 0;
	}else if(argc == 4 && strcasecmp(argv[1],"add")==0){
		tmp_port = atoi(argv[2]);
		tmp_ip_addr = inet_addr(argv[3]);
		mgw_add_nat_node(tmp_port,tmp_ip_addr);
	}else if(argc == 4 && strcasecmp(argv[1],"del")==0){
		tmp_port = atoi(argv[2]);
		tmp_ip_addr = inet_addr(argv[3]);
		mgw_sub_nat_node(tmp_port,tmp_ip_addr);	
	}else{
		printf("Invalid Parameter!\n");
		printf("usage: %s",cmdtp->usage);
		return 0;
	}

	return 0;
}

void rewrite_nat_table(struct config * cfg)
{
	struct JS_NAT_TABLE_T * tmp_js_nat_table_t;
	//unsigned short tmp_port;
	//unsigned long tmp_ip_addr;
	char dnat_cmd[128];
	struct in_addr addr;

	add_config_var(cfg,"DNAT","snat","iptables -t nat -A POSTROUTING -o eth1 -j MASQUERADE");

	INC_LIST_TRAVERSE_SAFE_BEGIN(&js_nat_table,tmp_js_nat_table_t,list)
	{
		typeof(tmp_js_nat_table_t->head) iterator = NULL;
		typeof(tmp_js_nat_table_t->head) next = NULL;
		//DEBUG_OUT("port %d: \n",tmp_js_nat_table_t->port);
		INCOBJ_CONTAINER_RDLOCK(tmp_js_nat_table_t);
		next = tmp_js_nat_table_t->head;
		//while(iterator = next){
		for(; NULL != next; iterator = next)
		{
			next = iterator->next[0];
			//printf("dst:%.8x\n",iterator->dst_ip_addr);
			addr.s_addr = iterator->dst_ip_addr;
			sprintf(dnat_cmd,"iptables -t nat -A PREROUTING -i eth1 -p udp --dport %d -j DNAT --to %s",
				tmp_js_nat_table_t->port,inc_inet_ntoa(addr));
			append_config_var(cfg,"DNAT","dnat",dnat_cmd);
		}
		INCOBJ_CONTAINER_UNLOCK(tmp_js_nat_table_t);
	}
	INC_LIST_TRAVERSE_SAFE_END
}

#if 0
void	*do_nat_monitor(void *arg)
{
	if(!(nat_ioc = io_context_create()))
	{
		VERBOSE_OUT(LOG_SYS,"Create IO Context Error!\n");
		return NULL;
	}
	
	INCOBJ_CONTAINER_INIT(&js_user_nat_list);
	nat_port_start = 20000;	/*端口范围*/
	nat_port_end  = 30000;

	VERBOSE_OUT(LOG_SYS,"do_nat_monitor is running...\n");
	
	while(1){
		io_wait(nat_ioc,20);
	}	
}

unsigned short	get_nat_port()
{
	return (nat_port_end == nat_port_start) ? nat_port_start : (random() % (nat_port_end - nat_port_start)) + nat_port_start;
}

int	mgw_nat_read(int *id, int fd, short events, void *cbdata)
{
	int res;
	struct sockaddr_in sin;
	UDP_SOCKET * sck = (UDP_SOCKET *)cbdata;

	res = recvfrom(sck->udp_handle,sck->rawdata,sizeof(sck->rawdata), \
		0,(struct sockaddr*)&sin,&sck->addr_len);

	if(res < 0)
	{
		/*接收回的报文如何改变源地址发送到席位用户*/
	}
}

int *nat_io_add(int fd, void *data)
{
	return io_add(nat_ioc,fd,mgw_nat_read,POLLIN,data);
}

int nat_compare(struct sockaddr_in sin_a,struct sockaddr_in sin_b)
{
	if(sin_a.sin_addr != sin_b.sin_addr)
		return 1;

	if(sin_a.sin_port != sin_b.sin_port)
		return 1;

	return 0;
}

int transfer_transmit_packet(struct sockaddr_in * src_sin,struct sockaddr_in * dst_sin,const char * buf,size_t len)
{
	struct JS_NAT_NODE * tmp_nat_node;
	tmp_nat_node = INCOBJ_CONTAINER_FIND_FULL(&js_user_nat_list,*sin,src_sin,NULL,NULL,nat_compare);

	if(!tmp_nat_node)
	{
		/*添加新的NAT的表项*/
		tmp_nat_node = inc_calloc(1,sizeof(struct JS_NAT_NODE));
		INCOBJ_INIT(tmp_nat_node);

		tmp_nat_node->src_sin.sin_addr = src_sin->sin_addr;
		tmp_nat_node->src_sin.sin_port = src_sin->sin_port;
		tmp_nat_node->transfer_port = get_nat_port();
		
		tmp_nat_node->nat_sck.udp_handle = socket(AF_INET,SOCK_DGRAM,0);
		if(tmp_nat_node->nat_sck.udp_handle < 0)
		{
			perror("Create Socket Error:");
			free(tmp_nat_node);
			return 1;
		}
		tmp_nat_node->nat_sck.addr_len = sizeof(struct sockaddr_in);
		
		tmp_nat_node->nat_sck.addr_us = js_udp_socket.addr_us;
		tmp_nat_node->nat_sck.addr_us.sin_port = htons(tmp_nat_node->transfer_port);
		tmp_nat_node->nat_sck.addr_them = *dst_sin;
		tmp_nat_node->nat_sck.addr_them.sin_family = AF_INET;

rebind:if(!bind(tmp_nat_node->nat_sck.udp_handle, \
			(struct sockaddr*)&tmp_nat_node->nat_sck.addr_us, \
			tmp_nat_node->nat_sck.addr_len) != 0){

			/*如果由重复绑定的错误则重新绑定*/
			if(errno == EADDRINUSE)
			{
				tmp_nat_node->transfer_port = get_nat_port();
				tmp_nat_node->nat_sck.addr_us.sin_port = htons(tmp_nat_node->transfer_port);
				goto rebind;
			}else{
				perror("Bind Socket Error:");
				free(tmp_nat_node);
				return 1;
			}
		}
		
		nat_io_add(tmp_nat_node->nat_sck.udp_handle,&tmp_nat_node->nat_sck);
		INCOBJ_CONTAINER_LINK(&js_user_nat_list,tmp_nat_node);
	}

	return sendto(tmp_nat_node->nat_sck.udp_handle,buf,len, \
		0,(struct sockaddr*)&tmp_nat_node->nat_sck.addr_them,tmp_nat_node->nat_sck.addr_len);
}
#endif

