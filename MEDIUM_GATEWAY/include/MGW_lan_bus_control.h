#ifndef _MGW_LAN_BUS_CONTROL_H_
#define	_MGW_LAN_BUS_CONTROL_H_

#if defined(__cplusplus)||defined(c_plusplus)
extern "C"
{
#endif


typedef struct
{
	unsigned char		baowen_type;
	unsigned char		dst_addr;
	unsigned char		src_addr;
	unsigned char		info_type;
	unsigned short		data_len;
}__attribute__ ((packed)) ST_HF_MGW_DCU_PRO_HEADER;

typedef struct
{
	ST_HF_MGW_DCU_PRO_HEADER header;
	unsigned char		body[INTMAXDATASIZE];
}__attribute__ ((packed)) ST_HF_MGW_DCU_PRO;



extern char	flag_dump;
enum
{
	DUMP_OPEN = 0,
	DUMP_CLOSE,
};

/* 函数声明，消除告警 */
int32 Board_Mng_SendTo_716(uint8 *buf, int32 len);
int32 Board_Mng_SendTo_50(uint8 *buf, int32 len);
int32 Board_Mng_SendTo_716_Ray(uint8 *buf, int32 len);
int32 Board_Mng_SendTo_716_Radio(uint8 *buf, int32 len);
int32 RpcSeatMng_SendTo_Seat(uint8 *buf, int32 len);
void RpcSeatMng_RxThread(void);
int32 RpcSeatMng_Socket_init(void);
int32 RpcSeatMng_Socket_Close(void);
int32 RpcSeatMng_Default_SendTo_Seat(uint8 *buf, int32 len);
void RpcSeatMng_Default_RxThread(void);
int32 RpcSeatMng_Default_Socket_init(void);
int32 RpcSeatMng_Default_Socket_Close(void);

int32 Board_716_Mng_Socket_init(void);
int32 Board_716_Mng_Socket_Close(void);
int32 Board_716_Radio_Socket_init(void);
int32 Board_716_Radio_Socket_Close(void);
int32 Board_716_Ray_Mng_Socket_init(void);
int32 Board_716_Ray_Mng_Socket_Close(void);
int32 Board_50_Mng_Socket_init(void);
int32 Board_50_Mng_Socket_Close(void);
int32 Board_50_Mng_before_Socket_init(void);
int32 Board_50_Mng_before_Socket_Close(void);
int RpcQinWu_Socket_init(void);
int RpcQinWu_Socket_Close(void);

int RpcSeat_Socket_init(void);
void RpcSeat_RxThread(void);
int RpcSeat_Socket_Close(void);
int32 Board_Display_Connect(void);

int RpcVoice_Socket_init(void);
void RpcVoice_RxThread(void);
int RpcVoice_Socket_Close(void);


int32 RpcDisplayMng_Socket_init(void);
void RpcDisplayMng_RxThread(void);
void Board_716_Mng_RxThread(void);
void Board_716_Ray_Mng_RxThread(void);
void Board_50_Mng_RxThread(void);
void Board_50_Mng_before_RxThread(void);
void Board_716_Radio_RxThread(void);
void RpcDisplayMng_Zhuangjia_From_dis_RxThread(void);
int32 RpcDisplayMng_Zhuangjia_From_dis_Socket_init(void);
int32 RpcDisplayMng_Zhuangjia_from_dis_Socket_Close(void);
void RpcDisplayMng_Zhuangjia_from_716_RxThread(void);
int32 RpcDisplayMng_from_716_Socket_init(void);
int32 RpcDisplayMng_Zhuangjia_Socket_from_716_Close(void);
int32 RpcDisplayMng_Socket_Close(void);

int	js_route_proc(int *id, int fd, short events, void *cbdata);
int	js_data_proc(int *id, int fd, short events, void *cbdata);

#if defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /*_MGW_LAN_BUS_CONTROL_H_*/

