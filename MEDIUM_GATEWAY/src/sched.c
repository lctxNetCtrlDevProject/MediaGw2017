/*! \file
 *
 * \brief Scheduler Routines (from cheops-NG)
 *
 */
#include "common/autoconfig.h"
//#include "common/compat.h"
	 
#include "common/paths.h"
#include "common/stringfields.h"

int option_debug;				/*!< Debug level */



#ifdef DEBUG_SCHEDULER
#define DEBUG(a) do { \
	if (option_debug) \
		DEBUG_M(a) \
	} while (0)
#else
#define DEBUG(a) 
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#include "common/sched.h"
#include "common/logger.h"
#include "common/lock.h"
#include "common/utils.h"
#include "common/linkedlists.h"
//#include "common/options.h"

struct sched {
	INC_LIST_ENTRY(sched) list;
	int id;                       /*!< ID number of event */
	struct timeval when;          /*!< Absolute time event should take place */
	int resched;                  /*!< When to reschedule */
	int variable;                 /*!< Use return value from callback to reschedule */
	const void *data;             /*!< Data */
	inc_sched_cb callback;        /*!< Callback */
};

struct sched_context {
	inc_mutex_t lock;
	unsigned int eventcnt;                  /*!< Number of events processed */
	unsigned int schedcnt;                  /*!< Number of outstanding schedule events */
	INC_LIST_HEAD_NOLOCK(, sched) schedq;   /*!< Schedule entry and main queue */

#ifdef SCHED_MAX_CACHE
	INC_LIST_HEAD_NOLOCK(, sched) schedc;   /*!< Cache of unused schedule structures and how many */
	unsigned int schedccnt;
#endif
};

struct sched_context *sched_context_create(void)
{
	struct sched_context *tmp;

	if (!(tmp = inc_calloc(1, sizeof(*tmp))))
		return NULL;

	inc_mutex_init(&tmp->lock);
	tmp->eventcnt = 1;
	
	return tmp;
}

