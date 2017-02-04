#ifndef _COMMON_H_
#define _COMMON_H_

#ifndef __uint8__
	#define __uint8__
	typedef unsigned char  uint8;                  
#endif

#ifndef __int8__
	#define __int8__
	typedef signed   char  int8;  
#endif

#ifndef __uint16__
	#define __uint16__
	typedef unsigned short uint16;   
#endif

#ifndef __int16__
	#define __int16__
	typedef signed   short int16;         
#endif

#ifndef __uint32__
	#define __uint32__
	typedef unsigned int   uint32; 
#endif

#ifndef __int32__
	#define __int32__
	typedef signed   int   int32;       
#endif

#ifndef __fp32__
	#define __fp32__
	typedef float          fp32;  
#endif

#ifndef __fp64__
	#define __fp64__
	typedef double         fp64;                  
#endif


#undef PRINT_ERR
#define PRINT_ERR
#ifdef PRINT_ERR
	#define ERR(fmt, args ... ) 	 		printf(fmt, ##args)
#else
	#define ERR(x)  
#endif

#undef PRINT_LOG
#define PRINT_LOG
#ifdef PRINT_LOG
	#define LOG(fmt, args ... ) 	 		printf(fmt, ##args)
#else
	#define LOG(x)  
#endif


#undef PRINT_DBG
#define PRINT_DBG
#ifdef PRINT_DBG
	#define DBG(fmt, args ... ) 	 		printf(fmt, ##args)
	//#define DBG(...) 				printf(__VA_ARGS__)
#else
	#define DBG(x)  
#endif

#undef DRV_OK
#define DRV_OK (0)
#undef DRV_ERR
#define DRV_ERR (-1)

#define INFO			"INFO"
#define UART 		"UART"
#define GPORT 		"GPORT"
#define ETH 			"ETH"
#define PHONE 		"PHONE"
#define DBG_CFG		"DBG"
#define END			"END"

#define CHETONG_SEAT_MAX	12
#define NUMBER_OF_CHANNELS					5



#endif
