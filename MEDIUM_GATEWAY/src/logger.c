/*! \file
 *
 * \brief INC_AG Logger
 * 
 * Logging routines
 *
 */
#include "common/autoconfig.h"
//#include "common/compat.h"
	 
#include "common/paths.h"
#include "common/stringfields.h"

#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#if ((defined(INC_DEVMODE)) && (defined(linux)))
#include <execinfo.h>
#define MAX_BACKTRACE_FRAMES 20
#endif

#define SYSLOG_NAMES /* so we can map syslog facilities names to their numeric values,
		        from <syslog.h> which is included by logger.h */
#include <syslog.h>

static int syslog_level_map[] = {
	LOG_DEBUG,
	LOG_INFO,    /* arbitrary equivalent of LOG_EVENT */
	LOG_NOTICE,
	LOG_WARNING,
	LOG_ERR,
	LOG_DEBUG,
	LOG_DEBUG
};

#define SYSLOG_NLEVELS sizeof(syslog_level_map) / sizeof(int)

#include "common/logger.h"
#include "common/lock.h"
//#include "common/options.h"
//#include "common/config.h"
//#include "common/term.h"
#include "common/utils.h"
#include "common/linkedlists.h"
#include "common/threadstorage.h"
#include "PUBLIC.h"

extern int option_verbose;


#if defined(__linux__) && !defined(__NR_gettid)
#include <asm/unistd.h>
#endif

#if defined(__linux__) && defined(__NR_gettid)
#define GETTID() syscall(__NR_gettid)
#else
#define GETTID() getpid()
#endif


static char dateformat[256] = "%T";//"%b %e %T";

static int filesize_reload_needed;
static int global_logmask = -1;

static struct {
	unsigned int queue_log:1;
	unsigned int event_log:1;
} logfiles = { 0, 0 };

static char hostname[MAXHOSTNAMELEN];

extern int glog_level;

enum logtypes {
	LOGTYPE_SYSLOG,
	LOGTYPE_FILE,
	LOGTYPE_CONSOLE,
};

struct logchannel {
	int logmask;			/* What to log to this channel */
	int disabled;			/* If this channel is disabled or not */
	int facility; 			/* syslog facility */
	enum logtypes type;		/* Type of log channel */
	FILE *fileptr;			/* logfile logging file pointer */
	char filename[256];		/* Filename */
	INC_LIST_ENTRY(logchannel) list;
};

static INC_LIST_HEAD_STATIC(logchannels, logchannel);

static FILE *eventlog;
static FILE *qlog;

static char *levels[] = {
	"DEBUG",
	"EVENT",
	"NOTICE",
	"WARNING",
	"ERROR",
	"VERBOSE",
	"DTMF"
};

#if 0
static int colors[] = {
	COLOR_BRGREEN,
	COLOR_BRBLUE,
	COLOR_YELLOW,
	COLOR_BRRED,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_BRGREEN
};
#endif

INC_THREADSTORAGE(verbose_buf, verbose_buf_init);
#define VERBOSE_BUF_INIT_SIZE   128

INC_THREADSTORAGE(log_buf, log_buf_init);
#define LOG_BUF_INIT_SIZE       128








/* 其他模块全局变量引用声明 */
//deg define
#define	MAX_DBG_MAXLINE	5
#define	MAX_DBG_CMDLEN	100

#define	MAX_DBGNAMELEN	20
#define	MAX_DBGPARALEN	80
#define 	I_SENDTO_BASE_PORT 		(33000 + 1)
#define 	I_RECVFROM_BASE_PORT		(34000 + 1)

#define 	QINWU_TIMESLOT 14
#define 	MAX_SOCKET_LEN 600



//static FILE * fp_log = NULL;

//static pthread_mutex_t dbg_lock;
extern void Send_TONE(WORD wport,TONE_TYPE tone_type);
extern void Send_DTMF(WORD wport,char dtmfnum);






//int g_test_printf = 1 << FP_TO_FILE;

int g_Destroy = 0;

uint32 g_dbg_printf = 1;
uint32 g_dbg_monitor = 0;

static int32 DbgLine_Process(uint8 *buf, uint32 len);


uint8 phone_num[10] = "1234567";
uint8 phone_len = 7;
uint8 phone_head = 0;
uint8 phone_start = 0;

#if 0
int32 Send_Dtmf_Phone(void)
{
	if(1 == phone_start)
	{
#if 1	
		if(phone_head < phone_len)
		{
			Send_DTMF(5, phone_num[phone_head]);
			phone_head++;
			
			if(phone_head == phone_len)
			{
				phone_start =0;
				phone_head = 0;
			}
		}
#else
		Send_DTMF(5, '7');
		phone_start =0;

#endif
	}
}
#endif


/* I machine */	
//deg define end
const char *ac491_str[] = 
{
	"OPEN491 HANGOFF!",
	"OPEN491 HANGON!",
	"OPEN491 CLOSE!",
};

const char *codec_tone_str[] = 
{
	"Send_TONE 0 TONE_DIAL",
	"Send_TONE 1 TONE_SILENT",
 	"Send_TONE 2 TONE_BUSY",
 	"Send_TONE 3 TONE_CNF",
	"Send_TONE 4 TONE_RING",
	"Send_TONE 5 TONE_HANG",
	"Send_TONE 6 TONE_STOP",
};

const char *g_socket_all_str[] = {
	"sokcet monitor All open",
	"sokcet monitor All close",
	"sokcet no para input or para is error!"
};

const char *printf_open_str[] = {
	"printf to /tmp/dbg.log",
	"printf to debug serial",
	"printf to param error",
};

const char *printf_close_str[] = {
	"close printf to /tmp/dbg.log",
	"close printf to debug serial",
};


const char *g_socket_open_str[] = {
	"I_50_sem  		socket open 	(33001)",
	"I_50_data 		socket open 	(33002)",
	"I_seat_sem		socket open	(33003)",
	"I_seat_voc 		socket open	(33004)",
	"I_sip_sem     	socket open	(33005)",
	"I_eth 			socket open	(33006)",
	"I_ethD 			socket open	(33007)",
	"I_adapter_sem     socket open	(33008)",
	"I_adapter_voc      socket open	(33009)",
	"I_gun_Yx      	socket open	(33010)",
	"I_gun_Wx     	socket open	(33011)",
	"I_gun_uart    	socket open	(33012)",
	"I_fsk     			socket open	(33013)",
		
	"50_I_sem 		socket open	(34001)",
	"50_I_data 		socket open	(34002)",
	"seat_I_sem  		socket open	(34003)",
	"seat_I_voc     	socket open	(34004)",
	"sip_I_sem 		socket open	(34005)",
	"eth_I      		socket open	(34006)",
	"ethD_I     		socket open	(34007)",
	"adapter_I_sem     socket open	(34008)",
	"adapter_I_voc      socket open	(34009)",	
	"gun_I_Yx     		socket open	(34010)",
	"gun_I_Wx      	socket open	(34011)",	
	"gun_I_uart 		socket open	(34012)",	
	"fsk_I      		socket open	(34013)",	
};

const char *g_socket_close_str[] = {
	"close_I_50_sem  		socket close 	(33001)",
	"close_I_50_data 		socket close 	(33002)",
	"close_I_seat_sem		socket close	(33003)",
	"close_I_seat_voc 		socket close	(33004)",
	"close_I_sip_sem     	socket close	(33005)",
	"close_I_eth 			socket close	(33006)",
	"close_I_ethD 		socket close	(33007)",
	"close_I_adapter_sem   socket close	(33008)",
	"close_I_adapter_voc    socket close	(33009)",	
	"close_I_gun_Yx   		socket close	(33010)",
	"close_I_gun_Wx    	socket close	(33011)",	
	"close_I_gun_uart    	socket close	(33012)",	
	"close_I_fsk    		socket close	(33013)",	
	
	"close_50_I_sem 		socket close	(34001)",
	"close_50_I_data 		socket close	(34002)",
	"close_seat_I_sem  	socket close	(34003)",
	"close_seat_I_voc     	socket close	(34004)",
	"close_sip_I_sem 		socket close	(34005)",
	"close_eth_I      		socket close	(34006)",
	"close_ethD_I     		socket close	(34007)",
	"close_adapter_I_sem   socket close	(34008)",
	"close_adapter_I_voc    socket close	(34009)",	
	"close_gun_I_Yx   		socket close	(34010)",
	"close_gun_I_Wx   	socket close	(34011)",
	"close_gun_I_uart   	socket close	(34012)",	
	"close_fsk_I  			socket close	(34013)",	
};

