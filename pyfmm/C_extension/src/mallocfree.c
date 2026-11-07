/**
 * @file   mallocfree.c
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
 */


#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#include "mallocfree.h"

void *** malloc3d(size_t n1, size_t n2, size_t n3, size_t size){
    void ***pt;
    if((pt = (void ***)malloc(n1*sizeof(void**))) == NULL){
        fprintf(stderr, "malloc3d out of memory\n");
        exit(EXIT_FAILURE);
    };
    for(size_t i=0; i<n1; ++i){
        if((pt[i] = (void **)malloc(n2*sizeof(void*))) == NULL){
            fprintf(stderr, "malloc3d out of memory\n");
            exit(EXIT_FAILURE);
        }
        for(size_t j=0; j<n2; ++j){
            if((pt[i][j] = malloc(n3*size)) == NULL){
                fprintf(stderr, "malloc3d out of memory\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return pt;
}

void ** malloc2d(size_t n1, size_t n2, size_t size){
    void **pt;
    if((pt = (void **)malloc(n1*sizeof(void*))) == NULL){
        fprintf(stderr, "malloc2d out of memory\n");
        exit(EXIT_FAILURE);
    };
    for(size_t i=0; i<n1; ++i){
        if((pt[i] = malloc(n2*size)) == NULL){
            fprintf(stderr, "malloc2d out of memory\n");
            exit(EXIT_FAILURE);
        }
    }
    return pt;
}

void * malloc1d(size_t n, size_t size){
    void *pt;
    if((pt = malloc(n*size)) == NULL){
        fprintf(stderr, "malloc1d out of memory\n");
        exit(EXIT_FAILURE);
    }
    return pt;
}

void free3d(void ***arr, size_t n1, size_t n2){
    for(size_t i1=0; i1<n1; ++i1){
        for(size_t i2=0; i2<n2; ++i2){
            free(arr[i1][i2]);
        }
        free(arr[i1]);
    }
    free(arr);
}

void free2d(void **arr, size_t n1){
    for(size_t i1=0; i1<n1; ++i1){
        free(arr[i1]);
    }
    free(arr);
}
