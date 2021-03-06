#include <stdint.h>
#include <common/stdlib.h>
#include <limits.h>

extern "C" void dummy ( unsigned int );

extern "C" void __aeabi_unwind_cpp_pr0();
extern "C" void __aeabi_unwind_cpp_pr1();
extern void __div0(void);

/*
 * 32-bit. (internal funtion)
 */
unsigned int __udivmodsi4(unsigned int num, unsigned int den, unsigned int * rem_p)
{
    unsigned int quot = 0, qbit = 1;

    if (den == 0)
    {
        return 0;
    }

    /*
     * left-justify denominator and count shift
     */
    while ((signed int) den >= 0)
    {
        den <<= 1;
        qbit <<= 1;
    }

    while (qbit)
    {
        if (den <= num)
        {
            num -= den;
            quot += qbit;
        }
        den >>= 1;
        qbit >>= 1;
    }

    if (rem_p)
        *rem_p = num;

    return quot;
}

/*
 * 32-bit signed integer divide.
 */
signed int __aeabi_idiv(signed int num, signed int den)
{
    signed int minus = 0;
    signed int v;

    if (num < 0)
    {
        num = -num;
        minus = 1;
    }
    if (den < 0)
    {
        den = -den;
        minus ^= 1;
    }

    v = __udivmodsi4(num, den, 0);
    if (minus)
        v = -v;

    return v;
}

/*
 * 32-bit unsigned integer divide.
 */
unsigned int __aeabi_uidiv(unsigned int num, unsigned int den)
{
    return __udivmodsi4(num, den, 0);
}

uint32_t div(uint32_t dividend, uint32_t divisor) 
{
    // Use long division, but in binary.  Copied from Stack overflow...
    uint32_t denom=divisor;
    uint32_t current = 1;
    uint32_t answer=0;

    if ( denom > dividend)
        return 0;

    if ( denom == dividend)
        return 1;

    while (denom <= dividend) {
        denom <<= 1;
        current <<= 1;
    }

    denom >>= 1;
    current >>= 1;

    while (current!=0) {
        if ( dividend >= denom) {
            dividend -= denom;
            answer |= current;
        }
        current >>= 1;
        denom >>= 1;
    }
    return answer;
}

divmod_t divmod(uint32_t dividend, uint32_t divisor) {
    divmod_t res;

    res.div = div(dividend, divisor);
    res.mod = dividend - res.div*divisor;

    return res;
}

void * memset ( void * ptr, int value, int num )
{
    unsigned char *dst = (unsigned char*)ptr;
    while (num > 0) {
        *dst = (unsigned char) value;
        dst++;
        num--;
    }

    return ptr;
}

void memcpy(void * dest, void * src, int bytes) {
    char * d = (char*)dest, * s = (char*)src;
    while (bytes--) {
        *d++ = *s++;
    }
}

void bzero(void * dest, int bytes) {
    char * d = (char*)dest;
    while (bytes--) {
        *d++ = 0;
    }
}

