#include <stdio.h>
#include <stdlib.h>

int main(void) {
	printf ("IN\n");
	sleep(1);
	system("true");
	system("false");
	sleep(1);
	printf ("OUT\n");
	return 0;
}
