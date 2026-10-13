//
//  AngelCode Scripting Library
//  Copyright (c) 2025 Andreas Jonsson
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


//
// Implements the E2K calling convention
//
// Author: Denis Drakhnia <numas13@gmail.com>
//

#if !defined(AS_MAX_PORTABILITY)
#ifdef AS_E2K


    .file "as_callfunc_e2k.S"
    .text
    .globl CallFunctionE2K
    .type CallFunctionE2K, #function
    .align 8
CallFunctionE2K:
    {
        setwd       wsz=16, nfx=1, dbl=0
        setbn       rsz=7, rbs=4, rcur=0
        movtd,0     %r0, %ctpr1
        cmpedb,1    %r2, 0, %pred0
        shld,2      1, 37, %g17                 // lsr.vlc
        disp        %ctpr2, 1f
    }
    {
        getsp,0     %r5, %r5                    // allocate space on stack for args or return value
        cmpedb,1    %r6, 0, %pred1              // is value returned in memory?
        ord,2       %r2, %g17, %g16             // lsr
        disp        %ctpr3, 0f
    }
    {
        rwd,0       %g16, %lsr
        ldd,2,sm    %r1, 0x00, %b[14]           // loop prologue
        ldd,3,sm    %r1, 0x08, %b[12]
        ldd,5,sm    %r1, 0x10, %b[10]
    }
    {
        ldd,0,sm    %r1, 0x18, %b[8]
        ldd,2,sm    %r1, 0x20, %b[6]
        ldd,3,sm    %r1, 0x28, %b[4]
        ldd,5,sm    %r1, 0x30, %b[2]
    }
    {
        addd,1      %r5, 0, %g16
        addd,2      %r1, 0x38, %g17
        ct          %ctpr2 ? %pred0             // 1f, do not copy arguments to memory
        nop 1 // wait %lsr
    }
 0:
    {
        alc         alcf=1, alct=1
        abn         abnf=1, abnt=1
        addd,1      %g17, 8, %g17
        ldd,2       %g17, 0, %b[0]
        addd,4      %g16, 8, %g16
        std,5       %b[14], %g16, 0
        ct          %ctpr3 ? #NOT_LOOP_END      // 0b
    }
1:
    {
        setwd       wsz=8, nfx=1, dbl=0
        setbn       rsz=3, rbs=4, rcur=0
        ldd,0       %r1, 0x00, %r8              // load arguments to registers
        ldd,2       %r1, 0x08, %r9
        ldd,3       %r1, 0x10, %r10
        ldd,5       %r1, 0x18, %r11
    }
    {
        ldd,0       %r1, 0x20, %b[4]
        ldd,2       %r1, 0x28, %b[5]
        ldd,3       %r1, 0x30, %b[6]
        ldd,5       %r1, 0x38, %b[7]
        call        %ctpr1, wbs=4               // %r0
    }
    {
        shld,0      1, 37, %g16                 // lsr.vlc
        std,2,sm    %b[0], %r3, 0x00 ? %pred1
        std,5,sm    %b[1], %r3, 0x08 ? %pred1
        return      %ctpr3
    }
    {
        ord,0       %r4, %g16, %g16             // lsr
        std,2,sm    %b[2], %r3, 0x10 ? %pred1
        std,5,sm    %b[3], %r3, 0x18 ? %pred1
        disp        %ctpr1, 0f
    }
    {
        rwd,0       %g16, %lsr
        std,2,sm    %b[4], %r3, 0x20 ? %pred1
        std,5,sm    %b[5], %r3, 0x28 ? %pred1
    }
    {
        std,2,sm    %b[6], %r3, 0x30 ? %pred1
        std,5,sm    %b[7], %r3, 0x38 ? %pred1
    }
    {
        setwd       wsz=16, nfx=1, dbl=0
        setbn       rsz=7, rbs=4, rcur=0
        ldw,0,sm    %r5, 0x00, %r22             // %b[14] loop prologue
        ldw,2,sm    %r5, 0x04, %r20             // %b[12]
        ldw,3,sm    %r5, 0x08, %r18             // %b[10]
        ldw,5,sm    %r5, 0x0c, %r16             // %b[8]
        ct          %ctpr3 ? %pred1             // return
    }
    {
        addd,1      %r5, 0x1c, %r5
        ldw,2,sm    %r5, 0x10, %b[6]
        ldw,3,sm    %r5, 0x14, %b[4]
        ldw,5,sm    %r5, 0x18, %b[2]
#if __iset__ > 5
        nop 3
#else
        nop 1
#endif
    }
0:
    {
        alc         alcf=1, alct=1
        abn         abnf=1, abnt=1
        addd,1      %r3, 4, %r3
        stw,2       %b[14], %r3, 0
        addd,3      %r5, 4, %r5
        ldw,5,sm    %r5, 0, %b[0]
        ct          %ctpr1 ? #NOT_LOOP_END      // 0b
    }
    {
        ct          %ctpr3                      // return
    }
    .size CallFunctionE2K, .-CallFunctionE2K

#endif /* AS_E2K */
#endif /* !AS_MAX_PORTABILITY */
