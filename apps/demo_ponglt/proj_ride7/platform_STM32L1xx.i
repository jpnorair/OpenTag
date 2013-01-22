# 1 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
# 1 "B:\\apps\\demo_ponglt\\proj_ride7//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
# 31 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
# 1 "B:\\otlib/OT_utils.h" 1
# 30 "B:\\otlib/OT_utils.h"
# 1 "B:\\otlib/OT_types.h" 1
# 83 "B:\\otlib/OT_types.h"
    typedef enum { False = 0, True = !False } ot_bool;







typedef signed char ot_s8;




typedef unsigned char ot_u8;






typedef signed short ot_int;
typedef signed short ot_s16;






typedef unsigned short ot_uint;
typedef unsigned short ot_u16;




typedef signed long ot_long;
typedef signed long ot_s32;





typedef unsigned long ot_ulong;
typedef unsigned long ot_u32;





typedef union {
    ot_u16 ushort;
    ot_s16 sshort;
    ot_u8 ubyte[2];
    ot_s8 sbyte[2];
} ot_uni16;




typedef union {
    ot_u32 ulong;
    ot_s32 slong;
    ot_u16 ushort[2];
    ot_s16 sshort;
    ot_u8 ubyte[4];
    ot_s8 sbyte[4];
} ot_uni32;
# 160 "B:\\otlib/OT_types.h"
typedef void (*ot_sub)(void);






typedef void (*ot_sig)(ot_int);






typedef void (*ot_sig2)(ot_int, ot_int);






typedef void (*ot_sigv)(void*);






typedef void (*ot_sigv2)(void*, void*);
# 31 "B:\\otlib/OT_utils.h" 2
# 1 "B:\\otlib/OT_config.h" 1
# 35 "B:\\otlib/OT_config.h"
# 1 "B:\\otlib/OT_support.h" 1
# 36 "B:\\otlib/OT_config.h" 2
# 1 "B:\\apps\\demo_ponglt\\code/app_config.h" 1
# 31 "B:\\apps\\demo_ponglt\\code/app_config.h"
# 1 "B:\\apps\\demo_ponglt\\code/build_config.h" 1
# 31 "B:\\apps\\demo_ponglt\\code/build_config.h"
# 1 "B:\\otlib/OT_support.h" 1
# 32 "B:\\apps\\demo_ponglt\\code/build_config.h" 2
# 32 "B:\\apps\\demo_ponglt\\code/app_config.h" 2
# 37 "B:\\otlib/OT_config.h" 2
# 1 "B:\\apps\\demo_ponglt\\code/extf_config.h" 1
# 38 "B:\\otlib/OT_config.h" 2
# 32 "B:\\otlib/OT_utils.h" 2
# 1 "B:\\otlib/session.h" 1
# 156 "B:\\otlib/session.h"
typedef struct m2session {
 void (*applet)(struct m2session*);
    ot_u16 counter;
    ot_u8 channel;
    ot_u8 netstate;
    ot_u8 protocol;
    ot_u8 dialog_id;
    ot_u8 subnet;
    ot_u8 flags;
} m2session;
# 179 "B:\\otlib/session.h"
typedef void (*ot_app)(m2session*);




typedef struct {
    ot_s8 top;
    ot_u8 seq_number;
    m2session heap[4];
} session_struct;

extern session_struct session;
# 200 "B:\\otlib/session.h"
void session_init();
# 209 "B:\\otlib/session.h"
ot_bool session_refresh(ot_uint elapsed_ti);
# 225 "B:\\otlib/session.h"
m2session* session_new(ot_app applet, ot_u16 new_counter, ot_u8 new_netstate, ot_u8 new_channel);
# 236 "B:\\otlib/session.h"
ot_bool session_occupied(ot_u8 chan_id);
# 245 "B:\\otlib/session.h"
void session_pop();
# 254 "B:\\otlib/session.h"
void session_flush();
# 263 "B:\\otlib/session.h"
void session_crop(ot_u16 threshold);
# 272 "B:\\otlib/session.h"
void session_drop();
# 281 "B:\\otlib/session.h"
ot_int session_count();
# 292 "B:\\otlib/session.h"
m2session* session_top();
# 301 "B:\\otlib/session.h"
ot_u8 session_netstate();
# 33 "B:\\otlib/OT_utils.h" 2



void otutils_null(void);
void otutils_sig_null(ot_int a);
void otutils_sig2_null(ot_int a, ot_int b);
void otutils_sigv_null(void* a);
void otutils_applet_null(m2session* a);




ot_u16 otutils_calc_timeout(ot_u8 timeout_code);



ot_u8 otutils_encode_timeout(ot_u16 timeout_ticks);



ot_int otutils_bin2hex(ot_u8* src, ot_u8* dst, ot_int size);



ot_int otutils_int2dec(ot_u8* dst, ot_int data);


ot_int slistf(ot_u8* dst, const char* label, char format, ot_u8 number, ot_u8* src);
# 32 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c" 2
# 1 "B:\\otlib/OT_types.h" 1
# 33 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c" 2
# 1 "B:\\otlib/OT_config.h" 1
# 34 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c" 2
# 1 "B:\\otlib/OT_platform.h" 1
# 38 "B:\\otlib/OT_platform.h"
# 1 "B:\\apps\\demo_ponglt\\code/platform_config.h" 1
# 75 "B:\\apps\\demo_ponglt\\code/platform_config.h"
# 1 "B:\\apps\\demo_ponglt\\code/isr_config_STM32L.h" 1
# 76 "B:\\apps\\demo_ponglt\\code/platform_config.h" 2
# 1 "B:\\board/stm32l1xx/board_IKR001.h" 1
# 37 "B:\\board/stm32l1xx/board_IKR001.h"
# 1 "B:\\apps\\demo_ponglt\\code/build_config.h" 1
# 38 "B:\\board/stm32l1xx/board_IKR001.h" 2
# 1 "B:\\otplatform\\stm32l1xx/platform_STM32L1xx.h" 1
# 33 "B:\\otplatform\\stm32l1xx/platform_STM32L1xx.h"
# 1 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\DeviceSupport/ST/STM32L1xx/stm32l1xx.h" 1
# 168 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\DeviceSupport/ST/STM32L1xx/stm32l1xx.h"
typedef enum IRQn
{

  NonMaskableInt_IRQn = -14,
  MemoryManagement_IRQn = -12,
  BusFault_IRQn = -11,
  UsageFault_IRQn = -10,
  SVC_IRQn = -5,
  DebugMonitor_IRQn = -4,
  PendSV_IRQn = -2,
  SysTick_IRQn = -1,


  WWDG_IRQn = 0,
  PVD_IRQn = 1,
  TAMPER_STAMP_IRQn = 2,
  RTC_WKUP_IRQn = 3,
  FLASH_IRQn = 4,
  RCC_IRQn = 5,
  EXTI0_IRQn = 6,
  EXTI1_IRQn = 7,
  EXTI2_IRQn = 8,
  EXTI3_IRQn = 9,
  EXTI4_IRQn = 10,
  DMA1_Channel1_IRQn = 11,
  DMA1_Channel2_IRQn = 12,
  DMA1_Channel3_IRQn = 13,
  DMA1_Channel4_IRQn = 14,
  DMA1_Channel5_IRQn = 15,
  DMA1_Channel6_IRQn = 16,
  DMA1_Channel7_IRQn = 17,
  ADC1_IRQn = 18,
  USB_HP_IRQn = 19,
  USB_LP_IRQn = 20,
  DAC_IRQn = 21,
  COMP_IRQn = 22,
  EXTI9_5_IRQn = 23,
  LCD_IRQn = 24,
  TIM9_IRQn = 25,
  TIM10_IRQn = 26,
  TIM11_IRQn = 27,
  TIM2_IRQn = 28,
  TIM3_IRQn = 29,
  TIM4_IRQn = 30,
  I2C1_EV_IRQn = 31,
  I2C1_ER_IRQn = 32,
  I2C2_EV_IRQn = 33,
  I2C2_ER_IRQn = 34,
  SPI1_IRQn = 35,
  SPI2_IRQn = 36,
  USART1_IRQn = 37,
  USART2_IRQn = 38,
  USART3_IRQn = 39,
  EXTI15_10_IRQn = 40,
  RTC_Alarm_IRQn = 41,
  USB_FS_WKUP_IRQn = 42,
  TIM6_IRQn = 43,
# 243 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\DeviceSupport/ST/STM32L1xx/stm32l1xx.h"
  TIM7_IRQn = 44,
  SDIO_IRQn = 45,
  TIM5_IRQn = 46,
  SPI3_IRQn = 47,
  UART4_IRQn = 48,
  UART5_IRQn = 49,
  DMA2_Channel1_IRQn = 50,
  DMA2_Channel2_IRQn = 51,
  DMA2_Channel3_IRQn = 52,
  DMA2_Channel4_IRQn = 53,
  DMA2_Channel5_IRQn = 54,
  AES_IRQn = 55,
  COMP_ACQ_IRQn = 56

} IRQn_Type;





# 1 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h" 1
# 90 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
# 1 "c:\\program files\\raisonance\\ride\\arm-gcc\\bin\\../lib/gcc/arm-none-eabi/4.5.2/include/stdint.h" 1 3 4


# 1 "c:\\program files\\raisonance\\ride\\arm-gcc\\bin\\../lib/gcc/arm-none-eabi/4.5.2/../../../../arm-none-eabi/include/stdint.h" 1 3 4
# 41 "c:\\program files\\raisonance\\ride\\arm-gcc\\bin\\../lib/gcc/arm-none-eabi/4.5.2/../../../../arm-none-eabi/include/stdint.h" 3 4
typedef signed char int8_t ;
typedef unsigned char uint8_t ;




typedef signed char int_least8_t;
typedef unsigned char uint_least8_t;




typedef signed short int16_t;
typedef unsigned short uint16_t;
# 67 "c:\\program files\\raisonance\\ride\\arm-gcc\\bin\\../lib/gcc/arm-none-eabi/4.5.2/../../../../arm-none-eabi/include/stdint.h" 3 4
typedef int16_t int_least16_t;
typedef uint16_t uint_least16_t;
# 79 "c:\\program files\\raisonance\\ride\\arm-gcc\\bin\\../lib/gcc/arm-none-eabi/4.5.2/../../../../arm-none-eabi/include/stdint.h" 3 4
typedef signed long int32_t;
typedef unsigned long uint32_t;
# 97 "c:\\program files\\raisonance\\ride\\arm-gcc\\bin\\../lib/gcc/arm-none-eabi/4.5.2/../../../../arm-none-eabi/include/stdint.h" 3 4
typedef int32_t int_least32_t;
typedef uint32_t uint_least32_t;
# 119 "c:\\program files\\raisonance\\ride\\arm-gcc\\bin\\../lib/gcc/arm-none-eabi/4.5.2/../../../../arm-none-eabi/include/stdint.h" 3 4
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
# 129 "c:\\program files\\raisonance\\ride\\arm-gcc\\bin\\../lib/gcc/arm-none-eabi/4.5.2/../../../../arm-none-eabi/include/stdint.h" 3 4
typedef int64_t int_least64_t;
typedef uint64_t uint_least64_t;
# 159 "c:\\program files\\raisonance\\ride\\arm-gcc\\bin\\../lib/gcc/arm-none-eabi/4.5.2/../../../../arm-none-eabi/include/stdint.h" 3 4
  typedef signed int int_fast8_t;
  typedef unsigned int uint_fast8_t;




  typedef signed int int_fast16_t;
  typedef unsigned int uint_fast16_t;




  typedef signed int int_fast32_t;
  typedef unsigned int uint_fast32_t;
# 213 "c:\\program files\\raisonance\\ride\\arm-gcc\\bin\\../lib/gcc/arm-none-eabi/4.5.2/../../../../arm-none-eabi/include/stdint.h" 3 4
  typedef int_least64_t int_fast64_t;
  typedef uint_least64_t uint_fast64_t;







  typedef long long int intmax_t;
# 231 "c:\\program files\\raisonance\\ride\\arm-gcc\\bin\\../lib/gcc/arm-none-eabi/4.5.2/../../../../arm-none-eabi/include/stdint.h" 3 4
  typedef long long unsigned int uintmax_t;
# 243 "c:\\program files\\raisonance\\ride\\arm-gcc\\bin\\../lib/gcc/arm-none-eabi/4.5.2/../../../../arm-none-eabi/include/stdint.h" 3 4
typedef signed int intptr_t;
typedef unsigned int uintptr_t;
# 4 "c:\\program files\\raisonance\\ride\\arm-gcc\\bin\\../lib/gcc/arm-none-eabi/4.5.2/include/stdint.h" 2 3 4
# 91 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h" 2
# 132 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
typedef struct
{
  volatile uint32_t ISER[8];
       uint32_t RESERVED0[24];
  volatile uint32_t ICER[8];
       uint32_t RSERVED1[24];
  volatile uint32_t ISPR[8];
       uint32_t RESERVED2[24];
  volatile uint32_t ICPR[8];
       uint32_t RESERVED3[24];
  volatile uint32_t IABR[8];
       uint32_t RESERVED4[56];
  volatile uint8_t IP[240];
       uint32_t RESERVED5[644];
  volatile uint32_t STIR;
} NVIC_Type;







