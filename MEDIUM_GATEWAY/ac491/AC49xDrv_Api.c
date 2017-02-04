/** 
 *  @file
 *  @brief This file contains the following API functions:
 *  @li @link DEBUG_AND_RECOVERY Debug and Recovery@endlink
 *  @li @link DEVICE_MEMORY_ACCESS Device Memory Access Tests@endlink
 *  @li @link PACKET_HANDLING Packet Handling@endlink
 *  @li @link DEVICE_CONFIGURATION Device Configuration@endlink
 *  @li @link CHANNEL_CONFIGURATION Channel Configuration@endlink
 *  @li @link CHANNEL_COMMANDS Channel Commands@endlink
 *  @li @link DEVICE_RESET_AND_DOWNLOAD Device Reset and Download@endlink
 *  @li @link CONVENIENT_GROUP Convenient Group@endlink (High-Level Initialization and Setup)
 */
 

/************************************************************************************/
/* Ac49xDrv_Api.c - AC49x Device Drivers / API Driver Functions						*/
/* Copyright (C) 2000 AudioCodes Ltd.												*/
/* Modifications :																	*/
/* 1/7/01 - Coding started.															*/
/************************************************************************************/
#include "PUBLIC.h"

#include "AC49xDrv_Drv.h"

/* This  driver is compiled only for the supported device types */
#if (  (AC49X_DEVICE_TYPE == AC490_DEVICE) || (AC49X_DEVICE_TYPE == AC491_DEVICE) || (AC49X_DEVICE_TYPE == AC491L_DEVICE) || (AC49X_DEVICE_TYPE == AC494_DEVICE) || (AC49X_DEVICE_TYPE == AC495_DEVICE) || (AC49X_DEVICE_TYPE == AC496_DEVICE) || (AC49X_DEVICE_TYPE == AC496E_DEVICE) || (AC49X_DEVICE_TYPE == AC496E_DEVICE) || (AC49X_DEVICE_TYPE == AC497_DEVICE)   )

Tac49xDriverVersionInfo ac49xDriverVersionInfo = 
    {  
/* ~Tac49xDriverVersionInfo Start~	  */
/*  char MandatoryVersionPrefix[32] = */ "580",  
/*  char SubVersion[32]             = */ "02", 
/*  char Date[11]                   = */ "06.10.2008",
/*  char sComments[300]             = */ ""
/* ~Tac49xDriverVersionInfo Stop~     */
    }; 	
                            
/*******************************************/
/*******************************************/
/**        Global Devices Arrays          **/
/*******************************************/
/*******************************************/
static Tac49xPacket						        Packet;
                                                
Tac49xDeviceControlRegister	                    ac49xDeviceControlRegister[                 AC49X_NUMBER_OF_DEVICES];
Tac49xBufferDescriptorCurrentIndex              ac49xHostBufferDescriptorCurrentIndex[      AC49X_NUMBER_OF_DEVICES];
Tac49xBufferDescriptorCurrentIndex              ac49xNetworkBufferDescriptorCurrentIndex[   AC49X_NUMBER_OF_DEVICES];
int			                                    ac49xRxHostPacketSequenceNumber   [         AC49X_NUMBER_OF_DEVICES];	
int		 	                                    ac49xTxHostPacketSequenceNumber   [         AC49X_NUMBER_OF_DEVICES];	
int			                                    ac49xRxNetworkPacketSequenceNumber[         AC49X_NUMBER_OF_DEVICES];	
int		 	                                    ac49xTxNetworkPacketSequenceNumber[         AC49X_NUMBER_OF_DEVICES];
U32									            ac49xHcrfCommandRegisterAddress[			AC49X_NUMBER_OF_DEVICES];
U32									            ac49xHcrfStatusRegisterAddress[				AC49X_NUMBER_OF_DEVICES];

#if FIFO_USAGE_ENABLE__HOST
Tac49x_DeclareFifo(FIFO_SIZE__HOST,     FIFO_CELL_SIZE__HOST,	    Host)	ac49xHostFifo[AC49X_NUMBER_OF_DEVICES];
#endif
#if FIFO_USAGE_ENABLE__NETWORK
Tac49x_DeclareFifo(FIFO_SIZE__NETWORK,  FIFO_CELL_SIZE__NETWORK,	Net)	ac49xNetFifo[AC49X_NUMBER_OF_DEVICES];
#endif

#if (AC49X_DEVICE_TYPE == AC490_DEVICE)
U8	acHcrfIntEntry[HCRF__INT_ENTRY_SIZE];
#elif (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
U8	acHcrfIntEntry[HCRF__INT_ENTRY_SIZE] = {0xea, 0x78, 0x00, 0x00, 0x5e, 0x80, 0x5f, 0x80};
#endif 

typedef enum
{
   CHANNEL_STATE_IDLE,
   CHANNEL_STATE_TDM_ONLY,
   CHANNEL_STATE_REGULAR_RTP,
   CHANNEL_STATE_MEDIATED_RTP,
   CHANNEL_STATE_CONFERENCE_MODE_1,
   CHANNEL_STATE_CONFERENCE_MODE_2,
   CHANNEL_STATE_CONFERENCE_MODE_3,
   CHANNEL_STATE_CONFERENCE_MODE_4,
   CHANNEL_STATE_CONFERENCE_MODE_1_MATE,
   CHANNEL_STATE_CONFERENCE_MODE_3_MATE,
   CHANNEL_STATE_UNKNOWN
} TChannelState;

int g_ac491_dev_fd = -1;
enum{
	LOOPBACK_OFF = 0,
	LOOPBACK_ON
};
static unsigned char flag_loopback = LOOPBACK_OFF;

Tac49xSetupDeviceAttr SetupDeviceAttr[NUMBER_OF_DEVICES_PER_DSP];
Tac49xSetupChannelAttr SetupChannelAttr[NUMBER_OF_DEVICES_PER_DSP][NUMBER_OF_CHANNELS];
TChannelState ac491ChannelState[NUMBER_OF_DEVICES_PER_DSP][NUMBER_OF_CHANNELS];

char DeviceRun[NUMBER_OF_DEVICES_PER_DSP];

U32	ac491RegularSuccessStat[NUMBER_OF_DEVICES_PER_DSP][NUMBER_OF_CHANNELS];
U32	ac491P2pSuccessStat[NUMBER_OF_DEVICES_PER_DSP][NUMBER_OF_CHANNELS];
U32	ac491ConfMd4SuccessStat[NUMBER_OF_DEVICES_PER_DSP][NUMBER_OF_CHANNELS]; 
U32	ac491RegularFalseStat[NUMBER_OF_DEVICES_PER_DSP][NUMBER_OF_CHANNELS];
U32	ac491P2pFalseStat[NUMBER_OF_DEVICES_PER_DSP][NUMBER_OF_CHANNELS];
U32	ac491ConfMd4FalseStat[NUMBER_OF_DEVICES_PER_DSP][NUMBER_OF_CHANNELS]; 
U32	ac491DeviceFalseStat[NUMBER_OF_DEVICES_PER_DSP];
U32	ac491DeviceRestartSuccessStat[NUMBER_OF_DEVICES_PER_DSP];

unsigned int ac491ChipAddr[14]={ 0x00000000, 0x0005ffff, 
                           0x00800000, 0x0085ffff,
                           0x01000000, 0x0105ffff,
                           0x01800000, 0x0185ffff,
                           0x02000000, 0x0205ffff,
                           0x02800000, 0x0285ffff,
                           0x00f40000, 0x00ffefff};

#define WRITE_HPIC1st_OFFSET 	0x00
#define WRITE_HPIC2nd_OFFSET 	0x02

#define WRITE_HPIDA1st_OFFSET 	0x04
#define WRITE_HPIDA2nd_OFFSET 	0x06

#define WRITE_HPIA1st_OFFSET 	0x08
#define WRITE_HPIA2nd_OFFSET 	0x0a

#define WRITE_HPID1st_OFFSET 	0x0c
#define WRITE_HPID2nd_OFFSET 	0x0e

#define READ_HPIC1st_OFFSET 	0x10
#define READ_HPIC2nd_OFFSET 	0x12

#define READ_HPIDA1st_OFFSET 	0x14
#define READ_HPIDA2nd_OFFSET 	0x16

#define READ_HPIA1st_OFFSET 	0x18
#define READ_HPIA2nd_OFFSET 	0x1a

#define READ_HPID1st_OFFSET 	0x1c
#define READ_HPID2nd_OFFSET 	0x1e

unsigned int hpiRead(unsigned int addr, unsigned int *data, int fd)
{
	U16 low = 0;
	U16 high = 0;
	ioctl(g_ac491_dev_fd, AC491_IOCTL_SET_ADDR, addr);

	read(fd, &high, 1);
	read(fd, &low, 2);
	*data = (high << 16) + low;
	return 0;
}

unsigned int hpiWrite(unsigned int addr, unsigned int data, int fd)
{
	short tmp;
	
	ioctl(g_ac491_dev_fd, AC491_IOCTL_SET_ADDR, addr);

	tmp = (data & 0xFFFF0000)>>16;
	write(fd, &tmp, 1);
	tmp = data & 0x0000FFFF;
	write(fd, &tmp, 2);

	return 0;
}

void ac491ActiveRegularRtpChannelConfig(void)
{
    int i,device, channel, ret;
    int Direction ;

    printf( "\n ActiveRegularRtp config: " ) ;
    for(i=0; i < NUMBER_OF_DEVICES_PER_DSP * NUMBER_OF_DEVICES_PER_DSP; i++)
    {
        device = i / NUMBER_OF_DEVICES_PER_DSP;
        channel = i % NUMBER_OF_DEVICES_PER_DSP;
	
        if(ac491ChannelState[device][channel] == CHANNEL_STATE_IDLE)
            continue ;

        printf( "." );	
		
        ret = Ac49xActivateRegularRtpChannelConfiguration
                         (device, channel, &SetupChannelAttr[device][channel].ActivateOrUpdateRtp[MEDIA_CHANNEL_TYPE__REGULAR]) ;
        if(ret)
        {
            printf( "\nAc49xActivateRegularRtpChannelConfiguration(%d,%d) is 0x%x ", device, channel, ret) ;
            DeviceRun[device] = FALSE;
            continue ;
        }
    		
        if(SetupChannelAttr[device][channel].ActivateOrUpdateRtp[MEDIA_CHANNEL_TYPE__REGULAR].Encoder 
                  == CODER__EVRC_8_8K_0_8_K)
        {
            if(Ac49xRfc3558InitializationConfiguration
                     (device,channel, &SetupChannelAttr[device][channel].Rfc3558InitializationConfiguration))
            {
                DeviceRun[device] = FALSE;
                printf("\nERROR: RFC 3558 Initialization FAIL to Channel#%d At Device#%d\n", channel, device);
            }
        }
    		
        switch(SetupChannelAttr[device][channel].ActivateOrUpdateRtp[MEDIA_CHANNEL_TYPE__REGULAR].AmrActiveMode)
        {
            case AMR_ACTIVE_MODE__3GPP_UP:
                if(Ac49xRegularChannel3GppUserPlaneInitializationConfiguration
                        (device, channel, 
                         &(SetupChannelAttr[device][channel]._3GppUserPlaneInitializationConfiguration[MEDIA_CHANNEL_TYPE__REGULAR])))
                {
                    DeviceRun[device] = FALSE;
                    printf("\nERROR: 3GPP Initialization FAIL to Channel#%d At Device#%d\n", channel, device);
                }
                break;
				
            case AMR_ACTIVE_MODE__RFC_3267:
                if(Ac49xRfc3267AmrInitializationConfiguration
                        (device, channel, &(SetupChannelAttr[device][channel].Rfc3267AmrInitializationConfiguration)))
                {
                    DeviceRun[device] = FALSE;
                    printf("\nERROR: RFC 3267 Initialization FAIL to Channel#%d At Device#%d\n", channel, device);
                }
                break;
            case AMR_ACTIVE_MODE__INACTIVE:
             	   break;
             	default:
             	   break;
        }
    		
        switch(SetupChannelAttr[device][channel].ActivateOrUpdateRtp[MEDIA_CHANNEL_TYPE__REGULAR].MediaProtectionMode)
        {
            case MEDIA_PROTECTION_MODE__PACKET_CABLE:
                for(  Direction=MEDIA_PROTECTION_DIRECTION__INBOUND_RX; 
                      Direction<=MEDIA_PROTECTION_DIRECTION__OUTBOUND_TX; 
                      Direction++)
                {
                    if(Ac49xRegularChannelPacketCableProtectionConfiguration
                          (device,channel, &SetupChannelAttr[device][channel].PacketCableProtectionConfiguration[MEDIA_CHANNEL_TYPE__REGULAR][Direction]))
                    {
                        DeviceRun[device] = FALSE;
                        printf("\nERROR: R-Channel Packet Cable Protection Config FAIL to Channel#%d At    Device#%d\n", channel, device);
                        continue;
                    }
                }
                break;
			
            case MEDIA_PROTECTION_MODE__SRTP:
                for(  Direction=MEDIA_PROTECTION_DIRECTION__INBOUND_RX; 
                      Direction<=MEDIA_PROTECTION_DIRECTION__OUTBOUND_TX;
                      Direction++)
                {
                    if(Ac49xRegularChannelSrtpProtectionConfiguration
                           (device,channel, &SetupChannelAttr[device][channel].SrtpProtectionConfiguration[MEDIA_CHANNEL_TYPE__REGULAR][Direction]))
                    {
                        DeviceRun[device] = FALSE;
                        printf("\nERROR: R-Channel SRTP Protection Config FAIL to Channel#%d At    Device#%d\n", channel, device);
                        continue;
                    }
                }
                break;
             case MEDIA_PROTECTION_MODE__DISABLE:
             	   break;
             	default:
             	   break;
        }
            
        ac491ChannelState[device][channel] = CHANNEL_STATE_REGULAR_RTP;
    }
	
}

void displayDeviceState( void )
{
    int device;

    DEBUG_OUT("\nDeviceRun: ");
    for( device=0; device < NUMBER_OF_DEVICES_PER_DSP; device++)
        DEBUG_OUT("%d-%d \n", device, DeviceRun[device]);
}

void clearAc491Statistics( void )
{
    int dev, ch;

    for(dev=0; dev < NUMBER_OF_DEVICES_PER_DSP; dev++)
    {
        for(ch=0; ch < NUMBER_OF_CHANNELS; ch++)
        {
            ac491RegularSuccessStat[dev][ch] = 0;
            ac491P2pSuccessStat[dev][ch] = 0;
            ac491ConfMd4SuccessStat[dev][ch] = 0;
            ac491RegularFalseStat[dev][ch] = 0;
            ac491P2pFalseStat[dev][ch] = 0;
            ac491ConfMd4FalseStat[dev][ch] = 0;
        }
        ac491DeviceFalseStat[dev] = 0;
        ac491DeviceRestartSuccessStat[dev] = 0;
    }
}

#if 0
/*需要上报状态事件的类型总数，参阅枚举Tac49xStatusOrEventPacketOpCode */
#define TOTAL_Tac49xStatusOrEventPacketOpCode  23

int ac491StatusRequestFlag[TOTAL_Tac49xStatusOrEventPacketOpCode][24][16];
#endif


const char *SYS_ERROR_STRING[] = {
	"Packet ERROR",
	"Time Slot Misalignment Detected and Corrected",
	"Command Sequence Error",
	"Command Checksum Error received",
	"Receive overrun",
	"Open Device params incorrect",
	"Illegal Command",
	"Playback Underrun",
	"Playback Overrun",
	"3-way conference configuration error",
	"Illegal UTOPIA cell",
	"3-way overrun error",
	"Reserved 12",
	"Bad UDP checksum - this error is sent only once after Open Channel, regardless of the count of checksum errors",
	"Wrong Channel ID",
	"Wrong Payload size",
	"Wrong Protocol",
	"Illegal 4-Way Conference Command",
	"Serial Port Error",
	"Time Slot Misalignment Detected and Corrected during initialization",
};

const char *GENERAL_EVENTS_STRING[] = {
	"CAS String Generation ended",
	"IBS String Generation ended",
	"Reserved",
	"Reserved",
	"Playback ended",
	"IBS String Generation aborted",
	"Reserved",
	"Switched to Voice",
	"Switched to Bypass",
	"Switched to Fax",
	"Switched to Data",
	"Reset Serial Port Acknowledged",
	"TDM Record Started",					  
	"DM Record Ended",					   
	"Network Record Started",					  
	"Network Record Ended", 					
	"TDM in-band Text Relay Started",					
	"TDM in-band Text Relay Ended", 					
	"Network in-band Text Relay Started",				
	"Network in-band Text Relay Ended", 				
	"In-band Text Relay Terminated",					
	"TDM Serial Port Halted – This event is sent if the TDM clock or framesync stops functioning",
	"TDM Serial Port Resumed",					   
	"AD – Detected Speech",					 
	"AD – Detected End of Speech",

};

typedef volatile struct{
	/*0 word*/
	U16		version;
	/*1 word*/
	U16		dir					:1;
	U16		subver				:7;
	U16		dev					:8;
	/*2 word*/
	U16		seqnum;
	/*3 word*/
	U16		resv;
	/*4 word*/
	U16		timestamp_Mswmsb	:8;
	U16		timestamp_Mswlsb	:8;
	/*5 word*/
	U16		timestamp_Lswmsb	:8;
	U16		timestamp_Lswlsb	:8;
}RECORD_HEAD;

UDP_SOCKET record_sck;

void get_full_packet(int Device, char *pPacket, U32 PacketSize, Tac49xPacketDirection PacketDirection)
{
	int res;
//	struct sockaddr_in sin;
	RECORD_HEAD * header;
	static U16	seqnum;
	time_t tm;
	
	header = (RECORD_HEAD*)record_sck.rawdata;
	
	header->version = 580;
	header->subver = 8;
	header->dev = Device;
	header->dir = PacketDirection;
	header->seqnum = (seqnum++);
	time(&tm);
	header->timestamp_Mswmsb = ((tm & 0xff000000)>>24);
	header->timestamp_Mswlsb = ((tm & 0xff0000)>>16);
	header->timestamp_Lswmsb = ((tm & 0xff00)>>8);
	header->timestamp_Lswlsb = (tm & 0xff);

	memcpy(record_sck.rawdata+16,pPacket,PacketSize);
	
	res = sendto(record_sck.udp_handle,record_sck.rawdata,PacketSize+16, \
			0,(struct sockaddr*)&record_sck.addr_them,record_sck.addr_len);

	if(res < 0)
		perror("Send ETH ERROR:");

	return;
}



int	Recv_XIWEI_RTP_Packet(char channel,char* Buffer,size_t len)
{
	return ac491SendData(channel/NUMBER_OF_CHANNELS,channel%NUMBER_OF_CHANNELS, (U8 *)Buffer,len,PROTOCOL__RTP);
}

void	Send_TONE(WORD wport,TONE_TYPE tone_type)
{
	Tac49xExtendedIbsStringAttr ExtendedIbsStringAttr;
	
	ExtendedIbsStringAttr.GeneratorRedirection = IBS_STRING_GENERATOR_REDIRECTION__INTO_DECODER_OUTPUT;
	ExtendedIbsStringAttr.Repeat = CONTROL__DISABLE;
	ExtendedIbsStringAttr.NumberOfDigits = 1;
	ExtendedIbsStringAttr.AmFactor = 0;
	ExtendedIbsStringAttr.Signal[0].SignalDuration = 60000;
	ExtendedIbsStringAttr.Signal[0].SignalLevel_minus_dbm = 0;
	
	{
		switch(tone_type)
		{
			case TONE_DIAL:
				ExtendedIbsStringAttr.Signal[0].Digit = 6;
				ExtendedIbsStringAttr.Signal[0].SystemCode = IBS_SYSTEM_CODE__CALL_PROGRESS_0_TO_15;				
			break;

			case TONE_BUSY:
				ExtendedIbsStringAttr.Signal[0].Digit = 8;
				ExtendedIbsStringAttr.Signal[0].SystemCode = IBS_SYSTEM_CODE__CALL_PROGRESS_0_TO_15;
			break;

			case TONE_RING:
				ExtendedIbsStringAttr.Signal[0].Digit = 2;
				ExtendedIbsStringAttr.Signal[0].SystemCode = IBS_SYSTEM_CODE__CALL_PROGRESS_0_TO_15;
			break;

			case TONE_SILENT:
				ExtendedIbsStringAttr.Signal[0].SystemCode = IBS_SYSTEM_CODE__SILENCE;
			break;

			case TONE_STOP:
				Ac49xSendIbsStop(wport/NUMBER_OF_CHANNELS,wport%NUMBER_OF_CHANNELS);
				return;
			break;
			
			default:
				Ac49xSendIbsStop(wport/NUMBER_OF_CHANNELS,wport%NUMBER_OF_CHANNELS);
				return;
			break;
		}
		
		Ac49xSendExtendedIbsString(wport/NUMBER_OF_CHANNELS,wport%NUMBER_OF_CHANNELS,&ExtendedIbsStringAttr);
	}
}

void	Send_DTMF(WORD wport,char dtmfnum)
{
	/*
	!brief
	* input:
	* wport - ac491 channel ID
	* dtmfnum - phone number
	* note:
	* use sched call this function transfer dtmf
	*/
	Tac49xExtendedIbsStringAttr ExtendedIbsStringAttr;
	ExtendedIbsStringAttr.GeneratorRedirection = IBS_STRING_GENERATOR_REDIRECTION__INTO_DECODER_OUTPUT;
	if(!strchr("0123456789*#",dtmfnum))
	{
		VERBOSE_OUT(LOG_SYS,"Phone Num %d Invalid\n",dtmfnum);
		return;
	}
	ExtendedIbsStringAttr.Repeat = CONTROL__DISABLE;
	ExtendedIbsStringAttr.AmFactor = 63;
	ExtendedIbsStringAttr.NumberOfDigits = 1;
	
	ExtendedIbsStringAttr.Signal[0].SignalDuration = 100;
	ExtendedIbsStringAttr.Signal[0].SignalLevel_minus_dbm = 0;

	if(dtmfnum == '*')
		ExtendedIbsStringAttr.Signal[0].Digit = 10;
	else if(dtmfnum == '#')
		ExtendedIbsStringAttr.Signal[0].Digit = 11;
	else
		ExtendedIbsStringAttr.Signal[0].Digit = dtmfnum - '0';

	ExtendedIbsStringAttr.Signal[0].SystemCode = IBS_SYSTEM_CODE__DTMF;
	Ac49xSendExtendedIbsString(wport/NUMBER_OF_CHANNELS,wport%NUMBER_OF_CHANNELS,&ExtendedIbsStringAttr);
}


int do_loopback_test(cmd_tbl_t * cmdtp, int argc, char *argv[])
{
    if((0 == argc)||(NULL == argv))
    {
        ERR("%s: param error\r\n", __func__);
        return DRV_ERR;
    }
    
	if(argc != 2){
		printf("Invalid Parameter!\n");
		printf("usage: %s",cmdtp->usage);
		return 0;
	}

	if(strcasecmp(argv[1],"on") == 0)
		flag_loopback = LOOPBACK_ON;
	else if(strcasecmp(argv[1],"off") == 0)
		flag_loopback = LOOPBACK_OFF;

	return 0;

}


extern int32 Ac491_Init(void);

static int ac491_cnt_err = 0;
static int ac491_cnt_overrun = 0;


void getAc491HpiPacket(int device, Tac49xTransferMedium tranMod)
{
    Tac49xRxPacketParams packetData;
    int value = 0;
    Tac49xPacket echoData;
	BYTE	current_channel;


    while(value == 0)
    {
        packetData.Device 			= device;
        packetData.TransferMedium 	= tranMod;
        packetData.pInPacket = (char*)(&echoData);

        value = Ac49xReceivePacket(&packetData);

		current_channel = echoData.HpiHeader.Channel + device*NUMBER_OF_CHANNELS;

        if(value == BUFFER_DESCRIPTOR_EMPTY_ERROR)
            break ;
        else if(value)
        {
            VERBOSE_OUT(LOG_SYS, "getAc491HpiPacket ERROR: dev=%d, return=0x%x\n", device, value );

    		ac491_cnt_err++;
    		if((1 <= ac491_cnt_err)&&(FALSE == Param_update.ac491_init))
    		{
    			ac491_cnt_err = 0;
    			Ac491_Init();
    		}
            
            break;
        }
		
        switch(echoData.HpiHeader.Protocol)
        {
            case PROTOCOL__PROPRIETARY:  /*状态事件告警包*/
            {
				//VERBOSE_OUT(LOG_SYS, "PROPRIETARY: Opcode=%d\n", echoData.u.ProprietaryPayload.ProprietaryHeader.Opcode);
				switch (echoData.u.ProprietaryPayload.ProprietaryHeader.Opcode)
				{
					case STATUS_OR_EVENT_PACKET_OP_CODE__SYSTEM_ERROR_EVENT:
					{
						int msg_index = echoData.u.ProprietaryPayload.u.StatusOrEvent.u.SystemErrorEvent.ErrorMessageIndex;
						if(msg_index < sizeof(SYS_ERROR_STRING)/sizeof(char *))
						{
							VERBOSE_OUT(LOG_SYS, "CH%d:SYS ERROR(%d):%s\n",current_channel, \
								msg_index, \
								SYS_ERROR_STRING[msg_index]);

								ac491_cnt_overrun++;
								if((1 <= ac491_cnt_overrun)&&(FALSE == Param_update.ac491_init))
								{
									ac491_cnt_overrun = 0;
									Ac491_Init();
								}	
						}
						break;
					}
					case STATUS_OR_EVENT_PACKET_OP_CODE__IBS_EVENT:
					{
						BYTE	press_statu;/*1--press down,0--press up*/
						/*
						=====================================================
						** Digit:5,PreTime:2090ms,Long DTMF:0,SC:1,D:0,chan=0
						** Digit:5,PreTime:750ms,Long DTMF:0,SC:0,D:0,chan=0
						** 按下按键5时得到两个状态包，第一个包是开始
						** 按下时得到的,第二个包是按键弹起时得到的包
						** 两个包以signal code域区分
						=====================================================
						*/
						
						press_statu = echoData.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.System;
						switch(press_statu)
						{
							case 1:
								/*detect dtmf*/
								VERBOSE_OUT(LOG_SYS, "Digit:%d,device=%d, current_channel=%d\n", \
									echoData.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.Digit, \
									device,current_channel);
								
								//append_phone_number(echoData.HpiHeader.Channel, echoData.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.Digit);
								append_phone_number(current_channel, echoData.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.Digit);
							break;

							case 6:
								/*detect Call-Progress TONE*/
								VERBOSE_OUT(LOG_SYS, "666: AC491: Detect CallProgress ch %d,Tone:%d\n", \
								current_channel, echoData.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.Digit);

								//detect_tone(echoData.HpiHeader.Channel, echoData.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.Digit);
								detect_tone(current_channel, echoData.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.Digit);
							break;

							case 7:
								/*detect Call-Progress TONE*/
								VERBOSE_OUT(LOG_SYS, "777: AC491: Detect CallProgress ch %d,Tone:%d\n", \
								current_channel, echoData.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.Digit);
							break;
						}
						break;
					}
					case STATUS_OR_EVENT_PACKET_OP_CODE__PACKETIZER_STATUS_OR_EVENT:
					{
						//VERBOSE_OUT(LOG_SYS, "PACKETIZER(%d)\n", echoData.u.ProprietaryPayload.u.StatusOrEvent.u.Packetizer.StatusOrEventIndex);
						break;
					}
					case STATUS_OR_EVENT_PACKET_OP_CODE__DEVICE_STATUS:
					{
						break;
					}
					case STATUS_OR_EVENT_PACKET_OP_CODE__GENERAL_EVENT:
					{
						int general_index = echoData.u.ProprietaryPayload.u.StatusOrEvent.u.GeneralEvent.EventIndex;
						if(general_index<sizeof(GENERAL_EVENTS_STRING)/sizeof(char*))
							VERBOSE_OUT(LOG_SYS, "CH%d:GENERAL_EVENT(%d):%s\n",current_channel, \
								general_index,GENERAL_EVENTS_STRING[general_index]);
						break;
					}
					default :
					{
						//VERBOSE_OUT(LOG_SYS, "PROPRIETARY: Opcode=%d\n", echoData.u.ProprietaryPayload.ProprietaryHeader.Opcode);
						break;
					}
				}
                break;
            }
    		
            case PROTOCOL__RTP :
            {
			if(flag_loopback == LOOPBACK_ON)
			{
				if(current_channel == 0)
				{
					int * rtpheader = (int *)echoData.u.Payload;
					rtpheader[2] = 0x11111111;
					ac491SendData(0,0, (U8 *)echoData.u.Payload,packetData.PayloadSize,PROTOCOL__RTP);
					break;
				}

			}

			/* listen */
			if((current_channel < USER_NUM_MAX) && (MC_LISTEN == terminal_group[current_channel].bCommand ))
			{
				Send_XIWEI_RTP_Packet(current_channel, (char *)echoData.u.Payload,packetData.PayloadSize);
				break;
			}

#if 0
			static int32 voc_cnt = 0;
			if(3 == current_channel)
			{
				voc_cnt++;
				if(50 == voc_cnt)
				{
					voc_cnt = 0;
					int i = 0;
					for(i = 0; i < 20; i++)
					{
						printf("%x:", echoData.u.Payload[12 + i]);
					}

					printf("\r\n");
				}
			}
#endif
			switch(Param_update.workmode)
			{
				case WORK_MODE_PAOBING:
				case WORK_MODE_ZHUANGJIA:
					if(((3 <= current_channel)&&(current_channel <= 14))\
						&&(MGW_SW_PHONE_TALK_STATE == terminal_group[current_channel].bPortState))
					{
					#ifdef ZHUANXIAN_VOICE_CTRL					
						if(NETWARE_ZXCREATE_OK == terminal_group[current_channel].zhuanxian_flg)
						{
							if(1 == terminal_group[current_channel].zhuanxian_swphone_voc_send)
							{
								Ac491_Voc_Process((uint8 *)echoData.u.Payload, \
									packetData.PayloadSize, current_channel);
							}
						}
						else
						{
							Ac491_Voc_Process((uint8 *)echoData.u.Payload, \
								packetData.PayloadSize, current_channel);
						}
					#else
						Ac491_Voc_Process((uint8 *)echoData.u.Payload, \
							packetData.PayloadSize, current_channel);
					#endif
					}
					break;
				case WORK_MODE_JILIAN:
					if((current_channel == 3) && (WORK_MODE_JILIAN == Param_update.workmode) && (terminal_group[current_channel].bPortType == PORT_TYPE_LOCAL_XIWEI )\
						&& (terminal_group[current_channel].bCommand == MC_TALK))
					{
						Hf_IVoc_Socket_Send((uint8 *)echoData.u.Payload, packetData.PayloadSize);
					}						
					break;
				case WORK_MODE_SEAT:
					break;
				case WORK_MODE_ADAPTER:
					if(MC_TALK == terminal_group[current_channel].bCommand)
					{
						switch(terminal_group[current_channel].bPortType)
						{
							case PORT_TYPE_QINWU:	
							case PORT_TYPE_TRK:
							case PORT_TYPE_KUOZHAN_PHONE:
								Kuozhan_Ac491_Voc_Process((uint8 *)echoData.u.Payload + 12, \
								packetData.PayloadSize - 12, current_channel);
								break;
							case PORT_TYPE_KUOZHAN_XIWEI:								
								break;
							case PORT_TYPE_ADP_PHONE:									
								break;									
							default:
								break;
						}	
					}
					break;
				default:
					break;
			}

			break;
            }
    		
            case PROTOCOL__RTCP :
            {
				break;
            }
    		
            case PROTOCOL__FAX :
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL__FAX\n");
				break;
            }
    		
            case PROTOCOL__ATM_AAL2 :
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL__ATM_AAL2\n");
				break;
            }
    		
            case PROTOCOL__ATM_AAL1 :
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL__ATM_AAL1\n");
				break;
            }
    		
            case PROTOCOL__MEDIATED_CHANNEL_RTP :
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL__MEDIATED_CHANNEL_RTP\n");
				break;
            }
    		
            case PROTOCOL__MEDIATED_CHANNEL_RTCP :
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL__MEDIATED_CHANNEL_RTCP\n");
				break;
            }
    		
            case PROTOCOL__CONFERENCE_1 :
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL__CONFERENCE_1\n");
				break;
            }
    		
            case PROTOCOL__CONFERENCE_2 :
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL__CONFERENCE_2\n");
				break;
            }
    		
            case PROTOCOL__DEBUG :
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL__DEBUG\n");
				break;
            }
    		
            case PROTOCOL__PLAYBACK_COMMAND :
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL__PLAYBACK_COMMAND\n");
				break;
            }
    		
            case PROTOCOL__PLAYBACK_VOICE :
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL__PLAYBACK_VOICE\n");
				break;
            }
    		
            case PROTOCOL__IP_SEC :
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL__IP_SEC\n");
				break;
            }
    		
            case PROTOCOL__CONFERENCE :
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL__CONFERENCE\n");
				break;
            }
    		
            case PROTOCOL__MEDIATED_CHANNEL_ATM_AAL1 :
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL__MEDIATED_CHANNEL_ATM_AAL1\n");
				break;
            }
    		
            case PROTOCOL__MEDIATED_CHANNEL_ATM_AAL2 :
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL__MEDIATED_CHANNEL_ATM_AAL2\n");
				break;
            }
    		
            case PROTOCOL__RECORD_COMMAND :
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL__RECORD_COMMAND\n");
				break;
            }
    		
            case PROTOCOL__RECORD_TDM_PACKET :
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL__RECORD_TDM_PACKET\n");
				break;
            }
    		
            case PROTOCOL__RECORD_NETWORK_PACKET :
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL__RECORD_NETWORK_PACKET\n");
				break;
            }
    		
            default:
            {
				VERBOSE_OUT(LOG_SYS, "PROTOCOL Type=%d\n", echoData.HpiHeader.Protocol);
				break;
            }
        }
    }
}


int ac491_thread_flg = 0;


void getAc491NetWorkPacket( void )
{
    int i;

    VERBOSE_OUT(LOG_SYS, "AC491: Start get ac491 NetWork packet...\n");
    while(1)
    {
		if(1 == ac491_thread_flg)
		{
			printf("%s: exit\r\n", __func__);
			pthread_exit(NULL);
		}
			
#if 1

		for(i = 0; i < NUMBER_OF_DEVICES_PER_DSP; i++)
		{
			getAc491HpiPacket(i, TRANSFER_MEDIUM__NETWORK);
			usleep(20);
		}   

		//usleep(100);

#elif 0   
        for(i = 0; i < NUMBER_OF_DEVICES_PER_DSP; i++)
        {
			getAc491HpiPacket(i, TRANSFER_MEDIUM__NETWORK);
			//usleep(1000);
			usleep(250);
        }
#else
		getAc491HpiPacket(0,TRANSFER_MEDIUM__NETWORK);
		for (i = 0; i < 20; ++i)
		{
			usleep(100);
		}
#endif		
    }
}


void getAc491HostPacket( void )
{
    int i;

    VERBOSE_OUT(LOG_SYS, "AC491: Start get ac491 Host packet...\n");
    while(1)
    {
		if(1 == ac491_thread_flg)
		{
			printf("%s: exit\r\n", __func__);
			pthread_exit(NULL);
		}    
#if 1

		for(i = 0; i < NUMBER_OF_DEVICES_PER_DSP; i++)
		{
			getAc491HpiPacket(i, TRANSFER_MEDIUM__HOST);
            usleep(10);
		}   

		//usleep(50);

#elif 0    
        for(i = 0; i < NUMBER_OF_DEVICES_PER_DSP; i++)
        {
			getAc491HpiPacket(i, TRANSFER_MEDIUM__HOST);
			//usleep(400);
			usleep(100);
        }
#else
		getAc491HpiPacket(0,TRANSFER_MEDIUM__HOST);
		for (i = 0; i < 50; ++i)
		{
			usleep(100);
		}
#endif
    }
}
/* 5ms检查一次AC491发过来的包
 -----------------------------------------------------------------
 input: none
 output: none 
 */
void startGetAc491Packet(void)
{
#if 0
	int device, channel, opcode;

    for(opcode=0; opcode < TOTAL_Tac49xStatusOrEventPacketOpCode; opcode++)
    {
        for(device = 0; device < NUMBER_OF_DEVICES_PER_DSP; device++)
        {
            for(channel=0; channel < NUMBER_OF_CHANNELS; channel++)
            {
                ac491StatusRequestFlag[opcode][device][channel] = 0;
            }
        }
    }
#endif
    /*创建收包任务*/
    pthread_t tNetWork;
	pthread_attr_t attrNetWork;

	pthread_t tHost;
	pthread_attr_t attrHost;

//	pthread_t thread_rtp_read;
//	pthread_attr_t attr_rtp_read;

//	pthread_t thread_tone;
//	pthread_attr_t attr_thread_tone;
	
	/* Start a new thread, and get something handling this channel. */
	pthread_attr_init(&attrNetWork);
	pthread_attr_setdetachstate(&attrNetWork, PTHREAD_CREATE_DETACHED);
	pthread_attr_setinheritsched(&attrNetWork, PTHREAD_INHERIT_SCHED);
	pthread_attr_setstacksize(&attrNetWork, ((sizeof(void *) * 8 * 8) - 16) * 1024);
	if(pthread_create(&tNetWork, &attrNetWork, (void *)getAc491NetWorkPacket, NULL)) 
	{
		VERBOSE_OUT(LOG_SYS, "AC491: Failed to create GetAc491NetWorkPacket thread\n");
	}
	pthread_attr_destroy(&attrNetWork);

	
#if 1
	pthread_attr_init(&attrHost);
	pthread_attr_setdetachstate(&attrHost, PTHREAD_CREATE_DETACHED);
	pthread_attr_setinheritsched(&attrHost, PTHREAD_INHERIT_SCHED);
	pthread_attr_setstacksize(&attrHost, ((sizeof(void *) * 8 * 8) - 16) * 1024);
	if (pthread_create(&tHost, &attrHost, (void *)getAc491HostPacket, NULL)) {
		VERBOSE_OUT(LOG_SYS, "Failed to create GetAc491HostPacket thread\n");
	}
	pthread_attr_destroy(&attrHost);
#else
	int ret;
	Tac49xIbsStringAttr IbsStringAttr;
	
	ret = Ac49xUpdateAtmChannelConfiguration(0, 0, &SetupChannelAttr[0][0].ActivateOrUpdateAtm[0]);
	if (ret)
		VERBOSE_OUT(LOG_SYS, "Ac49xUpdateAtmChannelConfiguration 1 is 0x%x \n", ret);
	
	ret = Ac49xActivateAtmChannelConfiguration(0, 0, &SetupChannelAttr[0][0].ActivateOrUpdateAtm[0]);
	if (ret)
		VERBOSE_OUT(LOG_SYS, "Ac49xActivateAtmChannelConfiguration 0 is 0x%x \n", ret);
	
	#if 0
	IbsStringAttr.GeneratorRedirection = IBS_STRING_GENERATOR_REDIRECTION__INTO_ENCODER_INPUT;
    IbsStringAttr.NumberOfDigits = 0;
    IbsStringAttr.OnDuration = 0;
    IbsStringAttr.OffDuration = 0;
    IbsStringAttr.PauseDuration = 0;
	IbsStringAttr.Signal[0].SystemCode = IBS_SYSTEM_CODE__DTMF;
	ret = Ac49xSendIbsString(0, 0, &IbsStringAttr);
	if (ret)
		VERBOSE_OUT(LOG_SYS, "Ac49xSendIbsString is 0x%x \n", ret);
	#endif
#endif

#if 0
/*Test Call Progress define tone*/
{
	Tac49xExtendedIbsStringAttr ExtendedIbsStringAttr;
	
	ExtendedIbsStringAttr.GeneratorRedirection = IBS_STRING_GENERATOR_REDIRECTION__INTO_DECODER_OUTPUT;
	ExtendedIbsStringAttr.Repeat = CONTROL__ENABLE;
	ExtendedIbsStringAttr.NumberOfDigits = 1;	
	ExtendedIbsStringAttr.Signal[0].SignalDuration = 5000;
	ExtendedIbsStringAttr.Signal[0].SignalLevel_minus_dbm = 0;
	ExtendedIbsStringAttr.Signal[0].Digit = 6;
	ExtendedIbsStringAttr.Signal[0].SystemCode = IBS_SYSTEM_CODE__CALL_PROGRESS_0_TO_15;
	Ac49xSendExtendedIbsString(0,0,&ExtendedIbsStringAttr);
	VERBOSE_OUT(LOG_SYS,"AC491: Send Extended IBS String\n");
}
#endif
	return;
}

int Dev = 0;

int Ac491DrvInit(void)
{
	int i, ret = 0, device, channel;
	int data = 0x12345678;
	unsigned int addr, startAddr, endAddr, readData;

	g_ac491_dev_fd = open("/dev/ac491", O_RDWR);
	if (g_ac491_dev_fd == -1) {
		VERBOSE_OUT(LOG_SYS, "Cann't open file \n");
		return -1;
	}
	
	Ac49xUserDef_VoppReset(0, 1);
	Ac49xUserDef_DelayForDeviceResponse();
	Ac49xUserDef_VoppReset(0, 0);
	
    for(i=0; i<3; i++) {
        startAddr = ac491ChipAddr[i*2] ;
        endAddr = ac491ChipAddr[i*2+1] ;
        for( addr=startAddr; addr<startAddr+16/*endAddr*/; addr=addr+4 ) {
			(void)hpiWrite(addr, data, g_ac491_dev_fd);
        }
        
        for( addr=startAddr; addr<startAddr+16/*endAddr*/; addr=addr+4 ) {
			(void)hpiRead(addr, &readData, g_ac491_dev_fd);
            if( readData != data ) {
                VERBOSE_OUT(LOG_SYS, "read!=write,dev=%d,addr=%x,read=0x%x,write=0x%x\n",
                          i, addr, readData, data);
				return -1;
            }
        }
    }

	ioctl(g_ac491_dev_fd, AC491_IOCTL_DOWNLOAD, 0);
	
	Ac49xInitDriver(CONTROL__DISABLE); /* DebugMode = Disable*/

#if 0
	/*===========record function===============*/
	VERBOSE_OUT(LOG_SYS,"Start Record all ac491 packets!\n");
	record_sck.local_ip = "192.168.1.100";
	record_sck.local_port = 50000;
	record_sck.remote_ip = "192.168.1.114";
	record_sck.remote_port = 50000;
	socket_new_with_bindaddr(&record_sck);
	Ac49xInitializePacketRecording(get_full_packet);/*抓包*/
#endif	

    for(Dev = 0; Dev < NUMBER_OF_DEVICES_PER_DSP; Dev++)
    {	
        Ac49xSetDefaultSetupDeviceAttr(&SetupDeviceAttr[Dev]);

		//SetupDeviceAttr[Dev].Open.Common.BusSpeed = BUS_SPEED__2MHZ ;//swf

		ret = Ac49xSetupDevice(Dev, &SetupDeviceAttr[Dev]);
        if(ret != SETUP_DEVICE_STATUS__SUCCESS)
        {
            DeviceRun[Dev] = 0;
            VERBOSE_OUT(LOG_SYS, "ERROR: Ac49xSetupDevice %d,ret=%d\n", i, ret);
            ret = -1;
        }
        else
        {
            DeviceRun[Dev] = 1;
        }
    }

	for(device=0;device < NUMBER_OF_DEVICES_PER_DSP;device++)
		for(channel=0;channel < NUMBER_OF_CHANNELS;channel++)
    	{
        Ac49xSetDefaultSetupChannelAttr(&SetupChannelAttr[device][channel]);
    	}
// luk
#if 0
	for(i=0; i < NUMBER_OF_DEVICES_PER_DSP * NUMBER_OF_CHANNELS; i++)
    {
        device = i / NUMBER_OF_DEVICES_PER_DSP;
        channel = i % NUMBER_OF_DEVICES_PER_DSP;
        ret = Ac49xAdvancedChannelConfiguration(device, channel, &SetupChannelAttr[device][channel].Advanced);
        if(ret)
        {
            VERBOSE_OUT(LOG_SYS, "Ac49xAdvancedChannelConfiguration(%d,%d) is 0x%x ", device, channel, ret);
            DeviceRun[device] = FALSE;
        }
    }
	
    for(i=0; i < NUMBER_OF_DEVICES_PER_DSP * NUMBER_OF_CHANNELS; i++)
    {
		device = i / NUMBER_OF_DEVICES_PER_DSP;
        channel = i % NUMBER_OF_DEVICES_PER_DSP;
	
        ret = Ac49xOpenChannelConfiguration(device, channel, &(SetupChannelAttr[device][channel].OpenOrUpdateChannel));
        if(ret)
        {
            VERBOSE_OUT(LOG_SYS, "Ac49xOpenChannelConfiguration(%d,%d) is 0x%x ", device, channel, ret);
            DeviceRun[device] = FALSE;
            ac491ChannelState[device][channel] = CHANNEL_STATE_IDLE ;
        }
        else
        {
            ac491ChannelState[device][channel] = CHANNEL_STATE_TDM_ONLY;
        }
    }

	ac491ActiveRegularRtpChannelConfig();
#else


#if 0
	SetupChannelAttr[0][0].ActivateOrUpdateRtp[0].DecoderPcmAdpcmFrameDuration = PCM_ADPCM_FRAME_DURATION__20_MSEC;
	SetupChannelAttr[0][0].ActivateOrUpdateRtp[0].EncoderPcmAdpcmFrameDuration = PCM_ADPCM_FRAME_DURATION__20_MSEC;
#endif

#if 0
	/*回声抵消设置,抵消长度设置为128ms*/
	SetupChannelAttr[0][0].OpenOrUpdateChannel.EchoCancelerNonLinearProcessor = ECHO_CANCELER_NON_LINEAR_PROCESSOR_MODE__HIGH_SENSITIVITY;
	SetupChannelAttr[0][0].OpenOrUpdateChannel.EchoCancelerLength = ECHO_CANCELER_LENGTH__128_MSEC;
	SetupChannelAttr[0][0].OpenOrUpdateChannel.EchoCancelerMode = ECHO_CANCELER_MODE__ENABLE;
	
	SetupChannelAttr[0][1].OpenOrUpdateChannel.EchoCancelerNonLinearProcessor = ECHO_CANCELER_NON_LINEAR_PROCESSOR_MODE__HIGH_SENSITIVITY;
	SetupChannelAttr[0][1].OpenOrUpdateChannel.EchoCancelerLength = ECHO_CANCELER_LENGTH__128_MSEC;	
	SetupChannelAttr[0][1].OpenOrUpdateChannel.EchoCancelerMode = ECHO_CANCELER_MODE__ENABLE;

	SetupChannelAttr[0][0].Advanced.EchoCancellerAutomaticToneResetMode = CONTROL__ENABLE;
	SetupChannelAttr[0][1].Advanced.EchoCancellerAutomaticToneResetMode = CONTROL__ENABLE;	
#endif

	for(i = 0; i < (QINWU_NUM_MAX+ TRUNK_NUM_MAX + PHONE_NUM_MAX ); i++)
	{
		SetupChannelAttr[i/NUMBER_OF_CHANNELS][i%NUMBER_OF_CHANNELS].OpenOrUpdateChannel.EchoCancelerNonLinearProcessor = ECHO_CANCELER_NON_LINEAR_PROCESSOR_MODE__HIGH_SENSITIVITY;
		SetupChannelAttr[i/NUMBER_OF_CHANNELS][i%NUMBER_OF_CHANNELS].OpenOrUpdateChannel.EchoCancelerLength = ECHO_CANCELER_LENGTH__128_MSEC;
		SetupChannelAttr[i/NUMBER_OF_CHANNELS][i%NUMBER_OF_CHANNELS].OpenOrUpdateChannel.EchoCancelerMode = ECHO_CANCELER_MODE__ENABLE;
		SetupChannelAttr[i/NUMBER_OF_CHANNELS][i%NUMBER_OF_CHANNELS].Advanced.EchoCancellerAutomaticToneResetMode = CONTROL__ENABLE;
	}

	/*Adjust Gain For QinWu*/
#if 0
	if(Param_update.workmode == WORK_MODE_ZHUANGJIA)
	{
		for(i = 0; i < (QINWU_NUM_MAX+ TRUNK_NUM_MAX); i++)
		{
			if(0 == i)
			{
				continue;
			}

			SetupChannelAttr[i/NUMBER_OF_CHANNELS][i%NUMBER_OF_CHANNELS].OpenOrUpdateChannel.VoiceOutputGain = VOICE_GAIN__minus5_DB; 
			SetupChannelAttr[i/NUMBER_OF_CHANNELS][i%NUMBER_OF_CHANNELS].OpenOrUpdateChannel.InputGain = VOICE_GAIN__plus5_DB; /*adatper OK*/
		}
	}
#endif

	if(Param_update.workmode == WORK_MODE_ADAPTER)
	{
		for(i = 0; i < (QINWU_NUM_MAX+ TRUNK_NUM_MAX + PHONE_NUM_MAX ); i++)
		{
			if((0 == i)||(1 == i)||(2 == i))
			{
				continue;
			}

			//SetupChannelAttr[i/NUMBER_OF_CHANNELS][i%NUMBER_OF_CHANNELS].OpenOrUpdateChannel.InputGain = VOICE_GAIN__minus10_DB; 
			SetupChannelAttr[i/NUMBER_OF_CHANNELS][i%NUMBER_OF_CHANNELS].OpenOrUpdateChannel.InputGain = VOICE_GAIN__minus2_DB; /*adatper OK*/
		}
	}
	
	//SetupChannelAttr[0][4].OpenOrUpdateChannel.VoiceOutputGain = VOICE_GAIN__plus10_DB;

	/*Open Detect Call Progress Feature for trunk*/
	for(i = 0; i < (QINWU_NUM_MAX+ TRUNK_NUM_MAX + PHONE_NUM_MAX ); i++)
	{
		SetupChannelAttr[i/NUMBER_OF_CHANNELS][i%NUMBER_OF_CHANNELS].OpenOrUpdateChannel.DetectCallProgress = CONTROL__ENABLE;
		SetupChannelAttr[i/NUMBER_OF_CHANNELS][i%NUMBER_OF_CHANNELS].OpenOrUpdateChannel.DetectUserDefinedTones = CONTROL__ENABLE;
	}

	for(i = 0; i < (QINWU_NUM_MAX+ TRUNK_NUM_MAX + PHONE_NUM_MAX ); i++)
	{
		SetupChannelAttr[i/NUMBER_OF_CHANNELS][i%NUMBER_OF_CHANNELS].OpenOrUpdateChannel.DetectCallProgress = CONTROL__ENABLE;
		SetupChannelAttr[i/NUMBER_OF_CHANNELS][i%NUMBER_OF_CHANNELS].OpenOrUpdateChannel.DetectUserDefinedTones = CONTROL__ENABLE;
	}	
	
#if 1
	for(device = 0;device < NUMBER_OF_DEVICES_PER_DSP;device++)
		for(channel = 0;channel<NUMBER_OF_CHANNELS;channel++){
			ret = Ac49xAdvancedChannelConfiguration(device, channel, &SetupChannelAttr[device][channel].Advanced);
			if (ret)
				VERBOSE_OUT(LOG_SYS, "Ac49xAdvancedChannelConfiguration (%d,%d) is 0x%x \n", device,channel,ret);

			ret = Ac49xOpenChannelConfiguration(device, channel, &(SetupChannelAttr[device][channel].OpenOrUpdateChannel));
			if (ret)
				VERBOSE_OUT(LOG_SYS, "Ac49xOpenChannelConfiguration (%d,%d) is 0x%x \n",device,channel,ret);

			ret = Ac49xActivateRegularRtpChannelConfiguration(device, channel, &SetupChannelAttr[device][channel].ActivateOrUpdateRtp[0]);
			if (ret)
				VERBOSE_OUT(LOG_SYS, "Ac49xActivateRegularRtpChannelConfiguration (%d,%d) is 0x%x \n", device,channel,ret);
		}
#endif

	
#endif
	displayDeviceState();
	clearAc491Statistics();
	startGetAc491Packet();
	return ret;
}

#if (PACKET_RECORDING_ENABLE==1)
/*************************************************************************************/
/*************************************************************************************/
/*                             PACKET RECORDING                                      */
/*************************************************************************************/
/*************************************************************************************/
Tac49xPacketRecording _49xPacketRecording;

/*************************************************************************************/
void Initialize(void){_49xPacketRecording.FlushPacket=0;}

/*************************************************************************************/
void Finalize(void){}

/*************************************************************************************/
Tac49xPacketRecordingErrorType Add(int Device, char *pPacket, unsigned int PacketSize, Tac49xPacketDirection PacketDirection)
{
	if(!_49xPacketRecording.FlushPacket)
		return PACKET_RECORDING_ERROR_TYPE__FLUSH_METHOD_NOT_APPLIED;
	if(PacketSize>PACKETS_RECORDING__MAX_PACKET_SIZE)
		return PACKET_RECORDING_ERROR_TYPE__PACKET_SIZE_EXCEEDED;
	_49xPacketRecording.FlushPacket(Device, pPacket, PacketSize, PacketDirection);
	return PACKET_RECORDING_ERROR_TYPE__NONE;
}

/*************************************************************************************/
void Ac49xInitializePacketRecording(TFlushPacketFunction FlushPacket)
{
	_49xPacketRecording.FlushPacket		= FlushPacket;
}
#endif /*(PACKET_RECORDING_ENABLE==1)*/
/*************************************************************************************/
/*************************************************************************************/
/*                      END OF PACKET RECORDING                                      */
/*************************************************************************************/
/*************************************************************************************/

/*******************************************/
/*******************************************/
/*      Device Memory Access Routines     **/
/*******************************************/
/*******************************************/

/**************************************************************************************/
Tac49xByteOrderingTestStatus Ac49xByteOrderingTest(int Device)
{
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )		
	/* e.g  Address=0x04030201 for AC49X_HPI_TYPE == HPI_TYPE_MULTIPLEX_16								*/
	/* If there is a conversion from one little endian to another than this address will be interpret	*/
	/* as Address=0x03040102 from the DSP point of view.												*/
	/* 0x0ff80000 is an address in the shared memory.													*/
	/* If there is an ordering byte problam than this address will actually interpret as 0xf80f0000		*/
	/* that is actualy a reserved address space.														*/
	U32 MemoryAddress   = 0x0ff80000;
	U32 TxData          = 0x04030201;
	U32 RxData;
	int bGpio0;
    int iteration;

    

	/* HPI Reset pulse */
	Ac49xUserDef_VoppReset(0, 1);	 
	Ac49xUserDef_DelayForDeviceResponse();
	Ac49xUserDef_VoppReset(0, 0);  

    /* from this stage HPI works. */
    /* if AC491_DEVICE, all devices works by polling address (0x100*2) while NULL */
	for(iteration=0; iteration<2; iteration++)
		{
		bGpio0 = Ac49xUserDef_GetGpio0(0);
		if(bGpio0)
			break;
   		Ac49xUserDef_DelayForDeviceResponse();
		}
    if(!bGpio0)
	    return BYTE_ORDERING_TEST_STATUS__FAILED_DUE_TO_AC491_GPIO_LOW;

	if(Device%NUMBER_OF_DEVICES_PER_DSP)
		return BYTE_ORDERING_TEST_STATUS__OK;
	Ac49xWriteBlock(Device, MemoryAddress, (char*)&TxData, sizeof(U32));
	Ac49xReadBlock( Device, MemoryAddress, (char*)&RxData, sizeof(U32));
	if(TxData != RxData)
		return BYTE_ORDERING_TEST_STATUS__FAILED_DUE_TO_SWAP; /*byte ordering missmatch.*/

		/* Verify reading Device ID Register */
#if (AC49X_DEVICE_TYPE == AC491L_DEVICE)
	Ac49xReadBlock( Device, HCRF_STATUS_REGISTER_ADDRESS__DEVICE_ID, (char*)&RxData, sizeof(U32));
	if ((U16)(RxData&0x0000FFFF)!=AC491L_DEVICE_ID_REGISTER_VALUE)
		return BYTE_ORDERING_TEST_STATUS__FAILED_DUE_TO_WRONG_DEVICE_ID;
#endif /*  */


	return BYTE_ORDERING_TEST_STATUS__OK;

#elif AC49X_DEVICE_TYPE==AC490_DEVICE			
	/* e.g  Address=HPIC register Address for AC49X_HPI_TYPE == HPI_TYPE_NON_MULTIPLEX					*/
	/* After VoPP Reset we writes 0xFFFF.																*/
	/* Due to bit access the value writen is 0xA000.													*/
	/* Thus if there is a byte ordering missmatch due to Big->Little Endian and vise versa   			*/
	/* The content after read will be 0x00A0															*/
	U8 RxData1[2];
	U8 TxData1[2]={0x20, 0x00};

	U8 RxData2[2];
	U8 TxData2[2]={0x00, 0x20};

	if(Device%NUMBER_OF_DEVICES_PER_DSP)
		return BYTE_ORDERING_TEST_STATUS__OK;
	Ac49xUserDef_VoppReset(Device, 1);	 
	Ac49xUserDef_DelayForDeviceResponse(); 
	Ac49xUserDef_VoppReset(Device, 0);

	Ac49xWriteBlock(Device, HPIC_Offset, (char*)&TxData1, 2);
    Ac49xReadBlock( Device, HPIC_Offset, (char*)&RxData1, 2); 

	Ac49xUserDef_DelayForDeviceResponse(); 
	Ac49xUserDef_VoppReset(Device, 1);	 
	Ac49xUserDef_DelayForDeviceResponse(); 
	Ac49xUserDef_VoppReset(Device, 0);
	
	Ac49xWriteBlock(Device, HPIC_Offset, (char*)&TxData2, 2);
    Ac49xReadBlock( Device, HPIC_Offset, (char*)&RxData2, 2); 

	if(RxData1[0]==0x00 && RxData1[1]==0x00)
	if(RxData2[0]==0x00 && RxData2[1]==0x20)
		return BYTE_ORDERING_TEST_STATUS__OK;
	if(RxData1[0]==0x20 && RxData1[1]==0x00)
	if(RxData2[0]==0x00 && RxData2[1]==0x00)
		return BYTE_ORDERING_TEST_STATUS__FAILED_DUE_TO_SWAP;
	return BYTE_ORDERING_TEST_STATUS__FAILED_DUE_TO_HPI_ACCESS_PROBLEM;
#endif
	return BYTE_ORDERING_TEST_STATUS__OK;
}

/**************************************************************************************/
U32	Ac49xHpiAccessForXaramTimingTest(int Device, U32 BottomAddress, U32 TopAddress)
{
	U32 Address;
	U32 TxData;
	U32 RxData;
    int ReadWriteSession;
    typedef enum
        {
        READ_SESSION,
        WRITE_SESSION
        }TReadOrWriteSession;
    for(ReadWriteSession=READ_SESSION; ReadWriteSession<=WRITE_SESSION; ReadWriteSession++)
        {
	    for(Address=BottomAddress; Address<TopAddress; Address+=sizeof(U32))
		    {              /* some Read/Write forbidden areas */
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )
		    if((Address+sizeof(U32)) >= TopAddress) 
			    break;
            if(((Address+28)>=0x60000)  && Address<0x0FF80000)
			    break;
		    if(Address>=0x30000         && Address<=0x31770)
			    continue;
		    if(Address>=0x0FF80         && Address<=0x0FFFF)
			    continue;
		    if(Address>=0x00200         && Address<=0x00203)
			    continue;
#endif /*(AC49X_DEVICE_TYPE == AC491_DEVICE) || (AC49X_DEVICE_TYPE == AC491L_DEVICE)*/

            if(ReadWriteSession==READ_SESSION)
                {
		        TxData = Address;
		        Ac49xWriteBlock(Device, Address, (char*)&TxData, sizeof(U32));
                }
            else            /* WRITE_SESSION */
                {
		        TxData = Address;
		        Ac49xReadBlock(Device, Address, (char*)&RxData, sizeof(U32));
		        if(TxData!=RxData)
			        return Address;
                }
		    }
        }
	return 0;
}

/**************************************************************************************/
U32	 Ac49xHpiAccessTimingTest(int Device)
{
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )
   	const U32 DARAM_BOTTOM_ADDRESS          = 0xC0;
	const U32 DARAM_TOP_ADDRESS		        = 0xFFFF;
	const U32 SHARED_MEMORY_BOTTOM_ADDRESS  = 0x0FF80000;
	const U32 SHARED_MEMORY_TOP_ADDRESS		= 0x0FFFEFFF;
	const U32 SaramBottomAddress[5]			= {0x10000, 0x20000, 0x30000, 0x40000, 0x50000};
	const U32 SaramTopAddress[5]			= {0x1FFFF, 0x2FFFF, 0x3FFFF, 0x4FFFF, 0x5FFFF};
	U32 ReturnAddress;
	int i;
   	if((Device%NUMBER_OF_DEVICES_PER_DSP) == 0)/*(Device == 0)*/
        {
	    ReturnAddress = Ac49xHpiAccessForXaramTimingTest(Device, SHARED_MEMORY_BOTTOM_ADDRESS, SHARED_MEMORY_TOP_ADDRESS);
	    if(ReturnAddress)
		    return ReturnAddress;
        }
	ReturnAddress = Ac49xHpiAccessForXaramTimingTest(Device, DARAM_BOTTOM_ADDRESS, DARAM_TOP_ADDRESS);
	if(ReturnAddress)
		return ReturnAddress;
	for(i=0; i<5; i++)
		{
		ReturnAddress = Ac49xHpiAccessForXaramTimingTest(Device, SaramBottomAddress[i], SaramTopAddress[i]);
		if(ReturnAddress)
			return ReturnAddress;
		}
#elif (AC49X_DEVICE_TYPE == AC490_DEVICE)
	const U32 DaramInitAddress				= 0x0;
	const U32 DaramBlockSize				= 0x2000;
	const U32 NumberOfDaramBlocks			= 8;
	const U32 SaramInitAddress				= 0x10000;
	const U32 SaramBlockSize				= 0x2000;
	const U32 NumberOfSaramBlocks			= 32;
	U32		  ReturnAddress;
	U32		  Block;
	U32		  BottomAddress;
	U32		  TopAddres;

	for(Block=1; Block<NumberOfDaramBlocks; Block++)
		{
		BottomAddress = DaramInitAddress+Block*DaramBlockSize;
		TopAddres	  = DaramInitAddress+(Block+1)*DaramBlockSize-1;
/*		
		if(Block==0)
			BottomAddress = 0x2000;
*/
		ReturnAddress = Ac49xHpiAccessForXaramTimingTest(Device, BottomAddress, TopAddres);
		if(ReturnAddress)
			return ReturnAddress;
		}
	for(Block=0; Block<NumberOfSaramBlocks; Block++)
		{
		ReturnAddress = Ac49xHpiAccessForXaramTimingTest(Device, SaramInitAddress+Block*SaramBlockSize, SaramInitAddress+(Block+1)*SaramBlockSize-1);
		if(ReturnAddress)
			return ReturnAddress;
		}
#endif
	return 0;
}

/****************************************************************/
void Ac49xDeviceInitHPI(int Device)
{
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )
	U16 Flags = (U16)(MSB_FIRST|HINT_DEACTIVE);

	if((Device%NUMBER_OF_DEVICES_PER_DSP) == 0)/*(Device == 0)*/
		{
		Ac49xUserDef_WriteShort(Device, HPIC1st, Flags);
		Ac49xUserDef_WriteShort(Device, HPIC2nd, Flags);
		}
#endif
}

/*****************************\
*******************************
*  HCRF Registers Handling   **
*******************************
\*****************************/

/****************************************************************/
void Ac49xHcrfSetHostControlRegisterAddresses(void)
{
	int Device;
	int DeviceOffset;
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )
	U32 CommandAddress = HCRF_COMMAND_REGISTER_ADDRESS__CORE_A;
	U32 StatusAddress  = HCRF_STATUS_REGISTER_ADDRESS__CORE_A;
#elif (  (AC49X_DEVICE_TYPE == AC490_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	U32 CommandAddress = HCRF__COMMAND_REGISTER_ADDRESS;
	U32 StatusAddress  = HCRF__STATUS_REGISTER_ADDRESS;
#endif
	for(Device=0; Device<AC49X_NUMBER_OF_DEVICES; Device++)
		{
		DeviceOffset = Device%NUMBER_OF_DEVICES_PER_DSP;
		ac49xHcrfCommandRegisterAddress[Device] = CommandAddress + sizeof(Tac49xHcrfCommandRegister)*DeviceOffset;
		ac49xHcrfStatusRegisterAddress[Device ] = StatusAddress  + sizeof(Tac49xHcrfStatusRegister )*DeviceOffset;
		}
}

/****************************************************************/
int HcrfWaitForResponse(int Device, Tac49xHcrfStatusRegister *pStatusRegister)
{
	int ReadTimeOut=100;
	int StatusIndex=0;
	do	{
		if(ReadTimeOut--==0)
			break;
		Ac49xUserDef_DelayForDeviceResponse();
		Ac49xUserDef_CriticalSection_EnterHpi(Device);
		Ac49xHcrfReadRegister(HCRF_CONTEXT__STATUS_REGISTER, Device, (char*)pStatusRegister);
		Ac49xUserDef_CriticalSection_LeaveHpi(Device);
		StatusIndex = MergeFieldsToShort(pStatusRegister->StatusIndex);
		}
	while(StatusIndex == 0);
	return (StatusIndex) ? 0 : 1;	 
}

/****************************************************************/
void Ac49xHcrfSetCommandRegister(int Device, Tac49xHcrfCommandRegisterIndex  CommandIndex, int ReferenceCoreId)
{
	Tac49xHcrfCommandRegister Register;
	memset((void*)&Register, 0, sizeof(Tac49xHcrfCommandRegister));
	Register.One			 = 1;
	Register.CommandIndex	 = CommandIndex;
	Register.ReferenceCoreId = (ReferenceCoreId%NUMBER_OF_DEVICES_PER_DSP);
	Ac49xUserDef_CriticalSection_EnterHpi(Device);
	Ac49xHcrfWriteRegister(HCRF_CONTEXT__COMMAND_REGISTER, Device, (char*)&Register);
	Ac49xUserDef_CriticalSection_LeaveHpi(Device);
}

#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )
int HcrfCommandRegisterReady(int Device);
#endif

#if (  (AC49X_DEVICE_TYPE == AC490_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC494_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
    )
/****************************************************************/
void HcrfSendInterrupt(int Device)
{
	Tac49xHcrfStatusRegister Register;
#if (AC49X_DEVICE_TYPE == AC490_DEVICE)
    Tac49xHpic Hpic;
#endif

	memset((void*)&Register, 0, sizeof(Tac49xHcrfStatusRegister));
#if (AC49X_DEVICE_TYPE == AC490_DEVICE)
    memset((void*)&Hpic,     0, sizeof(Tac49xHpic              ));
	Hpic.DspInt = 1;
#endif
	
	Ac49xUserDef_CriticalSection_EnterHpi(Device);
	
	Ac49xHcrfWriteRegister(HCRF_CONTEXT__STATUS_REGISTER, Device, (char*)&Register);
#if (AC49X_DEVICE_TYPE == AC490_DEVICE)
    Ac49xWriteBlock(Device, HPIC_Offset, (char*)&Hpic,sizeof(Tac49xHpic)); /* send an interrupt */ 
#elif (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	Ac49xUserDef_InterruptMips();
#endif
	Ac49xUserDef_CriticalSection_LeaveHpi(Device);
}
#endif

/****************************************************************/
/****************************************************************/
int HcrfDeviceHalt(int Device, Tac49xHcrfStatusRegisterIndex *pResponseValue)
{
	Tac49xHcrfStatusRegister StatusRegister;
	int						 ReturnedVal;

	memset((void*)&StatusRegister, (char)0, sizeof(Tac49xHcrfStatusRegister));
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )
	if(HcrfCommandRegisterReady(Device))
		return 1;
#elif (  (AC49X_DEVICE_TYPE == AC490_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	Ac49xWriteBlock(Device, HCRF__INT_ENTRY_ADDRESS, acHcrfIntEntry, HCRF__INT_ENTRY_SIZE); /* restore INT Entry */
#endif

	Ac49xHcrfSetCommandRegister(Device, HCRF_COMMAND_REGISTER_INDEX__CORE_HALT, 0);
#if   (  (AC49X_DEVICE_TYPE == AC490_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	HcrfSendInterrupt(Device);
#endif
	ReturnedVal		= HcrfWaitForResponse(Device, &StatusRegister);
	*pResponseValue = (Tac49xHcrfStatusRegisterIndex)MergeFieldsToShort(StatusRegister.StatusIndex);
	return ReturnedVal;
}

/****************************************************************/
/****************************************************************/

int HcrfDeviceResume(int Device, Tac49xHcrfStatusRegisterIndex *pResponseValue)
{
	Tac49xHcrfStatusRegister StatusRegister;
	int						 ReturnedVal;

	memset((void*)&StatusRegister, (char)0, sizeof(Tac49xHcrfStatusRegister));
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )
	if(HcrfCommandRegisterReady(Device))
		return 1;
#endif
	Ac49xHcrfSetCommandRegister(Device, HCRF_COMMAND_REGISTER_INDEX__CORE_RESUME, 0);
#if   (  (AC49X_DEVICE_TYPE == AC490_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	HcrfSendInterrupt(Device);
#endif
	ReturnedVal		= HcrfWaitForResponse(Device, &StatusRegister);
	*pResponseValue = (Tac49xHcrfStatusRegisterIndex)MergeFieldsToShort(StatusRegister.StatusIndex);
	return ReturnedVal;
}

/****************************************************************/
/****************************************************************/
int HcrfGetCpuRegisters(int Device, char *pInfo, Tac49xHcrfStatusRegisterIndex *pResponseValue)
{
	Tac49xHcrfStatusRegister StatusRegister;
	int						 ReturnedVal;

	memset((void*)&StatusRegister, (char)0, sizeof(Tac49xHcrfStatusRegister));
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )
	if(HcrfCommandRegisterReady(Device))
		return 1;
#endif
	Ac49xHcrfSetCommandRegister(Device, HCRF_COMMAND_REGISTER_INDEX__GET_CPU_REGISTERS, 0);
#if   (  (AC49X_DEVICE_TYPE == AC490_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	HcrfSendInterrupt(Device);
#endif
	ReturnedVal		= HcrfWaitForResponse(Device, &StatusRegister);
	*pResponseValue = (Tac49xHcrfStatusRegisterIndex)MergeFieldsToShort(StatusRegister.StatusIndex);
	if((ReturnedVal==0)&&(*pResponseValue==HCRF_STATUS_REGISTER_INDEX__CPU_REGISTERS_READY))
		Ac49xReadBlock(Device, HCRF__MONITOR_INFO_BUFFER_ADDRESS, pInfo, HCRF__CPU_REGISTERS_BUFFER_SIZE);
	return ReturnedVal;
}

/****************************************************************/
/****************************************************************/
int HcrfGetDspTraceBuffer(int Device, char *pInfo, Tac49xHcrfStatusRegisterIndex *pResponseValue)
{
	Tac49xHcrfStatusRegister StatusRegister;
	int						 ReturnedVal;

	memset((void*)&StatusRegister, (char)0, sizeof(Tac49xHcrfStatusRegister));
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )
	if(HcrfCommandRegisterReady(Device))
		return 1;
#endif
	Ac49xHcrfSetCommandRegister(Device, HCRF_COMMAND_REGISTER_INDEX__GET_TRACE_BUFFER, 0);
#if   (  (AC49X_DEVICE_TYPE == AC490_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	HcrfSendInterrupt(Device);
#endif
	ReturnedVal	  = HcrfWaitForResponse(Device, &StatusRegister);
	*pResponseValue = (Tac49xHcrfStatusRegisterIndex)MergeFieldsToShort(StatusRegister.StatusIndex);
	if((ReturnedVal==0)&&(*pResponseValue==HCRF_STATUS_REGISTER_INDEX__TRACE_BUFFER_READY))
		Ac49xReadBlock(Device, HCRF__MONITOR_INFO_BUFFER_ADDRESS, pInfo, HCRF__TRACE_BUFFER_SIZE);
	return ReturnedVal;
}

/****************************************************************/
void HcrfDeviceCrash(int Device)
{
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )
	if(HcrfCommandRegisterReady(Device))
		return;
#endif
	Ac49xHcrfSetCommandRegister(Device, HCRF_COMMAND_REGISTER_INDEX__CORE_CRASH, 0);
#if   (  (AC49X_DEVICE_TYPE == AC490_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	HcrfSendInterrupt(Device);
#endif
}
/*#endif*/ /*((AC49X_DEVICE_TYPE == AC491_DEVICE) || (AC49X_DEVICE_TYPE == AC490_DEVICE))*/

/****************************************************************/
int HcrfCommandRegisterReady(int Device)
{
	U32 Register;
	int ReadTimeOut=100;
	do	{
		if(ReadTimeOut--==0)
			break;
		Ac49xUserDef_DelayForDeviceResponse();
		Ac49xUserDef_CriticalSection_EnterHpi(Device);
		Ac49xHcrfReadRegister(HCRF_CONTEXT__COMMAND_REGISTER, Device, (char*)&Register);
		Ac49xUserDef_CriticalSection_LeaveHpi(Device);
		}
	while(Register); 
	return Register;
}

/****************************************************************/
int HcrfCompareLocalMemory(int Device, int ReferenceDevice, U32 *pAddress, Tac49xHcrfStatusRegisterIndex *pResponseValue)
{
	Tac49xHcrfStatusRegister StatusRegister;
	int						 ReturnedVal;
	U8						 Address[4];

	memset((void*)&StatusRegister, (char)0, sizeof(Tac49xHcrfStatusRegister));
	if(HcrfCommandRegisterReady(Device))
		return 1;
	Ac49xHcrfSetCommandRegister(Device, HCRF_COMMAND_REGISTER_INDEX__COMPARE_LOCAL_MEMORY_TO_REFERENCE, ReferenceDevice);
	ReturnedVal		= HcrfWaitForResponse(Device, &StatusRegister);
	*pResponseValue = (Tac49xHcrfStatusRegisterIndex)MergeFieldsToShort(StatusRegister.StatusIndex);
	if((ReturnedVal==0)&&(*pResponseValue==HCRF_STATUS_REGISTER_INDEX__LOCAL_MEMORY_TO_REFERENCE_COMPARISON_DIFFER))
		{
		Ac49xReadBlock(Device, HCRF__MONITOR_INFO_BUFFER_ADDRESS, (char *)Address, sizeof(U32));
		*pAddress = (Address[0]<<24) + (Address[1]<<16) + (Address[2]<<8) + (Address[3]);
		}
	else
		*pAddress=0;
	return ReturnedVal;
}

#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )
/****************************************************************/
int HcrfDeviceReload(int Device, int ReferenceDevice, Tac49xHcrfStatusRegisterIndex *pResponseValue)
{
	Tac49xHcrfStatusRegister StatusRegister;
	int						 ReturnedVal;

	memset((void*)&StatusRegister, (char)0, sizeof(Tac49xHcrfStatusRegister));
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )
	if(HcrfCommandRegisterReady(Device))
		return 1;
#endif
	Ac49xHcrfSetCommandRegister(Device, HCRF_COMMAND_REGISTER_INDEX__CORE_RELOAD, ReferenceDevice);
	ReturnedVal		= HcrfWaitForResponse(Device, &StatusRegister);
	*pResponseValue = (Tac49xHcrfStatusRegisterIndex)MergeFieldsToShort(StatusRegister.StatusIndex);
	return ReturnedVal;
}

#elif (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
/****************************************************************/
int HcrfDeviceReload(int Device, char* pProgram,	Tac49xHcrfStatusRegisterIndex *pResponseValue)
{
	Tac49xBootStatusRegister    BootStatusRegister      ;
	Tac49xHcrfStatusRegister	StatusRegister			;
	int							ReturnedVal				;
	Tac49xDeviceControlRegister DeviceControlRegister   ;
	unsigned long               BlockSize               ;
	unsigned long               DspOffset               ;	
    int                         iteration               ; 
	Tac49xProgramDownloadStatus Status               = PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS;
	Tac49xControl               LastBlock            = CONTROL__DISABLE; 
    Tac49xControl               ExternalMemory       = CONTROL__ENABLE; 
	unsigned char               *pTempProgramPointer = (unsigned char*)pProgram;

    const int VERSION_DATA_OFFSET   = 34;
	if(pTempProgramPointer)
		pTempProgramPointer += VERSION_DATA_OFFSET;

	pTempProgramPointer++;
	pTempProgramPointer++;

	memset((void*)&DeviceControlRegister, (char)0, sizeof(Tac49xDeviceControlRegister));        
	Ac49xWriteStruct(0, HPI_MEMORY__DEVICE_CONTROL_REG, &DeviceControlRegister);   
    while(!LastBlock)  /* Download DSP Code to External Memory */
		{
		if(pTempProgramPointer[0] & PROGRAM_DOWLOAD__LAST_BLOCK_FLAG)   
			LastBlock = CONTROL__ENABLE;
        if(!(pTempProgramPointer[1] & PROGRAM_DOWLOAD__EXTERNAL_MEMORY_INTERFACE_FLAG))     /*if the current block should be written to internal DSP memory */
            ExternalMemory = CONTROL__DISABLE;
		DspOffset =  (unsigned long)((*(pTempProgramPointer++) & 0x7F) << 24);
		DspOffset += (unsigned long)((*(pTempProgramPointer++)) << 16);
		DspOffset += (unsigned long)((*(pTempProgramPointer++)) << 8);
		DspOffset += (unsigned long)((*(pTempProgramPointer++)));
        DspOffset <<= DSP_TO_HOST_ADDRESS_SHIFT; 

        BlockSize =  (unsigned long)((*(pTempProgramPointer++)) << 8);
		BlockSize += (unsigned long)((*(pTempProgramPointer++)));
		pTempProgramPointer++;/*Dummy*/
		pTempProgramPointer++;/*Dummy*/
        BlockSize <<= DSP_TO_HOST_ADDRESS_SHIFT; 
        if(ExternalMemory)
			Ac49xUserDef_EMIF_MEMCPY(DspOffset, pTempProgramPointer, BlockSize);
        else
			Ac49xWriteBlock(0, DspOffset, pTempProgramPointer, BlockSize);
		pTempProgramPointer += BlockSize;               
		} /*while(!LastBlock): end of program blocks writing Loop */

	Ac49xHcrfSetCommandRegister(Device, HCRF_COMMAND_REGISTER_INDEX__CORE_RELOAD, 0);

	memset((void*)&BootStatusRegister, (char)0, sizeof(Tac49xBootStatusRegister));
	Ac49xWriteStruct(0, HPI_MEMORY__BOOT_STATUS_REG, &BootStatusRegister);

	HcrfSendInterrupt(Device);
	ReturnedVal		= HcrfWaitForResponse(Device, &StatusRegister);
	*pResponseValue = (Tac49xHcrfStatusRegisterIndex)MergeFieldsToShort(StatusRegister.StatusIndex);
	if(ReturnedVal != 0)
		return 1;

    for(iteration=1; iteration<MAX_DELAY_ITERATIONS; iteration++)
		{
		Status = PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS;
		Ac49xReadStruct(0, HPI_MEMORY__BOOT_STATUS_REG, &BootStatusRegister);
		Status *= (Tac49xProgramDownloadStatus)BootStatusRegister.Status;
	    Ac49xUserDef_DelayForDeviceResponse();
		if(Status != PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_NOT_READY)
			break;
		}
	if(Status!=PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS)
		return 1;
	return 0;
}
#endif /*(AC49X_DEVICE_TYPE == AC494_DEVICE) || (AC49X_DEVICE_TYPE == AC495_DEVICE) || (AC49X_DEVICE_TYPE == AC496_DEVICE) || (AC49X_DEVICE_TYPE == AC496E_DEVICE) || (AC49X_DEVICE_TYPE == AC497_DEVICE)*/

/****************************************************************/
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )
int  Ac49xHcrfDeviceReload(int Device, int ReferenceDevice,	Tac49xHcrfStatusRegisterIndex *pResponseValue)
{
	Tac49xHcrfStatusRegisterIndex ResponseValue       = (Tac49xHcrfStatusRegisterIndex)0;
	Tac49xHcrfStatusRegisterIndex ReloadResponseValue = (Tac49xHcrfStatusRegisterIndex)0;
	if(HcrfDeviceHalt(ReferenceDevice, &ResponseValue))
		return 1;
	if(ResponseValue != HCRF_STATUS_REGISTER_INDEX__HALT_ACKNOWLEDGE)
		return 1;
	if(HcrfDeviceReload(Device, ReferenceDevice, &ReloadResponseValue))
		return 1;
	if((ReloadResponseValue != HCRF_STATUS_REGISTER_INDEX__RELOAD_COMPLETED) && (ReloadResponseValue != HCRF_STATUS_REGISTER_INDEX__RELOAD_FAILED))
		return 1;
	if(HcrfDeviceResume(ReferenceDevice, &ResponseValue))
		return 1;
	if(ResponseValue != HCRF_STATUS_REGISTER_INDEX__RESUME_ACKNOWLEDGE)
		return 1;
	*pResponseValue = ReloadResponseValue;
	return 0;
}

/****************************************************************/
#elif (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
int  Ac49xHcrfDeviceReload(int Device, char* pProgram,	Tac49xHcrfStatusRegisterIndex *pResponseValue)
{
	Tac49xHcrfStatusRegisterIndex ResponseValue=(Tac49xHcrfStatusRegisterIndex)0;

	if(HcrfDeviceHalt(Device, &ResponseValue))
		return 1;
	if(ResponseValue != HCRF_STATUS_REGISTER_INDEX__HALT_ACKNOWLEDGE)
		return 1;

	if(HcrfDeviceReload(Device, pProgram, &ResponseValue))
		return 1;
	if((ResponseValue != HCRF_STATUS_REGISTER_INDEX__RELOAD_COMPLETED) && (ResponseValue != HCRF_STATUS_REGISTER_INDEX__RELOAD_FAILED))
		return 1;
	*pResponseValue = ResponseValue;
	return 0;
}
#endif

/****************************************************************/
int  Ac49xHcrfCompareLocalMemory(int Device, int ReferenceDevice, U32 *pAddress,  Tac49xHcrfStatusRegisterIndex *pResponseValue)
{
	Tac49xHcrfStatusRegisterIndex ResponseValue = (Tac49xHcrfStatusRegisterIndex)0;
	Tac49xHcrfStatusRegisterIndex CompareLocalMemoryResponseValue = (Tac49xHcrfStatusRegisterIndex)0;

	if(HcrfDeviceHalt(ReferenceDevice, &ResponseValue))
		return 1;
	if(ResponseValue != HCRF_STATUS_REGISTER_INDEX__HALT_ACKNOWLEDGE)
		return 1;
	if(HcrfDeviceHalt(Device, &ResponseValue))
		return 1;
	if(ResponseValue != HCRF_STATUS_REGISTER_INDEX__HALT_ACKNOWLEDGE)
		return 1;
	if(HcrfCompareLocalMemory(Device, ReferenceDevice, pAddress, &CompareLocalMemoryResponseValue))
		return 1;
	if((CompareLocalMemoryResponseValue != HCRF_STATUS_REGISTER_INDEX__LOCAL_MEMORY_TO_REFERENCE_COMPARISON_FIT) && (CompareLocalMemoryResponseValue != HCRF_STATUS_REGISTER_INDEX__LOCAL_MEMORY_TO_REFERENCE_COMPARISON_DIFFER))
		return 1;
	if(HcrfDeviceResume(ReferenceDevice, &ResponseValue))
		return 1;
	if(ResponseValue != HCRF_STATUS_REGISTER_INDEX__RESUME_ACKNOWLEDGE)
		return 1;
	if(HcrfDeviceResume(Device, &ResponseValue))
		return 1;
	if(ResponseValue != HCRF_STATUS_REGISTER_INDEX__RESUME_ACKNOWLEDGE)
		return 1;
	*pResponseValue = CompareLocalMemoryResponseValue;
	return 0;
}

#if (	(AC49X_DEVICE_TYPE == AC490_DEVICE)\
	||	(AC49X_DEVICE_TYPE == AC491_DEVICE)\
	||	(AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	||	(AC49X_DEVICE_TYPE == AC494_DEVICE)\
	||	(AC49X_DEVICE_TYPE == AC495_DEVICE)\
	||	(AC49X_DEVICE_TYPE == AC496_DEVICE)\
	||  (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
	||	(AC49X_DEVICE_TYPE == AC497_DEVICE)\
	)
/****************************************************************/
int  Ac49xHcrfReadMonitorInfo(int Device, char *pCpuRegisters, char *pTraceBuffer, char *pHpiLocalMemory)
{
	Tac49xHcrfStatusRegisterIndex ResponseValue;
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )
	typedef struct
		{
		U32 Bottom;
		U32 Size;
		}  TAddressRange;
	TAddressRange Saram[5] = {	{0x10000,0x1FFFF-0x10000+1},
								{0x20000,0x2FFFF-0x20000+1},
								{0x30000,0x3FFFF-0x30000+1},
								{0x40000,0x4FFFF-0x40000+1},
								{0x50000,0x5FFFF-0x50000+1}
							};

	TAddressRange Daram = {0xC0,0xFFFF-0xC0+1};
	int Offset[6];

	Offset[0] = 0;
	Offset[1] = Daram.Size;
	Offset[2] = Daram.Size+Saram[0].Size;
	Offset[3] = Daram.Size+Saram[0].Size+Saram[1].Size;
	Offset[4] = Daram.Size+Saram[0].Size+Saram[1].Size+Saram[2].Size;
	Offset[5] = Daram.Size+Saram[0].Size+Saram[1].Size+Saram[2].Size+Saram[3].Size;

/*
	int Offset[6] = {	0,
						Daram.Size,

						Daram.Size+Saram[0].Size,
						Daram.Size+Saram[0].Size+Saram[1].Size,
						Daram.Size+Saram[0].Size+Saram[1].Size+Saram[2].Size,
						Daram.Size+Saram[0].Size+Saram[1].Size+Saram[2].Size+Saram[3].Size
					};
*/
#endif



	if(HcrfDeviceHalt(Device, &ResponseValue))
		return 1;
	if(ResponseValue != HCRF_STATUS_REGISTER_INDEX__HALT_ACKNOWLEDGE)
		return 1;

#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )
	Ac49xReadBlock(Device, 0xC0, pHpiLocalMemory, HCRF__LOCAL_MEMORY_SIZE);
/*	Ac49xReadBlock(Device, Daram.Bottom,    &pHpiLocalMemory[Offset[0]], Daram.Size   );*/
/*	Ac49xReadBlock(Device, Saram[0].Bottom, &pHpiLocalMemory[Offset[1]], Saram[0].Size);*/
/*	Ac49xReadBlock(Device, Saram[1].Bottom, &pHpiLocalMemory[Offset[2]], Saram[1].Size);*/
/*	Ac49xReadBlock(Device, Saram[2].Bottom, &pHpiLocalMemory[Offset[3]], Saram[2].Size);*/
/*	Ac49xReadBlock(Device, Saram[3].Bottom, &pHpiLocalMemory[Offset[4]], Saram[3].Size);*/
/*	Ac49xReadBlock(Device, Saram[4].Bottom, &pHpiLocalMemory[Offset[5]], Saram[4].Size);*/
#elif (  (AC49X_DEVICE_TYPE == AC490_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	Ac49xReadBlock(Device, 0xC0, pHpiLocalMemory, HCRF__LOCAL_MEMORY_SIZE);
#endif

	if(HcrfGetCpuRegisters(Device, pCpuRegisters, &ResponseValue))
		return 1;
	if(ResponseValue != HCRF_STATUS_REGISTER_INDEX__CPU_REGISTERS_READY)
		return 1;
	if(HcrfGetDspTraceBuffer(Device, pTraceBuffer, &ResponseValue))
		return 1;
	if(ResponseValue != HCRF_STATUS_REGISTER_INDEX__TRACE_BUFFER_READY)
		return 1;
	if(HcrfDeviceResume(Device, &ResponseValue))
		return 1;
	if(ResponseValue != HCRF_STATUS_REGISTER_INDEX__RESUME_ACKNOWLEDGE)
		return 1;
	return 0;
}

/*
#if (AC49X_DEVICE_TYPE == AC490_DEVICE)	|| (AC49X_DEVICE_TYPE == AC491_DEVICE)
int  Ac49xHcrfReadMonitorInfo(int Device, char *pCpuRegisters, char *pTraceBuffer, char *pHpiLocalMemory)
{
	Tac49xHcrfStatusRegisterIndex ResponseValue;
	if(HcrfDeviceHalt(Device, &ResponseValue))
		return 1;
	if(ResponseValue != HCRF_STATUS_REGISTER_INDEX__HALT_ACKNOWLEDGE)
		return 1;
	Ac49xReadBlock(Device, 0xC0, pHpiLocalMemory, HCRF__LOCAL_MEMORY_SIZE);
	if(HcrfGetCpuRegisters(Device, pCpuRegisters, &ResponseValue))
		return 1;
	if(ResponseValue != HCRF_STATUS_REGISTER_INDEX__CPU_REGISTERS_READY)
		return 1;
	if(HcrfGetDspTraceBuffer(Device, pTraceBuffer, &ResponseValue))
		return 1;
	if(ResponseValue != HCRF_STATUS_REGISTER_INDEX__TRACE_BUFFER_READY)
		return 1;
	if(HcrfDeviceResume(Device, &ResponseValue))
		return 1;
	if(ResponseValue != HCRF_STATUS_REGISTER_INDEX__RESUME_ACKNOWLEDGE)
		return 1;
	return 0;
}
*/
/****************************************************************/
int  Ac49xHcrfWriteMonitorInfo(int Device, int ReferenceDevice, Tac49xHcrfStatusRegisterIndex *pResponseValue)
{/*TBD*/
	return 0;
}
#endif /*(AC49X_DEVICE_TYPE == AC490_DEVICE) || (AC49X_DEVICE_TYPE == AC491_DEVICE)*/

/*****************************\
*******************************
** Device Register Handling  **
*******************************
\*****************************/

/****************************************************************/
int Ac49xDeviceSetControlRegister(int Device, Tac49xDeviceControlRegisterAttr *pDeviceControlRegisterAttr)
{
	int ReadTimeOut=100;
    ac49xDeviceControlRegister[Device].HostCellSize    = pDeviceControlRegisterAttr->HostCellSize   ;
    ac49xDeviceControlRegister[Device].NetworkCellSize = pDeviceControlRegisterAttr->NetworkCellSize;
    ac49xDeviceControlRegister[Device].HostDebug       = pDeviceControlRegisterAttr->HostDebug      ;
    ac49xDeviceControlRegister[Device].NetworkDebug    = pDeviceControlRegisterAttr->NetworkDebug   ;
    ac49xDeviceControlRegister[Device].Reset           = pDeviceControlRegisterAttr->Reset          ;
    Ac49xWriteStruct(Device, HPI_MEMORY__DEVICE_CONTROL_REG, &ac49xDeviceControlRegister[Device]);

	do	{
		if(ReadTimeOut--==0)
			break;
		Ac49xUserDef_DelayForDeviceResponse();
		Ac49xReadStruct(Device, HPI_MEMORY__DEVICE_CONTROL_REG, &ac49xDeviceControlRegister[Device]);
		}
	while(ac49xDeviceControlRegister[Device].Reset);	 
	return (ac49xDeviceControlRegister[Device].Reset) ? RESET_DEVICE_FAILED : 0;	 
}


/*********************************/                                                         
/*********************************/                                                        
/**   Block Handling Routines   **/
/*********************************/
/*********************************/
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )
    #if (AC49X_ENDIAN_MODE==LITTLE_ENDIAN)
        #define AddressBytesSwapping(Address)           \
                        ( ((Address&0x00FF0000) << 8)   \
                        | ((Address&0xFF000000) >> 8)   \
                        | ((Address&0x000000FF) << 8)   \
                        | ((Address&0x0000FF00) >> 8)   \
                        )								
    #else
        #define AddressBytesSwapping(Address) Address	
    #endif
#endif

#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
    )
    #define	Ac49xSetAddress(Device, Address)    																	   	        \
				    Address = (Address+(Device%NUMBER_OF_DEVICES_PER_DSP)*(AC49XIF_DEVICE_OFFSET<<DSP_TO_HOST_ADDRESS_SHIFT));  \
                    Address = AddressBytesSwapping(Address);                                                                    \
                    ioctl(g_ac491_dev_fd, AC491_IOCTL_SET_ADDR, Address);
			 //Ac49xUserDef_WriteShort(Device, HPIA1st, (unsigned short)(Address>>16)); 
                    //Ac49xUserDef_WriteShort(Device, HPIA2nd, (unsigned short)Address      );
#endif /* AC49XHPI_TYPE */

/****************************************************************/
void Ac49xHcrfWriteRegister(Tac49x_HcrfContext HcrfContext, int Device, char *pOut4Bytes)
{
	U32 Address = (HcrfContext == HCRF_CONTEXT__COMMAND_REGISTER) ? ac49xHcrfCommandRegisterAddress[Device] : ac49xHcrfStatusRegisterAddress[Device];
#if   (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	long *pTempPointer = (long*)pOut4Bytes; 
	Ac49xUserDef_WriteLong(Device, Address, *pTempPointer);
#elif (AC49X_DEVICE_TYPE==AC490_DEVICE)
	short *pTempPointer = (short*)pOut4Bytes; 
	Ac49xUserDef_WriteShort(Device, Address, *pTempPointer++);
	Ac49xUserDef_WriteShort(Device, Address+2, *pTempPointer++);
#elif (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	  )
	short *pTempPointer = (short*)pOut4Bytes; 
    Address = AddressBytesSwapping(Address);                                
    Ac49xUserDef_WriteShort(Device, HPIA1st, (unsigned short)(Address>>16));
    Ac49xUserDef_WriteShort(Device, HPIA2nd, (unsigned short)Address      );
	Ac49xUserDef_WriteShort(Device, HPID1st, *pTempPointer++);
	Ac49xUserDef_WriteShort(Device, HPID2nd, *pTempPointer++);
#endif
}

/****************************************************************/
void Ac49xHcrfReadRegister(Tac49x_HcrfContext HcrfContext, int Device, char *pIn4Bytes)
{
	U32 Address = (HcrfContext == HCRF_CONTEXT__COMMAND_REGISTER) ? ac49xHcrfCommandRegisterAddress[Device] : ac49xHcrfStatusRegisterAddress[Device];
#if   (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	long *pTempPointer = (long*)pIn4Bytes;   
	*pTempPointer = Ac49xUserDef_ReadLong(Device, (Address));
#elif (AC49X_DEVICE_TYPE==AC490_DEVICE)
	short *pTempPointer = (short*)pIn4Bytes;   
	*(pTempPointer++) = Ac49xUserDef_ReadShort(Device, (Address));
	*(pTempPointer++) = Ac49xUserDef_ReadShort(Device, (Address+2));
#elif (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	  )
	short *pTempPointer = (short*)pIn4Bytes;   
	Address = AddressBytesSwapping(Address);
	Ac49xUserDef_WriteShort(Device, HPIA1st, (unsigned short)(Address>>16));                                    \
    Ac49xUserDef_WriteShort(Device, HPIA2nd, (unsigned short)Address      );
	*(pTempPointer++) = Ac49xUserDef_ReadShort(Device, HPID1st);
	*(pTempPointer++) = Ac49xUserDef_ReadShort(Device, HPID2nd);
#endif
}

/*--------------------------------------------------------------------
 * ROUTINE:   Ac49xWriteBlock()
 *
 * DESCRIPTION:
 *   Writes a block from the host memory to the DSP's (Host ==> DSP)
 *
 * ARGUMENTS:
 *   int Device               - serial number of AC49x device
 *   int BlockAddress		  - destination block address (bytes) relative to the device's base address
 *   char *OutBlock			  - pointer to the source memory location (host)
 *   int NumberOfBytesToWrite - number of bytes to write
 *
 *
 * RESULTS:
 *   none
 * 
 * ALGORITHM :
 * - Use the appropriate method (compilation switch) to write the block from
 *   the host memory into the device. (see inside code documentation for the
 *   various options).
 * 
 * NOTES :
 * - This function can be defined as _inline for better optimization.
 * - If NumberOfBytesToWrite is an odd number, Ac49xWriteBlock will write an extra
 *   0 byte at the end of the block. The total number of bytes written will
 *   always be an even number !
 *
 * MODIFICATIONS :
 * 
 *--------------------------------------------------------------------*/
void Ac49xWriteBlock(int Device, U32 BlockAddress, char *pOutBlock, int NumberOfBytesToWrite)
{
	int	  i;	
#if   (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	U32 *pTempPointer = (U32*)pOutBlock;
	for(i=0; i<(NumberOfBytesToWrite); i+=4)  \
    {
		Ac49xUserDef_WriteLong(Device, (BlockAddress+i), *(pTempPointer));
    pTempPointer++;
    }
#elif (AC49X_DEVICE_TYPE==AC490_DEVICE)
	short *pTempPointer = (short*)pOutBlock;
	short LastWord;	
	for(i=0; i<(NumberOfBytesToWrite-1); i+=2)
		Ac49xUserDef_WriteShort(Device, (BlockAddress+i), *(pTempPointer++));
	if(NumberOfBytesToWrite & 0x1)									/* If the number of bytes to be	   */ 
        	{   														/* written is an odd number:	   */
		LastWord = *pTempPointer;									/* Copy the last byte in the block */
		*( ((char*)&LastWord)+ 1)  = 0;								/* Add the zero byte			   */
		Ac49xUserDef_WriteShort(Device, (BlockAddress+i), LastWord);/* Write the two bytes :		   */
	   	}
#elif (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	  )
	short *pTempPointer = (short*)pOutBlock;
	short LastTwoShorts[2] = {0,0}; 
	char *pLastTwoShortsAsBytes;
	char *pTempPointerAsChars; 
	Ac49xSetAddress(Device, BlockAddress);
	for(i=0; i<(NumberOfBytesToWrite-3); i+=4)	 /* Write the requested block in shorts (as required by the Device HPI) */
		{
		Ac49xUserDef_WriteShort(Device, HPIDA1st, *(pTempPointer++));
		Ac49xUserDef_WriteShort(Device, HPIDA2nd, *(pTempPointer++));
		}
	NumberOfBytesToWrite -= i;
	if(NumberOfBytesToWrite>0)	 /* If the number of bytes to be written is an 1|2|3 Mod(4) number, */
		{
		/*memcpy(LastTwoShorts, pTempPointer, NumberOfBytesToWrite);*/
		pTempPointerAsChars   = (char*)pTempPointer;
		pLastTwoShortsAsBytes = (char*)LastTwoShorts;
		for(i=0; i<NumberOfBytesToWrite; i++)
			pLastTwoShortsAsBytes[i] = *pTempPointerAsChars++;	
		Ac49xUserDef_WriteShort(Device, HPIDA1st, LastTwoShorts[0]);
		Ac49xUserDef_WriteShort(Device, HPIDA2nd, LastTwoShorts[1]);
		}
#endif
}


/*--------------------------------------------------------------------
 * ROUTINE:   Ac49xReadBlock()
 *
 * DESCRIPTION:
 *   Reads a block from the DSP to the host's memory (DSP ==> Host)
 *
 * ARGUMENTS:
 *   int Device             - serial number of AC49x device
 *   int BlockAddress		- source block address relative to the device's base address (in bytes)
 *   char *InBlock			- pointer to the destination memory location (host)
 *   int NumberOfBytesToRead- number of bytes to read
 *
 *
 * RESULTS:
 *   none
 * 
 * ALGORITHM :
 * - Use the appropriate method (compilation switch) to read the block from
 *   the device into the host memory (see inside code documentation for the
 *   various options).
 * 
 * NOTES :
 * - This function can defined as _inline for better optimization.
 *
 * MODIFICATIONS :
 * 
 *--------------------------------------------------------------------*/
void Ac49xReadBlock(int Device, U32 BlockAddress, char *pInBlock, int NumberOfBytesToRead)
{
	int	  i;	 
#if   (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	long *pTempPointer = (long*)pInBlock;   
	for(i=0; i<(NumberOfBytesToRead); i+=4)
		*(pTempPointer++) = Ac49xUserDef_ReadLong(Device, (BlockAddress+i));
#elif (AC49X_DEVICE_TYPE==AC490_DEVICE)
	short *pTempPointer = (short*)pInBlock;   
	short LastWord;	
	for(i=0; i<(NumberOfBytesToRead-1); i+=2)
		*(pTempPointer++) = Ac49xUserDef_ReadShort(Device, (BlockAddress+i));
	if(NumberOfBytesToRead & 0x1)						/* If the number of bytes	   */
        {												/* to be read is an odd number */
		LastWord = Ac49xUserDef_ReadShort(Device, (BlockAddress+i));
		*((char*)pTempPointer) = *((char*)&LastWord);	/* Copy the last BYTE		   */
	    }
#elif (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	  )
	short *pTempPointer = (short*)pInBlock;   
	Ac49xSetAddress(Device, BlockAddress);
	for(i=0; i<(NumberOfBytesToRead); i+=4)	  /* Read the requested block in shorts (as required by the AC491xxx HPI) */
		{
		*(pTempPointer++) = Ac49xUserDef_ReadShort(Device, HPIDA1st);
		*(pTempPointer++) = Ac49xUserDef_ReadShort(Device, HPIDA2nd);
		}
#endif
}

/*--------------------------------------------------------------------
 * ROUTINE:   Ac49xWrite4Bytes()
 *
 * DESCRIPTION:
 *   Writes a 4 Bytes from the host memory to the DSP's (Host ==> DSP)
 *   This Function was created for the Multiplex 16 Access
 *   To short the HREADY Pulse From DSP To PLX BRIDG due to Post/Pre Increment
 *   This is done by writting through the data registers without an Pre/Post Increment.
 *
 * ARGUMENTS:
 *   int Device               - serial number of AC49x device
 *   int BlockAddress		  - destination block address (bytes) relative to the device's base address
 *   char *Out4Bytes		  - pointer to the source memory location (host)
 *
 *
 * RESULTS:
 *   none
 * 
 * ALGORITHM :
 * - Use the appropriate method (compilation switch) to write the block from
 *   the host memory into the device. (see inside code documentation for the
 *   various options).
 * 
 * NOTES :
 * - This function can be defined as _inline for better optimization.
 *
 * MODIFICATIONS :
 * 
 *--------------------------------------------------------------------*/
void Ac49xWrite4Bytes(int Device, U32 BlockAddress, char *pOut4Bytes)
{

#if   (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	long *pTempPointer = (long*)pOut4Bytes;
	Ac49xUserDef_WriteLong(Device, BlockAddress, *pTempPointer);
#elif (AC49X_DEVICE_TYPE==AC490_DEVICE)
	short *pTempPointer = (short*)pOut4Bytes; 
	Ac49xUserDef_WriteShort(Device, BlockAddress,   *pTempPointer++);
	Ac49xUserDef_WriteShort(Device, BlockAddress+2, *pTempPointer);
#elif (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	  )
	short *pTempPointer = (short*)pOut4Bytes; 
	Ac49xSetAddress(Device, BlockAddress);
	Ac49xUserDef_WriteShort(Device, HPID1st, *pTempPointer++);
	Ac49xUserDef_WriteShort(Device, HPID2nd, *pTempPointer);
#endif
}

/*--------------------------------------------------------------------
 * ROUTINE:   Ac49xRead4Bytes()
 *
 * DESCRIPTION:
 *   Writes a 4 Bytes from the host memory to the DSP's (Host ==> DSP)
 *   This Function was created for the Multiplex 16 Access
 *   To short the HREADY Pulse From DSP To PLX BRIDG due to Post/Pre Increment
 *   This is done by reading from the data registers without an Pre/Post Increment.
 *
 * ARGUMENTS:
 *   int Device               - serial number of AC49x device
 *   int BlockAddress		  - destination block address (bytes) relative to the device's base address
 *   char *Out4Bytes		  - pointer to the source memory location (host)
 *
 *
 * RESULTS:
 *   none

 * 
 * ALGORITHM :
 * - Use the appropriate method (compilation switch) to write the block from
 *   the host memory into the device. (see inside code documentation for the
 *   various options).
 * 
 * NOTES :
 * - This function can be defined as _inline for better optimization.
 *
 * MODIFICATIONS :
 * 
 *--------------------------------------------------------------------*/
void Ac49xRead4Bytes(int Device, U32 BlockAddress, char *pIn4Bytes)
{
#if   (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	long *pTempPointer = (long*)pIn4Bytes;   
	*pTempPointer = Ac49xUserDef_ReadLong(Device, BlockAddress);
#elif (AC49X_DEVICE_TYPE==AC490_DEVICE)
	short *pTempPointer = (short*)pIn4Bytes;   
	*(pTempPointer++) = Ac49xUserDef_ReadShort(Device, (BlockAddress));
	*pTempPointer   = Ac49xUserDef_ReadShort(Device, (BlockAddress+2));
#elif (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	  )
	short *pTempPointer = (short*)pIn4Bytes;   
	Ac49xSetAddress(Device, BlockAddress);
	*(pTempPointer++) = Ac49xUserDef_ReadShort(Device, HPID1st);
	*pTempPointer   = Ac49xUserDef_ReadShort(Device, HPID2nd);
#endif
}

/*********************************/
/*********************************/                                                        
/**  Packet Handling Routines   **/
/*********************************/
/*********************************/


/*//////////////    F I F O   /////////////////////*/
#if (FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__NETWORK)   

int ac491SendData(int Device, int Channel,U8 *Buffer, U16 Size, Tac49xProtocol Protocol)
{
	int temp_size = Size;
    //Tac49xAdvancedChannelConfigurationPacket Packet;
    Tac49xPacket Packet;
	Tac49xTxPacketParams TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Tac49xAdvancedChannelConfigurationPacket));
	memcpy((void *)Packet.u.Payload, (void *)Buffer, temp_size);
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Tac49xAdvancedChannelConfigurationPacket), TRANSFER_MEDIUM__HOST);
#endif

	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Tac49xHpiPacketHeader) + Size;
	TxPacketParams.Protocol				= Protocol;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__NETWORK;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
}

/****************************************************************/
void Ac49xDeviceInitFifo(int Device)
{
	#if(FIFO_USAGE_ENABLE__HOST)
   	Ac49xUserDef_CriticalSection_EnterFifo(Device, TRANSFER_MEDIUM__HOST);
    ac49xHostFifo[Device].Head              = 0;
    ac49xHostFifo[Device].Tail              = 0;
    ac49xHostFifo[Device].ResidentCounter   = 0;
    ac49xHostFifo[Device].MaxUsage          = 0;
  	Ac49xUserDef_CriticalSection_LeaveFifo(Device, TRANSFER_MEDIUM__HOST);
    #endif /*(FIFO_USAGE_ENABLE__HOST)*/
    #if(FIFO_USAGE_ENABLE__NETWORK)
	Ac49xUserDef_CriticalSection_EnterFifo(Device, TRANSFER_MEDIUM__NETWORK);
    ac49xNetFifo[Device].Head              = 0;
    ac49xNetFifo[Device].Tail              = 0;
    ac49xNetFifo[Device].ResidentCounter   = 0;
    ac49xNetFifo[Device].MaxUsage          = 0;
  	Ac49xUserDef_CriticalSection_LeaveFifo(Device, TRANSFER_MEDIUM__NETWORK);
    #endif /*(FIFO_USAGE_ENABLE__NETWORK)*/
}

/****************************************************************/
void Ac49xInitFifo(void)
{
    int Device;
    for(Device=0; Device<AC49X_NUMBER_OF_DEVICES; Device++)
		Ac49xDeviceInitFifo(Device);
}

/****************************************************************/
U16 Ac49xGetResidentCellNumber(int Device, Tac49xTransferMedium TransferMedium)
{
    U16 ResidentCounter=0;

    if(  (FIFO_USAGE_ENABLE__HOST    && TransferMedium==TRANSFER_MEDIUM__HOST)
      || (FIFO_USAGE_ENABLE__NETWORK && TransferMedium==TRANSFER_MEDIUM__NETWORK)
      )
        {
        if(TransferMedium == TRANSFER_MEDIUM__HOST)
            {
            #if(FIFO_USAGE_ENABLE__HOST)
  	        
            ResidentCounter = ac49xHostFifo[Device].ResidentCounter;
            if(ResidentCounter > ac49xHostFifo[Device].MaxUsage)
                ac49xHostFifo[Device].MaxUsage = ResidentCounter;
  	        
            #endif /*(FIFO_USAGE_ENABLE__HOST)*/
            }
        else
            {
            #if(FIFO_USAGE_ENABLE__NETWORK)
	        
            ResidentCounter = ac49xNetFifo[Device].ResidentCounter;
            if(ResidentCounter > ac49xNetFifo[Device].MaxUsage)
                ac49xNetFifo[Device].MaxUsage = ResidentCounter;
  	        
            #endif /*(FIFO_USAGE_ENABLE__NETWORK)*/
            }
        }
    return ResidentCounter;
}

/****************************************************************/
void Ac49xResetMaxUsage(void)
{    
    int Device;
    for(Device=0; Device<AC49X_NUMBER_OF_DEVICES; Device++)
		{
#if(FIFO_USAGE_ENABLE__HOST)
		
        ac49xHostFifo[Device].MaxUsage = 0;
		
#endif /*(FIFO_USAGE_ENABLE__HOST)*/
#if(FIFO_USAGE_ENABLE__NETWORK)
		
        ac49xNetFifo[Device].MaxUsage = 0;
		
#endif/*(FIFO_USAGE_ENABLE__NETWORK)*/
		}

}

/****************************************************************/
float Ac49xGetMaxUsage(int Device, Tac49xTransferMedium TransferMedium)
{
	if (TransferMedium==TRANSFER_MEDIUM__HOST)
        {
        #if(FIFO_USAGE_ENABLE__HOST)
		return (float)ac49xHostFifo[Device].MaxUsage/FIFO_SIZE__HOST;
        #endif/*(FIFO_USAGE_ENABLE__HOST)*/
        }
	else
        {
        #if(FIFO_USAGE_ENABLE__NETWORK)
    	return (float)ac49xNetFifo[Device].MaxUsage/FIFO_SIZE__NETWORK;
        #endif/*(FIFO_USAGE_ENABLE__NETWORK)*/
        }
    return 0;

}

/****************************************************************/
Tac49xFifoActionResult Ac49xEnqueueCell(int Device, Tac49xTransferMedium TransferMedium, char *pCell, int Size, int Partial)
{
    U16 CellId;

	if (TransferMedium==TRANSFER_MEDIUM__HOST)
		{
#if (FIFO_USAGE_ENABLE__HOST)
		if(ac49xHostFifo[Device].ResidentCounter == FIFO_SIZE__HOST)    
			return FIFO_ACTION_RESULT__FIFO_IS_FULL_ERROR;
		CellId = ac49xHostFifo[Device].Head;
					/* insert Data */
		ac49xHostFifo[Device].Cell[CellId].Size    = Size;
		ac49xHostFifo[Device].Cell[CellId].Partial = Partial;
		memcpy(ac49xHostFifo[Device].Cell[CellId].Buffer, pCell, Size);
					/* Advance 'Head' for next Enqueue */
		CellId++;
		CellId &= (FIFO_SIZE__HOST-1);
		ac49xHostFifo[Device].Head = CellId;
		ac49xHostFifo[Device].ResidentCounter++;
#endif
		}
	else	/* TransferMedium==AC49X_TRANSFER_MEDIUM__NETWORK */
		{
#if (FIFO_USAGE_ENABLE__NETWORK)
		if(ac49xNetFifo[Device].ResidentCounter == FIFO_SIZE__NETWORK)    
			return FIFO_ACTION_RESULT__FIFO_IS_FULL_ERROR;
		CellId = ac49xNetFifo[Device].Head;
					/* insert Data */
		ac49xNetFifo[Device].Cell[CellId].Size    = Size;
		ac49xNetFifo[Device].Cell[CellId].Partial = Partial;
		memcpy(ac49xNetFifo[Device].Cell[CellId].Buffer, pCell, Size);
					/* Advance 'Head' for next Enqueue */
		CellId++;
		CellId &= (FIFO_SIZE__NETWORK-1);
		ac49xNetFifo[Device].Head = CellId;
		ac49xNetFifo[Device].ResidentCounter++;
#endif
		}
    return FIFO_ACTION_RESULT__SUCCEEDED;
}

/****************************************************************/
int Ac49xTransmitCell(int Device, Tac49xTransferMedium TransferMedium)
{
	Tac49xBufferDescriptor  BufferDescriptor;			  
    U32                     BufferDescriptorBaseAddress;
	int	                    BufferDescriptorCurrentIndex;
	unsigned int            PacketAddress;
	int                     Partial;                
    U16                     Size;                   
    U8                      *Buffer; 
	
    if(  !(FIFO_USAGE_ENABLE__HOST    && TransferMedium==TRANSFER_MEDIUM__HOST)
      && !(FIFO_USAGE_ENABLE__NETWORK && TransferMedium==TRANSFER_MEDIUM__NETWORK)
      )
      return 0;

	Ac49xUserDef_CriticalSection_EnterHpi(Device);

	if (TransferMedium==TRANSFER_MEDIUM__HOST)
		{
#if(FIFO_USAGE_ENABLE__HOST)
		Partial		= ac49xHostFifo[Device].Cell[ac49xHostFifo[Device].Tail].Partial;
		Size		= ac49xHostFifo[Device].Cell[ac49xHostFifo[Device].Tail].Size;	
		Buffer		= ac49xHostFifo[Device].Cell[ac49xHostFifo[Device].Tail].Buffer;	
#endif/*(FIFO_USAGE_ENABLE__HOST)*/
		}
	else
		{
#if(FIFO_USAGE_ENABLE__NETWORK)
		Partial		= ac49xNetFifo[Device].Cell[ac49xNetFifo[Device].Tail].Partial;
		Size		= ac49xNetFifo[Device].Cell[ac49xNetFifo[Device].Tail].Size;	
		Buffer		= ac49xNetFifo[Device].Cell[ac49xNetFifo[Device].Tail].Buffer;
#endif/*(FIFO_USAGE_ENABLE__NETWORK)*/
		}

	BufferDescriptorCurrentIndex = (TransferMedium == TRANSFER_MEDIUM__HOST)? ac49xHostBufferDescriptorCurrentIndex[Device].Tx : ac49xNetworkBufferDescriptorCurrentIndex[Device].Tx;
    BufferDescriptorBaseAddress  = (TransferMedium == TRANSFER_MEDIUM__HOST)? HPI_MEMORY__HOST_TX_BUFFER_DESCRIPTOR : HPI_MEMORY__NETWORK_TX_BUFFER_DESCRIPTOR; 
    Ac49xReadBufferDescriptor(Device, BufferDescriptorBaseAddress, BufferDescriptorCurrentIndex, &BufferDescriptor);                                                                             
    if(!BufferDescriptor.Empty)                                                                                                  
        {
		Ac49xUserDef_CriticalSection_LeaveHpi(Device);
		return BUFFER_DESCRIPTOR_FULL_ERROR;                                                                                                                 
        }
    PacketAddress   = MergeFieldsToShort(BufferDescriptor.PacketAddress);                                                                                        
    PacketAddress  += (AC49XIF_BASE_PAGE<<16); 
	PacketAddress <<= DSP_TO_HOST_ADDRESS_SHIFT; 
	if(TransferMedium == TRANSFER_MEDIUM__HOST)
		{
		if(BufferDescriptorCurrentIndex<0 || BufferDescriptorCurrentIndex>=NUMBER_OF_HOST_TX_BUFFER_DESCRIPTORS)
			{
			Ac49xUserDef_CriticalSection_LeaveHpi(Device);
			return TX_BD_INDEX_ERROR;
			}
		}
	else
		{
		if(BufferDescriptorCurrentIndex<0 || BufferDescriptorCurrentIndex>=NUMBER_OF_NETWORK_TX_BUFFER_DESCRIPTORS)
			{
			Ac49xUserDef_CriticalSection_LeaveHpi(Device);
			return TX_BD_INDEX_ERROR;
			}
		}
	
	if(TransferMedium == TRANSFER_MEDIUM__HOST)
		{
		if((PacketAddress < HPI_MEMORY__HOST_TX_BUFFER) || ((Size+PacketAddress) > (HPI_MEMORY__HOST_TX_BUFFER+HOST_TX_BUFFER_SIZE))) 
			{
			Ac49xUserDef_CriticalSection_LeaveHpi(Device);
			return TX_BAD_PACKER_ADDRESS_ERROR;                                                                          	                                     
			}
		}
	else
		{
		if((PacketAddress < HPI_MEMORY__NETWORK_TX_BUFFER) || ((Size+PacketAddress) > (HPI_MEMORY__NETWORK_TX_BUFFER+NETWORK_TX_BUFFER_SIZE))) 
			{
			Ac49xUserDef_CriticalSection_LeaveHpi(Device);
			return TX_BAD_PACKER_ADDRESS_ERROR;                                                                          	                                     
			}
		}
#if ALLOW_DEBUG_MODE 
    /* Add more code here ..............*/
#endif /* AC49x_ALLOW_DEBUG_MODE */
    Ac49xWriteBlock(Device, PacketAddress, (char *)Buffer, Size); /* Copy Cell to DSP */
                  /* Fill in and Write back the Buffer Descriptor to the DSP memory */
    SplitFieldsFromShort(BufferDescriptor.PacketSize, Size);                                           
    BufferDescriptor.Partial = Partial;                                                                                                         
	BufferDescriptor.Empty   = CONTROL__DISABLE;	 
    Ac49xWriteBufferDescriptor(Device, BufferDescriptorBaseAddress, BufferDescriptorCurrentIndex, &BufferDescriptor);  
                 /* Update the global Buffer Descriptor Current Index */
    if(BufferDescriptor.Wrap)                                                                                                      
		BufferDescriptorCurrentIndex = 0;                                                                                                                  
	else                                                                                                                                          
		BufferDescriptorCurrentIndex++;
	if(TransferMedium == TRANSFER_MEDIUM__HOST)
		ac49xHostBufferDescriptorCurrentIndex[   Device].Tx = BufferDescriptorCurrentIndex;	                                                              
	else
		ac49xNetworkBufferDescriptorCurrentIndex[Device].Tx = BufferDescriptorCurrentIndex;	                                                              
	Ac49xUserDef_CriticalSection_LeaveHpi(Device);
	return 0;                                                                                                                
}

/****************************************************************/
Tac49xFifoActionResult Ac49xDequeueAndTransmitCell(int Device, Tac49xTransferMedium TransferMedium)
{
    Ac49xUserDef_CriticalSection_EnterFifo(Device, TransferMedium);
    if(
#if (FIFO_USAGE_ENABLE__HOST)
		  (TransferMedium==TRANSFER_MEDIUM__HOST    && !ac49xHostFifo[Device].ResidentCounter)
#else /*(AC45X_FIFO_USAGE_ENABLE__HOST)*/
        0
#endif      
#if (FIFO_USAGE_ENABLE__NETWORK)
        ||(TransferMedium==TRANSFER_MEDIUM__NETWORK && !ac49xNetFifo[Device].ResidentCounter)
#endif /*(FIFO_USAGE_ENABLE__NETWORK)*/
	  ) 
        {
        Ac49xUserDef_CriticalSection_LeaveFifo(Device, TransferMedium);
        return FIFO_ACTION_RESULT__FIFO_IS_EMPTY_ERROR;
        }

    if(Ac49xTransmitCell(Device, TransferMedium) == BUFFER_DESCRIPTOR_FULL_ERROR)
        {
        Ac49xUserDef_CriticalSection_LeaveFifo(Device, TransferMedium);
        return FIFO_ACTION_RESULT__BUFFER_DESCRIPTOR_FULL_ERROR;      
        }
                /* Advance 'Tail' for next Dequeue */

	if(TransferMedium==TRANSFER_MEDIUM__HOST)
		{
#if(FIFO_USAGE_ENABLE__HOST)
		ac49xHostFifo[Device].Tail++;
		ac49xHostFifo[Device].Tail &= (FIFO_SIZE__HOST-1);
		ac49xHostFifo[Device].ResidentCounter--;
#endif /*(FIFO_USAGE_ENABLE__HOST)*/
		}
	else /* TransferMedium==AC49x_TRANSFER_MEDIUM__NETWORK */
		{
#if(FIFO_USAGE_ENABLE__NETWORK)
		ac49xNetFifo[Device].Tail++;
		ac49xNetFifo[Device].Tail &= (FIFO_SIZE__NETWORK-1);
		ac49xNetFifo[Device].ResidentCounter--;
#endif /*(FIFO_USAGE_ENABLE__NETWORK)*/
		}
    Ac49xUserDef_CriticalSection_LeaveFifo(Device, TransferMedium);
    return FIFO_ACTION_RESULT__SUCCEEDED;
}

/****************************************************************/
void Ac49xFifoFlush(int Device, Tac49xTransferMedium TransferMedium) 
{
    Tac49xFifoActionResult FifoActionResult;
    if(  !(FIFO_USAGE_ENABLE__HOST    && TransferMedium==TRANSFER_MEDIUM__HOST)
      && !(FIFO_USAGE_ENABLE__NETWORK && TransferMedium==TRANSFER_MEDIUM__NETWORK)
      )
      return;
    while(1)
        {  
        FifoActionResult = Ac49xDequeueAndTransmitCell(Device, TransferMedium);
        switch(FifoActionResult)
            {
			case(FIFO_ACTION_RESULT__SUCCEEDED):
			case(FIFO_ACTION_RESULT__FIFO_IS_FULL_ERROR):
				break;
            case(FIFO_ACTION_RESULT__FIFO_IS_EMPTY_ERROR):
                return;
            case(FIFO_ACTION_RESULT__BUFFER_DESCRIPTOR_FULL_ERROR):
                Ac49xUserDef_DelayForDeviceResponse();
            }
        }
}
#endif /* (FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__NETWORK */

/****************************************************************/
/****************************************************************/
/****************************************************************/
/****************************************************************/
#ifndef AC49X_DRIVRES_VERSION
int  Ac49xTransmitPacket(int					_Device, 
						 int					_Channel, 
						 char					*_pOutPacket, 
						 int					_PacketSize, 
						 Tac49xProtocol			_Protocol,
						 Tac49xTransferMedium	_TransferMedium,
						 Tac49xControl			_UdpChecksumIncluded
						)
#else
int  Ac49xTransmitPacket(Tac49xTxPacketParams *pTxPacketParams)
#endif /*AC49X_DRIVRES_VERSION*/
{   
	int		                TailSize;		   
	int		                ActualSize;	 
	int						FirstIndex;
	int		                BufferDescriptorCurrentIndex;
	char	               *pCell;		   
    int                     Partial;
	int		                CellSize;  
#ifndef AC49X_DRIVRES_VERSION
	Tac49xTxPacketParams    TxPacketParams;
	Tac49xTxPacketParams  *pTxPacketParams = &TxPacketParams;
#endif /*AC49X_DRIVRES_VERSION*/
	unsigned int	        PacketAddress;
    U32                     BufferDescriptorBaseAddress;
	Tac49xBufferDescriptor  BufferDescriptor[NUMBER_OF_NETWORK_TX_BUFFER_DESCRIPTORS];			  

#if (FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__NETWORK)
    U32 bFifoEmpty;
    U32 bHpiEmpty  = 1;
#endif /*((FIFO_USAGE_ENABLE__HOST==1) || (FIFO_USAGE_ENABLE__HOST==1))*/

#if ALLOW_DEBUG_MODE
	int		                i;				/* packet index, partial packet index */
    Tac49xControl           DebugMode;
	unsigned char           Checksum = 0;	/* The packet Checksum value for debugging */
	Tac49xPacketFooter		DebugFooter;   
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	)
	U8						LastWordInBlockCombinedWithDebugFooter[4]; /* as the footer is one word */
#endif /* (AC49X_DEVICE_TYPE==AC491_DEVICE) || (AC49X_DEVICE_TYPE==AC491L_DEVICE) */
#endif /* ALLOW_DEBUG_MODE */

#ifndef AC49X_DRIVRES_VERSION
	TxPacketParams.Device				= Device			 ;
	TxPacketParams.Channel				= Channel			 ;
	TxPacketParams.pOutPacket			= pOutPacket		 ;
	TxPacketParams.PacketSize			= PacketSize		 ;
	TxPacketParams.Protocol				= Protocol			 ;
	TxPacketParams.TransferMedium		= TransferMedium	 ;
	TxPacketParams.UdpChecksumIncluded	= UdpChecksumIncluded;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE	 ;
#endif /*AC49X_DRIVRES_VERSION*/

    ((Tac49xHpiPacketHeader*)pTxPacketParams->pOutPacket)->Protocol				= pTxPacketParams->Protocol;
    ((Tac49xHpiPacketHeader*)pTxPacketParams->pOutPacket)->Channel				= pTxPacketParams->Channel;
    ((Tac49xHpiPacketHeader*)pTxPacketParams->pOutPacket)->Sync5				= 0x5;
    ((Tac49xHpiPacketHeader*)pTxPacketParams->pOutPacket)->UdpChecksumIncluded	= pTxPacketParams->UdpChecksumIncluded;
    ((Tac49xHpiPacketHeader*)pTxPacketParams->pOutPacket)->FavoriteStream		= pTxPacketParams->FavoriteStream;


    CellSize  = (pTxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
	          ? ac49xDeviceControlRegister[pTxPacketParams->Device].HostCellSize
	          : ac49xDeviceControlRegister[pTxPacketParams->Device].NetworkCellSize;
#if ALLOW_DEBUG_MODE
	    DebugMode = (pTxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
			      ? ac49xDeviceControlRegister[pTxPacketParams->Device].HostDebug
			      : ac49xDeviceControlRegister[pTxPacketParams->Device].NetworkDebug;
	    if(DebugMode) 
		    CellSize-=2;
#endif /* ALLOW_DEBUG_MODE */

    if(  (FIFO_USAGE_ENABLE__HOST    && pTxPacketParams->TransferMedium==TRANSFER_MEDIUM__HOST)
      || (FIFO_USAGE_ENABLE__NETWORK && pTxPacketParams->TransferMedium==TRANSFER_MEDIUM__NETWORK)
      )
        {

#if (FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__NETWORK)

	    Ac49xUserDef_CriticalSection_EnterHpi(pTxPacketParams->Device);
        Ac49xUserDef_CriticalSection_EnterFifo(pTxPacketParams->Device, pTxPacketParams->TransferMedium);
        bFifoEmpty = (Ac49xGetResidentCellNumber(pTxPacketParams->Device, pTxPacketParams->TransferMedium)==0) ? 1 : 0;
        if(bFifoEmpty)
            {
	        BufferDescriptorCurrentIndex = (pTxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
								         ? ac49xHostBufferDescriptorCurrentIndex   [pTxPacketParams->Device].Tx
								         : ac49xNetworkBufferDescriptorCurrentIndex[pTxPacketParams->Device].Tx;
            BufferDescriptorBaseAddress  = (pTxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
								         ? HPI_MEMORY__HOST_TX_BUFFER_DESCRIPTOR
								         : HPI_MEMORY__NETWORK_TX_BUFFER_DESCRIPTOR; 

	        /* Check for enough available Buffer Descriptors */
	        for(TailSize=pTxPacketParams->PacketSize, FirstIndex=-1; TailSize>0; TailSize-=CellSize)
                {      
                Ac49xReadBufferDescriptor(pTxPacketParams->Device, BufferDescriptorBaseAddress, BufferDescriptorCurrentIndex, &BufferDescriptor[BufferDescriptorCurrentIndex]);

				if (FirstIndex==BufferDescriptorCurrentIndex)
					{
					Ac49xUserDef_CriticalSection_LeaveFifo(pTxPacketParams->Device, pTxPacketParams->TransferMedium);
        	        Ac49xUserDef_CriticalSection_LeaveHpi( pTxPacketParams->Device);
					return FIFO_ACTION_RESULT__BUFFER_DESCRIPTOR_FULL_ERROR;
					}

                if(!BufferDescriptor[BufferDescriptorCurrentIndex].Empty)  
                    bHpiEmpty = 0;
				if (FirstIndex==-1)
					FirstIndex = BufferDescriptorCurrentIndex;
		        if(BufferDescriptor[BufferDescriptorCurrentIndex].Wrap)                                                                                                 
			        BufferDescriptorCurrentIndex=0;                                                                                                                  
		        else                                                                                                                                         
			        BufferDescriptorCurrentIndex++;                                                                                                             
                } 
            }
        /* Push all packet cells to FIFO because the HPI is full*/
        if(!bHpiEmpty || !bFifoEmpty)
            {
	        BufferDescriptorCurrentIndex = (pTxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
								         ? ac49xHostBufferDescriptorCurrentIndex   [pTxPacketParams->Device].Tx
								         : ac49xNetworkBufferDescriptorCurrentIndex[pTxPacketParams->Device].Tx;
            BufferDescriptorBaseAddress  = (pTxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
								         ? HPI_MEMORY__HOST_TX_BUFFER_DESCRIPTOR
								         : HPI_MEMORY__NETWORK_TX_BUFFER_DESCRIPTOR; 
	        for(TailSize=pTxPacketParams->PacketSize, pCell=pTxPacketParams->pOutPacket;  TailSize>0;  TailSize-=CellSize, pCell+=CellSize)                                          /*    )       */                                             
                {
                Partial = (int)(TailSize>CellSize);
		        ActualSize = Partial? CellSize : TailSize; 
                if(Ac49xEnqueueCell(pTxPacketParams->Device, pTxPacketParams->TransferMedium, pCell, ActualSize, Partial) == FIFO_ACTION_RESULT__FIFO_IS_FULL_ERROR)
		            {
		            Ac49xUserDef_CriticalSection_LeaveFifo(pTxPacketParams->Device, pTxPacketParams->TransferMedium);
        	        Ac49xUserDef_CriticalSection_LeaveHpi( pTxPacketParams->Device);
					pTxPacketParams->BufferDescriptorCurrentIndex = BufferDescriptorCurrentIndex;
                    return FIFO_ACTION_RESULT__FIFO_IS_FULL_ERROR;
		            }
                }
            }
        Ac49xUserDef_CriticalSection_LeaveFifo(pTxPacketParams->Device, pTxPacketParams->TransferMedium);
        if(!bHpiEmpty || !bFifoEmpty)
            {
            #if (PACKET_RECORDING_ENABLE==1)
	            _49xPacketRecording.Add(pTxPacketParams->Device, pTxPacketParams->pOutPacket, pTxPacketParams->PacketSize, PACKET_DIRECTION__TX);
            #endif
            Ac49xUserDef_CriticalSection_LeaveHpi(pTxPacketParams->Device);
            return 0;
            }
#endif /*(FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__NETWORK)*/
        }
    else
        {
	    Ac49xUserDef_CriticalSection_EnterHpi(pTxPacketParams->Device);

        BufferDescriptorBaseAddress  = (pTxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
						             ? HPI_MEMORY__HOST_TX_BUFFER_DESCRIPTOR
						             : HPI_MEMORY__NETWORK_TX_BUFFER_DESCRIPTOR; 
        BufferDescriptorCurrentIndex = (pTxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
							         ? ac49xHostBufferDescriptorCurrentIndex[pTxPacketParams->Device].Tx
							         : ac49xNetworkBufferDescriptorCurrentIndex[pTxPacketParams->Device].Tx; /* Go back to the starting Buffer Descriptor index */
	    for(TailSize=pTxPacketParams->PacketSize, FirstIndex=-1; TailSize>0; TailSize-=CellSize)
            {      
            Ac49xReadBufferDescriptor(pTxPacketParams->Device, BufferDescriptorBaseAddress, BufferDescriptorCurrentIndex, &BufferDescriptor[BufferDescriptorCurrentIndex]);

            if(!BufferDescriptor[BufferDescriptorCurrentIndex].Empty || FirstIndex==BufferDescriptorCurrentIndex)  
                {
                Ac49xUserDef_CriticalSection_LeaveHpi(pTxPacketParams->Device);
				pTxPacketParams->BufferDescriptorCurrentIndex = BufferDescriptorCurrentIndex;
	            return BUFFER_DESCRIPTOR_FULL_ERROR; /* InsertToTxFifo(); */
                }

			if (FirstIndex==-1)
				FirstIndex = BufferDescriptorCurrentIndex;
	        if(BufferDescriptor[BufferDescriptorCurrentIndex].Wrap)                                                                                                 
	            BufferDescriptorCurrentIndex=0;                                                                                                                  
	        else                                                                                                                                         
	            BufferDescriptorCurrentIndex++;                                                                                                             
            } 
        }

    BufferDescriptorBaseAddress  = (pTxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
					             ? HPI_MEMORY__HOST_TX_BUFFER_DESCRIPTOR
					             : HPI_MEMORY__NETWORK_TX_BUFFER_DESCRIPTOR; 
    BufferDescriptorCurrentIndex = (pTxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
						         ? ac49xHostBufferDescriptorCurrentIndex[pTxPacketParams->Device].Tx
						         : ac49xNetworkBufferDescriptorCurrentIndex[pTxPacketParams->Device].Tx; /* Go back to the starting Buffer Descriptor index */
    /* If debug mode is used and this is not a init-mode packet, the effective fixed packet size is */
    /* decremented by 2 (the debug word). */

    /* Transmit all partial packets and update all Buffer Descriptors */
    for(TailSize=pTxPacketParams->PacketSize, pCell=pTxPacketParams->pOutPacket;  TailSize>0;  TailSize-=CellSize, pCell+=CellSize)                                          /*    )       */                                             
        {
        Partial = (int)(TailSize>CellSize);
	    ActualSize = Partial? CellSize : TailSize; 
                                /* Note that there is no need to Read the Buffer Descriptor since they have been checked in the previous loop. */
        Ac49xReadBufferDescriptor(pTxPacketParams->Device, BufferDescriptorBaseAddress, BufferDescriptorCurrentIndex, &BufferDescriptor[BufferDescriptorCurrentIndex]);
        PacketAddress   = MergeFieldsToShort(BufferDescriptor[BufferDescriptorCurrentIndex].PacketAddress);                                                                                        
        PacketAddress  += AC49XIF_BASE_PAGE<<16; 
	    PacketAddress <<= DSP_TO_HOST_ADDRESS_SHIFT; 

	    if(pTxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
		    {
		    if(BufferDescriptorCurrentIndex<0 || BufferDescriptorCurrentIndex>=NUMBER_OF_HOST_TX_BUFFER_DESCRIPTORS)
			    {
			    Ac49xUserDef_CriticalSection_LeaveHpi(pTxPacketParams->Device);
				pTxPacketParams->BufferDescriptorCurrentIndex = BufferDescriptorCurrentIndex;
			    return TX_BD_INDEX_ERROR;
			    }
		    }
	    else
		    {
		    if(BufferDescriptorCurrentIndex<0 || BufferDescriptorCurrentIndex>=NUMBER_OF_NETWORK_TX_BUFFER_DESCRIPTORS)
			    {
			    Ac49xUserDef_CriticalSection_LeaveHpi(pTxPacketParams->Device);
				pTxPacketParams->BufferDescriptorCurrentIndex = BufferDescriptorCurrentIndex;
			    return TX_BD_INDEX_ERROR;
			    }
		    }
    
	    if(pTxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
		    {
		    if((PacketAddress < HPI_MEMORY__HOST_TX_BUFFER) || ((ActualSize+PacketAddress) > (HPI_MEMORY__HOST_TX_BUFFER+HOST_TX_BUFFER_SIZE))) 
			    {
			    Ac49xUserDef_CriticalSection_LeaveHpi(pTxPacketParams->Device);
				pTxPacketParams->BufferDescriptorCurrentIndex = BufferDescriptorCurrentIndex;
			    return TX_BAD_PACKER_ADDRESS_ERROR;                                                                          	                                     
			    }
		    }
	    else
		    {
		    if((PacketAddress < HPI_MEMORY__NETWORK_TX_BUFFER) || ((ActualSize+PacketAddress) > (HPI_MEMORY__NETWORK_TX_BUFFER+NETWORK_TX_BUFFER_SIZE))) 
			    {
			    Ac49xUserDef_CriticalSection_LeaveHpi(pTxPacketParams->Device);
				pTxPacketParams->BufferDescriptorCurrentIndex = BufferDescriptorCurrentIndex;
			    return TX_BAD_PACKER_ADDRESS_ERROR;                                                                          	                                     
			    }
		    }

#if ALLOW_DEBUG_MODE                                                                                                                                  
        if(DebugMode)           /* Handle debug word insertion for the current partial packet */
            {                                                                                                                                     
            Checksum = 0;
            for(i=0; i<ActualSize; i++)                                                      
                Checksum+=(unsigned char)pCell[i];                  /* Calculate the partial packet Checksum value */
                    /* Build the packet footer (debug word):*/
            DebugFooter.Checksum = Checksum;  
		    if(pTxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
			    {
			    if((DebugFooter.Sequence = ac49xTxHostPacketSequenceNumber[pTxPacketParams->Device]++) == 0x7F)  /* Insert the channel sequence number and update it : */
				    ac49xTxHostPacketSequenceNumber[pTxPacketParams->Device] = 0;                                /* If sequence number has reached 0x7f, reset the sequence number */
			    }
		    else
			    {
			    if((DebugFooter.Sequence = ac49xTxNetworkPacketSequenceNumber[pTxPacketParams->Device]++) == 0x7F)  /* Insert the channel sequence number and update it : */
				    ac49xTxNetworkPacketSequenceNumber[pTxPacketParams->Device] = 0;                                /* If sequence number has reached 0x7f, reset the sequence number */
			    }
            if(ActualSize>0)                                                                                                                     
                Ac49xWriteBlock(pTxPacketParams->Device, PacketAddress, pCell, ActualSize);      /* Copy the packet w/o the debug word to the DSP memory */
                    /* If the partial packet size is not an even number, Ac49xDeviceWriteBlock have */ 
		            /* written an extra 0 byte, and so the ActualSize should be incremented by 1.   */
            if(ActualSize&0x1)                                                                                                                   
                ActualSize++;                                                                                                                    
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	)
		    if((ActualSize>>1)&0x1)       
			    {
			    LastWordInBlockCombinedWithDebugFooter[0] = (U8)*(pCell+ActualSize-2);
			    LastWordInBlockCombinedWithDebugFooter[1] = (U8)*(pCell+ActualSize-1);
			    LastWordInBlockCombinedWithDebugFooter[2] = (U8)DebugFooter.Sequence;
			    LastWordInBlockCombinedWithDebugFooter[3] = (U8)DebugFooter.Checksum;
			    Ac49xWriteBlock(pTxPacketParams->Device, (PacketAddress+ActualSize), (char*)LastWordInBlockCombinedWithDebugFooter, 4); /* if the ActualSize in words is odd then */
			    }
		    else
#endif /* AC49XHPI_TYPE */
				Ac49xWriteBlock(pTxPacketParams->Device, (PacketAddress+ActualSize), (char*)&DebugFooter, sizeof(Tac49xPacketFooter));  /* Add the packet debug footer. Place the debug word in an even address by adding the ActualSize LS-bit. */
            SplitFieldsFromShort(BufferDescriptor[BufferDescriptorCurrentIndex].PacketSize, (ActualSize+sizeof(Tac49xPacketFooter)));	                                
            }                                                                                                                                     
        else                                                                                                                                      
#endif /* ALLOW_DEBUG_MODE */
            {                                                                                                                                     
            Ac49xWriteBlock(pTxPacketParams->Device, PacketAddress, pCell, ActualSize);          /* Copy Cell to DSP */
            SplitFieldsFromShort(BufferDescriptor[BufferDescriptorCurrentIndex].PacketSize, ActualSize);                                           
            }                                                                                                                                         
        BufferDescriptor[BufferDescriptorCurrentIndex].Partial = Partial;                                                                                                         
	    BufferDescriptor[BufferDescriptorCurrentIndex].Empty   = CONTROL__DISABLE;	 
        Ac49xWriteBufferDescriptor(pTxPacketParams->Device, BufferDescriptorBaseAddress, BufferDescriptorCurrentIndex, &BufferDescriptor[BufferDescriptorCurrentIndex]); /* Write back the Buffer Descriptor to the DSP memory */
                                    /* Advance BufferDescriptorCurrentIndex */
        if(BufferDescriptor[BufferDescriptorCurrentIndex].Wrap)                                                                                                      
		    BufferDescriptorCurrentIndex = 0;                                                                                                                  
	    else                                                                                                                                          
		    BufferDescriptorCurrentIndex++;                                                                                                                  
	    }       /*~~for(...Cell Transmission... */
    if(pTxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
	    ac49xHostBufferDescriptorCurrentIndex[   pTxPacketParams->Device].Tx = BufferDescriptorCurrentIndex;	                                                              
    else
	    ac49xNetworkBufferDescriptorCurrentIndex[pTxPacketParams->Device].Tx = BufferDescriptorCurrentIndex;

#if (PACKET_RECORDING_ENABLE==1)
	_49xPacketRecording.Add(pTxPacketParams->Device, pTxPacketParams->pOutPacket, pTxPacketParams->PacketSize, PACKET_DIRECTION__TX);
#endif
	
	Ac49xUserDef_CriticalSection_LeaveHpi(pTxPacketParams->Device);
    return 0;	/* success */
}



/****************************************************************
int  Ac49xReceivePacket(int						Device,                                                                                                               
						char					*pInPacket,                                                                                                               
						int						*pChannel,                                                                                                             
						int						*pPacketSize,                                                                                                               
						int						*pPayloadSize,                                                                                                              
						int						*pBufferDescriptorCurrentIndex,                                                                                                                  
						Tac49xProtocol			*pProtocol,
						Tac49xTransferMedium	TransferMedium
					   )
{
	Tac49xRxPacketParams RxPacketParams;


	RxPacketParams.Device							= Device					;
	RxPacketParams.pInPacket						= pInPacket					;
	RxPacketParams.TransferMedium					= TransferMedium			;
	return _Ac49xReceivePacket(&RxPacketParams);
	*pBufferDescriptorCurrentIndex	= RxPacketParams.BufferDescriptorCurrentIndex;
	*pChannel						= RxPacketParams.Channel					;
	*pPacketSize					= RxPacketParams.PacketSize					;
	*pPayloadSize					= RxPacketParams.PayloadSize				;
	*pProtocol						= RxPacketParams.Protocol					;
}
*/
#ifndef AC49X_DRIVRES_VERSION
int  Ac49xReceivePacket(int						Device,                                                                                                               
						char					*pInPacket,                                                                                                               
						int						*pChannel,                                                                                                             
						int						*pPacketSize,                                                                                                               
						int						*pPayloadSize,                                                                                                              
						int						*pBufferDescriptorCurrentIndex,                                                                                                                  
						Tac49xProtocol			*pProtocol,
						Tac49xTransferMedium	TransferMedium
					   )
#else
/****************************************************************/
int  Ac49xReceivePacket(Tac49xRxPacketParams *pRxPacketParams)
#endif /*AC49X_DRIVRES_VERSION*/
{
	unsigned int            PacketAddress;	                                                                                          
	int                     Error = 0;   
	Tac49xBufferDescriptor  BufferDescriptor;
    U32                     BufferDescriptorBaseAddress;
	int	                    BufferDescriptorCurrentIndex;
#ifndef AC49X_DRIVRES_VERSION
	Tac49xRxPacketParams	RxPacketParams;
	Tac49xRxPacketParams	*pRxPacketParams=&RxPacketParams;
#endif /*AC49X_DRIVRES_VERSION*/
#if ALLOW_DEBUG_MODE                                                                                             	                                     
	Tac49xPacketFooter     *pDebugFooter;                             	                                     
	int i;				                                                                                  
	unsigned char           Checksum = 0;	                                   	                                     
    Tac49xControl           DebugMode = (pRxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
									  ? ac49xDeviceControlRegister[pRxPacketParams->Device].HostDebug
									  : ac49xDeviceControlRegister[pRxPacketParams->Device].NetworkDebug;
#endif                                                                                                           	
	Ac49xUserDef_CriticalSection_EnterHpi(pRxPacketParams->Device);
    BufferDescriptorCurrentIndex    = (pRxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
									? ac49xHostBufferDescriptorCurrentIndex[pRxPacketParams->Device].Rx
									: ac49xNetworkBufferDescriptorCurrentIndex[pRxPacketParams->Device].Rx;   
    pRxPacketParams->BufferDescriptorCurrentIndex  = BufferDescriptorCurrentIndex;    
    BufferDescriptorBaseAddress     = (pRxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
									? HPI_MEMORY__HOST_RX_BUFFER_DESCRIPTOR
									: HPI_MEMORY__NETWORK_RX_BUFFER_DESCRIPTOR; 


	if(pRxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
		{
		if(BufferDescriptorCurrentIndex<0 || BufferDescriptorCurrentIndex>=NUMBER_OF_HOST_RX_BUFFER_DESCRIPTORS)
			{
			Ac49xUserDef_CriticalSection_LeaveHpi(pRxPacketParams->Device);
			return RX_BD_INDEX_ERROR;
			}
		}
	else
		{
		if(BufferDescriptorCurrentIndex<0 || BufferDescriptorCurrentIndex>=NUMBER_OF_NETWORK_RX_BUFFER_DESCRIPTORS)
			{
			Ac49xUserDef_CriticalSection_LeaveHpi(pRxPacketParams->Device);
			return RX_BD_INDEX_ERROR;
			}
		}

	Ac49xReadBufferDescriptor(pRxPacketParams->Device, BufferDescriptorBaseAddress, BufferDescriptorCurrentIndex, &BufferDescriptor);               /* Read the current Buffer Descriptor structure to the DSP memory image */
    if(BufferDescriptor.Empty)                                          	                                     
	    {                                                                                                            	                                     
		Ac49xUserDef_CriticalSection_LeaveHpi(pRxPacketParams->Device);
		pRxPacketParams->Channel = (-1);	                                                                                      	                                     
		return BUFFER_DESCRIPTOR_EMPTY_ERROR;                                                                                  	                                     
	    }                                                                                                            	                                     
	PacketAddress   = MergeFieldsToShort(BufferDescriptor.PacketAddress);       /* Get the packet address from the Buffer Descriptor and shift to translate to the host's addressing method */
    PacketAddress  += AC49XIF_BASE_PAGE<<16; 
	PacketAddress <<= DSP_TO_HOST_ADDRESS_SHIFT;

	pRxPacketParams->PacketAddress = PacketAddress;
    pRxPacketParams->PacketSize    = MergeFieldsToShort(BufferDescriptor.PacketSize);                                                                                               
	pRxPacketParams->PayloadSize   = pRxPacketParams->PacketSize-sizeof(Tac49xHpiPacketHeader);
	if(pRxPacketParams->PacketSize>MAX_PACKET_SIZE)
        {
		Ac49xUserDef_CriticalSection_LeaveHpi(pRxPacketParams->Device);
		return RX_PACKET_SIZE_ERROR;
        }
	if(pRxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
		{
		if((PacketAddress < HPI_MEMORY__HOST_RX_BUFFER) || ((pRxPacketParams->PacketSize+PacketAddress) > (HPI_MEMORY__HOST_RX_BUFFER+HOST_RX_BUFFER_SIZE))) 
			{
			Ac49xUserDef_CriticalSection_LeaveHpi(pRxPacketParams->Device);
			return RX_BAD_PACKER_ADDRESS_ERROR;                                                                          	                                     
			}
		}
	else
		{
		if((PacketAddress < HPI_MEMORY__NETWORK_RX_BUFFER) || ((pRxPacketParams->PacketSize+PacketAddress) > (HPI_MEMORY__NETWORK_RX_BUFFER+NETWORK_RX_BUFFER_SIZE))) 
			{
			Ac49xUserDef_CriticalSection_LeaveHpi(pRxPacketParams->Device);
			return RX_BAD_PACKER_ADDRESS_ERROR;                                                                          	                                     
			}
		}
#if ALLOW_DEBUG_MODE                                                                                             	                                     
	    if(DebugMode && (pRxPacketParams->PacketSize&0x1))   /* If debug mode is active and the packet size is an odd number, read an even number of bytes. */
		    Ac49xReadBlock(pRxPacketParams->Device, PacketAddress, pRxPacketParams->pInPacket, pRxPacketParams->PacketSize+1);
	    else                                                                                                         	                                     
#endif /*~~ALLOW_DEBUG_MODE*/
		    Ac49xReadBlock(pRxPacketParams->Device, PacketAddress, pRxPacketParams->pInPacket, pRxPacketParams->PacketSize);
	    if(((Tac49xHpiPacketHeader*)pRxPacketParams->pInPacket)->Sync5 != 0x5)                                                        	                                       
            {  
			Ac49xUserDef_CriticalSection_LeaveHpi(pRxPacketParams->Device);
	        pRxPacketParams->Channel = (-1);													 /* Indicate channel ID error */ 
	        return BAD_PACKET_HEADER; 
            }
	pRxPacketParams->Protocol = ((Tac49xHpiPacketHeader*)pRxPacketParams->pInPacket)->Protocol;
	pRxPacketParams->Channel  = ((Tac49xHpiPacketHeader*)pRxPacketParams->pInPacket)->Channel;                                                  	                                       
	if(BufferDescriptor.Wrap)    /* Update the current Buffer Descriptor index */
		{
		if(pRxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
		    ac49xHostBufferDescriptorCurrentIndex[pRxPacketParams->Device].Rx = 0;                                                                         	                                
		else
		    ac49xNetworkBufferDescriptorCurrentIndex[pRxPacketParams->Device].Rx = 0;                                                                         	                                
		}
	else 
		{
		if(pRxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
		    ac49xHostBufferDescriptorCurrentIndex[pRxPacketParams->Device].Rx++;                                                                         	                                
		else
		    ac49xNetworkBufferDescriptorCurrentIndex[pRxPacketParams->Device].Rx++;                                                                         	                                
		}
#if ALLOW_DEBUG_MODE                                                                                             	                                     
	            /* If debug mode is selected and the packet is not in init-mode, handle the debug footer */
	if(DebugMode)                                                                          	                                     
	    {                                       	                                     
		pRxPacketParams->PacketSize  -= sizeof(Tac49xPacketFooter);                                                                                         	                                      
		pRxPacketParams->PayloadSize -= sizeof(Tac49xPacketFooter);                                                                                        	                                      
		        /* Obtain a pointer to the packet debug word. */
		        /* Since the debug word is at an even address, add the pPacketSize LS-bit to the address. */
		pDebugFooter = (Tac49xPacketFooter*)(pRxPacketParams->pInPacket + pRxPacketParams->PacketSize + (pRxPacketParams->PacketSize&1));                              	                                         
		if(pRxPacketParams->TransferMedium == TRANSFER_MEDIUM__HOST)
			{
			if(pDebugFooter->Sequence != ac49xRxHostPacketSequenceNumber[pRxPacketParams->Device]++)                                             	                                
				{                                                                                                        	                                     
				Error = RX_SEQUENCE_ERROR;                                                                         	                                       
				ac49xRxHostPacketSequenceNumber[pRxPacketParams->Device] = pDebugFooter->Sequence+1;   /* Update ac49xRxPacketSequenceNumber to resynchronize with the DSP */
	    		}                                                                                                        	                                     
			if(pDebugFooter->Sequence == 0x7F)                                                                        	                                     
				ac49xRxHostPacketSequenceNumber[pRxPacketParams->Device] = 0;                                                                     	                                
			}
		else
			{
			if(pDebugFooter->Sequence != ac49xRxNetworkPacketSequenceNumber[pRxPacketParams->Device]++)                                             	                                
				{                                                                                                        	                                     
				Error = RX_SEQUENCE_ERROR;                                                                         	                                       
				ac49xRxNetworkPacketSequenceNumber[pRxPacketParams->Device] = pDebugFooter->Sequence+1;   /* Update ac49xRxPacketSequenceNumber to resynchronize with the DSP */
	    		}                                                                                                        	                                     
			if(pDebugFooter->Sequence == 0x7F)                                                                        	                                     
				ac49xRxNetworkPacketSequenceNumber[pRxPacketParams->Device] = 0;                                                                     	                                
			}

		for(i=0; i<pRxPacketParams->PacketSize; i++)                 /* Calculate the packet Checksum */
			Checksum += pRxPacketParams->pInPacket[i];                                                                     	                                       
		if(pDebugFooter->Checksum != Checksum)                                                                    	                                     
			Error = RX_CHECKSUM_ERROR;                                                                           	                                       
	    }                                                                                                            	                                     
#endif /* ALLOW_DEBUG_MODE */
	if(!Error)/* && (Error != RX_SEQUENCE_ERROR)) */
		Ac49xReleaseRxPacket(pRxPacketParams->Device, BufferDescriptorCurrentIndex, pRxPacketParams->TransferMedium); /* If the received packet is bad, release it before returning. */
#ifndef AC49X_DRIVRES_VERSION
	Tac49xRxPacketParams	RxPacketParams;
	Tac49xRxPacketParams	*pRxPacketParams=&RxPacketParams;
	*pBufferDescriptorCurrentIndex	= RxPacketParams.BufferDescriptorCurrentIndex;
	*pChannel						= RxPacketParams.Channel					;
	*pPacketSize					= RxPacketParams.PacketSize					;
	*pPayloadSize					= RxPacketParams.PayloadSize				;
	*pProtocol						= RxPacketParams.Protocol					;
#endif /*AC49X_DRIVRES_VERSION*/
#if (PACKET_RECORDING_ENABLE==1)
	_49xPacketRecording.Add(pRxPacketParams->Device, pRxPacketParams->pInPacket, pRxPacketParams->PacketSize, PACKET_DIRECTION__RX);
#endif
	Ac49xUserDef_CriticalSection_LeaveHpi(pRxPacketParams->Device);
	return Error;  
}

/****************************************************************/
int  Ac49xReleaseRxPacket(int Device, int BufferDescriptorCurrentIndex, Tac49xTransferMedium TransferMedium)
{
    Tac49xBufferDescriptor BufferDescriptor;
    U32 BufferDescriptorBaseAddress = (TransferMedium == TRANSFER_MEDIUM__HOST)? HPI_MEMORY__HOST_RX_BUFFER_DESCRIPTOR : HPI_MEMORY__NETWORK_RX_BUFFER_DESCRIPTOR; 
    Ac49xReadBufferDescriptor(Device, BufferDescriptorBaseAddress, BufferDescriptorCurrentIndex, &BufferDescriptor);                                                                             
    if(BufferDescriptor.Empty)                                                                                           
		return BUFFER_DESCRIPTOR_EMPTY_ERROR;                                                                                                              
	BufferDescriptor.Empty = CONTROL__ENABLE;
	Ac49xWriteBufferDescriptor(Device, BufferDescriptorBaseAddress, BufferDescriptorCurrentIndex, &BufferDescriptor); /* Write back the Buffer Descriptor to the DSP memory (if required) */
	return 0; /* success */
}

/**************************************************************************/                                                                                                                                                    
int Ac49xCheckRxPacketReady(int Device, Tac49xTransferMedium TransferMedium)                                                                                                            
{                                                                                                                                                     
    Tac49xBufferDescriptor	BufferDescriptor;
    U32 BufferDescriptorBaseAddress  = (TransferMedium == TRANSFER_MEDIUM__HOST)? HPI_MEMORY__HOST_RX_BUFFER_DESCRIPTOR : HPI_MEMORY__NETWORK_RX_BUFFER_DESCRIPTOR; 
	int BufferDescriptorCurrentIndex = (TransferMedium == TRANSFER_MEDIUM__HOST)?  ac49xHostBufferDescriptorCurrentIndex[Device].Rx : ac49xNetworkBufferDescriptorCurrentIndex[Device].Rx;   
    Ac49xReadBufferDescriptor(Device, BufferDescriptorBaseAddress, BufferDescriptorCurrentIndex, &BufferDescriptor); /* Read the current Buffer Descriptor structure into the DSP memory image */
    return (BufferDescriptor.Empty)? AC49X_RX_PACKET_NOT_READY : AC49X_RX_PACKET_READY;                                                                                                   
}                                                                                                                                                     

/****************************************************************/
void Ac49xResetDeviceBufferDescriptors(int Device)
{
	ac49xRxHostPacketSequenceNumber   [Device]          = 0;	
	ac49xTxHostPacketSequenceNumber   [Device]          = 0; 
	ac49xRxNetworkPacketSequenceNumber[Device]          = 0;
	ac49xTxNetworkPacketSequenceNumber[Device]          = 0;
	ac49xHostBufferDescriptorCurrentIndex[Device].Tx    = 0;
	ac49xHostBufferDescriptorCurrentIndex[Device].Rx    = 0;
	ac49xNetworkBufferDescriptorCurrentIndex[Device].Tx = 0;
	ac49xNetworkBufferDescriptorCurrentIndex[Device].Rx = 0;
}

/****************************************************************/
void Ac49xResetBufferDescriptors(void)
{
	int	Device;   
	for(Device=0; Device<AC49X_NUMBER_OF_DEVICES; Device++)

	    {
		ac49xRxHostPacketSequenceNumber   [Device]          = 0;	
		ac49xTxHostPacketSequenceNumber   [Device]          = 0; 
		ac49xRxNetworkPacketSequenceNumber[Device]          = 0;
		ac49xTxNetworkPacketSequenceNumber[Device]          = 0;
		ac49xHostBufferDescriptorCurrentIndex[Device].Tx    = 0;
		ac49xHostBufferDescriptorCurrentIndex[Device].Rx    = 0;
		ac49xNetworkBufferDescriptorCurrentIndex[Device].Tx = 0;
		ac49xNetworkBufferDescriptorCurrentIndex[Device].Rx = 0;
	    }
}

/*******************************************/
/*******************************************/
/*     Debugging Related Functions       **/
/*******************************************/
/*******************************************/

/****************************************************************/
void Ac49xSetDebugMode(Tac49xControl DebugMode) 
{
    int Device;
	for(Device=0; Device<AC49X_NUMBER_OF_DEVICES;Device++)
	    {    
        ac49xDeviceControlRegister[Device].HostDebug    = DebugMode;
        ac49xDeviceControlRegister[Device].NetworkDebug = DebugMode;
		ac49xRxHostPacketSequenceNumber   [Device] = 0;	
		ac49xTxHostPacketSequenceNumber   [Device] = 0;	
		ac49xRxNetworkPacketSequenceNumber[Device] = 0;	
		ac49xTxNetworkPacketSequenceNumber[Device] = 0;	
	    }
}

 /*--------------------------------------------------------------------
 * ROUTINE:   Ac49xCheckInitStateConfigurationEcho()
 *
 * DESCRIPTION:
 *   Wait for the DSP to echo the command, check it and return the result.
 *
 * ARGUMENTS:
 *   int Device                       - serial number of device
 *   (char *)CommandPacket            - pointer to the original command
 *   int PacketSize					  - size of the original command
 *
 * RESULTS:
 *      0 - OK
 * Timeout Fault:
 *          DEVICE_WAIT_TIMEOUT_ERROR     - Command Echo Packet did not arrive.
 * Echo Receiving Faults:
 *          BUFFER_DESCRIPTOR_EMPTY_ERROR - The current buffer descriptor is empty ; no packets to receive.                                                                
 *          RX_SEQUENCE_ERROR             - The received packet sequence number is wrong.                                                                               
 *          RX_CHECKSUM_ERROR             - The received packet Checksum value is wrong.                                                                                
 *          BAD_PACKET_HEADER  
 * Echo Cheking Faults:
 *          COMMAND_ECHO_ERROR_FLAG       - Echo packet is different than the transmitted command packet.
 *                                          in this case, if a status error packet was Received, 
 *                                          the error index will apeare on bits 16-31.    
 *          
 * ALGORITHM :
 * 
 * NOTES :
 * - The command echo mechanism makes sure no host to DSP command is lost.
 *   It also enables the verification of the packet receive/transmit
 *   mechanism.
 *
 * MODIFICATIONS :
 * 
 *--------------------------------------------------------------------*/
U32 Ac49xCheckInitStateConfigurationEcho(int Device, char *pConfigurationPacket, int PacketSize)
{
	int					 EchoPayloadSize;
    int					 EchoPacketSize;	  
	int					 iteration;		   
	int					 BufferDescriptorCurrentIndex;
    int					 Channel;   
	U32					 EchoResult = 0;
	Tac49xProtocol		 Protocol; 
    Tac49xPacket		 EchoPacket;
	Tac49xRxPacketParams RxPacketParams;
#if FIFO_USAGE_ENABLE__HOST
    while(Ac49xGetResidentCellNumber(Device, TRANSFER_MEDIUM__HOST))
		Ac49xUserDef_DelayForHostResponse();
#endif /* (FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__NETWORK) */
	for(iteration=0; iteration<MAX_DELAY_ITERATIONS; iteration++)
	    {
		if(Ac49xCheckRxPacketReady(Device, TRANSFER_MEDIUM__HOST) == AC49X_RX_PACKET_READY)
			break;
		Ac49xUserDef_DelayForDeviceResponse();
	    }
	if(iteration == MAX_DELAY_ITERATIONS)
		return DEVICE_WAIT_TIMEOUT_ERROR;
	            /* Receive the command echo packet */
	RxPacketParams.Device						= Device						;
	RxPacketParams.pInPacket						= (char*)&EchoPacket		;
	RxPacketParams.TransferMedium				= TRANSFER_MEDIUM__HOST			;
	EchoResult = Ac49xReceivePacket(&RxPacketParams);
	BufferDescriptorCurrentIndex = RxPacketParams.BufferDescriptorCurrentIndex	;
	Channel						 = RxPacketParams.Channel						;
	EchoPacketSize				 = RxPacketParams.PacketSize					; 
	EchoPayloadSize				 = RxPacketParams.PayloadSize					;
	Protocol					 = RxPacketParams.Protocol						;
/*	EchoResult = Ac49xReceivePacket(Device, (char*)&EchoPacket, &Channel, &EchoPacketSize, &EchoPayloadSize, &BufferDescriptorCurrentIndex, &Protocol, TRANSFER_MEDIUM__HOST);
*/	if(EchoResult && (EchoResult != RX_SEQUENCE_ERROR))
		return EchoResult;
#if CHECK_COMMAND_ECHO
    if(  (EchoPacket.u.ProprietaryPayload.ProprietaryHeader.PacketCategory == PACKET_CATEGORY__DEVICE_CONFIGURATION_PACKET)     
      && (EchoPacket.u.ProprietaryPayload.ProprietaryHeader.Opcode == ((Tac49xPacket*)pConfigurationPacket)->u.ProprietaryPayload.ProprietaryHeader.Opcode)
      )
        {
	    if(  (PacketSize != EchoPacketSize)
          || (memcmp((const void*)pConfigurationPacket, (const void*)&EchoPacket, EchoPacketSize))
          )
            {
		    EchoResult |= COMMAND_ECHO_ERROR_FLAG;
		    EchoResult |= COMMAND_ECHO_CHECKSUM_ERROR;
            }
        }
    else
        {
		EchoResult |= COMMAND_ECHO_ERROR_FLAG;
        if(  (EchoPacket.u.ProprietaryPayload.ProprietaryHeader.PacketCategory != PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET)     
          || (EchoPacket.u.ProprietaryPayload.ProprietaryHeader.Opcode         != STATUS_OR_EVENT_PACKET_OP_CODE__SYSTEM_ERROR_EVENT)
/*          || (EchoPacket.u.ProprietaryPayload.ProprietaryHeader.Coder          != 0)*/
          || (EchoPacket.HpiHeader.Channel									   != 0)
          || (EchoPacket.HpiHeader.Sync5									   != 0x5)
          )
            EchoResult |= COMMAND_ECHO_NOT_VALID_ERROR;
        else
            EchoResult |= (EchoPacket.u.ProprietaryPayload.u.StatusOrEvent.u.SystemErrorEvent.ErrorMessageIndex<<16);
        }
#endif /*~~CHECK_COMMAND_ECHO*/
/* Ac49xReleaseRxPacket(Device, BufferDescriptorCurrentIndex, TRANSFER_MEDIUM__HOST, CONTROL__DISABLE); */
	return EchoResult;
}

/*******************************************/
/*******************************************/
/*      Commands Issuing Routines         **/
/*******************************************/
/*******************************************/

            /*********************************************/
            /*   I N I T   S T A T E   C O M M A N D S   */
            /*********************************************/

/****************************************************************/
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	)
#	ifdef UTOPIA_ROUTING
U32 Ac49xUtopiaRoutingConfiguration(int Device, Tac49xUtopiaRoutingConfigurationAttr *pAttr)
{
    Tac49xUtopiaRoutingConfigurationPacket	Packet;
	Tac49xTxPacketParams					TxPacketParams;
    int										Channel;
	Tac49xMediaChannelType					Type;
    memset((void*)&Packet, 0, sizeof(Tac49xUtopiaRoutingConfigurationPacket));

    Packet.ProprietaryHeader.Opcode           = DEVICE_CONFIGURATION_OP_CODE__UTOPIA_ROUTING;
    Packet.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__DEVICE_CONFIGURATION_PACKET;

    for(Channel=0; Channel<AC49X_NUMBER_OF_CHANNELS; Channel++)
	    for(Type=MEDIA_CHANNEL_TYPE__REGULAR; Type<=MEDIA_CHANNEL_TYPE__CONFERENCE_2; Type++)
			{
			Packet.Route[Channel].MediaChannelType[Type].ToMediaChannelType	= pAttr->Route[Channel].MediaChannelType[Type].ToMediaChannelType	;
			Packet.Route[Channel].MediaChannelType[Type].ToDevice			= pAttr->Route[Channel].MediaChannelType[Type].ToDevice				;
			SplitFieldsFromShort(Packet.Route[Channel].MediaChannelType[Type].ToChannel, pAttr->Route[Channel].MediaChannelType[Type].ToChannel);
			}
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, 0, (char*)&Packet, sizeof(Tac49xUtopiaRoutingConfigurationPacket), TRANSFER_MEDIUM__HOST);
#endif
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= 0;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Tac49xUtopiaRoutingConfigurationPacket);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
	TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    Ac49xTransmitPacket(&TxPacketParams);
    return Ac49xCheckInitStateConfigurationEcho(Device, (char*)&Packet, sizeof(Tac49xUtopiaRoutingConfigurationPacket));
}
#	endif	/* UTOPIA_ROUTING */
#endif /* (AC49X_DEVICE_TYPE==AC491_DEVICE) || (AC49X_DEVICE_TYPE==AC491L_DEVICE) */

/****************************************************************/
U32 Ac49xCallProgressDeviceConfiguration(int Device, Tac49xCallProgressDeviceConfigurationAttr *pAttr)  
{
    int											Signal;
    Tac49xCallProgressDeviceConfigurationPacket Packet;
	Tac49xTxPacketParams						TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Tac49xCallProgressDeviceConfigurationPacket));

    Packet.ProprietaryHeader.Opcode         = DEVICE_CONFIGURATION_OP_CODE__CALL_PROGRESS   ;
    Packet.ProprietaryHeader.PacketCategory = PACKET_CATEGORY__DEVICE_CONFIGURATION_PACKET                  ;
    Packet.CommonAttributes.NumberOfSignals         = pAttr->NumberOfSignals     ;
    Packet.CommonAttributes.TotalEnergyThreshold    = pAttr->TotalEnergyThreshold;
    for(Signal=0; Signal<CALL_PROGRESS_AND_USER_DEFINED_TONES_SIGNAL_QUANTITY; Signal++)
        {
        SplitFieldsFromShort(Packet.Signal[Signal].ToneAFrequency                               , pAttr->Signal[Signal].ToneAFrequency                                  );
        SplitFieldsFromShort(Packet.Signal[Signal].ToneB_OrAmpModulationFrequency               , pAttr->Signal[Signal].ToneB_OrAmpModulationFrequency                  );
        SplitFieldsFromShort(Packet.Signal[Signal].ThirdToneOfTripleBatchDurationTypeFrequency  , pAttr->Signal[Signal].ThirdToneOfTripleBatchDurationTypeFrequency     );
        SplitFieldsFromShort(Packet.Signal[Signal].DetectionTimeOrCadenceFirstOnOrBurstDuration , pAttr->Signal[Signal].DetectionTimeOrCadenceFirstOnOrBurstDuration    );
        SplitFieldsFromShort(Packet.Signal[Signal].CadenceFirstOffDuration                      , pAttr->Signal[Signal].CadenceFirstOffDuration                         );
        SplitFieldsFromShort(Packet.Signal[Signal].CadenceSecondOnDuration                      , pAttr->Signal[Signal].CadenceSecondOnDuration                         );
        SplitFieldsFromShort(Packet.Signal[Signal].CadenceSecondOffDuration                     , pAttr->Signal[Signal].CadenceSecondOffDuration                        );
        SplitFieldsFromShort(Packet.Signal[Signal].CadenceThirdOnDuration                       , pAttr->Signal[Signal].CadenceThirdOnDuration                          );
        SplitFieldsFromShort(Packet.Signal[Signal].CadenceThirdOffDuration                      , pAttr->Signal[Signal].CadenceThirdOffDuration                         );
        SplitFieldsFromShort(Packet.Signal[Signal].CadenceFourthOnDuration                      , pAttr->Signal[Signal].CadenceFourthOnDuration                         );
        SplitFieldsFromShort(Packet.Signal[Signal].CadenceFourthOffDuration                     , pAttr->Signal[Signal].CadenceFourthOffDuration                        );
        Packet.Signal[Signal].Type						        = pAttr->Signal[Signal].Type							;                                                       
        Packet.Signal[Signal].TwistThreshold                    = pAttr->Signal[Signal].TwistThreshold                ;
        Packet.Signal[Signal].HighEnergyThreshold               = pAttr->Signal[Signal].HighEnergyThreshold           ;
        Packet.Signal[Signal].LowEnergyThreshold                = pAttr->Signal[Signal].LowEnergyThreshold            ;
        Packet.Signal[Signal].SignalToNoiseRatioThreshold       = pAttr->Signal[Signal].SignalToNoiseRatioThreshold   ;
        Packet.Signal[Signal].FrequencyDeviationThreshold       = pAttr->Signal[Signal].FrequencyDeviationThreshold   ;
        Packet.Signal[Signal].ToneALevel                        = pAttr->Signal[Signal].ToneALevel                    ;
        Packet.Signal[Signal].ToneBLevel                        = pAttr->Signal[Signal].ToneBLevel                    ;
        Packet.Signal[Signal].AmFactor                          = pAttr->Signal[Signal].AmFactor                      ;
        Packet.Signal[Signal].CadenceVoiceAddedWhileFirstOff    = pAttr->Signal[Signal].CadenceVoiceAddedWhileFirstOff ;
        Packet.Signal[Signal].CadenceVoiceAddedWhileSecondOff   = pAttr->Signal[Signal].CadenceVoiceAddedWhileSecondOff;
        Packet.Signal[Signal].CadenceVoiceAddedWhileThirdOff    = pAttr->Signal[Signal].CadenceVoiceAddedWhileThirdOff ;
        Packet.Signal[Signal].CadenceVoiceAddedWhileFourthOff   = pAttr->Signal[Signal].CadenceVoiceAddedWhileFourthOff;
        }                                                                                                           
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, 0, (char*)&Packet, sizeof(Tac49xCallProgressDeviceConfigurationPacket), TRANSFER_MEDIUM__HOST);
#endif
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= 0;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Tac49xCallProgressDeviceConfigurationPacket);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
	TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    Ac49xTransmitPacket(&TxPacketParams);
    return Ac49xCheckInitStateConfigurationEcho(Device, (char*)&Packet, sizeof(Tac49xCallProgressDeviceConfigurationPacket));
}

/****************************************************************/
U32 Ac49xUserDefinedTonesDeviceConfiguration(int Device, Tac49xUserDefinedTonesDeviceConfigurationAttr *pAttr)
{
    int Signal;
    Tac49xUserDefinedTonesDeviceConfigurationPacket Packet;
	Tac49xTxPacketParams							TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Tac49xUserDefinedTonesDeviceConfigurationPacket));

    Packet.ProprietaryHeader.Opcode					= DEVICE_CONFIGURATION_OP_CODE__USER_DEFINED_TONES;
    Packet.ProprietaryHeader.PacketCategory			= PACKET_CATEGORY__DEVICE_CONFIGURATION_PACKET    ;
    Packet.CommonAttributes.NumberOfSignals			= pAttr->NumberOfSignals						  ;
    Packet.CommonAttributes.TotalEnergyThreshold    = pAttr->TotalEnergyThreshold					  ;
    for(Signal=0; Signal<CALL_PROGRESS_AND_USER_DEFINED_TONES_SIGNAL_QUANTITY; Signal++)
        {
        SplitFieldsFromShort(Packet.Signal[Signal].ToneAFrequency, pAttr->Signal[Signal].ToneAFrequency);
        SplitFieldsFromShort(Packet.Signal[Signal].ToneBFrequency, pAttr->Signal[Signal].ToneBFrequency);
        Packet.Signal[Signal].TwistThreshold              = pAttr->Signal[Signal].TwistThreshold                ;
        Packet.Signal[Signal].HighEnergyThreshold         = pAttr->Signal[Signal].HighEnergyThreshold           ;
        Packet.Signal[Signal].LowEnergyThreshold          = pAttr->Signal[Signal].LowEnergyThreshold            ;
        Packet.Signal[Signal].SignalToNoiseRatioThreshold = pAttr->Signal[Signal].SignalToNoiseRatioThreshold   ;
        Packet.Signal[Signal].FrequencyDeviationThreshold = pAttr->Signal[Signal].FrequencyDeviationThreshold   ;
        Packet.Signal[Signal].ToneBLevel                  = pAttr->Signal[Signal].ToneBLevel                    ;
        Packet.Signal[Signal].ToneALevel                  = pAttr->Signal[Signal].ToneALevel                    ;
        }                                                                                                           
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, 0, (char*)&Packet, sizeof(Tac49xUserDefinedTonesDeviceConfigurationPacket), TRANSFER_MEDIUM__HOST);
#endif
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= 0;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Tac49xUserDefinedTonesDeviceConfigurationPacket);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
	TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    Ac49xTransmitPacket(&TxPacketParams);
    return Ac49xCheckInitStateConfigurationEcho(Device, (char*)&Packet, sizeof(Tac49xUserDefinedTonesDeviceConfigurationPacket));
}

/****************************************************************/
U32 Ac49xAgcDeviceConfiguration(int Device, Tac49xAgcDeviceConfigurationAttr *pAttr)
{
    Tac49xAgcDeviceConfigurationPacket	Packet;
	Tac49xTxPacketParams				TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Tac49xAgcDeviceConfigurationPacket));

    Packet.ProprietaryHeader.Opcode         = DEVICE_CONFIGURATION_OP_CODE__AGC			;
    Packet.ProprietaryHeader.PacketCategory = PACKET_CATEGORY__DEVICE_CONFIGURATION_PACKET;
    Packet.MinGain							= pAttr->MinGain							;
    Packet.MaxGain							= pAttr->MaxGain							;
    Packet.FastAdaptationGainSlope			= pAttr->FastAdaptationGainSlope			;
    SplitFieldsFromShort(Packet.FastAdaptationDuration, pAttr->FastAdaptationDuration)	;
    SplitFieldsFromShort(Packet.SilenceDuration,        pAttr->SilenceDuration)			;
    SplitFieldsFromShort(Packet.DetectionDuration,      pAttr->DetectionDuration)		;
    SplitFieldsFromShort(Packet.SignalingOverhang,      pAttr->SignalingOverhang)		;
                          
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, 0, (char*)&Packet, sizeof(Tac49xAgcDeviceConfigurationPacket), TRANSFER_MEDIUM__HOST);
#endif
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= 0;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Tac49xAgcDeviceConfigurationPacket);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
	TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    Ac49xTransmitPacket(&TxPacketParams);
    return Ac49xCheckInitStateConfigurationEcho(Device, (char*)&Packet, sizeof(Tac49xAgcDeviceConfigurationPacket));
}

/****************************************************************/
U32 Ac49xExtendedDeviceConfiguration(int Device, Tac49xExtendedDeviceConfigurationAttr *pAttr) /* The Core exits the init state and enters the run state after it completes executing the Open Device Command */
{
    Tac49xExtendedDeviceConfigurationPacket Packet;
	Tac49xTxPacketParams				TxPacketParams;

    memset((void*)&Packet, 0, sizeof(Tac49xExtendedDeviceConfigurationPacket));

    Packet.ProprietaryHeader.Opcode			= DEVICE_CONFIGURATION_OP_CODE__EXTENDED		;	
    Packet.ProprietaryHeader.PacketCategory	= PACKET_CATEGORY__DEVICE_CONFIGURATION_PACKET	;
	
	((char*)&Packet)[8 ] = -1;
	((char*)&Packet)[9 ] = -1;
	((char*)&Packet)[10] = -1;
	((char*)&Packet)[11] = -1;

	Packet.DebugUemClipEnable					= pAttr->DebugUemClipEnable					;
	Packet.TextV8Jm                             = pAttr->TextV8Jm						    ;
	Packet.TextV8Cm                             = pAttr->TextV8Cm					        ;
	Packet.TextV8Ci                             = pAttr->TextV8Ci						    ;
	Packet.VxxV8Cm                              = pAttr->VxxV8Cm						    ;
	Packet.VxxV8Ci                              = pAttr->VxxV8Ci						    ;
    Packet.FaxV8Cm							    = pAttr->FaxV8Cm						    ;
    Packet.FaxV8Ci							    = pAttr->FaxV8Ci						    ;
    Packet.FaxDcn								= pAttr->FaxDcn								;
    Packet.Tty									= pAttr->Tty								;
    Packet.Bell103AnswerTone					= pAttr->Bell103AnswerTone					;
    Packet.V21Channel1							= pAttr->V21Channel1						;
    Packet.V25CallingTone						= pAttr->V25CallingTone						;
    Packet.StuModem							    = pAttr->StuModem						    ;
    Packet.VxxV8JmVxxData						= pAttr->VxxV8JmVxxData						;
    Packet.V32Aa								= pAttr->V32Aa								;
    Packet.V22Usb1								= pAttr->V22Usb1							;
    Packet.InitiatingDualToneV8Bis				= pAttr->InitiatingDualToneV8Bis			;
    Packet.RespondingDualToneV8Bis				= pAttr->RespondingDualToneV8Bis			;
    Packet.DataSessionVxx						= pAttr->DataSessionVxx						;
    Packet.V21Channel2Data  					= pAttr->V21Channel2Data    				;
    Packet.V23ForwardChannel					= pAttr->V23ForwardChannel					;
    Packet.SilenceOrUnkown						= pAttr->SilenceOrUnkown					;
    Packet.CallingFaxToneCng					= pAttr->CallingFaxToneCng					;
    Packet.AnswerTone2100FaxCedOrModemAns		= pAttr->AnswerTone2100FaxCedOrModemAns		;
    Packet.AnswerWithReversalsAns				= pAttr->AnswerWithReversalsAns				;
    Packet.AnswerToneAm							= pAttr->AnswerToneAm						;
    Packet.AnswerToneAmReversals				= pAttr->AnswerToneAmReversals				;
    Packet.FaxV21PreambleFlags					= pAttr->FaxV21PreambleFlags				;
    Packet.FaxV8JmV34							= pAttr->FaxV8JmV34							;
    Packet.MfcR1DetectionStandard               = pAttr->MfcR1DetectionStandard             ;
    Packet.DtmfErasureSensitivity               = pAttr->DtmfErasureSensitivity             ;
    Packet.DtmfFrequencyDeviation               = pAttr->DtmfFrequencyDeviation             ;
    Packet.DtmfStepDistance						= pAttr->DtmfStepDistance					;
    Packet.DtmfStepRatio						= pAttr->DtmfStepRatio						;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, 0, (char*)&Packet, sizeof(Tac49xExtendedDeviceConfigurationPacket), TRANSFER_MEDIUM__HOST);
#endif
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= 0;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Tac49xExtendedDeviceConfigurationPacket);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
	TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    Ac49xTransmitPacket(&TxPacketParams);
    return Ac49xCheckInitStateConfigurationEcho(Device, (char*)&Packet, sizeof(Tac49xExtendedDeviceConfigurationPacket));
}

/****************************************************************/
U32 Ac49xOpenDeviceConfiguration(int Device, Tac49xOpenDeviceConfigurationAttr *pAttr) /* The Core exits the init state and enters the run state after it completes executing the Open Device Command */
{
    Tac49xOpenDeviceConfigurationPacket Packet;
	Tac49xTxPacketParams				TxPacketParams;
    int									Channel;
	U16 PacketSize = sizeof(Tac49xOpenDeviceConfigurationPacket) - sizeof(Tac49xPaddingForFutureNeed) - 4*(MAX_NUMBER_OF_ACTIVE_TIME_SLOTS-pAttr->NumberOfChannels);
    if(PacketSize>sizeof(Tac49xOpenDeviceConfigurationPacket))

        return (-1);
    memset((void*)&Packet, 0, sizeof(Tac49xOpenDeviceConfigurationPacket));

    Packet.ProprietaryHeader.Opcode			= DEVICE_CONFIGURATION_OP_CODE__OPEN			;	
    Packet.ProprietaryHeader.PacketCategory	= PACKET_CATEGORY__DEVICE_CONFIGURATION_PACKET	;
	Packet.ProprietaryHeader.Format			= FORMAT__0;
    Packet.JitterBufferStatusReportMode		= pAttr->JitterBufferStatusReportMode			;
    Packet.CasReversalsDetection			= pAttr->CasReversalsDetection					;
	Packet.DeviceStatusPacketEnable			= pAttr->DeviceStatusPacketEnable				;
	Packet.DebugInfoPacketEnable			= pAttr->DebugInfoPacketEnable					;
    Packet.CasValueDuringIdle				= pAttr->CasValueDuringIdle						;
    Packet.PcmValueDuringIdle				= pAttr->PcmValueDuringIdle						;
    Packet.CasDebounce						= pAttr->CasDebounce							;
    Packet.DtmfGenerationTwistDirection		= pAttr->DtmfGenerationTwistDirection			;
    Packet.DtmfGenerationTwist				= pAttr->DtmfGenerationTwist					;
	Packet.ErrorReportInterval				= pAttr->ErrorReportInterval					;
    Packet.DeviceStatusInterval				= pAttr->DeviceStatusInterval					;
    Packet.UtopiaTestEnable					= pAttr->UtopiaTestEnable						;

	Packet.ConnectedSlotEnable				= pAttr->ConnectedSlotEnable					;
	switch(pAttr->SerialPortSelection)
		{
		case SERIAL_PORT_SELECTION__VOICE_0_CAS_1:
			Packet.SerialPortSelection	  = 0;
			Packet.CasStreamPortSelection = 0;
			break;
		case SERIAL_PORT_SELECTION__VOICE_1_CAS_0:
			Packet.SerialPortSelection	  = 1;
			Packet.CasStreamPortSelection = 0;
			break;
		case SERIAL_PORT_SELECTION__VOICE_0_CAS_0:
			Packet.SerialPortSelection	  = 0;
			Packet.CasStreamPortSelection = 1;
			Packet.ConnectedSlotEnable    = CONTROL__ENABLE;
			break;
		case SERIAL_PORT_SELECTION__VOICE_1_CAS_1:
			Packet.SerialPortSelection	  = 1;
			Packet.CasStreamPortSelection = 1;
			Packet.ConnectedSlotEnable    = CONTROL__ENABLE;
			break;
		}
	Packet.DtmfSnr							        = pAttr->DtmfSnr								;
	Packet.DtmfGap							        = pAttr->DtmfGap								;
    Packet.CasMode							        = pAttr->CasMode								;
	Packet.CasBitsLocation					        = pAttr->Common.CasBitsLocation					;
	Packet.IpSecPortSelection				        = pAttr->Common.IpSecPortSelection				;
    Packet.BusSpeed							        = pAttr->Common.BusSpeed						;
    Packet.LawSelect						        = pAttr->Common.LawSelect						;
    Packet.UtopiaClockDetection				        = pAttr->Common.UtopiaClockDetection			;
    Packet.UtopiaCellSize					        = pAttr->Common.UtopiaCellSize					;
    Packet.PcmSerialPortRxTxDataDelay		        = pAttr->Common.PcmSerialPortRxTxDataDelay		;
    Packet.Utopia16BitMode					        = pAttr->Common.Utopia16BitMode					;
    Packet.UtopiaLogicalSlaveId					    = pAttr->Common.UtopiaLogicalSlaveId			;
    Packet.UtopiaPhysicalSlaveId					= pAttr->Common.UtopiaPhysicalSlaveId           ;
    Packet.WatchdogEnable					        = pAttr->Common.WatchdogEnable					;
    Packet.EventsPortType                           = pAttr->Common.EventsPortType                  ;
    Packet.TdmInterfaceConfiguration                = pAttr->Common.TdmInterfaceConfiguration       ;
	Packet.ModuleDebugPacketsTransferMethod			= pAttr->ModuleDebugPacketsTransferMethod		;

#if   (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	Packet.CodecEnable						= pAttr->Common.CodecEnable						;
	Packet.AicClkFrequency					= pAttr->Common.AicClkFrequency					;
#endif
	Packet.CpuClkOut						= pAttr->Common.CpuClkOut;
	Packet.PcmClkAndFrameSyncSource			= pAttr->Common.PcmClkAndFrameSyncSource		;
	Packet.PcmClkDivideRatio				= pAttr->Common.PcmClkDivideRatio				;
	Packet.PcmFrameSyncWidth				= pAttr->Common.PcmFrameSyncWidth				;
    SplitFieldsFromShort(Packet.NumberOfChannels, pAttr->NumberOfChannels);
    for(Channel=0; Channel<MAX_NUMBER_OF_ACTIVE_TIME_SLOTS; Channel++)
        {
        SplitFieldsFromShort(Packet.Channel[Channel].Slot              , pAttr->Channel[Channel].Slot);
        SplitFieldsFromShort(Packet.Channel[Channel].InterconnectedSlot, pAttr->Channel[Channel].InterconnectedSlot);
        }
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, 0, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= 0;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
	TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    Ac49xTransmitPacket(&TxPacketParams);
    return Ac49xCheckInitStateConfigurationEcho(Device, (char*)&Packet, PacketSize);
}

/****************************************************************/
U32 Ac49xOpenDeviceConfiguration_Format0(int Device, Tac49xOpenDeviceConfiguration_Format0Attr *pAttr) 
{
	return Ac49xOpenDeviceConfiguration(Device, (Tac49xOpenDeviceConfigurationAttr*)pAttr);
}

/****************************************************************/
U32 Ac49xOpenDeviceConfiguration_Format1(int Device, Tac49xOpenDeviceConfiguration_Format1Attr *pAttr) 
{
	Tac49xOpenDeviceConfigurationPacket_Format1 Packet;
	Tac49xTxPacketParams						TxPacketParams;
    int											Channel;
	U16 PacketSize = sizeof(Tac49xOpenDeviceConfigurationPacket_Format1) - sizeof(Tac49xPaddingForFutureNeed) - 4*(MAX_NUMBER_OF_ACTIVE_TIME_SLOTS-pAttr->NumberOfChannels);
    if(PacketSize>sizeof(Tac49xOpenDeviceConfigurationPacket_Format1))
        return (-1);
    memset((void*)&Packet, 0, sizeof(Tac49xOpenDeviceConfigurationPacket_Format1));

    Packet.ProprietaryHeader.Opcode			= DEVICE_CONFIGURATION_OP_CODE__OPEN			;	
    Packet.ProprietaryHeader.PacketCategory	= PACKET_CATEGORY__DEVICE_CONFIGURATION_PACKET	;
	Packet.ProprietaryHeader.Format			= FORMAT__1;
    Packet.JitterBufferStatusReportMode		= pAttr->JitterBufferStatusReportMode			;
    Packet.CasReversalsDetection			= pAttr->CasReversalsDetection					;
	Packet.DeviceStatusPacketEnable			= pAttr->DeviceStatusPacketEnable				;
	Packet.DebugInfoPacketEnable			= pAttr->DebugInfoPacketEnable					;
    Packet.CasValueDuringIdle				= pAttr->CasValueDuringIdle						;
    Packet.PcmValueDuringIdle				= pAttr->PcmValueDuringIdle						;
    Packet.CasDebounce						= pAttr->CasDebounce							;
    Packet.DtmfGenerationTwistDirection		= pAttr->DtmfGenerationTwistDirection			;
    Packet.DtmfGenerationTwist				= pAttr->DtmfGenerationTwist					;
	Packet.ErrorReportInterval				= pAttr->ErrorReportInterval					;
    Packet.DeviceStatusInterval				= pAttr->DeviceStatusInterval					;
    Packet.UtopiaTestEnable					= pAttr->UtopiaTestEnable						;

	Packet.DtmfSnr							        = pAttr->DtmfSnr								;
	Packet.DtmfGap							        = pAttr->DtmfGap								;
    Packet.CasMode							        = pAttr->CasMode								;
	Packet.CasBitsLocation					        = pAttr->Common.CasBitsLocation					;
	Packet.IpSecPortSelection				        = pAttr->Common.IpSecPortSelection				;
    Packet.BusSpeed							        = pAttr->Common.BusSpeed						;
    Packet.LawSelect						        = pAttr->Common.LawSelect						;
    Packet.UtopiaClockDetection				        = pAttr->Common.UtopiaClockDetection			;
    Packet.UtopiaCellSize					        = pAttr->Common.UtopiaCellSize					;
    Packet.PcmSerialPortRxTxDataDelay		        = pAttr->Common.PcmSerialPortRxTxDataDelay		;
    Packet.Utopia16BitMode					        = pAttr->Common.Utopia16BitMode					;
    Packet.UtopiaLogicalSlaveId					    = pAttr->Common.UtopiaLogicalSlaveId			;
    Packet.UtopiaPhysicalSlaveId					= pAttr->Common.UtopiaPhysicalSlaveId           ;
    Packet.WatchdogEnable					        = pAttr->Common.WatchdogEnable					;
    Packet.EventsPortType                           = pAttr->Common.EventsPortType                  ;
    Packet.TdmInterfaceConfiguration                = pAttr->Common.TdmInterfaceConfiguration       ;

	Packet.ConnectedSlots							= pAttr->ConnectedSlots							;
	Packet.ConnectedSlotsSerialPortSelection        = pAttr->ConnectedSlotsSerialPortSelection      ;
	Packet.CasSerialPortSelection					= pAttr->CasSerialPortSelection					;
	Packet.VoiceSerialPortSelection					= pAttr->VoiceSerialPortSelection				;
	Packet.ModuleDebugPacketsTransferMethod			= pAttr->ModuleDebugPacketsTransferMethod		;


#if   (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	Packet.CodecEnable						= pAttr->Common.CodecEnable						;
	Packet.AicClkFrequency					= pAttr->Common.AicClkFrequency					;
#endif
	Packet.CpuClkOut						= pAttr->Common.CpuClkOut;
	Packet.PcmClkAndFrameSyncSource			= pAttr->Common.PcmClkAndFrameSyncSource		;
	Packet.PcmClkDivideRatio				= pAttr->Common.PcmClkDivideRatio				;
	Packet.PcmFrameSyncWidth				= pAttr->Common.PcmFrameSyncWidth				;
    SplitFieldsFromShort(Packet.NumberOfChannels, pAttr->NumberOfChannels);
    for(Channel=0; Channel<MAX_NUMBER_OF_ACTIVE_TIME_SLOTS; Channel++)
        {
        SplitFieldsFromShort(Packet.Channel[Channel].CasSlot              , pAttr->Channel[Channel].CasSlot);
		SplitFieldsFromShort(Packet.Channel[Channel].PcmSlot              , pAttr->Channel[Channel].PcmSlot);
		SplitFieldsFromShort(Packet.Channel[Channel].InterconnectedPcmSlot, pAttr->Channel[Channel].InterconnectedPcmSlot);
        }
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, 0, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= 0;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
	TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    Ac49xTransmitPacket(&TxPacketParams);
    return Ac49xCheckInitStateConfigurationEcho(Device, (char*)&Packet, PacketSize);
}

/****************************************************************/
            /*********************************************/
            /*   R U N   S T A T E   C O M M A N D S     */
            /*********************************************/
            /* configurtion Functions */
/****************************************************************/
int OpenOrUpdateChannelConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xOpenOrUpdateChannelConfigurationAttr *pAttr)
{
    Tac49xOpenOrUpdateChannelConfigurationPacket Packet;
	Tac49xTxPacketParams						 TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Tac49xOpenOrUpdateChannelConfigurationPacket));

    Packet.ProprietaryHeader.Opcode            = OpCode											;
    Packet.ProprietaryHeader.PacketCategory    = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET					;
                    /* Mediated */ 
/*  Packet.SilenceCompressionMode                = pAttr->SilenceCompressionMode                ;*/
/*  Packet.NumberOfVoiceBlocks_minus_1           = pAttr->NumberOfVoiceBlocks_minus_1           ;*/
    Packet.HybridLoss							 = pAttr->HybridLoss							;
    Packet.DtmfTransferMode                      = pAttr->DtmfTransferMode                      ;
    Packet.DynamicJitterBufferOptimizationFactor = pAttr->DynamicJitterBufferOptimizationFactor ;
    SplitFieldsFromShort(Packet.VoiceJitterBufferMinDelay, pAttr->VoiceJitterBufferMinDelay)    ;
    SplitFieldsFromShort(Packet.VoiceJitterBufferMaxDelay, pAttr->VoiceJitterBufferMaxDelay)    ;
                    /* Not for Mediated Channel (ignore this fields) */ 
    Packet.ChannelType                           = pAttr->ChannelType                           ;
    Packet.EchoCancelerNonLinearProcessor        = pAttr->EchoCancelerNonLinearProcessor        ; 
	Packet.PcmLimiter							 = pAttr->PcmLimiter							;
    Packet.EchoCancelerMode                      = pAttr->EchoCancelerMode                      ;
    Packet.PcmSource                             = pAttr->PcmSource                             ;
	Packet.PacketToPacketMode                    = pAttr->PacketToPacketMode                    ;
    Packet.InputGain                             = pAttr->InputGain                             ;
    Packet.VoiceOutputGain                       = pAttr->VoiceOutputGain                       ;
    Packet.CasRelay                              = pAttr->CasRelay                              ;
    Packet.CallProgressTransferMode              = pAttr->CallProgressTransferMode              ;
	Packet.DetectPcmCas							 = pAttr->DetectPcmCas;
	Packet.DetectNetCasPackets					 = pAttr->DetectNetCasPackets					;
	Packet.DetectNetIbsPackets					 = pAttr->DetectNetIbsPackets					;
	Packet.IgnoreNetCasPackets					 = pAttr->IgnoreNetCasPackets					;
	Packet.IgnoreNetIbsPackets					 = pAttr->IgnoreNetIbsPackets					;
/*  Packet.LineSignalingTransferMode             = pAttr->LineSignalingTransferMode             ;*/
	Packet.DtmfErasureMode						 = pAttr->DtmfErasureMode						;
    Packet.MfTransferMode                        = pAttr->MfTransferMode                        ;
    Packet.TestFaxModem                          = pAttr->TestFaxModem                          ;
    Packet.IbsDetectionRedirection               = pAttr->IbsDetectionRedirection               ;
    Packet.DetectUserDefinedTones                = pAttr->DetectUserDefinedTones                ;
    Packet.DetectCallProgress                    = pAttr->DetectCallProgress                    ;
    Packet.DetectDTMFRegister                    = pAttr->DetectDTMFRegister                    ;
	Packet.Reserve_FX2							 = pAttr->Reserve_FX2							;
	Packet.Reserve_FX5							 = pAttr->Reserve_FX5							;
	Packet.Reserve_FX4							 = pAttr->Reserve_FX4							;
	Packet.Reserve_FX15							 = pAttr->Reserve_FX15							; 
	Packet.Reserve_FX1							 = pAttr->Reserve_FX1							;
    Packet.DetectMFR2BackwardRegister            = pAttr->DetectMFR2BackwardRegister            ;
    Packet.DetectMFR2ForwardRegister             = pAttr->DetectMFR2ForwardRegister             ;
    Packet.DetectMFSS5Register                   = pAttr->DetectMFSS5Register                   ;
    Packet.DetectMFR1Register                    = pAttr->DetectMFR1Register                    ;
    Packet.CallerIdOnHookService                 = pAttr->CallerIdOnHookService                 ;
    Packet.CallerIdOffHookService                = pAttr->CallerIdOffHookService                ;
    Packet.TestMode                              = pAttr->TestMode                              ;
    Packet.IbsLevel_minus_dbm                    = pAttr->IbsLevel_minus_dbm                    ;
    Packet.UdpChecksumEnable                     = pAttr->UdpChecksumEnable                     ;
/*	Packet.DataMaxRate                           = pAttr->DataMaxRate                           ;*/
    Packet.FaxMaxRate                            = pAttr->FaxMaxRate                            ;
	Packet.LongDtmfDetectionEnable				 = pAttr->LongDtmfDetectionEnable				;
    Packet.EchoCancelerLength                    = pAttr->EchoCancelerLength                    ;
    Packet.TriStateDuringIdle                    = pAttr->TriStateDuringIdle                    ;
    Packet.CasModeDuringIdle                     = pAttr->CasModeDuringIdle                     ;
    Packet.CasValueDuringIdle                    = pAttr->CasValueDuringIdle                    ; 
    Packet.PcmValueDuringIdle                    = pAttr->PcmValueDuringIdle                    ;
	Packet.ActiveChannelPattern					 = pAttr->ActiveChannelPattern					;
/*  Packet.EncoderPcmAdpcmFrameDuration          = pAttr->EncoderPcmAdpcmFrameDuration          ;*/
/*  Packet.DecoderPcmAdpcmFrameDuration          = pAttr->DecoderPcmAdpcmFrameDuration          ;*/
/*  Packet.BypassNumberOfBlocks_minus_1          = pAttr->BypassNumberOfBlocks_minus_1          ;*/
    Packet.FaxAndCallerIdLevel                   = pAttr->FaxAndCallerIdLevel                   ;
	Packet.TtyTransferMethod					 = pAttr->TtyTransferMethod						;
    Packet.V34FaxTransferMethod                  = pAttr->V34FaxTransferMethod                  ;
    Packet.V34TransferMethod                     = pAttr->V34TransferMethod                     ;
    Packet.V32TransferMethod                     = pAttr->V32TransferMethod                     ;
    Packet.V23TransferMethod                     = pAttr->V23TransferMethod                     ;
    Packet.V22TransferMethod                     = pAttr->V22TransferMethod                     ;
    Packet.V21TransferMethod                     = pAttr->V21TransferMethod                     ;
	Packet.Bell103TransferMethod				 = pAttr->Bell103TransferMethod					;
    Packet.FaxTransferMethod                     = pAttr->FaxTransferMethod                     ;
    Packet.FaxProtocol                           = pAttr->FaxProtocol							;
    Packet.CallerIdStandard                      = pAttr->CallerIdStandard                      ;
    Packet.FaxErrorCorrectionMode                = pAttr->FaxErrorCorrectionMode                ; 
    Packet.CngRelayEnable                        = pAttr->CngRelayEnable						;
	Packet.CedTransferMode						 = pAttr->CedTransferMode						;
    Packet.AgcSlowAdaptationGainSlope			 = pAttr->AgcSlowAdaptationGainSlope			; 
    Packet.AgcLocation                           = pAttr->AgcLocation							;
    Packet.AgcEnable							 = pAttr->AgcEnable								;
    Packet.AgcTargetEnergy                       = pAttr->AgcTargetEnergy						;
	Packet.FaxAndDataStatusAndEventsDisable		 = pAttr->FaxAndDataStatusAndEventsDisable		;
	Packet.PcmSidSize							 = pAttr->PcmSidSize							;
    Packet.EchoCancelerDebugPackets              = pAttr->EchoCancelerDebugPackets              ;
    Packet.VideoSynchronizationEnable            = pAttr->VideoSynchronizationEnable            ;
	Packet.AnswerDetectEnable					= pAttr->AnswerDetectEnable						;
	Packet.AnswerDetectDirection				= pAttr->AnswerDetectDirection					;
	Packet.AnswerDetectMode						= pAttr->AnswerDetectMode						;
	Packet.AnswerDetectSensitivity				= pAttr->AnswerDetectSensitivity				;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Tac49xOpenOrUpdateChannelConfigurationPacket), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Tac49xOpenOrUpdateChannelConfigurationPacket);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}


/****************************************************************/
int ActivateOrUpdateRtpChannelConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xActivateOrUpdateRtpChannelConfigurationAttr *pAttr)
{
    int													Character;
    Tac49xActivateOrUpdateRtpChannelConfigurationPacket Packet;
	Tac49xTxPacketParams								TxPacketParams;
    int                                                 PacketSize = sizeof(Tac49xActivateOrUpdateRtpChannelConfigurationPacket);
    memset((void*)&Packet, 0, PacketSize);

    Packet.ProprietaryHeader.Opcode           = OpCode											;
    Packet.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET	;


    Packet.VbdEventsRelayType						 = pAttr->VbdEventsRelayType	;
#if (AC49X_DRIVRES_VERSION>=AC49X_DRIVER_VERSION_580)
    Packet.TxVbdEventsRelayPayloadType	  			 = pAttr->TxVbdEventsRelayPayloadType	  	;
	Packet.RxVbdEventsRelayPayloadType	  			 = pAttr->RxVbdEventsRelayPayloadType	  	;
#else
	Packet.TxVbdEventsRelayPayloadType	  			 = pAttr->VbdEventsRelayPayloadType	  		;
	Packet.RxVbdEventsRelayPayloadType	  			 = pAttr->VbdEventsRelayPayloadType	  		;
#endif
    Packet.NumberOfVoiceBlocks_minus_1	  			 = pAttr->NumberOfVoiceBlocks_minus_1		;
    Packet.NumberOfBypassBlocks_minus_1  			 = pAttr->NumberOfBypassBlocks_minus_1  	;
    Packet.DecoderPcmAdpcmFrameDuration  			 = pAttr->DecoderPcmAdpcmFrameDuration  	;
    Packet.EncoderPcmAdpcmFrameDuration  			 = pAttr->EncoderPcmAdpcmFrameDuration  	;
    Packet.SilenceCompressionMode		  			 = pAttr->SilenceCompressionMode			;
	Packet.BrokenConnectionActivationMode			 = pAttr->BrokenConnectionActivationMode	;
    Packet.AmrActiveMode							 = pAttr->AmrActiveMode						;
    Packet.MediaProtectionMode                       = pAttr->MediaProtectionMode			    ;
    Packet.DisableRtcpIntervalRandomization          = pAttr->DisableRtcpIntervalRandomization	;
    Packet.AdpcmPayloadFormat				         = pAttr->AdpcmPayloadFormat				;
    Packet.Decoder                                   = pAttr->Decoder							;
    Packet.Encoder                                   = pAttr->Encoder							;
    Packet.TxSignalingRfc2833PayloadType             = pAttr->TxSignalingRfc2833PayloadType		;
    Packet.RxSignalingRfc2833PayloadType             = pAttr->RxSignalingRfc2833PayloadType		;
	Packet.Rfc2198PayloadType						 = pAttr->Rfc2198PayloadType				;

    Packet.IbsRedundancyLevel                        = pAttr->IbsRedundancyLevel				;
    Packet.RtpCanonicalNameStringLength              = pAttr->RtpCanonicalNameStringLength		;
    Packet.RtpRedundancy                             = pAttr->RtpRedundancy						;
    Packet.RxPayloadType                             = pAttr->RxPayloadType						;
    Packet.TxPayloadType                             = pAttr->TxPayloadType						;
    Packet.RxPayloadTypeSource                       = pAttr->RxPayloadTypeSource				;
    Packet.TxPayloadTypeSource                       = pAttr->TxPayloadTypeSource				;
    Packet.FaxBypassPayloadType                      = pAttr->FaxBypassPayloadType				;
    Packet.DataBypassPayloadType                     = pAttr->DataBypassPayloadType				;
    Packet.BypassCoder                               = pAttr->BypassCoder						;
    Packet.PcmAdPcmComfortNoiseRtpPayloadTypeEnable  = pAttr->PcmAdPcmComfortNoiseRtpPayloadTypeEnable;
    Packet.PcmAdPcmComfortNoiseRtpPayloadType        = pAttr->PcmAdPcmComfortNoiseRtpPayloadType;
    Packet.RxT38PayloadType							 = pAttr->RxT38PayloadType;
    Packet.TxT38PayloadType							 = pAttr->TxT38PayloadType;
    Packet.NoOperationEnable						 = pAttr->NoOperationEnable;
    Packet.NoOperationPayloadType					 = pAttr->NoOperationPayloadType;
	Packet.BypassRedundancyType						 = pAttr->BypassRedundancyType;
	Packet.TxFilterRtp						         = pAttr->TxFilterRtp;
	Packet.TxFilterRfc2833						     = pAttr->TxFilterRfc2833;
	Packet.TxFilterRtcp						         = pAttr->TxFilterRtcp;
	Packet.RxFilterRtp						         = pAttr->RxFilterRtp;
	Packet.RxFilterRfc2833						     = pAttr->RxFilterRfc2833;
	Packet.RxFilterRtcp						         = pAttr->RxFilterRtcp;
	Packet.PacketToPacketSequenceNumberMode			 = pAttr->PacketToPacketSequenceNumberMode;
	Packet.PacketToPacketSsrcMode					 = pAttr->PacketToPacketSsrcMode;
	Packet.PacketToPacketNoTranscodingPayloadType	 = pAttr->PacketToPacketNoTranscodingPayloadType;
	Packet.BlockNonePayloadType13					 = pAttr->BlockNonePayloadType13;
	Packet.ReportErrorOnReceivedStreamEnable		 = pAttr->ReportErrorOnReceivedStreamEnable;
	Packet.RtcpXrMode								 = pAttr->RtcpXrMode;
	Packet.RtcpXrEventsMode							 = pAttr->RtcpXrEventsMode;
	Packet.RtpSequenceNumberDetectionEnable			 = pAttr->RtpSequenceNumberDetectionEnable;
	Packet.AnswerToneOverRfc2833Enable				 = pAttr->AnswerToneOverRfc2833Enable;
	Packet.ModemRelayHighAvailabilityOptions		 = pAttr->ModemRelayHighAvailabilityOptions;
	Packet.SseRedundancyDepth	  					 = pAttr->SseRedundancyDepth;

	SplitFieldsFromShort(Packet.BrokenConnectionEventTimeout_x20msec,	pAttr->BrokenConnectionEventTimeout_x20msec);
    SplitFieldsFromShort(Packet.NoOperationTimeout_x20msec,				pAttr->NoOperationTimeout_x20msec);
    SplitFieldsFromShort(Packet.TxSequenceNumber,						pAttr->TxSequenceNumber);
    SplitFieldsFromLong(Packet.TxTimeStamp,								pAttr->TxTimeStamp);
    SplitFieldsFromLong(Packet.RtpSynchronizingSource,					pAttr->RtpSynchronizingSource); 
    SplitFieldsFromShort(Packet.RtcpMeanTxInterval,						pAttr->RtcpMeanTxInterval);
	SplitFieldsFromShort(Packet.IbsRelayTransmissionTimeout,			pAttr->IbsRelayTransmissionTimeout);
	SplitFieldsFromShort(Packet.SequenceNumberForDetection,				pAttr->SequenceNumberForDetection);
    if(Packet.RtpCanonicalNameStringLength>AC49X_MAX_RTP_CANONICAL_NAME_STRING_LENGTH)
        Packet.RtpCanonicalNameStringLength = AC49X_MAX_RTP_CANONICAL_NAME_STRING_LENGTH;
    for(Character=0; Character<Packet.RtpCanonicalNameStringLength; Character++)  /*AC49X_MAX_RTP_CANONICAL_NAME_STRING_LENGTH*/
        Packet.RtpCanonicalName[Character] = pAttr->RtpCanonicalName[Character];

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif
    
#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel              = Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}


/****************************************************************/
#ifndef AC49X_DRIVRES_VERSION
int CloseRtpChannelConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel)
#else
int CloseRtpChannelConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xCloseRtpChannelConfigurationAttr *pAttr)
#endif
{
    Tac49xCloseRtpChannelPacket Packet;
	Tac49xTxPacketParams		TxPacketParams;
	int							Character;
    memset((void*)&Packet, 0, sizeof(Tac49xCloseRtpChannelPacket));

    Packet.ProprietaryHeader.Opcode            =  OpCode;
    Packet.ProprietaryHeader.PacketCategory    = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;
#if (AC49X_DRIVRES_VERSION >= AC49X_DRIVER_VERSION_105)
	Packet.SendRtcpByePacket				   = pAttr->SendRtcpByePacket;
	Packet.SendReason						   = pAttr->SendReason;
	Packet.ReasonMessageLength				   = pAttr->ReasonMessageLength;

    if(Packet.ReasonMessageLength>AC49X_RTP_MAX_LEAVING_REASON_MESSAGE_SIZE)
        Packet.ReasonMessageLength = AC49X_RTP_MAX_LEAVING_REASON_MESSAGE_SIZE;
    for(Character=0; Character<Packet.ReasonMessageLength; Character++)
        Packet.LeavingReasonMessage[Character] = pAttr->LeavingReasonMessage[Character];
#endif

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Tac49xCloseRtpChannelPacket), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel              = Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Tac49xCloseRtpChannelPacket);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/****************************************************************/
int Ac49xUpdateChannelConfiguration(int Device, int Channel, Tac49xOpenOrUpdateChannelConfigurationAttr *pAttr)
{
    return OpenOrUpdateChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__UPDATE_CHANNEL, Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xAdvancedChannelConfiguration(int Device, int Channel, Tac49xAdvancedChannelConfigurationAttr *pAttr)
{
    Tac49xAdvancedChannelConfigurationPacket Packet;
	Tac49xTxPacketParams					 TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Tac49xAdvancedChannelConfigurationPacket));

    Packet.ProprietaryHeader.Opcode           = CHANNEL_CONFIGURATION_OP_CODE__ADVANCED		 ;
    Packet.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

    SplitFieldsFromShort(Packet.BypassJitter,   pAttr->BypassJitter);
    SplitFieldsFromShort(Packet.FaxModemJitter, pAttr->FaxModemJitter);
    Packet.EchoCancelerCng							= pAttr->EchoCancelerCng;
    Packet.EchoCancelerToneDetector					= pAttr->EchoCancelerToneDetector;
	Packet.EchoCancelerAggressiveNlp				= pAttr->EchoCancelerAggressiveNlp;
	Packet.EchoCancelerNoisyEnvironmentMode			= pAttr->EchoCancelerNoisyEnvironmentMode;
	Packet.EchoCancelerReconvergenceSupport			= pAttr->EchoCancelerReconvergenceSupport;
	Packet.AcousticEchoCancelerEnable				= pAttr->AcousticEchoCancelerEnable;
    Packet.DecoderIbsOverhang_10msec				= pAttr->DecoderIbsOverhang_10msec;
    Packet.EncoderIbsOverhang_10msec				= pAttr->EncoderIbsOverhang_10msec;
    Packet.EchoCancelerDcRemover					= pAttr->EchoCancelerDcRemover;
	Packet.EchoCancellerAutomaticToneResetMode		= pAttr->EchoCancellerAutomaticToneResetMode;
	Packet.EchoCancellerAutomaticToneResetOnMuteMode= pAttr->EchoCancellerAutomaticToneResetOnMuteMode;
    Packet.EchoCancelerNlpThreshold					= pAttr->EchoCancelerNlpThreshold;
    Packet.HdlcInterFrameFillPattern				= pAttr->HdlcInterFrameFillPattern;
    Packet.HdlcMinimumEndFlags						= pAttr->HdlcMinimumEndFlags;
    Packet.HdlcMinimumMessageLength					= pAttr->HdlcMinimumMessageLength;
    Packet.HdlcSampleRate							= pAttr->HdlcSampleRate;
    Packet.MuteEncoderDuringDtmfGeneration			= pAttr->MuteEncoderDuringDtmfGeneration;
    Packet.SignalingJitter							= pAttr->SignalingJitter;
    Packet.PlaybackWaterMark_msec					= pAttr->PlaybackWaterMark_msec;
    Packet.PlaybackRequestEnable					= pAttr->PlaybackRequestEnable;
	Packet.Ss7LinkRate								= pAttr->Ss7LinkRate;
	Packet.HdlcTransferMedium						= pAttr->HdlcTransferMedium;									
	Packet.Ss7TransferMedium						= pAttr->Ss7TransferMedium;									
    Packet.EchoCancelerDuringDataBypass				= pAttr->EchoCancelerDuringDataBypass;
    Packet.EchoCancelerDuringFaxBypassDisable		= pAttr->EchoCancelerDuringFaxBypassDisable;
    Packet.EchoCancelerNlpDuringBypass				= pAttr->EchoCancelerNlpDuringBypass;
	Packet.EchoCancelerAutomaticHandler				= pAttr->EchoCancelerAutomaticHandler;
	Packet.PcmCngThreshold							= pAttr->PcmCngThreshold;
	Packet.DataBypassOutputGain						= pAttr->DataBypassOutputGain;
	Packet.FaxBypassOutputGain						= pAttr->FaxBypassOutputGain;
    Packet.DuaFrameMode								= pAttr->DuaFrameMode;
    Packet.DuaReceiveTimeout_x10msec				= pAttr->DuaReceiveTimeout_x10msec;
	Packet.AnswerDetectNoneSpeechEventsEnable		= pAttr->AnswerDetectNoneSpeechEventsEnable;
	SplitFieldsFromShort(Packet.AnswerDetectSilenceTime_x100ms,   pAttr->AnswerDetectSilenceTime_x100ms);
	SplitFieldsFromShort(Packet.AnswerDetectPreWorkDelay_x100ms,  pAttr->AnswerDetectPreWorkDelay_x100ms);
	if(pAttr->AcousticEchoCancelerEnable)
		Packet.InputGainLocation				  = INPUT_GAIN_LOCATION__AFTER_ECHO_CANCELER_AND_BOFORE_ENCODER;
	else
		Packet.InputGainLocation				  = pAttr->InputGainLocation;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Tac49xAdvancedChannelConfigurationPacket), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Tac49xAdvancedChannelConfigurationPacket);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/****************************************************************/
int Ac49xCloseChannelConfiguration(int Device, int Channel) /* The channel exits the active state upon receiving the Close Channel Command */
{
	int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader);
	Tac49xTxPacketParams	TxPacketParams;
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);
    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = CHANNEL_CONFIGURATION_OP_CODE__CLOSE_CHANNEL;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET               ;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/****************************************************************/
int Ac49xActivateRegularRtpChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdateRtpChannelConfigurationAttr *pAttr)
{
    return ActivateOrUpdateRtpChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__ACTIVATE_REGULAR_RTP, Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xActivateMediatedRtpChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdateRtpChannelConfigurationAttr *pAttr)
{
    return ActivateOrUpdateRtpChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__ACTIVATE_MEDIATED_RTP, Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xUpdateRegularRtpChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdateRtpChannelConfigurationAttr *pAttr)   
{
    return ActivateOrUpdateRtpChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__UPDATE_REGULAR_RTP, Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xUpdateMediatedRtpChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdateRtpChannelConfigurationAttr *pAttr)   
{
    return ActivateOrUpdateRtpChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__UPDATE_MEDIATED_RTP, Device, Channel, pAttr);
}

/****************************************************************/
#ifndef AC49X_DRIVRES_VERSION
int Ac49xCloseRegularRtpChannelConfiguration(int Device, int Channel)
#else
int Ac49xCloseRegularRtpChannelConfiguration(int Device, int Channel, Tac49xCloseRtpChannelConfigurationAttr *pAttr)
#endif
{
#ifndef AC49X_DRIVRES_VERSION
    return CloseRtpChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__CLOSE_REGULAR_RTP, Device, Channel);
#else
    return CloseRtpChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__CLOSE_REGULAR_RTP, Device, Channel, pAttr);
#endif
}



/****************************************************************/
int ActivateOrUpdate3GppIubChannelConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xActivateOrUpdate3GppIubChannelConfigurationAttr *pAttr)
{
    int													    i;
    Tac49xActivateOrUpdate3GppIubChannelConfigurationPacket Packet;
	Tac49xTxPacketParams								    TxPacketParams;
    int                                                     PacketSize = sizeof(Tac49xActivateOrUpdate3GppIubChannelConfigurationPacket);
    memset((void*)&Packet, 0, PacketSize);

    Packet.ProprietaryHeader.Opcode           = OpCode											;
    Packet.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET	;

    Packet.Crc                                  = pAttr->Crc;
    Packet.SilenceCompression                   = pAttr->SilenceCompression;
    Packet.TransmitNoDataPacketsDuringSilence   = pAttr->TransmitNoDataPacketsDuringSilence;
    Packet.LocalRate                            = pAttr->LocalRate;
    Packet.BrokenConnectionActivationMode       = pAttr->BrokenConnectionActivationMode;
    SplitFieldsFromShort(Packet.BrokenConnectionEventTimeout_x20msec, pAttr->BrokenConnectionEventTimeout_x20msec);

    for(i=0; i<MAX_NUMBER_OF_3GPP_IUB_TFIS; i++)
        {
        Packet.Tfi[i].Tfi0  = pAttr->Tfi[i].Tfi0;
        Packet.Tfi[i].Tfi1  = pAttr->Tfi[i].Tfi1;
        Packet.Tfi[i].Tfi2  = pAttr->Tfi[i].Tfi2;
        Packet.Tfi[i].Valid = pAttr->Tfi[i].Valid;
        }

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif
    
#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel              = Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/****************************************************************/
int Ac49xActivate3GppIubChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdate3GppIubChannelConfigurationAttr *pAttr)
{
    return ActivateOrUpdate3GppIubChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__ACTIVATE_3GPP_IUB, Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xUpdate3GppIubChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdate3GppIubChannelConfigurationAttr *pAttr)
{
    return ActivateOrUpdate3GppIubChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__UPDATE_3GPP_IUB, Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xClose3GppIubChannel(int Device, int Channel)
{
    Tac49xPacket                Packet;
	Tac49xTxPacketParams		TxPacketParams;
    int                         PacketSize = sizeof(Tac49xHpiPacketHeader)+sizeof(Tac49xProprietaryPacketHeader);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode		 = CHANNEL_CONFIGURATION_OP_CODE__CLOSE_3GPP_IUB;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
    TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/****************************************************************/
#ifndef AC49X_DRIVRES_VERSION
int Ac49xCloseMediatedRtpChannelConfiguration(int Device, int Channel)
#else
int Ac49xCloseMediatedRtpChannelConfiguration(int Device, int Channel, Tac49xCloseRtpChannelConfigurationAttr *pAttr)
#endif
{
#ifndef AC49X_DRIVRES_VERSION
    return CloseRtpChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__CLOSE_MEDIATED_RTP, Device, Channel);
#else
    return CloseRtpChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__CLOSE_MEDIATED_RTP, Device, Channel, pAttr);
#endif
}

/****************************************************************/
int ActivateOrUpdateAtmChannelConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xActivateOrUpdateAtmChannelConfigurationAttr *pAttr)
{
    Tac49xActivateOrUpdateAtmChannelConfigurationPacket Packet;
	Tac49xTxPacketParams								TxPacketParams;
	int													ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);

    memset((void*)&Packet, 0, sizeof(Tac49xActivateOrUpdateAtmChannelConfigurationPacket));
    Packet.ProprietaryHeader.Opcode           = OpCode;
    Packet.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

    Packet.Protocol							= pAttr->Protocol ;
    Packet.AtmAdaptionLayer2RxChannelId		= pAttr->AtmAdaptionLayer2RxChannelId  ;
    Packet.AtmAdaptionLayer2TxChannelId		= pAttr->AtmAdaptionLayer2TxChannelId  ;
    Packet.ProfileGroup						= pAttr->ProfileGroup                  ;
    Packet.ProfileId						= pAttr->ProfileId                     ;
    Packet.SilenceCompressionMode			= pAttr->SilenceCompressionMode        ;
    Packet.G711CoderType					= pAttr->G711CoderType                 ;
	Packet.BrokenConnectionActivationMode	= pAttr->BrokenConnectionActivationMode;
    Packet.ProfileEntry						= pAttr->ProfileEntry                  ;
	SplitFieldsFromShort(Packet.BrokenConnectionEventTimeout_x20msec, pAttr->BrokenConnectionEventTimeout_x20msec);

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Tac49xActivateOrUpdateAtmChannelConfigurationPacket), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
    TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Tac49xActivateOrUpdateAtmChannelConfigurationPacket);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/****************************************************************/
int CloseRegularAtmChannelConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel)
{
	Tac49xTxPacketParams	TxPacketParams;
	int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = OpCode;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/****************************************************************/
int Ac49xActivateRegularAtmChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdateAtmChannelConfigurationAttr	*pAttr)
{
    return ActivateOrUpdateAtmChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__REGULAR_CHANNEL_ACTIVATE_ATM, Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xUpdateRegularAtmChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdateAtmChannelConfigurationAttr	*pAttr)
{
    return ActivateOrUpdateAtmChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__REGULAR_CHANNEL_UPDATE_ATM, Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xCloseRegularAtmChannelConfiguration(int Device, int Channel)
{
	return CloseRegularAtmChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__REGULAR_CHANNEL_CLOSE_ATM, Device, Channel);
}

/****************************************************************/
int Ac49xActivateMediatedAtmChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdateAtmChannelConfigurationAttr	*pAttr)
{
    return ActivateOrUpdateAtmChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__MEDIATED_CHANNEL_ACTIVATE_ATM, Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xUpdateMediatedAtmChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdateAtmChannelConfigurationAttr	*pAttr)
{
    return ActivateOrUpdateAtmChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__MEDIATED_CHANNEL_UPDATE_ATM, Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xCloseMediatedAtmChannelConfiguration(int Device, int Channel)
{
	return CloseRegularAtmChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__MEDIATED_CHANNEL_CLOSE_ATM, Device, Channel);
}

/****************************************************************/
int Ac49xMediatedChannelConfiguration(int Device, int Channel, Tac49xOpenOrUpdateChannelConfigurationAttr *pAttr) 
{
    return OpenOrUpdateChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__MEDIATED_CHANNEL, Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xOpenChannelConfiguration(int Device, int Channel, Tac49xOpenOrUpdateChannelConfigurationAttr *pAttr)  /* The channel enters Active state upon receiving the Open Channel Command */
{
    return OpenOrUpdateChannelConfiguration(CHANNEL_CONFIGURATION_OP_CODE__OPEN, Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xT38ChannelConfiguration(int Device, int Channel, Tac49xT38ConfigurationAttr *pAttr)
{
    Tac49xT38ConfigurationPacket Packet;
	Tac49xTxPacketParams		 TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Tac49xT38ConfigurationPacket));

    Packet.ProprietaryHeader.Opcode			= CHANNEL_CONFIGURATION_OP_CODE__T38;
    Packet.ProprietaryHeader.PacketCategory = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

	Packet.PacketizationPeriod				= pAttr->PacketizationPeriod			;
	Packet.RtpEncapsulation					= pAttr->RtpEncapsulation				;
	Packet.JbigImageTranscoding				= pAttr->JbigImageTranscoding			;
	Packet.T4MmrTranscoding					= pAttr->T4MmrTranscoding				;
	Packet.T4FillBitRemoval					= pAttr->T4FillBitRemoval				;
	Packet.MaxDatagramSize_In4BytesStep		= pAttr->MaxDatagramSize_In4BytesStep	;
	Packet.T38Version						= pAttr->T38Version						;
	Packet.UdpErrorCorrectionMethod			= pAttr->UdpErrorCorrectionMethod		;
	Packet.ImageDataRedundancyLevel			= pAttr->ImageDataRedundancyLevel		;
	Packet.T30ControlDataRedundancyLevel	= pAttr->T30ControlDataRedundancyLevel  ;
	Packet.NoOperationPeriod				= pAttr->NoOperationPeriod				;


#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Tac49xT38ConfigurationPacket), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
    TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Tac49xT38ConfigurationPacket);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/****************************************************************/

int Ac49xActivateFaxRelayCommand(int Device, int Channel, Tac49xActivateFaxRelayAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
	int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xActivateFaxRelayPayload);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = CHANNEL_CONFIGURATION_OP_CODE__ACTIVATE_FAX_RELAY;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET           ;

	Packet.u.ProprietaryPayload.u.Command.u.ActivateFaxRelay.SwitchToFax = pAttr->SwitchToFax;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
    TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/****************************************************************/
int Ac49xDeactivateFaxRelayCommand(int Device, int Channel)
{
	Tac49xTxPacketParams	TxPacketParams;
	int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = CHANNEL_CONFIGURATION_OP_CODE__DEACTIVATE_FAX_RELAY;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET           ;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/****************************************************************/
int Ac49xActivateOrDeactivate3WayConferenceConfiguration(int Device, int Channel, Tac49xActivateOrDeactivate3WayConferenceConfigurationAttr *pAttr)
{
    Tac49xActivateOrDeactivate3WayConferenceConfigurationPacket Packet;
	Tac49xTxPacketParams										TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Tac49xActivateOrDeactivate3WayConferenceConfigurationPacket));

    Packet.ProprietaryHeader.Opcode         = CHANNEL_CONFIGURATION_OP_CODE__ACTIVATE_3_WAY_CONFERENCE;
    Packet.ProprietaryHeader.PacketCategory = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET			;
	Packet.ConferenceChannelMate			= pAttr->ConferenceChannelMate			;
	Packet.Mode								= pAttr->Mode							;
	Packet.Conference1TransmitPayloadType	= pAttr->Conference1TransmitPayloadType	;
	Packet.Conference2TransmitPayloadType	= pAttr->Conference2TransmitPayloadType	;
	SplitFieldsFromShort(Packet.Conference1TransmitSequenceNumber,	pAttr->Conference1TransmitPayloadType);
	SplitFieldsFromShort(Packet.Conference2TransmitSequenceNumber,	pAttr->Conference2TransmitPayloadType);
	SplitFieldsFromLong( Packet.Conference1RtpSsrc,					pAttr->Conference1RtpSsrc);
	SplitFieldsFromLong( Packet.Conference2RtpSsrc,					pAttr->Conference2RtpSsrc);

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Tac49xActivateOrDeactivate3WayConferenceConfigurationPacket), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Tac49xActivateOrDeactivate3WayConferenceConfigurationPacket);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}


/****************************************************************/
int Channel3GppUserPlaneInitializationConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49x3GppUserPlaneInitializationConfigurationAttr *pAttr)
{
	int i;
    Tac49x3GppUserPlaneInitializationConfigurationPacket Packet;
	Tac49xTxPacketParams								 TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Tac49x3GppUserPlaneInitializationConfigurationPacket));

    Packet.ProprietaryHeader.Opcode         = OpCode;
    Packet.ProprietaryHeader.PacketCategory = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;
	Packet.Mode								= pAttr->Mode						;						
	Packet.SupportModeType					= pAttr->SupportModeType			;			
	Packet.DeliveryOfErroneousSDUs			= pAttr->DeliveryOfErroneousSDUs	;	
	Packet.ProtocolDataUnitType				= pAttr->ProtocolDataUnitType		;
	Packet.ErrorTransmissionDisable			= pAttr->ErrorTransmissionDisable	;
	Packet.LocalRate						= pAttr->LocalRate					;			
	Packet.RemoteRate						= pAttr->RemoteRate					;
	Packet.G711CoderType					= pAttr->G711CoderType				;
	Packet.SupporetedUserPlaneProtocolVersion_Release99 = pAttr->SupporetedUserPlaneProtocolVersion_Release99;			
	Packet.SupporetedUserPlaneProtocolVersion_Release4 	= pAttr->SupporetedUserPlaneProtocolVersion_Release4;	
	Packet.SupporetedUserPlaneProtocolVersion_Release5 	= pAttr->SupporetedUserPlaneProtocolVersion_Release5;	
	Packet.RateControlAcknowledgeTimeout	= pAttr->RateControlAcknowledgeTimeout;	
	Packet.InitializationAcknowledgeTimeout	= pAttr->InitializationAcknowledgeTimeout;

	for(i=0; i<MAX_NUMBER_OF_3GPP_RFCI; i++)
		{
		Packet.Rfci[i].Valid				= pAttr->Rfci[i].Valid;
		Packet.Rfci[i].FieldOfRate			= pAttr->Rfci[i].FieldOfRate;
		}
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Tac49x3GppUserPlaneInitializationConfigurationPacket), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Tac49x3GppUserPlaneInitializationConfigurationPacket);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/****************************************************************/
int Ac49xRegularChannel3GppUserPlaneInitializationConfiguration(int Device, int Channel, Tac49x3GppUserPlaneInitializationConfigurationAttr *pAttr)
{
	return Channel3GppUserPlaneInitializationConfiguration(CHANNEL_CONFIGURATION_OP_CODE__REGULAR_CHANNEL_3GPP_UP_INITIALIZATION, Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xMediatedChannel3GppUserPlaneInitializationConfiguration(int Device, int Channel, Tac49x3GppUserPlaneInitializationConfigurationAttr *pAttr)
{
	return Channel3GppUserPlaneInitializationConfiguration(CHANNEL_CONFIGURATION_OP_CODE__MEDIATED_CHANNEL_3GPP_UP_INITIALIZATION, Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xRfc3267AmrInitializationConfiguration(int Device, int Channel, Tac49xRfc3267AmrInitializationConfigurationAttr	*pAttr)
{
    int i;
    Tac49xRfc3267AmrInitializationConfigurationPacket Packet;
	Tac49xTxPacketParams							  TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Tac49xRfc3267AmrInitializationConfigurationPacket));

    Packet.ProprietaryHeader.Opcode         = CHANNEL_CONFIGURATION_OP_CODE__RFC3267_AMR_INITIALIZATION;
    Packet.ProprietaryHeader.PacketCategory = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET		;
	Packet.OctetAlignedEnable			= pAttr->OctetAlignedEnable	;						
	Packet.ChangeModeNeighbor			= pAttr->ChangeModeNeighbor	;			
	Packet.CrcEnable					= pAttr->CrcEnable			;	
	Packet.ModeChangePeriod				= pAttr->ModeChangePeriod	;		
	Packet.LocalRate					= pAttr->LocalRate			;			
	Packet.RemoteRateEnable				= pAttr->RemoteRateEnable	;			
	Packet.RemoteRate					= pAttr->RemoteRate			;
	Packet.Coder						= pAttr->Coder				;
    Packet.RedundancyLevel              = pAttr->RedundancyLevel    ;
    Packet.AutomaticRateControlMode     = pAttr->AutomaticRateControlMode;
    Packet.TransmitNoDataPackets        = pAttr->TransmitNoDataPackets;
    Packet.NumberOfCodecsPolicies       = pAttr->NumberOfCodecsPolicies;
    SplitFieldsFromShort(Packet.DelayHandoutHysteresis_x1msec, pAttr->DelayHandoutHysteresis_x1msec);
    SplitFieldsFromShort(Packet.DelayHandoutThreshold_x1msec,  pAttr->DelayHandoutThreshold_x1msec);

    for(i=0; i<pAttr->NumberOfCodecsPolicies; i++)
        {
        Packet.aCodecPolicies[i].FrameLossRatioThreshold  = pAttr->aCodecPolicies[i].FrameLossRatioThreshold;
        Packet.aCodecPolicies[i].FrameLossRatioHysteresis = pAttr->aCodecPolicies[i].FrameLossRatioHysteresis;
        Packet.aCodecPolicies[i].Rate                     = (U16)pAttr->aCodecPolicies[i].Rate;
        Packet.aCodecPolicies[i].RedundancyLevel          = pAttr->aCodecPolicies[i].RedundancyLevel;
        }

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Tac49xRfc3267AmrInitializationConfigurationPacket), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;

	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Tac49xRfc3267AmrInitializationConfigurationPacket);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/****************************************************************/
int PacketCableProtectionConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xPacketCableProtectionConfigurationAttr *pAttr)
{
    Tac49xMediaProtectionConfigurationPacket	Packet;
	Tac49xTxPacketParams						TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Packet));

    Packet.ProprietaryHeader.Opcode					= OpCode  ;
    Packet.ProprietaryHeader.PacketCategory			= PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

	Packet.ProtectionMode										= MEDIA_PROTECTION_MODE__PACKET_CABLE  						;
	Packet.Direction											= pAttr->Direction					        			    ;
	Packet.u.PacketCable.RtpAuthentication						= pAttr->RtpAuthentication			        			    ;
	Packet.u.PacketCable.RtpEncryption							= pAttr->RtpEncryption				        			    ;
	Packet.u.PacketCable.RtcpAuthentication						= pAttr->RtcpAuthentication			        			    ;
	Packet.u.PacketCable.RtcpEncryption							= pAttr->RtcpEncryption				        			    ;

	Packet.u.PacketCable.AntiReplayEnable						= pAttr->AntiReplayEnable				        			;
	Packet.u.PacketCable.NumberOfRtpTimeStampWrapAround			= pAttr->NumberOfRtpTimeStampWrapAround						;
    Packet.u.PacketCable.RtpAuthenticationMode					= pAttr->RtpAuthenticationMode               				;
    Packet.u.PacketCable.RtcpAuthenticationMode					= pAttr->RtcpAuthenticationMode              				;
    Packet.u.PacketCable.RtpCipherKeySize		   				= pAttr->RtpCipherKeySize		   			    	        ;
    Packet.u.PacketCable.RtcpCipherKeySize		   				= pAttr->RtcpCipherKeySize		   			 		        ;
    Packet.u.PacketCable.RtpMessageAuthenticationCodeKeySize	= pAttr->RtpMessageAuthenticationCodeKeySize    		    ;
    Packet.u.PacketCable.RtcpMessageAuthenticationCodeKeySize	= pAttr->RtcpMessageAuthenticationCodeKeySize			    ;
    Packet.u.PacketCable.RtpDigestSize	  		   				= pAttr->RtpDigestSize	  		   	            		    ;
    Packet.u.PacketCable.RtcpDigestSize	  		   				= pAttr->RtcpDigestSize	  		   	         			    ;
	SplitFieldsFromLong(Packet.u.PacketCable.RtpTimeStamp, pAttr->RtpTimeStamp);
    memcpy((void*)Packet.u.PacketCable.RtpCipherKey,                      (void*)pAttr->RtpCipherKey,                     MAX_KEY_SIZE__RTP_CIPHER                     );
    memcpy((void*)Packet.u.PacketCable.RtcpCipherKey,                     (void*)pAttr->RtcpCipherKey,                    MAX_KEY_SIZE__RTCP_CIPHER                    );
    memcpy((void*)Packet.u.PacketCable.RtpInitializationKey,              (void*)pAttr->RtpInitializationKey,             MAX_KEY_SIZE__RTP_INITIALIZATION             );
    memcpy((void*)Packet.u.PacketCable.RtpMessageAuthenticationCodeKey,   (void*)pAttr->RtpMessageAuthenticationCodeKey,  MAX_KEY_SIZE__RTP_MESSAGE_AUTHENTICATION_CODE);
    memcpy((void*)Packet.u.PacketCable.RtcpMessageAuthenticationCodeKey,  (void*)pAttr->RtcpMessageAuthenticationCodeKey, MAX_KEY_SIZE__RTCP_MESSAGE_AUTHENTICATION_CODE);
	SplitFieldsFromLong(Packet.u.PacketCable.RtcpSequenceNumber, pAttr->RtcpSequenceNumber);
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Packet), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Packet);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/****************************************************************/
int Ac49xRegularChannelPacketCableProtectionConfiguration(int Device, int Channel, Tac49xPacketCableProtectionConfigurationAttr *pAttr)
{
	return PacketCableProtectionConfiguration(CHANNEL_CONFIGURATION_OP_CODE__REGULAR_CHANNEL_MEDIA_PROTECTION_INITIALIZATION, Device, Channel, pAttr);
}
/****************************************************************/
int Ac49xMediatedChannelPacketCableProtectionConfiguration(int Device, int Channel, Tac49xPacketCableProtectionConfigurationAttr *pAttr)
{
	return PacketCableProtectionConfiguration(CHANNEL_CONFIGURATION_OP_CODE__MEDIATED_CHANNEL_MEDIA_PROTECTION_INITIALIZATION, Device, Channel, pAttr);
}

/****************************************************************/
int SrtpProtectionConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xSrtpProtectionConfigurationAttr *pAttr)
{
    Tac49xMediaProtectionConfigurationPacket	Packet;
	Tac49xTxPacketParams						TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Packet));


    Packet.ProprietaryHeader.Opcode						= OpCode  ;
    Packet.ProprietaryHeader.PacketCategory				= PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET		        ;

	Packet.ProtectionMode								= MEDIA_PROTECTION_MODE__SRTP  							;
	Packet.Direction									= pAttr->Direction					        			;

	Packet.u.Srtp.RtpEncryption							= pAttr->RtpEncryption			        			    ;
	Packet.u.Srtp.RtpAuthentication						= pAttr->RtpAuthentication				        		;
	Packet.u.Srtp.RtcpEncryption						= pAttr->RtcpEncryption			        			    ;
	Packet.u.Srtp.RtcpAuthentication					= pAttr->RtcpAuthentication				        		;
	Packet.u.Srtp.AntiReplayEnable						= pAttr->AntiReplayEnable				        		;
    Packet.u.Srtp.KeyDerivationRatePower				= pAttr->KeyDerivationRatePower               			;

    Packet.u.Srtp.RtpMessageAuthenticationCodeKeySize	= pAttr->RtpMessageAuthenticationCodeKeySize         	;
    Packet.u.Srtp.RtpCipherKeySize						= pAttr->RtpCipherKeySize              					;
    Packet.u.Srtp.RtpAuthenticationMode		   			= pAttr->RtpAuthenticationMode		   			    	;
    Packet.u.Srtp.RtpEncryptionMode		   				= pAttr->RtpEncryptionMode		   			 		    ;
    Packet.u.Srtp.RtpDigestSize							= pAttr->RtpDigestSize    								;

    Packet.u.Srtp.RtcpMessageAuthenticationCodeKeySize	= pAttr->RtcpMessageAuthenticationCodeKeySize         	;
    Packet.u.Srtp.RtcpCipherKeySize						= pAttr->RtcpCipherKeySize              				;
    Packet.u.Srtp.RtcpAuthenticationMode		   		= pAttr->RtcpAuthenticationMode		   			    	;
    Packet.u.Srtp.RtcpEncryptionMode		   			= pAttr->RtcpEncryptionMode		   			 		    ;
    Packet.u.Srtp.RtcpDigestSize						= pAttr->RtcpDigestSize    								;

    Packet.u.Srtp.MasterKeySize	  		   				= pAttr->MasterKeySize	  		   	            		;
    Packet.u.Srtp.MasterSaltSize	  		   			= pAttr->MasterSaltSize	  		   	            		;
    Packet.u.Srtp.SessionSaltSize	  		   			= pAttr->SessionSaltSize;
	Packet.u.Srtp.MasterKeyIndexSize	  		   		= pAttr->MasterKeyIndexSize;

	SplitFieldsFromShort(Packet.u.Srtp.RtpInitSequenceNumber,	pAttr->RtpInitSequenceNumber);
	SplitFieldsFromLong( Packet.u.Srtp.RtpInitRoc,				pAttr->RtpInitRoc);
	SplitFieldsFromLong( Packet.u.Srtp.RtcpInitIndex,			pAttr->RtcpInitIndex);

    memcpy((void*)Packet.u.Srtp.MasterKey,		(void*)pAttr->MasterKey,		MAX_KEY_SIZE__MASTER);
    memcpy((void*)Packet.u.Srtp.MasterSalt,		(void*)pAttr->MasterSalt,		MAX_KEY_SIZE__MASTER_SALT);
	memcpy((void*)Packet.u.Srtp.MasterKeyIndex, (void*)pAttr->MasterKeyIndex,	MAX_MASTER_KEY_INDEX_SIZE);
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Packet), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
    TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Packet);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/****************************************************************/
int Ac49xRegularChannelSrtpProtectionConfiguration(int Device, int Channel, Tac49xSrtpProtectionConfigurationAttr *pAttr)
{
	return SrtpProtectionConfiguration(CHANNEL_CONFIGURATION_OP_CODE__REGULAR_CHANNEL_MEDIA_PROTECTION_INITIALIZATION, Device, Channel, pAttr);
}
/****************************************************************/
int Ac49xMediatedChannelSrtpProtectionConfiguration(int Device, int Channel, Tac49xSrtpProtectionConfigurationAttr *pAttr)
{
	return SrtpProtectionConfiguration(CHANNEL_CONFIGURATION_OP_CODE__MEDIATED_CHANNEL_MEDIA_PROTECTION_INITIALIZATION, Device, Channel, pAttr);
}

/****************************************************************/
#if   (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
int Ac49xCodecConfiguration(int Device, int Channel, Tac49xCodecConfigurationAttr *pAttr)
{
    Tac49xCodecConfigurationPacket Packet;
	Tac49xTxPacketParams		   TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Packet));

    Packet.ProprietaryHeader.Opcode             = CHANNEL_CONFIGURATION_OP_CODE__CODEC_CONFIGURATION        ;
    Packet.ProprietaryHeader.PacketCategory     = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET		        ;

	Packet.AdcDitheringCircuit                 	 = pAttr->AdcDitheringCircuit                 	;
	Packet.BiasPinOutputVoltage                	 = pAttr->BiasPinOutputVoltage                	;
	Packet.SpeakerGain                         	 = pAttr->SpeakerGain                         	;
	Packet.BandPassFilter                      	 = pAttr->BandPassFilter                      	;
	Packet.MuteHandset		                     = pAttr->MuteHandset		                    ;
	Packet.SamplingRate							 = pAttr->SamplingRate							;
	Packet.MuteHeadset		                     = pAttr->MuteHeadset		                    ;
	Packet.MuteLineOutput		                 = pAttr->MuteLineOutput		                ;
	Packet.MuteSpeaker		                     = pAttr->MuteSpeaker		                    ;
	Packet.HeadsetInputToOutput	                 = pAttr->HeadsetInputToOutput	                ;
	Packet.HandsetInputToOutput	                 = pAttr->HandsetInputToOutput	                ;
	Packet.CallerIdInputSelect	                 = pAttr->CallerIdInputSelect	                ;
	Packet.LineInputSelect		                 = pAttr->LineInputSelect		                ;
	Packet.MicrophoneInputSelect               	 = pAttr->MicrophoneInputSelect               	;
	Packet.HandsetInputSelect	                 = pAttr->HandsetInputSelect	                ;
	Packet.HeadsetInputSelect	                 = pAttr->HeadsetInputSelect	                ;
	Packet.AnalogSidetoneGain	                 = pAttr->AnalogSidetoneGain	                ;
	Packet.DigitalSidetoneGain	                 = pAttr->DigitalSidetoneGain	                ;
	Packet.AnalogSidetoneOutputSelectForHeadset	 = pAttr->AnalogSidetoneOutputSelectForHeadset	;
	Packet.AnalogSidetoneOutputSelectForHandset	 = pAttr->AnalogSidetoneOutputSelectForHandset	;
	Packet.SpeakerSelect		                 = pAttr->SpeakerSelect		                	;
	Packet.LineOutputSelect		            	 = pAttr->LineOutputSelect		            	;
	Packet.HeadsetOutputSelect		             = pAttr->HeadsetOutputSelect		            ;
	Packet.HandsetOutputSelect		             = pAttr->HandsetOutputSelect		            ;
	Packet.DacOutputPgaGain		                 = pAttr->DacOutputPgaGain		                ;
	Packet.AdcInputPgaGain		                 = pAttr->AdcInputPgaGain		                ;
	Packet.AnalogLoopback						 = pAttr->AnalogLoopback						;
	Packet.DigitalLoopback						 = pAttr->DigitalLoopback						;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Packet), TRANSFER_MEDIUM__HOST);
#endif

	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Packet);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
}
#endif/*((AC49X_DEVICE_TYPE == AC494_DEVICE) || (AC49X_DEVICE_TYPE == AC495_DEVICE) || (AC49X_DEVICE_TYPE == AC496_DEVICE) || (AC49X_DEVICE_TYPE == AC496E_DEVICE) || (AC49X_DEVICE_TYPE == AC497_DEVICE))*/

/****************************************************************/
#if   (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
int Ac49xAcousticEchoCancelerConfiguration(int Device, int Channel, Tac49xAcousticEchoCancelerConfigurationAttr *pAttr)
{
    Tac49xAcousticEchoCancelerConfigurationPacket Packet;
	Tac49xTxPacketParams						 TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Packet));

    Packet.ProprietaryHeader.Opcode             = CHANNEL_CONFIGURATION_OP_CODE__ACOUSTIC_ECHO_CANCELER     ;
    Packet.ProprietaryHeader.PacketCategory     = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET		        ;

	Packet.CommandIndex					= 0;
	Packet.SupportedMode				= pAttr->SupportedMode				;
	Packet.SaturationAttenuator			= pAttr->SaturationAttenuator		;
	Packet.ResidualOverwriteByInput		= pAttr->ResidualOverwriteByInput	;
	Packet.SoftNlp						= pAttr->SoftNlp					;
	Packet.Cng							= pAttr->Cng						;
	Packet.ReinitAfterToneSignal		= pAttr->ReinitAfterToneSignal		;
	Packet.HalfDuplexMode				= pAttr->HalfDuplexMode	;
	Packet.AntiHowlingProcessor			= pAttr->AntiHowlingProcessor		;
	Packet.EchoReturnLoss				= pAttr->EchoReturnLoss				;
	Packet.MaxActiveFilterLength		= pAttr->MaxActiveFilterLength		;
	Packet.TailLength					= pAttr->TailLength					;
	Packet.CoefficientsThreshold		= pAttr->CoefficientsThreshold		;
	Packet.NlpActivationThreshold		= pAttr->NlpActivationThreshold		;
	Packet.SaturationThreshold			= pAttr->SaturationThreshold		;
    Packet.HighPassFilter               = pAttr->HighPassFilter             ;
    Packet.Decimation                   = pAttr->Decimation                 ;
    Packet.IntensiveMode                = pAttr->IntensiveMode              ;
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Packet), TRANSFER_MEDIUM__HOST);
#endif
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Packet);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
}
#endif/*((AC49X_DEVICE_TYPE == AC494_DEVICE) || (AC49X_DEVICE_TYPE == AC495_DEVICE) || (AC49X_DEVICE_TYPE == AC496_DEVICE) || (AC49X_DEVICE_TYPE == AC496E_DEVICE) || (AC49X_DEVICE_TYPE == AC497_DEVICE))*/

/****************************************************************/
int Ac49xRfc3558InitializationConfiguration(int Device, int Channel, Tac49xRfc3558InitializationConfigurationAttr *pAttr)
{
    Tac49xRfc3558InitializationConfigurationPacket Packet;
	Tac49xTxPacketParams						   TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Packet));

    Packet.ProprietaryHeader.Opcode             = CHANNEL_CONFIGURATION_OP_CODE__RFC3558_INITIALIZATION;
    Packet.ProprietaryHeader.PacketCategory     = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

	Packet.LocalRateMode								= pAttr->LocalRateMode;
	Packet.RemoteRateMode								= pAttr->RemoteRateMode;
	Packet.PacketFormat									= pAttr->PacketFormat;
	Packet.LocalRateModeControlledByRemoteSideEnable	= pAttr->LocalRateModeControlledByRemoteSideEnable;
	Packet.SilenceCompressionExtensionModeEnable		= pAttr->SilenceCompressionExtensionModeEnable;
	Packet.LocalMaxRate									= pAttr->LocalMaxRate;
	Packet.LocalMinRate									= pAttr->LocalMinRate;
	Packet.NoiseSuppressionFilterEnable					= pAttr->NoiseSuppressionFilterEnable;
	Packet.Hangover										= pAttr->Hangover;

	SplitFieldsFromShort(Packet.DtxMin, pAttr->DtxMin);
	SplitFieldsFromShort(Packet.DtxMax, pAttr->DtxMax);
	
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Packet), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
    TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Packet);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/****************************************************************/
int Ac49xG729EvInitializationConfiguration(int Device, int Channel, Tac49xG729EvInitializationConfigurationAttr *pAttr)
{
    Tac49xG729EvInitializationConfigurationPacket Packet;
	Tac49xTxPacketParams						  TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Packet));

    Packet.ProprietaryHeader.Opcode             = CHANNEL_CONFIGURATION_OP_CODE__G729EV_INITIALIZATION;
    Packet.ProprietaryHeader.PacketCategory     = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

	Packet.AutomaticLocalRateControlMode			= pAttr->AutomaticLocalRateControlMode;
	Packet.LocalBitRate								= pAttr->LocalBitRate;
	Packet.MaxRemoteBitRate							= pAttr->MaxRemoteBitRate;
    Packet.MaxSessionBitRate                        = pAttr->MaxSessionBitRate;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Packet), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
    TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Packet);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/****************************************************************/
int RtcpXrConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xRtcpXrConfigurationAttr *pAttr)
{
    Tac49xRtcpXrConfigurationPacket Packet;
	Tac49xTxPacketParams			TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Packet));

    Packet.ProprietaryHeader.Opcode             = OpCode;
    Packet.ProprietaryHeader.PacketCategory     = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

	Packet.BurstAlertSetting			 = pAttr->BurstAlertSetting;
	Packet.DelayAlertSetting			 = pAttr->DelayAlertSetting;
	Packet.LowVoiceQualityAlertSetting	 = pAttr->LowVoiceQualityAlertSetting;
	Packet.MaximumBurstLossRateThreshold = pAttr->MaximumBurstLossRateThreshold;
	SplitFieldsFromShort(Packet.MaximumBurstDurationThreshold,  pAttr->MaximumBurstDurationThreshold);
	SplitFieldsFromShort(Packet.MaximumOneWayDelayThreshold,	pAttr->MaximumOneWayDelayThreshold);
	SplitFieldsFromShort(Packet.LowVoiceQualityRValueThreshold, pAttr->LowVoiceQualityRValueThreshold);

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Packet), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
    TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Packet);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/****************************************************************/
int Ac49xRegularChannelRtcpXrConfiguration(int Device, int Channel, Tac49xRtcpXrConfigurationAttr *pAttr)
{
	return RtcpXrConfiguration(CHANNEL_CONFIGURATION_OP_CODE__REGULAR_CHANNEL_RTCP_XR, Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xMediatedChannelRtcpXrConfiguration(int Device, int Channel, Tac49xRtcpXrConfigurationAttr *pAttr)
{
	return RtcpXrConfiguration(CHANNEL_CONFIGURATION_OP_CODE__MEDIATED_CHANNEL_RTCP_XR, Device, Channel, pAttr);
}

/****************************************************************/
int OpenOrUpdate4WayConferenceConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, Tac49xOpenOrUpdate4WayConferenceConfigurationAttr *pAttr)
{
    Tac49xOpenOrUpdate4WayConferenceConfigurationPacket Packet;
	Tac49xTxPacketParams								TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Packet));

    Packet.ProprietaryHeader.Opcode             = OpCode;
    Packet.ProprietaryHeader.PacketCategory     = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

	Packet.ConferenceId			= pAttr->ConferenceId;
	Packet.Member3HearsMember3	= pAttr->Member3HearsMember3;
	Packet.Member3HearsMember2	= pAttr->Member3HearsMember2;
	Packet.Member3HearsMember1	= pAttr->Member3HearsMember1;
	Packet.Member3HearsMember0	= pAttr->Member3HearsMember0;
	Packet.Member2HearsMember3	= pAttr->Member2HearsMember3;
	Packet.Member2HearsMember2	= pAttr->Member2HearsMember2;
	Packet.Member2HearsMember1	= pAttr->Member2HearsMember1;
	Packet.Member2HearsMember0	= pAttr->Member2HearsMember0;
	Packet.Member1HearsMember3	= pAttr->Member1HearsMember3;
	Packet.Member1HearsMember2	= pAttr->Member1HearsMember2;
	Packet.Member1HearsMember1	= pAttr->Member1HearsMember1;
	Packet.Member1HearsMember0	= pAttr->Member1HearsMember0;
	Packet.Member0HearsMember3	= pAttr->Member0HearsMember3;
	Packet.Member0HearsMember2	= pAttr->Member0HearsMember2;
	Packet.Member0HearsMember1	= pAttr->Member0HearsMember1;
	Packet.Member0HearsMember0	= pAttr->Member0HearsMember0;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Packet), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
    TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= 0;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Packet);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/****************************************************************/
int Ac49xOpen4WayConferenceConfiguration(int Device, Tac49xOpenOrUpdate4WayConferenceConfigurationAttr *pAttr)
{
	return OpenOrUpdate4WayConferenceConfiguration(CHANNEL_CONFIGURATION_OP_CODE__4_WAY_CONFERENCE_OPEN, Device, pAttr);
}

/****************************************************************/
int Ac49xUpdate4WayConferenceConfiguration(int Device, Tac49xOpenOrUpdate4WayConferenceConfigurationAttr *pAttr)
{
	return OpenOrUpdate4WayConferenceConfiguration(CHANNEL_CONFIGURATION_OP_CODE__4_WAY_CONFERENCE_UPDATE, Device, pAttr);
}

/****************************************************************/
int Ac49xClose4WayConferenceConfiguration(int Device, Tac49xClose4WayConferenceConfigurationAttr *pAttr)
{
    Tac49xClose4WayConferenceConfigurationPacket Packet;
	Tac49xTxPacketParams								TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Packet));

    Packet.ProprietaryHeader.Opcode             = CHANNEL_CONFIGURATION_OP_CODE__4_WAY_CONFERENCE_CLOSE;
    Packet.ProprietaryHeader.PacketCategory     = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

	Packet.ConferenceId			= pAttr->ConferenceId;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Packet), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
    TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= 0;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Packet);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/****************************************************************/
int Ac49xAdd4WayConferenceLegConfiguration(int Device, int Channel, Tac49xAdd4WayConferenceLegConfigurationAttr *pAttr)
{
    Tac49xAdd4WayConferenceLegConfigurationPacket		Packet;
	Tac49xTxPacketParams								TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Packet));

    Packet.ProprietaryHeader.Opcode             = CHANNEL_CONFIGURATION_OP_CODE__4_WAY_CONFERENCE_ADD_LEG;
    Packet.ProprietaryHeader.PacketCategory     = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

	Packet.LegType			= pAttr->LegType;
	Packet.MemberId			= pAttr->MemberId;
	Packet.ConferenceId		= pAttr->ConferenceId;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Packet), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
    TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Packet);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/****************************************************************/
int Ac49xRemove4WayConferenceLegConfiguration(int Device, int Channel, Tac49xRemove4WayConferenceLegConfigurationAttr *pAttr)
{
    Tac49xAdd4WayConferenceLegConfigurationPacket		Packet;
	Tac49xTxPacketParams								TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Packet));

    Packet.ProprietaryHeader.Opcode             = CHANNEL_CONFIGURATION_OP_CODE__4_WAY_CONFERENCE_REMOVE_LEG;
    Packet.ProprietaryHeader.PacketCategory     = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

	Packet.LegType			= pAttr->LegType;
	Packet.MemberId			= pAttr->MemberId;
	Packet.ConferenceId		= pAttr->ConferenceId;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Packet), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
    TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Packet);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/****************************************************************/
int Ac49xUemClipInitializationConfiguration(int Device, int Channel, Tac49xUemClipInitializationAttr *pAttr)
{
    Tac49xUemClipInitializationPacket		Packet;
	Tac49xTxPacketParams					TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Packet));

    Packet.ProprietaryHeader.Opcode             = CHANNEL_CONFIGURATION_OP_CODE__UEM_CLIP_INITIALIZATION;
    Packet.ProprietaryHeader.PacketCategory     = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

	Packet.RtpTimeStampResolution = pAttr->RtpTimeStampResolution;
	Packet.TransmitMode			  = pAttr->TransmitMode;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Packet), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
    TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Packet);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/****************************************************************/
int ActivateOrUpdateDataRelayConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xActivateOrUpdateDataRelayConfigurationAttr *pAttr)
{
    Tac49xActivateOrUpdateDataRelayConfigurationPacket  Packet;
	Tac49xTxPacketParams								TxPacketParams;
	int													ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);

    memset((void*)&Packet, 0, sizeof(Tac49xActivateOrUpdateDataRelayConfigurationPacket));
    Packet.ProprietaryHeader.Opcode           = OpCode;
    Packet.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

    Packet.MaxPayloadSizeForSprtTransportChannel2 = pAttr->MaxPayloadSizeForSprtTransportChannel2;
	Packet.MaxPayloadSizeForSprtTransportChannel0 = pAttr->MaxPayloadSizeForSprtTransportChannel0;
	Packet.MaxPayloadSizeForSprtTransportChannel3 = pAttr->MaxPayloadSizeForSprtTransportChannel3;
	Packet.MaxWindowSizeForSprtTransportChannel2  = pAttr->MaxWindowSizeForSprtTransportChannel2;
	Packet.RxPayloadTypeOfSprtPackets			  = pAttr->RxPayloadTypeOfSprtPackets;
	Packet.TxPayloadTypeOfSprtPackets			  = pAttr->TxPayloadTypeOfSprtPackets;
	Packet.SprtPacketsTransmitProtocol			  = pAttr->SprtPacketsTransmitProtocol;
	Packet.SprtPacketsReceiveProtocol			  = pAttr->SprtPacketsReceiveProtocol;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Tac49xActivateOrUpdateAtmChannelConfigurationPacket), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
    TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Tac49xActivateOrUpdateAtmChannelConfigurationPacket);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xActivateDataRelayConfiguration(int Device, int Channel, Tac49xActivateOrUpdateDataRelayConfigurationAttr	*pAttr)
{
    return ActivateOrUpdateDataRelayConfiguration(CHANNEL_CONFIGURATION_OP_CODE__DATA_RELAY_ACTIVATE, Device, Channel, pAttr);
}

/***************************************************************************************************************************/
int Ac49xUpdateDataRelayConfiguration(int Device, int Channel, Tac49xActivateOrUpdateDataRelayConfigurationAttr	*pAttr)
{
    return ActivateOrUpdateDataRelayConfiguration(CHANNEL_CONFIGURATION_OP_CODE__DATA_RELAY_UPDATE, Device, Channel, pAttr);
}

/***************************************************************************************************************************/
int Ac49xDeactivateDataRelayConfiguration(int Device, int Channel)
{
	Tac49xTxPacketParams	TxPacketParams;
 	int						ReturnedValue;
	int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader);

   	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = CHANNEL_CONFIGURATION_OP_CODE__DATA_RELAY_DEACTIVATE;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xAdvancedDataRelayConfiguration(int Device, int Channel, Tac49xAdvancedDataRelayConfigurationAttr *pAttr)
{
    Tac49xAdvancedDataRelayConfigurationPacket		Packet;
	Tac49xTxPacketParams							TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Packet));

    Packet.ProprietaryHeader.Opcode             = CHANNEL_CONFIGURATION_OP_CODE__DATA_RELAY_ADVANCED;
    Packet.ProprietaryHeader.PacketCategory     = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

	Packet.ModemTransmissionPower		  = pAttr->ModemTransmissionPower;
	Packet.V32ModulationTypeEnable		  = pAttr->V32ModulationTypeEnable;
	Packet.V34ModulationTypeEnable		  = pAttr->V34ModulationTypeEnable;
	Packet.ModemDiagnosticsEnable		  = pAttr->ModemDiagnosticsEnable;
	Packet.PcmLevelOfTransmissionOfPhase1 = pAttr->PcmLevelOfTransmissionOfPhase1;
	Packet.MaxTxBitRate					  = pAttr->MaxTxBitRate;
	Packet.MinTxBitRate					  = pAttr->MinTxBitRate;
	Packet.MaxRxBitRate					  = pAttr->MaxRxBitRate;
	Packet.MinRxBitRate					  = pAttr->MinRxBitRate;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Packet), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
    TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Packet);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/***************************************************************************************************************************/
int Ac49xG7111InitializationConfiguration(int Device, int Channel, Tac49xG7111InitializationAttr *pAttr)
{
    Tac49xG7111InitializationPacket		Packet;
	Tac49xTxPacketParams					TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Packet));

    Packet.ProprietaryHeader.Opcode             = CHANNEL_CONFIGURATION_OP_CODE__G711_WB_INITIALIZATION;
    Packet.ProprietaryHeader.PacketCategory     = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

	Packet.TransmitMode			  = pAttr->TransmitMode;
	Packet.RtpTimestampResolution = pAttr->RtpTimestampResolution;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Packet), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
    TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Packet);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/***************************************************************************************************************************/
int Ac49xMicrosoftRtaInitializationConfiguration(int Device, int Channel, Tac49xMicrosoftRtaInitializationAttr *pAttr)
{
    Tac49xMicrosoftRtaInitializationPacket	Packet;
	Tac49xTxPacketParams					TxPacketParams;
    memset((void*)&Packet, 0, sizeof(Packet));

    Packet.ProprietaryHeader.Opcode             = CHANNEL_CONFIGURATION_OP_CODE__MICROSOFT_RTA_INITIALIZATION;
    Packet.ProprietaryHeader.PacketCategory     = PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET;

	Packet.VariableRateSupport		  = pAttr->VariableRateSupport;
	Packet.JitterBufferMode			  = pAttr->JitterBufferMode;
	Packet.ForwardErrorCorrectionMode = pAttr->ForwardErrorCorrectionMode;
	

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, sizeof(Packet), TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
    TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= sizeof(Packet);
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    return Ac49xTransmitPacket(&TxPacketParams);
#endif
}

/***************************************************************************************************************************/

/***********************************\
*************************************
**    Read and Write Functions     **
*************************************
\***********************************/

/****************************************************************/
int SetRtcpParameters(Tac49xCommandPacketOpCode OpCode, int Device, int Channel, Tac49xSetRtcpParametersAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
    int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xSetRtcpParametersPayload);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);
   
    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode							= OpCode                  ;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory					= PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;
	Packet.u.ProprietaryPayload.u.Command.u.SetRtcpParameters.ParameterIndex		= pAttr->ParameterIndex;
	Packet.u.ProprietaryPayload.u.Command.u.SetRtcpParameters.ParameterValue_MswMsb = (unsigned short)((pAttr->ParameterValue&0xFF000000)>>24);
	Packet.u.ProprietaryPayload.u.Command.u.SetRtcpParameters.ParameterValue_MswLsb = (unsigned short)((pAttr->ParameterValue&0x00FF0000)>>16);
	Packet.u.ProprietaryPayload.u.Command.u.SetRtcpParameters.ParameterValue_LswMsb = (unsigned short)((pAttr->ParameterValue&0x0000FF00)>>8 );
	Packet.u.ProprietaryPayload.u.Command.u.SetRtcpParameters.ParameterValue_LswLsb = (unsigned short) (pAttr->ParameterValue&0x000000FF)     ;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/****************************************************************/
int Ac49xSetRegularRtcpParameters(int Device, int Channel, Tac49xSetRtcpParametersAttr *pAttr)
{
    return SetRtcpParameters(COMMAND_PACKET_OP_CODE__SET_REGULAR_CHANNEL_RTCP_PARAMETERS, Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xSetMediatedRtcpParameters(int Device, int Channel, Tac49xSetRtcpParametersAttr *pAttr)
{
    return SetRtcpParameters(COMMAND_PACKET_OP_CODE__SET_MEDIATED_CHANNEL_RTCP_PARAMETERS, Device, Channel, pAttr);
}

/****************************************************************/
int GetRtcpParameters(Tac49xCommandPacketOpCode OpCode, int Device, int Channel)
{
	Tac49xTxPacketParams	TxPacketParams;
    int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);
	
    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = OpCode                  ;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/****************************************************************/
int Ac49xGetRegularRtcpParameters(int Device, int Channel)
{
    return GetRtcpParameters(COMMAND_PACKET_OP_CODE__GET_REGULAR_CHANNEL_RTCP_PARAMETERS, Device, Channel);
}

/****************************************************************/
int Ac49xGetMediatedRtcpParameters(int Device, int Channel)
{
    return GetRtcpParameters(COMMAND_PACKET_OP_CODE__GET_MEDIATED_CHANNEL_RTCP_PARAMETERS, Device, Channel);
}

/****************************************************************/
int Ac49xSendIbsString(int Device, int Channel, Tac49xIbsStringAttr *pAttr)  
{
    int i;
	Tac49xTxPacketParams	TxPacketParams;
    int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xIbsStringCommandPayload);
	int						ReturnedValue;


	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode					= COMMAND_PACKET_OP_CODE__IBS_STRING	;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory			= PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET			;
    Packet.u.ProprietaryPayload.u.Command.u.IbsString.GeneratorRedirection	= pAttr->GeneratorRedirection		;
    Packet.u.ProprietaryPayload.u.Command.u.IbsString.NumberOfDigits		= pAttr->NumberOfDigits				;
    SplitFieldsFromLong(Packet.u.ProprietaryPayload.u.Command.u.IbsString.OnDuration   , pAttr->OnDuration   );
    SplitFieldsFromLong(Packet.u.ProprietaryPayload.u.Command.u.IbsString.OffDuration  , pAttr->OffDuration  );
    SplitFieldsFromLong(Packet.u.ProprietaryPayload.u.Command.u.IbsString.PauseDuration, pAttr->PauseDuration);
    for(i=0; i<MAX_IBS_STRING_SIGNALS; i++)
        {
        Packet.u.ProprietaryPayload.u.Command.u.IbsString.Signal[i].Pause      = pAttr->Signal[i].Pause      ;
        Packet.u.ProprietaryPayload.u.Command.u.IbsString.Signal[i].Digit      = pAttr->Signal[i].Digit      ;
        Packet.u.ProprietaryPayload.u.Command.u.IbsString.Signal[i].SystemCode = pAttr->Signal[i].SystemCode ;
        }
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/****************************************************************/
int Ac49xSendIbsStop(int Device, int Channel)
{
	Tac49xTxPacketParams	TxPacketParams;
	int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);
    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__IBS_STOP;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/****************************************************************/
int Ac49xSendExtendedIbsString(int Device, int Channel, Tac49xExtendedIbsStringAttr	*pAttr)
{
    return Ac49xSendExtendedIbsString_Format0(Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xSendExtendedIbsString_Format0(int Device, int Channel, Tac49xExtendedIbsString_Format0_Attr *pAttr)
{
    int i;
	Tac49xTxPacketParams	TxPacketParams;
	int						UnionSize = (sizeof(TacExtendedIbsStringCommand_Format0_Payload)<sizeof(TacExtendedIbsStringCommand_Format1_Payload)) ? sizeof(TacExtendedIbsStringCommand_Format1_Payload) : sizeof(TacExtendedIbsStringCommand_Format0_Payload);
	int						PacketSize = sizeof(Tac49xHpiPacketHeader)
                                       + sizeof(Tac49xProprietaryPacketHeader)
                                       + sizeof(Tac49xExtendedIbsStringCommandPayload) 
									   - UnionSize
									   + sizeof(TacExtendedIbsStringCommand_Format0_Payload)
                                       - sizeof(TacExtendedIbsStringCommand_Format0_SignalPayload)*(MAX_NUMBER_OF_EXTENDED_IBS_DIGITS__FORMAT_0 - pAttr->NumberOfDigits);
 	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);
    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__EXTENDED_IBS_STRING;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;
    Packet.u.ProprietaryPayload.ProprietaryHeader.Format		   = FORMAT__0;
    
    Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.GeneratorRedirection	= pAttr->GeneratorRedirection;
    Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.Repeat				= pAttr->Repeat;
    Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.NumberOfDigits		= pAttr->NumberOfDigits;
    SplitFieldsFromLong(Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.TotalDuration,	 pAttr->TotalDuration);
    SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.UserDefinedToneA, pAttr->UserDefinedToneA);
    SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.UserDefinedToneB, pAttr->UserDefinedToneB);
    SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.UserDefinedToneC, pAttr->UserDefinedToneC);
    Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.AmFactor				= pAttr->AmFactor;
    for(i=0; i<MAX_NUMBER_OF_EXTENDED_IBS_DIGITS__FORMAT_0; i++)
		{
		Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.u.Format0.Signal[i].Digit					= pAttr->Signal[i].Digit;
		Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.u.Format0.Signal[i].SignalLevel_minus_dbm	= pAttr->Signal[i].SignalLevel_minus_dbm;
		Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.u.Format0.Signal[i].SystemCode			= pAttr->Signal[i].SystemCode;
		SplitFieldsFromLong(Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.u.Format0.Signal[i].SignalDuration, pAttr->Signal[i].SignalDuration);
		}
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/****************************************************************/
int Ac49xSendExtendedIbsString_Format1(int Device, int Channel, Tac49xExtendedIbsString_Format1_Attr *pAttr)
{
    int i;
	Tac49xTxPacketParams	TxPacketParams;
	int						UnionSize = (sizeof(TacExtendedIbsStringCommand_Format0_Payload)<sizeof(TacExtendedIbsStringCommand_Format1_Payload)) ? sizeof(TacExtendedIbsStringCommand_Format1_Payload) : sizeof(TacExtendedIbsStringCommand_Format0_Payload);
	int						PacketSize = sizeof(Tac49xHpiPacketHeader)
                                       + sizeof(Tac49xProprietaryPacketHeader)
                                       + sizeof(Tac49xExtendedIbsStringCommandPayload)
									   - UnionSize
									   + sizeof(TacExtendedIbsStringCommand_Format1_Payload)
                                       - sizeof(TacExtendedIbsStringCommand_Format1_SignalPayload)*(MAX_NUMBER_OF_EXTENDED_IBS_DIGITS__FORMAT_1 - pAttr->NumberOfDigits);
 	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
   memset((void*)&Packet, 0, PacketSize);
    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__EXTENDED_IBS_STRING;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;
	Packet.u.ProprietaryPayload.ProprietaryHeader.Format		   = FORMAT__1;
    
    Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.GeneratorRedirection	= pAttr->GeneratorRedirection;
    Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.Repeat				= pAttr->Repeat;
    Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.NumberOfDigits		= pAttr->NumberOfDigits;
    SplitFieldsFromLong(Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.TotalDuration,	 pAttr->TotalDuration);
    SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.UserDefinedToneA, pAttr->UserDefinedToneA);
    SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.UserDefinedToneB, pAttr->UserDefinedToneB);
    SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.UserDefinedToneC, pAttr->UserDefinedToneC);
    Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.SignalLevel_minus_dbm = pAttr->SignalLevel_minus_dbm;
    Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.AmFactor				= pAttr->AmFactor;
    SplitFieldsFromLong(Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.u.Format1.OnDuration    ,pAttr->OnDuration);
    SplitFieldsFromLong(Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.u.Format1.OffDuration   ,pAttr->OffDuration);
    SplitFieldsFromLong(Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.u.Format1.PauseDuration ,pAttr->PauseDuration);
    for(i=0; i<MAX_NUMBER_OF_EXTENDED_IBS_DIGITS__FORMAT_1; i++)
		{
		Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.u.Format1.Signal[i].Digit					= pAttr->Signal[i].Digit;
		Packet.u.ProprietaryPayload.u.Command.u.ExtendedIbsString.u.Format1.Signal[i].SystemCode			= pAttr->Signal[i].SystemCode;
		}
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/****************************************************************/
int Ac49xStartTrunkTestingMeasurement(int Device, int Channel, Tac49xTrunkTestingMeasurementAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
	int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xTrunkTestingMeasurementCommandPayload);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);
    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__TRUNK_TESTING_MEASUREMENT;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;
	
    Packet.u.ProprietaryPayload.u.Command.u.TrunkTestingMeasurement.Operation               = pAttr->Operation       ;
    Packet.u.ProprietaryPayload.u.Command.u.TrunkTestingMeasurement.Direction               = pAttr->Direction       ;
    Packet.u.ProprietaryPayload.u.Command.u.TrunkTestingMeasurement.TriggerToneIbsSystemCode= pAttr->TriggerToneIbsSystemCode;
    Packet.u.ProprietaryPayload.u.Command.u.TrunkTestingMeasurement.TriggerToneDigit        = pAttr->TriggerToneDigit        ;
    Packet.u.ProprietaryPayload.u.Command.u.TrunkTestingMeasurement.MeasurementUnits		= pAttr->MeasurementUnits;
    Packet.u.ProprietaryPayload.u.Command.u.TrunkTestingMeasurement.ReferenceEnergy         = pAttr->ReferenceEnergy ;
	Packet.u.ProprietaryPayload.u.Command.u.TrunkTestingMeasurement.CMessageFilter          = pAttr->CMessageFilter  ;
    Packet.u.ProprietaryPayload.u.Command.u.TrunkTestingMeasurement.CNotchFilter            = pAttr->CNotchFilter    ;  
    Packet.u.ProprietaryPayload.u.Command.u.TrunkTestingMeasurement.TriggerType             = pAttr->TriggerType     ;  
    SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.TrunkTestingMeasurement.MeasurementDuration, pAttr->MeasurementDuration); 
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/****************************************************************/
int Ac49xStartBerTest(int Device, int Channel, Tac49xBerTestAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
	int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xBerTestCommandPayload);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);
    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__BER_TEST;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;
	
    Packet.u.ProprietaryPayload.u.Command.u.BerTest.Rate                            = pAttr->Rate                         ;
    Packet.u.ProprietaryPayload.u.Command.u.BerTest.Action                          = pAttr->Action                       ;
    Packet.u.ProprietaryPayload.u.Command.u.BerTest.InjectedPattern                 = pAttr->InjectedPattern              ;
    Packet.u.ProprietaryPayload.u.Command.u.BerTest.Direction                       = pAttr->Direction                    ;
    Packet.u.ProprietaryPayload.u.Command.u.BerTest.UserDefinedPatternSize          = pAttr->UserDefinedPatternSize       ;
    Packet.u.ProprietaryPayload.u.Command.u.BerTest.ErrorBitsInsertionInterval      = pAttr->ErrorBitsInsertionInterval   ;
	Packet.u.ProprietaryPayload.u.Command.u.BerTest.ErrorBitsInsertionTimeMode      = pAttr->ErrorBitsInsertionTimeMode   ;
    Packet.u.ProprietaryPayload.u.Command.u.BerTest.ErrorBitsInsertionPatternMode   = pAttr->ErrorBitsInsertionPatternMode;  
    Packet.u.ProprietaryPayload.u.Command.u.BerTest.UserDefinedPattern_MswLsb       =(pAttr->UserDefinedPattern>>16)&0xFF;
    Packet.u.ProprietaryPayload.u.Command.u.BerTest.UserDefinedPattern_LswMsb       =(pAttr->UserDefinedPattern>>8 )&0xFF;
    Packet.u.ProprietaryPayload.u.Command.u.BerTest.UserDefinedPattern_LswLsb       = pAttr->UserDefinedPattern     &0xFF;
    SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.BerTest.ErrorBitsInsertionNumber, pAttr->ErrorBitsInsertionNumber); 
    SplitFieldsFromLong( Packet.u.ProprietaryPayload.u.Command.u.BerTest.TestDuration,             pAttr->TestDuration); 

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/****************************************************************/
int Ac49xUtopiaStatusRequest(int Device, Tac49xUtopiaStatusRequestAttr *pAttr)
{
	Tac49xTxPacketParams TxPacketParams;
	int					 PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xUtopiaStatusRequestPayload);
	int					 ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);
    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__UTOPIA_STATUS_REQUEST;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;
	
    Packet.u.ProprietaryPayload.u.Command.u.UtopiaStatusRequest.StatusRequest   = pAttr->StatusRequest;
    Packet.u.ProprietaryPayload.u.Command.u.UtopiaStatusRequest.ClearStatistics = pAttr->ClearStatistics;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, 0, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= 0;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/****************************************************************/
int Ac49xSs7Mtp1Command(int Device,	int Channel, Tac49xSs7Mtp1CommandAttr *pAttr, Tac49xPacket *pPacket, Tac49xTransferMedium TransferMedium)
{
	Tac49xTxPacketParams	TxPacketParams;
    U16						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + 8 + pAttr->DataSize;
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);

	if(pPacket==NULL)
		{
		pPacket=&Packet;
		memset((void*)pPacket, 0, PacketSize);
		memcpy((void*)(pPacket->u.ProprietaryPayload.u.Command.u.Ss7Mtp1.Data), (void*)(pAttr->Data), pAttr->DataSize);
		pPacket->u.ProprietaryPayload.u.Command.u.Ss7Mtp1.MessageType = pAttr->MessageType;
		SplitFieldsFromShort(pPacket->u.ProprietaryPayload.u.Command.u.Ss7Mtp1.DataSize ,pAttr->DataSize);
		}
    pPacket->u.ProprietaryPayload.ProprietaryHeader.Opcode		= COMMAND_PACKET_OP_CODE__SS7_MTP1;
    pPacket->u.ProprietaryPayload.ProprietaryHeader.PacketCategory	= PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;
	
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)pPacket, PacketSize, TransferMedium);
#endif
	
#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)pPacket;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TransferMedium;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/****************************************************************/
int Ac49xTandemFreeOperationCommand(int Device, int Channel, Tac49xTandemFreeOperationAttr *pAttr)  
{																
	int						i;							
	Tac49xTxPacketParams	TxPacketParams;
    int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xTandemFreeOperationPayload);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);
    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode			= COMMAND_PACKET_OP_CODE__TANDEM_FREE_OPERATION;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory	= PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;
	
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.System													= pAttr->System												;
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.Function												= pAttr->Function.u.Command									;
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.ActiveAmrCodecRateOptimization							= pAttr->ActiveAmrCodecRateOptimization						;
	
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.CodecList.CodecListEnable								= pAttr->CodecList.CodecListEnable									;
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.CodecList.GsmFr											= pAttr->CodecList.GsmFr									;
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.CodecList.GsmHr 										= pAttr->CodecList.GsmHr 									;
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.CodecList.GsmEfr 										= pAttr->CodecList.GsmEfr 									;
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.CodecList.FrAmr											= pAttr->CodecList.FrAmr									;
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.CodecList.HrAmr											= pAttr->CodecList.HrAmr									;
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.CodecList.UmtsAmr										= pAttr->CodecList.UmtsAmr									;
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.CodecList.UmtsAmr2										= pAttr->CodecList.UmtsAmr2									;
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.CodecList.TdmaEfr										= pAttr->CodecList.TdmaEfr									;
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.CodecList.PdcEfr										= pAttr->CodecList.PdcEfr									;
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.CodecList.FrAmrWb										= pAttr->CodecList.FrAmrWb									;
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.CodecList.UmtsAmrWb										= pAttr->CodecList.UmtsAmrWb								;
	
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.SupportedCodec.Type										= pAttr->SupportedCodec.Type								; 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.SupportedCodec.MaximumNumberOfAmrRates					= pAttr->SupportedCodec.MaximumNumberOfAmrRates				;
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.SupportedCodec.SupportedAmrRates.u.Wide._10k2			= pAttr->SupportedCodec.SupportedAmrRates.u.Wide._10k2		; 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.SupportedCodec.SupportedAmrRates.u.Wide._12k2			= pAttr->SupportedCodec.SupportedAmrRates.u.Wide._12k2		; 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.SupportedCodec.SupportedAmrRates.u.Wide._4k75			= pAttr->SupportedCodec.SupportedAmrRates.u.Wide._4k75		; 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.SupportedCodec.SupportedAmrRates.u.Wide._5k15			= pAttr->SupportedCodec.SupportedAmrRates.u.Wide._5k15		; 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.SupportedCodec.SupportedAmrRates.u.Wide._5k9			= pAttr->SupportedCodec.SupportedAmrRates.u.Wide._5k9		; 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.SupportedCodec.SupportedAmrRates.u.Wide._6k7			= pAttr->SupportedCodec.SupportedAmrRates.u.Wide._6k7		; 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.SupportedCodec.SupportedAmrRates.u.Wide._7k4			= pAttr->SupportedCodec.SupportedAmrRates.u.Wide._7k4		; 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.SupportedCodec.SupportedAmrRates.u.Wide._7k95			= pAttr->SupportedCodec.SupportedAmrRates.u.Wide._7k95		; 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.SupportedCodec.SupportedAmrRates.u.Wide.Wide8			= pAttr->SupportedCodec.SupportedAmrRates.u.Wide.Wide8		; 
																														 																 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.ActiveAmrCodecRates.u.Wide._10k2						= pAttr->ActiveAmrCodecRates.u.Wide._10k2					; 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.ActiveAmrCodecRates.u.Wide._12k2						= pAttr->ActiveAmrCodecRates.u.Wide._12k2					; 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.ActiveAmrCodecRates.u.Wide._4k75						= pAttr->ActiveAmrCodecRates.u.Wide._4k75					; 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.ActiveAmrCodecRates.u.Wide._5k15						= pAttr->ActiveAmrCodecRates.u.Wide._5k15					; 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.ActiveAmrCodecRates.u.Wide._5k9 						= pAttr->ActiveAmrCodecRates.u.Wide._5k9 					; 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.ActiveAmrCodecRates.u.Wide._6k7 						= pAttr->ActiveAmrCodecRates.u.Wide._6k7 					; 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.ActiveAmrCodecRates.u.Wide._7k4 						= pAttr->ActiveAmrCodecRates.u.Wide._7k4 					; 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.ActiveAmrCodecRates.u.Wide._7k95						= pAttr->ActiveAmrCodecRates.u.Wide._7k95					; 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.ActiveAmrCodecRates.u.Wide.Wide8						= pAttr->ActiveAmrCodecRates.u.Wide.Wide8					; 
																														  															 
	Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.NumberOfOptionalAmrCodecs								= pAttr->NumberOfOptionalAmrCodecs							;
																														 
	for(i=0; i<MAX_NUMBER_OF_TANDEM_FREE_OPERATION_OPTIONAL_AMR_CODECS; i++)																										
		{
		Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.OptionalAmrCodec[i].Type							= pAttr->OptionalAmrCodec[i].Type							;
		Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.OptionalAmrCodec[i].MaximumNumberOfAmrRates			= pAttr->OptionalAmrCodec[i].MaximumNumberOfAmrRates		;
		Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.OptionalAmrCodec[i].SupportedAmrRates.u.Wide._10k2	= pAttr->OptionalAmrCodec[i].SupportedAmrRates.u.Wide._10k2	;
		Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.OptionalAmrCodec[i].SupportedAmrRates.u.Wide._12k2	= pAttr->OptionalAmrCodec[i].SupportedAmrRates.u.Wide._12k2	;
		Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.OptionalAmrCodec[i].SupportedAmrRates.u.Wide._4k75	= pAttr->OptionalAmrCodec[i].SupportedAmrRates.u.Wide._4k75	;
		Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.OptionalAmrCodec[i].SupportedAmrRates.u.Wide._5k15	= pAttr->OptionalAmrCodec[i].SupportedAmrRates.u.Wide._5k15	;
		Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.OptionalAmrCodec[i].SupportedAmrRates.u.Wide._5k9	= pAttr->OptionalAmrCodec[i].SupportedAmrRates.u.Wide._5k9	;
		Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.OptionalAmrCodec[i].SupportedAmrRates.u.Wide._6k7	= pAttr->OptionalAmrCodec[i].SupportedAmrRates.u.Wide._6k7	;
		Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.OptionalAmrCodec[i].SupportedAmrRates.u.Wide._7k4	= pAttr->OptionalAmrCodec[i].SupportedAmrRates.u.Wide._7k4	;
		Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.OptionalAmrCodec[i].SupportedAmrRates.u.Wide._7k95	= pAttr->OptionalAmrCodec[i].SupportedAmrRates.u.Wide._7k95	;

		Packet.u.ProprietaryPayload.u.Command.u.TandemFreeOperation.OptionalAmrCodec[i].SupportedAmrRates.u.Wide.Wide8	= pAttr->OptionalAmrCodec[i].SupportedAmrRates.u.Wide.Wide8	;
		}																				  

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/****************************************************************/
int Ac49xSendCas(int Device, int Channel, Tac49xCasAttr *pAttr)  
{
	Tac49xTxPacketParams	TxPacketParams;
    int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xCasGenerateCommandPayload);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode			= COMMAND_PACKET_OP_CODE__CAS_GENERATE;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory	= PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;
    Packet.u.ProprietaryPayload.u.Command.u.Cas.A					= pAttr->A         ;
    Packet.u.ProprietaryPayload.u.Command.u.Cas.B					= pAttr->B         ;
    Packet.u.ProprietaryPayload.u.Command.u.Cas.C					= pAttr->C         ;
    Packet.u.ProprietaryPayload.u.Command.u.Cas.D					= pAttr->D         ;
    Packet.u.ProprietaryPayload.u.Command.u.Cas.ReversalsA			= pAttr->ReversalsA;
    Packet.u.ProprietaryPayload.u.Command.u.Cas.ReversalsB			= pAttr->ReversalsB;
	Packet.u.ProprietaryPayload.u.Command.u.Cas.CasGenerationDirection = pAttr->CasGenerationDirection;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/****************************************************************/
int Ac49xSendCasString(int Device, int Channel, Tac49xCasStringCommandAttr *pAttr)  
{
	Tac49xTxPacketParams	TxPacketParams;
	int						i;
    int						PacketSize  = sizeof(Tac49xHpiPacketHeader)
										+ sizeof(Tac49xProprietaryPacketHeader)
										+ 6	/*For 1'st word (Containing the following fields: TotalDuration, NumberOfPatterns, Repeat)*/
										+ pAttr->NumberOfPatterns*sizeof(Packet.u.ProprietaryPayload.u.Command.u.CasString.Pattern[0]);

	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode			= COMMAND_PACKET_OP_CODE__CAS_STRING;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory	= PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;


    SplitFieldsFromLong(Packet.u.ProprietaryPayload.u.Command.u.CasString.TotalDuration, pAttr->TotalDuration);
    Packet.u.ProprietaryPayload.u.Command.u.CasString.NumberOfPatterns		= pAttr->NumberOfPatterns;
    Packet.u.ProprietaryPayload.u.Command.u.CasString.Repeat				= pAttr->Repeat;
    Packet.u.ProprietaryPayload.u.Command.u.CasString.Direction				= pAttr->Direction;

	for(i=0; i<pAttr->NumberOfPatterns; i++)
		{
		Packet.u.ProprietaryPayload.u.Command.u.CasString.Pattern[i].ReversalsA			= pAttr->Pattern[i].ReversalsA;
		Packet.u.ProprietaryPayload.u.Command.u.CasString.Pattern[i].ReversalsB			= pAttr->Pattern[i].ReversalsB;
		Packet.u.ProprietaryPayload.u.Command.u.CasString.Pattern[i].A					= pAttr->Pattern[i].A;
		Packet.u.ProprietaryPayload.u.Command.u.CasString.Pattern[i].B					= pAttr->Pattern[i].B;
		Packet.u.ProprietaryPayload.u.Command.u.CasString.Pattern[i].C					= pAttr->Pattern[i].C;
		Packet.u.ProprietaryPayload.u.Command.u.CasString.Pattern[i].D					= pAttr->Pattern[i].D;
		Packet.u.ProprietaryPayload.u.Command.u.CasString.Pattern[i].Duration_MswLsb	= (U8)(pAttr->Pattern[i].Duration>>16)&0xFF;
		Packet.u.ProprietaryPayload.u.Command.u.CasString.Pattern[i].Duration_LswMsb	= (U8)(pAttr->Pattern[i].Duration>>8 )&0xFF;
		Packet.u.ProprietaryPayload.u.Command.u.CasString.Pattern[i].Duration_LswLsb	= (U8)(pAttr->Pattern[i].Duration    )&0xFF;
		}

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}
                                                                             
/****************************************************************/
int Ac49xSendHdlcFramerMessage(int Device, int Channel, Tac49xHdlcFramerMessageCommandAttr *pAttr, Tac49xPacket * pPacket, Tac49xTransferMedium TransferMedium)
{
	Tac49xTxPacketParams	TxPacketParams;
    U16						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + 8 + pAttr->MessageSize;
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);

    if(PacketSize>sizeof(Tac49xPacket))
        return (-1);
	if(pPacket==NULL)
		{
		pPacket=&Packet;
		memset((void*)pPacket, 0, PacketSize);
		memcpy((void*)pPacket->u.ProprietaryPayload.u.Command.u.HdlcFramerMessage.Message ,(void*)pAttr->Message, pAttr->MessageSize);
		pPacket->u.ProprietaryPayload.u.Command.u.HdlcFramerMessage.MessageType = pAttr->MessageType;
		SplitFieldsFromShort(pPacket->u.ProprietaryPayload.u.Command.u.HdlcFramerMessage.MessageSize, pAttr->MessageSize);
		}
    pPacket->u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__HDLC_FRAMER;
    pPacket->u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)pPacket, PacketSize, TransferMedium);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)pPacket;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TransferMedium;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xSetRfc3267AmrRateControl(int Device, int Channel, Tac49xSetRfc3267AmrRateControlAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
    int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xSetRfc3267AmrRateControlPayload);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode								= COMMAND_PACKET_OP_CODE__SET_RFC3267_AMR_RATE_CONTROL;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory						= PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET           ;
    Packet.u.ProprietaryPayload.u.Command.u.SetRfc3267AmrRateControl.LocalRate			= pAttr->LocalRate       ;
    Packet.u.ProprietaryPayload.u.Command.u.SetRfc3267AmrRateControl.RemoteRate			= pAttr->RemoteRate      ;
    Packet.u.ProprietaryPayload.u.Command.u.SetRfc3267AmrRateControl.LocalRateEnable	= pAttr->LocalRateEnable ;
    Packet.u.ProprietaryPayload.u.Command.u.SetRfc3267AmrRateControl.RemoteRateEnable	= pAttr->RemoteRateEnable;
    Packet.u.ProprietaryPayload.u.Command.u.SetRfc3267AmrRateControl.RedundancyLevel    = pAttr->RedundancyLevel ;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */

	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Set3GppUserPlaneRateControl(Tac49xCommandPacketOpCode Opcode, int Device, int Channel, Tac49xSet3GppUserPlaneRateControlAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
    int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xSet3GppUserPlaneRateControlPayload);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode									= Opcode;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory							= PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;
    Packet.u.ProprietaryPayload.u.Command.u.Set3GppUserPlaneRateControl.LocalRate			= pAttr->LocalRate       ;
    Packet.u.ProprietaryPayload.u.Command.u.Set3GppUserPlaneRateControl.RemoteRate			= pAttr->RemoteRate      ;
    Packet.u.ProprietaryPayload.u.Command.u.Set3GppUserPlaneRateControl.LocalRateEnable		= pAttr->LocalRateEnable ;
    Packet.u.ProprietaryPayload.u.Command.u.Set3GppUserPlaneRateControl.RemoteRateEnable	= pAttr->RemoteRateEnable;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Get3GppStatistics(Tac49xCommandPacketOpCode Opcode, int Device, int Channel)
{
	Tac49xTxPacketParams	TxPacketParams;
    int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = Opcode;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xRegularChannelSet3GppUserPlaneRateControl(int Device, int Channel, Tac49xSet3GppUserPlaneRateControlAttr *pAttr)
{
	return Set3GppUserPlaneRateControl(COMMAND_PACKET_OP_CODE__SET_REGULAR_CHANNEL_3GPP_USER_PLANE_RATE_CONTROL, Device, Channel, pAttr);
}

/***************************************************************************************************************************/
int Ac49xRegularChannelGet3GppStatistics(int Device, int Channel)
{
	return Get3GppStatistics(COMMAND_PACKET_OP_CODE__GET_REGULAR_CHANNEL_3GPP_STATISTICS, Device, Channel);
}

/***************************************************************************************************************************/
int Ac49xMediatedChannelSet3GppUserPlaneRateControl(int Device, int Channel, Tac49xSet3GppUserPlaneRateControlAttr *pAttr)
{
	return Set3GppUserPlaneRateControl(COMMAND_PACKET_OP_CODE__SET_MEDIATED_CHANNEL_3GPP_USER_PLANE_RATE_CONTROL, Device, Channel, pAttr);
}

/***************************************************************************************************************************/
int Ac49xMediatedChannelGet3GppStatistics(int Device, int Channel)
{
	return Get3GppStatistics(COMMAND_PACKET_OP_CODE__GET_MEDIATED_CHANNEL_3GPP_STATISTICS, Device, Channel);
}

/***************************************************************************************************************************/
int SendFskCallerIdMessage(int Device, int Channel, Tac49xCallerIdAttr *pAttr, U8 *pMessage)
{
	Tac49xTxPacketParams		TxPacketParams	;
    int			                MessageLength   ;
    int			                PacketSize      ;
	Tac49xCallerIdPayloadTitle *pTitle; 
	int							ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
	pTitle = (Tac49xCallerIdPayloadTitle*)pMessage;
    MessageLength = pTitle->Length.Value + sizeof(Tac49xCallerIdPayloadTitle);
	if(  (  (pTitle->Type.Value == 0x40) /* #define CALLER_ID_SEGMENTATION_MESSAGE_TYPE__NTT_TYPE_1	0x40 */
	     || (pTitle->Type.Value == 0x41) /* #define CALLER_ID_SEGMENTATION_MESSAGE_TYPE__NTT_TYPE_2	0x41 */
	     ) 
	  && (pTitle->Length.Value == 0x10)
	  )	
		MessageLength++;
    PacketSize    = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + 8 + MessageLength;
    if(PacketSize>sizeof(Tac49xPacket))
		{
		Ac49xUserDef_CriticalSection_LeaveCommand(Device);
        return (-1);
		}
    memset((void*)&Packet, 0, PacketSize); 
    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode				= COMMAND_PACKET_OP_CODE__CALLER_ID	;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory		= PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET			;
    Packet.u.ProprietaryPayload.u.Command.u.CallerId.Standard           = pAttr->Standard					;
    Packet.u.ProprietaryPayload.u.Command.u.CallerId.ChecksumByteAbsent = pAttr->ChecksumByteAbsent		    ;
    Packet.u.ProprietaryPayload.u.Command.u.CallerId.EtsiOnhookMethod   = pAttr->EtsiOnhookMethod			;
    Packet.u.ProprietaryPayload.u.Command.u.CallerId.ServiceType        = pAttr->ServiceType				;
    memcpy((void*)&Packet.u.ProprietaryPayload.u.Command.u.CallerId.Message[0], pMessage, MessageLength);

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);

#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/

int	TestEtsiDtmfCallerIdMessage(int Length, U8 *ptMessage, Tac49xCallerIdAttr *pAttr, Tac49xExtendedIbsString_Format1_Attr *pIbs)
{
	int	i;
	int NumberOfDigits;
    int	DTMFChar;
    int	Error;
	int ParamterDPresent;
    
	ParamterDPresent = 0;
	Error = DTMF_CALLER_ID_STATUS__DECODER_VALID_END;
	if (Length == 0) 
		{
		pIbs->NumberOfDigits = 0;	
		return DTMF_CALLER_ID_STATUS__INVALID_MESSAGE_LENGTH;
		}
	for(i=0,NumberOfDigits=0; i<Length; i++)
		{
		/* read new character */				
		DTMFChar = *ptMessage++;
		if (DTMFChar == 0x002A)
			{
			/* convert to * */
			DTMFChar = DTMF_DIGIT__ASTERISK;
			Error = DTMF_CALLER_ID_STATUS__INVALID_PARAMETER_TYPE;
			}
		else if (DTMFChar == 0x0023)
			{
			/* convert to # */
			DTMFChar = DTMF_DIGIT__POUND;
			Error = DTMF_CALLER_ID_STATUS__INVALID_PARAMETER_TYPE;
			}	
		else
			{
			/* convert to digit */
			DTMFChar -= 0x0030;
			/* convert to ABCD*/
			if ((DTMFChar >= 0x0011) && (DTMFChar <= 0x0014)) 
				DTMFChar -= 5;
			/* invalid character */
			else if ((DTMFChar > 9) || (DTMFChar < 0))
				{
				Error = DTMF_CALLER_ID_STATUS__DTMF_ERRONEOUS_CHARACTER;
				continue;
				}
			}			
		/* look for start character */
		if (NumberOfDigits == 0)
			{
			if (DTMFChar == DTMF_DIGIT__D)
				ParamterDPresent = 1;
			else if ((DTMFChar != DTMF_DIGIT__A) && (DTMFChar != DTMF_DIGIT__B))
				{
				Error = DTMF_CALLER_ID_STATUS__MISSING_START_CHARACTER;
				}
			}
		else if ((DTMFChar == DTMF_DIGIT__A) && ParamterDPresent)
			{
			Error = DTMF_CALLER_ID_STATUS__INVALID_PARAMETER_TYPE;
			}
		/* write DTMF index to OS buffer */	
		if(NumberOfDigits+1 >= MAX_IBS_STRING_SIGNALS)
			{
			Error = DTMF_CALLER_ID_STATUS__DTMF_STRING_OVERFLOWS_OUTPUT_BUFFER;
			break;
			}
		pIbs->Signal[NumberOfDigits  ].Digit      = DTMFChar;
		pIbs->Signal[NumberOfDigits++].SystemCode = IBS_SYSTEM_CODE__DTMF;
		if (DTMFChar == DTMF_DIGIT__D)
			ParamterDPresent = 1;
		/* check for end character */
		if (DTMFChar == DTMF_DIGIT__C)
			{
			if (i != Length-1)
				{
				Error = DTMF_CALLER_ID_STATUS__INVALID_MESSAGE_LENGTH;
				break;
				}
			}
		else if (i == Length-1)
			Error = DTMF_CALLER_ID_STATUS__MISSING_END_CHARACTER;		
		}
	/* update number of digits */
	pIbs->NumberOfDigits = NumberOfDigits;
	/* check hook service */
	if (pAttr->ServiceType == CALLER_ID_SERVICE_TYPE__2_OFF_HOOK)
		Error = DTMF_CALLER_ID_STATUS__INVALID_SERVICE_TYPE;
	return(Error);
}

/***************************************************************************************************************************/
int	TestDenmarkDtmfCallerIdMessage(int Length, U8 *ptMessage, Tac49xCallerIdAttr *pAttr, Tac49xExtendedIbsString_Format1_Attr *pIbs)
{
	int	i;
	int NumberOfDigits;
    int	DTMFChar;
    int	Error;
	int CurrentParameter=0;
	int ParameterLen=0;
	
	Error = DTMF_CALLER_ID_STATUS__DECODER_VALID_END;
	if (Length == 0) 
		{
		pIbs->NumberOfDigits = 0;	
		return DTMF_CALLER_ID_STATUS__INVALID_MESSAGE_LENGTH;
		}
	for(i=0,NumberOfDigits=0; i<Length; i++)
		{
		/* read new character */				
		DTMFChar = *ptMessage++;
		if (DTMFChar == 0x002A)
			{
			/* convert to * */
			DTMFChar = DTMF_DIGIT__ASTERISK;
			}
		else if (DTMFChar == 0x0023)
			{
			/* convert to # */
			DTMFChar = DTMF_DIGIT__POUND;
			}	
		else
			{
			/* convert to digit */
			DTMFChar -= 0x0030;
			/* convert to ABCD*/
			if ((DTMFChar >= 0x0011) && (DTMFChar <= 0x0014)) 
				DTMFChar -= 5;
			/* invalid character */
			else if ((DTMFChar > 9) || (DTMFChar < 0))
				{
				Error = DTMF_CALLER_ID_STATUS__DTMF_ERRONEOUS_CHARACTER;
				continue;
				}
			}			
		/* look for start character */
		if (NumberOfDigits == 0)
			{
			if ((DTMFChar == DTMF_DIGIT__A) || (DTMFChar == DTMF_DIGIT__C) || (DTMFChar == DTMF_DIGIT__D))
				{
				CurrentParameter = DTMFChar;
				ParameterLen = 0;
				}
			else
				return DTMF_CALLER_ID_STATUS__MISSING_START_CHARACTER;
			}
			/* write DTMF index to OS buffer */	
		if(NumberOfDigits+1 >= MAX_IBS_STRING_SIGNALS)
			{
			Error = DTMF_CALLER_ID_STATUS__DTMF_STRING_OVERFLOWS_OUTPUT_BUFFER;
			break;
			}
		pIbs->Signal[NumberOfDigits  ].Digit      = DTMFChar;
		pIbs->Signal[NumberOfDigits++].SystemCode = IBS_SYSTEM_CODE__DTMF;
		ParameterLen++;
		if (CurrentParameter == DTMF_DIGIT__B)
			Error = DTMF_CALLER_ID_STATUS__INVALID_PARAMETER_TYPE;
		else if (DTMFChar >= DTMF_DIGIT__A)
			{
			/* check for invalid parameter length */
			if (NumberOfDigits > 1)
				{
				if ((CurrentParameter == DTMF_DIGIT__D) && (ParameterLen != 1+1))
					Error = DTMF_CALLER_ID_STATUS__INVALID_PARAMTER_LENGTH;
				if ((CurrentParameter == DTMF_DIGIT__C) && (ParameterLen != 2+1))
					Error = DTMF_CALLER_ID_STATUS__INVALID_PARAMTER_LENGTH;
				}
			CurrentParameter = DTMFChar;
			ParameterLen = 0;
			}
		/* check for mwi hook service*/
		if ((DTMFChar == DTMF_DIGIT__C) && (pAttr->ServiceType == CALLER_ID_SERVICE_TYPE__2_OFF_HOOK)) 
			Error = DTMF_CALLER_ID_STATUS__INVALID_SERVICE_TYPE;

		/* check for end character */
		if ((DTMFChar == DTMF_DIGIT__ASTERISK) || (DTMFChar == DTMF_DIGIT__POUND))
			{
			if (i != Length-1)
				{
				Error = DTMF_CALLER_ID_STATUS__INVALID_MESSAGE_LENGTH;
				break;
				}
			if ((CurrentParameter == DTMF_DIGIT__D) && (ParameterLen != 1+1))
				Error = DTMF_CALLER_ID_STATUS__INVALID_PARAMTER_LENGTH;
			if ((CurrentParameter == DTMF_DIGIT__C) && (ParameterLen != 2+1))
				Error = DTMF_CALLER_ID_STATUS__INVALID_PARAMTER_LENGTH;				
			}
		else if (i == Length-1)
			Error = DTMF_CALLER_ID_STATUS__MISSING_END_CHARACTER;
		}
	/* update number of digits */	
	pIbs->NumberOfDigits = NumberOfDigits;
	return(Error);
}

/***************************************************************************************************************************/
int	TestIndiaDtmfCallerIdMessage(int Length, U8 *ptMessage, Tac49xCallerIdAttr *pAttr, Tac49xExtendedIbsString_Format1_Attr *pIbs)
{
	int	i;
	int NumberOfDigits;
    int	DTMFChar;
    int	Error;
	
	Error = DTMF_CALLER_ID_STATUS__DECODER_VALID_END;
	if (Length == 0) 
		{
		pIbs->NumberOfDigits = 0;	
		return DTMF_CALLER_ID_STATUS__INVALID_MESSAGE_LENGTH;
		}
	for(i=0,NumberOfDigits=0; i<Length; i++)
		{
		/* read new character */				
		DTMFChar = *ptMessage++;
		if (DTMFChar == 0x002A)
			{
			/* convert to * */
			DTMFChar = DTMF_DIGIT__ASTERISK;
			Error = DTMF_CALLER_ID_STATUS__INVALID_PARAMETER_DATA;
			}
		else if (DTMFChar == 0x0023)
			{
			/* convert to # */
			DTMFChar = DTMF_DIGIT__POUND;
			Error = DTMF_CALLER_ID_STATUS__INVALID_PARAMETER_DATA;
			}	
		else
			{
			/* convert to digit */
			DTMFChar -= 0x0030;
			/* convert to ABCD*/
			if ((DTMFChar >= 0x0011) && (DTMFChar <= 0x0014))
				{
				Error = DTMF_CALLER_ID_STATUS__INVALID_PARAMETER_DATA;
				DTMFChar -= 5;
				}
			/* invalid character */
			else if ((DTMFChar > 9) || (DTMFChar < 0))
				{
				Error = DTMF_CALLER_ID_STATUS__DTMF_ERRONEOUS_CHARACTER;
				continue;
				}
			}			
		/* write DTMF index to OS buffer */	
		if(NumberOfDigits+1 >= MAX_IBS_STRING_SIGNALS)
			{
			Error = DTMF_CALLER_ID_STATUS__DTMF_STRING_OVERFLOWS_OUTPUT_BUFFER;
			break;
			}
		pIbs->Signal[NumberOfDigits  ].Digit      = DTMFChar;
		pIbs->Signal[NumberOfDigits++].SystemCode = IBS_SYSTEM_CODE__DTMF;
		if ((DTMFChar < 0) || (DTMFChar > 9))
			Error = DTMF_CALLER_ID_STATUS__INVALID_PARAMETER_DATA;
		}
	/* update number of digits */	
	pIbs->NumberOfDigits = NumberOfDigits;
	/* check hook service */
	if (pAttr->ServiceType == CALLER_ID_SERVICE_TYPE__2_OFF_HOOK)
		Error = DTMF_CALLER_ID_STATUS__INVALID_SERVICE_TYPE;
	return(Error);
}

/***************************************************************************************************************************/
int	TestBrazilDtmfCallerIdMessage(int Length, U8 *ptMessage, Tac49xCallerIdAttr *pAttr, Tac49xExtendedIbsString_Format1_Attr *pIbs)
{
	int	i;
	int NumberOfDigits;
    int	DTMFChar;
    int	Error;
	
	Error = DTMF_CALLER_ID_STATUS__DECODER_VALID_END;
	if (Length == 0) 
		{
		pIbs->NumberOfDigits = 0;	
		return DTMF_CALLER_ID_STATUS__INVALID_MESSAGE_LENGTH;
		}
	for(i=0,NumberOfDigits=0; i<Length; i++)
		{
		/* read new character */				
		DTMFChar = *ptMessage++;
		if (DTMFChar == 0x002A)
			{
			/* convert to * */
			DTMFChar = DTMF_DIGIT__ASTERISK;
			Error = DTMF_CALLER_ID_STATUS__INVALID_PARAMETER_DATA;
			}
		else if (DTMFChar == 0x0023)
			{
			/* convert to # */
			DTMFChar = DTMF_DIGIT__POUND;
			Error = DTMF_CALLER_ID_STATUS__INVALID_PARAMETER_DATA;
			}	
		else
			{
			/* convert to digit */
			DTMFChar -= 0x0030;
			/* convert to ABCD*/
			if ((DTMFChar >= 0x0011) && (DTMFChar <= 0x0014)) 
				DTMFChar -= 5;
			/* invalid character */
			else if ((DTMFChar > 9) || (DTMFChar < 0))
				{
				Error = DTMF_CALLER_ID_STATUS__DTMF_ERRONEOUS_CHARACTER;
				continue;
				}
			}			
		/* look for start character */
		if ((NumberOfDigits == 0) && (DTMFChar != DTMF_DIGIT__A))
			Error = DTMF_CALLER_ID_STATUS__MISSING_START_CHARACTER;
		/* check for end character */
		if (DTMFChar == DTMF_DIGIT__C)
			{
			if (i != Length-1)
				{
				Error = DTMF_CALLER_ID_STATUS__INVALID_MESSAGE_LENGTH;
				break;
				}
			}
		else if (i == Length-1)
			Error = DTMF_CALLER_ID_STATUS__MISSING_END_CHARACTER;
		/* Check for first parameter data */
		else if (NumberOfDigits==1)
			{
			if ((DTMFChar != 1) && (DTMFChar != DTMF_DIGIT__B))
				Error = DTMF_CALLER_ID_STATUS__INVALID_PARAMETER_DATA;
			}
		/* Check for first parameter data */
		else if ((DTMFChar > 9) && (NumberOfDigits>1))
			Error = DTMF_CALLER_ID_STATUS__INVALID_PARAMETER_DATA;
		/* write DTMF index to OS buffer */	
		if(NumberOfDigits+1 >= MAX_IBS_STRING_SIGNALS)
			{
			Error = DTMF_CALLER_ID_STATUS__DTMF_STRING_OVERFLOWS_OUTPUT_BUFFER;
			break;
			}
		pIbs->Signal[NumberOfDigits  ].Digit      = DTMFChar;
		pIbs->Signal[NumberOfDigits++].SystemCode = IBS_SYSTEM_CODE__DTMF;
		}
	/* update number of digits */
	pIbs->NumberOfDigits = NumberOfDigits;
	/* check hook service */
	if (pAttr->ServiceType == CALLER_ID_SERVICE_TYPE__2_OFF_HOOK)
		Error = DTMF_CALLER_ID_STATUS__INVALID_SERVICE_TYPE;
	return(Error);
}

/***************************************************************************************************************************/
int SendDtmfCallerIdMessage_Format1(int Device, int Channel, Tac49xCallerIdAttr *pAttr, U8 *pMessage)
{
    Tac49xExtendedIbsString_Format1_Attr IbsStringAttr;    
	U8 *ptMessage = pMessage;
    U8  MessageLength;
	int ReturnedVal;
	int DTMFError;
	memset(&IbsStringAttr, 0x00, sizeof(IbsStringAttr));
    
	Ac49xUserDef_CriticalSection_EnterCommand(Device);

    IbsStringAttr.Repeat                = 0;
	IbsStringAttr.GeneratorRedirection  = IBS_STRING_GENERATOR_REDIRECTION__INTO_DECODER_OUTPUT;	
    IbsStringAttr.TotalDuration         = 0; 
	IbsStringAttr.UserDefinedToneA      = 0;
	IbsStringAttr.UserDefinedToneB      = 0;
	IbsStringAttr.UserDefinedToneC      = 0;
    IbsStringAttr.PauseDuration         = 0;
    IbsStringAttr.NumberOfDigits        = 0;
    IbsStringAttr.OffDuration           = 70;
    IbsStringAttr.OnDuration            = 70;
    IbsStringAttr.AmFactor              = 0;
    IbsStringAttr.SignalLevel_minus_dbm = (int)FAX_AND_CALLER_ID_LEVEL__minus_9_DBM;
	
	if (pAttr->Standard == CALLER_ID_STANDARD__DTMF_CLIP_INDIAN)
		{
		IbsStringAttr.OffDuration         = 50;
        IbsStringAttr.OnDuration          = 50;            
		}
	else if (pAttr->Standard > CALLER_ID_STANDARD__DTMF_CLIP_NTT_DID)
		{
		Ac49xUserDef_CriticalSection_LeaveCommand(Device);
		return(DTMF_CALLER_ID_STATUS__DECODER_ERRONEOUS_STANDARD);	
		}
    
	/* skip message type */
	ptMessage++;
	/* message length */
	MessageLength = *ptMessage++;
	DTMFError = 0;

	switch (pAttr->Standard)
		{
		case CALLER_ID_STANDARD__DTMF_CLIP_ETSI:
			DTMFError = TestEtsiDtmfCallerIdMessage(MessageLength, ptMessage, pAttr, &IbsStringAttr);
			break;
		case CALLER_ID_STANDARD__DTMF_CLIP_DANISH:
			DTMFError = TestDenmarkDtmfCallerIdMessage(MessageLength, ptMessage, pAttr, &IbsStringAttr);
			break;
		case CALLER_ID_STANDARD__DTMF_CLIP_NTT_DID:
		case CALLER_ID_STANDARD__DTMF_CLIP_INDIAN:
			DTMFError = TestIndiaDtmfCallerIdMessage(MessageLength, ptMessage, pAttr, &IbsStringAttr);
			break;
		case CALLER_ID_STANDARD__DTMF_CLIP_BRAZILIAN:
			DTMFError = TestBrazilDtmfCallerIdMessage(MessageLength, ptMessage, pAttr, &IbsStringAttr);
			break;
    default:;
		}		
		
    ReturnedVal = Ac49xSendExtendedIbsString_Format1(Device, Channel, &IbsStringAttr);
   	if (ReturnedVal == 0) ReturnedVal = DTMFError;
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
    return ReturnedVal;
}


/***************************************************************************************************************************/
int Ac49xSendCallerIdMessage(int Device, int Channel, Tac49xCallerIdAttr *pAttr, U8 *pMessage)
{
	Tac49xCallerIdPayloadTitle *pTitle; 
    pTitle = (Tac49xCallerIdPayloadTitle*)pMessage;

    if(pTitle->Type.Value == 0) /* #define CALLER_ID_SEGMENTATION_MESSAGE_TYPE__DTMF 0x0*/
        return SendDtmfCallerIdMessage_Format1(Device, Channel, pAttr, pMessage);
    else
        return SendFskCallerIdMessage(Device, Channel, pAttr, pMessage);
}

/***************************************************************************************************************************/
int Ac49xSendCallerIdStop(int Device, int Channel)
{
	Tac49xTxPacketParams	TxPacketParams;
	int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__CALLER_ID_STOP  ;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET               ;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xPlaybackNetworkStart(int Device, int Channel, Tac49xCoder Coder, Tac49xEndianness Endianness) 
{
	Tac49xTxPacketParams	TxPacketParams;
    int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xPlaybackCommandPayload);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.PlaybackPayload.u.Command.Opcode		= PLAYBACK_HOST_OPCODE__NETWORK_START;
    Packet.u.PlaybackPayload.u.Command.Coder		= Coder;
	Packet.u.PlaybackPayload.u.Command.Endianness	= Endianness;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__NETWORK);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PLAYBACK_COMMAND;
	TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__NETWORK;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xPlaybackTdmStart(int Device, int Channel, Tac49xCoder Coder, Tac49xEndianness Endianness) 
{
	Tac49xTxPacketParams	TxPacketParams;
    int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xPlaybackCommandPayload);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.PlaybackPayload.u.Command.Opcode		= PLAYBACK_HOST_OPCODE__TDM_START;
    Packet.u.PlaybackPayload.u.Command.Coder		= Coder;
	Packet.u.PlaybackPayload.u.Command.Endianness	= Endianness;	

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__NETWORK);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
 	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PLAYBACK_COMMAND;
	TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__NETWORK;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}


/***************************************************************************************************************************/
int Ac49xPlaybackPlaySilence(int Device, int Channel, int Silence) 
{
	Tac49xTxPacketParams	TxPacketParams;
    int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xPlaybackCommandPayload);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.PlaybackPayload.u.Command.Opcode = PLAYBACK_HOST_OPCODE__PLAY_SILENCE;
    Packet.u.PlaybackPayload.u.Command.u.Host.SilenceDuration_msec_FirstByte  = (Silence>>16)&0xFF;
    Packet.u.PlaybackPayload.u.Command.u.Host.SilenceDuration_msec_SecondByte = (Silence>>8 )&0xFF;
    Packet.u.PlaybackPayload.u.Command.u.Host.SilenceDuration_msec_ThirdByte  = Silence&0xFF;
#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__NETWORK);
#endif
    
#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PLAYBACK_COMMAND;
	TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__NETWORK;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xPlaybackPlayVoice(int Device, int Channel, char *pVoiceBuffer, int BufferSize) 
{
	Tac49xTxPacketParams TxPacketParams;
    int					 PacketSize	= sizeof(Tac49xHpiPacketHeader) + BufferSize;
	int					 ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
	
	Packet.HpiHeader.Channel    = Channel;
	Packet.HpiHeader.Protocol   = PROTOCOL__PLAYBACK_VOICE;
	Packet.HpiHeader.Sync5	    = 0x5;

	memcpy((void*)(&(Packet.u.PlaybackPayload.u.Voice)), pVoiceBuffer, BufferSize);

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__NETWORK);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PLAYBACK_VOICE;
	TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__NETWORK;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xPlaybackEnd(int Device, int Channel) 
{
	Tac49xTxPacketParams	TxPacketParams;
    int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xPlaybackCommandPayload);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.PlaybackPayload.u.Command.Opcode = PLAYBACK_HOST_OPCODE__END;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__NETWORK);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PLAYBACK_COMMAND;
	TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__NETWORK;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xRecordCommand(int Device, int Channel, Tac49xRecordCommandAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
    int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xRecordCommandPayload);
 	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.RecordPayload.u.Command.Type  = pAttr->Type;
    Packet.u.RecordPayload.u.Command.Coder = pAttr->Coder;


#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__NETWORK);
#endif
    
#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&Packet.u.RecordPayload,PacketSize-sizeof(Tac49xHpiPacketHeader));
    Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__RECORD_COMMAND;
	TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__NETWORK;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xResetSerialPort(int Device)
{
	Tac49xTxPacketParams	TxPacketParams;
	int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader);
	int						ReturnedValue;

#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	)
	Device = Device - (Device%NUMBER_OF_DEVICES_PER_DSP);
#endif
	
	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__RESET_SERIAL_PORT;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, 0, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device, Channel, (char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= 0;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int GetChannelMediaProtectionInfo(Tac49xCommandPacketOpCode Opcode, int Device, int Channel, Tac49xGetMediaProtectiontInfoAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
	int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xGetMediaProtectiontInfoPayload);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = Opcode;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

    Packet.u.ProprietaryPayload.u.Command.u.GetMediaProtectiontInfo.InfoType = pAttr->InfoType;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xGetRegularChannelMediaProtectionInfo(int Device, int Channel, Tac49xGetMediaProtectiontInfoAttr *pAttr)
{
	return GetChannelMediaProtectionInfo(COMMAND_PACKET_OP_CODE__GET_REGULAR_CHANNEL_MEDIA_PROTECTION_INFO_COMMAND, Device, Channel, pAttr);
}

/***************************************************************************************************************************/
int Ac49xGetMediatedChannelMediaProtectionInfo(int Device, int Channel, Tac49xGetMediaProtectiontInfoAttr *pAttr)
{
	return GetChannelMediaProtectionInfo(COMMAND_PACKET_OP_CODE__GET_MEDIATED_CHANNEL_MEDIA_PROTECTION_INFO_COMMAND, Device, Channel, pAttr);
}

/***************************************************************************************************************************/
int Ac49xSetRfc3558RateMode(int Device, int Channel, Tac49xSetRfc3558RateModeAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
	int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xSetRfc3558RateModePayload);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__SET_RFC3558_RATE_MODE;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

	Packet.u.ProprietaryPayload.u.Command.u.SetRfc3558RateMode.LocalRateMode							   = pAttr->LocalRateMode;
	Packet.u.ProprietaryPayload.u.Command.u.SetRfc3558RateMode.RemoteRateMode							   = pAttr->RemoteRateMode;
	Packet.u.ProprietaryPayload.u.Command.u.SetRfc3558RateMode.LocalRateModeControlledByRemoteSideEnable = pAttr->LocalRateModeControlledByRemoteSideEnable;
	Packet.u.ProprietaryPayload.u.Command.u.SetRfc3558RateMode.LocalMaxRate								   = pAttr->LocalMaxRate;
	Packet.u.ProprietaryPayload.u.Command.u.SetRfc3558RateMode.LocalMinRate								   = pAttr->LocalMinRate;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xTimeSlotManipulationCommand(int Device, Tac49xTimeSlotManipulationCommandAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
	int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xTimeSlotManipulationCommandPayload);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__TIME_SLOT_MANIPULATION;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

	Packet.u.ProprietaryPayload.u.Command.u.TimeSlotManipulation.CommandType	= pAttr->CommandType;
	Packet.u.ProprietaryPayload.u.Command.u.TimeSlotManipulation.RxTdmPortType	= pAttr->RxTdmPortType;
	Packet.u.ProprietaryPayload.u.Command.u.TimeSlotManipulation.RxChannelId	= pAttr->RxChannelId;
	Packet.u.ProprietaryPayload.u.Command.u.TimeSlotManipulation.TxTdmPortType	= pAttr->TxTdmPortType;
	Packet.u.ProprietaryPayload.u.Command.u.TimeSlotManipulation.TxChannelId	= pAttr->TxChannelId;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, 0, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= 0;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xModuleStatusRequest(int Device, int Channel, Tac49xModuleStatusRequestAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
	int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xModuleStatusRequestPayload);
	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__MODULE_STATUS_REQUEST;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

	Packet.u.ProprietaryPayload.u.Command.u.ModuleStatusRequest.ModuleType = pAttr->ModuleType;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int SendRtcpAppCommand(Tac49xCommandPacketOpCode Opcode, int Device, int Channel, Tac49xRtcpAppCommandAttr *pAttr)
{
	int						i;
	Tac49xTxPacketParams	TxPacketParams;
	int						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + 12 + pAttr->DataLength_In4Bytes*4;/*sizeof(Tac49xRtcpAppCommandPayload);*/
 	int						ReturnedValue;

	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = Opcode;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

	Packet.u.ProprietaryPayload.u.Command.u.RtcpApp.PacketFormat = pAttr->PacketFormat;
	Packet.u.ProprietaryPayload.u.Command.u.RtcpApp.SubType		 = pAttr->SubType;
	SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.RtcpApp.DataLength_In4Bytes, pAttr->DataLength_In4Bytes);

	for(i=0; i<RTCP_APP_NAME_LENGTH; i++)
		Packet.u.ProprietaryPayload.u.Command.u.RtcpApp.Name[i]    = pAttr->Name[i];
	for(i=0; i<(pAttr->DataLength_In4Bytes*4) && i<RTCP_APP_DATA_LENGTH; i++)
		Packet.u.ProprietaryPayload.u.Command.u.RtcpApp.Data[i]    = pAttr->Data[i];

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xSendRegularRtcpAppCommand(int Device, int Channel, Tac49xRtcpAppCommandAttr *pAttr)
{
	 return SendRtcpAppCommand(COMMAND_PACKET_OP_CODE__SEND_REGULAR_RTCP_APP, Device, Channel, pAttr);
}

/***************************************************************************************************************************/
int Ac49xSendMediatedRtcpAppCommand(int Device, int Channel, Tac49xRtcpAppCommandAttr *pAttr)
{
	return SendRtcpAppCommand(COMMAND_PACKET_OP_CODE__SEND_MEDIATED_RTCP_APP, Device, Channel, pAttr);
}

/***************************************************************************************************************************/
int Ac49xDuaFramerCommand(int Device, int Channel, Tac49xDuaFramerCommandAttr *pAttr, Tac49xPacket * pPacket, Tac49xTransferMedium TransferMedium)
{
	Tac49xTxPacketParams	TxPacketParams;
    U16						PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + 8 + pAttr->MessageSize;
 	int						ReturnedValue;

   	Ac49xUserDef_CriticalSection_EnterCommand(Device);

    if(pAttr->MessageSize>MAX_DUA_MESSAGE_LENGTH)
        return -1;

	if(pPacket==NULL)
        {
		pPacket=&Packet;
        memset((void*)&Packet, 0, PacketSize);
        Packet.u.ProprietaryPayload.u.Command.u.DuaFramer.MessageType = pAttr->MessageType;
        SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.DuaFramer.MessageSize, pAttr->MessageSize);
        memcpy((void*)(Packet.u.ProprietaryPayload.u.Command.u.DuaFramer.Message) ,(void*)(pAttr->Message), pAttr->MessageSize);
        }
     pPacket->u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__DUA_FRAMER;
     pPacket->u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TransferMedium);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)pPacket;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
	TxPacketParams.TransferMedium		= TransferMedium;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xEventsControlCommand(int Device, Tac49xEventsControlCommandAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
 	int						ReturnedValue;
	int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xEventsControlCommandPayload);

   	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__EVENTS_CONTROL;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.E24				 = pAttr->E24				;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.E25				 = pAttr->E25				;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.E26				 = pAttr->E26				;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.E27				 = pAttr->E27				;

	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.E28				 = pAttr->E28				;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.E29				 = pAttr->E29				;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.E30				 = pAttr->E30				;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.E31				 = pAttr->E31				;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.BerReport			 = pAttr->BerReport			;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.UtopiaStatus		 = pAttr->UtopiaStatus		;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.E18				 = pAttr->E18				;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.ModuleStatus		 = pAttr->ModuleStatus		;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.E20				 = pAttr->E20				;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.E21				 = pAttr->E21				;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.E22				 = pAttr->E22				;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.E23				 = pAttr->E23				;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.PacketizerStatus	 = pAttr->PacketizerStatus	;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.E9				 = pAttr->E9				;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.DeviceStatus		 = pAttr->DeviceStatus		;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.E11				 = pAttr->E11				;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.FaxAndDataStatus	 = pAttr->FaxAndDataStatus	;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.E13				 = pAttr->E13				;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.UtopiaTestError	 = pAttr->UtopiaTestError	;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.TrunkTestingsEvent = pAttr->TrunkTestingsEvent;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.SystemError		 = pAttr->SystemError		;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.GeneralEvents		 = pAttr->GeneralEvents		;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.IbsEvent			 = pAttr->IbsEvent			;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.CasEvent			 = pAttr->CasEvent			;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.HdlcFramerStatus	 = pAttr->HdlcFramerStatus	;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.FaxRelayStatus	 = pAttr->FaxRelayStatus	;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.JitterBufferStatus = pAttr->JitterBufferStatus;
	Packet.u.ProprietaryPayload.u.Command.u.EventsControl.CallerId			 = pAttr->CallerId			;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, 0, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= 0;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xNtpTimeStampCommand(int Device, int Channel, Tac49xNtpTimeStampCommandAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
 	int						ReturnedValue;
	int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xNtpTimeStampCommandPayload);

   	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__NTP_TIME_STAMP;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;
	SplitFieldsFromLong(Packet.u.ProprietaryPayload.u.Command.u.NtpTimeStamp.Time_x1Sec, pAttr->Time_x1Sec);
	SplitFieldsFromLong(Packet.u.ProprietaryPayload.u.Command.u.NtpTimeStamp.Time_x1SecFraction, pAttr->Time_x1SecFraction);

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, 0, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= 0;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xVideoAudioSyncCommand(int Device, int Channel, Tac49xPacket *pPacket)
{
	Tac49xTxPacketParams	TxPacketParams;
 	int						ReturnedValue;
	int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xVideoAudioSyncMsgPayload);

   	Ac49xUserDef_CriticalSection_EnterCommand(Device);
	memset((void*)&Packet, 0, PacketSize);

    pPacket->u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__VIDEO_AUDIO_SYNC;
    pPacket->u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, 0, (char*)pPacket, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)pPacket;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xSetG729EvRateControl(int Device, int Channel, Tac49xG729EvRateControlCommandAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
 	int						ReturnedValue;
	int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xG729EvRateControlCommandPayload);

   	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__G729EV_RATE_CONTROL;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

	Packet.u.ProprietaryPayload.u.Command.u.G729EvRateControl.LocalBitRate			        = pAttr->LocalBitRate;
	Packet.u.ProprietaryPayload.u.Command.u.G729EvRateControl.LocalBitRateFieldEnable       = pAttr->LocalBitRateFieldEnable;
	Packet.u.ProprietaryPayload.u.Command.u.G729EvRateControl.MaxRemoteBitRate		        = pAttr->MaxRemoteBitRate;
	Packet.u.ProprietaryPayload.u.Command.u.G729EvRateControl.MaxRemoteBitRateFieldEnable	= pAttr->MaxRemoteBitRateFieldEnable;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xGet3GppIubStatus(int Device, int Channel)
{
	Tac49xTxPacketParams	TxPacketParams;
 	int						ReturnedValue;
	int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader);

   	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__GET_3GPP_IUB_STATUS;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xAmdCommand(int Device, int Channel, Tac49xAmdCommandAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
 	int						ReturnedValue;
	int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader) + sizeof(Tac49xAmdCommandPayload);

   	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__AMD;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

	Packet.u.ProprietaryPayload.u.Command.u.Amd.Operation				= pAttr->Operation;
	Packet.u.ProprietaryPayload.u.Command.u.Amd.AnswerDetectSensitivity	= pAttr->AnswerDetectSensitivity;

	SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.Amd.DecisionParameter1, pAttr->DecisionParameter1);
	SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.Amd.DecisionParameter2, pAttr->DecisionParameter2);
	SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.Amd.DecisionParameter8, pAttr->DecisionParameter8);
	SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.Amd.DecisionParameter3, pAttr->DecisionParameter3);
	SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.Amd.MaxGreetingTime,	 pAttr->MaxGreetingTime);
	SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.Amd.SilentCallTimeout,  pAttr->SilentCallTimeout);
	SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.Amd.PreWorkDelay,		 pAttr->PreWorkDelay);
	SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.Amd.DecisionTimeout,	 pAttr->DecisionTimeout);	

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int GetRtcpXrMarkovModelStatisticsCommand(Tac49xCommandPacketOpCode OpCode, int Device, int Channel)
{
	Tac49xTxPacketParams	TxPacketParams;
 	int						ReturnedValue;
	int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader);

   	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = OpCode;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xGetRegularChannelRtcpXrMarkovModelStatisticsCommand(int Device, int Channel)
{
	return GetRtcpXrMarkovModelStatisticsCommand(COMMAND_PACKET_OP_CODE__GET_REGULAR_CHANNEL_RTCP_XR_MARKOV_MODEL_STATISTICS, Device, Channel);
}

/***************************************************************************************************************************/
int Ac49xGetMediatedChannelRtcpXrMarkovModelStatisticsCommand(int Device, int Channel)
{
	return GetRtcpXrMarkovModelStatisticsCommand(COMMAND_PACKET_OP_CODE__GET_MEDIATED_CHANNEL_RTCP_XR_MARKOV_MODEL_STATISTICS, Device, Channel);
}

/***************************************************************************************************************************/
int GetRtcpXrVoiceQualityStatisticsCommand(Tac49xCommandPacketOpCode OpCode, int Device, int Channel)
{
	Tac49xTxPacketParams	TxPacketParams;
 	int						ReturnedValue;
	int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader);

   	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = OpCode;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xGetRegularChannelRtcpXrVoiceQualityStatisticsCommand(int Device, int Channel)
{
	return GetRtcpXrVoiceQualityStatisticsCommand(COMMAND_PACKET_OP_CODE__GET_REGULAR_CHANNEL_RTCP_XR_VOICE_QUALITY_STATISTICS, Device, Channel);
}

/***************************************************************************************************************************/
int Ac49xGetMediatedChannelRtcpXrVoiceQualityStatisticsCommand(int Device, int Channel)
{
	return GetRtcpXrVoiceQualityStatisticsCommand(COMMAND_PACKET_OP_CODE__GET_MEDIATED_CHANNEL_RTCP_XR_VOICE_QUALITY_STATISTICS, Device, Channel);
}

/***************************************************************************************************************************/
int Ac49xGetLastCasDetectionCommand(int Device, int Channel)
{
	Tac49xTxPacketParams	TxPacketParams;
 	int						ReturnedValue;
	int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader);

   	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__GET_LAST_CAS_DETECTION;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xGet4WayConferenceStatusCommand(int Device)
{
	Tac49xTxPacketParams	TxPacketParams;
 	int						ReturnedValue;
	int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader);

   	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__GET_4_WAY_CONFERENCE_STATUS;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= 0;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}


/***************************************************************************************************************************/
int Ac49xGet4WayConferenceDetailsCommand(int Device, Tac49xGet4WayConferenceDetailsCommandAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
 	int						ReturnedValue;
	int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader)+ sizeof(Tac49xGet4WayConferenceDetailsCommandPayload);

   	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__GET_4_WAY_CONFERENCE_DETAILS;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

	Packet.u.ProprietaryPayload.u.Command.u.Get4WayConferenceDetails.ConferenceId = pAttr->ConferenceId;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= 0;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xProtectionTestCommand(int Device, int Channel, Tac49xProtectionTestCommandAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
 	int						ReturnedValue;
	int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader)+ sizeof(Tac49xProtectionTestCommandPayload) - 
					 (MAX_PAYLOAD_SIZE - sizeof(Tac49xHpiPacketHeader) - sizeof(Tac49xProprietaryPacketHeader) - 4) + pAttr->KeySize + pAttr->DataSize;

   	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__PROTECTION_TEST;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

	Packet.u.ProprietaryPayload.u.Command.u.ProtectionTest.Action  = pAttr->Action;
	Packet.u.ProprietaryPayload.u.Command.u.ProtectionTest.KeySize = pAttr->KeySize;
	SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.ProtectionTest.DataSize, pAttr->DataSize);

	if (pAttr->KeySize + pAttr->DataSize <= MAX_PAYLOAD_SIZE - sizeof(Tac49xHpiPacketHeader) - sizeof(Tac49xProprietaryPacketHeader) - 4)
		memcpy((char*)Packet.u.ProprietaryPayload.u.Command.u.ProtectionTest.Payload, (char*)pAttr->Payload, pAttr->KeySize + pAttr->DataSize);

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xGetRandomNumberCommand(int Device)
{
	Tac49xTxPacketParams	TxPacketParams;
 	int						ReturnedValue;
	int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader);

   	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__GET_RANDOM_NUMBER;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= 0;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xSwitchToModemCommand(int Device, int Channel, Tac49xSwitchToModemCommandAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
 	int						ReturnedValue;
	int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader)+ sizeof(Tac49xSwitchToModemCommandPayload);

   	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__SWITCH_TO_MODEM;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

	Packet.u.ProprietaryPayload.u.Command.u.SwitchToModem.InitiatingFlag	    = pAttr->InitiatingFlag;
	Packet.u.ProprietaryPayload.u.Command.u.SwitchToModem.ReasonIdentifierCode  = pAttr->ReasonIdentifierCode;
	SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.SwitchToModem.RtpTxSequenceNumber, pAttr->RtpTxSequenceNumber);
	SplitFieldsFromLong(Packet.u.ProprietaryPayload.u.Command.u.SwitchToModem.RtpTxTimeStamp,		pAttr->RtpTxTimeStamp);
	SplitFieldsFromLong(Packet.u.ProprietaryPayload.u.Command.u.SwitchToModem.RtpTxSsrc,			pAttr->RtpTxSsrc);

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/
int Ac49xModemRelayEndedCommand(int Device, int Channel, Tac49xModemRelayEndedCommandAttr *pAttr)
{
	Tac49xTxPacketParams	TxPacketParams;
 	int						ReturnedValue;
	int	PacketSize = sizeof(Tac49xHpiPacketHeader) + sizeof(Tac49xProprietaryPacketHeader)+ sizeof(Tac49xModemRelayEndedCommandPayload);

   	Ac49xUserDef_CriticalSection_EnterCommand(Device);
    memset((void*)&Packet, 0, PacketSize);

    Packet.u.ProprietaryPayload.ProprietaryHeader.Opcode           = COMMAND_PACKET_OP_CODE__MODEM_RELAY_ENDED;
    Packet.u.ProprietaryPayload.ProprietaryHeader.PacketCategory   = PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET;

	Packet.u.ProprietaryPayload.u.Command.u.ModemRelayEnded.ReasonIdentifierCode    = pAttr->ReasonIdentifierCode;
	SplitFieldsFromShort(Packet.u.ProprietaryPayload.u.Command.u.ModemRelayEnded.RtpTxSequenceNumber, pAttr->RtpTxSequenceNumber);
	SplitFieldsFromLong(Packet.u.ProprietaryPayload.u.Command.u.ModemRelayEnded.RtpTxTimeStamp,		  pAttr->RtpTxTimeStamp);
	SplitFieldsFromLong(Packet.u.ProprietaryPayload.u.Command.u.ModemRelayEnded.RtpTxSsrc,			  pAttr->RtpTxSsrc);

#if CRASH_TEST_ENABLE
	Ac49xUserDef_RaffleAndTrashPacketBits(Device, Channel, (char*)&Packet, PacketSize, TRANSFER_MEDIUM__HOST);
#endif

#ifdef TRANSMIT_VIA_UTOPIA
	/* when sending via UTOPIA there is no need for the HPI header */
	ReturnedValue =  Ac49xUserDef_TransmitViaUtopia(Device,Channel,(char*)&pPacket->u.ProprietaryPayload.ProprietaryHeader,PacketSize-sizeof(Tac49xHpiPacketHeader));
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#else
	TxPacketParams.Device				= Device;
	TxPacketParams.Channel				= Channel;
	TxPacketParams.pOutPacket			= (char*)&Packet;
	TxPacketParams.PacketSize			= PacketSize;
	TxPacketParams.Protocol				= PROTOCOL__PROPRIETARY;
    TxPacketParams.TransferMedium		= TRANSFER_MEDIUM__HOST;
	TxPacketParams.UdpChecksumIncluded	= CONTROL__DISABLE;
	TxPacketParams.FavoriteStream		= CONTROL__DISABLE;
    ReturnedValue =  Ac49xTransmitPacket(&TxPacketParams);
	Ac49xUserDef_CriticalSection_LeaveCommand(Device);
	return ReturnedValue;
#endif
}

/***************************************************************************************************************************/


/*****************************************\
*******************************************
**  Device Reset and Download Routines   **
*******************************************
\*****************************************/

/****************************************************************/
Tac49xProgramDownloadStatus Ac49xResetAndRelease(int Device)
{
#if (AC49X_DEVICE_TYPE == AC490_DEVICE)
	const unsigned short DEVICE_UNRESET = 0x100; /* due to Msb first (big endian) */
/*    const int HPIC_REGISTER_ADDRESS	    = (-AC49XIF_BASE_ADDRESS);*/
#endif	
	if((Device%NUMBER_OF_DEVICES_PER_DSP) == 0)
	    {
		Ac49xUserDef_VoppReset(Device, 1);	 
		Ac49xUserDef_DelayForDeviceResponse(); /* Reset pulse */
		Ac49xUserDef_VoppReset(Device, 0);  
	    }
#if (AC49X_DEVICE_TYPE == AC490_DEVICE)
	Ac49xWriteBlock(Device, HPIC_Offset, (char*)&DEVICE_UNRESET, 2); 	/* Device Unreset (By Setting "1" to HPIC Register) */
#endif	
    return PROGRAM_DOWNLOAD_STATUS__NULL_BOOT_BLOCK;
}

#if	(  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC495_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC497_DEVICE)\
	)
#if(AC49X_NUMBER_OF_DEVICES>1)

int Ac498_ProgamDownload(int Device, char* pProgram)
{
	Tac49xBootStatusRegister    BootStatusRegister      ;
	Tac49xDeviceControlRegister DeviceControlRegister   ;
	unsigned long               BlockSize               ;
	unsigned long               DspOffset               ;	
    int                         iteration               ; 
	Tac49xProgramDownloadStatus Status               = PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS;
	Tac49xControl               LastBlock            = CONTROL__DISABLE; 
    Tac49xControl               Daram0               = CONTROL__ENABLE; 
	unsigned char               *pTempProgramPointer = (unsigned char*)pProgram;
	pTempProgramPointer++;
	pTempProgramPointer++;

    if(Device<AC49X_NUMBER_OF_DEVICES-1)
        AcVlynq_MapToAc498Registers();
    Ac49xUserDef_498SetPeripheralResetRegister(Device, AC498_PRCR_DSP_MIF_BIT);
	  AcVlynq_MapToAc498LocalMemory();            /*DARAM0(0x0)-SARAM3(0x0004FFFF)                                */

    while(!LastBlock)  /* Download DSP Code to External Memory */
		{
		if(pTempProgramPointer[0] & PROGRAM_DOWLOAD__LAST_BLOCK_FLAG)   
			LastBlock = CONTROL__ENABLE;
 
        DspOffset =  (unsigned long)((*(pTempProgramPointer++) & 0x7F) << 24);
		DspOffset += (unsigned long)((*(pTempProgramPointer++)) << 16);
		DspOffset += (unsigned long)((*(pTempProgramPointer++)) << 8);
		DspOffset += (unsigned long)((*(pTempProgramPointer++)));
        DspOffset <<= DSP_TO_HOST_ADDRESS_SHIFT; 
        if(DspOffset>0x00003FFF)
			{
			if (Daram0)
				{
				memset((void*)&BootStatusRegister, (char)3, sizeof(Tac49xBootStatusRegister));
				Ac49xWriteStruct(Device, HPI_MEMORY__BOOT_STATUS_REG, &BootStatusRegister);                      
				memset((void*)&DeviceControlRegister, (char)0, sizeof(Tac49xDeviceControlRegister));        
				Ac49xWriteStruct(Device, HPI_MEMORY__DEVICE_CONTROL_REG, &DeviceControlRegister);   
	            AcVlynq_MapToAc498Registers();                  /* Remap to Reste,Clock ctrl  0x02080000-0x02081FFF*/
                Ac49xUserDef_498SetPeripheralResetRegister(Device, AC498_PRCR_DSP_MCDMA_BIT|AC498_PRCR_DSP_DSP_BIT|AC498_PRCR_DSP_PDMA_BIT|AC498_PRCR_DSP_MBSP0_BIT);

                if(Device==1)
                    {
                    if(AcVlynq_494UpadateVlynqClk())
                        return PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_NOT_READY;
                    if(AcVlynq_WaitForVlynqLinkStatus())
                        return PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_NOT_READY;
                    }
	            AcVlynq_MapToAc498LocalMemory();                /* DARAM0(0x0)-SARAM3(0x0004FFFF)                               */ 
				}
			Daram0 = CONTROL__DISABLE;

			}

        BlockSize =  (unsigned long)((*(pTempProgramPointer++)) << 8);
		BlockSize += (unsigned long)((*(pTempProgramPointer++)));
		pTempProgramPointer++;/*Dummy*/
		pTempProgramPointer++;/*Dummy*/
        BlockSize <<= DSP_TO_HOST_ADDRESS_SHIFT; 
		Ac49xWriteBlock(Device, DspOffset, pTempProgramPointer, BlockSize);
		pTempProgramPointer += BlockSize;               
		} /*while(!LastBlock): end of program blocks writing Loop */
    
	memset((void*)&BootStatusRegister, (char)0, sizeof(Tac49xBootStatusRegister));
	Ac49xWriteStruct(Device, HPI_MEMORY__BOOT_STATUS_REG, &BootStatusRegister);

    for(iteration=1; iteration<MAX_DELAY_ITERATIONS; iteration++)
		{
		Status = PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS;
		Ac49xReadStruct(Device, HPI_MEMORY__BOOT_STATUS_REG, &BootStatusRegister);
		Status *= (Tac49xProgramDownloadStatus)BootStatusRegister.Status;
	    Ac49xUserDef_DelayForDeviceResponse();
		if(Status != PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_NOT_READY)
			break;
		}
    return Status;
}

#endif /*(AC49X_NUMBER_OF_DEVICES>1)*/
#endif

Tac49xProgramDownloadStatus Ac49xProgramDownLoad(int Device, char *pProgram, int ApplicationParam)
{
#if (AC49X_DEVICE_TYPE == AC490_DEVICE)
	Tac49xBootStatusRegister     BootStatusRegister;
    int                          Iteration;
    Tac49xHpic                   Hpic;
	U16                          BlockSize;
  /*U32                          BlockAddress; msb is last block */
	int                          LastBlock				 = 0;	   
	U8                          *pBlock                  = (U8*)pProgram;	
/*  const int                    HPIC_REGISTER_ADDRESS	 = (-AC49XIF_BASE_ADDRESS);   zero address*/
    memset((void*)&Hpic, (char)0, sizeof(Tac49xHpic));
    Hpic.Reset  = 1;        /* Device Unreset (By Setting "1" to HPIC Register). DSP jumps to 10000h.*/
    Hpic.DspInt = 1;
                            /*wait until boot status empty */
    Iteration                   = MAX_DELAY_ITERATIONS;
    BootStatusRegister.Status   = PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_NOT_READY;
    while(Iteration-- && (BootStatusRegister.Status != PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS))
        {
        Ac49xReadStruct(Device, HPI_MEMORY__BOOT_STATUS_REG, &BootStatusRegister); 
        Ac49xUserDef_DelayForDeviceResponse();
        }
    if(BootStatusRegister.Status != PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS)
        return BootStatusRegister.Status;
    while(!LastBlock)           
		{                   /* program blocks writing Loop */
        LastBlock =  pBlock[0] & PROGRAM_DOWLOAD__LAST_BLOCK_FLAG; 
        BlockSize = (pBlock[4]<<8) | pBlock[5];
        BlockSize += 4; /* Boot block structure size in words */
        BlockSize <<= DSP_TO_HOST_ADDRESS_SHIFT;    /* tranlate words to bytes */      
	    Ac49xWriteBlock(Device, _490_PROGRAM_DOWNLOAD_BLOCK_ADDRESS, pBlock, BlockSize);
        BootStatusRegister.Status = PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_NOT_READY;
        Ac49xWriteStruct(Device, HPI_MEMORY__BOOT_STATUS_REG, &BootStatusRegister);         /* write 'full' to boot status register */
        Ac49xWriteBlock(Device, HPIC_Offset, (char*)&Hpic, sizeof(Tac49xHpic));   /* set hpi interrupt */	 
                            /* wait until boot status not full */
        Iteration = MAX_DELAY_ITERATIONS;
        while( (BootStatusRegister.Status == PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_NOT_READY) 
             && Iteration--
             )
            {
            Ac49xReadStruct(Device, HPI_MEMORY__BOOT_STATUS_REG, &BootStatusRegister); 
            Ac49xUserDef_DelayForDeviceResponse();
            }
        if(BootStatusRegister.Status != PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS) /* if boot status = Checksum error then return */
            return BootStatusRegister.Status;
        pBlock += BlockSize; /* to next block */
		}  
	Ac49xReadBlock(Device, HCRF__INT_ENTRY_ADDRESS, acHcrfIntEntry, HCRF__INT_ENTRY_SIZE);
    return BootStatusRegister.Status;
#elif (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	  ) 
	int							 bGpio0;/*StatusRegister; GeneralPurposeIO_Register0;*/
	int			                 TempDevice;
	unsigned long                DspOffset;	
	unsigned long                BlockSize;
	unsigned char               *pTempProgramPointer     = (unsigned char*)pProgram;	
	int                          LastBlock				 = 0;	   
	int                          BlockIdForDebuggingOnly = 0;    
    int                          iteration; 
	Tac49xProgramDownloadStatus  Status                  = PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS;
	Tac49xBootStatusRegister     BootStatusRegister    ;
	Tac49xDeviceControlRegister  DeviceControlRegister ;
    static int                   PreviousApplicationParam;
	static char EntryAddressWriteContent[NUMBER_OF_DEVICES_PER_DSP][4];

	if((Device%NUMBER_OF_DEVICES_PER_DSP) == 0) 
		{            /* Reset pulse */
		Ac49xUserDef_VoppReset(Device, 1);	 
		Ac49xUserDef_DelayForDeviceResponse(); 
		Ac49xUserDef_VoppReset(Device, 0);  
		}
                    /* from this stage HPI works. */
                    /* if AC490_DEVICE, all devices reset */
                    /* if AC491_DEVICE, all devices works by polling address (0x100*2) while NULL */
	if((Device%NUMBER_OF_DEVICES_PER_DSP) == 0)
        {
		for(iteration=0; iteration<2; iteration++)
			{
			bGpio0 = Ac49xUserDef_GetGpio0(Device);
			if(bGpio0)
				break;
    		Ac49xUserDef_DelayForDeviceResponse();
			}
	    if(!bGpio0)
		    return PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_NOT_READY; 

		Ac49xDeviceInitHPI(Device);
		/*
		Ac49xUserDef_WriteShort(Device, HPIC1st, (char)(MSB_FIRST|DSPINT_DEACTIVE|HINT_DEACTIVE));
		Ac49xUserDef_WriteShort(Device, HPIC2nd, (char)(MSB_FIRST|DSPINT_DEACTIVE|HINT_DEACTIVE));
		*/
        }
	while(!LastBlock)           /* program blocks writing Loop */
		{
        BlockIdForDebuggingOnly++;
		if(pTempProgramPointer[0] & PROGRAM_DOWLOAD__LAST_BLOCK_FLAG) 
			LastBlock = 1;
		DspOffset =  (unsigned long)((*(pTempProgramPointer++) & 0x7F) << 24);
		DspOffset += (unsigned long)((*(pTempProgramPointer++)) << 16);
		DspOffset += (unsigned long)((*(pTempProgramPointer++)) << 8);
		DspOffset += (unsigned long)((*(pTempProgramPointer++)));
		DspOffset <<= DSP_TO_HOST_ADDRESS_SHIFT;
		BlockSize =  (unsigned long)((*(pTempProgramPointer++)) << 8);
		BlockSize += (unsigned long)((*(pTempProgramPointer++)));
        BlockSize <<= DSP_TO_HOST_ADDRESS_SHIFT;
        if( (  ((Device%NUMBER_OF_DEVICES_PER_DSP) == 0)
            || (ApplicationParam != PreviousApplicationParam)
            )                               
          && (DspOffset >= 0xF000000)     
          && (DspOffset <= 0xFFFFFFF)     
          )
	        Ac49xWriteBlock((Device/NUMBER_OF_DEVICES_PER_DSP)*NUMBER_OF_DEVICES_PER_DSP, DspOffset, (char *)pTempProgramPointer, BlockSize); /* Case Shared Memory */
          else
              if(DspOffset<=0x1000000)
	        Ac49xWriteBlock(Device, DspOffset, (char *)pTempProgramPointer, BlockSize);    /* Case Local Memory */
		pTempProgramPointer += BlockSize;               
		} /*while(!LastBlock): end of program blocks writing Loop */

    PreviousApplicationParam = ApplicationParam;

	memset((void*)&BootStatusRegister, (char)0, sizeof(Tac49xBootStatusRegister));
	Ac49xWriteStruct(Device, HPI_MEMORY__BOOT_STATUS_REG, &BootStatusRegister); 
                                       
	memset((void*)&DeviceControlRegister, (char)0, sizeof(Tac49xDeviceControlRegister));
	Ac49xWriteStruct(Device, HPI_MEMORY__DEVICE_CONTROL_REG, &DeviceControlRegister);   
                        /* Make the program start running*/

	memcpy((void*)(&EntryAddressWriteContent[Device][0]) ,pTempProgramPointer, 4);
	if((Device % NUMBER_OF_DEVICES_PER_DSP) == NUMBER_OF_DEVICES_PER_DSP-1)
		{
		for(TempDevice=Device-NUMBER_OF_DEVICES_PER_DSP+1; TempDevice<=Device; TempDevice++)
			Ac49xWriteBlock(TempDevice, (AC49XIF_ENTRY_ADDRESS<<DSP_TO_HOST_ADDRESS_SHIFT), EntryAddressWriteContent[TempDevice], 4);    /* Write the Program Entry Address to the specific Polled cell */
				        /* Loop while reading the boot Status from the DSP. */
        for(iteration=1; iteration<MAX_DELAY_ITERATIONS; iteration++)
			{
			Status = PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS;
			for(TempDevice=Device-NUMBER_OF_DEVICES_PER_DSP+1; TempDevice<=Device; TempDevice++)
				{
				Ac49xReadStruct(TempDevice, HPI_MEMORY__BOOT_STATUS_REG, &BootStatusRegister);
				Status *= (Tac49xProgramDownloadStatus)BootStatusRegister.Status;
				Ac49xUserDef_DelayForDeviceResponse();
				}
			if(Status != PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_NOT_READY)
				break;
			}
        }
    return Status;
#elif ((AC49X_DEVICE_TYPE == AC494_DEVICE) || (AC49X_DEVICE_TYPE == AC495_DEVICE) || (AC49X_DEVICE_TYPE == AC496_DEVICE) || (AC49X_DEVICE_TYPE == AC497_DEVICE))

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                                          
~~~~  HPI_MEMORY__BOOT_STATUS_REG     ~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                                          
~~~~  HPI_MEMORY__DEVICE_CONTROL_REG  ~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                                          
*/
	Tac49xBootStatusRegister    BootStatusRegister      ;
	Tac49xDeviceControlRegister DeviceControlRegister   ;
	unsigned long               BlockSize               ;
	unsigned long               DspOffset               ;	
    int                         iteration               ; 
	Tac49xProgramDownloadStatus Status               = PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS;
	Tac49xControl               LastBlock            = CONTROL__DISABLE; 
    Tac49xControl               ExternalMemory       = CONTROL__ENABLE; 
    Tac49xControl               Daram2Or3            = CONTROL__DISABLE; 
	unsigned char               *pTempProgramPointer = (unsigned char*)pProgram;

	pTempProgramPointer++;
	pTempProgramPointer++;

    while(!LastBlock)  /* Download DSP Code to External Memory */
		{
		if(pTempProgramPointer[0] & PROGRAM_DOWLOAD__LAST_BLOCK_FLAG)   
			LastBlock = CONTROL__ENABLE;
        if(!(pTempProgramPointer[1] & PROGRAM_DOWLOAD__EXTERNAL_MEMORY_INTERFACE_FLAG))     /*if the current block should be written to internal DSP memory */
            {
            if(ExternalMemory)                                                              /* and the previous block was writen to External Memory */
                {                                                                           /* then init DSP */
        		Ac49xUserDef_HardResetAssertFor16Cycles();    
        		Ac49xUserDef_HardResetReleaseAndWait150Cycles();
				Ac49xUserDef_DelayForDeviceResponse();
				Ac49xUserDef_PllProgramTo125_MHz();
				Ac49xUserDef_InitCache();
        		Ac49xUserDef_MifAndPeripheralsResetRelease();  
                }
            ExternalMemory = CONTROL__DISABLE;
            }
		DspOffset =  (unsigned long)((*(pTempProgramPointer++) & 0x7F) << 24);
		DspOffset += (unsigned long)((*(pTempProgramPointer++)) << 16);
		DspOffset += (unsigned long)((*(pTempProgramPointer++)) << 8);
		DspOffset += (unsigned long)((*(pTempProgramPointer++)));
        if(DspOffset>=0x4000 && DspOffset<0x10000)
			{
			if (!Daram2Or3)
				{
				memset((void*)&BootStatusRegister, (char)3, sizeof(Tac49xBootStatusRegister));
				Ac49xWriteStruct(0, HPI_MEMORY__BOOT_STATUS_REG, &BootStatusRegister);                      
				memset((void*)&DeviceControlRegister, (char)0, sizeof(Tac49xDeviceControlRegister));        
				Ac49xWriteStruct(0, HPI_MEMORY__DEVICE_CONTROL_REG, &DeviceControlRegister);   
				Ac49xUserDef_DspResetRelease();
				Ac49xUserDef_CodecResetRelease();
				}
			Daram2Or3 = CONTROL__ENABLE;
			}

        DspOffset <<= DSP_TO_HOST_ADDRESS_SHIFT; 
        BlockSize =  (unsigned long)((*(pTempProgramPointer++)) << 8);
		BlockSize += (unsigned long)((*(pTempProgramPointer++)));
		pTempProgramPointer++;/*Dummy*/
		pTempProgramPointer++;/*Dummy*/
        BlockSize <<= DSP_TO_HOST_ADDRESS_SHIFT; 
        if(ExternalMemory)
			Ac49xUserDef_EMIF_MEMCPY(DspOffset, pTempProgramPointer, BlockSize);
        else
			Ac49xWriteBlock(0, DspOffset, pTempProgramPointer, BlockSize);
		pTempProgramPointer += BlockSize;               
		} /*while(!LastBlock): end of program blocks writing Loop */
	memset((void*)&BootStatusRegister, (char)0, sizeof(Tac49xBootStatusRegister));
	Ac49xWriteStruct(0, HPI_MEMORY__BOOT_STATUS_REG, &BootStatusRegister);

    for(iteration=1; iteration<MAX_DELAY_ITERATIONS; iteration++)
		{
		Status = PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS;
		Ac49xReadStruct(0, HPI_MEMORY__BOOT_STATUS_REG, &BootStatusRegister);
		Status *= (Tac49xProgramDownloadStatus)BootStatusRegister.Status;
	    Ac49xUserDef_DelayForDeviceResponse();
		if(Status != PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_NOT_READY)
			break;
		}
    return Status;
#elif (AC49X_DEVICE_TYPE == AC496E_DEVICE)
/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                                          
~~~~  HPI_MEMORY__BOOT_STATUS_REG     ~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                                          
~~~~  HPI_MEMORY__DEVICE_CONTROL_REG  ~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                                          
*/
	Tac49xBootStatusRegister    BootStatusRegister      ;
	Tac49xDeviceControlRegister DeviceControlRegister   ;
	unsigned long               BlockSize               ;
	unsigned long               DspOffset               ;	
    int                         iteration               ; 
	Tac49xProgramDownloadStatus Status               = PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS;
	Tac49xControl               LastBlock            = CONTROL__DISABLE; 
    Tac49xControl               ExternalMemory       = CONTROL__ENABLE; 
	unsigned char               *pTempProgramPointer = (unsigned char*)pProgram;
	pTempProgramPointer++;
	pTempProgramPointer++;

    while(!LastBlock)  /* Download DSP Code to External Memory */
		{
		if(pTempProgramPointer[0] & PROGRAM_DOWLOAD__LAST_BLOCK_FLAG)   
			LastBlock = CONTROL__ENABLE;
        if(!(pTempProgramPointer[1] & PROGRAM_DOWLOAD__EXTERNAL_MEMORY_INTERFACE_FLAG))     /*if the current block should be written to internal DSP memory */
            {
            if(ExternalMemory)                                                              /* and the previous block was writen to External Memory */
                {                                                                           /* then init DSP */
        		/*Ac49xUserDef_HardResetAssertFor16Cycles();*/    
        		Ac49xUserDef_HardResetReleaseAndWait150Cycles();
				Ac49xUserDef_DelayForDeviceResponse();
				/*Ac49xUserDef_PllProgramTo125_MHz();*/
				Ac49xUserDef_InitCache();
        		Ac49xUserDef_MifAndPeripheralsResetRelease();  
                }
            ExternalMemory = CONTROL__DISABLE;
            }
		DspOffset =  (unsigned long)((*(pTempProgramPointer++) & 0x7F) << 24);
		DspOffset += (unsigned long)((*(pTempProgramPointer++)) << 16);
		DspOffset += (unsigned long)((*(pTempProgramPointer++)) << 8);
		DspOffset += (unsigned long)((*(pTempProgramPointer++)));

        DspOffset <<= DSP_TO_HOST_ADDRESS_SHIFT; 
        BlockSize =  (unsigned long)((*(pTempProgramPointer++)) << 8);
		BlockSize += (unsigned long)((*(pTempProgramPointer++)));
		pTempProgramPointer++;/*Dummy*/
		pTempProgramPointer++;/*Dummy*/
        BlockSize <<= DSP_TO_HOST_ADDRESS_SHIFT; 
		/*
        if(ExternalMemory)
			Ac49xUserDef_EMIF_MEMCPY(DspOffset, pTempProgramPointer, BlockSize);
        else
			Ac49xWriteBlock(0, DspOffset, pTempProgramPointer, BlockSize);
		pTempProgramPointer += BlockSize;               
		*/
		} /*while(!LastBlock): end of program blocks writing Loop */

	memset((void*)&DeviceControlRegister, (char)0, sizeof(Tac49xDeviceControlRegister));        
	Ac49xWriteStruct(0, HPI_MEMORY__DEVICE_CONTROL_REG, &DeviceControlRegister);   

	memset((void*)&BootStatusRegister, (char)0, sizeof(Tac49xBootStatusRegister));
	Ac49xWriteStruct(0, HPI_MEMORY__BOOT_STATUS_REG, &BootStatusRegister);

	Ac49xUserDef_DspResetRelease();

    for(iteration=1; iteration<MAX_DELAY_ITERATIONS; iteration++)
		{
		Status = PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS;
		Ac49xReadStruct(0, HPI_MEMORY__BOOT_STATUS_REG, &BootStatusRegister);
		Status *= (Tac49xProgramDownloadStatus)BootStatusRegister.Status;
	    Ac49xUserDef_DelayForDeviceResponse();
		if(Status != PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_NOT_READY)
			break;
		}
    return Status;


#endif /*AC49X_DEVICE_TYPE*/
	return PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS;
}
/*********************************************************************/
void Ac49xKernelDownLoad(int Device, char *pKernel)
{
#if (AC49X_DEVICE_TYPE == AC490_DEVICE)

    U8                        *pBlock;
    U32                         BlockAddress;
    U16                         BlockSize;
    U32							LastBlock; 
	Tac49xBootStatusRegister    BootStatusRegister;
    Tac49xHpic                  Hpic;
    memset((void*)&Hpic,                (char)0, sizeof(Tac49xHpic));
    memset((void*)&BootStatusRegister,  (char)0, sizeof(Tac49xBootStatusRegister));

	Ac49xUserDef_VoppReset(Device, 1);	 
	Ac49xUserDef_DelayForDeviceResponse(); 
	Ac49xUserDef_VoppReset(Device, 0);
    
    pBlock      = (U8*)pKernel;
    LastBlock   = 0;
    while(!LastBlock)
        {
        LastBlock       = pBlock[0] & PROGRAM_DOWLOAD__LAST_BLOCK_FLAG; 
        BlockAddress    = ((pBlock[0] & 0x7f)<<24) | ((pBlock[1])<<16) | ((pBlock[2])<<8) | pBlock[3];
        BlockSize       = (pBlock[4]<<8) | pBlock[5];
        pBlock += 8;
        BlockSize    <<= DSP_TO_HOST_ADDRESS_SHIFT;  /* tranlate words to bytes */    
        BlockAddress <<= DSP_TO_HOST_ADDRESS_SHIFT;  /* tranlate words to bytes */ 

	    Ac49xWriteBlock(Device, BlockAddress, pBlock, BlockSize); 
        pBlock += BlockSize; /* point to next Block info */
        } 

    BootStatusRegister.Status = PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_NOT_READY;
    Ac49xWriteStruct(Device, HPI_MEMORY__BOOT_STATUS_REG, &BootStatusRegister); 
    Hpic.Reset  = 1;        /* Device Unreset (By Setting "1" to HPIC Register). DSP jumps to 10000h. */
    Ac49xWriteBlock(Device, HPIC_Offset, (char*)&Hpic, sizeof(Tac49xHpic)); 	 
#endif
}

/*********************************************************************/
#if (AC49X_DEVICE_TYPE == AC490_DEVICE)
Tac49xEmifTestStatus Ac49xEmifTest(int Device, char *pProgram, char *pKernel, Tac49xEmifTestErrorInformationAttr *pAttr)
{
	Tac49xProgramDownloadStatus     ProgramDownloadStatus;
	int MaxNumberOfErrors = EMIF_TEST__ERROR_INFORMATION_SIZE/sizeof(Tac49xSingleEmifTestErrorInformationPayload);
	int i;

	Tac49xEmifTestErrorInformationPayload Payload;

	ProgramDownloadStatus = Ac49xBoot(Device, pProgram, 0, pKernel);
    switch(ProgramDownloadStatus)
        {
        case PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_FAIL_CHECKSUM:
        case PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_NOT_READY:
        case PROGRAM_DOWNLOAD_STATUS__NULL_BOOT_BLOCK:
            return EMIF_TEST_STATUS__FAILED_DUE_TO_PROGRAM_DOWNLOAD;
        case PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS:
		    return EMIF_TEST_STATUS__PASS;
        case PROGRAM_DOWNLOAD_STATUS__EMIF_TEST_FAILED:
	        for(i=0; i<MaxNumberOfErrors; i++)
		        {
		        pAttr->SingleEmifTestErrorInformation[i].ErrorAddress  = MergeFieldsToLong( Payload.SingleEmifTestErrorInformation[i].ErrorAddress);
		        pAttr->SingleEmifTestErrorInformation[i].TestType	   = MergeFieldsToShort(Payload.SingleEmifTestErrorInformation[i].TestType);
		        pAttr->SingleEmifTestErrorInformation[i].ExpectedValue = MergeFieldsToShort(Payload.SingleEmifTestErrorInformation[i].ExpectedValue);
		        pAttr->SingleEmifTestErrorInformation[i].ReceivedValue = MergeFieldsToShort(Payload.SingleEmifTestErrorInformation[i].ReceivedValue);
		        }
            return EMIF_TEST_STATUS__FAILED_DUE_TO_TEST_RESULT;
        }
	return EMIF_TEST_STATUS__PASS;
}
#endif /*(AC49X_DEVICE_TYPE == AC490_DEVICE)*/
/***********************************************/
/***********************************************/
/*   T H E   C O N V E N I E N T   G R O U P  **/
/*   High-Level Initialization and Setup      **/
/***********************************************/
/***********************************************/

/****************************************************************/
void Ac49xResetDriver(void)
{
	Ac49xResetBufferDescriptors();
#if (FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__NETWORK)
    Ac49xInitFifo();
#endif /*(FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__NETWORK)*/
}

/****************************************************************/
int Ac49xInitDriver(Tac49xControl DebugMode)
{
    Ac49xUserDef_InitUserDefSection();
	Ac49xResetBufferDescriptors();
	Ac49xSetDebugMode(DebugMode);
	Ac49xHcrfSetHostControlRegisterAddresses();
#if (FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__NETWORK)
    Ac49xInitFifo();
#endif /*(FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__NETWORK)*/
#if (PACKET_RECORDING_ENABLE==1)
	_49xPacketRecording.Initialize		= Initialize;
	_49xPacketRecording.Finalize		= Finalize;
	_49xPacketRecording.Add				= Add;
    _49xPacketRecording.Initialize();
#endif /*(PACKET_RECORDING_ENABLE==1)*/
    return 0;
}

/****************************************************************/
Tac49xProgramDownloadStatus Ac49xBoot(int Device, char *pProgram, int ApplicationParam, char *pKernel)
{
    const int VERSION_DATA_OFFSET   = 34;

	if(pProgram)
		pProgram += VERSION_DATA_OFFSET;
	if(pKernel)
		pKernel  += VERSION_DATA_OFFSET;
    if(pKernel)
        Ac49xKernelDownLoad(Device, pKernel);
    if(pProgram)
        {
#if	((AC49X_DEVICE_TYPE == AC494_DEVICE) || (AC49X_DEVICE_TYPE == AC495_DEVICE) || (AC49X_DEVICE_TYPE == AC496_DEVICE) || (AC49X_DEVICE_TYPE == AC496E_DEVICE) || (AC49X_DEVICE_TYPE == AC497_DEVICE))
    #if(AC49X_NUMBER_OF_DEVICES>1)
        if(Device>0)
            return Ac498_ProgamDownload(Device, pProgram);
        else
    #endif
#endif
            return Ac49xProgramDownLoad(Device, pProgram, ApplicationParam);
        }
    else
        return Ac49xResetAndRelease(Device);
    return PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS;
}

/****************************************************************/
void Ac49xGetDriverVersionInfo(Tac49xDriverVersionInfo *pDriverVersionInfo){*pDriverVersionInfo = ac49xDriverVersionInfo;}

/****************************************************************/
void Ac49xGetDspDownloadInfo(Tac49xDspDownloadInfo *pDspDownloadInfo, const char *DownloadBlock)
{
    pDspDownloadInfo->Version[0]		= '\0';
    pDspDownloadInfo->Revision[0]		= '\0';
    pDspDownloadInfo->Patch[0]			= '\0';
    pDspDownloadInfo->PatchNumber[0]	= '\0';
    pDspDownloadInfo->Month[0]			= '\0';
    pDspDownloadInfo->Day[0]			= '\0';
    pDspDownloadInfo->Year[0]			= '\0';
    pDspDownloadInfo->Reserved[0]		= '\0';
    pDspDownloadInfo->Name[0]			= '\0';
    if(!DownloadBlock)
        return;
    pDspDownloadInfo->Version[0] = DownloadBlock[0];
    pDspDownloadInfo->Version[1] = DownloadBlock[1];
    pDspDownloadInfo->Version[2] = '.';
    pDspDownloadInfo->Version[3] = DownloadBlock[2];
    pDspDownloadInfo->Version[4] = DownloadBlock[3];
    pDspDownloadInfo->Version[5] = 0;
    pDspDownloadInfo->Revision[0] = DownloadBlock[4];
    pDspDownloadInfo->Revision[1] = DownloadBlock[5];
    pDspDownloadInfo->Revision[2] = 0;
    pDspDownloadInfo->Patch[0] = DownloadBlock[6];
    pDspDownloadInfo->Patch[1] = DownloadBlock[7];
    pDspDownloadInfo->Patch[2] = 0;
    pDspDownloadInfo->PatchNumber[0] = DownloadBlock[8];
    pDspDownloadInfo->PatchNumber[1] = DownloadBlock[9];
    pDspDownloadInfo->PatchNumber[2] = 0;
    pDspDownloadInfo->Reserved[0] = DownloadBlock[10];
    pDspDownloadInfo->Reserved[1] = DownloadBlock[11];
    pDspDownloadInfo->Reserved[2] = DownloadBlock[12];
    pDspDownloadInfo->Reserved[3] = DownloadBlock[13];
    pDspDownloadInfo->Reserved[4] = DownloadBlock[14];
    pDspDownloadInfo->Reserved[5] = DownloadBlock[15];
    pDspDownloadInfo->Reserved[6] = 0;
    pDspDownloadInfo->Month[0] = DownloadBlock[16];
    pDspDownloadInfo->Month[1] = DownloadBlock[17];
    pDspDownloadInfo->Month[2] = 0;
    pDspDownloadInfo->Day[0] = DownloadBlock[18];
    pDspDownloadInfo->Day[1] = DownloadBlock[19];
    pDspDownloadInfo->Day[2] = 0;
    pDspDownloadInfo->Year[0] = DownloadBlock[20];
    pDspDownloadInfo->Year[1] = DownloadBlock[21];
    pDspDownloadInfo->Year[2] = DownloadBlock[22];
    pDspDownloadInfo->Year[3] = DownloadBlock[23];
    pDspDownloadInfo->Year[4] = 0;
    pDspDownloadInfo->Name[0] = 'A';
    pDspDownloadInfo->Name[1] = 'C';
    pDspDownloadInfo->Name[2] = '4';
    pDspDownloadInfo->Name[3] = '9';
    pDspDownloadInfo->Name[4] = DownloadBlock[24];
    pDspDownloadInfo->Name[5] = DownloadBlock[25];
    pDspDownloadInfo->Name[6] = DownloadBlock[26];
    pDspDownloadInfo->Name[7] = DownloadBlock[27];
    pDspDownloadInfo->Name[8] = DownloadBlock[28];
    pDspDownloadInfo->Name[9] = DownloadBlock[29];
    pDspDownloadInfo->Name[10] = DownloadBlock[30];
    pDspDownloadInfo->Name[11] = DownloadBlock[31];
    pDspDownloadInfo->Name[12] = DownloadBlock[32];
    pDspDownloadInfo->Name[13] = DownloadBlock[33];
    pDspDownloadInfo->Name[14] = DownloadBlock[34]; 
    if(pDspDownloadInfo->Name[11] == 0)
		{
        pDspDownloadInfo->Name[11] = '-'; /* in case of shorter name */
		pDspDownloadInfo->Name[12] = 'S';
		pDspDownloadInfo->Name[13] = 0;
		}
	else
		{
		pDspDownloadInfo->Name[15] = '-';
		pDspDownloadInfo->Name[16] = 'S';
		pDspDownloadInfo->Name[17] = 0;
		}
}

/****************************************************************/
Tac49xSetupDeviceStatus Ac49xSetupDevice(int Device, Tac49xSetupDeviceAttr *pSetupDeviceAttr)
{
    int Error;
    if((Error = Ac49xDeviceSetControlRegister(             Device, &pSetupDeviceAttr->DeviceControlRegister)))
		return SETUP_DEVICE_STATUS__FAILED__DUE_TO_SET_DEVICE_CONTROL_REGISTER;
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	)
#ifdef UTOPIA_ROUTING
	if((Error = Ac49xUtopiaRoutingConfiguration(		   Device, &pSetupDeviceAttr->UtopiaRouting)))
		return SETUP_DEVICE_STATUS__FAILED__DUE_TO_UTOPIA_ROUTING_CONFIGURATION;
#endif /* UTOPIA_ROUTING */
#endif /* (AC49X_DEVICE_TYPE==AC491_DEVICE) || (AC49X_DEVICE_TYPE==AC491L_DEVICE) */
	if((Error = Ac49xCallProgressDeviceConfiguration(      Device, &pSetupDeviceAttr->CallProgress)))
		return SETUP_DEVICE_STATUS__FAILED__DUE_TO_CALL_PROGRESS_CONFIGURATION;
    if((Error = Ac49xUserDefinedTonesDeviceConfiguration(  Device, &pSetupDeviceAttr->UserDefinedTones)))
        return SETUP_DEVICE_STATUS__FAILED__DUE_TO_USER_DEFINED_TONES_CONFIGURATION;
    if((Error = Ac49xAgcDeviceConfiguration(               Device, &pSetupDeviceAttr->Agc)))
		return SETUP_DEVICE_STATUS__FAILED__DUE_TO_AGC_CONFIGURATION;
    if((Error = Ac49xExtendedDeviceConfiguration(          Device, &pSetupDeviceAttr->Extended)))
        return SETUP_DEVICE_STATUS__FAILED__DUE_TO_EXTENDED_CONFIGURATION;

	switch (pSetupDeviceAttr->OpenDeviceFormat)
		{
		case FORMAT__0:
			if((Error = Ac49xOpenDeviceConfiguration_Format0(Device, &pSetupDeviceAttr->Open))) /* The Core exits the init state and enters the run state after it completes executing the Open Device Command */
				return SETUP_DEVICE_STATUS__FAILED__DUE_TO_OPEN_CONFIGURATION;
			break;
		case FORMAT__1:
			if((Error = Ac49xOpenDeviceConfiguration_Format1(Device, &pSetupDeviceAttr->OpenFormat1))) /* The Core exits the init state and enters the run state after it completes executing the Open Device Command */
				return SETUP_DEVICE_STATUS__FAILED__DUE_TO_OPEN_CONFIGURATION;
			break;
    default:;
		}
    
    return SETUP_DEVICE_STATUS__SUCCESS;
}

/*******************************************/                                                                                                           
/*******************************************/                                                                                                           
/*     Attribute Setup Default Values     **/                                                                                                           
/*******************************************/                                                                                                           
/*******************************************/  

/****************************************************************/
void Ac49xSetDefaultDeviceControlRegisterAttr(Tac49xDeviceControlRegisterAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xDeviceControlRegisterAttr));
	pAttr->NetworkCellSize	= 56;
    pAttr->HostCellSize		= 44;
    pAttr->Reset			= CONTROL__ENABLE			;  
    pAttr->HostDebug 		= CONTROL__DISABLE			;  
    pAttr->NetworkDebug		= CONTROL__DISABLE			;  
}

/****************************************************************/
void Ac49xSetDefaultOpenDeviceConfigurationAttr(Tac49xOpenDeviceConfigurationAttr *pAttr)
{
	int Channel;
	memset((void*)pAttr, 0, sizeof(Tac49xOpenDeviceConfigurationAttr));
	pAttr->Common.IpSecPortSelection			            = IP_SEC_PORT_SELECTION__HOST;
   // pAttr->Common.BusSpeed						            = BUS_SPEED__4MHZ           ;
       pAttr->Common.BusSpeed						            = BUS_SPEED__2MHZ           ;
    pAttr->Common.WatchdogEnable				            = CONTROL__ENABLE			;
    pAttr->Common.LawSelect						            = LAW_SELECT__A_LAW         ;
    pAttr->Common.UtopiaClockDetection			            = 0							;
    pAttr->Common.UtopiaCellSize				            = UTOPIA_CELL_SIZE__56_BYTES;
    pAttr->Common.Utopia16BitMode				            = CONTROL__ENABLE			;
    pAttr->Common.PcmSerialPortRxTxDataDelay	            = PCM_SERIAL_PORT_RX_TX_DATA_DELAY__1_Bit;
    pAttr->Common.UtopiaLogicalSlaveId			            = 0                         ;
    pAttr->Common.UtopiaPhysicalSlaveId			            = 0                         ;
    pAttr->Common.EventsPortType                            = EVENTS_PORT_TYPE__HOST;
    pAttr->Common.TdmInterfaceConfiguration                 = TDM_INTERFACE_CONFIGURATION__8KHz;

#if   (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
    pAttr->Common.CodecEnable					= CODEC_ENABLE__DISABLE;
	pAttr->Common.AicClkFrequency				= AIC_CLK_FREQUENCY__12288_KHZ;
#endif
	pAttr->Common.CpuClkOut						= CONTROL__DISABLE;
	pAttr->Common.PcmClkAndFrameSyncSource		= PCM_CLK_AND_FRAME_SYNC_SOURCE__EXTERNAL;
#if   (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	pAttr->Common.PcmClkDivideRatio				= 29;
	pAttr->Common.PcmFrameSyncWidth				= 0;
#else
	pAttr->Common.PcmClkDivideRatio				= 0;
	pAttr->Common.PcmFrameSyncWidth				= 0;
#endif
	pAttr->Common.CasBitsLocation				= CAS_BITS_LOCATION__LOW_NIBLE;

#if   (AC49X_DEVICE_TYPE == AC490_DEVICE)
    pAttr->NumberOfChannels						= 8;
#elif (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	  )
    pAttr->NumberOfChannels						= NUMBER_OF_CHANNELS;
#elif (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
    pAttr->NumberOfChannels						= 2;
#endif
	pAttr->ConnectedSlotEnable					= CONTROL__DISABLE			;
    pAttr->CasMode								= CAS_MODE__NO_SIGNALING;
	pAttr->SerialPortSelection					= SERIAL_PORT_SELECTION__VOICE_0_CAS_1;
    pAttr->JitterBufferStatusReportMode			= JITTER_BUFFER_STATUS_REPORT_MODE__SEND_IF_OVERRUN_OR_UNDERRUN_ONLY;
    pAttr->UtopiaTestEnable						= 0							;
    pAttr->CasReversalsDetection				= CONTROL__DISABLE          ;
	pAttr->DeviceStatusPacketEnable				= CONTROL__ENABLE			;
	pAttr->DebugInfoPacketEnable				= CONTROL__DISABLE			;
    pAttr->CasValueDuringIdle					= 0x00                      ;
    pAttr->PcmValueDuringIdle					= 0xD5                      ;
    pAttr->CasDebounce							= CAS_DEBOUNCE__30MS        ;
    pAttr->DtmfGenerationTwistDirection			= DTMF_GENERATION_TWIST_DIRECTION__HIGH_FREQUENCY_LEVEL_IS_HIGHER_THAN_LOW_FREQUENCY_LEVEL           ;
    pAttr->DtmfGenerationTwist					= DTMF_GENERATION_TWIST__DISABLED                         ;
    pAttr->ErrorReportInterval					= 10						;
    pAttr->DeviceStatusInterval					= 10						;
	pAttr->DtmfSnr								= DTMF_SNR__15DB			;
	pAttr->DtmfGap								= DTMF_GAP__30_msec			;
	pAttr->ModuleDebugPacketsTransferMethod		= MODULE_DEBUG_PACKETS_TRANSFER_METHOD__TO_DEBUG_PROTOCOL_ON_NETWORK_PORT;
#if 0
	for(Channel=0; Channel<AC49X_NUMBER_OF_CHANNELS; Channel++)
        {
        pAttr->Channel[Channel].Slot               = Channel + Dev * NUMBER_OF_CHANNELS;
        pAttr->Channel[Channel].InterconnectedSlot = Channel+AC49X_NUMBER_OF_CHANNELS + Dev * NUMBER_OF_CHANNELS;
        }
#else
	for(Channel=0; Channel<NUMBER_OF_CHANNELS; Channel++)
	{
	pAttr->Channel[Channel].Slot			   = Channel + Dev * NUMBER_OF_CHANNELS;
	//pAttr->Channel[Channel].InterconnectedSlot = Channel+AC49X_NUMBER_OF_CHANNELS + Dev * NUMBER_OF_CHANNELS;
	}

#endif
}

/****************************************************************/
void Ac49xSetDefaultOpenDeviceConfiguration_Format0Attr(Tac49xOpenDeviceConfiguration_Format0Attr *pAttr)
{
	Ac49xSetDefaultOpenDeviceConfigurationAttr((Tac49xOpenDeviceConfiguration_Format0Attr*)pAttr);
}

/****************************************************************/
void Ac49xSetDefaultOpenDeviceConfiguration_Format1Attr(Tac49xOpenDeviceConfiguration_Format1Attr *pAttr)
{
	int Channel;
	memset((void*)pAttr, 0, sizeof(Tac49xOpenDeviceConfiguration_Format1Attr));
	pAttr->Common.IpSecPortSelection			            = IP_SEC_PORT_SELECTION__HOST;
    pAttr->Common.BusSpeed						            = BUS_SPEED__4MHZ           ;
    pAttr->Common.WatchdogEnable				            = CONTROL__ENABLE			;
    pAttr->Common.LawSelect						            = LAW_SELECT__A_LAW         ;
    pAttr->Common.UtopiaClockDetection			            = 0							;
    pAttr->Common.UtopiaCellSize				            = UTOPIA_CELL_SIZE__56_BYTES;
    pAttr->Common.Utopia16BitMode				            = CONTROL__ENABLE			;
    pAttr->Common.PcmSerialPortRxTxDataDelay	            = PCM_SERIAL_PORT_RX_TX_DATA_DELAY__1_Bit;
    pAttr->Common.UtopiaLogicalSlaveId			            = 0                         ;
    pAttr->Common.UtopiaPhysicalSlaveId			            = 0                         ;
    pAttr->Common.EventsPortType                            = EVENTS_PORT_TYPE__HOST;
    pAttr->Common.TdmInterfaceConfiguration                 = TDM_INTERFACE_CONFIGURATION__8KHz;

	pAttr->ConnectedSlots									= CONTROL__DISABLE;
	pAttr->ConnectedSlotsSerialPortSelection				= SERIAL_PORT_SELECTION_FORMAT1__PORT_0;
	pAttr->CasSerialPortSelection							= SERIAL_PORT_SELECTION_FORMAT1__PORT_1;
	pAttr->VoiceSerialPortSelection							= SERIAL_PORT_SELECTION_FORMAT1__PORT_0;
	

#if   (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
    pAttr->Common.CodecEnable					= CODEC_ENABLE__DISABLE;
	pAttr->Common.AicClkFrequency				= AIC_CLK_FREQUENCY__12288_KHZ;
#endif
	pAttr->Common.CpuClkOut						= CONTROL__DISABLE;
	pAttr->Common.PcmClkAndFrameSyncSource		= PCM_CLK_AND_FRAME_SYNC_SOURCE__EXTERNAL;
#if   (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	pAttr->Common.PcmClkDivideRatio				= 29;
	pAttr->Common.PcmFrameSyncWidth				= 0;
#else
	pAttr->Common.PcmClkDivideRatio				= 0;
	pAttr->Common.PcmFrameSyncWidth				= 0;
#endif
	pAttr->Common.CasBitsLocation				= CAS_BITS_LOCATION__LOW_NIBLE;

#if   (AC49X_DEVICE_TYPE == AC490_DEVICE)
    pAttr->NumberOfChannels						= 8;
#elif (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	  )
    pAttr->NumberOfChannels						= 6;
#elif (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
    pAttr->NumberOfChannels						= 2;
#endif
    pAttr->CasMode								= CAS_MODE__NO_SIGNALING;
    pAttr->JitterBufferStatusReportMode			= JITTER_BUFFER_STATUS_REPORT_MODE__SEND_IF_OVERRUN_OR_UNDERRUN_ONLY;
    pAttr->UtopiaTestEnable						= 0							;
    pAttr->CasReversalsDetection				= CONTROL__DISABLE          ;
	pAttr->DeviceStatusPacketEnable				= CONTROL__ENABLE			;
	pAttr->DebugInfoPacketEnable				= CONTROL__DISABLE			;
    pAttr->CasValueDuringIdle					= 0x00                      ;
    pAttr->PcmValueDuringIdle					= 0xD5                      ;
    pAttr->CasDebounce							= CAS_DEBOUNCE__30MS        ;
    pAttr->DtmfGenerationTwistDirection			= DTMF_GENERATION_TWIST_DIRECTION__HIGH_FREQUENCY_LEVEL_IS_HIGHER_THAN_LOW_FREQUENCY_LEVEL           ;
    pAttr->DtmfGenerationTwist					= DTMF_GENERATION_TWIST__DISABLED                         ;
    pAttr->ErrorReportInterval					= 10						;
    pAttr->DeviceStatusInterval					= 10						;
	pAttr->DtmfSnr								= DTMF_SNR__15DB			;
	pAttr->DtmfGap								= DTMF_GAP__30_msec			;
	pAttr->ModuleDebugPacketsTransferMethod		= MODULE_DEBUG_PACKETS_TRANSFER_METHOD__TO_DEBUG_PROTOCOL_ON_NETWORK_PORT;

	for(Channel=0; Channel<AC49X_NUMBER_OF_CHANNELS; Channel++)
        {
        pAttr->Channel[Channel].PcmSlot				  = Channel;
		pAttr->Channel[Channel].CasSlot				  = Channel;
        pAttr->Channel[Channel].InterconnectedPcmSlot = Channel+AC49X_NUMBER_OF_CHANNELS;
        }
}

/****************************************************************/
void Ac49xSetDefaultAgcDeviceConfigurationAttr(Tac49xAgcDeviceConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xAgcDeviceConfigurationAttr));
	pAttr->MinGain							    = AGC_MIN_GAIN__minus20_DB  ;
    pAttr->MaxGain							    = AGC_MAX_GAIN__15_DB       ;
    pAttr->FastAdaptationGainSlope			    = AGC_GAIN_SLOPE__10_00_DB_SEC;
	pAttr->FastAdaptationDuration			    = 1500                      ;
	pAttr->SilenceDuration					    = 120                       ;
	pAttr->DetectionDuration				    = 15                        ;
	pAttr->SignalingOverhang					= 500                       ;
}

/****************************************************************/
void Ac49xSetDefaultExtendedDeviceConfigurationAttr(Tac49xExtendedDeviceConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xExtendedDeviceConfigurationAttr));
	pAttr->DebugUemClipEnable					= CONTROL__DISABLE;
	pAttr->TextV8Jm                             = CONTROL__ENABLE;
	pAttr->TextV8Cm                             = CONTROL__ENABLE;
	pAttr->TextV8Ci                             = CONTROL__ENABLE;
	pAttr->VxxV8Cm                              = CONTROL__ENABLE;
	pAttr->VxxV8Ci                              = CONTROL__ENABLE;
	pAttr->FaxV8Cm                              = CONTROL__ENABLE;
	pAttr->FaxV8Ci                              = CONTROL__ENABLE;
    pAttr->FaxDcn                               = CONTROL__ENABLE;
    pAttr->Tty                                  = CONTROL__ENABLE;
    pAttr->Bell103AnswerTone                    = CONTROL__DISABLE;
    pAttr->V21Channel1                          = CONTROL__ENABLE;
	pAttr->V25CallingTone                       = CONTROL__ENABLE;
	pAttr->StuModem                             = CONTROL__ENABLE;
    pAttr->VxxV8JmVxxData						= CONTROL__ENABLE;
    pAttr->V32Aa								= CONTROL__ENABLE;
    pAttr->V22Usb1								= CONTROL__ENABLE;
    pAttr->InitiatingDualToneV8Bis				= CONTROL__ENABLE;
    pAttr->RespondingDualToneV8Bis				= CONTROL__ENABLE;
    pAttr->DataSessionVxx						= CONTROL__ENABLE;
    pAttr->V21Channel2Data 						= CONTROL__ENABLE;
    pAttr->V23ForwardChannel					= CONTROL__ENABLE;
    pAttr->SilenceOrUnkown						= CONTROL__ENABLE;
    pAttr->CallingFaxToneCng					= CONTROL__ENABLE;
    pAttr->AnswerTone2100FaxCedOrModemAns		= CONTROL__ENABLE;
    pAttr->AnswerWithReversalsAns				= CONTROL__ENABLE;
    pAttr->AnswerToneAm							= CONTROL__ENABLE;
    pAttr->AnswerToneAmReversals				= CONTROL__ENABLE;
    pAttr->FaxV21PreambleFlags					= CONTROL__ENABLE;
    pAttr->FaxV8JmV34							= CONTROL__ENABLE;
    pAttr->MfcR1DetectionStandard               = MFC_R1_DETECTION_STANDARD__ITU;
    pAttr->DtmfErasureSensitivity               = DTMF_ERASURE_SENSITIVITY__LOW;
    pAttr->DtmfFrequencyDeviation               = DTMF_FREQUENCY_DEVIATION__Default_1_5_Percentage;
    pAttr->DtmfStepDistance						= DTMF_STEP_DISTANCE__DEFAULT_0_msec;
    pAttr->DtmfStepRatio						= DTMF_STEP_RATIO__DEFAULT_DISBALED;
}

/****************************************************************/
void Ac49xSetDefaultUserDefinedTonesDeviceConfigurationAttr(Tac49xUserDefinedTonesDeviceConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xUserDefinedTonesDeviceConfigurationAttr));
	pAttr->TotalEnergyThreshold  = 44;
}

/****************************************************************/
void Ac49xSetDefaultCallProgressDeviceConfigurationAttr(Tac49xCallProgressDeviceConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xCallProgressDeviceConfigurationAttr));
		                    /* -dbm */
	pAttr->TotalEnergyThreshold  = 44;	                        /* -dbm */
	pAttr->NumberOfSignals       = 9;
                /* Dial Tone (ANSI TI.401-1993) */
    pAttr->Signal[0].ToneAFrequency                 = 350;
    pAttr->Signal[0].ToneB_OrAmpModulationFrequency = 440;
    pAttr->Signal[0].LowEnergyThreshold             = 35;	    /* -dbm */
    pAttr->Signal[0].HighEnergyThreshold            = 0;	    /* dbm  */
    pAttr->Signal[0].Type							= CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS;
    pAttr->Signal[0].AmFactor                       = 0;
    pAttr->Signal[0].TwistThreshold                 = 10;
    pAttr->Signal[0].ToneALevel                     = 13;
    pAttr->Signal[0].ToneBLevel                     = 13;
    pAttr->Signal[0].SignalToNoiseRatioThreshold    = 15;
    pAttr->Signal[0].FrequencyDeviationThreshold    = 10;
	pAttr->Signal[0].DetectionTimeOrCadenceFirstOnOrBurstDuration = 300; /* 3 sec */
                /* Dial Tone (ITU-T Q.35) */
    pAttr->Signal[1].ToneAFrequency                 = 425;
    pAttr->Signal[1].ToneB_OrAmpModulationFrequency = 0;
    pAttr->Signal[1].LowEnergyThreshold             = 35;	    /* -dbm */
    pAttr->Signal[1].HighEnergyThreshold            = 0;	    /* dbm  */
    pAttr->Signal[1].Type							= CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS;
    pAttr->Signal[1].AmFactor                       = 0;
    pAttr->Signal[1].TwistThreshold                 = 10;
    pAttr->Signal[1].ToneALevel                     = 10;
    pAttr->Signal[1].ToneBLevel                     = 0;
    pAttr->Signal[1].SignalToNoiseRatioThreshold    = 15;
    pAttr->Signal[1].FrequencyDeviationThreshold    = 10;
	pAttr->Signal[1].DetectionTimeOrCadenceFirstOnOrBurstDuration = 300; /* 3 sec */
	            /* Audible Ringing Tone (ANSI TI.401-1993) */
    pAttr->Signal[2].ToneAFrequency                 = 440;
    pAttr->Signal[2].ToneB_OrAmpModulationFrequency = 480;
    pAttr->Signal[2].LowEnergyThreshold             = 35;	    /* -dbm */
    pAttr->Signal[2].HighEnergyThreshold            = 0;	    /* dbm  */
    pAttr->Signal[2].Type							= CALL_PROGRESS_SIGNAL_TYPE__CADENCE;
    pAttr->Signal[2].AmFactor                       = 0;
    pAttr->Signal[2].TwistThreshold                 = 10;
    pAttr->Signal[2].ToneALevel                     = 19;
    pAttr->Signal[2].ToneBLevel                     = 19;
    pAttr->Signal[2].DetectionTimeOrCadenceFirstOnOrBurstDuration  = 200;
    pAttr->Signal[2].CadenceFirstOffDuration        = 400;
    pAttr->Signal[2].SignalToNoiseRatioThreshold    = 15;
    pAttr->Signal[2].FrequencyDeviationThreshold    = 10;
	            /* Busy Tone (ANSI TI.401-1993) */
    pAttr->Signal[3].ToneAFrequency                 = 480;
    pAttr->Signal[3].ToneB_OrAmpModulationFrequency = 620;
    pAttr->Signal[3].LowEnergyThreshold             = 35;	    /* -dbm */
    pAttr->Signal[3].HighEnergyThreshold            = 0;	    /* dbm  */
    pAttr->Signal[3].Type							= CALL_PROGRESS_SIGNAL_TYPE__CADENCE;
    pAttr->Signal[3].AmFactor                       = 0;
    pAttr->Signal[3].TwistThreshold                 = 10;
    pAttr->Signal[3].ToneALevel                     = 24;
    pAttr->Signal[3].ToneBLevel                     = 24;
    pAttr->Signal[3].DetectionTimeOrCadenceFirstOnOrBurstDuration  = 50;
    pAttr->Signal[3].CadenceFirstOffDuration        = 50;
    pAttr->Signal[3].SignalToNoiseRatioThreshold    = 15;
    pAttr->Signal[3].FrequencyDeviationThreshold    = 10;
	            /* Reorder (ANSI TI.401-1993) */
    pAttr->Signal[4].ToneAFrequency                 = 480;
    pAttr->Signal[4].ToneB_OrAmpModulationFrequency = 620;
    pAttr->Signal[4].LowEnergyThreshold             = 35;	    /* -dbm */
    pAttr->Signal[4].HighEnergyThreshold            = 0;	    /* dbm  */
    pAttr->Signal[4].Type							= CALL_PROGRESS_SIGNAL_TYPE__CADENCE;
    pAttr->Signal[4].AmFactor                       = 0;
    pAttr->Signal[4].TwistThreshold                 = 10;
    pAttr->Signal[4].ToneALevel                     = 24;

    pAttr->Signal[4].ToneBLevel                     = 24;
    pAttr->Signal[4].DetectionTimeOrCadenceFirstOnOrBurstDuration  = 25;
    pAttr->Signal[4].CadenceFirstOffDuration        = 25;
    pAttr->Signal[4].SignalToNoiseRatioThreshold    = 15;
    pAttr->Signal[4].FrequencyDeviationThreshold    = 10;
	            /* Audible Ringer Tone */
    pAttr->Signal[5].ToneAFrequency                 = 400;
    pAttr->Signal[5].ToneB_OrAmpModulationFrequency = 0;
    pAttr->Signal[5].LowEnergyThreshold             = 35;	    /* -dbm */
    pAttr->Signal[5].HighEnergyThreshold            = 0;	    /* dbm  */
    pAttr->Signal[5].Type							= CALL_PROGRESS_SIGNAL_TYPE__CADENCE;
    pAttr->Signal[5].AmFactor                       = 0;
    pAttr->Signal[5].TwistThreshold                 = 10;
    pAttr->Signal[5].ToneALevel                     = 16;
    pAttr->Signal[5].ToneBLevel                     = 0;
    pAttr->Signal[5].DetectionTimeOrCadenceFirstOnOrBurstDuration  = 100;
    pAttr->Signal[5].CadenceFirstOffDuration        = 300;
    pAttr->Signal[5].SignalToNoiseRatioThreshold    = 15;
    pAttr->Signal[5].FrequencyDeviationThreshold    = 10;
	            /* Dial Tone */
    pAttr->Signal[6].ToneAFrequency                 = 440;
    pAttr->Signal[6].ToneB_OrAmpModulationFrequency = 0;
    pAttr->Signal[6].LowEnergyThreshold             = 35;	    /* -dbm */
    pAttr->Signal[6].HighEnergyThreshold            = 0;	    /* dbm  */
    pAttr->Signal[6].Type							= CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS;
    pAttr->Signal[6].AmFactor                       = 0;
    pAttr->Signal[6].TwistThreshold                 = 10;
    pAttr->Signal[6].ToneALevel                     = 10;
    pAttr->Signal[6].ToneBLevel                     = 0;
    pAttr->Signal[6].SignalToNoiseRatioThreshold    = 15;
    pAttr->Signal[6].FrequencyDeviationThreshold    = 10;
	pAttr->Signal[6].DetectionTimeOrCadenceFirstOnOrBurstDuration = 300; /* 3 sec */ 
	            /* Fast Busy Tone(T1.401-1993) */
    pAttr->Signal[7].ToneAFrequency                 = 440;
    pAttr->Signal[7].ToneB_OrAmpModulationFrequency = 0;
    pAttr->Signal[7].LowEnergyThreshold             = 35;	    /* -dbm */
    pAttr->Signal[7].HighEnergyThreshold            = 0;	    /* dbm  */
    pAttr->Signal[7].Type							= CALL_PROGRESS_SIGNAL_TYPE__CADENCE;
    pAttr->Signal[7].AmFactor                       = 0;
    pAttr->Signal[7].TwistThreshold                 = 10;
    pAttr->Signal[7].ToneALevel                     = 24;
    pAttr->Signal[7].ToneBLevel                     = 0;
    pAttr->Signal[7].DetectionTimeOrCadenceFirstOnOrBurstDuration  = 24;
    pAttr->Signal[7].CadenceFirstOffDuration        = 24;
    pAttr->Signal[7].SignalToNoiseRatioThreshold    = 15;
    pAttr->Signal[7].FrequencyDeviationThreshold    = 10;
	            /* Busy Tone */
    pAttr->Signal[8].ToneAFrequency                 = 400;
    pAttr->Signal[8].ToneB_OrAmpModulationFrequency = 0;
    pAttr->Signal[8].LowEnergyThreshold             = 35;	    /* -dbm */
    pAttr->Signal[8].HighEnergyThreshold            = 0;	    /* dbm  */
    pAttr->Signal[8].Type							= CALL_PROGRESS_SIGNAL_TYPE__CADENCE;
    pAttr->Signal[8].AmFactor                       = 0;
    pAttr->Signal[8].TwistThreshold                 = 10;
    pAttr->Signal[8].ToneALevel                     = 24;
    pAttr->Signal[8].ToneBLevel                     = 0;
    pAttr->Signal[8].DetectionTimeOrCadenceFirstOnOrBurstDuration  = 50;
    pAttr->Signal[8].CadenceFirstOffDuration        = 50;
    pAttr->Signal[8].SignalToNoiseRatioThreshold    = 15;
    pAttr->Signal[8].FrequencyDeviationThreshold    = 10;
}

/****************************************************************/
void Ac49xSetDefaultSetupDeviceAttr(Tac49xSetupDeviceAttr *pSetupDeviceAttr)
{
    memset((void*)pSetupDeviceAttr, 0, sizeof(Tac49xSetupDeviceAttr));
	pSetupDeviceAttr->OpenDeviceFormat = FORMAT__0;

	Ac49xSetDefaultDeviceControlRegisterAttr				(&pSetupDeviceAttr->DeviceControlRegister);
	Ac49xSetDefaultOpenDeviceConfiguration_Format0Attr		(&pSetupDeviceAttr->Open);
	Ac49xSetDefaultOpenDeviceConfiguration_Format1Attr		(&pSetupDeviceAttr->OpenFormat1);
	Ac49xSetDefaultAgcDeviceConfigurationAttr				(&pSetupDeviceAttr->Agc);
	Ac49xSetDefaultExtendedDeviceConfigurationAttr			(&pSetupDeviceAttr->Extended);
	Ac49xSetDefaultUserDefinedTonesDeviceConfigurationAttr	(&pSetupDeviceAttr->UserDefinedTones);
	Ac49xSetDefaultCallProgressDeviceConfigurationAttr		(&pSetupDeviceAttr->CallProgress);
}
 
/****************************************************************/
void Ac49xSetDefaultChannelConfigurationAttr(Tac49xOpenOrUpdateChannelConfigurationAttr *pAttr, Tac49xMediaChannelType ChannelType)
{
	memset((void*)pAttr, 0, sizeof(Tac49xOpenOrUpdateChannelConfigurationAttr));

	switch (ChannelType)
		{
		case MEDIA_CHANNEL_TYPE__REGULAR:
			pAttr->ChannelType                     = CHANNEL_TYPE__VOICE_FAX;                                   
			pAttr->EchoCancelerNonLinearProcessor  = ECHO_CANCELER_NON_LINEAR_PROCESSOR_MODE__MEDIUM_SENSITIVITY;
			pAttr->PcmLimiter					   = PCM_LIMITER__DISABLE;
			pAttr->HybridLoss					   = HYBRID_LOSS__6_DB;
			pAttr->EchoCancelerMode                = ECHO_CANCELER_MODE__ENABLE;                              
			pAttr->PcmSource                       = PCM_SOURCE__FROM_SERIAL_PORT;                                     
			pAttr->PacketToPacketMode			   = PACKET_TO_PACKET_MODE__SYNCHRONOUS;
			pAttr->InputGain                       = VOICE_GAIN__0_DB;                              
			pAttr->VoiceOutputGain                 = VOICE_GAIN__0_DB;                              
			pAttr->CasRelay                        = CONTROL__DISABLE;                                     
			pAttr->DynamicJitterBufferOptimizationFactor = DYNAMIC_JITTER_BUFFER_OPTIMIZATION_FACTOR__7;                                     
			pAttr->CallProgressTransferMode        = IBS_TRANSFER_MODE__TRANSPARENT_THROUGH_VOICE;
			pAttr->DetectPcmCas					   = CONTROL__ENABLE;
			pAttr->DetectNetCasPackets			   = CONTROL__DISABLE;
			pAttr->DetectNetIbsPackets			   = CONTROL__DISABLE;
			pAttr->IgnoreNetCasPackets			   = CONTROL__DISABLE;
			pAttr->IgnoreNetIbsPackets			   = CONTROL__DISABLE;
			pAttr->MfTransferMode                  = IBS_TRANSFER_MODE__TRANSPARENT_THROUGH_VOICE;
			pAttr->DtmfTransferMode                = IBS_TRANSFER_MODE__TRANSPARENT_THROUGH_VOICE;
			pAttr->DtmfErasureMode				   = DTMF_ERASURE_MODE__ERASE_1_COMPONENT;
			pAttr->TestFaxModem                    = CONTROL__DISABLE;                       
			pAttr->IbsDetectionRedirection         = IBS_DETECTION_REDIRECTION__ON_INPUT_STREAM_ENCODER_INPUT;
			pAttr->DetectUserDefinedTones		   = CONTROL__DISABLE;                       
			pAttr->DetectCallProgress			   = CONTROL__DISABLE;                       
			pAttr->DetectDTMFRegister			   = CONTROL__ENABLE;                       
			pAttr->DetectMFR2BackwardRegister	   = CONTROL__DISABLE;                       
			pAttr->DetectMFR2ForwardRegister	   = CONTROL__DISABLE;                       
			pAttr->DetectMFSS5Register			   = CONTROL__DISABLE;                       
			pAttr->DetectMFR1Register			   = CONTROL__DISABLE;                       
			pAttr->CallerIdOnHookService           = CONTROL__DISABLE;  
			pAttr->CallerIdOffHookService          = CONTROL__DISABLE;  
			pAttr->TestMode                        = TEST_MODE__NONE;                       
			pAttr->IbsLevel_minus_dbm              = 9;                       
			pAttr->UdpChecksumEnable               = CONTROL__DISABLE;                       
			pAttr->FaxMaxRate                      = MAX_RATE__14_4_KBPS;
			pAttr->LongDtmfDetectionEnable		   = CONTROL__DISABLE;
			pAttr->EchoCancelerLength              = ECHO_CANCELER_LENGTH__64_MSEC;                       
			pAttr->TriStateDuringIdle              = CONTROL__DISABLE;                       
			pAttr->CasModeDuringIdle               = CAS_MODE_DURING_IDLE__IS_CAS_VALUE_DURING_IDLE;                       
			pAttr->CasValueDuringIdle              = 0;                       
			pAttr->PcmValueDuringIdle              = 0xFF;                       
			pAttr->VoiceJitterBufferMinDelay       = 35;
			pAttr->VoiceJitterBufferMaxDelay       = 150;
			pAttr->FaxAndCallerIdLevel             = FAX_AND_CALLER_ID_LEVEL__minus_12_DBM;
			pAttr->TtyTransferMethod			   = TTY_TRANSFER_METHOD__DISABLE;
            		pAttr->V34FaxTransferMethod            = FAX_MODEM_TRANSFER_METHOD__DISABLE;
			pAttr->V34TransferMethod               = FAX_MODEM_TRANSFER_METHOD__DISABLE;
			pAttr->V32TransferMethod               = FAX_MODEM_TRANSFER_METHOD__DISABLE;
			pAttr->V23TransferMethod               = FAX_MODEM_TRANSFER_METHOD__DISABLE;
			pAttr->V22TransferMethod               = FAX_MODEM_TRANSFER_METHOD__DISABLE;
			pAttr->V21TransferMethod               = FAX_MODEM_TRANSFER_METHOD__DISABLE;
			pAttr->Bell103TransferMethod		   = FAX_MODEM_TRANSFER_METHOD__DISABLE;
			pAttr->FaxTransferMethod               = FAX_MODEM_TRANSFER_METHOD__DISABLE;
			pAttr->FaxProtocol					   = FAX_PROTOCOL__T38_UDP_DATA_RATE_MANAGEMENT_2;
			pAttr->CallerIdStandard                = CALLER_ID_STANDARD__TELCORDIA_BELLCORE;
			pAttr->FaxErrorCorrectionMode          = CONTROL__ENABLE;                       
			pAttr->CngRelayEnable	               = CONTROL__DISABLE;                       
			pAttr->CedTransferMode				   = CED_TRANSFER_MODE__BY_FAX_RELAY;
			pAttr->AgcSlowAdaptationGainSlope	   = AGC_GAIN_SLOPE__1_00_DB_SEC;
			pAttr->AgcLocation					   = AGC_LOCATION__AT_ENCODER_INPUT;
			pAttr->AgcEnable					   = CONTROL__DISABLE;
			pAttr->AgcTargetEnergy				   = AGC_TARGET_ENERGY__minus19_DBM;
			pAttr->FaxAndDataStatusAndEventsDisable= CONTROL__DISABLE;
			pAttr->PcmSidSize					   = PCM_SID_SIZE__1_BYTE;
			pAttr->EchoCancelerDebugPackets        = CONTROL__DISABLE;
            		pAttr->VideoSynchronizationEnable      = CONTROL__DISABLE;
			pAttr->ActiveChannelPattern			   = ACTIVE_CHANNEL_PATTERN__PCM_VALUE_IS_MUTE;
			pAttr->AnswerDetectEnable			   = CONTROL__DISABLE;
			pAttr->AnswerDetectDirection		   = ANSWER_DETECT_DIRECTION__ENCODER_INPUT;
			pAttr->AnswerDetectMode				   = ANSWER_DETECT_MODE__VAD;
			pAttr->AnswerDetectSensitivity		   = 3;
			break;
		case MEDIA_CHANNEL_TYPE__MEDIATED:
			pAttr->DtmfTransferMode							= IBS_TRANSFER_MODE__TRANSPARENT_THROUGH_VOICE;
			pAttr->DynamicJitterBufferOptimizationFactor	= DYNAMIC_JITTER_BUFFER_OPTIMIZATION_FACTOR__7;
			pAttr->VoiceJitterBufferMinDelay				= 35;
			pAttr->VoiceJitterBufferMaxDelay				= 150;
			pAttr->PcmSidSize					            = PCM_SID_SIZE__1_BYTE;
            		pAttr->VideoSynchronizationEnable               = CONTROL__DISABLE;
			pAttr->PacketToPacketMode						= PACKET_TO_PACKET_MODE__SYNCHRONOUS;
			break;
    		default:;
	}
}

/****************************************************************/
void Ac49xSetDefaultAdvancedChannelConfigurationAttr(Tac49xAdvancedChannelConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xAdvancedChannelConfigurationAttr));
    pAttr->EncoderIbsOverhang_10msec			= 10;
    pAttr->DecoderIbsOverhang_10msec			= 100;
    pAttr->MuteEncoderDuringDtmfGeneration		= CONTROL__DISABLE;
    pAttr->SignalingJitter						= 35;
    pAttr->FaxModemJitter						= 0;
    pAttr->BypassJitter							= 35;
    pAttr->EchoCancelerDuringDataBypass			= CONTROL__DISABLE;
    pAttr->EchoCancelerDuringFaxBypassDisable	= CONTROL__DISABLE;
    pAttr->EchoCancelerNlpDuringBypass			= CONTROL__DISABLE;
    pAttr->EchoCancelerDcRemover				= ECHO_CANCELLER_HIGH_PASS_FILTER__DISABLED;
    pAttr->EchoCancelerThreshold				= ECHO_CANCELER_THRESHOLD__minus49_DBM;
	pAttr->EchoCancellerAutomaticToneResetMode	= CONTROL__ENABLE;
	pAttr->EchoCancellerAutomaticToneResetMode	= CONTROL__DISABLE;
    pAttr->EchoCancelerNlpThreshold				= ECHO_CANCELER_NLP_THRESHOLD__0_DBM;/*Not used for ver 105 and up*/
    pAttr->EchoCancelerCng						= CONTROL__ENABLE;
    pAttr->EchoCancelerToneDetector				= CONTROL__ENABLE;
	pAttr->EchoCancelerAggressiveNlp			= CONTROL__ENABLE;
	pAttr->EchoCancelerNoisyEnvironmentMode	= CONTROL__DISABLE;
	pAttr->EchoCancelerReconvergenceSupport	= CONTROL__ENABLE;
	pAttr->AcousticEchoCancelerEnable			= CONTROL__DISABLE;
    pAttr->HdlcInterFrameFillPattern			= HDLC_INTER_FRAME_FILL_PATTERN__7E;
    pAttr->HdlcMinimumMessageLength				= HDLC_MININUM_MESSAGE_LENGTH__3_BYTE;
    pAttr->HdlcMinimumEndFlags					= HDLC_MININUM_END_FLAGS__1;
    pAttr->HdlcSampleRate						= HDLC_SAMPLE_RATE__64_KBPS;
    pAttr->PlaybackWaterMark_msec				= 80;
    pAttr->PlaybackRequestEnable				= CONTROL__ENABLE;
	pAttr->Ss7LinkRate							= SS7_LINK_RATE__64_KBPS;
	pAttr->HdlcTransferMedium					= TRANSFER_MEDIUM__HOST;									
	pAttr->Ss7TransferMedium					= TRANSFER_MEDIUM__HOST;
	pAttr->EchoCancelerAutomaticHandler			= CONTROL__DISABLE;
	pAttr->PcmCngThreshold						= PCM_CNG_THRESHOLD__minus20dBm_Default;
	pAttr->InputGainLocation					= INPUT_GAIN_LOCATION__BEFORE_ECHO_CANCELER;
	pAttr->DataBypassOutputGain					= FAX_AND_DATA_GAIN__DEFAULT_0_DB;
	pAttr->FaxBypassOutputGain					= FAX_AND_DATA_GAIN__DEFAULT_0_DB;
    pAttr->DuaReceiveTimeout_x10msec			= 60;
    pAttr->DuaFrameMode							= DUA_FRAMER_MODE__DPNSS;
	pAttr->AnswerDetectNoneSpeechEventsEnable	= CONTROL__ENABLE;
	pAttr->AnswerDetectSilenceTime_x100ms		= 10;
	pAttr->AnswerDetectPreWorkDelay_x100ms		= 0;
}

/****************************************************************/
void Ac49xSetDefaultAtmChannelConfigurationAttr(Tac49xActivateOrUpdateAtmChannelConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xActivateOrUpdateAtmChannelConfigurationAttr));
	pAttr->Protocol								= ATM_PROTOCOL__AAL2_I_366_2;
	pAttr->AtmAdaptionLayer2RxChannelId			= 16;
	pAttr->AtmAdaptionLayer2TxChannelId			= 16;
	pAttr->G711CoderType						= LAW_SELECT__A_LAW;
	pAttr->SilenceCompressionMode				= SILENCE_COMPRESSION_MODE__ENABLE_NOISE_ADAPTATION_ENABLE;
	pAttr->ProfileGroup							= PROFILE_GROUP__ITU;
	pAttr->ProfileEntry							= 0;
	pAttr->ProfileId							= 0;
	pAttr->BrokenConnectionActivationMode		= BROKEN_CONNECTION_ACTIVATION_MODE__ACTIVATE_AFTER_FIRST_RTP_PCKAET_ARRIVAL;
	pAttr->BrokenConnectionEventTimeout_x20msec	= 15; /* That is 300 msec */
}

/****************************************************************/
void Ac49xSetDefaultRtpChannelConfigurationAttr(Tac49xActivateOrUpdateRtpChannelConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xActivateOrUpdateRtpChannelConfigurationAttr));
	pAttr->AmrActiveMode							= AMR_ACTIVE_MODE__INACTIVE;
    pAttr->RtpRedundancy							= CONTROL__DISABLE;
    pAttr->IbsRedundancyLevel						= IBS_REDUNDANCY_LEVEL__REPETITIONS_NONE;
    pAttr->DisableRtcpIntervalRandomization			= CONTROL__DISABLE;       
	pAttr->AdpcmPayloadFormat						= RTP_ADPCM_PAYLOAD_FORMAT__LITTLE_ENDIAN;
    pAttr->MediaProtectionMode  				    = MEDIA_PROTECTION_MODE__DISABLE;
    pAttr->Encoder									= CODER__G711ALAW;
    pAttr->Decoder									= CODER__G711ALAW;
    pAttr->BypassCoder								= CODER__G711ALAW;
    pAttr->RxPayloadTypeSource						= PAYLOAD_TYPE_SOURCE__DEFINED_INTERNALLY_ACCORDING_THE_ENCODER;
    pAttr->TxPayloadTypeSource						= PAYLOAD_TYPE_SOURCE__DEFINED_INTERNALLY_ACCORDING_THE_ENCODER;
    pAttr->RxPayloadType							= 8;
    pAttr->TxPayloadType							= 8;
    pAttr->FaxBypassPayloadType						= 100;
    pAttr->DataBypassPayloadType					= 101;
    pAttr->TxSignalingRfc2833PayloadType			= 96;
    pAttr->RxSignalingRfc2833PayloadType			= 96;
	pAttr->Rfc2198PayloadType						= 104;
    pAttr->PcmAdPcmComfortNoiseRtpPayloadType		= 13;
    pAttr->PcmAdPcmComfortNoiseRtpPayloadTypeEnable	= CONTROL__ENABLE;
    pAttr->RtcpMeanTxInterval						= 5000;
	pAttr->VbdEventsRelayType						= VBD_EVENTS_RELAY_TYPE__NONE;
#if (AC49X_DRIVRES_VERSION>=AC49X_DRIVER_VERSION_580)
	pAttr->TxVbdEventsRelayPayloadType	  			= 105;
	pAttr->RxVbdEventsRelayPayloadType	  			= 105;
#else
	pAttr->VbdEventsRelayPayloadType	  			= 105;
#endif
	pAttr->NumberOfVoiceBlocks_minus_1	  			= 0;
	pAttr->NumberOfBypassBlocks_minus_1  			= 0;
	pAttr->DecoderPcmAdpcmFrameDuration  			= PCM_ADPCM_FRAME_DURATION__20_MSEC;
	pAttr->EncoderPcmAdpcmFrameDuration  			= PCM_ADPCM_FRAME_DURATION__20_MSEC;
	pAttr->SilenceCompressionMode		  			= SILENCE_COMPRESSION_MODE__DISABLE;
	pAttr->BrokenConnectionActivationMode			= BROKEN_CONNECTION_ACTIVATION_MODE__ACTIVATE_AFTER_FIRST_RTP_PCKAET_ARRIVAL;
	pAttr->BrokenConnectionEventTimeout_x20msec		= 500; /* That is 300 msec */
	pAttr->RxT38PayloadType							= 106;
	pAttr->TxT38PayloadType							= 106;
	pAttr->NoOperationEnable						= CONTROL__DISABLE;
	pAttr->NoOperationPayloadType					= 120;
	pAttr->NoOperationTimeout_x20msec				= 500;
	pAttr->BypassRedundancyType						= BYPASS_REDUNDANCY_TYPE__NONE;
	pAttr->TxFilterRtp						        = CONTROL__DISABLE;
	pAttr->TxFilterRfc2833						    = CONTROL__DISABLE;
	pAttr->TxFilterRtcp						        = CONTROL__DISABLE;
	pAttr->RxFilterRtp						        = CONTROL__DISABLE;
	pAttr->RxFilterRfc2833						    = CONTROL__DISABLE;
	pAttr->RxFilterRtcp  						    = CONTROL__DISABLE;
	pAttr->PacketToPacketSequenceNumberMode			= PACKET_TO_PACKET_SEQUENCE_NUMBER_MODE__SEQUENTIAL;
	pAttr->PacketToPacketSsrcMode					= PACKET_TO_PACKET_SSRC_MODE__CONFIGURABLE;
	pAttr->PacketToPacketNoTranscodingPayloadType	= 0;
	pAttr->BlockNonePayloadType13					= BLOCK_NONE_PAYLOAD_TYPE_13__IGNORE_PACKET;
	pAttr->ReportErrorOnReceivedStreamEnable		= 0;
	pAttr->IbsRelayTransmissionTimeout				= 0;
	pAttr->RtcpXrEventsMode							= RTCP_XR_EVENTS_MODE__DISABLE_VOICE_QUALITY_STATISTICS;
	pAttr->RtcpXrMode								= RTCP_XR_MODE__DISABLED;
	pAttr->RtpSequenceNumberDetectionEnable			= CONTROL__DISABLE;
	pAttr->SequenceNumberForDetection				= 0;
	pAttr->AnswerToneOverRfc2833Enable				= CONTROL__DISABLE;
	pAttr->ModemRelayHighAvailabilityOptions	  	= MODEM_RELAY_HIGH_AVAILABILITY_OPTIONS__DISABLE;
	pAttr->SseRedundancyDepth	  					= 3;
}

/****************************************************************/
#if (AC49X_DRIVRES_VERSION>=AC49X_DRIVER_VERSION_105)
void Ac49xSetDefaultCloseRtpChannelConfigurationAttr(Tac49xCloseRtpChannelConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xCloseRtpChannelConfigurationAttr));
	pAttr->SendRtcpByePacket = CONTROL__DISABLE;
}
#endif

/****************************************************************/
void Ac49xSetDefault3GppIubChannelConfigurationAttr(Tac49xActivateOrUpdate3GppIubChannelConfigurationAttr *pAttr)
{
    int i;
    
    pAttr->Crc                                  = CONTROL__ENABLE;
    pAttr->SilenceCompression                   = CONTROL__DISABLE;
    pAttr->TransmitNoDataPacketsDuringSilence   = CONTROL__ENABLE;
    pAttr->LocalRate                            = _3GPP_RATE__12_2_KBPS;
    pAttr->BrokenConnectionActivationMode       = BROKEN_CONNECTION_ACTIVATION_MODE__ACTIVATE_AFTER_FIRST_RTP_PCKAET_ARRIVAL;
    pAttr->BrokenConnectionEventTimeout_x20msec = 500;

    for(i=0; i<MAX_NUMBER_OF_3GPP_IUB_TFIS; i++)
        {
        pAttr->Tfi[i].Tfi0  = i+2;
        pAttr->Tfi[i].Tfi1  = 0;
        pAttr->Tfi[i].Tfi2  = 0;
        pAttr->Tfi[i].Valid = 0;
        }
    pAttr->Tfi[_3GPP_RATE__12_2_KBPS].Valid = 1;
}

/****************************************************************/
void Ac49xSetDefaultT38ConfigurationAttr(Tac49xT38ConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xT38ConfigurationAttr));
	pAttr->PacketizationPeriod				= T38_PACKETIZATION_PERIOD__40_msec;
    pAttr->RtpEncapsulation					= CONTROL__DISABLE;
    pAttr->JbigImageTranscoding				= CONTROL__DISABLE;
    pAttr->T4MmrTranscoding					= CONTROL__DISABLE;
	pAttr->T4FillBitRemoval					= CONTROL__DISABLE;
	pAttr->MaxDatagramSize_In4BytesStep		= 0;
	pAttr->T38Version						= T38_VERSION__0;
	pAttr->UdpErrorCorrectionMethod			= T38_UDP_ERROR_CORRECTION_METHOD__REDUNDANCY;
	pAttr->ImageDataRedundancyLevel			= IMAGE_DATA_REDUNDANCY_LEVEL__NONE;
	pAttr->T30ControlDataRedundancyLevel	= T30_CONTROL_DATA_REDUNDANCY_LEVEL__NONE;
	pAttr->NoOperationPeriod				= T38_NO_OPERATION_PERIOD__HANDLER_DISABLE;
}

/****************************************************************/
void Ac49xSetDefault3WayConferenceConfigurationAttr(Tac49xActivateOrDeactivate3WayConferenceConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xActivateOrDeactivate3WayConferenceConfigurationAttr));
}

/****************************************************************/
void Ac49xSetDefault3GppUserPlaneInitializationConfigurationAttr(Tac49x3GppUserPlaneInitializationConfigurationAttr *pAttr, Tac49xMediaChannelType ChannelType)
{
	memset((void*)pAttr, 0, sizeof(Tac49x3GppUserPlaneInitializationConfigurationAttr));
	

	switch (ChannelType)
		{
		case MEDIA_CHANNEL_TYPE__REGULAR:
			pAttr->Mode											= _3GPP_MODE__SUPPORT;
			pAttr->SupportModeType								= _3GPP_SUPPORT_MODE_TYPE__SLAVE;
			pAttr->DeliveryOfErroneousSDUs						= CONTROL__ENABLE;
			pAttr->ProtocolDataUnitType							= _3GPP_PROTOCOL_DATA_UNIT_TYPE__TYPE_0;
			pAttr->ErrorTransmissionDisable						= CONTROL__DISABLE;
			pAttr->SupporetedUserPlaneProtocolVersion_Release99	= CONTROL__ENABLE;			
			pAttr->SupporetedUserPlaneProtocolVersion_Release4	= CONTROL__ENABLE;			
			pAttr->SupporetedUserPlaneProtocolVersion_Release5	= CONTROL__DISABLE;			
			pAttr->RateControlAcknowledgeTimeout				= 25;			
			pAttr->InitializationAcknowledgeTimeout				= 25;			
			pAttr->LocalRate									= _3GPP_RATE__12_2_KBPS;

			pAttr->RemoteRate									= _3GPP_RATE__12_2_KBPS;
			pAttr->G711CoderType								= G711_CODER_TYPE__A_LAW;
			pAttr->Rfci[_3GPP_RFCI__AMR_4_75_KBPS].Valid		= CONTROL__ENABLE;
			pAttr->Rfci[_3GPP_RFCI__AMR_5_15_KBPS].Valid		= CONTROL__ENABLE;
			pAttr->Rfci[_3GPP_RFCI__AMR_5_9_KBPS ].Valid		= CONTROL__ENABLE;
			pAttr->Rfci[_3GPP_RFCI__AMR_6_7_KBPS ].Valid		= CONTROL__ENABLE;
			pAttr->Rfci[_3GPP_RFCI__AMR_7_4_KBPS ].Valid		= CONTROL__ENABLE;
			pAttr->Rfci[_3GPP_RFCI__AMR_7_95_KBPS].Valid		= CONTROL__ENABLE;
			pAttr->Rfci[_3GPP_RFCI__AMR_10_2_KBPS].Valid		= CONTROL__ENABLE;
			pAttr->Rfci[_3GPP_RFCI__AMR_12_2_KBPS].Valid		= CONTROL__ENABLE;
			pAttr->Rfci[_3GPP_RFCI__AMR_SID		 ].Valid		= CONTROL__DISABLE;
			pAttr->Rfci[_3GPP_RFCI__GSM_EFR_SID	 ].Valid		= CONTROL__DISABLE;
			pAttr->Rfci[_3GPP_RFCI__TDMA_EFR_SID ].Valid		= CONTROL__DISABLE;
			pAttr->Rfci[_3GPP_RFCI__PDC_EFR_SID	 ].Valid		= CONTROL__DISABLE;
			pAttr->Rfci[_3GPP_RFCI__NO_DATA		 ].Valid		= CONTROL__ENABLE;
			pAttr->Rfci[_3GPP_RFCI__G711_64_KBPS ].Valid		= CONTROL__ENABLE;
			break;
		case MEDIA_CHANNEL_TYPE__MEDIATED:
			pAttr->Mode											= _3GPP_MODE__SUPPORT;
			pAttr->SupportModeType								= _3GPP_SUPPORT_MODE_TYPE__SLAVE;
			pAttr->DeliveryOfErroneousSDUs						= CONTROL__ENABLE;
			pAttr->ProtocolDataUnitType							= _3GPP_PROTOCOL_DATA_UNIT_TYPE__TYPE_0;
			pAttr->ErrorTransmissionDisable						= CONTROL__DISABLE;
			pAttr->SupporetedUserPlaneProtocolVersion_Release99	= CONTROL__ENABLE;			
			pAttr->SupporetedUserPlaneProtocolVersion_Release4	= CONTROL__ENABLE;			
			pAttr->SupporetedUserPlaneProtocolVersion_Release5	= CONTROL__DISABLE;			
			pAttr->RateControlAcknowledgeTimeout				= 25;			
			pAttr->InitializationAcknowledgeTimeout				= 25;			
			pAttr->LocalRate									= _3GPP_RATE__G711_64_KBPS;
			pAttr->RemoteRate									= _3GPP_RATE__G711_64_KBPS;
			pAttr->G711CoderType								= G711_CODER_TYPE__A_LAW;
			pAttr->Rfci[_3GPP_RFCI__AMR_4_75_KBPS	].Valid		= CONTROL__DISABLE;
			pAttr->Rfci[_3GPP_RFCI__AMR_5_15_KBPS	].Valid		= CONTROL__DISABLE;
			pAttr->Rfci[_3GPP_RFCI__AMR_5_9_KBPS	].Valid		= CONTROL__DISABLE;
			pAttr->Rfci[_3GPP_RFCI__AMR_6_7_KBPS	].Valid		= CONTROL__DISABLE;
			pAttr->Rfci[_3GPP_RFCI__AMR_7_4_KBPS	].Valid		= CONTROL__DISABLE;
			pAttr->Rfci[_3GPP_RFCI__AMR_7_95_KBPS	].Valid		= CONTROL__DISABLE;
			pAttr->Rfci[_3GPP_RFCI__AMR_10_2_KBPS	].Valid		= CONTROL__DISABLE;
			pAttr->Rfci[_3GPP_RFCI__AMR_12_2_KBPS	].Valid		= CONTROL__DISABLE;
			pAttr->Rfci[_3GPP_RFCI__AMR_SID			].Valid		= CONTROL__DISABLE;
			pAttr->Rfci[_3GPP_RFCI__GSM_EFR_SID		].Valid		= CONTROL__DISABLE;
			pAttr->Rfci[_3GPP_RFCI__TDMA_EFR_SID	].Valid		= CONTROL__DISABLE;
			pAttr->Rfci[_3GPP_RFCI__PDC_EFR_SID		].Valid		= CONTROL__DISABLE;
			pAttr->Rfci[_3GPP_RFCI__NO_DATA			].Valid		= CONTROL__ENABLE;
			pAttr->Rfci[_3GPP_RFCI__G711_64_KBPS	].Valid		= CONTROL__ENABLE;
			break;
    default:;
		}

	pAttr->Rfci[_3GPP_RFCI__AMR_4_75_KBPS	].FieldOfRate	= 0;
	pAttr->Rfci[_3GPP_RFCI__AMR_5_15_KBPS	].FieldOfRate	= 1;
	pAttr->Rfci[_3GPP_RFCI__AMR_5_9_KBPS	].FieldOfRate	= 2;
	pAttr->Rfci[_3GPP_RFCI__AMR_6_7_KBPS	].FieldOfRate	= 3;
	pAttr->Rfci[_3GPP_RFCI__AMR_7_4_KBPS	].FieldOfRate	= 4;
	pAttr->Rfci[_3GPP_RFCI__AMR_7_95_KBPS	].FieldOfRate	= 5;
	pAttr->Rfci[_3GPP_RFCI__AMR_10_2_KBPS	].FieldOfRate	= 6;
	pAttr->Rfci[_3GPP_RFCI__AMR_12_2_KBPS	].FieldOfRate	= 7;
	pAttr->Rfci[_3GPP_RFCI__AMR_SID			].FieldOfRate	= 8;
	pAttr->Rfci[_3GPP_RFCI__GSM_EFR_SID		].FieldOfRate	= 9;
	pAttr->Rfci[_3GPP_RFCI__TDMA_EFR_SID	].FieldOfRate	= 10;
	pAttr->Rfci[_3GPP_RFCI__PDC_EFR_SID		].FieldOfRate	= 11;
	pAttr->Rfci[_3GPP_RFCI__NO_DATA			].FieldOfRate	= 15;
	pAttr->Rfci[_3GPP_RFCI__G711_64_KBPS	].FieldOfRate	= 16;
}

/****************************************************************/
void Ac49xSetDefaultRfc3267AmrInitializationConfigurationAttr(Tac49xRfc3267AmrInitializationConfigurationAttr *pAttr)
{
    int i;
	memset((void*)pAttr, 0, sizeof(Tac49xRfc3267AmrInitializationConfigurationAttr));
	pAttr->OctetAlignedEnable               = CONTROL__ENABLE;
    pAttr->ChangeModeNeighbor               = CHANGE_MODE_NEIGHBOR__CHANGED_TO_ANY_RATE;
    pAttr->CrcEnable                        = CONTROL__DISABLE;
    pAttr->ModeChangePeriod                 = 0;
    pAttr->LocalRate                        = RFC3267_AMR_RATE__12_2_KBPS;
    pAttr->RemoteRateEnable                 = CONTROL__DISABLE;
    pAttr->RemoteRate                       = RFC3267_AMR_RATE__12_2_KBPS;
	pAttr->Coder			                = RFC3267_CODER__AMR;
    pAttr->RedundancyLevel                  = RFC3267_REDUNDANCY_LEVEL__NONE;
    pAttr->AutomaticRateControlMode         = RFC3267_AUTOMATIC_RATE_CONTROL_MODE__LOCAL_RATE_CHANGED_ACCORDING_THE_CMR_OF_REMOTE_SIDE;
    pAttr->TransmitNoDataPackets            = CONTROL__DISABLE;
    pAttr->NumberOfCodecsPolicies           = 0;
    pAttr->DelayHandoutHysteresis_x1msec    = 100;
    pAttr->DelayHandoutThreshold_x1msec     = 0;

    for(i=0; i<MAX_NUMBER_OF_RFC_3267_CODECS; i++)
        {
        pAttr->aCodecPolicies[i].Rate                     = 0;
        pAttr->aCodecPolicies[i].RedundancyLevel          = RFC3267_REDUNDANCY_LEVEL__NONE;
        pAttr->aCodecPolicies[i].FrameLossRatioThreshold  = RFC3267_FRAME_LOSS_RATIO_THRESHOLD__0__PERCENTAGE;
        pAttr->aCodecPolicies[i].FrameLossRatioHysteresis = RFC3267_FRAME_LOSS_RATIO_HYSTERESIS__0__PERCENTAGE;
        }
}

/****************************************************************/
void Ac49xSetDefaultPacketCableProtectionConfigurationAttr(Tac49xPacketCableProtectionConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xPacketCableProtectionConfigurationAttr));
	pAttr->Direction								= MEDIA_PROTECTION_DIRECTION__INBOUND_RX;
	pAttr->RtpAuthentication						= CONTROL__ENABLE;
	pAttr->RtpEncryption							= CONTROL__ENABLE;
	pAttr->RtcpAuthentication						= CONTROL__ENABLE;
	pAttr->RtcpEncryption							= CONTROL__ENABLE;
	pAttr->AntiReplayEnable							= CONTROL__ENABLE;
	pAttr->NumberOfRtpTimeStampWrapAround			= 0;
	pAttr->RtpCipherKeySize							= MAX_KEY_SIZE__RTP_CIPHER;
	pAttr->RtcpCipherKeySize						= MAX_KEY_SIZE__RTCP_CIPHER;
	pAttr->RtpMessageAuthenticationCodeKeySize		= MAX_KEY_SIZE__RTP_MESSAGE_AUTHENTICATION_CODE;
	pAttr->RtcpMessageAuthenticationCodeKeySize		= MAX_KEY_SIZE__RTCP_MESSAGE_AUTHENTICATION_CODE;
	pAttr->RtpAuthenticationMode		   			= MEDIA_AUTHENTICATION_MODE__MMH4;
	pAttr->RtcpAuthenticationMode		   			= MEDIA_AUTHENTICATION_MODE__SHA1;
	pAttr->RtpDigestSize	  		   				= 4;
	pAttr->RtcpDigestSize	  		   				= 12;
	pAttr->RtpTimeStamp								= 0;
	memset((void*)&pAttr->RtpCipherKey,                      0, MAX_KEY_SIZE__RTP_CIPHER                         );
	memset((void*)&pAttr->RtcpCipherKey,                     0, MAX_KEY_SIZE__RTCP_CIPHER                        );
	memset((void*)&pAttr->RtpMessageAuthenticationCodeKey,   0, MAX_KEY_SIZE__RTP_MESSAGE_AUTHENTICATION_CODE    );
	memset((void*)&pAttr->RtcpMessageAuthenticationCodeKey,  0, MAX_KEY_SIZE__RTCP_MESSAGE_AUTHENTICATION_CODE   );

	memset((void*)&pAttr->RtpInitializationKey,              0, MAX_KEY_SIZE__RTP_CIPHER                         );
	pAttr->RtcpSequenceNumber						= 0;
}

/****************************************************************/
void Ac49xSetDefaultSrtpProtectionConfigurationAttr(Tac49xSrtpProtectionConfigurationAttr *pAttr, Tac49xMediaProtectionDirection Direction)
{
	memset((void*)pAttr, 0, sizeof(Tac49xSrtpProtectionConfigurationAttr));
	pAttr->Direction					        = Direction;
	pAttr->RtpEncryption			        	= CONTROL__ENABLE;
	pAttr->RtpAuthentication				    = CONTROL__ENABLE;
	pAttr->RtcpEncryption			        	= CONTROL__ENABLE;
	pAttr->RtcpAuthentication				    = CONTROL__ENABLE;
	pAttr->AntiReplayEnable				        = CONTROL__ENABLE;
	pAttr->KeyDerivationRatePower               = 0;
	pAttr->RtpMessageAuthenticationCodeKeySize  = 20;
	pAttr->RtpCipherKeySize              		= 16;
	pAttr->RtpAuthenticationMode		   		= MEDIA_AUTHENTICATION_MODE__SHA1;
	pAttr->RtpEncryptionMode		   			= MEDIA_EBCRYPTION_MODE__AES_CM;
	pAttr->RtpDigestSize    					= 10;
	pAttr->RtcpMessageAuthenticationCodeKeySize = 20;
	pAttr->RtcpCipherKeySize              		= 16;
	pAttr->RtcpAuthenticationMode		   		= MEDIA_AUTHENTICATION_MODE__SHA1;
	pAttr->RtcpEncryptionMode		   			= MEDIA_EBCRYPTION_MODE__AES_CM;
	pAttr->RtcpDigestSize    					= 10;
	pAttr->MasterKeySize	  		   	        = 16;
	pAttr->MasterSaltSize	  		   	        = 14;
	pAttr->SessionSaltSize	  		   	        = 14;
	pAttr->RtpInitSequenceNumber				= 0;
	pAttr->RtpInitRoc							= 0;
	pAttr->RtcpInitIndex						= 0;
	memset((void*)&pAttr->MasterKey,  0, MAX_KEY_SIZE__MASTER);
	memset((void*)&pAttr->MasterSalt, 0, MAX_KEY_SIZE__MASTER_SALT);
	pAttr->MasterKeyIndexSize					= 0; 
}

/****************************************************************/
#if (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
    )
void Ac49xSetDefaultCodecConfigurationAttr(Tac49xCodecConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xCodecConfigurationAttr));
	pAttr->AdcDitheringCircuit                  = CONTROL__ENABLE;   
    pAttr->BiasPinOutputVoltage                 = BIAS_PIN_OUTPUT_VOLTAGE__2v35;   
    pAttr->SpeakerGain                          = SPEAKER_GAIN__0dB;   
    pAttr->BandPassFilter                       = CONTROL__ENABLE;   
	pAttr->SamplingRate							= SAMPLING_RATE__8_KHz;
    pAttr->MuteHandset		                    = CONTROL__DISABLE;   
    pAttr->MuteHeadset		                    = CONTROL__DISABLE;   
    pAttr->MuteLineOutput		                = CONTROL__DISABLE;   
    pAttr->MuteSpeaker		                    = CONTROL__DISABLE;   
    pAttr->HeadsetInputToOutput	                = CONTROL__DISABLE;   
    pAttr->HandsetInputToOutput	                = CONTROL__DISABLE;   
    pAttr->CallerIdInputSelect	                = CONTROL__DISABLE;   
    pAttr->LineInputSelect		                = CONTROL__DISABLE;   
    pAttr->MicrophoneInputSelect                = CONTROL__DISABLE;   
    pAttr->HandsetInputSelect	                = CONTROL__DISABLE;   
    pAttr->HeadsetInputSelect	                = CONTROL__DISABLE;   
    pAttr->AnalogSidetoneGain	                = SIDETONE_GAIN__minusd12dB;   
    pAttr->DigitalSidetoneGain	                = SIDETONE_GAIN__MUTE;   
    pAttr->AnalogSidetoneOutputSelectForHeadset = CONTROL__DISABLE;   
    pAttr->AnalogSidetoneOutputSelectForHandset = CONTROL__DISABLE;   
    pAttr->SpeakerSelect		                = CONTROL__DISABLE;   
    pAttr->LineOutputSelect		                = CONTROL__DISABLE;   
    pAttr->HeadsetOutputSelect		            = CONTROL__DISABLE;   
    pAttr->HandsetOutputSelect		            = CONTROL__DISABLE;   
    pAttr->DacOutputPgaGain		                = CODEC_DAC_OUTPUT_PGA_GAIN__minus12dB;   
    pAttr->AdcInputPgaGain		                = CODEC_ADC_INPUT_PGA_GAIN__0dB;   
	pAttr->AnalogLoopback						= CONTROL__DISABLE;
	pAttr->DigitalLoopback						= CONTROL__DISABLE;
}

/****************************************************************/
void Ac49xSetDefaultAcousticEchoCancelerConfigurationAttr(Tac49xAcousticEchoCancelerConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xAcousticEchoCancelerConfigurationAttr));
	pAttr->SupportedMode			= ACOUSTIC_ECHO_CANCELER_SUPPORTED_MODE__SPEAKER_PHONE_ONLY;
	pAttr->SaturationAttenuator		= CONTROL__ENABLE;
	pAttr->ResidualOverwriteByInput	= CONTROL__ENABLE;
	pAttr->SoftNlp					= CONTROL__ENABLE;
	pAttr->Cng						= CONTROL__ENABLE;
	pAttr->ReinitAfterToneSignal	= CONTROL__ENABLE;
	pAttr->HalfDuplexMode			= ACOUSTIC_ECHO_CANCELER_HALF_DUPLEX_MODE__ON_START_ONLY;
	pAttr->AntiHowlingProcessor		= CONTROL__DISABLE;
	pAttr->EchoReturnLoss			= ACOUSTIC_ECHO_CANCELER_ECHO_RETURN_LOSS__0_dB;
	pAttr->MaxActiveFilterLength	= ACOUSTIC_ECHO_CANCELER_MAX_ACTIVE_FILTER_LENGTH__32_msec;
	pAttr->TailLength				= ACOUSTIC_ECHO_CANCELER_TAIL_LENGTH__128_msec;
	pAttr->CoefficientsThreshold	= ACOUSTIC_ECHO_CANCELER_COEFFICIENT_THRESHOLD__minus_49_dBm;
	pAttr->SaturationThreshold		= ACOUSTIC_ECHO_CANCELER_SATURATION_THRESHOLD__28671;
	pAttr->NlpActivationThreshold	= ACOUSTIC_ECHO_CANCELER_NLP_ACTIVATION_THRESHOLD__AUTOMATIC;
    pAttr->HighPassFilter           = CONTROL__ENABLE;
    pAttr->Decimation               = CONTROL__ENABLE;
    pAttr->IntensiveMode            = CONTROL__ENABLE;
}

#endif/*((AC49X_DEVICE_TYPE == AC494_DEVICE) || (AC49X_DEVICE_TYPE == AC495_DEVICE))*/

/****************************************************************/
void Ac49xSetDefaultRfc3558InitializationConfigurationAttr(Tac49xRfc3558InitializationConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xRfc3558InitializationConfigurationAttr));
	pAttr->PacketFormat								= RFC_3558_PACKET_FORMAT__BUNDLED;
    pAttr->LocalRateMode							= RFC_3558_EVRC_RATE_MODE__0_PERCENTAGE_OF_FULL_RATE_FRAMES__CHANGE_TO_HALF_RATE;
    pAttr->RemoteRateMode							= RFC_3558_EVRC_RATE_MODE__0_PERCENTAGE_OF_FULL_RATE_FRAMES__CHANGE_TO_HALF_RATE;
    pAttr->LocalRateModeControlledByRemoteSideEnable= CONTROL__ENABLE;
    pAttr->SilenceCompressionExtensionModeEnable	= CONTROL__DISABLE;
    pAttr->LocalMaxRate								= RFC_3558_CODER_RATE__1;
    pAttr->LocalMinRate								= RFC_3558_CODER_RATE__0_125;
    pAttr->DtxMin									= 12;
    pAttr->DtxMax									= 32;
	pAttr->NoiseSuppressionFilterEnable				= CONTROL__DISABLE;
	pAttr->Hangover									= 1;
}

/****************************************************************/
void Ac49xSetDefaultG729EvInitializationConfigurationAttr(Tac49xG729EvInitializationConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xG729EvInitializationConfigurationAttr));
	pAttr->AutomaticLocalRateControlMode  = G729EV_AUTOMATIC_CONTROL_MODE__LOCAL_RATE_CHANGED_BY_MBS;
    pAttr->LocalBitRate                   = G729EV_RATE_8_kbps;
    pAttr->MaxRemoteBitRate               = G729EV_RATE_8_kbps;
    pAttr->MaxSessionBitRate              = G729EV_RATE_32_kbps;
}

/****************************************************************/
void Ac49xSetDefaultRtcpXrConfigurationAttr(Tac49xRtcpXrConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xRtcpXrConfigurationAttr));
	pAttr->BurstAlertSetting				= RTCP_XR_ALERT_STATUS__NO_CHANGE_IN_ALERT_MODE;
	pAttr->DelayAlertSetting				= RTCP_XR_ALERT_STATUS__NO_CHANGE_IN_ALERT_MODE;
	pAttr->LowVoiceQualityAlertSetting		= RTCP_XR_ALERT_STATUS__NO_CHANGE_IN_ALERT_MODE;
	pAttr->MaximumBurstLossRateThreshold	= 205;
	pAttr->MaximumBurstDurationThreshold	= 200;
	pAttr->MaximumOneWayDelayThreshold		= 400;
	pAttr->LowVoiceQualityRValueThreshold   = 65;
}

/****************************************************************/
void Ac49xSetDefaultSetupChannelAttr(Tac49xSetupChannelAttr *pSetupChannelAttr)
{
    int i;
    Tac49xMediaProtectionDirection Direction;
    
	Ac49xSetDefaultAdvancedChannelConfigurationAttr(&pSetupChannelAttr->Advanced);

    for(i=0; i<2; i++)
		{
		Ac49xSetDefaultRtpChannelConfigurationAttr(&pSetupChannelAttr->ActivateOrUpdateRtp[i]);
        Ac49xSetDefaultAtmChannelConfigurationAttr(&pSetupChannelAttr->ActivateOrUpdateAtm[i]);
		Ac49xSetDefaultRtcpXrConfigurationAttr    (&pSetupChannelAttr->RtcpXrConfiguration[i]);
		}
	
	Ac49xSetDefaultChannelConfigurationAttr	(&pSetupChannelAttr->OpenOrUpdateChannel, MEDIA_CHANNEL_TYPE__REGULAR);
	Ac49xSetDefaultChannelConfigurationAttr	(&pSetupChannelAttr->MediatedChannelConfiguration, MEDIA_CHANNEL_TYPE__MEDIATED);
	Ac49xSetDefaultT38ConfigurationAttr				(&pSetupChannelAttr->T38Configuration);
	Ac49xSetDefault3GppUserPlaneInitializationConfigurationAttr(&pSetupChannelAttr->_3GppUserPlaneInitializationConfiguration[MEDIA_CHANNEL_TYPE__REGULAR],  MEDIA_CHANNEL_TYPE__REGULAR);
	Ac49xSetDefault3GppUserPlaneInitializationConfigurationAttr(&pSetupChannelAttr->_3GppUserPlaneInitializationConfiguration[MEDIA_CHANNEL_TYPE__MEDIATED], MEDIA_CHANNEL_TYPE__MEDIATED);
	Ac49xSetDefaultRfc3267AmrInitializationConfigurationAttr(&pSetupChannelAttr->Rfc3267AmrInitializationConfiguration);
	Ac49xSetDefaultRfc3558InitializationConfigurationAttr	(&pSetupChannelAttr->Rfc3558InitializationConfiguration);
	Ac49xSetDefaultG729EvInitializationConfigurationAttr	(&pSetupChannelAttr->G729EvInitializationConfiguration);
	Ac49xSetDefault3WayConferenceConfigurationAttr			(&pSetupChannelAttr->ActivateOrDeactivate3WayConferenceConfiguration);
    Ac49xSetDefault3GppIubChannelConfigurationAttr          (&pSetupChannelAttr->_3GppIubChannelConfiguration);
    for(i=0; i<2; i++)
		for(Direction=MEDIA_PROTECTION_DIRECTION__INBOUND_RX; Direction<=MEDIA_PROTECTION_DIRECTION__OUTBOUND_TX; Direction++)
			{
			Ac49xSetDefaultPacketCableProtectionConfigurationAttr(&pSetupChannelAttr->PacketCableProtectionConfiguration[i][Direction]);
			Ac49xSetDefaultSrtpProtectionConfigurationAttr		 (&pSetupChannelAttr->SrtpProtectionConfiguration[i][Direction], Direction);
			}

#if   (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	  || (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
      || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	Ac49xSetDefaultCodecConfigurationAttr				(&pSetupChannelAttr->CodecConfiguration);
	Ac49xSetDefaultAcousticEchoCancelerConfigurationAttr(&pSetupChannelAttr->AcousticEchoCancelerConfiguration);
#endif/*((AC49X_DEVICE_TYPE == AC494_DEVICE) || (AC49X_DEVICE_TYPE == AC495_DEVICE) || (AC49X_DEVICE_TYPE == AC496_DEVICE) || (AC49X_DEVICE_TYPE == AC496E_DEVICE) || (AC49X_DEVICE_TYPE == AC497_DEVICE))*/

	Ac49xSetDefaultUemClipInitializationConfigurationAttr(&pSetupChannelAttr->UemClipInitialization);
	Ac49xSetDefaultG7111InitializationConfigurationAttr(&pSetupChannelAttr->G7111Initialization);
	Ac49xSetDefaultDataRelayConfigurationAttr(&pSetupChannelAttr->ActivateOrUpdateDataRelay);
	Ac49xSetDefaultAdvancedDataRelayConfigurationAttr(&pSetupChannelAttr->AdvancedDataRelay);
	Ac49xSetDefaultMicrosoftRtaInitializationConfigurationAttr(&pSetupChannelAttr->MicrosoftRtaInitialization);

	pSetupChannelAttr->ActivateFaxRelay.SwitchToFax = CONTROL__DISABLE;
}                                                                                                                          

/****************************************************************/
void Ac49xSetDefault4WayConferenceConfigurationAttr(Tac49xOpenOrUpdate4WayConferenceConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xOpenOrUpdate4WayConferenceConfigurationAttr));
	
	pAttr->Member3HearsMember3 = 0;
	pAttr->Member3HearsMember2 = 1;
	pAttr->Member3HearsMember1 = 1;
	pAttr->Member3HearsMember0 = 1;
	pAttr->Member2HearsMember3 = 1;
	pAttr->Member2HearsMember2 = 0;
	pAttr->Member2HearsMember1 = 1;
	pAttr->Member2HearsMember0 = 1;
	pAttr->Member1HearsMember3 = 1;
	pAttr->Member1HearsMember2 = 1;
	pAttr->Member1HearsMember1 = 0;
	pAttr->Member1HearsMember0 = 1;
	pAttr->Member0HearsMember3 = 1;
	pAttr->Member0HearsMember2 = 1;
	pAttr->Member0HearsMember1 = 1;
	pAttr->Member0HearsMember0 = 0;
}

/****************************************************************/
void Ac49xSetDefault4WayConferenceLegConfigurationAttr(Tac49xAdd4WayConferenceLegConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xAdd4WayConferenceLegConfigurationAttr));
	
	pAttr->LegType = _4_WAY_CONFERENCE_LEG_TYPE__IP;
}

/****************************************************************/
void Ac49xSetDefaultSetup4WayConferenceAttr(Tac49xSetup4WayConferenceAttr *pSetup4WayConferenceAttr)
{
	Ac49xSetDefault4WayConferenceConfigurationAttr(&pSetup4WayConferenceAttr->OpenOrUpdateConference);
	memset((void*)&pSetup4WayConferenceAttr->CloseConference, 0, sizeof(Tac49xClose4WayConferenceConfigurationAttr));
}

/****************************************************************/
void Ac49xSetDefaultSetup4WayConferenceLegAttr(Tac49xSetup4WayConferenceLegAttr *pSetup4WayConferenceLegAttr)
{
	Ac49xSetDefault4WayConferenceLegConfigurationAttr(&pSetup4WayConferenceLegAttr->AddLeg);
	memset((void*)&pSetup4WayConferenceLegAttr->RemoveLeg, 0, sizeof(Tac49xRemove4WayConferenceLegConfigurationAttr));
}

/****************************************************************/
void Ac49xSetDefaultUemClipInitializationConfigurationAttr(Tac49xUemClipInitializationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xUemClipInitializationAttr));
	
	pAttr->RtpTimeStampResolution   = RTP_TIMESTAMP_RESOLUTION__16KHz;
	pAttr->TransmitMode				= UEM_CLIP_TRANSMIT_MODE__MODE_4;
}

/****************************************************************/
void Ac49xSetDefaultDataRelayConfigurationAttr(Tac49xActivateOrUpdateDataRelayConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xActivateOrUpdateDataRelayConfigurationAttr));
	pAttr->MaxPayloadSizeForSprtTransportChannel2	= 132;
	pAttr->MaxPayloadSizeForSprtTransportChannel0	= 140;
	pAttr->MaxPayloadSizeForSprtTransportChannel3	= 140;
	pAttr->MaxWindowSizeForSprtTransportChannel2	= 8;
	pAttr->RxPayloadTypeOfSprtPackets				= 105;
	pAttr->TxPayloadTypeOfSprtPackets				= 105;
	pAttr->SprtPacketsTransmitProtocol				= SPRT_PACKETS_PROTOCOL__RTP;
	pAttr->SprtPacketsReceiveProtocol				= SPRT_PACKETS_PROTOCOL__RTP;
}

/****************************************************************/
void Ac49xSetDefaultAdvancedDataRelayConfigurationAttr(Tac49xAdvancedDataRelayConfigurationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xAdvancedDataRelayConfigurationAttr));
	pAttr->ModemTransmissionPower			= MODEM_TRANSMISSION_POWER__minus12_DBM;
	pAttr->V32ModulationTypeEnable			= CONTROL__ENABLE;
	pAttr->V34ModulationTypeEnable			= CONTROL__ENABLE;
	pAttr->ModemDiagnosticsEnable			= CONTROL__DISABLE;
	pAttr->PcmLevelOfTransmissionOfPhase1	= DATA_RELAY_TRANSMISSION_PCM_LEVEL__MINUS_12;
	pAttr->MaxTxBitRate						= DATA_RELAY_BIT_RATE__33600_BPS;
	pAttr->MinTxBitRate						= DATA_RELAY_BIT_RATE__4800_BPS;
	pAttr->MaxRxBitRate						= DATA_RELAY_BIT_RATE__33600_BPS;
	pAttr->MinRxBitRate						= DATA_RELAY_BIT_RATE__4800_BPS;
}

/****************************************************************/
void Ac49xSetDefaultG7111InitializationConfigurationAttr(Tac49xG7111InitializationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xG7111InitializationAttr));
	pAttr->TransmitMode				= G711_1_TRANSMIT_MODE__R3;
	pAttr->RtpTimestampResolution	= RTP_TIMESTAMP_RESOLUTION__16KHz;
}

/****************************************************************/
void Ac49xSetDefaultMicrosoftRtaInitializationConfigurationAttr(Tac49xMicrosoftRtaInitializationAttr *pAttr)
{
	memset((void*)pAttr, 0, sizeof(Tac49xMicrosoftRtaInitializationAttr));

	pAttr->VariableRateSupport			= VARIABLE_RATE_SUPPORT__VARIABLE_BIT_RATE;
	pAttr->JitterBufferMode				= JITTER_BUFFER_MODE__NORMAL;
	pAttr->ForwardErrorCorrectionMode	= FORWARD_CORRECTION_MODE__ENABLED;
}

/****************************************************************/

/********************************************************************************************/
/********************* B A C K W A R D S    C O M P A T I B I L I T Y ***********************/
/********************************************************************************************/
/********************************************************************************************/
/* Due to renaming of these three function													*/
/* 1. Ac49xActivateAtmChannelConfiguration	--> Ac49xActivateRegularAtmChannelConfiguration.*/
/* 2. Ac49xUpdateAtmChannelConfiguration	--> Ac49xUpdateRegularAtmChannelConfiguration.	*/
/* 3. Ac49xCloseAtmChannelConfiguration		--> Ac49xCloseRegularAtmChannelConfiguration.	*/
/* these functions are still supported by this drivers package for backwards competabilty	*/
/* NOTE: Please use these new functions for better performance.								*/ 
/********************************************************************************************/
int Ac49xActivateAtmChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdateAtmChannelConfigurationAttr *pAttr)
{
    return Ac49xActivateRegularAtmChannelConfiguration(Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xUpdateAtmChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdateAtmChannelConfigurationAttr *pAttr)
{                                                             
    return Ac49xUpdateRegularAtmChannelConfiguration(Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xCloseAtmChannelConfiguration(int Device, int Channel)
{
    return Ac49xCloseRegularAtmChannelConfiguration(Device, Channel);
}

/********************************************************************************************/
/* Due to renaming of these two function													*/
/* 1. Ac49xActivateT38Command				--> Ac49xActivateFaxRelayCommand.				*/
/* 2. Ac49xDeactivateT38Command				--> Ac49xDeactivateFaxRelayCommand.				*/
/* these functions are still supported by this drivers package for backwards competabilty	*/
/* NOTE: Please use these new functions for better performance.								*/ 
/********************************************************************************************/
/****************************************************************/
int Ac49xActivateT38Command(int Device, int Channel, Tac49xActivateFaxRelayAttr *pAttr)
{
	return Ac49xActivateFaxRelayCommand(Device, Channel, pAttr);
}

/****************************************************************/
int Ac49xDeactivateT38Command(int Device, int Channel)
{
	return Ac49xDeactivateFaxRelayCommand(Device, Channel);
}

/********************************************************************************************/
/* Due to renaming of this function															*/
/* 1. Ac49x3GppUserPlaneInitializationConfiguration -->										*/
/*    Ac49xRegularChannel3GppUserPlaneInitializationConfiguration.							*/
/* this function is still supported by this drivers package for backwards competabilty		*/
/* NOTE: Please use this new functions for better performance.								*/ 
/********************************************************************************************/
/****************************************************************/
int Ac49x3GppUserPlaneInitializationConfiguration(int Device, int Channel, Tac49x3GppUserPlaneInitializationConfigurationAttr *pAttr)
{
	return Ac49xRegularChannel3GppUserPlaneInitializationConfiguration(Device, Channel, pAttr);
}

/********************************************************************************************/
/* Due to renaming of this function															*/
/* 1. Ac49xSet3GppUserPlaneRateControl -->													*/
/*    Ac49xRegularChannelSet3GppUserPlaneRateControl.										*/
/* 2. Ac49xGet3GppStatistics -->															*/
/*    Ac49xRegularChannelGet3GppStatistics.													*/
/* this function is still supported by this drivers package for backwards competabilty		*/
/* NOTE: Please use this new functions for better performance.								*/ 
/********************************************************************************************/
/****************************************************************/
int Ac49xSet3GppUserPlaneRateControl(int Device, int Channel, Tac49xSet3GppUserPlaneRateControlAttr *pAttr)
{
	return Ac49xRegularChannelSet3GppUserPlaneRateControl(Device, Channel, pAttr);
}

/***************************************************************************************************************************/
int Ac49xGet3GppStatistics(int Device, int Channel)
{
	return Ac49xRegularChannelGet3GppStatistics(Device, Channel);
}
                                                                                 
#endif /* AC49X_DEVICE_TYPE... This  driver is compiled only for the supported device types */
