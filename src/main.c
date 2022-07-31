#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "stm8.h"
#include "stm8_io.h"
#include "board.h"
#include "usart.h"
#include "spi.h"
#include "cc1101.h"

// RF codes
struct rf_code {
	uint8_t *code;
	int length;
};

static uint8_t code1[] = { 0xAA, 0x12, 0x34 };
static uint8_t code2[] = { 0xAA, 0x12, 0x56 };
static uint8_t code3[] = { 0xAA, 0x78, 0x9A, 0xBC };
static uint8_t code4[] = { 0xAA, 0x78, 0xDE, 0xEF };

static struct rf_code codes[4] = {
	// 0
	{
		.code = code1,
		.length = 3
	},
	
	// 1
	{
		.code = code2,
		.length = 3
	},
	
	// 2
	{
		.code = code3,
		.length = 4
	},
	
	// 3
	{
		.code = code4,
		.length = 4
	}
};

// IRQ variables
static volatile int buttonTrigger = 0; // Which button is triggered
static volatile int tim1IRQ = 0; // TIM1 overflow

// Variables
static volatile int codeIndexToTransmit = 0;
static volatile int cc1101TX = 0; // Is CC1101 transmitting a packet?
static volatile int transmitting = 0; // Is the remote transmitting?

static void enable_peripherals()
{
	// Enable SPI1 clock
	CLK_PCKENR1 |= B4; // PCKEN14
	
	// Enable USART1 clock
	CLK_PCKENR1 |= B5; // PCKEN15	
	
	// Enable TIM1 clock
	CLK_PCKENR2 |= B1;
}

static void disable_peripherals()
{
	CLK_PCKENR1 = 0x00;
	CLK_PCKENR2 = 0x80; // Return to default value (BootROM)
}

static void system_init()
{
	CLK_DIVR = 0; // Set CPU to full speed
	
	SYSCFG_RMPCR1 = 0x2C; // Remap USART1 to PC5 (TX) and PC6 (RX)
	
	enable_peripherals();
}

static void gpio_init()
{
	#ifdef LOG
	OUTPUT(LOG_TX);
	usart_init();
	usart_tx_str("BOOT\r\n", 6);
	#endif
	
	// Buttons
	INPUT_PULLUP(BUTTON1);
	INPUT_PULLUP(BUTTON2);
	INPUT_PULLUP(BUTTON3);
	INPUT_PULLUP(BUTTON4);
	
	// Button IRQs
	INPUT_IRQ_ENABLE(BUTTON1);
	INPUT_IRQ_ENABLE(BUTTON2);
	INPUT_IRQ_ENABLE(BUTTON3);
	INPUT_IRQ_ENABLE(BUTTON4);
	
	// Set PA/PB[3:0] to falling edge detection
	EXTI_CR1 = IRQ_FALLING_EDGE | (IRQ_FALLING_EDGE << 2) | (IRQ_FALLING_EDGE << 4) | (IRQ_FALLING_EDGE << 6);
	
	// LED
	OUTPUT(LED);
	WRITE_ON(LED);
	
	// SPI
	spi_init();
	
	// CC1101 IO
	INPUT_FLOAT(CC1101_GDO0);
	INPUT_FLOAT(CC1101_GDO2);
}

static void tim1_init()
{	
	// Divide fosc by 256
	TIM1_PSCRH = 0x01;
	TIM1_PSCRL = 0x00;
	
	// Set the counter to 1000
	TIM1_ARRH = 0x1E;
	TIM1_ARRL = 0x84;
	
	// Enable updates interrupt
	TIM1_IER = TIM_IER_UIE;
	
	// Auto-reload + IRQ on overflow
	TIM1_CR1 = TIM_CR1_APRE | TIM_CR1_OPM | TIM_CR1_URS | TIM_CR1_CEN;
}

