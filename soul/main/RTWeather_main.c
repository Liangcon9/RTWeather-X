/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "driver/gpio.h"
#include "driver/spi.h"

#define LCD_RST GPIO_NUM_4
#define LCD_CS  GPIO_NUM_15
#define LCD_RS  GPIO_NUM_0

#define LCD_GPIO_MASK ((1 << LCD_RST) | (1 << LCD_CS) | (1 << LCD_RS))

#define LCD_RST_ENABLE  gpio_set_level(LCD_RST,0);
#define LCD_RST_DISABLE gpio_set_level(LCD_RST,1);
#define LCD_RS_COMMAND  gpio_set_level(LCD_RS,0);
#define LCD_RS_DATA     gpio_set_level(LCD_RS,1);

#define LCD_COLOR_BLACK 0x0000

typedef struct 
{
    uint16_t width;			//LCD ����
	uint16_t height;			//LCD �߶�
	uint16_t id;				//LCD ID
	uint8_t  dir;			//���������������ƣ�0��������1��������	
	uint16_t  wramcmd;		//��ʼдgramָ��
	uint16_t  setxcmd;		//����x����ָ��
	uint16_t  setycmd;		//����y����ָ��	 
}LCD_Dev_t;

LCD_Dev_t lcddev;

static void delay_ms(uint32_t time)
{
    vTaskDelay(time / portTICK_PERIOD_MS);
}

void LCD_GPIO_Init(void)
{
    gpio_config_t lcd_gpio_config;
    lcd_gpio_config.pin_bit_mask = LCD_GPIO_MASK;
    lcd_gpio_config.mode = GPIO_MODE_OUTPUT;
    lcd_gpio_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    lcd_gpio_config.pull_up_en = GPIO_PULLUP_ENABLE;
    lcd_gpio_config.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&lcd_gpio_config);
}

void LCD_SPI_Init(void)
{
    spi_config_t spi_config;
    spi_config.mode = SPI_MASTER_MODE;
    spi_config.clk_div = SPI_2MHz_DIV;
    spi_config.event_cb = NULL;
    spi_config.intr_enable = SPI_MASTER_DEFAULT_INTR_ENABLE;
    spi_config.interface = SPI_DEFAULT_INTERFACE;
    spi_config.interface.cpha = SPI_CPHA_HIGH;
    spi_config.interface.cpol = SPI_CPOL_HIGH;
    spi_config.interface.bit_tx_order = SPI_BIT_ORDER_MSB_FIRST;
    spi_config.interface.mosi_en = 1;
    spi_init(HSPI_HOST, &spi_config);
}

void LCD_Write_Command(uint8_t command)
{
    spi_trans_t trans;
    uint32_t u32_cmd = (command << 24);
    memset(&trans,sizeof(trans),0);

    trans.bits.mosi = 8;
    trans.mosi = &u32_cmd;

    LCD_RS_COMMAND
    spi_trans(HSPI_HOST, &trans);
}

void LCD_Write_Data(uint8_t data)
{
    spi_trans_t trans;
    uint32_t u32_data = (data << 24);
    memset(&trans,sizeof(trans),0);

    trans.bits.mosi = 8;
    trans.mosi = &u32_data;

    LCD_RS_DATA
    spi_trans(HSPI_HOST, &trans);
}

LCD_Write_Data_16Bit(uint16_t data)
{
    spi_trans_t trans;
    uint32_t u32_data = (data << 16);
    memset(&trans,sizeof(trans),0);

    trans.bits.mosi = 16;
    trans.mosi = &u32_data;

    LCD_RS_DATA
    spi_trans(HSPI_HOST, &trans);
}

void LCD_RESET(void)
{
	LCD_RST_ENABLE;
	delay_ms(100);	
	LCD_RST_DISABLE;
	delay_ms(50);
}

void LCD_SetParam(void)
{ 	
	lcddev.wramcmd=0x2C;
	lcddev.dir=0;//����				 	 		
	lcddev.width=128;
	lcddev.height=128;
	lcddev.setxcmd=0x2A;
	lcddev.setycmd=0x2B;	
    LCD_Write_Command(0x36);
    LCD_Write_Data(0xC8);
}	

void LCD_WriteRAM_Prepare(void)
{
	LCD_Write_Command(lcddev.wramcmd);
}	

void LCD_SetWindows(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd)
{

	LCD_Write_Command(lcddev.setxcmd);	
	LCD_Write_Data(xStar>>8);
	LCD_Write_Data(0x00FF&xStar+2);		
	LCD_Write_Data(xEnd>>8);
	LCD_Write_Data(0x00FF&xEnd+2);

	LCD_Write_Command(lcddev.setycmd);	
	LCD_Write_Data(yStar>>8);
	LCD_Write_Data(0x00FF&yStar+3);		
	LCD_Write_Data(yEnd>>8);
	LCD_Write_Data(0x00FF&yEnd+3);	

	LCD_WriteRAM_Prepare();	//��ʼд��GRAM				
}   