const char *g_DbgSocket_monitor_str[] = 
{
	"NOTE: Enter is one cmd finish,Cmd and Para use blank compart(ignore upper or low)",
	"support max 5 cmd once;close is close(para)",
	"help -> help debug information",

	"printf screen -> printf to debug serial",
	"printf file     -> printf to /tmp/dbg.log",

	"socket open_all   -> monitor all socket",	
	"socket I_50_sem   -> monitor (port 40000 -> 33001)",
	"socket I_50_data 	-> monitor  (port 9034 -> 33002)",
	"socket I_seat_sem -> monitor (port 30000 -> 33003)",
	"socket I_seat_voc -> monitor  (port 30008 -> 33004)",
	"socket I_sip_sem -> monitor (port 40001 -> 33005)",
	"socket I_eth -> monitor (port 20034 -> 33006)",
	"socket I_ethD -> monitor (port 20035 -> 33007)",
	"socket I_adapter_sem -> monitor (port 30000 -> 33008)",
	"socket I_adapter_voc -> monitor (port 30001 -> 33009)",
	"socket I_gun_Yx -> monitor (port 10000 -> 33010)",
	"socket I_gun_Wx -> monitor (port 20000 -> 33011)",
	"socket I_gun_uart -> monitor (port uart -> 33012)",	
	"socket I_fsk 	-> monitor (port 60051 -> 33013)",
	
	"socket 50_I_sem  -> monitor  (port 40000 -> 34001)",
	"socket 50_I_data  -> monitor  (port 9034 -> 34002)",
	"socket seat_I_sem -> monitor (port 30000 -> 34003)",
	"socket seat_I_voc -> monitor  (port 30008 -> 34004)",
	"socket sip_I_sem -> monitor (port 40001 -> 34005)",
	"socket eth_I -> monitor (port 20034 -> 34006)",
	"socket ethD_I -> monitor (port 20035 -> 34007)",
	"socket adapter_I_sem -> monitor (port 30000 -> 34008)",
	"socket adapter_I_voc -> monitor (port 30001 -> 34009)",
	"socket gun_I_Yx -> monitor (port 10000 -> 34010)",
	"socket gun_I_Wx -> monitor (port 20000 -> 34011)",
	"socket gun_I_uart -> monitor (port uart -> 34012)",
	"socket fsk_I -> monitor (port 60041 -> 34013)",

	"socket close_all   -> close all socket",
	"socket close_I_50_sem   -> close (port 40000 -> 33001)",
	"socket close_I_50_data 	-> close  (port 9034 -> 33002)",
	"socket close_I_seat_sem -> close (port 30000 -> 33003)",
	"socket close_I_seat_voc -> close (port 30008 -> 33004)",
	"socket close_I_sip_sem -> close (port 40001 -> 33005)",
	"socket close_I_eth -> close (port 20034 -> 33006)",
	"socket close_I_ethD -> close (port 20035 -> 33007)",
	"socket close_I_adapter_sem -> close (port 30000 -> 33008)",
	"socket close_I_adapter_voc -> close (port 30001 -> 33009)",
	"socket close_I_gun_Yx -> close (port 10000 -> 33010)",
	"socket close_I_gun_Wx -> close (port 20000 -> 33011)",
	"socket close_I_gun_uart -> close (port uart -> 33012)",
	"socket close_I_fsk  -> close (port 60051 -> 33013)",
		
	"socket close_50_I_sem  -> close  (port 40000 -> 34001)",
	"socket close_50_I_data  -> close  (port 9034 -> 34002)",
	"socket close_seat_I_sem -> close (port 30000 -> 34003)",
	"socket close_seat_I_voc -> close (port 30008 -> 34004)",
	"socket close_sip_I_sem -> close (port 40001 -> 34005)",
	"socket close_eth_I -> close (port 20034 -> 34006)",
	"socket close_ethD_I -> close (port 20035 -> 340007)",
	"socket close_adapter_I_sem -> close (port 30000 -> 34008)",
	"socket close_adapter_I_voc -> close (port 30001 -> 34009)",
	"socket close_gun_I_Yx -> close (port 10000 -> 34010)",
	"socket close_gun_I_Wx -> close (port 20000 -> 34011)",
	"socket close_gun_I_uart -> close (port uart -> 34012)",
	"socket close_fsk_I -> close (port 60041 -> 34013)",
	
	"send_tone dial	-> Send_TONE 0 TONE_DIAL",
	"send_tone silent	-> Send_TONE 1 TONE_SILENT",
	"send_tone busy	-> Send_TONE 2 TONE_BUSY",
	"send_tone cnf	-> Send_TONE 3 TONE_CNF",
	"send_tone ring	-> Send_TONE 4 TONE_RING",
	"send_tone hang	-> Send_TONE 5 TONE_HANG",
	"send_tone stop	-> Send_TONE 6 TONE_STOP",	

	"open491 hangoff",
	"open491 hangon",
};

/* 其他模块函数引用声明 */


#if 0
int32 log_printf(int level, const char *file, const char *function, int line, const char *fmt, ...)
{
	pthread_mutex_lock(&dbg_lock);
	va_list ap;	

	if (g_dbg_printf & (1 << FP_TO_FILE))
	{
		if (fp_log == NULL)	
		{
			pthread_mutex_unlock(&dbg_lock);
			return DRV_ERR;
		}
		
		if (level == 	__DBG_LOGF)
		{
			//fprintf(fp_log, "%s(%s) line:%d :\n", file, function, line);
			fprintf(fp_log, "%s : ", function);
		}
		va_start(ap, fmt);	
		vfprintf(fp_log, fmt, ap);
		fflush(fp_log);

		va_end(ap);	
	}

	/* Modified by lishibing 20130327, 调试信息输出到屏幕时，发生段错误
	    增加:va_start和va_end保护。
	*/
	if (g_dbg_printf & (1 << FP_TO_SCREEN))
	{
		printf ("%s : ", function);
		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
	}
	
	pthread_mutex_unlock(&dbg_lock);

	return DRV_OK;
	
}
#endif

#define ESC 0x1b

static char *term_strip(char *outbuf, char *inbuf, int maxout)
{
	char *outbuf_ptr = outbuf, *inbuf_ptr = inbuf;

	while (outbuf_ptr < outbuf + maxout) {
		switch (*inbuf_ptr) {
			case ESC:
				while (*inbuf_ptr && (*inbuf_ptr != 'm'))
					inbuf_ptr++;
				break;
			default:
				*outbuf_ptr = *inbuf_ptr;
				outbuf_ptr++;
				break;
		}
		if (! *inbuf_ptr)
			break;
		inbuf_ptr++;
	}
	return outbuf;
}

/* filter escape sequences */
static void term_filter_escapes(char *line)
{
	int i;
	int len = strlen(line);

	for (i = 0; i < len; i++) {
		if (line[i] != ESC)
			continue;
		if ((i < (len - 2)) &&
		    (line[i + 1] == 0x5B)) {
			switch (line[i + 2]) {
		 	case 0x30:
			case 0x31:
			case 0x33:
				continue;
			}
		}
		/* replace ESC with a space */
		line[i] = ' ';
	}
}


//****************************************************************************
//****************************************************************************
//user//ignore upper or low//cmd and para user blank compart
//cmd  para [Enter]
//cmd  para [Enter]
//void DbgPara_Process(uint8 *buf, int len, struct sockaddr_in *pdest)
int32 DbgPara_Save(void)
{

	//change_config_cat_var_val(zktxb_cfg, DBG_CFG, "dbg_monitor", g_dbg_monitor);

	//change_config_cat_var_val(zktxb_cfg, DBG_CFG, "dbg_printf", g_dbg_printf);

	//g_rw_cfg = 1;
	
	return DRV_OK;
}

