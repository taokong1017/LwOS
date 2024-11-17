#include <ctype.h>
#include <types.h>
#include <string.h>
#include <fnmatch.h>

#define EOS '\0'

static inline int foldcase(int ch, int flags) {
	if ((flags & FNM_CASEFOLD) != 0 && isupper(ch)) {
		return tolower(ch);
	}

	return ch;
}

#define FOLDCASE(ch, flags) foldcase((unsigned char)(ch), (flags))

static const char *rangematch(const char *pattern, int test, int flags) {
	bool negate, ok, need;
	char c, c2;

	if (pattern == NULL) {
		return NULL;
	}

	/*
	 * A bracket expression starting with an unquoted circumflex
	 * character produces unspecified results (IEEE 1003.2-1992,
	 * 3.13.2).  This implementation treats it like '!', for
	 * consistency with the regular expression syntax.
	 * J.T. Conklin (conklin@ngai.kaleida.com)
	 */
	negate = *pattern == '!' || *pattern == '^';
	if (negate) {
		++pattern;
	}

	for (need = true, ok = false, c = FOLDCASE(*pattern++, flags);
		 c != ']' || need; c = FOLDCASE(*pattern++, flags)) {
		need = false;
		if (c == '/') {
			return (void *)-1;
		}

		if (c == '\\' && !(flags & FNM_NOESCAPE)) {
			c = FOLDCASE(*pattern++, flags);
		}

		if (c == EOS) {
			return NULL;
		}

		if (*pattern == '-') {
			c2 = FOLDCASE(*(pattern + 1), flags);
			if (c2 != EOS && c2 != ']') {
				pattern += 2;
				if (c2 == '\\' && !(flags & FNM_NOESCAPE)) {
					c2 = FOLDCASE(*pattern++, flags);
				}

				if (c2 == EOS) {
					return NULL;
				}

				if (c <= test && test <= c2) {
					ok = true;
				}
			}
		} else if (c == test) {
			ok = true;
		}
	}

	return ok == negate ? NULL : pattern;
}

static int fnmatchx(const char *pattern, const char *string, int flags,
					size_t recursion) {
	const char *stringstart, *r;
	char c, test;

	if (pattern == NULL || string == NULL) {
		return FNM_NOMATCH;
	}

	if (recursion-- == 0) {
		return FNM_NORES;
	}

	for (stringstart = string;;) {
		c = FOLDCASE(*pattern++, flags);
		switch (c) {
		case EOS:
			if ((flags & FNM_LEADING_DIR) && *string == '/') {
				return 0;
			}

			return *string == EOS ? 0 : FNM_NOMATCH;
		case '?':
			if (*string == EOS) {
				return FNM_NOMATCH;
			}

			if (*string == '/' && (flags & FNM_PATHNAME)) {
				return FNM_NOMATCH;
			}

			if (*string == '.' && (flags & FNM_PERIOD) &&
				(string == stringstart ||
				 ((flags & FNM_PATHNAME) && *(string - 1) == '/'))) {
				return FNM_NOMATCH;
			}

			++string;
			break;
		case '*':
			c = FOLDCASE(*pattern, flags);
			/* Collapse multiple stars. */
			while (c == '*') {
				c = FOLDCASE(*++pattern, flags);
			}

			if (*string == '.' && (flags & FNM_PERIOD) &&
				(string == stringstart ||
				 ((flags & FNM_PATHNAME) && *(string - 1) == '/'))) {
				return FNM_NOMATCH;
			}

			/* Optimize for pattern with * at end or before /. */
			if (c == EOS) {
				if (flags & FNM_PATHNAME) {
					return (flags & FNM_LEADING_DIR) ||
								   strchr(string, '/') == NULL
							   ? 0
							   : FNM_NOMATCH;
				} else {
					return 0;
				}
			} else if (c == '/' && flags & FNM_PATHNAME) {
				string = strchr(string, '/');
				if (string == NULL) {
					return FNM_NOMATCH;
				}

				break;
			}

			/* General case, use recursion. */
			do {
				test = FOLDCASE(*string, flags);
				if (test == EOS) {
					break;
				}

				int e =
					fnmatchx(pattern, string, flags & ~FNM_PERIOD, recursion);

				if (e != FNM_NOMATCH) {
					return e;
				}

				if (test == '/' && flags & FNM_PATHNAME) {
					break;
				}

				++string;
			} while (true);

			return FNM_NOMATCH;
		case '[':
			if (*string == EOS) {
				return FNM_NOMATCH;
			}

			if (*string == '/' && flags & FNM_PATHNAME) {
				return FNM_NOMATCH;
			}

			r = rangematch(pattern, FOLDCASE(*string, flags), flags);
			if (r == NULL) {
				return FNM_NOMATCH;
			}

			if (r == (void *)-1) {
				if (*string != '[') {
					return FNM_NOMATCH;
				}
			} else {
				pattern = r;
			}

			++string;
			break;
		case '\\':
			if (!(flags & FNM_NOESCAPE)) {
				c = FOLDCASE(*pattern++, flags);
				if (c == EOS) {
					c = '\0';
					--pattern;
				}
			}
		default:
			if (c != FOLDCASE(*string++, flags)) {
				return FNM_NOMATCH;
			}

			break;
		}
	}
}

int fnmatch(const char *pattern, const char *string, int flags) {
	return fnmatchx(pattern, string, flags, 64);
}