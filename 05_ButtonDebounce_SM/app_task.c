// Layer:   app (consumer side)
// Owns:    application logic — responds to confirmed button events
// 
// Blocks on app_event_queue. Wakes only when button_sm.c confirms
// a stable press or release and posts APP_EVT_BTN_PRESS/RELEASE.
// 
// This file knows nothing about:
//   - GPIO, TivaWare
//   - debounce algorithm
//   - BtnEvent_t (raw events never reach here)
//   - btn_event_queue (internal to button driver/task)

#include "cmsis_os2.h"
#include "common_events.h"

//  app_event_queue                                                 
//  Defined in task_button.c — extern here so app task can read it  
extern osMessageQueueId_t app_event_queue;

//  App task                                                        
//                                                                  
//  Full path that leads here:                                      
//  PF4 edge -> GPIOF_Handler -> btn_event_queue                      
//  -> button_task -> button_sm_dispatch                              
//  -> state_debouncing (EVT_TIMER_EXP)                              
//  -> read_pin() -> emit_app_event()                                 
//  -> app_event_queue  ← we wake up HERE                            
void app_task(void *arg)
{
    (void)arg;

    AppEvent_t evt;

    while (1) {
        osMessageQueueGet(app_event_queue, &evt, NULL, osWaitForever);

        switch (evt) {
            case APP_EVT_BTN_PRESS:
                // confirmed press — do application work
                break;

            case APP_EVT_BTN_RELEASE:
                // confirmed release — do application work
                break;

            default:
                break;
        }
    }
}

