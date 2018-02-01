#include "conversion.h"

#define DAC_DHR12R2_ADDRESS    0x40007414
#define DAC_DHR12R1_ADDRESS    0x40007408

SemaphoreHandle_t xSemaphore_SignalAcquisition;

/* Sine Wave
const uint16_t Sine12bit[64] = {
                      2048,2248,2447,2642,2831,3013,3185,3346,
											3495,3630,3750,3853,3939,4007,4056,4085,
											4095,4085,4056,4007,3939,3853,3750,3630,
											3495,3346,3185,3013,2831,2642,2447,2248,
											2048,1847,1648,1453,1264,1082,910,749,
											600,465,345,242,156,88,39,10,
											0,10,39,88,156,242,345,465,
											600,749,910,1082,1264,1453,1648,1847,
};
*/
/*q15_t PosInBuf[BUFFERSIZE];
q15_t NegInBuf[BUFFERSIZE];

q15_t PosBuf[BUFFERSIZE];
q15_t NegBuf[BUFFERSIZE];

q15_t PosOutBuf[BUFFERSIZE];
q15_t NegOutBuf[BUFFERSIZE];
*/

DAC_InitTypeDef  DAC_InitStructure;

//Config Peripheral Clocks used//
static void InRCC_Config(void){
	//Enable TIM3, GPIOC, DMA2, ADC2 & ADC3 Clocks//
	//APB1, AHB1, AHB1, APB2 & APB2 respectively//
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_AHB1PeriphClockCmd(CONV_ADCx_CHANNEL_GPIO_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	RCC_APB2PeriphClockCmd(PosADC_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(NegADC_CLK, ENABLE);
}

//Config USR GPIO Pins as Analog Input (AN)//
// Positive => PC1, Negative => PC5
static void InGPIO_Config(void){
	//GPIO Initialization Structure//
	GPIO_InitTypeDef GPIO_InitStructure;

	//Config PC5 & PC1 as Analog (AN)//
	GPIO_InitStructure.GPIO_Pin = PosGPIO_PIN | NegGPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	//Initialize GPIOC//
	GPIO_Init(CONV_GPIO_PORT, &GPIO_InitStructure);
}

//Config DMA2_Stream1 & DMA2_Stream2 ISRs//
static void NVIC_Config(void){
	//NVIC Initialization Structure//
	NVIC_InitTypeDef NVIC_InitStructure;

	//Config DMA2_Stream2 with Priority 1 (lower number means higher priority)//
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	//Initialize NVIC//
	NVIC_Init(&NVIC_InitStructure);
}


//Config TIM3 with Frequency of 400kHz//
static void TIM3_Config(void){
	//Timer Initialization Base Structure//
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	//Config Base Structure with Default Parameters//
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	//Config TIM3 with Frequency of 400kHz//
	TIM_TimeBaseStructure.TIM_Prescaler = PRESCALER - 1;
	TIM_TimeBaseStructure.TIM_Period = SAMPLERATE_PERIOD - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	//Initialize TIM3//
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	//Enable TIM3 Trigger and Running//
	TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
//	TIM_Cmd(TIM3, ENABLE);
}

//Config DMA to transfer ADCs conversions values directly to Wave Buffers//
//Right USR: DMA2_Stream1_CH2 => mapped to ADC3
//Left USR: DMA2_Stream2_CH1 => mapped to ADC2
static void InDMA_Config(void){

	//DMA Initialization Structure//
	DMA_InitTypeDef DMA_InitStructure;
 
	//Initialize DMA Streams 1 & 2 with Default Parameters//
	DMA_DeInit(PosDMA_STREAM);
	DMA_DeInit(NegDMA_STREAM);
	
	//DMA Configuration used for both USRs//
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
		DMA_InitStructure.DMA_BufferSize = BUFFERSIZE;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		//ADC Values are 12bits
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
		//Buffer Values are 16bits
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
		DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	
	//DMA Configuration for Left USR//
		//Select DMA Channel 1
		DMA_InitStructure.DMA_Channel = NegDMA_CHANNEL;
		//Transfer to LBuf
		DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)NegInDMA_STORE_VAL;
		//Transfer from ADC2 conversion value register
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)NegADC_DR_ADDRESS;
		//Initialize DMA2_Stream2
		DMA_Init(NegDMA_STREAM, &DMA_InitStructure);
	
	//DMA Configuration for Right USR
		//Select DMA Channel 2
		DMA_InitStructure.DMA_Channel = PosDMA_CHANNEL;
		//Transfer to RBuf
		DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)PosInDMA_STORE_VAL;
		//Transfer from ADC3 conversion value register
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)PosADC_DR_ADDRESS;
		//Initialize DMA2_Stream1
		DMA_Init(PosDMA_STREAM, &DMA_InitStructure); 
	
	//Enable Left USR DMA Stream & Transfer Complete Interrupt//
	//DMA_ITConfig(NegDMA_STREAM, DMA_IT_TC, ENABLE);
	DMA_Cmd(NegDMA_STREAM, ENABLE);

	//Enable Right USR DMA Stream Transfer Complete Interrupt//
	DMA_ITConfig(PosDMA_STREAM, DMA_IT_TC, ENABLE);
	DMA_Cmd(PosDMA_STREAM, ENABLE);
}

