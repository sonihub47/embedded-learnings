

/* =====================================================================
 * ARM-style CoderPad practice exercise
 * Topic: Register bit manipulation (very common in ARM/embedded C screens)
 *
 * HOW TO USE:
 *   1. Read the problem statement below.
 *   2. Implement the TODO functions.
 *   3. Compile & run:  gcc -Wall -Wextra arm_practice_bitops.c -o test && ./test
 *   4. Re-run after every change until all tests PASS.
 *
 * This mimics the CoderPad flow: edit -> run tests -> see pass/fail -> repeat.
 * =====================================================================
 *
 * PROBLEM:
 * You are working with a 32-bit memory-mapped peripheral status/control
 * register. Implement the following utility functions:
 *
 *   1. void set_bits(uint32_t *reg, uint32_t mask);
 *      Set (to 1) all bit positions indicated by mask, without disturbing
 *      other bits.
 *
 *   2. void clear_bits(uint32_t *reg, uint32_t mask);
 *      Clear (to 0) all bit positions indicated by mask, without
 *      disturbing other bits.
 *
 *   3. void toggle_bits(uint32_t *reg, uint32_t mask);
 *      Invert all bit positions indicated by mask.
 *
 *   4. int is_bit_set(uint32_t reg, uint32_t bit_pos);
 *      Return 1 if bit at bit_pos (0-31) is set, else 0.
 *      bit_pos is a *position* (0..31), not a mask.
 *
 *   5. uint32_t extract_field(uint32_t reg, uint32_t shift, uint32_t width);
 *      Extract a 'width'-bit field starting at bit position 'shift' and
 *      return it right-aligned (i.e., as if shifted down to bit 0).
 *      Example: reg = 0xABCD1234, shift = 8, width = 8 -> should return 0x12.
 *
 *   6. int popcount32(uint32_t val);
 *      Return the number of set bits (population count) in val.
 *      Do NOT use __builtin_popcount (write it manually - this is often
 *      explicitly asked in interviews to test loop/bit-shift fluency).
 *
 * Constraints / things interviewers commonly probe:
 *   - Don't assume mask has only one bit set for set/clear/toggle.
 *   - Be careful with signed vs unsigned shifts.
 *   - extract_field must work correctly even when shift+width == 32.
 * =====================================================================
 */
 
#include <stdio.h>
#include <stdint.h>
 
/* ---------------------- TODO: implement these ---------------------- */
 
void set_bits(uint32_t *reg, uint32_t mask)
{
    *reg |= mask;
}
 
void clear_bits(uint32_t *reg, uint32_t mask)
{
    *reg &= ~mask;
}
 
void toggle_bits(uint32_t *reg, uint32_t mask)
{
    *reg ^= mask;
}
 
int is_bit_set(uint32_t reg, uint32_t bit_pos)
{
    if(reg & (1u << bit_pos))
        return 1;
    else
        return 0;
}
 
uint32_t extract_field(uint32_t reg, uint32_t shift, uint32_t width)
{
    uint32_t mask = 0xFFFFFFFF << width;
    
    // In C, shifting a value by an amount equal to or greater than the bit width of the type is undefined behavior.
    // Never rely on shifting by the full width of the type.
    //Always handle that case separately to avoid undefined behavior.
    
    if(width == 32)
        return reg;
    reg >>= shift;
    reg &= ~mask;
    return reg;
}
 
int popcount32(uint32_t val)
{
    int count = 0;
    uint32_t bit_pos = 0;

    do{
        if(is_bit_set(val, bit_pos))
            count++;

        bit_pos++;
    }while(bit_pos < 32);


    return count;
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
    uint32_t reg;
 
    /* set_bits */
    reg = 0x00000000;
    set_bits(&reg, 0x0000000F);
    CHECK("set_bits: sets low nibble", reg == 0x0000000F);
 
    reg = 0x000000F0;
    set_bits(&reg, 0x0000000F);
    CHECK("set_bits: preserves existing bits", reg == 0x000000FF);
 
    /* clear_bits */
    reg = 0xFFFFFFFF;
    clear_bits(&reg, 0x0000000F);
    CHECK("clear_bits: clears low nibble", reg == 0xFFFFFFF0);
 
    reg = 0x0000000F;
    clear_bits(&reg, 0xFFFFFFFF);
    CHECK("clear_bits: clears everything", reg == 0x00000000);
 
    /* toggle_bits */
    reg = 0x0000000F;
    toggle_bits(&reg, 0x000000FF);
    CHECK("toggle_bits: flips target byte", reg == 0x000000F0);
 
    /* is_bit_set */
    reg = 0x00000004; /* bit 2 set */
    CHECK("is_bit_set: bit 2 set", is_bit_set(reg, 2) == 1);
    CHECK("is_bit_set: bit 0 not set", is_bit_set(reg, 0) == 0);
    CHECK("is_bit_set: bit 31 not set", is_bit_set(reg, 31) == 0);
 
    reg = 0x80000000;
    CHECK("is_bit_set: bit 31 set (MSB edge case)", is_bit_set(reg, 31) == 1);
 
    /* extract_field */
    reg = 0xABCD1234;
    CHECK("extract_field: middle byte", extract_field(reg, 8, 8) == 0x12);
    CHECK("extract_field: top byte", extract_field(reg, 24, 8) == 0xAB);
    CHECK("extract_field: full width edge case",
          extract_field(0xFFFFFFFF, 0, 32) == 0xFFFFFFFF);
    CHECK("extract_field: single bit", extract_field(0x00000008, 3, 1) == 1);
 
    /* popcount32 */
    CHECK("popcount32: zero", popcount32(0x00000000) == 0);
    CHECK("popcount32: all ones", popcount32(0xFFFFFFFF) == 32);
    CHECK("popcount32: alternating bits", popcount32(0xAAAAAAAA) == 16);
    CHECK("popcount32: single bit", popcount32(0x00000001) == 1);
 
    printf("\n========================================\n");
    printf("Results: %d passed, %d failed (out of %d)\n",
           g_pass, g_fail, g_pass + g_fail);
    printf("========================================\n");
 
    return (g_fail == 0) ? 0 : 1;
}
 
