#include "GuiHelper.h"
#include "imgui.h"
#include "FontHelper.h"

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
void GuiHelper::HelpMarker(const char* desc)
{
    ImGui::PushFont(FontHelper::BoldFont);
    ImGui::TextDisabled("(?)");
    ImGui::PopFont();
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void GuiHelper::FormatDescriptions()
{
    ChannelDescriptions();
}

void GuiHelper::VerifiedHelp()
{
    ImGui::SameLine(); HelpMarker("If \"Yes\", The accuracy of measurements for this specific audio format have been verified using electronics measurement equipment, such as an oscilloscope.\n\n"
        "Measurements for audio formats that do not have verified accuracy are likely still extremely accurate, because the devices recommended for use with this tool "
        "operate with the same offset or latency, regardless of audio format.\n\n"
        "This is not related to the consistency of measurement results.");
}

void GuiHelper::ChannelDescriptions()
{
    if (ImGui::TreeNode("Speaker descriptions"))
    {
        ImGui::Text("FL: Front Left\n"
            "FR: Front Right\n"
            "FC: Front Center\n"
            "RC: Rear Center\n"
            "RL: Rear Left (a.k.a. Side Left or Surround Left)\n"
            "RR: Rear Right (a.k.a. Side Right or Surround Right)\n"
            "RLC: Rear Left of Center\n"
            "RRC: Rear Right of Center\n"
            "FLC: Front Left of Center\n"
            "FRC: Front Right of Center\n"
            "LFE: Low Frequency Effect (subwoofer)\n"
            "Default.Speakers: Speakers are chosen by the audio driver\n");
        ImGui::TreePop();
    }
}

void GuiHelper::OtherCombo(const char* comboName, const char* inputTextName, int* index, const char** options, int optionsLength, char* otherText, int otherTextLength)
{
    ImGui::Combo(comboName, index, options, optionsLength);
    if (*index == optionsLength - 1)
    {
        ImGui::InputText(inputTextName, otherText, otherTextLength, ImGuiInputTextFlags_CallbackCharFilter, GuiHelper::CsvInputFilter);
    }
}

void GuiHelper::OptionallyBoldText(const char* text, bool bold)
{
    if (bold)
    {
        ImGui::PushFont(FontHelper::BoldFont);
    }
    ImGui::Text(text);
    if (bold)
    {
        ImGui::PopFont();
    }
}

void GuiHelper::AdjustVolumeDisplay(const char* imGuiID, const AdjustVolumeManager::VolumeAnalysis& analysis, float DpiScale, float tickMonitorWidth, float fullMonitorWidth, const char* title, bool* useAutoThreshold, float* manualThreshold, bool* cableCrosstalkDetection)
{
    if (*useAutoThreshold)
    {
        *manualThreshold = analysis.AutoThreshold;
    }

    ImGui::PushID(imGuiID);

    float plotHeight = 100 * DpiScale;
    ImVec2 tickPlotSize = ImVec2(tickMonitorWidth, plotHeight);
    ImVec2 fullPlotSize = ImVec2(fullMonitorWidth, plotHeight);
    float plotVerticalScale = max(analysis.MaxEdgeMagnitude, *manualThreshold);

    ImGui::Spacing();
    ImGui::PushFont(FontHelper::HeaderFont);
    ImGui::Text(title);
    ImGui::PopFont();
    GuiHelper::PeakLevel(analysis.Grade, "");

    if (ImGui::CollapsingHeader("Raw Wave View"))
    {
        float zeroValues[2]{ 0, 0 };
        auto plotYPos = ImGui::GetCursorPosY();
        ImGui::PlotHistogram("", &analysis.RawWavePeak, 1, 0, NULL, 0, 1, ImVec2(20 * DpiScale, plotHeight));
        ImGui::SameLine();
        auto firstPlotXPos = ImGui::GetCursorPosX();
        ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.61f, 0.61f, 0.61f, 0.50f));
        ImGui::PushStyleColor(ImGuiCol_PlotLinesHovered, ImVec4(0.61f, 0.61f, 0.61f, 0.50f));
        ImGui::PlotLines("", zeroValues, 2, 0, NULL, -1 * analysis.RawWavePeak, analysis.RawWavePeak, tickPlotSize); 
        ImGui::SameLine();
        auto secondPlotXPos = ImGui::GetCursorPosX();
        ImGui::PlotLines("", zeroValues, 2, 0, NULL, -1 * analysis.RawWavePeak, analysis.RawWavePeak, fullPlotSize);
        ImGui::PopStyleColor(); // ImGuiCol_PlotLinesHovered
        ImGui::PopStyleColor(); // ImGuiCol_PlotLines

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_PlotLines, ImGui::GetColorU32(ImGuiCol_PlotHistogram));
        ImGui::SetCursorPosY(plotYPos);
        ImGui::SetCursorPosX(firstPlotXPos);
        ImGui::PlotLines("", analysis.RawWaveSamples + analysis.RawTickViewStartIndex, analysis.RawTickViewLength, 0, NULL, -1 * analysis.RawWavePeak, analysis.RawWavePeak, tickPlotSize);
        ImGui::SameLine();
        ImGui::PlotLines("", analysis.RawWaveSamples + analysis.RawFullViewStartIndex, analysis.RawFullViewLength, 0, NULL, -1 * analysis.RawWavePeak, analysis.RawWavePeak, fullPlotSize);
        ImGui::PopStyleColor(); // ImGuiCol_PlotLines
        ImGui::PopStyleColor(); // ImGuiCol_FrameBg

        HelpMarker("Peak audio level (visible range: 0.0 to 1.0)\n\nNote: Some audio devices are capable of fully capable of audio levels greater than 1.0.");
        ImGui::SameLine();
        ImGui::SetCursorPosX(firstPlotXPos);
        ImGui::Text(std::format("Duration: {:.3} ms", analysis.RawTickViewLength * 1000.0f / analysis.SampleRate).c_str());
        ImGui::SameLine();
        HelpMarker("X axis: Time\nY axis: Normalized audio level");
        ImGui::SameLine();
        ImGui::SetCursorPosX(secondPlotXPos);
        ImGui::Text(std::format("Duration: {:.3} ms", analysis.RawFullViewLength * 1000.0f / analysis.SampleRate).c_str());
        ImGui::SameLine();
        HelpMarker("X axis: Time\nY axis: Normalized audio level (visual aliasing and distortion may occur in this low-fidelity waveform view)");
    }
    ImGui::Spacing();
    if (ImGui::CollapsingHeader("High Frequency Edges", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto plotYPos = ImGui::GetCursorPosY();
        ImGui::PlotHistogram("", &analysis.MaxEdgeMagnitude, 1, 0, NULL, 0, 2, ImVec2(20 * DpiScale, plotHeight));
        ImGui::SameLine();
        auto firstPlotXPos = ImGui::GetCursorPosX();
        ImGui::PushStyleColor(ImGuiCol_PlotHistogramHovered, ImGui::GetColorU32(ImGuiCol_PlotHistogram));
        ImGui::PlotHistogram("", analysis.TickMonitorSamples, analysis.TickMonitorSamplesLength, 0, NULL, 0, plotVerticalScale, tickPlotSize);
        ImGui::SameLine();
        auto secondPlotXPos = ImGui::GetCursorPosX();
        ImGui::PlotHistogram("", analysis.FullMonitorSamples, analysis.FullMonitorSamplesLength, 0, NULL, 0, plotVerticalScale, fullPlotSize);
        ImGui::PopStyleColor(); // ImGuiCol_PlotHistogramHovered

        float thresholdValues[2]{ *manualThreshold, *manualThreshold };
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_PlotLines, ImGui::GetColorU32(ImGuiCol_Text));
        ImGui::SetCursorPosY(plotYPos);
        ImGui::SetCursorPosX(firstPlotXPos);
        ImGui::PlotLines("", thresholdValues, 2, 0, NULL, 0, plotVerticalScale, tickPlotSize);
        ImGui::SameLine();
        ImGui::PlotLines("", thresholdValues, 2, 0, NULL, 0, plotVerticalScale, fullPlotSize);
        ImGui::PopStyleColor(); // ImGuiCol_PlotLines
        ImGui::PopStyleColor(); // ImGuiCol_FrameBg

        HelpMarker("Largest magnitude high frequency edge (visible range: 0.0 to 2.0)\n\nNote: Some audio devices are capable of fully capable of edge magnitudes greater than 2.0.");
        ImGui::SameLine();
        ImGui::SetCursorPosX(firstPlotXPos);
        float duration = analysis.RawTickViewLength == 0 || analysis.TickMonitorSamplesLength == 0 ? 0
            : analysis.TickMonitorSamplesLength * 1000.0f / (((float)analysis.TickMonitorSamplesLength / analysis.RawTickViewLength) * analysis.SampleRate);
        ImGui::Text(std::format("Duration: {:.3} ms", duration).c_str());
        ImGui::SameLine();
        HelpMarker("X axis: Time\nY axis: Normalized magnitude of edges of high frequencies (> ~11 kHz)");
        ImGui::SameLine();
        ImGui::SetCursorPosX(secondPlotXPos);
        duration = analysis.RawFullViewLength == 0 || analysis.FullMonitorSamplesLength == 0 ? 0
            : analysis.FullMonitorSamplesLength * 1000.0f / (((float)analysis.FullMonitorSamplesLength / analysis.RawFullViewLength) * analysis.SampleRate);
        ImGui::Text(std::format("Duration: {:.3} ms", duration).c_str());
        ImGui::SameLine();
        HelpMarker("X axis: Time\nY axis: Normalized magnitude of edges of high frequencies (> ~11 kHz)");
    }

    ImGui::Checkbox("Automatic Threshold Detection", useAutoThreshold);
    ImGui::SameLine();
    HelpMarker("Automatically sets the audio pattern detection threshold to half of the largest edge magnitude. Disable this feature to increase the threshold when there is loud cable crosstalk or background noise.");
    if (!*useAutoThreshold)
    {
        ImGui::DragFloat("Manual Threshold", manualThreshold, 0.001, 0, 1.9, "%.4f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
    }
    else
    {
        ImGui::SameLine();
    }
    ImGui::Checkbox("Cable Crosstalk Detection", cableCrosstalkDetection);
    ImGui::SameLine();
    HelpMarker("Ensures accuracy of measurements by detecting when cable crosstalk may be exceeding the current threshold setting.");

    ImGui::PopID();
}

