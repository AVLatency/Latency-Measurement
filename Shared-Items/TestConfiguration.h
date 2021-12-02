#pragma once

class TestConfiguration
{
public:
	static bool SaveIndividualRecordingResults;
	static bool SaveIndividualWavFiles;
	static int NumMeasurements;
	static float OutputVolume;
	static float BaseDetectionThreadshold;
	static float DetectionThresholdMultiplier;
	static int AttemptsBeforeFail;

	static float DetectionThreshold();
};