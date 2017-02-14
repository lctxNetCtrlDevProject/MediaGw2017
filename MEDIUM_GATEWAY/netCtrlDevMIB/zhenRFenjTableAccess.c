/**
*@Desc: using Mng protocol to fetch zhenRuFenJiTable from netCtrlDev
*@Author: Andy-wei.hou
*@Log:	  2017.02.13, created by Andy-wei.hou
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "zhenRFenjTableAccess.h"
#include "paraMng.h"
#include "osa.h"

/**************Macro**************************/
#define ZRFJ_FETCH_TIMEOUT		5*10		/* fetching time out 5s*/

/************Global Variablies*******************/
zhenRFJTab_type g_zrfjTab[ZHENR_FENJI_ITEM_MAX];
int 			      g_zrfjTabItemCnt = 0;
OSA_MutexHndl    g_zrfjSynMutex;		/*mutex using to syn the query & response*/

void initZrfjTab(){
	memset(&g_zrfjTab,0xFF, sizeof(g_zrfjTab));
	g_zrfjTabItemCnt = 0;
	OSA_mutexCreate(&g_zrfjSynMutex);
	OSA_mutexLock(&g_zrfjSynMutex);		/*decrease mutex to 0*/
}

void setZrfjTab(zhenRFJTab_type tab[], int itemCnt){
	if(!tab || itemCnt <=0 )
		return;
	g_zrfjTabItemCnt = itemCnt;
	memcpy(&g_zrfjTab,tab,itemCnt*sizeof(zhenRFJTab_type));
	OSA_mutexUnlock(&g_zrfjSynMutex);
}

static void fetchZrfjTabTimeout(){
	memset(&g_zrfjTab,0xFF, sizeof(g_zrfjTab));
	g_zrfjTabItemCnt = 0;
	OSA_mutexUnlock(&g_zrfjSynMutex);
}

zhenRFJTab_type *getGZRFJTab(int *itemCnt){
	sndQueryZrfjTab();
	osa_add_timer(ZRFJ_FETCH_TIMEOUT,fetchZrfjTabTimeout,NULL,TIMER_ONCE);
	OSA_mutexLock(&g_zrfjSynMutex);
	*itemCnt = g_zrfjTabItemCnt;
	return g_zrfjTab;
}



