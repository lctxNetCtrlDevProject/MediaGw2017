/*
==================================================
** FileName: MGW_common.c
** Decription: 公共函数集
==================================================
*/

#include "PUBLIC.h"
#include <net/if.h>
#include <net/if_arp.h>

struct config * num_addr_cfg;

#if 0 
/*
 * support for 'show threads'. The start routine is wrapped by
 * dummy_start(), so that inc_register_thread() and
 * inc_unregister_thread() know the thread identifier.
 */

struct thr_arg {
	void *(*start_routine)(void *);
	void *data;
	char *name;
};

int mgw_pthread_create_stack(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *),
			     void *data, size_t stacksize, const char *file, const char *caller,
			     int line, const char *start_fn)
{
	struct thr_arg *a;

	if (!attr) {
		attr = alloca(sizeof(*attr));
		pthread_attr_init(attr);
	}

	/* On Linux, pthread_attr_init() defaults to PTHREAD_EXPLICIT_SCHED,
	   which is kind of useless. Change this here to
	   PTHREAD_INHERIT_SCHED; that way the -p option to set realtime
	   priority will propagate down to new threads by default.
	   This does mean that callers cannot set a different priority using
	   PTHREAD_EXPLICIT_SCHED in the attr argument; instead they must set
	   the priority afterwards with pthread_setschedparam(). */
	if ((errno = pthread_attr_setinheritsched(attr, PTHREAD_INHERIT_SCHED)))
		VERBOSE_OUT(LOG_SYS, "pthread_attr_setinheritsched: %s\n", strerror(errno));

	if (!stacksize)
		stacksize = MGW_STACKSIZE;

	if ((errno = pthread_attr_setstacksize(attr, stacksize ? stacksize : MGW_STACKSIZE)))
		VERBOSE_OUT(LOG_SYS, "pthread_attr_setstacksize: %s\n", strerror(errno));

	if ((a = malloc(sizeof(*a)))) {
		a->start_routine = start_routine;
		a->data = data;
		DEBUG_OUT("%-10s started at [%5d] %s %s()\n",
			     start_fn, line, file, caller);
		data = a;
	}

	return pthread_create(thread, attr, start_routine, data);
}


/*
** 使用poll函数，IO管理函数
*
*/
#if 0
struct io_context *io_context_create(void)
{
	/* Create an I/O context */
	struct io_context *tmp;
	if ((tmp = malloc(sizeof(*tmp)))) {
		tmp->needshrink = 0;
		tmp->fdcnt = 0;
		tmp->maxfdcnt = GROW_SHRINK_SIZE/2;
		tmp->current_ioc = -1;
		if (!(tmp->fds = calloc(1, (GROW_SHRINK_SIZE / 2) * sizeof(*tmp->fds)))) {
			free(tmp);
			tmp = NULL;
		} else {
			if (!(tmp->ior = calloc(1, (GROW_SHRINK_SIZE / 2) * sizeof(*tmp->ior)))) {
				free(tmp->fds);
				free(tmp);
				tmp = NULL;
			}
		}
	}
	return tmp;
}

void io_context_destroy(struct io_context *ioc)
{
	/* Free associated memory with an I/O context */
	if (ioc->fds)
		free(ioc->fds);
	if (ioc->ior)
		free(ioc->ior);
	free(ioc);
}

static int io_grow(struct io_context *ioc)
{
	/* 
	 * Grow the size of our arrays.  Return 0 on success or
	 * -1 on failure
	 */
	void *tmp;
	ioc->maxfdcnt += GROW_SHRINK_SIZE;
	if ((tmp = realloc(ioc->ior, (ioc->maxfdcnt + 1) * sizeof(*ioc->ior)))) {
		ioc->ior = tmp;
		if ((tmp = realloc(ioc->fds, (ioc->maxfdcnt + 1) * sizeof(*ioc->fds)))) {
			ioc->fds = tmp;
		} else {
			/*
			 * Failed to allocate enough memory for the pollfd.  Not
			 * really any need to shrink back the iorec's as we'll
			 * probably want to grow them again soon when more memory
			 * is available, and then they'll already be the right size
			 */
			ioc->maxfdcnt -= GROW_SHRINK_SIZE;
			return -1;
		}
	} else {
		/*
		 * Memory allocation failure.  We return to the old size, and 
		 * return a failure
		 */
		ioc->maxfdcnt -= GROW_SHRINK_SIZE;
		return -1;
	}
	return 0;
}

