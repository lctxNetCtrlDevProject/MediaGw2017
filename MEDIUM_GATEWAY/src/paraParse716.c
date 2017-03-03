/*
*@Desc: Implement para Inject for 716 board.
*@Author: Andy-wei.hou
*@Log:	   2017.02.03, created by Andy-wei.hou
*/

#include "PUBLIC.h"
#include "716MngProto.h"



extern int32 Board_Mng_SendTo_716(uint8 *buf, int32 len);

//------------------------------------------------------------------------------

/**
网管参数解析和分发
*/

static void dump_buf(unsigned char* str, const unsigned char* buf,int count)
{
	int i;
	
	printf("%s:\n", str);
	
	for(i=0;i<count;i++)
		printf("%2x ", buf[i]);

	printf("\n");

	return;
}

static void sendMsgTo716Board(uint8 *pMsg, int size){
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
	Board_Mng_SendTo_716((uint8 *)&Regmsg, len);
}

static void sendMsgTo716Ray(uint8 msgId, uint8 *pMsg, int size){
	ST_RAY_MNG_MSG Regmsg;
	int len = 0;

	if(!pMsg || size <0)
		return;
	memset(&Regmsg, 0x0, sizeof(Regmsg));

	Regmsg.header.msg_type = RAY_MSG_TYPE_SET;
	Regmsg.header.msg_flg = msgId;
	Regmsg.header.board_local = 0x0a;
	Regmsg.header.board_id = 00;
	Regmsg.header.data_len = size;
	memcpy(Regmsg.body, pMsg, size);

	len = sizeof(ST_RAY_MNG_HEADER) + Regmsg.header.data_len;

	Board_Mng_SendTo_716_Ray((uint8 *)&Regmsg, len);
}


/*Configure devId and devType*/
int Board_Mng_716_Set_Dev_Id(uint8 DevType, uint16 DevId)
{

	MNG_ZW_DEV_ID_MSG ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.InfoType = 0x91;
	ZxCmd.CmdLen = 0x03;
	ZxCmd.DevType = DevType;
	ZxCmd.DevId = htons(DevId);
	
	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));

	//printf("%s DevType=0x%x, DevId=0x%2x\n", __func__, DevType, DevId);
	
	return DRV_OK;
}


/*Configure ID of Army*/
int Board_Mng_716_Set_Army_Id(uint16 Id)
{
	MNG_ZW_ARMY_ID_MSG ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.InfoType = 0x92;
	ZxCmd.CmdLen = 0x02;
	ZxCmd.ArmyId = htons(Id);
	
	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));
	//printf("%s Id=0x%x\n", __func__, Id);
	
	return DRV_OK;
}

//NumLen: 3~7
/*configure phone Len*/
int Board_Mng_716_Set_Num_Len(uint8 NumLen)
{
	MNG_ZW_NUM_LEN_MSG ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.InfoType = 0x93;
	ZxCmd.CmdLen = 0x02;
	ZxCmd.NumLen = NumLen;
	
	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));

	//printf("%s NumLen=0x%x\n", __func__, NumLen);
	
	return DRV_OK;
}

//mode:4,7
/*configure audio codec type*/
int Board_Mng_716_Set_Speech_Encode(uint8 mode)
{
	MNG_ZW_SPEECH_ENCODE_MSG ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.InfoType = 0x94;
	ZxCmd.CmdLen = 0x02;
	ZxCmd.Mode = mode;
	
	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));

	//printf("%s mode=0x%x\n", __func__, mode);
	
	return DRV_OK;
}

/*configure devType, devId, armyId, phoneLen,audioCodecType*/
int zwjrpa_cb(unsigned char *buf, int len) {
	uint16 DevId, ArmyId;
	uint8 DevType, NumLen, SpeechEncode;
	
	dump_buf(__func__, buf, 11);
	DevType = buf[0];
	//DevId = (buf[2] << 8) | buf[1]; 
	DevId = ntohs(*(unsigned short *) &buf[1]);
	ArmyId = ntohs(*(unsigned short *) &buf[3]);
	NumLen = buf[5];
	SpeechEncode = buf[6];

	Board_Mng_716_Set_Dev_Id(DevType, DevId);
	Board_Mng_716_Set_Army_Id(ArmyId);
	Board_Mng_716_Set_Num_Len(NumLen);
	Board_Mng_716_Set_Speech_Encode(SpeechEncode);

	return 0;
}

