/* SDSLib 2.0 -- A C dynamic strings library
 *
 * Copyright (c) 2006-2015, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2015, Oran Agra
 * Copyright (c) 2015, Redis Labs, Inc
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __SDS_H
#define __SDS_H

#define SDS_MAX_PREALLOC (1024*1024)
const char *SDS_NOINIT;

#include <sys/types.h>
#include <stdarg.h>
#include <stdint.h>

// 注意:sds是sdshdr结构体的buf指针
typedef char *sds;

// hdr -> header
/* Note: sdshdr5 is never used, we just access the flags byte directly.
 * However is here to document the layout of type 5 SDS strings. */
struct __attribute__ ((__packed__)) sdshdr5 {
    unsigned char flags; /* 3 lsb of type, and 5 msb of string length */
    char buf[];
};
struct __attribute__ ((__packed__)) sdshdr8 {
    uint8_t len; /* used */
    uint8_t alloc; /* excluding the header and null terminator */
    unsigned char flags; /* 3 lsb of type, 5 unused bits */
    char buf[];
};
struct __attribute__ ((__packed__)) sdshdr16 {
    uint16_t len; /* used */
    uint16_t alloc; /* excluding the header and null terminator */
    unsigned char flags; /* 3 lsb of type, 5 unused bits */
    char buf[];
};
struct __attribute__ ((__packed__)) sdshdr32 {
    uint32_t len; /* used */
    uint32_t alloc; /* excluding the header and null terminator */
    unsigned char flags; /* 3 lsb of type, 5 unused bits */
    char buf[];
};
struct __attribute__ ((__packed__)) sdshdr64 {
    uint64_t len; /* used */
    uint64_t alloc; /* excluding the header and null terminator */
    unsigned char flags; /* 3 lsb of type, 5 unused bits */
    char buf[];
};

#define SDS_TYPE_5  0
#define SDS_TYPE_8  1
#define SDS_TYPE_16 2
#define SDS_TYPE_32 3
#define SDS_TYPE_64 4
#define SDS_TYPE_MASK 7
#define SDS_TYPE_BITS 3
#define SDS_HDR_VAR(T,s) struct sdshdr##T *sh = (void*)((s)-(sizeof(struct sdshdr##T)));
#define SDS_HDR(T,s) ((struct sdshdr##T *)((s)-(sizeof(struct sdshdr##T))))
#define SDS_TYPE_5_LEN(f) ((f)>>SDS_TYPE_BITS)

// 传进来buf指针，向前偏移得到结构体首地址，然后得到len值
static inline size_t sdslen(const sds s) {
    unsigned char flags = s[-1];
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5:
            return SDS_TYPE_5_LEN(flags);
        case SDS_TYPE_8:
            return SDS_HDR(8,s)->len;
        case SDS_TYPE_16:
            return SDS_HDR(16,s)->len;
        case SDS_TYPE_32:
            return SDS_HDR(32,s)->len;
        case SDS_TYPE_64:
            return SDS_HDR(64,s)->len;
    }
    return 0;
}

// 获得剩余可用空间长度
static inline size_t sdsavail(const sds s) {
    unsigned char flags = s[-1];
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5: {
            return 0;
        }
        case SDS_TYPE_8: {
            SDS_HDR_VAR(8,s);
            return sh->alloc - sh->len;
        }
        case SDS_TYPE_16: {
            SDS_HDR_VAR(16,s);
            return sh->alloc - sh->len;
        }
        case SDS_TYPE_32: {
            SDS_HDR_VAR(32,s);
            return sh->alloc - sh->len;
        }
        case SDS_TYPE_64: {
            SDS_HDR_VAR(64,s);
            return sh->alloc - sh->len;
        }
    }
    return 0;
}

static inline void sdssetlen(sds s, size_t newlen) {
    unsigned char flags = s[-1];
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5:
            {
                // type5的len信息保存在flags里面
                unsigned char *fp = ((unsigned char*)s)-1;
                *fp = SDS_TYPE_5 | (newlen << SDS_TYPE_BITS);
            }
            break;
        case SDS_TYPE_8:
            SDS_HDR(8,s)->len = newlen;
            break;
        case SDS_TYPE_16:
            SDS_HDR(16,s)->len = newlen;
            break;
        case SDS_TYPE_32:
            SDS_HDR(32,s)->len = newlen;
            break;
        case SDS_TYPE_64:
            SDS_HDR(64,s)->len = newlen;
            break;
    }
}

static inline void sdsinclen(sds s, size_t inc) {
    unsigned char flags = s[-1];
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5:
            {
                unsigned char *fp = ((unsigned char*)s)-1;
                unsigned char newlen = SDS_TYPE_5_LEN(flags)+inc;
                *fp = SDS_TYPE_5 | (newlen << SDS_TYPE_BITS);
            }
            break;
        case SDS_TYPE_8:
            SDS_HDR(8,s)->len += inc;
            break;
        case SDS_TYPE_16:
            SDS_HDR(16,s)->len += inc;
            break;
        case SDS_TYPE_32:
            SDS_HDR(32,s)->len += inc;
            break;
        case SDS_TYPE_64:
            SDS_HDR(64,s)->len += inc;
            break;
    }
}

