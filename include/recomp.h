#ifndef __RECOMP_H__
#define __RECOMP_H__

#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <setjmp.h>
#include <malloc.h>

#if 0 // treat GPRs as 32-bit, should be better codegen
typedef uint32_t gpr;

#define SIGNED(val) \
    ((int32_t)(val))
#else
typedef uint64_t gpr;

#define SIGNED(val) \
    ((int64_t)(val))
#endif

#define ADD32(a, b) \
    ((gpr)(int32_t)((a) + (b)))

#define SUB32(a, b) \
    ((gpr)(int32_t)((a) - (b)))

#define MEM_W(offset, reg) \
    (*(int32_t*)(rdram + ((((reg) + (offset))) - 0xFFFFFFFF80000000)))
    //(*(int32_t*)(rdram + ((((reg) + (offset))) & 0x3FFFFFF)))

#define MEM_H(offset, reg) \
    (*(int16_t*)(rdram + ((((reg) + (offset)) ^ 2) - 0xFFFFFFFF80000000)))
    //(*(int16_t*)(rdram + ((((reg) + (offset)) ^ 2) & 0x3FFFFFF)))

#define MEM_B(offset, reg) \
    (*(int8_t*)(rdram + ((((reg) + (offset)) ^ 3) - 0xFFFFFFFF80000000)))
    //(*(int8_t*)(rdram + ((((reg) + (offset)) ^ 3) & 0x3FFFFFF)))

#define MEM_HU(offset, reg) \
    (*(uint16_t*)(rdram + ((((reg) + (offset)) ^ 2) - 0xFFFFFFFF80000000)))
    //(*(uint16_t*)(rdram + ((((reg) + (offset)) ^ 2) & 0x3FFFFFF)))

#define MEM_BU(offset, reg) \
    (*(uint8_t*)(rdram + ((((reg) + (offset)) ^ 3) - 0xFFFFFFFF80000000)))
    //(*(uint8_t*)(rdram + ((((reg) + (offset)) ^ 3) & 0x3FFFFFF)))

#define SD(val, offset, reg) { \
    *(uint32_t*)(rdram + ((((reg) + (offset) + 4)) - 0xFFFFFFFF80000000)) = (uint32_t)((val) >> 0); \
    *(uint32_t*)(rdram + ((((reg) + (offset) + 0)) - 0xFFFFFFFF80000000)) = (uint32_t)((val) >> 32); \
}

//#define SD(val, offset, reg) { \
//    *(uint32_t*)(rdram + ((((reg) + (offset) + 4)) & 0x3FFFFFF)) = (uint32_t)((val) >> 32); \
//    *(uint32_t*)(rdram + ((((reg) + (offset) + 0)) & 0x3FFFFFF)) = (uint32_t)((val) >> 0); \
//}

static inline uint64_t load_doubleword(uint8_t* rdram, gpr reg, gpr offset) {
    uint64_t ret = 0;
    uint64_t lo = (uint64_t)(uint32_t)MEM_W(reg, offset + 4);
    uint64_t hi = (uint64_t)(uint32_t)MEM_W(reg, offset + 0);
    ret = (lo << 0) | (hi << 32);
    return ret;
}

#define LD(offset, reg) \
    load_doubleword(rdram, offset, reg)

// TODO proper lwl/lwr/swl/swr
static inline void do_swl(uint8_t* rdram, gpr offset, gpr reg, gpr val) {
    uint8_t byte0 = (uint8_t)(val >> 24);
    uint8_t byte1 = (uint8_t)(val >> 16);
    uint8_t byte2 = (uint8_t)(val >> 8);
    uint8_t byte3 = (uint8_t)(val >> 0);

    MEM_B(offset + 0, reg) = byte0;
    MEM_B(offset + 1, reg) = byte1;
    MEM_B(offset + 2, reg) = byte2;
    MEM_B(offset + 3, reg) = byte3;
}

