#pragma once
#include "RecordingSingleChannelResult.h"
#include <string>
#include <ctime>
#include "StringHelper.h"
#include "OutputOffsetProfile.h"

struct RecordingResult
{
public:
    std::string GUID;
    time_t Time;
    AudioFormat* Format; // Will be nullptr when current windows audio format is used by OffsetProfile
    RecordingSingleChannelResult Channel1;
    RecordingSingleChannelResult Channel2;

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

    RecordingResult(AudioFormat* format, OutputOffsetProfile* outputOffsetProfile, float outputOffsetFromProfile, bool verified, std::string referenceDacName, float referenceDacLatency)
        : GUID(StringHelper::GetGuidString()), Time(time(0)), Format(format), OffsetProfile(outputOffsetProfile), OutputOffsetFromProfile(outputOffsetFromProfile), Verified(verified), ReferenceDacName(referenceDacName), ReferenceDacLatency(referenceDacLatency) {}

    void SetRecordingSampleRate(int value) { Channel1.RecordingSampleRate = value; Channel2.RecordingSampleRate = value; }
    float Offset() const { return Channel2.MillisecondsToTick1() - Channel1.MillisecondsToTick1(); }

    float AudioLatency() const { return Offset() - OutputOffsetFromProfile - ReferenceDacLatency; }
};

