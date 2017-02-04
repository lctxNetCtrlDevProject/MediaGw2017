/*
 * Object Model for INC CC
 */

#ifndef _INC_CC_INCOBJ_H
#define _INC_CC_INCOBJ_H

#include <string.h>

#include "common/lock.h"
#include "common/compiler.h"

/*! \file
 * \brief A set of macros implementing objects and containers.
 * Macros are used for maximum performance, to support multiple inheritance,
 * and to be easily integrated into existing structures without additional
 * malloc calls, etc.
 *
 * These macros expect to operate on two different object types, INCOBJs and
 * INCOBJ_CONTAINERs.  These are not actual types, as any struct can be
 * converted into an INCOBJ compatible object or container using the supplied
 * macros.
 *
 * <b>Sample Usage:</b>
 * \code
 * struct sample_object {
 *    INCOBJ_COMPONENTS(struct sample_object);
 * };
 *
 * struct sample_container {
 *    INCOBJ_CONTAINER_COMPONENTS(struct sample_object);
 * } super_container;
 *
 * void sample_object_destroy(struct sample_object *obj)
 * {
 *    free(obj);
 * }
 *
 * int init_stuff()
 * {
 *    struct sample_object *obj1;
 *    struct sample_object *found_obj;
 *
 *    obj1 = malloc(sizeof(struct sample_object));
 *
 *    INCOBJ_CONTAINER_INIT(&super_container);
 *
 *    INCOBJ_INIT(obj1);
 *    INCOBJ_WRLOCK(obj1);
 *    inc_copy_string(obj1->name, "obj1", sizeof(obj1->name));
 *    INCOBJ_UNLOCK(obj1);
 *
 *    INCOBJ_CONTAINER_LINK(&super_container, obj1);
 *
 *    found_obj = INCOBJ_CONTAINER_FIND(&super_container, "obj1");
 *
 *    if(found_obj) {
 *       printf("Found object: %s", found_obj->name); 
 *       INCOBJ_UNREF(found_obj,sample_object_destroy);
 *    }
 *
 *    INCOBJ_CONTAINER_DESTROYALL(&super_container,sample_object_destroy);
 *    INCOBJ_CONTAINER_DESTROY(&super_container);
 * 
 *    return 0;
 * }
 * \endcode
 */

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define INCOBJ_DEFAULT_NAMELEN 	80
#define INCOBJ_DEFAULT_BUCKETS	256
#define INCOBJ_DEFAULT_HASH		inc_strhash

#define INCOBJ_FLAG_MARKED	(1 << 0)		/* Object has been marked for future operation */

/* C++ is simply a syntactic crutch for those who cannot think for themselves
   in an object oriented way. */

/*! \brief Lock an INCOBJ for reading.
 */
#define INCOBJ_RDLOCK(object) inc_mutex_lock(&(object)->_lock)

/*! \brief Lock an INCOBJ for writing.
 */
#define INCOBJ_WRLOCK(object) inc_mutex_lock(&(object)->_lock)

#define INCOBJ_TRYWRLOCK(object) inc_mutex_trylock(&(object)->_lock)

/*! \brief Unlock a locked object. */
#define INCOBJ_UNLOCK(object) inc_mutex_unlock(&(object)->_lock)

#ifdef INCOBJ_CONTAINER_HASHMODEL 
#define __INCOBJ_HASH(type,hashes) \
	type *next[hashes] 
#else 
#define __INCOBJ_HASH(type,hashes) \
	type *next[1] 
#endif	

/*! \brief Add INCOBJ components to a struct (without locking support).
 *
 * \param type The datatype of the object.
 * \param namelen The length to make the name char array.
 * \param hashes The number of containers the object can be present in.
 *
 * This macro adds components to a struct to make it an INCOBJ.  This macro
 * differs from INCOBJ_COMPONENTS_FULL in that it does not create a mutex for
 * locking.
 *
 * <b>Sample Usage:</b>
 * \code
 * struct sample_struct {
 *    INCOBJ_COMPONENTS_NOLOCK_FULL(struct sample_struct,1,1);
 * };
 * \endcode
 */
