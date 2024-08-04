// global.h
#ifndef SERVICE_2_H
#define SERVICE_2_H

#include <stdio.h>

// NEEDED in service 2 and 3
extern unsigned char image_store[60][(640*480*3)];
// extern char ppm_header[200];
// extern char ppm_dumpname[];
// needed for service 4



void* Service_2_frame_process(void* );

#endif 



