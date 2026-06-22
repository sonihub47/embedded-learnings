#include <stdint.h>
#include <stdbool.h>

// Simulated 16-bit hardware tick counter (wraps around every 65,535 seconds!)
static volatile uint16_t g_system_ticks = 0;

typedef struct {
    uint16_t start_time;
    uint16_t duration;
} SoftTimer_t;

/**
 * EXERCISE 1: Start a Software Timer
 * TODO:
 * 1. Initialize the timer struct with the current system tick and desired duration.
 * 2. Crucial Senior Nuance: Is reading 'g_system_ticks' atomic on a 32-bit MCU? 
 * What if this structure initialization is interrupted?
 */
void Timer_Start(SoftTimer_t *timer, uint16_t duration_ms) {
    if (timer == NULL) return;

    // --- YOUR CODE HERE ---
    timer->start_time = g_system_ticks;
    timer->duration = duration_ms;
    // ----------------------
}

/**
 * EXERCISE 2: Non-Blocking Expiry Check
 * This function is polled continuously in a background loop or thread.
 * * TODO:
 * 1. Return 'true' if the duration has completely elapsed, 'false' otherwise.
 * 2. MUST handle the case where 'g_system_ticks' has wrapped around (e.g., from 0xFFFF to 0x0000) 
 * since 'Timer_Start' was called.
 * 3. Do NOT use division, modulo, or typecasting to 32-bit integers to cheat the wrap-around. 
 * Solve it purely with 16-bit math.
 */
bool Timer_HasElapsed(const SoftTimer_t *timer) {

    uint16_t elapsed = g_system_ticks - timer->start_time;      // unsigned modulo subtraction (takes care of wrap around 0)
                                                                // unless the duration is within the range of uint16_t

    if (timer == NULL) return false;

    // --- YOUR CODE HERE ---
    if(elapsed >= timer->duration)
        return true;

    return false;
    // ----------------------
}