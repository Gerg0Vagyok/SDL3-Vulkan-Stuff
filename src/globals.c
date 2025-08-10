#include "globals.h"

uint32_t GlobalArgFlags = 0;

void SetArgFlag(ArgsFlags flag, int OnOff) {
	if (OnOff) {
		GlobalArgFlags |= (1 << flag);
	} else {
		GlobalArgFlags &= ~(1 << flag);
	}
}
