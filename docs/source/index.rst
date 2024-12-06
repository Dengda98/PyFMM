**PyFMM** 文档 
===================

`项目主页 <https://github.com/Dengda98/PyFMM>`_  

:Author: Zhu Dengda
:Email:  zhudengda@mail.iggcas.ac.cn


|pic1|   |pic2|

.. |pic1| image:: ../../figs/output2.png
   :width: 45%

.. |pic2| image:: ../../figs/output3.png
   :width: 45%


-----------------------------------------------------------


**PyFMM** ： 基于 **Fast Marching/Sweeping Method** 求解程函方程 :math:`|\nabla T|^2 = s^2` 的程序包。
代码根据 :ref:`主要参考 <main_ref>` 中详述的原理进行实现，其中 **Fast Sweeping 及其并行方法** 基于 :ref:`(Zhao, 2004) <zhao_2004>` 和 :ref:`(Zhao, 2007) <zhao_2007>`

我主要使用该代码计算地震波从震源出发在复杂介质中传播形成的初至波走时场，
并使用梯度下降获得满足费马原理的射线路径，故代码中的一些术语偏专业性。
类似的原理也可用于其它方面，如计算点到曲线/面的距离，或光学、电磁学等。  

**PyFMM** 目前可在Linux、Mac系统上运行，在Windows系统上可借助 `WSL <https://learn.microsoft.com/en-us/windows/wsl/>`_ 兼容运行。  

-----------------------------------------------------------


我还制作了一个简易图形界面 `PyFMM-GUI <https://github.com/Dengda98/PyFMM-GUI>`_ 
计算二维走时场，初学者可更好的理解射线追踪，也可更方便、直观地看到不同速度场下射线的
扭曲形态。


.. image:: https://github.com/Dengda98/PyFMM-GUI/blob/main/figs/example.gif
   :alt: gif_example
   :width: 80%
   :align: center

-----------------------------------------------------------


+ **Python语言的便携、可扩展性与C语言的计算高效特点结合**。
  C程序被编译链接成动态库 *libfmm.so* ，**PyFMM** 再基于Python的 
  `ctypes <https://docs.python.org/3/library/ctypes.html>`_ 标准库\
  实现对C库函数的调用。再基于第三方库 `NumPy <https://numpy.org/>`_ 、 
  `SciPy <https://scipy.org/>`_ 等可很方便地完成对C程序结果的数据整合。
\

+ C代码采取模块化编写，各功能分在不同代码文件中。
\

+ 支持二维和三维情况。
\

+ 支持直角坐标系和球坐标系。
\

+ 中文注释及示例。
\


主要使用的函数都在 :class:`pyfmm.traveltime <pyfmm.traveltime>` 模块中，参数简单，
建议使用前结合api说明以及示例试试。代码是我在初学时写的，如果遇到bug，欢迎联系我，我会完善！
也欢迎提出建议和更多示例！


---------------------------

.. toctree::
   :maxdepth: 1
   :caption: 目录

   install
   jupyter_examples
   about_FSM
   API/pyfmm_api
   API/h_pyfmm_api


.. _main_ref: 

主要参考
---------

.. [1] Sethian, J. A. (1996). A fast marching level set method 
       for monotonically advancing fronts., Proc. Natl. Acad. Sci. U.S.A. 93, no. 4, 
       1591–1595, doi: 10.1073/pnas.93.4.1591.

.. [2] Popovici, A. M., and J. A. Sethian (2002). 3‐D imaging using 
       higher order fast marching traveltimes, GEOPHYSICS 67, no. 2, 604–609, 
       doi: 10.1190/1.1468621.


.. [3] Rawlinson, N., and M. Sambridge (2004). Wave front evolution in 
       strongly heterogeneous layered media using the fast marching method, 
       Geophysical Journal International 156, no. 3, 631–647, doi: 10.1111/j.1365-246X.2004.02153.x.

.. _zhao_2004:

.. [4] Zhao, H. (2004). A fast sweeping method for Eikonal equations, 
       Math. Comp. 74, no. 250, 603–627, doi: 10.1090/S0025-5718-04-01678-3.  

.. _zhao_2007:

.. [5] Zhao, H. (2007). Parallel implementations of the fast sweeping method, 
       Journal of Computational Mathematics 25, no. 4, 421–429.

