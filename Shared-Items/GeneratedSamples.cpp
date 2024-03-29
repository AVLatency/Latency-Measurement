#include "GeneratedSamples.h"
#include <cmath>
// for M_PI:
#define _USE_MATH_DEFINES
#include <math.h>
#include "TestConfiguration.h"

#define CONSTANT_TONE_AMPLITUDE 0.002
#define TICK_AMPLITUDE (1.0 - CONSTANT_TONE_AMPLITUDE)

GeneratedSamples::GeneratedSamples(int samplesPerSecond, WaveType type)
    : GeneratedSamples(samplesPerSecond, Config(type)) { }

    GeneratedSamples::GeneratedSamples(int samplesPerSecond, Config config)
    : SamplesPerSecond(samplesPerSecond), Type(config.waveType)
{
    switch (Type)
    {
    case GeneratedSamples::WaveType::VolumeAdjustment:
        GenerateVolumeAdjustmentSamples();
        break;
    case GeneratedSamples::WaveType::FormatSwitch:
        GenerateFormatSwitchSamples();
        break;
    case GeneratedSamples::WaveType::TestPattern_TonePlusHighFreq:
        GenerateTestPattern_TonePlusHighFreqSamples();
        break;
    case GeneratedSamples::WaveType::TestPattern_Tone:
        GenerateTestPattern_ToneSamples();
        break;
    case GeneratedSamples::WaveType::TestPattern_ToneHighFreqBlip:
        GenerateTestPattern_ToneHighFreqBlip(config.blipSampleLength, config.blipFrequency); // NTSC trigger: (2, 60/1.001);
        break;
    case GeneratedSamples::WaveType::TestPattern_ToneHighFreqOnOff:
        GenerateTestPattern_ToneHighFreqOnOffSamples(config.onOffFrequency);
        break;
    case GeneratedSamples::WaveType::TestPattern_VisuallyIdentifiable:
        GenerateTestPattern_VisuallyIdentifiableSamples();
        break;
    case GeneratedSamples::WaveType::LatencyMeasurement:
    default:
        GenerateLatencyMeasurementSamples();
        break;
    }
}

GeneratedSamples::~GeneratedSamples()
{
    delete[] samples;
}

double GeneratedSamples::TestWaveDurationInSeconds() const
{
    return samplesLength / (double)SamplesPerSecond;
}

double GeneratedSamples::GetTickFrequency(int sampleRate)
{
    int divider = TestConfiguration::LowFreqPitch ? 8 : 4;
    return sampleRate == 44100 ? ((double)44100 / divider) : (48000 / divider);
}

void GeneratedSamples::GenerateLatencyMeasurementSamples()
{
    int sampleRate = SamplesPerSecond;

    // Timing
    double startPadding = TestConfiguration::LeadInDuration;
    double tickTimes[] = { startPadding, startPadding + patternTick2RelTime, startPadding + patternTick3RelTime };
    int tickTimesInSamples[] = {
        round(tickTimes[0] * sampleRate),
        round(tickTimes[1] * sampleRate),
        round(tickTimes[2] * sampleRate) }; int tickTimesInSamplesLength = 3;
    double endPadding = TestConfiguration::AdditionalRecordingTime;

    // Frequencies
    double tickFreq = GetTickFrequency(sampleRate);

    // Amplitudes
    double constantToneAmp = CONSTANT_TONE_AMPLITUDE;
    double tickAmp = TICK_AMPLITUDE;

    // Wave generation:
    samplesLength = (int)(tickTimesInSamples[tickTimesInSamplesLength - 1] + endPadding * sampleRate);
    samples = new float[samplesLength];

    // lead-in and constant tone generation
    for (int i = 0; i < samplesLength; i++)
    {
        float amplitude;
        if (i < (startPadding * sampleRate) - ((sampleRate / constantToneFreq) * 4))
        {
            // Lead-in tone is the entire startPadding, minus four cycles of the tone's frequency.
            // The purpose of this tone is to fight against dynamic normalization that occurs in
            // some input audio devices, such as onboard microphones. Professional audio devices
            // and many onboard microphone inputs don't need this at all.
            amplitude = max(TestConfiguration::LeadInToneAmplitude, constantToneAmp);
        }
        else
        {
            // A constant tone is played throughout, just to keep the audio device awake
            // (HDV-MB01 goes to sleep after two ticks at 192 kHz)
            amplitude = constantToneAmp;
        }

        double time = (double)i / sampleRate;
        samples[i] = (float)(sin(M_PI * 2 * constantToneFreq * time) * amplitude);
    }

    // Tick:
    int tickSamplesLength = round(sampleRate / tickFreq);
    float* tickSamples = new float[tickSamplesLength];
    for (int i = 0; i < tickSamplesLength; i++)
    {
        double time = (double)i / sampleRate;
        tickSamples[i] = (float)sin(M_PI * 2 * tickFreq * time);
    }

    // Add tick on top of existing wave
    for (int tickNum = 0; tickNum < tickTimesInSamplesLength; tickNum++)
    {
        for (int i = 0; i < tickSamplesLength; i++)
        {
            samples[tickTimesInSamples[tickNum] + i] += (float)(tickSamples[i] * tickAmp);
        }
    }

    delete[] tickSamples;
}

