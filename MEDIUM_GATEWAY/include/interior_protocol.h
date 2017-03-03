#ifndef	_INTERIOR_PROTOCOL_H_
#define	_INTERIOR_PROTOCOL_H_
#include "common.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"
{
#endif

/*============================ý�����ذ�<->���ݽ�����==================================*/
/* Э��μ�<��¼6����ͨ�ŷ�ϵͳͨ����������豸�������ͨ�Žӿ�Э��> */
#define	INTMAXDSTCOUNT	62
#define	INTMAXDATASIZE	600


enum
{
	BAOWEN_TYPE_MGW_DCU = 0x90,
};

enum{
	BAOWEN_ADDR_TYPE_834_BOARD = 0xB0,
	BAOWEN_ADDR_TYPE_834_DIS_BOARD = 0xD0,
	BAOWEN_ADDR_TYPE_834_PC = 0x90,
	BAOWEN_ADDR_TYPE_834_SNMP_AGENT = 0xA0,		/*2017.02.14, send to 834 snmp agent by Andy-wei.hou*/
	BAOWEN_ADDR_TYPE_716_BOARD = 0x80,
	BAOWEN_ADDR_TYPE_716_RAY_BOARD = 0xE0,
	BAOWEN_ADDR_TYPE_50_BOARD = 0x10,
	/*Added by lishibig 20151011,50��ר�������ּ���ģ�飬��ͬģ���ַ��ͬ */
	BAOWEN_ADDR_TYPE_50_WIRELESS = 0x20,
	BAOWEN_ADDR_TYPE_50_WIRE = 0x40,
	BAOWEN_ADDR_TYPE_50_MAIN = 0x50,
	BAOWEN_ADDR_TYPE_30_PC	 = 0x30,		/*30suo PC Mng*/
};

#define Z043_PROTOCAL_TYPE 0x01

enum{
	BAOWEN_MSG_TYPE_CMD = 0x01,
};

enum{
	RAY_MSG_TYPE_SET = 0xd1,
};


enum
{
	INT_FRAME_TYPE_DATA = 0x00,	/*����֡*/
	INT_FRAME_TYPE_CMD_CFG,		/*����֡---��Ԫ����Ϣ�ϱ�*/
	INT_FRAME_TYPE_CMD_ROUTE,	/*����֡---ý�����ذ��ṩ��·����Ϣ*/
	INT_FRAME_TYPE_CMD_XINLING,	/*����֡---ý�����ذ�͸��ǿ��ǿ������*/
};

enum
{
	MGW_DCU_ADDR_ID = 0x01,
	MGW_DCU_LINK_ID,
	MGW_DCU_RADIO_FLOW_CTRL_ID = 0x10,
	MGW_DCU_RADIO_FLOW_CTRL1_ID = 0x11,
	
	MGW_DCU_ROUTE_ID = 0x01,
	MGW_DCU_ROUTE_ANS_ID,
	MGW_DCU_ROUTE_FIND_ID,
	MGW_DCU_ADD_IP_ROUTE_ID,
	MGW_DCU_ADD_IP_ROUTE_ANS_ID,
	MGW_DCU_DEL_IP_ROUTE_ID,
	MGW_DCU_DEL_IP_ROUTE_ANS_ID,
	MGW_DCU_HF_ROUTE_QUERY_ID,
	MGW_DCU_HF_ROUTE_QUERY_ANS_ID,
};

enum{
	EN_TYPE_ROUTE_ANS_SUCCESS = 0x00,
	EN_TYPE_ROUTE_ANS_FAILURE = 0xff,
};


/*��ַ��Ϣ*/
typedef struct
{
	unsigned char		id;
	unsigned short		len;
	unsigned long		hf_addr;
}__attribute__ ((packed)) ST_HF_MGW_DCU_ADDR_MSG;

/*���������Ϣ*/
typedef struct
{
	unsigned char		id;
	unsigned short		len;
	unsigned char		content[2];
}__attribute__ ((packed)) ST_HF_MGW_DCU_LINK_MSG;


/*Ӧ��·���ϱ���Ϣ*/
typedef struct
{
	unsigned char		id;
	unsigned short		len;
	unsigned short		route_count;
	unsigned short		route_index;
	struct
	{
		unsigned long	hf_addr;
		unsigned long	next_hop;
		unsigned char	module_addr;
		unsigned short	cost;
	}__attribute__ ((packed)) route[16];
}__attribute__ ((packed)) ST_HF_MGW_DCU_ROUTE_MSG;

/*Ӧ��·��Ӧ����Ϣ*/
typedef struct
{
	unsigned char		id;
	unsigned short		len;
}__attribute__ ((packed)) ST_HF_MGW_DCU_ROUTE_ANS;

/*Ӧ��·�ɲ�ѯ��Ϣ*/
typedef struct
{
	unsigned char		id;
	unsigned short		len;
}__attribute__ ((packed)) ST_HF_MGW_DCU_ROUTE_FIND;


/*IP·��������Ϣ*/
typedef struct
{
	unsigned char		id;
	unsigned short		len;
	unsigned short		route_count;
	struct
	{
		unsigned long	ip_addr;
		unsigned char	mask;
	}__attribute__((packed)) route[16];
}__attribute__((packed)) ST_HF_MGW_DCU_ADD_IP_ROUTE_MSG;

/*IP·����Ϣ�ϱ�Ӧ����Ϣ*/
typedef struct
{
	unsigned char		id;
	unsigned short		len;
	unsigned short		route_count;
	struct
	{
		unsigned long	ip_addr;
		unsigned char	status;
	}__attribute__((packed)) route[16];	
}__attribute__((packed)) ST_HF_MGW_DCU_ADD_IP_ROUTE_ANS;

/*IP·��ɾ����Ϣ*/
typedef struct
{
	unsigned char		id;
	unsigned short		len;
	unsigned short		route_count;
	struct
	{
		unsigned long	ip_addr;
		unsigned char	mask;
	}__attribute__((packed)) route[16];
}__attribute__((packed)) ST_HF_MGW_DCU_DEL_IP_ROUTE_MSG;

/*IP·��ɾ��Ӧ����Ϣ*/
typedef struct
{
	unsigned char		id;
	unsigned short		len;
	unsigned short		route_count;
	struct
	{
		unsigned long	ip_addr;
		unsigned char	status;
	}__attribute__((packed)) route[16];
}__attribute__((packed)) ST_HF_MGW_DCU_DEL_IP_ROUTE_ANS;

/*ר��·�ɲ�ѯ��Ϣ*/
typedef struct
{
	unsigned char		id;
	unsigned short		len;
}__attribute__((packed)) ST_HF_MGW_DCU_HF_ROUTE_QUERY_MSG;

/*��ѯӦ����Ϣ*/
typedef struct
{
	unsigned char		id;
	unsigned short		len;
	unsigned short		route_count;
	struct
	{
		unsigned long	dst_hf_addr;
		unsigned short	cost;
	}__attribute__((packed)) route[128];
}__attribute__((packed)) ST_HF_MGW_DCU_HF_ROUTE_QUERY_ANS;


/*=========================ý�����ذ�<-->������===============================*/
/* Э��μ�<��¼5����ͨ�ŷ�ϵͳͨ����������豸��仰��ͨ�Žӿ�Э��> */

#define		MAXSCUPROSIZE			512

typedef enum
{
	INFO_TYPE_MANAGE = 0,	/*����ά����Ϣ*/
	INFO_TYPE_PARARM_SET,	/*����������Ϣ*/
	INFO_TYPE_CALL_CTL		/*��������*/
}EN_MGW_SCU_INFO_TYPE;

enum
{
	/*����ά����ϢID*/
	MGW_SCU_LINK_ID = 0x01,
	/*����������ϢID*/
	MGW_SCU_OPEN_ID = 0x01,
	MGW_SCU_USER_ASSIGN_REQ_ID,
	MGW_SCU_USER_ASSIGN_ANS_ID,
	MGW_SCU_USER_FREE_REQ_ID = 0x06,
	MGW_SCU_USER_FREE_ANS_ID = 0x07,
	MGW_SCU_CONFERENCE_ASSIGN_REQ_ID = 0x08,
	MGW_SCU_CONFERENCE_ASSIGN_ANS_ID = 0x09,
	MGW_SCU_ZHUANXIAN_REQ_ID = 0x0A,
	MGW_SCU_ZHUANXIAN_REQ_ACK_ID = 0x0B,
	MGW_SCU_ZHUANXIAN_FREE_ID = 0x0C,
	MGW_SCU_ZHUANXIAN_FREE_ACK_ID = 0x0D,	
	
	MGW_SCU_CLOSE_REQ_ID = 0x10,
	MGW_SCU_CLOSE_ANS_ID = 0x11,
	/*��������ID*/
	MGW_SCU_CALL_ID = 0x01,
	MGW_SCU_CALLACK_ID,
	MGW_SCU_CONNECT_ID,
	MGW_SCU_RELEASE_ID,
	MGW_SCU_RELEASE_CNF_ID,
	MGW_SCU_PTT_ID = 0x0a,
	MGW_SCU_NUM_TRANS_ID,
	MGW_SCU_CALL_TARNS_ID = 0x0c,
	MGW_SCU_LISTEN_ID = 0x0d,

	MGW_SCU_ZW_ZHUANXIAN_ID = 0x0e, //��ӦZ043��ר��ժ�һ�    
	
};

enum{
	PTT_STATE_RECV = 0x00,
	PTT_STATE_SEND,
};

#define HF_MGW_SCU_PRO_TYPE 	0x01

/* Z018��Z024������Э��汾�Ź̶�Ϊ0x01��Z043���ֶκ���ĳ�Ŀ���ַ���̶�Ϊ0xF2 */
#define HF_MGW_SCU_PRO_VER	0xF2//0x01

typedef struct
{
	unsigned char		protocol_type;	/*Э�����ͣ�дΪ1*/
	unsigned char		version;		/*�汾�ţ���λ1*/
	unsigned char		info_type;		/*��Ϣ����*/
	unsigned short		len;			/*����*/
}__attribute__ ((packed)) ST_HF_MGW_SCU_PRO_HEADER;

typedef struct
{
	ST_HF_MGW_SCU_PRO_HEADER	header;
	unsigned char	body[MAXSCUPROSIZE];
}__attribute__ ((packed)) ST_HF_MGW_SCU_PRO;


typedef struct
{
	unsigned char		protocol_type;	/*Э�����ͣ�дΪ1*/
	unsigned char		info_type;		/*��Ϣ����*/
	unsigned char		len;			/*����*/
}__attribute__ ((packed)) ST_HF_MGW_PHONE_PRO_HEADER;

typedef struct
{
	ST_HF_MGW_PHONE_PRO_HEADER	header;
	unsigned char	body[MAXSCUPROSIZE];
}__attribute__ ((packed)) ST_HF_MGW_PHONE_PRO;

typedef struct ADP_PHONE_SEM_HEAD
{
	uint8 ProType;		//Э������ 1
	uint8 Edition;		//�汾 1
	uint8 SemType;		//��Ϣ���� 0~2
	uint16 Len;
}__attribute__((packed)) ST_ADP_PHONE_SEM_HEAD;
typedef struct _ADP_PHONE_SEM
{	
	ST_ADP_PHONE_SEM_HEAD Head;
	uint8 Data[MAXSCUPROSIZE];	
}__attribute__((packed)) ST_ADP_PHONE_SEM;

typedef struct
{
	unsigned char		protocol_version;	/*Э�����ͣ�дΪ1*/
	unsigned char		dst_addr;		
	unsigned char		src_addr;		
	unsigned char		msg_type;	
	unsigned char		reserve;
	unsigned short		data_len;			/*����*/	
}__attribute__ ((packed)) ST_SEAT_MNG_HEADER;

typedef struct
{
	ST_SEAT_MNG_HEADER	header;
	unsigned char	body[MAXSCUPROSIZE];
}__attribute__ ((packed)) ST_SEAT_MNG_MSG;

typedef struct
{
	unsigned char		msg_type;	/*Э�����ͣ�дΪ1*/
	unsigned char		msg_flg;		
	unsigned char		board_local;		
	unsigned char		board_id;	
	unsigned short		data_len;			/*����*/	
}__attribute__ ((packed)) ST_RAY_MNG_HEADER;

typedef struct
{
	ST_RAY_MNG_HEADER	header;
	unsigned char	body[MAXSCUPROSIZE];
}__attribute__ ((packed)) ST_RAY_MNG_MSG;

typedef struct _PC_SEM_HEAD
{
	uint16 Head16;//ͷ/0x55aa
	uint8 Type;	//��������
	uint16 Resv; //����
	uint16 Len;
}__attribute__((packed)) ST_PC_SEM_HEAD;
typedef struct _PC_SEM
{	
	ST_PC_SEM_HEAD Head;
	uint8 Data[MAXSCUPROSIZE];
	uint16 CheckSum;//У����	
}__attribute__((packed)) ST_PC_SEM;

typedef struct
{
	uint8 header;
	uint16 total_len;
	uint8 cmd;
	uint8 type;
	uint8 data_len;
	unsigned char	body[16];
}__attribute__ ((packed)) ST_DISPLAY_MNG_MSG;


/*==========================����ά����Ϣ============================*/

/*������Ϣ*/
typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	link_status;
}__attribute__ ((packed)) ST_HF_MGW_SCU_LINK_MSG;

