#ifndef _EXTERIOR_PROTOCOL_H_
#define _EXTERIOR_PROTOCOL_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"{
#endif

#define		EXTMAXDATASIZE		1024
#define		EXTMAXDSTCOUNT		63
#define		INTMAXMEMCOUNT		16
#define		MAXPHONENUM			20
#define		MAXPHONENUMBYTE		(MAXPHONENUM>>1)

typedef enum
{
	EN_TYPE_USER_NUM_REQ = 0x01,						/*用户号码配置消息*/
	EN_TYPE_USER_NUM_ANS,								/*用户号码配置应答消息*/
	EN_TYPE_CONF_NUM_REG_REQ,							/*预约会议号码注册(注销)请求消息*/
	EN_TYPE_CONF_NUM_REG_ANS,							/*预约会议号码注册(注销)应答消息*/
	EN_TYPE_SP_NUM_REG_REQ,								/*专线号码注册(注销)请求消息*/
	EN_TYPE_SP_NUM_REG_ANS,								/*专线号码注册(注销)应答消息*/
	EN_TYPE_USER_RIGHT_REQ,								/*用户呼叫权限设置消息*/
	EN_TYPE_USER_RIGHT_ANS,								/*用户呼叫权限设置应答消息*/
}EN_MSG_CALL_CFG_TYPE;


typedef enum
{
	EN_TYPE_CALL_REQ = 0x01,								/*呼叫请求消息*/
	EN_TYPE_CALL_ANS,										/*呼叫应答消息*/
	EN_TYPE_CONNECT_REQ,									/*连接请求消息*/
	EN_TYPE_RELEASE_REQ,									/*释放请求消息*/
	EN_TYPE_RELEASE_ANS,									/*释放证实消息*/
	EN_TYPE_PTT = 0x0a,										/*PTT按键消息*/
	EN_TYPE_NUM_TRANS,										/*号码传递消息*/
	EN_TYPE_BUZZER_CTL = 0x10,										/*号码传递消息*/	
}EN_MSG_CALL_CTL_TYPE;

/*接续类型*/
typedef enum
{
	EN_SEL_CALL = 0x01,										/*选呼*/
	EN_PLAN_CONF,											/*预约全双工会议*/
	EN_RANDOM_CONF,											/*0x03   随机全双工会议*/
	EN_GROUP_CALL,											/*0x04   群呼*/
	EN_ADD_CALL_MEMBER,										/*0x05   增加会议成员*/
	EN_FORCE_CALL,											/*0x06   强插*/
	EN_FORCE_RELEASE_CALL,									/*0x07   强拆*/
	EN_LISTENING,											/*0x08   监听*/
	EN_CALL_FORWARD_TO,										/*0x09   呼叫转接*/
	EN_CALL_TRUNK,											/*0x0a	 中继*/
	EN_CALL_Z,										/*0x0b   战网/直呼*/
	EN_CALL_RADIO,										/*0x0c  radio direct call*/
	MAX_CALL_JIEXU_TYPE,
}EN_CALL_JIEXU_TYPE;

/*接续状态*/
typedef enum
{
	EN_JIEXU_SUCCESS = 0x00,
	EN_JIEXU_NONUM = 0x01,
	EN_JIEXU_NUMERROR = 0x02,
	EN_JIEXU_FAILURE = 0xff,
}EN_CALL_JIEXU_STATU;

/*释放原因*/
typedef enum
{
	EN_RELEASE_TYPE_NOR = 0x01,
	EN_RELEASE_TYPE_HARD_ERROR,
	EN_RELEASE_TYPE_TIMEOUT
}EN_RELEASE_TYPE;
typedef enum
{
	EN_BUZZER_CTL_OFF = 0x00,
	EN_BUZZER_CTL_ON = 0x01,
	EN_BUZZER_CTL_BUFF
}EN_BUZZER_CTL_TYPE;




