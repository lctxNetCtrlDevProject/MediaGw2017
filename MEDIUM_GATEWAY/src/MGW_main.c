/*
=================================================================
** File name: MGW_main.c
** Decription: 主控程序
** Modify History:
** 1.0.0.1: fix some bugs
** 1.0.0.2:	fix tunnel && dnat bugs
** 1.0.0.3: fix ip-slot && dnat bugs
** 1.0.0.4: fix ip-slot bugs
** 1.0.0.5: fix tunnel && dnat bugs
** 1.0.0.6: fix bug in network interface operator function
** 1.0.0.7: fix call_control release reason(timeout) && call
**          protocol sub len,
** 1.0.0.8: fix nat error in add over 20 ports && mgw_equal_if
** 1.0.0.9: fix bug in set eth0 ip addr
** 1.0.1.0: fix bug in set eth0 ip addr
** 1.0.1.1: fix bug in set eth0 ip addr
** 1.0.1.2: fix bug in set eth0 ip addr && port
** 1.0.1.3: fix bug in tunnel && dnat
** 1.0.1.4: fix bug in dnat port && config file
** 1.0.1.5: fix bug in yangsheng delete operator
** 1.0.1.6: js_socket bind INADDR_ANY as it go on && bug in call release
** 1.0.1.7: modify the voice ip stream
** 1.0.1.8: fix bug in network interface set
** 1.0.1.9: fix bug in eth0:1 interface ip configure
** 1.0.2.0: add radio hf direct talk protocol
** 1.0.2.1: add control in jishe call,use dynamic && static method
**          to update or add node-phone relation
** 1.0.2.2: change MAX_XIWEI value to fix Z024
** 1.0.2.3: unlimit the receive ip addr to fix Z024
** 1.0.2.4: fix Z024 bug(radio direct call && Qinwu PTT)
** 1.0.0.0: Z043基于Z024的1.0.2.4版本修改
==================================================================
*/
#include "PUBLIC.h"
#include "MGWparaInject.h"
#include <sys/resource.h>

#define MGW_VERSION	 "MGW_LCTX - 1.0.0.0"

const char version_string[] = MGW_VERSION" (" __DATE__ " - " __TIME__ ")";

#define PIPE_NAME		"/tmp/MGW_PIPE"
#define	MGW_FILE 		"/data/mgw.conf"			//config file
#define	CONF_FILE 		"/data/config.conf"			//config file

#undef SWITCH_VLAN_CTL
//#define SWITCH_VLAN_CTL


static pthread_t thread_monitor;
static pthread_t thread_rtp;
static pthread_t thread_shell;
static pthread_t thread_cfg_wr;
static pthread_t thread_pipe;
static pthread_t rpcDbgThrdId;
static pthread_t rpcSeatMngThrdId;
static pthread_t rpcSeatMngDefaultThrdId;
static pthread_t rpcBoard716MngThrdId;
static pthread_t rpcBoard716RayMngThrdId;
static pthread_t rpcBoard716RadioMngThrdId;
static pthread_t rpcBoard50MngThrdId;
static pthread_t rpcBoard50NetMngThrdId;


static pthread_t rpcSwPhoneDatThrdId;
static pthread_t rpcSwPhoneVocThrdId;
static pthread_t rpcSwPhoneMngThrdId;

static pthread_t rpcDisplayThrdId;
static pthread_t rpcDisplay_zhuangjiaThrdId;
static pthread_t rpcDisplay_zhuangjia716ThrdId;
static pthread_t rpcSeatThrdId;
static pthread_t rpcVoiceThrdId;
static pthread_t uartThrdId;
static pthread_t paraInjectThrdId;

static pthread_t ZjctAdapMng_ThrdId;
static pthread_t ZjctAdapVoc_ThrdId;

static pthread_t ZjctAdapMng_0_ThrdId;
static pthread_t ZjctAdapMng_1_ThrdId;
static pthread_t ZjctAdapMng_2_ThrdId;
static pthread_t ZjctAdapMng_3_ThrdId;
static pthread_t ZjctAdapMng_4_ThrdId;
static pthread_t ZjctAdapMng_5_ThrdId;
static pthread_t ZjctAdapMng_6_ThrdId;
static pthread_t ZjctAdapMng_7_ThrdId;
static pthread_t ZjctAdapMng_8_ThrdId;
static pthread_t ZjctAdapMng_9_ThrdId;
static pthread_t ZjctAdapMng_10_ThrdId;
static pthread_t ZjctAdapMng_11_ThrdId;

