#include "RecordingAnalyzer.h"
#include "RecordingSingleChannelResult.h"
#include "RecordingResult.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <format>
#include <filesystem>
#include <iomanip>
#include "StringHelper.h"
using namespace std;

const std::string RecordingAnalyzer::validResultsFilename{ "Valid-Results.csv" };
const std::string RecordingAnalyzer::invalidResultsFilename{ "Invalid-Results.csv" };

RecordingResult RecordingAnalyzer::AnalyzeRecording(const GeneratedSamples& config, const SystemInfo& sysInfo, const WasapiOutput& output, const WasapiInput& input, float detectionThresholdMultiplier)
{
    std::string testRootPath = GetTestRootPath(sysInfo);
    std::string recordingRootPath = testRootPath + format("{}ch-{}Hz-{}bit-{}-{}_{}-{}Hz-{}-{}-{}/",
        config.WaveFormat->nChannels,
        config.WaveFormat->nSamplesPerSec,
        config.WaveFormat->wBitsPerSample,
        GetAudioDataFormatString(config.WaveFormat),
        GetChannelMaskString(config.WaveFormat),
        sysInfo.VideoRes,
        sysInfo.VideoRefresh,
        sysInfo.VideoBitDepth,
        sysInfo.VideoFormat,
        sysInfo.VideoRange);

    filesystem::create_directories(filesystem::path(recordingRootPath));

    std::string guidStdString = StringHelper::GetGuidString();
    SaveRecording(input, recordingRootPath + guidStdString + ".wav");

    RecordingResult result;
    result.GUID = guidStdString;

    // Extract individual channels for analysis
    int inputSampleRate = input.waveFormat.Format.nSamplesPerSec;
    int channelSamplesLength = input.recordedAudioLength / input.recordedAudioNumChannels;
    float* ch1RecordedSamples = new float[channelSamplesLength];
    float* ch2RecordedSamples = new float[channelSamplesLength];
    int channelSampleIndex = 0;
    for (int i = 0; i < input.recordedAudioLength; i += input.recordedAudioNumChannels)
    {
        ch1RecordedSamples[channelSampleIndex] = input.recordedAudio[i];
        ch2RecordedSamples[channelSampleIndex] = input.recordedAudio[i + 1];
        channelSampleIndex++;
    }

    result.Channel1 = AnalyzeSingleChannel(config, ch1RecordedSamples, channelSamplesLength, inputSampleRate, detectionThresholdMultiplier);
    result.Channel2 = AnalyzeSingleChannel(config, ch2RecordedSamples, channelSamplesLength, inputSampleRate, detectionThresholdMultiplier);

    SaveResult(config, sysInfo, inputSampleRate, result, testRootPath, recordingRootPath, output.DeviceName, input.DeviceName);

    delete[] ch1RecordedSamples;
    delete[] ch2RecordedSamples;
    return result;
}

std::string RecordingAnalyzer::GetTestRootPath(const SystemInfo& sysInfo)
{
    return format("Results/{}/{}/", sysInfo.Extractor, sysInfo.DAC);
}

WORD RecordingAnalyzer::GetFormatID(WAVEFORMATEX* waveFormat)
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

std::string RecordingAnalyzer::GetAudioDataFormatString(WAVEFORMATEX* waveFormat)
{
    WORD formatID = GetFormatID(waveFormat);

    if (formatID == WAVE_FORMAT_IEEE_FLOAT)
    {
        return "IEEE_FLOAT";
    }
    else if (formatID == WAVE_FORMAT_PCM)
    {
        return "PCM";
    }
    else
    {
        return format("UnknownFormat0x{:X}", formatID);
    }
}

std::string RecordingAnalyzer::GetChannelMaskString(WAVEFORMATEX* waveFormat)
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
            result += format(".0x{:X}", extensibleFormat->dwChannelMask);
        }
    }
    return result;
}

