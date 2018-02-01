#include "pedal.h"

//Declare all tasks' handles to allow interaction with the tasks
TaskHandle_t 	xTask_BluetoothHandle,
							xTask_EffectsHandle,
							xTask_DisplayLCDHandle,
							xTask_RFHandle;

//Declare all queues' handles needed by the tasks
QueueHandle_t   xQueue_RF_Effect;

//Declare all semaphores' handles needed by the tasks

//Counter for Acquistion Rates
uint16_t counter;

//BaseType_t variable that holds the value returned when creating tasks
//Used to check whether the tasks were created successfully
BaseType_t xReturned;

void In_Buffered_Config(){

	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	//GPIO_ResetBits(GPIOE, GPIO_Pin_7);
	GPIO_SetBits(GPIOE, GPIO_Pin_7); //desligado
}

//Create Pedal's Tasks. Returns 0 if all tasks were created with success
int Create_Tasks(void){

	
	//Create the tBluetoothReceive Task. In case of error, exit function returning -1
	//Name: tBluetoothReceive
	//Thread Function: void BluetoothReceive(void*) , from "bluetooth.h"
	//Stack Size: 512 words = 2048 bytes
	//Arguments: 0
	//Priority: 7
	//Handler: xTask_BluetoothHandle
	xReturned = xTaskCreate(BluetoothReceive,"tBluetoothReceive",configMINIMAL_STACK_SIZE,
										NULL,1,&xTask_BluetoothHandle);
	if(xReturned!=pdPASS)
		return -1;
	
	//Create the tEffects Task. In case of error, exit function returning -1
	//Name: tProcessingEffects
	//Thread Function: void Effects(void*) , from "effects.h"
	//Stack Size: 512 words = 2048 bytes
	//Arguments: 0
	//Priority: 7
	//Handler: xTask_EffectsHandle
	xReturned = xTaskCreate(Effects,"tEffects",configMINIMAL_STACK_SIZE,
										NULL,1,&xTask_EffectsHandle);
	if(xReturned!=pdPASS)
		return -1;
	
	//Create the tRF Task. In case of error, exit function returning -1
	//Name: tRF
	//Thread Function: void RF(void*) , from "rf.h"
	//Stack Size: 512 words = 2048 bytes
	//Arguments: 0
	//Priority: 7
	//Handler: xTask_RFHandle
	xReturned = xTaskCreate(RF,"tRF",configMINIMAL_STACK_SIZE,
										NULL,1,&xTask_RFHandle);
	if(xReturned!=pdPASS)
		return -1;
	
	//Create the tDisplayLCD Task. In case of error, exit function returning -1
	//Name: tDisplayLCD
	//Thread Function: void DisplayLCD(void*) , from "lcd.h"
	//Stack Size: 512 words = 2048 bytes
	//Arguments: 0
	//Priority: 7
	//Handler: xTask_DisplayLCDHandle
	/*xReturned = xTaskCreate(DisplayLCD,"tDisplayLCD",configMINIMAL_STACK_SIZE,
										NULL,2,&xTask_DisplayLCDHandle);
	if(xReturned!=pdPASS)
		return -1;*/
	
	//All the tasks were created successfully, so return 0
	return 0;	
}

//Create Pedal's Message Queues. Returns 0 if all queues were created with success
static int Create_Queues(void){
		
	//Define Motorsqueue size and variable type
	//Size: 4
	//Type: char
	xQueue_RF_Effect = xQueueCreate(1, sizeof(int)) ;
	
	//Return 1 if at least one of the queues wasn't created successfully
	if(!xQueue_RF_Effect)
		return 1;

	//Return 0 if all queues were created successfully
	return 0;
}

//Create Pedal's Tasks. Returns 0 if all tasks were created with success
static int Create_Semaphores(void){

	//Create Binary Semaphores with the respective Handles
	xSemaphore_BluetoothReceive = xSemaphoreCreateBinary();
	xSemaphore_SignalAcquisition = xSemaphoreCreateBinary();
	xSemaphore_DisplayLCD = xSemaphoreCreateBinary();
	xSemaphore_RF = xSemaphoreCreateBinary();
	
	//Return 1 if one of the semaphores wasn't created successfully
	if(!xSemaphore_BluetoothReceive ||
		 !xSemaphore_DisplayLCD ||
		 !xSemaphore_RF||
		 !xSemaphore_SignalAcquisition)
		return 1;

	//Return 0 if all semaphores were created with success
	return 0;
}

//Start Pedal's Execution//
void Run(void){
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	int count=10;
	
	//Create Queues. Get stuck in infinite loop if it fails 10 times	
	while(Create_Queues() && count--);
	if(!count)
		//Stop execution
		while(1);

	//Create Semaphores. Get stuck in infinite loop if it fails 10 times
	count=10;
	
	while(Create_Semaphores() && count--);
	if(!count)
		//Stop execution
		while(1);
	
	//Timer Configuration for Signal Acquisition Rate
	In_Buffered_Config();
	Bluetooth_Config();
//	LCD_Config();
	RF_Config();
	Conv_Config();

	//Create Tasks. If one of them fails to be created, delete them all
	if(Create_Tasks()){
		vTaskDelete(xTask_BluetoothHandle);
		vTaskDelete(xTask_EffectsHandle);
		vTaskDelete(xTask_RFHandle);
		vTaskDelete(xTask_DisplayLCDHandle);
	}
	//If all tasks were created with success, continue execution
	else{
		//Start tasks' execution	
		vTaskStartScheduler();
	}
}
