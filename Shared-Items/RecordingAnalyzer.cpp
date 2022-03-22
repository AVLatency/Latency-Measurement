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

    // TODO: each of these could be done on a different thread to improve performance.
    result.Channel1 = AnalyzeSingleChannel(generatedSamples, ch1RecordedSamples, channelSamplesLength, inputSampleRate, TestConfiguration::Ch1DetectionThreshold);
    result.Channel2 = AnalyzeSingleChannel(generatedSamples, ch2RecordedSamples, channelSamplesLength, inputSampleRate, TestConfiguration::Ch2DetectionThreshold);

    delete[] ch1RecordedSamples;
    delete[] ch2RecordedSamples;
    return result;
}

RecordingSingleChannelResult RecordingAnalyzer::AnalyzeSingleChannel(const GeneratedSamples& config, float* recordedSamples, int recordedSamplesLength, int inputSampleRate, float threshold)
{
    RecordingSingleChannelResult result;
    result.RecordingSampleRate = inputSampleRate;
    result.DetectionThreshold = threshold;

    vector<TickPosition> possibleTickPositions = GetTicks(recordedSamples, recordedSamplesLength, inputSampleRate, GeneratedSamples::GetTickFrequency(config.WaveFormat->nSamplesPerSec), 3, threshold);

    if (possibleTickPositions.size() < 3)
    {
        result.ValidResult = false;
        result.InvalidReason = "Cannot find 3 high frequency ticks";
        if (possibleTickPositions.size() > 0)
        {
            result.SamplesToTick1 = possibleTickPositions[0].endIndex;
        }
        if (possibleTickPositions.size() > 1)
        {
            result.SamplesToTick2 = possibleTickPositions[1].endIndex;
        }
        if (possibleTickPositions.size() > 2)
        {
            result.SamplesToTick3 = possibleTickPositions[2].endIndex;
        }
        return result;
    }

    result.SamplesToTick1 = possibleTickPositions[0].endIndex;
    result.SamplesToTick2 = possibleTickPositions[1].endIndex;
    result.SamplesToTick3 = possibleTickPositions[2].endIndex;

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
        result.ValidResult = true;
    }

    return result;
}