/*========================数据终端用户接口协议======================================*/
typedef enum
{
	BAOWEN_TYPE_PBZK = 0,	/*炮兵指控通信网协议报文*/
	BAOWEN_TYPE_FKZK,			/*防空指控通信网协议报文*/
	BAOWEN_TYPE_WG,				/*网管协议报文*/
	BAOWEN_TYPE_YFKQ			/*远方空情报文*/
} EXTERIOR_BAOWEN_TYPE;

typedef struct
{
	unsigned char	protocol_type;						/*协议类型*/
	EXTERIOR_BAOWEN_TYPE	baowen_version		:8;		/*报文及版本标示*/
	unsigned char	header_len;							/*头部长度*/
	unsigned char	info_type;							/*信息类型*/
	unsigned char	chan_type;							/*信道类型*/
	unsigned char	server_type;						/*服务类型*/
	unsigned long	src_addr;							/*源地址*/
	unsigned char	dst_count;							/*目标地址总数*/
	unsigned long	dst_addr[EXTMAXDSTCOUNT];			/*目标地址数组*/
	unsigned short	len;								/*报文长度--正文长度*/
}__attribute__ ((packed)) ST_HF_CCC_CCU_PRO_HEADER;

typedef struct
{
	ST_HF_CCC_CCU_PRO_HEADER header;
	unsigned char	body[EXTMAXDATASIZE];			/*正文*/
}__attribute__ ((packed)) ST_HF_CCC_CCU_PRO;

/*=====================网管软件对媒体网关板的配置协议内容=====================*/
enum
{
	INFO_TYPE_CCU_MGW_IP_CONFIG_REQ_ID = 0x20,
	INFO_TYPE_CCU_MGW_IP_CONFIG_ANS_ID = 0x21,
	INFO_TYPE_CCU_MGW_ADD_IP_SLOT_REQ_ID = 0x30,
	INFO_TYPE_CCU_MGW_ADD_IP_SLOT_ANS_ID = 0x31,
	INFO_TYPE_CCU_MGW_SUB_IP_SLOT_REQ_ID = 0x32,
	INFO_TYPE_CCU_MGW_SUB_IP_SLOT_ANS_ID = 0x33,
	INFO_TYPE_CCU_MGW_MODIFY_IP_SLOT_REQ_ID = 0x34,
	INFO_TYPE_CCU_MGW_MODIFY_IP_SLOT_ANS_ID = 0x35,
	INFO_TYPE_CCU_MGW_ADD_FENJI_REQ_ID = 0x36,
	INFO_TYPE_CCU_MGW_ADD_FENJI_ANS_ID = 0x37,
	INFO_TYPE_CCU_MGW_SUB_FENJI_REQ_ID = 0x38,
	INFO_TYPE_CCU_MGW_SUB_FENJI_ANS_ID = 0x39,
	INFO_TYPE_CCU_MGW_ADD_TUNNEL_REQ_ID = 0x3a,
	INFO_TYPE_CCU_MGW_ADD_TUNNEL_ANS_ID = 0x3b,
	INFO_TYPE_CCU_MGW_SUB_TUNNEL_REQ_ID = 0x3c,
	INFO_TYPE_CCU_MGW_SUB_TUNNEL_ANS_ID = 0x3d,
	INFO_TYPE_CCU_MGW_ADD_DNAT_REQ_ID = 0x3e,
	INFO_TYPE_CCU_MGW_ADD_DNAT_ANS_ID = 0x3f,
	INFO_TYPE_CCU_MGW_SUB_DNAT_REQ_ID = 0x40,
	INFO_TYPE_CCU_MGW_SUB_DNAT_ANS_ID = 0x41,
	INFO_TYPE_CCU_MGW_SUB_DNAT_PORT_REQ_ID = 0x42,
	INFO_TYPE_CCU_MGW_SUB_DNAT_PORT_ANS_ID = 0x43,
	INFO_TYPE_CCU_MGW_DETECT_ETH_REQ_ID = 0x50,
	INFO_TYPE_CCU_MGW_DETECT_ETH_ANS_ID = 0x51,
	INFO_TYPE_CCU_MGW_DETECT_IP_SLOT_REQ_ID = 0x54,
	INFO_TYPE_CCU_MGW_DETECT_IP_SLOT_ANS_ID = 0x55,
	INFO_TYPE_CCU_MGW_DETECT_FENJI_REQ_ID = 0x56,
	INFO_TYPE_CCU_MGW_DETECT_FENJI_ANS_ID = 0x57,
	INFO_TYPE_CCU_MGW_DETECT_TUNNEL_REQ_ID = 0x58,
	INFO_TYPE_CCU_MGW_DETECT_TUNNEL_ANS_ID = 0x59,
	INFO_TYPE_CCU_MGW_DETECT_DNAT_PORT_REQ_ID = 0x5a,
	INFO_TYPE_CCU_MGW_DETECT_DNAT_PORT_ANS_ID = 0x5b,
	INFO_TYPE_CCU_MGW_DETECT_DNAT_IP_REQ_ID = 0x5c,
	INFO_TYPE_CCU_MGW_DETECT_DNAT_IP_ANS_ID = 0x5d,
	INFO_TYPE_CCU_MGW_SELF_CHECK_REQ_ID = 0x70,
	INFO_TYPE_CCU_MGW_SELF_CHECK_ANS_ID = 0x71,
	INFO_TYPE_CCU_MGW_SELF_RESET_REQ_ID = 0x72,
	INFO_TYPE_CCU_MGW_SELF_RESET_ANS_ID = 0x73,
};

