#include "TestConfiguration.h"

float TestConfiguration::OutputVolume = 1.0f;
float TestConfiguration::BaseDetectionThreadshold = 0.05f;
float TestConfiguration::DetectionThresholdMultiplier = 1.0f;
int TestConfiguration::NumMeasurements = 10;
int TestConfiguration::AttemptsBeforeFail = 4;

float TestConfiguration::DetectionThreshold()
{
	return BaseDetectionThreadshold * DetectionThresholdMultiplier;
}