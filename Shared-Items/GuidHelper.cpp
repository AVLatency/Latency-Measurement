#include "GuidHelper.h"
#include <atlconv.h>

std::string GuidHelper::GetGuidString()
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