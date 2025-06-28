;********************************************************************************************************
;                                               uC/OS-II
;                                         The Real-Time Kernel
;
;                          (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
;                                          All Rights Reserved
;
; File : OS_CPU_A.ASM
; By   : Jean J. Labrosse
;********************************************************************************************************

;********************************************************************************************************
;                                               ARM Port
;
;                 Target           : ARM (Includes ARM7, ARM9)
;                 Ported by        : Michael Anburaj
;                 URL              : http://geocities.com/michaelanburaj/    Email : michaelanburaj@hotmail.com
;
;********************************************************************************************************

	PRESERVE8	; current file preserves eight-byte alignment of the stack

	INCLUDE	..\..\inc\sysopt.inc

;Pre-defined constants
USERMODE    EQU         0x10
FIQMODE     EQU         0x11
IRQMODE     EQU         0x12
SVCMODE     EQU         0x13
ABORTMODE   EQU         0x17
UNDEFMODE   EQU         0x1b
MODEMASK    EQU         0x1f
NOINT       EQU         0xc0
TBIT        EQU         0x20

; Fix K310 IRQ/FIQ bug
RegCtrlBase EQU         0xd0000000
RegFiqMask  EQU         0xd0000004
RegIrqMask  EQU         0xd0000010
RegIntMask  EQU         0xd0000014
SdramTimeCtrl EQU       0xc0040038

    IF (CHIP_OPT < CHIP_OPT_A1016A)
FIXK310BUG  EQU         0x00000001
SPEEDUPIRQFIQ EQU       0x00000001
    ENDIF
    
    IF ((CHIP_OPT == CHIP_OPT_A1013A) || (CHIP_OPT == CHIP_OPT_A1013A_FPGA))
FIXK310BUG  EQU         0x00000000
SPEEDUPIRQFIQ EQU       0x00000000
    ENDIF

    IF ((CHIP_OPT == CHIP_OPT_A1013B) || (CHIP_OPT == CHIP_OPT_A1013B_FPGA) || (CHIP_OPT == CHIP_OPT_A1016A) || (CHIP_OPT == CHIP_OPT_A1016A_FPGA) || (CHIP_OPT == CHIP_OPT_A1018A) || (CHIP_OPT == CHIP_OPT_A1018A_FPGA) || (CHIP_OPT == CHIP_OPT_A1019A))
FIXK310BUG  EQU         0x00000002
SPEEDUPIRQFIQ EQU       0x00000001
    ENDIF
    IF ((CHIP_OPT == CHIP_OPT_A1020A) || (CHIP_OPT == CHIP_OPT_A1021A) || (CHIP_OPT == CHIP_OPT_A1022A))
FIXK310BUG  EQU         0x00000002
SPEEDUPIRQFIQ EQU       0x00000001
    ENDIF

        AREA  UCOS_ARM, CODE, READONLY

;*********************************************************************************************************
;                                          START MULTITASKING
;                                       void OSStartHighRdy(void)
;
; Note : OSStartHighRdy() MUST:
;           a) Call OSTaskSwHook() then,
;           b) Set OSRunning to TRUE,
;           c) Switch to the highest priority task.
;*********************************************************************************************************

        IMPORT  OSTaskSwHook
        IMPORT  OSRunning
        IMPORT  OSTCBHighRdy

        EXPORT  OSStartHighRdy

OSStartHighRdy  

        bl OSTaskSwHook             ; Call user defined task switch hook

        ldr r4,=OSRunning           ; Indicate that multitasking has started
        mov r5,#1
        strb r5,[r4]

        ldr r4,=OSTCBHighRdy        ; Get highest priority task TCB address

        ldr r4,[r4]                 ; get stack pointer
        ldr sp,[r4]                 ; switch to the new stack

        ldmfd sp!,{r4}              ; pop new task's spsr
        ldmfd sp!,{r4}              ; pop new task's psr
        msr SPSR_cxsf,r4
        ldmfd sp!,{r0-r12,lr,pc}^   ; pop new task's r0-r12,lr & pc

