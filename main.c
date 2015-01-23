#include <stdio.h>
#include <pthread.h>
#include <inttypes.h>
#include <time.h>

#define OWRLOOP 200000

#define PRLOOP64 200000
#define BASE64 0x1234ABCDEFL
#define ALTERN64 0xABCDEF1234L

#define PRLOOP16 100000000
#define BASE16 ((int16_t)0x12AB)
#define ALTERN16 ((int16_t)0xCD34)

int sharedValue=0;

typedef void*(threadFunction)(void*); // pointer to thread function

// thread_start_info
typedef struct {
    int thread_inx;
    threadFunction* fn;
    int base;
    pthread_t threadID;
} thread_start_info;

thread_start_info threads[10];

// struct aligned or not
typedef struct
__attribute__((__packed__))
{
    char padding;
    int64_t int64;
    int16_t int16;
} shared_struct_t;

// global share struct
shared_struct_t shared_struct;

struct timespec start, finish;
void startTimer(){
    clock_gettime(CLOCK_MONOTONIC, &start);
}
double elapsedms(){
    clock_gettime(CLOCK_MONOTONIC, &finish);
    double elapsedms;
    //printf("%lld %lld\n",start.tv_nsec,finish.tv_nsec);
    elapsedms = (finish.tv_sec - start.tv_sec) * 1000.0;
    elapsedms += (finish.tv_nsec - start.tv_nsec) / 1000000.0;
    return elapsedms;
}

void launch_thread(int inx, threadFunction* fn, int base){
    threads[inx].base=base;
    threads[inx].fn=fn;
    threads[inx].thread_inx=inx;
    int result=pthread_create(&(threads[inx].threadID),NULL,fn, &(threads[inx]));
    if (result!=0) {
            printf("ERROR: pthread_create t%d returned %d\n",inx,result);
        }
}

void wrt(int threadname, int toSet ){
        sharedValue = toSet;
        int readed = sharedValue;
		if (readed!=toSet) {
            printf("thread t%d writes %d reads %d\n",threadname,toSet,readed);
        }
}

/* This is our thread function.  It is like main(), but for a thread */
void* overwriteThreadFn(void* arg)
{
    thread_start_info info;
    info=(*(thread_start_info*)arg);

    int64_t start=clock();
    printf("thread t%d start %llx\n",info.thread_inx, arg);

	int i = 0;
	while(i < OWRLOOP )
	{
		//usleep(1);
        wrt(info.thread_inx, i+info.base);
		++i;
	}

    uintmax_t end=clock();
    printf("%.3f thread t%d finished\n",elapsedms(), info.thread_inx);
    fflush(stdout);
	return NULL;
}

/*
 * owerwrite_test
 */
int owerwrite_test(void)
{
    struct timespec start, finish;

	printf("--------------\n");
	printf("Overwrite test\n");
    startTimer();

    pthread_t pth;	// this is our thread identifier
	int i = 0;

	/* Create worker thread */
    launch_thread(0,overwriteThreadFn,0);

	/* Create worker thread */
    launch_thread(1,overwriteThreadFn,62000);

	/* Create worker thread */
    launch_thread(2,overwriteThreadFn,92000);

	while(i < OWRLOOP )
	{
		//usleep(1);
        wrt(9,-i);
		++i;
	}

	/* wait for our thread to finish before continuing */
	pthread_join(threads[0].threadID, NULL /* void ** return value could go here */);
	pthread_join(threads[1].threadID, NULL /* void ** return value could go here */);
	pthread_join(threads[2].threadID, NULL /* void ** return value could go here */);

    printf("%.3f ms Overwrite test finished\n",elapsedms());
    fflush(stdout);
	return 0;
}


/*
 * partial read test 64
 * ++++++++++++++++++++
 */

