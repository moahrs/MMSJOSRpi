#include <kernel/atag.h>

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
