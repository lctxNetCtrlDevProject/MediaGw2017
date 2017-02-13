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

/************Global Variablies*******************/
zhenRFJTab_type g_zrfjTab[ZHENR_FENJI_ITEM_MAX];


/*stake func, should fetch zrfj table by PDU*/
void fetchZrfjTab(){
	int i = 0;
	for(i = 0; i < ZHENR_FENJI_ITEM_MAX; i++){
		g_zrfjTab[i].zjID = i;
		g_zrfjTab[i].fenJID = i *random();
		memset(g_zrfjTab[i].bcdFenJNum,0x00,BCD_PHONE_NUM_LEN);
		sprintf(g_zrfjTab[i].bcdFenJNum,"Num_%d",i);
	}
}

zhenRFJTab_type *getGZRFJTab(int *itemCnt){
	fetchZrfjTab();

	*itemCnt = ZHENR_FENJI_ITEM_MAX;
	return g_zrfjTab;
}



