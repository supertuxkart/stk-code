// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;

//! This was an older Irrlicht implementation, tested against for reference.
static inline u32 old_strtol10(const char* in, const char** out=0)
{
	u32 value = 0;

	while ( ( *in >= '0') && ( *in <= '9' ))
	{
		value = ( value * 10 ) + ( *in - '0' );
		++in;
	}
	if (out)
		*out = in;
	return value;
}

//! This was an older Irrlicht implementation, tested against for reference.
static inline const char* old_fast_atof_move( const char* c, float& out)
{
	bool inv = false;
	const char *t;
	float f;

	if (*c=='-')
	{
		++c;
		inv = true;
	}

	//f = (float)strtol(c, &t, 10);
	f = (float) old_strtol10 ( c, &c );

	if (*c == '.')
	{
		++c;

		//float pl = (float)strtol(c, &t, 10);
		float pl = (float) old_strtol10 ( c, &t );
		pl *= fast_atof_table[t-c];

		f += pl;

		c = t;

		if (*c == 'e')
		{
			++c;
			//float exp = (float)strtol(c, &t, 10);
			bool einv = (*c=='-');
			if (einv)
				++c;

			float exp = (float)old_strtol10(c, &c);
			if (einv)
				exp *= -1.0f;

			f *= (float)pow(10.0f, exp);
		}
	}

	if (inv)
		f *= -1.0f;

	out = f;
	return c;
}

//! This was an older Irrlicht implementation, tested against for reference.
static inline float old_fast_atof(const char* c)
{
	float ret;
	old_fast_atof_move(c, ret);
	return ret;
}


static bool testCalculation_atof(const char * valueString)
{
	const f32 newFastValue = fast_atof(valueString);
	const f32 oldFastValue = old_fast_atof(valueString);
	const f32 atofValue = (f32)atof(valueString);

	logTestString("\n String '%s'\n New fast %.40f\n Old fast %.40f\n     atof %.40f\n",
		valueString, newFastValue, oldFastValue, atofValue);

	const f32 diffNew = fabs(newFastValue - atofValue) ;
	const f32 diffOld = fabs(newFastValue - atofValue) ;
	bool accurate = diffNew <= diffOld || equalsByUlp(diffNew, diffOld, 1);

	if(!accurate)
		logTestString("*** ERROR - less accurate than old method ***\n\n");

	return accurate;
}

static bool testCalculation_strtol(const char * valueString)
{
	const s32 newFastValue = strtol10(valueString);
	const s32 oldFastValue = old_strtol10(valueString);
	const s32 strtolValue = (s32)clamp(strtol(valueString, 0, 10), (long int)INT_MIN, (long int)INT_MAX);

	logTestString("\n String '%s'\n New fast %d\n Old fast %d\n   strtol %d\n",
		valueString, newFastValue, oldFastValue, strtolValue);

	bool accurate = (newFastValue == strtolValue) || (oldFastValue != strtolValue);

	if (!accurate)
		logTestString("*** ERROR - wrong calculation in new method ***\n\n");

	return accurate;
}

//! Test both the accuracy and speed of Irrlicht's fast_atof() implementation.
bool test_fast_atof(void)
{
	bool accurate = true;

	accurate &= testCalculation_atof("340282346638528859811704183484516925440.000000");
	accurate &= testCalculation_atof("3.402823466e+38F");
	accurate &= testCalculation_atof("3402823466e+29F");
	accurate &= testCalculation_atof("-340282346638528859811704183484516925440.000000");
	accurate &= testCalculation_atof("-3.402823466e+38F");
	accurate &= testCalculation_atof("-3402823466e+29F");
	accurate &= testCalculation_atof("34028234663852885981170418348451692544.000000");
	accurate &= testCalculation_atof("3.402823466e+37F");
	accurate &= testCalculation_atof("3402823466e+28F");
	accurate &= testCalculation_atof("-34028234663852885981170418348451692544.000000");
	accurate &= testCalculation_atof("-3.402823466e+37F");
	accurate &= testCalculation_atof("-3402823466e+28F");
	accurate &= testCalculation_atof(".00234567");
	accurate &= testCalculation_atof("-.00234567");
	accurate &= testCalculation_atof("0.00234567");
	accurate &= testCalculation_atof("-0.00234567");
	accurate &= testCalculation_atof("1.175494351e-38F");
	accurate &= testCalculation_atof("1175494351e-47F");
	accurate &= testCalculation_atof("1.175494351e-37F");
	accurate &= testCalculation_atof("1.175494351e-36F");
	accurate &= testCalculation_atof("-1.175494351e-36F");
	accurate &= testCalculation_atof("123456.789");
	accurate &= testCalculation_atof("-123456.789");
	accurate &= testCalculation_atof("0000123456.789");
	accurate &= testCalculation_atof("-0000123456.789");
	accurate &= testCalculation_atof("-0.0690462109446526");

	if (!accurate)
	{
		logTestString("Calculation is not accurate, so the speed is irrelevant\n");
		return false;
	}

#ifndef _DEBUG	// it's only faster in release
	IrrlichtDevice* device = createDevice(video::EDT_NULL);
	if (!device)
		return false;
	ITimer* timer = device->getTimer();

	const int ITERATIONS = 100000;
	int i;

	f32 value;
	u32 then = timer->getRealTime();
	for(i = 0; i < ITERATIONS; ++i)
		value = (f32)atof("-340282346638528859811704183484516925440.000000");

	const u32 atofTime = timer->getRealTime() - then;

	then += atofTime;
	for(i = 0; i < ITERATIONS; ++i)
		value = fast_atof("-340282346638528859811704183484516925440.000000");
	const u32 fastAtofTime = timer->getRealTime() - then;

	then += fastAtofTime;
	for(i = 0; i < ITERATIONS; ++i)
		value = old_fast_atof("-340282346638528859811704183484516925440.000000");
	const u32 oldFastAtofTime = timer->getRealTime() - then;

	logTestString("Speed test\n         atof time = %d\n    fast_atof Time = %d\nold fast_atof time = %d\n",
		atofTime, fastAtofTime, oldFastAtofTime);

	device->closeDevice();
	device->run();
	device->drop();

	if(fastAtofTime > (1.2f*atofTime))
	{
		logTestString("The fast method is slower than atof()\n");
		return false;
	}
#endif // #ifndef _DEBUG

	return true;
}

