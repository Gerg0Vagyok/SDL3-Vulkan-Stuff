#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include "error.h"
#include "globals.h"

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
			RFlags |= 1 << AUTO_NEWLINE;
		}
	}

	va_end(args);

	return RFlags;
}

void error(uint32_t Flags, const char *fmt, ...) {
	if (Flags & (1 << REACTION_FATAL) || Flags & (1 << REACTION_WARNING) || (Flags & (1 << REACTION_IGNORE) && GlobalArgFlags & (1 << VERBOSE))) {
		// prefix
		if (Flags & (1 << REACTION_FATAL)) {
			fprintf(stderr, "FATAL ERROR: ");
		} else if (Flags & (1 << REACTION_WARNING) || Flags & (1 << REACTION_IGNORE)) {
			fprintf(stderr, "WARNING: ");
		}

		va_list args;
		va_start(args, fmt);
		va_list args_copy;
		va_copy(args_copy, args);

		int len = vsnprintf(NULL, 0, fmt, args); // get length
		char *msg = malloc(len + 1);
		if (!msg) {
			va_end(args_copy);
			va_end(args);
			return;
		}
		vsnprintf(msg, len + 1, fmt, args_copy);

		va_end(args_copy);
		va_end(args);

		// message
		fprintf(stderr, "%s", msg);

		//others
		if (Flags & (1 << AUTO_NEWLINE)) { // INSERT NEWLINE
			fprintf(stderr, "\n");
		}

		if (Flags & (1 << REACTION_FATAL) || Flags & (1 << REACTION_WARNING)) {
			SDL_ShowSimpleMessageBox(
				SDL_MESSAGEBOX_ERROR,
				"Error",
				msg,
				NULL
			);
		}

		free(msg);

		if (Flags & (1 << REACTION_FATAL)) { // FATAL
			exit(1);
		}
	}
}