/**/
int Board_Mng_Set_Inf_Type(uint8 PortId, uint8 Type)
{
	MNG_ZW_INF_TYPE_MSG ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.InfoType = 0x96;
	ZxCmd.CmdLen = 0x02;
	ZxCmd.PortId = PortId;
	ZxCmd.Type = Type;

	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));
	//printf("%s PortId=%d, Type=0x%x\n", __func__, PortId, Type);
	
	return DRV_OK;
}


int Board_Mng_Set_Inf_Speed(uint8 PortId, uint8 Speed)

{
	MNG_ZW_INF_SPEED_MSG ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.InfoType = 0x98;
	ZxCmd.CmdLen = 0x02;
	ZxCmd.PortId = PortId;
	ZxCmd.PortSpeed = Speed;

	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));
	//printf("%s PortId=%d, Speed=0x%x\n", __func__, PortId, Speed);
	
	return DRV_OK;
}

int Board_Mng_Set_Inf_Mode(uint8 PortId, uint8 Mode)

{
	MNG_ZW_INF_Mode_MSG ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.InfoType = 0x9a;
	ZxCmd.CmdLen = 0x02;
	ZxCmd.PortId = PortId;
	ZxCmd.Mode = Mode;

	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));

	//printf("%s PortId=%d, Mode=0x%x\n", __func__, PortId, Mode);
	
	return DRV_OK;
}


int Board_Mng_Set_Inf_Clk(uint8 PortId, uint8 Clk)
{
	MNG_ZW_INF_Clk_MSG ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.InfoType = 0x9f;
	ZxCmd.CmdLen = 0x02;
	ZxCmd.PortId = PortId;
	ZxCmd.PortClk = Clk;

	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));
	//printf("%s PortId=%d, Clk=0x%x\n", __func__, PortId, Clk);
	
	return DRV_OK;
}

int Board_Mng_Set_Inf_SM(uint8 PortId, uint8 Master)
{
	MNG_ZW_INF_SM_MSG ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.InfoType = 0xb4;
	ZxCmd.CmdLen = 0x02;
	ZxCmd.PortId = PortId;
	ZxCmd.Master = Master;

	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));
	//printf("%s PortId=%d, Master=0x%x\n", __func__, PortId, Master);
	
	return DRV_OK;
}

int Board_Mng_Set_Inf_Huanhui(uint8 PortId, uint8 Huanhui)
{
	MNG_ZW_INF_HH_MSG ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.InfoType = 0xa5;
	ZxCmd.CmdLen = 0x02;
	ZxCmd.PortId = PortId;
	ZxCmd.Huanhui = Huanhui;

	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));
	//printf("%s PortId=%d, Huanhui=0x%x\n", __func__, PortId, Huanhui);
	
	return DRV_OK;
}

int qlintf_cb(unsigned char *buf, int len) {	
	typedef struct qlintf_tag_msg {
		uint8 id;
		uint8 type;
		uint8 speed;
		uint8 mode;
		uint8 clk;
		uint8 master;
		uint8 huanhui;
	} qlintf_msg;
	
	uint8 num, i;
	qlintf_msg *msg = &buf[1];

	num = buf[0];
	dump_buf(__func__, buf, 1);
	for (i = 0; i < num; i++) {
		dump_buf(__func__, &msg[i], 7);
		if (msg[i].id == 1)
			Board_Mng_Set_Inf_Type(msg[i].id, msg[i].type);
		Board_Mng_Set_Inf_Speed(msg[i].id, msg[i].speed);     

		if ((msg[i].id == 0x0d) || (msg[i].id == 0x0e) || ((msg[i].id == 0x1) && (msg[i].type == 2)))
			Board_Mng_Set_Inf_Mode(msg[i].id, msg[i].mode);
		
		Board_Mng_Set_Inf_Clk(msg[i].id, msg[i].clk);
		Board_Mng_Set_Inf_SM(msg[i].id, msg[i].master);
		Board_Mng_Set_Inf_Huanhui(msg[i].id, msg[i].huanhui);
		sleep(1);
	}

	return 0;
}

