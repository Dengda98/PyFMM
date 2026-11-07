/**
 * @file   fmm.c
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
*/

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/time.h>

#include "fmm/const.h"
#include "fmm/interp.h"
#include "fmm/query.h"
#include "fmm/coord.h"
#include "fmm/mallocfree.h"
#include "fmm/diff.h"
#include "fmm/heapsort.h"
#include "fmm/index.h"
#include "fmm/progressbar.h"
#include "fmm/fmm.h"



void FastMarching(
    const real_t *rs, size_t nr,
    const real_t *ts, size_t nt,
    const real_t *ps, size_t np,
    real_t rr,  real_t tt, real_t pp,
    size_t maxodr,  const real_t *Slw,
    real_t *TT, bool sphcoord,
    size_t rfgfac, size_t rfgn, bool printbar)
{
    // 程序运行开始时间
    struct timeval begin_t;
    gettimeofday(&begin_t, NULL);

    size_t ntp=nt*np;
    size_t nrtp=nr*ntp;
    size_t Ndots=nrtp;

    char *FMM_stat = (char *)malloc1d(nrtp, sizeof(char)); // 1 alive, 0 close, -1 far

    size_t heapsize=0, heapcapcity=nr*nt + ntp + nr*np;
    size_t *psize, *pcap;
    psize = &heapsize;
    pcap = &heapcapcity;
    HEAP_DATA *FMM_data = (HEAP_DATA *)malloc1d(heapcapcity, sizeof(HEAP_DATA));
    size_t *NroIdx = (size_t *)malloc1d(nrtp, sizeof(size_t));

    // All non-zero value of TT will be treated as efficient value,
    // and set FMM_CLS
    bool allzeroTT = true; 
    for(size_t i=0; i<nrtp; ++i){
        if(TT[i] == 0.0){
            TT[i] = 9.9e30;// init FAR Traveltime
            FMM_stat[i] = FMM_FAR;
        } else {
            FMM_data = HeapPush(FMM_data, psize, pcap, i, NroIdx, TT);
            FMM_stat[i] = FMM_CLS;

            Ndots--;
            allzeroTT = false;
        }
    }


    // if all zero in TT, then use rr, tt, pp
    if(allzeroTT){
        if(rfgfac>1 && rfgn>=1){
            FMM_data = init_source_TT_refinegrid(
                rs, nr, ts, nt, ps, np,
                rr, tt, pp,
                maxodr, Slw, TT, 
                FMM_stat, sphcoord,
                rfgfac, rfgn, printbar,
                FMM_data, psize, pcap, NroIdx, &Ndots);
        } else {
            FMM_data = init_source_TT(
            rs, nr, ts, nt, ps, np, 
            rr, tt, pp,
            Slw, TT, 
            FMM_stat, sphcoord,
            FMM_data, psize, pcap, NroIdx, &Ndots); 
        }
    }
     
    // print_FMM_HEAP(FMM_data, *psize, nr, nt, np, NroIdx, TT, NULL, NULL, NULL);

    FMM_data = FastMarching_with_initial(
        rs, nr, 
        ts, nt, 
        ps, np,
        maxodr, Slw, TT,
        FMM_stat, sphcoord, NULL, printbar,
        FMM_data, psize, pcap, NroIdx, &Ndots);

    // printf("done, Ndots=%d, size=%d\n", Ndots, *psize);
    free(FMM_data);
    free(FMM_stat);
    free(NroIdx);


    // 程序运行结束时间
    struct timeval end_t;
    gettimeofday(&end_t, NULL);
    if(printbar) printf("Runtime: %.3f s\n", (end_t.tv_sec - begin_t.tv_sec) + (end_t.tv_usec - begin_t.tv_usec) / 1e6);
    fflush(stdout);
}




