#ifndef _LCD_I2C_
#define _LCD_I2C_

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stm32f4xx_i2c.h>
#include <stm32f4xx_fmpi2c.h>

#define addr 0x7E 

#define EN_SET() Write_I2C(PortData|=0x04)
#define EN_RESET() Write_I2C(PortData&=~0x04)

#define RS_SET() Write_I2C(PortData|=0x01)
#define RS_RESET() Write_I2C(PortData&=~0x01)

#define RW_SET() Write_I2C(PortData|=0x02)
#define RW_RESET() Write_I2C(PortData&=~0x02)

#define LED_SET() Write_I2C(PortData|=0x08)
#define LED_RESET() Write_I2C(PortData&=~0x08)
//Address 0x38 , 0x7E(Write Mode)
//PCF	LCD	
//P0 	RS	 	
//P1 	RW  
//P2 	E		
//P3			
//P4 	D4  
//P5 	D5 	
//P6 	D6	
//P7	D7	

extern SemaphoreHandle_t xSemaphore_DisplayLCD;

void init_I2C1(void);
void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction);
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data);
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx);
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx);
void I2C_stop(I2C_TypeDef* I2Cx);
void LCD_init(void);
void LCD_gotoxy(unsigned char  x, unsigned char y);
void LCD_putc(char c);
void LCD_puts(char *str);
void LCD_clear(void );
void lcd_init (void);
void lcd_send_data (char data);
void lcd_send_cmd (char cmd);
void lcd_send_string (char *str);

void LCD_Config(void);
void TIM2_Config(void);
void DisplayLCD(void*);

#endif /*_LCD_I2C_*/
