/*
  AngelCode Scripting Library
  Copyright (c) 2003-2015 Andreas Jonsson

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you
     must not claim that you wrote the original software. If you use
     this software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and
     must not be misrepresented as being the original software.

  3. This notice may not be removed or altered from any source
     distribution.

  The original version of this library can be located at:
  http://www.angelcode.com/angelscript/

  Andreas Jonsson
  andreas@angelcode.com
*/

/*
   Assembly routines for the Playstation Vita SNC call convention.

   This code was adapted from as_callfunc_arm_gcc (ARM, Linux hard float) by Brandon Bare on October 2014.
*/

#if !defined(AS_MAX_PORTABILITY)

#ifdef __psp2__

.syntax unified
.cpu cortex-a9
.fpu neon

.section .text.armCallFunc
.balign 2
.thumb
.thumb_func

.align 2

.global armFunc
.global armFuncR0
.global armFuncR0R1
.global armFuncObjLast
.global armFuncR0ObjLast

.type armFunc, %function
.type armFuncR0, %function
.type armFuncR0R1, %function
.type armFuncObjLast, %function
.type armFuncR0ObjLast, %function

/* --------------------------------------------------------------------------------------------*/
armFunc:
    .fnstart

    push    {r4-r8, r10, r11, lr}   /* sp must be 8-byte alignment for ABI compliance, so the pushed registers must be even */

    mov     r6, r0  /* arg table */
    movs    r7, r1  /* arg size (also set the condition code flags so that we detect if there are no arguments) */
    mov     r4, r2  /* function address */

    /* Load float and double args into d0-d7 and s0-s15 */
    add       r10, r6, #272 /* r10 (r6 + 272) points to the first value for the VFP registers */
    mov       r8, #0
    vldmia.64 r10, {d0-d7}  /* Load contents starting at r10 into registers d0-d7 */

    /* If there are no arguments to set into r0-r3 */
    /* go check if there are arguments for the stack */
    beq     stackargs

    /* Load the first 4 arguments into r0-r3 */
    cmp     r7, #4

    it ge
    ldrge   r0, [r6]
    cmp     r7, #8

    it ge
    ldrge   r1, [r6, #4]
    cmp     r7, #12

    it ge
    ldrge   r2, [r6, #8]
    cmp     r7, #16

    it ge
    ldrge   r3, [r6, #12]

stackargs:
    ldr     r5, [r6, #268]  /* Load stack size into r5 */
    movs    r7, r5          /* Load stack size into r7, checking for 0 args */

    /* If there are no args for the stack, branch */
    beq     nomoreargs

    /* Load the rest of the arguments onto the stack */
    /* Ensure 8-byte stack alignment */
    mov     r8, sp
    sub     sp, sp, r7
    add     r6, r6, #16     /* Set r6 to point to the first arg to be placed on the stack */

    sub     r12, sp, #8
    bic     r12, r12, #7    /* thumb mode couldn't support "bic  sp, sp, #7" instruction */
    sub     r8, r8, r12
    mov     sp, r12         /* copy size != frame size, so store frame start sp, r12(ip) is not callee saved register */

stackargsloop:
    ldr     r5, [r6], #4
    subs    r7, r7, #4
    str     r5, [sp], #4
    bne     stackargsloop
    mov     sp, r12

nomoreargs:
    blx     r4
    add     sp, sp, r8
    vstmia.64 r10, {d0-d7}   /* Copy contents of registers d0-d7 to the address stored in r10 */

    pop {r4-r8, r10, r11, pc}

    .fnend

/* --------------------------------------------------------------------------------------------*/
armFuncObjLast:
    .fnstart

    push {r4-r8, r10, r11, lr}  /* We´re storing r11 just to keep the stack aligned to an 8 byte boundary */

    mov     r6, r0  /* arg table */
    movs    r7, r1  /* arg size (also set the condition code flags so that we detect if there are no arguments) */
    mov     r4, r2  /* function address */

    mov     r0, r3          /* objlast. might get overwritten */
    mov     r5, #0          /* This will hold an offset of #4 only if objlast couldn´t be placed into an "r" register */

    /* Load float and double args into d0-d7 and s0-s15 (r10 holds pointer to first float value) */
    add     r10, r6, #272   /* r10 (r6 + 272) points to the first value for the VFP registers */
    mov     r8, #0
    vldmia.64 r10, {d0-d7}  /* Load contents starting at r10 into registers d0-d7 */

    /* If there are no arguments to set into r0-r3 */
    /* go check if there are arguments for the stack */
    beq     stackargsFuncObjLast

    mov     r5, r3          /* store objlast in r5 temporarily */

    /* Load the first 4 arguments into r0-r3 */
    cmp     r7, #4

    it ge
    ldrge   r0, [r6]
    cmp     r7, #8

    it ge
    ldrge   r1, [r6,#4]

    it lt
    movlt   r1, r5
    cmp     r7, #12

    it ge
    ldrge   r2, [r6,#8]

    it lt
    movlt   r2, r5
    cmp     r7, #16

    it ge
    ldrge   r3, [r6,#12]

    ittt lt
    movlt   r3, r5
    movlt   r5, #0                  /* If objlast got placed into a register, r5 = 0 */
    blt     stackargsFuncObjLast    /* If objlast got placed into a register, go to stackargsFuncObjLast */

    str     r5, [r6, #12]           /* Put objlast in r6 + 12 */
    mov     r5, #4                  /* Set r5 with an offset of #4, so objlast can be loaded into the stack */

stackargsFuncObjLast:
    ldr     r7, [r6, #268]  /* Load stack size into r7 */
    add     r7, r7, r5      /* Add the offset placed in r5 (could be #0 or #4) */
    cmp     r7, #0          /* Check for 0 args */

    /* If there are no args for the stack, branch */
    beq     nomoreargsarmFuncObjLast

    /* Load the rest of the arguments onto the stack */
    /* Ensure 8-byte stack alignment */
    mov     r8, sp
    sub     sp, sp, r7
    add     r6, r6, #16     /* Set r6 to point to the first arg to be placed on the stack */

    sub     r12, sp, #8
    sub     r6, r6, r5      /* r6 = r6 - r5 (r5 can be #0 or #4) */
    bic     r12, r12, #7    /* thumb mode couldn't support "bic  sp, sp, #7" instruction */
    sub     r8, r8, r12
    mov     sp, r12         /* copy size != frame size, so store frame start sp, r12(ip) is not callee saved register */

stackargslooparmFuncObjLast:
    ldr     r5, [r6], #4
    subs    r7, r7, #4
    str     r5, [sp], #4
    bne     stackargslooparmFuncObjLast
    mov     sp, r12

nomoreargsarmFuncObjLast:
    blx     r4
    add     sp, sp, r8
    vstmia.64   r10, {d0-d7}    /* Copy contents of registers d0-d10 to the address stored in r10 */

    pop   {r4-r8, r10,r11, pc}

    .fnend

/* --------------------------------------------------------------------------------------------*/
armFuncR0ObjLast:
    .fnstart

    push    {r4-r8, r10, r11, lr}

    ldr     r5, [sp,#32]   /* objlast to temp reg */

    mov     r6, r0  /* arg table */
    movs    r7, r1  /* arg size (also set the condition code flags so that we detect if there are no arguments) */
    mov     r4, r2  /* function address */

    mov     r0, r3      /* r0 explicitly set */
    mov     r1, r5      /* objlast.  might get overwritten */
    mov     r5, #0      /* This will hold an offset of #4 or #8 if objlast or one arg couldn´t be placed into an "r" register */

    /* Load float and double args into d0-d7 and s0-s15 (r10 holds pointer to first float value) */
    add     r10, r6, #272   /* r10 (r6 + 272) points to the first value for the VFP registers */
    mov     r8, #0
    vldmia.64 r10, {d0-d7}  /* Load contents starting at r10 into registers d0-d7 */

    /* If there are no arguments to set into r0-r3 */
    /* go check if there are arguments for the stack */
    beq     stackargsFuncR0ObjLast

    mov     r5, r1          /* store objlast in r5 temporarily */

    /* Load the first 3 arguments into r1-r3 */
    cmp     r7, #4

    it ge
    ldrge   r1, [r6]
    cmp     r7, #8

    it ge
    ldrge   r2, [r6,#4]

    it lt
    movlt   r2, r5
    cmp     r7, #12

    it ge
    ldrge   r3, [r6,#8]

    ittt lt
    movlt   r3, r5
    movlt   r5, #0                  /* If objlast got placed into a register, r5 = 0 */
    blt     stackargsFuncR0ObjLast  /* If objlast got placed into a register, go to stackargsFuncR0ObjLast */

    cmp     r7, #16                 /* Else if we have one last arg set the offset accordingly and store the arg in the array */

    itt ge
    ldrge   r7, [r6, #12]
    strge   r7, [r6, #8]

    str     r5, [r6, #12]           /* Put objlast in r6 + 12 */
    mov     r5, #0

    it ge
    movge   r5, #4                  /* Set r5 with an offset of #4 if there´s one last arg that couldn´t be placed in r registers */
    add     r5, r5, #4              /* Set r5 with an offset of + #4, so objlast can be loaded into the stack */

stackargsFuncR0ObjLast:
    ldr     r7, [r6, #268]  /* Load stack size into r7 */
    add     r7, r7, r5      /* Add the offset placed in r5 (could be #0 or #4) */
    cmp     r7, #0          /* Check for 0 args */

    /* If there are no args for the stack, branch */
    beq     nomoreargsarmFuncR0ObjLast

    /* Load the rest of the arguments onto the stack */
    /* Ensure 8-byte stack alignment */
    mov     r8, sp
    sub     sp, sp, r7
    add     r6, r6, #16     /* Set r6 to point to the first arg to be placed on the stack */

    sub     r12, sp, #8
    sub     r6, r6, r5      /* r6 = r6 - r5 (r5 can be #0 or #4) */
    bic     r12, r12, #7    /* thumb mode couldn't support "bic  sp, sp, #7" instruction */
    sub     r8, r8, r12
    mov     sp, r12         /* copy size != frame size, so store frame start sp, r12(ip) is not callee saved register */

stackargslooparmFuncR0ObjLast:
    ldr     r5, [r6], #4
    subs    r7, r7, #4
    str     r5, [sp], #4
    bne     stackargslooparmFuncR0ObjLast
    mov     sp, r12

nomoreargsarmFuncR0ObjLast:
    blx     r4
    add     sp, sp, r8
    vstmia.64   r10, {d0-d7}    /* Copy contents of registers d0-d10 to the address stored in r10 */

    pop {r4-r8, r10, r11, pc}

    .fnend

/* --------------------------------------------------------------------------------------------*/
armFuncR0:
    .fnstart

    push {r4-r8, r10, r11, lr}

    mov     r6, r0  /* arg table */
    movs    r7, r1  /* arg size (also set the condition code flags so that we detect if there are no arguments) */
    mov     r4, r2  /* function address */
    mov     r11, #0 /* This will hold an offset of #4 only if the last arg that should have been placed into an "r" reg needs to go to the stack */
    mov     r0, r3  /* r0 explicitly set */

    /* Load float and double args into d0-d7 and s0-s15 (r10 holds pointer to first float value) */
    add     r10, r6, #272   /* r10 (r6 + 272) points to the first value for the VFP registers */
    mov     r8, #0
    vldmia.64 r10, {d0-d7}  /* Load contents starting at r10 into registers d0-d7 */

    /* If there are no arguments to set into r0-r3 */
    /* go check if there are arguments for the stack */
    beq     stackargsarmFuncR0

    /* Load the first 3 arguments into r1-r3 */
    cmp     r7, #4

    it ge
    ldrge   r1, [r6]
    cmp     r7, #8

    it ge
    ldrge   r2, [r6, #4]
    cmp     r7, #12

    it ge
    ldrge   r3, [r6, #8]
    cmp     r7, #16

    it ge
    movge   r11, #4         /* If there is still one arg to be placed, set the offset in r11 to #4 */

stackargsarmFuncR0:
    ldr     r5, [r6, #268]  /* Load stack size into r5 */
    add     r5, r11         /* Add the offset placed in r11 (could be #0 or #4) */
    movs    r7, r5          /* Load stack size into r7, checking for 0 args */

    /* If there are no args for the stack, branch */
    beq     nomoreargsarmFuncR0

    /* Load the rest of the arguments onto the stack */
    /* Ensure 8-byte stack alignment */
    mov     r8, sp
    sub     sp, sp, r7
    add     r6, r6, #16     /* Set r6 to point to the first arg to be placed on the stack */

    sub     r12, sp, #8
    sub     r6, r6, r11     /* r6 = r6 - r11 (r11 can be #0 or #4) */
    bic     r12, r12, #7    /* thumb mode couldn't support "bic  sp, sp, #7" instruction */
    sub     r8, r8, r12
    mov     sp, r12         /* copy size != frame size, so store frame start sp, r12(ip) is not callee saved register */

stackargslooparmFuncR0:
    ldr     r5, [r6], #4
    subs    r7, r7, #4
    str     r5, [sp], #4
    bne     stackargslooparmFuncR0
    mov     sp, r12

nomoreargsarmFuncR0:
    blx     r4
    add     sp, sp, r8
    vstmia.64   r10, {d0-d7}    /* Copy contents of registers d0-d10 to the address stored in r10 */

    pop {r4-r8, r10, r11, pc}

    .fnend

/* --------------------------------------------------------------------------------------------*/
armFuncR0R1:
    .fnstart

    push {r4-r8, r10, r11, lr}

    mov     r6, r0  /* arg table */
    movs    r7, r1  /* arg size (also set the condition code flags so that we detect if there are no arguments) */
    mov     r4, r2  /* function address */
    mov     r11, #0 /* This will hold an offset of #4 or #8 only if the last arg (or last 2 args) that should have been placed into "r" regs need to go to the stack */

    mov     r0, r3          /* r0 explicitly set */
    ldr     r1, [sp, #32]   /* r1 explicitly set too */

    /* Load float and double args into d0-d7 and s0-s15 (r10 holds pointer to first float value) */
    add     r10, r6, #272   /* r10 (r6 + 272) points to the first value for the VFP registers */
    mov     r8, #0
    vldmia.64 r10, {d0-d7}  /* Load contents starting at r10 into registers d0-d7 */

    /* If there are no arguments to set into r2-r3 */
    /* go check if there are arguments for the stack */
    beq     stackargsarmFuncR0R1

    /* Load the first 2 arguments into r2-r3 */
    cmp     r7, #4

    it ge
    ldrge   r2, [r6]
    cmp     r7, #8

    it ge
    ldrge   r3, [r6, #4]
    cmp     r7, #12

    it ge
    movge   r11, #4         /* If there is a third arg to be placed, set the offset in r11 to #4 */

    cmp     r7, #16

    it ge
    movge   r11, #8         /* If there is a fourth arg to be placed, set the offset in r11 to #8 */

    itt lt
    ldrlt   r7, [r6, #8]    /* Else copy the third arg to the correct place in the array */
    strlt   r7, [r6, #12]

stackargsarmFuncR0R1:
    ldr     r5, [r6, #268]  /* Load stack size into r5 */
    add     r5, r11         /* Add the offset placed in r11 (could be #0 or #4 or #8) */
    movs    r7, r5          /* Load stack size into r7, checking for 0 args */

    /* If there are no args for the stack, branch */
    beq     nomoreargsarmFuncR0R1

    /* Load the rest of the arguments onto the stack */
    /* Ensure 8-byte stack alignment */
    mov     r8, sp
    sub     sp, sp, r7
    add     r6, r6, #16     /* Set r6 to point to the first arg to be placed on the stack */

    sub     r12, sp, #8
    sub     r6, r6, r11     /* r6 = r6 - r11 (r11 can be #0 or #4 or #8) */
    bic     r12, r12, #7    /* thumb mode couldn't support "bic  sp, sp, #7" instruction */
    sub     r8, r8, r12
    mov     sp, r12         /* copy size != frame size, so store frame start sp, r12(ip) is not callee saved register */

stackargslooparmFuncR0R1:
    ldr     r5, [r6], #4
    subs    r7, r7, #4
    str     r5, [sp], #4
    bne     stackargslooparmFuncR0R1
    mov     sp, r12

nomoreargsarmFuncR0R1:
    blx     r4
    add     sp, sp, r8
    vstmia.64   r10, {d0-d7}    /* Copy contents of registers d0-d10 to the address stored in r10 */

    pop {r4-r8, r10, r11, pc}

    .fnend

#endif

#endif /* !AS_MAX_PORTABILITY */

