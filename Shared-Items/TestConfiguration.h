#pragma once

class TestConfiguration
{
public:
	static float OutputVolume;
	static float BaseDetectionThreadshold;
	static float DetectionThresholdMultiplier;
	static int NumMeasurements;
	static int AttemptsBeforeFail;

	static float DetectionThreshold();
};