extern int Dbg_Socket_SendToPc(int id, unsigned char *buf, int len)
{
	if(!(g_dbg_monitor >> id & 0x01))
	{
		return DRV_OK;
	}

	struct sockaddr_in Send;
	memset(&Send, 0x00, sizeof(struct sockaddr_in));

	Send.sin_family = AF_INET;
	Send.sin_addr.s_addr = htonl(SocketT_DbgIP.sin_addr.s_addr);	
	
	switch(id)
	{	
		/* I sendto */
		case MONITOR_I_50_SEM_BIT:
			Send.sin_port = htons(I_SENDTO_BASE_PORT+ MONITOR_I_50_SEM_BIT);
		 	break;
		case MONITOR_I_50_DATA_BIT:
			Send.sin_port = htons(I_SENDTO_BASE_PORT+ MONITOR_I_50_DATA_BIT);
		 	break;
		case MONITOR_I_SEAT_SEM_BIT:
			Send.sin_port = htons(I_SENDTO_BASE_PORT+ MONITOR_I_SEAT_SEM_BIT);
		 	break;
		case MONITOR_I_SEAT_VOC_BIT:
			Send.sin_port = htons(I_SENDTO_BASE_PORT+ MONITOR_I_SEAT_VOC_BIT);
		 	break;
		case MONITOR_I_SIP_SEM_BIT:
			Send.sin_port = htons(I_SENDTO_BASE_PORT+ MONITOR_I_SIP_SEM_BIT);
		 	break;
		case MONITOR_I_ETH_BIT:
			Send.sin_port = htons(I_SENDTO_BASE_PORT+ MONITOR_I_ETH_BIT);
		 	break;
		case MONITOR_I_ETHD_BIT:
			Send.sin_port = htons(I_SENDTO_BASE_PORT+ MONITOR_I_ETHD_BIT);
		 	break;
		case MONITOR_I_ADPT_SEM_BIT:
			Send.sin_port = htons(I_SENDTO_BASE_PORT+ MONITOR_I_ADPT_SEM_BIT);
		 	break;
		case MONITOR_I_ADPT_VOC_BIT:
			Send.sin_port = htons(I_SENDTO_BASE_PORT+ MONITOR_I_ADPT_VOC_BIT);
		 	break;
		case MONITOR_I_GUN_YX_BIT:
			Send.sin_port = htons(I_SENDTO_BASE_PORT+ MONITOR_I_GUN_YX_BIT);
		 	break;
		case MONITOR_I_GUN_WX_BIT:
			Send.sin_port = htons(I_SENDTO_BASE_PORT+ MONITOR_I_GUN_WX_BIT);
		 	break;
		case MONITOR_I_GUN_UART_BIT:
			Send.sin_port = htons(I_SENDTO_BASE_PORT+ MONITOR_I_GUN_UART_BIT);
		 	break;
		case MONITOR_I_FSK_BIT:
			Send.sin_port = htons(I_SENDTO_BASE_PORT+ MONITOR_I_FSK_BIT);
		 	break;
		 	
			
		/* sendto I */
		case MONITOR_50_I_SEM_BIT:
			Send.sin_port = htons(I_RECVFROM_BASE_PORT + 0);
		 	break;
		case MONITOR_50_I_DATA_BIT:
			Send.sin_port = htons(I_RECVFROM_BASE_PORT + 1);
		 	break;
		case MONITOR_SEAT_I_SEM_BIT:
			Send.sin_port = htons(I_RECVFROM_BASE_PORT + 2);
		 	break;
		case MONITOR_SEAT_I_VOC_BIT:
			Send.sin_port = htons(I_RECVFROM_BASE_PORT + 3);
		 	break;
		case MONITOR_SIP_I_SEM_BIT:
			Send.sin_port = htons(I_RECVFROM_BASE_PORT + 4);
		 	break;
		case MONITOR_ETH_I_BIT:
			Send.sin_port = htons(I_RECVFROM_BASE_PORT + 5);
		 	break;
		case MONITOR_ETHD_I_BIT:
			Send.sin_port = htons(I_RECVFROM_BASE_PORT + 6);
			break;
		case MONITOR_ADPT_I_SEM_BIT:
			Send.sin_port = htons(I_RECVFROM_BASE_PORT + 7);
		 	break;
		case MONITOR_ADPT_I_VOC_BIT:
			Send.sin_port = htons(I_RECVFROM_BASE_PORT + 8);
		 	break;
		case MONITOR_GUN_I_YX_BIT:
			Send.sin_port = htons(I_RECVFROM_BASE_PORT + 9);
		 	break;
		case MONITOR_GUN_I_WX_BIT:
			Send.sin_port = htons(I_RECVFROM_BASE_PORT + 10);
			break;
		case MONITOR_GUN_I_UART_BIT:
			Send.sin_port = htons(I_RECVFROM_BASE_PORT + 11);
			break;
		case MONITOR_FSK_I_BIT:
			Send.sin_port = htons(I_RECVFROM_BASE_PORT + 12);
			break;
		default:
			return DRV_ERR;

	}

	Socket_Send(gRpcDbgSocket, &Send, buf, len); 	

	return DRV_OK;

}