int *io_add(struct io_context *ioc, int fd, mgw_io_cb callback, short events, void *data)
{
	/*
	 * Add a new I/O entry for this file descriptor
	 * with the given event mask, to call callback with
	 * data as an argument.  Returns NULL on failure.
	 */
	int *ret;
	if (ioc->fdcnt >= ioc->maxfdcnt) {
		/* 
		 * We don't have enough space for this entry.  We need to
		 * reallocate maxfdcnt poll fd's and io_rec's, or back out now.
		 */
		if (io_grow(ioc))
			return NULL;
	}

	/*
	 * At this point, we've got sufficiently large arrays going
	 * and we can make an entry for it in the pollfd and io_r
	 * structures.
	 */
	ioc->fds[ioc->fdcnt].fd = fd;
	ioc->fds[ioc->fdcnt].events = events;
	ioc->fds[ioc->fdcnt].revents = 0;
	ioc->ior[ioc->fdcnt].callback = callback;
	ioc->ior[ioc->fdcnt].data = data;
	if (!(ioc->ior[ioc->fdcnt].id = malloc(sizeof(*ioc->ior[ioc->fdcnt].id)))) {
		/* Bonk if we couldn't allocate an int */
		return NULL;
	}
	*(ioc->ior[ioc->fdcnt].id) = ioc->fdcnt;
	ret = ioc->ior[ioc->fdcnt].id;
	ioc->fdcnt++;
	return ret;
}

int *io_change(struct io_context *ioc, int *id, int fd, mgw_io_cb callback, short events, void *data)
{
	if (*id < ioc->fdcnt) {
		if (fd > -1)
			ioc->fds[*id].fd = fd;
		if (callback)
			ioc->ior[*id].callback = callback;
		if (events)
			ioc->fds[*id].events = events;
		if (data)
			ioc->ior[*id].data = data;
		return id;
	}
	return NULL;
}

static int io_shrink(struct io_context *ioc)
{
	int getfrom;
	int putto = 0;
	/* 
	 * Bring the fields from the very last entry to cover over
	 * the entry we are removing, then decrease the size of the 
	 * arrays by one.
	 */
	for (getfrom = 0; getfrom < ioc->fdcnt; getfrom++) {
		if (ioc->ior[getfrom].id) {
			/* In use, save it */
			if (getfrom != putto) {
				ioc->fds[putto] = ioc->fds[getfrom];
				ioc->ior[putto] = ioc->ior[getfrom];
				*(ioc->ior[putto].id) = putto;
			}
			putto++;
		}
	}
	ioc->fdcnt = putto;
	ioc->needshrink = 0;
	/* FIXME: We should free some memory if we have lots of unused
	   io structs */
	return 0;
}

int io_remove(struct io_context *ioc, int *_id)
{
	int x;
	if (!_id) {
		VERBOSE_OUT(LOG_SYS, "Asked to remove NULL?\n");
		return -1;
	}
	for (x = 0; x < ioc->fdcnt; x++) {
		if (ioc->ior[x].id == _id) {
			/* Free the int immediately and set to NULL so we know it's unused now */
			free(ioc->ior[x].id);
			ioc->ior[x].id = NULL;
			ioc->fds[x].events = 0;
			ioc->fds[x].revents = 0;
			ioc->needshrink = 1;
			if (ioc->current_ioc == -1)
				io_shrink(ioc);
			return 0;
		}
	}
	
	VERBOSE_OUT(LOG_SYS, "Unable to remove unknown id %p\n", _id);
	return -1;
}

