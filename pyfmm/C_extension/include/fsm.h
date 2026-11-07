/**
 * @file   fsm.h
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-05
 * 
*/
#pragma once

#include "const.h"
#include "heapsort.h"


/**
 * 定义OpenMP多线程数
 * 
 * @param     num_threads     (in)线程数
 */
void set_fsm_num_threads(size_t num_threads);


/**
 * 使用Fast Sweeping Method计算全局走时场
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
 * @param     eps        (in)Sweep后的最大更新量达到收敛条件
 * @param     maxLoops   (in)Fast Sweeping Method整体迭代次数（对于3D模型，向8个方向各Sweep一次为迭代一次）  
 * @param     isparallel (in)是否使用并行FSM
 * 
 * @return    nsweep, sweep次数
 * 
 */
size_t FastSweeping(
    const real_t *rs, size_t nr,
    const real_t *ts, size_t nt,
    const real_t *ps, size_t np,
    real_t rr,  real_t tt, real_t pp,
    size_t maxodr,  const real_t *Slw,
    real_t *TT, bool sphcoord,
    size_t rfgfac, size_t rfgn, bool printbar,
    real_t eps, size_t maxLoops, bool isparallel);


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
 * @param     printbar  (in)是否打印进度条
 * @param     eps        (in)Sweep后的最大更新量达到收敛条件
 * @param     maxLoops   (in)Fast Sweeping Method整体迭代次数（对于3D模型，向8个方向各Sweep一次为迭代一次）  
 * @param     isparallel (in)是否使用并行FSM
 * 
 * @return    nsweep, sweep次数
 */
size_t FastSweeping_with_initial(
    const real_t *rs, size_t nr,
    const real_t *ts, size_t nt,
    const real_t *ps, size_t np,
    size_t maxodr,  const real_t *Slw, real_t *TT,
    char *FMM_stat, bool sphcoord, bool printbar, 
    real_t eps, size_t maxLoops, bool isparallel);

