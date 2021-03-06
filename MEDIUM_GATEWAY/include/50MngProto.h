#ifndef __50_MNG_PROTO_H__
#define __50_MNG_PROTO_H__

typedef enum {
	PF_INFO_TYPE_HANDSHAKE_CFG = 0x5010,
	PF_INFO_TYPE_NODE_ADDR_CFG = 0x1001,
	PF_INFO_TYPE_VHF_INF_TYPE_CFG = 0x1020,
} PfMngInfoType;

typedef enum {
#if 0
	PF_INFO_TYPE_HANDSHAKE_CFG_ACK = 0x5020,
	PF_INFO_TYPE_NODE_ADDR_CFG_ACK = 0x1101,
	PF_INFO_TYPE_VHF_INF_TYPE_CFG_ACK = 0x1120,
#endif
	PF_INFO_TYPE_HANDSHAKE_CFG_ACK = 0xa000,
	PF_INFO_TYPE_NODE_ADDR_CFG_ACK = 0xa81d,
	PF_INFO_TYPE_PHONENUM_LEN_CFG_ACK = 0xb425,
	PF_INFO_TYPE_PHONEBOOK_NUM_DEL_ACK1 = 0xb426,
	PF_INFO_TYPE_PHONEBOOK_NUM_DEL_ACK2 = 0x00e1,
	PF_INFO_TYPE_PHONEBOOK_NUM_ADD_ACK1 = 0xb426,
	PF_INFO_TYPE_PHONEBOOK_NUM_ADD_ACK2 = 0xb826,
	PF_INFO_TYPE_PHONE_LOCAL_NUM_ADD_ACK = 0xa421,	
	PF_INFO_TYPE_PHONE_LOCAL_NUM_DEL_ACK1 = 0xa021,	
	PF_INFO_TYPE_PHONE_LOCAL_NUM_DEL_ACK2 = 0x14e1,	
	PF_INFO_TYPE_CONF_ADD_ACK1 = 0xbc23,	
	PF_INFO_TYPE_CONF_ADD_ACK2 = 0xa023,
	PF_INFO_TYPE_WIRE_QUNLU_CFG_ACK = 0xbca1,
	PF_INFO_TYPE_IP_QUNLU_CFG_ACK1 = 0xad1f,
	PF_INFO_TYPE_IP_QUNLU_CFG_ACK2 = 0xad0e,
	
	PF_INFO_TYPE_VHF_TYPE_CFG_ACK = 0xa861,
	PF_INFO_TYPE_VHF_SLOT_CFG_ACK = 0xa867,
	PF_INFO_TYPE_VHF_CLK_CFG_ACK = 0xa866,
	PF_INFO_TYPE_VHF_REMOTE_CTRL_CFG_ACK = 0xa871,
	PF_INFO_TYPE_VHF_SYNC_CFG_ACK = 0xa86a,

	PF_INFO_TYPE_HF_CFG_ACK = 0xa493,

} PfMngReplyId;

#pragma pack(1)

typedef struct __Mng_Pf_Handshake_Msg__
{
	uint8 PortAddr;
	uint16 InfoType;
	uint16 CmdLen;
}__attribute__((packed))MNG_Pf_Handshake_MSG;

typedef struct __Mng_Pf_Node_Addr_Msg__
{
	uint8 PortAddr;
	uint16 InfoType;
	uint16 CmdLen;
	uint32 NodeAddr;
}__attribute__((packed))MNG_Pf_Node_Addr_MSG;

typedef struct __Mng_Pf_Addr_Msg__
{
	uint8 InfoType[3];
	uint32 IPAddr;
	uint8 Mask;
}__attribute__((packed))MNG_Pf_Addr_MSG;

typedef struct __Mng_Pf_VHF_Inf_Type_Msg__
{
	uint8 PortAddr;
	uint16 InfoType;
	uint16 CmdLen;
	uint8 InfId;
	uint8 InfType;
}__attribute__((packed))MNG_Pf_VHF_Inf_Type_MSG;

typedef struct
{
	uint8 InfoType[3];
	uint32 IPAddr;
	uint8 Count;
	uint8 PhoneNum[8];
}__attribute__((packed))MNG_Pf_Set_Phone_Book_Num_MSG;

typedef struct
{
	uint8 InfoType[3];
	uint8 PhoneNum[8];
	uint8 Channel;
	uint8 PriorityConven;
}__attribute__((packed))MNG_Pf_Set_Local_Phone_Num_MSG;

#define CONF_50_MEMB_CNT_MAX 20
typedef struct
{
	uint8 InfoType[3];
	uint16 ConfNum;
	uint8 MembCnt;
	uint8 Membs[CONF_50_MEMB_CNT_MAX][8];
}__attribute__((packed))MNG_Pf_Add_Conf_MSG;



#pragma pack(0)
#endif;