static int32 DbgLine_Process(uint8 *buf, uint32 len)
{
	uint8 *cur = NULL, *c = NULL;
	uint8 b_st = DBG_START;
	char Name[MAX_DBGNAMELEN] = {0};
	char Para[MAX_DBGPARALEN] = {0};
	int8 flg_error = FALSE;
	uint8 i = 0;	
	
	cur = buf;
	c = buf;
	while (*c != '\0')
	{
		switch (b_st)
		{
			case DBG_START:
				if (*c == ' ')
				{
					c++;
				}
				else
				{					
					cur = c;
					b_st = DBG_NAME;
				}
			break;
			case DBG_NAME:
				if (*c == ' ')
				{
					*c = '\0';
					if ((c-cur+1) >= MAX_DBGNAMELEN)
					{
						flg_error = TRUE;
						break;
					}
					memcpy (Name,cur,(c-cur+1));
					cur = c + 1;
					b_st = DBG_PARA;					
				}
				c++;
				break;
			break;
			case DBG_PARA:
				if (*c == ' ')
				{
					memcpy (c,c+1,(MAX_DBGPARALEN-((c+1-cur)+1)));					
				}				
				c++;
			break;
			default:
				return DRV_ERR;
			break;
		}
		if (flg_error == TRUE)
		{
			//log_printf(DBG_LOGF,"Dbg para is error\n");
			printf("Dbg para is error\n");
			break;
		}
	}
	
	if (flg_error != TRUE)
	{
		if (b_st == DBG_NAME)//添加无参数处理
		{
			if ((c-cur+1) > MAX_DBGNAMELEN)
			{
				printf("Dbg paraname is error\n");
			}
			else
			{
				memcpy (Name,cur,(c-cur+1));
			}
		}
		else if (b_st == DBG_PARA)
		{
			if ((c-cur+1) > MAX_DBGPARALEN)
			{
				printf("Dbg paralen is error\n");
			}
			else
			{
				memcpy (Para,cur,(c-cur+1));
			}
		}
		
		if (strcasecmp(Name,"g_dbg_monitor") == 0)
		{
			g_dbg_monitor = atoi(Para);
			printf("g_dbg_monitor=%d\r\n",g_dbg_monitor);
		}
		else if (strcasecmp(Name,"g_dbg_printf") == 0)
		{
			if(atoi(Para) > 3)
			{
				printf("g_dbg_printf  atoi(Para)= %d\r\n", atoi(Para));

				return DRV_ERR;
			}
			
			g_dbg_printf = atoi(Para);
			printf("g_dbg_printf=%d\r\n",g_dbg_printf);
		}
		else if (strcasecmp(Name,"Destroy") == 0)
		{
			g_Destroy = atoi(Para);
			printf("g_Destroy=%d\r\n",g_Destroy);
		}
		else if (strcasecmp(Name,"HELP") == 0)
		{			
			const uint8  b_time_data[] = "ZK_I "__DATE__ "--" __TIME__ "\n";
			Dbg_Socket_Send((uint8 *)b_time_data, sizeof(b_time_data));
			for (i=0; i<sizeof(g_DbgSocket_monitor_str)/sizeof(char *); i++)
			{
				Dbg_Socket_Send((uint8 *)g_DbgSocket_monitor_str[i], strlen(g_DbgSocket_monitor_str[i]));
			}
		}
		else if (strcasecmp(Name,"socket") == 0)
		{
			//open all
			if (strcasecmp(Para,"open_all") == 0)
			{
				g_dbg_monitor = 65535;
				Dbg_Socket_Send((uint8 *)g_socket_all_str[SOCKSET_OPEN_ALL],\
							strlen(g_socket_all_str[SOCKSET_OPEN_ALL]));
			}
			/* I sendto */
			else if (strcasecmp(Para,"I_50_sem") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_I_50_SEM_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_I_50_SEM_BIT], \
							strlen(g_socket_open_str[MONITOR_I_50_SEM_BIT]));
			}
			else if (strcasecmp(Para,"I_50_data") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_I_50_DATA_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_I_50_DATA_BIT], \
							strlen(g_socket_open_str[MONITOR_I_50_DATA_BIT]));
			}
			else if (strcasecmp(Para,"I_seat_sem") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_I_SEAT_SEM_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_I_SEAT_SEM_BIT], \
							strlen(g_socket_open_str[MONITOR_I_SEAT_SEM_BIT]));
			}
			else if (strcasecmp(Para,"I_seat_voc") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_I_SEAT_VOC_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_I_SEAT_VOC_BIT], \
							strlen(g_socket_open_str[MONITOR_I_SEAT_VOC_BIT]));
			}
			else if (strcasecmp(Para,"I_sip_sem") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_I_SIP_SEM_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_I_SIP_SEM_BIT], \
							strlen(g_socket_open_str[MONITOR_I_SIP_SEM_BIT]));
			}
			else if (strcasecmp(Para,"I_eth") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_I_ETH_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_I_ETH_BIT], \
							strlen(g_socket_open_str[MONITOR_I_ETH_BIT]));
			}
			else if (strcasecmp(Para,"I_ethD") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_I_ETHD_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_I_ETHD_BIT], \
							strlen(g_socket_open_str[MONITOR_I_ETHD_BIT]));
			}
			else if (strcasecmp(Para,"I_adapter_sem") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_I_ADPT_SEM_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_I_ADPT_SEM_BIT], \
							strlen(g_socket_open_str[MONITOR_I_ADPT_SEM_BIT]));
			}
			else if (strcasecmp(Para,"I_adapter_voc") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_I_ADPT_VOC_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_I_ADPT_VOC_BIT], \
							strlen(g_socket_open_str[MONITOR_I_ADPT_VOC_BIT]));
			}
			else if (strcasecmp(Para,"I_gun_Yx") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_I_GUN_YX_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_I_GUN_YX_BIT], \
							strlen(g_socket_open_str[MONITOR_I_GUN_YX_BIT]));
			}
			else if (strcasecmp(Para,"I_gun_Wx") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_I_GUN_WX_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_I_GUN_WX_BIT], \
							strlen(g_socket_open_str[MONITOR_I_GUN_WX_BIT]));
			}
			else if (strcasecmp(Para,"I_gun_uart") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_I_GUN_UART_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_I_GUN_UART_BIT], \
							strlen(g_socket_open_str[MONITOR_I_GUN_UART_BIT]));
			}
			else if (strcasecmp(Para,"I_fsk") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_I_FSK_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_I_FSK_BIT], \
							strlen(g_socket_open_str[MONITOR_I_FSK_BIT]));
			}
			
			/*sendto I*/
			else if (strcasecmp(Para,"50_I_sem") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_50_I_SEM_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_50_I_SEM_BIT], \
							strlen(g_socket_open_str[MONITOR_50_I_SEM_BIT]));
			}
			else if (strcasecmp(Para,"50_I_data") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_50_I_DATA_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_50_I_DATA_BIT], \
							strlen(g_socket_open_str[MONITOR_50_I_DATA_BIT]));
			}
			else if (strcasecmp(Para,"seat_I_sem") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_SEAT_I_SEM_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_SEAT_I_SEM_BIT], \
							strlen(g_socket_open_str[MONITOR_SEAT_I_SEM_BIT]));
			}
			else if (strcasecmp(Para,"seat_I_voc") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_SEAT_I_VOC_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_SEAT_I_VOC_BIT], \
							strlen(g_socket_open_str[MONITOR_SEAT_I_VOC_BIT]));
			}
			else if (strcasecmp(Para,"sip_I_sem") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_SIP_I_SEM_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_SIP_I_SEM_BIT], \
							strlen(g_socket_open_str[MONITOR_SIP_I_SEM_BIT]));
			}
			else if (strcasecmp(Para,"eth_I") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_ETH_I_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_ETH_I_BIT], \
							strlen(g_socket_open_str[MONITOR_ETH_I_BIT]));
			}
			else if (strcasecmp(Para,"ethD_I") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_ETHD_I_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_ETHD_I_BIT], \
							strlen(g_socket_open_str[MONITOR_ETHD_I_BIT]));
			}
			else if (strcasecmp(Para,"adapter_I_sem") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_ADPT_I_SEM_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_ADPT_I_SEM_BIT], \
							strlen(g_socket_open_str[MONITOR_ADPT_I_SEM_BIT]));
			}
			else if (strcasecmp(Para,"adapter_I_voc") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_ADPT_I_VOC_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_ADPT_I_VOC_BIT], \
							strlen(g_socket_open_str[MONITOR_ADPT_I_VOC_BIT]));
			}
			else if (strcasecmp(Para,"gun_I_Yx") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_GUN_I_YX_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_GUN_I_YX_BIT], \
							strlen(g_socket_open_str[MONITOR_GUN_I_YX_BIT]));
			}
			else if (strcasecmp(Para,"gun_I_Wx") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_GUN_I_WX_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_GUN_I_WX_BIT], \
							strlen(g_socket_open_str[MONITOR_GUN_I_WX_BIT]));
			}
			else if (strcasecmp(Para,"gun_I_uart") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_GUN_I_UART_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_GUN_I_UART_BIT], \
							strlen(g_socket_open_str[MONITOR_GUN_I_UART_BIT]));
			}
			else if (strcasecmp(Para,"fsk_I") == 0)
			{
				g_dbg_monitor |= 1 << MONITOR_FSK_I_BIT;						
				Dbg_Socket_Send((uint8 *)g_socket_open_str[MONITOR_FSK_I_BIT], \
							strlen(g_socket_open_str[MONITOR_FSK_I_BIT]));
			}

			/* close all socket*/
			else if (strcasecmp(Para, "close_all") == 0)
			{
				g_dbg_monitor = 0;
				Dbg_Socket_Send((uint8 *)g_socket_all_str[SOCKSET_CLOSE_ALL],\
							strlen(g_socket_all_str[SOCKSET_CLOSE_ALL]));
			}
			/* I sendto */
			else if (strcasecmp(Para, "close_I_50_sem") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_I_50_SEM_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_I_50_SEM_BIT], \
							strlen(g_socket_close_str[MONITOR_I_50_SEM_BIT]));
			}
			else if (strcasecmp(Para, "close_I_50_data") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_I_50_DATA_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_I_50_DATA_BIT], \
							strlen(g_socket_close_str[MONITOR_I_50_DATA_BIT]));
			}
			else if (strcasecmp(Para, "close_I_seat_sem") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_I_SEAT_SEM_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_I_SEAT_SEM_BIT], \
							strlen(g_socket_close_str[MONITOR_I_SEAT_SEM_BIT]));
			}
			else if (strcasecmp(Para, "close_I_seat_voc") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_I_SEAT_VOC_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_I_SEAT_VOC_BIT], \
							strlen(g_socket_close_str[MONITOR_I_SEAT_VOC_BIT]));
			}
			else if (strcasecmp(Para, "close_I_sip_sem") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_I_SIP_SEM_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_I_SIP_SEM_BIT], \
							strlen(g_socket_close_str[MONITOR_I_SIP_SEM_BIT]));
			}
			else if (strcasecmp(Para, "close_I_eth") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_I_ETH_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_I_ETH_BIT], \
							strlen(g_socket_close_str[MONITOR_I_ETH_BIT]));
			}
			else if (strcasecmp(Para, "close_I_ethD") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_I_ETHD_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_I_ETHD_BIT], \
							strlen(g_socket_close_str[MONITOR_I_ETHD_BIT]));
			}
			else if (strcasecmp(Para, "close_I_adapter_sem") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_I_ADPT_SEM_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_I_ADPT_SEM_BIT], \
							strlen(g_socket_close_str[MONITOR_I_ADPT_SEM_BIT]));
			}
			else if (strcasecmp(Para, "close_I_adapter_voc") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_I_ADPT_VOC_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_I_ADPT_VOC_BIT], \
							strlen(g_socket_close_str[MONITOR_I_ADPT_VOC_BIT]));
			}
			else if (strcasecmp(Para, "close_I_gun_Yx") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_I_GUN_YX_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_I_GUN_YX_BIT], \
							strlen(g_socket_close_str[MONITOR_I_GUN_YX_BIT]));
			}
			else if (strcasecmp(Para, "close_I_gun_Wx") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_I_GUN_WX_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_I_GUN_WX_BIT], \
							strlen(g_socket_close_str[MONITOR_I_GUN_WX_BIT]));
			}
			else if (strcasecmp(Para, "close_I_gun_uart") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_I_GUN_UART_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_I_GUN_UART_BIT], \
							strlen(g_socket_close_str[MONITOR_I_GUN_UART_BIT]));
			}
			else if (strcasecmp(Para, "close_I_fsk") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_I_FSK_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_I_FSK_BIT], \
							strlen(g_socket_close_str[MONITOR_I_FSK_BIT]));
			}
			
			/*sendto I*/
			else if (strcasecmp(Para, "close_50_I_sem") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_50_I_SEM_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_50_I_SEM_BIT], \
							strlen(g_socket_close_str[MONITOR_50_I_SEM_BIT]));
			}
			else if (strcasecmp(Para, "close_50_I_data") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_50_I_DATA_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_50_I_DATA_BIT], \
							strlen(g_socket_close_str[MONITOR_50_I_DATA_BIT]));
			}
			else if (strcasecmp(Para, "close_seat_I_sem") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_SEAT_I_SEM_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_SEAT_I_SEM_BIT], \
							strlen(g_socket_close_str[MONITOR_SEAT_I_SEM_BIT]));
			}
			else if (strcasecmp(Para, "close_seat_I_voc") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_SEAT_I_VOC_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_SEAT_I_VOC_BIT], \
							strlen(g_socket_close_str[MONITOR_SEAT_I_VOC_BIT]));
			}
			else if (strcasecmp(Para, "close_sip_I_sem") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_SIP_I_SEM_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_SIP_I_SEM_BIT], \
							strlen(g_socket_close_str[MONITOR_SIP_I_SEM_BIT]));
			}
			else if (strcasecmp(Para, "close_eth_I") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_ETH_I_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_ETH_I_BIT], \
							strlen(g_socket_close_str[MONITOR_ETH_I_BIT]));
			}
			else if (strcasecmp(Para, "close_ethD_I") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_ETHD_I_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_ETHD_I_BIT], \
							strlen(g_socket_close_str[MONITOR_ETHD_I_BIT]));
			}
			else if (strcasecmp(Para, "close_adapter_I_sem") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_ADPT_I_SEM_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_ADPT_I_SEM_BIT], \
							strlen(g_socket_close_str[MONITOR_ADPT_I_SEM_BIT]));
			}
			else if (strcasecmp(Para, "close_adapter_I_voc") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_ADPT_I_VOC_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_ADPT_I_VOC_BIT], \
							strlen(g_socket_close_str[MONITOR_ADPT_I_VOC_BIT]));
			}
			else if (strcasecmp(Para, "close_gun_I_Yx") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_GUN_I_YX_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_GUN_I_YX_BIT], \
							strlen(g_socket_close_str[MONITOR_GUN_I_YX_BIT]));
			}
			else if (strcasecmp(Para, "close_gun_I_Wx") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_GUN_I_WX_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_GUN_I_WX_BIT], \
							strlen(g_socket_close_str[MONITOR_GUN_I_WX_BIT]));
			}
			else if (strcasecmp(Para, "close_gun_I_uart") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_GUN_I_UART_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_GUN_I_UART_BIT], \
							strlen(g_socket_close_str[MONITOR_GUN_I_UART_BIT]));
			}
			else if (strcasecmp(Para, "close_fsk_I") == 0)
			{
				g_dbg_monitor &= ~(1 << MONITOR_FSK_I_BIT);						
				Dbg_Socket_Send((uint8 *)g_socket_close_str[MONITOR_FSK_I_BIT], \
							strlen(g_socket_close_str[MONITOR_FSK_I_BIT]));
			}
			//no para
			else
			{
				Dbg_Socket_Send((uint8 *)g_socket_all_str[SOCKSET_PARAM_ERROR],\
							strlen(g_socket_all_str[SOCKSET_PARAM_ERROR]));
				return DRV_ERR;
			}
			
			DbgPara_Save();
		}
		else if (strcasecmp(Name,"printf") == 0)
		{
			//open
			if (strcasecmp(Para,"file") == 0)
			{
				g_dbg_printf |= (1 << FP_TO_FILE);
				Dbg_Socket_Send((uint8 *) (uint8 *)printf_open_str[FP_TO_FILE], \
							strlen(printf_open_str[FP_TO_FILE]));
			}
			else if (strcasecmp(Para,"screen") == 0)
			{
				g_dbg_printf |= (1 << FP_TO_SCREEN);
				Dbg_Socket_Send((uint8 *)(uint8 *)printf_open_str[FP_TO_SCREEN], \
							strlen(printf_open_str[FP_TO_SCREEN]));
			}
			//close
			else if (strcasecmp(Para,"close_file") == 0)
			{
				g_dbg_printf &= ~(1 << FP_TO_FILE);
				Dbg_Socket_Send((uint8 *)(uint8 *)printf_close_str[FP_TO_FILE], \
							strlen(printf_close_str[FP_TO_FILE]));
			}
			else if (strcasecmp(Para,"close_screen") == 0)
			{
				g_dbg_printf &= ~(1 << FP_TO_SCREEN);
				Dbg_Socket_Send((uint8 *)(uint8 *)printf_close_str[FP_TO_SCREEN], \
							strlen(printf_close_str[FP_TO_SCREEN]));
			}
			//no para
			else
			{
				Dbg_Socket_Send((uint8 *)(uint8 *)printf_open_str[FP_TO_PARAM_ERROR], \
							strlen(printf_open_str[FP_TO_PARAM_ERROR]));
				return DRV_ERR;
			}
			
			DbgPara_Save();
		}
		else if (strcasecmp(Name,"send_tone") == 0)
		{
			if (strcasecmp(Para,"dial") == 0)
			{
				Send_TONE(QINWU_TIMESLOT, CODEC_DIAL);
				Send_TONE(5, CODEC_DIAL);
				Dbg_Socket_Send((uint8 *)(uint8 *)codec_tone_str[CODEC_DIAL], \
					strlen(codec_tone_str[CODEC_DIAL]));	
			}
			else if (strcasecmp(Para,"silent") == 0)
			{
				//Send_TONE(QINWU_TIMESLOT, CODEC_SILENT);
				//Send_TONE(5, CODEC_SILENT);
				//Send_DTMF(5, "12345");
				phone_start = 1;
				Dbg_Socket_Send((uint8 *)(uint8 *)codec_tone_str[CODEC_SILENT], \
					strlen(codec_tone_str[CODEC_SILENT]));
			}
			else if (strcasecmp(Para,"busy") == 0)
			{
				Send_TONE(QINWU_TIMESLOT, CODEC_BUSY);
				Send_TONE(5, CODEC_BUSY);
				Dbg_Socket_Send((uint8 *)(uint8 *)codec_tone_str[CODEC_BUSY], \
					strlen(codec_tone_str[CODEC_BUSY]));
			}
			else if (strcasecmp(Para,"cnf") == 0)
			{
				Send_TONE(QINWU_TIMESLOT, CODEC_CNF);
				Send_TONE(5, CODEC_CNF);
				Dbg_Socket_Send((uint8 *)(uint8 *)codec_tone_str[CODEC_CNF], \
					strlen(codec_tone_str[CODEC_CNF]));
			}
			else if (strcasecmp(Para,"ring") == 0)
			{
				Send_TONE(QINWU_TIMESLOT, CODEC_RING);
				Send_TONE(5, CODEC_RING);
				//Ring_Tone();
				Dbg_Socket_Send((uint8 *)(uint8 *)codec_tone_str[CODEC_RING], \
					strlen(codec_tone_str[CODEC_RING]));
					
			}
			else if (strcasecmp(Para,"hang") == 0)
			{
				Send_TONE(QINWU_TIMESLOT, CODEC_HANG);
				Send_TONE(5, CODEC_HANG);
				Dbg_Socket_Send((uint8 *)(uint8 *)codec_tone_str[CODEC_HANG], \
					strlen(codec_tone_str[CODEC_HANG]));
			}
			else// if (strcasecmp(Para,"TONE_STOP") == 0)
			{
				Send_TONE(QINWU_TIMESLOT, CODEC_STOP);
				Send_TONE(5, CODEC_STOP);
				Dbg_Socket_Send((uint8 *)(uint8 *)codec_tone_str[CODEC_STOP], \
					strlen(codec_tone_str[CODEC_STOP]));
			}
		}
	}
	return DRV_OK;
}


