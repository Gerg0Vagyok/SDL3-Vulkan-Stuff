#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "error.h"

uint32_t GetErrorFlags(int count, ...) {
	uint32_t RFlags = 0;
	va_list args;
	va_start(args, count);

	for (int i = 0; i < count; i++) {
		ErrorFlags flag = va_arg(args, int);
		if (flag == REACTION_FATAL) {
			RFlags |= 1 << REACTION_FATAL;
		} else if (flag == REACTION_WARNING) {
			RFlags |= 1 << REACTION_WARNING;
		} else if (flag == REACTION_IGNORE) {
			RFlags |= 1 << REACTION_IGNORE;
		} else if (flag == AUTO_NEWLINE) {
			RFlags |= 1 << 3;
		}
	}

	va_end(args);

	return RFlags;
}

void error(uint32_t Flags, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	fprintf(stderr, "ERROR: ");
	vfprintf(stderr, fmt, args);

	va_end(args);

	if (Flags & (1 << AUTO_NEWLINE)) { // INSERT NEWLINE
		fprintf(stderr, "\n");
	}

	if (Flags & (1 << 3)) { // FATAL
		exit(1);
	}
}
