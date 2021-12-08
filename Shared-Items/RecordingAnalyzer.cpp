#include "RecordingAnalyzer.h"
#include "RecordingSingleChannelResult.h"
#include "RecordingResult.h"
#include <fstream>
#include <iostream>
#include <atlconv.h>
#include <vector>
#include <algorithm>
#include <format>
#include <filesystem>
#include <iomanip>
#include "TestConfiguration.h"
#include <ctime>
using namespace std;

const std::string RecordingAnalyzer::validRecordingsFilename{ "Valid-Individual-Recordings.csv" };
const std::string RecordingAnalyzer::invalidRecordingsFilename{ "Invalid-Individual-Recordings.csv" };

RecordingResult RecordingAnalyzer::AnalyzeRecording(const GeneratedSamples& generatedSamples, const WasapiInput& input, const AudioFormat& format)
{
    RecordingResult result(format);

    // Extract individual channels for analysis
    int inputSampleRate = input.waveFormat.Format.nSamplesPerSec;
    int channelSamplesLength = input.recordingBufferLength / input.recordedAudioNumChannels;
    float* ch1RecordedSamples = new float[channelSamplesLength];
    float* ch2RecordedSamples = new float[channelSamplesLength];
    int channelSampleIndex = 0;
    for (int i = 0; i < input.recordingBufferLength; i += input.recordedAudioNumChannels)
    {
        ch1RecordedSamples[channelSampleIndex] = input.recordingBuffer1[i];
        ch2RecordedSamples[channelSampleIndex] = input.recordingBuffer1[i + 1];
        channelSampleIndex++;
    }

    result.Channel1 = AnalyzeSingleChannel(generatedSamples, ch1RecordedSamples, channelSamplesLength, inputSampleRate);
    result.Channel2 = AnalyzeSingleChannel(generatedSamples, ch2RecordedSamples, channelSamplesLength, inputSampleRate);

    delete[] ch1RecordedSamples;
    delete[] ch2RecordedSamples;
    return result;
}

RecordingSingleChannelResult RecordingAnalyzer::AnalyzeSingleChannel(const GeneratedSamples& config, float* recordedSamples, int recordedSamplesLength, int inputSampleRate)
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
        possibleTickPositions.erase(possibleTickPositions.begin() + 3, possibleTickPositions.begin() + possibleTickPositions.size() - 1); // TODO: make sure I wrote this right.
    }

    if (possibleTickPositions.size() < 3)
    {
        result.ValidResult = false;
        result.InvalidReason = format("Cannot find 3 possible high frequency ticks because {} is inverted and {} is not inverted.", invertedCount, nonInvertedCount);
        return result;
    }

    for (TickPosition position : possibleTickPositions)
    {
        float detectionThreshold = 0.05f * TestConfiguration::DetectionThreshold();
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
        // TODO: (Maybe this could be improved?) With these positions now known, check to make sure that the rest of the wave is smooth.
        // If it isn't, audio migtht be glitching.
        result.ValidResult = true;
    }

    return result;
}