;*********************************************************************************************************
;                                PERFORM A CONTEXT SWITCH (From task level)
;                                           void OSCtxSw(void)
;
; Note(s):    Upon entry, 
;             OSTCBCur     points to the OS_TCB of the task to suspend
;             OSTCBHighRdy points to the OS_TCB of the task to resume
;
;*********************************************************************************************************

        IMPORT  OSTCBCur
        IMPORT  OSTaskSwHook
        IMPORT  OSTCBHighRdy
        IMPORT  OSPrioCur
        IMPORT  OSPrioHighRdy

        EXPORT  OSCtxSw

OSCtxSw
; Special optimised code below:
        stmfd sp!,{lr}              ; push pc (lr should be pushed in place of PC)
        ldr lr,=0xdead1eaf          ; Just a dummy value is pushed for lr (for actual lr is safe inside callers stack)
        stmfd sp!,{r0-r12,lr}       ; push lr & register file
        mrs r4,cpsr
;    [ THUMB_TASKS
;        orr r4,r4,#TBIT
;    ]
        stmfd sp!,{r4}              ; push current psr
        mrs r4,spsr
        stmfd sp!,{r4}              ; push current spsr

        ; OSPrioCur = OSPrioHighRdy
        ldr r4,=OSPrioCur
        ldr r5,=OSPrioHighRdy
        ldrb r6,[r5]
        strb r6,[r4]
        
        ; Get current task TCB address
        ldr r4,=OSTCBCur
        ldr r5,[r4]
        str sp,[r5]                 ; store sp in preempted tasks's TCB

        bl OSTaskSwHook             ; call Task Switch Hook

        ; Get highest priority task TCB address
        ldr r6,=OSTCBHighRdy
        ldr r6,[r6]
        ldr sp,[r6]                 ; get new task's stack pointer

        ; OSTCBCur = OSTCBHighRdy
        str r6,[r4]                 ; set new current task TCB address

        ldmfd sp!,{r4}              ; pop new task's spsr
        ldmfd sp!,{r4}              ; pop new task's psr
        msr SPSR_cxsf,r4

        ldmfd sp!,{r0-r12,lr,pc}^   ; pop new task's r0-r12,lr & pc


;*********************************************************************************************************
;                                PERFORM A CONTEXT SWITCH (From an ISR)
;                                        void OSIntIRQCtxSw(void)
;
; Note(s): This function only flags a context switch to the ISR Handler
;
;*********************************************************************************************************

        IMPORT  OSIntIRQCtxSwFlag

        EXPORT  OSIntIRQCtxSw

OSIntIRQCtxSw

        ;OSIntIRQCtxSwFlag = True
        ldr r0,=OSIntIRQCtxSwFlag
        mov r1,#1
        str r1,[r0]
        bx lr
        
;*********************************************************************************************************
;                                PERFORM A CONTEXT SWITCH (From an ISR)
;                                        void OSIntFIQCtxSw(void)
;
; Note(s): This function only flags a context switch to the ISR Handler
;
;*********************************************************************************************************
        
        EXPORT  OSIntFIQCtxSw

        IMPORT  OSIntFIQCtxSwFlag
        
OSIntFIQCtxSw

        ;OSIntFIQCtxSwFlag = True
        ldr r0,=OSIntFIQCtxSwFlag
        mov r1,#1
        str r1,[r0]
        bx lr
        
;*********************************************************************************************************
;                                CHECK IF AN FIQ INTERRUPT AN IRQ (From an ISR)
;                                        void OSIntFIQNestIRQ(void)
;
; Note(s): This function only flags an event of fiq interrupt irq
;
;*********************************************************************************************************
        
        EXPORT  OSIntFIQNestIRQ

        IMPORT  OSIntFIQNestIRQFlag
        
OSIntFIQNestIRQ

        ;set OSIntFIQNestIRQ
        mov r1,#0
        mrs r0,spsr
        and r0,r0,#MODEMASK
        cmp r0,#IRQMODE
        bne OSIntFIQNestIRQ_1
        nop
        mov r1,#1
OSIntFIQNestIRQ_1        
        ldr r0,=OSIntFIQNestIRQFlag
        str r1,[r0]
        bx lr        
        
