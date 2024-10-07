#include <stdio.h>
#include <task.h>
#include <string.h>
#include <mem_domain.h>
#include <app_mem_domain.h>
#include <menuconfig.h>

static struct mem_domain app_mem_domain;
APP_PARTITION_DEFINE(app_1);
APP_DATA(app_1) int data = 1;
APP_BSS(app_1) char array[4000] = {1};

APP_PARTITION_DEFINE(app_2);
APP_DATA(app_2) int data2 = 1;
APP_BSS(app_2) char array2[8000] = {1};

int main() {
	printf("enter root task\n");
	mem_domain_init(&app_mem_domain, "Task_006_Domain");
	mem_domain_kernel_ranges_copy(&app_mem_domain);

	return 0;
}
