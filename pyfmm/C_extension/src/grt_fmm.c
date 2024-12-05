/**
 * @file   grt_fmm.c
 * @author Zhu Dengda (zhudengda@mail.iggcas.ac.cn)
 * @date   2024-11
 * 
 * 提供给GRT程序计算的辅助工具，计算对应全局初至波
 * 
 * GRT是计算1D层状半空间内的理论地震图的工具，详见
 *      https://github.com/Dengda98/PyGRT
 * 
*/


#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stddef.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>

#include "const.h"
#include "interp.h"
#include "fmm.h"
#include "sacio.h"


extern char *optarg;
extern int optind;
extern int optopt;

//****************** 在该文件以内的全局变量 ***********************//
// 命令名称
static char *command = NULL;
// 模型路径，模型PYMODEL1D指针，全局最大最小速度
static char *s_modelpath = NULL;
static char *s_modelname = NULL;
static int nlay=0;
static double *Dep1d = NULL;
static double *Vp1d = NULL;
static double *Vs1d = NULL;
static double minh = 9e30; // 模型最细厚度
// 三个维度的数组以及大小
static double *xs=NULL, *zs=NULL;
static int nx, nz, nxz;
static const double ys[1]={0.0};
static const int ny = 1;
// 震源坐标
static double xsrc=0.0, zsrc=0.0;
// 场点坐标
static double xrcv=0.0, zrcv=0.0;
// 慢度数组
static MYREAL *SlwP = NULL, *SlwS = NULL;
// 走时数组
static MYREAL *TTP = NULL, *TTS = NULL;
static int nTT=0;
// grt命令的输出路径, 以及符合要求的待处理文件夹路径
// 以及根据文件转化的震源深度、场点深度、震中距
static char *s_grtoutdir = NULL;
static int ndir=0;
static char **s_outsubdirs=NULL;
static double *depsrcs=NULL, *deprcvs=NULL, *epidists=NULL;
static double depmax=-1.0, epidistmax=-1.0;
// 在outsubdir中，要根据文件名中的非重复震源深度分配要计算多少个走时场，
// idx_nodup[i]反映的是depsrcs[i]第一次出现在depsrcs中的索引，长度为ndir
static int *idx_nodup=NULL; 
// 每个文件夹是否已计算对应的走时场，长度ndir
static bool *subdirs_hasTT=NULL;

// 各选项的标志变量，初始化为0，定义了则为1
static int X_flag=0, Z_flag=0, M_flag=0, O_flag=0;


static void print_help(){

}

/**
 * 从路径字符串中找到用/或\\分隔的最后一项
 * 
 * @param    path     路径字符串指针
 * 
 * @return   指向最后一项字符串的指针
 */
static char* get_basename(char* path) {
    // 找到最后一个 '/'
    char* last_slash = strrchr(path, '/'); 
    
#ifdef _WIN32
    char* last_backslash = strrchr(path, '\\');
    if (last_backslash && (!last_slash || last_backslash > last_slash)) {
        last_slash = last_backslash;
    }
#endif
    if (last_slash) {
        // 返回最后一个 '/' 之后的部分
        return last_slash + 1; 
    }
    // 如果没有 '/'，整个路径就是最后一项
    return path; 
}

/**
 * 找出double数组中某数第一次出现的索引
 * 
 */
static int find_index(double *arr, int n, double target){
    for(int i=0; i<n; ++i){
        if(arr[i] == target) return i;
    }
    return -1;
}


/**
 * 从命令行中读取选项，处理后记录到全局变量中
 * 
 * @param     argc      命令行的参数个数
 * @param     argv      多个参数字符串指针
 */
