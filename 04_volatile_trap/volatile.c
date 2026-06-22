#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"

#define SENSOR_STATUS_REG_ADDR   0x40021000
#define STATUS_DATA_READY_BIT    (1 << 2)

bool Sensor_WaitForDataReady(uint32_t timeout_ms) {
    // SYNTAX: The 'volatile' must modify 'uint32_t', meaning the contents 
    // at the address can change dynamically. The pointer address itself is fixed.
    volatile uint32_t * const status_reg = (volatile uint32_t *)SENSOR_STATUS_REG_ADDR;
    
    uint32_t start_tick = osKernelGetTickCount();           // if osKernelGetTickFreq() == 1000; then 1 tick = 1ms
    
    while (((*status_reg) & STATUS_DATA_READY_BIT) == 0) {

        // ANTIDOTE TO STARVATION (ONLY FOR EQUAL PRIORITY): 
        // Yields the current time slice to other threads of the EXACT SAME priority level.
        // WARNING: This will still starve lower-priority threads if no equal-priority tasks exist.
        osThreadYield();        // moving the thread from RUNNING to READY state
        
        // Guard against infinite stalls if the hardware dies
        if ((osKernelGetTickCount() - start_tick) >= timeout_ms) {
            return false; // Timeout reached
        }
    }
    
    return true; // Data is ready!
}