int io_wait(struct io_context *ioc, int howlong)
{
	/*
	 * Make the poll call, and call
	 * the callbacks for anything that needs
	 * to be handled
	 */
	int res;
	int x;
	int origcnt;
	res = poll(ioc->fds, ioc->fdcnt, howlong);
	if (res > 0) {
		/*
		 * At least one event
		 */
		origcnt = ioc->fdcnt;
		for(x = 0; x < origcnt; x++) {
			/* Yes, it is possible for an entry to be deleted and still have an
			   event waiting if it occurs after the original calling id */
			if (ioc->fds[x].revents && ioc->ior[x].id) {
				/* There's an event waiting */
				ioc->current_ioc = *ioc->ior[x].id;
				if (ioc->ior[x].callback) {
					if (!ioc->ior[x].callback(ioc->ior[x].id, ioc->fds[x].fd, ioc->fds[x].revents, ioc->ior[x].data)) {
						/* Time to delete them since they returned a 0 */
						io_remove(ioc, ioc->ior[x].id);
					}
				}
				ioc->current_ioc = -1;
			}
		}
		if (ioc->needshrink)
			io_shrink(ioc);
	}
	return res;
}
#endif
void io_dump(struct io_context *ioc)
{
	/*
	 * Print some debugging information via
	 * the logger interface
	 */
	int x;
	DEBUG_OUT("MGW IO Dump: %d entries, %d max entries\n", ioc->fdcnt, ioc->maxfdcnt);
	DEBUG_OUT("==================================================\n");
	DEBUG_OUT("| ID    FD     Callback     Data         Events  |\n");
	DEBUG_OUT("+------+------+------------+------------+--------+\n");
	for (x = 0; x < ioc->fdcnt; x++) {
		DEBUG_OUT("| %.4d | %.4d | %-10p | %-10p | %.6x |\n", 
				*ioc->ior[x].id,
				ioc->fds[x].fd,
				ioc->ior[x].callback,
				ioc->ior[x].data,
				ioc->fds[x].events);
	}
	DEBUG_OUT("==================================================\n");
}

/*
** Sched Managment
* 调度管理
*/

#if 0
/*
! note: 使用inc_cc的链表管理宏以便适应调度管理的需求
*/
#define INC_LIST_ENTRY(type)						\
struct {								\
	struct type *next;						\
}

#define	INC_LIST_FIRST(head)	((head)->first)

#define INC_LIST_HEAD_NOLOCK(name, type)				\
struct name {								\
	struct type *first;						\
	struct type *last;						\
}

#define INC_LIST_REMOVE_HEAD(head, field) ({				\
	typeof((head)->first) cur = (head)->first;		\
	if (cur) {						\
		(head)->first = cur->field.next;		\
		cur->field.next = NULL;				\
		if ((head)->last == cur)			\
			(head)->last = NULL;			\
	}							\
	cur;							\
})

#define INC_LIST_INSERT_HEAD(head, elm, field) do {			\
		(elm)->field.next = (head)->first;			\
		(head)->first = (elm);					\
		if (!(head)->last)					\
			(head)->last = (elm);				\
} while (0)

#define	INC_LIST_EMPTY(head)	(INC_LIST_FIRST(head) == NULL)

#define INC_LIST_TRAVERSE_SAFE_BEGIN(head, var, field) {				\
	typeof((head)->first) __list_next;						\
	typeof((head)->first) __list_prev = NULL;					\
	typeof((head)->first) __new_prev = NULL;					\
	for ((var) = (head)->first, __new_prev = (var),					\
	      __list_next = (var) ? (var)->field.next : NULL;				\
	     (var);									\
	     __list_prev = __new_prev, (var) = __list_next,				\
	     __new_prev = (var),							\
	     __list_next = (var) ? (var)->field.next : NULL				\
	    )

#define INC_LIST_INSERT_BEFORE_CURRENT(head, elm, field) do {		\
	if (__list_prev) {						\
		(elm)->field.next = __list_prev->field.next;		\
		__list_prev->field.next = elm;				\
	} else {							\
		(elm)->field.next = (head)->first;			\
		(head)->first = (elm);					\
	}								\
	__new_prev = (elm); 					\
} while (0)

#define INC_LIST_REMOVE_CURRENT(head, field) do { \
	__new_prev->field.next = NULL;							\
	__new_prev = __list_prev;							\
	if (__list_prev)								\
		__list_prev->field.next = __list_next;					\
	else										\
		(head)->first = __list_next;						\
	if (!__list_next)								\
		(head)->last = __list_prev; \
	} while (0)


