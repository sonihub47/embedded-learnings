// shared types and API declarations only

#ifndef CBUF_H
#define CBUF_H

#include <stdint.h>
#include <stdbool.h>

#define CBUF_BUFFER_SIZE 16

typedef enum {
    CBUF_INIT_SUCCESS,
    CBUF_PUSH_SUCCESS,
    CBUF_POP_SUCCESS,
    CBUF_BUFFER_FULL,
    CBUF_BUFFER_EMPTY,
    CBUF_NULL_POINTER
} cbuf_status;

typedef struct {
    uint8_t  data[16];
    uint32_t epoch_timestamp;
} chunk;

// shared buffer — declared here, defined once in cbuf_common.c
extern chunk cbuf_buffer[CBUF_BUFFER_SIZE];

// API
cbuf_status cbuf_init(void);
cbuf_status cbuf_push(const chunk* value);   // ISR calls this
cbuf_status cbuf_pop(chunk* value);          // task calls this

#endif