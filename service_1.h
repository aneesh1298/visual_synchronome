

// global.h
#ifndef SERVICE_1_H
#define SERVICE_1_H

#include <stdio.h>

void* Service_1_frame_acquisition(void*);
void mainloop(void);
static int read_frame(void);
static void process_image(const void *, int);
void yuv2rgb(int, int, int, unsigned char *, unsigned char *, unsigned char *);

#endif 



