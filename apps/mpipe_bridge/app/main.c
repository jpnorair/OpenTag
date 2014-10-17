/* Copyright 2014 JP Norair
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
  * @file       /apps/mpipe_bridge/app/main.c
  * @author     JP Norair
  * @version    R100
  * @date       27 Sept 2014
  * @brief      Mpipe USB to UART bridge example app
  *
  * Functional purpose of this app:
  * <LI> Provide Bridging from a USB to a UART connection. </LI>
  * <LI> Show how OpenTag can be built without any DASH7 Mode 2 interface </LI>
  *
  * Supported Boards & Platforms: <BR>
  * See /apps/mpipe_bridge/app/board_config.h for board & platform support.
  * @note Different boards may support different methods of input and output.
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>
#include <otlib/rand.h>
#include <otlib/logger.h>

#include <otsys.h>



/** Data Mapping <BR>
  * ===========================================================================
  * The MPipe Bridge needs a particular data mapping.  It is not unusual, but
  * the demo may not work if the data is not set correctly.  This define below
  * uses the default data mapping
  */
#define __DATA_DEFAULT






/** Application Global Variables <BR>
  * ========================================================================<BR>
  */






/** Applet Functions (platform & board independent) <BR>
  * ========================================================================
  */
// Main Application Functions
void app_init();


/** Application local subroutines (platform & board dependent) <BR>
  * ========================================================================<BR>
  * sub_button_init() is a board-dependent function, as
  */
void sub_button_init();
//#define APP_TASK    (&sys.task[TASK_external])





/** Application Events <BR>
  * ========================================================================<BR>
  * This Application can be initialized by a button-press or by an ALP command.
  * The ALP command is treated at a higher-level (see ALP callback later).
  * The button-press is unique to this application, and it is treated here.
  */
  
#if (defined(__STM32__) && defined(OT_SWITCH1_PINNUM) && (OT_SWITCH1_PINNUM >= 0))
///@todo need to put this somewhere else, like in board file
#   define PLATFORM_ISR_SW  platform_isr_exti6

void PLATFORM_ISR_SW(void) {
    // Ignore the button press if the task is in progress already
    //if (APP_TASK->event == 0) {
        
    //}
}

void sub_button_init() {
/// ARM Cortex M boards must prepare all EXTI line interrupts in their board
/// configuration files.
}


#else
#   warn "You are not using a known, compatible MCU.  Demo might not work."
    void sub_button_init() {}

#endif









/** Application Main <BR>
  * ==================================================================<BR>
  *
  */
void app_init() {
/// 1. Blink the board LEDs to show that it is starting up.
/// 2. Configure the board input button, which for this app will send a ping
    ot_u8 i;

    i=4;
    while (i != 0) {
        if (i&1)    BOARD_led1_on();
        else        BOARD_led2_on();

        delay_ms(100);
        BOARD_led2_off();
        BOARD_led1_off();
        i--;
    }

    sub_button_init();
}




void main(void) {
    ///1. Standard Power-on routine (Clocks, Timers, IRQ's, etc)
    ///2. Standard OpenTag Init (most stuff actually will not be used)
    platform_poweron();
    platform_init_OT();

    ///3. Initialize the User Applet & interrupts
    app_init();

    ///4. Set the app name (MPipeBridge) as a cookie in the User-ID.
    ///   This is used for query filtering
    {   static const ot_u8 appstr[] = "APP=MPipeBridge";
        vlFILE* fp;
        fp = ISF_open_su(ISF_ID(user_id));
        if (fp != NULL) {
            vl_store(fp, sizeof(appstr), (ot_u8*)appstr);
        }
        vl_close(fp);
    }

    ///5a. The device will wait (and block anything else) until you connect
    ///    it to a valid console app.
    ///5b. Load a message to show that main startup has passed
#   if (OT_FEATURE(MPIPE))
    mpipedrv_standby();
    logger_msg(MSG_utf8, 6, 26, (ot_u8*)"SYS_ON", (ot_u8*)"System on and Mpipe active");
#   endif


    ///6. MAIN RUNTIME (post-init)  <BR>
    ///<LI> Use a main loop with platform_ot_run(), and nothing more. </LI>
    ///<LI> You could put code before or after sys_runtime_manager, which will
    ///     run before or after the (task + kernel).  If you do, keep the code
    ///     very short or else you are risking timing glitches.</LI>
    ///<LI> To run any significant amount of user code, use tasks. </LI>
    while(1) {
        platform_ot_run();
    }
}




#if defined(__DATA_DEFAULT)
#   include <../_common/fs_default_startup.c>
#else
#   error "There is no data mapping specified.  Put one here."
#endif

