;
;  AngelCode Scripting Library
;  Copyright (c) 2020-2020 Andreas Jonsson
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


; Assembly routines for the ARM64/AArch64 call convention used for Windows 10 on ARM
; Written by Max Waine in July 2020, based on as_callfunc_arm_msvc.asm

; MSVC currently doesn't support inline assembly for the ARM64 platform,
; and if they're treating it like x64 /won't/ ever support inline assembly,
; so this separate file is needed.

; Compile with Microsoft ARM64 assembler (armasm64)
; http://msdn.microsoft.com/en-us/library/hh873190.aspx

    AREA    |.rdata|, DATA, READONLY
    EXPORT  GetHFAReturnDouble
    EXPORT  GetHFAReturnFloat
    EXPORT  CallARM64Ret128
    EXPORT  CallARM64RetInMemory
    EXPORT  CallARM64Double
    EXPORT  CallARM64Float
    EXPORT  CallARM64

    AREA    |.text|, CODE, ALIGN=2

    ALIGN   4
GetHFAReturnDouble PROC
    adr     x9, |populateDoubles|
    sub     x9, x9, x1, lsr 1 ; x9 -= returnSize >> 1; (/2 because double is 2x instruction size)
    br      x9

    str     d3, [x0, #0x18]
    str     d2, [x0, #0x10]
    str     d1, [x1]
    str     d0, [x0]
|populateDoubles|

    ret
    ENDP ; GetHFAReturnDouble

    ALIGN   4
GetHFAReturnFloat PROC
    adr     x9, |populateFloats|
    sub     x9, x9, x2 // x9 -= returnSize; (already 4 bytes per return)
    br      x9

    str     s3, [x1, #0x4]
    str     s2, [x1]
    str     s1, [x0, #0x4]
    str     s0, [x0]
|populateFloats|

    ret
    ENDP ; GetHFAReturnFloat


;[returnType] CallARM64[type](
;    const asQWORD *gpRegArgs,    asQWORD numGPRegArgs,
;    const asQWORD *floatRegArgs, asQWORD numFloatRegArgs,
;    const asQWORD *stackArgs,    asQWORD numStackArgs,
;    asFUNCTION_t func
;)
    ALIGN   4
CallARM64Double PROC
    stp     fp, lr, [sp,#-0x10]!
    bl      CallARM64
    ldp     fp, lr, [sp,#-0x10]!
    ret
    ENDP ; CallARM64Double

    ALIGN   4
CallARM64Float PROC
    stp     fp, lr, [sp,#-0x10]!
    bl      CallARM64
    ldp     fp, lr, [sp,#-0x10]!
    ret
    ENDP ; CallARM64Float

    ALIGN   4
CallARM64 PROC
    stp     fp, lr, [sp,#-0x20]!
    str     x20, [sp,#0x10]

    mov     x20, #0;

    cbz     x5, |stackArgsLoopEnd|

    ; Align count to 2, then multiply by 8, resulting in a size aligned to 16
    add x20, x5,  #1
    lsl x20, x20, #3
    and x20, x20, #-0x10
    ; Multiply count by 8
    lsl x10, x5, #3
    sub sp, sp, x20
|stackArgsLoopStart|
    ldp     x9,x11, [x4],#16
    stp     x9,x11, [sp],#16
    subs    x10, x10, #16
    bgt     |stackArgsLoopStart|
|stackArgsLoopEnd|

    ; Calculate amount to jump forward, avoiding pointless instructions
    adr     x9, |populateFloatRegisterArgsEnd|
    sub     x9, x9, x3, lsl 2 ; x9 -= numFloatRegArgs * 4
    br      x9

    ldr     d7, [x2, #0x38]
    ldr     d6, [x2, #0x30]
    ldr     d5, [x2, #0x28]
    ldr     d4, [x2, #0x20]
    ldr     d3, [x2, #0x18]
    ldr     d2, [x2, #0x10]
    ldr     d1, [x2, #0x08]
    ldr     d0, [x2]
|populateFloatRegisterArgsEnd|

    mov     x15, x6
    ; Calculate amount to jump forward, avoiding pointless instructions
    adr     x9, |populateGPRegisterArgsEnd|
    sub     x9, x9, x1, lsl 2 ; x9 -= numGPRegArgs * 4
    br      x9

    ldr     x7, [x0, #0x38]
    ldr     x6, [x0, #0x30]
    ldr     x5, [x0, #0x28]
    ldr     x4, [x0, #0x20]
    ldr     x3, [x0, #0x18]
    ldr     x2, [x0, #0x10]
    ldr     x1, [x0, #0x08]
    ldr     x0, [x0]
|populateGPRegisterArgsEnd|

    ; Actually call function
    sub     sp, sp, x20
    blr     x15
    add     sp, sp, x20

    ldr     x20, [sp,#0x10]
    ldp     fp, lr, [sp],#0x20

    ret
    ENDP ; CallARM64

    ALIGN   4
CallARM64Ret128 PROC
    stp     fp, lr, [sp,#-0x20]!
    str     x20, [sp,#0x10]
    mov     fp, sp

    mov     x20, x6
    mov     x6, x7
    mov     x7, #0
    bl      CallARM64

    str     x1, [x20]

    ldr     x20, [sp,#0x10]
    ldp     fp, lr, [sp],#0x20

    ret ; CallARM64Ret128

    ALIGN   4
CallARM64RetInMemory PROC
    stp     fp, lr, [sp,#-0x10]!
    mov     fp, sp

    mov     x8, x6
    mov     x6, x7
    mov     x7, #0
    bl      CallARM64

    mov     x0, x8

    ldp     fp, lr, [sp],#0x10

    ret ; CallARM64RetInMemory

    END
