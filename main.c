/*
 *
 *  Adapted by Sam Siewert for use with UVC web cameras and Bt878 frame
 *  grabber NTSC cameras to acquire digital video from a source,
 *  time-stamp each frame acquired, save to a PGM or PPM file.
 *
 *  The original code adapted was open source from V4L2 API and had the
 *  following use and incorporation policy:
 * 
 *  This program can be used and distributed without restrictions.
 *
 *      This program is provided with the V4L2 API
 * see http://linuxtv.org/docs.php for more information
 *
 * sudo apt-get install sysstat
 *
 */


#include "device_init.h"
#include "service_1.h"
#include "service_2.h"
#include "service_3.h"
#include "service_4.h"
#include "global.h"
#include "analysis.h"
#define NUM_THREADS (4)
#define HRES_STR "640"
#define VRES_STR "480"
#define NUM_CPU_CORES 4
// #define MAX_COUNT 181
// #define INIT -16
// #define EXTRA_FRAMES_USED 20


#define TIMING_ANALYSIS_NEEDED

// Format is used by a number of functions, so made as a file global
struct v4l2_format fmt;
struct buffer *buffers;
// User input arguments
char *dev_name;
int frame_count;
int HRES=0;
int VRES=0;
int frequency_to_capture=1;

int fd;
unsigned int n_buffers;
//long long int seqCnt=-8;
long long int seqCnt= INIT;
int abortTest=FALSE;
int abortS1=FALSE, abortS2=FALSE, abortS3=FALSE, abortS4= FALSE;
sem_t semS1, semS2, semS3, semS4;
static timer_t timer_1;
struct itimerval delay;

void print_scheduler(void);
void sequencer(void);
// We also need variables of timing analysis for the sequencer thread first.
double wcet_img_seq = 0;	
double avg_exec_time_seq = 0;	
double *execution_time_seq;	
double *start_time_seq;		
double *end_time_seq;	
//char ppm_header[200];
	

typedef struct
{
	int threadIdx;
	int MajorPeriods;
	int frame_count;
	unsigned long long sequencePeriods;
} threadParams_t;
int color_image=0;