static void cc1101_register_setup()
{
	// Make sure the CC1101 is in IDLE
	cc1101_strobe(CC1101_SIDLE);
	
	// Set GDO0 to CHIP_RDYn, INV off, no temp sensor (?) - CC1101 datasheet (page 62, table 41)
	// GDO0 by default is fosc / 192 - for optimal RF performance this is required to be changed
	cc1101_write_reg(CC1101_IOCFG0, 0x29);
	
	// Set GDO2 output mode, asserts when TX starts, deasserts when TX ends
	cc1101_write_reg(CC1101_IOCFG2, 0x06);
	
	// Set the PA table
	// In OOK mode [0] is used when transmiting a '0' and [1] is used when transmiting a '1'
	uint8_t paTable[] = { 0x00, 0xC2 };
	cc1101_write_reg_burst(CC1101_PATABLE, paTable, 2);
	
	// Set the frequency to 433MHz, considering the 24Mhz crystal
	cc1101_write_reg(CC1101_FREQ2, 0x12);
	cc1101_write_reg(CC1101_FREQ1, 0x14);
	cc1101_write_reg(CC1101_FREQ0, 0x7A);
	
	// Reset the SYNC word
	cc1101_write_reg(CC1101_SYNC1, 0x00);
	cc1101_write_reg(CC1101_SYNC0, 0x00);
	
	// Set channel number to 0
	cc1101_write_reg(CC1101_CHANNR, 0x00);

	// Setup PA power
	cc1101_write_reg(CC1101_FREND0, 0x11);
	
	// Channel spacing ~200kHz
	cc1101_write_reg(CC1101_MDMCFG0, 0xF8);
	
	// No FEC, 0 preamble, set channel spacing
	cc1101_write_reg(CC1101_MDMCFG1, 0x02);

	// Set OOK and no sync-word / preamble
	cc1101_write_reg(CC1101_MDMCFG2, 0x30);	
	
	// RX filter bw & data date
	cc1101_write_reg(CC1101_MDMCFG4, 0x86);
	cc1101_write_reg(CC1101_MDMCFG3, 0xE5);
	
	// No data whitening, No CRC, packet in FIFO, fixed packet length in PKTLEN
	cc1101_write_reg(CC1101_PKTCTRL0, 0x00);
}

// Bytes to OOK data
static int bytesToRf(uint8_t *bytes, int numBytes, uint8_t *buffer, int bufferSize)
{
	int bitPointer = 0;
	
	#define SKIP_BIT \
	{ \
		bitPointer++; \
		if (bitPointer >= bufferSize * 8) { \
			return -1; \
		} \
	}
	
	#define WRITE_BIT \
	{ \
		int _byteIndex = bitPointer / 8; \
		int _bitPointer = bitPointer % 8; \
		buffer[_byteIndex] |= 1 << (7 - _bitPointer); \
		SKIP_BIT \
	}
	
	// Intro
	SKIP_BIT;
	SKIP_BIT;
	SKIP_BIT;
	SKIP_BIT;
	SKIP_BIT;
	SKIP_BIT;
	SKIP_BIT;
	SKIP_BIT;
	
	for (int x = 0; x < numBytes; x++)
	{
		for (int b = 0; b < 8; b++)
		{
			int mask = 1 << (7 - b);
			if ((bytes[x] & mask) == mask)
			{
				WRITE_BIT;
				WRITE_BIT;
				SKIP_BIT;
			}
			else
			{
				WRITE_BIT;
				SKIP_BIT;
				SKIP_BIT;
			}
		}
	}
	
	return (bitPointer + 7) / 8;
}

static void cc1101_transmit()
{
	// Setup data to be transmitted
	struct rf_code *rf = &codes[codeIndexToTransmit];
	
	uint8_t fifoBuffer[64] = { 0 };
	int dataLen = 0;
	
	dataLen = bytesToRf(rf->code, rf->length, fifoBuffer, 64);
	
	if (dataLen <= 0)
	{
		#ifdef LOG
		usart_tx_str("RF_INV\r\n", 8);
		#endif
		
		return;
	}
	
	#ifdef LOG
	usart_tx_str("T1 ", 3);
	#endif
	
	cc1101_strobe(CC1101_SIDLE); // Force the CC1101 into IDLE
	cc1101_strobe(CC1101_SFTX); // Flush the TX fifo
	
	#ifdef LOG
	usart_tx_str("T2 ", 3);
	#endif
			
	while (CC1101_STATE(cc1101_strobe(CC1101_SNOP)) != 0) {} // Wait for an IDLE state
	
	#ifdef LOG
	usart_tx_str("T3 ", 3);
	#endif
							
	cc1101_strobe(CC1101_SCAL); // Calibrate the crystal
					
	while (CC1101_STATE(cc1101_strobe(CC1101_SNOP)) != 0) {} // Wait for an IDLE state
	
	#ifdef LOG
	usart_tx_str("T4 ", 3);
	#endif
	
	// Write the data length
	cc1101_write_reg(CC1101_PKTLEN, dataLen);
	
	// Write the TX FIFO
	cc1101_write_reg_burst(CC1101_TXFIFO, fifoBuffer, dataLen);
					
	// Transmit
	cc1101_strobe(CC1101_STX);
	
	#ifdef LOG
	usart_tx_str("TX\r\n", 4);
	#endif
}

