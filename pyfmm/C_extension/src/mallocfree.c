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

void *** malloc3d(MYINT n1, MYINT n2, MYINT n3, size_t size){
    void ***pt;
    if((pt = (void ***)malloc(n1*sizeof(void**))) == NULL){
        fprintf(stderr, "malloc3d out of memory\n");
        exit(EXIT_FAILURE);
    };
    for(MYINT i=0; i<n1; ++i){
        if((pt[i] = (void **)malloc(n2*sizeof(void*))) == NULL){
            fprintf(stderr, "malloc3d out of memory\n");
            exit(EXIT_FAILURE);
        }
        for(MYINT j=0; j<n2; ++j){
            if((pt[i][j] = malloc(n3*size)) == NULL){
                fprintf(stderr, "malloc3d out of memory\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return pt;
}

void ** malloc2d(MYINT n1, MYINT n2, size_t size){
    void **pt;
    if((pt = (void **)malloc(n1*sizeof(void*))) == NULL){
        fprintf(stderr, "malloc2d out of memory\n");
        exit(EXIT_FAILURE);
    };
    for(MYINT i=0; i<n1; ++i){
        if((pt[i] = malloc(n2*size)) == NULL){
            fprintf(stderr, "malloc2d out of memory\n");
            exit(EXIT_FAILURE);
        }
    }
    return pt;
}

void * malloc1d(MYINT n, size_t size){
    void *pt;
    if((pt = malloc(n*size)) == NULL){
        fprintf(stderr, "malloc1d out of memory\n");
        exit(EXIT_FAILURE);
    }
    return pt;
}

void free3d(void ***arr, MYINT n1, MYINT n2){
    for(MYINT i1=0; i1<n1; ++i1){
        for(MYINT i2=0; i2<n2; ++i2){
            free(arr[i1][i2]);
        }
        free(arr[i1]);
    }
    free(arr);
}

void free2d(void **arr, MYINT n1){
    for(MYINT i1=0; i1<n1; ++i1){
        free(arr[i1]);
    }
    free(arr);
}
