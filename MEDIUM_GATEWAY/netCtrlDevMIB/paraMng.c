/**
*@Desc: Get para from 834Board, 716Board by PDU of <xieXingWangKong SheBeiCanShuGuanLiJieKouXieYi>
*@Author: Andy-wei.hou
*@Log:	Created by Andy-wei.hou 2017.02.14
*/
#include <errno.h>
#include "osa.h"
#include "mgwParaAccess.h"
#include "zwParaAccess.h"

#include "interior_protocol.h"
#include "MGW_lan_bus_control.h"
#include "716MngProto.h"
#include "public.h"

#define	SNMP_AGENT_834_PORT	50002
#define 	BOARD_834_PORT		30006

#define 	BOARD_716_PORT		6060
#define 	BOARD_716_IP		"192.168.254.1"

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


//----------------------834 Board------------------------------------------------
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
	dispBuf((unsigned char *)&msg,sizeof(ST_SEAT_MNG_HEADER) + len,__func__);
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

void sndQueryworkMode(){
	char buf[4];
	short *len = (short *)&buf[1];
	OSA_DBG_MSG("%s_%d",__func__,__LINE__);
	memset(buf,0x00,sizeof(buf));
	buf[0] = MSG_834_WORKMODE_GET;
	*len = htons(1);
	sndPktTo834Board(buf,sizeof(buf));
}
void sndQueryGPortMode(){
	char buf[4];
	short *len = (short *)&buf[1];
	OSA_DBG_MSG("%s_%d",__func__,__LINE__);
	memset(buf,0x00,sizeof(buf));
	buf[0] = MSG_834_GPORT_MODE_GET;
	*len = htons(1);
	buf[3] = 6;
	sndPktTo834Board(buf,sizeof(buf));
}

//--------------------------Zw 716 Board------------------------------------
static int sendBufTo716Board(unsigned char *buf, int len){
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
	iRet = osa_udpSendData(g_paraPduRcvFd,buf,len, BOARD_716_IP, BOARD_716_PORT);
	if(iRet != len){
		OSA_ERROR("Send Fail");
		return  -1;
	}
	return iRet;
}

int sndPktTo716Board(uint8 *pMsg, int size){
	ST_HF_MGW_DCU_PRO Regmsg;
	int len = 0;
	if(!pMsg || size <0)
		return;
	memset(&Regmsg, 0x0, sizeof(Regmsg));
	memcpy(Regmsg.body, pMsg, size);

	/* 因716厂的通信控制板软件沿用了Z018海防项目与50所之间的板间参数交互协议，
	故业务接口板发送给716通信控制板的消息源地址需填充50所炮防通信控制板的地址0x10 */
	Regmsg.header.baowen_type = 0xA0; //0xA0;协议上写0x0A
	Regmsg.header.dst_addr = BAOWEN_ADDR_TYPE_716_BOARD;
	Regmsg.header.src_addr = BAOWEN_ADDR_TYPE_50_BOARD;
	Regmsg.header.info_type = BAOWEN_MSG_TYPE_CMD;
	Regmsg.header.data_len = size;

	len = sizeof(ST_HF_MGW_DCU_PRO_HEADER) + Regmsg.header.data_len;	
	dispBuf((unsigned char *)&Regmsg, len, __func__);
	return sendBufTo716Board((unsigned char *)&Regmsg,len);
	//Board_Mng_SendTo_716((uint8 *)&Regmsg, len);
}

void sndQueryZwMode(){
	MNG_ZW_DEV_ID_GET_MSG ZxCmd;
	memset(&ZxCmd, 0x00, sizeof(ZxCmd));
	ZxCmd.InfoType = MSG_716_MODE_GET_MSG;
	ZxCmd.CmdLen = htonl(2);

	OSA_DBG_MSG("%s_%d",__func__,__LINE__);
	sndPktTo716Board((uint8 *)&ZxCmd,sizeof(ZxCmd));
}

void sndQueryDevSeq(){
	MNG_ZW_DEV_ID_GET_MSG ZxCmd;
	memset(&ZxCmd, 0x00, sizeof(ZxCmd));
	ZxCmd.InfoType = MSG_716_MODE_GET_MSG;
	ZxCmd.CmdLen = htonl(2);

	OSA_DBG_MSG("%s_%d",__func__,__LINE__);
	sndPktTo716Board((uint8 *)&ZxCmd,sizeof(ZxCmd));
}

