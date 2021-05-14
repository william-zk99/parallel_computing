/*
 * using openMP to solve
 * PSRS sorting algorithm 
 *
 * the process of PSRS 
 * comes from our course slide
 * 
 * PB18111679 fanweneddie
*/

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <omp.h>
// for senitel in merge sort
#define INFI 999999 

// implement PSRS algorithm to sort the array
void My_PSRS(int thread_num,int ele_num,int* array);

// implement merge sort on _array(_start,_end)
void Merge_sort(int _start,int _end,int* _array);
// implement merge as an auxiliary function for merge sort
void Merge(int _start,int _mid,int _end,int* _array);

// implement PSRS algorithm on OpenMP to sort the array
// @_thread_num: number of threads for parallelism
// @_ele_num: number of elements to sort
// @_array: array to store the elements(which is accessed by each thread)
// the sorted array will still be placed in _array.
void My_PSRS(int _thread_num,int _ele_num,int* _array){

    // ************************************************** //
    // STEP 1: uniform partition to each thread
    // ************************************************** //
    
    // the start index of array in each thread
    int* start_index = (int*)malloc( (_thread_num + 1) * sizeof(int));
    // partition the array uniformly
    int gap = (int)ceil(_ele_num / _thread_num);
    for(int i = 0;i < _thread_num;++i){
        start_index[i] = gap * i;
    }
    start_index[_thread_num] = _ele_num;

    // ************************************************** //
    // STEP 2: local sort for each thread
    // STEP 3: sampling uniformly
    // ************************************************** //

    // the array of samples
    int *samples = (int*)malloc(_thread_num * _thread_num * sizeof(int));
    // set the threads
    omp_set_num_threads(_thread_num);
    // start the parallel domain
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        std::sort(_array + start_index[id], 
                    _array + start_index[id + 1]);
        // uniform sampling
        int step = (int)floor( (start_index[id + 1] - start_index[id]) / _thread_num );
        for(int i = 0;i < _thread_num;++i) {
            samples[ _thread_num*id + i] = _array[ start_index[id] + step * i];
        }
    }

    // ************************************************** //
    // STEP 4: sort the samples
    // ************************************************** //

    std::sort(samples,samples + _thread_num*_thread_num);

    // ************************************************** //
    // STEP 5: choose the pivots uniformly
    // ************************************************** //

    // the array of pivots
    int *pivots = (int*)malloc(_thread_num - 1);
    // get the pivots uniformly
    // btw, the step is _thread_num
    for(int i = 0; i < _thread_num - 1;++i){
        pivots[i] = samples[ (i + 1) * _thread_num ];
    }

    // ************************************************** //
    // STEP 6: partition according to pivots
    // ************************************************** //

    // this two-dimensional array shows the start index of partition
    // e.g. partition_start[2][1] = 6,partition_start[2][2] = 8
    // thread 2's 6th ~ 7th element should be partitioned to first part
    int **partition_start;
    partition_start = (int**)malloc( _thread_num * sizeof(int*) );
    for(int i = 0; i < _thread_num;++i){
        partition_start[i] = (int*)malloc( (_thread_num + 1) * sizeof(int) );
    }

    // new array to temporarily store elements after global change
    int *new_array = (int*)malloc(_ele_num * sizeof(int));

    // the size of each partition
    int *partition_size = (int*)malloc(_thread_num*sizeof(int));
    for(int i = 0; i < _thread_num;++i)
        partition_size[i] = 0;

    // the starting index of each partition in new_array
    int *new_start_index = (int*)malloc(_thread_num*sizeof(int));
    new_start_index[0] = 0;

    // start the parallelism domain again
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        // init partition_start[id]
        for(int i = 0; i < _thread_num;++i)
            partition_start[id][i] = start_index[id];
        partition_start[id][_thread_num] = start_index[id + 1];
        int j = 0;
        for(int i = start_index[id]; i < start_index[id + 1];++i){
            if( _array[i] > pivots[j] ){
                j++;
                partition_start[id][j] = i;
            }
            if(j == _thread_num - 1)
                break;
        }
        // add patch to partition_start[id]
        for(int i = 1;i < _thread_num;++i) {
            if(partition_start[id][i] == 0)
                partition_start[id][i] = partition_start[id][i-1];
        }

        // get partition_size in critical section
        #pragma omp critical
        {
            for(int i = 0; i < _thread_num;++i){
                partition_size[i] += partition_start[id][i+1] - partition_start[id][i];
            }
        }
    }

    // the start position in new_array of each partition
    int **new_partition_start;
    new_partition_start = (int**)malloc( _thread_num * sizeof(int*) );
    for(int i = 0; i < _thread_num;++i) {
        new_partition_start[i] = (int*)malloc( (_thread_num + 1) * sizeof(int) );
    }

    // get new_partition_start by leveraging partition_start
    new_partition_start[0][0] = 0;
    new_partition_start[0][_thread_num] = _ele_num;
    for(int j = 1; j < _thread_num;++j)
        new_partition_start[0][j] = new_partition_start[0][j-1] + partition_size[j-1]; 

    for(int j = 0;j < _thread_num;++j){
        for(int i = 1;i < _thread_num;++i){
            new_partition_start[i][j] = new_partition_start[i-1][j] + 
                    partition_start[i-1][j+1] - partition_start[i-1][j];
        }
    }

    // ************************************************** //
    // STEP 7: global exchange
    // ************************************************** //
    // start the parallelism domain again
    #pragma omp parallel
    {   
        int id = omp_get_thread_num();
        // global exchange from array to new_array
        for(int j = 0; j < _thread_num;++j){
            int old_start = partition_start[id][j]; 
            int new_start = new_partition_start[id][j];
            int num_copy = partition_start[id][j+1] - partition_start[id][j];
            for(int index = 0; index < num_copy; index++)
                new_array[index + new_start] = _array[index + old_start]; 
        }
    }

    // ************************************************** //
    // STEP 8: merge sort
    // ************************************************** //
    // start the parallelism domain again
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        // the start and end of new_array to be merged
        int start = new_partition_start[0][id];
        int end = new_partition_start[0][id+1] - 1;
        Merge_sort(start,end,new_array);
    }

    // copy from new_array to _array
    // (it may be slow,but what else can I do ?)
    for(int i = 0; i < _ele_num;++i){
        _array[i] = new_array[i];
    }

    // free those malloc ptrs
    free(start_index);
    free(samples);
    for(int i = 0; i < _thread_num;++i){
        free(partition_start[i]);
        free(new_partition_start[i]);
    }
    free(partition_start);
    free(new_partition_start);
    free(new_array);
    free(partition_size);
}

