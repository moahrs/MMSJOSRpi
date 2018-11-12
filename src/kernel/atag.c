#include <kernel/atag.h>
#include "common/mbox.h"

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

unsigned long get_memory(unsigned long vFunc)
{
    unsigned long memMemory;
    uintptr_t mb_addr = 0x00007000;     // 0x7000 in L2 cache coherent mode
    volatile uint32_t *mailbuffer = (uint32_t *)mb_addr;

    // Get the base clock rate 
    // set up the buffer
    mailbuffer[0] = 8 * 4;      // size of this message
    mailbuffer[1] = 0;          // this is a request

    // next comes the first tag
    mailbuffer[2] = vFunc;      // get tag (arm 0x00010005, vc 0x00010006)
    mailbuffer[3] = 0x8;        // value buffer size
    mailbuffer[4] = 0x0;        // 
    mailbuffer[5] = 0x0;        // start memmory
    mailbuffer[6] = 0x0;        // size memory 

    // closing tag
    mailbuffer[7] = 0x0;

    // send the message
    mbox_write(MBOX_PROP, mb_addr);

    // read the response
    mbox_read(MBOX_PROP);

    if(mailbuffer[1] != MBOX_SUCCESS)
        return 0;

    memMemory = mailbuffer[6];

    return memMemory;
}

