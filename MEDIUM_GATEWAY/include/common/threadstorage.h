/*!
 * \file threadstorage.h
 * \author Russell Bryant <russell@digium.com>
 *
 * \brief Definitions to aid in the use of thread local storage
*/

#ifndef _INC_CC_THREADSTORAGE_H
#define _INC_CC_THREADSTORAGE_H

#include <pthread.h>

#include "common/utils.h"
#include "common/inline_api.h"

/*!
 * \brief data for a thread locally stored variable
 */
struct inc_threadstorage {
	/*! Ensure that the key is only initialized by one thread */
	pthread_once_t once;
	/*! The key used to retrieve this thread's data */
	pthread_key_t key;
	/*! The function that initializes the key */
	void (*key_init)(void);
};

#if defined(DEBUG_THREADLOCALS)
void __inc_threadstorage_object_add(void *key, size_t len, const char *file, const char *function, unsigned int line);
void __inc_threadstorage_object_remove(void *key);
void __inc_threadstorage_object_replace(void *key_old, void *key_new, size_t len);
#endif /* defined(DEBUG_THREADLOCALS) */

/*!
 * \brief Define a thread storage variable
 *
 * \arg name The name of the thread storage
 * \arg name_init This is a name used to create the function that gets called
 *      to initialize this thread storage. It can be anything since it will not
 *      be referred to anywhere else
 *
 * This macro would be used to declare an instance of thread storage in a file.
 *
 * Example usage:
 * \code
 * INC_THREADSTORAGE(my_buf, my_buf_init);
 * \endcode
 */
#define INC_THREADSTORAGE(name, name_init) \
	INC_THREADSTORAGE_CUSTOM(name, name_init, inc_free_ptr) 

#if defined(PTHREAD_ONCE_INIT_NEEDS_BRACES)
# define INC_PTHREAD_ONCE_INIT { PTHREAD_ONCE_INIT }
#else
# define INC_PTHREAD_ONCE_INIT PTHREAD_ONCE_INIT
#endif

