
#ifndef _TEST_UTILS_H_
#define _TEST_UTILS_H_ 1

#include "irrlicht.h"
#include <assert.h>

#define TestWithAllDrivers(X) \
	logTestString("Running test " #X "\n"); \
	for (u32 i=1; i<video::EDT_COUNT; ++i) \
	result &= X(video::E_DRIVER_TYPE(i))
#define TestWithAllHWDrivers(X) \
	logTestString("Running test " #X "\n"); \
	for (u32 i=video::EDT_DIRECT3D8; i<video::EDT_COUNT; ++i) \
	result &= X(video::E_DRIVER_TYPE(i))

// replacement for assert which does log the lines instead
#define assert_log(X) \
do { \
	if ( !(X) ) \
	{ \
		logTestString("ASSERT in %s:%d: %s\n", __FILE__, __LINE__, #X); \
	} \
} while (false)

//! Compare two files
/** \param fileName1 The first file for comparison.
	\param fileName2 The second file for comparison.
	\return true if the files are identical, false on any error or difference. */
extern bool binaryCompareFiles(const char * fileName1, const char * fileName2);

//! Compare two xml-files (which can have different types of text-encoding)
/** \param fs Filesystem which should be used.
	\param fileName1 The first file for comparison.
	\param fileName2 The second file for comparison.
	\return true if the files are identical, false on any error or difference. */
extern bool xmlCompareFiles(irr::io::IFileSystem * fs, const char * fileName1, const char * fileName2);

//! Compare two images, returning the degree to which they match.
/** \param driver The Irrlicht video driver.
	\param fileName1 The first image to compare.
	\param fileName2 The second image to compare.
	\return The match, from 0.f to 100.f */
extern float fuzzyCompareImages(irr::video::IVideoDriver * driver,
		const char * fileName1, const char * fileName2);

//! Take a screenshot and compare it against a reference screenshot in the tests/media subdirectory
/** \param driver The Irrlicht video driver.
	\param fileName The unique filename suffix that will be appended to the name of the video driver.
	\param requiredMatch The degree to which the screenshot needs to match the reference image
	in order to be considered a match.
	\return true if the screenshot was taken and is identical to the reference image of the same name
	in the tests/media directory, false on any error or difference. */
extern bool takeScreenshotAndCompareAgainstReference(irr::video::IVideoDriver * driver,
													const char * fileName,
													irr::f32 requiredMatch = 99.f);

//! Stabilize the screen background eg. eliminate problems like an aero transparency effects etc.
/** \param driver The Irrlicht video driver.
	\return true if required color is the same as a window background color. */
extern void stabilizeScreenBackground(irr::video::IVideoDriver * driver,
													irr::video::SColor color = irr::video::SColor(255, 255, 255, 255));


//! Opens a test log file, deleting any existing contents.
/** \param startNewLog true to create a new log file, false to append to an
						existing one.
	\param filename The filename to open
	\return true if the test log file was opened, false on error. */
extern bool openTestLog(bool startNewLog, const char * filename = "tests.log");

//! Close the test log file opened with openTestLog()
extern void closeTestLog();

//! Log a string to the console and the test log file created by openTestLog().
/** \param format The format string
	\... optional parameters */
extern void logTestString(const char * format, ...);

//! Return a drivername for the driver which is useable in filenames
extern irr::core::stringc shortDriverName(irr::video::IVideoDriver * driver);

#endif // _TEST_UTILS_H_
