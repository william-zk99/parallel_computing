/*
 * use mpi to implement PSRS sorting algorithm
 * 
 * the process of PSRS 
 * comes from our course slide
 * 
 * I have borrowed some ideas from myl7's github
 * https://github.com/myl7/paracomp2021
 * 
 * PB18111679 fanweneddie
*/

#include <fstream>
#include <iostream>
#include <array>
#include <vector>
#include <math.h>
#include <algorithm>
#include <mpi.h>

constexpr auto MAX_THREAD_NUM = 10;
void serial(int _thread_num);
void parallel(int _thread_ID,int _thread_num);
int* get_input();

// get the input int array from the input file
// allocate the array and return it
// @_elem_num: the number of elements in the array
//          we need to revise it in the function
int* get_input(int& _elem_num){
    std::ios::sync_with_stdio(false);
    std::ifstream in("input.txt");
    if (!in) {
        std::cerr << "Getting input failed\n";
    }
    in >> _elem_num;
    int *array = new int[_elem_num];
    for (int i = 0; i < _elem_num; i++) {
        in >> array[i];
    }
    in.close();
    return array;
}

// serial working part
// for thread 0 
// do the serial work in PSRS,
// that is, to do
// (1) uniform partition
// (4) sample sorting
// (5) pivot select
// (7) global exchange
// (8) merge sort
// @_thread_num: number of parallel threads
void serial(int _thread_num){

    // get the input from the file
    int elem_num;
    int* array = get_input(elem_num);
    // show the input array
    std::cout << "original array is\n";
    for(int i = 0; i < elem_num;++i){
        std::cout << array[i] << " ";
    }
    std::cout << "\n";

    // ------------------------------------------------------------
    // (1) uniform partition
    // ------------------------------------------------------------
    // the index of start and end for each partition
    int start_index,end_index;
    // the gap between two neighbor partition
    int gap = (int)ceil(elem_num / _thread_num );
    for(int i = 1; i <= _thread_num;++i){
        start_index = gap * (i - 1);
        end_index = gap * i < elem_num ? gap * i : elem_num;
        int partition_size = end_index - start_index;
        // send the elements of each partition
        // to the corresponding thread
        MPI_Send(&partition_size,1,MPI_INT, i, 1, MPI_COMM_WORLD);
        MPI_Send(array + start_index, 
            end_index - start_index, MPI_INT, i, 2, MPI_COMM_WORLD);
    }

    // receive the samples from those threads
    int* samples = new int [_thread_num * _thread_num];
    // the offset in samples for each reception
    int offset = 0;
    for(int i = 0;i < _thread_num;++i){
        MPI_Recv(samples + offset, _thread_num, 
            MPI_INT,MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        offset += _thread_num;
    }

    // ------------------------------------------------------------
    // (4) sample sorting
    // ------------------------------------------------------------
    std::sort(samples,samples + offset);

    // ------------------------------------------------------------
    // (5) pivot select
    // ------------------------------------------------------------
    int *pivots = new int[_thread_num - 1];
    // get the pivots uniformly
    // the step is _thread_num
    for(int i = 0; i < _thread_num - 1;++i){
        pivots[i] = samples[ (i + 1) * _thread_num ];
    }
    // send the pivots to thread 1 ~ _thread_num
    for(int i = 1; i <= _thread_num;++i){
        MPI_Send(pivots,_thread_num - 1,MPI_INT, i, 4, MPI_COMM_WORLD);
    }

    // ------------------------------------------------------------
    // (7) global exchange & (8) merge sort
    // ------------------------------------------------------------
    // receive the partitions sent by thread 1 ~_thread_num - 1
    std::array<std::vector<int>,MAX_THREAD_NUM> replace_list;
    for (int thread_index = 1; thread_index <= _thread_num; ++ thread_index){
        for(int part_index = 0; part_index < _thread_num; ++ part_index){
            // get the partition in a thread
            unsigned len;
            MPI_Recv(&len, 1, MPI_UNSIGNED, thread_index, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::vector<int> partition(len, 0);
            MPI_Recv(&partition[0], len, MPI_INT, thread_index, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            int next_part_index = partition[len-1];
            // tmp for merging
            std::vector<int> tmp;
            std::merge(replace_list[next_part_index].begin(), replace_list[next_part_index].end(), 
                partition.begin(), --partition.end(), std::back_inserter(tmp));
            replace_list[next_part_index] = tmp;
        }
    }
    
    
    // new array to store the sorted elements
    std::vector<int> new_array;
    // concat the replace_lists to get the new_array
    for(int i = 0; i < _thread_num;++i){
        new_array.insert(new_array.end(),replace_list[i].begin(),replace_list[i].end());
    }

    std::cout << "after sorting, the array is\n";
    for (auto elem : new_array) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    delete []array;
    delete []samples;
    delete []pivots;
}

// parallel working part
// for thread 1 ~ _thread_num - 1
// do the parallel work in PSRS
// that is, to do
// (2) local sort
// (3) uniform sampling
// (6) privot partition
void parallel(int _thread_ID,int _thread_num){
    
    // receive the uniform partition from thread 0
    int array_size;
    MPI_Recv(&array_size,1, MPI_INT, 0, 1,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    int* local_array = new int[array_size];
    MPI_Recv(local_array, array_size, MPI_INT, 0, 2,
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // ------------------------------------------------------------
    // (2) local sort
    // ------------------------------------------------------------
    std::sort(local_array,local_array + array_size);

    // ------------------------------------------------------------
    // (3) normal sampling
    // ------------------------------------------------------------
    // get _thread_num samples in a partition
    int gap = (int)floor( array_size / _thread_num );
    int* local_samples = new int[_thread_num];
    int index = 0;
    for(int i = 0;i < _thread_num;++i){
        local_samples[i] = local_array[index];
        index += gap; 
    }
    // send the samples to thread_0 for next step
    MPI_Send(local_samples,_thread_num, MPI_INT, 0, 3, MPI_COMM_WORLD);

    // ------------------------------------------------------------
    // (6) privot partition
    // ------------------------------------------------------------
    // receive the pivots sent from the first partition
    int *local_pivots = new int[_thread_num - 1];
    MPI_Recv(local_pivots,_thread_num - 1, MPI_INT,
         0, 4,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    // partition according to the pivots
    // replace_list[i] means the elements to be sent to thread i
    std::array<std::vector<int>, MAX_THREAD_NUM> replace_list;
    int pivot_index = 0;
    for(int i = 0;i < array_size;){
        if(local_array[i] < local_pivots[pivot_index]){
            replace_list[pivot_index].push_back(local_array[i]);
            i++;
        }
        else if(pivot_index == _thread_num - 1){
            replace_list[pivot_index - 1].push_back(local_array[i]);
            i++;
        }
        else{
            pivot_index++;
        }
    }
    // send the result of partition to thread 0
    for(int i = 0;i < _thread_num;++i){
        int len = (unsigned)replace_list[i].size();
        // marks the object of this partition.
        replace_list[i].push_back(i);
        len++;
        MPI_Send(&len, 1, MPI_UNSIGNED, 0, 5, MPI_COMM_WORLD);
        MPI_Send(&replace_list[i][0], len, MPI_INT, 0, 6, MPI_COMM_WORLD);
    }

    delete [] local_array;
    delete [] local_samples;
    delete [] local_pivots;
}

int main(int argc, const char *const argv[]) {
    MPI_Init(&argc, const_cast<char ***>(&argv));
    // the thread ID of the current thread
    int thread_ID;
    MPI_Comm_rank(MPI_COMM_WORLD, &thread_ID);
    // the total number of threads
    int thread_num;
    MPI_Comm_size(MPI_COMM_WORLD, &thread_num);
    thread_num = 4;
    // for ID = 0, do the serial work
    if(thread_ID == 0){
        serial(thread_num - 1);
    }
    // for ID != 0, do parallel work
    else{
        parallel(thread_ID,thread_num - 1);
    }     
    MPI_Finalize();
    return 0;  
}