/** @file
 *  @brief This file ensures compatibility with Customer PCs whose architecture is little Endian and in which, within 
 *  a given multi-byte numeric representation, the most significant byte has the lowest address (the word is 
 *  stored "little-end-first"). Most processors, including the IBM 370 family, the PDP-10, the Motorola microprocessor 
 *  families, and most of the various RISC designs current in mid-1993, are little Endian.
 */

#ifndef AC49X_LITTLE_ENDIAN_BIT_FIELDS_H
#define AC49X_LITTLE_ENDIAN_BIT_FIELDS_H

typedef volatile struct
    {
    U16                 : 8;

    U16 Reset           : 1;
    U16 DspInt          : 1;
    U16                 : 6;
    } Tac49xHpic;

typedef volatile struct 
    {
    U16                     : 8;
    U16                     : 8;
    U16                     : 8;
    U16                     : 8;
    } Tac49xPaddingForFutureNeed;

typedef volatile struct 
    {
    U16 PacketSize_Msb      : 5;
    U16 Partial             : 1;
    U16 Wrap                : 1;
    U16 Empty               : 1;

    U16 PacketSize_Lsb      : 8;

    U16 PacketAddress_Msb   : 8;    /* The packet physical address (MSB) */

    U16 PacketAddress_Lsb   : 8;    /* The packet physical address (LSB) */
    } Tac49xBufferDescriptor;

typedef volatile struct 
    {
    U16 NetworkCellSize     : 8;   
    U16 HostCellSize        : 8; 
    
	U16                     : 5;   
	U16 NetworkDebug 		: 1;   
	U16 HostDebug 	        : 1;  
    U16 Reset				: 1; /* Host set, DSP release */  

	U16                     : 8;   
    } Tac49xDeviceControlRegister; 

typedef volatile struct 
    {
	U16                     : 8;   
	U16                     : 8;   
	U16                     : 8;   
	U16                     : 8;   
    } Tac49xDeviceStatusRegister; 

typedef volatile struct 
    {
	U16                     : 8;   
	U16 Status              : 8;   
    } Tac49xBootStatusRegister;

typedef volatile struct 
    {
	U16			   	  	    : 4;
	U16 UdpChecksumIncluded : 1; /*!< @li 0 = UDP checksum not included in this packet @li 1 = UDP checksum included in this packet*/
	U16 Sync5 		        : 3; /*!< Synchronization bits 101*/
	
	U16 Channel             : 8; /*!< Channel index within the device*/

	U16			   	  	    : 7;
	U16	FavoriteStream		: 1; /*!< This parameter is set to zero in all status packets.*/

	U16 Protocol            : 6; /*!< Refer to ::Tac49xProtocol*/
	U16			   	  	    : 2;
    } Tac49xHpiPacketHeader;

typedef volatile struct 
    {
	U16 		  	        : 4;
	U16 Format		  	    : 4; /*!< This field distinguishes between several formats of the same command. For example, the open device function has two formats (::Ac49xOpenDeviceConfiguration_Format0 and ::Ac49xOpenDeviceConfiguration_Format1). For all commands that have a single format, this field must be set to 0.*/
	
	U16 PacketCategory 	    : 2; /*!< Refer to ::Tac49xPacketCategory*/
	U16 Opcode 		        : 6; /*!< Refer to ::Tac49xStatusOrEventPacketOpCode for status Opcodes*/
    } Tac49xProprietaryPacketHeader;

typedef volatile struct 
    {
	U16 Sequence		    : 8;   
	U16 Checksum		    : 8;   
    } Tac49xPacketFooter;

typedef volatile struct 
    {
	U16                     : 8;   
	U16                     : 8;   

	U16                     : 2;   
	U16 ReferenceCoreId     : 3;   
	U16                     : 1;   
	U16 One                 : 1;   
	U16                     : 1;   

	U16 CommandIndex        : 4;   
	U16                     : 4;   
    } Tac49xHcrfCommandRegister; 

typedef volatile struct 
    {
	U16                     : 8;   
	U16                     : 8;   

	U16 StatusIndex_Msb     : 7;   
	U16 WatchdogTimeoutFlag : 1;  /*!< Watchdog Timeout Flag. Alerts the Host of a core timer timeout condition. This bit is cleared after the Host reads the status. @n 1 = Timeout did not occur. @n 0 = Timeout occurred.*/
	U16 StatusIndex_Lsb     : 8;   
    } Tac49xHcrfStatusRegister; 

            /********************************/
            /* Device Configuration section */
            /********************************/
    typedef volatile struct
        {
	    U16 NumberOfSignals		            : 6;
	    U16         			            : 2;
                                            
	    U16 TotalEnergyThreshold            : 6;
	    U16         			            : 2;
        }Tac49xSignalCommonAttributes;
    
    typedef volatile struct
        {          /*1*/
	    U16 ToneAFrequency_Msb		        : 4;
		U16									: 1;
	    U16 Type       	  					: 2;
		U16									: 1;

	    U16 ToneAFrequency_Lsb	            : 8;
                   /*2*/  
	    U16 ToneB_OrAmpModulationFrequency_Msb	: 4;
	    U16 TwistThreshold      	        : 4;

	    U16 ToneB_OrAmpModulationFrequency_Lsb	: 8;
                   /*3*/
	    U16 ThirdToneOfTripleBatchDurationTypeFrequency_Msb	: 4;
	    U16       			                : 4;

	    U16 ThirdToneOfTripleBatchDurationTypeFrequency_Lsb	: 8;
                   /*4*/
	    U16 HighEnergyThreshold             : 6;
	    U16       			                : 2;
                                            
	    U16 LowEnergyThreshold              : 6;
	    U16       			                : 2;
                   /*5*/
	    U16 SignalToNoiseRatioThreshold     : 7;
	    U16       			                : 1;

	    U16 FrequencyDeviationThreshold     : 6;
	    U16       			                : 2;
                   /*6*/                  
	    U16 ToneBLevel                      : 6;
	    U16       			                : 2;
                                            
	    U16 ToneALevel      	            : 6;
	    U16       			                : 2;
                   /*7*/
	    U16 DetectionTimeOrCadenceFirstOnOrBurstDuration_Msb:2;
	    U16 AmFactor                        : 6;

	    U16 DetectionTimeOrCadenceFirstOnOrBurstDuration_Lsb:8;
                   /*8*/
	    U16 CadenceFirstOffDuration_Msb	    : 2;
	    U16                                 : 5;
	    U16 CadenceVoiceAddedWhileFirstOff  : 1;

	    U16 CadenceFirstOffDuration_Lsb		: 8;
                   /*9*/
	    U16 CadenceSecondOnDuration_Msb	    : 2;
	    U16                                 : 6;

	    U16 CadenceSecondOnDuration_Lsb		: 8;
                   /*10*/
	    U16 CadenceSecondOffDuration_Msb	: 2;
	    U16                                 : 5;
	    U16 CadenceVoiceAddedWhileSecondOff : 1;

	    U16 CadenceSecondOffDuration_Lsb	: 8;
                   /*11*/
	    U16 CadenceThirdOnDuration_Msb	    : 2;
	    U16                                 : 6;
         
	    U16 CadenceThirdOnDuration_Lsb		: 8;
                   /*12*/
	    U16 CadenceThirdOffDuration_Msb	    : 2;
	    U16                                 : 5;
	    U16 CadenceVoiceAddedWhileThirdOff  : 1;

	    U16 CadenceThirdOffDuration_Lsb		: 8;
                   /*13*/
	    U16 CadenceFourthOnDuration_Msb	    : 2;
	    U16                                 : 6;
         
	    U16 CadenceFourthOnDuration_Lsb		: 8;
                   /*14*/
	    U16 CadenceFourthOffDuration_Msb    : 2;
	    U16                                 : 5;
	    U16 CadenceVoiceAddedWhileFourthOff : 1;

	    U16 CadenceFourthOffDuration_Lsb	: 8;
        }Tac49xCallProgressSignal;

    typedef volatile struct
        {          /*1*/
	    U16 ToneAFrequency_Msb		        : 4;
	    U16        	  	                    : 4;
                                            
	    U16 ToneAFrequency_Lsb	            : 8;
                   /*2*/                  
	    U16 ToneBFrequency_Msb	            : 4;
	    U16 TwistThreshold  	            : 4;
                                            
	    U16 ToneBFrequency_Lsb		        : 8;
                   /*3*/                  
	    U16 HighEnergyThreshold             : 6;
	    U16       			                : 2;
                                            
	    U16 LowEnergyThreshold              : 6;
	    U16       			                : 2;
                   /*4*/
	    U16 SignalToNoiseRatioThreshold     : 6;
	    U16       			                : 2;

	    U16 FrequencyDeviationThreshold     : 6;
	    U16       			                : 2;
                   /*5*/                  
	    U16 ToneBLevel                      : 6;
	    U16       			                : 2;
                                            
	    U16 ToneALevel      	            : 6;
	    U16       			                : 2;
        }Tac49xUserDefinedToneSignal;

typedef volatile struct
    {
	Tac49xHpiPacketHeader			HpiHeader;
	Tac49xProprietaryPacketHeader	ProprietaryHeader;

	U16 MinGain		                : 5;  
	U16 		                    : 3;	

	U16 MaxGain		                : 5;
	U16 		                    : 3;	
                 
	U16 		                    : 8;	

	U16 FastAdaptationGainSlope		: 5;	
	U16 		                    : 3;	

	U16 FastAdaptationDuration_Msb  : 8;
	U16 FastAdaptationDuration_Lsb  : 8; 

	U16 SilenceDuration_Msb		    : 8;	
	U16 SilenceDuration_Lsb		    : 8;  

	U16 DetectionDuration_Msb	    : 8;	
	U16 DetectionDuration_Lsb       : 8;  

	U16 SignalingOverhang_Msb	    : 8;	
	U16 SignalingOverhang_Lsb	    : 8; 
                 
	U16 		                    : 8;
	U16 		                    : 8;	
    Tac49xPaddingForFutureNeed _    ;
    } Tac49xAgcDeviceConfigurationPacket;

typedef volatile struct
    {
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;

	U16 									: 7;
	U16 DebugUemClipEnable					: 1; 
											  
	U16 									: 8;

	U16 VxxV8Ci								: 1;
	U16 VxxV8Cm								: 1;
	U16 TextV8Ci							: 1;
	U16 TextV8Cm							: 1;
	U16 TextV8Jm							: 1;
	U16 									: 3;

	U16 StuModem						    : 1;
	U16 V25CallingTone						: 1;
    U16 V21Channel1							: 1;
    U16 Bell103AnswerTone					: 1;
    U16 Tty									: 1;
    U16 FaxDcn								: 1;
	U16 FaxV8Ci								: 1;
	U16 FaxV8Cm								: 1;

	U16 VxxV8JmVxxData						: 1;
	U16 V32Aa								: 1;
	U16 V22Usb1								: 1;
	U16 InitiatingDualToneV8Bis				: 1;
	U16 RespondingDualToneV8Bis				: 1;
    U16 DataSessionVxx						: 1;
    U16 V21Channel2Data						: 1;
	U16 V23ForwardChannel					: 1;

	U16 SilenceOrUnkown						: 1;
	U16 CallingFaxToneCng					: 1;
	U16 AnswerTone2100FaxCedOrModemAns		: 1;
	U16 AnswerWithReversalsAns				: 1;
	U16 AnswerToneAm						: 1;
	U16 AnswerToneAmReversals				: 1;
	U16 FaxV21PreambleFlags					: 1;
	U16 FaxV8JmV34							: 1;

	U16 DtmfFrequencyDeviation				: 3;
    U16 DtmfErasureSensitivity              : 2;
	U16 									: 2;
    U16 MfcR1DetectionStandard              : 1;
	U16 DtmfStepDistance					: 4;
	U16 DtmfStepRatio						: 4;
	} Tac49xExtendedDeviceConfigurationPacket;

typedef volatile struct
    {
    U16 Slot_Msb                : 1;
    U16                         : 7;

    U16 Slot_Lsb                : 8;

    U16 InterconnectedSlot_Msb  : 1;
    U16                         : 7;

    U16 InterconnectedSlot_Lsb  : 8;
    } Tac49xSlot;
typedef volatile struct                     
    {                              
    U16 PcmSlot_Msb				   : 1;
    U16							   : 7;
									
    U16 PcmSlot_Lsb				   : 8;
									
	U16 CasSlot_Msb				   : 1;
    U16							   : 7;
									
    U16 CasSlot_Lsb				   : 8;

    U16 InterconnectedPcmSlot_Msb  : 1;
    U16							   : 7;

    U16 InterconnectedPcmSlot_Lsb  : 8;
    } Tac49xSlot_Format1;

typedef volatile struct                     
    {                              
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;

    U16 BusSpeed							    : 3;
	U16	DebugInfoPacketEnable				    : 1;
    U16 DeviceStatusPacketEnable			    : 1;
    U16 CasReversalsDetection				    : 1;
    U16 ConnectedSlotEnable					    : 1;
    U16 JitterBufferStatusReportMode		    : 1;
											      
    U16	PcmClkAndFrameSyncSource			    : 1;
	U16	DtmfSnr								    : 3;
    U16	CasStreamPortSelection				    : 1;
    U16	IpSecPortSelection					    : 1;
    U16	SerialPortSelection					    : 1;
    U16	WatchdogEnable						    : 1;
                                                
    U16 CasValueDuringIdle					    : 4;
    U16 CasMode								    : 2;			   
    U16 LawSelect							    : 2;			   
											      
    U16 PcmValueDuringIdle					    : 8;	
											      
    U16 UtopiaClockDetection				    : 8;	
											      
    U16 CasDebounce							    : 4;
    U16 UtopiaCellSize						    : 4;	
											    
    U16	DtmfGenerationTwist					    : 4;	
    U16	DtmfGenerationTwistDirection		    : 1;	
    U16	CasBitsLocation						    : 1;	
    U16 PcmSerialPortRxTxDataDelay			    : 2;
											      
    U16 UtopiaLogicalSlaveId				    : 6;	
    U16 UtopiaTestEnable					    : 1;	
    U16 Utopia16BitMode						    : 1;	
											    
    U16 DeviceStatusInterval				    : 8;	
    U16 ErrorReportInterval					    : 8;	
											    
    U16										    : 8;	
    U16										    : 8;
											      
    U16	NumberOfChannels_Msb				    : 1;
    U16	DtmfGap								    : 2;	
    U16	AicClkFrequency						    : 2;
    U16	CpuClkOut							    : 1;
    U16	CodecEnable							    : 2;
    U16	NumberOfChannels_Lsb				    : 8;
											    
    U16	PcmFrameSyncWidth					    : 8;	
    U16	PcmClkDivideRatio					    : 8;
		
    U16	UtopiaPhysicalSlaveId				    : 5;	
	U16										    : 2;
    U16	ModuleDebugPacketsTransferMethod		: 1;
                                                  
    U16	EventsPortType                          : 1;
    U16	TdmInterfaceConfiguration			    : 2;
    U16										    : 5;
    Tac49xSlot							Channel[MAX_NUMBER_OF_ACTIVE_TIME_SLOTS];
    Tac49xPaddingForFutureNeed _;
    } Tac49xOpenDeviceConfigurationPacket;    

typedef Tac49xOpenDeviceConfigurationPacket Tac49xOpenDeviceConfigurationPacket_Format0;

typedef volatile struct 
	{
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
		/* 3 */
	U16 BusSpeed							    : 3;
	U16	DebugInfoPacketEnable				    : 1;
	U16 DeviceStatusPacketEnable			    : 1;
	U16 CasReversalsDetection				    : 1;
	U16 										: 1;
	U16 JitterBufferStatusReportMode		    : 1;
												
	U16	PcmClkAndFrameSyncSource			    : 1;
	U16	DtmfSnr								    : 3;
	U16										    : 1;
	U16	IpSecPortSelection					    : 1;
	U16 										: 1;
	U16	WatchdogEnable						    : 1;
		/* 4 */									  
	U16 CasValueDuringIdle					    : 4;
	U16 CasMode								    : 2;			   
	U16 LawSelect							    : 2;			   
												  
	U16 PcmValueDuringIdle					    : 8;	
		/* 5 */									  
	U16 UtopiaClockDetection				    : 8;	
												  
	U16 CasDebounce							    : 4;
	U16 UtopiaCellSize						    : 4;	
        /* 6 */                                    
	U16	DtmfGenerationTwist					    : 4;	
	U16	DtmfGenerationTwistDirection		    : 1;	
	U16	CasBitsLocation						    : 1;	
	U16 PcmSerialPortRxTxDataDelay			    : 2;
                                            
	U16 UtopiaLogicalSlaveId				    : 6;	
	U16 UtopiaTestEnable					    : 1;	
	U16 Utopia16BitMode						    : 1;	
		/* 7 */										
	U16 DeviceStatusInterval				    : 8;	
	U16 ErrorReportInterval					    : 8;	
		/* 8 */									
	U16										    : 8;	
	U16										    : 8;
		/* 9 */										  
	U16	NumberOfChannels_Msb				    : 1;
	U16	DtmfGap								    : 2;	
	U16	AicClkFrequency						    : 2;
	U16	CpuClkOut							    : 1;
	U16	CodecEnable							    : 2;
	U16	NumberOfChannels_Lsb				    : 8;
		/* 10 */									
	U16	PcmFrameSyncWidth					    : 8;
	U16	PcmClkDivideRatio					    : 8;
		/* 11 */										
	U16	UtopiaPhysicalSlaveId				    : 5;	
	U16										    : 2;
	U16	ModuleDebugPacketsTransferMethod		: 1;
                                              
	U16	EventsPortType                          : 1;
	U16	TdmInterfaceConfiguration			    : 2;
	U16										    : 5;
		/* 12-15 */
	U16										    : 8;
	U16										    : 8;
	U16										    : 8;
	U16										    : 8;
	U16										    : 8;
	U16										    : 8;
	U16										    : 8;
	U16										    : 8;
		/* 16 */
	U16										    : 4;
	U16	ConnectedSlots						    : 1;
	U16	ConnectedSlotsSerialPortSelection 	    : 1;
	U16	CasSerialPortSelection 				    : 1;
	U16	VoiceSerialPortSelection			    : 1;
												  
	U16										    : 8;
		/* 17-811*/
	Tac49xSlot_Format1					Channel[MAX_NUMBER_OF_ACTIVE_TIME_SLOTS];
	Tac49xPaddingForFutureNeed _;
	} Tac49xOpenDeviceConfigurationPacket_Format1;
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	)
#ifdef UTOPIA_ROUTING
typedef volatile struct
	{
	U16	ToChannel_Msb		:1;
	U16	ToDevice			:4;
	U16	ToMediaChannelType	:3;

	U16	ToChannel_Lsb		:8;
	} Tac49xSingleMediaChannelType;
typedef volatile struct
	{
	Tac49xSingleMediaChannelType MediaChannelType[AC49X_NUMBER_OF_MEDIA_CHANNEL_TYPES];
	} Tac49xSingleUtopiaRouting;
typedef volatile struct
	{
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
	Tac49xSingleUtopiaRouting Route[AC49X_NUMBER_OF_CHANNELS]; 
    Tac49xPaddingForFutureNeed _    ;
	} Tac49xUtopiaRoutingConfigurationPacket;
#endif /* UTOPIA_ROUTING*/
#endif /* (AC49X_DEVICE_TYPE == AC491_DEVICE) || (AC49X_DEVICE_TYPE == AC491L_DEVICE)*/
	
            /*********************************/
            /* Channel Configuration Section */
            /*********************************/

typedef volatile struct
    {
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
    U16 EncoderIbsOverhang_10msec					: 8;
													  
    U16 DecoderIbsOverhang_10msec					: 8;
													  
    U16	PcmCngThreshold								: 6;	
    U16												: 1;	
    U16 MuteEncoderDuringDtmfGeneration				: 1;
													
    U16 SignalingJitter								: 8;
													  
    U16 FaxModemJitter_Msb							: 3;
    U16												: 5;	
													  
    U16 FaxModemJitter_Lsb							: 8;
													
    U16 BypassJitter_Msb							: 3;
    U16												: 1;	
    U16	EchoCancelerAutomaticHandler				: 1;	
    U16 EchoCancelerNlpDuringBypass					: 1;
    U16 EchoCancelerDuringFaxBypassDisable			: 1;
    U16 EchoCancelerDuringDataBypass				: 1;
													  
    U16 BypassJitter_Lsb							: 8;
													  
    U16 EchoCancelerNlpThreshold					: 6;
    U16 EchoCancelerDcRemover						: 2;
											  
	U16	AcousticEchoCancelerEnable					: 1;	
	U16	EchoCancellerAutomaticToneResetOnMuteMode	: 1;	
	U16	EchoCancellerAutomaticToneResetMode			: 1;
	U16 EchoCancelerReconvergenceSupport			: 1;
	U16 EchoCancelerNoisyEnvironmentMode			: 1;
    U16 EchoCancelerAggressiveNlp					: 1;
    U16 EchoCancelerToneDetector					: 1;
    U16	EchoCancelerCng								: 1;
													  
    U16 HdlcInterFrameFillPattern					: 1; 
    U16												: 4;	
	U16	Ss7LinkRate									: 1;
	U16	HdlcTransferMedium							: 1;									
	U16	Ss7TransferMedium							: 1;									
													  
    U16 HdlcSampleRate								: 2;
    U16 HdlcMinimumEndFlags							: 2;
    U16 HdlcMinimumMessageLength					: 3;
    U16												: 1;	
													
	U16												: 6;
    U16	InputGainLocation							: 1;
    U16 PlaybackRequestEnable						: 1;
													
    U16 PlaybackWaterMark_msec						: 8;
													
    U16	DataBypassOutputGain						: 6;
	U16												: 2;
													
    U16	FaxBypassOutputGain							: 6;
	U16												: 2;
													
	U16	DuaFrameMode								: 1;
	U16												: 7;
	U16	DuaReceiveTimeout_x10msec					: 8;
													
													
	U16 AnswerDetectSilenceTime_x100ms_Msb			: 3;
	U16												: 4;
	U16 AnswerDetectNoneSpeechEventsEnable			: 1;
													
	U16 AnswerDetectSilenceTime_x100ms_Lsb			: 8;
													
	U16 AnswerDetectPreWorkDelay_x100ms_Msb			: 3;
	U16												: 5;
													
	U16 AnswerDetectPreWorkDelay_x100ms_Lsb			: 8;

    } Tac49xAdvancedChannelConfigurationPacket;
                                          
