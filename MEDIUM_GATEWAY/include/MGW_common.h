#ifndef	_MGW_COMMON_H_
#define	_MGW_COMMON_H_

/*
!\description:
* 公共头文件，包含类型定义及公共函数
*
*/

#if	defined(__cplusplus) || defined(c_plusplus)
extern "C"{
#endif

/*type define*/
#ifndef BYTE
#define	BYTE	unsigned char
#endif

#ifndef WORD
#define	WORD	unsigned short
#endif

#ifndef	DWORD
#define	DWORD	unsigned long
#endif

#ifndef	NULL
#define	NULL	(void *)0
#endif

#ifndef	TRUE
#define	TRUE	1
#endif

#ifndef	FALSE
#define	FALSE	0
#endif



/*common function*/
#define		MGW_MAX(a,b)	((a)<(b)?(b):(a))
#define		MGW_MIN(a,b)	((a)<(b)?(a):(b))
#define		MGW_EQUAL(a,b)	((a) == (b))


#ifdef	DEBUG_INFO
/*
** Usage:
** DEBUG_OUT("USE DEBUG_OUT %d\n",7);//output:USE DEBUG_OUT 7
** VERBOSE_OUT(LOG_SYS,"USE VERBOSE_OUT %d\n",8);//output:EXT_CONTROL.c(34):USE VERBOSE_OUT 8
** use VERBOSE_OUT with param file,line mask the file and line that call VERBOSE_OUT,and
** this is useful to find bug
*/
	#define		DEBUG_OUT	printf
	#define		LINE		__LINE__
	#define		FUNCTION	__PRETTY_FUNCTION__
	#define		LOG_SYS		__FILE__,LINE
void	VERBOSE_OUT(const char*file,int line,const char *fmt,...);
#else
	#define		DEBUG_OUT
	#define		VERBOSE_OUT
	#define		LOG_SYS		""
#endif


/*
** Thread interface:
** use same form create thread use for mangment
*/
#if 0
#define MGW_STACKSIZE (((sizeof(void *) * 8 * 8) - 16) * 1024)

extern int mgw_pthread_create_stack(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), \
			     void *data, size_t stacksize, const char *file, const char *caller, \
			     int line, const char *start_fn);

#define mgw_pthread_create(a, b, c, d) mgw_pthread_create_stack(a, b, c, d,			\
							        0,				\
	 						        __FILE__, __FUNCTION__,		\
 							        __LINE__, #c)

#define mgw_pthread_create_background(a, b, c, d) mgw_pthread_create_stack(a, b, c, d,			\
									   MGW_STACKSIZE,	\
									   __FILE__, __FUNCTION__,	\
									   __LINE__, #c)
#endif

/*const num define*/
#define		IDLE		0xee
#define		IDLEW		0xeeee
#define		IDLEDW		0xeeeeeeeeul

#define MAX_SOCKET_LEN 600


/*==============================global variable===============================*/
extern unsigned long	g_hf_addr;
extern unsigned int 	time_cnt_20;
extern struct config *mgw_cfg;
extern struct config *config_cfg;


/*==============================IO Managment====================================*/
/*I/O Managment*/
#if 0
typedef int (*mgw_io_cb)(int *id, int fd, short events, void *cbdata);

struct io_rec {
	mgw_io_cb callback;		/* What is to be called */
	void *data; 				/* Data to be passed */
	int *id; 					/* ID number */
};

#define GROW_SHRINK_SIZE 512

/* Global variables are now in a struct in order to be
   made threadsafe */
struct io_context {
	/* Poll structure */
	struct pollfd *fds;
	/* Associated I/O records */
	struct io_rec *ior;
	/* First available fd */
	unsigned int fdcnt;
	/* Maximum available fd */
	unsigned int maxfdcnt;
	/* Currently used io callback */
	int current_ioc;
	/* Whether something has been deleted */
	int needshrink;
};
struct io_context *io_context_create(void);
void io_context_destroy(struct io_context *ioc);
int *io_add(struct io_context *ioc, int fd, mgw_io_cb callback, short events, void *data);
int *io_change(struct io_context *ioc, int *id, int fd, mgw_io_cb callback, short events, void *data);
int io_remove(struct io_context *ioc, int *_id);
int io_wait(struct io_context *ioc, int howlong);
void io_dump(struct io_context *ioc);
/*================================================================================*/
/*
** Sched Managment
*
*/

typedef int (*sched_cb)(const void *data);
#define SCHED_CB(a) ((sched_cb)(a))

struct sched;
struct sched_context;

struct sched_context *sched_context_create(void);
void sched_context_destroy(struct sched_context *con);
int sched_wait(struct sched_context *con);
int sched_add(struct sched_context *con, int when, sched_cb callback, const void *data);
int sched_del(struct sched_context *con, int id);
void sched_dump(const struct sched_context *con);
int sched_runq(struct sched_context *con);
#endif
#define WORKMODE_PAOBIN
#ifdef WORKMODE_PAOBIN
	#define SEAT_MNG_PORT 50717
#else
	#define SEAT_MNG_PORT 50730