typedef struct
{
  volatile const uint32_t CPUID;
  volatile uint32_t ICSR;
  volatile uint32_t VTOR;
  volatile uint32_t AIRCR;
  volatile uint32_t SCR;
  volatile uint32_t CCR;
  volatile uint8_t SHP[12];
  volatile uint32_t SHCSR;
  volatile uint32_t CFSR;
  volatile uint32_t HFSR;
  volatile uint32_t DFSR;
  volatile uint32_t MMFAR;
  volatile uint32_t BFAR;
  volatile uint32_t AFSR;
  volatile const uint32_t PFR[2];
  volatile const uint32_t DFR;
  volatile const uint32_t ADR;
  volatile const uint32_t MMFR[4];
  volatile const uint32_t ISAR[5];
} SCB_Type;
# 365 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
typedef struct
{
  volatile uint32_t CTRL;
  volatile uint32_t LOAD;
  volatile uint32_t VAL;
  volatile const uint32_t CALIB;
} SysTick_Type;
# 410 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
typedef struct
{
  volatile union
  {
    volatile uint8_t u8;
    volatile uint16_t u16;
    volatile uint32_t u32;
  } PORT [32];
       uint32_t RESERVED0[864];
  volatile uint32_t TER;
       uint32_t RESERVED1[15];
  volatile uint32_t TPR;
       uint32_t RESERVED2[15];
  volatile uint32_t TCR;
       uint32_t RESERVED3[29];
  volatile uint32_t IWR;
  volatile uint32_t IRR;
  volatile uint32_t IMCR;
       uint32_t RESERVED4[43];
  volatile uint32_t LAR;
  volatile uint32_t LSR;
       uint32_t RESERVED5[6];
  volatile const uint32_t PID4;
  volatile const uint32_t PID5;
  volatile const uint32_t PID6;
  volatile const uint32_t PID7;
  volatile const uint32_t PID0;
  volatile const uint32_t PID1;
  volatile const uint32_t PID2;
  volatile const uint32_t PID3;
  volatile const uint32_t CID0;
  volatile const uint32_t CID1;
  volatile const uint32_t CID2;
  volatile const uint32_t CID3;
} ITM_Type;
# 503 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
typedef struct
{
       uint32_t RESERVED0;
  volatile const uint32_t ICTR;

  volatile uint32_t ACTLR;



} InterruptType_Type;
# 535 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
typedef struct
{
  volatile const uint32_t TYPE;
  volatile uint32_t CTRL;
  volatile uint32_t RNR;
  volatile uint32_t RBAR;
  volatile uint32_t RASR;
  volatile uint32_t RBAR_A1;
  volatile uint32_t RASR_A1;
  volatile uint32_t RBAR_A2;
  volatile uint32_t RASR_A2;
  volatile uint32_t RBAR_A3;
  volatile uint32_t RASR_A3;
} MPU_Type;
# 620 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
typedef struct
{
  volatile uint32_t DHCSR;
  volatile uint32_t DCRSR;
  volatile uint32_t DCRDR;
  volatile uint32_t DEMCR;
} CoreDebug_Type;
# 1204 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline void __enable_irq() { __asm volatile ("cpsie i"); }
static inline void __disable_irq() { __asm volatile ("cpsid i"); }

static inline void __enable_fault_irq() { __asm volatile ("cpsie f"); }
static inline void __disable_fault_irq() { __asm volatile ("cpsid f"); }

static inline void __NOP() { __asm volatile ("nop"); }
static inline void __WFI() { __asm volatile ("wfi"); }
static inline void __WFE() { __asm volatile ("wfe"); }
static inline void __SEV() { __asm volatile ("sev"); }
static inline void __ISB() { __asm volatile ("isb"); }
static inline void __DSB() { __asm volatile ("dsb"); }
static inline void __DMB() { __asm volatile ("dmb"); }
static inline void __CLREX() { __asm volatile ("clrex"); }
# 1227 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern uint32_t __get_PSP(void);
# 1237 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern void __set_PSP(uint32_t topOfProcStack);
# 1247 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern uint32_t __get_MSP(void);
# 1257 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern void __set_MSP(uint32_t topOfMainStack);
# 1266 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern uint32_t __get_BASEPRI(void);
# 1275 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern void __set_BASEPRI(uint32_t basePri);
# 1284 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern uint32_t __get_PRIMASK(void);
# 1293 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern void __set_PRIMASK(uint32_t priMask);
# 1302 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern uint32_t __get_FAULTMASK(void);
# 1311 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern void __set_FAULTMASK(uint32_t faultMask);
# 1320 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern uint32_t __get_CONTROL(void);
# 1329 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern void __set_CONTROL(uint32_t control);
# 1339 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern uint32_t __REV(uint32_t value);
# 1349 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern uint32_t __REV16(uint16_t value);
# 1359 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern int32_t __REVSH(int16_t value);
# 1369 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern uint32_t __RBIT(uint32_t value);
# 1379 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern uint8_t __LDREXB(uint8_t *addr);
# 1389 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern uint16_t __LDREXH(uint16_t *addr);
# 1399 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern uint32_t __LDREXW(uint32_t *addr);
# 1410 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern uint32_t __STREXB(uint8_t value, uint8_t *addr);
# 1421 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern uint32_t __STREXH(uint16_t value, uint16_t *addr);
# 1432 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern uint32_t __STREXW(uint32_t value, uint32_t *addr);
# 1468 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline void NVIC_SetPriorityGrouping(uint32_t PriorityGroup)
{
  uint32_t reg_value;
  uint32_t PriorityGroupTmp = (PriorityGroup & 0x07);

  reg_value = ((SCB_Type *) ((0xE000E000) + 0x0D00))->AIRCR;
  reg_value &= ~((0xFFFFul << 16) | (7ul << 8));
  reg_value = (reg_value |
                (0x5FA << 16) |
                (PriorityGroupTmp << 8));
  ((SCB_Type *) ((0xE000E000) + 0x0D00))->AIRCR = reg_value;
}
# 1489 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline uint32_t NVIC_GetPriorityGrouping(void)
{
  return ((((SCB_Type *) ((0xE000E000) + 0x0D00))->AIRCR & (7ul << 8)) >> 8);
}
# 1502 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline void NVIC_EnableIRQ(IRQn_Type IRQn)
{
  ((NVIC_Type *) ((0xE000E000) + 0x0100))->ISER[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F));
}
# 1515 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline void NVIC_DisableIRQ(IRQn_Type IRQn)
{
  ((NVIC_Type *) ((0xE000E000) + 0x0100))->ICER[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F));
}
# 1529 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline uint32_t NVIC_GetPendingIRQ(IRQn_Type IRQn)
{
  return((uint32_t) ((((NVIC_Type *) ((0xE000E000) + 0x0100))->ISPR[(uint32_t)(IRQn) >> 5] & (1 << ((uint32_t)(IRQn) & 0x1F)))?1:0));
}
# 1542 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline void NVIC_SetPendingIRQ(IRQn_Type IRQn)
{
  ((NVIC_Type *) ((0xE000E000) + 0x0100))->ISPR[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F));
}
# 1555 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline void NVIC_ClearPendingIRQ(IRQn_Type IRQn)
{
  ((NVIC_Type *) ((0xE000E000) + 0x0100))->ICPR[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F));
}
# 1569 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline uint32_t NVIC_GetActive(IRQn_Type IRQn)
{
  return((uint32_t)((((NVIC_Type *) ((0xE000E000) + 0x0100))->IABR[(uint32_t)(IRQn) >> 5] & (1 << ((uint32_t)(IRQn) & 0x1F)))?1:0));
}
# 1586 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline void NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority)
{
  if(IRQn < 0) {
    ((SCB_Type *) ((0xE000E000) + 0x0D00))->SHP[((uint32_t)(IRQn) & 0xF)-4] = ((priority << (8 - 4)) & 0xff); }
  else {
    ((NVIC_Type *) ((0xE000E000) + 0x0100))->IP[(uint32_t)(IRQn)] = ((priority << (8 - 4)) & 0xff); }
}
# 1609 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline uint32_t NVIC_GetPriority(IRQn_Type IRQn)
{

  if(IRQn < 0) {
    return((uint32_t)(((SCB_Type *) ((0xE000E000) + 0x0D00))->SHP[((uint32_t)(IRQn) & 0xF)-4] >> (8 - 4))); }
  else {
    return((uint32_t)(((NVIC_Type *) ((0xE000E000) + 0x0100))->IP[(uint32_t)(IRQn)] >> (8 - 4))); }
}
# 1634 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline uint32_t NVIC_EncodePriority (uint32_t PriorityGroup, uint32_t PreemptPriority, uint32_t SubPriority)
{
  uint32_t PriorityGroupTmp = (PriorityGroup & 0x07);
  uint32_t PreemptPriorityBits;
  uint32_t SubPriorityBits;

  PreemptPriorityBits = ((7 - PriorityGroupTmp) > 4) ? 4 : 7 - PriorityGroupTmp;
  SubPriorityBits = ((PriorityGroupTmp + 4) < 7) ? 0 : PriorityGroupTmp - 7 + 4;

  return (
           ((PreemptPriority & ((1 << (PreemptPriorityBits)) - 1)) << SubPriorityBits) |
           ((SubPriority & ((1 << (SubPriorityBits )) - 1)))
         );
}
# 1665 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline void NVIC_DecodePriority (uint32_t Priority, uint32_t PriorityGroup, uint32_t* pPreemptPriority, uint32_t* pSubPriority)
{
  uint32_t PriorityGroupTmp = (PriorityGroup & 0x07);
  uint32_t PreemptPriorityBits;
  uint32_t SubPriorityBits;

  PreemptPriorityBits = ((7 - PriorityGroupTmp) > 4) ? 4 : 7 - PriorityGroupTmp;
  SubPriorityBits = ((PriorityGroupTmp + 4) < 7) ? 0 : PriorityGroupTmp - 7 + 4;

  *pPreemptPriority = (Priority >> SubPriorityBits) & ((1 << (PreemptPriorityBits)) - 1);
  *pSubPriority = (Priority ) & ((1 << (SubPriorityBits )) - 1);
}
# 1694 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline uint32_t SysTick_Config(uint32_t ticks)
{
  if (ticks > (0xFFFFFFul << 0)) return (1);

  ((SysTick_Type *) ((0xE000E000) + 0x0010))->LOAD = (ticks & (0xFFFFFFul << 0)) - 1;
  NVIC_SetPriority (SysTick_IRQn, (1<<4) - 1);
  ((SysTick_Type *) ((0xE000E000) + 0x0010))->VAL = 0;
  ((SysTick_Type *) ((0xE000E000) + 0x0010))->CTRL = (1ul << 2) |
                   (1ul << 1) |
                   (1ul << 0);
  return (0);
}
# 1719 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline void NVIC_SystemReset(void)
{
  ((SCB_Type *) ((0xE000E000) + 0x0D00))->AIRCR = ((0x5FA << 16) |
                 (((SCB_Type *) ((0xE000E000) + 0x0D00))->AIRCR & (7ul << 8)) |
                 (1ul << 2));
  __DSB();
  while(1);
}
# 1742 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
extern volatile int ITM_RxBuffer;
# 1756 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline uint32_t ITM_SendChar (uint32_t ch)
{
  if ((((CoreDebug_Type *) (0xE000EDF0))->DEMCR & (1ul << 24)) &&
      (((ITM_Type *) (0xE0000000))->TCR & (1ul << 0)) &&
      (((ITM_Type *) (0xE0000000))->TER & (1ul << 0) ) )
  {
    while (((ITM_Type *) (0xE0000000))->PORT[0].u32 == 0);
    ((ITM_Type *) (0xE0000000))->PORT[0].u8 = (uint8_t) ch;
  }
  return (ch);
}
# 1778 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline int ITM_ReceiveChar (void) {
  int ch = -1;

  if (ITM_RxBuffer != 0x5AA55AA5) {
    ch = ITM_RxBuffer;
    ITM_RxBuffer = 0x5AA55AA5;
  }

  return (ch);
}
# 1798 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
static inline int ITM_CheckChar (void) {

  if (ITM_RxBuffer == 0x5AA55AA5) {
    return (0);
  } else {
    return (1);
  }
}
# 264 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\DeviceSupport/ST/STM32L1xx/stm32l1xx.h" 2
# 1 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\DeviceSupport/ST/STM32L1xx/system_stm32l1xx.h" 1
# 87 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\DeviceSupport/ST/STM32L1xx/system_stm32l1xx.h"
void SystemInit(void);
# 265 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\DeviceSupport/ST/STM32L1xx/stm32l1xx.h" 2






typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;

typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;


typedef enum {ERROR = 0, SUCCESS = !ERROR} ErrorStatus;
# 330 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\DeviceSupport/ST/STM32L1xx/stm32l1xx.h"
typedef struct
{
  volatile uint32_t SR;
  volatile uint32_t CR1;
  volatile uint32_t CR2;
  volatile uint32_t SMPR1;
  volatile uint32_t SMPR2;
  volatile uint32_t SMPR3;
  volatile uint32_t JOFR1;
  volatile uint32_t JOFR2;
  volatile uint32_t JOFR3;
  volatile uint32_t JOFR4;
  volatile uint32_t HTR;
  volatile uint32_t LTR;
  volatile uint32_t SQR1;
  volatile uint32_t SQR2;
  volatile uint32_t SQR3;
  volatile uint32_t SQR4;
  volatile uint32_t SQR5;
  volatile uint32_t JSQR;
  volatile uint32_t JDR1;
  volatile uint32_t JDR2;
  volatile uint32_t JDR3;
  volatile uint32_t JDR4;
  volatile uint32_t DR;
  volatile uint32_t SMPR0;
} ADC_TypeDef;

typedef struct
{
  volatile uint32_t CSR;
  volatile uint32_t CCR;
} ADC_Common_TypeDef;






typedef struct
{
  volatile uint32_t CR;
  volatile uint32_t SR;
  volatile uint32_t DINR;
  volatile uint32_t DOUTR;
  volatile uint32_t KEYR0;
  volatile uint32_t KEYR1;
  volatile uint32_t KEYR2;
  volatile uint32_t KEYR3;
  volatile uint32_t IVR0;
  volatile uint32_t IVR1;
  volatile uint32_t IVR2;
  volatile uint32_t IVR3;
} AES_TypeDef;





typedef struct
{
  volatile uint32_t CSR;
} COMP_TypeDef;





typedef struct
{
  volatile uint32_t DR;
  volatile uint8_t IDR;
  uint8_t RESERVED0;
  uint16_t RESERVED1;
  volatile uint32_t CR;
} CRC_TypeDef;





typedef struct
{
  volatile uint32_t CR;
  volatile uint32_t SWTRIGR;
  volatile uint32_t DHR12R1;
  volatile uint32_t DHR12L1;
  volatile uint32_t DHR8R1;
  volatile uint32_t DHR12R2;
  volatile uint32_t DHR12L2;
  volatile uint32_t DHR8R2;
  volatile uint32_t DHR12RD;
  volatile uint32_t DHR12LD;
  volatile uint32_t DHR8RD;
  volatile uint32_t DOR1;
  volatile uint32_t DOR2;
  volatile uint32_t SR;
} DAC_TypeDef;