int main(int argc, char **argv)
{
    int i, rc, scope, flags=0;
    cpu_set_t threadcpu;
    cpu_set_t allcpuset;

    pthread_t threads[NUM_THREADS];
    threadParams_t threadParams[NUM_THREADS];
    pthread_attr_t rt_sched_attr[NUM_THREADS];
    int rt_max_prio, rt_min_prio, cpuidx;

    struct sched_param rt_param[NUM_THREADS];
    struct sched_param main_param;

    pthread_attr_t main_attr;
    pid_t mainpid;
    syslog(LOG_CRIT,"System has %d processors configured and %d available.\n", get_nprocs_conf(), get_nprocs());
    CPU_ZERO(&allcpuset);

    for(i=0; i < NUM_CPU_CORES; i++)
       CPU_SET(i, &allcpuset);
    syslog(LOG_CRIT,"Using CPUS=%d from total available.\n", CPU_COUNT(&allcpuset));
    if (sem_init (&semS1, 0, 0)) { syslog(LOG_CRIT,"Failed to initialize S1 semaphore\n"); exit (-1); }
    if (sem_init (&semS2, 0, 0)) { syslog(LOG_CRIT,"Failed to initialize S2 semaphore\n"); exit (-1); }
    if (sem_init (&semS3, 0, 0)) { syslog(LOG_CRIT,"Failed to initialize S3 semaphore\n"); exit (-1); }
    if (sem_init (&semS4, 0, 0)) { syslog(LOG_CRIT,"Failed to initialize S4 semaphore\n"); exit (-1); }
    // if (sem_init (&semS5, 0, 0)) { syslog(LOG_CRIT,"Failed to initialize S5 semaphore\n"); exit (-1); }

    mainpid=getpid();

    rt_max_prio = sched_get_priority_max(SCHED_FIFO);
    rt_min_prio = sched_get_priority_min(SCHED_FIFO);

    rc=sched_getparam(mainpid, &main_param);
    main_param.sched_priority=rt_max_prio;
    rc=sched_setscheduler(getpid(), SCHED_FIFO, &main_param);
    if(rc < 0) perror("main_param");
    print_scheduler();
    pthread_attr_getscope(&main_attr, &scope);

    if(scope == PTHREAD_SCOPE_SYSTEM)
      syslog(LOG_CRIT,"PTHREAD SCOPE SYSTEM\n");
    else if (scope == PTHREAD_SCOPE_PROCESS)
      syslog(LOG_CRIT,"PTHREAD SCOPE PROCESS\n");
    else
      syslog(LOG_CRIT,"PTHREAD SCOPE UNKNOWN\n");

    syslog(LOG_CRIT,"rt_max_prio=%d\n", rt_max_prio);
    syslog(LOG_CRIT,"rt_min_prio=%d\n", rt_min_prio);
    int cpu = sched_getcpu();
    if (cpu == -1) {
        perror("sched_getcpu");
        return 1;
    }
    for(i=0; i < NUM_THREADS-2; i++)
    {

      // run ALL threads on core RT_CORE
      CPU_ZERO(&threadcpu);

      // BE CAREFUL ON THIS AFFINITY
      if(cpu==0)
      {
        cpuidx=2;
      }
      else
      {
        cpuidx=(cpu);
      }
      syslog(LOG_CRIT, "1,2 indexed threads are running on %d cpu\n",cpuidx);
      CPU_SET(cpuidx, &threadcpu);

      rc=pthread_attr_init(&rt_sched_attr[i]);
      rc=pthread_attr_setinheritsched(&rt_sched_attr[i], PTHREAD_EXPLICIT_SCHED);
      rc=pthread_attr_setschedpolicy(&rt_sched_attr[i], SCHED_FIFO);
      rc=pthread_attr_setaffinity_np(&rt_sched_attr[i], sizeof(cpu_set_t), &threadcpu);

      rt_param[i].sched_priority=rt_max_prio-i;
      pthread_attr_setschedparam(&rt_sched_attr[i], &rt_param[i]);

      threadParams[i].threadIdx=i;
    }
    for(i=NUM_THREADS-2; i < NUM_THREADS; i++)
    {

      // run ALL threads on core RT_CORE
      CPU_ZERO(&threadcpu);
      // BE CAREFUL ON THIS AFFINITY
      if(cpu <3)
      {
        cpuidx=3;
      }
      else
      {
        cpuidx=(2);
      }
      syslog(LOG_CRIT, "3,4 indexed threads are running on %d cpu\n",cpuidx);
      CPU_SET(cpuidx, &threadcpu);

      rc=pthread_attr_init(&rt_sched_attr[i]);
      rc=pthread_attr_setinheritsched(&rt_sched_attr[i], PTHREAD_EXPLICIT_SCHED);
      rc=pthread_attr_setschedpolicy(&rt_sched_attr[i], SCHED_FIFO);
      rc=pthread_attr_setaffinity_np(&rt_sched_attr[i], sizeof(cpu_set_t), &threadcpu);

      rt_param[i].sched_priority=rt_max_prio-i;
      pthread_attr_setschedparam(&rt_sched_attr[i], &rt_param[i]);

      threadParams[i].threadIdx=i;
    }
    if(argc>1)
    {
        dev_name= argv[1];
        frame_count= atoi(argv[2]);
        HRES= atoi(argv[3]);
        VRES= atoi(argv[4]);
        frequency_to_capture= atoi(argv[5]);
        compress_enable= atoi(argv[6]);
        color_image= atoi(argv[7]);
    }
    else
    {
        syslog(LOG_CRIT, "Usage: ./program dev_name HRES VRES frquency compression\n");
        return 0;
    }

    open_device();
    init_device();
    start_capturing();

        // Real time services
    // Servcie_1 = RT_MAX-1	
    //
    rt_param[0].sched_priority=rt_max_prio-1;
    pthread_attr_setschedparam(&rt_sched_attr[0], &rt_param[0]);
    rc=pthread_create(&threads[0],               // pointer to thread descriptor
                      &rt_sched_attr[0],         // use specific attributes
                      //(void *)0,               // default attributes
                      Service_1_frame_acquisition,                 // thread function entry point
                      (void *)&(threadParams[0]) // parameters to pass in
                     );
    if(rc < 0)
        perror("pthread_create for service 1 - V4L2 video frame acquisition");
    else
        syslog(LOG_CRIT,"pthread_create successful for service 1\n");

    // Service_2 = RT_MAX-2
    //
    rt_param[1].sched_priority=rt_max_prio-2;
    pthread_attr_setschedparam(&rt_sched_attr[1], &rt_param[1]);
    rc=pthread_create(&threads[1], &rt_sched_attr[1], Service_2_frame_process, (void *)&(threadParams[1]));
    if(rc < 0)
        perror("pthread_create for service 2 - frame process");
    else
        syslog(LOG_CRIT,"pthread_create successful for service 2\n");

    syslog(LOG_CRIT,"Service threads will run on %d CPU cores\n", CPU_COUNT(&threadcpu));
    // Non-Real Time services
    // Compress Given more prio than dump
    rt_param[NUM_THREADS-2].sched_priority=rt_max_prio;
    pthread_attr_setschedparam(&rt_sched_attr[NUM_THREADS-2], &rt_param[NUM_THREADS-2]);
    rc=pthread_create(&threads[NUM_THREADS-2], &rt_sched_attr[NUM_THREADS-2], Service_3_frame_compress, (void *)&(threadParams[NUM_THREADS-2]));
    if(rc < 0)
        perror("pthread_create for service 3 - flash frame compress");
    else
        syslog(LOG_CRIT,"pthread_create successful for service 3\n");

    //  dump
    rt_param[NUM_THREADS-1].sched_priority=rt_max_prio-1;
    pthread_attr_setschedparam(&rt_sched_attr[NUM_THREADS-1], &rt_param[NUM_THREADS-1]);
    rc=pthread_create(&threads[NUM_THREADS-1], &rt_sched_attr[NUM_THREADS-1], Service_4_frame_dump, (void *)&(threadParams[NUM_THREADS-1]));
    if(rc < 0)
        perror("pthread_create for service 4 - flash frame storage");
    else
        syslog(LOG_CRIT,"pthread_create successful for service 4\n");




    // // SEQUENCER
    // //
    // rt_param[2].sched_priority=rt_max_prio;
    // pthread_attr_setschedparam(&rt_sched_attr[2], &rt_param[2]);
    // rc=pthread_create(&threads[2], &rt_sched_attr[2], Sequencer, (void *)&(threadParams[2]));
    // if(rc < 0)
    //     perror("pthread_create for sequencer service");
    // else
    //     syslog(LOG_CRIT,"pthread_create successful for sequencer service\n");
    int ret;
    signal(SIGALRM,  sequencer);

    if(frequency_to_capture==10)
    {
        delay.it_value.tv_sec=0;
        delay.it_value.tv_usec=1000;
        delay.it_interval.tv_sec=0;
        delay.it_interval.tv_usec=100000; 
    }
    else if(frequency_to_capture==1)
    {
        delay.it_value.tv_sec=0;
        delay.it_value.tv_usec=10000;
        delay.it_interval.tv_sec=1;
        delay.it_interval.tv_usec=0; 
    }
    ret= setitimer(ITIMER_REAL,&delay,NULL);
    if(ret)
    {
        perror("settimer");
        return 0;
    }  

    for(i=0;i<NUM_THREADS;i++)
    {
        if(rc=pthread_join(threads[i], NULL) < 0)
		perror("main pthread_join");
	else
		syslog(LOG_CRIT,"joined thread %d\n", i);
    }

    stop_capturing();
    uninit_device();
    close_device();

    print_sequencer_analysis();	// Print analysis for sequencer
	print_image_capture_analysis();	// Print analysis for image capture
	print_image_store_analysis();	//Print analysis for image store
    print_image_comp_analysis();
    print_image_dump_analysis();	// Print analysis for image dump

    return 0;

}

