//------------------------------------------------------------------------------
//
//  interrupt.c
//  
//  All interrupt anmd timer functions used at the Demo Reader
//
//
//  Juergen Austen,  Texas Instruments, Inc
//  December 2008
//
//  Revision history:
//  0.1     27-Nov      First implementation
//
//
//------------------------------------------------------------------------------
#include  <msp430x12x.h>
#include  <defines.h>
#include  <Board defines.h>

unsigned char   ucWaitForDebounce,ucWaitforTimeout,ucTest;
unsigned int    uiTimeOverflows,uiWaitOverflows;

struct stTimerCtrl    TimerCtrl;

//******************************************************************************
// 
//  ACLK = MCLK = TACLK = HF XTAL
//  As coded with TACLK= HF XTAL and assuming HF XTAL= 4MHz, frequencies are:
//  Devider set to 4 --> Running frequency: 1 MHz
//
//******************************************************************************

#include  <msp430x12x2.h>

void Timer_Init(void)
{
  // Timer A is configured for continous up mode with a time base of 1us/1MHz 
  CCR0  = 0;
  CCR1  = 0;
  CCR2  = 0;
  TACTL = TASSEL_1 + ID_2 + MC_2 + TAIE;    // ACLK, contmode, interrupt enabled

  _BIS_SR(GIE);                             // Enable interrupt
}

/**************************** Port1 *********************************************/
#pragma vector= PORT1_VECTOR                    // used to detect USB transmission
__interrupt void Port1_isr(void)
{
    if (P1IFG & P1_RX_UART)                     // interrupt coming from the USB input P1_RX_UART
        {   
            P1IE &= ~P1_RX_UART;                // Interrupt disable - rest of the byte is read with TimerA timing
            _BIS_SR(GIE);                       // GIE enable to allow interrupt nesting
            ReceiveByteUSB();
            TimerCtrl.b2USB_DATA = USB_DATA_STARTED;
            P1IFG &= ~P1_RX_UART;               // clear flag
            P1IE  |=  P1_RX_UART;               // Interrupt enable -  for the next byte
        };
        P1IFG = 0x00;                           // clear all flags
}

// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A0 (void)
{
  CCTL0 &= ~CCIFG;
  if (TimerCtrl.uiCCR0_Count_1ms)
        {
          TimerCtrl.uiCCR0_Count_1ms--;
          CCR0 = TAR + 997;
        }
  else
      {
        TimerCtrl.bCCR0_TimeOut = TRUE;
        CCTL0 &= ~CCIE;
      }
  
}

// Timer_A3 Interrupt Vector (TAIV) handler
#pragma vector=TIMERA1_VECTOR
__interrupt void Timer_A1(void)
{
  switch( TAIV )
  {
  case  2: //CCR1
           CCTL1 = 0;                       // disable interrupt and clear flag
           TimerCtrl.bCCR1_TimeOut = TRUE;
           break;
  case  4: 
           TimerCtrl.bCCR2_TimeOut = TRUE;
           P2DIR |= P2_SI;
           //P2OUT ^= P2_SI;  //toggle SI
           if (TimerCtrl.b2USB_DATA == USB_DATA_STARTED)
                {   
                TimerCtrl.b2USB_DATA = USB_DATA_RECEIVED;
                }
           CCTL2 &= ~CCIE;
           break;
  case 10: 
           P1OUT ^= BIT2;                   // Timer_A3 overflow
           break;
 }
}


void TimerAWait_CCR1(unsigned int uiDurationUSEC, unsigned char ucStop)
{ //  Uses the TimerA for fast timing.
  //  Max. duration is 2^16 * 1us ~= 65 ms
  //  CCR1 is used for this function
    
    CCR1  = TAR + uiDurationUSEC - 24;      // add us to current counter
    TimerCtrl.bCCR1_TimeOut = FALSE;
    CCTL1 = CCIE;                           // enable interrupt and clear flag
    
    if (ucStop == HALT)
        while (TimerCtrl.bCCR1_TimeOut);                                                    // Wait for TimerA ISR to set the flag                              
}

void TimerAWait_CCR2(unsigned int uiDurationUSEC, unsigned char ucStop)
{ //  Uses the TimerA for fast timing.
  //  Max. duration is 2^16 * 1us ~= 65 ms
  //  CCR2 is used for this function
    
    CCR2  = TAR + uiDurationUSEC - 24;      // add us to current counter
    TimerCtrl.bCCR2_TimeOut = FALSE;
    CCTL2 = CCIE;                           // enable interrupt and clear flag
    
    if (ucStop == HALT)
        while (TimerCtrl.bCCR2_TimeOut);                                                    // Wait for TimerA ISR to set the flag                              
}


