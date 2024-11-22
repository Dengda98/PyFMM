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
#include <omp.h>
#include <sys/time.h>

#include "fsm.h"
#include "fmm.h"
#include "const.h"
#include "index.h"
#include "mallocfree.h"
#include "progressbar.h"


void set_fsm_num_threads(int num_threads){
#ifdef _OPENMP
    omp_set_num_threads(num_threads);
#endif
}

int FastSweeping(
    const double *rs, int nr, 
    const double *ts, int nt, 
    const double *ps, int np,
    double rr,  double tt, double pp,
    int maxodr,  const MYREAL *Slw, 
    MYREAL *TT, bool sphcoord, 
    int rfgfac, int rfgn, bool printbar, 
    double eps, int maxLoops, bool isparallel)
{
    // 程序运行开始时间
    struct timeval begin_t;
    gettimeofday(&begin_t, NULL);

    int ntp=nt*np;
    int nrtp=nr*ntp;
    char *FMM_stat = (char *)malloc1d(nrtp, sizeof(char));

    // All non-zero value of TT will be treated as efficient value,
    // and set FMM_ALV
    bool allzeroTT = true; 
    for(int i=0; i<nrtp; ++i){
        if(TT[i] == 0.0){
            TT[i] = 9.9e30f;// init FAR Traveltime 
            FMM_stat[i] = FMM_FAR;
        } else {
            FMM_stat[i] = FMM_ALV;
            allzeroTT = false;
        }
    }

    
    // if all zero in TT, then use rr, tt, pp
    if(allzeroTT){
        if(rfgfac>1 && rfgn>=1){
            init_source_TT_refinegrid(
                rs, nr, ts, nt, ps, np,
                rr, tt, pp,
                maxodr, Slw, TT, 
                FMM_stat, sphcoord,
                rfgfac, rfgn, printbar,
                NULL, NULL, NULL, NULL, NULL);
        } else {
            init_source_TT(
            rs, nr, ts, nt, ps, np, 
            rr, tt, pp,
            Slw, TT, 
            FMM_stat, sphcoord,
            NULL, NULL, NULL, NULL, NULL); 
        }
    }

    int nsweep;
    nsweep = FastSweeping_with_initial(
        rs, nr, 
        ts, nt, 
        ps, np,
        maxodr, Slw, TT,
        FMM_stat, sphcoord, printbar, 
        eps, maxLoops, isparallel);

    free(FMM_stat);


    // 程序运行结束时间
    struct timeval end_t;
    gettimeofday(&end_t, NULL);
    if(printbar) printf("Runtime: %.3f s\n", (end_t.tv_sec - begin_t.tv_sec) + (end_t.tv_usec - begin_t.tv_usec) / 1e6);
    fflush(stdout);

    return nsweep;
}