int Board_Mng_Set_User_Num(uint8 *pUsrNum, uint16 SecurityNum, uint8 channel)
{
	ST_HF_MGW_DCU_PRO Regmsg;
	MNG_ZW_USR_CFG_PKT ZxCmd;
	int len = 0;

	memset(&Regmsg, 0x0, sizeof(Regmsg));
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.header.InfoType = ZW_INFO_TYPE_USR_CFG;
	ZxCmd.header.CmdLen = 33;
	ZxCmd.CmdId = ZW_USR_NUM_SPEC;	
	ZxCmd.OpMode = ZW_USR_REG;
	ZxCmd.MsgLen = 30;
	ZxCmd.Ack = 1;
	memcpy(ZxCmd.UserNum,pUsrNum,4);
	ZxCmd.SecurityNum = SecurityNum;
	ZxCmd.ChannelId = channel;//此项必须有

	memcpy(Regmsg.body, &ZxCmd, sizeof(ZxCmd));

	/* 因716厂的通信控制板软件沿用了Z018海防项目与50所之间的板间参数交互协议，
	故业务接口板发送给716通信控制板的消息源地址需填充50所炮防通信控制板的地址0x10 */
	Regmsg.header.baowen_type = 0xA0; //0xA0;协议上写0x0A
	Regmsg.header.dst_addr = BAOWEN_ADDR_TYPE_716_BOARD;
	Regmsg.header.src_addr = BAOWEN_ADDR_TYPE_50_BOARD;
	Regmsg.header.info_type = BAOWEN_MSG_TYPE_CMD;
	Regmsg.header.data_len = sizeof(ZxCmd);

	len = sizeof(ST_HF_MGW_DCU_PRO_HEADER) + Regmsg.header.data_len;	
	Board_Mng_SendTo_716((uint8 *)&Regmsg, len);

	return 0;
}

/*usr num configuration*/
int usrnum_cb(unsigned char *buf, int len) {
	int cnt, i; 
	uint8 *pUsrNum;
	uint16 secNum;
	uint8 channel;

	cnt = buf[0];
	dump_buf(__func__, buf, 1 + 7*cnt);
	DBG("try configure %d users\n",cnt);
	
	for(i = 0; i < cnt; i++){
		channel = buf[1+i*7];
		pUsrNum =&buf[2+i*7];
		secNum = htons(*(uint16 *)(&buf[1+1+i*7+4]));
		Board_Mng_Set_User_Num(pUsrNum,secNum, channel);
	}

	return 0;
}	


/**
834 网管参数解析和分发
*/


int pfzsys_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int xwvsip_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int gidpar_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int madlst_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int tiiptb_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int pf2ipl_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int subnrt_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int fwrttb_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int ridlst_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int pfrtbl_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int pfiprt_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int pfmrtb_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int pfopti_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int voisys_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int tirnum_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int rapnum_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int reglst_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int spelst_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int conlst_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int dsllst_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int kplist_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int sslist_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int pfradr_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int vhftdm_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int uhftdm_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int pfvsph_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int pfvsip_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int pfhfpr_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int xyrlst_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int Board_Mng_Meet_Cfg(uint8 *pConfNum, uint8 *pMebs, int mebsCnt)
{
	MNG_ZW_CONF_CFG_PKT ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.header.InfoType = ZW_INFO_TYPE_CONF_CFG;
	ZxCmd.header.CmdLen = 9 + 4*mebsCnt;
	ZxCmd.CmdId = ZW_MEET_NUM_SPEC;	
	ZxCmd.OpMode = ZW_USR_REG;
	ZxCmd.MsgLen = 6 + mebsCnt * 4;
	
	memcpy(ZxCmd.confNum,pConfNum,2);
	ZxCmd.mebsCnt= mebsCnt;
	memset(ZxCmd.mebs,0xFF,4*32);
	memcpy(ZxCmd.mebs, pMebs, 4*mebsCnt);


	//DBG("\r\n>> %s, len=%d\n", __func__, sizeof(ZxCmd.header)+ZxCmd.header.CmdLen);
	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd.header)+ZxCmd.header.CmdLen);
}

