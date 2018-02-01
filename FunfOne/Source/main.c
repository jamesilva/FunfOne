#include "pedal.h"
#include <effects.h>
#include <conversion.h>

short int PosInBuf[BUFFERSIZE];
short int NegInBuf[BUFFERSIZE];

//Processing Buffers//
short int PosBuf[BUFFERSIZE]={0};
short int NegBuf[BUFFERSIZE]={0};


//Output Buffers, source from the DACs//
short int PosOutBuf[BUFFERSIZE];
short int NegOutBuf[BUFFERSIZE];

int main(void){
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	//Run Pedal
	Run();

//	Conv_Config();
	//Input_Enable();
//	Output_Enable();
	
	//int i=0, j=0;
	//Never end execution
	
  while(1){
		//PosBuf[0]=PosInBuf[0];
		//NegBuf[0]=NegInBuf[0];

		//PosOutBuf[0] = PosBuf[0];
		//NegOutBuf[0] = NegBuf[0];
	};
}
