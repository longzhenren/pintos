#ifndef FIXED_POINT
#define FIXED_POINT

typedef int fp_t;

#define ADD(A, B) (A + B)
#define SUB(A, B) (A - B)
#define MUL(A, B) ((fp_t)(((int64_t)A) * B >> 14))
#define DIV(A, B) ((fp_t)((((int64_t)A) << 14) / B))
#define IADD(A, B) (A + (B << 14))
#define ISUB(A, B) (A - (B << 14))
#define IMUL(A, B) (A * B)
#define IDIV(A, B) (A / B)
#define INT(A) (A >> 14)
#define CONST(A) ((fp_t)(A << 14))
#define ROUND(A) (A >= 0 ? ((A + (1 << (14)) / 2) >> 14) : ((A - (1 << (14)) / 2) >> 14))

#endif
