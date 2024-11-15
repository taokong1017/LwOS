#ifndef __ITERABLE_SECTION_H__
#define __ITERABLE_SECTION_H__

#define concat(x, y) x##y
#define type_section_start(section_name) concat(_##section_name, _start)
#define type_section_end(section_name) concat(_##section_name, _end)

#define type_section_start_extern(type, section_name)                          \
	extern type type_section_start(section_name)[]
#define type_section_end_extern(type, section_name)                            \
	extern type type_section_end(section_name)[]

/**
 * @brief Iterate over a specified iterable section for a generic type
 *
 * @details
 * Iterator for structure instances gathered by type_section_foreach().
 * The linker must provide a _<section_name>_start symbol and a
 * _<section_name>_end symbol to mark the start and the end of the
 * list of struct objects to iterate over
 */
#define type_section_foreach(type, section_name, iterator)                     \
	type_section_start_extern(type, section_name);                             \
	type_section_end_extern(type, section_name);                               \
	for (type *iterator = type_section_start(section_name);                    \
		 iterator < type_section_end(section_name); iterator++)

/**
 * @brief Get element from section for a generic type
 *
 * @note There is no protection against reading beyond the section
 *
 * @param[in]  section_name type of element
 * @param[in]  secname name of output section
 * @param[in]  index Index.
 * @param[out] dest Pointer to location where pointer to element is written
 */
#define type_section_get(type, section_name, index, dest)                      \
	do {                                                                       \
		type_section_start_extern(type, section_name);                         \
		*(dest) = &type_section_start(section_name)[i];                        \
	} while (0)

/**
 * @brief Count elements in a section for a generic type
 *
 * @param[in]  type type of element
 * @param[in]  section_name name of output section
 * @param[out] dest Pointer to location where result is written
 */
#define type_section_count(type, section_name, dest)                           \
	do {                                                                       \
		type_section_start_extern(type, section_name);                         \
		type_section_end_extern(type, section_name);                           \
		*(dest) = ((virt_addr_t)type_section_end(section_name) -               \
				   (virt_addr_t)type_section_start(section_name)) /            \
				  sizeof(type);                                                \
	} while (0)

#endif