// implement ascending merge sort on _array(_start,_end)
// @_start: starting index
// @_end: ending index
// @_array: part of which to be sorted
void Merge_sort(int _start,int _end,int *_array) {
    if(_start < _end){
        int mid = (_start + _end)/2;
        // divide
        Merge_sort(_start, mid, _array);
        Merge_sort(mid + 1, _end, _array);
        // merge
        Merge(_start,mid,_end, _array);
    }
}

// merge the sorted array of _array[_start,_mid] 
// and _array[_mid + 1,_end] to _array[_start,_end]
void Merge(int _start,int _mid,int _end,int* _array) {
    int i,j;
    // size of Left and Right
    int left_size = _mid - _start + 1;
    int right_size = _end - _mid;
    // initialize array Left and Right
    int* Left = (int*) malloc( (left_size + 1)*sizeof(int) );
    int* Right = (int*) malloc( (right_size + 1)*sizeof(int) );
    for(i = 0;i < left_size;++i)
        Left[i] = _array[_start + i];
    for(j = 0;j < right_size;++j)
        Right[j] = _array[_mid + 1 + j];
    // a sentinel to show that 
    // the Left or Right has nothing remaining
    Left[left_size] = INFI;
    Right[right_size] = INFI;
    // merge the data
    i = 0;
    j = 0;
    for(int k = _start; k <= _end;++k)
    {
        // Left[i,...,mid] > Right[j]
        // so there are (mid - start - i + 1) 
        // pairs of inversion 
        if( Left[i] > Right[j] )
        {
            _array[k] = Right[j];
            j++;
        }
        // Left[i] <= Left[j]
        else
        {
            _array[k] = Left[i];
            i++;
        }
    }
    free(Left);
    free(Right);
}


// @argv[1]: the number of threads
// @argv[2]: the number of elements
// the argv later is the integer elements to be sorted
// show the array before sorting and after sorting
int main(int argc, char* argv[]){
    // Checking input arguments
    if (argc < 3) {
        printf("Use: %s <Number of threads> <Number of Elements>\n", argv[0]);
        return 1;
    }

    // number of threads
    int thread_num = atoi(argv[1]);
    // number of elements
    int ele_num = atoi(argv[2]);

    // check whether the number of input elements in correct
    if(argc - 3 < ele_num){
        printf("Not enough input elements.\n");
        return 1;
    }
    
    if(argc - 3 > ele_num){
        printf("Too much input elements.\n");
        return 1;
    }

    // check whether there are too many threads to destroy the partition
    if(thread_num * thread_num > ele_num){
        printf("thread number exceeds the need.\n");
        return 1;
    }

    // get the array of the elements
    int* array = (int*)malloc( ele_num * sizeof(int) );
    for(int i = 0; i < ele_num;++i){
        sscanf(argv[i+3],"%d",&array[i]); 
    }

    // show the original input array
    printf("original array is\n");
    for(int i = 0; i < ele_num;++i){
        printf("%d ",array[i]);
    }
    printf("\n");

    // call PSRS function to sort
    My_PSRS(thread_num,ele_num,array);

    // show the sorted array
    printf("sorted array is\n");
    for(int i = 0; i < ele_num;++i){
        printf("%d ",array[i]);
    }
    printf("\n");
    free(array);
}