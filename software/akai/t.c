/*
 * COMPREHENSIVE TEST SUITE FOR SIRIUS BACKEND
 * Tests ALL data types, operations, and edge cases
 */

//#include <stdio.h>

// ============================================================================
// SECTION 1: 64-BIT INTEGER TESTS
// ============================================================================

// Test basic 64-bit constants
long long test_ll_constants(void) {
    long long a = 0x0011223344556677LL;
    long long b = 0xFFEEDDCCBBAA9988LL;
    long long zero = 0LL;
    long long one = 1LL;
    long long neg_one = -1LL;
    
    return a + b + zero + one + neg_one;
}

// Test 64-bit arithmetic
long long test_ll_arithmetic(long long a, long long b) {
    long long sum = a + b;
    long long diff = a - b;
    long long product = sum * diff;  // May not be implemented yet
    return sum + diff;
}

// Test 64-bit bitwise
long long test_ll_bitwise(long long a, long long b) {
    return (a & b) | (a ^ b) | ~a;
}

// Test 64-bit shifts
long long test_ll_shifts(long long a) {
    long long left = a << 4;
    long long right = a >> 4;
    unsigned long long uright = ((unsigned long long)a) >> 4;
    return left + right + (long long)uright;
}

// Test 64-bit comparisons signed
int test_ll_compare_signed(long long a, long long b) {
    int result = 0;
    if (a == b) result |= (1 << 0);
    if (a != b) result |= (1 << 1);
    if (a < b)  result |= (1 << 2);
    if (a <= b) result |= (1 << 3);
    if (a > b)  result |= (1 << 4);
    if (a >= b) result |= (1 << 5);
    return result;
}

// Test 64-bit comparisons unsigned
int test_ll_compare_unsigned(unsigned long long a, unsigned long long b) {
    int result = 0;
    if (a == b) result |= (1 << 0);
    if (a != b) result |= (1 << 1);
    if (a < b)  result |= (1 << 2);
    if (a <= b) result |= (1 << 3);
    if (a > b)  result |= (1 << 4);
    if (a >= b) result |= (1 << 5);
    return result;
}

// Test conversions 32↔64
long long test_conversions_i32_to_i64(int x) {
    return x;  // Sign extend
}

long long test_conversions_u32_to_u64(unsigned int x) {
    return x;  // Zero extend
}

int test_conversions_i64_to_i32(long long x) {
    return (int)x;  // Truncate
}

unsigned int test_conversions_u64_to_u32(unsigned long long x) {
    return (unsigned int)x;  // Truncate
}

// ============================================================================
// SECTION 2: 32-BIT INTEGER TESTS
// ============================================================================

int test_i32_arithmetic(int a, int b) {
    return (a + b) * (a - b);
}

int test_i32_bitwise(int a, int b) {
    return (a & b) | (a ^ b) | ~a;
}

int test_i32_shifts(int a) {
    return (a << 3) + (a >> 3) + (((unsigned)a) >> 3);
}

int test_i32_compare(int a, int b) {
    int result = 0;
    if (a == b) result |= (1 << 0);
    if (a != b) result |= (1 << 1);
    if (a < b)  result |= (1 << 2);
    if (a > b)  result |= (1 << 3);
    return result;
}

// ============================================================================
// SECTION 3: 16-BIT AND 8-BIT INTEGER TESTS
// ============================================================================

short test_i16_ops(short a, short b) {
    return (a + b) - (a * b);
}

char test_i8_ops(char a, char b) {
    return (a + b) & (a | b);
}

unsigned short test_u16_ops(unsigned short a, unsigned short b) {
    return (a + b) ^ (a - b);
}

unsigned char test_u8_ops(unsigned char a, unsigned char b) {
    return (a * 2) + (b / 2);
}

// ============================================================================
// SECTION 4: POINTER TESTS
// ============================================================================

int test_pointer_arithmetic(int *ptr, int offset) {
    int *p = ptr + offset;
    return *p + *(p + 1);
}

long long test_ll_pointer(long long *ptr) {
    long long value = *ptr;
    ptr++;
    *ptr = value + 1;
    return *ptr;
}

void test_pointer_store(int *dest, int value) {
    *dest = value;
    *(dest + 1) = value + 1;
    *(dest + 2) = value + 2;
}

// ============================================================================
// SECTION 5: ARRAY TESTS
// ============================================================================

int test_array_i32(void) {
    int arr[5] = {10, 20, 30, 40, 50};
    int sum = 0;
	int i;
    for (i = 0; i < 5; i++) {
        sum += arr[i];
    }
    return sum;
}

long long test_array_i64(void) {
    long long arr[3] = {
        0x1111111111111111LL,
        0x2222222222222222LL,
        0x3333333333333333LL
    };
    return arr[0] + arr[1] + arr[2];
}

// ============================================================================
// SECTION 6: STRUCT TESTS (BY REFERENCE)
// ============================================================================

struct Point2D {
    int x;
    int y;
};

