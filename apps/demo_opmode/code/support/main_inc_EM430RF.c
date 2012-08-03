/* Copyright 2012 JP Norair
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
  * @file       /apps/demo_opmode/code/main_inc_EM430RF.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2012
  * @brief      Opmode Switching Demo for EM430RF board: included app components
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
  * below.  They are specific to this app.  Change them if you want.  With the
  * EM430, they just go to unused pins.
  */
#define APP_TRIG3_PORTNUM	3
#define APP_TRIG3_PORT      GPIO3
#define APP_TRIG3_PIN       GPIO_Pin_5
#define APP_TRIG4_PORTNUM	3
#define APP_TRIG4_PORT      GPIO3
#define APP_TRIG4_PIN       GPIO_Pin_7

#define APP_BUTTON_PORTNUM  1
#define APP_BUTTON_VECTOR	PORT1_VECTOR
#define APP_BUTTON_PORT     BOARD_SW2_PORT
#define APP_BUTTON_PIN      BOARD_SW2_PIN
#define APP_BUTTON_POL      BOARD_SW2_POLARITY



/** LED routines, plus a process to show that initialization has succeeded <BR>
  * ===========================================================================
  * The RX and TX indicators are part of the mainline platform code.  All boards
  * must specify at least two "triggers" (i.e. output indicators, usually LEDs).
  * This app requires FOUR triggers.  Triggers 3 and 4 are defined in this app,
  */
void sub_trig3_high() {    APP_TRIG3_PORT->DOUT |= APP_TRIG3_PIN; }
void sub_trig3_low() {     APP_TRIG3_PORT->DOUT &= ~APP_TRIG3_PIN; }
void sub_trig3_toggle() {  APP_TRIG3_PORT->DOUT ^= APP_TRIG3_PIN; }
void sub_trig4_high() {    APP_TRIG4_PORT->DOUT |= APP_TRIG4_PIN; }
void sub_trig4_low() {     APP_TRIG4_PORT->DOUT &= ~APP_TRIG4_PIN; }
void sub_trig4_toggle() {  APP_TRIG4_PORT->DOUT ^= APP_TRIG4_PIN; }


void sub_trig_init() {
#if (APP_TRIG3_PORTNUM == APP_TRIG4_PORTNUM)
    APP_TRIG3_PORT->DDIR |= (APP_TRIG3_PIN | APP_TRIG4_PIN);
    APP_TRIG3_PORT->DS  |= (APP_TRIG3_PIN | APP_TRIG4_PIN);
#else
    APP_TRIG3_PORT->DDIR |= APP_TRIG3_PIN;
    APP_TRIG3_PORT->DS  |= APP_TRIG3_PIN;
    APP_TRIG4_PORT->DDIR |= APP_TRIG4_PIN;
    APP_TRIG4_PORT->DS  |= APP_TRIG4_PIN;
#endif
}


void sub_button_init() {
    // Pin 2.0  = "Key" button, the interrupt source
    // Pin ?.?  = Joystick Right
    // Pin ?.?  = Joystick Left
    // Pin ?.?  = Joystick Up
    // Pin ?.?  = Joystick Down


	// Apply pull-up/down resistor.  The CC430 method to do this is strange.
	// The DOUT needs to be set to 1/0 for pull-up/pull-down.  Lines of code
	// that are startup default are commented-out for optimization.
  //APP_BUTTON_PORT->DDIR   &= ~APP_BUTTON_PIN;
	APP_BUTTON_PORT->REN   |= APP_BUTTON_PIN;
#   if (APP_BUTTON_POL == 1)
	  //APP_BUTTON_PORT->DOUT  &= ~APP_BUTTON_PIN;
#   else
	    APP_BUTTON_PORT->DOUT  |= APP_BUTTON_PIN;
#   endif


    // Check if button is hard-wired, or depressed at startup, in which case
	// the startup behavior will be endpoint.
#   if (APP_BUTTON_POL == 1)
	if (APP_BUTTON_PORT->DIN & APP_BUTTON_PIN) {
#   else
    if ((APP_BUTTON_PORT->DIN & APP_BUTTON_PIN) == 0) {
#   endif
        app_goto_endpoint();
        app_devicemode = SYSMODE_ENDPOINT;
    }

    // Configure Button interrupt to happen when the button is released.
    // Positive/Negative polarity means Falling/Rising Edge detection.
#   if (APP_BUTTON_POL == 1)
        APP_BUTTON_PORT->IES   |= APP_BUTTON_PIN;
#   else
      //APP_BUTTON_PORT->IES   &= ~APP_BUTTON_PIN;
#   endif
    APP_BUTTON_PORT->IFG    = 0;
    APP_BUTTON_PORT->IE    |= APP_BUTTON_PIN;
}



#if (CC_SUPPORT == CL430)
#   pragma vector=APP_BUTTON_VECTOR
#elif (CC_SUPPORT == IAR_V5)
    // don't know yet
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(APP_BUTTON_VECTOR)
#endif

OT_INTERRUPT void app_buttons_isr(void) {
    ot_u8 exti_source;
    exti_source				= APP_BUTTON_PORT->DIN & APP_BUTTON_PIN;
    APP_BUTTON_PORT->IFG	= 0;

    // Make sure the interrupt is the one we want, otherwise ignore
    // The input source (PG8) is active low
    if (exti_source) {
    	sys.loadapp = (app_devicemode != SYSMODE_GATEWAY) ? \
    					&app_goto_gateway : &app_goto_endpoint;

        // No joystick on the board yet!
        // Joystick LEFT:   sys.loadapp = &app_send_query;
        // Joystick RIGHT:  sys.loadapp = &app_send_beacon;
        // Joystick UP:     sys.loadapp = &app_goto_gateway;
        // Joystick DOWN:   sys.loadapp = &app_goto_endpoint;
    }
}


