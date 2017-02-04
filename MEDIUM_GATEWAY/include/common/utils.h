/*! \file
 * \brief Utility functions
 */

#ifndef _INC_CC_UTILS_H
#define _INC_CC_UTILS_H

//#include "common/compat.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <arpa/inet.h>	/* we want to override inet_ntoa */
#include <netdb.h>
#include <limits.h>
#include <time.h>	/* we want to override localtime_r */
#include <unistd.h>

#include "common/lock.h"
#include "common/time.h"
#include "common/strings.h"
#include "common/logger.h"
#include "common/compiler.h"

/*! \note
 \verbatim
   Note:
   It is very important to use only unsigned variables to hold
   bit flags, as otherwise you can fall prey to the compiler's
   sign-extension antics if you try to use the top two bits in
   your variable.

   The flag macros below use a set of compiler tricks to verify
   that the caller is using an "unsigned int" variable to hold
   the flags, and nothing else. If the caller uses any other
   type of variable, a warning message similar to this:

   warning: comparison of distinct pointer types lacks cast
   will be generated.

   The "dummy" variable below is used to make these comparisons.

   Also note that at -O2 or above, this type-safety checking
   does _not_ produce any additional object code at all.
 \endverbatim
*/

extern unsigned int __unsigned_int_flags_dummy;

#define inc_test_flag(p,flag) 		({ \
					typeof ((p)->flags) __p = (p)->flags; \
					typeof (__unsigned_int_flags_dummy) __x = 0; \
					(void) (&__p == &__x); \
					((p)->flags & (flag)); \
					})

#define inc_set_flag(p,flag) 		do { \
					typeof ((p)->flags) __p = (p)->flags; \
					typeof (__unsigned_int_flags_dummy) __x = 0; \
					(void) (&__p == &__x); \
					((p)->flags |= (flag)); \
					} while(0)

#define inc_clear_flag(p,flag) 		do { \
					typeof ((p)->flags) __p = (p)->flags; \
					typeof (__unsigned_int_flags_dummy) __x = 0; \
					(void) (&__p == &__x); \
					((p)->flags &= ~(flag)); \
					} while(0)

#define inc_copy_flags(dest,src,flagz)	do { \
					typeof ((dest)->flags) __d = (dest)->flags; \
					typeof ((src)->flags) __s = (src)->flags; \
					typeof (__unsigned_int_flags_dummy) __x = 0; \
					(void) (&__d == &__x); \
					(void) (&__s == &__x); \
					(dest)->flags &= ~(flagz); \
					(dest)->flags |= ((src)->flags & (flagz)); \
					} while (0)

#define inc_set2_flag(p,value,flag)	do { \
					typeof ((p)->flags) __p = (p)->flags; \
					typeof (__unsigned_int_flags_dummy) __x = 0; \
					(void) (&__p == &__x); \
					if (value) \
						(p)->flags |= (flag); \
					else \
						(p)->flags &= ~(flag); \
					} while (0)

#define inc_set_flags_to(p,flag,value)	do { \
					typeof ((p)->flags) __p = (p)->flags; \
					typeof (__unsigned_int_flags_dummy) __x = 0; \
					(void) (&__p == &__x); \
					(p)->flags &= ~(flag); \
					(p)->flags |= (value); \
					} while (0)

/* Non-type checking variations for non-unsigned int flags.  You
   should only use non-unsigned int flags where required by 
   protocol etc and if you know what you're doing :)  */
#define inc_test_flag_nonstd(p,flag) \
					((p)->flags & (flag))

#define inc_set_flag_nonstd(p,flag) 		do { \
					((p)->flags |= (flag)); \
					} while(0)

#define inc_clear_flag_nonstd(p,flag) 		do { \
					((p)->flags &= ~(flag)); \
					} while(0)

#define inc_copy_flags_nonstd(dest,src,flagz)	do { \
					(dest)->flags &= ~(flagz); \
					(dest)->flags |= ((src)->flags & (flagz)); \
					} while (0)

#define inc_set2_flag_nonstd(p,value,flag)	do { \
					if (value) \
						(p)->flags |= (flag); \
					else \
						(p)->flags &= ~(flag); \
					} while (0)

#define INC_FLAGS_ALL UINT_MAX

struct inc_flags {
	unsigned int flags;
};

struct inc_hostent {
	struct hostent hp;
	char buf[1024];
};

struct hostent *inc_gethostbyname(const char *host, struct inc_hostent *hp);

