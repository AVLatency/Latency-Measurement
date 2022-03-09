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

    bool fallingEdge;
    vector<TickPosition> possibleTickPositions = GetTicks(recordedSamples, recordedSamplesLength, inputSampleRate, GeneratedSamples::GetTickFrequency(config.WaveFormat->nSamplesPerSec), 3, fallingEdge);
    result.PhaseInverted = !fallingEdge;

    if (possibleTickPositions.size() < 3)
    {
        result.ValidResult = false;
        result.InvalidReason = "Cannot find 3 high frequency ticks";
        if (possibleTickPositions.size() > 0)
        {
            result.SamplesToTick1 = possibleTickPositions[0].index;
        }
        if (possibleTickPositions.size() > 1)
        {
            result.SamplesToTick2 = possibleTickPositions[1].index;
        }
        if (possibleTickPositions.size() > 2)
        {
            result.SamplesToTick3 = possibleTickPositions[2].index;
        }
        return result;
    }

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

std::vector<RecordingAnalyzer::TickPosition> RecordingAnalyzer::GetTicks(float* recordedSamples, int recordedSamplesLength, int sampleRate, int expectedTickFrequency, int numTicks, bool& fallingEdge)
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
    // filter out edges that directly surround the highest magnitude edges
    //
    // Next, decide which of the two lists to use (the rising or the falling edge list):
    // first, see if one list has notably higher magnitude of change than the other list:
    //      average all three for each list, see if one is > 0.8 of the other. If it is, use that list
    // otherwise:
    //      use the list that has the earlier first index
    //
    // whether a mic has inverted polarity or not does not matter, but it is returned from this function anyway. (rising edge list is inverted ones)

    float maxAmp = -1;
    float minAmp = 1;
    for (int i = 0; i < recordedSamplesLength; i++)
    {
        if (recordedSamples[i] > maxAmp)
        {
            maxAmp = recordedSamples[i];
        }
        if (recordedSamples[i] < minAmp)
        {
            minAmp = recordedSamples[i];
        }
    }
    float ampRange = maxAmp - minAmp;
    // if it's less than 0.05 times the max possible magnitude in this recording, it can be ignored
    float detectionThresholdRelativeToSampleSet = 0.05 * ampRange;

    int tickDurationInSamples = ceil((float)sampleRate / expectedTickFrequency);

    vector<TickPosition> fallingEdges;
    float largestFallingEdge = 0;
    vector<TickPosition> risingEdges;
    float largestRisingEdge = 0;

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

        bool falling = recordedSamples[i] > recordedSamples[highestMagnitudeIndex];

        // This next if statement contains optimizations to prevent every single change in amplitude being added to the rising or falling edges vector:
        if (highestMagnitude > TestConfiguration::DetectionThreshold() // This is needed to address cable crosstalk. It also addressess the scenario where there are no legitimate ticks in this sample set.
            && highestMagnitude > detectionThresholdRelativeToSampleSet // This is mostly just an optimization
            && highestMagnitude > 0.25 * (falling ? largestFallingEdge : largestRisingEdge))
        {
            TickPosition tick;
            tick.index = i;
            tick.magnitude = highestMagnitude;
            if (falling)
            {
                fallingEdges.push_back(tick);
                if (highestMagnitude > largestFallingEdge)
                {
                    largestFallingEdge = highestMagnitude;
                }
            }
            else
            {
                risingEdges.push_back(tick);
                if (highestMagnitude > largestRisingEdge)
                {
                    largestRisingEdge = highestMagnitude;
                }
            }
        }
    }

    CleanUpEdgesList(fallingEdges, largestFallingEdge, numTicks, sampleRate);
    CleanUpEdgesList(risingEdges, largestRisingEdge, numTicks, sampleRate);

    vector<TickPosition> result;

    if (fallingEdges.size() < numTicks && risingEdges.size() < numTicks)
    {
        // neither list has enough ticks. Return whichever has more, favouring falling edges.
        if (risingEdges.size() > fallingEdges.size())
        {
            copy(risingEdges.begin(), risingEdges.end(), back_inserter(result));
            fallingEdge = false;
        }
        else
        {
            copy(fallingEdges.begin(), fallingEdges.end(), back_inserter(result));
            fallingEdge = true;
        }
    }
    else if (fallingEdges.size() < numTicks || risingEdges.size() < numTicks)
    {
        // one list has enough ticks, but the other doesn't
        if (risingEdges.size() > fallingEdges.size())
        {
            copy(risingEdges.begin(), risingEdges.begin() + numTicks, back_inserter(result));
            fallingEdge = false;
        }
        else
        {
            copy(fallingEdges.begin(), fallingEdges.begin() + numTicks, back_inserter(result));
            fallingEdge = true;
        }
    }
    else
    {
        // both lists have enough ticks, so favour the one with much higher magnitudes or, if they're around
        // the same magnitudes, favour the one with the earlier ticks
        float avgFalling = 0;
        for (int i = 0; i < numTicks; i++)
        {
            avgFalling += fallingEdges[i].magnitude;
        }
        avgFalling /= numTicks;

        float avgRising = 0;
        for (int i = 0; i < numTicks; i++)
        {
            avgRising += risingEdges[i].magnitude;
        }
        avgRising /= numTicks;

        if (avgFalling > avgRising && avgRising < (0.8 * avgFalling))
        {
            // falling edges have notably higher magnitude
            copy(fallingEdges.begin(), fallingEdges.begin() + numTicks, back_inserter(result));
            fallingEdge = true;
        }
        else if (avgRising > avgFalling && avgFalling < (0.8 * avgRising))
        {
            // rising edges have notably higher magnitude
            copy(risingEdges.begin(), risingEdges.begin() + numTicks, back_inserter(result));
            fallingEdge = false;
        }
        else
        {
            // around the same magnitude, choose the one that has the earliest first index
            if (fallingEdges[0].index < risingEdges[0].index)
            {
                copy(fallingEdges.begin(), fallingEdges.begin() + numTicks, back_inserter(result));
                fallingEdge = true;
            }
            else
            {
                copy(risingEdges.begin(), risingEdges.begin() + numTicks, back_inserter(result));
                fallingEdge = false;
            }
        }
    }

    return result;
}