typedef struct
{
	unsigned char	info_type;		/*信息类型*/
	unsigned short	len;			/*信息长度*/
}__attribute__ ((packed)) ST_HF_CCU_MGW_PRO_HEADER;

typedef struct
{
	ST_HF_CCU_MGW_PRO_HEADER header;
	unsigned char	body[256];
}__attribute__ ((packed)) ST_HF_CCU_MGW_PRO;

/*网口设置，应答消息*/
typedef struct
{
	unsigned char	eth_index;		/*网口编号*/
	unsigned long	ip_addr;		/*IP地址*/
	unsigned long	ip_mask;		/*IP地址掩码*/
	unsigned short	udp_port;		/*端口号*/
}__attribute__ ((packed)) ST_HF_CCU_MGW_IP_CONFIG_REQ;

typedef struct
{
	unsigned char	status;			/*状态*/
}__attribute__ ((packed)) ST_HF_CCU_MGW_IP_CONFIG_ANS;

/*时隙<->IP下发消息*/
typedef struct
{
	unsigned char	slot;			/*时隙号*/
	unsigned long	ip_addr;		/*IP地址*/
}__attribute__ ((packed)) ST_HF_CCU_MGW_ADD_IP_SLOT_REQ,ST_HF_CCU_MGW_MODIFY_IP_SLOT_REQ;

typedef struct
{
	unsigned char	status;			/*应答状态*/
}__attribute__ ((packed)) ST_HF_CCU_MGW_ADD_IP_SLOT_ANS,ST_HF_CCU_MGW_MODIFY_IP_SLOT_ANS;

typedef struct
{
	unsigned char	slot;			/*成员逻辑号*/
}__attribute__ ((packed)) ST_HF_CCU_MGW_SUB_IP_SLOT_REQ;

typedef struct
{
	unsigned char	status;			/*应答状态*/
}__attribute__ ((packed)) ST_HF_CCU_MGW_SUB_IP_SLOT_ANS;

/*振入分机号码设置*/
typedef struct
{
	unsigned char	index;
	unsigned long	fenji_num;
}__attribute__ ((packed)) ST_HF_CCU_MGW_ADD_FENJI_REQ;

typedef struct
{
	unsigned char	status;			/*应答状态*/
}__attribute__ ((packed)) ST_HF_CCU_MGW_ADD_FENJI_ANS;

typedef struct
{
	unsigned char	index;
	unsigned long	fenji_num;
}__attribute__ ((packed)) ST_HF_CCU_MGW_SUB_FENJI_REQ;

typedef struct
{
	unsigned char	status;			/*应答状态*/
}__attribute__ ((packed)) ST_HF_CCU_MGW_SUB_FENJI_ANS;

