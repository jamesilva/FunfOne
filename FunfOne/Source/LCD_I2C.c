#include "LCD_I2C.h"

//PCF	LCD	
//P0 	RS		
//P1 	RW  
//P2 	E		
//P3	LED		
//P4 	D4  
//P5 	D5 	
//P6 	D6	
//P7	D7
#define SLAVE_ADDRESS 0x3F // the slave address (example)

SemaphoreHandle_t xSemaphore_DisplayLCD;

uint8_t buf[1]={0};
uint8_t PortData=0;

void LCD_Config(void){
	
	init_I2C1();
	LCD_init();
	
	LCD_gotoxy(0,0);
	LCD_puts("LCD 2004 + PCF8574AT");
	LCD_gotoxy(1,2);
	LCD_puts("->->->->->->->->");
	LCD_gotoxy(2,4);
	LCD_puts("Hello Word!!");
	LCD_gotoxy(3,2);
	LCD_puts("-<-<-<-<-<-<-<-<");	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	
	//Config TIM2 IRQ with Preemption Priority 1//
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	//Initialize NVIC//
	NVIC_Init(&NVIC_InitStructure);

	//Config TIM2 with a Frequency of 10Hz//
	TIM_TimeBaseStructure.TIM_Prescaler = 40000;
	TIM_TimeBaseStructure.TIM_Period = 42-1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	//Initialize TIM2//
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);



}

void TIM2_IRQHandler(void){
	//BaseType to inform if a task with higher priority than//
	//the one running before the interrupt is ready to run//
	static BaseType_t xHigherPriorityTaskWoken=pdFALSE;
	
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET){
		//Clear TIM2 IRQ flag//
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		xSemaphoreGiveFromISR(xSemaphore_DisplayLCD,&xHigherPriorityTaskWoken);
	}
}

void init_I2C1(void){
	
	GPIO_InitTypeDef GPIO_InitStruct;
	I2C_InitTypeDef I2C_InitStruct;
	
	// enable APB1 peripheral clock for I2C1
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	// enable clock for SCL and SDA pins
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	/* setup SCL and SDA pins
	 * You can connect I2C1 to two different
	 * pairs of pins:
	 * 1. SCL on PB6 and SDA on PB7 
	 * 2. SCL on PB8 and SDA on PB9
	 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9; // we are going to use PB6 and PB7
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;			// set pins to alternate function
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		// set GPIO speed
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;			// set output to open drain --> the line has to be only pulled low, not driven high
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;			// enable pull up resistors
	GPIO_Init(GPIOB, &GPIO_InitStruct);					// init GPIOB
	
	// Connect I2C1 pins to AF  
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_I2C1);	// SCL
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1); // SDA
	
	// configure I2C1 
	I2C_InitStruct.I2C_ClockSpeed = 100000; 		// 100kHz
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;			// I2C mode
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;	// 50% duty cycle --> standard
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;			// own address, not relevant in master mode
	I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;		// disable acknowledge when reading (can be changed later on)
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // set address length to 7 bit addresses
	
	I2C_Init(I2C1, &I2C_InitStruct);				// init I2C1
	
	// enable I2C1
	I2C_Cmd(I2C1, ENABLE);
}

/* This function issues a start condition and 
 * transmits the slave address + R/W bit
 * 
 * Parameters:
 * 		I2Cx --> the I2C peripheral e.g. I2C1
 * 		address --> the 7 bit slave address
 * 		direction --> the tranmission direction can be:
 * 						I2C_Direction_Tranmitter for Master transmitter mode
 * 						I2C_Direction_Receiver for Master receiver
 */
void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction){
	// wait until I2C1 is not busy anymore
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
  
	// Send I2C1 START condition 
	I2C_GenerateSTART(I2Cx, ENABLE);
	  
	// wait for I2C1 EV5 --> Slave has acknowledged start condition
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
		
	// Send slave Address for write 
	I2C_Send7bitAddress(I2Cx, address, direction);
	  
	/* wait for I2C1 EV6, check if 
	 * either Slave has acknowledged Master transmitter or
	 * Master receiver mode, depending on the transmission
	 * direction
	 */ 
	if(direction == I2C_Direction_Transmitter){
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	}
	else if(direction == I2C_Direction_Receiver){
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	}
}

/* This function transmits one byte to the slave device
 * Parameters:
 *		I2Cx --> the I2C peripheral e.g. I2C1 
 *		data --> the data byte to be transmitted
 */
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data){
	I2C_SendData(I2Cx, data);
	// wait for I2C1 EV8_2 --> byte has been transmitted
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

/* This function reads one byte from the slave device 
 * and acknowledges the byte (requests another byte)
 */
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx){
	// enable acknowledge of recieved data
	I2C_AcknowledgeConfig(I2Cx, ENABLE);
	// wait until one byte has been received
	while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	// read data from I2C data register and return data byte
	uint8_t data = I2C_ReceiveData(I2Cx);
	return data;
}

