/**
 * @file   query.c
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
*/

#include <stdlib.h>

#include "fmm/query.h"

size_t dicho_find(const real_t *arr, size_t n, real_t target){
    size_t left=0;
    size_t right=n-1;
    size_t mid=0;
    if(target <= arr[0]){}
    else if(target >= arr[right]){
        left = right;
    }
    else {
        while(left < right){
            mid = left + ((right-left) >> 1);
            if(arr[mid] == target){
                return mid;
            }
            else if(arr[mid] < target){
                left = mid + 1;
            }
            else{
                right = mid;
            }
        }
        left -= 1;
    }
    return left;
}