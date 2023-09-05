#include "QpcHelper.h"

LARGE_INTEGER QpcHelper::frequency = { .QuadPart = 0 };

LARGE_INTEGER QpcHelper::GetFrequency()
{
	if (frequency.QuadPart == 0)
	{
		QueryPerformanceFrequency(&frequency);
	}
	return frequency;
}

float QpcHelper::GetDifferenceInMilliseconds(LARGE_INTEGER qpcValue1, LARGE_INTEGER qpcValue2)
{
	//
	// We now have the elapsed number of ticks, along with the
	// number of ticks-per-second. We use these values
	// to convert to the number of elapsed microseconds.
	// To guard against loss-of-precision, we convert
	// to microseconds *before* dividing by ticks-per-second.
	//
	LARGE_INTEGER ElapsedMicroseconds;
	ElapsedMicroseconds.QuadPart = qpcValue2.QuadPart - qpcValue1.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= QpcHelper::GetFrequency().QuadPart;

	return ElapsedMicroseconds.QuadPart / (float)1000;
}