#define INC_LIST_INSERT_TAIL(head, elm, field) do {			\
	  if (!(head)->first) { 					\
		(head)->first = (elm);					\
		(head)->last = (elm);					\
	  } else {								\
		(head)->last->field.next = (elm);			\
		(head)->last = (elm);					\
	  } 								\
} while (0)

#define INC_LIST_TRAVERSE_SAFE_END  }

#define INC_LIST_TRAVERSE(head,var,field) 				\
	for((var) = (head)->first; (var); (var) = (var)->field.next)
#endif


struct sched {
	INC_LIST_ENTRY(sched) list;
	int id; 					  /*!< ID number of event */
	struct timeval when;		  /*!< Absolute time event should take place */
	int resched;				  /*!< When to reschedule */
	int variable;				  /*!< Use return value from callback to reschedule */
	const void *data;			  /*!< Data */
	sched_cb callback;		  /*!< Callback */
};

struct sched_context {
	unsigned int eventcnt;					/*!< Number of events processed */
	unsigned int schedcnt;					/*!< Number of outstanding schedule events */
	INC_LIST_HEAD_NOLOCK(, sched) schedq;   /*!< Schedule entry and main queue */
	INC_LIST_HEAD_NOLOCK(, sched) schedc;	/*!< Cache of unused schedule structures and how many */
	unsigned int schedccnt;
};

#define SCHED_MAX_CACHE 128

struct sched_context *sched_context_create(void)
{
	struct sched_context *tmp;

	if (!(tmp = calloc(1, sizeof(*tmp))))
		return NULL;

	tmp->eventcnt = 1;
	
	return tmp;
}

void sched_context_destroy(struct sched_context *con)
{
	struct sched *s;

#ifdef SCHED_MAX_CACHE
	/* Eliminate the cache */
	while ((s = INC_LIST_REMOVE_HEAD(&con->schedc, list)))
		free(s);
#endif

	/* And the queue */
	while ((s = INC_LIST_REMOVE_HEAD(&con->schedq, list)))
		free(s);
	
	/* And the context */
	free(con);
}

static struct sched *sched_alloc(struct sched_context *con)
{
	struct sched *tmp;

	/*
	 * We keep a small cache of schedule entries
	 * to minimize the number of necessary malloc()'s
	 */
#ifdef SCHED_MAX_CACHE
	if ((tmp = INC_LIST_REMOVE_HEAD(&con->schedc, list)))
		con->schedccnt--;
	else
#endif
		tmp = calloc(1, sizeof(*tmp));

	return tmp;
}

static void sched_release(struct sched_context *con, struct sched *tmp)
{
	/*
	 * Add to the cache, or just free() if we
	 * already have too many cache entries
	 */

#ifdef SCHED_MAX_CACHE	 
	if (con->schedccnt < SCHED_MAX_CACHE) {
		INC_LIST_INSERT_HEAD(&con->schedc, tmp, list);
		con->schedccnt++;
	} else
#endif
	free(tmp);
}

/*! \brief
 * Return the number of milliseconds 
 * until the next scheduled event
 */
int sched_wait(struct sched_context *con)
{
	int ms;

	if (INC_LIST_EMPTY(&con->schedq)) {
		ms = -1;
	} else {
		ms = mgw_tvdiff_ms(INC_LIST_FIRST(&con->schedq)->when, mgw_tvnow());
		if (ms < 0)
			ms = 0;
	}

	return ms;
}


/*! \brief
 * Take a sched structure and put it in the
 * queue, such that the soonest event is
 * first in the list. 
 */
static void schedule(struct sched_context *con, struct sched *s)
{
	 
	struct sched *cur = NULL;
	
	INC_LIST_TRAVERSE_SAFE_BEGIN(&con->schedq, cur, list) {
		if (mgw_tvcmp(s->when, cur->when) == -1) {
			INC_LIST_INSERT_BEFORE_CURRENT(&con->schedq, s, list);
			break;
		}
	}
	INC_LIST_TRAVERSE_SAFE_END
	if (!cur)
		INC_LIST_INSERT_TAIL(&con->schedq, s, list);
	
	con->schedcnt++;
}

/*! \brief
 * given the last event *tv and the offset in milliseconds 'when',
 * computes the next value,
 */