//! Test both the accuracy and speed of Irrlicht's strtol10() implementation.
bool test_strtol(void)
{
	bool accurate = true;

	accurate &= testCalculation_strtol("340282346638528859811704183484516925440");
	accurate &= testCalculation_strtol("3402823466");
	accurate &= testCalculation_strtol("3402823466e+29F");
	accurate &= testCalculation_strtol("-340282346638528859811704183484516925440");
	accurate &= testCalculation_strtol("-3402823466");
	accurate &= testCalculation_strtol("-3402823466e+29F");
	accurate &= testCalculation_strtol("402823466385288598117");
	accurate &= testCalculation_strtol("402823466");
	accurate &= testCalculation_strtol("402823466e+28F");
	accurate &= testCalculation_strtol("402823466385288598117");
	accurate &= testCalculation_strtol("-402823466");
	accurate &= testCalculation_strtol("-402823466e+28F");
	accurate &= testCalculation_strtol(".00234567");
	accurate &= testCalculation_strtol("-234567");
	accurate &= testCalculation_strtol("234567");
	accurate &= testCalculation_strtol("-234567");
	accurate &= testCalculation_strtol("1175494351");
	accurate &= testCalculation_strtol("11754943512");
	accurate &= testCalculation_strtol("11754943513");
	accurate &= testCalculation_strtol("11754943514");
	accurate &= testCalculation_strtol("-1175494351");
	accurate &= testCalculation_strtol("123456789");
	accurate &= testCalculation_strtol("-123456789");
	accurate &= testCalculation_strtol("123456.789");
	accurate &= testCalculation_strtol("-123456.789");
	accurate &= testCalculation_strtol("-109446526");

	if(!accurate)
	{
		logTestString("Calculation is not accurate, so the speed is irrelevant\n");
		return false;
	}

#ifndef _DEBUG	// it's only faster in release
	IrrlichtDevice* device = createDevice(video::EDT_NULL);
	if (!device)
		return false;
	ITimer* timer = device->getTimer();

	const int ITERATIONS = 1000000;
	int i;

	s32 value;
	u32 then = timer->getRealTime();
	for(i = 0; i < ITERATIONS; ++i)
		value = strtol("-3402823466", 0, 10);

	const u32 strtolTime = timer->getRealTime() - then;

	then += strtolTime;
	for(i = 0; i < ITERATIONS; ++i)
		value = strtol10("-3402823466");
	const u32 strtol10Time = timer->getRealTime() - then;

	then += strtol10Time;
	for(i = 0; i < ITERATIONS; ++i)
		value = old_strtol10("-3402823466");
	const u32 oldstrtol10Time = timer->getRealTime() - then;

	logTestString("Speed test\n      strtol time = %d\n    strtol10 time = %d\nold strtol10 time = %d\n",
		strtolTime, strtol10Time, oldstrtol10Time);

	device->closeDevice();
	device->run();
	device->drop();

	if (strtol10Time > (1.2f*strtolTime))
	{
		logTestString("The fast method is slower than strtol()\n");
		return false;
	}
#endif // #ifndef _DEBUG

	return true;
}

bool fast_atof(void)
{
	bool ok = true;
	ok &= test_fast_atof() ;
	ok &= test_strtol();
	return ok;
}
