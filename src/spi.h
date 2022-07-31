#ifndef __SPI__
#define __SPI__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

void spi_init();

// Transfer one byte
uint8_t spi_transfer(uint8_t data);

// Transfer multiple bytes
uint8_t spi_transfer_multi_begin(uint8_t data);
uint8_t spi_transfer_multi(uint8_t data, int end);

#endif