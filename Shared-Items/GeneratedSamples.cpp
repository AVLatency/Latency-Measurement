#include "GeneratedSamples.h"
#include <cmath>
// for M_PI:
#define _USE_MATH_DEFINES
#include <math.h>
#include "TestConfiguration.h"

GeneratedSamples::GeneratedSamples(WAVEFORMATEX* waveFormat, WaveType type)
    : WaveFormat(waveFormat), Type(type)
{
    switch (Type)
    {
    case GeneratedSamples::WaveType::VolumeAdjustment:
        GenerateVolumeAdjustmentSamples();
        break;
    case GeneratedSamples::WaveType::TestPattern_TonePlusHighFreq:
        GenerateTestPattern_TonePlusHighFreqSamples();
        break;
    case GeneratedSamples::WaveType::TestPattern_Tone:
        GenerateTestPattern_ToneSamples();
        break;
    case GeneratedSamples::WaveType::TestPattern_ToneHighFreqOnOff:
        GenerateTestPattern_ToneHighFreqOnOffSamples();
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
    return samplesLength / (double)WaveFormat->nSamplesPerSec;
}

int GeneratedSamples::GetTickFrequency(int sampleRate)
{
    return sampleRate == 44100 ? 11025 : 12000;
}

void GeneratedSamples::GenerateLatencyMeasurementSamples()
{
    int sampleRate = WaveFormat->nSamplesPerSec;

    // Timing
    double startPadding = 0.4;
    double tickTimes[] = { startPadding, startPadding + patternTick2RelTime, startPadding + patternTick3RelTime };
    int tickTimesInSamples[] = {
        round(tickTimes[0] * sampleRate),
        round(tickTimes[1] * sampleRate),
        round(tickTimes[2] * sampleRate) }; int tickTimesInSamplesLength = 3;
    double endPadding = 0.2f;

    // Frequencies
    int tickFreq = GetTickFrequency(sampleRate);

    // Amplitudes
    double constantToneAmp = 0.1;
    double tickAmp = 0.6;

    // Wave generation:
    int totalSamples = (int)(tickTimesInSamples[tickTimesInSamplesLength - 1] + endPadding * sampleRate);
    samples = new float[totalSamples];
    samplesLength = totalSamples;

    // Wake-up Constant tone generation
    for (int i = 0; i < totalSamples; i++)
    {
        // Wake up tone is the entire startPadding, minus two cycles of the wake up tone's frequency:
        if (i < (startPadding * sampleRate) - ((sampleRate / constantToneFreq) * 2))
        {
            double time = (double)i / sampleRate;
            samples[i] = (float)(sin(M_PI * 2 * constantToneFreq * time) * constantToneAmp);
        }
        else
        {
            // Fill in the rest with no sound
            samples[i] = 0;
        }
    }

    // Tick:
    int tickSamplesLength = sampleRate / tickFreq;
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
    int sampleRate = WaveFormat->nSamplesPerSec;

    // Timing
    double durationSeconds = 0.1f;

    // Frequencies
    int tickFreq = GetTickFrequency(sampleRate);

    // Amplitudes
    double tickAmp = 0.6;

    // Wave generation:
    int totalSamples = (int)(durationSeconds * sampleRate);
    samples = new float[totalSamples];
    samplesLength = totalSamples;

    // Tick followed by silence
    int tickSamplesLength = sampleRate / tickFreq;
    for (int i = 0; i < totalSamples; i++)
    {
        if (i < tickSamplesLength)
        {
            double time = (double)i / sampleRate;
            samples[i] = (float)sin(M_PI * 2 * tickFreq * time) * tickAmp;
        }
        else
        {
            samples[i] = 0;
        }
    }
}

void GeneratedSamples::GenerateTestPattern_ToneSamples()
{
    int sampleRate = WaveFormat->nSamplesPerSec;

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
    enum struct WaveProgression { Zero = 0, High, Middle, Low };

    GenerateTestPattern_ToneSamples();

    WaveProgression currentWavePoint = WaveProgression::Zero;
    for (int i = 0; i < samplesLength; i++)
    {
        float currentSample = 0;
        switch (currentWavePoint)
        {
        case WaveProgression::Zero:
            currentSample = 0;
            currentWavePoint = WaveProgression::High;
            break;
        case WaveProgression::High:
            currentSample = 1;
            currentWavePoint = WaveProgression::Middle;
            break;
        case WaveProgression::Middle:
            currentSample = 0;
            currentWavePoint = WaveProgression::Low;
            break;
        case WaveProgression::Low:
            currentSample = -1;
            currentWavePoint = WaveProgression::Zero;
            break;
        }

        samples[i] = samples[i] * 0.3 + currentSample * 0.5;
    }
}

void GeneratedSamples::GenerateTestPattern_ToneHighFreqOnOffSamples()
{
    enum struct WaveProgression { Zero = 0, High, Middle, Low };
    
    int sampleRate = WaveFormat->nSamplesPerSec;
    samplesLength = sampleRate; // 1 second
    samples = new float[samplesLength];

    WaveProgression currentWavePoint = WaveProgression::Zero;
    for (int i = 0; i < samplesLength; i++)
    {
        float currentSample = 0;

        // First half presents the high frequency tone, second half presents nothing.
        if (i < sampleRate / 2)
        {
            switch (currentWavePoint)
            {
            case WaveProgression::Zero:
                currentSample = 0;
                currentWavePoint = WaveProgression::High;
                break;
            case WaveProgression::High:
                currentSample = 1;
                currentWavePoint = WaveProgression::Middle;
                break;
            case WaveProgression::Middle:
                currentSample = 0;
                currentWavePoint = WaveProgression::Low;
                break;
            case WaveProgression::Low:
                currentSample = -1;
                currentWavePoint = WaveProgression::Zero;
                break;
            }
        }

        samples[i] = currentSample;
    }
}
