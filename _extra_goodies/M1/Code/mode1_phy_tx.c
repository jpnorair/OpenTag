#include <otplatform.h>
#include <m2/radio.h>

/**************** not using OTlib for PHY test... ********************/
void sys_init(void) { }
void session_init(void) { }
//void buffers_init(void) { }
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
ot_u8 foo;

static void
tx_test()
{
    if (++bar == 10)
        bar = 0;

    q_empty(&txq);

/*    switch (bar) {
        case 0:
            q_writebyte(&txq, 0x00);
            q_writebyte(&txq, 0x00);
            break;
        case 1:
            q_writebyte(&txq, 0xff);
            q_writebyte(&txq, 0xff);
            break;
        case 2:
            q_writebyte(&txq, 0x00);
            q_writebyte(&txq, 0xff);
            break;
        case 3:
            q_writebyte(&txq, 0xff);
            q_writebyte(&txq, 0x00);
            break;
        case 4:
            q_writebyte(&txq, 0xaa);
            q_writebyte(&txq, 0x55);
            break;
        case 5:
            q_writebyte(&txq, 0x55);
            q_writebyte(&txq, 0xaa);
            break;
        case 6:
            q_writebyte(&txq, 0x01);
            q_writebyte(&txq, 0x10);
            break;
        case 7:
            q_writebyte(&txq, 0x10);
            q_writebyte(&txq, 0x01);
            break;
        case 8:
            q_writebyte(&txq, 0xbe);
            q_writebyte(&txq, 0xef);
            break;
        case 9:
            q_writebyte(&txq, 0xde);
            q_writebyte(&txq, 0xad);
            break;
    } // ...switch (bar) */

    from_tag = False;

    //q_writebyte(&txq, 0xc6);
/*    q_writebyte(&txq, '1');
    q_writebyte(&txq, '2');
    q_writebyte(&txq, '3');
    q_writebyte(&txq, '4');
    q_writebyte(&txq, '5');
    q_writebyte(&txq, '6');
    q_writebyte(&txq, '7');
    q_writebyte(&txq, '8');
    q_writebyte(&txq, '9');*/
    q_writebyte(&txq, foo++);

    rm2_txinit_ff(0, NULL);
    //rm2_txinit_bf(NULL);  // send wakeup signal

}


volatile ot_u16 foobar;

int
main()
{
    platform_poweron();     // Clocks, Timers, IRQ's, etc
    platform_init_OT();     // OpenTag module inits

    platform_ot_run();

    while (1) {
        SLEEP_MCU();
/*        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");*/
        if (++foobar > 32) {
            foobar = 0;
            //GPIO_WriteBit(GPIO_Port_IO2, GPIO_Pin_IO2, Bit_SET);
            tx_test();
            //GPIO_WriteBit(GPIO_Port_IO2, GPIO_Pin_IO2, Bit_RESET);
        }
    }

}
