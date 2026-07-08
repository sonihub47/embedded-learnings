```In C, shifting a value by an amount equal to or greater than the bit width of the type is undefined behavior.
Never rely on shifting by the full width of the type.
Always handle that case separately to avoid undefined behavior.```

# "why do-while(0) in macros") is a genuinely common one in embedded C interviews (TI, ARM, etc.)
since it tests whether you understand macro expansion is textual substitution, not a function call
— worth having a crisp one-line answer ready:
"it makes the macro act like a single statement so it's safe in if/else without braces."
