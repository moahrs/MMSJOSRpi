#include <kernel/atag.h>
#include "common/mbox.h"

mBoxInfoResp MBOX_INFO_RESP;

uint32_t get_mem_size(atag_t * tag) {
    int ix = 0;
    while ((tag->tag != NONE) && (ix <= 10)) 
    {
    	ix++;
        if (tag->tag == MEM) {
            return tag->mem.size;
        }
        tag = (atag_t *)(((uint32_t *)tag) + tag->tag_size);
    }
   
    return 0;
}

mBoxInfoResp get_info_arm(unsigned long vFunc)
{
    mBoxInfoResp memResp;
    uintptr_t mb_addr = 0x00007000;     // 0x7000 in L2 cache coherent mode
    volatile uint32_t *mailbuffer = (uint32_t *)mb_addr;

    // Get the base clock rate 
    // set up the buffer
    mailbuffer[0] = 12 * 4;      // size of this message
    mailbuffer[1] = 0;          // this is a request

    // next comes the first tag
    mailbuffer[2] = vFunc;      // get tag (0x00010005 = arm memory, 0x00010006 = vc memory)

    switch (vFunc)
    {
        case MBOX_GET_BOARD_FIRMWARE:
        case MBOX_GET_BOARD_MODEL:
        case MBOX_GET_BOARD_REVISION:
            mailbuffer[3] = 0x4;
            break;
        case MBOX_GET_BOARD_MAC_ADDRESS:
            mailbuffer[3] = 0x6;
            break;
        case MBOX_GET_BOARD_SERIAL:
            mailbuffer[3] = 0x8;
            break;
        case MBOX_GET_ARM_MEMORY:
        case MBOX_GET_VC_MEMORY:
            mailbuffer[3] = 0x8;
            break;
    }

    mailbuffer[4] = 0x0;        // 
    mailbuffer[5] = 0x0;        // start memmory
    mailbuffer[6] = 0x0;        // size memory 
    mailbuffer[7] = 0x0;
    mailbuffer[8] = 0x0;
    mailbuffer[9] = 0x0;
    mailbuffer[10] = 0x0;
    mailbuffer[11] = 0x0;

    // send the message
    mbox_write(MBOX_PROP, mb_addr);

    // read the response
    mbox_read(MBOX_PROP);

    if(mailbuffer[1] != MBOX_SUCCESS) {
        memResp.resp = 0;
        return memResp;
    }

    switch (vFunc)
    {
        case MBOX_GET_BOARD_FIRMWARE:
        case MBOX_GET_BOARD_MODEL:
        case MBOX_GET_BOARD_REVISION:
            memResp.modelrev = mailbuffer[5];
            break;
        case MBOX_GET_BOARD_MAC_ADDRESS:
            memResp.byte00 = mailbuffer[5];
            memResp.byte01 = mailbuffer[6];
            memResp.byte02 = mailbuffer[7];
            memResp.byte03 = mailbuffer[8];
            memResp.byte04 = mailbuffer[9];
            memResp.byte05 = mailbuffer[10];
            break;
        case MBOX_GET_BOARD_SERIAL:
            memResp.serial = mailbuffer[5];
            break;
        case MBOX_GET_ARM_MEMORY:
        case MBOX_GET_VC_MEMORY:
            memResp.size = mailbuffer[6];
            memResp.start = mailbuffer[7];
            break;
    }

    memResp.resp = 1;
    
    return memResp;
}

uint32_t get_clock_hz(unsigned long vFunc)
{
    uint32_t mbase_clock;
    uintptr_t mb_addr = 0x00007000;     // 0x7000 in L2 cache coherent mode
    volatile uint32_t *mailbuffer = (uint32_t *)mb_addr;

    /* Get the base clock rate */
    // set up the buffer
    mailbuffer[0] = 8 * 4;      // size of this message
    mailbuffer[1] = 0;          // this is a request

    // next comes the first tag
    mailbuffer[2] = vFunc;      // get clock rate tag (0x00030002 = actual clock rate, 0x00030004 = max clock rate)
    mailbuffer[3] = 0x8;        // value buffer size
    mailbuffer[4] = 0x4;        // is a request, value length = 4
    mailbuffer[5] = 0x3;        // clock id + space to return clock id
    mailbuffer[6] = 0;          // space to return rate (in Hz)

    // closing tag
    mailbuffer[7] = 0;

    // send the message
    mbox_write(MBOX_PROP, mb_addr);

    // read the response
    mbox_read(MBOX_PROP);

    if(mailbuffer[1] != MBOX_SUCCESS)
        return 0;

    mbase_clock = mailbuffer[6];

    return mbase_clock;
}

uint32_t set_clock_hz(unsigned long vclock)
{
    uint32_t mbase_clock;
    uintptr_t mb_addr = 0x00007000;     // 0x7000 in L2 cache coherent mode
    volatile uint32_t *mailbuffer = (uint32_t *)mb_addr;

    /* Get the base clock rate */
    // set up the buffer
    mailbuffer[0] = 9 * 4;      // size of this message
    mailbuffer[1] = 0;          // this is a request

    // next comes the first tag
    mailbuffer[2] = 0x00038002; // set clock rate tag 
    mailbuffer[3] = 0x8;        // value buffer size
    mailbuffer[4] = 0x12;       // is a request, value length = 12
    mailbuffer[5] = 0x3;        // clock id + space to return clock id
    mailbuffer[6] = vclock;     // set rate (in Hz)
    mailbuffer[7] = 1;          // skip turbo

    // closing tag
    mailbuffer[8] = 0;

    // send the message
    mbox_write(MBOX_PROP, mb_addr);

    // read the response
    mbox_read(MBOX_PROP);

    if(mailbuffer[1] != MBOX_SUCCESS)
        return 0;

    mbase_clock = mailbuffer[6];

    return mbase_clock;
}

