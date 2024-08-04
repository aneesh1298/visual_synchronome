#include <stdio.h>
#include <stdlib.h>
#include "device_init.h"
#include "global.h"

/* File related variables for dumping analysis into excel file */
static FILE *fptr_S1;		//File pointer to file consisiting of image capture analysis
static FILE *fptr_S2;		//File pointer to file consisting of image dump analysis
static FILE *fptr_S3;		//File pointer to file consisting of image send analysis
static FILE *fptr_seq;		//File pointer to file consisting of sequencer analysis
static FILE *fptr_S4;	//File pointer to file consisting of image store analysis
char filename_S1[] = "Image_Capture_Jitter_Analysis.csv";	//Image capture file name
char filename_S2[] = "Image_store_Jitter_Analysis.csv";		//Image dump file name
char filename_S3[] = "Image_comp_Jitter_Analysis.csv";		//Image send file name
char filename_seq[]= "Sequencer_Jitter_Analysis.csv";		//Sequencer file name
char filename_S4[] = "Image_dump_Jitter_Analysis.csv";

/* Variables for analysis of image capture service */
double wcet_img_capture;			//Store worst case execution for image capturing
double total_exec_time_img_capture;		//Store average execution time
double jitter_img_capture;			//Store jitter for image capture

/* Variables for analysis of image dump service */
double wcet_img_dump;			//Store worst case execution for image capturing
double total_exec_time_img_dump;	//Store average execution time
double jitter_img_dump;			//Store jitter for image capture

/* Variables for analysis of image dump service */
double wcet_img_comp;			//Store worst case execution for image capturing
double total_exec_time_img_comp;	//Store average execution time
double jitter_img_comp;			//Store jitter for image capture

/* Variables for analysis of sequencer analysis */
double wcet_seq;			//Store worst case execution for image capturing
double total_exec_time_seq;		//Store average execution time
double jitter_seq;			//Store jitter for image capture

/* Variables for analysis of image store analysis */
double wcet_image_store;			//Store worst case execution for image capturing
double total_exec_time_image_store;		//Store average execution time
double jitter_image_store;			//Store jitter for image capture

/* Variables necessary for jitter analysis (Start Time Jitter) */
struct timespec delay_1hz = {1,0};		//For jitter analysis when system runs at 1 hertz
struct timespec delay_10hz = {0,100000000};	//For jitter analysis when system runs at 10 hertz

void print_sequencer_analysis()
{

	total_exec_time_seq = 0;
	fptr_seq = fopen(filename_seq,"w+");
    fprintf(fptr_seq,"Sequencer Count,Start_Time(in sec),End_Time(in sec),Execution Time(in msec),Jitter(in msec)");
	/* Calculate execution time, WCET and average execution time */
	for(int i=0;i < (MAX_COUNT) ;i++)
	{
		/* Calculate execution time of each iteration of sequencer (in secs) */
		*(execution_time_seq + i) = (*(end_time_seq + i) - *(start_time_seq + i))*SEC_TO_MSEC;
		if(i == 0)
		{
			wcet_seq = *(execution_time_seq + i);
		}
		if(wcet_seq < *(execution_time_seq + i))
		{
			wcet_seq = *(execution_time_seq + i);
		}
		/* Calculate total time of execution for image capture thread */
		total_exec_time_seq += *(execution_time_seq + i);
	}

	for(int i=0; i< (MAX_COUNT) ;i++)
	{
		if(frequency_to_capture == 1)
		{
			if(i == 0)
			{
				jitter_seq = 0;
				fprintf(fptr_seq,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_seq + i)*SEC_TO_MSEC,*(end_time_seq + i)*SEC_TO_MSEC,*(execution_time_seq + i),jitter_seq*SEC_TO_MSEC);
			}
			else
			{
				jitter_seq = (*(start_time_seq + i - 1) + delay_1hz.tv_sec) - (*(start_time_seq + i)) ;
				fprintf(fptr_seq,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_seq + i)*SEC_TO_MSEC,*(end_time_seq + i)*SEC_TO_MSEC,*(execution_time_seq + i),jitter_seq*SEC_TO_MSEC);      //Write all values calculated in file
			}
		}
		else if (frequency_to_capture == 10)
		{
			if(i == 0)
			{
				jitter_seq = 0;
				fprintf(fptr_seq,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_seq + i)*SEC_TO_MSEC,*(end_time_seq + i)*SEC_TO_MSEC,*(execution_time_seq + i),jitter_seq*SEC_TO_MSEC);
			}
			else
			{
				jitter_seq = (*(start_time_seq + i - 1) + (delay_10hz.tv_nsec/NSEC_PER_SEC)) - (*(start_time_seq + i)) ;
				fprintf(fptr_seq,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_seq + i)*SEC_TO_MSEC,*(end_time_seq + i)*SEC_TO_MSEC,*(execution_time_seq + i),jitter_seq*SEC_TO_MSEC);      //Write all values calculated in file
			}

		}
	}
	syslog(LOG_CRIT,"Sequencer worst execution time  = %lf\n",wcet_seq);
	syslog(LOG_CRIT,"Sequencer average execution time = %lf\n",total_exec_time_seq/(MAX_COUNT));
    fclose(fptr_seq);
	
}

