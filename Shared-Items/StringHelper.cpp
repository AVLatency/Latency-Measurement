#include "StringHelper.h"
#include <atlconv.h>
#include <ctime>
#include <format>

std::string StringHelper::GetGuidString()
{
    GUID guid;
    HRESULT h = CoCreateGuid(&guid);
    OLECHAR* tempStr;
    h = StringFromCLSID(guid, &tempStr);
    USES_CONVERSION;
    std::string guidStdString = OLE2CA(tempStr);
    CoTaskMemFree(tempStr);
    return guidStdString;
}

std::string StringHelper::GetTimeString(time_t ttime, bool filenameSafe)
{
    tm* localTime = localtime(&ttime);
    std::string formatString = filenameSafe ? "{}-{:02}-{:02} {:02}.{:02}.{:02}" : "{}-{:02}-{:02} {:02}:{:02}:{:02}";
    return std::format(formatString, 1900 + localTime->tm_year, 1 + localTime->tm_mon, localTime->tm_mday, localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
}

std::string StringHelper::GetRootPath()
{
    return std::string("Results");
}
