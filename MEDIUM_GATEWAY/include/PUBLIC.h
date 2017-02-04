#ifndef _PUBLIC_H_
#define _PUBLIC_H_


#include "common/autoconfig.h"
//#include "common/compat.h"
//#include "common/paths.h"
#include "common/stringfields.h"
#include "common/inc_obj.h"
//#include "common/options.h"
#include "common/logger.h"
#include "common/linkedlists.h"
#include "common/threadstorage.h"
#include "common/utils.h"
#include "common/sched.h"
#include "common/io.h"


#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <termios.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <malloc.h> 
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include "common.h"
#include "exterior_protocol.h"
#include "interior_protocol.h"
#include "MGW_command.h"
#include "MGW_common.h"
#include "MGW_config.h"
#include "MGW_time.h"
#include "MGW_call_control.h"
#include "MGW_rcb.h"
#include "sipag_common.h"
#include "MGW_protocol_parse.h"
#include "gpio_api.h"
#include "common/logger.h"
#include "uart_api.h"
#include "zjct_proc.h"
#include "phone.h"
#include "adapter.h"
#include "MGW_lan_bus_control.h"
#include "pcm_comm.h"
//#include "AC49xDrv_Definitions.h"
//#include "AC49xDrv_Drv.h"


#endif
