#include <stdint.h>

#ifndef ATAG_H
#define ATAG_H

typedef enum {
    NONE = 0x00000000,
    CORE = 0x54410001,
    MEM = 0x54410002,
    INITRD2 = 0x54420005,
    CMDLINE = 0x54410009,
} atag_tag_t;

typedef struct {
    uint32_t size;
    uint32_t start;
} mem_t;

typedef struct {
    uint32_t start;
    uint32_t size;
} initrd2_t;

typedef struct {
    char line[1];
} cmdline_t;

typedef struct atag {
    uint32_t tag_size;
    atag_tag_t tag;
    union {
        mem_t mem;
        initrd2_t initrd2;
        cmdline_t cmdline;
    };
} atag_t;

#define MBOX_GET_BOARD_MODEL 0x00010001
#define MBOX_GET_BOARD_REVISION 0x00010002
#define MBOX_GET_BOARD_MAC_ADDRESS 0x00010003
#define MBOX_GET_BOARD_SERIAL 0x00010004
#define MBOX_GET_ARM_MEMORY 0x00010005
#define MBOX_GET_VC_MEMORY 0x00010006
#define MBOX_GET_CLOCK_RATE 0x00030002
#define MBOX_GET_MAX_CLOCK_RATE 0x00030004

typedef struct 
{
    uint32_t size;
    uint32_t start;
    uint32_t modelrev;
    uint64_t serial;
    uint8_t byte00;
    uint8_t byte01;
    uint8_t byte02;
    uint8_t byte03;
    uint8_t byte04;
    uint8_t byte05;
    uint8_t byte06;
    uint8_t byte07;
    uint8_t resp;
} mBoxInfoResp;

extern uint32_t get_mem_size(atag_t * atags);
extern mBoxInfoResp get_info_arm(unsigned long vFunc);
extern uint32_t get_clock_hz(unsigned long vFunc);
extern uint32_t set_clock_hz(unsigned long vclock);

#endif
