#include <stdio.h>
#include <stdint.h>

void rotate_right(int32_t *arr, uint32_t len, uint32_t k);

void swap(int32_t *p, int32_t *q)
{
    int32_t temp = *q;
    *q = *p;
    *p = temp;
    return;
}

void reverse_arr(int32_t *arr, uint32_t len)
{
    uint32_t index = 0, pos = 0;
    printf("len = %u\n",len);
    // reverse the original array in place
    for(index = 0, pos = (len-1); (index < pos); index ++, pos--)
    {
        printf("index = %u | pos = %u\n", index, pos);
        swap(&arr[index], &arr[pos]);
    }
}

void rotate_right(int32_t *arr, uint32_t len, uint32_t k)
{
    if((k == 0) || (k == len) || (len ==0))
        return;

    if(k > len)
        k = k % len;	

    if(len == 1)
        return;

    reverse_arr(arr, len);

    reverse_arr(arr, k);
    
    reverse_arr(arr+k, len-k);
}

/* ---------------- Test Harness ---------------- */
typedef struct {
    int32_t arr[8];
    uint32_t len;
    uint32_t k;
    int32_t expected[8];
} test_case_t;

static int arrays_equal(const int32_t *a, const int32_t *b, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
        if (a[i] != b[i]) return 0;
    return 1;
}

int main(void)
{
    test_case_t tests[] = {
        { {1,2,3,4,5,6,7}, 7, 3, {5,6,7,1,2,3,4} },
        { {1,2,3,4,5},     5, 1, {5,1,2,3,4} },
        { {1,2,3},         3, 0, {1,2,3} },
        { {1,2,3},         3, 3, {1,2,3} },   // k == len, no-op
        { {1,2,3},         3, 4, {3,1,2} },   // k > len, must wrap
        { {1},             1, 5, {1} },
    };

    uint32_t num_tests = sizeof(tests) / sizeof(tests[0]);
    uint32_t pass = 0;

    for (uint32_t i = 0; i < num_tests; i++) {
        rotate_right(tests[i].arr, tests[i].len, tests[i].k);
        if (arrays_equal(tests[i].arr, tests[i].expected, tests[i].len)) {
            printf("Test %u: PASS\n", i);
            pass++;
        } else {
            printf("Test %u: FAIL\n", i);
        }
    }

    printf("\n%u/%u tests passed\n", pass, num_tests);
    return 0;
}