typedef struct
{
	unsigned long	fenji1;
	unsigned long 	fenji2;
}__attribute__ ((packed)) ST_HF_CCU_MGW_DETECT_FENJI_ANS;

/*查询网口参数参数*/
typedef struct
{
	struct
	{
		unsigned char	eth_index;
		unsigned long	ip_addr;
		unsigned long	ip_mask;
		unsigned short	udp_port;
	}__attribute__((packed)) eth_local[5];
}__attribute__ ((packed)) ST_HF_CCU_MGW_DETECT_ETH_ANS;

/*查询IP<->时隙对应信息*/
typedef struct
{
	unsigned char	terminal_count;
	struct
	{
		unsigned char	slot;
		unsigned long	ip_addr;
	}__attribute__((packed)) ter_local[17];
}__attribute__ ((packed)) ST_HF_CCU_MGW_DETECT_IP_SLOT_ANS;

/*模块自检*/
typedef struct
{
	unsigned char	dst_addr;
}__attribute__ ((packed)) ST_HF_CCU_MGW_SELF_CHECK_REQ;

typedef struct
{
	unsigned char	err_count;
	unsigned char	err_index;
	unsigned char	err_info;
	unsigned char	err_type;
}__attribute__ ((packed)) ST_HF_CCU_MGW_SELF_CHECK_ANS;

/*恢复出厂设置消息*/
typedef struct
{
	unsigned char	status;
}__attribute__ ((packed)) ST_HF_CCU_MGW_SELF_RESET_ANS;

typedef struct
{
	unsigned long	src_ip_addr;
	unsigned char	src_ip_mask;
	unsigned long	gw_ip_addr;
	unsigned char	tunnel_count;
	struct
	{
		unsigned long	dst_ip_addr;
		unsigned char	dst_ip_mask;
		unsigned char	tunnel_type;
	}__attribute__ ((packed)) tunnel[4];
}__attribute__ ((packed)) ST_HF_CCU_MGW_ADD_TUNNEL_REQ,ST_HF_CCU_MGW_DETECT_TUNNEL_ANS;

typedef struct
{
	unsigned char	status;
}__attribute__ ((packed)) ST_HF_CCU_MGW_ADD_TUNNEL_ANS;

typedef struct
{
	unsigned long	src_ip_addr;
	unsigned char	tunnel_count;
	struct
	{
		unsigned long	dst_ip_addr;
	}__attribute__ ((packed)) tunnel[4];
}__attribute__ ((packed)) ST_HF_CCU_MGW_SUB_TUNNEL_REQ;

typedef struct
{
	unsigned char	status;
}__attribute__ ((packed)) ST_HF_CCU_MGW_SUB_TUNNEL_ANS;

/*DNAT配置消息*/
typedef struct
{
	unsigned short	port;
	unsigned char	dnat_count;
	struct
	{
		unsigned long	dst_ip_addr;
	}__attribute__ ((packed)) dnat[30];
}__attribute__ ((packed)) ST_HF_CCU_MGW_ADD_DNAT_REQ,ST_HF_CCU_MGW_SUB_DNAT_REQ,ST_HF_CCU_MGW_DETECT_DNAT_IP_ANS;

typedef struct
{
	unsigned char	status;
}__attribute__ ((packed)) ST_HF_CCU_MGW_ADD_DNAT_ANS,ST_HF_CCU_MGW_SUB_DNAT_ANS,ST_HF_CCU_MGW_SUB_DNAT_PORT_ANS;

typedef struct
{
	unsigned short	port;
}__attribute__ ((packed)) ST_HF_CCU_MGW_SUB_DNAT_PORT_REQ,ST_HF_CCU_MGW_DETECT_DNAT_IP_REQ;

typedef struct
{
	unsigned char	port_count;
	struct{
		unsigned short	port;
	}__attribute__ ((packed)) port[20];
}__attribute__ ((packed)) ST_HF_CCU_MGW_DETECT_DNAT_PORT_ANS;

