#pragma once

#include <fstream>
#include <string>

using namespace std;

class FileManager
{
public:
	FileManager() = default;
	FileManager(const string inFilename, const string outFilename);
	~FileManager();

	void SetInFile(const string filename);
	void SetOutFile(const string filename);

	const int GetInFileSize();

	string Read(const int size);
	void Write(const string& data);
	void Seek(const int offset);

private:
	string mInFile;
	string mOutFile;

	string mLastString;

	ifstream mInStream;
	ofstream mOutStream;
};