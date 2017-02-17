/**
*@Desc: using Mng protocol to fetch zhenRuFenJiTable from netCtrlDev
*@Author: Andy-wei.hou
*@Log:	  2017.02.13, created by Andy-wei.hou
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mgwParaAccess.h"
#include "paraMng.h"
#include "osa.h"

/**************Macro**************************/
#define FETCH_TIMEOUT		5		/* fetching time out 500s*/

/************Global Variablies*******************/

/*--ZhenRuFenJiTable---*/
zhenRFJTab_type g_zrfjTab[ZHENR_FENJI_ITEM_MAX];
int 			      g_zrfjTabItemCnt = 0;
OSA_MutexHndl    g_zrfjSynMutex;		/*mutex using to syn the query & response*/
int			      g_zrfjTimerID;

/*--ConferenceTable---*/
confTab_type 	g_confTab[CONF_ITEM_MAX];
int 			g_confTabItemCnt = 0;
OSA_MutexHndl    g_confTabSynMutex;		/*mutex using to syn the query & response*/
int			      g_confTabTimerID;

/*--WorkMode---*/
int g_workMode = 2;	/*0, paofang; 2,zhuangjai;3,xiweikuozhan*/
OSA_MutexHndl    g_workModSynMutex;		/*mutex using to syn the query & response*/
int			      g_workModTimerID;

/*--GportPara---*/
int g_GportMode =0;	/*0,kzxw_2M; 1,kzxw_1M, 2,kzxw_162K,4,kss*/
OSA_MutexHndl    g_gPortModSynMutex;		/*mutex using to syn the query & response*/
int			      g_gPortModTimerID;


/*********************Funcs*****************/
static void freeTimer(int timerID){
	if(timerID != -1){
		osa_del_timer(timerID);
		timerID = -1;
	}
}

void initZrfjTab(){
	memset(g_zrfjTab,0xFF, sizeof(g_zrfjTab));
	g_zrfjTabItemCnt = 0;
	OSA_mutexCreate(&g_zrfjSynMutex);
	OSA_mutexLock(&g_zrfjSynMutex);		/*decrease mutex to 0*/
}


void setZrfjTab(zhenRFJTab_type tab[], int itemCnt){
	if(!tab || itemCnt <=0 )
		return;
	freeTimer(g_zrfjTimerID);
	
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
	sndQueryZrfjTab();
	freeTimer(g_zrfjTimerID);
	g_zrfjTimerID = osa_add_timer(FETCH_TIMEOUT,fetchZrfjTabTimeout,NULL,TIMER_ONCE);
	OSA_mutexLock(&g_zrfjSynMutex);	
	*itemCnt = g_zrfjTabItemCnt;
	return g_zrfjTab;
}



void initConfTab(){
	memset(g_confTab,0xFF, sizeof(g_confTab));
	g_confTabItemCnt = 0;
	OSA_mutexCreate(&g_confTabSynMutex);
	OSA_mutexLock(&g_confTabSynMutex);		/*decrease mutex to 0*/
}

void setConfTabItemCnt(int cnt){

	freeTimer(g_confTabTimerID);
	g_confTabItemCnt = cnt;
	OSA_mutexUnlock(&g_confTabSynMutex);
}

void setConfTabItem(confTab_type *item, int i){
	if(!item )
		return;
	//dispBuf(item,CONF_NAME_LEN + 1 + BCD_PHONE_NUM_LEN*(item->partCnt),__func__);
	memcpy(&g_confTab[i],item,CONF_NAME_LEN + 1 + BCD_PHONE_NUM_LEN*(item->partCnt));
}

static void fetchConfTabTimeout(){
	OSA_DBG_MSGX(" ");
	memset(g_confTab,0xFF, sizeof(g_confTab));
	g_confTabItemCnt = 0;
	OSA_mutexUnlock(&g_confTabSynMutex);
}

confTab_type *getConfTab(int *itemCnt){
	sndQueryConfTab();
	freeTimer(g_confTabTimerID);
	g_confTabTimerID = osa_add_timer(FETCH_TIMEOUT,fetchConfTabTimeout,NULL,TIMER_ONCE);
	OSA_mutexLock(&g_confTabSynMutex);	
	*itemCnt = g_confTabItemCnt;
	return g_confTab;
}

void initWorkMode(){
	g_workMode = 2;
	OSA_mutexCreate(&g_workModSynMutex);
	OSA_mutexLock(&g_workModSynMutex);		/*decrease mutex to 0*/
}

void setWorkMode(int mode){
	g_workMode = mode;
	OSA_mutexUnlock(&g_workModSynMutex);
}

static void fetchWorkModTimeout(){
	OSA_DBG_MSGX(" ");
	g_workMode = -1;
	OSA_mutexUnlock(&g_workModSynMutex);
}

int getWorkMode(){
	sndQueryworkMode();
	freeTimer(g_workModTimerID);
	g_workModTimerID = osa_add_timer(FETCH_TIMEOUT,fetchWorkModTimeout,NULL,TIMER_ONCE);
	OSA_mutexLock(&g_workModSynMutex);	
	return g_workMode;
}



void initGPortMode(){
	g_GportMode= 0;
	OSA_mutexCreate(&g_gPortModSynMutex);
	OSA_mutexLock(&g_gPortModSynMutex);		/*decrease mutex to 0*/
}


void setGPortMode(int mode){
	g_GportMode = mode;
	OSA_mutexUnlock(&g_gPortModSynMutex);
}

static void fetchGPortModTimeout(){
	OSA_DBG_MSGX(" ");
	g_GportMode = -1;
	OSA_mutexUnlock(&g_gPortModSynMutex);
}

int getGPortMode(){
	sndQueryGPortMode();
	freeTimer(g_gPortModTimerID);
	g_gPortModTimerID = osa_add_timer(FETCH_TIMEOUT,fetchGPortModTimeout,NULL,TIMER_ONCE);
	OSA_mutexLock(&g_gPortModSynMutex);	
	return g_GportMode;
}

