// ISR side, owns write state

#include "cbuf.h"

// forward declaration — we need to READ consumer's index
// but this is an implementation detail, not a public API
extern uint32_t cbuf_get_read_idx(void);

// write_idx is static — invisible outside this file
// ONLY the ISR (cbuf_push) can advance it
static volatile uint32_t write_idx = 0;

// exposed as read-only to consumer via a getter
uint32_t cbuf_get_write_idx(void)
{
    return write_idx;
}

cbuf_status cbuf_push(const chunk* value)
{
    if(!value) return CBUF_NULL_POINTER;

    uint32_t r_idx = cbuf_get_read_idx();   // read consumer's index, never write it
    if((write_idx - r_idx) == CBUF_BUFFER_SIZE)
        return CBUF_BUFFER_FULL;

    uint32_t slot = write_idx & (CBUF_BUFFER_SIZE - 1);
    cbuf_buffer[slot] = *value;
    write_idx++;          // only this file ever does this

    return CBUF_PUSH_SUCCESS;
}