typedef volatile struct                            
    {                                     
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
            /* 3 */
	U16 SilenceCompressionMode					: 2;
    U16 BrokenConnectionActivationMode			: 1;	
    U16											: 1;	
	U16 AdpcmPayloadFormat						: 1;
    U16 DisableRtcpIntervalRandomization		: 1;
    U16 AmrActiveMode							: 2;
												
    U16	MediaProtectionMode						: 3;
    U16 RtpRedundancy							: 2;
    U16 IbsRedundancyLevel						: 2;
    U16											: 1;	
            /* 4 */								
    U16 RxPayloadType							: 7;
    U16 RxPayloadTypeSource						: 1;	
												  
    U16 TxPayloadType							: 7;
    U16 TxPayloadTypeSource						: 1;	
            /* 5 */								
    U16 TxSequenceNumber_Msb					: 8;
    U16 TxSequenceNumber_Lsb					: 8;
            /* 6 */								
    U16 FaxBypassPayloadType					: 7;
    U16											: 1;
												
    U16 DataBypassPayloadType					: 7;
    U16											: 1;
            /* 7 */								
    U16 RtpSynchronizingSource_MswMsb			: 8;
    U16 RtpSynchronizingSource_MswLsb			: 8;
            /* 8 */								
    U16 RtpSynchronizingSource_LswMsb			: 8;
    U16 RtpSynchronizingSource_LswLsb			: 8;
            /* 9 */								
    U16 TxTimeStamp_MswMsb						: 8;
    U16 TxTimeStamp_MswLsb						: 8;
            /* 10 */							
    U16 TxTimeStamp_LswMsb						: 8;
    U16 TxTimeStamp_LswLsb						: 8;
            /* 11 */
    U16 PcmAdPcmComfortNoiseRtpPayloadType		: 7;
    U16 PcmAdPcmComfortNoiseRtpPayloadTypeEnable: 1;

    U16 BypassCoder								: 8;
            /* 12 */							
    U16 TxVbdEventsRelayPayloadType				: 7;
    U16 										: 1;
												
    U16 NumberOfBypassBlocks_minus_1			: 4;
    U16 NumberOfVoiceBlocks_minus_1				: 4;
            /* 13 */							
    U16 DecoderPcmAdpcmFrameDuration			: 8;
    U16 EncoderPcmAdpcmFrameDuration			: 8;
            /* 14 */							
    U16 RtcpMeanTxInterval_Msb					: 8;
    U16 RtcpMeanTxInterval_Lsb					: 8;
            /* 15 */							
    U16 Decoder									: 8;
												
    U16 Encoder									: 8;  
            /* 16 */							
    U16 TxSignalingRfc2833PayloadType			: 7;
    U16											: 1;
												
    U16 RtpCanonicalNameStringLength			: 8;
            /* 17 */
    U16 BrokenConnectionEventTimeout_x20msec_Msb: 8;
    U16 BrokenConnectionEventTimeout_x20msec_Lsb: 8;
			/* 18 */
    U16 RxSignalingRfc2833PayloadType			: 7;
    U16											: 1;
												
    U16 Rfc2198PayloadType						: 7;
    U16											: 1;
			/* 19 */							
	U16 RxT38PayloadType						: 7;
	U16											: 1;
	U16 TxT38PayloadType						: 7;
	U16											: 1;
			/* 20 */							
	U16 NoOperationPayloadType					: 7;
	U16	NoOperationEnable						: 1;
    U16											: 8;
			/* 21 */							
	U16 NoOperationTimeout_x20msec_Msb			: 7;
	U16											: 1;
	U16 NoOperationTimeout_x20msec_Lsb			: 8;
			/* 22 */							
    U16											: 1;
	U16	ReportErrorOnReceivedStreamEnable		: 1;
	U16 BlockNonePayloadType13					: 1;
	U16 PacketToPacketSsrcMode					: 1;
	U16 PacketToPacketSequenceNumberMode		: 1;
    U16 BypassRedundancyType					: 1;
    U16 VbdEventsRelayType						: 2;
																							
	U16 PacketToPacketNoTranscodingPayloadType	: 7;
    U16											: 1;
			/* 23 */                                
    U16 TxFilterRtp								: 1;
    U16 TxFilterRfc2833							: 1;
    U16 TxFilterRtcp							: 1;
    U16											: 5;
												
    U16 RxFilterRtp								: 1;
    U16 RxFilterRfc2833							: 1;
    U16 RxFilterRtcp							: 1;
    U16											: 5;
			/* 24 */
	U16 IbsRelayTransmissionTimeout_Msb			: 8;
	U16 IbsRelayTransmissionTimeout_Lsb			: 8;
			/* 25 */						
	U16 AnswerToneOverRfc2833Enable				: 1;
	U16 ModemRelayHighAvailabilityOptions		: 2;
	U16 SseRedundancyDepth						: 3;
	U16											: 1;
	U16 RtpSequenceNumberDetectionEnable		: 1;

	U16	RtcpXrMode								: 2;
	U16	RtcpXrEventsMode						: 2;
    U16											: 4;
			/* 26 */
    U16	SequenceNumberForDetection_Msb			: 8;
    U16	SequenceNumberForDetection_Lsb			: 8;
		/* 27 */
    U16											: 8;
    
    U16 RxVbdEventsRelayPayloadType				: 7;
	U16 										: 1;
		/* 28..31 */
    U16											: 8;
    U16											: 8;
    U16											: 8;
    U16											: 8;
    U16											: 8;
    U16											: 8;
    U16											: 8;
    U16											: 8;
		/* 32..95 */
    U8 RtpCanonicalName[AC49X_MAX_RTP_CANONICAL_NAME_STRING_LENGTH];
    Tac49xPaddingForFutureNeed _;
    } Tac49xActivateOrUpdateRtpChannelConfigurationPacket;

typedef volatile struct
	{
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
    U16                                 : 6;  
    U16 SendReason                      : 1;  
    U16 SendRtcpByePacket               : 1; 

	U16									: 8;

	U8	ReasonMessageLength				: 8;
	U8	LeavingReasonMessage[AC49X_RTP_MAX_LEAVING_REASON_MESSAGE_SIZE];
    Tac49xPaddingForFutureNeed _;
	} Tac49xCloseRtpChannelPacket;

typedef volatile struct
    {
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
    U16												: 8;	
													
    U16 Protocol									: 3;  
    U16 G711CoderType								: 2;  
    U16 BrokenConnectionActivationMode				: 1;	
    U16												: 2;  
													
    U16 ProfileEntry								: 5;
    U16 ProfileGroup								: 2;	
    U16												: 1;	
													
    U16 ProfileId									: 5;
    U16												: 1;	
    U16 SilenceCompressionMode						: 2;  
													
    U16 AtmAdaptionLayer2RxChannelId				: 8;  
    U16 AtmAdaptionLayer2TxChannelId				: 8;  
    U16 BrokenConnectionEventTimeout_x20msec_Msb	: 8;
    U16 BrokenConnectionEventTimeout_x20msec_Lsb	: 8;
    Tac49xPaddingForFutureNeed _;
    } Tac49xActivateOrUpdateAtmChannelConfigurationPacket;

typedef volatile struct
    {
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
                /*Word 3*/
    U16 ChannelType                     : 3; 
    U16                                 : 3; 
	U16 PacketToPacketMode				: 1;
    U16 PcmSource                       : 1; 

    U16                                 : 8; 
                /*Word 4*/
    U16 DtmfTransferMode                : 3; 
    U16 MfTransferMode                  : 3; 
    U16 DtmfErasureMode                 : 2; 

	U16 PcmLimiter						: 3;
    U16                                 : 3; 
    U16 EchoCancelerNonLinearProcessor  : 2;
/*  U16 SilenceCompressionMode          : 2;*/
/*  U16 NumberOfVoiceBlocks_minus_1     : 4;*/ 
                /*Word 5*/
    U16 VoiceOutputGain                 : 6;
    U16 HybridLoss                      : 2;

    U16 InputGain                       : 6;
    U16 EchoCancelerMode                : 2;
                /*Word 6*/                                      
    U16 DynamicJitterBufferOptimizationFactor : 4; 
    U16 TestMode                        : 3;
    U16 CasRelay                        : 1;

    U16 IgnoreNetCasPackets				: 1;
    U16	IgnoreNetIbsPackets				: 1;
    U16	DetectPcmCas					: 1;
    U16 CallProgressTransferMode        : 3;
    U16	DetectNetIbsPackets				: 1;
    U16 DetectNetCasPackets				: 1;
                /*Word 7*/        
    U16 Reserve_FX2						: 1;
    U16 DetectDTMFRegister			    : 1;	
	U16 DetectCallProgress			    : 1;		
	U16 					            : 1;				
	U16 DetectUserDefinedTones		    : 1;			
	U16 					            : 2;			
    U16 TestFaxModem                    : 1;

	U16 DetectMFR1Register			    : 1;
	U16 DetectMFSS5Register		        : 1;			
	U16 DetectMFR2ForwardRegister	    : 1;
	U16 DetectMFR2BackwardRegister	    : 1;
	U16 Reserve_FX1   					: 1;
	U16 Reserve_FX15					: 1;
	U16 Reserve_FX4   					: 1;
	U16 Reserve_FX5   					: 1;
	            /*Word 8*/                                      
    U16 IbsLevel_minus_dbm              : 5;
    U16                                 : 1;
    U16 IbsDetectionRedirection         : 1;
    U16 UdpChecksumEnable               : 1;
                                          
    U16 FaxMaxRate                      : 4;
    U16                                 : 3;     
    U16 LongDtmfDetectionEnable			: 1;     
                /*Word 9*/                                      
    U16 PcmSidSize                      : 4;
    U16                                 : 2;
    U16 CallerIdOnHookService           : 1;
    U16 CallerIdOffHookService          : 1;
    
    U16 EchoCancelerLength              : 6;
    U16                                 : 2;
                /*Word 10*/
    U16 CasValueDuringIdle              : 4;
    U16                                 : 1;
	U16 ActiveChannelPattern			: 1;
    U16 CasModeDuringIdle               : 1;  
    U16 TriStateDuringIdle              : 1;
                                          
    U16 PcmValueDuringIdle              : 8;
                /*Word 11*/                          
    U16                                 : 8;
                                                      
/*  U16 BypassNumberOfBlocks_minus_1    : 4;*/
    U16 FaxAndCallerIdLevel             : 4;
    U16                                 : 4; 
                /*Word 12*/
    U16 V23TransferMethod               : 2;
    U16 V32TransferMethod               : 2;
    U16 V34TransferMethod               : 2;
    U16 V34FaxTransferMethod            : 2;

    U16 FaxTransferMethod               : 2;
    U16 Bell103TransferMethod           : 2;
    U16 V21TransferMethod               : 2;
    U16 V22TransferMethod               : 2;
                /*Word 13*/                                      
    U16                                 : 1;
    U16 FaxProtocol                     : 3;
    U16 TtyTransferMethod               : 2;
    U16                                 : 1;
    U16 CedTransferMode                 : 1;
 
    U16 CngRelayEnable					: 1;
    U16 FaxErrorCorrectionMode          : 1;
    U16 CallerIdStandard                : 4;
    U16                                 : 2;
                /*Word 14*/                                      
    U16                                 : 3;
	U16	AgcSlowAdaptationGainSlope		: 5;

	U16	AgcTargetEnergy					: 6;
	U16	AgcEnable						: 1;
	U16	AgcLocation						: 1;
                /*Word 15*/                            
    U16                                 : 7; 
    U16 FaxAndDataStatusAndEventsDisable: 1;

    U16                                 : 8; 
/*  U16 EncoderPcmAdpcmFrameDuration    : 8;*/
                /*Word 16*/
    U16 VoiceJitterBufferMinDelay_Msb   : 1;
    U16                                 : 6; 
	U16 VideoSynchronizationEnable      : 1;

    U16 VoiceJitterBufferMinDelay_Lsb   : 8;
/*  U16 DecoderPcmAdpcmFrameDuration    : 8;*/
				/* Word 17*/
	U16 VoiceJitterBufferMaxDelay_Msb	: 1;
	U16 AnswerDetectSensitivity         : 3;			  
	U16		                            : 1;
	U16 AnswerDetectMode                : 1;
	U16 AnswerDetectDirection			: 1;
	U16 AnswerDetectEnable              : 1;

	U16 VoiceJitterBufferMaxDelay_Lsb	: 8;
				/* Word 18*/
    U16                                 : 7;
    U16 EchoCancelerDebugPackets        : 1;
    U16                                 : 8;
    Tac49xPaddingForFutureNeed _;
    } Tac49xOpenOrUpdateChannelConfigurationPacket;           
           
typedef volatile struct
	{
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
	U16	T4FillBitRemoval				: 1;
	U16	T4MmrTranscoding				: 1;
	U16	JbigImageTranscoding			: 1;
    U16 RtpEncapsulation				: 1;
    U16 PacketizationPeriod             : 3;
    U16                                 : 1;
	
	U16	MaxDatagramSize_In4BytesStep	: 8;

	U16	T38Version						: 4;
    U16                                 : 4;

	U16	T30ControlDataRedundancyLevel	: 4;
	U16	ImageDataRedundancyLevel		: 3;
	U16	UdpErrorCorrectionMethod		: 1;

    U16                                 : 8;
    U16 NoOperationPeriod		        : 5;
    U16                                 : 3;

    Tac49xPaddingForFutureNeed _;
	} Tac49xT38ConfigurationPacket;
	
typedef volatile struct
	{
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
	U16	ConferenceChannelMate					: 8;
												
	U16	Mode									: 3;
	U16											: 5;
												
	U16	Conference1TransmitPayloadType			: 7;
	U16											: 1;
												
	U16	Conference2TransmitPayloadType			: 7;
	U16											: 1;
	
	U16	Conference1TransmitSequenceNumber_Msb	: 8;
	U16	Conference1TransmitSequenceNumber_Lsb	: 8;

	U16	Conference2TransmitSequenceNumber_Msb	: 8;
	U16	Conference2TransmitSequenceNumber_Lsb	: 8;

	U16 Conference1RtpSsrc_MswMsb				: 8;
	U16 Conference1RtpSsrc_MswLsb				: 8;
	U16 Conference1RtpSsrc_LswMsb				: 8;
	U16 Conference1RtpSsrc_LswLsb				: 8;

	U16 Conference2RtpSsrc_MswMsb				: 8;
	U16 Conference2RtpSsrc_MswLsb				: 8;
	U16 Conference2RtpSsrc_LswMsb				: 8;
	U16 Conference2RtpSsrc_LswLsb				: 8;
    Tac49xPaddingForFutureNeed _;
	} Tac49xActivateOrDeactivate3WayConferenceConfigurationPacket;

typedef volatile struct
	{
	U8 FieldOfRate  : 6;
	U8				: 1;
	U8 Valid		: 1;
	} Tac49xRfciStructBitField;

typedef volatile struct
	{
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
	U16										: 2;
	U16	ErrorTransmissionDisable			: 1;
	U16	ProtocolDataUnitType				: 1;
	U16	DeliveryOfErroneousSDUs				: 1;
	U16	SupportModeType						: 2;
	U16	Mode								: 1;
											
	U16										: 6;
	U16	G711CoderType						: 2;
											
	U16	LocalRate							: 5;
	U16										: 3;
											
	U16	RemoteRate							: 5;
	U16										: 3;
	Tac49xRfciStructBitField Rfci[18];

	U16										: 8;
	U16										: 8;
	U16										: 8;
	U16										: 8;

	U16										: 8;
											  
	U16	SupporetedUserPlaneProtocolVersion_Release99 : 1;
	U16	SupporetedUserPlaneProtocolVersion_Release4	 : 1;
	U16	SupporetedUserPlaneProtocolVersion_Release5	 : 1;
	U16												 : 5;
	
	U16 RateControlAcknowledgeTimeout		: 8;
	U16 InitializationAcknowledgeTimeout	: 8;
    Tac49xPaddingForFutureNeed _;
	} Tac49x3GppUserPlaneInitializationConfigurationPacket;

typedef volatile struct
	{
    U8			  Tfi0  : 5;
	U8				    : 3;
    U8			  Tfi1  : 5;
	U8				    : 3;
    U8			  Tfi2  : 5;
	U8				    : 3;
    U8            Valid : 1;
	U8				    : 7;
	} Tac49xTfiStructBitField;

typedef volatile struct
	{
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;

	U8				                                                                : 4;
    U8	                                BrokenConnectionActivationMode              : 1;
    U8				                    SilenceCompression                          : 1;
    U8				                    TransmitNoDataPacketsDuringSilence          : 1;
    U8                                  Crc                                         : 1;
	U8				                                                                : 8;

    U8							        BrokenConnectionEventTimeout_x20msec_Msb    : 8;
    U8							        BrokenConnectionEventTimeout_x20msec_Lsb    : 8;

    U8                                  LocalRate                                   : 5;
	U8				                                                                : 3;
    
	U8				                                                                : 8;
	U8				                                                                : 8;
	U8				                                                                : 8;
	Tac49xTfiStructBitField		Tfi[MAX_NUMBER_OF_3GPP_IUB_TFIS];
	} Tac49xActivateOrUpdate3GppIubChannelConfigurationPacket;

typedef volatile struct
    {
	U16				RedundancyLevel		    : 2;
	U16										: 1;
	U16				Rate				    : 5;
    
	U16				FrameLossRatioThreshold	: 8;

	U16										: 4;
	U16				FrameLossRatioHysteresis: 4;

	U16										: 8;
    }Tac49xRfc3267CodecMode;
typedef volatile struct
	{
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;

	U16				RedundancyLevel			            : 2;
	U16				AutomaticRateControlMode            : 1;
	U16				Coder					            : 2;
	U16				CrcEnable				            : 1;
	U16				ChangeModeNeighbor		            : 1;
	U16				OctetAlignedEnable		            : 1;
                                                        
	U16										            : 7;
	U16				TransmitNoDataPackets               : 1;
	                                                    
	U16										            : 8;
	U16				ModeChangePeriod		            : 8;
                                                        
	U16				LocalRate				            : 5;
	U16										            : 3;
                                                        
	U16				RemoteRate				            : 5;
	U16										            : 2;
	U16				RemoteRateEnable		            : 1;
                                                        
	U16										            : 8;
	U16										            : 8;
	U16										            : 8;
	U16										            : 8;
                                                        
	U16				DelayHandoutHysteresis_x1msec_Msb   : 1;
	U16										            : 2;
	U16				NumberOfCodecsPolicies	            : 5;
                                                        
	U16				DelayHandoutHysteresis_x1msec_Lsb   : 8;
                                                        
	U16				DelayHandoutThreshold_x1msec_Msb    : 8;
	U16				DelayHandoutThreshold_x1msec_Lsb    : 8;
    Tac49xRfc3267CodecMode aCodecPolicies[MAX_NUMBER_OF_RFC_3267_CODECS];
	} Tac49xRfc3267AmrInitializationConfigurationPacket;

typedef volatile struct
	{
                    /*4*/						
	U16											: 1;
	U16	NumberOfRtpTimeStampWrapAround			: 2;
	U16	AntiReplayEnable						: 1;
	U16	RtcpAuthentication						: 1;
	U16	RtcpEncryption							: 1;
	U16	RtpAuthentication						: 1;
	U16	RtpEncryption							: 1;
												
	U16											: 8;
                    /*5*/						
	U16	RtpMessageAuthenticationCodeKeySize		: 8;
	U16	RtpCipherKeySize		   				: 8;
                    /*6*/						
	U16			   								: 4;
	U16	RtpAuthenticationMode		   			: 4;
												
	U16	RtpDigestSize	  		   				: 8;
                    /*7*/						
	U16	RtcpMessageAuthenticationCodeKeySize	: 8;
	U16	RtcpCipherKeySize	      				: 8;
                    /*8*/						
	U16	                         	   			: 4;
	U16	RtcpAuthenticationMode     	   			: 4;
												
	U16	RtcpDigestSize			   				: 8;
                    /*9-16*/
    U8  RtpCipherKey[							MAX_KEY_SIZE__RTP_CIPHER];	 
					/*17-18*/
    U16 RtpTimeStamp_MswMsb   					: 8;
    U16 RtpTimeStamp_MswLsb   					: 8;
    U16 RtpTimeStamp_LswMsb   					: 8;
    U16 RtpTimeStamp_LswLsb   					: 8;
                    /*19-26*/
	U8	RtpInitializationKey[					MAX_KEY_SIZE__RTP_INITIALIZATION];	  
                    /*27-113*/					
	U8	RtpMessageAuthenticationCodeKey[		MAX_KEY_SIZE__RTP_MESSAGE_AUTHENTICATION_CODE];	  
                    /*114-121*/					
	U8	RtcpCipherKey[							MAX_KEY_SIZE__RTCP_CIPHER];
                    /*122-131*/					
	U8	RtcpMessageAuthenticationCodeKey[		MAX_KEY_SIZE__RTCP_MESSAGE_AUTHENTICATION_CODE];
                    /*132-133*/					
    U16 RtcpSequenceNumber_MswMsb   			: 8;
    U16 RtcpSequenceNumber_MswLsb   			: 8;
    U16 RtcpSequenceNumber_LswMsb   			: 8;
    U16 RtcpSequenceNumber_LswLsb   			: 8;
    } Tac49xPacketCableProtectionConfigurationPayload;

typedef volatile struct
	{
                    /*4*/						
	U16											: 3;
	U16	AntiReplayEnable						: 1;
	U16	RtcpAuthentication						: 1;
	U16	RtcpEncryption							: 1;
	U16	RtpAuthentication						: 1;
	U16	RtpEncryption							: 1;

	U16	KeyDerivationRatePower					: 6;
	U16											: 2;
                    /*5*/						
	U16	RtpMessageAuthenticationCodeKeySize		: 8;
	U16	RtpCipherKeySize		   				: 8;
                    /*6*/						
	U16	RtpEncryptionMode						: 4;
	U16	RtpAuthenticationMode		   			: 4;
												
	U16	RtpDigestSize	  		   				: 8;
                    /*7*/						
	U16	RtcpMessageAuthenticationCodeKeySize	: 8;
	U16	RtcpCipherKeySize	      				: 8;
                    /*8*/						
	U16	RtcpEncryptionMode						: 4;
	U16	RtcpAuthenticationMode     	   			: 4;
												
	U16	RtcpDigestSize			   				: 8;
                    /*9-10*/
	U16	MasterKeySize			   				: 8;
	U16	MasterSaltSize			   				: 8;
	U16	SessionSaltSize			   				: 8;
	U16							   				: 8;
                    /*11*/
	U16	RtpInitSequenceNumber_Msb				: 8;
	U16	RtpInitSequenceNumber_Lsb				: 8;
                    /*12-13*/
	U16 RtpInitRoc_MswMsb						: 8;
	U16 RtpInitRoc_MswLsb						: 8;
	U16 RtpInitRoc_LswMsb						: 8;
	U16 RtpInitRoc_LswLsb						: 8;
                    /*14-15*/
	U16 RtcpInitIndex_MswMsb					: 8;
	U16 RtcpInitIndex_MswLsb					: 8;
	U16 RtcpInitIndex_LswMsb					: 8;
	U16 RtcpInitIndex_LswLsb					: 8;
                    /*16-19*/
	U16											: 8;
	U16											: 8;

	U16											: 8;
	U16											: 8;

	U16											: 8;
	U16											: 8;

	U16											: 8;
	U16											: 8;
					/*20-27*/
    U8  MasterKey[MAX_KEY_SIZE__MASTER];	 
					/*28-34*/
    U8  MasterSalt[MAX_KEY_SIZE__MASTER_SALT];	 
					/* 35 */
	U8											: 8;

	U8 MasterKeyIndexSize						: 3;
	U8											: 5;
					/*36-37*/
	U8 MasterKeyIndex[MAX_MASTER_KEY_INDEX_SIZE];

    } Tac49xSrtpProtectionConfigurationPayload;

typedef volatile struct
	{
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;

	U16											: 7;
	U16	Direction								: 1;
												
	U16	ProtectionMode 							: 3;
	U16											: 5;
	union
		{
		Tac49xPacketCableProtectionConfigurationPayload PacketCable;
		Tac49xSrtpProtectionConfigurationPayload		Srtp;	
		} u;
	} Tac49xMediaProtectionConfigurationPacket;

#if (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
    )
