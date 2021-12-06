#include "HdmiResultsWriter.h"

HdmiResultsWriter HdmiResultsWriter::Writer;

void HdmiResultsWriter::WriteIndividualRecordingResults(bool writeHeader, std::fstream& detailedResultsStream, const GeneratedSamples& generatedSamples, AudioFormat* audioFormat, int inputSampleRate, RecordingResult& result)
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
        detailedResultsStream << "Output Offset Profile,";
        detailedResultsStream << "Output Offset Profile Value (ms),";
        detailedResultsStream << ",";
        detailedResultsStream << "DUT Model,";
        detailedResultsStream << "DUT Firmware Version,";
        detailedResultsStream << "DUT Output Type,";
        detailedResultsStream << "DUT Video Mode,";
        detailedResultsStream << "DUT Audio Settings,";
        detailedResultsStream << "DUT Other Settings,";
        detailedResultsStream << ",";
        detailedResultsStream << "Audio Format,";
        detailedResultsStream << "Audio Channels,";
        detailedResultsStream << "Audio Sample Rate,";
        detailedResultsStream << "Audio Bit Depth,";
        detailedResultsStream << "Audio Channel Description,";
        detailedResultsStream << ",";
        detailedResultsStream << "Video Resolution,";
        detailedResultsStream << "Video Refresh Rate,";
        detailedResultsStream << "Video Bit Depth,";
        detailedResultsStream << "Video Color Format,";
        detailedResultsStream << "Video Color Space,";
        detailedResultsStream << ",";
        detailedResultsStream << "Notes 1,";
        detailedResultsStream << "Notes 2,";
        detailedResultsStream << "Notes 3,";
        detailedResultsStream << "Notes 4,";
        detailedResultsStream << ",";
        detailedResultsStream << "Audio Output Device,";
        detailedResultsStream << "Audio Input Device,";
        detailedResultsStream << "Audio Input Sample Rate,";
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

    // TODO
    //detailedResultsStream << "\"" << StringHelper::GetTimeString(result.Time, false) << "\","; //"Time,";
    //detailedResultsStream << "\"" << audioFormat->FormatString << "\","; //"Audio Format,";
    //detailedResultsStream << "\"" << result.GUID << ".wav\","; //"Recording,";
    //detailedResultsStream << "\"" << "\",";
    //detailedResultsStream << "\"" << result.Offset() << "\","; //"Audio Latency (ms),"; // TODO
    //detailedResultsStream << "\"" << "No" << "\","; //"Verified,"; // TODO
    //detailedResultsStream << "\"" << "\",";
    //detailedResultsStream << "\"" << result.Offset() << "\","; //"Raw Offset (ms),";
    //detailedResultsStream << "\"" << "TODO" << "\","; //"Output Offset Profile,"; // TODO
    //detailedResultsStream << "\"" << "TODO" << "\","; //"Output Offset Profile Value (ms),"; // TODO
    //detailedResultsStream << "\"" << "\",";
    //detailedResultsStream << "\"" << sysInfo.Display << "\","; //"Display,";
    //detailedResultsStream << "\"" << sysInfo.ComputerName << "\","; //"Computer Name,";
    //detailedResultsStream << "\"" << sysInfo.AdditionalNotes << "\","; //"Additional Notes,";
    //detailedResultsStream << "\"" << "\",";
    //detailedResultsStream << "\"" << outputDeviceName << "\","; //"Audio output device,";
    //detailedResultsStream << "\"" << sysInfo.AudioOutputDrivers << "\","; //"Audio output drivers,";
    //detailedResultsStream << "\"" << sysInfo.VideoOutputDrivers << "\","; //"video output drivers,";
    //detailedResultsStream << "\"" << sysInfo.WindowsVersion << "\","; //"Windows version,";
    //detailedResultsStream << "\"" << "\",";
    //detailedResultsStream << "\"" << inputDeviceName << "\","; //"Audio input device,";
    //detailedResultsStream << "\"" << inputSampleRate << "\","; //"Audio input sample rate,";
    //detailedResultsStream << "\"" << "\",";
    //detailedResultsStream << "\"" << sysInfo.Extractor << "\","; //"Extractor,";
    //detailedResultsStream << "\"" << sysInfo.DAC << "\","; //"DAC,";
    //detailedResultsStream << "\"" << "\",";
    //detailedResultsStream << "\"" << config.WaveFormat->nChannels << "\","; //"Channels,";
    //detailedResultsStream << "\"" << config.WaveFormat->nSamplesPerSec << "\","; //"SampleRate,";
    //detailedResultsStream << "\"" << config.WaveFormat->wBitsPerSample << "\","; //"BitDepth,";
    //detailedResultsStream << "\"" << GetAudioDataFormatString(config.WaveFormat) << "\","; //"AudioFormat,";
    //detailedResultsStream << "\"" << GetChannelMaskString(config.WaveFormat) << "\","; //"ChannelMask,";
    //detailedResultsStream << "\"" << sysInfo.VideoRes << "\","; //"VideoRes,";
    //detailedResultsStream << "\"" << sysInfo.VideoRefresh << "\","; //"VideoRefresh,";
    //detailedResultsStream << "\"" << sysInfo.VideoBitDepth << "\","; //"VideoBitDepth,";
    //detailedResultsStream << "\"" << sysInfo.VideoFormat << "\","; //"VideoFormat,";
    //detailedResultsStream << "\"" << sysInfo.VideoRange << "\","; //"VideoRange,";
    //detailedResultsStream << "\"" << "\",";
    //detailedResultsStream << "\"" << result.Channel1.MillisecondsToTick1() << "\","; //"Ch 1 Milliseconds to Tick 1,";
    //detailedResultsStream << "\"" << result.Channel2.MillisecondsToTick1() << "\","; //"Ch 2 Milliseconds to Tick 1,";
    //detailedResultsStream << "\"" << result.Channel1.RelMillisecondsToTick2() << "\","; //"Ch 1 Rel Milliseconds to Tick 2,";
    //detailedResultsStream << "\"" << result.Channel2.RelMillisecondsToTick2() << "\","; //"Ch 2 Rel Milliseconds to Tick 2,";
    //detailedResultsStream << "\"" << result.Channel1.RelMillisecondsToTick3() << "\","; //"Ch 1 Rel Milliseconds to Tick 3,";
    //detailedResultsStream << "\"" << result.Channel2.RelMillisecondsToTick3() << "\","; //"Ch 2 Rel Milliseconds to Tick 3,";
    //detailedResultsStream << "\"" << (result.Channel1.PhaseInverted ? "true" : "false") << "\","; //"Ch 1 Phase Inverted,";
    //detailedResultsStream << "\"" << result.Channel1.SamplesToTick1 << "\","; //"Ch 1 Samples to Tick 1,";
    //detailedResultsStream << "\"" << result.Channel1.SamplesToTick2 << "\","; //"Ch 1 Samples to Tick 2,";
    //detailedResultsStream << "\"" << result.Channel1.SamplesToTick3 << "\","; //"Ch 1 Samples to Tick 3,";
    //detailedResultsStream << "\"" << (result.Channel2.PhaseInverted ? "true" : "false") << "\","; //"Ch 2 Phase Inverted,";
    //detailedResultsStream << "\"" << result.Channel2.SamplesToTick1 << "\","; //"Ch 2 Samples to Tick 1,";
    //detailedResultsStream << "\"" << result.Channel2.SamplesToTick2 << "\","; //"Ch 2 Samples to Tick 2,";
    //detailedResultsStream << "\"" << result.Channel2.SamplesToTick3 << "\","; //"Ch 2 Samples to Tick 3,";
    //detailedResultsStream << "\"" << result.Channel1.InvalidReason << "\","; //"Ch 1 Invalid Reason,";
    //detailedResultsStream << "\"" << result.Channel2.InvalidReason << "\"" << endl; //"Ch 2 Invalid Reason";
}
