/* Ships all the way around */

#include <stdio.h>

void go_s_e(int * lat, int * lon) {
	*lat = *lat - 1;
	*lon = *lon + 1;
}

int main() {
	int latitude = 32;
	int longitude = -64;

	go_s_e(&latitude, &longitude);
		printf("Avast! Now at: %i, %i \n", latitude, longitude);
		return 0;
}
