/* Copyright 2011-2012 JP Norair
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
/**
  * @file       /apps/demo_opmode/code/main_inc_MLX73Proto_E.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2012
  * @brief      Opmode Switching Demo for MLX73Proto_E board
  *
  * This file is included into main.c.
  *
  ******************************************************************************
  */





/** Application Triggers & Button(s) <BR>
  * ========================================================================<BR>
  * The RX and TX indicators are part of the mainline platform code, using the
  * two required platform triggers.  All boards must specify at least two 
  * "triggers" (i.e. output indicators, usually LEDs).
  *
  * This app requires FOUR triggers.  Therefore, triggers 3 and 4 are defined 
  * below.  They are specific to this app.  Change them if you want.
  *
  * The Application button is used as an interrupt source.  For now, it also
  * changes modes.
  */
#define APP_TRIG3_PORTNUM	'F'
#define APP_TRIG3_PORT      GPIOF
#define APP_TRIG3_PIN       GPIO_Pin_8
#define APP_TRIG4_PORTNUM	'F'
#define APP_TRIG4_PORT      GPIOF
#define APP_TRIG4_PIN       GPIO_Pin_9

#define APP_BUTTON_PORTNUM  2
#define APP_BUTTON_VECTOR	PORT2_VECTOR
#define APP_BUTTON_PORT     GPIO2
#define APP_BUTTON_PIN      GPIO_Pin_0
#define APP_BUTTON_POL      0




/** LED routines, plus a process to show that initialization has succeeded <BR>
  * ===========================================================================
  * The RX and TX indicators are part of the mainline platform code.  All boards
  * must specify at least two "triggers" (i.e. output indicators, usually LEDs).
  * This app requires FOUR triggers.  Triggers 3 and 4 are defined in this app,
  */
void sub_trig3_high() {    APP_TRIG3_PORT->ODR |= APP_TRIG3_PIN; }      //Red LED
void sub_trig3_low() {     APP_TRIG3_PORT->ODR &= ~APP_TRIG3_PIN; }
void sub_trig3_toggle() {  APP_TRIG3_PORT->ODR ^= APP_TRIG3_PIN; }
void sub_trig4_high() {    APP_TRIG4_PORT->ODR |= APP_TRIG4_PIN; }      // Blue LED
void sub_trig4_low() {     APP_TRIG4_PORT->ODR &= ~APP_TRIG4_PIN; }
void sub_trig4_toggle() {  APP_TRIG4_PORT->ODR ^= APP_TRIG4_PIN; }


void sub_trig_init() {
    GPIO_InitTypeDef GPIOinit;
    GPIOinit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIOinit.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIOinit.GPIO_Pin   = APP_TRIG3_PIN | APP_TRIG4_PIN;
    GPIO_Init(APP_TRIG3_PORT, &GPIOinit); 
}


void sub_button_init() {
///These buttons are falling edge-triggered and they have pull-up resistors.
    GPIO_InitTypeDef GPIOinit;

    //Change devicemode so that the key press causes startup
    app_devicemode = SYSMODE_OFF;

    // Pin G8   = "Key" button, the interrupt source
    // Pin G13  = Joystick Right
    // Pin G14  = Joystick Left
    // Pin G15  = Joystick Up
    // Pin D3   = Joystick Down
    GPIOinit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIOinit.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIOinit.GPIO_Pin   =  GPIO_Pin_8 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOG, &GPIOinit);
    GPIOinit.GPIO_Pin   =  GPIO_Pin_3;
    GPIO_Init(GPIOD, &GPIOinit);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOG, GPIO_PinSource8);

    // External interrupt works on rising edge (button release)
    EXTI->IMR  |= GPIO_Pin_8;
    EXTI->RTSR |= GPIO_Pin_8;
    EXTI->FTSR &= ~GPIO_Pin_8;
    
    //Use Priority 3,0 (application group) for the button ISR
    NVIC->IP[(uint32_t)EXTI9_5_IRQn]        = b1100 << (8 - __NVIC_PRIO_BITS);
    NVIC->ISER[((uint32_t)EXTI9_5_IRQn>>5)] = (1 << ((uint32_t)EXTI9_5_IRQn & 0x1F));
}


void EXTI9_5_IRQHandler(void) {
    ot_u16 exti_source;
    exti_source = EXTI->PR & GPIO_Pin_8;
    EXTI->PR    = 0x03E0;               //Clear interrupt flags

    // Make sure the interrupt is the one we want, otherwise ignore
    // The input source (PG8) is active low
    if (exti_source) {
        // Joystick LEFT:   send a query request (gateway only)
        // Joystick RIGHT:  send a beacon
        // Joystick UP:     go to gateway mode
        // Joystick DOWN:   go to endpoint mode
        if (app_devicemode == SYSMODE_OFF) {
            app_devicemode = SYSMODE_GATEWAY;
            sub_trig4_high();
        }
        else if ((GPIOG->IDR & GPIO_Pin_14) == 0)   sys.loadapp = &app_send_query;
        else if ((GPIOG->IDR & GPIO_Pin_13) == 0)   sys.loadapp = &app_send_beacon;
        else if ((GPIOG->IDR & GPIO_Pin_15) == 0)   sys.loadapp = &app_goto_gateway;    
        else if ((GPIOD->IDR & GPIO_Pin_3) == 0)    sys.loadapp = &app_goto_endpoint; 
    }
}


