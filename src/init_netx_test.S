	.section .init_code, "ax"
	.arm

	.extern show_cfg_main
	.extern stack_top

	.global start

@--------------------------------------

  .func start

start:
	stmfd   sp!, {r0, r1, r2, lr}

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

	b       show_cfg_main

  .endfunc

@--------------------------------------

  .end

