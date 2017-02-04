/*! \file
 * \brief Compiler-specific macros and other items
 */

#ifndef _INC_CC_COMPILER_H
#define _INC_CC_COMPILER_H

#ifdef HAVE_ATTRIBUTE_always_inline
#define force_inline __attribute__((always_inline)) inline
#else
#define force_inline inline
#endif

#ifdef HAVE_ATTRIBUTE_pure
#define attribute_pure __attribute__((pure))
#else
#define attribute_pure
#endif

#ifdef HAVE_ATTRIBUTE_const
#define attribute_const __attribute__((const))
#else
#define attribute_const
#endif

#ifdef HAVE_ATTRIBUTE_unused
#define attribute_unused __attribute__((unused))
#else
#define attribute_unused
#endif

#ifdef HAVE_ATTRIBUTE_malloc
#define attribute_malloc __attribute__((malloc))
#else
#define attribute_malloc
#endif

#ifdef HAVE_ATTRIBUTE_deprecated
#define attribute_deprecated __attribute__((deprecated))
#else
#define attribute_deprecated
#endif

#endif /* _INC_CC_COMPILER_H */