HEAP_DATA * FastMarching_with_initial(
    const real_t *rs, size_t nr,
    const real_t *ts, size_t nt,
    const real_t *ps, size_t np,
    size_t maxodr,  const real_t *Slw, real_t *TT,
    char *FMM_stat, bool sphcoord, bool *edgeStop, bool printbar,
    HEAP_DATA *FMM_data, size_t *psize, size_t *pcap, size_t *NroIdx, size_t *pNdots)
{
    real_t dr = (nr>1)? rs[1] - rs[0] : 0.0;
    real_t dt = (nt>1)? ts[1] - ts[0] : 0.0;
    real_t dp = (np>1)? ps[1] - ps[0] : 0.0;

    // DON'T CHANGE.
    static const int xr[6] = {-1, 1,  0, 0,  0, 0};
    static const int xt[6] = { 0, 0, -1, 1,  0, 0};
    static const int xp[6] = { 0, 0,  0, 0, -1, 1};

    // convenient arrays
    real_t sin_ts[nt];
    if(sphcoord){
        for(size_t it=0; it<nt; ++it){
            sin_ts[it] = fabs(sin(ts[it]));
            if(sin_ts[it] < 1e-12) sin_ts[it] += 1e-12;
        }
    }

    size_t ntp=nt*np;

    HEAP_DATA popdata, newdata;
    size_t ir0, it0, ip0, ir, it, ip;
    size_t idx, idx0;
    real_t s;
    real_t travt_bak, travt, travt0, travt1;
    real_t h;

    real_t *pt;
    char *pstat;

    // 打印进度条时每隔print_interv打印一次
    size_t size_bak = nr*ntp;
    size_t last_barpercent = 0, barpercent;

    // printf("loop start, size=%d\n", *psize );
    char travt_stat;
    real_t maxtravt=-999;
    while(*psize > 0){

        // get the minimum one 
        popdata = HeapPop(FMM_data, psize, NroIdx, TT);
        idx0 = popdata;
        FMM_stat[idx0] = FMM_ALV;

        unravel_index(idx0, ntp, np, &ir0, &it0, &ip0);
        travt0 = TT[idx0];

        // break loop in advance when reach the boundary
        if( edgeStop!=NULL && (
            (edgeStop[0]&&ir0==0) || (edgeStop[1]&&ir0==nr-1) || 
            (edgeStop[2]&&it0==0) || (edgeStop[3]&&it0==nt-1) || 
            (edgeStop[4]&&ip0==0) || (edgeStop[5]&&ip0==np-1)))  break;

        

        if(travt0 > maxtravt) maxtravt = travt0;
        else if(travt0 < maxtravt){
            // 当在源点附近使用加密网格时，由于需要使用一般方法初始化源点附近的走时，
            printf("pNdots=%zu, WRONG! travt0(%f) < maxtravt(%f) \n", *pNdots, travt0, maxtravt);
            print_HEAP(FMM_data, *psize, nr, nt, np, NroIdx, TT, NULL, NULL, NULL);
            printf("Tiny bug here, please let author know.\n");
        } 

        // get neighbours (max 6)
        for(size_t k=0; k<6; ++k){
            
            if(!neighbour_index(ir0, nr, xr[k], &ir)) continue;
            if(!neighbour_index(it0, nt, xt[k], &it)) continue;
            if(!neighbour_index(ip0, np, xp[k], &ip)) continue;
            
            ravel_index(&idx, ntp, np, ir, it, ip);
            // idx6_bak[k] = idx;

            pstat = FMM_stat+idx;
            // skip alive point 
            if(*pstat == FMM_ALV) continue;

            if(k<2){
                h = dr;
            } else if(k<4){
                h = dt;
                if(sphcoord) h *= rs[ir];
            } else if(k<6){
                h = dp;
                if(sphcoord) h *= rs[ir]*sin_ts[it];
            } else {
                fprintf(stderr, "BAD interval h\n");
                exit(EXIT_FAILURE);
            }

            s = Slw[idx];
            
            travt1 = travt0 + h*s;
            // compute traveltime
            pt = TT+idx;
            travt_bak = *pt;
            if(travt1 < travt_bak)  *pt = travt1;

            if(sphcoord){
                travt = get_neighbour_travt(
                    nr, nt, np, ntp,
                    ir, it, ip, idx,
                    maxodr, TT,
                    FMM_stat, s, dr, dt*rs[ir], dp*rs[ir]*sin_ts[it], 
                    &travt_stat);
            } else {
                travt = get_neighbour_travt(
                    nr, nt, np, ntp,
                    ir, it, ip, idx,
                    maxodr, TT,
                    FMM_stat, s, dr, dt, dp, 
                    &travt_stat);
            }
            
            // printf("k, travt, travt_bak = %d, %f, %f\n", k, travt, travt_bak);
            *pt = travt_bak;
            if(travt_stat<0 || travt<0) {
                // printf("get_neighbour_travt failed, use the lazy one.\n");
                travt = travt1;
            }

            // Forced Causality
            if(travt < maxtravt) travt = maxtravt;

            if(travt < TT[idx]){
                // printf("get, travt, TT[idx] = %f, %f\n", travt, TT[idx]);
                TT[idx] = travt;

                if(*pstat == FMM_CLS){ // CLOSE
                    MinHeap_AdjustUp(FMM_data, NroIdx[idx], NroIdx, TT);
                }
                else if(*pstat == FMM_FAR){ // FAR
                    newdata= idx;
                    FMM_data = HeapPush(FMM_data, psize, pcap, newdata, NroIdx, TT);
                    *pstat = FMM_CLS;
                    (*pNdots)--;
                }
                
            }
            // print_FMM_HEAP(FMM_data, *psize, nr, nt, np, NroIdx, TT, gTr, gTt, gTp);

        } 


        // 打印进度条 
        barpercent = 100.0 - (real_t)(*pNdots) / (real_t)(size_bak) * 100.0;
        if(printbar && barpercent != last_barpercent){
            printprogressBar("Fast Marching...  ", barpercent);
            // printf("\npNdots=%zu, barpercent=%d, size_bak=%d\n", *pNdots, barpercent, size_bak);
            last_barpercent = barpercent;
        }

    }


    return FMM_data;
}





