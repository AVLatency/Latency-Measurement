#pragma once
#include <string>
#include <ctime>
#include "AudioFormat.h"
#include "AudioEndpoint.h"

struct AveragedResult
{
public:
    time_t Time;
    const AudioFormat* Format;
    const AudioEndpoint& OutputEndpoint;

    /// <summary>
    /// Before taking output offset profile into account
    /// </summary>
    std::vector<float> Offsets;

    std::string OutputOffsetProfileName;
    float OutputOffsetFromProfile;
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

    AveragedResult(time_t time, const AudioFormat* format, const AudioEndpoint& outputEndpoint, std::string outputOffsetProfileName, float outputOffsetFromProfile, bool verified, std::string referenceDacName, float referenceDacLatency)
        : Time(time), Format(format), OutputEndpoint(outputEndpoint), OutputOffsetProfileName(outputOffsetProfileName), OutputOffsetFromProfile(outputOffsetFromProfile), Verified(verified), ReferenceDacName(referenceDacName), ReferenceDacLatency(referenceDacLatency) {};
};

