# File: scripts/config.mk

# If CONFIG_FILE does not exist, copy defconfig as default configuration
ifeq ($(wildcard $(CONFIG_FILE)),)
$(CONFIG_FILE): $(DEFAULT_CONFIG)
	@mkdir -p $(@D)
	@cp -f $< $@
endif
-include $(CONFIG_FILE)

# Add include path for generated headers
CFLAGS += -I$(BUILD_ROOT)/generated/include
ASFLAGS += -I$(BUILD_ROOT)/generated/include
# Define the generated header file path
CONFIG_HEADER = $(BUILD_ROOT)/generated/include/menuconfig.h

# Rule to generate config.h from config file
$(CONFIG_HEADER): $(CONFIG_FILE)
	@mkdir -p $(@D)
	@echo "Generating config header: $(patsubst $(SRC_ROOT)/%,%,$@)"
	@echo "/* Auto-generated config */" > $@
	@awk -F= '/^CONFIG_/ {print "#define", $$1, $$2}' $< >> $@ || \
		(echo "Error: Failed to process $<"; exit 1)
	@echo "Config header generation completed"