/* inc_md5_hash 
	\brief Produces MD5 hash based on input string */
void inc_md5_hash(char *output, char *input);

int inc_base64encode_full(char *dst, const unsigned char *src, int srclen, int max, int linebreaks);
int inc_base64encode(char *dst, const unsigned char *src, int srclen, int max);
int inc_base64decode(unsigned char *dst, const char *src, int max);

/*! inc_uri_encode
	\brief Turn text string to URI-encoded %XX version 
 	At this point, we're converting from ISO-8859-x (8-bit), not UTF8
	as in the SIP protocol spec 
	If doreserved == 1 we will convert reserved characters also.
	RFC 2396, section 2.4
	outbuf needs to have more memory allocated than the instring
	to have room for the expansion. Every char that is converted
	is replaced by three ASCII characters.
	\param string	String to be converted
	\param outbuf	Resulting encoded string
	\param buflen	Size of output buffer
	\param doreserved	Convert reserved characters
*/

char *inc_uri_encode(const char *string, char *outbuf, int buflen, int doreserved);

/*!	\brief Decode URI, URN, URL (overwrite string)
	\param s	String to be decoded 
 */
void inc_uri_decode(char *s);

static force_inline void inc_slinear_saturated_add(short *input, short *value)
{
	int res;

	res = (int) *input + *value;
	if (res > 32767)
		*input = 32767;
	else if (res < -32767)
		*input = -32767;
	else
		*input = (short) res;
}
	
static force_inline void inc_slinear_saturated_multiply(short *input, short *value)
{
	int res;

	res = (int) *input * *value;
	if (res > 32767)
		*input = 32767;
	else if (res < -32767)
		*input = -32767;
	else
		*input = (short) res;
}

static force_inline void inc_slinear_saturated_divide(short *input, short *value)
{
	*input /= *value;
}

/*!
 * \brief thread-safe replacement for inet_ntoa().
 *
 * \note It is very important to note that even though this is a thread-safe
 *       replacement for inet_ntoa(), it is *not* reentrant.  In a single
 *       thread, the result from a previous call to this function is no longer
 *       valid once it is called again.  If the result from multiple calls to
 *       this function need to be kept or used at once, then the result must be
 *       copied to a local buffer before calling this function again.
 */
const char *inc_inet_ntoa(struct in_addr ia);

#ifdef inet_ntoa
#undef inet_ntoa
#endif
//#define inet_ntoa __dont__use__inet_ntoa__use__inc_inet_ntoa__instead__

int inc_utils_init(void);
int inc_wait_for_input(int fd, int ms);

/*! inc_carefulwrite
	\brief Try to write string, but wait no more than ms milliseconds
	before timing out.

	\note If you are calling inc_carefulwrite, it is assumed that you are calling
	it on a file descriptor that _DOES_ have NONBLOCK set.  This way,
	there is only one system call made to do a write, unless we actually
	have a need to wait.  This way, we get better performance.
*/
int inc_carefulwrite(int fd, char *s, int len, int timeoutms);

/*! Compares the source address and port of two sockaddr_in */
static force_inline int inaddrcmp(const struct sockaddr_in *sin1, const struct sockaddr_in *sin2)
{
	return ((sin1->sin_addr.s_addr != sin2->sin_addr.s_addr) 
		|| (sin1->sin_port != sin2->sin_port));
}

#define INC_STACKSIZE (((sizeof(void *) * 8 * 8) - 16) * 1024)

#if defined(LOW_MEMORY)
#define INC_BACKGROUND_STACKSIZE (((sizeof(void *) * 8 * 2) - 16) * 1024)
#else
#define INC_BACKGROUND_STACKSIZE INC_STACKSIZE
#endif

void inc_register_thread(char *name);
void inc_unregister_thread(void *id);

int inc_pthread_create_stack(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *),
			     void *data, size_t stacksize, const char *file, const char *caller,
			     int line, const char *start_fn);

#define inc_pthread_create(a, b, c, d) inc_pthread_create_stack(a, b, c, d,			\
							        0,				\
	 						        __FILE__, __FUNCTION__,		\
 							        __LINE__, #c)

#define inc_pthread_create_background(a, b, c, d) inc_pthread_create_stack(a, b, c, d,			\
									   INC_BACKGROUND_STACKSIZE,	\
									   __FILE__, __FUNCTION__,	\
									   __LINE__, #c)

