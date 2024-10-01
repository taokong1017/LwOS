#include <stdio.h>
#include <task.h>
#include <string.h>
#include <mem_domain.h>

static struct mem_domain app_mem_domain;

int main() {
	printf("enter root task\n");
	mem_domain_init(&app_mem_domain, "Task_006_Domain");
	mem_domain_kernel_ranges_copy(&app_mem_domain);

	return 0;
}
