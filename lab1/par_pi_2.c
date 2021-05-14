/*
 * method 2:
 * calculate the approximate value of pi by 
 * computing parallelism integral in a range
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
    double x, pi, sum[NUM_THREADS];
    step = 1.0/(double) num_steps;
    // set 2 theads
    omp_set_num_threads(NUM_THREADS);
    // start the parallel domain for each thread
    #pragma omp parallel private(i)
    {
        double x;
        int id;
        id = omp_get_thread_num();
        sum[id] = 0;
        // thread 0 calculates case i = 0 ~ 49999
        // thread 1 calculates case i = 50000 ~ 99999
        #pragma omp for
        for (i = 0;i < num_steps; ++i){
            x = (i + 0.5) * step;
            sum[id] += 4.0 / ( 1.0 + x*x );
        }
        
    }
    for(i = 0, pi = 0.0;i < NUM_THREADS;++i) 
        pi += sum[i] * step;
    printf("%lf\n",pi);
}