#ifndef BUTTON_SM_H
#define BUTTON_SM_H

#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"

// Events - input to this state machine
// from GPIO and timer interrupts 
typedef enum {
    EVT_FALLING_EDGE,       
    EVT_RISING_EDGE,        
    EVT_TIMER_EXP,          
    BTN_NUM_EVENTS
} BtnEvent_t;

// States
typedef enum {
    BTN_IDLE,          // stable released — waiting for activity
    BTN_DEBOUNCING,    // edge seen, timer running, line noisy
    BTN_PRESSED,       // stable pressed — waiting for release
    BTN_NUM_STATES
} BtnState_t;


// State machine instance
// One per physical button — caller owns storage 
typedef struct {
    BtnState_t          state;
    osTimerId_t         debounce_timer;
    osMessageQueueId_t  event_queue;        // this SM's input queue
    osMessageQueueId_t  app_event_queue;    // events to app layer
    uint32_t            debounceTimeMs;     // per-instance, from spec
} ButtonSM_t;


// public APIs

// button_sm_init()
// Called once at startup. Caller passes in pre-created RTOS objects.
// SM owns no RTOS resources — caller does.

void button_sm_init(ButtonSM_t         *sm,
                    osMessageQueueId_t  event_queue,
                    osMessageQueueId_t  app_event_queue,
                    osTimerId_t         debounce_timer,
                    uint32_t            debounceTimeMs);

// button_sm_dispatch()
// Feed one event into the machine.
// Must be called from task context only — never directly from ISR.2
// Reason: internally calls osTimerStart() / osTimerStop() which are
// task-context-only APIs in CMSIS-RTOSv2. ISR posts to queue instead;
// task drains queue and calls dispatch.
 
void button_sm_dispatch(ButtonSM_t *sm, BtnEvent_t e);

// button_sm_state()
// Query current stable state at any moment.
// Useful for combo / multi-press detection where the consumer needs
// to know if the button is already held when a second event arrives.
// Returns BTN_IDLE, BTN_DEBOUNCING, or BTN_PRESSED.
 
BtnState_t button_sm_state(const ButtonSM_t *sm);

#endif /* BUTTON_SM_H */