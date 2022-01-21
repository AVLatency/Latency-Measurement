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
    ImGui::Text("[V] Verified Accuracy");
    ImGui::SameLine(); HelpMarker("The accuracy of measurements for this audio format have been verified using electronics measurement equipment, such as an oscilloscope.\n\n"
        "Audio formats that do not have the [V] verified accuracy marker may or may not be accurate, depending on whether the HDMI Audio Device is operating with the same audio output offset as it does for verified formats.\n\n"
        "This is not related to the consistency of measurement results.");

    ChannelDescriptions();
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

void GuiHelper::PeakLevel(AdjustVolumeManager::PeakLevelGrade grade, const char* helpText)
{
    ImGui::Text("Peak level:");
    ImGui::SameLine(); GuiHelper::HelpMarker(helpText);
    ImGui::PushFont(FontHelper::BoldFont);
    switch (grade)
    {
    case AdjustVolumeManager::PeakLevelGrade::Good:
        ImGui::Text("Good");
        break;
    case AdjustVolumeManager::PeakLevelGrade::Loud:
        ImGui::Text("Loud");
        break;
    case AdjustVolumeManager::PeakLevelGrade::Quiet:
        ImGui::TextColored((ImVec4)ImColor::HSV(0, 0.6f, 0.6f), "Quiet");
        break;
    default:
        break;
    }
    ImGui::PopFont();
}

void GuiHelper::VerifiedMarker(bool verified, float dpiScale)
{
    if (verified)
    {
        ImGui::Text("[V]"); ImGui::SameLine();
    }
    ImGui::SetCursorPosX(dpiScale * 35);
}

void GuiHelper::LeoBodnarNote(const AudioFormat* format)
{
    if (format->WaveFormat->nChannels == 2 && format->WaveFormat->nSamplesPerSec == 48000 && format->WaveFormat->wBitsPerSample == 16)
    {
        ImGui::SameLine();
        ImGui::PushFont(FontHelper::BoldFont);
        ImGui::Text("[Leo Bodnar]");
        ImGui::PopFont();
        ImGui::SameLine(); GuiHelper::HelpMarker("This audio format is used by the Leo Bodnar Input Lag Tester. "
            "Visit avlatency.com for instructions on how to measure and calculate video latency with the Leo Bodnar tool and how to calculate lip sync error with the results from this software.\n\n"
            "Please note that some TVs will switch to \"PC\" video mode when connected to a computer and will not switch to \"PC\" video mode when using the Leo Bodnar tester. "
            "Please ensure both tests are using the same video mode to accurately calculate lip sync error.");
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