//(c)2010 by Texas Instruments Incorporated, All Rights Reserved.
/*  
 * ======== main.c ========
 * LED Control Demo:
 *
 * This USB demo example is to be used with a PC application (e.g. HyperTerminal)
 * This demo application is used to control the operation of the LED at P1.0
 *
 * Typing the following pharses in the HyperTerminal Window does the following
 * 1. "LED ON" Turns on the LED and returns "LED is ON" phrase to PC
 * 2. "LED OFF" Turns off the LED and returns "LED is OFF" back to HyperTerminal
 * 3. "LED TOGGLE - SLOW" Turns on the timer used to toggle LED with a large
 *   period and returns "LED is toggling slowly" phrase back to HyperTerminal
 * 4. "LED TOGGLE - FAST" Turns on the timer used to toggle LED with a smaller
 *   period and returns "LED is toggling fast" phrase back to HyperTerminal
 *
 +----------------------------------------------------------------------------+
 * Please refer to the MSP430 USB API Stack Programmer's Guide,located
 * in the root directory of this installation for more details.
 * ---------------------------------------------------------------------------*/
#include <intrinsics.h>
#include <string.h>

#include "USB_config/descriptors.h"

#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/types.h"               //Basic Type declarations
#include "USB_API/USB_Common/usb.h"                 //USB-specific functions

#include "F5xx_F6xx_Core_Lib/HAL_UCS.h"
#include "F5xx_F6xx_Core_Lib/HAL_PMM.h"

#include "USB_API/USB_CDC_API/UsbCdc.h"
#include "usbConstructs.h"

//Function declarations
VOID Init_Ports (VOID);
VOID Init_Clock (VOID);
VOID Init_TimerA1 (VOID);
BYTE retInString (char* string);

//Global flags set by events
volatile BYTE bCDCDataReceived_event = FALSE;   //Indicates data has been received without an open rcv operation

#define MAX_STR_LENGTH 64
char wholeString[MAX_STR_LENGTH] = "";          //The entire input string from the last 'return'
unsigned int SlowToggle_Period = 20000 - 1;
unsigned int FastToggle_Period = 1000 - 1;

/*  
 * ======== main ========
 */
