#include <stdio.h>
#include <string.h>
#include "paraInjectAgent.h"
#include "MGWparaInject.h"
#include "debug.h"

#ifndef	NULL
#define	NULL	(void *)0
#endif


/****************Macro & Define***************/
typedef int (*tag_cb)(unsigned char *buf, int len);
typedef struct _tag {
	char name[8];
	tag_cb callback;
//	char *p;
} tag;


extern int zwjrpa_cb(unsigned char *buf, int len);
extern int qlintf_cb(unsigned char *buf, int len);
extern int usrnum_cb(unsigned char *buf, int len);
extern int pfzsys_cb(unsigned char *buf, int len);
extern int xwvsip_cb(unsigned char *buf, int len);
extern int gidpar_cb(unsigned char *buf, int len);
extern int madlst_cb(unsigned char *buf, int len);
extern int tiiptb_cb(unsigned char *buf, int len);
extern int pf2ipl_cb(unsigned char *buf, int len);
extern int subnrt_cb(unsigned char *buf, int len);
extern int fwrttb_cb(unsigned char *buf, int len);
extern int ridlst_cb(unsigned char *buf, int len);
extern int pfrtbl_cb(unsigned char *buf, int len);
extern int pfiprt_cb(unsigned char *buf, int len);
extern int pfmrtb_cb(unsigned char *buf, int len);
extern int pfopti_cb(unsigned char *buf, int len);
extern int voisys_cb(unsigned char *buf, int len);
extern int tirnum_cb(unsigned char *buf, int len);
extern int rapnum_cb(unsigned char *buf, int len);
extern int reglst_cb(unsigned char *buf, int len);
extern int spelst_cb(unsigned char *buf, int len);
extern int conlst_cb(unsigned char *buf, int len);
extern int dsllst_cb(unsigned char *buf, int len);
extern int kplist_cb(unsigned char *buf, int len);
extern int sslist_cb(unsigned char *buf, int len);
extern int pfradr_cb(unsigned char *buf, int len);
extern int vhftdm_cb(unsigned char *buf, int len);
extern int uhftdm_cb(unsigned char *buf, int len);
extern int pfvsph_cb(unsigned char *buf, int len);
extern int pfvsip_cb(unsigned char *buf, int len);
extern int pfhfpr_cb(unsigned char *buf, int len);
extern int xyrlst_cb(unsigned char *buf, int len);
extern int meetpa_cb(unsigned char *buf, int len);
extern int linepa_cb(unsigned char *buf, int len);
extern int tdhjys_cb(unsigned char *buf, int len);
extern int raintf_cb(unsigned char *buf, int len);
extern int ipintf_cb(unsigned char *buf, int len);
extern int frppar_cb(unsigned char *buf, int len);
extern int tirppa_cb(unsigned char *buf, int len);
extern int lycfbp_cb(unsigned char *buf, int len);
extern int lyjhpa_cb(unsigned char *buf, int len);
extern int jtlypa_cb(unsigned char *buf, int len);
extern int wkmode_cb(unsigned char *buf, int len);
extern int mgwpar_cb(unsigned char *buf, int len);
extern int mgwcof_cb(unsigned char *buf, int len);
extern int compar_cb(unsigned char *buf, int len);
extern int fibrpa_cb(unsigned char *buf, int len);
extern int ippara_cb(unsigned char *buf, int len);
extern int initQueryEvent(int size);
extern void dispPkt(const char *dispName, unsigned char *pkt, int len);
extern int Board_Mng_ZX_DelAll();
extern int Board_Mng_StRt_DelAll();
extern int Board_Mng_Meet_DelAll();
extern int Board_Mng_Lyjh_DelAll();
extern int Board_Mng_IPIntf_Addr_DelAll();
extern int Board_Mng_DelAll_User_Num();




