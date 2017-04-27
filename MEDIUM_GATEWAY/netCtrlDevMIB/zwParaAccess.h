
#ifndef __ZW_PARA_ACCESS_H__
#define __ZW_PARA_ACCESS_H__



#pragma pack(1)

#define BCD_USR_NUM_LEN 8
#define BCD_SEC_NUM_LEN 4

#define USR_NUM_LEN 8
#define SEC_NUM_LEN 4

typedef struct {
	unsigned char chanId;
	unsigned char usrNum[USR_NUM_LEN];
	unsigned char secNum[SEC_NUM_LEN];
}zwUsrNum_type;



extern void initZwMode();
extern void setZwMode(int mode);
extern int getZwMode();

extern void initArmyId();
extern void setArmyId(int ArmyId);
extern int getArmyId();



#pragma pack(0)

#endif

