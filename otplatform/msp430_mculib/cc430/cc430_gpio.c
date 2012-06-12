/* Copyright 2009 JP Norair
  *
  * Licensed under the OpenTag License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  * http://www.indigresso.com/wiki/doku.php?id=opentag:license_1_0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  */
/** @file       /Platforms/CC430/cc430_lib/cc430_gpio.c
  * @author     JP Norair
  * @version    V1.0
  * @date       1 Dec 2009
  * @brief      Library resources for GPIO peripheral(s)
  * @ingroup    CC430 Library
  *
  * "GPIO" includes the "Digital I/O" Peripheral and "Port Mapping Controller"
  * of the CC430.
  ******************************************************************************
  */



#include "cc430_conf.h"
#include "cc430_lib.h"




/*
void GPIO_Init(void* GPIOx, GPIO_InitType* GPIO_InitStruct) {

    
    // Set the Digital IO options
    switch (GPIO_InitStruct->GPIO_Mode) {
    
        case GPIO_Mode_In_Floating: GPIOn->REN &= ~(GPIO_InitStruct->GPIO_PinMask);
                                    GPIOn->DIR &= ~(GPIO_InitStruct->GPIO_PinMask);
                                    break;
        
        case GPIO_Mode_In_Pullup:   GPIOn->REN |= GPIO_InitStruct->GPIO_PinMask;
                                    GPIOn->DIR &= ~(GPIO_InitStruct->GPIO_PinMask);
                                    GPIOn->OUT |= GPIO_InitStruct->GPIO_PinMask;
                                    break;
        
        case GPIO_Mode_In_Pulldown: GPIOn->REN |= GPIO_InitStruct->GPIO_PinMask;
                                    GPIOn->DIR &= ~(GPIO_InitStruct->GPIO_PinMask);
                                    GPIOn->OUT &= ~(GPIO_InitStruct->GPIO_PinMask);
                                    break;
        
        case GPIO_Mode_Out_Weak:    GPIOn->REN &= ~(GPIO_InitStruct->GPIO_PinMask);
                                    GPIOn->DIR |= GPIO_InitStruct->GPIO_PinMask;
                                    GPIOn->DS  &= ~(GPIO_InitStruct->GPIO_PinMask);
                                    if ( GPIO_InitStruct->GPIO_Values == 0 ) {
                                        GPIOn->OUT &= ~(GPIO_InitStruct->GPIO_PinMask);
                                    }
                                    else {
                                        GPIOn->OUT |= GPIO_InitStruct->GPIO_PinMask;
                                    }
                                    break;
        
        case GPIO_Mode_Out_Strong:  GPIOn->REN &= ~(GPIO_InitStruct->GPIO_PinMask);
                                    GPIOn->DIR |= GPIO_InitStruct->GPIO_PinMask;
                                    GPIOn->DS  |= GPIO_InitStruct->GPIO_PinMask;
                                    if ( GPIO_InitStruct->GPIO_Values == 0 ) {
                                        GPIOn->OUT &= ~(GPIO_InitStruct->GPIO_PinMask);
                                    }
                                    else {
                                        GPIOn->OUT |= GPIO_InitStruct->GPIO_PinMask;
                                    }
                                    break;
    }

}




void GPIO_ITConfig(GPIO_Type* GPIOx, u8 GPIO_PinMask, u8 GPIO_EdgeMask, FunctionalState NewState ) {
    
    GPIOx->IES  &= ~GPIO_PinMask;
    GPIOx->IES  |= GPIO_EdgeMask;
    
    if ( NewState == DISABLE ) {
        GPIOx->IE &= ~(GPIO_PinMask);
    }
    else {
        GPIOx->IE |= GPIO_PinMask;
    }
}




void GPIO_ClearITPendingBit(GPIO_Type* GPIOx, u8 GPIO_PinMask) {
    GPIOx->IV &= ~(GPIO_PinMask << 1);
}




void GPIO_ClearITStatus(GPIO_Type* GPIOx, u8 GPIO_PinMask) {
    GPIOx->IFG &= ~(GPIO_PinMask);
}




bool GPIO_GetITStatus(GPIO_Type* GPIOx, u8 GPIO_PinMask) {
    return (bool)(GPIOx->IFG & GPIO_PinMask);
}




void GPIO_Write(GPIO_Type* GPIOx, u8 GPIO_PinMask, u8 Values) {
    (u8)(GPIOx->OUT) = GPIO_PinMask & Values;
}




u8 GPIO_Read(GPIO_Type* GPIOx, u8 GPIO_PinMask) {
    return ( (u8)(GPIOx->IN) & GPIO_PinMask );
}
*/



