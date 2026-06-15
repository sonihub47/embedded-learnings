`Event-driven state machine — per-state function pattern with entry/exit actions`

> Current State + Event => Transition (next state) + Actions

Actions could be conditional(depends of state|event combo) or unconditional 
(same for a specific state - while entering or exiting)

Conditional actions live inside state functions — because they depend on which 
transition caused the entry, not just which state was entered.
Unconditional actions live in entry/exit hooks — fires regardless of source, 
keeping the "what changes about the world" logic in one place.


A state machine without events is a sequential executor — not the same thing.
It drives itself forward by doing work inside state bodies and self-transitioning.
Works for simple linear flows but has no natural place for timeouts, async errors,
or external interrupts — leading to frozen states that are hard to debug.

