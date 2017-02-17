/**
*@Desc: Get para from 834Board, 716Board by PDU of <xieXingWangKong SheBeiCanShuGuanLiJieKouXieYi>
*@Author: Andy-wei.hou
*@Log:	Created by Andy-wei.hou 2017.02.14
*/
#include <errno.h>
#include "osa.h"
#include "mgwParaAccess.h"
#include "interior_protocol.h"
#include "public.h"

#define	SNMP_AGENT_834_PORT	50002
#define 	BOARD_834_PORT		30006


/***********Macro & Definations*********/
enum __834_PARA_ID__
{
	MSG_834_HANDLE_MSG = 0x01,
	MSG_834_HANDLE_MSG_ACK = 0x02,	
	MSG_834_ZHENRUFENJI_ADD = 0x03,
	MSG_834_ZHENRUFENJI_ADD_ACK = 0x04,
	MSG_834_ZHENRUFENJI_DEL = 0x05,
	MSG_834_ZHENRUFENJI_DEL_ACK = 0x06,	
	MSG_834_ZHENRUFENJI_GET= 0x07,
	MSG_834_ZHENRUFENJI_GET_ACK = 0x08,		
	MSG_834_GPORT_MODE_SET = 0x09,
	MSG_834_GPORT_MODE_SET_ACK = 0x0A,	
	MSG_834_GPORT_MODE_GET = 0x0B,
	MSG_834_GPORT_MODE_GET_ACK = 0x0C,	
	MSG_834_WORKMODE_SET = 0x0D,
	MSG_834_WORKMODE_SET_ACK = 0x0E,
	MSG_834_WORKMODE_GET = 0x0F,
	MSG_834_WORKMODE_GET_ACK = 0x10,	

	MSG_834_GROUP_ID_ADD = 0x11,
	MSG_834_GROUP_ID_ADD_ACK = 0x12,		
	MSG_834_GROUP_ID_DEL = 0x13,
	MSG_834_GROUP_ID_DEL_ACK = 0x14,	
	MSG_834_GROUP_ID_GET = 0x15,
	MSG_834_GROUP_ID_GET_ACK = 0x16,	
	MSG_834_GROUP_MEM_ADD = 0x17,
	MSG_834_GROUP_MEM_ADD_ACK = 0x18,		
	MSG_834_GROUP_MEM_DEL = 0x19,
	MSG_834_GROUP_MEM_DEL_ACK = 0x1A,	
	MSG_834_MSG_BUFF
};




/*******Global Variablies ******************/
SOCKET_TYPE g_paraPduRcvFd ;			/*Rcving fd from 834 board*/



static int sendBufTo834Board(unsigned char *buf, int len){
	int iRet = -1;
	if(!buf || len <=0){
		OSA_ERROR("Invalid Para");
		return iRet;
	}
	if(g_paraPduRcvFd <=0)
	{
		OSA_ERROR("Listen fd not created");
		return iRet;
	}
	iRet = osa_udpSendData(g_paraPduRcvFd,buf,len,"127.0.0.1",BOARD_834_PORT);
	if(iRet != len){
		OSA_ERROR("Send Fail");
		return  -1;
	}
	return iRet;
}
int  sndPktTo834Board(char *buf, int len){
	ST_SEAT_MNG_MSG msg;
	if(!buf || len <=0 )
		return -1;
	msg.header.protocol_version = 1;
	msg.header.dst_addr = BAOWEN_ADDR_TYPE_834_BOARD;
	msg.header.src_addr = BAOWEN_ADDR_TYPE_834_SNMP_AGENT;
	msg.header.msg_type = BAOWEN_MSG_TYPE_CMD;
	msg.header.data_len = len;
	memcpy(msg.body,buf,len);
	return sendBufTo834Board((unsigned char *)&msg,sizeof(ST_SEAT_MNG_HEADER) + len);
}

void sndQueryZrfjTab(){
	char buf[3];
	OSA_DBG_MSG("%s_%d",__func__,__LINE__);
	memset(buf,0x00,sizeof(buf));
	buf[0] = MSG_834_ZHENRUFENJI_GET;
	sndPktTo834Board(buf,sizeof(buf));
}

void sndQueryConfTab(){
	char buf[3];
	OSA_DBG_MSG("%s_%d",__func__,__LINE__);
	memset(buf,0x00,sizeof(buf));
	buf[0] = MSG_834_GROUP_ID_GET;
	sndPktTo834Board(buf,sizeof(buf));
}


