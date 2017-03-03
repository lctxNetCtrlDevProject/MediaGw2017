/**
*@FileDesc: Implement a Para Ingect Agent for supporting zdk_30s Parameters Inject flow
*@Author: Andy-wei.hou
*@Note: the bytes order of para inject PDU is network bytes order, be carefully
*@Log: 2017.01.09 Created by Andy-wei.hou
**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#include "paraInjectAgent.h"
#include "debug.h"

/***************Macros & Definations******************/

#define 	PARA_INJECT_DEV_PORT		6013
#define 	PARA_INJECT_MNG_PORT		6012
#define 	PARA_INJECT_TAG_ADDR		"255.255.255.255"

#define 	PARA_INJECT_FILE_MAX_SIZE	1024

#define 	PARA_INJECT_CONST_HEAD	0xFC

typedef struct {
	paraProcCbFunType paraProcFun;
	clrParaCbFunType    clrParaProcFun;
}PARA_CB_FUNS;


#pragma pack(1)


/*********PDU defination********************/
typedef struct {
	unsigned char sc;			/*const start character, 0xFC*/
	unsigned char ope;			/*operation code*/
	unsigned short len;			/*len of whole pkt, including header*/
	unsigned short grpId;
	unsigned short checkSum;
	//unsigned int checkSum;		/**/
}PARA_INJECT_PKT_HEAD;

/*query online device pkt*/
typedef struct {
	PARA_INJECT_PKT_HEAD head;
	unsigned char			      data[8];
}PARA_INJECT_DEV_QUERY_PKT;

/*respone online device pkt*/
typedef struct {
	PARA_INJECT_PKT_HEAD head;
	DEV_ID_TYPE		      dev_id;
}PARA_INJECT_DEV_QUERY_ACK_PKT;

/*set len of the whole para file*/
typedef struct {
	PARA_INJECT_PKT_HEAD 	head;
	DEV_ID_TYPE		      	dev_id;
	unsigned 	int			      	file_len;
}PARA_INJECT_SET_PARA_FILE_LEN_PKT;

/*snd the spilite of para file*/
typedef struct {
	PARA_INJECT_PKT_HEAD 	head;
	//unsigned short				split_cnt;
	DEV_ID_TYPE		      	dev_id;
	unsigned 	char			      	para[PARA_INJECT_FILE_MAX_SIZE];
}PARA_INJECT_SND_PARA_FILE_PKT;

/*clr all para of device*/
typedef struct {
	PARA_INJECT_PKT_HEAD head;
	DEV_ID_TYPE		      dev_id;
}PARA_INJECT_CLR_ALL_PARA_PKT;

/*operation success pkt*/
typedef struct {
	PARA_INJECT_PKT_HEAD head;
	DEV_ID_TYPE		      dev_id;
}PARA_INJECT_OPE_ACK_SUC_PKT;

/*operation fail pkt*/
typedef struct {
	PARA_INJECT_PKT_HEAD head;
	DEV_ID_TYPE		      dev_id;
	unsigned char			      err_code;
	char					      err_str[65];
}PARA_INJECT_OPE_ACK_FAIL_PKT;

#pragma pack(0)


typedef enum {
	ERR_CHECKSUM = 1,
	ERR_INVA_SPILITE,
	ERR_NO_FREE_SPACE,
	ERR_INVA_FILE_LEN,
	ERR_OTHERS,
}PARA_INJEC_ACK_CODE;

typedef enum{
	PARA_INJEC_DEV_QUERY= 0x01,			/*broadcast to query online devices*/
	PARA_INJEC_DEV_QUERY_ACK = 0x02,		/*response local device id to net-mng*/
	
	PARA_INJEC_OPE_ACK_SUC = 0x04,		/*operation success for set or clr opreations*/
	PARA_INJEC_OPE_ACK_FAIL = 0x05,		/*operation fail*/
	PARA_INJEC_SET_PARA_FILE_LEN  = 0x08,	/*set the length of whole parameter inject file*/
	PARA_INJEC_SND_PARA_SPLITS	= 0x09,	/*snd a parameter split to local device, if the size of paraFile is less than 1024, one splite is the whole file*/
	
	PARA_INJEC_CLR_ALL =  0x0a,			/*clear all parameters of device*/
	
}PARA_OPE_CODE;


/***************Global Variablies*********************/
DEV_ID_TYPE g_devID;		/*local device id*/
unsigned char * g_pParaBuf;	/*pointer to the  rcving para file*/
int 			g_paraLen;		/*current para bytes need to be rcved*/
PARA_CB_FUNS  g_pParaInjecCbFun;
int 			g_listenFd;






