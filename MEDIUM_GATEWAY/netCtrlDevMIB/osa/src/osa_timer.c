/******************************************************************************
 * Copyright 2010-2011 ABB Genway Co.,Ltd.
 * FileName:      osa_timer.c
 * Desc:    平台无关定时器实现
 *
 *
 * Author:    Andy-wei.hou
 * Date:      2016/10/26
 * Notes:
 *
 * -----------------------------------------------------------------
 * Histroy: v1.0   2016/10/26, Andy-wei.hou create this file
 ******************************************************************************/
#include <errno.h>
#include "osa_timer.h"
#include "osa.h"


/*-------------------- Global Definitions and Declarations -------------------*/


/*----------------------- Constant / Macro Definitions -----------------------*/


/*------------------------ Type Declarations ---------------------------------*/


/*------------------------ Variable Declarations -----------------------------*/
//OSA_ThrHndl Signfn_thr;
static OSA_ThrHndl Signfn_thr ;

static struct timer_list timerList;
static bool bTimerFuncRunning = TRUE;

/*------------------------ Function Prototype --------------------------------*/

static void sig_func(int signo);

/*------------------------ Function Implement --------------------------------*/
/******************************************************************************
* Name:      init_timer
*
* Desc:      Create a timer list.
* Param:
* Return:    0 means ok, the other means fail.
* Global:
* Note:
* Author:    Andy-wei.hou
* -------------------------------------
* Log:   2010/08/19, Nancy-xiaofeng.xu Create this function
 ******************************************************************************/
int osa_init_timer(int count)
{
	int ret = 0;


	if(count <=0 || count > MAX_TIMER_NUM)
    {
		printf("the timer max number MUST less than %d.\n", MAX_TIMER_NUM);
		return -1;
	}
	//for(i=0;i<MAX_TIMER_NUM;i++)
	{
    	memset(&timerList, 0, sizeof(struct timer_list));
	}
    timerList.num = 0;
	timerList.max_num = count;
    OSA_mutexCreate(&timerList.Mutex);

    bTimerFuncRunning = TRUE;
    ret = OSA_ThreadCreate(&Signfn_thr,(void *)sig_func,NULL);
	OSA_DBG_MSG("Timer thread created");
    if(ret != OSA_SOK)
	{
		OSA_ERROR("Create SW Rtc thread error!\n");
//        ASSERT(0);
        return FALSE;
	}
	return ret;
}

/******************************************************************************
* Name:      destroy_timer
*
* Desc:      Destroy the timer list.
* Param:
* Return:    0 means ok, the other means fail.
* Global:
* Note:
* Author:    Andy-wei.hou
* -------------------------------------
* Log:   2010/08/19, Nancy-xiaofeng.xu Create this function
 ******************************************************************************/

int osa_destroy_timer(void)
{

    bTimerFuncRunning = FALSE;
    OSA_thrDelete(&Signfn_thr);
	memset(&timerList, 0, sizeof(struct timer_list));

	return 0;
}

/******************************************************************************
* Name:      add_timer
*
* Desc:      Add a timer to timer list.
* Param:
*      interval   The timer interval.
                  others:100ms (eg:if set a timer last for 500ms ,the interval is 5)
*      cb         When cb!= NULL and timer expiry, call it.
*      user_data  Callback's param.
*      len        The length of the user_data.

* Return:    The timer ID, if == INVALID_TIMER_ID, add timer fail.
* Global:
* Note:
* Author:    Andy-wei.hou
* -------------------------------------
* Log:   2010/08/19, Nancy-xiaofeng.xu Create this function
 ******************************************************************************/
timer_id osa_add_timer(int interval, timer_expiry *cb, void *user_data, TIMER_TYPE times)
{
	struct timer *node = NULL;
    int i =0;
    int len;

    len = sizeof(int);

    if (cb == NULL || interval <= 0)
    {
		return INVALID_TIMER_ID;
	}
    OSA_DBG_MSG("add_timer  timerList.num=%d,timerList.max_num=%d\n",timerList.num,timerList.max_num);

    OSA_mutexLock(&timerList.Mutex);
    if(timerList.num < timerList.max_num)
    {
		timerList.num++;
	}
    else
    {
        OSA_mutexUnlock(&timerList.Mutex);
		return INVALID_TIMER_ID;
	}

    for(i=0;i<MAX_TIMER_NUM;i++)
    {
        node =  &timerList.list[i];
        if(node->used_flag == 0)
        {
            if(user_data != NULL && len != 0)
            {
                #if 1
                node->user_data = user_data;
                #else
        		node->user_data = MemMalloc(len);
NOT_MEM_PRINT(node->user_data);
                DEBUG_MEM(("MEM-MALLOC-017 %x\n", (INT32)node->user_data));
        		memcpy(node->user_data, user_data, len);
                #endif
        		node->len = len;
        	}
            else
            {
                node->user_data = NULL;
                node->len =0 ;
            }

        	node->cb = cb;
            node->interval = interval;
        	node->elapse = 0;
        	node->id = i+1;
            node->used_flag =1;
            node->times = times;
            node->index ++ ;
            if(node->index >=300)
            {
                node->index = 0;
            }
            OSA_mutexUnlock(&timerList.Mutex);
            return node->id;
        }
    }
    OSA_mutexUnlock(&timerList.Mutex);
    return INVALID_TIMER_ID;

}

