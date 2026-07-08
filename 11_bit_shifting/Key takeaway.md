```In C, shifting a value by an amount equal to or greater than the bit width of the type is undefined behavior.
Never rely on shifting by the full width of the type.
Always handle that case separately to avoid undefined behavior.```