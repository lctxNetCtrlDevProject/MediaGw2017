#ifndef __PARA_INJECT_AGENT_H__
#define __PARA_INJECT_AGENT_H__

#pragma pack(1)
typedef struct {
	unsigned char reserve1;
	unsigned char dev_type;	/*device type*/
	unsigned short manu_id;	/*manufactory, eg 0x0834 */
	unsigned short reserve2;
	unsigned short dev_seq;	/*device sequence num*/
}DEV_ID_TYPE;

typedef struct {
	unsigned int 		file_len;
	unsigned short 	gen_year;
	unsigned char		gen_mon;
	unsigned char 	gen_day;
	unsigned char 	gen_hour;
	unsigned char 	gen_min;
	unsigned char 	gen_sec;
}PARA_INJEC_FILE_HEAD;
#pragma pack(0)


typedef int (*paraProcCbFunType)(unsigned char *buf, int len);
typedef int (*clrParaCbFunType)();
extern int  initParaInjectAgent(DEV_ID_TYPE *devId, paraProcCbFunType paraProcFun, clrParaCbFunType clrParaFun);


#endif


