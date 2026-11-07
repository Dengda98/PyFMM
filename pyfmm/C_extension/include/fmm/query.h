/**
 * @file   query.h
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
 * 
 * 
*/

#pragma once

#include "fmm/const.h"

/**
 * 使用二分法查找元素，返回较小的一个
 * 
 * @param     arr       (in)数组，要求从小到大排列
 * @param     n         (in)数组
 * @param     target    (in)待查找元素
 * 
 * @return    索引值
 * 
 */
size_t dicho_find(const real_t *arr, size_t n, real_t target);
