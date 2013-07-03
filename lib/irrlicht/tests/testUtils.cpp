// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS 1
#define TESTING_ON_WINDOWS
#endif // _MSC_VER

#include "testUtils.h"
#include <memory.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

#if defined(TESTING_ON_WINDOWS)
#include <windows.h> // For OutputDebugString()
#endif // #if defined(TESTING_ON_WINDOWS)

using namespace irr;

bool binaryCompareFiles(const char * fileName1, const char * fileName2)
{
	assert(fileName1);
	assert(fileName2);
	if(!fileName1 || !fileName2)
		return false;

	FILE * file1 = fopen(fileName1, "rb");
	if(!file1)
	{
		logTestString("binaryCompareFiles: File '%s' cannot be opened\n", fileName1);
		assert(file1);
		return false;
	}

	FILE * file2 = fopen(fileName2, "rb");
	if(!file2)
	{
		logTestString("binaryCompareFiles: File '%s' cannot be opened\n", fileName2);
		(void)fclose(file1);
		assert(file2);
		return false;
	}


	(void)fseek(file1, 0, SEEK_END);
	(void)fseek(file2, 0, SEEK_END);
	const size_t file1Size = ftell(file1);
	const size_t file2Size = ftell(file2);
	if(file1Size != file2Size)
	{
		logTestString("binaryCompareFiles: Files are different sizes: %d vs %d\n",
			file1Size, file2Size);
		(void)fclose(file1);
		(void)fclose(file2);
		return false;
	}

	(void)fseek(file1, 0, SEEK_SET);
	(void)fseek(file2, 0, SEEK_SET);

	char file1Buffer[8196];
	char file2Buffer[8196];

	while(!feof(file1))
	{
		if(feof(file2)
			||(fread(file1Buffer, sizeof(file1Buffer), 1, file1) !=
			fread(file2Buffer, sizeof(file2Buffer), 1, file2)))
		{
			logTestString("binaryCompareFiles: Error during file reading\n");
			break;
		}

		if(memcmp(file1Buffer, file2Buffer, sizeof(file1Buffer)))
		{
			logTestString("binaryCompareFiles: files are different\n");
			break;
		}
	}

	bool filesAreIdentical = feof(file1) && feof(file2);
	(void)fclose(file1);
	(void)fclose(file2);

	return filesAreIdentical;
}

