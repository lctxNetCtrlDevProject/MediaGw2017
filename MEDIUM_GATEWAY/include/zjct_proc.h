/****************************************************************************************************
* Copyright @,  2012-2017,  LCTX Co., Ltd. 
* Filename:     zjct_proc.c  
* Version:      1.00
* Date:         2012-04-20	    
* Description:  Z024便携式指挥通信终端II型指挥通信板与装甲车通话音通信协议处理。
			<20120315_ZJCT-002装甲车通话音通信规范.doc>
* Modification History: 李石兵 2012-04-20 新建文件

*****************************************************************************************************/
//header: zjct_proc.h
#ifndef __ZJCT_PROC_H__
#define __ZJCT_PROC_H__

#if defined(__cplusplus)
extern "C" 
{ 
#endif

/*****************************************************************************************************
 * 宏定义
*****************************************************************************************************/

#define IN 
#define OUT

/* 装甲车通系统内设备数量，II型席位最大32，车通内设备最大13 
(其中12个电台和1个中心控制盒)，预留11个扩展用

排列为:中心控制盒，电台0~12, II 0~31
*/

#undef	ZJCT_DEV_TYPE_MAX
#define ZJCT_DEV_TYPE_MAX 			12 	//12种类型的盒?

#undef 	ZJCT_DEV_INDX_MAX
#define ZJCT_DEV_INDX_MAX 			32	//每种类型盒子最大32。

#undef 	LOCAL_INCMEM_NUM
#define LOCAL_INCMEM_NUM		1   /*本地INC 成员个数*/

#undef 	REMOTE_INCMEM_NUM
#define REMOTE_INCMEM_NUM		10 /*远端INC 成员个数*/


/* 网管和话音通信的udp数据包报文头*/
//规范中规定1467字节，但实现时为节省空间，采用600字节
#undef 	ZJCT_MAX_DATA_LEN_600
#define ZJCT_MAX_DATA_LEN_600			600 

//规范中未明确规定，暂时取最大消息长度采用128 字节
#undef 	ZJCT_MAX_CALLSEM_DATA_LEN
#define ZJCT_MAX_CALLSEM_DATA_LEN		ZJCT_MAX_DATA_LEN_600

#define ZJCT_VOC_G711_20MS_LEN_160  	160

#define ZJCT_INC_VOC_20MS_LEN_166 		(160 + 6)

#define ZJCT_VOC_PCM_20MS_LEN_320  		(2 * ZJCT_VOC_G711_20MS_LEN_160)

//每20ms发送一个328字节的话音包，其中数据320个
#define ZJCT_PKGVOC_LEN_328  			(ZJCT_VOC_PCM_20MS_LEN_320 + 8)  

#define RTP_HEADER_SIZE  				12
#define RTP_RXBUFF_SIZE  				(ZJCT_VOC_G711_20MS_LEN_160 + RTP_HEADER_SIZE)
#define RTP_TXBUFF_SIZE  				(ZJCT_VOC_G711_20MS_LEN_160 + RTP_HEADER_SIZE)
#define RTP_G711_MSG_LEN_172			(ZJCT_VOC_G711_20MS_LEN_160 + RTP_HEADER_SIZE)


/*乘员一号盒状态帧长度*/
#define ZJCT_STATUS_MSG_LEN 			(ZJCT_MNG_MSG_HEADER_LEN + \
										ZJCT_STATUS_MSG_DATA_LEN) /* 34 */
										
	
#define ZJCT_STATUS_MSG_DATA_LEN  		0x1d  		/* 29 */
#define ZJCT_STATUS_MSG_UPDATE_DATA_LEN 0x0d 	/* 13 */

#define RADIO_NUM_PER_DEV 				0x04
#define RADIO_DEV_NUM					0x02
#define RADIO_NUM_MAX					(RADIO_NUM_PER_DEV * RADIO_DEV_NUM)

#define VOICE_TIME_1min		(50 * 2)
#define VOICE_MAX_LEN		(ZJCT_VOC_PCM_20MS_LEN_320 * (VOICE_TIME_1min))
#define VOICE_MEM_MAX		(ZJCT_DEV_INDX_MAX)
#define VOICE_RADIO_MAX		(RADIO_NUM_MAX)

/*
装甲车通设备内部网络信令是在UDP/IP之上的应用层协议，
采用IP/UDP封装，信令和话音采用不同的UDP端口号，
信令UDP端口号0x6600（26112），话音UDP端口号0x6601（26113），
*/
//数据通信用端口，数据通信主要进行系统管理
#define ZJCT_ADAPTER_MNG_PORT			(0x6600)
//话音数据 端口号
#define ZJCT_ADAPTER_VOICE_PORT 		(0x6601)

/*  与INC通信用端口号话音信令端口号*/
#define ZJCT_INC_SEM_PORT				(30000)
/*  与INC通信用端口号话音数据端口号*/
#define ZJCT_INC_VOC_PORT				(30001)

#define ZJCT_CALL_TO_PC 				(45000)
#define ZJCT_TESTVOC_LEN 				960000



#define ZJCT_CALL_CFG_PER_USERINFO_LEN 	16
#define	SIP_MAXNUMLEN				8  /*最大号码数组长度*/

#define ZJCT_PRO_TYPE 				0x01 //类型当前定义为1
#define ZJCT_PRO_VER				0x01  //现版本为1