static void getopt_from_command(int argc, char **argv){
    int opt;
    while ((opt = getopt(argc, argv, ":M:X:Z:O:")) != -1) {
        switch (opt) {
            // grt命令使用的模型文件
            case 'M':
                M_flag = 1;
                s_modelpath = (char*)malloc(sizeof(char)*(strlen(optarg)+1));
                if(strcpy(s_modelpath, optarg) == NULL){
                    fprintf(stderr, "[%s] Error in -M.\n", command);
                    exit(EXIT_FAILURE);
                }
                s_modelname = get_basename(s_modelpath);
                break;
            
            // grt命令计算输出的目录
            case 'O':
                O_flag = 1;
                s_grtoutdir = (char*)malloc(sizeof(char)*(strlen(optarg)+1));
                if(strcpy(s_grtoutdir, optarg) == NULL){
                    fprintf(stderr, "[%s] Error in -O.\n", command);
                    exit(EXIT_FAILURE);
                }
                break;

            // X维度
            case 'X':
                X_flag = 1;
                double xe=-1.0, dx;
                if(2 != sscanf(optarg, "%lf/%d", &xe, &nx)){
                    fprintf(stderr, "[%s] Error in -X.\n", command);
                    exit(EXIT_FAILURE);
                }
                if(xe < 0.0 || nx < 0){
                    fprintf(stderr, "[%s] Nonpositive value in -X is not supported.\n", command);
                    exit(EXIT_FAILURE);
                }
                
                // 写数组
                xs = (double*)malloc(sizeof(double)*nx);
                dx = xe/nx;
                for(int i=0; i<nx; ++i){
                    xs[i] = i*dx;
                }
                break;

            // Z数组
            case 'Z':
                Z_flag = 1;
                double ze=-1.0, dz;
                if(2 != sscanf(optarg, "%lf/%d", &ze, &nz)){
                    fprintf(stderr, "[%s] Error in -Z.\n", command);
                    exit(EXIT_FAILURE);
                }
                if(ze < 0.0 || nz < 0){
                    fprintf(stderr, "[%s] Nonpositive value in -Z is not supported.\n", command);
                    exit(EXIT_FAILURE);
                }

                // 写数组
                zs = (double*)malloc(sizeof(double)*nz);
                dz = ze/nz;
                for(int i=0; i<nz; ++i){
                    zs[i] = i*dz;
                }
                break;
            
            // 帮助
            case 'h':
                print_help();
                exit(EXIT_SUCCESS);
                break;

            // 参数缺失
            case ':':
                fprintf(stderr, "[%s] Option '-%c' requires an argument. Use '-h' for help.\n", command, optopt);
                exit(EXIT_FAILURE);
                break;

            // 非法选项
            case '?':
            default:
                fprintf(stderr, "[%s] Option '-%c' is invalid. Use '-h' for help.\n", command, optopt);
                exit(EXIT_FAILURE);
                break;
        }
    }


    // 检查必须设置的参数是否有设置
    if(argc == 1){
        fprintf(stderr, "[%s] Need set options. Use '-h' for help.\n", command);
        exit(EXIT_FAILURE);
    }
    if(O_flag == 0){
        fprintf(stderr, "[%s] Need set -O. Use '-h' for help.\n", command);
        exit(EXIT_FAILURE);
    }
    // if(X_flag == 0){
    //     fprintf(stderr, "[%s] Need set -X. Use '-h' for help.\n", command);
    //     exit(EXIT_FAILURE);
    // }
    // if(Z_flag == 0){
    //     fprintf(stderr, "[%s] Need set -Z. Use '-h' for help.\n", command);
    //     exit(EXIT_FAILURE);
    // }
    if(M_flag == 0){
        fprintf(stderr, "[%s] Need set -M. Use '-h' for help.\n", command);
        exit(EXIT_FAILURE);
    }

}


/**
 * 判断一个字符串是否是浮点数或整数
 * 
 * @param    str    字符串指针 
 * 
 * @return   是否是数字
 */ 
static bool is_number(const char *str) {
    int dot_count = 0;
    
    // 遍历字符串的每个字符
    while (*str) {
        if (*str == '.') {
            dot_count++;
        } else if (!isdigit(*str) && *str != '-') {
            return false;  // 非数字字符
        }
        str++;
    }
    
    // 浮点数只能有一个小数点
    return dot_count <= 1;
}

