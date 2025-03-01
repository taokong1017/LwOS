#include <stdarg.h>
#include <stdio.h>
#include <types.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <uart_pl011.h>
#include <menuconfig.h>
#include <spin_lock.h>
#include <ctype.h>

#define SIGN 1	   /* unsigned/signed, must be 1 */
#define LEFT 2	   /* left justified */
#define PLUS 4	   /* show plus */
#define SPACE 8	   /* space if plus */
#define ZEROPAD 16 /* pad with zero, must be 16 == '0' - ' ' */
#define SMALL 32   /* use lowercase in hex (must be 32 == 0x20) */
#define SPECIAL 64 /* prefix hex with "0x", octal with "0" */
#define BUF_SIZE 1024

enum format_type {
	FORMAT_TYPE_NONE, /* Just a string part */
	FORMAT_TYPE_WIDTH,
	FORMAT_TYPE_PRECISION,
	FORMAT_TYPE_CHAR,
	FORMAT_TYPE_STR,
	FORMAT_TYPE_PTR,
	FORMAT_TYPE_PERCENT_CHAR,
	FORMAT_TYPE_INVALID,
	FORMAT_TYPE_LONG_LONG,
	FORMAT_TYPE_ULONG,
	FORMAT_TYPE_LONG,
	FORMAT_TYPE_UBYTE,
	FORMAT_TYPE_BYTE,
	FORMAT_TYPE_USHORT,
	FORMAT_TYPE_SHORT,
	FORMAT_TYPE_UINT,
	FORMAT_TYPE_INT,
	FORMAT_TYPE_SIZE_T,
	FORMAT_TYPE_PTRDIFF
};
struct printf_spec {
	unsigned int type : 8;		 /* format_type enum */
	signed int field_width : 24; /* width of output field */
	unsigned int flags : 8;		 /* flags to number() */
	unsigned int base : 8;		 /* number base, 8, 10 or 16 only */
	signed int precision : 16;	 /* # of digits/chars */
};

static const char hex_asc_upper[] = "0123456789ABCDEF";
static char buf[BUF_SIZE] = {0};

static int skip_atoi(const char **s) {
	int i = 0;

	do {
		i = i * 10 + *((*s)++) - '0';
	} while (isdigit(**s));

	return i;
}

static void set_field_width(struct printf_spec *spec, int width) {
	spec->field_width = width;
}

static void set_precision(struct printf_spec *spec, int prec) {
	spec->precision = prec;
}

static void move_right(char *buf, char *end, unsigned len, unsigned spaces) {
	size_t size;
	if (buf >= end) /* nowhere to put anything */
		return;
	size = end - buf;
	if (size <= spaces) {
		memset(buf, ' ', size);
		return;
	}
	if (len) {
		if (len > size - spaces)
			len = size - spaces;
		memmove(buf + spaces, buf, len);
	}
	memset(buf, ' ', spaces);
}

static char *widen_string(char *buf, int n, char *end,
						  struct printf_spec *spec) {
	unsigned spaces;

	if (n >= spec->field_width)
		return buf;
	/* we want to pad the sucker */
	spaces = spec->field_width - n;
	if (!(spec->flags & LEFT)) {
		move_right(buf - n, end, n, spaces);
		return buf + spaces;
	}
	while (spaces--) {
		if (buf < end)
			*buf = ' ';
		++buf;
	}
	return buf;
}

static char *string(char *buf, char *end, const char *s,
					struct printf_spec *spec) {
	int len = 0;
	int lim = spec->precision;

	while (lim--) {
		char c = *s++;
		if (!c)
			break;
		if (buf < end)
			*buf = c;
		++buf;
		++len;
	}
	return widen_string(buf, len, end, spec);
}