void sched_context_destroy(struct sched_context *con)
{
	struct sched *s;

	inc_mutex_lock(&con->lock);

#ifdef SCHED_MAX_CACHE
	/* Eliminate the cache */
	while ((s = INC_LIST_REMOVE_HEAD(&con->schedc, list)))
		free(s);
#endif

	/* And the queue */
	while ((s = INC_LIST_REMOVE_HEAD(&con->schedq, list)))
		free(s);
	
	/* And the context */
	inc_mutex_unlock(&con->lock);
	inc_mutex_destroy(&con->lock);
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
		tmp = inc_calloc(1, sizeof(*tmp));

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
int inc_sched_wait(struct sched_context *con)
{
	int ms;

	DEBUG(inc_log(LOG_DEBUG, "inc_sched_wait()\n"));

	inc_mutex_lock(&con->lock);
	if (INC_LIST_EMPTY(&con->schedq)) {
		ms = -1;
	} else {
		ms = inc_tvdiff_ms(INC_LIST_FIRST(&con->schedq)->when, inc_tvnow());
		if (ms < 0)
			ms = 0;
	}
	inc_mutex_unlock(&con->lock);

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
		if (inc_tvcmp(s->when, cur->when) == -1) {
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
	struct timeval now = inc_tvnow();

	/*inc_log(LOG_DEBUG, "TV -> %lu,%lu\n", tv->tv_sec, tv->tv_usec);*/
	if (inc_tvzero(*tv))	/* not supplied, default to now */
		*tv = now;
	*tv = inc_tvadd(*tv, inc_samp2tv(when, 1000));
	if (inc_tvcmp(*tv, now) < 0) {
		*tv = now;
	}
	return 0;
}


/*! \brief
 * Schedule callback(data) to happen when ms into the future
 */
int inc_sched_add_variable(struct sched_context *con, int when, inc_sched_cb callback, const void *data, int variable)
{
	struct sched *tmp;
	int res = -1;

	DEBUG(inc_log(LOG_DEBUG, "inc_sched_add()\n"));

	inc_mutex_lock(&con->lock);
	if ((tmp = sched_alloc(con))) {
		tmp->id = con->eventcnt++;
		tmp->callback = callback;
		tmp->data = data;
		tmp->resched = when;
		tmp->variable = variable;
		tmp->when = inc_tv(0, 0);
		if (sched_settime(&tmp->when, when)) {
			sched_release(con, tmp);
		} else {
			schedule(con, tmp);
			res = tmp->id;
		}
	}
#ifdef DUMP_SCHEDULER
	/* Dump contents of the context while we have the lock so nothing gets screwed up by accident. */
	if (option_debug)
		inc_sched_dump(con);
#endif
	inc_mutex_unlock(&con->lock);

	return res;
}

int inc_sched_add(struct sched_context *con, int when, inc_sched_cb callback, const void *data)
{
	return inc_sched_add_variable(con, when, callback, data, 0);
}

/*! \brief
 * Delete the schedule entry with number
 * "id".  It's nearly impossible that there
 * would be two or more in the list with that
 * id.
 */
#ifndef INC_DEVMODE
int inc_sched_del(struct sched_context *con, int id)
#else
int _inc_sched_del(struct sched_context *con, int id, const char *file, int line, const char *function)
#endif
{
	struct sched *s;

	DEBUG(inc_log(LOG_DEBUG, "inc_sched_del()\n"));
	
	inc_mutex_lock(&con->lock);
	INC_LIST_TRAVERSE_SAFE_BEGIN(&con->schedq, s, list) {
		if (s->id == id) {
			INC_LIST_REMOVE_CURRENT(&con->schedq, list);
			con->schedcnt--;
			sched_release(con, s);
			break;
		}
	}
	INC_LIST_TRAVERSE_SAFE_END

#ifdef DUMP_SCHEDULER
	/* Dump contents of the context while we have the lock so nothing gets screwed up by accident. */
	if (option_debug)
		inc_sched_dump(con);
#endif
	inc_mutex_unlock(&con->lock);

	if (!s) {
		if (option_debug)
			inc_log(LOG_DEBUG, "Attempted to delete nonexistent schedule entry %d!\n", id);
#ifndef INC_DEVMODE
		inc_assert(s != NULL);
#else
		_inc_assert(0, "s != NULL", file, line, function);
#endif
		return -1;
	}
	
	return 0;
}

/*! \brief Dump the contents of the scheduler to LOG_DEBUG */
void inc_sched_dump(const struct sched_context *con)
{
	struct sched *q;
	struct timeval tv = inc_tvnow();
#ifdef SCHED_MAX_CACHE
	inc_log(LOG_DEBUG, "INC_CC Schedule Dump (%d in Q, %d Total, %d Cache)\n", con->schedcnt, con->eventcnt - 1, con->schedccnt);
#else
	inc_log(LOG_DEBUG, "INC_CC Schedule Dump (%d in Q, %d Total)\n", con->schedcnt, con->eventcnt - 1);
#endif

	inc_log(LOG_DEBUG, "=============================================================\n");
	inc_log(LOG_DEBUG, "|ID    Callback          Data              Time  (sec:ms)   |\n");
	inc_log(LOG_DEBUG, "+-----+-----------------+-----------------+-----------------+\n");

/*
=============================================================
|ID    Callback          Data              Time  (sec:ms)   |
+-----+-----------------+-----------------+-----------------+
|0025 | 0x1002b0b8      | 0x1017eef0      | 000000 : 999978 |
|0015 | 0x100364c8      | 0x10175c30      | 000004 : 619751 |
|0019 | 0x100364c8      | 0x101773b0      | 000004 : 715690 |
|0023 | 0x100364c8      | 0x10178d38      | 000004 : 838203 |
|0026 | 0x1002b018      | 0x1017a6c0      | 000031 : 999996 |
|0012 | 0x1003efd0      | 0x10173a68      | 000077 : 550967 |
|0016 | 0x1003efd0      | 0x10172ff0      | 000077 : 620119 |
|0020 | 0x1003efd0      | 0x10172578      | 000077 : 716060 |
|0024 | 0x1003efd0      | 0x10171c08      | 000077 : 838610 |
=============================================================
*/
	
	INC_LIST_TRAVERSE(&con->schedq, q, list) {
		struct timeval delta = inc_tvsub(q->when, tv);

		inc_log(LOG_DEBUG, "|%.4d | %-15p | %-15p | %.6ld : %.6ld |\n", 
			q->id,
			q->callback,
			q->data,
			delta.tv_sec,
			(long int)delta.tv_usec);
	}
	inc_log(LOG_DEBUG, "=============================================================\n");
	
}

/*! \brief
 * Launch all events which need to be run at this time.
 */
int inc_sched_runq(struct sched_context *con)
{
	struct sched *current;
	struct timeval tv;
	int numevents;
	int res;

	DEBUG(inc_log(LOG_DEBUG, "inc_sched_runq()\n"));
		
	inc_mutex_lock(&con->lock);

	for (numevents = 0; !INC_LIST_EMPTY(&con->schedq); numevents++) {
		/* schedule all events which are going to expire within 1ms.
		 * We only care about millisecond accuracy anyway, so this will
		 * help us get more than one event at one time if they are very
		 * close together.
		 */
		tv = inc_tvadd(inc_tvnow(), inc_tv(0, 1000));
		if (inc_tvcmp(INC_LIST_FIRST(&con->schedq)->when, tv) != -1)
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
			
		inc_mutex_unlock(&con->lock);
		res = current->callback(current->data);
		inc_mutex_lock(&con->lock);
			
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

	inc_mutex_unlock(&con->lock);
	
	return numevents;
}

long inc_sched_when(struct sched_context *con,int id)
{
	struct sched *s;
	long secs = -1;
	DEBUG(inc_log(LOG_DEBUG, "inc_sched_when()\n"));

	inc_mutex_lock(&con->lock);
	INC_LIST_TRAVERSE(&con->schedq, s, list) {
		if (s->id == id)
			break;
	}
	if (s) {
		struct timeval now = inc_tvnow();
		secs = s->when.tv_sec - now.tv_sec;
	}
	inc_mutex_unlock(&con->lock);
	
	return secs;
}
