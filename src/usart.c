#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "stm8.h"
#include "stm8_io.h"
#include "board.h"
#include "usart.h"

#ifdef LOG
void usart_init()
{
	USART1_PSCR = 1; // Set prescaler to 1
	
	// Set clock divider to 9600 baud
	uint16_t div = 0x0682;
	USART1_BRR2 = ((div & 0xF000) >> 12) | (div & 0x000F);
	USART1_BRR1 = (div & 0x0FF0) >> 4;
	
	// Enable transmitter
	USART1_CR2 |= B3;
}

void usart_wait()
{
	while (!(USART1_SR & B6)); // Wait until tx complete
}

void usart_tx(uint8_t val)
{
	while (!(USART1_SR & B7)); // Wait until ready
	
	USART1_DR = val; // Transmit byte
}

void usart_tx_str(char* data, int len)
{
	for (int x = 0; x < len; x++)
	{
		usart_tx(data[x]);
	}
}
	
void usart_tx_hex(uint8_t hex)
{
	uint8_t up = (hex & 0xF0) >> 4;
	uint8_t low = (hex & 0x0F);
	
	usart_tx(up >= 10 ? ('A' + (up - 10)) : ('0' + up));
	usart_tx(low >= 10 ? ('A' + (low - 10)) : ('0' + low));
}
#endif