static const uint16_t decpair[100] = {
#ifdef CONFIG_BIG_ENDIANE
#define bswap_16(x) (((x) >> 8) | ((x) << 8))
#define cpu_to_le16 bswap_16
#else
#define cpu_to_le16
#endif
#define _(x) (uint16_t) cpu_to_le16(((x % 10) | ((x / 10) << 8)) + 0x3030)
	_(0),  _(1),  _(2),	 _(3),	_(4),  _(5),  _(6),	 _(7),	_(8),  _(9),
	_(10), _(11), _(12), _(13), _(14), _(15), _(16), _(17), _(18), _(19),
	_(20), _(21), _(22), _(23), _(24), _(25), _(26), _(27), _(28), _(29),
	_(30), _(31), _(32), _(33), _(34), _(35), _(36), _(37), _(38), _(39),
	_(40), _(41), _(42), _(43), _(44), _(45), _(46), _(47), _(48), _(49),
	_(50), _(51), _(52), _(53), _(54), _(55), _(56), _(57), _(58), _(59),
	_(60), _(61), _(62), _(63), _(64), _(65), _(66), _(67), _(68), _(69),
	_(70), _(71), _(72), _(73), _(74), _(75), _(76), _(77), _(78), _(79),
	_(80), _(81), _(82), _(83), _(84), _(85), _(86), _(87), _(88), _(89),
	_(90), _(91), _(92), _(93), _(94), _(95), _(96), _(97), _(98), _(99),
#undef _
};

static char *put_dec_trunc8(char *buf, unsigned r) {
	unsigned q;

	/* 1 <= r < 10^8 */
	if (r < 100)
		goto out_r;

	/* 100 <= r < 10^8 */
	q = (r * (uint64_t)0x28f5c29) >> 32;
	*((uint16_t *)buf) = decpair[r - 100 * q];
	buf += 2;

	/* 1 <= q < 10^6 */
	if (q < 100)
		goto out_q;

	/*  100 <= q < 10^6 */
	r = (q * (uint64_t)0x28f5c29) >> 32;
	*((uint16_t *)buf) = decpair[q - 100 * r];
	buf += 2;

	/* 1 <= r < 10^4 */
	if (r < 100)
		goto out_r;

	/* 100 <= r < 10^4 */
	q = (r * 0x147b) >> 19;
	*((uint16_t *)buf) = decpair[r - 100 * q];
	buf += 2;
out_q:
	/* 1 <= q < 100 */
	r = q;
out_r:
	/* 1 <= r < 100 */
	*((uint16_t *)buf) = decpair[r];
	buf += r < 10 ? 1 : 2;
	return buf;
}

static void put_dec_full4(char *buf, unsigned r) {
	unsigned q;

	/* 0 <= r < 10^4 */
	q = (r * 0x147b) >> 19;
	*((uint16_t *)buf) = decpair[r - 100 * q];
	buf += 2;
	/* 0 <= q < 100 */
	*((uint16_t *)buf) = decpair[q];
}

static unsigned put_dec_helper4(char *buf, unsigned x) {
	uint32_t q = (x * (uint64_t)0x346DC5D7) >> 43;

	put_dec_full4(buf, x - q * 10000);
	return q;
}

static char *put_dec(char *buf, unsigned long long n) {
	uint32_t d3, d2, d1, q, h;

	if (n < 100 * 1000 * 1000)
		return put_dec_trunc8(buf, n);

	d1 = ((uint32_t)n >> 16); /* implicit "& 0xffff" */
	h = (n >> 32);
	d2 = (h)&0xffff;
	d3 = (h >> 16); /* implicit "& 0xffff" */

	/* n = 2^48 d3 + 2^32 d2 + 2^16 d1 + d0
		 = 281_4749_7671_0656 d3 + 42_9496_7296 d2 + 6_5536 d1 + d0 */
	q = 656 * d3 + 7296 * d2 + 5536 * d1 + ((uint32_t)n & 0xffff);
	q = put_dec_helper4(buf, q);

	q += 7671 * d3 + 9496 * d2 + 6 * d1;
	q = put_dec_helper4(buf + 4, q);

	q += 4749 * d3 + 42 * d2;
	q = put_dec_helper4(buf + 8, q);

	q += 281 * d3;
	buf += 12;
	if (q)
		buf = put_dec_trunc8(buf, q);
	else
		while (buf[-1] == '0')
			--buf;

	return buf;
}

