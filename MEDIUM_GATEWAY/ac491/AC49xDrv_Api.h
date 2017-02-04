/** @file
 *  @brief This file contains the definitions of the following lists of API functions:
 *  @li @link DEBUG_AND_RECOVERY Debug and Recovery@endlink
 *  @li @link DEVICE_MEMORY_ACCESS Device Memory Access Tests@endlink
 *  @li @link PACKET_HANDLING Packet Handling@endlink
 *  @li @link DEVICE_CONFIGURATION Device Configuration@endlink
 *  @li @link CHANNEL_CONFIGURATION Channel Configuration@endlink
 *  @li @link DEVICE_RESET_AND_DOWNLOAD Device Reset and Download@endlink
 *  @li @link CONVENIENT_GROUP Convenient Group@endlink (High-Level Initialization and Setup)
 */


/************************************************************************************/
/* Ac49xDrv_Api.h - AC49x Device Drivers / API Function Prototypes					*/
/*																					*/	
/* Copyright (C) 2000 AudioCodes Ltd.												*/	
/*																					*/
/* Modifications :																	*/	
/*																					*/
/* 1/7/01 - Coding started.															*/	
/*																					*/
/************************************************************************************/

#ifndef AC49XLO_H
#define AC49XLO_H 

#if __cplusplus
extern "C" {
#endif

#ifndef FALSE
#define FALSE    0
#endif

#ifndef TRUE
#define TRUE     1
#endif

/*******************************************/
/*******************************************/
/**       API Function Prototypes         **/
/*******************************************/
/*******************************************/

            /* HCRF Routines */

    /**
     * @defgroup Ac49xHcrfSetHostControlRegisterAddresses Ac49xHcrfSetHostControlRegisterAddresses
     * @ingroup DEBUG_AND_RECOVERY
     * @brief 
     * Sets the Host control register addresses.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xHcrfSetHostControlRegisterAddresses function sets the Host control register addresses. The function 
     * should be called in the initialization phase, before booting the VoPP. It is called by the ::Ac49xInitDriver
     * function.
     * 
     * @param
     * This function has no parameters.
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

void Ac49xHcrfSetHostControlRegisterAddresses(void);

    /** @} */

    /**
     * @defgroup Ac49xHcrfReadRegister Ac49xHcrfReadRegister
     * @ingroup DEBUG_AND_RECOVERY
     * @brief 
     * Reades the content of the device's status registers.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xHcrfReadRegister function reades the content of the device's status registers. This function should be 
     * used in AC491 VoPP when a watch-dog event is triggered. The host application should read the status registers of 
     * all 6 cores of the malfunctioned VoPP to verify which core is not functioning. In the malfunctioned core the
     * Tac49xHcrfStatusRegister::WatchdogTimeoutFlag is set to 1.
     * 
     * @param HcrfContext  [in] Must be set to HCRF_CONTEXT__STATUS_REGISTER. Refer to ::Tac49x_HcrfContext.
     * @param Device       [in] The core ID.
     * @param pIn4Bytes    [in] Pointer to a four byte array. The status register content is copied to this buffer. Refer to Tac49xHcrfStatusRegister for the status register structure.
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

void Ac49xHcrfReadRegister(Tac49x_HcrfContext  HcrfContext, int Device, char *pIn4Bytes);

    /** @} */

void Ac49xHcrfWriteRegister(Tac49x_HcrfContext HcrfContext, int Device, char *pOut4Bytes);

    /**
     * @defgroup Ac49xHcrfDeviceReload Ac49xHcrfDeviceReload
     * @ingroup DEBUG_AND_RECOVERY
     * @brief
     * Reloads a device.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xHcrfDeviceReload function reloads a device. It is available only on the AC491 VoPP and AC494/5/6/7 SoC. 
     * On the AC491 VoPP it enables the Host to perform a quick device reload with minimum usage of the Host CPU. 
     * The function should be used after a device malfunction is detected. The reload is performed from a reference device
     * on the same processor.
     * @n On the AC494/5/6/7 SoC, it enables the MIPS to perform DSP download without requiring a power-up. It is useful 
     * after a DSP malfunction or for a software update.
     * 
     * @param Device           [in] the malfunctioned core.
     * @param ReferenceDevice  [in] the correctly functioning core which will be used to reload the code from (valid on 
     *                              the AC491 only).
     * @param pProgram         [in] Pointer to the program memory block (valid on the AC494/5/6/7 SoC).
     * @param pResponseValue   [out] pointer to the response value.
     *    
     * @par Return values:
     * The function returns two values in the variable *pResponseValue:
     * @li RELOAD_COMPLETED 
     * @li or RELOAD_FAILED.
     *
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */


#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	)
	int  Ac49xHcrfDeviceReload(int Device, int ReferenceDevice,	Tac49xHcrfStatusRegisterIndex *pResponseValue);
#elif (    (AC49X_DEVICE_TYPE == AC494_DEVICE)\
        || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
        || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
		|| (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
        || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
      )
	int  Ac49xHcrfDeviceReload(int Device, char* pProgram,	Tac49xHcrfStatusRegisterIndex *pResponseValue);
#endif

    /** @} */

    /**
     * @defgroup Ac49xHcrfCompareLocalMemory Ac49xHcrfCompareLocalMemory
     * @ingroup DEBUG_AND_RECOVERY
     * @brief
     * Causes the AC491 VoPP to perform a memory comparison.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xHcrfCompareLocalMemory function is available only on the AC491 VoPP. It causes the AC491 VoPP 
     * to perform a memory comparison. The tested core compares its local memory to the local memory of the 
     * reference core. The function should be used after a core malfunction. The data returned by the function 
     * must be sent to AudioCodes Technical Support in order to trace the cause of the malfunction. 
     * If the comparison fails, the function returns the address of the unmatched values. 
     * 
     * @param Device            [in] the malfunctioned core.
     * @param ReferenceDevice   [in] the correctly functioning core which will be used to compare the code with.
     * @param pAddress          [out] pointer to the returned value of the address of the unmatched values.
     * @param pResponseValue    [out] pointer to the response value.
     *    
     * @par Return values:
     * The function returns two values in the variable *pResponseValue: LOCAL_MEMORY_TO_REFERENCE_COMPARISON_FIT 
     * or LOCAL_MEMORY_TO_REFERENCE_COMPARISON_DIFFER.
     *
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */


int  Ac49xHcrfCompareLocalMemory(int Device, int ReferenceDevice, U32 *pAddress, Tac49xHcrfStatusRegisterIndex *pResponseValue);

    /** @} */

int  Ac49xHcrfWriteMonitorInfo(	 int Device, int ReferenceDevice,				 Tac49xHcrfStatusRegisterIndex *pResponseValue);


    /**
     * @defgroup Ac49xHcrfReadMonitorInfo Ac49xHcrfReadMonitorInfo
     * @ingroup DEBUG_AND_RECOVERY
     * @brief 
     * Obtains data that is required to diagnose a cause of a malfunction.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xHcrfReadMonitorInfo function should be used if a malfunction occurs in a device. It obtains 
     * data that is required to diagnose the cause of the malfunction. The data returned by the function must 
     * be sent to AudioCodes Technical Support in order to trace the cause of the malfunction.
     * 
     * @param Device           [in] the malfunctioned device.
     * @param pCpuRegisters    [out] pointer to the returned diagnostic data.
     * @param pTraceBuffer     [out] pointer to the returned diagnostic data.
     * @param pHpiLocalMemory  [out] pointer to the returned diagnostic data.
     *    
     * @par Return values:
     * 0 - Read monitor information OK.
     * @n 1 - Read monitor information failed.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */


int  Ac49xHcrfReadMonitorInfo(int Device, char *pCpuRegisters, char *pTraceBuffer, char *pHpiLocalMemory);

    /** @} */

            /* Device Memory Access Routines */

    /**
     * @defgroup Ac49xByteOrderingTest Ac49xByteOrderingTest
     * @ingroup DEVICE_MEMORY_ACCESS
     * @brief 
     * Checks the byte ordering between the user's application and the hardware.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xByteOrderingTest function checks the byte ordering between the user's application and the hardware.
     * 
     * @param Device  [in] Dsp.
     *    
     * @par Return values:
     * 0 - Byte ordering test status OK.
     * @n 1 - Byte ordering test status failed due to swap.
     * @n 2 - Byte ordering test status failed due to HPI access problem.
     * @n 3 - Byte ordering test status failed due to low GPIO 0 (AC491 and AC491L only).
     * @n 4 - Byte ordering test status failed due to incorrect reading of device ID (AC491L only).
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */


Tac49xByteOrderingTestStatus  Ac49xByteOrderingTest(int Device);

    /** @} */


#ifndef NDOC
U32	 Ac49xHpiAccessForXaramTimingTest(int Device, U32 BottomAddress, U32 TopAddress);
#endif /* NDOC */


    /**
     * @defgroup Ac49xHpiAccessTimingTest Ac49xHpiAccessTimingTest
     * @ingroup DEVICE_MEMORY_ACCESS
     * @brief 
     * Writes and reads-back random numbers to the entire HPI memory range.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xHpiAccessTimingTest function writes and reads-back random numbers to the entire HPI memory range.
     * It reports an error if the read data is different to the written data.
     * 
     * @param Device         [in] .
     *    
     * @par Return values:
     * 0 = Timing okay.
     * @n 32-bit address = the address in which the comparison failed.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */


U32	 Ac49xHpiAccessTimingTest(int Device);

    /** @} */

#ifndef NDOC
void Ac49xDeviceInitHPI(int Device);
#endif /* NDOC */


            /* Device Register Handling */

    /**
     * @defgroup Ac49xDeviceSetControlRegister Ac49xDeviceSetControlRegister
     * @ingroup DEVICE_CONFIGURATION
     * @brief 
     * Writes to the AC49x device control register.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xDeviceSetControlRegister function writes to the AC49x device control register. The device control
     * register is used to configure packet transfer on the host port. Each time the function is invoked, 
     * the AC49x performs software reset. It should always be called immediately after program download and before the 
     * first configuration function is called. This function is invoked by the ::Ac49xSetupDevice function.
     * @n This function can be used to transfer the AC49x VoPP from 'run' state 
     * back to 'init' state. 
     * 
     * @param Device                      [in] .
     * @param pDeviceControlRegisterAttr  [in] ::Tac49xDeviceControlRegisterAttr.
     *    
     * @par Return values:
     * @n 0 = Successful operation.
     * @n RESET_DEVICE_FAILED = Unsuccessful operation.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */


int Ac49xDeviceSetControlRegister(int Device, Tac49xDeviceControlRegisterAttr *pDeviceControlRegisterAttr);

    /** @} */

            /* Block Handling Routines */

#ifndef NDOC

void Ac49xWriteBlock(int Device, U32 BlockAddress, char *pOutBlock, int NumberOfBytesToWrite);
void Ac49xReadBlock(int Device, U32 BlockAddress, char *pInBlock,  int NumberOfBytesToRead);
void Ac49xWrite4Bytes(int Device, U32 BlockAddress, char *pOut4Bytes);
void Ac49xRead4Bytes(int Device, U32 BlockAddress, char *pIn4Bytes);

#define Ac49xWriteStruct(          Device, Address,            pStructure)          Ac49xWriteBlock( Device, (int)((char*)Address),                (char*)pStructure,   sizeof(*(pStructure)))
#define Ac49xReadStruct(           Device, Address,            pStructure)          Ac49xReadBlock(  Device, (int)((char*)Address),                (char*)pStructure,   sizeof(*(pStructure)))
#define Ac49xWriteBufferDescriptor(Device, BaseAddress, Index, pBufferDescriptor)   Ac49xWrite4Bytes(Device, (int)((char*)BaseAddress)+(Index<<2), (char*)pBufferDescriptor)
#define Ac49xReadBufferDescriptor( Device, BaseAddress, Index, pBufferDescriptor)   Ac49xRead4Bytes( Device, (int)((char*)BaseAddress)+(Index<<2), (char*)pBufferDescriptor)

#endif /* NDOC */

            /* Packet Handling Routines */


    /**
     * @defgroup Ac49xTransmitPacket Ac49xTransmitPacket
     * @ingroup PACKET_HANDLING_Functions
     * @brief 
     * Transmits a packet to the VoPP.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xTransmitPacket function transmits a packet to the VoPP.
     * 
     * @param pTxPacketParams  [in] Tac49xTxPacketParams.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

#ifndef AC49X_DRIVRES_VERSION
int  Ac49xTransmitPacket(int                    Device,
                         int                    Channel,
                         char                   *pOutPacket,
                         int                    PacketSize,
                         Tac49xProtocol         Protocol,
                         Tac49xTransferMedium   TransferMedium,
                         Tac49xControl          UdpChecksumIncluded
                        );

#else
int  Ac49xTransmitPacket(Tac49xTxPacketParams *pTxPacketParams);
#endif /*AC49X_DRIVRES_VERSION*/
    /** @} */

    /**
     * @defgroup Ac49xReceivePacket Ac49xReceivePacket
     * @ingroup PACKET_HANDLING_Functions
     * @brief 
     * Receives a packet from the AC49x VoPP.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xReceivePacket function receives a packet from the AC49x VoPP.
     * @n The user receives packet from the AC49x VoPP in polling mode. This function should be called at least
     * every 5 ms for both ports (network and host). If the function returns BUFFER_DESCRIPTOR_EMPTY_ERROR, then no 
     * packets are pending. If it returns '0', then a packet was received. In this case the user should repeatedly 
     * call this function until no packets are pending.
     * @n If the user polls at an insufficient rate then the AC49x VoPP returns Tac49xSystemErrorEventPacketIndex::SYSTEM_ERROR_EVENT_PACKET_INDEX__RX_TX_HOST_AND_NETWORK_OVERRUN.
     * @n If the receive operation is successful, the function releases the packet in the buffer descriptors. 
     * If the receive operation failed (fatal error), the user should reload and reset the VoPP.
     * 
     * @param pRxPacketParams  [in] Tac49xRxPacketParams.
     *    
     * @par Return values:
     * @li 0 - Successful.
     * @li BUFFER_DESCRIPTOR_EMPTY_ERROR = No packet was received.
     * @li BAD_PACKET_ERROR = Fatal error.
     * @li RX_SEQUENCE_ERROR = Fatal error.
     * @li RX_CHECKSUM_ERROR = Fatal error.
     * @li RX_BD_INDEX_ERROR = Fatal error.
     * @li RX_PACKET_SIZE_ERROR = Fatal error.
     * @li RX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

#ifndef AC49X_DRIVRES_VERSION
int  Ac49xReceivePacket(int                     Device,
                        char                    *pInPacket,
                        int                     *pChannel,
                        int                     *pPacketSize,
                        int                     *pPayloadSize,
                        int                     *pBufferDescriptorCurrentIndex,
                        Tac49xProtocol          *pProtocol,
                        Tac49xTransferMedium    TransferMedium
                       );
#else
int  Ac49xReceivePacket(Tac49xRxPacketParams *pRxPacketParams);
#endif /*AC49X_DRIVRES_VERSION*/

    /** @} */

    /**
     * @defgroup Ac49xReleaseRxPacket Ac49xReleaseRxPacket
     * @ingroup PACKET_HANDLING_Functions
     * @brief 
     * Releases the buffer descriptor after a packet is received from the AC49x VoPP.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xReleaseRxPacket function releases the buffer descriptor after a packet is received from the AC49x VoPP.
     * This function should be called only if the ::Ac49xReceivePacket function returned an error 
     * (exluding BUFFER_DESCRIPTOR_EMPTY_ERROR).
     * 
     * @param Device                        [in] .
     * @param BufferDescriptorCurrentIndex  [in] The index returned by the ::Ac49xReceivePacket function.
     * @param TransferMedium                [in] ::Tac49xTransferMedium.
     *    
     * @par Return values:
     * @li 0 - Successful.
     * @li BUFFER_DESCRIPTOR_EMPTY_ERROR : no need to release.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int  Ac49xReleaseRxPacket(int Device, int BufferDescriptorCurrentIndex, Tac49xTransferMedium TransferMedium);

    /** @} */

    /**
     * @defgroup Ac49xCheckRxPacketReady Ac49xCheckRxPacketReady
     * @ingroup PACKET_HANDLING_Functions
     * @brief 
     * Checks the Buffer Descriptor for any packet waiting to be received from the AC49x VoPP.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xCheckRxPacketReady function checks the Buffer Descriptor for any packet waiting to 
     * be received from the AC49x VoPP. This is an alternative method to check for a pending packet.
     * @n @b Note: It is not recommended to use this function, use ::Ac49xReceivePacket instead.
     * 
     * @param Device                [in] .
     * @param TransferMedium        [in] ::Tac49xTransferMedium.
     *    
     * @par Return values:
     * @li AC_RX_PACKET_NOT_READY.
     * @li AC_RX_PACKET_READY.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int  Ac49xCheckRxPacketReady(int Device, Tac49xTransferMedium TransferMedium);

    /** 
     * @param Device                [in] .
    * @} 
    */

void Ac49xResetDeviceBufferDescriptors(int Device);


    /**
     * @defgroup Ac49xResetBufferDescriptors Ac49xResetBufferDescriptors
     * @ingroup PACKET_HANDLING_Functions
     * @brief 
     * initializes the buffer descriptor variables.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xResetBufferDescriptors function initializes the buffer descriptor variables. It is called by
     * the ::Ac49xInitDriver function. It should be called every time after VoPP reset (transfer from 'run' to 'init') 
     * or program reload.
     * 
     * @param
     * This function has no parameters.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

void Ac49xResetBufferDescriptors(void);

    /** @} */

            /* Debugging Related Functions */

    /**
     * @defgroup Ac49xSetDebugMode Ac49xSetDebugMode
     * @ingroup PACKET_HANDLING_Functions
     * @brief 
     * Enables/disables the debug mode.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSetDebugMode function enables/disables the debug mode. It initializes the debug mode parameters.
     * @n This function should be called prior to the ::Ac49xSetupDevice function. 
     * It is called by the ::Ac49xInitDriver function.
     * @n In debug mode every packet contains an additional footer that includes checksum and sequence number.
     * If error is detected, then the ::Ac49xReceivePacket function returns one of the following:
     * @li RX_SEQUENCE_ERROR @li RX_CHECKSUM_ERROR
     * 
     * If the AC49x VoPP detects an error, then the following error packets are received:
     * @li SYSTEM_ERROR_EVENT_PACKET_INDEX__COMMAND_SEQUENCE @li SYSTEM_ERROR_EVENT_PACKET_INDEX__COMMAND_CHECKSUM
     * 
     * If debug mode is not used, it can be removed from the code by setting ALLOW_DEBUG_MODE to zero.
     *
     * @param DebugMode  [in] .
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

void Ac49xSetDebugMode(Tac49xControl DebugMode); 

    /** @} */

#ifndef NDOC
U32 Ac49xCheckInitStateConfigurationEcho(int Device, char *pConfigurationPacket, int PacketSize);
#endif /* NDOC */


            /*   I N I T   S T A T E   C O M M A N D S   */
#if (  (AC49X_DEVICE_TYPE == AC491_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC491L_DEVICE)\
	)
