#include "GeneratedSamples.h"
#include <cmath>
// for M_PI:
#define _USE_MATH_DEFINES
#include <math.h>
#include "TestConfiguration.h"

GeneratedSamples::GeneratedSamples(WAVEFORMATEX* waveFormat, WaveType type)
    : GeneratedSamples(waveFormat, Config(type) ) { }

    GeneratedSamples::GeneratedSamples(WAVEFORMATEX* waveFormat, Config config)
    : WaveFormat(waveFormat), Type(config.waveType)
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
    return samplesLength / (double)WaveFormat->nSamplesPerSec;
}

int GeneratedSamples::GetTickFrequency(int sampleRate)
{
    return sampleRate == 44100 ? (44100 / 4) : (48000 / 4);
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
    double wakeupToneAmp = 0.04;
    double constantToneAmp = 0.001;
    double tickAmp = 0.6;

    // Wave generation:
    samplesLength = (int)(tickTimesInSamples[tickTimesInSamplesLength - 1] + endPadding * sampleRate);
    samples = new float[samplesLength];

    // Wake-up Constant tone generation
    for (int i = 0; i < samplesLength; i++)
    {
        // Wake up tone is the entire startPadding, minus two cycles of the wake up tone's frequency:
        if (i < (startPadding * sampleRate) - ((sampleRate / constantToneFreq) * 2))
        {
            double time = (double)i / sampleRate;
            samples[i] = (float)(sin(M_PI * 2 * constantToneFreq * time) * wakeupToneAmp);
        }
        else
        {
            // The rest is very quiet, just to keep the audio device awake through the ticks
            // (HDV-MB01 goes to sleep after two ticks at 192 kHz)
            double time = (double)i / sampleRate;
            samples[i] = (float)(sin(M_PI * 2 * constantToneFreq * time) * constantToneAmp);
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
    double constantToneAmp = 0.001;

    // Wave generation:
    samplesLength = (int)(durationSeconds * sampleRate);
    samples = new float[samplesLength];

    // Constant tone is needed to wake up some audio devices, for example my Sony reciever
    for (int i = 0; i < samplesLength; i++)
    {
        double time = (double)i / sampleRate;
        samples[i] = (float)sin(M_PI * 2 * constantToneFreq * time) * constantToneAmp;
    }

    //// Tick once per durationInSeconds
    //int tickSamplesLength = sampleRate / tickFreq;
    //for (int i = 0; i < samplesLength; i++)
    //{
    //    if (i < tickSamplesLength)
    //    {
    //        double time = (double)i / sampleRate;
    //        samples[i] = (float)sin(M_PI * 2 * tickFreq * time) * tickAmp;
    //    }
    //}
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
    int sampleRate = WaveFormat->nSamplesPerSec;
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
    int sampleRate = WaveFormat->nSamplesPerSec;
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
    int sampleRate = WaveFormat->nSamplesPerSec;
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
        // The remaining audio segments always start and end on a high value, with the exception of the small two-sample ticks
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

        // Between  700 and 1500: add in a couple of two sample long ticks
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