void RecordingAnalyzer::CleanUpEdgesList(std::vector<RecordingAnalyzer::TickPosition>& edgesList, float largestEdge, int numTicks, int sampleRate)
{
    // Clean up the lists to only have edges that are > 0.25 of the largest one, to match the previous detection logic for edges that are early in the list
    edgesList.erase(remove_if(edgesList.begin(), edgesList.end(), [largestEdge](auto tick) { return tick.magnitude < 0.25 * largestEdge; }), edgesList.end());

    sort(edgesList.begin(), edgesList.end(), [&](auto a, auto b) { return a.magnitude > b.magnitude; });

    // This results in a bunch of edges clustered together (from echos). Now these need to be cleaned up to only contain one from each cluster.
    
    //// METHOD 1: Use the largest magnitude from each cluster
    //std::vector<TickPosition> newList;
    //for (int i = 0; i < numTicks; i++)
    //{
    //    if (i < edgesList.size())
    //    {
    //        int indexLowerBound = edgesList[i].index - round(0.005 * sampleRate); // 5 milliseconds before the highest magnitude
    //        int indexUpperBound = edgesList[i].index - round(0.01 * sampleRate); // 10 milliseconds after the highest magnitude
    //        edgesList.erase(remove_if(edgesList.begin() + i, edgesList.end(), [indexLowerBound, indexUpperBound](auto tick)
    //            {
    //                return tick.index < indexUpperBound && tick.index > indexLowerBound;
    //            }), edgesList.end());
    //    }
    //}

    // METHOD 2: Use the first from each cluster that is within 0.6 times the highest magnitude in the cluster
    // 0.6 times is used because this is more than half of the max, meaning it should represent the top of the
    // first peak, rather than just before the first peak.
    std::vector<TickPosition> newList;
    for (int i = 0; i < numTicks; i++)
    {
        if (i < edgesList.size())
        {
            int indexLowerBound = edgesList[i].index - round(0.005 * sampleRate); // 5 milliseconds before the highest magnitude
            int indexUpperBound = edgesList[i].index - round(0.01 * sampleRate); // 10 milliseconds after the highest magnitude
            int firstEdgeInCluster = i;
            for (int j = i; j < edgesList.size(); j++)
            {
                if (edgesList[j].index < indexUpperBound
                    && edgesList[j].index > indexLowerBound
                    && edgesList[j].magnitude > 0.6 * edgesList[i].magnitude
                    && edgesList[j].index < edgesList[firstEdgeInCluster].index)
                {
                    firstEdgeInCluster = j;
                }
            }
            // replace the highest magnitude edge with the first edge from its cluster
            edgesList[i] = edgesList[firstEdgeInCluster];
            // then remove the old edge that was just copied to the beginning
            edgesList.erase(remove_if(edgesList.begin() + i, edgesList.end(), [indexLowerBound, indexUpperBound](auto tick)
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
