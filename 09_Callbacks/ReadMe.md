# Callback Pattern — Single Consumer

Two files. One concept.

```
driver.h   — defines the callback type and driver struct
main.c     — driver impl + consumer + wiring
```

---

## What problem does this solve?

The driver knows when a byte arrives (it owns the hardware). The consumer
knows what to do with it (it owns the application logic). Neither should
reach into the other's internals.

A callback is the handshake: the consumer tells the driver *"when something
happens, call this function"* — and then walks away.

---

## The flow in three steps

```
INIT TIME

  consumer                          driver
     │                                │
     │── register_cb(fn, &self) ─────>│  driver stores fn + &self
     │                                │  (knows nothing else about consumer)


RUNTIME — byte arrives in HW FIFO

  [ISR]
     │
     │── driver_simulate_byte()
              │
              │── drv->cb(byte, drv->ctx)     fn ptr fires
                          │         │
                        byte      &self        both handed back to consumer
                          │         │
                          v         v
              on_byte_received(byte, context)
                    │
                    │  consumer_t *self = (consumer_t *)context
                    │  self->buf[self->count++] = byte
                    │
                   done
```

---

## The role of `void *`

`void *` is what makes the driver reusable across any consumer type.

```c
// driver.h — no mention of consumer_t anywhere
typedef void (*rx_cb_fn)(uint8_t byte, void *context);

typedef struct {
    rx_cb_fn  cb;
    void     *ctx;   // opaque — driver never dereferences this
} driver_t;
```

The driver stores `&con` as a `void *`. It cannot read `con.buf`, cannot
read `con.count`. It has no idea what lives at that address. It just holds
the pointer and hands it back untouched when the callback fires.

The consumer recovers its identity with a single cast:

```c
void on_byte_received(uint8_t byte, void *context)
{
    consumer_t *self = (consumer_t *)context;  // identity restored
    self->buf[self->count++] = byte;
}
```

This is how C simulates `this`. The `void *` is the `this` pointer —
it carries instance state across a plain function pointer boundary.

Two things `void *` gives you here:

**Zero compile-time coupling.** `driver.h` includes no consumer header.
The driver compiles without knowing `consumer_t` exists. Dependency arrow
points one way only: consumer knows about driver, driver knows nothing
about consumer.

**Reusability.** Swap `consumer_t` for `protocol_layer_t`, `logger_t`,
`lcd_driver_t` — the driver doesn't change. Register a different struct,
get a different `self` back inside the callback.

---

## Stack frame note

The callback's stack frame is built on **whoever calls the driver**.

In this demo that's `main()` — so `on_byte_received` runs on the main
stack. In real hardware, `driver_simulate_byte` is replaced by the ISR:

```
UART0_Handler()          ← CPU switches to ISR stack here
  └─ driver_isr_handler()
         └─ drv->cb(byte, drv->ctx)
                └─ on_byte_received()   ← runs on ISR stack
```

Consequence: everything inside `on_byte_received` must be ISR-safe.
No `osDelay()`, no mutex, no blocking queue send. Post to a queue with
`FromISR` and return immediately. A task on its own stack does the rest.

---

## Scaling to multiple consumers

This driver holds one `cb`/`ctx` pair — one consumer. For multiple
consumers (e.g. a 1ms tick timer that needs to notify debounce, blinker,
and watchdog), replace the single pair with a table:

```c
typedef struct {
    rx_cb_fn  cb;
    void     *ctx;
} subscriber_t;

subscriber_t slots[MAX_SUBS];   // same pattern, N times
```

`register()` drops a pair into the next free slot and returns the index
as the consumer ID. The ISR iterates the table and fires each one.