void GeneratedSamples::GenerateVolumeAdjustmentSamples()
{
    int sampleRate = SamplesPerSecond;

    // Timing
    double durationSeconds = 0.1f;

    // Frequencies
    double tickFreq = GetTickFrequency(sampleRate);

    // Amplitudes
    double tickAmp = TICK_AMPLITUDE;
    double constantToneAmp = CONSTANT_TONE_AMPLITUDE;

    // Wave generation:
    samplesLength = (int)(durationSeconds * sampleRate);
    samples = new float[samplesLength];

    // lead-in and constant tone generation
    for (int i = 0; i < samplesLength; i++)
    {
        float amplitude;
        if (i < samplesLength / 2)
        {
            // Lead-in tone is the entire startPadding, minus four cycles of the tone's frequency.
            // The purpose of this tone is to fight against dynamic normalization that occurs in
            // some input audio devices, such as onboard microphones. Professional audio devices
            // and many onboard microphone inputs don't need this at all.
            amplitude = max(TestConfiguration::LeadInToneAmplitude, constantToneAmp);
        }
        else
        {
            // A constant tone is played throughout, just to keep the audio device awake
            // (HDV-MB01 goes to sleep after two ticks at 192 kHz)
            amplitude = constantToneAmp;
        }

        double time = (double)i / sampleRate;
        samples[i] = (float)(sin(M_PI * 2 * constantToneFreq * time) * amplitude);
    }

    // Tick once per durationInSeconds on top of existing wave
    int tickSamplesLength = round(sampleRate / tickFreq);
    for (int i = 0; i < tickSamplesLength && i < samplesLength; i++)
    {
        double time = (double)i / sampleRate;
        // Tick happsn 4 constant tone cyles after the lead-in tone ends, just like in the measurement pattern
        samples[i + (samplesLength / 2) + ((sampleRate / constantToneFreq) * 4)] += (float)sin(M_PI * 2 * tickFreq * time) * tickAmp;
    }
}

void GeneratedSamples::GenerateFormatSwitchSamples()
{
    int sampleRate = SamplesPerSecond;

    // Timing
    double durationSeconds = 1.0f;

    // Wave generation:
    samplesLength = (int)(durationSeconds * sampleRate);
    samples = new float[samplesLength];

    // Play the lead-in tone for the full duration
    for (int i = 0; i < samplesLength; i++)
    {
        float amplitude = max(TestConfiguration::LeadInToneAmplitude, CONSTANT_TONE_AMPLITUDE);

        double time = (double)i / sampleRate;
        samples[i] = (float)(sin(M_PI * 2 * constantToneFreq * time) * amplitude);
    }
}

