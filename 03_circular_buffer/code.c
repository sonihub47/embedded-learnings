/*
 * Circular buffer — count-based implementation (learning checkpoint).
 *
 * Known limitation: 'count' is read and written by both ISR (push)
 * and task (pop). On Cortex-M, count++/count-- are not atomic
 * (load/modify/store). Requires a critical section around push/pop
 * to be safe in production.
 *
 * Next: index-based lock-free implementation
 * where ISR owns write_idx and task owns read_idx exclusively.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define CBUF_BUFFER_SIZE 16
#define ZERO_FILL_MEMORY  0

typedef enum {
    CBUF_INIT_SUCCESS,
    CBUF_BUFFER_FULL,
    CBUF_BUFFER_EMPTY,
    CBUF_PUSH_SUCCESS,
    CBUF_POP_SUCCESS,
    CBUF_NULL_POINTER
} cbuf_status;

typedef struct{
    uint8_t data[16];
    uint32_t epoch_timestamp;
}chunk;

typedef struct{
    chunk* buff;
    uint8_t count;
    chunk* head;
    chunk* tail;
}cbuf;

chunk buffer[CBUF_BUFFER_SIZE];
cbuf buff_mgr;

cbuf_status cbuf_init(cbuf* gbuff_mgr, chunk* gbuffer)
{
    if(!gbuff_mgr || !gbuffer)
        return CBUF_NULL_POINTER;
    
    gbuff_mgr->buff = gbuffer;
    gbuff_mgr->head = gbuffer;
    gbuff_mgr->tail = gbuffer;
    gbuff_mgr->count = 0;

    // zero out the entire buffer upon initialization
    memset(gbuff_mgr->buff, ZERO_FILL_MEMORY, sizeof(chunk) * CBUF_BUFFER_SIZE);

    return CBUF_INIT_SUCCESS;
}

bool cbuf_isFull(const cbuf* buff_mgr)
{
    assert(buff_mgr != NULL);
    return (buff_mgr->count == CBUF_BUFFER_SIZE);
}

bool cbuf_isEmpty(const cbuf* buff_mgr)
{
    assert(buff_mgr != NULL);
    return (buff_mgr->count == 0);
}

// Single producer - called from ISR context 
cbuf_status cbuf_push(cbuf* buff_mgr, chunk* value)
{
 
    if(!buff_mgr || !value)
        return CBUF_NULL_POINTER;

    if(cbuf_isFull(buff_mgr))
    {
        // buffer full policy
        // drop the packet
        return CBUF_BUFFER_FULL;
    }
    else
    {
        // -- ENTERing critical section - buff_mgr is a global shared state
        // data copy
        memcpy((*(buff_mgr->head)).data, value->data, sizeof(value->data));
        (*(buff_mgr->head)).epoch_timestamp = value->epoch_timestamp;

        // increment count and head
        buff_mgr->count++;
        if(buff_mgr->head == &(buff_mgr->buff[CBUF_BUFFER_SIZE-1]))
            buff_mgr->head = buff_mgr->buff; // reset head to index 0 to mimic circular buffer!
        else
            buff_mgr->head++;     
        
        //  -- EXITing critical section - buff_mgr is a global shared state

        // update status
        return CBUF_PUSH_SUCCESS;
    }
}

// Single consumer - called from Task context
cbuf_status cbuf_pop(cbuf* buff_mgr, chunk* value)
{

    if(!buff_mgr || !value)
        return CBUF_NULL_POINTER;

    if(cbuf_isEmpty(buff_mgr))
    {
        // nothing to POP
        return CBUF_BUFFER_EMPTY;
    }
    else
    {
        // -- ENTERing critical section - buff_mgr is a global shared state

        // data copy
        memcpy(value->data, (*(buff_mgr->tail)).data, sizeof(value->data));
        value->epoch_timestamp = (*(buff_mgr->tail)).epoch_timestamp;

        // decrement count and increment tail
        buff_mgr->count--;
        if(buff_mgr->tail == &(buff_mgr->buff[CBUF_BUFFER_SIZE-1]))
            buff_mgr->tail = buff_mgr->buff; // reset tail to index 0 to mimic circular buffer!
        else
            buff_mgr->tail++;     
        
        //  -- EXITing critical section - buff_mgr is a global shared state

        //update status
        return CBUF_POP_SUCCESS;
    }
}

