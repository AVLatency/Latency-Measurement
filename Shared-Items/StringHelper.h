#pragma once
#include <string>

class StringHelper
{
public:
    static std::string GetGuidString();
    static std::string GetTimeString(time_t ttime, bool filenameSafe);
    static std::string GetRootPath(std::string appSubDirectory);
    static std::string GetFilenameSafeString(std::string filename);
};