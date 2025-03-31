# File: scripts/analysis.mk

# Define cppcheck executable path
CPPCHECK  ?= cppcheck
CHECK_DIR ?= $(SRC_DIRS)

# Configure cppcheck flags
CPPCHECK_FLAGS += -v --suppress=unusedFunction --suppress=variableScope \
	--suppress=unreadVariable --suppress=oppositeExpression \
	--suppress=missingIncludeSystem --enable=all

# Target for running static code analysis
analyze:
	@echo "Checking cppcheck installation..."
	@which $(CPPCHECK) > /dev/null || \
		(echo "Error: cppcheck not found. Please install cppcheck."; exit 1)
	@echo "Running static analysis..."
	@$(CPPCHECK) $(CPPCHECK_FLAGS) $(CHECK_DIR) 2> $(BUILD_ROOT)/analysis.log
	@$(shell lizard $(CHECK_DIR) > $(BUILD_ROOT)/analysis.log)