#define INCOBJ_COMPONENTS_NOLOCK_FULL(type,namelen,hashes) \
	char name[namelen]; \
	unsigned int refcount; \
	unsigned int objflags; \
	__INCOBJ_HASH(type,hashes)
	
/*! \brief Add INCOBJ components to a struct (without locking support).
 *
 * \param type The datatype of the object.
 *
 * This macro works like #INCOBJ_COMPONENTS_NOLOCK_FULL() except it only accepts a
 * type and uses default values for namelen and hashes.
 * 
 * <b>Sample Usage:</b>
 * \code
 * struct sample_struct_componets {
 *    INCOBJ_COMPONENTS_NOLOCK(struct sample_struct);
 * };
 * \endcode
 */
#define INCOBJ_COMPONENTS_NOLOCK(type) \
	INCOBJ_COMPONENTS_NOLOCK_FULL(type,INCOBJ_DEFAULT_NAMELEN,1)

/*! \brief Add INCOBJ components to a struct (with locking support).
 *
 * \param type The datatype of the object.
 *
 * This macro works like #INCOBJ_COMPONENTS_NOLOCK() except it includes locking
 * support.
 *
 * <b>Sample Usage:</b>
 * \code
 * struct sample_struct {
 *    INCOBJ_COMPONENTS(struct sample_struct);
 * };
 * \endcode
 */
#define INCOBJ_COMPONENTS(type) \
	INCOBJ_COMPONENTS_NOLOCK(type); \
	inc_mutex_t _lock; 
	
/*! \brief Add INCOBJ components to a struct (with locking support).
 *
 * \param type The datatype of the object.
 * \param namelen The length to make the name char array.
 * \param hashes The number of containers the object can be present in.
 *
 * This macro adds components to a struct to make it an INCOBJ and includes
 * support for locking.
 *
 * <b>Sample Usage:</b>
 * \code
 * struct sample_struct {
 *    INCOBJ_COMPONENTS_FULL(struct sample_struct,1,1);
 * };
 * \endcode
 */
#define INCOBJ_COMPONENTS_FULL(type,namelen,hashes) \
	INCOBJ_COMPONENTS_NOLOCK_FULL(type,namelen,hashes); \
	inc_mutex_t _lock; 

/*! \brief Increment an object reference count.
 * \param object A pointer to the object to operate on.
 * \return The object.
 */
#define INCOBJ_REF(object) \
	({ \
		INCOBJ_WRLOCK(object); \
		(object)->refcount++; \
		INCOBJ_UNLOCK(object); \
		(object); \
	})
	
/*! \brief Decrement the reference count on an object.
 *
 * \param object A pointer the object to operate on.
 * \param destructor The destructor to call if the object is no longer referenced.  It will be passed the pointer as an argument.
 *
 * This macro unreferences an object and calls the specfied destructor if the
 * object is no longer referenced.  The destructor should free the object if it
 * was dynamically allocated.
 */
#define INCOBJ_UNREF(object,destructor) \
	do { \
		int newcount = 0; \
		INCOBJ_WRLOCK(object); \
		if (__builtin_expect((object)->refcount > 0, 1)) \
			newcount = --((object)->refcount); \
		else \
			inc_log(LOG_WARNING, "Unreferencing unreferenced (object)!\n"); \
		INCOBJ_UNLOCK(object); \
		if (newcount == 0) { \
			inc_mutex_destroy(&(object)->_lock); \
			destructor((object)); \
		} \
		(object) = NULL; \
	} while(0)

/*! \brief Mark an INCOBJ by adding the #INCOBJ_FLAG_MARKED flag to its objflags mask. 
 * \param object A pointer to the object to operate on.
 *
 * This macro "marks" an object.  Marked objects can later be unlinked from a container using
 * #INCOBJ_CONTAINER_PRUNE_MARKED().
 * 
 */
#define INCOBJ_MARK(object) \
	do { \
		INCOBJ_WRLOCK(object); \
		(object)->objflags |= INCOBJ_FLAG_MARKED; \
		INCOBJ_UNLOCK(object); \
	} while(0)
	
/*! \brief Unmark an INCOBJ by subtracting the #INCOBJ_FLAG_MARKED flag from its objflags mask.
 * \param object A pointer to the object to operate on.
 */
