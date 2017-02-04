/*
===========================================================================
** file name: sipag_common.h
** description: 与SIP端通用头文件
===========================================================================
*/
#ifndef __SIPAG_COMMON_H__
#define __SIPAG_COMMON_H__

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" { 
#endif


#define		MAXNUMLEN	20/*最大号码数组长度*/
#define		IDLE		0xee/*无效数据标示*/


/*SIP函数调用的返回值*/
enum{
	EN_SIP_OK = 0,
	EN_SIP_ERROR = -1,
};


typedef enum{
	PACK_TYPE_RTP = 1,
	PACK_TYPE_RTCP = 2
}EN_SIPAG_PACK_TYPE;

typedef enum{
	SIPAG_CALL_STATUS_ANSWER = 0x01,
	SIPAG_CALL_STATUS_BUSY ,
	SIPAG_CALL_STATUS_CANCEL ,
}EN_SIPAG_CALL_STATUS;

typedef enum{
	SIPAG_PTT_DOWN = 0x01,
	SIPAG_PTT_UP,
	SIPAG_PTT_HOLD,
}EN_SIPAG_PTT_STATUS;

/*1.呼叫请求,响应*/
#if 0
!brief -- sip example
ST_SIP_IPC_MSG_CALL_REQ CallReq;
strcpy(CallReq.CallerNum,"1234567");/*填写主叫号码*/
strcpy(CallReq.CalleeNum,"7654321");/*填写被叫号码*/
CallReq.Domain = inet_addr("10.11.12.13");/*填写对端IP地址*/
if(st_sipag_intf_func.SIP_Call_Request(&CallReq) == EN_SIP_OK)
	call_id = CallReq.CallID;/*获取callid,作为后续消息的标识传递*/
#endif

typedef struct{
        unsigned char useType;/*useType暂无意义*/
		char CallerNum[MAXNUMLEN];
		char CalleeNum[MAXNUMLEN];
		unsigned long Domain;/*对端UA地址信息,网络字节序*/
		unsigned short CallID;
}ST_SIP_IPC_MSG_CALL_REQ,ST_SIP_IPC_MSG_CALL_EVENT;


/*2.呼叫应答请求,响应*/
typedef struct{
		unsigned short CallID;
		unsigned char useType;
		EN_SIPAG_CALL_STATUS useStatus;
		unsigned long Domain;/*对端UA地址信息,网络字节序*/
		char CalleeNum[MAXNUMLEN];
}ST_SIP_IPC_MSG_CALLACK_REQ,ST_SIP_IPC_MSG_CALLACK_EVENT;


/*3.连接请求,响应*/
typedef struct{
		unsigned short CallID;
		unsigned char useType;
}ST_SIP_IPC_MSG_CONNECT_REQ,ST_SIP_IPC_MSG_CONNECT_EVENT;

/*4.释放请求,响应*/
typedef struct{
		unsigned short CallID;
		unsigned char useType;
		EN_SIPAG_CALL_STATUS useStatus;
}ST_SIP_IPC_MSG_RELEASE_REQ,ST_SIP_IPC_MSG_RELEASE_EVENT;

/*5.增加用户*/
typedef struct{
		char name[MAXNUMLEN];/*用户号码*/
}ST_SIP_IPC_MSG_USER_MSG;

typedef struct{
		unsigned short CallID;
		EN_SIPAG_PACK_TYPE packType;
		short packLen;
		char pack[1600];
}ST_SIP_IPC_MSG_PACK_READ,ST_SIP_IPC_MSG_PACK_WRITE;

typedef struct{
	unsigned short CallID;
	EN_SIPAG_PTT_STATUS status;
}ST_SIP_IPC_MSG_PTT_MSG;

typedef struct{
		int (*SIP_Call_Request)(ST_SIP_IPC_MSG_CALL_REQ *);
		int (*SIP_Call_Notify)(ST_SIP_IPC_MSG_CALL_EVENT *);
		
		int (*SIP_CallAck_Request)(ST_SIP_IPC_MSG_CALLACK_REQ *);
		int (*SIP_CallAck_Notify)(ST_SIP_IPC_MSG_CALLACK_EVENT *);
		
		int (*SIP_Connect_Request)(ST_SIP_IPC_MSG_CONNECT_REQ *);
		int (*SIP_Connect_Notify)(ST_SIP_IPC_MSG_CONNECT_EVENT *);
		
		int (*SIP_Release_Request)(ST_SIP_IPC_MSG_RELEASE_REQ *);
		int (*SIP_Release_Notify)(ST_SIP_IPC_MSG_RELEASE_EVENT *);

		int (*SIP_Append_User)(ST_SIP_IPC_MSG_USER_MSG *);

		int (*SIP_Pack_Read)(ST_SIP_IPC_MSG_PACK_READ *);
		int (*SIP_Pack_Write)(ST_SIP_IPC_MSG_PACK_WRITE *);

		int (*SIP_PTT_Request)(ST_SIP_IPC_MSG_PTT_MSG *);
		int (*SIP_PTT_Notify)(ST_SIP_IPC_MSG_PTT_MSG *);
}ST_SIPAG_INTF_FUNC;

extern	ST_SIPAG_INTF_FUNC st_sipag_intf_func;
extern int init_logger(void);
extern int sip_load_module(struct in_addr);

#if defined(__cplusplus) || defined(c_plusplus)
} 
#endif 

#endif
