#pragma once
#include <string>

struct RecordingSingleChannelResult
{
public:
    bool ValidResult = false;
    std::string InvalidReason;

    int RecordingSampleRate = 0;

    int SamplesToTick1 = 0;
    float MillisecondsToTick1() { return SamplesToTick1 / (float)RecordingSampleRate * 1000.0f; };
    int SamplesToTick2 = 0;
    float RelMillisecondsToTick2() { return (SamplesToTick2 - SamplesToTick1) / (float)RecordingSampleRate * 1000.0f; };
    int SamplesToTick3 = 0;
    float RelMillisecondsToTick3() { return (SamplesToTick3 - SamplesToTick1) / (float)RecordingSampleRate * 1000.0f; };

    bool PhaseInverted = false;
};
