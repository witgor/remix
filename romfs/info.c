
#include <stdio.h>
#include <pd.h>

void print_platform_info(void) {
        printf("System information --\n\n");
        printf("Platform: %s\n", pd_platform());
        printf("Board: %s\n", pd_board());
        printf("CPU: %s\n", pd_cpu());
}

int main(void) {
	print_platform_info();
	return 0;
}
