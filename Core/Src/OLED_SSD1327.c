/*
 * OLED_SSD1327.c
 *
 *  The MIT License.
 *  Created on: 16.07.2020
 *      Author: Mateusz Salamon
 *      www.msalamon.pl
 *      mateusz@msalamon.pl
 */

#include "main.h"

#include "OLED_SSD1327.h"

#include "string.h"

#ifdef SSD1327_I2C_CONTROL
I2C_HandleTypeDef *ssd1337_i2c;
#endif
#ifdef SSD1327_SPI_CONTROL
SPI_HandleTypeDef *ssd1337_spi;
#endif
#define SSD1327_BUFFERSIZE	(SSD1327_LCDHEIGHT * SSD1327_LCDWIDTH / 2)
static uint8_t buffer[SSD1327_BUFFERSIZE];

void SSD1327_Command(uint8_t com)
{
#ifdef SSD1327_I2C_CONTROL
	// I2C
	HAL_I2C_Mem_Write(ssd1337_i2c, SSD1327_I2C_ADDRESS, 0x00, 1, &com, sizeof(com), 100);
#endif
#ifdef SSD1327_SPI_CONTROL
	//SPI
	HAL_GPIO_WritePin(SSD1327_DC_GPIO_Port, SSD1327_DC_Pin, GPIO_PIN_RESET);
#ifndef SPI_CS_HARDWARE_CONTROL
	HAL_GPIO_WritePin(SSD1327_CS_GPIO_Port, SSD1327_CS_Pin, GPIO_PIN_RESET);
#endif
	HAL_SPI_Transmit(ssd1337_spi, &com, 1, 10);
#ifndef SPI_CS_HARDWARE_CONTROL
	HAL_GPIO_WritePin(SSD1327_CS_GPIO_Port, SSD1327_CS_Pin, GPIO_PIN_SET);
#endif
#endif
}

void SSD1327_Data(uint8_t dat)
{
#ifdef SSD1327_I2C_CONTROL
	// I2C
	HAL_I2C_Mem_Write(ssd1337_i2c, SSD1327_I2C_ADDRESS, 0x40, 1, &dat, sizeof(dat), 100);
#endif
#ifdef SSD1327_SPI_CONTROL
	// SPI
	HAL_GPIO_WritePin(SSD1327_DC_GPIO_Port, SSD1327_DC_Pin, GPIO_PIN_SET);
#ifndef SPI_CS_HARDWARE_CONTROL
	HAL_GPIO_WritePin(SSD1327_CS_GPIO_Port, SSD1327_CS_Pin, GPIO_PIN_RESET);
#endif
	HAL_SPI_Transmit(ssd1337_spi, &dat, 1, 10);
#ifndef SPI_CS_HARDWARE_CONTROL
	HAL_GPIO_WritePin(SSD1327_CS_GPIO_Port, SSD1327_CS_Pin, GPIO_PIN_SET);
#endif
#endif
}

#if defined(SSD1327_SPI_CONTROL) || defined(SSD1327_RESET_USE)
void SSD1327_Reset(void)
{
	HAL_GPIO_WritePin(SSD1327_RESET_GPIO_Port, SSD1327_RESET_Pin, GPIO_PIN_RESET);
	HAL_Delay(5);
	HAL_GPIO_WritePin(SSD1327_RESET_GPIO_Port, SSD1327_RESET_Pin, GPIO_PIN_SET);
}
#endif

//
// Configuration functions
//
void SSD1327_InvertColors(uint8_t Invert)
{
	SSD1327_Command(Invert ? SSD1327_INVERTDISPLAY : SSD1327_NORMALDISPLAY);
}

void SSD1327_RotateDisplay(uint8_t Rotate)
{
	if(Rotate > 1) Rotate = 1;

	SSD1327_Command(0xA0 | (0x01 & Rotate));  // Set Segment Re-Map Default
							// 0xA0 (0x00) => column Address 0 mapped to 127
                			// 0xA1 (0x01) => Column Address 127 mapped to 0

	SSD1327_Command(0xC0 | (0x08 & (Rotate<<3)));  // Set COM Output Scan Direction
							// 0xC0	(0x00) => normal mode (RESET) Scan from COM0 to COM[N-1];Where N is the Multiplex ratio.
							// 0xC8	(0xC8) => remapped mode. Scan from COM[N-1] to COM0;;Where N is the Multiplex ratio.
}

void SSD1327_DisplayON(uint8_t On)
{
	SSD1327_Command(On ? SSD1327_DISPLAYON : SSD1327_DISPLAYOFF);
}

void SSD1327_SetContrast(uint8_t Contrast)
{
	SSD1327_Command(SSD1327_SETCONTRASTCURRENT);	// Set Contrast Control
	SSD1327_Command(Contrast);
}

