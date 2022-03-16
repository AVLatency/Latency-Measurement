#include "TestConfiguration.h"

#ifdef _DEBUG
#define IS_DEBUG true
#else
#define IS_DEBUG false
#endif // _DEBUG


bool TestConfiguration::SaveIndividualRecordingResults = IS_DEBUG;
bool TestConfiguration::SaveIndividualWavFiles = IS_DEBUG;
int TestConfiguration::NumMeasurements = 10;
float TestConfiguration::RecordingLegnth = 0.9;
float TestConfiguration::OutputVolume = 1.0f;
bool TestConfiguration::Ch1AutoThresholdDetection = true;
float TestConfiguration::Ch1DetectionThreshold = 0.05f * 2.0f;
bool TestConfiguration::Ch2AutoThresholdDetection = true;
float TestConfiguration::Ch2DetectionThreshold = 0.05f * 2.0f;
int TestConfiguration::AttemptsBeforeFail = 6;
