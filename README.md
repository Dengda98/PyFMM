
<h1 align="center"><b><i>PyFMM</i></b></h1>

<p align="center">
  <a href="https://pyfmm.readthedocs.io/zh-cn/latest/?badge=latest">
    <img src="https://readthedocs.org/projects/pyfmm/badge/?version=latest" alt="Documentation Status" />
  </a>
  <a href="https://zenodo.org/doi/10.5281/zenodo.13823187">
    <img src="https://zenodo.org/badge/860537381.svg" alt="DOI" />
  </a>
  <img alt="GitHub code size in bytes" src="https://img.shields.io/github/languages/code-size/Dengda98/PyFMM">
  <img alt="GitHub License" src="https://img.shields.io/github/license/Dengda98/PyFMM">
  <img alt="GitHub Actions Workflow Status" src="https://img.shields.io/github/actions/workflow/status/Dengda98/PyFMM/build.yml">

</p>


<p align="center">
  <img src="./figs/output2.png" alt="Image 2" width="55%" />
  <img src="./figs/output3.png" alt="Image 3" width="42%" />
</p>

**欢迎Star！**

[**PyFMM**](https://github.com/Dengda98/PyFMM) 是一个基于 **Fast Marching/Sweeping Method** 求解程函方程 $|\nabla T|^2 = s^2$ 的C/Python程序包，包括示例和注释。  其中 **Fast Sweeping Method** 包括了并行版本，详见[**在线文档**](https://pyfmm.readthedocs.io/zh-cn/latest/)或文献 [(Zhao, 2007)](https://www.jstor.org/stable/43693378)。

[**PyFMM**](https://github.com/Dengda98/PyFMM) is a C/Python package for solving eikonal equation using Fast Marching/Sweeping Method, with examples and annotations.  

At present, **PyFMM** can run on
  - [x] Linux
  - [x] macOS
  - [x] Windows

----

我还制作了一个简易图形界面 [**PyFMM-GUI**](https://github.com/Dengda98/PyFMM-GUI) 计算二维走时场，初学者可更好的理解射线追踪，也可更方便、直观地看到不同速度场下射线的扭曲形态。

![](https://github.com/Dengda98/PyFMM-GUI/blob/main/figs/example.gif)

-------
</br>

我主要使用 **PyFMM** 计算地震波从震源出发在复杂介质中传播形成的初至波走时场，
并使用梯度下降获得满足费马原理的射线路径，故代码中的一些术语偏专业性。
类似的原理也可用于其它方面，如计算点到曲线/面的距离，或光学、电磁学等。


+ **Python语言的便携、可扩展性与C语言的计算高效特点结合**。
  C程序被编译链接成动态库 *libfmm.so* ，**PyFMM** 再基于Python的 [ctypes](https://docs.python.org/3/library/ctypes.html)
  标准库实现对C库函数的调用。再基于第三方库 [NumPy](https://numpy.org/)、 
  [SciPy](https://scipy.org/) 等可很方便地完成对C程序结果的数据整合；


+ C代码采取模块化编写，各功能分在不同代码文件中，方便移植到其它程序；


+ 支持二维和三维情况；2D and 3D


+ 支持直角坐标系和球坐标系；Cartesian and Spherical Coordinate


+ 中文注释及示例；

<br>


# 文档 Documents
为方便使用，我建立了[**在线文档**](https://pyfmm.readthedocs.io/zh-cn/latest/)，包括简易安装、API的介绍以及使用示例。  

<br>

# 安装 Installation 

**新版本已添加预编译的C动态库**，无需本地再编译，支持`pip`命令一键安装：
```bash
pip install pyfmm-kit
```


<br>


# 使用示例 Usage Example 
更多使用示例详见[**在线文档**](https://pyfmm.readthedocs.io/zh-cn/latest/)。
``` python 
import pyfmm 
import numpy as np 
import matplotlib.pyplot as plt
from scipy import interpolate

pyfmm.logger.myLogger.setLevel('ERROR')

# 定义网格 
nx, ny, nz = 401, 1, 101
xarr = np.linspace(0, 200, nx)
yarr = np.array([0.0])
zarr = np.linspace(0, 50, nz)

# 定义1D速度
vel1d = np.array([
    [0.0, 3.2],
    [5.0, 5.8],
    [15.0, 6.5],
    [30.0, 6.8],
    [35.0, 8.1],
    [80.0, 8.2]
])

# 插值1d分层速度
# _idxs = np.searchsorted(vel1d[:,0], zarr)
# velocity = vel1d[_idxs, 1]
# OR
# 插值1d梯度速度 
velocity = interpolate.interpn((vel1d[:,0],), vel1d[:,1], zarr)

# 慢度数组 
slowness = np.empty((nx, ny, nz))
slowness[...] = 1.0/velocity[None,None,:]

# 定义震源位置
srcloc = [0.0, 0.0, 0.0]

# 计算时间场
TT = pyfmm.travel_time_source(
    srcloc,
    xarr, yarr, zarr, slowness)

#====================================================================
# 绘制走时场和射线
fig, ax1 = plt.subplots(1, 1)
cs = ax1.contour(xarr, zarr, TT[:, 0, :].T, levels=30, linewidths=0.5)
ax1.clabel(cs)

for x in np.arange(5, 200, 5):
    # 射线追踪
    rcvloc = [x, 0, 0]

    travt, rays = pyfmm.raytracing(
        TT, srcloc, rcvloc, xarr, yarr, zarr, 0.1)
    ax1.plot(rays[:,0], rays[:,2], c='r', lw=0.8, ls='--')

ax1.set_aspect('equal')
ax1.set_xlim(0, 200)
ax1.set_ylim(0, 50)
ax1.yaxis.set_inverted(True)

```
![](https://github.com/Dengda98/PyFMM/blob/main/figs/example.png)


# 其它
代码是我在研二写的，如果遇到bug，欢迎联系我(zhudengda@mail.iggcas.ac.cn)，我会完善！
也欢迎提出建议和更多示例！

基于PyFMM的体波走时反演以及面波反演后续也会开源。
