"""
    :file:     c_interfaces.py  
    :author:   Zhu Dengda (zhudengda@mail.iggcas.ac.cn)  
    :date:     2023-04

    该文件包括 C库的调用接口  

"""

import os
from ctypes import *

__all__ = [
    'USE_FLOAT',
    'NPCT_REAL_TYPE',
    'C_FastMarching',
    'C_FMM_raytracing'
]

USE_FLOAT = False
"""libfmm库中走时和慢度数组是否使用单精度浮点数"""

NPCT_REAL_TYPE = 'f4' if USE_FLOAT else 'f8'

REAL = c_float if USE_FLOAT else c_double
PREAL = POINTER(REAL)
PDOUBLE= POINTER(c_double)
PINT= POINTER(c_int)


libfmm = cdll.LoadLibrary(
    os.path.join(
        os.path.abspath(os.path.dirname(__file__)), 
        "C_extension/lib/libfmm.so"))
"""libfmm库"""


C_FastMarching = libfmm.FastMarching
"""C库中计算走时场的主函数 FastMarching, 详见C API同名函数"""

C_FMM_raytracing = libfmm.FMM_raytracing
"""C库中根据走时场进行射线追踪 FMM_raytracing, 详见C API同名函数"""


C_FastMarching.restype = None 
C_FastMarching.argtypes = [
    PDOUBLE, c_int,
    PDOUBLE, c_int,
    PDOUBLE, c_int,
    c_double, c_double, c_double, 
    c_int, PREAL,
    PREAL, c_bool,
    c_int, c_int, c_bool
]


C_FMM_raytracing.restype = c_float 
C_FMM_raytracing.argtypes = [
    PDOUBLE, c_int,
    PDOUBLE, c_int,
    PDOUBLE, c_int,
    c_double, c_double, c_double, 
    c_double, c_double, c_double, c_double, c_double, 
    PREAL, c_bool,
    PDOUBLE, PINT
]