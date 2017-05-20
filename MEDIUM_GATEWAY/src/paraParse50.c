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

/* 炮防网管协议
* 采用半字节转义，有两种转义表，标识C0和A0各代表一种
*/
typedef struct
{
	uint8 srcCode;
	uint8 ZhuanYiCode;
} ZhuanYiTab;

/*C0转义表*/
ZhuanYiTab ZhuanYiTabC0[] = {
	{0, 5},
	{1, 4},
	{2, 7},
	{3, 6},
	{4, 1},
	{5, 0},
	{6, 3},
	{7, 2},
	{8, 0xd},
	{9, 0xc},
	{0xa, 0xf},
	{0xb, 0xe},
	{0xc, 0x9},
	{0xd, 0x8},
	{0xe, 0xb},
	{0xf, 0xa},
};

/*A0转义表*/
ZhuanYiTab ZhuanYiTabA0[] = {
	{0, 0xa},
	{1, 0xb},
	{2, 8},
	{3, 9},
	{4, 0xe},
	{5, 0xf},
	{6, 0xc},
	{7, 0xd},
	{8, 2},
	{9, 3},
	{0xa, 0},
	{0xb, 1},
	{0xc, 6},
	{0xd, 7},
	{0xe, 4},
	{0xf, 5},
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

static void SrcToZhuanYiCode(int codeFormat, uint8 *data, int len)
{
	int i = 0;
	uint8 src = 0, dst = 0, tmp = 0;
	if (len < 0 || !data) {
		printf("%s,LINE %d, error \n", __func__, __LINE__);
	}
	dump_buf(__func__, data, len);
	
	if (codeFormat == 0xC0) {
		while (i < len) {
			src = data[i];
			tmp = (src & 0xf0) >> 4;
			dst = (ZhuanYiTabC0[tmp].ZhuanYiCode) << 4;
			//printf("src=0x%2x, tmp=0x%2x, dst=0x%2x\n",src, tmp, dst);
			tmp = src & 0x0f;
			dst |= (ZhuanYiTabC0[tmp].ZhuanYiCode);
			//printf("tmp=0x%2x, dst=0x%2x\n", tmp, dst);
			data[i] = dst;
			i++;
		}
	} else if (codeFormat == 0xA0) {
		while (i < len) {
			src = data[i];
			tmp = (src & 0xf0) >> 4;
			dst = (ZhuanYiTabA0[tmp].ZhuanYiCode) << 4;
			//printf("src=0x%2x, tmp=0x%2x, dst=0x%2x\n",src, tmp, dst);
			tmp = src & 0x0f;
			dst |= (ZhuanYiTabA0[tmp].ZhuanYiCode);
			//printf("tmp=0x%2x, dst=0x%2x\n", tmp, dst);
			data[i] = dst;
			i++;
		}
	} else {
		printf("%s unknown format! \n", __func__);
	}
	
	dump_buf(__func__, data, len);
}


static void sendNetMngMsgTo50Board(uint8 *pMsg, int size){
	uint8 header[6] = {0x0, 0x49, 0xf, 0, 0, 0};
	NET_MNG_50_MSG mngMsg;
	int len = 0;
		
	if(!pMsg || size <0)
		return;
	
	memset(&mngMsg, 0, sizeof(mngMsg));
	
	memcpy(mngMsg.Head.Header, header, 6);
	mngMsg.Head.SrcAddr = inet_addr(PAOFANG_BRD_MNG_IP_LOCAL);
	mngMsg.Head.DstAddrNum = 1;
	mngMsg.Head.DstAddr = inet_addr("50.1.36.167");
	mngMsg.Head.MsgLen = size + 1;
	mngMsg.Head.ZhuanYiTab = 0xc0;

	SrcToZhuanYiCode(mngMsg.Head.ZhuanYiTab, pMsg, size);
	memcpy(mngMsg.Data, pMsg, size);
	len = sizeof(NET_MNG_50_MSG_HEAD) + size;	
	dump_buf(__func__, (uint8 *)&mngMsg, len);
	
	Board_Net_Mng_SendTo_50((uint8 *)&mngMsg, len);

	//waitQueryEventTimed(ge50RespOrder(*(uint16 *)&pMsg[1]), 1000*2);//pMsg[0]:InfoType, is wait condition
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

#if 0
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
#else
int Board_Mng_50_Set_Node_Addr(uint32 NodeAddr, uint8 mask)
{
	MNG_Pf_Addr_MSG PfCmd;
	uint8 cmdId[3] = {0, 0x48, 0x1d};
	
	memset(&PfCmd, 0x0, sizeof(PfCmd));
	memcpy(PfCmd.InfoType, cmdId, 3);
	PfCmd.IPAddr= htonl(NodeAddr);
	PfCmd.Mask = mask;
	
	sendNetMngMsgTo50Board((uint8 *)&PfCmd,sizeof(PfCmd));

	printf("%s NodeAddr=0x%x, mask=%d\n", __func__, NodeAddr, mask);
	
	return DRV_OK;
}

#endif

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

	//Board_Mng_50_Set_VHF_Inf_Type(1, 1);
	
	Board_Mng_50_Set_Node_Addr(htonl(inet_addr("50.1.36.166")), 28);

	return 0;
}

