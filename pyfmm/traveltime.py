"""
    :file:     traveltime.py  
    :author:   Zhu Dengda (zhudengda@mail.iggcas.ac.cn)  
    :date:     2023-04

"""


import os 
import numpy as np
import numpy.ctypeslib as npct
from ctypes import byref, c_int
from scipy import interpolate

from . import c_interfaces

FSM_nsweep = 0

def get_FSM_nsweep():
    r'''
        返回使用FSM计算走时场时，sweep的次数
    '''
    global FSM_nsweep 
    return FSM_nsweep

def travel_time_source(
    srcloc:list, 
    xarr:np.ndarray, yarr:np.ndarray, zarr:np.ndarray, slw:np.ndarray,
    maxodr:int=2, sphcoord:bool=False, rfgfac:int=0, rfgn:int=0, printbar:bool=False,
    useFSM:bool=False, FSMeps:float=0.0, FSMmaxLoops:int=1, FSMparallel:bool=False):
    r'''
        给定源点坐标，计算全局走时场

        .. note::  最大差分阶数maxodr不建议取3，会有数值不稳定导致结果偏差的情况。默认取2。

        .. warning::  源点附近加密网格的方法不稳定，效果时好时坏，不建议使用。

        :param     srcloc:    源点坐标，直角坐标系 :math:`(x,y,z)` 或球坐标系 :math:`(r,\theta,\phi)` 
        :param       xarr:    :math:`x` 或 :math:`r` 节点坐标数组，要求等距升序排列 
        :param       yarr:    :math:`y` 或 :math:`\theta` 节点坐标数组，要求等距升序排列 
        :param       zarr:    :math:`z` 或 :math:`\phi` 节点坐标数组，要求等距升序排列 
        :param        slw:    形状为(nx, ny, nz)的三维慢度场
        :param     maxodr:    使用的最大差分阶数, 1 or 2 or 3
        :param   sphcoord:    是否为球坐标系
        :param     rfgfac:    对于源点附近的格点间加密倍数，>1
        :param       rfgn:    对于源点附近的格点间加密处理的辐射半径，>=1
        :param   printbar:    是否打印进度条
        :param     useFSM:    是否改用Fast Sweeping Method计算全局走时场
        :param     FSMeps:    Fast Sweeping Method收敛条件，衡量Sweep后的最大更新量
        :param  FSMmaxLoops:  Fast Sweeping Method整体迭代次数（对于3D模型，向8个方向各Sweep一次为迭代一次）  
        :param  FSMparallel:  是否使用并行Fast Sweeping Method

        :return:   三维走时场
    '''
    global FSM_nsweep 

    check_xyz_arr(xarr, yarr, zarr, sphcoord)

    # 对于并行情况，至少迭代两次
    if FSMparallel:
        if FSMmaxLoops <= 1:
            print("WARNING! For parallel FSM, maxLoops must set at least 2 (already changed).")
            FSMmaxLoops = 2

    maxodr = int(maxodr)
    rfgfac = int(rfgfac)
    rfgn = int(rfgn)

    xx, yy, zz = np.array(srcloc).astype('f8')

    if xx < xarr[0] or xx > xarr[-1]:
        raise ValueError("xx out of bound.")
    if yy < yarr[0] or yy > yarr[-1]:
        raise ValueError("yy out of bound.")
    if zz < zarr[0] or zz > zarr[-1]:
        raise ValueError("zz out of bound.")

    c_xarr = npct.as_ctypes(xarr.astype('f8'))
    c_yarr = npct.as_ctypes(yarr.astype('f8'))
    c_zarr = npct.as_ctypes(zarr.astype('f8'))
    slw_ravel = slw.ravel().astype(c_interfaces.NPCT_REAL_TYPE)
    c_slw = npct.as_ctypes(slw_ravel)

    TT = np.zeros_like(slw).astype(c_interfaces.NPCT_REAL_TYPE)
    TT_ravel = TT.ravel()
    c_TT = npct.as_ctypes(TT_ravel)

    FastFunc = c_interfaces.C_FastMarching if not useFSM else c_interfaces.C_FastSweeping
    parse_args = [
        c_xarr, len(xarr),
        c_yarr, len(yarr),
        c_zarr, len(zarr),
        xx, yy, zz,
        maxodr, c_slw, 
        c_TT, sphcoord,
        rfgfac, rfgn, printbar
    ]
    if useFSM:
        parse_args.extend([FSMeps, FSMmaxLoops, FSMparallel])

    FSM_nsweep = FastFunc(*parse_args)

    return TT


