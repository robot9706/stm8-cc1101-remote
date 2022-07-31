#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "stm8.h"
#include "stm8_io.h"
#include "board.h"
#include "spi.h"
#include "cc1101.h"

void cc1101_select()
{
	WRITE_OFF(CC1101_CSN);
	
	// Wait until SO goes low
	while (READ(CC1101_SO))
	{
	}
}

void cc1101_deselect()
{
	WRITE_ON(CC1101_CSN);
}

void cc1101_reset()
{
	// Make sure NOT to use cc1101_ functions here as they manipulate CSn
	
	// CC1101 datasheet page 51
	WRITE_ON(CC1101_SCLK);
	WRITE_OFF(CC1101_SI);
	
	WRITE_OFF(CC1101_CSN);
	
	for (int x = 0; x < 100; x++) nop();
	
	WRITE_ON(CC1101_CSN);
	
	for (int x = 0; x < 1000; x++) nop(); //~40us
	
	cc1101_select();
	
	for (int x = 0; x < 100; x++) nop();
	
	spi_transfer(CC1101_SRES); // Avoid cc1101_strobe because it pulls CSn high
	
	// Wait until SO goes low
	while (READ(CC1101_SO))
	{
	}
	
	cc1101_deselect();
	
	// CC1101 is now reset and in IDLE
}

uint8_t cc1101_status(uint8_t addr)
{
	uint8_t cmd = addr | READ_BURST;

	cc1101_select();
	
	spi_transfer_multi_begin(cmd);
	uint8_t value = spi_transfer_multi(0, 1);

	cc1101_deselect();
	
	return value;
}

void cc1101_write_reg(uint8_t addr, uint8_t value)
{
	cc1101_select();
	
	spi_transfer_multi_begin(addr);
	spi_transfer_multi(value, 1);
	
	cc1101_deselect();
}

void cc1101_write_reg_burst(uint8_t addr, uint8_t* value, int len)
{
	uint8_t cmd = addr | WRITE_BURST;
	
	cc1101_select();
	
	spi_transfer_multi_begin(cmd);
	for (int x = 0; x < len; x++)
	{
		spi_transfer_multi(value[x], (x == len - 1));
	}
	
	cc1101_deselect();
}

uint8_t cc1101_strobe(uint8_t cmd)
{
	cc1101_select();
	
	uint8_t val = spi_transfer(cmd);
	
	cc1101_deselect();
	
	return val;
}