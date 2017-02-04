#ifndef _MGW_RCB_H_
#define _MGW_RCB_H_


#if defined(__cplusplus)||defined(c_plusplus)
extern "C"{
#endif

enum{
	JS_NODE_STATE_INIT = 0x00,
	JS_NODE_STATE_ALIVE,
	JS_NODE_STATE_ACTIVE,
	JS_NODE_STATE_INACTIVE,
	JS_NODE_STATE_IDLE,
};

enum{
	JS_ROUTE_NODE_STATE_INIT = 0x00,
	JS_ROUTE_NODE_STATE_ALIVE,
	JS_ROUTE_NODE_STATE_ACTIVE,
	JS_ROUTE_NODE_STATE_INACTIVE,
	JS_ROUTE_NODE_STATE_IDLE,
};

enum{
	ROUTE_TYPE_HF = 0x00,
	ROUTE_TYPE_JS,
};

struct MGW_INTERFACE{
	unsigned long		ip_addr;
	unsigned char		ip_mask;
	unsigned long		ip_gw;
};

#define	MAX_TUNNEL_COUNT			25
#define	MAX_DNAT_PORT_COUNT		20
#define	MAX_DNAT_IP_COUNT		30

struct JS_NODE{
	INCOBJ_COMPONENTS(struct JS_NODE);
	
	unsigned long		hf_addr;
	struct MGW_INTERFACE mgw_if;		/*IP接口*/
	unsigned long		js_ip_addr;
	unsigned char		js_ip_mask;
	unsigned short		cost;
	unsigned char		dep_num[16];
	unsigned char		state;			/*标识既设网节点的状态*/
	unsigned char		inactive_time;	/*非活动时间*/
};

struct JS_ROUTE_NODE{
	INCOBJ_COMPONENTS(struct JS_ROUTE_NODE);
	
	unsigned long		dst_hf_addr;
	unsigned long		next_hop;		/*下一跳海防地址*/
	unsigned short		cost;
	unsigned char		dep_num[16];
	unsigned char		state;			/*标识既设网节点的状态*/
	unsigned char		inactive_time;	/*非活动时间*/	
	unsigned char		type;			/*路由节点类型*/
};

struct JS_NAT_NODE{
	INCOBJ_COMPONENTS(struct JS_NAT_NODE);
	unsigned long		dst_ip_addr;
};

struct JS_LIST{
	INCOBJ_CONTAINER_COMPONENTS(struct JS_NODE);
};

struct JS_ROUTE_TABLE_T{
	INCOBJ_CONTAINER_COMPONENTS(struct JS_ROUTE_NODE);
	INC_LIST_ENTRY(JS_ROUTE_TABLE_T) list;
	unsigned long		mask;
};

struct JS_NAT_TABLE_T{
	INCOBJ_CONTAINER_COMPONENTS(struct JS_NAT_NODE);
	INC_LIST_ENTRY(JS_NAT_TABLE_T) list;
	unsigned short		port;
};

#include <arpa/inet.h>
#define		is_equal(a,b)			((a)!=(b))
#define		GET_HF_MASK(addr)		(addr & 0xFFFFFFF0)


INC_LIST_HEAD(JS_ROUTE_TABLE,JS_ROUTE_TABLE_T);
INC_LIST_HEAD(JS_NAT_TABLE,JS_NAT_TABLE_T);

/*===================gobal variable======================*/
extern struct JS_LIST js_user_list;
extern struct JS_ROUTE_TABLE js_route_user_table;
extern struct JS_NAT_TABLE js_nat_table;
/*===================command function====================*/
int do_add_js_node (cmd_tbl_t * cmdtp, int argc, char *argv[]);
int do_sub_js_node (cmd_tbl_t * cmdtp, int argc, char *argv[]);
int do_insert_js_rt (cmd_tbl_t * cmdtp, int argc, char *argv[]);
int mgw_js_route_msg(void);
int mgw_add_js_node(unsigned long hf_addr,struct MGW_INTERFACE ,unsigned long ip_addr,unsigned char ip_mask,unsigned short cost,const char*);
int mgw_insert_js_rt(unsigned long dst_addr,unsigned long next_hop_addr,unsigned long mask,unsigned short cost,unsigned char type,const char * dep_num);
int	init_mgw_rcb(const void * data);
int mgw_add_nat_node(unsigned short port,unsigned long ip_addr);
int mgw_sub_nat_node(unsigned short port,unsigned long ip_addr);
int do_nat_atction (cmd_tbl_t * cmdtp, int argc, char *argv[]);
int mgw_sub_js_node(unsigned long ip_addr);
int mgw_sub_nat_port(unsigned short port);
void rewrite_nat_table(struct config * cfg);
struct JS_NODE * mgw_find_js_node(unsigned long ip_addr);
void send_route_msg(struct JS_NODE * node);
void mgw_dcu_route_msg(void);
int send_dcu_route_msg(struct JS_ROUTE_NODE * node,unsigned char rt_index);
int process_tunnel_cmd_str(const char* cmd_str);
int process_dnat_cmd_str(const char* cmd_str);

#if defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif

