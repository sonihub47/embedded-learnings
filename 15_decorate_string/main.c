#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// ----------------------------------------------------------------------
// TODO: Implement this function.
//
// Rules:
//   1. Every contiguous group of '!' characters gets one extra '!' appended
//      (a single '!' is also a "group" of size 1 -> becomes 2).
//   2. AFTER step 1 is fully done, every '.' character is replaced with '!'.
//      (Dots do NOT participate in the grouping logic in step 1 -- a '.'
//       does not extend or merge with a neighboring '!' group.)
//
// You decide: does the buffer need to grow? Who owns the returned memory?
// (malloc vs modifying in place vs fixed-size caller-provided buffer --
//  pick your approach, just be consistent and don't overflow anything.)
// ----------------------------------------------------------------------
char* decorate(char* str) {
    // YOUR CODE HERE
    int32_t curr = 0, prev = -1;        // prev = -1 => invalid

    if(strlen(str) == 0)
        return str;

    uint32_t ori_len = strlen(str);

    do
    {
        if((str[prev] == '!') && (str[curr] != '!') && (prev != -1))       // transition detected
        {
            if(str[curr] != '\0')
                memmove(&str[curr+1], &str[curr], ori_len - curr + 1);
            else
                memmove(&str[curr+1], &str[curr], 1);
            
            str[curr] = '!';

            prev = -1;  // so that new ! is not considered for transition 
        }
        else
        {
            prev = curr;
        }

        curr++;
    }while(str[curr-1] != '\0');

    curr = 0;                   // reset
    while(str[curr] != '\0')
    {
        // convert . to ! logic
        if(str[curr] == '.')
            str[curr] = '!';
        
        curr++;
    }

    return str;
}


// Test harness
// ----------------------------------------------------------------------
typedef struct {
    const char* input;
    const char* expected;
} TestCase;

int run_test(const char* input, const char* expected) {
    // Worst case: EVERY character in input is '!' or '.', and every one of
    // them expands (a string of all '!' is one giant group -> +1 total;
    // but to be safe and simple we size generously: 2x + 1 covers any
    // plausible expansion this problem could produce).
    size_t cap = (strlen(input) * 2) + 1;
    char* buf = malloc(cap);
    strcpy(buf, input);

    // If your implementation writes in place (e.g. memmove-ing bytes right
    // to open a gap for the extra '!'), it now has real slack to do that
    // safely inside `buf`. If instead you malloc a brand new buffer inside
    // decorate() and return that, `buf` is simply unused after the call --
    // either approach is fine, the harness doesn't care which you pick.
    char* result = decorate(buf);

    int pass = (result != NULL) && (strcmp(result, expected) == 0);

    printf("Input:    \"%s\"\n", input);
    printf("Expected: \"%s\"\n", expected);
    printf("Got:      \"%s\"\n", result ? result : "(NULL)");
    printf("Result:   %s\n\n", pass ? "PASS" : "FAIL");

    // NOTE: if your decorate() mallocs a NEW buffer, you may want to free
    // both `result` and `buf` here depending on your implementation.
    free(buf);
    return pass;
}

int main() {
    TestCase tests[] = {
        // Given example
        { "xx!xx!!xx!!!",
          "xx!!xx!!!xx!!!!" },

        // No special chars at all
        { "Just plain text", "Just plain text" },

        // Only dots
        { "...", "!!!" },

        // Single trailing bang
        { "Wow!", "Wow!!" },

        // Multiple groups back to back with space
        { "! !! !!!", "!! !!! !!!!" },

        // Dot sits between two separate '!' groups. Since grouping (step 1)
        // happens BEFORE dot-conversion (step 2), the '.' does NOT merge the
        // two '!' groups together -- each '!' is its own group of size 1,
        // independently expanded, and only afterward does '.' become '!'.
        //   "Wait!.!"
        //   -> step 1: "!" (grp of 1) -> "!!" ,  "!" (grp of 1) -> "!!"
        //              gives "Wait!!.!!"
        //   -> step 2: '.' -> '!'
        //              gives "Wait!!!!!"
        { "Wait!.!", "Wait!!!!!" },

        // Empty string
        { "", "" },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;


    for (int i = 0; i < num_tests; i++) {
        printf("--- Test %d ---\n", i + 1);
        if (run_test(tests[i].input, tests[i].expected)) {
            passed++;
        }
    }
    printf("========================\n");
    printf("Passed %d / %d\n", passed, num_tests);

    return 0;
}
