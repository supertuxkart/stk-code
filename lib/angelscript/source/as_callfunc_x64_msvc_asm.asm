;
;  AngelCode Scripting Library
;  Copyright (c) 2003-2011 Andreas Jonsson
;
;  This software is provided 'as-is', without any express or implied 
;  warranty. In no event will the authors be held liable for any 
;  damages arising from the use of this software.
;
;  Permission is granted to anyone to use this software for any 
;  purpose, including commercial applications, and to alter it and 
;  redistribute it freely, subject to the following restrictions:
;
;  1. The origin of this software must not be misrepresented; you 
;     must not claim that you wrote the original software. If you use
;     this software in a product, an acknowledgment in the product 
;     documentation would be appreciated but is not required.
;
;  2. Altered source versions must be plainly marked as such, and 
;     must not be misrepresented as being the original software.
;
;  3. This notice may not be removed or altered from any source 
;     distribution.
;
;  The original version of this library can be located at:
;  http://www.angelcode.com/angelscript/
;
;  Andreas Jonsson
;  andreas@angelcode.com
;

.code
PUBLIC CallX64

; asQWORD CallX64(const asQWORD *args, const asQWORD *floatArgs, int paramSize, asQWORD func)

CallX64 PROC FRAME

	; PROLOG

	; We must save preserved registers that are used
	; TODO: No need to save unused registers

	push rbp
.pushreg rbp
	push rsi
.pushreg rsi
	push r11
.pushreg r11
	push rdi
.pushreg rdi
	push r12
.pushreg r12
	push r13
.pushreg r13
	push r14
.pushreg r14
	push r15
.pushreg r15
	push rbx
.pushreg rbx
	sub rsp, 050h
.allocstack 050h
	mov rbp, rsp
.setframe rbp, 0
.endprolog

	; Move function param to non-scratch register
	mov r14, r9		; r14 = function

	; Allocate space on the stack for the arguments
	; Make room for at least 4 arguments even if there are less. When
    ; the compiler does optimizations for speed it may use these for 
	; temporary storage.
	mov rdi, r8
	add rdi, 32

	; Make sure the stack pointer is 16byte aligned so the
	; whole program optimizations will work properly
	; TODO: optimize: Can this be optimized with fewer instructions?
	mov rsi, rsp
	sub rsi, rdi
	and rsi, 8h
	add rdi, rsi	
	sub rsp, rdi
		
	; Jump straight to calling the function if no parameters
	cmp r8d, 0		; Compare paramSize with 0
	je	callfunc	; Jump to call funtion if (paramSize == 0)

	; Move params to non-scratch registers
	mov rsi, rcx	; rsi = pArgs
	mov r11, rdx	; r11 = pFloatArgs (can be NULL)
	mov r12d, r8d	; r12 = paramSize
	
	; Copy arguments from script stack to application stack
	; Order is (first to last):
	; rcx, rdx, r8, r9 & everything else goes on stack
	mov rcx, qword ptr [rsi]
	mov rdx, qword ptr [rsi + 8]
	mov r8,  qword ptr [rsi + 16]
	mov r9,  qword ptr [rsi + 24]
	
	; Negate the 4 params from the size to be copied
	sub r12d, 32
	js  copyfloat	; Jump if negative result
	jz	copyfloat	; Jump if zero result
	
	; Now copy all remaining params onto stack allowing space for first four
	; params to be flushed back to the stack if required by the callee.
	
	add rsi, 32		; Position input pointer 4 args ahead
	mov r13, rsp	; Put the stack pointer into r13
	add r13, 32	 	; Leave space for first 4 args on stack

copyoverflow:
	mov r15, qword ptr [rsi]	; Read param from source stack into r15
	mov qword ptr [r13], r15	; Copy param to real stack
	add r13, 8					; Move virtual stack pointer
	add rsi, 8					; Move source stack pointer
	sub r12d, 8					; Decrement remaining count
	jnz copyoverflow			; Continue if more params

copyfloat:
	; Any floating point params?
	cmp r11, 0
	je  callfunc
	
	movlpd xmm0, qword ptr [r11]
	movlpd xmm1, qword ptr [r11 + 8]
	movlpd xmm2, qword ptr [r11 + 16]
	movlpd xmm3, qword ptr [r11 + 24]
	
callfunc:
	
	; Call function
	call r14
	
	; Restore the stack
	mov rsp, rbp
		
	; EPILOG: Restore stack & preserved registers
	add rsp, 050h
	pop rbx
	pop r15
	pop r14
	pop r13
	pop r12
	pop rdi
	pop r11
	pop rsi
	pop rbp

	; return value in RAX
	ret

CallX64 ENDP


PUBLIC GetReturnedFloat

; asDWORD GetReturnedFloat()

GetReturnedFloat PROC FRAME

	; PROLOG: Store registers and allocate stack space
	
	sub rsp, 8   ; We'll need 4 bytes for temporary storage (8 bytes with alignment)
.allocstack 8
.endprolog

	; Move the float value from the XMM0 register to RAX register
	movss dword ptr [rsp], xmm0
	mov   eax, dword ptr [rsp]
	
	; EPILOG: Clean up
	
	add rsp, 8

	ret

GetReturnedFloat ENDP


PUBLIC GetReturnedDouble

; asDWORD GetReturnedDouble()

GetReturnedDouble PROC FRAME

	; PROLOG: Store registers and allocate stack space
	
	sub rsp, 8	; We'll need 8 bytes for temporary storage
.allocstack 8
.endprolog

	; Move the double value from the XMM0 register to the RAX register
	movlpd qword ptr [rsp], xmm0
	mov    rax, qword ptr [rsp]
	
	; EPILOG: Clean up
	
	add rsp, 8
	
	ret
	
GetReturnedDouble ENDP

END