int meetpa_cb(unsigned char *buf, int len) {
	int cnt, i;
	int confMebCnt;
	uint8 *pConfNum = NULL, *pPartMebs = NULL;

	cnt =buf[0];
	DBG("%s, cnt=%d\n", __func__, cnt);

	pConfNum = &buf[1];//会议号码
	for(i = 0; i < cnt; i++){
		confMebCnt = *(pConfNum+2);//成员个数
		pPartMebs = (pConfNum+3);//成员号码
		dump_buf(__func__, pConfNum, 3 + confMebCnt * 4);
		Board_Mng_Meet_Cfg(pConfNum, pPartMebs, confMebCnt);
		pConfNum += 3+4*confMebCnt;
	}


	return 0;
}

int Board_Mng_ZX_Cfg(uint8 type, uint8 phoneId, uint8 chanId, uint8 *pArmyId,uint8 *pCalleeNum)
{
	MNG_ZW_ZX_CFG_PKT ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.header.InfoType = ZW_INFO_TYPE_ZXRX_CFG;
	ZxCmd.header.CmdLen = 16;
	ZxCmd.CmdId = ZW_ZX_NUM_SPEC;	
	ZxCmd.OpMode = ZW_USR_REG;
	ZxCmd.MsgLen = 0x0d;

	ZxCmd.type = type;
	ZxCmd.phoneId = phoneId;
	ZxCmd.chanId = chanId;
	ZxCmd.Ack = 1;
	memcpy(ZxCmd.armyID, pArmyId,2);
	memcpy(ZxCmd.calleeNum, pCalleeNum,4);

	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));
}

/*ZhuanXian Cfg*/
int linepa_cb(unsigned char *buf, int len) {
	uint8 *pZxCfg = NULL;
	uint8  zxType, phoneId,chanId,*pCalleeNum, *pArmyId;
	int cnt, i= 0;

	cnt = buf[0];
	dump_buf(__func__, buf, 1 + cnt * 9);
	
	for(i = 0; i < cnt; i++){
		pZxCfg = &(buf[1+ 9*i]);
		zxType = *pZxCfg;
		phoneId = *(pZxCfg+1);
		chanId = *(pZxCfg +2 );
		pArmyId = pZxCfg +3;
		pCalleeNum = pZxCfg + 5;
		Board_Mng_ZX_Cfg(zxType,phoneId,chanId, pArmyId, pCalleeNum);
	}

	return 0;
}

int tdhjys_cb(unsigned char *buf, int len) {
	dump_buf(__func__, buf, 0);

	return 0;
}

int Board_Mng_RadioIntf_Cfg(uint8 port, uint8 portType, uint8 workMod)
{
	MNG_ZW_RADIO_INTF_CFG_PKT ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.header.InfoType = ZW_INFO_TYPE_RADIO_INTF_CFG;
	ZxCmd.header.CmdLen = 3;
	ZxCmd.port = port;
	ZxCmd.portType = portType;
	ZxCmd.workMod = workMod;
	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));
	return 0;
}

/*configure wireless radio interface*/
int raintf_cb(unsigned char *buf, int len) {
	int cnt, i = 0;
	uint8 port, chanType, workMod, *pRadioCfg;


	cnt = buf[0];
	dump_buf(__func__, buf, 1 + cnt * 3);

	for(i = 0; i < cnt;  i++){
		pRadioCfg = &buf[1+i*3];
		port = *pRadioCfg;
		chanType = *(pRadioCfg+1);
		workMod = *(pRadioCfg +2);
		Board_Mng_RadioIntf_Cfg(port,chanType,workMod);
	}

	return 0;
}