static char *number(char *buf, char *end, unsigned long long num,
					struct printf_spec *spec) {
	/* put_dec requires 2-byte alignment of the buffer. */
	char tmp[3 * sizeof(num)];
	char sign;
	char locase;
	int need_pfx = ((spec->flags & SPECIAL) && spec->base != 10);
	int i;
	bool is_zero = num == 0LL;
	int field_width = spec->field_width;
	int precision = spec->precision;

	/* locase = 0 or 0x20. ORing digits or letters with 'locase'
	 * produces same digits or (maybe lowercased) letters */
	locase = (spec->flags & SMALL);
	if (spec->flags & LEFT)
		spec->flags &= ~ZEROPAD;
	sign = 0;
	if (spec->flags & SIGN) {
		if ((signed long long)num < 0) {
			sign = '-';
			num = -(signed long long)num;
			field_width--;
		} else if (spec->flags & PLUS) {
			sign = '+';
			field_width--;
		} else if (spec->flags & SPACE) {
			sign = ' ';
			field_width--;
		}
	}
	if (need_pfx) {
		if (spec->base == 16)
			field_width -= 2;
		else if (!is_zero)
			field_width--;
	}

	/* generate full string in tmp[], in reverse order */
	i = 0;
	if (num < spec->base)
		tmp[i++] = hex_asc_upper[num] | locase;
	else if (spec->base != 10) { /* 8 or 16 */
		int mask = spec->base - 1;
		int shift = 3;

		if (spec->base == 16)
			shift = 4;
		do {
			tmp[i++] = (hex_asc_upper[((unsigned char)num) & mask] | locase);
			num >>= shift;
		} while (num);
	} else { /* base 10 */
		i = put_dec(tmp, num) - tmp;
	}

	/* printing 100 using %2d gives "100", not "00" */
	if (i > precision)
		precision = i;
	/* leading space padding */
	field_width -= precision;
	if (!(spec->flags & (ZEROPAD | LEFT))) {
		while (--field_width >= 0) {
			if (buf < end)
				*buf = ' ';
			++buf;
		}
	}
	/* sign */
	if (sign) {
		if (buf < end)
			*buf = sign;
		++buf;
	}
	/* "0x" / "0" prefix */
	if (need_pfx) {
		if (spec->base == 16 || !is_zero) {
			if (buf < end)
				*buf = '0';
			++buf;
		}
		if (spec->base == 16) {
			if (buf < end)
				*buf = ('X' | locase);
			++buf;
		}
	}
	/* zero or space padding */
	if (!(spec->flags & LEFT)) {
		char c = ' ' + (spec->flags & ZEROPAD);

		while (--field_width >= 0) {
			if (buf < end)
				*buf = c;
			++buf;
		}
	}
	/* hmm even more zero padding? */
	while (i <= --precision) {
		if (buf < end)
			*buf = '0';
		++buf;
	}
	/* actual digits of result */
	while (--i >= 0) {
		if (buf < end)
			*buf = tmp[i];
		++buf;
	}
	/* trailing space padding */
	while (--field_width >= 0) {
		if (buf < end)
			*buf = ' ';
		++buf;
	}

	return buf;
}

static char *pointer_string(char *buf, char *end, const void *ptr,
							struct printf_spec *spec) {
	spec->base = 16;
	spec->flags |= SMALL;
	if (spec->field_width == -1) {
		spec->field_width = 2 * sizeof(ptr);
		spec->flags |= ZEROPAD;
	}

	return number(buf, end, (unsigned long int)ptr, spec);
}

