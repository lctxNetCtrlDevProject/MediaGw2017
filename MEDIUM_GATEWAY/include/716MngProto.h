#ifndef __716_MNG_PROTO_H__
#define __716_MNG_PROTO_H__

typedef enum{
	ZW_INFO_TYPE_USR_CFG = 0x40,		/*user configuration*/
	ZW_INFO_TYPE_CONF_CFG= 0x41, 	/*conference configuration*/
	ZW_INFO_TYPE_ZXRX_CFG= 0x42,		/*zhuanXian, ReXian Configuration*/

	ZW_INFO_TYPE_IPINTF_ADDR_CFG = 0x80,
	ZW_INFO_TYPE_IPINTF_ROUTE_CFG = 0x81,
	ZW_INFO_TYPE_IP_AS_CFG = 0x87,
	ZW_INFO_TYPE_IP_CFG = 0x88,

	ZW_INFO_TYPE_DEVID_CFG = 0x91,
	ZW_INFO_TYPE_ARMYID_CFG = 0x92,
	ZW_INFO_TYPE_PHONENUM_LEN_CFG = 0x93,
	ZW_INFO_TYPE_SPEECH_ENCODE_CFG = 0x94,

	ZW_INFO_TYPE_INTF_TYPE_CFG = 0x96,
	ZW_INFO_TYPE_INTF_SPEED_CFG = 0x98,
	ZW_INFO_TYPE_INTF_MODE_CFG = 0x9a,
	ZW_INFO_TYPE_INTF_CLK_CFG = 0x9f,
	ZW_INFO_TYPE_INTF_LOOP_CFG = 0xa5,
	ZW_INFO_TYPE_INTF_MASTER_CFG = 0xb4,

	ZW_INFO_TYPE_ST_RT_CFG = 0xa7,
	ZW_INFO_TYPE_LYCFF_CFG = 0xaa,
	ZW_INFO_TYPE_FRP_CFG = 0xb0,
	ZW_INFO_TYPE_TIRP_CFG = 0x2a,
	ZW_INFO_TYPE_LYJH_CFG = 0xbe,
	ZW_INFO_TYPE_RADIO_INTF_CFG = 0xb5,	/*radio intf cfg*/

	ZW_INFO_TYPE_FIBR_STA_CFG  = 0xa1,
	ZW_INFO_TYPE_FIBR_INTF_CFG = 0xa3,
	ZW_INFO_TYPE_FIBR_YW_CFG = 0xab,
	
	MSG_716_MODE_GET_MSG = 0x8d,	
	MSG_716_ARMY_ID_GET_MSG = 0x8e,
	MSG_716_USR_NUM_GET_MSG = 0x40,
}ZwMngInfoType;

typedef enum{
	ZW_INFO_TYPE_USR_CFG_ACK = 0x40,		/*user configuration*/
	ZW_INFO_TYPE_CONF_CFG_ACK = 0x41, 	/*conference configuration*/
	ZW_INFO_TYPE_ZXRX_CFG_ACK = 0x42,		/*zhuanXian, ReXian Configuration*/

	ZW_INFO_TYPE_IPINTF_ADDR_CFG_ACK = 0xc0,
	ZW_INFO_TYPE_IPINTF_ROUTE_CFG_ACK = 0xc1,
	ZW_INFO_TYPE_IP_AS_CFG_ACK = 0xc7,
	ZW_INFO_TYPE_IP_CFG_ACK = 0xc8,

	ZW_INFO_TYPE_DEVID_CFG_ACK = 0xd1,
	ZW_INFO_TYPE_ARMYID_CFG_ACK = 0xd2,
	ZW_INFO_TYPE_PHONENUM_LEN_CFG_ACK = 0xd3,
	ZW_INFO_TYPE_SPEECH_ENCODE_CFG_ACK = 0xd4,
	ZW_INFO_TYPE_INTF_TYPE_CFG_ACK = 0xd6,
	ZW_INFO_TYPE_INTF_SPEED_CFG_ACK = 0xd8,	
	ZW_INFO_TYPE_INTF_MODE_CFG_ACK = 0xda,
	ZW_INFO_TYPE_INTF_CLK_CFG_ACK = 0xdf,
	ZW_INFO_TYPE_INTF_LOOP_CFG_ACK = 0xe5,
	ZW_INFO_TYPE_INTF_MASTER_CFG_ACK = 0xf4,

	ZW_INFO_TYPE_ST_RT_CFG_ACK = 0xe7,
	ZW_INFO_TYPE_LYCFF_CFG_ACK = 0xea,
	ZW_INFO_TYPE_FRP_CFG_ACK = 0xf0,
	ZW_INFO_TYPE_TIRP_CFG_ACK = 0x2b,
	ZW_INFO_TYPE_LYJH_CFG_ACK = 0xfe,
	ZW_INFO_TYPE_RADIO_INTF_CFG_ACK = 0xf5,	/*radio intf cfg*/

	ZW_INFO_TYPE_FIBR_STA_CFG_ACK  = 0xa1,
	ZW_INFO_TYPE_FIBR_INTF_CFG_ACK = 0xa3,
	ZW_INFO_TYPE_FIBR_YW_CFG_ACK = 0xab,

	MSG_716_MODE_GET_MSG_ACK = 0xcd,
	MSG_716_ARMY_ID_GET_MSG_ACK = 0xce, 
	MSG_716_USR_NUM_GET_MSG_ACK = 0x40,

}ZwMngReplyId;