int Board_Mng_IP_Cfg(uint32 ipaddr, uint8 mask)
{
	MNG_ZW_IP_CFG_PKT ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.header.InfoType = ZW_INFO_TYPE_IP_CFG;
	ZxCmd.header.CmdLen = 5;
	ZxCmd.ipaddr = htonl(ipaddr);
	ZxCmd.mask = mask;
	
	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));
	return 0;
}

int Board_Mng_IPAs_Cfg(uint32 asNum)
{
	MNG_ZW_IP_AS_CFG_PKT ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.header.InfoType = ZW_INFO_TYPE_IP_AS_CFG;
	ZxCmd.header.CmdLen = 4;
	ZxCmd.asNum = htonl(asNum);
	
	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));
	return 0;
}

int ippara_cb(unsigned char *buf, int len) {
	uint32 ipaddr, asNum;
	uint8 mask;

	dump_buf(__func__, buf, 9);
	ipaddr = ntohl(*(uint32 *)buf);
	mask = buf[4];
	asNum = ntohl(*(uint32 *)&buf[5]);

	Board_Mng_IP_Cfg(ipaddr, mask);
	Board_Mng_IPAs_Cfg(asNum);

	sleep(2);
	
	return 0;
}

int Board_Mng_IPIntf_Addr_Cfg(uint8 port, uint8 addrMod, uint32 ipaddr, uint8 mask)
{
	MNG_ZW_IPINTF_ADDR_CFG_PKT ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.header.InfoType = ZW_INFO_TYPE_IPINTF_ADDR_CFG;
	ZxCmd.header.CmdLen = 9;
	ZxCmd.ope = 1;	/**Add*/
	ZxCmd.boardType = 1;	/*ZW board*/
	ZxCmd.port = port;
	ZxCmd.addrType = addrMod;
	ZxCmd.ipaddr = htonl(ipaddr);
	ZxCmd.mask = mask;
	
	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));
	return 0;
}

int Board_Mng_IPIntf_Route_Cfg(uint8 port, uint8 routeP, uint8 groupEn)
{
	MNG_ZW_IPINTF_ROUTE_CFG_PKT ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.header.InfoType = ZW_INFO_TYPE_IPINTF_ROUTE_CFG;
	ZxCmd.header.CmdLen = 5;
	ZxCmd.ope = 1;	/**Add*/
	ZxCmd.boardType = 1;	/*ZW board*/
	ZxCmd.port = port;
	ZxCmd.routeP = routeP;
	ZxCmd.groupEn = groupEn;

	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));
	return 0;
}

int ipintf_cb(unsigned char *buf, int len) {
	int cnt, i;
	uint8 *pIpIntfCfg;
	uint8 port, portType, addrMod, mask, route, group, mpls, linkP;
	uint32 ipaddr, mtu;

	cnt = buf[0];
	dump_buf(__func__, buf, 1);

	for(i = 0; i < cnt; i++){
		pIpIntfCfg = &buf[1+ i * 16];
		dump_buf(__func__, pIpIntfCfg, 16);
		port = *(pIpIntfCfg);
		portType = *(pIpIntfCfg +1);
		addrMod = *(pIpIntfCfg +2 );
		ipaddr = ntohl(*(uint32 *)(pIpIntfCfg+3));
		mask  = *(pIpIntfCfg +7);
		route  = *(pIpIntfCfg+8);
		group = *(pIpIntfCfg+9);
		mpls   = *(pIpIntfCfg+10);
		linkP  = *(pIpIntfCfg+11);
		mtu 	 = ntohl(*(uint32 *)(pIpIntfCfg+12));

		Board_Mng_IPIntf_Addr_Cfg(port,addrMod,ipaddr,mask);
		Board_Mng_IPIntf_Route_Cfg(port,route,group);
		sleep(1);
		DBG("--sleep-- 1s \r\n");//wait for 716 process
	}

	DBG(">>>>>>>>>>>>%s<<<<<<<<<<<<<end \r\n", __func__);

	return 0;
}