typedef volatile struct
	{
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
                    /*3*/
    U16	SpeakerGain                             : 2;
    U16	BiasPinOutputVoltage                    : 1;
    U16	AdcDitheringCircuit                     : 1;
    U16			                                : 2;
    U16	DigitalLoopback							: 1;
    U16	AnalogLoopback							: 1;

    U16	MuteSpeaker		                        : 1;
    U16	MuteLineOutput		                    : 1;
    U16	MuteHeadset		                        : 1;
    U16	MuteHandset		                        : 1;
    U16	SamplingRate                            : 1;
    U16	BandPassFilter                          : 1;
    U16			                                : 2;
                    /*4*/ 
    U16	HeadsetInputSelect	                    : 1;
    U16	HandsetInputSelect	                    : 1;
    U16	MicrophoneInputSelect                   : 1;
    U16	LineInputSelect		                    : 1;
    U16	CallerIdInputSelect	                    : 1;
    U16	HandsetInputToOutput	                : 1;
    U16	HeadsetInputToOutput	                : 1;
    U16			                                : 1;            
                    
    U16	DigitalSidetoneGain		                : 3;            
    U16	AnalogSidetoneGain		                : 3;            
    U16			                                : 2;            
                    /*5*/                       
    U16			                                : 8;

    U16	HeadsetOutputSelect		                : 1;            
    U16	HandsetOutputSelect		                : 1;            
    U16	LineOutputSelect		                : 1;            
    U16	SpeakerSelect		                    : 1;            
    U16	AnalogSidetoneOutputSelectForHandset	: 1;            
    U16	AnalogSidetoneOutputSelectForHeadset	: 1;            
    U16			                                : 2; 
                    /*6*/
    U16	DacOutputPgaGain	                    : 5;
    U16			                                : 3;
                                                
    U16	AdcInputPgaGain		                    : 5;
    U16			                                : 3;
    } Tac49xCodecConfigurationPacket;

typedef volatile struct
	{
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;

	U16											: 4;
	U16	SupportedMode							: 4;

	U16	CommandIndex							: 4;
	U16											: 4;

	U16	SoftNlp									: 1;
	U16	ResidualOverwriteByInput				: 1;
	U16	SaturationAttenuator					: 1;
	U16	IntensiveMode							: 1;
	U16	Decimation								: 1;
	U16	HighPassFilter							: 1;
	U16											: 2;

	U16											: 3;
	U16	AntiHowlingProcessor					: 1;
	U16	HalfDuplexMode							: 2;
	U16	ReinitAfterToneSignal					: 1;
	U16	Cng										: 1;

	U16	TailLength								: 2;
	U16	MaxActiveFilterLength					: 2;
	U16	EchoReturnLoss							: 3;
	U16											: 1;
	
	U16	CoefficientsThreshold					: 6;
	U16											: 2;

	U16	NlpActivationThreshold					: 6;
	U16											: 2;
	
	U16	SaturationThreshold						: 4;
	U16											: 4;
	} Tac49xAcousticEchoCancelerConfigurationPacket;
#endif/*((AC49X_DEVICE_TYPE == AC494_DEVICE) || (AC49X_DEVICE_TYPE == AC495_DEVICE) || (AC49X_DEVICE_TYPE == AC496_DEVICE) || (AC49X_DEVICE_TYPE == AC496E_DEVICE) || (AC49X_DEVICE_TYPE == AC497_DEVICE))*/

typedef volatile struct
	{
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;

	U16 LocalRateModeControlledByRemoteSideEnable	: 1;
	U16 RemoteRateMode								: 3;
	U16 LocalRateMode								: 3;
	U16 											: 1;

	U16												: 5;
	U16	PacketFormat								: 2;
	U16	SilenceCompressionExtensionModeEnable		: 1;

	U16	LocalMinRate								: 4;
	U16 LocalMaxRate								: 4;
	U16												: 8;

	U16	DtxMin_Msb									: 7;
	U16												: 1;
	U16	DtxMin_Lsb									: 8;

	U16	DtxMax_Msb									: 7;
	U16												: 1;
	U16	DtxMax_Lsb									: 8;

	U16	NoiseSuppressionFilterEnable				: 1;
	U16												: 7;
	U16	Hangover									: 8;
	} Tac49xRfc3558InitializationConfigurationPacket;

typedef volatile struct
    {
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;

	U16 LocalBitRate				                : 5;
	U16 AutomaticLocalRateControlMode	            : 1;
	U16												: 2;

	U16 MaxRemoteBitRate			                : 5;
	U16												: 3;

	U16												: 8;
	U16	MaxSessionBitRate							: 5;
	U16												: 3;
    } Tac49xG729EvInitializationConfigurationPacket;

typedef volatile struct
    {
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
		/* 3 */
	U16												: 2;
	U16 LowVoiceQualityAlertSetting					: 2;
	U16 DelayAlertSetting							: 2;
	U16 BurstAlertSetting							: 2;

	U16												: 8;
		/* 4 */
	U16												: 8;
	U16	MaximumBurstLossRateThreshold				: 8;
		/* 5 */
	U16 MaximumBurstDurationThreshold_Msb			: 8;
	U16 MaximumBurstDurationThreshold_Lsb			: 8;
		/* 6 */
	U16 MaximumOneWayDelayThreshold_Msb				: 8;
	U16 MaximumOneWayDelayThreshold_Lsb				: 8;
		/* 7 */
	U16 LowVoiceQualityRValueThreshold_Msb			: 8;
	U16 LowVoiceQualityRValueThreshold_Lsb			: 8;
	} Tac49xRtcpXrConfigurationPacket;

typedef volatile struct
    {
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
		/* 3 */
	U16 RtpTimeStampResolution 						: 2;
	U16						 						: 6;
		/* 4 */
	U16 TransmitMode 								: 3;
	U16				 								: 5;
	} Tac49xUemClipInitializationPacket;

typedef volatile struct 
	{
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
		/* 3 */
	U16	MaxPayloadSizeForSprtTransportChannel2		: 8;
	U16	MaxPayloadSizeForSprtTransportChannel0		: 8;
		/* 4 */
	U16	MaxPayloadSizeForSprtTransportChannel3		: 8;
	U16	MaxWindowSizeForSprtTransportChannel2		: 8;
		/* 5 */
	U16 RxPayloadTypeOfSprtPackets					: 8;
	U16 TxPayloadTypeOfSprtPackets					: 8;
		/* 6 */
	U16												: 6;
	U16 SprtPacketsTransmitProtocol					: 1;
	U16 SprtPacketsReceiveProtocol					: 1;

	U16												: 8;
	} Tac49xActivateOrUpdateDataRelayConfigurationPacket;

typedef volatile struct
    {
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
		/* 3 */
	U8											: 4;
	U8 ModemTransmissionPower 					: 4;

	U8 V32ModulationTypeEnable					: 1;
	U8 V34ModulationTypeEnable					: 1;
	U8											: 1;
	U8 ModemDiagnosticsEnable					: 1;
	U8 PcmLevelOfTransmissionOfPhase1			: 2;
	U8											: 2;
		/* 4 */
	U8 MaxTxBitRate								: 5;
	U8											: 3;

	U8 MinTxBitRate								: 5;
	U8											: 3;
		/* 5 */
	U8 MaxRxBitRate								: 5;
	U8											: 3;

	U8 MinRxBitRate								: 5;
	U8											: 3;
	} Tac49xAdvancedDataRelayConfigurationPacket;

            /********************************************/
            /* Command, Status & Network Packet section */
            /********************************************/
typedef volatile struct
	{
	U16	LocalRate			: 5;
	U16						: 1;
	U16	RemoteRateEnable	: 1;
	U16	LocalRateEnable		: 1;

	U16						: 3;
	U16	RemoteRate			: 5;
    Tac49xPaddingForFutureNeed _;
	} Tac49xSet3GppUserPlaneRateControlPayload;

typedef volatile struct
	{
	U16	LocalRate			: 5;
	U16						: 1;
	U16	RemoteRateEnable	: 1;
	U16	LocalRateEnable		: 1;

	U16	RedundancyLevel		: 2;
	U16						: 1;
	U16	RemoteRate			: 5;
    Tac49xPaddingForFutureNeed _;
	} Tac49xSetRfc3267AmrRateControlPayload;

typedef volatile struct
	{
	U16									: 7;
    U16 ChannelType                     : 1; 

	U16	ParameterIndex					: 8;

	U16	ParameterValue_MswMsb			: 8;
	U16	ParameterValue_MswLsb			: 8;
	U16	ParameterValue_LswMsb			: 8;
	U16	ParameterValue_LswLsb			: 8;
    Tac49xPaddingForFutureNeed _;
	} Tac49xSetRtcpParametersPayload;

typedef volatile struct                      
    {                                   
    U16    Pause                        : 1;
    U16                                 : 7;
                                        
    U16    SystemCode                   : 4;
    U16    Digit                        : 4;
    } Tac49xIbsStringSignal;            
                                        
typedef volatile struct                 
    {                                   
    U16                                 : 5;
    U16 GeneratorRedirection            : 3;
                                          
    U16 NumberOfDigits                  : 6;
    U16                                 : 2;
                                          
    U16 OnDuration_MswMsb               : 8;
    U16 OnDuration_MswLsb               : 8;
    U16 OnDuration_LswMsb               : 8;
    U16 OnDuration_LswLsb               : 8;
    U16 OffDuration_MswMsb              : 8;
    U16 OffDuration_MswLsb              : 8;
    U16 OffDuration_LswMsb              : 8;
    U16 OffDuration_LswLsb              : 8;
    U16 PauseDuration_MswMsb            : 8;
    U16 PauseDuration_MswLsb            : 8;
    U16 PauseDuration_LswMsb            : 8;
    U16 PauseDuration_LswLsb            : 8;
    Tac49xIbsStringSignal Signal[MAX_IBS_STRING_SIGNALS];
    } Tac49xIbsStringCommandPayload;          

typedef struct 
	{
    U16 SignalLevel_minus_dbm       : 6;
    U16                             : 2;
                                    
    U16 SystemCode                  : 4; 
    U16 Digit                       : 4; 

    U16 SignalDuration_MswMsb       : 8;
    U16 SignalDuration_MswLsb       : 8;
    U16 SignalDuration_LswMsb       : 8;
    U16 SignalDuration_LswLsb       : 8;
    } TacExtendedIbsStringCommand_Format0_SignalPayload;
typedef struct 
	{
    TacExtendedIbsStringCommand_Format0_SignalPayload Signal[MAX_NUMBER_OF_EXTENDED_IBS_DIGITS__FORMAT_0];
    } TacExtendedIbsStringCommand_Format0_Payload;

typedef struct 
	{
    U8 SystemCode                 : 4; 
    U8 Digit                      : 4; 
    } TacExtendedIbsStringCommand_Format1_SignalPayload;
typedef struct 
	{
    U16 OnDuration_MswMsb       : 4;
    U16                         : 4;
    U16 OnDuration_MswLsb       : 8;
    U16 OnDuration_LswMsb       : 8;
    U16 OnDuration_LswLsb       : 8;

    U16 OffDuration_MswMsb      : 4;
    U16                         : 4;
    U16 OffDuration_MswLsb      : 8;
    U16 OffDuration_LswMsb      : 8;
    U16 OffDuration_LswLsb      : 8;

    U16 PauseDuration_MswMsb    : 4;
    U16                         : 4;
    U16 PauseDuration_MswLsb    : 8;
    U16 PauseDuration_LswMsb    : 8;
    U16 PauseDuration_LswLsb    : 8;
    TacExtendedIbsStringCommand_Format1_SignalPayload Signal[MAX_NUMBER_OF_EXTENDED_IBS_DIGITS__FORMAT_1];
    } TacExtendedIbsStringCommand_Format1_Payload;

typedef struct
    {
    U16								: 2;
    U16 Format      				: 1;
    U16             				: 1;
    U16 Repeat						: 1;
	U16 GeneratorRedirection	    : 3;

    U16 NumberOfDigits				: 6;
	U16								: 2;
    
    U16 TotalDuration_MswMsb        : 8;
    U16 TotalDuration_MswLsb        : 8;
    U16 TotalDuration_LswMsb        : 8;
    U16 TotalDuration_LswLsb        : 8;

	U16 UserDefinedToneA_Msb		: 4;
	U16								: 4;
	U16 UserDefinedToneA_Lsb		: 8;

	U16 UserDefinedToneB_Msb		: 4;
	U16								: 4;
	U16 UserDefinedToneB_Lsb		: 8;

	U16 UserDefinedToneC_Msb		: 4;
	U16								: 4;
	U16 UserDefinedToneC_Lsb		: 8;

    U16	SignalLevel_minus_dbm	    : 6;
    U16								: 2;

    U16 AmFactor					: 6;
    U16								: 2;
    union
        {
        TacExtendedIbsStringCommand_Format0_Payload Format0;
        TacExtendedIbsStringCommand_Format1_Payload Format1;
        }u;
    } Tac49xExtendedIbsStringCommandPayload;

typedef volatile struct
    {
    U16 Direction                       : 1;
    U16                                 : 7;

    U16 Operation                       : 4;
    U16 CMessageFilter                  : 1;
    U16 CNotchFilter                    : 1; 
    U16 MeasurementUnits                : 2;

    U16 ReferenceEnergy                 : 5;
    U16 TriggerType                     : 2;
    U16                                 : 1;
                                             
    U16 TriggerToneIbsSystemCode        : 4;
    U16 TriggerToneDigit                : 4;
                                             
    U16 MeasurementDuration_Msb         : 3;
    U16                                 : 5;

    U16 MeasurementDuration_Lsb         : 8;
    } Tac49xTrunkTestingMeasurementCommandPayload;

typedef volatile struct
    {
    U16                                 : 7;
    U16 CasGenerationDirection          : 1;
                                        
    U16 D                               : 1;
    U16 C                               : 1;
    U16 B                               : 1;
    U16 A                               : 1;
    U16 ReversalsB                      : 1;
    U16 ReversalsA                      : 1;
    U16                                 : 2;
    Tac49xPaddingForFutureNeed _;
    } Tac49xCasGenerateCommandPayload;

typedef volatile struct
    {
	U16                                 : 6;
	U16  Repeat                         : 1;
	U16  Direction                      : 1;

	U16  NumberOfPatterns               : 6;
	U16                                 : 2;
	
	U16 TotalDuration_MswMsb			: 8;
	U16 TotalDuration_MswLsb			: 8;
	U16 TotalDuration_LswMsb			: 8;
	U16 TotalDuration_LswLsb			: 8;

	struct
		{
		U16                             : 2;
		U16 D                           : 1;
		U16 C                           : 1;
		U16 B                           : 1;
		U16 A                           : 1;
		U16 ReversalsB                  : 1;
		U16 ReversalsA                  : 1;

		U16 Duration_MswLsb				: 8;
		U16 Duration_LswMsb				: 8;
		U16 Duration_LswLsb				: 8;
		} Pattern[MAX_CAS_STRING_LENGTH];
    Tac49xPaddingForFutureNeed _;
    } Tac49xCasStringCommandPayload;

/*                                    
typedef volatile struct                 
    {                                   
    U16 Fourth                          : 4;
    U16 Third                           : 4;
                                            
    U16 Second                          : 4;
    U16 First                           : 4;
    } Tac49xQuadDigit;                  
                                        
typedef volatile struct                 
    {                                   
    U16                                 : 8;
                                        
    U16                                 : 2;
    U16 NumberOfDigits                  : 6;

    U16                                 : 4;
    U16 FirstAbcdStateDuration_MswMsb   : 4;

    U16 FirstAbcdStateDuration_MswLsb   : 8;
    U16 FirstAbcdStateDuration_LswMsb   : 8;
    U16 FirstAbcdStateDuration_LswLsb   : 8;

    U16                                 : 4;
    U16 SecondAbcdStateDuration_MswMsb  : 4;

    U16 SecondAbcdStateDuration_MswLsb  : 8;
    U16 SecondAbcdStateDuration_LswMsb  : 8;
    U16 SecondAbcdStateDuration_LswLsb  : 8;

    U16                                 : 4;
    U16 InterDigitDuration_MswMsb       : 4;

    U16 InterDigitDuration_MswLsb       : 8;
    U16 InterDigitDuration_LswMsb       : 8;
    U16 InterDigitDuration_LswLsb       : 8;

    U16                                 : 8;

    U16 SecondAbcdState                 : 4;
    U16 FirstAbcdState                  : 4;
    Tac49xQuadDigit QuadDigit[MAX_NUMBER_OF_ABCD_PULSE_DIGITS>>2];
    } Tac49xCasStringCommandPayload;
*/
/*
typedef volatile struct
    {
    U16                                 : 6;
    U16 Command                         : 2;
                                        
    U16 NumberOfBlocks                  : 8;
    Tac49xPaddingForFutureNeed _;
    } Tac49xPlaybackCommandPayload;
*/

typedef volatile struct
    {
	U16									: 8;
	U16									: 8;

    U16 AvailableBufferSpace_msec_Msb   : 7;
    U16                                 : 1;
    U16 AvailableBufferSpace_msec_Lsb   : 8;

	U16									: 8;
	U16									: 8;
    } Tac49xPlaybackDspCommand;

typedef volatile struct
    {
	U16									: 8;
	U16									: 8;

	U16									: 8;
	U16 SilenceDuration_msec_FirstByte	: 8;

    U16 SilenceDuration_msec_SecondByte	: 8;
    U16 SilenceDuration_msec_ThirdByte	: 8;
    } Tac49xPlaybackHostCommand;

typedef volatile struct
    {
    U16 Opcode							: 3;
    U16                                 : 4;
	U16 Endianness						: 1;
                                        
    U16 Coder							: 8;
	union
		{
		Tac49xPlaybackDspCommand  Dsp; 
		Tac49xPlaybackHostCommand Host;
		} u;
    Tac49xPaddingForFutureNeed _;
    } Tac49xPlaybackCommandPayload;

typedef volatile struct
    {
	union
		{
		Tac49xPlaybackCommandPayload  Command;
        U8							  Voice[MAX_PAYLOAD_SIZE];
		} u;
    } Tac49xPlaybackPayload;

typedef volatile struct
    {
    U16 SequenceNumber_MswMsb : 8;  /*!< 32-bit sequence number*/
    U16 SequenceNumber_MswLsb : 8;  /*!< 32-bit sequence number*/
    U16 SequenceNumber_LswMsb : 8;  /*!< 32-bit sequence number*/
    U16 SequenceNumber_LswLsb : 8;  /*!< 32-bit sequence number*/

    U16 PayloadFormat					: 2;
    U16                                 : 6;
    
    U16 PayloadType						: 4;
    U16                                 : 4;

    U8	Data[MAX_PAYLOAD_SIZE];
    } Tac49xEchoCancelerDebugPayload;

typedef volatile struct
    {
    U16                                 : 8;

    U16 Module							: 3;
    U16                                 : 5;
    union
        {
        Tac49xEchoCancelerDebugPayload EchoCanceler;
        } u;
    } Tac49xDebugPayload;

typedef volatile struct
	{
    U16 Type							: 3;
    U16                                 : 5;
                                        
    U16 Coder							: 8;
	} Tac49xRecordCommandPayload;

typedef volatile struct
	{
	U8  Voice[MAX_PAYLOAD_SIZE];
	} Tac49xRecordEventPayload;

typedef volatile struct
    {
	union
		{
		Tac49xRecordCommandPayload  Command;
		Tac49xRecordEventPayload	Event;
		} u;
    } Tac49xRecordPayload;

typedef volatile struct
	{         
	U16 InjectedPattern					: 3;
	U16									: 4;
	U16 Direction						: 1;
										
	U16 Rate							: 2;
	U16									: 3;
	U16 Action							: 3;

	U16 UserDefinedPattern_MswLsb		: 8;
										
	U16 UserDefinedPatternSize			: 5;
	U16									: 3;

	U16 UserDefinedPattern_LswMsb	    : 8;
                                       
	U16 UserDefinedPattern_LswLsb	    : 8;

	U16 ErrorBitsInsertionNumber_Msb	: 2;
	U16 	                            : 1;
	U16 ErrorBitsInsertionInterval	    : 3;
	U16 ErrorBitsInsertionTimeMode	    : 1;
	U16 ErrorBitsInsertionPatternMode	: 1;

	U16 ErrorBitsInsertionNumber_Lsb	: 8;

	U16 TestDuration_MswMsb				: 7;
	U16 	                            : 1;
	U16 TestDuration_MswLsb				: 8;
	U16 TestDuration_LswMsb				: 8;
	U16 TestDuration_LswLsb				: 8;
    } Tac49xBerTestCommandPayload;

typedef volatile struct
    {
	U16	                            : 6;
    U16 StatusRequest				: 1;
    U16 ClearStatistics				: 1;

	U16	                            : 8;
    } Tac49xUtopiaStatusRequestPayload;

typedef volatile struct
    {
	U16	                            : 8;

    U16 InfoType					: 4;
	U16	                            : 4;
    } Tac49xGetMediaProtectiontInfoPayload;

typedef volatile struct
    {
	U16	                            : 8;
	U16	                            : 8;
	U16	                            : 8;
	U16	                            : 8;

	U16	                            : 8;
	U16	MessageType					: 8;

	U16	DataSize_Msb				: 8;
	U16	DataSize_Lsb				: 8;

    U8	Data[MAX_SS7_DATA_SIZE];
    } Tac49xSs7Mtp1CommandPayload;

typedef volatile struct
    {
    U16					            : 8;
    U16					            : 8;
    U16					            : 8;
    U16					            : 8;

    U16					            : 8;
    U16 MessageType					: 8;

	U16 MessageSize_Msb				: 8;
	U16 MessageSize_Lsb				: 8;
    U8  Message[MAX_HDLC_MESSAGE_LENGTH];
    } Tac49xHdlcFramerMessageCommandPayload;

typedef volatile struct
	{
	U16 LocalRateModeControlledByRemoteSideEnable	: 1;
	U16 RemoteRateMode								: 3;
	U16 LocalRateMode								: 3;
	U16												: 1;

	U16												: 8;
													  
	U16	LocalMinRate								: 4;
	U16 LocalMaxRate								: 4;
	U16												: 8;
	} Tac49xSetRfc3558RateModePayload;

typedef volatile struct
	{
	U16												: 8;
	U16	CommandType									: 3;
	U16												: 5;
	
	U16												: 6;
	U16 RxTdmPortType								: 2;
	U16 RxChannelId									: 8;

	U16												: 6;
	U16 TxTdmPortType								: 2;
	U16 TxChannelId									: 8;
	} Tac49xTimeSlotManipulationCommandPayload;

typedef volatile struct
	{
	U16												: 8;
	U16 ModuleType									: 8;
	} Tac49xModuleStatusRequestPayload;

typedef volatile struct
	{
	U16	SubType										: 5;
	U16												: 2;
	U16 PacketFormat								: 1;
	U16												: 8;

	U16	DataLength_In4Bytes_Msb						: 8;
	U16	DataLength_In4Bytes_Lsb						: 8;

	U16												: 8;
	U16												: 8;
	U16												: 8;
	U16												: 8;
	
	U8  Name[RTCP_APP_NAME_LENGTH];
	U8  Data[RTCP_APP_DATA_LENGTH];
	} Tac49xRtcpAppCommandPayload;

typedef volatile struct
	{
	U16												: 8;
	U16												: 8;
	U16												: 8;
	U16												: 8;

	U16         									: 8;
	U16 MessageType									: 8;

	U16 MessageSize_Msb								: 8;
	U16 MessageSize_Lsb								: 8;

    U8  Message[MAX_DUA_MESSAGE_LENGTH];
	} Tac49xDuaFramerCommandPayload;

