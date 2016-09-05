	.section .init_code, "ax"
	.arm

	.extern show_cfg_main
	.extern stack_top

	.global start

@--------------------------------------

    .func start, start
    .type start, STT_FUNC
start:
	stmfd   sp!, {r0-r12, lr}

	@ save romcode stack position
	mov     r0, sp
	ldr     r1, =sp_save
	str     r0, [r1]
	
	@ Set own stack position.
	ldr     r0, =stack_top
	mov     sp, r0

	@ Clear the .bss section (zero init).
	mov     r0, #0
	ldr     r1, =__bss_start__
	ldr     r2, =__bss_end__
clear_bss:
	cmp     r1, r2
	strlo   r0, [r1], #4
	blo     clear_bss

	blx     show_cfg_main
	
	@ restore romcode stack position
	ldr     r1, =sp_save
	ldr     r0, [r1]
	mov     sp, r0

	ldmfd   sp!, {r0-r12, lr}
	bx      lr
	
  .endfunc
	
@--------------------------------------
	.align 4
sp_save: 
	.word 0
  .end

