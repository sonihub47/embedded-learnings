#ifndef DRIVER_H
#define DRIVER_H

#include <stdint.h>

// The callback type: a function that takes a byte and an opaque context pointer.
// The driver doesn't know what 'context' is. It stores it, hands it back.
typedef void (*rx_cb_fn)(uint8_t byte, void *context);

typedef struct {
    rx_cb_fn  cb;   // who to call
    void     *ctx;  // their state pointer (opaque to us)
} driver_t;

void driver_init(driver_t *drv);
void driver_register_cb(driver_t *drv, rx_cb_fn cb, void *context);

// In real life: called from ISR. Here we call it manually to simulate.
void driver_simulate_byte(driver_t *drv, uint8_t byte);

#endif

