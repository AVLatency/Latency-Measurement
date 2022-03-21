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
    //auto pos = ImGui::GetCursorPosX();
    //ImGui::SetCursorPosX(pos);

    float plotHeight = 100 * DpiScale;
    ImVec2 tickPlotSize = ImVec2(tickMonitorWidth, plotHeight);
    ImVec2 fullPlotSize = ImVec2(fullMonitorWidth, plotHeight);
    float plotVerticalScale = max(analysis.MaxEdgeMagnitude, *manualThreshold);

    ImGui::Spacing();
    ImGui::PushFont(FontHelper::BoldFont);
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

        HelpMarker("Peak audio level");
        ImGui::SameLine();
        ImGui::SetCursorPosX(firstPlotXPos);
        ImGui::Text(std::format("Duration: {} ms", analysis.RawTickViewLength * 1000.0f / analysis.SampleRate).c_str());
        ImGui::SameLine();
        HelpMarker("X axis: Time\nY axis: Normalized magnitude of edges of high frequencies (> ~11 kHz)");
        ImGui::SameLine();
        ImGui::SetCursorPosX(secondPlotXPos);
        ImGui::Text(std::format("Duration: {} ms", analysis.RawFullViewLength * 1000.0f / analysis.SampleRate).c_str());
        ImGui::SameLine();
        HelpMarker("X axis: Time\nY axis: Normalized magnitude of edges of high frequencies (> ~11 kHz)");
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

        HelpMarker("Peak audio level");
        ImGui::SameLine();
        ImGui::SetCursorPosX(firstPlotXPos);
        ImGui::Text(std::format("Duration: {} ms", analysis.TickMonitorSamplesLength * 1000.0f / analysis.SampleRate).c_str());
        ImGui::SameLine();
        HelpMarker("X axis: Time\nY axis: Normalized magnitude of edges of high frequencies (> ~11 kHz)");
        ImGui::SameLine();
        ImGui::SetCursorPosX(secondPlotXPos);
        ImGui::Text(std::format("Duration: {} ms", analysis.FullMonitorSamplesLength * 1000.0f / analysis.SampleRate).c_str());
        ImGui::SameLine();
        HelpMarker("X axis: Time\nY axis: Normalized magnitude of edges of high frequencies (> ~11 kHz)");
    }

    ImGui::Checkbox("Automatic Threshold Detection", useAutoThreshold);
    if (!*useAutoThreshold)
    {
        ImGui::DragFloat("Manual Threshold", manualThreshold, 0.001, 0, 1.9, "%.4f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
    }

    ImGui::Checkbox("Cable Crosstalk Detection", cableCrosstalkDetection);

    ImGui::PopID();
}

void GuiHelper::PeakLevel(AdjustVolumeManager::PeakLevelGrade grade, const char* helpText)
{
    ImGui::Text("Volume level:");
    ImGui::SameLine();
    ImGui::PushFont(FontHelper::BoldFont);
    switch (grade)
    {
    case AdjustVolumeManager::PeakLevelGrade::Good:
        ImGui::Text("OK");
        break;
    case AdjustVolumeManager::PeakLevelGrade::Quiet:
        ImGui::TextColored((ImVec4)ImColor::HSV(0, 0.6f, 0.6f), "Noisy / Quiet");
        break;
    case AdjustVolumeManager::PeakLevelGrade::Crosstalk:
        ImGui::TextColored((ImVec4)ImColor::HSV(0, 0.6f, 0.6f), "Cable crosstalk detected");
        break;
    default:
        break;
    }
    ImGui::PopFont();
    //ImGui::SameLine(); GuiHelper::HelpMarker(helpText);
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