long strtol(const char *nptr, char **endptr, register int base)
{
    register unsigned long acc;
    register unsigned char c;
    register unsigned long cutoff;
    register signed char any;
    unsigned char flag = 0;
    #define FL_NEG  0x01        /* number is negative */
    #define FL_0X   0x02        /* number has a 0x prefix */

    if (endptr)
        *endptr = (char *)nptr;
    if (base != 0 && (base < 2 || base > 36))
        return 0;

    do {
        c = *nptr++;
    } while (c == 0x20);
    if (c == '-') {
        flag = FL_NEG;
        c = *nptr++;
    } else if (c == '+')
        c = *nptr++;
    if ((base == 0 || base == 16) &&
        c == '0' && (*nptr == 'x' || *nptr == 'X')) {
        c = nptr[1];
        nptr += 2;
        base = 16;
        flag |= FL_0X;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    switch (base) {
        case 10:
        cutoff = ((unsigned long)LONG_MAX + 1) / 10;
        break;
        case 16:
        cutoff = ((unsigned long)LONG_MAX + 1) / 16;
        break;
        case 8:
        cutoff = ((unsigned long)LONG_MAX + 1) / 8;
        break;
        case 2:
        cutoff = ((unsigned long)LONG_MAX + 1) / 2;
        break;
        default:
        cutoff = ((unsigned long)LONG_MAX + 1) / base;
    }

    for (acc = 0, any = 0;; c = *nptr++) {
        if (c >= '0' && c <= '9')
            c -= '0';
        else if (c >= 'A' && c <= 'Z')
            c -= 'A' - 10;
        else if (c >= 'a' && c <= 'z')
            c -= 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0)
            continue;
        if (acc > cutoff) {
            any = -1;
            continue;
        }
        acc = acc * base + c;
        if (acc > (unsigned long)LONG_MAX + 1)
            any = -1;
        else
            any = 1;
    }
    if (endptr) {
        if (any)
            *endptr = (char *)nptr - 1;
        else if (flag & FL_0X)
            *endptr = (char *)nptr - 2;
    }
    if (any < 0) {
        acc = (flag & FL_NEG) ? LONG_MIN : LONG_MAX;
    } else if (flag & FL_NEG) {
        acc = -acc;
    } else if ((signed long)acc < 0) {
        acc = LONG_MAX;
    }
    return (acc);
}

long atol(const char *p)
{
    return strtol(p, (char **) NULL, 10);
}

int atoi(const char *p)
{
    return (int) atol(p);
}

int abs(int j)
{
  if (j < 0)
    {
      j = -j;
    }
  return j;
}

int oldstrlen(const char *s)
{
  const char *sc;
  for (sc = s; *sc != '\0'; ++sc);
  return sc - s;
}

int strcmp(const char *cs, const char *ct)
{
  register signed char result;
  for (;;)
    {
      if ((result = *cs - *ct++) != 0 || !*cs++)
    break;
    }
  return result;
}

char *strcpy(char *dest, const char *src)
{
  char *tmp = dest;
  while ((*dest++ = *src++) != '\0');
  return tmp;
}

char * itoa(int num, char* intbuf, int base) {
//    static char intbuf[33];
    uint32_t j = 0, isneg = 0, i;
    divmod_t divmod_res;

    if (num == 0) {
        intbuf[0] = '0';
        intbuf[1] = '\0';
        return intbuf;
    }

    if (base == 10 && num < 0) {
        isneg = 1;
        num = -num;
    }

    i = (uint32_t) num;

    while (i != 0) {
       divmod_res = divmod(i,base);
       intbuf[j++] = (divmod_res.mod) < 10 ? '0' + (divmod_res.mod) : 'a' + (divmod_res.mod) - 10;
       i = divmod_res.div;
    }

    if (isneg)
        intbuf[j++] = '-';

    if (base == 16) {
        intbuf[j++] = 'x';
        intbuf[j++] = '0';
    } else if(base == 8) {
        intbuf[j++] = '0';
    } else if(base == 2) {
        intbuf[j++] = 'b';
        intbuf[j++] = '0';
    }

    intbuf[j] = '\0';
    j--;
    i = 0;
    while (i < j) {
        isneg = intbuf[i];
        intbuf[i] = intbuf[j];
        intbuf[j] = isneg;
        i++;
        j--;
    }

    return intbuf;
}

int nanosleep (long pTimeNano)
{
    long ra;

    for(ra=0;ra<pTimeNano;ra++) dummy(ra);

    return 1;
}

void Delays(int pTime)
{
    nanosleep(pTime * 5000000);
}

void Delayms(int pTime)
{
    nanosleep(pTime * 5000);
}

void Delayus(int pTime)
{
    nanosleep(pTime * 5);
}

void Delayns(long pTime)
{
    nanosleep(pTime);
}

void __aeabi_unwind_cpp_pr0()
{
}

void __aeabi_unwind_cpp_pr1()
{
}

