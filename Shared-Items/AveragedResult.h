#pragma once
#include <string>
#include <ctime>
#include "AudioFormat.h"
#include "AudioEndpoint.h"

struct AveragedResult
{
public:
	std::string GUID;
    time_t Time;
    const AudioFormat* Format;
    const AudioEndpoint& OutputEndpoint;
    /// <summary>
    /// After taking output offset profile into account
    /// </summary>
    float AverageLatency() const;
    float MinLatency() const;
    float MaxLatency() const;
    /// <summary>
    /// Before taking output offset profile into account
    /// </summary>
    float OutputOffsetFromProfile = 0; // TODO

    std::vector<float> Offsets;

    AveragedResult(std::string guid, time_t time, const AudioFormat* format, const AudioEndpoint& outputEndpoint)
        : GUID(guid), Time(time), Format(format), OutputEndpoint(outputEndpoint) {};
};