extern int DbgPara_Process(unsigned char *buf, int len)
{		
	char b_pbuf[MAX_SOCKET_LEN+1];
	char CmdLine[MAX_DBG_MAXLINE][MAX_DBG_CMDLEN];
	char *pcur,*c;
	uint8 i;
	uint8 b_line = 0;

	if ((len>MAX_SOCKET_LEN)||(buf==NULL))	
	{
		return DRV_ERR;
	}
	
	memcpy (b_pbuf, buf,len);
	b_pbuf[len] = '\0';

	pcur = b_pbuf;
	while((c=strchr(pcur, '\r')) != NULL)
	{
		memcpy (c,c+1,(len+1-(c+1-b_pbuf)));
		pcur = c;
	}	
	
	pcur = b_pbuf;
	while((c=strchr(pcur, '\n')) != NULL)
	{
		*c = '\0';
		if ((c-pcur+1) > MAX_DBG_CMDLEN)
		{
			printf("Cmd line len > 100.\n");
		}
		else
		{
			memcpy (CmdLine[b_line], pcur, (c-pcur+1));
			b_line++;
			if (b_line >= MAX_DBG_MAXLINE)
			{
				break;
			}			
		}
		pcur = c + 1;
	}

	for (i=0; i<b_line; i++)
	{
		DbgLine_Process((uint8 *)CmdLine[i], 0);
	}

	return DRV_OK;
}