static pthread_t ZjctIncCal_0_ThrdId;
static pthread_t ZjctIncVoc_0_ThrdId;
static pthread_t ZjctIncCal_1_ThrdId;
static pthread_t ZjctIncVoc_1_ThrdId;
static pthread_t ZjctIncCal_2_ThrdId;
static pthread_t ZjctIncVoc_2_ThrdId;
static pthread_t ZjctIncCal_3_ThrdId;
static pthread_t ZjctIncVoc_3_ThrdId;
static pthread_t ZjctIncCal_4_ThrdId;
static pthread_t ZjctIncVoc_4_ThrdId;
static pthread_t ZjctIncCal_5_ThrdId;
static pthread_t ZjctIncVoc_5_ThrdId;
static pthread_t ZjctIncCal_6_ThrdId;
static pthread_t ZjctIncVoc_6_ThrdId;
static pthread_t ZjctIncCal_7_ThrdId;
static pthread_t ZjctIncVoc_7_ThrdId;
static pthread_t ZjctIncCal_8_ThrdId;
static pthread_t ZjctIncVoc_8_ThrdId;
static pthread_t ZjctIncCal_9_ThrdId;
static pthread_t ZjctIncVoc_9_ThrdId;
static pthread_t ZjctIncCal_10_ThrdId;
static pthread_t ZjctIncVoc_10_ThrdId;
static pthread_t ZjctIncCal_11_ThrdId;
static pthread_t ZjctIncVoc_11_ThrdId;

static pthread_t rpcXiWeiDatThrdId;
static pthread_t rpcXiWeiVocThrdId;

static pthread_t rpcAdpPhoneDatThrdId;
static pthread_t rpcAdpPhoneVocThrdId;

static pthread_t rpcSeatDataThrdId;


static void *safe_system_prev_handler;

struct sched_context * con = NULL;
static struct io_context	* ioc = NULL;

unsigned long g_hf_addr;
unsigned int time_cnt_20 = 0;
struct config *mgw_cfg = NULL;
struct config *config_cfg = NULL;

//struct inc_flags inc_options = { INC_DEFAULT_OPTIONS };

int option_verbose;				/*!< Verbosity level */

//double option_maxload;				/*!< Max load avg on system */
//int option_maxcalls;				/*!< Max number of active calls */
int glog_level = -1;
/*! @} */

//char record_cache_dir[INC_CACHE_DIR_LEN] = "/var/spool/mgw/tmp";
//char debug_filename[INC_FILENAME_MAX] = "";

char inc_config_dir[PATH_MAX] = "/tmp/";
char inc_config_file[PATH_MAX] = "/tmp/mgwsip.conf";
char inc_config_log_dir[PATH_MAX] = "/tmp/log";
char system_name[20];

extern uint32 time_cnt_100ms; //计算成员是否掉线时间
extern int32 snmpAgentMng_Socket_init(void);
extern int32 Board_50_Mng_Socket_init(void);
extern int DisplayBoardShowVersion();
extern int32 Board_50_Net_Mng_Socket_init(void);


static void * do_monitor(void *arg)
{
	if(!(ioc = io_context_create()))
	{
		VERBOSE_OUT(LOG_SYS,"Create IO Context Error!\n");
		return NULL;
	}

	switch(Param_update.workmode)
	{
		case WORK_MODE_PAOBING:
		case WORK_MODE_ZHUANGJIA:
			gIpcSemSocket.ioc = ioc;	
			gIpcSemSocket.ioc_id = inc_io_add(ioc,gIpcSemSocket.udp_handle,lan_bus_ctrl,POLLIN,&gIpcSemSocket);

			
			ext_udp_socket.ioc = ioc;	
			ext_udp_socket.ioc_id = inc_io_add(ioc,ext_udp_socket.udp_handle,lan_bus_ctrl,POLLIN,&ext_udp_socket);
			
			gIpcDatSocket.ioc = ioc;
			gIpcDatSocket.ioc_id = inc_io_add(ioc,gIpcDatSocket.udp_handle,lan_bus_ctrl,POLLIN,&gIpcDatSocket);
			break;
		case WORK_MODE_JILIAN:
		case WORK_MODE_SEAT:
			break;
		case WORK_MODE_ADAPTER:
			gIpcDatSocket.ioc = ioc;
			gIpcDatSocket.ioc_id = inc_io_add(ioc,gIpcDatSocket.udp_handle,lan_bus_ctrl,POLLIN,&gIpcDatSocket);
			break;
		default:
			break;
	}	


	printf("%s:start\r\n", __func__);	
	while(1)
	{
		inc_io_wait(ioc,inc_sched_wait(con));
		inc_sched_runq(con);
	}



	return NULL;
}