typedef volatile struct
	{
	U16												: 8;
	U16												: 8;

	U16	E24											: 1;
	U16	E25											: 1;
	U16	E26											: 1;
	U16	E27											: 1;
	U16	E28											: 1;
	U16	E29											: 1;
	U16	E30											: 1;
	U16	E31											: 1;
                                                      
	U16	BerReport									: 1;
	U16	UtopiaStatus								: 1;
	U16	E18											: 1;
	U16 ModuleStatus								: 1;
	U16	E20											: 1;
	U16	E21											: 1;
	U16	E22											: 1;
	U16	E23											: 1;

	U16	PacketizerStatus							: 1;
	U16	E9											: 1;
	U16	DeviceStatus								: 1;
	U16	E11											: 1;
	U16	FaxAndDataStatus							: 1;
	U16	E13											: 1;
	U16	UtopiaTestError								: 1;
	U16	TrunkTestingsEvent							: 1;

	U16	SystemError									: 1;
	U16	GeneralEvents								: 1;
	U16	IbsEvent									: 1;
	U16	CasEvent									: 1;
	U16	HdlcFramerStatus							: 1;
	U16	FaxRelayStatus								: 1;
	U16	JitterBufferStatus							: 1;
	U16	CallerId									: 1;
	} Tac49xEventsControlCommandPayload;

typedef volatile struct
    {
    U16 Time_x1Sec_MswMsb                            : 8;
    U16 Time_x1Sec_MswLsb                            : 8;
    U16 Time_x1Sec_LswMsb                            : 8;
    U16 Time_x1Sec_LswLsb                            : 8;
    U16 Time_x1SecFraction_MswMsb                    : 8;
    U16 Time_x1SecFraction_MswLsb                    : 8;
    U16 Time_x1SecFraction_LswMsb                    : 8;
    U16 Time_x1SecFraction_LswLsb                    : 8;
    } Tac49xNtpTimeStampCommandPayload;

typedef volatile struct
    {
    U16 TimeStamp_MswMsb            : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_MswLsb            : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswMsb            : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswLsb            : 8;  /*!< 32-bit time stamp*/

    U16 RemoteNtpTime_x1msec_MswMsb    : 8;
    U16 RemoteNtpTime_x1msec_MswLsb    : 8;
    U16 RemoteNtpTime_x1msec_LswMsb    : 8;
    U16 RemoteNtpTime_x1msec_LswLsb    : 8;

    U16 LocalPlayoutNtpTime_x1msec_MswMsb    : 8;
    U16 LocalPlayoutNtpTime_x1msec_MswLsb    : 8;
    U16 LocalPlayoutNtpTime_x1msec_LswMsb    : 8;
    U16 LocalPlayoutNtpTime_x1msec_LswLsb    : 8;

    U16 MaxLocalPlayoutNtpTime_x1msec_MswMsb    : 8;
    U16 MaxLocalPlayoutNtpTime_x1msec_MswLsb    : 8;
    U16 MaxLocalPlayoutNtpTime_x1msec_LswMsb    : 8;
    U16 MaxLocalPlayoutNtpTime_x1msec_LswLsb    : 8;
    } Tac49xVideoAudioSyncMsgPayload;

typedef volatile struct
	{
	U16 LocalBitRate				                : 5;
    U16                                             : 1;
    U16 MaxRemoteBitRateFieldEnable                 : 1;
    U16 LocalBitRateFieldEnable                     : 1;
    
	U16 MaxRemoteBitRate			                : 5;
	U16												: 3;
	} Tac49xG729EvRateControlCommandPayload;

typedef volatile struct
	{
		/* 3 */
	U16							                   : 8;

    U16 Operation					               : 2;
	U16							                   : 6;
		/* 4 */
	U16							                   : 8;

    U16 AnswerDetectSensitivity					   : 3;
	U16							                   : 5;
		/* 5 */
	U16 DecisionParameter8_Msb					   : 8;
	U16 DecisionParameter8_Lsb					   : 8;
		/* 6 */
	U16 DecisionParameter1_Msb					   : 8;
	U16 DecisionParameter1_Lsb					   : 8;
		/* 7 */
	U16 DecisionParameter2_Msb					   : 8;
	U16 DecisionParameter2_Lsb					   : 8;
		/* 8 */
	U16 DecisionParameter3_Msb					   : 7;
	U16											   : 1;

	U16 DecisionParameter3_Lsb					   : 8;
		/* 9 */
	U16 MaxGreetingTime_Msb						   : 7;
	U16											   : 1;

	U16 MaxGreetingTime_Lsb						   : 8;
		/* 10 */
	U16 SilentCallTimeout_Msb					   : 7;
	U16											   : 1;

	U16 SilentCallTimeout_Lsb					   : 8;
		/* 11 */
	U16 PreWorkDelay_Msb						   : 7;
	U16											   : 1;

	U16 PreWorkDelay_Lsb						   : 8;
		/* 12 */
	U16 DecisionTimeout_Msb						   : 7;
	U16											   : 1;

	U16 DecisionTimeout_Lsb						   : 8;
	} Tac49xAmdCommandPayload;

typedef volatile struct
	{
		/* 3 */
	U16												: 7;
	U16 SwitchToFax									: 1;

	U16												: 8;
	} Tac49xActivateFaxRelayPayload;

typedef volatile struct                     
    {
		/* 3 */
	U16				 						: 1;
    U16 InitiatingFlag 						: 1;
	U16				 						: 6;

	U16 ReasonIdentifierCode				: 8;
		/* 4 */
	U16 RtpTxSequenceNumber_Msb				: 8;
	U16 RtpTxSequenceNumber_Lsb				: 8;
		/* 5-6 */
	U16 RtpTxTimeStamp_MswMsb				: 8;
	U16 RtpTxTimeStamp_MswLsb				: 8;
	U16 RtpTxTimeStamp_LswMsb				: 8;
	U16 RtpTxTimeStamp_LswLsb				: 8;
		/* 7-8 */
	U16 RtpTxSsrc_MswMsb					: 8;
	U16 RtpTxSsrc_MswLsb					: 8;
	U16 RtpTxSsrc_LswMsb					: 8;
	U16 RtpTxSsrc_LswLsb					: 8;
	} Tac49xSwitchToModemCommandPayload;

typedef volatile struct                     
    {
		/* 3 */
	U16				 						: 8;
   
	U16 ReasonIdentifierCode				: 2;
	U16				 						: 6;
		/* 4 */
	U16 RtpTxSequenceNumber_Msb				: 8;
	U16 RtpTxSequenceNumber_Lsb				: 8;
		/* 5-6 */
	U16 RtpTxTimeStamp_MswMsb				: 8;
	U16 RtpTxTimeStamp_MswLsb				: 8;
	U16 RtpTxTimeStamp_LswMsb				: 8;
	U16 RtpTxTimeStamp_LswLsb				: 8;
		/* 7-8 */
	U16 RtpTxSsrc_MswMsb					: 8;
	U16 RtpTxSsrc_MswLsb					: 8;
	U16 RtpTxSsrc_LswMsb					: 8;
	U16 RtpTxSsrc_LswLsb					: 8;
	} Tac49xModemRelayEndedCommandPayload;

typedef volatile struct                     
    {
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
		/* 3 */
	U16 RtpTimestampResolution			 	: 2;
	U16				 						: 6;

	U16 TransmitMode			 			: 3;										  
	U16				 						: 5;
	} Tac49xG7111InitializationPacket;

typedef volatile struct                     
    {
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
		/* 3 */
	U16										: 7;
	U16 VariableRateSupport			 		: 1;

	U16 ForwardErrorCorrectionMode			: 2;
	U16										: 2;
	U16 JitterBufferMode					: 2;
	U16										: 2;
	} Tac49xMicrosoftRtaInitializationPacket;

/************************************************************************/
/*      4 - Way Conference                                              */
/************************************************************************/

typedef volatile struct
	{
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
		/* 3 */
	U16												: 8;

	U16	ConferenceId								: 3;
	U16												: 5;
		/* 4 */
	U16 Member2HearsMember0							: 1;
	U16 Member2HearsMember1							: 1;
	U16 Member2HearsMember2							: 1;
	U16 Member2HearsMember3							: 1;
	U16 Member3HearsMember0							: 1;
	U16 Member3HearsMember1							: 1;
	U16 Member3HearsMember2							: 1;
	U16 Member3HearsMember3							: 1;
		/* 5 */
	U16 Member0HearsMember0							: 1;
	U16 Member0HearsMember1							: 1;
	U16 Member0HearsMember2							: 1;
	U16 Member0HearsMember3							: 1;
	U16 Member1HearsMember0							: 1;
	U16 Member1HearsMember1							: 1;
	U16 Member1HearsMember2							: 1;
	U16 Member1HearsMember3							: 1;
	} Tac49xOpenOrUpdate4WayConferenceConfigurationPacket;

typedef volatile struct
	{
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
		/* 3 */
	U16												: 8;

	U16	ConferenceId								: 3;
	U16												: 5;
	} Tac49xClose4WayConferenceConfigurationPacket;

typedef volatile struct
	{
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
		/* 3 */
	U16 MemberId									: 3;
	U16												: 3;
	U16	LegType										: 2;

	U16	ConferenceId								: 3;
	U16												: 5;
	} Tac49xAdd4WayConferenceLegConfigurationPacket;

typedef volatile struct
	{
	Tac49xHpiPacketHeader				HpiHeader;
	Tac49xProprietaryPacketHeader		ProprietaryHeader;
		/* 3 */
	U16 MemberId									: 3;
	U16												: 3;
	U16	LegType										: 2;

	U16	ConferenceId								: 3;
	U16												: 5;
	} Tac49xRemove4WayConferenceLegConfigurationPacket;

typedef volatile struct
	{
		/* 3 */
	U16												: 8;

	U16	ConferenceId								: 3;
	U16												: 5;
	} Tac49xGet4WayConferenceDetailsCommandPayload;

typedef volatile struct
	{
		/* 3 */
	U16	KeySize										: 5;
	U16												: 3;

	U16	Action										: 3;
	U16												: 5;
		/* 4 */
	U16	DataSize_Msb								: 3;
	U16												: 5;

	U16	DataSize_Lsb								: 8;
		/* 5 */
	U8  Payload[MAX_PAYLOAD_SIZE - sizeof(Tac49xHpiPacketHeader) - sizeof(Tac49xProprietaryPacketHeader) - 4];
	} Tac49xProtectionTestCommandPayload;

    /**
     * @defgroup Tac49xHdlcFramerMessageStatusPayload Tac49xHdlcFramerMessageStatusPayload
     * @ingroup PACKET_STRUCTURES
     * @brief
     * The HDLC Framer Status packet is sent when the channel is opened as an HDLC Framer.
     * @{
     */

    /**
     * <BR>
     * The HDLC Framer Status packet is sent when the channel is opened as an HDLC Framer. This packet transfers HDLC 
     * events, errors and detected HDLC data to the host. If more than one event/data is detected within a 5 msec 
     * period, they are linked sequentially in the same packet.
     */

typedef volatile struct
    {
    U16 TimeStamp_MswMsb            : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_MswLsb            : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswMsb            : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswLsb            : 8;  /*!< 32-bit time stamp*/

    U8  MessageBatch[MAX_HDLC_MESSAGE_LENGTH];  /*!< Tac49xHdlcFramerMessageStatusSegment*/
    } Tac49xHdlcFramerMessageStatusPayload;

   /** @} */

   /**  
    * The message payload is relevant in the following Message Types: HDLC Frame Received, HDLC CRC Error Detected, 
    * HDLC Short Frame Error, HDLC Frame Exceed Limit Error.
    * @n If the parameter Message Type is set to 9 (HDLC Buffer Status Response), the Message Payload Size is set 
    * to 2 and the two data bytes are set to the remaining space in the HDLC Transmit Buffer, in bytes.
    */

typedef volatile struct
    {
    U16                             : 8;
    U16 Type                        : 8;  /*!< ::Tac49xHdlcFramerStatusMessageType*/

	U16 Size_Msb				    : 8;  /*!< Message payload size, in bytes. Range: 1-300. @n This field is relevant only when the parameter Type equals one of the followings: @li HDLC_FRAMER_STATUS_MESSAGE_TYPE__HDLC_FRAME_RECEIVED @li HDLC_FRAMER_STATUS_MESSAGE_TYPE__HDLC_FRAMER_FREE_BUFFER_SIZE */
	U16 Size_Lsb				    : 8;
    U8  MessageFirstByte;                 /*!< HDLC Data Message. These bytes are relevant only when the parameter Type equals one of the followings: @li HDLC_FRAMER_STATUS_MESSAGE_TYPE__HDLC_FRAME_RECEIVED @li HDLC_FRAMER_STATUS_MESSAGE_TYPE__HDLC_FRAMER_FREE_BUFFER_SIZE */
    } Tac49xHdlcFramerMessageStatusSegment;

    /**
     * @defgroup Tac49xCallerIdPayload Tac49xCallerIdPayload
     * @ingroup PACKET_STRUCTURES
     * @brief
     * The Caller ID event packet is sent after caller ID detection.
     * @{
     */

    /**
     * <BR>
     * The Caller ID event packet is sent after caller ID detection.
     */

typedef volatile struct                 
    {       /* word 2 */                  
    U16 TimeStamp_MswMsb                : 8;    /*!< 32-bit time stamp*/
    U16 TimeStamp_MswLsb                : 8;    /*!< 32-bit time stamp*/
            /* word 3 */                   
    U16 TimeStamp_LswMsb                : 8;    /*!< 32-bit time stamp*/
    U16 TimeStamp_LswLsb                : 8;    /*!< 32-bit time stamp*/
            /* word 4 */                   
    U16 Standard                        : 4;    /*!< ::Tac49xCallerIdStandard*/
    U16                                 : 4;
                                        
    U16 Error                           : 1;    /*!< Error flag @li 0 = No Error @li 1 = Error - there was an error in the process of detection of the current CID message.*/
    U16 ChecksumByteAbsent              : 1;    /*!< Checksum Absence @li 0 = Checksum byte is present at the end of message (none ADSI mode) @li 1 = Checksum byte is absent from the end of message (ADSI mode)*/
    U16                                 : 2;
    U16 ServiceType                     : 1;    /*!< ::Tac49xCallerIdServiceType Protocol of the packet*/
    U16 EtsiOnhookMethod                : 1;    /*!< ::Tac49xEtsiOnhookMethod Method. Applies only to ETSI "on hook" service.*/
    U16                                 : 2;
            /* word 5 */                  
    U16                                 : 8;
                                        
    U16 Event                           : 6;    /*!< ::Tac49xCallerIdEvent In the case of an error, this field provides information regarding the cause of the error.*/
    U16                                 : 2;
    U8  Message[MAX_CALLER_ID_MESSAGE_LENGTH];  /*!< The Caller ID payload. The maximum Caller ID message length is 256 bytes.*/
    } Tac49xCallerIdPayload;

    /** @} */

typedef volatile struct
    {
    U16 _Msb : 8;
    U16 _Lsb : 8;
    } Tac49xUnderrunOrOverrunPayload;

    /**
     * This error payload content is sent when the ErrorMessageIndex is set to SYSTEM_ERROR_EVENT_PACKET_INDEX__RX_TX_HOST_AND_NETWORK_OVERRUN.
     */

typedef volatile struct
    {
    Tac49xUnderrunOrOverrunPayload	HostRxOverrun;  /*!< Number of host receive overruns in the host port*/
    Tac49xUnderrunOrOverrunPayload	NetRxOverrun;   /*!< Number of host receive overruns in the network port*/
    Tac49xUnderrunOrOverrunPayload	NetTxOverrun;   /*!< Number of host transmit overruns in the network port. Applicable only in AC491*/
    U16                     : 8;
    U16                     : 8;
    } Tac49xRxTxHostAndNetworkOverrunPayload;

    /**
     * @defgroup Tac49xSystemErrorEventPayload Tac49xSystemErrorEventPayload
     * @ingroup PACKET_STRUCTURES
     * @brief
     * A system error packet is sent when the AC49x VoPP detects an error.
     * @{
     */

    /**
     * <BR>
     * A system error packet is sent when the AC49x VoPP detects an error. For example, when a bad packet 
     * is received from the Host, overruns, etc.
     * @n Refer to <A HREF="System Error Index.html">System Error Index</A> for information on available errors.
     */

typedef volatile struct
    {
    U16 TimeStamp_MswMsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_MswLsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswMsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswLsb                : 8;  /*!< 32-bit time stamp*/
    U16                                 : 8;
    U16 ErrorMessageIndex               : 8;  /*!< Refer to ::Tac49xSystemErrorEventPacketIndex. The error payload content is according to the ErrorMessageIndex.*/
    union
        {
		Tac49xRxTxHostAndNetworkOverrunPayload		RxTxHostAndNetworkOverrunPayload;
		Tac49xUnderrunOrOverrunPayload				PlaybackTxUnderrunPayload;
		Tac49xUnderrunOrOverrunPayload				PlaybackTxOverrunPayload;
		Tac49xUnderrunOrOverrunPayload				_3WayConferenceTxOverrunPayload; 
/*      Tac49xPacketErrorMessagePayload;
        Tac49xTimeSlotMisalighnmentErrorMessagePayload;
        Tac49xCommandSequenceErrorMessagePayload;
        Tac49xCommandChecksumErrorMessagePayload;
        Tac49xHostRereiveOverrunErrorMessagePayload;
*/
        U8 Payload[MAX_ERROR_MESSAGE_LENGTH];
        }u;        
    } Tac49xSystemErrorEventPayload;

    /** @} */

    /**
     * <BR>
     * This structure defines the payload of the following event packets: @li Switched to Voice @li Switched to Bypass 
     * @li Switched to Fax @li Switched to Data
     */

typedef volatile struct
    {
    U16								: 1;
    U16 InitiatingFlag				: 1;/*!< Refer to ::Tac49xInitiatingFlag.*/
    U16 MediaGatewayState			: 6;/*!< Refer to ::Tac49xMediaGatewayState.*/
    U16 ReasonIdentifierCode		: 8;/*!< Refer to ::Tac49xReasonIdentifierCode.*/

    U16								: 8;
    U16								: 8;
    } Tac49xStateSignalingEventPayload;

typedef volatile struct
    {
    U16 VoiceLength_Msb		: 8;
	U16 VoiceLength_Lsb		: 8;

	U16 SilenceLength_Msb	: 8;
	U16 SilenceLength_Lsb	: 8;

	U16 AmdFrameCounter_Msb		: 8;
	U16 AmdFrameCounter_Lsb		: 8;
    } Tac49xAnswerMachineDetectorStatusEventPayload;

    /**
     * @defgroup Tac49xGeneralEventPayload Tac49xGeneralEventPayload
     * @ingroup PACKET_STRUCTURES
     * @brief
     * The AC49x VoPP reports events with the general event status packet.
     * @{
     */

    /**
     * <BR>
     * The AC49x VoPP reports events with the general event status packet. The reported events are CAS 
     * Generation ended, IBS String Generation aborted, etc. The user distinguishes the event type with the
     * Tac49xGeneralEventPayload::EventIndex. @n Each event has a different payload structure, refer to 
     * the <A HREF="Event Index.html">Event Index</A> table.
     */

typedef volatile struct
    {
    U16 TimeStamp_MswMsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_MswLsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswMsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswLsb                : 8;  /*!< 32-bit time stamp*/
    U16                                 : 8;
    U16 EventIndex                      : 8;  /*!< ::Tac49xGeneralEventPacketIndex*/
    union
        {
/*      Tac49xCasStringGenerationEndedEventMessagePayload;
        Tac49xIbsStringGenerationEndedEventMessagePayload;
*/
		Tac49xStateSignalingEventPayload				StateSignalingEventPayload; /*!< Tac49xStateSignalingEventPayload*/
		Tac49xAnswerMachineDetectorStatusEventPayload	AnswerMachineDetectorStatusEventPayload;
        U8 Payload[MAX_EVENT_MESSAGE_LENGTH];
        }u;        
    } Tac49xGeneralEventPayload;

    /** @} */

    /**
     * @defgroup Tac49xIbsEventPayload Tac49xIbsEventPayload
     * @ingroup PACKET_STRUCTURES
     * @brief
     * The IBS detection event packet is sent when the AC49x VoPP detects start and end of DTMF / MF / Call Progress 
     * Signal.
     * @{
     */

    /**
     * <BR>
     * The IBS detection event packet is sent when the AC49x VoPP detects start and end of DTMF / MF / Call Progress 
     * Signal.
     */

typedef volatile struct
    {
    U16 TimeStamp_MswMsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_MswLsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswMsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswLsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeSincePreviousIbs_Msb        : 8;  /*!< Time, in msec, since the previous IBS event*/
    U16 TimeSincePreviousIbs_Lsb        : 8;
                                        
    U16                                 : 4;
    U16 Direction                       : 2;  /*!< ::Tac49xIbsDetectionDirection*/
    U16                                 : 1;
    U16 LongDtmfDetection               : 1;  /*!< Long DTMF detection. This bit indicates that the detected DTMF digit duration is more than 2 seconds. The event is transmitted only if the parameter ::Tac49xOpenOrUpdateChannelConfigurationAttr.LongDtmfDetectionEnable in the function ::Ac49xOpenChannelConfiguration is set to CONTROL__ENABLE.*/
                                        
    U16 System                          : 4;  /*!< ::Tac49xIbsSystemCode*/
    U16 Digit                           : 4;  /*!< Digit field according to <A HREF="DIGIT Field According to Signaling Type.html">Signaling Type table</A> and to <A HREF="Advanced Signals.html"> Advanced Signals Digits</A>.*/

    U16                                 : 8;
    U16 Level                           : 8;  /*!< Signal level ( in -dBm). (This field represents the level of the signal during the detection frame only).*/
    } Tac49xIbsEventPayload;

    /** @} */

    /**
     * @defgroup Tac49xCasEventPayload Tac49xCasEventPayload
     * @ingroup PACKET_STRUCTURES
     * @brief
     * The CAS event packet is sent when the AC49x VoPP detects a change in one of the ABCD signaling bits.
     * @{
     */

    /**
     * <BR>
     * The CAS event packet is sent when the AC49x VoPP detects a change in one of the ABCD signaling bits.
     * The CAS event packet is sent only if the CAS ditection is enabled. The CAS bits are transmitted to the 
     * AC491 and AC490 VoPPs through a dedicated signaling PCM highway.
     */

typedef volatile struct                 
    {                                   
    U16 TimeStamp_MswMsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_MswLsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswMsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswLsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeSincePreviousCas_Msb        : 8;  /*!< Time, in msec, since the previous CAS event.*/
    U16 TimeSincePreviousCas_Lsb        : 8;
                                        
    U16                                 : 4;
    U16 CasDetectionDirection			: 2;  /*!< ::Tac49xCasDetectionDirection*/
    U16                                 : 2;
                                        
    U16 D                               : 1;  /*!< Signaling D bit (bit 0)*/
    U16 C                               : 1;  /*!< Signaling C bit (bit 1)*/
    U16 B                               : 1;  /*!< Signaling B bit (bit 2)*/
    U16 A                               : 1;  /*!< Signaling A bit (bit 3)*/
    U16 ReversalsB                      : 1;  /*!< Reversals on B bit*/
    U16 ReversalsA                      : 1;  /*!< Reversals on A bit*/
    U16                                 : 2;
    } Tac49xCasEventPayload;

    /** @} */

    /**
     * @defgroup Tac49xFaxRelayStatusPayload Tac49xFaxRelayStatusPayload
     * @ingroup PACKET_STRUCTURES
     * @brief
     * Fax Relay Status packets are sent while the channel is in Fax Relay mode.
     * @{
     */

    /**
     * <BR>
     * Fax Relay Status packets are sent while the channel is in Fax Relay mode.
     */