/**
 * 判断文件夹名是否符合 {modelname}_{depsrc}_{deprcv}_{r} 格式
 * 
 * @param    folder_name    待检查的文件夹名
 * @param    depsrc         提取的震源深度
 * @param    depsrc         提取的场点深度
 * @param    epidist        提取的震中距
 */
int match_folder_name(const char *folder_name, double *depsrc, double *deprcv, double *epidist) {
    char *name_copy;
    char *token;
    int ntoken = 0;
    
    // 复制文件夹名，以便分割
    name_copy = strdup(folder_name);

    // 分割文件夹名，按下划线分割
    token = strtok(name_copy, "_");
    int istr=0;
    bool checkstr_done = false;
    int findnumber = 0;
    double *pnumber[3] = {depsrc, deprcv, epidist};
    while (token != NULL) {
        ntoken++;
        if (!checkstr_done) {
            // 第一部分是 str，假设它是有效的字符串（包含字母或下划线）
            for (int i = 0; token[i] != '\0'; i++) {
                if (token[i]!=s_modelname[istr] && token[i] != '_') {
                    return 0;  // 如果包含无效字符，返回0
                }
                istr++;
                if(s_modelname[istr]=='\0') checkstr_done=true;
            }
        } else if (findnumber < 3) {
            // 后三部分是数字（整数或浮点数）
            if (!is_number(token)) {
                return 0;  // 如果不是数字，返回0
            }
            *pnumber[findnumber] = atof(token);
            findnumber++;
        } else {
            // 超过4，格式不匹配
            return 0;
        }
        
        // 获取下一个部分
        token = strtok(NULL, "_");
    }

    free(name_copy);

    // 检查是否有4部分
    return ntoken == 4;
}


/**
 * 筛选符合命令要求的文件夹
 * 
 * @param    indir     父级目录
 */
void list_matching_folders(const char *indir) {
    DIR *dir;
    struct dirent *entry;
    int lenmax = strlen(indir)+strlen(s_modelpath)+100;
    char folder_path[lenmax];

    if ((dir = opendir(indir)) == NULL) {
        fprintf(stderr, "[%s] Error when open %s.\n", command, indir);
        exit(EXIT_FAILURE);
    }

    ndir = 0;
    while ((entry = readdir(dir)) != NULL) {
        // 跳过 "." 和 ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // 构建文件夹的完整路径
        snprintf(folder_path, sizeof(folder_path), "%s/%s", indir, entry->d_name);

        // 判断是否是文件夹
        if (opendir(folder_path) != NULL) {
            // 检查文件夹名称是否符合模式
            double f1, f2, f3;
            if (match_folder_name(entry->d_name, &f1, &f2, &f3)) {
                printf("Find Matching folder: %s\n", entry->d_name);
                // 写入数组
                s_outsubdirs = (char**)realloc(s_outsubdirs, sizeof(char*)*(ndir+1));
                s_outsubdirs[ndir] = NULL;
                s_outsubdirs[ndir] = (char*)realloc(s_outsubdirs[ndir], sizeof(char)*lenmax);
                strcpy(s_outsubdirs[ndir], folder_path);
                
                depsrcs = (double*)realloc(depsrcs, sizeof(double)*(ndir+1));
                depsrcs[ndir] = f1;
                deprcvs = (double*)realloc(deprcvs, sizeof(double)*(ndir+1));
                deprcvs[ndir] = f2;
                epidists = (double*)realloc(epidists, sizeof(double)*(ndir+1));
                epidists[ndir] = f3;
                idx_nodup = (int*)realloc(idx_nodup, sizeof(int)*(ndir+1));
                idx_nodup[ndir] = find_index(depsrcs, ndir+1, f1);
                subdirs_hasTT = (bool*)realloc(subdirs_hasTT, sizeof(bool)*(nlay+1));
                subdirs_hasTT[ndir] = false;

                if(depmax < f1)  depmax=f1;
                if(depmax < f2)  depmax=f2;
                if(epidistmax < f3)  epidistmax = f3;
                
                ndir++;

            }
        }
    }

    closedir(dir);
}


/**
 * 从1D模型文件中读取Vp、Vs、Depth
 */
