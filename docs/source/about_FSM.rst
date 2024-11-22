关于Fast Sweeping Method
=========================

:Author: Zhu Dengda
:Email:  zhudengda@mail.iggcas.ac.cn 


-----------------------------------------------------------

**Fast Sweeping Method(FSM)** 的基本原理详见 :ref:`(Zhao, 2004) <zhao_2004>`，
**PyFMM** 使用的并行方法详见 :ref:`(Zhao, 2007) <zhao_2007>` 中2.1节。并行实现基于 :code:`OpenMP`。


参数简介
------------

主要使用的函数都在 :class:`pyfmm.traveltime <pyfmm.traveltime>` 模块中，
其中函数参数和 **FSM** 相关的包括：

+ :code:`useFSM` (Bool)   (default: False)
  
  是否使用 **FSM** 计算全局走时场。这将决定以下相关的参数是否起作用。

+ :code:`FSMmaxLoops` (int)   (default: 1)

  最大迭代次数。对于3D模型，一次迭代包括向8个方向扫描（Sweep）。根据
  :ref:`(Zhao, 2004) <zhao_2004>` 的测试，速度缓和变化的情况下，
  一次迭代即可达到很好的精度，但对于速度剧烈变化的情况，需要一次以上的迭代来收敛。

+ :code:`FSMparallel` (bool)   (default: False)

  是否使用 **并行FSM**。并行方法详见 :ref:`(Zhao, 2007) <zhao_2007>` 的
  2.1节。简单说，8个方向的Sweep多线程同时进行，再对每个节点取最小值，算一次迭代。
  但要注意的是，这种并行方法是一种 **FSM** 变体，即 **流程本身并不是可并行的。** 
  原始的  **FSM** 每次Sweep是一次Gauss-Seidel式迭代，上一次Sweep结果会影响到
  下一次Sweep。 **并行FSM** 将8次Sweep分多线程单独进行，这要求 
  :code:`FSMmaxLoops > 2` 。

+ :code:`FSMeps` (float)   (default: 0.0)

  每次Sweep后，走时的最大更新量小于 :code:`FSMeps` 时提前结束计算。 




Fast Marching OR Fast Sweeping ?
------------------------------------ 

不存在谁好谁坏。  

**FSM** 的一个核心技巧就是 **并行** ，基于此有不少工作对 **FSM** 做各种并行改进。而 **Fast Marching Method(FMM)** 本身是串行实现以保证强因果性。

这里引用 :ref:`(Zhao, 2007) <zhao_2007>` 中关于 **FSM** 关于串行或并行的一些讨论： 

.. epigraph::

    In general if the characteristics are straight lines, sweepings with different orderings are almost independent of each other and the parallel sweeping algorithm should be as efficient as the original fast sweeping algorithm. However if the characteristics are curved then different orderings implemented sequentially may propagate information faster on a curved characteristics than different orderings implemented in parallel.

结合我的测试结果，我的观点是：

+ 对于速度缓和变化的模型， **FSM** 仅需8次sweep（1次迭代）即可收敛，此时计算效率和 **FMM** 持平，再加上并行， **FSM** 计算效率甚至可以达到 **FMM** 的8倍（接近理论值），且计算误差保持一致（可以以均匀模型做测试）。


+ 但对于速度变化剧烈的模型， **FSM** 需要更多的迭代次数来收敛，尤其是并行算法。当只设置收敛条件 :code:`FSMeps` （如1e-3） 而不设置 :code:`FSMmaxLoops` 时（给定一个较大的 :code:`FSMeps` ），迭代过程中的走时最大更新量可能很难收敛到给定的条件而导致计算效率下降。此时串行的 **FSM** 相比与并行尽管表现出更好的稳定性，但也需要2~3次迭代（1次往往不够），计算时间也是 **FSM** 的2~3倍。

在实际模型中速度变化是未知的，而 **FSM** 的计算结果受较多参数的影响。但影响 **FMM** 结果的因素不多，基本就是靠网格点的稠密程度决定。根据原理， **FMM** 的计算结果呈现无条件稳定，再加上堆排序算法， **FMM** 的计算效率也不落下风。
