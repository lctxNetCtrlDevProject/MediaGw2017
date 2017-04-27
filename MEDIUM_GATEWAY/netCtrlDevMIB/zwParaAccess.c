/**
*@Desc: using Mng protocol to fetch zhenRuFenJiTable from netCtrlDev
*@Author: Andy-wei.hou
*@Log:	  2017.02.13, created by Andy-wei.hou
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "zwParaAccess.h"
#include "paraMng.h"
#include "osa.h"

/**************Macro**************************/
#define FETCH_TIMEOUT		10		/* fetching time out 500s*/

/************Global Variablies*******************/

/*--WorkMode---*/
int zwMode = 0x10;	/*0x10, TongKongQi; 0x11,GanXianJi; 0x12,DaRuKou; 0x13, XiaoRuKou; 0x14, ZhuangJiaXing*/
OSA_MutexHndl    g_zwModeSynMutex;		/*mutex using to syn the query & response*/
int			      g_zwModeTimerID;



/*********************Funcs*****************/
static void freeTimer(int timerID){
	if(timerID != -1){
		osa_del_timer(timerID);
		timerID = -1;
	}
}

//-----------zwMode-------------------
void initZwMode(){
	zwMode = 2;
	OSA_mutexCreate(&g_zwModeSynMutex);
	OSA_mutexLock(&g_zwModeSynMutex);		/*decrease mutex to 0*/
}

void setZwMode(int mode){
	zwMode = mode;
	OSA_mutexUnlock(&g_zwModeSynMutex);
}

static void fetchZwModeTimeout(){
	OSA_DBG_MSGX(" ");
	zwMode = -1;
	OSA_mutexUnlock(&g_zwModeSynMutex);
}

int getZwMode(){
	sndQueryZwMode();
	freeTimer(g_zwModeTimerID);
	g_zwModeTimerID = osa_add_timer(FETCH_TIMEOUT,fetchZwModeTimeout,NULL,TIMER_ONCE);
	OSA_mutexLock(&g_zwModeSynMutex);	
	return zwMode;
}

//-----------zwArmyId----------------------

int zwArmyId = 0;	
OSA_MutexHndl    g_AmryIdSynMutex;		/*mutex using to syn the query & response*/
int			     g_AmryIdTimerID;

void initArmyId(){
	zwArmyId = 2;
	OSA_mutexCreate(&g_AmryIdSynMutex);
	OSA_mutexLock(&g_AmryIdSynMutex);		/*decrease mutex to 0*/
	freeTimer(g_AmryIdTimerID);
}

void setArmyId(int ArmyId){
	zwArmyId = ArmyId;
	OSA_mutexUnlock(&g_AmryIdSynMutex);
	freeTimer(g_AmryIdTimerID);
}

static void fetchArmyIdTimeout(){
	OSA_DBG_MSGX(" ");
	zwArmyId = -1;
	OSA_mutexUnlock(&g_AmryIdSynMutex);
}

int getArmyId(){
	sndQueryArmyId();
	g_AmryIdTimerID = osa_add_timer(FETCH_TIMEOUT,fetchArmyIdTimeout,NULL,TIMER_ONCE);
	OSA_mutexLock(&g_AmryIdSynMutex);	
	return zwArmyId;
}

//-----------zwUsrNumTab----------------------
#define USR_NUM_ITEM_MAX 100

zwUsrNum_type g_zwUsrNumTab[USR_NUM_ITEM_MAX];
int g_zwUsrNumTabItemCnt = 0;
OSA_MutexHndl g_zwUsrNumSynMutex;		/*mutex using to syn the query & response*/
int	g_zwUsrNumTimerID;

void initUsrNumTab(){
	memset(g_zwUsrNumTab,0, sizeof(g_zwUsrNumTab));
	g_zwUsrNumTabItemCnt = 0;
	OSA_mutexCreate(&g_zwUsrNumSynMutex);
	OSA_mutexLock(&g_zwUsrNumSynMutex);		/*decrease mutex to 0*/
}

void setUsrNumTab(zwUsrNum_type tab[], int itemCnt){
	int i;
	
	OSA_DBG_MSGX(" ");
	freeTimer(g_zwUsrNumTimerID);
	
	if(!tab || itemCnt <=0 ) {
		g_zwUsrNumTabItemCnt = 0;
		OSA_mutexUnlock(&g_zwUsrNumSynMutex);
		return;
	}
	
	if (itemCnt > USR_NUM_ITEM_MAX)
		itemCnt = USR_NUM_ITEM_MAX;

	g_zwUsrNumTabItemCnt = itemCnt; 
	memcpy(g_zwUsrNumTab,tab,itemCnt*sizeof(zwUsrNum_type));

	OSA_mutexUnlock(&g_zwUsrNumSynMutex);
}

static void fetchUsrNumTabTimeout(){
	OSA_DBG_MSGX(" ");
	memset(g_zwUsrNumTab,0 , sizeof(g_zwUsrNumTab));
	g_zwUsrNumTabItemCnt = 0;
	OSA_mutexUnlock(&g_zwUsrNumSynMutex);
}

zwUsrNum_type *getUsrNumTab(int *itemCnt){
	sndQueryZwUsrNumTabAll();
	g_zwUsrNumTimerID = osa_add_timer(FETCH_TIMEOUT,fetchUsrNumTabTimeout,NULL,TIMER_ONCE);
	OSA_mutexLock(&g_zwUsrNumSynMutex);	
	*itemCnt = g_zwUsrNumTabItemCnt;
	OSA_DBG_MSGX(" itemCnt=%d ", *itemCnt);
	return g_zwUsrNumTab;
}
