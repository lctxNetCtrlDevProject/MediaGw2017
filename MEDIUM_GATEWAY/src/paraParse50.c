/*
*@Desc: Implement para Inject for 50 board.
*@Log:	   2017.04.14
*/

#include "PUBLIC.h"
#include "50MngProto.h"


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

typedef struct
{
	uint16 cmdId;
	uint16 replyId;
} PfCmdReplyMap;

PfCmdReplyMap PfCmdReplyArray[] = {
	PF_INFO_TYPE_HANDSHAKE_CFG, PF_INFO_TYPE_HANDSHAKE_CFG_ACK,
	PF_INFO_TYPE_NODE_ADDR_CFG, PF_INFO_TYPE_NODE_ADDR_CFG_ACK,
	//PF_INFO_TYPE_VHF_INF_TYPE_CFG, PF_INFO_TYPE_VHF_INF_TYPE_CFG_ACK,
};

static void dump_buf(unsigned char* str, const unsigned char* buf,int count)
{
	int i;
	
	printf("%s:\n", str);
	
	for(i=0;i<count;i++)
		printf("%2x ", buf[i]);

	printf("\n");

	return;
}

uint16 ge50RespOrder(uint16 queryOrder){
	int i;

	for (i = 0; i < sizeof(PfCmdReplyArray)/sizeof(PfCmdReplyArray[0]); i++) {
		if (queryOrder == PfCmdReplyArray[i].cmdId)
			return PfCmdReplyArray[i].replyId;
	}

	printf("%s, cann't find queryOrder! ERROR! \r\n", __func__);
	
	return queryOrder;	
}

/* follow <携行式通信网络控制设备 设备参数管理接口协议V1.2(20160907)>
however, many cmd not support, so this func may not work, reserve this func as a backup.
Instead, please use sendNetMngMsgTo50Board() api to send net management cmd to PaoFang50.
*/
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
	waitQueryEventTimed(ge50RespOrder(*(uint16 *)&pMsg[1]), 1000*2);//pMsg[1]:InfoType, is wait condition
}

void SrcToZhuanYiCode(int codeFormat, uint8 *data, int len)
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

/* send net management cmd to PaoFang50.
follow <Pao Fang Wang Guan Xie Yi>
*/
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
	mngMsg.Head.DstAddr = get_paofang50_mng_ip();
	mngMsg.Head.MsgLen = size + 1;
	mngMsg.Head.ZhuanYiTab = 0xc0;

	SrcToZhuanYiCode(mngMsg.Head.ZhuanYiTab, pMsg, size);
	memcpy(mngMsg.Data, pMsg, size);
	len = sizeof(NET_MNG_50_MSG_HEAD) + size;	
	dump_buf(__func__, (uint8 *)&mngMsg, len);
	
	Board_Net_Mng_SendTo_50((uint8 *)&mngMsg, len);
}

static void sendNetMngMsgTo50Board_sync(uint8 *pMsg, int size, uint16 condition){
	sendNetMngMsgTo50Board(pMsg, size);
	waitQueryEventTimed(condition, 1000*5);
}

#if 0
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
#else
int Board_Mng_50_Handshake()
{
	uint8 cmdId[4] = {0, 0x20, 0, 0x01};

	set_paofang50_mng_ip(inet_addr("255.255.255.255"));
	sendNetMngMsgTo50Board_sync((uint8 *)&cmdId,sizeof(cmdId), PF_INFO_TYPE_HANDSHAKE_CFG_ACK);
	
	return DRV_OK;
}
#endif


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
	
	sendNetMngMsgTo50Board_sync((uint8 *)&PfCmd,sizeof(PfCmd), PF_INFO_TYPE_NODE_ADDR_CFG_ACK);
	printf("%s NodeAddr=0x%x, mask=%d\n", __func__, NodeAddr, mask);
	return DRV_OK;
}

#endif

int Board_Mng_50_Set_Phone_Len(uint8 len)
{
	uint8 PfCmd[4] = {0, 0x54, 0x25};
	PfCmd[3] = len;
	sendNetMngMsgTo50Board_sync((uint8 *)&PfCmd,sizeof(PfCmd), PF_INFO_TYPE_PHONENUM_LEN_CFG_ACK);
	printf("%s len=%d\n", __func__, len);
	return DRV_OK;
}