//#define MAX_SOCKET_LEN 300

typedef struct _BOARDD_CMD_//板间数据正文
{
	uint8  type;
	uint16 len;
	uint8  cmddat[81];
}__attribute__((packed)) ST_BOARDDTEMP_CMD;

typedef struct _BOARD_DATA_HEAD
{
	uint8 	PackType;	//报文类型
	uint8 	EquDestAddr8;//设备目的地址(接口板地址或接口板端口地址)
	uint8 	EquSourceAddr8;//设备源地址
	uint8 	FrameType;//信息帧类型
	uint16 	DataLen;	//数据长度
}__attribute__((packed)) ST_BOARD_DATA_HEAD;
typedef struct _BOARD_DATA
{
	ST_BOARD_DATA_HEAD Head;
	uint8 	Data[300];//数据
}__attribute__((packed)) ST_BOARD_DATA;


/*呼叫信令*/
enum
{
	BAOWEN_TYPE_CALLCFG = 0x00,
	BAOWEN_TYPE_CALLCMD
};

enum
{
	ENCODER_G711 = 0x04,
	ENCODER_G729
};

typedef struct
{
	EN_MSG_CALL_CTL_TYPE	id					:8;		/*消息标示*/
	unsigned char			msg_len;					/*消息长度--sizeof(ST_HF_MGW_EXT_CALL_REQ)-2*/
	unsigned char			slot;						/*终端标示--时隙*/
	EN_CALL_JIEXU_TYPE		call_type			:8;		/*接续类型*/
	unsigned char			encoder;					/*话音编码*/
	unsigned char			phone_len;					/*号码长度<=20*/
	unsigned char			callee_num[10];				/*被叫号码*/
}__attribute__ ((packed)) ST_HF_MGW_EXT_CALL_REQ,ST_HF_MGW_EXT_CALL_EVENT;


typedef struct
{
	EN_MSG_CALL_CTL_TYPE	id					:8;		/*消息标示*/
	unsigned char			msg_len;					/*消息长度*/
	unsigned char			slot;						/*终端标示--时隙*/
	EN_CALL_JIEXU_TYPE		call_type			:8;		/*接续类型*/
	EN_CALL_JIEXU_STATU		call_statu			:8;		/*接续状态*/
}__attribute__ ((packed)) ST_HF_MGW_EXT_CALLACK_REQ,ST_HF_MGW_EXT_CALLACK_EVENT;

typedef struct
{
	EN_MSG_CALL_CTL_TYPE	id					:8;		/*消息标示*/
	unsigned char			msg_len;					/*消息长度*/
	unsigned char			slot;						/*终端标示--时隙*/
	EN_CALL_JIEXU_TYPE		call_type			:8;		/*接续类型*/
	unsigned char			encoder;					/*语音编码*/
	unsigned char			phone_len;					/*号码长度*/
	unsigned char			caller_num[10];				/*被叫号码*/
	unsigned short			rtp_port;					/*RTP端口号*/
}__attribute__ ((packed)) ST_HF_MGW_EXT_CONNECT_REQ,ST_HF_MGW_EXT_CONNECT_EVENT;


typedef struct
{
	EN_MSG_CALL_CTL_TYPE	id					:8;		/*消息标示*/
	unsigned char			msg_len;					/*消息长度*/
	unsigned char			slot;						/*终端标示--时隙*/
	EN_CALL_JIEXU_TYPE		call_type			:8;		/*接续类型*/
	EN_RELEASE_TYPE			rel_type			:8;		/*释放原因*/
}__attribute__ ((packed)) ST_HF_MGW_EXT_RELEASE_REQ;


typedef struct
{
	EN_MSG_CALL_CTL_TYPE	id					:8;		/*消息标示*/
	unsigned char			msg_len;					/*消息长度*/
	unsigned char			slot;						/*终端标示--时隙*/
	EN_CALL_JIEXU_TYPE		call_type			:8;		/*接续类型*/
}__attribute__ ((packed)) ST_HF_MGW_EXT_RELEASE_CNF;

