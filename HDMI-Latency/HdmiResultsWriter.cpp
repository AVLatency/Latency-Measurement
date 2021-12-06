#include "HdmiResultsWriter.h"
#include "TestNotes.h"
#include <imgui.h>

HdmiResultsWriter HdmiResultsWriter::Writer;

void HdmiResultsWriter::WriteIndividualRecordingResults(bool writeHeader, std::fstream& detailedResultsStream, const GeneratedSamples& generatedSamples, AudioFormat* audioFormat, int inputSampleRate, const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, RecordingResult& result)
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
    detailedResultsStream << "\"" << StringHelper::GetTimeString(result.Time, false) << "\","; //"Time,";
    detailedResultsStream << "\"" << audioFormat->FormatString << "\","; //"Audio Format,";
    detailedResultsStream << "\"" << result.GUID << ".wav\","; //"Recording,";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << result.Offset() << "\","; //"Audio Latency (ms),"; // TODO
    detailedResultsStream << "\"" << "No" << "\","; //"Verified,"; // TODO
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << result.Offset() << "\","; //"Raw Offset (ms),";
    detailedResultsStream << "\"" << "TODO" << "\","; //"Output Offset Profile,"; // TODO
    detailedResultsStream << "\"" << "TODO" << "\","; //"Output Offset Profile Value (ms),"; // TODO
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << TestNotes::Notes.DutModel << "\","; //"DUT Model,";
    detailedResultsStream << "\"" << TestNotes::Notes.DutFirmwareVersion << "\","; //"DUT Firmware Version,";
    detailedResultsStream << "\"" << TestNotes::Notes.DutOutputType() << "\","; //"DUT,";
    detailedResultsStream << "\"" << TestNotes::Notes.DutVideoMode << "\","; //"DUT Video Mode,";
    detailedResultsStream << "\"" << TestNotes::Notes.DutAudioSettings << "\",";
    detailedResultsStream << "\"" << TestNotes::Notes.DutOtherSettings << "\",";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << "LPCM" << "\","; //"Audio Format,";
    detailedResultsStream << "\"" << audioFormat->WaveFormat->nChannels << "\",";
    detailedResultsStream << "\"" << audioFormat->WaveFormat->nSamplesPerSec << "\",";
    detailedResultsStream << "\"" << audioFormat->WaveFormat->wBitsPerSample << "\",";
    detailedResultsStream << "\"" << AudioFormat::GetChannelInfoString(audioFormat->WaveFormat) << "\",";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << TestNotes::Notes.VideoRes() << "\",";
    detailedResultsStream << "\"" << TestNotes::Notes.VideoRefreshRate << "\",";
    detailedResultsStream << "\"" << TestNotes::Notes.VideoBitDepth << "\",";
    detailedResultsStream << "\"" << TestNotes::Notes.VideoColorFormat << "\",";
    detailedResultsStream << "\"" << TestNotes::Notes.VideoColorSpace() << "\",";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << TestNotes::Notes.Notes1 << "\",";
    detailedResultsStream << "\"" << TestNotes::Notes.Notes2 << "\",";
    detailedResultsStream << "\"" << TestNotes::Notes.Notes3 << "\",";
    detailedResultsStream << "\"" << TestNotes::Notes.Notes4 << "\",";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << outputEndpoint.Name << " (" << outputEndpoint.ID << ")" << "\","; //"Audio output device,";
    detailedResultsStream << "\"" << inputEndpoint.Name << " (" << inputEndpoint.ID << ")" << "\","; //"Audio input device,";
    detailedResultsStream << "\"" << inputSampleRate << "\","; //"Audio input sample rate,";
    detailedResultsStream << "\"" << "\",";
    detailedResultsStream << "\"" << "\",";
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
