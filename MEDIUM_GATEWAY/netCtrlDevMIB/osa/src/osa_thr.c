/******************************************************************************
* FileName: 	 osa_thr.c
* Desc:
* 平台无关的  OSA 抽象层对 Thread 操作的封装实现文件
*
* Author: 	 Andy-wei.hou
* Date: 	 2016/10/26
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2016/10/26, Andy-wei.hou create this file
*
******************************************************************************/
/*-------------------------------- Includes ----------------------------------*/
#include <errno.h>
//#include <unistd.h>

#include "osa_thr.h"
#include "osa_debug.h"



/*-------------------- Global Definitions and Declarations -------------------*/


/*----------------------- Constant / Macro Definitions -----------------------*/


/*------------------------ Type Declarations ---------------------------------*/


/*------------------------ Variable Declarations -----------------------------*/


/*------------------------ LOCAL Function Prototype --------------------------*/


/*------------------------ Function Implement --------------------------------*/
int OSA_ThreadCreate(OSA_ThrHndl *pThrHandle, OSA_ThrEntryFunc ThreadFunc, void * pParamter){
	int rel=0;
    rel = pthread_create(&(pThrHandle->hndl), NULL, ThreadFunc, pParamter);
	if(rel != 0){
		OSA_ERROR("Create Thread Fail,errno=%d",errno);
	}
	return rel;
}

int OSA_thrJoin(OSA_ThrHndl *hndl)
{
    int status = OSA_SOK;
    void *returnVal;

    if(NULL == hndl)
    {
        OSA_ERROR("Invalid parameter...");
        return OSA_EFAIL;
    }
#if 0
    if(OSA_THREAD_HANDLE_INVLALID == hndl->hndl )
    {
        OSA_ERROR("Invalid pthread handle...");
        return OSA_EFAIL;
    }
#endif
    status =pthread_join ((hndl->hndl), &returnVal);

    if(EINVAL == status)
    {
        //OSA_ERROR("The target thread 0x%x is Detach", (int)(hndl->hndl));
        status = OSA_SOK;
    }

//    OSA_DBG_MSG(" %s status = %d", __func__ , status);

    return status;
}
int OSA_thrDelete(OSA_ThrHndl *hndl)
{
    int status = OSA_SOK;

    if(NULL == hndl)
    {
        OSA_ERROR("Invalid parameter...");
        return OSA_EFAIL;
    }
#if 0
    if(OSA_THREAD_HANDLE_INVLALID == hndl->hndl )
    {
        OSA_ERROR("Invalid pthread handle...");
        return OSA_EFAIL;
    }
#endif
    status |= pthread_cancel(hndl->hndl);
	//OSA_DBG_MSG(" %s cancel 0x%x handle status = %d", __func__, (int)hndl->hndl, status);

    status |= OSA_thrJoin(hndl);
	//OSA_DBG_MSG(" %s cancel 0x%x handle status = %d", __func__, (int)hndl->hndl, status);

    return status;
}


int OSA_CondEventInit(OSA_COND_EVENT_HANDLE *pOSA_COND_EVENT_HANDLE)
{
    int iRet = OSA_SOK;
    
    if(NULL == pOSA_COND_EVENT_HANDLE)
    {
        OSA_ERROR("Invalid parameter...");
        iRet = OSA_EFAIL;
        return iRet;
    }
    iRet = pthread_mutex_init(&(pOSA_COND_EVENT_HANDLE->stMutex), NULL);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_init fail... error=%d",iRet);        
        iRet = OSA_EFAIL;
        goto __EXIT__;        
    }
    iRet = pthread_cond_init(&(pOSA_COND_EVENT_HANDLE->stCond), NULL);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_cond_init fail... error=%d",iRet);        
        iRet = OSA_EFAIL;
        goto __EXIT__;        
    }

    if(OSA_SOK == iRet)
    {
        OSA_DBG_MSG("OSA_CondEventInit call successfully!");
    }

__EXIT__:
	
    return iRet;

}

int OSA_CondEventDestroy(OSA_COND_EVENT_HANDLE *pOSA_COND_EVENT_HANDLE)
{
    int iRet = OSA_SOK;
    
    if(NULL == pOSA_COND_EVENT_HANDLE)
    {
        OSA_ERROR("Invalid parameter...");
        iRet = OSA_EFAIL;
        goto __EXIT__;
    }

    iRet = pthread_mutex_lock(&(pOSA_COND_EVENT_HANDLE->stMutex));
    OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int *)&(pOSA_COND_EVENT_HANDLE->stMutex), (int)&pOSA_COND_EVENT_HANDLE->stMutex);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_lock fail... error=%d",iRet); 
        iRet = OSA_EFAIL;

        goto __EXIT__;
    }

    iRet = pthread_cond_destroy(&(pOSA_COND_EVENT_HANDLE->stCond));
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_cond_destroy fail... error=%d",iRet); 
        iRet = OSA_EFAIL;
    }
    
    iRet = pthread_mutex_unlock(&(pOSA_COND_EVENT_HANDLE->stMutex));
    OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int *)&(pOSA_COND_EVENT_HANDLE->stMutex), (int)&pOSA_COND_EVENT_HANDLE->stMutex);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_unlock fail... error=%d",iRet); 
        iRet = OSA_EFAIL;

        goto __EXIT__;
    }
    
    iRet = pthread_mutex_destroy(&(pOSA_COND_EVENT_HANDLE->stMutex)); 
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_destroy fail... error=%d",iRet);
        iRet = OSA_EFAIL;
    }

__EXIT__:
    return iRet;
}


