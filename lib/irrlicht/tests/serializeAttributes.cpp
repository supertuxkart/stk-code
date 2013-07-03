#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace io;

#define COMPARE(a, b) if ( (a) != (b) ) { logTestString("Not identical %s in %s:%d\n", #a, __FILE__, __LINE__ ); return false; }

const u32 BINARY_BLOCK_SIZE = 10;

enum EMockEnum
{
	EME_NONE,
	EME_ONE,
	EME_COUNT
};

const c8* const MockEnumNames[EME_COUNT+1] =
{
	"none",
	"one",
	0,
};

class SerializableMock : public virtual io::IAttributeExchangingObject
{
public:
	SerializableMock(bool comparePointers=true) : ComparePointers(comparePointers)
	{
	}

	virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const
	{
		out->addInt("ValInt", ValInt);
		out->addFloat("ValFloat", ValFloat);
		out->addString("ValString", ValString.c_str());
		out->addString("ValStringW", ValStringW.c_str());
		out->addBinary("ValBinary", (void*)ValBinary, BINARY_BLOCK_SIZE);
		out->addArray("ValStringWArray", ValStringWArray);
		out->addBool("ValBool", ValBool);
		out->addEnum("ValEnum", ValEnum, MockEnumNames);
		out->addColor("ValColor", ValColor);
		out->addColorf("ValColorf", ValColorf);
		out->addVector3d("ValVector3df", ValVector3df);
		out->addVector2d("ValVector2df", ValVector2df);
		out->addDimension2d("ValDimension2du", ValDimension2du);
		out->addPosition2d("ValPosition2di", ValPosition2di);
		out->addRect("ValRect", ValRect);
		out->addMatrix("ValMatrix", ValMatrix);
		out->addQuaternion("ValQuaternion", ValQuaternion);
		out->addBox3d("ValAabbox3df", ValAabbox3df);
		out->addPlane3d("ValPlane3df", ValPlane3df);
		out->addTriangle3d("ValTriangle3df", ValTriangle3df);
		out->addLine2d("ValLine2df", ValLine2df);
		out->addLine3d("ValLine3df", ValLine3df);
		out->addTexture("ValTexture", ValTexture );
		out->addUserPointer("ValPointer", ValPointer);
	}

	virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options)
	{
		ValInt = in->getAttributeAsInt("ValInt");
		ValFloat = in->getAttributeAsFloat("ValFloat");
		ValString = in->getAttributeAsString("ValString");
		ValStringW = in->getAttributeAsStringW("ValStringW");
		in->getAttributeAsBinaryData("ValBinary", ValBinary, BINARY_BLOCK_SIZE);
		ValStringWArray = in->getAttributeAsArray("ValStringWArray");
		ValBool = in->getAttributeAsBool("ValBool");
		ValEnum = (EMockEnum)in->getAttributeAsEnumeration("ValEnum", MockEnumNames);
		ValColor = in->getAttributeAsColor("ValColor");
		ValColorf = in->getAttributeAsColorf("ValColorf");
		ValVector3df = in->getAttributeAsVector3d("ValVector3df");
		ValVector2df = in->getAttributeAsVector2d("ValVector2df");
		ValDimension2du = in->getAttributeAsDimension2d("ValDimension2du");
		ValPosition2di = in->getAttributeAsPosition2d("ValPosition2di");
		ValRect = in->getAttributeAsRect("ValRect");
		ValMatrix = in->getAttributeAsMatrix("ValMatrix");
		ValQuaternion = in->getAttributeAsQuaternion("ValQuaternion");
		ValAabbox3df = in->getAttributeAsBox3d("ValAabbox3df");
		ValPlane3df = in->getAttributeAsPlane3d("ValPlane3df");
		ValTriangle3df = in->getAttributeAsTriangle3d("ValTriangle3df");
		ValLine2df = in->getAttributeAsLine2d("ValLine2df");
		ValLine3df = in->getAttributeAsLine3d("ValLine3df");
		ValTexture = in->getAttributeAsTexture("ValTexture");
		ValPointer = in->getAttributeAsUserPointer("ValPointer");
	}

	bool operator==(const SerializableMock& other)
	{
		COMPARE(ValInt, other.ValInt);
		COMPARE(ValFloat, other.ValFloat);
		COMPARE(ValString, other.ValString);
		COMPARE(ValStringW, other.ValStringW);
		if ( memcmp( ValBinary, other.ValBinary, BINARY_BLOCK_SIZE) != 0 )
		{
			logTestString("Not identical %s in %s:%d", "ValBinary",  __FILE__, __LINE__ );
			return false;
		}
		COMPARE(ValStringWArray, other.ValStringWArray);
		COMPARE(ValBool, other.ValBool);
		COMPARE(ValEnum, other.ValEnum);
		COMPARE(ValColor, other.ValColor);
		if ( ValColorf.r != other.ValColorf.r || ValColorf.g != other.ValColorf.g || ValColorf.b != other.ValColorf.b || ValColorf.a != other.ValColorf.a )
		{
			logTestString("Not identical %s in %s:%d", "ValColorf",  __FILE__, __LINE__ );
			return false;
		}
		COMPARE(ValVector3df, other.ValVector3df);
		COMPARE(ValVector2df, other.ValVector2df);
		COMPARE(ValDimension2du, other.ValDimension2du);
		COMPARE(ValPosition2di, other.ValPosition2di);
		COMPARE(ValRect, other.ValRect);
		COMPARE(ValMatrix, other.ValMatrix);
		COMPARE(ValQuaternion, other.ValQuaternion);
		COMPARE(ValAabbox3df, other.ValAabbox3df);
		COMPARE(ValPlane3df, other.ValPlane3df);
		COMPARE(ValTriangle3df, other.ValTriangle3df);
		COMPARE(ValLine2df, other.ValLine2df);
		COMPARE(ValLine3df, other.ValLine3df);
//		ValTexture;	// TODO
		if ( ComparePointers )
			COMPARE(ValPointer, other.ValPointer);
		return true;
	}

	void reset()
	{
		ValInt = 0;
		ValFloat = 0.f;
		ValString = "";
		ValStringW = L"";
		memset(ValBinary, 0, BINARY_BLOCK_SIZE);
		ValStringWArray.clear();
		ValBool = false;
		ValEnum = EME_NONE;
		ValColor.set(0,0,0,0);
		ValColorf.set(0.f, 0.f, 0.f, 0.f);
		ValVector3df.set(0.f, 0.f, 0.f);
		ValVector2df.set(0.f, 0.f);
		ValDimension2du.set(0, 0);
		ValPosition2di.set(0,0);
		ValRect = core::rect<s32>(0,0,0,0);
		ValMatrix.makeIdentity();
		ValQuaternion.set(0,0,0,0);
		ValAabbox3df.reset(0,0,0);
		ValPlane3df.setPlane(vector3df(0.f,0.f,0.f), 0.f);
		ValTriangle3df.set( vector3df(0.f,0.f,0.f), vector3df(0.f,0.f,0.f), vector3df(0.f,0.f,0.f) );
		ValLine2df.setLine(0.f, 0.f, 0.f, 0.f);
		ValLine3df.setLine(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
		ValTexture = NULL;
		ValPointer = 0;
	}

	void set()
	{
		ValInt = 152722522;
		ValFloat = 1.f;
		ValString = "one";
		ValStringW = L"ONE";
		memset(ValBinary, 0xff, BINARY_BLOCK_SIZE);
		ValStringWArray.push_back( stringw("ONE") );
		ValStringWArray.push_back( stringw("TWO") );
		ValStringWArray.push_back( stringw("THREE") );
		ValBool = true;
		ValEnum = EME_ONE;
		ValColor.set(1,2,3,4);
		ValColorf.set(1.f, 2.f, 3.f, 4.f);
		ValVector3df.set(1.f, 2.f, 3.f);
		ValVector2df.set(1.f, 2.f);
		ValDimension2du.set(1, 2);
		ValPosition2di.set(1,2);
		ValRect = core::rect<s32>(1,2,3,4);
		ValMatrix = 99.9f;
		ValQuaternion.set(1,2,3,4);
		ValAabbox3df.reset(1,2,3);
		ValPlane3df.setPlane(vector3df(1.f,2.f,3.f), 4.f);
		ValTriangle3df.set( vector3df(1.f,2.f,3.f), vector3df(4.f,5.f,6.f), vector3df(7.f,8.f,9.f) );
		ValLine2df.setLine(1.f, 2.f, 3.f, 4.f);
		ValLine3df.setLine(1.f, 2.f, 3.f, 4.f, 5.f, 6.f);
		ValTexture = NULL;	// TODO
		ValPointer = (void*)0xffffff;
	}

	s32 						ValInt;
	f32 						ValFloat;
	core::stringc 				ValString;
	core::stringw 				ValStringW;
	char 						ValBinary[BINARY_BLOCK_SIZE];
	core::array<core::stringw> 	ValStringWArray;
	bool 						ValBool;
	EMockEnum					ValEnum;
	video::SColor 				ValColor;
	video::SColorf 				ValColorf;
	core::vector3df 			ValVector3df;
	core::vector2df 			ValVector2df;
	core::dimension2du			ValDimension2du;
	core::position2di 			ValPosition2di;
	core::rect<s32> 			ValRect;
	core::matrix4 				ValMatrix;
	core::quaternion 			ValQuaternion;
	core::aabbox3df 			ValAabbox3df;
	core::plane3df 				ValPlane3df;
	core::triangle3df 			ValTriangle3df;
	core::line2df 				ValLine2df;
	core::line3df 				ValLine3df;
	video::ITexture* 			ValTexture;
	void* 						ValPointer;

	bool ComparePointers;
};