void GuiHelper::PeakLevel(AdjustVolumeManager::PeakLevelGrade grade, const char* helpText)
{
    ImGui::PushFont(FontHelper::BoldFont);
    ImGui::Text("Signal Quality:");
    ImGui::SameLine();
    switch (grade)
    {
    case AdjustVolumeManager::PeakLevelGrade::Good:
        ImGui::Text("OK");
        break;
    case AdjustVolumeManager::PeakLevelGrade::Quiet:
        ImGui::TextColored((ImVec4)ImColor::HSV(0, 0.5f, 1.0f), "Noisy / Quiet");
        break;
    case AdjustVolumeManager::PeakLevelGrade::Crosstalk:
        ImGui::TextColored((ImVec4)ImColor::HSV(0, 0.5f, 1.0f), "Cable crosstalk detected");
        break;
    default:
        break;
    }
    ImGui::PopFont();
}

void GuiHelper::AdjustVolumeInstructionsTroubleshooting(int lastCheckedInputSampleRate, float* outputVolume, void* exampleTexture, int exampleTextureWidth, int exampleTextureHeight, float DpiScale)
{
    ImGui::PushFont(FontHelper::HeaderFont);
    ImGui::Text("Instructions and Troubleshooting");
    ImGui::PopFont();

    ImGui::Spacing();
    ImGui::PushFont(FontHelper::BoldFont);
    ImGui::Text("Basic Instructions:");
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::Text("Adjust microphone placement, output volume of the DUT, and input device volume to make the Signal Quality for both channels OK.");
    ImGui::Spacing();

    if (ImGui::TreeNode("Example"))
    {
        ImGui::Text(std::format("The following is a screenshot of correctly adjusted volume levels:", lastCheckedInputSampleRate).c_str());
        float exampleTextureScale = 0.95 * DpiScale;
        ImGui::Image(exampleTexture, ImVec2(exampleTextureWidth * exampleTextureScale, exampleTextureHeight * exampleTextureScale));
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Detailed Instructions and Troubleshooting"))
    {
        ImGui::PushFont(FontHelper::BoldFont);
        ImGui::Text("Current Input Sample Rate");
        ImGui::PopFont();
        ImGui::Indent();
            ImGui::Text(std::format("{} Hz", lastCheckedInputSampleRate).c_str());
            ImGui::SameLine();
            HelpMarker("An input device sample rate of 44100 Hz or higher is recommended. This sample rate can be configured in the Windows control panel.");
        ImGui::Unindent();
        
        ImGui::Spacing();
        ImGui::PushFont(FontHelper::BoldFont);
        ImGui::Text("How to Ensure Cable Wiring is Correct");
        ImGui::PopFont();
        ImGui::Indent();
            ImGui::TextWrapped("The level meters on the left side of the \"Raw Wave View\" show the current volume of each channel. "
                "These can be used to give immediate feedback on the audio input of each channel, which can help in determining if your wiring is correct. "
                "Simply tap on the microphone or unplug and re-plug a device and watch for a change in this meter. "
                "If your wiring is correct, changes to one channel should not affect the other.");
        ImGui::Unindent();

        ImGui::Spacing();
        ImGui::PushFont(FontHelper::BoldFont);
        ImGui::Text("How to Fix \"Cable crosstalk detected\"");
        ImGui::PopFont();
        ImGui::Indent();
        ImGui::TextWrapped("Cable crosstalk is most often detected when one of the two channels is recieving no audio signal at all. "
            "When no audio signal is present, a very weak cable crosstalk signal is detected instead. "
            "To address this, first make sure that your wiring is correct and that both channels are receiving audio input. "
            "Next, try manually increasing the Threshold by disabling the \"Automatic threshold detection\" to bring the Threshold above the cable crosstalk signal. "
            "Finally, if those two strategies do not resolve the issue, you may need to use an inline analog volume control on the line-level device to reduce its audio level and, in turn, reduce the cable crosstalk that it is causing.\n\n"
            "NOTE: In cases that are very rare for digital audio, the audio signal for both channels may be very closely aligned. When this happens, you may need to disable cable crosstalk detection. "
            "Before doing so, it is important to test a number of devices and become familiar with how this tool works and gain confidence in your wiring/microphone setup and audio levels.");
        ImGui::Unindent();

        ImGui::Spacing();
        ImGui::PushFont(FontHelper::BoldFont);
        ImGui::Text("How to Fix \"Noisy / Quiet\"");
        ImGui::PopFont();
        ImGui::Indent();
            ImGui::TextWrapped("First ensure your cable wiring is correct using the steps above. Second, turn up the volume of the DUT and position the microphone closer to the DUT's left speaker to increase the signal to noise ratio. "
                "You may also want to check your audio device volume / microphone boost settings.");
        ImGui::Unindent();

        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Advanced Configuration"))
    {
        ImGui::DragFloat("Output Volume", outputVolume, .001f, .1f, 1, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine(); GuiHelper::HelpMarker("Should normally be left at 1. If you are experiencing cable crosstalk, you can try turning this volume down or using a physical, inline volume control on your HDMI Audio Extractor output.");
        ImGui::TreePop();
    }
}

void GuiHelper::DearImGuiLegal()
{
    ImGui::Separator();
    ImGui::TextWrapped("This software uses the Dear ImGui library which is licensed under the following terms:");
    if (ImGui::TreeNode("Dear ImGui permission notice"))
    {
        ImGui::TextWrapped("The MIT License (MIT)\n\n"
            "Copyright(c) 2014 - 2022 Omar Cornut\n\n"
            "Permission is hereby granted, free of charge, to any person obtaining a copy of this softwareand associated documentation files(the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and /or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :\n\n"
            "The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.\n\n"
            "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.");
        ImGui::TreePop();
    }
}

int GuiHelper::CsvInputFilter(ImGuiInputTextCallbackData * data)
{
    if (strchr("\"", (char)data->EventChar)
        || strchr(",", (char)data->EventChar)
        || strchr("\n", (char)data->EventChar)
        || strchr("\r", (char)data->EventChar))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void GuiHelper::DialogVolumeAdjustDisabledCrosstalk(bool openDialog, ImVec2 center)
{
    const char* title = "Cable Crosstalk Detection Disabled";
    if (openDialog)
    {
        ImGui::OpenPopup(title);
    }
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Cable crosstalk detection is an important accuracy feature in this tool:\nDisalbing it may result in an incorrect 0 ms audio latency measurement!\n\n"
            "To address a cable crosstalk problem, try manually increasing the Threshold\ninstead. See the \"Instructions and Troubleshooting\" section for more details.");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}