#	ifdef UTOPIA_ROUTING

    /**
     * @defgroup Ac49xUtopiaRoutingConfiguration Ac49xUtopiaRoutingConfiguration
     * @ingroup DEVICE_CONFIGURATION
     * @brief 
     * Used for debugging in order to perform a packet connection between two channels on the same AC49x.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUtopiaRoutingConfiguration function is used for debugging in order to perform a packet connection 
     * between two channels on the same AC49x VoPP when the UTOPIA master loops back all packets arriving from 
     * the device. This function is called by the ::Ac49xSetupDevice function.
     * @n If the function is not sent, then each UTOPIA packet header will contain the transmitted channel 
     * Core and channel ID. If the function is sent, then the UTOPIA packet header will contain the configured 
     * channel and Core ID. 
     * @n To disable this function set UTOPIA_ROUTING to zero.
     * 
     * @param Device  [in] .
     * @param pAttr   [in] ::Tac49xUtopiaRoutingConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 - Successful.
     * @li BAD_PACKET_ERROR = Fatal error.
     * @li RX_SEQUENCE_ERROR = Fatal error.
     * @li RX_CHECKSUM_ERROR = Fatal error.
     * @li RX_BD_INDEX_ERROR = Fatal error.
     * @li RX_PACKET_SIZE_ERROR = Fatal error.
     * @li RX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * @li DEVICE_WAIT_TIMEOUT_ERROR = Fatal error (echo wasn't received).
     * @li COMMAND_ECHO_ERROR_FLAG | COMMAND_ECHO_CHECKSUM_ERROR = Fatal error (Bad echo was received).
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

	U32	 Ac49xUtopiaRoutingConfiguration(int Device, Tac49xUtopiaRoutingConfigurationAttr *pAttr); /* The Core exits the init state and enters the run state after it completes executing the Open Device Command */

    /** @} */

#	endif   /* UTOPIA_ROUTING */
#endif      /* AC49X_DEVICE_TYPE */

    /**
     * @defgroup Ac49xCallProgressDeviceConfiguration Ac49xCallProgressDeviceConfiguration
     * @ingroup DEVICE_CONFIGURATION
     * @brief 
     * Enables Users to define up to 32 dual-frequency tones that introduce various signaling types for 
     * Call Progress indication.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xCallProgressDeviceConfiguration function enables Users to define up to 32 dual-frequency tones that 
     * introduce various signaling types for Call Progress indication. This function is called by the ::Ac49xSetupDevice function.
     * @n Each Call Progress multi-frequency signal must be configured for the following parameters: 
     * @li Up to two frequencies of 300 Hz to 1980 Hz (12 bits)
     * @li Twist
     * @li Cadence Pattern
     * @li High and Low Frequency output power levels
     * @li SNR
     * @li Frequency deviation
     * @li Signal type. (Continues, burst or Cadence)
     * @li Weather it is AM modulated
     * @li SIT (Special Information Tone)
     * @li Energy Detection Thresholds
     *
     * @n Refer to <A HREF="Call Progress Defaults.html">Call Progress Defaults Table</A> for the Call Progress default values.
     * 
     * @param Device  [in] .
     * @param pAttr   [in] ::Tac49xCallProgressDeviceConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 - Successful.
     * @li BAD_PACKET_ERROR = Fatal error.
     * @li RX_SEQUENCE_ERROR = Fatal error.
     * @li RX_CHECKSUM_ERROR = Fatal error.
     * @li RX_BD_INDEX_ERROR = Fatal error.
     * @li RX_PACKET_SIZE_ERROR = Fatal error.
     * @li RX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * @li DEVICE_WAIT_TIMEOUT_ERROR = Fatal error (echo wasn't received).
     * @li COMMAND_ECHO_ERROR_FLAG | COMMAND_ECHO_CHECKSUM_ERROR = Fatal error (Bad echo was received).
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */


U32 Ac49xCallProgressDeviceConfiguration(int Device, Tac49xCallProgressDeviceConfigurationAttr *pAttr);  

    /** @} */

    /**
     * @defgroup Ac49xUserDefinedTonesDeviceConfiguration Ac49xUserDefinedTonesDeviceConfiguration
     * @ingroup DEVICE_CONFIGURATION
     * @brief 
     * Enables the definition of up to 32 general-usage, continuous dual-frequency tones.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDefinedTonesDeviceConfiguration function enables the definition of up to 32 general-usage,
     * continuous dual-frequency tones. These signals can be generated and detected by any channel.
     * @n This function is called by the ::Ac49xSetupDevice function.
     *
     * @param Device  [in] .
     * @param pAttr   [in] ::Tac49xUserDefinedTonesDeviceConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 - Successful.
     * @li BAD_PACKET_ERROR = Fatal error.
     * @li RX_SEQUENCE_ERROR = Fatal error.
     * @li RX_CHECKSUM_ERROR = Fatal error.
     * @li RX_BD_INDEX_ERROR = Fatal error.
     * @li RX_PACKET_SIZE_ERROR = Fatal error.
     * @li RX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * @li DEVICE_WAIT_TIMEOUT_ERROR = Fatal error (echo wasn't received).
     * @li COMMAND_ECHO_ERROR_FLAG | COMMAND_ECHO_CHECKSUM_ERROR = Fatal error (Bad echo was received).
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

U32 Ac49xUserDefinedTonesDeviceConfiguration(int Device, Tac49xUserDefinedTonesDeviceConfigurationAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xAgcDeviceConfiguration Ac49xAgcDeviceConfiguration
     * @ingroup DEVICE_CONFIGURATION
     * @brief 
     * Configures the AGC module in the AC49x VoPP.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xAgcDeviceConfiguration function is used to configure the AGC module in the AC49x VoPP.
     * These parameters are common to all channels on the device.
     * @n This function is called by the ::Ac49xSetupDevice function.
     * 
     * @param Device  [in] .
     * @param pAttr   [in] ::Tac49xAgcDeviceConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 - Successful.
     * @li BAD_PACKET_ERROR = Fatal error.
     * @li RX_SEQUENCE_ERROR = Fatal error.
     * @li RX_CHECKSUM_ERROR = Fatal error.
     * @li RX_BD_INDEX_ERROR = Fatal error.
     * @li RX_PACKET_SIZE_ERROR = Fatal error.
     * @li RX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * @li DEVICE_WAIT_TIMEOUT_ERROR = Fatal error (echo wasn't received).
     * @li COMMAND_ECHO_ERROR_FLAG | COMMAND_ECHO_CHECKSUM_ERROR = Fatal error (Bad echo was received).
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

U32 Ac49xAgcDeviceConfiguration(int Device, Tac49xAgcDeviceConfigurationAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xExtendedDeviceConfiguration Ac49xExtendedDeviceConfiguration
     * @ingroup DEVICE_CONFIGURATION
     * @brief 
     * Configures parameters that are common to all channels.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xExtendedDeviceConfiguration function Configures parameters that are common to all channels.
     * @n This function is called by the ::Ac49xSetupDevice function.
     * 
     * @param Device  [in] .
     * @param pAttr   [in] Tac49xExtendedDeviceConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 - Successful.
     * @li BAD_PACKET_ERROR = Fatal error.
     * @li RX_SEQUENCE_ERROR = Fatal error.
     * @li RX_CHECKSUM_ERROR = Fatal error.
     * @li RX_BD_INDEX_ERROR = Fatal error.
     * @li RX_PACKET_SIZE_ERROR = Fatal error.
     * @li RX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * @li DEVICE_WAIT_TIMEOUT_ERROR = Fatal error (echo wasn't received).
     * @li COMMAND_ECHO_ERROR_FLAG | COMMAND_ECHO_CHECKSUM_ERROR = Fatal error (Bad echo was received).
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

U32 Ac49xExtendedDeviceConfiguration(int Device, Tac49xExtendedDeviceConfigurationAttr *pAttr);

    /** @} */


