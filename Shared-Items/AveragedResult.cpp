#include "AveragedResult.h"

float AveragedResult::AverageLatency() const
{
	float average = 0;
	for (float offset : Offsets)
	{
		average += offset;
	}
	average /= Offsets.size();
	average -= OutputOffsetFromProfile;
	average -= ReferenceDacLatency;

	return average;
}

float AveragedResult::MinLatency() const
{
	float min = FLT_MAX;
	for (float offset : Offsets)
	{
		if (offset < min)
		{
			min = offset;
		}
	}
	return min - OutputOffsetFromProfile - ReferenceDacLatency;
}

float AveragedResult::MaxLatency() const
{
	float max = -FLT_MAX;
	for (float offset : Offsets)
	{
		if (offset > max)
		{
			max = offset;
		}
	}
	return max - OutputOffsetFromProfile - ReferenceDacLatency;
}
