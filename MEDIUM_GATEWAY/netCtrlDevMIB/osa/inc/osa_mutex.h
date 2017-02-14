/******************************************************************************
* FileName: 	 osa_mutex.h 
* Desc: 平台无关的互斥(Mutex)相关API封装定义头文件

* 
* 
* Author: 	 Andy-wei.hou
* Date: 	 2016.10.25
* Notes: 	 For windows platform, based on pthreadVc2 open source project
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2016.10.26, Andy-wei.hou Create this file
* 
******************************************************************************/
#ifndef __OSA_MUTEX_H__
#define __OSA_MUTEX_H__
#include <pthread.h>
#include "osa.h"


/*------------------------------ Global Defines ------------------------------*/


/*------------------------------ Global Typedefs -----------------------------*/
#ifndef __OSA_MUTEX_STRUCT
#define __OSA_MUTEX_STRUCT
typedef struct osaMutex_tag
{
	pthread_mutex_t lock;
}OSA_MutexHndl;
#endif

typedef enum __osaMutexType_tag
{
	eMutexNormal ,
	eMutexErrorCheck,
	eMutexRecursive, 
	eMutexMax
}E_OSA_MUTEXTYPE;

/*----------------------------- External Variables ---------------------------*/



/*------------------------- Global Function Prototypes -----------------------*/
extern int OSA_mutexCreate(OSA_MutexHndl *hndl);
extern int OSA_mutexDelete(OSA_MutexHndl *hndl);
extern int OSA_mutexLock(OSA_MutexHndl *hndl);
extern int OSA_mutexUnlock(OSA_MutexHndl *hndl);


#endif