typedef struct
{
	EN_MSG_CALL_CTL_TYPE	id					:8;		/*消息标示*/
	unsigned char			msg_len;					/*消息长度*/
	unsigned char			slot;						/*终端标示--时隙*/
	EN_BUZZER_CTL_TYPE	buzzer_status			:8;		/*蜂鸣器状态*/
}__attribute__ ((packed)) ST_HF_MGW_EXT_BUZZER_CTL;
typedef struct
{
	EN_MSG_CALL_CTL_TYPE	id					:8;		/*消息标示*/
	unsigned char			msg_len;					/*消息长度*/
	unsigned char			slot;						/*终端标示--时隙*/
	unsigned char			ptt_value;					/*ptt按键值*/
}__attribute__ ((packed)) ST_HF_MGW_EXT_PTT_MSG;


typedef struct
{
	EN_MSG_CALL_CTL_TYPE	id					:8;		/*消息标示*/
	unsigned char			msg_len;					/*消息长度*/
	unsigned char			slot;						/*终端标示--时隙*/
	EN_CALL_JIEXU_TYPE		call_type			:8;		/*接续类型*/
	unsigned char			phone_len;					/*号码长度<=20*/
	unsigned char			trans_phone[10];			/*被叫号码*/	
}__attribute__ ((packed))ST_HF_MGW_EXT_USER_NUMTRANS_MSG;

typedef struct
{
	EN_MSG_CALL_CTL_TYPE	id					:8;		/*消息标示*/
	unsigned char			msg_len;					/*消息长度*/
	unsigned char			slot;						/*终端标示--时隙*/
	EN_CALL_JIEXU_TYPE		call_type			:8;		/*接续类型*/
	unsigned char			radio_idx;					/*接入战网电台接口编号*/
}__attribute__ ((packed))ST_HF_MGW_EXT_Z_MSG;


typedef struct
{
	EN_MSG_CALL_CFG_TYPE			id			:8;
	unsigned char					msg_len;
	unsigned long					mem_addr;
	unsigned char					op_code;
	unsigned char					callernum[4];
}ST_HF_MGW_EXT_USER_REG_REQ,ST_HF_MGW_EXT_SP_NUM_REQ;

typedef struct
{
	EN_MSG_CALL_CFG_TYPE			id			:8;
	unsigned char					msg_len;
	unsigned char					mem_slot;
	unsigned char					ans_code;
}ST_HF_MGW_EXT_USER_REG_ANS,ST_HF_MGW_EXT_USER_NUM_ANS,ST_HF_MGW_EXT_SP_NUM_ANS;


typedef struct
{
	EN_MSG_CALL_CFG_TYPE			id			:8;
	unsigned char					msg_len;
	unsigned char					op_code;
	unsigned char					mem_count;
	struct{
		unsigned char					mem_slot;
		unsigned char					callernum[10];
		unsigned char					conf;
		unsigned char					prior;
		unsigned char					encoder;
	}__attribute__ ((packed)) terminal[25];
}__attribute__ ((packed)) ST_HF_MGW_EXT_USER_NUM_REQ;

typedef struct
{
	EN_MSG_CALL_CFG_TYPE			id			:8;
	unsigned char					msg_len;
	unsigned long					mem_addr;
	unsigned char					mem_slot;
	unsigned char					op_code;
	unsigned char					conf_index;
	unsigned char					conf_num[4];
	unsigned char					conf_mem_count;
	unsigned char					mem_num[INTMAXMEMCOUNT];
}ST_HF_MGW_EXT_CONF_NUM_REG;


typedef struct
{
	EN_MSG_CALL_CFG_TYPE			id			:8;
	unsigned char					msg_len;
	unsigned long					mem_addr;
	unsigned char					mem_slot;
	unsigned char					op_code;
	unsigned char					conf_index;
	unsigned char					ans_code;
}ST_HF_MGW_EXT_CONF_NUM_ANS;


