#ifndef RMAT_CSR_GRAPH_H
#define RMAT_CSR_GRAPH_H
#include "/usr/local/cuda-10.1/include/cuda_runtime.h"

class CSR_GRAPH {
    bool weighted;
    unsigned int *v_array;
    unsigned int *e_array;
    float *weigths;
    int vertices_count;
    int edges_count;
    unsigned int *labels;
    unsigned int *dest_labels;

    unsigned int *dev_v_array;
    unsigned int *dev_e_array;
    unsigned int *dev_labels;
    unsigned int *dev_dest_labels;
    float * dev_weigths;

public:

    unsigned int * get_dev_v_array(){ return dev_v_array; }
    unsigned int * get_dev_e_array(){return dev_e_array;}
    unsigned int * get_dev_labels(){ return dev_labels;}
    unsigned int *get_dev_dest_labels(){return dev_dest_labels;}
    float * get_dev_weigths(){return dev_weigths;}

    explicit CSR_GRAPH(int v, int e, int *_src_ids, int *_dst_ids, float *_weigths,bool weighted);
    void print_CSR_format();
    ~CSR_GRAPH();
    void print_adj_format();
    void adj_distribution(int _edges);
    void generate_labels(int _omp_threads);
    void form_label_array(int _omp_threads);
    void print_label_info(int _omp_threads);
    void move_to_device();
    void move_to_host();
};


#endif //RMAT_CSR_GRAPH_H
