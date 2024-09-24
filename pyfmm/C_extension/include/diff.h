/**
 * @file   diff.h
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
*/

#pragma once

#include "const.h"


/**
 * 一阶差分,  \f$ \frac{T-T_{i-1}}{h} \f$ , 形成 \f$ aT-b \f$ 的形式 
 * 
 * @param    pt      (in)数组
 * @param    h       (in)差分间隔
 * @param    acoef   (out)系数结果a
 * @param    bcoef   (out)系数结果b
 * @param    diff    (out) \f$ aT-b \f$ 值
 */
void get_diff_odr1(const MYREAL *pt, double h, double *acoef, double *bcoef, double *diff);


/**
 * 二阶差分, \f$ \frac{3T - 4T_{i-1} + T_{i-2}}{2h} \f$ , 形成 \f$ aT-b \f$ 的形式 
 * 
 * @param    pt      (in)数组
 * @param    h       (in)差分间隔
 * @param    acoef   (out)系数结果a
 * @param    bcoef   (out)系数结果b
 * @param    diff    (out) \f$ aT-b \f$ 值
 */
void get_diff_odr2(const MYREAL *pt, double h, double *acoef, double *bcoef, double *diff);


/**
 * 三阶差分, \f$ \frac{11T - 18T_{i-1} + 9T_{i-2} - 2T_{i-3}}{6h} \f$ , 形成 \f$ aT-b \f$ 的形式 
 * 
 * @param    pt      (in)数组
 * @param    h       (in)差分间隔
 * @param    acoef   (out)系数结果a
 * @param    bcoef   (out)系数结果b
 * @param    diff    (out) \f$ aT-b \f$ 值
 */
void get_diff_odr3(const MYREAL *pt, double h, double *acoef, double *bcoef, double *diff);


/**
 * 计算一 or 二 or 三阶差分 , 形成 \f$ aT-b \f$ 的形式 
 * 
 * @param    odr     (in)阶数，1or2or3
 * @param    pt      (in)数组
 * @param    h       (in)差分间隔
 * @param    acoef   (out)系数结果a
 * @param    bcoef   (out)系数结果b
 * @param    diff    (out) \f$ aT-b \f$ 值
 */
void get_diff_odr123(int odr, const MYREAL *pt, double h, double *acoef, double *bcoef, double *diff);