#define HF_MGW_SCU_LINK_STATUS_OK 		0x01
#define HF_MGW_SCU_LINK_STATUS_FAILED   0x00

/*״̬�㱨��Ϣ--ȡ��*/


/*=========================����������Ϣ=============================*/
/* ��ǰֻ֧�ֿ�ͨ���21���û����˺������USER_NUM_MAX һ��*/
#define HF_MGW_SCU_TERM_NUM 21

/*�û���ͨ��Ϣ*/
typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	mem_count;	/*��Ա����*/
	unsigned char	terminal[HF_MGW_SCU_TERM_NUM];	/*�ն˱�־--ʱ϶*/
}__attribute__ ((packed)) ST_HF_MGW_SCU_OPEN_MSG;


/*�ر���Ϣ*/
typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	mem_count;
	unsigned char	terminal[HF_MGW_SCU_TERM_NUM];
}__attribute__ ((packed)) ST_HF_MGW_SCU_CLOSE_REQ;

typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	mem_count;
	struct
	{
		unsigned char	terminal_id;
		unsigned char	ans;
	}__attribute__ ((packed))terminal[25];
}__attribute__ ((packed)) ST_HF_MGW_SCU_CLOSE_ANS;


/*�û�ע����Ϣ*/
typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	mem_count;	
	struct
	{
		unsigned char	terminal_id;
		unsigned char	ans;
		unsigned char	num_len;
		unsigned char	user_num[10];
		unsigned char	conference_enable;
		unsigned char	priority;
		unsigned char	encoder;
	}__attribute__ ((packed))terminal[25];
}__attribute__ ((packed)) ST_HF_MGW_SCU_USER_ASSIGN_REQ;

typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	mem_count;
	struct
	{
		unsigned char	terminal_id;
		unsigned char	ans;
	}__attribute__ ((packed))terminal[25];
}__attribute__ ((packed)) ST_HF_MGW_SCU_USER_ASSIGN_ANS;

typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	mem_count;
	struct
	{
		unsigned char	terminal_id;
	}__attribute__ ((packed))terminal[25];
}__attribute__ ((packed)) ST_HF_MGW_SCU_USER_FREE_REQ;

typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	mem_count;
	struct
	{
		unsigned char	terminal_id;
		unsigned char	ans;
	}__attribute__ ((packed))terminal[25];
}__attribute__ ((packed)) ST_HF_MGW_SCU_USER_FREE_ANS;

/*ԤԼ����ע����Ϣ*/
typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	conference_count;
	unsigned char	conference_body[256];/*��Ϣ�ṹ���ֽܷ�Ϊ�����Ľṹ��*/
}__attribute__ ((packed)) ST_HF_MGW_SCU_CONFERENCE_ASSIGN_REQ;

typedef struct
{
	unsigned char	id;
	unsigned char	len;
	struct
	{
		unsigned char	conference_id;
		unsigned char	ans;
	}__attribute__ ((packed))conference[10];
}__attribute__ ((packed)) ST_HF_MGW_SCU_CONFERENCE_ASSIGN_ANS;

