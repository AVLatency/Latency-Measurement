#pragma once

class TestConfiguration
{
public:
	static bool SaveIndividualRecordingResults;
	static bool SaveIndividualWavFiles;
	static int NumMeasurements;
	/// <summary>
	/// In seconds. This is the end padding that is added to the end of the recording samples.
	/// It will increase the overall recording time.
	/// </summary>
	static float AdditionalRecordingTime;
	/// <summary>
	/// In seconds. This will increase the overall recording time, but only during file playback.
	/// The base recording time will be the duration of the file.
	/// </summary>
	static float AdditionalRecordingTimeForFilePlayback;
	/// <summary>
	/// In milliseconds. This is the time during the start of the recording that will be ignored.
	/// </summary>
	static int InitialIgnoreLength;
	static float OutputVolume;
	static float LeadInDuration;
	static float LeadInToneAmplitude;
	static bool LowFreqPitch;
	static float AutoThresholdMultiplier;

	static bool Ch1AutoThresholdDetection;
	/// <summary>
	/// For signal detection to work correctly, this threshold must be lager than
	/// any crosstalk signals or background noise.
	/// </summary>
	static float Ch1DetectionThreshold;
	static bool Ch1CableCrosstalkDetection;

	static bool Ch2AutoThresholdDetection;
	/// <summary>
	/// For signal detection to work correctly, this threshold must be lager than
	/// any crosstalk signals or background noise.
	/// </summary>
	static float Ch2DetectionThreshold;
	static bool Ch2CableCrosstalkDetection;

	static int AttemptsBeforeFail;
	static bool MeasureAverageLatency;
};