# Challenge 2 — Lock-Free Circular Buffer (SPSC)

## Problem Statement

Implement a circular buffer in C for a bare-metal or RTOS embedded system storing fixed-size elements.
Used in a **single-producer / single-consumer (SPSC)** scenario:

- Producer runs in **ISR context** — calls `cbuf_push`
- Consumer runs in **task context** — calls `cbuf_pop`

API requirements: `cbuf_init`, `cbuf_push`, `cbuf_pop`, `cbuf_is_full`, `cbuf_is_empty`

The implementation must be **lock-free** — no mutex, no critical section.

---

## Approach 1 — count-based (learning checkpoint)

### How full/empty is detected
A `count` member tracks the number of occupied slots. Full when `count == SIZE`, empty when `count == 0`.

### Why this needs a critical section
`count` is modified from two contexts:
- ISR increments it (`count++`) on every push
- Task decrements it (`count--`) on every pop

On Cortex-M4, `count++` is not a single instruction. It compiles to three steps:

```
LOAD  r0, [count]   ; read current value
ADD   r0, r0, #1    ; increment
STORE [count], r0   ; write back
```

If the task's `LOAD` is preempted by the ISR between the task's `ADD` and `STORE`, one update is silently lost. This is a **Read-Modify-Write (RMW) race condition**.

A critical section (disable interrupts) must wrap `count++` and `count--` specifically — not the entire push/pop body.

### What about the data buffer itself?
The buffer array is shared between ISR and task, but it is **not** a race condition:
- ISR writes to `buffer[head]`
- Task reads from `buffer[tail]`
- `head` and `tail` always point to different slots (enforced by the full check)

Since they never touch the same slot concurrently, no protection is needed around the buffer. `head` is exclusively written by the ISR, `tail` exclusively by the task — each side owns one pointer.

### Key learning
`count` is the problem because it is the only variable written by both sides. Everything else has a clear owner.

---

## Approach 2 — index-based, lock-free

### Core idea: eliminate `count` entirely

Instead of a shared counter, use two monotonically incrementing indices:

```c
static volatile uint32_t write_idx = 0;  // owned by ISR
static volatile uint32_t read_idx  = 0;  // owned by task
```

Full/empty derived on the fly:

```c
empty: read_idx == write_idx
full:  (write_idx - read_idx) == BUFFER_SIZE
```

### Ownership enforced by the language

Each index is declared `static` at file scope inside its owner's translation unit:

- `write_idx` lives in `cbuf_producer.c` — physically unreachable from any other file
- `read_idx` lives in `cbuf_consumer.c` — physically unreachable from any other file

`static` at file scope is not just a comment — it is a compiler-enforced ownership boundary. Another file cannot accidentally write an index it doesn't own; the build will fail if it tries.

### How the other side reads the index it doesn't own

Each file exposes a read-only getter, forward-declared privately in the file that needs it:

```c
/* cbuf_producer.c — needs to read consumer's index */
extern uint32_t cbuf_get_read_idx(void);
```

```c
/* cbuf_consumer.c — needs to read producer's index */
extern uint32_t cbuf_get_write_idx(void);
```

These getters are **not** declared in `cbuf.h`. They are narrow cross-file wires between the two implementation files — invisible to the rest of the system. This keeps the public API clean while allowing the two sides to observe each other's progress.

### Why indices are never reset to zero

Both indices increment forever and are never reset. The actual buffer slot is derived by masking:

```c
uint32_t slot = write_idx & (BUFFER_SIZE - 1);
```

The indices naturally wrap around at `2^32` — and crucially, **unsigned subtraction wraps correctly too**:

```
write_idx = 2          (just wrapped past 2^32)
read_idx  = 4294967290 (not yet wrapped)

2 - 4294967290  (mod 2^32)  =  8  ✓
```

The occupancy calculation `write_idx - read_idx` gives the correct answer regardless of how many times the indices have wrapped, as long as both are the same unsigned type.

### Why bitmask instead of modulo

```c
slot = write_idx % BUFFER_SIZE;   // UDIV instruction — 4–12 cycles on Cortex-M4
slot = write_idx & (BUFFER_SIZE - 1);  // AND instruction  — 1 cycle
```

The bitmask trick works only when `BUFFER_SIZE` is a power of 2. When a number is a power of 2, subtracting 1 produces a mask of all 1s in the lower bits:

```
16 - 1 = 15 = 0000 1111
```

ANDing with this mask keeps only the lower bits — identical to `% 16`, but in one cycle. Enforce the power-of-2 constraint at compile time:

```c
static_assert((BUFFER_SIZE & (BUFFER_SIZE - 1)) == 0, "BUFFER_SIZE must be a power of 2");
```

### Why no critical section is needed

Each index is written by exactly one context and read by the other. On Cortex-M, a naturally aligned 32-bit read is atomic by architecture — the read of the other side's index cannot be torn. Neither side ever writes the other's index. There is no shared mutable state — and therefore nothing to protect.

---