int Board_Mng_50_Del_Phone_Book_Num(uint32 ipAddr)
{
	uint8 PfCmd[7] = {0, 0x94, 0x26};
	*(uint32 *)&PfCmd[3] = htonl(ipAddr);
	sendNetMngMsgTo50Board_sync((uint8 *)&PfCmd,sizeof(PfCmd), PF_INFO_TYPE_PHONEBOOK_NUM_DEL_ACK1);	
	
	uint8 PfCmd2[7] = {0, 0x38, 0x26};
	*(uint32 *)&PfCmd2[3] = htonl(ipAddr);
	sendNetMngMsgTo50Board_sync((uint8 *)&PfCmd,sizeof(PfCmd), PF_INFO_TYPE_PHONEBOOK_NUM_DEL_ACK2);		
}

int Board_Mng_50_Set_Phone_Book_Num(uint32 ipAddr, uint8 *phoneNum)
{
	MNG_Pf_Set_Phone_Book_Num_MSG PfCmd;

	uint8 cmdId[3] = {0, 0x74, 0x26};
	memcpy(PfCmd.InfoType, cmdId, 3);
	PfCmd.IPAddr = htonl(ipAddr);
	PfCmd.Count = 1;
	memcpy(PfCmd.PhoneNum, phoneNum, 8);
	dump_buf(__func__, &PfCmd, sizeof(PfCmd));
	sendNetMngMsgTo50Board_sync((uint8 *)&PfCmd,sizeof(PfCmd), PF_INFO_TYPE_PHONEBOOK_NUM_ADD_ACK1);

	uint8 cmdId2[3] = {0, 0x38, 0x26};
	memcpy(PfCmd.InfoType, cmdId2, 3);
	PfCmd.IPAddr = ipAddr;
	sendNetMngMsgTo50Board_sync((uint8 *)&PfCmd,7, PF_INFO_TYPE_PHONEBOOK_NUM_ADD_ACK2);	

	printf("%s \n", __func__);
	
	return DRV_OK;
}

int pfyhhm_cb(unsigned char *buf, int len) {
	int cnt, i;
	uint8 *pCfg, phoneNum[8];
	uint32 ipAddr;

	cnt = buf[0];
	dump_buf(__func__, buf, 1 + cnt * 12);

	for(i = 0; i < cnt; i++){
		pCfg = &buf[1+i*12];
		dump_buf(__func__, pCfg, 12);
		ipAddr = ntohl(*(uint32 *)(pCfg));
		memcpy(phoneNum, pCfg + 4, 8);
		Board_Mng_50_Set_Phone_Book_Num(ipAddr, phoneNum);
	}
		
	return 0;
}

int Board_Mng_50_Set_Local_Phone_Num(uint8 *phoneNum, uint16 channel, uint8 priority, uint8 conven)
{
	MNG_Pf_Set_Local_Phone_Num_MSG PfCmd;
	uint8 cmdId[3] = {0, 0x64, 0x21};
	
	memcpy(PfCmd.InfoType, cmdId, 3);
	memcpy(PfCmd.PhoneNum, phoneNum, 8);
	PfCmd.Channel = channel;
	PfCmd.PriorityConven= (priority << 4 & 0xf0) | (conven & 0x0f);

	dump_buf(__func__, &PfCmd, sizeof(PfCmd));
	sendNetMngMsgTo50Board_sync((uint8 *)&PfCmd,sizeof(PfCmd), PF_INFO_TYPE_PHONE_LOCAL_NUM_ADD_ACK);
	
	return DRV_OK;
}

int Board_Mng_50_Del_Local_Phone_Num()
{
	uint8 cmdId[3] = {0, 0x80, 0x21};
	sendNetMngMsgTo50Board_sync((uint8 *)&cmdId,sizeof(cmdId), PF_INFO_TYPE_PHONE_LOCAL_NUM_DEL_ACK1);

	uint8 cmdId2[3] = {0, 0x24, 0x21};
	sendNetMngMsgTo50Board_sync((uint8 *)&cmdId,sizeof(cmdId), PF_INFO_TYPE_PHONE_LOCAL_NUM_DEL_ACK2);
	
	return DRV_OK;
}

int pfbdhm_cb(unsigned char *buf, int len) {
	int cnt, i;
	uint8 *pCfg, prior, conv, phoneNum[8];
	uint16 channel;

	cnt = buf[0];
	dump_buf(__func__, buf, 1 + cnt * 8);

	for(i = 0; i < cnt; i++){
		pCfg = &buf[1+i*8];
		channel = ntohs(*(uint16 *)(pCfg));
		memcpy(phoneNum, pCfg+2, 8);
		prior = *(pCfg + 10);
		conv = *(pCfg + 11);
		Board_Mng_50_Set_Local_Phone_Num(phoneNum, channel, prior, conv);
	}
		
	return 0;
}