/*�м̺���ע����Ϣ*/

/*============================����������Ϣ=================================*/

/*��������*/
typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	slot;
	unsigned char	call_type;
	unsigned char	encoder;
	unsigned char	phone_len;
	unsigned char	callee_num[10];
}__attribute__ ((packed)) ST_HF_MGW_SCU_CALL_REQ,ST_HF_MGW_SCU_CALL_EVENT;

/*����Ӧ��*/
typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	slot;
	unsigned char	call_type;
	unsigned char	call_status;	/*����״̬*/
}__attribute__ ((packed)) ST_HF_MGW_SCU_CALLACK_REQ,ST_HF_MGW_SCU_CALLACK_EVENT;

/*��������*/
typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	slot;
	unsigned char	call_type;
	unsigned char	encoder;
	unsigned char	phone_len;
	unsigned char	caller_num[10];
}__attribute__ ((packed)) ST_HF_MGW_SCU_CONNECT_REQ,ST_HF_MGW_SCU_CONNECT_EVENT;

/*�ͷ�����*/
typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	slot;
	unsigned char	call_type;
	unsigned char	rel_reason;
}__attribute__ ((packed)) ST_HF_MGW_SCU_RELEASE_REQ,ST_HF_MGW_SCU_RELEASE_EVENT;

