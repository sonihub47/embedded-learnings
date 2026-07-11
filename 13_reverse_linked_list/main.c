#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    int data;
    struct node *next;
} node_t;

node_t *reverse_list(node_t *head);

node_t *reverse_list(node_t *head)
{
    node_t* prev, *next, *current;

    if((head == NULL) || (head->next == NULL))
        return head;

    current = head;
    next = head->next;
    current->next = NULL;
    prev = current;

    while(next != NULL)
    {
        current = next;
        next = current->next;
        current->next = prev;
        prev = current;
    }

    head = current;

    return head;
}

/* ---------------- Test Harness ---------------- */
static node_t *make_list(int *vals, int n)
{
    node_t *head = NULL, *tail = NULL;
    for (int i = 0; i < n; i++) {
        node_t *nn = malloc(sizeof(node_t));
        nn->data = vals[i];
        nn->next = NULL;
        if (!head) head = tail = nn;
        else { tail->next = nn; tail = nn; }
    }
    return head;
}

static int lists_equal(node_t *a, int *expected, int n)
{
    for (int i = 0; i < n; i++) {
        if (!a || a->data != expected[i]) return 0;
        a = a->next;
    }
    return (a == NULL);  // must also be exhausted, not longer than expected
}

static void free_list(node_t *head)
{
    while (head) { node_t *tmp = head; head = head->next; free(tmp); }
}

int main(void)
{
    struct {
        int vals[6];
        int n;
        int expected[6];
    } tests[] = {
        { {1,2,3,4,5}, 5, {5,4,3,2,1} },
        { {1},         1, {1} },
        { {1,2},       2, {2,1} },
        { {0},         0, {0} },  // n=0, empty list — expected unused
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int pass = 0;

    for (int i = 0; i < num_tests; i++) {
        node_t *head = make_list(tests[i].vals, tests[i].n);
        node_t *rev = reverse_list(head);

        int ok = (tests[i].n == 0) ? (rev == NULL)
                                     : lists_equal(rev, tests[i].expected, tests[i].n);
        printf("Test %d: %s\n", i, ok ? "PASS" : "FAIL");
        if (ok) pass++;

        free_list(rev);
    }

    printf("\n%d/%d tests passed\n", pass, num_tests);
    return 0;
}