HEAP_DATA * init_source_TT(
    const real_t *rs, size_t nr,
    const real_t *ts, size_t nt,
    const real_t *ps, size_t np,
    real_t rr, real_t tt, real_t pp,
    const real_t *Slw, real_t *TT,
    char *FMM_stat, bool sphcoord,
    HEAP_DATA *FMM_data, size_t *psize, size_t *pcap, size_t *NroIdx, size_t *pNdots)
{   
    size_t ir, it, ip;
    ir = dicho_find(rs, nr, rr);
    it = dicho_find(ts, nt, tt);
    ip = dicho_find(ps, np, pp);
    if(ir==nr-1 && ir>0)    ir--;
    if(it==nt-1 && it>0)    it--;
    if(ip==np-1 && ip>0)    ip--;
    

    real_t xx, yy, zz;
    if(sphcoord) rtp2xyz(rr, tt, pp, &xx, &yy, &zz);
    

    HEAP_DATA newdata;
    real_t travt, s;
    size_t jr, jt, jp;
    size_t jdx;
    real_t dist;
    real_t r2, t2, p2, dr, dt, dp;
    real_t x2, y2, z2;
    real_t dx, dy, dz;
    
    real_t mtravt=9.9e30;
    size_t mir, mit, mip;
    size_t midx=0;  // 最小走时节点的索引

    // Tiny 2x2x2 cube
    for(size_t kr=0; kr<2; ++kr){
        jr = ir+kr;
        if(jr > nr-1) continue;
        
        r2 = rs[jr];
        dr = r2 - rr;
        for(size_t kt=0; kt<2; ++kt){
            jt = it+kt;
            if(jt > nt-1) continue;

            t2 = ts[jt];
            dt = t2 - tt;
            for(size_t kp=0; kp<2; ++kp){
                jp = ip+kp;
                if(jp > np-1) continue;

                p2 = ps[jp];
                dp = p2 - pp;

                if(sphcoord) rtp2xyz(r2, t2, p2, &x2, &y2, &z2);

                ravel_index(&jdx, nt*np, np, jr, jt, jp);
                s = Slw[jdx];
                
                if(sphcoord){
                    dx = x2-xx;
                    dy = y2-yy;
                    dz = z2-zz;
                    dist = sqrt(dx*dx + dy*dy + dz*dz);
                } else {
                    dist = sqrt(dr*dr + dt*dt + dp*dp);
                }
                
                
                travt = dist * s;
                TT[jdx] = travt;
                if(mtravt > travt){
                    midx = jdx;
                    mtravt = travt;
                }

                if(FMM_data!=NULL){
                    newdata = jdx;
                    FMM_data = HeapPush(FMM_data, psize, pcap, newdata, NroIdx, TT);
                }
                
                if(FMM_stat!=NULL) FMM_stat[jdx] = FMM_CLS;
                // print_FMM_HEAP(FMM_data, *psize, nr, nt, np, NroIdx);
                
                if(pNdots!=NULL) (*pNdots)--;
                // return FMM_data;
            }
        }
    }

    // set the node with minimum traveltime `alive`
    if(FMM_data!=NULL){
        HEAP_DATA popdata;
        popdata = HeapPop(FMM_data, psize, NroIdx, TT);
        midx = popdata;
    }
    if(FMM_stat!=NULL) FMM_stat[midx] = FMM_ALV;


    unravel_index(midx, nt*np, np, &mir, &mit, &mip);
    // Full 3x3x3 cube 
    for(int kr=-1; kr<2; ++kr){
        if(!neighbour_index(mir, nr, kr, &jr)) continue;

        r2 = rs[jr];
        dr = r2 - rr;
        for(int kt=-1; kt<2; ++kt){
            if(!neighbour_index(mit, nt, kt, &jt)) continue;

            t2 = ts[jt];
            dt = t2 - tt;
            for(int kp=-1; kp<2; ++kp){
                if(!neighbour_index(mip, np, kp, &jp)) continue;
                
                p2 = ps[jp];
                dp = p2 - pp;

                if(sphcoord) rtp2xyz(r2, t2, p2, &x2, &y2, &z2);

                ravel_index(&jdx, nt*np, np, jr, jt, jp);

                if(FMM_stat!=NULL && FMM_stat[jdx] != FMM_FAR) continue;

                s = Slw[jdx];
                
                if(sphcoord){
                    dx = x2-xx;
                    dy = y2-yy;
                    dz = z2-zz;
                    dist = sqrt(dx*dx + dy*dy + dz*dz);
                } else {
                    dist = sqrt(dr*dr + dt*dt + dp*dp);
                }
                
                
                travt = dist * s;
                TT[jdx] = travt;
                
                if(FMM_data!=NULL){
                    newdata = jdx;
                    FMM_data = HeapPush(FMM_data, psize, pcap, newdata, NroIdx, TT);
                }
                
                if(FMM_stat!=NULL) FMM_stat[jdx] = FMM_CLS;
                // print_FMM_HEAP(FMM_data, *psize, nr, nt, np, NroIdx);
                
                if(pNdots!=NULL) (*pNdots)--;
            }
        }
    }

    // print_HEAP(FMM_data, *psize, nr, nt, np, NroIdx, TT, NULL, NULL, NULL);

    return FMM_data;
}



