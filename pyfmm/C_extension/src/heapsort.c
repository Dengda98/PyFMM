/**
 * @file   heapsort.c
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
 *      
 * 
*/

#include <stdio.h>
#include <stdlib.h>

#include "const.h"
#include "heapsort.h"
#include "index.h"


void MinHeap_AdjustUp(HEAP_DATA * HEAP_data, size_t child, size_t *NroIdx, const real_t *TT){
    size_t parent;
    HEAP_DATA *pdata1, *pdata2;
    while(child > 0){
        parent = (child-1)/2;
        pdata1 = HEAP_data+child;
        pdata2 = HEAP_data+parent;
        if(TT[*pdata1] >= TT[*pdata2]) break;

        if(NroIdx!=NULL){
            NroIdx[*pdata1] = parent;
            NroIdx[*pdata2] = child;
        }
        Swap(pdata1, pdata2);
        child = parent;
    }
}

void MinHeap_AdjustDown(HEAP_DATA * HEAP_data, size_t size, size_t root, size_t *NroIdx, const real_t *TT){
    size_t parent = root;
    size_t child = parent*2 + 1;
    HEAP_DATA *pdata1, *pdata2;
    while(child < size){
        pdata1 = HEAP_data+child;
        pdata2 = HEAP_data+parent;
        if(child+1 < size && TT[*(pdata1+1)] < TT[*pdata1]){
            child++;
            pdata1++;
        }

        if(TT[*pdata1] >= TT[*pdata2]) break;

        if(NroIdx!=NULL){
            NroIdx[*pdata1] = parent;
            NroIdx[*pdata2] = child;
        }
        Swap(pdata1, pdata2);
        parent = child;
        child = parent*2 + 1;
    }
}

HEAP_DATA HeapPop(HEAP_DATA *HEAP_data, size_t *psize, size_t *NroIdx, const real_t *TT){
    HEAP_DATA popdata = HEAP_data[0];
    if(NroIdx!=NULL){
        NroIdx[*(HEAP_data+(*psize-1))] = 0;
    }
    Swap(HEAP_data, HEAP_data+(*psize-1));
    (*psize)--;

    MinHeap_AdjustDown(HEAP_data, *psize, 0, NroIdx, TT);

    return popdata;
}

HEAP_DATA * HeapPush(HEAP_DATA *HEAP_data, size_t *psize, size_t *pcap, HEAP_DATA newdata, size_t *NroIdx, const real_t *TT){
    if(*psize == *pcap){
        size_t newcap = (*pcap==0)? 8 : (*pcap)*2;
        HEAP_DATA *HEAP_data0 = realloc(HEAP_data, sizeof(HEAP_DATA)*newcap);
        if(HEAP_data==NULL){
            fprintf(stderr, "reallocation failed in fmm. exit.");
            exit(EXIT_FAILURE);
        }
        HEAP_data = HEAP_data0;
        *pcap = newcap;
    }
    HEAP_data[*psize] = newdata;
    (*psize)++;

    if(NroIdx!=NULL) NroIdx[newdata] = *psize-1;

    MinHeap_AdjustUp(HEAP_data, *psize-1, NroIdx, TT);

    return HEAP_data;
}

void HeapBuild(HEAP_DATA * HEAP_data, size_t size, size_t idx, size_t *NroIdx, const real_t *TT){
    // 倒序遍历父节点，避免无符号索引下溢
    for(size_t i=idx/2 + idx%2; i>0; --i){
        MinHeap_AdjustDown(HEAP_data, size, i-1, NroIdx, TT);
    }
}


/**
 * 仅用于debug
 */
void print_HEAP(
    HEAP_DATA * data, size_t size, size_t nr, size_t nt, size_t np, size_t *NroIdx,
    real_t *TT, real_t *gTr, real_t *gTt, real_t *gTp)
{
    size_t ir, it, ip;
    
    printf("size %zu\n", size);
    for(size_t i=0; i<size; ++i){
        unravel_index(data[i], nt*np, np, &ir, &it, &ip);
        printf("ir=%2zu, it=%2zu, ip=%2zu, t=%f",
         ir, it, ip, 
        TT[data[i]]);
        // printf("ir=%2zu, it=%2zu, ip=%2zu, t=%f, tr=%+f, tt=%+f, tp=%+f, |gradT|=%f  ",
        //  ir, it, ip, 
        // TT[data[i]], 
        // gTr[data[i]], gTt[data[i]], gTp[data[i]], 
        // sqrt(pow(gTr[data[i]],2) + pow(gTt[data[i]],2) + pow(gTp[data[i]],2)));
        if(NroIdx!=NULL){
            printf(" %zu", NroIdx[data[i]]);
        }
        printf("\n");

        if(i>20) break;
    }
    // getchar();
}