def travel_time_iniTT(
    iniTT:np.ndarray,
    xarr:np.ndarray, yarr:np.ndarray, zarr:np.ndarray, slw:np.ndarray,
    maxodr:int=2, sphcoord:bool=False, printbar:bool=False,
    useFSM:bool=False, FSMeps:float=0.0, FSMmaxLoops:int=1, FSMparallel:bool=False):
    r'''
        给定走时场初始状态，计算全局走时场

        .. note::  最大差分阶数maxodr不建议取3，会有数值不稳定导致结果偏差的情况。默认取2。

        :param      iniTT:    走时场初始状态
        :param       xarr:    :math:`x` 或 :math:`r` 节点坐标数组，要求等距升序排列 
        :param       yarr:    :math:`y` 或 :math:`\theta` 节点坐标数组，要求等距升序排列 
        :param       yarr:    :math:`z` 或 :math:`\phi` 节点坐标数组，要求等距升序排列 
        :param        slw:    形状为(nx, ny, nz)的三维慢度场
        :param     maxodr:    使用的最大差分阶数, 1 or 2 or 3
        :param   sphcoord:    是否为球坐标系
        :param   printbar:    是否打印进度条 
        :param     useFSM:    是否改用Fast Sweeping Method计算全局走时场
        :param     FSMeps:    Fast Sweeping Method收敛条件，衡量Sweep后的最大更新量
        :param  FSMmaxLoops:  Fast Sweeping Method整体迭代次数（对于3D模型，向8个方向各Sweep一次为迭代一次）  
        :param  FSMparallel:  是否使用并行Fast Sweeping Method

        :return:   三维走时场
    '''
    global FSM_nsweep

    check_xyz_arr(xarr, yarr, zarr, sphcoord)

    # 对于并行情况，至少迭代两次
    if FSMparallel:
        if FSMmaxLoops <= 1:
            print("WARNING! For parallel FSM, maxLoops must set at least 2 (already changed).")
            FSMmaxLoops = 2

    maxodr = int(maxodr)

    c_xarr = npct.as_ctypes(xarr.astype('f8'))
    c_yarr = npct.as_ctypes(yarr.astype('f8'))
    c_zarr = npct.as_ctypes(zarr.astype('f8'))
    slw_ravel = slw.ravel().astype(c_interfaces.NPCT_REAL_TYPE)
    c_slw = npct.as_ctypes(slw_ravel)

    TT_ravel = iniTT.ravel().astype(c_interfaces.NPCT_REAL_TYPE)
    c_TT = npct.as_ctypes(TT_ravel)

    FastFunc = c_interfaces.C_FastMarching if not useFSM else c_interfaces.C_FastSweeping
    parse_args = [
        c_xarr, len(xarr),
        c_yarr, len(yarr),
        c_zarr, len(zarr),
        0.0, 0.0, 0.0,
        maxodr, c_slw, 
        c_TT, sphcoord,
        0, 0, printbar
    ]
    if useFSM:
        parse_args.extend([FSMeps, FSMmaxLoops, FSMparallel])

    FSM_nsweep = FastFunc(*parse_args)
    

    return TT_ravel.reshape(iniTT.shape)