/* 车通与话音呼叫协议格式中1：参数设置消息，2：话音呼叫消息*/
#define ZJCT_INC_MNG 				0x00 
#define ZJCT_INC_CONFIG 			0x01 
#define ZJCT_INC_VOICE_CMD  		0x02
#define ZJCT_INC_VOICE_DATA      	0x03

#undef	ZJCT_XW_MIN_INDX
#define	ZJCT_XW_MIN_INDX 			1

#undef	ZJCT_XW_MAX_INDX
#define	ZJCT_XW_MAX_INDX 			16


#define PERIOD_4s  					40
#define PERIOD_3S					30
#define PERIOD_15S					150

#define ZXKZH_TYPE_IDX 				0x00
#define CY1HH_TYPE_IDX				0x03
#define CY2HH_TYPE_IDX 				0x04

#define CY1HH_DEV_IDX_MEM8			(0x07)/*(8 -1)*/

#define ZJCT_BROADCASE_ADDR 		0xfeff

#define ZJCT_LOCALDEVID_OFFSET 		0x01
#define ZJCT_MODE_OFFSET 	  		0x03
#define ZJCT_DSTDEVID_OFFSET 	  	0x04

#define ZJCT_LOCALMEM_STACK_CNT		0x05		/*最大保存本地状态个数*/


#define FLG_OK 			1
#define FLG_FAILED 		0

#define GET_NETREG_FLG(id)  		(gsend_register[id])
#define SET_NETREG_FLG(id)  		(gsend_register[id] = FLG_OK)
#define CLR_NETREG_FLG(id)  		(gsend_register[id] = FLG_FAILED)

#define GET_PARAMREQ_FLG(id)  	(grequest_status[id])
#define SET_PARAMREQ_FLG(id)  	(grequest_status[id] = FLG_OK)
#define CLR_PARAMREQ_FLG(id) 		(grequest_status[id] = FLG_FAILED)

#define GET_SOCKTINIT_FLG()  		gsocketinit_status
#define SET_SOCKTINIT_FLG()  		gsocketinit_status = FLG_OK
#define CLR_SOCKTINIT_FLG() 		gsocketinit_status = FLG_FAILED

#define GET_OPENUSERMSG_FLG()  	gopenusermsg_status
#define SET_OPENUSERMSG_FLG()  	gopenusermsg_status = FLG_OK
#define CLR_OPENUSERMSG_FLG()  	gopenusermsg_status = FLG_FAILED

#define GET_USERNUMREG_FLG()		gusernum_status
#define SET_USERNUMREG_FLG()		gusernum_status = FLG_OK
#define CLR_USERNUMREG_FLG()		gusernum_status = FLG_FAILED

#define ADAPTER_INC_ADDR 	0xc0a80800  /*192.168.8.0*/
#define ADAPTER_ADDR 		0xc0a80b00	/*192.168.11.0*/


#define	PTT_DOWN	0x00//PTT按键值
#define	PTT_UP		0x01//PTT按键值



/*****************************************************************************************************
 * 结构体定义
*****************************************************************************************************/
/* RTP header */
struct TRTPHeader
{	
	unsigned       RTPVer    :2; /*RTP version (2)*/	
	unsigned       RTPPadding:1; /*RTP padding (0)*/
	unsigned       RTPExt    :1; /*RTP extension (0)*/
	unsigned       RTPCSRCCnt:4; /*RTP CSRC count (0)*/  
	unsigned       RTPMarker :1; /*RTP marker (0)*/
	unsigned       RTPPT     :7; /*RTP payload type*/	
	unsigned       RTPSeqNum :16;    /*RTP sequence number*/
	unsigned       RTPTimeStamp:32; /*RPT Time stamp*/
	unsigned       RTPSSRC  :32;   /*RTP synchronization source identifier*/
};

enum __PANEL_STATUS__
{
	PANEL_OFFLINE = 0x00,
	PANEL_ONLINE 
};

typedef enum fmq_control {
    FMQ_CTRL_OFF = 0x0,
    FMQ_CTRL_SLOW = 0x1,
    FMQ_CTRL_FAST = 0x2,
	FMQ_CTRL_BUTT
}EN_FMQ_CTRL;

/* 装甲车通内部网络信令帧结构信令类型定义*/
typedef enum en_zjct_mng_type
{
	ZJCT_MNG_TYPE_PARAMREQ = 0x60,
	ZJCT_MNG_TYPE_PARAMCFG = 0x61,
	ZJCT_MNG_TYPE_PARAMCFGACK = 0x62,

	ZJCT_MNG_TYPE_REPORTSTATUS = 0x63,
	ZJCT_MNG_TYPE_REPORTSTATUSACK = 0x63,
	
	ZJCT_MNG_TYPE_PARAMUPADATE = 0x64,
	ZJCT_MNG_TYPE_PARAMUPADATEACK = 0x65,

	ZJCT_MNG_TYPE_QUERYSTATUS = 0x66, //??

	ZJCT_MNG_TYPE_REGREQ = 0x6f,
	ZJCT_MNG_TYPE_REGREQACK = 0x6f,
	
	ZJCT_MNG_TYPE_PCM64K = 0x89,//64k PCM话音
	ZJCT_MNG_TYPE_PCM128K = 0x8b,//128k PCM话音
 
	ZJCT_MNG_TYPE_CONFIG = 0x90,//参数设置
	ZJCT_MNG_TYPE_CONFIG_ACK = 0x91,//参数设置应答

	ZJCT_MNG_TYPE_DEV_STATUS_INFO = 0x92,//设备状态信息

	ZJCT_MNG_TYPE_DT_CONTROL = 0x93,//电台控制
	ZJCT_MNG_TYPE_DT_CONTROL_ACK = 0x94,//电台控制应答
/*
	ZJCT_SEAT_TXB_CMD = 0x98
*/
	SEM_TYPE_BUTT 
}EN_ZJCT_MNG_TYPE;

