/**
 * @file   fmm.h
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
*/

#pragma once

#include "fmm/const.h"
#include "fmm/heapsort.h"

#define _PRINT_ODR_BUG_ 0

#define FMM_FAR -1   ///< 波前还未触及的区域
#define FMM_CLS 0    ///< 波前面
#define FMM_ALV 1    ///< 波前已完全扫过的区域，走时已确定



/**
 * 使用Fast Marching Method计算全局走时场
 * 
 * @param     rs     (in)维度1坐标数组
 * @param     nr     (in)rs长度
 * @param     ts     (in)维度2坐标数组
 * @param     nt     (in)ts长度
 * @param     ps     (in)维度2坐标数组
 * @param     np     (in)ps长度
 * @param     rr     (in)源点维度1坐标
 * @param     tt     (in)源点维度2坐标
 * @param     pp     (in)源点维度3坐标
 * @param     maxodr (in)使用的最大差分阶数
 * @param     Slw    (in)展平的三维慢度场
 * @param     TT     (inout)展平的三维走时场，如果初始值有非零值，会被直接加入堆中，此时源点不再使用
 * @param     sphcoord  (in)是否使用球坐标
 * @param     rfgfac    (in)对于源点附近的格点间加密倍数，>1
 * @param     rfgn      (in)对于源点附近的格点间加密处理的辐射半径，>=1
 * @param     printbar  (in)是否打印进度条
 * 
 * 
 */
void FastMarching(
    const real_t *rs, size_t nr,
    const real_t *ts, size_t nt,
    const real_t *ps, size_t np,
    real_t rr,  real_t tt, real_t pp,
    size_t maxodr,  const real_t *Slw,
    real_t *TT, bool sphcoord,
    size_t rfgfac, size_t rfgn, bool printbar);


/**
 * 在有初始走时的情况下使用Fast Marching Method计算全局走时场
 * 
 * @param     rs     (in)维度1坐标数组
 * @param     nr     (in)rs长度
 * @param     ts     (in)维度2坐标数组
 * @param     nt     (in)ts长度
 * @param     ps     (in)维度2坐标数组
 * @param     np     (in)ps长度
 * @param     maxodr (in)使用的最大差分阶数
 * @param     Slw    (in)展平的三维慢度场
 * @param     TT     (inout)展平的三维走时场
 * @param     FMM_stat  (out)记录每个节点的状态(alive, close, far)
 * @param     sphcoord  (in)是否使用球坐标
 * @param     edgeStop  (in)是否在波前传播到6个边界面时提前结束计算
 * @param     printbar  (in)是否打印进度条
 * @param     FMM_data  (inout)堆首指针
 * @param     psize     (inout)堆大小，会被调整大小
 * @param     pcap      (inout)堆最大容量，视情况会被调整大小
 * @param     NroIdx    (out)一维指针，用于在节点索引位置处填上堆中的索引值
 * @param     pNdots    (inout)记录还剩下多少节点的走时未计算
 * 
 */
HEAP_DATA * FastMarching_with_initial(
    const real_t *rs, size_t nr,
    const real_t *ts, size_t nt,
    const real_t *ps, size_t np,
    size_t maxodr,  const real_t *Slw, real_t *TT,
    char *FMM_stat, bool sphcoord, bool *edgeStop, bool printbar,
    HEAP_DATA *FMM_data, size_t *psize, size_t *pcap, size_t *NroIdx, size_t *pNdots);



/**
 * 计算源点附近的走时
 * 
 * @param     rs     (in)维度1坐标数组
 * @param     nr     (in)rs长度
 * @param     ts     (in)维度2坐标数组
 * @param     nt     (in)ts长度
 * @param     ps     (in)维度2坐标数组
 * @param     np     (in)ps长度
 * @param     rr     (in)源点维度1坐标
 * @param     tt     (in)源点维度2坐标
 * @param     pp     (in)源点维度3坐标
 * @param     Slw    (in)展平的三维慢度场
 * @param     TT     (inout)展平的三维走时场
 * @param     FMM_stat  (out)记录每个节点的状态(alive, close, far)
 * @param     sphcoord  (in)是否使用球坐标
 * @param     FMM_data  (inout)堆首指针
 * @param     psize     (inout)堆大小，会被调整大小
 * @param     pcap      (inout)堆最大容量，视情况会被调整大小
 * @param     NroIdx    (out)一维指针，用于在节点索引位置处填上堆中的索引值
 * @param     pNdots    (inout)记录还剩下多少节点的走时未计算
 * 
 * 
 * @return    堆首指针
 */
HEAP_DATA * init_source_TT(
    const real_t *rs, size_t nr,
    const real_t *ts, size_t nt,
    const real_t *ps, size_t np,
    real_t rr, real_t tt, real_t pp,
    const real_t *Slw, real_t *TT,
    char *FMM_stat, bool sphcoord,
    HEAP_DATA *FMM_data, size_t *psize, size_t *pcap, size_t *NroIdx, size_t *pNdots);



