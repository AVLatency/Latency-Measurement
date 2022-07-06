#include "StringHelper.h"
#include <Objbase.h>
#include <ctime>
#include <format>

std::string StringHelper::GetGuidString()
{
    GUID guid;
    HRESULT h = CoCreateGuid(&guid);
    LPOLESTR tempStr;
    h = StringFromCLSID(guid, &tempStr);
    std::wstring ws(tempStr); // LPOLESTR is basically a wchar_t* ( https://stackoverflow.com/a/7751580/1123295 )
    std::string guidStdString(ws.begin(), ws.end()); // it's just a GUID, so it doesn't have anything more than ASCII, so use this method of converting: https://stackoverflow.com/a/6623809/1123295
    CoTaskMemFree(tempStr);
    return guidStdString;
}

std::string StringHelper::GetTimeString(time_t ttime, bool filenameSafe)
{
    tm* localTime = localtime(&ttime);
    std::string formatString = filenameSafe ? "{}-{:02}-{:02} {:02}.{:02}.{:02}" : "{}-{:02}-{:02} {:02}:{:02}:{:02}";
    return std::vformat(formatString, std::make_format_args(1900 + localTime->tm_year, 1 + localTime->tm_mon, localTime->tm_mday, localTime->tm_hour, localTime->tm_min, localTime->tm_sec));
}

std::string StringHelper::GetRootPath(std::string appSubDirectory)
{
    return std::format(".\\{} Results", appSubDirectory);
}

std::string StringHelper::GetFilenameSafeString(std::string filename)
{
    const std::string cAllowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890.-+=_' !@#$%^&(){}[]~`";

    auto new_end = std::remove_if(filename.begin(), filename.end(),
        [cAllowed](std::string::value_type c)
        {
            return cAllowed.find(c) == std::string::npos;
        });
    filename.erase(new_end, filename.end());

    return filename;
}