static void *do_command(void *arg)
{
	char cmd_buf[128];
	int i;

	printf("%s:start\r\n", __func__);
	
	while(1)
	{
		if(fgets(cmd_buf,sizeof(cmd_buf),stdin))
		{
			if(strcmp(cmd_buf,"\n") == 0)
				printf(">>");
			else
			{
				i = strlen(cmd_buf);
				cmd_buf[i-1] = '\0';
				run_command(cmd_buf,0);
				printf(">>");
			}
		}
	}

}

static void *do_pipe(void *arg)
{
	char cmd_buf[128];
	//int i;
	FILE * pipe_stream;
	inc_log(LOG_DEBUG,"do_pipe thread is running...\n");
	pipe_stream	= fopen(PIPE_NAME,"r");
	if(!pipe_stream){
		inc_log(LOG_DEBUG,"Can't Open pipe %s\n",PIPE_NAME);
        fclose(pipe_stream);
		return NULL;
	}
	
	if(fgets(cmd_buf,sizeof(cmd_buf),pipe_stream))
	{
		run_command("exit",0);
	}

	fclose(pipe_stream);
	return NULL;
}



/*================================调试输出函数===================================*/
#ifdef	DEBUG_INFO
void	VERBOSE_OUT(const char *file,int line,const char *fmt,...)
{
	char	buf[256];
	va_list	ap;
	va_start(ap,fmt);
	vsprintf(buf,fmt,ap);
	va_end(ap);
	printf("%s(%d): %s",file,line,buf);
}
#endif

static int32 ip_addr_config(void)
{
	char cmd_str[128] = {0};
	char cmd_str1[128] = {0};	

	switch(Param_update.workmode)
	{
		case WORK_MODE_PAOBING:
		case WORK_MODE_ZHUANGJIA:
		case WORK_MODE_ADAPTER:

			snprintf (cmd_str1, sizeof(cmd_str1), "%s", get_config_var_str(mgw_cfg,"EXTEND_LOCAL_IP"));
			snprintf(cmd_str ,sizeof(cmd_str), "ifconfig eth0 %s netmask %s up", \
				  cmd_str1, get_config_var_str(mgw_cfg,"EXTEND_LOCAL_NETMASK"));			
				  			

			mgw_sys_exec(cmd_str);

			usleep(50 * 1000);

			/* 炮防模式下，与50所传递战网单元参数的网管消息 采用192.168.254.3 */
			snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:1 %s netmask 255.255.255.0",\
			    get_config_var_str(config_cfg,"DEFAULT_LOCAL_IP"));
			mgw_sys_exec(cmd_str);
			usleep(50 * 1000);
			
			snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:2 %s netmask 255.255.255.0", DISPLAY_BRD_MNG_LOCAL_IP);
			mgw_sys_exec(cmd_str);
			usleep(50 * 1000);

			snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:3 192.168.9.3 netmask 255.255.255.0");
			mgw_sys_exec(cmd_str);	
			usleep(50 * 1000);

			if(1 == Param_update.ip_addr_default_flg)
			{
				snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:4 10.0.0.3 netmask 255.255.255.0");
				mgw_sys_exec(cmd_str);	
				usleep(50 * 1000);
			}

			snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:5 %s netmask 255.255.255.0", PAOFANG_BRD_MNG_IP_LOCAL);
			mgw_sys_exec(cmd_str);	
			usleep(50 * 1000);

			break;
		case WORK_MODE_JILIAN:
	            	snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"EXTEND_LOCAL_SEAT_IP"));
	    		mgw_sys_exec(cmd_str);

	    		usleep(50 * 1000);
	    		
	            	snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:1 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"DEFAULT_SEAT_LOCAL_IP"));
	    		mgw_sys_exec(cmd_str);
			break;
		case WORK_MODE_SEAT:
	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"EXTEND_LOCAL_SEAT_IP"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:0 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"DEFAULT_LOCAL_IP"));
	    		mgw_sys_exec(cmd_str);		
	    		
	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:1 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_adptipaddr0"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:2 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_incipaddr0"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);	

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:3 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_adptipaddr1"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:4 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_incipaddr1"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);		

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:5 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_adptipaddr2"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:6 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_incipaddr2"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);	
	    		
	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:7 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_adptipaddr3"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:8 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_incipaddr3"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);	

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:9 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_adptipaddr4"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:10 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_incipaddr4"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);	

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:11 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_adptipaddr5"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:12 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_incipaddr5"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);		

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:13 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_adptipaddr6"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:14 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_incipaddr6"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);	
			
	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:15 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_adptipaddr7"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:16 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_incipaddr7"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);			

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:17 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_adptipaddr8"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:18 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_incipaddr8"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);	

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:19 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_adptipaddr9"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:20 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_incipaddr9"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);		

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:21 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_adptipaddr10"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:22 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_incipaddr10"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);	
	    		
	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:23 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_adptipaddr11"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);

	    		snprintf(cmd_str, sizeof(cmd_str), "ifconfig eth0:24 %s netmask 255.255.255.0",get_config_var_str(config_cfg,"local_zjct_incipaddr11"));
	    		mgw_sys_exec(cmd_str);
	    		usleep(50 * 1000);	
			break;
		default:
			break;
	}	

	mgw_sys_exec("route add -host 255.255.255.255 dev eth0");

	return DRV_OK;
}