void print_image_capture_analysis()
{
	total_exec_time_img_capture = 0;
	fptr_S1 = fopen(filename_S1,"w+");
	fprintf(fptr_S1,"S1 Count,Start_Time(in sec),End_Time(in sec),Image_Capture_Time(in msec),Jitter(in msec)");
	/* Calculate execution time, WCET and average execution time */
	for(int i=0;i< (MAX_COUNT);i++)
	{
		*(execution_time_img_capture + i) = (*(end_time_img_capture + i) - *(start_time_img_capture + i))*SEC_TO_MSEC;
		if(i == 0)
		{
			wcet_img_capture = *(execution_time_img_capture + i);
		}
		if(wcet_img_capture < *(execution_time_img_capture + i))
		{
			wcet_img_capture = *(execution_time_img_capture + i);
		}
		total_exec_time_img_capture += *(execution_time_img_capture + i);
	}

	for(int i=0;i<(MAX_COUNT);i++)
	{

		if(frequency_to_capture == 1)
		{
			if(i == 0)
			{
				jitter_img_capture = 0;
				fprintf(fptr_S1,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_img_capture + i)*SEC_TO_MSEC,*(end_time_img_capture + i)*SEC_TO_MSEC,*(execution_time_img_capture + i),jitter_img_capture*SEC_TO_MSEC);
			}
			else
			{
				jitter_img_capture = (*(start_time_img_capture + i - 1) + delay_1hz.tv_sec) - (*(start_time_img_capture + i)) ;
				fprintf(fptr_S1,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_img_capture + i)*SEC_TO_MSEC,*(end_time_img_capture + i)*SEC_TO_MSEC,*(execution_time_img_capture + i),jitter_img_capture*SEC_TO_MSEC);      //Write all values calculated in file
			}
		}
		else if (frequency_to_capture == 10)
		{
			if(i == 0)
			{
				jitter_img_capture = 0;
				fprintf(fptr_S1,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_img_capture + i)*SEC_TO_MSEC,*(end_time_img_capture + i)*SEC_TO_MSEC,*(execution_time_img_capture + i),jitter_img_capture*SEC_TO_MSEC);
			}
			else
			{
				jitter_img_capture = (*(start_time_img_capture + i - 1) + (delay_10hz.tv_nsec/NSEC_PER_SEC)) - (*(start_time_img_capture + i)) ;
				fprintf(fptr_S1,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_img_capture + i)*SEC_TO_MSEC,*(end_time_img_capture + i)*SEC_TO_MSEC,*(execution_time_img_capture + i),jitter_img_capture*SEC_TO_MSEC);      //Write all values calculated in file
			}

		}
	}
	syslog(LOG_CRIT,"Service 1 worst execution time= %lf\n",wcet_img_capture);
	syslog(LOG_CRIT,"Service 1 average execution time= %lf\n",total_exec_time_img_capture/(MAX_COUNT));
	fclose(fptr_S1);
}