static int sched_settime(struct timeval *tv, int when)
{
	struct timeval now = mgw_tvnow();

	/*inc_log(LOG_DEBUG, "TV -> %lu,%lu\n", tv->tv_sec, tv->tv_usec);*/
	if (mgw_tvzero(*tv))	/* not supplied, default to now */
		*tv = now;
	*tv = mgw_tvadd(*tv, mgw_samp2tv(when, 1000));
	if (mgw_tvcmp(*tv, now) < 0) {
		*tv = now;
	}
	return 0;
}


/*! \brief
 * Schedule callback(data) to happen when ms into the future
 */
int sched_add_variable(struct sched_context *con, int when, sched_cb callback, const void *data, int variable)
{
	struct sched *tmp;
	int res = -1;

	if ((tmp = sched_alloc(con))) {
		tmp->id = con->eventcnt++;
		tmp->callback = callback;
		tmp->data = data;
		tmp->resched = when;
		tmp->variable = variable;
		tmp->when = mgw_tv(0, 0);
		if (sched_settime(&tmp->when, when)) {
			sched_release(con, tmp);
		} else {
			schedule(con, tmp);
			res = tmp->id;
		}
	}

	return res;
}

int sched_add(struct sched_context *con, int when, sched_cb callback, const void *data)
{
	return sched_add_variable(con, when, callback, data, 0);
}

/*! \brief
 * Delete the schedule entry with number
 * "id".  It's nearly impossible that there
 * would be two or more in the list with that
 * id.
 */
int sched_del(struct sched_context *con, int id)
{
	struct sched *s;

	INC_LIST_TRAVERSE_SAFE_BEGIN(&con->schedq, s, list) {
		if (s->id == id) {
			INC_LIST_REMOVE_CURRENT(&con->schedq, list);
			con->schedcnt--;
			sched_release(con, s);
			break;
		}
	}
	INC_LIST_TRAVERSE_SAFE_END

	if (!s) {
		return -1;
	}
	
	return 0;
}

/*! \brief Dump the contents of the scheduler to LOG_DEBUG */
void sched_dump(const struct sched_context *con)
{
	struct sched *q;
	struct timeval tv = mgw_tvnow();
	DEBUG_OUT("MGW Schedule Dump (%d in Q, %d Total, %d Cache)\n", con->schedcnt, con->eventcnt - 1, con->schedccnt);
	DEBUG_OUT("=============================================================\n");
	DEBUG_OUT("|ID    Callback          Data              Time  (sec:ms)   |\n");
	DEBUG_OUT("+-----+-----------------+-----------------+-----------------+\n");

	INC_LIST_TRAVERSE(&con->schedq, q, list) {
		struct timeval delta = mgw_tvsub(q->when, tv);

		DEBUG_OUT("|%.4d | %-15p | %-15p | %.6ld : %.6ld |\n", 
			q->id,
			q->callback,
			q->data,
			delta.tv_sec,
			(long int)delta.tv_usec);
	}
	DEBUG_OUT("=============================================================\n");
	
}

/*! \brief
 * Launch all events which need to be run at this time.
 */
int sched_runq(struct sched_context *con)
{
	struct sched *current;
	struct timeval tv;
	int numevents;
	int res;

	for (numevents = 0; !INC_LIST_EMPTY(&con->schedq); numevents++) {
		/* schedule all events which are going to expire within 1ms.
		 * We only care about millisecond accuracy anyway, so this will
		 * help us get more than one event at one time if they are very
		 * close together.
		 */
		tv = mgw_tvadd(mgw_tvnow(), mgw_tv(0, 1000));
		if (mgw_tvcmp(INC_LIST_FIRST(&con->schedq)->when, tv) != -1)
			break;
		
		current = INC_LIST_REMOVE_HEAD(&con->schedq, list);
		con->schedcnt--;

		/*
		 * At this point, the schedule queue is still intact.  We
		 * have removed the first event and the rest is still there,
		 * so it's permissible for the callback to add new events, but
		 * trying to delete itself won't work because it isn't in
		 * the schedule queue.  If that's what it wants to do, it 
		 * should return 0.
		 */
			
		res = current->callback(current->data);
			
		if (res) {
		 	/*
			 * If they return non-zero, we should schedule them to be
			 * run again.
			 */
			if (sched_settime(&current->when, current->variable? res : current->resched)) {
				sched_release(con, current);
			} else
				schedule(con, current);
		} else {
			/* No longer needed, so release it */
		 	sched_release(con, current);
		}
	}
	
	return numevents;
}

