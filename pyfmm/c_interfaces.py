"""
    :file:     c_interfaces.py  
    :author:   Zhu Dengda (zhudengda@mail.iggcas.ac.cn)  
    :date:     2023-04

    该文件包括 C库的调用接口  

"""

import os
from ctypes import POINTER, c_bool, c_double, c_int, c_size_t, cdll

REAL = c_double
PREAL = POINTER(REAL)
PSIZE_T = POINTER(c_size_t)


libfmm = cdll.LoadLibrary(
    os.path.join(
        os.path.abspath(os.path.dirname(__file__)),
        "C_extension/lib/libfmm.so"))
"""libfmm 库"""


C_FastMarching = libfmm.FastMarching
"""C库中计算走时场的主函数 FastMarching, 详见C API同名函数"""

C_FMM_raytracing = libfmm.FMM_raytracing
"""C库中根据走时场进行射线追踪 FMM_raytracing, 详见C API同名函数"""


C_FastMarching.restype = None
C_FastMarching.argtypes = [
    PREAL, c_size_t,
    PREAL, c_size_t,
    PREAL, c_size_t,
    REAL, REAL, REAL,
    c_size_t, PREAL,
    PREAL, c_bool,
    c_size_t, c_size_t, c_bool
]


C_FMM_raytracing.restype = REAL
C_FMM_raytracing.argtypes = [
    PREAL, c_size_t,
    PREAL, c_size_t,
    PREAL, c_size_t,
    REAL, REAL, REAL,
    REAL, REAL, REAL, REAL, REAL,
    PREAL, PREAL, c_bool,
    PREAL, PSIZE_T, POINTER(c_int)
]

C_FastSweeping = libfmm.FastSweeping
C_FastSweeping.restype = c_size_t
C_FastSweeping.argtypes = [
    PREAL, c_size_t,
    PREAL, c_size_t,
    PREAL, c_size_t,
    REAL, REAL, REAL,
    c_size_t, PREAL,
    PREAL, c_bool,
    c_size_t, c_size_t, c_bool,
    REAL, c_size_t, c_bool
]

C_set_fsm_num_threads = libfmm.set_fsm_num_threads
C_set_fsm_num_threads.restype = None
C_set_fsm_num_threads.argtypes = [c_size_t]


def set_fsm_num_threads(n):
    r'''
        定义Fast Sweeping Method使用的多线程数

        :param       n:    线程数
    '''
    C_set_fsm_num_threads(n)
