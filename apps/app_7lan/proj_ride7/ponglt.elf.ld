
/*
!!! Primary linker script file generated automatically !!!
This file contains the input and output files names
It also includes the secondary/main linker script that is specified in the Ride options (default or custom)
*/

SEARCH_DIR(".")
SEARCH_DIR("B:\apps\app_7lan\proj_ride7")
SEARCH_DIR("C:\Program Files\Raisonance\Ride\Lib\ARM")
STARTUP("startup_stm32l1xx_md.o")
INPUT("B:\apps\app_7lan\proj_ride7\main.o")
INPUT("B:\apps\app_7lan\proj_ride7\dll_sig_rfinit.o")
INPUT("B:\apps\app_7lan\proj_ride7\dll_sig_rfterminate_2.o")
INPUT("B:\apps\app_7lan\proj_ride7\mpipe_sig_done.o")
INPUT("B:\apps\app_7lan\proj_ride7\network_sig_route.o")
INPUT("B:\apps\app_7lan\proj_ride7\sys_sig_panic.o")
INPUT("B:\apps\app_7lan\proj_ride7\sys_sig_powerdown_[stm32lp].o")
INPUT("B:\apps\app_7lan\proj_ride7\system_gulp.o")
INPUT("B:\apps\app_7lan\proj_ride7\system_hicculp.o")
INPUT("B:\apps\app_7lan\proj_ride7\alp_api_client.o")
INPUT("B:\apps\app_7lan\proj_ride7\alp_api_server.o")
INPUT("B:\apps\app_7lan\proj_ride7\alp_dashforth.o")
INPUT("B:\apps\app_7lan\proj_ride7\alp_filedata.o")
INPUT("B:\apps\app_7lan\proj_ride7\alp_logger.o")
INPUT("B:\apps\app_7lan\proj_ride7\alp_main.o")
INPUT("B:\apps\app_7lan\proj_ride7\alp_security.o")
INPUT("B:\apps\app_7lan\proj_ride7\alp_sensor.o")
INPUT("B:\apps\app_7lan\proj_ride7\auth.o")
INPUT("B:\apps\app_7lan\proj_ride7\buffers.o")
INPUT("B:\apps\app_7lan\proj_ride7\crc16.o")
INPUT("B:\apps\app_7lan\proj_ride7\crypto_aes128.o")
INPUT("B:\apps\app_7lan\proj_ride7\external.o")
INPUT("B:\apps\app_7lan\proj_ride7\m2_dll_task.o")
INPUT("B:\apps\app_7lan\proj_ride7\m2_encode.o")
INPUT("B:\apps\app_7lan\proj_ride7\m2_network.o")
INPUT("B:\apps\app_7lan\proj_ride7\m2_transport.o")
INPUT("B:\apps\app_7lan\proj_ride7\mpipe_task.o")
INPUT("B:\apps\app_7lan\proj_ride7\OT_utils.o")
INPUT("B:\apps\app_7lan\proj_ride7\OTAPI_logger.o")
INPUT("B:\apps\app_7lan\proj_ride7\OTAPI_tasker.o")
INPUT("B:\apps\app_7lan\proj_ride7\queue.o")
INPUT("B:\apps\app_7lan\proj_ride7\radio_task.o")
INPUT("B:\apps\app_7lan\proj_ride7\session.o")
INPUT("B:\apps\app_7lan\proj_ride7\veelite.o")
INPUT("B:\apps\app_7lan\proj_ride7\core_cm3.o")
INPUT("B:\apps\app_7lan\proj_ride7\mpipe_uart_STM32L1xx.o")
INPUT("B:\apps\app_7lan\proj_ride7\platform_crc_STM32L1xx.o")
INPUT("B:\apps\app_7lan\proj_ride7\platform_gptim_STM32L1xx.o")
INPUT("B:\apps\app_7lan\proj_ride7\platform_isr_STM32L1xx.o")
INPUT("B:\apps\app_7lan\proj_ride7\platform_STM32L1xx.o")
INPUT("B:\apps\app_7lan\proj_ride7\veelite_core_EE_STM32L1xx.o")
INPUT("B:\apps\app_7lan\proj_ride7\stm32l1xx_flash.o")
INPUT("B:\apps\app_7lan\proj_ride7\radio_SPIRIT1.o")
INPUT("B:\apps\app_7lan\proj_ride7\SPIRIT1_STM32L1xx.o")
EXTERN( __io_putchar )
EXTERN( _write )
GROUP(
"STM32x_io_putchar_thumb.a"
"std_sbrk_thumb.lib"
"smallprintf_thumb.a"
"libc.a"
"libm.a"

)
INCLUDE "B:\apps\app_7lan\proj_ride7\STM32L151_32K_10K_FLASH.ld"
OUTPUT("B:\apps\app_7lan\proj_ride7\ponglt.elf")
