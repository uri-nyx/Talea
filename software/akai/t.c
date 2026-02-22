/* test_ll.c - Minimal test for 8-byte register pairs */

/* Use a global to force a LOADI8 from memory */
long long global_val = 0x1122334455667788L;

/* Stub function to check ARGI8 and register selection */
void sink(long long a, int b) {
    /* No-op */
}

int main(void) {
    /* Test loading and passing */
    sink(global_val, 0xAA);
    return 0;
}
