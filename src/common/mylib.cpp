#include "common/mylib.h"
#include "kernel/mmio.h"
#include <circle/types.h>

#ifdef __USE_TFT_LCD__
	#include <circle/interrupt.h>
	#include <ScreenTFT/screentft.h>
#else
	#include <circle/screen.h>
#endif

extern void io_cli();

//#define GPFSEL1		0x20200004
//#define GPSET0	 	0x2020001C
//#define GPCLR0	 	0x20200028

//#define SYST_CLO	0x20003004
//#define SYST_CHI	0x20003008

unsigned char extDebugActiv = 0;

static inline void wait(int32_t count)
{
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}

void waitCycle(int32_t count)
{
	wait(count);
}

uint64_t get_system_timer(void)
{
	uint32_t chi, clo;
	
	chi = mmio_read(STIMER_CHI);
	clo = mmio_read(STIMER_CLO);

	if(chi !=  mmio_read(STIMER_CHI)){
		chi = mmio_read(STIMER_CHI);
		clo = mmio_read(STIMER_CLO);
	}

	return (((unsigned long long) chi) << 32) | clo;

}

void waitMicro(uint32_t tt)
{
	uint64_t t0;
	t0 = get_system_timer();
	
	while(get_system_timer() < t0 + tt);

	return;
	
}

int strCpy(char *dst, char const *src)
{
	int i;
	i = 0;
	while(*src!=0) {
		*(dst++) = *(src++);
		i++;
	} 
	*dst = 0;
	return i;
}

int strCmp(char const *str1, char const *str2)
{
	char *s1 = (char *) str1;
	char *s2 = (char *) str2;
	while(*s1 == *s2) {
		if (*s1 == 0) {
			return 0;
		}
		s1++;
		s2++;
	}
	if (*s1<*s2) {
		return -1;
	} else {
		return 1;
	}
}

int strNcmp(char const *str1, char const *str2, int len)
{
	char *s1 = (char *) str1;
	char *s2 = (char *) str2;
	while(len--) {
		if (*s1 != *s2) {
			return *s1-*s2;
		}
		s1++;
		s2++;
	}
	return 0;
}

void *memSet(void *str, int c, size_t n)
{
	char *dst = (char *) str;
	while(n--) {
		*dst++ = (char) (c & 0xFF);
	}
	return str;
}


void *memCopy(void *dest, const void *src, size_t n)
{
	char *d = (char *) dest;
	char *s = (char *) src;
	while(n--) {
		*d++ = *s++;
	}
	return dest;
}

int memCompare(const void *str1, const void *str2, size_t n)
{
	const char *s1 = (char*)str1;
	const char *s2 = (char*)str2;
	while(n--) {
		if (*s1<*s2) {
			return -1;
		} else if (*s1>*s2) {
			return 1;
		}
		s1++;
		s2++;
	}
	return 0;
}


/* uint32_t u32_div(uint32_t n, uint32_t d) */
/* { */
/* 	signed int q; */
/* 	uint32_t bit; */

/* 	q = 0; */
/* 	if (d==0) { */
/* 		q=0xFFFFFFFF; */
/* 	} else { */
/* 		bit = 1; */
/* 		while(((d & 0x80000000) == 0) && (d<n)) { */
/* 			d <<=1; */
/* 			bit <<=1; */
/* 		} */
		
/* 		while(bit>0) { */
/* 			if (n>=d) { */
/* 				n -= d; */
/* 				q += bit; */
/* 			} */
/* 			d   >>= 1; */
/* 			bit >>= 1; */
/* 		} */
/* 	} */

/* 	return q; */
/* } */

size_t strLen(const char* str)
{
	size_t ret = 0;
	while ( str[ret] != 0 ) {
		ret++;
	}
	return ret;
}



/*int putChar(char a)
{
	// todo: add stdio support
	uart_putc(a);
	return (int) a;
}

int getChar()
{
	// todo: add stdio support
	return uart_getc();
}*/

int strHex(char *str, uint32_t ad, int len, int fill)
{
	int i, j=0;
	int c;
	char s[1024];
	int st;

	st = 0;
	for(i=0; i<8; i++) {
		if ((c = ((ad>>28) & 0x0F))>0) {
			c = (c<10) ? (c+'0') : (c+'a'-10);
			s[j++] = (char) c;
			st = 1;
		} else if (st || i==7) {
			s[j++] = '0';
		}
		ad = ad << 4;
	}
	s[j]=0;
	for(i=0; i<len-j; i++) {
		str[i] = (fill) ? '0' : ' ';
	}
	strCpy(str+i, s);
	
	return j+i;
}