int FastSweeping_with_initial(
    const double *rs, int nr, 
    const double *ts, int nt, 
    const double *ps, int np,
    int maxodr,  const MYREAL *Slw, MYREAL *TT, 
    char *FMM_stat, bool sphcoord, bool printbar, 
    double eps, int maxLoops, bool isparallel)
{
    if(! isparallel) set_fsm_num_threads(1);

    double dr = (nr>1)? rs[1] - rs[0] : 0.0;
    double dt = (nt>1)? ts[1] - ts[0] : 0.0;
    double dp = (np>1)? ps[1] - ps[0] : 0.0;

    // convenient arrays
    double sin_ts[nt];
    if(sphcoord){
        for(int it=0; it<nt; ++it){
            sin_ts[it] = fabs(sin(ts[it]));
            if(sin_ts[it] < 1e-12) sin_ts[it] += 1e-12;
        }
    }

    int ntp=nt*np;
    int nrtp=nr*ntp;


    // DON'T CHANGE.
    static const char xr[6] = {-1, 1,  0, 0,  0, 0};
    static const char xt[6] = { 0, 0, -1, 1,  0, 0};
    static const char xp[6] = { 0, 0,  0, 0, -1, 1};
    const double hrtp[6] = {dr, dr, dt, dt, dp, dp};
    static const bool direcr_arr[8] = {1, 0, 0, 1, 1, 0, 0, 1};
    static const bool direct_arr[8] = {1, 1, 0, 0, 1, 1, 0, 0};
    static const bool direcp_arr[8] = {1, 1, 1, 1, 0, 0, 0, 0};


    MYREAL *TT_thread_all = NULL;
    char *FMM_stat_thread_all = NULL;
    

    if(isparallel){
        TT_thread_all = (MYREAL *)malloc1d(nrtp*8, sizeof(MYREAL));
        FMM_stat_thread_all = (char *)malloc1d(nrtp*8, sizeof(char));
    }
    

    int iloop=0, nloop=maxLoops, nsweep=0;
    // if(! isparallel) nloop = 1;

    // compute MAX UPDATE in sweeping
    MYREAL maxUpdate=0.0;

    while(iloop++ < nloop){

        if(isparallel){
            // init and copy data 
            for(int i=0; i<nrtp; ++i){
                char stat = FMM_FAR;
                if(FMM_stat[i]!=FMM_FAR) stat = FMM_ALV;
                for(int k=0; k<8; ++k){
                    TT_thread_all[i+k*nrtp] = TT[i];
                    FMM_stat_thread_all[i+k*nrtp] = stat;
                }
            }
        }


        #pragma omp parallel for default(shared)
        for(int isweep=0; isweep<8; ++isweep){
            // not use break, but continue, to make thread safe
            // break in advance for sequential mode
            if(!isparallel && iloop > 1 && eps > 0.0 && maxUpdate <= eps) continue;

            MYREAL *TT_thread = NULL;
            char *FMM_stat_thread = NULL;

            if(isparallel){
                TT_thread = TT_thread_all + isweep*nrtp;
                FMM_stat_thread = FMM_stat_thread_all + isweep*nrtp;
            } else {
                TT_thread = TT;
                FMM_stat_thread = FMM_stat;
                maxUpdate = 0.0;
            }
            

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


            direcr = direcr_arr[isweep];
            direct = direct_arr[isweep];
            direcp = direcp_arr[isweep];

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
            MYREAL update0;

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

                    if(FMM_stat_thread[jdx]==FMM_FAR) continue; 

                    // forever
                    FMM_stat_thread[jdx] = FMM_ALV;

                    if(mintravt > TT_thread[jdx] || mintravt < 0) {
                        mintravt = TT_thread[jdx];
                        mintravt_h = hrtp[k];
                        // modify interval for spherical coordinate
                        if(sphcoord && k>=2){
                            if(k<4) mintravt_h *= rs[ir];
                            else if(k<6) mintravt_h *= rs[ir]*sin_ts[it];
                        }

                        t_bak0 = mintravt + mintravt_h * slw;
                        if(t_bak > t_bak0 || t_bak < 0){
                            t_bak = t_bak0;
                        }
                    }
                }
                if(mintravt < 0.0) continue;
            
                // if(mintravt > maxnghT)  continue;

                MYREAL travt;
                char travt_stat;

                // temporary set 
                t_bak0 = TT_thread[idx];
                if(t_bak0 > t_bak) TT_thread[idx] = t_bak;

                if(sphcoord){
                    travt = get_neighbour_travt(
                        nr, nt, np, ntp,
                        ir, it, ip, idx,
                        maxodr, TT_thread,
                        FMM_stat_thread, slw, dr, dt*rs[ir], dp*rs[ir]*sin_ts[it], 
                        &travt_stat);
                } else {
                    travt = get_neighbour_travt(
                        nr, nt, np, ntp,
                        ir, it, ip, idx,
                        maxodr, TT_thread,
                        FMM_stat_thread, slw, dr, dt, dp, 
                        &travt_stat);
                }
                // set back
                TT_thread[idx] = t_bak0;

                if(travt_stat>=0 && travt>0){
                    if(t_bak < travt) travt = t_bak;
                } else {
                    travt = t_bak;
                }
                

                if(travt < TT_thread[idx]) {
                    if(! isparallel){
                        update0 = fabs(TT_thread[idx] - travt);
                        if(update0 > maxUpdate) maxUpdate = update0;
                    }

                    TT_thread[idx] = travt;
                    FMM_stat_thread[idx] = FMM_ALV;
                }
                
            }}} // end sweep in one direction

            // if(!isparallel)  printf("isweep=%d, maxUpdate=%f\n", isweep, maxUpdate);

        } // end 8 sweeps for-loop

        nsweep += 8;

        if(isparallel){
            // merge results
            maxUpdate = 0.0;
            MYREAL minTT, update;
            for(int i=0; i<nrtp; ++i){
                minTT = 9.9e30;
                for(int k=0; k<8; ++k){
                    update = TT_thread_all[i+k*nrtp];
                    if(minTT > update) minTT = update;
                }

                if(minTT < TT[i]){
                    update = fabs(minTT - TT[i]);
                    if(update > maxUpdate) maxUpdate = update;
                    TT[i] = minTT;
                }  
            }
    
            // printf("iloop=%d, maxUpdate=%f\n", iloop, maxUpdate);
        } 


        // break in advance
        if(eps > 0.0 && maxUpdate <= eps) break;

        for(int i=0; i<nrtp; ++i){
            FMM_stat[i] = FMM_ALV;
        }

    }  // end while loop

    if(TT_thread_all!=NULL)          free(TT_thread_all);
    if(FMM_stat_thread_all!=NULL)    free(FMM_stat_thread_all);


    return nsweep;
}

