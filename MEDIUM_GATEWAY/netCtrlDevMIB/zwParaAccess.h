
#ifndef __ZW_PARA_ACCESS_H__
#define __ZW_PARA_ACCESS_H__



#pragma pack(1)

#define BCD_USR_NUM_LEN 8
#define BCD_SEC_NUM_LEN 4

#define USR_NUM_LEN 8
#define SEC_NUM_LEN 4

#define BCD_CONF_NUM_LEN 4
#define BCD_CONF_PART_NUM_LEN 8
#define USR_NUM_ITEM_MAX 100

#define CONF_PART_CNT 16
#define CONF_NUM_LEN 4
#define CONF_PART_NUM_LEN 8
#define CONF_ITEM_MAX 10

#define BCD_ARMY_NUM_LEN 3

#define ZHUAN_XIAN_ITEM_MAX 10

typedef struct {
	unsigned char chanId;
	unsigned char usrNum[USR_NUM_LEN];
	unsigned char secNum[SEC_NUM_LEN];
}zwUsrNum_type;

typedef struct {
	unsigned short confNum;
	unsigned char partCnt;
	unsigned char partNum[CONF_PART_CNT][CONF_PART_NUM_LEN];
}zwConf_type;

typedef struct {
	unsigned char phoneId;
	unsigned char port;
	unsigned short army;
	unsigned int calleeNum;
	unsigned char zxStatus;
}zwZx_type;

extern void initZwMode();
extern void setZwMode(int mode);
extern int getZwMode();

extern void initArmyId();
extern void setArmyId(int ArmyId);
extern int getArmyId();



#pragma pack(0)

#endif

