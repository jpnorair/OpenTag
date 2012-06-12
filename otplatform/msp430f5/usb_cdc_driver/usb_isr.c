//(c)2009 by Texas Instruments Incorporated, All Rights Reserved.
/*----------------------------------------------------------------------------+
 |                                                                             |
 |                              Texas Instruments                              |
 |                                                                             |
 |                          MSP430 USB-Example (CDC/HID Driver)                |
 |                                                                             |
 +-----------------------------------------------------------------------------+
 |  Source: UsbIsr.h, File Version 1.00 2009/12/03                             |
 |  Author: RSTO                                                               |
 |                                                                             |
 |  WHO          WHEN         WHAT                                             |
 |  ---          ----------   ------------------------------------------------ |
 |  RSTO         2008/09/03   born                                             |
 |  RSTO         2008/12/23   enhancements of CDC API                          |
 +----------------------------------------------------------------------------*/

#include "usb_cdc_driver/usb_descriptors.h"
#include "usb_cdc_driver/defMSP430USB.h"
#include "usb_cdc_driver/usb_isr.h"
#include "usb_cdc_driver/usb_main.h"
#include "usb_cdc_driver/usb_cdc_backend.h"



#pragma vector=USB_UBM_VECTOR
__interrupt VOID iUsbInterruptHandler(VOID) {
    BYTE bWakeUp = FALSE;
    //Check if the setup interrupt is pending.
    //We need to check it before other interrupts,
    //to work around that the Setup Int has lower priority then Input Endpoint 0
    if (USBIFG & SETUPIFG) {
        bWakeUp = SetupPacketInterruptHandler();  
        USBIFG &= ~SETUPIFG;    // clear the interrupt bit
    }
    
    switch (__even_in_range(USBVECINT & 0x3f, USBVECINT_OUTPUT_ENDPOINT7)) {
        case USBVECINT_NONE:        break;

        case USBVECINT_PWR_DROP:    __no_operation();
                                    break;
                                    
        case USBVECINT_PLL_LOCK:    break;
        case USBVECINT_PLL_SIGNAL:  break;
        
#		if (USBEVT_MASK & USBEVT_CLOCKFAULT)
        case USBVECINT_PLL_RANGE:   bWakeUp = USB_handleClockEvent();
                                    break;
#		endif
        
        case USBVECINT_PWR_VBUSOn:  PWRVBUSonHandler();
#									if (USBEVT_MASK & USBEVT_VBUSON)
                                    	bWakeUp = USB_handleVbusOnEvent();
#									endif
                                    break;
        
        case USBVECINT_PWR_VBUSOff: PWRVBUSoffHandler();
#									if (USBEVT_MASK & USBEVT_VBUSOFF)
                                        bWakeUp = USB_handleVbusOffEvent();
#									endif
                                    break;
        
        case USBVECINT_USB_TIMESTAMP: break;
        
        case USBVECINT_INPUT_ENDPOINT0: IEP0InterruptHandler();
                                        break;
                                        
        case USBVECINT_OUTPUT_ENDPOINT0: OEP0InterruptHandler();
                                        break;
        
        case USBVECINT_RSTR:        USB_reset();
#									if (USBEVT_MASK & USBEVT_RESET)
                                        bWakeUp = USB_handleResetEvent();
#									endif
                                    break;
        
        case USBVECINT_SUSR:        USB_suspend();
#									if (USBEVT_MASK & USBEVT_SUSPEND)
                                        bWakeUp = USB_handleSuspendEvent();
#									endif
                                    break;
        
        case USBVECINT_RESR:        USB_resume();
#									if (USBEVT_MASK & USBEVT_RESUME)
                                        USB_handleResumeEvent();
#									endif
                                    bWakeUp = TRUE;		//Always wake on resume
                                    break;
        
        case USBVECINT_SETUP_PACKET_RECEIVED:   // NAK both IEP and OEP enpoints
                                    tEndPoint0DescriptorBlock.bIEPBCNT = EPBCNT_NAK;
                                    tEndPoint0DescriptorBlock.bOEPBCNT = EPBCNT_NAK;
                                    SetupPacketInterruptHandler();
                                    break;
        
        case USBVECINT_STPOW_PACKET_RECEIVED: break;
        
        case USBVECINT_INPUT_ENDPOINT1: break;

        case USBVECINT_INPUT_ENDPOINT2: bWakeUp = CdcToHostFromBuffer(CDC0_INTFNUM);
        	                            break;
                                    
        case USBVECINT_INPUT_ENDPOINT3: break;
        case USBVECINT_INPUT_ENDPOINT4: break;
        case USBVECINT_INPUT_ENDPOINT5: break;
        case USBVECINT_INPUT_ENDPOINT6: break;
        case USBVECINT_INPUT_ENDPOINT7: break;

        case USBVECINT_OUTPUT_ENDPOINT1: break;

        case USBVECINT_OUTPUT_ENDPOINT2: bWakeUp = CdcToBufferFromHost(CDC0_INTFNUM);
            						    break;
            
        case USBVECINT_OUTPUT_ENDPOINT3: break;
        case USBVECINT_OUTPUT_ENDPOINT4: break;
        case USBVECINT_OUTPUT_ENDPOINT5: break;
        case USBVECINT_OUTPUT_ENDPOINT6: break;
        case USBVECINT_OUTPUT_ENDPOINT7: break;
        default: break;
    }
    
    if (bWakeUp) {
    	 __bic_SR_register_on_exit(LPM3_bits);   // Exit LPM0-3
    	 __no_operation();                       // Required for debugger
    }
}

