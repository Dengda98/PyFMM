/**
 * @file   heapsort.h
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
 *       非标准版本的最小推排序，堆元素本身数据是某个数据节点的索引值，
 *       判断元素大小关系使用节点对应的走时进行比较
 * 
*/

#pragma once

#include "const.h"

typedef int HEAP_DATA;

/**
 * 最小堆向上调整
 * 
 * @param      HEAP_data     (inout)指向堆首的指针
 * @param      child         (in)某个堆元素在堆中的索引值
 * @param      NroIdx        (out)一维指针，用于在节点索引位置处填上堆中的索引值
 * @param      TT            (in)一维指针，记录每个节点的走时
 * 
 * 
 */
void MinHeap_AdjustUp(HEAP_DATA * HEAP_data, int child, int *NroIdx, const MYREAL *TT);



/**
 * 最小堆向下调整
 * 
 * @param      HEAP_data     (inout)指向堆首的指针
 * @param      size          (in)堆大小
 * @param      root          (in)某个堆元素在堆中的索引值
 * @param      NroIdx        (out)一维指针，用于在节点索引位置处填上堆中的索引值
 * @param      TT            (in)一维指针，记录每个节点的走时
 * 
 * 
 */
void MinHeap_AdjustDown(HEAP_DATA * HEAP_data, int size, int root, int *NroIdx, const MYREAL *TT);


/**
 * 从堆首弹出元素，即返回堆首元素并且从堆中删除堆首，并做一次向下调整堆
 * 
 * @param      HEAP_data     (inout)指向堆首的指针
 * @param      psize         (inout)堆大小，会被调整大小
 * @param      NroIdx        (out)一维指针，用于在节点索引位置处填上堆中的索引值
 * @param      TT            (in)一维指针，记录每个节点的走时
 * 
 * @return     堆首元素
 * 
 */
HEAP_DATA HeapPop(HEAP_DATA *HEAP_data, int *psize, int *NroIdx, const MYREAL *TT);


/**
 * 从堆尾压入元素，即添加元素，并做一次向上调整堆
 * 
 * @param      HEAP_data     (inout)指向堆首的指针
 * @param      psize         (inout)堆大小，会被调整大小
 * @param      pcap          (inout)堆最大容量，视情况会被调整大小
 * @param      newdata       (in)新元素
 * @param      NroIdx        (out)一维指针，用于在节点索引位置处填上堆中的索引值
 * @param      TT            (in)一维指针，记录每个节点的走时
 * 
 * @return     堆首指针
 * 
 */
HEAP_DATA * HeapPush(HEAP_DATA *HEAP_data, int *psize, int *pcap, HEAP_DATA newdata, int *NroIdx, const MYREAL *TT);


/**
 * 建立堆
 * 
 * @param      HEAP_data     (inout)指向堆首的指针
 * @param      size          (in)堆大小
 * @param      idx           (in)指定子堆的索引，完全建立堆则idx==size
 * @param      NroIdx        (out)一维指针，用于在节点索引位置处填上堆中的索引值
 * @param      TT            (in)一维指针，记录每个节点的走时
 * 
 * 
 */
void HeapBuild(HEAP_DATA * HEAP_data, int size, int idx, int *NroIdx, const MYREAL *TT);


/**
 * 交换数据（内联函数）
 * 
 * @param     data1     数据1的指针
 * @param     data2     数据2的指针
 */
inline void Swap(HEAP_DATA * data1, HEAP_DATA * data2){
    HEAP_DATA FMM = *data1;
    *data1 = *data2;
    *data2 = FMM;
}


/**
 * 打印推数据【用于debug】
 */
void print_HEAP(
    HEAP_DATA * data, int size, int nr, int nt, int np, int *NroIdx, 
    MYREAL *TT, MYREAL *gTr, MYREAL *gTt, MYREAL *gTp);


