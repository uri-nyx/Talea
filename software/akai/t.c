/* test_ll.c - Minimal test for 8-byte register pairs */

/* Use a global to force a LOADI8 from memory */
// long long global_val = 0x11AABBCCDDEEFFEELL;

struct S {
    int   x, y, z;
    float f, g;
};

/* Stub function to check ARGI8 and register selection
 */
// void sink(long long a, int b)
// {
//     /* No-op */
//     long long c = a;
//}

struct S struct_val(struct S n, int c, float d)
{
    struct S z;
	n.x = 512;
	n.y = 1024;
    z.y = n.y;
    z.x = c;
    z.f = d;
    return z;
}

int main(void)
{
    /* Test loading and passing */
    // long long local_val = 0x1122334455667788LL;
    struct S A = { 0, 1, 2, 1.0f, -1.0 };
    struct S B;

    // sink(global_val, 0xAA);
    B = struct_val(A, 18, 40.32f);
    // sink(global_val, B.y);
    B.x = 100;

    return 0;
}
