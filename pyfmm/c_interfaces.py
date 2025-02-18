"""
    :file:     c_interfaces.py  
    :author:   Zhu Dengda (zhudengda@mail.iggcas.ac.cn)  
    :date:     2023-04

    该文件包括 C库的调用接口  

"""

import os
from ctypes import *
from typing import Any

PDOUBLE = POINTER(c_double)
PFLOAT = POINTER(c_float)

USE_FLOAT:bool = False
"""libfmm库中走时和慢度数组是否使用单精度浮点数"""
NPCT_REAL_TYPE:str = 'f8'

USE_LONG:bool = True 
"""使用长整型整数避免统计网格点数量时溢出"""
INT = c_long if USE_LONG else c_int
PINT = POINTER(INT)


C_FastMarching:Any = None
C_FMM_raytracing:Any = None
C_FastSweeping:Any = None
C_set_fsm_num_threads:Any = None

def load_c_lib(use_float:bool=False):
    r'''
        加载单精度或双精度的C库，修改c_interfaces下的NPCT_REAL_TYPE变量和C函数接口

        :param       use_float:    是否使用单精度
    '''
    global USE_FLOAT, NPCT_REAL_TYPE, C_FastMarching, C_FMM_raytracing, C_FastSweeping, C_set_fsm_num_threads

    USE_FLOAT = use_float
    NPCT_REAL_TYPE = 'f4' if USE_FLOAT else 'f8'
    _suffix = 'float' if USE_FLOAT else 'double'

    REAL = c_float if USE_FLOAT else c_double
    PREAL = POINTER(REAL)

    libfmm = cdll.LoadLibrary(
        os.path.join(
            os.path.abspath(os.path.dirname(__file__)), 
            f"C_extension/lib/libfmm_{_suffix}.so"))
    """libfmm库"""


    C_FastMarching = libfmm.FastMarching
    """C库中计算走时场的主函数 FastMarching, 详见C API同名函数"""

    C_FMM_raytracing = libfmm.FMM_raytracing
    """C库中根据走时场进行射线追踪 FMM_raytracing, 详见C API同名函数"""


    C_FastMarching.restype = None 
    C_FastMarching.argtypes = [
        PDOUBLE, INT, 
        PDOUBLE, INT,
        PDOUBLE, INT,
        c_double, c_double, c_double, 
        INT, PREAL,
        PREAL, c_bool,
        INT, INT, c_bool
    ]


    C_FMM_raytracing.restype = REAL 
    C_FMM_raytracing.argtypes = [
        PDOUBLE, INT,
        PDOUBLE, INT,
        PDOUBLE, INT,
        c_double, c_double, c_double, 
        c_double, c_double, c_double, c_double, c_double, 
        PREAL, PREAL, c_bool,
        PDOUBLE, PINT
    ]

    C_FastSweeping = libfmm.FastSweeping
    C_FastSweeping.restype = INT 
    C_FastSweeping.argtypes = [
        PDOUBLE, INT,
        PDOUBLE, INT,
        PDOUBLE, INT,
        c_double, c_double, c_double, 
        INT, PREAL,
        PREAL, c_bool,
        INT, INT, c_bool, 
        c_double, INT, c_bool
    ]

    C_set_fsm_num_threads = libfmm.set_fsm_num_threads
    C_set_fsm_num_threads.restype = None
    C_set_fsm_num_threads.argtypes = [INT]


def set_fsm_num_threads(n):
    r'''
        定义Fast Sweeping Method使用的多线程数

        :param       n:    线程数
    '''
    C_set_fsm_num_threads(n)