VOID main (VOID)
{
    WDTCTL = WDTPW + WDTHOLD;                                   //Stop watchdog timer

    Init_Ports();                                               //Init ports (do first ports because clocks do change ports)
    SetVCore(3);
    Init_Clock();                                               //Init clocks
	
    USB_init();                 //Init USB

    Init_TimerA1();
	
    //Enable various USB event handling routines
    USB_setEnabledEvents(
        kUSB_VbusOnEvent + kUSB_VbusOffEvent + kUSB_receiveCompletedEvent
        + kUSB_dataReceivedEvent + kUSB_UsbSuspendEvent + kUSB_UsbResumeEvent +
        kUSB_UsbResetEvent);

    //See if we're already attached physically to USB, and if so, connect to it
    //Normally applications don't invoke the event handlers, but this is an exception.
    if (USB_connectionInfo() & kUSB_vbusPresent){
        USB_handleVbusOnEvent();
    }

	__enable_interrupt();                           //Enable interrupts globally
    while (1)
    {
        BYTE i;
        //Check the USB state and directly main loop accordingly
        switch (USB_connectionState())
        {
            case ST_USB_DISCONNECTED:
                __bis_SR_register(LPM3_bits + GIE);                                 //Enter LPM3 w/ interrupts enabled
                _NOP();                                                             //For Debugger
                break;

            case ST_USB_CONNECTED_NO_ENUM:
                break;

            case ST_ENUM_ACTIVE:
                __bis_SR_register(LPM0_bits + GIE);                                 //Enter LPM0 (can't do LPM3 when active)
                _NOP();                                                             //For Debugger

                                                                                    //Exit LPM on USB receive and perform a receive
                                                                                    //operation
                if (bCDCDataReceived_event){                                        //Some data is in the buffer; begin receiving a
                                                                                    //command
                    char pieceOfString[MAX_STR_LENGTH] = "";                        //Holds the new addition to the string
                    char outString[MAX_STR_LENGTH] = "";                            //Holds the outgoing string

                                                                                    //Add bytes in USB buffer to theCommand
                    cdcReceiveDataInBuffer((BYTE*)pieceOfString,
                        MAX_STR_LENGTH,
                        CDC0_INTFNUM);                                                         //Get the next piece of the string
                    strcat(wholeString,pieceOfString);
                    cdcSendDataInBackground((BYTE*)pieceOfString,
                        strlen(pieceOfString),CDC0_INTFNUM,0);                      //Echoes back the characters received (needed
                                                                                    //for Hyperterm)

                    if (retInString(wholeString)){                                  //Has the user pressed return yet?
                        if (!(strcmp(wholeString, "LED ON"))){                      //Compare to string #1, and respond
                            TA1CTL &= ~MC_1;                                        //Turn off Timer
                            P1OUT |= BIT0;                                          //Turn on LED P1.0
                            strcpy(outString,"\r\nLED is ON\r\n\r\n");              //Prepare the outgoing string
                            cdcSendDataInBackground((BYTE*)outString,
                                strlen(outString),CDC0_INTFNUM,0);                  //Send the response over USB
                        } else if (!(strcmp(wholeString, "LED OFF"))){              //Compare to string #2, and respond
                            TA1CTL &= ~MC_1;                                        //Turn off Timer
                            P1OUT &= ~BIT0;                                         //Turn off LED P1.0
                            strcpy(outString,"\r\nLED is OFF\r\n\r\n");             //Prepare the outgoing string
                            cdcSendDataInBackground((BYTE*)outString,
                                strlen(outString),CDC0_INTFNUM,0);                  //Send the response over USB
                        } else if (!(strcmp(wholeString,
                                         "LED TOGGLE - SLOW"))){                    //Compare to string #3, and respond
                            TA1CTL &= ~MC_1;                                        //Turn off Timer
                            TA1CCR0 = SlowToggle_Period;                            //Set Timer Period for slow LED toggle
                            TA1CTL |= MC_1;                                         //Start Timer
                            strcpy(outString,
                                "\r\nLED is toggling slowly\r\n\r\n");              //Prepare the outgoing string
                            cdcSendDataInBackground((BYTE*)outString,
                                strlen(outString),CDC0_INTFNUM,0);                  //Send the response over USB
                        } else if (!(strcmp(wholeString,
                                         "LED TOGGLE - FAST"))){                    //Compare to string #4, and respond
                            TA1CTL &= ~MC_1;                                        //Turn off Timer
                            TA1CCR0 = FastToggle_Period;                            //Set Timer Period for fast LED toggle
                            TA1CTL |= MC_1;
                            strcpy(outString,"\r\nLED is toggling fast\r\n\r\n");   //Prepare the outgoing string
                            cdcSendDataInBackground((BYTE*)outString,
                                strlen(outString),CDC0_INTFNUM,0);                  //Send the response over USB
                        } else {                                                    //Handle other
                            strcpy(outString,"\r\nNo such command!\r\n\r\n");       //Prepare the outgoing string
                            cdcSendDataInBackground((BYTE*)outString,
                                strlen(outString),CDC0_INTFNUM,0);                  //Send the response over USB
                        }
                        for (i = 0; i < MAX_STR_LENGTH; i++){                       //Clear the string in preparation for the next
                                                                                    //one
                            wholeString[i] = 0x00;
                        }
                    }
                    bCDCDataReceived_event = FALSE;
                }
                break;

            case ST_ENUM_SUSPENDED:
                P1OUT &= ~BIT0;                                                     //When suspended, turn off LED
                __bis_SR_register(LPM3_bits + GIE);                                 //Enter LPM3 w/ interrupts
                _NOP();
                break;

            case ST_ENUM_IN_PROGRESS:
                break;

            case ST_NOENUM_SUSPENDED:
                P1OUT &= ~BIT0;
                __bis_SR_register(LPM3_bits + GIE);
                _NOP();
                break;

            case ST_ERROR:
                _NOP();
                break;

            default:;
        }
    }  //while(1)
} //main()

/*  
 * ======== Init_Clock ========
 */
VOID Init_Clock (VOID)
{
    //Initialization of clock module
    if (USB_PLL_XT == 2){
		#if defined (__MSP430F552x) || defined (__MSP430F550x)
			P5SEL |= 0x0C;                                      //enable XT2 pins for F5529
		#elif defined (__MSP430F563x_F663x)
			P7SEL |= 0x0C;
		#endif

        //use REFO for FLL and ACLK
        UCSCTL3 = (UCSCTL3 & ~(SELREF_7)) | (SELREF__REFOCLK);
        UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | (SELA__REFOCLK);

        //MCLK will be driven by the FLL (not by XT2), referenced to the REFO
        Init_FLL_Settle(USB_MCLK_FREQ / 1000, USB_MCLK_FREQ / 32768);   //Start the FLL, at the freq indicated by the config
                                                                        //constant USB_MCLK_FREQ
        XT2_Start(XT2DRIVE_0);                                          //Start the "USB crystal"
    } 
	else {
		#if defined (__MSP430F552x) || defined (__MSP430F550x)
			P5SEL |= 0x10;                                      //enable XT1 pins
		#endif
        //Use the REFO oscillator to source the FLL and ACLK
        UCSCTL3 = SELREF__REFOCLK;
        UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | (SELA__REFOCLK);

        //MCLK will be driven by the FLL (not by XT2), referenced to the REFO
        Init_FLL_Settle(USB_MCLK_FREQ / 1000, USB_MCLK_FREQ / 32768);   //set FLL (DCOCLK)

        XT1_Start(XT1DRIVE_0);                                          //Start the "USB crystal"
    }
}

