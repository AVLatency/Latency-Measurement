#pragma once
#include <windows.h>

static class QpcHelper
{
public:
	static LARGE_INTEGER GetFrequency();
	static float GetDifferenceInMilliseconds(LARGE_INTEGER qpcValue1, LARGE_INTEGER qpcValue2);

private:
	static LARGE_INTEGER frequency;
};