#define INCOBJ_UNMARK(object) \
	do { \
		INCOBJ_WRLOCK(object); \
		(object)->objflags &= ~INCOBJ_FLAG_MARKED; \
		INCOBJ_UNLOCK(object); \
	} while(0)

/*! \brief Initialize an object.
 * \param object A pointer to the object to operate on.
 *
 * \note This should only be used on objects that support locking (objects
 * created with #INCOBJ_COMPONENTS() or #INCOBJ_COMPONENTS_FULL())
 */
#define INCOBJ_INIT(object) \
	do { \
		inc_mutex_init(&(object)->_lock); \
		object->name[0] = '\0'; \
		object->refcount = 1; \
	} while(0)

/* Containers for objects -- current implementation is linked lists, but
   should be able to be converted to hashes relatively easily */

/*! \brief Lock an INCOBJ_CONTAINER for reading.
 */
#define INCOBJ_CONTAINER_RDLOCK(container) inc_mutex_lock(&(container)->_lock)

/*! \brief Lock an INCOBJ_CONTAINER for writing. 
 */
#define INCOBJ_CONTAINER_WRLOCK(container) inc_mutex_lock(&(container)->_lock)

/*! \brief Unlock an INCOBJ_CONTAINER. */
#define INCOBJ_CONTAINER_UNLOCK(container) inc_mutex_unlock(&(container)->_lock)

#ifdef INCOBJ_CONTAINER_HASHMODEL
#error "Hash model for object containers not yet implemented!"
#else
/* Linked lists */

/*! \brief Create a container for INCOBJs (without locking support).
 *
 * \param type The type of objects the container will hold.
 * \param hashes Currently unused.
 * \param buckets Currently unused.
 *
 * This macro is used to create a container for INCOBJs without locking
 * support.
 *
 * <b>Sample Usage:</b>
 * \code
 * struct sample_struct_nolock_container {
 *    INCOBJ_CONTAINER_COMPONENTS_NOLOCK_FULL(struct sample_struct,1,1);
 * };
 * \endcode
 */
#define INCOBJ_CONTAINER_COMPONENTS_NOLOCK_FULL(type,hashes,buckets) \
	type *head

/*! \brief Initialize a container.
 *
 * \param container A pointer to the container to initialize.
 * \param hashes Currently unused.
 * \param buckets Currently unused.
 *
 * This macro initializes a container.  It should only be used on containers
 * that support locking.
 * 
 * <b>Sample Usage:</b>
 * \code
 * struct sample_struct_container {
 *    INCOBJ_CONTAINER_COMPONENTS_FULL(struct sample_struct,1,1);
 * } container;
 *
 * int func()
 * {
 *    INCOBJ_CONTAINER_INIT_FULL(&container,1,1);
 * }
 * \endcode
 */
#define INCOBJ_CONTAINER_INIT_FULL(container,hashes,buckets) \
	do { \
		inc_mutex_init(&(container)->_lock); \
	} while(0)
	
/*! \brief Destroy a container.
 *
 * \param container A pointer to the container to destroy.
 * \param hashes Currently unused.
 * \param buckets Currently unused.
 *
 * This macro frees up resources used by a container.  It does not operate on
 * the objects in the container.  To unlink the objects from the container use
 * #INCOBJ_CONTAINER_DESTROYALL().
 *
 * \note This macro should only be used on containers with locking support.
 */
#define INCOBJ_CONTAINER_DESTROY_FULL(container,hashes,buckets) \
	do { \
		inc_mutex_destroy(&(container)->_lock); \
	} while(0)

/*! \brief Iterate through the objects in a container.
 *
 * \param container A pointer to the container to traverse.
 * \param continue A condition to allow the traversal to continue.
 * \param eval A statement to evaluate in the iteration loop.
 *
 * This is macro is a little complicated, but it may help to think of it as a
 * loop.  Basically it iterates through the specfied containter as long as the
 * condition is met.  Two variables, iterator and next, are provided for use in
 * your \p eval statement.  See the sample code for an example.
 *
 * <b>Sample Usage:</b>
 * \code
 * INCOBJ_CONTAINER_TRAVERSE(&sample_container,1, {
 *    INCOBJ_RDLOCK(iterator);
 *    printf("Currently iterating over '%s'\n", iterator->name);
 *    INCOBJ_UNLOCK(iterator);
 * } );
 * \endcode
 *
 * \code
 * INCOBJ_CONTAINER_TRAVERSE(&sample_container,1, sample_func(iterator));
 * \endcode
 */
