#include <stddef.h>
#include <stdint.h>
#include <drivers/bcm2835min.h>
#include <drivers/uart.h>
#include <common/stdlib.h>
#include "common/mylib.h"

// Loop <delay> times in a way that the compiler won't optimize away
void delaycyles(int32_t count)
{
    asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
            : "=r"(count): [count]"0"(count) : "cc");
}

void uart_init()
{
    uart_control_t control;

    // Disable UART0.
    bzero(&control, 4);
    bcm2835_peri_write((uint32_t*)UART0_CR, control.as_int);

    // Setup the GPIO pin 14 && 15.
    // Disable pull up/down for all GPIO pins & delay for 150 cycles.
    bcm2835_gpio_pud(BCM2835_GPIO_PUD_OFF);
    delaycyles(150);

/*    // Disable pull up/down for pin 14,15 & delay for 150 cycles.
    bcm2835_peri_write((uint32_t*)BCM2835_GPPUDCLK0, (1 << 14) | (1 << 15));
    delaycyles(150);

    // Write 0 to GPPUDCLK0 to make it take effect.
    bcm2835_peri_write((uint32_t*)BCM2835_GPPUDCLK0, 0x00000000);
    delaycyles(150);*/

    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_08, BCM2835_GPIO_FSEL_ALT0);   // GPIO14
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_10, BCM2835_GPIO_FSEL_ALT0);   // GPIO15

    while (read_flags().as_int & (1 << 3));

    // Clear pending interrupts.
    bcm2835_peri_write((uint32_t*)UART0_ICR, 0x7FF);

    // Set integer & fractional part of baud rate.
    // Divider = UART_CLOCK/(16 * Baud)
    // Fraction part register = (Fractional part * 64) + 0.5
    // UART_CLOCK = 4000000; Baud = 115200.

    // Divider = 4000000 / (16 * 9600) = 26.0417 = ~26.
    bcm2835_peri_write((uint32_t*)UART0_IBRD, 2);
    // Fractional part register = (.0417 * 64) + 0.5 = 3.1688 = ~3.
    bcm2835_peri_write((uint32_t*)UART0_FBRD, 11);

    // Enable FIFO & 8 bit data transmissio (1 stop bit, no parity).
    bcm2835_peri_write((uint32_t*)UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

    // Mask all interrupts.
    bcm2835_peri_write((uint32_t*)UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
            (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));

    // Enable UART0, receive & transfer part of UART.
    control.uart_enabled = 1;
    control.transmit_enabled = 1;
    control.receive_enabled = 1;
    bcm2835_peri_write((uint32_t*)UART0_CR, control.as_int);
}


uart_flags_t read_flags(void) {
    uart_flags_t flags;
    flags.as_int = bcm2835_peri_read((uint32_t*)UART0_FR);
    return flags;
}

void flush_uart(void)
{
    uart_flags_t flags;
    do {
        bcm2835_peri_read((uint32_t*)UART0_DR);
        flags = read_flags();
    }
    while (!flags.recieve_queue_empty);
}

int uart_setbaud(int pBaud)
{
    int retBaud;
    uart_control_t control;

    // Disable UART0.
    bzero(&control, 4);
    bcm2835_peri_write((uint32_t*)UART0_CR, control.as_int);

    switch (pBaud)
    {
        case 9600:     // 9600
            // Divider = 4000000 / (16 * 9600) = 26.0417 = ~26.
            bcm2835_peri_write((uint32_t*)UART0_IBRD, 26);
            // Fractional part register = (.0417 * 64) + 0.5 = 3.16 = ~3.
            bcm2835_peri_write((uint32_t*)UART0_FBRD, 3);
            retBaud = 9600;
            break;
        case 38400:     // 38400
            // Divider = 4000000 / (16 * 38400) = 6.5104 = ~6.
            bcm2835_peri_write((uint32_t*)UART0_IBRD, 6);
            // Fractional part register = (.5104 * 64) + 0.5 = 33.16 = ~33.
            bcm2835_peri_write((uint32_t*)UART0_FBRD, 33);
            retBaud = 38400;
            break;
        case 115200:     // 115200
            // Divider = 4000000 / (16 * 115200) = 2.1701 = ~2.
            bcm2835_peri_write((uint32_t*)UART0_IBRD, 2);
            // Fractional part register = (.1701 * 64) + 0.5 = 11.38 = ~11.
            bcm2835_peri_write((uint32_t*)UART0_FBRD, 11);
            retBaud = 115200;
            break;
        case 230400:     // 230400
            // Divider = 4000000 / (16 * 230400) = 1.0851 = ~1.
            bcm2835_peri_write((uint32_t*)UART0_IBRD, 1);
            // Fractional part register = (.0851 * 64) + 0.5 = 5.94 = ~6.
            bcm2835_peri_write((uint32_t*)UART0_FBRD, 6);
            retBaud = 230400;
            break;
        default:    // 9600
            bcm2835_peri_write((uint32_t*)UART0_IBRD, 26);
            bcm2835_peri_write((uint32_t*)UART0_FBRD, 3);
            retBaud = 9600;
    }

    // Enable UART0, receive & transfer part of UART.
    control.uart_enabled = 1;
    control.transmit_enabled = 1;
    control.receive_enabled = 1;
    bcm2835_peri_write((uint32_t*)UART0_CR, control.as_int);

    return retBaud;
}

void uart_putc(unsigned char c)
{
    uart_flags_t flags;
    // Wait for UART to become ready to transmit.

    do {
        flags = read_flags();
    }
    while ( flags.transmit_queue_full );
    bcm2835_peri_write((uint32_t*)UART0_DR, c);
}

int uart_getc(unsigned long pTimeOut)
{
    // Wait for UART to have received something, or timeout running out.
    uart_flags_t flags;
    do {
        flags = read_flags();

        if (pTimeOut != 0xFFFFFFFF)
            pTimeOut--;
    }
    while ( flags.recieve_queue_empty && pTimeOut > 0);

    if (pTimeOut > 0)
        return bcm2835_peri_read((uint32_t*)UART0_DR);
    else 
        return -1;
}

char getc(void) {
    return (char)uart_getc(0xFFFFFFFF);
}

int getct(unsigned long pTimeOut) {
    return uart_getc(pTimeOut);
}

void putc(char c) {
    uart_putc(c);
}

void puts(const char * str) {
    int i;
    for (i = 0; str[i] != '\0'; i ++) 
        putc(str[i]);
}


void getst(char * buf, int buflen) {
    int i;
    int c;
    // Leave a spot for null char in buffer
    for (i = 0; (c = getct(0xFFFFFF)) != -1 && buflen > 1; i++, buflen--)
        buf[i] = (char)c;

    buf[i] = '\0';
}

// This version of gets copies until newline, replacing newline with null char, or until buflen.
// whichever comes first
void gets(char * buf, int buflen) {
    int i;
    char c;
    // Leave a spot for null char in buffer
    for (i = 0; (c = getc()) != '\r' && buflen > 1; i++, buflen--) {
        putc(c);
        buf[i] = c;
    }

    buf[i] = '\0';
}
