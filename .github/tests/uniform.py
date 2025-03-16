import pyfmm
import numpy as np

xarr = np.arange(0, 100, 0.08)
yarr = np.arange(0, 50, 0.05)
zarr = np.array([0.0])  # 二维情况


# 慢度场
slw  = np.ones((len(xarr), len(yarr), len(zarr)), dtype='f')
print(slw.shape)

srcloc = [10, 20, 0.0]

# FMM解
FMMTT = pyfmm.travel_time_source(
    srcloc,
    xarr, yarr, zarr, slw)

# FSM解
FSMTT = pyfmm.travel_time_source(
    srcloc,
    xarr, yarr, zarr, slw, useFSM=True)

# 真实解
xx, yy, zz = srcloc
real_TT = np.sqrt(((xarr-xx)**2)[:,None,None] + ((yarr-yy)**2)[None,:,None] + ((zarr-zz)**2)[None,None,:])

# 误差 
FMM_error = np.mean(np.abs(FMMTT - real_TT))
FSM_error = np.mean(np.abs(FSMTT - real_TT))
print("FMM_error = ", FMM_error)
print("FSM_error = ", FSM_error)

tol = 0.1

if FMM_error > tol:
    raise ValueError(f"FMM_error({FMM_error}) > tol({tol})")
if FSM_error > tol:
    raise ValueError(f"FMM_error({FSM_error}) > tol({tol})")