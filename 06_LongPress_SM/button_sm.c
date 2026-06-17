
// Layer:   driver
// Owns:    state machine logic, timer control, pin read, app event emit

#include "button_sm.h"
#include "common_events.h"

/* TivaWare — hardware access confined to this file */
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"

//  Forward declarations — state functions (private)
static BtnState_t state_idle      (ButtonSM_t *sm, BtnEvent_t e);
static BtnState_t state_debouncing(ButtonSM_t *sm, BtnEvent_t e);
static BtnState_t state_pressed   (ButtonSM_t *sm, BtnEvent_t e);


// Forward declarations — entry/exit actions (private)
static void on_enter_debouncing(ButtonSM_t *sm);
static void on_enter_pressed   (ButtonSM_t *sm);
static void on_enter_idle      (ButtonSM_t *sm);

static void on_exit_pressed    (ButtonSM_t *sm);

//  Forward declarations — internal helpers (private)
static uint8_t read_pin      (void);
static void    timer_start   (ButtonSM_t *sm);
static void    timer_restart (ButtonSM_t *sm);
static void    emit_app_event(ButtonSM_t *sm, AppEvent_t evt);

//  Dispatch table — state functions per state
typedef BtnState_t (*BtnStateFunc)(ButtonSM_t *, BtnEvent_t);

static const BtnStateFunc state_table[BTN_NUM_STATES] = {
    [BTN_IDLE]       = state_idle,
    [BTN_DEBOUNCING] = state_debouncing,
    [BTN_PRESSED]    = state_pressed,
};

//  Entry action tables                                              
//  Called by dispatcher on every state entry — regardless of source 
//  NULL = no entry action for that state                            
typedef void (*BtnActionFunc)(ButtonSM_t *);

static const BtnActionFunc entry_table[BTN_NUM_STATES] = {
    [BTN_IDLE]       = on_enter_idle,
    [BTN_DEBOUNCING] = on_enter_debouncing,
    [BTN_PRESSED]    = on_enter_pressed,
};

// Exit actions — none needed for this machine; kept for sake of completeness
static const BtnActionFunc exit_table[BTN_NUM_STATES] = {
    [BTN_IDLE]       = NULL,
    [BTN_DEBOUNCING] = NULL,
    [BTN_PRESSED]    = on_exit_pressed,
};

// public APIs
void button_sm_init(ButtonSM_t         *sm,
                    osMessageQueueId_t  event_queue,
                    osMessageQueueId_t  app_event_queue,
                    osTimerId_t         debounce_timer,
                    uint32_t            debounceTimeMs,
                    uint32_t            longPressTimeSec)
{
    sm->state            = BTN_IDLE;
    sm->event_queue      = event_queue;
    sm->app_event_queue  = app_event_queue;
    sm->debounce_timer   = debounce_timer;
    sm->debounceTimeMs   = debounceTimeMs;
    sm->longPressTimeSec = longPressTimeSec;
    sm->isLongPressFired = false;
}


// Only place where sm->state is written (ownership!)
// 'next' kept as local variable for two reasons:
//   1. entry/exit hooks need to compare old vs new before overwriting
//   2. a breakpoint here shows both current and next before transition

void button_sm_dispatch(ButtonSM_t *sm, BtnEvent_t e)
{
    BtnState_t next = state_table[sm->state](sm, e);

    if (next != sm->state) {
        if (exit_table[sm->state])  exit_table[sm->state](sm);
        sm->state = next;
        if (entry_table[sm->state]) entry_table[sm->state](sm);
    }
}

BtnState_t button_sm_state(const ButtonSM_t *sm)
{
    return sm->state;
}

// Entry actions                                                     
//                                                                   
// Fire on every entry to a state, regardless of which transition    
// caused it. This is where "what changes does this transition bring 
// into the system" lives —    */
// separate from "which state is next" (state functions).            
//                                                                   
// The mental test before deciding where an action goes:
// "Should this happen every time I enter state X, regardless of where I came from?"
// Yes -> entry action
// No, it depends on the path -> transition logic in the departing state's function                                    

// on_enter_DEBOUNCING
// Every path into DEBOUNCING needs a fresh timer window.
static void on_enter_debouncing(ButtonSM_t *sm)
{
    timer_start(sm);
}

// on_enter_PRESSED
// Stable press confirmed. Emit the logical event upward.
// Moved here from state_debouncing() so that any future path
// into PRESSED (e.g. restore-from-sleep with pin already low)
// automatically notifies the app — no duplication needed.
static void on_enter_pressed(ButtonSM_t *sm)
{
    // moving it inside state_pressed - EVT_TIMER_EXP
    // as it is now becoming a conditional action!
    // emit_app_event(sm, APP_EVT_BTN_PRESS);

    // also start timer to track for long press (say, 2secs)
    timer_start(sm);

    sm->isLongPressFired = false;   // resetting for a fresh case

}

// on_enter_IDLE
// Stable release confirmed (or initial startup).
// Emit release event only if we were previously PRESSED
// (not on cold init — handled by init setting state directly).
// Note: for simplicity here we emit unconditionally; in a hardened
// driver you would guard against emitting on cold entry from init.
static void on_enter_idle(ButtonSM_t *sm)
{
    emit_app_event(sm, APP_EVT_BTN_RELEASE);
    // TODO: currently, event is being sent
    // unconditionally | need to consider cold init
    // maybe, include a field = sm->prev_state
}

