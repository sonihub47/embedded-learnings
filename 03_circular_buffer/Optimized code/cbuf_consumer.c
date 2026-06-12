// task side, owns read state

#include "cbuf.h"

// forward declaration — we need to READ producer's index
extern uint32_t cbuf_get_write_idx(void);

// read_idx is static — invisible outside this file (ownership enforced)
// ONLY the task (cbuf_pop) can advance it
static volatile uint32_t read_idx = 0;

// exposed as read-only to producer via a getter
uint32_t cbuf_get_read_idx(void)
{
    return read_idx;
}

cbuf_status cbuf_pop(chunk* value)
{
    if(!value) return CBUF_NULL_POINTER;

    uint32_t w_idx = cbuf_get_write_idx();  // read producer's index, never write it
    if(read_idx == w_idx)
        return CBUF_BUFFER_EMPTY;

    uint32_t slot = read_idx & (CBUF_BUFFER_SIZE - 1);      // alternatively - burning more CPU cycles use modulo since this trick works only when CBUF_BUFFER_SIZE is 2^n
    *value = cbuf_buffer[slot];
    read_idx++;           // only this file ever does this      // lock-free since ownership enforced already

    return CBUF_POP_SUCCESS;
}