HEAP_DATA * init_source_TT_refinegrid(
    const real_t *rs, size_t nr,
    const real_t *ts, size_t nt,
    const real_t *ps, size_t np,
    real_t rr, real_t tt, real_t pp,
    size_t maxodr,  const real_t *Slw, real_t *TT,
    char *FMM_stat, bool sphcoord,
    size_t rfgfac, size_t rfgn, // refine grid factor and number of grids
    bool printbar,
    HEAP_DATA *FMM_data, size_t *psize, size_t *pcap, size_t *NroIdx, size_t *pNdots)
{   
    real_t dr = (nr>1)? rs[1] - rs[0] : 0.0;
    real_t dt = (nt>1)? ts[1] - ts[0] : 0.0;
    real_t dp = (np>1)? ps[1] - ps[0] : 0.0;

    size_t ntp=nt*np;

    // find the closest point
    size_t ir, it, ip;
    ir = dicho_find(rs, nr, rr);
    it = dicho_find(ts, nt, tt);
    ip = dicho_find(ps, np, pp);
    if(ir<nr-1 && fabs(rs[ir+1]-rr) < fabs(rs[ir]-rr)) ir++;
    if(it<nt-1 && fabs(ts[it+1]-tt) < fabs(ts[it]-tt)) it++;
    if(ip<np-1 && fabs(ps[ip+1]-pp) < fabs(ps[ip]-pp)) ip++;

    real_t rfg_dr, rfg_dt, rfg_dp;
    rfg_dr = dr/rfgfac;
    rfg_dt = dt/rfgfac;
    rfg_dp = dp/rfgfac;
    
    size_t rfg_ir1, rfg_ir2;
    size_t rfg_it1, rfg_it2;
    size_t rfg_ip1, rfg_ip2;
    rfg_ir1 = rfg_ir2 = ir;
    rfg_it1 = rfg_it2 = it;
    rfg_ip1 = rfg_ip2 = ip;

    // 确定加密范围
    for(size_t i=0; i<rfgn; ++i){
        if(rfg_ir1>0)    rfg_ir1--;
        if(rfg_ir2<nr-1) rfg_ir2++;
        if(rfg_it1>0)    rfg_it1--;
        if(rfg_it2<nt-1) rfg_it2++;
        if(rfg_ip1>0)    rfg_ip1--;
        if(rfg_ip2<np-1) rfg_ip2++;
    }
    // printf("rfg_ir12: %d %d, rfg_it12: %d %d, rfg_ip12: %d %d\n ", 
    //         rfg_ir1, rfg_ir2, rfg_it1, rfg_it2, rfg_ip1, rfg_ip2);

    size_t rfg_nr, rfg_nt, rfg_np;
    size_t rfg_ntp;
    size_t rfg_nrtp;
    size_t rfg_Ndots;
    rfg_nr = (rfg_ir2 - rfg_ir1)*rfgfac + 1;
    rfg_nt = (rfg_it2 - rfg_it1)*rfgfac + 1;
    rfg_np = (rfg_ip2 - rfg_ip1)*rfgfac + 1;
    rfg_ntp = rfg_nt*rfg_np;
    rfg_nrtp = rfg_nr*rfg_ntp;
    rfg_Ndots = rfg_nrtp;

    real_t *rfg_rs = (real_t *)malloc1d(rfg_nr, sizeof(real_t));
    real_t *rfg_ts = (real_t *)malloc1d(rfg_nt, sizeof(real_t));
    real_t *rfg_ps = (real_t *)malloc1d(rfg_np, sizeof(real_t));
    
    for(size_t i=0; i<rfg_nr; ++i){
        rfg_rs[i] = rs[rfg_ir1] + rfg_dr*i;
    }
    for(size_t i=0; i<rfg_nt; ++i){
        rfg_ts[i] = ts[rfg_it1] + rfg_dt*i;
    }
    for(size_t i=0; i<rfg_np; ++i){
        rfg_ps[i] = ps[rfg_ip1] + rfg_dp*i;
    }

    real_t *rfg_TT = (real_t *)malloc1d(rfg_nrtp, sizeof(real_t));
    real_t *rfg_Slw = (real_t *)malloc1d(rfg_nrtp, sizeof(real_t));
    char *rfg_FMM_stat = (char *)malloc1d(rfg_nrtp, sizeof(char)); // 1 alive, 0 close, -1 far
    
    // 插值加密的慢度场
    for(size_t i=0; i<rfg_nrtp; ++i){
        rfg_TT[i] = 9.9e30;// init FAR Traveltime
        rfg_FMM_stat[i] = FMM_FAR;

        size_t ir0, it0, ip0;
        unravel_index(i, rfg_ntp, rfg_np, &ir0, &it0, &ip0);
        rfg_Slw[i] = trilinear_one_ravel(
            rs, nr, 
            ts, nt, 
            ps, np, ntp, Slw, 
            rfg_rs[ir0], rfg_ts[it0], rfg_ps[ip0], false,
            NULL, NULL, NULL, NULL, NULL);
    }

    size_t rfg_heapsize=0, rfg_heapcapcity=rfg_nr*rfg_nt + rfg_ntp + rfg_nr*rfg_np;
    size_t *prfg_size, *prfg_cap;
    prfg_size = &rfg_heapsize;
    prfg_cap = &rfg_heapcapcity;
    HEAP_DATA *rfg_FMM_data = (HEAP_DATA *)malloc1d(rfg_heapcapcity, sizeof(HEAP_DATA));
    size_t *rfg_NroIdx = (size_t *)malloc1d(rfg_nrtp, sizeof(size_t));

    rfg_FMM_data = init_source_TT(
        rfg_rs, rfg_nr, rfg_ts, rfg_nt, rfg_ps, rfg_np, 
        rr, tt, pp,
        rfg_Slw, rfg_TT, 
        rfg_FMM_stat, sphcoord,
        rfg_FMM_data, prfg_size, prfg_cap, rfg_NroIdx, &rfg_Ndots);   


    bool edgeStop[6] = {true, true, true, true, true, true};
    if(rfg_ir1==0)    edgeStop[0] = false;
    if(rfg_ir2==nr-1) edgeStop[1] = false;
    if(rfg_it1==0)    edgeStop[2] = false;
    if(rfg_it2==nt-1) edgeStop[3] = false;
    if(rfg_ip1==0)    edgeStop[4] = false;
    if(rfg_ip2==np-1) edgeStop[5] = false;
    rfg_FMM_data = FastMarching_with_initial(
        rfg_rs, rfg_nr, 
        rfg_ts, rfg_nt, 
        rfg_ps, rfg_np,
        maxodr, rfg_Slw, rfg_TT,
        rfg_FMM_stat, sphcoord, edgeStop, printbar, // break loop in advance
        rfg_FMM_data, prfg_size, prfg_cap, rfg_NroIdx, &rfg_Ndots);

    // record result to main TT 
    for(size_t jr=rfg_ir1, rfg_jr=0; jr<=rfg_ir2; ++jr, rfg_jr+=rfgfac){
    for(size_t jt=rfg_it1, rfg_jt=0; jt<=rfg_it2; ++jt, rfg_jt+=rfgfac){
        size_t jdx1=0, jdx2=0;
        bool has_point = false;
        for(size_t jp=rfg_ip1, rfg_jp=0; jp<=rfg_ip2; ++jp, rfg_jp+=rfgfac){
            size_t jdx, rfg_jdx;
            ravel_index(&jdx, ntp, np, jr, jt, jp);
            ravel_index(&rfg_jdx, rfg_ntp, rfg_np, rfg_jr, rfg_jt, rfg_jp);

            if(rfg_FMM_stat[rfg_jdx] == FMM_FAR) continue;

            // 将最外侧的节点加入堆
            if(!has_point) jdx1 = jdx;
            jdx2 = jdx;
            has_point = true;

            TT[jdx] = rfg_TT[rfg_jdx];
            // printf("rfg_TT = %f, jr, jt, jp %d, %d, %d\n", rfg_TT[rfg_jdx], jr, jt, jp);
            FMM_stat[jdx] = FMM_ALV;
            if(pNdots!=NULL) (*pNdots)--;
        }
        if(has_point){
            FMM_data = HeapPush(FMM_data, psize, pcap, jdx1, NroIdx, TT);
            FMM_stat[jdx1] = FMM_CLS;
        }
        if(has_point && jdx2!=jdx1){
            FMM_data = HeapPush(FMM_data, psize, pcap, jdx2, NroIdx, TT);
            FMM_stat[jdx2] = FMM_CLS;
        }

    }}

    free(rfg_TT);
    free(rfg_Slw);
    free(rfg_rs);
    free(rfg_ts);
    free(rfg_ps);

    free(rfg_FMM_data);
    free(rfg_NroIdx);


    return FMM_data;
}





