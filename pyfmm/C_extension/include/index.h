/**
 * @file   index.h
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
*/

#pragma once


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
inline void unravel_index(int idx, int ntp, int np, int *ir, int *it, int *ip){
    *ir = idx / ntp;
    *it = (idx - (*ir)*ntp) / np;
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
inline void ravel_index(int *idx, int ntp, int np, int ir, int it, int ip){
    *idx = ir*ntp + it*np + ip;
}