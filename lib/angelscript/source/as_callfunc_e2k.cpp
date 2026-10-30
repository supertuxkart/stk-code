/*
   AngelCode Scripting Library
   Copyright (c) 2025 Andreas Jonsson

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
 * Implements the E2K calling convention
 *
 * Author: Denis Drakhnia <numas13@gmail.com>
 */

#include "as_config.h"

#ifndef AS_MAX_PORTABILITY
#ifdef AS_E2K

#include "as_scriptengine.h"
#include "as_texts.h"
#include "as_context.h"

BEGIN_AS_NAMESPACE

#define E2K_MAX_ARGS 64

struct E2KArguments {
    asDWORD *m_args;
    int m_index = 0;

    E2KArguments(asDWORD *args): m_args(args) {}

    asDWORD *ptr() {
        return &m_args[m_index];
    }

    asDWORD takeDWord() {
        return m_args[m_index++];
    }

    asQWORD takeQWord() {
        asQWORD lo = takeDWord();
        asQWORD hi = takeDWord();
        return (hi << 32) | lo;
    }

    asQWORD take(int count) {
        switch (count) {
        case 1: return takeDWord();
        case 2: return takeQWord();
        default:
            asASSERT(0 && "implement me");
            return 0;
        }
    }
};

struct E2KBuffer {
    int m_count = 0;
    asQWORD m_buffer[E2K_MAX_ARGS];

    const asQWORD *ptr() const {
        return m_buffer;
    }

    int count() const {
        return m_count;
    }

    void align() {
        m_count += m_count & 1;
    }

    void push(asQWORD arg) {
        asASSERT(m_count < E2K_MAX_ARGS);
        m_buffer[m_count++] = arg;
    }

    void push_object(void *parm, int size) {
        int c = (size + 7) / 8;
        asASSERT(m_count + c <= E2K_MAX_ARGS);
        memcpy(&m_buffer[m_count], parm, size);
        m_count += c;
    }
};

extern "C" void CallFunctionE2K(asFUNCTION_t func, const asQWORD *args,
        int argsCount, void *retPointer, int retSize, int stackSize, bool returnInMemory);

void CallSystemFunctionNative(asCContext *context, asCScriptFunction *descr,
    void *obj, asDWORD *args, void *retPointer, asQWORD *retQW, void *secondObject)
{
    asCScriptEngine *engine = context->m_engine;
    asSSystemFunctionInterface *sysFunc = descr->sysFuncIntf;
    const asCDataType &retType = descr->returnType;
    asFUNCTION_t func = sysFunc->func;
    int callConv = sysFunc->callConv;
    E2KArguments parms(args);
    E2KBuffer buffer;
    bool retComplex = retType.IsObject() && (retType.GetTypeInfo()->flags & COMPLEX_RETURN_MASK);

    if (sysFunc->hostReturnInMemory && retComplex) {
        // The return is made in memory.
        buffer.push((asPWORD) retPointer);
    }

    if (obj) {
        asFUNCTION_t *vftable;
        switch (callConv) {
        case ICC_VIRTUAL_THISCALL:
        case ICC_VIRTUAL_THISCALL_RETURNINMEM:
#ifndef AS_NO_THISCALL_FUNCTOR_METHOD
        case ICC_VIRTUAL_THISCALL_OBJFIRST:
        case ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM:
        case ICC_VIRTUAL_THISCALL_OBJLAST:
        case ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM:
#endif
            vftable = *((asFUNCTION_t**) obj);
            func = vftable[FuncPtrToUInt(asFUNCTION_t(func)) >> 3];
            break;
        }
    }

    switch (callConv) {
    case ICC_CDECL_OBJFIRST:
    case ICC_THISCALL:
    case ICC_VIRTUAL_THISCALL:
    case ICC_CDECL_OBJFIRST_RETURNINMEM:
    case ICC_THISCALL_RETURNINMEM:
    case ICC_VIRTUAL_THISCALL_RETURNINMEM:
#ifndef AS_NO_THISCALL_FUNCTOR_METHOD
    case ICC_THISCALL_OBJLAST:
    case ICC_VIRTUAL_THISCALL_OBJLAST:
    case ICC_THISCALL_OBJLAST_RETURNINMEM:
    case ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM:
#endif
        buffer.push((asPWORD) obj);
        break;
#ifndef AS_NO_THISCALL_FUNCTOR_METHOD
    case ICC_THISCALL_OBJFIRST:
    case ICC_VIRTUAL_THISCALL_OBJFIRST:
    case ICC_THISCALL_OBJFIRST_RETURNINMEM:
    case ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM:
        buffer.push((asPWORD) obj);
        buffer.push((asPWORD) secondObject);
        break;
#endif
    }

    for (int i = 0; i < descr->parameterTypes.GetLength(); ++i) {
        const asCDataType& parmType = descr->parameterTypes[i];

        if (parmType.GetTokenType() == ttQuestion) {
            // The variable args are really two, one pointer and one type id.
            buffer.push(parms.takeQWord());
            buffer.push(parms.takeDWord());
        } else if (parmType.IsPrimitive() || parmType.IsReference() || parmType.IsObjectHandle()) {
            // Primitive types and references are single slot values.
            buffer.push(parms.take(parmType.GetSizeOnStackDWords()));
        } else {
            // An object is being passed by value.
            if ((parmType.GetTypeInfo()->flags & COMPLEX_MASK)) {
                // Copy the address of the object.
                buffer.push(parms.takeQWord());
            } else {
                // Copy the value of the object.
                int size = parmType.GetSizeInMemoryBytes();
                void *parm = *(void **) parms.ptr();
                if (size > 8) {
                    buffer.align();
                    buffer.push_object(parm, size);
                    buffer.align();
                } else {
                    buffer.push_object(parm, size);
                }
                // Skip pointer.
                parms.takeQWord();
                // Delete the original memory.
                engine->CallFree(parm);
            }
        }
    }

    switch (callConv) {
    case ICC_CDECL_OBJLAST:
    case ICC_CDECL_OBJLAST_RETURNINMEM:
        buffer.push((asPWORD) obj);
        break;
#ifndef AS_NO_THISCALL_FUNCTOR_METHOD
    case ICC_THISCALL_OBJLAST:
    case ICC_THISCALL_OBJLAST_RETURNINMEM:
    case ICC_VIRTUAL_THISCALL_OBJLAST:
    case ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM:
        buffer.push((asPWORD) secondObject);
        break;
#endif
    }

    int stackSize = buffer.count() * (sizeof(asQWORD) / sizeof(asDWORD));
    int retSize = sysFunc->hostReturnSize;
    bool returnInMemory = sysFunc->hostReturnInMemory && !retComplex;

    if (returnInMemory) {
        // Returned on the user stack and copied to retPointer.
        retSize = retType.GetSizeInMemoryDWords();
        if (retSize > stackSize) {
            stackSize = retSize;
        }
    } else {
        // Returned in registers and copied to retQW.
        asASSERT(retSize <= RETURN_VALUE_MAX_SIZE);
        retPointer = retQW;
    }

    stackSize = (stackSize * sizeof(asDWORD) + 15) & ~15;
    CallFunctionE2K(func, buffer.ptr(), buffer.count(), retPointer, retSize, -stackSize, returnInMemory);
}

END_AS_NAMESPACE

#endif // AS_E2K
#endif // AS_MAX_PORTABILITY
