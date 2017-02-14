
#ifndef __OSA_TIMER_H__
#define __OSA_TIMER_H__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>


#include "osa.h"

/*------------------------------ Global Defines ------------------------------*/
#define MAX_TIMER_NUM		40
#define TIMER_START 		1
#define TIMER_TICK 		1
#define INVALID_TIMER_ID 	(-1)

#ifndef TRUE
#define TRUE 	1
#endif

#ifndef FALSE
#define FALSE	0
#endif


/*------------------------------ Global Typedefs -----------------------------*/
typedef void timer_expiry(void *user_data);
typedef int timer_id;

typedef enum{
	// 1 once  2 continue
	TIMER_ONCE = 1,
	TIMER_REPEAT = 2
}TIMER_TYPE;

struct timer
{
	timer_id id;			/**< timer id		*/
	int interval;
	int elapse;
	timer_expiry *cb;		/*callback funtion*/
	void *user_data;		/**º¯Êý²ÎÊý*/
	int len;			/**user_data length	*/
    int used_flag;
    int index;
    TIMER_TYPE times; //continue or once
};

#ifndef __OSA_MUTEX_STRUCT
#define __OSA_MUTEX_STRUCT
typedef struct osaMutex_tag
{
	pthread_mutex_t lock;
}OSA_MutexHndl;
#endif

struct timer_list
{
	struct timer  list[MAX_TIMER_NUM];
	int num;
	int max_num;
	void (*old_sigfunc)(int);
	void (*new_sigfunc)(int);
    OSA_MutexHndl Mutex;
};

/*----------------------------- External Variables ---------------------------*/

/*------------------------- Global Function Prototypes -----------------------*/

int osa_init_timer(int count);

int osa_destroy_timer(void);

timer_id osa_add_timer(int interval, timer_expiry *cb, void *user_data, TIMER_TYPE times);

int osa_del_timer(timer_id id);






#endif

