#ifndef __SHELL_HISTORY_H__
#define __SHELL_HISTORY_H__

#include <ring_buffer.h>

struct shell_history {
	struct ring_buffer ring_buf;
};

#endif
