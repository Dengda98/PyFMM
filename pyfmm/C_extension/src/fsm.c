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

#include "fmm/fsm.h"
#include "fmm/fmm.h"
#include "fmm/const.h"
#include "fmm/index.h"
#include "fmm/mallocfree.h"
#include "fmm/progressbar.h"


void set_fsm_num_threads(size_t num_threads){
#ifdef _OPENMP
    omp_set_num_threads((int)num_threads);
#endif
}

size_t FastSweeping(
    const real_t *rs, size_t nr,
    const real_t *ts, size_t nt,
    const real_t *ps, size_t np,
    real_t rr,  real_t tt, real_t pp,
    size_t maxodr,  const real_t *Slw,
    real_t *TT, bool sphcoord,
    size_t rfgfac, size_t rfgn, bool printbar,
    real_t eps, size_t maxLoops, bool isparallel)
{
    // 程序运行开始时间
    struct timeval begin_t;
    gettimeofday(&begin_t, NULL);

    size_t ntp=nt*np;
    size_t nrtp=nr*ntp;
    char *FMM_stat = (char *)malloc1d(nrtp, sizeof(char));

    // All non-zero value of TT will be treated as efficient value,
    // and set FMM_ALV
    bool allzeroTT = true; 
    for(size_t i=0; i<nrtp; ++i){
        if(TT[i] == 0.0){
            TT[i] = 9.9e30;// init FAR Traveltime
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

    size_t nsweep;
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



size_t FastSweeping_with_initial(
    const real_t *rs, size_t nr,
    const real_t *ts, size_t nt,
    const real_t *ps, size_t np,
    size_t maxodr,  const real_t *Slw, real_t *TT,
    char *FMM_stat, bool sphcoord, bool printbar, 
    real_t eps, size_t maxLoops, bool isparallel)
{
    if(! isparallel) set_fsm_num_threads(1);

    real_t dr = (nr>1)? rs[1] - rs[0] : 0.0;
    real_t dt = (nt>1)? ts[1] - ts[0] : 0.0;
    real_t dp = (np>1)? ps[1] - ps[0] : 0.0;

    // convenient arrays
    real_t sin_ts[nt];
    if(sphcoord){
        for(size_t it=0; it<nt; ++it){
            sin_ts[it] = fabs(sin(ts[it]));
            if(sin_ts[it] < 1e-12) sin_ts[it] += 1e-12;
        }
    }

    size_t ntp=nt*np;
    size_t nrtp=nr*ntp;


    // DON'T CHANGE.
    static const int xr[6] = {-1, 1,  0, 0,  0, 0};
    static const int xt[6] = { 0, 0, -1, 1,  0, 0};
    static const int xp[6] = { 0, 0,  0, 0, -1, 1};
    const real_t hrtp[6] = {dr, dr, dt, dt, dp, dp};
    static const bool direcr_arr[8] = {1, 0, 0, 1, 1, 0, 0, 1};
    static const bool direct_arr[8] = {1, 1, 0, 0, 1, 1, 0, 0};
    static const bool direcp_arr[8] = {1, 1, 1, 1, 0, 0, 0, 0};


    real_t *TT_thread_all = NULL;
    char *FMM_stat_thread_all = NULL;
    

    if(isparallel){
        TT_thread_all = (real_t *)malloc1d(nrtp*8, sizeof(real_t));
        FMM_stat_thread_all = (char *)malloc1d(nrtp*8, sizeof(char));
    }
    

    size_t iloop=0, nloop=maxLoops, nsweep=0;
    // if(! isparallel) nloop = 1;

    // compute MAX UPDATE in sweeping
    real_t maxUpdate=0.0;

    while(iloop < nloop){
        ++iloop;

        if(isparallel){
            // init and copy data 
            for(size_t i=0; i<nrtp; ++i){
                char stat = FMM_FAR;
                if(FMM_stat[i]!=FMM_FAR) stat = FMM_ALV;
                for(size_t k=0; k<8; ++k){
                    TT_thread_all[i+k*nrtp] = TT[i];
                    FMM_stat_thread_all[i+k*nrtp] = stat;
                }
            }
        }


        #pragma omp parallel for default(shared)
        for(size_t isweep=0; isweep<8; ++isweep){
            // not use break, but continue, to make thread safe
            // break in advance for sequential mode
            if(!isparallel && iloop > 1 && eps > 0.0 && maxUpdate <= eps) continue;

            real_t *TT_thread = NULL;
            char *FMM_stat_thread = NULL;

            if(isparallel){
                TT_thread = TT_thread_all + isweep*nrtp;
                FMM_stat_thread = FMM_stat_thread_all + isweep*nrtp;
            } else {
                TT_thread = TT;
                FMM_stat_thread = FMM_stat;
                maxUpdate = 0.0;
            }
            

            bool direcr = direcr_arr[isweep];
            bool direct = direct_arr[isweep];
            bool direcp = direcp_arr[isweep];
            size_t idx;

            real_t mintravt=-999.0;
            real_t mintravt_h=0.0;
            real_t slw;
            real_t update0;

            // Start Sweeping
            // 使用非负计数控制扫描方向，避免倒序循环在零处下溢
            for(size_t qr=0; qr<nr; ++qr){
                size_t ir = direcr ? qr : nr-1-qr;
                for(size_t qt=0; qt<nt; ++qt){
                    size_t it = direct ? qt : nt-1-qt;
                    for(size_t qp=0; qp<np; ++qp){
                        size_t ip = direcp ? qp : np-1-qp;
                        ravel_index(&idx, ntp, np, ir, it, ip);
                        slw = Slw[idx];

                        // find minimum traveltime in 6 neighbours
                        real_t t_bak=-999.9, t_bak0=-999.9;
                        mintravt=-999.0;
                        mintravt_h=0.0;
                        for(size_t k=0; k<6; ++k){
                            size_t jdx;
                            size_t iir, iit, iip;
                            if(!neighbour_index(ir, nr, xr[k], &iir)) continue;
                            if(!neighbour_index(it, nt, xt[k], &iit)) continue;
                            if(!neighbour_index(ip, np, xp[k], &iip)) continue;

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

                        real_t travt;
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

                    }
                }
            } // end sweep in one direction

            // if(!isparallel)  printf("isweep=%d, maxUpdate=%f\n", isweep, maxUpdate);

        } // end 8 sweeps for-loop

        nsweep += 8;

        if(isparallel){
            // merge results
            maxUpdate = 0.0;
            real_t minTT, update;
            for(size_t i=0; i<nrtp; ++i){
                minTT = 9.9e30;
                for(size_t k=0; k<8; ++k){
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

        for(size_t i=0; i<nrtp; ++i){
            FMM_stat[i] = FMM_ALV;
        }

    }  // end while loop

    if(TT_thread_all!=NULL)          free(TT_thread_all);
    if(FMM_stat_thread_all!=NULL)    free(FMM_stat_thread_all);


    return nsweep;
}

