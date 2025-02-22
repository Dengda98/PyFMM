
安装
=============

:Author: Zhu Dengda
:Email:  zhudengda@mail.iggcas.ac.cn

-----------------------------------------------------------


**PyFMM**  目前可在Linux、Mac系统上运行，在Windows系统上可借助 `WSL <https://learn.microsoft.com/en-us/windows/wsl/>`_ 兼容运行。依赖库包括：

:: 

    python >= 3.9
    numpy >= 1.20, < 2.0
    scipy >= 1.10 
    matplotlib >= 3.5 
    jupyter 


C程序的编译链接基于 :code:`gcc` 编译器 + :code:`OpenMP` ，编译命令使用 :code:`make`，**确保系统已安装相应工具**。 

以ubuntu为例，安装 :code:`gcc` 和 :code:`make` :

::

    sudo apt install build-essential 

目前常见编译器应该已内置 :code:`OpenMP` 工具，或可运行以下命令安装:

:: 

    sudo apt install libomp-dev

在Mac系统上则运行:

::

    brew install libomp

Makefile位于 :code:`pyfmm/C_extension/Makefile` ，你可以自定义你的编译器和编译命令。 


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

.. _for_linux_or_mac:

For Linux or Mac
^^^^^^^^^^^^^^^^^^



要求系统上有 :code:`gcc`，:code:`make`，:code:`openmp` 等开发工具。有多种下载方式：  

+ **从** `PYPI <https://pypi.org/project/pyfmm-kit/>`_ **下载（推荐）**    
  
  **最新更新，已将PyFMM程序源码同步到** `PYPI <https://pypi.org/project/pyfmm-kit/>`_ **，可直接运行：**    
  ::
    
    pip install -v pyfmm-kit

  进行安装，-v选项以查看源码的编译进度。各操作平台的wheels后续会逐步支持。

+ 从Github下载安装，安装有两个选择：

  + **不提前下载程序包**  

    要求你当前环境中有 :code:`git` 工具。这样可直接运行
    ::

        # vx.x.x指代Release中的版本号，建议下载最新稳定版本
        pip install -v git+https://github.com/Dengda98/PyFMM@vx.x.x
        # 若网速不好可改为gitee镜像
        # pip install -v git+https://gitee.com/Dengda98/PyFMM@vx.x.x

  + **提前下载好程序包** 

    注意代码主页更新频繁，**建议在** `Release <https://github.com/Dengda98/PyFMM/releases>`_ **中下载最新稳定版本**，
    不推荐直接clone，不推荐直接下载主分支。将压缩包解压后，进入目录，直接运行pip安装命令  
    ::

        pip install -v . 


这将完成以上Python库的安装，以及C程序的编译链接。


For Windows (based on WSL)
^^^^^^^^^^^^^^^^^^

尽管我仍然建议在Linux或Mac上运行，但借助于 `WSL <https://learn.microsoft.com/en-us/windows/wsl/>`_ ， **PyFMM** 可以在Windows系统上运行。WSL可以简单理解为Windows上的更轻便简洁的Linux虚拟机。安装见 `WSL <https://learn.microsoft.com/en-us/windows/wsl/>`_ 链接，建议使用WSL2版本。在WSL中，安装 **PyFMM** 的方法就和在 :ref:`Linux <for_linux_or_mac>` 中一样了。事实上，由于条件受限，我就是在WSL上开发的 **PyFMM** 。


关于一些安装问题
^^^^^^^^^^^^^^^^^^

+ Q：安装过程没发现问题，但在Python中运行 :code:`import pyfmm` 报错：
  ::
  
    OSError: dlopen(...) ... (no such file) ... (mach-o file, but is anincompatible architecture (have 'arm64', need 'x86_64'))


  A：这是安装编译的 **PyFMM** 和系统架构不匹配，属于比较少见的情况。正常情况下，不论Mac的芯片是Intel(x86_64)还是Apple Silicon(arm64)，编译C库时编译器会自动匹配架构。如果出现上述特殊情况，可以手动修改Makefile，在编译时指定架构，具体为  
    
  + 下载 **PyFMM** 程序包
  + 在 :code:`pyfmm/C_extension/Makefile` 中修改编译器为 :code:`CC = clang` （似乎 :code:`gcc` 不太好做交叉编译），将编译选项增加为 :code:`CFLAGS = --target=x86_64-apple-darwin ...(其它不变)`  
  + 在程序根目录下，运行 :code:`pip install -v .` 重新安装
  
  这样更改后重新安装，就可解决架构不匹配的问题。 **如果你是类似问题，可以在** :code:`CFLAGS` **中指定其它架构，详见** `clang编译器说明 <https://clang.llvm.org/docs/CrossCompilation.html>`_ 。

+ Q：关于安装错误  
  
  A：建议在使用 :code:`pip` 安装时加上 :code:`-v` 选项，对于安装过程的输出可往前翻看具体原因，常常输出的最后一段不是直接原因。例如常见的，系统没有 :code:`openmp` 导致安装错误，建议在安装前确保这些编译工具已安装。