typedef enum{
	ZW_USR_NUM_SPEC  = 0x01,
	ZW_USR_CHN_SPEC 	= 0x02,
	ZW_USR_ALL_NUM 	= 0x03,

	ZW_ZX_NUM_SPEC = 0x08,
	ZX_ZX_ALL_NUM = 0x09,

	ZW_MEET_NUM_SPEC = 0x10,
	ZW_MEET_ALL_NUM	= 0x11,
	ZW_MEET_MEB_SPEC = 0x12,
	
}ZwUsrType;

typedef enum{
	ZW_USR_REG = 0x01,
	ZW_USR_QUERY= 0x02,
	ZW_USR_DEL,
	ZW_USR_REQ_ACK,
	ZW_USR_QUERY_ACK,
	ZW_USR_DEL_ACK,
	ZW_USR_REP_REG = 0x07,	/*report reg*/
	ZW_USR_REP_DEL,			/*report del*/
}ZwUsrMngOpeType;

#pragma pack(1)

typedef struct __Mng_Zw_Interface_Dev_Id_Msg__
{
	uint8 InfoType;
	uint16 CmdLen;
	uint8 DevType;
	uint16 DevId;
}__attribute__((packed))MNG_ZW_DEV_ID_MSG;

typedef struct __Mng_Zw_Interface_Army_Id_Msg__
{
	uint8 InfoType;
	uint16 CmdLen;
	uint16 ArmyId;
}__attribute__((packed))MNG_ZW_ARMY_ID_MSG;

typedef struct __Mng_Zw_Interface_Num_Len_Msg__
{
	uint8 InfoType;
	uint16 CmdLen;
	uint8 NumLen;
}__attribute__((packed))MNG_ZW_NUM_LEN_MSG;

typedef struct __Mng_Zw_Interface_Speech_Encode_Msg__
{
	uint8 InfoType;
	uint16 CmdLen;
	uint8 Mode;
}__attribute__((packed))MNG_ZW_SPEECH_ENCODE_MSG;

typedef struct __Mng_Zw_Interface_Type_Msg__
{
	uint8 InfoType;
	uint16 CmdLen;
	uint8 PortId;
	uint8 Type;
}__attribute__((packed))MNG_ZW_INF_TYPE_MSG;


typedef struct __Mng_Zw_Interface_Speed_Msg__
{
	uint8 InfoType;
	uint16 CmdLen;
	uint8 PortId;
	uint8 PortSpeed;
}__attribute__((packed))MNG_ZW_INF_SPEED_MSG;


typedef struct __Mng_Zw_Interface_Mode_Msg__
{
	uint8 InfoType;
	uint16 CmdLen;
	uint8 PortId;
	uint8 Mode;
}__attribute__((packed))MNG_ZW_INF_Mode_MSG;


typedef struct __Mng_Zw_Interface_Clk_Msg__
{
	uint8 InfoType;
	uint16 CmdLen;
	uint8 PortId;
	uint8 PortClk;
}__attribute__((packed))MNG_ZW_INF_Clk_MSG;

typedef struct __Mng_Zw_Interface_SM_Msg__
{
	uint8 InfoType;
	uint16 CmdLen;
	uint8 PortId;
	uint8 Master;
}__attribute__((packed))MNG_ZW_INF_SM_MSG;

typedef struct __Mng_Zw_Interface_Huanhui_Msg__
{
	uint8 InfoType;
	uint16 CmdLen;
	uint8 PortId;
	uint8 Huanhui;
}__attribute__((packed))MNG_ZW_INF_HH_MSG;


typedef struct{
	uint8 InfoType;
	uint16 CmdLen;
}__attribute__((packed))ZwMngHeader;

typedef struct __Mng_Zw_Set_UserNum__
{
	ZwMngHeader header;
	uint8 CmdId;
	uint8 OpMode;
	uint8 MsgLen;
	uint8 Ack;
	uint16 Index;
	uint8  UserNum[4];
	uint16 SecurityNum;
	uint8 PhoneId;
	uint8 ChannelId;
	uint32 LiZhuanId;
	uint32 MangZhuanId;
	uint32 NoAckZhuanId;
	uint8 ServiceGroupId;
	uint8 TerminalMode;
	uint8 NetMode;
	uint16 GroupOutCode;
	uint16 GroupInCode;
}__attribute__((packed))MNG_ZW_USR_CFG_PKT;