int strNum(char *str, uint32_t ui, int len, int fill, int sflag)
{
	unsigned int cmp = 1;
	int i, j;
	int d;
	int l;
	char c;
	char s[1024];

	cmp=1;
	l=1;
	//	printf("ui:%d, cmp:%d, l:%d\n", ui, cmp, l);
	while(cmp*10<=ui && l<10) {
		//2^32 = 4.29 * 10^9 -> Max digits =10 for 32 bit
		cmp *=10;
		l++;
		//		printf("ui:%d, cmp:%d, l:%d\n", ui, cmp, l);
	}

	j = 0;
	if (sflag) {
		s[j++]='-';
		l++; // add one space for '0'
	}
	//	printf("ui:%d, cmp:%d, j:%d, d:%d, c:%d\n", ui, cmp, j, d, c);
	while(j<l) {
		//		d = u32_div(ui, cmp);
		d = ui/cmp;
		c = (char) (d+'0');
		s[j++] = c;
		ui = ui - d*cmp;
		//cmp = u32_div(cmp, 10);
		cmp = cmp/10;
		//		Printf("ui:%d, cmp:%d, j:%d, d:%d, c:%d\n", ui, cmp, j, d, c);
	}
	s[j]=0;
	for(i=0; i<len-l; i++) {
		str[i] = (fill) ? '0' : ' ';
	}
	strCpy(str+i, s);
	
	return j+i;
}

//-----------------------------------------------------------------------------
void putS(const char* str)
{
	#ifdef __USE_TFT_LCD__
    	CScrTft::Get ()->writestr((char*)str);
    #else
    #endif
}

//-----------------------------------------------------------------------------
int printf(const char *format, ...)
{
    va_list ap;
    char s[1024];
    int i;

    va_start(ap, format);
    i = vsprintf(s, format, ap);
    putS(s);
    va_end(ap);
    return i;
}

//-----------------------------------------------------------------------------
int vsprintf(char *str, const char *format, va_list listPointer)
{
    char c;
    int i, si;
    unsigned int ui;
    int pf;
    char *s;
    int len, fill, sign_flag;
    
    i=0;
    pf = 0;
    len = 0;
    fill = 0;
    while((c=*format++)!=0 && i<STR_GUARD) {
        if (pf==0) {
            // after regular character
            if (c=='%') {
                pf=1;
                len = 0;
                fill = 0;
            } else {
                str[i++]=c;
            }
        } else if (pf>0) {
            // previous character was '%'
            if (c=='x' || c=='X') {
                ui = va_arg(listPointer, unsigned);
                i += strHex(str+i, (unsigned) ui, len, fill);
                pf=0;
            } else if (c=='u' || c=='U' || c=='i' || c=='I') {
                ui = va_arg(listPointer, unsigned);
                i += strNum(str+i, (unsigned) ui, len, fill, 0);
                pf=0;
            } else if (c=='d' || c=='D') {
                si = va_arg(listPointer, int);
                if (si<0) {
                    ui = -si;
                    sign_flag = 1;
                } else {
                    ui = si;
                    sign_flag = 0;
                }
                i += strNum(str+i, (unsigned) ui, len, fill, sign_flag);
                pf=0;
            } else if (c=='s' || c=='S') {
                s = va_arg(listPointer, char *);
                i += strCpy(str+i, s);
                pf=0;
            } else if ('0'<=c && c<='9') {
                if (pf==1 && c=='0') {
                    fill = 1;
                } else {
                    len = len*10+(c-'0');
                }
                pf=2;
            } else {
                // this shouldn't happen
                str[i++]=c;
                pf=0;
            }
        }
    }
    str[i]=0;
/*    if (i>STR_GUARD) {
        log_error(1);
    }*/
    return (i>0) ? i : -1;
}

//-----------------------------------------------------------------------------
int sprintf(char *str, const char *format, ...)
{
    va_list ap;
    int i;

    va_start(ap, format);
    i = vsprintf(str, format, ap);
    va_end(ap);
    return i;
}
