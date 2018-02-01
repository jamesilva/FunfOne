#include "rf.h"

SemaphoreHandle_t xSemaphore_RF;

extern QueueHandle_t xQueue_RF_Effect;

uint8_t MyAddress[] = {0xF0,0xF0,0xF0,0xF0,0xF0};
uint8_t TxAddress[] = {0xE7,0xF0,0xF0,0xF0,0xF0};

void TIM4_Config(void){
	
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	
	/* Enable the TIM4 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_TimeBaseStructure.TIM_Period = 19999; // maximum with ADC config between readings for touch 2 axis
  TIM_TimeBaseStructure.TIM_Prescaler = 41;//(uint16_t) ((SystemCoreClock /2) / 1000000) - 1;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	/* TIM IT enable */
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	/* TIM4 enable counter */
	TIM_Cmd(TIM4, ENABLE);
}

void TIM4_IRQHandler(void){
	
	static BaseType_t xHigherPriorityTaskWoken=pdFALSE;
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET){
		//Clear TIM2 IRQ flag//
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		xSemaphoreGiveFromISR(xSemaphore_RF,&xHigherPriorityTaskWoken);
	}
}

void RF_Config(void){
	/*SPI + NVIC + WHATEVER*/
	
	//SystemInit(); // initialize clocks
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	TIM4_Config();
	
	TM_NRF24L01_Init(50, 32); // Só funciona com 32 bytes de payload !!!SHAME!!!

	/* Set 250kBps data rate and 0dBm output power */
	TM_NRF24L01_SetRF(TM_NRF24L01_DataRate_250k, TM_NRF24L01_OutputPower_0dBm);

	TM_NRF24L01_SetMyAddress(MyAddress);
	TM_NRF24L01_SetTxAddress(TxAddress);
	
	TM_NRF24L01_PowerUpRx();	// start listening
}

void RF(void* args){
		//uint8_t msg;
		//uint8_t msg_send = 25;
		uint8_t buff[RF_BUFFERSIZE]={0},i=0;
		uint16_t rf_value=0;
		static BaseType_t xHigherPriorityTaskWoken=pdFALSE;
		
		NRF24L01_CLEAR_INTERRUPTS;	
		
		for(;;){
			//Waits for the semaphore given from the RF ISR
			xSemaphoreTake(xSemaphore_RF,portMAX_DELAY);
			GPIO_ToggleBits(GPIOD, GPIO_PIN_15);
			if(TM_NRF24L01_DataReady()){ 
				
				rf_value=0;
				for(i=0;i<RF_BUFFERSIZE;i++) 
					buff[i]=0;
				//Get data from NRF24L01+
				TM_NRF24L01_GetData((uint8_t*)buff);
				NRF24L01_CLEAR_INTERRUPTS;

				for(i=0;i<RF_BUFFERSIZE;i++)
					rf_value=rf_value*10+(buff[i]-48);
				
				if(rf_value>0)
					xQueueSend(xQueue_RF_Effect, &rf_value, 0);
			
			//msg = buff[0];
			//sendString(buff);
			//SendChar('\n');
			}
	}
}
