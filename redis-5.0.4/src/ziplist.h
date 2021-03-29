/*
 * Copyright (c) 2009-2012, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 * Copyright (c) 2009-2012, Salvatore Sanfilippo <antirez at gmail dot com>
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

#ifndef _ZIPLIST_H
#define _ZIPLIST_H

#define ZIPLIST_HEAD 0
#define ZIPLIST_TAIL 1

unsigned char *ziplistNew(void);
// 合并两个压缩表（将短的拼到长的后面，短的内存销毁）
unsigned char *ziplistMerge(unsigned char **first, unsigned char **second);
// 在端点位置插入字符串(s, slen), where决定是头部还是尾部
unsigned char *ziplistPush(unsigned char *zl, unsigned char *s, unsigned int slen, int where);
// 返回指定序号的数据块地址，如果index是负数，逆向计算，如果没找到返回NULL
unsigned char *ziplistIndex(unsigned char *zl, int index);
// 返回下一个数据块地址，如果到边界返回NULL
unsigned char *ziplistNext(unsigned char *zl, unsigned char *p);
// 返回上一个数据块地址，如果到边界返回NULL
unsigned char *ziplistPrev(unsigned char *zl, unsigned char *p);
// 从数据块中读取字符串或者整数
unsigned int ziplistGet(unsigned char *p, unsigned char **sval, unsigned int *slen, long long *lval);
// 在p数据块位置插入数据s
unsigned char *ziplistInsert(unsigned char *zl, unsigned char *p, unsigned char *s, unsigned int slen);
// 删除p位置数据块，返回删除后新的p位置
unsigned char *ziplistDelete(unsigned char *zl, unsigned char **p);
// 删除范围(index, num)的数据块
unsigned char *ziplistDeleteRange(unsigned char *zl, int index, unsigned int num);
// 比较(p数据块的数据)和(s字符串)
unsigned int ziplistCompare(unsigned char *p, unsigned char *s, unsigned int slen);
// 按skip步进数，在p位置开始查找vstr匹配数据块
unsigned char *ziplistFind(unsigned char *p, unsigned char *vstr, unsigned int vlen, unsigned int skip);
// 获得ziplist元素数
unsigned int ziplistLen(unsigned char *zl);
// 获得ziplist字节长度
size_t ziplistBlobLen(unsigned char *zl);
// 格式化打印ziplist
void ziplistRepr(unsigned char *zl);

#ifdef REDIS_TEST
int ziplistTest(int argc, char *argv[]);
#endif

#endif /* _ZIPLIST_H */
