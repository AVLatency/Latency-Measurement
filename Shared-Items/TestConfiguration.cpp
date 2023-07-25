#include "TestConfiguration.h"

#ifdef _DEBUG
#define IS_DEBUG true
#else
#define IS_DEBUG false
#endif // _DEBUG


bool TestConfiguration::SaveIndividualRecordingResults = IS_DEBUG;
bool TestConfiguration::SaveIndividualWavFiles = IS_DEBUG;
int TestConfiguration::NumMeasurements = 10;
float TestConfiguration::AdditionalRecordingTime = 0.42;
int TestConfiguration::InitialIgnoreLength = 10;
float TestConfiguration::OutputVolume = 0.75f;
float TestConfiguration::LeadInDuration = 0.4f;
// This LeadInToneAmplitude value corrects an issue found with a specific mic input.
// This is not based on extensive testing and could possibly be reduced(?)
float TestConfiguration::LeadInToneAmplitude = 0.07;
// If this is ever set to false, the auto threshold multiplier should be changed, likely to 0.5
bool TestConfiguration::LowFreqPitch = true;

// Auto threshold is chosen to detect the largest edges, and no others.
// Largest edge may have been sampled here with the ideal wave alignment (Nyquist stuff)
// but later during the test it may have been sampled with the worst case wave
// alignment. In the later, it would only be 75% of the magnitude of this initial
// sampling. Because we're looking for the first edge, it is expected to be 0.5 of the largest
// magnitude. This would mean a threshold of 0.375 (75% of 0.5). But a bit of wiggle room
// is given because this first peak is probably not as high as the second peak, so I've gone
// with a 0.35 auto threshold.
// This does NOT do a good job of accounting for echos. It's possible that echos might
// be causing this largest edge to be substantially larger than the initial edges in the
// tick cluster that preceed it. But for this case, the user can switch to manual edge
// detection mode to fine-tune the tool for use in this scenario. Regardless, the 0.35 will
// still account for some of these echoes.
float TestConfiguration::AutoThresholdMultiplier = 0.35;

bool TestConfiguration::Ch1AutoThresholdDetection = true;
float TestConfiguration::Ch1DetectionThreshold = 0.05f * 2.0f;
bool TestConfiguration::Ch1CableCrosstalkDetection = true;

bool TestConfiguration::Ch2AutoThresholdDetection = true;
float TestConfiguration::Ch2DetectionThreshold = 0.05f * 2.0f;
bool TestConfiguration::Ch2CableCrosstalkDetection = true;

int TestConfiguration::AttemptsBeforeFail = 6;
bool TestConfiguration::InsertFormatSwitch = true;