typedef struct
{
  volatile uint32_t IDCODE;
  volatile uint32_t CR;
  volatile uint32_t APB1FZ;
  volatile uint32_t APB2FZ;
}DBGMCU_TypeDef;





typedef struct
{
  volatile uint32_t CCR;
  volatile uint32_t CNDTR;
  volatile uint32_t CPAR;
  volatile uint32_t CMAR;
} DMA_Channel_TypeDef;

typedef struct
{
  volatile uint32_t ISR;
  volatile uint32_t IFCR;
} DMA_TypeDef;





typedef struct
{
  volatile uint32_t IMR;
  volatile uint32_t EMR;
  volatile uint32_t RTSR;
  volatile uint32_t FTSR;
  volatile uint32_t SWIER;
  volatile uint32_t PR;
} EXTI_TypeDef;





typedef struct
{
  volatile uint32_t ACR;
  volatile uint32_t PECR;
  volatile uint32_t PDKEYR;
  volatile uint32_t PEKEYR;
  volatile uint32_t PRGKEYR;
  volatile uint32_t OPTKEYR;
  volatile uint32_t SR;
  volatile uint32_t OBR;
  volatile uint32_t WRPR;
  uint32_t RESERVED[23];
  volatile uint32_t WRPR1;
  volatile uint32_t WRPR2;
} FLASH_TypeDef;





typedef struct
{
  volatile uint32_t RDP;
  volatile uint32_t USER;
  volatile uint32_t WRP01;
  volatile uint32_t WRP23;
  volatile uint32_t WRP45;
  volatile uint32_t WRP67;
  volatile uint32_t WRP89;
  volatile uint32_t WRP1011;
} OB_TypeDef;





typedef struct
{
  volatile uint32_t CSR;
  volatile uint32_t OTR;
  volatile uint32_t LPOTR;
} OPAMP_TypeDef;





typedef struct
{
  volatile uint32_t BTCR[8];
} FSMC_Bank1_TypeDef;





typedef struct
{
  volatile uint32_t BWTR[7];
} FSMC_Bank1E_TypeDef;





typedef struct
{
  volatile uint32_t MODER;
  volatile uint16_t OTYPER;
  uint16_t RESERVED0;
  volatile uint32_t OSPEEDR;
  volatile uint32_t PUPDR;
  volatile uint16_t IDR;
  uint16_t RESERVED1;
  volatile uint16_t ODR;
  uint16_t RESERVED2;
  volatile uint16_t BSRRL;
  volatile uint16_t BSRRH;
  volatile uint32_t LCKR;
  volatile uint32_t AFR[2];
  volatile uint16_t BRR;
  uint16_t RESERVED3;
} GPIO_TypeDef;





typedef struct
{
  volatile uint32_t MEMRMP;
  volatile uint32_t PMC;
  volatile uint32_t EXTICR[4];
} SYSCFG_TypeDef;





typedef struct
{
  volatile uint16_t CR1;
  uint16_t RESERVED0;
  volatile uint16_t CR2;
  uint16_t RESERVED1;
  volatile uint16_t OAR1;
  uint16_t RESERVED2;
  volatile uint16_t OAR2;
  uint16_t RESERVED3;
  volatile uint16_t DR;
  uint16_t RESERVED4;
  volatile uint16_t SR1;
  uint16_t RESERVED5;
  volatile uint16_t SR2;
  uint16_t RESERVED6;
  volatile uint16_t CCR;
  uint16_t RESERVED7;
  volatile uint16_t TRISE;
  uint16_t RESERVED8;
} I2C_TypeDef;





typedef struct
{
  volatile uint32_t KR;
  volatile uint32_t PR;
  volatile uint32_t RLR;
  volatile uint32_t SR;
} IWDG_TypeDef;






typedef struct
{
  volatile uint32_t CR;
  volatile uint32_t FCR;
  volatile uint32_t SR;
  volatile uint32_t CLR;
  uint32_t RESERVED;
  volatile uint32_t RAM[16];
} LCD_TypeDef;





typedef struct
{
  volatile uint32_t CR;
  volatile uint32_t CSR;
} PWR_TypeDef;





typedef struct
{
  volatile uint32_t CR;
  volatile uint32_t ICSCR;
  volatile uint32_t CFGR;
  volatile uint32_t CIR;
  volatile uint32_t AHBRSTR;
  volatile uint32_t APB2RSTR;
  volatile uint32_t APB1RSTR;
  volatile uint32_t AHBENR;
  volatile uint32_t APB2ENR;
  volatile uint32_t APB1ENR;
  volatile uint32_t AHBLPENR;
  volatile uint32_t APB2LPENR;
  volatile uint32_t APB1LPENR;
  volatile uint32_t CSR;
} RCC_TypeDef;





typedef struct
{
  volatile uint32_t ICR;
  volatile uint32_t ASCR1;
  volatile uint32_t ASCR2;
  volatile uint32_t HYSCR1;
  volatile uint32_t HYSCR2;
  volatile uint32_t HYSCR3;
  volatile uint32_t HYSCR4;
} RI_TypeDef;





typedef struct
{
  volatile uint32_t TR;
  volatile uint32_t DR;
  volatile uint32_t CR;
  volatile uint32_t ISR;
  volatile uint32_t PRER;
  volatile uint32_t WUTR;
  volatile uint32_t CALIBR;
  volatile uint32_t ALRMAR;
  volatile uint32_t ALRMBR;
  volatile uint32_t WPR;
  volatile uint32_t SSR;
  volatile uint32_t SHIFTR;
  volatile uint32_t TSTR;
  volatile uint32_t TSDR;
  volatile uint32_t TSSSR;
  volatile uint32_t CALR;
  volatile uint32_t TAFCR;
  volatile uint32_t ALRMASSR;
  volatile uint32_t ALRMBSSR;
  uint32_t RESERVED7;
  volatile uint32_t BKP0R;
  volatile uint32_t BKP1R;
  volatile uint32_t BKP2R;
  volatile uint32_t BKP3R;
  volatile uint32_t BKP4R;
  volatile uint32_t BKP5R;
  volatile uint32_t BKP6R;
  volatile uint32_t BKP7R;
  volatile uint32_t BKP8R;
  volatile uint32_t BKP9R;
  volatile uint32_t BKP10R;
  volatile uint32_t BKP11R;
  volatile uint32_t BKP12R;
  volatile uint32_t BKP13R;
  volatile uint32_t BKP14R;
  volatile uint32_t BKP15R;
  volatile uint32_t BKP16R;
  volatile uint32_t BKP17R;
  volatile uint32_t BKP18R;
  volatile uint32_t BKP19R;
  volatile uint32_t BKP20R;
  volatile uint32_t BKP21R;
  volatile uint32_t BKP22R;
  volatile uint32_t BKP23R;
  volatile uint32_t BKP24R;
  volatile uint32_t BKP25R;
  volatile uint32_t BKP26R;
  volatile uint32_t BKP27R;
  volatile uint32_t BKP28R;
  volatile uint32_t BKP29R;
  volatile uint32_t BKP30R;
  volatile uint32_t BKP31R;
} RTC_TypeDef;





typedef struct
{
  volatile uint32_t POWER;
  volatile uint32_t CLKCR;
  volatile uint32_t ARG;
  volatile uint32_t CMD;
  volatile const uint32_t RESPCMD;
  volatile const uint32_t RESP1;
  volatile const uint32_t RESP2;
  volatile const uint32_t RESP3;
  volatile const uint32_t RESP4;
  volatile uint32_t DTIMER;
  volatile uint32_t DLEN;
  volatile uint32_t DCTRL;
  volatile const uint32_t DCOUNT;
  volatile const uint32_t STA;
  volatile uint32_t ICR;
  volatile uint32_t MASK;
  uint32_t RESERVED0[2];
  volatile const uint32_t FIFOCNT;
  uint32_t RESERVED1[13];
  volatile uint32_t FIFO;
} SDIO_TypeDef;





typedef struct
{
  volatile uint16_t CR1;
  uint16_t RESERVED0;
  volatile uint16_t CR2;
  uint16_t RESERVED1;
  volatile uint16_t SR;
  uint16_t RESERVED2;
  volatile uint16_t DR;
  uint16_t RESERVED3;
  volatile uint16_t CRCPR;
  uint16_t RESERVED4;
  volatile uint16_t RXCRCR;
  uint16_t RESERVED5;
  volatile uint16_t TXCRCR;
  uint16_t RESERVED6;
  volatile uint16_t I2SCFGR;
  uint16_t RESERVED7;
  volatile uint16_t I2SPR;
  uint16_t RESERVED8;
} SPI_TypeDef;





typedef struct
{
  volatile uint16_t CR1;
  uint16_t RESERVED0;
  volatile uint16_t CR2;
  uint16_t RESERVED1;
  volatile uint16_t SMCR;
  uint16_t RESERVED2;
  volatile uint16_t DIER;
  uint16_t RESERVED3;
  volatile uint16_t SR;
  uint16_t RESERVED4;
  volatile uint16_t EGR;
  uint16_t RESERVED5;
  volatile uint16_t CCMR1;
  uint16_t RESERVED6;
  volatile uint16_t CCMR2;
  uint16_t RESERVED7;
  volatile uint16_t CCER;
  uint16_t RESERVED8;
  volatile uint32_t CNT;
  volatile uint16_t PSC;
  uint16_t RESERVED10;
  volatile uint32_t ARR;
  uint32_t RESERVED12;
  volatile uint32_t CCR1;
  volatile uint32_t CCR2;
  volatile uint32_t CCR3;
  volatile uint32_t CCR4;
  uint32_t RESERVED17;
  volatile uint16_t DCR;
  uint16_t RESERVED18;
  volatile uint16_t DMAR;
  uint16_t RESERVED19;
  volatile uint16_t OR;
  uint16_t RESERVED20;
} TIM_TypeDef;





typedef struct
{
  volatile uint16_t SR;
  uint16_t RESERVED0;
  volatile uint16_t DR;
  uint16_t RESERVED1;
  volatile uint16_t BRR;
  uint16_t RESERVED2;
  volatile uint16_t CR1;
  uint16_t RESERVED3;
  volatile uint16_t CR2;
  uint16_t RESERVED4;
  volatile uint16_t CR3;
  uint16_t RESERVED5;
  volatile uint16_t GTPR;
  uint16_t RESERVED6;
} USART_TypeDef;





typedef struct
{
  volatile uint32_t CR;
  volatile uint32_t CFR;
  volatile uint32_t SR;
} WWDG_TypeDef;
# 34 "B:\\otplatform\\stm32l1xx/platform_STM32L1xx.h" 2
# 192 "B:\\otplatform\\stm32l1xx/platform_STM32L1xx.h"
void HardFault_Handler(void);
void NMI_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
# 214 "B:\\otplatform\\stm32l1xx/platform_STM32L1xx.h"
void platform_isr_wwdg(void);
void platform_isr_pvd(void);
void platform_isr_tamper(void);
void platform_isr_rtcwkup(void);
void platform_isr_flash(void);
void platform_isr_rcc(void);
void platform_isr_exti0(void);
void platform_isr_exti1(void);
void platform_isr_exti2(void);
void platform_isr_exti3(void);
void platform_isr_exti4(void);
void platform_isr_dma1ch1(void);
void platform_isr_dma1ch2(void);
void platform_isr_dma1ch3(void);
void platform_isr_dma1ch4(void);
void platform_isr_dma1ch5(void);
void platform_isr_dma1ch6(void);
void platform_isr_dma1ch7(void);
void platform_isr_adc1(void);
void platform_isr_usbhp(void);
void platform_isr_usblp(void);
void platform_isr_dac(void);
void platform_isr_comp(void);
void platform_isr_exti5(void);
void platform_isr_exti6(void);
void platform_isr_exti7(void);
void platform_isr_exti8(void);
void platform_isr_exti9(void);
void platform_isr_lcd(void);
void platform_isr_tim9(void);
void platform_isr_tim10(void);
void platform_isr_tim11(void);
void platform_isr_tim2(void);
void platform_isr_tim3(void);
void platform_isr_tim4(void);
void platform_isr_i2c1ev(void);
void platform_isr_i2c1er(void);
void platform_isr_i2c2ev(void);
void platform_isr_i2c2er(void);
void platform_isr_spi1(void);
void platform_isr_spi2(void);
void platform_isr_usart1(void);
void platform_isr_usart2(void);
void platform_isr_usart3(void);
void platform_isr_exti10(void);
void platform_isr_exti11(void);
void platform_isr_exti12(void);
void platform_isr_exti13(void);
void platform_isr_exti14(void);
void platform_isr_exti15(void);
void platform_isr_rtcalarm(void);
void platform_isr_fswkup(void);
void platform_isr_tim6(void);
void platform_isr_tim7(void);


void platform_isr_tim5(void);
void platform_isr_spi3(void);
void platform_isr_dma2ch1(void);
void platform_isr_dma2ch2(void);
void platform_isr_dma2ch3(void);
void platform_isr_dma2ch4(void);
void platform_isr_dma2ch5(void);
void platform_isr_aes(void);
void platform_isr_compacq(void);


void platform_isr_sdio(void);
void platform_isr_uart4(void);
void platform_isr_uart5(void);
# 312 "B:\\otplatform\\stm32l1xx/platform_STM32L1xx.h"
typedef struct {
    ot_u8 disabled;
    ot_u8 taskid;
    ot_u16 mask;
    ot_u16 value;
} rtcalarm;



typedef struct {

    ot_u32 sstack[(160)/4];







    ot_u32 utc;





    void* task_exit;
    ot_u16 last_evt;
    ot_u16 next_evt;


    ot_u16 cpu_khz;
    ot_u16 prand_reg;
    ot_u16 crc16;

} platform_ext_struct;


extern platform_ext_struct platform_ext;
# 39 "B:\\board/stm32l1xx/board_IKR001.h" 2




# 1 "B:\\otradio\\spirit1/radio_SPIRIT1.h" 1
# 176 "B:\\otradio\\spirit1/radio_SPIRIT1.h"
typedef struct {
    ot_u8 state;
    ot_u8 flags;
    ot_u16 nextcal;
    ot_int txlimit;
    ot_int rxlimit;
} rfctl_struct;

