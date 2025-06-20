// From https://github.com/skeeto/w64devkit/blob/master/src/libchkstk.S

#if 0
# Implementations of ___chkstk_ms (GCC) and __chkstk (MSVC). Unlike
# libgcc, no work happens if the stack is already committed. Execute
# this source with a shell to build libchkstk.a.
# This is free and unencumbered software released into the public domain.
set -ex
${CC:-cc} -c -DCHKSTK_MS -Wa,--no-pad-sections -o chkstk_ms.o $0
${CC:-cc} -c -DCHKSTK    -Wa,--no-pad-sections -o chkstk.o    $0
rm -f "${DESTDIR}libchkstk.a"
${AR:-ar} r "${DESTDIR}libchkstk.a" chkstk_ms.o chkstk.o
rm chkstk_ms.o chkstk.o
exit 0
#endif

#if __amd64
// On x64, ___chkstk_ms and __chkstk have identical semantics. Unlike
// x86 __chkstk, neither adjusts the stack pointer. This implementation
// preserves all registers.
//
// The frame size is passed in rax, and this function ensures that
// enough of the stack is committed for the frame. It commits stack
// pages by writing to the guard page, one page at a time.
#  if CHKSTK_MS
	.globl ___chkstk_ms
___chkstk_ms:
#  elif CHKSTK
	.globl __chkstk
__chkstk:
#  endif
	push %rax
	push %rcx
	mov  %gs:(0x10), %rcx	// rcx = stack low address
	neg  %rax		// rax = frame low address
	add  %rsp, %rax		// "
	jb   1f			// frame low address overflow?
	xor  %eax, %eax		// overflowed: frame low address = null
0:	sub  $0x1000, %rcx	// extend stack into guard page
	test %eax, (%rcx)	// commit page (two instruction bytes)
1:	cmp  %rax, %rcx
	ja   0b
	pop  %rcx
	pop  %rax
	ret
#endif  // __amd64

#if __i386
#  if CHKSTK_MS
// Behaves exactly like x64 ___chkstk_ms.
	.globl ___chkstk_ms
___chkstk_ms:
	push %eax
	push %ecx
	mov  %fs:(0x08), %ecx	// ecx = stack low address
	neg  %eax		// eax = frame low address
	add  %esp, %eax		// "
	jb   1f			// frame low address overflow?
	xor  %eax, %eax		// overflowed: frame low address = null
0:	sub  $0x1000, %ecx	// extend stack into guard page
	test %eax, (%ecx)	// commit page (two instruction bytes)
1:	cmp  %eax, %ecx
	ja   0b
	pop  %ecx
	pop  %eax
	ret
#  elif CHKSTK
// On x86, __chkstk allocates the new stack frame. This implementation
// clobbers eax. MSVC only seems to care about ebp and ecx (this).
	.globl __chkstk
__chkstk:
	push %ecx		// preserve ecx
	mov  %fs:(0x08), %ecx	// ecx = stack low address
	neg  %eax		// eax = frame low address
	lea  8(%esp,%eax), %eax	// "
	cmp  %esp, %eax		// frame low address overflow?
	jb   1f			// "
	xor  %eax, %eax		// overflowed: frame low address = null
0:	sub  $0x1000, %ecx	// extend stack into guard page
	test %eax, (%ecx)	// commit page (two instruction bytes)
1:	cmp  %eax, %ecx
	ja   0b
	pop  %ecx		// restore ecx
	xchg %eax, %esp		// allocate frame
	jmp  *(%eax)		// return
#  endif
#endif  // __i386