bool GPIO_UnlockRemap( ) {

    // Write port map password
    PMAPPWD = PMAPPW;
    
    // Allow reconfiguration during runtime:
    PMAPCTL = PMAPRECFG;

    return GPIO_RemapStatus();
}




void GPIO_RemapControl( FunctionalState NewState ) {

    // Write port map password
    PMAPPWD = PMAPPW;

    // If disabling, device requires PUC for subsequent remaps
    if (NewState == DISABLE) {
        PMAPCTL = 0x01;
    }
    else {
        PMAPCTL = 0x02;
    }
}




bool GPIO_RemapStatus() {

    // See if the Remap code is as specified
    if (PMAPPWD == 0x96A5) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}




void GPIO_Remap(u8 Port_Number, u8 Pin_Number, u8 Remap_Index) {
    u8* pin_addr;
    u8 globalInterruptState;
  
    // Setup the port pin selector for alternate usage
    switch (Port_Number) {
#       ifdef _GPIO1
            case 1: GPIO1->SEL |= ( (u8)1 << Pin_Number ); break;
#       endif
#       ifdef _GPIO2
            case 2: GPIO2->SEL |= ( (u8)1 << Pin_Number ); break;
#       endif
#       ifdef _GPIO3
            case 3: GPIO3->SEL |= ( (u8)1 << Pin_Number ); break;
#       endif
#       ifdef _GPIO4
            case 4: GPIO4->SEL |= ( (u8)1 << Pin_Number ); break;
#       endif
#       ifdef _GPIO5
            case 5: GPIO5->SEL |= ( (u8)1 << Pin_Number ); break;
#       endif
         
        default: return;    // Port invalid
    }
  
    // Now do Remapping
    
    globalInterruptState = __get_SR_register() & GIE;                   // Disable all interrupts during Remap
    __disable_interrupt();
    
    PMAPPWD = PMAPPW;                   // Write port map password
    PMAPCTL = PMAPRECFG;                // Allow reconfiguration during runtime:
    
    // Put alternate function on the desired port
    pin_addr    = (u8*)( (u16)PM_BASE + (Port_Number<<3) );
    pin_addr   += Pin_Number;
    *pin_addr   = Remap_Index;
    
    PMAPPWD = 0;                        // Disable write-access to port mapping registers:
    __bis_SR_register(globalInterruptState);
}




/* ***************************************************************************
 * configure_ports
 *  Configures the MSP430 Port Mapper
 *
 * \param *port_mapping:     Pointer to init Data
 * \param PxMAPy:            Pointer start of first Port Mapper to initialize
 * \param num_of_ports:      Number of Ports to initialize
 * \param port_map_reconfig: Flag to enable/disable reconfiguration
 *
*
void configure_ports(u8* port_mapping, u8* PxMAPy, u8 num_of_ports, u8 port_map_reconfig) {
    u8 i;
    u8 globalInterruptState;
  
    globalInterruptState = __get_SR_register() & GIE;
  
    // Disable all interrupts
    __disable_interrupt();
    
    // Get write-access to port mapping registers:
    PMAPPWD = PMAPPW;
    
    if (port_map_reconfig) {
        // Allow reconfiguration during runtime:
        PMAPCTL = PMAPRECFG;
    }
    
    // Configure Port Mapping:
    for (i=0; i<num_of_ports*8; i++) {
        PxMAPy[i] = port_mapping[i];
    }
    
    // Disable write-access to port mapping registers:
    PMAPPWD = 0;
    
    __bis_SR_register(globalInterruptState);
}
*/