//------------------------------------------------------------------------------
void SetOutput(unsigned char volatile *usPortOut, unsigned char ucBit, unsigned char ucState, unsigned int uiDuration, unsigned char ucDurationUnit, unsigned char ucHaltProgram)
{
    //  Example Function call:  SetOutput(&P2OUT, BIT3, HIGH, 60, MSEC, HALT);
    //  Generic function allowing to set one or multiple bits (ucBit) of any output port (usPortOut)
    //  to the desired state (bState) for a chosen duration (usDuration, bDurationUnit).
    //  Examples: Waiting for a certain time, starting up the TMS3705, Power Bursts, data transmission.
    //  The function can halt by running a loop until TimerA flag appears (ucHaltProgram == HALT),
    //  or return directly so that other code can be executed while TimerB is running (in that case the surrounding
    //  code has to check for TimerCtrl.bContinue == true).
    //  Maximum duration is 2^16 * 1us = 65.5s.
    //  Minimum duration is 50us  @ 4MHz crystal !
    
    if ((uiDuration == 0) || (uiDuration < 30 && ucDurationUnit == USEC))
        return;

    if ( (ucDurationUnit == MSEC) && (uiDuration == 1))
    {
      TimerCtrl.bCCR0_TimeOut    = FALSE;
      TimerCtrl.uiCCR0_Count_1ms = 0;
      CCR0  = TAR + 997;                                    // duration is in us (1000us = 1ms)
      CCTL0 = CCIE;                                         // CCR0 interrupt enabled
    }
    if (ucDurationUnit == MSEC)
    {
      TimerCtrl.bCCR0_TimeOut    = FALSE;
      TimerCtrl.uiCCR0_Count_1ms = uiDuration - 1 ;         //uiCCR0_Count_1ms = usDuration - 1 default
                                                            //TBR = 0xFFFF-(1000*usDuration);       
      CCR0  = TAR + 997;
      CCTL0 = CCIE;                                         // CCR0 interrupt enabled
    }
    else
    {
      TimerCtrl.bCCR0_TimeOut    = FALSE;
      TimerCtrl.uiCCR0_Count_1ms = 0;
      if (uiDuration > 49)
        CCR0  = TAR + uiDuration - 38;                      // duration is in microseconds - with correction factor
      else          
        CCR0  = TAR + uiDuration - 20;                      // duration is in microseconds - with correction factor
      CCTL0 = CCIE;                                         // CCR0 interrupt enabled
    }

    if (ucState == HIGH)
    {
      *usPortOut |= ucBit;                              // set chosen bit(s) to high
    }
    else
    {   
      *usPortOut &= ~ucBit;                             // reset chosen bit(s) to low
    }
    if (ucHaltProgram == HALT)
    {
      while (TimerCtrl.bCCR0_TimeOut);              // Wait for TimerB ISR to set the flag  timeout
      TimerCtrl.bCCR0_TimeOut = FALSE;
    }
  TimerCtrl.bContinue = FALSE;      
  CCTL0 &= ~CCIE;   
}










/*
void Timer_Init(unsigned int uiTime_ms)
{
  //SMCLK = typ. 3.2 MHz  --> One cycle: 312.5 ns
  TAR   = 0;
  CCTL0 = CCIE;                             // CCR0 interrupt enabled
  if (uiTime_ms > 10)
    {
      uiTimeOverflows = uiTime_ms / 10;
      uiTime_ms       = uiTime_ms - (uiTimeOverflows*10);
      uiTimeOverflows++;
    }
  CCR0  = uiTime_ms * 3200;                                                     // wait typ. 0-9 ms
  if (CCR0 < 1000)
    CCR0 = 200;                                                                 // if 0 additional ms then 0.1ms header
  TACTL = TASSEL_2 + MC_2;                                                      // SMCLK, continuousmode    
  ucWaitForDebounce = TRUE;                                                     // debouncing active !
}


// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
  ucTest++;
  if (ucWaitforTimeout)
  {
    TAR = 0;
    if (uiWaitOverflows > 1)
      {
       CCR0  = OSC_CLOCK;                                                       // adjust to 10ms timing
       TACTL = TASSEL_2 + MC_2;                                                 // SMCLK, continuousmode
       uiWaitOverflows--;
      }
    else
      {
      _BIC_SR_IRQ(LPM1_bits);                   // Exit Low Power Mode 1 after this interrupt function
      TACTL  = 0;                               // Stop timer
      ucWaitforTimeout = FALSE;                 // debouncing done !
      }
  }
  if (ucWaitForDebounce)
  {
    TAR = 0;
    if (uiTimeOverflows > 1)
      {
       CCR0  = OSC_CLOCK;                                                            // adjust to 10ms timing
       TACTL = TASSEL_2 + MC_2;                                                 // SMCLK, continuousmode
       uiTimeOverflows--;
      }
    else
      {
      _BIC_SR_IRQ(LPM1_bits);                   // Exit Low Power Mode 1 after this interrupt function
      TACTL   = 0;                               // Stop timer
      TACCTL0 = 0;
      ucWaitForDebounce = FALSE;                // debouncing done !
      }
  }
}


void Timer_Wait(unsigned int uiDurationUSEC, unsigned char ucStop)
{
  //TACTL = 0;
  if (uiDurationUSEC > 10000)                                                   // wait more than 10ms
  {
    uiWaitOverflows = uiDurationUSEC / 10000;
    uiDurationUSEC  = uiDurationUSEC - (uiWaitOverflows*10000);
    uiWaitOverflows++;
  }
  else
  {
     uiWaitOverflows = 0;
  }
  CCR0  = uiDurationUSEC * OSC_CLOCK_MULTIPLY;                                                 // wait typ. 0-9 ms

  TAR   = 0;
  CCTL0 = CCIE;                             // CCR0 interrupt enabled
  TACTL = TASSEL_2 + MC_2;                  // SMCLK, continuousmode
  ucWaitforTimeout = TRUE;

  if (ucStop)
  {
    while (ucWaitforTimeout)
       _BIS_SR(LPM1_bits);                                                      // Enter Low Power Mode 1 , LPM3 could be used with external Oscillator
  }
}
*/

