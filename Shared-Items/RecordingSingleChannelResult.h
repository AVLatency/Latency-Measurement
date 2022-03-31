#pragma once
#include <string>

struct RecordingSingleChannelResult
{
public:
    bool ValidResult = false;
    std::string InvalidReason;

    int RecordingSampleRate = 0;

    int SamplesToTick1 = 0;
    float MillisecondsToTick1() const { return SamplesToTick1 / (float)RecordingSampleRate * 1000.0f; };
    int SamplesToTick2 = 0;
    float RelMillisecondsToTick2() const { return (SamplesToTick2 - SamplesToTick1) / (float)RecordingSampleRate * 1000.0f; };
    int SamplesToTick3 = 0;
    float RelMillisecondsToTick3() const { return (SamplesToTick3 - SamplesToTick1) / (float)RecordingSampleRate * 1000.0f; };

    float DetectionThreshold = 0;

    // While "SampelsToTick" above represent the peak, these are the start of the edge
    // that leads to the peak:
    int SamplesToIndex1 = 0;
    int SamplesToIndex2 = 0;
    int SamplesToIndex3 = 0;
};
