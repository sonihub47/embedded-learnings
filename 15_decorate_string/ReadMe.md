#Decorate string: Implement this function.

Rules:
  1. Every contiguous group of '!' characters gets one extra '!' appended
     (a single '!' is also a "group" of size 1 -> becomes 2).
  2. AFTER step 1 is fully done, every '.' character is replaced with '!'.
     (Dots do NOT participate in the grouping logic in step 1 -- a '.'
      does not extend or merge with a neighboring '!' group.)
	  
Example - 

Input string = Hello! Hope you are fine. Great !! Take care !!!
Output string = Hello!! Hope you are fine! Great !!! Take care !!!!

You decide: does the buffer need to grow? Who owns the returned memory?
(malloc vs modifying in place vs fixed-size caller-provided buffer --
 pick your approach, just be consistent and don't overflow anything.)
 
  #Copying/moving strings in overlapping regions = Use memmove !!


 ---

 #Update

 The version committed is expensive in terms of multiple memory function invocations. 

 There is a better approach to it called "Backward two-pointer fill"

The core idea: two passes — count first, then fill from the back.

Pass 1 (left to right): figure out the final length before touching anything.

Pass 2 (right to left): write directly into final positions, no shifting.

Why backward is the trick that makes it safe: writing forward while growing the 
string means you keep overwriting data you haven't read yet (that's exactly the
shifting problem you were fighting). Writing backward, the write pointer is always ahead 
of the read pointer (since new_len ≥ old_len), so you never clobber a character before you've read it.
 ---