#define ZJCT_SEAT_TXB_CMD  ZJCT_MNG_TYPE_DEV_STATUS_INFO

/* 装甲车通设备地址定义*/
typedef enum en_zjct_dev_addr
{
	ZJCT_DEV_ADDR_ZHZXH = 0xF0,	//指挥型中心控制盒
	ZJCT_DEV_ADDR_ZDZXH = 0xF0,	//战斗型中心控制盒
 
	ZJCT_DEV_ADDR_DTJKH = 0xF1,	//电台接口设备
	ZJCT_DEV_ADDR_CY1H = 0xF4,	//乘员1号盒(便携式指挥通信终端II型使用的地址)
	ZJCT_DEV_ADDR_CY2H = 0xF5,	//乘员2号盒

	ZJCT_DEV_ADDR_YHJKH = 0xF6,	//用户接口盒
	ZJCT_DEV_ADDR_YHKZH = 0xF7,	//用户扩展盒
	ZJCT_DEV_ADDR_ZY1H = 0xF8,	//载员1号盒

	ZJCT_USER_ADDR = 0xFE,
	ZJCT_DEV_ADDR_BRDCAST = 0xFF,

	ZJCT_DEV_ADDR_BUTT
}EN_ZJCT_DEV_ADDR;

/*
序	工作方式	编码	工作目的	发起方
1	强呼发送	0x01	0xFE FF（乘员广播）	乘员终端
2	强呼接收	0x02	同上	乘员终端
3	车内广播	0x03	同上	乘员终端
4	车通组呼	0x04	0xF4 01～0A（乘员1～10，车通组呼发起者地址编号）	乘员1号盒
5	单呼			0x05	0xF4 01～0A（乘员1～10）	乘员1号盒
6	控制电台发送	0x06	0xF1 01～0B（电台1~12）	乘员终端
7	控制电台接收	0x07	同上	乘员终端
8	共听发送	0x08	0xF1 01～0B（电台1~12）	乘员终端
9	共听接收	0x09	同上	乘员终端
10	拨号/接听发送	0x0A	0xF0（中心控制盒I型）	乘员1号盒
11	拨号/接听接收	0x0B	同上	乘员1号盒
*/

typedef enum _zjct_work_mode_type
{
	ZJCT_MODE_NULL = 0x00,
	ZJCT_MODE_QHFS = 0x01,
	ZJCT_MODE_QHJS = 0x02,
	ZJCT_MODE_CNGB = 0x03,
	ZJCT_MODE_CTZH = 0x04,
	ZJCT_MODE_DH = 0x05,
	ZJCT_MODE_KZDTFS = 0x06,
	ZJCT_MODE_KZDTJS = 0x07,
	ZJCT_MODE_GTFS = 0x08,
	ZJCT_MODE_GTJS = 0x09,
	ZJCT_MODE_BHFS = 0x0A,
	ZJCT_MODE_BHJS = 0x0B,
	ZJCT_MODE_DTFS = 0x0C,
	ZJCT_MODE_DTJS = 0x0D,
	ZJCT_MODE_DTSDZXFS = 0x0E,
	ZJCT_MODE_DTSDZXJS = 0x0F,
	ZJCT_MODE_DTZDZXFS = 0x10,
	ZJCT_MODE_DTZDZXJS = 0x11,
	ZJCT_MODE_DTZDZX   = 0x12,
	ZJCT_MODE_SETTING  = 0x14,
	ZJCT_MODE_INQUIRE_MEM  = 0x15,
	ZJCT_MODE_HANDLE   = 0x16,
	ZJCT_MODE_WORKMODE = 0x17,
	ZJCT_MODE_INQUIRE_RADIO = 0x18,
	ZJCT_MODE_TERMINAL_CLASH = 0x19,

	ZJCT_MODE_BUTT, 
}ZJCT_WORKMODE_EN;

/* 
  装甲车通呼叫过程参数配置类型定义
*/
typedef enum _zjct_call_config_type
{
	ZJCT_CALL_CFG_USEROPEN_REQ = 0x01, 	//5.1.2	开通消息

	ZJCT_CALL_CFG_USERINFO_INJECT,		//5.1.3	用户信息注入消息
	ZJCT_CALL_CFG_USERINFO_INJECT_ACK,		

	ZJCT_CALL_CFG_USERPHONENUM_REGISTER_REQ,	//5.1.4	用户号码注册消息
	ZJCT_CALL_CFG_USERPHONENUM_REGISTER_ACK,	

	ZJCT_CALL_CFG_USERPHONENUM_UNREGISTER_REQ,	//5.1.5	用户号码注销消息
	ZJCT_CALL_CFG_USERPHONENUM_UNREGISTER_ACK,
	
	ZJCT_CALL_CFG_BUTT
}ZJCT_CALL_CONFIG_TYPE;

