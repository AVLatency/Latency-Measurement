#pragma once
#include "RecordingSingleChannelResult.h"
#include <string>
#include <ctime>
#include "StringHelper.h"

struct RecordingResult
{
public:
    std::string GUID;
    time_t Time;
    AudioFormat* Format;
    RecordingSingleChannelResult Channel1;
    RecordingSingleChannelResult Channel2;

    std::string OutputOffsetProfileName;
    float OutputOffsetFromProfile;
    bool Verified;
    std::string ReferenceDacName;
    /// <summary>
    /// This will be subracted from the offset. This is used when a reference DAC is generating the
    /// analog audio signal. This is the case with the HDMI to Digital Audio tool.
    /// </summary>
    float ReferenceDacLatency;

    RecordingResult(AudioFormat* format, std::string outputOffsetProfileName, float outputOffsetFromProfile, bool verified, std::string referenceDacName, float referenceDacLatency)
        : GUID(StringHelper::GetGuidString()), Time(time(0)), Format(format), OutputOffsetProfileName(outputOffsetProfileName), OutputOffsetFromProfile(outputOffsetFromProfile), Verified(verified), ReferenceDacName(referenceDacName), ReferenceDacLatency(referenceDacLatency) {}

    void SetRecordingSampleRate(int value) { Channel1.RecordingSampleRate = value; Channel2.RecordingSampleRate = value; }
    float Offset() const { return Channel2.MillisecondsToTick1() - Channel1.MillisecondsToTick1(); }

    float AudioLatency() const { return Offset() - OutputOffsetFromProfile - ReferenceDacLatency; }
};

