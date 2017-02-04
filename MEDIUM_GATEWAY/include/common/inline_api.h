#ifndef __INC_CC_INLINEAPI_H
#define __INC_CC_INLINEAPI_H

/*! \file
 * \brief Inlinable API function macro

  Small API functions that are candidates for inlining need to be specially
  declared and defined, to ensure that the 'right thing' always happens.
  For example:
  	- there must _always_ be a non-inlined version of the function
	available for modules compiled out of the tree to link to
	- references to a function that cannot be inlined (for any
	reason that the compiler deems proper) must devolve into an
	'extern' reference, instead of 'static', so that multiple
	copies of the function body are not built in different modules
	- when LOW_MEMORY is defined, inlining should be disabled
	completely, even if the compiler is configured to support it

  The INC_INLINE_API macro allows this to happen automatically, when
  used to define your function. Proper usage is as follows:
  - define your function one place, in a header file, using the macro
  to wrap the function (see strings.h or time.h for examples)
  - choose a module to 'host' the function body for non-inline
  usages, and in that module _only_, define INC_API_MODULE before
  including the header file
 */

#if !defined(LOW_MEMORY)

#if !defined(INC_API_MODULE)
#define INC_INLINE_API(hdr, body) hdr; extern inline hdr body
#else
#define INC_INLINE_API(hdr, body) hdr; hdr body
#endif

#else /* defined(LOW_MEMORY) */

#if !defined(INC_API_MODULE)
#define INC_INLINE_API(hdr, body) hdr;
#else
#define INC_INLINE_API(hdr, body) hdr; hdr body
#endif

#endif

#undef INC_API_MODULE

#endif /* __INC_CC_INLINEAPI_H */
