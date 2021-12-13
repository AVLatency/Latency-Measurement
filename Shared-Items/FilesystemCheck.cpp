#include "FilesystemCheck.h"
#include <fstream>
bool FilesystemCheck::IsFilesystemWritable(std::filesystem::path testFilePath)
{
	bool result = true;
	try
	{
		std::filesystem::create_directories(testFilePath.parent_path());
		std::fstream testFileStream { testFilePath, std::ios_base::app };
		testFileStream << "This is a temporary file that can be safely deleted." << std::endl;
		testFileStream.close();
		std::filesystem::remove(testFilePath);
	}
	catch (...)
	{
		result = false;
	}
	return result;
}
