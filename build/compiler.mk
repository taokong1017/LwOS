CC       = $(CROSS_COMPILE)gcc
GPP      = $(CROSS_COMPILE)g++
AS       = $(CROSS_COMPILE)as
AR       = $(CROSS_COMPILE)ar
LD       = $(CROSS_COMPILE)ld
OBJCOPY  = $(CROSS_COMPILE)objcopy
OBJDUMP  = $(CROSS_COMPILE)objdump
SIZE     = $(CROSS_COMPILE)size
NM       = $(CROSS_COMPILE)nm

$(if $(CROSS_COMPILE),,$(error CROSS_COMPILE is not set))