/* 
  装甲车通呼叫消息类型定义
*/
typedef enum _zjct_call_sem_type
{
	ZJCT_CALL_REQ = 0x01, 	//5.2.1	呼叫请求
	ZJCT_CALL_ACK,			//5.2.2	呼叫应答
	ZJCT_CONNECT_REQ,		//5.2.3	连接请求		
	ZJCT_RELEASE_REQ,		//5.2.4	释放请求
	ZJCT_RELEASE_ACK,		//5.2.5	释放证实
	
	ZJCT_PTT_SEM = 0x0a,//5.2.6	PTT按键消息
	
	ZJCT_CALL_SEM_BUTT
}ZJCT_CALL_SEM_TYPE;

typedef enum _zjct_inc_connect_status_type
{
	ZJCT_INC_DISCONNECT = 0x00,
	ZJCT_INC_CONNECT 	= 0x01,
}ZJCT_INC_CONNECT_STATUS_TYPE;

typedef enum _zjct_connect_type
{
	ZJCT_CHOOSE_CALL = 0x01,
	ZJCT_YYQSGHY_CALL = 0x02,
	ZJCT_SJQSGHY_CALL = 0x03,
	ZJCT_GROUNP_CALL = 0x04,
	ZJCT_RADIO_CALL = 0x05
}ZJCT_CONNECT_TYPE;

typedef enum _zjct_release_reason
{
	ZJCT_NORMAL_RELEASE = 0x01,
	ZJCT_DEVICE_ERROR = 0x02,
	ZJCT_TIMEOUT_RELEASE = 0x03
}ZJCT_RELEASE_REASON;

typedef enum _zjct_encode_type
{
	ZJCT_ENCODE_G729 = 0x01,
	ZJCT_ENCODE_CVSD = 0x02,
	ZJCT_ENCODE_G729_CVSD = 0x03,
	ZJCT_ENCODE_PCM = 0X04
}ZJCT_ENCODE_TYPE;


/*
	装甲车通参数设置定义
*/
typedef enum _zjct_param_config_type
{
	ZJCT_SEND_MSG_CONTROL 		= 0x03,		//新增、广播
	ZJCT_LOWER_MEM_LISTEN_RANGE = 0x04,		//新增、单播
	ZJCT_SUBNET_IP_ADDR			= 0x05,		//新增、单播
	ZJCT_GROUPCALLMEM_START		= 0x06,		//新增、广播
	ZJCT_GROUPCALLMEM_END		= 0x07,		//新增、广播
	ZJCT_LOWER_M_VALUE			= 0x08,		//新增、单播
	ZJCT_LOWER_MEM_SENDTO		= 0x09,		//新增、单播
	ZJCT_HIGH_PERMISSION		= 0x0a,		//新增、单播
	ZJCT_LOWER_PARAM_INQUIRE	= 0x0b		//新增、单播
}ZJCT_PARAM_CONFIG_TYPE;

/*
	"车通发信控制"设置类型中设置参数定义
*/
typedef enum _zjct_send_msg_control
{
	ZJCT_SEND_MSG_CTL_RECEIVE 	= 0x01,
	ZJCT_SEND_MSG_CTL_LEADER	= 0x02,
	ZJCT_SEND_MSG_CTL_ALL		= 0x03
}ZJCT_SEND_MSG_CTL;

/*
	"低级乘员共听收听范围"设置类型中设置参数定义
*/
#if 0
typedef struct _zjct_lower_memlisten
{
	uint8 lowermemlisten[12];	
}__attribute__((packed))ZJCT_LOWER_MEMLISEN;
#endif
typedef enum _zjct_lower_mem_listen_range
{
	RADIO_NUM1 = 0x01,
	RADIO_NUM2 = 0x02,
	RADIO_NUM3 = 0x03,
	RADIO_NUM4 = 0x04,
	RADIO_NUM5 = 0x05,
	RADIO_NUM6 = 0x06,
	RADIO_NUM7 = 0x07,
	RADIO_NUM8 = 0x08,
	RADIO_NUM9 = 0x09,
	RADIO_NUM10 = 0x0a,
	RADIO_NUM11 = 0x0b,
	RADIO_NUM12 = 0x0c	
}ZJCT_LOWER_MEMLISEN;


/*
"子网IP地址"设置类型中设置参数定义
*/
typedef struct _zjct_subnet_ipaddr
{
	uint32	ipaddr;
	uint8	netmask_bit;
}__attribute__((packed)) ZJCT_SUBIPADDR;

/*
"组呼开通"设置类型中设置参数定义
*/
typedef enum _zjct_groupcallmem_start
{
	GROUPCALL_MEM1 = 0x01,
	GROUPCALL_MEM2 = 0x02,
	GROUPCALL_MEM3 = 0x03,
	GROUPCALL_MEM4 = 0x04,
	GROUPCALL_MEM5 = 0x05,
	GROUPCALL_MEM6 = 0x06,
	GROUPCALL_MEM7 = 0x07,
	GROUPCALL_MEM8 = 0x08,
	GROUPCALL_MEM9 = 0x09,
	GROUPCALL_MEM10 = 0x0a,
	GROUPCALL_MEM11 = 0x0b,
	GROUPCALL_MEM12 = 0x0c,
	
}ZJCT_GROUPCALLSTART;

