/*
=======================================================
** FileName: MGW_time.c
** Description: 时间调度的相关处理
=======================================================
*/

#include "PUBLIC.h"

#define		PERIOD		10000


#define ONE_MILLION	1000000
/*
 * put timeval in a valid range. usec is 0..999999
 * negative values are not allowed and truncated.
 */
static struct timeval tvfix(struct timeval a)
{
	if (a.tv_usec >= ONE_MILLION) {
		VERBOSE_OUT(LOG_SYS, "warning too large timestamp %ld.%ld\n",
			a.tv_sec, (long int) a.tv_usec);
		a.tv_sec += a.tv_usec / ONE_MILLION;
		a.tv_usec %= ONE_MILLION;
	} else if (a.tv_usec < 0) {
		VERBOSE_OUT(LOG_SYS, "warning negative timestamp %ld.%ld\n",
			a.tv_sec, (long int) a.tv_usec);
		a.tv_usec = 0;
	}
	return a;
}

struct timeval mgw_tvadd(struct timeval a, struct timeval b)
{
	/* consistency checks to guarantee usec in 0..999999 */
	a = tvfix(a);
	b = tvfix(b);
	a.tv_sec += b.tv_sec;
	a.tv_usec += b.tv_usec;
	if (a.tv_usec >= ONE_MILLION) {
		a.tv_sec++;
		a.tv_usec -= ONE_MILLION;
	}
	return a;
}

struct timeval mgw_tvsub(struct timeval a, struct timeval b)
{
	/* consistency checks to guarantee usec in 0..999999 */
	a = tvfix(a);
	b = tvfix(b);
	a.tv_sec -= b.tv_sec;
	a.tv_usec -= b.tv_usec;
	if (a.tv_usec < 0) {
		a.tv_sec-- ;
		a.tv_usec += ONE_MILLION;
	}
	return a;
}
#undef ONE_MILLION

inline int mgw_tvdiff_ms(struct timeval end, struct timeval start)
{
	return  ((end.tv_sec - start.tv_sec) * 1000) +
		(((1000000 + end.tv_usec - start.tv_usec) / 1000) - 1000);
}


inline int mgw_tvzero(const struct timeval t)
{
	return (t.tv_sec == 0 && t.tv_usec == 0);
}

inline int mgw_tvcmp(struct timeval _a, struct timeval _b)
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

inline int mgw_tveq(struct timeval _a, struct timeval _b)
{
	return (_a.tv_sec == _b.tv_sec && _a.tv_usec == _b.tv_usec);
}


inline struct timeval mgw_tvnow(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t;
}

inline struct timeval mgw_tv(mgw_time_t sec, mgw_suseconds_t usec)
{
	struct timeval t;
	t.tv_sec = sec;
	t.tv_usec = usec;
	return t;
}


inline struct timeval mgw_samp2tv(unsigned int _nsamp, unsigned int _rate)
{
	return mgw_tv(_nsamp / _rate, ((_nsamp % _rate) * (4000000 / _rate)) / 4); /* this calculation is accurate up to 32000Hz. */
}



void *time_process(void *arg)
{
	/*时间处理，测试线程运行情况*/
	struct timeval tv_old,tv_new;
	while(1)
	{
		gettimeofday(&tv_new,NULL);
		if((mgw_tvdiff_ms(tv_new,tv_old)-PERIOD)>0)
		{
			memcpy(&tv_old,&tv_new,sizeof(tv_new));
			/*添加100ms时间处理函数*/
			DEBUG_OUT("%ds Arrive!\n",PERIOD/1000);
		}
		else
			sleep(1);
	}
	return NULL;
}


