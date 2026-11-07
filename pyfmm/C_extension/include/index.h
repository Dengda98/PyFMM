/**
 * @file   index.h
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
*/

#pragma once

#include <stddef.h>
#include <stdbool.h>

// 网格尺寸及有效索引统一使用 size_t

/**
 * 根据相邻偏移取得有效索引，在执行减法前检查边界
 *
 * @param      index     (in)当前索引
 * @param      n         (in)维度尺寸
 * @param      offset    (in)相邻偏移，仅取 -1、0、1
 * @param      shifted   (out)有效的相邻索引
 *
 * @return     相邻索引是否位于维度范围内
 */
inline bool neighbour_index(size_t index, size_t n, int offset, size_t *shifted){
    if((offset<0 && index==0) || (offset>0 && index==n-1)) return false;
    *shifted = offset<0 ? index-1 : index+(size_t)offset;
    return true;
}


/**
 * 将一维索引恢复成三维索引(内联函数)
 * 
 * @param      idx      (in)展开后的索引
 * @param      ntp      (in)2、3维度尺寸乘积
 * @param      np       (in)第3维度尺寸
 * @param      ir       (out)第1维索引
 * @param      it       (out)第2维索引
 * @param      ip       (out)第3维索引
 * 
 */
inline void unravel_index(size_t idx, size_t ntp, size_t np, size_t *ir, size_t *it, size_t *ip){
    *ir = idx / ntp;
    *it = (idx % ntp) / np;
    *ip = idx % np;
}


/**
 * 将三维索引展开成一维索引(内联函数)
 * 
 * @param      idx      (out)展开后的索引
 * @param      ntp      (in)2、3维度尺寸乘积
 * @param      np       (in)第3维度尺寸
 * @param      ir       (in)第1维索引
 * @param      it       (in)第2维索引
 * @param      ip       (in)第3维索引
 * 
 */
inline void ravel_index(size_t *idx, size_t ntp, size_t np, size_t ir, size_t it, size_t ip){
    *idx = ir*ntp + it*np + ip;
}