#if GRAPHIC_ACCELERATION_COMMANDS == 1
//
// Graphic Acceleration Command
//
void SSD1327_StartScrollRight(uint8_t StartPage, uint8_t EndPage, uint8_t Speed)
{
	SSD1327_Command(SSD1327_RIGHT_HORIZONTAL_SCROLL);
	SSD1327_Command(0x00);
	SSD1327_Command(StartPage);
	SSD1327_Command(Speed);
	SSD1327_Command(EndPage);
	SSD1327_Command(SSD1327_ACTIVATE_SCROLL);
}

void SSD1327_StartScrollLeft(uint8_t StartPage, uint8_t EndPage, uint8_t Speed)
{
	SSD1327_Command(SSD1327_LEFT_HORIZONTAL_SCROLL);
	SSD1327_Command(0x00);
	SSD1327_Command(StartPage);
	SSD1327_Command(Speed);
	SSD1327_Command(EndPage);
	SSD1327_Command(SSD1327_ACTIVATE_SCROLL);
}

void SSD1327_StopScroll(void)
{
	SSD1327_Command(SSD1327_DEACTIVATE_SCROLL);
}
#endif

//
// Initialization
//
void SSD1327_Init(void)
{
	SSD1327_Command(SSD1327_DISPLAYOFF);  // Display Off

	SSD1327_Command(SSD1327_SETMULTIPLEX);
	SSD1327_Command(0x5F);

	SSD1327_Command(SSD1327_SETDISPLAYSTARTLINE);
	SSD1327_Command(0x00);

	SSD1327_Command(SSD1327_SETDISPLAYOFFSET);
	SSD1327_Command(0x20);

	SSD1327_Command(SSD1327_SEGREMAP);
	SSD1327_Command(0x51);

	SSD1327_SetContrast(0x7F);

	SSD1327_Command(SSD1327_SETPHASELENGTH);
	SSD1327_Command(0x22);

	SSD1327_Command(SSD1327_SETFRONTCLOCKDIVIDER_OSCILLATORFREQUENCY);
	SSD1327_Command(0x50);

	SSD1327_Command(SSD1327_SELECTDEFAULTLINEARGRAYSCALETABLE);

	SSD1327_Command(SSD1327_SETPRECHARGEVOLTAGE);
	SSD1327_Command(0x10);

	SSD1327_Command(SSD1327_SETSETVCOMVOLTAGE);
	SSD1327_Command(0x05);

	SSD1327_Command(SSD1327_SETSECONDPRECHARGEPERTIOD);
	SSD1327_Command(0x0a);

	SSD1327_Command(SSD1327_FUNCTIONSELECTIONB);
	SSD1327_Command(0x62);

	SSD1327_Command(SSD1327_SETCOLUMNADDRESS);
	SSD1327_Command(0x00);
	SSD1327_Command(0x3F);

	SSD1327_Command(SSD1327_SETROWADDRESS);
	SSD1327_Command(0x00);
	SSD1327_Command(0x5F);

	SSD1327_Command(SSD1327_NORMALDISPLAY);  // Set Normal Display

	SSD1327_Command(SSD1327_DISPLAYALLON_RESUME);  // Entire Display ON

#if GRAPHIC_ACCELERATION_COMMANDS == 1
	SSD1327_StopScroll();
#endif

	SSD1327_DisplayON(1);
}

#ifdef SSD1327_I2C_CONTROL
void SSD1327_I2cInit(I2C_HandleTypeDef *i2c)
{
	ssd1337_i2c = i2c;

	SSD1327_Init();
}
#endif

#ifdef SSD1327_SPI_CONTROL
void SSD1327_SpiInit(SPI_HandleTypeDef *spi)
{
	ssd1337_spi = spi;

#if defined(SSD1327_RESET_USE)
	SSD1327_Reset();
#endif
	SSD1327_Init();
}
#endif

//
// Draw pixel in the buffer
//
void SSD1327_DrawPixel(int16_t x, int16_t y, uint8_t Color)
{
	 if ((x < 0) || (x >= SSD1327_LCDWIDTH) || (y < 0) || (y >= SSD1327_LCDHEIGHT))
		 return;

	 uint8_t SelectedCell = buffer[x/2 + y*(SSD1327_LCDWIDTH/2)];

	 if(x % 2)
	 {
		 SelectedCell &= ~(0x0F);
		 SelectedCell |= (0x0F & Color);
	 }
	 else
	 {
		 SelectedCell &= ~(0xF0);
		 SelectedCell |= (0xF0 & (Color<<4));
	 }

	 buffer[x/2 + y*(SSD1327_LCDWIDTH/2)] = SelectedCell;
}

//
// Clear the buffer
//
void SSD1327_Clear(uint8_t Color)
{
	if(Color > WHITE) Color = WHITE;

	memset(buffer, (Color << 4 | Color), SSD1327_BUFFERSIZE);
}

