#ifndef __METAL_MUTEX_H__
#define __METAL_MUTEX_H__

/* fixme: this is not a real mutex, just a placeholder */
typedef long metal_mutex_t;

/**
 * @brief	Initialize a libmetal mutex.
 * @param[in]	mutex	Mutex to initialize.
 */
void metal_mutex_init(metal_mutex_t *mutex);

/**
 * @brief	Deinitialize a libmetal mutex.
 * @param[in]	mutex	Mutex to deinitialize.
 */
void metal_mutex_deinit(metal_mutex_t *mutex);

/**
 * @brief	Acquire a mutex
 * @param[in]	mutex	Mutex to mutex.
 */
void metal_mutex_acquire(metal_mutex_t *mutex);

/**
 * @brief	Release a previously acquired mutex.
 * @param[in]	mutex	Mutex to mutex.
 * @see metal_mutex_try_acquire, metal_mutex_acquire
 */
void metal_mutex_release(metal_mutex_t *mutex);

#endif