/*
"高级权限"设置类型中设置参数定义如下
*/
typedef enum _zjct_high_permission_type
{
		HIGH_PERMISSION_SINGLE_CALL	= 0x01,
		HIGH_PERMISSION_GROUP_CALL	= 0x02,
		HIGH_PERMISSION_TRANSMITE	= 0x03,
		HIGH_PERMISSION_SETTING		= 0X04
}ZJCT_HIGH_PERMISSION_TYPE;

typedef enum _zjct_permission_status
{
	PERMISSION_FORBID	= 0x00,
	PERMISSION_ALLOW	= 0x01
}ZJCT_PERMISSION_STATUS;

/*
"低级乘员参数查询"设置类型中设置参数定义
*/
typedef enum _zjct_lower_param_inquire_type
{
	M_VAULE_INQUIRE	= 0x01,
	GROUP_LISTEN_RANGE_IRQUIRE	= 0x02,
	GROUP_LISTEN_SENDTO_IRQUIRE	= 0x03
}ZJCT_PARAMTYPE;

typedef enum _file_status
{
	FILE_CLOSE = 0,
	FILE_OPEN    = 1
}FILE_STATUS;

/*****************************************
 *  装甲车通内部用户通信消息定义
*****************************************/



/*****************************************
 *  装甲车通与INC通信消息定义
*****************************************/


/* 装甲车通设备内部网络信令头部封装形式如下*/
typedef struct _zjct_mng_header
{
	uint8 ucType;//信令类型是车通内部网络信令的首部标识，长度1字节
	uint8 ucDstAddr; //目的设备地址是车通内部网络信令的目的通信终端的逻辑设备地址，长度1字节
	uint8 ucSrcAddr; //源设备地址是车通内部网络信令的源通信终端的逻辑设备地址，长度1字节。
	uint16 usLen;//信令长度是车通内部网络信令的数据部分的长度，长度2字节，取值范围0～1472
}__attribute__((packed)) ST_ZJCT_MNG_HEADER;

#define ZJCT_MNG_MSG_HEADER_LEN sizeof(ST_ZJCT_MNG_HEADER)

typedef struct _zjct_radio_mem_
{
	uint8 	dev_srcId;
	uint8	dev_prio;
	uint8	dev_mode;
	uint16	dev_dscId;
}__attribute__((packed))ZJCT_RADIO_MEM;

typedef struct _zjct_radio_status_msg
{
	ST_ZJCT_MNG_HEADER stHead;	//信令头部
	uint8 cfg_status;
	ZJCT_RADIO_MEM radio_mem[RADIO_NUM_PER_DEV];
	uint16 	hw_ver;
	uint32	sw_ver;
}__attribute__((packed))ZJCT_RADIO_STATUS_MSG;

typedef struct _zjct_radio_status_
{
	ZJCT_RADIO_MEM mem_info;
	uint32 	time_cnt;
	uint8 	online;
	uint8	radio_save;
	uint8	radio_get;
	uint16	hw_ver;
	uint32	sw_ver;
	uint32	ip_addr;
	uint8	net_mask;
	int8 		chan_id;
	uint8 	radio_voc_flg;
	uint8 	radio_voc[VOICE_TIME_1min][ZJCT_VOC_PCM_20MS_LEN_320];  /* remote radio voc data*/	
}__attribute__((packed))ZJCT_RADIO_STATUS;

/* 装甲车通设备内部网络网管消息完整定义*/
typedef struct _zjct_mng_msg
{
	ST_ZJCT_MNG_HEADER stHead;	//信令头部
	uint8 ucData[ZJCT_MAX_DATA_LEN_600];//信令内容,当前最大支持600字节
}__attribute__((packed)) ST_ZJCT_MNG_MSG;

#define MAX_MNG_MSG_LEN	sizeof(ST_ZJCT_MNG_MSG)


typedef struct _BOARD_SEM_HEAD
{
	uint8 ProType;		//协议类型 1
	uint8 Edition;		//版本 1
	uint8 SemType;		//消息类型 0~2
	uint16 Len;
}__attribute__((packed)) ST_BOARD_SEM_HEAD;
typedef struct _BOARD_SEM
{	
	ST_BOARD_SEM_HEAD Head;
	uint8 Data[ZJCT_MAX_DATA_LEN_600];	
}__attribute__((packed)) ST_BOARD_SEM;


typedef struct _PHONE_SEM_HEAD
{
	uint8 ProType;		//协议类型 1
	uint8 SemType;		//消息类型 0~2
	uint8 Len;
}__attribute__((packed)) ST_PHONE_SEM_HEAD;
typedef struct _PHONE_SEM
{	
	ST_PHONE_SEM_HEAD Head;
	uint8 Data[ZJCT_MAX_DATA_LEN_600];	
}__attribute__((packed)) ST_PHONE_SEM;

/*****************************************
 * 装甲车通话音通信消息定义
*****************************************/
/* 车通与话音呼叫协议消息头*/
typedef struct _zjct_call_sem_header
{
	uint8 ucProType;	//协议类型 1
	uint8 ucProVer;		//表示协议的版本号，便于协议将来的修改和扩展，现版本为1
	uint8 ucSemType;	//1：参数设置消息，2：话音呼叫消息
	uint16 usLen;		//表示正文的长度，长度为2个字节
}__attribute__((packed)) ST_ZJCT_CALL_SEM_HEADER;

