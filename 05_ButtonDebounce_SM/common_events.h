// Neutral shared contract — owned by neither driver nor app layer.
// Both button_sm.c (driver) and task_button.c (app) include this.
// Layer:   shared / neutral
// Owns:    application-level event vocabulary

#ifndef COMMON_EVENTS_H
#define COMMON_EVENTS_H

typedef enum {
    APP_EVT_BTN_PRESS,
    APP_EVT_BTN_RELEASE,
} AppEvent_t;

#endif /* COMMON_EVENTS_H */