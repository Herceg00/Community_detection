#include <omp.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <algorithm>
#include "stdlib.h"
#include "CSR_GRAPH.h"
#include <stdio.h>
#include <math.h>
#include <sstream>
#include <string>
#include <vector>
#include "lp.h"
#include "map"

/*#define SAFE_CALL(CallInstruction) { \
    cudaError_t cuerr = CallInstruction; \
    if(cuerr != cudaSuccess) { \
         printf("CUDA error: %s at call \"" #CallInstruction "\"\n", cudaGetErrorString(cuerr)); \
         throw "error in CUDA API function, aborting..."; \
    } \
}

#define SAFE_KERNEL_CALL(KernelCallInstruction){ \
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

*/
void label_stats(unsigned int *labels, unsigned int vertices_count) { // Почему то в map много нулей
    std::map<unsigned int, int> mp;
    for (unsigned int i = 0; i < vertices_count; i++) {
        if (mp.count(labels[i])) {
            mp[labels[i]]++;
        } else {
            mp[labels[i]] = 1;
        }
    }
    std::map<int, int> components;
    for (auto it = mp.begin(); it != mp.end(); it++) {
        if (components.count(it->second)) {
            components[it->second]++;
        } else {
            components[it->second] = 1;
        }
    }
    for (auto it = components.begin(); it != components.end(); it++) {
        if (it->first != 0) {
            cout << "there are " << it->second << " components of size " << it->first << endl;
        }
    }
}



void input(char *filename, bool directed, unsigned int *&src_ids, unsigned int *&dst_ids, unsigned int &vertices_count,
           unsigned int &edges_count, float*& weights) {
    unsigned int max_vertice = 0;

    std::ifstream infile(filename);
    std::string line;
    unsigned int i = 0;

    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        int a, b;
        if (!(iss >> a >> b)) {
            break;
        } else {
            if (max(a, b) > max_vertice) {
                max_vertice = (unsigned) max(a, b);
            }
        }
        i++;
    }
    edges_count = i;
    vertices_count = max_vertice;
    src_ids = new unsigned int[edges_count];
    dst_ids = new unsigned int[edges_count];
    weights = new float[edges_count];

    std::ifstream infile1(filename);
    i = 0;
    while (std::getline(infile1, line)) {
        std::istringstream iss(line);
        unsigned int a, b;
        float c;
        if (!(iss >> a >> b >> c)) {
            break;
        } else {
            src_ids[i] = a;
            dst_ids[i] = b;
            weights[i] = c;
            if (!directed) {
                src_ids[i + 1] = b;
                dst_ids[i + 1] = a;
                weights[i + 1] = c;
            }
        }
        i++;
    }
}

#include "CSR_GRAPH.h"
#include "generator.h"
#include "device_gather.h"

//#include "/usr/local/cuda-10.1/include/cuda_runtime.h"
//#include "cuda_runtime.h"
//#include "/usr/local/cuda-10.1/include/cuda_profiler_api.h"
//#include "cuda_profiler_api.h"

using namespace std;


#ifndef uint32_t
#define uint32_t int
#endif


