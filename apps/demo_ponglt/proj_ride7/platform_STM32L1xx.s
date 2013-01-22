	.syntax unified
	.cpu cortex-m3
	.fpu softvfp
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 1
	.eabi_attribute 30, 4
	.eabi_attribute 18, 4
	.file	"platform_STM32L1xx.c"
@ GNU C (Sourcery G++ Lite 2011.03-42) version 4.5.2 (arm-none-eabi)
@	compiled by GNU C version 4.3.2, GMP version 4.3.2, MPFR version 2.4.2, MPC version 0.8.1
@ GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
@ options passed:  -fpreprocessed platform_STM32L1xx.i -mthumb
@ -mcpu=cortex-m3 -mlittle-endian -auxbase-strip
@ B:\apps\demo_ponglt\proj_ride7\platform_STM32L1xx.o -g -Os
@ -funsigned-char -ffunction-sections -fverbose-asm
@ options enabled:  -falign-loops -fargument-alias -fauto-inc-dec
@ -fbranch-count-reg -fcaller-saves -fcommon -fcprop-registers
@ -fcrossjumping -fcse-follow-jumps -fdefer-pop
@ -fdelete-null-pointer-checks -fdwarf2-cfi-asm -fearly-inlining
@ -feliminate-unused-debug-types -fexpensive-optimizations
@ -fextension-elimination -fforward-propagate -ffunction-cse
@ -ffunction-sections -fgcse -fgcse-lm -fguess-branch-probability -fident
@ -fif-conversion -fif-conversion2 -findirect-inlining -finline
@ -finline-functions -finline-functions-called-once
@ -finline-small-functions -fipa-cp -fipa-pure-const -fipa-reference
@ -fipa-sra -fira-share-save-slots -fira-share-spill-slots -fivopts
@ -fkeep-static-consts -fleading-underscore -fmath-errno -fmerge-constants
@ -fmerge-debug-strings -fomit-frame-pointer -foptimize-register-move
@ -foptimize-sibling-calls -fpeephole -fpeephole2 -freg-struct-return
@ -fregmove -freorder-blocks -freorder-functions -frerun-cse-after-loop
@ -fsched-critical-path-heuristic -fsched-dep-count-heuristic
@ -fsched-group-heuristic -fsched-interblock -fsched-last-insn-heuristic
@ -fsched-rank-heuristic -fsched-spec -fsched-spec-insn-heuristic
@ -fsched-stalled-insns-dep -fschedule-insns2 -fsection-anchors
@ -fshow-column -fsigned-zeros -fsplit-ivs-in-unroller -fsplit-wide-types
@ -fstrict-aliasing -fstrict-overflow -fstrict-volatile-bitfields
@ -fthread-jumps -ftoplevel-reorder -ftrapping-math -ftree-builtin-call-dce
@ -ftree-ccp -ftree-ch -ftree-copy-prop -ftree-copyrename -ftree-cselim
@ -ftree-dce -ftree-dominator-opts -ftree-dse -ftree-forwprop -ftree-fre
@ -ftree-if-to-switch-conversion -ftree-loop-im -ftree-loop-ivcanon
@ -ftree-loop-optimize -ftree-parallelize-loops= -ftree-phiprop -ftree-pta
@ -ftree-reassoc -ftree-scev-cprop -ftree-sink -ftree-slp-vectorize
@ -ftree-sra -ftree-switch-conversion -ftree-ter -ftree-vect-loop-version
@ -ftree-vrp -funit-at-a-time -fvar-tracking -fvar-tracking-assignments
@ -fverbose-asm -fzero-initialized-in-bss -mfix-cortex-m3-ldrd
@ -mlittle-endian -msched-prolog -mthumb -munaligned-access

	.section	.debug_abbrev,"",%progbits
.Ldebug_abbrev0:
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.section	.debug_line,"",%progbits
.Ldebug_line0:
	.text
.Ltext0:
@ Compiler executable checksum: d985438640072664965a7048a2edf242

	.cfi_sections	.debug_frame
	.section	.text.BOARD_PERIPH_INIT,"ax",%progbits
	.align	1
	.global	BOARD_PERIPH_INIT
	.thumb
	.thumb_func
	.type	BOARD_PERIPH_INIT, %function
