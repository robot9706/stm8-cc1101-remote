#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "stm8.h"
#include "stm8_io.h"
#include "board.h"
#include "spi.h"

void spi_init()
{	
	// SPI
	// Set CSn ON (deselect chip), set SCLK ON and SI off
	// this generates an SIDLE strobe (IF pin control is enabled)
	// (CC1101 datasheet page 51 (manual reset))
	
	OUTPUT(CC1101_CSN);
	WRITE_ON(CC1101_CSN);
	
	OUTPUT(CC1101_SCLK);
	WRITE_ON(CC1101_SCLK);
	
	OUTPUT(CC1101_SI);
	WRITE_OFF(CC1101_SI);
	
	INPUT_PULLUP(CC1101_SO);
	
	// SPI control
	SPI_CR1 = 0;
    SPI_CR2 = 0;
	
    SPI_CR1 &= ~SPI_CR1_LSBFIRST; // MSB first
    SPI_CR1 |= SPI_CR1_BR(0b111); // Baud Rate: 0b111 = fmaster / 256 (62,500 baud)
    SPI_CR1 &= ~SPI_CR1_CPOL; // SPI_CR1 CPOL=0 (Clock Phase, The first clock transition is the first data capture edge)
    SPI_CR1 &= ~SPI_CR1_CPHA; // SPI_CR1 CPHA=0 (Clock Polarity, SCK to 0 when idle)

    SPI_CR2 |= SPI_CR2_SSM; // bit 1 SSM=1 Software slave management, enabled
    SPI_CR2 |= SPI_CR2_SSI; // bit 0 SSI=1 Internal slave select, Master mode
    SPI_CR1 |= SPI_CR1_MSTR;  // CR1 bit 2 MSTR = 1, Master configuration.
}

uint8_t spi_transfer(uint8_t data)
{
	SPI_CR1 |= SPI_CR1_SPE; // SPE, SPI Enable, Peripheral enabled
	
    SPI_DR = data; // Transfer byte
    while ((SPI_SR & SPI_SR_BSY) || (SPI_SR & SPI_SR_TXE) == 0); // SPI is busy in communication or TX buffer is not empty
	
    SPI_CR1 &= ~SPI_CR1_SPE; // Disable SPI
	
    data = SPI_DR; // Received byte
    return data;
}

uint8_t spi_transfer_multi_begin(uint8_t data)
{
	SPI_CR1 |= SPI_CR1_SPE; // SPE, SPI Enable, Peripheral enabled
	
    SPI_DR = data; // Transfer byte
	
	while ((SPI_SR & SPI_SR_TXE) == 0); // Wait until TX buffer is not empty
	
	while ((SPI_SR & SPI_SR_RxNE) == 0); // Wait until receive buffer is not empty
	data = SPI_DR; // Read the data
	
	return data;
}

uint8_t spi_transfer_multi(uint8_t data, int end)
{
	while ((SPI_SR & SPI_SR_TXE) == 0); // Wait until TX buffer is not empty
	SPI_DR = data; // Write next data
	
	while ((SPI_SR & SPI_SR_RxNE) == 0); // Wait until receive buffer is not empty
	data = SPI_DR; // Receive byte
	
	if (end)
	{
		while ((SPI_SR & SPI_SR_BSY) || (SPI_SR & SPI_SR_TXE) == 0); // SPI is busy in communication or TX buffer is not empty
		
		SPI_CR1 &= ~SPI_CR1_SPE; // Disable SPI
	}
	
    return data;
}