typedef struct
{
	EN_MSG_CALL_CFG_TYPE			id			:8;
	unsigned char					msg_len;
	unsigned char					mem_slot;
	unsigned char					priority;
	unsigned char					conf_right;
}ST_HF_MGW_EXT_USER_RIGHT_REQ;


typedef	struct
{
	EN_MSG_CALL_CFG_TYPE			id			:8;
	unsigned char					msg_len;
	unsigned char					mem_slot;
	unsigned char					ans_code;
}ST_HF_MGW_EXT_USER_RIGHT_ANS;



typedef struct
{
	unsigned short		sync_head;		/*0x55aa*/
	unsigned char		type;			/*报文类型*/
	unsigned short		reserve;		/*保留*/
	unsigned short		len;			/*正文长度*/
}__attribute__ ((packed)) ST_HF_MGW_EXT_PRO_HEADER;

typedef struct
{
	ST_HF_MGW_EXT_PRO_HEADER	header;
	unsigned char		body[EXTMAXDATASIZE];	/*正文*/
	unsigned short		checksum;				/*校验码*/
}__attribute__ ((packed)) ST_HF_MGW_EXT_PRO;



/*与既设网用户之间的协议*/
enum{
	EN_TYPE_JS_COST_FIBER = 5,
	EN_TYPE_JS_COST_JUNZONG = 20,
	EN_TYPE_JS_COST_WEIBO = 30,
	EN_TYPE_JS_COST_SATELLITIC = 40,
};

enum{
	MGW_JS_HELLO_ID = 0x01,
	MGW_JS_HF_ROUTE_ID,
	MGW_JS_HF_ROUTE_ANS_ID,
	MGW_JS_HF_TEST_REQ_ID = 0x30,
	MGW_JS_HF_TEST_ANS_ID = 0x40,
};


/*路由探测协议*/
typedef struct
{
	unsigned long		hf_addr;
	unsigned long		ip_addr;
	unsigned char		mask;
	unsigned short		cost;
	unsigned char		dep_num[2];
}__attribute__ ((packed)) ST_HF_MGW_JS_HELLO;

typedef struct
{
	unsigned long		hf_addr;
	unsigned short		route_count;
	struct{
		unsigned long	dst_hf_addr;
		unsigned short	cost;
		unsigned char	dep_num[2];
	}__attribute__((packed))route[128];
}__attribute__ ((packed)) ST_HF_MGW_JS_HF_ROUTE_MSG;

typedef struct
{
	unsigned long		hf_addr;
}__attribute__((packed)) ST_HF_MGW_JS_HF_ROUTE_ANS;

typedef struct
{
	unsigned char		protocol;
	unsigned char		id;
	unsigned short		len;
}__attribute__ ((packed)) ST_HF_MGW_JS_PRO_HEADER;

typedef struct
{
	ST_HF_MGW_JS_PRO_HEADER		header;
	unsigned char	body[512];
}__attribute__ ((packed)) ST_HF_MGW_JS_PRO;


typedef struct{
	unsigned char	id;
	unsigned short	checksum;
	unsigned short	seqno;
	unsigned char	data[32];
}__attribute__ ((packed)) ST_HF_MGW_JS_TEST_REQ,ST_HF_MGW_JS_TEST_ANS;


typedef struct{
	unsigned long	dst;
	unsigned long	src;
}__attribute__((packed)) ST_HF_MGW_JS_ADD_XINLING_MSG_HEADER;

typedef struct{
	unsigned short  id;
	unsigned short	len;
	unsigned char	data[32];
}__attribute__((packed)) ST_HF_MGW_FLOW_CONTROL_MSG;

struct iphdr {
	unsigned char	version:4,
  		ihl:4;

	unsigned char	tos;
	unsigned short	tot_len;
	unsigned short	id;
	unsigned short	frag_off;
	unsigned char	ttl;
	unsigned char	protocol;
	unsigned short	check;
	unsigned long	saddr;
	unsigned long	daddr;
};

struct udphdr {
	unsigned short	source;
	unsigned short	dest;
	unsigned short	len;
	unsigned short	check;
};



#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /*_EXTERIOR_PROTOCOL_H_*/

