/****************************************************************************************************
* Copyright @,  2010-2015,  LCTX Co., Ltd. 
* Filename:     zjct_node.h  
* Version:      1.01
* Date:         2013.6.18	    
* Description:  
* Modification History: 

*****************************************************************************************************/

#ifndef __ZJCT_H__
#define __ZJCT_H__

#if defined(__cplusplus) 
extern "C" 
{ 
#endif

enum __MSG_TYPE__
{
	ZJCT_MNG_MSG = 0x00,
	ZJCT_PARAM_MSG = 0x01,
	ZJCT_SEM_MSG = 0x02,
	ZJCT_VOC_MSG = 0x03,
	ZJCT_DATA_MSG = 0x10,
};


enum __MNG_MSG_TYPE__
{
	ZJCT_MNG_LINK_MSG = 0x01,
	ZJCT_MNG_LINK_ACK_MSG = 0x02,	
	
};
enum __PARAM_MSG__
{
	ZJCT_OPEN_MSG = 0x01,
	ZJCT_USER_INJECT_MSG = 0x02,
	ZJCT_USER_INJECT_MSG_ACK = 0x03,
#if 0	
	ZJCT_USER_NUM_REG = 0x04,
	ZJCT_USER_NUM_REG_ACK = 0x05,
	ZJCT_USER_NUM_UNREG = 0x06,
	ZJCT_USER_NUM_UNREG_ACK = 0x07,
#else
	ZJCT_ZHENRU_FENJI_REG = 0x05,
	ZJCT_ZHENRU_FENJI_REG_ACK = 0x06,
#endif

	ZJCT_CLOSE_MSG =0x10,
	ZJCT_CLOSE_MSG_ACK = 0x11,
	
	ZJCT_PARAM_BUFF,
};


extern int32 Sw_Phone_init(void);
extern int32 Sw_Phone_Data_Send_by_IP(uint8 *buf, int32 len, uint32 ipaddr);
extern int32 Sw_Phone_Voc_Send_by_IP(uint8 *buf, int32 len, uint32 ipaddr);
extern int32 Check_834_Sw_Phone_Timeout(const void *data);
extern void Sw_Phone_Voc_RxThread(void);
extern void Sw_Phone_Data_RxThread(void);
extern void Sw_Phone_Mng_RxThread(void);
extern int32 Sw_Phone_Mng_Send_by_IP(uint8 *buf, int32 len, uint32 ipaddr);
extern int32 Ac491_Voc_Process(uint8 *buf, int32 len, uint8 id);
extern int32 Sw_Phone_User_Info_Inject(uint8 port_id, uint32 ipaddr);
extern int32 Sw_Phone_Socket_Close(void);



#if defined(__cplusplus) 
} 
#endif 

#endif