void* partialReadThread64Fn(void* arg)
{
    thread_start_info info;
    info=*(thread_start_info*)arg;

    printf("%.3f thread t%d started\n",elapsedms(),info.thread_inx);

	int i = 0;
	while(i < PRLOOP64 )
	{
		//usleep(1);
        int64_t readed = shared_struct.int64;
		if (readed!=BASE64 && readed!=ALTERN64) {
            printf("%.3f thread t%d reads %llx\n",elapsedms(),info.thread_inx,readed);
        }
		++i;
	}

    printf("%.3f thread t%d finished\n",elapsedms(),info.thread_inx);
    fflush(stdout);
	return NULL;
}

int partial_read64_test(void)
{
	printf("--------------\n");
	printf("Partial read test 64\n");
    printf("msg if !=%lld and !=%llx\n",BASE64,ALTERN64);
    printf("alternated between %lld and %llx\n",BASE64,ALTERN64);
    printf("addr is %llx\n",&shared_struct.int64);
    startTimer();

    pthread_t pth;	// this is our thread identifier
	int i = 0;

    shared_struct.int64 = BASE64;

	/* Create worker thread */
    launch_thread(0,partialReadThread64Fn,0);

	/* Create worker thread */
    launch_thread(1,partialReadThread64Fn,62000);

	/* Create worker thread */
    launch_thread(2,partialReadThread64Fn,92000);

	while(i < PRLOOP64*2 )
	{
		shared_struct.int64 = ALTERN64;
        //usleep(1);
        shared_struct.int64 = BASE64;
		++i;
	}

	/* wait for our thread to finish before continuing */
	pthread_join(threads[0].threadID, NULL /* void ** return value could go here */);
	pthread_join(threads[1].threadID, NULL /* void ** return value could go here */);
	pthread_join(threads[2].threadID, NULL /* void ** return value could go here */);

    printf("%.3f partial read 64 test finished\n",elapsedms());
    fflush(stdout);
	return 0;
}

/*
 * partial read test 16
 * ++++++++++++++++++++
 */

void* partialReadThread16Fn(void* arg)
{
    thread_start_info info;
    info=*(thread_start_info*)arg;

    printf("%.3f thread t%d started\n",elapsedms(),info.thread_inx);

	int i = 0;
	while(i < PRLOOP16 )
	{
		//usleep(1);
        int16_t readed = shared_struct.int16;
		if (readed!=BASE16 && readed!=ALTERN16) {
            printf("%.3f thread t%d reads %" PRIx16 "\n",elapsedms(),info.thread_inx,readed);
        }
		++i;
	}

    printf("%.3f thread t%d finished\n",elapsedms(),info.thread_inx);
    fflush(stdout);
	return NULL;
}

int partial_read16_test(void)
{
	printf("--------------\n");
	printf("Partial read test 16\n");
    printf("msg if !=%" PRIx16 " and !=%" PRIx16 "\n",BASE16,ALTERN16);
    printf("alternated between %" PRIx16 " and %" PRIx16 "\n",BASE16,ALTERN16);
    printf("addr is %llx\n",&shared_struct.int16);
    startTimer();

    pthread_t pth;	// this is our thread identifier
	int i = 0;

    shared_struct.int16 = BASE16;

	/* Create worker thread */
    launch_thread(0,partialReadThread16Fn,0);

	/* Create worker thread */
    launch_thread(1,partialReadThread16Fn,62000);

	/* Create worker thread */
    launch_thread(2,partialReadThread16Fn,92000);

	while(i < PRLOOP16*2 )
	{
		shared_struct.int16 = ALTERN16;
        //usleep(1);
        shared_struct.int16 = BASE16;
		++i;
	}

	/* wait for our thread to finish before continuing */
	pthread_join(threads[0].threadID, NULL /* void ** return value could go here */);
	pthread_join(threads[1].threadID, NULL /* void ** return value could go here */);
	pthread_join(threads[2].threadID, NULL /* void ** return value could go here */);

    printf("%.3f partial read 16 test finished\n",elapsedms());
    fflush(stdout);

	return 0;
}

int main(void)
{
    //owerwrite_test();

    //partial_read64_test();

    partial_read16_test();

}