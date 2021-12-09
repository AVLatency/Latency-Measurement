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

    /// <summary>
    /// After taking output offset profile into account
    /// </summary>
    float AverageLatency() const;
    float MinLatency() const;
    float MaxLatency() const;

    AveragedResult(time_t time, const AudioFormat* format, const AudioEndpoint& outputEndpoint, std::string outputOffsetProfileName, float outputOffsetFromProfile, bool verified)
        : Time(time), Format(format), OutputEndpoint(outputEndpoint), OutputOffsetProfileName(outputOffsetProfileName), OutputOffsetFromProfile(outputOffsetFromProfile), Verified(verified) {};
};

