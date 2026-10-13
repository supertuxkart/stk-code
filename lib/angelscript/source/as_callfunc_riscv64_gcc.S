//
//  AngelCode Scripting Library
//  Copyright (c) 2024 Andreas Jonsson
//
//  This software is provided 'as-is', without any express or implied
//  warranty. In no event will the authors be held liable for any
//  damages arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any
//  purpose, including commercial applications, and to alter it and
//  redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented// you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//     distribution.
//
//  The original version of this library can be located at:
//  http://www.angelcode.com/angelscript/
//
//  Andreas Jonsson
//  andreas@angelcode.com
//


// Assembly routines for the 64bit RISC-V call convention used for Linux

// Compile with GCC/GAS

#if !defined(AS_MAX_PORTABILITY)

#if defined(__riscv) && defined(__LP64__)

	.file "as_callfunc_riscv64_gcc.S"
	.option pic
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.align 1
	.globl CallRiscVFunc
	.type CallRiscVFunc, @function
CallRiscVFunc:
	.cfi_startproc

	// Setup call stack
	addi    sp,sp,-336        // reserve bytes on stack (aligned to 16bytes). 256 bytes (32*8) is reserved for values that will be pushed on the stack for the function call, the rest is for local backup of registers
	.cfi_def_cfa_offset 336
	sd      ra,328(sp)        // backup return address on stack, just below original stack frame pointer
	sd      s0,320(sp)        // backup frame pointer on stack, below return address
	.cfi_offset 1, -8
	.cfi_offset 8, -16
	addi    s0,sp,336         // load new frame pointer with the reserved space
	.cfi_def_cfa 8,0

	// Backup arguments on stack
	// TODO: skip the backup and move them into the temp registers directly
	sd      a0,-24(s0)        // store func arg on stack, below backup of s0
	sd      a1,-32(s0)        // store retfloat arg on stack
	sd      a2,-40(s0)        // store argValues arg on stack
	sd      a3,-48(s0)        // store numRegularValues arg on stack
	sd      a4,-56(s0)        // store numFloatValues arg on stack
	sd      a5,-64(s0)        // store numStackValues arg on stack

	ld      t1,-40(s0)        // load argValues arg into t1

	// Load regular values into regular registers
	// TODO: skip ahead to the number of generic args
	ld      a7,56(t1)         // a7 = argValues[7]
	ld      a6,48(t1)         // a6 = argValues[6]
	ld      a5,40(t1)         // a5 = argValues[5]
	ld      a4,32(t1)         // a4 = argValues[4]
	ld      a3,24(t1)         // a3 = argValues[3]
	ld      a2,16(t1)         // a2 = argValues[2]
	ld      a1,8(t1)          // a1 = argValues[1]
	ld      a0,0(t1)          // a0 = argValues[0]
	
	// Load float values into float registers
	// TODO: skip ahead to the number of float args
	fld     fa7,120(t1)       // fa7 = argValues[15]
	fld     fa6,112(t1)       // fa6 = argValues[14]
	fld     fa5,104(t1)       // fa5 = argValues[13]
	fld     fa4,96(t1)        // fa4 = argValues[12]
	fld     fa3,88(t1)        // fa3 = argValues[11]
	fld     fa2,80(t1)        // fa2 = argValues[10]
	fld     fa1,72(t1)        // fa1 = argValues[9]
	fld     fa0,64(t1)        // fa0 = argValues[8]

	// Push the remaining args on the stack
	addi    t1,t1,128         // t1 = &argValues[16]
	mv      t3,sp             // t3 = sp
	ld      t2,-64(s0)        // t2 = numStackValues
	beq     t2,x0,.L_nomore   // jump if t2 == 0
.L_next:
	addi    t2,t2,-1          // t2--
    ld      t5,0(t1)          // t5 value from argValues
	sd      t5,0(t3)          // store the value on the stack
	addi    t3,t3,8           // move t3 to the next slot on the stack
	addi    t1,t1,8           // move to the next value in argValues
	bne     t2,x0,.L_next     // reiterate if t2 is not zero
.L_nomore:

	// Call the function
	ld      t1,-24(s0)        // load func arg to t1
	jalr    t1                // call the function in func arg
	nop

	// If the function returns a float value, then retrieve that
	ld      a5,-32(s0)        // restore retfloat
	sext.w  a4,a5             // load lower 32bit word of retfloat into a4
	li      a5,1              // set a5 = 1
	bne     a4,a5,.L_nofloat  // jump if a4 != 1  TODO: rewrite to compare with x0 which is hardwired to 0 so there is no need to set a5
	fsd     fa0,-32(s0)       // store the raw returned 64bit float/double value on the stack (where retfloat was)
	ld      a0,-32(s0)        // load the raw 64bit value into a0 for return
	fsd     fa1,-32(s0)       // store the raw returned 64bit float/double value on the stack (where retfloat was)
	ld      a1,-32(s0)        // load the raw 64bit value into a1 for return
.L_nofloat:

	// Clean up call stack
	ld      ra,328(sp)        // restore return address from stack
	.cfi_restore 1
	ld      s0,320(sp)        // restore frame pointer from stack
	.cfi_restore 8
	.cfi_def_cfa 2, 336
	addi    sp,sp,336         // clear reserved space from stack
	.cfi_def_cfa_offset 0
	jr      ra

	.cfi_endproc
	.size CallRiscVFunc, .-CallRiscVFunc

#endif /* __riscv && __LP64__ */

#endif /* !AS_MAX_PORTABILITY */
