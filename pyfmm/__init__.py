from .c_interfaces import load_c_lib, set_fsm_num_threads

# 默认使用双精度
load_c_lib(use_float=False)

from . import traveltime
from .traveltime import *

from . import c_interfaces


from ._version import __version__
