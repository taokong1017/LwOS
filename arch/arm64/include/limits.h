#ifndef __LIMITS_H__
#define __LIMITS_H__

#define U8_MAX ((uint8_t)~0U)
#define S8_MAX ((int8_t)(U8_MAX >> 1))
#define S8_MIN ((int8_t)(-S8_MAX - 1))
#define U16_MAX ((uint16_t)~0U)
#define S16_MAX ((int16_t)(U16_MAX >> 1))
#define S16_MIN ((int16_t)(-S16_MAX - 1))
#define U32_MAX ((uint32_t)~0U)
#define U32_MIN ((uint32_t)0)
#define S32_MAX ((int32_t)(U32_MAX >> 1))
#define S32_MIN ((int32_t)(-S32_MAX - 1))
#define U64_MAX ((uint64_t)~0ULL)
#define S64_MAX ((int64_t)(U64_MAX >> 1))
#define S64_MIN ((int64_t)(-S64_MAX - 1))

#endif