void sndQueryArmyId(){
	MNG_ZW_ARMY_ID_GET_MSG ZxCmd;
	memset(&ZxCmd, 0x00, sizeof(ZxCmd));
	ZxCmd.InfoType = MSG_716_ARMY_ID_GET_MSG;
	ZxCmd.CmdLen = htonl(2);

	OSA_DBG_MSG("%s_%d",__func__,__LINE__);
	sndPktTo716Board((uint8 *)&ZxCmd,sizeof(ZxCmd));
}

void sndQueryPhoneLen(){
	MNG_ZW_PHONE_LEN_GET_MSG ZxCmd;
	memset(&ZxCmd, 0x00, sizeof(ZxCmd));
	ZxCmd.InfoType = MSG_716_PHONE_LEN_GET_MSG;
	ZxCmd.CmdLen = htonl(2);

	OSA_DBG_MSG("%s_%d",__func__,__LINE__);
	sndPktTo716Board((uint8 *)&ZxCmd,sizeof(ZxCmd));
}

void sndQueryAudioCodec(){
	MNG_ZW_AUDIO_CODEC_GET_MSG ZxCmd;
	memset(&ZxCmd, 0x00, sizeof(ZxCmd));
	ZxCmd.InfoType = MSG_716_AUDIO_CODEC_GET_MSG;
	ZxCmd.CmdLen = htonl(2);

	OSA_DBG_MSG("%s_%d",__func__,__LINE__);
	sndPktTo716Board((uint8 *)&ZxCmd,sizeof(ZxCmd));
}

void sndQueryAsNum(){
	MNG_ZW_AS_NUM_GET_MSG ZxCmd;
	memset(&ZxCmd, 0x00, sizeof(ZxCmd));
	ZxCmd.InfoType = MSG_716_AS_NUM_GET_MSG;
	ZxCmd.CmdLen = htonl(2);

	OSA_DBG_MSG("%s_%d",__func__,__LINE__);
	sndPktTo716Board((uint8 *)&ZxCmd,sizeof(ZxCmd));
}

void sndQueryZwUsrNumTabAll(){
	MNG_ZW_USR_CFG_PKT ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.header.InfoType = MSG_716_USR_NUM_GET_MSG;
	ZxCmd.header.CmdLen = htons(4);
	ZxCmd.CmdId = ZW_USR_ALL_NUM;	
	ZxCmd.OpMode = ZW_USR_QUERY;
	ZxCmd.MsgLen = 1;
	ZxCmd.Ack = 1;

	OSA_DBG_MSG(" ");
	dispBuf((unsigned char *)&ZxCmd, 7, __func__);
	
	sndPktTo716Board((uint8 *)&ZxCmd,7);
}

void sndQueryZwUsrNumTabIndex(uint16 index){
	MNG_ZW_USR_CFG_PKT ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.header.InfoType = MSG_716_USR_NUM_GET_MSG;
	ZxCmd.header.CmdLen = 6;
	ZxCmd.CmdId = ZW_USR_ALL_NUM;	
	ZxCmd.OpMode = ZW_USR_QUERY;
	ZxCmd.MsgLen = 3;
	ZxCmd.Ack = 1;
	ZxCmd.Index = htons(index);

	OSA_DBG_MSG("%s_%d",__func__,__LINE__);
	sndPktTo716Board((uint8 *)&ZxCmd,9);
}

void sndQueryZwConfTabIndex(uint16 index){
	MNG_ZW_CONF_CFG_PKT ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.header.InfoType = MSG_716_CONF_GET_MSG;
	ZxCmd.header.CmdLen = htons(6);
	ZxCmd.CmdId = ZW_MEET_ALL_NUM;	
	ZxCmd.OpMode = ZW_USR_QUERY;
	ZxCmd.MsgLen = 3;
	ZxCmd.Ack = 0;
	ZxCmd.Index = htons(index);

	OSA_DBG_MSG("%s_%d",__func__,__LINE__);
	dispBuf((unsigned char *)&ZxCmd, 9, __func__);
	
	sndPktTo716Board((uint8 *)&ZxCmd,9);
}


void sndQueryZwConfTabAll(){
	MNG_ZW_CONF_CFG_PKT ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.header.InfoType = MSG_716_CONF_GET_MSG;
	ZxCmd.header.CmdLen = htons(3);
	ZxCmd.CmdId = ZW_MEET_ALL_NUM;	
	ZxCmd.OpMode = ZW_USR_QUERY;
	ZxCmd.MsgLen = 0;

	OSA_DBG_MSG(" ");
	dispBuf((unsigned char *)&ZxCmd, 6, __func__);
	
	sndPktTo716Board((uint8 *)&ZxCmd,6);
}

//----------------------------Main Code----------------------------------------------

