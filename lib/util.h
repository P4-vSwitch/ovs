/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UTIL_H
#define UTIL_H 1

#include <arpa/inet.h>
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "openvswitch/types.h"
#include "openvswitch/util.h"

#ifndef va_copy
#ifdef __va_copy
#define va_copy __va_copy
#else
#define va_copy(dst, src) ((dst) = (src))
#endif
#endif

#ifdef __CHECKER__
#define BUILD_ASSERT(EXPR) ((void) 0)
#define BUILD_ASSERT_DECL(EXPR) extern int (*build_assert(void))[1]
#elif !defined(__cplusplus)
/* Build-time assertion building block. */
#define BUILD_ASSERT__(EXPR) \
        sizeof(struct { unsigned int build_assert_failed : (EXPR) ? 1 : -1; })

/* Build-time assertion for use in a statement context. */
#define BUILD_ASSERT(EXPR) (void) BUILD_ASSERT__(EXPR)

/* Build-time assertion for use in a declaration context. */
#define BUILD_ASSERT_DECL(EXPR) \
        extern int (*build_assert(void))[BUILD_ASSERT__(EXPR)]
#else /* __cplusplus */
#include <boost/static_assert.hpp>
#define BUILD_ASSERT BOOST_STATIC_ASSERT
#define BUILD_ASSERT_DECL BOOST_STATIC_ASSERT
#endif /* __cplusplus */

#ifdef __GNUC__
#define BUILD_ASSERT_GCCONLY(EXPR) BUILD_ASSERT(EXPR)
#define BUILD_ASSERT_DECL_GCCONLY(EXPR) BUILD_ASSERT_DECL(EXPR)
#else
#define BUILD_ASSERT_GCCONLY(EXPR) ((void) 0)
#define BUILD_ASSERT_DECL_GCCONLY(EXPR) ((void) 0)
#endif

/* Like the standard assert macro, except writes the failure message to the
 * log. */
