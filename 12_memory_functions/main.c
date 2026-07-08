/* =====================================================================
 * ARM-style CoderPad practice exercise #2
 * Topic: Manual memcpy / memmove (pointer arithmetic, buffer overlap)
 *
 * TIME LIMIT: 20 minutes. Set a timer. Stop when it rings, run the tests,
 * and see how far you got — then review whatever failed.
 *
 * HOW TO USE:
 *   1. Read the problem statement below.
 *   2. Implement the TODO functions.
 *   3. Compile & run:  gcc -Wall -Wextra arm_practice_memcpy.c -o test && ./test
 *   4. Re-run after every change until all tests PASS.
 * =====================================================================
 *
 * PROBLEM:
 * Implement your own versions of memcpy and memmove without using the
 * standard library's memcpy/memmove/memmove-family functions.
 *
 *   1. void *my_memcpy(void *dest, const void *src, size_t n);
 *      Copies n bytes from src to dest. Behavior is undefined if the
 *      regions overlap (same contract as real memcpy) - so you do NOT
 *      need to handle overlap here. Return dest.
 *
 *   2. void *my_memmove(void *dest, const void *src, size_t n);
 *      Copies n bytes from src to dest, and MUST behave correctly even
 *      when the source and destination buffers overlap. Return dest.
 *
 * Constraints / things interviewers commonly probe:
 *   - Why can memcpy get away with UB on overlap but memmove can't?
 *   - What determines which copy direction (forward vs backward) is
 *     safe for a given overlap case? (Hint: compare dest vs src address.)
 *   - n == 0 and dest == src should both be handled gracefully (no-ops
 *     that don't crash or corrupt data).
 *   - You may only use byte-level (unsigned char*) pointer arithmetic -
 *     no casting to a wider type to "cheat" the copy.
 * =====================================================================
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>  /* only used by the test harness for setup/verify, not by your code */

/* ---------------------- TODO: implement these ---------------------- */

void *my_memcpy(void *dest, const void *src, size_t n)
{
    if(!dest || !src)
        return NULL;
    
    if(n <= 0)
        return NULL;

    if(dest == src)
        return dest;
    
    unsigned int counter = 0;
    for(counter =0; counter < n; counter++)
    {
        *((unsigned char*)dest + counter) = *((unsigned char*)src + counter);
    }
    return dest;
}

void *my_memmove(void *dest, const void *src, size_t n)
{
    if(!dest || !src)
        return NULL;
    
    if(n <= 0)
        return NULL;

    if(dest == src)
        return dest;

    unsigned int counter = 0;

    if(dest > src)
    {
        for(counter =(n-1); counter >= 0; counter--)
        {
            *((unsigned char*)dest + counter) = *((unsigned char*)src + counter);
        }
    }
    
    if(dest < src)
    {
        for(counter =0; counter < n; counter++)
        {
            *((unsigned char*)dest + counter) = *((unsigned char*)src + counter);
        }
    }

    return dest;
}

/* ------------------------- TEST HARNESS ----------------------------- */
/* Do not need to edit below this line. Just run and iterate. */

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(desc, cond) do { \
    if (cond) { g_pass++; printf("[PASS] %s\n", desc); } \
    else      { g_fail++; printf("[FAIL] %s\n", desc); } \
} while (0)

int main(void)
{
    unsigned char buf[32];
    unsigned char expected[32];

    /* --- my_memcpy: basic non-overlapping copy --- */
    {
        unsigned char src[8]  = {1,2,3,4,5,6,7,8};
        unsigned char dst[8]  = {0};
        my_memcpy(dst, src, 8);
        CHECK("my_memcpy: basic copy", memcmp(dst, src, 8) == 0);
    }

    /* --- my_memcpy: zero length is a no-op --- */
    {
        unsigned char src[4] = {9,9,9,9};
        unsigned char dst[4] = {1,2,3,4};
        my_memcpy(dst, src, 0);
        CHECK("my_memcpy: n=0 no-op", dst[0]==1 && dst[1]==2 && dst[2]==3 && dst[3]==4);
    }

    /* --- my_memcpy: return value is dest --- */
    {
        unsigned char src[4] = {1,2,3,4};
        unsigned char dst[4] = {0};
        void *ret = my_memcpy(dst, src, 4);
        CHECK("my_memcpy: returns dest pointer", ret == (void *)dst);
    }

    /* --- my_memmove: non-overlapping behaves like memcpy --- */
    {
        unsigned char src[8] = {1,2,3,4,5,6,7,8};
        unsigned char dst[8] = {0};
        my_memmove(dst, src, 8);
        CHECK("my_memmove: non-overlapping copy", memcmp(dst, src, 8) == 0);
    }

    /* --- my_memmove: dest overlaps src, dest > src (forward overlap) --- */
    {
        /* buf: 1 2 3 4 5 6 7 8 ... ; copy buf[0..5] into buf[2..7] */
        for (int i = 0; i < 8; i++) buf[i] = (unsigned char)(i + 1);
        for (int i = 0; i < 8; i++) expected[i] = buf[i];
        memmove(expected + 2, expected, 6); /* reference result via libc */
        my_memmove(buf + 2, buf, 6);
        CHECK("my_memmove: dest > src overlap", memcmp(buf, expected, 8) == 0);
    }

    /* --- my_memmove: dest overlaps src, dest < src (backward overlap) --- */
    {
        for (int i = 0; i < 8; i++) buf[i] = (unsigned char)(i + 1);
        for (int i = 0; i < 8; i++) expected[i] = buf[i];
        memmove(expected, expected + 2, 6);
        my_memmove(buf, buf + 2, 6);
        CHECK("my_memmove: dest < src overlap", memcmp(buf, expected, 8) == 0);
    }

    /* --- my_memmove: dest == src is a safe no-op --- */
    {
        unsigned char data[4] = {5,6,7,8};
        unsigned char before[4];
        memcpy(before, data, 4);
        my_memmove(data, data, 4);
        CHECK("my_memmove: dest==src no-op", memcmp(data, before, 4) == 0);
    }

    /* --- my_memmove: large overlap, full buffer shift --- */
    {
        unsigned char big[16], big_expected[16];
        for (int i = 0; i < 16; i++) big[i] = (unsigned char)i;
        memcpy(big_expected, big, 16);
        memmove(big_expected + 4, big_expected, 12);
        my_memmove(big + 4, big, 12);
        CHECK("my_memmove: large forward overlap", memcmp(big, big_expected, 16) == 0);
    }

    printf("\n========================================\n");
    printf("Results: %d passed, %d failed (out of %d)\n",
           g_pass, g_fail, g_pass + g_fail);
    printf("========================================\n");

    return (g_fail == 0) ? 0 : 1;
}