#ifndef NDOC
U32 Ac49xOpenDeviceConfiguration(int Device, Tac49xOpenDeviceConfigurationAttr *pAttr); /* The Core exits the init state and enters the run state after it completes executing the Open Device Command */
#endif /* NDOC */



 
    /**
     * @defgroup Ac49xOpenDeviceConfiguration_Format0 Ac49xOpenDeviceConfiguration_Format0
     * @ingroup DEVICE_CONFIGURATION
     * @brief 
     * Transfers the device's state from Init to Run.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xOpenDeviceConfiguration_Format0 function transfers the device's state from Init to Run. After executing this 
     * function, the TDM serial ports and the UTOPIA port should function. All channels are in Idle state and are 
     * transmitting configurable (user-defined) PCM and CAS Idle values to the serial TDM port's slots. 
     * This function configures parameters that are common to all channels on the device, for example: 
     * @li TDM serial ports' configuration
     * @li UTOPIA port's configuration
     * 
     * Out of all the Init state functions, this function must be called last because it changes the 
     * device state from Init to Run.
     * @n This function is called by the ::Ac49xSetupDevice function.
     * @n @b Note: After executing the Open Device command, the AC491 VoPP sends a dummy UTOPIA cell with 
     * 'Core ID' set to 0. The Host should ignore this cell.
	 * This function and the function ::Ac49xOpenDeviceConfiguration_Format1 perform the same operation. However, only one of 
	 * these functions can be executed. The function ::Ac49xOpenDeviceConfiguration_Format1 provides more flexibility with regards 
	 * to TDM timeslots configurations.  
     * 
     * @param Device  [in] .
     * @param pAttr   [in] Tac49xOpenDeviceConfiguration_Format0Attr.
     *    
     * @par Return values:
     * @li 0 - Successful.
     * @li BAD_PACKET_ERROR = Fatal error.
     * @li RX_SEQUENCE_ERROR = Fatal error.
     * @li RX_CHECKSUM_ERROR = Fatal error.
     * @li RX_BD_INDEX_ERROR = Fatal error.
     * @li RX_PACKET_SIZE_ERROR = Fatal error.
     * @li RX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * @li DEVICE_WAIT_TIMEOUT_ERROR = Fatal error (echo wasn't received).
     * @li COMMAND_ECHO_ERROR_FLAG | COMMAND_ECHO_CHECKSUM_ERROR = Fatal error (Bad echo was received).
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
U32 Ac49xOpenDeviceConfiguration_Format0(int Device, Tac49xOpenDeviceConfiguration_Format0Attr *pAttr);
    /** @} */



    /**
     * @defgroup Ac49xOpenDeviceConfiguration_Format1 Ac49xOpenDeviceConfiguration_Format1
     * @ingroup DEVICE_CONFIGURATION
     * @brief 
     * Transfers the device's state from Init to Run.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xOpenDeviceConfiguration_Format1 function transfers the device's state from Init to Run. After executing this 
     * function, the TDM serial ports and the UTOPIA port should function. All channels are in Idle state and are 
     * transmitting configurable (user-defined) PCM and CAS Idle values to the serial TDM port's slots. 
     * This function configures parameters that are common to all channels on the device, for example: 
     * @li TDM serial ports' configuration
     * @li UTOPIA port's configuration
     * 
     * Out of all the Init state functions, this function must be called last because it changes the 
     * device state from Init to Run.
     * @n This function is called by the ::Ac49xSetupDevice function.
     * @n @b Note: After executing the Open Device command, the AC491 VoPP sends a dummy UTOPIA cell with 
     * 'Core ID' set to 0. The Host should ignore this cell.
     * This function and the function ::Ac49xOpenDeviceConfiguration_Format0 perform the same operation. However, only one of 
	 * these functions can be executed. The function Ac49xOpenDeviceConfiguration_Format1 provides more flexibility with regards 
	 * to TDM timeslots configurations.  
     * @param Device  [in] .
     * @param pAttr   [in] Tac49xOpenDeviceConfiguration_Format1Attr.
     *    
     * @par Return values:
     * @li 0 - Successful.
     * @li BAD_PACKET_ERROR = Fatal error.
     * @li RX_SEQUENCE_ERROR = Fatal error.
     * @li RX_CHECKSUM_ERROR = Fatal error.
     * @li RX_BD_INDEX_ERROR = Fatal error.
     * @li RX_PACKET_SIZE_ERROR = Fatal error.
     * @li RX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * @li DEVICE_WAIT_TIMEOUT_ERROR = Fatal error (echo wasn't received).
     * @li COMMAND_ECHO_ERROR_FLAG | COMMAND_ECHO_CHECKSUM_ERROR = Fatal error (Bad echo was received).
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
U32 Ac49xOpenDeviceConfiguration_Format1(int Device, Tac49xOpenDeviceConfiguration_Format1Attr *pAttr);
    /** @} */


            /*  R U N   S T A T E   configurtion Functions  */

    /**
     * @defgroup Ac49xUpdateChannelConfiguration Ac49xUpdateChannelConfiguration
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Updates the channel parameters while the channel is open
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUpdateChannelConfiguration function updates the channel parameters while the channel is open.
     * This function has exactly the same input structure as the 
     * ::Ac49xOpenChannelConfiguration function. The function is sent only after the channel is open. 
     * Note that not all parameters in the Tac49xOpenOrUpdateChannelConfigurationAttr structure can be updated with 
     * this function. 
     * Note also that when a channel is in Active PCM State, more parameters can be updated than when 
     * the channel is in Active Network State. The VoPP channel ignores parameters that cannot be updated.
     * @n Refer to <A HREF="Tac49xOpenOrUpdateChannelConfigurationAttr - Compliance.html">pAttr compliance</A> to classify the updated parameters.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xOpenOrUpdateChannelConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xUpdateChannelConfiguration(int Device, int Channel, Tac49xOpenOrUpdateChannelConfigurationAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xAdvancedChannelConfiguration Ac49xAdvancedChannelConfiguration
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Configures the advanced channel parameters.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xAdvancedChannelConfiguration function configures the advanced channel parameters.
     * It can be sent only when the channel is closed (Idle state). 
     * When the device moves from Init state to Run state the advanced parameters are set to their default state.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xAdvancedChannelConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xAdvancedChannelConfiguration(int Device, int Channel, Tac49xAdvancedChannelConfigurationAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xCloseChannelConfiguration Ac49xCloseChannelConfiguration
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Closes the channel.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xCloseChannelConfiguration function closes the channel. It moves the channel from active state to
     * idle state.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xCloseChannelConfiguration(int Device, int Channel); /* The channel exits the active state upon receiving the Close Channel Command */

    /** @} */

    /**
     * @defgroup Ac49xActivateRegularRtpChannelConfiguration Ac49xActivateRegularRtpChannelConfiguration
     * @ingroup RTP_STANDARD_CONFIGURATION
     * @brief 
     * Enables the channel's network connection.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xActivateRegularRtpChannelConfiguration function enables the channel's network connection.
     * It activates the selected voice coder. After receiving the function, the channel immediately starts 
     * transmitting voice RTP packets and RTCP packets to the network port. Any RTP and RTCP packets received 
     * prior to this function are ignored. If AMR is active, the channel waits for the 
     * ::Ac49xRegularChannel3GppUserPlaneInitializationConfiguration function 
     * or ::Ac49xRfc3267AmrInitializationConfiguration function before it sends packets to the network.
     * @n If protection is enabled, the channel waits for the ::Ac49xRegularChannelPacketCableProtectionConfiguration function.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xActivateOrUpdateRtpChannelConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xActivateRegularRtpChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdateRtpChannelConfigurationAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xUpdateRegularRtpChannelConfiguration Ac49xUpdateRegularRtpChannelConfiguration
     * @ingroup RTP_STANDARD_CONFIGURATION
     * @brief 
     * Updates the channel's RTP parameters while the RTP channel is activated.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUpdateRegularRtpChannelConfiguration function updates the channel's RTP parameters while the 
     * RTP channel is activated. This function updates the relevant fields on the fly. 
     * For information on the relevant fields refer to <A HREF="Tac49xActivateOrUpdateRtpChannelConfigurationAttr Fields.html">pAttr</A>.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xActivateOrUpdateRtpChannelConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xUpdateRegularRtpChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdateRtpChannelConfigurationAttr *pAttr);   

    /** @} */

    /**
     * @defgroup Ac49xCloseRegularRtpChannelConfiguration Ac49xCloseRegularRtpChannelConfiguration
     * @ingroup RTP_STANDARD_CONFIGURATION
     * @brief 
     * Closes the RTP/RTCP channel.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xCloseRegularRtpChannelConfiguration function closes the RTP/RTCP channel. The channel stops transmitting
     * and receiving packets from the network. In response to this command, the channel sends an RTCP event status packet.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] Tac49xCloseRtpChannelConfigurationAttr.
     *
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

#ifndef AC49X_DRIVRES_VERSION
int Ac49xCloseRegularRtpChannelConfiguration(int Device, int Channel);
#else
int Ac49xCloseRegularRtpChannelConfiguration(int Device, int Channel, Tac49xCloseRtpChannelConfigurationAttr *pAttr);
#endif

    /** @} */

    /**
     * @defgroup Ac49xActivate3GppIubChannelConfiguration Ac49xActivate3GppIubChannelConfiguration
     * @ingroup CHANNEL_CONFIGURATION_IUB
     * @brief 
     * Activates the 3GPP Iub channel.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xActivate3GppIubChannelConfiguration function activates the 3GPP Iub channel. It enables the channel's network connection.
     * This function activates the selected voice coder. After receiving the function, the channel immediately starts 
     * transmitting voice 3GPP Iub packets to the network port. Any 3GPP Iub packets received 
     * prior to this function are ignored. 
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xActivateOrUpdate3GppIubChannelConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xActivate3GppIubChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdate3GppIubChannelConfigurationAttr *pAttr);
    /** @} */


    /**
     * @defgroup Ac49xUpdate3GppIubChannelConfiguration Ac49xUpdate3GppIubChannelConfiguration
     * @ingroup CHANNEL_CONFIGURATION_IUB
     * @brief 
     * Updates 3GPP Iub configuration parameters
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUpdate3GppIubChannelConfiguration function updates 3GPP Iub configuration parameters.
     * @n Only parameters ::Tac49xActivateOrUpdate3GppIubChannelConfigurationAttr.LocalRate and Tac49xActivateOrUpdate3GppIubChannelConfigurationAttr.SilenceCompression can be updated on-the-fly. 
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xActivateOrUpdate3GppIubChannelConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xUpdate3GppIubChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdate3GppIubChannelConfigurationAttr *pAttr);
	/** @} */


   /**
     * @defgroup Ac49xClose3GppIubChannel Ac49xClose3GppIubChannel
     * @ingroup CHANNEL_CONFIGURATION_IUB
     * @brief 
     * Closes the 3GPP Iub channel.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xClose3GppIubChannel function closes the 3GPP Iub channel.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xClose3GppIubChannel(int Device, int Channel);
	/** @} */


    /**
     * @defgroup Ac49xActivateRegularAtmChannelConfiguration Ac49xActivateRegularAtmChannelConfiguration
     * @ingroup CHANNEL_CONFIGURATION_ATM
     * @brief 
     * Activates the ATM.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xActivateRegularAtmChannelConfiguration function activates the ATM. It enables the channel's network connection.
     * This function activates the selected voice coder. After receiving the function, the channel immediately starts 
     * transmitting voice ATM AAL2 or AAL1 packets to the network port. Any ATM packets received 
     * prior to this function are ignored. If AMR is active, the channel waits for the 
     * ::Ac49xRegularChannel3GppUserPlaneInitializationConfiguration function before it sends packets to the network.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xActivateOrUpdateAtmChannelConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xActivateRegularAtmChannelConfiguration( int Device, int Channel, Tac49xActivateOrUpdateAtmChannelConfigurationAttr	*pAttr);

    /** @} */

    /**
     * @defgroup Ac49xUpdateRegularAtmChannelConfiguration Ac49xUpdateRegularAtmChannelConfiguration
     * @ingroup CHANNEL_CONFIGURATION_ATM
     * @brief 
     * Updates ATM configuration parameters
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUpdateRegularAtmChannelConfiguration function updates ATM configuration parameters.
     * @n This function updates the relevant fields on the fly. For information on the relevant fields refer 
     * to <A HREF="Tac49xActivateOrUpdateAtmChannelConfigurationAttr  Fields.html">pAttr</A>.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xActivateOrUpdateAtmChannelConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xUpdateRegularAtmChannelConfiguration(	 int Device, int Channel, Tac49xActivateOrUpdateAtmChannelConfigurationAttr	*pAttr);

    /** @} */

    /**
     * @defgroup Ac49xCloseRegularAtmChannelConfiguration Ac49xCloseRegularAtmChannelConfiguration
     * @ingroup CHANNEL_CONFIGURATION_ATM
     * @brief 
     * Closes the ATM channel.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xCloseRegularAtmChannelConfiguration function closes the ATM channel.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xCloseRegularAtmChannelConfiguration(	 int Device, int Channel);

    /** @} */

    /**
     * @defgroup Ac49xActivateMediatedAtmChannelConfiguration Ac49xActivateMediatedAtmChannelConfiguration
     * @ingroup CHANNEL_CONFIGURATION_ATM
     * @brief 
     * Activates the mediated ATM.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xActivateMediatedAtmChannelConfiguration function activates the mediated ATM. It enables the mediated channel's network connection.
     * This function activates the selected voice coder (only G.711 is supported). After receiving the function, the channel immediately starts 
     * transmitting voice ATM AAL2 or AAL1 packets to the network port. Any ATM packets received 
     * prior to this function are ignored. If AMR is active, the channel waits for the 
     * ::Ac49xMediatedChannel3GppUserPlaneInitializationConfiguration function before it sends packets to the network.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xActivateOrUpdateAtmChannelConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xActivateMediatedAtmChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdateAtmChannelConfigurationAttr	*pAttr);

    /** @} */

    /**
     * @defgroup Ac49xUpdateMediatedAtmChannelConfiguration Ac49xUpdateMediatedAtmChannelConfiguration
     * @ingroup CHANNEL_CONFIGURATION_ATM
     * @brief 
     * Updates the mediated ATM configuration parameters
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUpdateMediatedAtmChannelConfiguration function updates the mediated ATM configuration parameters.
     * This function updates the relevant fields on the fly. For information on the relevant fields refer to 
     * <A HREF="Tac49xActivateOrUpdateAtmChannelConfigurationAttr  Fields.html">pAttr</A>.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xActivateOrUpdateAtmChannelConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xUpdateMediatedAtmChannelConfiguration(	 int Device, int Channel, Tac49xActivateOrUpdateAtmChannelConfigurationAttr	*pAttr);

    /** @} */

    /**
     * @defgroup Ac49xCloseMediatedAtmChannelConfiguration Ac49xCloseMediatedAtmChannelConfiguration
     * @ingroup CHANNEL_CONFIGURATION_ATM
     * @brief 
     * Closes the mediated ATM channel.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xCloseMediatedAtmChannelConfiguration function closes the mediated ATM channel.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xCloseMediatedAtmChannelConfiguration(	 int Device, int Channel);

    /** @} */

    /**
     * @defgroup Ac49xMediatedChannelConfiguration Ac49xMediatedChannelConfiguration
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Configures the mediated channel parameters in packet to packet mode.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xMediatedChannelConfiguration function configures the mediated channel parameters in packet to packet
     * mode. It should be invoked after ::Ac49xOpenChannelConfiguration function
     * when the Tac49xOpenOrUpdateChannelConfigurationAttr::PcmSource is set to PCM_SOURCE__FROM_NETWORK.
     * Only the following parameters can be configured for the mediated channel:
     * @li DtmfTransferMode @li DynamicJitterBufferOptimizationFactor @li VoiceJitterBufferMinDelay @li VoiceJitterBufferMaxDelay
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xOpenOrUpdateChannelConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xMediatedChannelConfiguration(int Device, int Channel, Tac49xOpenOrUpdateChannelConfigurationAttr *pAttr); 

    /** @} */

    /**
     * @defgroup Ac49xT38ChannelConfiguration Ac49xT38ChannelConfiguration
     * @ingroup CHANNEL_CONFIGURATION_FAX_RELAY
     * @brief 
     * Configures the T.38 transfer.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xT38ChannelConfiguration function configures the T.38 transfer. It can be sent before the 
     * ::Ac49xOpenChannelConfiguration function is called. 
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xT38ConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned, the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xT38ChannelConfiguration(  int Device, int Channel, Tac49xT38ConfigurationAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xActivateFaxRelayCommand Ac49xActivateFaxRelayCommand
     * @ingroup CHANNEL_CONFIGURATION_FAX_RELAY
     * @brief 
     * Activates fax relay.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xActivateFaxRelayCommand function activates fax relay. It can be sent after the ::Ac49xOpenChannelConfiguration 
     * function, before or after the ::Ac49xActivateRegularRtpChannelConfiguration function.
     * The function enables transmit of fax packets (T.38) through the network port when fax is detected. 
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xActivateFaxRelayAttr.

     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned, the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xActivateFaxRelayCommand(  int Device, int Channel, Tac49xActivateFaxRelayAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xDeactivateFaxRelayCommand Ac49xDeactivateFaxRelayCommand
     * @ingroup CHANNEL_CONFIGURATION_FAX_RELAY
     * @brief 
     * Deactivates fax relay.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xDeactivateFaxRelayCommand function deactivates fax relay.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned, the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xDeactivateFaxRelayCommand(int Device, int Channel);

    /** @} */

    /**
     * @defgroup Ac49xActivateOrDeactivate3WayConferenceConfiguration Ac49xActivateOrDeactivate3WayConferenceConfiguration
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Activates or deactivates the 3-way conferencing feature.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xActivateOrDeactivate3WayConferenceConfiguration function activates or deactivates the 3-way 
     * conferencing feature. The AC49x supports various modes of three-way conference:
     * @li 3 IP nodes (requires 3 channels) @li 3 PSTN nodes (requires 3 channels) @li 2 IP and 1 PSTN nodes 
     * (requires 2 channels on the same device) @li 1 IP and 2 PSTN nodes (requires 2 channels on the same device)
     *
     * When 3 channels participate in a conference, an external connection between the channels is required
     * (PCM or packet connection). All conference channels must be opened before this function invoked.
     * If error is made in the configuration, the Tac49xSystemErrorEventPacketIndex::SYSTEM_ERROR_EVENT_PACKET_INDEX__3WAY_CONFERENCE_CONFIGURATION_ERROR
     * error is returned by the AC49x VoPP.
     *
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xActivateOrDeactivate3WayConferenceConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xActivateOrDeactivate3WayConferenceConfiguration(int Device, int Channel, Tac49xActivateOrDeactivate3WayConferenceConfigurationAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xOpenChannelConfiguration Ac49xOpenChannelConfiguration
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Sets the channel parameters and activates the channel.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xOpenChannelConfiguration function sets the channel parameters and activates the channel 
     * (the channel enters Active State when the function is called). When the channel opens, it doesn't send 
     * packets to the network and it ignores packets that arrive from the network until the 
     * ::Ac49xActivateRegularRtpChannelConfiguration function is called. When the channel opens, Events packets 
     * are sent regularly to the Host.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xOpenOrUpdateChannelConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xOpenChannelConfiguration(int Device, int Channel, Tac49xOpenOrUpdateChannelConfigurationAttr *pAttr);  /* The channel enters Active state upon receiving the Open Channel Command */

    /** @} */

    /**
     * @defgroup Ac49xActivateMediatedRtpChannelConfiguration Ac49xActivateMediatedRtpChannelConfiguration
     * @ingroup RTP_STANDARD_CONFIGURATION
     * @brief 
     * Activates the mediated G.711 RTP channel.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xActivateMediatedRtpChannelConfiguration function activates the mediated G.711 RTP channel.
     * In Packet-to-Packet mode, the channel performs mediation between G.711 RTP and any other LBR vocoders (ATM or RTP).
     * @n @b Note: Only the G.711 coder can be selected in the Decoder/Encoder fields.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xActivateOrUpdateRtpChannelConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xActivateMediatedRtpChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdateRtpChannelConfigurationAttr	*pAttr);

    /** @} */

    /**
     * @defgroup Ac49xUpdateMediatedRtpChannelConfiguration Ac49xUpdateMediatedRtpChannelConfiguration
     * @ingroup RTP_STANDARD_CONFIGURATION
     * @brief 
     * Updates the RTP parameters of the mediated channel.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUpdateMediatedRtpChannelConfiguration function updates the RTP parameters of the mediated channel.
     * It is called after the RTP mediated channel is activated.
     * This function updates the relevant fields on the fly. 
     * For information on the relevant fields refer to <A HREF="Tac49xActivateOrUpdateRtpChannelConfigurationAttr Fields.html">pAttr</A>.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xActivateOrUpdateRtpChannelConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xUpdateMediatedRtpChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdateRtpChannelConfigurationAttr	*pAttr);   

    /** @} */

    /**
     * @defgroup Ac49xCloseMediatedRtpChannelConfiguration Ac49xCloseMediatedRtpChannelConfiguration
     * @ingroup RTP_STANDARD_CONFIGURATION
     * @brief 
     * Closes the mediated RTP/RTCP channel
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xCloseMediatedRtpChannelConfiguration function closes the mediated RTP/RTCP channel.
     * In response to this command, the channel sends a mediated channel RTCP event status packet.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] Tac49xCloseRtpChannelConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

#ifndef AC49X_DRIVRES_VERSION
int Ac49xCloseMediatedRtpChannelConfiguration(int Device, int Channel);
#else
int Ac49xCloseMediatedRtpChannelConfiguration(int Device, int Channel, Tac49xCloseRtpChannelConfigurationAttr *pAttr);
#endif

    /** @} */

    /**
     * @defgroup Ac49xRegularChannel3GppUserPlaneInitializationConfiguration Ac49xRegularChannel3GppUserPlaneInitializationConfiguration
     * @ingroup CHANNEL_CONFIGURATION_3GPP
     * @brief 
     * Initializes the 3GPP User Plane Protocol.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xRegularChannel3GppUserPlaneInitializationConfiguration function initializes the 3GPP User Plane Protocol.
     * It should be sent after:
     * @li The ::Ac49xActivateRegularRtpChannelConfiguration function is invoked and the field 
     * Tac49xActivateOrUpdateRtpChannelConfigurationAttr::AmrActiveMode is set to AMR_ACTIVE_MODE__3GPP_UP.
     * @li The ::Ac49xActivateRegularAtmChannelConfiguration function is invoked and
     * Tac49xActivateOrUpdateAtmChannelConfigurationAttr::Protocol is set to ATM_PROTOCOL__3GPP_USER_PLANE.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49x3GppUserPlaneInitializationConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xRegularChannel3GppUserPlaneInitializationConfiguration( int Device, int Channel, Tac49x3GppUserPlaneInitializationConfigurationAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xMediatedChannel3GppUserPlaneInitializationConfiguration Ac49xMediatedChannel3GppUserPlaneInitializationConfiguration
     * @ingroup CHANNEL_CONFIGURATION_3GPP
     * @brief 
     * Initializes the 3GPP User Plane Protocol of the mediated channel.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xMediatedChannel3GppUserPlaneInitializationConfiguration function initializes the 3GPP User Plane Protocol.of the mediated channel.
     * It should be sent after the ::Ac49xActivateMediatedRtpChannelConfiguration or ::Ac49xActivateMediatedAtmChannelConfiguration 
     * functions. When the field Tac49xActivateOrUpdateRtpChannelConfigurationAttr::AmrActiveMode is set 
     * to AMR_ACTIVE_MODE__3GPP_UP.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49x3GppUserPlaneInitializationConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xMediatedChannel3GppUserPlaneInitializationConfiguration(int Device, int Channel, Tac49x3GppUserPlaneInitializationConfigurationAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xRfc3267AmrInitializationConfiguration Ac49xRfc3267AmrInitializationConfiguration
     * @ingroup RTP_RFC3267
     * @brief 
     * Initializes the AMR/AMR-WB RFC 2367 mode
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xRfc3267AmrInitializationConfiguration function initializes the AMR/AMR-WB RFC 2367 mode.
     * It should be sent after the ::Ac49xActivateRegularRtpChannelConfiguration function when the field 
     * Tac49xActivateOrUpdateRtpChannelConfigurationAttr::AmrActiveMode is set to AMR_ACTIVE_MODE__RFC_3267.
     * @n If ::Tac49xRfc3267AmrInitializationConfiguration is sent while AMR/AMR-WB RFC 3267 is running, the protocol is 
     * reinitialized.
     * @n CRC, MCN, MCP are currently not supported. Also, only one frame is allowed per RTP packet 
     * (i.e., the Voice NOB must be zero).
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xRfc3267AmrInitializationConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xRfc3267AmrInitializationConfiguration(int Device, int Channel, Tac49xRfc3267AmrInitializationConfigurationAttr	*pAttr);

    /** @} */

    /**
     * @defgroup Ac49xRegularChannelPacketCableProtectionConfiguration Ac49xRegularChannelPacketCableProtectionConfiguration
     * @ingroup CHANNEL_CONFIGURATION_MEDIA_PROTECTION
     * @brief 
     * Initializes the media protection.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xRegularChannelPacketCableProtectionConfiguration function initializes the media protection. This function should be
     * sent after ::Ac49xActivateRegularRtpChannelConfiguration function is invoked, when the 
     * Tac49xActivateOrUpdateRtpChannelConfigurationAttr::MediaProtectionMode field is set to 
     * MEDIA_PROTECTION_MODE__PACKET_CABLE. After this function is called, RTP packets are transmited encrypted and authenticated.
     * @n @b Note: This function must be invoked twice, once with Tac49xPacketCableProtectionConfigurationAttr::Direction = 
     * MEDIA_PROTECTION_DIRECTION__INBOUND_RX to configure the protection of the received packets, and a second time
     * with Tac49xPacketCableProtectionConfigurationAttr::Direction = MEDIA_PROTECTION_DIRECTION__OUTBOUND_TX to 
     * configure the protection of the transmitted packets.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] Tac49xPacketCableProtectionConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xRegularChannelPacketCableProtectionConfiguration(int Device, int Channel, Tac49xPacketCableProtectionConfigurationAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xMediatedChannelPacketCableProtectionConfiguration Ac49xMediatedChannelPacketCableProtectionConfiguration
     * @ingroup CHANNEL_CONFIGURATION_MEDIA_PROTECTION
     * @brief 
     * Initializes the media protection.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xMediatedChannelPacketCableProtectionConfiguration function initializes the media protection.
     * This function should be sent after ::Ac49xActivateMediatedRtpChannelConfiguration function is invoked, when the 
     * Tac49xActivateOrUpdateRtpChannelConfigurationAttr::MediaProtectionMode field is set to 
     * MEDIA_PROTECTION_MODE__PACKET_CABLE.
     * @n After this function is called, RTP packets are transmited encrypted and authenticated.
     * @n @b Note: This function must be invoked twice, once with Tac49xPacketCableProtectionConfigurationAttr::Direction = 
     * MEDIA_PROTECTION_DIRECTION__INBOUND_RX to configure the protection of the received packets, and a second time
     * with Tac49xPacketCableProtectionConfigurationAttr::Direction = MEDIA_PROTECTION_DIRECTION__OUTBOUND_TX to 
     * configure the protection of the transmitted packets.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] Tac49xPacketCableProtectionConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xMediatedChannelPacketCableProtectionConfiguration(int Device, int Channel, Tac49xPacketCableProtectionConfigurationAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xRegularChannelSrtpProtectionConfiguration Ac49xRegularChannelSrtpProtectionConfiguration
     * @ingroup CHANNEL_CONFIGURATION_MEDIA_PROTECTION
     * @brief 
     * Configures the SRTP (Secure Real-Time Transport Protocol) media protection.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xRegularChannelSrtpProtectionConfiguration function initializes the media protection.
     * This function should be sent after ::Ac49xActivateRegularRtpChannelConfiguration function is invoked,
     * when the ::Tac49xActivateOrUpdateRtpChannelConfigurationAttr.MediaProtectionMode field is set to 
     * MEDIA_PROTECTION_MODE__SRTP. After this function is called, RTP packets are transmited encrypted and authenticated.
     * @n <b> Note 1: </b> This function must be invoked twice, once with ::Tac49xSrtpProtectionConfigurationAttr.Direction = 
     * MEDIA_PROTECTION_DIRECTION__INBOUND_RX to configure the protection of the received packets, and a second time 
     * with Tac49xSrtpProtectionConfigurationAttr.Direction = MEDIA_PROTECTION_DIRECTION__OUTBOUND_TX to configure 
     * the protection of the transmitted packets.
     * @n <b> Note 2: </b> The Tac49xSrtpProtectionConfigurationAttr.RtpInitSequenceNumber parameter and the 
     * Tac49xActivateOrUpdateRtpChannelConfigurationAttr.TxSequenceNumber in the function ::Ac49xActivateRegularRtpChannelConfiguration 
     * must be set to less than 2^15 at the beginning of the session.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xSrtpProtectionConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xRegularChannelSrtpProtectionConfiguration(int Device, int Channel, Tac49xSrtpProtectionConfigurationAttr *pAttr);

    /** @} */

