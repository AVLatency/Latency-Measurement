#include "TestConfiguration.h"

#ifdef _DEBUG
#define IS_DEBUG true
#else
#define IS_DEBUG false
#endif // _DEBUG


bool TestConfiguration::SaveIndividualRecordingResults = IS_DEBUG;
bool TestConfiguration::SaveIndividualWavFiles = IS_DEBUG;
int TestConfiguration::NumMeasurements = 10;
float TestConfiguration::OutputVolume = 1.0f;
float TestConfiguration::BaseDetectionThreadshold = 0.05f;
float TestConfiguration::DetectionThresholdMultiplier = 1.0f;
int TestConfiguration::AttemptsBeforeFail = 6;

/// <summary>
/// The detection threshold exists for the sole reason of preventing crosstalk
/// from giving incorrectly valid results when one of the channels is not wired
/// up or working correctly.
/// </summary>
float TestConfiguration::DetectionThreshold()
{
	return BaseDetectionThreadshold * DetectionThresholdMultiplier;
}