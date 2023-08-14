#include "AudioFormat.h"
#include <format>

AudioFormat::AudioFormat(WAVEFORMATEX* waveFormat) : WaveFormat(waveFormat)
{
    FormatString = GetFormatString(waveFormat, false, true);
}

std::string AudioFormat::GetCurrentWinAudioFormatString()
{
    return "Current Windows audio format";
}

std::string AudioFormat::GetFormatString(WAVEFORMATEX* waveFormat, bool includeEncoding, bool includeChannelInfo)
{
    std::string encodingStr = std::format("{} ", GetAudioDataFormatString(waveFormat));
    std::string channelInfoStr = std::format("-{}", GetChannelInfoString(waveFormat));
    return std::format("{}{}ch-{}kHz-{}bit{}",
        includeEncoding ? encodingStr : "",
        waveFormat->nChannels,
        (waveFormat->nSamplesPerSec == 44100 ? "44.1" : std::format("{}", waveFormat->nSamplesPerSec / 1000)),
        waveFormat->wBitsPerSample,
        includeChannelInfo ? channelInfoStr : "");
}

std::string AudioFormat::GetChannelInfoString(WAVEFORMATEX* waveFormat)
{
    std::string result;
    if (waveFormat->wFormatTag != WAVE_FORMAT_EXTENSIBLE)
    {
        result = "Default.Speakers";
    }
    else
    {
        WAVEFORMATEXTENSIBLE* extensibleFormat = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(waveFormat);
        if (extensibleFormat->dwChannelMask == 0)
        {
            result = "Empty.Channel.Mask";
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

WORD AudioFormat::GetFormatID(WAVEFORMATEX* waveFormat)
{
    if (waveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
        return EXTRACT_WAVEFORMATEX_ID(&(reinterpret_cast<WAVEFORMATEXTENSIBLE*>(waveFormat)->SubFormat));
    }
    else
    {
        return waveFormat->wFormatTag;
    }
}

std::string AudioFormat::GetAudioDataFormatString(WAVEFORMATEX* waveFormat)
{
    WORD formatID = GetFormatID(waveFormat);

    if (formatID == WAVE_FORMAT_IEEE_FLOAT)
    {
        return "IEEE_FLOAT";
    }
    else if (formatID == WAVE_FORMAT_PCM)
    {
        return "LPCM";
    }
    else
    {
        return std::format("UnknownFormat0x{:X}", formatID);
    }
}
