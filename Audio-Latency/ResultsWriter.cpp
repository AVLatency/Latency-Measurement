#include "ResultsWriter.h"
#include "TestNotes.h"
#include <imgui.h>

ResultsWriter ResultsWriter::Writer;

void ResultsWriter::WriteIndividualRecordingResults(bool writeHeader, std::fstream& detailedResultsStream, const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, RecordingResult& result, std::string inputFormat)
{
    writeHeader ? detailedResultsStream << "Audio Latency Type," : detailedResultsStream << "\"" << OutputOffsetProfile::OutputTypeName(result.OffsetProfile->OutType) << "\",";
    writeHeader ? detailedResultsStream << "Time," : detailedResultsStream << "\"" << StringHelper::GetTimeString(result.Time, false) << "\",";
    writeHeader ? detailedResultsStream << "Audio Format," : detailedResultsStream << "\"" << result.Format->FormatString << "\",";
    writeHeader ? detailedResultsStream << "Recording," : detailedResultsStream << "\"" << result.GUID << ".wav\",";
    writeHeader ? detailedResultsStream << "," : detailedResultsStream << "\"" << "\",";
    writeHeader ? detailedResultsStream << "Audio Latency (ms)," : detailedResultsStream << "\"" << result.AudioLatency() << "\",";
    writeHeader ? detailedResultsStream << "Verified," : detailedResultsStream << "\"" << (result.Verified ? "Yes" : "No") << "\",";
    writeHeader ? detailedResultsStream << "," : detailedResultsStream << "\"" << "\",";
    writeHeader ? detailedResultsStream << "DUT Model," : detailedResultsStream << "\"" << TestNotes::Notes.DutModel << "\",";
    writeHeader ? detailedResultsStream << "DUT Firmware Version," : detailedResultsStream << "\"" << TestNotes::Notes.DutFirmwareVersion << "\",";
    writeHeader ? detailedResultsStream << "DUT Output Type," : detailedResultsStream << "\"" << TestNotes::Notes.DutOutputType() << "\",";
    if (result.OffsetProfile->OutType == OutputOffsetProfile::OutputType::Hdmi)
    {
        writeHeader ? detailedResultsStream << "DUT Video Mode," : detailedResultsStream << "\"" << TestNotes::Notes.DutVideoMode << "\",";
    }
    writeHeader ? detailedResultsStream << "DUT Audio Settings," : detailedResultsStream << "\"" << TestNotes::Notes.DutAudioSettings << "\",";
    writeHeader ? detailedResultsStream << "DUT Other Settings," : detailedResultsStream << "\"" << TestNotes::Notes.DutOtherSettings << "\",";
    writeHeader ? detailedResultsStream << "," : detailedResultsStream << "\"" << "\",";
    writeHeader ? detailedResultsStream << "Audio Format," : detailedResultsStream << "\"" << AudioFormat::GetAudioDataFormatString(result.Format->WaveFormat) << "\",";
    writeHeader ? detailedResultsStream << "Audio Channels," : detailedResultsStream << "\"" << result.Format->WaveFormat->nChannels << "\",";
    writeHeader ? detailedResultsStream << "Audio Sample Rate," : detailedResultsStream << "\"" << result.Format->WaveFormat->nSamplesPerSec << "\",";
    writeHeader ? detailedResultsStream << "Audio Bit Depth," : detailedResultsStream << "\"" << result.Format->WaveFormat->wBitsPerSample << "\",";
    writeHeader ? detailedResultsStream << "Audio Speakers Description," : detailedResultsStream << "\"" << AudioFormat::GetChannelInfoString(result.Format->WaveFormat) << "\",";
    writeHeader ? detailedResultsStream << "," : detailedResultsStream << "\"" << "\",";
    if (result.OffsetProfile->OutType == OutputOffsetProfile::OutputType::Hdmi)
    {
        writeHeader ? detailedResultsStream << "Video Resolution," : detailedResultsStream << "\"" << TestNotes::Notes.VideoRes() << "\",";
        writeHeader ? detailedResultsStream << "Video Refresh Rate," : detailedResultsStream << "\"" << TestNotes::Notes.VideoRefreshRate << "\",";
        writeHeader ? detailedResultsStream << "Video Bit Depth," : detailedResultsStream << "\"" << TestNotes::Notes.VideoBitDepth << "\",";
        writeHeader ? detailedResultsStream << "Video Color Format," : detailedResultsStream << "\"" << TestNotes::Notes.VideoColorFormat << "\",";
        writeHeader ? detailedResultsStream << "Video Color Space," : detailedResultsStream << "\"" << TestNotes::Notes.VideoColorSpace() << "\",";
        writeHeader ? detailedResultsStream << "," : detailedResultsStream << "\"" << "\",";
    }
    writeHeader ? detailedResultsStream << "Notes 1," : detailedResultsStream << "\"" << TestNotes::Notes.Notes1 << "\",";
    writeHeader ? detailedResultsStream << "Notes 2," : detailedResultsStream << "\"" << TestNotes::Notes.Notes2 << "\",";
    writeHeader ? detailedResultsStream << "Notes 3," : detailedResultsStream << "\"" << TestNotes::Notes.Notes3 << "\",";
    writeHeader ? detailedResultsStream << "Notes 4," : detailedResultsStream << "\"" << TestNotes::Notes.Notes4 << "\",";
    writeHeader ? detailedResultsStream << "," : detailedResultsStream << "\"" << "\",";
    writeHeader ? detailedResultsStream << "Raw Offset (ms)," : detailedResultsStream << "\"" << result.Offset() << "\",";
    writeHeader ? detailedResultsStream << "Dual-Out Reference Device," : detailedResultsStream << "\"" << TestNotes::Notes.DaulOutRefDevice << "\",";
    writeHeader ? detailedResultsStream << "Recording Method," : detailedResultsStream << "\"" << TestNotes::Notes.RecordingMethod() << "\",";
    writeHeader ? detailedResultsStream << "Output Offset Profile," : detailedResultsStream << "\"" << result.OffsetProfile->Name << "\",";
    writeHeader ? detailedResultsStream << "Output Offset Profile Value (ms)," : detailedResultsStream << "\"" << result.OutputOffsetFromProfile << "\",";
    writeHeader ? detailedResultsStream << "," : detailedResultsStream << "\"" << "\",";
    writeHeader ? detailedResultsStream << "Audio Output Device," : detailedResultsStream << "\"" << outputEndpoint.Name << " (" << outputEndpoint.ID << ")" << "\",";
    writeHeader ? detailedResultsStream << "Audio Input Device," : detailedResultsStream << "\"" << inputEndpoint.Name << " (" << inputEndpoint.ID << ")" << "\",";
    writeHeader ? detailedResultsStream << "Audio Input Format," : detailedResultsStream << "\"" << inputFormat << "\",";
    writeHeader ? detailedResultsStream << "," : detailedResultsStream << "\"" << "\",";
    writeHeader ? detailedResultsStream << "DEBUG INFO FOLLOWS -->," : detailedResultsStream << "\"" << "DEBUG INFO FOLLOWS -->" << "\",";
    writeHeader ? detailedResultsStream << "," : detailedResultsStream << "\"" << "\",";
    writeHeader ? detailedResultsStream << "Ch 1 Milliseconds to Tick 1," : detailedResultsStream << "\"" << result.Channel1.MillisecondsToTick1() << "\",";
    writeHeader ? detailedResultsStream << "Ch 2 Milliseconds to Tick 1," : detailedResultsStream << "\"" << result.Channel2.MillisecondsToTick1() << "\",";
    writeHeader ? detailedResultsStream << "Ch 1 Rel Milliseconds to Tick 2," : detailedResultsStream << "\"" << result.Channel1.RelMillisecondsToTick2() << "\",";
    writeHeader ? detailedResultsStream << "Ch 2 Rel Milliseconds to Tick 2," : detailedResultsStream << "\"" << result.Channel2.RelMillisecondsToTick2() << "\",";
    writeHeader ? detailedResultsStream << "Ch 1 Rel Milliseconds to Tick 3," : detailedResultsStream << "\"" << result.Channel1.RelMillisecondsToTick3() << "\",";
    writeHeader ? detailedResultsStream << "Ch 2 Rel Milliseconds to Tick 3," : detailedResultsStream << "\"" << result.Channel2.RelMillisecondsToTick3() << "\",";
    writeHeader ? detailedResultsStream << "Ch 1 Detection Threshold," : detailedResultsStream << "\"" << result.Channel1.DetectionThreshold << "\",";
    writeHeader ? detailedResultsStream << "Ch 1 Samples to Tick 1," : detailedResultsStream << "\"" << result.Channel1.SamplesToTick1 << "\",";
    writeHeader ? detailedResultsStream << "Ch 1 Samples to Tick 2," : detailedResultsStream << "\"" << result.Channel1.SamplesToTick2 << "\",";
    writeHeader ? detailedResultsStream << "Ch 1 Samples to Tick 3," : detailedResultsStream << "\"" << result.Channel1.SamplesToTick3 << "\",";
    writeHeader ? detailedResultsStream << "Ch 1 Samples to Earliest 1," : detailedResultsStream << "\"" << result.Channel1.SamplesToIndex1 << "\",";
    writeHeader ? detailedResultsStream << "Ch 1 Samples to Earliest 2," : detailedResultsStream << "\"" << result.Channel1.SamplesToIndex2 << "\",";
    writeHeader ? detailedResultsStream << "Ch 1 Samples to Earliest 3," : detailedResultsStream << "\"" << result.Channel1.SamplesToIndex3 << "\",";
    writeHeader ? detailedResultsStream << "Ch 2 Detection Threshold," : detailedResultsStream << "\"" << result.Channel2.DetectionThreshold << "\",";
    writeHeader ? detailedResultsStream << "Ch 2 Samples to Tick 1," : detailedResultsStream << "\"" << result.Channel2.SamplesToTick1 << "\",";
    writeHeader ? detailedResultsStream << "Ch 2 Samples to Tick 2," : detailedResultsStream << "\"" << result.Channel2.SamplesToTick2 << "\",";
    writeHeader ? detailedResultsStream << "Ch 2 Samples to Tick 3," : detailedResultsStream << "\"" << result.Channel2.SamplesToTick3 << "\",";
    writeHeader ? detailedResultsStream << "Ch 2 Samples to Earliest 1," : detailedResultsStream << "\"" << result.Channel2.SamplesToIndex1 << "\",";
    writeHeader ? detailedResultsStream << "Ch 2 Samples to Earliest 2," : detailedResultsStream << "\"" << result.Channel2.SamplesToIndex2 << "\",";
    writeHeader ? detailedResultsStream << "Ch 2 Samples to Earliest 3," : detailedResultsStream << "\"" << result.Channel2.SamplesToIndex3 << "\",";
    writeHeader ? detailedResultsStream << "Ch 1 Invalid Reason," : detailedResultsStream << "\"" << result.Channel1.InvalidReason << "\",";
    writeHeader ? detailedResultsStream << "Ch 2 Invalid Reason" : detailedResultsStream << "\"" << result.Channel2.InvalidReason << "\"";

    detailedResultsStream << std::endl;

    if (writeHeader)
    {
        WriteIndividualRecordingResults(false, detailedResultsStream, outputEndpoint, inputEndpoint, result, inputFormat);
    }
}

