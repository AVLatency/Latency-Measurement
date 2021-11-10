#pragma once
struct TestConfiguration
{
	// Test -> Pass -> Audio Formats -> Batch -> Recording
public:
	// These go slow because the format is switched between each recording.
	// These should be done first to make sure that no errors come during the final formats.
	int NumberOfSingleRecordingPasses = 100;

	// These go fast because the format is switched only once per format
	int NumberInBatchPass = 0; // Disabled because these numbers are always the same. Seems that swithcing format forces a re-sync, which is what I want to test.

	// Number of retries after failure, per recording
	int NumberOfRetries = 3;

	// Default value eliminates risk of measuring cable crosstalk. Set lower if using a mic with line in, etc.
	float DetectionThresholdMultiplier = 1.0f;
};

