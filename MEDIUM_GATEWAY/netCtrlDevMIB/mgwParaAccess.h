
#ifndef __MGW_PARA_ACCESS_H__
#define __MGW_PARA_ACCESS_H__


#define BCD_PHONE_NUM_LEN		10
#define ZHENR_FENJI_ITEM_MAX	12

#define CONF_NAME_LEN			2
#define CONF_ITEM_MAX			12

#pragma pack(1)
typedef struct {
	unsigned char zjID;
	unsigned char  fenJID;
	unsigned char  bcdFenJNum[BCD_PHONE_NUM_LEN]; 
}zhenRFJTab_type;

typedef struct {
	unsigned char bcdConfNum[CONF_NAME_LEN];
	char			 partCnt;
	unsigned char  bcdPartNum0[BCD_PHONE_NUM_LEN]; 
	unsigned char  bcdPartNum1[BCD_PHONE_NUM_LEN]; 
	unsigned char  bcdPartNum2[BCD_PHONE_NUM_LEN]; 
	unsigned char  bcdPartNum3[BCD_PHONE_NUM_LEN]; 
	unsigned char  bcdPartNum4[BCD_PHONE_NUM_LEN]; 
	unsigned char  bcdPartNum5[BCD_PHONE_NUM_LEN]; 
	
}confTab_type;



extern void initZrfjTab();
extern zhenRFJTab_type *getGZRFJTab(int *itemCnt);
extern void setZrfjTab(zhenRFJTab_type tab[], int itemCnt);

extern void initConfTab();
extern void setConfTabItem(confTab_type *item, int i);
extern void setConfTabItemCnt(int cnt);
extern confTab_type *getConfTab(int *itemCnt);


#pragma pack(0)

#endif

