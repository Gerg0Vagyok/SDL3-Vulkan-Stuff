#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include <stdint.h>

typedef enum {
	// REACTION
	REACTION_IGNORE = 2, // just print error. - prints to console - can be turned off later maybe.
	REACTION_WARNING = 1, // shows popup as well. - prints to console and gives popup
	REACTION_FATAL = 0, // exits as well. - prints to console and gives popup and exits
	
	// SOME ADDITIONAL FLAGS 3-8
	AUTO_NEWLINE = 3,
} ErrorFlags;

uint32_t GetErrorFlags(int count, ...);
void error(uint32_t Flags, const char *fmt, ...);

#endif
