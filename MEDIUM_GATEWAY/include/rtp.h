/*!
 * \file rtp.h
 * \brief Supports RTP and RTCP with Symmetric RTP support for NAT traversal.
 *
 * RTP is defined in RFC 3550.
 */

#ifndef _INC_CC_RTP_H
#define _INC_CC_RTP_H

#include <netinet/in.h>

#include "common/frame.h"
#include "common/io.h"
#include "common/sched.h"
#include "common/linkedlists.h"
#include "sipua_common.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* Codes for RTP-specific data - not defined by our INC_FORMAT codes */
/*! DTMF (RFC2833) */
#define INC_RTP_DTMF            	(1 << 0)
/*! 'Comfort Noise' (RFC3389) */
#define INC_RTP_CN              	(1 << 1)
/*! DTMF (Cisco Proprietary) */
#define INC_RTP_CISCO_DTMF      	(1 << 2)
/*! Maximum RTP-specific code */
#define INC_RTP_MAX             	INC_RTP_CISCO_DTMF

#define MAX_RTP_PT			256

enum inc_rtp_options {
	INC_RTP_OPT_G726_NONSTANDARD = (1 << 0),
};

enum inc_rtp_get_result {
	/*! Failed to find the RTP structure */
	INC_RTP_GET_FAILED = 0,
	/*! RTP structure exists but true native bridge can not occur so try partial */
	INC_RTP_TRY_PARTIAL,
	/*! RTP structure exists and native bridge can occur */
	INC_RTP_TRY_NATIVE,
};

struct inc_rtp;

struct inc_rtp_protocol {
	/*! Get RTP struct, or NULL if unwilling to transfer */
	enum inc_rtp_get_result (* const get_rtp_info)(struct sip_pvt *chan, struct inc_rtp **rtp);
	/*! Get RTP struct, or NULL if unwilling to transfer */
	enum inc_rtp_get_result (* const get_vrtp_info)(struct sip_pvt *chan, struct inc_rtp **rtp);
	/*! Set RTP peer */
	int (* const set_rtp_peer)(struct sip_pvt *chan, struct inc_rtp *peer, struct inc_rtp *vpeer, int codecs, int nat_active);
	int (* const get_codec)(struct sip_pvt *chan);
	const char * const type;
	INC_LIST_ENTRY(inc_rtp_protocol) list;
};

struct inc_rtp_quality {
	unsigned int local_ssrc;          /* Our SSRC */
	unsigned int local_lostpackets;   /* Our lost packets */
	double       local_jitter;        /* Our calculated jitter */
	unsigned int local_count;         /* Number of received packets */
	unsigned int remote_ssrc;         /* Their SSRC */
	unsigned int remote_lostpackets;  /* Their lost packets */
	double       remote_jitter;       /* Their reported jitter */
	unsigned int remote_count;        /* Number of transmitted packets */
	double       rtt;                 /* Round trip time */
};


#define FLAG_3389_WARNING		(1 << 0)

typedef int (*inc_rtp_callback)(struct inc_rtp *rtp, struct inc_frame *f, void *data);

/*!
 * \brief Get the amount of space required to hold an RTP session
 * \return number of bytes required
 */
size_t inc_rtp_alloc_size(void);

/*!
 * \brief Initializate a RTP session.
 *
 * \param sched
 * \param io
 * \param rtcpenable
 * \param callbackmode
 * \returns A representation (structure) of an RTP session.
 */
struct inc_rtp *inc_rtp_new(struct sched_context *sched, struct io_context *io, int rtcpenable, int callbackmode);

/*!
 * \brief Initializate a RTP session using an in_addr structure.
 *
 * This fuction gets called by inc_rtp_new().
 *
 * \param sched
 * \param io
 * \param rtcpenable
 * \param callbackmode
 * \param in
 * \returns A representation (structure) of an RTP session.
 */
struct inc_rtp *inc_rtp_new_with_bindaddr(struct sched_context *sched, struct io_context *io, int rtcpenable, int callbackmode, struct in_addr in);

void inc_rtp_set_peer(struct inc_rtp *rtp, struct sockaddr_in *them);

/*! 
 * \since 1.4.26
 * \brief set potential alternate source for RTP media
 *
 * This function may be used to give the RTP stack a hint that there is a potential
 * second source of media. One case where this is used is when the SIP stack receives
 * a REINVITE to which it will be replying with a 491. In such a scenario, the IP and
 * port information in the SDP of that REINVITE lets us know that we may receive media
 * from that source/those sources even though the SIP transaction was unable to be completed
 * successfully
 *
 * \param rtp The RTP structure we wish to set up an alternate host/port on
 * \param alt The address information for the alternate media source
 * \retval void
 */
void inc_rtp_set_alt_peer(struct inc_rtp *rtp, struct sockaddr_in *alt);

/* Copies from rtp to them and returns 1 if there was a change or 0 if it was already the same */
int inc_rtp_get_peer(struct inc_rtp *rtp, struct sockaddr_in *them);

void inc_rtp_get_us(struct inc_rtp *rtp, struct sockaddr_in *us);

struct inc_rtp *inc_rtp_get_bridged(struct inc_rtp *rtp);

void inc_rtp_destroy(struct inc_rtp *rtp);

void inc_rtp_reset(struct inc_rtp *rtp);

