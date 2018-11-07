#include <stdint.h>

#ifndef STDLIB_H
#define STDLIB_H

typedef struct divmod_result {
    uint32_t div;
    uint32_t mod;
} divmod_t;

divmod_t divmod(uint32_t dividend, uint32_t divisor);
extern uint32_t div(uint32_t dividend, uint32_t divisor);
extern void * memset ( void * ptr, int value, int num );
extern void memcpy(void * dest, void * src, int bytes);
extern void bzero(void * dest, int bytes);
extern long strtol(const char *nptr, char **endptr, register int base);
extern char * itoa(int num, char* intbuf, int base);
extern int atoi (const char * str);
extern long atol(const char *p);
extern int atoi(const char *p);
extern unsigned int toint(unsigned char * s);
extern int abs(int j);
extern int strlen(const char *s);
extern int strcmp(const char *cs, const char *ct);
extern char *strcpy(char *dest, const char *src);
extern int nanosleep (long pTimeNano);
extern void Delays(int pTime);
extern void Delayus(int pTime);
extern void Delayms(int pTime);
extern void Delayns(int pTime);

#define toupper(c) \
  (((c) >= 'a' && (c) <= 'z') ? ((c) - 'a' + 'A') : (c))

#endif
