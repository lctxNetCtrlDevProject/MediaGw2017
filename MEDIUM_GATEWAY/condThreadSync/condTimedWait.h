#ifndef __COND_TIMED_WAIT_H__
#define __COND_TIMED_WAIT_H__

/*init the waiting list, size is the max waiting node in the list*/
extern int initQueryEvent(int size);

/*
*Func:		waitQueryEventTimed
*Desc: 		wait until equal order signal or expTimMs expired
*Para:	  	order, order code that will be compare during signal
*		 	expTimMs, waiting expired time, ms
*return:  	0, wakeup by signal
		  	-1, invalid para
		  	-2, wakeup by timeout
		  	-3, other error occur, pls check errno
*/
extern int waitQueryEventTimed(unsigned char order, int expTimMs);

/*singal the wait event by respOrder equalled*/
extern int signalQueryEvent(unsigned char respOrder);

#endif