std::vector<RecordingAnalyzer::TickPosition> RecordingAnalyzer::GetTicks(float* recordedSamples, int recordedSamplesLength, int sampleRate, int expectedTickFrequency, int numTicks, float threshold)
{
    // Needs to work with the following scenarios:
    // - Legitimate ticks exist in the sample set
    //      - clean direct line recording
    //      - echoy microphone recording where the highest tick peak might be in the middle of a noisy mess
    // - no ticks exist at all, but instead lots of low amplitude noise that could look like ticks
    // - cable crosstalk from a different channel that has a legitimate tick, but isn't actually a singal that's intended for this channel
    // The solution to those scenarios is to heavily depend on a correclty configured threshold, handled by the AdjustVolumeManager.

    // Algorithm goes something like this:
    // 
    // look for all rising and falling edges, recording magnitude
    // magnitude is greatest change over tickDurationInSamples / 2, which is the tick wave frequency we expect
    // sort each list of edges by magnitude. There will be clusters of large edges surrounding the largest edges
    // record the earliest edge from each cluster. This is the tick.
    // Then find the peak that follows that earliest edge index. This may be the first or second peak
    // of the original tick signal, but that's OK: it's a small time difference between the two.

    int tickDurationInSamples = ceil((float)sampleRate / expectedTickFrequency);
    int halfTickDurationInSamples = ceil((float)(sampleRate / 2) / expectedTickFrequency);

    TickPosition* allEdges = new TickPosition[recordedSamplesLength];
    vector<TickPosition> largeEdges;
    for (int i = 0; i < recordedSamplesLength; i++)
    {
        float highestMagnitude = 0;
        int highestMagnitudeIndex = i;
        // The highest change in magnitude for a tick will occur over halfTickDurationInSamples, give or take
        // It's possible that more change happens over a slightly longer period of time, but this is not important
        // because the bulk of the change will still happen over this time, which will cause it to exceed the
        // TestConfiguration::DetectionThreshold, which is all that matters.
        for (int j = i; j < recordedSamplesLength && j - i < halfTickDurationInSamples; j++)
        {
            float thisMagnitude = abs(recordedSamples[i] - recordedSamples[j]);
            if (thisMagnitude > highestMagnitude)
            {
                highestMagnitude = thisMagnitude;
                highestMagnitudeIndex = j;
            }
        }
        allEdges[i].index = i;
        allEdges[i].magnitude = highestMagnitude;
        allEdges[i].endIndex = highestMagnitudeIndex;

        // It is important that the DetectionThreshold is configured to filter cable crosstalk.
        // It may also filter the majority of edges when there are no legitimate ticks in this sample set,
        // but this is more valuable as an optimization because the later code will validate the tick
        // positions based on their expected positions, so noise will usually never result in a valid measurement.
        if (highestMagnitude > threshold)
        {
            largeEdges.push_back(allEdges[i]);
        }
    }

    // The edge list has a bunch of edges clustered together (from echos). Now these need to be cleaned up to only contain one from each cluster.
    {
        // Sort by magnitude to find the top numTicks clusters
        sort(largeEdges.begin(), largeEdges.end(), [&](auto a, auto b) { return a.magnitude > b.magnitude; });

        // for each of the clusters
        for (int i = 0; i < numTicks && i < largeEdges.size(); i++)
        {
            // Find the first edge from this cluster:
            TickPosition clusterStart = largeEdges[i];
            // Look as far as 10 milliseconds back from the largest tick in this cluster.
            // This is a lot of time for the types of echos I would expect and have seen in rough tests,
            // but I don't see why it couldn't go even higher if it's ever needed...
            // (I'm thinking maybe a theatre could have weird echos that result in the maximum
            //  magnitude being a while after the initial sound)
            while (clusterStart.index > largeEdges[i].index - round(0.01 * sampleRate))
            {
                int earliestIndex = clusterStart.index;
                // Look back 3 full cycles at a time. If more than 3 cycles of the tick don't pass the
                // detection threshold test, then we've found the start of this cluster.
                // Using 3 cycles here isn't based off of anything in particular. Just sounds like a good amount
                // to deal with crazy acoustics where echos are causing all types of weird wave effects
                int earliestIndexToCheck = clusterStart.index - (tickDurationInSamples * 3);
                for (int j = clusterStart.index - 1; j >= earliestIndexToCheck && j >= 0; j--)
                {
                    if (allEdges[j].magnitude > threshold)
                    {
                        earliestIndex = j;
                    }
                }
                if (earliestIndex != clusterStart.index)
                {
                    clusterStart = allEdges[earliestIndex];
                    if (earliestIndexToCheck <= 0)
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
            int indexLowerBound = clusterStart.index;
            
            // Now that we've found the first index of the cluster (the start of the first edge that passes the threshold)
            // it's time to detect the peak that follows that edge, which might be a small bit further along.
            // NOTE: This has a limitation that the peak may be the first or second peak in the tick signal! I haven't
            // figured out a way to determine which it is, in a general case that works with weird echos as well.
            // But because this is a simple solution, it's a small price to pay for something that's somewhat reliable.
            // Adjust this end point to be the actual peak of the wave:
            for (int j = clusterStart.endIndex; j < clusterStart.endIndex + halfTickDurationInSamples && j < recordedSamplesLength; j++)
            {
                float newMagnitude = abs(recordedSamples[clusterStart.index] - recordedSamples[j]);
                if (clusterStart.magnitude < newMagnitude)
                {
                    clusterStart.magnitude = newMagnitude;
                    clusterStart.endIndex = j;
                }
            }

            // replace the highest magnitude edge with the earliest peak that we were looking for
            largeEdges[i] = clusterStart;

            // 20 milliseconds after the highest magnitude. Don't worry about any smart detection of the end because
            // we are assuming weird lingering echos.
            // 20 ms is a nice spot because we're expecting tick 2 to arrive 30 ms after tick 1 (see GeneratedSamples::patternTick2RelTime)
            int indexUpperBound = clusterStart.index + round(0.020 * sampleRate);

            // Remove the rest of the edges that are in this cluster
            largeEdges.erase(remove_if(largeEdges.begin() + i + 1, largeEdges.end(), [&](auto tick)
                {
                    return tick.index < indexUpperBound && tick.index >= indexLowerBound;
                }), largeEdges.end());
        }
    }

    // Take the top ticks, up to numTicks
    if (largeEdges.size() > numTicks)
    {
        largeEdges.erase(largeEdges.begin() + numTicks, largeEdges.end());
    }

    // put back in chronological order
    sort(largeEdges.begin(), largeEdges.end(), [&](auto a, auto b) { return a.index < b.index; });

    delete[] allEdges;

    return largeEdges;
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
