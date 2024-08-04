#include "global.h"
#include "device_init.h"
#include "service_2.h"
#include "service_4.h"
#define TIMING_ANALYSIS_NEEDED

struct utsname host_name;

typedef struct
{
	int threadIdx;
	int MajorPeriods;
	int frame_count;
	unsigned long long sequencePeriods;
} threadParams_t;

double *execution_time_img_dump;
double *start_time_img_dump;     
double *end_time_img_dump;      
//char ppm_header[114]="P6\n#9999999999 sec 9999999999 msec \n"HRES_STR" "VRES_STR"\n255\n";
char ppm_header[]="P6\n#9999999999 sec 9999999999 msec \n"HRES_STR" "VRES_STR"\n255\n";
char ppm_dumpname[]="frames/test0000.ppm";
void dump_ppm(const void *p, int size, unsigned int tag, struct timespec *time)
{

	int written, i, total, dumpfd;
	//snprintf(&ppm_dumpname[4], 9, "%08d", tag);
    snprintf(&ppm_dumpname[11], 9, "%04d", tag);
	strncat(&ppm_dumpname[15], ".ppm", 5);
	dumpfd = open(ppm_dumpname, O_WRONLY | O_NONBLOCK | O_CREAT, 00666);
	snprintf(&ppm_header[4], 11, "%010.0f", (double)time->tv_sec);
	strncat(&ppm_header[14], " sec ", 5);
	snprintf(&ppm_header[19], 11, "%010.0f", (double)((time->tv_nsec)/1000000));
	strncat(&ppm_header[29]," msec \n"HRES_STR" "VRES_STR"\n255\n",19);
	// snprintf(&ppm_header[48], 53,"#%s\n",(host_name.version));
	// snprintf(&ppm_header[100], 14,"#%s\n",(host_name.nodename));
	//ppm_header[111]='\n';
	if(color_image==0)
	{
		ppm_header[1]='5';
	}
	syslog(LOG_INFO,"DUMPING frame time %lf seconds %lf usec\n",(double)time->tv_sec,(double)time->tv_nsec);
	written=write(dumpfd, ppm_header, sizeof(ppm_header)-1);
	total=0;
	do
	{
		written=write(dumpfd, p, size);
		total+=written;
	} while(total < size);
	close(dumpfd);
}


void *Service_4_frame_dump(void *threadp)
{
    //long long int S4Cnt=-8;
	long long int S4Cnt=INIT;
	threadParams_t *threadParams = (threadParams_t *)threadp;
    // execution_time_img_dump = (double *)malloc((frame_count + 10)*sizeof(double));
	// start_time_img_dump = (double *)malloc((frame_count + 10)*sizeof(double));
	// end_time_img_dump = (double *)malloc((frame_count + 10)*sizeof(double));
	    execution_time_img_dump = (double *)malloc((frame_count + EXTRA_FRAMES_USED)*sizeof(double));
	start_time_img_dump = (double *)malloc((frame_count + EXTRA_FRAMES_USED)*sizeof(double));
	end_time_img_dump = (double *)malloc((frame_count + EXTRA_FRAMES_USED)*sizeof(double));

	double start_time;	
	double end_time;	
	double diff_time;	

	struct timespec S4_start_time;
	struct timespec S4_end_time;
    uname(&host_name);
    while((!abortS4) && S4Cnt <MAX_COUNT)
    {
        sem_wait(&semS4);
		// if(framecnt<0)
		// {
		// 	continue;
		// }
		if(S4Cnt>=0)
		{
		syslog(LOG_CRIT,"IMAGE DUMPING START\n");
#ifdef TIMING_ANALYSIS_NEEDED
        clock_gettime(CLOCK_REALTIME, &S4_start_time);
		start_time = ((double)S4_start_time.tv_sec + (double)((S4_start_time.tv_nsec)/(double)1000000000));
		syslog(LOG_INFO,"S4 Count: %lld\t with start Time: %lf seconds\n",S4Cnt,start_time);
		*(start_time_img_dump + S4Cnt) = start_time;
#endif
        
		if(color_image==1)
		{
			dump_ppm( (image_store + (S4Cnt % 60)), ((size_of_image*6)/4), S4Cnt, &frame_time);
		}
		else if (color_image==0)
		{
			dump_ppm((image_store + (S4Cnt % 60)), (size_of_image/2), S4Cnt, &frame_time);
		}
#ifdef TIMING_ANALYSIS_NEEDED
        clock_gettime(CLOCK_REALTIME, &S4_end_time);
		end_time = ((double)S4_end_time.tv_sec + (double)((S4_end_time.tv_nsec)/(double)1000000000));
		syslog(LOG_INFO,"S4 Count: %lld\t with end Time: %lf seconds\n",S4Cnt,end_time);
		*(end_time_img_dump + S4Cnt) = end_time;
#endif
		}

		S4Cnt++;
    }
    syslog(LOG_CRIT,"S4 thread exited\n");
    pthread_exit((void *)0);
}