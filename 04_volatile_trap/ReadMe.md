#The Scenario: The Critical Section & The "Volatile-Pointer" Trap
You are writing a driver for an external hardware sensor connected via an SPI or parallel bus. The hardware has a Status Register mapped to a specific physical memory address.

To prevent data corruption, a high-priority background thread reads this status register in a tight loop, waiting for a "Data Ready" flag to be set by the hardware. Simultaneously, other threads might be trying to access the same peripheral bus, meaning you must protect this register read sequence using a critical section (disabling/enabling interrupts) or an RTOS primitive.

```c
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"

// Simulated Hardware Register Address (e.g., Sensor Status Register)
#define SENSOR_STATUS_REG_ADDR   0x40021000
#define STATUS_DATA_READY_BIT    (1 << 2)

/**
 * EXERCISE: Read Status Register Safely
 * * TODO:
 * 1. Create a pointer pointing to the hardware register address.
 * 2. Implement a lock-free or critical-section protected loop that polls the 
 * STATUS_DATA_READY_BIT until it becomes 1.
 * 3. Crucial Senior Nuance 1: Where exactly do you place the 'volatile' keyword? 
 * Is it 'volatile uint32_t *reg' or 'uint32_t * volatile reg'? 
 * 4. Crucial Senior Nuance 2: What happens if your loop spins infinitely? 
 * How do you protect the rest of your RTOS system from CPU starvation?
 */
bool Sensor_WaitForDataReady(uint32_t timeout_ms) {
    // --- YOUR CODE HERE ---

    // ----------------------
    return true;
}
```

