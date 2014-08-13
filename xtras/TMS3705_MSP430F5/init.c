//------------------------------------------------------------------------------
//
//  init.c
// 	
//	Initialization of the CED80 and peripherials
//
//
// 	Juergen Austen,  Texas Instruments, Inc
//  December 2008
//
//	Revision history:
//	0.1 	27-Nov		First implementation
//
//
//------------------------------------------------------------------------------
#include  <msp430x12x.h>
#include  <Board defines.h>

void PortInit(void)
{
  //--------------------------------------- Port 1 -----------------------------
  //Error P1DIR = P1_TXCT + P1_CSN + P1_TX_UART + P1_EN_UART + P1_INT_BODY;
  P1DIR = P1_TXCT + P1_TX_UART + P1_EN_UART + P1_INT_BODY;
  P1OUT = P1_CSN + P1_EN_UART;
  P1IES = P1_RX_UART;
  P1IE  = P1_RX_UART;
  P1IFG = 0;
  
	//--------------------------------------- Port 2 -----------------------------
  P2DIR = P2_SO + P2_SCLK + P2_EN_UCC;
  P2OUT = 0;

		
	//--------------------------------------- Port 3 -----------------------------
// 	P3DIR = MCU_SPI_STE + MCU_SPI_SIMO + MCU_SPI_CLK + MCU_LIN_TXD;
//	P3OUT = MCU_SPI_STE + MCU_SPI_SIMO + MCU_LIN_TXD;
//	P3SEL = MCU_LIN_TXD + MCU_LIN_RXD;

}



//------------------------------------------------------------------------------
void ClockInit(void)
{ unsigned char i;
		
  // Basic Clock Control Register1
  //  -Turn on external oscillator (XT2).
  //  -Select high-frequency mode for oscillator (LFXT1CLK), external crystal is used
  //  -Set Auxiliary Clock Divider to 1
  BCSCTL1 = XTS + DIVA_0;		
  do
	  {
	  IFG1 &= ~OFIFG;                       // Clear OSCFault flag
	  for (i = 0xFF; i > 0; i--);           // Time for flag to set
  }
  while ((IFG1 & OFIFG));                   // OSCFault flag still set?    
  BCSCTL2 = SELM_3 + SELS;					// MCLK= LFXT1 (safe) 																													
}



//------------------------------------------------------------------------------
void Init(void)
{
	WDTCTL = WDTPW + WDTHOLD;       	// Stop watchdog timer
	PortInit();
	ClockInit();
	//TimerInit();
	LF_Init();
	_BIS_SR(GIE);					    // intrinsic function to set the Global Interrupt Enable bit in Status Register
}

