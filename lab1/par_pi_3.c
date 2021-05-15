/*
 * method 3:
 * calculate the approximate value of pi by 
 * computing parallelism integral over step
 * and aggregating the result in critical section
 * 
 * borrowed from lab guide
 * PB18111679 fanweneddie
*/

#include <stdio.h>
#include <omp.h>
#include <time.h>

static long num_steps = 100000;
double step;
#define NUM_THREADS 2
#define TEST_TIME 100

int main () {
    int i;
    double pi = 0.0;
    double x = 0.0;
    step = 1.0/(double) num_steps;
    double sum = 0.0;
    double result[TEST_TIME];
    clock_t start,end;

    start = clock();
    for(int t = 0;t < TEST_TIME;++t){
        x = 0;
        sum = 0;
        pi = 0;
        // set 2 threads
        omp_set_num_threads(NUM_THREADS);
        // the start of parallelism domain
        // i,x,sum are private for each thread
        // that is, they are not shared objects
        // (note that the original program in guide is incorrect!!!)
        #pragma omp parallel private(i,x,sum)
        {
            int id;
            id = omp_get_thread_num();
            for (i = id, sum = 0.0;i < num_steps; i = i + NUM_THREADS) {
                x = (i + 0.5)*step;
                sum += 4.0/(1.0 + x*x);
            }
            // in critical section
            // every thread should access it mutually exclusively
            #pragma omp critical
            pi += sum*step;
        }
        result[t] = pi;
    }

    end = clock();
    pi = 0.0;
    for(int t = 0; t < TEST_TIME;++t){
        pi += result[t];
    }
    pi /= TEST_TIME;
    printf("result 3: %lf\n",pi);
    printf("time   3: %lfs\n",(double)(end-start)/CLOCKS_PER_SEC );
    printf("-----------------------------------------\n");
    return 0;
}