static int load_model1D(){
    FILE *fp;
    if((fp = fopen(s_modelpath, "r")) == NULL){
        fprintf(stderr, "[%s] Model file open error.\n", command);
        return -1;
    }

    char line[1024];
    int iline = 0;
    double h, va, vb, rho, qa, qb;
    h = va = vb = rho = qa = qb = -9.0;
    nlay = 0;
    double dep=0.0;
    while(fgets(line, sizeof(line), fp)) {
        iline++;
        if(h > 0.0 && h < minh)  minh = h;

        h = va = vb = rho = qa = qb = -9.0;
        if(6 != sscanf(line, "%lf %lf %lf %lf %lf %lf\n", &h, &va, &vb, &rho, &qa, &qb)){
            fprintf(stderr, "[%s] Model file read error in line %d.\n", command, iline);
            return -1;
        };

        if(h < 0.0 || va < 0.0 || vb < 0.0 || rho < 0.0 || qa < 0.0 || qb < 0.0){
            fprintf(stderr, "[%s] In line %d, negative value is not supported.\n", command, iline);
            return -1;
        }


        Vp1d = (double*)realloc(Vp1d, sizeof(double)*(nlay+1));
        Vs1d = (double*)realloc(Vs1d, sizeof(double)*(nlay+1));
        Dep1d = (double*)realloc(Dep1d, sizeof(double)*(nlay+1));
        Vp1d[nlay] = va;
        Vs1d[nlay] = vb;
        Dep1d[nlay] = dep;

        nlay++;
        dep += h;
    }

    if(iline==0 || Vp1d==NULL){
        fprintf(stderr, "[%s] Model file read error.\n", command);
        return -1;
    }



    return 0;
}

/**
 * 根据设置的1D速度离散得到差分网格对应的速度
 * 
 * @param     Vel1d    1D速度
 * @param     Slw      2D差分网格慢度
 */
static void interp1d_vel(const double *Vel1d, MYREAL *Slw){
    for(int iz=0; iz<nz; ++iz){
        double v0;
        if(nlay==1){
            v0 = Vel1d[0];
        } else {
            // 找到索引值
            double dep = zs[iz];
            int j=0; 
            for(j=0; j<nlay; ++j){
                v0 = Vel1d[j];
                if(Dep1d[j] <= dep && dep < Dep1d[j+1]){
                    break;
                } 
            }
        }

        if(v0 == 0.0)  v0 += 1e-5;

        // 扩展到X维
        for(int ix=0; ix<nx; ++ix){
            Slw[iz+ix*nz] = 1.0/v0;
        }
    }
}

/**
 * 将计算好的走时写到sac头文件中
 * 
 * @param   subdir    某个子文件夹路径
 * @param   travtP    计算的初至P波走时
 * @param   travtS    计算的初至S波走时
 */
static void write_travt_to_sac(const char *subdir, double travtP, double travtS){
    DIR *dir;
    struct dirent *entry;
    char *fullpath = (char*)malloc(sizeof(char)*(strlen(subdir)+100));

    if ((dir = opendir(subdir)) == NULL) {
        fprintf(stderr, "[%s] Error when open %s.\n", command, subdir);
        exit(EXIT_FAILURE);
    }

    SACHEAD hd;
    while ((entry = readdir(dir)) != NULL) {
        // 跳过 "." 和 ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        sprintf(fullpath, "%s/%s", subdir, entry->d_name);

        // 读sac文件
        float *arr = read_sac(fullpath, &hd);
        // 写入走时
        hd.t0 = hd.o + travtP;
        strcpy(hd.kt0, "P");
        hd.t1 = hd.o + travtS;
        strcpy(hd.kt1, "S");
        // 写回sac
        write_sac(fullpath, hd, arr);

        free(arr);
    }

}



