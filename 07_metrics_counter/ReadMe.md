```Design and implement a thread-safe, lock-free (where possible), non-blocking metrics counter system.```

>The Scenario
You are building an IoT asset tracker. A high-frequency timer interrupt (ISR) increments an uptime tick counter. Simultaneously, a low-priority background thread reads this counter to broadcast health packets over a slow network.

If you use a standard mutex or osDelay inside the ISR to protect the data, the system crashes. If you read a 64-bit counter on a 32-bit MCU without protection, you risk a torn read (reading partial stale data during a context switch).

>The Interview Sandbox Code

```c
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"

// Simulated 32-bit architecture (e.g., ARM Cortex-M4)
typedef struct {
    uint64_t total_ticks;      // Accumulates system ticks (Note: 64-bit on 32-bit CPU!)
    uint32_t missed_deadlines; // Incremented if processing takes too long
} SystemMetrics_t;

static volatile SystemMetrics_t g_metrics;

/**
 * EXERCISE 1: The Interrupt Service Routine (ISR Context)
 * Triggered by a hardware timer every 1ms.
 * 
 * TODO: 
 * 1. Safely increment 'total_ticks'. 
 * 2. This runs in an ISR. Identify why standard CMSIS Mutexes/Semaphores cannot be used here.
 * 3. Handle the 'missed_deadlines' tracking if a race condition is detected.
 */
void HighFrequency_Timer_IRQHandler(void) {
    // --- YOUR CODE HERE ---
    g_metrics.total_ticks++; 
    // ----------------------
}

/**
 * EXERCISE 2: The Consumer Thread (Thread Context)
 * Invoked periodically by the RTOS to snapshot metrics.
 * 
 * TODO:
 * 1. Implement a lock-free strategy (like a read-loop or atomic snapshot) to read 
 *    the 64-bit 'total_ticks' into 'out_snapshot' without a torn read.
 * 2. Ensure your solution does not disable global interrupts for more than a few cycles.
 * 3. Discuss: Why is 'volatile' alone insufficient to solve this?
 */
bool System_GetMetricsSnapshot(SystemMetrics_t *out_snapshot) {
    if (out_snapshot == NULL) return false;

    // --- YOUR CODE HERE ---

    // ----------------------
    return true;
}
```