real_t get_neighbour_travt(
    size_t nr, size_t nt, size_t np, size_t ntp,
    size_t ir, size_t it, size_t ip, size_t idx,
    size_t maxodr, real_t *TT,
    char *FMM_stat,  real_t s,
    real_t dr, real_t dt, real_t dp,
    char *stat)
{   
    if(stat!=NULL) *stat = 0;

    real_t Acoef, Bcoef, Ccoef;
    Acoef = Bcoef = 0.0;
    Ccoef = - s*s;

    real_t tarr[5], tarrR[5], tarrT[5], tarrP[5]; // max(maxodr) = 3
    tarr[0] = tarrR[0] = tarrT[0] = tarrP[0] = TT[idx];
    size_t odr, odrR, odrT, odrP;
    odr = odrR = odrT = odrP = 0;

    size_t jdx;
    size_t i;

    char sgn_r, sgn_t, sgn_p;
    sgn_r = sgn_t = sgn_p = 0;
    real_t dif, pos_dif, neg_dif;
    real_t acoef, bcoef;
    real_t pos_acoef, neg_acoef, pos_bcoef, neg_bcoef;
    //------------------------------------------- R ---------------------------------------
    // --------------------------------------- negative -----------------------------------
    for(odr=0; odr<maxodr; ++odr){
        if(odr>=ir) break;
        jdx = idx - (odr+1)*ntp;
        if(FMM_stat[jdx]!=FMM_ALV) break;
        tarr[odr+1] = TT[jdx];
        if(tarr[odr+1] >= TT[jdx+ntp]) break;
    }
    get_diff_odr123(odr, tarr, dr, &neg_acoef, &neg_bcoef, &neg_dif);
    // --------------------------------------- positive -----------------------------------
    for(odrR=0; odrR<maxodr; ++odrR){
        if(odrR>=nr-ir-1) break;
        jdx = idx + (odrR+1)*ntp;
        if(FMM_stat[jdx]!=FMM_ALV) break;
        tarrR[odrR+1] = TT[jdx];
        if(tarrR[odrR+1] >= TT[jdx-ntp]) break;
    }
    get_diff_odr123(odrR, tarrR, dr, &pos_acoef, &pos_bcoef, &pos_dif);
    // compare positive and negative 
    if(neg_dif < pos_dif){
        dif = pos_dif;
        acoef = pos_acoef; // ignore *(-1)
        bcoef = pos_bcoef; // ignore *(-1)
        sgn_r = -1;
    } else {
        dif = neg_dif;
        acoef = neg_acoef;
        bcoef = neg_bcoef;
        sgn_r = 1;
        odrR = odr;
        for(i=0; i<=odrR; tarrR[i]=tarr[i], ++i);
    }
    if(dif < 0.0) acoef = bcoef = 0.0;
    Acoef += acoef*acoef;
    Bcoef += 2*acoef*bcoef;
    Ccoef += bcoef*bcoef;

    
    //------------------------------------------- T ---------------------------------------
    // --------------------------------------- negative -----------------------------------
    for(odr=0; odr<maxodr; ++odr){
        if(odr>=it) break;
        jdx = idx - (odr+1)*np;
        if(FMM_stat[jdx]!=FMM_ALV) break;
        tarr[odr+1] = TT[jdx];
        if(tarr[odr+1] >= TT[jdx+np]) break;
    }
    get_diff_odr123(odr, tarr, dt, &neg_acoef, &neg_bcoef, &neg_dif);
    // --------------------------------------- positive -----------------------------------
    for(odrT=0; odrT<maxodr; ++odrT){
        if(odrT>=nt-it-1) break;
        jdx = idx + (odrT+1)*np;
        if(FMM_stat[jdx]!=FMM_ALV) break;
        tarrT[odrT+1] = TT[jdx];
        if(tarrT[odrT+1] >= TT[jdx-np]) break;
    }
    get_diff_odr123(odrT, tarrT, dt, &pos_acoef, &pos_bcoef, &pos_dif);
    // compare positive and negative 
    if(neg_dif < pos_dif){
        dif = pos_dif;
        acoef = pos_acoef; // ignore *(-1)
        bcoef = pos_bcoef; // ignore *(-1)
        sgn_t = -1;
    } else {
        dif = neg_dif;
        acoef = neg_acoef;
        bcoef = neg_bcoef;
        sgn_t = 1;
        odrT = odr;
        for(i=0; i<=odrT; tarrT[i]=tarr[i], ++i);
    }
    if(dif < 0.0){
        acoef = bcoef = 0.0;
    }
    Acoef += acoef*acoef;
    Bcoef += 2*acoef*bcoef;
    Ccoef += bcoef*bcoef;


    //------------------------------------------- P ---------------------------------------
    // --------------------------------------- negative -----------------------------------
    for(odr=0; odr<maxodr; ++odr){
        if(odr>=ip) break;
        jdx = idx - (odr+1);
        if(FMM_stat[jdx]!=FMM_ALV) break;
        tarr[odr+1] = TT[jdx];
        if(tarr[odr+1] >= TT[jdx+1]) break;
    }
    get_diff_odr123(odr, tarr, dp, &neg_acoef, &neg_bcoef, &neg_dif);
    // --------------------------------------- positive -----------------------------------
    for(odrP=0; odrP<maxodr; ++odrP){
        if(odrP>=np-ip-1) break;
        jdx = idx + (odrP+1);
        if(FMM_stat[jdx]!=FMM_ALV) break;
        tarrP[odrP+1] = TT[jdx];
        if(tarrP[odrP+1] >= TT[jdx-1]) break;
    }
    get_diff_odr123(odrP, tarrP, dp, &pos_acoef, &pos_bcoef, &pos_dif);
    // compare positive and negative 
    if(neg_dif < pos_dif){
        dif = pos_dif;
        acoef = pos_acoef; // ignore *(-1)
        bcoef = pos_bcoef; // ignore *(-1)
        sgn_p = -1;
    } else {
        dif = neg_dif;
        acoef = neg_acoef;
        bcoef = neg_bcoef;
        sgn_p = 1;
        odrP = odr;
        for(i=0; i<=odrP; tarrP[i]=tarr[i], ++i);
    }
    if(dif < 0.0){
        acoef = bcoef = 0.0;
    }
    Acoef += acoef*acoef;
    Bcoef += 2*acoef*bcoef;
    Ccoef += bcoef*bcoef;
    



    //--------------------------------------------------
    // solve second-order equation with one unknown
    // (A*T^2 - B*T + C = 0)
    real_t jdg = Bcoef*Bcoef - 4*Acoef*Ccoef;
    if(jdg <= 0.0) jdg = 0.0;
    if(fabs(Acoef)<1e-10 || fabs(Bcoef)<1e-10){
        if(stat!=NULL) *stat = -1;
#if _PRINT_ODR_BUG_ == 1
        printf("Acoef, Bcoef, Ccoef = %f, %f, %f\n", Acoef, Bcoef, Ccoef);
        printf("jdg=%f <= 0.0\n", jdg);
        getchar();
#endif      
        return -1.0;
    }
        

    jdg = sqrt(jdg);
#if _PRINT_ODR_BUG_ == 1
    {
    printf("Acoef, Bcoef, Ccoef = %f, %f, %f\n", Acoef, Bcoef, Ccoef);
    printf("res=%f\n", (Bcoef + jdg)/(2*Acoef));
    // getchar();
    }
#endif
    return (Bcoef + jdg)/(2.0*Acoef);

}