#ifndef NDOC
int Ac49xMediatedChannelSrtpProtectionConfiguration(int Device, int Channel, Tac49xSrtpProtectionConfigurationAttr *pAttr);
#endif /* NDOC */

    /**
     * @defgroup Ac49xCodecConfiguration Ac49xCodecConfiguration
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Configures the internal codec in the AC494 device.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xCodecConfiguration function configures the internal codec in the AC494 device.
     * 
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xCodecConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

#if (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
    )
int Ac49xCodecConfiguration(int Device, int Channel, Tac49xCodecConfigurationAttr *pAttr);
#endif
    /** @} */

    /**
     * @defgroup Ac49xAcousticEchoCancelerConfiguration Ac49xAcousticEchoCancelerConfiguration
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Configures the acoustic echo canceler in the AC494 device.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xAcousticEchoCancelerConfiguration function configures the acoustic echo canceler in the AC494 device.
     * This function is applicable only if the parameter Tac49xAdvancedChannelConfigurationAttr::AcousticEchoCancelerEnable
     * is set to CONTROL__ENABLE.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] Tac49xAcousticEchoCancelerConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

#if (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
    )
int Ac49xAcousticEchoCancelerConfiguration(int Device, int Channel, Tac49xAcousticEchoCancelerConfigurationAttr *pAttr);
#endif

    /** @} */

    /**
     * @defgroup Rfc3558InitializationConfiguration Rfc3558InitializationConfiguration
     * @ingroup RTP_RFC3558
     * @brief
     * Initializes the EVRC vocoder transfer protocol.
     * @{
     */

    /**
     * <BR>
     * The @b Rfc3558InitializationConfiguration function Initializes the EVRC vocoder transfer protocol.
     * @n This function must be sent after calling the function ::Ac49xActivateRegularRtpChannelConfiguration 
     * when both the encoder and decoder are set to EVRC. If the decoder or the encoder are not set to EVRC, 
     * this function is ignored.
     * If the function is called while EVRC is running, the protocol is reinitialized.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] Tac49xRfc3558InitializationConfigurationAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xRfc3558InitializationConfiguration(int Device, int Channel, Tac49xRfc3558InitializationConfigurationAttr *pAttr);

    /** @} */

/**
     * @defgroup Ac49xG729EvInitializationConfiguration Ac49xG729EvInitializationConfiguration
     * @ingroup RTP_G729EV
     * @brief
     * Initializes the G.729EV transfer protocol.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xG729EvInitializationConfiguration initializes the G.729EV transfer protocol. It must be called after the 
     * ::Ac49xActivateRegularRtpChannelConfiguration is called with the encoder or decoder input parameters set to G.729EV. 
     * If G.729EV isn't set in the function ::Ac49xActivateRegularRtpChannelConfiguration, this function is 
     * ignored. If this function is called while G.729EV is running, the protocol is reinitialized.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] Tac49xG729EvInitializationConfigurationAttr
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xG729EvInitializationConfiguration(int Device, int Channel, Tac49xG729EvInitializationConfigurationAttr *pAttr);
    /** @} */

/**
     * @defgroup Ac49xRegularChannelRtcpXrConfiguration Ac49xRegularChannelRtcpXrConfiguration
     * @ingroup RTP_RTCP_XR
     * @brief
     * Configures and enables the RTCP-XR Alerts.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xRegularChannelRtcpXrConfiguration configures and enables the RTCP-XR Alerts. This function can be invoked (a few times) 
     * only while the RTCP-XR is active; otherwise, it is ignored.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] Tac49xRtcpXrConfigurationAttr
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xRegularChannelRtcpXrConfiguration(int Device, int Channel, Tac49xRtcpXrConfigurationAttr *pAttr);
    /** @} */

/**
     * @defgroup Ac49xMediatedChannelRtcpXrConfiguration Ac49xMediatedChannelRtcpXrConfiguration
     * @ingroup RTP_RTCP_XR
     * @brief
     * Configures and enables the RTCP-XR Alerts.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xRegularChannelRtcpXrConfiguration configures and enables the RTCP-XR Alerts for the mediated channel. This function can be invoked (a few times) 
     * only while the RTCP-XR is active; otherwise, it is ignored.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] Tac49xRtcpXrConfigurationAttr
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xMediatedChannelRtcpXrConfiguration(int Device, int Channel, Tac49xRtcpXrConfigurationAttr *pAttr);
    /** @} */

/**
     * @defgroup Ac49xOpen4WayConferenceConfiguration Ac49xOpen4WayConferenceConfiguration
     * @ingroup CHANNEL_CONFIGURATION_4WAY_CONFERENCE
     * @brief
     * Activates the 4-Way Conference module.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xOpen4WayConferenceConfiguration configures and activates the 4-Way Conference module.
     *
     * @param Device   [in] .
     * @param pAttr    [in] Tac49xOpenOrUpdate4WayConferenceConfigurationAttr
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xOpen4WayConferenceConfiguration(int Device, Tac49xOpenOrUpdate4WayConferenceConfigurationAttr *pAttr);
    /** @} */


/**
     * @defgroup Ac49xUpdate4WayConferenceConfiguration Ac49xUpdate4WayConferenceConfiguration
     * @ingroup CHANNEL_CONFIGURATION_4WAY_CONFERENCE
     * @brief
     * Updates the Cross Gain Matrix of the 4-Way Conference module.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xUpdate4WayConferenceConfiguration updates the Cross Gain Matrix of the 4-Way Conference module.
     *
     * @param Device   [in] .
     * @param pAttr    [in] Tac49xOpenOrUpdate4WayConferenceConfigurationAttr
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xUpdate4WayConferenceConfiguration(int Device, Tac49xOpenOrUpdate4WayConferenceConfigurationAttr *pAttr);
    /** @} */

    /**
     * @defgroup Ac49xClose4WayConferenceConfiguration Ac49xClose4WayConferenceConfiguration
     * @ingroup CHANNEL_CONFIGURATION_4WAY_CONFERENCE
     * @brief
     * Closes the 4-Way Conference resource.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xClose4WayConferenceConfiguration closes the 4-Way Conference resource. All of the participants in the conference are automatically closed.
     *
     * @param Device   [in] .
     * @param pAttr    [in] Tac49xClose4WayConferenceConfigurationAttr
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xClose4WayConferenceConfiguration(int Device, Tac49xClose4WayConferenceConfigurationAttr *pAttr);
    /** @} */

    /**
     * @defgroup Ac49xAdd4WayConferenceLegConfiguration Ac49xAdd4WayConferenceLegConfiguration
     * @ingroup CHANNEL_CONFIGURATION_4WAY_CONFERENCE
     * @brief
     * Assigns an open channel to an active conference.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xAdd4WayConferenceLegConfiguration assigns an open channel to an active conference.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] Tac49xAdd4WayConferenceLegConfigurationAttr
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xAdd4WayConferenceLegConfiguration(int Device, int Channel, Tac49xAdd4WayConferenceLegConfigurationAttr *pAttr);
    /** @} */

    /**
     * @defgroup Ac49xRemove4WayConferenceLegConfiguration Ac49xRemove4WayConferenceLegConfiguration
     * @ingroup CHANNEL_CONFIGURATION_4WAY_CONFERENCE
     * @brief
     * Disconnects a participant from the conference.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xRemove4WayConferenceLegConfiguration disconnects a participant from the conference, without closing the channel.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] Tac49xRemove4WayConferenceLegConfigurationAttr
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xRemove4WayConferenceLegConfiguration(int Device, int Channel, Tac49xRemove4WayConferenceLegConfigurationAttr *pAttr);
    /** @} */


    
    /**
     * @defgroup Ac49xUemClipInitializationConfiguration Ac49xUemClipInitializationConfiguration
     * @ingroup RTP_UEMCLIP
     * @brief
     * Configures the UEMCLIP vocoder.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xUemClipInitializationConfiguration configures the UEMCLIP vocoder. It should be sent if the input parameters Encoder and Decoder are configured to UEMCLIP (79) for the function ::Ac49xActivateRegularRtpChannelConfiguration.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] Tac49xUemClipInitializationAttr
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xUemClipInitializationConfiguration(int Device, int Channel, Tac49xUemClipInitializationAttr *pAttr);
    /** @} */

    /**
     * @defgroup Ac49xG7111InitializationConfiguration Ac49xG7111InitializationConfiguration
     * @ingroup RTP_G711_1
     * @brief
     * Configures the G.711.1 vocoder.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xG7111InitializationConfiguration configures the G.711.1 vocoder. It should be sent if the input parameters  Encoder and Decoder are configured to G.711.1 (80 or 81) for the function ::Ac49xActivateRegularRtpChannelConfiguration.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] Tac49xG7111InitializationAttr
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xG7111InitializationConfiguration (int Device, int Channel,  Tac49xG7111InitializationAttr *pAttr);
    /** @} */
  
    /**
     * @defgroup Ac49xMicrosoftRtaInitializationConfiguration Ac49xMicrosoftRtaInitializationConfiguration
     * @ingroup RTP_MS_RTA
     * @brief
     * Configures the Microsoft RTA vocoder.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xMicrosoftRtaInitializationConfiguration configures the Microsoft RTA vocoder. It should be sent if the input parameters  Encoder and Decoder are configured to Microsoft RTA (82 or 83) for the function ::Ac49xActivateRegularRtpChannelConfiguration.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] Tac49xMicrosoftRtaInitializationAttr
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xMicrosoftRtaInitializationConfiguration(int Device, int Channel, Tac49xMicrosoftRtaInitializationAttr *pAttr);
    /** @} */

/********************************************************************************************/
/********************* B A C K W A R D    C O M P A T I B I L I T Y ***********************/
/********************************************************************************************/
/********************************************************************************************/
/* Due to renaming of this function															*/
/* 1. Ac49xActivateAtmChannelConfiguration	--> Ac49xActivateRegularAtmChannelConfiguration.*/
/* 2. Ac49xUpdateAtmChannelConfiguration	--> Ac49xUpdateRegularAtmChannelConfiguration.	*/
/* 3. Ac49xCloseAtmChannelConfiguration		--> Ac49xCloseRegularAtmChannelConfiguration.	*/
/* 4. Ac49xActivateT38Command				--> Ac49xActivateFaxRelayCommand.				*/
/* 5. Ac49xDeactivateT38Command				--> Ac49xDeactivateFaxRelayCommand.				*/
/* 6. Ac49x3GppUserPlaneInitializationConfiguration -->										*/
/*    Ac49xRegularChannel3GppUserPlaneInitializationConfiguration.							*/
/*																							*/
/* These functions are still supported by this drivers package for backwards competabilty	*/
/* NOTE: Please use this new functions for better performance.								*/ 
/********************************************************************************************/


#ifndef NDOC
int Ac49xActivateAtmChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdateAtmChannelConfigurationAttr	*pAttr);
int Ac49xUpdateAtmChannelConfiguration(int Device, int Channel, Tac49xActivateOrUpdateAtmChannelConfigurationAttr	*pAttr);
int Ac49xCloseAtmChannelConfiguration(int Device, int Channel);
int Ac49xActivateT38Command(int Device, int Channel, Tac49xActivateFaxRelayAttr *pAttr);
int Ac49xDeactivateT38Command(int Device, int Channel);
int Ac49x3GppUserPlaneInitializationConfiguration(int Device, int Channel, Tac49x3GppUserPlaneInitializationConfigurationAttr *pAttr);
int Ac49xActivateDataRelayConfiguration(int Device, int Channel, Tac49xActivateOrUpdateDataRelayConfigurationAttr	*pAttr);
int Ac49xUpdateDataRelayConfiguration(int Device, int Channel, Tac49xActivateOrUpdateDataRelayConfigurationAttr	*pAttr);
int Ac49xDeactivateDataRelayConfiguration(int Device, int Channel);
int Ac49xAdvancedDataRelayConfiguration(int Device, int Channel, Tac49xAdvancedDataRelayConfigurationAttr *pAttr);

