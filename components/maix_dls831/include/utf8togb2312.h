/*
 * convert.h
 *
 *  Created on: 2016-8-6
 *      Author: wilson
 */

#ifndef CONVERY_H_
#define CONVERY_H_

typedef unsigned char            u8;
typedef unsigned short           u16;
typedef unsigned int             u32;
typedef unsigned long long int   u64;
typedef char                     s8;
typedef short                    s16;
typedef int                      s32;
typedef long long int            s64;
typedef unsigned char BOOL8;
typedef unsigned int BOOL;

/*内联用__attribute__*/
#define INLINE__            inline

/*数组大小*/
#undef  ARRAY_SIZE
#define ARRAY_SIZE(a)       ((sizeof(a))/(sizeof((a)[0])))

/***********************************************************
 * 函数名称：gbkk2utf8
 * 功能描述：将 gbk转为 utf8
 * 输入参数：pin_buf 输入缓冲区
 *       in_len  输入长度
 * 输出参数：ptr 成功后的起始位置(malloc产生),(数据末尾加上'0')
 * 返 回 值：  转换后的长度, < 0 失败,其它成功
 ***********************************************************/
int gbk2utf8(char **ptr, void *pin_buf, s32 in_len);

/***********************************************************
 * 函数名称：gbkk2utf8
 * 功能描述：将 gbk转为 utf8
 * 输入参数：pin_buf 输入缓冲区
 *       in_len  输入长度
 * 输出参数：ptr 成功后的起始位置(malloc产生),(数据末尾加上'0')
 * 返 回 值：  转换后的长度, < 0 失败,其它成功
 ***********************************************************/
int utf82gbk(char **ptr, void *pin_buf, s32 in_len);

// void convet_test(void);

#endif /* TEST_H_ */