static void handle716Para(unsigned char *buf, int len){
}
static void handle834Para(unsigned char *buf, int len){

	unsigned char cmd = buf[0];
	OSA_DBG_MSGX("cmd =%x",cmd);
	dispBuf(buf,len,__func__);
	short msgLen =  ntohs(*(short *)(&buf[1]));
	switch(cmd){
		case MSG_834_ZHENRUFENJI_GET_ACK:{
			short itemCnt = msgLen / sizeof(zhenRFJTab_type);
			zhenRFJTab_type *item = (zhenRFJTab_type *)&buf[3];
			setZrfjTab(item,itemCnt);
		}break;
		case MSG_834_GPORT_MODE_GET_ACK:{
		}break;
		case MSG_834_WORKMODE_GET_ACK:{
		}break;
		case MSG_834_GROUP_ID_GET_ACK:{
			int itemCnt = buf[3];
			int i =0;
			confTab_type *item = (confTab_type *)&buf[4];
			for(i = 0; i < itemCnt; i++){
				setConfTabItem(item,i);
				item += CONF_NAME_LEN + 1 + BCD_PHONE_NUM_LEN * (item->partCnt);
			}
			setConfTabItemCnt(itemCnt);
			
		}break;
		default:{
		}
	}
	
	
}
static void handle50Para(unsigned char *buf, int len){
	OSA_ERROR("unable process para from 50 suo board");
}



/*proc rcving pdu from board*/
static void paraProc(unsigned char *buf, int len){
	ST_SEAT_MNG_MSG *msg = NULL;

	if(!buf || len <=0)
		return;
	
	msg = (ST_SEAT_MNG_MSG *)buf;
	switch(msg->header.src_addr){
		case BAOWEN_ADDR_TYPE_716_BOARD:{
			/*handle pkt from 716 board*/
			handle716Para(msg->body,msg->header.data_len);
		}break;
		case BAOWEN_ADDR_TYPE_834_SNMP_AGENT:
		case BAOWEN_ADDR_TYPE_834_BOARD:{
			handle834Para(msg->body,msg->header.data_len);
		}break;
		case BAOWEN_ADDR_TYPE_50_BOARD:{
			handle50Para(msg->body,msg->header.data_len);
		}break;
		default:{
			OSA_ERROR("Unknow srcBoard Type=%x ", msg->header.src_addr);
		}
	}

	
}
void paraPduRcvThr(){
	struct sockaddr_in fromAddr;
	int fromAddrLen ;
	fd_set fSets;
	struct timeval time_out;
	int iRet = -1;
	unsigned char buf[500];

	fromAddrLen = sizeof(fromAddr);
	
	OSA_DBG_MSG("Thread %s Running, g_paraPduRcvFd=%d ",__func__,g_paraPduRcvFd);
	while(1)
		{
		FD_ZERO(&fSets);
		FD_SET(g_paraPduRcvFd,&fSets);
		time_out.tv_sec  = 0;
		time_out.tv_usec = 500*1000;
		iRet= select(g_paraPduRcvFd+1, &fSets,  NULL, NULL,  &time_out);
		//OSA_DBG_MSGX(" iRet =%d  ",iRet);
		if(iRet > 0){		
			if(FD_ISSET(g_paraPduRcvFd,&fSets)){
				memset(buf,0x00,sizeof(buf));
				iRet = recvfrom(g_paraPduRcvFd, buf, sizeof(buf), 0,  (struct sockaddr *)&fromAddr, &fromAddrLen);
				if(iRet > 0){
					//dispBuf (buf,iRet,__func__);
					paraProc(buf,iRet);
				}
				else{
					OSA_ERROR("recvfrom fail,errno=%d",errno);
				}
			}
		}	
		
	}

}
static SOCKET_TYPE initPduRcvPort(){
	SOCKET_TYPE fd =  osa_udpCreateBindSock(NULL,SNMP_AGENT_834_PORT);
	if(fd< 0){
		OSA_ERROR("Can't Create pdu recving port");
		return -1;
	}
	return fd;
}
int startAgentRcvPduThr(){
	int iRet = 0;
	OSA_ThrHndl handle;

	g_paraPduRcvFd =initPduRcvPort();
	if(g_paraPduRcvFd <0)
		return -1;

	iRet = OSA_ThreadCreate(&handle,paraPduRcvThr,NULL);
	if( iRet != 0 ){
		OSA_DBG_MSGX("create Thread  paraPduRcvThr Fail");
	}
	return iRet;
	
}



