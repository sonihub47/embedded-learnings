`Long Press`

On top of the existing 05_ButtonDebounce_SM — detect a long press. If the button is held down for 2 seconds after a confirmed press, emit APP_EVT_BTN_LONG_PRESS upward. If the button is released before 2 seconds — just a normal press, no long press event.

Constraints
C1. No new states — long press is detectable entirely within BTN_PRESSED.
C2. The long press timer must be cancelled if the button is released before 2 seconds. No ghost events.
C3. After a long press fires — the machine stays in BTN_PRESSED. The button is still physically held down.
C4. If the button is held beyond 2 seconds — emit once, not repeatedly. No re-firing on the same hold.
C5. ButtonSM_t gets exactly one new field. common_events.h gets exactly one new entry.