# File: scripts/kbuild.mk
ENTRY_OBJ := head.o
LINKER    := $(BUILD_ROOT)/samples/linker/lwos.ld
APP_LD    := $(BUILD_ROOT)/samples/linker/app.ld

# Modified Kbuild recursive processing engine
define process_kbuild
$(if $(wildcard $(1)/Kbuild),\
	$(eval -include $(1)/.config.mk) \
	$(eval -include $(1)/Kbuild) \
	$(eval subdir-y = $(addprefix $(1)/,$(subdir-y))) \
	$(foreach dir,$(subdir-y),\
		$(call process_kbuild,$(dir))\
	) \
	$(eval cur-c-obj := $(filter %.o,$(obj-y))) \
	$(eval cur-s-obj := $(filter %.o,$(obj-y))) \
	$(if $(strip $(cur-c-obj)), \
		$(foreach obj,$(cur-c-obj),\
			$(eval src_file = $(patsubst %.o,%.c,$(obj))) \
			$(if $(wildcard $(1)/$(src_file)),\
				$(eval all-c-objs := $(if $(strip $(all-c-objs)),$(all-c-objs) $(addprefix $(1)/,$(src_file)),$(addprefix $(1)/,$(src_file)))) \
				$(eval local_flag_$(addprefix $(SRC_ROOT)/,$(1)/$(src_file)) := $(local_flag)) \
			,)\
		)\
	) \
	$(if $(strip $(cur-s-obj)), \
		$(foreach obj,$(cur-s-obj),\
			$(eval src_file = $(patsubst %.o,%.S,$(obj))) \
			$(if $(wildcard $(1)/$(src_file)),\
				$(eval all-s-objs := $(if $(strip $(all-s-objs)),$(all-s-objs) $(addprefix $(1)/,$(src_file)),$(addprefix $(1)/,$(src_file)))) \
				$(eval local_flag_$(addprefix $(SRC_ROOT)/,$(1)/$(src_file)) := $(local_flag)) \
			,)\
		)\
	) \
	$(eval obj-y := ) \
	$(eval local_flag := ) \
)
endef

# Auto-discovery mechanism
KBUILD_FILES := $(foreach d,$(SRC_DIRS), $(shell find $(d) -maxdepth 1 -name Kbuild))
$(foreach kbuild,$(KBUILD_FILES),\
	$(eval dir = $(patsubst %/,%,$(dir $(kbuild)))) \
	$(call process_kbuild,$(dir))\
)

# Modified object file list definition
C_OBJS := $(addprefix $(BUILD_ROOT)/,$(all-c-objs:.c=.o))
S_OBJS := $(addprefix $(BUILD_ROOT)/,$(all-s-objs:.S=.o))

# Add command formatting
CMD_CC = $(strip $(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(local_flag_$<) \
		-MT $@ -MMD -MP -MF $(@D)/.$(basename $(@F)).d \
		-c $< -o $@)
CMD_AS = $(strip $(AS) $(ASFLAGS) $(EXTRA_AFLAGS) $(local_flag_$<) \
		-MT $@ -MMD -MP -MF $(@D)/.$(basename $(@F)).d \
		-c $< -o $@)

# Define entry object and other objects for linking
ENTRY_OBJS = $(filter %/$(ENTRY_OBJ),$(C_OBJS) $(S_OBJS))
OTHER_OBJS = $(filter-out %/$(ENTRY_OBJ),$(C_OBJS) $(S_OBJS))

# Define linking command
CMD_LD = $(strip $(LD) $(LDFLAGS) -T $(LINKER) -e __start -Map=$(BUILD_ROOT)/$(PROJECT).map $(ENTRY_OBJS) $(OTHER_OBJS) -o $@)

# Unified processing for C and assembly files
$(C_OBJS): $(BUILD_ROOT)/%.o: $(SRC_ROOT)/%.c $(LOGO_HEADER) $(OFFSET_HEADER) 
	@mkdir -p $(@D)
	$(if $(QUIET), \
		@echo "  CC      $(patsubst $(SRC_ROOT)/%,%,$<)", \
		@echo "  CC      $(patsubst $(SRC_ROOT)/%,%,$<)"; \
	)
	$(Q)$(CMD_CC)

$(S_OBJS): $(BUILD_ROOT)/%.o: $(SRC_ROOT)/%.S $(OFFSET_HEADER) 
	@mkdir -p $(@D)
	$(if $(QUIET), \
		@echo "  AS      $(patsubst $(SRC_ROOT)/%,%,$<)", \
		@echo "  AS      $(patsubst $(SRC_ROOT)/%,%,$<)"; \
	)
	$(Q)$(CMD_AS)

# Include dependency files
-include $(C_OBJS:.o=.d)
-include $(S_OBJS:.o=.d)

# Final target generation rule
$(TARGET): $(C_OBJS) $(S_OBJS)
	$(Q)mkdir -p $(dir $(APP_LD))
	$(Q)$(PYTHON_CMD) $(SRC_ROOT)/scripts/gen_app_ld.py -s 0x1000 -d $(BUILD_ROOT)/samples/user -o $(APP_LD)
	$(Q)$(CPP) $(CFLAGS) -D__ASSEMBLY__ $(SRC_ROOT)/samples/linker/lwos.ld.S |grep -v "^#" > $(LINKER)
	@echo "  LD      $(patsubst $(SRC_ROOT)/%,%,$@)"
	$(Q)$(CMD_LD)
	$(Q)$(OBJDUMP) -d $(TARGET) > $(BUILD_ROOT)/$(PROJECT).sym
	$(Q)$(READELF) -e $(TARGET) > $(BUILD_ROOT)/$(PROJECT).stat
	@echo "Build completed: $(patsubst $(SRC_ROOT)/%,%,$@)"
	$(Q)$(SIZE) $(TARGET)
