#Rotate Array In-Place (right rotation by k)
Context: Given an array and a rotation count k, rotate the array to the right by k positions, in-place (no second array). This is a classic screening question because the naive solution (extra buffer) is O(n) space, but the expected solution is O(1) space using the reversal algorithm — a technique interviewers love because it tests whether you know a non-obvious trick, similar to XOR just now.

#Hint to get you started (since this trick is new): the reversal algorithm does 3 reversals:

Reverse the whole array.
Reverse the first k elements.
Reverse the remaining len - k elements.