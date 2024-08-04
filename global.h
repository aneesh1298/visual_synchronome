// global.h
#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include "device_init.h"

#define USEC_PER_MSEC 	(1000)
#define SEC_TO_MSEC	(1000)
#define NSEC_PER_SEC 	(1000000000)
#define NSEC_PER_USEC	(1000000)
#define TRUE		(1)
#define FALSE		(0)
#define HRES_STR "640"
#define VRES_STR "480"
#define MAX_COUNT 181
#define INIT -16
#define EXTRA_FRAMES_USED 20

extern char* dev_name;
extern int fd;
extern struct v4l2_format fmt;
extern int force_format;
extern struct buffer *buffers;
extern int HRES;
extern int VRES;
extern unsigned int n_buffers;
extern int abortTest;
extern int abortS1 ;
extern int abortS2 ;
extern int abortS3;
extern int abortS4;
extern int compress_enable;
extern int size_of_image;
extern struct timespec frame_time;
extern int color_image;


// NEEDED in service 1
extern struct v4l2_buffer buf;
extern int frame_count;
extern unsigned char bigbuffer[(640*480*3)];
extern sem_t semS1, semS2, semS3, semS4;

// NEEDED for analysis
extern double *execution_time_img_capture;	//To store execution time for each iteration
extern double *start_time_img_capture;		//To store start time for each iteration
extern double *end_time_img_capture;		//To store end time for each iteration

extern double *execution_time_img_dump;	//To store execution time for each iteration
extern double *start_time_img_dump;	//To store start time for each iteration
extern double *end_time_img_dump;	//To store end time for each iteration

extern double *execution_time_seq;	//To store execution time for each iteration
extern double *start_time_seq;		//To store start time for each iteration
extern double *end_time_seq;		//To store end time for each iteration

extern double *execution_time_image_store;	//To store execution time for each iteration
extern double *start_time_image_store;		//To store start time for each iteration
extern double *end_time_image_store;		//To store end time for each iteration

extern double *execution_time_img_comp;	//To store execution time for each iteration
extern double *start_time_img_comp;	//To store start time for each iteration
extern double *end_time_img_comp;	//To store end time for each iteration

extern int frequency_to_capture;
extern int framecnt;


#endif // GLOBAL_H