#endif /* NDOC */


            /*   R U N   S T A T E   Read and Write Functions   */

    /**
     * @defgroup Ac49xGetRegularRtcpParameters Ac49xGetRegularRtcpParameters
     * @ingroup RTP_RTCP
     * @brief 
     * Requests the channel to send RTCP parameters.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xGetRegularRtcpParameters function requests the channel to send RTCP parameters. In response to 
     * it, the channel sends an RTCP Event packet (Tac49xRtcpReceiveStatus).
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xGetRegularRtcpParameters(int Device, int Channel);

    /** @} */

    /**
     * @defgroup Ac49xSetRegularRtcpParameters Ac49xSetRegularRtcpParameters
     * @ingroup RTP_RTCP
     * @brief 
     * Modifies a single channel's RTCP parameters.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSetRegularRtcpParameters function modifies a single channel's RTCP parameters.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xSetRtcpParametersAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xSetRegularRtcpParameters(int Device, int Channel, Tac49xSetRtcpParametersAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xGetMediatedRtcpParameters Ac49xGetMediatedRtcpParameters
     * @ingroup RTP_RTCP
     * @brief 
     * Requests the mediated channel to send RTCP parameters.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xGetMediatedRtcpParameters function requests the mediated channel to send RTCP parameters. 
     * In response to it, the mediated channel sends an RTCP event packet (Tac49xRtcpReceiveStatus).
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xGetMediatedRtcpParameters(int Device, int Channel);													

    /** @} */

    /**
     * @defgroup Ac49xSetMediatedRtcpParameters Ac49xSetMediatedRtcpParameters
     * @ingroup RTP_RTCP
     * @brief 
     * Modifies a single mediated channel's RTCP parameters.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSetMediatedRtcpParameters function modifies a single mediated channel's RTCP parameters. 
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xSetRtcpParametersAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xSetMediatedRtcpParameters(int Device, int Channel, Tac49xSetRtcpParametersAttr	*pAttr);

    /** @} */

    /**
     * @defgroup Ac49xSendIbsString Ac49xSendIbsString
     * @ingroup CHANNEL_CONFIGURATION_SIGNALING
     * @brief 
     * generates an IBS (DTMF, MF, User Defined Tones, Call Progress Tones) string.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSendIbsString function generates an IBS (DTMF, MF, User Defined Tones, Call Progress Tones) string.
     * The string generation starts imediately (replaces the previous IBS activity if existed). The DTMF/MF 
     * generation level is according to the Tac49xOpenOrUpdateChannelConfigurationAttr::IbsLevel_minus_dbm parameter
     * in the ::Ac49xOpenChannelConfiguration function. The Call Progress/User defined Tone generation level is
     * according to the ::Ac49xCallProgressDeviceConfiguration and ::Ac49xUserDefinedTonesDeviceConfiguration functions.
     * IBS string can be directed to one of the following:
     * @li PCM out @li Network (through voice encoder payload) @li Network (RFC 2833 relay)
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xIbsStringAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xSendIbsString(int Device, int Channel, Tac49xIbsStringAttr	*pAttr);  

    /** @} */

    /**
     * @defgroup Ac49xSendIbsStop Ac49xSendIbsStop
     * @ingroup CHANNEL_CONFIGURATION_SIGNALING
     * @brief 
     * Stops IBS relay or string generation.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSendIbsStop function stops IBS relay or string generation.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xSendIbsStop(int Device, int Channel);													

    /** @} */

#ifndef NDOC
int Ac49xSendExtendedIbsString(int Device, int Channel, Tac49xExtendedIbsStringAttr *pAttr);
#endif /* NDOC */

    /**
     * @defgroup Ac49xSendExtendedIbsString_Format0 Ac49xSendExtendedIbsString_Format0
     * @ingroup CHANNEL_CONFIGURATION_SIGNALING
     * @brief 
     * Generates an IBS string with more flexibility than with the regular IBS function ::Ac49xSendIbsString.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSendExtendedIbsString_Format0 function generates an IBS string (DTMF, User-Defined Tones, Call Progress Tone) 
     * with more flexibility than with the function ::Ac49xSendExtendedIbsString_Format1. The flexibility manifests 
     * in that the user can configure different levels and durations per digit (but only up to 6 digits). The function enables loops and 
	 * creating tones in sequence (without a silence gap), etc.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xExtendedIbsString_Format0_Attr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned, the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xSendExtendedIbsString_Format0(int Device, int Channel, Tac49xExtendedIbsString_Format0_Attr *pAttr);
    /** @} */

    /**
     * @defgroup Ac49xSendExtendedIbsString_Format1 Ac49xSendExtendedIbsString_Format1
     * @ingroup CHANNEL_CONFIGURATION_SIGNALING
     * @brief 
     * Generates an IBS string with more flexibility than with the regular IBS function ::Ac49xSendIbsString.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSendExtendedIbsString_Format1 function generates an IBS string (DTMF, User-Defined Tones, Call Progress Tone) 
     * with less flexibility than with the function ::Ac49xSendExtendedIbsString_Format0. With this function, all digits posses 
	 * the same level and duration (up to 40 digits). The function enables loops and 
	 * creating tones in sequence (without a silence gap), etc.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xExtendedIbsString_Format1_Attr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned, the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xSendExtendedIbsString_Format1(int Device, int Channel, Tac49xExtendedIbsString_Format1_Attr *pAttr);

    /** @} */

#ifndef NDOC
int Ac49xStartTrunkTestingMeasurement(int Device, int Channel, Tac49xTrunkTestingMeasurementAttr *pAttr);
#endif /* NDOC */

    /**
     * @defgroup Ac49xStartBerTest Ac49xStartBerTest
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Enables Bit Error Rate (BER) testing over digital links.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xStartBerTest function enables Bit Error Rate (BER) testing over digital links. 
     * It complies with the ITU O.150, O.152, and Telcordia GR.822 standards. There are two modes 
     * of operation: @li Towards the TDM @li Towards the Network.
     *
     * @n @b Note: If the test is performed on the network side, the host should configure parameter 
     * VoiceJitterBufferMinDelay (in the ::Ac49xOpenChannelConfiguration function) to the maximum 160 msec 
     * and parameter DynamicJitterBufferOptimizationFactor to 13 (Static Jitter Buffer).
     * @n @b Note: The test can be performed on the network side only if the decoder and encoder are Transparent.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xBerTestAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @note 
     * Refer to the following important points when configuring the BER:
     * @li All fields are ignored when Tac49xBerTestAttr::Action is set to a value other than START_TEST and 
     *     INSERT_ERROR_IN_CURRENT_TEST. 
     * @li After STOP_TEST and SHOW_REPORT commands, the channel responds with a BER report status packet.
     * @li If a START_TEST command is sent when the test is already performed, the command is ignored.
     * @li If the test is performed on the network side and there is a packet loss or Jitter Buffer fix, the test is halted until synchronization is reestablished.
     * @li The fields Tac49xBerTestAttr::ErrorBitsInsertionNumber, Tac49xBerTestAttr::ErrorBitsInsertionInterval, 
     *     Tac49xBerTestAttr::ErrorBitsInsertionTimeMode and Tac49xBerTestAttr::ErrorBitsInsertionPatternMode 
     *     are relevant only when the field Tac49xBerTestAttr::Action is set to INSERT_ERROR_IN_CURRENT_TEST.
     *
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xStartBerTest(int Device, int Channel, Tac49xBerTestAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xUtopiaStatusRequest Ac49xUtopiaStatusRequest
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Requests the present status (statistical data) of the UTOPIA interface.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUtopiaStatusRequest function requests the present status (statistical data) of the UTOPIA interface.
     * In response to this function, the device sends the Tac49xUtopiaStatusPayload packet
     *
     * @param Device  [in] .
     * @param pAttr   [in] :Tac49xUtopiaStatusRequestAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xUtopiaStatusRequest(int Device,Tac49xUtopiaStatusRequestAttr *pAttr);

    /** @} */

int Ac49xSs7Mtp1Command(int Device,	int Channel, Tac49xSs7Mtp1CommandAttr *pAttr, Tac49xPacket *pPacket, Tac49xTransferMedium TransferMedium);

    /**
     * @defgroup Ac49xSendCas Ac49xSendCas
     * @ingroup CHANNEL_CONFIGURATION_SIGNALING
     * @brief 
     * Generates ABCD CAS signal.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSendCas function generates ABCD CAS signal. There are two modes of CAS generation
     * @li Towards CAS PCM serial port. @li Towards the network (RFC 2833 relay)
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xCasAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xSendCas(int Device, int Channel, Tac49xCasAttr	*pAttr);  

    /** @} */

    /**
     * @defgroup Ac49xSendCasString Ac49xSendCasString
     * @ingroup CHANNEL_CONFIGURATION_SIGNALING
     * @brief 
     * Generates ABCD CAS string.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSendCasString function generates ABCD CAS string. There are two modes of CAS generation
     * @li Towards CAS PCM serial port. @li Towards the network (RFC 2833 relay)
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xCasStringCommandAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xSendCasString(int Device, int Channel, Tac49xCasStringCommandAttr *pAttr);

    /** @} */

#ifndef NDOC
int Ac49xTandemFreeOperationCommand(int Device, int Channel, Tac49xTandemFreeOperationAttr *pAttr);  
#endif /* NDOC */

    /**
     * @defgroup Ac49xSendHdlcFramerMessage Ac49xSendHdlcFramerMessage
     * @ingroup CHANNEL_CONFIGURATION_SIGNALING
     * @brief 
     * Commands the channel to send an HDLC framer message through the TDM port.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSendHdlcFramerMessage function commands the channel to send an HDLC framer message through the TDM port.
     * It can be used only if the channel is opened with field Tac49xOpenOrUpdateChannelConfigurationAttr::ChannelType 
     * set to CHANNEL_TYPE__HDLC.
     *
     * @param Device          [in] .
     * @param Channel         [in] .
     * @param pAttr           [in] ::Tac49xHdlcFramerMessageCommandAttr.
     * @param pPacket         [in] Reserved for internal use, must be set to NULL.
     * @param TransferMedium  [in] ::Tac49xTransferMedium. This parameter determines the port used to send the command (host port or network port). It must correspond to Tac49xAdvancedChannelConfigurationAttr::HdlcTransferMedium field.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xSendHdlcFramerMessage(int Device, int Channel, Tac49xHdlcFramerMessageCommandAttr *pAttr, Tac49xPacket * pPacket, Tac49xTransferMedium TransferMedium);

    /** @} */

    /**
     * @defgroup Ac49xSetRfc3267AmrRateControl Ac49xSetRfc3267AmrRateControl
     * @ingroup RTP_RFC3267
     * @brief 
     * Controls the rate of the local and remote points (AMR/AMR-WB RFC 3267 protocol).
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSetRfc3267AmrRateControl function is used in the AMR/AMR-WB RFC 3267 protocol to control the rate of 
     * the local and remote points. It is enabled only after the ::Ac49xRfc3267AmrInitializationConfiguration 
     * function is sent. The remote rate is sent to the other side every transmitted voice packet.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xSetRfc3267AmrRateControlAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xSetRfc3267AmrRateControl(							int Device, int Channel, Tac49xSetRfc3267AmrRateControlAttr					*pAttr);

    /** @} */

    /**
     * @defgroup Ac49xRegularChannelSet3GppUserPlaneRateControl Ac49xRegularChannelSet3GppUserPlaneRateControl
     * @ingroup CHANNEL_CONFIGURATION_3GPP
     * @brief 
     * Can change local and remote (the channel on the other side of the network) AMR rates, as defined 
     * in 3GPP UP protocol.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xRegularChannelSet3GppUserPlaneRateControl function can change local and remote (the channel on the other 
     * side of the network) AMR rates, as defined in 3GPP UP protocol. The function is enabled only after the 
     * ::Ac49xRegularChannel3GppUserPlaneInitializationConfiguration function is sent.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xSet3GppUserPlaneRateControlAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xRegularChannelSet3GppUserPlaneRateControl(			int Device, int Channel, Tac49xSet3GppUserPlaneRateControlAttr				*pAttr);

    /** @} */

    /**
     * @defgroup Ac49xRegularChannelGet3GppStatistics Ac49xRegularChannelGet3GppStatistics
     * @ingroup CHANNEL_CONFIGURATION_3GPP
     * @brief 
     * Causes a channel to send statistics on the quality of the network.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xRegularChannelGet3GppStatistics function causes a channel to send statistics 
     * (via Tac49x3GppUserPlaneStatisticStatus) on the quality of the network (packet loss, CRC errors, etc.). 
     * The function is relevant only for 3GPP UP Support Mode.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xRegularChannelGet3GppStatistics(					int Device, int Channel);

    /** @} */

    /**
     * @defgroup Ac49xMediatedChannelSet3GppUserPlaneRateControl Ac49xMediatedChannelSet3GppUserPlaneRateControl
     * @ingroup CHANNEL_CONFIGURATION_3GPP
     * @brief 
     * Can change local and remote (the channel on the other side of the network) AMR rates, as defined 
     * in 3GPP UP protocol.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xMediatedChannelSet3GppUserPlaneRateControl function can change local and remote (the channel on the other 
     * side of the network) AMR rates of the mediated channel, as defined in 3GPP UP protocol. The function is enabled only after the 
     * ::Ac49xMediatedChannel3GppUserPlaneInitializationConfiguration function is sent.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xSet3GppUserPlaneRateControlAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xMediatedChannelSet3GppUserPlaneRateControl(		int Device, int Channel, Tac49xSet3GppUserPlaneRateControlAttr				*pAttr);

    /** @} */

    /**
     * @defgroup Ac49xMediatedChannelGet3GppStatistics Ac49xMediatedChannelGet3GppStatistics
     * @ingroup CHANNEL_CONFIGURATION_3GPP
     * @brief 
     * Causes a mediated channel to send statistics on the quality of the network.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xMediatedChannelGet3GppStatistics function causes a mediated channel to send statistics 
     * (via Tac49x3GppUserPlaneStatisticStatus) on the quality of the 
     * network (packet loss, CRC errors, etc.). The function is relevant only for 3GPP UP Support Mode.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xMediatedChannelGet3GppStatistics(					int Device, int Channel);

    /** @} */

    /**
     * @defgroup Ac49xSendCallerIdMessage Ac49xSendCallerIdMessage
     * @ingroup CHANNEL_CONFIGURATION_CID
     * @brief 
     * Generates ETSI, Telcordia (Bellcore) and/or NTT Caller ID types.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSendCallerIdMessage function generates ETSI, Telcordia (Bellcore) and/or NTT Caller ID types. 
     * Note that the user is responsible for the signaling and CID message synchronization.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xCallerIdAttr.
     * @param pMessage [in] Pointer to the Caller ID message bytes (according to the standard).
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xSendCallerIdMessage(int Device, int Channel, Tac49xCallerIdAttr *pAttr, U8 *pMessage);

    /** @} */

    /**
     * @defgroup Ac49xSendCallerIdStop Ac49xSendCallerIdStop
     * @ingroup CHANNEL_CONFIGURATION_CID
     * @brief 
     * Stops Caller ID generation.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSendCallerIdStop function stops Caller ID generation.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xSendCallerIdStop(int Device, int Channel);

    /** @} */

    /**
     * @defgroup Ac49xPlaybackNetworkStart Ac49xPlaybackNetworkStart
     * @ingroup CHANNEL_CONFIGURATION_PLAYBACK_GROUP
     * @brief 
     * Playbacks Network packets towards the encoder and from there to the network.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xPlaybackNetworkStart function Playbacks Network packets (always G.711 packets) towards the 
     * encoder and from there to the network.
     @ @n @b Note: This function is not available on the AC491 device (on the AC491 device the playback is performed
     * through the UTOPIA port).
     *
     * @param Device     [in] .
     * @param Channel    [in] .
     * @param Coder      [in] ::Tac49xCoder. Can only be set to either CODER__G711ALAW or CODER__G711MULAW.
     * @param Endianness [in] ::Tac49xEndianness. Defines the payload format of ADPCM and linear 16-bit PCM.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xPlaybackNetworkStart(int Device, int Channel, Tac49xCoder Coder, Tac49xEndianness Endianness);

    /** @} */

    /**
     * @defgroup Ac49xPlaybackTdmStart Ac49xPlaybackTdmStart
     * @ingroup CHANNEL_CONFIGURATION_PLAYBACK_GROUP
     * @brief 
     * Playbacks TDM packets towards the decoder and from there to the TDM interface.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xPlaybackTdmStart function playbacks TDM packets towards the decoder and from there 
     * to the TDM interface.
     @ @n @b Note: This function is not available on the AC491 device (on the AC491 device the playback is performed
     * through the UTOPIA port).
     *
     * @param Device     [in] .
     * @param Channel    [in] .
     * @param Coder      [in] ::Tac49xCoder.
     * @param Endianness [in] ::Tac49xEndianness. Defines the payload format of ADPCM and linear 16-bit PCM.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xPlaybackTdmStart(int Device, int Channel, Tac49xCoder Coder, Tac49xEndianness Endianness);

    /** @} */

    /**
     * @defgroup Ac49xPlaybackPlaySilence Ac49xPlaybackPlaySilence
     * @ingroup CHANNEL_CONFIGURATION_PLAYBACK_GROUP
     * @brief 
     * Plays silence for a given duration.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xPlaybackPlaySilence function plays silence for a given duration.
     * It can be sent before or after Playback Voice packets.
     @ @n @b Note: This function is not available on the AC491 device (on the AC491 device the playback is performed
     * through the UTOPIA port).
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param Silence  [in] in msec resolution; the duration is rounded to the nearest multiple of the 
     *                      coder duration; the range is 0-16000 seconds.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xPlaybackPlaySilence(int Device, int Channel, int Silence);

    /** @} */

    /**
     * @defgroup Ac49xPlaybackPlayVoice Ac49xPlaybackPlayVoice
     * @ingroup CHANNEL_CONFIGURATION_PLAYBACK_GROUP
     * @brief 
     * Sends voice packet to the VoPP
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xPlaybackPlayVoice function sends voice packet to the VoPP.
     @ @n @b Note: This function is not available on the AC491 device (on the AC491 device the playback is performed
     * through the UTOPIA port).
     *
     * @param Device        [in] .
     * @param Channel       [in] .
     * @param pVoiceBuffer  [in] Pointer to the voice payload buffer that is played by the AC49x VoPP.
     * @param BufferSize    [in] Size of the voice payload. It must be smaller than the received AvailableBufferSpace_msec. Refer to Tac49xPlaybackDspCommand.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xPlaybackPlayVoice(int Device, int Channel, char *pVoiceBuffer, int BufferSize);

    /** @} */

    /**
     * @defgroup Ac49xPlaybackEnd Ac49xPlaybackEnd
     * @ingroup CHANNEL_CONFIGURATION_PLAYBACK_GROUP
     * @brief 
     * Stops the playback operation.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xPlaybackEnd function stops the playback operation.
     @ @n @b Note: This function is not available on the AC491 device (on the AC491 device the playback is performed
     * through the UTOPIA port).
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xPlaybackEnd(int Device, int Channel);

    /** @} */

    /**
     * @defgroup Ac49xResetSerialPort Ac49xResetSerialPort
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Commands the Vopp to reset the serial port.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xResetSerialPort commands the VoPP to reset the serial port. This function should be used after 
     * the serial port signals (framesync and clock) were distorted. It aproves the recovery time.
     *
     * @param FirstDevice  [in] Device. In AC491 this command must only be sent to core 0.
     *
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xResetSerialPort(int Device);

    /** @} */

    /**
     * @defgroup Ac49xRecordCommand Ac49xRecordCommand
     * @ingroup CHANNEL_CONFIGURATION_RECORD_GROUP
     * @brief 
     * Commands the AC49x VoPP to start or stop recording.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xRecordCommand function commands the AC49x VoPP to start or stop recording the TDM or decoder 
     * output data..
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xRecordCommandAttr.
     *
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xRecordCommand(int Device, int Channel, Tac49xRecordCommandAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xGetRegularChannelMediaProtectionInfo Ac49xGetRegularChannelMediaProtectionInfo
     * @ingroup CHANNEL_CONFIGURATION_MEDIA_PROTECTION
     * @brief 
     * Requests the AC49x VoPP to send an updated Media Protection Audit status.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xGetRegularChannelMediaProtectionInfo function requests the AC49x VoPP to send an updated 
     * Media Protection Audit status.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] Tac49xGetMediaProtectiontInfoAttr.
     *
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xGetRegularChannelMediaProtectionInfo(int Device, int Channel, Tac49xGetMediaProtectiontInfoAttr *pAttr);

    /** @} */

