#include "OTAPI.h"
#include "OT_platform.h"
#include "system.h"

/** Global Variables
  * ============================================================================
  * - t1a3_wait: used in sleep loop to assure proper timing while allowing 
  *              other interrupts to operate.
  *
  * - red_vector: "red" = "Radio Event Debug."  This is a bitvector used by the 
  *               callback routines associated with the RF event "terminate" 
  *               and "init" signals.  During crashes, you can see which bit is
  *               set to determine which event went wrong.
  */
//ot_bool t1a3_wait;      
ot_u16  red_vector;

/** RED Vector Callbacks
  * ============================================================================
  * - Radio Event init signal is type ot_sig: "void signal(ot_int)"
  * - Radio Event terminate signal is type ot_sig2: "void signal(ot_int, ot_int)"
  * - Init is called when state opens, terminate when state closes.
  * - Argument 1 is a state ID, 1-5
  * - Argument 2 is a state-dependent value
  * - See system.h (System Module) for more information
  */

void red_init(ot_int state_id) {
    red_vector |= (1 << (state_id-1));
}

void red_terminate(ot_int state_id, ot_int input) {
    red_vector &= ~(1 << (state_id-1));
}

int
main(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    /// Platform power-up initialization:
    platform_poweron();     // 1. Clocks, Timers, IRQ's, etc
    platform_init_gpio();   // 2. Not built into platform_poweron()
    platform_init_OT();     // 3. OpenTag module inits

    /// Set up the user timer (you can change this if you want).  This code is
    /// lifted from platform_CC430.c (platform_init_gptim)
    TIM_TimeBaseStructure.TIM_Period = 665;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM11, &TIM_TimeBaseStructure);

    
    /// Set up the System RFA callbacks to use the red_ functions
    red_vector              = 0;
    sys.evt.RFA.init        = &red_init;
    sys.evt.RFA.terminate   = &red_terminate;
    
    
    /// Pre-empt the OpenTag Daemon:
    /// Run this once.  Afterward, it will automatically run whenever there is
    /// a GPTIM interrupt (usually TIM0A5).  OT manages this timer internally.
    platform_ot_run();
    
    /// You could put the system to sleep here -- end the program -- and OpenTag
    /// would just run automatically without any higher-level features.  If you
    /// don't stop here, OpenTag will still run automatically, but you can add
    /// pre-emptive higher-layer features.
    
    /// This demo runs mostly automatically, but it shows how to build a dialog
    /// from a higher-layer application.
    
    
    
    
    while (1) {
        ot_u8 status = 0;
        
        /// 1. Wait for the system to be done with intensive tasks.  This is not
        ///    required (tasks will be killed otherwise), but it is the best way
        ///    to make sure that any ongoing data processes can finish cleanly.
        while ( (sys_get_mutex() > SYS_MUTEX_RADIO_LISTEN) ) {
            SLEEP_MCU();
        }
        
        /// 2. Pause OpenTag Daemon so it doesn't interrupt our work below
        platform_ot_pause();
        
        
        /// 3. Build a Dialog Session.  This could be encapsulated into builder
        ///    functions, to make things simple, but the API is lower level, to
        ///    allow the most functionality as "lowest common denomenator."
        
        /// 3a. Create an ad-hoc (pre-emptive) session from our app layer
        {
            session_tmpl app_session;
            app_session.channel     = 0x10;
            app_session.subnet      = 0xFF;     //universal subnet (hits everything)
            app_session.subnetmask  = 0xFF;
            app_session.flags       = 0x00;     //don't care about the built-in flags
            app_session.flagmask    = 0x00;
            app_session.timeout     = 64;       //Contention period = 64 ticks
                                                //(can be 0-65535 ticks, 1 ti = 0.977 ms)
            otapi_new_session(&app_session);
        }
        
        /// 3b. Open a Broadcast Addressed request (Simplest)
        otapi_open_request(BROADCAST, NULL);
        
        /// 3c. Write some command to the Request. I chose Discovery Collection. 
               
        {   // The command tmpl dictates what protocol templates come next
            command_tmpl command;
            command.type        = 0x40;
            command.opcode      = 0x04;
            command.extension   = 0;
            otapi_put_command_tmpl(&status, &command);
        }
        {   // The dialog tmpl instructs the receivers how to respond
            dialog_tmpl dialog;
            dialog.timeout      = 60;           // should be same or less than app_session.timeout
            dialog.channels     = 0;            // use the active channel only
            otapi_put_dialog_tmpl(&status, &dialog);
        }
        {   // The ISFcomp tmpl makes receivers run a comparison check on the 
            // specified file (or batch files).  If you preceed with a query,
            // the comparision can be elaborate.  For broadcast, it just makes
            // sure only receivers that contain the specified file respond.
            isfcomp_tmpl isfcomp;
            isfcomp.is_series   = 0;
            isfcomp.isf_id      = ISF_ID(device_features);
            isfcomp.offset      = 0;
            otapi_put_isf_comp(&status, &isfcomp);
        }
        {   // The ISFcall tmpl tells receivers which file to respond, and what
            // data in the file.  (doesn't have to be the same as comp file)
            isfcall_tmpl isfcall;
            isfcall.is_series   = 0;
            isfcall.isf_id      = ISF_ID(device_features);
            isfcall.max_return  = 80;       // cap file response at 80 bytes
            isfcall.offset      = 0;        // start file response at first byte
            otapi_put_isf_call(&status, &isfcall);
        }
        
        /// 3d. Close the request (package for output).  This allows footer data
        ///     to be written to the frame/packet.
        otapi_close_request();
        
        
        /// 4. Pre-empt the new ad-hoc dialog
        platform_ot_run();
        
        
        
        
        /// 5. This will be needed to prevent race conditions if the radio driver is made
        ///    non-blocking for channel listening
		for (status = 0; status < 0xff; status++) {
			ot_u8 foo;
			asm("nop");
			asm("nop");
			for (foo = 0; foo < 0xff; foo++)
				asm("nop");
		}
        /*
        {
            ot_u16 random_time;
        
            //bounds: 0 to 496 ticks (0 to 484ms)
            random_time = (ot_u16)(platform_prand_u8() & 0x1F) << 4;
            
            // Flush User Timer and use up-counting interrupt mode
            TIM1A3->CTL &= ~0x0033;
            TIM1A3->CTL  = random_time;
            TIM1A3->CTL |= 0x0004;
            TIM1A3->CTL |= 0x0012;
        }
    
        t1a3_wait = True;
        while (t1a3_wait == True) {
            SLEEP_MCU();
        }
        */
    } // ...while (1)

}