bool xmlCompareFiles(irr::io::IFileSystem * fs, const char * fileName1, const char * fileName2)
{
	if(!fileName1 || !fileName2)
		return false;

	io::IXMLReaderUTF8* reader1 = fs->createXMLReaderUTF8(fileName1);
	if (!reader1)
	{
		logTestString("xmlCompareFiles: Could not create a XML reader for '%s'\n", fileName1);
		return false;
	}
	io::IXMLReaderUTF8* reader2 = fs->createXMLReaderUTF8(fileName2);
	if (!reader2)
	{
		logTestString("xmlCompareFiles: Could not create a XML reader for '%s'\n", fileName2);
		reader1->drop();
		return false;
	}

	bool different = false;
	bool read1 = reader1->read();
	bool read2 = reader2->read();
	if ( !read1 && !read2 )
	{
		logTestString("xmlCompareFiles: Both files have no nodes: '%s' %s - is this ok?\n", fileName1, fileName2);
		reader1->drop();
		reader2->drop();
		return true;
	}

	while (read1 && read2)
	{
		io::EXML_NODE type1 = reader1->getNodeType();
		io::EXML_NODE type2 = reader2->getNodeType();
		if ( type1 != type2 )
		{
			const c8* name1 = reader1->getNodeName();
			const c8* name2 = reader2->getNodeName();
			logTestString("xmlCompareFiles: file '%s' has different nodes than %s in nodes \"%s\" and \"%s\"\n"
							, fileName1, fileName2, name1 ? name1 : "NULL", name2 ? name2 : "NULL");
			different = true;
			break;
		}

		if ( reader1->isEmptyElement() != reader2->isEmptyElement() )
		{
			logTestString("xmlCompareFiles: file '%s' has different empty elements than %s\n", fileName1, fileName2);
			different = true;
			break;
		}

		switch ( type1 )
		{
			case io::EXN_NONE:
			case io::EXN_ELEMENT_END:
				break;

			case io::EXN_ELEMENT:
			{
				core::stringc name1(reader1->getNodeName());
				core::stringc name2(reader2->getNodeName());
				if ( name1 != name2 )
				{
					logTestString("xmlCompareFiles: %s has node %s where %s has node %s\n"
									, fileName1, name1.c_str(), fileName2, name2.c_str() );
					different = true;
					break;
				}

				unsigned int numAttributes1 = reader1->getAttributeCount();
				unsigned int numAttributes2 = reader2->getAttributeCount();
				if ( numAttributes1 != numAttributes2 )
				{
					logTestString("xmlCompareFiles: %s node %s has %d attributes where %s node %s has %d attributes\n"
									, fileName1, name1.c_str(), numAttributes1
									, fileName2, name2.c_str(), numAttributes2);
					different = true;
					break;
				}

				for ( unsigned int i=0; i < numAttributes1; ++i )
				{
					core::stringc attribName1(reader1->getAttributeName(int(i)));
					core::stringc attribName2(reader2->getAttributeName(int(i)));

					if ( attribName1 != attribName2 )
					{
						logTestString("xmlCompareFiles: %s node %s has attribute-name \"%s\" where %s node %s has name \"%s\"\n"
									, fileName1, name1.c_str(), attribName1.c_str()
									, fileName2, name2.c_str(), attribName2.c_str());
						different = true;
						break;
					}

					core::stringc attribVal1(reader1->getAttributeValue(int(i)));
					core::stringc attribVal2(reader2->getAttributeValue(int(i)));
					if ( attribName1 != attribName2 )
					{
						logTestString("xmlCompareFiles: %s node %s has attribute-value \"%s\" where %s node %s has value \"%s\"\n"
									, fileName1, name1.c_str(), attribVal1.c_str()
									, fileName2, name2.c_str(), attribVal2.c_str());
						different = true;
						break;
					}
				}

				break;
			}

			case io::EXN_TEXT:
			case io::EXN_UNKNOWN:
			case io::EXN_COMMENT:
			case io::EXN_CDATA:
			{
				core::stringc nodeData1( reader1->getNodeData() );
				core::stringc nodeData2( reader1->getNodeData() );

				// removeChars('\r') needed because irrXML doesn't do that (as it probably should)
				nodeData1.removeChars(core::stringc('\r'));
				nodeData2.removeChars(core::stringc('\r'));

				if ( nodeData1 != nodeData2 )
				{
					logTestString("xmlCompareFiles: %s has data \"%s\" where %s has data \"%s\"\n"
									, fileName1, nodeData1.c_str()
									, fileName2, nodeData2.c_str());
					different = true;
				}
				break;
			}
		}

		if ( different )
			break;

		read1 = reader1->read();
		read2 = reader2->read();
	}

	if ( !different && !read1 && !read2 )
	{
		reader1->drop();
		reader2->drop();
		return true;
	}

	if ( !different && read1 )
	{
		logTestString("xmlCompareFiles: file '%s' has more nodes than %s\n", fileName1, fileName2);
	}
	if ( !different && read2 )
	{
		logTestString("xmlCompareFiles: file '%s' has more nodes than %s\n", fileName2, fileName1);
	}

	reader1->drop();
	reader2->drop();

	return false;
}


//! Compare two images, returning the degree to which they match.
/** \param image1 The first image to compare.
	\param image2 The second image to compare.
	\return The match, from 0.f to 100.f */