extern rfctl_struct rfctl;
# 44 "B:\\board/stm32l1xx/board_IKR001.h" 2
# 457 "B:\\board/stm32l1xx/board_IKR001.h"
inline void BOARD_PERIPH_INIT(void) {

    ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->AHBENR |= (((uint32_t)0x01000000) | ((uint32_t)0x00008000) | 0 | ( ((uint32_t)0x00000001) | ((uint32_t)0x00000002) | ((uint32_t)0x00000004) | ((uint32_t)0x00000008) | ((uint32_t)0x00000010) ));


    ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->AHBLPENR = (((uint32_t)0x01000000) | ((uint32_t)0x00010000) | ((uint32_t)0x00008000) | 0 | ( ((uint32_t)0x00000001) | ((uint32_t)0x00000002) | ((uint32_t)0x00000004) | ((uint32_t)0x00000008) | ((uint32_t)0x00000010) ));





    ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->APB2ENR |= ((uint32_t)0x00000001);






    ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->APB1ENR |= ((uint32_t)0x10000000);
}



inline void BOARD_DMA_CLKON(void) {
# 489 "B:\\board/stm32l1xx/board_IKR001.h"
}

inline void BOARD_DMA_CLKOFF(void) {
# 500 "B:\\board/stm32l1xx/board_IKR001.h"
}
# 519 "B:\\board/stm32l1xx/board_IKR001.h"
inline void BOARD_EXTI_STARTUP(void) {




    ((SYSCFG_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0000))->EXTICR[2] |= ( (4 << 4)
                          | (4 << 8)
                          | (2 << 12) );


    ((SYSCFG_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0000))->EXTICR[3] |= ( (2 << 0)
                          | (2 << 4)
                          | (2 << 8) );



}





inline void BOARD_PORT_STARTUP(void) {
# 590 "B:\\board/stm32l1xx/board_IKR001.h"
    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0000))->MODER |= (((uint32_t)0x00000003) << (5*2));



    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0800))->MODER |= ( (((uint32_t)0x00000003) << (5*2))
                                  | (((uint32_t)0x00000003) << (4*2)) );




        ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0000))->MODER |= ( (((uint32_t)0x00000001) << (0*2))
                                  | (((uint32_t)0x00000001) << (1*2))
                                  | (((uint32_t)0x00000002) << (3*2))
                                  | (((uint32_t)0x00000002) << (2*2)) );

            ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0000))->AFR[0] |= ( (7 << (3*4))
                                       | (7 << (2*4)) );
# 627 "B:\\board/stm32l1xx/board_IKR001.h"
        ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x1400))->MODER = ( (((uint32_t)0x00000001) << (0*2)) | (((uint32_t)0x00000001) << (1*2))
                                  | (((uint32_t)0x00000001) << (2*2)) );
        ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0000))->MODER |= ( (((uint32_t)0x00000001) << (11*2))
                                  | (((uint32_t)0x00000001) << (12*2)) );




    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0400))->OTYPER |= ( (1<<10) | (1<<11) );
    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0400))->OSPEEDR |= ( (((uint32_t)0x00000001) << (10*2))
                                  | (((uint32_t)0x00000001) << (11*2)) );
    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0400))->MODER |= ( (((uint32_t)0x00000002) << (10*2))
                                  | (((uint32_t)0x00000002) << (11*2)) );

        ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0400))->AFR[1] |= ( (4 << ((10 -8)*4))
                                  | (4 << ((11 -8)*4)) );






    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0000))->MODER |= (((uint32_t)0x00000001) << (9*2));
    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x1000))->MODER|= (((uint32_t)0x00000001) << (10*2));
    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0400))->MODER |= ( (((uint32_t)0x00000001) << (15*2))
                                  | (((uint32_t)0x00000001) << (14*2))
                                  | (((uint32_t)0x00000001) << (13*2))
                                  | (((uint32_t)0x00000001) << (12*2)) );




    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0000))->MODER |= (((uint32_t)0x00000001) << (8*2));




    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0800))->MODER |= (((uint32_t)0x00000001) << (13*2));
    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0800))->OSPEEDR |= (((uint32_t)0x00000001) << (13*2));
    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x1000))->MODER |= ( (((uint32_t)0x00000002) << (15*2))
                                  | (((uint32_t)0x00000002) << (14*2))
                                  | (((uint32_t)0x00000002) << (13*2))
                                  | (((uint32_t)0x00000002) << (12*2)) );
    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x1000))->OSPEEDR |= ( (((uint32_t)0x00000002) << (15*2))
                                  | (((uint32_t)0x00000002) << (14*2))
                                  | (((uint32_t)0x00000002) << (13*2))
                                  | (((uint32_t)0x00000002) << (12*2)) );
    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x1000))->AFR[1] |= ( (5 << ((15 -8)*4))
                                  | (5 << ((14 -8)*4))
                                  | (5 << ((13 -8)*4))
                                  | (5 << ((12 -8)*4)) );




    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x1000))->MODER |= ( (((uint32_t)0x00000002) << (4*2))
                                  | (((uint32_t)0x00000002) << (3*2)) );





    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x1000))->MODER |= (((uint32_t)0x00000002) << (5*2));
    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x1000))->MODER |= (((uint32_t)0x00000002) << (6*2));
    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x1000))->AFR[0] |= ( (3 << (5*4)) );
    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x1000))->AFR[0] |= ( (3 << (6*4)) );




    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0000))->MODER |= ( (((uint32_t)0x00000001) << (4*2)) | (((uint32_t)0x00000001) << (6*2))
                      | (((uint32_t)0x00000001) << (7*2)) | (((uint32_t)0x00000001) << (10*2)) );

    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0400))->MODER |= (((uint32_t)0x00000001) << (5*2));

    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0C00))->MODER |= ( (((uint32_t)0x00000001) << (5*2)) | (((uint32_t)0x00000001) << (6*2))
                      | (((uint32_t)0x00000001) << (7*2)) | (((uint32_t)0x00000001) << (8*2))
                      | (((uint32_t)0x00000001) << (9*2))
                      | (((uint32_t)0x00000001) << (10*2))| (((uint32_t)0x00000001) << (11*2))
                      | (((uint32_t)0x00000001) << (12*2))| (((uint32_t)0x00000001) << (13*2))
                      | (((uint32_t)0x00000001) << (14*2))| (((uint32_t)0x00000001) << (15*2)) );

    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x1000))->MODER |= ( (((uint32_t)0x00000001) << (0*2)) | (((uint32_t)0x00000001) << (1*2))
                      | (((uint32_t)0x00000001) << (2*2)) | (((uint32_t)0x00000001) << (11*2)) );


}



inline void BOARD_PORT_STANDBY() {
# 741 "B:\\board/stm32l1xx/board_IKR001.h"
        ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0000))->MODER |= ( (((uint32_t)0x00000001) << (0*2))
                                  | (((uint32_t)0x00000001) << (1*2))
                                  | (((uint32_t)0x00000002) << (3*2))
                                  | (((uint32_t)0x00000002) << (2*2)) );
# 771 "B:\\board/stm32l1xx/board_IKR001.h"
    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x1000))->MODER |= ( (((uint32_t)0x00000002) << (15*2))
                                  | (((uint32_t)0x00000002) << (14*2))
                                  | (((uint32_t)0x00000002) << (13*2))
                                  | (((uint32_t)0x00000002) << (12*2)) );
    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x1000))->OSPEEDR |= ( (((uint32_t)0x00000002) << (15*2))
                                  | (((uint32_t)0x00000002) << (14*2))
                                  | (((uint32_t)0x00000002) << (13*2))
                                  | (((uint32_t)0x00000002) << (12*2)) );
    ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x1000))->AFR[1] |= ( (5 << ((15 -8)*4))
                                  | (5 << ((14 -8)*4))
                                  | (5 << ((13 -8)*4))
                                  | (5 << ((12 -8)*4)) );
# 795 "B:\\board/stm32l1xx/board_IKR001.h"
}






inline void BOARD_POWER_STARTUP(void) {





}
# 817 "B:\\board/stm32l1xx/board_IKR001.h"
inline void BOARD_XTAL_STARTUP(void) {


}
# 77 "B:\\apps\\demo_ponglt\\code/platform_config.h" 2
# 39 "B:\\otlib/OT_platform.h" 2




typedef struct {
    ot_int error_code;
} platform_struct;

extern platform_struct platform;


void platform_rtc_isr();
# 65 "B:\\otlib/OT_platform.h"
void platform_poweron();
# 76 "B:\\otlib/OT_platform.h"
void platform_poweroff();
# 90 "B:\\otlib/OT_platform.h"
void platform_init_OT();







void platform_disable_interrupts();







void platform_enable_interrupts();
# 125 "B:\\otlib/OT_platform.h"
void platform_standard_speed();
# 139 "B:\\otlib/OT_platform.h"
void platform_full_speed();
# 158 "B:\\otlib/OT_platform.h"
void platform_flank_speed();
# 197 "B:\\otlib/OT_platform.h"
void platform_drop_context(ot_uint task_id);
# 212 "B:\\otlib/OT_platform.h"
void platform_ot_pause();
# 225 "B:\\otlib/OT_platform.h"
void platform_ot_run();
# 239 "B:\\otlib/OT_platform.h"
void platform_ot_preempt();
# 256 "B:\\otlib/OT_platform.h"
void platform_init_busclk();
# 272 "B:\\otlib/OT_platform.h"
void platform_init_periphclk();
# 283 "B:\\otlib/OT_platform.h"
void platform_init_interruptor();
# 294 "B:\\otlib/OT_platform.h"
void platform_init_gpio();
# 309 "B:\\otlib/OT_platform.h"
    void platform_init_gptim(ot_u16 prescaler);



void platform_init_watchdog();
# 323 "B:\\otlib/OT_platform.h"
void platform_init_resetswitch();
# 342 "B:\\otlib/OT_platform.h"
void platform_init_systick(ot_uint period);
# 353 "B:\\otlib/OT_platform.h"
void platform_init_rtc(ot_u32 value);


void platform_init_memcpy();


void platform_init_prand(ot_u16 seed);
# 377 "B:\\otlib/OT_platform.h"
ot_u16 platform_get_ktim();







ot_u16 platform_next_ktim();


void platform_pend_ktim();







void platform_enable_ktim();


void platform_disable_ktim();







void platform_set_ktim(ot_u16 value);

void platform_set_gptim2(ot_u16 value);
void platform_set_gptim3(ot_u16 value);
void platform_set_gptim4(ot_u16 value);
void platform_set_gptim5(ot_u16 value);







void platform_flush_ktim();


void platform_run_watchdog();


void platform_reset_watchdog(ot_u16 reset);


void platform_enable_rtc();
void platform_disable_rtc();
ot_u32 platform_get_time();
void platform_set_time();
# 453 "B:\\otlib/OT_platform.h"
void platform_set_rtc_alarm(ot_u8 alarm_id, ot_u8 task_id, ot_u16 offset);
# 465 "B:\\otlib/OT_platform.h"
void platform_clear_rtc_alarms();
# 474 "B:\\otlib/OT_platform.h"
void platform_trig1_high();
void platform_trig1_low();
void platform_trig1_toggle();
void platform_trig2_high();
void platform_trig2_low();
void platform_trig2_toggle();
# 512 "B:\\otlib/OT_platform.h"
void platform_rand(ot_u8* rand_out, ot_int bytes_out);
# 523 "B:\\otlib/OT_platform.h"
ot_u8 platform_prand_u8();
# 534 "B:\\otlib/OT_platform.h"
ot_u16 platform_prand_u16();
# 547 "B:\\otlib/OT_platform.h"
void platform_memcpy(ot_u8* dest, ot_u8* src, ot_int length);
# 560 "B:\\otlib/OT_platform.h"
void platform_memcpy_2(ot_u16* dest, ot_u16* src, ot_int length);
# 573 "B:\\otlib/OT_platform.h"
void platform_memset(ot_u8* dest, ot_u8 value, ot_int length);
# 599 "B:\\otlib/OT_platform.h"
void platform_block(ot_u16 sti);
# 615 "B:\\otlib/OT_platform.h"
void platform_delay(ot_uint n);
# 625 "B:\\otlib/OT_platform.h"
void platform_swdelay_ms(ot_uint n);
# 635 "B:\\otlib/OT_platform.h"
void platform_swdelay_us(ot_uint n);
# 35 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c" 2


# 1 "B:\\otlib/veelite.h" 1
# 44 "B:\\otlib/veelite.h"
# 1 "B:\\otlib/OT_utils.h" 1
# 45 "B:\\otlib/veelite.h" 2
# 1 "B:\\otlib/OTAPI.h" 1
# 34 "B:\\otlib/OTAPI.h"
# 1 "B:\\otlib/OTAPI_tmpl.h" 1
# 59 "B:\\otlib/OTAPI_tmpl.h"
typedef struct {
    ot_u8 block;
    ot_u8 id;
} vladdr_tmpl;


typedef struct {
    ot_u8 block;
    ot_u8 id;
    ot_u8 permissions;
    ot_u8 is_mirrored;
    ot_u16 length;
    ot_u16 alloc;
} vlheader_tmpl;


typedef struct {
    ot_u16 offset;
    ot_u16 bytes;
    ot_u8* data;
} vldata_tmpl;







typedef struct {
    ot_u8 reserved;
    ot_u8 channel;
    ot_u8 subnet;
    ot_u8 subnetmask;
    ot_u8 flags;
    ot_u8 flagmask;
} session_tmpl;






typedef enum {
    ADDR_unicast = 0x00,
    ADDR_broadcast = 0x40,
    ADDR_anycast = 0x80,
    ADDR_multicast = 0xC0
} addr_type;



typedef struct {
    ot_u8 duty_off;
    ot_u8 duty_on;
    ot_u8 reserved;
    ot_u8 channel;
    ot_u16 duration;
} advert_tmpl;



typedef struct {
    ot_u16 length;
    ot_u8* list;
} chanlist_tmpl;



typedef struct {
    ot_u8 csma_type;
    ot_u16 csma_guard_us;
    ot_u16 csma_guess_us;
    ot_u16 csma_timeout;
} csma_tmpl;



