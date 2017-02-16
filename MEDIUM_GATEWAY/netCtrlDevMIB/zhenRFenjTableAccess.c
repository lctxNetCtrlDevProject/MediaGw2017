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
#define ZRFJ_FETCH_TIMEOUT		1*10		/* fetching time out 5s*/

/************Global Variablies*******************/
zhenRFJTab_type g_zrfjTab[ZHENR_FENJI_ITEM_MAX];
int 			      g_zrfjTabItemCnt = 0;
OSA_MutexHndl    g_zrfjSynMutex;		/*mutex using to syn the query & response*/
int			      g_zrfjTimerID;

void initZrfjTab(){
#if 1
	memset(g_zrfjTab,0xFF, sizeof(g_zrfjTab));
	g_zrfjTabItemCnt = 0;
	OSA_mutexCreate(&g_zrfjSynMutex);
	OSA_mutexLock(&g_zrfjSynMutex);		/*decrease mutex to 0*/
#else
	int i =0;
	g_zrfjTabItemCnt = ZHENR_FENJI_ITEM_MAX;
	for(i =0; i < g_zrfjTabItemCnt; i++){
		g_zrfjTab[i].zjID = i;
		g_zrfjTab[i].fenJID = i*10*random();
	}

#endif
}

static void freeZrfjTimer(){
	if(g_zrfjTimerID != -1){
		osa_del_timer(g_zrfjTimerID);
		g_zrfjTimerID = -1;
	}
}

void setZrfjTab(zhenRFJTab_type tab[], int itemCnt){
	if(!tab || itemCnt <=0 )
		return;
	freeZrfjTimer();
	
	g_zrfjTabItemCnt = itemCnt;
	memcpy(g_zrfjTab,tab,itemCnt*sizeof(zhenRFJTab_type));
	OSA_mutexUnlock(&g_zrfjSynMutex);
}

static void fetchZrfjTabTimeout(){
	OSA_DBG_MSGX(" ");
	memset(g_zrfjTab,0xFF, sizeof(g_zrfjTab));
	g_zrfjTabItemCnt = 0;
	OSA_mutexUnlock(&g_zrfjSynMutex);
}

zhenRFJTab_type *getGZRFJTab(int *itemCnt){
#if 1	
	sndQueryZrfjTab();
	freeZrfjTimer();
	g_zrfjTimerID = osa_add_timer(ZRFJ_FETCH_TIMEOUT,fetchZrfjTabTimeout,NULL,TIMER_ONCE);
	OSA_mutexLock(&g_zrfjSynMutex);	
#endif
	*itemCnt = g_zrfjTabItemCnt;
	return g_zrfjTab;
}