int Board_Mng_Ffp_Cfg(uint8 port, uint16 helloT, uint16 holdT, uint8 qltd, uint8 spfg)
{
	MNG_ZW_FRP_CFG_PKT ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.header.InfoType = ZW_INFO_TYPE_FRP_CFG;
	ZxCmd.header.CmdLen = 8;
	ZxCmd.boardType = 1;	/*ZW board*/
	ZxCmd.port = port;
	ZxCmd.helloT = htons(helloT);
	ZxCmd.holdT = htons(holdT);
	ZxCmd.qltd = qltd;
	ZxCmd.spfg = spfg;

	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));
	return 0;
}


int frppar_cb(unsigned char *buf, int len) {
	int cnt, i;
	uint8 *pFrpCfg, port, qltEn, spfgEn;
	uint16 helloT, holdT;


	cnt =  buf[0];
	dump_buf(__func__, buf, 1 + cnt * 8);

	for(i = 0; i < cnt; i++){
		pFrpCfg = & buf[1 + i* 8];
		port = *(pFrpCfg);
		helloT = ntohs(*(uint16 *)(pFrpCfg+2));
		holdT = ntohs(*(uint16 *)(pFrpCfg+4));
		qltEn = *(pFrpCfg+6);
		spfgEn = *(pFrpCfg +7);
		Board_Mng_Ffp_Cfg( port, helloT, holdT, qltEn, spfgEn);
	}
			
	return 0;
}

int Board_Mng_Tirp_Cfg(uint8 port, uint16 flashT, uint16 expiredT, uint16 delT, uint8 spfgEn, uint8 compress)
{
	MNG_ZW_TIRP_CFG_PKT ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.header.InfoType = ZW_INFO_TYPE_TIRP_CFG;
	ZxCmd.header.CmdLen = 10;
	ZxCmd.boardType = 1;	/*ZW board*/
	ZxCmd.port = port;
	ZxCmd.flashT = htons(flashT);
	ZxCmd.expiredT = htons(expiredT);
	ZxCmd.delT = htons(delT);
	ZxCmd.spfgEn = spfgEn;
	ZxCmd.compress = compress;

	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));

	return 0;
}

int tirppa_cb(unsigned char *buf, int len) {
	int cnt, i;
	uint8 *pTirpCfg, port, spfgEn, compress;
	uint32 flashT, expiredT, delT;


	cnt = buf[0];
	dump_buf(__func__, buf, 1 + cnt * 10);

	for(i = 0; i < cnt; i++){
		pTirpCfg = &buf[1+i* 10];
		port = *(pTirpCfg);
		flashT = ntohs(*(uint16 *)(pTirpCfg + 2));
		expiredT = ntohs(*(uint16 *)(pTirpCfg + 4));
		delT = ntohs(*(uint16 *)(pTirpCfg + 6));
		spfgEn = *(pTirpCfg +8);
		compress = *(pTirpCfg +9);
		Board_Mng_Tirp_Cfg(port, flashT, expiredT, delT, spfgEn, compress);		
	}
	return 0;
}

int Board_Mng_Lycff_Cfg(
	uint8 routeType, uint8 sRT, uint8 dRT, uint8 frpRT, uint8 ospfRT, uint8 ripRT, uint8 tirpRT, uint32 frpAs)
{
	MNG_ZW_LYCFF_CFG_PKT ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.header.InfoType = ZW_INFO_TYPE_LYCFF_CFG;
	ZxCmd.header.CmdLen = 11;
	ZxCmd.routeType = routeType;
	ZxCmd.staticRT_En = sRT;
	ZxCmd.directRT_EN = dRT;
	ZxCmd.frpRT_EN = frpRT;
	ZxCmd.ospfRT_EN = ospfRT;
	ZxCmd.ripRT_EN = ripRT;
	ZxCmd.tirpRT_EN = tirpRT;
	ZxCmd.frpAS = htonl(frpAs);

	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));
	usleep(200*1000);
	
	return 0;
}

