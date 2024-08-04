

// global.h
#ifndef DEVICE_INIT_H
#define DEVICE_INIT_H

// Any global declarations
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sched.h>
#include <syslog.h>
#include <linux/videodev2.h>
#include <semaphore.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/utsname.h>
#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct buffer 
{
        void   *start;
        size_t  length;
};

void open_device(void);
void init_device(void);
int xioctl(int, int, void*);
void errno_exit(const char *);
void init_mmap(void);
void start_capturing(void);
void stop_capturing(void);
void uninit_device(void);
void close_device(void);

#endif // GLOBAL_H




