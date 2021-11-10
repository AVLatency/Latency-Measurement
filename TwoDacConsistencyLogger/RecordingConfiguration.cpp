#include "RecordingConfiguration.h"
#include <cmath>
// for M_PI:
#define _USE_MATH_DEFINES
#include <math.h>

RecordingConfiguration::RecordingConfiguration(WAVEFORMATEX* waveFormat)
{
	this->WaveFormat = waveFormat;
    CreateTestWave();
}

RecordingConfiguration::~RecordingConfiguration()
{
    delete[] testWave;
}

double RecordingConfiguration::TestWaveDurationInSeconds() const
{
    return testWaveLength / (double)WaveFormat->nSamplesPerSec;
}

int RecordingConfiguration::GetTickFrequency(int sampleRate)
{
    return sampleRate == 44100 ? 11025 : 12000;
}

void RecordingConfiguration::CreateTestWave()
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
    testWave = new float[totalSamples];
    testWaveLength = totalSamples;

    // Wake-up Constant tone generation
    for (int i = 0; i < totalSamples; i++)
    {
        // Wake up tone is the entire startPadding, minus two cycles of the wake up tone's frequency:
        if (i < (startPadding * sampleRate) - ((sampleRate / constantToneFreq) * 2))
        {
            double time = (double)i / sampleRate;
            testWave[i] = (float)(sin(M_PI * 2 * constantToneFreq * time) * constantToneAmp);
        }
        else
        {
            // Fill in the rest with no sound
            testWave[i] = 0;
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
            testWave[tickTimesInSamples[tickNum] + i] += (float)(tickSamples[i] * tickAmp);
        }
    }

    delete[] tickSamples;
}
