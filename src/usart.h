#ifndef __USART__
#define __USART__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "board.h"

#ifdef LOG
void usart_init();
void usart_wait();
void usart_tx(uint8_t val);
void usart_tx_str(char* data, int len);
void usart_tx_hex(uint8_t hex);
#endif

#endif