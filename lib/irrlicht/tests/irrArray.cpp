// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"
#include <irrlicht.h>

using namespace irr;
using namespace core;

core::map<int, int> countReferences;

struct SDummy
{
	SDummy(int a) : x(a)
	{
		countReferences.insert(x,1);
	}

	SDummy() : x(0)
	{
		countReferences.insert(x,1);
	}

	SDummy(const SDummy& other)
	{
		x = other.x;
		countReferences[x] = countReferences[x] + 1;
	}

	~SDummy()
	{
		countReferences[x] = countReferences[x] - 1;
	}

	int x;
};

static bool testErase()
{
	{
		core::array<SDummy> aaa;
		aaa.push_back(SDummy(0));
		aaa.push_back(SDummy(1));
		aaa.push_back(SDummy(2));
		aaa.push_back(SDummy(3));
		aaa.push_back(SDummy(4));
		aaa.push_back(SDummy(5));

		aaa.erase(0,2);
	}

	for ( core::map<int,int>::Iterator it = countReferences.getIterator(); !it.atEnd(); it++ )
	{
		if ( it->getValue() != 0 )
		{
			logTestString("testErase: wrong count for %d, it's: %d\n", it->getKey(), it->getValue());
			return false;
		}
	}
	return true;
}


struct VarArray
{
	core::array < int, core::irrAllocatorFast<int> > MyArray;
};

static bool testSelfAssignment()
{
	core::array<int> myArray;
	myArray.push_back(1);
	myArray = myArray;
	return myArray.size() == 1;
}

// this will (did once) crash when wrong due to deallocating memory twice, so no return value
static void crashTestFastAlloc()
{
	core::array < VarArray, core::irrAllocatorFast<VarArray> > ArrayArray;
	ArrayArray.setAllocStrategy(core::ALLOC_STRATEGY_SAFE); // force more re-allocations
	VarArray var;
	var.MyArray.setAllocStrategy(core::ALLOC_STRATEGY_SAFE); // force more re-allocations
	var.MyArray.push_back( 0 );

	for ( int i=0; i< 100; ++i )
	{
		ArrayArray.push_back(var);
		ArrayArray.push_back(var);
	}
}

static bool testSwap()
{
	bool result = true;

	core::array<int> array1, array2, copy1, copy2;
	for ( int i=0; i<99; ++i )
	{
		array1.push_back(i);
		if ( i < 10 )	// we want also different container sizes
			array2.push_back(99-i);
	}
	copy1 = array1;
	copy2 = array2;
	array1.swap(array2);

	result &= (array1 == copy2);
	result &= (array2 == copy1);

	assert_log( result );

	return result;
}

// Test the functionality of core::array
bool testIrrArray(void)
{
	bool allExpected = true;

	logTestString("crashTestFastAlloc\n");
	crashTestFastAlloc();
	allExpected &= testSelfAssignment();
	allExpected &= testSwap();
	allExpected &= testErase();

	if(allExpected)
		logTestString("\nAll tests passed\n");
	else
		logTestString("\nFAIL!\n");

	return allExpected;
}
