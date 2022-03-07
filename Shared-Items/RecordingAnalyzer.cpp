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
#include "TestConfiguration.h"
#include <ctime>
using namespace std;

const std::string RecordingAnalyzer::validRecordingsFilename{ "Valid-Individual-Recordings.csv" };
const std::string RecordingAnalyzer::invalidRecordingsFilename{ "Invalid-Individual-Recordings.csv" };

RecordingResult RecordingAnalyzer::AnalyzeRecording(const GeneratedSamples& generatedSamples, const WasapiInput& input, AudioFormat* format, OutputOffsetProfile* currentProfile, DacLatencyProfile* referenceDacLatency)
{
    OutputOffsetProfile::OutputOffset offset = currentProfile->GetOffsetFromWaveFormat(format->WaveFormat);
    RecordingResult result(format, currentProfile->Name, offset.value, offset.verified, referenceDacLatency->Name, referenceDacLatency->Latency);

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
    vector<TickPosition> allTickPositions;

    bool lastIndexWasHigh = recordedSamples[0] > 0;
    int minIndex = 0;
    int maxIndex = 0;
    for (int i = 0; i < recordedSamplesLength; i++)
    {
        if (lastIndexWasHigh)
        {
            if (recordedSamples[i] > recordedSamples[maxIndex])
            {
                maxIndex = i;
            }
        }
        else
        {
            if (recordedSamples[i] < recordedSamples[minIndex])
            {
                minIndex = i;
            }
        }

        bool isIndexHigh = recordedSamples[i] > 0;

        if (lastIndexWasHigh != isIndexHigh)
        {
            // We just crossed the zero mark

            // Checking for slightly (2 times) lower frequency ticks just in case the speaker / DAC is slow
            // to suddenly present a high frequency.
            if (i - (lastIndexWasHigh ? maxIndex : minIndex) <= tickDurationInSamples / 2) // this should actually be only (tickDurationInSamples / 4) samples long
            {
                // This tick seems to be a high frequency and is likely real. Let's keep track of its position.
                TickPosition pos;
                pos.tickInverted = !lastIndexWasHigh;
                pos.index = pos.tickInverted ? minIndex : maxIndex;
                allTickPositions.push_back(pos);
            }

            // reset and continue going through the samples
            lastIndexWasHigh = isIndexHigh;
            minIndex = i;
            maxIndex = i;
        }
    }

    // Find the min and max amplitudes to help further filter the ticks that have been found
    float min = 0;
    float max = 0;
    for (TickPosition tick : allTickPositions)
    {
        if (tick.tickInverted && recordedSamples[tick.index] < min)
        {
            min = recordedSamples[tick.index];
        }
        if (!tick.tickInverted && recordedSamples[tick.index] > max)
        {
            max = recordedSamples[tick.index];
        }
    }

    vector<TickPosition> possibleTickPositions; // This is a filtered list of allTickPositions that only contains the first ticks of a group of ticks
    // min and max acceptable are set to .4 because sampling alignment means that it is possible to sample anywhere between 1x and 0.5x of the tick at 48 kHz input. (Nyquist frequency stuff)
    float minAcceptable = min * 0.4;
    float maxAcceptable = max * 0.4;
    for (TickPosition tick : allTickPositions)
    {
        if (possibleTickPositions.size() > 0)
        {
            if (tick.index - possibleTickPositions[possibleTickPositions.size() - 1].index < tickDurationInSamples * 75) // 75 times the tick duration to give lots of time for echos to settle down
            {
                // In this case, we've already recorded this group of ticks to possibleTickPositions. It can be ignored.
                continue;
            }
        }
        if ((tick.tickInverted && recordedSamples[tick.index] < minAcceptable)
            || (!tick.tickInverted && recordedSamples[tick.index] > maxAcceptable))
        {
            possibleTickPositions.push_back(tick);
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
        result.InvalidReason = "There are three or more posssible inverted ticks AND three or more possible non-inverted ticks";
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

    // My tests have shown times where the third tick can be a full 5 samples off at 48kHz even though the recording is perfect. This is due to different audio clocks, etc.
    // But to be safe, add on a bit more wiggle room to deal with microphone recording nonsense (acoustics and physical effects, etc.).
    int errorThresholdInSamples = round(10 * (inputSampleRate / 48000.0f));

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

std::vector<AveragedResult> RecordingAnalyzer::AnalyzeResults(std::vector<RecordingResult> results, time_t tTime, const AudioEndpoint& outputEndpoint)
{
    std::vector<AveragedResult> averagedResults;

    for (const RecordingResult& recordingResult : results)
    {
        if (recordingResult.Channel1.ValidResult && recordingResult.Channel2.ValidResult)
        {
            bool alreadyHasAvgResult = false;
            for (AveragedResult& result : averagedResults)
            {
                if (result.Format == recordingResult.Format)
                {
                    result.Offsets.push_back(recordingResult.Offset());
                    alreadyHasAvgResult = true;
                    break;
                }
            }

            if (!alreadyHasAvgResult)
            {
                AveragedResult avgResult(tTime, recordingResult.Format, outputEndpoint, recordingResult.OutputOffsetProfileName, recordingResult.OutputOffsetFromProfile, recordingResult.Verified, recordingResult.ReferenceDacName, recordingResult.ReferenceDacLatency);
                avgResult.Offsets.push_back(recordingResult.Offset());
                averagedResults.push_back(avgResult);
            }
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

void RecordingAnalyzer::SaveRecording(const WasapiInput& input, std::string path, std::string filename)
{
    filesystem::create_directories(filesystem::path(path));

    // http://www.topherlee.com/software/pcm-tut-wavformat.html
    // https://www.cplusplus.com/forum/beginner/166954/
    // https://gist.github.com/csukuangfj/c1d1d769606260d436f8674c30662450

    ofstream f(std::format("{}/{}", path, filename), ios::binary);

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

void RecordingAnalyzer::SaveIndividualResult(IResultsWriter& writer, const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, RecordingResult& result, std::string testRootPath, std::string inputFormat)
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

    writer.WriteIndividualRecordingResults(writeHeader, detailedResultsStream, outputEndpoint, inputEndpoint, result, inputFormat);

    detailedResultsStream.close();
}

void RecordingAnalyzer::SaveFinalResults(IResultsWriter& writer, std::vector<AveragedResult> results, std::string testRootPath, std::string csvFilename)
{
    filesystem::create_directories(filesystem::path(testRootPath));

    std::string csvPath = format("{}/{}", testRootPath, csvFilename);

    bool writeHeader = false;
    if (!filesystem::exists(csvPath))
    {
        writeHeader = true;
    }
    std::fstream resultsStream{ csvPath, std::ios_base::app };

    for (auto avgResult : results)
    {
        writer.WriteFinalResultsLine(writeHeader, resultsStream, avgResult);
        writeHeader = false;
    }

    resultsStream.close();
}