static int32 param_init(void)
{
	uint8 i = 0;
	char buf[100] = {0};
	uint32 tmp1 = 0;
	uint32 tmp2  =0;
	
	Param_update.workmode = get_config_var_val(mgw_cfg,"workmode");
	if(Param_update.workmode > WORK_MODE_BUFF)
	{
		Param_update.workmode = WORK_MODE_PAOBING;
	}

	printf("workmode = %d\r\n", Param_update.workmode);

	for(i = 0; i < CHETONG_SEAT_MAX; i++)
	{
		snprintf(buf, 100, "local_zjct_adptipaddr%d", i);
		II_Adpt_local[i].ipaddr = inet_addr(get_config_cat_var_str(config_cfg, ETH, buf));
		II_Adpt_local[i].netmask = inet_addr(get_config_cat_var_str(config_cfg, ETH, "local_zjct_adptnetmask"));
		II_Adpt_local[i].zjct_adptmng_port = get_config_cat_var_val(config_cfg, ETH, "local_zjct_adptmngport");
		II_Adpt_local[i].zjct_adptvoc_port = get_config_cat_var_val(config_cfg, ETH, "local_zjct_adptvocport");	
		
		snprintf(buf, 100, "local_zjct_incipaddr%d", i);
		II_Inc_local[i].ipaddr = inet_addr(get_config_cat_var_str(config_cfg, ETH, buf));
		II_Inc_local[i].netmask = inet_addr(get_config_cat_var_str(config_cfg, ETH, "local_zjct_incnetmask"));
		II_Inc_local[i].zjct_incsem_port = get_config_cat_var_val(config_cfg, ETH, "local_zjct_incsemport");
		II_Inc_local[i].zjct_incvoc_port = get_config_cat_var_val(config_cfg, ETH, "local_zjct_incvocport");		
	}

	II_Adpt_remote.ipaddr = inet_addr(get_config_cat_var_str(config_cfg, ETH, "remote_zjct_adptipaddr"));
	II_Adpt_remote.zjct_adptmng_port = get_config_cat_var_val(config_cfg, ETH, "remote_zjct_adptmngport");
	II_Adpt_remote.zjct_adptvoc_port = get_config_cat_var_val(config_cfg, ETH, "remote_zjct_adptvocport");

	II_Inc_remote.ipaddr = inet_addr(get_config_cat_var_str(config_cfg, ETH, "remote_zjct_incipaddr"));
	II_Inc_remote.zjct_incsem_port = get_config_cat_var_val(config_cfg, ETH, "remote_zjct_incsemport");
	II_Inc_remote.zjct_incvoc_port = get_config_cat_var_val(config_cfg, ETH, "remote_zjct_incvocport");	

	tmp1 = htonl(inet_addr(get_config_var_str(mgw_cfg,"EXTEND_LOCAL_IP")));
	tmp2 = htonl(inet_addr(get_config_var_str(config_cfg,"EXTEND_DEFAULT_IP")));

	if(tmp1 != tmp2)
	{
		Param_update.ip_addr_default_flg = 1;
	}
	
	return DRV_OK;
}