std::map<const AudioFormat*, AveragedResult> RecordingAnalyzer::AnalyzeResults(std::vector<RecordingResult> results, std::string testGuid, time_t tTime, const AudioEndpoint& outputEndpoint)
{
    std::map<const AudioFormat*, AveragedResult> averagedResults;

    for (const RecordingResult& recordingResult : results)
    {
        if (recordingResult.Channel1.ValidResult && recordingResult.Channel2.ValidResult)
        {
            if (!averagedResults.contains(&recordingResult.Format))
            {
                averagedResults.insert({ &recordingResult.Format, AveragedResult(testGuid, tTime, &recordingResult.Format, outputEndpoint) });
            }
            const auto& pair = averagedResults.find(&recordingResult.Format);
            pair->second.Offsets.push_back(recordingResult.Offset());
        }
    }

    return averagedResults;
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

void RecordingAnalyzer::SaveRecording(const WasapiInput& input, std::string path)
{
    filesystem::create_directories(filesystem::path(path));

    // http://www.topherlee.com/software/pcm-tut-wavformat.html
    // https://www.cplusplus.com/forum/beginner/166954/
    // https://gist.github.com/csukuangfj/c1d1d769606260d436f8674c30662450

    ofstream f(path, ios::binary);

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
    for (int i = 0; i < input.recordingBufferLength; i++)
    {
        write_word(f, (INT16)(round(input.recordingBuffer1[i] * SHRT_MAX)), 2);
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

void RecordingAnalyzer::SaveIndividualResult(IResultsWriter& writer, const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, RecordingResult& result, std::string testRootPath)
{
    filesystem::create_directories(filesystem::path(testRootPath));

    bool isValid = result.Channel1.ValidResult && result.Channel2.ValidResult;
    std::string detailedResultsCsvPath = format("{}/{}", testRootPath, (isValid ? validRecordingsFilename : invalidRecordingsFilename));

    bool writeHeader = false;
    if (!filesystem::exists(detailedResultsCsvPath))
    {
        writeHeader = true;
    }
    std::fstream detailedResultsStream{ detailedResultsCsvPath, std::ios_base::app };

    writer.WriteIndividualRecordingResults(writeHeader, detailedResultsStream, outputEndpoint, inputEndpoint, result);

    detailedResultsStream.close();
}

void RecordingAnalyzer::SaveFinalResults(IResultsWriter& writer, std::map<const AudioFormat*, AveragedResult> results, std::string testRootPath, std::string csvFilename)
{
    filesystem::create_directories(filesystem::path(testRootPath));

    std::string csvPath = format("{}/{}", testRootPath, csvFilename);

    bool writeHeader = false;
    if (!filesystem::exists(csvPath))
    {
        writeHeader = true;
    }
    std::fstream resultsStream{ csvPath, std::ios_base::app };

    for (auto pair : results)
    {
        writer.WriteFinalResultsLine(writeHeader, resultsStream, pair.second);
        writeHeader = false;
    }

    resultsStream.close();
}

//void RecordingAnalyzer::UpdateSummary()
//{
    //std::string testRootPath = GetTestRootPath();
    //filesystem::create_directories(filesystem::path(testRootPath));

    //std::string summaryCsvPath = testRootPath + "Summary.csv";

    //if (filesystem::exists(summaryCsvPath))
    //{
    //    remove(summaryCsvPath.c_str());
    //}

    //std::fstream summaryStream{ summaryCsvPath, std::ios_base::app };
    //summaryStream << "Format,";
    //summaryStream << "Offset Variance,";
    //summaryStream << "Total Valid,";
    //summaryStream << "Total Invalid,";
    //summaryStream << "Min Offset,";
    //summaryStream << "Max Offset" << endl;
    //summaryStream.close();

    //summaryStream.open(summaryCsvPath, std::ios_base::app);

    //for (const filesystem::directory_entry& entry : filesystem::directory_iterator(testRootPath))
    //{
    //    if (!entry.is_directory())
    //    {
    //        continue;
    //    }

    //    struct Summary
    //    {
    //        std::string Format = "";
    //        float MinOffset = 999999;
    //        float MaxOffset = -999999;
    //        int TotalValid = 0;
    //        int TotalInvalid = 0;
    //        float OffsetVariance()
    //        {
    //            return MaxOffset - MinOffset;
    //        }
    //    };

    //    Summary summary;
    //    summary.Format = entry.path().filename().string();

    //    filesystem::path validResultsPath = entry.path();
    //    validResultsPath.append(validResultsFilename);
    //    if (filesystem::exists(validResultsPath))
    //    {
    //        summary.TotalValid = CountLinesInFile(validResultsPath.string()) - 1;
    //        GetMinMaxOffset(validResultsPath.string(), summary.MinOffset, summary.MaxOffset);
    //    }

    //    filesystem::path invalidResultsPath = entry.path();
    //    invalidResultsPath.append(invalidResultsFilename);
    //    if (filesystem::exists(invalidResultsPath))
    //    {
    //        summary.TotalInvalid = CountLinesInFile(invalidResultsPath.string()) - 1;
    //    }

    //    if (summary.TotalValid > 0)
    //    {
    //        summaryStream << "\"" << summary.Format << "\","; //"Format,";
    //        if (summary.TotalValid > 1)
    //        {
    //            summaryStream << "\"" << summary.OffsetVariance() << "\","; //"Offset Variance,";
    //        }
    //        else
    //        {
    //            summaryStream << "\"" << "" << "\","; //"Offset Variance,";
    //        }
    //        summaryStream << "\"" << summary.TotalValid << "\","; //"Total Valid,";
    //        summaryStream << "\"" << summary.TotalInvalid << "\","; //"Total Invalid,";
    //        summaryStream << "\"" << summary.MinOffset << "\","; //"Min Offset,";
    //        summaryStream << "\"" << summary.MaxOffset << "\"" << endl; //"Max Offset" << endl;
    //    }
    //}
//}
