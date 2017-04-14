/*
*@Desc: Implement para Inject for 50 board.
*@Log:	   2017.04.14
*/

#include "PUBLIC.h"
#include "50MngProto.h"


typedef struct
{
	uint16 cmdId;
	uint16 replyId;
} PfCmdReplyMap;

PfCmdReplyMap PfCmdReplyArray[] = {
	PF_INFO_TYPE_HANDSHAKE_CFG, PF_INFO_TYPE_HANDSHAKE_CFG_ACK,
	PF_INFO_TYPE_NODE_ADDR_CFG, PF_INFO_TYPE_NODE_ADDR_CFG_ACK,
	PF_INFO_TYPE_VHF_INF_TYPE_CFG, PF_INFO_TYPE_VHF_INF_TYPE_CFG_ACK,
};

uint16 ge50RespOrder(uint16 queryOrder){
	int i;

	for (i = 0; i < sizeof(PfCmdReplyArray)/sizeof(PfCmdReplyArray[0]); i++) {
		if (queryOrder == PfCmdReplyArray[i].cmdId)
			return PfCmdReplyArray[i].replyId;
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

static void sendMsgTo50Board(uint8 *pMsg, int size){
	MNG_50_MSG Regmsg;
	int len = 0;
		
	if(!pMsg || size <0)
		return;
	
	memset(&Regmsg, 0x0, sizeof(Regmsg));

#if 1
	Regmsg.Head.ProType = 0x00;
	Regmsg.Head.VerId = 0x01;
	Regmsg.Head.Headlen = 0x0f;
	Regmsg.Head.PackType = 0x80;
	Regmsg.Head.ChanId = 0x00;
	Regmsg.Head.ServiceType = 0x04;
	Regmsg.Head.SrcAddr = BAOWEN_ADDR_TYPE_834_BOARD;
	Regmsg.Head.NumOfDstAddr = 0x01;
	Regmsg.Head.DstAddr = BAOWEN_ADDR_TYPE_50_BOARD;
	Regmsg.Head.MsgLen = size;
	memcpy(Regmsg.Data, pMsg, size);
#else
	unsigned char herader50[MSG_HEADER_LEN + 2] = {
		0x00,0x01,0x0f,0x80,0x00,0x04,0x0a,0x00,0x00,0x06,0x01,0x0a,0x00,0x00,0x01,0x00,0x00
	};

	memcpy(&Regmsg.Head, herader50, sizeof(herader50));
	Regmsg.Head.MsgLen = size;
	memcpy(Regmsg.Data, pMsg, size);
#endif

	len = sizeof(MNG_50_MSG_HEAD) + Regmsg.Head.MsgLen;	
	dump_buf(__func__, (uint8 *)&Regmsg, len);
	
	Board_Mng_SendTo_50((uint8 *)&Regmsg, len);

	waitQueryEventTimed(ge50RespOrder(*(uint16 *)&pMsg[1]), 1000*2);//pMsg[0]:InfoType, is wait condition
}

int Board_Mng_50_Handshake()
{

	MNG_Pf_Handshake_MSG PfCmd;
	memset(&PfCmd, 0x0, sizeof(PfCmd));

	PfCmd.PortAddr = 0x10;
	PfCmd.InfoType = htons(PF_INFO_TYPE_HANDSHAKE_CFG);
	PfCmd.CmdLen = 0x0;
	
	sendMsgTo50Board((uint8 *)&PfCmd,sizeof(PfCmd));
	
	return DRV_OK;
}

int Board_Mng_50_Set_Node_Addr(uint32 NodeAddr)
{

	MNG_Pf_Node_Addr_MSG PfCmd;
	memset(&PfCmd, 0x0, sizeof(PfCmd));

	PfCmd.PortAddr = 0x10;
	PfCmd.InfoType = htons(PF_INFO_TYPE_NODE_ADDR_CFG);
	PfCmd.CmdLen = 0x4;
	PfCmd.NodeAddr = htonl(NodeAddr);
	
	sendMsgTo50Board((uint8 *)&PfCmd,sizeof(PfCmd));

	printf("%s NodeAddr=0x%x \n", __func__, NodeAddr);
	
	return DRV_OK;
}

int Board_Mng_50_Set_VHF_Inf_Type(uint8 InfId, uint8 InfType)
{

	MNG_Pf_VHF_Inf_Type_MSG PfCmd;
	memset(&PfCmd, 0x0, sizeof(PfCmd));

	PfCmd.PortAddr = 0x20;
	PfCmd.InfoType = htons(PF_INFO_TYPE_VHF_INF_TYPE_CFG);
	PfCmd.CmdLen = 0x2;
	PfCmd.InfId = InfId;
	PfCmd.InfType = InfType;
	
	sendMsgTo50Board((uint8 *)&PfCmd,sizeof(PfCmd));
	
	return DRV_OK;
}

int test_example_board_50()
{
	int i;

	for (i = 0; i < 3; i++) { 
		Board_Mng_50_Handshake();
		sleep(3);
	}

	Board_Mng_50_Set_VHF_Inf_Type(1, 1);
	
	//Board_Mng_50_Set_Node_Addr(htonl(inet_addr("193.168.50.50")));

	return 0;
}

