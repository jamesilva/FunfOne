#ifndef _RF_H_
#define _RF_H_

#include <stm32f4xx.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "tm_stm32f4_nrf24l01.h"
#include "misc.h"

#define RF_BUFFERSIZE 4

extern SemaphoreHandle_t xSemaphore_RF;
void RF_Config(void);
void TIM4_Config(void);
void RF(void*);

#endif /*_RF_H_*/
