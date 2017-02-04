/*! \file
 * \brief General channel locking definitions.
 *
 * - See \ref LockDef
 */

/*! \page LockDef thread locking models
 *
 * This file provides different implementation of the functions,
 * depending on the platform, the use of DEBUG_THREADS, and the way
 * module-level mutexes are initialized.
 *
 *  - \b static: the mutex is assigned the value INC_MUTEX_INIT_VALUE
 *        this is done at compile time, and is the way used on Linux.
 *        This method is not applicable to all platforms e.g. when the
 *        initialization needs that some code is run.
 *
 *  - \b through constructors: for each mutex, a constructor function is
 *        defined, which then runs when the program (or the module)
 *        starts. The problem with this approach is that there is a
 *        lot of code duplication (a new block of code is created for
 *        each mutex). Also, it does not prevent a user from declaring
 *        a global mutex without going through the wrapper macros,
 *        so sane programming practices are still required.
 */

#ifndef _INC_CC_LOCK_H
#define _INC_CC_LOCK_H

#include <pthread.h>
#include <netdb.h>
#include <time.h>
#include <sys/param.h>
#include <unistd.h>

#ifndef HAVE_PTHREAD_RWLOCK_TIMEDWRLOCK
#include "common/time.h"
#endif
#include "common/logger.h"

/* internal macro to profile mutexes. Only computes the delay on
 * non-blocking calls.
 */
#ifndef	HAVE_MTX_PROFILE
#define	__MTX_PROF(a)	return pthread_mutex_lock((a))
#else
#define	__MTX_PROF(a)	do {			\
	int i;					\
	/* profile only non-blocking events */	\
	inc_mark(mtx_prof, 1);			\
	i = pthread_mutex_trylock((a));		\
	inc_mark(mtx_prof, 0);			\
	if (!i)					\
		return i;			\
	else					\
		return pthread_mutex_lock((a)); \
	} while (0)
#endif	/* HAVE_MTX_PROFILE */

#define INC_PTHREADT_NULL (pthread_t) -1
#define INC_PTHREADT_STOP (pthread_t) -2

#if defined(SOLARIS) || defined(BSD)
#define INC_MUTEX_INIT_W_CONSTRUCTORS
#endif /* SOLARIS || BSD */

/* INC_CC REQUIRES recursive (not error checking) mutexes
   and will not run without them. */
#if defined(PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP)
#define PTHREAD_MUTEX_INIT_VALUE	PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#define INC_MUTEX_KIND			PTHREAD_MUTEX_RECURSIVE_NP
#else
#define PTHREAD_MUTEX_INIT_VALUE	PTHREAD_MUTEX_INITIALIZER
#define INC_MUTEX_KIND			PTHREAD_MUTEX_RECURSIVE_NP
#endif /* PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP */

#ifdef DEBUG_THREADS

#define __inc_mutex_logger(...)  do { if (canlog) inc_log(LOG_ERROR, __VA_ARGS__); else fprintf(stderr, __VA_ARGS__); } while (0)

#ifdef THREAD_CRASH
#define DO_THREAD_CRASH do { *((int *)(0)) = 1; } while(0)
#else
#define DO_THREAD_CRASH do { } while (0)
#endif

#include <errno.h>
#include <string.h>
#include <stdio.h>

#define INC_MUTEX_INIT_VALUE { PTHREAD_MUTEX_INIT_VALUE, 1, { NULL }, { 0 }, 0, { NULL }, { 0 }, PTHREAD_MUTEX_INIT_VALUE }
#define INC_MUTEX_INIT_VALUE_NOTRACKING \
                             { PTHREAD_MUTEX_INIT_VALUE, 0, { NULL }, { 0 }, 0, { NULL }, { 0 }, PTHREAD_MUTEX_INIT_VALUE }

#define INC_MAX_REENTRANCY 10

struct inc_mutex_info {
	pthread_mutex_t mutex;
	/*! Track which thread holds this lock */
	unsigned int track:1;
	const char *file[INC_MAX_REENTRANCY];
	int lineno[INC_MAX_REENTRANCY];
	int reentrancy;
	const char *func[INC_MAX_REENTRANCY];
	pthread_t thread[INC_MAX_REENTRANCY];
	pthread_mutex_t reentr_mutex;
};

typedef struct inc_mutex_info inc_mutex_t;

typedef pthread_cond_t inc_cond_t;

static pthread_mutex_t empty_mutex;

enum inc_lock_type {
	INC_MUTEX,
	INC_RDLOCK,
	INC_WRLOCK,
};

/*!
 * \brief Store lock info for the current thread
 *
 * This function gets called in inc_mutex_lock() and inc_mutex_trylock() so
 * that information about this lock can be stored in this thread's
 * lock info struct.  The lock is marked as pending as the thread is waiting
 * on the lock.  inc_mark_lock_acquired() will mark it as held by this thread.
 */
#if !defined(LOW_MEMORY)
void inc_store_lock_info(enum inc_lock_type type, const char *filename,
	int line_num, const char *func, const char *lock_name, void *lock_addr);
#else
#define inc_store_lock_info(I,DONT,CARE,ABOUT,THE,PARAMETERS)
#endif


/*!
 * \brief Mark the last lock as acquired
 */
#if !defined(LOW_MEMORY)
void inc_mark_lock_acquired(void *lock_addr);
#else
#define inc_mark_lock_acquired(ignore)
#endif

/*!
 * \brief Mark the last lock as failed (trylock)
 */
#if !defined(LOW_MEMORY)
void inc_mark_lock_failed(void *lock_addr);
#else
#define inc_mark_lock_failed(ignore)
#endif

/*!
 * \brief remove lock info for the current thread
 *
 * this gets called by inc_mutex_unlock so that information on the lock can
 * be removed from the current thread's lock info struct.
 */
#if !defined(LOW_MEMORY)
void inc_remove_lock_info(void *lock_addr);
#else
#define inc_remove_lock_info(ignore)
#endif

/*!
 * \brief retrieve lock info for the specified mutex
 *
 * this gets called during deadlock avoidance, so that the information may
 * be preserved as to what location originally acquired the lock.
 */
#if !defined(LOW_MEMORY)
int inc_find_lock_info(void *lock_addr, char *filename, size_t filename_size, int *lineno, char *func, size_t func_size, char *mutex_name, size_t mutex_name_size);
#else
#define inc_find_lock_info(a,b,c,d,e,f,g,h) -1
#endif

/*!
 * \brief Unlock a lock briefly
 *
 * used during deadlock avoidance, to preserve the original location where
 * a lock was originally acquired.
 */
