/**
 * @file   interp.c
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2023-03
 * 
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include "fmm/const.h"
#include "fmm/interp.h"
#include "fmm/index.h"
#include "fmm/query.h"



real_t trilinear_one_ravel(
    const real_t *x, size_t nx, const real_t *y, size_t ny, const real_t *z, size_t nz, size_t nyz, const real_t *values,
    real_t xi, real_t yi, real_t zi, bool extrapolate, real_t *pdiffx, real_t *pdiffy, real_t *pdiffz,
    size_t IXYZ[6], real_t WGHT[2][2][2])
{
    size_t IXYZ0[6];
    real_t WGHT0[2][2][2];
    trilinear_one_fac(x, nx, y, ny, z, nz, xi, yi, zi, extrapolate, IXYZ0, WGHT0);

    real_t vi;
    vi = trilinear_one_Idx_ravel(IXYZ0, WGHT0, values, nx, ny, nz, nyz, pdiffx, pdiffy, pdiffz);

    if(IXYZ!=NULL){
        for(size_t i=0; i<6; ++i){
            IXYZ[i] = IXYZ0[i];
        }
    } 
    if(WGHT!=NULL){
        for(size_t i=0; i<2; ++i){
            for(size_t j=0; j<2; ++j){
                for(size_t k=0; k<2; ++k){
                    WGHT[i][j][k] = WGHT0[i][j][k];
                }
            }
        }
    }

    return vi;
}



void trilinear_one_fac(
    const real_t *x, size_t nx, const real_t *y, size_t ny, const real_t *z, size_t nz,
    real_t xi, real_t yi, real_t zi, bool extrapolate, size_t IXYZ[6], real_t WGHT[2][2][2])
{
    size_t ix = dicho_find(x, nx, xi);
    size_t iy = dicho_find(y, ny, yi);
    size_t iz = dicho_find(z, nz, zi);

    // 外推使用边界处的网格单元，单节点维度不进行偏移
    if(extrapolate){
        if(nx>1 && ix==nx-1) ix--;
        if(ny>1 && iy==ny-1) iy--;
        if(nz>1 && iz==nz-1) iz--;
    }
    size_t ix1 = ix<nx-1 ? ix+1 : ix;
    size_t iy1 = iy<ny-1 ? iy+1 : iy;
    size_t iz1 = iz<nz-1 ? iz+1 : iz;

    // printf("%d,%d,%d, %f,%f,%f\n", ix, iy, iz, xi, yi, zi);
    real_t x1 = x[ix];
    real_t x2 = x[ix1];
    real_t y1 = y[iy];
    real_t y2 = y[iy1];
    real_t z1 = z[iz];
    real_t z2 = z[iz1];

    // 限制范围
    if(!extrapolate){
        if(xi < x1) xi = x1;
        if(yi < y1) yi = y1;
        if(zi < z1) zi = z1;
        if(xi > x2) xi = x2;
        if(yi > y2) yi = y2;
        if(zi > z2) zi = z2;
    }

    real_t xfac=0.0, yfac=0.0, zfac=0.0, xfac1, yfac1, zfac1;
    if(ix!=ix1) xfac = (xi-x1)/(x2-x1);
    if(iy!=iy1) yfac = (yi-y1)/(y2-y1);
    if(iz!=iz1) zfac = (zi-z1)/(z2-z1);

    xfac1 = 1.0 - xfac; 
    yfac1 = 1.0 - yfac; 
    zfac1 = 1.0 - zfac; 

    real_t f111, f121, f211, f221, f112, f122, f212, f222;
    f111 = xfac1 * yfac1 * zfac1;
    f121 = xfac1 * yfac  * zfac1;
    f211 = xfac  * yfac1 * zfac1;
    f221 = xfac  * yfac  * zfac1;
    f112 = xfac1 * yfac1 * zfac ;
    f122 = xfac1 * yfac  * zfac ;
    f212 = xfac  * yfac1 * zfac ;
    f222 = xfac  * yfac  * zfac ;

    if(IXYZ!=NULL){
        IXYZ[0] = ix;
        IXYZ[1] = ix1;
        IXYZ[2] = iy;
        IXYZ[3] = iy1;
        IXYZ[4] = iz;
        IXYZ[5] = iz1;
    }
    if(WGHT!=NULL){
        WGHT[0][0][0] = f111; WGHT[0][1][0] = f121;
        WGHT[1][0][0] = f211; WGHT[1][1][0] = f221;
        WGHT[0][0][1] = f112; WGHT[0][1][1] = f122;
        WGHT[1][0][1] = f212; WGHT[1][1][1] = f222;
    }
}


real_t trilinear_one_Idx_ravel(
    const size_t IXYZ[6], const real_t WGHT[2][2][2],  const real_t *values, size_t nx, size_t ny, size_t nz, size_t nyz,
    real_t *pdiffx, real_t *pdiffy, real_t *pdiffz)
{
    size_t ix, ix1, iy, iy1, iz, iz1;
    real_t f111, f121, f211, f221, f112, f122, f212, f222;
    real_t v111,v121,v211,v221, v112,v122,v212,v222;

    ix  = IXYZ[0];
    ix1 = IXYZ[1];
    iy  = IXYZ[2];
    iy1 = IXYZ[3];
    iz  = IXYZ[4];
    iz1 = IXYZ[5];


    size_t idx;
    ravel_index(&idx, nyz, nz, ix, iy, iz);
    size_t dx, dy, dz, DX, DY, DZ;
    DX = nyz;
    DY = nz;
    DZ = 1;
    dx = (ix1>ix)? DX  : 0;
    dy = (iy1>iy)? DY  : 0;
    dz = (iz1>iz)? DZ  : 0;

    v111 = values[idx              ];  v121 = values[idx      + dy     ]; 
    v211 = values[idx + dx         ];  v221 = values[idx + dx + dy     ]; 
    v112 = values[idx          + dz];  v122 = values[idx      + dy + dz]; 
    v212 = values[idx + dx     + dz];  v222 = values[idx + dx + dy + dz]; 

    f111 = WGHT[0][0][0]; f121 = WGHT[0][1][0];
    f211 = WGHT[1][0][0]; f221 = WGHT[1][1][0];
    f112 = WGHT[0][0][1]; f122 = WGHT[0][1][1];
    f212 = WGHT[1][0][1]; f222 = WGHT[1][1][1];

    real_t dv111,dv121,dv211,dv221, dv112,dv122,dv212,dv222;
    dv111=dv121=dv211=dv221=dv112=dv122=dv212=dv222=0.0;
    if(pdiffx!=NULL){
        if(ix==0){
            dv111 = v211 - v111;
            dv121 = v221 - v121;
            dv112 = v212 - v112;
            dv122 = v222 - v122;

        } else if(ix==nx-1){
            dv111 = v111 - values[idx - DX];
            dv121 = v121 - values[idx - DX + dy];
            dv112 = v112 - values[idx - DX + dz];
            dv122 = v122 - values[idx - DX + dy + dz];

        } else {
            dv111 = (values[idx + DX] - values[idx - DX])/2.0;
            dv121 = (values[idx + DX + dy] - values[idx - DX + dy])/2.0;
            dv112 = (values[idx + DX + dz] - values[idx - DX + dz])/2.0;
            dv122 = (values[idx + DX + dy + dz] - values[idx - DX + dy + dz])/2.0;

        }

        if(nx > 2 && ix < nx-2){
            dv211 = (values[idx + 2*DX] - v111)/2.0;
            dv221 = (values[idx + 2*DX + dy] - v121)/2.0;
            dv212 = (values[idx + 2*DX + dz] - v112)/2.0;
            dv222 = (values[idx + 2*DX + dy + dz] - v122)/2.0;
        } else if(nx > 1){ // 设置nx > 1的判断，以防越界
            dv211 = (v111 - values[idx - 2*DX])/2.0;
            dv221 = (v121 - values[idx - 2*DX + dy])/2.0;
            dv212 = (v112 - values[idx - 2*DX + dz])/2.0;
            dv222 = (v122 - values[idx - 2*DX + dy + dz])/2.0;
        }

        *pdiffx = f111*dv111 + f121*dv121 + f211*dv211 + f221*dv221 +
                  f112*dv112 + f122*dv122 + f212*dv212 + f222*dv222;
    }
    
    
    if(pdiffy!=NULL){
        if(iy==0){
            dv111 = v121 - v111;
            dv211 = v221 - v211;
            dv112 = v122 - v112;
            dv212 = v222 - v212;

        } else if(iy==ny-1){
            dv111 = v111 - values[idx - DY];
            dv211 = v211 - values[idx + dx - DY];
            dv112 = v112 - values[idx - DY + dz];
            dv212 = v212 - values[idx + dx - DY + dz];

        } else {
            dv111 = (values[idx + DY] - values[idx - DY])/2.0;
            dv211 = (values[idx + dx + DY] - values[idx + dx - DY])/2.0;
            dv112 = (values[idx + DY + dz] - values[idx - DY + dz])/2.0;
            dv212 = (values[idx + dx + DY + dz] - values[idx + dx - DY + dz])/2.0;
        }

        if(ny > 2 && iy < ny-2){
            dv121 = (values[idx + 2*DY] - v111)/2.0;
            dv221 = (values[idx + dx + 2*DY] - v211)/2.0;
            dv122 = (values[idx + 2*DY + dz] - v112)/2.0;
            dv222 = (values[idx + dx + 2*DY + dz] - v212)/2.0;
        } else if(ny > 1){
            dv121 = (v111 - values[idx - 2*DY])/2.0;
            dv221 = (v211 - values[idx + dx - 2*DY])/2.0;
            dv122 = (v112 - values[idx - 2*DY + dz])/2.0;
            dv222 = (v212 - values[idx + dx - 2*DY + dz])/2.0;
        }
        

        *pdiffy = f111*dv111 + f121*dv121 + f211*dv211 + f221*dv221 +
                  f112*dv112 + f122*dv122 + f212*dv212 + f222*dv222;
    }

    if(pdiffz!=NULL) {
        if(iz==0){
            dv111 = v112 - v111;
            dv211 = v212 - v211;
            dv121 = v122 - v121;
            dv221 = v222 - v221;

        } else if(iz==nz-1){
            dv111 = v111 - values[idx - DZ];
            dv211 = v211 - values[idx + dx - DZ];
            dv121 = v121 - values[idx + dy - DZ];
            dv221 = v221 - values[idx + dx + dy - DZ];

        } else {
            dv111 = (values[idx + DZ] - values[idx - DZ])/2.0;
            dv211 = (values[idx + dx + DZ] - values[idx + dx - DZ])/2.0;
            dv121 = (values[idx + dy + DZ] - values[idx + dy - DZ])/2.0;
            dv221 = (values[idx + dx + dy + DZ] - values[idx + dx + dy - DZ])/2.0;

        }

        if(nz > 2 && iz < nz-2){
            dv112 = (values[idx + 2*DZ] - v111)/2.0;
            dv212 = (values[idx + dx + 2*DZ] - v211)/2.0;
            dv122 = (values[idx + dy + 2*DZ] - v121)/2.0;
            dv222 = (values[idx + dx + dy + 2*DZ] - v221)/2.0;
        } else if(nz > 1) {
            dv112 = (v111 - values[idx - 2*DZ])/2.0;
            dv212 = (v211 - values[idx + dx - 2*DZ])/2.0;
            dv122 = (v121 - values[idx + dy - 2*DZ])/2.0;
            dv222 = (v221 - values[idx + dx + dy - 2*DZ])/2.0;
        }

        *pdiffz = f111*dv111 + f121*dv121 + f211*dv211 + f221*dv221 +
                  f112*dv112 + f122*dv122 + f212*dv212 + f222*dv222;

    }
    

    return f111*v111 + f121*v121 + f211*v211 + f221*v221 +
            f112*v112 + f122*v122 + f212*v212 + f222*v222;
}







