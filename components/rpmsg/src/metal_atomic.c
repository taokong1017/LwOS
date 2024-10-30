#include <types.h>
#include <metal_atomic.h>

static enum SHM_SYNC_TYPE sync_type = SHM_NO_SYNC;

void atomic_thread_fence() {
	if (sync_type == SHM_SYNC_INNER) {
		/**
		 * Here we use outer barrier instead, but should use
		 * inner barrier instruction.
		 */
		__sync_synchronize();
	}

	if (sync_type == SHM_SYNC_OUTER) {
		__sync_synchronize();
	}
}

void atomic_type_set(int32_t type) {
	if (type < SHM_NO_SYNC || type > SHM_SYNC_OUTER) {
		return;
	}

	sync_type = type;
}