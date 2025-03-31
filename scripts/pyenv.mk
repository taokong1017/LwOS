# File: scripts/python.mk

# Find Python executable (prefer python3 over python)
PYTHON := $(shell which python3 || which python)
# Get Python major version
PYTHON_VERSION := $(shell $(PYTHON) -c "import sys; print(sys.version_info[0])")
# Get Python minor version
PYTHON_MINOR_VERSION := $(shell $(PYTHON) -c "import sys; print(sys.version_info[1])")

# Check Python version requirements
ifeq ($(PYTHON_VERSION),3)
	ifeq ($(shell test $(PYTHON_MINOR_VERSION) -ge 6; echo $$?),0)
		PYTHON_CMD := $(PYTHON)
	else
		$(error "Python 3.6 or higher is required, but found Python 3.$(PYTHON_MINOR_VERSION)")
	endif
else
	$(error "Python 3 is required, but found Python $(PYTHON_VERSION)")
endif