// Turn off peripherals, power down CC1101 and HALT the CPU
static void system_sleep()
{
	#ifdef LOG
	usart_wait();
	#endif
	
	cc1101_strobe(CC1101_SPWD);
	WRITE_ON(CC1101_CSN); // Make sure CSn is on after SPWD
	
	disable_peripherals();
	
	halt(); // HALT the CPU, wakes on IRQ
	
	// Enable peripherals after waking up
	enable_peripherals();
	
	// Reset CC1101
	cc1101_reset();
	cc1101_register_setup();
}

// Entry point
int main()
{
	disableIRQ();
	
	system_init();
	gpio_init();
	spi_init();
	
	cc1101_reset();
	cc1101_register_setup();
	
	enableIRQ();
	
	while(1)
	{
		// Is button triggered?
		if (buttonTrigger != 0)
		{
			#ifdef LOG
			usart_tx_str("B_IRQ=", 6);
			usart_tx_hex(buttonTrigger);
			usart_tx_str("\r\n", 2);
			#endif
		
			if (!transmitting)
			{
				codeIndexToTransmit = buttonTrigger - 1; // Buttons (1-4) to index (0-3)
				
				transmitting = 1;
				cc1101TX = 0;
				
				#ifdef LOG
				usart_tx_str("TX_I=", 5);
				usart_tx_hex(codeIndexToTransmit);
				usart_tx_str("\r\n", 2);
				#endif
				
				cc1101_transmit();
			}
		
			buttonTrigger = 0;
		}
		
		if (transmitting)
		{
			if (cc1101TX == 0)
			{
				if (READ(CC1101_GDO2))
				{
					WRITE_OFF(LED);
					
					// GDO2 went high, packet is being transmitted
					cc1101TX = 1;
				}
			}
			else
			{
				if (!READ(CC1101_GDO2))
				{
					WRITE_ON(LED);
					
					// GDO2 went low, packet is done
					cc1101TX = 0;
					
					// Check if any of the buttons are still pressed
					if (!READ(BUTTON1) || !READ(BUTTON2) || !READ(BUTTON3) || !READ(BUTTON4))
					{
						#ifdef LOG
						usart_tx_str("CONT\r\n", 6);
						#endif
						
						tim1_init(); // Use TIM1 to time another transmission
					}
					else	
					{
						transmitting = 0;
					}	
				}
			}
		}
		
		if (tim1IRQ)
		{
			tim1IRQ = 0;
			
			#ifdef LOG
			usart_tx_str("TTTT\r\n", 6);
			#endif
			
			if (transmitting)
			{
				#ifdef LOG
				usart_tx_str("TIM1\r\n", 5);
				#endif
				
				transmitting = 1;
				cc1101TX = 0;
				
				cc1101_transmit();
			}
		}
		
		// If not trasmitting, put the system to sleep
		if (!transmitting)
		{
			system_sleep();
		}
	}
	
	return 0;
}

// Interrupt handlers
void EXTI0_IRQ(void) __interrupt(EXTI0)
{
	if (buttonTrigger == 0)
	{
		buttonTrigger = 1;
	}
	
	EXTI_SR1 = 0xFF; // Reset IRQs
}

void EXTI1_IRQ(void) __interrupt(EXTI1)
{
	if (buttonTrigger == 0)
	{
		buttonTrigger = 2;
	}
	
	EXTI_SR1 = 0xFF; // Reset IRQs
}

void EXTI2_IRQ(void) __interrupt(EXTI2)
{
	if (buttonTrigger == 0)
	{
		buttonTrigger = 3;
	}
	
	EXTI_SR1 = 0xFF; // Reset IRQs
}

void EXTI3_IRQ(void) __interrupt(EXTI3)
{
	if (buttonTrigger == 0)
	{
		buttonTrigger = 4;
	}
	
	EXTI_SR1 = 0xFF; // Reset IRQs
}

void TIM_OVERFLOW_IRQ(void) __interrupt(TIM1_IRQ)
{	
	tim1IRQ = 1;
	
	TIM1_SR1 = 0; // Clear IRQ flags
}