void inc_rtp_stun_request(struct inc_rtp *rtp, struct sockaddr_in *suggestion, const char *username);

void inc_rtp_set_callback(struct inc_rtp *rtp, inc_rtp_callback callback);

void inc_rtp_set_data(struct inc_rtp *rtp, void *data);

int inc_rtp_write(struct inc_rtp *rtp, ST_SIP_IPC_MSG_PACK_WRITE *f);

struct inc_frame *inc_rtp_read(struct inc_rtp *rtp);

struct inc_frame *inc_rtcp_read(struct inc_rtp *rtp);

int inc_rtp_fd(struct inc_rtp *rtp);

int inc_rtcp_fd(struct inc_rtp *rtp);

int inc_rtp_senddigit_begin(struct inc_rtp *rtp, char digit);

int inc_rtp_senddigit_end(struct inc_rtp *rtp, char digit);

int inc_rtp_sendcng(struct inc_rtp *rtp, int level);

int inc_rtp_settos(struct inc_rtp *rtp, int tos);

/*! \brief When changing sources, don't generate a new SSRC */
void inc_rtp_set_constantssrc(struct inc_rtp *rtp);

void inc_rtp_new_source(struct inc_rtp *rtp);

/*! \brief  Setting RTP payload types from lines in a SDP description: */
void inc_rtp_pt_clear(struct inc_rtp* rtp);
/*! \brief Set payload types to defaults */
void inc_rtp_pt_default(struct inc_rtp* rtp);

/*! \brief Copy payload types between RTP structures */
void inc_rtp_pt_copy(struct inc_rtp *dest, struct inc_rtp *src);

/*! \brief Activate payload type */
void inc_rtp_set_m_type(struct inc_rtp* rtp, int pt);

/*! \brief clear payload type */
void inc_rtp_unset_m_type(struct inc_rtp* rtp, int pt);

/*! \brief Initiate payload type to a known MIME media type for a codec */
int inc_rtp_set_rtpmap_type(struct inc_rtp* rtp, int pt,
			     char *mimeType, char *mimeSubtype,
			     enum inc_rtp_options options);

/*! \brief  Mapping between RTP payload format codes and INC_CC codes: */
struct rtpPayloadType inc_rtp_lookup_pt(struct inc_rtp* rtp, int pt);
int inc_rtp_lookup_code(struct inc_rtp* rtp, int isAgFormat, int code);

void inc_rtp_get_current_formats(struct inc_rtp* rtp,
			     int* agFormats, int* nonAgFormats);

/*! \brief  Mapping an INC_CC code into a MIME subtype (string): */
const char *inc_rtp_lookup_mime_subtype(int isAgFormat, int code,
					enum inc_rtp_options options);

/*! \brief Build a string of MIME subtype names from a capability list */
char *inc_rtp_lookup_mime_multiple(char *buf, size_t size, const int capability,
				   const int isAgFormat, enum inc_rtp_options options);

void inc_rtp_setnat(struct inc_rtp *rtp, int nat);

int inc_rtp_getnat(struct inc_rtp *rtp);

/*! \brief Indicate whether this RTP session is carrying DTMF or not */
void inc_rtp_setdtmf(struct inc_rtp *rtp, int dtmf);

/*! \brief Compensate for devices that send RFC2833 packets all at once */
void inc_rtp_setdtmfcompensate(struct inc_rtp *rtp, int compensate);

/*! \brief Enable STUN capability */
void inc_rtp_setstun(struct inc_rtp *rtp, int stun_enable);

int inc_rtp_proto_register(struct inc_rtp_protocol *proto);

void inc_rtp_proto_unregister(struct inc_rtp_protocol *proto);

void inc_rtp_stop(struct inc_rtp *rtp);

/*! \brief Return RTCP quality string */
char *inc_rtp_get_quality(struct inc_rtp *rtp, struct inc_rtp_quality *qual);

void inc_rtp_new_init(struct inc_rtp *rtp);

void inc_rtp_init(void);

int inc_rtp_reload(void);

int inc_rtp_codec_setpref(struct inc_rtp *rtp, struct inc_codec_pref *prefs);

struct inc_codec_pref *inc_rtp_codec_getpref(struct inc_rtp *rtp);

int inc_rtp_codec_getformat(int pt);

/*! \brief Set rtp timeout */
void inc_rtp_set_rtptimeout(struct inc_rtp *rtp, int timeout);
/*! \brief Set rtp hold timeout */
void inc_rtp_set_rtpholdtimeout(struct inc_rtp *rtp, int timeout);
/*! \brief set RTP keepalive interval */
void inc_rtp_set_rtpkeepalive(struct inc_rtp *rtp, int period);
/*! \brief Get RTP keepalive interval */
int inc_rtp_get_rtpkeepalive(struct inc_rtp *rtp);
/*! \brief Get rtp hold timeout */
int inc_rtp_get_rtpholdtimeout(struct inc_rtp *rtp);
/*! \brief Get rtp timeout */
int inc_rtp_get_rtptimeout(struct inc_rtp *rtp);
/* \brief Put RTP timeout timers on hold during another transaction, like T.38 */
void inc_rtp_set_rtptimers_onhold(struct inc_rtp *rtp);


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _INC_CC_RTP_H */