#ifndef NDOC
int Ac49xGetMediatedChannelMediaProtectionInfo(int Device, int Channel, Tac49xGetMediaProtectiontInfoAttr *pAttr);
#endif /* NDOC */

    /**
     * @defgroup Ac49xSetRfc3558RateMode Ac49xSetRfc3558RateMode
     * @ingroup RTP_RFC3558
     * @brief 
     * Sets the RFC 3558 rate mode.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSetRfc3558RateMode function is used in RFC 3558 protocol to control the rate mode of the local and 
     * remote points. It is enabled only after the RFC 3558 initialization function (::Rfc3558InitializationConfiguration) is sent.
     * The remote rate is sent to the other side every voice packet until this function is called again with a new remote rate mode.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] type of Tac49xSetRfc3558RateModeAttr.
     *
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xSetRfc3558RateMode(int Device, int Channel, Tac49xSetRfc3558RateModeAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xTimeSlotManipulationCommand Ac49xTimeSlotManipulationCommand
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Performs manipulations on the PCM time slots.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xTimeSlotManipulationCommand function performs PCM highway timeslot manipulation. Only the configured 
     * timeslot in the ::Ac49xOpenDeviceConfiguration function can be manipulated. 
     * @n This function can perform:
     * @li TDM side loop: Rx -> Tx between any 2 timeslots
     * @li Network side loop: Tx -> Rx between any 2 timeslots
     * @li Swap between two channels
     *
     * This function is available on the AC491 VoPP and the AC494 SoC.
     * 
     * @param Device   [in] .
     * @param pAttr    [in] ::Tac49xTimeSlotManipulationCommandAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xTimeSlotManipulationCommand(int Device, Tac49xTimeSlotManipulationCommandAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xModuleStatusRequest Ac49xModuleStatusRequest
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Requests the status of a specific module.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xModuleStatusRequest function requests the status of a specific module.
     * In response to this function, the channel sends a module status packet. The module status packet includes 
     * configuration and status of a specific module. 
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xModuleStatusRequestAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xModuleStatusRequest(int Device, int Channel, Tac49xModuleStatusRequestAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xSendRegularRtcpAppCommand Ac49xSendRegularRtcpAppCommand
     * @ingroup RTP_RTCP
     * @brief 
     * Commands the AC49x VoPP to send an RTCP APP packet to the remote side 
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSendRegularRtcpAppCommand function commands the AC49x VoPP to send an RTCP APP packet to the remote 
     * side (refer to RFC 3550 in URL http://www.ietf.org/rfc/rfc3550.txt for detailed information).
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xRtcpAppCommandAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xSendRegularRtcpAppCommand(int Device, int Channel, Tac49xRtcpAppCommandAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xSendMediatedRtcpAppCommand Ac49xSendMediatedRtcpAppCommand
     * @ingroup RTP_RTCP
     * @brief 
     * Commands the mediated channel of the AC49x VoPP to send an RTCP APP packet to the remote side 
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSendMediatedRtcpAppCommand function commands the mediated channel of the AC49x VoPP to send an RTCP 
     * APP packet to the remote side (refer to RFC 3550 in URL http://www.ietf.org/rfc/rfc3550.txt for detailed 
     * information).
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xRtcpAppCommandAttr.
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */


int Ac49xSendMediatedRtcpAppCommand(int Device, int Channel, Tac49xRtcpAppCommandAttr *pAttr);

    /** @} */

    /**
     * @defgroup Ac49xNtpTimeStampCommand Ac49xNtpTimeStampCommand
     * @ingroup RTP_STANDARD_CONFIGURATION
     * @brief 
     * Sets or updates the local NTP timestamp.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xNtpTimeStampCommand sets or updates the local NTP timestamp. The NTP time appears in the RTCP packet transmitted to the network.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xNtpTimeStampCommandAttr
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xNtpTimeStampCommand(int Device, int Channel, Tac49xNtpTimeStampCommandAttr *pAttr);
    /** @} */


    /**
     * @defgroup Ac49xSetG729EvRateControl Ac49xSetG729EvRateControl
     * @ingroup RTP_G729EV
     * @brief 
     * Used to control the bit rate of the local and remote G.729EV points. 
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xSetG729EvRateControl is used to control the bit rate of the local and remote G.729EV points. 
     * It is enabled only after the function ::Ac49xG729EvInitializationConfiguration is called. The remote bit rate 
     * is sent to other side every voice packet until the function Ac49xSetG729EvRateControl is called again.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     * @param pAttr    [in] ::Tac49xG729EvRateControlCommandAttr
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xSetG729EvRateControl(int Device, int Channel, Tac49xG729EvRateControlCommandAttr *pAttr);
    /** @} */


   /**
     * @defgroup Ac49xGet3GppIubStatus Ac49xGet3GppIubStatus
     * @ingroup CHANNEL_CONFIGURATION_IUB
     * @brief 
     * Requests a 3GPP Iub status packet from the channel. 
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xGet3GppIubStatus is used to requests a 3GPP Iub status packet from the channel. 
     * In response to this function, the channel sends the packet ::Tac49x3GppIubStatus.
     * 
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it again when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xGet3GppIubStatus(int Device, int Channel);
    /** @} */


/**
     * @defgroup Ac49xGetRegularChannelRtcpXrMarkovModelStatisticsCommand Ac49xGetRegularChannelRtcpXrMarkovModelStatisticsCommand
     * @ingroup RTP_RTCP_XR
     * @brief
     * Commands the AC49x channel to send the ::Tac49xRtcpXrMarkovModelStatisticsEvent event.
     * @{
     */

    /**
     * <BR>
     * When the function @b Ac49xGetRegularChannelRtcpXrMarkovModelStatisticsCommand is invoked, the AC49x channel sends the ::Tac49xRtcpXrMarkovModelStatisticsEvent event.
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xGetRegularChannelRtcpXrMarkovModelStatisticsCommand(int Device, int Channel);
    /** @} */

/**
     * @defgroup Ac49xGetMediatedChannelRtcpXrMarkovModelStatisticsCommand Ac49xGetMediatedChannelRtcpXrMarkovModelStatisticsCommand
     * @ingroup RTP_RTCP_XR
     * @brief
     * Commands the AC49x mediated channel to send the ::Tac49xRtcpXrMarkovModelStatisticsEvent event.
     * @{
     */

    /**
     * <BR>
     * When the function @b Ac49xGetMediatedChannelRtcpXrMarkovModelStatisticsCommand is invoked, the AC49x mediated channel sends the ::Tac49xRtcpXrMarkovModelStatisticsEvent event.
     * 
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xGetMediatedChannelRtcpXrMarkovModelStatisticsCommand(int Device, int Channel);
    /** @} */

/**
     * @defgroup Ac49xGetRegularChannelRtcpXrVoiceQualityStatisticsCommand Ac49xGetRegularChannelRtcpXrVoiceQualityStatisticsCommand
     * @ingroup RTP_RTCP_XR
     * @brief
     * Commands the AC49x channel to send the ::Tac49xRtcpXrVoiceQualityStatisticsEvent event.
     * @{
     */

    /**
     * <BR>
     * When the function @b Ac49xGetRegularChannelRtcpXrVoiceQualityStatisticsCommand is invoked, the AC49x channel sends the ::Tac49xRtcpXrVoiceQualityStatisticsEvent event.
     * 
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xGetRegularChannelRtcpXrVoiceQualityStatisticsCommand(int Device, int Channel);
    /** @} */

/**
     * @defgroup Ac49xGetMediatedChannelRtcpXrVoiceQualityStatisticsCommand Ac49xGetMediatedChannelRtcpXrVoiceQualityStatisticsCommand
     * @ingroup RTP_RTCP_XR
     * @brief
     * Commands the AC49x mediated channel to send the ::Tac49xRtcpXrVoiceQualityStatisticsEvent event.
     * @{
     */

    /**
     * <BR>
     *  When the function @b Ac49xGetMediatedChannelRtcpXrVoiceQualityStatisticsCommand is invoked, the AC49x mediated channel sends the ::Tac49xRtcpXrVoiceQualityStatisticsEvent event.
     *  
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xGetMediatedChannelRtcpXrVoiceQualityStatisticsCommand(int Device, int Channel);
    /** @} */

/**
     * @defgroup Ac49xGetLastCasDetectionCommand Ac49xGetLastCasDetectionCommand
     * @ingroup CHANNEL_CONFIGURATION_SIGNALING
     * @brief
     * Commands the AC49x channel to send the CAS Detection event.
     * @{
     */

    /**
     * <BR>
     *  When the function @b Ac49xGetLastCasDetectionCommand is invoked, the AC49x channel sends the ::Tac49xCasEventPayload event.
     *  
     *
     * @param Device   [in] .
     * @param Channel  [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int Ac49xGetLastCasDetectionCommand(int Device, int Channel);
    /** @} */


#ifndef NDOC

int Ac49xDuaFramerCommand(    int Device, int Channel, Tac49xDuaFramerCommandAttr *pAttr, Tac49xPacket * pPacket, Tac49xTransferMedium TransferMedium);
int Ac49xEventsControlCommand(int Device, Tac49xEventsControlCommandAttr *pAttr);
int Ac49xVideoAudioSyncCommand(int Device, int Channel, Tac49xPacket *pPacket);
int Ac49xAmdCommand(int Device, int Channel, Tac49xAmdCommandAttr *pAttr);
int Ac49xProtectionTestCommand(int Device, int Channel, Tac49xProtectionTestCommandAttr *pAttr);
int Ac49xGetRandomNumberCommand(int Device);
int Ac49xSwitchToModemCommand(int Device, int Channel, Tac49xSwitchToModemCommandAttr *pAttr);
int Ac49xModemRelayEndedCommand(int Device, int Channel, Tac49xModemRelayEndedCommandAttr *pAttr);

#endif /* NDOC */

/**
     * @defgroup Ac49xGet4WayConferenceStatusCommand Ac49xGet4WayConferenceStatusCommand
     * @ingroup CHANNEL_CONFIGURATION_4WAY_CONFERENCE
     * @brief
     * Requests the packet '4-Way Conference Status'.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xGet4WayConferenceStatusCommand requests the packet '4-Way Conference Status'. In response to this command, the AC49x VoPP sends the  
     * status packet ::Tac49x4WayConferenceStatusPayload, which displays the status of all conferences and their sessions on the device.
     *
     * @param Device   [in] .
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xGet4WayConferenceStatusCommand(int Device);
    /** @} */

/**
     * @defgroup Ac49xGet4WayConferenceDetailsCommand Ac49xGet4WayConferenceDetailsCommand
     * @ingroup CHANNEL_CONFIGURATION_4WAY_CONFERENCE
     * @brief
     * Requests the packet '4-Way Conference Details Event'.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xGet4WayConferenceDetailsCommand requests the packet '4-Way Conference Details Event'. 
     * In response to this command, the device sends the packet ::Tac49x4WayConferenceDetailsEventPayload.
     *
     * @param Device   [in] .
     * @param pAttr    [in] Tac49xGet4WayConferenceDetailsCommandAttr
     *    
     * @par Return values:
     * @li 0 = Successful.
     * @li BUFFER_DESCRIPTOR_FULL_ERROR = When this value is returned the user should hold the packet and retransmit it when the buffer is free.
     * @li TX_BD_INDEX_ERROR = Fatal error.
     * @li TX_BAD_PACKER_ADDRESS_ERROR = Fatal error.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
int Ac49xGet4WayConferenceDetailsCommand(int Device, Tac49xGet4WayConferenceDetailsCommandAttr *pAttr);
    /** @} */

/********************************************************************************************/
/********************* B A C K W A R D    C O M P A T A B I L I T Y ***********************/
/********************************************************************************************/
/********************************************************************************************/
/* Due to renaming of this function															*/
/* 1. Ac49xSet3GppUserPlaneRateControl -->													*/
/*    Ac49xRegularChannelSet3GppUserPlaneRateControl.										*/
/* 2. Ac49xGet3GppStatistics -->															*/
/*    Ac49xRegularChannelGet3GppStatistics.													*/
/*																							*/
/* These functions are still supported by this drivers package for backwards competabilty	*/
/* NOTE: Please use this new functions for better performance.								*/ 
/********************************************************************************************/



#ifndef NDOC
int Ac49xSet3GppUserPlaneRateControl(int Device, int Channel, Tac49xSet3GppUserPlaneRateControlAttr *pAttr);
int Ac49xGet3GppStatistics(int Device, int Channel);
#endif /* NDOC */


            /* Device Reset and Download Routines */

#ifndef NDOC
Tac49xProgramDownloadStatus Ac49xResetAndRelease(int Device);
#endif /* NDOC */

    /**
     * @defgroup Ac49xProgramDownLoad Ac49xProgramDownLoad
     * @ingroup DEVICE_RESET_AND_DOWNLOAD
     * @brief 
     * Downloads the code block(s) into the device and run it.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xProgramDownLoad function downloads the code block(s) into the device and run it.
     * It is invoked by the ::Ac49xBoot function.
     *
     * @param Device            [in] .
     * @param pProgram          [in] Pointer to the program memory block.
     * @param ApplicationParam  [in] Used for multi-application systems, default value may be zero.
     *    
     * @par Return values:
     * @e ::Tac49xProgramDownloadStatus.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

Tac49xProgramDownloadStatus Ac49xProgramDownLoad(int Device, char *pProgram, int ApplicationParam);

    /** @} */

    /**
     * @defgroup Ac49xKernelDownLoad Ac49xKernelDownLoad
     * @ingroup DEVICE_RESET_AND_DOWNLOAD
     * @brief 
     * Downloads the kernel.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xKernelDownLoad function downloads the kernel. It is available only in the Ac490 device.
     * Invoke prior to ::Ac49xProgramDownLoad. It is invoked by the ::Ac49xBoot function.
     *
     * @param Device   [in] .
     * @param pKernel  [in] Pointer to the kernel memory block.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

void Ac49xKernelDownLoad(int Device, char *pKernel);

    /** @} */

    /**
     * @defgroup Ac49xEmifTest Ac49xEmifTest
     * @ingroup DEVICE_MEMORY_ACCESS
     * @brief 
     * Performs external memory tests.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xEmifTest function performs external memory tests. This function downloads a special firmware 
     * that tests the external memory interface. This function returns the test diagnostics in the 
     * Tac49xEmifTestErrorInformationAttr structure. It includes 10 error substructures.
     * The test can track up to 10 incorrect memory locations. Each time the memory test program detects a memory 
     * error, it fills the error substructures. The unused substructures are zeroed.
     *
     * @param Device    [in] .
     * @param pProgram  [in] Pointer the memory test block.
     * @param pKernel   [in] Pointer to the kernel memory block.
     * @param pAttr     [in] Tac49xEmifTestErrorInformationAttr. Pointer to the diagnostics report structure.
     *    
     * @par Return values:
     * @e ::Tac49xEmifTestStatus.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

#if (AC49X_DEVICE_TYPE == AC490_DEVICE)
Tac49xEmifTestStatus Ac49xEmifTest(int Device, char *pProgram, char *pKernel, Tac49xEmifTestErrorInformationAttr *pAttr);
#endif /*(AC49X_DEVICE_TYPE == AC490_DEVICE)*/

    /** @} */


