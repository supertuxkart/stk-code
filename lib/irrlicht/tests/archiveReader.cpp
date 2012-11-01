#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace io;

namespace
{

bool testArchive(IFileSystem* fs, const io::path& archiveName)
{
	// make sure there is no archive mounted
	if ( fs->getFileArchiveCount() )
	{
		logTestString("Already mounted archives found\n");
		return false;
	}

	if ( !fs->addFileArchive(archiveName, /*bool ignoreCase=*/true, /*bool ignorePaths=*/false) )
	{
		logTestString("Mounting archive failed\n");
		return false;
	}

	// make sure there is an archive mounted
	if ( !fs->getFileArchiveCount() )
	{
		logTestString("Mounted archive not in list\n");
		return false;
	}

	// mount again
	if ( !fs->addFileArchive(archiveName, /*bool ignoreCase=*/true, /*bool ignorePaths=*/false) )
	{
		logTestString("Mounting a second time failed\n");
		fs->removeFileArchive(fs->getFileArchiveCount()-1);
		return false;
	}

	// make sure there is exactly one archive mounted
	if ( fs->getFileArchiveCount() != 1 )
	{
		logTestString("Duplicate mount not recognized\n");
		while (fs->getFileArchiveCount())
			fs->removeFileArchive(fs->getFileArchiveCount()-1);
		return false;
	}
	if (fs->getFileArchive(0)->getType()==io::EFAT_FOLDER)
	{
		// mount again with different path end symbol (either with slash or without)
		core::stringc newArchiveName=archiveName;
		if (archiveName.lastChar()=='/')
			newArchiveName.erase(newArchiveName.size()-1);
		else
			newArchiveName.append('/');
		if ( !fs->addFileArchive(newArchiveName, /*bool ignoreCase=*/true, /*bool ignorePaths=*/false) )
		{
			logTestString("Mounting a second time with different name failed\n");
			fs->removeFileArchive(fs->getFileArchiveCount()-1);
			return false;
		}

		// make sure there is exactly one archive mounted
		if ( fs->getFileArchiveCount() != 1 )
		{
			logTestString("Duplicate mount with different filename not recognized\n");
			while (fs->getFileArchiveCount())
				fs->removeFileArchive(fs->getFileArchiveCount()-1);
			return false;
		}
	}

#if 0
	// log what we got
	io::IFileArchive* archive = fs->getFileArchive(fs->getFileArchiveCount()-1);
	const io::IFileList* fileList = archive->getFileList();
	for ( u32 f=0; f < fileList->getFileCount(); ++f)
	{
		logTestString("File name: %s\n", fileList->getFileName(f).c_str());
		logTestString("Full path: %s\n", fileList->getFullFileName(f).c_str());
		logTestString("ID: %d\n", fileList->getID(f));
	}
#endif

	io::path filename("mypath/mypath/myfile.txt");
	if (!fs->existFile(filename))
	{
		logTestString("existFile with deep path failed\n");
		while (fs->getFileArchiveCount())
			fs->removeFileArchive(fs->getFileArchiveCount()-1);
		return false;
	}

	const char* names[] = {"test/test.txt", "mypath/myfile.txt", "mypath/mypath/myfile.txt"};
	const char* basenames[] = {"test.txt", "myfile.txt", "myfile.txt"};
	const char* content[] = {"Hello world!", "1est\n", "2est"};

	for (u32 i=0; i<3; ++i)
	{
		if (!fs->existFile(names[i]))
		{
			logTestString("existFile failed\n");
			while (fs->getFileArchiveCount())
				fs->removeFileArchive(fs->getFileArchiveCount()-1);
			return false;
		}

		IReadFile* readFile = fs->createAndOpenFile(names[i]);
		if (!readFile)
		{
			logTestString("createAndOpenFile failed\n");
			while (fs->getFileArchiveCount())
				fs->removeFileArchive(fs->getFileArchiveCount()-1);
			return false;
		}

		if (fs->getFileBasename(readFile->getFileName()) != basenames[i])
		{
			logTestString("Wrong filename, file list seems to be corrupt\n");
			while (fs->getFileArchiveCount())
				fs->removeFileArchive(fs->getFileArchiveCount()-1);
			readFile->drop();
			return false;
		}
		char tmp[13] = {'\0'};
		readFile->read(tmp, 12);
		if (strcmp(tmp, content[i]))
		{
			logTestString("Read bad data from archive: %s\n", tmp);
			while (fs->getFileArchiveCount())
				fs->removeFileArchive(fs->getFileArchiveCount()-1);
			readFile->drop();
			return false;
		}
		readFile->drop();
	}

	if (!fs->removeFileArchive(fs->getFileArchiveCount()-1))
	{
		logTestString("Couldn't remove archive.\n");
		return false;
	}

	// make sure there is no archive mounted
	if ( fs->getFileArchiveCount() )
		return false;

	return true;
}

bool testEncryptedZip(IFileSystem* fs)
{
	// make sure there is no archive mounted
	if ( fs->getFileArchiveCount() )
	{
		logTestString("Already mounted archives found\n");
		return false;
	}

	const char* archiveName = "media/enc.zip";
	if ( !fs->addFileArchive(archiveName, /*bool ignoreCase=*/true, /*bool ignorePaths=*/false) )
	{
		logTestString("Mounting archive failed\n");
		return false;
	}

	// make sure there is an archive mounted
	if ( !fs->getFileArchiveCount() )
	{
		logTestString("Mounted archive not in list\n");
		return false;
	}

	// mount again
	if ( !fs->addFileArchive(archiveName, /*bool ignoreCase=*/true, /*bool ignorePaths=*/false) )
	{
		logTestString("Mounting a second time failed\n");
		fs->removeFileArchive(fs->getFileArchiveCount()-1);
		return false;
	}

	// make sure there is exactly one archive mounted
	if ( fs->getFileArchiveCount() != 1 )
	{
		logTestString("Duplicate mount not recognized\n");
		while (fs->getFileArchiveCount())
			fs->removeFileArchive(fs->getFileArchiveCount()-1);
		return false;
	}

	// log what we got
	io::IFileArchive* archive = fs->getFileArchive(fs->getFileArchiveCount()-1);
	io::path filename("doc");
	const io::IFileList* fileList = archive->getFileList();
	for ( u32 f=0; f < fileList->getFileCount(); ++f)
	{
		logTestString("%s name: %s\n", fileList->isDirectory(f)?"Directory":"File", fileList->getFileName(f).c_str());
		logTestString("Full path: %s\n", fileList->getFullFileName(f).c_str());
	}
	if (fileList->findFile(filename) != -1)
	{
		logTestString("findFile wrongly succeeded on directory\n");
		fs->removeFileArchive(fs->getFileArchiveCount()-1);
		return false;
	}
	if (fileList->findFile(filename, true)==-1)
	{
		logTestString("findFile failed on directory\n");
		fs->removeFileArchive(fs->getFileArchiveCount()-1);
		return false;
	}

	filename="doc/readme.txt";
	if (fileList->findFile(filename)==-1)
	{
		logTestString("findFile failed\n");
		fs->removeFileArchive(fs->getFileArchiveCount()-1);
		return false;
	}
	if (fileList->findFile(filename, true) != -1)
	{
		logTestString("findFile wrongly succeeded on non-directory\n");
		fs->removeFileArchive(fs->getFileArchiveCount()-1);
		return false;
	}

	if (!fs->existFile(filename))
	{
		logTestString("existFile failed\n");
		fs->removeFileArchive(fs->getFileArchiveCount()-1);
		return false;
	}

	filename="doc";
	if (fs->existFile(filename))
	{
		logTestString("existFile succeeded wrongly on directory\n");
		fs->removeFileArchive(fs->getFileArchiveCount()-1);
		return false;
	}

	filename="doc/readme.txt";
	IReadFile* readFile = fs->createAndOpenFile(filename);
	if ( readFile )
	{
		logTestString("createAndOpenFile succeeded, even though no password was set.\n");
		readFile->drop();
		fs->removeFileArchive(fs->getFileArchiveCount()-1);
		return false;
	}

	archive->Password="33445";
	readFile = fs->createAndOpenFile(filename);
#ifdef _IRR_COMPILE_WITH_ZIP_ENCRYPTION_
	if ( !readFile )
	{
		logTestString("createAndOpenFile failed\n");
		fs->removeFileArchive(fs->getFileArchiveCount()-1);
		return false;
	}

	char tmp[13] = {'\0'};
	readFile->read(tmp, 12);
	readFile->drop();
	if (strncmp(tmp, "Linux Users:", 12))
	{
		logTestString("Read bad data from archive: %s\n", tmp);
		return false;
	}
#endif

	if (!fs->removeFileArchive(fs->getFileArchiveCount()-1))
	{
		logTestString("Couldn't remove archive.\n");
		return false;
	}

	// make sure there is no archive mounted
	if ( fs->getFileArchiveCount() )
		return false;

	return true;
}

bool testSpecialZip(IFileSystem* fs, const char* archiveName, const char* filename, const void* content)
{
	// make sure there is no archive mounted
	if ( fs->getFileArchiveCount() )
	{
		logTestString("Already mounted archives found\n");
		return false;
	}

	if ( !fs->addFileArchive(archiveName, /*bool ignoreCase=*/true, /*bool ignorePaths=*/false) )
	{
		logTestString("Mounting archive failed\n");
		return false;
	}

	// make sure there is an archive mounted
	if ( !fs->getFileArchiveCount() )
	{
		logTestString("Mounted archive not in list\n");
		return false;
	}

	// log what we got
	io::IFileArchive* archive = fs->getFileArchive(fs->getFileArchiveCount()-1);
	const io::IFileList* fileList = archive->getFileList();
	for ( u32 f=0; f < fileList->getFileCount(); ++f)
	{
		logTestString("%s name: %s\n", fileList->isDirectory(f)?"Directory":"File", fileList->getFileName(f).c_str());
		logTestString("Full path: %s\n", fileList->getFullFileName(f).c_str());
	}

	if (!fs->existFile(filename))
	{
		logTestString("existFile failed\n");
		fs->removeFileArchive(fs->getFileArchiveCount()-1);
		return false;
	}

	IReadFile* readFile = fs->createAndOpenFile(filename);
	if ( !readFile )
	{
		logTestString("createAndOpenFile failed\n");
		fs->removeFileArchive(fs->getFileArchiveCount()-1);
		return false;
	}

	char tmp[6] = {'\0'};
	readFile->read(tmp, 5);
	if (memcmp(tmp, content, 5))
	{
		logTestString("Read bad data from archive: %s\n", tmp);
		readFile->drop();
		fs->removeFileArchive(fs->getFileArchiveCount()-1);
		return false;
	}

	readFile->drop();

	if (!fs->removeFileArchive(fs->getFileArchiveCount()-1))
	{
		logTestString("Couldn't remove archive.\n");
		return false;
	}

	// make sure there is no archive mounted
	if ( fs->getFileArchiveCount() )
		return false;

	return true;
}

static bool testMountFile(IFileSystem* fs)
{
	bool result = true;
#if 1
	fs->changeWorkingDirectoryTo("empty");
	// log what we got
	const io::IFileList* fileList = fs->createFileList();
	for ( u32 f=0; f < fileList->getFileCount(); ++f)
	{
		logTestString("File name: %s\n", fileList->getFileName(f).c_str());
		logTestString("Full path: %s\n", fileList->getFullFileName(f).c_str());
		logTestString("ID: %d\n", fileList->getID(f));
	}
	fileList->drop();
	fs->changeWorkingDirectoryTo("..");
#endif
	if (!fs->addFileArchive("empty"), false)
		result = false;
	const IFileList* list = fs->getFileArchive(0)->getFileList();
#if 1
	// log what we got
	io::IFileArchive* archive = fs->getFileArchive(fs->getFileArchiveCount()-1);
	fileList = archive->getFileList();
	for ( u32 f=0; f < fileList->getFileCount(); ++f)
	{
		logTestString("File name: %s\n", fileList->getFileName(f).c_str());
		logTestString("Full path: %s\n", fileList->getFullFileName(f).c_str());
		logTestString("ID: %d\n", fileList->getID(f));
	}
#endif

	if (list->getFileName(0) != "burnings video 0.39b.png")
		result = false;
	return result;
}

bool testAddRemove(IFileSystem* fs, const io::path& archiveName)
{
	// make sure there is no archive mounted
	if ( fs->getFileArchiveCount() )
	{
		logTestString("Already mounted archives found\n");
		return false;
	}

	if ( !fs->addFileArchive(archiveName, /*bool ignoreCase=*/true, /*bool ignorePaths=*/false) )
	{
		logTestString("Mounting archive failed\n");
		return false;
	}

	// make sure there is an archive mounted
	if ( !fs->getFileArchiveCount() )
	{
		logTestString("Mounted archive not in list\n");
		return false;
	}

	if (!fs->removeFileArchive(archiveName))
	{
		logTestString("Couldn't remove archive.\n");
		return false;
	}

	// make sure there is no archive mounted
	if ( fs->getFileArchiveCount() )
		return false;

	return true;
}
}


