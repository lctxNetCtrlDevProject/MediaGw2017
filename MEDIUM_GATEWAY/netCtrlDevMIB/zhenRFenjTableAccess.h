
#ifndef __ZHEN_R_FEN_J_TABLE_ACCESS_H__
#define __ZHEN_R_FEN_J_TABLE_ACCESS_H__


#define BCD_PHONE_NUM_LEN		10
#define ZHENR_FENJI_ITEM_MAX	12


#pragma pack(1)
typedef struct {
	unsigned char zjID;
	unsigned char  fenJID;
	unsigned char  bcdFenJNum[BCD_PHONE_NUM_LEN]; 
}zhenRFJTab_type;


extern void initZrfjTab();
extern zhenRFJTab_type *getGZRFJTab(int *itemCnt);
extern void setZrfjTab(zhenRFJTab_type tab[], int itemCnt);


#pragma pack(0)

#endif

