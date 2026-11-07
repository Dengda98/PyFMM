/**
 * @file   progressbar.c
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2024-07
 * 
 * 以下代码实现进度条的输出
 * 
 */

#include <stdio.h>

#include "progressbar.h"


void printprogressBar(const char *prefix, size_t percentage) {
    printf("\r\033[K"); // 移动到行首并清空行
    if(prefix!=NULL) printf("%s", prefix);
    printf("[");
    size_t pos = _PROGRESSBAR_WIDTH_ * percentage / 100;
    for (size_t i = 0; i < _PROGRESSBAR_WIDTH_; ++i) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    printf("] %zu %%", percentage);
    if(percentage==100){
        printf("\n");
    }
    fflush(stdout);
}