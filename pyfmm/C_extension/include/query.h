/**
 * @file   query.h
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
 * 
 * 
*/

#pragma once


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
int dicho_find(const double *arr, int n, double target);