/*****************Funcs****************************/

void dispPkt(const char *dispName, unsigned char *pkt, int len){
	int i =0;
	if(dispName){
		printf("\r\n%s [",dispName);
	}
	else 
		printf("\r\n [");
	for(i =0 ;i < len; i++)
		printf("%02X ",*(pkt+i));
	printf("]\r\n");
}

unsigned short inetCksum(const unsigned short *buffer, int size){
	unsigned int cksum = 0;
	unsigned short answer = 0;
	while(size > 1){
		cksum += *(buffer++);
		size  -= sizeof(unsigned short);
	}
	if(size){
		*(unsigned char *)&answer = *(unsigned char *)buffer;
		cksum += answer;
	}
	cksum = (cksum >> 16)  + (cksum  & 0xFFFF);
	cksum +=(cksum >> 16);

	return (unsigned short)(~cksum);
}

void testCksum(void)
{
	unsigned char buffer[] = {0xFC, 0x09, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x07, 0x16, 0x00, 0x00, 0x00, 0x02, 
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 
		0x01, 0x02, 0x03, 0x04, 0x05, 0x04, 0x03, 0x01, 0x04, 0x00, 0x00, 0x00 };
	int size = sizeof(buffer);
	unsigned short ret;

	ret = inetCksum((unsigned short *)buffer, size);
	printf("%s ret=0x%x\n", __func__, ret);
}

bool isLocalDev(DEV_ID_TYPE *devID){
	if(!devID){
		ERROR("Invalid Para \r\n");
		return false;
	}
	dispPkt(__func__,devID,sizeof(DEV_ID_TYPE));
	dispPkt(__func__,&g_devID,sizeof(DEV_ID_TYPE));
	if(0 == memcmp(devID, &g_devID,sizeof(DEV_ID_TYPE))){
		return true;
	}
	return false;
}
unsigned short getCheckSum(unsigned char *buf, int len){

	return inetCksum((const unsigned short *)buf,len);
}

int sndPkt2Mng(unsigned char *buf, int len){
	struct sockaddr_in addr;
	int addr_len =sizeof(struct sockaddr_in);
	int iRet = -1;

	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PARA_INJECT_MNG_PORT);
	addr.sin_addr.s_addr = inet_addr(PARA_INJECT_TAG_ADDR);
	//addr.sin_addr.s_addr = inet_addr("10.0.0.121");
	dispPkt(__func__,buf,len);
	iRet = sendto(g_listenFd,buf,len,0,&addr,addr_len);
	if(iRet != len){
		ERROR("Sendto Fail, errno=%d",errno);
	}
	return iRet;
}

void replyFail(PARA_INJECT_PKT_HEAD *srcPkt, PARA_INJEC_ACK_CODE code, char *reason){
	unsigned len = 0;
	int reaLen = 0;
	PARA_INJECT_OPE_ACK_FAIL_PKT pkt;

	memset(&pkt,0x00,sizeof(pkt));
	
	pkt.head.sc = PARA_INJECT_CONST_HEAD;
	pkt.head.ope = PARA_INJEC_OPE_ACK_FAIL;
	
	memcpy(&pkt.dev_id,&g_devID,sizeof(DEV_ID_TYPE));
	pkt.err_code = code;
	if(reason){
		sprintf(pkt.err_str, "%s",reason);
		reaLen = strlen(reason);
	}
	len = sizeof(PARA_INJECT_PKT_HEAD) + sizeof(DEV_ID_TYPE) + 1 + reaLen;
	
	pkt.head.len = htons(len);

	pkt.head.grpId = srcPkt->grpId;
	pkt.head.checkSum = htons(getCheckSum(&pkt,len));
	sndPkt2Mng(&pkt, len);
}

void replySuc(PARA_INJECT_PKT_HEAD *srcPkt){
	unsigned len = 0;
	PARA_INJECT_OPE_ACK_SUC_PKT pkt;

	memset(&pkt,0x00,sizeof(pkt));
	
	pkt.head.sc = PARA_INJECT_CONST_HEAD;
	pkt.head.ope = PARA_INJEC_OPE_ACK_SUC;
	
	memcpy(&pkt.dev_id,&g_devID,sizeof(DEV_ID_TYPE));

	len = sizeof(pkt);	
	pkt.head.len = htons(len);	
	pkt.head.grpId = srcPkt->grpId;
	pkt.head.checkSum = htons(getCheckSum(&pkt,len));
	sndPkt2Mng(&pkt, len);
}

