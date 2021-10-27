/* code by zzb start */
/* 17.14 fixed-point calc operations */
#ifndef __THREAD_FP_CALC_H
#define __THREAD_FP_CALC_H

typedef int fp_t;
#define FP_BITS 14

#define FP_CONST(A) ((fp_t)(A << FP_BITS))
#define FP_ADD(A, B) (A + B)
#define FP_ADD_INT(A, B) (A + (B << FP_BITS))
#define FP_SUB_FP(A, B) (A - B)
#define FP_SUB_INT(A, B) (A - (B << FP_BITS))
#define FP_MULT_INT(A, B) (A * B)
#define FP_DIV_INT(A, B) (A / B)
#define FP_MULT_FP(A, B) ((fp_t)(((int64_t)A) * B >> FP_BITS))
#define FP_DIV_FP(A, B) ((fp_t)((((int64_t)A) << FP_BITS) / B))
#define FP_INT_PART(A) (A >> FP_BITS)
#define FP_ROUND(A) (A >= 0 ? ((A + (1 << (FP_BITS - 1))) >> FP_BITS) : ((A - (1 << (FP_BITS - 1))) >> FP_BITS))

#endif
/* code by zzb end */