#define INCOBJ_CONTAINER_TRAVERSE(container,continue,eval) \
	do { \
		typeof((container)->head) iterator; \
		typeof((container)->head) next; \
		INCOBJ_CONTAINER_RDLOCK(container); \
		next = (container)->head; \
		while((continue) && (iterator = next)) { \
			next = iterator->next[0]; \
			eval; \
		} \
		INCOBJ_CONTAINER_UNLOCK(container); \
	} while(0)

/*! \brief Find an object in a container.
 *
 * \param container A pointer to the container to search.
 * \param namestr The name to search for.
 *
 * Use this function to find an object with the specfied name in a container.
 *
 * \note When the returned object is no longer in use, #INCOBJ_UNREF() should
 * be used to free the additional reference created by this macro.
 *
 * \return A new reference to the object located or NULL if nothing is found.
 */
#define INCOBJ_CONTAINER_FIND(container,namestr) \
	({ \
		typeof((container)->head) found = NULL; \
		INCOBJ_CONTAINER_TRAVERSE(container, !found, do { \
			if (!(strcasecmp(iterator->name, (namestr)))) \
				found = INCOBJ_REF(iterator); \
		} while (0)); \
		found; \
	})

/*! \brief Find an object in a container.
 * 
 * \param container A pointer to the container to search.
 * \param data The data to search for.
 * \param field The field/member of the container's objects to search.
 * \param hashfunc The hash function to use, currently not implemented.
 * \param hashoffset The hash offset to use, currently not implemented.
 * \param comparefunc The function used to compare the field and data values.
 *
 * This macro iterates through a container passing the specified field and data
 * elements to the specified comparefunc.  The function should return 0 when a match is found.
 * 
 * \note When the returned object is no longer in use, #INCOBJ_UNREF() should
 * be used to free the additional reference created by this macro.
 *
 * \return A pointer to the object located or NULL if nothing is found.
 */
#define INCOBJ_CONTAINER_FIND_FULL(container,data,field,hashfunc,hashoffset,comparefunc) \
	({ \
		typeof((container)->head) found = NULL; \
		INCOBJ_CONTAINER_TRAVERSE(container, !found, do { \
			INCOBJ_RDLOCK(iterator); \
			if (!(comparefunc(iterator->field, (data)))) { \
				found = (iterator); \
			} \
			INCOBJ_UNLOCK(iterator); \
		} while (0)); \
		found; \
	})

/*! \brief Empty a container.
 *
 * \param container A pointer to the container to operate on.
 * \param destructor A destructor function to call on each object.
 *
 * This macro loops through a container removing all the items from it using
 * #INCOBJ_UNREF().  This does not destroy the container itself, use
 * #INCOBJ_CONTAINER_DESTROY() for that.
 *
 * \note If any object in the container is only referenced by the container,
 * the destructor will be called for that object once it has been removed.
 */
#define INCOBJ_CONTAINER_DESTROYALL(container,destructor) \
	do { \
		typeof((container)->head) iterator; \
		INCOBJ_CONTAINER_WRLOCK(container); \
		while((iterator = (container)->head)) { \
			(container)->head = (iterator)->next[0]; \
			INCOBJ_UNREF(iterator,destructor); \
		} \
		INCOBJ_CONTAINER_UNLOCK(container); \
	} while(0)

/*! \brief Remove an object from a container.
 *
 * \param container A pointer to the container to operate on.
 * \param obj A pointer to the object to remove.
 *
 * This macro iterates through a container and removes the specfied object if
 * it exists in the container.
 *
 * \note This macro does not destroy any objects, it simply unlinks
 * them from the list.  No destructors are called.
 *
 * \return The container's reference to the removed object or NULL if no
 * matching object was found.
 */
