#ifndef _CONVERSION_H_
#define _CONVERSION_H_

#include <stm32f4xx.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <arm_math.h>

//Peripherals used//
//Negative:	PC5 => ADC2_CH15 triggered by TIM3
//Positive:	PC1 => ADC3_CH11 triggered by TIM3

#define PRESCALER 1
//Period of USR Wave Readings (219 895Hz)//
#define SAMPLERATE_PERIOD				1905
//USR Received Wave Buffer Size//
#define BUFFERSIZE							1

//Left USR ADC & DMA defines//
#define NegADC							ADC2
#define NegADC_CHANNEL			ADC_Channel_15
#define NegADC_CLK					RCC_APB2Periph_ADC2
#define NegGPIO_PIN					GPIO_Pin_5
#define NegDMA_CHANNEL			DMA_Channel_1
#define NegDMA_STREAM				DMA2_Stream2
#define NegDMA_STREAM_IRQ		DMA2_Stream2_IRQn
#define NegADC_DR_ADDRESS		&ADC2->DR

//Right USR ADC & DMA defines//
#define PosADC							ADC3
#define PosADC_CHANNEL			ADC_Channel_11
#define PosADC_CLK					RCC_APB2Periph_ADC3
#define PosGPIO_PIN					GPIO_Pin_1
#define PosDMA_CHANNEL			DMA_Channel_2
#define PosDMA_STREAM				DMA2_Stream1
#define PosDMA_STREAM_IRQ		DMA2_Stream1_IRQn
#define PosADC_DR_ADDRESS		&ADC3->DR

#define NegInDMA_STORE_VAL		&NegInBuf
#define PosInDMA_STORE_VAL		&PosInBuf

#define NegOutDMA_STORE_VAL		&NegOutBuf
#define PosOutDMA_STORE_VAL		&PosOutBuf

//Common GPIO defines for both USR Pins//
#define CONV_ADCx_CHANNEL_GPIO_CLK		RCC_AHB1Periph_GPIOC
#define CONV_GPIO_PORT								GPIOC

#define DAC_CH1_R12ADDRESS    0x40007408
#define DAC_CH2_R12ADDRESS    0x40007414

extern SemaphoreHandle_t 	xSemaphore_SignalAcquisition;

//Input Buffers, destiny to the ADCs//
extern short int PosInBuf[BUFFERSIZE];
extern short int NegInBuf[BUFFERSIZE];

//Processing Buffers//
extern short int PosBuf[BUFFERSIZE];
extern short int NegBuf[BUFFERSIZE];


//Output Buffers, source from the DACs//
extern short int PosOutBuf[BUFFERSIZE];
extern short int NegOutBuf[BUFFERSIZE];


//Configuration Functions//
void Conv_Config(void);			//General Configuration//

void Input_Config(void);
void InRCC_Config(void);		//Config Peripheral Clocks used//
void InGPIO_Config(void);		//Config USR GPIO Pins as Analog Input (AN)//
void TIM3_Config(void);			//Config TIM3 with Frequency of 400kHz//
void InDMA_Config(void);		//Config DMA to transfer ADCs conversions values directly to Wave Buffers//
void ADC_Config(void);			//Config ADC2 & ADC3//

void Output_Config(void);
void OutRCC_Config(void);		//Config Peripheral Clocks used//
void OutGPIO_Config(void);	//Config USR GPIO Pins as Analog Input (AN)//
void TIM6_Config(void);			//Config TIM3 with Frequency of 400kHz//
void OutDMA_Config(void);		//Config DMA to transfer ADCs conversions values directly to Wave Buffers//
void DAC_Config(void);			//Config ADC2 & ADC3//

//Enable USR Readings//
void Input_Enable(void);
//Disable USR Readings//
void Input_Disable(void);

//Enable USR Readings//
void Output_Enable(void);
//Disable USR Readings//
void Output_Disable(void);

//Function to Clear passed Buffer//
void ClearBuffer(volatile short int[]);

//Thread Function from the tReadWave Task//
void ReadWave(void*);
//Thread Function from the tLocateBeacon Task//
void LocateBeacon(void*);
	
#endif	/*_CONVERSION_H_*/