/***********************************************/
/***********************************************/
/**  T H E   C O N V E N I E N T   G R O U P  **/
/**  High-Level Initialization and Setup      **/
/***********************************************/
/***********************************************/

    /**
     * @defgroup Ac49xInitDriver Ac49xInitDriver
     * @ingroup CONVENIENT_GROUP
     * @brief 
     * Used for high-level initialization and setup.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xInitDriver function is used for high-level initialization and setup. It should be invoked
     * prior to the the first host access to the AC49x.
     * Following are the functions called by Ac49xInitDriver to be sent from the VoPP to the Host:
     * @li Send ::Ac49xUserDef_InitUserDefSection.
     * @li Send ::Ac49xResetBufferDescriptors.
     * @li Send ::Ac49xSetDebugMode.
     * @li Send ::Ac49xHcrfSetHostControlRegisterAddresses.
     * @li Send ::Ac49xInitFifo.
     * 
     * @param DebugMode  [in] .
     *    
     * @par Return values:
     * @e 0.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

int  Ac49xInitDriver(Tac49xControl DebugMode); 

    /** @} */

#ifndef NDOC
//void  Ac49xResetDriver(void); 
#endif /* NDOC */


    /**
     * @defgroup Ac49xBoot Ac49xBoot
     * @ingroup CONVENIENT_GROUP
     * @brief 
     * Resets and downloads the AC49x VoPP
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xBoot function resets and downloads the AC49x VoPP. The device enters 'Init' State immediately after 
     * completion of the download process. When using Ac491 or Ac494 pKernel must be initialized to NULL.
     * In AC491 VoPP the Ac49xBoot function @b must be invoked in sequential order for all devices on the multi-processor.
     * 
     * @param Device            [in] 
     * @param pProgram          [in] Pointer to the program memory block.
     * @param ApplicationParam  [in] Used for multi-application systems, default value may be zero.
     * @param pKernel           [in] Pointer to the kernel memory block.
     *    
     * @par Return values:
     * @e Tac49xProgramDownloadStatus.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

Tac49xProgramDownloadStatus  Ac49xBoot(int Device, char *pProgram, int ApplicationParam, char *pKernel);

    /** @} */

    /**
     * @defgroup Ac49xSetupDevice Ac49xSetupDevice
     * @ingroup CONVENIENT_GROUP
     * @brief 
     * Configures all parameters common to all channels in the device and puts the device in Run State.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSetupDevice function configures all parameters common to all channels in the device and 
     * puts the device in Run State. 
     * @n The following configuration functions are called by Ac49xSetupDevice (sent by the Host to the AC49x VoPP).
     * @li Send ::Ac49xDeviceSetControlRegister
     * @li Send ::Ac49xUtopiaRoutingConfiguration if AC491 and UTOPIA_ROUTING.
     * @li Send ::Ac49xCallProgressDeviceConfiguration.
     * @li Send ::Ac49xUserDefinedTonesDeviceConfiguration.
     * @li Send ::Ac49xAgcDeviceConfiguration.
     * @li Send ::Ac49xOpenDeviceConfiguration.
     *
     * @b Note: Use this function at startup, after program download, before configuring the channels.
     * 
     * @param Device            [in] 
     * @param pSetupDeviceAttr  [in] ::Tac49xSetupDeviceAttr.
     *    
     * @par Return values:
     * ::Tac49xSetupDeviceStatus
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

Tac49xSetupDeviceStatus  Ac49xSetupDevice(int Device, Tac49xSetupDeviceAttr *pSetupDeviceAttr);

    /** @} */

    /**
     * @defgroup Ac49xSetDefaultSetupDeviceAttr Ac49xSetDefaultSetupDeviceAttr
     * @ingroup CONVENIENT_GROUP
     * @brief 
     * Initializes all parameters common to all channels to their default values
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSetDefaultSetupDeviceAttr function initializes all parameters common 
     * to all channels to their default values.
     * 
     * @param pSetupDeviceAttr  [in] ::Tac49xSetupDeviceAttr.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

void Ac49xSetDefaultSetupDeviceAttr(Tac49xSetupDeviceAttr *pSetupDeviceAttr);

    /** @} */

void Ac49xSetDefaultDeviceControlRegisterAttr				(Tac49xDeviceControlRegisterAttr				*pAttr);
void Ac49xSetDefaultOpenDeviceConfigurationAttr				(Tac49xOpenDeviceConfigurationAttr				*pAttr);
void Ac49xSetDefaultAgcDeviceConfigurationAttr				(Tac49xAgcDeviceConfigurationAttr				*pAttr);
void Ac49xSetDefaultExtendedDeviceConfigurationAttr			(Tac49xExtendedDeviceConfigurationAttr			*pAttr);
void Ac49xSetDefaultUserDefinedTonesDeviceConfigurationAttr	(Tac49xUserDefinedTonesDeviceConfigurationAttr	*pAttr);
void Ac49xSetDefaultCallProgressDeviceConfigurationAttr		(Tac49xCallProgressDeviceConfigurationAttr		*pAttr);
void Ac49xSetDefaultOpenDeviceConfiguration_Format0Attr		(Tac49xOpenDeviceConfiguration_Format0Attr *pAttr);
void Ac49xSetDefaultOpenDeviceConfiguration_Format1Attr		(Tac49xOpenDeviceConfiguration_Format1Attr *pAttr);


    /**
     * @defgroup Ac49xSetDefaultSetupChannelAttr Ac49xSetDefaultSetupChannelAttr
     * @ingroup CONVENIENT_GROUP
     * @brief 
     * Initializes all the channel parameters to their default values.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xSetDefaultSetupChannelAttr function initializes all the channel parameters to their default values.
     * 
     * @param pSetupChannelAttr  [in] ::Tac49xSetupChannelAttr.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
void Ac49xSetDefaultSetupChannelAttr(Tac49xSetupChannelAttr *pSetupChannelAttr);

	 /** @} */
	
    /**
     * @defgroup Ac49xSetDefaultChannelConfigurationAttr Ac49xSetDefaultChannelConfigurationAttr
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Initializes the structure ::Tac49xOpenOrUpdateChannelConfigurationAttr to its default values.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xSetDefaultChannelConfigurationAttr initializes the structure ::Tac49xOpenOrUpdateChannelConfigurationAttr 
     * to its default values. This structure is used as input to the functions ::Ac49xUpdateChannelConfiguration, 
     * ::Ac49xMediatedChannelConfiguration and ::Ac49xOpenChannelConfiguration.
     * 
     * @param *pAttr  [in] ::Tac49xOpenOrUpdateChannelConfigurationAttr.
     * @param ChannelType  [in] ::Tac49xMediaChannelType.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
void Ac49xSetDefaultChannelConfigurationAttr(Tac49xOpenOrUpdateChannelConfigurationAttr *pAttr, Tac49xMediaChannelType ChannelType);
	 /** @} */

    /**
     * @defgroup Ac49xSetDefaultAdvancedChannelConfigurationAttr Ac49xSetDefaultAdvancedChannelConfigurationAttr
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Initializes the structure ::Tac49xAdvancedChannelConfigurationAttr to its default values.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xSetDefaultAdvancedChannelConfigurationAttr initializes the structure 
     * ::Tac49xAdvancedChannelConfigurationAttr to its default values. This structure is used as input to the
     * function ::Ac49xAdvancedChannelConfiguration.
     * 
     * @param *pAttr  [in] ::Tac49xAdvancedChannelConfigurationAttr.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
void Ac49xSetDefaultAdvancedChannelConfigurationAttr(Tac49xAdvancedChannelConfigurationAttr *pAttr);
	 /** @} */

    /**
     * @defgroup Ac49xSetDefaultAtmChannelConfigurationAttr Ac49xSetDefaultAtmChannelConfigurationAttr
     * @ingroup CHANNEL_CONFIGURATION_ATM
     * @brief 
     * Initializes the structure ::Tac49xActivateOrUpdateAtmChannelConfigurationAttr to its default values.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xSetDefaultAtmChannelConfigurationAttr initializes the structure 
     * ::Tac49xActivateOrUpdateAtmChannelConfigurationAttr to its default values. This structure is used as input to the
     * function ::Ac49xActivateRegularAtmChannelConfiguration.
     * 
     * @param *pAttr  [in] ::Tac49xActivateOrUpdateAtmChannelConfigurationAttr.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
void Ac49xSetDefaultAtmChannelConfigurationAttr(Tac49xActivateOrUpdateAtmChannelConfigurationAttr *pAttr);
	 /** @} */

     /**
     * @defgroup Ac49xSetDefaultRtpChannelConfigurationAttr Ac49xSetDefaultRtpChannelConfigurationAttr
     * @ingroup RTP_STANDARD_CONFIGURATION
     * @brief 
     * Initializes the structure ::Tac49xActivateOrUpdateRtpChannelConfigurationAttr to its default values.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xSetDefaultRtpChannelConfigurationAttr initializes the structure 
     * ::Tac49xActivateOrUpdateRtpChannelConfigurationAttr to its default values. This structure is used as input to the
     * function ::Ac49xActivateRegularRtpChannelConfiguration.
     * 
     * @param *pAttr  [in] ::Tac49xActivateOrUpdateRtpChannelConfigurationAttr.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
void Ac49xSetDefaultRtpChannelConfigurationAttr(Tac49xActivateOrUpdateRtpChannelConfigurationAttr *pAttr);
	 /** @} */

    /**
     * @defgroup Ac49xSetDefaultCloseRtpChannelConfigurationAttr Ac49xSetDefaultCloseRtpChannelConfigurationAttr
     * @ingroup RTP_STANDARD_CONFIGURATION
     * @brief 
     * Initializes the structure ::Tac49xCloseRtpChannelConfigurationAttr to its default values.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xSetDefaultCloseRtpChannelConfigurationAttr initializes the structure 
     * ::Tac49xCloseRtpChannelConfigurationAttr to its default values. This structure is used as input to the
     * function ::Ac49xCloseRegularRtpChannelConfiguration.
     * 
     * @param *pAttr  [in] ::Tac49xCloseRtpChannelConfigurationAttr.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
void Ac49xSetDefaultCloseRtpChannelConfigurationAttr(Tac49xCloseRtpChannelConfigurationAttr *pAttr);
	 /** @} */

    /**
     * @defgroup Ac49xSetDefaultT38ConfigurationAttr Ac49xSetDefaultT38ConfigurationAttr
     * @ingroup CHANNEL_CONFIGURATION_FAX_RELAY
     * @brief 
     * Initializes the structure ::Tac49xT38ConfigurationAttr to its default values.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xSetDefaultT38ConfigurationAttr initializes the structure ::Tac49xT38ConfigurationAttr to its default values.
     * This structure is used as input to the function ::Ac49xT38ChannelConfiguration.
     * 
     * @param *pAttr  [in] ::Tac49xT38ConfigurationAttr.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
void Ac49xSetDefaultT38ConfigurationAttr(Tac49xT38ConfigurationAttr *pAttr);
	 /** @} */

   /**
     * @defgroup Ac49xSetDefault3GppIubChannelConfigurationAttr Ac49xSetDefault3GppIubChannelConfigurationAttr
     * @ingroup CHANNEL_CONFIGURATION_IUB
     * @brief 
     * Initializes the structure ::Tac49xActivateOrUpdate3GppIubChannelConfigurationAttr to its default values.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xSetDefault3GppIubChannelConfigurationAttr initializes the structure 
     * ::Tac49xActivateOrUpdate3GppIubChannelConfigurationAttr to its default values. This structure is used as input to the
     * function ::Ac49xActivate3GppIubChannelConfiguration.
     * 
     * @param *pAttr  [in] ::Tac49xActivateOrUpdate3GppIubChannelConfigurationAttr.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
void Ac49xSetDefault3GppIubChannelConfigurationAttr(Tac49xActivateOrUpdate3GppIubChannelConfigurationAttr *pAttr);
	 /** @} */

    /**
     * @defgroup Ac49xSetDefault3WayConferenceConfigurationAttr Ac49xSetDefault3WayConferenceConfigurationAttr
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Initializes the structure ::Tac49xActivateOrDeactivate3WayConferenceConfigurationAttr to its default values.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xSetDefault3WayConferenceConfigurationAttr initializes the structure 
     * ::Tac49xActivateOrDeactivate3WayConferenceConfigurationAttr to its default values. 
     * This structure is used as input to the function ::Ac49xActivateOrDeactivate3WayConferenceConfiguration.
     * 
     * @param *pAttr  [in] ::Tac49xActivateOrDeactivate3WayConferenceConfigurationAttr.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
void Ac49xSetDefault3WayConferenceConfigurationAttr				(Tac49xActivateOrDeactivate3WayConferenceConfigurationAttr *pAttr);
	 /** @} */

    /**
     * @defgroup Ac49xSetDefault3GppUserPlaneInitializationConfigurationAttr Ac49xSetDefault3GppUserPlaneInitializationConfigurationAttr
     * @ingroup CHANNEL_CONFIGURATION_3GPP
     * @brief 
     * Initializes the structure ::Tac49x3GppUserPlaneInitializationConfigurationAttr to its default values.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xSetDefault3GppUserPlaneInitializationConfigurationAttr initializes the structure ::Tac49x3GppUserPlaneInitializationConfigurationAttr 
     * to its default values. This structure is used as input to the functions ::Ac49xRegularChannel3GppUserPlaneInitializationConfiguration
     * and ::Ac49xMediatedChannel3GppUserPlaneInitializationConfiguration.
     * 
     * @param *pAttr  [in] ::Tac49x3GppUserPlaneInitializationConfigurationAttr.
     * @param ChannelType  [in] ::Tac49xMediaChannelType.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
void Ac49xSetDefault3GppUserPlaneInitializationConfigurationAttr(Tac49x3GppUserPlaneInitializationConfigurationAttr *pAttr, Tac49xMediaChannelType ChannelType);
	 /** @} */

    /**
     * @defgroup Ac49xSetDefaultRfc3267AmrInitializationConfigurationAttr Ac49xSetDefaultRfc3267AmrInitializationConfigurationAttr
     * @ingroup RTP_RFC3267
     * @brief 
     * Initializes the structure ::Tac49xRfc3267AmrInitializationConfigurationAttr to its default values.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xSetDefaultRfc3267AmrInitializationConfigurationAttr initializes the structure ::Tac49xRfc3267AmrInitializationConfigurationAttr to its default values. 
     * This structure is used as input to the function ::Ac49xRfc3267AmrInitializationConfiguration.
     * 
     * @param *pAttr  [in] ::Tac49xRfc3267AmrInitializationConfigurationAttr.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
void Ac49xSetDefaultRfc3267AmrInitializationConfigurationAttr	(Tac49xRfc3267AmrInitializationConfigurationAttr *pAttr);
	 /** @} */

    /**
     * @defgroup Ac49xSetDefaultPacketCableProtectionConfigurationAttr Ac49xSetDefaultPacketCableProtectionConfigurationAttr
     * @ingroup CHANNEL_CONFIGURATION_MEDIA_PROTECTION
     * @brief 
     * Initializes the structure ::Tac49xPacketCableProtectionConfigurationAttr to its default values.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xSetDefaultPacketCableProtectionConfigurationAttr initializes the structure ::Tac49xPacketCableProtectionConfigurationAttr to its default values. 
     * This structure is used as input to the function ::Ac49xRfc3267AmrInitializationConfiguration.
     * 
     * @param *pAttr  [in] ::Ac49xRegularChannelPacketCableProtectionConfiguration.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
void Ac49xSetDefaultPacketCableProtectionConfigurationAttr (Tac49xPacketCableProtectionConfigurationAttr *pAttr);
	 /** @} */

     /**
     * @defgroup Ac49xSetDefaultSrtpProtectionConfigurationAttr Ac49xSetDefaultSrtpProtectionConfigurationAttr
     * @ingroup CHANNEL_CONFIGURATION_MEDIA_PROTECTION
     * @brief 
     * Initializes the structure ::Tac49xSrtpProtectionConfigurationAttr to its default values.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xSetDefaultSrtpProtectionConfigurationAttr initializes the structure ::Tac49xSrtpProtectionConfigurationAttr to its default values. 
     * This structure is used as input to the function ::Ac49xRegularChannelSrtpProtectionConfiguration.
     * 
     * @param *pAttr  [in] ::Tac49xSrtpProtectionConfigurationAttr.
     * @param Direction  [in] ::Tac49xMediaProtectionDirection.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
void Ac49xSetDefaultSrtpProtectionConfigurationAttr	(Tac49xSrtpProtectionConfigurationAttr *pAttr, Tac49xMediaProtectionDirection Direction);
	 /** @} */