/* This function reads one byte from the slave device
 * and doesn't acknowledge the recieved data 
 */
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx){
	// disabe acknowledge of received data
	// nack also generates stop condition after last byte received
	// see reference manual for more info
	I2C_AcknowledgeConfig(I2Cx, DISABLE);
	I2C_GenerateSTOP(I2Cx, ENABLE);
	// wait until one byte has been received
	while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	// read data from I2C data register and return data byte
	uint8_t data = I2C_ReceiveData(I2Cx);
	return data;
}

/* This funtion issues a stop condition and therefore
 * releases the bus
 */
void I2C_stop(I2C_TypeDef* I2Cx){
	// Send I2C1 STOP Condition 
	I2C_GenerateSTOP(I2Cx, ENABLE);
}


__STATIC_INLINE void DelayuUs(__IO uint32_t micros)
{
	micros *=(SystemCoreClock / 1000000) / 5;
	while (micros--);
}


void Write_I2C(unsigned char data){
	
	//*buf=data;
	int8_t received_data[2],nada;
	//HAL_I2C_Master_Transmit(&hi2c1,(uint16_t )addr,buf,1,1000);
	I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Transmitter);
	I2C_write(I2C1,data);
	I2C_stop(I2C1);	
	I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Receiver); // start a transmission in Master receiver mode
	received_data[0] = I2C_read_ack(I2C1); // read one byte and request another byte
	received_data[1] = I2C_read_nack(I2C1);
	/* anti warning lines, they're useless */
	nada=received_data[1]+received_data[0];
	received_data[1]=nada-received_data[0];
}

void Write_4bit(unsigned char data){
	//PortData=0;
	EN_SET();	
	DelayuUs(50);
	data<<=4;
	Write_I2C(PortData|data);
	EN_RESET();
	
	DelayuUs(50);
}

void Write_Byte(unsigned char data,int status){
	if(status==1) RS_SET();
	else RS_RESET();
	uint8_t DH=0; //Data High
	DH=data>>4;
	Write_4bit(DH);
	Write_4bit(data);
}

void HAL_Delay(int Count){
	while(Count--);
}

void LCD_init(){
	HAL_Delay(15);
	Write_4bit(0x03);	
	HAL_Delay(4);
	Write_4bit(0x03);
	DelayuUs(100);
	
	Write_4bit(0x03);
	HAL_Delay(1);
	
	Write_4bit(0x02);
	HAL_Delay(1);
	
	//Clear LCD
	Write_Byte(0x01,0);
	HAL_Delay(2);
	
	//Function Set: DL=0-4bit, N=1-2 line, F=5x10-5x8 dots
	Write_Byte(0x28,0); //4bit, 2 line, 5x8 dots
	DelayuUs(40);
	
	//Display ON/OFF cursor: D: display, C: cursor, B: blink cursor
	Write_Byte(0x0C,0); // not Cursor
	DelayuUs(40);
	
	//Entry Mode Set
	Write_Byte(0x06,0); //Insert new character in the right of old character. 
	DelayuUs(40);	
	
	LED_SET();
}

//Set DDRAM address
void LCD_gotoxy(unsigned char  x,unsigned char y){
	if(y>=20) y=19;
	
	if (x==0) Write_Byte(0x80 + y,0);
	else if(x==1) Write_Byte(0xC0+y,0);
	else if(x==2) Write_Byte(0x80+0x14+y,0);
	else  Write_Byte(0xC0+0x14+y,0);
}

void LCD_putc(char c){
	Write_Byte(c,1);
}

void LCD_puts(char *str){
	int i=0;
	while(str[i])
	{
		LCD_putc(str[i]);
		i++;
	}
}

void LCD_clear(){
	//Clear LCD
	Write_Byte(0x01,0);
	HAL_Delay(2);
	
	//Function Set: DL=0-4bit, N=1-2 line, F=5x10-5x8 dots
	Write_Byte(0x28,0); 
	DelayuUs(40);
	
	//Entry Mode Set
	Write_Byte(0x06,0);
	DelayuUs(40);
}

void DisplayLCD(void* args){
	
	char* preset;
		
	static BaseType_t xHigherPriorityTaskWoken=pdFALSE;
  for(;;){
		//LCD_puts("Hello Word!!");
		xSemaphoreTakeFromISR(xSemaphore_DisplayLCD,&xHigherPriorityTaskWoken);
		
		LCD_gotoxy(0,0);
		LCD_puts(preset);
		LCD_gotoxy(1,2);
		LCD_puts("->->->->->->->->");
		LCD_gotoxy(2,4);
		LCD_puts("Hello Word!!");
		LCD_gotoxy(3,2);
		LCD_puts("-<-<-<-<-<-<-<-<");	
  } 
}
