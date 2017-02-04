/*
=========================================================
** FileName:	MGW_time.h
** Description: 时间相关函数，类型定义，与SCHED配合，
** 组成简单调度器
=========================================================
*/
#ifndef _MGW_TIME_H_
#define _MGW_TIME_H_

#if defined(__cplusplus)||defined(c_plusplus)
extern "C"
{
#endif

#include <sys/time.h>
#include <stdlib.h>

/*type define*/
extern struct timeval tv;
typedef typeof(tv.tv_sec) mgw_time_t;
typedef typeof(tv.tv_usec) mgw_suseconds_t;

inline int mgw_tvdiff_ms(struct timeval end, struct timeval start);

inline int mgw_tvzero(const struct timeval t);

inline int mgw_tvcmp(struct timeval _a, struct timeval _b);

inline int mgw_tveq(struct timeval _a, struct timeval _b);

inline struct timeval mgw_tvnow(void);

inline struct timeval mgw_tv(mgw_time_t sec, mgw_suseconds_t usec);

inline struct timeval mgw_samp2tv(unsigned int _nsamp, unsigned int _rate);

struct timeval mgw_tvadd(struct timeval a, struct timeval b);

struct timeval mgw_tvsub(struct timeval a, struct timeval b);

void *time_process(void *arg);

#if defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif/*_MGW_TIME_H_*/

