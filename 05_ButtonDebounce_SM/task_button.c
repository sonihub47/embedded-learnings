// Layer:   application
// Owns:    RTOS object creation, GPIO ISR, timer callback, task loop
//
// This file is the wiring layer. It:
//   - creates all RTOS objects (queues, timer, task)
//   - owns the GPIO ISR — posts raw edge events, nothing else
//   - owns the timer callback — posts EVT_TIMER_EXP, nothing else
//   - runs the task loop — drains queue, calls dispatcher
//
// It does NOT contain any state machine logic.
// It does NOT know about debounce algorithm internals.

#include "cmsis_os2.h"
#include "button_sm.h"
#include "common_events.h"

// TivaWare — GPIO ISR needs it, confined to this file in app layer
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"

//  RTOS objects                                                      
//  Defined here — passed into SM at init
static osMessageQueueId_t  btn_event_queue;     // ISR/timer -> task   
static osMessageQueueId_t  app_event_queue;     // SM -> app consumer  
static osTimerId_t         debounce_timer;

//  State machine instance — one per button                           
static ButtonSM_t g_btn;

//  GPIO ISR                                                          
//                                                                    
//  Responsibility: detect raw edge, post event, return.              
//  No debounce logic. No state machine knowledge. No blocking.       
//                                                                    
//  Why both edges?                                                   
//  The SM needs to know about rising edges too (release path).       
//  The ISR is edge-agnostic — it just reports what happened.         
void GPIOF_Handler(void)
{
    // read and clear interrupt status in one step
    uint32_t status = GPIOIntStatus(GPIO_PORTF_BASE, true);
    GPIOIntClear(GPIO_PORTF_BASE, status);

    if (status & GPIO_PIN_4) {
        // read pin to determine edge direction
        uint8_t pin_val = (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) != 0);

        BtnEvent_t evt = pin_val ? EVT_RISING_EDGE : EVT_FALLING_EDGE;

        // non-blocking — ISR must never block
        osMessageQueuePut(btn_event_queue, &evt, 0, 0);
    }

    // Flow from here:
    // btn_event_queue ← event sitting here
    // button_task() unblocks from osMessageQueueGet()
    // button_sm_dispatch() called with this event
}

//  Timer callback                                                  
//                                                                  
//  Runs in RTOS timer daemon task context — must not block.        
//  Responsibility: post EVT_TIMER_EXP. Nothing else.               
//  SM decides what to do with it — not the callback's concern.     
static void debounce_timer_cb(void *arg)
{
    (void)arg;
    BtnEvent_t evt = EVT_TIMER_EXP;
    osMessageQueuePut(btn_event_queue, &evt, 0, 0);

    // Flow from here:
    // Same path as ISR — event lands in btn_event_queue,
    // task unblocks, dispatcher called with EVT_TIMER_EXP.
    // state_debouncing() reads pin and emits APP_EVT_BTN_PRESS
    // or APP_EVT_BTN_RELEASE into app_event_queue.
}

//  GPIO hardware init            
static void gpio_hw_init(void)
{
    // enable clock, configure PF4 as input with pull-up,
    // set both-edges interrupt, enable in NVIC         
    // ... TivaWare calls ... 

    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_BOTH_EDGES);
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4);
    IntEnable(INT_GPIOF);
}

//  Button task                                                      
//                                                                   
//  Sole job: drain btn_event_queue and feed events to dispatcher.   
//  Blocked on queue — consumes zero CPU when nothing is happening.  
//  Wakes only when ISR or timer callback posts an event.            
void button_task(void *arg)
{
    (void)arg;

    // create RTOS objects
    btn_event_queue = osMessageQueueNew(8, sizeof(BtnEvent_t), NULL);
    app_event_queue = osMessageQueueNew(8, sizeof(AppEvent_t), NULL);

    debounce_timer  = osTimerNew(debounce_timer_cb,
                                 osTimerOnce,       // one-shot      
                                 NULL, NULL);

    // init hardware 
    gpio_hw_init();

    // init state machine 
    button_sm_init(&g_btn,
                   btn_event_queue,
                   app_event_queue,
                   debounce_timer,
                   g_btn->debounceTimeMs); // debounce window

    // event loop 
    BtnEvent_t evt;

    while (1) {
        
		// Block here — task is sleeping, consuming no CPU.
        // Wakes when GPIOF_Handler() or debounce_timer_cb()
        // posts to btn_event_queue.
        
        osMessageQueueGet(btn_event_queue, &evt, NULL, osWaitForever);

        // Hand event to dispatcher.
        // Dispatcher calls the correct state function.
        // State function returns next state.
        // Dispatcher writes sm->state — only place it is written.
        // If transition confirmed: state function already called
        // emit_app_event() before returning.
        
        button_sm_dispatch(&g_btn, evt);
    }
}