static int make_components(char *s, int lineno)
{
	char *w;
	int res = 0;
	char *stringp = s;

	while ((w = strsep(&stringp, ","))) {
		w = inc_skip_blanks(w);
		if (!strcasecmp(w, "error")) 
			res |= (1 << __LOG_ERROR);
		else if (!strcasecmp(w, "warning"))
			res |= (1 << __LOG_WARNING);
		else if (!strcasecmp(w, "notice"))
			res |= (1 << __LOG_NOTICE);
		else if (!strcasecmp(w, "event"))
			res |= (1 << __LOG_EVENT);
		else if (!strcasecmp(w, "debug"))
			res |= (1 << __LOG_DEBUG);
		else if (!strcasecmp(w, "verbose"))
			res |= (1 << __LOG_VERBOSE);
		else if (!strcasecmp(w, "dtmf"))
			res |= (1 << __LOG_DTMF);
		else {
			fprintf(stderr, "Logfile Warning: Unknown keyword '%s' at line %d of .conf\n", w, lineno);
		}
	}

	return res;
}

struct logchannel *make_logchannel(char *channel, char *components, int lineno)
{
	struct logchannel *chan;
	char *facility;
#ifndef SOLARIS
	CODE *cptr;
#endif

	if (inc_strlen_zero(channel) || !(chan = inc_calloc(1, sizeof(*chan))))
		return NULL;

	if (!strcasecmp(channel, "console")) {
		chan->type = LOGTYPE_CONSOLE;
	} else if (!strncasecmp(channel, "syslog", 6)) {
		/*
		* syntax is:
		*  syslog.facility => level,level,level
		*/
		facility = strchr(channel, '.');
		if(!facility++ || !facility) {
			facility = "local0";
		}

#ifndef SOLARIS
		/*
 		* Walk through the list of facilitynames (defined in sys/syslog.h)
		* to see if we can find the one we have been given
		*/
		chan->facility = -1;
 		cptr = facilitynames;
		while (cptr->c_name) {
			if (!strcasecmp(facility, cptr->c_name)) {
		 		chan->facility = cptr->c_val;
				break;
			}
			cptr++;
		}
#else
		chan->facility = -1;
		if (!strcasecmp(facility, "kern")) 
			chan->facility = LOG_KERN;
		else if (!strcasecmp(facility, "USER")) 
			chan->facility = LOG_USER;
		else if (!strcasecmp(facility, "MAIL")) 
			chan->facility = LOG_MAIL;
		else if (!strcasecmp(facility, "DAEMON")) 
			chan->facility = LOG_DAEMON;
		else if (!strcasecmp(facility, "AUTH")) 
			chan->facility = LOG_AUTH;
		else if (!strcasecmp(facility, "SYSLOG")) 
			chan->facility = LOG_SYSLOG;
		else if (!strcasecmp(facility, "LPR")) 
			chan->facility = LOG_LPR;
		else if (!strcasecmp(facility, "NEWS")) 
			chan->facility = LOG_NEWS;
		else if (!strcasecmp(facility, "UUCP")) 
			chan->facility = LOG_UUCP;
		else if (!strcasecmp(facility, "CRON")) 
			chan->facility = LOG_CRON;
		else if (!strcasecmp(facility, "LOCAL0")) 
			chan->facility = LOG_LOCAL0;
		else if (!strcasecmp(facility, "LOCAL1")) 
			chan->facility = LOG_LOCAL1;
		else if (!strcasecmp(facility, "LOCAL2")) 
			chan->facility = LOG_LOCAL2;
		else if (!strcasecmp(facility, "LOCAL3")) 
			chan->facility = LOG_LOCAL3;
		else if (!strcasecmp(facility, "LOCAL4")) 
			chan->facility = LOG_LOCAL4;
		else if (!strcasecmp(facility, "LOCAL5")) 
			chan->facility = LOG_LOCAL5;
		else if (!strcasecmp(facility, "LOCAL6")) 
			chan->facility = LOG_LOCAL6;
		else if (!strcasecmp(facility, "LOCAL7")) 
			chan->facility = LOG_LOCAL7;
#endif /* Solaris */

		if (0 > chan->facility) {
			fprintf(stderr, "Logger Warning: bad syslog facility in logger.conf\n");
			free(chan);
			return NULL;
		}

		chan->type = LOGTYPE_SYSLOG;
		snprintf(chan->filename, sizeof(chan->filename), "%s", channel);
		openlog("inc_cc", LOG_PID, chan->facility);
	} else {
		if (!inc_strlen_zero(hostname)) {
			snprintf(chan->filename, sizeof(chan->filename), "%s/%s.%s.%s",
				 channel[0] != '/' ? inc_config_log_dir : "", channel, system_name, hostname);
		} else {
			snprintf(chan->filename, sizeof(chan->filename), "%s/%s.%s",
				 channel[0] != '/' ? inc_config_log_dir : "", channel, system_name);
		}
		chan->fileptr = fopen(chan->filename, "a");
		if (!chan->fileptr) {
			/* Can't log here, since we're called with a lock */
			fprintf(stderr, "Logger Warning: Unable to open log file '%s': %s\n", chan->filename, strerror(errno));
		} 
		chan->type = LOGTYPE_FILE;
	}
	chan->logmask = make_components(components, lineno);
	return chan;
}