/**
 * 以加密网格的方式计算源点附近的走时
 * 
 * @param     rs     (in)维度1坐标数组
 * @param     nr     (in)rs长度
 * @param     ts     (in)维度2坐标数组
 * @param     nt     (in)ts长度
 * @param     ps     (in)维度2坐标数组
 * @param     np     (in)ps长度
 * @param     rr     (in)源点维度1坐标
 * @param     tt     (in)源点维度2坐标
 * @param     pp     (in)源点维度3坐标
 * @param     maxodr (in)使用的最大差分阶数
 * @param     Slw    (in)展平的三维慢度场
 * @param     TT     (inout)展平的三维走时场
 * @param     FMM_stat  (out)记录每个节点的状态(alive, close, far)
 * @param     sphcoord  (in)是否使用球坐标
 * @param     rfgfac    (in)对于源点附近的格点间加密倍数，>1
 * @param     rfgn      (in)对于源点附近的格点间加密处理的辐射半径，>=1
 * @param     printbar  (in)是否打印进度条
 * @param     FMM_data  (inout)堆首指针
 * @param     psize     (inout)堆大小，会被调整大小
 * @param     pcap      (inout)堆最大容量，视情况会被调整大小
 * @param     NroIdx    (out)一维指针，用于在节点索引位置处填上堆中的索引值
 * @param     pNdots    (inout)记录还剩下多少节点的走时未计算
 * 
 * @return    堆首指针
 */
HEAP_DATA * init_source_TT_refinegrid(
    const real_t *rs, size_t nr,
    const real_t *ts, size_t nt,
    const real_t *ps, size_t np,
    real_t rr, real_t tt, real_t pp,
    size_t maxodr,  const real_t *Slw, real_t *TT,
    char *FMM_stat, bool sphcoord,
    size_t rfgfac, size_t rfgn, // refine grid factor and number of grids
    bool printbar,
    HEAP_DATA *FMM_data, size_t *psize, size_t *pcap, size_t *NroIdx, size_t *pNdots);



/**
 * 依据邻近的节点走时，以解一元二次方程的形式求解某点的走时
 * 
 * @param      nr      (in)维度1长度
 * @param      nt      (in)维度2长度
 * @param      np      (in)维度3长度
 * @param      ntp     (in)nt*np
 * @param      ir      (in)某点的维度1索引
 * @param      it      (in)某点的维度2索引
 * @param      ip      (in)某点的维度3索引
 * @param      idx     (in)某点的三维展开索引
 * @param      maxodr  (in)使用的最大差分阶数
 * @param      TT      (inout)展平的三维走时场
 * @param      FMM_stat  (out)记录每个节点的状态(alive, close, far)
 * @param      s         (in)某点的慢度
 * @param      dr        (in)维度1坐标间隔
 * @param      dt        (in)维度2坐标间隔
 * @param      dp        (in)维度3坐标间隔
 * @param      stat      (out)求解情况，-1表示求解出现问题，0为正常求解
 * 
 * @return     走时结果
 * 
 */
real_t get_neighbour_travt(
    size_t nr, size_t nt, size_t np, size_t ntp,
    size_t ir, size_t it, size_t ip, size_t idx,
    size_t maxodr, real_t *TT,
    char *FMM_stat,  real_t s,
    real_t dr, real_t dt, real_t dp,
    char *stat);



/**
 * 根据梯度下降，从走时场中提取初至射线
 * 
 * @param     rs     (in)维度1坐标数组
 * @param     nr     (in)rs长度
 * @param     ts     (in)维度2坐标数组
 * @param     nt     (in)ts长度
 * @param     ps     (in)维度2坐标数组
 * @param     np     (in)ps长度
 * @param     r0     (in)源点维度1坐标
 * @param     t0     (in)源点维度2坐标
 * @param     p0     (in)源点维度3坐标
 * @param     rr     (in)接收点维度1坐标
 * @param     tt     (in)接收点维度2坐标
 * @param     pp     (in)接收点维度3坐标
 * @param     seglen (in)名义追踪步长，走时积分使用实际端点距离
 * @param     segfac (in)t < segfac*seglen/v，当射线追踪到在源点附近时，射线直接连接源点
 * @param     Slw    (in)展平的三维慢度场，若非NULL则按实际线段长度作中点积分，球坐标使用三维弦长，否则从走时场插值
 * @param     TT     (in)展平的三维走时场
 * @param     sphcoord  (in)是否使用球坐标
 * @param     rays      (out)输出展平的三维射线
 * @param     N         (inout)输入射线缓冲区容量，输出实际射线点数
 * 
 * @return    射线走时
 * 
 */
real_t FMM_raytracing(
    const real_t *rs, size_t nr,
    const real_t *ts, size_t nt,
    const real_t *ps, size_t np,
    real_t r0, real_t t0, real_t p0,
    real_t rr, real_t tt, real_t pp, real_t seglen, real_t segfac,
    const real_t *Slw, const real_t *TT, bool sphcoord,
    real_t *rays, size_t *N);
