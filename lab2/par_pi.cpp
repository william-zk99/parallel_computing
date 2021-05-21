/*
 * using MPI to calculate the approximate value of Pi
 * by computing parallelism integral over step
 * 
 * I have borrowed some ideas from myl7's github
 * https://github.com/myl7/paracomp2021
 * 
 * PB18111679 fanweneddie
*/
#include <iostream>
#include <iomanip>
#include <mpi.h>

static long num_steps = 100000;

void calculate_over_step(int _thread_ID,int _thread_num);
double aggregate(int _thread_num);

// calculate the integral 
// by getting sum of the interval over step
// @_thread_ID: current thread ID
// @_thread_num: total number of threads for computing integral
// send back value of the partial integral for aggregation
void calculate_over_step(int _thread_ID,int _thread_num){
    double x;
    double step = 1.0 / num_steps;
    double partial_sum = 0.0;
    // calculate the partial sum over step
    for(int i = _thread_ID;i < num_steps;i += _thread_num){
        x = (i + 0.5)*step;
        partial_sum += 4.0/(1.0 + x*x);
    }
    partial_sum *= step;
    // send the partial_sum to thread 0
    MPI_Send(&partial_sum,1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
}

// aggregate the partial sum computed by thread 1 ~ _thread_num - 1
// @_thread_num: the number of threads
double aggregate(int _thread_num){
    double pi = 0.0;
    double partial_sum;
    // receive the partial_sum calculated by other threads
    // and aggregate them
    for(int i = 0;i < _thread_num - 1;++i){
        MPI_Recv(&partial_sum,1,MPI_DOUBLE,MPI_ANY_SOURCE,
                0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        pi += partial_sum;
    }
    return pi;
}


int main(int argc, const char *const argv[]) {

    MPI_Init(&argc, const_cast<char ***>(&argv));
    // the thread ID of the current thread
    int thread_ID;
    MPI_Comm_rank(MPI_COMM_WORLD, &thread_ID);
    // the total number of threads
    int thread_num;
    MPI_Comm_size(MPI_COMM_WORLD, &thread_num);

    // for ID != 0, calculate the partial sum over step
    if(thread_ID != 0){
        calculate_over_step(thread_ID,thread_num-1);
    }
    // for ID = 0, aggregate the partial sums
    else{
        double pi = aggregate(thread_num);
        std::cout << "pi is " << std::setprecision(8) << pi << std::endl;
    }     
    MPI_Finalize();
    return 0;  
}