int lycfbp_cb(unsigned char *buf, int len) 
{
	int cnt, i;
	uint8 *pbuf;

	cnt = buf[0];
	pbuf = &buf[1];
	dump_buf(__func__, buf, 1);

	for (i = 0; i < cnt; i++) {
		dump_buf(__func__, pbuf, 11);
		Board_Mng_Lycff_Cfg(pbuf[0],pbuf[1],pbuf[2],pbuf[3],pbuf[4],pbuf[5],pbuf[6], ntohl(*(uint32 *)(pbuf+7)));
		pbuf += 11;
	}

	return 0;
}

int Board_Mng_Lyjh_Cfg(uint8 port, uint32 net, uint8 mask, uint8 routeP, uint8 enFlg)
{
	MNG_ZW_LYJH_CFG_PKT ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.header.InfoType = ZW_INFO_TYPE_LYJH_CFG;
	ZxCmd.header.CmdLen = 8;

	ZxCmd.port = port;
	ZxCmd.net = htonl(net);
	ZxCmd.mask = mask;
	ZxCmd.routeP = routeP;
	ZxCmd.ope = enFlg;
	

	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));
	return 0;
}

int lyjhpa_cb(unsigned char *buf, int len) {
	int cnt, i;
	uint8 *pCfg, port, routeP, enFlg, mask;
	uint32 net;

	cnt = buf[0];
	dump_buf(__func__, buf, 1 + cnt * 8);

	for(i = 0; i < cnt; i++){
		pCfg = &buf[1+ 8 * i];
		port = *(pCfg);
		net = ntohl(*(uint32 *)(pCfg+1));
		mask = *(pCfg +5);
		routeP = *(pCfg +6);
		enFlg = *(pCfg +7);
		Board_Mng_Lyjh_Cfg(port,net,mask,routeP,enFlg);
	}
	
	return 0;
}

int Board_Mng_StRt_Cfg(uint32 dstAddr, uint8 mask, uint32 nextHpAddr)
{
	MNG_ZW_STATIC_RT_CFG_PKT ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	DBG("dstAddr=0x%x, mask=0x%x, nextHpAddr=0x%x\n", dstAddr, mask, nextHpAddr);
	ZxCmd.header.InfoType = ZW_INFO_TYPE_ST_RT_CFG;
	ZxCmd.header.CmdLen = 12;
	ZxCmd.ope = 1;		/*1,add; 2, del; 3, delAll*/
	ZxCmd.dstAddr = htonl(dstAddr);
	ZxCmd.mask = mask;
	ZxCmd.nextHpAddr = htonl(nextHpAddr);
	ZxCmd.metric = 1;

	sendMsgTo716Board(&ZxCmd,sizeof(ZxCmd));
	return 0;
}

int jtlypa_cb(unsigned char *buf, int len) {
	int cnt, i;
	uint8 *pCfg, mask;
	uint32 dstAddr, nextHopAddr;

	cnt = buf[0];
	dump_buf(__func__, buf, 1 + cnt * 9);

	for(i = 0; i < cnt; i++){
		pCfg = &buf[1+i*9];
		dstAddr = ntohl(*(uint32 *)(pCfg));
		mask = *(pCfg +4);
		nextHopAddr = ntohl(*(uint32 *)(pCfg +5));
		Board_Mng_StRt_Cfg(dstAddr,mask,nextHopAddr);
	}
		
	return 0;
}


int Board_Mng_fibrSta_Cfg(uint8 lSta, uint8 mSta)
{
	uint8 msg[2];
	msg[0] = lSta;
	msg[1] = mSta;
	
	sendMsgTo716Ray(ZW_INFO_TYPE_FIBR_STA_CFG, &msg,sizeof(msg));
	return 0;
}

int Board_Mng_fibrIntf_Cfg(uint8 speed, uint8 mode)
{
	uint8 msg[2];
	msg[0] = speed;
	msg[1] = mode;

	sendMsgTo716Ray(ZW_INFO_TYPE_FIBR_INTF_CFG, &msg,sizeof(msg));
	return 0;
}