/*!
	\brief Process a string to find and replace characters
	\param start The string to analyze
	\param find The character to find
	\param replace_with The character that will replace the one we are looking for
*/
char *inc_process_quotes_and_slashes(char *start, char find, char replace_with);

#ifdef linux
#define inc_random random
#else
long int inc_random(void);
#endif

/*! 
 * \brief free() wrapper
 *
 * inc_free_ptr should be used when a function pointer for free() needs to be passed
 * as the argument to a function. Otherwise, astmm will cause seg faults.
 */
#ifdef __INC_DEBUG_MALLOC
static void inc_free_ptr(void *ptr) attribute_unused;
static void inc_free_ptr(void *ptr)
{
	free(ptr);
}
#else
#define inc_free free
#define inc_free_ptr inc_free
#endif

#ifndef __INC_DEBUG_MALLOC

#define MALLOC_FAILURE_MSG \
	inc_log(LOG_ERROR, "Memory Allocation Failure in function %s at line %d of %s\n", func, lineno, file);
/*!
 * \brief A wrapper for malloc()
 *
 * inc_malloc() is a wrapper for malloc() that will generate an INC_CC log
 * message in the case that the allocation fails.
 *
 * The argument and return value are the same as malloc()
 */
#define inc_malloc(len) \
	_inc_malloc((len), __FILE__, __LINE__, __PRETTY_FUNCTION__)

INC_INLINE_API(
void * attribute_malloc _inc_malloc(size_t len, const char *file, int lineno, const char *func),
{
	void *p;

	if (!(p = malloc(len)))
		MALLOC_FAILURE_MSG;

	return p;
}
)

/*!
 * \brief A wrapper for calloc()
 *
 * inc_calloc() is a wrapper for calloc() that will generate an INC_CC log
 * message in the case that the allocation fails.
 *
 * The arguments and return value are the same as calloc()
 */
#define inc_calloc(num, len) \
	_inc_calloc((num), (len), __FILE__, __LINE__, __PRETTY_FUNCTION__)

INC_INLINE_API(
void * attribute_malloc _inc_calloc(size_t num, size_t len, const char *file, int lineno, const char *func),
{
	void *p;

	if (!(p = calloc(num, len)))
		MALLOC_FAILURE_MSG;

	return p;
}
)

/*!
 * \brief A wrapper for calloc() for use in cache pools
 *
 * inc_calloc_cache() is a wrapper for calloc() that will generate an INC_CC log
 * message in the case that the allocation fails. When memory debugging is in use,
 * the memory allocated by this function will be marked as 'cache' so it can be
 * distinguished from normal memory allocations.
 *
 * The arguments and return value are the same as calloc()
 */
#define inc_calloc_cache(num, len) \
	_inc_calloc((num), (len), __FILE__, __LINE__, __PRETTY_FUNCTION__)

/*!
 * \brief A wrapper for realloc()
 *
 * inc_realloc() is a wrapper for realloc() that will generate an INC_CC log
 * message in the case that the allocation fails.
 *
 * The arguments and return value are the same as realloc()
 */
#define inc_realloc(p, len) \
	_inc_realloc((p), (len), __FILE__, __LINE__, __PRETTY_FUNCTION__)

INC_INLINE_API(
void * attribute_malloc _inc_realloc(void *p, size_t len, const char *file, int lineno, const char *func),
{
	void *newp;

	if (!(newp = realloc(p, len)))
		MALLOC_FAILURE_MSG;

	return newp;
}
)

/*!
 * \brief A wrapper for strdup()
 *
 * inc_strdup() is a wrapper for strdup() that will generate an INC_CC log
 * message in the case that the allocation fails.
 *
 * inc_strdup(), unlike strdup(), can safely accept a NULL argument. If a NULL
 * argument is provided, inc_strdup will return NULL without generating any
 * kind of error log message.
 *
 * The argument and return value are the same as strdup()
 */
#define inc_strdup(str) \
	_inc_strdup((str), __FILE__, __LINE__, __PRETTY_FUNCTION__)

INC_INLINE_API(
char * attribute_malloc _inc_strdup(const char *str, const char *file, int lineno, const char *func),
{
	char *newstr = NULL;

	if (str) {
		if (!(newstr = strdup(str)))
			MALLOC_FAILURE_MSG;
	}

	return newstr;
}
)

