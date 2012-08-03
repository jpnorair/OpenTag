/**
  * @file       /apps/demo_opmode/code/main_inc_STM32H152.c
  * @author     
  * @version    V1.0
  * @date       16 December 2011
  * @brief      Opmode Switching Demo for STM32H152 board
  *
  * This file is included into main.c.
  *
  ******************************************************************************
  */

#include "debug_uart.h"



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
/*#define APP_TRIG3_PORTNUM	'F'
#define APP_TRIG3_PORT      GPIOF
#define APP_TRIG3_PIN       GPIO_Pin_8
#define APP_TRIG4_PORTNUM	'F'
#define APP_TRIG4_PORT      GPIOF
#define APP_TRIG4_PIN       GPIO_Pin_9

#define APP_BUTTON_PORTNUM  A
#define APP_BUTTON_VECTOR	PORTA_VECTOR
#define APP_BUTTON_PORT     GPIOA
#define APP_BUTTON_PIN      GPIO_Pin_0
#define APP_BUTTON_POL      0*/




/** LED routines, plus a process to show that initialization has succeeded <BR>
  * ===========================================================================
  * The RX and TX indicators are part of the mainline platform code.  All boards
  * must specify at least two "triggers" (i.e. output indicators, usually LEDs).
  * This app requires FOUR triggers.  Triggers 3 and 4 are defined in this app,
  */
void sub_trig3_high() {    /*APP_TRIG3_PORT->ODR |= APP_TRIG3_PIN;*/ }      //Red LED
void sub_trig3_low() {     /*APP_TRIG3_PORT->ODR &= ~APP_TRIG3_PIN;*/ }
void sub_trig3_toggle() {  /*APP_TRIG3_PORT->ODR ^= APP_TRIG3_PIN;*/ }
void sub_trig4_high() {    /*APP_TRIG4_PORT->ODR |= APP_TRIG4_PIN;*/ }      // Blue LED
void sub_trig4_low() {     /*APP_TRIG4_PORT->ODR &= ~APP_TRIG4_PIN;*/ }
void sub_trig4_toggle() {  /*APP_TRIG4_PORT->ODR ^= APP_TRIG4_PIN;*/ }


void sub_trig_init() {
}


void sub_button_init() {
    //Change devicemode so that the key press causes startup
    app_devicemode = SYSMODE_OFF;
}


#ifdef RADIO_DEBUG
void
console_service()
{
    if (new_uart_rx == 0)
        return;

    radio_console_service();

    if (uart_rx_buf[0] == '?') {
        debug_printf("\r\na     send query request");
        debug_printf("\r\nd     send beacon");
        debug_printf("\r\nw     become gateway");
        debug_printf("\r\nx     become endpoint");
    } else if (uart_rx_buf[1] == 0) {   // single character
        switch (uart_rx_buf[0]) {
            case 'a': // LEFT:   send a query request (gateway only)
                sys.loadapp = &app_send_query;
                debug_printf("\r\napp_send_query");
                if (app_devicemode == SYSMODE_OFF)
                    app_devicemode = SYSMODE_GATEWAY;
                break;
            case 'd': // RIGHT:  send a beacon
                sys.loadapp = &app_send_beacon;
                debug_printf("\r\napp_send_beacon");
                if (app_devicemode == SYSMODE_OFF)
                    app_devicemode = SYSMODE_GATEWAY;
                break;
            case 'w': // UP:     go to gateway mode
                sys.loadapp = &app_goto_gateway;    
                debug_printf("\r\napp_goto_gateway");
                if (app_devicemode == SYSMODE_OFF)
                    app_devicemode = SYSMODE_GATEWAY;
                break;
            case 'x': // DOWN:   go to endpoint mode
                sys.loadapp = &app_goto_endpoint; 
                debug_printf("\r\napp_goto_endpoint");
                if (app_devicemode == SYSMODE_OFF)
                    app_devicemode = SYSMODE_GATEWAY;
                break;
        } // ...switch (uart_rx_buf[0])
    }

    debug_printf("\r\n> ");

    new_uart_rx = 0;
    uart_rx_buf_idx = 0;
}
#endif /* RADIO_DEBUG */