static float fuzzyCompareImages(irr::video::IImage * image1,
								irr::video::IImage * image2)
{
	assert(image1);
	assert(image2);
	if(!image1 || !image2)
		return 0.f;

	if(image1->getDimension() != image2->getDimension())
	{
		logTestString("fuzzyCompareImages: images are different sizes\n");
		return 0.f;
	}

	video::ECOLOR_FORMAT format1 = image1->getColorFormat();
	if(video::ECF_A8R8G8B8 != format1 && video::ECF_R8G8B8 != format1)
	{
		logTestString("fuzzyCompareImages: image 1 must be ECF_AR8G8B8 or ECF_R8G8B8\n");
		return 0.f;
	}

	video::ECOLOR_FORMAT format2 = image2->getColorFormat();
	if(video::ECF_A8R8G8B8 != format2 && video::ECF_R8G8B8 != format2)
	{
		logTestString("fuzzyCompareImages: image 2 must be ECF_AR8G8B8 or ECF_R8G8B8\n");
		return 0.f;
	}

	u8 * image1Data = (u8*)image1->lock();
	u8 * image2Data = (u8*)image2->lock();

	const u32 pixels = (image1->getPitch() * image1->getDimension().Height) / 4;
	u32 mismatchedColours = 0;
	for(u32 pixel = 0; pixel < pixels; ++pixel)
	{
		if(video::ECF_A8R8G8B8 == format1)
			image1Data++;

		const u8 r1 = *(image1Data++);
		const u8 g1 = *(image1Data++);
		const u8 b1 = *(image1Data++);

		if(video::ECF_A8R8G8B8 == format2)
			image2Data++;

		const u8 r2 = *(image2Data++);
		const u8 g2 = *(image2Data++);
		const u8 b2 = *(image2Data++);

		mismatchedColours += abs(r1 - r2) + abs(g1 - g2) + abs(b1 - b2);
	}

	image1->unlock();
	image2->unlock();

	const u32 totalColours = pixels * 255*3;
	return 100.f * (totalColours - mismatchedColours) / totalColours;
}


//! Compare two images, returning the degree to which they match.
/** \param image1 The first image to compare.
	\param image2 The second image to compare.
	\return The match, from 0.f to 100.f */
float fuzzyCompareImages(irr::video::IVideoDriver * driver,
		const char * fileName1, const char * fileName2)
{
	assert(fileName1);
	assert(fileName2);
	irr::video::IImage * img1 = driver->createImageFromFile(fileName1);
	if (!img1)
		return 0;
	irr::video::IImage * img2 = driver->createImageFromFile(fileName2);
	const float result = fuzzyCompareImages(img1, img2);
	logTestString("Image match: %f%%\n", result);
	img1->drop();
	if (img2)
		img2->drop();
	return result;
}

void stabilizeScreenBackground(irr::video::IVideoDriver * driver,
		irr::video::SColor color)
{
	for(int i = 0; i < 100; ++i) // 100 - max checks
	{
		driver->beginScene(true, true, color);
		driver->endScene();

		irr::video::IImage * screenshot = driver->createScreenShot();
		if(!screenshot)
			return;

		const video::ECOLOR_FORMAT format = screenshot->getColorFormat();
		if(format != video::ECF_R8G8B8)
		{
			irr::video::IImage * fixedScreenshot = driver->createImage(video::ECF_R8G8B8, screenshot->getDimension());
			screenshot->copyTo(fixedScreenshot);
			screenshot->drop();

			if(!fixedScreenshot)
				return;

			screenshot = fixedScreenshot;
		}

		u8 * image1Data = (u8*)screenshot->lock();

		const u32 pixels = (screenshot->getPitch() * screenshot->getDimension().Height) / 4;
		bool status = true;
		for(u32 pixel = 0; pixel < pixels; ++pixel)
		{
			const u8 r = *(image1Data++);
			const u8 g = *(image1Data++);
			const u8 b = *(image1Data++);

			if(r != color.getRed() || g != color.getGreen() || b != color.getBlue())
			{
				status = false;
				break;
			}
		}

		if(status)
		{
			screenshot->drop();
			return;
		}
		screenshot->drop();
	}
}