#if !defined(DEBUG_THREADLOCALS)
#define INC_THREADSTORAGE_CUSTOM(name, name_init, cleanup)  \
static void name_init(void);                                \
static struct inc_threadstorage name = {                    \
	.once = INC_PTHREAD_ONCE_INIT,                          \
	.key_init = name_init,                                  \
};                                                          \
static void name_init(void)                                 \
{                                                           \
	pthread_key_create(&(name).key, cleanup);               \
}
#else /* defined(DEBUG_THREADLOCALS) */
#define INC_THREADSTORAGE_CUSTOM(name, name_init, cleanup)  \
static void name_init(void);                                \
static struct inc_threadstorage name = {                    \
	.once = INC_PTHREAD_ONCE_INIT,                          \
	.key_init = name_init,                                  \
};                                                          \
static void __cleanup_##name(void *data)                    \
{                                                           \
	__inc_threadstorage_object_remove(data);                \
	cleanup(data);                                          \
}                                                           \
static void name_init(void)                                 \
{                                                           \
	pthread_key_create(&(name).key, __cleanup_##name);      \
}
#endif /* defined(DEBUG_THREADLOCALS) */

/*!
 * \brief Retrieve thread storage
 *
 * \arg ts This is a pointer to the thread storage structure declared by using
 *      the INC_THREADSTORAGE macro.  If declared with 
 *      INC_THREADSTORAGE(my_buf, my_buf_init), then this argument would be 
 *      (&my_buf).
 * \arg init_size This is the amount of space to be allocated the first time
 *      this thread requests its data. Thus, this should be the size that the
 *      code accessing this thread storage is assuming the size to be.
 *
 * \return This function will return the thread local storage associated with
 *         the thread storage management variable passed as the first argument.
 *         The result will be NULL in the case of a memory allocation error.
 *
 * Example usage:
 * \code
 * INC_THREADSTORAGE(my_buf, my_buf_init);
 * #define MY_BUF_SIZE   128
 * ...
 * void my_func(const char *fmt, ...)
 * {
 *      void *buf;
 *
 *      if (!(buf = inc_threadstorage_get(&my_buf, MY_BUF_SIZE)))
 *           return;
 *      ...
 * }
 * \endcode
 */
#if !defined(DEBUG_THREADLOCALS)
INC_INLINE_API(
void *inc_threadstorage_get(struct inc_threadstorage *ts, size_t init_size),
{
	void *buf;

	pthread_once(&ts->once, ts->key_init);
	if (!(buf = pthread_getspecific(ts->key))) {
		if (!(buf = inc_calloc(1, init_size)))
			return NULL;
		pthread_setspecific(ts->key, buf);
	}

	return buf;
}
)
#else /* defined(DEBUG_THREADLOCALS) */
INC_INLINE_API(
void *__inc_threadstorage_get(struct inc_threadstorage *ts, size_t init_size, const char *file, const char *function, unsigned int line),
{
	void *buf;

	pthread_once(&ts->once, ts->key_init);
	if (!(buf = pthread_getspecific(ts->key))) {
		if (!(buf = inc_calloc(1, init_size)))
			return NULL;
		pthread_setspecific(ts->key, buf);
		__inc_threadstorage_object_add(buf, init_size, file, function, line);
	}

	return buf;
}
)

#define inc_threadstorage_get(ts, init_size) __inc_threadstorage_get(ts, init_size, __FILE__, __PRETTY_FUNCTION__, __LINE__)
#endif /* defined(DEBUG_THREADLOCALS) */

/*!
 * \brief A dynamic length string
 */
struct inc_dynamic_str {
	/* The current maximum length of the string */
	size_t len;
	/* The string buffer */
	char str[0];
};

/*!
 * \brief Create a dynamic length string
 *
 * \arg init_len This is the initial length of the string buffer
 *
 * \return This function returns a pointer to the dynamic string length.  The
 *         result will be NULL in the case of a memory allocation error.
 *
 * /note The result of this function is dynamically allocated memory, and must
 *       be free()'d after it is no longer needed.
 */
INC_INLINE_API(
struct inc_dynamic_str * attribute_malloc inc_dynamic_str_create(size_t init_len),
{
	struct inc_dynamic_str *buf;

	if (!(buf = inc_calloc(1, sizeof(*buf) + init_len)))
		return NULL;
	
	buf->len = init_len;

	return buf;
}
)

/*!
 * \brief Retrieve a thread locally stored dynamic string
 *
 * \arg ts This is a pointer to the thread storage structure declared by using
 *      the INC_THREADSTORAGE macro.  If declared with 
 *      INC_THREADSTORAGE(my_buf, my_buf_init), then this argument would be 
 *      (&my_buf).
 * \arg init_len This is the initial length of the thread's dynamic string. The
 *      current length may be bigger if previous operations in this thread have
 *      caused it to increase.
 *
 * \return This function will return the thread locally storaged dynamic string
 *         associated with the thread storage management variable passed as the
 *         first argument.
 *         The result will be NULL in the case of a memory allocation error.
 *
 * Example usage:
 * \code
 * INC_THREADSTORAGE(my_str, my_str_init);
 * #define MY_STR_INIT_SIZE   128
 * ...
 * void my_func(const char *fmt, ...)
 * {
 *      struct inc_dynamic_str *buf;
 *
 *      if (!(buf = inc_dynamic_str_thread_get(&my_str, MY_STR_INIT_SIZE)))
 *           return;
 *      ...
 * }
 * \endcode
 */
#if !defined(DEBUG_THREADLOCALS)
INC_INLINE_API(
struct inc_dynamic_str *inc_dynamic_str_thread_get(struct inc_threadstorage *ts,
	size_t init_len),
{
	struct inc_dynamic_str *buf;

	if (!(buf = inc_threadstorage_get(ts, sizeof(*buf) + init_len)))
		return NULL;
	
	if (!buf->len)
		buf->len = init_len;

	return buf;
}
)
#else /* defined(DEBUG_THREADLOCALS) */
INC_INLINE_API(
struct inc_dynamic_str *__inc_dynamic_str_thread_get(struct inc_threadstorage *ts,
	size_t init_len, const char *file, const char *function, unsigned int line),
{
	struct inc_dynamic_str *buf;

	if (!(buf = __inc_threadstorage_get(ts, sizeof(*buf) + init_len, file, function, line)))
		return NULL;
	
	if (!buf->len)
		buf->len = init_len;

	return buf;
}
)

#define inc_dynamic_str_thread_get(ts, init_len) __inc_dynamic_str_thread_get(ts, init_len, __FILE__, __PRETTY_FUNCTION__, __LINE__)
#endif /* defined(DEBUG_THREADLOCALS) */ 

/*!
 * \brief Error codes from inc_dynamic_str_thread_build_va()
 */
enum {
	/*! An error has occured and the contents of the dynamic string
	 *  are undefined */
	INC_DYNSTR_BUILD_FAILED = -1,
	/*! The buffer size for the dynamic string had to be increased, and
	 *  inc_dynamic_str_thread_build_va() needs to be called again after
	 *  a va_end() and va_start().
	 */
	INC_DYNSTR_BUILD_RETRY = -2
};

/*!
 * \brief Set a thread locally stored dynamic string from a va_list
 *
 * \arg buf This is the address of a pointer to an inc_dynamic_str which should
 *      have been retrieved using inc_dynamic_str_thread_get.  It will need to
 *      be updated in the case that the buffer has to be reallocated to
 *      accomodate a longer string than what it currently has space for.
 * \arg max_len This is the maximum length to allow the string buffer to grow
 *      to.  If this is set to 0, then there is no maximum length.
 * \arg ts This is a pointer to the thread storage structure declared by using
 *      the INC_THREADSTORAGE macro.  If declared with 
 *      INC_THREADSTORAGE(my_buf, my_buf_init), then this argument would be 
 *      (&my_buf).
 * \arg fmt This is the format string (printf style)
 * \arg ap This is the va_list
 *
 * \return The return value of this function is the same as that of the printf
 *         family of functions.
 *
 * Example usage:
 * \code
 * INC_THREADSTORAGE(my_str, my_str_init);
 * #define MY_STR_INIT_SIZE   128
 * ...
 * void my_func(const char *fmt, ...)
 * {
 *      struct inc_dynamic_str *buf;
 *      va_list ap;
 *
 *      if (!(buf = inc_dynamic_str_thread_get(&my_str, MY_STR_INIT_SIZE)))
 *           return;
 *      ...
 *      va_start(fmt, ap);
 *      inc_dynamic_str_thread_set_va(&buf, 0, &my_str, fmt, ap);
 *      va_end(ap);
 * 
 *      printf("This is the string we just built: %s\n", buf->str);
 *      ...
 * }
 * \endcode
 */
#define inc_dynamic_str_thread_set_va(buf, max_len, ts, fmt, ap)                 \
	({                                                                       \
		int __res;                                                       \
		while ((__res = inc_dynamic_str_thread_build_va(buf, max_len,    \
			ts, 0, fmt, ap)) == INC_DYNSTR_BUILD_RETRY) {            \
			va_end(ap);                                              \
			va_start(ap, fmt);                                       \
		}                                                                \
		(__res);                                                         \
	})

/*!
 * \brief Append to a thread local dynamic string using a va_list
 *
 * The arguments, return values, and usage of this are the same as those for
 * inc_dynamic_str_thread_set_va().  However, instead of setting a new value
 * for the string, this will append to the current value.
 */
#define inc_dynamic_str_thread_append_va(buf, max_len, ts, fmt, ap)              \
	({                                                                       \
		int __res;                                                       \
		while ((__res = inc_dynamic_str_thread_build_va(buf, max_len,    \
			ts, 1, fmt, ap)) == INC_DYNSTR_BUILD_RETRY) {            \
			va_end(ap);                                              \
			va_start(ap, fmt);                                       \
		}                                                                \
		(__res);                                                         \
	})

/*!
 * \brief Core functionality of inc_dynamic_str_thread_(set|append)_va
 *
 * The arguments to this function are the same as those described for
 * inc_dynamic_str_thread_set_va except for an addition argument, append.
 * If append is non-zero, this will append to the current string instead of
 * writing over it.
 */
int inc_dynamic_str_thread_build_va(struct inc_dynamic_str **buf, size_t max_len,
				    struct inc_threadstorage *ts, int append, const char *fmt, va_list ap)  __attribute__((format(printf, 5, 0)));

/*!
 * \brief Set a thread locally stored dynamic string using variable arguments
 *
 * \arg buf This is the address of a pointer to an inc_dynamic_str which should
 *      have been retrieved using inc_dynamic_str_thread_get.  It will need to
 *      be updated in the case that the buffer has to be reallocated to
 *      accomodate a longer string than what it currently has space for.
 * \arg max_len This is the maximum length to allow the string buffer to grow
 *      to.  If this is set to 0, then there is no maximum length.
 * \arg ts This is a pointer to the thread storage structure declared by using
 *      the INC_THREADSTORAGE macro.  If declared with 
 *      INC_THREADSTORAGE(my_buf, my_buf_init), then this argument would be 
 *      (&my_buf).
 * \arg fmt This is the format string (printf style)
 *
 * \return The return value of this function is the same as that of the printf
 *         family of functions.
 *
 * Example usage:
 * \code
 * INC_THREADSTORAGE(my_str, my_str_init);
 * #define MY_STR_INIT_SIZE   128
 * ...
 * void my_func(int arg1, int arg2)
 * {
 *      struct inc_dynamic_str *buf;
 *      va_list ap;
 *
 *      if (!(buf = inc_dynamic_str_thread_get(&my_str, MY_STR_INIT_SIZE)))
 *           return;
 *      ...
 *      inc_dynamic_str_thread_set(&buf, 0, &my_str, "arg1: %d  arg2: %d\n",
 *           arg1, arg2);
 * 
 *      printf("This is the string we just built: %s\n", buf->str);
 *      ...
 * }
 * \endcode
 */

int inc_dynamic_str_thread_set(struct inc_dynamic_str **buf, size_t max_len, 
								struct inc_threadstorage *ts, const char *fmt, ...) __attribute__((format(printf, 4, 5)));

/*!
 * \brief Append to a thread local dynamic string
 *
 * The arguments, return values, and usage of this function are the same as
 * inc_dynamic_str_thread_set().  However, instead of setting a new value for
 * the string, this function appends to the current value.
 */
INC_INLINE_API(
int __attribute__((format(printf, 4, 5))) inc_dynamic_str_thread_append(
	struct inc_dynamic_str **buf, size_t max_len, 
	struct inc_threadstorage *ts, const char *fmt, ...),
{
	int res;
	va_list ap;

	va_start(ap, fmt);
	res = inc_dynamic_str_thread_append_va(buf, max_len, ts, fmt, ap);
	va_end(ap);

	return res;
}
)

/*!
 * \brief Set a dynamic string
 *
 * \arg buf This is the address of a pointer to an inc_dynamic_str.  It will
 *      need to be updated in the case that the buffer has to be reallocated to
 *      accomodate a longer string than what it currently has space for.
 * \arg max_len This is the maximum length to allow the string buffer to grow
 *      to.  If this is set to 0, then there is no maximum length.
 *
 * \return The return value of this function is the same as that of the printf
 *         family of functions.
 */
INC_INLINE_API(
int __attribute__((format(printf, 3, 4))) inc_dynamic_str_set(
	struct inc_dynamic_str **buf, size_t max_len,
	const char *fmt, ...),
{
	int res;
	va_list ap;
	
	va_start(ap, fmt);
	res = inc_dynamic_str_thread_set_va(buf, max_len, NULL, fmt, ap);
	va_end(ap);

	return res;
}
)

/*!
 * \brief Append to a dynatic string
 *
 * The arguments, return values, and usage of this function are the same as
 * inc_dynamic_str_set().  However, this function appends to the string instead
 * of setting a new value.
 */
INC_INLINE_API(
int __attribute__((format(printf, 3, 4))) inc_dynamic_str_append(
	struct inc_dynamic_str **buf, size_t max_len,
	const char *fmt, ...),
{
	int res;
	va_list ap;
	
	va_start(ap, fmt);
	res = inc_dynamic_str_thread_append_va(buf, max_len, NULL, fmt, ap);
	va_end(ap);

	return res;
}
)

void threadstorage_init(void);

#endif /* _INC_CC_THREADSTORAGE_H */