RecordingSingleChannelResult RecordingAnalyzer::AnalyzeSingleChannel(const GeneratedSamples& config, float* recordedSamples, int recordedSamplesLength, int inputSampleRate, float detectionThresholdMultiplier)
{
    RecordingSingleChannelResult result;
    result.RecordingSampleRate = inputSampleRate;

    int tickDurationInSamples = inputSampleRate / GeneratedSamples::GetTickFrequency(config.WaveFormat->nSamplesPerSec);

    // Find the max and min amplitudes
    float maxAmplitude = 0;
    float minAmplitude = 0;
    for (int i = 0; i < recordedSamplesLength; i++)
    {
        if (recordedSamples[i] > maxAmplitude)
        {
            maxAmplitude = recordedSamples[i];
        }
        if (recordedSamples[i] < minAmplitude)
        {
            minAmplitude = recordedSamples[i];
        }
    }
    float halfMaxAmplitude = maxAmplitude / 2.0f;
    float halfMinAmplitude = minAmplitude / 2.0f;

    // create a list of times where the sound goes higher or lower than half of the max amplitude
    vector<int> possibleTickStarts;
    // double duration to account for error due to physical effects of sudden change in amplitude.
    int tickDurationPlusErrorDurationInSamples = tickDurationInSamples * 2;
    for (int i = 0; i < recordedSamplesLength; i++)
    {
        if (recordedSamples[i] > halfMaxAmplitude || recordedSamples[i] < halfMinAmplitude)
        {
            possibleTickStarts.push_back(i);
            // Skip over the rest of this possible tick.
            i += tickDurationPlusErrorDurationInSamples;
        }
    }

    vector<TickPosition> possibleTickPositions;
    for(int tickStart : possibleTickStarts)
    {
        int minIndex = tickStart;
        int maxIndex = tickStart;
        bool inverted = false;
        for (int tickIndex = tickStart + 1; tickIndex < tickStart + tickDurationPlusErrorDurationInSamples; tickIndex++)
        {
            if (recordedSamples[tickIndex] > recordedSamples[maxIndex])
            {
                maxIndex = tickIndex;
            }
            if (recordedSamples[tickIndex] < recordedSamples[minIndex])
            {
                minIndex = tickIndex;
            }
        }

        if (recordedSamples[minIndex] > halfMinAmplitude || recordedSamples[maxIndex] < halfMaxAmplitude)
        {
            // This tick isn't a high enough frequency to match the type of tick we're looking for.
        }
        else
        {
            // This tick seems to be a high frequency and is likely real. Let's keep track of its position.
            if (minIndex < maxIndex)
            {
                // this means tick is inverted, if this is a tick...
                inverted = true;
            }

            TickPosition pos;
            pos.index = inverted ? minIndex : maxIndex;
            pos.tickInverted = inverted;
            possibleTickPositions.push_back(pos);
        }
    }

    if (possibleTickPositions.size() < 3)
    {
        result.ValidResult = false;
        result.InvalidReason = "Cannot find 3 possible high frequency ticks";
        return result;
    }

    int invertedCount = count_if(possibleTickPositions.begin(), possibleTickPositions.end(), [](auto pos) { return pos.tickInverted; });
    int nonInvertedCount = possibleTickPositions.size() - invertedCount;
    if (invertedCount >= 3 && nonInvertedCount >= 3)
    {
        result.ValidResult = false;
        result.InvalidReason = "There are three or more posssible inverted ticks AND three ore more possible non-inverted ticks";
        return result;
    }

    if (invertedCount >= 3)
    {
        if (nonInvertedCount > 0)
        {
            possibleTickPositions.erase(remove_if(possibleTickPositions.begin(), possibleTickPositions.end(), [](auto pos) { return !pos.tickInverted; }));
        }
        result.PhaseInverted = true;
        // sort accending because these are the inverted ones:
        sort(possibleTickPositions.begin(), possibleTickPositions.end(), [&](auto a, auto b) { return recordedSamples[a.index] < recordedSamples[b.index]; });

    }
    else
    {
        if (invertedCount > 0)
        {
            possibleTickPositions.erase(remove_if(possibleTickPositions.begin(), possibleTickPositions.end(), [](auto pos) { return pos.tickInverted; }));
        }
        // sort decending because these are the non-inverted ones:
        sort(possibleTickPositions.begin(), possibleTickPositions.end(), [&](auto a, auto b) { return recordedSamples[a.index] > recordedSamples[b.index]; });
    }

    // take only the loudest 3 ticks:
    if (possibleTickPositions.size() > 3)
    {
        possibleTickPositions.erase(possibleTickPositions.begin() + 3, possibleTickPositions.end());
    }

    if (possibleTickPositions.size() < 3)
    {
        result.ValidResult = false;
        result.InvalidReason = format("Cannot find 3 possible high frequency ticks because {} is inverted and {} is not inverted.", invertedCount, nonInvertedCount);
        return result;
    }

    for (TickPosition position : possibleTickPositions)
    {
        float detectionThreshold = 0.05f * detectionThresholdMultiplier;
        if (!position.tickInverted && recordedSamples[position.index] < detectionThreshold
            || position.tickInverted && recordedSamples[position.index] > -1.0f * detectionThreshold)
        {
            result.ValidResult = false;
            result.InvalidReason = format("Possible tick at {} is too quiet", position.index);
            return result;
        }
    }

    // put them back in the order they were in the original recording:
    sort(possibleTickPositions.begin(), possibleTickPositions.end(), [](auto a, auto b) { return a.index < b.index; });

    result.SamplesToTick1 = possibleTickPositions[0].index;
    result.SamplesToTick2 = possibleTickPositions[1].index;
    result.SamplesToTick3 = possibleTickPositions[2].index;

    // Finally, check to see if the ticks we detected were where we expected them to be:
    int errorThresholdInSamples = round(6 * (inputSampleRate / 48000.0f)); // My tests have shown times where the third tick can be a full 5 samples off at 48kHz even though the recording is perfect.

    int samplesExpectedFrom1to2 = round(config.patternTick2RelTime * inputSampleRate);
    int actualSamplesFrom1to2 = result.SamplesToTick2 - result.SamplesToTick1;
    int samplesExpectedFrom1to3 = round(config.patternTick3RelTime * inputSampleRate);
    int actualSamplesFrom1to3 = result.SamplesToTick3 - result.SamplesToTick1;
    if (actualSamplesFrom1to2 > samplesExpectedFrom1to2 + errorThresholdInSamples
        || actualSamplesFrom1to2 < samplesExpectedFrom1to2 - errorThresholdInSamples)
    {
        result.ValidResult = false;
        result.InvalidReason = format("Expected tick 2 to be around {} samples after tick 1, but instead it was {}", samplesExpectedFrom1to2, actualSamplesFrom1to2);
    }
    else if (actualSamplesFrom1to3 > samplesExpectedFrom1to3 + errorThresholdInSamples
        || actualSamplesFrom1to3 < samplesExpectedFrom1to3 - errorThresholdInSamples)
    {
        result.ValidResult = false;
        result.InvalidReason = format("Expected tick 3 to be around {} samples after tick 1, but instead it was {}", samplesExpectedFrom1to3, actualSamplesFrom1to3);
    }
    else
    {
        // TODO: With these positions now known, check to make sure that the rest of the wave is smooth.
        // If it isn't, audio migtht be glitching.
        result.ValidResult = true;
    }

    return result;
}

