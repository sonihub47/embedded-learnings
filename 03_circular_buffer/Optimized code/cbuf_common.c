// init, shared read-only access

#include "cbuf.h"
#include <string.h>

// buffer storage — defined once, shared via extern in header
chunk cbuf_buffer[CBUF_BUFFER_SIZE];

cbuf_status cbuf_init(void)
{
    memset(cbuf_buffer, 0, sizeof(cbuf_buffer));
    return CBUF_INIT_SUCCESS;
}