;*********************************************************************************************************
;                                            IRQ HANDLER
;
;        This handles all the IRQs
;
;*********************************************************************************************************
	
        ;//IMPORT  IRQ_Handler             ; In int_handler.c
        IMPORT  OSIntIRQEnter
        IMPORT  OSIntIRQExit

        IMPORT  OSIntIRQCtxSwFlag
        IMPORT  OSTCBCur
        IMPORT  OSTaskSwHook
        IMPORT  OSTCBHighRdy
        IMPORT  OSPrioCur
        IMPORT  OSPrioHighRdy
	
	    IMPORT  marsIntIRQHandler
	
        EXPORT  UCOS_IRQHandler
UCOS_IRQHandler

        ; Fix the return address
        sub lr,lr,#4
        stmfd sp!,{r0-r3,r12,lr}

        mov r0,#0xd0000000
	ldr r2,[r0,#0x14]
	tst r2,#2
	BNE	_IRQ_FAIL_EXIT

    ; Fix K310 IRQ/FIQ bug, enable all IRQ/FIQ mask
    IF (FIXK310BUG == 1)
        ldr r0, =RegCtrlBase
        ldr r1, [r0, #0x10]
        ;ldr r2, =0x000fffff
        ;teq r1, r2
        ;beq _IRQExit
        ldr r2, =IrqMaskBackupI
        str r1, [r2]
        ldr r1, [r0, #0x04]
        ldr r2, =FiqMaskBackupI
        str r1, [r2]
        mov r1, #0xffffffff
        str r1, [r0, #0x04]
        str r1, [r0, #0x10]
    ELIF (FIXK310BUG == 2)
    	ldr r0, =RegIntMask
    	mov r1, #0x00000003
    	str r1, [r0]
    ENDIF

        
        ;chanel piority: 5>1>4>2>3
       IF (SPEEDUPIRQFIQ == 1)
        ldr r0, =SdramTimeCtrl
        ldr r1, [r0]
        bic r1, r1, #0x00c00000
        orr r1, r1, #0x00800000
        str r1, [r0]
       ENDIF

        ; cytsai: 0421
        mrs r0,cpsr
        orr r0,r0,#NOINT
        msr cpsr_c,r0
        
    ; Fix K310 IRQ/FIQ bug, recover IRQ/FIQ mask
    IF (FIXK310BUG == 1)
        ldr r0, =RegCtrlBase
        ldr r2, FiqMaskBackupI
        str r2, [r0, #0x04]
        ldr r2, IrqMaskBackupI
        str r2, [r0, #0x10]
    ELIF (FIXK310BUG == 2)
    	ldr r0, =RegIntMask
    	mov r1, #0x00000000
    	str r1, [r0]
    ENDIF

        bl OSIntIRQEnter
        bl marsIntIRQHandler     ;// IRQ_Handler
        bl OSIntIRQExit

        ; Is OSIntIRQCtxSwFlag == True ?
        ldr r0,=OSIntIRQCtxSwFlag
        ldr r1,[r0]
        cmp r1,#1
        beq _IntIRQCtxSw
_IRQExit
    ; chanel piority: 5>1>2>3>4
       IF (SPEEDUPIRQFIQ == 1)
        ldr r0, =SdramTimeCtrl
        ldr r1, [r0]
        bic r1, r1, #0x00c00000
        str r1, [r0]
       ENDIF
_IRQ_FAIL_EXIT       
        ldmfd sp!,{r0-r3,r12,pc}^


_IntIRQCtxSw
        ; OSIntIRQCtxSwFlag = False
        mov r1,#0
        str r1,[r0]

    ; Fix K310 IRQ/FIQ bug, enable all IRQ/FIQ mask
    IF (FIXK310BUG == 1)
        ldr r0, =RegCtrlBase
        ldr r1, [r0, #0x04]
        ldr r2, =FiqMaskBackupI
        str r1, [r2]
        ldr r1, [r0, #0x10]
        ldr r2, =IrqMaskBackupI
        str r1, [r2]
        mov r1, #0xffffffff
        str r1, [r0, #0x04]
        str r1, [r0, #0x10]
    ELIF (FIXK310BUG == 2)
    	ldr r0, =RegIntMask
    	mov r1, #0x00000003
    	str r1, [r0]
    ENDIF

        ; Get old task's sp & lr
        mrs r0,spsr
        orr r0,r0,#NOINT
        bic r0,r0,#TBIT
        msr cpsr_c,r0
        mov r1,sp
        mov r2,lr
        bic r0,r0,#MODEMASK
        orr r0,r0,#IRQMODE
        msr cpsr_c,r0

    ; Fix K310 IRQ/FIQ bug, recover IRQ/FIQ mask
    IF (FIXK310BUG == 1)
        ldr r0, =RegCtrlBase
        ldr r3, FiqMaskBackupI
        str r3, [r0, #0x04]
        ldr r3, IrqMaskBackupI
        str r3, [r0, #0x10]
    ELIF (FIXK310BUG == 2)
    	ldr r0, =RegIntMask
    	mov r3, #0x00000000
    	str r3, [r0]
    ENDIF

        ; Unwind IRQ's sp without poping r0-r3
        mov r3,sp
        add sp,sp,#(4*4)
        ldmfd sp!,{r12,lr}

        stmfd r1!,{r2,lr}           ; push old task's pc & lr
        stmfd r1!,{r4-r12}          ; push old task's r12-r4

        ; Get r0-r3 from IRQ's stack
        mov r6,r1
        ldmfd r3,{r0-r3}
        stmfd r6!,{r0-r3}           ; push old task's r3-r0
        
        mrs r3,spsr
        stmfd r6!,{r3}              ; push old task's psr
        stmfd r6!,{r3}              ; push old task's spsr (don't care)
        
        ; OSPrioCur = OSPrioHighRdy
        ldr r4,=OSPrioCur
        ldr r5,=OSPrioHighRdy
        ldrb r5,[r5]
        strb r5,[r4]
        
        ; Get current task's TCB address
        ldr r4,=OSTCBCur
        ldr r5,[r4]
        str r6,[r5]                 ; store sp in preempted tasks's TCB

        bl OSTaskSwHook             ; call Task Switch Hook

        ; Get highest priority task's TCB address
        ldr r6,=OSTCBHighRdy
        ldr r6,[r6]
        ldr r5,[r6]                 ; get new task's stack pointer

        ; OSTCBCur = OSTCBHighRdy
        str r6,[r4]                 ; set new current task TCB address

        ldmfd r5!,{r4}              ; pop new task's spsr (don't care)
        ldmfd r5!,{r4}              ; pop new task's psr

    ; Fix K310 IRQ/FIQ bug, enable all IRQ/FIQ mask
    IF (FIXK310BUG == 1)
        ldr r0, =RegCtrlBase
        ldr r1, [r0, #0x04]
        ldr r2, =FiqMaskBackupI
        str r1, [r2]
        ldr r1, [r0, #0x10]
        ldr r2, =IrqMaskBackupI
        str r1, [r2]
        mov r1, #0xffffffff
        str r1, [r0, #0x04]
        str r1, [r0, #0x10]
    ELIF (FIXK310BUG == 2)
    	ldr r0, =RegIntMask
    	mov r1, #0x00000003
    	str r1, [r0]
    ENDIF

        ; Leave IRQ mode
        orr r0,r4,#NOINT
        bic r0,r0,#TBIT
        msr cpsr_c,r0

        msr spsr_cxsf,r4
        mov sp,r5

    ; Fix K310 IRQ/FIQ bug, recover IRQ/FIQ mask
    IF (FIXK310BUG == 1)
        ldr r0, =RegCtrlBase
        ldr r2, FiqMaskBackupI
        str r2, [r0, #0x04]
        ldr r2, IrqMaskBackupI
        str r2, [r0, #0x10]
    ELIF (FIXK310BUG == 2)
    	ldr r0, =RegIntMask
    	mov r1, #0x00000000
    	str r1, [r0]
    ENDIF

        ; chanel piority: 5>1>2>3>4
      IF (SPEEDUPIRQFIQ == 1)
        ldr r0, =SdramTimeCtrl
        ldr r1, [r0]
        bic r1, r1, #0x00c00000
        str r1, [r0]
      ENDIF

        ldmfd sp!,{r0-r12,lr,pc}^   ; pop new task's r0-r12,lr & pc
        
;*********************************************************************************************************
;                                            FIQ HANDLER
;
;        This handles all the FIQs
;
;*********************************************************************************************************
	
	    ;//IMPORT  FIQ_Handler             ; In int_handler.c
        IMPORT  OSIntFIQEnter
        IMPORT  OSIntFIQExit

        IMPORT  OSIntFIQCtxSwFlag
        IMPORT  OSTCBCur
        IMPORT  OSTaskSwHook
        IMPORT  OSTCBHighRdy
        IMPORT  OSPrioCur
        IMPORT  OSPrioHighRdy
        IMPORT  marsIntFIQHandler
        
        EXPORT  UCOS_FIQHandler
UCOS_FIQHandler

        ; Fix the return address
        sub lr,lr,#4
        stmfd sp!,{r0-r3,r12,lr}

        mov r9,#0xd0000000
        ldr r10,[r9,#0x14]
	tst r10,#1
	BNE	_FIQ_DISABLE_EXIT

    ; Fix K310 IRQ/FIQ bug
    IF (FIXK310BUG == 1)
        ;ldr r0, =RegCtrlBase
        ;ldr r1, [r0, #0x04]
        ;teq r1, #0x0000007f
        ;beq _FIQExit
    ENDIF

        ;chanel piority: 5>1>4>2>3
      IF (SPEEDUPIRQFIQ == 1)
        ldr r0, =SdramTimeCtrl
        ldr r1, [r0]
        bic r1, r1, #0x00c00000
        orr r1, r1, #0x00800000
        str r1, [r0]
      ENDIF

        bl OSIntFIQEnter
        bl marsIntFIQHandler   ;//FIQ_Handler    ;//
        bl OSIntFIQExit
                
        ; Is OSIntFIQCtxSwFlag == True ?
        ldr r0,=OSIntFIQCtxSwFlag
        ldr r1,[r0]
        cmp r1,#1
        beq _IntFIQCtxSw
_FIQExit
    ; chanel piority: 5>1>2>3>4
      IF (SPEEDUPIRQFIQ == 1)
        ldr r0, =SdramTimeCtrl
        ldr r1, [r0]
        bic r1, r1, #0x00c00000
        str r1, [r0]
      ENDIF
_FIQ_DISABLE_EXIT	      
        ldmfd sp!,{r0-r3,r12,pc}^


_IntFIQCtxSw
        ; OSIntFIQCtxSwFlag = False
        mov r1,#0
        str r1,[r0]

    ; Fix K310 IRQ/FIQ bug, enable all IRQ/FIQ mask
    IF (FIXK310BUG == 1)
        ldr r0, =RegCtrlBase
        ldr r1, [r0, #0x04]
        ldr r2, =FiqMaskBackupF
        str r1, [r2]
        ldr r1, [r0, #0x10]
        ldr r2, =IrqMaskBackupF
        str r1, [r2]
        mov r1, #0xffffffff
        str r1, [r0, #0x04]
        str r1, [r0, #0x10]
    ELIF (FIXK310BUG == 2)
    	ldr r0, =RegIntMask
    	mov r1, #0x00000003
    	str r1, [r0]
    ENDIF

        ; Get old task's sp & lr
        mrs r0,spsr
        orr r0,r0,#NOINT
        bic r0,r0,#TBIT
        msr cpsr_c,r0
        mov r1,sp
        mov r2,lr
        bic r0,r0,#MODEMASK
        orr r0,r0,#FIQMODE
        msr cpsr_c,r0

        ; Unwind FIQ's sp without poping r0-r3
        mov r3,sp
        add sp,sp,#(4*4)
        ldmfd sp!,{r12,lr}

        stmfd r1!,{r2,lr}           ; push old task's pc & lr
        ;CY 0601 S
        ;stmfd r1!,{r4-r12}          ; push old task's r12-r4
	; Get old task's r4-r12
        mrs r0,spsr
        orr r0,r0,#NOINT
        bic r0,r0,#TBIT
        msr cpsr_c,r0
        stmfd r1!,{r4-r12}          ; push old task's r12-r4
        bic r0,r0,#MODEMASK
        orr r0,r0,#FIQMODE
        msr cpsr_c,r0
	;CY 0601 E
	
    ; Fix K310 IRQ/FIQ bug, recover IRQ/FIQ mask
    ;IF (FIXK310BUG == 1)
    ;    ldr r0, =RegCtrlBase
    ;    ldr r3, FiqMaskBackupF
    ;    str r3, [r0, #0x04]
    ;    ldr r3, IrqMaskBackupF
    ;    str r3, [r0, #0x10]
    ;ENDIF

        ; Get r0-r3 from FIQ's stack
        mov r6,r1
        ldmfd r3,{r0-r3}
        stmfd r6!,{r0-r3}           ; push old task's r3-r0
        
        mrs r3,spsr
        stmfd r6!,{r3}              ; push old task's psr
        stmfd r6!,{r3}              ; push old task's spsr (don't care)
        
        ; OSPrioCur = OSPrioHighRdy
        ldr r4,=OSPrioCur
        ldr r5,=OSPrioHighRdy
        ldrb r5,[r5]
        strb r5,[r4]
        
        ; Get current task's TCB address
        ldr r4,=OSTCBCur
        ldr r5,[r4]
        str r6,[r5]                 ; store sp in preempted tasks's TCB

        bl OSTaskSwHook             ; call Task Switch Hook

        ; Get highest priority task's TCB address
        ldr r6,=OSTCBHighRdy
        ldr r6,[r6]
        ldr r5,[r6]                 ; get new task's stack pointer

        ; OSTCBCur = OSTCBHighRdy
        str r6,[r4]                 ; set new current task TCB address

        ldmfd r5!,{r4}              ; pop new task's spsr (don't care)
        ldmfd r5!,{r4}              ; pop new task's psr

    ; Fix K310 IRQ/FIQ bug, enable all IRQ/FIQ mask
    ;IF (FIXK310BUG == 1)
    ;    ldr r0, =RegCtrlBase
    ;    ldr r1, [r0, #0x04]
    ;    ldr r2, =FiqMaskBackupF
    ;    str r1, [r2]
    ;    ldr r1, [r0, #0x10]
    ;    ldr r2, =IrqMaskBackupF
    ;    str r1, [r2]
    ;    mov r1, #0xffffffff
    ;    str r1, [r0, #0x04]
    ;    str r1, [r0, #0x10]
    ;ENDIF

        ; Leave FIQ mode
        orr r0,r4,#NOINT
        bic r0,r0,#TBIT
        msr cpsr_c,r0

        msr spsr_cxsf,r4
        mov sp,r5

    ; Fix K310 IRQ/FIQ bug, recover IRQ/FIQ mask
    IF (FIXK310BUG == 1)
        ldr r0, =RegCtrlBase
        ldr r2, FiqMaskBackupF
        str r2, [r0, #0x04]
        ldr r2, IrqMaskBackupF
        str r2, [r0, #0x10]
    ELIF (FIXK310BUG == 2)
    	ldr r0, =RegIntMask
    	mov r1, #0x00000000
    	str r1, [r0]
    ENDIF

        ; chanel piority: 5>1>2>3>4
      IF (SPEEDUPIRQFIQ == 1)
        ldr r0, =SdramTimeCtrl
        ldr r1, [r0]
        bic r1, r1, #0x00c00000
        str r1, [r0]
      ENDIF

        ldmfd sp!,{r0-r12,lr,pc}^   ; pop new task's r0-r12,lr & pc

;*********************************************************************************************************
;                                   CRITICAL SECTION METHOD 3 FUNCTIONS
;
; Description: Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
;              would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
;              disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to 
;              disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
;              into the CPU's status register.
;
;              OS_CPU_SR OSCPUSaveSR()
; Arguments  : none
;
; Returns    : OS_CPU_SR
;
;              OSCPURestoreSR(OS_CPU_SR cpu_sr)
; Arguments  : OS_CPU_SR
;
; Returns    : none
;
; Note(s)    : These functions are used in general like this,
;
;            void Task (void *data)
;            {
;                    #if OS_CRITICAL_METHOD == 3 /* Allocate storage for CPU status register */
;                          OS_CPU_SR  cpu_sr;
;                    #endif
;                         :
;                         :
;                    OS_ENTER_CRITICAL(); /* cpu_sr = OSCPUSaveSR(); */
;                         :
;                         :
;                    OS_EXIT_CRITICAL();  /* OSCPURestoreSR(cpu_sr); */
;                         :
;                         :
;            }
;*********************************************************************************************************

        EXPORT  OSCPUSaveSR
OSCPUSaveSR

    ; Fix K310 IRQ/FIQ bug, enable all IRQ/FIQ mask
    IF (FIXK310BUG == 1)
        ldr r0, =RegCtrlBase
        ldr r2, [r0, #0x04]
        ;stmfd sp!,{r2-r6}
        ;ldr r0, =RegIrqMaskL
        ;ldr r2, [r0, #-0x10]
        ;ldr r2, =FiqMaskBackup
        ;str r1, [r2]
        ldr r3, [r0, #0x10]
        ;ldr r3, [r0, #-0x04]
        ;ldr r2, =IrqMaskBackup
        ;str r1, [r2]
        ;mov r1, #0xffffffff			;; modify by ben
        ;mov r4, #0xffffffff
        ;mov r5, #0xffffffff
        ;mov r6, #0xffffffff
        ;str r1, [r0, #0x04]			;; modify by ben
        ;str r1, [r0, #0x10]			;; modify by ben
        ;stmfd r0,{r1,r4,r5,r6}
    ELIF (FIXK310BUG == 2)
    	stmfd sp!,{r1-r3}
    	ldr r2, =RegIntMask
    	mov r3, #0x00000003
    	str r3, [r2]
    	;ldmfd sp!,{r1-r3}
    ENDIF

        mrs r0,CPSR
        orr r1,r0,#NOINT
        msr CPSR_c,r1
        
    ; Fix K310 IRQ/FIQ bug, recover IRQ/FIQ mask
    IF (FIXK310BUG == 1)
        ldr r1, =RegCtrlBase
        ;ldr r2, FiqMaskBackup
        str r2, [r1, #0x04]
        ;ldr r2, IrqMaskBackup
        str r3, [r1, #0x10]
        ;ldmfd sp!,{r2-r6}
    ELIF (FIXK310BUG == 2)
    	ldr r2, =RegIntMask
    	mov r3, #0x00000000
    	str r3, [r2]
    	ldmfd sp!,{r1-r3}
    ENDIF

        bx lr


        EXPORT  OSCPURestoreSR
OSCPURestoreSR

    IF (FIXK310BUG == 2)
	    stmfd sp!,{r1-r2}
    	ldr r1, =RegIntMask
    	mov r2, #0x00000003
    	str r2, [r1]
    	ldmfd sp!,{r1-r2}
    ENDIF

        msr CPSR_c,r0

    IF (FIXK310BUG == 2)
	stmfd sp!,{r1-r2}
    	ldr r1, =RegIntMask
    	mov r2, #0x00000000
    	str r2, [r1]
    	ldmfd sp!,{r1-r2}
    ENDIF

    	bx lr

    ; Fix K310 IRQ/FIQ bug
    IF (FIXK310BUG == 1)
FiqMaskBackup   dcd 0x00000000
IrqMaskBackup   dcd 0x00000000
FiqMaskBackupI  dcd 0x00000000
IrqMaskBackupI  dcd 0x00000000
FiqMaskBackupF  dcd 0x00000000
IrqMaskBackupF  dcd 0x00000000
    ENDIF

    ; for K310 debug
;        EXPORT  Read_LR
;Read_LR
;        mov     r0, sp
;        ldmfd   sp,{r0}
;        bx      lr

        END