/*!
 * \brief A wrapper for strndup()
 *
 * inc_strndup() is a wrapper for strndup() that will generate an INC_CC log
 * message in the case that the allocation fails.
 *
 * inc_strndup(), unlike strndup(), can safely accept a NULL argument for the
 * string to duplicate. If a NULL argument is provided, inc_strdup will return  
 * NULL without generating any kind of error log message.
 *
 * The arguments and return value are the same as strndup()
 */
#define inc_strndup(str, len) \
	_inc_strndup((str), (len), __FILE__, __LINE__, __PRETTY_FUNCTION__)

INC_INLINE_API(
char * attribute_malloc _inc_strndup(const char *str, size_t len, const char *file, int lineno, const char *func),
{
	char *newstr = NULL;

	if (str) {
		if (!(newstr = strndup(str, len)))
			MALLOC_FAILURE_MSG;
	}

	return newstr;
}
)

/*!
 * \brief A wrapper for asprintf()
 *
 * inc_asprintf() is a wrapper for asprintf() that will generate an INC_CC log
 * message in the case that the allocation fails.
 *
 * The arguments and return value are the same as asprintf()
 */
#define inc_asprintf(ret, fmt, ...) \
	_inc_asprintf((ret), __FILE__, __LINE__, __PRETTY_FUNCTION__, fmt, __VA_ARGS__)

int _inc_asprintf(char **ret, const char *file, int lineno, const char *func, const char *fmt, ...) __attribute__((format(printf, 5, 6)));

/*!
 * \brief A wrapper for vasprintf()
 *
 * inc_vasprintf() is a wrapper for vasprintf() that will generate an INC_CC log
 * message in the case that the allocation fails.
 *
 * The arguments and return value are the same as vasprintf()
 */
#define inc_vasprintf(ret, fmt, ap) \
	_inc_vasprintf((ret), __FILE__, __LINE__, __PRETTY_FUNCTION__, (fmt), (ap))

INC_INLINE_API(
int __attribute__((format(printf, 5, 0))) _inc_vasprintf(char **ret, const char *file, int lineno, const char *func, const char *fmt, va_list ap),
{
	int res;

	if ((res = vasprintf(ret, fmt, ap)) == -1)
		MALLOC_FAILURE_MSG;

	return res;
}
)

#endif /* INC_DEBUG_MALLOC */

#if !defined(inc_strdupa) && defined(__GNUC__)
/*!
  \brief duplicate a string in memory from the stack
  \param s The string to duplicate

  This macro will duplicate the given string.  It returns a pointer to the stack
  allocatted memory for the new string.
*/
#define inc_strdupa(s)                                                    \
	(__extension__                                                    \
	({                                                                \
		const char *__old = (s);                                  \
		size_t __len = strlen(__old) + 1;                         \
		char *__new = __builtin_alloca(__len);                    \
		memcpy (__new, __old, __len);                             \
		__new;                                                    \
	}))
#endif

/*!
  \brief Disable PMTU discovery on a socket
  \param sock The socket to manipulate
  \return Nothing

  On Linux, UDP sockets default to sending packets with the Dont Fragment (DF)
  bit set. This is supposedly done to allow the application to do PMTU
  discovery, but INC_CC does not do this.

  Because of this, UDP packets sent by INC_CC that are larger than the MTU
  of any hop in the path will be lost. This function can be called on a socket
  to ensure that the DF bit will not be set.
 */
void inc_enable_packet_fragmentation(int sock);

#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))

#ifdef INC_DEVMODE
#define inc_assert(a) _inc_assert(a, # a, __FILE__, __LINE__, __PRETTY_FUNCTION__)
static void force_inline _inc_assert(int condition, const char *condition_str, 
	const char *file, int line, const char *function)
{
	if (__builtin_expect(!condition, 1)) {
		/* Attempt to put it into the logger, but hope that at least someone saw the
		 * message on stderr ... */
		inc_log(LOG_ERROR, "FRACK!, Failed assertion %s (%d) at line %d in %s of %s\n",
			condition_str, condition, line, function, file);
		fprintf(stderr, "FRACK!, Failed assertion %s (%d) at line %d in %s of %s\n",
			condition_str, condition, line, function, file);
		/* Give the logger a chance to get the message out, just in case we abort(), or
		 * INC_CC crashes due to whatever problem just happened after we exit inc_assert(). */
		usleep(1);
#ifdef DO_CRASH
		abort();
		/* Just in case abort() doesn't work or something else super silly,
		 * and for Qwell's amusement. */
		*((int*)0)=0;
#endif
	}
}
#else
#define inc_assert(a)
#endif

#endif /* _INC_CC_UTILS_H */
