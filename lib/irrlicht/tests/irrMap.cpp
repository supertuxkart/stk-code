#include "testUtils.h"
#include <irrlicht.h>

using namespace irr;
using namespace core;

// map has no operator== currently so we have to check manually
// TODO: Add an operator== to core::map and the kick this function out
template <class KeyType, class ValueType>
static bool compareMaps(core::map<KeyType,ValueType> & a, core::map<KeyType,ValueType> & b)
{
	if ( a.size() != b.size() )
		return false;
	// can't test allocator because we have no access to it here
	typename core::map<KeyType, ValueType>::Iterator iterA = a.getIterator();
	typename core::map<KeyType, ValueType>::Iterator iterB = b.getIterator();
	for ( ; !iterA.atEnd(); iterA++, iterB++ )	// TODO: only iter++, no ++iter in irr::map
	{
		if ( iterA->getValue() != iterB->getValue() )
			return false;
	}
	return true;
}


static bool testSwap()
{
	bool result = true;

	core::map<int, int> map1, map2, copy1, copy2;
	for ( int i=0; i<99; ++i )
	{
		map1[i] = i;
		copy1[i] = i;	// TODO: whatever the reason - irr::map does not want operator= so we have to assign to identical values
		if ( i < 10 )	// we want also different container sizes
		{
			map2[i] = 99-i;
			copy2[i] = 99-i;	// TODO: whatever the reason - irr::map does not want operator= so we have to assign to identical values
		}
	}
	map1.swap(map2);

	result &= compareMaps(map1, copy2);
	result &= compareMaps(map2, copy1);

	assert_log( result );

	return result;
}

// Test the functionality of core::list
bool testIrrMap(void)
{
	bool success = true;

	success &= testSwap();

	if(success)
		logTestString("\nAll tests passed\n");
	else
		logTestString("\nFAIL!\n");

	return success;
}