#if (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
	|| (AC49X_DEVICE_TYPE == AC496E_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
    )

    /**
     * @defgroup Ac49xSetDefaultCodecConfigurationAttr Ac49xSetDefaultCodecConfigurationAttr
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Initializes the structure ::Tac49xCodecConfigurationAttr to its default values.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xSetDefaultCodecConfigurationAttr initializes the structure ::Tac49xCodecConfigurationAttr to its default values. 
     * This structure is used as input to the function ::Ac49xCodecConfiguration.
     * 
     * @param *pAttr  [in] ::Tac49xCodecConfigurationAttr.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

void Ac49xSetDefaultCodecConfigurationAttr(Tac49xCodecConfigurationAttr *pAttr);
	 /** @} */

    /**
     * @defgroup Ac49xSetDefaultAcousticEchoCancelerConfigurationAttr Ac49xSetDefaultAcousticEchoCancelerConfigurationAttr
     * @ingroup CHANNEL_CONFIGURATION_GENERAL
     * @brief 
     * Initializes the structure ::Tac49xAcousticEchoCancelerConfigurationAttr to its default values.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xSetDefaultAcousticEchoCancelerConfigurationAttr initializes the structure ::Tac49xAcousticEchoCancelerConfigurationAttr to its default values. 
     * This structure is used as input to the function ::Ac49xAcousticEchoCancelerConfiguration.
     * 
     * @param *pAttr  [in] ::Tac49xAcousticEchoCancelerConfigurationAttr.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
void Ac49xSetDefaultAcousticEchoCancelerConfigurationAttr(Tac49xAcousticEchoCancelerConfigurationAttr *pAttr);
	 /** @} */
#endif

    /**
     * @defgroup Ac49xSetDefaultRfc3558InitializationConfigurationAttr Ac49xSetDefaultRfc3558InitializationConfigurationAttr
     * @ingroup RTP_RFC3558
     * @brief 
     * Initializes the structure ::Tac49xRfc3558InitializationConfigurationAttr to its default values.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xSetDefaultRfc3558InitializationConfigurationAttr initializes the structure ::Tac49xRfc3558InitializationConfigurationAttr to its default values. 
     * This structure is used as input to the function ::Ac49xRfc3558InitializationConfiguration.
     * 
     * @param *pAttr  [in] ::Tac49xRfc3558InitializationConfigurationAttr.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
void Ac49xSetDefaultRfc3558InitializationConfigurationAttr(Tac49xRfc3558InitializationConfigurationAttr *pAttr);
	 /** @} */

    /**
     * @defgroup Ac49xSetDefaultG729EvInitializationConfigurationAttr Ac49xSetDefaultG729EvInitializationConfigurationAttr
     * @ingroup RTP_G729EV
     * @brief 
     * Initializes the structure ::Tac49xG729EvInitializationConfigurationAttr to its default values.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xSetDefaultG729EvInitializationConfigurationAttr initializes the structure ::Tac49xG729EvInitializationConfigurationAttr to its default values. 
     * This structure is used as input to the function ::Ac49xG729EvInitializationConfiguration.
     * 
     * @param *pAttr  [in] ::Tac49xG729EvInitializationConfigurationAttr.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */
void Ac49xSetDefaultG729EvInitializationConfigurationAttr (Tac49xG729EvInitializationConfigurationAttr *pAttr);
	 /** @} */


void Ac49xSetDefault4WayConferenceConfigurationAttr(Tac49xOpenOrUpdate4WayConferenceConfigurationAttr *pAttr);
void Ac49xSetDefault4WayConferenceLegConfigurationAttr(Tac49xAdd4WayConferenceLegConfigurationAttr *pAttr);
void Ac49xSetDefaultSetup4WayConferenceAttr(Tac49xSetup4WayConferenceAttr *pSetup4WayConferenceAttr);
void Ac49xSetDefaultSetup4WayConferenceLegAttr(Tac49xSetup4WayConferenceLegAttr *pSetup4WayConferenceLegAttr);

void Ac49xSetDefaultDataRelayConfigurationAttr(Tac49xActivateOrUpdateDataRelayConfigurationAttr *pAttr);
void Ac49xSetDefaultAdvancedDataRelayConfigurationAttr(Tac49xAdvancedDataRelayConfigurationAttr *pAttr);

void Ac49xSetDefaultUemClipInitializationConfigurationAttr(Tac49xUemClipInitializationAttr *pAttr);
void Ac49xSetDefaultG7111InitializationConfigurationAttr(Tac49xG7111InitializationAttr *pAttr);
void Ac49xSetDefaultMicrosoftRtaInitializationConfigurationAttr(Tac49xMicrosoftRtaInitializationAttr *pAttr);

    /**
     * @defgroup Ac49xGetDspDownloadInfo Ac49xGetDspDownloadInfo
     * @ingroup CONVENIENT_GROUP
     * @brief 
     * Reads the header of the program/kernel download blocks.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xGetDspDownloadInfo function reads the header of the program/kernel download blocks.
     * The header contains information such as name, date, version, etc.
     * 
     * @param pDspDownloadInfo  [out] ::Tac49xDspDownloadInfo.
     * @param DownloadBlock     [in] Pointer to the kernel or program block.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

void Ac49xGetDspDownloadInfo(Tac49xDspDownloadInfo *pDspDownloadInfo, const char *DownloadBlock);

    /** @} */

    /**
     * @defgroup Ac49xGetDriverVersionInfo Ac49xGetDriverVersionInfo
     * @ingroup CONVENIENT_GROUP
     * @brief 
     * Returns the driver version.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xGetDriverVersionInfo function returns the driver version.
     * 
     * @param pDriverVersionInfo  [out] ::Tac49xDriverVersionInfo.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

void Ac49xGetDriverVersionInfo(Tac49xDriverVersionInfo *pDriverVersionInfo);

    /** @} */

#if (FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__HOST)


    /**
     * @defgroup Ac49xInitFifo Ac49xInitFifo
     * @ingroup FIFO_USAGE
     * @brief 
     * Initializes the FIFO mechanism.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xInitFifo function initializes the FIFO mechanism of all devices on the board. It should be called in the initialization phase.
     * It is called by the ::Ac49xInitDriver function.
     * 
     * @param
     * This function has no parameters.
     *    
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */


void Ac49xInitFifo(void);

    /** @} */

    /**
     * @defgroup Ac49xDeviceInitFifo Ac49xDeviceInitFifo
     * @ingroup FIFO_USAGE
     * @brief
     * Initializes the FIFO mechanism.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xDeviceInitFifo initializes the FIFO mechanism of a specific device in contrast to the function ::Ac49xInitFifo, which 
     * initializes the FIFO mechanism of all the devices on the board. It should be invoked after a specific device has been reloaded. 
     *
     * @param Device          [in] .
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

void Ac49xDeviceInitFifo(int Device);

    /** @} */



    /**
     * @defgroup Ac49xDequeueAndTransmitCell Ac49xDequeueAndTransmitCell
     * @ingroup FIFO_USAGE
     * @brief 
     * Dequeues a cell from FIFO and transmits it to the VoPP.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xDequeueAndTransmitCell function dequeues a cell from FIFO and transmits it to the VoPP.
     * 
     * @param Device          [in] .
     * @param TransferMedium  [in] ::Tac49xTransferMedium.
     *    
     * @par Return values:
     * @li FIFO_ACTION_RESULT__FIFO_IS_EMPTY_ERROR = The FIFO buffer was empty.
     * @li FIFO_ACTION_RESULT__BUFFER_DESCRIPTOR_FULL_ERROR = The AC49x buffer descriptors is full and the cell couldn't be transmitted.
     * @li FIFO_ACTION_RESULT__SUCCEEDED = A cell was successufly transmitted to the AC49x.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

Tac49xFifoActionResult Ac49xDequeueAndTransmitCell(int Device, Tac49xTransferMedium TransferMedium);

    /** @} */

    /**
     * @defgroup Ac49xGetResidentCellNumber Ac49xGetResidentCellNumber
     * @ingroup FIFO_USAGE
     * @brief 
     * Returns the current number of cells in the FIFO buffer
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xGetResidentCellNumber function returns the current number of cells in the FIFO buffer
     * (note that one packet is composed of one or several cells).
     * 
     * @param Device          [in] .
     * @param TransferMedium  [in] ::Tac49xTransferMedium.
     *    
     * @par Return values:
     * @e U16 - number of cells resident in the FIFO.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

U16 Ac49xGetResidentCellNumber(int Device, Tac49xTransferMedium TransferMedium);

    /** @} */

#ifndef NDOC
float Ac49xGetMaxUsage(int Device, Tac49xTransferMedium TransferMedium);
#endif /* NDOC */

    /**
     * @defgroup Ac49xFifoFlush Ac49xFifoFlush
     * @ingroup FIFO_USAGE
     * @brief 
     * Empties the FIFO buffer.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xFifoFlush function empties the FIFO buffer. It transmitts @b all cells in the FIFO buffer to
     * the AC49x VoPP. Note that the ::Ac49xDequeueAndTransmitCell function dequeues a single cell.
     * 
     * @param Device          [in] .
     * @param TransferMedium  [in] Tac49xTransferMedium.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_Api.h.
     */

void Ac49xFifoFlush(int Device, Tac49xTransferMedium TransferMedium); 

    /** @} */

#ifndef NDOC
void Ac49xResetMaxUsage(void);
#endif /* NDOC */

#endif /* (FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__HOST) */

    /**
     * @defgroup Ac49xInitializePacketRecording Ac49xInitializePacketRecording
     * @ingroup PACKET_RECORDING
     * @brief 
     * Registers the ::TFlushPacketFunction callback function to the packet recording service in the drivers.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xInitializePacketRecording function registers the ::TFlushPacketFunction callback function to the 
     * packet recording service in the drivers. This function assigns a user-defined function that records all 
     * of the received and transmitted packets.
     *
     * @param FlushPacket  [in] Pointer to the ::TFlushPacketFunction callback function.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

void Ac49xInitializePacketRecording(TFlushPacketFunction FlushPacket);

    /** @} */

int Ac491DrvInit(void);

unsigned int hpiRead(unsigned int addr, unsigned int *data, int fd);
unsigned int hpiWrite(unsigned int addr, unsigned int data, int fd);
void ac491ActiveRegularRtpChannelConfig(void);
void displayDeviceState(void);
void clearAc491Statistics( void );
void Thread_RTP_READ(void);
void Thread_RTP_READ1(void);
void getAc491NetWorkPacket( void );
void getAc491HostPacket( void );
void startGetAc491Packet(void);
void getAc491HpiPacket(int device, Tac49xTransferMedium tranMod);
int HcrfWaitForResponse(int Device, Tac49xHcrfStatusRegister *pStatusRegister);
void Ac49xHcrfSetCommandRegister(int Device, Tac49xHcrfCommandRegisterIndex  CommandIndex, int ReferenceCoreId);
int HcrfDeviceHalt(int Device, Tac49xHcrfStatusRegisterIndex *pResponseValue);
int HcrfDeviceResume(int Device, Tac49xHcrfStatusRegisterIndex *pResponseValue);
int HcrfGetCpuRegisters(int Device, char *pInfo, Tac49xHcrfStatusRegisterIndex *pResponseValue);
int HcrfGetDspTraceBuffer(int Device, char *pInfo, Tac49xHcrfStatusRegisterIndex *pResponseValue);
void HcrfDeviceCrash(int Device);
int ActivateOrUpdate3GppIubChannelConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xActivateOrUpdate3GppIubChannelConfigurationAttr *pAttr);
int HcrfCompareLocalMemory(int Device, int ReferenceDevice, U32 *pAddress, Tac49xHcrfStatusRegisterIndex *pResponseValue);
Tac49xFifoActionResult Ac49xEnqueueCell(int Device, Tac49xTransferMedium TransferMedium, char *pCell, int Size, int Partial);
int Ac49xTransmitCell(int Device, Tac49xTransferMedium TransferMedium);
int OpenOrUpdateChannelConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xOpenOrUpdateChannelConfigurationAttr *pAttr);
int ActivateOrUpdateRtpChannelConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xActivateOrUpdateRtpChannelConfigurationAttr *pAttr);
int Channel3GppUserPlaneInitializationConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49x3GppUserPlaneInitializationConfigurationAttr *pAttr);
int PacketCableProtectionConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xPacketCableProtectionConfigurationAttr *pAttr);
int SrtpProtectionConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xSrtpProtectionConfigurationAttr *pAttr);
int RtcpXrConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xRtcpXrConfigurationAttr *pAttr);
int OpenOrUpdate4WayConferenceConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, Tac49xOpenOrUpdate4WayConferenceConfigurationAttr *pAttr);
int ActivateOrUpdateDataRelayConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xActivateOrUpdateDataRelayConfigurationAttr *pAttr);
int SetRtcpParameters(Tac49xCommandPacketOpCode OpCode, int Device, int Channel, Tac49xSetRtcpParametersAttr *pAttr);
int GetRtcpParameters(Tac49xCommandPacketOpCode OpCode, int Device, int Channel);
int Set3GppUserPlaneRateControl(Tac49xCommandPacketOpCode Opcode, int Device, int Channel, Tac49xSet3GppUserPlaneRateControlAttr *pAttr);
int Get3GppStatistics(Tac49xCommandPacketOpCode Opcode, int Device, int Channel);
int SendFskCallerIdMessage(int Device, int Channel, Tac49xCallerIdAttr *pAttr, U8 *pMessage);
int	TestEtsiDtmfCallerIdMessage(int Length, U8 *ptMessage, Tac49xCallerIdAttr *pAttr, Tac49xExtendedIbsString_Format1_Attr *pIbs);
int	TestDenmarkDtmfCallerIdMessage(int Length, U8 *ptMessage, Tac49xCallerIdAttr *pAttr, Tac49xExtendedIbsString_Format1_Attr *pIbs);
int	TestIndiaDtmfCallerIdMessage(int Length, U8 *ptMessage, Tac49xCallerIdAttr *pAttr, Tac49xExtendedIbsString_Format1_Attr *pIbs);
int	TestBrazilDtmfCallerIdMessage(int Length, U8 *ptMessage, Tac49xCallerIdAttr *pAttr, Tac49xExtendedIbsString_Format1_Attr *pIbs);
int	TestBrazilDtmfCallerIdMessage(int Length, U8 *ptMessage, Tac49xCallerIdAttr *pAttr, Tac49xExtendedIbsString_Format1_Attr *pIbs);
int GetChannelMediaProtectionInfo(Tac49xCommandPacketOpCode Opcode, int Device, int Channel, Tac49xGetMediaProtectiontInfoAttr *pAttr);
int SendRtcpAppCommand(Tac49xCommandPacketOpCode Opcode, int Device, int Channel, Tac49xRtcpAppCommandAttr *pAttr);
int GetRtcpXrMarkovModelStatisticsCommand(Tac49xCommandPacketOpCode OpCode, int Device, int Channel);
int GetRtcpXrVoiceQualityStatisticsCommand(Tac49xCommandPacketOpCode OpCode, int Device, int Channel);
int ActivateOrUpdateAtmChannelConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xActivateOrUpdateAtmChannelConfigurationAttr *pAttr);
int CloseRegularAtmChannelConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel);
int SendDtmfCallerIdMessage_Format1(int Device, int Channel, Tac49xCallerIdAttr *pAttr, U8 *pMessage);
void Ac49xSetDefaultRtcpXrConfigurationAttr(Tac49xRtcpXrConfigurationAttr *pAttr);
void Ac49xResetDriver(void);
void Ac49xUserDef_RaffleAndTrashPacketBits(int Device, int Channel, char *_pPacket, int _PacketSize, Tac49xTransferMedium TransferMedium);
int Ac49xCloseMediatedRtpChannelConfiguration(int Device, int Channel, Tac49xCloseRtpChannelConfigurationAttr *pAttr);
int HcrfDeviceReload(int Device, int ReferenceDevice, Tac49xHcrfStatusRegisterIndex *pResponseValue);
int CloseRtpChannelConfiguration(Tac49xChannelConfigurationOpCode OpCode, int Device, int Channel, Tac49xCloseRtpChannelConfigurationAttr *pAttr);
void get_full_packet(int Device, char *pPacket, U32 PacketSize, Tac49xPacketDirection PacketDirection);
//int do_loopback_test(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
void Initialize(void);
void Finalize(void);
Tac49xPacketRecordingErrorType Add(int Device, char *pPacket, unsigned int PacketSize, Tac49xPacketDirection PacketDirection);

int ac491SendData(int Device, int Channel,U8 *Buffer, U16 Size, Tac49xProtocol Protocol);

#if __cplusplus
}
#endif

#endif /* ifndef AC49XLO_H */