#define ZJCT_CALL_SEM_MSG_HEADER_LEN sizeof(ST_ZJCT_CALL_SEM_HEADER)

/* 装甲车通设备话音呼叫信令消息内容*/
#define ZJCT_SUBDATA_HEADLEN 	2  //消息子类型和消息内容数据长度

typedef struct _zjct_sem_msg_content_
{
	uint8  subType;
	uint8  subLen;
	uint8  subData[ZJCT_MAX_CALLSEM_DATA_LEN];
}__attribute__((packed)) ST_ZJCT_SEM_MSG_CONTENT;

/* 装甲车通设备话音呼叫信令消息完整定义*/
typedef struct _zjct_call_sem_msg
{	
	ST_ZJCT_CALL_SEM_HEADER Head;
	//uint8 Data[ZJCT_SUBDATA_HEADLEN + ZJCT_MAX_CALLSEM_DATA_LEN]; //130字节
	ST_ZJCT_SEM_MSG_CONTENT MsgData;
}__attribute__((packed)) ST_CALL_SEM_MSG;

typedef struct _zjct_inccall_voc_msg
{
	ST_ZJCT_CALL_SEM_HEADER Head;
	uint8 inc_vocdata[ZJCT_MAX_CALLSEM_DATA_LEN];
}__attribute__((packed))ZJCT_INCCALL_VOC_MSG;
/* 设备状态更新帧*/
typedef struct _zjct_status_frame
{
	uint8	cfg_status;
	uint8 	dev_srcId;
	uint8	dev_prio;
	uint8	dev_mode;
	uint16	dev_dscId;
	uint16 	hw_ver;
	uint32	sw_ver;
	uint8	user_num[17];
}__attribute__((packed))ZJCT_STATUS_FRAME_DATA;
 /* 装甲车通成员*/
typedef struct _zjct_mem
{
	ZJCT_STATUS_FRAME_DATA data;
	uint32	Ipaddr;				/* network ip addr */
	uint8 	Netmask;			/* network netmask */
	uint8 	online;				/* online flag */
//	uint8 	used;				/* online flag */
	uint32  time_cnt;			/* save the time every 10ms */
//	uint8 	user_name_flg;          /* user phone number register flag */
//	uint8 	user_name_length;	/* user phone number length */
//	uint8	user_name[10];		/* user phone number */
	uint8	user_password[10];	
	uint8 	connect; 			//单呼
	uint8 	callIn;				//单呼
	uint8 	callIn_id;			//单呼	
	uint8   active_release;	//单呼
	uint8 	force_call_srcId;
	uint8 	radio_ctl_status;	/*radio control*/
	uint8 	radio_gtctl_status;	/*radio gongting control*/
	uint8 	inc_connect;		/*network control device connect status*/
	uint8 	inc_callIn;	
	uint8	group_call_status;
	uint8	group_call_id;
	uint8	mem_save;			/* voice save */
	uint8	mem_get;			/* voice get */
	uint8 	mem_chan_id;			/* ac491 channel id ,0xFF show no chan*/
//	uint8 	user_phone_length;	
//	uint8 	user_phone_num[10];
	uint8 	mem_voc_flg;
	uint8 	mem_voc[VOICE_TIME_1min][ZJCT_VOC_PCM_20MS_LEN_320];  /* remote member voc data*/

	uint8 	glocaldev_num;
}__attribute__((packed))ZJCT_MEM;

typedef enum _radio_ctl_status
{
	RADIO_CTL_DISCONNECT = 0x00,
	RADIO_CTL_CONNECT = 0x01,
}RADIO_CTL_STATUS;

 typedef enum  _work_mode{
	SEAT_MODE = 0,
	CY1HH_MODE = 1,	
}WORK_MODE;

 typedef enum  _ac491work_mode{
	AC491_STD_MODE = 0,
	AC491_ADD_MODE = 1,	
}AC491WORK_MODE;

 typedef struct _BOARD_INC_MEM_INFO
{
	uint8 mem_num;
	uint8 msgack_status;
	uint8 pnum_len;
	uint8 user_num[10];
	uint8 omeeting;
	uint8 prio;
	uint8 encode_type;
	uint8 timeslot;
	uint8 online;
	uint8 usernumreq_status;
	uint8 usernumunreq_status;
}__attribute__((packed)) ZJCT_INC_MEM_INFO;

#undef 	ZJCT_MAX_SEMDATLEN
#define ZJCT_MAX_SEMDATLEN	600
typedef struct _zjct_seat_status_msg
{	
	ST_ZJCT_MNG_HEADER Head;
	uint8 param_cfg;
	uint8 srcId;
	uint8 src_prio;
	uint8 src_mode;
	uint8 desId;
	uint8 callIn;
}__attribute__((packed)) ZJCT_SEAT_STATUS_MSG;

typedef struct _ETH_PORT_
{
	uint32 ipaddr;
	uint32 netmask;
	uint16 seat_sem_port;
	uint16 seat_voc_port;
	uint16 seat_dbg_port;
	uint16 seat_data_port;
	uint16 seat_mng_port;
	uint16 zjct_adptmng_port;
	uint16 zjct_adptvoc_port;
	uint16 zjct_incsem_port;
	uint16 zjct_incvoc_port;
}__attribute__((packed)) ETH_PORT_DES;

