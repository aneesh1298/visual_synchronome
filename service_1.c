#include "device_init.h"
#include "global.h"
#include "service_1.h"
#define TIMING_ANALYSIS_NEEDED

typedef struct
{
	int threadIdx;
	int MajorPeriods;
	int frame_count;
	unsigned long long sequencePeriods;
} threadParams_t;
int size_of_image=0;
unsigned char bigbuffer[(640*480*3)];

//double wcet_img_capture = 0;		
//double avg_exec_time_img_capture = 0;	
double *execution_time_img_capture;	
double *start_time_img_capture;		
double *end_time_img_capture;

struct timespec S1_end_time;
struct timespec S1_start_time;
struct timespec frame_time;


void* Service_1_frame_acquisition(void* threadp)
{
    // execution_time_img_capture = (double *)malloc((frame_count + 10)*sizeof(double));
	// start_time_img_capture = (double *)malloc((frame_count + 10)*sizeof(double));
	// end_time_img_capture = (double *)malloc((frame_count + 10)*sizeof(double));
        execution_time_img_capture = (double *)malloc((frame_count + EXTRA_FRAMES_USED)*sizeof(double));
	start_time_img_capture = (double *)malloc((frame_count + EXTRA_FRAMES_USED)*sizeof(double));
	end_time_img_capture = (double *)malloc((frame_count + EXTRA_FRAMES_USED)*sizeof(double));

    // long long int S1Cnt = -8;
    long long int S1Cnt = INIT;
	threadParams_t *threadParams = (threadParams_t *)threadp;
    double start_time, end_time;
    while((!abortS1) && S1Cnt<MAX_COUNT)
    {
        sem_wait(&semS1);
        framecnt++;
        // if(framecnt<0)
        // {
        //     continue;
        // }
        if(S1Cnt>=0)
        {
#ifdef TIMING_ANALYSIS_NEEDED
        clock_gettime(CLOCK_REALTIME, &S1_start_time); 
		start_time = ((double)S1_start_time.tv_sec + (double)((S1_start_time.tv_nsec)/(double)1000000000));	
		syslog(LOG_CRIT,"S1 Count: %lld\t service 1 start Time: %lf seconds\n",S1Cnt,start_time);
		*(start_time_img_capture + S1Cnt) = start_time;
#endif
        }
	
        mainloop();
         if(S1Cnt>=0)
         {
#ifdef TIMING_ANALYSIS_NEEDED
        clock_gettime(CLOCK_REALTIME, &S1_end_time);
        end_time= ((double)S1_end_time.tv_sec + (double)((S1_end_time.tv_nsec)/(double)1000000000));	
        syslog(LOG_CRIT,"S1 Count: %lld\t service 1 end Time: %lf seconds\n",S1Cnt,end_time);
        *(end_time_img_capture + S1Cnt) =end_time;
        syslog(LOG_INFO,"Image capture end\n");
#endif
        }

        S1Cnt++;
        

    }
    syslog(LOG_INFO,"S1 Thread captured\n");
    pthread_exit((void *)0);
}

void mainloop(void)
{
	struct timespec read_delay;
	struct timespec time_error;

	read_delay.tv_sec=0;
	read_delay.tv_nsec=30000;

	fd_set fds;
	struct timeval tv;
	int r;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	/* Timeout. */
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	r = select(fd + 1, &fds, NULL, NULL, &tv);

	if (-1 == r)
	{
		if (EINTR == errno)
			//continue;
			errno_exit("select");
	}

	if (0 == r)
	{
		fprintf(stderr, "select timeout\n");
		exit(EXIT_FAILURE);
	}

	if (read_frame())
	{
		// if(nanosleep(&read_delay, &time_error) != 0)
		// 	perror("nanosleep");
		// else
		// {
		// }
	}
	
}

static int read_frame(void)
{
    struct v4l2_buffer buf;
    unsigned int i;

    CLEAR(buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    // clock_gettime(CLOCK_MONOTONIC, &itr_time_before_acqi);
    //         //itrstart = (double)itr_time_start.tv_sec + (double)itr_time_start.tv_nsec / 1000000000.0;


    if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
        {
            switch (errno)
            {
                case EAGAIN:
                    return 0;

                case EIO:
                    /* Could ignore EIO, but drivers should only set for serious errors, although some set for
                           non-fatal errors too.
                     */
                    return 0;
                default:
                    syslog(LOG_CRIT,"mmap failure\n");
                    errno_exit("VIDIOC_DQBUF");
            }
            }


            assert(buf.index < n_buffers);

            process_image(buffers[buf.index].start, buf.bytesused);

            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                    errno_exit("VIDIOC_QBUF");

    //printf("R");
    return 1;
}

static void process_image(const void *p, int size)
{
    int i, newi, newsize=0;
    
    int y_temp, y2_temp, u_temp, v_temp;
    unsigned char *pptr = (unsigned char *)p;
    size_of_image=size;
    clock_gettime(CLOCK_REALTIME, &frame_time);

    //itr_time_before_dump,itr_time_after_dump;
    // record when process was called
#ifdef TIMING_ANALYSIS_NEEDED
    clock_gettime(CLOCK_REALTIME, &frame_time);    
#endif
    //frame_count++;

    if((fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV) && (color_image==1))
    {
        for(i=0, newi=0; i<size; i=i+4, newi=newi+6)
        {
            y_temp=(int)pptr[i]; u_temp=(int)pptr[i+1]; y2_temp=(int)pptr[i+2]; v_temp=(int)pptr[i+3];
            yuv2rgb(y_temp, u_temp, v_temp, &bigbuffer[newi], &bigbuffer[newi+1], &bigbuffer[newi+2]);
            yuv2rgb(y2_temp, u_temp, v_temp, &bigbuffer[newi+3], &bigbuffer[newi+4], &bigbuffer[newi+5]);
        }        
    }
    else if (color_image==0)
    {
        for(i=0, newi=0; i<size; i=i+4, newi=newi+2)
        {
            // Y1=first byte and Y2=third byte
            bigbuffer[newi]=pptr[i];
            bigbuffer[newi+1]=pptr[i+2];
        }
    }
    
}

void yuv2rgb(int y, int u, int v, unsigned char *r, unsigned char *g, unsigned char *b)
{
	int r1, g1, b1;

	// replaces floating point coefficients
	int c = y-16, d = u - 128, e = v - 128;

	// Conversion that avoids floating point
	r1 = (298 * c           + 409 * e + 128) >> 8;
	g1 = (298 * c - 100 * d - 208 * e + 128) >> 8;
	b1 = (298 * c + 516 * d           + 128) >> 8;

	// Computed values may need clipping.
	if (r1 > 255) r1 = 255;
	if (g1 > 255) g1 = 255;
	if (b1 > 255) b1 = 255;

	if (r1 < 0) r1 = 0;
	if (g1 < 0) g1 = 0;
	if (b1 < 0) b1 = 0;

	*r = r1 ;
	*g = g1 ;
	*b = b1 ;
}