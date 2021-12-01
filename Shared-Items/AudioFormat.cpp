#include "AudioFormat.h"
#include <format>

AudioFormat::AudioFormat(WAVEFORMATEX* waveFormat) : WaveFormat(waveFormat)
{
    FormatString = std::format("{}ch-{}kHz-{}bit-{}",
        waveFormat->nChannels,
        waveFormat->nSamplesPerSec,// == 44100 ? "44.1" : std::format("{}", waveFormat->nSamplesPerSec / 1000)),
        waveFormat->wBitsPerSample,
        GetChannelInfoString(waveFormat));
}

std::string AudioFormat::GetChannelInfoString(WAVEFORMATEX* waveFormat)
{
    std::string result;
    if (waveFormat->wFormatTag != WAVE_FORMAT_EXTENSIBLE)
    {
        result = "NoChInfo.WindowsLegacy";
    }
    else
    {
        WAVEFORMATEXTENSIBLE* extensibleFormat = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(waveFormat);
        if (extensibleFormat->dwChannelMask == 0)
        {
            result = "NoChInfo";
        }
        else
        {
            if (extensibleFormat->dwChannelMask & SPEAKER_FRONT_LEFT)
            {
                result += ".FL";
            }
            if (extensibleFormat->dwChannelMask & SPEAKER_FRONT_RIGHT)
            {
                result += ".FR";
            }
            if (extensibleFormat->dwChannelMask & SPEAKER_FRONT_CENTER)
            {
                result += ".FC";
            }
            if (extensibleFormat->dwChannelMask & SPEAKER_BACK_CENTER)
            {
                result += ".RC";
            }
            if (extensibleFormat->dwChannelMask & SPEAKER_SIDE_LEFT)
            {
                result += ".RL";
            }
            if (extensibleFormat->dwChannelMask & SPEAKER_SIDE_RIGHT)
            {
                result += ".RR";
            }
            if (extensibleFormat->dwChannelMask & SPEAKER_BACK_LEFT)
            {
                result += ".RLC";
            }
            if (extensibleFormat->dwChannelMask & SPEAKER_BACK_RIGHT)
            {
                result += ".RRC";
            }
            if (extensibleFormat->dwChannelMask & SPEAKER_FRONT_LEFT_OF_CENTER)
            {
                result += ".FLC";
            }
            if (extensibleFormat->dwChannelMask & SPEAKER_FRONT_RIGHT_OF_CENTER)
            {
                result += ".FRC";
            }
            if (extensibleFormat->dwChannelMask & SPEAKER_LOW_FREQUENCY)
            {
                result += ".LFE";
            }
            result = result.substr(1, result.size() - 1); // remove initial '.'
        }
    }
    return result;
}
