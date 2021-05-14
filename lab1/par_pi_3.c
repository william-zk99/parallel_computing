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

static long num_steps = 100000;
double step;
#define NUM_THREADS 2

void main () {
    int i;
    double pi = 0.0;
    double sum = 0.0;
    double x = 0.0;
    step = 1.0/(double) num_steps;
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
    printf("%lf\n",pi);
}