real_t FMM_raytracing(
    const real_t *rs, size_t nr,
    const real_t *ts, size_t nt,
    const real_t *ps, size_t np,
    real_t r0, real_t t0, real_t p0,
    real_t rr, real_t tt, real_t pp, real_t seglen, real_t segfac,
    const real_t *Slw, const real_t *TT, bool sphcoord,
    // real_t *gTr, real_t *gTt, real_t *gTp,
    real_t *rays, size_t *N)
{
    real_t dr = (nr>1)? rs[1] - rs[0] : 1e-6;
    real_t dt = (nt>1)? ts[1] - ts[0] : 1e-6;
    real_t dp = (np>1)? ps[1] - ps[0] : 1e-6;
    real_t seglen0 = seglen;

    real_t xx, yy, zz, x0, y0, z0;
    real_t dx, dy, dz;
    if(sphcoord) {
        rtp2xyz(r0, t0, p0, &x0, &y0, &z0);
        rtp2xyz(rr, tt, pp, &xx, &yy, &zz);
    }

    
    size_t ntp = nt*np;
    
    size_t idot = 0;

    real_t gtr, gtt, gtp, norm;
    real_t gtr_next, gtt_next, gtp_next;
    real_t limitdist;
    if(sphcoord){
        limitdist = sqrt(dr*dr + pow(dt*rs[0],2) + pow(dp*rs[0]*sin(ts[0]),2));
    } else {
        limitdist = sqrt(dr*dr + dt*dt + dp*dp);
    }
    if(limitdist < segfac*seglen) limitdist = segfac*seglen;

    real_t r1, t1, p1;
    real_t x1, y1, z1;
    real_t r11, t11, p11;
    real_t rmid, tmid, pmid, vmid;
    r1 = rr;
    t1 = tt;
    p1 = pp;
    if(sphcoord) {
        x1 = xx;
        y1 = yy;
        z1 = zz;
    }

    // strictly speaking, r0,t0,p0 should be used here, 
    // however here gradient equals zero,
    trilinear_one_ravel(rs, nr, ts, nt, ps, np, ntp, TT, r0+dr, t0+dt, p0+dp, false, &gtr, &gtt, &gtp, NULL,NULL);
    gtr /= dr; 
    gtt /= dt; 
    gtp /= dp; 
    if(sphcoord){
        gtt /= r0;
        gtp /= (r0*sin(t0));
    }
    norm = sqrt(gtr*gtr + gtt*gtt + gtp*gtp);
    if(norm <= 1e-2) norm = 1e-2;
    real_t limt = limitdist * norm;
    // printf("FMM, limitdist=%f, v=%f\n", limitdist ,norm);

    real_t travt = trilinear_one_ravel(
        rs, nr, ts, nt, ps, np, ntp, TT, r1, t1, p1, false,
        &gtr, &gtt, &gtp, NULL, NULL);
    real_t trem = travt, trem1;
    real_t travt1 = 0.0;

    // normalize gradient
    gtr /= dr; 
    gtt /= dt; 
    gtp /= dp; 
    if(sphcoord){
        gtt /= r1;
        gtp /= (r1*sin(t1));
    }
    norm = sqrt(gtr*gtr + gtt*gtt + gtp*gtp);
    gtr /= norm;
    gtt /= norm;
    gtp /= norm;

    // printf("%f, %f, %f, %f, \n", trem, gtr, gtt, gtp);

    //-------------------------------------------------------------------
    size_t N0 = *N;
    real_t dist;
    while(idot+1 < N0){
        rays[3*idot] = r1;
        rays[3*idot+1] = t1;
        rays[3*idot+2] = p1;

        idot++;

        // if(dist <= limitdist) break;
        if(trem <= limt) break;
        
        // Try up to 5 step lengths along the current unit gradient direction; backtracking only halves the step length
        seglen = seglen0;
        for(size_t i=0; i<5; ++i){
            if(sphcoord){
                p11 = p1 - gtp*seglen/(r1*sin(t1));
                t11 = t1 - gtt*seglen/r1;
                r11 = r1 - gtr*seglen;
            } else {
                r11 = r1 - gtr*seglen;
                t11 = t1 - gtt*seglen;
                p11 = p1 - gtp*seglen;
            }

            // get gradient
            trem1 = trilinear_one_ravel(
                rs, nr, ts, nt, ps, np, ntp, TT, r11, t11, p11, false,
                &gtr_next, &gtt_next, &gtp_next, NULL, NULL);

            // printf("%f, %f, %f, %f, \n", trem1, gtr, gtt, gtp);

            if(trem > trem1) break;

            // compare travt first, 
            // if traveltime field too complex, consider distance.
            if(sphcoord){
                rtp2xyz(r1, t1, p1, &x1, &y1, &z1);
                dx = x1-x0;
                dy = y1-y0;
                dz = z1-z0;
                dist = sqrt(dx*dx + dy*dy + dz*dz);
            } else {
                dist = sqrt(pow(r0-r1,2) + pow(t0-t1,2) + pow(p0-p1,2));
            }

            if(dist <= limitdist) {
                trem1 = 0.0;
                break;
            }

            seglen /= 2.0;
        }
        r1 = r11;
        t1 = t11;
        p1 = p11;
        trem = trem1;
        gtr = gtr_next;
        gtt = gtt_next;
        gtp = gtp_next;

        // Convert and normalize the gradient at the final trial point for the next tracing segment
        gtr /= dr; 
        gtt /= dt; 
        gtp /= dp; 
        if(sphcoord){
            gtt /= r1;
            gtp /= (r1*sin(t1));
        }
        norm = sqrt(gtr*gtr + gtt*gtt + gtp*gtp);
        gtr /= norm;
        gtt /= norm;
        gtp /= norm;

        // 求中点速度，算走时
        // 这里做了近似，认为射线段比较小，不论坐标系为何，直接去平均值作为中点
        if(Slw != NULL){
            rmid = (r1 + rays[3*idot-3])/2.0;
            tmid = (t1 + rays[3*idot-2])/2.0;
            pmid = (p1 + rays[3*idot-1])/2.0;

            // Integrate using the actual endpoint distance, matching the geometry of the output ray
            // In spherical coordinates, use chord lengths for all segments, as for the final segment
            if(sphcoord){
                rtp2xyz(rays[3*idot-3], rays[3*idot-2], rays[3*idot-1], &xx, &yy, &zz);
                rtp2xyz(r1, t1, p1, &x1, &y1, &z1);
                dx = x1 - xx;
                dy = y1 - yy;
                dz = z1 - zz;
            } else {
                dx = r1 - rays[3*idot-3];
                dy = t1 - rays[3*idot-2];
                dz = p1 - rays[3*idot-1];
            }
            real_t segment_length = sqrt(dx*dx + dy*dy + dz*dz);
            travt1 += trilinear_one_ravel(
                rs, nr, ts, nt, ps, np, ntp, Slw, rmid, tmid, pmid, false,
                NULL, NULL, NULL, NULL, NULL) * segment_length;
        }
        
    } // END tracing

    rays[3*idot] = r0;
    rays[3*idot+1] = t0;
    rays[3*idot+2] = p0;

    if(Slw != NULL){
        // compute distance of last two points
        if(sphcoord){
            rtp2xyz(r1, t1, p1, &x1, &y1, &z1);
            dx = x1-x0;
            dy = y1-y0;
            dz = z1-z0;
            dist = sqrt(dx*dx + dy*dy + dz*dz);
        } else {
            dist = sqrt(pow(r0-r1,2) + pow(t0-t1,2) + pow(p0-p1,2));
        }
        rmid = (r0 + r1)/2.0;
        tmid = (t0 + t1)/2.0;
        pmid = (p0 + p1)/2.0;
        travt1 += trilinear_one_ravel(
            rs, nr, ts, nt, ps, np, ntp, Slw, rmid, tmid, pmid, false,
            NULL, NULL, NULL, NULL, NULL) * dist;
    }
   
    idot++;
    *N = idot;

    if(Slw != NULL){
        return travt1;
    } else {
        return travt;
    }
    
}


