#define ZJCT_SEAT_STATUS_LEN sizeof(ZJCT_SEAT_STATUS_MSG)
#define ZJCT_SEAT_STATUS_DATALEN 7

typedef enum _zjct_cmd_type
{
	ZJCT_SEAT_CONFIG = 0x00,
	ZJCT_SEAT_VOICE = 0x01
}ZJCT_CMD_TYPE;

typedef enum _zjct_danhu_cmd_
{
	DANHU_CALLIN = 0x01,				/*被叫呼入*/
	DANHU_ANSWER = 0x02,				/*被叫接听*/
	DANHU_CALLOUT =0x03,				/*主叫呼出*/
	DANHU_CONNECT = 0x04,				/*单呼连接*/
	DANHU_DISCONNECT_PASSIVE = 0x05,	/*被动挂机*/
	DANHU_DISCONNECT_PASSIVE_ACK = 0x06,/*被动挂机应答*/
	DANHU_DISCONNECT_ACTIVE = 0x07,		/*主动挂机*/
	DANHU_TIMEOUT = 0x08,				/*主动呼叫超时*/
	DANHU_BUSY = 0x09,					/*主叫忙*/
	DANHU_CALLIN_REFUSE = 0x0A,			/*被叫拒绝*/

}ZJCT_DANHU_CMD;

#define	DANHU_MSG_BYTE	 0x0d

typedef enum _zjct_forcecall_cmd
{
	FORCECALLOUT = 0x01,
	CANCEL_FORCECALLOUT = 0x02,
	FORCECALLIN = 0x03,
	FORCECALLIN_END = 0x04
}ZJCT_FORCECALL_CMD;

typedef enum _zjct_groupcall_cmd
{
	GROUPCALLOUT = 0x01,
	CANCEL_GROUPCALLOUT = 0x02,
	GROUPCALLIN = 0x03,
	CANCEL_GROUPCALLIN = 0x04,
	GROUPCALL_HANDUP = 0x05,
	
}ZJCT_GROUPCALL_CMD;

typedef enum _zjct_setting
{

	ADDR_SETTING = 0x01,
		
}ZJCT_SETTING;

typedef enum _zjct_handle
{
	HANDLE_INIT  = 0x01,
	HANDLE_PC = 0x02
}ZJCT_HANDLE;

typedef enum _zjct_inquire_terminal
{
	INQUIRE_ALL_TERMINAL = 0x01,
	INQUIRE_ALL_TERMINAL_ACK = 0x02,
	INQUIRE_ONE_TERMINAL = 0x03,
	INQUIRE_ONE_TERMINAL_ACK = 0x04
}ZJCT_INQUIRE_TERMINAL;

typedef enum _zjct_inquire_radio
{
	INQUIRE_ALL_RADIO = 0x01,
	INQUIRE_ALL_RADIO_ACK = 0x02,
	INQUIRE_ONE_RADIO = 0x03,
	INQUIRE_ONE_RADIO_ACK = 0x04
}ZJCT_INQUIRE_RADIO;

/* 电台转信*/
enum __SEAT_DTZX_MODULE_
{
	SEAT_RADIO_ADDR = 0x01,
	SEAT_STOP,
	SEAT_A_TO_B,
	SEAT_B_TO_A,
	SEAT_AB_AUTO
};
enum _DTZX_MODULE_
{
	DTZX_START = 0x01,
	DTZX_STOP = 0x02
};
enum _DTZX_CTL_TYPE_
{
	DTZX_HANDLE = 0x01,
	DTZX_AUTO,
	DTZX_INTERFACE,
};

enum __DHCP_STATUS__
{
	DHCP_FAILED = 0x01,
	DHCP_OK,
};

enum __DHCP_CFG__
{
	DHCP_NOCFG = 0x00,
	DHCP_CFG,
};

enum __CHAN_CFG__
{
	CHAN_NOCFG = 0x00,
	CHAN_CFG,
};

enum __AC491_CHAN_STATUS__
{
	CHAN_FREE = 0x00,
	CHAN_USING,
};

enum __AC491_VOC_SENDTO__
{
	ZJCT_SENDTO_NULL	= 0x00,
	ZJCT_SENDTO_ADAPTER,				
	ZJCT_SENDTO_INC,						
	ZJCT_SENDTO_RADIO,					
	ZJCT_SENDTO_GROUP,					
};


/*****************************************************************************************************
 * 回调函数声明
*****************************************************************************************************/

/*****************************************************************************************************
 * 内部函数声明
*****************************************************************************************************/


