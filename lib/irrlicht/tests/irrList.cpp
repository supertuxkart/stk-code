#include "testUtils.h"
#include <irrlicht.h>

using namespace irr;
using namespace core;

// list has no operator== currently so we have to check manually
// TODO: Add an operator== to core::list and the kick this function out
template <typename T>
static bool compareLists(const core::list<T> & a, const core::list<T> & b)
{
	if ( a.size() != b.size() )
		return false;
	// can't test allocator because we have no access to it here
	typename core::list<T>::ConstIterator iterA = a.begin();
	typename core::list<T>::ConstIterator iterB = b.begin();
	for ( ; iterA != a.end(); ++iterA, ++iterB )
	{
		if ( (*iterA) != (*iterB) )
			return false;
	}
	return true;
}

// Make sure that we can get a const iterator from a non-const list
template <typename T>
static void constIteratorCompileTest(core::list<T> & a)
{
	typename core::list<T>::ConstIterator iterA = a.begin();
	while (iterA != a.end() )
	{
		++iterA;
	}
}

static bool testSwap()
{
	bool result = true;

	core::list<int> list1, list2, copy1, copy2;
	for ( int i=0; i<99; ++i )
	{
		list1.push_back(i);
		if ( i < 10 )	// we want also different container sizes i < 50 )
			list2.push_back(99-i);
	}
	copy1 = list1;
	copy2 = list2;
	list1.swap(list2);


	result &= compareLists<int>(list1, copy2);
	result &= compareLists<int>(list2, copy1);

	assert_log( result );

	return result;
}

// Test the functionality of core::list
bool testIrrList(void)
{
	bool success = true;

	core::list<int> compileThisList;
	constIteratorCompileTest(compileThisList);

	success &= testSwap();

	if(success)
		logTestString("\nAll tests passed\n");
	else
		logTestString("\nFAIL!\n");

	return success;
}