typedef volatile struct                 
    {                                   
    U16 TimeStamp_MswMsb					: 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_MswLsb					: 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswMsb					: 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswLsb					: 8;  /*!< 32-bit time stamp*/
											
    U16	DatagramOverflow					: 1;  /*!< If the length of the output packet is longer than the specified datagram size, the packet is not transmitted to the network.*/
    U16										: 7;
											
    U16	TimeOutErrorFlag					: 1;  /*!< A Timeout error occurred.*/
    U16	ReceivedBadCommandFlag				: 1;  /*!< A Bad Command was received. Words BadInputPacketId, BadInputPacketTotalSize describe the Bad Command.*/
    U16	InternalErrorFlag					: 1;  /*!< Internal error*/
    U16	HostTransmitUnderRunFlag			: 1;  /*!< Channel received a buffer underrun*/
    U16	HostTransmitOverRunFlag				: 1;  /*!< Channel received a buffer overrun*/
    U16	FaxExitWithNoMcfFrameFlag			: 1;  /*!< Fax exit with no MCF Frame*/
    U16	IllegalHdlcFrameDetectedFlag		: 1;  /*!< Illegal HDLC frame detected*/
    U16	UnableToRecoverFlag					: 1;  /*!< Unable to recover lost packet*/
											
    U16	T30TxState							: 8;  /*!< T30 Transmit State. Refer to ::Tac49xFaxRelayT30State */
    U16	T30RxState							: 8;  /*!< T30 Receive State. Refer to ::Tac49xFaxRelayT30State */
											
	U16 NumberOfTransferredPages_Msb		: 8;  /*!< Number of transmitted or received pages positively confirmed by answering fax.*/
	U16 NumberOfTransferredPages_Lsb		: 8;
											
	U16 BadInputPacketId_Msb				: 8;  /*!< Sequence number of Bad Input UDPTL packet, or the first two bytes of the input TCP packet. The field is relevant if the "ReceivedBadCommandFlag" or the "UnableToRecoverFlag" bit is set.*/
	U16 BadInputPacketId_Lsb				: 8;
											
	U16 BadInputPacketTotalSize_Msb			: 8;  /*!< The field is relevant if  the "ReceivedBadCommandFlag" bit is set.*/
	U16 BadInputPacketTotalSize_Lsb			: 8;
											
    U16										: 8;

	U16 FaxBitRate							: 4;  /*!< Current bit rate of fax modem (refer to ::Tac49xFaxBitRate).*/
    U16										: 4;
											
    U16										: 8;
    U16										: 8;

    U16	EyeQualityMonitor_Msb				: 2;  /*!< Eye Quality Monitor measure for TCF or fax image. The value of 0 corresponds to the best quality of demodulation.*/
    U16										: 5;
    U16	DemodulationStatus					: 1;  /*!< Demodulation Status. @li 0 = EQM and Timing Offset fields are invalid. @li 1 = EQM and Timing Offset fields are valid.*/
    U16	EyeQualityMonitor_Lsb				: 8;
	
	U16 TimingOffsetPpm_Msb					: 8;  /*!< Timing Offset (ppm) recovered during demodulation of TCF or fax image. The offset is a short integer equal to the sum of the sampling rate error and the sending fax baud rate offset.*/
	U16 TimingOffsetPpm_Lsb					: 8;
	} Tac49xFaxRelayStatusPayload;

    /** @} */

    /**
     * @defgroup Tac49xJitterBufferStatusPayload Tac49xJitterBufferStatusPayload
     * @ingroup PACKET_STRUCTURES
     * @brief
     * The jitter buffer status packet reports the jitter buffer statistics.
     * @{
     */

    /**
     * <BR>
     * The jitter buffer status packet reports the jitter buffer statistics.
     * It is sent according to the field Tac49xOpenDeviceConfigurationAttr::JitterBufferStatusReportMode.
     */

typedef volatile struct
    {
    U16 TimeStamp_MswMsb				: 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_MswLsb				: 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswMsb				: 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswLsb				: 8;  /*!< 32-bit time stamp*/
										
    U16 SteadyStateDelay_Msb			: 8;  /*!< The Steady State Delay of the Dynamic Jitter Buffer, in msec.*/
    U16 SteadyStateDelay_Lsb			: 8;
										
    U16 NumberOfVoiceUnderrun_Msb		: 8;  /*!< The number of underruns that occured in the Tac49xOpenDeviceConfigurationAttr::ErrorReportInterval duration*/
    U16 NumberOfVoiceUnderrun_Lsb		: 8;
										
    U16 NumberOfVoiceOverrun_Msb		: 8;  /*!< The number of overruns that occured in the Tac49xOpenDeviceConfigurationAttr::ErrorReportInterval duration*/
    U16 NumberOfVoiceOverrun_Lsb		: 8;
										
    U16 NumberOfVoiceDecoderBfi_Msb		: 8;  /*!< Number of Voice Decoder BFI in the Tac49xOpenDeviceConfigurationAttr::ErrorReportInterval duration*/
    U16 NumberOfVoiceDecoderBfi_Lsb		: 8;

    U16 NumberOfVoicePacketsDropped_Msb : 8;  /*!< Number of Voice Packets Dropped in the Tac49xOpenDeviceConfigurationAttr::ErrorReportInterval duration*/
    U16 NumberOfVoicePacketsDropped_Lsb : 8;

    U16 NumberOfVoiceNetPacketsLost_Msb	: 8;  /*!< Number of Voice Net Packets Lost in the Tac49xOpenDeviceConfigurationAttr::ErrorReportInterval duration*/
    U16 NumberOfVoiceNetPacketsLost_Lsb	: 8;

    U16 NumberOfIbsOverrun_Msb			: 8;  /*!< Number of DTMF relay packets overruns in the Tac49xOpenDeviceConfigurationAttr::ErrorReportInterval duration*/
    U16 NumberOfIbsOverrun_Lsb			: 8;
										
    U16 NumberOfCasOverrun_Msb			: 8;  /*!< Number of CAS relay packets overruns in the Tac49xOpenDeviceConfigurationAttr::ErrorReportInterval duration*/
    U16 NumberOfCasOverrun_Lsb			: 8;

    U16 VideoDelay_Msb			        : 8;  /*!< Reserved for internal use. */
    U16 VideoDelay_Lsb			        : 8;

    U16 OptimalDelay_Msb		        : 8;  /*!< Reserved for internal use. */
    U16 OptimalDelay_Lsb		        : 8;

    U16 PacketCount_Msb			        : 8;  /*!< Actual current number of packets in the Jitter Buffer. */
    U16 PacketCount_Lsb			        : 8;
    } Tac49xJitterBufferStatusPayload;

    /** @} */

    /** This packet is sent when: @li An RTCP packet is received from the network @li In response to the function ::Ac49xGetRegularRtcpParameters*/

typedef volatile struct
    {
    U16 TransmitPacketCount_MswMsb  	   : 8;  /*!< The number of transmitted RTP packets. @li If Report Type = RR, the parameter is not valid (-1) @li If Report Type = SR, this is the Transmit Packet Count of the remote side. @li If Report Type = 'Get RTCP Command Event', this is the Transmit Packet Count of the local side.*/
    U16 TransmitPacketCount_MswLsb  	   : 8;
    U16 TransmitPacketCount_LswMsb  	   : 8;
    U16 TransmitPacketCount_LswLsb  	   : 8;

    U16 TransmitOctetCount_MswMsb   	   : 8;  /*!< The number of transmitted RTP bytes. @li If Report Type = RR, the parameter is not valid (-1) @li If Report Type = SR, this is the Transmit Octet Count of the remote side. @li If Report Type = 'Get RTCP Command Event', this is the Transmit Octet Count of the local side.*/
    U16 TransmitOctetCount_MswLsb   	   : 8;
    U16 TransmitOctetCount_LswMsb   	   : 8;
    U16 TransmitOctetCount_LswLsb   	   : 8;

    U16 ReceivePacketCount_MswMsb   	   : 8;  /*!< Receive Packet Count of the local side.*/
    U16 ReceivePacketCount_MswLsb   	   : 8;
    U16 ReceivePacketCount_LswMsb   	   : 8;
    U16 ReceivePacketCount_LswLsb   	   : 8;
	
    U16 ReceiveOctetCount_MswMsb    	   : 8;  /*!< Receive Octet Count of the local side.*/
    U16 ReceiveOctetCount_MswLsb    	   : 8;
    U16 ReceiveOctetCount_LswMsb    	   : 8;
    U16 ReceiveOctetCount_LswLsb    	   : 8;
	
    U16 Jitter_MswMsb					   : 8;  /*!< An estimate of the statistical variance of the RTP data packet interarrival time. @li If Report Type = RR or SR, this is the jitter of the remote side. @li If Report Type = 'Get RTCP Command Event', this is the jitter of the local side. @li If not available, the field equals -1.*/
    U16 Jitter_MswLsb					   : 8;
    U16 Jitter_LswMsb					   : 8;
    U16 Jitter_LswLsb					   : 8;
	
    U16 RoundTrip_MswMsb				   : 8;  /*!< The network round-trip delay. @n If not available, the field equals -1.*/
    U16 RoundTrip_MswLsb				   : 8;
    U16 RoundTrip_LswMsb				   : 8;
    U16 RoundTrip_LswLsb				   : 8;
	
    U16 CumulateLostPackets_MswMsb		   : 8;  /*!< The sum of the packets lost in reception. @li If Report Type = RR or SR, this is Cumulate Lost Packets of the remote side. @li If Report Type = 'Get RTCP Command Event', this is the Cumulate Lost Packets of the local side. @li If not available, the field equals -1.*/
    U16 CumulateLostPackets_MswLsb		   : 8;
    U16 CumulateLostPackets_LswMsb		   : 8;
    U16 CumulateLostPackets_LswLsb		   : 8;
	
    U16 ExtendedHighSequenceNumber_MswMsb  : 8;  /*!< The low 16 bits contain the highest sequence number received in an RTP data packet from source SSRC_n, and the most significant 16 bits extend that sequence number with the corresponding count of sequence number cycles. @li If Report Type = RR or SR, this is the Ext High Seq Number of the remote side. @li If Report Type = 'Get RTCP Command Event', this is the Ext High Seq Number of the local side. @li If not available, the field equals -1.*/
    U16 ExtendedHighSequenceNumber_MswLsb  : 8;
    U16 ExtendedHighSequenceNumber_LswMsb  : 8;
    U16 ExtendedHighSequenceNumber_LswLsb  : 8;
	
    U16 SynchronizationSourceSender_MswMsb : 8;  /*!< Synchronization Source Identifier. @li If Report Type = RR or SR, this is the SSRC of the remote side. @li If Report Type = 'Get RTCP Command Event', this is the SSRC of the local side. @li If not available, the field equals -1.*/
    U16 SynchronizationSourceSender_MswLsb : 8;
    U16 SynchronizationSourceSender_LswMsb : 8;
    U16 SynchronizationSourceSender_LswLsb : 8;
	
    U16 RtpTimeStamp_MswMsb				   : 8;  /*!< @li If Report Type = RR, this is the last received time stamp of the local side. @li If Report Type = SR, this is the Transmit Time Stamp of the remote side. @li If Report Type = 'Get RTCP Command Event', this is the Transmit Time Stamp of the local side.*/
    U16 RtpTimeStamp_MswLsb				   : 8;
    U16 RtpTimeStamp_LswMsb				   : 8;
    U16 RtpTimeStamp_LswLsb				   : 8;
	
    U16 FractionLost_Msb				   : 8;  /*!< The fraction of RTP data packets from source SSRC_n lost since the previous SR or RR packet was sent, expressed as a fixed point number with the binary point at the left edge of the field. @li If Report Type = RR or SR, this is the Fraction Lost of the remote side. @li If Report Type = 'Get RTCP Command Event', this is the Fraction Lost of the local side. @li If not available, the field equals -1.*/
    U16 FractionLost_Lsb				   : 8;
	
    U16 								   : 7;
    U16 ByePacketReceived				   : 1;  /*!< The channel received an RTCP Bye packet from the remote channel. @n 0 = Bye packet was not received. @n 1= Bye packet was received.*/

    U16 ReportType						   : 3;  /*!< Describes the type of the report (refer to ::Tac49xPacketizerStatusOrEvent__RtcpStatusOrEventReportType).*/
    U16 								   : 5;

    U16 SmoothRoundTrip_MswMsb             : 8;  /*!< Averaged round trip delay. If this parameter is not available, this field is set to -1.*/
	U16 SmoothRoundTrip_MswLsb             : 8;
	U16 SmoothRoundTrip_LswMsb             : 8;
	U16 SmoothRoundTrip_LswLsb             : 8;

    U16                                    : 8;
    U16                                    : 8;
    U16                                    : 8;
    U16                                    : 8;
    U16                                    : 8;
    U16                                    : 8;
    U16                                    : 8;
    U16                                    : 8;
    U16                                    : 8;
    U16                                    : 8;
	U16                                    : 8;
    U16                                    : 8;											 

    U16 RtcpCanonicalNameLength_Msb        : 8;    /*!< Size of RTCP CNAME. @li If Report Type = RR or SR, this is the RTCP Cname Length of the remote side. @li If Report Type = 'Get RTCP Command Event', this is the RTCP Cname Length of the local side.*/
    U16 RtcpCanonicalNameLength_Lsb        : 8;

    U8  RtcpCanonicalName[AC49X_MAX_RTP_CANONICAL_NAME_STRING_LENGTH];   /*!< The chars of RTCP CNAME. @li If Report Type = RR or SR, this is the RTCP Cname of the remote side. @li If Report Type = 'Get RTCP Command Event', this is the RTCP Cname of the local side.*/                                          
	} Tac49xRtcpReceiveStatus;

    /** This packet is sent when an RTCP packet is sent to the network.*/

typedef volatile struct
	{
    U16 TransmitPacketCount_MswMsb  	   : 8;  /*!< @li If Report Type = SR, this is the Transmit Packet Count of the local side. @li If Report Type = RR, the field equals -1.*/
    U16 TransmitPacketCount_MswLsb  	   : 8;
    U16 TransmitPacketCount_LswMsb  	   : 8;
    U16 TransmitPacketCount_LswLsb  	   : 8;
	
    U16 TransmitOctetCount_MswMsb   	   : 8;  /*!< @li If Report Type = SR, this is the Transmit Octet Count of the local side. @li If Report Type = RR, the field equals -1.*/
    U16 TransmitOctetCount_MswLsb   	   : 8;
    U16 TransmitOctetCount_LswMsb   	   : 8;
    U16 TransmitOctetCount_LswLsb   	   : 8;
	
    U16 ReceivePacketCount_MswMsb   	   : 8;  /*!< The number of received RTP packets.*/
    U16 ReceivePacketCount_MswLsb   	   : 8;
    U16 ReceivePacketCount_LswMsb   	   : 8;
    U16 ReceivePacketCount_LswLsb   	   : 8;
	
    U16 ReceiveOctetCount_MswMsb    	   : 8;  /*!< The number of received RTP bytes.*/
    U16 ReceiveOctetCount_MswLsb    	   : 8;
    U16 ReceiveOctetCount_LswMsb    	   : 8;
    U16 ReceiveOctetCount_LswLsb    	   : 8;
	
    U16 Jitter_MswMsb					   : 8;  /*!< An estimate of the statistical variance of the RTP data packet interarrival time. @n If not available, the field equals -1.*/
    U16 Jitter_MswLsb					   : 8;
    U16 Jitter_LswMsb					   : 8;
    U16 Jitter_LswLsb					   : 8;
	
    U16 RoundTrip_MswMsb				   : 8;  /*!< Always 0 – isn’t calculated in RTCP transmit.*/
    U16 RoundTrip_MswLsb				   : 8;
    U16 RoundTrip_LswMsb				   : 8;
    U16 RoundTrip_LswLsb				   : 8;
	
    U16 CumulateLostPackets_MswMsb		   : 8;  /*!< The sum of the packets lost in reception. @n If not available, the field equals -1.*/
    U16 CumulateLostPackets_MswLsb		   : 8;
    U16 CumulateLostPackets_LswMsb		   : 8;
    U16 CumulateLostPackets_LswLsb		   : 8;
	
    U16 ExtendedHighSequenceNumber_MswMsb  : 8;  /*!< The low 16 bits contain the highest sequence number received in an RTP data packet from source SSRC_n, and the most significant 16 bits extend that sequence number with the corresponding count of sequence number cycles. @n If not available, the field equals -1.*/
    U16 ExtendedHighSequenceNumber_MswLsb  : 8;
    U16 ExtendedHighSequenceNumber_LswMsb  : 8;
    U16 ExtendedHighSequenceNumber_LswLsb  : 8;
	
    U16 SynchronizationSourceSender_MswMsb : 8;  /*!< Synchronization Source Identifier*/
    U16 SynchronizationSourceSender_MswLsb : 8;
    U16 SynchronizationSourceSender_LswMsb : 8;
    U16 SynchronizationSourceSender_LswLsb : 8;
	
    U16 RtpTimeStamp_MswMsb				   : 8;  /*!< The time stamp of the RTCP packet.*/
    U16 RtpTimeStamp_MswLsb				   : 8;
    U16 RtpTimeStamp_LswMsb				   : 8;
    U16 RtpTimeStamp_LswLsb				   : 8;
	
    U16 FractionLost_Msb				   : 8;  /*!< The fraction of RTP data packets from source SSRC_n lost since the previous SR or RR packet was sent, expressed as a fixed point number with the binary point at the left edge of the field. @n If not available, the field equals -1.*/
    U16 FractionLost_Lsb				   : 8;
	
    U16 								   : 8;

    U16 ReportType						   : 3;  /*!< ::Tac49xPacketizerStatusOrEvent__RtcpStatusOrEventReportType*/
    U16 								   : 5;

    U16 SmoothRoundTrip_MswMsb             : 8;  /*!< This field is always set to zero (0).*/
	U16 SmoothRoundTrip_MswLsb             : 8;
	U16 SmoothRoundTrip_LswMsb             : 8;
	U16 SmoothRoundTrip_LswLsb             : 8;

    U16                                    : 8;
    U16                                    : 8;
    U16                                    : 8;
    U16                                    : 8;
    U16                                    : 8;
    U16                                    : 8;
    U16                                    : 8;
    U16                                    : 8;
    U16                                    : 8;
    U16                                    : 8;
    } Tac49xRtcpTransmitStatus;

typedef volatile struct
	{
    U16				: 8;
    U16	FieldOfRate	: 8;
	} TRfciStruct;


typedef volatile struct
	{
    U16								    : 8;
    U16				ProtocolState	    : 8;        /*!< ::Tac49x3GppUserPlaneProtocolState describes the 3GPP UP protocol state.*/

    TRfciStruct		Rfci[MAX_NUMBER_OF_3GPP_RFCI];  /*!< The RFCI values of all rates. These fields are relevant only when ProtocolState = 3G UP RUN. If RFCI = -1, it isn't valid.*/

    U16								    : 8;

    U16 _3GppUserPlaneVersion		    : 4;        /*!< ::Tac49x3GppUserPlaneVersion The selected 3GPP UP version. This field is relevant only when EventState = 3G UP RUN.*/
    U16	ProtocolDataUnitType		    : 1;        /*!< ::Tac49x3GppProtocolDataUnitType @n This field is relevant only when ProtocolState = 3G UP RUN.*/
    U16								    : 3;

    U16								    : 8;
    U16 Encoder						    : 8;        /*!<The chosen Encoder, after initialization. This field is relevant only when ProtocolState = 3G UP RUN.*/
	} Tac49x3GppUserPlaneStateEvent;

    /**
     * This event is sent when a Negative Acknowledgment (NACK) is received or an error is detected by the 3GPP UP 
     * support mode protocol.
     */

typedef volatile struct
	{
    U16 CauseValue_Msb				    : 7;  /*!< ::Tac49xPacketizerStatusOrEvent__3GppUserPlaneErrorEventOrNackIndex describes the kind of error that occurred.*/
    U16 Cause							: 1;  /*!< ::Tac49xPacketizerStatusOrEvent__3GppUserPlaneErrorEventOrNackCause */
    U16 CauseValue_Lsb				    : 8;

    U16 Distance_Msb				    : 8;  /*!< Indicates where the error occurred. @li 0 = Occurred at the local side. @li > 0 = Occurred at a remote side*/
    U16 Distance_Lsb				    : 8;
	} Tac49x3GppUserPlaneErrorOrNackEvent;

    /**
     * This packet is sent in response to the ::Ac49xRegularChannelGet3GppStatistics and 
     * ::Ac49xMediatedChannelGet3GppStatistics functions.
     */

typedef volatile struct
	{
    U16 RxPacketCounter_MswMsb			: 8;  /*!< Counts the number of all packets arriving*/
    U16 RxPacketCounter_MswLsb			: 8;
    U16 RxPacketCounter_LswMsb			: 8;
    U16 RxPacketCounter_LswLsb			: 8;

    U16 RxVoicePacketLossCounter_MswMsb	: 8;  /*!< Counts the number of voice packets lost (that don't arive or that are dropped due to a CRC error)*/
    U16 RxVoicePacketLossCounter_MswLsb	: 8;
    U16 RxVoicePacketLossCounter_LswMsb	: 8;
    U16 RxVoicePacketLossCounter_LswLsb	: 8;
										
    U16 RxControlPacketLossCounter_Msb	: 8;  /*!< Counts the number of  all control packets lost*/
    U16 RxControlPacketLossCounter_Lsb	: 8;
										
    U16 RxCrcErrorCounter_Msb			: 8;  /*!< Counts the number of times that packets had a CRC error*/
    U16 RxCrcErrorCounter_Lsb			: 8;
	} Tac49x3GppUserPlaneStatisticStatus;


    /** 
     * This event occurs in 3GPP UP Support Mode protocol when: 
     * @li In response to the ::Ac49xRegularChannelSet3GppUserPlaneRateControl or ::Ac49xMediatedChannelSet3GppUserPlaneRateControl functions.
     * @li The channel gets the "3GPP UP Rate Control" command from the other side, and moves to the new Encoder rate.
     *
     * In Transparent mode, the event doesn't occur.
     */

typedef volatile struct
	{
    U16								    : 8;

    U16 SendOrRecievedRateControl	    : 1;  /*!< ::Tac49xPacketizerStatusOrEvent__3GppUserPlaneSelectRateEvent__SentOrReceivedRateControl*/
    U16								    : 7;
										
    U16								    : 8;

    U16 EncoderRate					    : 5;  /*!< ::Tac49x3GppRate*/
    U16								    : 3;
	} Tac49x3GppUserPlaneSelectRateEvent;


    /**
     * This event occurs when:
     * @li The channel changes the CMR field (Command Mode Request) in the packets transmitted to the other side.
     * @li The channel detects a change in the CMR field of a received packet, and moves to the new Encoder Rate.
     */

typedef volatile struct
	{
    U16								    : 8;

    U16 SendOrRecievedRateControl	    : 1;  /*!< @li 0 = Encoder was changed, according to new Rate Control Request @li 1 = New Rate Control Request */
    U16	Coder						    : 2;  /*!< @li 0 = AMR @li 1 = WB-AMR */
    U16								    : 5;
										
    U16	CodecModeRequest			    : 8;  /*!< The CMR field received from the remote side. */

    U16 EncoderRate					    : 5;  /*!<The encoder rate. Refer to the table <A HREF="AMR and Wide-Band AMR Rates.html">AMR and AMR-WB rates</A>.*/
    U16								    : 3;
	} Tac49xRfc3267AmrSelectRateEvent;

    /**
     * This event occurs when:
     * @li The channel detects a change in parameter MMM of a received packet and moves to the new encoder mode.
     * @li The channel changes parameter MMM (mode request) in the transmitted packets to the other side, without a command from Host (for future TFO support).
     */