namespace little_endian_io
{
    template <typename Word>
    std::ostream& write_word(std::ostream& outs, Word value, unsigned size = sizeof(Word))
    {
        for (; size; --size, value >>= 8)
            outs.put(static_cast <char> (value & 0xFF));
        return outs;
    }
}
using namespace little_endian_io;

void RecordingAnalyzer::SaveRecording(const WasapiInput& input, std::string guid)
{
    // http://www.topherlee.com/software/pcm-tut-wavformat.html
    // https://www.cplusplus.com/forum/beginner/166954/
    // https://gist.github.com/csukuangfj/c1d1d769606260d436f8674c30662450

    ofstream f(guid, ios::binary);

    unsigned short bitsPerSample = 16;

    // Write the file headers
    f << "RIFF----WAVEfmt ";     // (file size to be filled in later)
    write_word(f, 16, 4);  // size of the fmt chunk
    write_word(f, 1, 2);  // data waveFormat: PCM - integer samples
    write_word(f, input.recordedAudioNumChannels, 2);  // two channels (stereo file)
    write_word(f, input.waveFormat.Format.nSamplesPerSec, 4);  // samples per second (Hz)
    write_word(f, input.waveFormat.Format.nSamplesPerSec * input.recordedAudioNumChannels * (bitsPerSample / 8), 4);  // bytes per second
    write_word(f, (bitsPerSample / 8) * input.recordedAudioNumChannels, 2);  // data block size (size of two integer samples, one for each channel, in bytes)
    write_word(f, bitsPerSample, 2);  // number of bits per sample (use a multiple of 8)

    // Write the data chunk header
    size_t data_chunk_pos = f.tellp();
    f << "data----";  // (chunk size to be filled in later)

    // Write the audio samples
    for (int i = 0; i < input.recordedAudioLength; i++)
    {
        write_word(f, (INT16)(round(input.recordedAudio[i] * SHRT_MAX)), 2);
    }

    // (We'll need the final file size to fix the chunk sizes above)
    size_t file_length = f.tellp();

    // Fix the file header to contain the proper RIFF chunk file size, which is (file size - 8) bytes
    f.seekp(0 + 4); // first four are the "RIFF" characters
    write_word(f, file_length - 8, 4);

    // Fix the data chunk header to contain the data size
    f.seekp(data_chunk_pos + (unsigned long long)4); // first four are the "data" characters
    write_word(f, file_length - data_chunk_pos + 8, 4); // size of all the actual audio data in bytes. (Adding 8 to account for the "data----" characters)
}
void RecordingAnalyzer::SaveResult(const GeneratedSamples& config, const SystemInfo& sysInfo, int inputSampleRate, RecordingResult result, std::string testRootPath, std::string recordingRootPath, std::string outputDeviceName, std::string inputDeviceName)
{
    bool isValid = result.Channel1.ValidResult && result.Channel2.ValidResult;
    std::string detailedResultsCsvPath = recordingRootPath + (isValid ? validResultsFilename : invalidResultsFilename);

    bool writeHeader = false;
    if (!filesystem::exists(detailedResultsCsvPath))
    {
        writeHeader = true;
    }
    std::fstream detailedResultsStream { detailedResultsCsvPath, std::ios_base::app };
    if (writeHeader)
    {
        detailedResultsStream << "Recording ID,";
        detailedResultsStream << "Time,";
        detailedResultsStream << ",";
        detailedResultsStream << "Offset,";
        detailedResultsStream << ",";
        detailedResultsStream << "Display,";
        detailedResultsStream << "Computer Name,";
        detailedResultsStream << "Additional Notes,";
        detailedResultsStream << ",";
        detailedResultsStream << "Audio output device,";
        detailedResultsStream << "Audio output drivers,";
        detailedResultsStream << "video output drivers,";
        detailedResultsStream << "Windows version,";
        detailedResultsStream << ",";
        detailedResultsStream << "Audio input device,";
        detailedResultsStream << "Audio input sample rate,";
        detailedResultsStream << ",";
        detailedResultsStream << "Extractor,";
        detailedResultsStream << "DAC,";
        detailedResultsStream << ",";
        detailedResultsStream << "Channels,";
        detailedResultsStream << "SampleRate,";
        detailedResultsStream << "BitDepth,";
        detailedResultsStream << "AudioFormat,";
        detailedResultsStream << "ChannelMask,";
        detailedResultsStream << "VideoRes,";
        detailedResultsStream << "VideoRefresh,";
        detailedResultsStream << "VideoBitDepth,";
        detailedResultsStream << "VideoFormat,";
        detailedResultsStream << "VideoRange,";
        detailedResultsStream << ",";
        detailedResultsStream << "Ch 1 Milliseconds to Tick 1,";
        detailedResultsStream << "Ch 2 Milliseconds to Tick 1,";
        detailedResultsStream << "Ch 1 Rel Milliseconds to Tick 2,";
        detailedResultsStream << "Ch 2 Rel Milliseconds to Tick 2,";
        detailedResultsStream << "Ch 1 Rel Milliseconds to Tick 3,";
        detailedResultsStream << "Ch 2 Rel Milliseconds to Tick 3,";
        detailedResultsStream << "Ch 1 Phase Inverted,";
        detailedResultsStream << "Ch 1 Samples to Tick 1,";
        detailedResultsStream << "Ch 1 Samples to Tick 2,";
        detailedResultsStream << "Ch 1 Samples to Tick 3,";
        detailedResultsStream << "Ch 2 Phase Inverted,";
        detailedResultsStream << "Ch 2 Samples to Tick 1,";
        detailedResultsStream << "Ch 2 Samples to Tick 2,";
        detailedResultsStream << "Ch 2 Samples to Tick 3,";
        detailedResultsStream << "Ch 1 Invalid Reason,";
        detailedResultsStream << "Ch 2 Invalid Reason" << endl;
    }

    detailedResultsStream << "\"" << result.GUID << "\","; //"Recording ID,";
    detailedResultsStream << "\"" << std::put_time(std::localtime(&result.Time), "%c") << "\","; //"Time,";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << result.Offset() << "\","; //"Offset,";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << sysInfo.Display << "\","; //"Display,";
    detailedResultsStream << "\"" << sysInfo.ComputerName << "\","; //"Computer Name,";
    detailedResultsStream << "\"" << sysInfo.AdditionalNotes << "\","; //"Additional Notes,";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << outputDeviceName << "\","; //"Audio output device,";
    detailedResultsStream << "\"" << sysInfo.AudioOutputDrivers << "\","; //"Audio output drivers,";
    detailedResultsStream << "\"" << sysInfo.VideoOutputDrivers << "\","; //"video output drivers,";
    detailedResultsStream << "\"" << sysInfo.WindowsVersion << "\","; //"Windows version,";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << inputDeviceName << "\","; //"Audio input device,";
    detailedResultsStream << "\"" << inputSampleRate << "\","; //"Audio input sample rate,";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << sysInfo.Extractor << "\","; //"Extractor,";
    detailedResultsStream << "\"" << sysInfo.DAC << "\","; //"DAC,";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << config.WaveFormat->nChannels << "\","; //"Channels,";
    detailedResultsStream << "\"" << config.WaveFormat->nSamplesPerSec << "\","; //"SampleRate,";
    detailedResultsStream << "\"" << config.WaveFormat->wBitsPerSample << "\","; //"BitDepth,";
    detailedResultsStream << "\"" << GetAudioDataFormatString(config.WaveFormat) << "\","; //"AudioFormat,";
    detailedResultsStream << "\"" << GetChannelMaskString(config.WaveFormat) << "\","; //"ChannelMask,";
    detailedResultsStream << "\"" << sysInfo.VideoRes << "\","; //"VideoRes,";
    detailedResultsStream << "\"" << sysInfo.VideoRefresh << "\","; //"VideoRefresh,";
    detailedResultsStream << "\"" << sysInfo.VideoBitDepth << "\","; //"VideoBitDepth,";
    detailedResultsStream << "\"" << sysInfo.VideoFormat << "\","; //"VideoFormat,";
    detailedResultsStream << "\"" << sysInfo.VideoRange << "\","; //"VideoRange,";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << result.Channel1.MillisecondsToTick1() << "\","; //"Ch 1 Milliseconds to Tick 1,";
    detailedResultsStream << "\"" << result.Channel2.MillisecondsToTick1() << "\","; //"Ch 2 Milliseconds to Tick 1,";
    detailedResultsStream << "\"" << result.Channel1.RelMillisecondsToTick2() << "\","; //"Ch 1 Rel Milliseconds to Tick 2,";
    detailedResultsStream << "\"" << result.Channel2.RelMillisecondsToTick2() << "\","; //"Ch 2 Rel Milliseconds to Tick 2,";
    detailedResultsStream << "\"" << result.Channel1.RelMillisecondsToTick3() << "\","; //"Ch 1 Rel Milliseconds to Tick 3,";
    detailedResultsStream << "\"" << result.Channel2.RelMillisecondsToTick3() << "\","; //"Ch 2 Rel Milliseconds to Tick 3,";
    detailedResultsStream << "\"" << (result.Channel1.PhaseInverted ? "true" : "false") << "\","; //"Ch 1 Phase Inverted,";
    detailedResultsStream << "\"" << result.Channel1.SamplesToTick1 << "\","; //"Ch 1 Samples to Tick 1,";
    detailedResultsStream << "\"" << result.Channel1.SamplesToTick2 << "\","; //"Ch 1 Samples to Tick 2,";
    detailedResultsStream << "\"" << result.Channel1.SamplesToTick3 << "\","; //"Ch 1 Samples to Tick 3,";
    detailedResultsStream << "\"" << (result.Channel2.PhaseInverted ? "true" : "false") << "\","; //"Ch 2 Phase Inverted,";
    detailedResultsStream << "\"" << result.Channel2.SamplesToTick1 << "\","; //"Ch 2 Samples to Tick 1,";
    detailedResultsStream << "\"" << result.Channel2.SamplesToTick2 << "\","; //"Ch 2 Samples to Tick 2,";
    detailedResultsStream << "\"" << result.Channel2.SamplesToTick3 << "\","; //"Ch 2 Samples to Tick 3,";
    detailedResultsStream << "\"" << result.Channel1.InvalidReason << "\","; //"Ch 1 Invalid Reason,";
    detailedResultsStream << "\"" << result.Channel2.InvalidReason << "\"" << endl; //"Ch 2 Invalid Reason";

    detailedResultsStream.close();
}