/*
double wcet_img_seq = 0;	
double avg_exec_time_seq = 0;	
double *execution_time_seq;	
double *start_time_seq;		
double *end_time_seq;*/

void sequencer(void)
{
    struct timespec current_time_val;
    double current_realtime;
    int rc, flags=0;
    // these 5 are for the sequesnecer start and the end time calculations needed.
    struct timespec sequencer_start_time;		
	struct timespec sequencer_end_time;		
	double start_time;	
	double end_time;	
	double diff_time;
    if(abortTest || seqCnt == MAX_COUNT)
    {
        // disable interval timer
        delay.it_value.tv_sec=0;
        delay.it_value.tv_usec=0;
        delay.it_interval.tv_sec=0;
        delay.it_interval.tv_usec=0; 
        setitimer(ITIMER_REAL,&delay,NULL);
	    syslog(LOG_CRIT,"Disabling sequencer interval timer with abort=%d and %llu\n", abortTest, seqCnt);

	// shutdown all services
        abortS1=TRUE; abortS2=TRUE; abortS3=TRUE; abortS4=TRUE;
        sem_post(&semS1); sem_post(&semS2); //sem_post(&semS3);sem_post(&semS4);
        pthread_exit((void*)0);

    }	

    // execution_time_seq= (double*)malloc((frame_count+10)*sizeof(double));
    // start_time_seq= (double*)malloc((frame_count+10)*sizeof(double));
    // end_time_seq= (double*)malloc((frame_count+10)*sizeof(double));
        execution_time_seq= (double*)malloc((frame_count+EXTRA_FRAMES_USED)*sizeof(double));
    start_time_seq= (double*)malloc((frame_count+EXTRA_FRAMES_USED)*sizeof(double));
    end_time_seq= (double*)malloc((frame_count+EXTRA_FRAMES_USED)*sizeof(double));
    if(execution_time_seq== NULL)
    {
        syslog(LOG_CRIT, " ERROR in malloc of execution time array\n");
    }
    if(start_time_seq== NULL)
    {
        syslog(LOG_CRIT, " ERROR in malloc of start time array\n");
    }
    if(end_time_seq== NULL)
    {
        syslog(LOG_CRIT, " ERROR in malloc of end time array\n");
    }
#ifdef TIMING_ANALYSIS_NEEDED
    clock_gettime(CLOCK_REALTIME, &sequencer_start_time);
    start_time = ((double)sequencer_start_time.tv_sec + (double)((sequencer_start_time.tv_nsec)/(double)1000000000));
    if(seqCnt<MAX_COUNT && seqCnt>=0)
    {
        *(start_time_seq + seqCnt) = start_time;
    }
    syslog(LOG_CRIT,"Sequence count is shown as : %lld\t with start Time: %lf seconds\n",seqCnt,start_time);
#endif          
    
    // Release each service at a sub-rate of the generic sequencer rate
    // Servcie_1 @ 5 Hz
    sem_post(&semS1);
    // Service_2 @ 1 Hz
    sem_post(&semS2);
    // Service_3 @ 1 Hz
    // if(compression_enable==1)
    // {
    //     sem_post(&semS3);
    // }
    // else
    // {
    //     sem_post(&semS4);
    // }
#ifdef TIMING_ANALYSIS_NEEDED
	clock_gettime(CLOCK_REALTIME, &sequencer_end_time);
	end_time = ((double)sequencer_end_time.tv_sec + (double)((sequencer_end_time.tv_nsec)/(double)1000000000));
	*(end_time_seq + seqCnt) = end_time;
    syslog(LOG_CRIT,"Sequence count is shown as : %lld\t with end Time: %lf seconds\n",seqCnt,end_time);
#endif
seqCnt++;

}

void print_scheduler(void)
{
   int schedType;

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
       case SCHED_FIFO:
           printf("Pthread Policy is SCHED_FIFO\n");
           break;
       case SCHED_OTHER:
           printf("Pthread Policy is SCHED_OTHER\n"); exit(-1);
         break;
       case SCHED_RR:
           printf("Pthread Policy is SCHED_RR\n"); exit(-1);
           break;
       //case SCHED_DEADLINE:
       //    printf("Pthread Policy is SCHED_DEADLINE\n"); exit(-1);
       //    break;
       default:
           printf("Pthread Policy is UNKNOWN\n"); exit(-1);
   }
}
