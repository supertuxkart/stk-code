// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"
#include <irrlicht.h>

using namespace irr;
using namespace core;

static bool testSelfAssignment()
{
	core::stringw myString(L"foo");
	myString = myString;
	return myString == core::stringw(L"foo");
}

static bool testSplit()
{
	logTestString("Test stringw::split()\n");
	core::stringw teststring(L"[b]this [/b] is a [color=0xff000000]test[/color].");
	core::list<core::stringw> parts1;
	teststring.split<core::list<core::stringw> >(parts1, L"[");
	core::list<core::stringw> parts2;
	teststring.split<core::list<core::stringw> >(parts2, L"[", 1, false, true);
	return (parts1.getSize()==4) && (parts2.getSize()==5);
}

static bool testFastAlloc()
{
	core::string<wchar_t, core::irrAllocatorFast<wchar_t> > FastString(L"abc");
	core::string<wchar_t, core::irrAllocatorFast<wchar_t> > FastStringLong(L"longer");

	FastString  = L"test";

	// cause a reallocation
	FastString = FastStringLong;

	// this test should either not compile or crash when the allocaters are messed up
	return true;
}

static bool testReplace()
{
	// test string getting longer
	core::stringw str = L"no";
	str.replace(L"no", L"yes");
	if ( str != L"yes" )
		return false;
	str = L"nonono";
	str.replace(L"no", L"yes");
	if ( str != L"yesyesyes" )
		return false;
	str = L"nomaybenomaybeno";
	str.replace(L"no", L"yes");
	if ( str != L"yesmaybeyesmaybeyes" )
		return false;

	// test string staying same length
	str = L"one";
	str.replace(L"one", L"two");
	if ( str != L"two" )
		return false;
	str = L"oneone";
	str.replace(L"one", L"two");
	if ( str != L"twotwo" )
		return false;

	// test string getting shorter
	str = L"yes";
	str.replace(L"yes", L"no");
	if ( str != L"no" )
		return false;

	str = L"yesyes";
	str.replace(L"yes", L"no");
	if ( str != L"nono" )
		return false;

	// remove string-parts completely
	str = L"killme";
	str.replace(L"killme", L"");
	if ( str != L"" )
		return false;

	str = L"killmenow";
	str.replace(L"killme", L"");
	if ( str != L"now" )
		return false;

	str = L"nowkillme";
	str.replace(L"killme", L"");
	if ( str != L"now" )
		return false;

	// remove nothing
	str = L"keepme";
	str.replace(L"", L"whatever");
	if ( str != L"keepme" )
		return false;

	str = L"keepme";
	str.replace(L"", L"");
	if ( str != L"keepme" )
		return false;

	return true;
}


bool testAppendStringc()
{
	core::stringc str;
	// Test with character
	if (str != "")
		return false;
	str += 'W';
	if (str != "W")
		return false;
	str += 'i';
	if (str != "Wi")
		return false;
	str="";
	if (str != "")
		return false;

	// Test with C-style string
	str += "Another Test";
	if (str != "Another Test")
		return false;
	str="";
	str += 'A';
	str += "nother Test";
	if (str != "Another Test")
		return false;
	str="";

	// Test with int
	str += 10;
	if (str != "10")
		return false;
	str += 0;
	if (str != "100")
		return false;
	str="";
	str += "-32";
	if (str != "-32")
		return false;
	str="";

	// Test with unsigned int
	str += 21u;
	if (str != "21")
		return false;
	str += 0u;
	if (str != "210")
		return false;
	str="";

	// Test with long int
	str += 456l;
	if (str != "456")
		return false;
	str += 0l;
	if (str != "4560")
		return false;
	str="";
	str += -456l;
	if (str != "-456")
		return false;
	str="";

	// Test with unsigned long
	str += 994ul;
	if (str != "994")
		return false;
	str += 0ul;
	if (str != "9940")
		return false;
	str="";
	return true;
}

