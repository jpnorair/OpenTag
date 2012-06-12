#ifndef BOARDDEFINES_H_
#define BOARDDEFINES_H_

#define P1_TXCT			BIT0
#define P1_SCIO			BIT1
#define P1_CSN 			BIT2
// No P1.3
#define P1_RX_UART		BIT4
#define P1_TX_UART		BIT5
#define P1_EN_UART		BIT6
#define P1_INT_BODY		BIT7

#define P2_SO  			BIT0
// No P2.1
#define P2_SI  			BIT2
#define P2_SCLK    		BIT3
#define P2_BUSY   		BIT4
#define P2_EN_UCC		BIT5


#define	SPI_SIMO		BIT1                       // P3.1 Output:	Output to Slave 
#define	SPI_SOMI		BIT2                       // P3.2 Input:	Input from Slave
#define	SPI_CLK 		BIT3                       // P3.3 Output:	SPI clock


#define OSC_CLOCK				40000			//4MHz XTAL
#define OSC_CLOCK_MULTIPLY		4.000			//4MHz XTAL



#endif /*BOARDDEFINES_H_*/
