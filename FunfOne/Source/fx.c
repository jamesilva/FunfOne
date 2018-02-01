#include "fx.h"

uint16_t nSineTable[BUFFERSIZE];

short int efeito;

long map(long x, long in_min, long in_max, long out_min, long out_max){
	
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	
}
void createSineTable(){
	
  for(uint32_t nIndex=0; nIndex<BUFFERSIZE; nIndex++)
  {
    // normalised to 12 bit range 0-4095
    nSineTable[nIndex] = (uint16_t)(((1+sin(((2.0*PI)/BUFFERSIZE)*nIndex))*4095.0)/2);
  }
}
void FX(void *args){
	
	static BaseType_t xHigherPriorityTaskWoken=pdFALSE;
	for(;;) {
		xSemaphoreTakeFromISR(xSemaphore_SignalAcquisition,&xHigherPriorityTaskWoken);
		
	}
}