typedef struct {
    ot_u8 length;
    ot_u8* value;
} id_tmpl;



typedef struct {
    ot_u8 idcount;
    ot_u8 listlen;
    ot_u8* idlist;
} idlist_tmpl;



typedef struct {
    ot_u8 hop_code;
    ot_u8 hop_ext;
    id_tmpl dlog;
    id_tmpl orig;
    id_tmpl dest;
} routing_tmpl;
# 178 "B:\\otlib/OTAPI_tmpl.h"
typedef struct {
    ot_u8 count;
    ot_u8 length;
    ot_u8* list;
} ack_tmpl;



typedef struct {
    ot_u8 type;
    ot_u8 opcode;
    ot_u8 extension;
} command_tmpl;


typedef enum {
    CMDEXT_none = 0,
    CMDEXT_no_response = 0x02,
    CMDEXT_no_csma = 0x04,
    CMDEXT_ca_raind = (1<<3),
    CMDEXT_ca_aind = (2<<3),
    CMDEXT_ca_mac = (7<<3)
} command_extensions;


typedef enum {
    CMD_announce_file = 0,
    CMD_announce_series = 1,
    CMD_inventory_on_file = 2,
    CMD_inventory_on_series = 3,
    CMD_udp_on_file = 4,
    CMD_udp_on_series = 5,
    CMD_collect_file_on_file = 6,
    CMD_collect_series_on_file = 7,
    CMD_collect_file_on_series = 8,
    CMD_collect_series_on_series = 9,
    CMD_request_datastream = 12,
    CMD_propose_datastream = 13,
    CMD_ack_datastream = 14,
    CMD_reserved = 15
} command_opcodes;


typedef enum {
    CMDTYPE_response = 0,
    CMDTYPE_error = (1 << 4),
    CMDTYPE_na2p_request = (2 << 4),
    CMDTYPE_a2p_init_request = (4 << 4),
    CMDTYPE_a2p_inter_request = (5 << 4),
    CMDTYPE_a2p_final_request = (7 << 4)
} command_types;



typedef struct {
    ot_u8 code;
    ot_u8 subcode;
    ot_u8* data;
} error_tmpl;



typedef struct {
    ot_u16 timeout;
    ot_u16 channels;
    ot_u8* chanlist;
} dialog_tmpl;



typedef struct {
    ot_u16 data_condition;
    ot_u8 length;
    ot_u8 pid;
    ot_u8* data;
} dp_tmpl;



typedef struct {
    ot_u8 is_series;
    ot_u8 isf_id;
    ot_s16 offset;
    ot_s16 max_return;
} isfcall_tmpl;



typedef struct {
    ot_u8 is_series;
    ot_u8 isf_id;
    ot_s16 offset;
} isfcomp_tmpl;



typedef struct {
    ot_u8 isf_id;
    ot_u8 offset;
    ot_u8 length;
    ot_u8* data;
} isfreturn_tmpl;



typedef struct {
    ot_u8 isfs_id;
    ot_u8 series_length;
    ot_s16 contents_offset;
    ot_s16 content_length;
    ot_u8* series_data;
    ot_u8* contents_data;
} isfseriesreturn_tmpl;


typedef enum {
    QCODE_ismasked = 0x80,
    QCODE_nonnull = 0,
    QCODE_notequal = 0x20,
    QCODE_equal = 0x21,
    QCODE_lessthan = 0x22,
    QCODE_lessthan_equal = 0x23,
    QCODE_greaterthan = 0x24,
    QCODE_greaterthan_equal = 0x25,
    QCODE_search = 0x26
} query_codes;



typedef struct {
    ot_u8 code;
    ot_u8 length;
    ot_u8* mask;
    ot_u8* value;
} query_tmpl;



typedef struct {
    ot_u8 src_port;
    ot_u8 dst_port;
    ot_u8 data_length;
    ot_u8* data;
} udp_tmpl;
# 35 "B:\\otlib/OTAPI.h" 2



# 1 "B:\\otlib/auth.h" 1
# 38 "B:\\otlib/auth.h"
# 1 "B:\\otlib/OTAPI.h" 1
# 39 "B:\\otlib/auth.h" 2
# 54 "B:\\otlib/auth.h"
extern const id_tmpl* auth_guest;
# 67 "B:\\otlib/auth.h"
typedef struct {
    ot_u8 mod;
    ot_u8 protocol;
    ot_u32 lifetime;
    id_tmpl* id;
    ot_u8* key;
} auth_entry;
# 85 "B:\\otlib/auth.h"
void auth_init();
# 94 "B:\\otlib/auth.h"
ot_bool auth_isroot(id_tmpl* user_id);
# 105 "B:\\otlib/auth.h"
ot_u8 auth_check(ot_u8 data_mod, ot_u8 req_mod, id_tmpl* user_id);
# 118 "B:\\otlib/auth.h"
auth_entry* auth_new_nlsuser(auth_entry* new_user, ot_u8* new_data);
# 128 "B:\\otlib/auth.h"
auth_entry* auth_search_user(id_tmpl* user_id, ot_u8 mod_flags);
# 39 "B:\\otlib/OTAPI.h" 2
# 1 "B:\\otlib/buffers.h" 1
# 33 "B:\\otlib/buffers.h"
# 1 "B:\\otlib/queue.h" 1
# 59 "B:\\otlib/queue.h"
typedef struct {
    ot_u16 alloc;
    ot_u16 length;
    ot_uni16 options;
    ot_u8* front;
    ot_u8* back;
    ot_u8* getcursor;
    ot_u8* putcursor;
} ot_queue;



typedef ot_queue Queue;
# 82 "B:\\otlib/queue.h"
void q_init(ot_queue* q, ot_u8* buffer, ot_u16 alloc);
# 93 "B:\\otlib/queue.h"
void q_rebase(ot_queue* q, ot_u8* buffer);



void q_copy(ot_queue* q1, Queue* q2);
# 106 "B:\\otlib/queue.h"
void q_empty(ot_queue* q);
# 116 "B:\\otlib/queue.h"
ot_u8* q_start(ot_queue* q, ot_uint offset, ot_u16 options);
# 126 "B:\\otlib/queue.h"
ot_u8* q_markbyte(ot_queue* q, ot_int shift);
# 135 "B:\\otlib/queue.h"
void q_writebyte(ot_queue* q, ot_u8 byte_in);
# 148 "B:\\otlib/queue.h"
void q_writeshort(ot_queue* q, ot_u16 short_in);
void q_writeshort_be(ot_queue* q, ot_u16 short_in);
# 163 "B:\\otlib/queue.h"
void q_writelong(ot_queue* q, ot_u32 long_in);
# 172 "B:\\otlib/queue.h"
ot_u8 q_readbyte(ot_queue* q);
# 184 "B:\\otlib/queue.h"
ot_u16 q_readshort(ot_queue* q);
ot_u16 q_readshort_be(ot_queue* q);
# 197 "B:\\otlib/queue.h"
ot_u32 q_readlong(ot_queue* q);


void q_writestring(ot_queue* q, ot_u8* string, ot_int length);
void q_readstring(ot_queue* q, ot_u8* string, ot_int length);
# 34 "B:\\otlib/buffers.h" 2
# 66 "B:\\otlib/buffers.h"
extern ot_u8 otbuf[1024];





    extern Queue rxq;
    extern Queue txq;







    extern Queue dir_in;
    extern Queue dir_out;
# 98 "B:\\otlib/buffers.h"
void buffers_init();
# 40 "B:\\otlib/OTAPI.h" 2

# 1 "B:\\otlib/m2_dll.h" 1
# 35 "B:\\otlib/m2_dll.h"
# 1 "B:\\otlib/system.h" 1
# 43 "B:\\otlib/system.h"
# 1 "B:\\otkernel/hicculp/system_hicculp.h" 1
# 109 "B:\\otkernel/hicculp/system_hicculp.h"
typedef struct task_marker_struct {
    ot_u8 event;
    ot_u8 cursor;
    ot_u8 reserve;
    ot_u8 latency;
    ot_long nextevent;






} task_marker;


typedef task_marker* ot_task;
typedef task_marker ot_task_struct;

typedef void (*systask_fn)(ot_task);
# 44 "B:\\otlib/system.h" 2
# 67 "B:\\otlib/system.h"
typedef enum {
    TASK_idle = -1,

    TASK_radio = 0,


    TASK_mpipe,





    TASK_hold,


    TASK_sleep,


    TASK_beacon,


    TASK_external,




    TASK_terminus
} Task_Index;


typedef enum {
    HALT_off = 0,
    HALT_standby,
    HALT_nopower,
    HALT_lowpower,
    HALT_terminus
} Halt_Request;
# 122 "B:\\otlib/system.h"
typedef Task_Index ot_task_handle;




typedef struct {
# 136 "B:\\otlib/system.h"
    ot_task_handle active;
    ot_task_struct task[TASK_terminus];
} sys_struct;

extern sys_struct sys;
# 220 "B:\\otlib/system.h"
void sys_init();
# 243 "B:\\otlib/system.h"
void sys_panic(ot_u8 err_code);
# 265 "B:\\otlib/system.h"
void sys_powerdown();
# 290 "B:\\otlib/system.h"
void sys_halt(Halt_Request halt_request);
# 305 "B:\\otlib/system.h"
void sys_resume();
# 320 "B:\\otlib/system.h"
void sys_task_enable(ot_u8 task_id, ot_u8 task_ctrl, ot_u16 sleep);
# 332 "B:\\otlib/system.h"
void sys_task_disable(ot_u8 task_id);
# 354 "B:\\otlib/system.h"
ot_uint sys_event_manager();
# 376 "B:\\otlib/system.h"
void sys_task_manager();
# 395 "B:\\otlib/system.h"
void sys_run_task();





void sys_kill_all();


void sys_kill_active();
# 417 "B:\\otlib/system.h"
void sys_preempt(ot_task task, ot_uint nextevent_ti);






void sys_task_setevent(ot_task task, ot_u8 event);
void sys_task_setcursor(ot_task task, ot_u8 cursor);
void sys_task_setreserve(ot_task task, ot_u8 reserve);
void sys_task_setlatency(ot_task task, ot_u8 latency);
void sys_task_setnext(ot_task task, ot_u16 nextevent_ti);
void sys_task_setnext_clocks(ot_task task, ot_long nextevent_clocks);
# 444 "B:\\otlib/system.h"
void sys_synchronize(Task_Index task_id);







void sys_refresh_scheduler();
# 476 "B:\\otlib/system.h"
void sys_sig_panic(ot_int code);
# 490 "B:\\otlib/system.h"
void sys_sig_powerdown(ot_int code);
# 503 "B:\\otlib/system.h"
void sys_sig_extprocess(void* event_data);
# 36 "B:\\otlib/m2_dll.h" 2
# 150 "B:\\otlib/m2_dll.h"
typedef struct {
    ot_u8 subnet;
    ot_u8 b_subnet;
    ot_u8 dd_flags;
    ot_u8 b_attempts;
    ot_u16 active;
    ot_u16 hold_limit;
} netconf_struct;
# 259 "B:\\otlib/m2_dll.h"
typedef struct {
    ot_long tc;
    ot_long tca;
    ot_uint rx_timeout;
    ot_u8 csmaca_params;
    ot_u8 redundants;
    ot_u8 tx_channels;
    ot_u8 rx_channels;
    ot_u8* tx_chanlist;
    ot_u8* rx_chanlist;
    ot_u8 scratch[2];
} m2comm_struct;

typedef struct {
    ot_sig rfinit;
    ot_sig2 rfterminate;
} dll_sig_struct;

typedef struct {
    ot_u8 idle_state;
    ot_u8 reserved;
    ot_uint counter;
    ot_uint adv_time;
    ot_int last_nrssi;
    netconf_struct netconf;
    m2comm_struct comm;



} m2dll_struct;

extern m2dll_struct dll;
# 303 "B:\\otlib/m2_dll.h"
void dll_block();
void dll_clock(ot_uint ticks);
void dll_next();
void dll_systask_init();
void dll_systask_rf(ot_task task_dll);






void dll_systask_holdscan();
void dll_systask_sleepscan();
void dll_systask_beacon();


void dll_sig_rfinit(ot_int pcode);
void dll_sig_rfterminate(ot_int pcode, ot_int scode);
# 334 "B:\\otlib/m2_dll.h"
void dll_init();
# 347 "B:\\otlib/m2_dll.h"
void dll_refresh();
# 361 "B:\\otlib/m2_dll.h"
void dll_change_settings(ot_u16 new_mask, ot_u16 new_settings);
# 372 "B:\\otlib/m2_dll.h"
void dll_goto_off();
# 386 "B:\\otlib/m2_dll.h"
void dll_idle();
# 398 "B:\\otlib/m2_dll.h"
void dll_set_defaults(m2session* session);
# 414 "B:\\otlib/m2_dll.h"
ot_u8 dll_default_csma(ot_u8 chan_id);







void dll_quit_rf();
# 42 "B:\\otlib/OTAPI.h" 2
# 1 "B:\\otlib/m2_network.h" 1
# 39 "B:\\otlib/m2_network.h"
# 1 "B:\\otlib/alp.h" 1
# 59 "B:\\otlib/alp.h"
typedef enum {
    MSG_Null = 0,
    MSG_Chunking_Out= 2,
    MSG_Chunking_In = 4,
    MSG_End = 6
} ALP_status;





typedef struct {
    ot_u8 flags;
    ot_u8 plength;
    ot_u8 id;
    ot_u8 cmd;
    void* bookmark;
} alp_record;

typedef struct {
    alp_record inrec;
    alp_record outrec;
    Queue* inq;
    Queue* outq;
} alp_tmpl;







