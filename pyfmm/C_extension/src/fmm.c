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

#include "const.h"
#include "interp.h"
#include "query.h"
#include "coord.h"
#include "mallocfree.h"
#include "diff.h"
#include "heapsort.h"
#include "index.h"
#include "progressbar.h"
#include "fmm.h"



void FastMarching(
    const double *rs, int nr, 
    const double *ts, int nt, 
    const double *ps, int np,
    double rr,  double tt, double pp,
    int maxodr,  const MYREAL *Slw, 
    MYREAL *TT, bool sphcoord, 
    int rfgfac, int rfgn, bool printbar)
{
    // 程序运行开始时间
    struct timeval begin_t;
    gettimeofday(&begin_t, NULL);

    int ntp=nt*np;
    int nrtp=nr*ntp;
    int Ndots=nrtp;

    char *FMM_stat = (char *)malloc1d(nrtp, sizeof(char)); // 1 alive, 0 close, -1 far

    int heapsize=0, heapcapcity=nr*nt + nt*np + nr*np;
    int *psize, *pcap;
    psize = &heapsize;
    pcap = &heapcapcity;
    HEAP_DATA *FMM_data = (HEAP_DATA *)malloc1d(heapcapcity, sizeof(HEAP_DATA));
    int *NroIdx = (int *)malloc1d(nrtp, sizeof(int));

    // All non-zero value of TT will be treated as efficient value,
    // and set FMM_CLS
    bool allzeroTT = true; 
    for(int i=0; i<nrtp; ++i){
        if(TT[i] == 0.0){
            TT[i] = 9.9e30f;// init FAR Traveltime 
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
    const double *rs, int nr, 
    const double *ts, int nt, 
    const double *ps, int np,
    int maxodr,  const MYREAL *Slw, MYREAL *TT, 
    char *FMM_stat, bool sphcoord, bool *edgeStop, bool printbar,
    HEAP_DATA *FMM_data, int *psize, int *pcap, int *NroIdx, int *pNdots)
{
    double dr = (nr>1)? rs[1] - rs[0] : 0.0;
    double dt = (nt>1)? ts[1] - ts[0] : 0.0;
    double dp = (np>1)? ps[1] - ps[0] : 0.0;

    // DON'T CHANGE.
    static const char xr[6] = {-1, 1,  0, 0,  0, 0};
    static const char xt[6] = { 0, 0, -1, 1,  0, 0};
    static const char xp[6] = { 0, 0,  0, 0, -1, 1};

    // convenient arrays
    double sin_ts[nt];
    if(sphcoord){
        for(int it=0; it<nt; ++it){
            sin_ts[it] = fabs(sin(ts[it]));
            if(sin_ts[it] < 1e-12) sin_ts[it] += 1e-12;
        }
    }

    int ntp=nt*np;

    HEAP_DATA popdata, newdata;
    int ir0, it0, ip0, ir, it, ip;
    int idx, idx0;
    MYREAL s;
    MYREAL travt_bak, travt, travt0, travt1;
    double h;

    MYREAL *pt;
    char *pstat;

    // 打印进度条时每隔print_interv打印一次
    int size_bak = nr*ntp;
    int last_barpercent = 0, barpercent;

    // printf("loop start, size=%d\n", *psize );
    char travt_stat;
    MYREAL maxtravt=-999;
    while(*psize > 0){

        // get the minimum one 
        popdata = HeapPop(FMM_data, psize, NroIdx, TT);
        idx0 = popdata;
        FMM_stat[idx0] = FMM_ALV;

        unravel_index(idx0, ntp, np, &ir0, &it0, &ip0);
        travt0 = TT[idx0];

        // break loop in advance when reach the boundary
        if( edgeStop!=NULL && (
            edgeStop[0]&&ir0==0 || edgeStop[1]&&ir0==nr-1 || 
            edgeStop[2]&&it0==0 || edgeStop[3]&&it0==nt-1 || 
            edgeStop[4]&&ip0==0 || edgeStop[5]&&ip0==np-1))  break;

        

        if(travt0 > maxtravt) maxtravt = travt0;
        else if(travt0 < maxtravt){
            // 当在源点附近使用加密网格时，由于需要使用一般方法初始化源点附近的走时，
            printf("pNdots=%d, WRONG! travt0(%f) < maxtravt(%f) \n", *pNdots, travt0, maxtravt);
            print_HEAP(FMM_data, *psize, nr, nt, np, NroIdx, TT, NULL, NULL, NULL);
            printf("Tiny bug here, please let author know.\n");
        } 

        // get neighbours (max 6)
        for(char k=0; k<6; ++k){
            
            ir = ir0 + xr[k];
            it = it0 + xt[k];
            ip = ip0 + xp[k];

            
            if(ir<0 || ir>nr-1) continue;
            if(it<0 || it>nt-1) continue;
            if(ip<0 || ip>np-1) continue;
            
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
        barpercent = 100 - (*pNdots*100) / size_bak;
        if(printbar && barpercent != last_barpercent){
            printprogressBar("Fast Marching...  ", barpercent);
            last_barpercent = barpercent;
        }

    }


    return FMM_data;
}





HEAP_DATA * init_source_TT(
    const double *rs, int nr, 
    const double *ts, int nt, 
    const double *ps, int np,
    double rr, double tt, double pp,
    const MYREAL *Slw, MYREAL *TT, 
    char *FMM_stat, bool sphcoord,
    HEAP_DATA *FMM_data, int *psize, int *pcap, int *NroIdx, int *pNdots)
{   
    int ir, it, ip;
    ir = dicho_find(rs, nr, rr);
    it = dicho_find(ts, nt, tt);
    ip = dicho_find(ps, np, pp);
    if(ir==nr-1 && ir>0)    ir--;
    if(it==nt-1 && it>0)    it--;
    if(ip==np-1 && ip>0)    ip--;
    

    double xx, yy, zz;
    if(sphcoord) rtp2xyz(rr, tt, pp, &xx, &yy, &zz);
    

    HEAP_DATA newdata;
    MYREAL travt, s;
    int jr, jt, jp;
    int jdx;
    double dist;
    double r2, t2, p2, dr, dt, dp;
    double x2, y2, z2;
    double dx, dy, dz;
    
    MYREAL mtravt=9.9e30;
    int mir, mit, mip, midx=0;  // 最小走时节点的索引

    // Tiny 2x2x2 cube
    for(char kr=0; kr<2; ++kr){
        jr = ir+kr;
        if(jr > nr-1) continue;
        
        r2 = rs[jr];
        dr = r2 - rr;
        for(char kt=0; kt<2; ++kt){
            jt = it+kt;
            if(jt > nt-1) continue;

            t2 = ts[jt];
            dt = t2 - tt;
            for(char kp=0; kp<2; ++kp){
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
    for(char kr=-1; kr<2; ++kr){
        jr = mir+kr;
        if(jr<0 || jr>nr-1) continue;

        r2 = rs[jr];
        dr = r2 - rr;
        for(char kt=-1; kt<2; ++kt){
            jt = mit+kt;
            if(jt<0 || jt>nt-1) continue;

            t2 = ts[jt];
            dt = t2 - tt;
            for(char kp=-1; kp<2; ++kp){
                jp = mip+kp;
                if(jp<0 || jp>np-1) continue;
                
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
    const double *rs, int nr, 
    const double *ts, int nt, 
    const double *ps, int np,
    double rr, double tt, double pp, 
    int maxodr,  const MYREAL *Slw, MYREAL *TT, 
    char *FMM_stat, bool sphcoord,
    int rfgfac, int rfgn, // refine grid factor and number of grids
    bool printbar,
    HEAP_DATA *FMM_data, int *psize, int *pcap, int *NroIdx, int *pNdots)
{   
    double dr = (nr>1)? rs[1] - rs[0] : 0.0;
    double dt = (nt>1)? ts[1] - ts[0] : 0.0;
    double dp = (np>1)? ps[1] - ps[0] : 0.0;

    int ntp=nt*np;

    // find the closest point
    int ir, it, ip;
    ir = dicho_find(rs, nr, rr);
    it = dicho_find(ts, nt, tt);
    ip = dicho_find(ps, np, pp);
    if(ir<nr-1 && fabs(rs[ir+1]-rr) < fabs(rs[ir]-rr)) ir++;
    if(it<nt-1 && fabs(ts[it+1]-tt) < fabs(ts[it]-tt)) it++;
    if(ip<np-1 && fabs(ps[ip+1]-pp) < fabs(ps[ip]-pp)) ip++;

    double rfg_dr, rfg_dt, rfg_dp;
    rfg_dr = dr/rfgfac;
    rfg_dt = dt/rfgfac;
    rfg_dp = dp/rfgfac;
    
    int rfg_ir1, rfg_ir2;
    int rfg_it1, rfg_it2;
    int rfg_ip1, rfg_ip2;
    rfg_ir1 = rfg_ir2 = ir;
    rfg_it1 = rfg_it2 = it;
    rfg_ip1 = rfg_ip2 = ip;

    // 确定加密范围
    for(int i=1; i<=rfgn; ++i){
        if(rfg_ir1>0)    rfg_ir1--;
        if(rfg_ir2<nr-1) rfg_ir2++;
        if(rfg_it1>0)    rfg_it1--;
        if(rfg_it2<nt-1) rfg_it2++;
        if(rfg_ip1>0)    rfg_ip1--;
        if(rfg_ip2<np-1) rfg_ip2++;
    }
    // printf("rfg_ir12: %d %d, rfg_it12: %d %d, rfg_ip12: %d %d\n ", 
    //         rfg_ir1, rfg_ir2, rfg_it1, rfg_it2, rfg_ip1, rfg_ip2);

    int rfg_nr, rfg_nt, rfg_np, rfg_nrtp, rfg_ntp;
    int rfg_Ndots;
    rfg_nr = (rfg_ir2 - rfg_ir1)*rfgfac + 1;
    rfg_nt = (rfg_it2 - rfg_it1)*rfgfac + 1;
    rfg_np = (rfg_ip2 - rfg_ip1)*rfgfac + 1;
    rfg_ntp = rfg_nt*rfg_np;
    rfg_nrtp = rfg_nr*rfg_ntp;
    rfg_Ndots = rfg_nrtp;

    double *rfg_rs = (double *)malloc1d(rfg_nr, sizeof(double));
    double *rfg_ts = (double *)malloc1d(rfg_nt, sizeof(double));
    double *rfg_ps = (double *)malloc1d(rfg_np, sizeof(double));
    
    for(int i=0; i<rfg_nr; ++i){
        rfg_rs[i] = rs[rfg_ir1] + rfg_dr*i;
    }
    for(int i=0; i<rfg_nt; ++i){
        rfg_ts[i] = ts[rfg_it1] + rfg_dt*i;
    }
    for(int i=0; i<rfg_np; ++i){
        rfg_ps[i] = ps[rfg_ip1] + rfg_dp*i;
    }

    MYREAL *rfg_TT = (MYREAL *)malloc1d(rfg_nrtp, sizeof(MYREAL));
    MYREAL *rfg_Slw = (MYREAL *)malloc1d(rfg_nrtp, sizeof(MYREAL));
    char *rfg_FMM_stat = (char *)malloc1d(rfg_nrtp, sizeof(char)); // 1 alive, 0 close, -1 far
    
    // 插值加密的慢度场
    for(int i=0; i<rfg_nrtp; ++i){
        rfg_TT[i] = 9.9e30f;// init FAR Traveltime 
        rfg_FMM_stat[i] = FMM_FAR;

        int ir0, it0, ip0;
        unravel_index(i, rfg_ntp, rfg_np, &ir0, &it0, &ip0);
        rfg_Slw[i] = trilinear_one_ravel(
            rs, nr, 
            ts, nt, 
            ps, np, ntp, Slw, 
            rfg_rs[ir0], rfg_ts[it0], rfg_ps[ip0],
            NULL, NULL, NULL, NULL, NULL);
    }

    int rfg_heapsize=0, rfg_heapcapcity=rfg_nr*rfg_nt + rfg_nt*rfg_np + rfg_nr*rfg_np;
    int *prfg_size, *prfg_cap;
    prfg_size = &rfg_heapsize;
    prfg_cap = &rfg_heapcapcity;
    HEAP_DATA *rfg_FMM_data = (HEAP_DATA *)malloc1d(rfg_heapcapcity, sizeof(HEAP_DATA));
    int *rfg_NroIdx = (int *)malloc1d(rfg_nrtp, sizeof(int));

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
    for(int jr=rfg_ir1, rfg_jr=0; jr<=rfg_ir2; ++jr, rfg_jr+=rfgfac){
    for(int jt=rfg_it1, rfg_jt=0; jt<=rfg_it2; ++jt, rfg_jt+=rfgfac){
        int jdx1, jdx2;
        jdx1 = jdx2 = -1;
        for(int jp=rfg_ip1, rfg_jp=0; jp<=rfg_ip2; ++jp, rfg_jp+=rfgfac){
            int jdx, rfg_jdx;
            ravel_index(&jdx, ntp, np, jr, jt, jp);
            ravel_index(&rfg_jdx, rfg_ntp, rfg_np, rfg_jr, rfg_jt, rfg_jp);

            if(rfg_FMM_stat[rfg_jdx] == FMM_FAR) continue;

            // 将最外侧的节点加入堆
            if(jdx1<0) jdx1 = jdx;
            if(jdx1>=0) jdx2 = jdx;

            TT[jdx] = rfg_TT[rfg_jdx];
            // printf("rfg_TT = %f, jr, jt, jp %d, %d, %d\n", rfg_TT[rfg_jdx], jr, jt, jp);
            FMM_stat[jdx] = FMM_ALV;
            if(pNdots!=NULL) (*pNdots)--;
        }
        if(jdx1>=0){
            FMM_data = HeapPush(FMM_data, psize, pcap, jdx1, NroIdx, TT);
            FMM_stat[jdx1] = FMM_CLS;
        }
        if(jdx2>=0 && jdx2!=jdx1){
            FMM_data = HeapPush(FMM_data, psize, pcap, jdx2, NroIdx, TT);
            FMM_stat[jdx2] = FMM_CLS;
        }

    }}

    free1d(rfg_TT);
    free1d(rfg_Slw);
    free1d(rfg_rs);
    free1d(rfg_ts);
    free1d(rfg_ps);

    free1d(rfg_FMM_data);
    free1d(rfg_NroIdx);


    return FMM_data;
}





MYREAL get_neighbour_travt(
    int nr, int nt, int np, int ntp,
    int ir, int it, int ip, int idx,
    int maxodr, MYREAL *TT,
    char *FMM_stat,  double s,
    double dr, double dt, double dp, 
    char *stat)
{   
    if(stat!=NULL) *stat = 0;

    double Acoef, Bcoef, Ccoef;
    Acoef = Bcoef = 0.0;
    Ccoef = - s*s;

    MYREAL tarr[5], tarrR[5], tarrT[5], tarrP[5]; // max(maxodr) = 3
    tarr[0] = tarrR[0] = tarrT[0] = tarrP[0] = TT[idx];
    int odr, odrR, odrT, odrP;
    odr = odrR = odrT = odrP = 0;

    int jdx;
    int i;

    char sgn_r, sgn_t, sgn_p;
    sgn_r = sgn_t = sgn_p = 0;
    double dif, pos_dif, neg_dif;
    double acoef, bcoef;
    double pos_acoef, neg_acoef, pos_bcoef, neg_bcoef;
    //------------------------------------------- R ---------------------------------------
    // --------------------------------------- negative -----------------------------------
    for(odr=0; odr<maxodr; ++odr){
        if(ir-odr<1) break;
        jdx = idx - (odr+1)*ntp;
        if(FMM_stat[jdx]!=FMM_ALV) break;
        tarr[odr+1] = TT[jdx];
        if(tarr[odr+1] >= TT[jdx+ntp]) break;
    }
    get_diff_odr123(odr, tarr, dr, &neg_acoef, &neg_bcoef, &neg_dif);
    // --------------------------------------- positive -----------------------------------
    for(odrR=0; odrR<maxodr; ++odrR){
        if(ir+odrR+1>nr-1) break;
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
        if(it-odr<1) break;
        jdx = idx - (odr+1)*np;
        if(FMM_stat[jdx]!=FMM_ALV) break;
        tarr[odr+1] = TT[jdx];
        if(tarr[odr+1] >= TT[jdx+np]) break;
    }
    get_diff_odr123(odr, tarr, dt, &neg_acoef, &neg_bcoef, &neg_dif);
    // --------------------------------------- positive -----------------------------------
    for(odrT=0; odrT<maxodr; ++odrT){
        if(it+odrT+1>nt-1) break;
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
        if(ip-odr<1) break;
        jdx = idx - (odr+1);
        if(FMM_stat[jdx]!=FMM_ALV) break;
        tarr[odr+1] = TT[jdx];
        if(tarr[odr+1] >= TT[jdx+1]) break;
    }
    get_diff_odr123(odr, tarr, dp, &neg_acoef, &neg_bcoef, &neg_dif);
    // --------------------------------------- positive -----------------------------------
    for(odrP=0; odrP<maxodr; ++odrP){
        if(ip+odrP+1>np-1) break;
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
    double jdg = Bcoef*Bcoef - 4*Acoef*Ccoef;
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





MYREAL FMM_raytracing(
    const double *rs, int nr, 
    const double *ts, int nt, 
    const double *ps, int np,
    double r0, double t0, double p0,
    double rr, double tt, double pp, double seglen, double segfac,
    const MYREAL *TT, bool sphcoord,
    // MYREAL *gTr, MYREAL *gTt, MYREAL *gTp, 
    double *rays, int *N)
{
    double dr = (nr>1)? rs[1] - rs[0] : 1e-6;
    double dt = (nt>1)? ts[1] - ts[0] : 1e-6;
    double dp = (np>1)? ps[1] - ps[0] : 1e-6;
    double seglen0 = seglen;

    double xx, yy, zz, x0, y0, z0;
    double dx, dy, dz;
    if(sphcoord) {
        rtp2xyz(r0, t0, p0, &x0, &y0, &z0);
        rtp2xyz(rr, tt, pp, &xx, &yy, &zz);
    }

    
    int ntp = nt*np;
    
    int idot = 0;

    double gtr, gtt, gtp, norm;
    double limitdist;  
    if(sphcoord){
        limitdist = sqrt(dr*dr + pow(dt*rs[0],2) + pow(dp*rs[0]*sin(ts[0]),2));
    } else {
        limitdist = sqrt(dr*dr + dt*dt + dp*dp);
    }
    if(limitdist < segfac*seglen) limitdist = segfac*seglen;

    double r1, t1, p1;
    double x1, y1, z1;
    double r11, t11, p11;
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
    trilinear_one_ravel(rs, nr, ts, nt, ps, np, ntp, TT, r0+dr, t0+dt, p0+dp, &gtr, &gtt, &gtp, NULL,NULL);
    gtr /= dr; 
    gtt /= dt; 
    gtp /= dp; 
    if(sphcoord){
        gtt /= r0;
        gtp /= (r0*sin(t0));
    }
    norm = sqrt(gtr*gtr + gtt*gtt + gtp*gtp);
    if(norm <= 1e-2) norm = 1e-2;
    MYREAL limt = limitdist * norm;
    // printf("FMM, limitdist=%f, v=%f\n", limitdist ,norm);

    MYREAL travt = trilinear_one_ravel(
        rs, nr, ts, nt, ps, np, ntp, TT, r1, t1, p1, 
        &gtr, &gtt, &gtp, NULL, NULL);
    MYREAL trem = travt, trem1;

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
    int N0 = *N;
    double dist;
    while(idot < N0-1){
        rays[3*idot] = r1;
        rays[3*idot+1] = t1;
        rays[3*idot+2] = p1;

        idot++;

        // if(dist <= limitdist) break;
        if(trem <= limt) break;
        
        // update
        seglen = seglen0;
        for(int i=0; i<5; ++i){
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
                rs, nr, ts, nt, ps, np, ntp, TT, r11, t11, p11, 
                &gtr, &gtt, &gtp, NULL, NULL);

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

    } // END tracing

    rays[3*idot] = r0;
    rays[3*idot+1] = t0;
    rays[3*idot+2] = p0;
   
    idot++;
    *N = idot;

    return travt;
}


















/*
        // if(idot==1) {
        //     printf("IXYZ, %d, %d, %d, %d, %d, %d\n", IXYZ[0], IXYZ[1], IXYZ[2], IXYZ[3], IXYZ[4], IXYZ[5]);
        //     printf("WGHT: \n");
        //     for(int i=0; i<2; ++i){
        //         for(int j=0; j<2; ++j){
        //             for(int k=0; k<2; ++k){
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