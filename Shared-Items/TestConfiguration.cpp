#include "TestConfiguration.h"



bool TestConfiguration::SaveIndividualRecordingResults = false;
bool TestConfiguration::SaveIndividualWavFiles = false;
int TestConfiguration::NumMeasurements = 10;
float TestConfiguration::OutputVolume = 1.0f;
float TestConfiguration::BaseDetectionThreadshold = 0.05f;
float TestConfiguration::DetectionThresholdMultiplier = 1.0f;
int TestConfiguration::AttemptsBeforeFail = 4;

float TestConfiguration::DetectionThreshold()
{
	return BaseDetectionThreadshold * DetectionThresholdMultiplier;
}