int test_struct_ref(struct Point2D *p) {
    return p->x + p->y;
}

void test_struct_modify(struct Point2D *p) {
    p->x = 100;
    p->y = 200;
}

struct Point2D test_struct_return_ref(void) {
    struct Point2D p;
    p.x = 42;
    p.y = 84;
    return p;
}

// ============================================================================
// SECTION 7: STRUCT TESTS (BY VALUE) - THE IMPORTANT ONES!
// ============================================================================

struct Small {
    int a;
    int b;
};

// Pass small struct by value
int test_struct_small_byval(struct Small s) {
    return s.a + s.b;
}

// Return small struct by value
struct Small test_struct_small_return(int x, int y) {
    struct Small s;
    s.a = x;
    s.b = y;
    return s;
}

struct Medium {
    int a;
    int b;
    int c;
    int d;
};

// Pass medium struct by value
int test_struct_medium_byval(struct Medium m) {
    return m.a + m.b + m.c + m.d;
}

// Return medium struct by value
struct Medium test_struct_medium_return(void) {
    struct Medium m;
    m.a = 1;
    m.b = 2;
    m.c = 3;
    m.d = 4;
    return m;
}

struct Large {
    long long x;
    long long y;
    int z;
};

// Pass large struct by value
long long test_struct_large_byval(struct Large l) {
    return l.x + l.y + l.z;
}

// Return large struct by value
struct Large test_struct_large_return(void) {
    struct Large l;
    l.x = 0x1111111111111111LL;
    l.y = 0x2222222222222222LL;
    l.z = 333;
    return l;
}

struct Nested {
    struct Small inner;
    int outer;
};

int test_struct_nested_byval(struct Nested n) {
    return n.inner.a + n.inner.b + n.outer;
}

// ============================================================================
// SECTION 8: MIXED STRUCT AND 64-BIT
// ============================================================================

struct With64 {
    long long big;
    int small;
};

long long test_struct_with_ll(struct With64 w) {
    return w.big + w.small;
}

struct With64 test_return_struct_with_ll(long long x) {
    struct With64 w;
    w.big = x;
    w.small = 42;
    return w;
}

// ============================================================================
// SECTION 9: FLOAT TESTS (if supported)
// ============================================================================

float test_float_arithmetic(float a, float b) {
    return (a + b) * (a - b);
}

float test_float_compare(float a, float b) {
    if (a < b) return 1.0f;
    if (a > b) return 2.0f;
    if (a == b) return 3.0f;
    return 0.0f;
}

float test_float_conversion(int x) {
    return (float)x;
}

int test_float_to_int(float x) {
    return (int)x;
}

// ============================================================================
// SECTION 10: DOUBLE TESTS (if supported)
// ============================================================================

double test_double_arithmetic(double a, double b) {
    return a * b + (a / b);
}

double test_double_conversion(long long x) {
    return (double)x;
}

long long test_double_to_ll(double x) {
    return (long long)x;
}

// ============================================================================
// SECTION 11: GLOBAL VARIABLE TESTS
// ============================================================================

int global_i32 = 12345;
long long global_i64 = 0x0123456789ABCDEFLL;
struct Small global_struct = {100, 200};

int test_global_i32(void) {
    global_i32 += 10;
    return global_i32;
}

long long test_global_i64(void) {
    global_i64 += 0x1111LL;
    return global_i64;
}

int test_global_struct(void) {
    global_struct.a *= 2;
    global_struct.b *= 3;
    return global_struct.a + global_struct.b;
}

// ============================================================================
// SECTION 12: CONTROL FLOW TESTS
// ============================================================================

int test_if_else(int x) {
    if (x < 0) {
        return -x;
    } else if (x > 100) {
        return 100;
    } else {
        return x;
    }
}

int test_switch(int x) {
    switch (x) {
        case 0: return 100;
        case 1: return 200;
        case 2: return 300;
        default: return -1;
    }
}

int test_while_loop(int n) {
    int sum = 0;
    int i = 0;
    while (i < n) {
        sum += i;
        i++;
    }
    return sum;
}

int test_for_loop(int n) {
    int sum = 0;
	int i;
    for (i = 0; i < n; i++) {
        sum += i * i;
    }
    return sum;
}

int test_do_while(int n) {
    int sum = 0;
    int i = 0;
    do {
        sum += i;
        i++;
    } while (i < n);
    return sum;
}

// ============================================================================
// SECTION 13: FUNCTION CALL TESTS
// ============================================================================

int helper_add(int a, int b) {
    return a + b;
}

int test_simple_call(void) {
    return helper_add(10, 20);
}

long long helper_ll_add(long long a, long long b, long long c) {
    return a + b + c;
}

long long test_ll_call(void) {
    return helper_ll_add(
        0x1111111111111111LL,
        0x2222222222222222LL,
        0x3333333333333333LL
    );
}

int helper_struct_extract(struct Small s) {
    return s.a * s.b;
}