#define INCOBJ_CONTAINER_UNLINK(container,obj) \
	({ \
		typeof((container)->head) found = NULL; \
		typeof((container)->head) prev = NULL; \
		INCOBJ_CONTAINER_TRAVERSE(container, !found, do { \
			if (iterator == obj) { \
				found = iterator; \
				found->next[0] = NULL; \
				INCOBJ_CONTAINER_WRLOCK(container); \
				if (prev) \
					prev->next[0] = next; \
				else \
					(container)->head = next; \
				INCOBJ_CONTAINER_UNLOCK(container); \
			} \
			prev = iterator; \
		} while (0)); \
		found; \
	})

/*! \brief Find and remove an object from a container.
 * 
 * \param container A pointer to the container to operate on.
 * \param namestr The name of the object to remove.
 *
 * This macro iterates through a container and removes the first object with
 * the specfied name from the container.
 *
 * \note This macro does not destroy any objects, it simply unlinks
 * them.  No destructors are called.
 *
 * \return The container's reference to the removed object or NULL if no
 * matching object was found.
 */
#define INCOBJ_CONTAINER_FIND_UNLINK(container,namestr) \
	({ \
		typeof((container)->head) found = NULL; \
		typeof((container)->head) prev = NULL; \
		INCOBJ_CONTAINER_TRAVERSE(container, !found, do { \
			if (!(strcasecmp(iterator->name, (namestr)))) { \
				found = iterator; \
				found->next[0] = NULL; \
				INCOBJ_CONTAINER_WRLOCK(container); \
				if (prev) \
					prev->next[0] = next; \
				else \
					(container)->head = next; \
				INCOBJ_CONTAINER_UNLOCK(container); \
			} \
			prev = iterator; \
		} while (0)); \
		found; \
	})

/*! \brief Find and remove an object in a container.
 * 
 * \param container A pointer to the container to search.
 * \param data The data to search for.
 * \param field The field/member of the container's objects to search.
 * \param hashfunc The hash function to use, currently not implemented.
 * \param hashoffset The hash offset to use, currently not implemented.
 * \param comparefunc The function used to compare the field and data values.
 *
 * This macro iterates through a container passing the specified field and data
 * elements to the specified comparefunc.  The function should return 0 when a match is found.
 * If a match is found it is removed from the list. 
 *
 * \note This macro does not destroy any objects, it simply unlinks
 * them.  No destructors are called.
 *
 * \return The container's reference to the removed object or NULL if no match
 * was found.
 */
#define INCOBJ_CONTAINER_FIND_UNLINK_FULL(container,data,field,hashfunc,hashoffset,comparefunc) \
	({ \
		typeof((container)->head) found = NULL; \
		typeof((container)->head) prev = NULL; \
		INCOBJ_CONTAINER_TRAVERSE(container, !found, do { \
			INCOBJ_RDLOCK(iterator); \
			if (!(comparefunc(iterator->field, (data)))) { \
				found = iterator; \
				found->next[0] = NULL; \
				INCOBJ_CONTAINER_WRLOCK(container); \
				if (prev) \
					prev->next[0] = next; \
				else \
					(container)->head = next; \
				INCOBJ_CONTAINER_UNLOCK(container); \
			} \
			INCOBJ_UNLOCK(iterator); \
			prev = iterator; \
		} while (0)); \
		found; \
	})

/*! \brief Add an object to the end of a container.
 *
 * \param container A pointer to the container to operate on.
 * \param newobj A pointer to the object to be added.
 *
 * This macro adds an object to the end of a container.
 */
#define INCOBJ_CONTAINER_LINK_END(container,newobj) \
	do { \
		typeof((container)->head) iterator; \
		typeof((container)->head) next; \
		typeof((container)->head) prev; \
		INCOBJ_CONTAINER_RDLOCK(container); \
		prev = NULL; \
		next = (container)->head; \
		while((iterator = next)) { \
			next = iterator->next[0]; \
			prev = iterator; \
		} \
		if(prev) { \
			INCOBJ_CONTAINER_WRLOCK((container)); \
			prev->next[0] = INCOBJ_REF(newobj); \
			(newobj)->next[0] = NULL; \
			INCOBJ_CONTAINER_UNLOCK((container)); \
		} else { \
			INCOBJ_CONTAINER_LINK_START((container),(newobj)); \
		} \
		INCOBJ_CONTAINER_UNLOCK((container)); \
	} while(0)

