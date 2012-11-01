#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace io;

static bool testgetAbsoluteFilename(io::IFileSystem* fs)
{
	bool result=true;
	io::path apath = fs->getAbsolutePath("media");
	io::path cwd = fs->getWorkingDirectory();
	if (apath!=(cwd+"/media"))
	{
		logTestString("getAbsolutePath failed on existing dir %s\n", apath.c_str());
		result = false;
	}

	apath = fs->getAbsolutePath("../media/");
	core::deletePathFromPath(cwd, 1);
	if (apath!=(cwd+"media/"))
	{
		logTestString("getAbsolutePath failed on dir with postfix / %s\n", apath.c_str());
		result = false;
	}

	apath = fs->getAbsolutePath ("../nothere.txt");   // file does not exist
	if (apath!=(cwd+"nothere.txt"))
	{
		logTestString("getAbsolutePath failed on non-existing file %s\n", apath.c_str());
		result = false;
	}

	return result;
}

static bool testFlattenFilename(io::IFileSystem* fs)
{
	bool result=true;
	io::path tmpString="../tmp";
	io::path refString="../tmp/";
	fs->flattenFilename(tmpString);
	if (tmpString != refString)
	{
		logTestString("flattening destroys path.\n%s!=%s\n", tmpString.c_str(),refString.c_str());
		result = false;
	}

	tmpString="tmp/tmp/../";
	refString="tmp/";
	fs->flattenFilename(tmpString);
	if (tmpString != refString)
	{
		logTestString("flattening destroys path.\n%s!=%s\n", tmpString.c_str(),refString.c_str());
		result = false;
	}

	tmpString="tmp/tmp/..";
	fs->flattenFilename(tmpString);
	if (tmpString != refString)
	{
		logTestString("flattening destroys path.\n%s!=%s\n", tmpString.c_str(),refString.c_str());
		result = false;
	}

	tmpString="tmp/next/../third";
	refString="tmp/third/";
	fs->flattenFilename(tmpString);
	if (tmpString != refString)
	{
		logTestString("flattening destroys path.\n%s!=%s\n", tmpString.c_str(),refString.c_str());
		result = false;
	}

	tmpString="this/tmp/next/../../my/fourth";
	refString="this/my/fourth/";
	fs->flattenFilename(tmpString);
	if (tmpString != refString)
	{
		logTestString("flattening destroys path.\n%s!=%s\n", tmpString.c_str(),refString.c_str());
		result = false;
	}

	tmpString="this/is/../../../a/fifth/test/";
	refString="../a/fifth/test/";
	fs->flattenFilename(tmpString);
	if (tmpString != refString)
	{
		logTestString("flattening destroys path.\n%s!=%s\n", tmpString.c_str(),refString.c_str());
		result = false;
	}

	tmpString="this/../is/../../a/sixth/test/";
	refString="../a/sixth/test/";
	fs->flattenFilename(tmpString);
	if (tmpString != refString)
	{
		logTestString("flattening destroys path.\n%s!=%s\n", tmpString.c_str(),refString.c_str());
		result = false;
	}

	return result;
}

static bool testgetRelativeFilename(io::IFileSystem* fs)
{
	bool result=true;
	io::path apath = fs->getAbsolutePath("media");
	io::path cwd = fs->getWorkingDirectory();
	if (fs->getRelativeFilename(apath, cwd) != "media")
	{
		logTestString("getRelativePath failed on %s\n", apath.c_str());
		result = false;
	}

	apath = fs->getAbsolutePath("../media/");
	if (fs->getRelativeFilename(apath, cwd) != "../media/")
	{
		logTestString("getRelativePath failed on %s\n", apath.c_str());
		result = false;
	}

	return result;
}

bool filesystem(void)
{
	IrrlichtDevice * device = irr::createDevice(video::EDT_NULL, dimension2d<u32>(1, 1));
	assert_log(device);
	if(!device)
		return false;

	io::IFileSystem * fs = device->getFileSystem ();
	if ( !fs )
		return false;

	bool result = true;

	io::path workingDir = device->getFileSystem()->getWorkingDirectory();

	io::path empty;
	if ( fs->existFile(empty) )
	{
		logTestString("Empty filename should not exist.\n");
		result = false;
	}

	io::path newWd = workingDir + "/media";
	bool changed = device->getFileSystem()->changeWorkingDirectoryTo(newWd);
	assert_log(changed);

	if ( fs->existFile(empty) )
	{
		logTestString("Empty filename should not exist even in another workingdirectory.\n");
		result = false;
	}

	// The working directory must be restored for the other tests to work.
	changed = device->getFileSystem()->changeWorkingDirectoryTo(workingDir.c_str());
	assert_log(changed);

	// adding  a folder archive which just should not really change anything
	device->getFileSystem()->addFileArchive( "./" );

	if ( fs->existFile(empty) )
	{
		logTestString("Empty filename should not exist in folder file archive.\n");
		result = false;
	}

	// remove it again to not affect other tests
	device->getFileSystem()->removeFileArchive( device->getFileSystem()->getFileArchiveCount() );

	result &= testFlattenFilename(fs);
	result &= testgetAbsoluteFilename(fs);
	result &= testgetRelativeFilename(fs);

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

