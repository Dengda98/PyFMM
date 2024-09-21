/**
 * @file   diff.c
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
*/


#include <stdio.h>
#include <stdlib.h>

#include "diff.h"



void get_diff_odr1(const float *pt, double h, double *acoef, double *bcoef, double *diff){
    float a = 1/h;
    float b = pt[1]/h;
    if(acoef!=NULL) *acoef = a;
    if(bcoef!=NULL) *bcoef = b;
    if(diff!=NULL)  *diff = a*pt[0] - b;
}


void get_diff_odr2(const float *pt, double h, double *acoef, double *bcoef, double *diff){
    float a =  3/(2*h);
    float b =  (4*pt[1] - pt[2])/(2*h);
    if(acoef!=NULL) *acoef = a;
    if(bcoef!=NULL) *bcoef = b;
    if(diff!=NULL)  *diff = a*pt[0] - b;
}


void get_diff_odr3(const float *pt, double h, double *acoef, double *bcoef, double *diff){
    float a =  11/(6*h);
    float b =  (18*pt[1] - 9*pt[2] + 2*pt[3])/(6*h);
    if(acoef!=NULL) *acoef = a;
    if(bcoef!=NULL) *bcoef = b;
    if(diff!=NULL)  *diff = a*pt[0] - b;
}


void get_diff_odr123(int odr, const float *pt, double h, double *acoef, double *bcoef, double *diff){
    if(odr==0){
        if(acoef!=NULL) *acoef = 0.0;
        if(bcoef!=NULL) *bcoef = 0.0;
        if(diff!=NULL)  *diff = 0.0;
    }
    else if(odr==1){
        get_diff_odr1(pt, h, acoef, bcoef, diff);
    } else if(odr==2){
        get_diff_odr2(pt, h, acoef, bcoef, diff);
    } else if(odr==3){
        get_diff_odr3(pt, h, acoef, bcoef, diff);
    } else {
        fprintf(stderr, "WRONG DIFFERENCE ORDER (%d)\n", odr);
        exit(EXIT_FAILURE);
    }
}