static void null_sig_handler(int signal)
{
	return;
}

void mgw_replace_sigchld(void)
{
	safe_system_prev_handler = signal(SIGCHLD, null_sig_handler);

	return;
}


static int32 thread_process(const void *data)
{
	Uart_data_process_20ms();
	
	time_cnt_20++;
	
	return 1;
}


static void *mng_process(void *arg)
{
	printf("%s start !\r\n", __func__);

    while(1)
    {
        //if(0x00 == (time_cnt_100ms%10))
        {
            if(1 == Param_update.cfg_wr_flg)
            {
                Param_update.cfg_wr_flg = 0x00;
                rewrite_config(mgw_cfg); 
            }  
        }
        
        sleep(2);
    }
}

int main(int argc, char *argv[])
{	
	DEBUG_OUT("=================%s=================\n",version_string);
	
	{
		int c;
		while ((c = getopt(argc, argv, "gch")) != -1) {
			switch(c){
				case 'g':
				{
					struct rlimit l;
					memset(&l, 0, sizeof(l));
					l.rlim_cur = RLIM_INFINITY;
					l.rlim_max = RLIM_INFINITY;
					if (setrlimit(RLIMIT_CORE, &l)) {
						DEBUG_OUT("Unable to disable core size resource limit: %s\n", strerror(errno));
					}
				}break;
				
				case 'c':
				{
					DEBUG_OUT("Is Don't Realize,Run with console\n");
				}break;

				case 'h':
				{
					DEBUG_OUT("options: \n"
						"-g\t\tdump core\n"
						"-c\t\tdon't realize\n"
						"-h\t\tget help&&version\n");
					exit(0);
				}break;
			}
		}
	}

	/* gdb调试使能*/
	{
		struct rlimit l;
		memset(&l, 0, sizeof(l));
		//l.rlim_cur = RLIM_INFINITY;
		//l.rlim_max = RLIM_INFINITY;
		if (setrlimit(RLIMIT_CORE, &l)) {
			fprintf(stderr,"Unable to disable core size resource limit: %s\n", strerror(errno));
		}
	}
	
	printf("Z043 compile at %s, %s\r\n", __DATE__, __TIME__);	

	if (DRV_OK != init_config(&mgw_cfg, MGW_FILE))
	{
		printf("mgw file failed!\n");
		return DRV_ERR;
	}

	if (DRV_OK != init_config(&config_cfg, CONF_FILE))
	{
		printf("config file failed!\n");
		return DRV_ERR;
	}	

	if (DRV_OK != param_init())
	{
		printf("param_init failed!\n");
		return DRV_ERR;
	}

	if (DRV_OK != ip_addr_config())
	{
		printf("ip_addr_config failed!\n");
		return DRV_ERR;
	}	
	
#ifdef SWITCH_VLAN_CTL	
	Uart_init();	
#endif
	Gpio_init();

	/*create sched_context list*/
	if(!(con = sched_context_create()))
	{
		VERBOSE_OUT(LOG_SYS,"Create Sched Context Error!\n");
		return  -1;
	}

	if(con)
	{
		switch(Param_update.workmode)
		{
			case WORK_MODE_PAOBING:
			case WORK_MODE_ZHUANGJIA:
			case WORK_MODE_JILIAN:
			case WORK_MODE_SEAT:
				inc_sched_add(con,20,init_terminal,con);
				inc_sched_add(con,50,init_lan_bus,NULL);	
				break;
			case WORK_MODE_ADAPTER:
				inc_sched_add(con, 20, kuozhan_init_terminal, con);
				inc_sched_add(con, 50, kuozhan_socket_init, con);
				inc_sched_add(con,50,  init_lan_bus,NULL);
				break;
			default:
				break;
		}		
	}

	RpcDbg_Socket_init();
	snmpAgentMng_Socket_init();		/*cfg dst addr&port of 834 snmp agent*/
	RpcDisplayMng_Socket_init();
	RpcSeatMng_Socket_init();
	Board_716_Mng_Socket_init();
	Board_716_Ray_Mng_Socket_init();
	Board_716_Radio_Socket_init();
	Board_50_Mng_Socket_init();
	RpcDisplayMng_Zhuangjia_From_dis_Socket_init();
	RpcDisplayMng_from_716_Socket_init();	
	Board_50_Net_Mng_Socket_init();

	while(1)
	{
		if(inc_sched_runq(con) == 0)
		{
			usleep(500);
			if(inc_sched_wait(con) < 0)
			{
				DEBUG_OUT("The lan_socket had been initialized!\n");
				break;
			}
		}
	}

	switch(Param_update.workmode)
	{
		case WORK_MODE_PAOBING:
		case WORK_MODE_ZHUANGJIA:
		case WORK_MODE_JILIAN:
		case WORK_MODE_SEAT:
			inc_sched_add(con, 100, terminal_call_control,NULL);
			inc_sched_add(con, 20, thread_process,NULL);	
			inc_sched_add(con, 1000, Check_834_Sw_Phone_Timeout,NULL);	
			inc_sched_add(con, 1000, Check_Control_Board_Timeout,NULL);	
			break;
		case WORK_MODE_ADAPTER:
			inc_sched_add(con, 20, Kuozhan_Ac491_Conf_Voc_Process, con);
			inc_sched_add(con, 100, kuozhan_terminal_call_control, NULL);
			inc_sched_add(con, 100, Check_834_KuoZhan_XiWei_Timeout, NULL);	
			inc_sched_add(con, 100, Check_834_AdpPhone_Timeout, NULL);	
			break;
		default:
			break;
	}

   // inc_sched_add(con, 3000, mng_process, con);

	switch(Param_update.workmode)
	{
		case WORK_MODE_PAOBING:
		case WORK_MODE_ZHUANGJIA:
			//inc_sched_add(con,1000,close_channel,NULL);
			inc_sched_add(con,5000,mgw_5s_proc,NULL);
			inc_pthread_create(&thread_rtp,NULL,do_rtp_monitor,NULL);	
			inc_pthread_create(&thread_pipe,NULL,do_pipe,NULL);	
			inc_pthread_create(&rpcSwPhoneDatThrdId,NULL,(void *)Sw_Phone_Data_RxThread, NULL);	
			inc_pthread_create(&rpcSwPhoneVocThrdId,NULL,(void *)Sw_Phone_Voc_RxThread, NULL);	
			inc_pthread_create(&rpcSwPhoneMngThrdId,NULL,(void *)Sw_Phone_Mng_RxThread, NULL);			
			break;
		case WORK_MODE_JILIAN:
			inc_pthread_create(&rpcSeatThrdId,NULL,(void *)RpcSeat_RxThread, NULL);
			inc_pthread_create(&rpcVoiceThrdId,NULL,(void *)RpcVoice_RxThread, NULL);
			inc_pthread_create(&rpcDbgThrdId,NULL,(void *)RpcDbg_RxThread, NULL);
			inc_pthread_create(&rpcDisplayThrdId,NULL,(void *)RpcDisplayMng_RxThread, NULL);		
			break;
		case WORK_MODE_SEAT:
			inc_sched_add(con,100,Zjct_Mng_Thread,NULL);

			inc_pthread_create(&ZjctAdapMng_ThrdId,NULL,(void *)Zjct_AdptMng_RxThread, NULL);
			inc_pthread_create(&ZjctAdapVoc_ThrdId,NULL,(void *)Zjct_AdptVoc_RxThread, NULL);	

			inc_pthread_create(&ZjctAdapMng_0_ThrdId,NULL,(void *)Zjct_AdptMng_0_RxThread, NULL);		
			inc_pthread_create(&ZjctAdapMng_1_ThrdId,NULL,(void *)Zjct_AdptMng_1_RxThread, NULL);	
			inc_pthread_create(&ZjctAdapMng_2_ThrdId,NULL,(void *)Zjct_AdptMng_2_RxThread, NULL);	
			inc_pthread_create(&ZjctAdapMng_3_ThrdId,NULL,(void *)Zjct_AdptMng_3_RxThread, NULL);	
			inc_pthread_create(&ZjctAdapMng_4_ThrdId,NULL,(void *)Zjct_AdptMng_4_RxThread, NULL);	
			inc_pthread_create(&ZjctAdapMng_5_ThrdId,NULL,(void *)Zjct_AdptMng_5_RxThread, NULL);	
			inc_pthread_create(&ZjctAdapMng_6_ThrdId,NULL,(void *)Zjct_AdptMng_6_RxThread, NULL);	
			inc_pthread_create(&ZjctAdapMng_7_ThrdId,NULL,(void *)Zjct_AdptMng_7_RxThread, NULL);	
			inc_pthread_create(&ZjctAdapMng_8_ThrdId,NULL,(void *)Zjct_AdptMng_8_RxThread, NULL);	
			inc_pthread_create(&ZjctAdapMng_9_ThrdId,NULL,(void *)Zjct_AdptMng_9_RxThread, NULL);	
			inc_pthread_create(&ZjctAdapMng_10_ThrdId,NULL,(void *)Zjct_AdptMng_10_RxThread, NULL);	
			inc_pthread_create(&ZjctAdapMng_11_ThrdId,NULL,(void *)Zjct_AdptMng_11_RxThread, NULL);	
			
			inc_pthread_create(&ZjctIncCal_0_ThrdId,NULL,(void *)Zjct_IncCall_0_RxThread, NULL);
			inc_pthread_create(&ZjctIncVoc_0_ThrdId,NULL,(void *)Zjct_IncVoc_0_RxThread, NULL);	
			inc_pthread_create(&ZjctIncCal_1_ThrdId,NULL,(void *)Zjct_IncCall_1_RxThread, NULL);
			inc_pthread_create(&ZjctIncVoc_1_ThrdId,NULL,(void *)Zjct_IncVoc_1_RxThread, NULL);	
			inc_pthread_create(&ZjctIncCal_2_ThrdId,NULL,(void *)Zjct_IncCall_2_RxThread, NULL);
			inc_pthread_create(&ZjctIncVoc_2_ThrdId,NULL,(void *)Zjct_IncVoc_2_RxThread, NULL);	
			inc_pthread_create(&ZjctIncCal_3_ThrdId,NULL,(void *)Zjct_IncCall_3_RxThread, NULL);
			inc_pthread_create(&ZjctIncVoc_3_ThrdId,NULL,(void *)Zjct_IncVoc_3_RxThread, NULL);	
			inc_pthread_create(&ZjctIncCal_4_ThrdId,NULL,(void *)Zjct_IncCall_4_RxThread, NULL);
			inc_pthread_create(&ZjctIncVoc_4_ThrdId,NULL,(void *)Zjct_IncVoc_4_RxThread, NULL);	
			inc_pthread_create(&ZjctIncCal_5_ThrdId,NULL,(void *)Zjct_IncCall_5_RxThread, NULL);
			inc_pthread_create(&ZjctIncVoc_5_ThrdId,NULL,(void *)Zjct_IncVoc_5_RxThread, NULL);	
			inc_pthread_create(&ZjctIncCal_6_ThrdId,NULL,(void *)Zjct_IncCall_6_RxThread, NULL);
			inc_pthread_create(&ZjctIncVoc_6_ThrdId,NULL,(void *)Zjct_IncVoc_6_RxThread, NULL);	
			inc_pthread_create(&ZjctIncCal_7_ThrdId,NULL,(void *)Zjct_IncCall_7_RxThread, NULL);
			inc_pthread_create(&ZjctIncVoc_7_ThrdId,NULL,(void *)Zjct_IncVoc_7_RxThread, NULL);	
			inc_pthread_create(&ZjctIncCal_8_ThrdId,NULL,(void *)Zjct_IncCall_8_RxThread, NULL);
			inc_pthread_create(&ZjctIncVoc_8_ThrdId,NULL,(void *)Zjct_IncVoc_8_RxThread, NULL);	
			inc_pthread_create(&ZjctIncCal_9_ThrdId,NULL,(void *)Zjct_IncCall_9_RxThread, NULL);
			inc_pthread_create(&ZjctIncVoc_9_ThrdId,NULL,(void *)Zjct_IncVoc_9_RxThread, NULL);	
			inc_pthread_create(&ZjctIncCal_10_ThrdId,NULL,(void *)Zjct_IncCall_10_RxThread, NULL);
			inc_pthread_create(&ZjctIncVoc_10_ThrdId,NULL,(void *)Zjct_IncVoc_10_RxThread, NULL);	
			inc_pthread_create(&ZjctIncCal_11_ThrdId,NULL,(void *)Zjct_IncCall_11_RxThread, NULL);
			inc_pthread_create(&ZjctIncVoc_11_ThrdId,NULL,(void *)Zjct_IncVoc_11_RxThread, NULL);	
			break;
		case WORK_MODE_ADAPTER:
			inc_pthread_create(&rpcXiWeiDatThrdId,NULL,(void *)XiWei_Data_RxThread, NULL);	
			inc_pthread_create(&rpcXiWeiVocThrdId,NULL,(void *)XiWei_Voc_RxThread, NULL);	
        		inc_pthread_create(&rpcAdpPhoneDatThrdId,NULL,(void *)Adp_Phone_Data_RxThread, NULL);	
			inc_pthread_create(&rpcAdpPhoneVocThrdId,NULL,(void *)Adp_Phone_Voc_RxThread, NULL);		
			inc_pthread_create(&rpcSeatDataThrdId,NULL,(void *)Seat_Data_RxThread, NULL);	
			break;
		default:
			break;
	}	

	inc_pthread_create(&rpcDbgThrdId,NULL,(void *)RpcDbg_RxThread, NULL);
	inc_pthread_create(&rpcDisplayThrdId,NULL,(void *)RpcDisplayMng_RxThread, NULL);		
	inc_pthread_create(&rpcSeatMngThrdId,NULL,(void *)RpcSeatMng_RxThread, NULL);
	inc_pthread_create(&rpcBoard716MngThrdId,NULL,(void *)Board_716_Mng_RxThread, NULL);		
	inc_pthread_create(&rpcBoard716RayMngThrdId,NULL,(void *)Board_716_Ray_Mng_RxThread, NULL);
	inc_pthread_create(&rpcBoard716RadioMngThrdId,NULL,(void *)Board_716_Radio_RxThread, NULL);	
	inc_pthread_create(&rpcBoard50MngThrdId,NULL,(void *)Board_50_Mng_RxThread, NULL);	
	inc_pthread_create(&rpcDisplay_zhuangjiaThrdId,NULL,(void *)RpcDisplayMng_Zhuangjia_From_dis_RxThread, NULL);		
	inc_pthread_create(&rpcDisplay_zhuangjia716ThrdId,NULL,(void *)RpcDisplayMng_Zhuangjia_from_716_RxThread, NULL);
	inc_pthread_create(&rpcBoard50NetMngThrdId,NULL,(void *)Board_50_Net_Mng_RxThread, NULL);	

	ParaInject();
	// here, we bind *:30006 in RpcSeatMng_RxThread. so , this one do not need RpcSeatMng_Default_RxThread
	//by -Andy-wei.hou 2017.02.20

	//test_example_board_50();

	inc_pthread_create(&thread_monitor,NULL,do_monitor,NULL);

#ifdef SWITCH_VLAN_CTL
	inc_pthread_create(&uartThrdId,NULL,Uart_Pthread,NULL);	
#endif

	inc_pthread_create(&thread_shell,NULL,do_command,NULL);
	inc_pthread_create(&thread_cfg_wr,NULL,mng_process,NULL);

#ifdef SWITCH_VLAN_CTL
	switch_vlan_init();
#endif

	int i = 0;
	for (i = 0; i < 6; i++) {
		sleep(4);
		DisplayBoardShowVersion();
	}
	
	pthread_join(thread_shell, NULL);
	pthread_join(thread_monitor, NULL);	
	pthread_join(thread_pipe, NULL);	
	pthread_join(thread_rtp, NULL);
	pthread_join(rpcDbgThrdId, NULL);	
	pthread_join(rpcDisplayThrdId, NULL);		
	pthread_join(rpcSeatThrdId, NULL);	
	pthread_join(rpcVoiceThrdId, NULL);	
	pthread_join(uartThrdId, NULL);	
	
	return 0;
}