typedef volatile struct
	{
    U16								    : 8;

    U16	EventReason					    : 1;  /*!< Send/Receive mode rate. @li 0 = Encoder mode was changed, according to a new Rate Mode Request from the far end. @li 1 = New Rate Mode request to the far end.*/
    U16								    : 7;

    U16								    : 8;

    U16 RateMode						: 4;  /*!< The rate mode of the encoder according to RFC 3558. @li 0 = All rate 1 frames remain in rate 1. @li 1 = 3/4 of the rate 1 frames remain as rate 1 and 1/4 are changed to rate 1/2. @li 2 = 1/2 of the rate 1 frames remain as rate 1 and 1/2 are changed to rate 1/2. @li 3 = 1/4 of the rate 1 frames remain as rate 1 and 3/4 are changed to rate 1/2. @li 4 = All rate 1 frames are changed to rate 1/2.*/
    U16								    : 4;
	} Tac49xRfc3558RateModeEvent;


    /**
     * This event occurs when the channel detects a change in the MBS field of a received packet. The local encoder rate is 
     * changed according to ARCM (defined in ::Ac49xG729EvInitializationConfiguration).
     * 
     */
typedef volatile struct
	{
    U16	NewMaxRemoteBitRate			    : 5;    /*!< This field indicates the MBS received from the remote side. */
    U16								    : 3;

    U16 LocalBitRate					: 5;    /*!< This field indicates the local rate. */
    U16								    : 3;
	} Tac49xG729EvRateEvent;

    /**
     * This event occurs when: 
     * @li The channel detects a high packet loss or a high delay. The event alerts the Host to perform a handout, 
     * switching the call to another network. 
     * @li The channel returns from a state of high packet loss or high delay. 
     * <BR> <BR> The event currently operates only with RFC 3267. The handout thresholds are defined in the 
     * function ::Ac49xRfc3267AmrInitializationConfiguration. The last policy’s fields (::Tac49xRfc3267CodecModeAttr) 
     * define this event’s threshold.
     */

typedef volatile struct
    {
    U16                                 : 8;
    U16 Report                          : 1;    /*!< Handout @li 0 = Handout unnecessary @li 1 = Handout required */
    U16                                 : 7;    
    } Tac49xHandoutEvent;



    /**
	* This status packet is sent in response to the function ::Ac49xGet3GppIubStatus or each time an Iub error occurs.
     */
typedef volatile struct
    {
    U16 Reason_Msb                      : 8; /*!< The reason for the event (refer to ::Tac49x3GppIubStatusReason).*/
    U16 Reason_Lsb                      : 8;

    U16 PacketCounter_MswMsb            : 8; /*!< Counts the number of all the packets that have arrived.*/
    U16 PacketCounter_MswLsb            : 8;
    U16 PacketCounter_LswMsb            : 8;
    U16 PacketCounter_LswLsb            : 8;

    U16 PacketLossCounter_MswMsb        : 8; /*!< Counts the number of packets that were lost. The protocol can detect up to 255 successive packet losses.*/
    U16 PacketLossCounter_MswLsb        : 8;
    U16 PacketLossCounter_LswMsb        : 8;
    U16 PacketLossCounter_LswLsb        : 8;

    U16 HeaderCrcErrorCounter_MswMsb    : 8; /*!< Counts the number of packets in which a Header CRC error was detected.*/
    U16 HeaderCrcErrorCounter_MswLsb    : 8;
    U16 HeaderCrcErrorCounter_LswMsb    : 8;
    U16 HeaderCrcErrorCounter_LswLsb    : 8;

    U16 PayloadCrcErrorCounter_MswMsb   : 8; /*!< Counts the number of packets in which a Payload CRC error was detected.*/
    U16 PayloadCrcErrorCounter_MswLsb   : 8;
    U16 PayloadCrcErrorCounter_LswMsb   : 8;
    U16 PayloadCrcErrorCounter_LswLsb   : 8;
    } Tac49x3GppIubStatus;


    /**
	* This status packet is sent in response to the function ::Ac49xGetRegularChannelRtcpXrVoiceQualityStatisticsCommand or each time the RTCP-XR packet is sent or received. This event is the exact copy of the RTCP-XR packet.
     */
typedef volatile struct
    {
											
	U16	LossRate								: 8; /*!< The fraction of RTP data packets from the source that have been lost since being received.*/
	U16	DiscardRate								: 8; /*!< The fraction of RTP data packets from the source that have been discarded since being received.*/
		  							
	U16	BurstDensity							: 8; /*!< The fraction of RTP data packets within burst periods that were either lost or discarded since being received.*/
	U16 GapDensity								: 8; /*!< The fraction of RTP data packets within inter-burst gaps that were either lost or discarded since being received.*/
	   								
	U16	BurstDuration_Msb						: 8; /*!< The mean duration, expressed in milliseconds, of the burst periods.*/
	U16	BurstDuration_Lsb						: 8;
	     							
	U16	GapDuration_Msb          				: 8; /*!< The mean duration, expressed in milliseconds, of the gap periods.*/
	U16	GapDuration_Lsb          				: 8;
	      							
	U16	RoundTripDelay_Msb						: 8; /*!< The most recently calculated round trip time between RTP interfaces, expressed in milliseconds.*/
	U16	RoundTripDelay_Lsb						: 8;
		  							
	U16	EndSystemDelay_Msb						: 8; /*!< The most recently estimated end system delay, expressed in milliseconds.*/
	U16	EndSystemDelay_Lsb						: 8;
										
	U16 SignalLevel								: 8; /*!< The voice signal relative level defined as the ratio of the signal level to a 0 dBm0 reference.*/
	U16 NoiseLevel								: 8; /*!< The noise level defined as the ratio of the silent period background noise level to a 0 dBm0 reference.*/
										
	U16 ResidualEchoReturnLoss 					: 8; /*!< The residual echo return loss value can be measured directly by the VoIP end system's echo canceller.*/
	U16 GapMin									: 8; /*!< The gap threshold.*/
		   							
	U16 RFactor									: 8; /*!< The voice quality metric describing the segment of the call that is carried over the RTP session.*/
	U16 ExtendedRFactor							: 8; /*!< The voice quality metric describing the segment of the call that is carried over a network segment.*/
	   								
	U16 MeanOpinionScoreListeningQuality		: 8; /*!< The estimated mean opinion score for listening quality (MOS-LQ) is a voice quality metric on a scale from 1 to 5, 
                                                      * where 5 is excellent and 1 is unacceptable. This metric does not include the effects of delay. It is expressed as an 
                                                      * integer in the range 10 to 50, corresponding to MOS x 10. For example, a value of 35 corresponds to an estimated MOS score of 3.5. 
                                                      * A value of 127 indicates that this parameter is unavailable.*/
   
    U16 MeanOpinionScoreConversationalQuality	: 8; /*!< The estimated mean opinion score for conversational quality (MOS-CQ) is a voice quality metric on a scale from 1 to 5, 
                                                      * where 5 is excellent and 1 is unacceptable. This metric includes the effects of delay and other effects that affect conversational quality. 
                                                      * It is expressed as an integer in the range 10 to 50, corresponding to MOS x 10 (as for MOS-LQ).*/
		
	U16 RxConfig								: 8; /*!< Receiver configuration byte (see RFC 3611).*/
	U16											: 8;
	   								
	U16 JitterBufferNominalDelay_Msb			: 8; /*!< Current nominal jitter buffer delay.*/
	U16 JitterBufferNominalDelay_Lsb			: 8;
	   								
	U16 JitterBufferMaxDelay_Msb				: 8; /*!< Jitter buffer delay (in milliseconds) which corresponds to the earliest received packet that was not discarded.*/
	U16 JitterBufferMaxDelay_Lsb				: 8;
	      							
	U16 JitterBufferAbsoluteMaxDelay_Msb		: 8; /*!< Absolute maximum delay of jitter buffer.*/
	U16 JitterBufferAbsoluteMaxDelay_Lsb		: 8;
	} Tac49xRtcpXrVoiceQualityStatisticsEvent;


    /**
	* This status packet is sent in response to the function ::Ac49xGetRegularChannelRtcpXrMarkovModelStatisticsCommand.
     */
typedef volatile struct
    {
									
	U16	Bursts_Msb						    : 8; /*!< Number of bursts during the call.*/
	U16	Bursts_Lsb							: 8;
		
	U16 TotalBurstReceiveFrames_MswMsb		: 8; /*!< The total number of frames received during bursts.*/
	U16 TotalBurstReceiveFrames_MswLsb		: 8; 
	U16 TotalBurstReceiveFrames_LswMsb		: 8; 
	U16 TotalBurstReceiveFrames_LswLsb		: 8; 
		
	U16 TotalBurstLossFrames_MswMsb			: 8; /*!< The total number of frames lost during bursts.*/
	U16 TotalBurstLossFrames_MswLsb			: 8; 
	U16 TotalBurstLossFrames_LswMsb			: 8; 
	U16 TotalBurstLossFrames_LswLsb			: 8; 							
		   										
	U16 BurstExcess_msec_Msb				: 8; /*!< The cumulative number of milliseconds of consecutive losses/discards that extend beyond the PLC effectiveness period (currently 50 milliseconds) during burst states.*/
	U16 BurstExcess_msec_Lsb				: 8;
													
	U16 Gaps_Msb							: 8; /*!< The number of gaps occurring during the session.*/
	U16 Gaps_Lsb							: 8;
									
	U16 TotalGapReceivedFrames_MswMsb		: 8; /*!< The total number of frames received during gaps.*/
	U16 TotalGapReceivedFrames_MswLsb		: 8; 
	U16 TotalGapReceivedFrames_LswMsb		: 8; 
	U16 TotalGapReceivedFrames_LswLsb		: 8; 
	   							
	U16 TotalGapLossFrames_MswMsb			: 8; /*!< The total number of frames lost during gaps.*/
	U16 TotalGapLossFrames_MswLsb			: 8; 
	U16 TotalGapLossFrames_LswMsb			: 8; 
	U16 TotalGapLossFrames_LswLsb			: 8; 
	     										
	U16 GapExcess_msec_Msb					: 8; /*!< The cumulative number of milliseconds of consecutive losses/discards that extend beyond the PLC effectiveness period (currently 50 milliseconds) during gap states.*/
	U16 GapExcess_msec_Lsb					: 8;
		
	U16 RunFramesConsecutiveGood_MswMsb		: 8; /*!< The number of consecutive good frames received since the last VMM event.*/
	U16 RunFramesConsecutiveGood_MswLsb		: 8; 
	U16 RunFramesConsecutiveGood_LswMsb		: 8; 
	U16 RunFramesConsecutiveGood_LswLsb		: 8; 
		
	U16 RunFramesFromRecentBurst_MswMsb		: 8; /*!< The number of MIU periods elapsed since the last significant burst.*/
	U16 RunFramesFromRecentBurst_MswLsb		: 8; 
	U16 RunFramesFromRecentBurst_LswMsb		: 8; 
	U16 RunFramesFromRecentBurst_LswLsb		: 8; 
		
	U16 FramesReceived_MswMsb				: 8; /*!< The total number of frames received.*/
	U16 FramesReceived_MswLsb				: 8; 
	U16 FramesReceived_LswMsb				: 8; 
	U16 FramesReceived_LswLsb				: 8; 
		
	U16 FramesLost_MswMsb					: 8;  /*!< The total number of frames lost in the network.*/
	U16 FramesLost_MswLsb					: 8; 
	U16 FramesLost_LswMsb					: 8; 
	U16 FramesLost_LswLsb					: 8; 
	   
	U16 FramesDiscarded_MswMsb				: 8;  /*!< The total number of frames discarded by the jitter buffer.*/
	U16 FramesDiscarded_MswLsb				: 8; 
	U16 FramesDiscarded_LswMsb				: 8; 
	U16 FramesDiscarded_LswLsb				: 8; 
		
	U16 OneWayDelay_Msb						: 8;  /*!< Delay (in ms) in one direction of the network.*/
	U16 OneWayDelay_Lsb						: 8;
		
	U16 SignalLevel							: 8;  /*!< The ratio of the silent period background noise level to a 0 dBm0 reference.*/
	U16 NoiseLevel							: 8;  /*!< The ratio of the signal level to a 0 dBm0 reference.*/
		                                          
	U16 ResidualEchoReturnLoss				: 8;  /*!< Residual Echo Return Loss.*/
	U16										: 8;
	} Tac49xRtcpXrMarkovModelStatisticsEvent;

    /**
	* This status packet is sent (to inform) upon any change in the RTCP-XR alerts state.
     */
typedef volatile struct
    {
	   						
	U16										: 2;

	U16 LowVoiceQualityAlertSetting			: 2; /*!< Low voice quality alert status. @li 0 = No change in alert mode @li 1 = Alert started @li 2 = Alert stopped */

	U16 DelayAlertSetting					: 2; /*!< Delay alert status. @li 0 = No change in alert mode @li 1 = Alert started @li 2 = Alert stopped */

	U16 BurstAlertSetting					: 2; /*!< Burst alert status. @li 0 = No change in alert mode @li 1 = Alert started @li 2 = Alert stopped */

	U16									    : 8;
} Tac49xRtcpXrAlertStatus;

/**
	* This status packet is sent to inform of the detection of the ::Tac49xActivateOrUpdateRtpChannelConfigurationAttr.SequenceNumberForDetection parameter on the function ::Ac49xActivateRegularRtpChannelConfiguration.
     */
typedef volatile struct
    {
	   
	U16 SequenceNumber_Msb					: 8; /*!< Received Sequence Number.*/

	U16 SequenceNumber_Lsb					: 8;
	   
	U16										: 8;

	U16	ReceivedBeforeTrigger				: 1;  /*!< Informs whether the sequence number was received before or after the function ::Ac49xActivateRegularRtpChannelConfiguration was invoked. @li 0 = Sequence number was received after the function was invoked @li 1 = Sequence number was received before the function was invoked */
	U16										: 7;

} Tac49xRtpSequenceNumberReceivedEvent;

    /**
     * This event occurs if the VoPP receives an RTCP APP packet from the remote side.
     * 
     */
typedef volatile struct
    {
    U16 SubType                         : 5;  /*!< Refer to RFC 3550 in URL http://www.ietf.org/rfc/rfc3550.txt for detailed information.*/
    U16                                 : 3;

    U16 PayloadType                     : 8;  /*!< Always set to 204.*/

    U16 DataLength_In4Bytes_Msb         : 8;
    U16 DataLength_In4Bytes_Lsb         : 8;  /*!< Length of the Application Dependent Data in 32 bit.*/

    U16 RemoteSsrc_MswMsb               : 8;
    U16 RemoteSsrc_MswLsb               : 8;
    U16 RemoteSsrc_LswMsb               : 8;
    U16 RemoteSsrc_LswLsb               : 8;  /*!< Remote SSRC.*/

    U8  Name[4];                              /*!< Four ASCII characters. Refer to RFC 3550 in URL http://www.ietf.org/rfc/rfc3550.txt for detailed information.*/
    U8  Data[64];                             /*!< Must be multiple of 32 bit. Max size is 64 bytes. Refer to RFC 3550 in URL http://www.ietf.org/rfc/rfc3550.txt for detailed information.*/
    } Tac49xRtcpAppEvent;

    /**
     * This event occurs when the packetizer detects an error.
     * @n Refer to <A HREF="Packetizer Error – Parameters.html">Packetizer Error</A> table.
     */

typedef volatile struct
    {
    U16 ErrorIndex_Msb	                : 8;
    U16 ErrorIndex_Lsb	                : 8;
    U16 Payload[MAX_EVENT_MESSAGE_LENGTH];
    } Tac49xPacketizerErrorEvent;


    /** 
     * This event occurs in the beginning of the stream, or after broken connection.
     */

typedef volatile struct
    {
    U16 TimeFromBrokenConnection_msec_MswMsb	: 8;  /*!< The time that passed since the Broken Connection Event. Resolution is in msec. If no broken connection occurred before, the time that passed is zero.*/
    U16 TimeFromBrokenConnection_msec_MswLsb	: 8;
    U16 TimeFromBrokenConnection_msec_LswMsb	: 8;
    U16 TimeFromBrokenConnection_msec_LswLsb	: 8;
    } Tac49xConnectionEstablishEvent;

    /** 
     * This event occurs when: @n 1) First RTP packet is received (with new remote SSRC). @n 2) Remote SSRC is changed.
     */

typedef volatile struct
    {
    U16 NewRemoteSsrc_MswMsb	: 8;  /*!< The new SSRC of the remote side.*/
    U16 NewRemoteSsrc_MswLsb	: 8;
    U16 NewRemoteSsrc_LswMsb	: 8;
    U16 NewRemoteSsrc_LswLsb	: 8;
    } Tac49xRemoteSsrcChangedEvent;

    /**
     * <BR>
     * This event is sent in response to the functions ::Ac49xGetRegularChannelMediaProtectionInfo when the parameter ::Tac49xGetMediaProtectiontInfoAttr.InfoType is set to AUDIT.
     */

typedef volatile struct
    {
    U16 ReceivedRtpPacketsTotalCount_MswMsb			: 8;  /*!< Total count of RTP packets received.*/
    U16 ReceivedRtpPacketsTotalCount_MswLsb			: 8;
    U16 ReceivedRtpPacketsTotalCount_LswMsb			: 8;
    U16 ReceivedRtpPacketsTotalCount_LswLsb			: 8;

    U16 DroppedRtpPacketsMacFailCount_MswMsb		: 8;  /*!< Number of RTP packets dropped due to MAC (Message Authentication Code) failure.*/
    U16 DroppedRtpPacketsMacFailCount_MswLsb		: 8;
    U16 DroppedRtpPacketsMacFailCount_LswMsb		: 8;
    U16 DroppedRtpPacketsMacFailCount_LswLsb		: 8;

    U16 DroppedRtpPacketsTimeStampFailCount_MswMsb	: 8;  /*!< Number of RTP packets dropped due to TimeStamp check failure.*/
    U16 DroppedRtpPacketsTimeStampFailCount_MswLsb	: 8;
    U16 DroppedRtpPacketsTimeStampFailCount_LswMsb	: 8;
    U16 DroppedRtpPacketsTimeStampFailCount_LswLsb	: 8;
	
    U16 ReceivedRtcpPacketsTotalCount_MswMsb		: 8;  /*!< Total count of RTCP packets received.*/
    U16 ReceivedRtcpPacketsTotalCount_MswLsb		: 8;
    U16 ReceivedRtcpPacketsTotalCount_LswMsb		: 8;
    U16 ReceivedRtcpPacketsTotalCount_LswLsb		: 8;
													
    U16 DroppedRtcpPacketsMacFailCount_MswMsb		: 8;  /*!< Number of RTCP packets dropped due to MAC failure.*/
    U16 DroppedRtcpPacketsMacFailCount_MswLsb		: 8;
    U16 DroppedRtcpPacketsMacFailCount_LswMsb		: 8;
    U16 DroppedRtcpPacketsMacFailCount_LswLsb		: 8;
													
    U16 DroppedRtcpPacketsReplayCount_MswMsb		: 8;  /*!< Number of RTCP packets dropped due to replay protection.*/
    U16 DroppedRtcpPacketsReplayCount_MswLsb		: 8;
    U16 DroppedRtcpPacketsReplayCount_LswMsb		: 8;
    U16 DroppedRtcpPacketsReplayCount_LswLsb		: 8;
													
    U16 DroppedRtcpPacketsTooOldCount_MswMsb		: 8;  /*!< Number of RTCP packets dropped due to old sequence number.*/
    U16 DroppedRtcpPacketsTooOldCount_MswLsb		: 8;
    U16 DroppedRtcpPacketsTooOldCount_LswMsb		: 8;
    U16 DroppedRtcpPacketsTooOldCount_LswLsb		: 8;
    } Tac49xPacketCableProtectionAuditEvent;

    /**
     * <BR>
     * This event is sent in response to the functions ::Ac49xGetRegularChannelMediaProtectionInfo when the parameter ::Tac49xGetMediaProtectiontInfoAttr.InfoType is set to AUDIT.
     */

typedef volatile struct
    {
    U16 ReceivedRtpPacketsTotalCount_MswMsb			: 8;  /*!< Total count of RTP packets received.*/
    U16 ReceivedRtpPacketsTotalCount_MswLsb			: 8;
    U16 ReceivedRtpPacketsTotalCount_LswMsb			: 8;
    U16 ReceivedRtpPacketsTotalCount_LswLsb			: 8;

    U16 DroppedRtpPacketsMacFailCount_MswMsb		: 8;  /*!< Number of RTP packets dropped due to MAC (Message Authentication Code) failure.*/
    U16 DroppedRtpPacketsMacFailCount_MswLsb		: 8;
    U16 DroppedRtpPacketsMacFailCount_LswMsb		: 8;
    U16 DroppedRtpPacketsMacFailCount_LswLsb		: 8;

    U16 DroppedRtpPacketsReplayCount_MswMsb			: 8;  /*!< Number of RTP packets dropped due to replay protection.*/
    U16 DroppedRtpPacketsReplayCount_MswLsb			: 8;
    U16 DroppedRtpPacketsReplayCount_LswMsb			: 8;
    U16 DroppedRtpPacketsReplayCount_LswLsb			: 8;

    U16 DroppedRtpPacketsTooOldCount_MswMsb			: 8;  /*!< Number of RTP packets dropped due to old sequence number.*/
    U16 DroppedRtpPacketsTooOldCount_MswLsb			: 8;
    U16 DroppedRtpPacketsTooOldCount_LswMsb			: 8;
    U16 DroppedRtpPacketsTooOldCount_LswLsb			: 8;

    U16 TxRtpKeyDerivationCount_Msb					: 8;  /*!< Number of transmitted RTP Key derivations that occured.*/
    U16 TxRtpKeyDerivationCount_Lsb					: 8;
    U16 RxRtpKeyDerivationCount_Msb					: 8;  /*!< Number of recevied RTP Key derivations that occured.*/
    U16 RxRtpKeyDerivationCount_Lsb					: 8;

    U16 ReceivedRtcpPacketsTotalCount_MswMsb		: 8;  /*!< Total count of RTCP packets received.*/
    U16 ReceivedRtcpPacketsTotalCount_MswLsb		: 8;
    U16 ReceivedRtcpPacketsTotalCount_LswMsb		: 8;
    U16 ReceivedRtcpPacketsTotalCount_LswLsb		: 8;

    U16 DroppedRtcpPacketsMacFailCount_MswMsb		: 8;  /*!< Number of RTCP packets dropped due to MAC failure.*/
    U16 DroppedRtcpPacketsMacFailCount_MswLsb		: 8;
    U16 DroppedRtcpPacketsMacFailCount_LswMsb		: 8;
    U16 DroppedRtcpPacketsMacFailCount_LswLsb		: 8;

    U16 DroppedRtcpPacketsReplayCount_MswMsb		: 8;  /*!< Number of RTCP packets dropped due to replay protection.*/
    U16 DroppedRtcpPacketsReplayCount_MswLsb		: 8;
    U16 DroppedRtcpPacketsReplayCount_LswMsb		: 8;
    U16 DroppedRtcpPacketsReplayCount_LswLsb		: 8;

    U16 DroppedRtcpPacketsTooOldCount_MswMsb		: 8;  /*!< Number of RTCP packets dropped due to old sequence number.*/
    U16 DroppedRtcpPacketsTooOldCount_MswLsb		: 8;
    U16 DroppedRtcpPacketsTooOldCount_LswMsb		: 8;
    U16 DroppedRtcpPacketsTooOldCount_LswLsb		: 8;

    U16 TxRtcpKeyDerivationCount_Msb				: 8;  /*!< Number of transmitted RTCP Key derivations that occured.*/
    U16 TxRtcpKeyDerivationCount_Lsb				: 8;
    U16 RxRtcpKeyDerivationCount_Msb				: 8;  /*!< Number of received RTCP Key derivations that occured.*/
    U16 RxRtcpKeyDerivationCount_Lsb				: 8;

	U16 DroppedRtpPacketsWrongMkiCount_Msb			: 8;  /*!< Number of RTP packets dropped due to incorrect MKI.*/
	U16 DroppedRtpPacketsWrongMkiCount_Lsb			: 8;
	U16 DroppedRtcpPacketsWrongMkiCount_Msb			: 8;  /*!< Number of RTCP packets dropped due to incorrect MKI.*/
	U16 DroppedRtcpPacketsWrongMkiCount_Lsb			: 8;
    } Tac49xSrtpProtectionAuditEvent;

    /**
     * <BR>
     * This event is sent in response to the functions ::Ac49xGetRegularChannelMediaProtectionInfo when the parameter ::Tac49xGetMediaProtectiontInfoAttr.InfoType is set to STATE.
     */