#endif
#define DISPLAY_BRD_MNG_IP 			"192.168.254.6"
#define DISPLAY_BRD_MNG_LOCAL_IP 		"192.168.254.3"
#define HF_MGW_SCU_SCUDATA_IP  		"192.168.254.2" 
#define HF_MGW_SCU_SCUDATA_PORT		50718

#undef ZHUANXIAN_VOICE_CTRL
//#define ZHUANXIAN_VOICE_CTRL

/*common function*/
int mgw_sys_exec(char * cmd);
void mgw_load_extension(char* filename);
unsigned long find_node_by_callee(char * num);
char* find_extension_by_node(unsigned long node);
/*==================================================================================*/


/*============================udp socket operation==================================*/
typedef	struct
{
	int	udp_handle;
	WORD	local_port;
	const char* local_ip;
	WORD	remote_port;
	const char* remote_ip;
	struct sockaddr_in addr_us;
	socklen_t	addr_len;
	struct sockaddr_in addr_them;
	struct io_context * ioc;	/*保存调用io_context上下文*/
	int*	ioc_id;				/*调用id*/
	char	name[80];
	char	rawdata[EXTMAXDATASIZE];
}UDP_SOCKET;

int		socket_recreate(UDP_SOCKET *sck);
int		socket_new_with_bindaddr(UDP_SOCKET *sck);
int		socket_new(UDP_SOCKET *sck);
int		socket_close(UDP_SOCKET *sck);
/*==================================================================================*/


/*====================lan_bus_control.c && vlan_bus_control.c start=================*/

extern UDP_SOCKET gIpcSemSocket;
//extern UDP_SOCKET scu_js_udp_socket;
extern UDP_SOCKET gIpcDatSocket;
extern UDP_SOCKET ext_udp_socket;
extern UDP_SOCKET ext_rtp_socket;
//extern UDP_SOCKET js_udp_socket;
//extern UDP_SOCKET js_data_udp_socket;
extern int gRpcDbgSocket;
extern int gRpcDisSocket;
extern int32 gRpcSeatSocket;
extern int32 gRpcVoiceSocket;

extern struct sockaddr_in SocketT_DbgIP;	
extern struct sockaddr_in SocketT_DisPlayIP;	
extern struct sockaddr_in Terminal_I_IP;	//指控I型信令集成IP
extern struct sockaddr_in Terminal_I_IPV;	//指控I型话音集成IP

void dump_buff(const unsigned char* buf,int count);
int	lan_bus_ctrl(int *id, int fd, short events, void *cbdata);
int	init_lan_bus(const void *data);
int32 RpcDbg_Socket_init(void);
void RpcDbg_RxThread(void);
int switch_vlan_init(void);


int32 Board_Mng_Process(uint8 *buf, int32 len);
int find_local_port_by_slot(unsigned char slot);

int mgw_scu_ShakeHand(void);
int32 Board_Mng_SendTo_Dis(uint8 *buf, int32 len);
int Board_Mng_ZhuanXianInit(void);
int Board_Mng_ZhuanXianMsgProc(uint8 *buf, int32 len);
int Board_Mng_ZhuanXianAckMsgProc(uint8 *buf, int32 len);

int	init_vlan_bus(const void *data);
/*rtp function*/
void *do_rtp_monitor(void *arg);
int *rtp_io_add(int fd, void *data);
int rtp_io_remove(int *_id);
int	mgw_rtp_read(int *id, int fd, short events, void *cbdata);
unsigned short get_random_port(void);
int Recv_XIWEI_RTP_Packet(char channel,char* Buffer,size_t len);
int Send_XIWEI_RTP_Packet(int channel,char * Buffer,size_t len);
int do_io_dump (cmd_tbl_t *cmdtp, int argc, char *argv[]);
int do_buf_dump (cmd_tbl_t *cmdtp, int argc, char *argv[]);
int do_broadcast_test(cmd_tbl_t * cmdtp, int argc,char * argv [ ]);
/*====================lan_bus_control.c && vlan_bus_control.c end===================*/


/*==========================protocol_parse.c================================*/
/*protocol parse function*/
void protocol_parse(void *pro,size_t count,struct sockaddr_in * addr_src);
int do_show_route(cmd_tbl_t *cmdtp, int argc, char *argv[]);
int do_submit_del_route(cmd_tbl_t *cmdtp, int argc, char *argv[]);
int do_submit_ip_route(cmd_tbl_t *cmdtp, int argc, char *argv[]);
int do_submit_del_ip_route(cmd_tbl_t *cmdtp, int argc, char *argv[]);
int do_query_hf_route(cmd_tbl_t *cmdtp,  int argc, char *argv[]);
int mgw_5s_proc(const void * data);
int mgw_scu_open(const void * data);
int mgw_add_num_addr(unsigned long hf_addr, char* phone);
unsigned long mgw_if_fetch_ip(char* if_name);
unsigned long mgw_if_fetch_mask(char* if_name);
unsigned long mgw_if_fetch_boardaddr(char* if_name);
/*========================protocol_parse.c end==============================*/


void mgw_replace_sigchld(void);

#if	defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif	/*_MGW_COMMON_H_*/

