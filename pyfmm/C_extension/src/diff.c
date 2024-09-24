/**
 * @file   diff.c
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
*/


#include <stdio.h>
#include <stdlib.h>

#include "const.h"
#include "diff.h"



void get_diff_odr1(const MYREAL *pt, double h, double *acoef, double *bcoef, double *diff){
    MYREAL a = 1.0/h;
    MYREAL b = pt[1]/h;
    if(acoef!=NULL) *acoef = a;
    if(bcoef!=NULL) *bcoef = b;
    if(diff!=NULL)  *diff = a*pt[0] - b;
}


void get_diff_odr2(const MYREAL *pt, double h, double *acoef, double *bcoef, double *diff){
    MYREAL a =  3.0/(2.0*h);
    MYREAL b =  (4.0*pt[1] - pt[2])/(2.0*h);
    if(acoef!=NULL) *acoef = a;
    if(bcoef!=NULL) *bcoef = b;
    if(diff!=NULL)  *diff = a*pt[0] - b;
}


void get_diff_odr3(const MYREAL *pt, double h, double *acoef, double *bcoef, double *diff){
    MYREAL a =  11.0/(6.0*h);
    MYREAL b =  (18.0*pt[1] - 9.0*pt[2] + 2.0*pt[3])/(6.0*h);
    if(acoef!=NULL) *acoef = a;
    if(bcoef!=NULL) *bcoef = b;
    if(diff!=NULL)  *diff = a*pt[0] - b;
}


void get_diff_odr123(int odr, const MYREAL *pt, double h, double *acoef, double *bcoef, double *diff){
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
