
安装
=============

:Author: Zhu Dengda
:Email:  zhudengda@mail.iggcas.ac.cn

-----------------------------------------------------------


从当前本版本起， **PyFMM** 提供多平台预编译版本的安装包，无需再进行本地编译。

支持操作系统：

.. raw:: html

   <input type="checkbox" checked disabled> Linux<br>
   <input type="checkbox" checked disabled> macOS<br>
   <input type="checkbox" checked disabled> Windows<br>

\  


依赖库包括：

:: 

    python >= 3.9
    numpy >= 1.20, < 2.0
    scipy >= 1.10 
    matplotlib >= 3.5 
    jupyter 



程序下载和安装
--------------


由于以上涉及的库均为常用库，如果你对上述依赖库很熟悉，并且了解Python库的安装以及虚拟环境的使用，\
则可根据自己Python环境的情况快速安装，直接到 :ref:`第2步 <install_step_2>` 。


如果你还不了解Python库的安装以及虚拟环境的搭建，可以先在网上寻找相关教程。这里以 `Anaconda <https://www.anaconda.com>`_\
为例，搭建虚拟环境，完成Python库的安装。


环境创建  
~~~~~~~~~~  

新建虚拟环境，命名为py310，安装python=3.10  
:: 

    conda create -n py310 python=3.10  

激活虚拟环境  
::

    conda activate py310  

.. _install_step_2:

下载和安装 **PyFMM**   
~~~~~~~~~~~~~~~~~~~~~

Make Life Easier，可直接运行pip进行安装：

::
  
  pip install pyfmm-kit



关于一些安装问题
^^^^^^^^^^^^^^^^^^

+ Q: Python中运行 :code:`import pyfmm` 提示找不到库路径或缺少库依赖？  
  
  A: 这一般出现在Mac和Ubuntu系统上，很可能是找不到 :code:`OpenMP` 库，尝试重新安装 :code:`gcc`。