typedef volatile struct
    {
    U16 TxRtpTimeStamp_MswMsb       : 8;                  /*!< The current Tx RTP TimeStamp.*/
    U16 TxRtpTimeStamp_MswLsb       : 8;
    U16 TxRtpTimeStamp_LswMsb       : 8;
    U16 TxRtpTimeStamp_LswLsb       : 8;

    U16 RxRtpTimeStamp_MswMsb       : 8;                  /*!< The current Rx RTP TimeStamp.*/
    U16 RxRtpTimeStamp_MswLsb       : 8;
    U16 RxRtpTimeStamp_LswMsb       : 8;
    U16 RxRtpTimeStamp_LswLsb       : 8;

    U16 TxRtpSequenceNumber_Msb     : 8;                  /*!< The current Tx RTP Seq Number.*/
    U16 TxRtpSequenceNumber_Lsb     : 8;

    U16 RxRtpSequenceNumber_Msb     : 8;                  /*!< The current Rx RTP Seq Number.*/
    U16 RxRtpSequenceNumber_Lsb     : 8;

    U16 TxRtpNumberOfTimeStampWrapAround_Msb        : 8;  /*!< The current Packet Cable Tx RTP Nwrap.*/
    U16 TxRtpNumberOfTimeStampWrapAround_Lsb        : 8;

    U16 RxRtpNumberOfTimeStampWrapAround_Msb        : 8;  /*!< The current Packet Cable Rx RTP Nwrap.*/
    U16 RxRtpNumberOfTimeStampWrapAround_Lsb        : 8;

    U16 TxRtcpSequenceNumber_MswMsb     : 8;              /*!< The current Packet Cable Tx RTCP Seq Number.*/
    U16 TxRtcpSequenceNumber_MswLsb     : 8;
    U16 TxRtcpSequenceNumber_LswMsb     : 8;
    U16 TxRtcpSequenceNumber_LswLsb     : 8;

    U16 RxRtcpSequenceNumber_MswMsb     : 8;              /*!< The current Packet Cable Rx RTCP Seq Number.*/
    U16 RxRtcpSequenceNumber_MswLsb     : 8;
    U16 RxRtcpSequenceNumber_LswMsb     : 8;
    U16 RxRtcpSequenceNumber_LswLsb     : 8;
    } Tac49xPacketCableProtectionStateEvent;

    /**
     * <BR>
     * This event is sent in response to the functions ::Ac49xGetRegularChannelMediaProtectionInfo when the parameter ::Tac49xGetMediaProtectiontInfoAttr.InfoType is set to STATE.
     */

typedef volatile struct
    {
    U16 TxRtpTimeStamp_MswMsb			: 8;  /*!< The current Tx RTP TimeStamp.*/
    U16 TxRtpTimeStamp_MswLsb			: 8;
    U16 TxRtpTimeStamp_LswMsb			: 8;
    U16 TxRtpTimeStamp_LswLsb			: 8;

    U16 RxRtpTimeStamp_MswMsb			: 8;  /*!< The current Rx RTP TimeStamp.*/
    U16 RxRtpTimeStamp_MswLsb			: 8;
    U16 RxRtpTimeStamp_LswMsb			: 8;
    U16 RxRtpTimeStamp_LswLsb			: 8;

    U16 TxRtpSequenceNumber_Msb			: 8;  /*!< The current Tx RTP Seq Number.*/
    U16 TxRtpSequenceNumber_Lsb			: 8;

    U16 RxRtpSequenceNumber_Msb			: 8;  /*!< The current Rx RTP Seq Number.*/
    U16 RxRtpSequenceNumber_Lsb			: 8;

    U16 TxRtpRoc_MswMsb					: 8;  /*!< The current SRTP Tx RTP ROC.*/
    U16 TxRtpRoc_MswLsb					: 8;
    U16 TxRtpRoc_LswMsb					: 8;
    U16 TxRtpRoc_LswLsb					: 8;

    U16 RxRtpRoc_MswMsb					: 8;  /*!< The current SRTP Rx RTP ROC.*/
    U16 RxRtpRoc_MswLsb					: 8;
    U16 RxRtpRoc_LswMsb					: 8;
    U16 RxRtpRoc_LswLsb					: 8;

    U16 TxRtcpIndex_MswMsb				: 8;  /*!< The current SRTP Tx RTCP Index.*/
    U16 TxRtcpIndex_MswLsb				: 8;
    U16 TxRtcpIndex_LswMsb				: 8;
    U16 TxRtcpIndex_LswLsb				: 8;

    U16 RxRtcpIndex_MswMsb				: 8;  /*!< The current SRTP Rx RTCP Index.*/
    U16 RxRtcpIndex_MswLsb				: 8;
    U16 RxRtcpIndex_LswMsb				: 8;
    U16 RxRtcpIndex_LswLsb				: 8;
	} Tac49xSrtpProtectionStateEvent;

    /**
     * <BR>
     * This event is sent in response to the functions ::Ac49xGetRegularChannelMediaProtectionInfo.
     */

typedef volatile struct
	{
    U16 											: 8;

    U16 ProtectionMode								: 3;  /*!< Protection mode, refer to ::Tac49xMediaProtectionMode.*/
    U16 InfoType									: 4;  /*!< Information type, refer to ::Tac49xMediaProtectionInfoType.*/
    U16 											: 1;

	union
		{
		Tac49xPacketCableProtectionAuditEvent	PacketCableAudit;
		Tac49xSrtpProtectionAuditEvent			SrtpAudit;
		Tac49xPacketCableProtectionStateEvent	PacketCableState;
		Tac49xSrtpProtectionStateEvent			SrtpState;
		} u;
	} Tac49xMediaProtectionInfoEvent;

typedef volatile struct                     
    {
		/* 3 */
	U16				 						: 1;
    U16 InitiatingFlag 						: 1;
	U16				 						: 6;

	U16 ReasonIdentifierCode				: 8;
		/* 4 */
	U16 RtpTxSequenceNumber_Msb				: 8;
	U16 RtpTxSequenceNumber_Lsb				: 8;
		/* 5-6 */
	U16 RtpTxTimeStamp_MswMsb				: 8;
	U16 RtpTxTimeStamp_MswLsb				: 8;
	U16 RtpTxTimeStamp_LswMsb				: 8;
	U16 RtpTxTimeStamp_LswLsb				: 8;
		/* 7-8 */
	U16 RtpTxSsrc_MswMsb					: 8;
	U16 RtpTxSsrc_MswLsb					: 8;
	U16 RtpTxSsrc_LswMsb					: 8;
	U16 RtpTxSsrc_LswLsb					: 8;
	} Tac49xSwitchToModemEvent;

    /**
     * @defgroup Tac49xPacketizerStatusOrEventPayload Tac49xPacketizerStatusOrEventPayload
     * @ingroup PACKET_STRUCTURES
     * @brief
     * This event is sent by the packetizer.
     * @{
     */

    /**
     * <BR>
     * This event is sent by the packetizer.
     * @n The following <A HREF="Packetizer Event Indices.html">table </A> describes the Packetizer Event indices:
     */

typedef volatile struct
	{
    U16 TimeStamp_MswMsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_MswLsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswMsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswLsb                : 8;  /*!< 32-bit time stamp*/
	U16                                 : 8;
	U16 StatusOrEventIndex			    : 8;  /*!< ::Tac49xPacketizerStatusOrEventIndex*/
	union
		{
		Tac49xRtcpReceiveStatus					RtcpReceiveStatus;
		Tac49xRtcpTransmitStatus				RtcpTransmitStatus;
		Tac49x3GppUserPlaneStateEvent			_3GppUserPlaneStateEvent;
		Tac49x3GppUserPlaneErrorOrNackEvent		_3GppUserPlaneErrorOrNackEvent;
		Tac49x3GppUserPlaneStatisticStatus		_3GppUserPlaneStatisticStatus;
		Tac49x3GppUserPlaneSelectRateEvent		_3GppUserPlaneSelectRateEvent;
		Tac49xRfc3267AmrSelectRateEvent			Rfc3267AmrSelectRateEvent;
		Tac49xRfc3558RateModeEvent				Rfc3558RateModeEvent;
        Tac49xG729EvRateEvent					G729EvRateEvent;
		Tac49xPacketizerErrorEvent				PacketizerErrorEvent;
		Tac49xConnectionEstablishEvent			ConnectionEstablishEvent;
		Tac49xRemoteSsrcChangedEvent			RemoteSsrcChangedEvent;
		Tac49xMediaProtectionInfoEvent			MediaProtectionInfoEvent;
		Tac49xRtcpAppEvent						RtcpAppEvent;
        Tac49xHandoutEvent						Handout;
        Tac49x3GppIubStatus						_3GppIubStatus;
		Tac49xRtcpXrVoiceQualityStatisticsEvent RtcpXrVoiceQualityStatisticsEvent;
		Tac49xRtcpXrMarkovModelStatisticsEvent	RtcpXrMarkovModelStatisticsEvent;
		Tac49xRtcpXrAlertStatus					RtcpXrAlertStatus;
		Tac49xRtpSequenceNumberReceivedEvent	RtpSequenceNumberReceivedEvent;
		Tac49xSwitchToModemEvent				SwitchToModemEvent;
        U16 Payload[MAX_EVENT_MESSAGE_LENGTH];
		} u;
	} Tac49xPacketizerStatusOrEventPayload;

    /** @} */

typedef volatile struct
	{
	U16 FrameEnergy			            :8;  /*!< Frame Energy of the Channel input PCM signal in -dBm*/
	                                    
	U16 ChannelState                    :3;  /*!< ::Tac49xChannelState*/
	U16                                 :5;
                                        
	U16 PacketsCounter_Msb              :8;  /*!< Counts the number of packets transmitted to the network within device status interval*/
	U16 PacketsCounter_Lsb              :8;
                                        
    U16                                 :8;
	U16                                 :8;
    U16                                 :8;
	U16                                 :8;
	} Tac49xChannelStatusPayload;

    /**
     * @defgroup Tac49xDeviceStatusPayload Tac49xDeviceStatusPayload
     * @ingroup PACKET_STRUCTURES
     * @brief
     * The device status packet reports information and statistics on all channels.
     * @{
     */

    /**
     * <BR>
     * The device status packet reports information and statistics on all channels.
     * This packet is sent every Tac49xOpenDeviceConfigurationAttr::DeviceStatusInterval.
     * The packet is enabled only if the Tac49xOpenDeviceConfigurationAttr::DeviceStatusPacketEnable bit is set to 1.
     */

typedef volatile struct
    {
    U16 TimeStamp_MswMsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_MswLsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswMsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswLsb                : 8;  /*!< 32-bit time stamp*/
    U16                                 : 8;
	U16                                 : 8;
    U16                                 : 8;
	U16                                 : 8;
	Tac49xChannelStatusPayload ChannelStatusPayload[AC49X_NUMBER_OF_CHANNELS];  /*!< ::Tac49xChannelStatusPayload*/
    } Tac49xDeviceStatusPayload;

    /** @} */

typedef volatile struct
	{
	U16                                 :8;
	U16 DtmfErasureCounter	            :8;
                                        
	U16                                 :8;
	U16                                 :8;
                                        
	U16                                 :8;
	U16                                 :8;

	U16                                 :8;
	U16                                 :8;
	} Tac49xChannelInformationPayload;

typedef volatile struct
    {
    U16 TimeStamp_MswMsb                : 8;
    U16 TimeStamp_MswLsb                : 8;
    U16 TimeStamp_LswMsb                : 8;
    U16 TimeStamp_LswLsb                : 8;
                                        
    U16                                 : 8;
                                        
	U16 CoreUsage			            : 7;
	U16 RealTimeError				    : 1;
                                        
	U16						            : 8;
	U16						            : 8;
	Tac49xChannelInformationPayload ChannelInformationPayload[AC49X_NUMBER_OF_CHANNELS];
    } Tac49xDebugInformationStatusPayload;

    /**
     * @defgroup Tac49xFaxOrDataSignalStatusPayload Tac49xFaxOrDataSignalStatusPayload
     * @ingroup PACKET_STRUCTURES
     * @brief
     * This packet is transmitted during bypass or fax/data event modes.
     * @{
     */

    /**
     * <BR>
     * This packet is transmitted during bypass or fax/data event modes. It informs the Host what kind of 
     * signal was detected in the PCM input (Fax, V22, V23, etc.).
     */

typedef volatile struct                 
    {                                   
    U16 TimeStamp_MswMsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_MswLsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswMsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswLsb                : 8;  /*!< 32-bit time stamp*/
                                        
    U16	SignalDetectedOnDecoderOutput	: 5; /*!< Refer to ::Tac49xFaxOrDataSignalEvent*/
    U16								    : 2;
    U16	FaxBypassFlag					: 1;  /*!< 0 = Voice, Data Bypass or Fax Relay. @n 1 = Fax Bypass is active (Fax call transfer in VBD mode).*/
    U16	SignalDetectedOnEncoderInput	: 8; /*!< Refer to ::Tac49xFaxOrDataSignalEvent*/
	} Tac49xFaxOrDataSignalStatusPayload;

    /** @} */

typedef volatile struct
    {
    U16 TimeStamp_MswMsb                : 8;
    U16 TimeStamp_MswLsb                : 8;
    U16 TimeStamp_LswMsb                : 8;
    U16 TimeStamp_LswLsb                : 8;

    U16                                 : 4;
    U16 ErrorIndex                      : 4;

    U16 EventIndex                      : 4;
    U16                                 : 4;

    U16 SignalLevel_Msb                 : 3;
    U16                                 : 5;

    U16 SignalLevel_Lsb                 : 8;

    U16 SignalLevel_msec_Msb            : 2;
    U16                                 : 6;

    U16 SignalLevel_msec_Lsb            : 8;

    U16 SignalFrequency_Msb             : 8;
    U16 SignalFrequency_Lsb             : 8;

    U16 BfiOccurred_msec_Msb            : 8;
    U16 BfiOccurred_msec_Lsb            : 8;
    } Tac49xTrunkTestingMeasurementEventPayload;

    /**
     * @defgroup Tac49xBerTestReportEventPayload Tac49xBerTestReportEventPayload
     * @ingroup PACKET_STRUCTURES
     * @brief
     * The BER report status packet is sent during a BER test.
     * @{
     */

    /**
     * <BR>
     * The BER report status packet is sent during a BER test. It reports BER statistics, errors and events.
     */

typedef volatile struct
	{          
		   /* #2 */
    U16 TimeStamp_MswMsb               :8;
    U16 TimeStamp_MswLsb               :8;
		   /* #3 */
    U16 TimeStamp_LswMsb               :8;
    U16 TimeStamp_LswLsb			   :8;
			/* #4 */
    U16 Pattern                        :3; /*!< Detected pattern, refer to ::Tac49xBerTestInjectedPattern*/
    U16                                :4;
    U16 SynchronizationState           :1; /*!< @li 0 = Synchronization Lost BER >= 0.2 during one second @li 1 = Synchronization Found*/
                                       
    U16 Rate                           :2; /*!< Refer to ::Tac49xBerTestRate*/
    U16                                :2;
    U16 Event                          :4; /*!< Refer to ::Tac49xBerTestReportEventIndex*/
			/* #5 */                       
    U16 TestDuration_MswMsb            :7; /*!< 31-bit test duration, until the report, in seconds.*/
    U16                                :1;
                                       
    U16 TestDuration_MswLsb            :8; 
			/* #6 */                       
    U16 TestDuration_LswMsb            :8;
                                       
    U16 TestDuration_LswLsb            :8;
			/* #7 */                       
    U16 NumberOfErrorBits_MswMsb       :7; /*!< 31-bit number of error bits detected during the test.*/
    U16                                :1;
                                       
    U16 NumberOfErrorBits_MswLsb       :8;
			/* #8 */                       
    U16 NumberOfErrorBits_LswMsb       :8;
                                       
    U16 NumberOfErrorBits_LswLsb       :8;
			/* #9 */                       
    U16 BerMantissa_Msb                :4; /*!< Floating point presentation of the BER x: x*(10-y) The Mantissa is in Q11 (Mantissa = BER Mantissa/2048)*/
    U16 BerExponent                    :4; /*!< Floating point presentation of the BER y: x*(10-y) The BER measurement range is 10-2 to 10-15.*/
                                       
    U16 BerMantissa_Lsb                :8;
			/* #10 */                       
    U16 ErroredSeconds_MswMsb          :7; /*!< 31-bit counter that counts the seconds which have one or more bits in error.*/
    U16                                :1;
                                       
    U16 ErroredSeconds_MswLsb          :8;
			/* #11 */                       
    U16 ErroredSeconds_LswMsb          :8;
                                       
    U16 ErroredSeconds_LswLsb          :8;
			/* #12 */
    U16 SeverelyErroredSeconds_MswMsb  :7; /*!< 31-bit counter that counts the seconds which have BER >= 10-3*/
    U16                                :1;
           
    U16 SeverelyErroredSeconds_MswLsb  :8;
			/* #13 */
    U16 SeverelyErroredSeconds_LswMsb  :8;

    U16 SeverelyErroredSeconds_LswLsb  :8;
			/* #14 */
    U16 UnavailableTime_MswMsb         :7; /*!< 31-bit counter that counts seconds that are defined as unavailable. The unavailable period begins when the BER, in each second, is worse than 10-3 for a period of ten consecutive seconds. A period of available time begins with the first second of a period of ten consecutive seconds, each of which has a BER better than 10-3.*/
    U16                                :1;

    U16 UnavailableTime_MswLsb         :8;
			/* #15 */
    U16 UnavailableTime_LswMsb         :8;

    U16 UnavailableTime_LswLsb         :8;
			/* #16 */
    U16 TimeInSync_MswMsb              :7; /*!< 31-bit total time, in seconds, of synchronization.*/
    U16                                :1;
                                       
    U16 TimeInSync_MswLsb              :8;
			/* #17 */                      
    U16 TimeInSync_LswMsb              :8;
                                       
    U16 TimeInSync_LswLsb              :8;
			/* #18 */                      
    U16 BlocksSent_MswMsb              :7; /*!< 47-bit total number of patterns sent during the test.*/
    U16                                :1;
                                       
    U16 BlocksSent_MswLsb              :8;
			/* #19 */                      
    U16 BlocksSent1_Msb                :8;
    U16 BlocksSent1_Lsb                :8;
			/* #20 */                      
    U16 BlocksSent_LswMsb              :8;
                                       
    U16 BlocksSent_LswLsb              :8;
			/* #21 */
    U16 Slips_MswMsb                   :7;  /*!< 31-bit counter that counts the number of times synchronization was lost.*/
    U16                                :1;
                                       
    U16 Slips_MswLsb                   :8;
			/* #22 */                      
    U16 Slips_LswMsb                   :8;
                                       
    U16 Slips_LswLsb                   :8;
			/* #23 */
    U16 NetworkErroredSeconds_MswMsb   :7;  /*!< 31-bit counter that counts the seconds during which packet loss occurs due to a network problem. In these seconds, statistics are not calculated.*/
    U16                                :1;

    U16 NetworkErroredSeconds_MswLsb   :8;
			/* #24 */
    U16 NetworkErroredSeconds_LswMsb   :8;

    U16 NetworkErroredSeconds_LswLsb   :8;
    } Tac49xBerTestReportEventPayload;

    /** @} */

    /**
     * @defgroup Tac49xUtopiaStatusPayload Tac49xUtopiaStatusPayload
     * @ingroup PACKET_STRUCTURES
     * @brief
     * A UTOPIA Status packet is sent in response to the function ::Ac49xUtopiaStatusRequest.
     * @{
     */

    /**
     * <BR>
     * A UTOPIA Status packet is sent in response to the function ::Ac49xUtopiaStatusRequest. 
     * @n The packet presents statistics pertaining to the UTOPIA port. All the counters are 32 bits unsigned.
     * They wrap around when they reach the maximum value. The counters are reset in the function 
     * ::Ac49xOpenDeviceConfiguration and when the Clear bit in the function ::Ac49xUtopiaStatusRequest is set.
     * @li TimeStamp = The timestamp value of a system event payload.
     * @li TransmittedCells = Total number of cells transmitted through the UTOPIA port by the AC49x device.
     * @li ReceivedCells = Total number of good cells received through the UTOPIA port by the AC49x device.
     * @li TransmittedPackets = Total number of packets transmitted through the UTOPIA port by the AC49x device. 
     * @li ReceivedPackets = Total number of good packets received through the UTOPIA port by the AC49x device (excluding loopbacked cells)
     * @li ReceivedIllegalCells = Total number of bad cells received through the UTOPIA port by the AC49x device (mismatch in the segment or packet size).
     * @li ReceivedIllegalPackets = Total number of bad packets received through the UTOPIA port by the AC49x device (bad channel ID, protocol)
     * @li UnTransmittedPacketsDueToOverrun = Total number of packets which were not transmitted through the UTOPIA port by the AC49x device due to an overrun in the AC49x transmit buffer.
     */

typedef volatile struct
    {
    U16 TimeStamp_MswMsb						: 8;
    U16 TimeStamp_MswLsb						: 8;
    U16 TimeStamp_LswMsb						: 8;
    U16 TimeStamp_LswLsb						: 8;

    U16 TransmittedCells_MswMsb                 : 8;
    U16 TransmittedCells_MswLsb                 : 8;
    U16 TransmittedCells_LswMsb                 : 8;
    U16 TransmittedCells_LswLsb                 : 8;

    U16 ReceivedCells_MswMsb					: 8;
    U16 ReceivedCells_MswLsb					: 8;
    U16 ReceivedCells_LswMsb					: 8;
    U16 ReceivedCells_LswLsb					: 8;

    U16 TransmittedPackets_MswMsb               : 8;
    U16 TransmittedPackets_MswLsb               : 8;
    U16 TransmittedPackets_LswMsb               : 8;
    U16 TransmittedPackets_LswLsb               : 8;

    U16 ReceivedPackets_MswMsb					: 8;
    U16 ReceivedPackets_MswLsb					: 8;
    U16 ReceivedPackets_LswMsb					: 8;
    U16 ReceivedPackets_LswLsb					: 8;

    U16 ReceivedIllegalCells_MswMsb             : 8;
    U16 ReceivedIllegalCells_MswLsb             : 8;
    U16 ReceivedIllegalCells_LswMsb             : 8;
    U16 ReceivedIllegalCells_LswLsb             : 8;

    U16 ReceivedIllegalPackets_MswMsb           : 8;
    U16 ReceivedIllegalPackets_MswLsb           : 8;
    U16 ReceivedIllegalPackets_LswMsb           : 8;
    U16 ReceivedIllegalPackets_LswLsb           : 8;

    U16 UnTransmittedPacketsDueToOverrun_MswMsb : 8;
    U16 UnTransmittedPacketsDueToOverrun_MswLsb : 8;
    U16 UnTransmittedPacketsDueToOverrun_LswMsb : 8;
    U16 UnTransmittedPacketsDueToOverrun_LswLsb : 8;

    U16 TransmittedLoopedBackCells_MswMsb		: 8;
    U16 TransmittedLoopedBackCells_MswLsb		: 8;
    U16 TransmittedLoopedBackCells_LswMsb		: 8;
    U16 TransmittedLoopedBackCells_LswLsb		: 8;
    } Tac49xUtopiaStatusPayload;

    /** @} */

