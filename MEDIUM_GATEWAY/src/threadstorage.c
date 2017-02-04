/*! \file
 *
 * \brief Debugging support for thread-local-storage objects
 *
 */



#if defined(DEBUG_THREADLOCALS)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/logger.h"
#include "common/strings.h"
#include "common/utils.h"
#include "common/threadstorage.h"
#include "common/linkedlists.h"

struct tls_object {
	void *key;
	size_t size;
	const char *file;
	const char *function;
	unsigned int line;
	pthread_t thread;
	INC_LIST_ENTRY(tls_object) entry;
};

static INC_LIST_HEAD_NOLOCK_STATIC(tls_objects, tls_object);

/* Allow direct use of pthread_mutex_t and friends */
#undef pthread_mutex_t
#undef pthread_mutex_lock
#undef pthread_mutex_unlock
#undef pthread_mutex_init
#undef pthread_mutex_destroy

/*!
 * \brief lock for the tls_objects list
 *
 * \note We can not use an inc_mutex_t for this.  The reason is that this
 *       lock is used within the context of thread-local data destructors,
 *       and the inc_mutex_* API uses thread-local data.  Allocating more
 *       thread-local data at that point just causes a memory leak.
 */
static pthread_mutex_t threadstoragelock;

void __inc_threadstorage_object_add(void *key, size_t len, const char *file, const char *function, unsigned int line)
{
	struct tls_object *to;

	if (!(to = inc_calloc(1, sizeof(*to))))
		return;

	to->key = key;
	to->size = len;
	to->file = file;
	to->function = function;
	to->line = line;
	to->thread = pthread_self();

	pthread_mutex_lock(&threadstoragelock);
	INC_LIST_INSERT_TAIL(&tls_objects, to, entry);
	pthread_mutex_unlock(&threadstoragelock);
}

void __inc_threadstorage_object_remove(void *key)
{
	struct tls_object *to;

	pthread_mutex_lock(&threadstoragelock);
	INC_LIST_TRAVERSE_SAFE_BEGIN(&tls_objects, to, entry) {
		if (to->key == key) {
			INC_LIST_REMOVE_CURRENT(&tls_objects, entry);
			break;
		}
	}
	INC_LIST_TRAVERSE_SAFE_END;
	pthread_mutex_unlock(&threadstoragelock);
	if (to)
		free(to);
}

void __inc_threadstorage_object_replace(void *key_old, void *key_new, size_t len)
{
	struct tls_object *to;

	pthread_mutex_lock(&threadstoragelock);
	INC_LIST_TRAVERSE_SAFE_BEGIN(&tls_objects, to, entry) {
		if (to->key == key_old) {
			to->key = key_new;
			to->size = len;
			break;
		}
	}
	INC_LIST_TRAVERSE_SAFE_END;
	pthread_mutex_unlock(&threadstoragelock);
}

static int handle_show_allocations(int fd, int argc, char *argv[])
{
	char *fn = NULL;
	size_t len = 0;
	unsigned int count = 0;
	struct tls_object *to;

	if (argc > 3)
		fn = argv[3];

	pthread_mutex_lock(&threadstoragelock);

	INC_LIST_TRAVERSE(&tls_objects, to, entry) {
		if (fn && strcasecmp(to->file, fn))
			continue;

		inc_log(LOG_EVENT, "%10d bytes allocated in %20s at line %5d of %25s (thread %p)\n",
			(int) to->size, to->function, to->line, to->file, (void *) to->thread);
		len += to->size;
		count++;
	}

	pthread_mutex_unlock(&threadstoragelock);

	inc_log(LOG_EVENT, "%10d bytes allocated in %d allocation%s\n", (int) len, count, count > 1 ? "s" : "");
	
	return RESULT_SUCCESS;
}

static int handle_show_summary(int fd, int argc, char *argv[])
{
	char *fn = NULL;
	size_t len = 0;
	unsigned int count = 0;
	struct tls_object *to;
	struct file {
		const char *name;
		size_t len;
		unsigned int count;
		INC_LIST_ENTRY(file) entry;
	} *file;
	INC_LIST_HEAD_NOLOCK_STATIC(file_summary, file);

	if (argc > 3)
		fn = argv[3];

	pthread_mutex_lock(&threadstoragelock);

	INC_LIST_TRAVERSE(&tls_objects, to, entry) {
		if (fn && strcasecmp(to->file, fn))
			continue;

		INC_LIST_TRAVERSE(&file_summary, file, entry) {
			if ((!fn && (file->name == to->file)) || (fn && (file->name == to->function)))
				break;
		}

		if (!file) {
			file = alloca(sizeof(*file));
			memset(file, 0, sizeof(*file));
			file->name = fn ? to->function : to->file;
			INC_LIST_INSERT_TAIL(&file_summary, file, entry);
		}

		file->len += to->size;
		file->count++;
	}

	pthread_mutex_unlock(&threadstoragelock);
	
	INC_LIST_TRAVERSE(&file_summary, file, entry) {
		len += file->len;
		count += file->count;
		if (fn) {
			inc_log(LOG_EVENT, "%10d bytes in %d allocation%ss in function %s\n",
				(int) file->len, file->count, file->count > 1 ? "s" : "", file->name);
		} else {
			inc_log(LOG_EVENT, "%10d bytes in %d allocation%s in file %s\n",
				(int) file->len, file->count, file->count > 1 ? "s" : "", file->name);
		}
	}

	inc_log(LOG_EVENT, "%10d bytes allocated in %d allocation%s\n", (int) len, count, count > 1 ? "s" : "");
	
	return RESULT_SUCCESS;
}

void threadstorage_init(void)
{
	pthread_mutex_init(&threadstoragelock, NULL);
}

#else /* !defined(DEBUG_THREADLOCALS) */

void threadstorage_init(void)
{
}

#endif /* !defined(DEBUG_THREADLOCALS) */