int test_struct_call(void) {
    struct Small s;
    s.a = 5;
    s.b = 7;
    return helper_struct_extract(s);
}

// ============================================================================
// SECTION 14: EDGE CASES
// ============================================================================

long long test_overflow_add(void) {
    long long max = 0x7FFFFFFFFFFFFFFFLL;
    return max + 1;  // Should wrap
}

long long test_underflow_sub(void) {
    long long min = 0x8000000000000000LL;
    return min - 1;  // Should wrap
}

int test_divide_by_zero_safe(int x, int y) {
    if (y == 0) return 0;
    return x / y;
}

int test_modulo(int x) {
    return (x % 10) + (x % 7) + (x % 3);
}

// Test alternating bit patterns
long long test_bit_patterns(void) {
    long long all_ones = -1LL;
    long long alternating1 = 0xAAAAAAAAAAAAAAAALL;
    long long alternating2 = 0x5555555555555555LL;
    return all_ones + (alternating1 ^ alternating2);
}

// ============================================================================
// SECTION 15: MAIN TEST DRIVER
// ============================================================================

int main(void) {
    int result = 0;
    struct Point2D p = {10, 20};
    struct Small s = {5, 10};
    struct Small s2 = test_struct_small_return(15, 25);
    struct Medium m = {1, 2, 3, 4};
    struct Large l;
    struct With64 w = {0x123456789LL, 100};
    
    // 64-bit tests
    result += (int)test_ll_constants();
    result += (int)test_ll_arithmetic(1000LL, 500LL);
    result += (int)test_ll_bitwise(0xFF00FF00FF00FF00LL, 0x00FF00FF00FF00FFLL);
    result += test_ll_compare_signed(100LL, 50LL);
    result += test_ll_compare_unsigned(100ULL, 50ULL);
    result += (int)test_conversions_i32_to_i64(-1000);
    result += (int)test_conversions_u32_to_u64(2000);
    result += test_conversions_i64_to_i32(0x123456789ABCDEFLL);
    
    // 32-bit tests
    result += test_i32_arithmetic(100, 50);
    result += test_i32_bitwise(0xAAAA, 0x5555);
    result += test_i32_shifts(256);
    result += test_i32_compare(10, 20);
    
    // 16-bit and 8-bit tests
    result += test_i16_ops(100, 50);
    result += test_i8_ops(10, 5);
    result += test_u16_ops(200, 100);
    result += test_u8_ops(50, 10);
    
    // Array tests
    result += test_array_i32();
    result += (int)test_array_i64();
    
    // Struct by reference
    result += test_struct_ref(&p);
    test_struct_modify(&p);
    result += p.x + p.y;
    
    // Struct by value (CRITICAL TESTS)
    result += test_struct_small_byval(s);
    
    result += s2.a + s2.b;
    
    result += test_struct_medium_byval(m);
    
    l.x = 1000LL;
    l.y = 2000LL;
    l.z = 30;
    result += (int)test_struct_large_byval(l);
    
    // Mixed struct and 64-bit
    result += (int)test_struct_with_ll(w);
    
    // Float tests (may not compile if no float support)
    // result += (int)test_float_arithmetic(1.5f, 2.5f);
    
    // Global tests
    result += test_global_i32();
    result += (int)test_global_i64();
    result += test_global_struct();
    
    // Control flow
    result += test_if_else(50);
    result += test_switch(1);
    result += test_while_loop(10);
    result += test_for_loop(10);
    result += test_do_while(10);
    
    // Function calls
    result += test_simple_call();
    result += (int)test_ll_call();
    result += test_struct_call();
    
    // Edge cases
    result += (int)test_overflow_add();
    result += test_divide_by_zero_safe(100, 5);
    result += test_modulo(123);
    result += (int)test_bit_patterns();
    
    return result;
}

// ============================================================================
// SECTION 16: ENDIANNESS VERIFICATION TESTS
// ============================================================================

// These functions help verify correct endianness
void test_endianness_store(unsigned char *buf) {
    long long val = 0x0011223344556677LL;
    *((long long*)buf) = val;
    // Big endian should have:
    // buf[0] = 0x00, buf[1] = 0x11, buf[2] = 0x22, buf[3] = 0x33
    // buf[4] = 0x44, buf[5] = 0x55, buf[6] = 0x66, buf[7] = 0x77
}

long long test_endianness_load(unsigned char *buf) {
    // If big endian is correct, this should reconstruct 0x0011223344556677
    return *((long long*)buf);
}

int test_endianness_truncate(void) {
    long long val = 0x0011223344556677LL;
    int low = (int)val;
    // Big endian: low should be 0x44556677 (low 32 bits)
    return low;
}

int test_endianness_extend(void) {
    int val = 0x80000000;  // Negative number
    long long extended = val;
    // Big endian: high word should be 0xFFFFFFFF (sign extended)
    int high = (int)(extended >> 32);
    return high;  // Should be -1 (0xFFFFFFFF)
}
