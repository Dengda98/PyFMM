
安装
=============

:Author: Zhu Dengda
:Email:  zhudengda@mail.iggcas.ac.cn

-----------------------------------------------------------


**PyFMM**  目前可在Linux、Mac系统上运行，在Windows系统上可借助 `WSL <https://learn.microsoft.com/en-us/windows/wsl/>`_ 、 `WinLibs <https://winlibs.com/>`_ 等工具运行。依赖库包括：

:: 

    python >= 3.9
    numpy >= 1.20, < 2.0
    scipy >= 1.10 
    matplotlib >= 3.5 
    jupyter 


C程序的编译链接基于 `gcc` 编译器，编译命令使用 `make`，确保系统已安装相应工具。 

以ubuntu为例，安装 `gcc` 和 `make` :

::

    sudo apt install build-essential 


Makefile位于 *C_extension/Makefile* ，你可以自定义你的编译器和编译命令。 


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

要求系统上有`gcc`，`make`开发工具。安装有两个选择：

+ **不提前下载程序包**  

  要求你当前环境中有git工具。这样可直接运行
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


For Windows
^^^^^^^^^^^^^^^^^^

尽管我仍然建议在Linux或Mac上运行，不过经过测试和调整，借助于WSL、WinLibs等工具， **PyFMM** 可以在Windows系统上运行。这里介绍两种方法：

+ `WSL <https://learn.microsoft.com/en-us/windows/wsl/>`_ （**推荐**）

  WSL可以简单理解为Windows上的更轻便简洁的Linux虚拟机。安装见 `WSL <https://learn.microsoft.com/en-us/windows/wsl/>`_ 链接，建议使用WSL2版本。在WSL中，安装 **PyFMM** 的方法就和在 :ref:`Linux <for_linux_or_mac>` 中一样了。事实上，由于条件受限，我就是在WSL上开发的 **PyFMM** 。

+ `WinLibs <https://winlibs.com/>`_

  WinLibs可认为是GCC+ `MinGW-w64 <http://mingw-w64.org/>`_ 的集成体，可以在Windows平台使用Linux中的工具。无需安装，二进制文件均已打包好。

  1. 下载 `WinLibs <https://winlibs.com/>`_ (with POSIX threads)(UCRT)，解压后将文件夹放在合适目录，将路径下的 `bin` 目录路径添加到`PATH`环境变量。
  2. 将 `bin` 目录中的 `mingw32-make.exe` 原地复制一份，并改名为 `make.exe` 。
  3. 此时在Windows的终端上就可以运行 `gcc` 和 `make` 命令，之后安装 **PyFMM** 的方法就和在 :ref:`Linux <for_linux_or_mac>` 中一样了。