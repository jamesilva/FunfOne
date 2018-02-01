#ifdef _FX_H_
#define _FX_H_

#include "conversion.h"
#include "rf.h"
#include "bluetooth.h"
#include <stm32f4xx.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

long map(long,long,long,long,long);
void createSineTable(void);

void FX(void*);

#endif /*_EFFECTS_H_*/