/*  
 * ======== Init_Ports ========
 */
VOID Init_Ports (VOID)
{
    //Initialization of ports (all unused pins as outputs with low-level
    P1OUT = 0x00;
    P1DIR = 0xFF;
	P2OUT = 0x00;
    P2DIR = 0xFF;
    P3OUT = 0x00;
    P3DIR = 0xFF;
    P4OUT = 0x00;
    P4DIR = 0xFF;
    P5OUT = 0x00;
    P5DIR = 0xFF;
    P6OUT = 0x00;
    P6DIR = 0xFF;
	P7OUT = 0x00;
    P7DIR = 0xFF;
    P8OUT = 0x00;
    P8DIR = 0xFF;
    #if defined (__MSP430F563x_F663x)
		P9OUT = 0x00;
		P9DIR = 0xFF;
    #endif
}

/*  
 * ======== UNMI_ISR ========
 */
#pragma vector = UNMI_VECTOR
__interrupt VOID UNMI_ISR (VOID)
{
    switch (__even_in_range(SYSUNIV, SYSUNIV_BUSIFG))
    {
        case SYSUNIV_NONE:
            __no_operation();
            break;
        case SYSUNIV_NMIIFG:
            __no_operation();
            break;
        case SYSUNIV_OFIFG:
            UCSCTL7 &= ~(DCOFFG + XT1LFOFFG + XT2OFFG); //Clear OSC flaut Flags fault flags
            SFRIFG1 &= ~OFIFG;                          //Clear OFIFG fault flag
            break;
        case SYSUNIV_ACCVIFG:
            __no_operation();
            break;
        case SYSUNIV_BUSIFG:

                                                                //If bus error occured - the cleaning of flag and re-initializing of
                                                                //USB is required.
            SYSBERRIV = 0;                                      //clear bus error flag
            USB_disable();                                      //Disable
    }
}

/*  
 * ======== Init_TimerA1 ========
 */
VOID Init_TimerA1 (VOID)
{
    TA1CCTL0 = CCIE;                                            //CCR0 interrupt enabled
    TA1CTL = TASSEL_1 + TACLR;                                  //ACLK, clear TAR
}

/*  
 * ======== retInString ========
 */
//This function returns true if there's an 0x0D character in the string; and if so,
//it trims the 0x0D and anything that had followed it.
BYTE retInString (char* string)
{
    BYTE retPos = 0,i,len;
    char tempStr[MAX_STR_LENGTH] = "";

    strncpy(tempStr,string,strlen(string));                     //Make a copy of the string
    len = strlen(tempStr);
    while ((tempStr[retPos] != 0x0A) && (tempStr[retPos] != 0x0D) &&
           (retPos++ < len)) ;                                  //Find 0x0D; if not found, retPos ends up at len

    if ((retPos < len) && (tempStr[retPos] == 0x0D)){           //If 0x0D was actually found...
        for (i = 0; i < MAX_STR_LENGTH; i++){                   //Empty the buffer
            string[i] = 0x00;
        }
        strncpy(string,tempStr,retPos);                         //...trim the input string to just before 0x0D
        return ( TRUE) ;                                        //...and tell the calling function that we did so
    } else if ((retPos < len) && (tempStr[retPos] == 0x0A)){    //If 0x0D was actually found...
        for (i = 0; i < MAX_STR_LENGTH; i++){                   //Empty the buffer
            string[i] = 0x00;
        }
        strncpy(string,tempStr,retPos);                         //...trim the input string to just before 0x0D
        return ( TRUE) ;                                        //...and tell the calling function that we did so
    } else if (tempStr[retPos] == 0x0D){
        for (i = 0; i < MAX_STR_LENGTH; i++){                   //Empty the buffer
            string[i] = 0x00;
        }
        strncpy(string,tempStr,retPos);                         //...trim the input string to just before 0x0D
        return ( TRUE) ;                                        //...and tell the calling function that we did so
    } else if (retPos < len){
        for (i = 0; i < MAX_STR_LENGTH; i++){                   //Empty the buffer
            string[i] = 0x00;
        }
        strncpy(string,tempStr,retPos);                         //...trim the input string to just before 0x0D
        return ( TRUE) ;                                        //...and tell the calling function that we did so
    }

    return ( FALSE) ;                                           //Otherwise, it wasn't found
}

/*  
 * ======== TIMER1_A0_ISR ========
 */
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR (void)
{
    P1OUT ^= BIT0;                                              //Toggle LED P1.0
}