void RecordingAnalyzer::UpdateSummary(const SystemInfo& sysInfo)
{
    std::string testRootPath = GetTestRootPath(sysInfo);
    filesystem::create_directories(filesystem::path(testRootPath));

    std::string summaryCsvPath = testRootPath + "Summary.csv";

    if (filesystem::exists(summaryCsvPath))
    {
        remove(summaryCsvPath.c_str());
    }

    std::fstream summaryStream{ summaryCsvPath, std::ios_base::app };
    summaryStream << "Format,";
    summaryStream << "Offset Variance,";
    summaryStream << "Total Valid,";
    summaryStream << "Total Invalid,";
    summaryStream << "Min Offset,";
    summaryStream << "Max Offset" << endl;
    summaryStream.close();

    summaryStream.open(summaryCsvPath, std::ios_base::app);

    for (const filesystem::directory_entry& entry : filesystem::directory_iterator(testRootPath))
    {
        if (!entry.is_directory())
        {
            continue;
        }

        struct Summary
        {
            std::string Format = "";
            float MinOffset = 999999;
            float MaxOffset = -999999;
            int TotalValid = 0;
            int TotalInvalid = 0;
            float OffsetVariance()
            {
                return MaxOffset - MinOffset;
            }
        };

        Summary summary;
        summary.Format = entry.path().filename().string();

        filesystem::path validResultsPath = entry.path();
        validResultsPath.append(validResultsFilename);
        if (filesystem::exists(validResultsPath))
        {
            summary.TotalValid = CountLinesInFile(validResultsPath.string()) - 1;
            GetMinMaxOffset(validResultsPath.string(), summary.MinOffset, summary.MaxOffset);
        }

        filesystem::path invalidResultsPath = entry.path();
        invalidResultsPath.append(invalidResultsFilename);
        if (filesystem::exists(invalidResultsPath))
        {
            summary.TotalInvalid = CountLinesInFile(invalidResultsPath.string()) - 1;
        }

        if (summary.TotalValid > 0)
        {
            summaryStream << "\"" << summary.Format << "\","; //"Format,";
            if (summary.TotalValid > 1)
            {
                summaryStream << "\"" << summary.OffsetVariance() << "\","; //"Offset Variance,";
            }
            else
            {
                summaryStream << "\"" << "" << "\","; //"Offset Variance,";
            }
            summaryStream << "\"" << summary.TotalValid << "\","; //"Total Valid,";
            summaryStream << "\"" << summary.TotalInvalid << "\","; //"Total Invalid,";
            summaryStream << "\"" << summary.MinOffset << "\","; //"Min Offset,";
            summaryStream << "\"" << summary.MaxOffset << "\"" << endl; //"Max Offset" << endl;
        }
    }
}

int RecordingAnalyzer::CountLinesInFile(std::string filePath)
{
    int result = 0;
    // Count valid
    ifstream ifs(filePath);
    if (ifs.is_open())
    {
        string line;
        while (getline(ifs, line))
        {
            result++;
        }
    }
    else
    {
        //TODO: error handling
    }
    return result;
}

void RecordingAnalyzer::GetMinMaxOffset(std::string filePath, float& min, float& max)
{
    // Count valid
    ifstream ifs(filePath);
    if (ifs.is_open())
    {
        string line;
        getline(ifs, line); // skip first line
        while (getline(ifs, line))
        {
            stringstream ss(line);
            string word;
            for (int i = 0; i < 4; i++) // The fourth word is the offset
            {
                getline(ss, word, ',');
            }
            if (word.length() > 0)
            {
                word.erase(remove(word.begin(), word.end(), '"'), word.end());
                try
                {
                    float offset = stof(word);
                    if (offset > max)
                    {
                        max = offset;
                    }
                    if (offset < min)
                    {
                        min = offset;
                    }
                }
                catch (...)
                {

                }
            }
        }
    }
    else
    {
        //TODO: error handling
    }
}