void LCD_Init(void)
{
    LCD_GPIO_Init();
    LCD_SPI_Init();
    LCD_RESET();

    //Param setting
    LCD_Write_Command(0x11);//Sleep exit 
	delay_ms(120);		
	//ST7735R Frame Rate
	LCD_Write_Command(0xB1); 
	LCD_Write_Data(0x01); 
	LCD_Write_Data(0x2C); 
	LCD_Write_Data(0x2D); 

	LCD_Write_Command(0xB2); 
	LCD_Write_Data(0x01); 
	LCD_Write_Data(0x2C); 
	LCD_Write_Data(0x2D); 

	LCD_Write_Command(0xB3); 
	LCD_Write_Data(0x01); 
	LCD_Write_Data(0x2C); 
	LCD_Write_Data(0x2D); 
	LCD_Write_Data(0x01); 
	LCD_Write_Data(0x2C); 
	LCD_Write_Data(0x2D); 
	
	LCD_Write_Command(0xB4); //Column inversion 
	LCD_Write_Data(0x07); 
	
	//ST7735R Power Sequence
	LCD_Write_Command(0xC0); 
	LCD_Write_Data(0xA2); 
	LCD_Write_Data(0x02); 
	LCD_Write_Data(0x84); 
	LCD_Write_Command(0xC1); 
	LCD_Write_Data(0xC5); 

	LCD_Write_Command(0xC2); 
	LCD_Write_Data(0x0A); 
	LCD_Write_Data(0x00); 

	LCD_Write_Command(0xC3); 
	LCD_Write_Data(0x8A); 
	LCD_Write_Data(0x2A); 
	LCD_Write_Command(0xC4); 
	LCD_Write_Data(0x8A); 
	LCD_Write_Data(0xEE); 
	
	LCD_Write_Command(0xC5); //VCOM 
	LCD_Write_Data(0x0E); 
	
	LCD_Write_Command(0x36); //MX, MY, RGB mode 				 
	LCD_Write_Data(0xC8); 
	
	//ST7735R Gamma Sequence
	LCD_Write_Command(0xe0); 
	LCD_Write_Data(0x0f); 
	LCD_Write_Data(0x1a); 
	LCD_Write_Data(0x0f); 
	LCD_Write_Data(0x18); 
	LCD_Write_Data(0x2f); 
	LCD_Write_Data(0x28); 
	LCD_Write_Data(0x20); 
	LCD_Write_Data(0x22); 
	LCD_Write_Data(0x1f); 
	LCD_Write_Data(0x1b); 
	LCD_Write_Data(0x23); 
	LCD_Write_Data(0x37); 
	LCD_Write_Data(0x00); 	
	LCD_Write_Data(0x07); 
	LCD_Write_Data(0x02); 
	LCD_Write_Data(0x10); 

	LCD_Write_Command(0xe1); 
	LCD_Write_Data(0x0f); 
	LCD_Write_Data(0x1b); 
	LCD_Write_Data(0x0f); 
	LCD_Write_Data(0x17); 
	LCD_Write_Data(0x33); 
	LCD_Write_Data(0x2c); 
	LCD_Write_Data(0x29); 
	LCD_Write_Data(0x2e); 
	LCD_Write_Data(0x30); 
	LCD_Write_Data(0x30); 
	LCD_Write_Data(0x39); 
	LCD_Write_Data(0x3f); 
	LCD_Write_Data(0x00); 
	LCD_Write_Data(0x07); 
	LCD_Write_Data(0x03); 
	LCD_Write_Data(0x10);  
	
	LCD_Write_Command(0x2a);
	LCD_Write_Data(0x00);
	LCD_Write_Data(0x00);
	LCD_Write_Data(0x00);
	LCD_Write_Data(0x7f);

	LCD_Write_Command(0x2b);
	LCD_Write_Data(0x00);
	LCD_Write_Data(0x00);
	LCD_Write_Data(0x00);
	LCD_Write_Data(0x9f);
	
	LCD_Write_Command(0xF0); //Enable test command  
	LCD_Write_Data(0x01); 
	LCD_Write_Command(0xF6); //Disable ram power save mode 
	LCD_Write_Data(0x00); 
	
	LCD_Write_Command(0x3A); //65k mode 
	LCD_Write_Data(0x05); 	
	LCD_Write_Command(0x29);//Display on	

    LCD_SetParam();
}

void LCD_Clear(uint16_t Color)
{
	uint16_t i,j;      
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);	  
	for(i=0;i<lcddev.width;i++)
	{
		for(j=0;j<lcddev.height;j++)
		LCD_Write_Data_16Bit(Color);	//What you need to show on the LCD	 
	}
}  

void app_main()
{
    LCD_Init(void);
    LCD_Clear(LCD_COLOR_BLACK);
}
