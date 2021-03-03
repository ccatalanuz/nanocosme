/*  loop_handlers.cpp
 *
 *  CCC
 * 
 *  v3.0 11/2015
 *  v4.0 1/2016
 *  v5.0 6/2017
 *  v6.0 5/2018
 * 
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <pthread.h>
#include <limits.h>

#include "name_server.h"
#include "application.h"


#define MY_STACK_SIZE       (10*1024)      /* 10 kB is enough for now. */

#define TIMESPEC_ADD(ts, pts) {			\
  ts->tv_sec = ts->tv_sec + pts->tv_sec;	\
  ts->tv_nsec = ts->tv_nsec + pts->tv_nsec;	\
  if (ts->tv_nsec > 1000000000) {		\
    ts->tv_nsec = ts->tv_nsec - 1000000000;	\
    ts->tv_sec++;				\
  }						\
}

// Macro to automatic code

#define START_LOOP_HANDLER(lhp_n, loop_n, loop_handler, prty, msec) { 		\
    struct loop_handler_param lhp_n;         			\
    long nsec = msec * 1000LL * 1000LL;				\
    lhp_n.period.tv_sec = nsec / 1000000000;			\
    lhp_n.period.tv_nsec = nsec % 1000000000;			\
    lhp_n.period_msec = (int)msec;				\
    lhp_n.priority = prty;					\
    lhp_n.loop = loop_n;					\
    start_rt_thread(loop_handler, &lhp_n);			\
}

#define LOOP_HANDLER(loopn_cycles, loopn_period, loop_handler_n)		\
  static void *loop_handler_n(void *args) {			\
    double *addr_loopn_cycles;                                  \
    struct sched_param param;  					\
    loop_handler_param *lhp = (loop_handler_param *)args;	\
    struct timespec next;					\
    param.sched_priority = lhp->priority;			\
    if (sched_setscheduler(0, SCHED_RR, &param) < 0) {		\
      printf("sched setscheduler");				\
      exit(1);							\
    }								\
    new_name(loopn_cycles, DOUBLE_NAME, 0, RDONLY);             \
    addr_loopn_cycles = (double *)get_addr_name(loopn_cycles);  \
    new_name(loopn_period, INT_NAME, 0, RDONLY);                \
    set_name(loopn_period, lhp->period_msec);                   \
                                                                \
    clock_gettime(CLOCK_REALTIME, &next);			\
    while(1) {							\
      wait_period(&next, &lhp->period);				\
      printf("-");                                              \
      if (go) {							\
        lhp->loop(lhp->period_msec);				\
        (*addr_loopn_cycles)++;                                 \
      }                                                         \
    }								\
    return NULL; 						\
  }                                                             \


struct loop_handler_param {
  int priority;
  struct timespec period;
  int period_msec;
  void (* loop)(int period);
}; 

long long nsec;
int go = 0;	// set 1 in gateway

/************************************************
* unsigned long long crononsec
*************************************************/
unsigned long long crononsec(int startstop, struct timespec *pre_ts) {
  struct timespec ts;
      
  if (startstop)  {
    clock_gettime(CLOCK_MONOTONIC, pre_ts);
  } else {      
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec - pre_ts->tv_sec) * 1000000000 + ts.tv_nsec - pre_ts->tv_nsec;    
  }
                                                        
  return 0;
} 
 
/******************************************************
* void wait_period(struct timespec *next, long periodo)
******************************************************/
void wait_period(struct timespec *next, struct timespec *period) {
  TIMESPEC_ADD(next, period);
  clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, next, NULL);
}  

/******************************************************
* static void start_rt_thread(void *(* loop_handler)(void *))
******************************************************/ 
static void start_rt_thread(void *(* loop_handler)(void *), loop_handler_param *lhp) {
   pthread_t thread;
   pthread_attr_t attr;
   
   /* init to default values */
   if (pthread_attr_init(&attr)) {
     printf("Attr_init\n");
     exit(1);
   }
     
   /* Set the requested stacksize for this thread */
   if (pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + MY_STACK_SIZE)) {
     printf("attr_setstacksize");
     exit(1);
   }
     
   /* And finally start the actual thread */
   pthread_create(&thread, &attr, loop_handler, lhp);
}


// code generated by preprocesor

LOOP_HANDLER(".loop1_cycles", ".loop1_period", loop_handler1)

void init_loop_handlers() {
  START_LOOP_HANDLER(lhp1, loop1, loop_handler1, 20, 1000);
}
