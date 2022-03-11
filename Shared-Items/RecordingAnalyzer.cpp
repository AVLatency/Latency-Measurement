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
// TODO: is 0.25 really the best choice? It seems to be, since lower is getting pretty close to cable crosstalk on my presonus audio interface
// when mixing line in and Sure SM58 mic...
const float RecordingAnalyzer::relMinEdgeMagnitude{ 0.25 };

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

    vector<TickPosition> possibleTickPositions = GetTicks(recordedSamples, recordedSamplesLength, inputSampleRate, GeneratedSamples::GetTickFrequency(config.WaveFormat->nSamplesPerSec), 3);

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
        // TODO: (Maybe this could be improved?) With these positions now known, check to make sure that the rest of the wave is smooth.
        // If it isn't, audio migtht be glitching.
        result.ValidResult = true;
    }

    return result;
}

std::vector<RecordingAnalyzer::TickPosition> RecordingAnalyzer::GetTicks(float* recordedSamples, int recordedSamplesLength, int sampleRate, int expectedTickFrequency, int numTicks)
{
    // Needs to work with the following scenarios:
    // - Legitimate ticks exist in the sample set
    //      - clean direct line recording
    //      - echoy microphone recording where the highest tick peak might be in the middle of a noisy mess
    // - no ticks exist at all, but instead lots of low amplitude noise that could look like ticks
    // - cable crosstalk from a different channel that has a legitimate tick, but isn't actually a singal that's intended for this channel

    // Algorithm goes something like this:
    // 
    // look for all rising and falling edges, recording magnitude
    // magnitude is greatest change over tickDurationInSamples, which is as low as half the tick wave frequency we expect
    // and gives some wiggle room for unexpected DAC behaviour, acoustics, audio clock issues, sampling limitations, etc.
    // sort each list of edges by magnitude
    // filter out edges with relatively low magnitude compared to the highest magnitude
    // find the top numTicks in terms of magnitude and assume that these are clusters representing the tick
    // record the earliest edge from each cluster. This is the tick.

    int tickDurationInSamples = ceil((float)sampleRate / expectedTickFrequency);

    // TODO: Maybe interleaving or using a struct would improve performance regarding memory access(?)
    float* allEdgeMagnitudes = new float[recordedSamplesLength];
    int* allEdgeEnds = new int[recordedSamplesLength];

    float largestEdge = 0;
    for (int i = 0; i < recordedSamplesLength; i++)
    {
        float highestMagnitude = 0;
        int highestMagnitudeIndex = i;
        for (int j = i; j < recordedSamplesLength && j - i < tickDurationInSamples; j++)
        {
            float thisMagnitude = abs(recordedSamples[i] - recordedSamples[j]);
            if (thisMagnitude > highestMagnitude)
            {
                highestMagnitude = thisMagnitude;
                highestMagnitudeIndex = j;
            }
        }
        allEdgeMagnitudes[i] = highestMagnitude;
        allEdgeEnds[i] = highestMagnitudeIndex;

        if (highestMagnitude > largestEdge)
        {
            largestEdge = highestMagnitude;
        }
    }

    vector<TickPosition> largeEdges;
    for (int i = 0; i < recordedSamplesLength; i++)
    {
        // This next if statement contains optimizations to prevent every single change in amplitude being added to the rising or falling edges vector:
        if (allEdgeMagnitudes[i] > TestConfiguration::DetectionThreshold() // This is needed to address cable crosstalk. It also addressess the scenario where there are no legitimate ticks in this sample set.
            && allEdgeMagnitudes[i] > relMinEdgeMagnitude * largestEdge) // This filtering allows for easy detection of the first edge in a cluster of edges that represents a tick.
        {
            TickPosition tick;
            tick.index = i;
            tick.endIndex = allEdgeEnds[i];
            tick.magnitude = allEdgeMagnitudes[i];
            largeEdges.push_back(tick);
        }
    }

    CleanUpEdgesList(largeEdges, largestEdge, numTicks, sampleRate);

    delete[] allEdgeMagnitudes;
    delete[] allEdgeEnds;

    return largeEdges;
}

void RecordingAnalyzer::CleanUpEdgesList(std::vector<RecordingAnalyzer::TickPosition>& edgesList, float largestEdge, int numTicks, int sampleRate)
{
    sort(edgesList.begin(), edgesList.end(), [&](auto a, auto b) { return a.magnitude > b.magnitude; });

    // This results in a bunch of edges clustered together (from echos). Now these need to be cleaned up to only contain one from each cluster.
    // Use the first from each cluster:
    for (int i = 0; i < numTicks; i++)
    {
        if (i < edgesList.size())
        {
            int indexLowerBound = edgesList[i].index - round(0.002 * sampleRate); // 2 milliseconds before the highest magnitude to account for weird echos and aucoustics
            int indexUpperBound = edgesList[i].index; // the highest magnitude is the upper bound of this cluster.
            int firstEdgeInCluster = i;
            for (int j = i; j < edgesList.size(); j++)
            {
                if (edgesList[j].index < indexUpperBound
                    && edgesList[j].index > indexLowerBound
                    && edgesList[j].index < edgesList[firstEdgeInCluster].index)
                {
                    firstEdgeInCluster = j;
                }
            }
            // replace the highest magnitude edge with the first edge from its cluster
            edgesList[i] = edgesList[firstEdgeInCluster];
            // Remove the rest of the edges that are in this cluster
            edgesList.erase(remove_if(edgesList.begin() + i + 1, edgesList.end(), [&](auto tick)
                {
                    return tick.index < indexUpperBound && tick.index > indexLowerBound;
                }), edgesList.end());
        }
    }

    // Take the top ticks, up to numTicks
    if (edgesList.size() > numTicks)
    {
        edgesList.erase(edgesList.begin() + numTicks, edgesList.end());
    }

    // put back in chronological order
    sort(edgesList.begin(), edgesList.end(), [&](auto a, auto b) { return a.index < b.index; });
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