BOARD_PERIPH_INIT:
.LFB29:
	.file 1 "B:\\board/stm32l1xx/board_IKR001.h"
	.loc 1 457 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 1 459 0
	ldr	r3, .L2	@ tmp139,
	ldr	r2, .L2+4	@ D.7895,
	ldr	r1, [r3, #28]	@ D.7894, 1073887232B->AHBENR
	orr	r2, r1, r2	@ D.7895, D.7894, D.7895
	str	r2, [r3, #28]	@ D.7895, 1073887232B->AHBENR
	.loc 1 462 0
	ldr	r2, .L2+8	@ tmp143,
	str	r2, [r3, #40]	@ tmp143, 1073887232B->AHBLPENR
	.loc 1 468 0
	ldr	r2, [r3, #32]	@ D.7896, 1073887232B->APB2ENR
	orr	r2, r2, #1	@ D.7897, D.7896,
	str	r2, [r3, #32]	@ D.7897, 1073887232B->APB2ENR
	.loc 1 475 0
	ldr	r2, [r3, #36]	@ D.7898, 1073887232B->APB1ENR
	orr	r2, r2, #268435456	@ D.7899, D.7898,
	str	r2, [r3, #36]	@ D.7899, 1073887232B->APB1ENR
	.loc 1 476 0
	bx	lr	@
.L3:
	.align	2
.L2:
	.word	1073887232
	.word	16810015
	.word	16875551
	.cfi_endproc
.LFE29:
	.size	BOARD_PERIPH_INIT, .-BOARD_PERIPH_INIT
	.section	.text.BOARD_DMA_CLKON,"ax",%progbits
	.align	1
	.global	BOARD_DMA_CLKON
	.thumb
	.thumb_func
	.type	BOARD_DMA_CLKON, %function
BOARD_DMA_CLKON:
.LFB30:
	.loc 1 480 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 1 489 0
	bx	lr	@
	.cfi_endproc
.LFE30:
	.size	BOARD_DMA_CLKON, .-BOARD_DMA_CLKON
	.section	.text.BOARD_DMA_CLKOFF,"ax",%progbits
	.align	1
	.global	BOARD_DMA_CLKOFF
	.thumb
	.thumb_func
	.type	BOARD_DMA_CLKOFF, %function
BOARD_DMA_CLKOFF:
.LFB31:
	.loc 1 491 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 1 500 0
	bx	lr	@
	.cfi_endproc
.LFE31:
	.size	BOARD_DMA_CLKOFF, .-BOARD_DMA_CLKOFF
	.section	.text.BOARD_EXTI_STARTUP,"ax",%progbits
	.align	1
	.global	BOARD_EXTI_STARTUP
	.thumb
	.thumb_func
	.type	BOARD_EXTI_STARTUP, %function
BOARD_EXTI_STARTUP:
.LFB32:
	.loc 1 519 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 1 524 0
	ldr	r3, .L7	@ tmp137,
	ldr	r2, [r3, #16]	@ D.7889, 1073807360B->EXTICR
	orr	r2, r2, #9280	@ D.7890, D.7889,
	str	r2, [r3, #16]	@ D.7890, 1073807360B->EXTICR
	.loc 1 529 0
	ldr	r2, [r3, #20]	@ D.7891, 1073807360B->EXTICR
	orr	r2, r2, #544	@ D.7892, D.7891,
	orr	r2, r2, #2	@ D.7892, D.7892,
	str	r2, [r3, #20]	@ D.7892, 1073807360B->EXTICR
	.loc 1 535 0
	bx	lr	@
.L8:
	.align	2
.L7:
	.word	1073807360
	.cfi_endproc
.LFE32:
	.size	BOARD_EXTI_STARTUP, .-BOARD_EXTI_STARTUP
	.section	.text.BOARD_PORT_STARTUP,"ax",%progbits
	.align	1
	.global	BOARD_PORT_STARTUP
	.thumb
	.thumb_func
	.type	BOARD_PORT_STARTUP, %function
BOARD_PORT_STARTUP:
.LFB33:
	.loc 1 541 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	.loc 1 590 0
	ldr	r2, .L10	@ tmp187,
	.loc 1 594 0
	ldr	r0, .L10+4	@ tmp189,
	.loc 1 590 0
	ldr	r3, [r2, #0]	@ D.7829, 1073872896B->MODER
	.loc 1 541 0
	push	{r4, lr}	@
.LCFI0:
	.cfi_def_cfa_offset 8
	.loc 1 590 0
	orr	r3, r3, #3072	@ D.7830, D.7829,
	str	r3, [r2, #0]	@ D.7830, 1073872896B->MODER
	.loc 1 594 0
	ldr	r3, [r0, #0]	@ D.7832, 1073874944B->MODER
	.loc 1 627 0
	movs	r1, #21	@ tmp196,
	.loc 1 594 0
	orr	r3, r3, #3840	@ D.7833, D.7832,
	str	r3, [r0, #0]	@ D.7833, 1073874944B->MODER
	.loc 1 600 0
	ldr	r3, [r2, #0]	@ D.7834, 1073872896B->MODER
	orr	r3, r3, #165	@ D.7835, D.7834,
	str	r3, [r2, #0]	@ D.7835, 1073872896B->MODER
	.loc 1 605 0
	ldr	r3, [r2, #32]	@ D.7836, 1073872896B->AFR
	orr	r3, r3, #30464	@ D.7837, D.7836,
	str	r3, [r2, #32]	@ D.7837, 1073872896B->AFR
	.loc 1 627 0
	ldr	r3, .L10+8	@ tmp195,
	str	r1, [r3, #0]	@ tmp196, 1073878016B->MODER
	.loc 1 629 0
	ldr	r3, [r2, #0]	@ D.7839, 1073872896B->MODER
	.loc 1 635 0
	ldr	r1, .L10+12	@ tmp199,
	.loc 1 629 0
	orr	r3, r3, #20971520	@ D.7840, D.7839,
	str	r3, [r2, #0]	@ D.7840, 1073872896B->MODER
	.loc 1 635 0
	ldrh	r3, [r1, #4]	@, 1073873920B->OTYPER
	uxth	r3, r3	@ D.7842, 1073873920B->OTYPER
	orr	r3, r3, #3072	@ D.7843, D.7842,
	strh	r3, [r1, #4]	@ movhi	@ D.7843, 1073873920B->OTYPER
	.loc 1 636 0
	ldr	r3, [r1, #8]	@ D.7844, 1073873920B->OSPEEDR
	orr	r3, r3, #5242880	@ D.7845, D.7844,
	str	r3, [r1, #8]	@ D.7845, 1073873920B->OSPEEDR
	.loc 1 638 0
	ldr	r3, [r1, #0]	@ D.7846, 1073873920B->MODER
	orr	r3, r3, #10485760	@ D.7847, D.7846,
	str	r3, [r1, #0]	@ D.7847, 1073873920B->MODER
	.loc 1 641 0
	ldr	r3, [r1, #36]	@ D.7848, 1073873920B->AFR
	orr	r3, r3, #17408	@ D.7849, D.7848,
	str	r3, [r1, #36]	@ D.7849, 1073873920B->AFR
	.loc 1 649 0
	ldr	r3, [r2, #0]	@ D.7850, 1073872896B->MODER
	orr	r3, r3, #262144	@ D.7851, D.7850,
	str	r3, [r2, #0]	@ D.7851, 1073872896B->MODER
	.loc 1 650 0
	ldr	r3, .L10+16	@ tmp212,
	ldr	r4, [r3, #0]	@ D.7853, 1073876992B->MODER
	.cfi_offset 14, -4
	.cfi_offset 4, -8
	orr	r4, r4, #1048576	@ D.7854, D.7853,
	str	r4, [r3, #0]	@ D.7854, 1073876992B->MODER
	.loc 1 651 0
	ldr	r4, [r1, #0]	@ D.7855, 1073873920B->MODER
	orr	r4, r4, #1426063360	@ D.7856, D.7855,
	str	r4, [r1, #0]	@ D.7856, 1073873920B->MODER
	.loc 1 659 0
	ldr	r4, [r2, #0]	@ D.7857, 1073872896B->MODER
	orr	r4, r4, #65536	@ D.7858, D.7857,
	str	r4, [r2, #0]	@ D.7858, 1073872896B->MODER
	.loc 1 664 0
	ldr	r4, [r0, #0]	@ D.7859, 1073874944B->MODER
	orr	r4, r4, #67108864	@ D.7860, D.7859,
	str	r4, [r0, #0]	@ D.7860, 1073874944B->MODER
	.loc 1 665 0
	ldr	r4, [r0, #8]	@ D.7861, 1073874944B->OSPEEDR
	orr	r4, r4, #67108864	@ D.7862, D.7861,
	str	r4, [r0, #8]	@ D.7862, 1073874944B->OSPEEDR
	.loc 1 666 0
	ldr	r0, [r3, #0]	@ D.7863, 1073876992B->MODER
	orr	r0, r0, #-1442840576	@ D.7864, D.7863,
	str	r0, [r3, #0]	@ D.7864, 1073876992B->MODER
	.loc 1 670 0
	ldr	r0, [r3, #8]	@ D.7865, 1073876992B->OSPEEDR
	orr	r0, r0, #-1442840576	@ D.7866, D.7865,
	str	r0, [r3, #8]	@ D.7866, 1073876992B->OSPEEDR
	.loc 1 674 0
	ldr	r0, [r3, #36]	@ D.7867, 1073876992B->AFR
	orr	r0, r0, #1426063360	@ D.7868, D.7867,
	orr	r0, r0, #5570560	@ D.7868, D.7868,
	str	r0, [r3, #36]	@ D.7868, 1073876992B->AFR
	.loc 1 682 0
	ldr	r0, [r3, #0]	@ D.7869, 1073876992B->MODER
	orr	r0, r0, #640	@ D.7870, D.7869,
	str	r0, [r3, #0]	@ D.7870, 1073876992B->MODER
	.loc 1 689 0
	ldr	r0, [r3, #0]	@ D.7871, 1073876992B->MODER
	orr	r0, r0, #2048	@ D.7872, D.7871,
	str	r0, [r3, #0]	@ D.7872, 1073876992B->MODER
	.loc 1 690 0
	ldr	r0, [r3, #0]	@ D.7873, 1073876992B->MODER
	orr	r0, r0, #8192	@ D.7874, D.7873,
	str	r0, [r3, #0]	@ D.7874, 1073876992B->MODER
	.loc 1 691 0
	ldr	r0, [r3, #32]	@ D.7875, 1073876992B->AFR
	orr	r0, r0, #3145728	@ D.7876, D.7875,
	str	r0, [r3, #32]	@ D.7876, 1073876992B->AFR
	.loc 1 692 0
	ldr	r0, [r3, #32]	@ D.7877, 1073876992B->AFR
	orr	r0, r0, #50331648	@ D.7878, D.7877,
	str	r0, [r3, #32]	@ D.7878, 1073876992B->AFR
	.loc 1 697 0
	ldr	r0, [r2, #0]	@ D.7879, 1073872896B->MODER
	orr	r0, r0, #1064960	@ D.7880, D.7879,
	orr	r0, r0, #4352	@ D.7880, D.7880,
	str	r0, [r2, #0]	@ D.7880, 1073872896B->MODER
	.loc 1 700 0
	ldr	r2, [r1, #0]	@ D.7881, 1073873920B->MODER
	orr	r2, r2, #1024	@ D.7882, D.7881,
	str	r2, [r1, #0]	@ D.7882, 1073873920B->MODER
	.loc 1 702 0
	ldr	r1, .L10+20	@ tmp244,
	ldr	r2, .L10+24	@ D.7885,
	ldr	r0, [r1, #0]	@ D.7884, 1073875968B->MODER
	orr	r2, r0, r2	@ D.7885, D.7884, D.7885
	str	r2, [r1, #0]	@ D.7885, 1073875968B->MODER
	.loc 1 709 0
	ldr	r2, [r3, #0]	@ D.7886, 1073876992B->MODER
	orr	r2, r2, #4194304	@ D.7887, D.7886,
	orr	r2, r2, #21	@ D.7887, D.7887,
	str	r2, [r3, #0]	@ D.7887, 1073876992B->MODER
	.loc 1 713 0
	pop	{r4, pc}	@
.L11:
	.align	2
.L10:
	.word	1073872896
	.word	1073874944
	.word	1073878016
	.word	1073873920
	.word	1073876992
	.word	1073875968
	.word	1431655424
	.cfi_endproc
.LFE33:
	.size	BOARD_PORT_STARTUP, .-BOARD_PORT_STARTUP
	.section	.text.BOARD_PORT_STANDBY,"ax",%progbits
	.align	1
	.global	BOARD_PORT_STANDBY
	.thumb
	.thumb_func
	.type	BOARD_PORT_STANDBY, %function
BOARD_PORT_STANDBY:
.LFB34:
	.loc 1 717 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 1 741 0
	ldr	r3, .L13	@ tmp141,
	ldr	r2, [r3, #0]	@ D.7819, 1073872896B->MODER
	orr	r2, r2, #165	@ D.7820, D.7819,
	str	r2, [r3, #0]	@ D.7820, 1073872896B->MODER
	.loc 1 771 0
	ldr	r3, .L13+4	@ tmp143,
	ldr	r2, [r3, #0]	@ D.7822, 1073876992B->MODER
	orr	r2, r2, #-1442840576	@ D.7823, D.7822,
	str	r2, [r3, #0]	@ D.7823, 1073876992B->MODER
	.loc 1 775 0
	ldr	r2, [r3, #8]	@ D.7824, 1073876992B->OSPEEDR
	orr	r2, r2, #-1442840576	@ D.7825, D.7824,
	str	r2, [r3, #8]	@ D.7825, 1073876992B->OSPEEDR
	.loc 1 779 0
	ldr	r2, [r3, #36]	@ D.7826, 1073876992B->AFR
	orr	r2, r2, #1426063360	@ D.7827, D.7826,
	orr	r2, r2, #5570560	@ D.7827, D.7827,
	str	r2, [r3, #36]	@ D.7827, 1073876992B->AFR
	.loc 1 795 0
	bx	lr	@
.L14:
	.align	2
.L13:
	.word	1073872896
	.word	1073876992
	.cfi_endproc
.LFE34:
	.size	BOARD_PORT_STANDBY, .-BOARD_PORT_STANDBY
	.section	.text.BOARD_POWER_STARTUP,"ax",%progbits
	.align	1
	.global	BOARD_POWER_STARTUP
	.thumb
	.thumb_func
	.type	BOARD_POWER_STARTUP, %function
BOARD_POWER_STARTUP:
.LFB35:
	.loc 1 802 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 1 808 0
	bx	lr	@
	.cfi_endproc
.LFE35:
	.size	BOARD_POWER_STARTUP, .-BOARD_POWER_STARTUP
	.section	.text.BOARD_XTAL_STARTUP,"ax",%progbits
	.align	1
	.global	BOARD_XTAL_STARTUP
	.thumb
	.thumb_func
	.type	BOARD_XTAL_STARTUP, %function
BOARD_XTAL_STARTUP:
.LFB36:
	.loc 1 817 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 1 820 0
	bx	lr	@
	.cfi_endproc
.LFE36:
	.size	BOARD_XTAL_STARTUP, .-BOARD_XTAL_STARTUP
	.section	.text.NMI_Handler,"ax",%progbits
	.align	1
	.global	NMI_Handler
	.thumb
	.thumb_func
	.type	NMI_Handler, %function
NMI_Handler:
.LFB37:
	.file 2 "B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c"
	.loc 2 225 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 229 0
	ldr	r3, .L20	@ tmp135,
	ldrh	r0, [r3, #0]	@ D.7814, platform.error_code
	cbnz	r0, .L18	@ D.7814,
.L19:
	b	.L19	@
.L18:
.LVL0:
	.loc 2 234 0
	movs	r2, #0	@,
	.loc 2 235 0
	uxtb	r0, r0	@, D.7814
.LVL1:
	.loc 2 234 0
	strh	r2, [r3, #0]	@ movhi	@, platform.error_code
	.loc 2 236 0
	.loc 2 235 0
	b	sys_panic	@
.L21:
	.align	2
.L20:
	.word	platform
	.cfi_endproc
.LFE37:
	.size	NMI_Handler, .-NMI_Handler
	.section	.text.HardFault_Handler,"ax",%progbits
	.align	1
	.global	HardFault_Handler
	.thumb
	.thumb_func
	.type	HardFault_Handler, %function
HardFault_Handler:
.LFB38:
	.loc 2 239 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.L23:
	b	.L23	@
	.cfi_endproc
.LFE38:
	.size	HardFault_Handler, .-HardFault_Handler
	.section	.text.MemManage_Handler,"ax",%progbits
	.align	1
	.global	MemManage_Handler
	.thumb
	.thumb_func
	.type	MemManage_Handler, %function
MemManage_Handler:
.LFB39:
	.loc 2 248 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.L25:
	b	.L25	@
	.cfi_endproc
.LFE39:
	.size	MemManage_Handler, .-MemManage_Handler
	.section	.text.BusFault_Handler,"ax",%progbits
	.align	1
	.global	BusFault_Handler
	.thumb
	.thumb_func
	.type	BusFault_Handler, %function
BusFault_Handler:
.LFB40:
	.loc 2 257 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.L27:
	b	.L27	@
	.cfi_endproc
.LFE40:
	.size	BusFault_Handler, .-BusFault_Handler
	.section	.text.UsageFault_Handler,"ax",%progbits
	.align	1
	.global	UsageFault_Handler
	.thumb
	.thumb_func
	.type	UsageFault_Handler, %function
UsageFault_Handler:
.LFB41:
	.loc 2 266 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.L29:
	b	.L29	@
	.cfi_endproc
.LFE41:
	.size	UsageFault_Handler, .-UsageFault_Handler
	.section	.text.DebugMon_Handler,"ax",%progbits
	.align	1
	.global	DebugMon_Handler
	.thumb
	.thumb_func
	.type	DebugMon_Handler, %function
DebugMon_Handler:
.LFB42:
	.loc 2 275 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 275 0
	bx	lr	@
	.cfi_endproc
.LFE42:
	.size	DebugMon_Handler, .-DebugMon_Handler
	.section	.text.SVC_Handler,"ax",%progbits
	.align	1
	.global	SVC_Handler
	.thumb
	.thumb_func
	.type	SVC_Handler, %function
SVC_Handler:
.LFB43:
	.loc 2 282 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r3, lr}	@
.LCFI1:
	.cfi_def_cfa_offset 8
	.loc 2 287 0
	.cfi_offset 14, -4
	.cfi_offset 3, -8
	bl	sys_event_manager	@
	ldr	r3, .L34	@ tmp138,
	strh	r0, [r3, #170]	@ movhi	@ tmp137, platform_ext.next_evt
	.loc 2 288 0
	ldr	r3, .L34+4	@ tmp140,
	cmp	r0, #0	@ tmp137,
	ite	eq	@
	moveq	r0, #268435456	@, iftmp.16,
	movne	r0, #0	@, iftmp.16,
	ldr	r2, [r3, #4]	@ D.7807, 3758157056B->ICSR
	orrs	r0, r0, r2	@, D.7813, iftmp.16, D.7807
	str	r0, [r3, #4]	@ D.7813, 3758157056B->ICSR
	.loc 2 320 0
	pop	{r3, pc}	@
.L35:
	.align	2
.L34:
	.word	platform_ext
	.word	-536810240
	.cfi_endproc
.LFE43:
	.size	SVC_Handler, .-SVC_Handler
	.section	.text.OT_GPTIM_ISR,"ax",%progbits
	.align	1
	.global	OT_GPTIM_ISR
	.thumb
	.thumb_func
	.type	OT_GPTIM_ISR, %function
OT_GPTIM_ISR:
.LFB44:
	.loc 2 323 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 327 0
	.loc 2 326 0
	b	SET_PENDSV	@
	.cfi_endproc
.LFE44:
	.size	OT_GPTIM_ISR, .-OT_GPTIM_ISR
	.section	.text.PendSV_Handler,"ax",%progbits
	.align	1
	.global	PendSV_Handler
	.thumb
	.thumb_func
	.type	PendSV_Handler, %function
PendSV_Handler:
.LFB45:
	.loc 2 330 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 346 0
	.loc 2 339 0
	b	CLR_PENDSV	@
	.cfi_endproc
.LFE45:
	.size	PendSV_Handler, .-PendSV_Handler
	.section	.text.WWDG_IRQHandler,"ax",%progbits
	.align	1
	.global	WWDG_IRQHandler
	.thumb
	.thumb_func
	.type	WWDG_IRQHandler, %function
WWDG_IRQHandler:
.LFB46:
	.loc 2 359 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 360 0
	bx	lr	@
	.cfi_endproc
.LFE46:
	.size	WWDG_IRQHandler, .-WWDG_IRQHandler
	.section	.text.platform_poweroff,"ax",%progbits
	.align	1
	.global	platform_poweroff
	.thumb
	.thumb_func
	.type	platform_poweroff, %function
platform_poweroff:
.LFB48:
	.loc 2 472 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}	@
.LCFI2:
	.cfi_def_cfa_offset 8
	.loc 2 473 0
	.cfi_offset 14, -4
	.cfi_offset 4, -8
	bl	ISF_syncmirror	@
	.loc 2 479 0
	pop	{r4, lr}	@
	.loc 2 474 0
	b	vworm_save	@
	.cfi_endproc
.LFE48:
	.size	platform_poweroff, .-platform_poweroff
	.section	.text.platform_init_OT,"ax",%progbits
	.align	1
	.global	platform_init_OT
	.thumb
	.thumb_func
	.type	platform_init_OT, %function
platform_init_OT:
.LFB49:
	.loc 2 485 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}	@
.LCFI3:
	.cfi_def_cfa_offset 16
	.loc 2 489 0
	.cfi_offset 14, -4
	.cfi_offset 6, -8
	.cfi_offset 5, -12
	.cfi_offset 4, -16
	bl	buffers_init	@
	.loc 2 490 0
	bl	vl_init	@
	.loc 2 500 0
	bl	sys_init	@
.LBB4:
	.loc 2 513 0
	movs	r0, #1	@,
	bl	ISF_open_su	@
	.loc 2 514 0
	ldr	r4, .L42	@ hwid,
	.loc 2 513 0
	mov	r5, r0	@ fpid,
.LVL2:
	.loc 2 515 0
	movs	r6, #6	@ i,
.LVL3:
.L41:
	.loc 2 513 0 discriminator 2
	uxth	r6, r6	@ i.14, i
	.loc 2 516 0 discriminator 2
	mov	r1, r6	@, i.14
	ldrh	r2, [r4], #2	@,
.LVL4:
	mov	r0, r5	@, fpid
	bl	vl_write	@
	.loc 2 515 0 discriminator 2
	ldr	r3, .L42+4	@ tmp192,
	.loc 2 485 0 discriminator 2
	subs	r6, r6, #2	@ tmp191, i.14,
.LVL5:
	.loc 2 515 0 discriminator 2
	cmp	r4, r3	@ hwid, tmp192
	bne	.L41	@,
	.loc 2 518 0
	mov	r0, r5	@, fpid
.LBE4:
	.loc 2 521 0
	pop	{r4, r5, r6, lr}	@
.LBB5:
	.loc 2 518 0
	b	vl_close	@
.L43:
	.align	2
.L42:
	.word	536346704
	.word	536346710
.LBE5:
	.cfi_endproc
.LFE49:
	.size	platform_init_OT, .-platform_init_OT
	.section	.text.sub_msflash_config,"ax",%progbits
	.align	1
	.global	sub_msflash_config
	.thumb
	.thumb_func
	.type	sub_msflash_config, %function
sub_msflash_config:
.LFB50:
	.loc 2 525 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 528 0
	ldr	r3, .L45	@ tmp137,
	ldr	r2, [r3, #0]	@ D.7788, 1073888256B->ACR
	orr	r2, r2, #4	@ D.7789, D.7788,
	str	r2, [r3, #0]	@ D.7789, 1073888256B->ACR
	.loc 2 531 0
	ldr	r2, [r3, #0]	@ D.7790, 1073888256B->ACR
	orr	r2, r2, #2	@ D.7791, D.7790,
	str	r2, [r3, #0]	@ D.7791, 1073888256B->ACR
	.loc 2 535 0
	bx	lr	@
.L46:
	.align	2
.L45:
	.word	1073888256
	.cfi_endproc
.LFE50:
	.size	sub_msflash_config, .-sub_msflash_config
	.section	.text.sub_hsflash_config,"ax",%progbits
	.align	1
	.global	sub_hsflash_config
	.thumb
	.thumb_func
	.type	sub_hsflash_config, %function
sub_hsflash_config:
.LFB51:
	.loc 2 537 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 540 0
	ldr	r3, .L48	@ tmp137,
	ldr	r2, [r3, #0]	@ D.7783, 1073888256B->ACR
	orr	r2, r2, #4	@ D.7784, D.7783,
	str	r2, [r3, #0]	@ D.7784, 1073888256B->ACR
	.loc 2 546 0
	ldr	r2, [r3, #0]	@ D.7785, 1073888256B->ACR
	orr	r2, r2, #3	@ D.7786, D.7785,
	str	r2, [r3, #0]	@ D.7786, 1073888256B->ACR
	.loc 2 548 0
	bx	lr	@
.L49:
	.align	2
.L48:
	.word	1073888256
	.cfi_endproc
.LFE51:
	.size	sub_hsflash_config, .-sub_hsflash_config
	.section	.text.sub_voltage_config,"ax",%progbits
	.align	1
	.global	sub_voltage_config
	.thumb
	.thumb_func
	.type	sub_voltage_config, %function
sub_voltage_config:
.LFB52:
	.loc 2 551 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 553 0
	ldr	r3, .L52	@ tmp137,
	ldr	r2, [r3, #36]	@ D.7777, 1073887232B->APB1ENR
	orr	r2, r2, #268435456	@ D.7778, D.7777,
	str	r2, [r3, #36]	@ D.7778, 1073887232B->APB1ENR
	.loc 2 557 0
	ldr	r3, .L52+4	@ tmp139,
	mov	r2, #4096	@ tmp140,
	str	r2, [r3, #0]	@ tmp140, 1073770496B->CR
.L51:
	.loc 2 565 0 discriminator 1
	ldr	r3, .L52+4	@ tmp141,
	ldr	r3, [r3, #4]	@ D.7780, 1073770496B->CSR
	tst	r3, #16	@ D.7780,
	bne	.L51	@,
	.loc 2 566 0
	bx	lr	@
.L53:
	.align	2
.L52:
	.word	1073887232
	.word	1073770496
	.cfi_endproc
.LFE52:
	.size	sub_voltage_config, .-sub_voltage_config
	.section	.text.sub_hsosc_config,"ax",%progbits
	.align	1
	.global	sub_hsosc_config
	.thumb
	.thumb_func
	.type	sub_hsosc_config, %function
sub_hsosc_config:
.LFB53:
	.loc 2 569 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 589 0
	ldr	r3, .L56	@ tmp137,
	ldr	r2, [r3, #8]	@ D.7772, 1073887232B->CFGR
	orr	r2, r2, #1	@ D.7773, D.7772,
	str	r2, [r3, #8]	@ D.7773, 1073887232B->CFGR
.L55:
	.loc 2 590 0 discriminator 1
	ldr	r3, .L56	@ tmp139,
	ldr	r3, [r3, #8]	@ D.7774, 1073887232B->CFGR
	and	r3, r3, #12	@ tmp140, D.7774,
	cmp	r3, #8	@ tmp140,
	bne	.L55	@,
	.loc 2 594 0
	ldr	r3, .L56+4	@ tmp141,
	mov	r2, #16000	@ movhi	@,
	strh	r2, [r3, #172]	@ movhi	@, platform_ext.cpu_khz
	.loc 2 595 0
	bx	lr	@
.L57:
	.align	2
.L56:
	.word	1073887232
	.word	platform_ext
	.cfi_endproc
.LFE53:
	.size	sub_hsosc_config, .-sub_hsosc_config
	.section	.text.platform_init_busclk,"ax",%progbits
	.align	1
	.global	platform_init_busclk
	.thumb
	.thumb_func
	.type	platform_init_busclk, %function
platform_init_busclk:
.LFB54:
	.loc 2 600 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}	@
.LCFI4:
	.cfi_def_cfa_offset 8
	.loc 2 606 0
	ldr	r4, .L59	@ tmp146,
	.cfi_offset 14, -4
	.cfi_offset 4, -8
	ldr	r3, [r4, #0]	@ D.7757, 1073887232B->CR
	orr	r3, r3, #256	@ D.7758, D.7757,
	str	r3, [r4, #0]	@ D.7758, 1073887232B->CR
	.loc 2 607 0
	ldr	r2, [r4, #8]	@ D.7759, 1073887232B->CFGR
	ldr	r3, .L59+4	@ D.7760,
	and	r3, r2, r3	@ D.7760, D.7759, D.7760
	str	r3, [r4, #8]	@ D.7760, 1073887232B->CFGR
	.loc 2 609 0
	ldr	r2, [r4, #0]	@ D.7761, 1073887232B->CR
	ldr	r3, .L59+8	@ D.7762,
	and	r3, r2, r3	@ D.7762, D.7761, D.7762
	str	r3, [r4, #0]	@ D.7762, 1073887232B->CR
	.loc 2 610 0
	ldr	r3, [r4, #0]	@ D.7763, 1073887232B->CR
	.loc 2 632 0
	movw	r2, 4200	@ movhi	@,
	.loc 2 610 0
	bic	r3, r3, #262144	@ D.7764, D.7763,
	str	r3, [r4, #0]	@ D.7764, 1073887232B->CR
	.loc 2 611 0
	ldr	r3, [r4, #8]	@ D.7765, 1073887232B->CFGR
	bic	r3, r3, #16580608	@ D.7766, D.7765,
	str	r3, [r4, #8]	@ D.7766, 1073887232B->CFGR
	.loc 2 612 0
	movs	r3, #0	@ tmp159,
	str	r3, [r4, #12]	@ tmp159, 1073887232B->CIR
	.loc 2 632 0
	ldr	r3, .L59+12	@ tmp160,
	strh	r2, [r3, #172]	@ movhi	@, platform_ext.cpu_khz
	.loc 2 636 0
	bl	sub_msflash_config	@
	.loc 2 637 0
	bl	sub_voltage_config	@
	.loc 2 640 0
	ldr	r3, [r4, #8]	@ D.7767, 1073887232B->CFGR
	.loc 2 718 0
	mov	r2, #134217728	@ tmp168,
	.loc 2 640 0
	str	r3, [r4, #8]	@ D.7767, 1073887232B->CFGR
	.loc 2 644 0
	ldr	r3, [r4, #4]	@ D.7768, 1073887232B->ICSCR
	eor	r3, r3, #24576	@ D.7769, D.7768,
	str	r3, [r4, #4]	@ D.7769, 1073887232B->ICSCR
	.loc 2 718 0
	ldr	r3, .L59+16	@ tmp167,
	str	r2, [r3, #8]	@ tmp168, 3758157056B->VTOR
	.loc 2 720 0
	pop	{r4, pc}	@
.L60:
	.align	2
.L59:
	.word	1073887232
	.word	-1996505076
	.word	-285278210
	.word	platform_ext
	.word	-536810240
	.cfi_endproc
.LFE54:
	.size	platform_init_busclk, .-platform_init_busclk
	.section	.text.platform_init_periphclk,"ax",%progbits
	.align	1
	.global	platform_init_periphclk
	.thumb
	.thumb_func
	.type	platform_init_periphclk, %function
platform_init_periphclk:
.LFB55:
	.loc 2 726 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 728 0
	bx	lr	@
	.cfi_endproc
.LFE55:
	.size	platform_init_periphclk, .-platform_init_periphclk
	.section	.text.platform_disable_interrupts,"ax",%progbits
	.align	1
	.global	platform_disable_interrupts
	.thumb
	.thumb_func
	.type	platform_disable_interrupts, %function
platform_disable_interrupts:
.LFB56:
	.loc 2 734 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LBB8:
.LBB9:
	.file 3 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\CoreSupport/core_cm3.h"
	.loc 3 1205 0
@ 1205 "B:\otplatform\stm32_mculib\CMSIS\CM3\CoreSupport/core_cm3.h" 1
	cpsid i
@ 0 "" 2
	.thumb
.LBE9:
.LBE8:
	.loc 2 736 0
	bx	lr	@
	.cfi_endproc
.LFE56:
	.size	platform_disable_interrupts, .-platform_disable_interrupts
	.section	.text.platform_enable_interrupts,"ax",%progbits
	.align	1
	.global	platform_enable_interrupts
	.thumb
	.thumb_func
	.type	platform_enable_interrupts, %function
platform_enable_interrupts:
.LFB57:
	.loc 2 741 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LBB12:
.LBB13:
	.loc 3 1204 0
@ 1204 "B:\otplatform\stm32_mculib\CMSIS\CM3\CoreSupport/core_cm3.h" 1
	cpsie i
@ 0 "" 2
	.thumb
.LBE13:
.LBE12:
	.loc 2 743 0
	bx	lr	@
	.cfi_endproc
.LFE57:
	.size	platform_enable_interrupts, .-platform_enable_interrupts
	.section	.text.platform_standard_speed,"ax",%progbits
	.align	1
	.global	platform_standard_speed
	.thumb
	.thumb_func
	.type	platform_standard_speed, %function
platform_standard_speed:
.LFB58:
	.loc 2 757 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 763 0
	ldr	r3, .L69	@ tmp150,
	ldr	r2, [r3, #0]	@ D.7738, 1073887232B->CR
	tst	r2, #256	@ D.7738,
	bne	.L64	@,
.LBB14:
	.loc 2 767 0
	ldr	r2, [r3, #0]	@ D.7742, 1073887232B->CR
	orr	r2, r2, #256	@ D.7743, D.7742,
	str	r2, [r3, #0]	@ D.7743, 1073887232B->CR
.LVL6:
	.loc 2 768 0
	mov	r3, #300	@ counter,
.LVL7:
.L67:
	.loc 2 769 0 discriminator 1
	ldr	r2, .L69	@ tmp154,
	ldr	r2, [r2, #0]	@ D.7744, 1073887232B->CR
	tst	r2, #512	@ D.7744,
	bne	.L66	@,
	.loc 2 769 0 is_stmt 0 discriminator 2
	subs	r3, r3, #1	@ tmp156, counter,
.LVL8:
	uxth	r3, r3	@ counter, tmp156
.LVL9:
	cmp	r3, #0	@ counter
	bne	.L67	@
.LVL10:
.L66:
	.loc 2 773 0 is_stmt 1
	ldr	r3, .L69	@ tmp157,
	ldr	r2, [r3, #8]	@ D.7747, 1073887232B->CFGR
	bic	r2, r2, #3	@ D.7748, D.7747,
	str	r2, [r3, #8]	@ D.7748, 1073887232B->CFGR
.L68:
	.loc 2 774 0 discriminator 1
	ldr	r3, .L69	@ tmp159,
	ldr	r2, [r3, #8]	@ D.7749, 1073887232B->CFGR
	tst	r2, #12	@ D.7749,
	bne	.L68	@,
	.loc 2 777 0
	ldr	r1, [r3, #0]	@ D.7751, 1073887232B->CR
	ldr	r2, .L69+4	@ D.7752,
	and	r2, r1, r2	@ D.7752, D.7751, D.7752
	str	r2, [r3, #0]	@ D.7752, 1073887232B->CR
	.loc 2 785 0
	ldr	r3, .L69+8	@ tmp164,
	ldr	r2, [r3, #0]	@ D.7754, 1073888256B->ACR
	bic	r2, r2, #1	@ D.7755, D.7754,
	str	r2, [r3, #0]	@ D.7755, 1073888256B->ACR
	.loc 2 789 0
	ldr	r3, .L69+12	@ tmp166,
	movw	r2, 4200	@ movhi	@,
	strh	r2, [r3, #172]	@ movhi	@, platform_ext.cpu_khz
.L64:
	bx	lr	@
.L70:
	.align	2
.L69:
	.word	1073887232
	.word	-16842754
	.word	1073888256
	.word	platform_ext
.LBE14:
	.cfi_endproc
.LFE58:
	.size	platform_standard_speed, .-platform_standard_speed
	.section	.text.platform_full_speed,"ax",%progbits
	.align	1
	.global	platform_full_speed
	.thumb
	.thumb_func
	.type	platform_full_speed, %function
platform_full_speed:
.LFB59:
	.loc 2 794 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}	@
.LCFI5:
	.cfi_def_cfa_offset 8
	.loc 2 796 0
	ldr	r4, .L73	@ tmp139,
	.cfi_offset 14, -4
	.cfi_offset 4, -8
	ldr	r3, [r4, #0]	@ D.7728, 1073887232B->CR
	tst	r3, #256	@ D.7728,
	beq	.L71	@,
	.loc 2 803 0
	ldr	r3, .L73+4	@ tmp141,
	ldr	r2, [r3, #0]	@ D.7733, 1073888256B->ACR
	orr	r2, r2, #1	@ D.7734, D.7733,
	str	r2, [r3, #0]	@ D.7734, 1073888256B->ACR
	.loc 2 808 0
	bl	sub_hsosc_config	@
	.loc 2 811 0
	ldr	r3, [r4, #0]	@ D.7735, 1073887232B->CR
	bic	r3, r3, #256	@ D.7736, D.7735,
	str	r3, [r4, #0]	@ D.7736, 1073887232B->CR
.L71:
	pop	{r4, pc}	@
.L74:
	.align	2
.L73:
	.word	1073887232
	.word	1073888256
	.cfi_endproc
.LFE59:
	.size	platform_full_speed, .-platform_full_speed
	.section	.text.platform_flank_speed,"ax",%progbits
	.align	1
	.global	platform_flank_speed
	.thumb
	.thumb_func
	.type	platform_flank_speed, %function
platform_flank_speed:
.LFB60:
	.loc 2 815 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 819 0
	.loc 2 818 0
	b	platform_full_speed	@
	.cfi_endproc
.LFE60:
	.size	platform_flank_speed, .-platform_flank_speed
	.section	.text.platform_save_context,"ax",%progbits
	.align	1
	.global	platform_save_context
	.thumb
	.thumb_func
	.type	platform_save_context, %function
platform_save_context:
.LFB61:
	.loc 2 831 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 834 0
@ 834 "B:\otplatform\stm32l1xx\platform_STM32L1xx.c" 1
	   MRS    r0, psp	@ tsp
   STMDB  r0!, {r4-r11}	@ tsp
   MSR    psp, r0	@ tsp

@ 0 "" 2
.LVL11:
	.loc 2 841 0
	.thumb
	bx	lr	@
	.cfi_endproc
.LFE61:
	.size	platform_save_context, .-platform_save_context
	.section	.text.platform_load_context,"ax",%progbits
	.align	1
	.global	platform_load_context
	.thumb
	.thumb_func
	.type	platform_load_context, %function
platform_load_context:
.LFB62:
	.loc 2 845 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL12:
	.loc 2 848 0
@ 848 "B:\otplatform\stm32l1xx\platform_STM32L1xx.c" 1
	   MRS    r3, psp	@ scratch
   LDMFD  r3!, {r4-r11}	@ scratch
   MSR    psp, r3	@ scratch

@ 0 "" 2
	.loc 2 854 0
	.thumb
	bx	lr	@
	.cfi_endproc
.LFE62:
	.size	platform_load_context, .-platform_load_context
	.section	.text.platform_switch_context,"ax",%progbits
	.align	1
	.global	platform_switch_context
	.thumb
	.thumb_func
	.type	platform_switch_context, %function
platform_switch_context:
.LFB63:
	.loc 2 858 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL13:
	.loc 2 859 0
	bx	lr	@
	.cfi_endproc
.LFE63:
	.size	platform_switch_context, .-platform_switch_context
	.section	.text.platform_drop_context,"ax",%progbits
	.align	1
	.global	platform_drop_context
	.thumb
	.thumb_func
	.type	platform_drop_context, %function
platform_drop_context:
.LFB64:
	.loc 2 863 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL14:
	.loc 2 874 0
	ldr	r3, .L81	@ tmp139,
	ldr	r3, [r3, #164]	@ D.7719, platform_ext.task_exit
	cbz	r3, .L79	@ D.7719,
.LBB15:
	.loc 2 879 0
@ 879 "B:\otplatform\stm32l1xx\platform_STM32L1xx.c" 1
	MRS  r2, PSP	@ task_lr
@ 0 "" 2
.LVL15:
	.loc 2 880 0
	.thumb
	str	r3, [r2, #24]	@ D.7719,
.LVL16:
.L79:
	bx	lr	@
.L82:
	.align	2
.L81:
	.word	platform_ext
.LBE15:
	.cfi_endproc
.LFE64:
	.size	platform_drop_context, .-platform_drop_context
	.section	.text.platform_ot_preempt,"ax",%progbits
	.align	1
	.global	platform_ot_preempt
	.thumb
	.thumb_func
	.type	platform_ot_preempt, %function
platform_ot_preempt:
.LFB65:
	.loc 2 887 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 893 0
	ldr	r3, .L85	@ tmp135,
	ldr	r3, [r3, #0]	@ D.7715, 1073808384B->IMR
	tst	r3, #32	@ D.7715,
	beq	.L83	@,
	.loc 2 894 0
@ 894 "B:\otplatform\stm32l1xx\platform_STM32L1xx.c" 1
	 svc  #0	@
@ 0 "" 2
	.thumb
.L83:
	bx	lr	@
.L86:
	.align	2
.L85:
	.word	1073808384
	.cfi_endproc
.LFE65:
	.size	platform_ot_preempt, .-platform_ot_preempt
	.section	.text.platform_ot_run,"ax",%progbits
	.align	1
	.global	platform_ot_run
	.thumb
	.thumb_func
	.type	platform_ot_run, %function
platform_ot_run:
.LFB66:
	.loc 2 909 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
.L88:
	push	{r4, lr}	@
.LCFI6:
	.cfi_def_cfa_offset 8
	.loc 2 920 0
	.cfi_offset 14, -4
	.cfi_offset 4, -8
	bl	__get_PSP	@
	ldr	r3, .L90	@ tmp139,
	str	r0, [r3, #164]	@, platform_ext.task_exit
	.loc 2 922 0
	ldr	r3, .L90+4	@ tmp140,
@ 922 "B:\otplatform\stm32l1xx\platform_STM32L1xx.c" 1
	PUSH {r3}	@ tmp140
@ 0 "" 2
	.loc 2 932 0
	.thumb
	ldr	r4, .L90	@ tmp141,
	.loc 2 926 0
	bl	sys_run_task	@
	.loc 2 932 0
	ldr	r0, [r4, #164]	@, platform_ext.task_exit
	bl	__set_PSP	@
	.loc 2 933 0
	movs	r3, #0	@ tmp143,
	str	r3, [r4, #164]	@ tmp143, platform_ext.task_exit
	.loc 2 939 0
@ 939 "B:\otplatform\stm32l1xx\platform_STM32L1xx.c" 1
	 svc  #0	@
@ 0 "" 2
	.loc 2 946 0
	.thumb
	ldrh	r3, [r4, #170]	@ platform_ext.next_evt, platform_ext.next_evt
	cbz	r3, .L87	@ platform_ext.next_evt,
	.loc 2 947 0
	bl	platform_disable_interrupts	@
	.loc 2 950 0
	pop	{r4, lr}	@
	.loc 2 948 0
	b	sys_powerdown	@
.L87:
	pop	{r4, pc}	@
.L91:
	.align	2
.L90:
	.word	platform_ext
	.word	.L88
	.cfi_endproc
.LFE66:
	.size	platform_ot_run, .-platform_ot_run
	.section	.text.platform_init_interruptor,"ax",%progbits
	.align	1
	.global	platform_init_interruptor
	.thumb
	.thumb_func
	.type	platform_init_interruptor, %function
platform_init_interruptor:
.LFB67:
	.loc 2 964 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL17:
.LBB18:
.LBB19:
	.loc 3 1473 0
	ldr	r3, .L93	@ tmp136,
	.loc 3 1474 0
	movw	r1, #63743	@ reg_value,
	.loc 3 1473 0
	ldr	r2, [r3, #12]	@ reg_value, 3758157056B->AIRCR
.LVL18:
	.loc 3 1474 0
	and	r1, r2, r1	@ reg_value, reg_value, reg_value
.LVL19:
	.loc 3 1475 0
	ldr	r2, .L93+4	@ reg_value,
	orr	r2, r1, r2	@ reg_value, reg_value, reg_value
.LVL20:
	.loc 3 1478 0
	str	r2, [r3, #12]	@ reg_value, 3758157056B->AIRCR
.LBE19:
.LBE18:
	.loc 2 1014 0
	movs	r2, #0	@ tmp141,
.LVL21:
	strb	r2, [r3, #31]	@ tmp141, 3758157056B->SHP
	.loc 2 1016 0
	adds	r2, r2, #240	@ tmp144, tmp144,
	strb	r2, [r3, #34]	@ tmp144, 3758157056B->SHP
	.loc 2 1017 0
	subs	r2, r2, #112	@ tmp147, tmp147,
	strb	r2, [r3, #35]	@ tmp147, 3758157056B->SHP
	.loc 2 1041 0
	ldr	r3, .L93+8	@ tmp149,
.LVL22:
	subs	r2, r2, #112	@ tmp150, tmp150,
	strb	r2, [r3, #791]	@ tmp150, 3758153984B->IP
	.loc 2 1042 0
	mov	r2, #8388608	@ tmp153,
	str	r2, [r3, #0]	@ tmp153, 3758153984B->ISER
	.loc 2 1071 0
	bx	lr	@
.L94:
	.align	2
.L93:
	.word	-536810240
	.word	100271616
	.word	-536813312
	.cfi_endproc
.LFE67:
	.size	platform_init_interruptor, .-platform_init_interruptor
	.section	.text.platform_init_gpio,"ax",%progbits
	.align	1
	.global	platform_init_gpio
	.thumb
	.thumb_func
	.type	platform_init_gpio, %function
platform_init_gpio:
.LFB68:
	.loc 2 1082 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1089 0
	.loc 2 1088 0
	b	BOARD_PORT_STARTUP	@
	.cfi_endproc
.LFE68:
	.size	platform_init_gpio, .-platform_init_gpio
	.section	.text.platform_init_gptim,"ax",%progbits
	.align	1
	.global	platform_init_gptim
	.thumb
	.thumb_func
	.type	platform_init_gptim, %function
platform_init_gptim:
.LFB69:
	.loc 2 1095 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL23:
	.loc 2 1123 0
	ldr	r3, .L97	@ tmp138,
	.loc 2 1126 0
	mov	r0, #16384	@ movhi	@,
.LVL24:
	.loc 2 1123 0
	ldr	r2, [r3, #8]	@ D.7697, 1073808384B->RTSR
	.loc 2 1134 0
	movs	r1, #16	@,
	.loc 2 1123 0
	orr	r2, r2, #96	@ D.7698, D.7697,
	str	r2, [r3, #8]	@ D.7698, 1073808384B->RTSR
	.loc 2 1126 0
	ldr	r3, .L97+4	@ tmp140,
	.loc 2 1127 0
	movs	r2, #0	@ tmp145,
	.loc 2 1126 0
	strh	r0, [r3, #8]	@ movhi	@, 1073809408B->SMCR
	.loc 2 1127 0
	strh	r2, [r3, #12]	@ movhi	@ tmp145, 1073809408B->DIER
	.loc 2 1137 0
	movs	r0, #31	@,
	.loc 2 1128 0
	strh	r2, [r3, #16]	@ movhi	@ tmp145, 1073809408B->SR
	.loc 2 1134 0
	strh	r1, [r3, #24]	@ movhi	@, 1073809408B->CCMR1
	.loc 2 1135 0
	movs	r1, #1	@ tmp154,
	strh	r1, [r3, #32]	@ movhi	@ tmp154, 1073809408B->CCER
	.loc 2 1137 0
	strh	r0, [r3, #40]	@ movhi	@, 1073809408B->PSC
	.loc 2 1138 0
	movw	r0, #65535	@ tmp159,
	str	r0, [r3, #44]	@ tmp159, 1073809408B->ARR
	.loc 2 1139 0
	strh	r2, [r3, #80]	@ movhi	@ tmp145, 1073809408B->OR
	.loc 2 1142 0
	strh	r1, [r3, #20]	@ movhi	@ tmp154, 1073809408B->EGR
	.loc 2 1143 0
	ldrh	r2, [r3, #0]	@, 1073809408B->CR1
	uxth	r2, r2	@ D.7700, 1073809408B->CR1
	orrs	r2, r2, r1	@, D.7701, D.7700,
	strh	r2, [r3, #0]	@ movhi	@ D.7701, 1073809408B->CR1
	.loc 2 1144 0
	bx	lr	@
.L98:
	.align	2
.L97:
	.word	1073808384
	.word	1073809408
	.cfi_endproc
.LFE69:
	.size	platform_init_gptim, .-platform_init_gptim
	.section	.text.platform_poweron,"ax",%progbits
	.align	1
	.global	platform_poweron
	.thumb
	.thumb_func
	.type	platform_poweron, %function
platform_poweron:
.LFB47:
	.loc 2 429 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}	@
.LCFI7:
	.cfi_def_cfa_offset 8
	.loc 2 432 0
	.cfi_offset 14, -4
	.cfi_offset 4, -8
	bl	__get_MSP	@
	bl	__set_PSP	@
	.loc 2 433 0
	movs	r0, #2	@,
	bl	__set_CONTROL	@
	.loc 2 434 0
	ldr	r0, .L100	@,
	bl	__set_MSP	@
	.loc 2 437 0
	bl	platform_init_busclk	@
	.loc 2 438 0
	bl	platform_init_perihclk	@
	.loc 2 441 0
	bl	BOARD_PERIPH_INIT	@
	.loc 2 446 0
	ldr	r3, .L100+4	@ tmp141,
	ldr	r2, [r3, #4]	@ D.7799, 3758366720B->CR
	orr	r2, r2, #7	@ D.7800, D.7799,
	str	r2, [r3, #4]	@ D.7800, 3758366720B->CR
	.loc 2 450 0
	ldr	r2, [r3, #8]	@ D.7801, 3758366720B->APB1FZ
	orr	r2, r2, #7168	@ D.7802, D.7801,
	str	r2, [r3, #8]	@ D.7802, 3758366720B->APB1FZ
	.loc 2 454 0
	ldr	r2, [r3, #12]	@ D.7803, 3758366720B->APB2FZ
	orr	r2, r2, #28	@ D.7804, D.7803,
	str	r2, [r3, #12]	@ D.7804, 3758366720B->APB2FZ
	.loc 2 460 0
	bl	platform_init_gpio	@
	.loc 2 461 0
	bl	platform_init_interruptor	@
	.loc 2 462 0
	movs	r0, #0	@,
	bl	platform_init_gptim	@
	.loc 2 467 0
	pop	{r4, lr}	@
	.loc 2 466 0
	b	vworm_init	@
.L101:
	.align	2
.L100:
	.word	platform_ext
	.word	-536600576
	.cfi_endproc
.LFE47:
	.size	platform_poweron, .-platform_poweron
	.section	.text.platform_init_watchdog,"ax",%progbits
	.align	1
	.global	platform_init_watchdog
	.thumb
	.thumb_func
	.type	platform_init_watchdog, %function
platform_init_watchdog:
.LFB70:
	.loc 2 1149 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1153 0
	bx	lr	@
	.cfi_endproc
.LFE70:
	.size	platform_init_watchdog, .-platform_init_watchdog
	.section	.text.platform_init_resetswitch,"ax",%progbits
	.align	1
	.global	platform_init_resetswitch
	.thumb
	.thumb_func
	.type	platform_init_resetswitch, %function
platform_init_resetswitch:
.LFB71:
	.loc 2 1158 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1160 0
	bx	lr	@
	.cfi_endproc
.LFE71:
	.size	platform_init_resetswitch, .-platform_init_resetswitch
	.section	.text.platform_init_systick,"ax",%progbits
	.align	1
	.global	platform_init_systick
	.thumb
	.thumb_func
	.type	platform_init_systick, %function
platform_init_systick:
.LFB72:
	.loc 2 1165 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL25:
	.loc 2 1171 0
	ldr	r3, .L105	@ tmp134,
	movs	r2, #0	@ tmp135,
	str	r2, [r3, #8]	@ tmp135, 3758153744B->VAL
	.loc 2 1172 0
	adds	r2, r2, #7	@ tmp137, tmp137,
	str	r2, [r3, #0]	@ tmp137, 3758153744B->CTRL
	.loc 2 1176 0
	bx	lr	@
.L106:
	.align	2
.L105:
	.word	-536813552
	.cfi_endproc
.LFE72:
	.size	platform_init_systick, .-platform_init_systick
	.section	.text.platform_init_rtc,"ax",%progbits
	.align	1
	.global	platform_init_rtc
	.thumb
	.thumb_func
	.type	platform_init_rtc, %function
platform_init_rtc:
.LFB73:
	.loc 2 1181 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL26:
	.loc 2 1213 0
	bx	lr	@
	.cfi_endproc
.LFE73:
	.size	platform_init_rtc, .-platform_init_rtc
	.section	.text.platform_init_memcpy,"ax",%progbits
	.align	1
	.global	platform_init_memcpy
	.thumb
	.thumb_func
	.type	platform_init_memcpy, %function
platform_init_memcpy:
.LFB74:
	.loc 2 1217 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1220 0
	bx	lr	@
	.cfi_endproc
.LFE74:
	.size	platform_init_memcpy, .-platform_init_memcpy
	.section	.text.platform_get_ktim,"ax",%progbits
	.align	1
	.global	platform_get_ktim
	.thumb
	.thumb_func
	.type	platform_get_ktim, %function
platform_get_ktim:
.LFB75:
	.loc 2 1231 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1232 0
	ldr	r3, .L110	@ tmp138,
	ldr	r0, [r3, #36]	@ D.7689, 1073809408B->CNT
	ldr	r3, .L110+4	@ tmp140,
	ldrh	r3, [r3, #168]	@, platform_ext.last_evt
	subs	r0, r0, r3	@ tmp143, D.7689, platform_ext.last_evt
	.loc 2 1233 0
	uxth	r0, r0	@, tmp143
	bx	lr	@
.L111:
	.align	2
.L110:
	.word	1073809408
	.word	platform_ext
	.cfi_endproc
.LFE75:
	.size	platform_get_ktim, .-platform_get_ktim
	.section	.text.platform_next_ktim,"ax",%progbits
	.align	1
	.global	platform_next_ktim
	.thumb
	.thumb_func
	.type	platform_next_ktim, %function
platform_next_ktim:
.LFB76:
	.loc 2 1235 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1236 0
	ldr	r3, .L113	@ tmp139,
	ldr	r0, [r3, #52]	@ D.7682, 1073809408B->CCR1
	ldr	r3, [r3, #36]	@ D.7684, 1073809408B->CNT
	subs	r0, r0, r3	@ tmp144, D.7682, D.7684
	.loc 2 1237 0
	uxth	r0, r0	@, tmp144
	bx	lr	@
.L114:
	.align	2
.L113:
	.word	1073809408
	.cfi_endproc
.LFE76:
	.size	platform_next_ktim, .-platform_next_ktim
	.section	.text.platform_enable_ktim,"ax",%progbits
	.align	1
	.global	platform_enable_ktim
	.thumb
	.thumb_func
	.type	platform_enable_ktim, %function
platform_enable_ktim:
.LFB77:
	.loc 2 1239 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1240 0
	ldr	r3, .L116	@ tmp135,
	ldr	r2, [r3, #0]	@ D.7678, 1073808384B->IMR
	orr	r2, r2, #32	@ D.7679, D.7678,
	str	r2, [r3, #0]	@ D.7679, 1073808384B->IMR
	.loc 2 1241 0
	bx	lr	@
.L117:
	.align	2
.L116:
	.word	1073808384
	.cfi_endproc
.LFE77:
	.size	platform_enable_ktim, .-platform_enable_ktim
	.section	.text.platform_disable_ktim,"ax",%progbits
	.align	1
	.global	platform_disable_ktim
	.thumb
	.thumb_func
	.type	platform_disable_ktim, %function
platform_disable_ktim:
.LFB78:
	.loc 2 1243 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1244 0
	ldr	r3, .L119	@ tmp135,
	ldr	r2, [r3, #0]	@ D.7675, 1073808384B->IMR
	bic	r2, r2, #32	@ D.7676, D.7675,
	str	r2, [r3, #0]	@ D.7676, 1073808384B->IMR
	.loc 2 1245 0
	bx	lr	@
.L120:
	.align	2
.L119:
	.word	1073808384
	.cfi_endproc
.LFE78:
	.size	platform_disable_ktim, .-platform_disable_ktim
	.section	.text.platform_pend_ktim,"ax",%progbits
	.align	1
	.global	platform_pend_ktim
	.thumb
	.thumb_func
	.type	platform_pend_ktim, %function
platform_pend_ktim:
.LFB79:
	.loc 2 1247 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1248 0
	ldr	r3, .L122	@ tmp135,
	ldr	r2, [r3, #16]	@ D.7672, 1073808384B->SWIER
	orr	r2, r2, #32	@ D.7673, D.7672,
	str	r2, [r3, #16]	@ D.7673, 1073808384B->SWIER
	.loc 2 1249 0
	bx	lr	@
.L123:
	.align	2
.L122:
	.word	1073808384
	.cfi_endproc
.LFE79:
	.size	platform_pend_ktim, .-platform_pend_ktim
	.section	.text.platform_flush_ktim,"ax",%progbits
	.align	1
	.global	platform_flush_ktim
	.thumb
	.thumb_func
	.type	platform_flush_ktim, %function
platform_flush_ktim:
.LFB80:
	.loc 2 1251 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1252 0
	ldr	r3, .L125	@ tmp137,
	ldr	r2, [r3, #36]	@ D.7666, 1073809408B->CNT
	ldr	r3, .L125+4	@ tmp138,
	strh	r2, [r3, #168]	@ movhi	@ D.7666, platform_ext.last_evt
	.loc 2 1253 0
	ldr	r3, .L125+8	@ tmp140,
	ldr	r2, [r3, #0]	@ D.7669, 1073808384B->IMR
	bic	r2, r2, #32	@ D.7670, D.7669,
	str	r2, [r3, #0]	@ D.7670, 1073808384B->IMR
	.loc 2 1254 0
	bx	lr	@
.L126:
	.align	2
.L125:
	.word	1073809408
	.word	platform_ext
	.word	1073808384
	.cfi_endproc
.LFE80:
	.size	platform_flush_ktim, .-platform_flush_ktim
	.section	.text.platform_set_ktim,"ax",%progbits
	.align	1
	.global	platform_set_ktim
	.thumb
	.thumb_func
	.type	platform_set_ktim, %function
platform_set_ktim:
.LFB81:
	.loc 2 1256 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL27:
	.loc 2 1260 0
	ldr	r3, .L128	@ tmp140,
	ldr	r1, .L128+4	@ tmp141,
	ldr	r2, [r3, #36]	@ D.7658, 1073809408B->CNT
	uxth	r2, r2	@ D.7659, D.7658
	strh	r2, [r1, #168]	@ movhi	@ D.7659, platform_ext.last_evt
	.loc 2 1261 0
	adds	r2, r0, r2	@ D.7664, value, D.7659
	str	r2, [r3, #52]	@ D.7664, 1073809408B->CCR1
	.loc 2 1262 0
	bx	lr	@
.L129:
	.align	2
.L128:
	.word	1073809408
	.word	platform_ext
	.cfi_endproc
.LFE81:
	.size	platform_set_ktim, .-platform_set_ktim
	.section	.text.platform_set_gptim2,"ax",%progbits
	.align	1
	.global	platform_set_gptim2
	.thumb
	.thumb_func
	.type	platform_set_gptim2, %function
platform_set_gptim2:
.LFB82:
	.loc 2 1264 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL28:
	.loc 2 1266 0
	ldr	r3, .L133	@ tmp140,
	ldr	r2, [r3, #36]	@ D.7647, 1073809408B->CNT
	adds	r2, r0, r2	@ D.7649, value, D.7647
	str	r2, [r3, #56]	@ D.7649, 1073809408B->CCR2
	.loc 2 1267 0
	ldr	r3, .L133+4	@ tmp142,
	cmp	r0, #0	@ value,
	ite	ne	@
	movne	r0, #1	@, iftmp.13,
	moveq	r0, #2	@, iftmp.13,
.LVL29:
	ldr	r2, [r3, #16]	@ D.7651, 1073808384B->SWIER
	orrs	r0, r0, r2	@, D.7656, iftmp.13, D.7651
	str	r0, [r3, #16]	@ D.7656, 1073808384B->SWIER
	.loc 2 1268 0
	bx	lr	@
.L134:
	.align	2
.L133:
	.word	1073809408
	.word	1073808384
	.cfi_endproc
.LFE82:
	.size	platform_set_gptim2, .-platform_set_gptim2
	.section	.text.platform_get_gptim,"ax",%progbits
	.align	1
	.global	platform_get_gptim
	.thumb
	.thumb_func
	.type	platform_get_gptim, %function
platform_get_gptim:
.LFB83:
	.loc 2 1270 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1271 0
	ldr	r3, .L136	@ tmp136,
	ldr	r0, [r3, #36]	@ D.7644, 1073809408B->CNT
	.loc 2 1272 0
	uxth	r0, r0	@, D.7644
	bx	lr	@
.L137:
	.align	2
.L136:
	.word	1073809408
	.cfi_endproc
.LFE83:
	.size	platform_get_gptim, .-platform_get_gptim
	.section	.text.platform_set_watchdog,"ax",%progbits
	.align	1
	.global	platform_set_watchdog
	.thumb
	.thumb_func
	.type	platform_set_watchdog, %function
platform_set_watchdog:
.LFB84:
	.loc 2 1277 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL30:
	.loc 2 1301 0
	bx	lr	@
	.cfi_endproc
.LFE84:
	.size	platform_set_watchdog, .-platform_set_watchdog
	.section	.text.platform_kill_watchdog,"ax",%progbits
	.align	1
	.global	platform_kill_watchdog
	.thumb
	.thumb_func
	.type	platform_kill_watchdog, %function
platform_kill_watchdog:
.LFB85:
	.loc 2 1303 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1305 0
	bx	lr	@
	.cfi_endproc
.LFE85:
	.size	platform_kill_watchdog, .-platform_kill_watchdog
	.section	.text.platform_pause_watchdog,"ax",%progbits
	.align	1
	.global	platform_pause_watchdog
	.thumb
	.thumb_func
	.type	platform_pause_watchdog, %function
platform_pause_watchdog:
.LFB86:
	.loc 2 1307 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1309 0
	bx	lr	@
	.cfi_endproc
.LFE86:
	.size	platform_pause_watchdog, .-platform_pause_watchdog
	.section	.text.platform_resume_watchdog,"ax",%progbits
	.align	1
	.global	platform_resume_watchdog
	.thumb
	.thumb_func
	.type	platform_resume_watchdog, %function
platform_resume_watchdog:
.LFB87:
	.loc 2 1311 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1317 0
	bx	lr	@
	.cfi_endproc
.LFE87:
	.size	platform_resume_watchdog, .-platform_resume_watchdog
	.section	.text.platform_enable_rtc,"ax",%progbits
	.align	1
	.global	platform_enable_rtc
	.thumb
	.thumb_func
	.type	platform_enable_rtc, %function
platform_enable_rtc:
.LFB88:
	.loc 2 1323 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1331 0
	bx	lr	@
	.cfi_endproc
.LFE88:
	.size	platform_enable_rtc, .-platform_enable_rtc
	.section	.text.platform_disable_rtc,"ax",%progbits
	.align	1
	.global	platform_disable_rtc
	.thumb
	.thumb_func
	.type	platform_disable_rtc, %function
platform_disable_rtc:
.LFB89:
	.loc 2 1336 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1343 0
	bx	lr	@
	.cfi_endproc
.LFE89:
	.size	platform_disable_rtc, .-platform_disable_rtc
	.section	.text.platform_set_time,"ax",%progbits
	.align	1
	.global	platform_set_time
	.thumb
	.thumb_func
	.type	platform_set_time, %function
platform_set_time:
.LFB90:
	.loc 2 1348 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL31:
	.loc 2 1351 0
	ldr	r3, .L145	@ tmp134,
	str	r0, [r3, #160]	@ utc_time, platform_ext.utc
	.loc 2 1356 0
	bx	lr	@
.L146:
	.align	2
.L145:
	.word	platform_ext
	.cfi_endproc
.LFE90:
	.size	platform_set_time, .-platform_set_time
	.section	.text.platform_get_time,"ax",%progbits
	.align	1
	.global	platform_get_time
	.thumb
	.thumb_func
	.type	platform_get_time, %function
platform_get_time:
.LFB91:
	.loc 2 1361 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1366 0
	bx	lr	@
	.cfi_endproc
.LFE91:
	.size	platform_get_time, .-platform_get_time
	.section	.text.platform_set_rtc_alarm,"ax",%progbits
	.align	1
	.global	platform_set_rtc_alarm
	.thumb
	.thumb_func
	.type	platform_set_rtc_alarm, %function
platform_set_rtc_alarm:
.LFB92:
	.loc 2 1371 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL32:
	.loc 2 1405 0
	bx	lr	@
	.cfi_endproc
.LFE92:
	.size	platform_set_rtc_alarm, .-platform_set_rtc_alarm
	.section	.text.platform_trig1_high,"ax",%progbits
	.align	1
	.global	platform_trig1_high
	.thumb
	.thumb_func
	.type	platform_trig1_high, %function
platform_trig1_high:
.LFB93:
	.loc 2 1421 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1421 0
	ldr	r3, .L150	@ tmp135,
	ldr	r2, [r3, #0]	@ D.7640, 1073875968B->MODER
	orr	r2, r2, #2	@ D.7641, D.7640,
	str	r2, [r3, #0]	@ D.7641, 1073875968B->MODER
	bx	lr	@
.L151:
	.align	2
.L150:
	.word	1073875968
	.cfi_endproc
.LFE93:
	.size	platform_trig1_high, .-platform_trig1_high
	.section	.text.platform_trig1_low,"ax",%progbits
	.align	1
	.global	platform_trig1_low
	.thumb
	.thumb_func
	.type	platform_trig1_low, %function
platform_trig1_low:
.LFB94:
	.loc 2 1422 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1422 0
	ldr	r3, .L153	@ tmp135,
	ldr	r2, [r3, #0]	@ D.7637, 1073875968B->MODER
	bic	r2, r2, #2	@ D.7638, D.7637,
	str	r2, [r3, #0]	@ D.7638, 1073875968B->MODER
	bx	lr	@
.L154:
	.align	2
.L153:
	.word	1073875968
	.cfi_endproc
.LFE94:
	.size	platform_trig1_low, .-platform_trig1_low
	.section	.text.platform_trig1_toggle,"ax",%progbits
	.align	1
	.global	platform_trig1_toggle
	.thumb
	.thumb_func
	.type	platform_trig1_toggle, %function
platform_trig1_toggle:
.LFB95:
	.loc 2 1423 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1423 0
	b	OT_TRIG1_TOG	@
	.cfi_endproc
.LFE95:
	.size	platform_trig1_toggle, .-platform_trig1_toggle
	.section	.text.platform_trig2_high,"ax",%progbits
	.align	1
	.global	platform_trig2_high
	.thumb
	.thumb_func
	.type	platform_trig2_high, %function
platform_trig2_high:
.LFB96:
	.loc 2 1431 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1431 0
	ldr	r3, .L157	@ tmp135,
	ldr	r2, [r3, #0]	@ D.7634, 1073875968B->MODER
	orr	r2, r2, #4	@ D.7635, D.7634,
	str	r2, [r3, #0]	@ D.7635, 1073875968B->MODER
	bx	lr	@
.L158:
	.align	2
.L157:
	.word	1073875968
	.cfi_endproc
.LFE96:
	.size	platform_trig2_high, .-platform_trig2_high
	.section	.text.platform_trig2_low,"ax",%progbits
	.align	1
	.global	platform_trig2_low
	.thumb
	.thumb_func
	.type	platform_trig2_low, %function
platform_trig2_low:
.LFB97:
	.loc 2 1432 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1432 0
	ldr	r3, .L160	@ tmp135,
	ldr	r2, [r3, #0]	@ D.7631, 1073875968B->MODER
	bic	r2, r2, #4	@ D.7632, D.7631,
	str	r2, [r3, #0]	@ D.7632, 1073875968B->MODER
	bx	lr	@
.L161:
	.align	2
.L160:
	.word	1073875968
	.cfi_endproc
.LFE97:
	.size	platform_trig2_low, .-platform_trig2_low
	.section	.text.platform_trig2_toggle,"ax",%progbits
	.align	1
	.global	platform_trig2_toggle
	.thumb
	.thumb_func
	.type	platform_trig2_toggle, %function
platform_trig2_toggle:
.LFB98:
	.loc 2 1433 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 2 1433 0
	ldr	r3, .L163	@ tmp135,
	ldr	r2, [r3, #0]	@ D.7628, 1073875968B->MODER
	eor	r2, r2, #4	@ D.7629, D.7628,
	str	r2, [r3, #0]	@ D.7629, 1073875968B->MODER
	bx	lr	@
.L164:
	.align	2
.L163:
	.word	1073875968
	.cfi_endproc
.LFE98:
	.size	platform_trig2_toggle, .-platform_trig2_toggle
	.section	.text.platform_rand,"ax",%progbits
	.align	1
	.global	platform_rand
	.thumb
	.thumb_func
	.type	platform_rand, %function
platform_rand:
.LFB99:
	.loc 2 1456 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL33:
	.loc 2 1537 0
	bx	lr	@
	.cfi_endproc
.LFE99:
	.size	platform_rand, .-platform_rand
	.section	.text.platform_init_prand,"ax",%progbits
	.align	1
	.global	platform_init_prand
	.thumb
	.thumb_func
	.type	platform_init_prand, %function
platform_init_prand:
.LFB100:
	.loc 2 1542 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL34:
	.loc 2 1543 0
	ldr	r3, .L167	@ tmp134,
	strh	r0, [r3, #174]	@ movhi	@ seed, platform_ext.prand_reg
	.loc 2 1544 0
	bx	lr	@
.L168:
	.align	2
.L167:
	.word	platform_ext
	.cfi_endproc
.LFE100:
	.size	platform_init_prand, .-platform_init_prand
	.section	.text.platform_prand_u16,"ax",%progbits
	.align	1
	.global	platform_prand_u16
	.thumb
	.thumb_func
	.type	platform_prand_u16, %function
platform_prand_u16:
.LFB102:
	.loc 2 1554 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, lr}	@
.LCFI8:
	.cfi_def_cfa_offset 16
	.loc 2 1556 0
	ldr	r3, .L170	@ tmp139,
	add	r0, sp, #8	@ tmp141,,
	.cfi_offset 14, -4
	.cfi_offset 2, -8
	.cfi_offset 1, -12
	.cfi_offset 0, -16
	ldr	r3, [r3, #36]	@ D.7618, 1073809408B->CNT
	.loc 2 1558 0
	movs	r1, #1	@,
	.loc 2 1556 0
	strh	r3, [r0, #-2]!	@ movhi	@ D.7618, timer_value
.LVL35:
	.loc 2 1558 0
	bl	platform_crc_block	@
	.loc 2 1559 0
	uxth	r0, r0	@,
	pop	{r1, r2, r3, pc}
.L171:
	.align	2
.L170:
	.word	1073809408
	.cfi_endproc
.LFE102:
	.size	platform_prand_u16, .-platform_prand_u16
	.section	.text.platform_prand_u8,"ax",%progbits
	.align	1
	.global	platform_prand_u8
	.thumb
	.thumb_func
	.type	platform_prand_u8, %function
platform_prand_u8:
.LFB101:
	.loc 2 1548 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r3, lr}	@
.LCFI9:
	.cfi_def_cfa_offset 8
	.loc 2 1549 0
	.cfi_offset 14, -4
	.cfi_offset 3, -8
	bl	platform_prand_u16	@
	.loc 2 1550 0
	uxtb	r0, r0	@,
	pop	{r3, pc}	@
	.cfi_endproc
.LFE101:
	.size	platform_prand_u8, .-platform_prand_u8
	.section	.text.sub_memcpy_dma,"ax",%progbits
	.align	1
	.global	sub_memcpy_dma
	.thumb
	.thumb_func
	.type	sub_memcpy_dma, %function
sub_memcpy_dma:
.LFB103:
	.loc 2 1594 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
.LVL36:
	.loc 2 1601 0
	ldr	r3, .L175	@ tmp157,
	.loc 2 1594 0
	push	{r4, lr}	@
.LCFI10:
	.cfi_def_cfa_offset 8
	.loc 2 1601 0
	mov	r4, #65536	@ tmp158,
	.cfi_offset 14, -4
	.cfi_offset 4, -8
	str	r4, [r3, #4]	@ tmp158, 1073897472B->IFCR
	.loc 2 1602 0
	str	r0, [r3, #96]	@ dest, 1073897560B->CPAR
	.loc 2 1603 0
	str	r1, [r3, #100]	@ src, 1073897560B->CMAR
	.loc 2 1599 0
	orrs	r1, r1, r2	@, tmp163, src, length
.LVL37:
	orr	r0, r1, r0	@ tmp166, tmp163, dest
.LVL38:
	.loc 2 1604 0
	ldr	r4, .L175+4	@ tmp169,
	and	r0, r0, #3	@ D.7608, tmp166,
	ldrh	r1, [r4, r0, lsl #1]	@ tmp170, len_div
	.loc 2 1605 0
	add	r0, r4, r0, lsl #1	@, tmp173, tmp169, D.7608,
	.loc 2 1604 0
	asrs	r2, r2, r1	@ D.7612, length, tmp170
.LVL39:
	str	r2, [r3, #92]	@ D.7612, 1073897560B->CNDTR
	.loc 2 1605 0
	ldrh	r2, [r0, #8]	@ D.7614, ccr
	str	r2, [r3, #88]	@ D.7614, 1073897560B->CCR
.LVL40:
.L174:
	.loc 2 1607 0 discriminator 1
	ldr	r3, .L175	@ tmp176,
.LVL41:
	ldr	r3, [r3, #0]	@ D.7615, 1073897472B->ISR
.LVL42:
	tst	r3, #65536	@ D.7615,
	beq	.L174	@,
	.loc 2 1608 0
	pop	{r4, pc}	@
.L176:
	.align	2
.L175:
	.word	1073897472
	.word	.LANCHOR0
	.cfi_endproc
.LFE103:
	.size	sub_memcpy_dma, .-sub_memcpy_dma
	.section	.text.sub_memcpy2_dma,"ax",%progbits
	.align	1
	.global	sub_memcpy2_dma
	.thumb
	.thumb_func
	.type	sub_memcpy2_dma, %function
sub_memcpy2_dma:
.LFB104:
	.loc 2 1610 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
.LVL43:
	.loc 2 1615 0
	orr	r3, r1, r0	@ tmp147, src, dest
	orrs	r3, r3, r2	@, tmp148, tmp147, length
	tst	r3, #3	@ tmp148,
	.loc 2 1610 0
	push	{r4, r5, lr}	@
.LCFI11:
	.cfi_def_cfa_offset 12
	.loc 2 1615 0
	bne	.L180	@,
	.cfi_offset 14, -4
	.cfi_offset 5, -8
	.cfi_offset 4, -12
	.loc 2 1616 0
	asrs	r2, r2, #1	@ length, length,
.LVL44:
	.loc 2 1617 0
	movw	r4, #19153	@ ccr_val,
	b	.L178	@
.LVL45:
.L180:
	.loc 2 1612 0
	movw	r4, #17873	@ ccr_val,
.LVL46:
.L178:
	.loc 2 1619 0
	ldr	r3, .L181	@ tmp151,
	mov	r5, #65536	@ tmp152,
	str	r5, [r3, #4]	@ tmp152, 1073897472B->IFCR
	.loc 2 1620 0
	str	r0, [r3, #96]	@ dest, 1073897560B->CPAR
	.loc 2 1621 0
	str	r1, [r3, #100]	@ src, 1073897560B->CMAR
	.loc 2 1622 0
	str	r2, [r3, #92]	@ length, 1073897560B->CNDTR
	.loc 2 1623 0
	str	r4, [r3, #88]	@ ccr_val, 1073897560B->CCR
.L179:
	.loc 2 1625 0 discriminator 1
	ldr	r3, .L181	@ tmp157,
	ldr	r3, [r3, #0]	@ D.7593, 1073897472B->ISR
	tst	r3, #65536	@ D.7593,
	beq	.L179	@,
	.loc 2 1626 0
	pop	{r4, r5, pc}	@
.L182:
	.align	2
.L181:
	.word	1073897472
	.cfi_endproc
.LFE104:
	.size	sub_memcpy2_dma, .-sub_memcpy2_dma
	.section	.text.platform_memcpy,"ax",%progbits
	.align	1
	.global	platform_memcpy
	.thumb
	.thumb_func
	.type	platform_memcpy, %function
platform_memcpy:
.LFB105:
	.loc 2 1629 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL47:
	.loc 2 1646 0
	.loc 2 1635 0
	b	sub_memcpy_dma	@
	.cfi_endproc
.LFE105:
	.size	platform_memcpy, .-platform_memcpy
	.section	.text.platform_memcpy_2,"ax",%progbits
	.align	1
	.global	platform_memcpy_2
	.thumb
	.thumb_func
	.type	platform_memcpy_2, %function
platform_memcpy_2:
.LFB106:
	.loc 2 1649 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL48:
	.loc 2 1663 0
	.loc 2 1651 0
	b	sub_memcpy2_dma	@
	.cfi_endproc
.LFE106:
	.size	platform_memcpy_2, .-platform_memcpy_2
	.section	.text.platform_memset,"ax",%progbits
	.align	1
	.global	platform_memset
	.thumb
	.thumb_func
	.type	platform_memset, %function
platform_memset:
.LFB107:
	.loc 2 1667 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
.LVL49:
	push	{r0, r1, r4, lr}	@
.LCFI12:
	.cfi_def_cfa_offset 16
.LVL50:
	.loc 2 1667 0
	add	r3, sp, #8	@ value.1,,
.LVL51:
	strb	r1, [r3, #-1]!	@ value, value
	.loc 2 1672 0
	ldr	r1, .L187	@ tmp142,
	.cfi_offset 14, -4
	.cfi_offset 4, -8
	.cfi_offset 1, -12
	.cfi_offset 0, -16
.LVL52:
	mov	r4, #65536	@ tmp143,
	str	r4, [r1, #4]	@ tmp143, 1073897472B->IFCR
	.loc 2 1673 0
	str	r0, [r1, #96]	@ dest, 1073897560B->CPAR
	.loc 2 1674 0
	str	r3, [r1, #100]	@ value.1, 1073897560B->CMAR
	.loc 2 1676 0
	movw	r3, #16465	@ tmp148,
	.loc 2 1675 0
	str	r2, [r1, #92]	@ length, 1073897560B->CNDTR
	.loc 2 1676 0
	str	r3, [r1, #88]	@ tmp148, 1073897560B->CCR
.L186:
	.loc 2 1681 0 discriminator 1
	ldr	r3, .L187	@ tmp149,
	ldr	r3, [r3, #0]	@ D.7577, 1073897472B->ISR
	tst	r3, #65536	@ D.7577,
	beq	.L186	@,
	.loc 2 1686 0
	pop	{r2, r3, r4, pc}
.L188:
	.align	2
.L187:
	.word	1073897472
	.cfi_endproc
.LFE107:
	.size	platform_memset, .-platform_memset
	.section	.text.sub_timed_wfe,"ax",%progbits
	.align	1
	.global	sub_timed_wfe
	.thumb
	.thumb_func
	.type	sub_timed_wfe, %function
sub_timed_wfe:
.LFB108:
	.loc 2 1691 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL53:
	.loc 2 1699 0
	b	.L190	@
.L191:
.LBB22:
.LBB23:
	.loc 3 1212 0
@ 1212 "B:\otplatform\stm32_mculib\CMSIS\CM3\CoreSupport/core_cm3.h" 1
	wfe
@ 0 "" 2
	.thumb
.L190:
.LBE23:
.LBE22:
	.loc 2 1699 0 discriminator 1
	ldr	r3, .L192	@ tmp138,
	ldrh	r3, [r3, #16]	@, 1073810432B->SR
	tst	r3, #1	@ 1073810432B->SR,
	beq	.L191	@,
	.loc 2 1703 0
	bx	lr	@
.L193:
	.align	2
.L192:
	.word	1073810432
	.cfi_endproc
.LFE108:
	.size	sub_timed_wfe, .-sub_timed_wfe
	.section	.text.platform_block,"ax",%progbits
	.align	1
	.global	platform_block
	.thumb
	.thumb_func
	.type	platform_block, %function
platform_block:
.LFB109:
	.loc 2 1707 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL54:
	.loc 2 1708 0
	movs	r1, #0	@,
	.loc 2 1709 0
	.loc 2 1708 0
	b	sub_timed_wfe	@
	.cfi_endproc
.LFE109:
	.size	platform_block, .-platform_block
	.section	.text.platform_delay,"ax",%progbits
	.align	1
	.global	platform_delay
	.thumb
	.thumb_func
	.type	platform_delay, %function
platform_delay:
.LFB110:
	.loc 2 1714 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL55:
	.loc 2 1715 0
	movs	r1, #31	@,
	.loc 2 1716 0
	.loc 2 1715 0
	b	sub_timed_wfe	@
	.cfi_endproc
.LFE110:
	.size	platform_delay, .-platform_delay
	.section	.text.platform_swdelay_ms,"ax",%progbits
	.align	1
	.global	platform_swdelay_ms
	.thumb
	.thumb_func
	.type	platform_swdelay_ms, %function
platform_swdelay_ms:
.LFB111:
	.loc 2 1722 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL56:
	.loc 2 1728 0
	bx	lr	@
	.cfi_endproc
.LFE111:
	.size	platform_swdelay_ms, .-platform_swdelay_ms
	.section	.text.platform_swdelay_us,"ax",%progbits
	.align	1
	.global	platform_swdelay_us
	.thumb
	.thumb_func
	.type	platform_swdelay_us, %function
platform_swdelay_us:
.LFB112:
	.loc 2 1733 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL57:
	.loc 2 1739 0
	bx	lr	@
	.cfi_endproc
.LFE112:
	.size	platform_swdelay_us, .-platform_swdelay_us
	.comm	platform_ext,180,4
	.comm	platform,2,2
	.section	.rodata
	.align	1
	.set	.LANCHOR0,. + 0
	.type	len_div.6290, %object
	.size	len_div.6290, 8
len_div.6290:
	.short	2
	.short	0
	.short	1
	.short	0
	.type	ccr.6289, %object
	.size	ccr.6289, 8
ccr.6289:
	.short	19153
	.short	16593
	.short	17873
	.short	16593
	.text
.Letext0:
	.section	.debug_loc,"",%progbits
.Ldebug_loc0:
.LLST0:
	.4byte	.LFB33
	.4byte	.LCFI0
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI0
	.4byte	.LFE33
	.2byte	0x2
	.byte	0x7d
	.sleb128 8
	.4byte	0x0
	.4byte	0x0
.LLST1:
	.4byte	.LVL0
	.4byte	.LVL1
	.2byte	0x1
	.byte	0x50
	.4byte	0x0
	.4byte	0x0
.LLST2:
	.4byte	.LFB43
	.4byte	.LCFI1
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI1
	.4byte	.LFE43
	.2byte	0x2
	.byte	0x7d
	.sleb128 8
	.4byte	0x0
	.4byte	0x0
.LLST3:
	.4byte	.LFB48
	.4byte	.LCFI2
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI2
	.4byte	.LFE48
	.2byte	0x2
	.byte	0x7d
	.sleb128 8
	.4byte	0x0
	.4byte	0x0
.LLST4:
	.4byte	.LFB49
	.4byte	.LCFI3
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI3
	.4byte	.LFE49
	.2byte	0x2
	.byte	0x7d
	.sleb128 16
	.4byte	0x0
	.4byte	0x0
.LLST5:
	.4byte	.LVL2
	.4byte	.LVL3
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL3
	.4byte	.LFE49
	.2byte	0x1
	.byte	0x55
	.4byte	0x0
	.4byte	0x0
.LLST6:
	.4byte	.LVL2
	.4byte	.LVL3
	.2byte	0x6
	.byte	0xc
	.4byte	0x1ff80050
	.byte	0x9f
	.4byte	.LVL3
	.4byte	.LVL4
	.2byte	0x1
	.byte	0x54
	.4byte	.LVL4
	.4byte	.LVL5
	.2byte	0x3
	.byte	0x74
	.sleb128 -2
	.byte	0x9f
	.4byte	.LVL5
	.4byte	.LFE49
	.2byte	0x1
	.byte	0x54
	.4byte	0x0
	.4byte	0x0
.LLST7:
	.4byte	.LVL2
	.4byte	.LVL3
	.2byte	0x2
	.byte	0x36
	.byte	0x9f
	.4byte	.LVL5
	.4byte	.LFE49
	.2byte	0x1
	.byte	0x56
	.4byte	0x0
	.4byte	0x0
.LLST8:
	.4byte	.LFB54
	.4byte	.LCFI4
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI4
	.4byte	.LFE54
	.2byte	0x2
	.byte	0x7d
	.sleb128 8
	.4byte	0x0
	.4byte	0x0
.LLST9:
	.4byte	.LVL6
	.4byte	.LVL7
	.2byte	0x4
	.byte	0xa
	.2byte	0x12c
	.byte	0x9f
	.4byte	.LVL7
	.4byte	.LVL8
	.2byte	0x1
	.byte	0x53
	.4byte	.LVL9
	.4byte	.LVL10
	.2byte	0x1
	.byte	0x53
	.4byte	0x0
	.4byte	0x0
.LLST10:
	.4byte	.LFB59
	.4byte	.LCFI5
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI5
	.4byte	.LFE59
	.2byte	0x2
	.byte	0x7d
	.sleb128 8
	.4byte	0x0
	.4byte	0x0
.LLST11:
	.4byte	.LVL15
	.4byte	.LVL16
	.2byte	0x1
	.byte	0x52
	.4byte	0x0
	.4byte	0x0
.LLST12:
	.4byte	.LFB66
	.4byte	.LCFI6
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI6
	.4byte	.LFE66
	.2byte	0x2
	.byte	0x7d
	.sleb128 8
	.4byte	0x0
	.4byte	0x0
.LLST13:
	.4byte	.LVL18
	.4byte	.LVL19
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL19
	.4byte	.LVL20
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL20
	.4byte	.LVL21
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL21
	.4byte	.LVL22
	.2byte	0x2
	.byte	0x73
	.sleb128 12
	.4byte	.LVL22
	.4byte	.LFE67
	.2byte	0x6
	.byte	0x11
	.sleb128 -536810228
	.4byte	0x0
	.4byte	0x0
.LLST14:
	.4byte	.LVL23
	.4byte	.LVL24
	.2byte	0x1
	.byte	0x50
	.4byte	0x0
	.4byte	0x0
.LLST15:
	.4byte	.LFB47
	.4byte	.LCFI7
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI7
	.4byte	.LFE47
	.2byte	0x2
	.byte	0x7d
	.sleb128 8
	.4byte	0x0
	.4byte	0x0
.LLST16:
	.4byte	.LVL28
	.4byte	.LVL29
	.2byte	0x1
	.byte	0x50
	.4byte	0x0
	.4byte	0x0
.LLST17:
	.4byte	.LFB102
	.4byte	.LCFI8
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI8
	.4byte	.LFE102
	.2byte	0x2
	.byte	0x7d
	.sleb128 16
	.4byte	0x0
	.4byte	0x0
.LLST18:
	.4byte	.LFB101
	.4byte	.LCFI9
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI9
	.4byte	.LFE101
	.2byte	0x2
	.byte	0x7d
	.sleb128 8
	.4byte	0x0
	.4byte	0x0
.LLST19:
	.4byte	.LFB103
	.4byte	.LCFI10
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI10
	.4byte	.LFE103
	.2byte	0x2
	.byte	0x7d
	.sleb128 8
	.4byte	0x0
	.4byte	0x0
.LLST20:
	.4byte	.LVL36
	.4byte	.LVL38
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL38
	.4byte	.LVL40
	.2byte	0x3
	.byte	0x73
	.sleb128 96
	.4byte	.LVL40
	.4byte	.LVL41
	.2byte	0x5
	.byte	0xc
	.4byte	0x40026060
	.4byte	.LVL41
	.4byte	.LVL42
	.2byte	0x3
	.byte	0x73
	.sleb128 96
	.4byte	.LVL42
	.4byte	.LFE103
	.2byte	0x5
	.byte	0xc
	.4byte	0x40026060
	.4byte	0x0
	.4byte	0x0
.LLST21:
	.4byte	.LVL36
	.4byte	.LVL37
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL37
	.4byte	.LVL40
	.2byte	0x3
	.byte	0x73
	.sleb128 100
	.4byte	.LVL40
	.4byte	.LVL41
	.2byte	0x5
	.byte	0xc
	.4byte	0x40026064
	.4byte	.LVL41
	.4byte	.LVL42
	.2byte	0x3
	.byte	0x73
	.sleb128 100
	.4byte	.LVL42
	.4byte	.LFE103
	.2byte	0x5
	.byte	0xc
	.4byte	0x40026064
	.4byte	0x0
	.4byte	0x0
.LLST22:
	.4byte	.LVL36
	.4byte	.LVL39
	.2byte	0x1
	.byte	0x52
	.4byte	0x0
	.4byte	0x0
.LLST23:
	.4byte	.LVL36
	.4byte	.LVL37
	.2byte	0xb
	.byte	0x71
	.sleb128 0
	.byte	0x72
	.sleb128 0
	.byte	0x21
	.byte	0x70
	.sleb128 0
	.byte	0x21
	.byte	0x33
	.byte	0x1a
	.byte	0x9f
	.4byte	0x0
	.4byte	0x0
.LLST24:
	.4byte	.LFB104
	.4byte	.LCFI11
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI11
	.4byte	.LFE104
	.2byte	0x2
	.byte	0x7d
	.sleb128 12
	.4byte	0x0
	.4byte	0x0
.LLST25:
	.4byte	.LVL43
	.4byte	.LVL44
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL44
	.4byte	.LVL46
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL46
	.4byte	.LFE104
	.2byte	0x1
	.byte	0x52
	.4byte	0x0
	.4byte	0x0
.LLST26:
	.4byte	.LVL43
	.4byte	.LVL44
	.2byte	0x4
	.byte	0xa
	.2byte	0x45d1
	.byte	0x9f
	.4byte	.LVL44
	.4byte	.LVL45
	.2byte	0x4
	.byte	0xa
	.2byte	0x4ad1
	.byte	0x9f
	.4byte	.LVL45
	.4byte	.LVL46
	.2byte	0x4
	.byte	0xa
	.2byte	0x45d1
	.byte	0x9f
	.4byte	.LVL46
	.4byte	.LFE104
	.2byte	0x1
	.byte	0x54
	.4byte	0x0
	.4byte	0x0
.LLST27:
	.4byte	.LFB107
	.4byte	.LCFI12
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI12
	.4byte	.LFE107
	.2byte	0x2
	.byte	0x7d
	.sleb128 16
	.4byte	0x0
	.4byte	0x0
.LLST28:
	.4byte	.LVL49
	.4byte	.LVL50
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL51
	.4byte	.LVL52
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL52
	.4byte	.LFE107
	.2byte	0x2
	.byte	0x73
	.sleb128 -1
	.4byte	0x0
	.4byte	0x0
	.file 4 "B:\\otlib/OT_types.h"
	.file 5 "c:\\program files\\raisonance\\ride\\arm-gcc\\bin\\../lib/gcc/arm-none-eabi/4.5.2/../../../../arm-none-eabi/include/stdint.h"
	.file 6 "B:\\otplatform\\stm32_mculib\\CMSIS\\CM3\\DeviceSupport/ST/STM32L1xx/stm32l1xx.h"
	.file 7 "B:\\otplatform\\stm32l1xx/platform_STM32L1xx.h"
	.file 8 "B:\\otlib/OT_platform.h"
	.file 9 "B:\\otlib/system.h"
	.file 10 "B:\\otlib/veelite_core.h"
	.file 11 "B:\\otlib/veelite.h"
	.section	.debug_info
	.4byte	0x1a27
	.2byte	0x2
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.LASF329
	.byte	0x1
	.4byte	.LASF330
	.4byte	0x0
	.4byte	0x0
	.4byte	.Ldebug_ranges0+0x18
	.4byte	.Ldebug_line0
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.4byte	.LASF0
	.uleb128 0x3
	.4byte	.LASF2
	.byte	0x4
	.byte	0x60
	.4byte	0x37
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.4byte	.LASF1
	.uleb128 0x3
	.4byte	.LASF3
	.byte	0x4
	.byte	0x67
	.4byte	0x49
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.4byte	.LASF4
	.uleb128 0x3
	.4byte	.LASF5
	.byte	0x4
	.byte	0x6f
	.4byte	0x5b
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.4byte	.LASF6
	.uleb128 0x3
	.4byte	.LASF7
	.byte	0x4
	.byte	0x70
	.4byte	0x5b
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.4byte	.LASF8
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF9
	.uleb128 0x3
	.4byte	.LASF10
	.byte	0x4
	.byte	0x7d
	.4byte	0x74
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF11
	.uleb128 0x4
	.byte	0x4
	.uleb128 0x5
	.4byte	.LASF331
	.byte	0x1
	.byte	0x6
	.byte	0xa8
	.4byte	0x222
	.uleb128 0x6
	.4byte	.LASF12
	.sleb128 -14
	.uleb128 0x6
	.4byte	.LASF13
	.sleb128 -12
	.uleb128 0x6
	.4byte	.LASF14
	.sleb128 -11
	.uleb128 0x6
	.4byte	.LASF15
	.sleb128 -10
	.uleb128 0x6
	.4byte	.LASF16
	.sleb128 -5
	.uleb128 0x6
	.4byte	.LASF17
	.sleb128 -4
	.uleb128 0x6
	.4byte	.LASF18
	.sleb128 -2
	.uleb128 0x6
	.4byte	.LASF19
	.sleb128 -1
	.uleb128 0x6
	.4byte	.LASF20
	.sleb128 0
	.uleb128 0x6
	.4byte	.LASF21
	.sleb128 1
	.uleb128 0x6
	.4byte	.LASF22
	.sleb128 2
	.uleb128 0x6
	.4byte	.LASF23
	.sleb128 3
	.uleb128 0x6
	.4byte	.LASF24
	.sleb128 4
	.uleb128 0x6
	.4byte	.LASF25
	.sleb128 5
	.uleb128 0x6
	.4byte	.LASF26
	.sleb128 6
	.uleb128 0x6
	.4byte	.LASF27
	.sleb128 7
	.uleb128 0x6
	.4byte	.LASF28
	.sleb128 8
	.uleb128 0x6
	.4byte	.LASF29
	.sleb128 9
	.uleb128 0x6
	.4byte	.LASF30
	.sleb128 10
	.uleb128 0x6
	.4byte	.LASF31
	.sleb128 11
	.uleb128 0x6
	.4byte	.LASF32
	.sleb128 12
	.uleb128 0x6
	.4byte	.LASF33
	.sleb128 13
	.uleb128 0x6
	.4byte	.LASF34
	.sleb128 14
	.uleb128 0x6
	.4byte	.LASF35
	.sleb128 15
	.uleb128 0x6
	.4byte	.LASF36
	.sleb128 16
	.uleb128 0x6
	.4byte	.LASF37
	.sleb128 17
	.uleb128 0x6
	.4byte	.LASF38
	.sleb128 18
	.uleb128 0x6
	.4byte	.LASF39
	.sleb128 19
	.uleb128 0x6
	.4byte	.LASF40
	.sleb128 20
	.uleb128 0x6
	.4byte	.LASF41
	.sleb128 21
	.uleb128 0x6
	.4byte	.LASF42
	.sleb128 22
	.uleb128 0x6
	.4byte	.LASF43
	.sleb128 23
	.uleb128 0x6
	.4byte	.LASF44
	.sleb128 24
	.uleb128 0x6
	.4byte	.LASF45
	.sleb128 25
	.uleb128 0x6
	.4byte	.LASF46
	.sleb128 26
	.uleb128 0x6
	.4byte	.LASF47
	.sleb128 27
	.uleb128 0x6
	.4byte	.LASF48
	.sleb128 28
	.uleb128 0x6
	.4byte	.LASF49
	.sleb128 29
	.uleb128 0x6
	.4byte	.LASF50
	.sleb128 30
	.uleb128 0x6
	.4byte	.LASF51
	.sleb128 31
	.uleb128 0x6
	.4byte	.LASF52
	.sleb128 32
	.uleb128 0x6
	.4byte	.LASF53
	.sleb128 33
	.uleb128 0x6
	.4byte	.LASF54
	.sleb128 34
	.uleb128 0x6
	.4byte	.LASF55
	.sleb128 35
	.uleb128 0x6
	.4byte	.LASF56
	.sleb128 36
	.uleb128 0x6
	.4byte	.LASF57
	.sleb128 37
	.uleb128 0x6
	.4byte	.LASF58
	.sleb128 38
	.uleb128 0x6
	.4byte	.LASF59
	.sleb128 39
	.uleb128 0x6
	.4byte	.LASF60
	.sleb128 40
	.uleb128 0x6
	.4byte	.LASF61
	.sleb128 41
	.uleb128 0x6
	.4byte	.LASF62
	.sleb128 42
	.uleb128 0x6
	.4byte	.LASF63
	.sleb128 43
	.uleb128 0x6
	.4byte	.LASF64
	.sleb128 44
	.uleb128 0x6
	.4byte	.LASF65
	.sleb128 45
	.uleb128 0x6
	.4byte	.LASF66
	.sleb128 46
	.uleb128 0x6
	.4byte	.LASF67
	.sleb128 47
	.uleb128 0x6
	.4byte	.LASF68
	.sleb128 48
	.uleb128 0x6
	.4byte	.LASF69
	.sleb128 49
	.uleb128 0x6
	.4byte	.LASF70
	.sleb128 50
	.uleb128 0x6
	.4byte	.LASF71
	.sleb128 51
	.uleb128 0x6
	.4byte	.LASF72
	.sleb128 52
	.uleb128 0x6
	.4byte	.LASF73
	.sleb128 53
	.uleb128 0x6
	.4byte	.LASF74
	.sleb128 54
	.uleb128 0x6
	.4byte	.LASF75
	.sleb128 55
	.uleb128 0x6
	.4byte	.LASF76
	.sleb128 56
	.byte	0x0
	.uleb128 0x3
	.4byte	.LASF77
	.byte	0x5
	.byte	0x2a
	.4byte	0x37
	.uleb128 0x3
	.4byte	.LASF78
	.byte	0x5
	.byte	0x36
	.4byte	0x5b
	.uleb128 0x3
	.4byte	.LASF79
	.byte	0x5
	.byte	0x50
	.4byte	0x74
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.4byte	.LASF80
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.4byte	.LASF81
	.uleb128 0x7
	.byte	0x4
	.byte	0x5
	.ascii	"int\000"
	.uleb128 0x8
	.2byte	0xe04
	.byte	0x3
	.byte	0x84
	.4byte	0x322
	.uleb128 0x9
	.4byte	.LASF82
	.byte	0x3
	.byte	0x86
	.4byte	0x332
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.4byte	.LASF83
	.byte	0x3
	.byte	0x87
	.4byte	0x337
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x9
	.4byte	.LASF84
	.byte	0x3
	.byte	0x88
	.4byte	0x347
	.byte	0x3
	.byte	0x23
	.uleb128 0x80
	.uleb128 0x9
	.4byte	.LASF85
	.byte	0x3
	.byte	0x89
	.4byte	0x337
	.byte	0x3
	.byte	0x23
	.uleb128 0xa0
	.uleb128 0x9
	.4byte	.LASF86
	.byte	0x3
	.byte	0x8a
	.4byte	0x34c
	.byte	0x3
	.byte	0x23
	.uleb128 0x100
	.uleb128 0x9
	.4byte	.LASF87
	.byte	0x3
	.byte	0x8b
	.4byte	0x337
	.byte	0x3
	.byte	0x23
	.uleb128 0x120
	.uleb128 0x9
	.4byte	.LASF88
	.byte	0x3
	.byte	0x8c
	.4byte	0x351
	.byte	0x3
	.byte	0x23
	.uleb128 0x180
	.uleb128 0x9
	.4byte	.LASF89
	.byte	0x3
	.byte	0x8d
	.4byte	0x337
	.byte	0x3
	.byte	0x23
	.uleb128 0x1a0
	.uleb128 0x9
	.4byte	.LASF90
	.byte	0x3
	.byte	0x8e
	.4byte	0x356
	.byte	0x3
	.byte	0x23
	.uleb128 0x200
	.uleb128 0x9
	.4byte	.LASF91
	.byte	0x3
	.byte	0x8f
	.4byte	0x35b
	.byte	0x3
	.byte	0x23
	.uleb128 0x220
	.uleb128 0xa
	.ascii	"IP\000"
	.byte	0x3
	.byte	0x90
	.4byte	0x37b
	.byte	0x3
	.byte	0x23
	.uleb128 0x300
	.uleb128 0x9
	.4byte	.LASF92
	.byte	0x3
	.byte	0x91
	.4byte	0x380
	.byte	0x3
	.byte	0x23
	.uleb128 0x3f0
	.uleb128 0x9
	.4byte	.LASF93
	.byte	0x3
	.byte	0x92
	.4byte	0x391
	.byte	0x3
	.byte	0x23
	.uleb128 0xe00
	.byte	0x0
	.uleb128 0xb
	.4byte	0x238
	.4byte	0x332
	.uleb128 0xc
	.4byte	0x86
	.byte	0x7
	.byte	0x0
	.uleb128 0xd
	.4byte	0x322
	.uleb128 0xb
	.4byte	0x238
	.4byte	0x347
	.uleb128 0xc
	.4byte	0x86
	.byte	0x17
	.byte	0x0
	.uleb128 0xd
	.4byte	0x322
	.uleb128 0xd
	.4byte	0x322
	.uleb128 0xd
	.4byte	0x322
	.uleb128 0xd
	.4byte	0x322
	.uleb128 0xb
	.4byte	0x238
	.4byte	0x36b
	.uleb128 0xc
	.4byte	0x86
	.byte	0x37
	.byte	0x0
	.uleb128 0xb
	.4byte	0x222
	.4byte	0x37b
	.uleb128 0xc
	.4byte	0x86
	.byte	0xef
	.byte	0x0
	.uleb128 0xd
	.4byte	0x36b
	.uleb128 0xb
	.4byte	0x238
	.4byte	0x391
	.uleb128 0xe
	.4byte	0x86
	.2byte	0x283
	.byte	0x0
	.uleb128 0xd
	.4byte	0x238
	.uleb128 0x3
	.4byte	.LASF94
	.byte	0x3
	.byte	0x93
	.4byte	0x258
	.uleb128 0xf
	.byte	0x74
	.byte	0x3
	.byte	0x9b
	.4byte	0x4b4
	.uleb128 0x9
	.4byte	.LASF95
	.byte	0x3
	.byte	0x9d
	.4byte	0x4b4
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.4byte	.LASF96
	.byte	0x3
	.byte	0x9e
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x9
	.4byte	.LASF97
	.byte	0x3
	.byte	0x9f
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x9
	.4byte	.LASF98
	.byte	0x3
	.byte	0xa0
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0xa
	.ascii	"SCR\000"
	.byte	0x3
	.byte	0xa1
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0xa
	.ascii	"CCR\000"
	.byte	0x3
	.byte	0xa2
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0xa
	.ascii	"SHP\000"
	.byte	0x3
	.byte	0xa3
	.4byte	0x4c9
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x9
	.4byte	.LASF99
	.byte	0x3
	.byte	0xa4
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x24
	.uleb128 0x9
	.4byte	.LASF100
	.byte	0x3
	.byte	0xa5
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.uleb128 0x9
	.4byte	.LASF101
	.byte	0x3
	.byte	0xa6
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x2c
	.uleb128 0x9
	.4byte	.LASF102
	.byte	0x3
	.byte	0xa7
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x30
	.uleb128 0x9
	.4byte	.LASF103
	.byte	0x3
	.byte	0xa8
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x34
	.uleb128 0x9
	.4byte	.LASF104
	.byte	0x3
	.byte	0xa9
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x38
	.uleb128 0x9
	.4byte	.LASF105
	.byte	0x3
	.byte	0xaa
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x3c
	.uleb128 0xa
	.ascii	"PFR\000"
	.byte	0x3
	.byte	0xab
	.4byte	0x4de
	.byte	0x2
	.byte	0x23
	.uleb128 0x40
	.uleb128 0xa
	.ascii	"DFR\000"
	.byte	0x3
	.byte	0xac
	.4byte	0x4b4
	.byte	0x2
	.byte	0x23
	.uleb128 0x48
	.uleb128 0xa
	.ascii	"ADR\000"
	.byte	0x3
	.byte	0xad
	.4byte	0x4b4
	.byte	0x2
	.byte	0x23
	.uleb128 0x4c
	.uleb128 0x9
	.4byte	.LASF106
	.byte	0x3
	.byte	0xae
	.4byte	0x4f8
	.byte	0x2
	.byte	0x23
	.uleb128 0x50
	.uleb128 0x9
	.4byte	.LASF107
	.byte	0x3
	.byte	0xaf
	.4byte	0x512
	.byte	0x2
	.byte	0x23
	.uleb128 0x60
	.byte	0x0
	.uleb128 0x10
	.4byte	0x391
	.uleb128 0xb
	.4byte	0x222
	.4byte	0x4c9
	.uleb128 0xc
	.4byte	0x86
	.byte	0xb
	.byte	0x0
	.uleb128 0xd
	.4byte	0x4b9
	.uleb128 0xb
	.4byte	0x238
	.4byte	0x4de
	.uleb128 0xc
	.4byte	0x86
	.byte	0x1
	.byte	0x0
	.uleb128 0x10
	.4byte	0x4e3
	.uleb128 0xd
	.4byte	0x4ce
	.uleb128 0xb
	.4byte	0x238
	.4byte	0x4f8
	.uleb128 0xc
	.4byte	0x86
	.byte	0x3
	.byte	0x0
	.uleb128 0x10
	.4byte	0x4fd
	.uleb128 0xd
	.4byte	0x4e8
	.uleb128 0xb
	.4byte	0x238
	.4byte	0x512
	.uleb128 0xc
	.4byte	0x86
	.byte	0x4
	.byte	0x0
	.uleb128 0x10
	.4byte	0x517
	.uleb128 0xd
	.4byte	0x502
	.uleb128 0x3
	.4byte	.LASF108
	.byte	0x3
	.byte	0xb0
	.4byte	0x3a1
	.uleb128 0x11
	.byte	0x10
	.byte	0x3
	.2byte	0x16d
	.4byte	0x56d
	.uleb128 0x12
	.4byte	.LASF109
	.byte	0x3
	.2byte	0x16f
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x12
	.4byte	.LASF110
	.byte	0x3
	.2byte	0x170
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x13
	.ascii	"VAL\000"
	.byte	0x3
	.2byte	0x171
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x12
	.4byte	.LASF111
	.byte	0x3
	.2byte	0x172
	.4byte	0x4b4
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.byte	0x0
	.uleb128 0x14
	.4byte	.LASF112
	.byte	0x3
	.2byte	0x173
	.4byte	0x527
	.uleb128 0xd
	.4byte	0x22d
	.uleb128 0x15
	.byte	0x1
	.byte	0x6
	.2byte	0x10f
	.4byte	0x594
	.uleb128 0x6
	.4byte	.LASF113
	.sleb128 0
	.uleb128 0x16
	.ascii	"SET\000"
	.sleb128 1
	.byte	0x0
	.uleb128 0x11
	.byte	0x10
	.byte	0x6
	.2byte	0x1b1
	.4byte	0x5d9
	.uleb128 0x12
	.4byte	.LASF114
	.byte	0x6
	.2byte	0x1b3
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x13
	.ascii	"CR\000"
	.byte	0x6
	.2byte	0x1b4
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x12
	.4byte	.LASF115
	.byte	0x6
	.2byte	0x1b5
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x12
	.4byte	.LASF116
	.byte	0x6
	.2byte	0x1b6
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.byte	0x0
	.uleb128 0x14
	.4byte	.LASF117
	.byte	0x6
	.2byte	0x1b7
	.4byte	0x594
	.uleb128 0x11
	.byte	0x10
	.byte	0x6
	.2byte	0x1bd
	.4byte	0x62b
	.uleb128 0x13
	.ascii	"CCR\000"
	.byte	0x6
	.2byte	0x1bf
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x12
	.4byte	.LASF118
	.byte	0x6
	.2byte	0x1c0
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x12
	.4byte	.LASF119
	.byte	0x6
	.2byte	0x1c1
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x12
	.4byte	.LASF120
	.byte	0x6
	.2byte	0x1c2
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.byte	0x0
	.uleb128 0x14
	.4byte	.LASF121
	.byte	0x6
	.2byte	0x1c3
	.4byte	0x5e5
	.uleb128 0x11
	.byte	0x8
	.byte	0x6
	.2byte	0x1c5
	.4byte	0x65f
	.uleb128 0x13
	.ascii	"ISR\000"
	.byte	0x6
	.2byte	0x1c7
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x12
	.4byte	.LASF122
	.byte	0x6
	.2byte	0x1c8
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0x14
	.4byte	.LASF123
	.byte	0x6
	.2byte	0x1c9
	.4byte	0x637
	.uleb128 0x11
	.byte	0x18
	.byte	0x6
	.2byte	0x1cf
	.4byte	0x6ce
	.uleb128 0x13
	.ascii	"IMR\000"
	.byte	0x6
	.2byte	0x1d1
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x13
	.ascii	"EMR\000"
	.byte	0x6
	.2byte	0x1d2
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x12
	.4byte	.LASF124
	.byte	0x6
	.2byte	0x1d3
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x12
	.4byte	.LASF125
	.byte	0x6
	.2byte	0x1d4
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x12
	.4byte	.LASF126
	.byte	0x6
	.2byte	0x1d5
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x13
	.ascii	"PR\000"
	.byte	0x6
	.2byte	0x1d6
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.byte	0x0
	.uleb128 0x14
	.4byte	.LASF127
	.byte	0x6
	.2byte	0x1d7
	.4byte	0x66b
	.uleb128 0x11
	.byte	0x88
	.byte	0x6
	.2byte	0x1dd
	.4byte	0x799
	.uleb128 0x13
	.ascii	"ACR\000"
	.byte	0x6
	.2byte	0x1df
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x12
	.4byte	.LASF128
	.byte	0x6
	.2byte	0x1e0
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x12
	.4byte	.LASF129
	.byte	0x6
	.2byte	0x1e1
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x12
	.4byte	.LASF130
	.byte	0x6
	.2byte	0x1e2
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x12
	.4byte	.LASF131
	.byte	0x6
	.2byte	0x1e3
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x12
	.4byte	.LASF132
	.byte	0x6
	.2byte	0x1e4
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x13
	.ascii	"SR\000"
	.byte	0x6
	.2byte	0x1e5
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x13
	.ascii	"OBR\000"
	.byte	0x6
	.2byte	0x1e6
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x1c
	.uleb128 0x12
	.4byte	.LASF133
	.byte	0x6
	.2byte	0x1e7
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x12
	.4byte	.LASF134
	.byte	0x6
	.2byte	0x1e8
	.4byte	0x799
	.byte	0x2
	.byte	0x23
	.uleb128 0x24
	.uleb128 0x12
	.4byte	.LASF135
	.byte	0x6
	.2byte	0x1e9
	.4byte	0x391
	.byte	0x3
	.byte	0x23
	.uleb128 0x80
	.uleb128 0x12
	.4byte	.LASF136
	.byte	0x6
	.2byte	0x1ea
	.4byte	0x391
	.byte	0x3
	.byte	0x23
	.uleb128 0x84
	.byte	0x0
	.uleb128 0xb
	.4byte	0x238
	.4byte	0x7a9
	.uleb128 0xc
	.4byte	0x86
	.byte	0x16
	.byte	0x0
	.uleb128 0x14
	.4byte	.LASF137
	.byte	0x6
	.2byte	0x1eb
	.4byte	0x6da
	.uleb128 0x11
	.byte	0x2c
	.byte	0x6
	.2byte	0x21e
	.4byte	0x8a0
	.uleb128 0x12
	.4byte	.LASF138
	.byte	0x6
	.2byte	0x220
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x12
	.4byte	.LASF139
	.byte	0x6
	.2byte	0x221
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x12
	.4byte	.LASF83
	.byte	0x6
	.2byte	0x222
	.4byte	0x22d
	.byte	0x2
	.byte	0x23
	.uleb128 0x6
	.uleb128 0x12
	.4byte	.LASF140
	.byte	0x6
	.2byte	0x223
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x12
	.4byte	.LASF141
	.byte	0x6
	.2byte	0x224
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x13
	.ascii	"IDR\000"
	.byte	0x6
	.2byte	0x225
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x12
	.4byte	.LASF142
	.byte	0x6
	.2byte	0x226
	.4byte	0x22d
	.byte	0x2
	.byte	0x23
	.uleb128 0x12
	.uleb128 0x13
	.ascii	"ODR\000"
	.byte	0x6
	.2byte	0x227
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x12
	.4byte	.LASF87
	.byte	0x6
	.2byte	0x228
	.4byte	0x22d
	.byte	0x2
	.byte	0x23
	.uleb128 0x16
	.uleb128 0x12
	.4byte	.LASF143
	.byte	0x6
	.2byte	0x229
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x12
	.4byte	.LASF144
	.byte	0x6
	.2byte	0x22a
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x1a
	.uleb128 0x12
	.4byte	.LASF145
	.byte	0x6
	.2byte	0x22b
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x1c
	.uleb128 0x13
	.ascii	"AFR\000"
	.byte	0x6
	.2byte	0x22c
	.4byte	0x8a0
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x13
	.ascii	"BRR\000"
	.byte	0x6
	.2byte	0x22d
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.uleb128 0x12
	.4byte	.LASF89
	.byte	0x6
	.2byte	0x22e
	.4byte	0x22d
	.byte	0x2
	.byte	0x23
	.uleb128 0x2a
	.byte	0x0
	.uleb128 0xd
	.4byte	0x4ce
	.uleb128 0x14
	.4byte	.LASF146
	.byte	0x6
	.2byte	0x22f
	.4byte	0x7b5
	.uleb128 0x11
	.byte	0x18
	.byte	0x6
	.2byte	0x235
	.4byte	0x8e8
	.uleb128 0x12
	.4byte	.LASF147
	.byte	0x6
	.2byte	0x237
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x13
	.ascii	"PMC\000"
	.byte	0x6
	.2byte	0x238
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x12
	.4byte	.LASF148
	.byte	0x6
	.2byte	0x239
	.4byte	0x8e8
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.byte	0x0
	.uleb128 0xd
	.4byte	0x4e8
	.uleb128 0x14
	.4byte	.LASF149
	.byte	0x6
	.2byte	0x23a
	.4byte	0x8b1
	.uleb128 0x11
	.byte	0x8
	.byte	0x6
	.2byte	0x275
	.4byte	0x920
	.uleb128 0x13
	.ascii	"CR\000"
	.byte	0x6
	.2byte	0x277
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x13
	.ascii	"CSR\000"
	.byte	0x6
	.2byte	0x278
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0x0
	.uleb128 0x14
	.4byte	.LASF150
	.byte	0x6
	.2byte	0x279
	.4byte	0x8f9
	.uleb128 0x11
	.byte	0x38
	.byte	0x6
	.2byte	0x27f
	.4byte	0xa07
	.uleb128 0x13
	.ascii	"CR\000"
	.byte	0x6
	.2byte	0x281
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x12
	.4byte	.LASF151
	.byte	0x6
	.2byte	0x282
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x12
	.4byte	.LASF152
	.byte	0x6
	.2byte	0x283
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x13
	.ascii	"CIR\000"
	.byte	0x6
	.2byte	0x284
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x12
	.4byte	.LASF153
	.byte	0x6
	.2byte	0x285
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x12
	.4byte	.LASF154
	.byte	0x6
	.2byte	0x286
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x12
	.4byte	.LASF155
	.byte	0x6
	.2byte	0x287
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x12
	.4byte	.LASF156
	.byte	0x6
	.2byte	0x288
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x1c
	.uleb128 0x12
	.4byte	.LASF157
	.byte	0x6
	.2byte	0x289
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x12
	.4byte	.LASF158
	.byte	0x6
	.2byte	0x28a
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x24
	.uleb128 0x12
	.4byte	.LASF159
	.byte	0x6
	.2byte	0x28b
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.uleb128 0x12
	.4byte	.LASF160
	.byte	0x6
	.2byte	0x28c
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x2c
	.uleb128 0x12
	.4byte	.LASF161
	.byte	0x6
	.2byte	0x28d
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x30
	.uleb128 0x13
	.ascii	"CSR\000"
	.byte	0x6
	.2byte	0x28e
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x14
	.4byte	.LASF162
	.byte	0x6
	.2byte	0x28f
	.4byte	0x92c
	.uleb128 0x11
	.byte	0x54
	.byte	0x6
	.2byte	0x316
	.4byte	0xc19
	.uleb128 0x13
	.ascii	"CR1\000"
	.byte	0x6
	.2byte	0x318
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x12
	.4byte	.LASF83
	.byte	0x6
	.2byte	0x319
	.4byte	0x22d
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x13
	.ascii	"CR2\000"
	.byte	0x6
	.2byte	0x31a
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x12
	.4byte	.LASF142
	.byte	0x6
	.2byte	0x31b
	.4byte	0x22d
	.byte	0x2
	.byte	0x23
	.uleb128 0x6
	.uleb128 0x12
	.4byte	.LASF163
	.byte	0x6
	.2byte	0x31c
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x12
	.4byte	.LASF87
	.byte	0x6
	.2byte	0x31d
	.4byte	0x22d
	.byte	0x2
	.byte	0x23
	.uleb128 0xa
	.uleb128 0x12
	.4byte	.LASF164
	.byte	0x6
	.2byte	0x31e
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x12
	.4byte	.LASF89
	.byte	0x6
	.2byte	0x31f
	.4byte	0x22d
	.byte	0x2
	.byte	0x23
	.uleb128 0xe
	.uleb128 0x13
	.ascii	"SR\000"
	.byte	0x6
	.2byte	0x320
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x12
	.4byte	.LASF91
	.byte	0x6
	.2byte	0x321
	.4byte	0x22d
	.byte	0x2
	.byte	0x23
	.uleb128 0x12
	.uleb128 0x13
	.ascii	"EGR\000"
	.byte	0x6
	.2byte	0x322
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x12
	.4byte	.LASF92
	.byte	0x6
	.2byte	0x323
	.4byte	0x22d
	.byte	0x2
	.byte	0x23
	.uleb128 0x16
	.uleb128 0x12
	.4byte	.LASF165
	.byte	0x6
	.2byte	0x324
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x12
	.4byte	.LASF166
	.byte	0x6
	.2byte	0x325
	.4byte	0x22d
	.byte	0x2
	.byte	0x23
	.uleb128 0x1a
	.uleb128 0x12
	.4byte	.LASF167
	.byte	0x6
	.2byte	0x326
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x1c
	.uleb128 0x12
	.4byte	.LASF168
	.byte	0x6
	.2byte	0x327
	.4byte	0x22d
	.byte	0x2
	.byte	0x23
	.uleb128 0x1e
	.uleb128 0x12
	.4byte	.LASF169
	.byte	0x6
	.2byte	0x328
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x12
	.4byte	.LASF170
	.byte	0x6
	.2byte	0x329
	.4byte	0x22d
	.byte	0x2
	.byte	0x23
	.uleb128 0x22
	.uleb128 0x13
	.ascii	"CNT\000"
	.byte	0x6
	.2byte	0x32a
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x24
	.uleb128 0x13
	.ascii	"PSC\000"
	.byte	0x6
	.2byte	0x32b
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.uleb128 0x12
	.4byte	.LASF171
	.byte	0x6
	.2byte	0x32c
	.4byte	0x22d
	.byte	0x2
	.byte	0x23
	.uleb128 0x2a
	.uleb128 0x13
	.ascii	"ARR\000"
	.byte	0x6
	.2byte	0x32d
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x2c
	.uleb128 0x12
	.4byte	.LASF172
	.byte	0x6
	.2byte	0x32e
	.4byte	0x238
	.byte	0x2
	.byte	0x23
	.uleb128 0x30
	.uleb128 0x12
	.4byte	.LASF173
	.byte	0x6
	.2byte	0x32f
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x34
	.uleb128 0x12
	.4byte	.LASF174
	.byte	0x6
	.2byte	0x330
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x38
	.uleb128 0x12
	.4byte	.LASF175
	.byte	0x6
	.2byte	0x331
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x3c
	.uleb128 0x12
	.4byte	.LASF176
	.byte	0x6
	.2byte	0x332
	.4byte	0x391
	.byte	0x2
	.byte	0x23
	.uleb128 0x40
	.uleb128 0x12
	.4byte	.LASF177
	.byte	0x6
	.2byte	0x333
	.4byte	0x238
	.byte	0x2
	.byte	0x23
	.uleb128 0x44
	.uleb128 0x13
	.ascii	"DCR\000"
	.byte	0x6
	.2byte	0x334
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x48
	.uleb128 0x12
	.4byte	.LASF178
	.byte	0x6
	.2byte	0x335
	.4byte	0x22d
	.byte	0x2
	.byte	0x23
	.uleb128 0x4a
	.uleb128 0x12
	.4byte	.LASF179
	.byte	0x6
	.2byte	0x336
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x4c
	.uleb128 0x12
	.4byte	.LASF180
	.byte	0x6
	.2byte	0x337
	.4byte	0x22d
	.byte	0x2
	.byte	0x23
	.uleb128 0x4e
	.uleb128 0x13
	.ascii	"OR\000"
	.byte	0x6
	.2byte	0x338
	.4byte	0x579
	.byte	0x2
	.byte	0x23
	.uleb128 0x50
	.uleb128 0x12
	.4byte	.LASF181
	.byte	0x6
	.2byte	0x339
	.4byte	0x22d
	.byte	0x2
	.byte	0x23
	.uleb128 0x52
	.byte	0x0
	.uleb128 0x14
	.4byte	.LASF182
	.byte	0x6
	.2byte	0x33a
	.4byte	0xa13
	.uleb128 0x11
	.byte	0xb4
	.byte	0x7
	.2byte	0x141
	.4byte	0xcae
	.uleb128 0x12
	.4byte	.LASF183
	.byte	0x7
	.2byte	0x143
	.4byte	0xcae
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x13
	.ascii	"utc\000"
	.byte	0x7
	.2byte	0x14b
	.4byte	0x7b
	.byte	0x3
	.byte	0x23
	.uleb128 0xa0
	.uleb128 0x12
	.4byte	.LASF184
	.byte	0x7
	.2byte	0x151
	.4byte	0x8d
	.byte	0x3
	.byte	0x23
	.uleb128 0xa4
	.uleb128 0x12
	.4byte	.LASF185
	.byte	0x7
	.2byte	0x152
	.4byte	0x62
	.byte	0x3
	.byte	0x23
	.uleb128 0xa8
	.uleb128 0x12
	.4byte	.LASF186
	.byte	0x7
	.2byte	0x153
	.4byte	0x62
	.byte	0x3
	.byte	0x23
	.uleb128 0xaa
	.uleb128 0x12
	.4byte	.LASF187
	.byte	0x7
	.2byte	0x156
	.4byte	0x62
	.byte	0x3
	.byte	0x23
	.uleb128 0xac
	.uleb128 0x12
	.4byte	.LASF188
	.byte	0x7
	.2byte	0x157
	.4byte	0x62
	.byte	0x3
	.byte	0x23
	.uleb128 0xae
	.uleb128 0x12
	.4byte	.LASF189
	.byte	0x7
	.2byte	0x158
	.4byte	0x62
	.byte	0x3
	.byte	0x23
	.uleb128 0xb0
	.byte	0x0
	.uleb128 0xb
	.4byte	0x7b
	.4byte	0xcbe
	.uleb128 0xc
	.4byte	0x86
	.byte	0x27
	.byte	0x0
	.uleb128 0x14
	.4byte	.LASF190
	.byte	0x7
	.2byte	0x15a
	.4byte	0xc25
	.uleb128 0xf
	.byte	0x2
	.byte	0x8
	.byte	0x2b
	.4byte	0xce1
	.uleb128 0x9
	.4byte	.LASF191
	.byte	0x8
	.byte	0x2c
	.4byte	0x3e
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.byte	0x0
	.uleb128 0x3
	.4byte	.LASF192
	.byte	0x8
	.byte	0x2d
	.4byte	0xcca
	.uleb128 0x17
	.byte	0x4
	.4byte	0x2c
	.uleb128 0x18
	.byte	0x1
	.byte	0x9
	.byte	0x43
	.4byte	0xd2b
	.uleb128 0x6
	.4byte	.LASF193
	.sleb128 -1
	.uleb128 0x6
	.4byte	.LASF194
	.sleb128 0
	.uleb128 0x6
	.4byte	.LASF195
	.sleb128 1
	.uleb128 0x6
	.4byte	.LASF196
	.sleb128 2
	.uleb128 0x6
	.4byte	.LASF197
	.sleb128 3
	.uleb128 0x6
	.4byte	.LASF198
	.sleb128 4
	.uleb128 0x6
	.4byte	.LASF199
	.sleb128 5
	.uleb128 0x6
	.4byte	.LASF200
	.sleb128 6
	.byte	0x0
	.uleb128 0xb
	.4byte	0x62
	.4byte	0xd3b
	.uleb128 0xc
	.4byte	0x86
	.byte	0x3
	.byte	0x0
	.uleb128 0x3
	.4byte	.LASF201
	.byte	0xa
	.byte	0x58
	.4byte	0x62
	.uleb128 0xf
	.byte	0x14
	.byte	0xb
	.byte	0x3f
	.4byte	0xdb1
	.uleb128 0x9
	.4byte	.LASF202
	.byte	0xb
	.byte	0x40
	.4byte	0xd3b
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x9
	.4byte	.LASF203
	.byte	0xb
	.byte	0x41
	.4byte	0x62
	.byte	0x2
	.byte	0x23
	.uleb128 0x2
	.uleb128 0x9
	.4byte	.LASF204
	.byte	0xb
	.byte	0x42
	.4byte	0x62
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x9
	.4byte	.LASF205
	.byte	0xb
	.byte	0x43
	.4byte	0x62
	.byte	0x2
	.byte	0x23
	.uleb128 0x6
	.uleb128 0x9
	.4byte	.LASF206
	.byte	0xb
	.byte	0x44
	.4byte	0x62
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x9
	.4byte	.LASF207
	.byte	0xb
	.byte	0x45
	.4byte	0xdc1
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x9
	.4byte	.LASF208
	.byte	0xb
	.byte	0x46
	.4byte	0xddc
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.byte	0x0
	.uleb128 0x19
	.byte	0x1
	.4byte	0x62
	.4byte	0xdc1
	.uleb128 0x1a
	.4byte	0x50
	.byte	0x0
	.uleb128 0x17
	.byte	0x4
	.4byte	0xdb1
	.uleb128 0x19
	.byte	0x1
	.4byte	0x2c
	.4byte	0xddc
	.uleb128 0x1a
	.4byte	0x50
	.uleb128 0x1a
	.4byte	0x62
	.byte	0x0
	.uleb128 0x17
	.byte	0x4
	.4byte	0xdc7
	.uleb128 0x3
	.4byte	.LASF209
	.byte	0xb
	.byte	0x47
	.4byte	0xd46
	.uleb128 0x1b
	.byte	0x1
	.4byte	.LASF332
	.byte	0x1
	.2byte	0x322
	.byte	0x1
	.byte	0x3
	.uleb128 0x1c
	.4byte	.LASF333
	.byte	0x3
	.2byte	0x5bc
	.byte	0x1
	.byte	0x3
	.4byte	0xe2b
	.uleb128 0x1d
	.4byte	.LASF334
	.byte	0x3
	.2byte	0x5bc
	.4byte	0x238
	.uleb128 0x1e
	.4byte	.LASF210
	.byte	0x3
	.2byte	0x5be
	.4byte	0x238
	.uleb128 0x1e
	.4byte	.LASF211
	.byte	0x3
	.2byte	0x5bf
	.4byte	0x238
	.byte	0x0
	.uleb128 0x1f
	.byte	0x1
	.4byte	.LASF212
	.byte	0x1
	.2byte	0x1c9
	.byte	0x1
	.4byte	.LFB29
	.4byte	.LFE29
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x1f
	.byte	0x1
	.4byte	.LASF213
	.byte	0x1
	.2byte	0x1e0
	.byte	0x1
	.4byte	.LFB30
	.4byte	.LFE30
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x1f
	.byte	0x1
	.4byte	.LASF214
	.byte	0x1
	.2byte	0x1eb
	.byte	0x1
	.4byte	.LFB31
	.4byte	.LFE31
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x1f
	.byte	0x1
	.4byte	.LASF215
	.byte	0x1
	.2byte	0x207
	.byte	0x1
	.4byte	.LFB32
	.4byte	.LFE32
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x20
	.byte	0x1
	.4byte	.LASF216
	.byte	0x1
	.2byte	0x21d
	.byte	0x1
	.4byte	.LFB33
	.4byte	.LFE33
	.4byte	.LLST0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF229
	.byte	0x1
	.2byte	0x2cd
	.4byte	.LFB34
	.4byte	.LFE34
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x22
	.4byte	0xded
	.4byte	.LFB35
	.4byte	.LFE35
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x1f
	.byte	0x1
	.4byte	.LASF217
	.byte	0x1
	.2byte	0x331
	.byte	0x1
	.4byte	.LFB36
	.4byte	.LFE36
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x23
	.byte	0x1
	.4byte	.LASF224
	.byte	0x2
	.byte	0xe1
	.byte	0x1
	.4byte	.LFB37
	.4byte	.LFE37
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0xef6
	.uleb128 0x24
	.4byte	.LASF233
	.byte	0x2
	.byte	0xe2
	.4byte	0x3e
	.4byte	.LLST1
	.byte	0x0
	.uleb128 0x25
	.byte	0x1
	.4byte	.LASF218
	.byte	0x2
	.byte	0xef
	.byte	0x1
	.4byte	.LFB38
	.4byte	.LFE38
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x25
	.byte	0x1
	.4byte	.LASF219
	.byte	0x2
	.byte	0xf8
	.byte	0x1
	.4byte	.LFB39
	.4byte	.LFE39
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x1f
	.byte	0x1
	.4byte	.LASF220
	.byte	0x2
	.2byte	0x101
	.byte	0x1
	.4byte	.LFB40
	.4byte	.LFE40
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x1f
	.byte	0x1
	.4byte	.LASF221
	.byte	0x2
	.2byte	0x10a
	.byte	0x1
	.4byte	.LFB41
	.4byte	.LFE41
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x1f
	.byte	0x1
	.4byte	.LASF222
	.byte	0x2
	.2byte	0x113
	.byte	0x1
	.4byte	.LFB42
	.4byte	.LFE42
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x20
	.byte	0x1
	.4byte	.LASF223
	.byte	0x2
	.2byte	0x11a
	.byte	0x1
	.4byte	.LFB43
	.4byte	.LFE43
	.4byte	.LLST2
	.uleb128 0x26
	.byte	0x1
	.4byte	.LASF231
	.byte	0x2
	.2byte	0x143
	.4byte	.LFB44
	.4byte	.LFE44
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0xf9c
	.uleb128 0x27
	.byte	0x1
	.4byte	.LASF226
	.byte	0x2
	.2byte	0x146
	.4byte	0x251
	.byte	0x1
	.uleb128 0x28
	.byte	0x0
	.byte	0x0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF225
	.byte	0x2
	.2byte	0x14a
	.byte	0x1
	.4byte	.LFB45
	.4byte	.LFE45
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0xfc6
	.uleb128 0x27
	.byte	0x1
	.4byte	.LASF227
	.byte	0x2
	.2byte	0x153
	.4byte	0x251
	.byte	0x1
	.uleb128 0x28
	.byte	0x0
	.byte	0x0
	.uleb128 0x1f
	.byte	0x1
	.4byte	.LASF228
	.byte	0x2
	.2byte	0x167
	.byte	0x1
	.4byte	.LFB46
	.4byte	.LFE46
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x2a
	.byte	0x1
	.4byte	.LASF230
	.byte	0x2
	.2byte	0x1d8
	.4byte	.LFB48
	.4byte	.LFE48
	.4byte	.LLST3
	.uleb128 0x2b
	.byte	0x1
	.4byte	.LASF232
	.byte	0x2
	.2byte	0x1e5
	.4byte	.LFB49
	.4byte	.LFE49
	.4byte	.LLST4
	.4byte	0x103e
	.uleb128 0x2c
	.4byte	.Ldebug_ranges0+0x0
	.uleb128 0x2d
	.4byte	.LASF234
	.byte	0x2
	.2byte	0x1fd
	.4byte	0x103e
	.4byte	.LLST5
	.uleb128 0x2d
	.4byte	.LASF235
	.byte	0x2
	.2byte	0x1fe
	.4byte	0x1044
	.4byte	.LLST6
	.uleb128 0x2e
	.ascii	"i\000"
	.byte	0x2
	.2byte	0x1ff
	.4byte	0x3e
	.4byte	.LLST7
	.byte	0x0
	.byte	0x0
	.uleb128 0x17
	.byte	0x4
	.4byte	0xde2
	.uleb128 0x17
	.byte	0x4
	.4byte	0x62
	.uleb128 0x1f
	.byte	0x1
	.4byte	.LASF236
	.byte	0x2
	.2byte	0x20d
	.byte	0x1
	.4byte	.LFB50
	.4byte	.LFE50
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x1f
	.byte	0x1
	.4byte	.LASF237
	.byte	0x2
	.2byte	0x219
	.byte	0x1
	.4byte	.LFB51
	.4byte	.LFE51
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x1f
	.byte	0x1
	.4byte	.LASF238
	.byte	0x2
	.2byte	0x227
	.byte	0x1
	.4byte	.LFB52
	.4byte	.LFE52
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x1f
	.byte	0x1
	.4byte	.LASF239
	.byte	0x2
	.2byte	0x239
	.byte	0x1
	.4byte	.LFB53
	.4byte	.LFE53
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x2b
	.byte	0x1
	.4byte	.LASF240
	.byte	0x2
	.2byte	0x258
	.4byte	.LFB54
	.4byte	.LFE54
	.4byte	.LLST8
	.4byte	0x10c4
	.uleb128 0x1e
	.4byte	.LASF241
	.byte	0x2
	.2byte	0x25b
	.4byte	0x62
	.byte	0x0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF242
	.byte	0x2
	.2byte	0x2d6
	.4byte	.LFB55
	.4byte	.LFE55
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x2f
	.4byte	.LASF244
	.byte	0x3
	.2byte	0x4b5
	.byte	0x3
	.uleb128 0x26
	.byte	0x1
	.4byte	.LASF243
	.byte	0x2
	.2byte	0x2de
	.4byte	.LFB56
	.4byte	.LFE56
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x110a
	.uleb128 0x30
	.4byte	0x10d8
	.4byte	.LBB8
	.4byte	.LBE8
	.byte	0x2
	.2byte	0x2df
	.byte	0x0
	.uleb128 0x2f
	.4byte	.LASF245
	.byte	0x3
	.2byte	0x4b4
	.byte	0x3
	.uleb128 0x26
	.byte	0x1
	.4byte	.LASF246
	.byte	0x2
	.2byte	0x2e5
	.4byte	.LFB57
	.4byte	.LFE57
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x113c
	.uleb128 0x30
	.4byte	0x110a
	.4byte	.LBB12
	.4byte	.LBE12
	.byte	0x2
	.2byte	0x2e6
	.byte	0x0
	.uleb128 0x26
	.byte	0x1
	.4byte	.LASF247
	.byte	0x2
	.2byte	0x2f5
	.4byte	.LFB58
	.4byte	.LFE58
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x116f
	.uleb128 0x31
	.4byte	.LBB14
	.4byte	.LBE14
	.uleb128 0x2d
	.4byte	.LASF241
	.byte	0x2
	.2byte	0x2fc
	.4byte	0x62
	.4byte	.LLST9
	.byte	0x0
	.byte	0x0
	.uleb128 0x2a
	.byte	0x1
	.4byte	.LASF248
	.byte	0x2
	.2byte	0x31a
	.4byte	.LFB59
	.4byte	.LFE59
	.4byte	.LLST10
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF249
	.byte	0x2
	.2byte	0x32f
	.4byte	.LFB60
	.4byte	.LFE60
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x32
	.byte	0x1
	.4byte	.LASF335
	.byte	0x2
	.2byte	0x33f
	.byte	0x1
	.4byte	0x8d
	.4byte	.LFB61
	.4byte	.LFE61
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x11c4
	.uleb128 0x33
	.ascii	"tsp\000"
	.byte	0x2
	.2byte	0x341
	.4byte	0x7b
	.byte	0x1
	.byte	0x50
	.byte	0x0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF250
	.byte	0x2
	.2byte	0x34d
	.byte	0x1
	.4byte	.LFB62
	.4byte	.LFE62
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x11f8
	.uleb128 0x34
	.ascii	"tsp\000"
	.byte	0x2
	.2byte	0x34d
	.4byte	0x8d
	.byte	0x1
	.byte	0x50
	.uleb128 0x1e
	.4byte	.LASF251
	.byte	0x2
	.2byte	0x34f
	.4byte	0x7b
	.byte	0x0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF252
	.byte	0x2
	.2byte	0x35a
	.byte	0x1
	.4byte	.LFB63
	.4byte	.LFE63
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x1220
	.uleb128 0x34
	.ascii	"tsp\000"
	.byte	0x2
	.2byte	0x35a
	.4byte	0x8d
	.byte	0x1
	.byte	0x50
	.byte	0x0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF253
	.byte	0x2
	.2byte	0x35f
	.byte	0x1
	.4byte	.LFB64
	.4byte	.LFE64
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x1260
	.uleb128 0x34
	.ascii	"i\000"
	.byte	0x2
	.2byte	0x35f
	.4byte	0x50
	.byte	0x1
	.byte	0x50
	.uleb128 0x31
	.4byte	.LBB15
	.4byte	.LBE15
	.uleb128 0x2d
	.4byte	.LASF254
	.byte	0x2
	.2byte	0x36e
	.4byte	0x7b
	.4byte	.LLST11
	.byte	0x0
	.byte	0x0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF255
	.byte	0x2
	.2byte	0x377
	.4byte	.LFB65
	.4byte	.LFE65
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x2b
	.byte	0x1
	.4byte	.LASF256
	.byte	0x2
	.2byte	0x38d
	.4byte	.LFB66
	.4byte	.LFE66
	.4byte	.LLST12
	.4byte	0x12a6
	.uleb128 0x1e
	.4byte	.LASF257
	.byte	0x2
	.2byte	0x397
	.4byte	0x7b
	.uleb128 0x35
	.4byte	.LASF336
	.byte	0x2
	.2byte	0x3a3
	.4byte	.L88
	.byte	0x0
	.uleb128 0x26
	.byte	0x1
	.4byte	.LASF258
	.byte	0x2
	.2byte	0x3c4
	.4byte	.LFB67
	.4byte	.LFE67
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x12ef
	.uleb128 0x36
	.4byte	0xdf8
	.4byte	.LBB18
	.4byte	.LBE18
	.byte	0x2
	.2byte	0x3ee
	.uleb128 0x31
	.4byte	.LBB19
	.4byte	.LBE19
	.uleb128 0x37
	.4byte	0xe12
	.4byte	.LLST13
	.uleb128 0x38
	.4byte	0xe1e
	.byte	0x6
	.uleb128 0x39
	.4byte	0xe06
	.byte	0x6
	.byte	0x0
	.byte	0x0
	.byte	0x0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF259
	.byte	0x2
	.2byte	0x43a
	.4byte	.LFB68
	.4byte	.LFE68
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF260
	.byte	0x2
	.2byte	0x447
	.byte	0x1
	.4byte	.LFB69
	.4byte	.LFE69
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x132d
	.uleb128 0x3a
	.4byte	.LASF261
	.byte	0x2
	.2byte	0x447
	.4byte	0x50
	.4byte	.LLST14
	.byte	0x0
	.uleb128 0x2b
	.byte	0x1
	.4byte	.LASF262
	.byte	0x2
	.2byte	0x1ad
	.4byte	.LFB47
	.4byte	.LFE47
	.4byte	.LLST15
	.4byte	0x1357
	.uleb128 0x27
	.byte	0x1
	.4byte	.LASF263
	.byte	0x2
	.2byte	0x1b6
	.4byte	0x251
	.byte	0x1
	.uleb128 0x28
	.byte	0x0
	.byte	0x0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF264
	.byte	0x2
	.2byte	0x47d
	.4byte	.LFB70
	.4byte	.LFE70
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF265
	.byte	0x2
	.2byte	0x486
	.4byte	.LFB71
	.4byte	.LFE71
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF266
	.byte	0x2
	.2byte	0x48d
	.byte	0x1
	.4byte	.LFB72
	.4byte	.LFE72
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x13a7
	.uleb128 0x3b
	.4byte	.LASF267
	.byte	0x2
	.2byte	0x48d
	.4byte	0x50
	.byte	0x1
	.byte	0x50
	.byte	0x0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF268
	.byte	0x2
	.2byte	0x49d
	.byte	0x1
	.4byte	.LFB73
	.4byte	.LFE73
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x13cf
	.uleb128 0x3b
	.4byte	.LASF269
	.byte	0x2
	.2byte	0x49d
	.4byte	0x7b
	.byte	0x1
	.byte	0x50
	.byte	0x0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF270
	.byte	0x2
	.2byte	0x4c1
	.4byte	.LFB74
	.4byte	.LFE74
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x3c
	.byte	0x1
	.4byte	.LASF271
	.byte	0x2
	.2byte	0x4cf
	.4byte	0x62
	.4byte	.LFB75
	.4byte	.LFE75
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x3c
	.byte	0x1
	.4byte	.LASF272
	.byte	0x2
	.2byte	0x4d3
	.4byte	0x62
	.4byte	.LFB76
	.4byte	.LFE76
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF273
	.byte	0x2
	.2byte	0x4d7
	.4byte	.LFB77
	.4byte	.LFE77
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF274
	.byte	0x2
	.2byte	0x4db
	.4byte	.LFB78
	.4byte	.LFE78
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF275
	.byte	0x2
	.2byte	0x4df
	.4byte	.LFB79
	.4byte	.LFE79
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF276
	.byte	0x2
	.2byte	0x4e3
	.4byte	.LFB80
	.4byte	.LFE80
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF277
	.byte	0x2
	.2byte	0x4e8
	.byte	0x1
	.4byte	.LFB81
	.4byte	.LFE81
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x148b
	.uleb128 0x3b
	.4byte	.LASF269
	.byte	0x2
	.2byte	0x4e8
	.4byte	0x62
	.byte	0x1
	.byte	0x50
	.byte	0x0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF278
	.byte	0x2
	.2byte	0x4f0
	.byte	0x1
	.4byte	.LFB82
	.4byte	.LFE82
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x14b5
	.uleb128 0x3a
	.4byte	.LASF269
	.byte	0x2
	.2byte	0x4f0
	.4byte	0x62
	.4byte	.LLST16
	.byte	0x0
	.uleb128 0x3c
	.byte	0x1
	.4byte	.LASF279
	.byte	0x2
	.2byte	0x4f6
	.4byte	0x62
	.4byte	.LFB83
	.4byte	.LFE83
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF280
	.byte	0x2
	.2byte	0x4fd
	.byte	0x1
	.4byte	.LFB84
	.4byte	.LFE84
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x14f5
	.uleb128 0x3b
	.4byte	.LASF281
	.byte	0x2
	.2byte	0x4fd
	.4byte	0x62
	.byte	0x1
	.byte	0x50
	.byte	0x0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF282
	.byte	0x2
	.2byte	0x517
	.4byte	.LFB85
	.4byte	.LFE85
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF283
	.byte	0x2
	.2byte	0x51b
	.4byte	.LFB86
	.4byte	.LFE86
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF284
	.byte	0x2
	.2byte	0x51f
	.4byte	.LFB87
	.4byte	.LFE87
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF285
	.byte	0x2
	.2byte	0x52b
	.4byte	.LFB88
	.4byte	.LFE88
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF286
	.byte	0x2
	.2byte	0x538
	.4byte	.LFB89
	.4byte	.LFE89
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF287
	.byte	0x2
	.2byte	0x544
	.byte	0x1
	.4byte	.LFB90
	.4byte	.LFE90
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x1581
	.uleb128 0x3b
	.4byte	.LASF288
	.byte	0x2
	.2byte	0x544
	.4byte	0x7b
	.byte	0x1
	.byte	0x50
	.byte	0x0
	.uleb128 0x3c
	.byte	0x1
	.4byte	.LASF289
	.byte	0x2
	.2byte	0x551
	.4byte	0x7b
	.4byte	.LFB91
	.4byte	.LFE91
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF290
	.byte	0x2
	.2byte	0x55b
	.byte	0x1
	.4byte	.LFB92
	.4byte	.LFE92
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x15dd
	.uleb128 0x3b
	.4byte	.LASF291
	.byte	0x2
	.2byte	0x55b
	.4byte	0x2c
	.byte	0x1
	.byte	0x50
	.uleb128 0x3b
	.4byte	.LASF292
	.byte	0x2
	.2byte	0x55b
	.4byte	0x2c
	.byte	0x1
	.byte	0x51
	.uleb128 0x3b
	.4byte	.LASF293
	.byte	0x2
	.2byte	0x55b
	.4byte	0x62
	.byte	0x1
	.byte	0x52
	.byte	0x0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF294
	.byte	0x2
	.2byte	0x58d
	.4byte	.LFB93
	.4byte	.LFE93
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF295
	.byte	0x2
	.2byte	0x58e
	.4byte	.LFB94
	.4byte	.LFE94
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x26
	.byte	0x1
	.4byte	.LASF296
	.byte	0x2
	.2byte	0x58f
	.4byte	.LFB95
	.4byte	.LFE95
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x162e
	.uleb128 0x27
	.byte	0x1
	.4byte	.LASF297
	.byte	0x2
	.2byte	0x58f
	.4byte	0x251
	.byte	0x1
	.uleb128 0x28
	.byte	0x0
	.byte	0x0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF298
	.byte	0x2
	.2byte	0x597
	.4byte	.LFB96
	.4byte	.LFE96
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF299
	.byte	0x2
	.2byte	0x598
	.4byte	.LFB97
	.4byte	.LFE97
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x21
	.byte	0x1
	.4byte	.LASF300
	.byte	0x2
	.2byte	0x599
	.4byte	.LFB98
	.4byte	.LFE98
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF301
	.byte	0x2
	.2byte	0x5b0
	.byte	0x1
	.4byte	.LFB99
	.4byte	.LFE99
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x16a0
	.uleb128 0x3b
	.4byte	.LASF302
	.byte	0x2
	.2byte	0x5b0
	.4byte	0xcec
	.byte	0x1
	.byte	0x50
	.uleb128 0x3b
	.4byte	.LASF303
	.byte	0x2
	.2byte	0x5b0
	.4byte	0x3e
	.byte	0x1
	.byte	0x51
	.byte	0x0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF304
	.byte	0x2
	.2byte	0x606
	.byte	0x1
	.4byte	.LFB100
	.4byte	.LFE100
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x16c8
	.uleb128 0x3b
	.4byte	.LASF305
	.byte	0x2
	.2byte	0x606
	.4byte	0x62
	.byte	0x1
	.byte	0x50
	.byte	0x0
	.uleb128 0x3d
	.byte	0x1
	.4byte	.LASF306
	.byte	0x2
	.2byte	0x612
	.4byte	0x62
	.4byte	.LFB102
	.4byte	.LFE102
	.4byte	.LLST17
	.4byte	0x1705
	.uleb128 0x3e
	.4byte	.LASF307
	.byte	0x2
	.2byte	0x613
	.4byte	0x62
	.byte	0x2
	.byte	0x70
	.sleb128 -2
	.uleb128 0x27
	.byte	0x1
	.4byte	.LASF308
	.byte	0x2
	.2byte	0x616
	.4byte	0x251
	.byte	0x1
	.uleb128 0x28
	.byte	0x0
	.byte	0x0
	.uleb128 0x3f
	.byte	0x1
	.4byte	.LASF309
	.byte	0x2
	.2byte	0x60c
	.4byte	0x2c
	.4byte	.LFB101
	.4byte	.LFE101
	.4byte	.LLST18
	.uleb128 0x40
	.byte	0x1
	.4byte	.LASF310
	.byte	0x2
	.2byte	0x63a
	.byte	0x1
	.4byte	.LFB103
	.4byte	.LFE103
	.4byte	.LLST19
	.4byte	0x179d
	.uleb128 0x3a
	.4byte	.LASF311
	.byte	0x2
	.2byte	0x63a
	.4byte	0xcec
	.4byte	.LLST20
	.uleb128 0x41
	.ascii	"src\000"
	.byte	0x2
	.2byte	0x63a
	.4byte	0xcec
	.4byte	.LLST21
	.uleb128 0x3a
	.4byte	.LASF206
	.byte	0x2
	.2byte	0x63a
	.4byte	0x3e
	.4byte	.LLST22
	.uleb128 0x33
	.ascii	"ccr\000"
	.byte	0x2
	.2byte	0x63c
	.4byte	0x179d
	.byte	0x5
	.byte	0x3
	.4byte	ccr.6289
	.uleb128 0x3e
	.4byte	.LASF312
	.byte	0x2
	.2byte	0x63d
	.4byte	0x17a2
	.byte	0x5
	.byte	0x3
	.4byte	len_div.6290
	.uleb128 0x2d
	.4byte	.LASF313
	.byte	0x2
	.2byte	0x63e
	.4byte	0x3e
	.4byte	.LLST23
	.byte	0x0
	.uleb128 0x10
	.4byte	0xd2b
	.uleb128 0x10
	.4byte	0xd2b
	.uleb128 0x40
	.byte	0x1
	.4byte	.LASF314
	.byte	0x2
	.2byte	0x64a
	.byte	0x1
	.4byte	.LFB104
	.4byte	.LFE104
	.4byte	.LLST24
	.4byte	0x17fe
	.uleb128 0x3b
	.4byte	.LASF311
	.byte	0x2
	.2byte	0x64a
	.4byte	0xcec
	.byte	0x1
	.byte	0x50
	.uleb128 0x34
	.ascii	"src\000"
	.byte	0x2
	.2byte	0x64a
	.4byte	0xcec
	.byte	0x1
	.byte	0x51
	.uleb128 0x3a
	.4byte	.LASF206
	.byte	0x2
	.2byte	0x64a
	.4byte	0x3e
	.4byte	.LLST25
	.uleb128 0x2d
	.4byte	.LASF315
	.byte	0x2
	.2byte	0x64c
	.4byte	0x62
	.4byte	.LLST26
	.byte	0x0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF316
	.byte	0x2
	.2byte	0x65d
	.byte	0x1
	.4byte	.LFB105
	.4byte	.LFE105
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x1842
	.uleb128 0x3b
	.4byte	.LASF311
	.byte	0x2
	.2byte	0x65d
	.4byte	0xcec
	.byte	0x1
	.byte	0x50
	.uleb128 0x34
	.ascii	"src\000"
	.byte	0x2
	.2byte	0x65d
	.4byte	0xcec
	.byte	0x1
	.byte	0x51
	.uleb128 0x3b
	.4byte	.LASF206
	.byte	0x2
	.2byte	0x65d
	.4byte	0x3e
	.byte	0x1
	.byte	0x52
	.byte	0x0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF317
	.byte	0x2
	.2byte	0x671
	.byte	0x1
	.4byte	.LFB106
	.4byte	.LFE106
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x1886
	.uleb128 0x3b
	.4byte	.LASF311
	.byte	0x2
	.2byte	0x671
	.4byte	0x1044
	.byte	0x1
	.byte	0x50
	.uleb128 0x34
	.ascii	"src\000"
	.byte	0x2
	.2byte	0x671
	.4byte	0x1044
	.byte	0x1
	.byte	0x51
	.uleb128 0x3b
	.4byte	.LASF206
	.byte	0x2
	.2byte	0x671
	.4byte	0x3e
	.byte	0x1
	.byte	0x52
	.byte	0x0
	.uleb128 0x40
	.byte	0x1
	.4byte	.LASF318
	.byte	0x2
	.2byte	0x683
	.byte	0x1
	.4byte	.LFB107
	.4byte	.LFE107
	.4byte	.LLST27
	.4byte	0x18cd
	.uleb128 0x3b
	.4byte	.LASF311
	.byte	0x2
	.2byte	0x683
	.4byte	0xcec
	.byte	0x1
	.byte	0x50
	.uleb128 0x3a
	.4byte	.LASF269
	.byte	0x2
	.2byte	0x683
	.4byte	0x2c
	.4byte	.LLST28
	.uleb128 0x3b
	.4byte	.LASF206
	.byte	0x2
	.2byte	0x683
	.4byte	0x3e
	.byte	0x1
	.byte	0x52
	.byte	0x0
	.uleb128 0x2f
	.4byte	.LASF319
	.byte	0x3
	.2byte	0x4bc
	.byte	0x3
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF320
	.byte	0x2
	.2byte	0x69b
	.byte	0x1
	.4byte	.LFB108
	.4byte	.LFE108
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x191c
	.uleb128 0x3b
	.4byte	.LASF321
	.byte	0x2
	.2byte	0x69b
	.4byte	0x62
	.byte	0x1
	.byte	0x50
	.uleb128 0x3b
	.4byte	.LASF261
	.byte	0x2
	.2byte	0x69b
	.4byte	0x62
	.byte	0x1
	.byte	0x51
	.uleb128 0x30
	.4byte	0x18cd
	.4byte	.LBB22
	.4byte	.LBE22
	.byte	0x2
	.2byte	0x6a4
	.byte	0x0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF322
	.byte	0x2
	.2byte	0x6ab
	.byte	0x1
	.4byte	.LFB109
	.4byte	.LFE109
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x1944
	.uleb128 0x34
	.ascii	"sti\000"
	.byte	0x2
	.2byte	0x6ab
	.4byte	0x62
	.byte	0x1
	.byte	0x50
	.byte	0x0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF323
	.byte	0x2
	.2byte	0x6b2
	.byte	0x1
	.4byte	.LFB110
	.4byte	.LFE110
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x196a
	.uleb128 0x34
	.ascii	"n\000"
	.byte	0x2
	.2byte	0x6b2
	.4byte	0x62
	.byte	0x1
	.byte	0x50
	.byte	0x0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF324
	.byte	0x2
	.2byte	0x6ba
	.byte	0x1
	.4byte	.LFB111
	.4byte	.LFE111
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x199a
	.uleb128 0x34
	.ascii	"n\000"
	.byte	0x2
	.2byte	0x6ba
	.4byte	0x62
	.byte	0x1
	.byte	0x50
	.uleb128 0x42
	.ascii	"c\000"
	.byte	0x2
	.2byte	0x6bb
	.4byte	0x7b
	.byte	0x0
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF325
	.byte	0x2
	.2byte	0x6c5
	.byte	0x1
	.4byte	.LFB112
	.4byte	.LFE112
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	0x19ca
	.uleb128 0x34
	.ascii	"n\000"
	.byte	0x2
	.2byte	0x6c5
	.4byte	0x62
	.byte	0x1
	.byte	0x50
	.uleb128 0x42
	.ascii	"c\000"
	.byte	0x2
	.2byte	0x6c6
	.4byte	0x7b
	.byte	0x0
	.uleb128 0x43
	.4byte	.LASF326
	.byte	0x3
	.2byte	0x6ce
	.4byte	0x19d8
	.byte	0x1
	.byte	0x1
	.uleb128 0xd
	.4byte	0x251
	.uleb128 0x43
	.4byte	.LASF327
	.byte	0x7
	.2byte	0x15d
	.4byte	0xcbe
	.byte	0x1
	.byte	0x1
	.uleb128 0x44
	.4byte	.LASF328
	.byte	0x8
	.byte	0x2f
	.4byte	0xce1
	.byte	0x1
	.byte	0x1
	.uleb128 0x43
	.4byte	.LASF326
	.byte	0x3
	.2byte	0x6ce
	.4byte	0x19d8
	.byte	0x1
	.byte	0x1
	.uleb128 0x45
	.4byte	.LASF327
	.byte	0x2
	.byte	0xc1
	.4byte	0xcbe
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.4byte	platform_ext
	.uleb128 0x45
	.4byte	.LASF328
	.byte	0x2
	.byte	0xc0
	.4byte	0xce1
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.4byte	platform
	.byte	0x0
	.section	.debug_abbrev
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x52
	.uleb128 0x1
	.uleb128 0x55
	.uleb128 0x6
	.uleb128 0x10
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0x0
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x16
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x4
	.uleb128 0xf
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x5
	.uleb128 0x4
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x6
	.uleb128 0x28
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1c
	.uleb128 0xd
	.byte	0x0
	.byte	0x0
	.uleb128 0x7
	.uleb128 0x24
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0x0
	.byte	0x0
	.uleb128 0x8
	.uleb128 0x13
	.byte	0x1
	.uleb128 0xb
	.uleb128 0x5
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x9
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0xa
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0xb
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0xc
	.uleb128 0x21
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0xd
	.uleb128 0x35
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0xe
	.uleb128 0x21
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0x5
	.byte	0x0
	.byte	0x0
	.uleb128 0xf
	.uleb128 0x13
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x10
	.uleb128 0x26
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x11
	.uleb128 0x13
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x12
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x13
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x14
	.uleb128 0x16
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x15
	.uleb128 0x4
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x16
	.uleb128 0x28
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x1c
	.uleb128 0xd
	.byte	0x0
	.byte	0x0
	.uleb128 0x17
	.uleb128 0xf
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x18
	.uleb128 0x4
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x19
	.uleb128 0x15
	.byte	0x1
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x1a
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x1b
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x20
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x1c
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x20
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x1d
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x1e
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x1f
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x20
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x21
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x22
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x23
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x24
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x25
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x26
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x27
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0x0
	.byte	0x0
	.uleb128 0x28
	.uleb128 0x18
	.byte	0x0
	.byte	0x0
	.byte	0x0
	.uleb128 0x29
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x2a
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x2b
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x2c
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x55
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x2d
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x2e
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x2f
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x20
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x30
	.uleb128 0x1d
	.byte	0x0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x58
	.uleb128 0xb
	.uleb128 0x59
	.uleb128 0x5
	.byte	0x0
	.byte	0x0
	.uleb128 0x31
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.byte	0x0
	.byte	0x0
	.uleb128 0x32
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x33
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x34
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x35
	.uleb128 0xa
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x11
	.uleb128 0x1
	.byte	0x0
	.byte	0x0
	.uleb128 0x36
	.uleb128 0x1d
	.byte	0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x58
	.uleb128 0xb
	.uleb128 0x59
	.uleb128 0x5
	.byte	0x0
	.byte	0x0
	.uleb128 0x37
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x38
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x39
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x3a
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x3b
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x3c
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x3d
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x3e
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x3f
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x40
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.uleb128 0x1
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x41
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x42
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x43
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0x0
	.byte	0x0
	.uleb128 0x44
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0x0
	.byte	0x0
	.uleb128 0x45
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.byte	0x0
	.section	.debug_pubnames,"",%progbits
	.4byte	0x7d4
	.2byte	0x2
	.4byte	.Ldebug_info0
	.4byte	0x1a2b
	.4byte	0xe2b
	.ascii	"BOARD_PERIPH_INIT\000"
	.4byte	0xe40
	.ascii	"BOARD_DMA_CLKON\000"
	.4byte	0xe55
	.ascii	"BOARD_DMA_CLKOFF\000"
	.4byte	0xe6a
	.ascii	"BOARD_EXTI_STARTUP\000"
	.4byte	0xe7f
	.ascii	"BOARD_PORT_STARTUP\000"
	.4byte	0xe95
	.ascii	"BOARD_PORT_STANDBY\000"
	.4byte	0xea9
	.ascii	"BOARD_POWER_STARTUP\000"
	.4byte	0xeb9
	.ascii	"BOARD_XTAL_STARTUP\000"
	.4byte	0xece
	.ascii	"NMI_Handler\000"
	.4byte	0xef6
	.ascii	"HardFault_Handler\000"
	.4byte	0xf0a
	.ascii	"MemManage_Handler\000"
	.4byte	0xf1e
	.ascii	"BusFault_Handler\000"
	.4byte	0xf33
	.ascii	"UsageFault_Handler\000"
	.4byte	0xf48
	.ascii	"DebugMon_Handler\000"
	.4byte	0xf5d
	.ascii	"SVC_Handler\000"
	.4byte	0xf73
	.ascii	"OT_GPTIM_ISR\000"
	.4byte	0xf9c
	.ascii	"PendSV_Handler\000"
	.4byte	0xfc6
	.ascii	"WWDG_IRQHandler\000"
	.4byte	0xfdb
	.ascii	"platform_poweroff\000"
	.4byte	0xff0
	.ascii	"platform_init_OT\000"
	.4byte	0x104a
	.ascii	"sub_msflash_config\000"
	.4byte	0x105f
	.ascii	"sub_hsflash_config\000"
	.4byte	0x1074
	.ascii	"sub_voltage_config\000"
	.4byte	0x1089
	.ascii	"sub_hsosc_config\000"
	.4byte	0x109e
	.ascii	"platform_init_busclk\000"
	.4byte	0x10c4
	.ascii	"platform_init_periphclk\000"
	.4byte	0x10e1
	.ascii	"platform_disable_interrupts\000"
	.4byte	0x1113
	.ascii	"platform_enable_interrupts\000"
	.4byte	0x113c
	.ascii	"platform_standard_speed\000"
	.4byte	0x116f
	.ascii	"platform_full_speed\000"
	.4byte	0x1184
	.ascii	"platform_flank_speed\000"
	.4byte	0x1198
	.ascii	"platform_save_context\000"
	.4byte	0x11c4
	.ascii	"platform_load_context\000"
	.4byte	0x11f8
	.ascii	"platform_switch_context\000"
	.4byte	0x1220
	.ascii	"platform_drop_context\000"
	.4byte	0x1260
	.ascii	"platform_ot_preempt\000"
	.4byte	0x1274
	.ascii	"platform_ot_run\000"
	.4byte	0x12a6
	.ascii	"platform_init_interruptor\000"
	.4byte	0x12ef
	.ascii	"platform_init_gpio\000"
	.4byte	0x1303
	.ascii	"platform_init_gptim\000"
	.4byte	0x132d
	.ascii	"platform_poweron\000"
	.4byte	0x1357
	.ascii	"platform_init_watchdog\000"
	.4byte	0x136b
	.ascii	"platform_init_resetswitch\000"
	.4byte	0x137f
	.ascii	"platform_init_systick\000"
	.4byte	0x13a7
	.ascii	"platform_init_rtc\000"
	.4byte	0x13cf
	.ascii	"platform_init_memcpy\000"
	.4byte	0x13e3
	.ascii	"platform_get_ktim\000"
	.4byte	0x13fb
	.ascii	"platform_next_ktim\000"
	.4byte	0x1413
	.ascii	"platform_enable_ktim\000"
	.4byte	0x1427
	.ascii	"platform_disable_ktim\000"
	.4byte	0x143b
	.ascii	"platform_pend_ktim\000"
	.4byte	0x144f
	.ascii	"platform_flush_ktim\000"
	.4byte	0x1463
	.ascii	"platform_set_ktim\000"
	.4byte	0x148b
	.ascii	"platform_set_gptim2\000"
	.4byte	0x14b5
	.ascii	"platform_get_gptim\000"
	.4byte	0x14cd
	.ascii	"platform_set_watchdog\000"
	.4byte	0x14f5
	.ascii	"platform_kill_watchdog\000"
	.4byte	0x1509
	.ascii	"platform_pause_watchdog\000"
	.4byte	0x151d
	.ascii	"platform_resume_watchdog\000"
	.4byte	0x1531
	.ascii	"platform_enable_rtc\000"
	.4byte	0x1545
	.ascii	"platform_disable_rtc\000"
	.4byte	0x1559
	.ascii	"platform_set_time\000"
	.4byte	0x1581
	.ascii	"platform_get_time\000"
	.4byte	0x1599
	.ascii	"platform_set_rtc_alarm\000"
	.4byte	0x15dd
	.ascii	"platform_trig1_high\000"
	.4byte	0x15f1
	.ascii	"platform_trig1_low\000"
	.4byte	0x1605
	.ascii	"platform_trig1_toggle\000"
	.4byte	0x162e
	.ascii	"platform_trig2_high\000"
	.4byte	0x1642
	.ascii	"platform_trig2_low\000"
	.4byte	0x1656
	.ascii	"platform_trig2_toggle\000"
	.4byte	0x166a
	.ascii	"platform_rand\000"
	.4byte	0x16a0
	.ascii	"platform_init_prand\000"
	.4byte	0x16c8
	.ascii	"platform_prand_u16\000"
	.4byte	0x1705
	.ascii	"platform_prand_u8\000"
	.4byte	0x171e
	.ascii	"sub_memcpy_dma\000"
	.4byte	0x17a7
	.ascii	"sub_memcpy2_dma\000"
	.4byte	0x17fe
	.ascii	"platform_memcpy\000"
	.4byte	0x1842
	.ascii	"platform_memcpy_2\000"
	.4byte	0x1886
	.ascii	"platform_memset\000"
	.4byte	0x18d6
	.ascii	"sub_timed_wfe\000"
	.4byte	0x191c
	.ascii	"platform_block\000"
	.4byte	0x1944
	.ascii	"platform_delay\000"
	.4byte	0x196a
	.ascii	"platform_swdelay_ms\000"
	.4byte	0x199a
	.ascii	"platform_swdelay_us\000"
	.4byte	0x1a06
	.ascii	"platform_ext\000"
	.4byte	0x1a18
	.ascii	"platform\000"
	.4byte	0x0
	.section	.debug_pubtypes,"",%progbits
	.4byte	0x193
	.2byte	0x2
	.4byte	.Ldebug_info0
	.4byte	0x1a2b
	.4byte	0x2c
	.ascii	"ot_u8\000"
	.4byte	0x3e
	.ascii	"ot_int\000"
	.4byte	0x50
	.ascii	"ot_uint\000"
	.4byte	0x62
	.ascii	"ot_u16\000"
	.4byte	0x7b
	.ascii	"ot_u32\000"
	.4byte	0x8f
	.ascii	"IRQn\000"
	.4byte	0x222
	.ascii	"uint8_t\000"
	.4byte	0x22d
	.ascii	"uint16_t\000"
	.4byte	0x238
	.ascii	"uint32_t\000"
	.4byte	0x396
	.ascii	"NVIC_Type\000"
	.4byte	0x51c
	.ascii	"SCB_Type\000"
	.4byte	0x56d
	.ascii	"SysTick_Type\000"
	.4byte	0x5d9
	.ascii	"DBGMCU_TypeDef\000"
	.4byte	0x62b
	.ascii	"DMA_Channel_TypeDef\000"
	.4byte	0x65f
	.ascii	"DMA_TypeDef\000"
	.4byte	0x6ce
	.ascii	"EXTI_TypeDef\000"
	.4byte	0x7a9
	.ascii	"FLASH_TypeDef\000"
	.4byte	0x8a5
	.ascii	"GPIO_TypeDef\000"
	.4byte	0x8ed
	.ascii	"SYSCFG_TypeDef\000"
	.4byte	0x920
	.ascii	"PWR_TypeDef\000"
	.4byte	0xa07
	.ascii	"RCC_TypeDef\000"
	.4byte	0xc19
	.ascii	"TIM_TypeDef\000"
	.4byte	0xcbe
	.ascii	"platform_ext_struct\000"
	.4byte	0xce1
	.ascii	"platform_struct\000"
	.4byte	0xd3b
	.ascii	"vaddr\000"
	.4byte	0xde2
	.ascii	"vlFILE\000"
	.4byte	0x0
	.section	.debug_aranges,"",%progbits
	.4byte	0x2b4
	.2byte	0x2
	.4byte	.Ldebug_info0
	.byte	0x4
	.byte	0x0
	.2byte	0x0
	.2byte	0x0
	.4byte	.LFB29
	.4byte	.LFE29-.LFB29
	.4byte	.LFB30
	.4byte	.LFE30-.LFB30
	.4byte	.LFB31
	.4byte	.LFE31-.LFB31
	.4byte	.LFB32
	.4byte	.LFE32-.LFB32
	.4byte	.LFB33
	.4byte	.LFE33-.LFB33
	.4byte	.LFB34
	.4byte	.LFE34-.LFB34
	.4byte	.LFB35
	.4byte	.LFE35-.LFB35
	.4byte	.LFB36
	.4byte	.LFE36-.LFB36
	.4byte	.LFB37
	.4byte	.LFE37-.LFB37
	.4byte	.LFB38
	.4byte	.LFE38-.LFB38
	.4byte	.LFB39
	.4byte	.LFE39-.LFB39
	.4byte	.LFB40
	.4byte	.LFE40-.LFB40
	.4byte	.LFB41
	.4byte	.LFE41-.LFB41
	.4byte	.LFB42
	.4byte	.LFE42-.LFB42
	.4byte	.LFB43
	.4byte	.LFE43-.LFB43
	.4byte	.LFB44
	.4byte	.LFE44-.LFB44
	.4byte	.LFB45
	.4byte	.LFE45-.LFB45
	.4byte	.LFB46
	.4byte	.LFE46-.LFB46
	.4byte	.LFB48
	.4byte	.LFE48-.LFB48
	.4byte	.LFB49
	.4byte	.LFE49-.LFB49
	.4byte	.LFB50
	.4byte	.LFE50-.LFB50
	.4byte	.LFB51
	.4byte	.LFE51-.LFB51
	.4byte	.LFB52
	.4byte	.LFE52-.LFB52
	.4byte	.LFB53
	.4byte	.LFE53-.LFB53
	.4byte	.LFB54
	.4byte	.LFE54-.LFB54
	.4byte	.LFB55
	.4byte	.LFE55-.LFB55
	.4byte	.LFB56
	.4byte	.LFE56-.LFB56
	.4byte	.LFB57
	.4byte	.LFE57-.LFB57
	.4byte	.LFB58
	.4byte	.LFE58-.LFB58
	.4byte	.LFB59
	.4byte	.LFE59-.LFB59
	.4byte	.LFB60
	.4byte	.LFE60-.LFB60
	.4byte	.LFB61
	.4byte	.LFE61-.LFB61
	.4byte	.LFB62
	.4byte	.LFE62-.LFB62
	.4byte	.LFB63
	.4byte	.LFE63-.LFB63
	.4byte	.LFB64
	.4byte	.LFE64-.LFB64
	.4byte	.LFB65
	.4byte	.LFE65-.LFB65
	.4byte	.LFB66
	.4byte	.LFE66-.LFB66
	.4byte	.LFB67
	.4byte	.LFE67-.LFB67
	.4byte	.LFB68
	.4byte	.LFE68-.LFB68
	.4byte	.LFB69
	.4byte	.LFE69-.LFB69
	.4byte	.LFB47
	.4byte	.LFE47-.LFB47
	.4byte	.LFB70
	.4byte	.LFE70-.LFB70
	.4byte	.LFB71
	.4byte	.LFE71-.LFB71
	.4byte	.LFB72
	.4byte	.LFE72-.LFB72
	.4byte	.LFB73
	.4byte	.LFE73-.LFB73
	.4byte	.LFB74
	.4byte	.LFE74-.LFB74
	.4byte	.LFB75
	.4byte	.LFE75-.LFB75
	.4byte	.LFB76
	.4byte	.LFE76-.LFB76
	.4byte	.LFB77
	.4byte	.LFE77-.LFB77
	.4byte	.LFB78
	.4byte	.LFE78-.LFB78
	.4byte	.LFB79
	.4byte	.LFE79-.LFB79
	.4byte	.LFB80
	.4byte	.LFE80-.LFB80
	.4byte	.LFB81
	.4byte	.LFE81-.LFB81
	.4byte	.LFB82
	.4byte	.LFE82-.LFB82
	.4byte	.LFB83
	.4byte	.LFE83-.LFB83
	.4byte	.LFB84
	.4byte	.LFE84-.LFB84
	.4byte	.LFB85
	.4byte	.LFE85-.LFB85
	.4byte	.LFB86
	.4byte	.LFE86-.LFB86
	.4byte	.LFB87
	.4byte	.LFE87-.LFB87
	.4byte	.LFB88
	.4byte	.LFE88-.LFB88
	.4byte	.LFB89
	.4byte	.LFE89-.LFB89
	.4byte	.LFB90
	.4byte	.LFE90-.LFB90
	.4byte	.LFB91
	.4byte	.LFE91-.LFB91
	.4byte	.LFB92
	.4byte	.LFE92-.LFB92
	.4byte	.LFB93
	.4byte	.LFE93-.LFB93
	.4byte	.LFB94
	.4byte	.LFE94-.LFB94
	.4byte	.LFB95
	.4byte	.LFE95-.LFB95
	.4byte	.LFB96
	.4byte	.LFE96-.LFB96
	.4byte	.LFB97
	.4byte	.LFE97-.LFB97
	.4byte	.LFB98
	.4byte	.LFE98-.LFB98
	.4byte	.LFB99
	.4byte	.LFE99-.LFB99
	.4byte	.LFB100
	.4byte	.LFE100-.LFB100
	.4byte	.LFB102
	.4byte	.LFE102-.LFB102
	.4byte	.LFB101
	.4byte	.LFE101-.LFB101
	.4byte	.LFB103
	.4byte	.LFE103-.LFB103
	.4byte	.LFB104
	.4byte	.LFE104-.LFB104
	.4byte	.LFB105
	.4byte	.LFE105-.LFB105
	.4byte	.LFB106
	.4byte	.LFE106-.LFB106
	.4byte	.LFB107
	.4byte	.LFE107-.LFB107
	.4byte	.LFB108
	.4byte	.LFE108-.LFB108
	.4byte	.LFB109
	.4byte	.LFE109-.LFB109
	.4byte	.LFB110
	.4byte	.LFE110-.LFB110
	.4byte	.LFB111
	.4byte	.LFE111-.LFB111
	.4byte	.LFB112
	.4byte	.LFE112-.LFB112
	.4byte	0x0
	.4byte	0x0
	.section	.debug_ranges,"",%progbits
.Ldebug_ranges0:
	.4byte	.LBB4
	.4byte	.LBE4
	.4byte	.LBB5
	.4byte	.LBE5
	.4byte	0x0
	.4byte	0x0
	.4byte	.LFB29
	.4byte	.LFE29
	.4byte	.LFB30
	.4byte	.LFE30
	.4byte	.LFB31
	.4byte	.LFE31
	.4byte	.LFB32
	.4byte	.LFE32
	.4byte	.LFB33
	.4byte	.LFE33
	.4byte	.LFB34
	.4byte	.LFE34
	.4byte	.LFB35
	.4byte	.LFE35
	.4byte	.LFB36
	.4byte	.LFE36
	.4byte	.LFB37
	.4byte	.LFE37
	.4byte	.LFB38
	.4byte	.LFE38
	.4byte	.LFB39
	.4byte	.LFE39
	.4byte	.LFB40
	.4byte	.LFE40
	.4byte	.LFB41
	.4byte	.LFE41
	.4byte	.LFB42
	.4byte	.LFE42
	.4byte	.LFB43
	.4byte	.LFE43
	.4byte	.LFB44
	.4byte	.LFE44
	.4byte	.LFB45
	.4byte	.LFE45
	.4byte	.LFB46
	.4byte	.LFE46
	.4byte	.LFB48
	.4byte	.LFE48
	.4byte	.LFB49
	.4byte	.LFE49
	.4byte	.LFB50
	.4byte	.LFE50
	.4byte	.LFB51
	.4byte	.LFE51
	.4byte	.LFB52
	.4byte	.LFE52
	.4byte	.LFB53
	.4byte	.LFE53
	.4byte	.LFB54
	.4byte	.LFE54
	.4byte	.LFB55
	.4byte	.LFE55
	.4byte	.LFB56
	.4byte	.LFE56
	.4byte	.LFB57
	.4byte	.LFE57
	.4byte	.LFB58
	.4byte	.LFE58
	.4byte	.LFB59
	.4byte	.LFE59
	.4byte	.LFB60
	.4byte	.LFE60
	.4byte	.LFB61
	.4byte	.LFE61
	.4byte	.LFB62
	.4byte	.LFE62
	.4byte	.LFB63
	.4byte	.LFE63
	.4byte	.LFB64
	.4byte	.LFE64
	.4byte	.LFB65
	.4byte	.LFE65
	.4byte	.LFB66
	.4byte	.LFE66
	.4byte	.LFB67
	.4byte	.LFE67
	.4byte	.LFB68
	.4byte	.LFE68
	.4byte	.LFB69
	.4byte	.LFE69
	.4byte	.LFB47
	.4byte	.LFE47
	.4byte	.LFB70
	.4byte	.LFE70
	.4byte	.LFB71
	.4byte	.LFE71
	.4byte	.LFB72
	.4byte	.LFE72
	.4byte	.LFB73
	.4byte	.LFE73
	.4byte	.LFB74
	.4byte	.LFE74
	.4byte	.LFB75
	.4byte	.LFE75
	.4byte	.LFB76
	.4byte	.LFE76
	.4byte	.LFB77
	.4byte	.LFE77
	.4byte	.LFB78
	.4byte	.LFE78
	.4byte	.LFB79
	.4byte	.LFE79
	.4byte	.LFB80
	.4byte	.LFE80
	.4byte	.LFB81
	.4byte	.LFE81
	.4byte	.LFB82
	.4byte	.LFE82
	.4byte	.LFB83
	.4byte	.LFE83
	.4byte	.LFB84
	.4byte	.LFE84
	.4byte	.LFB85
	.4byte	.LFE85
	.4byte	.LFB86
	.4byte	.LFE86
	.4byte	.LFB87
	.4byte	.LFE87
	.4byte	.LFB88
	.4byte	.LFE88
	.4byte	.LFB89
	.4byte	.LFE89
	.4byte	.LFB90
	.4byte	.LFE90
	.4byte	.LFB91
	.4byte	.LFE91
	.4byte	.LFB92
	.4byte	.LFE92
	.4byte	.LFB93
	.4byte	.LFE93
	.4byte	.LFB94
	.4byte	.LFE94
	.4byte	.LFB95
	.4byte	.LFE95
	.4byte	.LFB96
	.4byte	.LFE96
	.4byte	.LFB97
	.4byte	.LFE97
	.4byte	.LFB98
	.4byte	.LFE98
	.4byte	.LFB99
	.4byte	.LFE99
	.4byte	.LFB100
	.4byte	.LFE100
	.4byte	.LFB102
	.4byte	.LFE102
	.4byte	.LFB101
	.4byte	.LFE101
	.4byte	.LFB103
	.4byte	.LFE103
	.4byte	.LFB104
	.4byte	.LFE104
	.4byte	.LFB105
	.4byte	.LFE105
	.4byte	.LFB106
	.4byte	.LFE106
	.4byte	.LFB107
	.4byte	.LFE107
	.4byte	.LFB108
	.4byte	.LFE108
	.4byte	.LFB109
	.4byte	.LFE109
	.4byte	.LFB110
	.4byte	.LFE110
	.4byte	.LFB111
	.4byte	.LFE111
	.4byte	.LFB112
	.4byte	.LFE112
	.4byte	0x0
	.4byte	0x0
	.section	.debug_str,"MS",%progbits,1
.LASF85:
	.ascii	"RSERVED1\000"
.LASF181:
	.ascii	"RESERVED20\000"
.LASF23:
	.ascii	"RTC_WKUP_IRQn\000"
.LASF28:
	.ascii	"EXTI2_IRQn\000"
.LASF198:
	.ascii	"TASK_beacon\000"
.LASF213:
	.ascii	"BOARD_DMA_CLKON\000"
.LASF148:
	.ascii	"EXTICR\000"
.LASF203:
	.ascii	"start\000"
.LASF139:
	.ascii	"OTYPER\000"
.LASF322:
	.ascii	"platform_block\000"
.LASF131:
	.ascii	"PRGKEYR\000"
.LASF35:
	.ascii	"DMA1_Channel5_IRQn\000"
.LASF257:
	.ascii	"return_from_task\000"
.LASF45:
	.ascii	"TIM9_IRQn\000"
.LASF16:
	.ascii	"SVC_IRQn\000"
.LASF208:
	.ascii	"write\000"
.LASF231:
	.ascii	"OT_GPTIM_ISR\000"
.LASF291:
	.ascii	"alarm_id\000"
.LASF97:
	.ascii	"VTOR\000"
.LASF67:
	.ascii	"SPI3_IRQn\000"
.LASF24:
	.ascii	"FLASH_IRQn\000"
.LASF151:
	.ascii	"ICSCR\000"
.LASF260:
	.ascii	"platform_init_gptim\000"
.LASF235:
	.ascii	"hwid\000"
.LASF130:
	.ascii	"PEKEYR\000"
.LASF215:
	.ascii	"BOARD_EXTI_STARTUP\000"
.LASF118:
	.ascii	"CNDTR\000"
.LASF48:
	.ascii	"TIM2_IRQn\000"
.LASF201:
	.ascii	"vaddr\000"
.LASF179:
	.ascii	"DMAR\000"
.LASF88:
	.ascii	"ICPR\000"
.LASF125:
	.ascii	"FTSR\000"
.LASF321:
	.ascii	"count\000"
.LASF332:
	.ascii	"BOARD_POWER_STARTUP\000"
.LASF185:
	.ascii	"last_evt\000"
.LASF173:
	.ascii	"CCR1\000"
.LASF174:
	.ascii	"CCR2\000"
.LASF175:
	.ascii	"CCR3\000"
.LASF176:
	.ascii	"CCR4\000"
.LASF56:
	.ascii	"SPI2_IRQn\000"
.LASF285:
	.ascii	"platform_enable_rtc\000"
.LASF98:
	.ascii	"AIRCR\000"
.LASF147:
	.ascii	"MEMRMP\000"
.LASF212:
	.ascii	"BOARD_PERIPH_INIT\000"
.LASF298:
	.ascii	"platform_trig2_high\000"
.LASF199:
	.ascii	"TASK_external\000"
.LASF307:
	.ascii	"timer_value\000"
.LASF8:
	.ascii	"long int\000"
.LASF225:
	.ascii	"PendSV_Handler\000"
.LASF209:
	.ascii	"vlFILE\000"
.LASF95:
	.ascii	"CPUID\000"
.LASF280:
	.ascii	"platform_set_watchdog\000"
.LASF269:
	.ascii	"value\000"
.LASF262:
	.ascii	"platform_poweron\000"
.LASF252:
	.ascii	"platform_switch_context\000"
.LASF331:
	.ascii	"IRQn\000"
.LASF124:
	.ascii	"RTSR\000"
.LASF127:
	.ascii	"EXTI_TypeDef\000"
.LASF3:
	.ascii	"ot_int\000"
.LASF157:
	.ascii	"APB2ENR\000"
.LASF194:
	.ascii	"TASK_radio\000"
.LASF305:
	.ascii	"seed\000"
.LASF19:
	.ascii	"SysTick_IRQn\000"
.LASF133:
	.ascii	"WRPR\000"
.LASF59:
	.ascii	"USART3_IRQn\000"
.LASF119:
	.ascii	"CPAR\000"
.LASF284:
	.ascii	"platform_resume_watchdog\000"
.LASF297:
	.ascii	"OT_TRIG1_TOG\000"
.LASF96:
	.ascii	"ICSR\000"
.LASF0:
	.ascii	"signed char\000"
.LASF77:
	.ascii	"uint8_t\000"
.LASF317:
	.ascii	"platform_memcpy_2\000"
.LASF327:
	.ascii	"platform_ext\000"
.LASF182:
	.ascii	"TIM_TypeDef\000"
.LASF83:
	.ascii	"RESERVED0\000"
.LASF142:
	.ascii	"RESERVED1\000"
.LASF1:
	.ascii	"unsigned char\000"
.LASF89:
	.ascii	"RESERVED3\000"
.LASF91:
	.ascii	"RESERVED4\000"
.LASF92:
	.ascii	"RESERVED5\000"
.LASF166:
	.ascii	"RESERVED6\000"
.LASF168:
	.ascii	"RESERVED7\000"
.LASF170:
	.ascii	"RESERVED8\000"
.LASF123:
	.ascii	"DMA_TypeDef\000"
.LASF312:
	.ascii	"len_div\000"
.LASF90:
	.ascii	"IABR\000"
.LASF52:
	.ascii	"I2C1_ER_IRQn\000"
.LASF310:
	.ascii	"sub_memcpy_dma\000"
.LASF75:
	.ascii	"AES_IRQn\000"
.LASF202:
	.ascii	"header\000"
.LASF31:
	.ascii	"DMA1_Channel1_IRQn\000"
.LASF22:
	.ascii	"TAMPER_STAMP_IRQn\000"
.LASF87:
	.ascii	"RESERVED2\000"
.LASF66:
	.ascii	"TIM5_IRQn\000"
.LASF15:
	.ascii	"UsageFault_IRQn\000"
.LASF165:
	.ascii	"CCMR1\000"
.LASF167:
	.ascii	"CCMR2\000"
.LASF279:
	.ascii	"platform_get_gptim\000"
.LASF108:
	.ascii	"SCB_Type\000"
.LASF102:
	.ascii	"DFSR\000"
.LASF236:
	.ascii	"sub_msflash_config\000"
.LASF246:
	.ascii	"platform_enable_interrupts\000"
.LASF296:
	.ascii	"platform_trig1_toggle\000"
.LASF101:
	.ascii	"HFSR\000"
.LASF192:
	.ascii	"platform_struct\000"
.LASF299:
	.ascii	"platform_trig2_low\000"
.LASF278:
	.ascii	"platform_set_gptim2\000"
.LASF239:
	.ascii	"sub_hsosc_config\000"
.LASF186:
	.ascii	"next_evt\000"
.LASF115:
	.ascii	"APB1FZ\000"
.LASF255:
	.ascii	"platform_ot_preempt\000"
.LASF50:
	.ascii	"TIM4_IRQn\000"
.LASF62:
	.ascii	"USB_FS_WKUP_IRQn\000"
.LASF58:
	.ascii	"USART2_IRQn\000"
.LASF245:
	.ascii	"__enable_irq\000"
.LASF288:
	.ascii	"utc_time\000"
.LASF330:
	.ascii	"B:\\otplatform\\stm32l1xx\\platform_STM32L1xx.c\000"
.LASF99:
	.ascii	"SHCSR\000"
.LASF287:
	.ascii	"platform_set_time\000"
.LASF135:
	.ascii	"WRPR1\000"
.LASF136:
	.ascii	"WRPR2\000"
.LASF93:
	.ascii	"STIR\000"
.LASF51:
	.ascii	"I2C1_EV_IRQn\000"
.LASF227:
	.ascii	"CLR_PENDSV\000"
.LASF324:
	.ascii	"platform_swdelay_ms\000"
.LASF134:
	.ascii	"RESERVED\000"
.LASF117:
	.ascii	"DBGMCU_TypeDef\000"
.LASF274:
	.ascii	"platform_disable_ktim\000"
.LASF152:
	.ascii	"CFGR\000"
.LASF71:
	.ascii	"DMA2_Channel2_IRQn\000"
.LASF289:
	.ascii	"platform_get_time\000"
.LASF200:
	.ascii	"TASK_terminus\000"
.LASF27:
	.ascii	"EXTI1_IRQn\000"
.LASF250:
	.ascii	"platform_load_context\000"
.LASF189:
	.ascii	"crc16\000"
.LASF275:
	.ascii	"platform_pend_ktim\000"
.LASF132:
	.ascii	"OPTKEYR\000"
.LASF12:
	.ascii	"NonMaskableInt_IRQn\000"
.LASF314:
	.ascii	"sub_memcpy2_dma\000"
.LASF20:
	.ascii	"WWDG_IRQn\000"
.LASF261:
	.ascii	"prescaler\000"
.LASF9:
	.ascii	"long unsigned int\000"
.LASF158:
	.ascii	"APB1ENR\000"
.LASF247:
	.ascii	"platform_standard_speed\000"
.LASF13:
	.ascii	"MemoryManagement_IRQn\000"
.LASF34:
	.ascii	"DMA1_Channel4_IRQn\000"
.LASF164:
	.ascii	"DIER\000"
.LASF318:
	.ascii	"platform_memset\000"
.LASF292:
	.ascii	"task_id\000"
.LASF70:
	.ascii	"DMA2_Channel1_IRQn\000"
.LASF40:
	.ascii	"USB_LP_IRQn\000"
.LASF145:
	.ascii	"LCKR\000"
.LASF238:
	.ascii	"sub_voltage_config\000"
.LASF249:
	.ascii	"platform_flank_speed\000"
.LASF295:
	.ascii	"platform_trig1_low\000"
.LASF54:
	.ascii	"I2C2_ER_IRQn\000"
.LASF244:
	.ascii	"__disable_irq\000"
.LASF153:
	.ascii	"AHBRSTR\000"
.LASF184:
	.ascii	"task_exit\000"
.LASF205:
	.ascii	"idmod\000"
.LASF263:
	.ascii	"platform_init_perihclk\000"
.LASF140:
	.ascii	"OSPEEDR\000"
.LASF14:
	.ascii	"BusFault_IRQn\000"
.LASF69:
	.ascii	"UART5_IRQn\000"
.LASF217:
	.ascii	"BOARD_XTAL_STARTUP\000"
.LASF41:
	.ascii	"DAC_IRQn\000"
.LASF64:
	.ascii	"TIM7_IRQn\000"
.LASF80:
	.ascii	"long long int\000"
.LASF128:
	.ascii	"PECR\000"
.LASF241:
	.ascii	"counter\000"
.LASF141:
	.ascii	"PUPDR\000"
.LASF216:
	.ascii	"BOARD_PORT_STARTUP\000"
.LASF55:
	.ascii	"SPI1_IRQn\000"
.LASF323:
	.ascii	"platform_delay\000"
.LASF39:
	.ascii	"USB_HP_IRQn\000"
.LASF116:
	.ascii	"APB2FZ\000"
.LASF154:
	.ascii	"APB2RSTR\000"
.LASF144:
	.ascii	"BSRRH\000"
.LASF259:
	.ascii	"platform_init_gpio\000"
.LASF143:
	.ascii	"BSRRL\000"
.LASF313:
	.ascii	"align\000"
.LASF110:
	.ascii	"LOAD\000"
.LASF190:
	.ascii	"platform_ext_struct\000"
.LASF68:
	.ascii	"UART4_IRQn\000"
.LASF160:
	.ascii	"APB2LPENR\000"
.LASF268:
	.ascii	"platform_init_rtc\000"
.LASF254:
	.ascii	"task_lr\000"
.LASF114:
	.ascii	"IDCODE\000"
.LASF74:
	.ascii	"DMA2_Channel5_IRQn\000"
.LASF138:
	.ascii	"MODER\000"
.LASF76:
	.ascii	"COMP_ACQ_IRQn\000"
.LASF240:
	.ascii	"platform_init_busclk\000"
.LASF207:
	.ascii	"read\000"
.LASF11:
	.ascii	"unsigned int\000"
.LASF109:
	.ascii	"CTRL\000"
.LASF7:
	.ascii	"ot_u16\000"
.LASF316:
	.ascii	"platform_memcpy\000"
.LASF226:
	.ascii	"SET_PENDSV\000"
.LASF53:
	.ascii	"I2C2_EV_IRQn\000"
.LASF243:
	.ascii	"platform_disable_interrupts\000"
.LASF256:
	.ascii	"platform_ot_run\000"
.LASF309:
	.ascii	"platform_prand_u8\000"
.LASF120:
	.ascii	"CMAR\000"
.LASF37:
	.ascii	"DMA1_Channel7_IRQn\000"
.LASF328:
	.ascii	"platform\000"
.LASF218:
	.ascii	"HardFault_Handler\000"
.LASF104:
	.ascii	"BFAR\000"
.LASF293:
	.ascii	"offset\000"
.LASF161:
	.ascii	"APB1LPENR\000"
.LASF73:
	.ascii	"DMA2_Channel4_IRQn\000"
.LASF265:
	.ascii	"platform_init_resetswitch\000"
.LASF44:
	.ascii	"LCD_IRQn\000"
.LASF325:
	.ascii	"platform_swdelay_us\000"
.LASF276:
	.ascii	"platform_flush_ktim\000"
.LASF10:
	.ascii	"ot_u32\000"
.LASF112:
	.ascii	"SysTick_Type\000"
.LASF258:
	.ascii	"platform_init_interruptor\000"
.LASF193:
	.ascii	"TASK_idle\000"
.LASF211:
	.ascii	"PriorityGroupTmp\000"
.LASF229:
	.ascii	"BOARD_PORT_STANDBY\000"
.LASF286:
	.ascii	"platform_disable_rtc\000"
.LASF248:
	.ascii	"platform_full_speed\000"
.LASF234:
	.ascii	"fpid\000"
.LASF267:
	.ascii	"period\000"
.LASF253:
	.ascii	"platform_drop_context\000"
.LASF197:
	.ascii	"TASK_sleep\000"
.LASF230:
	.ascii	"platform_poweroff\000"
.LASF126:
	.ascii	"SWIER\000"
.LASF121:
	.ascii	"DMA_Channel_TypeDef\000"
.LASF210:
	.ascii	"reg_value\000"
.LASF61:
	.ascii	"RTC_Alarm_IRQn\000"
.LASF224:
	.ascii	"NMI_Handler\000"
.LASF94:
	.ascii	"NVIC_Type\000"
.LASF308:
	.ascii	"platform_crc_block\000"
.LASF329:
	.ascii	"GNU C 4.5.2\000"
.LASF81:
	.ascii	"long long unsigned int\000"
.LASF206:
	.ascii	"length\000"
.LASF78:
	.ascii	"uint16_t\000"
.LASF319:
	.ascii	"__WFE\000"
.LASF60:
	.ascii	"EXTI15_10_IRQn\000"
.LASF49:
	.ascii	"TIM3_IRQn\000"
.LASF169:
	.ascii	"CCER\000"
.LASF172:
	.ascii	"RESERVED12\000"
.LASF57:
	.ascii	"USART1_IRQn\000"
.LASF177:
	.ascii	"RESERVED17\000"
.LASF178:
	.ascii	"RESERVED18\000"
.LASF180:
	.ascii	"RESERVED19\000"
.LASF281:
	.ascii	"timeout_ticks\000"
.LASF232:
	.ascii	"platform_init_OT\000"
.LASF294:
	.ascii	"platform_trig1_high\000"
.LASF159:
	.ascii	"AHBLPENR\000"
.LASF84:
	.ascii	"ICER\000"
.LASF163:
	.ascii	"SMCR\000"
.LASF191:
	.ascii	"error_code\000"
.LASF47:
	.ascii	"TIM11_IRQn\000"
.LASF237:
	.ascii	"sub_hsflash_config\000"
.LASF113:
	.ascii	"RESET\000"
.LASF183:
	.ascii	"sstack\000"
.LASF21:
	.ascii	"PVD_IRQn\000"
.LASF311:
	.ascii	"dest\000"
.LASF105:
	.ascii	"AFSR\000"
.LASF129:
	.ascii	"PDKEYR\000"
.LASF111:
	.ascii	"CALIB\000"
.LASF251:
	.ascii	"scratch\000"
.LASF233:
	.ascii	"code\000"
.LASF26:
	.ascii	"EXTI0_IRQn\000"
.LASF195:
	.ascii	"TASK_mpipe\000"
.LASF100:
	.ascii	"CFSR\000"
.LASF17:
	.ascii	"DebugMonitor_IRQn\000"
.LASF301:
	.ascii	"platform_rand\000"
.LASF214:
	.ascii	"BOARD_DMA_CLKOFF\000"
.LASF266:
	.ascii	"platform_init_systick\000"
.LASF46:
	.ascii	"TIM10_IRQn\000"
.LASF33:
	.ascii	"DMA1_Channel3_IRQn\000"
.LASF106:
	.ascii	"MMFR\000"
.LASF162:
	.ascii	"RCC_TypeDef\000"
.LASF103:
	.ascii	"MMFAR\000"
.LASF4:
	.ascii	"short int\000"
.LASF122:
	.ascii	"IFCR\000"
.LASF25:
	.ascii	"RCC_IRQn\000"
.LASF187:
	.ascii	"cpu_khz\000"
.LASF155:
	.ascii	"APB1RSTR\000"
.LASF107:
	.ascii	"ISAR\000"
.LASF264:
	.ascii	"platform_init_watchdog\000"
.LASF156:
	.ascii	"AHBENR\000"
.LASF188:
	.ascii	"prand_reg\000"
.LASF5:
	.ascii	"ot_uint\000"
.LASF42:
	.ascii	"COMP_IRQn\000"
.LASF282:
	.ascii	"platform_kill_watchdog\000"
.LASF146:
	.ascii	"GPIO_TypeDef\000"
.LASF326:
	.ascii	"ITM_RxBuffer\000"
.LASF221:
	.ascii	"UsageFault_Handler\000"
.LASF32:
	.ascii	"DMA1_Channel2_IRQn\000"
.LASF219:
	.ascii	"MemManage_Handler\000"
.LASF335:
	.ascii	"platform_save_context\000"
.LASF228:
	.ascii	"WWDG_IRQHandler\000"
.LASF63:
	.ascii	"TIM6_IRQn\000"
.LASF270:
	.ascii	"platform_init_memcpy\000"
.LASF137:
	.ascii	"FLASH_TypeDef\000"
.LASF277:
	.ascii	"platform_set_ktim\000"
.LASF303:
	.ascii	"bytes_out\000"
.LASF79:
	.ascii	"uint32_t\000"
.LASF334:
	.ascii	"PriorityGroup\000"
.LASF150:
	.ascii	"PWR_TypeDef\000"
.LASF300:
	.ascii	"platform_trig2_toggle\000"
.LASF43:
	.ascii	"EXTI9_5_IRQn\000"
.LASF304:
	.ascii	"platform_init_prand\000"
.LASF196:
	.ascii	"TASK_hold\000"
.LASF30:
	.ascii	"EXTI4_IRQn\000"
.LASF272:
	.ascii	"platform_next_ktim\000"
.LASF6:
	.ascii	"short unsigned int\000"
.LASF65:
	.ascii	"SDIO_IRQn\000"
.LASF171:
	.ascii	"RESERVED10\000"
.LASF86:
	.ascii	"ISPR\000"
.LASF29:
	.ascii	"EXTI3_IRQn\000"
.LASF18:
	.ascii	"PendSV_IRQn\000"
.LASF38:
	.ascii	"ADC1_IRQn\000"
.LASF223:
	.ascii	"SVC_Handler\000"
.LASF290:
	.ascii	"platform_set_rtc_alarm\000"
.LASF320:
	.ascii	"sub_timed_wfe\000"
.LASF333:
	.ascii	"NVIC_SetPriorityGrouping\000"
.LASF242:
	.ascii	"platform_init_periphclk\000"
.LASF283:
	.ascii	"platform_pause_watchdog\000"
.LASF273:
	.ascii	"platform_enable_ktim\000"
.LASF220:
	.ascii	"BusFault_Handler\000"
.LASF82:
	.ascii	"ISER\000"
.LASF271:
	.ascii	"platform_get_ktim\000"
.LASF315:
	.ascii	"ccr_val\000"
.LASF306:
	.ascii	"platform_prand_u16\000"
.LASF204:
	.ascii	"alloc\000"
.LASF36:
	.ascii	"DMA1_Channel6_IRQn\000"
.LASF2:
	.ascii	"ot_u8\000"
.LASF149:
	.ascii	"SYSCFG_TypeDef\000"
.LASF336:
	.ascii	"RETURN_FROM_TASK\000"
.LASF222:
	.ascii	"DebugMon_Handler\000"
.LASF302:
	.ascii	"rand_out\000"
.LASF72:
	.ascii	"DMA2_Channel3_IRQn\000"
	.ident	"GCC: (Sourcery G++ Lite 2011.03-42) 4.5.2"
