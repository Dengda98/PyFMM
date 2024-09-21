/**
 * @file   coord.h
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
 */


#pragma once

#include <math.h>

/**
 * 球坐标 \f$ (r,\theta,\phi) \f$  转直角坐标 \f$ (x,y,z) \f$
 * 
 * @param     r      (in)坐标 \f$ r \f$
 * @param     t      (in)坐标 \f$ \theta \f$
 * @param     p      (in)坐标 \f$ \phi \f$
 * @param     x      (out)坐标 \f$ x \f$
 * @param     y      (out)坐标 \f$ y \f$
 * @param     z      (out)坐标 \f$ z \f$
 * 
 */
void rtp2xyz(double r, double t, double p, double *x, double *y, double *z){
    *x = r*sin(t)*cos(p);
    *y = r*sin(t)*sin(p);
    *z = r*cos(t);
}