/***********Global Vari****************/
/*all para tag in paraInject file*/
tag tagList[] = {
#if 0
	{"pfzsys", pfzsys_cb},
	{"xwvsip", xwvsip_cb},
	{"gidpar", gidpar_cb},
	{"madlst", madlst_cb},
	{"tiiptb", tiiptb_cb},
	{"pf2ipl", pf2ipl_cb},
	{"subnrt", subnrt_cb},
	{"fwrttb", fwrttb_cb},
	{"ridlst", ridlst_cb},
	{"pfrtbl", pfrtbl_cb},
	{"pfiprt", pfiprt_cb},
	{"pfmrtb", pfmrtb_cb},
	{"pfopti", pfopti_cb},
	{"voisys", voisys_cb},
	{"tirnum", tirnum_cb},
	{"rapnum", rapnum_cb},
	{"reglst", reglst_cb},
	{"spelst", spelst_cb},
	{"conlst", conlst_cb},
	{"dsllst", dsllst_cb},
	{"kplist", kplist_cb},
	{"sslist", sslist_cb},
	{"pfradr", pfradr_cb},
	{"vhftdm", vhftdm_cb},
	{"uhftdm", uhftdm_cb},
	{"pfvsph", pfvsph_cb},
	{"pfvsip", pfvsip_cb},
	{"pfhfpr", pfhfpr_cb},
	{"xyrlst", xyrlst_cb},

#endif	
	{"zwjrpa", zwjrpa_cb},
	{"usrnum", usrnum_cb},
	{"meetpa", meetpa_cb},
	{"linepa", linepa_cb},
	{"qlintf", qlintf_cb},
	{"raintf", raintf_cb},
	{"ipintf", ipintf_cb},
	{"frppar", frppar_cb},
	{"tirppa", tirppa_cb},
	{"lycfbp", lycfbp_cb},
	{"lyjhpa", lyjhpa_cb},
	{"jtlypa", jtlypa_cb},
	{"ippara", ippara_cb},
	{"wkmode", wkmode_cb},
	{"mgwpar", mgwpar_cb},
	{"mgwcof",mgwcof_cb},
	{"compar", compar_cb},
	{"fibrpa", fibrpa_cb},
	{"", NULL},
};

/****----------------Stake funs------------------------------***/

/*proc para Inject bytes order without inject file header*/
int  paraFileProcFun(unsigned char *buf, int len){

	int i, j;
	int ret = 0;

	//how to deal with buffer size > len
	DBG(" process para inject files");
	printf("-------------start------------");

	dispPkt(__func__, buf, len);

	/*we have to guarantee workMode Configure first.
	**Only in these case, some workmode specific paras can be configured
	*/

	/*then Config*/
	for (i = 0; strlen(tagList[i].name); i++)
	{
		for (j = 0; j < len; j++) {
			ret = memcmp(&buf[j], &tagList[i], 6);
			if (ret == 0) { //find tag
				ret = tagList[i].callback(&buf[j]+6, 0);
				if (ret) {
					printf("tag:%s callback error\n", tagList[i].name);
					return -1;
				}

				break;
			} 						
		}
		
		if (j == len) {
			printf("can't find tagname %s\n", tagList[i].name);
			return -1;
		}
	}

#if 0	
		 int n = -1;
		 unsigned char *new_buf = NULL;
		 unsigned char *old_buf = buf;
		 unsigned char *end_buf = buf + len;

		 new_buf = strstr(old_buf, tagList[i].name);
		 if (new_buf)
		 {
			if (n >= 0) {
				if (tagList[n].callback)
	 				ret = tagList[n].callback((old_buf + strlen(tagList[n].name)) , (new_buf - old_buf - strlen(tagList[n].name)));
					if (ret)
						return ret;
			}
			n = i;
			old_buf = new_buf;
		 }
		 else
		 {
			printf("can't find tagname %s\n", tagList[i].name);
			return -1;
		 }
	}

	if (n >= 0 && tagList[n].callback)
		ret = tagList[n].callback(old_buf  + strlen(tagList[n].name), end_buf - old_buf - strlen(tagList[n].name));
#endif

	return ret;


}

/*proc clear device parameter*/
int paraClrFun(){
	DBGX("clr dev paras");
	Board_Mng_DelAll_User_Num();
	Board_Mng_Meet_DelAll();
	Board_Mng_ZX_DelAll();
	Board_Mng_IPIntf_Addr_DelAll();
	Board_Mng_StRt_DelAll();
	Board_Mng_Lyjh_DelAll();
	return 0;
}

void ParaInject(){	
	DEV_ID_TYPE devID;
#if 1
	devID.dev_type = 0xF5;
	devID.manu_id = 0x0834;
	devID.dev_seq = 0x0001;
#else
	devID.dev_type = 0x10;
	devID.manu_id = 0x0716;
	devID.dev_seq = 0x0002;

#endif

	initQueryEvent(1);

	if(initParaInjectAgent(&devID,paraFileProcFun,paraClrFun) < 0 ){
		exit(0);
	}
	//while(1)
	//	sleep(10);

}

