/*! \file
 * \brief main include file. File version handling, generic CC functions.
 */

#ifndef _INC_CC_MAIN_H
#define _INC_CC_MAIN_H

#include "common/autoconfig.h"
//#include "common/compat.h"

#include "common/paths.h"
#include "common/stringfields.h"

#define DEFAULT_LANGUAGE "en"

#define INC_MAX_USER_FIELD	256

#define DEFAULT_SAMPLE_RATE 8000
#define DEFAULT_SAMPLES_PER_MS  ((DEFAULT_SAMPLE_RATE)/1000)
#define	setpriority	__PLEASE_USE_inc_set_priority_INSTEAD_OF_setpriority__
#define	sched_setscheduler	__PLEASE_USE_inc_set_priority_INSTEAD_OF_sched_setscheduler__

#if !defined(STANDALONE) && !defined(STANDALONE_AEL)
/* These includes are all about ordering */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#endif


#define RESULT_SUCCESS		0
#define RESULT_SHOWUSAGE	1
#define RESULT_FAILURE		2

#define INC_MAX_EXTENSION	80	/*!< Max length of an extension */
#define INC_MAX_CONTEXT		80	/*!< Max length of a context */
#define INC_CHANNEL_NAME	80	/*!< Max length of an channel name */
#define MAX_LANGUAGE		20	/*!< Max length of the language setting */
#define MAX_MUSICCLASS		80	/*!< Max length of the music class setting */

#define INC_MAX_ACCOUNT_CODE		20

/* provided in inc_cc_main.c */

#define INC_TMP_DIR    "/var/spool/inc_ag/tmp"
#define PATH_MAX 	100

extern char inc_config_dir[PATH_MAX];
extern char inc_config_file[PATH_MAX];
extern char inc_config_log_dir[PATH_MAX];
extern char system_name[20];
extern int glog_level;

#define AG_TYPE_USER		1
#define AG_TYPE_G			2
#define AG_TYPE_RADIO		3

int inc_set_priority(int);			/*!< Provided by inc_cc_main.c */
int sip_load_module(int ag_type);			/*!< Provided by ag_sip.c */
int init_logger(void);				/*!< Provided by logger.c */
void close_logger(void);			/*!< Provided by logger.c */
int reload_logger(int);				/*!< Provided by logger.c */
int init_framer(void);				/*!< Provided by frame.c */
int inc_term_init(void);			/*!< Provided by term.c */
int inc_obj2_init(void);			/*! Provided by inc_obj2.c */

#define CC_MAX_FDS		8

enum {
	/*! Soft hangup by device */
	INC_SOFTHANGUP_DEV =       (1 << 0),
	/*! Soft hangup for async goto */
	INC_SOFTHANGUP_ASYNCGOTO = (1 << 1),
	INC_SOFTHANGUP_SHUTDOWN =  (1 << 2),
	INC_SOFTHANGUP_TIMEOUT =   (1 << 3),
	INC_SOFTHANGUP_APPUNLOAD = (1 << 4),
	INC_SOFTHANGUP_EXPLICIT =  (1 << 5),
	INC_SOFTHANGUP_UNBRIDGE =  (1 << 6),
};

/*! \brief Channel reload reasons for manager events at load or reload of configuration */
enum channelreloadreason {
	CHANNEL_MODULE_LOAD,
	CHANNEL_MODULE_RELOAD,
	CHANNEL_MANAGER_RELOAD,
};
/*! 
 * \brief call states
 *
 * \note Bits 0-15 of state are reserved for the state (up/down) of the line
 *       Bits 16-32 of state are reserved for flags
 */
enum call_state {
	/*! Channel is down and available */
	INC_STATE_DOWN,
	/*! Channel is down, but reserved */
	INC_STATE_RESERVED,
	/*! Channel is off hook */
	INC_STATE_OFFHOOK,
	/*! Digits (or equivalent) have been dialed */
	INC_STATE_DIALING,
	/*! Line is ringing */
	INC_STATE_RING,
	/*! Remote end is ringing */
	INC_STATE_RINGING,
	/*! Line is up */
	INC_STATE_UP,
	/*! Line is busy */
	INC_STATE_BUSY,
	/*! Digits (or equivalent) have been dialed while offhook */
	INC_STATE_DIALING_OFFHOOK,
	/*! Channel has detected an incoming call and is waiting for ring */
	INC_STATE_PRERING,

	/*! Do not transmit voice data */
	INC_STATE_MUTE = (1 << 16),
};

/*! \brief Structure for all kinds of caller ID identifications.
 * \note All string fields here are malloc'ed, so they need to be
 * freed when the structure is deleted.
 * Also, NULL and "" must be considered equivalent.
 */
struct cc_callerid {
	char *cid_dnid;		/*!< Malloc'd Dialed Number Identifier */
	char *cid_num;		/*!< Malloc'd Caller Number */
	char *cid_name;		/*!< Malloc'd Caller Name */
	char *cid_ani;		/*!< Malloc'd ANI */
	char *cid_rdnis;	/*!< Malloc'd RDNIS */
	int cid_pres;		/*!< Callerid presentation/screening */
	int cid_ani2;		/*!< Callerid ANI 2 (Info digits) */
	int cid_ton;		/*!< Callerid Type of Number */
	int cid_tns;		/*!< Callerid Transit Network Select */
};

/* Many headers need 'inc_module' to be defined */
struct inc_module;

/*!
 * \brief Reload INC_CC modules.
 * \param name the name of the module to reload
 *
 * This function reloads the specified module, or if no modules are specified,
 * it will reload all loaded modules.
 *
 * \note Modules are reloaded using their reload() functions, not unloading
 * them and loading them again.
 * 
 * \return Zero if the specified module was not found, 1 if the module was
 * found but cannot be reloaded, -1 if a reload operation is already in
 * progress, and 2 if the specfied module was found and reloaded.
 */
int inc_module_reload(const char *name);

/*!
 * \brief Process reload requests received during startup.
 *
 * This function requests that the loader execute the pending reload requests
 * that were queued during server startup.
 *
 * \note This function will do nothing if the server has not completely started
 *       up.  Once called, the reload queue is emptied, and further invocations
 *       will have no affect.
 */
void inc_process_pending_reloads(void);

/*!
 * \brief Register a function to be executed before INC_CC exits.
 * \param func The callback function to use.
 *
 * \return Zero on success, -1 on error.
 */
int inc_register_atexit(void (*func)(void));

/*!   
 * \brief Unregister a function registered with inc_register_atexit().
 * \param func The callback function to unregister.   
 */
void inc_unregister_atexit(void (*func)(void));

#if !defined(LOW_MEMORY)
int64_t inc_mark(int, int start1_stop0);
#else /* LOW_MEMORY */
#define inc_mark(a, b) do { } while (0)
#endif /* LOW_MEMORY */

#endif /* _INC_CC_MAIN_H */