static void handle716Para(unsigned char *buf, int len){
	unsigned char cmd = buf[0];
	OSA_DBG_MSGX("cmd =%x",cmd);

	dispBuf(buf,len,__func__);
	short msgLen =	ntohs(*(short *)(&buf[1]));
	
	switch(cmd){
		case MSG_716_MODE_GET_MSG_ACK:{
			setZwMode(buf[3]);
			setDevSeq(ntohs(*(uint16 *)&buf[4]));
		}break;
		case MSG_716_ARMY_ID_GET_MSG_ACK:{
			setArmyId(ntohs(*(uint16 *)&buf[3]));
		}break;
		case MSG_716_PHONE_LEN_GET_MSG_ACK:{
			setPhoneLen(buf[3]);
		}break;
		case MSG_716_AUDIO_CODEC_GET_MSG_ACK:{
			setAudioCodec(buf[3]);
		}break;	
		case MSG_716_AS_NUM_GET_MSG_ACK:{
			setAsNum(ntohl(*(uint32 *)&buf[3]));
		}break;	
		case MSG_716_USR_NUM_GET_MSG_ACK:{
			static int zwUsrNum_index = 0;
			static zwUsrNum_type zwUsrNum[USR_NUM_ITEM_MAX];

			if (buf[4] != ZW_USR_QUERY_ACK) //only handle query ack
				break;
		
			if (buf[6] == 1) {//usrNum exit		
				OSA_DBG_MSGX("zwUsrNum_index =%d", zwUsrNum_index);

				zwUsrNum[zwUsrNum_index].chanId = buf[16];
				memcpy(zwUsrNum[zwUsrNum_index].usrNum, &buf[9], USR_NUM_LEN); 
				memcpy(zwUsrNum[zwUsrNum_index].secNum,&buf[13], SEC_NUM_LEN);
				//dispBuf(&zwUsrNum[zwUsrNum_index], sizeof(zwUsrNum[zwUsrNum_index]), __func__);
				zwUsrNum_index++; 
				sndQueryZwUsrNumTabIndex((*(uint16 *)&buf[7]) + 1); //index
			} else {//usrNum not exit, or the last usrNum
				OSA_DBG_MSGX(" usrNum is end, or not exist");
				setUsrNumTab(zwUsrNum, zwUsrNum_index);
				zwUsrNum_index = 0;
				memset(zwUsrNum, 0, sizeof(zwUsrNum));
			}
		}break;
		case MSG_716_CONF_GET_MSG_ACK:{
			static int zwConf_index = 0;
			static zwConf_type zwConf[CONF_ITEM_MAX];
			int i;
			
			if (buf[4] != ZW_CONF_QUERY_ACK) //only handle query ack
				break;

			if (buf[6]) {//conf exit	
				OSA_DBG_MSGX("zwConf_index =%d", zwConf_index);
				zwConf[zwConf_index].confNum = *(uint16 *)&buf[9];
				zwConf[zwConf_index].partCnt = buf[11];
				for (i = 0; i < CONF_PART_CNT; i++) {
					memcpy(&zwConf[zwConf_index].partNum[i], &buf[12 + 4*i], CONF_PART_NUM_LEN);
				}
				//memcpy(zwConf[zwConf_index].partNum, &buf[12], CONF_PART_NUM_LEN * buf[11]);
				zwConf_index++; 
				sndQueryZwConfTabIndex((*(uint16 *)&buf[7]) + 1); //index
			} else {//conf not exit, or the last conf
				OSA_DBG_MSGX(" conf is end, or not exist");
				setZwConfTab(zwConf, zwConf_index);
				zwConf_index = 0;
				memset(zwConf, 0, sizeof(zwConf));			

			}
		}break;
		default:{
		}
	}		
}

static void handle834Para(unsigned char *buf, int len){

	unsigned char cmd = buf[0];
	OSA_DBG_MSGX("cmd =%x",cmd);
	//dispBuf(buf,len,__func__);
	short msgLen =  ntohs(*(short *)(&buf[1]));
	switch(cmd){
		case MSG_834_ZHENRUFENJI_GET_ACK:{
			short itemCnt = msgLen / sizeof(zhenRFJTab_type);
			zhenRFJTab_type *item = (zhenRFJTab_type *)&buf[3];
			setZrfjTab(item,itemCnt);
		}break;
		case MSG_834_GPORT_MODE_GET_ACK:{
			setGPortMode(buf[5]);
		}break;
		case MSG_834_WORKMODE_GET_ACK:{
			setWorkMode(buf[3]);
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