void alp_init(alp_tmpl* alp, ot_queue* inq, ot_queue* outq);
# 110 "B:\\otlib/alp.h"
void alp_break(alp_tmpl* alp);
# 137 "B:\\otlib/alp.h"
void alp_new_record(alp_tmpl* alp, ot_u8 flags, ot_u8 payload_limit, ot_int payload_remaining);
# 155 "B:\\otlib/alp.h"
void alp_new_message(alp_tmpl* alp, ot_u8 payload_limit, ot_int payload_remaining);
# 187 "B:\\otlib/alp.h"
ALP_status alp_parse_message(alp_tmpl* alp, id_tmpl* user_id);
# 203 "B:\\otlib/alp.h"
ot_bool alp_parse_header(alp_tmpl* alp);
# 216 "B:\\otlib/alp.h"
ot_bool alp_load_retval(alp_tmpl* alp, ot_u16 retval);
# 241 "B:\\otlib/alp.h"
ot_bool alp_proc(alp_tmpl* alp, id_tmpl* user_id);
# 259 "B:\\otlib/alp.h"
ot_bool alp_proc_filedata(alp_tmpl* alp, id_tmpl* user_id);
# 303 "B:\\otlib/alp.h"
ot_bool alp_proc_logger(alp_tmpl* alp, id_tmpl* user_id);
# 322 "B:\\otlib/alp.h"
ot_bool alp_proc_api_session(alp_tmpl* alp, id_tmpl* user_id);







ot_bool alp_proc_api_system(alp_tmpl* alp, id_tmpl* user_id);







ot_bool alp_proc_api_query(alp_tmpl* alp, id_tmpl* user_id);
# 40 "B:\\otlib/m2_network.h" 2
# 102 "B:\\otlib/m2_network.h"
typedef struct {
    ot_u8 fr_info;
    ot_u8 addr_ctl;
} header_struct;


typedef struct {
    ot_sigv2 route;
} m2npsig_struct;


typedef struct {
    routing_tmpl rt;
    header_struct header;



} m2np_struct;


typedef struct {
    ot_u8 ctl;


    ot_u8 dmg_count;

} dscfg_struct;


typedef struct {
    dscfg_struct dscfg;
} m2dp_struct;





extern m2np_struct m2np;
# 162 "B:\\otlib/m2_network.h"
void network_init();







m2session* network_parse_bf();
# 184 "B:\\otlib/m2_network.h"
void network_mark_ff();
# 199 "B:\\otlib/m2_network.h"
ot_int network_route_ff(m2session* session);
# 218 "B:\\otlib/m2_network.h"
void network_sig_route(void* route, void* session);
# 242 "B:\\otlib/m2_network.h"
void m2np_header(m2session* session, ot_u8 addressing, ot_u8 nack);
# 255 "B:\\otlib/m2_network.h"
void m2np_footer();
# 267 "B:\\otlib/m2_network.h"
void m2np_put_deviceid(ot_bool use_vid);
# 284 "B:\\otlib/m2_network.h"
ot_bool m2np_idcmp(ot_int length, void* id);
# 309 "B:\\otlib/m2_network.h"
void m2advp_open(m2session* session);
# 323 "B:\\otlib/m2_network.h"
void m2advp_close();
# 336 "B:\\otlib/m2_network.h"
void m2advp_update(ot_u16 countdown);
# 399 "B:\\otlib/m2_network.h"
void m2dp_append();
# 408 "B:\\otlib/m2_network.h"
void m2dp_footer();
# 43 "B:\\otlib/OTAPI.h" 2
# 1 "B:\\otlib/m2_transport.h" 1
# 114 "B:\\otlib/m2_transport.h"
typedef struct {
    ot_u8 code;
    ot_u8 ext;
} cmd_data;

typedef struct {
    ot_u16 sequence;
    ot_u8 fpp;
    ot_u8 srcport;
} ds_data;
# 133 "B:\\otlib/m2_transport.h"
typedef union {
    ot_s32 slong[4];
    ot_u32 ulong[4];
    ot_s16 sshort[8];
    ot_u16 ushort[8];
    ot_u8 ubyte[16];
} dstr_16;

typedef union {
    ot_s32 slong[2];
    ot_u32 ulong[2];
    ot_s16 sshort[4];
    ot_u16 ushort[4];
    ot_u8 ubyte[8];
} dstr_8;

typedef struct {
    ot_u8 comp_id;
    ot_int comp_offset;
} query_data;
# 166 "B:\\otlib/m2_transport.h"
typedef ot_bool (*ot_m2qpsig)(ot_u8, ot_u8, id_tmpl*);





typedef struct {

    ot_m2qpsig isf;
# 186 "B:\\otlib/m2_transport.h"
    ot_m2qpsig error;


    ot_m2qpsig a2p;

}
m2qp_sigs;






typedef struct {
    cmd_data cmd;
    query_data qdata;
    query_tmpl qtmpl;






} m2qp_struct;


extern m2qp_struct m2qp;
# 232 "B:\\otlib/m2_transport.h"
ot_int m2qp_put_beacon(ot_u8 cmd_code, ot_u8 cmd_ext, Queue* input);
# 245 "B:\\otlib/m2_transport.h"
void m2qp_put_na2ptmpl( ot_u16 rx_timeout,
                        ot_u8 rx_channels,
                        ot_u8* rx_chanlist);



void m2qp_put_a2ptmpl( ot_u16 rx_timeout,
                        ot_u8 csma_guard,
                        ot_u8 rx_channels,
                        ot_u8* rx_chanlist);



void m2qp_set_suppliedid( ot_bool vid, ot_u8* supplied_addr );



ot_int m2qp_put_isfs( ot_u8* isf_template );
ot_int m2qp_put_isf( ot_u8 isf_id, ot_u8 offset, ot_u16 max_length );
# 289 "B:\\otlib/m2_transport.h"
ot_bool m2qp_sigresp_null(id_tmpl* responder_id, ot_int payload_length, ot_u8* payload);
# 302 "B:\\otlib/m2_transport.h"
void m2qp_init();
# 314 "B:\\otlib/m2_transport.h"
ot_int m2qp_parse_frame(m2session* session);
# 325 "B:\\otlib/m2_transport.h"
void m2qp_parse_dspkt(m2session* session);
# 335 "B:\\otlib/m2_transport.h"
void m2qp_mark_dsframe();
# 346 "B:\\otlib/m2_transport.h"
ot_bool m2qp_sig_isf( ot_u8 type, ot_u8 opcode, id_tmpl* user_id );
ot_bool m2qp_sig_udp( ot_u8 srcport, ot_u8 dstport, id_tmpl* user_id );
ot_bool m2qp_sig_error( ot_u8 code, ot_u8 subcode, id_tmpl* user_id );
ot_bool m2qp_sig_a2p( ot_u8 code, ot_u8 subcode, id_tmpl* user_id );


ot_bool m2qp_sig_dspkt( ot_u8 code, ot_u8 subcode, id_tmpl* user_id );
ot_bool m2qp_sig_dsack( ot_u8 code, ot_u8 subcode, id_tmpl* user_id );
# 394 "B:\\otlib/m2_transport.h"
ot_int m2qp_isf_comp(ot_u8 is_series, id_tmpl* user_id);
# 415 "B:\\otlib/m2_transport.h"
ot_int m2qp_isf_call( ot_u8 is_series, Queue* input_q, id_tmpl* user_id );
# 444 "B:\\otlib/m2_transport.h"
ot_int m2qp_load_isf( ot_u8 is_series,
                        ot_u8 isf_id,
                        ot_int offset,
                        ot_int window_bytes,
                        ot_int (*load_function)(ot_int*, ot_u8),
                        id_tmpl* user_id );
# 44 "B:\\otlib/OTAPI.h" 2
# 1 "B:\\otlib/radio.h" 1
# 50 "B:\\otlib/radio.h"
typedef enum {
    RADIO_Idle = 0,
    RADIO_Listening = 1,
    RADIO_Csma = 2,
    RADIO_DataRX = 5,
    RADIO_DataTX = 6
} radio_state;







typedef struct {
    radio_state state;
    ot_int last_rssi;
    ot_sig2 evtdone;
} radio_struct;

extern radio_struct radio;
# 115 "B:\\otlib/radio.h"
typedef struct {
    ot_int tg;
    ot_u8 channel;
    ot_u8 autoscale;
    ot_u8 tx_eirp;
    ot_u8 link_qual;
    ot_u8 cs_thr;
    ot_u8 cca_thr;
}
phymac_struct;
# 137 "B:\\otlib/radio.h"
extern phymac_struct phymac[1];
# 150 "B:\\otlib/radio.h"
    extern ot_int air_i;
    extern ot_u8 air[520];
# 229 "B:\\otlib/radio.h"
void radio_init();
# 245 "B:\\otlib/radio.h"
void radio_mac_isr();
# 263 "B:\\otlib/radio.h"
ot_int radio_rssi();
# 276 "B:\\otlib/radio.h"
ot_u8 radio_buffer(ot_int index);
# 302 "B:\\otlib/radio.h"
void radio_off();
# 316 "B:\\otlib/radio.h"
void radio_gag();
# 335 "B:\\otlib/radio.h"
void radio_sleep();
# 349 "B:\\otlib/radio.h"
void radio_idle();



void radio_set_mactimer(ot_u16 clocks);
# 366 "B:\\otlib/radio.h"
void radio_flush_tx();
# 377 "B:\\otlib/radio.h"
void radio_flush_rx();
# 386 "B:\\otlib/radio.h"
void radio_putbyte(ot_u8 databyte);
# 397 "B:\\otlib/radio.h"
void radio_putfourbytes(ot_u8* data);
# 406 "B:\\otlib/radio.h"
ot_u8 radio_getbyte();
# 417 "B:\\otlib/radio.h"
void radio_getfourbytes(ot_u8* data);
# 426 "B:\\otlib/radio.h"
ot_bool radio_rxopen();






ot_bool radio_rxopen_4();
# 442 "B:\\otlib/radio.h"
ot_bool radio_txopen();






ot_bool radio_txopen_4();
# 470 "B:\\otlib/radio.h"
ot_int rm2_default_tgd(ot_u8 chan_id);
# 483 "B:\\otlib/radio.h"
ot_int rm2_pkt_duration(ot_int pkt_bytes);
# 496 "B:\\otlib/radio.h"
ot_int rm2_scale_codec(ot_int buf_bytes);
# 514 "B:\\otlib/radio.h"
void rm2_reenter_rx(ot_sig2 callback);
# 527 "B:\\otlib/radio.h"
void rm2_resend(ot_sig2 callback);
# 563 "B:\\otlib/radio.h"
void rm2_rxinit(ot_u8 channel, ot_u8 psettings, ot_sig2 callback);
# 584 "B:\\otlib/radio.h"
void rm2_txinit(ot_u8 psettings, ot_sig2 callback);
# 597 "B:\\otlib/radio.h"
void rm2_txstop_flood();
# 607 "B:\\otlib/radio.h"
void rm2_kill();
# 620 "B:\\otlib/radio.h"
void rm2_rxsync_isr();
# 632 "B:\\otlib/radio.h"
void rm2_rxtimeout_isr();
# 644 "B:\\otlib/radio.h"
void rm2_rxdata_isr();
# 656 "B:\\otlib/radio.h"
void rm2_rxend_isr();
# 667 "B:\\otlib/radio.h"
void rm2_txcsma_isr();
# 679 "B:\\otlib/radio.h"
void rm2_txdata_isr();
# 45 "B:\\otlib/OTAPI.h" 2

# 1 "B:\\otlib/veelite.h" 1
# 47 "B:\\otlib/OTAPI.h" 2
# 62 "B:\\otlib/OTAPI.h"
# 1 "B:\\otlib/mpipe.h" 1
# 63 "B:\\otlib/mpipe.h"
typedef enum {
    MPIPE_9600bps = 0,
    MPIPE_28800bps = 1,
    MPIPE_57600bps = 2,
    MPIPE_115200bps = 3,
    MPIPE_230400bps = 4,
    MPIPE_460800bps = 5
} mpipe_speed;



typedef enum {
    MPIPE_Low = 0,
    MPIPE_High = 1,
    MPIPE_Broadcast = 2,
    MPIPE_Ack = 3
} mpipe_priority;







typedef enum {
    MPIPE_Null = -1,
    MPIPE_Idle = 0,
    MPIPE_RxHeader = 1,
    MPIPE_RxPayload = 2,
    MPIPE_TxAck_Wait = 3,
    MPIPE_TxAck_Done = 4,
    MPIPE_Tx_Wait = 5,
    MPIPE_Tx_Done = 6,
    MPIPE_RxAck = 7,
    MPIPE_RxAckHeader = 8
} mpipe_state;
# 112 "B:\\otlib/mpipe.h"
typedef struct {
    mpipe_state state;
    alp_tmpl alp;






} mpipe_struct;

extern mpipe_struct mpipe;
# 148 "B:\\otlib/mpipe.h"
void mpipe_connect(void* port_id);
# 166 "B:\\otlib/mpipe.h"
void mpipe_disconnect(void* port_id);
# 177 "B:\\otlib/mpipe.h"
mpipe_state mpipe_status();
# 189 "B:\\otlib/mpipe.h"
void mpipe_open();
# 201 "B:\\otlib/mpipe.h"
void mpipe_close();







void mpipe_send();
# 221 "B:\\otlib/mpipe.h"
void mpipe_systask(ot_task task);
# 237 "B:\\otlib/mpipe.h"
void mpipeevt_rxinit(ot_int code);
# 254 "B:\\otlib/mpipe.h"
void mpipeevt_rxdetect(ot_int code);
# 263 "B:\\otlib/mpipe.h"
void mpipeevt_rxdone(ot_int code);
# 278 "B:\\otlib/mpipe.h"
void mpipeevt_txdone(ot_int code);
# 298 "B:\\otlib/mpipe.h"
ot_u8 mpipedrv_footerbytes();
# 319 "B:\\otlib/mpipe.h"
ot_int mpipedrv_init(void* port_id);
# 337 "B:\\otlib/mpipe.h"
void mpipedrv_standby();
# 353 "B:\\otlib/mpipe.h"
void mpipedrv_detach(void* port_id);
# 362 "B:\\otlib/mpipe.h"
void mpipedrv_kill();
# 372 "B:\\otlib/mpipe.h"
void mpipedrv_block();
# 382 "B:\\otlib/mpipe.h"
void mpipedrv_unblock();
# 391 "B:\\otlib/mpipe.h"
void mpipedrv_wait();
# 404 "B:\\otlib/mpipe.h"
void mpipedrv_setspeed(mpipe_speed speed);
# 426 "B:\\otlib/mpipe.h"
void mpipedrv_txndef(ot_bool blocking, mpipe_priority data_priority);
# 442 "B:\\otlib/mpipe.h"
void mpipedrv_rxndef(ot_bool blocking, mpipe_priority data_priority);
# 462 "B:\\otlib/mpipe.h"
void mpipedrv_isr();
# 63 "B:\\otlib/OTAPI.h" 2
# 74 "B:\\otlib/OTAPI.h"
# 1 "B:\\otlib/OTAPI_c.h" 1
# 55 "B:\\otlib/OTAPI_c.h"
void otapi_poweron();