/*! \brief Add an object to the front of a container.
 *
 * \param container A pointer to the container to operate on.
 * \param newobj A pointer to the object to be added.
 *
 * This macro adds an object to the start of a container.
 */
#define INCOBJ_CONTAINER_LINK_START(container,newobj) \
	do { \
		INCOBJ_CONTAINER_WRLOCK(container); \
		(newobj)->next[0] = (container)->head; \
		(container)->head = INCOBJ_REF(newobj); \
		INCOBJ_CONTAINER_UNLOCK(container); \
	} while(0)

/*! \brief Remove an object from the front of a container.
 *
 * \param container A pointer to the container to operate on.
 *
 * This macro removes the first object in a container.
 *
 * \note This macro does not destroy any objects, it simply unlinks
 * them from the list.  No destructors are called.
 *
 * \return The container's reference to the removed object or NULL if no
 * matching object was found.
 */
#define INCOBJ_CONTAINER_UNLINK_START(container) \
	({ \
		typeof((container)->head) found = NULL; \
		INCOBJ_CONTAINER_WRLOCK(container); \
	 	if((container)->head) { \
	 		found = (container)->head; \
	 		(container)->head = (container)->head->next[0]; \
	 		found->next[0] = NULL; \
	 	} \
		INCOBJ_CONTAINER_UNLOCK(container); \
	 	found; \
	})

/*! \brief Prune marked objects from a container.
 *
 * \param container A pointer to the container to prune.
 * \param destructor A destructor function to call on each marked object.
 * 
 * This macro iterates through the specfied container and prunes any marked
 * objects executing the specfied destructor if necessary.
 */
#define INCOBJ_CONTAINER_PRUNE_MARKED(container,destructor) \
	do { \
		typeof((container)->head) prev = NULL; \
		INCOBJ_CONTAINER_TRAVERSE(container, 1, do { \
			INCOBJ_RDLOCK(iterator); \
			if (iterator->objflags & INCOBJ_FLAG_MARKED) { \
				INCOBJ_CONTAINER_WRLOCK(container); \
				if (prev) \
					prev->next[0] = next; \
				else \
					(container)->head = next; \
				INCOBJ_CONTAINER_UNLOCK(container); \
				INCOBJ_UNLOCK(iterator); \
				INCOBJ_UNREF(iterator,destructor); \
				continue; \
			} \
			INCOBJ_UNLOCK(iterator); \
			prev = iterator; \
		} while (0)); \
	} while(0)

/*! \brief Add an object to a container.
 *
 * \param container A pointer to the container to operate on.
 * \param newobj A pointer to the object to be added.
 * \param data Currently unused.
 * \param field Currently unused.
 * \param hashfunc Currently unused.
 * \param hashoffset Currently unused.
 * \param comparefunc Currently unused.
 *
 * Currently this function adds an object to the head of the list.  One day it
 * will support adding objects atthe position specified using the various
 * options this macro offers.
 */
#define INCOBJ_CONTAINER_LINK_FULL(container,newobj,data,field,hashfunc,hashoffset,comparefunc) \
	do { \
		INCOBJ_CONTAINER_WRLOCK(container); \
		(newobj)->next[0] = (container)->head; \
		(container)->head = INCOBJ_REF(newobj); \
		INCOBJ_CONTAINER_UNLOCK(container); \
	} while(0)

#endif /* List model */

/* Common to hash and linked list models */

/*! \brief Create a container for INCOBJs (without locking support).
 *
 * \param type The type of objects the container will hold.
 *
 * This macro is used to create a container for INCOBJs without locking
 * support.
 *
 * <b>Sample Usage:</b>
 * \code
 * struct sample_struct_nolock_container {
 *    INCOBJ_CONTAINER_COMPONENTS_NOLOCK(struct sample_struct);
 * };
 * \endcode
 */
#define INCOBJ_CONTAINER_COMPONENTS_NOLOCK(type) \
	INCOBJ_CONTAINER_COMPONENTS_NOLOCK_FULL(type,1,INCOBJ_DEFAULT_BUCKETS)


