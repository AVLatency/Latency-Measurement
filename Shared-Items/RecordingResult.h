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

    RecordingResult(AudioFormat* format, std::string outputOffsetProfileName, float outputOffsetFromProfile, bool verified)
        : GUID(StringHelper::GetGuidString()), Time(time(0)), Format(format), OutputOffsetProfileName(outputOffsetProfileName), OutputOffsetFromProfile(outputOffsetFromProfile), Verified(verified) {}

    void SetRecordingSampleRate(int value) { Channel1.RecordingSampleRate = value; Channel2.RecordingSampleRate = value; }
    float Offset() const { return Channel2.MillisecondsToTick1() - Channel1.MillisecondsToTick1(); }

    float AudioLatency() const { return Offset() - OutputOffsetFromProfile; }
};

