#include <stdlib.h>
#include <user_mem_domain.h>

global_data_section uint64_t seed = 0;

void srand(unsigned s) { seed = s - 1; }

int rand(void) {
	seed = 6364136223846793005ULL * seed + 1;
	return seed >> 33;
}
