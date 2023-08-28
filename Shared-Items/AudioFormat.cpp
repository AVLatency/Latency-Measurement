#include "AudioFormat.h"
#include <format>

AudioFormat::AudioFormat(WAVEFORMATEX* waveFormat) : WaveFormat(waveFormat)
{
    FormatString = GetFormatString(waveFormat, true, true);
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

std::string AudioFormat::GetCurrentWinAudioFormatString()
{
    return "Current Windows audio format";
}

std::string AudioFormat::GetFormatString(WAVEFORMATEX* waveFormat, bool includeEncoding, bool includeChannelInfo)
{
    std::string encodingStr = std::format("{} ", GetAudioDataEncodingString(waveFormat));
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
    if (waveFormat == nullptr || waveFormat->wFormatTag != WAVE_FORMAT_EXTENSIBLE)
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

std::string AudioFormat::GetAudioDataEncodingString(WAVEFORMATEX* waveFormat)
{
    if (waveFormat == nullptr)
    {
        return "UnknownEncoding";
    }

    if (waveFormat->wFormatTag != WAVE_FORMAT_EXTENSIBLE)
    {
        WORD formatID = waveFormat->wFormatTag;

        if (formatID == WAVE_FORMAT_PCM)
        {
            return "LPCM";
        }
        else if (formatID == WAVE_FORMAT_ADPCM)
        {
            return "ADPCM";
        }
        else if (formatID == WAVE_FORMAT_IEEE_FLOAT)
        {
            return "IEEE_FLOAT";
        }
        else if (formatID == WAVE_FORMAT_ALAW)
        {
            return "ALAW";
        }
        else if (formatID == WAVE_FORMAT_MULAW)
        {
            return "MULAW";
        }
        else if (formatID == WAVE_FORMAT_DOLBY_AC2)
        {
            return "DOLBY_AC2";
        }
        else if (formatID == WAVE_FORMAT_DOLBY_AC3_SPDIF)
        {
            return "DOLBY_AC3_SPDIF";
        }
        else if (formatID == WAVE_FORMAT_DOLBY_AC4)
        {
            return "DOLBY_AC4";
        }
        else if (formatID == WAVE_FORMAT_DTS)
        {
            return "DTS";
        }
        else if (formatID == WAVE_FORMAT_DTS_DS)
        {
            return "DTS_DS";
        }
        else if (formatID == WAVE_FORMAT_DTS2)
        {
            return "DTS2";
        }
        else
        {
            return std::format("UnknownFormat0x{:X}", formatID);
        }
    }
    else
    {
        auto waveFormatExtensible = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(waveFormat);
        auto subFormat = waveFormatExtensible->SubFormat;
        if (subFormat == KSDATAFORMAT_SUBTYPE_PCM)
        {
            return "LPCM";
        }
        else if (subFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
        {
            return "IEEE_FLOAT";
        }
        else if (subFormat == KSDATAFORMAT_SUBTYPE_ALAW)
        {
            return "ALAW";
        }
        else if (subFormat == KSDATAFORMAT_SUBTYPE_MULAW)
        {
            return "MULAW";
        }
        else if (subFormat == KSDATAFORMAT_SUBTYPE_ADPCM)
        {
            return "ADPCM";
        }
        else if (subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL)
        {
            return "IEC61937_DOLBY_DIGITAL_AC3";
        }
        else if (subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL_PLUS)
        {
            return "IEC61937_DOLBY_DIGITAL_PLUS";
        }
        else if (subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL_PLUS_ATMOS)
        {
            return "IEC61937_DOLBY_DIGITAL_PLUS_ATMOS";
        }
        else if (subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_MLP)
        {
            return "IEC61937_DOLBY_MLP_MAT10";
        }
        else if (subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_MAT20)
        {
            return "IEC61937_DOLBY_MAT20";
        }
        else if (subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_MAT21)
        {
            return "IEC61937_DOLBY_MAT21";
        }
        else if (subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DTS)
        {
            return "IEC61937_DTS";
        }
        else if (subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DTS_HD)
        {
            return "IEC61937_DTS_HD";
        }
        else if (subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DTSX_E1)
        {
            return "IEC61937_DTSX_E1";
        }
        else if (subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DTSX_E2)
        {
            return "IEC61937_DTSX_E2";
        }
        else if (subFormat == KSDATAFORMAT_SUBTYPE_DTS_AUDIO)
        {
            return "DTS_AUDIO";
        }
        else
        {
            return std::format("UnknownExtensibleFormat0x{:X}0x{:X}0x{:X}", subFormat.Data1, subFormat.Data2, subFormat.Data3);
        }
    }
}