#define DEADLOCK_AVOIDANCE(lock) \
	do { \
		char __filename[80], __func[80], __mutex_name[80]; \
		int __lineno; \
		int __res = inc_find_lock_info(lock, __filename, sizeof(__filename), &__lineno, __func, sizeof(__func), __mutex_name, sizeof(__mutex_name)); \
		inc_mutex_unlock(lock); \
		usleep(1); \
		if (__res < 0) { /* Shouldn't ever happen, but just in case... */ \
			inc_mutex_lock(lock); \
		} else { \
			__inc_pthread_mutex_lock(__filename, __lineno, __func, __mutex_name, lock); \
		} \
	} while (0)

static void __attribute__((constructor)) init_empty_mutex(void)
{
	memset(&empty_mutex, 0, sizeof(empty_mutex));
}

static inline void inc_reentrancy_lock(inc_mutex_t *p_inc_mutex)
{
	pthread_mutex_lock(&p_inc_mutex->reentr_mutex);
}

static inline void inc_reentrancy_unlock(inc_mutex_t *p_inc_mutex)
{
	pthread_mutex_unlock(&p_inc_mutex->reentr_mutex);
}

static inline void inc_reentrancy_init(inc_mutex_t *p_inc_mutex)
{
	int i;
	pthread_mutexattr_t reentr_attr;

	for (i = 0; i < INC_MAX_REENTRANCY; i++) {
		p_inc_mutex->file[i] = NULL;
		p_inc_mutex->lineno[i] = 0;
		p_inc_mutex->func[i] = NULL;
		p_inc_mutex->thread[i] = 0;
	}

	p_inc_mutex->reentrancy = 0;

	pthread_mutexattr_init(&reentr_attr);
	pthread_mutexattr_settype(&reentr_attr, INC_MUTEX_KIND);
	pthread_mutex_init(&p_inc_mutex->reentr_mutex, &reentr_attr);
	pthread_mutexattr_destroy(&reentr_attr);
}

static inline void delete_reentrancy_cs(inc_mutex_t * p_inc_mutex)
{
	pthread_mutex_destroy(&p_inc_mutex->reentr_mutex);
}

static inline int __inc_pthread_mutex_init(int track, const char *filename, int lineno, const char *func,
						const char *mutex_name, inc_mutex_t *t) 
{
	int res;
	pthread_mutexattr_t  attr;

#ifdef INC_MUTEX_INIT_W_CONSTRUCTORS

	if ((t->mutex) != ((pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER)) {
/*
		int canlog = strcmp(filename, "logger.c") & track;
		__inc_mutex_logger("%s line %d (%s): NOTICE: mutex '%s' is already initialized.\n",
				   filename, lineno, func, mutex_name);
		DO_THREAD_CRASH;
*/
		return 0;
	}

#endif /* INC_MUTEX_INIT_W_CONSTRUCTORS */

	inc_reentrancy_init(t);
	t->track = track;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, INC_MUTEX_KIND);

	res = pthread_mutex_init(&t->mutex, &attr);
	pthread_mutexattr_destroy(&attr);
	return res;
}

