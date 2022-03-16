#pragma once

class TestConfiguration
{
public:
	static bool SaveIndividualRecordingResults;
	static bool SaveIndividualWavFiles;
	static int NumMeasurements;
	/// <summary>
	/// In seconds
	/// </summary>
	static float RecordingLegnth;
	static float OutputVolume;
	static bool Ch1AutoThresholdDetection;
	/// <summary>
	/// For signal detection to work correctly, this threshold must be lager than
	/// any cable crosstalk signals or background noise.
	/// </summary>
	static float Ch1DetectionThreshold;
	static bool Ch2AutoThresholdDetection;
	/// <summary>
	/// For signal detection to work correctly, this threshold must be lager than
	/// any cable crosstalk signals or background noise.
	/// </summary>
	static float Ch2DetectionThreshold;
	static int AttemptsBeforeFail;
};