static int32 init_logger_chain(void)
{
	struct logchannel *chan;
//	struct inc_variable *var;
//	const char *s;

	/* delete our list of log channels */
	INC_LIST_LOCK(&logchannels);
	while ((chan = INC_LIST_REMOVE_HEAD(&logchannels, list)))
		free(chan);
	INC_LIST_UNLOCK(&logchannels);
	
	global_logmask = 0;
	errno = 0;
	/* close syslog */
	closelog();
	if (!(chan = inc_calloc(1, sizeof(*chan))))
		return DRV_ERR;
	if (!inc_strlen_zero(hostname)) {
		snprintf(chan->filename, sizeof(chan->filename), "%s/mgw.log.%s", inc_config_log_dir, hostname);
	} else {
		snprintf(chan->filename, sizeof(chan->filename), "%s/mgw.log", inc_config_log_dir);
	}
	chan->fileptr = fopen(chan->filename, "a");
	if (!chan->fileptr) {
		/* Can't log here, since we're called with a lock */
		fprintf(stderr, "Logger Warning: Unable to open log file '%s': %s\n", chan->filename, strerror(errno));
	}
	chan->type = LOGTYPE_FILE;

	if (glog_level == -1)
		chan->logmask = 28;
	else 
		chan->logmask = glog_level;

	INC_LIST_LOCK(&logchannels);
	INC_LIST_INSERT_HEAD(&logchannels, chan, list);
	INC_LIST_UNLOCK(&logchannels);
	global_logmask |= chan->logmask;
	return DRV_OK;
}

void inc_queue_log(const char *queuename, const char *callid, const char *agent, const char *event, const char *fmt, ...)
{
	va_list ap;
	INC_LIST_LOCK(&logchannels);
	if (qlog) {
		va_start(ap, fmt);
		fprintf(qlog, "%ld|%s|%s|%s|%s|", (long)time(NULL), callid, queuename, agent, event);
		vfprintf(qlog, fmt, ap);
		fprintf(qlog, "\n");
		va_end(ap);
		fflush(qlog);
	}
	INC_LIST_UNLOCK(&logchannels);
}

int reload_logger(int rotate)
{
	char old[PATH_MAX] = "";
	char new[PATH_MAX];
	int event_rotate = rotate, queue_rotate = rotate;
	struct logchannel *f;
	FILE *myf;
	int x, res = 0;

	INC_LIST_LOCK(&logchannels);

	if (eventlog) 
		fclose(eventlog);
	else 
		event_rotate = 0;
	eventlog = NULL;

	if (qlog) 
		fclose(qlog);
	else 
		queue_rotate = 0;
	qlog = NULL;

	mkdir((char *)inc_config_log_dir, 0755);

	INC_LIST_TRAVERSE(&logchannels, f, list) {
		if (f->disabled) {
			f->disabled = 0;	/* Re-enable logging at reload */
		}
		if (f->fileptr && (f->fileptr != stdout) && (f->fileptr != stderr)) {
			fclose(f->fileptr);	/* Close file */
			f->fileptr = NULL;
			if (rotate) {
				inc_copy_string(old, f->filename, sizeof(old));
	
				for (x = 0; ; x++) {
					//snprintf(new, sizeof(new), "%s.%d", f->filename, system_name, x);
					snprintf(new, sizeof(new), "%s.%d", f->filename, x);
					myf = fopen((char *)new, "r");
					if (myf)
						fclose(myf);
					else
						break;
				}
	    
				/* do it */
				if (rename(old,new))
					fprintf(stderr, "Unable to rename file '%s' to '%s'\n", old, new);
			}
		}
	}

	filesize_reload_needed = 0;
	
	init_logger_chain();

	if (logfiles.event_log) {
		snprintf(old, sizeof(old), "%s/%s", (char *)inc_config_log_dir, EVENTLOG);
		if (event_rotate) {
			for (x=0;;x++) {
				snprintf(new, sizeof(new), "%s/%s.%d", (char *)inc_config_log_dir, EVENTLOG, x);
				myf = fopen((char *)new, "r");
				if (myf) 	/* File exists */
					fclose(myf);
				else
					break;
			}
	
			/* do it */
			if (rename(old,new))
				inc_log(LOG_ERROR, "Unable to rename file '%s' to '%s'\n", old, new);
		}

		eventlog = fopen(old, "a");
		if (eventlog) {
			inc_log(LOG_EVENT, "Restarted inc_ag Event Logger\n");
			if (option_verbose)
				inc_verbose("INC_AG Event Logger restarted\n");
		} else {
			inc_log(LOG_ERROR, "Unable to create event log: %s\n", strerror(errno));
			res = -1;
		}
	}

	if (logfiles.queue_log) {
		snprintf(old, sizeof(old), "%s/%s", (char *)inc_config_log_dir, QUEUELOG);
		if (queue_rotate) {
			for (x = 0; ; x++) {
				snprintf(new, sizeof(new), "%s/%s.%d", (char *)inc_config_log_dir, QUEUELOG,x);
				myf = fopen((char *)new, "r");
				if (myf) 	/* File exists */
					fclose(myf);
				else
					break;
			}
	
			/* do it */
			if (rename(old, new))
				inc_log(LOG_ERROR, "Unable to rename file '%s' to '%s'\n", old, new);
		}

		qlog = fopen(old, "a");
		if (qlog) {
			inc_queue_log("NONE", "NONE", "NONE", "CONFIGRELOAD", "%s", "");
			inc_log(LOG_EVENT, "Restarted INC_AG Queue Logger\n");
			if (option_verbose)
				inc_verbose("INC_AG Queue Logger restarted\n");
		} else {
			inc_log(LOG_ERROR, "Unable to create queue log: %s\n", strerror(errno));
			res = -1;
		}
	}

	INC_LIST_UNLOCK(&logchannels);

	return res;
}

/*! \brief Reload the logger module without rotating log files (also used from loader.c during
	a full INC_AG reload) */
int logger_reload(void)
{
	if(reload_logger(0))
		return RESULT_FAILURE;
	return RESULT_SUCCESS;
}

struct verb {
	void (*verboser)(const char *string);
	INC_LIST_ENTRY(verb) list;
};

static INC_LIST_HEAD_STATIC(verbosers, verb);

static int handle_SIGXFSZ(int sig) 
{
	/* Indicate need to reload */
	filesize_reload_needed = 1;
	return 0;
}

int init_logger(void)
{
	char tmp[256];
	int res = 0;

	/* auto rotate if sig SIGXFSZ comes a-knockin */
	(void) signal(SIGXFSZ,(void *) handle_SIGXFSZ);

	mkdir((char *)inc_config_log_dir, 0755);
  
	/* create log channels */
	init_logger_chain();

	/* create the eventlog */
	if (logfiles.event_log) {
		mkdir((char *)inc_config_log_dir, 0755);
		snprintf(tmp, sizeof(tmp), "%s/%s", (char *)inc_config_log_dir, EVENTLOG);
		eventlog = fopen((char *)tmp, "a");
		if (eventlog) {
			inc_log(LOG_EVENT, "Started INC_AG Event Logger\n");
			if (option_verbose)
				inc_verbose("INC_AG Event Logger Started %s\n",(char *)tmp);
		} else {
			inc_log(LOG_ERROR, "Unable to create event log: %s\n", strerror(errno));
			res = -1;
		}
	}

	if (logfiles.queue_log) {
		snprintf(tmp, sizeof(tmp), "%s/%s", (char *)inc_config_log_dir, QUEUELOG);
		qlog = fopen(tmp, "a");
		inc_queue_log("NONE", "NONE", "NONE", "QUEUESTART", "%s", "");
	}
	return res;
}

void close_logger(void)
{
	struct logchannel *f;

	INC_LIST_LOCK(&logchannels);

	if (eventlog) {
		fclose(eventlog);
		eventlog = NULL;
	}

	if (qlog) {
		fclose(qlog);
		qlog = NULL;
	}

	INC_LIST_TRAVERSE(&logchannels, f, list) {
		if (f->fileptr && (f->fileptr != stdout) && (f->fileptr != stderr)) {
			fclose(f->fileptr);
			f->fileptr = NULL;
		}
	}

	closelog(); /* syslog */

	INC_LIST_UNLOCK(&logchannels);

	return;
}

static void __attribute__((format(printf, 5, 0))) inc_log_vsyslog(int level, const char *file, int line, const char *function, const char *fmt, va_list args) 
{
	char buf[BUFSIZ];
	char *s;

	if (level >= SYSLOG_NLEVELS) {
		/* we are locked here, so cannot inc_log() */
		fprintf(stderr, "inc_log_vsyslog called with bogus level: %d\n", level);
		return;
	}
	if (level == __LOG_VERBOSE) {
		snprintf(buf, sizeof(buf), "VERBOSE[%ld]: ", (long)GETTID());
		level = __LOG_DEBUG;
	} else if (level == __LOG_DTMF) {
		snprintf(buf, sizeof(buf), "DTMF[%ld]: ", (long)GETTID());
		level = __LOG_DEBUG;
	} else {
		snprintf(buf, sizeof(buf), "%s[%ld]: %s:%d in %s: ",
			 levels[level], (long)GETTID(), file, line, function);
	}
	s = buf + strlen(buf);
	vsnprintf(s, sizeof(buf) - strlen(buf), fmt, args);
	term_strip(s, s, strlen(s) + 1);
	syslog(syslog_level_map[level], "%s", buf);
}