//========================================================================
//========================================================================
//========================================================================
int main(int argc, char **argv){
    command = argv[0];
    getopt_from_command(argc, argv);
    
    // 提取符合条件的文件夹
    list_matching_folders(s_grtoutdir);

    // 读入一维模型 
    if(load_model1D() < 0){
        exit(EXIT_FAILURE);
    }

    // 自动设置Z网格
    if(Z_flag == 0){
        double ze = depmax, dz = minh/5.0;
        
    }

    // 自动设置X网格
    if(X_flag == 0){
        double xe = epidistmax; 
    }


    // 计算维度 
    nxz = nx*nz;
    SlwP = (MYREAL*)malloc(sizeof(MYREAL)*nxz);
    SlwS = (MYREAL*)malloc(sizeof(MYREAL)*nxz);

    // 插值速度场
    interp1d_vel(Vp1d, SlwP);
    interp1d_vel(Vs1d, SlwS);

    // for(int ix=0; ix<nx; ++ix){
    //     printf("%8.2f ", xs[ix]);
    // }
    // printf("\n");
    // for(int iz=0; iz<nz; ++iz){
    //     for(int ix=0; ix<nx; ++ix){
    //         printf("%8.2f ", zs[iz]);
    //         printf("%8.2f ", 1.0/SlwS[iz+ix*nz]);
    //     }
    //     printf("\n");
    // }


    for(int idir=0; idir<ndir; ++idir){
        TTP = NULL;
        TTS = NULL;
        int isrc = idx_nodup[idir];

        // 该深度的震源已经计算过 
        if(subdirs_hasTT[idir]) continue;

        // 计算走时场, 初始化全为0
        TTP = (MYREAL*)calloc(sizeof(MYREAL), nxz);
        TTS = (MYREAL*)calloc(sizeof(MYREAL), nxz);

        printf("isrc=%d, depsrc=%f, nxz=%d\n", isrc, depsrcs[isrc], nxz);        
        FastMarching(xs, nx, ys, ny, zs, nz, 0.0, 0.0, depsrcs[isrc], 2, SlwP, TTP, false, 0, 0, false);
        FastMarching(xs, nx, ys, ny, zs, nz, 0.0, 0.0, depsrcs[isrc], 2, SlwS, TTS, false, 0, 0, false);

        // 给适合该走时的格林函数写入走时
        for(int kdir=idir; kdir<ndir; ++kdir){
            if(idir != idx_nodup[kdir]) continue;

            printf("%s, depsrc=%f, deprcv=%f, r=%f\n", s_outsubdirs[kdir], depsrcs[kdir], deprcvs[kdir], epidists[kdir]);
            MYREAL travtP = trilinear_one_ravel(xs, nx, ys, ny, zs, nz, nz, TTP, epidists[kdir], 0.0, deprcvs[kdir], 
                                                NULL, NULL, NULL, NULL, NULL);
            MYREAL travtS = trilinear_one_ravel(xs, nx, ys, ny, zs, nz, nz, TTS, epidists[kdir], 0.0, deprcvs[kdir], 
                                                NULL, NULL, NULL, NULL, NULL);
            printf("travtP=%f, travtS=%f\n", travtP, travtS);

            // 写入SAC
            write_travt_to_sac(s_outsubdirs[kdir], travtP, travtS);

            subdirs_hasTT[kdir] = true;
        }

        free(TTP); 
        free(TTS);

        nTT += 1;
    }

    // for(int iz=0; iz<nz; ++iz){
    //     for(int ix=0; ix<nx; ++ix){
    //         printf("%8.2e ", TTPs[0][iz+ix*nz]);
    //     }
    //     printf("\n");
    // }

    // printf("%d\n", ndir);
    // for(int i=0; i<ndir; ++i){
    //     printf("%d ", idx_nodup[i]);
    // }
    // printf("\n");

    
    

    free(s_modelpath);
    free(s_grtoutdir);
    free(xs);
    free(zs);
    free(Vp1d); free(Vs1d); free(Dep1d);
    free(SlwP); free(SlwS);
    for(int idir=0; idir<ndir; ++idir){
        free(s_outsubdirs[idir]);
    }
    free(s_outsubdirs);
    free(depsrcs); free(deprcvs); free(epidists);
    free(idx_nodup);
    free(subdirs_hasTT);
}