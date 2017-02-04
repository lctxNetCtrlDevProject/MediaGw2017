/*
===========================================================================
** file name: sipua_common.h
** description: 与SIP端通用头文件
===========================================================================
*/
#ifndef __SIPUA_COMMON_H__
#define __SIPUA_COMMON_H__

#if defined(__cplusplus) 
extern "C" 
{ 
#endif




typedef enum
{
	EXT_GETEWAY = 1,
	DIANTAI_GATEWAY,
	G_GATEWAY
} EN_SIP_IPC_GATEWAY_TYPE;

typedef enum  {

	EN_MSG_CALL_REQUEST = 0x01,//呼叫请求消息	0x01
	EN_MSG_CALL_NUMBER,//号码传递消息	0x02
	EN_MSG_CALL_ACK,//呼叫应答消息	0x03
	EN_MSG_CONNECT_REQUEST,//连接请求消息	0x04
	EN_MSG_RELEASE_REQUEST,//释放请求消息	0x05
	EN_MSG_RELEASE_ACK,//释放证实消息	0x06
	EN_MSG_DELETE_CONFORCE,//删除会议成员请求消息	0x07
	EN_MSG_DELETE_CONFORCE_ACK,//删除会议成员证实消息	0x08
    EN_MSG_PTT,//PTT按键消息	0x09
    EN_MSG_CALL_BUTT
} EN_SIP_IPC_MSG_TYPE;

typedef enum  {
	EN_SEL_CALL = 0x01,//0x01   选呼
	EN_PLAN_CONF,//0x02   预约全双工会议
	EN_RANDOM_CONF,//0x03   随机全双工会议
	EN_ADD_CALL_MEMBER,//0x04   增加会议成员
	EN_GROUP_CALL,//0x05   群呼
	EN_FORCE_CALL,//0x06   强插
	EN_FORCE_RELEASE_CALL,//0x07   强拆
	EN_LISTENING,//0x08   监听
	EN_CALL_FORWARD,//0x09   呼叫转移
	EN_CALL_FORWARD_TO,//0x0a   呼叫转接
	EN_SPECIFIC_CALL,//0x0b   专线
	EN_OMIT_CALL,//0x0c   缩位拨号

    EN_MSG_USE_TYPE_BUTT
} EN_SIP_IPC_MSG_USE_TYPE;


typedef enum
{
	EN_SPEAK_ONLY = 1,
	EN_LISTEN_ONLY,
	EN_DOUBLE,//双向
	EN_SILENT //静默
}EN_SIP_IPC_MSG_MEDIA_TYPE;


typedef enum
{
	EN_ERROR_FORWARD = 1,
	EN_ERROR_BACKWARD
}EN_SIP_IPC_MSG_ERROR_DIR;


/*SIP函数调用的出错信息标识*/
typedef enum
{
	EN_SIP_OK = 0,
	EN_SIP_ERROR = -1
}EN_SIP_IPC_ERRNO;

/*包类型*/
typedef enum
{
	PACK_TYPE_RTP = 1,
	PACK_TYPE_RTCP = 2
}EN_SIP_IPC_MSG_PACK_TYPE;

//前向请求,后向响应
/*1.呼叫请求,响应*/
typedef struct _Sip_IPC_Msg_Call{
		EN_SIP_IPC_MSG_TYPE msgSubType;//消息子类型,0x01
		unsigned char membPortIndex;//成员对应的物理端口号
        	EN_SIP_IPC_MSG_USE_TYPE useType;//接续类型
		unsigned char chanType;//信道类型,有线：0x00；无线：0x01
		unsigned char CallerNum[MAXNUMLEN];//主叫号码
		unsigned char CalleeNum[MAXNUMLEN];//被叫号码
		unsigned short CallID;//呼叫ID 0--error  >0--正常
		unsigned int msgSize;//消息长度
}ST_SIP_IPC_MSG_CALL_REQ,ST_SIP_IPC_MSG_CALL_EVENT;


/*2.呼叫应答请求,响应*/
typedef struct _Sip_IPC_Msg_CallAck{
		EN_SIP_IPC_MSG_TYPE msgSubType;//消息子类型,0x02
		unsigned char membPortIndex;//成员对应的物理端口号
        	EN_SIP_IPC_MSG_USE_TYPE useType;//接续类型
		unsigned char useStatus;//接续状态
		unsigned char msgSize;//消息长度
		unsigned char CallerNum[MAXNUMLEN];//主叫号码
		unsigned char CalleeNum[MAXNUMLEN];//被叫号码
		unsigned short CallID;//呼叫ID 0--error  >0--正常
}ST_SIP_IPC_MSG_CALLACK_REQ,ST_SIP_IPC_MSG_CALLACK_EVENT;


/*3.连接请求,响应*/
typedef struct _Sip_IPC_Msg_Connect{
		EN_SIP_IPC_MSG_TYPE msgSubType;//消息子类型
		unsigned char membPortIndex;//成员对应的物理端口号
        	EN_SIP_IPC_MSG_USE_TYPE useType;//接续类型
		unsigned char chanType;//信道类型,有线：0x00；无线：0x01
		unsigned char CallerNum[MAXNUMLEN];//主叫号码
		unsigned char CalleeNum[MAXNUMLEN];//被叫号码
		unsigned short CallID;//呼叫ID 0--error  >0--正常
		unsigned char msgSize;//消息长度
}ST_SIP_IPC_MSG_CONNECT_REQ,ST_SIP_IPC_MSG_CONNECT_EVENT;

/*4.释放请求,响应*/
typedef struct _Sip_IPC_Msg_Release{
		EN_SIP_IPC_MSG_TYPE msgSubType;//消息子类型
		unsigned char membPortIndex;//成员对应的物理端口号
        	EN_SIP_IPC_MSG_USE_TYPE useType;//接续类型
		unsigned char useStatus;//释放原因
		unsigned char CallerNum[MAXNUMLEN];//主叫号码
		unsigned char CalleeNum[MAXNUMLEN];//被叫号码
		unsigned short CallID;//呼叫ID 0--error  >0--正常
		unsigned char msgSize;//消息长度
}ST_SIP_IPC_MSG_RELEASE_REQ,ST_SIP_IPC_MSG_RELEASE_EVENT;

/*5.增加会议成员*/
typedef struct _Sip_IPC_Msg_Add_Memb{
		EN_SIP_IPC_MSG_TYPE msgSubType;//消息子类型
		unsigned char membPortIndex;//成员对应的物理端口号
		unsigned char AddNumLen;//增加成员的号码长度
		unsigned char AddCalleeNum[MAXNUMLEN];//增加成员的号码
		unsigned short CallID;//呼叫ID 0--error  >0--正常
		unsigned char msgSize;//消息长度
}ST_SIP_IPC_MSG_ADD_MEMB_REQ,ST_SIP_IPC_MSG_ADD_MEMB_EVENT;

/*6.删除会议成员*/
typedef struct _Sip_IPC_Msg_Del_Memb{
		EN_SIP_IPC_MSG_TYPE msgSubType;//消息子类型
		unsigned char membPortIndex;//成员对应的物理端口号
		unsigned char delNumLen;//删除成员的号码长度
		unsigned char delCalleeNum[MAXNUMLEN];//删除成员的号码
		unsigned short CallID;//呼叫ID 0--error  >0--正常
		unsigned char msgSize;//消息长度
}ST_SIP_IPC_MSG_DEL_MEMB_REQ,ST_SIP_IPC_MSG_DEL_MEMB_EVENT;

/*7.媒体流协商请求,响应*/
typedef struct _Sip_IPC_Msg_Media{
		EN_SIP_IPC_MSG_TYPE msgSubType;
		unsigned char membPortIndex;
		unsigned short CallID;
		EN_SIP_IPC_MSG_MEDIA_TYPE mediaType;
		unsigned char msgSize;
}ST_SIP_IPC_MSG_MEDIA_REQ,ST_SIP_IPC_MSG_MEDIA_EVENT;

/*
** 8.前向,后向错误信息--->在需进行异步操作的调用过程中,
** 需使用异步的错误消息传递机制
*/
typedef struct _Sip_IPC_Msg_Error
{
		EN_SIP_IPC_MSG_TYPE msgSubType;
		unsigned char membPortIndex;
		EN_SIP_IPC_MSG_ERROR_DIR err_direction;/*1--forward--网关板->呼叫板 2--backward--呼叫板->网关板*/
		unsigned short CallID;
		EN_SIP_IPC_ERRNO err_type;
		unsigned char err_info[64];
}ST_SIP_IPC_MSG_ERROR;


/*9.rtp/rtcp包收发*/
typedef struct _Sip_IPC_Msg_Pack_Trans{
		EN_SIP_IPC_MSG_TYPE msgSubType;//消息子类型
		unsigned char membPortIndex;//成员对应的物理端口号
		unsigned char msgSize;//消息长度
		unsigned short CallID;//呼叫ID 0--error  >0--正常
		EN_SIP_IPC_MSG_PACK_TYPE packTyep;
		unsigned short packLen;
		char pack[1600];
}ST_SIP_IPC_MSG_PACK_READ,ST_SIP_IPC_MSG_PACK_WRITE;

//函数接口集
typedef struct _Sip_Ua_intf_Func{
		int (*SIP_Call_Request)(ST_SIP_IPC_MSG_CALL_REQ *Callreq);
		int (*SIP_Call_Notify)(ST_SIP_IPC_MSG_CALL_EVENT* Callevent);
		
		int (*SIP_CallAck_Request)(ST_SIP_IPC_MSG_CALLACK_REQ *CallAckreq);
		int (*SIP_CallAck_Notify)(ST_SIP_IPC_MSG_CALLACK_EVENT* CallAckevent);
		
		
		int (*SIP_Connect_Request)(ST_SIP_IPC_MSG_CONNECT_REQ *Connectreq);
		int (*SIP_Connect_Notify)(ST_SIP_IPC_MSG_CONNECT_EVENT* Connectevent);
		
		
		int (*SIP_Release_Request)(ST_SIP_IPC_MSG_RELEASE_REQ *Rlsreq);
		int (*SIP_Release_Notify)(ST_SIP_IPC_MSG_RELEASE_EVENT* Rlsevent);
		
		int (*SIP_Add_Member_Request)(ST_SIP_IPC_MSG_ADD_MEMB_REQ* AddMembreq);
		int (*SIP_Add_Member_Notify)(ST_SIP_IPC_MSG_ADD_MEMB_EVENT* AddMembevent);
		
		int (*SIP_Del_Member_Request)(ST_SIP_IPC_MSG_DEL_MEMB_REQ *DelMembreq);
		int (*SIP_Del_Member_Notify)(ST_SIP_IPC_MSG_DEL_MEMB_EVENT *DelMembevent);
		
		
		int (*SIP_Media_Request)(ST_SIP_IPC_MSG_MEDIA_REQ* Mediareq);
		int (*SIP_Media_Notify)(ST_SIP_IPC_MSG_MEDIA_EVENT* Mediaevent);
		
		
		int (*SIP_Error_Forward)(ST_SIP_IPC_MSG_ERROR* Msgerr);
		int (*SIP_Error_Backward)(ST_SIP_IPC_MSG_ERROR* Msgerr);

		int (*SIP_Pack_Read)(ST_SIP_IPC_MSG_PACK_READ * MsgPack);
		int (*SIP_Pack_Write)(ST_SIP_IPC_MSG_PACK_WRITE * MsgPack);

		//int (*SIP_Ptt_Request)(unsigned int chan, ST_SIP_IPC_MSG_PTT *ptt);
		
}ST_UA_INTF_FUNC;

//请UA提供回调函数注册功能
extern	ST_UA_INTF_FUNC st_ua_intf_func;
int ast_lctx_register(ST_UA_INTF_FUNC *func);


#if defined(__cplusplus) 
} 
#endif 

#endif // __SIPUA_COMMON_H__