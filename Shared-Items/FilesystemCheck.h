#pragma once
#include <filesystem>

class FilesystemCheck
{
public:
	static bool IsFilesystemWritable(std::filesystem::path testFilePath);
};

