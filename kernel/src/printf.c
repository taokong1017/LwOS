#include <stdarg.h>
#include <stdio.h>
#include <types.h>
#include <errno.h>
#include <string.h>

static int write(int fd, const void *buf, size_t size) { return size; }

static int fileno(FILE *stream) {
	intptr_t i = (intptr_t)stream;

	if (i >= 0) {
		set_errno(EBADF);
		return -1;
	}
	return ~i;
}

static int fwrite(const void *buf, size_t size, FILE *stream) {
	size_t ret;
	int fd = fileno(stream);

	while (size) {
		ret = write(fd, buf, size);
		if (ret <= 0)
			return EOF;
		size -= ret;
		buf += ret;
	}
	return 0;
}

int u64toa_r(uint64_t in, char *buffer) {
	unsigned long long lim;
	int digits = 0;
	int pos = 19; /* start with the highest possible digit */
	int dig;

	do {
		for (dig = 0, lim = 1; dig < pos; dig++)
			lim *= 10;

		if (digits || in >= lim || !pos) {
			for (dig = 0; in >= lim; dig++)
				in -= lim;
			buffer[digits++] = '0' + dig;
		}
	} while (pos--);

	buffer[digits] = 0;
	return digits;
}

static int i64toa_r(int64_t in, char *buffer) {
	char *ptr = buffer;
	int len = 0;

	if (in < 0) {
		in = -in;
		*(ptr++) = '-';
		len++;
	}
	len += u64toa_r(in, ptr);
	return len;
}

static int u64toh_r(uint64_t in, char *buffer) {
	signed char pos = 60;
	int digits = 0;
	int dig;

	do {
		if (sizeof(long) >= 8) {
			dig = (in >> pos) & 0xF;
		} else {
			/* 32-bit platforms: avoid a 64-bit shift */
			uint32_t d = (pos >= 32) ? (in >> 32) : in;
			dig = (d >> (pos & 31)) & 0xF;
		}
		if (dig > 9)
			dig += 'a' - '0' - 10;
		pos -= 4;
		if (dig || digits || pos < 0)
			buffer[digits++] = '0' + dig;
	} while (pos >= 0);

	buffer[digits] = 0;
	return digits;
}

int vfprintf(FILE *stream, const char *fmt, va_list args) {
	char escape, lpref, c;
	unsigned long long v;
	unsigned int written;
	uint32_t len, ofs;
	char tmpbuf[21];
	const char *outstr;

	written = ofs = escape = lpref = 0;
	while (1) {
		c = fmt[ofs++];

		if (escape) {
			/* we're in an escape sequence, ofs == 1 */
			escape = 0;
			if (c == 'c' || c == 'd' || c == 'u' || c == 'x' || c == 'p') {
				char *out = tmpbuf;

				if (c == 'p')
					v = va_arg(args, unsigned long);
				else if (lpref) {
					if (lpref > 1)
						v = va_arg(args, unsigned long long);
					else
						v = va_arg(args, unsigned long);
				} else
					v = va_arg(args, unsigned int);

				if (c == 'd') {
					/* sign-extend the value */
					if (lpref == 0)
						v = (long long)(int)v;
					else if (lpref == 1)
						v = (long long)(long)v;
				}

				switch (c) {
				case 'c':
					out[0] = v;
					out[1] = 0;
					break;
				case 'd':
					i64toa_r(v, out);
					break;
				case 'u':
					u64toa_r(v, out);
					break;
				case 'p':
					*(out++) = '0';
					*(out++) = 'x';
					/* fall through */
				default: /* 'x' and 'p' above */
					u64toh_r(v, out);
					break;
				}
				outstr = tmpbuf;
			} else if (c == 's') {
				outstr = va_arg(args, char *);
				if (!outstr)
					outstr = "(null)";
			} else if (c == '%') {
				/* queue it verbatim */
				continue;
			} else {
				/* modifiers or final 0 */
				if (c == 'l') {
					/* long format prefix, maintain the escape */
					lpref++;
				}
				escape = 1;
				goto do_escape;
			}
			len = strlen(outstr);
			goto flush_str;
		}

		/* not an escape sequence */
		if (c == 0 || c == '%') {
			/* flush pending data on escape or end */
			escape = 1;
			lpref = 0;
			outstr = fmt;
			len = ofs - 1;
		flush_str:
			if (fwrite(outstr, len, stream) != 0)
				break;

			written += len;
		do_escape:
			if (c == 0)
				break;
			fmt += ofs;

			ofs = 0;
			continue;
		}

		/* literal char, just queue it */
	}
	return written;
}

int printf(const char *fmt, ...) {
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = vfprintf(stdout, fmt, args);
	va_end(args);
	return ret;
}
