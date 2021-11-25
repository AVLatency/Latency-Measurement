#pragma once
#include <audioclient.h>

struct GeneratedSamples
{
public:
	enum struct WaveType { LatencyMeasurement, VolumeAdjustment };

	WAVEFORMATEX* WaveFormat;
	WaveType Type;

    // Test wave is constructed like this:
	// Quiet [constantToneFreq] Hz tone played at start
	// - Purpose is to wake up audio devices that might be sleeping during audio silence.
	// - This also gives an indication to the user that the test is in progress.
	// - Frequency doesn't matter. Just something that isn't horribly high pitched.
	// High frequency loud tick pattern used for latency measurement
	// - Pattern is spaced unevenly to prevent echo/other speakers from interfering with detection
	// - Constant tone at the start allows time for audio device to wake up
	//   ...More may be needed. I know some recievers take multiple seconds before playing back audio
	// - frequency of ticks are 12 kHz for sample rates that are multiples of 48 kHz
	// - frequency of tikcs is 11.025 kHz for 44.1 kHz sample rate
	// - These tick frequencies are chosen because most speakers should reproduce them well and
	//   they align perfectly to these sample rates.
	// - ticks should happen on times that are multiples of 0.01 seconds because 100 is a common
	//   factor of all common frequencies
	// Duration is (final tick time) + end padding
	// - This gives time for the audio to feedback into the recording input.
	// - Audio will immediately stop being recorded when playback has finished, which means some
	//   of the end of this test wave will be cut off.
	float* samples;
	int samplesLength;

	/// <summary>
	/// Time relative to tick 1 in the tick pattern
	/// </summary>
	double patternTick2RelTime = 0.03;
	/// <summary>
	/// Time relative to tick 1 in the tick pattern
	/// </summary>
	double patternTick3RelTime = 0.08;
	/// <summary>
	/// This constant tone is played for half of the wake-up beginning padding.
	/// </summary>
	double constantToneFreq = 300;

	GeneratedSamples(WAVEFORMATEX* waveFormat, WaveType type);
	~GeneratedSamples();

	double TestWaveDurationInSeconds() const;

	static int GetTickFrequency(int sampleRate);

private:
	void GenerateLatencyMeasurementSamples();
	void GenerateVolumeAdjustmentSamples();
};