int Board_Mng_fibrYw_Cfg(
	uint8 ywld, uint8 slot, uint8 mod, uint8 sta1, uint8 port1, uint8 chan1, uint8 sta2, uint8 port2, uint8 chan2
)
{
	RAY_FIBR_YW_CFG_PKT ZxCmd;
	memset(&ZxCmd, 0x0, sizeof(ZxCmd));

	ZxCmd.ope = 1; 	/*1, add; 2 del*/
	ZxCmd.cnt = 1; 	/*only support one*/
	ZxCmd.ywld = ywld;
	ZxCmd.slot = slot;
	ZxCmd.ywMod = mod;
	ZxCmd.sta1 = sta1;
	ZxCmd.port1 = port1;
	ZxCmd.chan1 = chan1;
	ZxCmd.sta2 =sta2;
	ZxCmd.port2 = port2;
	ZxCmd.chan2 = chan2;
	ZxCmd.eof = 0xFF;

	sendMsgTo716Ray(ZW_INFO_TYPE_FIBR_YW_CFG, &ZxCmd,sizeof(ZxCmd));

	return 0;
}


int fibrpa_cb(unsigned char *buf, int len) {
	int cnt,i;
	uint8 lStaID, mStaID,speed, mode, simSpeed;
	uint8 *pCfg, ywld, slot,ywMod,sta1,port1, chan1, sta2,port2,chan2;	/*one qunlu yewu*/

	lStaID = buf[0];
	mStaID = buf[1];
	speed = buf[2];
	mode = buf[3];
	simSpeed = buf[4];

	dump_buf(__func__, buf, 6);
	Board_Mng_fibrSta_Cfg(lStaID,mStaID);
	Board_Mng_fibrIntf_Cfg(speed,mode);

	cnt = buf[5];		//cnt only be supported to 1
	for( i = 0; i < cnt; i++){
		pCfg = &buf[6 + i *10];
		dump_buf(__func__, pCfg, 10);	
		ywld =  *(pCfg);
		slot = *(pCfg +1);
		ywMod = *(pCfg +2);
		sta1 = *(pCfg +3);
		port1 = *(pCfg +4);
		chan1 = *(pCfg +5);
		sta2 = *(pCfg +6);
		port2 = *(pCfg +7);
		chan2 = *(pCfg +8);
		Board_Mng_fibrYw_Cfg(ywld,slot,ywMod,sta1,port1,chan1,sta2,port2,chan2);
	}
	
	return 0;
}

int DisplayBoardCLear()
{
	ST_DISPLAY_MNG_MSG msg;
	uint16 x, y;

	DBG("%s \r\n", __func__);
	msg.header = 0xfe;
	msg.total_len = 14;
	msg.cmd = 0x00;
	msg.type = 0;
	msg.data_len = 8;
	msg.body[0] = 0x00;
	msg.body[1] = 0x00;
	msg.body[2] = 0;
	msg.body[3] = 0;

	x = htons(319);
	y = htons(240);
	memcpy(&msg.body[4], &x, 2);
	memcpy(&msg.body[6], &y, 2);		

	Board_Mng_SendTo_Display(&msg, msg.total_len);

	return 0;
}


int DisplayBoardShowOK()
{
	ST_DISPLAY_MNG_MSG msg;

	DBG("%s \r\n", __func__);
	msg.header = 0xfe;
	msg.total_len = 28;
	msg.cmd = 0x03;
	msg.type = 0;
	msg.data_len = 22;
	msg.body[0] = 0x00;
	msg.body[1] = 100;
	msg.body[2] = 0;
	msg.body[3] = 110;
	
	msg.body[4] = 215; //注
	msg.body[5] = 162;
	msg.body[6] = 178; //参
	msg.body[7] = 206;
	msg.body[8] = 205; //完
	msg.body[9] = 234;	
	msg.body[10] = 179; //成
	msg.body[11] = 201;	
	msg.body[12] = 163; //!
	msg.body[13] = 161;
	msg.body[14] = 199; //请
	msg.body[15] = 235;
	msg.body[16] = 214; //重
	msg.body[17] = 216;
	msg.body[18] = 198; //启
	msg.body[19] = 244;		
	msg.body[20] = 163; //!
	msg.body[21] = 161;

	Board_Mng_SendTo_Display(&msg, msg.total_len);

	return 0;
}



