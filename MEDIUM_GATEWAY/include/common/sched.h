/*! \file
 * \brief Scheduler Routines (derived from cheops)
 */

#ifndef _INC_CC_SCHED_H
#define _INC_CC_SCHED_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*! \brief Max num of schedule structs
 * \note The max number of schedule structs to keep around
 * for use.  Undefine to disable schedule structure
 * caching. (Only disable this on very low memory
 * machines)
 */
#define SCHED_MAX_CACHE 128

/*! \brief a loop construct to ensure that
 * the scheduled task get deleted. The idea is that
 * if we loop attempting to remove the scheduled task,
 * then whatever callback had been running will complete
 * and reinsert the task into the scheduler.
 *
 * Since macro expansion essentially works like pass-by-name
 * parameter passing, this macro will still work correctly even
 * if the id of the task to delete changes. This holds as long as 
 * the name of the id which could change is passed to the macro 
 * and not a copy of the value of the id.
 */
#define INC_SCHED_DEL(sched, id) \
	({ \
		int _count = 0; \
		int _sched_res = -1; \
		while (id > -1 && (_sched_res = inc_sched_del(sched, id)) && ++_count < 10) \
			usleep(1); \
		if (_count == 10 && option_debug > 2) { \
			inc_log(LOG_DEBUG, "Unable to cancel schedule ID %d.\n", id); \
		} \
		id = -1; \
		(_sched_res); \
	})

#define INC_SCHED_DEL_SPINLOCK(sched, id, lock) \
	({ \
		int _count = 0; \
		int _sched_res = -1; \
		while (id > -1 && (_sched_res = inc_sched_del(sched, id)) && ++_count < 10) { \
			inc_mutex_unlock(lock); \
			usleep(1); \
			inc_mutex_lock(lock); \
		} \
		if (_count == 10 && option_debug > 2) { \
			inc_log(LOG_DEBUG, "Unable to cancel schedule ID %d.\n", id); \
		} \
		id = -1; \
		(_sched_res); \
	})

struct sched_context;

/*! \brief New schedule context
 * \note Create a scheduling context
 * \return Returns a malloc'd sched_context structure, NULL on failure
 */
struct sched_context *sched_context_create(void);

/*! \brief destroys a schedule context
 * Destroys (free's) the given sched_context structure
 * \param c Context to free
 * \return Returns 0 on success, -1 on failure
 */
void sched_context_destroy(struct sched_context *c);

/*! \brief callback for a cheops scheduler
 * A cheops scheduler callback takes a pointer with callback data and
 * \return returns a 0 if it should not be run again, or non-zero if it should be
 * rescheduled to run again
 */
typedef int (*inc_sched_cb)(const void *data);
#define INC_SCHED_CB(a) ((inc_sched_cb)(a))

/*! \brief Adds a scheduled event
 * Schedule an event to take place at some point in the future.  callback
 * will be called with data as the argument, when milliseconds into the
 * future (approximately)
 * If callback returns 0, no further events will be re-scheduled
 * \param con Scheduler context to add
 * \param when how many milliseconds to wait for event to occur
 * \param callback function to call when the amount of time expires
 * \param data data to pass to the callback
 * \return Returns a schedule item ID on success, -1 on failure
 */
int inc_sched_add(struct sched_context *con, int when, inc_sched_cb callback, const void *data);

/*!Adds a scheduled event with rescheduling support
 * \param con Scheduler context to add
 * \param when how many milliseconds to wait for event to occur
 * \param callback function to call when the amount of time expires
 * \param data data to pass to the callback
 * \param variable If true, the result value of callback function will be
 *       used for rescheduling
 * Schedule an event to take place at some point in the future.  Callback
 * will be called with data as the argument, when milliseconds into the
 * future (approximately)
 * If callback returns 0, no further events will be re-scheduled
 * \return Returns a schedule item ID on success, -1 on failure
 */
int inc_sched_add_variable(struct sched_context *con, int when, inc_sched_cb callback, const void *data, int variable);

/*! \brief Deletes a scheduled event
 * Remove this event from being run.  A procedure should not remove its
 * own event, but return 0 instead.
 * \param con scheduling context to delete item from
 * \param id ID of the scheduled item to delete
 * \return Returns 0 on success, -1 on failure
 */
#ifndef INC_DEVMODE
int inc_sched_del(struct sched_context *con, int id);
#else
int _inc_sched_del(struct sched_context *con, int id, const char *file, int line, const char *function);
#define	inc_sched_del(a, b)	_inc_sched_del(a, b, __FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

/*! \brief Determines number of seconds until the next outstanding event to take place
 * Determine the number of seconds until the next outstanding event
 * should take place, and return the number of milliseconds until
 * it needs to be run.  This value is perfect for passing to the poll
 * call.
 * \param con context to act upon
 * \return Returns "-1" if there is nothing there are no scheduled events
 * (and thus the poll should not timeout)
 */
int inc_sched_wait(struct sched_context *con);

/*! \brief Runs the queue
 * \param con Scheduling context to run
 * Run the queue, executing all callbacks which need to be performed
 * at this time.
 * \param con context to act upon
 * \return Returns the number of events processed.
 */
int inc_sched_runq(struct sched_context *con);

/*! \brief Dumps the scheduler contents
 * Debugging: Dump the contents of the scheduler to stderr
 * \param con Context to dump
 */
void inc_sched_dump(const struct sched_context *con);

/*! \brief Returns the number of seconds before an event takes place
 * \param con Context to use
 * \param id Id to dump
 */
long inc_sched_when(struct sched_context *con,int id);

/*!
 * \brief Convenience macro for objects and reference (add)
 *
 */
#define inc_sched_add_object(obj,con,when,callback) inc_sched_add((con),(when),(callback), INCOBJ_REF((obj)))

/*!
 * \brief Convenience macro for objects and reference (del)
 *
 */
#define inc_sched_del_object(obj,destructor,con,id) do { \
	if ((id) > -1) { \
		inc_sched_del((con),(id)); \
		(id) = -1; \
		INCOBJ_UNREF((obj),(destructor)); \
	} \
} while(0)

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _INC_CC_SCHED_H */
