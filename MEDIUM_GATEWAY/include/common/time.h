/*! \file
 * \brief Time-related functions and macros
 */

#ifndef _INC_CC_TIME_H
#define _INC_CC_TIME_H

#include <sys/time.h>
#include <stdlib.h>

#include "common/inline_api.h"

/* We have to let the compiler learn what types to use for the elements of a
   struct timeval since on linux, it's time_t and suseconds_t, but on *BSD,
   they are just a long. */
extern struct timeval tv;
typedef typeof(tv.tv_sec) inc_time_t;
typedef typeof(tv.tv_usec) inc_suseconds_t;

/*!
 * \brief Computes the difference (in milliseconds) between two \c struct \c timeval instances.
 * \param end end of the time period
 * \param start beginning of the time period
 * \return the difference in milliseconds
 */
INC_INLINE_API(
int inc_tvdiff_ms(struct timeval end, struct timeval start),
{
	/* the offset by 1,000,000 below is intentional...
	   it avoids differences in the way that division
	   is handled for positive and negative numbers, by ensuring
	   that the divisor is always positive
	*/
	return  ((end.tv_sec - start.tv_sec) * 1000) +
		(((1000000 + end.tv_usec - start.tv_usec) / 1000) - 1000);
}
)

/*!
 * \brief Returns true if the argument is 0,0
 */
INC_INLINE_API(
int inc_tvzero(const struct timeval t),
{
	return (t.tv_sec == 0 && t.tv_usec == 0);
}
)

/*!
 * \brief Compres two \c struct \c timeval instances returning
 * -1, 0, 1 if the first arg is smaller, equal or greater to the second.
 */
INC_INLINE_API(
int inc_tvcmp(struct timeval _a, struct timeval _b),
{
	if (_a.tv_sec < _b.tv_sec)
		return -1;
	if (_a.tv_sec > _b.tv_sec)
		return 1;
	/* now seconds are equal */
	if (_a.tv_usec < _b.tv_usec)
		return -1;
	if (_a.tv_usec > _b.tv_usec)
		return 1;
	return 0;
}
)

/*!
 * \brief Returns true if the two \c struct \c timeval arguments are equal.
 */
INC_INLINE_API(
int inc_tveq(struct timeval _a, struct timeval _b),
{
	return (_a.tv_sec == _b.tv_sec && _a.tv_usec == _b.tv_usec);
}
)

/*!
 * \brief Returns current timeval. Meant to replace calls to gettimeofday().
 */
INC_INLINE_API(
struct timeval inc_tvnow(void),
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t;
}
)

/*!
 * \brief Returns the sum of two timevals a + b
 */
struct timeval inc_tvadd(struct timeval a, struct timeval b);

/*!
 * \brief Returns the difference of two timevals a - b
 */
struct timeval inc_tvsub(struct timeval a, struct timeval b);

/*!
 * \brief Returns a timeval from sec, usec
 */
INC_INLINE_API(
struct timeval inc_tv(inc_time_t sec, inc_suseconds_t usec),
{
	struct timeval t;
	t.tv_sec = sec;
	t.tv_usec = usec;
	return t;
}
)

/*!
 * \brief Returns a timeval corresponding to the duration of n samples at rate r.
 * Useful to convert samples to timevals, or even milliseconds to timevals
 * in the form inc_samp2tv(milliseconds, 1000)
 */
INC_INLINE_API(
struct timeval inc_samp2tv(unsigned int _nsamp, unsigned int _rate),
{
	return inc_tv(_nsamp / _rate, ((_nsamp % _rate) * (4000000 / _rate)) / 4); /* this calculation is accurate up to 32000Hz. */
}
)

#endif /* _INC_CC_TIME_H */
