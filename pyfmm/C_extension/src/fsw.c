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
    int maxodr, MYREAL maxnghT, MYREAL *Slw,  MYREAL *TT,  bool sphcoord)
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

    // convenient arrays
    double sin_ts[nt];
    if(sphcoord){
        for(int it=0; it<nt; ++it){
            sin_ts[it] = sin(ts[it]);
            if(fabs(sin_ts[it]) < 1e-12) sin_ts[it] += 1e-12;
        }
    }

    // DON'T CHANGE.
    static const char xr[6] = {-1, 1,  0, 0,  0, 0};
    static const char xt[6] = { 0, 0, -1, 1,  0, 0};
    static const char xp[6] = { 0, 0,  0, 0, -1, 1};
    const double hrtp[6] = {dr, dr, dt, dt, dp, dp};


    init_source_TT(
        rs, nr, ts, nt, ps, np, 
        rr, tt, pp, 
        Slw, TT, 
        FMM_stat, sphcoord,
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

        MYREAL mintravt=-999.0;
        double mintravt_h=0.0;
        MYREAL slw;

        // Start Sweeping
        for(int ir=begr; ir!=endr; ir+=stepr){
        for(int it=begt; it!=endt; it+=stept){
        for(int ip=begp; ip!=endp; ip+=stepp){
            ravel_index(&idx, ntp, np, ir, it, ip);
            slw = Slw[idx];

            // find minimum traveltime in 6 neighbours
            MYREAL t_bak=-999.9, t_bak0=-999.9;
            mintravt=-999.0;
            mintravt_h=0.0;
            for(int k=0; k<6; ++k){
                int jdx, iir, iit, iip;
                iir = ir+xr[k];
                iit = it+xt[k];
                iip = ip+xp[k];

                if(iir<0 || iir>nr-1) continue;
                if(iit<0 || iit>nt-1) continue;
                if(iip<0 || iip>np-1) continue;

                ravel_index(&jdx, ntp, np, iir, iit, iip);

                if(FMM_stat[jdx]==FMM_FAR) continue; 

                FMM_stat[jdx] = FMM_ALV;

                if(mintravt > TT[jdx] || mintravt < 0) {
                    mintravt = TT[jdx];
                    mintravt_h = hrtp[k];
                    // modify interval for spherical coordinate
                    if(sphcoord && k>2){
                        if(k<4) mintravt_h *= rs[ir];
                        else if(k<6) mintravt_h *= rs[ir]*sin_ts[it];
                    }

                    t_bak0 = mintravt + mintravt_h * slw;
                    if(t_bak > t_bak0 || t_bak < 0){
                        t_bak = t_bak0;
                    }
                }
            }
            if(mintravt < 0) continue;
        
            if(mintravt > maxnghT)  continue;

            MYREAL travt;
            char travt_stat;

            // temporary set 
            t_bak0 = TT[idx];
            if(t_bak0 > t_bak) TT[idx] = t_bak;

            if(sphcoord){
                travt = get_neighbour_travt(
                    nr, nt, np, ntp,
                    ir, it, ip, idx,
                    maxodr, TT,
                    FMM_stat, slw, dr, dt*rs[ir], dp*rs[ir]*sin_ts[it], 
                    &travt_stat);
            } else {
                travt = get_neighbour_travt(
                    nr, nt, np, ntp,
                    ir, it, ip, idx,
                    maxodr, TT,
                    FMM_stat, slw, dr, dt, dp, 
                    &travt_stat);
            }
            // set back
            TT[idx] = t_bak0;

            if(travt_stat>=0){
                if(t_bak < travt) travt = t_bak;
            } else {
                travt = t_bak;
            }
            

            if(travt < TT[idx]) {
                TT[idx] = travt;
                FMM_stat[idx] = FMM_ALV;
            }
            
        }}}

        isweep++;
        
    }

    free(FMM_stat);

}