void GeneratedSamples::GenerateTestPattern_ToneSamples()
{
    int sampleRate = SamplesPerSecond;

    samplesLength = sampleRate; // 1 second
    samples = new float[samplesLength];

    for (int i = 0; i < samplesLength; i++)
    {
        double time = (double)i / sampleRate;
        samples[i] = (float)sin(M_PI * 2 * constantToneFreq * time);
    }
}

void GeneratedSamples::GenerateTestPattern_TonePlusHighFreqSamples()
{
    GenerateTestPattern_ToneSamples();

    bool high = true;
    for (int i = 0; i < samplesLength; i++)
    {
        float currentSample = high ? 1 : -1;
        high = !high;

        samples[i] = samples[i] * 0.3 + currentSample * 0.7;
    }
}

void GeneratedSamples::GenerateTestPattern_ToneHighFreqOnOffSamples(double frequency)
{
    int sampleRate = SamplesPerSecond;
    samplesLength = sampleRate / frequency; // high frequency on/off
    if (samplesLength < 2)
    {
        samplesLength = 2;
    }
    samples = new float[samplesLength];

    bool high = true;
    for (int i = 0; i < samplesLength; i++)
    {
        float currentSample = 0;

        // First half presents the high frequency tone, second half presents nothing.
        if (i < samplesLength / 2)
        {
            currentSample = high ? 1 : -1;
            high = !high;
        }

        samples[i] = currentSample;
    }
}

void GeneratedSamples::GenerateTestPattern_ToneHighFreqBlip(int blipSampleLength, double frequency)
{
    int sampleRate = SamplesPerSecond;
    samplesLength = round(sampleRate / frequency); // high frequency on/off
    if (samplesLength < 2)
    {
        samplesLength = 2;
    }
    samples = new float[samplesLength];

    bool high = true;
    for (int i = 0; i < samplesLength; i++)
    {
        float currentSample = 0;

        if (i < blipSampleLength)
        {
            currentSample = high ? 1 : -1;
            high = !high;
        }

        samples[i] = currentSample;
    }
}

void GeneratedSamples::GenerateTestPattern_VisuallyIdentifiableSamples()
{
    int sampleRate = SamplesPerSecond;
    samplesLength = sampleRate / 10; // 0.1 second pattern: enough to easily visualize 100 ms latency
    if (samplesLength < 2)
    {
        samplesLength = 2;
    }
    samples = new float[samplesLength];

    bool high = true;
    for (int i = 0; i < samplesLength; i++)
    {
        // start by bouncing between high and low
        float currentSample = 0;
        currentSample = high ? 1 : -1;
        high = !high;

        // remove some samples to make an interesting pattern
        // The remaining audio segments always start and end on a high value, with the exception of the small two or four sample ticks
        if (i > 100 && i < 200)
        {
            currentSample = 0;
        }
        if (i > 300 && i < 500)
        {
            currentSample = 0;
        }
        if (i > 700 && i < 1500)
        {
            currentSample = 0;
        }

        // Between  700 and 1500: add in a couple of two and four sample long ticks
        if (i == 1000)
        {
            currentSample = 1;
        }
        if (i == 1001)
        {
            currentSample = -1;
        }
        if (i == 1200)
        {
            currentSample = 1;
        }
        if (i == 1201)
        {
            currentSample = -1;
        }
        if (i == 1202)
        {
            currentSample = 1;
        }
        if (i == 1203)
        {
            currentSample = -1;
        }

        if (i > 1700 && i < 1706)
        {
            currentSample = 0;
        }
        if (i > 1710 && i < 1716)
        {
            currentSample = 0;
        }
        if (i > 1720 && i < 1726)
        {
            currentSample = 0;
        }
        if (i > 1730 && i < 1736)
        {
            currentSample = 0;
        }
        if (i > 2000 && i < 3000)
        {
            currentSample = 0;
        }
        if (i > 4000 && i < 5500)
        {
            currentSample = 0;
        }
        if (i > 7000 && i < 9000)
        {
            currentSample = 0;
        }
        if (i > 12000 && i < 14500)
        {
            currentSample = 0;
        }
        if (i > 17500 && i <19000)
        {
            currentSample = 0;
        }

        samples[i] = currentSample;
    }
}