void replyDevQuery(){
	unsigned len = 0;
	PARA_INJECT_DEV_QUERY_ACK_PKT pkt;

	memset(&pkt,0x00,sizeof(pkt));
	
	pkt.head.sc = PARA_INJECT_CONST_HEAD;
	pkt.head.ope = PARA_INJEC_DEV_QUERY_ACK;
	
	memcpy(&pkt.dev_id,&g_devID,sizeof(DEV_ID_TYPE));

	len = sizeof(pkt);	
	pkt.head.len = htons(len);	
	pkt.head.checkSum = htons(getCheckSum(&pkt,len));
	
	sndPkt2Mng(&pkt, len);
}

void  handleDevQuery(PARA_INJECT_DEV_QUERY_PKT *pPkt){
	printf("%s \r\n",__func__);
	replyDevQuery();
	
}

void handleClrAllPara(PARA_INJECT_CLR_ALL_PARA_PKT *pPkt){
	printf("%s \r\n",__func__);
	if(isLocalDev(&pPkt->dev_id) == false){
		DBG("Not to local dev, discard it");
		return;
	}
	if(g_pParaInjecCbFun.clrParaProcFun){
		if (0 == g_pParaInjecCbFun.clrParaProcFun())
			replySuc(pPkt);
		else
			replyFail(pPkt, ERR_OTHERS,"can't clear parameter");
	}
}

void resetRcving(){
	if(g_pParaBuf)
		free(g_pParaBuf);
	g_pParaBuf = NULL;
	g_paraLen = 0;
}
void handleSetParaFileLen(PARA_INJECT_SET_PARA_FILE_LEN_PKT *pPkt){
	int pktLen = 0;
	if(isLocalDev(&pPkt->dev_id) == false){
		DBG("Not to local dev, discard it");
		return;
	}
	if(g_pParaBuf){
		DBGX("Rcv set Len Pkt, discard all rcving para, retry rcving");
		resetRcving();		
	}

	pktLen = ntohl(pPkt->file_len);
	DBG("PARA FILE LEN = 0x%x", pktLen);
	g_pParaBuf = (unsigned char *) malloc(pktLen);
	if(!g_pParaBuf){
		ERROR("Malloc Fail, errno=%d",errno);
		replyFail(pPkt, ERR_NO_FREE_SPACE,NULL);
		return;
	}
	g_paraLen = pktLen;
	replySuc(pPkt);
	
}

void  procParaFile(PARA_INJECT_PKT_HEAD *srcPkt){
	if(g_pParaInjecCbFun.paraProcFun){
		if (0 == g_pParaInjecCbFun.paraProcFun(g_pParaBuf,g_paraLen))
			replySuc(srcPkt);
		else {
			replyFail(srcPkt, ERR_OTHERS,"Can't Inject para");
			if(g_pParaInjecCbFun.clrParaProcFun){
				if (0 == g_pParaInjecCbFun.clrParaProcFun()) 
					printf("clear para success\n");
				else
					printf("clear para fail\n");
			}
		}
	}
}

void handleSndParaFile(PARA_INJECT_SND_PARA_FILE_PKT *pPkt){
	int curParaLen = 0;
	int spilitLen = 0;
	static int fileLen= 0;
	if(isLocalDev(&pPkt->dev_id) == false){
		DBG("Not to local dev, discard it");
		return;
	}
	if(!g_pParaBuf){
		DBG("Not set Para File Len, reply fail");
		replyFail(pPkt, ERR_INVA_FILE_LEN,NULL);
		return;
	}

	spilitLen = ntohs(pPkt->head.len) - sizeof(DEV_ID_TYPE)- sizeof(PARA_INJECT_PKT_HEAD);
	DBG("spilitLen=0x%x, head len=0x%x \r\n", spilitLen, pPkt->head.len);
	DBG("fileLen=0x%x, g_paraLen=0x%x \r\n", fileLen, g_paraLen);

	if(spilitLen > 1024){
		DBGX("Invalid splitLen");
		replyFail(pPkt, ERR_INVA_SPILITE,NULL);
		return;
	} else {
		if (fileLen + spilitLen > g_paraLen)
		{
			DBGX("Invalid fileLen");
			replyFail(pPkt, ERR_INVA_FILE_LEN,NULL);
			resetRcving();
			return;
		}	

		//dispPkt(__func__, pPkt->para, spilitLen);
		memcpy((g_pParaBuf+fileLen), pPkt->para,spilitLen);
		fileLen += spilitLen;
	}

	DBG("fileLen=0x%x, g_paraLen=0x%x\r\n", fileLen, g_paraLen);
	if (fileLen == g_paraLen){
		DBG("Complete rcv para file, try analysi it"); 
		replySuc(pPkt);
		procParaFile(pPkt); 
		resetRcving();
		DBG("INJECT END --- \r\n");
		DisplayBoardCLear();
		DisplayBoardShowOK();
		fileLen = 0;
		return;
	}

	if (spilitLen < 1024) {
		DBGX("Invalid fileLen");
		replyFail(pPkt, ERR_INVA_FILE_LEN,NULL);
		resetRcving();
		fileLen = 0;
		return;
	}
	
	replySuc(pPkt);

	return;
}