#define inc_mutex_init(pmutex) __inc_pthread_mutex_init(1, __FILE__, __LINE__, __PRETTY_FUNCTION__, #pmutex, pmutex)
#define inc_mutex_init_notracking(pmutex) \
	__inc_pthread_mutex_init(0, __FILE__, __LINE__, __PRETTY_FUNCTION__, #pmutex, pmutex)

#define	ROFFSET	((t->reentrancy > 0) ? (t->reentrancy-1) : 0)
static inline int __inc_pthread_mutex_destroy(const char *filename, int lineno, const char *func,
						const char *mutex_name, inc_mutex_t *t)
{
	int res;
	int canlog = strcmp(filename, "logger.c") & t->track;

#ifdef INC_MUTEX_INIT_W_CONSTRUCTORS
	if ((t->mutex) == ((pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER)) {
		/* Don't try to uninitialize non initialized mutex
		 * This may no effect on linux
		 * And always ganerate core on *BSD with 
		 * linked libpthread
		 * This not error condition if the mutex created on the fly.
		 */
		__inc_mutex_logger("%s line %d (%s): NOTICE: mutex '%s' is uninitialized.\n",
				   filename, lineno, func, mutex_name);
		return 0;
	}
#endif

	res = pthread_mutex_trylock(&t->mutex);
	switch (res) {
	case 0:
		pthread_mutex_unlock(&t->mutex);
		break;
	case EINVAL:
		__inc_mutex_logger("%s line %d (%s): Error: attempt to destroy invalid mutex '%s'.\n",
				  filename, lineno, func, mutex_name);
		break;
	case EBUSY:
		__inc_mutex_logger("%s line %d (%s): Error: attempt to destroy locked mutex '%s'.\n",
				   filename, lineno, func, mutex_name);
		inc_reentrancy_lock(t);
		__inc_mutex_logger("%s line %d (%s): Error: '%s' was locked here.\n",
			    t->file[ROFFSET], t->lineno[ROFFSET], t->func[ROFFSET], mutex_name);
		inc_reentrancy_unlock(t);
		break;
	}

	if ((res = pthread_mutex_destroy(&t->mutex)))
		__inc_mutex_logger("%s line %d (%s): Error destroying mutex %s: %s\n",
				   filename, lineno, func, mutex_name, strerror(res));
#ifndef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
	else
		t->mutex = PTHREAD_MUTEX_INIT_VALUE;
#endif
	inc_reentrancy_lock(t);
	t->file[0] = filename;
	t->lineno[0] = lineno;
	t->func[0] = func;
	t->reentrancy = 0;
	t->thread[0] = 0;
	inc_reentrancy_unlock(t);
	delete_reentrancy_cs(t);

	return res;
}

static inline int __inc_pthread_mutex_lock(const char *filename, int lineno, const char *func,
                                           const char* mutex_name, inc_mutex_t *t)
{
	int res;
	int canlog = strcmp(filename, "logger.c") & t->track;

#if defined(INC_MUTEX_INIT_W_CONSTRUCTORS)
	if ((t->mutex) == ((pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER)) {
		/* Don't warn abount uninitialized mutex.
		 * Simple try to initialize it.
		 * May be not needed in linux system.
		 */
		res = __inc_pthread_mutex_init(t->track, filename, lineno, func, mutex_name, t);
		if ((t->mutex) == ((pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER)) {
			__inc_mutex_logger("%s line %d (%s): Error: mutex '%s' is uninitialized and unable to initialize.\n",
					 filename, lineno, func, mutex_name);
			return res;
		}		
	}
#endif /* INC_MUTEX_INIT_W_CONSTRUCTORS */

	if (t->track)
		inc_store_lock_info(INC_MUTEX, filename, lineno, func, mutex_name, &t->mutex);

#ifdef DETECT_DEADLOCKS
	{
		time_t seconds = time(NULL);
		time_t wait_time, reported_wait = 0;
		do {
#ifdef	HAVE_MTX_PROFILE
			inc_mark(mtx_prof, 1);
#endif
			res = pthread_mutex_trylock(&t->mutex);
#ifdef	HAVE_MTX_PROFILE
			inc_mark(mtx_prof, 0);
#endif
			if (res == EBUSY) {
				wait_time = time(NULL) - seconds;
				if (wait_time > reported_wait && (wait_time % 5) == 0) {
					__inc_mutex_logger("%s line %d (%s): Deadlock? waited %d sec for mutex '%s'?\n",
							   filename, lineno, func, (int) wait_time, mutex_name);
					inc_reentrancy_lock(t);
					__inc_mutex_logger("%s line %d (%s): '%s' was locked here.\n",
							   t->file[ROFFSET], t->lineno[ROFFSET],
							   t->func[ROFFSET], mutex_name);
					inc_reentrancy_unlock(t);
					reported_wait = wait_time;
				}
				usleep(200);
			}
		} while (res == EBUSY);
	}
#else
#ifdef	HAVE_MTX_PROFILE
	inc_mark(mtx_prof, 1);
	res = pthread_mutex_trylock(&t->mutex);
	inc_mark(mtx_prof, 0);
	if (res)
#endif
	res = pthread_mutex_lock(&t->mutex);
#endif /* DETECT_DEADLOCKS */

	if (!res) {
		inc_reentrancy_lock(t);
		if (t->reentrancy < INC_MAX_REENTRANCY) {
			t->file[t->reentrancy] = filename;
			t->lineno[t->reentrancy] = lineno;
			t->func[t->reentrancy] = func;
			t->thread[t->reentrancy] = pthread_self();
			t->reentrancy++;
		} else {
			__inc_mutex_logger("%s line %d (%s): '%s' really deep reentrancy!\n",
							   filename, lineno, func, mutex_name);
		}
		inc_reentrancy_unlock(t);
		if (t->track)
			inc_mark_lock_acquired(&t->mutex);
	} else {
		if (t->track)
			inc_remove_lock_info(&t->mutex);
		__inc_mutex_logger("%s line %d (%s): Error obtaining mutex: %s\n",
				   filename, lineno, func, strerror(res));
		DO_THREAD_CRASH;
	}

	return res;
}

static inline int __inc_pthread_mutex_trylock(const char *filename, int lineno, const char *func,
                                              const char* mutex_name, inc_mutex_t *t)
{
	int res;
	int canlog = strcmp(filename, "logger.c") & t->track;

#if defined(INC_MUTEX_INIT_W_CONSTRUCTORS)
	if ((t->mutex) == ((pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER)) {
		/* Don't warn abount uninitialized mutex.
		 * Simple try to initialize it.
		 * May be not needed in linux system.
		 */
		res = __inc_pthread_mutex_init(t->track, filename, lineno, func, mutex_name, t);
		if ((t->mutex) == ((pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER)) {
			__inc_mutex_logger("%s line %d (%s): Error: mutex '%s' is uninitialized and unable to initialize.\n",
					 filename, lineno, func, mutex_name);
			return res;
		}		
	}
#endif /* INC_MUTEX_INIT_W_CONSTRUCTORS */

	if (t->track)
		inc_store_lock_info(INC_MUTEX, filename, lineno, func, mutex_name, &t->mutex);

	if (!(res = pthread_mutex_trylock(&t->mutex))) {
		inc_reentrancy_lock(t);
		if (t->reentrancy < INC_MAX_REENTRANCY) {
			t->file[t->reentrancy] = filename;
			t->lineno[t->reentrancy] = lineno;
			t->func[t->reentrancy] = func;
			t->thread[t->reentrancy] = pthread_self();
			t->reentrancy++;
		} else {
			__inc_mutex_logger("%s line %d (%s): '%s' really deep reentrancy!\n",
					   filename, lineno, func, mutex_name);
		}
		inc_reentrancy_unlock(t);
		if (t->track)
			inc_mark_lock_acquired(&t->mutex);
	} else if (t->track) {
		inc_mark_lock_failed(&t->mutex);
	}

	return res;
}

static inline int __inc_pthread_mutex_unlock(const char *filename, int lineno, const char *func,
					     const char *mutex_name, inc_mutex_t *t)
{
	int res;
	int canlog = strcmp(filename, "logger.c") & t->track;

#ifdef INC_MUTEX_INIT_W_CONSTRUCTORS
	if ((t->mutex) == ((pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER)) {
		__inc_mutex_logger("%s line %d (%s): Error: mutex '%s' is uninitialized.\n",
				   filename, lineno, func, mutex_name);
		res = __inc_pthread_mutex_init(t->track, filename, lineno, func, mutex_name, t);
		if ((t->mutex) == ((pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER)) {
			__inc_mutex_logger("%s line %d (%s): Error: mutex '%s' is uninitialized and unable to initialize.\n",
					 filename, lineno, func, mutex_name);
		}
		return res;
	}
#endif /* INC_MUTEX_INIT_W_CONSTRUCTORS */

	inc_reentrancy_lock(t);
	if (t->reentrancy && (t->thread[ROFFSET] != pthread_self())) {
		__inc_mutex_logger("%s line %d (%s): attempted unlock mutex '%s' without owning it!\n",
				   filename, lineno, func, mutex_name);
		__inc_mutex_logger("%s line %d (%s): '%s' was locked here.\n",
				   t->file[ROFFSET], t->lineno[ROFFSET], t->func[ROFFSET], mutex_name);
		DO_THREAD_CRASH;
	}

	if (--t->reentrancy < 0) {
		__inc_mutex_logger("%s line %d (%s): mutex '%s' freed more times than we've locked!\n",
				   filename, lineno, func, mutex_name);
		t->reentrancy = 0;
	}

	if (t->reentrancy < INC_MAX_REENTRANCY) {
		t->file[t->reentrancy] = NULL;
		t->lineno[t->reentrancy] = 0;
		t->func[t->reentrancy] = NULL;
		t->thread[t->reentrancy] = 0;
	}
	inc_reentrancy_unlock(t);

	if (t->track)
		inc_remove_lock_info(&t->mutex);

	if ((res = pthread_mutex_unlock(&t->mutex))) {
		__inc_mutex_logger("%s line %d (%s): Error releasing mutex: %s\n", 
				   filename, lineno, func, strerror(res));
		DO_THREAD_CRASH;
	}

	return res;
}

static inline int __inc_cond_init(const char *filename, int lineno, const char *func,
				  const char *cond_name, inc_cond_t *cond, pthread_condattr_t *cond_attr)
{
	return pthread_cond_init(cond, cond_attr);
}

static inline int __inc_cond_signal(const char *filename, int lineno, const char *func,
				    const char *cond_name, inc_cond_t *cond)
{
	return pthread_cond_signal(cond);
}

static inline int __inc_cond_broadcast(const char *filename, int lineno, const char *func,
				       const char *cond_name, inc_cond_t *cond)
{
	return pthread_cond_broadcast(cond);
}

static inline int __inc_cond_destroy(const char *filename, int lineno, const char *func,
				     const char *cond_name, inc_cond_t *cond)
{
	return pthread_cond_destroy(cond);
}

static inline int __inc_cond_wait(const char *filename, int lineno, const char *func,
				  const char *cond_name, const char *mutex_name,
				  inc_cond_t *cond, inc_mutex_t *t)
{
	int res;
	int canlog = strcmp(filename, "logger.c") & t->track;

#ifdef INC_MUTEX_INIT_W_CONSTRUCTORS
	if ((t->mutex) == ((pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER)) {
		__inc_mutex_logger("%s line %d (%s): Error: mutex '%s' is uninitialized.\n",
				   filename, lineno, func, mutex_name);
		res = __inc_pthread_mutex_init(t->track, filename, lineno, func, mutex_name, t);
		if ((t->mutex) == ((pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER)) {
			__inc_mutex_logger("%s line %d (%s): Error: mutex '%s' is uninitialized and unable to initialize.\n",
					 filename, lineno, func, mutex_name);
		}
		return res;
	}
#endif /* INC_MUTEX_INIT_W_CONSTRUCTORS */

	inc_reentrancy_lock(t);
	if (t->reentrancy && (t->thread[ROFFSET] != pthread_self())) {
		__inc_mutex_logger("%s line %d (%s): attempted unlock mutex '%s' without owning it!\n",
				   filename, lineno, func, mutex_name);
		__inc_mutex_logger("%s line %d (%s): '%s' was locked here.\n",
				   t->file[ROFFSET], t->lineno[ROFFSET], t->func[ROFFSET], mutex_name);
		DO_THREAD_CRASH;
	}

	if (--t->reentrancy < 0) {
		__inc_mutex_logger("%s line %d (%s): mutex '%s' freed more times than we've locked!\n",
				   filename, lineno, func, mutex_name);
		t->reentrancy = 0;
	}

	if (t->reentrancy < INC_MAX_REENTRANCY) {
		t->file[t->reentrancy] = NULL;
		t->lineno[t->reentrancy] = 0;
		t->func[t->reentrancy] = NULL;
		t->thread[t->reentrancy] = 0;
	}
	inc_reentrancy_unlock(t);

	if (t->track)
		inc_remove_lock_info(&t->mutex);

	if ((res = pthread_cond_wait(cond, &t->mutex))) {
		__inc_mutex_logger("%s line %d (%s): Error waiting on condition mutex '%s'\n", 
				   filename, lineno, func, strerror(res));
		DO_THREAD_CRASH;
	} else {
		inc_reentrancy_lock(t);
		if (t->reentrancy < INC_MAX_REENTRANCY) {
			t->file[t->reentrancy] = filename;
			t->lineno[t->reentrancy] = lineno;
			t->func[t->reentrancy] = func;
			t->thread[t->reentrancy] = pthread_self();
			t->reentrancy++;
		} else {
			__inc_mutex_logger("%s line %d (%s): '%s' really deep reentrancy!\n",
							   filename, lineno, func, mutex_name);
		}
		inc_reentrancy_unlock(t);

		if (t->track)
			inc_store_lock_info(INC_MUTEX, filename, lineno, func, mutex_name, &t->mutex);
	}

	return res;
}

static inline int __inc_cond_timedwait(const char *filename, int lineno, const char *func,
				       const char *cond_name, const char *mutex_name, inc_cond_t *cond,
				       inc_mutex_t *t, const struct timespec *abstime)
{
	int res;
	int canlog = strcmp(filename, "logger.c") & t->track;

#ifdef INC_MUTEX_INIT_W_CONSTRUCTORS
	if ((t->mutex) == ((pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER)) {
		__inc_mutex_logger("%s line %d (%s): Error: mutex '%s' is uninitialized.\n",
				   filename, lineno, func, mutex_name);
		res = __inc_pthread_mutex_init(t->track, filename, lineno, func, mutex_name, t);
		if ((t->mutex) == ((pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER)) {
			__inc_mutex_logger("%s line %d (%s): Error: mutex '%s' is uninitialized and unable to initialize.\n",
					 filename, lineno, func, mutex_name);
		}
		return res;
	}
#endif /* INC_MUTEX_INIT_W_CONSTRUCTORS */

	inc_reentrancy_lock(t);
	if (t->reentrancy && (t->thread[ROFFSET] != pthread_self())) {
		__inc_mutex_logger("%s line %d (%s): attempted unlock mutex '%s' without owning it!\n",
				   filename, lineno, func, mutex_name);
		__inc_mutex_logger("%s line %d (%s): '%s' was locked here.\n",
				   t->file[ROFFSET], t->lineno[ROFFSET], t->func[ROFFSET], mutex_name);
		DO_THREAD_CRASH;
	}

	if (--t->reentrancy < 0) {
		__inc_mutex_logger("%s line %d (%s): mutex '%s' freed more times than we've locked!\n",
				   filename, lineno, func, mutex_name);
		t->reentrancy = 0;
	}

	if (t->reentrancy < INC_MAX_REENTRANCY) {
		t->file[t->reentrancy] = NULL;
		t->lineno[t->reentrancy] = 0;
		t->func[t->reentrancy] = NULL;
		t->thread[t->reentrancy] = 0;
	}
	inc_reentrancy_unlock(t);

	if (t->track)
		inc_remove_lock_info(&t->mutex);

	if ((res = pthread_cond_timedwait(cond, &t->mutex, abstime)) && (res != ETIMEDOUT)) {
		__inc_mutex_logger("%s line %d (%s): Error waiting on condition mutex '%s'\n", 
				   filename, lineno, func, strerror(res));
		DO_THREAD_CRASH;
	} else {
		inc_reentrancy_lock(t);
		if (t->reentrancy < INC_MAX_REENTRANCY) {
			t->file[t->reentrancy] = filename;
			t->lineno[t->reentrancy] = lineno;
			t->func[t->reentrancy] = func;
			t->thread[t->reentrancy] = pthread_self();
			t->reentrancy++;
		} else {
			__inc_mutex_logger("%s line %d (%s): '%s' really deep reentrancy!\n",
							   filename, lineno, func, mutex_name);
		}
		inc_reentrancy_unlock(t);

		if (t->track)
			inc_store_lock_info(INC_MUTEX, filename, lineno, func, mutex_name, &t->mutex);
	}

	return res;
}

#define inc_mutex_destroy(a) __inc_pthread_mutex_destroy(__FILE__, __LINE__, __PRETTY_FUNCTION__, #a, a)
#define inc_mutex_lock(a) __inc_pthread_mutex_lock(__FILE__, __LINE__, __PRETTY_FUNCTION__, #a, a)
#define inc_mutex_unlock(a) __inc_pthread_mutex_unlock(__FILE__, __LINE__, __PRETTY_FUNCTION__, #a, a)
#define inc_mutex_trylock(a) __inc_pthread_mutex_trylock(__FILE__, __LINE__, __PRETTY_FUNCTION__, #a, a)
#define inc_cond_init(cond, attr) __inc_cond_init(__FILE__, __LINE__, __PRETTY_FUNCTION__, #cond, cond, attr)
#define inc_cond_destroy(cond) __inc_cond_destroy(__FILE__, __LINE__, __PRETTY_FUNCTION__, #cond, cond)
#define inc_cond_signal(cond) __inc_cond_signal(__FILE__, __LINE__, __PRETTY_FUNCTION__, #cond, cond)
#define inc_cond_broadcast(cond) __inc_cond_broadcast(__FILE__, __LINE__, __PRETTY_FUNCTION__, #cond, cond)
#define inc_cond_wait(cond, mutex) __inc_cond_wait(__FILE__, __LINE__, __PRETTY_FUNCTION__, #cond, #mutex, cond, mutex)
#define inc_cond_timedwait(cond, mutex, time) __inc_cond_timedwait(__FILE__, __LINE__, __PRETTY_FUNCTION__, #cond, #mutex, cond, mutex, time)

#else /* !DEBUG_THREADS */

#define	DEADLOCK_AVOIDANCE(lock) \
	inc_mutex_unlock(lock); \
	usleep(1); \
	inc_mutex_lock(lock);


typedef pthread_mutex_t inc_mutex_t;

#define INC_MUTEX_INIT_VALUE	((inc_mutex_t) PTHREAD_MUTEX_INIT_VALUE)
#define INC_MUTEX_INIT_VALUE_NOTRACKING \
	((inc_mutex_t) PTHREAD_MUTEX_INIT_VALUE)

#define inc_mutex_init_notracking(m) inc_mutex_init(m)

static inline int inc_mutex_init(inc_mutex_t *pmutex)
{
	int res;
	pthread_mutexattr_t attr;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, INC_MUTEX_KIND);

	res = pthread_mutex_init(pmutex, &attr);
	pthread_mutexattr_destroy(&attr);
	return res;
}

#define inc_pthread_mutex_init(pmutex,a) pthread_mutex_init(pmutex,a)

static inline int inc_mutex_unlock(inc_mutex_t *pmutex)
{
	return pthread_mutex_unlock(pmutex);
}

static inline int inc_mutex_destroy(inc_mutex_t *pmutex)
{
	return pthread_mutex_destroy(pmutex);
}

static inline int inc_mutex_lock(inc_mutex_t *pmutex)
{
	__MTX_PROF(pmutex);
}

static inline int inc_mutex_trylock(inc_mutex_t *pmutex)
{
	return pthread_mutex_trylock(pmutex);
}

typedef pthread_cond_t inc_cond_t;

static inline int inc_cond_init(inc_cond_t *cond, pthread_condattr_t *cond_attr)
{
	return pthread_cond_init(cond, cond_attr);
}

static inline int inc_cond_signal(inc_cond_t *cond)
{
	return pthread_cond_signal(cond);
}

static inline int inc_cond_broadcast(inc_cond_t *cond)
{
	return pthread_cond_broadcast(cond);
}

static inline int inc_cond_destroy(inc_cond_t *cond)
{
	return pthread_cond_destroy(cond);
}

static inline int inc_cond_wait(inc_cond_t *cond, inc_mutex_t *t)
{
	return pthread_cond_wait(cond, t);
}

static inline int inc_cond_timedwait(inc_cond_t *cond, inc_mutex_t *t, const struct timespec *abstime)
{
	return pthread_cond_timedwait(cond, t, abstime);
}

#endif /* !DEBUG_THREADS */

#if defined(INC_MUTEX_INIT_W_CONSTRUCTORS)
/* If INC_MUTEX_INIT_W_CONSTRUCTORS is defined, use file scope
 destructors to destroy mutexes and create it on the fly.  */
#define __INC_MUTEX_DEFINE(scope, mutex, init_val, track) \
	scope inc_mutex_t mutex = init_val; \
static void  __attribute__((constructor)) init_##mutex(void) \
{ \
	if (track) \
		inc_mutex_init(&mutex); \
	else \
		inc_mutex_init_notracking(&mutex); \
} \
static void  __attribute__((destructor)) fini_##mutex(void) \
{ \
	inc_mutex_destroy(&mutex); \
}
#else /* !INC_MUTEX_INIT_W_CONSTRUCTORS */
/* By default, use static initialization of mutexes. */ 
#define __INC_MUTEX_DEFINE(scope, mutex, init_val, track) \
	scope inc_mutex_t mutex = init_val
#endif /* INC_MUTEX_INIT_W_CONSTRUCTORS */

#define pthread_mutex_t use_inc_mutex_t_instead_of_pthread_mutex_t
#define pthread_mutex_lock use_inc_mutex_lock_instead_of_pthread_mutex_lock
#define pthread_mutex_unlock use_inc_mutex_unlock_instead_of_pthread_mutex_unlock
#define pthread_mutex_trylock use_inc_mutex_trylock_instead_of_pthread_mutex_trylock
#define pthread_mutex_init use_inc_mutex_init_instead_of_pthread_mutex_init
#define pthread_mutex_destroy use_inc_mutex_destroy_instead_of_pthread_mutex_destroy
#define pthread_cond_t use_inc_cond_t_instead_of_pthread_cond_t
#define pthread_cond_init use_inc_cond_init_instead_of_pthread_cond_init
#define pthread_cond_destroy use_inc_cond_destroy_instead_of_pthread_cond_destroy
#define pthread_cond_signal use_inc_cond_signal_instead_of_pthread_cond_signal
#define pthread_cond_broadcast use_inc_cond_broadcinc_instead_of_pthread_cond_broadcast
#define pthread_cond_wait use_inc_cond_wait_instead_of_pthread_cond_wait
#define pthread_cond_timedwait use_inc_cond_timedwait_instead_of_pthread_cond_timedwait

#define INC_MUTEX_DEFINE_STATIC(mutex) __INC_MUTEX_DEFINE(static, mutex, INC_MUTEX_INIT_VALUE, 1)
#define INC_MUTEX_DEFINE_STATIC_NOTRACKING(mutex) __INC_MUTEX_DEFINE(static, mutex, INC_MUTEX_INIT_VALUE_NOTRACKING, 0)

#define INC_MUTEX_INITIALIZER __use_INC_MUTEX_DEFINE_STATIC_rather_than_INC_MUTEX_INITIALIZER__

#define gethostbyname __gethostbyname__is__not__reentrant__use__inc_gethostbyname__instead__

#ifndef __linux__
#define pthread_create __use_inc_pthread_create_instead__
#endif

typedef pthread_rwlock_t inc_rwlock_t;

#ifdef HAVE_PTHREAD_RWLOCK_INITIALIZER
#define INC_RWLOCK_INIT_VALUE PTHREAD_RWLOCK_INITIALIZER
#else
#define INC_RWLOCK_INIT_VALUE NULL
#endif

#ifdef DEBUG_THREADS

#define inc_rwlock_init(rwlock) __inc_rwlock_init(__FILE__, __LINE__, __PRETTY_FUNCTION__, #rwlock, rwlock)


static inline int __inc_rwlock_init(const char *filename, int lineno, const char *func, const char *rwlock_name, inc_rwlock_t *prwlock)
{
	int res;
	pthread_rwlockattr_t attr;
#ifdef INC_MUTEX_INIT_W_CONSTRUCTORS
        int canlog = strcmp(filename, "logger.c");

        if (*prwlock != ((inc_rwlock_t) INC_RWLOCK_INIT_VALUE)) {
		__inc_mutex_logger("%s line %d (%s): Warning: rwlock '%s' is already initialized.\n",
				filename, lineno, func, rwlock_name);
		return 0;
	}
#endif /* INC_MUTEX_INIT_W_CONSTRUCTORS */
	pthread_rwlockattr_init(&attr);

#ifdef HAVE_PTHREAD_RWLOCK_PREFER_WRITER_NP
	pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NP);
#endif

	res = pthread_rwlock_init(prwlock, &attr);
	pthread_rwlockattr_destroy(&attr);
	return res;
}

#define inc_rwlock_destroy(rwlock) __inc_rwlock_destroy(__FILE__, __LINE__, __PRETTY_FUNCTION__, #rwlock, rwlock)

static inline int __inc_rwlock_destroy(const char *filename, int lineno, const char *func, const char *rwlock_name, inc_rwlock_t *prwlock)
{
	int res;
	int canlog = strcmp(filename, "logger.c");

#ifdef INC_MUTEX_INIT_W_CONSTRUCTORS
	if (*prwlock == ((inc_rwlock_t) INC_RWLOCK_INIT_VALUE)) {
		__inc_mutex_logger("%s line %d (%s): Warning: rwlock '%s' is uninitialized.\n",
				   filename, lineno, func, rwlock_name);
		return 0;
	}
#endif /* INC_MUTEX_INIT_W_CONSTRUCTORS */
	
	if ((res = pthread_rwlock_destroy(prwlock)))
		__inc_mutex_logger("%s line %d (%s): Error destroying rwlock %s: %s\n",
				filename, lineno, func, rwlock_name, strerror(res));

	return res;
}

#define inc_rwlock_unlock(a) \
	_inc_rwlock_unlock(a, # a, __FILE__, __LINE__, __PRETTY_FUNCTION__)

static inline int _inc_rwlock_unlock(inc_rwlock_t *lock, const char *name,
	const char *file, int line, const char *func)
{
	int res;
#ifdef INC_MUTEX_INIT_W_CONSTRUCTORS
	int canlog = strcmp(file, "logger.c");

	if (*lock == ((inc_rwlock_t) INC_RWLOCK_INIT_VALUE)) {
		__inc_mutex_logger("%s line %d (%s): Warning: rwlock '%s' is uninitialized.\n",
				   file, line, func, name);
		res = __inc_rwlock_init(file, line, func, name, lock);
		if (*lock == ((inc_rwlock_t) INC_RWLOCK_INIT_VALUE)) {
			__inc_mutex_logger("%s line %d (%s): Error: rwlock '%s' is uninitialized and unable to initialize.\n",
					file, line, func, name);
		}
		return res;
	}
#endif /* INC_MUTEX_INIT_W_CONSTRUCTORS */
	
	res = pthread_rwlock_unlock(lock);
	inc_remove_lock_info(lock);
	return res;
}

#define inc_rwlock_rdlock(a) \
	_inc_rwlock_rdlock(a, # a, __FILE__, __LINE__, __PRETTY_FUNCTION__)

static inline int _inc_rwlock_rdlock(inc_rwlock_t *lock, const char *name,
	const char *file, int line, const char *func)
{
	int res;
#ifdef INC_MUTEX_INIT_W_CONSTRUCTORS
	int canlog = strcmp(file, "logger.c");
	
	if (*lock == ((inc_rwlock_t) INC_RWLOCK_INIT_VALUE)) {
		 /* Don't warn abount uninitialized lock.
		  * Simple try to initialize it.
		  * May be not needed in linux system.
		  */
		res = __inc_rwlock_init(file, line, func, name, lock);
		if (*lock == ((inc_rwlock_t) INC_RWLOCK_INIT_VALUE)) {
			__inc_mutex_logger("%s line %d (%s): Error: rwlock '%s' is uninitialized and unable to initialize.\n",
					file, line, func, name);
			return res;
		}
	}
#endif /* INC_MUTEX_INIT_W_CONSTRUCTORS */
	
	inc_store_lock_info(INC_RDLOCK, file, line, func, name, lock);
	res = pthread_rwlock_rdlock(lock);
	if (!res)
		inc_mark_lock_acquired(lock);
	else
		inc_remove_lock_info(lock);
	return res;
}

#define inc_rwlock_wrlock(a) \
	_inc_rwlock_wrlock(a, # a, __FILE__, __LINE__, __PRETTY_FUNCTION__)

static inline int _inc_rwlock_wrlock(inc_rwlock_t *lock, const char *name,
	const char *file, int line, const char *func)
{
	int res;
#ifdef INC_MUTEX_INIT_W_CONSTRUCTORS
	int canlog = strcmp(file, "logger.c");
	
	if (*lock == ((inc_rwlock_t) INC_RWLOCK_INIT_VALUE)) {
		 /* Don't warn abount uninitialized lock.
		  * Simple try to initialize it.
		  * May be not needed in linux system.
		  */
		res = __inc_rwlock_init(file, line, func, name, lock);
		if (*lock == ((inc_rwlock_t) INC_RWLOCK_INIT_VALUE)) {
			__inc_mutex_logger("%s line %d (%s): Error: rwlock '%s' is uninitialized and unable to initialize.\n",
					file, line, func, name);
			return res;
		}
	}
#endif /* INC_MUTEX_INIT_W_CONSTRUCTORS */

	inc_store_lock_info(INC_WRLOCK, file, line, func, name, lock);
	res = pthread_rwlock_wrlock(lock);
	if (!res)
		inc_mark_lock_acquired(lock);
	else
		inc_remove_lock_info(lock);
	return res;
}

#define inc_rwlock_timedrdlock(a,b) \
	_inc_rwlock_timedrdlock(a, # a, b, __FILE__, __LINE__, __PRETTY_FUNCTION__)

static inline int _inc_rwlock_timedrdlock(inc_rwlock_t *lock, const char *name,
	const struct timespec *abs_timeout, const char *file, int line, const char *func)
{
	int res;
#ifdef INC_MUTEX_INIT_W_CONSTRUCTORS
	int canlog = strcmp(file, "logger.c");
	
	if (*lock == ((inc_rwlock_t) INC_RWLOCK_INIT_VALUE)) {
		 /* Don't warn abount uninitialized lock.
		  * Simple try to initialize it.
		  * May be not needed in linux system.
		  */
		res = __inc_rwlock_init(file, line, func, name, lock);
		if (*lock == ((inc_rwlock_t) INC_RWLOCK_INIT_VALUE)) {
			__inc_mutex_logger("%s line %d (%s): Error: rwlock '%s' is uninitialized and unable to initialize.\n",
					file, line, func, name);
			return res;
		}
	}
#endif /* INC_MUTEX_INIT_W_CONSTRUCTORS */
	
	inc_store_lock_info(INC_RDLOCK, file, line, func, name, lock);
#ifdef HAVE_PTHREAD_RWLOCK_TIMEDWRLOCK
	res = pthread_rwlock_timedrdlock(lock, abs_timeout);
#else
	do {
		struct timeval _start = inc_tvnow(), _diff;
		for (;;) {
			if (!(res = pthread_rwlock_tryrdlock(lock))) {
				break;
			}
			_diff = inc_tvsub(inc_tvnow(), _start);
			if (_diff.tv_sec > abs_timeout->tv_sec || (_diff.tv_sec == abs_timeout->tv_sec && _diff.tv_usec * 1000 > abs_timeout->tv_nsec)) {
				break;
			}
			usleep(1);
		}
	} while (0);
#endif
	if (!res)
		inc_mark_lock_acquired(lock);
	else
		inc_remove_lock_info(lock);
	return res;
}

#define inc_rwlock_timedwrlock(a,b) \
	_inc_rwlock_timedwrlock(a, # a, b, __FILE__, __LINE__, __PRETTY_FUNCTION__)

static inline int _inc_rwlock_timedwrlock(inc_rwlock_t *lock, const char *name,
	const struct timespec *abs_timeout, const char *file, int line, const char *func)
{
	int res;
#ifdef INC_MUTEX_INIT_W_CONSTRUCTORS
	int canlog = strcmp(file, "logger.c");
	
	if (*lock == ((inc_rwlock_t) INC_RWLOCK_INIT_VALUE)) {
		 /* Don't warn abount uninitialized lock.
		  * Simple try to initialize it.
		  * May be not needed in linux system.
		  */
		res = __inc_rwlock_init(file, line, func, name, lock);
		if (*lock == ((inc_rwlock_t) INC_RWLOCK_INIT_VALUE)) {
			__inc_mutex_logger("%s line %d (%s): Error: rwlock '%s' is uninitialized and unable to initialize.\n",
					file, line, func, name);
			return res;
		}
	}
#endif /* INC_MUTEX_INIT_W_CONSTRUCTORS */

	inc_store_lock_info(INC_WRLOCK, file, line, func, name, lock);
#ifdef HAVE_PTHREAD_RWLOCK_TIMEDWRLOCK
	res = pthread_rwlock_timedwrlock(lock, abs_timeout);
#else
	do {
		struct timeval _start = inc_tvnow(), _diff;
		for (;;) {
			if (!(res = pthread_rwlock_trywrlock(lock))) {
				break;
			}
			_diff = inc_tvsub(inc_tvnow(), _start);
			if (_diff.tv_sec > abs_timeout->tv_sec || (_diff.tv_sec == abs_timeout->tv_sec && _diff.tv_usec * 1000 > abs_timeout->tv_nsec)) {
				break;
			}
			usleep(1);
		}
	} while (0);
#endif
	if (!res)
		inc_mark_lock_acquired(lock);
	else
		inc_remove_lock_info(lock);
	return res;
}

#define inc_rwlock_tryrdlock(a) \
	_inc_rwlock_tryrdlock(a, # a, __FILE__, __LINE__, __PRETTY_FUNCTION__)

static inline int _inc_rwlock_tryrdlock(inc_rwlock_t *lock, const char *name,
	const char *file, int line, const char *func)
{
	int res;
#ifdef INC_MUTEX_INIT_W_CONSTRUCTORS
	int canlog = strcmp(file, "logger.c");
	
	if (*lock == ((inc_rwlock_t) INC_RWLOCK_INIT_VALUE)) {
		 /* Don't warn abount uninitialized lock.
		  * Simple try to initialize it.
		  * May be not needed in linux system.
		  */
		res = __inc_rwlock_init(file, line, func, name, lock);
		if (*lock == ((inc_rwlock_t) INC_RWLOCK_INIT_VALUE)) {
			__inc_mutex_logger("%s line %d (%s): Error: rwlock '%s' is uninitialized and unable to initialize.\n",
					file, line, func, name);
			return res;
		}
	}
#endif /* INC_MUTEX_INIT_W_CONSTRUCTORS */

	inc_store_lock_info(INC_RDLOCK, file, line, func, name, lock);
	res = pthread_rwlock_tryrdlock(lock);
	if (!res)
		inc_mark_lock_acquired(lock);
	else
		inc_remove_lock_info(lock);
	return res;
}

#define inc_rwlock_trywrlock(a) \
	_inc_rwlock_trywrlock(a, # a, __FILE__, __LINE__, __PRETTY_FUNCTION__)

static inline int _inc_rwlock_trywrlock(inc_rwlock_t *lock, const char *name,
	const char *file, int line, const char *func)
{
	int res;
#ifdef INC_MUTEX_INIT_W_CONSTRUCTORS
	int canlog = strcmp(file, "logger.c");
	
	if (*lock == ((inc_rwlock_t) INC_RWLOCK_INIT_VALUE)) {
		 /* Don't warn abount uninitialized lock.
		  * Simple try to initialize it.
		  * May be not needed in linux system.
		  */
		res = __inc_rwlock_init(file, line, func, name, lock);
		if (*lock == ((inc_rwlock_t) INC_RWLOCK_INIT_VALUE)) {
			__inc_mutex_logger("%s line %d (%s): Error: rwlock '%s' is uninitialized and unable to initialize.\n",
					file, line, func, name);
			return res;
		}
	}
#endif /* INC_MUTEX_INIT_W_CONSTRUCTORS */

	inc_store_lock_info(INC_WRLOCK, file, line, func, name, lock);
	res = pthread_rwlock_trywrlock(lock);
	if (!res)
		inc_mark_lock_acquired(lock);
	else
		inc_remove_lock_info(lock);
	return res;
}

#else /* !DEBUG_THREADS */

static inline int inc_rwlock_init(inc_rwlock_t *prwlock)
{
	int res;
	pthread_rwlockattr_t attr;

	pthread_rwlockattr_init(&attr);

#ifdef HAVE_PTHREAD_RWLOCK_PREFER_WRITER_NP
	pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NP);
#endif

	res = pthread_rwlock_init(prwlock, &attr);
	pthread_rwlockattr_destroy(&attr);
	return res;
}

static inline int inc_rwlock_destroy(inc_rwlock_t *prwlock)
{
	return pthread_rwlock_destroy(prwlock);
}

static inline int inc_rwlock_unlock(inc_rwlock_t *prwlock)
{
	return pthread_rwlock_unlock(prwlock);
}

static inline int inc_rwlock_rdlock(inc_rwlock_t *prwlock)
{
	return pthread_rwlock_rdlock(prwlock);
}

static inline int inc_rwlock_timedrdlock(inc_rwlock_t *prwlock, const struct timespec *abs_timeout)
{
	int res;
#ifdef HAVE_PTHREAD_RWLOCK_TIMEDWRLOCK
	res = pthread_rwlock_timedrdlock(prwlock, abs_timeout);
#else
	struct timeval _start = inc_tvnow(), _diff;
	for (;;) {
		if (!(res = pthread_rwlock_tryrdlock(prwlock))) {
			break;
		}
		_diff = inc_tvsub(inc_tvnow(), _start);
		if (_diff.tv_sec > abs_timeout->tv_sec || (_diff.tv_sec == abs_timeout->tv_sec && _diff.tv_usec * 1000 > abs_timeout->tv_nsec)) {
			break;
		}
		usleep(1);
	}
#endif
	return res;
}

static inline int inc_rwlock_tryrdlock(inc_rwlock_t *prwlock)
{
	return pthread_rwlock_tryrdlock(prwlock);
}

static inline int inc_rwlock_wrlock(inc_rwlock_t *prwlock)
{
	return pthread_rwlock_wrlock(prwlock);
}

static inline int inc_rwlock_timedwrlock(inc_rwlock_t *prwlock, const struct timespec *abs_timeout)
{
	int res;
#ifdef HAVE_PTHREAD_RWLOCK_TIMEDWRLOCK
	res = pthread_rwlock_timedwrlock(prwlock, abs_timeout);
#else
	do {
		struct timeval _start = inc_tvnow(), _diff;
		for (;;) {
			if (!(res = pthread_rwlock_trywrlock(prwlock))) {
				break;
			}
			_diff = inc_tvsub(inc_tvnow(), _start);
			if (_diff.tv_sec > abs_timeout->tv_sec || (_diff.tv_sec == abs_timeout->tv_sec && _diff.tv_usec * 1000 > abs_timeout->tv_nsec)) {
				break;
			}
			usleep(1);
		}
	} while (0);
#endif
	return res;
}

static inline int inc_rwlock_trywrlock(inc_rwlock_t *prwlock)
{
	return pthread_rwlock_trywrlock(prwlock);
}
#endif /* !DEBUG_THREADS */

/* Statically declared read/write locks */

#ifndef HAVE_PTHREAD_RWLOCK_INITIALIZER
#define __INC_RWLOCK_DEFINE(scope, rwlock) \
        scope inc_rwlock_t rwlock; \
static void  __attribute__((constructor)) init_##rwlock(void) \
{ \
        inc_rwlock_init(&rwlock); \
} \
static void  __attribute__((destructor)) fini_##rwlock(void) \
{ \
        inc_rwlock_destroy(&rwlock); \
}
#else
#define __INC_RWLOCK_DEFINE(scope, rwlock) \
        scope inc_rwlock_t rwlock = INC_RWLOCK_INIT_VALUE
#endif

#define INC_RWLOCK_DEFINE_STATIC(rwlock) __INC_RWLOCK_DEFINE(static, rwlock)

/*
 * Initial support for atomic instructions.
 * For platforms that have it, use the native cpu instruction to
 * implement them. For other platforms, resort to a 'slow' version
 * (defined in utils.c) that protects the atomic instruction with
 * a single lock.
 * The slow versions is always available, for testing purposes,
 * as inc_atomic_fetchadd_int_slow()
 */

int inc_atomic_fetchadd_int_slow(volatile int *p, int v);

#include "common/inline_api.h"

#if defined(HAVE_OSX_ATOMICS)
#include "libkern/OSAtomic.h"
#endif

/*! \brief Atomically add v to *p and return * the previous value of *p.
 * This can be used to handle reference counts, and the return value
 * can be used to generate unique identifiers.
 */

#if defined(HAVE_GCC_ATOMICS)
INC_INLINE_API(int inc_atomic_fetchadd_int(volatile int *p, int v),
{
	return __sync_fetch_and_add(p, v);
})
#elif defined(HAVE_OSX_ATOMICS) && (SIZEOF_INT == 4)
INC_INLINE_API(int inc_atomic_fetchadd_int(volatile int *p, int v),
{
	return OSAtomicAdd32(v, (int32_t *) p) - v;
})
#elif defined(HAVE_OSX_ATOMICS) && (SIZEOF_INT == 8)
INC_INLINE_API(int inc_atomic_fetchadd_int(volatile int *p, int v),
{
	return OSAtomicAdd64(v, (int64_t *) p) - v;
#elif defined (__i386__)
#ifdef sun
INC_INLINE_API(int inc_atomic_fetchadd_int(volatile int *p, int v),
{
	__asm __volatile (
	"       lock;  xaddl   %0, %1 ;        "
	: "+r" (v),                     /* 0 (result) */   
	  "=m" (*p)                     /* 1 */
	: "m" (*p));                    /* 2 */
	return (v);
})
#else /* ifndef sun */
INC_INLINE_API(int inc_atomic_fetchadd_int(volatile int *p, int v),
{
	__asm __volatile (
	"       lock   xaddl   %0, %1 ;        "
	: "+r" (v),                     /* 0 (result) */   
	  "=m" (*p)                     /* 1 */
	: "m" (*p));                    /* 2 */
	return (v);
})
#endif
#else   /* low performance version in utils.c */
INC_INLINE_API(int inc_atomic_fetchadd_int(volatile int *p, int v),
{
	return inc_atomic_fetchadd_int_slow(p, v);
})
#endif

/*! \brief decrement *p by 1 and return true if the variable has reached 0.
 * Useful e.g. to check if a refcount has reached 0.
 */
#if defined(HAVE_GCC_ATOMICS)
INC_INLINE_API(int inc_atomic_dec_and_test(volatile int *p),
{
	return __sync_sub_and_fetch(p, 1) == 0;
})
#elif defined(HAVE_OSX_ATOMICS) && (SIZEOF_INT == 4)
INC_INLINE_API(int inc_atomic_dec_and_test(volatile int *p),
{
	return OSAtomicAdd32( -1, (int32_t *) p) == 0;
})
#elif defined(HAVE_OSX_ATOMICS) && (SIZEOF_INT == 8)
INC_INLINE_API(int inc_atomic_dec_and_test(volatile int *p),
{
	return OSAtomicAdd64( -1, (int64_t *) p) == 0;
#else
INC_INLINE_API(int inc_atomic_dec_and_test(volatile int *p),
{
	int a = inc_atomic_fetchadd_int(p, -1);
	return a == 1; /* true if the value is 0 now (so it was 1 previously) */
})
#endif

#endif /* _INC_CC_LOCK_H */