static inline gpr do_lwl(uint8_t* rdram, gpr offset, gpr reg) {
    uint8_t byte0 = MEM_B(offset + 0, reg);
    uint8_t byte1 = MEM_B(offset + 1, reg);
    uint8_t byte2 = MEM_B(offset + 2, reg);
    uint8_t byte3 = MEM_B(offset + 3, reg);

    // Cast to int32_t to sign extend first
    return (gpr)(int32_t)((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | (byte3 << 0));
}

#define S32(val) \
    ((int32_t)(val))
    
#define U32(val) \
    ((uint32_t)(val))

#define S64(val) \
    ((int64_t)(val))

#define U64(val) \
    ((uint64_t)(val))

#define MUL_S(val1, val2) \
    ((val1) * (val2))

#define MUL_D(val1, val2) \
    ((val1) * (val2))

#define DIV_S(val1, val2) \
    ((val1) / (val2))

#define DIV_D(val1, val2) \
    ((val1) / (val2))

#define CVT_S_W(val) \
    ((float)((int32_t)(val)))

#define CVT_D_W(val) \
    ((double)((int32_t)(val)))

#define CVT_D_S(val) \
    ((double)(val))

#define CVT_S_D(val) \
    ((float)(val))

#define TRUNC_W_S(val) \
    ((int32_t)(val))

#define TRUNC_W_D(val) \
    ((int32_t)(val))

#define TRUNC_L_S(val) \
    ((int64_t)(val))

#define TRUNC_L_D(val) \
    ((int64_t)(val))

#define DEFAULT_ROUNDING_MODE 0

static inline int32_t do_cvt_w_s(float val, unsigned int rounding_mode) {
    switch (rounding_mode) {
        case 0: // round to nearest value
            return (int32_t)lroundf(val);
        case 1: // round to zero (truncate)
            return (int32_t)val;
        case 2: // round to positive infinity (ceil)
            return (int32_t)ceilf(val);
        case 3: // round to negative infinity (floor)
            return (int32_t)floorf(val);
    }
    assert(0);
    return 0;
}

#define CVT_W_S(val) \
    do_cvt_w_s(val, rounding_mode)

static inline int32_t do_cvt_w_d(double val, unsigned int rounding_mode) {
    switch (rounding_mode) {
        case 0: // round to nearest value
            return (int32_t)lround(val);
        case 1: // round to zero (truncate)
            return (int32_t)val;
        case 2: // round to positive infinity (ceil)
            return (int32_t)ceil(val);
        case 3: // round to negative infinity (floor)
            return (int32_t)floor(val);
    }
    assert(0);
    return 0;
}

#define CVT_W_D(val) \
    do_cvt_w_d(val, rounding_mode)

#define NAN_CHECK(val) \
    assert(val == val)

//#define NAN_CHECK(val)

typedef union {
    double d;
    struct {
        float fl;
        float fh;
    };
    struct {
        uint32_t u32l;
        uint32_t u32h;
    };
    uint64_t u64;
} fpr;

typedef struct {
    gpr r0,  r1,  r2,  r3,  r4,  r5,  r6,  r7,
        r8,  r9,  r10, r11, r12, r13, r14, r15,
        r16, r17, r18, r19, r20, r21, r22, r23,
        r24, r25, r26, r27, r28, r29, r30, r31;
    fpr f0,  f2,  f4,  f6,  f8,  f10, f12, f14,
        f16, f18, f20, f22, f24, f26, f28, f30;
    uint64_t hi, lo;
} recomp_context;

#ifdef __cplusplus
extern "C" {
#endif

void switch_error(const char* func, uint32_t vram, uint32_t jtbl);
void do_break(uint32_t vram);

typedef void (recomp_func_t)(uint8_t* rdram, recomp_context* ctx);

recomp_func_t* get_function(int32_t vram);

#define LOOKUP_FUNC(val) \
    get_function((int32_t)(val))

extern int32_t section_addresses[];

#define LO16(x) \
    ((x) & 0xFFFF)

#define HI16(x) \
    (((x) >> 16) + (((x) >> 15) & 1))

#define RELOC_HI16(section_index, offset) \
    HI16(section_addresses[section_index] + (offset))

#define RELOC_LO16(section_index, offset) \
    LO16(section_addresses[section_index] + (offset))

// For the Mario Party games (not working)
//// This has to be in this file so it can be inlined
//struct jmp_buf_storage {
//    jmp_buf buffer;
//};
//
//struct RecompJmpBuf {
//    int32_t owner;
//    struct jmp_buf_storage* storage;
//    uint64_t magic;
//};
//
//// Randomly generated constant
//#define SETJMP_MAGIC 0xe17afdfa939a437bu
//
//int32_t osGetThreadEx(void);
//
//#define setjmp_recomp(rdram, ctx) { \
//    struct RecompJmpBuf* buf = (struct RecompJmpBuf*)(&rdram[(uint64_t)ctx->r4 - 0xFFFFFFFF80000000]); \
//    \
//    /* Check if this jump buffer was previously set up */ \
//    if (buf->magic == SETJMP_MAGIC) { \
//        /* If so, free the old jmp_buf */ \
//        free(buf->storage); \
//    } \
//    \
//    buf->magic = SETJMP_MAGIC; \
//    buf->owner = osGetThreadEx(); \
//    buf->storage = (struct jmp_buf_storage*)calloc(1, sizeof(struct jmp_buf_storage)); \
//    ctx->r2 = setjmp(buf->storage->buffer); \
//}

#ifdef __cplusplus
}
#endif

#endif
