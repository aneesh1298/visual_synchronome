#include "global.h"
#include "device_init.h"
#include "service_2.h"
#define TIMING_ANALYSIS_NEEDED
typedef struct
{
	int threadIdx;
	int MajorPeriods;
	int frame_count;
	unsigned long long sequencePeriods;
} threadParams_t;

double *execution_time_img_comp; 
double *start_time_img_comp;    
double *end_time_img_comp;     

unsigned char image_store[60][(640*480*3)];
int compress_enable;
int framecnt=-8;

void* Service_3_frame_compress(void *threadp)
{
    // long long int S3Cnt=-8;
    long long int S3Cnt=INIT;
	threadParams_t *threadParams = (threadParams_t *)threadp;
    // execution_time_img_comp = (double *)calloc((frame_count + 10),sizeof(double));
	// start_time_img_comp = (double *)calloc((frame_count + 10),sizeof(double));
	// end_time_img_comp = (double *)calloc((frame_count + 10),sizeof(double));
        execution_time_img_comp = (double *)calloc((frame_count + EXTRA_FRAMES_USED),sizeof(double));
	start_time_img_comp = (double *)calloc((frame_count + EXTRA_FRAMES_USED),sizeof(double));
	end_time_img_comp = (double *)calloc((frame_count + EXTRA_FRAMES_USED),sizeof(double));
    double start_time;	
	double end_time;	
	double diff_time;	
	struct timespec S3_start_time;
	struct timespec S3_end_time;

    while((!abortS3) && S3Cnt<MAX_COUNT)
    {
        if(compress_enable==0)
        {
            pthread_exit((void *)0);
        }
        sem_wait(&semS3);
        // if(framecnt<0)
        // {
        //     continue;
        // }
        if(S3Cnt>=0)
        {
        syslog(LOG_CRIT,"IMAGE COMPRESSION START\n");
#ifdef TIMING_ANALYSIS_NEEDED
        clock_gettime(CLOCK_REALTIME, &S3_start_time);
		start_time = ((double)S3_start_time.tv_sec + (double)((S3_start_time.tv_nsec)/(double)1000000000));
		syslog(LOG_INFO,"S3 Count: %lld\t with start Time: %lf seconds\n",S3Cnt,start_time);
		*(start_time_img_comp + S3Cnt) = start_time;
#endif
        // reducing color depth by half.
        int shift = 4;
        for (int i = 0; i < 640 * 480 * 3; i++) 
        {
            image_store[S3Cnt%60][i] = (image_store[S3Cnt%60][i] >> shift) << shift;
        }
#ifdef TIMING_ANALYSIS_NEEDED
        clock_gettime(CLOCK_REALTIME, &S3_end_time);
		end_time = ((double)S3_end_time.tv_sec + (double)((S3_end_time.tv_nsec)/(double)1000000000));
		syslog(LOG_INFO,"S3 Count: %lld\t with end Time: %lf seconds\n",S3Cnt,end_time);
		*(end_time_img_comp + S3Cnt) = end_time;
#endif
        syslog(LOG_INFO,"IMAGE compression end\n");
        }

        S3Cnt++;
        sem_post(&semS4);
    }
    syslog(LOG_CRIT,"S3 thread exited\n");
    pthread_exit((void *)0);

}