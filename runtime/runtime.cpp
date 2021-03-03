/*  runtime.cpp
 *
 *  IO Access based on:
 *  RPi GPIO Code Samples (http://elinux.org/RPi_GPIO_Code_Samples)
 * 
 *  v4.0 CCC 03/2021
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

#include "runtime.h"

#include "application.h"
#include "loop_handlers.h"
#include "gateway_mqtt.h"

#define PRE_ALLOCATION_SIZE (1*1024*1024) /* 1MB pagefault free buffer */

#define LONG_MSG 256

int  mem_fd;
char *gpio_mem, *gpio_map;
char *spi0_mem, *spi0_map;

char msg[LONG_MSG];

int end = 0;

pthread_mutexattr_t mtxattr;

// I/O access
volatile unsigned *gpio;

char error_msg[192 * 1024];


/*********************************************************
*  void delay(int seg, int ms)
**********************************************************/
void delay(int seg, int ms) {
  struct timespec ts; 
 
  ts.tv_sec = seg;
  if ((ms * 1000 * 1000) < LONG_MAX) {
    ts.tv_nsec = ms * 1000 * 1000; 
  }

  clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL); 
}

/************************************************
* void runtime_finalize()
*************************************************/
void runtime_finalize() {
  finalize();
}

/************************************************
*  sigint_handler
*************************************************/
void sig_handler(int signal) {
  end = 1;
}

/************************************************
*  void debug(int level, char *msg)
*************************************************/
void debug(int level, const char *msg) {
  switch(level) {
    case FATAL_ERROR:
      printf("\n*** %s ****\n\n", msg);
      runtime_finalize();
      break;
    
    default:
      printf("\n*** %s ****\n", msg);
  }
}

/******************************************************
* void init_cntr_c()
*******************************************************/
void init_cntr_c() {
  struct sigaction saCC;
 
  saCC.sa_handler = sig_handler;
  sigemptyset(&saCC.sa_mask);
  saCC.sa_flags = 0;

  if(sigaction(SIGINT, &saCC, NULL) == -1) {
    printf("Establishing SIGINT handler\n");
    exit(1);
  }

  if(sigaction(SIGTERM, &saCC, NULL) == -1) {
    printf("Establishing SIGTERM handler\n");
    exit(1);
  }
}  

/******************************************************
* static void configure_malloc_behavior(void)
******************************************************/
static void configure_malloc_behavior(void) {
  /* Now lock all current and future pages 
     from preventing of being paged */
  if (mlockall(MCL_CURRENT | MCL_FUTURE)) {
    printf("Error mlockall failed\n");
    exit(1);
  }  

  /* Turn off malloc trimming.*/
  mallopt(M_TRIM_THRESHOLD, -1);

  /* Turn off mmap usage. */
  mallopt(M_MMAP_MAX, 0);
}

/******************************************************
* static void reserve_process_memory(int size)
******************************************************/
static void reserve_process_memory(int size) {
  int i;
  char *buffer;

  buffer = (char *)malloc(size);

  /* Touch each page in this piece of memory to get it mapped into RAM */
  for (i = 0; i < size; i += sysconf(_SC_PAGESIZE)) {
    /* Each write to this buffer will generate a pagefault.
       Once the pagefault is handled a page will be locked in
       memory and never given back to the system. */
    buffer[i] = 0;
  }

  /* buffer will now be released. As Glibc is configured such that it 
     never gives back memory to the kernel, the memory allocated above is
     locked for this process. All malloc() and new() calls come from
     the memory pool reserved and locked above. Issuing free() and
     delete() does NOT make this locking undone. So, with this locking
     mechanism we can build C++ applications that will never run into
     a major/minor pagefault, even with swapping enabled. */
  free(buffer);
}

/******************************************************
* void setup_io()
******************************************************/
void setup_io() {
   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
     printf("Error can't open /dev/mem \n");
     exit(1);
   }
  /* mmap GPIO*/

   // Allocate MAP block
   if ((gpio_mem = (char *)malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL) {
      printf("Error allocation error \n");
      exit(1);
   }

   // Make sure pointer is on 4K boundary
   if ((unsigned long)gpio_mem % PAGE_SIZE)
     gpio_mem += PAGE_SIZE - ((unsigned long)gpio_mem % PAGE_SIZE);
  
   // Now map it
   gpio_map = (char *)mmap(
      (caddr_t)gpio_mem,
      BLOCK_SIZE,
      PROT_READ|PROT_WRITE,
      MAP_SHARED|MAP_FIXED,
      mem_fd,
      GPIO_BASE
   );

   if ((long)gpio_map < 0) {
      printf("Error mmap error\n");
      exit(1);
   }

   // Always use volatile pointer!
   gpio = (volatile unsigned *)gpio_map;
}

/******************************************************
* void init_mutexes() {
******************************************************/
void init_mutexes() {
  pthread_mutexattr_init(&mtxattr);
  pthread_mutexattr_setprotocol(&mtxattr, PTHREAD_PRIO_PROTECT);
  pthread_mutexattr_setprioceiling(&mtxattr, 1);
  pthread_mutexattr_destroy(&mtxattr);
}

/******************************************************
* int main(int argc, char **argv)
******************************************************/
int main(int argc, char **argv) {	
    printf("\nStarting %s...\n", NANOCOSME_VERSION);

//    setup_io();
    
    configure_malloc_behavior();
    reserve_process_memory(PRE_ALLOCATION_SIZE);

    init_cntr_c();
    
    init_gateway_mqtt();
    
    setup();

    init_mutexes();       

    init_loop_handlers();

    while(! end);

    finalize_gateway_mqtt();

    runtime_finalize();
   
    printf("\n%s shutdown\n", NANOCOSME_VERSION);
    
    return 0;     
}