/*!
 * \brief send log messages to syslog and/or the console
 */
void inc_log(int level, const char *file, int line, const char *function, const char *fmt, ...)
{
	struct logchannel *chan;
	struct inc_dynamic_str *buf;
	time_t t;
	struct tm *tm = NULL;
	char date[256];

	va_list ap;

	if (!(buf = inc_dynamic_str_thread_get(&log_buf, LOG_BUF_INIT_SIZE)))
		return;

	if (INC_LIST_EMPTY(&logchannels))
	{
		/*
		 * we don't have the logger chain configured yet,
		 * so just log to stdout
		*/
		if (level != __LOG_VERBOSE) {
			int res;
			va_start(ap, fmt);
			res = inc_dynamic_str_thread_set_va(&buf, BUFSIZ, &log_buf, fmt, ap);
			va_end(ap);
			if (res != INC_DYNSTR_BUILD_FAILED) {
				term_filter_escapes(buf->str);
				fputs(buf->str, stdout);
			}
		}
		return;
	}
	
	/* Ignore anything that never gets logged anywhere */
	if (!(global_logmask & (1 << level)))
		return;

	time(&t);
	tm = localtime(&t);
	strftime(date, sizeof(date), dateformat, tm);

	INC_LIST_LOCK(&logchannels);

	if (logfiles.event_log && level == __LOG_EVENT) {
		va_start(ap, fmt);

		fprintf(eventlog, "%s inc_cc[%ld]: ", date, (long)getpid());
		vfprintf(eventlog, fmt, ap);
		fflush(eventlog);

		va_end(ap);
		INC_LIST_UNLOCK(&logchannels);
		return;
	}

	INC_LIST_TRAVERSE(&logchannels, chan, list) {
		if (chan->disabled)
			break;
		/* Check syslog channels */
		if (chan->type == LOGTYPE_SYSLOG && (chan->logmask & (1 << level))) {
			va_start(ap, fmt);
			inc_log_vsyslog(level, file, line, function, fmt, ap);
			va_end(ap);
		/* Console channels */
		} else if ((chan->logmask & (1 << level)) && (chan->fileptr)) {
			int res;
			inc_dynamic_str_thread_set(&buf, BUFSIZ, &log_buf, 
				"[%s]%s-%ld: ",
				date, levels[level],(long)GETTID());
			res = fprintf(chan->fileptr, "%s", term_strip(buf->str, buf->str, strlen(buf->str) + 1));
			if (res <= 0 && !inc_strlen_zero(buf->str)) {	/* Error, no characters printed */
				fprintf(stderr,"**** MGW Logging Error: ***********\n");
				if (errno == ENOMEM || errno == ENOSPC) {
					fprintf(stderr, "MGW logging error: Out of disk space, can't log to log file %s\n", chan->filename);
				} else
					fprintf(stderr, "Logger Warning: Unable to write to log file '%s': %s (disabled)\n", chan->filename, strerror(errno));
				chan->disabled = 1;	
			} else {
				int res;
				/* No error message, continue printing */
				va_start(ap, fmt);
				res = inc_dynamic_str_thread_set_va(&buf, BUFSIZ, &log_buf, fmt, ap);
				va_end(ap);
				if (res != INC_DYNSTR_BUILD_FAILED) {
					term_strip(buf->str, buf->str, buf->len);
					fputs(buf->str, chan->fileptr);
					fflush(chan->fileptr);
				}
			}
		}
	}

	INC_LIST_UNLOCK(&logchannels);

	if (filesize_reload_needed) {
		reload_logger(1);
		inc_log(LOG_EVENT,"Rotated Logs Per SIGXFSZ (Exceeded file size limit)\n");
		if (option_verbose)
			inc_verbose("Rotated Logs Per SIGXFSZ (Exceeded file size limit)\n");
	}
}

void inc_backtrace(void)
{
#ifdef linux
#ifdef INC_DEVMODE
	int count=0, i=0;
	void **addresses;
	char **strings;

	if ((addresses = inc_calloc(MAX_BACKTRACE_FRAMES, sizeof(*addresses)))) {
		count = backtrace(addresses, MAX_BACKTRACE_FRAMES);
		if ((strings = backtrace_symbols(addresses, count))) {
			inc_log(LOG_DEBUG, "Got %d backtrace record%c\n", count, count != 1 ? 's' : ' ');
			for (i=0; i < count ; i++) {
#if __WORDSIZE == 32
				inc_log(LOG_DEBUG, "#%d: [%08X] %s\n", i, (unsigned int)addresses[i], strings[i]);
#elif __WORDSIZE == 64
				inc_log(LOG_DEBUG, "#%d: [%016lX] %s\n", i, (unsigned long)addresses[i], strings[i]);
#endif
			}
			free(strings);
		} else {
			inc_log(LOG_DEBUG, "Could not allocate memory for backtrace\n");
		}
		free(addresses);
	}
#else
	inc_log(LOG_WARNING, "Must run configure with '--enable-dev-mode' for stack backtraces.\n");
#endif
#else /* ndef linux */
	inc_log(LOG_WARNING, "Inline stack backtraces are only available on the Linux platform.\n");
#endif
}

void inc_verbose(const char *fmt, ...)
{
	struct verb *v;
	struct inc_dynamic_str *buf;
	int res;
	va_list ap;

//	if (inc_opt_timestamp) {
	if(0){
		time_t t;
		struct tm *tm = NULL;
		char date[40];
		char *datefmt;

		time(&t);
		tm = localtime(&t);
		strftime(date, sizeof(date), dateformat, tm);
		datefmt = alloca(strlen(date) + 3 + strlen(fmt) + 1);
		//sprintf(datefmt, "%c[%s] %s", 127, date, fmt);
		snprintf(datefmt, strlen(date) + 3 + strlen(fmt) + 1, \
		    "%c[%s] %s", 127, date, fmt);
		fmt = datefmt;
	} else {
		char *tmp = alloca(strlen(fmt) + 2);
		//sprintf(tmp, "%c%s", 127, fmt);
		snprintf(tmp, strlen(fmt) + 2, "%c%s", 127, fmt);
		fmt = tmp;
	}

	if (!(buf = inc_dynamic_str_thread_get(&verbose_buf, VERBOSE_BUF_INIT_SIZE)))
		return;

	va_start(ap, fmt);
	res = inc_dynamic_str_thread_set_va(&buf, 0, &verbose_buf, fmt, ap);
	va_end(ap);

	if (res == INC_DYNSTR_BUILD_FAILED)
		return;
	
	/* filter out possibly hazardous escape sequences */
	term_filter_escapes(buf->str);

	INC_LIST_LOCK(&verbosers);
	INC_LIST_TRAVERSE(&verbosers, v, list)
		v->verboser(buf->str);
	INC_LIST_UNLOCK(&verbosers);

	inc_log(LOG_VERBOSE, "%s", buf->str + 1);
}

int inc_register_verbose(void (*v)(const char *string)) 
{
	struct verb *verb;

	if (!(verb = inc_malloc(sizeof(*verb))))
		return -1;

	verb->verboser = v;

	INC_LIST_LOCK(&verbosers);
	INC_LIST_INSERT_HEAD(&verbosers, verb, list);
	INC_LIST_UNLOCK(&verbosers);
	
	return 0;
}

int inc_unregister_verbose(void (*v)(const char *string))
{
	struct verb *cur;

	INC_LIST_LOCK(&verbosers);
	INC_LIST_TRAVERSE_SAFE_BEGIN(&verbosers, cur, list) {
		if (cur->verboser == v) {
			INC_LIST_REMOVE_CURRENT(&verbosers, list);
			free(cur);
			break;
		}
	}
	INC_LIST_TRAVERSE_SAFE_END
	INC_LIST_UNLOCK(&verbosers);
	
	return cur ? 0 : -1;
}

