void print_image_dump_analysis()
{
	total_exec_time_img_dump = 0;
	fptr_S4 = fopen(filename_S4,"w+");
	fprintf(fptr_S4,"S4 Count,Start_Time(in sec),End_Time(in sec),Image_Dump_Time(in msec),Jitter(in sec)");
	for(int i=0;i<(MAX_COUNT);i++)
	{
		*(execution_time_img_dump + i) = (*(end_time_img_dump + i) - *(start_time_img_dump + i))*SEC_TO_MSEC;
		if(i == 0)
		{
			wcet_img_dump = *(execution_time_img_dump + i);
		}
		if(wcet_img_dump < *(execution_time_img_dump + i))
		{
			wcet_img_dump = *(execution_time_img_dump + i);
		}
		/* Calculate total time of execution for image capture thread */
		total_exec_time_img_dump += *(execution_time_img_dump + i);
	}

	for(int i=0;i<(MAX_COUNT);i++)
	{
		if(frequency_to_capture == 1)
		{
			if(i == 0)
			{
				jitter_img_dump = 0;
				fprintf(fptr_S4,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_img_dump + i)*SEC_TO_MSEC,*(end_time_img_dump + i)*SEC_TO_MSEC,*(execution_time_img_dump + i),jitter_img_dump*SEC_TO_MSEC);
			}
			else
			{
				jitter_img_dump = (*(start_time_img_dump + i - 1) + delay_1hz.tv_sec) - (*(start_time_img_dump + i)) ;
				fprintf(fptr_S4,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_img_dump + i)*SEC_TO_MSEC,*(end_time_img_dump + i)*SEC_TO_MSEC,*(execution_time_img_dump + i),jitter_img_dump*SEC_TO_MSEC);      //Write all values calculated in file
			}
		}
		else if (frequency_to_capture == 10)
		{
			if(i == 0)
			{
				jitter_img_dump = 0;
				fprintf(fptr_S4,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_img_dump + i)*SEC_TO_MSEC,*(end_time_img_dump + i)*SEC_TO_MSEC,*(execution_time_img_dump + i),jitter_img_dump*SEC_TO_MSEC);
			}
			else
			{
				jitter_img_dump = (*(start_time_img_dump + i - 1) + (delay_10hz.tv_nsec/NSEC_PER_SEC)) - (*(start_time_img_dump + i)) ;
				fprintf(fptr_S4,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_img_dump + i)*SEC_TO_MSEC,*(end_time_img_dump + i)*SEC_TO_MSEC,*(execution_time_img_dump + i),jitter_img_dump*SEC_TO_MSEC);      //Write all values calculated in file
			}

		}
	}

    syslog(LOG_CRIT,"Service 4 worst execution time= %lf\n",wcet_img_dump);
	syslog(LOG_CRIT,"Service 4 average execution time= %lf\n",total_exec_time_img_dump/(MAX_COUNT));
    fclose(fptr_S4);
}

void print_image_store_analysis()
{
	total_exec_time_image_store = 0;
	fptr_S2 = fopen(filename_S2,"w+");
	fprintf(fptr_S2,"S2 Count,Start_Time(in sec),End_Time(in sec),Image_Dump_Time(in msec),Jitter(in sec)");
	for(int i=0;i<(MAX_COUNT);i++)
	{
		*(execution_time_image_store + i) = (*(end_time_image_store + i) - *(start_time_image_store + i))*SEC_TO_MSEC;
		if(i == 0)
		{
			wcet_image_store = *(execution_time_image_store + i);
		}
		if(wcet_image_store < *(execution_time_image_store + i))
		{
			wcet_image_store = *(execution_time_image_store + i);
		}
		total_exec_time_image_store += *(execution_time_image_store + i);
	}

	for(int i=0;i<(MAX_COUNT);i++)
	{

		if(frequency_to_capture == 1)
		{
			if(i == 0)
			{
				jitter_image_store = 0;
				fprintf(fptr_S2,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_image_store + i)*SEC_TO_MSEC,*(end_time_image_store + i)*SEC_TO_MSEC,*(execution_time_image_store + i),jitter_image_store*SEC_TO_MSEC);
			}
			else
			{
				jitter_image_store = (*(start_time_image_store + i - 1) + delay_1hz.tv_sec) - (*(start_time_image_store + i)) ;
				fprintf(fptr_S2,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_image_store + i)*SEC_TO_MSEC,*(end_time_image_store + i)*SEC_TO_MSEC,*(execution_time_image_store + i),jitter_image_store*SEC_TO_MSEC);      //Write all values calculated in file
			}
		}
		else if (frequency_to_capture == 10)
		{
			if(i == 0)
			{
				jitter_image_store = 0;
				fprintf(fptr_S2,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_image_store + i)*SEC_TO_MSEC,*(end_time_image_store + i)*SEC_TO_MSEC,*(execution_time_image_store + i),jitter_image_store*SEC_TO_MSEC);
			}
			else
			{
				jitter_image_store = (*(start_time_image_store + i - 1) + (delay_10hz.tv_nsec/NSEC_PER_SEC)) - (*(start_time_image_store + i)) ;
				fprintf(fptr_S2,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_image_store + i)*SEC_TO_MSEC,*(end_time_image_store + i)*SEC_TO_MSEC,*(execution_time_image_store + i),jitter_image_store*SEC_TO_MSEC);      //Write all values calculated in file
			}

		}
	}

    syslog(LOG_CRIT,"Service 2 worst execution time= %lf\n",wcet_image_store);
	syslog(LOG_CRIT,"Service 2 average execution time= %lf\n",total_exec_time_image_store/(MAX_COUNT));
    fclose(fptr_S2);
}