/******************************************************************************
* Name:      del_timer
*
* Desc:      Delete a timer from timer list.
* Param:     timer_id  	    The timer ID.
* Return:    0 means ok, the other means fail.
* Global:
* Note:
* Author:    Nancy-xiaofeng.Xu
* -------------------------------------
* Log:   2010/08/19, Nancy-xiaofeng.xu Create this function
 ******************************************************************************/
int osa_del_timer(timer_id id)
{
    int i = 0;
    struct timer *node = NULL;

	if (id <0 || id >MAX_TIMER_NUM)
    {
		return -1;
	}

    if(timerList.num<=0)
    {
        return -1;
    }

    OSA_mutexLock(&timerList.Mutex);
	for ( i=0;i<MAX_TIMER_NUM;i++)
    {
        node =  &timerList.list[i];
        if(node->used_flag)
        {
    		if (id == node->id)
            {
                if(node->user_data != NULL)
                {
                    #if 0
                    DEBUG_MEM(("MEM-FREE-017 %x\n", (INT32)node->user_data));
                    SAFE_DELETE_MEM(node->user_data);
                    #endif
                    node->user_data = NULL;
                }
                node->cb = NULL;
                node->elapse =0 ;
                node->id =0;
                node->interval =0;
                node->used_flag=0;
                node->times = 0;
                if(timerList.num>0)
                {
                    timerList.num--;
                }
                OSA_mutexUnlock(&timerList.Mutex);
    			return 0;
    		}
        }
	}
	/* Can't find the timer */
    OSA_mutexUnlock(&timerList.Mutex);
	return -1;
}
/******************************************************************************
* Name:      sig_func
*
* Desc:
* Param:
* Return:
* Global:
* Note:
* Author:    Nancy-xiaofeng.Xu
* -------------------------------------
* Log:   2010/08/19, Nancy-xiaofeng.xu Create this function
 ******************************************************************************/
static void sig_func(int signo)
{
    int i;
    struct timer *node = NULL;
    int tmp_index;
    int iRet =0 ;
    struct timeval stTimeout = {0};
	fd_set fdRead;
	SOCKET_TYPE sockfd = -1;

	/*As in Win32 Platform, if fd_set_read of select is NULL or is empty will lead to select fail,
	**So, we has to create a socket and then add it to fd_set_read, then invoked select.
	** by Andy-wei.hou 2016.11.10
	*/
	sockfd = osa_udpCreateSock();
    while(bTimerFuncRunning)
    {
		FD_ZERO(&fdRead);
		FD_SET(sockfd, &fdRead);
		
        stTimeout.tv_sec = 0 ;
        stTimeout.tv_usec = 100*1000;	/*timeout 100ms*/
		//iRet = select(0, &fdRead, NULL, NULL, &stTimeout) ;
		if((iRet = select(0, &fdRead, NULL, NULL, NULL))== SOCKET_ERROR){
			OSA_ERROR("Select Fail Errno=%d",errno);
		}
        if (0 == iRet)//超时
        {
        	for (i=0;i<MAX_TIMER_NUM;i++)
            {
                node =  &timerList.list[i];
                tmp_index = node->index;
                if(node->used_flag)
                {
            		node->elapse++;
                    //printf("sig_func  00000000000000000000000--node->elapse=%d\n",node->elapse);
            		if(node->elapse == node->interval)
                    {
                        //OSA_DBG_MSG("sig_func  00000000000000000000000------01 id=%d type=%d\n",node->id,node->times);
                        if(node->cb != NULL && node->id != 0)
                        {
                            #if 1   //def YELLOWRIVERO7
                            if(node->cb != NULL)
                            {
                                node->cb(node->user_data);
                            }
                            #else
                            if(node->cb != NULL)
                            {
                			    node->cb(node->id, node->user_data, node->len);
                            }
                            #endif
                            //pthread_create(&Signfn_thr.hndl, NULL, (void*)Timer_Fn, (void*)node);
                            //pthread_join(Signfn_thr.hndl,NULL);
                            OSA_mutexLock(&timerList.Mutex);
                            if(node->times == TIMER_ONCE)     // 1 once  2 continue
                            {
                                if(tmp_index == node->index
                                    && node->used_flag == 1)
                                {
                                    printf("self del the timer\n");
                                    //SAFE_DELETE_MEM(node->user_data);
                                    node->user_data = NULL;
                                    node->elapse = 0;
                                    node->cb = NULL;
                                    node->used_flag =0;
                                    node->times = 0;
                                    if(timerList.num>0)
                                    {
                                        timerList.num--;
                                    }
                                }
                            }
                            else if(node->times == TIMER_REPEAT)
                            {
                                node->elapse = 0;
                            }
                            OSA_mutexUnlock(&timerList.Mutex);
                        }
                        //OSA_DBG_MSG("sig_func  00000000000000000000000------02\n");
            		}
                }

        	}
        }
    }
}