bool archiveReader()
{
	IrrlichtDevice * device = irr::createDevice(video::EDT_NULL, dimension2d<u32>(1, 1));
	assert_log(device);
	if(!device)
		return false;

	io::IFileSystem * fs = device->getFileSystem ();
	if ( !fs )
		return false;

	bool ret = true;
	logTestString("Testing mount file.\n");
	ret &= testArchive(fs, "media/file_with_path");
	logTestString("Testing mount file.\n");
	ret &= testArchive(fs, "media/file_with_path/");
	logTestString("Testing zip files.\n");
	ret &= testArchive(fs, "media/file_with_path.zip");
	logTestString("Testing pak files.\n");
	ret &= testArchive(fs, "media/sample_pakfile.pak");
	logTestString("Testing npk files.\n");
	ret &= testArchive(fs, "media/file_with_path.npk");
	logTestString("Testing encrypted zip files.\n");
	ret &= testEncryptedZip(fs);
	logTestString("Testing special zip files.\n");
	ret &= testSpecialZip(fs, "media/Monty.zip", "monty/license.txt", "Monty");
	logTestString("Testing special zip files lzma.\n");
	const u8 buf[] = {0xff, 0xfe, 0x3c, 0x00, 0x3f};
	ret &= testSpecialZip(fs, "media/lzmadata.zip", "tahoma10_.xml", buf);
//	logTestString("Testing complex mount file.\n");
//	ret &= testMountFile(fs);
	logTestString("Testing add/remove with filenames.\n");
	ret &= testAddRemove(fs, "media/file_with_path.zip");

	device->closeDevice();
	device->run();
	device->drop();

	return ret;
}