// Serialization in memory
bool MemorySerialization(io::IFileSystem * fs )
{
	SerializableMock origMock, copyMock;
	origMock.set();
	copyMock.reset();

	io::IAttributes* attr = fs->createEmptyAttributes();
	origMock.serializeAttributes(attr, 0);
	copyMock.deserializeAttributes(attr, 0);
	attr->drop();

	return origMock == copyMock;
}

// Serialization to/from an xml-file
bool XmlSerialization(io::IFileSystem * fs, video::IVideoDriver * driver )
{
	const io::path XML_FILENAME("results/attributeSerialization.xml");
	SerializableMock origMock(false), copyMock;
	origMock.set();
	copyMock.reset();

	// write to xml
	io::IWriteFile* fileWrite = fs->createAndWriteFile(XML_FILENAME);
	if (!fileWrite)
	{
		logTestString("Could not create xml in %s:%d\n", __FILE__, __LINE__ );
		return false;
	}
	io::IXMLWriter* writer = fs->createXMLWriter(fileWrite);
	if (!writer)
	{
		logTestString("Could not create xml-writer in %s:%d\n", __FILE__, __LINE__ );
		return false;
	}
	writer->writeXMLHeader();
	writer->writeLineBreak();
	io::IAttributes* attrToXml = fs->createEmptyAttributes();
	origMock.serializeAttributes(attrToXml, 0);
	attrToXml->write(writer);
	attrToXml->drop();
	writer->writeLineBreak();
	writer->drop();
	fileWrite->drop();

	// read from xml
	io::IReadFile* fileRead = fs->createAndOpenFile(XML_FILENAME);
	if (!fileRead)
	{
		logTestString("Could not open xml-file in %s:%d\n", __FILE__, __LINE__ );
		return false;
	}
	io::IXMLReader* reader = fs->createXMLReader(fileRead);
	if (!reader)
	{
		logTestString("createXMLReader failed in %s:%d\n", __FILE__, __LINE__ );
		return false;
	}
	while(reader->read())
	{
		switch (reader->getNodeType())
		{
			case EXN_ELEMENT:
			{
				// read attributes
				io::IAttributes* attr = fs->createEmptyAttributes(driver);
				if ( attr->read(reader, true) )
				{
					copyMock.deserializeAttributes(attr, 0);
				}
				else
				{
					logTestString("attr->read failed in %s:%d\n", __FILE__, __LINE__ );
				}
				attr->drop();
			}
			break;
			default:
			break;
		}
	}
	reader->drop();
	fileRead->drop();

	return origMock == copyMock;
}

bool serializeAttributes()
{
	bool result = true;

	IrrlichtDevice * device = irr::createDevice(video::EDT_NULL, dimension2d<u32>(1, 1));
	assert(device);
	if(!device)
	{
		logTestString("device creation failed in %s:%d\n", __FILE__, __LINE__ );
		return false;
	}

	io::IFileSystem * fs = device->getFileSystem ();
	if ( !fs )
	{
		return false;
	}

	result &= MemorySerialization(fs);
	if ( !result )
	{
		logTestString("MemorySerialization failed in %s:%d\n", __FILE__, __LINE__ );
	}

	result &= XmlSerialization(fs, device->getVideoDriver());
	if ( !result )
	{
		logTestString("XmlSerialization failed in %s:%d\n", __FILE__, __LINE__ );
	}

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}
