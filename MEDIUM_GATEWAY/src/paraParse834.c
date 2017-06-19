/*
*@Desc: Implement para Inject for 834 board.
*@Author: Andy-wei.hou
*@Log:	   2017.01.21, created by Andy-wei.hou
*/
#include "PUBLIC.h"



extern int32 Board_Mng_SendTo_834(uint8 *buf, int32 len);
extern int waitQueryEventTimed(unsigned int order, int expTimMs);

typedef struct
{
	uint8 cmdId;
	uint8 replyId;
} Board834CmdReplyMap;

Board834CmdReplyMap Board834CmdReplyArray[] = {
	MSG_834_WORKMODE_SET, MSG_834_WORKMODE_SET_ACK, 
	MSG_834_ZHENRUFENJI_ADD, MSG_834_ZHENRUFENJI_ADD_ACK,
	MSG_834_GROUP_ID_ADD, MSG_834_GROUP_ID_ADD_ACK,
	MSG_834_GPORT_MODE_SET, MSG_834_GPORT_MODE_SET_ACK,
};

unsigned char getBoard834RespOrder(unsigned char queryOrder){
	int i;

	for (i = 0; i < sizeof(Board834CmdReplyArray)/sizeof(Board834CmdReplyArray[0]); i++) {
		if (queryOrder == Board834CmdReplyArray[i].cmdId)
			return Board834CmdReplyArray[i].replyId;
		}

	printf("%s, cann't find queryOrder! ERROR! \r\n", __func__);
	
	return queryOrder;	
}


static void dump_buf(unsigned char* str, const unsigned char* buf,int count)
{
	int i;
	
	printf("%s:\n", str);
	
	for(i=0;i<count;i++)
		printf("%2x ", buf[i]);

	printf("\n");

	return;
}

static void sendMsgTo834Board(uint8 *pMsg, int size){
	ST_SEAT_MNG_MSG *msg = pMsg;
	Board_Mng_SendTo_834(pMsg, size);
	waitQueryEventTimed(getBoard834RespOrder(msg->body[0]), 1000*2);//pMsg[0]:InfoType, is wait condition
}


/*configure work mode*/
int wkmode_cb(unsigned char *buf, int len) {
	int8 setMode = -1;
	ST_SEAT_MNG_MSG msg;
	dump_buf(__func__, buf, 1);

	setMode = buf[0];
	/*send mng pdu to notify 834board to do the configuration*/
	memset(&msg, 0x00,sizeof(msg));
	msg.header.protocol_version = 1;
	msg.header.dst_addr = BAOWEN_ADDR_TYPE_834_BOARD;
	msg.header.src_addr = BAOWEN_ADDR_TYPE_30_PC;
	msg.header.msg_type = BAOWEN_MSG_TYPE_CMD;
	msg.header.data_len = 0x04;
	msg.body[0] = MSG_834_WORKMODE_SET;
	msg.body[1] = 0x00;
	msg.body[2] = 0x01;
	msg.body[3] = setMode;

	sendMsgTo834Board(&msg, sizeof(ST_SEAT_MNG_HEADER) + msg.header.data_len);
	return 0;
}

/*configure zhenRuFenji, DianTaiZhiHu*/
int mgwpar_cb(unsigned char *buf, int len) {
	ST_SEAT_MNG_MSG msg;
	unsigned char *pFenJi = NULL, *pDianTai = NULL;
	int cnt = -1, i =0;

	/*Configure ZhenRuFenJi*/
	memset(&msg, 0x00,sizeof(msg));
	pFenJi = buf;
	msg.header.protocol_version = 1;
	msg.header.dst_addr = BAOWEN_ADDR_TYPE_834_BOARD;
	msg.header.src_addr = BAOWEN_ADDR_TYPE_30_PC;
	msg.header.msg_type = BAOWEN_MSG_TYPE_CMD;
	msg.header.data_len = 15;
	msg.body[0] = MSG_834_ZHENRUFENJI_ADD;
	msg.body[1] = 0x00;
	msg.body[2] = 12;

	cnt = pFenJi[0];
	dump_buf(__func__, buf, 1 + cnt * 2);
	for(i = 0; i < cnt; i++){
		msg.body[3] = pFenJi[1+i];
		msg.body[4] = pFenJi[1+i+1];
		sendMsgTo834Board(&msg, sizeof(ST_SEAT_MNG_HEADER) + msg.header.data_len);
	}

	/*Configure DianTaiZhiHu*/
	pDianTai = pFenJi + 1 + 2 * pFenJi[0];
	

	return 0;
}

/*configure HuiYi*/
int mgwcof_cb(unsigned char *buf, int len) {
	ST_SEAT_MNG_MSG msg;
	unsigned char *pConf = NULL;
	int cntConf = 0, i =0, cntMebs = 0;

	dump_buf(__func__, buf, len);

	/*Configure ZhenRuFenJi*/
	memset(&msg, 0x00,sizeof(msg));
	msg.header.protocol_version = 1;
	msg.header.dst_addr = BAOWEN_ADDR_TYPE_834_BOARD;
	msg.header.src_addr = BAOWEN_ADDR_TYPE_30_PC;
	msg.header.msg_type = BAOWEN_MSG_TYPE_CMD;

	cntConf = buf[0];
	if(cntConf > 0)
		pConf = &buf[1];
	DBG("cntConf = %d\n", cntConf);
	for(i = 0; i < cntConf; i++){
		cntMebs = pConf[2];
		msg.header.data_len = 3 + 3 + 10 * cntMebs ;
		msg.body[0] = MSG_834_GROUP_ID_ADD;
		msg.body[1] = 0x00;
		msg.body[2] = 3+10*cntMebs;
		dump_buf(__func__, pConf, 3 + 10 * cntMebs);
		memcpy(&msg.body[3],pConf,3+10*cntMebs);
		sendMsgTo834Board(&msg, sizeof(ST_SEAT_MNG_HEADER) + msg.header.data_len);
	}
	return 0;
}

int compar_cb(unsigned char *buf, int len) {
	int8 gMod = -1;
	ST_SEAT_MNG_MSG msg;
	dump_buf(__func__, buf, 1);

	gMod = buf[0];
	/*send mng pdu to notify 834board to do the configuration*/
	memset(&msg, 0x00,sizeof(msg));
	msg.header.protocol_version = 1;
	msg.header.dst_addr = BAOWEN_ADDR_TYPE_834_BOARD;
	msg.header.src_addr = BAOWEN_ADDR_TYPE_30_PC;
	msg.header.msg_type = BAOWEN_MSG_TYPE_CMD;
	msg.header.data_len = 6;
	msg.body[0] = MSG_834_GPORT_MODE_SET;
	msg.body[1] = 0x00;
	msg.body[2] = 3;
	msg.body[3] = 6;	/*for Z043, const setting chan_6*/
	msg.body[4] = 0;
	msg.body[5] = gMod;
	sendMsgTo834Board(&msg, sizeof(ST_SEAT_MNG_HEADER) + msg.header.data_len);

	
	return 0;
}


