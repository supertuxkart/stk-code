/*
   AngelCode Scripting Library
   Copyright (c) 2003-2025 Andreas Jonsson

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


//
// angelscript.h
//
// The script engine interface
//


#ifndef ANGELSCRIPT_H
#define ANGELSCRIPT_H

#include <stddef.h>
#ifndef _MSC_VER
#include <stdint.h>
#endif

#ifdef AS_USE_NAMESPACE
 #define BEGIN_AS_NAMESPACE namespace AngelScript {
 #define END_AS_NAMESPACE }
 #define AS_NAMESPACE_QUALIFIER AngelScript::
#else
 #define BEGIN_AS_NAMESPACE
 #define END_AS_NAMESPACE
 #define AS_NAMESPACE_QUALIFIER ::
#endif

BEGIN_AS_NAMESPACE

// AngelScript version

#define ANGELSCRIPT_VERSION        23800
#define ANGELSCRIPT_VERSION_STRING "2.38.0"

// Data types

class asIScriptEngine;
class asIScriptModule;
class asIScriptContext;
class asIScriptGeneric;
class asIScriptObject;
class asITypeInfo;
class asIScriptFunction;
class asIBinaryStream;
class asIJITCompilerAbstract;
class asIThreadManager;
class asILockableSharedBool;
class asIStringFactory;

//
// asBYTE  =  8 bits
// asWORD  = 16 bits
// asDWORD = 32 bits
// asQWORD = 64 bits
// asPWORD = size of pointer
//
typedef signed char    asINT8;
typedef signed short   asINT16;
typedef signed int     asINT32;
typedef unsigned char  asBYTE;
typedef unsigned short asWORD;
typedef unsigned int   asUINT;
#if (defined(_MSC_VER) && _MSC_VER <= 1200) || defined(__S3E__) || (defined(_MSC_VER) && defined(__clang__))
// size_t is not really correct, since it only guaranteed to be large enough to hold the segment size.
// For example, on 16bit systems the size_t may be 16bits only even if pointers are 32bit. But nobody
// is likely to use MSVC6 to compile for 16bit systems anymore, so this should be ok.
typedef size_t         asPWORD;
#else
typedef uintptr_t      asPWORD;
#endif
#ifdef __LP64__
typedef unsigned int  asDWORD;
typedef unsigned long asQWORD;
typedef long asINT64;
#else
typedef unsigned long asDWORD;
#if !defined(_MSC_VER) && (defined(__GNUC__) || defined(__MWERKS__) || defined(__SUNPRO_CC) || defined(__psp2__))
typedef uint64_t asQWORD;
typedef int64_t asINT64;
#else
typedef unsigned __int64 asQWORD;
typedef __int64 asINT64;
#endif
#endif

// Enumerations and constants

// Return codes
enum asERetCodes
{
	asSUCCESS                              =  0,
	asERROR                                = -1,
	asCONTEXT_ACTIVE                       = -2,
	asCONTEXT_NOT_FINISHED                 = -3,
	asCONTEXT_NOT_PREPARED                 = -4,
	asINVALID_ARG                          = -5,
	asNO_FUNCTION                          = -6,
	asNOT_SUPPORTED                        = -7,
	asINVALID_NAME                         = -8,
	asNAME_TAKEN                           = -9,
	asINVALID_DECLARATION                  = -10,
	asINVALID_OBJECT                       = -11,
	asINVALID_TYPE                         = -12,
	asALREADY_REGISTERED                   = -13,
	asMULTIPLE_FUNCTIONS                   = -14,
	asNO_MODULE                            = -15,
	asNO_GLOBAL_VAR                        = -16,
	asINVALID_CONFIGURATION                = -17,
	asINVALID_INTERFACE                    = -18,
	asCANT_BIND_ALL_FUNCTIONS              = -19,
	asLOWER_ARRAY_DIMENSION_NOT_REGISTERED = -20,
	asWRONG_CONFIG_GROUP                   = -21,
	asCONFIG_GROUP_IS_IN_USE               = -22,
	asILLEGAL_BEHAVIOUR_FOR_TYPE           = -23,
	asWRONG_CALLING_CONV                   = -24,
	asBUILD_IN_PROGRESS                    = -25,
	asINIT_GLOBAL_VARS_FAILED              = -26,
	asOUT_OF_MEMORY                        = -27,
	asMODULE_IS_IN_USE                     = -28
};

// Engine properties
enum asEEngineProp
{
	asEP_ALLOW_UNSAFE_REFERENCES            = 1,
	asEP_OPTIMIZE_BYTECODE                  = 2,
	asEP_COPY_SCRIPT_SECTIONS               = 3,
	asEP_MAX_STACK_SIZE                     = 4,
	asEP_USE_CHARACTER_LITERALS             = 5,
	asEP_ALLOW_MULTILINE_STRINGS            = 6,
	asEP_ALLOW_IMPLICIT_HANDLE_TYPES        = 7,
	asEP_BUILD_WITHOUT_LINE_CUES            = 8,
	asEP_INIT_GLOBAL_VARS_AFTER_BUILD       = 9,
	asEP_REQUIRE_ENUM_SCOPE                 = 10,
	asEP_SCRIPT_SCANNER                     = 11,
	asEP_INCLUDE_JIT_INSTRUCTIONS           = 12,
	asEP_STRING_ENCODING                    = 13,
	asEP_PROPERTY_ACCESSOR_MODE             = 14,
	asEP_EXPAND_DEF_ARRAY_TO_TMPL           = 15,
	asEP_AUTO_GARBAGE_COLLECT               = 16,
	asEP_DISALLOW_GLOBAL_VARS               = 17,
	asEP_ALWAYS_IMPL_DEFAULT_CONSTRUCT      = 18,
	asEP_COMPILER_WARNINGS                  = 19,
	asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE = 20,
	asEP_ALTER_SYNTAX_NAMED_ARGS            = 21,
	asEP_DISABLE_INTEGER_DIVISION           = 22,
	asEP_DISALLOW_EMPTY_LIST_ELEMENTS       = 23,
	asEP_PRIVATE_PROP_AS_PROTECTED          = 24,
	asEP_ALLOW_UNICODE_IDENTIFIERS          = 25,
	asEP_HEREDOC_TRIM_MODE                  = 26,
	asEP_MAX_NESTED_CALLS                   = 27,
	asEP_GENERIC_CALL_MODE                  = 28,
	asEP_INIT_STACK_SIZE                    = 29,
	asEP_INIT_CALL_STACK_SIZE               = 30,
	asEP_MAX_CALL_STACK_SIZE                = 31,
	asEP_IGNORE_DUPLICATE_SHARED_INTF       = 32,
	asEP_NO_DEBUG_OUTPUT                    = 33,
	asEP_DISABLE_SCRIPT_CLASS_GC            = 34,
	asEP_JIT_INTERFACE_VERSION              = 35,
	asEP_ALWAYS_IMPL_DEFAULT_COPY           = 36,
	asEP_ALWAYS_IMPL_DEFAULT_COPY_CONSTRUCT = 37,
	asEP_MEMBER_INIT_MODE                   = 38,
	asEP_BOOL_CONVERSION_MODE               = 39,
	asEP_FOREACH_SUPPORT                    = 40,

	asEP_LAST_PROPERTY
};

// Calling conventions
enum asECallConvTypes
{
	asCALL_CDECL             = 0,
	asCALL_STDCALL           = 1,
	asCALL_THISCALL_ASGLOBAL = 2,
	asCALL_THISCALL          = 3,
	asCALL_CDECL_OBJLAST     = 4,
	asCALL_CDECL_OBJFIRST    = 5,
	asCALL_GENERIC           = 6,
	asCALL_THISCALL_OBJLAST  = 7,
	asCALL_THISCALL_OBJFIRST = 8
};

// Object type flags
enum asEObjTypeFlags : asQWORD
{
	asOBJ_REF                         = (1<<0),
	asOBJ_VALUE                       = (1<<1),
	asOBJ_GC                          = (1<<2),
	asOBJ_POD                         = (1<<3),
	asOBJ_NOHANDLE                    = (1<<4),
	asOBJ_SCOPED                      = (1<<5),
	asOBJ_TEMPLATE                    = (1<<6),
	asOBJ_ASHANDLE                    = (1<<7),
	asOBJ_APP_CLASS                   = (1<<8),
	asOBJ_APP_CLASS_CONSTRUCTOR       = (1<<9),
	asOBJ_APP_CLASS_DESTRUCTOR        = (1<<10),
	asOBJ_APP_CLASS_ASSIGNMENT        = (1<<11),
	asOBJ_APP_CLASS_COPY_CONSTRUCTOR  = (1<<12),
	asOBJ_APP_CLASS_C                 = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR),
	asOBJ_APP_CLASS_CD                = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_DESTRUCTOR),
	asOBJ_APP_CLASS_CA                = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT),
	asOBJ_APP_CLASS_CK                = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	asOBJ_APP_CLASS_CDA               = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT),
	asOBJ_APP_CLASS_CDK               = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	asOBJ_APP_CLASS_CAK               = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	asOBJ_APP_CLASS_CDAK              = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	asOBJ_APP_CLASS_D                 = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_DESTRUCTOR),
	asOBJ_APP_CLASS_DA                = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT),
	asOBJ_APP_CLASS_DK                = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	asOBJ_APP_CLASS_DAK               = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	asOBJ_APP_CLASS_A                 = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_ASSIGNMENT),
	asOBJ_APP_CLASS_AK                = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_ASSIGNMENT + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	asOBJ_APP_CLASS_K                 = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	asOBJ_APP_CLASS_MORE_CONSTRUCTORS = (asQWORD(1) << 31),
	asOBJ_APP_PRIMITIVE               = (1<<13),
	asOBJ_APP_FLOAT                   = (1<<14),
	asOBJ_APP_ARRAY                   = (1<<15),
	asOBJ_APP_CLASS_ALLINTS           = (1<<16),
	asOBJ_APP_CLASS_ALLFLOATS         = (1<<17),
	asOBJ_NOCOUNT                     = (1<<18),
	asOBJ_APP_CLASS_ALIGN8            = (1<<19),
	asOBJ_IMPLICIT_HANDLE             = (1<<20),
	asOBJ_APP_CLASS_UNION             = (asQWORD(1)<<32),
	asOBJ_MASK_VALID_FLAGS            = 0x1801FFFFFul,
	// Internal flags
	asOBJ_SCRIPT_OBJECT               = (1<<21),
	asOBJ_SHARED                      = (1<<22),
	asOBJ_NOINHERIT                   = (1<<23),
	asOBJ_FUNCDEF                     = (1<<24),
	asOBJ_LIST_PATTERN                = (1<<25),
	asOBJ_ENUM                        = (1<<26),
	asOBJ_TEMPLATE_SUBTYPE            = (1<<27),
	asOBJ_TYPEDEF                     = (1<<28),
	asOBJ_ABSTRACT                    = (1<<29),
	asOBJ_APP_ALIGN16                 = (1<<30)
};

// Behaviours
enum asEBehaviours
{
	// Value object memory management
	asBEHAVE_CONSTRUCT,
	asBEHAVE_LIST_CONSTRUCT,
	asBEHAVE_DESTRUCT,

	// Reference object memory management
	asBEHAVE_FACTORY,
	asBEHAVE_LIST_FACTORY,
	asBEHAVE_ADDREF,
	asBEHAVE_RELEASE,
	asBEHAVE_GET_WEAKREF_FLAG,

	// Object operators
	asBEHAVE_TEMPLATE_CALLBACK,

	// Garbage collection behaviours
	asBEHAVE_FIRST_GC,
	 asBEHAVE_GETREFCOUNT = asBEHAVE_FIRST_GC,
	 asBEHAVE_SETGCFLAG,
	 asBEHAVE_GETGCFLAG,
	 asBEHAVE_ENUMREFS,
	 asBEHAVE_RELEASEREFS,
	asBEHAVE_LAST_GC = asBEHAVE_RELEASEREFS,

	asBEHAVE_MAX
};

// Context states
enum asEContextState
{
	asEXECUTION_FINISHED        = 0,
	asEXECUTION_SUSPENDED       = 1,
	asEXECUTION_ABORTED         = 2,
	asEXECUTION_EXCEPTION       = 3,
	asEXECUTION_PREPARED        = 4,
	asEXECUTION_UNINITIALIZED   = 5,
	asEXECUTION_ACTIVE          = 6,
	asEXECUTION_ERROR           = 7,
	asEXECUTION_DESERIALIZATION = 8
};

// Message types
enum asEMsgType
{
	asMSGTYPE_ERROR       = 0,
	asMSGTYPE_WARNING     = 1,
	asMSGTYPE_INFORMATION = 2
};

// Garbage collector flags
enum asEGCFlags
{
	asGC_FULL_CYCLE      = 1,
	asGC_ONE_STEP        = 2,
	asGC_DESTROY_GARBAGE = 4,
	asGC_DETECT_GARBAGE  = 8
};

// Token classes
enum asETokenClass
{
	asTC_UNKNOWN    = 0,
	asTC_KEYWORD    = 1,
	asTC_VALUE      = 2,
	asTC_IDENTIFIER = 3,
	asTC_COMMENT    = 4,
	asTC_WHITESPACE = 5
};

// Type id flags
enum asETypeIdFlags
{
	asTYPEID_VOID           = 0,
	asTYPEID_BOOL           = 1,
	asTYPEID_INT8           = 2,
	asTYPEID_INT16          = 3,
	asTYPEID_INT32          = 4,
	asTYPEID_INT64          = 5,
	asTYPEID_UINT8          = 6,
	asTYPEID_UINT16         = 7,
	asTYPEID_UINT32         = 8,
	asTYPEID_UINT64         = 9,
	asTYPEID_FLOAT          = 10,
	asTYPEID_DOUBLE         = 11,
	asTYPEID_OBJHANDLE      = 0x40000000,
	asTYPEID_HANDLETOCONST  = 0x20000000,
	asTYPEID_MASK_OBJECT    = 0x1C000000,
	asTYPEID_APPOBJECT      = 0x04000000,
	asTYPEID_SCRIPTOBJECT   = 0x08000000,
	asTYPEID_TEMPLATE       = 0x10000000,
	asTYPEID_MASK_SEQNBR    = 0x03FFFFFF
};

// Type modifiers
enum asETypeModifiers
{
	asTM_NONE     = 0,
	asTM_INREF    = 1,
	asTM_OUTREF   = 2,
	asTM_INOUTREF = 3,
	asTM_CONST    = 4
};

// GetModule flags
enum asEGMFlags
{
	asGM_ONLY_IF_EXISTS       = 0,
	asGM_CREATE_IF_NOT_EXISTS = 1,
	asGM_ALWAYS_CREATE        = 2
};

// Compile flags
enum asECompileFlags
{
	asCOMP_ADD_TO_MODULE = 1
};

// Function types
enum asEFuncType
{
	asFUNC_DUMMY     =-1,
	asFUNC_SYSTEM    = 0,
	asFUNC_SCRIPT    = 1,
	asFUNC_INTERFACE = 2,
	asFUNC_VIRTUAL   = 3,
	asFUNC_FUNCDEF   = 4,
	asFUNC_IMPORTED  = 5,
	asFUNC_DELEGATE  = 6,
	asFUNC_TEMPLATE  = 7
};

// Is the target a 64bit system?
#if defined(__LP64__) || defined(__amd64__) || defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || defined(_M_ARM64)
	#ifndef AS_64BIT_PTR
		#define AS_64BIT_PTR
	#endif
#endif

typedef void (*asFUNCTION_t)();
typedef void (*asGENFUNC_t)(asIScriptGeneric *);
typedef void *(*asALLOCFUNC_t)(size_t);
typedef void (*asFREEFUNC_t)(void *);
typedef void (*asCLEANENGINEFUNC_t)(asIScriptEngine *);
typedef void (*asCLEANMODULEFUNC_t)(asIScriptModule *);
typedef void (*asCLEANCONTEXTFUNC_t)(asIScriptContext *);
typedef void (*asCLEANFUNCTIONFUNC_t)(asIScriptFunction *);
typedef void (*asCLEANTYPEINFOFUNC_t)(asITypeInfo *);
typedef void (*asCLEANSCRIPTOBJECTFUNC_t)(asIScriptObject *);
typedef asIScriptContext *(*asREQUESTCONTEXTFUNC_t)(asIScriptEngine *, void *);
typedef void (*asRETURNCONTEXTFUNC_t)(asIScriptEngine *, asIScriptContext *, void *);
typedef void (*asCIRCULARREFFUNC_t)(asITypeInfo *, const void *, void *);

struct asSVMRegisters;
typedef void (*asJITFunction)(asSVMRegisters* registers, asPWORD jitArg);

// Check if the compiler can use C++11 features
#if !defined(_MSC_VER) || _MSC_VER >= 1700   // MSVC 2012
 #if !defined(__GNUC__) || defined(__clang__) || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)  // gnuc 4.7 or clang
  #if !(defined(__GNUC__) && defined(__cplusplus) && __cplusplus < 201103L) // gnuc and clang require compiler flag -std=c++11
   #if !defined(__SUNPRO_CC) // Oracle Solaris Studio
    #define AS_CAN_USE_CPP11 1
   #endif
  #endif
 #endif
#endif

// This macro does basically the same thing as offsetof defined in stddef.h, but
// GNUC should not complain about the usage as I'm not using 0 as the base pointer.
#define asOFFSET(s,m) ((int)(size_t)(&reinterpret_cast<s*>(100000)->m)-100000)

#define asFUNCTION(f) asFunctionPtr(f)
#if (defined(_MSC_VER) && _MSC_VER <= 1200) || (defined(__BORLANDC__) && __BORLANDC__ < 0x590)
// MSVC 6 has a bug that prevents it from properly compiling using the correct asFUNCTIONPR with operator >
// so we need to use ordinary C style cast instead of static_cast. The drawback is that the compiler can't
// check that the cast is really valid.
// BCC v5.8 (C++Builder 2006) and earlier have a similar bug which forces us to fall back to a C-style cast.
#define asFUNCTIONPR(f,p,r) asFunctionPtr((void (*)())((r (*)p)(f)))
#else
#define asFUNCTIONPR(f,p,r) asFunctionPtr(reinterpret_cast<void (*)()>(static_cast<r (*)p>(f)))
#endif

#ifndef AS_NO_CLASS_METHODS

class asCUnknownClass;
typedef void (asCUnknownClass::*asMETHOD_t)();

struct asSFuncPtr
{
	asSFuncPtr(asBYTE f = 0)
	{
		for( size_t n = 0; n < sizeof(ptr.dummy); n++ )
			ptr.dummy[n] = 0;
		flag = f;
	}

	void CopyMethodPtr(const void *mthdPtr, size_t size)
	{
		for( size_t n = 0; n < size; n++ )
			ptr.dummy[n] = reinterpret_cast<const char *>(mthdPtr)[n];
	}

	union
	{
		// The largest known method point is 20 bytes (MSVC 64bit),
		// but with 8byte alignment this becomes 24 bytes. So we need
		// to be able to store at least that much.
		char dummy[25];
		struct {asMETHOD_t   mthd; char dummy[25-sizeof(asMETHOD_t)];} m;
		struct {asFUNCTION_t func; char dummy[25-sizeof(asFUNCTION_t)];} f;
	} ptr;
	asBYTE flag; // 1 = generic, 2 = global func, 3 = method
};

#if defined(__BORLANDC__)
// A bug in BCC (QC #85374) makes it impossible to distinguish const/non-const method overloads
// with static_cast<>. The workaround is to use an _implicit_cast instead.

 #if  __BORLANDC__ < 0x590
 // BCC v5.8 (C++Builder 2006) and earlier have an even more annoying bug which causes
 // the "pretty" workaround below (with _implicit_cast<>) to fail. For these compilers
 // we need to use a traditional C-style cast.
  #define AS_METHOD_AMBIGUITY_CAST(t) (t)
 #else
template <typename T>
  T _implicit_cast (T val)
{ return val; }
  #define AS_METHOD_AMBIGUITY_CAST(t) AS_NAMESPACE_QUALIFIER _implicit_cast<t >
 #endif
#else
 #define AS_METHOD_AMBIGUITY_CAST(t) static_cast<t >
#endif

#define asMETHOD(c,m) asSMethodPtr<sizeof(void (c::*)())>::Convert((void (c::*)())(&c::m))
#define asMETHODPR(c,m,p,r) asSMethodPtr<sizeof(void (c::*)())>::Convert(AS_METHOD_AMBIGUITY_CAST(r (c::*)p)(&c::m))

#else // Class methods are disabled

struct asSFuncPtr
{
	asSFuncPtr(asBYTE f)
	{
		for( size_t n = 0; n < sizeof(ptr.dummy); n++ )
			ptr.dummy[n] = 0;
		flag = f;
	}

	union
	{
		char dummy[25]; // largest known class method pointer
		struct {asFUNCTION_t func; char dummy[25-sizeof(asFUNCTION_t)];} f;
	} ptr;
	asBYTE flag; // 1 = generic, 2 = global func
};

#endif

struct asSMessageInfo
{
	const char *section;
	int         row;
	int         col;
	asEMsgType  type;
	const char *message;
};


// API functions

// ANGELSCRIPT_EXPORT is defined when compiling the dll or lib
// ANGELSCRIPT_DLL_LIBRARY_IMPORT is defined when dynamically linking to the
// dll through the link lib automatically generated by MSVC++
// ANGELSCRIPT_DLL_MANUAL_IMPORT is defined when manually loading the dll
// Don't define anything when linking statically to the lib

#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)
  #if defined(ANGELSCRIPT_EXPORT)
    #define AS_API __declspec(dllexport)
  #elif defined(ANGELSCRIPT_DLL_LIBRARY_IMPORT)
    #define AS_API __declspec(dllimport)
  #else // statically linked library
    #define AS_API
  #endif
#elif defined(__GNUC__)
  #if defined(ANGELSCRIPT_EXPORT)
    #define AS_API __attribute__((visibility ("default")))
  #else
    #define AS_API
  #endif
#else
  #define AS_API
#endif

#ifndef ANGELSCRIPT_DLL_MANUAL_IMPORT
extern "C"
{
	// Engine
	AS_API asIScriptEngine *asCreateScriptEngine(asDWORD version = ANGELSCRIPT_VERSION);
	AS_API const char      *asGetLibraryVersion();
	AS_API const char      *asGetLibraryOptions();

	// Context
	AS_API asIScriptContext *asGetActiveContext();

	// Thread support
	AS_API int               asPrepareMultithread(asIThreadManager *externalMgr = 0);
	AS_API void              asUnprepareMultithread();
	AS_API asIThreadManager *asGetThreadManager();
	AS_API void              asAcquireExclusiveLock();
	AS_API void              asReleaseExclusiveLock();
	AS_API void              asAcquireSharedLock();
	AS_API void              asReleaseSharedLock();
	AS_API int               asAtomicInc(int &value);
	AS_API int               asAtomicDec(int &value);
	AS_API int               asThreadCleanup();

	// Memory management
	AS_API int   asSetGlobalMemoryFunctions(asALLOCFUNC_t allocFunc, asFREEFUNC_t freeFunc);
	AS_API int   asResetGlobalMemoryFunctions();
	AS_API void *asAllocMem(size_t size);
	AS_API void  asFreeMem(void *mem);

	// Auxiliary
	AS_API asILockableSharedBool *asCreateLockableSharedBool();
}
#endif // ANGELSCRIPT_DLL_MANUAL_IMPORT

// Determine traits of a type for registration of value types
// Relies on C++11 features so it can not be used with non-compliant compilers
#ifdef AS_CAN_USE_CPP11

END_AS_NAMESPACE
#include <type_traits>
BEGIN_AS_NAMESPACE

template<typename T>
asUINT asGetTypeTraits()
{
#if defined(_MSC_VER) || defined(_LIBCPP_TYPE_TRAITS) || (__GNUC__ >= 5) || (defined(__clang__) && !defined(CLANG_PRE_STANDARD))
	// MSVC, XCode/Clang, and gnuc 5+
	// C++11 compliant code
	bool hasConstructor        = std::is_default_constructible<T>::value && !std::is_trivially_default_constructible<T>::value;
	bool hasDestructor         = std::is_destructible<T>::value          && !std::is_trivially_destructible<T>::value;
	bool hasAssignmentOperator = std::is_copy_assignable<T>::value       && !std::is_trivially_copy_assignable<T>::value;
	bool hasCopyConstructor    = std::is_copy_constructible<T>::value    && !std::is_trivially_copy_constructible<T>::value;
#elif (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))) || (defined(__clang__) && defined(CLANG_PRE_STANDARD))
	// gnuc 4.8 is using a mix of C++11 standard and pre-standard templates
	bool hasConstructor        = std::is_default_constructible<T>::value && !std::has_trivial_default_constructor<T>::value;
	bool hasDestructor         = std::is_destructible<T>::value          && !std::is_trivially_destructible<T>::value;
	bool hasAssignmentOperator = std::is_copy_assignable<T>::value       && !std::has_trivial_copy_assign<T>::value;
	bool hasCopyConstructor    = std::is_copy_constructible<T>::value    && !std::has_trivial_copy_constructor<T>::value;
#else
	// All other compilers and versions are assumed to use non C++11 compliant code until proven otherwise
	// Not fully C++11 compliant. The has_trivial checks were used while the standard was still
	// being elaborated, but were then removed in favor of the above is_trivially checks
	// http://stackoverflow.com/questions/12702103/writing-code-that-works-when-has-trivial-destructor-is-defined-instead-of-is
	// https://github.com/mozart/mozart2/issues/51
	bool hasConstructor        = std::is_default_constructible<T>::value && !std::has_trivial_default_constructor<T>::value;
	bool hasDestructor         = std::is_destructible<T>::value          && !std::has_trivial_destructor<T>::value;
	bool hasAssignmentOperator = std::is_copy_assignable<T>::value       && !std::has_trivial_copy_assign<T>::value;
	bool hasCopyConstructor    = std::is_copy_constructible<T>::value    && !std::has_trivial_copy_constructor<T>::value;
#endif
	bool isFloat     = std::is_floating_point<T>::value;
	bool isPrimitive = std::is_integral<T>::value || std::is_pointer<T>::value || std::is_enum<T>::value;
	bool isClass     = std::is_class<T>::value;
	bool isArray     = std::is_array<T>::value;

	if( isFloat )
		return asOBJ_APP_FLOAT;
	if( isPrimitive )
		return asOBJ_APP_PRIMITIVE;

	if( isClass )
	{
		asDWORD flags = asOBJ_APP_CLASS;
		if( hasConstructor )
			flags |= asOBJ_APP_CLASS_CONSTRUCTOR;
		if( hasDestructor )
			flags |= asOBJ_APP_CLASS_DESTRUCTOR;
		if( hasAssignmentOperator )
			flags |= asOBJ_APP_CLASS_ASSIGNMENT;
		if( hasCopyConstructor )
			flags |= asOBJ_APP_CLASS_COPY_CONSTRUCTOR;
		return flags;
	}

	if( isArray )
		return asOBJ_APP_ARRAY;

	// Unknown type traits
	return 0;
}

#endif // c++11

// Interface declarations

class asIScriptEngine
{
public:
	// Memory management
	virtual int AddRef() const = 0;
	virtual int Release() const = 0;
	virtual int ShutDownAndRelease() = 0;

	// Engine properties
	virtual int     SetEngineProperty(asEEngineProp property, asPWORD value) = 0;
	virtual asPWORD GetEngineProperty(asEEngineProp property) const = 0;

	// Compiler messages
	virtual int SetMessageCallback(const asSFuncPtr &callback, void *obj, asDWORD callConv) = 0;
	virtual int GetMessageCallback(asSFuncPtr *callback, void **obj, asDWORD *callConv) = 0;
	virtual int ClearMessageCallback() = 0;
	virtual int WriteMessage(const char *section, int row, int col, asEMsgType type, const char *message) = 0;

	// JIT Compiler
	virtual int SetJITCompiler(asIJITCompilerAbstract *compiler) = 0;
	virtual asIJITCompilerAbstract *GetJITCompiler() const = 0;

	// Global functions
	virtual int                RegisterGlobalFunction(const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *auxiliary = 0) = 0;
	virtual asUINT             GetGlobalFunctionCount() const = 0;
	virtual asIScriptFunction *GetGlobalFunctionByIndex(asUINT index) const = 0;
	virtual asIScriptFunction *GetGlobalFunctionByDecl(const char *declaration) const = 0;

	// Global properties
	virtual int    RegisterGlobalProperty(const char *declaration, void *pointer) = 0;
	virtual asUINT GetGlobalPropertyCount() const = 0;
	virtual int    GetGlobalPropertyByIndex(asUINT index, const char **name, const char **nameSpace = 0, int *typeId = 0, bool *isConst = 0, const char **configGroup = 0, void **pointer = 0, asDWORD *accessMask = 0) const = 0;
	virtual int    GetGlobalPropertyIndexByName(const char *name) const = 0;
	virtual int    GetGlobalPropertyIndexByDecl(const char *decl) const = 0;

	// Object types
	virtual int            RegisterObjectType(const char *obj, int byteSize, asQWORD flags) = 0;
	virtual int            RegisterObjectProperty(const char *obj, const char *declaration, int byteOffset, int compositeOffset = 0, bool isCompositeIndirect = false) = 0;
	virtual int            RegisterObjectMethod(const char *obj, const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *auxiliary = 0, int compositeOffset = 0, bool isCompositeIndirect = false) = 0;
	virtual int            RegisterObjectBehaviour(const char *obj, asEBehaviours behaviour, const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *auxiliary = 0, int compositeOffset = 0, bool isCompositeIndirect = false) = 0;
	virtual int            RegisterInterface(const char *name) = 0;
	virtual int            RegisterInterfaceMethod(const char *intf, const char *declaration) = 0;
	virtual asUINT         GetObjectTypeCount() const = 0;
	virtual asITypeInfo   *GetObjectTypeByIndex(asUINT index) const = 0;

	// String factory
	virtual int RegisterStringFactory(const char *datatype, asIStringFactory *factory) = 0;
	virtual int GetStringFactory(asDWORD* typeModifiers = 0, asIStringFactory** factory = 0) const = 0;
#ifdef AS_DEPRECATED
	// deprecated since 2024-07-27, 2.38.0
	virtual int GetStringFactoryReturnTypeId(asDWORD *flags = 0) const = 0;
#endif

	// Default array type
	virtual int RegisterDefaultArrayType(const char *type) = 0;
	virtual int GetDefaultArrayTypeId() const = 0;

	// Enums
	virtual int          RegisterEnum(const char *type) = 0;
	virtual int          RegisterEnumValue(const char *type, const char *name, int value) = 0;
	virtual asUINT       GetEnumCount() const = 0;
	virtual asITypeInfo *GetEnumByIndex(asUINT index) const = 0;

	// Funcdefs
	virtual int          RegisterFuncdef(const char *decl) = 0;
	virtual asUINT       GetFuncdefCount() const = 0;
	virtual asITypeInfo *GetFuncdefByIndex(asUINT index) const = 0;

	// Typedefs
	virtual int          RegisterTypedef(const char *type, const char *decl) = 0;
	virtual asUINT       GetTypedefCount() const = 0;
	virtual asITypeInfo *GetTypedefByIndex(asUINT index) const = 0;

	// Configuration groups
	virtual int         BeginConfigGroup(const char *groupName) = 0;
	virtual int         EndConfigGroup() = 0;
	virtual int         RemoveConfigGroup(const char *groupName) = 0;
	virtual asDWORD     SetDefaultAccessMask(asDWORD defaultMask) = 0;
	virtual int         SetDefaultNamespace(const char *nameSpace) = 0;
	virtual const char *GetDefaultNamespace() const = 0;

	// Script modules
	virtual asIScriptModule *GetModule(const char *module, asEGMFlags flag = asGM_ONLY_IF_EXISTS) = 0;
	virtual int              DiscardModule(const char *module) = 0;
	virtual asUINT           GetModuleCount() const = 0;
	virtual asIScriptModule *GetModuleByIndex(asUINT index) const = 0;

	// Script functions
	virtual int                GetLastFunctionId() const = 0;
	virtual asIScriptFunction *GetFunctionById(int funcId) const = 0;

	// Type identification
	virtual int            GetTypeIdByDecl(const char *decl) const = 0;
	virtual const char    *GetTypeDeclaration(int typeId, bool includeNamespace = false) const = 0;
	virtual int            GetSizeOfPrimitiveType(int typeId) const = 0;
	virtual asITypeInfo   *GetTypeInfoById(int typeId) const = 0;
	virtual asITypeInfo   *GetTypeInfoByName(const char *name) const = 0;
	virtual asITypeInfo   *GetTypeInfoByDecl(const char *decl) const = 0;

	// Script execution
	virtual asIScriptContext      *CreateContext() = 0;
	virtual void                  *CreateScriptObject(const asITypeInfo *type) = 0;
	virtual void                  *CreateScriptObjectCopy(void *obj, const asITypeInfo *type) = 0;
	virtual void                  *CreateUninitializedScriptObject(const asITypeInfo *type) = 0;
	virtual asIScriptFunction     *CreateDelegate(asIScriptFunction *func, void *obj) = 0;
	virtual int                    AssignScriptObject(void *dstObj, void *srcObj, const asITypeInfo *type) = 0;
	virtual void                   ReleaseScriptObject(void *obj, const asITypeInfo *type) = 0;
	virtual void                   AddRefScriptObject(void *obj, const asITypeInfo *type) = 0;
	virtual int                    RefCastObject(void *obj, asITypeInfo *fromType, asITypeInfo *toType, void **newPtr, bool useOnlyImplicitCast = false) = 0;
	virtual asILockableSharedBool *GetWeakRefFlagOfScriptObject(void *obj, const asITypeInfo *type) const = 0;

	// Context pooling
	virtual asIScriptContext      *RequestContext() = 0;
	virtual void                   ReturnContext(asIScriptContext *ctx) = 0;
	virtual int                    SetContextCallbacks(asREQUESTCONTEXTFUNC_t requestCtx, asRETURNCONTEXTFUNC_t returnCtx, void *param = 0) = 0;

	// String interpretation
	virtual asETokenClass ParseToken(const char *string, size_t stringLength = 0, asUINT *tokenLength = 0) const = 0;

	// Garbage collection
	virtual int  GarbageCollect(asDWORD flags = asGC_FULL_CYCLE, asUINT numIterations = 1) = 0;
	virtual void GetGCStatistics(asUINT *currentSize, asUINT *totalDestroyed = 0, asUINT *totalDetected = 0, asUINT *newObjects = 0, asUINT *totalNewDestroyed = 0) const = 0;
	virtual int  NotifyGarbageCollectorOfNewObject(void *obj, asITypeInfo *type) = 0;
	virtual int  GetObjectInGC(asUINT idx, asUINT *seqNbr = 0, void **obj = 0, asITypeInfo **type = 0) = 0;
	virtual void GCEnumCallback(void *reference) = 0;
	virtual void ForwardGCEnumReferences(void *ref, asITypeInfo *type) = 0;
	virtual void ForwardGCReleaseReferences(void *ref, asITypeInfo *type) = 0;
	virtual void SetCircularRefDetectedCallback(asCIRCULARREFFUNC_t callback, void *param = 0) = 0;

	// User data
	virtual void *SetUserData(void *data, asPWORD type = 0) = 0;
	virtual void *GetUserData(asPWORD type = 0) const = 0;
	virtual void  SetEngineUserDataCleanupCallback(asCLEANENGINEFUNC_t callback, asPWORD type = 0) = 0;
	virtual void  SetModuleUserDataCleanupCallback(asCLEANMODULEFUNC_t callback, asPWORD type = 0) = 0;
	virtual void  SetContextUserDataCleanupCallback(asCLEANCONTEXTFUNC_t callback, asPWORD type = 0) = 0;
	virtual void  SetFunctionUserDataCleanupCallback(asCLEANFUNCTIONFUNC_t callback, asPWORD type = 0) = 0;
	virtual void  SetTypeInfoUserDataCleanupCallback(asCLEANTYPEINFOFUNC_t callback, asPWORD type = 0) = 0;
	virtual void  SetScriptObjectUserDataCleanupCallback(asCLEANSCRIPTOBJECTFUNC_t callback, asPWORD type = 0) = 0;

	// Exception handling
	virtual int SetTranslateAppExceptionCallback(const asSFuncPtr &callback, void *param, int callConv) = 0;

protected:
	virtual ~asIScriptEngine() {}
};

class asIStringFactory
{
public:
	virtual const void *GetStringConstant(const char *data, asUINT length) = 0;
	virtual int         ReleaseStringConstant(const void *str) = 0;
	virtual int         GetRawStringData(const void *str, char *data, asUINT *length) const = 0;

	// The destructor doesn't have to be protected as the string factory is not necessarily reference counted
	virtual ~asIStringFactory() {}
};

class asIThreadManager
{
protected:
	virtual ~asIThreadManager() {}
};

class asIScriptModule
{
public:
	virtual asIScriptEngine *GetEngine() const = 0;
	virtual void             SetName(const char *name) = 0;
	virtual const char      *GetName() const = 0;
	virtual void             Discard() = 0;

	// Compilation
	virtual int         AddScriptSection(const char *name, const char *code, size_t codeLength = 0, int lineOffset = 0) = 0;
	virtual int         Build() = 0;
	virtual int         CompileFunction(const char *sectionName, const char *code, int lineOffset, asDWORD compileFlags, asIScriptFunction **outFunc) = 0;
	virtual int         CompileGlobalVar(const char *sectionName, const char *code, int lineOffset) = 0;
	virtual asDWORD     SetAccessMask(asDWORD accessMask) = 0;
	virtual int         SetDefaultNamespace(const char *nameSpace) = 0;
	virtual const char *GetDefaultNamespace() const = 0;

	// Functions
	virtual asUINT             GetFunctionCount() const = 0;
	virtual asIScriptFunction *GetFunctionByIndex(asUINT index) const = 0;
	virtual asIScriptFunction *GetFunctionByDecl(const char *decl) const = 0;
	virtual asIScriptFunction *GetFunctionByName(const char *name) const = 0;
	virtual int                RemoveFunction(asIScriptFunction *func) = 0;

	// Global variables
	virtual int         ResetGlobalVars(asIScriptContext *ctx = 0) = 0;
	virtual asUINT      GetGlobalVarCount() const = 0;
	virtual int         GetGlobalVarIndexByName(const char *name) const = 0;
	virtual int         GetGlobalVarIndexByDecl(const char *decl) const = 0;
	virtual const char *GetGlobalVarDeclaration(asUINT index, bool includeNamespace = false) const = 0;
	virtual int         GetGlobalVar(asUINT index, const char **name, const char **nameSpace = 0, int *typeId = 0, bool *isConst = 0) const = 0;
	virtual void       *GetAddressOfGlobalVar(asUINT index) = 0;
	virtual int         RemoveGlobalVar(asUINT index) = 0;

	// Type identification
	virtual asUINT         GetObjectTypeCount() const = 0;
	virtual asITypeInfo   *GetObjectTypeByIndex(asUINT index) const = 0;
	virtual int            GetTypeIdByDecl(const char *decl) const = 0;
	virtual asITypeInfo   *GetTypeInfoByName(const char *name) const = 0;
	virtual asITypeInfo   *GetTypeInfoByDecl(const char *decl) const = 0;

	// Enums
	virtual asUINT       GetEnumCount() const = 0;
	virtual asITypeInfo *GetEnumByIndex(asUINT index) const = 0;

	// Typedefs
	virtual asUINT       GetTypedefCount() const = 0;
	virtual asITypeInfo *GetTypedefByIndex(asUINT index) const = 0;

	// Dynamic binding between modules
	virtual asUINT      GetImportedFunctionCount() const = 0;
	virtual int         GetImportedFunctionIndexByDecl(const char *decl) const = 0;
	virtual const char *GetImportedFunctionDeclaration(asUINT importIndex) const = 0;
	virtual const char *GetImportedFunctionSourceModule(asUINT importIndex) const = 0;
	virtual int         BindImportedFunction(asUINT importIndex, asIScriptFunction *func) = 0;
	virtual int         UnbindImportedFunction(asUINT importIndex) = 0;
	virtual int         BindAllImportedFunctions() = 0;
	virtual int         UnbindAllImportedFunctions() = 0;

	// Byte code saving and loading
	virtual int SaveByteCode(asIBinaryStream *out, bool stripDebugInfo = false) const = 0;
	virtual int LoadByteCode(asIBinaryStream *in, bool *wasDebugInfoStripped = 0) = 0;

	// User data
	virtual void *SetUserData(void *data, asPWORD type = 0) = 0;
	virtual void *GetUserData(asPWORD type = 0) const = 0;

protected:
	virtual ~asIScriptModule() {}
};

class asIScriptContext
{
public:
	// Memory management
	virtual int AddRef() const = 0;
	virtual int Release() const = 0;

	// Miscellaneous
	virtual asIScriptEngine *GetEngine() const = 0;

	// Execution
	virtual int             Prepare(asIScriptFunction *func) = 0;
	virtual int             Unprepare() = 0;
	virtual int             Execute() = 0;
	virtual int             Abort() = 0;
	virtual int             Suspend() = 0;
	virtual asEContextState GetState() const = 0;
	virtual int             PushState() = 0;
	virtual int             PopState() = 0;
	virtual bool            IsNested(asUINT *nestCount = 0) const = 0;

	// Object pointer for calling class methods
	virtual int   SetObject(void *obj) = 0;

	// Arguments
	virtual int   SetArgByte(asUINT arg, asBYTE value) = 0;
	virtual int   SetArgWord(asUINT arg, asWORD value) = 0;
	virtual int   SetArgDWord(asUINT arg, asDWORD value) = 0;
	virtual int   SetArgQWord(asUINT arg, asQWORD value) = 0;
	virtual int   SetArgFloat(asUINT arg, float value) = 0;
	virtual int   SetArgDouble(asUINT arg, double value) = 0;
	virtual int   SetArgAddress(asUINT arg, void *addr) = 0;
	virtual int   SetArgObject(asUINT arg, void *obj) = 0;
	virtual int   SetArgVarType(asUINT arg, void *ptr, int typeId) = 0;
	virtual void *GetAddressOfArg(asUINT arg) = 0;

	// Return value
	virtual asBYTE  GetReturnByte() = 0;
	virtual asWORD  GetReturnWord() = 0;
	virtual asDWORD GetReturnDWord() = 0;
	virtual asQWORD GetReturnQWord() = 0;
	virtual float   GetReturnFloat() = 0;
	virtual double  GetReturnDouble() = 0;
	virtual void   *GetReturnAddress() = 0;
	virtual void   *GetReturnObject() = 0;
	virtual void   *GetAddressOfReturnValue() = 0;

	// Exception handling
	virtual int                SetException(const char *info, bool allowCatch = true) = 0;
	virtual int                GetExceptionLineNumber(int *column = 0, const char **sectionName = 0) = 0;
	virtual asIScriptFunction *GetExceptionFunction() = 0;
	virtual const char *       GetExceptionString() = 0;
	virtual bool               WillExceptionBeCaught() = 0;
	virtual int                SetExceptionCallback(const asSFuncPtr &callback, void *obj, int callConv) = 0;
	virtual void               ClearExceptionCallback() = 0;

	// Debugging
	virtual int                SetLineCallback(const asSFuncPtr &callback, void *obj, int callConv) = 0;
	virtual void               ClearLineCallback() = 0;
	virtual asUINT             GetCallstackSize() const = 0;
	virtual asIScriptFunction *GetFunction(asUINT stackLevel = 0) = 0;
	virtual int                GetLineNumber(asUINT stackLevel = 0, int *column = 0, const char **sectionName = 0) = 0;
	virtual int                GetVarCount(asUINT stackLevel = 0) = 0;
	virtual int                GetVar(asUINT varIndex, asUINT stackLevel, const char** name, int* typeId = 0, asETypeModifiers* typeModifiers = 0, bool* isVarOnHeap = 0, int* stackOffset = 0) = 0;
#ifdef AS_DEPRECATED
	// deprecated since 2022-05-04, 2.36.0
	virtual const char        *GetVarName(asUINT varIndex, asUINT stackLevel = 0) = 0;
#endif
	virtual const char        *GetVarDeclaration(asUINT varIndex, asUINT stackLevel = 0, bool includeNamespace = false) = 0;
#ifdef AS_DEPRECATED
	// deprecated since 2022-05-04, 2.36.0
	virtual int                GetVarTypeId(asUINT varIndex, asUINT stackLevel = 0) = 0;
#endif
	virtual void              *GetAddressOfVar(asUINT varIndex, asUINT stackLevel = 0, bool dontDereference = false, bool returnAddressOfUnitializedObjects = false) = 0;
	virtual bool               IsVarInScope(asUINT varIndex, asUINT stackLevel = 0) = 0;
	virtual int                GetThisTypeId(asUINT stackLevel = 0) = 0;
	virtual void              *GetThisPointer(asUINT stackLevel = 0) = 0;
	virtual asIScriptFunction *GetSystemFunction() = 0;

	// User data
	virtual void *SetUserData(void *data, asPWORD type = 0) = 0;
	virtual void *GetUserData(asPWORD type = 0) const = 0;

	// Serialization
	virtual int StartDeserialization() = 0;
	virtual int FinishDeserialization() = 0;
	virtual int PushFunction(asIScriptFunction *func, void *object) = 0;
	virtual int GetStateRegisters(asUINT stackLevel, asIScriptFunction **callingSystemFunction, asIScriptFunction **initialFunction, asDWORD *origStackPointer, asDWORD *argumentsSize, asQWORD *valueRegister, void **objectRegister, asITypeInfo **objectTypeRegister) = 0;
	virtual int GetCallStateRegisters(asUINT stackLevel, asDWORD *stackFramePointer, asIScriptFunction **currentFunction, asDWORD *programPointer, asDWORD *stackPointer, asDWORD *stackIndex) = 0;
	virtual int SetStateRegisters(asUINT stackLevel, asIScriptFunction *callingSystemFunction, asIScriptFunction *initialFunction, asDWORD origStackPointer, asDWORD argumentsSize, asQWORD valueRegister, void *objectRegister, asITypeInfo *objectTypeRegister) = 0;
	virtual int SetCallStateRegisters(asUINT stackLevel, asDWORD stackFramePointer, asIScriptFunction *currentFunction, asDWORD programPointer, asDWORD stackPointer, asDWORD stackIndex) = 0;
	virtual int GetArgsOnStackCount(asUINT stackLevel) = 0;
	virtual int GetArgOnStack(asUINT stackLevel, asUINT arg, int* typeId, asUINT *flags, void** address) = 0;

protected:
	virtual ~asIScriptContext() {}
};

class asIScriptGeneric
{
public:
	// Miscellaneous
	virtual asIScriptEngine   *GetEngine() const = 0;
	virtual asIScriptFunction *GetFunction() const = 0;
	virtual void              *GetAuxiliary() const = 0;

	// Object
	virtual void   *GetObject() = 0;
	virtual int     GetObjectTypeId() const = 0;

	// Arguments
	virtual int     GetArgCount() const = 0;
	virtual int     GetArgTypeId(asUINT arg, asDWORD *flags = 0) const = 0;
	virtual asBYTE  GetArgByte(asUINT arg) = 0;
	virtual asWORD  GetArgWord(asUINT arg) = 0;
	virtual asDWORD GetArgDWord(asUINT arg) = 0;
	virtual asQWORD GetArgQWord(asUINT arg) = 0;
	virtual float   GetArgFloat(asUINT arg) = 0;
	virtual double  GetArgDouble(asUINT arg) = 0;
	virtual void   *GetArgAddress(asUINT arg) = 0;
	virtual void   *GetArgObject(asUINT arg) = 0;
	virtual void   *GetAddressOfArg(asUINT arg) = 0;

	// Return value
	virtual int     GetReturnTypeId(asDWORD *flags = 0) const = 0;
	virtual int     SetReturnByte(asBYTE val) = 0;
	virtual int     SetReturnWord(asWORD val) = 0;
	virtual int     SetReturnDWord(asDWORD val) = 0;
	virtual int     SetReturnQWord(asQWORD val) = 0;
	virtual int     SetReturnFloat(float val) = 0;
	virtual int     SetReturnDouble(double val) = 0;
	virtual int     SetReturnAddress(void *addr) = 0;
	virtual int     SetReturnObject(void *obj) = 0;
	virtual void   *GetAddressOfReturnLocation() = 0;

protected:
	virtual ~asIScriptGeneric() {}
};

class asIScriptObject
{
public:
	// Memory management
	virtual int                    AddRef() const = 0;
	virtual int                    Release() const = 0;
	virtual asILockableSharedBool *GetWeakRefFlag() const = 0;

	// Type info
	virtual int            GetTypeId() const = 0;
	virtual asITypeInfo   *GetObjectType() const = 0;

	// Class properties
	virtual asUINT      GetPropertyCount() const = 0;
	virtual int         GetPropertyTypeId(asUINT prop) const = 0;
	virtual const char *GetPropertyName(asUINT prop) const = 0;
	virtual void       *GetAddressOfProperty(asUINT prop) = 0;

	// Miscellaneous
	virtual asIScriptEngine *GetEngine() const = 0;
	virtual int              CopyFrom(const asIScriptObject *other) = 0;

	// User data
	virtual void *SetUserData(void *data, asPWORD type = 0) = 0;
	virtual void *GetUserData(asPWORD type = 0) const = 0;

protected:
	virtual ~asIScriptObject() {}
};

class asITypeInfo
{
public:
	// Miscellaneous
	virtual asIScriptEngine *GetEngine() const = 0;
	virtual const char      *GetConfigGroup() const = 0;
	virtual asDWORD          GetAccessMask() const = 0;
	virtual asIScriptModule *GetModule() const = 0;

	// Memory management
	virtual int AddRef() const = 0;
	virtual int Release() const = 0;

	// Type info
	virtual const char      *GetName() const = 0;
	virtual	const char      *GetNamespace() const = 0;
	virtual asITypeInfo     *GetBaseType() const = 0;
	virtual bool             DerivesFrom(const asITypeInfo *objType) const = 0;
	virtual asQWORD          GetFlags() const = 0;
	virtual asUINT           GetSize() const = 0;
	virtual int              GetTypeId() const = 0;
	virtual int              GetSubTypeId(asUINT subTypeIndex = 0) const = 0;
	virtual asITypeInfo     *GetSubType(asUINT subTypeIndex = 0) const = 0;
	virtual asUINT           GetSubTypeCount() const = 0;

	// Interfaces
	virtual asUINT           GetInterfaceCount() const = 0;
	virtual asITypeInfo     *GetInterface(asUINT index) const = 0;
	virtual bool             Implements(const asITypeInfo *objType) const = 0;

	// Factories
	virtual asUINT             GetFactoryCount() const = 0;
	virtual asIScriptFunction *GetFactoryByIndex(asUINT index) const = 0;
	virtual asIScriptFunction *GetFactoryByDecl(const char *decl) const = 0;

	// Methods
	virtual asUINT             GetMethodCount() const = 0;
	virtual asIScriptFunction *GetMethodByIndex(asUINT index, bool getVirtual = true) const = 0;
	virtual asIScriptFunction *GetMethodByName(const char *name, bool getVirtual = true) const = 0;
	virtual asIScriptFunction *GetMethodByDecl(const char *decl, bool getVirtual = true) const = 0;

	// Properties
	virtual asUINT      GetPropertyCount() const = 0;
	virtual int         GetProperty(asUINT index, const char **name, int *typeId = 0, bool *isPrivate = 0, bool *isProtected = 0, int *offset = 0, bool *isReference = 0, asDWORD *accessMask = 0, int *compositeOffset = 0, bool *isCompositeIndirect = 0, bool *isConst = 0) const = 0;
	virtual const char *GetPropertyDeclaration(asUINT index, bool includeNamespace = false) const = 0;

	// Behaviours
	virtual asUINT             GetBehaviourCount() const = 0;
	virtual asIScriptFunction *GetBehaviourByIndex(asUINT index, asEBehaviours *outBehaviour) const = 0;

	// Child types
	virtual asUINT       GetChildFuncdefCount() const = 0;
	virtual asITypeInfo *GetChildFuncdef(asUINT index) const = 0;
	virtual asITypeInfo *GetParentType() const = 0;

	// Enums
	virtual asUINT      GetEnumValueCount() const = 0;
	virtual const char *GetEnumValueByIndex(asUINT index, int *outValue) const = 0;

	// Typedef
	virtual int GetTypedefTypeId() const = 0;

	// Funcdef
	virtual asIScriptFunction *GetFuncdefSignature() const = 0;

	// User data
	virtual void *SetUserData(void *data, asPWORD type = 0) = 0;
	virtual void *GetUserData(asPWORD type = 0) const = 0;

protected:
	virtual ~asITypeInfo() {}
};

class asIScriptFunction
{
public:
	virtual asIScriptEngine *GetEngine() const = 0;

	// Memory management
	virtual int              AddRef() const = 0;
	virtual int              Release() const = 0;

	// Miscellaneous
	virtual int              GetId() const = 0;
	virtual asEFuncType      GetFuncType() const = 0;
	virtual const char      *GetModuleName() const = 0;
	virtual asIScriptModule *GetModule() const = 0;
#ifdef AS_DEPRECATED
	// deprecated since 2025-04-25, 2.38.0
	virtual const char      *GetScriptSectionName() const = 0;
#endif
	virtual const char      *GetConfigGroup() const = 0;
	virtual asDWORD          GetAccessMask() const = 0;
	virtual void            *GetAuxiliary() const = 0;

	// Function signature
	virtual asITypeInfo     *GetObjectType() const = 0;
	virtual const char      *GetObjectName() const = 0;
	virtual const char      *GetName() const = 0;
	virtual const char      *GetNamespace() const = 0;
	virtual const char      *GetDeclaration(bool includeObjectName = true, bool includeNamespace = false, bool includeParamNames = false) const = 0;
	virtual bool             IsReadOnly() const = 0;
	virtual bool             IsPrivate() const = 0;
	virtual bool             IsProtected() const = 0;
	virtual bool             IsFinal() const = 0;
	virtual bool             IsOverride() const = 0;
	virtual bool             IsShared() const = 0;
	virtual bool             IsExplicit() const = 0;
	virtual bool             IsProperty() const = 0;
	virtual bool             IsVariadic() const = 0;
	virtual asUINT           GetParamCount() const = 0;
	virtual int              GetParam(asUINT index, int *typeId, asDWORD *flags = 0, const char **name = 0, const char **defaultArg = 0) const = 0;
	virtual int              GetReturnTypeId(asDWORD *flags = 0) const = 0;

	// Template functions
	virtual asUINT           GetSubTypeCount() const = 0;
	virtual int              GetSubTypeId(asUINT subTypeIndex = 0) const = 0;
	virtual asITypeInfo     *GetSubType(asUINT subTypeIndex = 0) const = 0;

	// Type id for function pointers
	virtual int              GetTypeId() const = 0;
	virtual bool             IsCompatibleWithTypeId(int typeId) const = 0;

	// Delegates
	virtual void              *GetDelegateObject() const = 0;
	virtual asITypeInfo       *GetDelegateObjectType() const = 0;
	virtual asIScriptFunction *GetDelegateFunction() const = 0;

	// Debug information
	virtual asUINT           GetVarCount() const = 0;
	virtual int              GetVar(asUINT index, const char **name, int *typeId = 0) const = 0;
	virtual const char      *GetVarDecl(asUINT index, bool includeNamespace = false) const = 0;
	virtual int              FindNextLineWithCode(int line) const = 0;
	virtual int              GetDeclaredAt(const char** scriptSection, int* row, int* col) const = 0;

	// For JIT compilation
	virtual asDWORD         *GetByteCode(asUINT *length = 0) = 0;
	virtual int              SetJITFunction(asJITFunction jitFunc) = 0;
	virtual asJITFunction    GetJITFunction() const = 0;

	// User data
	virtual void            *SetUserData(void *userData, asPWORD type = 0) = 0;
	virtual void            *GetUserData(asPWORD type = 0) const = 0;

protected:
	virtual ~asIScriptFunction() {};
};

class asIBinaryStream
{
public:
	virtual int Read(void *ptr, asUINT size) = 0;
	virtual int Write(const void *ptr, asUINT size) = 0;

public:
	virtual ~asIBinaryStream() {}
};

class asILockableSharedBool
{
public:
	// Memory management
	virtual int AddRef() const = 0;
	virtual int Release() const = 0;

	// Value
	virtual bool Get() const = 0;
	virtual void Set(bool val) = 0;

	// Thread management
	virtual void Lock() const = 0;
	virtual void Unlock() const = 0;

protected:
	virtual ~asILockableSharedBool() {}
};

//-----------------------------------------------------------------
// Function pointers

// Template function to capture all global functions,
// except the ones using the generic calling convention
template <class T>
inline asSFuncPtr asFunctionPtr(T func)
{
	// Mark this as a global function
	asSFuncPtr p(2);

#ifdef AS_64BIT_PTR
	// The size_t cast is to avoid a compiler warning with asFUNCTION(0)
	// on 64bit, as 0 is interpreted as a 32bit int value
	p.ptr.f.func = reinterpret_cast<asFUNCTION_t>(size_t(func));
#else
	// MSVC6 doesn't like the size_t cast above so I
	// solved this with a separate code for 32bit.
	p.ptr.f.func = reinterpret_cast<asFUNCTION_t>(func);
#endif

	return p;
}

// Specialization for functions using the generic calling convention
template<>
inline asSFuncPtr asFunctionPtr<asGENFUNC_t>(asGENFUNC_t func)
{
	// Mark this as a generic function
	asSFuncPtr p(1);
	p.ptr.f.func = reinterpret_cast<asFUNCTION_t>(func);
	return p;
}

#ifndef AS_NO_CLASS_METHODS

// Method pointers

// Declare a dummy class so that we can determine the size of a simple method pointer
class asCSimpleDummy {};
typedef void (asCSimpleDummy::*asSIMPLEMETHOD_t)();
const int SINGLE_PTR_SIZE = sizeof(asSIMPLEMETHOD_t);

// Define template
template <int N>
struct asSMethodPtr
{
	template<class M>
	static asSFuncPtr Convert(M Mthd)
	{
		// This version of the function should never be executed, nor compiled,
		// as it would mean that the size of the method pointer cannot be determined.

		int ERROR_UnsupportedMethodPtr[N-100];

		asSFuncPtr p(0);
		return p;
	}
};

// Template specialization
template <>
struct asSMethodPtr<SINGLE_PTR_SIZE>
{
	template<class M>
	static asSFuncPtr Convert(M Mthd)
	{
		// Mark this as a class method
		asSFuncPtr p(3);
		p.CopyMethodPtr(&Mthd, SINGLE_PTR_SIZE);
		return p;
	}
};

#if defined(_MSC_VER) && !defined(__MWERKS__)

// MSVC and Intel uses different sizes for different class method pointers
template <>
struct asSMethodPtr<SINGLE_PTR_SIZE+1*sizeof(int)>
{
	template <class M>
	static asSFuncPtr Convert(M Mthd)
	{
		// Mark this as a class method
		asSFuncPtr p(3);
		p.CopyMethodPtr(&Mthd, SINGLE_PTR_SIZE+sizeof(int));
		return p;
	}
};

template <>
struct asSMethodPtr<SINGLE_PTR_SIZE+2*sizeof(int)>
{
	template <class M>
	static asSFuncPtr Convert(M Mthd)
	{
		// On 32bit platforms with is where a class with virtual inheritance falls.
		// On 64bit platforms we can also fall here if 8byte data alignments is used.

		// Mark this as a class method
		asSFuncPtr p(3);
		p.CopyMethodPtr(&Mthd, SINGLE_PTR_SIZE+2*sizeof(int));

		// Microsoft has a terrible optimization on class methods with virtual inheritance.
		// They are hardcoding an important offset, which is not coming in the method pointer.

#if defined(_MSC_VER) && !defined(AS_64BIT_PTR)
			// Method pointers for virtual inheritance is not supported,
			// as it requires the location of the vbase table, which is
			// only available to the C++ compiler, but not in the method
			// pointer.

			// You can get around this by forward declaring the class and
			// storing the sizeof its method pointer in a constant. Example:

			// class ClassWithVirtualInheritance;
			// const int ClassWithVirtualInheritance_workaround = sizeof(void ClassWithVirtualInheritance::*());

			// This will force the compiler to use the unknown type
			// for the class, which falls under the next case


			// Copy the virtual table index to the 4th dword so that AngelScript
			// can properly detect and deny the use of methods with virtual inheritance.
			*(reinterpret_cast<asDWORD*>(&p)+3) = *(reinterpret_cast<asDWORD*>(&p)+2);
#endif

		return p;
	}
};

template <>
struct asSMethodPtr<SINGLE_PTR_SIZE+3*sizeof(int)>
{
	template <class M>
	static asSFuncPtr Convert(M Mthd)
	{
		// Mark this as a class method
		asSFuncPtr p(3);
		p.CopyMethodPtr(&Mthd, SINGLE_PTR_SIZE+3*sizeof(int));
		return p;
	}
};

template <>
struct asSMethodPtr<SINGLE_PTR_SIZE+4*sizeof(int)>
{
	template <class M>
	static asSFuncPtr Convert(M Mthd)
	{
		// On 64bit platforms with 8byte data alignment
		// the unknown class method pointers will come here.

		// Mark this as a class method
		asSFuncPtr p(3);
		p.CopyMethodPtr(&Mthd, SINGLE_PTR_SIZE+4*sizeof(int));
		return p;
	}
};

#endif

#endif // AS_NO_CLASS_METHODS

//----------------------------------------------------------------
// JIT compiler

struct asSVMRegisters
{
	asDWORD          *programPointer;     // points to current bytecode instruction
	asDWORD          *stackFramePointer;  // function stack frame
	asDWORD          *stackPointer;       // top of stack (grows downward)
	asQWORD           valueRegister;      // temp register for primitives
	void             *objectRegister;     // temp register for objects and handles
	asITypeInfo      *objectType;         // type of object held in object register
	bool              doProcessSuspend;   // whether or not the JIT should break out when it encounters a suspend instruction
	asIScriptContext *ctx;                // the active context
};

class asIJITCompilerAbstract 
{ 
public: 
	virtual ~asIJITCompilerAbstract() {}
};

// JIT Compiler interface version 1
class asIJITCompiler : public asIJITCompilerAbstract
{
public:
	virtual int  CompileFunction(asIScriptFunction *function, asJITFunction *output) = 0;
	virtual void ReleaseJITFunction(asJITFunction func) = 0;
public:
	virtual ~asIJITCompiler() {}
};

// JIT Compiler interface version 2
class asIJITCompilerV2 : public asIJITCompilerAbstract
{
public:
	virtual void NewFunction(asIScriptFunction* scriptFunc) = 0;
	virtual void CleanFunction(asIScriptFunction *scriptFunc, asJITFunction jitFunc) = 0;
public:
	virtual ~asIJITCompilerV2() {}
};

// Byte code instructions
enum asEBCInstr
{
	asBC_PopPtr			= 0,
	asBC_PshGPtr		= 1,
	asBC_PshC4			= 2,
	asBC_PshV4			= 3,
	asBC_PSF			= 4,
	asBC_SwapPtr		= 5,
	asBC_NOT			= 6,
	asBC_PshG4			= 7,
	asBC_LdGRdR4		= 8,
	asBC_CALL			= 9,
	asBC_RET			= 10,
	asBC_JMP			= 11,
	asBC_JZ				= 12,
	asBC_JNZ			= 13,
	asBC_JS				= 14,
	asBC_JNS			= 15,
	asBC_JP				= 16,
	asBC_JNP			= 17,
	asBC_TZ				= 18,
	asBC_TNZ			= 19,
	asBC_TS				= 20,
	asBC_TNS			= 21,
	asBC_TP				= 22,
	asBC_TNP			= 23,
	asBC_NEGi			= 24,
	asBC_NEGf			= 25,
	asBC_NEGd			= 26,
	asBC_INCi16			= 27,
	asBC_INCi8			= 28,
	asBC_DECi16			= 29,
	asBC_DECi8			= 30,
	asBC_INCi			= 31,
	asBC_DECi			= 32,
	asBC_INCf			= 33,
	asBC_DECf			= 34,
	asBC_INCd			= 35,
	asBC_DECd			= 36,
	asBC_IncVi			= 37,
	asBC_DecVi			= 38,
	asBC_BNOT			= 39,
	asBC_BAND			= 40,
	asBC_BOR			= 41,
	asBC_BXOR			= 42,
	asBC_BSLL			= 43,
	asBC_BSRL			= 44,
	asBC_BSRA			= 45,
	asBC_COPY			= 46,
	asBC_PshC8			= 47,
	asBC_PshVPtr		= 48,
	asBC_RDSPtr			= 49,
	asBC_CMPd			= 50,
	asBC_CMPu			= 51,
	asBC_CMPf			= 52,
	asBC_CMPi			= 53,
	asBC_CMPIi			= 54,
	asBC_CMPIf			= 55,
	asBC_CMPIu			= 56,
	asBC_JMPP			= 57,
	asBC_PopRPtr		= 58,
	asBC_PshRPtr		= 59,
	asBC_STR			= 60,
	asBC_CALLSYS		= 61,
	asBC_CALLBND		= 62,
	asBC_SUSPEND		= 63,
	asBC_ALLOC			= 64,
	asBC_FREE			= 65,
	asBC_LOADOBJ		= 66,
	asBC_STOREOBJ		= 67,
	asBC_GETOBJ			= 68,
	asBC_REFCPY			= 69,
	asBC_CHKREF			= 70,
	asBC_GETOBJREF		= 71,
	asBC_GETREF			= 72,
	asBC_PshNull		= 73,
	asBC_ClrVPtr		= 74,
	asBC_OBJTYPE		= 75,
	asBC_TYPEID			= 76,
	asBC_SetV4			= 77,
	asBC_SetV8			= 78,
	asBC_ADDSi			= 79,
	asBC_CpyVtoV4		= 80,
	asBC_CpyVtoV8		= 81,
	asBC_CpyVtoR4		= 82,
	asBC_CpyVtoR8		= 83,
	asBC_CpyVtoG4		= 84,
	asBC_CpyRtoV4		= 85,
	asBC_CpyRtoV8		= 86,
	asBC_CpyGtoV4		= 87,
	asBC_WRTV1			= 88,
	asBC_WRTV2			= 89,
	asBC_WRTV4			= 90,
	asBC_WRTV8			= 91,
	asBC_RDR1			= 92,
	asBC_RDR2			= 93,
	asBC_RDR4			= 94,
	asBC_RDR8			= 95,
	asBC_LDG			= 96,
	asBC_LDV			= 97,
	asBC_PGA			= 98,
	asBC_CmpPtr			= 99,
	asBC_VAR			= 100,
	asBC_iTOf			= 101,
	asBC_fTOi			= 102,
	asBC_uTOf			= 103,
	asBC_fTOu			= 104,
	asBC_sbTOi			= 105,
	asBC_swTOi			= 106,
	asBC_ubTOi			= 107,
	asBC_uwTOi			= 108,
	asBC_dTOi			= 109,
	asBC_dTOu			= 110,
	asBC_dTOf			= 111,
	asBC_iTOd			= 112,
	asBC_uTOd			= 113,
	asBC_fTOd			= 114,
	asBC_ADDi			= 115,
	asBC_SUBi			= 116,
	asBC_MULi			= 117,
	asBC_DIVi			= 118,
	asBC_MODi			= 119,
	asBC_ADDf			= 120,
	asBC_SUBf			= 121,
	asBC_MULf			= 122,
	asBC_DIVf			= 123,
	asBC_MODf			= 124,
	asBC_ADDd			= 125,
	asBC_SUBd			= 126,
	asBC_MULd			= 127,
	asBC_DIVd			= 128,
	asBC_MODd			= 129,
	asBC_ADDIi			= 130,
	asBC_SUBIi			= 131,
	asBC_MULIi			= 132,
	asBC_ADDIf			= 133,
	asBC_SUBIf			= 134,
	asBC_MULIf			= 135,
	asBC_SetG4			= 136,
	asBC_ChkRefS		= 137,
	asBC_ChkNullV		= 138,
	asBC_CALLINTF		= 139,
	asBC_iTOb			= 140,
	asBC_iTOw			= 141,
	asBC_SetV1			= 142,
	asBC_SetV2			= 143,
	asBC_Cast			= 144,
	asBC_i64TOi			= 145,
	asBC_uTOi64			= 146,
	asBC_iTOi64			= 147,
	asBC_fTOi64			= 148,
	asBC_dTOi64			= 149,
	asBC_fTOu64			= 150,
	asBC_dTOu64			= 151,
	asBC_i64TOf			= 152,
	asBC_u64TOf			= 153,
	asBC_i64TOd			= 154,
	asBC_u64TOd			= 155,
	asBC_NEGi64			= 156,
	asBC_INCi64			= 157,
	asBC_DECi64			= 158,
	asBC_BNOT64			= 159,
	asBC_ADDi64			= 160,
	asBC_SUBi64			= 161,
	asBC_MULi64			= 162,
	asBC_DIVi64			= 163,
	asBC_MODi64			= 164,
	asBC_BAND64			= 165,
	asBC_BOR64			= 166,
	asBC_BXOR64			= 167,
	asBC_BSLL64			= 168,
	asBC_BSRL64			= 169,
	asBC_BSRA64			= 170,
	asBC_CMPi64			= 171,
	asBC_CMPu64			= 172,
	asBC_ChkNullS		= 173,
	asBC_ClrHi			= 174,
	asBC_JitEntry		= 175,
	asBC_CallPtr		= 176,
	asBC_FuncPtr		= 177,
	asBC_LoadThisR		= 178,
	asBC_PshV8			= 179,
	asBC_DIVu			= 180,
	asBC_MODu			= 181,
	asBC_DIVu64			= 182,
	asBC_MODu64			= 183,
	asBC_LoadRObjR		= 184,
	asBC_LoadVObjR		= 185,
	asBC_RefCpyV		= 186,
	asBC_JLowZ			= 187,
	asBC_JLowNZ			= 188,
	asBC_AllocMem		= 189,
	asBC_SetListSize	= 190,
	asBC_PshListElmnt	= 191,
	asBC_SetListType	= 192,
	asBC_POWi			= 193,
	asBC_POWu			= 194,
	asBC_POWf			= 195,
	asBC_POWd			= 196,
	asBC_POWdi			= 197,
	asBC_POWi64			= 198,
	asBC_POWu64			= 199,
	asBC_Thiscall1		= 200,
	asBC_MAXBYTECODE	= 201,

	// Temporary tokens. Can't be output to the final program
	asBC_TryBlock		= 250,
	asBC_VarDecl		= 251,
	asBC_Block			= 252,
	asBC_ObjInfo		= 253,
	asBC_LINE			= 254,
	asBC_LABEL			= 255
};

// Instruction types
enum asEBCType
{
	asBCTYPE_INFO         = 0,
	asBCTYPE_NO_ARG       = 1,
	asBCTYPE_W_ARG        = 2,
	asBCTYPE_wW_ARG       = 3,
	asBCTYPE_DW_ARG       = 4,
	asBCTYPE_rW_DW_ARG    = 5,
	asBCTYPE_QW_ARG       = 6,
	asBCTYPE_DW_DW_ARG    = 7,
	asBCTYPE_wW_rW_rW_ARG = 8,
	asBCTYPE_wW_QW_ARG    = 9,
	asBCTYPE_wW_rW_ARG    = 10,
	asBCTYPE_rW_ARG       = 11,
	asBCTYPE_wW_DW_ARG    = 12,
	asBCTYPE_wW_rW_DW_ARG = 13,
	asBCTYPE_rW_rW_ARG    = 14,
	asBCTYPE_wW_W_ARG     = 15,
	asBCTYPE_QW_DW_ARG    = 16,
	asBCTYPE_rW_QW_ARG    = 17,
	asBCTYPE_W_DW_ARG     = 18,
	asBCTYPE_rW_W_DW_ARG  = 19,
	asBCTYPE_rW_DW_DW_ARG = 20
};

// Instruction type sizes
const int asBCTypeSize[21] =
{
	0, // asBCTYPE_INFO
	1, // asBCTYPE_NO_ARG
	1, // asBCTYPE_W_ARG
	1, // asBCTYPE_wW_ARG
	2, // asBCTYPE_DW_ARG
	2, // asBCTYPE_rW_DW_ARG
	3, // asBCTYPE_QW_ARG
	3, // asBCTYPE_DW_DW_ARG
	2, // asBCTYPE_wW_rW_rW_ARG
	3, // asBCTYPE_wW_QW_ARG
	2, // asBCTYPE_wW_rW_ARG
	1, // asBCTYPE_rW_ARG
	2, // asBCTYPE_wW_DW_ARG
	3, // asBCTYPE_wW_rW_DW_ARG
	2, // asBCTYPE_rW_rW_ARG
	2, // asBCTYPE_wW_W_ARG
	4, // asBCTYPE_QW_DW_ARG
	3, // asBCTYPE_rW_QW_ARG
	2, // asBCTYPE_W_DW_ARG
	3, // asBCTYPE_rW_W_DW_ARG
	3  // asBCTYPE_rW_DW_DW_ARG
};

// Instruction info
struct asSBCInfo
{
	asEBCInstr  bc;
	asEBCType   type;
	int         stackInc;
	const char *name;
};

#ifndef AS_64BIT_PTR
	#define asBCTYPE_PTR_ARG    asBCTYPE_DW_ARG
	#define asBCTYPE_PTR_DW_ARG asBCTYPE_DW_DW_ARG
	#define asBCTYPE_wW_PTR_ARG asBCTYPE_wW_DW_ARG
	#define asBCTYPE_rW_PTR_ARG asBCTYPE_rW_DW_ARG
	#ifndef AS_PTR_SIZE
		#define AS_PTR_SIZE 1
	#endif
#else
	#define asBCTYPE_PTR_ARG    asBCTYPE_QW_ARG
	#define asBCTYPE_PTR_DW_ARG asBCTYPE_QW_DW_ARG
	#define asBCTYPE_wW_PTR_ARG asBCTYPE_wW_QW_ARG
	#define asBCTYPE_rW_PTR_ARG asBCTYPE_rW_QW_ARG
	#ifndef AS_PTR_SIZE
		#define AS_PTR_SIZE 2
	#endif
#endif

#define asBCINFO(b,t,s) {asBC_##b, asBCTYPE_##t, s, #b}
#define asBCINFO_DUMMY(b) {asBC_MAXBYTECODE, asBCTYPE_INFO, 0, "BC_" #b}

const asSBCInfo asBCInfo[256] =
{
	asBCINFO(PopPtr,	NO_ARG,			-AS_PTR_SIZE),
	asBCINFO(PshGPtr,	PTR_ARG,		AS_PTR_SIZE),
	asBCINFO(PshC4,		DW_ARG,			1),
	asBCINFO(PshV4,		rW_ARG,			1),
	asBCINFO(PSF,		rW_ARG,			AS_PTR_SIZE),
	asBCINFO(SwapPtr,	NO_ARG,			0),
	asBCINFO(NOT,		rW_ARG,			0),
	asBCINFO(PshG4,		PTR_ARG,		1),
	asBCINFO(LdGRdR4,	wW_PTR_ARG,		0),
	asBCINFO(CALL,		DW_ARG,			0xFFFF),
	asBCINFO(RET,		W_ARG,			0xFFFF),
	asBCINFO(JMP,		DW_ARG,			0),
	asBCINFO(JZ,		DW_ARG,			0),
	asBCINFO(JNZ,		DW_ARG,			0),
	asBCINFO(JS,		DW_ARG,			0),
	asBCINFO(JNS,		DW_ARG,			0),
	asBCINFO(JP,		DW_ARG,			0),
	asBCINFO(JNP,		DW_ARG,			0),
	asBCINFO(TZ,		NO_ARG,			0),
	asBCINFO(TNZ,		NO_ARG,			0),
	asBCINFO(TS,		NO_ARG,			0),
	asBCINFO(TNS,		NO_ARG,			0),
	asBCINFO(TP,		NO_ARG,			0),
	asBCINFO(TNP,		NO_ARG,			0),
	asBCINFO(NEGi,		rW_ARG,			0),
	asBCINFO(NEGf,		rW_ARG,			0),
	asBCINFO(NEGd,		rW_ARG,			0),
	asBCINFO(INCi16,	NO_ARG,			0),
	asBCINFO(INCi8,		NO_ARG,			0),
	asBCINFO(DECi16,	NO_ARG,			0),
	asBCINFO(DECi8,		NO_ARG,			0),
	asBCINFO(INCi,		NO_ARG,			0),
	asBCINFO(DECi,		NO_ARG,			0),
	asBCINFO(INCf,		NO_ARG,			0),
	asBCINFO(DECf,		NO_ARG,			0),
	asBCINFO(INCd,		NO_ARG,			0),
	asBCINFO(DECd,		NO_ARG,			0),
	asBCINFO(IncVi,		rW_ARG,			0),
	asBCINFO(DecVi,		rW_ARG,			0),
	asBCINFO(BNOT,		rW_ARG,			0),
	asBCINFO(BAND,		wW_rW_rW_ARG,	0),
	asBCINFO(BOR,		wW_rW_rW_ARG,	0),
	asBCINFO(BXOR,		wW_rW_rW_ARG,	0),
	asBCINFO(BSLL,		wW_rW_rW_ARG,	0),
	asBCINFO(BSRL,		wW_rW_rW_ARG,	0),
	asBCINFO(BSRA,		wW_rW_rW_ARG,	0),
	asBCINFO(COPY,		W_DW_ARG,		-AS_PTR_SIZE),
	asBCINFO(PshC8,		QW_ARG,			2),
	asBCINFO(PshVPtr,	rW_ARG,			AS_PTR_SIZE),
	asBCINFO(RDSPtr,	NO_ARG,			0),
	asBCINFO(CMPd,		rW_rW_ARG,		0),
	asBCINFO(CMPu,		rW_rW_ARG,		0),
	asBCINFO(CMPf,		rW_rW_ARG,		0),
	asBCINFO(CMPi,		rW_rW_ARG,		0),
	asBCINFO(CMPIi,		rW_DW_ARG,		0),
	asBCINFO(CMPIf,		rW_DW_ARG,		0),
	asBCINFO(CMPIu,		rW_DW_ARG,		0),
	asBCINFO(JMPP,		rW_ARG,			0),
	asBCINFO(PopRPtr,	NO_ARG,			-AS_PTR_SIZE),
	asBCINFO(PshRPtr,	NO_ARG,			AS_PTR_SIZE),
	asBCINFO(STR,		W_ARG,			1+AS_PTR_SIZE),
	asBCINFO(CALLSYS,	DW_ARG,			0xFFFF),
	asBCINFO(CALLBND,	DW_ARG,			0xFFFF),
	asBCINFO(SUSPEND,	NO_ARG,			0),
	asBCINFO(ALLOC,		PTR_DW_ARG,		0xFFFF),
	asBCINFO(FREE,		wW_PTR_ARG,		0),
	asBCINFO(LOADOBJ,	rW_ARG,			0),
	asBCINFO(STOREOBJ,	wW_ARG,			0),
	asBCINFO(GETOBJ,	W_ARG,			0),
	asBCINFO(REFCPY,	PTR_ARG,		-AS_PTR_SIZE),
	asBCINFO(CHKREF,	NO_ARG,			0),
	asBCINFO(GETOBJREF,	W_ARG,			0),
	asBCINFO(GETREF,	W_ARG,			0),
	asBCINFO(PshNull,	NO_ARG,			AS_PTR_SIZE),
	asBCINFO(ClrVPtr,	wW_ARG,			0),
	asBCINFO(OBJTYPE,	PTR_ARG,		AS_PTR_SIZE),
	asBCINFO(TYPEID,	DW_ARG,			1),
	asBCINFO(SetV4,		wW_DW_ARG,		0),
	asBCINFO(SetV8,		wW_QW_ARG,		0),
	asBCINFO(ADDSi,		W_DW_ARG,		0),
	asBCINFO(CpyVtoV4,	wW_rW_ARG,		0),
	asBCINFO(CpyVtoV8,	wW_rW_ARG,		0),
	asBCINFO(CpyVtoR4,	rW_ARG,			0),
	asBCINFO(CpyVtoR8,	rW_ARG,			0),
	asBCINFO(CpyVtoG4,	rW_PTR_ARG,		0),
	asBCINFO(CpyRtoV4,	wW_ARG,			0),
	asBCINFO(CpyRtoV8,	wW_ARG,			0),
	asBCINFO(CpyGtoV4,	wW_PTR_ARG,		0),
	asBCINFO(WRTV1,		rW_ARG,			0),
	asBCINFO(WRTV2,		rW_ARG,			0),
	asBCINFO(WRTV4,		rW_ARG,			0),
	asBCINFO(WRTV8,		rW_ARG,			0),
	asBCINFO(RDR1,		wW_ARG,			0),
	asBCINFO(RDR2,		wW_ARG,			0),
	asBCINFO(RDR4,		wW_ARG,			0),
	asBCINFO(RDR8,		wW_ARG,			0),
	asBCINFO(LDG,		PTR_ARG,		0),
	asBCINFO(LDV,		rW_ARG,			0),
	asBCINFO(PGA,		PTR_ARG,		AS_PTR_SIZE),
	asBCINFO(CmpPtr,	rW_rW_ARG,		0),
	asBCINFO(VAR,		rW_ARG,			AS_PTR_SIZE),
	asBCINFO(iTOf,		rW_ARG,			0),
	asBCINFO(fTOi,		rW_ARG,			0),
	asBCINFO(uTOf,		rW_ARG,			0),
	asBCINFO(fTOu,		rW_ARG,			0),
	asBCINFO(sbTOi,		rW_ARG,			0),
	asBCINFO(swTOi,		rW_ARG,			0),
	asBCINFO(ubTOi,		rW_ARG,			0),
	asBCINFO(uwTOi,		rW_ARG,			0),
	asBCINFO(dTOi,		wW_rW_ARG,		0),
	asBCINFO(dTOu,		wW_rW_ARG,		0),
	asBCINFO(dTOf,		wW_rW_ARG,		0),
	asBCINFO(iTOd,		wW_rW_ARG,		0),
	asBCINFO(uTOd,		wW_rW_ARG,		0),
	asBCINFO(fTOd,		wW_rW_ARG,		0),
	asBCINFO(ADDi,		wW_rW_rW_ARG,	0),
	asBCINFO(SUBi,		wW_rW_rW_ARG,	0),
	asBCINFO(MULi,		wW_rW_rW_ARG,	0),
	asBCINFO(DIVi,		wW_rW_rW_ARG,	0),
	asBCINFO(MODi,		wW_rW_rW_ARG,	0),
	asBCINFO(ADDf,		wW_rW_rW_ARG,	0),
	asBCINFO(SUBf,		wW_rW_rW_ARG,	0),
	asBCINFO(MULf,		wW_rW_rW_ARG,	0),
	asBCINFO(DIVf,		wW_rW_rW_ARG,	0),
	asBCINFO(MODf,		wW_rW_rW_ARG,	0),
	asBCINFO(ADDd,		wW_rW_rW_ARG,	0),
	asBCINFO(SUBd,		wW_rW_rW_ARG,	0),
	asBCINFO(MULd,		wW_rW_rW_ARG,	0),
	asBCINFO(DIVd,		wW_rW_rW_ARG,	0),
	asBCINFO(MODd,		wW_rW_rW_ARG,	0),
	asBCINFO(ADDIi,		wW_rW_DW_ARG,	0),
	asBCINFO(SUBIi,		wW_rW_DW_ARG,	0),
	asBCINFO(MULIi,		wW_rW_DW_ARG,	0),
	asBCINFO(ADDIf,		wW_rW_DW_ARG,	0),
	asBCINFO(SUBIf,		wW_rW_DW_ARG,	0),
	asBCINFO(MULIf,		wW_rW_DW_ARG,	0),
	asBCINFO(SetG4,		PTR_DW_ARG,		0),
	asBCINFO(ChkRefS,	NO_ARG,			0),
	asBCINFO(ChkNullV,	rW_ARG,			0),
	asBCINFO(CALLINTF,	DW_ARG,			0xFFFF),
	asBCINFO(iTOb,		rW_ARG,			0),
	asBCINFO(iTOw,		rW_ARG,			0),
	asBCINFO(SetV1,		wW_DW_ARG,		0),
	asBCINFO(SetV2,		wW_DW_ARG,		0),
	asBCINFO(Cast,		DW_ARG,			-AS_PTR_SIZE),
	asBCINFO(i64TOi,	wW_rW_ARG,		0),
	asBCINFO(uTOi64,	wW_rW_ARG,		0),
	asBCINFO(iTOi64,	wW_rW_ARG,		0),
	asBCINFO(fTOi64,	wW_rW_ARG,		0),
	asBCINFO(dTOi64,	rW_ARG,			0),
	asBCINFO(fTOu64,	wW_rW_ARG,		0),
	asBCINFO(dTOu64,	rW_ARG,			0),
	asBCINFO(i64TOf,	wW_rW_ARG,		0),
	asBCINFO(u64TOf,	wW_rW_ARG,		0),
	asBCINFO(i64TOd,	rW_ARG,			0),
	asBCINFO(u64TOd,	rW_ARG,			0),
	asBCINFO(NEGi64,	rW_ARG,			0),
	asBCINFO(INCi64,	NO_ARG,			0),
	asBCINFO(DECi64,	NO_ARG,			0),
	asBCINFO(BNOT64,	rW_ARG,			0),
	asBCINFO(ADDi64,	wW_rW_rW_ARG,	0),
	asBCINFO(SUBi64,	wW_rW_rW_ARG,	0),
	asBCINFO(MULi64,	wW_rW_rW_ARG,	0),
	asBCINFO(DIVi64,	wW_rW_rW_ARG,	0),
	asBCINFO(MODi64,	wW_rW_rW_ARG,	0),
	asBCINFO(BAND64,	wW_rW_rW_ARG,	0),
	asBCINFO(BOR64,		wW_rW_rW_ARG,	0),
	asBCINFO(BXOR64,	wW_rW_rW_ARG,	0),
	asBCINFO(BSLL64,	wW_rW_rW_ARG,	0),
	asBCINFO(BSRL64,	wW_rW_rW_ARG,	0),
	asBCINFO(BSRA64,	wW_rW_rW_ARG,	0),
	asBCINFO(CMPi64,	rW_rW_ARG,		0),
	asBCINFO(CMPu64,	rW_rW_ARG,		0),
	asBCINFO(ChkNullS,	W_ARG,			0),
	asBCINFO(ClrHi,		NO_ARG,			0),
	asBCINFO(JitEntry,	PTR_ARG,		0),
	asBCINFO(CallPtr,	rW_ARG,			0xFFFF),
	asBCINFO(FuncPtr,	PTR_ARG,		AS_PTR_SIZE),
	asBCINFO(LoadThisR,	W_DW_ARG,		0),
	asBCINFO(PshV8,		rW_ARG,			2),
	asBCINFO(DIVu,		wW_rW_rW_ARG,	0),
	asBCINFO(MODu,		wW_rW_rW_ARG,	0),
	asBCINFO(DIVu64,	wW_rW_rW_ARG,	0),
	asBCINFO(MODu64,	wW_rW_rW_ARG,	0),
	asBCINFO(LoadRObjR,	rW_W_DW_ARG,	0),
	asBCINFO(LoadVObjR,	rW_W_DW_ARG,	0),
	asBCINFO(RefCpyV,	wW_PTR_ARG,		0),
	asBCINFO(JLowZ,		DW_ARG,			0),
	asBCINFO(JLowNZ,	DW_ARG,			0),
	asBCINFO(AllocMem,	wW_DW_ARG,		0),
	asBCINFO(SetListSize, rW_DW_DW_ARG,	0),
	asBCINFO(PshListElmnt, rW_DW_ARG,	AS_PTR_SIZE),
	asBCINFO(SetListType, rW_DW_DW_ARG,	0),
	asBCINFO(POWi,		wW_rW_rW_ARG,	0),
	asBCINFO(POWu,		wW_rW_rW_ARG,	0),
	asBCINFO(POWf,		wW_rW_rW_ARG,	0),
	asBCINFO(POWd,		wW_rW_rW_ARG,	0),
	asBCINFO(POWdi,		wW_rW_rW_ARG,	0),
	asBCINFO(POWi64,	wW_rW_rW_ARG,	0),
	asBCINFO(POWu64,	wW_rW_rW_ARG,	0),
	asBCINFO(Thiscall1, DW_ARG,			-AS_PTR_SIZE-1),

	asBCINFO_DUMMY(201),
	asBCINFO_DUMMY(202),
	asBCINFO_DUMMY(203),
	asBCINFO_DUMMY(204),
	asBCINFO_DUMMY(205),
	asBCINFO_DUMMY(206),
	asBCINFO_DUMMY(207),
	asBCINFO_DUMMY(208),
	asBCINFO_DUMMY(209),
	asBCINFO_DUMMY(210),
	asBCINFO_DUMMY(211),
	asBCINFO_DUMMY(212),
	asBCINFO_DUMMY(213),
	asBCINFO_DUMMY(214),
	asBCINFO_DUMMY(215),
	asBCINFO_DUMMY(216),
	asBCINFO_DUMMY(217),
	asBCINFO_DUMMY(218),
	asBCINFO_DUMMY(219),
	asBCINFO_DUMMY(220),
	asBCINFO_DUMMY(221),
	asBCINFO_DUMMY(222),
	asBCINFO_DUMMY(223),
	asBCINFO_DUMMY(224),
	asBCINFO_DUMMY(225),
	asBCINFO_DUMMY(226),
	asBCINFO_DUMMY(227),
	asBCINFO_DUMMY(228),
	asBCINFO_DUMMY(229),
	asBCINFO_DUMMY(230),
	asBCINFO_DUMMY(231),
	asBCINFO_DUMMY(232),
	asBCINFO_DUMMY(233),
	asBCINFO_DUMMY(234),
	asBCINFO_DUMMY(235),
	asBCINFO_DUMMY(236),
	asBCINFO_DUMMY(237),
	asBCINFO_DUMMY(238),
	asBCINFO_DUMMY(239),
	asBCINFO_DUMMY(240),
	asBCINFO_DUMMY(241),
	asBCINFO_DUMMY(242),
	asBCINFO_DUMMY(243),
	asBCINFO_DUMMY(244),
	asBCINFO_DUMMY(245),
	asBCINFO_DUMMY(246),
	asBCINFO_DUMMY(247),
	asBCINFO_DUMMY(248),
	asBCINFO_DUMMY(249),

	asBCINFO(TryBlock,		DW_ARG,			0),
	asBCINFO(VarDecl,		W_ARG,			0),
	asBCINFO(Block,			INFO,			0),
	asBCINFO(ObjInfo,		rW_DW_ARG,		0),
	asBCINFO(LINE,			INFO,			0),
	asBCINFO(LABEL,			INFO,			0)
};

// Macros to access bytecode instruction arguments
#define asBC_DWORDARG(x)  (*(((asDWORD*)x)+1))
#define asBC_INTARG(x)    (*(int*)(((asDWORD*)x)+1))
#define asBC_QWORDARG(x)  (*(asQWORD*)(((asDWORD*)x)+1))
#define asBC_FLOATARG(x)  (*(float*)(((asDWORD*)x)+1))
#define asBC_PTRARG(x)    (*(asPWORD*)(((asDWORD*)x)+1))
#define asBC_WORDARG0(x)  (*(((asWORD*)x)+1))
#define asBC_WORDARG1(x)  (*(((asWORD*)x)+2))
#define asBC_SWORDARG0(x) (*(((short*)x)+1))
#define asBC_SWORDARG1(x) (*(((short*)x)+2))
#define asBC_SWORDARG2(x) (*(((short*)x)+3))


END_AS_NAMESPACE

#endif
