#include <stdio.h>
#include <stdint.h>

int main(void) {
	printf("Hello, exception %d", INT32_MIN / -1);
	return 0;
}