typedef struct
{
	ZwMngHeader header;
	uint8 CmdId;
	uint8 OpMode;
	uint8 MsgLen;
	uint8 Ack;
	uint16 Index;
	uint8  confNum[2];
	uint8  mebsCnt;
	uint8  mebs[4*32];

}MNG_ZW_CONF_CFG_PKT;

typedef struct
{
	ZwMngHeader header;
	uint8 CmdId;
	uint8 OpMode;
	uint8 MsgLen;
	uint8 type;
	uint16 Index;
	uint8 Ack;
	uint8 phoneId;
	uint8 chanId;
	uint8 armyID[2];
	uint8 calleeNum[4];
	uint8 status;

}MNG_ZW_ZX_CFG_PKT;

typedef struct
{
	ZwMngHeader header;
	uint8 port;
	uint8 portType;
	uint8 workMod;
}MNG_ZW_RADIO_INTF_CFG_PKT;

typedef struct
{
	ZwMngHeader header;
	uint32 asNum;
}MNG_ZW_IP_AS_CFG_PKT;

typedef struct
{
	ZwMngHeader header;
	uint32 ipaddr;
	uint8 mask;
}MNG_ZW_IP_CFG_PKT;


typedef struct
{
	ZwMngHeader header;
	uint8 ope;			/*1,add; 2, del*/
	uint8 boardType;		/*default 1*/
	uint8 port;
	uint8 addrType;
	uint32 ipaddr;
	uint8 mask;
}MNG_ZW_IPINTF_ADDR_CFG_PKT;

typedef struct
{
	ZwMngHeader header;
	uint8 ope;			/*1,add; 2, del*/
	uint8 boardType;		/*default 1*/
	uint8 port;
	uint8 routeP;
	uint8 groupEn;
}MNG_ZW_IPINTF_ROUTE_CFG_PKT;


typedef struct
{
	ZwMngHeader header;
	uint8 boardType;		/*default 1*/
	uint8 port;
	uint16 helloT;
	uint16 holdT;
	uint8 qltd;
	uint8 spfg;
}MNG_ZW_FRP_CFG_PKT;

typedef struct
{
	ZwMngHeader header;
	uint8 boardType;		/*default 1*/
	uint8 port;
	uint16 flashT;
	uint16 expiredT;
	uint16 delT;
	uint8 spfgEn;
	uint8 compress;
}MNG_ZW_TIRP_CFG_PKT;

typedef struct
{
	ZwMngHeader header;
	uint8 routeType;
	uint8 staticRT_En;
	uint8 directRT_EN;
	uint8 frpRT_EN;
	uint8 ospfRT_EN;
	uint8 ripRT_EN;
	uint8 tirpRT_EN;
	uint32 frpAS;
}MNG_ZW_LYCFF_CFG_PKT;

typedef struct
{
	ZwMngHeader header;
	uint8 port;
	uint32 net;
	uint8 mask;
	uint8 routeP;
	uint8 ope;
}MNG_ZW_LYJH_CFG_PKT;


typedef struct
{
	ZwMngHeader header;
	uint8 ope;
	uint32 dstAddr;
	uint8 mask;
	uint32 nextHpAddr;
	uint8 outPort;
	uint8 metric;
}MNG_ZW_STATIC_RT_CFG_PKT;

typedef struct
{
	ZwMngHeader header;
	uint8 lSta;
	uint8 mSta;
}MNG_ZW_FIBR_STA_CFG_PKT;

typedef struct
{
	ZwMngHeader header;
	uint8 speed;
	uint8 mode;
}MNG_ZW_FIBR_INTF_CFG_PKT;


typedef struct
{
	uint8 ope;
	uint8 cnt;
	uint8 ywld;
	uint8 slot;
	uint8 ywMod;
	uint8 sta1;
	uint8 port1;
	uint8 chan1;
	uint8 sta2;
	uint8 port2;
	uint8 chan2;
	uint8 eof;
}RAY_FIBR_YW_CFG_PKT;

typedef struct __Mng_Zw_Interface_Dev_Id_Get_Msg__
{
	uint8 InfoType;
	uint16 CmdLen;
	uint16 CmdData;
}__attribute__((packed))MNG_ZW_DEV_ID_GET_MSG;

typedef struct __Mng_Zw_Interface_Army_Id_Get_Msg__
{
	uint8 InfoType;
	uint16 CmdLen;
	uint16 CmdData;
}__attribute__((packed))MNG_ZW_ARMY_ID_GET_MSG;



#pragma pack(0)
#endif;
