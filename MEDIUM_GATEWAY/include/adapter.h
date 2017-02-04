/****************************************************************************************************
* Copyright @,  2010-2015,  LCTX Co., Ltd. 
* Filename:     zjct_node.h  
* Version:      1.01
* Date:         2013.6.18	    
* Description:  
* Modification History: 

*****************************************************************************************************/

#ifndef __ADAPTER_H__
#define __ADAPTER_H__

#if defined(__cplusplus) 
extern "C" 
{ 
#endif


typedef struct _CONF_PARAM_
{
	char		conf_name[80];
	char		conf_user_name[80];
	uint8 	conf_num_len;
	uint8 	conf_cnt;
	uint8 	talk_flg;
	char 	conf_phone_flg[KUOZHAN_CONF_MEM_MAX];
	char 	conf_phone[KUOZHAN_CONF_MEM_MAX][80];	
	uint8	mem_save[KUOZHAN_CONF_MEM_MAX];			/* voice save */
	uint8	mem_get[KUOZHAN_CONF_MEM_MAX];	
	uint8 	mem_voc[KUOZHAN_CONF_MEM_MAX][VOICE_TIME_1min][ZJCT_VOC_PCM_20MS_LEN_320];
}__attribute__((packed))CONF_PARAM;

extern CONF_PARAM kuozhan_conf[KUOZHAN_CONF_NUM_MAX];

extern int32 kuozhan_terminal_call_control(const void *data);
extern int32 kuozhan_init_terminal(const void *data);
extern int32 Check_834_KuoZhan_XiWei_Timeout(const void *data);
extern void XiWei_Data_RxThread(void);
extern void XiWei_Voc_RxThread(void);
extern int32 Check_834_AdpPhone_Timeout(const void *data);
extern void Adp_Phone_Voc_RxThread(void);
extern void Adp_Phone_Data_RxThread(void);
extern int32 kuozhan_socket_init(const void *data);
extern int32 kuozhan_socket_Close(void);
extern void Seat_Data_RxThread(void);
extern int32 Kuozhan_Ac491_Voc_Process(uint8 *buf, int32 len, uint8 port);
extern int32 Kuozhan_Ac491_Conf_Voc_Process(const void *data);
extern int32 Adp_Phone_Zhenru_Fenji_Inject_all(void);


#if defined(__cplusplus) 
} 
#endif 

#endif













