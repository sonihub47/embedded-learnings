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