void otapi_poweroff();







void otapi_init();
# 80 "B:\\otlib/OTAPI_c.h"
void otapi_exec();







void otapi_preempt();







void otapi_pause();



void otapi_led1_on();
void otapi_led2_on();
void otapi_led1_off();
void otapi_led2_off();
# 162 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_sysinit();
# 179 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_new_dialog(session_tmpl* s_tmpl, void* applet);
# 191 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_new_advdialog(advert_tmpl* adv_tmpl, session_tmpl* s_tmpl, void* applet);
# 217 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_open_request(addr_type addr, routing_tmpl* routing);
# 239 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_close_request();
# 264 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_start_flood(ot_u16 flood_duration);
# 278 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_start_dialog(ot_u16 timeout);
# 311 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_session_number();
# 320 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_flush_sessions();
# 334 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_is_session_blocked(ot_u8 chan_id);
# 365 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_put_command_tmpl(ot_u8* status, command_tmpl* command);
# 384 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_put_dialog_tmpl(ot_u8* status, dialog_tmpl* dialog);
# 405 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_put_query_tmpl(ot_u8* status, query_tmpl* query);
# 417 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_put_ack_tmpl(ot_u8* status, ack_tmpl* ack);
# 432 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_put_error_tmpl(ot_u8* status, error_tmpl* error);
# 447 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_put_isf_comp(ot_u8* status, isfcomp_tmpl* isfcomp);
# 462 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_put_isf_call(ot_u8* status, isfcall_tmpl* isfcall);
# 477 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_put_isf_return(ot_u8* status, isfcall_tmpl* isfcall);
# 502 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_put_reqds(ot_u8* status, Queue* dsq);
# 525 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_put_propds(ot_u8* status, Queue* dsq);
# 540 "B:\\otlib/OTAPI_c.h"
ot_u16 otapi_put_udp_tmpl(ot_u8* status, udp_tmpl* udp);
# 75 "B:\\otlib/OTAPI.h" 2
# 137 "B:\\otlib/OTAPI.h"
m2session* otapi_task_immediate(session_tmpl* s_tmpl, ot_app applet);
# 161 "B:\\otlib/OTAPI.h"
m2session* otapi_task_schedule(session_tmpl* s_tmpl, ot_app applet, ot_u16 offset);
# 195 "B:\\otlib/OTAPI.h"
m2session* otapi_task_advertise(advert_tmpl* adv_tmpl, session_tmpl* s_tmpl, ot_app applet);
# 208 "B:\\otlib/OTAPI.h"
typedef enum {
 DATA_raw = 0,
 DATA_utf8 = 1,
 DATA_utf16 = 2,
 DATA_utf8hex = 3,
 MSG_raw = 4,
 MSG_utf8 = 5,
 MSG_utf16 = 6,
 MSG_utf8hex = 7
} logmsg_type;
# 230 "B:\\otlib/OTAPI.h"
ot_bool otapi_log_header(ot_u8 id_subcode, ot_int payload_length);
# 243 "B:\\otlib/OTAPI.h"
void otapi_log_direct();
# 260 "B:\\otlib/OTAPI.h"
void otapi_log(ot_u8 subcode, ot_int length, ot_u8* data);
# 279 "B:\\otlib/OTAPI.h"
void otapi_log_msg(logmsg_type logcmd, ot_int label_len, ot_int data_len, ot_u8* label, ot_u8* data);
# 301 "B:\\otlib/OTAPI.h"
void otapi_log_hexmsg(ot_int label_len, ot_int data_len, ot_u8* label, ot_u8* data);
# 319 "B:\\otlib/OTAPI.h"
void otapi_log_code(ot_int label_len, ot_u8* label, ot_u16 code);
# 46 "B:\\otlib/veelite.h" 2
# 1 "B:\\otlib/veelite_core.h" 1
# 74 "B:\\otlib/veelite_core.h"
typedef enum {
    vas_error = 0,
    in_vworm,
    in_veeprom,
    in_vsram
} vas_loc;
# 88 "B:\\otlib/veelite_core.h"
typedef ot_u16 vaddr;
# 126 "B:\\otlib/veelite_core.h"
extern ot_u8 memory_faults;
# 135 "B:\\otlib/veelite_core.h"
vas_loc vas_check(vaddr addr);
# 146 "B:\\otlib/veelite_core.h"
ot_u8 vworm_format( );
# 159 "B:\\otlib/veelite_core.h"
ot_u8 vworm_init( );
# 172 "B:\\otlib/veelite_core.h"
ot_u8 vworm_save( );
# 184 "B:\\otlib/veelite_core.h"
ot_u16 vworm_read(vaddr addr);
# 203 "B:\\otlib/veelite_core.h"
ot_u8 vworm_write(vaddr addr, ot_u16 data);
# 220 "B:\\otlib/veelite_core.h"
ot_u8 vworm_mark(vaddr addr, ot_u16 value);
ot_u8 vworm_mark_physical(ot_u16* addr, ot_u16 value);
# 233 "B:\\otlib/veelite_core.h"
ot_u8* vworm_get(vaddr addr);
# 242 "B:\\otlib/veelite_core.h"
void vworm_print_table();
# 254 "B:\\otlib/veelite_core.h"
ot_u8 vworm_wipeblock(vaddr addr, ot_uint wipe_span);
# 275 "B:\\otlib/veelite_core.h"
ot_u16 vsram_read(vaddr addr);
# 287 "B:\\otlib/veelite_core.h"
ot_u8 vsram_mark(vaddr addr, ot_u16 value);
ot_u8 vsram_mark_physical(ot_u16* addr, ot_u16 value);
# 298 "B:\\otlib/veelite_core.h"
ot_u8* vsram_get(vaddr addr);
# 47 "B:\\otlib/veelite.h" 2



typedef enum {
    VL_NULL_BLOCKID = 0,
    VL_GFB_BLOCKID = 1,
    VL_ISFS_BLOCKID = 2,
    VL_ISF_BLOCKID = 3
} vlBLOCK;







typedef struct {
    vaddr header;
    ot_u16 start;
    ot_u16 alloc;
    ot_u16 idmod;
    ot_u16 length;
    ot_u16 (*read)(ot_uint);
    ot_u8 (*write)(ot_uint, ot_u16);
} vlFILE;
# 85 "B:\\otlib/veelite.h"
typedef struct {
    ot_u16 manuf_id;
    ot_u32 serial;
    ot_u32 model_no;
    ot_u32 fw_version;
    ot_u16 max_response;
} M1TAG_struct;
# 100 "B:\\otlib/veelite.h"
typedef struct {
    ot_u32 placeholder;
} M1table_header;
# 118 "B:\\otlib/veelite.h"
typedef struct {
    ot_u16 length;
    ot_u16 alloc;
    ot_u16 idmod;
    vaddr base;
    vaddr mirror;
} vl_header;
# 144 "B:\\otlib/veelite.h"
typedef struct {
    ot_u8 id;
    ot_u8 reserved;
    vl_header* header;
} vl_link;
# 225 "B:\\otlib/veelite.h"
void vl_init();
# 239 "B:\\otlib/veelite.h"
vlFILE* vl_get_fp(ot_int fd);
# 249 "B:\\otlib/veelite.h"
ot_int vl_get_fd(vlFILE* fp);
# 275 "B:\\otlib/veelite.h"
ot_u8 vl_new(vlFILE** fp_new, vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, ot_uint max_length, id_tmpl* user_id);
# 301 "B:\\otlib/veelite.h"
ot_u8 vl_delete(vlBLOCK block_id, ot_u8 data_id, id_tmpl* user_id);
# 327 "B:\\otlib/veelite.h"
ot_u8 vl_getheader_vaddr(vaddr* header, vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, id_tmpl* user_id);
# 345 "B:\\otlib/veelite.h"
ot_u8 vl_getheader(vl_header* header, vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, id_tmpl* user_id);
# 358 "B:\\otlib/veelite.h"
vlFILE* vl_open_file(vaddr header);
# 377 "B:\\otlib/veelite.h"
vlFILE* vl_open(vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, id_tmpl* user_id);
vlFILE* GFB_open_su( ot_u8 id );
vlFILE* ISFS_open_su( ot_u8 id );
vlFILE* ISF_open_su( ot_u8 id );
vlFILE* GFB_open( ot_u8 id, ot_u8 mod, id_tmpl* user_id );
vlFILE* ISFS_open( ot_u8 id, ot_u8 mod, id_tmpl* user_id );
vlFILE* ISF_open( ot_u8 id, ot_u8 mod, id_tmpl* user_id );
# 414 "B:\\otlib/veelite.h"
ot_u8 vl_chmod(vlBLOCK block_id, ot_u8 data_id, ot_u8 mod, id_tmpl* user_id);
ot_u8 GFB_chmod_su( ot_u8 id, ot_u8 mod );
ot_u8 ISFS_chmod_su( ot_u8 id, ot_u8 mod );
ot_u8 ISF_chmod_su( ot_u8 id, ot_u8 mod );
# 428 "B:\\otlib/veelite.h"
ot_u16 vl_read( vlFILE* fp, ot_uint offset );
# 443 "B:\\otlib/veelite.h"
ot_u8 vl_write( vlFILE* fp, ot_uint offset, ot_u16 data );
# 456 "B:\\otlib/veelite.h"
ot_uint vl_load( vlFILE* fp, ot_uint length, ot_u8* data );
# 467 "B:\\otlib/veelite.h"
ot_u8 vl_store( vlFILE* fp, ot_uint length, ot_u8* data );
# 494 "B:\\otlib/veelite.h"
ot_u8 vl_close( vlFILE* fp );






ot_uint vl_checklength( vlFILE* fp );






ot_uint vl_checkalloc( vlFILE* fp );
# 539 "B:\\otlib/veelite.h"
ot_u8 ISF_syncmirror( );
# 549 "B:\\otlib/veelite.h"
ot_u8 ISF_loadmirror( );
# 38 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c" 2
# 1 "B:\\otlib/veelite_core.h" 1
# 39 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c" 2
# 1 "B:\\otlib/buffers.h" 1
# 40 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c" 2
# 1 "B:\\otlib/auth.h" 1
# 41 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c" 2
# 1 "B:\\otlib/mpipe.h" 1
# 42 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c" 2
# 1 "B:\\otlib/radio.h" 1
# 43 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c" 2
# 1 "B:\\otlib/system.h" 1
# 44 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c" 2
# 1 "B:\\otlib/session.h" 1
# 45 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c" 2
# 192 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
platform_struct platform;
platform_ext_struct platform_ext;
# 202 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
void sub_msflash_config(void);
void sub_hsflash_config(void);
void sub_voltage_config(void);
void sub_hsosc_config(void);
# 225 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
void NMI_Handler(void) {
    ot_int code;


    if (platform.error_code == 0)
        while(1);


    code = platform.error_code;
    platform.error_code = 0;
    sys_panic(code);
}


void HardFault_Handler(void) {

    while (1);



}


void MemManage_Handler(void) {

    while (1);



}


void BusFault_Handler(void) {

    while (1);



}


void UsageFault_Handler(void) {

    while (1);



}


void DebugMon_Handler(void) { }






void SVC_Handler(void) {




    platform_ext.next_evt = sys_event_manager();
    ((SCB_Type *) ((0xE000E000) + 0x0D00))->ICSR |= (platform_ext.next_evt == 0) << 28;
# 320 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
}


void OT_GPTIM_ISR() {


    SET_PENDSV();
}


void PendSV_Handler(void) {
# 339 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
    CLR_PENDSV();






}
# 359 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
void WWDG_IRQHandler(void) {
}
# 429 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
void platform_poweron() {


    __set_PSP( __get_MSP() );
    __set_CONTROL(2);
    __set_MSP( (ot_u32)platform_ext.sstack );


    platform_init_busclk();
    platform_init_perihclk();


    BOARD_PERIPH_INIT();
    BOARD_POWER_STARTUP();



    ((DBGMCU_TypeDef *) ((uint32_t)0xE0042000))->CR |= ( ((uint32_t)0x00000001)
                      | ((uint32_t)0x00000002)
                      | ((uint32_t)0x00000004));

    ((DBGMCU_TypeDef *) ((uint32_t)0xE0042000))->APB1FZ |= ( ((uint32_t)0x00000400)
                      | ((uint32_t)0x00000800)
                      | ((uint32_t)0x00001000));

    ((DBGMCU_TypeDef *) ((uint32_t)0xE0042000))->APB2FZ |= ( ((uint32_t)0x00000004)
                      | ((uint32_t)0x00000008)
                      | ((uint32_t)0x00000010));



    platform_init_gpio();
    platform_init_interruptor();
    platform_init_gptim(0);



    vworm_init();
}




void platform_poweroff() {
    ISF_syncmirror();
    vworm_save();




}





void platform_init_OT() {



    buffers_init();
    vl_init();
# 500 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
    sys_init();







    {
        vlFILE* fpid;
        ot_u16* hwid;
        ot_int i;

        fpid = ISF_open_su(0x01);
        hwid = (ot_u16*)(0x1FF80050);
        for (i=6; i!=0; i-=2) {
            vl_write(fpid, i, *hwid++);
        }
        vl_close(fpid);
    }

}



void sub_msflash_config(void) {


    ((FLASH_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3C00))->ACR |= ((uint32_t)0x00000004);


        ((FLASH_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3C00))->ACR |= (((uint32_t)0x00000002));



}

void sub_hsflash_config(void) {


    ((FLASH_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3C00))->ACR |= ((uint32_t)0x00000004);





        ((FLASH_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3C00))->ACR |= (((uint32_t)0x00000002) | ((uint32_t)0x00000001));

}


