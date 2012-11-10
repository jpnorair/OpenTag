#include "OT_platform.h"
#include "radio.h"
#include "M1_stm32l1xx_it.h"

/**************** not using OTlib for PHY test... ********************/
void sys_init(void) { }
void session_init(void) { }
void auth_init(void) { }
void vl_init(void) { }
ot_int mpipe_init(void* port_id) { return 0; }
ot_u8 vworm_init(void) { return 1; /* 1 for not implemented */ }
ot_uint sys_event_manager(ot_uint elapsed)
{
    return 16;  // the number of ticks between wakeups
}
/**************** ...not using OTlib for PHY test ********************/


ot_u8 bar;


void
print_payload()
{
    int i;

    if (from_tag)
        debug_printf("tag ");
    else
        debug_printf("int ");

    debug_printf("%d: ", rxq.length);
    for (i = 0; i < rxq.length; i++)
        debug_printf("%02x ", q_readbyte(&rxq));

    debug_printf("\r\n");
}

volatile ot_u16 foobar;

int
main()
{
    platform_poweron();     // Clocks, Timers, IRQ's, etc
    platform_init_OT();     // OpenTag module inits

    platform_ot_run();

    rm2_rxinit_ff(0, 0, 0, NULL);    // start receiver

    while (1) {
        SLEEP_MCU();
        if (tim4dma_int != TIM4DMA_NONE)
            radio_dma_service();
        if (rxq_ready) {
            print_payload();
            rm2_rxinit_ff(0, 0, 0, NULL);    // start receiver
        }
    } // ...while (1)

}
