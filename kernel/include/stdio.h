#ifndef __STD_IO_H__
#define __STD_IO_H__

typedef struct FILE {
	char dummy[1];
} FILE;

#define EOF (-1)

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define stdin ((FILE *)~STDIN_FILENO)
#define stdout ((FILE *)~STDOUT_FILENO)
#define stderr ((FILE *)~STDERR_FILENO)

int printf(const char *fmt, ...);

#endif
