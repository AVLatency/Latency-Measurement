#pragma once
#include "RecordingSingleChannelResult.h"
#include <string>
#include <ctime>

struct RecordingResult
{
public:
    std::string GUID;
    std::time_t Time = time(nullptr);
    RecordingSingleChannelResult Channel1;
    RecordingSingleChannelResult Channel2;

    void SetRecordingSampleRate(int value) { Channel1.RecordingSampleRate = value; Channel2.RecordingSampleRate = value; }
    float Offset() { return Channel2.MillisecondsToTick1() - Channel1.MillisecondsToTick1(); }
};

