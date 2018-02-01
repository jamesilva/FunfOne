#include "bluetooth.h"

static char Command[MAX_PRESET_SIZE]="";
static char PresetNum=0;
static char error=0;


struct s_trem 		s_FXtrem;
struct s_dist 		s_FXdist;
struct s_chorus 	s_FXchorus;
struct s_reverb 	s_FXreverb;
struct s_pitch		s_FXpitch;
struct s_volume 	s_FXvolume;
struct s_wah 			s_FXwah;
struct s_delay		s_FXdelay;
struct s_octaver 	s_FXoctaver;

char FXorder[5]="";
char preset[5][MAX_PRESET_SIZE]={0};

SemaphoreHandle_t 	xSemaphore_BluetoothReceive;
//**************************************************************************************

static void RCC_Configuration(void) {
	
		/* --------------------------- System Clocks Configuration -----------------*/
		/* USART3 clock enable */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
		/* GPIOC clock enable */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
}
 
//**************************************************************************************
 
static void GPIO_Configuration(void) {
	
		GPIO_InitTypeDef GPIO_InitStructure;
	
		//-------------------------- GPIO Configuration ----------------------------
		GPIO_InitStructure.GPIO_Pin = BLUETOOTH_TX | BLUETOOTH_RX;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
	
		// Connect USART pins to AF 
		GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);
		GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);
}
 
//**************************************************************************************
 
static void NVIC_Configuration(void) {

		NVIC_InitTypeDef NVIC_InitStructure;
	
		// Enable the USART3 Interrupt 
		NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
}

//**************************************************************************************

void Bluetooth_Config(void) {
	
		USART_InitTypeDef USART_InitStructure;
		
		RCC_Configuration();
		GPIO_Configuration();
		NVIC_Configuration();
		
		// ----------------------------USARTx configuration----------------------------
		/* USARTx configured as follow:
					- BaudRate = 28800 baud
					- Word Length = 8 Bits
					- One Stop Bit
					- No parity
					- Hardware flow control disabled (RTS and CTS signals)
					- Receive and transmit enabled
		*/
		USART_InitStructure.USART_BaudRate = 9600;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	
		USART_Init(USART3, &USART_InitStructure);
	
		USART_Cmd(USART3, ENABLE);
	
		USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
}
 
//**************************************************************************************
 
void USART3_IRQHandler(void) {
	
	static portBASE_TYPE xHigherPriorityTaskWoken;
	static int rx_index = 0;
	
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);

		Command[rx_index] = USART_ReceiveData(USART3);
		if(Command[rx_index++] == TERM_C){
			xHigherPriorityTaskWoken = pdFALSE;
			
			
			
			//Sends the semaphore for the Bluetooth Communication
			xSemaphoreGiveFromISR(xSemaphore_BluetoothReceive,&xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			rx_index = 0;
		}
	}
}
 
//**************************************************************************************

void BluetoothReceive(void* args) {
	
	int i=0, j=0, end=0;
	
		//static BaseType_t xHigherPriorityTaskWoken=pdFALSE;
		for(;;) {
			//Waits for the semaphore given from the Bluetooth ISR
			xSemaphoreTake(xSemaphore_BluetoothReceive, portMAX_DELAY);
			/* Insert Code C: */
			end=0;
			i=0;
			GPIO_ToggleBits(GPIOD, GPIO_Pin_14);
			while(!end){
			switch(Command[i++]){
				case PRESET_C: // case change in preset number
					
					PresetNum=Command[i++]-48-1;
					for(j=0;j<MAX_PRESET_SIZE;j++)
						preset[PresetNum][j]=Command[j];
					break;
				case ADD_EFFECT_C:
					
					// adding or editing effect
					switch(Command[i++]){
						case TREM:
							FXorder[Command[i++]-48]=TREM;
							s_FXtrem.depth=(Command[i++]-48)*10;
						  s_FXtrem.depth+=(Command[i++]-48);
							if(Command[i]=='X'){
								s_FXtrem.tremolo_rate_efeito = 1;
								i = i + 2;
							}
							else{
								s_FXtrem.tremolo_rate_efeito = 0;
								s_FXtrem.rate=(Command[i++]-48)*10;
								s_FXtrem.rate+=(Command[i++]-48);
							}
							break;
						case DISTORTION:
							FXorder[Command[i++]-48]=DISTORTION;
							//GPIO_ToggleBits(GPIOD, GPIO_Pin_15);
							s_FXdist.gain=(Command[i++]-48)*10;
							s_FXdist.gain+=(Command[i++]-48);
							break;
						case CHORUS:
							FXorder[Command[i++]-48]=CHORUS;
							//GPIO_ToggleBits(GPIOD, GPIO_Pin_14);
							s_FXchorus.depth=(Command[i++]-48)*10;
						  s_FXchorus.depth+=(Command[i++]-48);
							if(Command[i]=='X'){
								s_FXchorus.chorus_rate_efeito = 1;
								i = i + 2;
							}
							else{
								s_FXchorus.chorus_rate_efeito = 0;
								s_FXchorus.rate=(Command[i++]-48)*10;
								s_FXchorus.rate+=(Command[i++]-48);
							}
							break;
						case REVERB:
							FXorder[Command[i++]-48]=REVERB;
							s_FXreverb.decay=(Command[i++]-48)*10;
						  s_FXreverb.decay+=(Command[i++]-48);
							s_FXreverb.density=(Command[i++]-48)*10;
						  s_FXreverb.density+=(Command[i++]-48);
							break;
						case PITCH:
							FXorder[Command[i++]-48]=PITCH;
							if(Command[i]=='X'){
								s_FXpitch.pitch_shift_efeito = 1;
								i = i + 2;
							}
							else{
								s_FXpitch.pitch_shift_efeito = 0;
								s_FXpitch.pitch=(Command[i++]-48)*10;
								s_FXpitch.pitch+=(Command[i++]-48);
							}
							break;
						case VOLUME:
							FXorder[Command[i++]-48]=VOLUME;
							s_FXvolume.volume_efeito = 1;
							break;
						case WAH:
							FXorder[Command[i++]-48]=WAH;	
							s_FXvolume.volume_efeito = 1;
							break;
						case DELAY:
							FXorder[Command[i++]-48]=DELAY;
							if(Command[i]=='X'){
								s_FXdelay.delay_time_efeito = 1;
								i = i + 2;
							}
							else{
								s_FXdelay.delay_time_efeito = 0;
								s_FXdelay.time=(Command[i++]-48)*10;
								s_FXdelay.time+=(Command[i++]-48);
							}
							s_FXdelay.feedback=(Command[i++]-48)*10;
						  s_FXdelay.feedback+=(Command[i++]-48);
							break;
						case OCTAVER:
							FXorder[Command[i++]-48]=OCTAVER;											
							s_FXoctaver.dir_volume=(Command[i++]-48)*10;
							s_FXoctaver.dir_volume+=(Command[i++]-48);
							s_FXoctaver.oct_volume=(Command[i++]-48)*10;
							s_FXoctaver.oct_volume+=(Command[i++]-48);
							s_FXoctaver.octave=Command[i++]-48;
							break;
						case NO_EFFECT:
							FXorder[Command[i++]-48]=NO_EFFECT;
							break;
						default:
							error=1;
							break;
					}
					break;
				case TERM_C:
					
					end=1;
					break;
				default:
					//ERROR
					error=1;
					break;
					}
				}
				for(j=0;j<MAX_PRESET_SIZE;j++){
					Command[j]=0;			
				}
	}
}
 
//**************************************************************************************