/*! \brief Create a container for INCOBJs (with locking support).
 *
 * \param type The type of objects the container will hold.
 *
 * This macro is used to create a container for INCOBJs with locking support.
 *
 * <b>Sample Usage:</b>
 * \code
 * struct sample_struct_container {
 *    INCOBJ_CONTAINER_COMPONENTS(struct sample_struct);
 * };
 * \endcode
 */
#define INCOBJ_CONTAINER_COMPONENTS(type) \
	inc_mutex_t _lock; \
	INCOBJ_CONTAINER_COMPONENTS_NOLOCK(type)

/*! \brief Initialize a container.
 *
 * \param container A pointer to the container to initialize.
 *
 * This macro initializes a container.  It should only be used on containers
 * that support locking.
 * 
 * <b>Sample Usage:</b>
 * \code
 * struct sample_struct_container {
 *    INCOBJ_CONTAINER_COMPONENTS(struct sample_struct);
 * } container;
 *
 * int func()
 * {
 *    INCOBJ_CONTAINER_INIT(&container);
 * }
 * \endcode
 */
#define INCOBJ_CONTAINER_INIT(container) \
	INCOBJ_CONTAINER_INIT_FULL(container,1,INCOBJ_DEFAULT_BUCKETS)

/*! \brief Destroy a container.
 *
 * \param container A pointer to the container to destory.
 *
 * This macro frees up resources used by a container.  It does not operate on
 * the objects in the container.  To unlink the objects from the container use
 * #INCOBJ_CONTAINER_DESTROYALL().
 *
 * \note This macro should only be used on containers with locking support.
 */
#define INCOBJ_CONTAINER_DESTROY(container) \
	INCOBJ_CONTAINER_DESTROY_FULL(container,1,INCOBJ_DEFAULT_BUCKETS)

/*! \brief Add an object to a container.
 *
 * \param container A pointer to the container to operate on.
 * \param newobj A pointer to the object to be added.
 *
 * Currently this macro adds an object to the head of a container.  One day it
 * should add an object in alphabetical order.
 */
#define INCOBJ_CONTAINER_LINK(container,newobj) \
	INCOBJ_CONTAINER_LINK_FULL(container,newobj,(newobj)->name,name,INCOBJ_DEFAULT_HASH,0,strcasecmp)

/*! \brief Mark all the objects in a container.
 * \param container A pointer to the container to operate on.
 */
#define INCOBJ_CONTAINER_MARKALL(container) \
	INCOBJ_CONTAINER_TRAVERSE(container, 1, INCOBJ_MARK(iterator))

/*! \brief Unmark all the objects in a container.
 * \param container A pointer to the container to operate on.
 */
#define INCOBJ_CONTAINER_UNMARKALL(container) \
	INCOBJ_CONTAINER_TRAVERSE(container, 1, INCOBJ_UNMARK(iterator))

/*! \brief Dump information about an object into a string.
 *
 * \param s A pointer to the string buffer to use.
 * \param slen The length of s.
 * \param obj A pointer to the object to dump.
 *
 * This macro dumps a text representation of the name, objectflags, and
 * refcount fields of an object to the specfied string buffer.
 */
#define INCOBJ_DUMP(s,slen,obj) \
	snprintf((s),(slen),"name: %s\nobjflags: %d\nrefcount: %d\n\n", (obj)->name, (obj)->objflags, (obj)->refcount);

/*! \brief Dump information about all the objects in a container to a file descriptor.
 *
 * \param fd The file descriptor to write to.
 * \param s A string buffer, same as #INCOBJ_DUMP().
 * \param slen The length of s, same as #INCOBJ_DUMP().
 * \param container A pointer to the container to dump.
 *
 * This macro dumps a text representation of the name, objectflags, and
 * refcount fields of all the objects in a container to the specified file
 * descriptor.
 */
#define INCOBJ_CONTAINER_DUMP(fd,s,slen,container) \
	INCOBJ_CONTAINER_TRAVERSE(container, 1, do { INCOBJ_DUMP(s,slen,iterator); inc_log(LOG_DEBUG, "%s", s); } while(0))

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _INC_CC_INCOBJ_H */
