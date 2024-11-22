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


void MinHeap_AdjustUp(HEAP_DATA * HEAP_data, int child, int *NroIdx, const MYREAL *TT){
    int parent = (child-1)/2; 
    HEAP_DATA *pdata1, *pdata2;
    while(child > 0){
        pdata1 = HEAP_data+child;
        pdata2 = HEAP_data+parent;
        if(TT[*pdata1] >= TT[*pdata2]) break;

        if(NroIdx!=NULL){
            NroIdx[*pdata1] = parent;
            NroIdx[*pdata2] = child;
        }
        Swap(pdata1, pdata2);
        child = parent;
        parent = (child-1)/2;
    }
}

void MinHeap_AdjustDown(HEAP_DATA * HEAP_data, int size, int root, int *NroIdx, const MYREAL *TT){
    int parent = root;
    int child = parent*2 + 1;
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

HEAP_DATA HeapPop(HEAP_DATA *HEAP_data, int *psize, int *NroIdx, const MYREAL *TT){
    HEAP_DATA popdata = HEAP_data[0];
    if(NroIdx!=NULL){
        NroIdx[*(HEAP_data+(*psize-1))] = 0;
    }
    Swap(HEAP_data, HEAP_data+(*psize-1));
    (*psize)--;

    MinHeap_AdjustDown(HEAP_data, *psize, 0, NroIdx, TT);

    return popdata;
}

HEAP_DATA * HeapPush(HEAP_DATA *HEAP_data, int *psize, int *pcap, HEAP_DATA newdata, int *NroIdx, const MYREAL *TT){
    if(*psize == *pcap){
        int newcap = (*pcap==0)? 8 : (*pcap)*2;
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

void HeapBuild(HEAP_DATA * HEAP_data, int size, int idx, int *NroIdx, const MYREAL *TT){
    for(int i=(idx-1)/2; i>=0; --i){
        MinHeap_AdjustDown(HEAP_data, size, i, NroIdx, TT);
    }
}


/**
 * 仅用于debug
 */
void print_HEAP(
    HEAP_DATA * data, int size, int nr, int nt, int np, int *NroIdx, 
    MYREAL *TT, MYREAL *gTr, MYREAL *gTt, MYREAL *gTp)
{
    int ir, it, ip;
    
    printf("size %d\n", size);
    for(int i=0; i<size; ++i){
        unravel_index(data[i], nt*np, np, &ir, &it, &ip);
        printf("ir=%2d, it=%2d, ip=%2d, t=%f", 
         ir, it, ip, 
        TT[data[i]]);
        // printf("ir=%2d, it=%2d, ip=%2d, t=%f, tr=%+f, tt=%+f, tp=%+f, |gradT|=%f  ", 
        //  ir, it, ip, 
        // TT[data[i]], 
        // gTr[data[i]], gTt[data[i]], gTp[data[i]], 
        // sqrt(pow(gTr[data[i]],2) + pow(gTt[data[i]],2) + pow(gTp[data[i]],2)));
        if(NroIdx!=NULL){
            printf(" %d", NroIdx[data[i]]);
        }
        printf("\n");

        if(i>20) break;
    }
    // getchar();
}