//
// Send buffer to OLDE GCRAM
//
void SSD1327_Display(void)
{
	SSD1327_Command(SSD1327_SETCOLUMNADDRESS);
	SSD1327_Command(0x00);
	SSD1327_Command(0x3F);

	SSD1327_Command(SSD1327_SETROWADDRESS);
	SSD1327_Command(0x00);
	SSD1327_Command(0x5F);

#ifdef SSD1327_I2C_CONTROL
#ifdef SSD1327_I2C_DMA_ENABLE
	if(ssd1337_i2c->hdmatx->State == HAL_DMA_STATE_READY)
	{
		HAL_I2C_Mem_Write_DMA(ssd1337_i2c, SSD1327_I2C_ADDRESS, 0x40, 1, (uint8_t*)&buffer, SSD1327_BUFFERSIZE);
	}
#else
	HAL_I2C_Mem_Write(ssd1337_i2c, SSD1327_I2C_ADDRESS, 0x40, 1, (uint8_t*)&buffer, SSD1327_BUFFERSIZE, 1000);
#endif
#endif
#ifdef SSD1327_SPI_CONTROL
#ifdef SSD1327_SPI_DMA_ENABLE
	HAL_GPIO_WritePin(SSD1327_DC_GPIO_Port, SSD1327_DC_Pin, GPIO_PIN_SET);
#ifndef SPI_CS_HARDWARE_CONTROL
	HAL_GPIO_WritePin(SSD1327_CS_GPIO_Port, SSD1327_CS_Pin, GPIO_PIN_RESET);
#endif
	if(ssd1337_spi->hdmatx->State == HAL_DMA_STATE_READY)
	{
		HAL_SPI_Transmit_DMA(ssd1337_spi, (uint8_t*)&buffer, SSD1327_BUFFERSIZE);
	}
#else
	HAL_GPIO_WritePin(SSD1327_DC_GPIO_Port, SSD1327_DC_Pin, GPIO_PIN_SET);
#ifndef SPI_CS_HARDWARE_CONTROL
	HAL_GPIO_WritePin(SSD1327_CS_GPIO_Port, SSD1327_CS_Pin, GPIO_PIN_RESET);
#endif
	HAL_SPI_Transmit(ssd1337_spi, (uint8_t*)&buffer, SSD1327_BUFFERSIZE, 100);
#ifndef SPI_CS_HARDWARE_CONTROL
	HAL_GPIO_WritePin(SSD1327_CS_GPIO_Port, SSD1327_CS_Pin, GPIO_PIN_SET);
#endif
#endif
#endif
}

//
// Display Bitmap directly on screen
//
void SSD1327_Bitmap(uint8_t *bitmap)
{
	SSD1327_Command(0x22);
	SSD1327_Command(0x00);
	SSD1327_Command(0x07);
#ifdef SSD1327_I2C_CONTROL
#ifdef SSD1327_I2C_DMA_ENABLE
	if(ssd1337_i2c->hdmatx->State == HAL_DMA_STATE_READY)
	{
		HAL_I2C_Mem_Write_DMA(ssd1337_i2c, SSD1327_I2C_ADDRESS, 0x40, 1, bitmap, (SSD1327_LCDHEIGHT * SSD1327_LCDWIDTH / 8));
	}
#else
	HAL_I2C_Mem_Write(ssd1337_i2c, SSD1327_I2C_ADDRESS, 0x40, 1, bitmap, (SSD1327_LCDHEIGHT * SSD1327_LCDWIDTH / 8), 100);
#endif
#endif
#ifdef SSD1327_SPI_CONTROL
#ifdef SSD1327_SPI_DMA_ENABLE
	HAL_GPIO_WritePin(SSD1327_DC_GPIO_Port, SSD1327_DC_Pin, GPIO_PIN_SET);
#ifndef SPI_CS_HARDWARE_CONTROL
	HAL_GPIO_WritePin(SSD1327_CS_GPIO_Port, SSD1327_CS_Pin, GPIO_PIN_RESET);
#endif
	if(ssd1337_spi->hdmatx->State == HAL_DMA_STATE_READY)
	{
		HAL_SPI_Transmit_DMA(ssd1337_spi, bitmap, (SSD1327_LCDHEIGHT * SSD1327_LCDWIDTH / 8));
	}
#else
	HAL_GPIO_WritePin(SSD1327_DC_GPIO_Port, SSD1327_DC_Pin, GPIO_PIN_SET);
#ifndef SPI_CS_HARDWARE_CONTROL
	HAL_GPIO_WritePin(SSD1327_CS_GPIO_Port, SSD1327_CS_Pin, GPIO_PIN_RESET);
#endif
	HAL_SPI_Transmit(ssd1337_spi, bitmap, (SSD1327_LCDHEIGHT * SSD1327_LCDWIDTH / 8), 100);
#ifndef SPI_CS_HARDWARE_CONTROL
	HAL_GPIO_WritePin(SSD1327_CS_GPIO_Port, SSD1327_CS_Pin, GPIO_PIN_SET);
#endif
#endif
#endif
}

#if defined(SSD1327_SPI_CONTROL) && !defined(SSD1327_SPI_DMA_ENABLE) && !defined(SPI_CS_HARDWARE_CONTROL)
void SSD1327_DmaEndCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi == ssd1337_spi)
	{
		HAL_GPIO_WritePin(SSD1327_CS_GPIO_Port, SSD1327_CS_Pin, GPIO_PIN_SET);
	}
}
#endif