void paraInjectProc(unsigned char *buf, int len ){
	PARA_INJECT_PKT_HEAD *pPduPkt = NULL;
	unsigned short tmpCkSum = 0;
	unsigned char ope = 0;
	if(!buf || len <= 0){
		ERROR("Invalid Parameter");
		return;
	}
	
	dispPkt(__func__, buf, len);
	printf("%s, len=0x%x\n", __func__, len);
	pPduPkt = (PARA_INJECT_PKT_HEAD *)buf;
	ope = pPduPkt->ope;

	tmpCkSum = inetCksum(buf, len);
	printf("tmpCkSum = 0x%x\n", tmpCkSum);
	if (tmpCkSum != 0) {
		DBG("CheckSum Err, Reply checkSumErr to MNG");
		replyFail(buf, ERR_CHECKSUM, NULL);
		return;
	}
	switch(ope){
		case PARA_INJEC_DEV_QUERY:{
			handleDevQuery((PARA_INJECT_DEV_QUERY_ACK_PKT * )pPduPkt);
			}break;
		case PARA_INJEC_CLR_ALL:{
			handleClrAllPara((PARA_INJECT_CLR_ALL_PARA_PKT * )pPduPkt);
			}break;
		case PARA_INJEC_SET_PARA_FILE_LEN:{
			handleSetParaFileLen((PARA_INJECT_SET_PARA_FILE_LEN_PKT * )pPduPkt);
			}break;
		case PARA_INJEC_SND_PARA_SPLITS:{
			handleSndParaFile((PARA_INJECT_SND_PARA_FILE_PKT *)pPduPkt);
			}break;
		default:{
			DBGX("Unknow ope type =%02X",pPduPkt->ope);
		}
	}
}

int createListenSocket(){
	int fd = 0;
	int flag = 1;
	struct sockaddr_in addr = { 0 };
	
	fd = socket(AF_INET,SOCK_DGRAM, 0);
	if(fd < 0){
		ERROR("create socket fail ,errno=%d",errno);
		return -1;
	}
	setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &flag, sizeof(flag));
	bzero((void *)&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PARA_INJECT_DEV_PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(fd,&addr,sizeof(addr)) < 0){
		ERROR("bind fail, errno =%d \r\n",errno);
		return -1;
	}
	return fd;
	
}


void paraInjectAgnetThr(){
	unsigned char buf[1024+16];//data len: 1024, header len: 16
	int len = 0;
	struct sockaddr_in addr;
	int addr_len =sizeof(struct sockaddr_in);

	DBG("%s",__func__);
	g_listenFd = createListenSocket();
	if(g_listenFd < 0 ){
		ERROR("can't listen socket, exit");
		return;
	}
	while(1){
		memset(buf, 0x00, sizeof(buf));
		len  = recvfrom(g_listenFd, buf,sizeof(buf),0,&addr,&addr_len);
		if(len >0){
			paraInjectProc(buf,len);
		}
	}

	
}


int  initParaInjectAgent(DEV_ID_TYPE *devId, paraProcCbFunType paraProcFun, clrParaCbFunType clrParaFun){
		pthread_t paraInjecThrHandle; 
		g_devID.dev_seq = htons(devId->dev_seq);
		g_devID.manu_id = htons(devId->manu_id);
		g_devID.dev_type = devId->dev_type;

		
		g_pParaInjecCbFun.clrParaProcFun = clrParaFun;
		g_pParaInjecCbFun.paraProcFun = paraProcFun;

		if(0 != pthread_create(&paraInjecThrHandle,NULL,paraInjectAgnetThr,NULL)){
			ERROR("Create thr fail ,errno=%d",errno);
			return -1;
		}
		return 0;
		
}