/*
        // if(idot==1) {
        //     printf("IXYZ, %d, %d, %d, %d, %d, %d\n", IXYZ[0], IXYZ[1], IXYZ[2], IXYZ[3], IXYZ[4], IXYZ[5]);
        //     printf("WGHT: \n");
        //     for(size_t i=0; i<2; ++i){
        //         for(size_t j=0; j<2; ++j){
        //             for(size_t k=0; k<2; ++k){
        //                 printf("%f ", WGHT[i][j][k]);
        //             }
        //             printf("\n");
        //         }
        //         printf("\n");
        //     }

        //     printf("TT: \n");
        //     for(int i=-1; i<2; ++i){
        //         for(int j=-1; j<2; ++j){
        //             for(int k=-1; k<2; ++k){
        //                 printf("%f ", TT[(148+i)*ntp + (147+j)*np + 149+k]);
        //             }
        //             printf("\n");
        //         }
        //         printf("\n");
        //     }
        //     printf("gtr, gtt, gtp, %f, %f, %f\n", gtr, gtt, gtp);
        // }



        // gtr = trilinear_one_Idx_ravel(IXYZ, WGHT, gTr, nr, nt, np, ntp, NULL, NULL, NULL);
        // gtt = trilinear_one_Idx_ravel(IXYZ, WGHT, gTt, nr, nt, np, ntp, NULL, NULL, NULL);
        // gtp = trilinear_one_Idx_ravel(IXYZ, WGHT, gTp, nr, nt, np, ntp, NULL, NULL, NULL);


        // if(sphcoord){
        //     rtp2xyz(r1, t1, p1, &x1, &y1, &z1);
        //     dx = x1-x0;
        //     dy = y1-y0;
        //     dz = z1-z0;
        //     dist = sqrt(dx*dx + dy*dy + dz*dz);
        // } else {
        //     dist = sqrt(pow(r0-r1,2) + pow(t0-t1,2) + pow(p0-p1,2));
        // }
        // printf("idot=%d, dist=%f\n", idot, dist);
*/