long sched_when(struct sched_context *con,int id)
{
	struct sched *s;
	long secs = -1;

	INC_LIST_TRAVERSE(&con->schedq, s, list) {
		if (s->id == id)
			break;
	}
	if (s) {
		struct timeval now = mgw_tvnow();
		secs = s->when.tv_sec - now.tv_sec;
	}
	
	return secs;
}
#endif

/*!
 * When looking up extensions, we can have different requests
 * identified by the 'action' argument, as follows.
 * Note that the coding is such that the low 4 bits are the
 * third argument to extension_match_core.
 */
enum ext_match_t {
	E_MATCHMORE = 	0x00,	/* extension can match but only with more 'digits' */
	E_CANMATCH =	0x01,	/* extension can match with or without more 'digits' */
	E_MATCH =	0x02,	/* extension is an exact match */
	E_MATCH_MASK =	0x03,	/* mask for the argument to extension_match_core() */
	E_SPAWN =	0x12,	/* want to spawn an extension. Requires exact match */
	E_FINDLABEL =	0x22	/* returns the priority for a given label. Requires exact match */
};

//static int _extension_match_core(const char *pattern, const char *data, enum ext_match_t mode)
static int _extension_match_core(char *pattern, char *data, enum ext_match_t mode)
{
	mode &= E_MATCH_MASK;	/* only consider the relevant bits */

	if ( (mode == E_MATCH) && (pattern[0] == '_') && (strcasecmp(pattern,data)==0) ) /* note: if this test is left out, then _x. will not match _x. !!! */
		return 1;

	if (pattern[0] != '_') { /* not a pattern, try exact or partial match */
		int ld = strlen(data), lp = strlen(pattern);

		if (lp < ld)		/* pattern too short, cannot match */
			return 0;
		/* depending on the mode, accept full or partial match or both */
		if (mode == E_MATCH)
			return !strcmp(pattern, data); /* 1 on match, 0 on fail */
		if (ld == 0 || !strncasecmp(pattern, data, ld)) /* partial or full match */
			return (mode == E_MATCHMORE) ? lp > ld : 1; /* XXX should consider '!' and '/' ? */
		else
			return 0;
	}
	pattern++; /* skip leading _ */
	/*
	 * XXX below we stop at '/' which is a separator for the CID info. However we should
	 * not store '/' in the pattern at all. When we insure it, we can remove the checks.
	 */
	while (*data && *pattern && *pattern != '/') {
		char *end = NULL;

		if (*data == '-') { /* skip '-' in data (just a separator) */
			data++;
			continue;
		}
		switch (toupper(*pattern)) {
		case '[':	/* a range */
			end = strchr(pattern+1, ']'); /* XXX should deal with escapes ? */
			if (end == NULL) {
				printf("Wrong usage of [] in the extension\n");
				return 0;	/* unconditional failure */
			}
			for (pattern++; pattern != end; pattern++) {
				if (pattern+2 < end && pattern[1] == '-') { /* this is a range */
					if (*data >= pattern[0] && *data <= pattern[2])
						break;	/* match found */
					else {
						pattern += 2; /* skip a total of 3 chars */
						continue;
					}
				} else if (*data == pattern[0])
					break;	/* match found */
			}
			if (pattern == end)
				return 0;
			pattern = end;	/* skip and continue */
			break;
		case 'N':
			if (*data < '2' || *data > '9')
				return 0;
			break;
		case 'X':
			if (*data < '0' || *data > '9')
				return 0;
			break;
		case 'Z':
			if (*data < '1' || *data > '9')
				return 0;
			break;
		case '.':	/* Must match, even with more digits */
			return 1;
		case '!':	/* Early match */
			return 2;
		case ' ':
		case '-':	/* Ignore these in patterns */
			data--; /* compensate the final data++ */
			break;
		default:
			if (*data != *pattern)
				return 0;
		}
		data++;
		pattern++;
	}
	if (*data)			/* data longer than pattern, no match */
		return 0;
	/*
	 * match so far, but ran off the end of the data.
	 * Depending on what is next, determine match or not.
	 */
	if (*pattern == '\0' || *pattern == '/')	/* exact match */
		return (mode == E_MATCHMORE) ? 0 : 1;	/* this is a failure for E_MATCHMORE */
	else if (*pattern == '!')			/* early match */
		return 2;
	else						/* partial match */
		return (mode == E_MATCH) ? 0 : 1;	/* this is a failure for E_MATCH */
}

