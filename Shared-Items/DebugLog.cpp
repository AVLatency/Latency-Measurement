#include "DebugLog.h"
#include <Windows.h>

void DebugLog::Print(const wchar_t* message)
{
	OutputDebugString(message);
}