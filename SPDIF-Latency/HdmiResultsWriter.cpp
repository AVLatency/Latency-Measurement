#include "HdmiResultsWriter.h"
#include "TestNotes.h"
#include <imgui.h>

HdmiResultsWriter HdmiResultsWriter::Writer;

void HdmiResultsWriter::WriteIndividualRecordingResults(bool writeHeader, std::fstream& detailedResultsStream, const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, RecordingResult& result, std::string inputFormat)
{
    if (writeHeader)
    {
        detailedResultsStream << "Time,";
        detailedResultsStream << "Audio Format,";
        detailedResultsStream << "Recording,";
        detailedResultsStream << ",";
        detailedResultsStream << "Audio Latency (ms),";
        detailedResultsStream << "Verified,";
        detailedResultsStream << ",";
        detailedResultsStream << "Raw Offset (ms),";
        detailedResultsStream << "Audio Device,";
        detailedResultsStream << "Recording Method,";
        detailedResultsStream << "Output Offset Profile,";
        detailedResultsStream << "Output Offset Profile Value (ms),";
        detailedResultsStream << ",";
        detailedResultsStream << "DUT Model,";
        detailedResultsStream << "DUT Firmware Version,";
        detailedResultsStream << "DUT Output Type,";
        detailedResultsStream << "DUT Audio Settings,";
        detailedResultsStream << "DUT Other Settings,";
        detailedResultsStream << ",";
        detailedResultsStream << "Audio Format,";
        detailedResultsStream << "Audio Channels,";
        detailedResultsStream << "Audio Sample Rate,";
        detailedResultsStream << "Audio Bit Depth,";
        detailedResultsStream << "Audio Speakers Description,";
        detailedResultsStream << ",";
        detailedResultsStream << "Notes 1,";
        detailedResultsStream << "Notes 2,";
        detailedResultsStream << "Notes 3,";
        detailedResultsStream << "Notes 4,";
        detailedResultsStream << ",";
        detailedResultsStream << "Audio Output Device,";
        detailedResultsStream << "Audio Input Device,";
        detailedResultsStream << "Audio Input Format,";
        detailedResultsStream << ",";
        detailedResultsStream << "DEBUG INFO FOLLOWS -->,";
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
        detailedResultsStream << "Ch 2 Invalid Reason" << std::endl;
    }

    detailedResultsStream << "\"" << StringHelper::GetTimeString(result.Time, false) << "\","; //"Time,";
    detailedResultsStream << "\"" << result.Format->FormatString << "\","; //"Audio Format,";
    detailedResultsStream << "\"" << result.GUID << ".wav\","; //"Recording,";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << result.AudioLatency() << "\","; //"Audio Latency (ms),";
    detailedResultsStream << "\"" << (result.Verified ? "Yes" : "No") << "\","; //"Verified,";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << result.Offset() << "\","; //"Raw Offset (ms),";
    detailedResultsStream << "\"" << TestNotes::Notes.HDMIAudioDevice << "\","; //"Audio Device,";
    detailedResultsStream << "\"" << TestNotes::Notes.RecordingMethod() << "\","; //"Recording Method,";
    detailedResultsStream << "\"" << result.OutputOffsetProfileName << "\","; //"Output Offset Profile,";
    detailedResultsStream << "\"" << result.OutputOffsetFromProfile << "\","; //"Output Offset Profile Value (ms),";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << TestNotes::Notes.DutModel << "\","; //"DUT Model,";
    detailedResultsStream << "\"" << TestNotes::Notes.DutFirmwareVersion << "\","; //"DUT Firmware Version,";
    detailedResultsStream << "\"" << TestNotes::Notes.DutOutputType() << "\","; //"DUT,";
    detailedResultsStream << "\"" << TestNotes::Notes.DutAudioSettings << "\",";
    detailedResultsStream << "\"" << TestNotes::Notes.DutOtherSettings << "\",";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << AudioFormat::GetAudioDataFormatString(result.Format->WaveFormat) << "\","; //"Audio Format,";
    detailedResultsStream << "\"" << result.Format->WaveFormat->nChannels << "\",";
    detailedResultsStream << "\"" << result.Format->WaveFormat->nSamplesPerSec << "\",";
    detailedResultsStream << "\"" << result.Format->WaveFormat->wBitsPerSample << "\",";
    detailedResultsStream << "\"" << AudioFormat::GetChannelInfoString(result.Format->WaveFormat) << "\",";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << TestNotes::Notes.Notes1 << "\",";
    detailedResultsStream << "\"" << TestNotes::Notes.Notes2 << "\",";
    detailedResultsStream << "\"" << TestNotes::Notes.Notes3 << "\",";
    detailedResultsStream << "\"" << TestNotes::Notes.Notes4 << "\",";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << outputEndpoint.Name << " (" << outputEndpoint.ID << ")" << "\","; //"Audio output device,";
    detailedResultsStream << "\"" << inputEndpoint.Name << " (" << inputEndpoint.ID << ")" << "\","; //"Audio input device,";
    detailedResultsStream << "\"" << inputFormat << "\","; //"Audio input format,";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << "DEBUG INFO FOLLOWS -->" << "\",";
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
    detailedResultsStream << "\"" << result.Channel2.InvalidReason << "\"" << std::endl; //"Ch 2 Invalid Reason";
}