/* sdsalloc() = sdsavail() + sdslen() */
static inline size_t sdsalloc(const sds s) {
    unsigned char flags = s[-1];
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5:
            return SDS_TYPE_5_LEN(flags);
        case SDS_TYPE_8:
            return SDS_HDR(8,s)->alloc;
        case SDS_TYPE_16:
            return SDS_HDR(16,s)->alloc;
        case SDS_TYPE_32:
            return SDS_HDR(32,s)->alloc;
        case SDS_TYPE_64:
            return SDS_HDR(64,s)->alloc;
    }
    return 0;
}

static inline void sdssetalloc(sds s, size_t newlen) {
    unsigned char flags = s[-1];
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5:
            /* Nothing to do, this type has no total allocation info. */
            break;
        case SDS_TYPE_8:
            SDS_HDR(8,s)->alloc = newlen;
            break;
        case SDS_TYPE_16:
            SDS_HDR(16,s)->alloc = newlen;
            break;
        case SDS_TYPE_32:
            SDS_HDR(32,s)->alloc = newlen;
            break;
        case SDS_TYPE_64:
            SDS_HDR(64,s)->alloc = newlen;
            break;
    }
}

// 创建一个sdshdr,将init字符串中initlen长度的数据复制进去
sds sdsnewlen(const void *init, size_t initlen);
// 创建一个sdshdr,将init字符串完整复制进去
sds sdsnew(const char *init);
// 创建一个空的sdshdr
sds sdsempty(void);
// 复制一个sdshdr
sds sdsdup(const sds s);
// 回收sdshdr
void sdsfree(sds s);
// 把sds长度扩充为len,用0填充新增长度
sds sdsgrowzero(sds s, size_t len);
// 把t指向的内存拷贝len长度到sds尾部
sds sdscatlen(sds s, const void *t, size_t len);
// 把t指向的全部内存拷贝到sds尾部
sds sdscat(sds s, const char *t);
// 把t拷贝到s尾部
sds sdscatsds(sds s, const sds t);
// 把t指向的len长度的数据覆盖sds
sds sdscpylen(sds s, const char *t, size_t len);
// 把t指向的数据全部覆盖sds
sds sdscpy(sds s, const char *t);

// 用格式化字符串方式把数据填入sds
sds sdscatvprintf(sds s, const char *fmt, va_list ap);
#ifdef __GNUC__
sds sdscatprintf(sds s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
sds sdscatprintf(sds s, const char *fmt, ...);
#endif

// 作用同上，不过内部直接实现了sprintf功能，所以速度更快
// 缺点是只支持固定几种格式
sds sdscatfmt(sds s, char const *fmt, ...);
// 利用cset表示的字符集，剔除sds左右两端的对应字符
sds sdstrim(sds s, const char *cset);
// 用sds的指定范围子串覆盖sds本身
void sdsrange(sds s, ssize_t start, ssize_t end);
// 如果sds中间有\0字符，刷新sdshdr->len=strlen()
void sdsupdatelen(sds s);
// 清空sds数据,但不回收
void sdsclear(sds s);
// 比较两个sds内容是否相同
int sdscmp(const sds s1, const sds s2);
// 把s字符串按sep格式切分合成sds数组
sds *sdssplitlen(const char *s, ssize_t len, const char *sep, int seplen, int *count);
// 把sds数组销毁
void sdsfreesplitres(sds *tokens, int count);
void sdstolower(sds s);
void sdstoupper(sds s);
// 把数字转成sds
sds sdsfromlonglong(long long value);
// 往sds后面拼接字符串,如果包含转义字符,将其转成文本
sds sdscatrepr(sds s, const char *p, size_t len);
// 把line字符串按空格分割合成sds数组,argc赋值为sds数量
sds *sdssplitargs(const char *line, int *argc);
// 在sds中把from匹配到的字符用to对应位置的字符替换
sds sdsmapchars(sds s, const char *from, const char *to, size_t setlen);
// 把argv字符串数组中的字符串用sep字符作为分隔符拼接起来
sds sdsjoin(char **argv, int argc, char *sep);
// 同上，拼接的是sds
sds sdsjoinsds(sds *argv, int argc, const char *sep, size_t seplen);

/* Low level functions exposed to the user API */
// 在sds中预留addlen的未使用空间
sds sdsMakeRoomFor(sds s, size_t addlen);
// 根据incr参数直接拉长或者缩短sdshdr->len
void sdsIncrLen(sds s, ssize_t incr);
// 回收sds中已申请但未使用的空间
sds sdsRemoveFreeSpace(sds s);
// 获得sdshdr开辟内存总大小
size_t sdsAllocSize(sds s);
// 获得sdshdr指针
void *sdsAllocPtr(sds s);

/* Export the allocator used by SDS to the program using SDS.
 * Sometimes the program SDS is linked to, may use a different set of
 * allocators, but may want to allocate or free things that SDS will
 * respectively free or allocate. */
void *sds_malloc(size_t size);
void *sds_realloc(void *ptr, size_t size);
void sds_free(void *ptr);

#ifdef REDIS_TEST
int sdsTest(int argc, char *argv[]);
#endif

#endif
