

//#include "/usr/local/cuda-10.1/include/cuda_runtime.h"
#include "cuda_runtime.h"
#include "device_gather.h"
#include <iostream>
#include <stdio.h>
#include <assert.h>
#define WARP_SIZE 32

#define SAFE_CALL( CallInstruction ) { \
    cudaError_t cuerr = CallInstruction; \
    if(cuerr != cudaSuccess) { \
         printf("CUDA error: %s at call \"" #CallInstruction "\"\n", cudaGetErrorString(cuerr)); \
		 throw "error in CUDA API function, aborting..."; \
    } \
}

#define SAFE_KERNEL_CALL( KernelCallInstruction ){ \
    KernelCallInstruction; \
    cudaError_t cuerr = cudaGetLastError(); \
    if(cuerr != cudaSuccess) { \
        printf("CUDA error in kernel launch: %s at kernel \"" #KernelCallInstruction "\"\n", cudaGetErrorString(cuerr)); \
		throw "error in CUDA kernel launch, aborting..."; \
    } \
    cuerr = cudaDeviceSynchronize(); \
    if(cuerr != cudaSuccess) { \
        printf("CUDA error in kernel execution: %s at kernel \"" #KernelCallInstruction "\"\n", cudaGetErrorString(cuerr)); \
		throw "error in CUDA kernel execution, aborting..."; \
    } \
}



__global__ void gather_warp_per_vertex(  int *v_array,  int *e_array,  int *dest_labels ,  int *labels,
                                         long  long edges,   long long vertices) {

      long int i = threadIdx.x + blockIdx.x * blockDim.x;

    if (i < vertices*WARP_SIZE) {

          long long warp_num = i / 32 ;
        short warp_pos = i % 32;
          long long int v_begin = v_array[warp_num];
          long long int v_end = v_array[warp_num + 1];
          long long position = v_begin + warp_pos;
        while(position < v_end){
            dest_labels[position] = labels[e_array[position]];
            position+=WARP_SIZE;
        }
    }
}






void CSR_GRAPH::move_to_device(  int* dest_labels,   int* labels,   int* dev_dest_labels ,  int* dev_labels) {

    SAFE_CALL((cudaMalloc((void**)&dev_v_array,(size_t)sizeof(this->v_array[0])*(vertices_count+1))));
    SAFE_CALL((cudaMalloc((void**)&dev_e_array,(size_t)sizeof(this->e_array[0])*edges_count)));
    if(weighted){
        SAFE_CALL((cudaMalloc((void**)&dev_weigths,(size_t)sizeof(this->e_array[0])*edges_count)));
    }


    SAFE_CALL((cudaMemcpy(dev_dest_labels,dest_labels,(size_t)(sizeof(this->e_array[0])*edges_count),cudaMemcpyHostToDevice)));
    SAFE_CALL((cudaMemcpy(dev_v_array,v_array,(size_t)((vertices_count+1)* sizeof(this->v_array[0])),cudaMemcpyHostToDevice)));
    SAFE_CALL((cudaMemcpy(dev_e_array,e_array,(size_t)(sizeof(this->e_array[0])*edges_count),cudaMemcpyHostToDevice)));
    SAFE_CALL((cudaMemcpy(dev_weigths,weights,(size_t)(sizeof(this->e_array[0])*edges_count),cudaMemcpyHostToDevice)));
    SAFE_CALL((cudaMemcpy(dev_labels,labels,(size_t)(sizeof(this->v_array[0])*(vertices_count)),cudaMemcpyHostToDevice)));

    std::cout<<"moved to device"<<std::endl;

}


void CSR_GRAPH::move_to_host (  int* dest_labels,   int* labels,   int* dev_dest_labels ,  int* dev_labels) {

    SAFE_CALL((cudaMemcpy(v_array,dev_v_array,(size_t)(vertices_count+1)* sizeof(this->v_array[0]),cudaMemcpyDeviceToHost)));
    SAFE_CALL((cudaMemcpy(e_array,dev_e_array,(size_t)edges_count* sizeof(this->e_array[0]),cudaMemcpyDeviceToHost)));
    SAFE_CALL((cudaMemcpy(weights,dev_weigths,(size_t)edges_count* sizeof(this->e_array[0]),cudaMemcpyDeviceToHost)));
    SAFE_CALL((cudaMemcpy(labels,dev_labels,(size_t)(vertices_count)* sizeof(this->v_array[0]),cudaMemcpyDeviceToHost)));
    SAFE_CALL((cudaMemcpy(dest_labels,dev_dest_labels,(size_t)edges_count* sizeof(this->e_array[0]),cudaMemcpyDeviceToHost)));

    //SAFE_CALL(cudaFree(dev_v_array));
    //SAFE_CALL(cudaFree(dev_e_array));
    //if(weighted){
    //    SAFE_CALL(cudaFree(dev_weigths));
    //}

    std::cout<<"moved back"<<std::endl;

}