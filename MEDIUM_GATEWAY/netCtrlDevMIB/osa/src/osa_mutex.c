/******************************************************************************
* Copyright 2010-2011 ABB Genway Co.,Ltd.
* FileName: 	 osa_mutex.c 
* Desc:
* 
* 
* Author: 	 Davis Chen
* Date: 	 2010/06/18
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2010/06/18, Davis Chen create this file
* 
******************************************************************************/
/*-------------------------------- Includes ----------------------------------*/
#include "commonTypes.h"
#include "osa_mutex.h"
#include "osa_debug.h"



/*-------------------- Global Definitions and Declarations -------------------*/


/*----------------------- Constant / Macro Definitions -----------------------*/


/*------------------------ Type Declarations ---------------------------------*/


/*------------------------ Variable Declarations -----------------------------*/


/*------------------------ LOCAL Function Prototype --------------------------*/


/*------------------------ Function Implement --------------------------------*/

/******************************************************************************
* Name: 	 OSA_mutexCreate 
*
* Desc: 	 创建 Mutex (互斥量)
* Param: 	 hndl 【in】 -- 传入的互斥量的指针
* Return: 	 创建成功 返回 OSA_SOK
             其它值表示失败
* Global: 	 
* Note: 	 
* Author: 	 Andy-wei.hou
* -------------------------------------
* Log: 	 2016/10/26, Create this function by Andy-wei.hou
 ******************************************************************************/
int OSA_mutexCreate(OSA_MutexHndl *hndl)
{
  pthread_mutexattr_t mutex_attr;
  int status=OSA_SOK;
 
  status |= pthread_mutexattr_init(&mutex_attr);
  status |= pthread_mutex_init(&hndl->lock, &mutex_attr);
  
  if(status!=OSA_SOK)
    OSA_ERROR("OSA_mutexCreate() = %d \r\n", status);

  pthread_mutexattr_destroy(&mutex_attr);
    
  return status;
}
/******************************************************************************
* Name: 	 OSA_mutexDelete 
*
* Desc: 	 删除Mutex 对象
* Param: 	 hndl [in] -- 要删除的 mutex 对象句柄
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Andy-wei.hou
* -------------------------------------
* Log: 	 2016/10/26, Create this function by Andy-wei.hou
 ******************************************************************************/
int OSA_mutexDelete(OSA_MutexHndl *hndl)
{
  pthread_mutex_destroy(&hndl->lock);  

  return OSA_SOK;
}


/******************************************************************************
* Name: 	 OSA_mutexLock 
*
* Desc: 	 互斥Lock
* Param: 	 hndl [in] -- 要lock的 mutex 对象句柄
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Andy-wei.hou
* -------------------------------------
* Log: 	 2016/10/26, Create this function by Andy-wei.hou
 ******************************************************************************/
int OSA_mutexLock(OSA_MutexHndl *hndl)
{
  return pthread_mutex_lock(&hndl->lock);
}

/******************************************************************************
* Name: 	 OSA_mutexUnlock 
*
* Desc: 	 互斥Unlock
* Param: 	 hndl [in] -- 要Unlock的 mutex 对象句柄
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Andy-wei.hou
* -------------------------------------
* Log: 	 2016/10/26, Create this function by Andy-wei.hou
 ******************************************************************************/
int OSA_mutexUnlock(OSA_MutexHndl *hndl)
{
  return pthread_mutex_unlock(&hndl->lock);
}