typedef volatile struct
    {
    U16 TimeStamp_MswMsb			: 8;
    U16 TimeStamp_MswLsb			: 8;
    U16 TimeStamp_LswMsb			: 8;
    U16 TimeStamp_LswLsb			: 8;
	
	U16	                            : 8;
	U16	MessageType					: 8;

	U16	DataSize_Msb				: 8;
	U16	DataSize_Lsb				: 8;

    U8  Data[MAX_SS7_DATA_SIZE];
    } Tac49xSs7Mtp1StatusPayload;

        /*******/
		/* TFO */
        /*******/

#ifndef NDOC
typedef volatile struct
    {
    U16 PdcEfr			: 1;
    U16 FrAmrWb			: 1;
    U16 UmtsAmrWb		: 1;
    U16 				: 4;
	U16 CodecListEnable	: 1;
						
    U16 GsmFr			: 1;
    U16 GsmHr 			: 1;
    U16 GsmEfr 			: 1;
    U16 FrAmr			: 1;
    U16 HrAmr			: 1;
    U16 UmtsAmr			: 1;
    U16 UmtsAmr2		: 1;
    U16 TdmaEfr			: 1;
	} Tac49xTandemFreeOperationCodecList;

typedef volatile struct
    {
    U16 				: 7;
    U16					: 1;

	U16 _4k75			: 1;				   
	U16 _5k15			: 1;			  	
	U16 _5k9			: 1;				
	U16 _6k7			: 1;			  	
	U16 _7k4			: 1;				
	U16 _7k95			: 1;			  	
	U16 _10k2			: 1;				
	U16 _12k2			: 1;				
	} Tac49xTandemFreeOperationNarrowAmrRates;

typedef volatile struct
    {
	U16 _4k75			: 1;				   
	U16 _5k15			: 1;			  	
	U16 _5k9			: 1;				
	U16 _6k7			: 1;			  	
	U16 _7k4			: 1;				
	U16 _7k95			: 1;			  	
	U16 _10k2			: 1;				
	U16 _12k2			: 1;				
	 
    U16 Wide8			: 1;
    U16 				: 6;
    U16					: 1;
	} Tac49xTandemFreeOperationWideAmrRates;

typedef volatile struct
    {
	union
		{
		Tac49xTandemFreeOperationNarrowAmrRates Narrow;
		Tac49xTandemFreeOperationWideAmrRates   Wide;
		}u;
	} Tac49xTandemFreeOperationAmrCodecRates;

typedef volatile struct
    {
    U16										MaximumNumberOfAmrRates	: 4;
    U16																: 4;
										
    U16										Type					: 8;
	Tac49xTandemFreeOperationAmrCodecRates	SupportedAmrRates;
	} Tac49xTandemFreeOperationCodec;

typedef volatile struct
    {		 /* word 3 */
    U16												System						  	: 4;
    U16												  							  	: 2;
    U16												ActiveAmrCodecRateOptimization	: 1;
    U16																  				: 1;
																						
    U16												Function					  	: 8;
			 /* word 4 */																
	Tac49xTandemFreeOperationCodecList				CodecList;							
			 /* words 5,6 */															
	Tac49xTandemFreeOperationCodec					SupportedCodec;						
			 /* word 7 */																
	Tac49xTandemFreeOperationAmrCodecRates			ActiveAmrCodecRates;				
			 /* word 8 */																
	U16																			  	: 8;
																						
	U16												NumberOfOptionalAmrCodecs	  	: 3;
	U16																			  	: 5;
			 /* words 9 - 18 */
	Tac49xTandemFreeOperationCodec					OptionalAmrCodec[MAX_NUMBER_OF_TANDEM_FREE_OPERATION_OPTIONAL_AMR_CODECS];
    } Tac49xTandemFreeOperationPayload;

    /**
     * <BR>
     * This status packet is sent in response to the function ::Ac49xModuleStatusRequest when the parameter ::Tac49xModuleStatusRequestAttr.ModuleType equals ECHO_CANCELER.
     */

typedef volatile struct
	{
	U16	NoiseLevel										: 8;  /*!< Noise Level.*/
	U16	SignalLevel										: 8;  /*!< Signal Level.*/
	U16	SaturationCounter								: 8;  /*!< Reserved for internal use.*/
	U16 ResidualEchoReturnLoss							: 8;  /*!< Residual echo return loss.*/
														
	U16 CrossCorrelationMaximumIndex_Msb				: 3;  /*!< Reserved for internal use.*/
	U16 EchoCancelerLength								: 5;  /*!< Echo Canceler Length, refer to ::Tac49xEchoCancelerLength.*/
														
	U16 CrossCorrelationMaximumIndex_Lsb				: 8;
														
	U16 DoubleTalkFlag									: 6;  /*!< Reserved for internal use.*/
	U16 HybridLoss										: 2;  /*!< Hybrid Loss, refer to ::Tac49xHybridLoss.*/
														
	U16 FilterLength									: 6;  /*!< Reserved for internal use.*/
	U16 NlpMode											: 2;  /*!< Reflects the NLP status. NLP is disabled (for example) due to Fax Relay or Fax Bypass. In this case: @n 1. The NLP configuration is overridden @n 2. Parameter 'NLP Status' reflects the actual status of the NLP rather than the NLP configuration. @n Refer to ::Tac49xEchoCancelerNonLinearProcessor.*/
														
	U16													: 2;
	U16 ReconvergenceDetection							: 1;  /*!< Reconvergence detection. 0 = Reconvergence did not occur; 1 = Reconvergence has occurred.*/
	U16 EchoCancellerAutomaticToneResetModeActivation	: 1;  /*!< Echo Canceler Automatic Reset after Tone detection. 0 = Automatic reset did not occur; 1 = Automatic reset has occurred.*/
	U16 DcRemover										: 1;  /*!< DC Remover. 0 = Disabled. 1 = Enabled.*/
	U16 EchoCancelerStatus								: 1;  /*!< Echo Canceler Status. 0 = Disabled. 1 = Enabled.*/
	U16 Cng												: 1;  /*!< Comfort Noise Generation mode. 0 = Disabled. 1 = Enabled.*/
	U16 ToneDetector									: 1;  /*!< Tone Detector. 0 = Disabled. 1 = Enabled.*/

	U16	NlpThreshold									: 6;  /*!< The configured NLP threshold.*/
	U16													: 2;
	} Tac49xEchoCancelerModuleStatusPayload;

    /**
     * @defgroup Tac49xModuleStatusPayload Tac49xModuleStatusPayload
     * @ingroup PACKET_STRUCTURES
     * @brief
     * This status packet is sent in response to the function ::Ac49xModuleStatusRequest.
     * @{
     */

    /**
     * <BR>
     * This status packet is sent in response to the function ::Ac49xModuleStatusRequest.
     */

typedef volatile struct
	{
    U16 TimeStamp_MswMsb			: 8;  /*!< 32-bit time stamp.*/
    U16 TimeStamp_MswLsb			: 8;
    U16 TimeStamp_LswMsb			: 8;
    U16 TimeStamp_LswLsb			: 8;
	
	U16	                            : 8;
									  
	U16	ModuleType					: 8;  /*!< The module type, refer to ::Tac49xModuleType.*/
	union
		{
		Tac49xEchoCancelerModuleStatusPayload EchoCanceler;
		} u;
	} Tac49xModuleStatusPayload;

   /** @} */

typedef volatile struct
    {
    U16 TimeStamp_MswMsb			: 8;  /*!< 32-bit time stamp.*/
    U16 TimeStamp_MswLsb			: 8;
    U16 TimeStamp_LswMsb			: 8;
    U16 TimeStamp_LswLsb			: 8;

	U16         					: 8;
	U16 MessageType					: 8;

	U16 MessageSize_Msb				: 8;
	U16 MessageSize_Lsb				: 8;

    U8  Message[MAX_DUA_MESSAGE_LENGTH];
    } Tac49xDuaFramerStatusPayload;

    /**
     * @defgroup Tac49xUtopiaTestErrorEventPayload Tac49xUtopiaTestErrorEventPayload
     * @ingroup PACKET_STRUCTURES
     * @brief
     * This status packet is sent if the UTOPIA test is activated and an error was detected.
     * @{
     */

    /**
     * <BR>
     * This status packet is sent if the UTOPIA test is activated and an error was detected.
     */

typedef volatile struct
    {
    U16 TimeStamp_MswMsb			: 8;
    U16 TimeStamp_MswLsb			: 8;
    U16 TimeStamp_LswMsb			: 8;
    U16 TimeStamp_LswLsb			: 8;
	
	U16	                            : 8;
									  
	U16	ErrorType					: 6;  /*!< Refer to ::Tac49xUtopiaTestErrorType.*/
	U16	                            : 2;

    U8  Payload[MAX_PAYLOAD_SIZE];
	} Tac49xUtopiaTestErrorEventPayload;

   /** @} */

typedef volatile struct
    {
		/* 3-4 */
	U16 TimeStamp_MswMsb			: 8;
    U16 TimeStamp_MswLsb			: 8;
    U16 TimeStamp_LswMsb			: 8;
    U16 TimeStamp_LswLsb			: 8;
		/* 5 */	
	U16	                            : 8;

	U16 Conference0State			: 1;
	U16 Conference1State			: 1;
	U16 Conference2State			: 1;
	U16 Conference3State			: 1;
	U16	                            : 4;
		/* 6 */
	U16							    : 1;
	U16	RegularLegStateOfChannel2   : 1;
	U16	RegularLegTypeOfChannel2	: 2;
	U16							    : 1;
	U16	RegularLegStateOfChannel3   : 1;
	U16	RegularLegTypeOfChannel3	: 2;
	
	U16							    : 1;
	U16	RegularLegStateOfChannel0   : 1;
	U16	RegularLegTypeOfChannel0	: 2;
	U16							    : 1;
	U16	RegularLegStateOfChannel1   : 1;
	U16	RegularLegTypeOfChannel1	: 2;
		/* 7 */
	U16							    : 1;
	U16	RegularLegStateOfChannel6   : 1;
	U16	RegularLegTypeOfChannel6	: 2;
	U16							    : 1;
	U16	RegularLegStateOfChannel7   : 1;
	U16	RegularLegTypeOfChannel7	: 2;
	
	U16							    : 1;
	U16	RegularLegStateOfChannel4   : 1;
	U16	RegularLegTypeOfChannel4	: 2;
	U16							    : 1;
	U16	RegularLegStateOfChannel5   : 1;
	U16	RegularLegTypeOfChannel5	: 2;
		/* 8 */
	U16							    : 1;
	U16	RegularLegStateOfChannel10  : 1;
	U16	RegularLegTypeOfChannel10	: 2;
	U16							    : 1;
	U16	RegularLegStateOfChannel11  : 1;
	U16	RegularLegTypeOfChannel11	: 2;
	
	U16							    : 1;
	U16	RegularLegStateOfChannel8   : 1;
	U16	RegularLegTypeOfChannel8	: 2;
	U16							    : 1;
	U16	RegularLegStateOfChannel9   : 1;
	U16	RegularLegTypeOfChannel9	: 2;
	} Tac49x4WayConferenceStatusPayload;

typedef volatile struct
    {
	U16 MemberLegType						: 3;
	U16										: 3;
	U16 MemberActiveSpeakerState			: 1;
	U16 MemberStatus						: 1;

	U16 MemberChannelId						: 8;
	} Tac49x4WayConferenceMemberInfoPayload;

typedef volatile struct
    {
		/* 3-4 */
	U16 TimeStamp_MswMsb						: 8;
    U16 TimeStamp_MswLsb						: 8;
    U16 TimeStamp_LswMsb						: 8;
    U16 TimeStamp_LswLsb						: 8;
		/* 5 */
	U16											: 8;

	U16 ConferenceId							: 3;
	U16											: 5;
		/* 6 */
	U16 Member2HearsMember0						: 1;
	U16 Member2HearsMember1						: 1;
	U16 Member2HearsMember2						: 1;
	U16 Member2HearsMember3						: 1;
	U16 Member3HearsMember0						: 1;
	U16 Member3HearsMember1						: 1;
	U16 Member3HearsMember2						: 1;
	U16 Member3HearsMember3						: 1;
	
	U16 Member0HearsMember0						: 1;
	U16 Member0HearsMember1						: 1;
	U16 Member0HearsMember2						: 1;
	U16 Member0HearsMember3						: 1;
	U16 Member1HearsMember0						: 1;
	U16 Member1HearsMember1						: 1;
	U16 Member1HearsMember2						: 1;
	U16 Member1HearsMember3						: 1;
		/* 7..10 */
	Tac49x4WayConferenceMemberInfoPayload	MemberInfo[MAX_NUMBER_OF_4_WAY_CONFERENCE_PARTICIPANTS_PER_CONFERENCE];
	} Tac49x4WayConferenceDetailsEventPayload;


typedef volatile struct
    {
		/* 3-4 */
	U16 TimeStamp_MswMsb						: 8;
    U16 TimeStamp_MswLsb						: 8;
    U16 TimeStamp_LswMsb						: 8;
    U16 TimeStamp_LswLsb						: 8;
		/* 5 */
	U16	ErrorIndex								: 8;

	U16 Action									: 3;
	U16											: 5;
		/* 6 */
	U16	DataSize_Msb							: 3;
	U16											: 5;

	U16	DataSize_Lsb							: 8;
		/* 7... */
	U8  Payload[MAX_PAYLOAD_SIZE - sizeof(Tac49xHpiPacketHeader) - sizeof(Tac49xProprietaryPacketHeader) - 8];
	} Tac49xProtectionTestResponseEventPayload;

typedef volatile struct
    {
		/* 3-4 */
	U16 TimeStamp_MswMsb						: 8;
    U16 TimeStamp_MswLsb						: 8;
    U16 TimeStamp_LswMsb						: 8;
    U16 TimeStamp_LswLsb						: 8;
		/* 5 */
	U16	RandomNumber_Msb						: 8;
	U16	RandomNumber_Lsb						: 8;
	} Tac49xRandomNumberResponseEventPayload;

typedef volatile struct
    {
		/* 3-4 */
	U16 TimeStamp_MswMsb						: 8;
    U16 TimeStamp_MswLsb						: 8;
    U16 TimeStamp_LswMsb						: 8;
    U16 TimeStamp_LswLsb						: 8;
		/* 5 */
	U16											: 8;
												
    U16 Module									: 3;
    U16											: 5;
    union
        {
        Tac49xEchoCancelerDebugPayload EchoCanceler;
        } u;
	} Tac49xModuleDebugEventPayload;

typedef volatile struct                     
    {
		/* 6 */
	U16				 						: 8;
   
	U16 ReasonIdentifierCode				: 2;
	U16				 						: 6;
		/* 7 */
	U16 RtpTxSequenceNumber_Msb				: 8;
	U16 RtpTxSequenceNumber_Lsb				: 8;
		/* 8-9 */
	U16 RtpTxTimeStamp_MswMsb				: 8;
	U16 RtpTxTimeStamp_MswLsb				: 8;
	U16 RtpTxTimeStamp_LswMsb				: 8;
	U16 RtpTxTimeStamp_LswLsb				: 8;
		/* 10-11 */
	U16 RtpTxSsrc_MswMsb					: 8;
	U16 RtpTxSsrc_MswLsb					: 8;
	U16 RtpTxSsrc_LswMsb					: 8;
	U16 RtpTxSsrc_LswLsb					: 8;
	} Tac49xModemRelayEndedEventPayload;

typedef volatile struct                     
    {
		/* 6 */
	U16										: 7;
	U16 Modulation							: 1;

	U16										: 8;
		/* 7 */
	U16 NumberOfRetrains_Msb					: 8;
	U16 NumberOfRetrains_Lsb					: 8;
		/* 7 */
	U16 NumberOfRateNegotiations_Msb			: 8;
	U16 NumberOfRateNegotiations_Lsb			: 8;
		/* 8 */
	U16 TxRate_Msb							: 8;
	U16 TxRate_Lsb							: 8;
		/* 9 */
	U16 RxRate_Msb							: 8;
	U16 RxRate_Lsb							: 8;
		/* 10 */
	U16 RoundTripDelay_Msb					: 8;
	U16 RoundTripDelay_Lsb					: 8;
		/* 11 */
	U16 RxSymbolRate_Msb						: 8;
	U16 RxSymbolRate_Lsb						: 8;
		/* 12 */
	U16 TxSymbolRate_Msb						: 8;
	U16 TxSymbolRate_Lsb						: 8;
		/* 13 */
	U16										: 8;
	U16 ConnectionFailReason					: 8;
	} Tac49xMoipSystemLogEventPayload;

typedef volatile struct
	{
	U16 R_Msb								: 8;
	U16 R_Lsb								: 8;

	U16 I_Msb								: 8;
	U16 I_Lsb								: 8;
	} Tac49xConstellationPointPayload;

typedef volatile struct
	{
		/* 5 */
	U16 TxStatus_Msb						: 8;
	U16 TxStatus_Lsb						: 8;
		/* 6 */
	U16 RxStatus_Msb						: 8;
	U16 RxStatus_Lsb						: 8;
		/* 7 */
	U16 Error_Msb							: 8;
	U16 Error_Lsb							: 8;
		/* 8 */
	U16 Pdsnr_Msb							: 8;
	U16 Pdsnr_Lsb							: 8;
		/* 9 */
	U16 Phase2State_Msb						: 8;
	U16 Phase2State_Lsb						: 8;
		/* 10 */
	U16 TxRate_Msb							: 8;
	U16 TxRate_Lsb							: 8;
		/* 11 */
	U16 RxRate_Msb							: 8;
	U16 RxRate_Lsb							: 8;
		/* 12 */
	U16 VirtualCircuits_Msb					: 8;
	U16 VirtualCircuits_Lsb					: 8;
		/* 13 */
	U16										: 8;
	U16										: 8;
		/* 14 */
	U16										: 8;
	U16 DataPumpInternalState				: 8;
		/* 15 */
	U16 RoundTripDelay_Msb					: 8;
	U16 RoundTripDelay_Lsb					: 8;
		/* 16 */
	U16 TimingOffset_Msb					: 8;
	U16 TimingOffset_Lsb					: 8;
		/* 17 */
	U16 CarrierOffset_Msb					: 8;
	U16 CarrierOffset_Lsb					: 8;
		/* 18 */
	U16 WaitForAnswerToneCounter_Msb		: 8;
	U16 WaitForAnswerToneCounter_Lsb		: 8;
		/* 19-20 */
	U16 RxLevel_MswMsb						: 8;
	U16 RxLevel_MswLsb						: 8;
	U16 RxLevel_LswMsb						: 8;
	U16 RxLevel_LswLsb						: 8;
		/* 21-22 */
	U16 TxLevel_MswMsb						: 8;
	U16 TxLevel_MswLsb						: 8;
	U16 TxLevel_LswMsb						: 8;
	U16 TxLevel_LswLsb						: 8;
		/* 23-24 */
	U16 RxSymbolRate_MswMsb					: 8;
	U16 RxSymbolRate_MswLsb					: 8;
	U16 RxSymbolRate_LswMsb					: 8;
	U16 RxSymbolRate_LswLsb					: 8;
		/* 25-26 */
	U16 TxSymbolRate_MswMsb					: 8;
	U16 TxSymbolRate_MswLsb					: 8;
	U16 TxSymbolRate_LswMsb					: 8;
	U16 TxSymbolRate_LswLsb					: 8;
		/* 27 */
	U16										: 8;
	U16										: 8;
		/* 28 */
	U16										: 8;
	U16 ConnectionFailReason				: 8;
		/* 29 */
	U16										: 8;
	U16										: 8;
		/* 30 */
	U16										: 8;
	U16 P2mohStatusModemOnHoldTimeout		: 8;
		/* 31 */
	U16										: 8;
	U16										: 8;
		/* 32 */
	U16										: 8;
	U16 P2mohStatusNextState				: 8;
		/* 33 */
	U16 SubStandard							: 8;
	U16 CurrentConnectionConfig				: 8;
		/* 34 */
	U16 NumberOfPointsInConstellation_Msb	: 8;
	U16 NumberOfPointsInConstellation_Lsb	: 8;
		/* 35-36 */
	U16 LocalRenegotiationCount_MswMsb		: 8;
	U16 LocalRenegotiationCount_MswLsb		: 8;
	U16 LocalRenegotiationCount_LswMsb		: 8;
	U16 LocalRenegotiationCount_LswLsb		: 8;
		/* 37-38 */
	U16 RemoteRenegotiationCount_MswMsb		: 8;
	U16 RemoteRenegotiationCount_MswLsb		: 8;
	U16 RemoteRenegotiationCount_LswMsb		: 8;
	U16 RemoteRenegotiationCount_LswLsb		: 8;
		/* 39-70 */
	Tac49xConstellationPointPayload ConstellationPoints[MAX_NUMBER_OF_CONSTELLATION_POINTS];
	} Tac49xDataPumpStatusEventPayload;

typedef volatile struct
	{
    U16 TimeStamp_MswMsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_MswLsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswMsb                : 8;  /*!< 32-bit time stamp*/
    U16 TimeStamp_LswLsb                : 8;  /*!< 32-bit time stamp*/
	U16                                 : 8;
	U16 StatusOrEventIndex			    : 8;  /*!< ::Tac49xModemEventIndex*/
	union
		{
		Tac49xModemRelayEndedEventPayload		ModemRelayEnded;
		Tac49xMoipSystemLogEventPayload			MoipSystemLog;
		Tac49xDataPumpStatusEventPayload		DataPumpStatus;
        U8 Payload[MAX_EVENT_MESSAGE_LENGTH];
		} u;
	} Tac49xModemRelayEventPayload;

#if (AC49X_DEVICE_TYPE == AC490_DEVICE)
typedef volatile struct
	{
	U16 ErrorAddress_MswMsb			: 8;
	U16 ErrorAddress_MswLsb			: 8;
	U16 ErrorAddress_LswMsb			: 8;
	U16 ErrorAddress_LswLsb			: 8;

	U16	TestType_Msb				: 8;
	U16	TestType_Lsb				: 8;

	U16	ExpectedValue_Msb           : 8;
	U16	ExpectedValue_Lsb           : 8;

	U16	ReceivedValue_Msb           : 8;
	U16	ReceivedValue_Lsb           : 8;

	U16	                            : 8;
	U16	                            : 8;
	}Tac49xSingleEmifTestErrorInformationPayload;

typedef volatile struct
	{
	Tac49xSingleEmifTestErrorInformationPayload SingleEmifTestErrorInformation[EMIF_TEST__ERROR_INFORMATION_SIZE/sizeof(Tac49xSingleEmifTestErrorInformationPayload)];
	} Tac49xEmifTestErrorInformationPayload;
#endif /*(AC49X_DEVICE_TYPE == AC490_DEVICE)*/

#endif /* NDOC */

#endif /*~~AC49X_BIG_ENDIAN_BIT_FIELDS_H*/