void sub_voltage_config(void) {

    ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->APB1ENR |= ((uint32_t)0x10000000);



    ((PWR_TypeDef *) (((uint32_t)0x40000000) + 0x7000))->CR = ((uint16_t)0x1000);







    while((((PWR_TypeDef *) (((uint32_t)0x40000000) + 0x7000))->CSR & ((uint16_t)0x0010)) != RESET) { }
}


void sub_hsosc_config(void) {
# 589 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
    ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->CFGR |= (((uint32_t)0x00000001) | (0<<4) | (0<<8) | (0<<11));
    while ((((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->CFGR & (uint32_t)((uint32_t)0x0000000C)) != (uint32_t)((uint32_t)0x00000008)) { }



    platform_ext.cpu_khz = (((16000000*1)/1)/1000);
}




void platform_init_busclk() {


    ot_u16 counter;


    ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->CR |= (uint32_t)0x00000100;
    ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->CFGR &= (uint32_t)0x88FFC00C;

    ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->CR &= (uint32_t)0xEEFEFFFE;
    ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->CR &= (uint32_t)0xFFFBFFFF;
    ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->CFGR &= (uint32_t)0xFF02FFFF;
    ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->CIR = 0x00000000;
# 632 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
            platform_ext.cpu_khz = ((4200000)/1000);



            sub_msflash_config();
            sub_voltage_config();


            ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->CFGR |= ((0<<4) | (0<<8) | (0<<11));



            ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->ICSCR ^= 0x00006000;
# 718 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
        ((SCB_Type *) ((0xE000E000) + 0x0D00))->VTOR = ((uint32_t)0x08000000);

}





void platform_init_periphclk() {

}





void platform_disable_interrupts() {
    __disable_irq();
}




void platform_enable_interrupts() {
    __enable_irq();
}
# 757 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
void platform_standard_speed() {





    if ((((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->CR & ((uint32_t)0x00000100)) == 0) {
        ot_u16 counter;


        ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->CR |= ((uint32_t)0x00000100);
        counter = 300;
        while (((((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->CR & ((uint32_t)0x00000200)) == 0) && (--counter != 0));



        ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->CFGR &= ~3;
        while (((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->CFGR & 0xC);


        ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->CR &= ~(((uint32_t)0x01000000) | ((uint32_t)0x00010000) | ((uint32_t)0x00000001));







            ((FLASH_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3C00))->ACR &= ~((uint32_t)0x00000001);



        platform_ext.cpu_khz = ((4200000)/1000);
    }

}

void platform_full_speed() {

    if (((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->CR & ((uint32_t)0x00000100)) {






            ((FLASH_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3C00))->ACR |= ((uint32_t)0x00000001);




        sub_hsosc_config();


        ((RCC_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x3800))->CR &= ~((uint32_t)0x00000100);
    }
}

void platform_flank_speed() {


    platform_full_speed();
}
# 831 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
inline void* platform_save_context(void) {

    ot_u32 tsp;
    asm volatile (
    "   MRS    %0, psp\n"
    "   STMDB  %0!, {r4-r11}\n"
    "   MSR    psp, %0\n"
        : "=r" (tsp)
    );
    return (void*)tsp;
}



inline void platform_load_context(void* tsp) {

    ot_u32 scratch;
    asm volatile (
    "   MRS    %0, psp\n"
    "   LDMFD  %0!, {r4-r11}\n"
    "   MSR    psp, %0\n"
        : "=r" (scratch)
    );
}



inline void platform_switch_context(void* tsp) {
}



void platform_drop_context(ot_uint i) {
# 874 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
    if (platform_ext.task_exit != 0) {



        register ot_u32 task_lr;
        asm volatile ("MRS  %0, PSP" : "=r"(task_lr) );
        ((ot_u32*)task_lr)[6] = (ot_u32)platform_ext.task_exit;
    }
}




void platform_ot_preempt() {





    if (((EXTI_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0400))->IMR & (1<<5)) {
        asm volatile(" svc  %0" : : "I"(0) );;
    }
}
# 909 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
inline void platform_ot_run() {
# 919 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
    register ot_u32 return_from_task;
    platform_ext.task_exit = (void*)__get_PSP();
    return_from_task = (ot_u32)&&RETURN_FROM_TASK;
    asm volatile ("PUSH {%0}" : : "r"(return_from_task) );



    sys_run_task();




    RETURN_FROM_TASK:
    __set_PSP( (ot_u32)platform_ext.task_exit );
    platform_ext.task_exit = 0;





    asm volatile(" svc  %0" : : "I"(0) );;






    if (platform_ext.next_evt != 0) {
        platform_disable_interrupts();
        sys_powerdown();
    }
}
# 964 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
void platform_init_interruptor() {
# 1006 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
    NVIC_SetPriorityGrouping(6);







    ((SCB_Type *) ((0xE000E000) + 0x0D00))->SHP[((uint32_t)(SVC_IRQn)&0xF)-4] = (0 << 4);

    ((SCB_Type *) ((0xE000E000) + 0x0D00))->SHP[((uint32_t)(PendSV_IRQn)&0xF)-4] = (15 << 4);
    ((SCB_Type *) ((0xE000E000) + 0x0D00))->SHP[((uint32_t)(SysTick_IRQn)&0xF)-4] = (8 << 4);
# 1041 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
    ((NVIC_Type *) ((0xE000E000) + 0x0100))->IP[(uint32_t)(EXTI9_5_IRQn)] = ((0 +(1)) << 4);
    ((NVIC_Type *) ((0xE000E000) + 0x0100))->ISER[((uint32_t)(EXTI9_5_IRQn)>>5)] = (1 << ((uint32_t)(EXTI9_5_IRQn) & 0x1F));
# 1071 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
}
# 1082 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
void platform_init_gpio() {





    BOARD_PORT_STARTUP();
}





void platform_init_gptim(ot_uint prescaler) {
# 1123 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
    ((EXTI_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0400))->RTSR |= ((1<<5) | ((0 != 1) << 6));


    ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->SMCR = ((uint16_t)0x4000);
    ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->DIER = 0;
    ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->SR = 0;





    ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->CCMR1 = ((uint16_t)0x0010);
    ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->CCER = ((uint16_t)0x0001);

    ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->PSC = ((32768 / 1024)-1);
    ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->ARR = 65535;
    ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->OR = 0;


    ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->EGR = ((uint8_t)0x01);
    ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->CR1 |= ((uint16_t)0x0001);
}




void platform_init_watchdog() {



}




void platform_init_resetswitch() {

}




void platform_init_systick(ot_uint period) {




    if (period <= (0xFFFFFFul << 0)) {
        ((SysTick_Type *) ((0xE000E000) + 0x0010))->VAL = 0;
        ((SysTick_Type *) ((0xE000E000) + 0x0010))->CTRL = (1ul << 2)
                        | (1ul << 1)
                        | (1ul << 0);
    }
}




void platform_init_rtc(ot_u32 value) {
# 1213 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
}



void platform_init_memcpy() {


}
# 1231 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
ot_u16 platform_get_ktim() {
    return (((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->CNT - platform_ext.last_evt);
}

ot_u16 platform_next_ktim() {
    return (((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->CCR1 - ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->CNT);
}

void platform_enable_ktim() {
    ((EXTI_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0400))->IMR |= (1<<5);
}

void platform_disable_ktim() {
    ((EXTI_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0400))->IMR &= ~(1<<5);
}

void platform_pend_ktim() {
    ((EXTI_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0400))->SWIER |= (1<<5);
}

void platform_flush_ktim() {
    platform_ext.last_evt = ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->CNT;
    ((EXTI_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0400))->IMR &= ~(1<<5);
}

void platform_set_ktim(ot_u16 value) {



    platform_ext.last_evt = ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->CNT;
    ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->CCR1 = platform_ext.last_evt + value;
}

void platform_set_gptim2(ot_u16 value) {

    ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->CCR2 = ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->CNT + value;
    ((EXTI_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0400))->SWIER |= (1<<(value==0));
}

ot_u16 platform_get_gptim() {
    return ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->CNT;
}




void platform_set_watchdog(ot_u16 timeout_ticks) {
# 1301 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
}

void platform_kill_watchdog() {

}

void platform_pause_watchdog() {

}

void platform_resume_watchdog() {





}





void platform_enable_rtc() {







}




void platform_disable_rtc() {






}




void platform_set_time(ot_u32 utc_time) {


    platform_ext.utc = utc_time;




}




ot_u32 platform_get_time() {




}




void platform_set_rtc_alarm(ot_u8 alarm_id, ot_u8 task_id, ot_u16 offset) {
# 1405 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
}
# 1421 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
void platform_trig1_high() { ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0C00))->MODER |= (1*2);; }
void platform_trig1_low() { ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0C00))->MODER &= ~(1*2);; }
void platform_trig1_toggle() { OT_TRIG1_TOG(); }







void platform_trig2_high() { ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0C00))->MODER |= (2*2);; }
void platform_trig2_low() { ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0C00))->MODER &= ~(2*2);; }
void platform_trig2_toggle() { ((GPIO_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x0C00))->MODER ^= (2*2);; }
# 1456 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
void platform_rand(ot_u8* rand_out, ot_int bytes_out) {
# 1537 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
}




void platform_init_prand(ot_u16 seed) {
    platform_ext.prand_reg = seed;
}



ot_u8 platform_prand_u8() {
    return (ot_u8)platform_prand_u16();
}



ot_u16 platform_prand_u16() {
    ot_u16 timer_value;
    timer_value = ((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0800))->CNT;

    return platform_crc_block((ot_u8*)&timer_value, 1);
}
# 1594 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
void sub_memcpy_dma(ot_u8* dest, ot_u8* src, ot_int length) {

    static const ot_u16 ccr[4] = { 0x4AD1, 0x40D1, 0x45D1, 0x40D1 };
    static const ot_u16 len_div[4] = { 2, 0, 1, 0 };
    ot_int align;
    align = ((ot_u32)dest | (ot_u32)src | (ot_u32)length) & 3;

    ((DMA_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x6000))->IFCR = (1 << ((5 -1)*4));
    ((DMA_Channel_TypeDef *) (((((uint32_t)0x40000000) + 0x20000) + 0x6000) + 0x0058))->CPAR = (ot_u32)dest;
    ((DMA_Channel_TypeDef *) (((((uint32_t)0x40000000) + 0x20000) + 0x6000) + 0x0058))->CMAR = (ot_u32)src;
    ((DMA_Channel_TypeDef *) (((((uint32_t)0x40000000) + 0x20000) + 0x6000) + 0x0058))->CNDTR = length >> len_div[align];
    ((DMA_Channel_TypeDef *) (((((uint32_t)0x40000000) + 0x20000) + 0x6000) + 0x0058))->CCR = ccr[align];

    while((((DMA_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x6000))->ISR & (1 << ((5 -1)*4))) == 0);
}

void sub_memcpy2_dma(ot_u8* dest, ot_u8* src, ot_int length) {

    ot_u16 ccr_val = 0x45D1;


    if ((((ot_u32)dest | (ot_u32)src | (ot_u32)length) & 3) == 0) {
        length >>= 1;
        ccr_val += 0x0500;
    }
    ((DMA_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x6000))->IFCR = (1 << ((5 -1)*4));
    ((DMA_Channel_TypeDef *) (((((uint32_t)0x40000000) + 0x20000) + 0x6000) + 0x0058))->CPAR = (ot_u32)dest;
    ((DMA_Channel_TypeDef *) (((((uint32_t)0x40000000) + 0x20000) + 0x6000) + 0x0058))->CMAR = (ot_u32)src;
    ((DMA_Channel_TypeDef *) (((((uint32_t)0x40000000) + 0x20000) + 0x6000) + 0x0058))->CNDTR = length;
    ((DMA_Channel_TypeDef *) (((((uint32_t)0x40000000) + 0x20000) + 0x6000) + 0x0058))->CCR = ccr_val;

    while((((DMA_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x6000))->ISR & (1 << ((5 -1)*4))) == 0);
}


void platform_memcpy(ot_u8* dest, ot_u8* src, ot_int length) {





    sub_memcpy_dma(dest, src, length);
# 1646 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
}


void platform_memcpy_2(ot_u16* dest, ot_u16* src, ot_int length) {

    sub_memcpy2_dma( (ot_u8*)dest, (ot_u8*)src, length);
# 1663 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
}



void platform_memset(ot_u8* dest, ot_u8 value, ot_int length) {




    ((DMA_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x6000))->IFCR = (1 << ((5 -1)*4));
    ((DMA_Channel_TypeDef *) (((((uint32_t)0x40000000) + 0x20000) + 0x6000) + 0x0058))->CPAR = (ot_u32)dest;
    ((DMA_Channel_TypeDef *) (((((uint32_t)0x40000000) + 0x20000) + 0x6000) + 0x0058))->CMAR = (ot_u32)&value;
    ((DMA_Channel_TypeDef *) (((((uint32_t)0x40000000) + 0x20000) + 0x6000) + 0x0058))->CNDTR = length;
    ((DMA_Channel_TypeDef *) (((((uint32_t)0x40000000) + 0x20000) + 0x6000) + 0x0058))->CCR = ((uint16_t)0x0010) |
                              ((uint16_t)0x0040) |
                              ((uint16_t)0x0000) |
                              ((uint16_t)0x4000) |
                              ((uint16_t)0x0001);
    while((((DMA_TypeDef *) ((((uint32_t)0x40000000) + 0x20000) + 0x6000))->ISR & (1 << ((5 -1)*4))) == 0);




}




void sub_timed_wfe(ot_u16 count, ot_u16 prescaler) {







    while ((((TIM_TypeDef *) ((((uint32_t)0x40000000) + 0x10000) + 0x0C00))->SR & ((uint16_t)0x0001)) == 0) {
        __WFE();
    }

}



void platform_block(ot_u16 sti) {
    sub_timed_wfe(sti, 0);
}




void platform_delay(ot_u16 n) {
    sub_timed_wfe(n, 31);
}





void platform_swdelay_ms(ot_u16 n) {
    ot_u32 c;
    c = platform_ext.cpu_khz;
    c *= n;
    c >>= 2;
    while (c--);
}




void platform_swdelay_us(ot_u16 n) {
    ot_u32 c;
    c = platform_ext.cpu_khz;
    c *= n;
    c >>= 12;
    while (c--);
}