int Board_Mng_50_Add_Conf(uint16 confNum, uint8 membCnt, uint8* membs)
{
	MNG_Pf_Add_Conf_MSG PfCmd;
	int i;

	memset(&PfCmd, 0, sizeof(PfCmd));
	uint8 cmdId[3] = {0, 0x7C, 0x23};
	memcpy(PfCmd.InfoType, cmdId, 3);
	PfCmd.ConfNum = confNum;
	PfCmd.MembCnt = membCnt;
	DBG("%s confNum=%d, membCnt=%d\n", __func__, confNum, membCnt);
	for (i = 0; i < membCnt; i++) {
		memcpy(&PfCmd.Membs[i][0], membs+8*i, 8);
	}
	sendNetMngMsgTo50Board_sync((uint8 *)&PfCmd, 6+8*membCnt, PF_INFO_TYPE_CONF_ADD_ACK1);

	uint8 cmdId2[3] = {0, 0x20, 0x23};
	sendNetMngMsgTo50Board_sync((uint8 *)&cmdId2, sizeof(cmdId2), PF_INFO_TYPE_CONF_ADD_ACK2);	
	return DRV_OK;
}

int Board_Mng_50_Del_Conf(uint16 confNum)
{

	uint8 cmdId[5] = {0, 0x9c, 0x23};
	*(uint16 *)&cmdId[3] = htons(confNum);
	sendNetMngMsgTo50Board_sync((uint8 *)&cmdId, sizeof(cmdId), PF_INFO_TYPE_CONF_ADD_ACK1);

	uint8 cmdId2[3] = {0, 0x20, 0x23};
	sendNetMngMsgTo50Board_sync((uint8 *)&cmdId2, sizeof(cmdId2), PF_INFO_TYPE_CONF_ADD_ACK2);	
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

/* 将整形数转成N字节数组*/
/* 例如0x1234 ->a[8] : {34, 12, 0, 0, 0, 0, 0, 0} */
void int_to_array(int data, uint8 *array, int N) {
	int i;

	for (i = 0; i < N; i++)
		array[i] = (data >> (8 * i)) & 0xff;

	DBG("%s data=0x%x\n", __func__, data);
	dump_buf(__func__, array, N);
}

/* 将整形数转成N字节数组*/
/* 例如0x1234 ->a[8] : {0, 0, 0, 0, 0, 0, 12, 34} */

void int_to_array_reverse(int data, uint8 *array, int N) {
	int i;

	for (i = 0; i < N; i++)
		array[N-i-1] = (data >> (8 * i)) & 0xff;

	DBG("%s data=0x%x\n", __func__, data);
	dump_buf(__func__, array, N);
}


int test_example_board_50()
{
	printf("=========================================Board_Mng_50_Handshake \n");
	Board_Mng_50_Handshake();

	//printf("=========================================Board_Mng_50_Set_Node_Addr \n");
	//Board_Mng_50_Set_Node_Addr(htonl(inet_addr("50.1.36.165")), 25);

	//printf("=========================================Board_Mng_50_Set_Phone_Len \n");
	//Board_Mng_50_Set_Phone_Len(7);
	//printf("==========================================Board_Mng_50_Set_Phone_Book_Num\n");
	//uint32 phoneNum1 = 2222222;
	//uint8 phoneNum[8];
	//int_to_array_reverse(phoneNum1,phoneNum,8);
	//Board_Mng_50_Set_Phone_Book_Num(get_paofang50_mng_ip(), phoneNum);
	//printf("==========================================Board_Mng_50_Set_Local_Phone_Num\n");
	//int channel = 10;
	//Board_Mng_50_Set_Local_Phone_Num(phoneNum, channel, 1, 1);
	//printf("==========================================Board_Mng_50_Del_Phone_Book_Num\n");
	//Board_Mng_50_Del_Phone_Book_Num(get_paofang50_mng_ip());	
	//printf("==========================================Board_Mng_50_Del_Local_Phone_Num\n");
	//Board_Mng_50_Del_Local_Phone_Num();
	//printf("===========================================Board_Mng_50_Add_Conf \n");
	//uint8 membs[3*8];
	//uint32 memb[3] = {7111906, 7111907, 7111908};
	//uint8 cnt = 3;
	//int conf = 124;
	//int i;
	//for (i = 0; i < cnt; i++)
	//	int_to_array_reverse(memb[i], membs + 8*i, 8);
	//Board_Mng_50_Add_Conf(conf, cnt, membs);	
	//printf("===========================================Board_Mng_50_Del_Conf \n");
	//Board_Mng_50_Del_Conf(conf);

	
	sleep(100);
	return 0;
}