/*****************************************************************************************************
 * 导出函数声明
*****************************************************************************************************/
extern int32 Zjct_init(void);
extern int32 Zjct_Dbg_func_for_710(void);
extern int32 Zjct_Ac491Voc_To_MainBoard(uint16 btimeslot, uint8 *p, uint32 len);
extern int32 Zjct_AdptMng_AutoSendStatusMsg(uint8 id);
extern int32 Zjct_AdptMng_SendRegisterNetwork_Req(uint8 id);
extern int32 Zjct_AdptMng_SendParamConfig_Req(uint8 id);
extern int32 Zjct_AdptMng_Update_MemList_OffLine_Msg(void);
extern int32 Zjct_IncCall_SendOpenUserMsg(void);
extern int32 Zjct_IncCall_Register_UserPhoneNumber(uint8 *buf, uint32 len);
extern int32 Zjct_AdptMng_AutoSend_Open_GroupCall(void);
extern int32 Zjct_Handle_Status_Check(void);
extern int32 Zjct_Handle_Status_Frame_SendTo_PC(void);
extern int32 Zjct_DanHu_Status_TimeOut_Check(void);

extern void Zjct_SeatMng_RxThread(void);
extern void Zjct_AdptMng_RxThread(void);
extern void Zjct_AdptVoc_RxThread(void);
extern void Zjct_SeatCall_RxThread(void);
extern void Zjct_SeatData_RxThread(void);
extern void Zjct_IncCall_RxThread(void);
extern void Zjct_IncVoc_RxThread(void);
extern int32 Zjct_Ptt_Process(uint8 flag, uint8 id);
extern int32 Zjct_SeatMng_Socket_init(void);
extern int32 Zjct_AdptMng_Update_Radio_OffLine_Msg(void);
extern int32 Zjct_AdptVoc_SendTo_AC491_Save(void);
extern int32 Zjct_SeatMng_Socket_Send(uint8 *buf, uint32 len);
extern int32 Zjct_Socket_Close(void);
extern int32 Zjct_AdptVoc_Msg_Process_Add(uint8 *buf, uint32 len);
extern int32 Zjct_AdptMng_Update_Mem2List_OnLine_Msg(uint8 *buf, uint32 len, uint32 Ipaddr );
extern int32 Zjct_IncCall_Socket_Send(uint8 *buf, uint32 len, uint8 id);
extern int32 Zjct_IncVoc_Socket_Send(uint8 *buf, uint32 len, uint8 id);
extern int32 BoardSem_CallAck_busy(uint8 *buf, int32 len, uint8 id);
extern int32 BoardSem_Call_HandUp(uint8 *buf, int32 len, uint8 id);
extern int32 Zjct_AdptVoc_Socket_Send(uint8 *buf, uint32 len);
extern int32 Zjct_Ptt_Send_Inc(uint8 ptt_flg, uint8 id);

extern int Zjct_Mng_Thread(const void *data);

extern void Zjct_AdptMng_0_RxThread(void);
extern void Zjct_AdptMng_1_RxThread(void);
extern void Zjct_AdptMng_2_RxThread(void);
extern void Zjct_AdptMng_3_RxThread(void);
extern void Zjct_AdptMng_4_RxThread(void);
extern void Zjct_AdptMng_5_RxThread(void);
extern void Zjct_AdptMng_6_RxThread(void);
extern void Zjct_AdptMng_7_RxThread(void);
extern void Zjct_AdptMng_8_RxThread(void);
extern void Zjct_AdptMng_9_RxThread(void);
extern void Zjct_AdptMng_10_RxThread(void);
extern void Zjct_AdptMng_11_RxThread(void);


extern void Zjct_IncCall_0_RxThread(void);
extern void Zjct_IncVoc_0_RxThread(void);
extern void Zjct_IncCall_1_RxThread(void);
extern void Zjct_IncVoc_1_RxThread(void);
extern void Zjct_IncCall_2_RxThread(void);
extern void Zjct_IncVoc_2_RxThread(void);
extern void Zjct_IncCall_3_RxThread(void);
extern void Zjct_IncVoc_3_RxThread(void);
extern void Zjct_IncCall_4_RxThread(void);
extern void Zjct_IncVoc_4_RxThread(void);
extern void Zjct_IncCall_5_RxThread(void);
extern void Zjct_IncVoc_5_RxThread(void);
extern void Zjct_IncCall_6_RxThread(void);
extern void Zjct_IncVoc_6_RxThread(void);
extern void Zjct_IncCall_7_RxThread(void);
extern void Zjct_IncVoc_7_RxThread(void);
extern void Zjct_IncCall_8_RxThread(void);
extern void Zjct_IncVoc_8_RxThread(void);
extern void Zjct_IncCall_9_RxThread(void);
extern void Zjct_IncVoc_9_RxThread(void);
extern void Zjct_IncCall_10_RxThread(void);
extern void Zjct_IncVoc_10_RxThread(void);
extern void Zjct_IncCall_11_RxThread(void);
extern void Zjct_IncVoc_11_RxThread(void);


extern uint8 gsend_register[CHETONG_SEAT_MAX];
extern uint8 gnetwork_config ;
extern uint8 grequest_status[CHETONG_SEAT_MAX];
extern uint8 gopenusermsg_status;
extern uint8 gsocketinit_status;
extern uint8 gusernum_status;
extern uint32 gdanhu_timeout_flg;
extern uint8 gauto_send_cfg_flg;
extern uint8 panel_status;
extern ETH_PORT_DES II_Adpt_local[CHETONG_SEAT_MAX];
extern ETH_PORT_DES II_Adpt_remote;

extern ETH_PORT_DES II_Inc_local[CHETONG_SEAT_MAX];
extern ETH_PORT_DES II_Inc_remote;

#if defined(__cplusplus) 
} 
#endif 

#endif // __ZJCT_PROC_H__

