/****
*Desc: 	Implement a syn mechanism through multi-thread by pthread_cond.
*		This mechanism provide order compare and multi-syncs(if initQueryEvent with size >1)
*Author: Andy-wei.hou
*Log:	2017.03.20, Andy-wei.hou Created this file
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define INVALID_ORDER 0

typedef struct {
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
	struct timespec expTimer;
}COND_EVENT;


typedef struct {
	COND_EVENT	event;			/*cond_event to syn query and response*/
	unsigned int 	order;			/*related compared response order*/
}QUERY_EVENT;


/***********Global Variablies*******************/
QUERY_EVENT	*g_pQueryEventList;	/*waiting conding list*/
int				 g_listSize = 1;		/*number of node in the waiting list*/
pthread_mutex_t 	 g_mutex;			/*mutex to access g_pQueryEventList*/



int initCond(COND_EVENT *event){	
	pthread_condattr_t attr;
	int res = -1;
	
	if(!event){
		printf("%s() Invalid Para\r\n",__func__);
		return -1;
	}
	res = pthread_condattr_init(&attr);
	if (res != 0) {
		perror("Attribute creation failed\r\n");
		return -1;
	}
	/*as pthread_cond_timedwait() will be in trouble if in realTime mode while user changed system time*/
	/*So we using clock_monotonic to avoiding this problem*/
	res = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
	if(res != 0){
		perror("PthreadCondAttrSetClock fail \r\n");
		return -1;
	}

	
	pthread_mutex_init(&(event->mutex), NULL);
  	pthread_cond_init(&(event->cond), &attr);
	event->expTimer.tv_sec = 0;
	event->expTimer.tv_nsec = 0;
	return 0;
}


QUERY_EVENT *getFreeQueryEvent(){
	int  i = 0;
	QUERY_EVENT *queryEvent = NULL;
	pthread_mutex_lock(&g_mutex);
	for(i = 0; i< g_listSize; i++){
		queryEvent = &g_pQueryEventList[i];
		//printf("%s() QueryEvent=%p \r\n",__func__,queryEvent);
		if(queryEvent && queryEvent->order == INVALID_ORDER)
			break;
	}
	if(i >= g_listSize)
		queryEvent = NULL;
	pthread_mutex_unlock(&g_mutex);
	return queryEvent;
}

void freeQueryEvent(QUERY_EVENT *queryEvent){
	if(!queryEvent)
		return;
	//printf("%s(), queryEvent=%p\r\n",__func__,queryEvent);
	pthread_mutex_lock(&g_mutex);
	queryEvent->order = INVALID_ORDER;
	queryEvent->event.expTimer.tv_sec = 0;
	queryEvent->event.expTimer.tv_nsec = 0;
	pthread_mutex_unlock(&g_mutex);
}

void setQueryEvent(QUERY_EVENT *queryEvent, unsigned int order, int expTimMs){
	struct timespec now;
	pthread_mutex_lock(&g_mutex);
	clock_gettime(CLOCK_MONOTONIC, &now);
	//printf("Now Time=(%d_%d) \r\n",now.tv_sec,now.tv_nsec);
	queryEvent->event.expTimer.tv_sec = now.tv_sec + (int)(expTimMs/1000);
	queryEvent->event.expTimer.tv_nsec = now.tv_nsec+((expTimMs%1000)*1000*1000);
	//printf("exp Time=(%d_%d) \r\n",queryEvent->event.expTimer.tv_sec ,queryEvent->event.expTimer.tv_nsec );
	queryEvent->order = order;
	pthread_mutex_unlock(&g_mutex);
	printf("%s() QueryEvent=%p ,order =%04x\r\n",__func__,queryEvent,queryEvent->order);
}


int waitQueryEventTimed(unsigned int order, int expTimMs){
	QUERY_EVENT *queryEvent;
	int iRet = -1;

	printf("%s(), order=%04x, exp=%d ms\r\n", __func__, order,expTimMs);
	
	queryEvent = getFreeQueryEvent();
	if(!queryEvent){
		printf("Can't get free QueryEvent, discard the request \r\n");
		return iRet;
	}
	
	setQueryEvent(queryEvent,order,expTimMs);
	pthread_mutex_lock(&(queryEvent->event.mutex));
	iRet = pthread_cond_timedwait(&(queryEvent->event.cond), &(queryEvent->event.mutex), &(queryEvent->event.expTimer));
	if(iRet == 0){
		printf("wakedUp by signal \r\n");
	}else if(iRet == ETIMEDOUT){
		iRet = -2;
		printf("Timeout without signal!----------------------------------------------\r\n");
	}else{
		printf("pthread_cond_timedwait error ,errno=%d \r\n",errno);
		iRet = -3;
	}
	//printf("pthread_cond_timedwait return =%d\r\n",iRet);
	pthread_mutex_unlock(&(queryEvent->event.mutex));

	freeQueryEvent(queryEvent);
	return iRet;
}


int signalQueryEvent( unsigned char respOrder){
	int iRet = -1;
	int i = 0;
	QUERY_EVENT *queryEvent = NULL;
	if(respOrder == INVALID_ORDER)
	{
		printf("%s(), Invliad Para \r\n", __func__);
		return iRet;
	}
	pthread_mutex_lock(&(g_mutex));
	for(i = 0 ; i< g_listSize; i++){
		queryEvent = &g_pQueryEventList[i];
		//printf("Test QueryEvent=%p \r\n",queryEvent);
		if(queryEvent &&queryEvent->order != INVALID_ORDER && respOrder == queryEvent->order){
			iRet = 0;
			printf("%s(), order=%04x\r\n", __func__, respOrder);
			pthread_mutex_lock(&(queryEvent->event.mutex));
	   	 	pthread_cond_signal(&(queryEvent->event.cond));
			pthread_mutex_unlock(&(queryEvent->event.mutex));
		}
	}	
	pthread_mutex_unlock(&(g_mutex));
	if(i >= g_listSize)
		iRet = -2;
					   
	return iRet;			   

	
}

int initQueryEvent(int size){
	int res = -1;
	int i = 0;
	QUERY_EVENT *queryEvent = NULL;
	if(size <=0){
		printf("%s() Invalid Para\r\n",__func__);
		return -1;
	}

	
	g_listSize = size;
	g_pQueryEventList  = (QUERY_EVENT *) malloc(sizeof(QUERY_EVENT) *g_listSize);
	if(!g_pQueryEventList){
		printf("%s_%d, malloc fail, errno=%d \r\n", __FILE__,__LINE__, errno);
		return -1;
	}
	pthread_mutex_init(&(g_mutex),NULL);
	for(i = 0; i < g_listSize; i++){
		queryEvent = &g_pQueryEventList[i];
		//printf("%s()_queryEvent=%p\r\n",queryEvent);
		res = initCond(&(queryEvent->event));
		queryEvent->order = INVALID_ORDER;
	}
	printf("%s()_g_pQueryEventList=%p, g_listSize=%d \r\n",__func__, g_pQueryEventList, g_listSize);
	return res;
}



