//Config ADC2 & ADC3//
static void ADC_Config(void){
	
	//ADC Common Initialization Structure//
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	//ADC Initialization Structure//
	ADC_InitTypeDef ADC_InitStructure;
	
	//Config Common ADC mode as Independent Multi ADC mode//
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
	//5 Cycles of minimum delay between each Conversion
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	//Initialize Common ADC//
	ADC_CommonInit(&ADC_CommonInitStructure);
	
	//Configurations for Both ADC2 & ADC3//
		ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
		//Disable Multi-Channel ADC
		ADC_InitStructure.ADC_ScanConvMode = DISABLE;
		//Enable Continuous Conversions
		ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
		//Enable Triggering
		ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
		//Triggered by TIM3
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
		//1 Conversion Channel per ADC
		ADC_InitStructure.ADC_NbrOfConversion = 1;
	
	//Init ADC2//
	ADC_Init(PosADC, &ADC_InitStructure);
	//Init ADC3//
	ADC_Init(NegADC, &ADC_InitStructure);
	//Init ADC2 CH11. Each Conversion takes 28 Cycles to complete//
	ADC_RegularChannelConfig(PosADC, PosADC_CHANNEL, 1, ADC_SampleTime_56Cycles);
	//Init ADC3 CH12. Each Conversion takes 28 Cycles to complete//
	ADC_RegularChannelConfig(NegADC, NegADC_CHANNEL, 1, ADC_SampleTime_56Cycles);
	
	//Enable Continuous Transfers//
	ADC_DMARequestAfterLastTransferCmd(PosADC,ENABLE);
	ADC_DMARequestAfterLastTransferCmd(NegADC,ENABLE);
}

void DMA2_Stream1_IRQHandler(void){

	static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	if(DMA_GetITStatus(DMA2_Stream1, DMA_IT_TCIF1)){
		//Clear Transfer Complete Flag//
		DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1);
		//Disable Right USR ADC (ADC3)
	
		xSemaphoreGiveFromISR(xSemaphore_SignalAcquisition,&xHigherPriorityTaskWoken);	
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

//Main Function of USR Configuration//
//Enable USR Readings//

void OutRCC_Config(void){
	/* DMA1 clock and GPIOA clock enable (to be used with DAC) */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 | RCC_AHB1Periph_GPIOA, ENABLE);
  /* DAC and TIM6 Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC | RCC_APB1Periph_TIM6, ENABLE);
}

void OutGPIO_Config(void){
	GPIO_InitTypeDef GPIO_InitStructure;
  /* DAC channel 1 & 2 (DAC_OUT1 = PA.4)(DAC_OUT2 = PA.5) configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void TIM6_Config(void){
  TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;

  /* Time base configuration */
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period = SAMPLERATE_PERIOD - 1;
  TIM_TimeBaseStructure.TIM_Prescaler = PRESCALER - 1;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

  /* TIM6 TRGO selection */
  TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);  
  /* TIM6 enable counter */
  TIM_Cmd(TIM6, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
}

void OutDMA_Config(void){
  DMA_InitTypeDef DMA_InitStructure;
	
  /* DMA1_Stream5 channel7 configuration */
  DMA_DeInit(DMA1_Stream5);
	DMA_DeInit(DMA1_Stream6);
	
  DMA_InitStructure.DMA_Channel = DMA_Channel_7;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)DAC_CH1_R12ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)PosOutDMA_STORE_VAL;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = BUFFERSIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream5, &DMA_InitStructure);

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)DAC_CH2_R12ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)NegOutDMA_STORE_VAL;
	DMA_Init(DMA1_Stream6, &DMA_InitStructure);

  /* Enable DMA1_Stream5/DMA1_Stream_6 */
  DMA_Cmd(DMA1_Stream5, ENABLE);
	DMA_Cmd(DMA1_Stream6, ENABLE);
}

void DAC_Config(void){
	//DAC_InitTypeDef  DAC_InitStructure;
	/* DAC channel2 Configuration */
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_2, &DAC_InitStructure);
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);
}
//Enables//
void Input_Enable(void){
	//Enable ADC2 and ADC2 with DMA
	ADC_DMACmd(NegADC, ENABLE);
	ADC_Cmd(NegADC, ENABLE);
	//Enable ADC3 and ADC3 with DMA
	ADC_DMACmd(PosADC, ENABLE);
	ADC_Cmd(PosADC, ENABLE);
}    
void Input_Disable(void){	//Disable ADC2 & ADC3
	//Disable ADC2 and ADC2 with DMA
	ADC_DMACmd(PosADC, DISABLE);
  ADC_Cmd(PosADC, DISABLE);
	//Disable ADC3 and ADC3 with DMA
  ADC_DMACmd(NegADC, DISABLE);
  ADC_Cmd(NegADC, DISABLE);
}

void Output_Enable(void){
	/* Enable DAC Channel2/Channel1 */
  DAC_Cmd(DAC_Channel_1, ENABLE);
	DAC_Cmd(DAC_Channel_2, ENABLE);
  /* Enable DMA for DAC Channel2/Channel1 */
  DAC_DMACmd(DAC_Channel_1, ENABLE);
	DAC_DMACmd(DAC_Channel_2, ENABLE);	
}    

void Output_Disable(void){	//Disable ADC2 & ADC3
	/* Disable DAC Channel2/Channel1 */
  DAC_Cmd(DAC_Channel_1, DISABLE);
	DAC_Cmd(DAC_Channel_2, DISABLE);
  /* Disable DMA for DAC Channel2/Channel1 */
  DAC_DMACmd(DAC_Channel_1, DISABLE);
	DAC_DMACmd(DAC_Channel_2, DISABLE);
}

void Conv_Config(void){
	Input_Config();
	Output_Config();
}
void Output_Config(void){	
	OutRCC_Config();
	OutGPIO_Config();
	TIM6_Config();
	DAC_Config();
	OutDMA_Config();
	
}
void Input_Config(void){	
	InRCC_Config();
	InGPIO_Config();
	TIM3_Config();
	NVIC_Config();
	InDMA_Config();
	ADC_Config();
}
