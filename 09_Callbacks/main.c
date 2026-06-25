#include <stdio.h>
#include <string.h>
#include "driver.h"

//  driver.c (inlined for simplicity) 

void driver_init(driver_t *drv)
{
    drv->cb  = NULL;
    drv->ctx = NULL;
}

void driver_register_cb(driver_t *drv, rx_cb_fn cb, void *context)
{
    drv->cb  = cb;       // store the function pointer
    drv->ctx = context;  // store their opaque state pointer
}

void driver_simulate_byte(driver_t *drv, uint8_t byte)
{
    // This is what an ISR does: read hardware, then call whoever registered.
    // The driver does NOT know what happens next. It just fires and forgets.
    if (drv->cb != NULL) {
        drv->cb(byte, drv->ctx);  // <── the callback fires here
    }
}

//  consumer (the layer that wants to react)

typedef struct {
    uint8_t buf[64];
    uint8_t count;
} consumer_t;

// This is the function we'll register. Note the signature matches rx_cb_fn.
// 'context' arrives as void* — we cast it back to our own type.
void on_byte_received(uint8_t byte, void *context)
{
    consumer_t *self = (consumer_t *)context;  // <── restore identity

    self->buf[self->count++] = byte;
    printf("consumer got: 0x%02X  (total so far: %u bytes)\n", byte, self->count);
}

//  main: wiring 

int main(void)
{
    driver_t   drv;
    consumer_t con;

    memset(&con, 0, sizeof(con));
    driver_init(&drv);

    // REGISTRATION: consumer hands the driver two things:
    //   1. the function to call (on_byte_received)
    //   2. its own state pointer (&con) — so the callback can use self->buf
    // After this line, the driver knows nothing about consumer_t internals.
    driver_register_cb(&drv, on_byte_received, &con);

    // RUNTIME: simulate three bytes arriving (as if from ISR)
    driver_simulate_byte(&drv, 0xAA);
    driver_simulate_byte(&drv, 0x0C);
    driver_simulate_byte(&drv, 0x55);

    return 0;
}

