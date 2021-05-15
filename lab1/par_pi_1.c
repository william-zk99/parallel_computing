/* 
 * method 1:
 * calculate the approximate value of pi by 
 * computing parallelism integral over step
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
#define TEST_TIME 10

int main () {
    int i;
    double pi = 0.0;
    double x = 0.0;
    step = 1.0/(double) num_steps;
    double sum[NUM_THREADS];
    double result[TEST_TIME];
    clock_t start,end;

    start = clock();
    for(int t = 0;t < TEST_TIME;++t){
        x = 0;
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
        for(i = 0, pi = 0.0;i < NUM_THREADS;++i) 
            pi += sum[i] * step;
        result[t] = pi;
    }

    end = clock();
    pi = 0.0;
    for(int t = 0; t < TEST_TIME;++t){
        pi += result[t];
    }
    pi /= TEST_TIME;
    printf("result 1: %lf\n",pi);
    printf("time   1: %lfs\n",(double)(end-start)/CLOCKS_PER_SEC );
    printf("-----------------------------------------\n");
    return 0;
}