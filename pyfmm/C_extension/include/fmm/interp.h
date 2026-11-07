/**
 * @file   interp.h
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
*/

#pragma once

#include "fmm/const.h"


/**
 * 将三维数据展平进行三次线性插值
 * 
 * @param     x           (in)x方向坐标数组
 * @param     nx          (in)x长度
 * @param     y           (in)y方向坐标
 * @param     ny          (in)y长度
 * @param     z           (in)z方向坐标
 * @param     nz          (in)z长度
 * @param     nyz         (in)ny*nz
 * @param     values      (in)展平的三维数据数组
 * @param     xi          (in)待插值的x坐标
 * @param     yi          (in)待插值的y坐标
 * @param     zi          (in)待插值的z坐标
 * @param     extrapolate (in)是否允许外推；为false时将坐标限制在网格范围内，单节点维度保持常值
 * @param     pdiffx      (out)非NULL时，插值x方向梯度
 * @param     pdiffy      (out)非NULL时，插值y方向梯度
 * @param     pdiffz      (out)非NULL时，插值z方向梯度
 * @param     IXYZ        (out)非NULL时返回索引坐标(i,i+1,j,j+1,k,k+1)
 * @param     WGHT        (out)非NULL时，8个插值权重
 * 
 * @return    插值结果
 * 
 */
real_t trilinear_one_ravel(
    const real_t *x, size_t nx, const real_t *y, size_t ny, const real_t *z, size_t nz, size_t nyz, const real_t *values,
    real_t xi, real_t yi, real_t zi, bool extrapolate, real_t *pdiffx, real_t *pdiffy, real_t *pdiffz,
    size_t IXYZ[6], real_t WGHT[2][2][2]);


/**
 * 计算三次线性插值的索引和权重
 * 
 * @param     x           (in)x方向坐标数组
 * @param     nx          (in)x长度
 * @param     y           (in)y方向坐标
 * @param     ny          (in)y长度
 * @param     z           (in)z方向坐标
 * @param     nz          (in)z长度
 * @param     xi          (in)待插值的x坐标
 * @param     yi          (in)待插值的y坐标
 * @param     zi          (in)待插值的z坐标
 * @param     extrapolate (in)是否允许外推；为false时将坐标限制在网格范围内，单节点维度保持常值
 * @param     IXYZ        (out)非NULL时返回索引坐标(i,i+1,j,j+1,k,k+1)
 * @param     WGHT        (out)非NULL时，8个插值权重
 */
void trilinear_one_fac(
    const real_t *x, size_t nx, const real_t *y, size_t ny, const real_t *z, size_t nz,
    real_t xi, real_t yi, real_t zi, bool extrapolate, size_t IXYZ[6], real_t WGHT[2][2][2]);



/**
 * 在已知索引和权重的情况下做三次线性插值
 * 
 * @param     IXYZ        (in)(xi,yi,zi)所在的索引坐标(i,i+1,j,j+1,k,k+1)
 * @param     WGHT        (in)8个插值权重
 * @param     values      (in)展平的三维数据数组
 * @param     nx          (in)x长度
 * @param     ny          (in)y长度
 * @param     nz          (in)z长度
 * @param     nyz         (in)ny*nz
 * @param     pdiffx      (out)非NULL时，插值x方向梯度
 * @param     pdiffy      (out)非NULL时，插值y方向梯度
 * @param     pdiffz      (out)非NULL时，插值z方向梯度
 * 
 * @return    插值结果
 * 
 */
real_t trilinear_one_Idx_ravel(
    const size_t IXYZ[6], const real_t WGHT[2][2][2],  const real_t *values, size_t nx, size_t ny, size_t nz, size_t nyz,
    real_t *pdiffx, real_t *pdiffy, real_t *pdiffz);