def raytracing(
    TT:np.ndarray, srcloc:list, rcvloc:list,
    xarr:np.ndarray, yarr:np.ndarray, zarr:np.ndarray,
    seglen:float, segfac:int=3, sphcoord:bool=False, maxdots:int=10000):
    r'''
        根据给定源点坐标计算的走时场，使用梯度下降法做射线追踪

        :param         TT:    走时场
        :param     srcloc:    源点坐标，直角坐标系 :math:`(x,y,z)` 或球坐标系 :math:`(r,\theta,\phi)` 
        :param     rcvloc:    接收点坐标，直角坐标系 :math:`(x,y,z)` 或球坐标系 :math:`(r,\theta,\phi)` 
        :param       xarr:    :math:`x` 或 :math:`r` 节点坐标数组，要求等距升序排列 
        :param       yarr:    :math:`y` 或 :math:`\theta` 节点坐标数组，要求等距升序排列 
        :param       yarr:    :math:`z` 或 :math:`\phi` 节点坐标数组，要求等距升序排列 
        :param     seglen:    射线段长度
        :param     segfac:    t < segfac*seglen/v，当射线追踪到在源点附近时，射线直接连接源点
        :param   sphcoord:    是否使用球坐标
        :param    maxdots:    射线最大点数

        :return:  (接收点走时，形状为(ndots, 3)的射线坐标)
    '''


    TT_ravel = TT.ravel().astype(c_interfaces.NPCT_REAL_TYPE)
    c_TT = npct.as_ctypes(TT_ravel)

    c_xarr = npct.as_ctypes(xarr.astype('f8'))
    c_yarr = npct.as_ctypes(yarr.astype('f8'))
    c_zarr = npct.as_ctypes(zarr.astype('f8'))

    sx, sy, sz = np.array(srcloc).astype('f8')
    rx, ry, rz = np.array(rcvloc).astype('f8')

    rays = np.empty((maxdots*3,), dtype='f8')
    c_rays = npct.as_ctypes(rays)
    c_ndots = c_int(maxdots)

    travt = c_interfaces.C_FMM_raytracing(
        c_xarr, len(xarr),
        c_yarr, len(yarr),
        c_zarr, len(zarr),
        sx, sy, sz,
        rx, ry, rz, float(seglen), int(segfac),
        c_TT, sphcoord, 
        c_rays, byref(c_ndots)
    )

    return travt, rays.reshape((-1,3))[:c_ndots.value, ]



def get_traveltime(
    TT:np.ndarray, rcvloc:list,
    xarr:np.ndarray, yarr:np.ndarray, zarr:np.ndarray):
    r'''
        基于线性插值，从走时场中获取任一点的走时

        :param         TT:    走时场
        :param     rcvloc:    接收点坐标，直角坐标系 :math:`(x,y,z)` 或球坐标系 :math:`(r,\theta,\phi)` 
        :param       xarr:    :math:`x` 或 :math:`r` 节点坐标数组，要求等距升序排列 
        :param       yarr:    :math:`y` 或 :math:`\theta` 节点坐标数组，要求等距升序排列 
        :param       yarr:    :math:`z` 或 :math:`\phi` 节点坐标数组，要求等距升序排列 

        :return:     接收点走时
    
    '''

    return float(interpolate.interpn((xarr, yarr, zarr), TT, np.array(rcvloc)))


def check_xyz_arr(
    xarr:np.ndarray, yarr:np.ndarray, zarr:np.ndarray, sphcoord:bool):

    # 检查维数
    if len(xarr)==0:
        raise ValueError("xarr is empty.")
    if len(yarr)==0:
        raise ValueError("yarr is empty.")
    if len(zarr)==0:
        raise ValueError("zarr is empty.")

    # 检查是否升序排列
    if not np.all(xarr[1:] >= xarr[:-1]):
        raise ValueError("xarr should be in ascending order.")
    if not np.all(yarr[1:] >= yarr[:-1]):
        raise ValueError("yarr should be in ascending order.")
    if not np.all(zarr[1:] >= zarr[:-1]):
        raise ValueError("zarr should be in ascending order.")
    
    # 检查特殊点 
    # if(sphcoord):
    #     if np.any(xarr < 0.0):
    #         raise ValueError("negative values in radius are not allowed.")
        
    #     if np.any(xarr == 0.0):
    #         print("WARNING! 0.0 in radius array, a slight offset (1e-5) is added.")
    #         xarr[...] += 1e-5

    #     if np.any(yarr < 0.0) or np.any(yarr > np.pi):
    #         raise ValueError(f"Theta array is out of bound.")
