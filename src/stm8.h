#ifndef __STM8__
#define __STM8__

#define B0	(1u << 0)
#define B1	(1u << 1)
#define B2	(1u << 2)
#define B3	(1u << 3)
#define B4	(1u << 4)
#define B5	(1u << 5)
#define B6	(1u << 6)
#define B7	(1u << 7)

#define nop()			{__asm__("NOP\n");}

#define enableIRQ()		{__asm__("RIM\n");}
#define disableIRQ()	{__asm__("SIM\n");}

#define halt()			{__asm__("HALT\n");}

#define SYSCFG_RMPCR1	*((volatile uint8_t*)0x509E)

#define PA_ODR			*((volatile uint8_t*)0x5000)
#define PA_IDR			*((volatile uint8_t*)0x5001)
#define PA_DDR			*((volatile uint8_t*)0x5002)
#define PA_CR1			*((volatile uint8_t*)0x5003)
#define PA_CR2			*((volatile uint8_t*)0x5004)

#define PB_ODR			*((volatile uint8_t*)0x5005)
#define PB_IDR			*((volatile uint8_t*)0x5006)
#define PB_DDR			*((volatile uint8_t*)0x5007)
#define PB_CR1			*((volatile uint8_t*)0x5008)
#define PB_CR2			*((volatile uint8_t*)0x5009)

#define PC_ODR			*((volatile uint8_t*)0x500A)
#define PC_IDR			*((volatile uint8_t*)0x500B)
#define PC_DDR			*((volatile uint8_t*)0x500C)
#define PC_CR1			*((volatile uint8_t*)0x500D)
#define PC_CR2			*((volatile uint8_t*)0x500E)

#define PD_ODR			*((volatile uint8_t*)0x500F)
#define PD_IDR			*((volatile uint8_t*)0x5010)
#define PD_DDR			*((volatile uint8_t*)0x5011)
#define PD_CR1			*((volatile uint8_t*)0x5012)
#define PD_CR2			*((volatile uint8_t*)0x5013)

#define SPI_CR1			*((volatile uint8_t*)0x5200)
#define SPI_CR2			*((volatile uint8_t*)0x5201)
#define SPI_ICR			*((volatile uint8_t*)0x5202)
#define SPI_SR			*((volatile uint8_t*)0x5203)
#define SPI_DR			*((volatile uint8_t*)0x5204)
#define SPI_CRCPR		*((volatile uint8_t*)0x5205)
#define SPI_RXCRCR		*((volatile uint8_t*)0x5206)
#define SPI_TXCRCR		*((volatile uint8_t*)0x5207)

#define SPI_CR1_LSBFIRST 	(1 << 7)
#define SPI_CR1_SPE 		(1 << 6)
#define SPI_CR1_BR(br) 		((br) << 3)
#define SPI_CR1_MSTR 		(1 << 2)
#define SPI_CR1_CPOL 		(1 << 1)
#define SPI_CR1_CPHA 		(1 << 0)

#define SPI_CR2_BDM 		(1 << 7)
#define SPI_CR2_BDOE 		(1 << 6)
#define SPI_CR2_CRCEN 		(1 << 5)
#define SPI_CR2_CRCNEXT 	(1 << 4)
#define SPI_CR2_RXONLY 		(1 << 2)
#define SPI_CR2_SSM 		(1 << 1)
#define SPI_CR2_SSI 		(1 << 0)

#define SPI_ICR_TXIE 	(1 << 7)
#define SPI_ICR_RXIE 	(1 << 6)
#define SPI_ICR_ERRIE	(1 << 5)
#define SPI_ICR_WKIE 	(1 << 4)

#define SPI_SR_BSY		(1 << 7)
#define SPI_SR_OVR 		(1 << 6)
#define SPI_SR_MODF 	(1 << 5)
#define SPI_SR_CRCERR 	(1 << 4)
#define SPI_SR_WKUP 	(1 << 3)
#define SPI_SR_TXE 		(1 << 1)
#define SPI_SR_RxNE 	(1 << 0)

#define CLK_DIVR	*((volatile uint8_t*)0x50C0)
#define CLK_PCKENR1	*((volatile uint8_t*)0x50C3)
#define CLK_PCKENR2	*((volatile uint8_t*)0x50C4)

#define USART1_SR	*((volatile uint8_t*)0x5230)
#define USART1_DR	*((volatile uint8_t*)0x5231)
#define USART1_BRR1	*((volatile uint8_t*)0x5232)
#define USART1_BRR2	*((volatile uint8_t*)0x5233)
#define USART1_CR1	*((volatile uint8_t*)0x5234)
#define USART1_CR2	*((volatile uint8_t*)0x5235)
#define USART1_CR3	*((volatile uint8_t*)0x5236)
#define USART1_CR4	*((volatile uint8_t*)0x5237)
#define USART1_CR5	*((volatile uint8_t*)0x5238)
#define USART1_GTR	*((volatile uint8_t*)0x5239)
#define USART1_PSCR	*((volatile uint8_t*)0x523A)

#define EXTI0	8
#define EXTI1	9
#define EXTI2	10
#define EXTI3	11
#define TIM1_IRQ 23 // TIM1 Update/overflow/trigger

#define EXTI_CR1	*((volatile uint8_t*)0x50A0)
#define EXTI_CR3	*((volatile uint8_t*)0x50A2)
#define EXTI_CONF1	*((volatile uint8_t*)0x50A5)
#define EXTI_SR1 	*((volatile uint8_t*)0x50A3)

#define IRQ_FALLING_EDGE 2

#define TIM1_CR1	*((volatile uint8_t*)0x52B0)
#define TIM1_IER	*((volatile uint8_t*)0x52B5)
#define TIM1_SR1 	*((volatile uint8_t*)0x52B6)
#define TIM1_CNTRH	*((volatile uint8_t*)0x52BF)
#define TIM1_CNTRL	*((volatile uint8_t*)0x52C0)
#define TIM1_PSCRH 	*((volatile uint8_t*)0x52C1)
#define TIM1_PSCRL 	*((volatile uint8_t*)0x52C2)
#define TIM1_ARRH	*((volatile uint8_t*)0x52C3)
#define TIM1_ARRL	*((volatile uint8_t*)0x52C4)

#define TIM_CR1_APRE	B7
#define TIM_CR1_OPM		B3
#define TIM_CR1_URS		B2
#define TIM_CR1_CEN		B0

#define TIM_IER_UIE		B0

#endif