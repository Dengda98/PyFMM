"""
    :file:     logger.py  
    :author:   Zhu Dengda (zhudengda@mail.iggcas.ac.cn)  
    :date:     2024-12

    主要用于统一地给出WARNING

"""

import logging


myLogger = logging.getLogger("pyfmm")
'''自定义logger, 基于logging库'''

myLogger.setLevel(logging.WARNING)

console_handler = logging.StreamHandler()
console_handler.setLevel(logging.WARNING)

formatter = logging.Formatter('[%(levelname)s] %(message)s')
console_handler.setFormatter(formatter)

myLogger.addHandler(console_handler)