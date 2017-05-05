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
#define FETCH_TIMEOUT		20		/* fetching time out 2000s*/

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
	zwMode = 0;
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
//-----------zwDevSeq---------------------
int zwDevSeq = 0;	
OSA_MutexHndl    g_DevSeqSynMutex;		/*mutex using to syn the query & response*/
int			     g_DevSeqTimerID;

void initDevSeq(){
	zwDevSeq = 0;
	OSA_mutexCreate(&g_DevSeqSynMutex);
	OSA_mutexLock(&g_DevSeqSynMutex);		/*decrease mutex to 0*/
	freeTimer(g_DevSeqTimerID);
}

void setDevSeq(int DevSeq){
	zwDevSeq = DevSeq;
	OSA_DBG_MSGX("DevSeq =%x",DevSeq);
	OSA_mutexUnlock(&g_DevSeqSynMutex);
	freeTimer(g_DevSeqTimerID);
}

static void fetchDevSeqTimeout(){
	OSA_DBG_MSGX(" ");
	zwDevSeq = -1;
	OSA_mutexUnlock(&g_DevSeqSynMutex);
}

int getDevSeq(){
	sndQueryDevSeq();
	g_DevSeqTimerID = osa_add_timer(FETCH_TIMEOUT,fetchDevSeqTimeout,NULL,TIMER_ONCE);
	OSA_mutexLock(&g_DevSeqSynMutex);	
	return zwDevSeq;
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

//-------------zwPhoneLen------------------------
int zwPhoneLen = 0;
OSA_MutexHndl    g_PhoneLenSynMutex;		/*mutex using to syn the query & response*/
int			     g_PhoneLenTimerID;

void initPhoneLen() {
	zwPhoneLen = 0;
	OSA_mutexCreate(&g_PhoneLenSynMutex);
	OSA_mutexLock(&g_PhoneLenSynMutex);		/*decrease mutex to 0*/
	freeTimer(g_PhoneLenTimerID);
}

void setPhoneLen(int PhoneLen){
	zwPhoneLen = PhoneLen;
	OSA_mutexUnlock(&g_PhoneLenSynMutex);
	freeTimer(g_PhoneLenTimerID);
}

static void fetchPhoneLenTimeout(){
	OSA_DBG_MSGX(" ");
	zwPhoneLen = -1;
	OSA_mutexUnlock(&g_PhoneLenSynMutex);
}

int getPhoneLen(){
	sndQueryPhoneLen();
	g_PhoneLenTimerID = osa_add_timer(FETCH_TIMEOUT,fetchPhoneLenTimeout,NULL,TIMER_ONCE);
	OSA_mutexLock(&g_PhoneLenSynMutex);	
	return zwPhoneLen;
}

//--------------zwAudioCodec--------------------
int zwAudioCodec = 0;
OSA_MutexHndl	 g_AudioCodecSynMutex;		/*mutex using to syn the query & response*/
int 			 g_AudioCodecTimerID;

void initAudioCodec() {
	zwAudioCodec = 0;
	OSA_mutexCreate(&g_AudioCodecSynMutex);
	OSA_mutexLock(&g_AudioCodecSynMutex); 	/*decrease mutex to 0*/
	freeTimer(g_AudioCodecTimerID);
}

void setAudioCodec(int AudioCodec){
	zwAudioCodec = AudioCodec;
	OSA_mutexUnlock(&g_AudioCodecSynMutex);
	freeTimer(g_AudioCodecTimerID);
}

static void fetchAudioCodecTimeout(){
	OSA_DBG_MSGX(" ");
	zwAudioCodec = -1;
	OSA_mutexUnlock(&g_AudioCodecSynMutex);
}

int getAudioCodec(){
	sndQueryAudioCodec();
	g_AudioCodecTimerID = osa_add_timer(FETCH_TIMEOUT,fetchAudioCodecTimeout,NULL,TIMER_ONCE);
	OSA_mutexLock(&g_AudioCodecSynMutex); 
	return zwAudioCodec;
}

//------------zwAsNum------------------------
int zwAsNum = 0;
OSA_MutexHndl	 g_AsNumSynMutex;		/*mutex using to syn the query & response*/
int 			 g_AsNumTimerID;

void initAsNum() {
	zwAsNum = 2;
	OSA_mutexCreate(&g_AsNumSynMutex);
	OSA_mutexLock(&g_AsNumSynMutex); 	/*decrease mutex to 0*/
	freeTimer(g_AsNumTimerID);
}

void setAsNum(int AsNum){
	zwAsNum = AsNum;
	OSA_mutexUnlock(&g_AsNumSynMutex);
	freeTimer(g_AsNumTimerID);
}

static void fetchAsNumTimeout(){
	OSA_DBG_MSGX(" ");
	zwAsNum = -1;
	OSA_mutexUnlock(&g_AsNumSynMutex);
}

int getAsNum(){
	sndQueryAsNum();
	g_AsNumTimerID = osa_add_timer(FETCH_TIMEOUT,fetchAsNumTimeout,NULL,TIMER_ONCE);
	OSA_mutexLock(&g_AsNumSynMutex); 
	return zwAsNum;
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
	OSA_DBG_MSGX(" itemCnt=%d", itemCnt);
	
#if 1
	if(!tab || itemCnt < 0 )
		return;
	freeTimer(g_zwUsrNumTimerID);
#else
	freeTimer(g_zwUsrNumTimerID);

	if(!tab || itemCnt <=0 ) {
		g_zwUsrNumTabItemCnt = 0;
		OSA_mutexUnlock(&g_zwUsrNumSynMutex);
		return;
	}
#endif

	if (itemCnt > USR_NUM_ITEM_MAX)
		itemCnt = USR_NUM_ITEM_MAX;

	g_zwUsrNumTabItemCnt = itemCnt; 

#if 1
	for (i = 0; i < itemCnt; i++) {
		g_zwUsrNumTab[i].chanId = tab[i].chanId;
		strncpy(g_zwUsrNumTab[i].usrNum, tab[i].usrNum, USR_NUM_LEN);
		strncpy(g_zwUsrNumTab[i].secNum, tab[i].secNum, SEC_NUM_LEN);		
	}
#else
	memcpy(g_zwUsrNumTab,tab,itemCnt*sizeof(zwUsrNum_type));
#endif
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
