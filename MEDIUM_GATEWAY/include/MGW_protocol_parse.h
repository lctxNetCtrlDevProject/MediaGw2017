#ifndef _MGW_PROTOCOL_PARSE_H_
#define	_MGW_PROTOCOL_PARSE_H_

#if defined(__cplusplus)||defined(c_plusplus)
extern "C"
{
#endif


/* Added by lishibing 20140723, IP地址和端口号用宏定义 */

/* 板间数据通信时，数据交换板IP 和端口 ,
注意: 

HF_MGW_SCU_SCUDATA_IP必须与mgw.conf中 DEFAULT_REMOTE_IP相同。
mgw.conf中 DEFAULT_LOCAL_IP 为834媒体网关板IP,固定为192.168.254.4/29 ，端口号：50718

Z024上50所分数据交换板和呼叫控制板，
数据交换板：IP 地址：192.168.9.1,目的端口号：9050
呼叫控制板：IP 地址：192.168.9.2,目的端口号：40000.
而Z034上50所只有一块单板,
炮防通信控制板：IP 地址：192.168.254.2/29    端口号：50718

对应834媒体网关板代码，原来与呼叫控制板通信IP在mgw.conf的DEFAULT_REMOTE_IP配置。
与数据交换板通信的IP在代码中固定为。
*/



#define ACK_OK 			0x00
#define ACK_FAILED 	0x01
#define MSG_DISPLAY_TYPE		0x90
#define MSG_PFB50_ADDR			0x10
#define MSG_YWB834_ADDR		0xB0
#define MSG_DISPLAY_ADDR		0xD0

enum __50_PARA_ID__
{
	MSG_50_BOARD_CONNECT = 0xFE,
	MSG_50_WORKMODE_SET = 0xF0, //竞标状态，50所需设置整机模式，定型状态下不需要
	MSG_50_WORKMODE_SET_ACK = 0xF1,
	MSG_50_WORKMODE_GET = 0xF2,
	MSG_50_WORKMODE_GET_ACK = 0xF3,

	MSG_50_K_MODE_SET = 0x10, //取5.1.5的信息类型中前一个字节，下同
	MSG_50_K_MODE_SET_ACK = 0x11,
	MSG_50_K_MODE_GET = 0x20,
	MSG_50_K_MODE_GET_ACK = 0x21,
};
enum __716_PARA_ID__
{
	MSG_834_REF716_WORKMODE_SET = 0x0D,
	MSG_834_REF716_WORKMODE_SET_ACK = 0x0E,
	MSG_834_REF716_WORKMODE_GET = 0x0F,
	MSG_834_REF716_WORKMODE_GET_ACK = 0x10,
};

enum __834_PARA_ID__
{
	MSG_834_HANDLE_MSG = 0x01,
	MSG_834_HANDLE_MSG_ACK = 0x02,	
	MSG_834_ZHENRUFENJI_ADD = 0x03,
	MSG_834_ZHENRUFENJI_ADD_ACK = 0x04,
	MSG_834_ZHENRUFENJI_DEL = 0x05,
	MSG_834_ZHENRUFENJI_DEL_ACK = 0x06,	
	MSG_834_ZHENRUFENJI_GET= 0x07,
	MSG_834_ZHENRUFENJI_GET_ACK = 0x08,		
	MSG_834_GPORT_MODE_SET = 0x09,
	MSG_834_GPORT_MODE_SET_ACK = 0x0A,	
	MSG_834_GPORT_MODE_GET = 0x0B,
	MSG_834_GPORT_MODE_GET_ACK = 0x0C,	
	MSG_834_WORKMODE_SET = 0x0D,
	MSG_834_WORKMODE_SET_ACK = 0x0E,
	MSG_834_WORKMODE_GET = 0x0F,
	MSG_834_WORKMODE_GET_ACK = 0x10,	

	MSG_834_GROUP_ID_ADD = 0x11,
	MSG_834_GROUP_ID_ADD_ACK = 0x12,		
	MSG_834_GROUP_ID_DEL = 0x13,
	MSG_834_GROUP_ID_DEL_ACK = 0x14,	
	MSG_834_GROUP_ID_GET = 0x15,
	MSG_834_GROUP_ID_GET_ACK = 0x16,	
	MSG_834_GROUP_MEM_ADD = 0x17,
	MSG_834_GROUP_MEM_ADD_ACK = 0x18,		
	MSG_834_GROUP_MEM_DEL = 0x19,
	MSG_834_GROUP_MEM_DEL_ACK = 0x1A,	
	MSG_834_MSG_BUFF
};

#define MSG_TYPE_PM_SET 		0x09
#define MSG_TYPE_PM_SET_ACK 	0x0A
#define MSG_TYPE_PM_GET 		0x0B
#define MSG_TYPE_PM_GET_ACK 	0x0C

enum __WORK_MODE__
{
	WORK_MODE_PAOBING 	= 0x01,
	WORK_MODE_ZHUANGJIA	= 0x02,
	WORK_MODE_JILIAN		= 0x03,
	WORK_MODE_SEAT		= 0x04,	
	WORK_MODE_ADAPTER	= 0x05,
	
	WORK_MODE_BUFF,
};


int32 Board_Sem_SendTo_Inc(uint8 *buf, int32 len);
int32 Board_Data_SendTo_Inc(uint8 *buf, int32 len);
int mgw_check_MsgLen(unsigned short sMsgBodyLen, unsigned short sMsgDefLen);
int string_cmp(const char *s1, const char *s2);
int string_ncmp(const char *s1, const char *s2, int count);
char *string_dup(const char *s1);
int find_net(struct sockaddr_in * addr);
unsigned char mgw_fetch_mask(unsigned long mask);
unsigned long mgw_equal_if(unsigned long ip_addr);
int mgw_test_ip(unsigned long ip_addr);
char * mgw_find_if(unsigned long ip_addr);
struct in_addr mgw_cal_net(struct in_addr addr,int logmask);
int find_tunnel_type_by_cost(unsigned char cost);
void phone2bcd(char *dest, char *src,size_t count);
int bcd2phone(char *dest, char *src, size_t count);
unsigned short cal_checksum(BYTE* src, size_t count);
int find_port_by_ip(struct sockaddr_in *addr);
int find_port_by_slot(unsigned char slot);
int mgw_ext_assign_user_number(const void * data);
int mgw_ext_release_cnf_notify(TERMINAL_BASE* terext);
int mgw_scu_calltrans_request(TERMINAL_BASE* terext);
int mgw_scu_listen_request(TERMINAL_BASE* terext,char operate);
int mgw_scu_js_call_request(TERMINAL_BASE * terext);
int mgw_scu_js_callack_request(TERMINAL_BASE * terext);
int mgw_scu_js_connect_request(TERMINAL_BASE * terext);
int mgw_scu_js_connect_cnf_request(TERMINAL_BASE * terext);
int mgw_scu_js_release_request(TERMINAL_BASE * terext);
int mgw_scu_js_release_cnf_request(TERMINAL_BASE * terext);
int mgw_scu_js_ptt_request(TERMINAL_BASE * terext);
int mgw_scu_js_ptt_ans_request(TERMINAL_BASE * terext,unsigned char ptt);
void mgw_dcu_link(void);
int32 Socket_Send(int32 bsockfd,struct sockaddr_in *pt, uint8 *ptr,int32 len);
int32 Dbg_Socket_Send(uint8 *buf, int32 len);
int32 Hf_IVoc_Socket_Send(uint8 *buf, uint32 len);
int32 Hf_SeatSem_Socket_Send(uint8 *buf, uint32 len);
int32 Board_Data_SendTo_Inc_Addr(uint8 *buf, int32 len, 	struct sockaddr_in addr);
int Board_Mng_Query_SpecificZhuanXian(uint8 kPortId);
int32 Board_Mng_834Proc(uint8 *buf, int32 len);
int32 Board_Mng_Sendto50Proc(uint8 *buf, int32 len);
int32 Board_Mng_RxFrom50Proc(uint8 *buf, int32 len);
int Board_Mng_RegisterZhuanXian(uint8 kPortId, char *phonenum);
int Board_Mng_UnRegisterZhuanXian(uint8 kPortId);
extern int mgw_sw_phone_connect_request(TERMINAL_BASE* terext);
extern int mgw_ext_buzzer_ctl(TERMINAL_BASE* terext);
extern int mgw_sw_phone_callack_request(TERMINAL_BASE* terext);
extern int mgw_sw_phone_release_request(TERMINAL_BASE* terext);
extern int mgw_sw_phone_release_cnf_request(TERMINAL_BASE* terext);
extern int mgw_sw_phone_call_request(TERMINAL_BASE* terext);
extern int mgw_sw_phone_ptt_request(TERMINAL_BASE * terext);
extern int mgw_scu_pf_zwzhuanxian_callack(TERMINAL_BASE* terext);
extern int mgw_scu_ZhuanXian_callack(TERMINAL_BASE* terext);
extern int32 Sip_BoardPhToSipPh(char *pfrom, int32 len, char *pout);
extern int Board_Mng_UnRegister_All_ZhuanXian(void);
extern int Board_Mng_Query_All_ZhuanXian(uint16 IdNum);
extern int32 Check_Control_Board_Timeout(const void *data);




extern const unsigned char DIRECTCALL_RADIO_INDEX[];

#if defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /*_MGW_CALL_CONTROL_H_*/