/*�ͷ�֤ʵ*/
typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	slot;
	unsigned char	call_type;
}__attribute__ ((packed)) ST_HF_MGW_SCU_RELEASE_CNF_REQ,ST_HF_MGW_SCU_RELEASE_CNF_EVENT;


/*���ӻ����Ա*/

/*ɾ�������Ա*/

/*PTT����*/
typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	slot;
	unsigned char	ptt_status;
	unsigned char	peer_num[10];
}__attribute__ ((packed)) ST_HF_MGW_SCU_PTT_MSG;

typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	slot;
	unsigned char	call_type;
	unsigned char	phone_len;
	unsigned char	trans_phone[10];	
}__attribute__ ((packed)) ST_HF_MGW_SCU_NUM_TRANS_MSG;


/*����ת����Ϣ*/
typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	slot;
	unsigned char	phone_len;
	unsigned char	trans_phone[10];	
}__attribute__ ((packed)) ST_HF_MGW_SCU_CALL_TRANS_MSG;

enum{
	MGW_LISTEN_OPEN = 0x01,
	MGW_LISTEN_CLOSE = 0x00,
};

/*������Ϣ*/
typedef struct
{
	unsigned char	id;
	unsigned char	len;
	unsigned char	slot;
	unsigned char	operate;	
}__attribute__ ((packed)) ST_HF_MGW_SCU_LISTEN_MSG;


/*==============================��������������=================================*/
enum{
	MGW_SCU_JS_CALL_ID = 0x8801,
	MGW_SCU_JS_CALLACK_ID,
	MGW_SCU_JS_CONNECT_ID,
	MGW_SCU_JS_CONNECT_CNF_ID,
	MGW_SCU_JS_RELEASE_ID,
	MGW_SCU_JS_RELEASE_CNF_ID,
	MGW_SCU_JS_PTT_ID,
	MGW_SCU_JS_PTT_ANS_ID,
};

typedef struct{
	unsigned short	id;
	unsigned char	len;
}__attribute__((packed)) ST_HF_MGW_SCU_JS_PRO_HEADER;

typedef struct{
	ST_HF_MGW_SCU_JS_PRO_HEADER header;
	char body[64];
}__attribute__((packed)) ST_HF_MGW_SCU_JS_PRO;

typedef struct{
	char	caller[10];
	char	callee[10];
	unsigned char	slot;
}__attribute__((packed)) ST_HF_MGW_SCU_JS_CALL_REQ,ST_HF_MGW_SCU_JS_CALL_EVENT;

typedef struct{
	char	caller[10];
	unsigned char	call_status;
	unsigned char	slot;
}__attribute__((packed)) ST_HF_MGW_SCU_JS_CALLACK_REQ,ST_HF_MGW_SCU_JS_CALLACK_EVENT;

typedef struct{
	char	caller[10];
	unsigned char	slot;
}__attribute__((packed)) ST_HF_MGW_SCU_JS_CONNECT_REQ,ST_HF_MGW_SCU_JS_CONNECT_EVENT;

typedef struct{
	char	caller[10];
	unsigned char	slot;
}__attribute__((packed)) ST_HF_MGW_SCU_JS_CONNECT_CNF_MSG;

typedef struct{
	char	caller[10];
	unsigned char	slot;
}__attribute__((packed)) ST_HF_MGW_SCU_JS_RELEASE_REQ,ST_HF_MGW_SCU_JS_RELEASE_EVENT;

typedef struct{
	char	caller[10];
	unsigned char	slot;
}__attribute__((packed)) ST_HF_MGW_SCU_JS_RELEASE_CNF_MSG;

typedef struct{
	char	caller[10];
	unsigned char	slot;
	unsigned char	ptt_status;
}__attribute__((packed)) ST_HF_MGW_SCU_JS_PTT_MSG,ST_HF_MGW_SCU_JS_PTT_ANS_MSG;

/* Added by lishibing 20150925, Z043װ��ģʽ����ս��ר��ע����Ϣ ��
Э�鰴<ר����ս�������豸����ӿ�Э��v011 20150922>
*/

#define MNG_50_MSG_LEN 600 
typedef struct __Mng_50_Msg_Head__
{
	uint8 ProType;
	uint8 VerId;
	uint8 Headlen;
	uint8 PackType;
	uint8 ChanId;
	uint8 ServiceType;
	uint32 SrcAddr;
	uint8 NumOfDstAddr;
	uint32 DstAddr;
	uint16 MsgLen;
}__attribute__((packed))MNG_50_MSG_HEAD;