#ifndef NDEBUG
#define ovs_assert(CONDITION)                                           \
    if (!OVS_LIKELY(CONDITION)) {                                       \
        ovs_assert_failure(OVS_SOURCE_LOCATOR, __func__, #CONDITION);       \
    }
#else
#define ovs_assert(CONDITION) ((void) (CONDITION))
#endif
OVS_NO_RETURN void ovs_assert_failure(const char *, const char *, const char *);

/* Casts 'pointer' to 'type' and issues a compiler warning if the cast changes
 * anything other than an outermost "const" or "volatile" qualifier.
 *
 * The cast to int is present only to suppress an "expression using sizeof
 * bool" warning from "sparse" (see
 * http://permalink.gmane.org/gmane.comp.parsers.sparse/2967). */
#define CONST_CAST(TYPE, POINTER)                               \
    ((void) sizeof ((int) ((POINTER) == (TYPE) (POINTER))),     \
     (TYPE) (POINTER))

extern char *program_name;

#define __ARRAY_SIZE_NOCHECK(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))
#ifdef __GNUC__
/* return 0 for array types, 1 otherwise */
#define __ARRAY_CHECK(ARRAY) 					\
    !__builtin_types_compatible_p(typeof(ARRAY), typeof(&ARRAY[0]))

/* compile-time fail if not array */
#define __ARRAY_FAIL(ARRAY) (sizeof(char[-2*!__ARRAY_CHECK(ARRAY)]))
#define __ARRAY_SIZE(ARRAY)					\
    __builtin_choose_expr(__ARRAY_CHECK(ARRAY),			\
        __ARRAY_SIZE_NOCHECK(ARRAY), __ARRAY_FAIL(ARRAY))
#else
#define __ARRAY_SIZE(ARRAY) __ARRAY_SIZE_NOCHECK(ARRAY)
#endif

/* Returns the number of elements in ARRAY. */
#define ARRAY_SIZE(ARRAY) __ARRAY_SIZE(ARRAY)

/* Returns X / Y, rounding up.  X must be nonnegative to round correctly. */
#define DIV_ROUND_UP(X, Y) (((X) + ((Y) - 1)) / (Y))

/* Returns X rounded up to the nearest multiple of Y. */
#define ROUND_UP(X, Y) (DIV_ROUND_UP(X, Y) * (Y))

/* Returns the least number that, when added to X, yields a multiple of Y. */
#define PAD_SIZE(X, Y) (ROUND_UP(X, Y) - (X))

/* Returns X rounded down to the nearest multiple of Y. */
#define ROUND_DOWN(X, Y) ((X) / (Y) * (Y))

/* Returns true if X is a power of 2, otherwise false. */
#define IS_POW2(X) ((X) && !((X) & ((X) - 1)))

static inline bool
is_pow2(uintmax_t x)
{
    return IS_POW2(x);
}

/* Returns X rounded up to a power of 2.  X must be a constant expression. */
#define ROUND_UP_POW2(X) RUP2__(X)
#define RUP2__(X) (RUP2_1(X) + 1)
#define RUP2_1(X) (RUP2_2(X) | (RUP2_2(X) >> 16))
#define RUP2_2(X) (RUP2_3(X) | (RUP2_3(X) >> 8))
#define RUP2_3(X) (RUP2_4(X) | (RUP2_4(X) >> 4))
#define RUP2_4(X) (RUP2_5(X) | (RUP2_5(X) >> 2))
#define RUP2_5(X) (RUP2_6(X) | (RUP2_6(X) >> 1))
#define RUP2_6(X) ((X) - 1)

/* Returns X rounded down to a power of 2.  X must be a constant expression. */
#define ROUND_DOWN_POW2(X) RDP2__(X)
#define RDP2__(X) (RDP2_1(X) - (RDP2_1(X) >> 1))
#define RDP2_1(X) (RDP2_2(X) | (RDP2_2(X) >> 16))
#define RDP2_2(X) (RDP2_3(X) | (RDP2_3(X) >> 8))
#define RDP2_3(X) (RDP2_4(X) | (RDP2_4(X) >> 4))
#define RDP2_4(X) (RDP2_5(X) | (RDP2_5(X) >> 2))
#define RDP2_5(X) (      (X) | (      (X) >> 1))

/* This system's cache line size, in bytes.
 * Being wrong hurts performance but not correctness. */
#define CACHE_LINE_SIZE 64
BUILD_ASSERT_DECL(IS_POW2(CACHE_LINE_SIZE));

static inline void
ovs_prefetch_range(const void *start, size_t size)
{
    const char *addr = (const char *)start;
    size_t ofs;

    for (ofs = 0; ofs < size; ofs += CACHE_LINE_SIZE) {
        OVS_PREFETCH(addr + ofs);
    }
}

#ifndef MIN
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifndef MAX
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#endif

#define OVS_NOT_REACHED() abort()

/* Given a pointer-typed lvalue OBJECT, expands to a pointer type that may be
 * assigned to OBJECT. */
#ifdef __GNUC__
#define OVS_TYPEOF(OBJECT) typeof(OBJECT)
#else
#define OVS_TYPEOF(OBJECT) void *
#endif

/* Given OBJECT of type pointer-to-structure, expands to the offset of MEMBER
 * within an instance of the structure.
 *
 * The GCC-specific version avoids the technicality of undefined behavior if
 * OBJECT is null, invalid, or not yet initialized.  This makes some static
 * checkers (like Coverity) happier.  But the non-GCC version does not actually
 * dereference any pointer, so it would be surprising for it to cause any
 * problems in practice.
 */
#ifdef __GNUC__
#define OBJECT_OFFSETOF(OBJECT, MEMBER) offsetof(typeof(*(OBJECT)), MEMBER)
#else
#define OBJECT_OFFSETOF(OBJECT, MEMBER) \
    ((char *) &(OBJECT)->MEMBER - (char *) (OBJECT))
#endif

/* Yields the size of MEMBER within STRUCT. */
#define MEMBER_SIZEOF(STRUCT, MEMBER) (sizeof(((STRUCT *) NULL)->MEMBER))

/* Given POINTER, the address of the given MEMBER in a STRUCT object, returns
   the STRUCT object. */
#define CONTAINER_OF(POINTER, STRUCT, MEMBER)                           \
        ((STRUCT *) (void *) ((char *) (POINTER) - offsetof (STRUCT, MEMBER)))

/* Given POINTER, the address of the given MEMBER within an object of the type
 * that that OBJECT points to, returns OBJECT as an assignment-compatible
 * pointer type (either the correct pointer type or "void *").  OBJECT must be
 * an lvalue.
 *
 * This is the same as CONTAINER_OF except that it infers the structure type
 * from the type of '*OBJECT'. */
#define OBJECT_CONTAINING(POINTER, OBJECT, MEMBER)                      \
    ((OVS_TYPEOF(OBJECT)) (void *)                                      \
     ((char *) (POINTER) - OBJECT_OFFSETOF(OBJECT, MEMBER)))

/* Given POINTER, the address of the given MEMBER within an object of the type
 * that that OBJECT points to, assigns the address of the outer object to
 * OBJECT, which must be an lvalue.
 *
 * Evaluates to (void) 0 as the result is not to be used. */
#define ASSIGN_CONTAINER(OBJECT, POINTER, MEMBER) \
    ((OBJECT) = OBJECT_CONTAINING(POINTER, OBJECT, MEMBER), (void) 0)

/* As explained in the comment above OBJECT_OFFSETOF(), non-GNUC compilers
 * like MSVC will complain about un-initialized variables if OBJECT
 * hasn't already been initialized. To prevent such warnings, INIT_CONTAINER()
 * can be used as a wrapper around ASSIGN_CONTAINER. */
#define INIT_CONTAINER(OBJECT, POINTER, MEMBER) \
    ((OBJECT) = NULL, ASSIGN_CONTAINER(OBJECT, POINTER, MEMBER))

/* Given ATTR, and TYPE, cast the ATTR to TYPE by first casting ATTR to
 * (void *). This is to suppress the alignment warning issued by clang. */
#define ALIGNED_CAST(TYPE, ATTR) ((TYPE) (void *) (ATTR))

/* Use "%"PRIuSIZE to format size_t with printf(). */
#ifdef _WIN32
#define PRIdSIZE "Id"
#define PRIiSIZE "Ii"
#define PRIoSIZE "Io"
#define PRIuSIZE "Iu"
#define PRIxSIZE "Ix"
#define PRIXSIZE "IX"
#else
#define PRIdSIZE "zd"
#define PRIiSIZE "zi"
#define PRIoSIZE "zo"
#define PRIuSIZE "zu"
#define PRIxSIZE "zx"
#define PRIXSIZE "zX"
#endif

#ifndef _WIN32
typedef uint32_t HANDLE;
#endif

#ifdef  __cplusplus
extern "C" {
#endif

#define set_program_name(name) \
        ovs_set_program_name(name, OVS_PACKAGE_VERSION)

const char *get_subprogram_name(void);
    void set_subprogram_name(const char *);

void ovs_print_version(uint8_t min_ofp, uint8_t max_ofp);

OVS_NO_RETURN void out_of_memory(void);
void *xmalloc(size_t) MALLOC_LIKE;
void *xcalloc(size_t, size_t) MALLOC_LIKE;
void *xzalloc(size_t) MALLOC_LIKE;
void *xrealloc(void *, size_t);
void *xmemdup(const void *, size_t) MALLOC_LIKE;
char *xmemdup0(const char *, size_t) MALLOC_LIKE;
char *xstrdup(const char *) MALLOC_LIKE;
char *xasprintf(const char *format, ...) OVS_PRINTF_FORMAT(1, 2) MALLOC_LIKE;
char *xvasprintf(const char *format, va_list) OVS_PRINTF_FORMAT(1, 0) MALLOC_LIKE;
void *x2nrealloc(void *p, size_t *n, size_t s);

void *xmalloc_cacheline(size_t) MALLOC_LIKE;
void *xzalloc_cacheline(size_t) MALLOC_LIKE;
void free_cacheline(void *);

void ovs_strlcpy(char *dst, const char *src, size_t size);
void ovs_strzcpy(char *dst, const char *src, size_t size);

OVS_NO_RETURN void ovs_abort(int err_no, const char *format, ...)
    OVS_PRINTF_FORMAT(2, 3);
OVS_NO_RETURN void ovs_abort_valist(int err_no, const char *format, va_list)
    OVS_PRINTF_FORMAT(2, 0);
OVS_NO_RETURN void ovs_fatal(int err_no, const char *format, ...)
    OVS_PRINTF_FORMAT(2, 3);
OVS_NO_RETURN void ovs_fatal_valist(int err_no, const char *format, va_list)
    OVS_PRINTF_FORMAT(2, 0);
void ovs_error(int err_no, const char *format, ...) OVS_PRINTF_FORMAT(2, 3);
void ovs_error_valist(int err_no, const char *format, va_list)
    OVS_PRINTF_FORMAT(2, 0);
const char *ovs_retval_to_string(int);
const char *ovs_strerror(int);
void ovs_hex_dump(FILE *, const void *, size_t, uintptr_t offset, bool ascii);

bool str_to_int(const char *, int base, int *);
bool str_to_long(const char *, int base, long *);
bool str_to_llong(const char *, int base, long long *);
bool str_to_uint(const char *, int base, unsigned int *);

bool ovs_scan(const char *s, const char *format, ...) OVS_SCANF_FORMAT(2, 3);
bool ovs_scan_len(const char *s, int *n, const char *format, ...);

bool str_to_double(const char *, double *);

int hexit_value(int c);
uintmax_t hexits_value(const char *s, size_t n, bool *ok);

int parse_int_string(const char *s, uint8_t *valuep, int field_width,
                     char **tail);

const char *english_list_delimiter(size_t index, size_t total);

char *get_cwd(void);
#ifndef _WIN32
char *dir_name(const char *file_name);
char *base_name(const char *file_name);
#endif
char *abs_file_name(const char *dir, const char *file_name);

char *follow_symlinks(const char *filename);

void ignore(bool x OVS_UNUSED);

/* Bitwise tests. */

/* Returns the number of trailing 0-bits in 'n'.  Undefined if 'n' == 0. */
#if __GNUC__ >= 4
static inline int
raw_ctz(uint64_t n)
{
    /* With GCC 4.7 on 32-bit x86, if a 32-bit integer is passed as 'n', using
     * a plain __builtin_ctzll() here always generates an out-of-line function
     * call.  The test below helps it to emit a single 'bsf' instruction. */
    return (__builtin_constant_p(n <= UINT32_MAX) && n <= UINT32_MAX
            ? __builtin_ctz(n)
            : __builtin_ctzll(n));
}

static inline int
raw_clz64(uint64_t n)
{
    return __builtin_clzll(n);
}
#elif _MSC_VER
static inline int
raw_ctz(uint64_t n)
{
#ifdef _WIN64
    unsigned long r = 0;
    _BitScanForward64(&r, n);
    return r;
#else
    unsigned long low = n, high, r = 0;
    if (_BitScanForward(&r, low)) {
        return r;
    }
    high = n >> 32;
    _BitScanForward(&r, high);
    return r + 32;
#endif
}

static inline int
raw_clz64(uint64_t n)
{
#ifdef _WIN64
    unsigned long r = 0;
    _BitScanReverse64(&r, n);
    return 63 - r;
#else
    unsigned long low, high = n >> 32, r = 0;
    if (_BitScanReverse(&r, high)) {
        return 31 - r;
    }
    low = n;
    _BitScanReverse(&r, low);
    return 63 - r;
#endif
}
#else
/* Defined in util.c. */
int raw_ctz(uint64_t n);
int raw_clz64(uint64_t n);
#endif

/* Returns the number of trailing 0-bits in 'n', or 32 if 'n' is 0. */
static inline int
ctz32(uint32_t n)
{
    return n ? raw_ctz(n) : 32;
}

/* Returns the number of trailing 0-bits in 'n', or 64 if 'n' is 0. */
static inline int
ctz64(uint64_t n)
{
    return n ? raw_ctz(n) : 64;
}

/* Returns the number of leading 0-bits in 'n', or 32 if 'n' is 0. */
static inline int
clz32(uint32_t n)
{
    return n ? raw_clz64(n) - 32 : 32;
}

/* Returns the number of leading 0-bits in 'n', or 64 if 'n' is 0. */
static inline int
clz64(uint64_t n)
{
    return n ? raw_clz64(n) : 64;
}

/* Given a word 'n', calculates floor(log_2('n')).  This is equivalent
 * to finding the bit position of the most significant one bit in 'n'.  It is
 * an error to call this function with 'n' == 0. */
static inline int
log_2_floor(uint64_t n)
{
    return 63 - raw_clz64(n);
}

/* Given a word 'n', calculates ceil(log_2('n')).  It is an error to
 * call this function with 'n' == 0. */
static inline int
log_2_ceil(uint64_t n)
{
    return log_2_floor(n) + !is_pow2(n);
}

/* unsigned int count_1bits(uint64_t x):
 *
 * Returns the number of 1-bits in 'x', between 0 and 64 inclusive. */
#if UINTPTR_MAX == UINT64_MAX
static inline unsigned int
count_1bits(uint64_t x)
{
#if __GNUC__ >= 4 && __POPCNT__
    return __builtin_popcountll(x);
#else
    /* This portable implementation is the fastest one we know of for 64
     * bits, and about 3x faster than GCC 4.7 __builtin_popcountll(). */
    const uint64_t h55 = UINT64_C(0x5555555555555555);
    const uint64_t h33 = UINT64_C(0x3333333333333333);
    const uint64_t h0F = UINT64_C(0x0F0F0F0F0F0F0F0F);
    const uint64_t h01 = UINT64_C(0x0101010101010101);
    x -= (x >> 1) & h55;               /* Count of each 2 bits in-place. */
    x = (x & h33) + ((x >> 2) & h33);  /* Count of each 4 bits in-place. */
    x = (x + (x >> 4)) & h0F;          /* Count of each 8 bits in-place. */
    return (x * h01) >> 56;            /* Sum of all bytes. */
#endif
}
#else /* Not 64-bit. */
#if __GNUC__ >= 4 && __POPCNT__
static inline unsigned int
count_1bits_32__(uint32_t x)
{
    return __builtin_popcount(x);
}
#else
#define NEED_COUNT_1BITS_8 1
extern const uint8_t count_1bits_8[256];
static inline unsigned int
count_1bits_32__(uint32_t x)
{
    /* This portable implementation is the fastest one we know of for 32 bits,
     * and faster than GCC __builtin_popcount(). */
    return (count_1bits_8[x & 0xff] +
            count_1bits_8[(x >> 8) & 0xff] +
            count_1bits_8[(x >> 16) & 0xff] +
            count_1bits_8[x >> 24]);
}
#endif
static inline unsigned int
count_1bits(uint64_t x)
{
    return count_1bits_32__(x) + count_1bits_32__(x >> 32);
}
#endif

/* Returns the rightmost 1-bit in 'x' (e.g. 01011000 => 00001000), or 0 if 'x'
 * is 0. */
static inline uintmax_t
rightmost_1bit(uintmax_t x)
{
    return x & -x;
}

/* Returns 'x' with its rightmost 1-bit changed to a zero (e.g. 01011000 =>
 * 01010000), or 0 if 'x' is 0. */
static inline uintmax_t
zero_rightmost_1bit(uintmax_t x)
{
    return x & (x - 1);
}

/* Returns the index of the rightmost 1-bit in 'x' (e.g. 01011000 => 3), or an
 * undefined value if 'x' is 0. */
static inline int
rightmost_1bit_idx(uint64_t x)
{
    return ctz64(x);
}

/* Returns the index of the leftmost 1-bit in 'x' (e.g. 01011000 => 6), or an
 * undefined value if 'x' is 0. */
static inline uint32_t
leftmost_1bit_idx(uint64_t x)
{
    return log_2_floor(x);
}

/* Return a ovs_be32 prefix in network byte order with 'plen' highest bits set.
 * Shift with 32 is undefined behavior, but we rather use 64-bit shift than
 * compare. */
static inline ovs_be32 be32_prefix_mask(int plen)
{
    return htonl((uint64_t)UINT32_MAX << (32 - plen));
}

// @P4:
uint64_t be24_to_u64(const uint8_t *);
uint64_t be40_to_u64(const uint8_t *);
uint64_t be48_to_u64(const uint8_t *);
uint64_t be56_to_u64(const uint8_t *);
ovs_be64 u24_to_be64(const uint8_t *);
ovs_be64 u40_to_be64(const uint8_t *);
ovs_be64 u48_to_be64(const uint8_t *);
ovs_be64 u56_to_be64(const uint8_t *);
void apply_mask_0(const uint8_t *src, const uint8_t *mask, uint8_t *dst,
                  size_t n);
void apply_mask_1(const uint8_t *key, const uint8_t * header,
                  const uint8_t *mask, uint8_t *res, size_t);
void apply_mask_1_u16(const uint16_t *key, const uint16_t * header,
                    const uint16_t *mask, uint16_t *res, size_t);

bool is_all_zeros(const void *, size_t);
bool is_all_ones(const void *, size_t);
void bitwise_copy(const void *src, unsigned int src_len, unsigned int src_ofs,
                  void *dst, unsigned int dst_len, unsigned int dst_ofs,
                  unsigned int n_bits);
void bitwise_zero(void *dst_, unsigned int dst_len, unsigned dst_ofs,
                  unsigned int n_bits);
void bitwise_one(void *dst_, unsigned int dst_len, unsigned dst_ofs,
                 unsigned int n_bits);
bool bitwise_is_all_zeros(const void *, unsigned int len, unsigned int ofs,
                          unsigned int n_bits);
unsigned int bitwise_scan(const void *, unsigned int len,
                          bool target, unsigned int start, unsigned int end);
int bitwise_rscan(const void *, unsigned int len, bool target,
                  int start, int end);
void bitwise_put(uint64_t value,
                 void *dst, unsigned int dst_len, unsigned int dst_ofs,
                 unsigned int n_bits);
uint64_t bitwise_get(const void *src, unsigned int src_len,
                     unsigned int src_ofs, unsigned int n_bits);
bool bitwise_get_bit(const void *src, unsigned int len, unsigned int ofs);
void bitwise_put0(void *dst, unsigned int len, unsigned int ofs);
void bitwise_put1(void *dst, unsigned int len, unsigned int ofs);
void bitwise_put_bit(void *dst, unsigned int len, unsigned int ofs, bool);
void bitwise_toggle_bit(void *dst, unsigned int len, unsigned int ofs);

/* Returns non-zero if the parameters have equal value. */
static inline int
ovs_u128_equals(const ovs_u128 *a, const ovs_u128 *b)
{
    return (a->u64.hi == b->u64.hi) && (a->u64.lo == b->u64.lo);
}

void xsleep(unsigned int seconds);

#ifdef _WIN32

char *ovs_format_message(int error);
char *ovs_lasterror_to_string(void);
int ftruncate(int fd, off_t length);
#endif

#ifdef  __cplusplus
}
#endif

#endif /* util.h */
