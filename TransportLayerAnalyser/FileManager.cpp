#include "FileManager.h"

FileManager::FileManager(const string inFilename, const string outFilename)
	: mInFile(inFilename)
	, mOutFile(outFilename)
{
	mInStream.open(inFilename, fstream::in | fstream::binary);
	mOutStream.open(outFilename, fstream::out | fstream::binary);
}

FileManager::~FileManager()
{
	mInStream.close();
	mOutStream.close();
}

void FileManager::SetInFile(const string filename)
{
	if (mInStream.is_open())
	{
		mInStream.close();
	}

	mInFile = filename;
	mInStream.open(mInFile, fstream::in | fstream::binary);
}

void FileManager::SetOutFile(const string filename)
{
	if (mOutStream.is_open())
	{
		mOutStream.close();
	}

	mOutFile = filename;
	mOutStream.open(mOutFile, fstream::out | fstream::binary);
}

const int FileManager::GetInFileSize()
{
	if (!mInStream.is_open())
	{
		return -1;
	}

	const int currentPos = mInStream.tellg();

	mInStream.seekg(0, ios::end);
	const int length = mInStream.tellg();
	mInStream.seekg(currentPos, ios::beg);

	return length;
}

string FileManager::Read(const int size)
{
	char * buffer = new char[size];
	mInStream.get(buffer, size, EOF);
	mLastString = buffer;
	delete buffer;
	return mLastString;
}

void FileManager::Write(const string& data)
{
	mOutStream.write(data.c_str(), data.length());
}

void FileManager::Seek(const int offset)
{
	mInStream.seekg(offset);
}
