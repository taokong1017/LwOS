#ifndef __TASK_CMD_H__
#define __TASK_CMD_H__

#include <task.h>

enum task_cmd_type {
	TASK_CMD_STOP = 0,
	TASK_CMD_NUM,
};

struct task_cmd {
	task_id_t id;
	enum task_cmd_type cmd;
	void *data;
};

#endif
