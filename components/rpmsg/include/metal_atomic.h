#ifndef __METAL_ATOMIC_H__
#define __METAL_ATOMIC_H__

#include <types.h>

/**
 * @brief Enumeration of shared memory synchronization types.
 *
 * This enumeration defines the different types of shared memory synchronization
 * that can be used.
 *
 * - `SHM_NO_SYNC`: No synchronization is required.
 * - `SHM_SYNC_INNER`: Inner synchronization is required.
 * - `SHM_SYNC_OUTER`: Outer synchronization is required.
 */
enum SHM_SYNC_TYPE {
	SHM_NO_SYNC = 0,
	SHM_SYNC_INNER,
	SHM_SYNC_OUTER,
};

/**
 * @brief Ensures all memory operations before the fence are completed before
 * any memory operations after the fence.
 *
 * This function acts as a memory fence, guaranteeing that all memory operations
 * before the fence are completed before any memory operations after the fence
 * are started.
 */
void atomic_thread_fence();

/**
 * @brief Sets the atomic synchronization type.
 *
 * This function sets the type of shared memory synchronization to be used. The
 * synchronization type can be one of the following:
 *
 * - `SHM_NO_SYNC`: No synchronization is required.
 * - `SHM_SYNC_INNER`: Inner synchronization is required.
 * - `SHM_SYNC_OUTER`: Outer synchronization is required.
 *
 * @param type The synchronization type to set.
 */
void atomic_type_set(int32_t type);

#endif
