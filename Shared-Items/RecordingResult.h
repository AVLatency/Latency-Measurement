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
    const AudioFormat& Format;
    RecordingSingleChannelResult Channel1;
    RecordingSingleChannelResult Channel2;

    RecordingResult(const AudioFormat& format) : GUID(StringHelper::GetGuidString()), Time(time(0)), Format(format) {}

    void SetRecordingSampleRate(int value) { Channel1.RecordingSampleRate = value; Channel2.RecordingSampleRate = value; }
    float Offset() const { return Channel2.MillisecondsToTick1() - Channel1.MillisecondsToTick1(); }
};