irr::core::stringc shortDriverName(irr::video::IVideoDriver * driver)
{
	irr::core::stringc driverName = driver->getName();

	// For OpenGL and Burning, chop the version number out. Other drivers have more stable version numbers.
	// TA: Sorry Rogerborg. burnings video also has the version number inside;-)
	//     maybe you sould take the getDriverType Info for this
	if(driverName.find("OpenGL") > -1)
		driverName = "OpenGL";
	else if(driverName.find("Burning's Video") > -1)
		driverName = "Burning's Video";

	return driverName;
}

bool takeScreenshotAndCompareAgainstReference(irr::video::IVideoDriver * driver,
					const char * fileName,
					irr::f32 requiredMatch)
{
	irr::video::IImage * screenshot = driver->createScreenShot();
	if(!screenshot)
	{
		logTestString("Failed to take screenshot\n");
		assert(false);
		return false;
	}

	const video::ECOLOR_FORMAT format = screenshot->getColorFormat();
	if(format != video::ECF_R8G8B8)
	{
		irr::video::IImage * fixedScreenshot = driver->createImage(video::ECF_R8G8B8, screenshot->getDimension());
		screenshot->copyTo(fixedScreenshot);
		screenshot->drop();

		if(!fixedScreenshot)
		{
			logTestString("Failed to convert screenshot to ECF_A8R8G8B8\n");
			assert(false);
			return false;
		}

		screenshot = fixedScreenshot;
	}

	irr::core::stringc driverName = shortDriverName(driver);

	irr::core::stringc referenceFilename = "media/";
	referenceFilename += driverName;
	referenceFilename += fileName;
	irr::video::IImage * reference = driver->createImageFromFile(referenceFilename.c_str());
	if(!reference)
	{
		logTestString("\n*** Failed to load reference image '%s'\n*** Creating from screenshot - please check this image.\n\n",
			referenceFilename.c_str());
		(void)driver->writeImageToFile(screenshot, referenceFilename.c_str());
		screenshot->drop();
		return false;
	}

	const float match = fuzzyCompareImages(screenshot, reference);
	logTestString("Image match: %f%%\n", match);

	if (match < requiredMatch)
	{
		irr::core::stringc mismatchFilename = "results/";
		mismatchFilename += driverName;
		mismatchFilename += fileName;
		logTestString("Writing mismatched image to '%s'\n", mismatchFilename.c_str());
		(void)driver->writeImageToFile(screenshot, mismatchFilename.c_str());
	}


	screenshot->drop();
	reference->drop();

	return (match >= requiredMatch);
}

static FILE * logFile = 0;

bool openTestLog(bool startNewLog, const char * filename)
{
	closeTestLog();

	if(startNewLog)
		logFile = fopen(filename, "w");
	else
		logFile = fopen(filename, "a");

	assert(logFile);
	if(!logFile)
		logTestString("\nWARNING: unable to open the test log file %s\n", filename);

	return (logFile != 0);
}

void closeTestLog(void)
{
	if (logFile)
	{
		(void)fclose(logFile);
		logFile = 0;
	}
}


void logTestString(const char * format, ...)
{
	char logString[1024];

	va_list arguments;
	va_start(arguments, format);
	vsprintf(logString, format, arguments);
	va_end(arguments);

#if defined(_IRR_WINDOWS_API_)
#if defined (_WIN32_WCE )
	core::stringw tmp(logString);
	tmp += L"\n";
	OutputDebugStringW(tmp.c_str());
#else
	OutputDebugStringA(logString);
	OutputDebugStringA("\n");
#endif
#endif

	(void)printf(logString);
	if(logFile)
	{
		(void)fprintf(logFile, logString);
		(void)fflush(logFile);
	}

#if defined(TESTING_ON_WINDOWS)
	OutputDebugStringA(logString);
#endif // #if defined(TESTING_ON_WINDOWS)
}

