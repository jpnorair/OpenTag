//------------------------------------------------------------------------------
//
//  main.c
// 	
//	Main part of Demo Reader ADR2
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
#include  <defines.h>
#include  <Board defines.h>

extern struct   stPWM				PWMtimes;
extern struct 	stBLC 				BLCtimes; 
extern struct 	stTimerCtrl    		TimerCtrl;
extern struct	stASCIIbuffer		USB_Buffer;
extern union 	unCMD				command1;
extern union	unCMD2 				command2;

void ReadPage3(void)
{
// Read Page 3 command   

    PWMtimes.usToffL = 170;
    PWMtimes.usTonL  = 330;
    PWMtimes.usToffH = 480;
    PWMtimes.usTonH  = 520;
    
    BLCtimes.usToff  = 170;
    BLCtimes.usTonL  = 230;
    BLCtimes.usTonH  = 350;
    BLCtimes.usTonSC = 600;
    
    USB_Buffer.ucData[0] = 0x01;      
    USB_Buffer.ucData[1] = 0x06;        
    //USB_Buffer.ucData[2] = 0x04; 	//PWM        
    USB_Buffer.ucData[2] = 0x06; 	//BLC
    USB_Buffer.ucData[3] = 0x32;        
    USB_Buffer.ucData[4] = 0x08;        
    USB_Buffer.ucData[5] = 0x0C;        
    USB_Buffer.ucData[6] = 0x00;        
    USB_Buffer.ucData[7] = 0x0A;        
    //USB_Buffer.ucData[8] = 0x3E;	//PWM
    USB_Buffer.ucData[8] = 0x3C;	//BLC
           
    USB_Buffer.ucLength   = 9;
    TimerCtrl.b2USB_DATA  = USB_DATA_RECEIVED;
}

void main(void)
{ unsigned char i;
	
  Init();
  Timer_Init();
  TimerCtrl.b2USB_DATA 	 = NO_USB_DATA;
  TimerCtrl.bTriggerRead = FALSE;
  
  //P2OUT |= P2_EN_UCC;					// enable UCC Amplifier
  
  for (i=0; i<RX_BUF_SIZE;i++)
  	USB_Buffer.ucData[i] = 0;
  USB_Buffer.ucLength = 0;
  while (1)
  {
  	if (P1IN & P1_INT_BODY)
	  	{
	  	 TimerAWait_CCR2(25000, HALT);
	  	 ReadPage3();
	  	}
  	
    if (TimerCtrl.b2USB_DATA == USB_DATA_RECEIVED)			// new data from USB
	    {
        switch (DownlinkUSB())								// packet header check
        {
          case 0:	// Normal mode : includes transponder communication
                    NormalModeUSB();						// analyze packet payload                    
                    
                    #ifdef UHF_CONNECTED
                        if (!command1.field.LF_UHF)
                             InitUHF_PEPS_Receiver();
                    #endif    
                     
                    DownlinkLF();							// transmit over LF
                    if (command1.field.LF_UHF)  
                    {
                       if (!command2.field.KeepTXon) 	 
                       		UplinkLF();				// get LF transponder response or error code
                       UplinkUSB();				// Send response
                    }

                    else
                    {
    					/*                 
	                     Start_TBCCR3_TimeOut(250);                                 // Timeout after 100ms
	                     while (!GDO_2 && !TimerCtrl.bTBCCR3_TimeOut)
	                      {
	                        if (GDO_2)                                                                  // GDO_2 asserts on CRC ok
	                          { P4OUT = 0xFF;
	                            Stop_TBCCR3_TimeOut();
	                            UHF_Protocol_Manager(&ucChallenge[0],&ucSignature[0]);
	                            TimerCtrl.bTBCCR3_TimeOut = true;                   // abort while loop
	                          }
	                      }
	                     #ifdef UHF_CONNECTED
	                        InitUHF_RKE_Receiver();
	                     #endif
	                     */
                    }
                    //GetNewChallengeResponse(&ucChallenge[0],&ucSignature[0]);
                    
                    break;

          case 1:		// Setup mode : straight acknowledge via USB
                    SetupModeUSB();
                    UplinkUSB();								//  output response to host PC
                    break;

         default:	;

        }
      TimerCtrl.b2USB_DATA = NO_USB_DATA;
      USB_Buffer.ucLength = 0;	
	  }
  }
}