// on_exit_PRESSED
// if it's a LONG PRESS - timer is already stopped
// if it's a NORMAL PRESS - stop the timer
// to avoid ghost events in other states.
static void on_exit_pressed(ButtonSM_t *sm)
{
    // stop the timer
    timer_stop(sm);

}


//  State functions                                                   
//                                                                    
//  Responsibility: answer ONE question only —                        
//  "given I am in state X and event Y arrived, which state is next?" 


// BTN_IDLE
// Stable released. Waiting for first sign of activity on the pin.
static BtnState_t state_idle(ButtonSM_t *sm, BtnEvent_t e)
{
    (void)sm;
    switch (e) {
        case EVT_FALLING_EDGE:
             
            // Pin went LOW — possible press starting.
            // Transition to DEBOUNCING; on_enter_debouncing() will
            // start the timer.
            
            return BTN_DEBOUNCING;

        case EVT_RISING_EDGE:
            // Pin HIGH while already idle — spurious, ignore.
            return BTN_IDLE;

        case EVT_TIMER_EXP:
            
            // Impossible in this state.
            // Timer is started only inside on_enter_DEBOUNCING and on_enter_PRESSED.
            // To reach IDLE the machine must have left on_enter_PRESSED,
            // which means the timer already expired or was stopped.
            // Defensive: stay idle.
            
            return BTN_IDLE;

        default:
            return BTN_IDLE;
    }
}


// BTN_DEBOUNCING
// Edge was seen. Timer is running. Line may still be bouncing.
// Waiting for silence (timer expiry) before confirming anything.
static BtnState_t state_debouncing(ButtonSM_t *sm, BtnEvent_t e)
{
    switch (e) {
        case EVT_FALLING_EDGE:
        case EVT_RISING_EDGE:
            
            //Another edge during bounce window — line still noisy.
            //Restart timer: silence window resets from NOW.
            //We stay in DEBOUNCING — no entry action re-fires
            //(self-transition is not a state change).
            
            timer_restart(sm);
            return BTN_DEBOUNCING;

        case EVT_TIMER_EXP:
            
             // 50ms(say) of silence. Line is stable. Read pin NOW to find
             // the true settled state — do not assume based on which
             // edge first triggered debounce; bouncing could have
             // left the pin in either level.
             // Transition target drives entry action which emits
             // the appropriate app event.
            
            return (read_pin() == 0) ? BTN_PRESSED : BTN_IDLE;

        default:
            return BTN_DEBOUNCING;
    }
}


// BTN_PRESSED
// Stable pressed. Waiting for release activity.

static BtnState_t state_pressed(ButtonSM_t *sm, BtnEvent_t e)
{
    //(void)sm;     -- now that we are actually using sm in our function body - we don't need to void cast
    switch (e) {
        case EVT_RISING_EDGE:
            
            // Pin went HIGH — possible release starting.
            // Transition to DEBOUNCING; on_enter_debouncing() will
            // start the timer.
            
            if(!sm->isLongPressFired)    // if 'long press app event' is not yet sent
            {
                timer_stop(sm);
                emit_app_event(sm, APP_EVT_BTN_PRESS);  // conclude as normal press
            }

            return BTN_DEBOUNCING;

        case EVT_FALLING_EDGE:
            // Pin LOW while already pressed — spurious, ignore.
            return BTN_PRESSED;

        case EVT_TIMER_EXP:         // timer has elapsed and stopped
            
            // long press has been detected 
            emit_app_event(sm, APP_EVT_BTN_LONG_PRESS);
            sm->isLongPressFired = true;
            
            return BTN_PRESSED;     // state remains BTN_PRESSED

        default:
            return BTN_PRESSED;
    }
}

//  Internal helpers

// read_pin()
// Returns 0 if LOW (pressed), 1 if HIGH (released).
// Active-low inversion handled HERE — caller sees logical level.
// TivaWare confined to this function.

static uint8_t read_pin(void)
{
    return (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) != 0) ? 1 : 0;
}


// timer_start()
// Arms the one-shot debounce timer.

static void timer_start(ButtonSM_t *sm)
{
    if(sm->state == BTN_PRESSED)
        osTimerStart(sm->debounce_timer,
                 osKernelGetTickFreq() * sm->longPressTimeSec);
    else
        osTimerStart(sm->debounce_timer,
                 osKernelGetTickFreq() * sm->debounceTimeMs / 1000U);
}

// stops the timer
static void timer_stop(ButtonSM_t* sm)
{
    osTimerStop(sm->debounce_timer);
}

// timer_restart()
// Resets the debounce window from now.
// Called on bounce edges arriving during DEBOUNCING.
static void timer_restart(ButtonSM_t *sm)
{
    timer_stop(sm);
    timer_start(sm);
}

// emit_app_event()
// Posts a logical button event to the app layer queue.
// Non-blocking — if queue is full, event is dropped.
static void emit_app_event(ButtonSM_t *sm, AppEvent_t evt)
{
    osMessageQueuePut(sm->app_event_queue, &evt, 0, 0);
}
