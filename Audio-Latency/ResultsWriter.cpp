#include "ResultsWriter.h"
#include "TestNotes.h"
#include <imgui.h>

ResultsWriter ResultsWriter::Writer;

void ResultsWriter::WriteIndividualRecordingResults(bool writeHeader, std::fstream& detailedResultsStream, AudioEndpoint* outputEndpoint, AudioEndpoint* inputEndpoint, RecordingResult& result, std::string inputFormat)
{
    SupportedAudioFormat* format = result.Format;
    std::string formatString = format == nullptr ? "CurrentWindowsAudioFormat" : format->Format->FormatString;

    writeHeader ? detailedResultsStream << "Audio Latency Type," : detailedResultsStream << "\"" << OutputOffsetProfile::OutputTypeName(result.OffsetProfile->OutType) << "\",";
    writeHeader ? detailedResultsStream << "Time," : detailedResultsStream << "\"" << StringHelper::GetTimeString(result.Time, false) << "\",";
    writeHeader ? detailedResultsStream << "Audio Format," : detailedResultsStream << "\"" << formatString << "\",";
    writeHeader ? detailedResultsStream << "Recording," : detailedResultsStream << "\"" << result.GUID << ".wav\",";
    writeHeader ? detailedResultsStream << "," : detailedResultsStream << "\"" << "\",";
    writeHeader ? detailedResultsStream << "Audio Latency (ms)," : detailedResultsStream << "\"" << result.AudioLatency() << "\",";
    writeHeader ? detailedResultsStream << "Verified," : detailedResultsStream << "\"" << (result.Verified ? "Yes" : "No") << "\",";
    writeHeader ? detailedResultsStream << "," : detailedResultsStream << "\"" << "\",";
    writeHeader ? detailedResultsStream << "DUT Model," : detailedResultsStream << "\"" << TestNotes::Notes.DutModel << "\",";
    writeHeader ? detailedResultsStream << "DUT Firmware Version," : detailedResultsStream << "\"" << TestNotes::Notes.DutFirmwareVersion << "\",";
    if (result.OffsetProfile->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
    {
        writeHeader ? detailedResultsStream << "DUT Output Type," : detailedResultsStream << "\"" << TestNotes::Notes.DutPassthroughOutputType() << "\",";
    }
    else
    {
        writeHeader ? detailedResultsStream << "DUT Output Type," : detailedResultsStream << "\"" << TestNotes::Notes.DutOutputType() << "\",";
    }
    if (result.OffsetProfile->OutType == OutputOffsetProfile::OutputType::Hdmi
        || result.OffsetProfile->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
    {
        writeHeader ? detailedResultsStream << "DUT Video Mode," : detailedResultsStream << "\"" << TestNotes::Notes.DutVideoMode << "\",";
    }
    writeHeader ? detailedResultsStream << "DUT Audio Settings," : detailedResultsStream << "\"" << TestNotes::Notes.DutAudioSettings << "\",";
    writeHeader ? detailedResultsStream << "DUT Other Settings," : detailedResultsStream << "\"" << TestNotes::Notes.DutOtherSettings << "\",";
    writeHeader ? detailedResultsStream << "," : detailedResultsStream << "\"" << "\",";
    writeHeader ? detailedResultsStream << "Audio Encoding," : detailedResultsStream << "\"" << AudioFormat::GetAudioDataEncodingString(format == nullptr ? nullptr : format->Format) << "\",";
    writeHeader ? detailedResultsStream << "Audio Channels," : detailedResultsStream << "\"" << (format == nullptr ? 0 : format->Format->GetNumChannels()) << "\",";
    writeHeader ? detailedResultsStream << "Audio Sample Rate," : detailedResultsStream << "\"" << (format == nullptr ? 0 : format->Format->GetSamplesPerSec()) << "\",";
    writeHeader ? detailedResultsStream << "Audio Bit Depth," : detailedResultsStream << "\"" << (format == nullptr ? 0 : format->Format->GetBitsPerSample()) << "\",";
    writeHeader ? detailedResultsStream << "Audio Speakers Description," : detailedResultsStream << "\"" << AudioFormat::GetChannelInfoString(format == nullptr ? nullptr : format->Format) << "\",";
    writeHeader ? detailedResultsStream << "," : detailedResultsStream << "\"" << "\",";
    if (result.OffsetProfile->OutType == OutputOffsetProfile::OutputType::Hdmi
        || result.OffsetProfile->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
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
    if (result.OffsetProfile->OutType != OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
    {
        writeHeader ? detailedResultsStream << "Recording Method," : detailedResultsStream << "\"" << TestNotes::Notes.RecordingMethod() << "\",";
    }
    writeHeader ? detailedResultsStream << "Output Offset Profile," : detailedResultsStream << "\"" << result.OffsetProfile->Name << "\",";
    writeHeader ? detailedResultsStream << "Output Offset Profile Value (ms)," : detailedResultsStream << "\"" << result.OutputOffsetFromProfile << "\",";
    if (result.OffsetProfile->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
    {
        writeHeader ? detailedResultsStream << "Reference DAC," : detailedResultsStream << "\"" << TestNotes::Notes.DAC << "\",";
        writeHeader ? detailedResultsStream << "Reference DAC Latency Profile," : detailedResultsStream << "\"" << result.ReferenceDacName << "\",";
        writeHeader ? detailedResultsStream << "Reference DAC Latency Value (ms)," : detailedResultsStream << "\"" << result.ReferenceDacLatency << "\",";
    }
    writeHeader ? detailedResultsStream << "," : detailedResultsStream << "\"" << "\",";
    writeHeader ? detailedResultsStream << "Audio Output Device," : detailedResultsStream << "\"" << outputEndpoint->Name << " (" << outputEndpoint->ID << ")" << "\",";
    writeHeader ? detailedResultsStream << "Audio Input Device," : detailedResultsStream << "\"" << inputEndpoint->Name << " (" << inputEndpoint->ID << ")" << "\",";
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
    const SupportedAudioFormat* format = result.Format;
    std::string formatString = format == nullptr ? "CurrentWindowsAudioFormat" : result.Format->Format->FormatString;

    writeHeader ? resultsStream << "Audio Latency Type," : resultsStream << "\"" << OutputOffsetProfile::OutputTypeName(result.OffsetProfile->OutType) << "\",";
    writeHeader ? resultsStream << "Time," : resultsStream << "\"" << StringHelper::GetTimeString(result.Time, false) << "\",";
    writeHeader ? resultsStream << "Audio Format," : resultsStream << "\"" << formatString << "\",";
    writeHeader ? resultsStream << "," : resultsStream << "\"" << "\",";
    writeHeader ? resultsStream << "Avg. Audio Latency (ms)," : resultsStream << "\"" << result.AverageLatency() << "\",";
    writeHeader ? resultsStream << "Min Audio Latency (ms)," : resultsStream << "\"" << result.MinLatency() << "\",";
    writeHeader ? resultsStream << "Max Audio Latency (ms)," : resultsStream << "\"" << result.MaxLatency() << "\",";
    writeHeader ? resultsStream << "Verified Accuracy," : resultsStream << "\"" << (result.Verified ? "Yes" : "No") << "\",";
    writeHeader ? resultsStream << "Valid Measurements," : resultsStream << "\"" << result.Offsets.size() << "\",";
    writeHeader ? resultsStream << "," : resultsStream << "\"" << "\",";
    writeHeader ? resultsStream << "DUT Model," : resultsStream << "\"" << TestNotes::Notes.DutModel << "\",";
    writeHeader ? resultsStream << "DUT Firmware Version," : resultsStream << "\"" << TestNotes::Notes.DutFirmwareVersion << "\",";
    if (result.OffsetProfile->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
    {
        writeHeader ? resultsStream << "DUT Output Type," : resultsStream << "\"" << TestNotes::Notes.DutPassthroughOutputType() << "\",";
    }
    else
    {
        writeHeader ? resultsStream << "DUT Output Type," : resultsStream << "\"" << TestNotes::Notes.DutOutputType() << "\",";
    }
    if (result.OffsetProfile->OutType == OutputOffsetProfile::OutputType::Hdmi
        || result.OffsetProfile->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
    {
        writeHeader ? resultsStream << "DUT Video Mode," : resultsStream << "\"" << TestNotes::Notes.DutVideoMode << "\",";
    }
    writeHeader ? resultsStream << "DUT Audio Settings," : resultsStream << "\"" << TestNotes::Notes.DutAudioSettings << "\",";
    writeHeader ? resultsStream << "DUT Other Settings," : resultsStream << "\"" << TestNotes::Notes.DutOtherSettings << "\",";
    writeHeader ? resultsStream << "," : resultsStream << "\"" << "\",";
    writeHeader ? resultsStream << "Audio Encoding," : resultsStream << "\"" << AudioFormat::GetAudioDataEncodingString(format == nullptr ? nullptr : format->Format) << "\",";
    writeHeader ? resultsStream << "Audio Channels," : resultsStream << "\"" << (format == nullptr ? 0 : format->Format->GetNumChannels()) << "\",";
    writeHeader ? resultsStream << "Audio Sample Rate," : resultsStream << "\"" << (format == nullptr ? 0 : format->Format->GetSamplesPerSec()) << "\",";
    writeHeader ? resultsStream << "Audio Bit Depth," : resultsStream << "\"" << (format == nullptr ? 0 : format->Format->GetBitsPerSample()) << "\",";
    writeHeader ? resultsStream << "Audio Speakers Description," : resultsStream << "\"" << AudioFormat::GetChannelInfoString(format == nullptr ? nullptr : format->Format) << "\",";
    writeHeader ? resultsStream << "," : resultsStream << "\"" << "\",";
    if (result.OffsetProfile->OutType == OutputOffsetProfile::OutputType::Hdmi
        || result.OffsetProfile->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
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
    if (result.OffsetProfile->OutType != OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
    {
        writeHeader ? resultsStream << "Recording Method," : resultsStream << "\"" << TestNotes::Notes.RecordingMethod() << "\",";
    }
    writeHeader ? resultsStream << "Output Offset Profile," : resultsStream << "\"" << result.OffsetProfile->Name << "\",";
    writeHeader ? resultsStream << "Output Offset Profile Value (ms)," : resultsStream << "\"" << result.OutputOffsetFromProfile << "\",";
    if (result.OffsetProfile->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
    {
        writeHeader ? resultsStream << "Reference DAC," : resultsStream << "\"" << TestNotes::Notes.DAC << "\",";
        writeHeader ? resultsStream << "Reference DAC Latency Profile," : resultsStream << "\"" << result.ReferenceDacName << "\",";
        writeHeader ? resultsStream << "Reference DAC Latency Value (ms)," : resultsStream << "\"" << result.ReferenceDacLatency << "\",";
    }
    writeHeader ? resultsStream << "," : resultsStream << "\"" << "\",";
    writeHeader ? resultsStream << "Audio Output Device" : resultsStream << "\"" << result.OutputEndpoint->Name << " (" << result.OutputEndpoint->ID << ")" << "\"";

    resultsStream << std::endl;

    if (writeHeader)
    {
        WriteFinalResultsLine(false, resultsStream, result);
    }
}