int OSA_SetCondEvent(OSA_COND_EVENT_HANDLE *pOSA_COND_EVENT_HANDLE)
{
    int iRet = OSA_SOK;
    
    if(NULL == pOSA_COND_EVENT_HANDLE)
    {
        OSA_ERROR("Invalid parameter...");
        iRet = OSA_EFAIL;
        goto __EXIT__;
    }
    OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int *)&(pOSA_COND_EVENT_HANDLE->stMutex), (int)&pOSA_COND_EVENT_HANDLE->stMutex);
    iRet = pthread_mutex_lock(&(pOSA_COND_EVENT_HANDLE->stMutex));
    OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int *)&(pOSA_COND_EVENT_HANDLE->stMutex), (int)&pOSA_COND_EVENT_HANDLE->stMutex);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_lock fail... error=%d",iRet); 
        iRet = OSA_EFAIL;

        goto __EXIT__;
    }
 

    OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stCond(%d)", (int)&pOSA_COND_EVENT_HANDLE->stCond);
    iRet = pthread_cond_signal(&(pOSA_COND_EVENT_HANDLE->stCond));
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_cond_signal fail... error=%d",iRet); 
        iRet = OSA_EFAIL;
    }
    
    OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int *)&(pOSA_COND_EVENT_HANDLE->stMutex), (int)&pOSA_COND_EVENT_HANDLE->stMutex);
    iRet = pthread_mutex_unlock(&(pOSA_COND_EVENT_HANDLE->stMutex));
    OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int *)&(pOSA_COND_EVENT_HANDLE->stMutex), (int)&pOSA_COND_EVENT_HANDLE->stMutex);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_unlock fail... error=%d",iRet); 
        iRet = OSA_EFAIL;

    }
   
__EXIT__:
    if(OSA_SOK == iRet)
    {
        OSA_DBG_MSG("OSA_SetCondEvent call successfully!");
    }
    
    return iRet;    
}

/*
* 0, return success
*-1, system API errror or invalid parameter
*-2, timetout
*/
int OSA_WaitCondEvent(OSA_COND_EVENT_HANDLE *pOSA_COND_EVENT_HANDLE, int iTimeOutSec)
{
    int iRet = OSA_SOK;
    int iResult = OSA_SOK;
    struct timespec waittime;

    if((NULL == pOSA_COND_EVENT_HANDLE) || iTimeOutSec < 0)
    {
        OSA_ERROR("Handle Invalid !");
        iRet = OSA_EFAIL;        
        return iRet;
    }

    OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int *)&(pOSA_COND_EVENT_HANDLE->stMutex), (int)&pOSA_COND_EVENT_HANDLE->stMutex);
    iRet = pthread_mutex_lock(&(pOSA_COND_EVENT_HANDLE->stMutex));
    OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int *)&(pOSA_COND_EVENT_HANDLE->stMutex), (int)&pOSA_COND_EVENT_HANDLE->stMutex);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_lock fail... error=%d",iRet); 
        iResult = OSA_EFAIL;

        goto __EXIT__;
    }

    switch(iTimeOutSec)
    {
        case 0://永久等待
        {
	        OSA_DBG_MSG("Wait condition forerver...");
            OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int *)&(pOSA_COND_EVENT_HANDLE->stMutex), (int)&pOSA_COND_EVENT_HANDLE->stMutex);
            iRet = pthread_cond_wait(&(pOSA_COND_EVENT_HANDLE->stCond), &(pOSA_COND_EVENT_HANDLE->stMutex));
            OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int *)&(pOSA_COND_EVENT_HANDLE->stMutex), (int)&pOSA_COND_EVENT_HANDLE->stMutex);
            
            if(OSA_SOK != iRet)
            {
                OSA_ERROR("Call System API pthread_cond_wait return val=%d",iRet);
                iResult = OSA_EFAIL;
                goto __EXIT__ ;
            }
            
        }              
        default://正常超时值
        {   
         	waittime.tv_sec = time(NULL)+iTimeOutSec;  
			waittime.tv_nsec = 0;  
            OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int *)&(pOSA_COND_EVENT_HANDLE->stMutex), (int)&pOSA_COND_EVENT_HANDLE->stMutex);
            iRet = pthread_cond_timedwait(&(pOSA_COND_EVENT_HANDLE->stCond), &(pOSA_COND_EVENT_HANDLE->stMutex),&waittime);
            OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int *)&(pOSA_COND_EVENT_HANDLE->stMutex), (int)&pOSA_COND_EVENT_HANDLE->stMutex);
            if( ETIMEDOUT == iRet )
            {//超时
                iResult = OSA_TIMEOUT;
                break;
            }
            else
            {
                if(OSA_SOK == iRet)//在超时时间内得到 singal
                {
                	iResult = OSA_SOK;
                }
                else
                {//其他错误
                    OSA_ERROR("call pthread_cond_timedwait fail... error=%d",iRet); 
                    iResult = OSA_EFAIL;

                    goto __EXIT__ ;
                }
            } 
        }
    }
__EXIT__:    
    OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int *)&(pOSA_COND_EVENT_HANDLE->stMutex), (int)&pOSA_COND_EVENT_HANDLE->stMutex);
    iRet = pthread_mutex_unlock(&(pOSA_COND_EVENT_HANDLE->stMutex));
    OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int *)&(pOSA_COND_EVENT_HANDLE->stMutex), (int)&pOSA_COND_EVENT_HANDLE->stMutex);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_unlock fail... error=%d",iRet); 
    }   

    
    return iResult;
}