void ResultsWriter::WriteFinalResultsLine(bool writeHeader, std::fstream& resultsStream, const AveragedResult& result)
{
    writeHeader ? resultsStream << "Audio Latency Type," : resultsStream << "\"" << OutputOffsetProfile::OutputTypeName(result.OffsetProfile->OutType) << "\",";
    writeHeader ? resultsStream << "Time," : resultsStream << "\"" << StringHelper::GetTimeString(result.Time, false) << "\",";
    writeHeader ? resultsStream << "Audio Format," : resultsStream << "\"" << result.Format->FormatString << "\",";
    writeHeader ? resultsStream << "," : resultsStream << "\"" << "\",";
    writeHeader ? resultsStream << "Avg. Audio Latency (ms)," : resultsStream << "\"" << result.AverageLatency() << "\",";
    writeHeader ? resultsStream << "Min Audio Latency (ms)," : resultsStream << "\"" << result.MinLatency() << "\",";
    writeHeader ? resultsStream << "Max Audio Latency (ms)," : resultsStream << "\"" << result.MaxLatency() << "\",";
    writeHeader ? resultsStream << "Verified Accuracy," : resultsStream << "\"" << (result.Verified ? "Yes" : "No") << "\",";
    writeHeader ? resultsStream << "Valid Measurements," : resultsStream << "\"" << result.Offsets.size() << "\",";
    writeHeader ? resultsStream << "," : resultsStream << "\"" << "\",";
    writeHeader ? resultsStream << "DUT Model," : resultsStream << "\"" << TestNotes::Notes.DutModel << "\",";
    writeHeader ? resultsStream << "DUT Firmware Version," : resultsStream << "\"" << TestNotes::Notes.DutFirmwareVersion << "\",";
    writeHeader ? resultsStream << "DUT Output Type," : resultsStream << "\"" << TestNotes::Notes.DutOutputType() << "\",";
    if (result.OffsetProfile->OutType == OutputOffsetProfile::OutputType::Hdmi)
    {
        writeHeader ? resultsStream << "DUT Video Mode," : resultsStream << "\"" << TestNotes::Notes.DutVideoMode << "\",";
    }
    writeHeader ? resultsStream << "DUT Audio Settings," : resultsStream << "\"" << TestNotes::Notes.DutAudioSettings << "\",";
    writeHeader ? resultsStream << "DUT Other Settings," : resultsStream << "\"" << TestNotes::Notes.DutOtherSettings << "\",";
    writeHeader ? resultsStream << "," : resultsStream << "\"" << "\",";
    writeHeader ? resultsStream << "Audio Format," : resultsStream << "\"" << AudioFormat::GetAudioDataFormatString(result.Format->WaveFormat) << "\",";
    writeHeader ? resultsStream << "Audio Channels," : resultsStream << "\"" << result.Format->WaveFormat->nChannels << "\",";
    writeHeader ? resultsStream << "Audio Sample Rate," : resultsStream << "\"" << result.Format->WaveFormat->nSamplesPerSec << "\",";
    writeHeader ? resultsStream << "Audio Bit Depth," : resultsStream << "\"" << result.Format->WaveFormat->wBitsPerSample << "\",";
    writeHeader ? resultsStream << "Audio Speakers Description," : resultsStream << "\"" << AudioFormat::GetChannelInfoString(result.Format->WaveFormat) << "\",";
    writeHeader ? resultsStream << "," : resultsStream << "\"" << "\",";
    if (result.OffsetProfile->OutType == OutputOffsetProfile::OutputType::Hdmi)
    {
        writeHeader ? resultsStream << "Video Resolution," : resultsStream << "\"" << TestNotes::Notes.VideoRes() << "\",";
        writeHeader ? resultsStream << "Video Refresh Rate," : resultsStream << "\"" << TestNotes::Notes.VideoRefreshRate << "\",";
        writeHeader ? resultsStream << "Video Bit Depth," : resultsStream << "\"" << TestNotes::Notes.VideoBitDepth << "\",";
        writeHeader ? resultsStream << "Video Color Format," : resultsStream << "\"" << TestNotes::Notes.VideoColorFormat << "\",";
        writeHeader ? resultsStream << "Video Color Space," : resultsStream << "\"" << TestNotes::Notes.VideoColorSpace() << "\",";
        writeHeader ? resultsStream << "," : resultsStream << "\"" << "\",";
    }
    writeHeader ? resultsStream << "Notes 1," : resultsStream << "\"" << TestNotes::Notes.Notes1 << "\",";
    writeHeader ? resultsStream << "Notes 2," : resultsStream << "\"" << TestNotes::Notes.Notes2 << "\",";
    writeHeader ? resultsStream << "Notes 3," : resultsStream << "\"" << TestNotes::Notes.Notes3 << "\",";
    writeHeader ? resultsStream << "Notes 4," : resultsStream << "\"" << TestNotes::Notes.Notes4 << "\",";
    writeHeader ? resultsStream << "," : resultsStream << "\"" << "\",";
    writeHeader ? resultsStream << "Dual-Out Reference Device," : resultsStream << "\"" << TestNotes::Notes.DaulOutRefDevice << "\",";
    writeHeader ? resultsStream << "Recording Method," : resultsStream << "\"" << TestNotes::Notes.RecordingMethod() << "\",";
    writeHeader ? resultsStream << "Output Offset Profile," : resultsStream << "\"" << result.OffsetProfile->Name << "\",";
    writeHeader ? resultsStream << "Output Offset Profile Value (ms)," : resultsStream << "\"" << result.OutputOffsetFromProfile << "\",";
    writeHeader ? resultsStream << "," : resultsStream << "\"" << "\",";
    writeHeader ? resultsStream << "Audio Output Device" : resultsStream << "\"" << result.OutputEndpoint.Name << " (" << result.OutputEndpoint.ID << ")" << "\"";

    resultsStream << std::endl;

    if (writeHeader)
    {
        WriteFinalResultsLine(false, resultsStream, result);
    }
}
