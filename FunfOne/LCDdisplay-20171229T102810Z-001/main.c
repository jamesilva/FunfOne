#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "FreeRTOS.h"
#include "task.h"
#define SLAVE_ADDRESS 0x3D 

#include "LCD_I2C.h"


void lcd(void* arg)
{
	
	 
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


  while (1)
  {
		LCD_puts("Hello Word!!");

  }
 
}





int main()
{
	xTaskCreate(lcd, "display", 128, NULL, 1, NULL);
	
	vTaskStartScheduler();
	
	while(1);
}