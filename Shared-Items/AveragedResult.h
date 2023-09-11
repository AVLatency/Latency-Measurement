#pragma once
#include <string>
#include <ctime>
#include "AudioFormat.h"
#include "AudioEndpoint.h"
#include "OutputOffsetProfile.h"

struct AveragedResult
{
public:
    time_t Time;
    const SupportedAudioFormat* Format;
    AudioEndpoint* OutputEndpoint;

    /// <summary>
    /// Before taking output offset profile into account
    /// </summary>
    std::vector<float> Offsets;

    OutputOffsetProfile* OffsetProfile;
    /// <summary>
    /// The actual offset value used during the test that generated this RecordingResult
    /// </summary>
    float OutputOffsetFromProfile;
    /// <summary>
    /// If the actual offset value used during the test was verified
    /// </summary>
    bool Verified;
    std::string ReferenceDacName;
    /// <summary>
    /// This will be subracted from the offset. This is used when a reference DAC is generating the
    /// analog audio signal. This is the case with the HDMI to Digital Audio tool.
    /// </summary>
    float ReferenceDacLatency;

    /// <summary>
    /// After taking output offset profile into account
    /// </summary>
    float AverageLatency() const;
    float MinLatency() const;
    float MaxLatency() const;

    AveragedResult(time_t time, const SupportedAudioFormat* format, AudioEndpoint* outputEndpoint, OutputOffsetProfile* outputOffsetProfile, float outputOffsetFromProfile, bool verified, std::string referenceDacName, float referenceDacLatency)
        : Time(time), Format(format), OutputEndpoint(outputEndpoint), OffsetProfile(outputOffsetProfile), OutputOffsetFromProfile(outputOffsetFromProfile), Verified(verified), ReferenceDacName(referenceDacName), ReferenceDacLatency(referenceDacLatency) {};
};