void HdmiResultsWriter::WriteFinalResultsLine(bool writeHeader, std::fstream& resultsStream, const AveragedResult& result)
{
    if (writeHeader)
    {
        resultsStream << "Time,";
        resultsStream << "Audio Format,";
        resultsStream << ",";
        resultsStream << "Avg. Audio Latency (ms),";
        resultsStream << "Min Audio Latency (ms),";
        resultsStream << "Max Audio Latency (ms),";
        resultsStream << "Verified Accuracy,";
        resultsStream << "Valid Measurements,";
        resultsStream << ",";
        resultsStream << "Audio Device,";
        resultsStream << "Recording Method,";
        resultsStream << "Output Offset Profile,";
        resultsStream << "Output Offset Profile Value (ms),";
        resultsStream << ",";
        resultsStream << "DUT Model,";
        resultsStream << "DUT Firmware Version,";
        resultsStream << "DUT Output Type,";
        resultsStream << "DUT Audio Settings,";
        resultsStream << "DUT Other Settings,";
        resultsStream << ",";
        resultsStream << "Audio Format,";
        resultsStream << "Audio Channels,";
        resultsStream << "Audio Sample Rate,";
        resultsStream << "Audio Bit Depth,";
        resultsStream << "Audio Speakers Description,";
        resultsStream << ",";
        resultsStream << "Notes 1,";
        resultsStream << "Notes 2,";
        resultsStream << "Notes 3,";
        resultsStream << "Notes 4,";
        resultsStream << ",";
        resultsStream << "Audio Output Device" << std::endl;
    }

    resultsStream << "\"" << StringHelper::GetTimeString(result.Time, false) << "\","; //"Time,";
    resultsStream << "\"" << result.Format->FormatString << "\","; //"Audio Format,";
    resultsStream << "\"" << "\",";
    resultsStream << "\"" << result.AverageLatency() << "\",";
    resultsStream << "\"" << result.MinLatency() << "\",";
    resultsStream << "\"" << result.MaxLatency() << "\",";
    resultsStream << "\"" << (result.Verified ? "Yes" : "No") << "\","; //"Verified,";
    resultsStream << "\"" << result.Offsets.size() << "\",";
    resultsStream << "\"" << "\",";
    resultsStream << "\"" << TestNotes::Notes.HDMIAudioDevice << "\","; //"Audio Device,";
    resultsStream << "\"" << TestNotes::Notes.RecordingMethod() << "\","; //"Recording Method,";
    resultsStream << "\"" << result.OutputOffsetProfileName << "\","; //"Output Offset Profile,";
    resultsStream << "\"" << result.OutputOffsetFromProfile << "\","; //"Output Offset Profile Value (ms),";
    resultsStream << "\"" << "\",";
    resultsStream << "\"" << TestNotes::Notes.DutModel << "\","; //"DUT Model,";
    resultsStream << "\"" << TestNotes::Notes.DutFirmwareVersion << "\","; //"DUT Firmware Version,";
    resultsStream << "\"" << TestNotes::Notes.DutOutputType() << "\","; //"DUT,";
    resultsStream << "\"" << TestNotes::Notes.DutAudioSettings << "\",";
    resultsStream << "\"" << TestNotes::Notes.DutOtherSettings << "\",";
    resultsStream << "\"" << "\",";
    resultsStream << "\"" << AudioFormat::GetAudioDataFormatString(result.Format->WaveFormat) << "\","; //"Audio Format,";
    resultsStream << "\"" << result.Format->WaveFormat->nChannels << "\",";
    resultsStream << "\"" << result.Format->WaveFormat->nSamplesPerSec << "\",";
    resultsStream << "\"" << result.Format->WaveFormat->wBitsPerSample << "\",";
    resultsStream << "\"" << AudioFormat::GetChannelInfoString(result.Format->WaveFormat) << "\",";
    resultsStream << "\"" << "\",";
    resultsStream << "\"" << TestNotes::Notes.Notes1 << "\",";
    resultsStream << "\"" << TestNotes::Notes.Notes2 << "\",";
    resultsStream << "\"" << TestNotes::Notes.Notes3 << "\",";
    resultsStream << "\"" << TestNotes::Notes.Notes4 << "\",";
    resultsStream << "\"" << "\",";
    resultsStream << "\"" << result.OutputEndpoint.Name << " (" << result.OutputEndpoint.ID << ")" << "\"" << std::endl; //"Audio output device,";
}
