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

#include "const.h"
#include "interp.h"
#include "index.h"
#include "query.h"



MYREAL trilinear_one_ravel(
    const double *x, int nx, const double *y, int ny, const double *z, int nz, int nyz, const MYREAL *values, 
    double xi, double yi, double zi, double *pdiffx, double *pdiffy, double *pdiffz, 
    int IXYZ[6], double WGHT[2][2][2])
{
    int IXYZ0[6];
    double WGHT0[2][2][2];
    trilinear_one_fac(x, nx, y, ny, z, nz, xi, yi, zi, IXYZ0, WGHT0);

    double vi;
    vi = trilinear_one_Idx_ravel(IXYZ0, WGHT0, values, nx, ny, nz, nyz, pdiffx, pdiffy, pdiffz);

    if(IXYZ!=NULL){
        for(int i=0; i<6; ++i){
            IXYZ[i] = IXYZ0[i];
        }
    } 
    if(WGHT!=NULL){
        for(int i=0; i<2; ++i){
            for(int j=0; j<2; ++j){
                for(int k=0; k<2; ++k){
                    WGHT[i][j][k] = WGHT0[i][j][k];
                }
            }
        }
    }

    return vi;
}



void trilinear_one_fac(
    const double *x, int nx, const double *y, int ny, const double *z, int nz, 
    double xi, double yi, double zi, int IXYZ[6], double WGHT[2][2][2])
{
    int ix = dicho_find(x, nx, xi);
    // int ix1 = (ix+1>nx-1) ? nx-1 : ix+1;
    int iy = dicho_find(y, ny, yi);
    // int iy1 = (iy+1>ny-1) ? ny-1 : iy+1;
    int iz = dicho_find(z, nz, zi);
    // int iz1 = (iz+1>nz-1) ? nz-1 : iz+1;
    int ix1, iy1, iz1;
    if(IXYZ!=NULL && IXYZ[0]==-9){ // do extrapolation
        if(ix==nx-1) ix--; 
        ix1 = ix+1;
        if(iy==ny-1) iy--; 
        iy1 = iy+1;
        if(iz==nz-1) iz--; 
        iz1 = iz+1;
    } else {
        ix1 = (ix+1>nx-1) ? nx-1 : ix+1;
        iy1 = (iy+1>ny-1) ? ny-1 : iy+1;
        iz1 = (iz+1>nz-1) ? nz-1 : iz+1;
    }

    // printf("%d,%d,%d, %f,%f,%f\n", ix, iy, iz, xi, yi, zi);
    double x1 = x[ix];
    double x2 = x[ix1];
    double y1 = y[iy];
    double y2 = y[iy1];
    double z1 = z[iz];
    double z2 = z[iz1];

    // 限制范围
    if(IXYZ==NULL || IXYZ[0]!=-9){
        if(xi > x2) xi = x2;
        if(yi > y2) yi = y2;
        if(zi > z2) zi = z2;
    }

    double xfac=0.0, yfac=0.0, zfac=0.0, xfac1, yfac1, zfac1;
    if(ix!=ix1) xfac = (xi-x1)/(x2-x1);
    if(iy!=iy1) yfac = (yi-y1)/(y2-y1);
    if(iz!=iz1) zfac = (zi-z1)/(z2-z1);

    xfac1 = 1.0 - xfac; 
    yfac1 = 1.0 - yfac; 
    zfac1 = 1.0 - zfac; 

    double f111, f121, f211, f221, f112, f122, f212, f222;
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


MYREAL trilinear_one_Idx_ravel(
    const int IXYZ[6], const double WGHT[2][2][2],  const MYREAL *values, int nx, int ny, int nz, int nyz, 
    double *pdiffx, double *pdiffy, double *pdiffz)
{
    int ix, ix1, iy, iy1, iz, iz1;
    double f111, f121, f211, f221, f112, f122, f212, f222;
    double v111,v121,v211,v221, v112,v122,v212,v222;

    ix  = IXYZ[0]; 
    ix1 = IXYZ[1]; 
    iy  = IXYZ[2]; 
    iy1 = IXYZ[3]; 
    iz  = IXYZ[4]; 
    iz1 = IXYZ[5]; 


    int idx;
    ravel_index(&idx, nyz, nz, ix, iy, iz);
    int dx, dy, dz, DX, DY, DZ;
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

    double dv111,dv121,dv211,dv221, dv112,dv122,dv212,dv222;
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

        if(ix < nx-2){
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

        if(iy < ny-2){
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

        if(iz < nz-2){
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