//void mgw_load_extension(const char* filename)
void mgw_load_extension(char* filename)
{
	if(!filename){
		VERBOSE_OUT(LOG_SYS,"filename is nul!!!\n");
		return;
	}
	num_addr_cfg = inc_calloc(1,sizeof(struct config));
	if(!num_addr_cfg){
		perror("inc_calloc error:");
		return;
	}
	//strcpy(num_addr_cfg->name,filename);
	strncpy((char *)num_addr_cfg->name,filename, sizeof(num_addr_cfg->name));
	if(!load_config(num_addr_cfg))
		VERBOSE_OUT(LOG_SYS,"Load %s Success\n",filename);
	else{
		VERBOSE_OUT(LOG_SYS,"Load %s Fail,Please Check it out\n",filename);
		exit(-1);
	}
}

//unsigned long find_node_by_callee(const char * num)
unsigned long find_node_by_callee(char * num)
{
	/*!breif
	* extension.cfg file text format like that:
	* [default]
	* 88c00210 = _7210XXX
	* 88c01000 = _1000.
	* ...
	*/
	struct category * iterator_cat;
	struct variable * iterator_var;
	int res;
	
	for(iterator_cat=(num_addr_cfg->category_list)->root;iterator_cat;iterator_cat=iterator_cat->next)
	{
		/*Travel all category*/
		for(iterator_var=iterator_cat->variable_list->root;iterator_var;iterator_var=iterator_var->next)
		{
			/*Travel all variable*/
			res = _extension_match_core(iterator_var->value,num,E_MATCH);
			if(res == E_CANMATCH || res == E_MATCH)
				return strtoul(iterator_var->name,NULL,16);
		}
	}
	
	return 0;
}

char* find_extension_by_node(unsigned long node)
{
	struct category * iterator_cat;
	struct variable * iterator_var;
	for(iterator_cat=num_addr_cfg->category_list->root;iterator_cat;iterator_cat=iterator_cat->next)
	{
		/*Travel all category*/
		for(iterator_var=iterator_cat->variable_list->root;iterator_var;iterator_var=iterator_var->next)
		{
			/*Travel all variable*/
			if(strtoul(iterator_var->name,NULL,16) ==  node)
				return iterator_var->value;
		}
	}
	return NULL;
}

/*!
* \breif add num addr relactionship 
* @param hf_addr[IN]: the node address
* @param phone[IN]: the phone string
* @return 0--success;other--fail
*/
//int mgw_add_num_addr(unsigned long hf_addr,const char* phone)
int mgw_add_num_addr(unsigned long hf_addr,char* phone)
{
	char str_hf_addr[16];
	char * result,*src1,*src2;
	char tmp_phone[32]="";

	VERBOSE_OUT(LOG_SYS,"add %08x-%s\n",hf_addr,phone);
	
	if(!hf_addr || !phone)
		return (-1);

	if(!num_addr_cfg)
		return (-1);

	strncpy((char *)tmp_phone,"_",1);
	//strcat(tmp_phone,phone);
	if(strlen(phone)> 31)
	{
        return DRV_ERR;
	}
	
	strncat(tmp_phone, phone, strlen(phone));
	snprintf(str_hf_addr, sizeof(str_hf_addr),"%08x", (unsigned int)hf_addr);
	
	/*! step 1. adjust if there is match addr? */
	result = find_config_var(num_addr_cfg, str_hf_addr);
	if(!result){
		/* we not find the match,we add new item */
		add_config_var(num_addr_cfg,"default",str_hf_addr,tmp_phone);
		rewrite_config(num_addr_cfg);
	}else{
		/*! step 2. adjust if there is match phone? */
		if(find_node_by_callee(phone) != hf_addr){
			/* if phone isn't match,then we will combine the phone */
			src1 = find_extension_by_node(hf_addr);
			src2 = tmp_phone;
			/* combine arithmetic */
			while(*src1 && *src2){
				if(*src1 == 'X'){
					src1++;
					src2++;
				}else{
					if(*src1 == *src2){
						src1++;
						src2++;
					}else{
						/* change the value */
						*src1 = 'X';
						src1++;
						src2++;
					}
				}
			}
			rewrite_config(num_addr_cfg);
		}
	}
	return 0;
}

