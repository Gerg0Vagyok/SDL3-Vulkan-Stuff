#ifndef GLOBALS_H
#define GLOBALS_H
#include <stdint.h>

#define CS_S_R(x) #x
#define S(x) CS_S_R(x)

typedef enum {
	VERBOSE = 0,
} ArgsFlags;

extern uint32_t GlobalArgFlags;

void SetArgFlag(ArgsFlags flag, int OnOff);

#endif // GLOBALS_H