void print_image_comp_analysis()
{
	total_exec_time_img_comp = 0;
	fptr_S3 = fopen(filename_S3,"w+");
	fprintf(fptr_S3,"S3 Count,Start_Time(in sec),End_Time(in sec),Image_comp_Time(in msec),Jitter(in sec)");
	for(int i=0;i<(MAX_COUNT);i++)
	{
		*(execution_time_img_comp + i) = (*(end_time_img_comp + i) - *(start_time_img_comp + i))*SEC_TO_MSEC;
		if(i == 0)
		{
			wcet_img_comp = *(execution_time_img_comp + i);
		}
		if(wcet_img_comp < *(execution_time_img_comp + i))
		{
			wcet_img_comp = *(execution_time_img_comp + i);
		}
		/* Calculate total time of execution for image capture thread */
		total_exec_time_img_comp += *(execution_time_img_comp + i);
	}

	for(int i=0;i<(MAX_COUNT);i++)
	{
		if(frequency_to_capture == 1)
		{
			if(i == 0)
			{
				jitter_img_comp = 0;
				fprintf(fptr_S3,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_img_comp + i)*SEC_TO_MSEC,*(end_time_img_comp + i)*SEC_TO_MSEC,*(execution_time_img_comp + i),jitter_img_comp*SEC_TO_MSEC);
			}
			else
			{
				jitter_img_comp = (*(start_time_img_comp + i - 1) + delay_1hz.tv_sec) - (*(start_time_img_comp + i)) ;
				fprintf(fptr_S3,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_img_comp + i)*SEC_TO_MSEC,*(end_time_img_comp + i)*SEC_TO_MSEC,*(execution_time_img_comp + i),jitter_img_comp*SEC_TO_MSEC);      //Write all values calculated in file
			}
		}
		else if (frequency_to_capture == 10)
		{
			if(i == 0)
			{
				jitter_img_comp = 0;
				fprintf(fptr_S3,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_img_comp + i)*SEC_TO_MSEC,*(end_time_img_comp + i)*SEC_TO_MSEC,*(execution_time_img_comp + i),jitter_img_comp*SEC_TO_MSEC);
			}
			else
			{
				jitter_img_comp = (*(start_time_img_dump + i - 1) + (delay_10hz.tv_nsec/NSEC_PER_SEC)) - (*(start_time_img_dump + i)) ;
				fprintf(fptr_S3,"\n%d,%lf,%lf,%lf,%lf",i,*(start_time_img_comp + i)*SEC_TO_MSEC,*(end_time_img_comp + i)*SEC_TO_MSEC,*(execution_time_img_comp + i),jitter_img_comp*SEC_TO_MSEC);      //Write all values calculated in file
			}

		}
	}

    syslog(LOG_CRIT,"Service 3 worst execution time= %lf\n",wcet_img_dump);
	syslog(LOG_CRIT,"Service 3 average execution time= %lf\n",total_exec_time_img_dump/(MAX_COUNT));
    fclose(fptr_S3);
}
