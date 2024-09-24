/**
 * @file   interp.h
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
*/

#pragma once

#include "const.h"


/**
 * 将三维数据展平进行三次线性插值
 * 
 * @param     x       (in)x方向坐标数组
 * @param     nx      (in)x长度
 * @param     y       (in)y方向坐标
 * @param     ny      (in)y长度
 * @param     z       (in)z方向坐标
 * @param     nz      (in)z长度
 * @param     nyz     (in)ny*nz
 * @param     values  (in)展平的三维数据数组
 * @param     xi      (in)待插值的x坐标
 * @param     yi      (in)待插值的y坐标
 * @param     zi      (in)待插值的z坐标
 * @param     pdiffx  (out)非NULL时，插值x方向梯度
 * @param     pdiffy  (out)非NULL时，插值y方向梯度
 * @param     pdiffz  (out)非NULL时，插值z方向梯度
 * @param     IXYZ    (out)非NULL时，(xi,yi,zi)所在的索引坐标(i,i+1,j,j+1,k,k+1)
 * @param     WGHT    (out)非NULL时，8个插值权重
 * 
 * @return    插值结果
 * 
 */
MYREAL trilinear_one_ravel(
    const double *x, int nx, const double *y, int ny, const double *z, int nz, int nyz, const MYREAL *values, 
    double xi, double yi, double zi, double *pdiffx, double *pdiffy, double *pdiffz, 
    int IXYZ[6], double WGHT[2][2][2]);


/**
 * 计算三次线性插值的索引和权重
 * 
 * @param     x       (in)x方向坐标数组
 * @param     nx      (in)x长度
 * @param     y       (in)y方向坐标
 * @param     ny      (in)y长度
 * @param     z       (in)z方向坐标
 * @param     nz      (in)z长度
 * @param     xi      (in)待插值的x坐标
 * @param     yi      (in)待插值的y坐标
 * @param     zi      (in)待插值的z坐标
 * @param     IXYZ    (out)非NULL时，(xi,yi,zi)所在的索引坐标(i,i+1,j,j+1,k,k+1)
 * @param     WGHT    (out)非NULL时，8个插值权重
 */
void trilinear_one_fac(
    const double *x, int nx, const double *y, int ny, const double *z, int nz, 
    double xi, double yi, double zi, int IXYZ[6], double WGHT[2][2][2]);



/**
 * 在已知索引和权重的情况下做三次线性插值
 * 
 * @param     IXYZ    (in)(xi,yi,zi)所在的索引坐标(i,i+1,j,j+1,k,k+1)
 * @param     WGHT    (in)8个插值权重
 * @param     values  (in)展平的三维数据数组
 * @param     nx      (in)x长度
 * @param     ny      (in)y长度
 * @param     nz      (in)z长度
 * @param     nyz     (in)ny*nz
 * @param     pdiffx  (out)非NULL时，插值x方向梯度
 * @param     pdiffy  (out)非NULL时，插值y方向梯度
 * @param     pdiffz  (out)非NULL时，插值z方向梯度
 * 
 * @return    插值结果
 * 
 */
MYREAL trilinear_one_Idx_ravel(
    const int IXYZ[6], const double WGHT[2][2][2],  const MYREAL *values, int nx, int ny, int nz, int nyz, 
    double *pdiffx, double *pdiffy, double *pdiffz);