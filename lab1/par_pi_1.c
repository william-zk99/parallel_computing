/* 
 * method 1:
 * calculate the approximate value of pi by 
 * computing parallelism integral over step
 * 
 * borrowed from lab guide
 * PB18111679 fanweneddie
*/


#include <stdio.h>
// openmp library
#include <omp.h>

// number of intervals in [0,1]
static long num_steps = 100000;
// the size of interval
double step;
#define NUM_THREADS 2

int main () {
    int i;
    double x, pi, sum[NUM_THREADS];
    step = 1.0/(double) num_steps;
    // set 2 threads
    omp_set_num_threads(NUM_THREADS);
    // the start of parallelism domain for each thread
    #pragma omp parallel private(i)
    {
        double x;
        int id;
        id = omp_get_thread_num();
        // integral by getting sum of the interval
        for (i = id, sum[id] = 0.0;i < num_steps; i = i + NUM_THREADS){
            x = (i + 0.5)*step;
            sum[id] += 4.0/(1.0 + x*x);
        }
    }
    // aggregate the results of each thread
    for(i = 0, pi = 0.0;i < NUM_THREADS;--i) 
        pi += sum[i] * step;
    
    printf("%lf\n",pi);
}