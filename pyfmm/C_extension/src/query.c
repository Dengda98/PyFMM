/**
 * @file   query.c
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
*/

#include <stdlib.h>

#include "query.h"

MYINT dicho_find(const double *arr, MYINT n, double target){
    MYINT left=0;
    MYINT right=n-1;
    MYINT mid=0;
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