bool testLowerUpper()
{
	irr::core::array <irr::core::stringc> stringsOrig, targetLower, targetUpper;
	stringsOrig.push_back("abc");
	targetLower.push_back("abc");
	targetUpper.push_back("ABC");
	stringsOrig.push_back("ABC");
	targetLower.push_back("abc");
	targetUpper.push_back("ABC");
	stringsOrig.push_back("Abc");
	targetLower.push_back("abc");
	targetUpper.push_back("ABC");
	stringsOrig.push_back("aBBc");
	targetLower.push_back("abbc");
	targetUpper.push_back("ABBC");
	stringsOrig.push_back("abC");
	targetLower.push_back("abc");
	targetUpper.push_back("ABC");
	stringsOrig.push_back("");
	targetLower.push_back("");
	targetUpper.push_back("");
	/* TODO: those are not supported so far
	stringsOrig.push_back("ßäöü");
	targetLower.push_back("ßäöü");
	targetUpper.push_back("ßÄÖÜ");
	stringsOrig.push_back("ßÄÖÜ");
	targetLower.push_back("ßäöü");
	targetUpper.push_back("ßÄÖÜ");
	*/

	for ( irr::u32 i=0; i<stringsOrig.size(); ++i )
	{
		irr::core::stringc c = stringsOrig[i];
		c.make_lower();
		if ( c != targetLower[i] )
		{
			logTestString("make_lower for stringc failed in test %d %s\n", i, stringsOrig[i].c_str());
			return false;
		}

		c = stringsOrig[i];
		c.make_upper();
		if ( c != targetUpper[i] )
		{
			logTestString("make_upper for stringc failed in test %d %s %s\n", i, stringsOrig[i].c_str(), c.c_str());
			return false;
		}

		irr::core::stringw w = irr::core::stringw(stringsOrig[i]);
		c.make_lower();
		if ( c != irr::core::stringw(targetLower[i]) )
		{
			logTestString("make_lower for stringw failed in test %d %s\n", i, stringsOrig[i].c_str());
			return false;
		}

		c = irr::core::stringw(stringsOrig[i]);
		c.make_upper();
		if ( c != irr::core::stringw(targetUpper[i]) )
		{
			logTestString("make_upper for stringw failed in test %d %s\n", i, stringsOrig[i].c_str());
			return false;
		}
	}

	return true;
}

bool testFindFunctions()
{
	irr::core::stringc dot(".");
	irr::s32 p = dot.findFirst(0);
	if ( p >= 0 )
		return false;

	irr::core::stringc empty("");
	p = empty.findLastCharNotInList("x",1);
	if ( p >= 0 )
		return false;

	p = empty.findLast('x');
	if ( p >= 0 )
		return false;

	p = dot.findLast('.');
	if ( p != 0 )
		return false;

	p = empty.findLastChar("ab", 2);
	if ( p >= 0 )
		return false;

	p = dot.findLastChar("-.", 2);
	if ( p != 0 )
		return false;

	return true;
}

// Test the functionality of irrString
/** Validation is done with assert_log() against expected results. */
bool testIrrString(void)
{
	bool allExpected = true;

	logTestString("Test stringc\n");
	{
		// Check empty string
		core::stringc empty;
		assert_log(empty.size()==0);
		assert_log(empty[0]==0);
		assert_log(empty.c_str()!=0);
		assert_log(*(empty.c_str())==0);
		// Assign content
		empty = "Test";
		assert_log(empty.size()==4);
		assert_log(empty[0]=='T');
		assert_log(empty[3]=='t');
		assert_log(*(empty.c_str())=='T');
		//Assign empty string, should be same as in the beginning
		empty = "";
		assert_log(empty.size()==0);
		assert_log(empty[0]==0);
		assert_log(*(empty.c_str())==0);
	}
	logTestString("Test stringw\n");
	{
		core::stringw empty;
		assert_log(empty.size()==0);
		assert_log(empty[0]==0);
		assert_log(empty.c_str()!=0);
		assert_log(*(empty.c_str())==0);
		empty = L"Test";
		assert_log(empty.size()==4);
		assert_log(empty[0]==L'T');
		assert_log(empty[3]=='t');
		assert_log(*(empty.c_str())==L'T');
		empty = L"";
		assert_log(empty.size()==0);
		assert_log(empty[0]==0);
		assert_log(*(empty.c_str())==0);
		assert_log(allExpected &= testSplit());
	}
	allExpected &= testAppendStringc();

	logTestString("Test io::path\n");
	{
		// Only test that this type exists, it's one from above
		io::path myPath;
		myPath = "Some text"; // Only to avoid wrong optimizations
	}

	logTestString("Test self assignment\n");
	allExpected &= testSelfAssignment();

	logTestString("test fast alloc\n");
	allExpected &= testFastAlloc();

	logTestString("test replace\n");
	allExpected &= testReplace();

	logTestString("test make_lower and make_uppers\n");
	allExpected &= testLowerUpper();

	logTestString("test find functions\n");
	allExpected &= testFindFunctions();

	if(allExpected)
		logTestString("\nAll tests passed\n");
	else
		logTestString("\nFAIL!\n");

	return allExpected;
}
