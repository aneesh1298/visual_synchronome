#include <stdio.h>
#include <stdlib.h>
#include "device_init.h"
#include "global.h"
#include "service_2.h"


#define TIMING_ANALYSIS_NEEDED
//unsigned char image_store[60][640*480*3];

double *execution_time_image_store;	
double *start_time_image_store;		
double *end_time_image_store;	

typedef struct
{
	int threadIdx;
	int MajorPeriods;
	int frame_count;
	unsigned long long sequencePeriods;
} threadParams_t;

void* Service_2_frame_process(void* threadp)
{
    // long long int S2Cnt=-8;
	long long int S2Cnt=INIT;
	threadParams_t *threadParams = (threadParams_t *)threadp;
    struct timespec S2_start_time; 
    struct timespec S2_end_time;
    double start_time, end_time, diff_time;

    // execution_time_image_store = (double *)malloc((frame_count + 10)*sizeof(double));
	// start_time_image_store = (double *)malloc((frame_count + 10)*sizeof(double));
	// end_time_image_store = (double *)malloc((frame_count + 10)*sizeof(double));
	    execution_time_image_store = (double *)malloc((frame_count + EXTRA_FRAMES_USED)*sizeof(double));
	start_time_image_store = (double *)malloc((frame_count + EXTRA_FRAMES_USED)*sizeof(double));
	end_time_image_store = (double *)malloc((frame_count + EXTRA_FRAMES_USED)*sizeof(double));
    while((!abortS2) && S2Cnt<MAX_COUNT)
    {
        sem_wait(&semS2); 
		// if(framecnt<0)
        // {
        //     continue;
        // }
		if(S2Cnt>=0)
		{
        syslog(LOG_CRIT, "Image store service is started\n");
#ifdef TIMING_ANALYSIS_NEEDED
        clock_gettime(CLOCK_REALTIME, &S2_start_time);  
		start_time = ((double)S2_start_time.tv_sec + (double)((S2_start_time.tv_nsec)/(double)1000000000));	
        syslog(LOG_INFO,"S2 Count: %lld\t with Start Time: %lf seconds\n",S2Cnt,start_time);
		*(start_time_image_store + S2Cnt) = start_time;		
#endif

		if(color_image==1)
		{
    		for(int i=0;i<(640*480*3);i++)
			{
				image_store[S2Cnt % 60][i] = bigbuffer[i];
			}
		}
		else
		{
			for(int i=0;i<(640*480);i++)
			{
				image_store[S2Cnt % 60][i] = bigbuffer[i];
			}
		}
#ifdef TIMING_ANALYSIS_NEEDED
		clock_gettime(CLOCK_REALTIME, &S2_end_time);    //Get end time of the service
		end_time = ((double)S2_end_time.tv_sec + (double)((S2_end_time.tv_nsec)/(double)1000000000));		//Store end time in seconds
		syslog(LOG_INFO,"S2 Count: %lld\t with End Time: %lf seconds\n",S2Cnt,end_time);
		*(end_time_image_store + S2Cnt) = end_time;		//Store end time in array
		syslog(LOG_CRIT,"Image store end\n");
#endif
		}
		S2Cnt++;                                 
		if(compress_enable==0)
        { 
            sem_post(&semS4);
        }
		else
		{
			syslog(LOG_CRIT,"IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII\n");
			sem_post(&semS3);
		}
    }
    syslog(LOG_CRIT,"S2 thread exited\n");
    pthread_exit((void *)0);
}