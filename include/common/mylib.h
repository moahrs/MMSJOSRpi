#ifndef MYLIB_H
#define MYLIB_H
#if !defined(__cplusplus)
#include <stdbool.h>
#endif
#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

inline void io_cli()
{
	//asm volatile("cpsid i");
}

extern unsigned char extDebugActiv;

void waitCycle(int32_t count);
void waitMicro(uint32_t us);
int strCpy(char *dst, char const *src);
int strCmp(char const *str1, char const *str2);
int strNcmp(char const *str1, char const *str2, int len);
void *memSet(void *str, int c, size_t n);
void *memCopy(void *dest, const void *src, size_t n);
int memCompare(const void *str1, const void *str2, size_t n);
size_t strLen(const char* str);
int strHex(char *str, uint32_t ad, int len, int fill);
int strNum(char *str, uint32_t ui, int len, int fill, int sflag);

#define STR_GUARD 1024

#endif

