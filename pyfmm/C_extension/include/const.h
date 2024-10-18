/**
 * @file   const.h
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
 */

#pragma once

#define RADIUS 6371.0              ///< 地球半径 (km)
#define PI 3.141592653589793       ///< \f$ \pi \f$
#define PI2 6.283185307179586      ///< \f$ 2\pi \f$
#define HALFPI 1.5707963267948966  ///< \f$ \frac{\pi}{2} \f$
#define RAD1 57.29577951308232     ///< \f$ \frac{180}{\pi} \f$
#define DEG1 0.017453292519943295  ///< \f$ \frac{\pi}{180} \f$
#define KM1DEG 111.194926644       ///< \f$ R_{e} \times \frac{\pi}{180} \f$

#ifdef USE_FLOAT
typedef float MYREAL;   ///< 单精度 or 双精度
#else
typedef double MYREAL;
#endif