int main(int argc, char **argv) {
    try {
        //cudaEvent_t start, stop;
        //cudaEventCreate(&start);
        //cudaEventCreate(&stop);
        int threads = omp_get_max_threads();
        int vertices_index;
        int density_degree;
        bool check_flag = false;
        bool test_flag = false;
        char *graph_type;
        bool lp_flag = false;
        bool gather_flag = false;
        char *test_file = NULL;
        for (int i = 1; i < argc; i++) {
            string option(argv[i]);

            if ((option.compare("-scale") == 0) || (option.compare("-s") == 0)) {
                vertices_index = atoi(argv[++i]);
            }

            if ((option.compare("-edges") == 0) || (option.compare("-e") == 0)) {
                density_degree = atoi(argv[++i]);
            }

            if ((option.compare("-check") == 0)) {
                check_flag = true;
            }

            if ((option.compare("-nocheck") == 0)) {
                check_flag = false;
            }
            if ((option.compare("-type") == 0)) {
                graph_type = argv[++i];
            }
            if ((option.compare("-testing") == 0)) {
                test_file = argv[++i];
                test_flag = true;
                cout << "FLAG FOUND" << endl;
            }
            if ((option.compare("-lp")) == 0) {
                lp_flag = true;
            }
            if ((option.compare("-gather")) == 0) {
                gather_flag = true;
            }

        }

        unsigned int vertices_count = pow(2.0, vertices_index);
        unsigned int edges_count = density_degree * vertices_count;
        unsigned int *src_ids = NULL;
        unsigned int *dst_ids = NULL;
        float *weights = new float[edges_count];

        if (!test_flag) {
            src_ids = new unsigned int[edges_count];
            dst_ids = new unsigned int[edges_count];
            cout << "test_flag" << endl;
            if (strcmp(graph_type, "rmat") == 0) {
                R_MAT(src_ids, dst_ids, weights, vertices_count, edges_count, 45, 20, 20, 15, threads, true, true);

            } else {
                cout << "UR_GEN" << endl;
                uniform_random(src_ids, dst_ids, weights, vertices_count, edges_count, threads, true, true);
                cout << "Generated_UR" << endl;
            }
        } else {
            cout << test_flag << endl;
            cout << "file_init" << endl;
            input(test_file, false, src_ids, dst_ids, vertices_count, edges_count,weights);
            vertices_count++;
            cout << "vertices:" << vertices_count << endl;
            cout << "edges: " << edges_count << endl;
        }


        for (int i = 0; i < edges_count; i++) {
            cout << src_ids[i] << "----" << dst_ids[i] << endl;
        }

        cout << endl;
        CSR_GRAPH a(vertices_count, edges_count, src_ids, dst_ids, weights, true);
        a.save_to_graphviz_file("graph_pred", NULL);
        a.print_CSR_format();

        unsigned int *labels = new unsigned int[vertices_count];
        unsigned int *dest_labels = new unsigned int[edges_count];
        unsigned int *dev_labels;
        unsigned int *dev_dest_labels;


        if (gather_flag) {
            /*
            SAFE_CALL((cudaMalloc((void **) &dev_labels, (size_t) (sizeof(unsigned int)) * (vertices_count))));
            SAFE_CALL((cudaMalloc((void **) &dev_dest_labels, (size_t) (sizeof(unsigned int)) * edges_count)));
            a.move_to_device(dest_labels, labels, dev_dest_labels, dev_labels);

            SAFE_CALL(cudaEventRecord(start));
            dim3 block(1024, 1);
            dim3 grid(vertices_count * 32 / block.x, 1);
            //dim3 block(16,1);
            //dim3 grid(1,1);

            printf("starting...");
            SAFE_KERNEL_CALL((gather_warp_per_vertex << < grid, block >> >
                                                                (a.get_dev_v_array(), a.get_dev_e_array(), dev_dest_labels, dev_labels, edges_count, vertices_count)));
            printf("terminating....");
            SAFE_CALL(cudaEventRecord(stop));
            SAFE_CALL(cudaEventSynchronize(stop));
            float time;
            SAFE_CALL(cudaEventElapsedTime(&time, start, stop));
            time *= 1000000;
            a.move_to_host(dest_labels, labels, dev_dest_labels, dev_labels);
            SAFE_CALL(cudaFree(dev_labels));
            SAFE_CALL(cudaFree(dev_dest_labels));

            if (check_flag) {
                unsigned int *test_dest_labels = new unsigned int[edges_count];
                form_label_array(threads, vertices_count, edges_count, test_dest_labels, a.get_dev_v_array(), labels,
                                 a.get_e_array());
                int flag = check(edges_count, dest_labels, test_dest_labels);
                if (flag == 0) {
                    printf("CORRECT");
                }
                delete[] test_dest_labels;
            }


            printf("GATHER Bandwidth for 2^%d vertices and 2^%d edges is %f GB/s\n ", vertices_index,
                   vertices_index + (int) log2((double) density_degree),
                   sizeof(unsigned int) * (2 * vertices_count + 2 * edges_count) / (time));
                   */
        }
        //cout<<"2"<<endl;
        if (lp_flag) {
            //lp(vertices_count, a.get_e_array(), a.get_v_array(), labels);
            louvain(vertices_count, edges_count, a.get_e_array(), a.get_v_array(), labels,a.get_weights(),true);
            a.save_to_graphviz_file("graph_res", labels);
            label_stats(labels, vertices_count);
            delete[] labels;
        }
        delete[] src_ids;
        delete[] dst_ids;
        delete[] weights;

    }
    catch (const char *error) {
        cout << error << endl;
        getchar();
        return 1;
    }
    catch (...) {
        cout << "unknown error" << endl;
    }

    //SAFE_CALL(cudaProfilerStop());
    return 0;
}