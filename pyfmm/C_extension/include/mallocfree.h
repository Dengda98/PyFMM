/**
 * @file   mallocfree.h
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
 */


#pragma once

#include <stddef.h>


/**
 * 申请三维指针内存空间
 * 
 * @param     n1      (in)第一维尺寸
 * @param     n2      (in)第二维尺寸
 * @param     n3      (in)第三维尺寸
 * @param     size    (in)每个元素字节数
 * 
 * @return    三维指针
 * 
 */
void *** malloc3d(int n1, int n2, int n3, size_t size);


/**
 * 申请二维指针内存空间
 * 
 * @param     n1      (in)第一维尺寸
 * @param     n2      (in)第二维尺寸
 * @param     size    (in)每个元素字节数
 * 
 * @return    二维指针
 * 
 */
void ** malloc2d(int n1, int n2, size_t size);



/**
 * 申请一维指针内存空间
 * 
 * @param     n       (in)第一维尺寸
 * @param     size    (in)每个元素字节数
 * 
 * @return    一维指针
 * 
 */
void * malloc1d(int n, size_t size);


/**
 * 释放三维指针内存空间
 * 
 * @param     arr     (in)三维指针
 * @param     n1      (in)第一维尺寸
 * @param     n2      (in)第二维尺寸
 * 
 */
void free3d(void ***arr, int n1, int n2);


/**
 * 释放二维指针内存空间
 * 
 * @param     arr     (in)二维指针
 * @param     n1      (in)第一维尺寸
 * 
 */
void free2d(void **arr, int n1);


/**
 * 释放一维指针内存空间
 * 
 * @param     arr     (in)一维指针
 * 
 */
void free1d(void *arr);