int mgw_sys_exec(char * cmd)
{
	int status = 0;

	fflush(stdout);
	switch(vfork())
	{
		case 0:
			if(system(cmd) == (-1))
				VERBOSE_OUT(LOG_SYS,"exec cmd %s fail!\n",cmd);
			else
				VERBOSE_OUT(LOG_SYS,"exec cmd %s\n",cmd);
			exit(1);
			break;

		case (-1):
			break;

		default:
			wait(&status);
	}
	
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		VERBOSE_OUT(LOG_SYS,"exec cmd %s success!\n",cmd);
	return 0;
}


//unsigned long mgw_if_fetch_ip(const char* if_name)
unsigned long mgw_if_fetch_ip(char* if_name)
{
	struct ifreq ifr;
	int skfd;
	struct sockaddr	addr;
	struct sockaddr_in * p_addr;
	unsigned long s_addr = 0xffffffff;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd < 0)
	{
        return DRV_ERR;
	}
	
	strncpy((char *)ifr.ifr_name, if_name,16);
	ifr.ifr_addr.sa_family = AF_INET;
	memset(&addr, 0xff, sizeof(struct sockaddr));
	
	if (ioctl(skfd, SIOCGIFDSTADDR, &ifr) == 0){
		addr = ifr.ifr_dstaddr;
		p_addr = (struct sockaddr_in *)&addr;
		s_addr = p_addr->sin_addr.s_addr;
	}
	close(skfd);
	return s_addr;
}

unsigned long mgw_if_fetch_mask(char* if_name)
{
	struct ifreq ifr;
	int skfd;
	struct sockaddr	mask;
	struct sockaddr_in * p_mask = (struct sockaddr_in *)&mask;
	unsigned long s_addr = 0xffffffff;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd < 0)
	{
        return DRV_ERR;
	}	
	strncpy((char *)ifr.ifr_name, if_name,16);
	ifr.ifr_addr.sa_family = AF_INET;
	memset(&mask, 0xff, sizeof(struct sockaddr));	
	if ((ioctl(skfd, SIOCGIFDSTADDR, &ifr) == 0) && \
		(ioctl(skfd, SIOCGIFNETMASK, &ifr) >= 0)){
		mask = ifr.ifr_netmask;
		p_mask= (struct sockaddr_in *)&mask;
		s_addr = p_mask->sin_addr.s_addr;
	}
	close(skfd);
	return s_addr;	
}

//unsigned long mgw_if_fetch_boardaddr(const char* if_name)
unsigned long mgw_if_fetch_boardaddr(char* if_name)
{
#if 0
	struct ifreq ifr;
	int skfd;
	struct sockaddr	broadaddr;
	struct sockaddr_in * p_broadaddr=&broadaddr;
	unsigned long s_addr = 0xffffffff;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, if_name,16);
	ifr.ifr_addr.sa_family = AF_INET;
	memset(&broadaddr, 0xff, sizeof(struct sockaddr));	
	if ((ioctl(skfd, SIOCGIFDSTADDR, &ifr) == 0) && \
		(ioctl(skfd, SIOCGIFBRDADDR, &ifr) >= 0)){
		broadaddr = ifr.ifr_netmask;
		p_broadaddr= &broadaddr;
		s_addr = p_broadaddr->sin_addr.s_addr;
	}
	close(skfd);
	return s_addr;
#else
	unsigned long s_addr = 0xffffffff;
	unsigned long mask,ip_addr;
	ip_addr = mgw_if_fetch_ip(if_name);
	mask = mgw_if_fetch_mask(if_name);
	s_addr = ip_addr|(~mask);
	return s_addr;
#endif
}