static int format_decode(const char *fmt, struct printf_spec *spec) {
	const char *start = fmt;
	char qualifier;

	/* we finished early by reading the field width */
	if (spec->type == FORMAT_TYPE_WIDTH) {
		if (spec->field_width < 0) {
			spec->field_width = -spec->field_width;
			spec->flags |= LEFT;
		}
		spec->type = FORMAT_TYPE_NONE;
		goto precision;
	}

	/* we finished early by reading the precision */
	if (spec->type == FORMAT_TYPE_PRECISION) {
		if (spec->precision < 0)
			spec->precision = 0;

		spec->type = FORMAT_TYPE_NONE;
		goto qualifier;
	}

	/* By default */
	spec->type = FORMAT_TYPE_NONE;

	for (; *fmt; ++fmt) {
		if (*fmt == '%')
			break;
	}

	/* Return the current non-format string */
	if (fmt != start || !*fmt)
		return fmt - start;

	/* Process flags */
	spec->flags = 0;

	while (1) { /* this also skips first '%' */
		bool found = true;

		++fmt;

		switch (*fmt) {
		case '-':
			spec->flags |= LEFT;
			break;
		case '+':
			spec->flags |= PLUS;
			break;
		case ' ':
			spec->flags |= SPACE;
			break;
		case '#':
			spec->flags |= SPECIAL;
			break;
		case '0':
			spec->flags |= ZEROPAD;
			break;
		default:
			found = false;
		}

		if (!found)
			break;
	}

	/* get field width */
	spec->field_width = -1;

	if (isdigit(*fmt))
		spec->field_width = skip_atoi(&fmt);
	else if (*fmt == '*') {
		/* it's the next argument */
		spec->type = FORMAT_TYPE_WIDTH;
		return ++fmt - start;
	}

precision:
	/* get the precision */
	spec->precision = -1;
	if (*fmt == '.') {
		++fmt;
		if (isdigit(*fmt)) {
			spec->precision = skip_atoi(&fmt);
			if (spec->precision < 0)
				spec->precision = 0;
		} else if (*fmt == '*') {
			/* it's the next argument */
			spec->type = FORMAT_TYPE_PRECISION;
			return ++fmt - start;
		}
	}

qualifier:
	/* get the conversion qualifier */
	qualifier = 0;
	if (*fmt == 'h' || tolower(*fmt) == 'l' || *fmt == 'z' || *fmt == 't') {
		qualifier = *fmt++;
		if (qualifier == *fmt) {
			if (qualifier == 'l') {
				qualifier = 'L';
				++fmt;
			} else if (qualifier == 'h') {
				qualifier = 'H';
				++fmt;
			}
		}
	}

	/* default base */
	spec->base = 10;
	switch (*fmt) {
	case 'c':
		spec->type = FORMAT_TYPE_CHAR;
		return ++fmt - start;

	case 's':
		spec->type = FORMAT_TYPE_STR;
		return ++fmt - start;

	case 'p':
		spec->type = FORMAT_TYPE_PTR;
		return ++fmt - start;

	case '%':
		spec->type = FORMAT_TYPE_PERCENT_CHAR;
		return ++fmt - start;

	/* integer number formats - set up the flags and "break" */
	case 'o':
		spec->base = 8;
		break;

	case 'x':
		spec->flags |= SMALL;

	case 'X':
		spec->base = 16;
		break;

	case 'd':
	case 'i':
		spec->flags |= SIGN;
		break;
	case 'u':
		break;

	case 'n':
		/*
		 * Since %n poses a greater security risk than
		 * utility, treat it as any other invalid or
		 * unsupported format specifier.
		 */

	default:
		spec->type = FORMAT_TYPE_INVALID;
		return fmt - start;
	}

	if (qualifier == 'L')
		spec->type = FORMAT_TYPE_LONG_LONG;
	else if (qualifier == 'l') {
		spec->type = FORMAT_TYPE_ULONG + (spec->flags & SIGN);
	} else if (qualifier == 'z') {
		spec->type = FORMAT_TYPE_SIZE_T;
	} else if (qualifier == 't') {
		spec->type = FORMAT_TYPE_PTRDIFF;
	} else if (qualifier == 'H') {
		spec->type = FORMAT_TYPE_UBYTE + (spec->flags & SIGN);
	} else if (qualifier == 'h') {
		spec->type = FORMAT_TYPE_USHORT + (spec->flags & SIGN);
	} else {
		spec->type = FORMAT_TYPE_UINT + (spec->flags & SIGN);
	}

	return ++fmt - start;
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
	unsigned long long num;
	char *str, *end;
	struct printf_spec spec = {0};

	/* Reject out-of-range values early.  Large positive sizes are
	   used for unknown buffer sizes. */
	if (size > U32_MAX) {
		return 0;
	}

	str = buf;
	end = buf + size;

	/* Make sure end is always > buf */
	if (end <= buf) {
		return 0;
	}

	while (*fmt) {
		const char *old_fmt = fmt;
		int read = format_decode(fmt, &spec);

		fmt += read;

		switch (spec.type) {
		case FORMAT_TYPE_NONE: {
			int copy = read;
			if (str < end) {
				if (copy > end - str)
					copy = end - str;
				memcpy(str, old_fmt, copy);
			}
			str += read;
			break;
		}

		case FORMAT_TYPE_WIDTH:
			set_field_width(&spec, va_arg(args, int));
			break;

		case FORMAT_TYPE_PRECISION:
			set_precision(&spec, va_arg(args, int));
			break;

		case FORMAT_TYPE_CHAR: {
			char c;

			if (!(spec.flags & LEFT)) {
				while (--spec.field_width > 0) {
					if (str < end)
						*str = ' ';
					++str;
				}
			}
			c = (unsigned char)va_arg(args, int);
			if (str < end)
				*str = c;
			++str;
			while (--spec.field_width > 0) {
				if (str < end)
					*str = ' ';
				++str;
			}
			break;
		}

		case FORMAT_TYPE_STR:
			str = string(str, end, va_arg(args, char *), &spec);
			break;

		case FORMAT_TYPE_PTR:
			str = pointer_string(str, end, (const void *)va_arg(args, void *),
								 &spec);
			while (isalnum(*fmt))
				fmt++;
			break;

		case FORMAT_TYPE_PERCENT_CHAR:
			if (str < end)
				*str = '%';
			++str;
			break;

		case FORMAT_TYPE_INVALID:
			/*
			 * Presumably the arguments passed gcc's type
			 * checking, but there is no safe or sane way
			 * for us to continue parsing the format and
			 * fetching from the va_list; the remaining
			 * specifiers and arguments would be out of
			 * sync.
			 */
			goto out;

		default:
			switch (spec.type) {
			case FORMAT_TYPE_LONG_LONG:
				num = va_arg(args, long long);
				break;
			case FORMAT_TYPE_ULONG:
				num = va_arg(args, unsigned long);
				break;
			case FORMAT_TYPE_LONG:
				num = va_arg(args, long);
				break;
			case FORMAT_TYPE_SIZE_T:
				if (spec.flags & SIGN)
					num = va_arg(args, ssize_t);
				else
					num = va_arg(args, size_t);
				break;
			case FORMAT_TYPE_PTRDIFF:
				num = va_arg(args, ptrdiff_t);
				break;
			case FORMAT_TYPE_UBYTE:
				num = (unsigned char)va_arg(args, int);
				break;
			case FORMAT_TYPE_BYTE:
				num = (signed char)va_arg(args, int);
				break;
			case FORMAT_TYPE_USHORT:
				num = (unsigned short)va_arg(args, int);
				break;
			case FORMAT_TYPE_SHORT:
				num = (short)va_arg(args, int);
				break;
			case FORMAT_TYPE_INT:
				num = (int)va_arg(args, int);
				break;
			default:
				num = va_arg(args, unsigned int);
			}

			str = number(str, end, num, &spec);
		}
	}

out:
	if (size > 0) {
		if (str < end)
			*str = '\0';
		else
			end[-1] = '\0';
	}

	/* the trailing null byte doesn't count towards the total */
	return str - buf;
}

int printf(const char *fmt, ...) {
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = vsnprintf(buf, BUF_SIZE - 1, fmt, args);
	va_end(args);
	uart_puts(buf, ret);

	return ret;
}
