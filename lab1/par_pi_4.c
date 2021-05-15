/*
 * method 4:
 * calculate the approximate value of pi by 
 * computing parallelism integral in range
 * and aggregating the result by reduction
 * 
 * borrowed lab guide
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
    double sum = 0.0;
    double x = 0.0;
    step = 1.0/(double) num_steps;
    double result[TEST_TIME];
    clock_t start,end;
    start = clock();
    for(int t = 0;t < TEST_TIME;++t){
        x = 0;
        sum = 0;
        // set 2 threads    
        omp_set_num_threads(NUM_THREADS);
        // every thread can keep a copy of sum
        // at last do "+" reduction to sum
        #pragma omp parallel for reduction(+:sum) private(i,x) 
        // thread 0 computes case 0 ~ 49999
        // thread 1 computes case 50000 ~ 99999
        for(i = 1;i <= num_steps; i++) {
            x = (i - 0.5)*step;
            sum += 4.0/(1.0 + x*x);
        }
        // sum is aggregated by reduction
        pi = sum * step;
        result[t] = pi;
    }
    end = clock();
    pi = 0.0;
    for(int t = 0; t < TEST_TIME;++t){
        pi += result[t];
    }
    pi /= TEST_TIME;
    printf("result 4: %lf\n",pi);
    printf("time   4: %lfs\n",(double)(end-start)/CLOCKS_PER_SEC );
    printf("-----------------------------------------\n");
    return 0;
}