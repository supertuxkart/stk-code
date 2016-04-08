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


//
// as_typeinfo.cpp
//
// This class holds extra type info for the compiler
//

#include "as_config.h"

#ifndef AS_NO_COMPILER

#include "as_typeinfo.h"
#include "as_scriptengine.h"

BEGIN_AS_NAMESPACE

asCTypeInfo::asCTypeInfo()
{
	isTemporary           = false;
	stackOffset           = 0;
	isConstant            = false;
	isVariable            = false;
	isExplicitHandle      = false;
	qwordValue            = 0;
	isLValue              = false;
	isRefToLocal          = false;
}

void asCTypeInfo::Set(const asCDataType &dt)
{
	dataType         = dt;

	isTemporary      = false;
	stackOffset      = 0;
	isConstant       = false;
	isVariable       = false;
	isExplicitHandle = false;
	qwordValue       = 0;
	isLValue         = false;
	isRefToLocal     = false;
}

void asCTypeInfo::SetVariable(const asCDataType &dt, int stackOffset, bool isTemporary)
{
	Set(dt);

	this->isVariable  = true;
	this->isTemporary = isTemporary;
	this->stackOffset = (short)stackOffset;
}

void asCTypeInfo::SetConstantQW(const asCDataType &dt, asQWORD value)
{
	Set(dt);

	isConstant = true;
	qwordValue = value;
}

void asCTypeInfo::SetConstantDW(const asCDataType &dt, asDWORD value)
{
	Set(dt);

	isConstant = true;
	dwordValue = value;
}

void asCTypeInfo::SetConstantB(const asCDataType &dt, asBYTE value)
{
	Set(dt);

	isConstant = true;
	byteValue = value;
}

void asCTypeInfo::SetConstantF(const asCDataType &dt, float value)
{
	Set(dt);

	isConstant = true;
	floatValue = value;
}

void asCTypeInfo::SetConstantD(const asCDataType &dt, double value)
{
	Set(dt);

	isConstant = true;
	doubleValue = value;
}

void asCTypeInfo::SetUndefinedFuncHandle(asCScriptEngine *engine)
{
	// This is used for when the expression evaluates to a 
	// function, but it is not yet known exactly which. The
	// owner expression will hold the name of the function
	// to determine the exact function when the signature is
	// known.
	Set(asCDataType::CreateObjectHandle(&engine->functionBehaviours, true));
	isConstant       = true;
	isExplicitHandle = false;
	qwordValue       = 1; // Set to a different value than 0 to differentiate from null constant
	isLValue         = false;
}

bool asCTypeInfo::IsUndefinedFuncHandle() const
{
	if( isConstant == false ) return false;
	if( qwordValue == 0 ) return false;
	if( isLValue ) return false;
	if( dataType.GetObjectType() == 0 ) return false;
	if( dataType.GetObjectType()->name != "$func" ) return false;
	if( dataType.GetFuncDef() ) return false;

	return true;
}

void asCTypeInfo::SetNullConstant()
{
	Set(asCDataType::CreateNullHandle());
	isConstant       = true;
	isExplicitHandle = false;
	qwordValue       = 0;
	isLValue         = false;
}

bool asCTypeInfo::IsNullConstant() const
{
	// We can't check the actual object type, because the null constant may have been cast to another type
	if( isConstant && dataType.IsObjectHandle() && qwordValue == 0 )
		return true;

	return false;
}

void asCTypeInfo::SetVoid()
{
	Set(asCDataType::CreatePrimitive(ttVoid, false));
	isLValue = false;
	isConstant = true;
}

bool asCTypeInfo::IsVoid() const
{
	if( dataType.GetTokenType() == ttVoid )
		return true;

	return false;
}

void asCTypeInfo::SetDummy()
{
	SetConstantQW(asCDataType::CreatePrimitive(ttInt, true), 0);
}


END_AS_NAMESPACE

#endif // AS_NO_COMPILER
