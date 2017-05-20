#ifndef __50_MNG_PROTO_H__
#define __50_MNG_PROTO_H__

typedef enum {
	PF_INFO_TYPE_HANDSHAKE_CFG = 0x5010,
	PF_INFO_TYPE_NODE_ADDR_CFG = 0x1001,
	PF_INFO_TYPE_VHF_INF_TYPE_CFG = 0x1020,
} PfMngInfoType;

typedef enum {
	PF_INFO_TYPE_HANDSHAKE_CFG_ACK = 0x5020,
	PF_INFO_TYPE_NODE_ADDR_CFG_ACK = 0x1101,
	PF_INFO_TYPE_VHF_INF_TYPE_CFG_ACK = 0x1120,
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




#pragma pack(0)
#endif;