typedef struct __MNG_50_MSG__
{
	MNG_50_MSG_HEAD Head;
	uint8  Data[MNG_50_MSG_LEN];//����
}__attribute__((packed))MNG_50_MSG;

enum 
{
	EN_PORT_50MAINCTRL = 0x10,
	EN_PORT_INTF1_WIRELESS = 0x20,
	EN_PORT_INTF1_VHF1 = 0x21,
	EN_PORT_INTF1_VHF2 = 0x22,
	EN_PORT_INTF1_VHF3 = 0x23,
	EN_PORT_INTF1_HF1 = 0x25,
	
	EN_PORT_INTF2_WIRELESS = 0x30,
	EN_PORT_INTF2_VHF1 = 0x31,
	
	EN_PORT_INTF_WIRE = 0x40,
	EN_PORT_INTF1_WIRE = 0x41,
	EN_PORT_INTF2_WIRE = 0x42,
	EN_PORT_INTF3_WIRE = 0x43,
	EN_PORT_INTF4_WIRE = 0x44,
	EN_PORT_INTF5_WIRE = 0x45,
	EN_PORT_INTF6_WIRE = 0x46,
	
	EN_PORT_VIOICE_BRD = 0x50,
	
	EN_PORT_716NETWARE = 0x80,

	EN_PORT_PC_NETMANAGE = 0x90,
	
	EN_PORT_834_YEWU_BRD = 0xb0,

	EN_PORT_834_KEY_BRD = 0xd0,
	
	EN_PORT_ALL_BOARD = 0xff	
};

#ifdef MNG_FOR_JINBIAO 	
typedef struct __Mng_Zw_ZhuanXian_Msg__
{
	uint8 DstAddr;
	uint16 InfoType;
	uint16 CmdLen;
	uint8 CmdCode;
	uint8 OpMode;
	uint8 CmdMsgLen;
	uint8 OpTypeSel;
	uint16 IndxNum;
	uint8 AckFlgSel;
	uint8 CallerFlgSel;
	uint8 CallerChanId;
	uint8 JtjId[2];
	uint8 BJPhoneNum[4];
	uint8 ZXianStatus;		
}__attribute__((packed))MNG_ZW_ZHUANXIAN_MSG;
#else
typedef struct __Mng_Zw_ZhuanXian_Msg__
{
	//uint8 DstAddr;
	//uint16 InfoType;
	uint8 InfoType;
	uint16 CmdLen;
	uint8 CmdCode;
	uint8 OpMode;
	uint8 CmdMsgLen;
	uint8 OpTypeSel;
	uint16 IndxNum;
	uint8 AckFlgSel;
	uint8 CallerFlgSel;
	uint8 CallerChanId;
	uint8 JtjId[2];
	char BJPhoneNum[4];
	uint8 ZXianStatus;		
}__attribute__((packed))MNG_ZW_ZHUANXIAN_MSG;
#endif

//++++++++++++

//--------------
typedef struct __Mng_ZwZx_Query_Msg__
{
	//uint8 DstAddr;
	//uint16 InfoType;
	uint8 InfoType;
	uint16 CmdLen;
	uint8 CmdCode;
	uint8 OpMode;
	uint8 CmdMsgLen;
	uint8 OpTypeSel;
	uint16 IndxNum;
	uint8 AckFlgSel;
	uint8 CallerFlgSel;
	uint8 CallerChanId;	
}__attribute__((packed))MNG_ZWZX_QUERY_MSG, MNG_ZWZX_UNREGISTER_MSG;

enum 
{
	EN_OP_ZWZHUANXIAN_REGIST = 0x01,
	EN_OP_ZWZHUANXIAN_QUERY = 0x02,
	EN_OP_ZWZHUANXIAN_UNREGIST = 0x03,
	EN_OP_ZWZHUANXIAN_REGIST_ACK = 0x04,
	EN_OP_ZWZHUANXIAN_QUERY_ACK = 0x05,
	EN_OP_ZWZHUANXIAN_UNREGIST_ACK = 0x06,
	EN_OP_ZWZHUANXIAN_HUIBAO_REG = 0x07,
	EN_OP_ZWZHUANXIAN_HUIBAO_UNREG = 0x08
};
/* K��ר��ע�ᡢע�������Ϣͷ������ */
#define MSG_HEADER_LEN 15

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif	/*_INTERIOR_PROTOCOL_H_*/
