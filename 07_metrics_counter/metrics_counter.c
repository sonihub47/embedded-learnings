#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"

typedef struct {
    uint64_t total_ticks;      
    uint32_t missed_deadlines; 
} SystemMetrics_t;

// Use volatile to force memory access every time
static volatile SystemMetrics_t g_metrics;

void HighFrequency_Timer_IRQHandler(void) {
    // Single-cycle or multi-word write from ISR is fine because it's atomic 
    // from the ISR's own perspective (it can't interrupt itself).
    g_metrics.total_ticks++; 
}

bool System_GetMetricsSnapshot(SystemMetrics_t *out_snapshot) {
    if (out_snapshot == NULL) return false;

    uint64_t first_read;
    uint64_t second_read;

    do {
        // 1. Snapshot the value the first time
        first_read = g_metrics.total_ticks;
        
        // 2. Data Memory Barrier (DMB)
        // Forces the CPU and compiler to complete the first read completely 
        // before executing the second read. No instruction reordering allowed.
        __DMB(); 
        
        // 3. Snapshot the value a second time
        second_read = g_metrics.total_ticks;
        
    // 4. If they match, it means the ISR did not execute a write split across our two reads.
    } while (first_read != second_read);

    // Commit the stable, un-torn read to the output snapshot
    out_snapshot->total_ticks = first_read;
    out_snapshot->missed_deadlines = g_metrics.missed_deadlines;

    return true;
}

// A memory barrier (also called a memory fence) is a hardware instruction that tells the CPU 
// pipeline and the compiler: "Do not allow memory accesses before this point to mix with 
// memory accesses(Read or write) after this point."


