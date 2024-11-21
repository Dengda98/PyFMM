/**
 * @file   fsm.c
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-05
 * 
*/

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "fsm.h"
#include "fmm.h"
#include "const.h"
#include "index.h"
#include "mallocfree.h"

static void _fsw_sort3(FSW_DATA *arr){
    FSW_DATA tmp;
    if(arr[0].t > arr[1].t){
        tmp = arr[0];
        arr[0] = arr[1];
        arr[1] = tmp;
    }
    if(arr[1].t > arr[2].t){
        tmp = arr[1];
        arr[1] = arr[2];
        arr[2] = tmp;
    }
    if(arr[0].t > arr[1].t){
        tmp = arr[0];
        arr[0] = arr[1];
        arr[1] = tmp;
    }
}


void FastSweeping(
    double *rs, int nr, 
    double *ts, int nt, 
    double *ps, int np,
    double rr,  double tt, double pp,
    MYREAL maxnghT, MYREAL *Slw,  MYREAL *TT,  bool sphcoord)
{
    double dr = rs[1] - rs[0];
    double dt = ts[1] - ts[0];
    double dp = ps[1] - ps[0];

    int begr, stepr, endr;
    int begt, stept, endt;
    int begp, stepp, endp;
    begr = begt = begp = 0;
    stepr = stept = stepp = 1;
    endr = nr;
    endt = nt;
    endp = np;
    bool direcr, direct, direcp;
    direcr = direct = direcp = false;

    int idx;
    int ntp=nt*np;
    int nrtp=nr*ntp;
    char *FMM_stat = (char *)malloc1d(nrtp, sizeof(char));
    for(int i=0; i<nrtp; ++i){
        TT[i] = 9.9e30f;
        FMM_stat[i] = FMM_FAR;
    }



    init_source_TT(
        rs, nr, ts, nt, ps, np, 
        rr, tt, pp, 
        Slw, TT, 
        NULL, sphcoord,
        NULL, NULL, NULL, NULL, NULL);    


    int isweep = 0;
    while(isweep < 8){
        direcr = !(direcr);
        if(isweep%2 == 0) direct = !(direct);
        if(isweep%4 == 0) direcp = !(direcp);

        if(direcr) {
            begr = 0; stepr = 1; endr = nr;
        } else {
            begr = nr-1; stepr = -1; endr = -1;
        }
        if(direct) {
            begt = 0; stept = 1; endt = nt;
        } else {
            begt = nt-1; stept = -1; endt = -1;
        }
        if(direcp) {
            begp = 0; stepp = 1; endp = np;
        } else {
            begp = np-1; stepp = -1; endp = -1;
        }

        FSW_DATA fswarr[3] = {
            {0, 0.0},
            {0, 0.0},
            {0, 0.0}
        }; 
        FSW_DATA fswtmp;
        // printf("%d, %i, %i, %i \n", isweep, direcr, direct, direcp);
        // Start Sweeping
        for(int ir=begr; ir!=endr; ir+=stepr){
        for(int it=begt; it!=endt; it+=stept){
        for(int ip=begp; ip!=endp; ip+=stepp){
            ravel_index(&idx, ntp, np, ir, it, ip);

            // find neighbours, get 3 possible directions
            if(nr>1){
                if(ir==0){
                    fswtmp.t = TT[idx+ntp]; 
                } else if(ir==nr-1)  {
                    fswtmp.t = TT[idx-ntp]; 
                } else {
                    fswtmp.t = (TT[idx+ntp] < TT[idx-ntp])? TT[idx+ntp] : TT[idx-ntp];
                }
            } else {
                fswtmp.t = 9.9e30;
            }
            fswtmp.h = dr;
            fswarr[0] = fswtmp;  
            
            if(nt>1){
                if(it==0){
                    fswtmp.t = TT[idx+np]; 
                } else if(it==nt-1)  {
                    fswtmp.t = TT[idx-np]; 
                } else {
                    fswtmp.t = (TT[idx+np] < TT[idx-np])? TT[idx+np] : TT[idx-np];
                }
            } else {
                fswtmp.t = 9.9e30;
            }
            fswtmp.h = dt;
            fswarr[1] = fswtmp;  
            
            if(np>1){
                if(ip==0){
                    fswtmp.t = TT[idx+1]; 
                } else if(ip==np-1)  {
                    fswtmp.t = TT[idx-1]; 
                } else {
                    fswtmp.t = (TT[idx+1] < TT[idx-1])? TT[idx+1] : TT[idx-1];
                }
            } else {
                fswtmp.t = 9.9e30;
            }
            fswtmp.h = dp;
            fswarr[2] = fswtmp;  
            
            // sort tarr, from small to large 
            // NOTICE!! sort by Traveltime or sort by h*s ?
            _fsw_sort3(fswarr);
            
            

            // skip this point
            fswtmp = fswarr[0];
            if(fswtmp.t > maxnghT)  continue;

            MYREAL s = Slw[idx];

            // solve a quaratic equation directly
            // MYREAL t_cand;
            // double Acoef=0.0, Bcoef=0.0, Ccoef=0.0, jdg;
            // double hinv2;
            // MYREAL t1;
            // Ccoef = - s*s;
            // for(char i=0; i<3; ++i){
            //     fswtmp = fswarr[i];
            //     t1 = fswtmp.t;
            //     if(t1 >= maxnghT) continue;
            //     hinv2  = 1.0/fswtmp.h;
            //     hinv2 = hinv2*hinv2;
            //     Acoef += hinv2;
            //     Bcoef += 2*t1*hinv2;
            //     Ccoef += t1*t1*hinv2;
            // }
            // jdg = Bcoef*Bcoef - 4.0*Acoef*Ccoef;
            // if(jdg >= 0.0){
            //     jdg = sqrt(jdg);
            //     t_cand = (Bcoef + jdg)/(2*Acoef);
            // } else {
            //     t_cand = fswarr[0].t + fswarr[0].h * s;
            // }
        
            // solve a quaratic equation directly one by one
            MYREAL t_bak = fswtmp.t + fswtmp.h * s;
            MYREAL t_cand = t_bak;
            if(t_cand > fswarr[1].t){
                double Acoef=0.0, Bcoef=0.0, Ccoef=0.0, jdg;
                double hinv2;
                MYREAL t1;
                Ccoef = - s*s;
                for(char i=0; i<3; ++i){
                    fswtmp = fswarr[i];
                    t1 = fswtmp.t;
                    hinv2  = 1.0/fswtmp.h;
                    hinv2 = hinv2*hinv2;
                    Acoef += hinv2;
                    Bcoef += 2*t1*hinv2;
                    Ccoef += t1*t1*hinv2;
                    if(i==0) continue;

                    jdg = Bcoef*Bcoef - 4.0*Acoef*Ccoef;
                    if(jdg < 0.0) break;

                    jdg = sqrt(jdg);
                    t_cand = (Bcoef + jdg)/(2*Acoef);

                    // break in advance
                    if(i<2 && t_cand < fswarr[i+1].t) break;
                }
            }

            if(t_cand < TT[idx]) TT[idx] = t_cand;
            
        }}}

        isweep++;
        
    }

    free(FMM_stat);

}