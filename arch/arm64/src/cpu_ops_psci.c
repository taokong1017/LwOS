#include <cpu_ops_psci.h>
#include <cpu_ops.h>
#include <smccc.h>
#include <log.h>
#include <string.h>
#include <compiler.h>

#define PSCI_TAG "PSCI"

typedef uint64_t (*psci_fn)(uint64_t, uint64_t, uint64_t, uint64_t);
struct psci_data {
	enum arch_smccc_type smcc_type;
	psci_fn psci_fn;
	uint32_t version;
};

static struct psci_data psci_data = {
	.smcc_type = SMCCC_TYPE_NONE, .psci_fn = NULL, .version = 0};

static uint64_t psci_fn_hvc(uint64_t function_id, uint64_t arg0, uint64_t arg1,
							uint64_t arg2) {
	struct arch_smccc res;

	arch_smccc_hvc(function_id, arg0, arg1, arg2, 0, 0, 0, 0, &res);
	return res.a0;
}

static uint64_t psci_fn_smc(uint64_t function_id, uint64_t arg0, uint64_t arg1,
							uint64_t arg2) {
	struct arch_smccc res;

	arch_smccc_smc(function_id, arg0, arg1, arg2, 0, 0, 0, 0, &res);
	return res.a0;
}

errno_t psci_init(const char *smccc_name) {
	if (!smccc_name) {
		log_err(PSCI_TAG, "smccc_name is NULL\n");
		return ERRNO_PSCI_EMPTY_NAME;
	}

	if (strncmp(smccc_name, "smc", 3) == 0) {
		psci_data.smcc_type = SMCCC_TYPE_SMC;
		psci_data.psci_fn = psci_fn_smc;
	} else if (strncmp(smccc_name, "hvc", 3) == 0) {
		psci_data.smcc_type = SMCCC_TYPE_HVC;
		psci_data.psci_fn = psci_fn_hvc;
	} else {
		log_err(PSCI_TAG, "smccc_name is invalid\n");
		return ERRNO_PSCI_INVALID_NAME;
	}
	psci_data.version = psci_data.psci_fn(PSCI_0_2_FN_PSCI_VERSION, 0, 0, 0);

	log_debug(PSCI_TAG, "PSCI version: %d.%d\n",
			  PSCI_VERSION_MAJOR(psci_data.version),
			  PSCI_VERSION_MINOR(psci_data.version));

	return OK;
}

static errno_t psci_value_to_err(uint64_t code) {
	errno_t ret = OK;

	switch (code) {
	case PSCI_RET_SUCCESS:
		log_debug(PSCI_TAG, "psci return success\n");
		ret = ERRNO_PSCI_SUCCESS;
		break;
	case PSCI_RET_NOT_SUPPORTED:
		log_err(PSCI_TAG, "psci return not supported\n");
		ret = ERRNO_PSCI_NOT_SUPPORTED;
		break;
	case PSCI_RET_INVALID_PARAMS:
		log_err(PSCI_TAG, "psci return invalid params\n");
		ret = ERRNO_PSCI_INVALID_PARAMS;
		break;
	case PSCI_RET_DENIED:
		log_err(PSCI_TAG, "psci return denied\n");
		ret = ERRNO_PSCI_DENIED;
		break;
	case PSCI_RET_ALREADY_ON:
		log_err(PSCI_TAG, "psci return already on\n");
		ret = ERRNO_PSCI_ALREADY_ON;
		break;
	case PSCI_RET_ON_PENDING:
		log_err(PSCI_TAG, "psci return on pending\n");
		ret = ERRNO_PSCI_ON_PENDING;
		break;
	case PSCI_RET_INTERNAL_FAILURE:
		log_err(PSCI_TAG, "psci return internal failure\n");
		ret = ERRNO_PSCI_INTERNAL_FAILURE;
		break;
	case PSCI_RET_NOT_PRESENT:
		log_err(PSCI_TAG, "psci return not present\n");
		ret = ERRNO_PSCI_NOT_PRESENT;
		break;
	case PSCI_RET_DISABLED:
		log_err(PSCI_TAG, "psci return disabled\n");
		ret = ERRNO_PSCI_DISABLED;
		break;
	case PSCI_RET_INVALID_ADDRESS:
		log_err(PSCI_TAG, "psci return invalid address\n");
		ret = ERRNO_PSCI_INVALID_ADDRESS;
		break;
	default:
		log_fatal(PSCI_TAG, "PSCI returned unknown error code: %lu\n", code);
		break;
	}

	return ret;
}

static errno_t psci_features_check(uint64_t function_id) {
	if (!(PSCI_VERSION_MAJOR(psci_data.version) >= 1)) {
		return ERRNO_PSCI_NOT_SUPPORTED;
	}

	return psci_data.psci_fn(PSCI_FN_NATIVE(1_0, PSCI_FEATURES), function_id, 0,
							 0);
}

void sys_poweroff() {
	errno_t ret = OK;

	if (psci_data.smcc_type == SMCCC_TYPE_NONE) {
		log_err(PSCI_TAG,
				"system power off failed without psci initialization\n");
		return;
	}

	ret = psci_value_to_err(psci_data.psci_fn(PSCI_0_2_FN_SYSTEM_OFF, 0, 0, 0));
	if (ret) {
		log_err(PSCI_TAG, "system power off failed\n");
	}

	forever();
}

errno_t sys_reset(enum cpu_reset_type type) {
	errno_t ret = OK;

	if (psci_data.smcc_type == SMCCC_TYPE_NONE) {
		log_err(PSCI_TAG, "system reset failed without psci initialization\n");
		return ERRNO_PSCI_NO_INITIALIZATION;
	}

	if ((type == CPU_WARM_RESET) &&
		(!psci_features_check(PSCI_FN_NATIVE(1_1, SYSTEM_RESET2)))) {
		ret = psci_data.psci_fn(PSCI_FN_NATIVE(1_1, SYSTEM_RESET2), 0, 0, 0);
	} else if (type == CPU_COLD_RESET) {
		ret = psci_data.psci_fn(PSCI_FN_NATIVE(0_2, SYSTEM_RESET), 0, 0, 0);
	} else {
		log_err(PSCI_TAG, "Invalid system reset type issued\n");
		return ERRNO_PSCI_INVLAID_RESET_TYPE;
	}

	return psci_value_to_err(ret);
}

errno_t cpu_on(uint64_t cpuid, uintptr_t entry_point) {
	uint64_t ret = OK;

	if (psci_data.smcc_type == SMCCC_TYPE_NONE) {
		log_err(PSCI_TAG, "cpu on failed without psci initialization\n");
		return ERRNO_PSCI_NO_INITIALIZATION;
	}

	ret = psci_data.psci_fn(PSCI_FN_NATIVE(0_2, CPU_ON), cpuid,
							(unsigned long)entry_point, 0);

	return psci_value_to_err(ret);
}

errno_t cpu_off() {
	uint64_t ret = OK;

	if (psci_data.smcc_type == SMCCC_TYPE_NONE) {
		log_err(PSCI_TAG, "cpu off failed without psci initialization\n");
		return ERRNO_PSCI_NO_INITIALIZATION;
	}

	ret = psci_data.psci_fn(PSCI_0_2_FN_CPU_OFF, 0, 0, 0);

	return psci_value_to_err(ret);
}
