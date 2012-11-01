#include "testUtils.h"

using namespace irr;
using namespace core;

// Test the functionality of the Irrlicht timer
bool testTimer(void)
{
	bool success = true;

	IrrlichtDevice* device = createDevice(video::EDT_NULL);
	if (!device)
		return false;

	logTestString("Testing virtual timer.\n");

	ITimer* timer = device->getTimer();

	// must be running at start
	success &= !timer->isStopped();

	// starting more often should not stop the timer
	timer->start();
	success &= !timer->isStopped();

	// one stop should not stop the timer because it's started twice now
	timer->stop();
	success &= !timer->isStopped();

	// another stop should really stop it
	timer->stop();
	success &= timer->isStopped();

	// third stop - timer should still be stopped
	timer->stop();
	success &= timer->isStopped();

	// should not start yet
	timer->start();
	success &= timer->isStopped();

	// start again
	timer->start();
	success &= !timer->isStopped();

	logTestString("Testing virtual timer done. %s\n", success?"Success":"Failure");

	logTestString("Testing real timer.\n");
	const u32 startVirtual = timer->getTime();
	const u32 startReal = timer->getRealTime();
	device->sleep(2);
	if (startReal != timer->getRealTime())
		logTestString("Warning: Real timer did not progress. Maybe the time slices are too coarse to see.\n");
	if (startVirtual != timer->getTime())
		logTestString("Warning: Virtual timer did not progress. Maybe the time slices are too coarse to see.\n");

	irr::ITimer::RealTimeDate date = timer->getRealTimeAndDate();
	logTestString("Real time and date. %d.%d.%d at %d:%d:%d\n", date.Day, date.Month, date.Year, date.Hour, date.Minute, date.Second);
	logTestString("This is day %d of the year and weekday %d. The current time zone has daylight saving %s\n", date.Yearday, date.Weekday, date.IsDST?"enabled":"disabled");

	device->closeDevice();
	device->run();
	device->drop();

	return success;
}
