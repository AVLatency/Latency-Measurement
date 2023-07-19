#include "GuiHelper.h"
#include "imgui.h"
#include "FontHelper.h"
#include "TestConfiguration.h"
#include <format>

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

void GuiHelper::DeveloperOptions()
{
#ifdef _DEBUG
    ImGui::Spacing();

    if (ImGui::TreeNode("Developer Debug Options"))
    {
        // TODO: remove this once I've settled on an amplitude I'm happy with. This doesn't actually adjust anything in real-time because the audio tone is set to repeat.
        ImGui::DragFloat("Lead-In Tone Amplitude", &TestConfiguration::LeadInToneAmplitude, 0.001, 0.002, 1, "%.4f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
        ImGui::SameLine(); GuiHelper::HelpMarker("Default: 0.07. This lead-in tone is used to stop dynamic normalization that exists in some onboard microphone inputs. If your input audio device does not have any dynamic normalization (such as a professional audio interface), you can turn this down to make the audio patterns less annoying to listen to.");

        ImGui::Checkbox("Low Freqency Pitch", &TestConfiguration::LowFreqPitch);

        ImGui::DragFloat("Auto Threshold Multiplier", &TestConfiguration::AutoThresholdMultiplier, 0.001, 0.001, .9, "%.4f", ImGuiSliderFlags_AlwaysClamp);

        ImGui::TreePop();
    }
#endif
}

void GuiHelper::FormatDescriptions()
{
    ChannelDescriptions();
    AMDSpeakersNote();
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
            "Default.Speakers: Speakers are chosen by the audio driver");
        ImGui::TreePop();
    }
}

void GuiHelper::AMDSpeakersNote()
{
    if (ImGui::TreeNode("Note on AMD audio drivers"))
    {
        ImGui::TextWrapped("When using AMD audio drivers, speaker configurations will be ignored and the following will be used instead:\n\n"
            "2ch: FL FR\n"
            "4ch: FL FR RL RR\n"
            "6ch: FL FR FC RL RR LFE\n"
            "8ch: FL FR FC RL RR RLC RRC LFE\n");
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

void GuiHelper::AdjustVolumeDisplay(const char* imGuiID, const AdjustVolumeManager::VolumeAnalysis& analysis, float DpiScale, float tickMonitorWidth, float fullMonitorWidth, const char* title, bool* useAutoThreshold, float* manualThreshold, bool* cableCrosstalkDetection, bool setDefaultState)
{
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    if (*useAutoThreshold)
    {
        *manualThreshold = analysis.AutoThreshold;
    }

    ImGui::PushID(imGuiID);

    float plotHeight = 100 * DpiScale;
    ImVec2 tickPlotSize = ImVec2(tickMonitorWidth, plotHeight);
    ImVec2 fullPlotSize = ImVec2(fullMonitorWidth, plotHeight);
    float plotVerticalScale = max(analysis.MaxEdgeMagnitude, *manualThreshold);
    float clipMarkHeight = 5 * DpiScale;
    float clipMarkWidth = 15.5 * DpiScale;
    ImGui::PushFont(FontHelper::HeaderFont);
    ImGui::Text(title);
    ImGui::PopFont();
    GuiHelper::PeakLevel(analysis.Grade, "");

    if (setDefaultState)
    {
        ImGui::SetNextTreeNodeOpen(false);
    }
    if (ImGui::CollapsingHeader("Raw Wave View"))
    {
        float zeroValues[2]{ 0, 0 };
        auto topLeftPos = ImGui::GetCursorPos();
        const ImVec2 topLeftScreenPos = ImGui::GetCursorScreenPos();

        ImGui::PlotHistogram("", &analysis.RawWavePeak, 1, 0, NULL, 0, 1, ImVec2(20 * DpiScale, plotHeight));
        
        if (analysis.RawWavePeak > 0.95)
        {
            ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(topLeftScreenPos.x + 4 * DpiScale, topLeftScreenPos.y), ImVec2(topLeftScreenPos.x + clipMarkWidth, topLeftScreenPos.y + clipMarkHeight), ImColor::HSV(0, 1.0f, 1.0f));
        }

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
        ImGui::SetCursorPosY(topLeftPos.y);
        ImGui::SetCursorPosX(firstPlotXPos);
        ImGui::PlotLines("", analysis.RawWaveSamples + analysis.RawTickViewStartIndex, analysis.RawTickViewLength, 0, NULL, -1 * analysis.RawWavePeak, analysis.RawWavePeak, tickPlotSize);
        ImGui::SameLine();
        ImGui::PlotLines("", analysis.RawWaveSamples + analysis.RawFullViewStartIndex, analysis.RawFullViewLength, 0, NULL, -1 * analysis.RawWavePeak, analysis.RawWavePeak, fullPlotSize);
        ImGui::PopStyleColor(); // ImGuiCol_PlotLines
        ImGui::PopStyleColor(); // ImGuiCol_FrameBg

        HelpMarker("Peak audio level (visible range: 0.0 to 1.0)\n\nNote: Some audio devices are capable of fully capable of audio levels greater than 1.0 without any clipping. Use the Raw Wave View to determine if clipping is occuring with a high audio level.");
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

    if (setDefaultState)
    {
        ImGui::SetNextTreeNodeOpen(true);
    }
    if (ImGui::CollapsingHeader("High Frequency Edges", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto topLeftPos = ImGui::GetCursorPos();
        const ImVec2 topLeftScreenPos = ImGui::GetCursorScreenPos();

        ImGui::PlotHistogram("", &analysis.MaxEdgeMagnitude, 1, 0, NULL, 0, 2, ImVec2(20 * DpiScale, plotHeight));

        if (analysis.MaxEdgeMagnitude > 1.9)
        {
            ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(topLeftScreenPos.x + 4 * DpiScale, topLeftScreenPos.y), ImVec2(topLeftScreenPos.x + clipMarkWidth, topLeftScreenPos.y + clipMarkHeight), ImColor::HSV(0, 1.0f, 1.0f));
        }

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
        ImGui::SetCursorPosY(topLeftPos.y);
        ImGui::SetCursorPosX(firstPlotXPos);
        ImGui::PlotLines("", thresholdValues, 2, 0, NULL, 0, plotVerticalScale, tickPlotSize);
        ImGui::SameLine();
        ImGui::PlotLines("", thresholdValues, 2, 0, NULL, 0, plotVerticalScale, fullPlotSize);
        ImGui::PopStyleColor(); // ImGuiCol_PlotLines
        ImGui::PopStyleColor(); // ImGuiCol_FrameBg

        HelpMarker("Peak magnitude high frequency edge (visible range: 0.0 to 2.0)\n\nNote: Some audio devices are capable of fully capable of edge magnitudes greater than 2.0 without any clipping. Use the Raw Wave View to determine if clipping is occuring with a high audio level.");
        ImGui::SameLine();
        ImGui::SetCursorPosX(firstPlotXPos);
        float duration = analysis.RawTickViewLength == 0 || analysis.TickMonitorSamplesLength == 0 ? 0
            : analysis.TickMonitorSamplesLength * 1000.0f / (((float)analysis.TickMonitorSamplesLength / analysis.RawTickViewLength) * analysis.SampleRate);
        ImGui::Text(std::format("Duration: {:.3} ms", duration).c_str());
        ImGui::SameLine();
        HelpMarker("X axis: Time\nY axis: Normalized magnitude of edges of high frequencies (> ~5.5 kHz)");
        ImGui::SameLine();
        ImGui::SetCursorPosX(secondPlotXPos);
        duration = analysis.RawFullViewLength == 0 || analysis.FullMonitorSamplesLength == 0 ? 0
            : analysis.FullMonitorSamplesLength * 1000.0f / (((float)analysis.FullMonitorSamplesLength / analysis.RawFullViewLength) * analysis.SampleRate);
        ImGui::Text(std::format("Duration: {:.3} ms", duration).c_str());
        ImGui::SameLine();
        HelpMarker("X axis: Time\nY axis: Normalized magnitude of edges of high frequencies (> ~5.5 kHz)");
    }

    ImGui::Checkbox("Automatic Threshold Detection", useAutoThreshold);
    ImGui::SameLine();
    HelpMarker("Automatically sets the audio pattern detection threshold to a portion of the largest edge magnitude.\n\nDisable this feature to increase the threshold when there is loud crosstalk or background noise or decrease the threshold when using professional audio equipment in a quiet environment with lots of echoes.");
    if (!*useAutoThreshold)
    {
        ImGui::DragFloat("Manual Threshold", manualThreshold, 0.001, 0, 1.9, "%.4f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
    }
    else
    {
        ImGui::SameLine();
    }
    ImGui::Checkbox("Crosstalk Detection", cableCrosstalkDetection);
    ImGui::SameLine();
    HelpMarker("Ensures accuracy of measurements by detecting when crosstalk may be exceeding the current threshold setting.");

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
        ImGui::TextColored((ImVec4)ImColor::HSV(0, 0.5f, 1.0f), "Crosstalk detected");
        break;
    default:
        break;
    }
    ImGui::PopFont();
}

std::string GuiHelper::CableHelpText(Tool tool, OutputOffsetProfile::OutputType outType)
{
    if (tool == GuiHelper::Tool::HdmiToDigitalAudio)
    {
        return "For your Dual-Out Reference Device, the time offset between analog audio output and HDMI audio output must be known. For your Reference DAC, the digital to analog latency must be known. Recommended devices can be found on the AV Latency.com Toolkit webpage.\n\n"
            "AV Latency.com Toolkit Webpage: avlatency.com/tools/av-latency-com-toolkit";
    }
    else
    {
        switch (outType)
        {
        case OutputOffsetProfile::OutputType::Spdif:
            return "To record audio output from the Device Under Test (DUT) you can use a microphone or directly connect to the headphone or speaker output of the DUT.\n\n"
                "- Microphone: Make sure to position the mic as close as possible to the speaker because sound travels measurably slow. Position the mic close to the tweeter if there are separate speaker components. When recording with a mic, the Mic port must be used on computers that have separate Line In and Mic ports.\n"
                "- DUT headphone output: Note that speaker and headphone output can sometimes have different latency on some devices.\n"
                "- Directly connect to DUT speaker output: Start the volume low as some amplifiers may be capable of high voltage outputs that could damage your audio input device.\n\n"
                "Your \"Dual-Out Reference Device\" must be capable of analog audio output *and* S/PDIF audio output at the same time. The time offset between analog audio output and S/PDIF audio output (the \"output offset\") must be known. Recommended devices can be found on the AV Latency.com Toolkit webpage.\n\n"
                "AV Latency.com Toolkit Webpage: avlatency.com/tools/av-latency-com-toolkit";
            break;
        case OutputOffsetProfile::OutputType::Hdmi:
        default:
            return "To record audio output from the Device Under Test (DUT) you can use a microphone or directly connect to the headphone or speaker output of the DUT.\n\n"
                "- Microphone: Make sure to position the mic as close as possible to the speaker because sound travels measurably slow. Position the mic close to the tweeter if there are separate speaker components. When recording with a mic, the Mic port must be used on computers that have separate Line In and Mic ports.\n"
                "- DUT headphone output: Note that speaker and headphone output can sometimes have different latency on some devices.\n"
                "- Directly connect to DUT speaker output: Start the volume low as some amplifiers may be capable of high voltage outputs that could damage your audio input device.\n\n"
                "Your \"Dual-Out Reference Device\" must be capable of analog audio output *and* HDMI audio output at the same time. The time offset between analog audio output and HDMI audio output (the \"output offset\") must be known. Recommended devices can be found on the AV Latency.com Toolkit webpage.\n\n"
                "AV Latency.com Toolkit Webpage: avlatency.com/tools/av-latency-com-toolkit";
            break;
        }
    }
}

void GuiHelper::AdjustVolumeInstructionsTroubleshooting(Tool tool, OutputOffsetProfile::OutputType outType, int lastCheckedInputSampleRate, float* outputVolume, bool* overrideNoisyQuiet, void* exampleTexture, int exampleTextureWidth, int exampleTextureHeight, float DpiScale)
{
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::PushFont(FontHelper::HeaderFont);
    ImGui::Text("Instructions and Troubleshooting");
    ImGui::PopFont();

    ImGui::Spacing();
    ImGui::PushFont(FontHelper::BoldFont);
    ImGui::Text("Basic Instructions:");
    ImGui::PopFont();
    ImGui::Text("- Verify device and cable setup using visual feedback from this tool.");
    if (tool == Tool::AudioLatency && outType == OutputOffsetProfile::OutputType::Hdmi)
    {
        ImGui::Text("- Adjust output volume of the DUT and input device volume to make the Signal Quality for both channels OK.");
    }
    else
    {
        ImGui::Text("- Adjust microphone placement, output volume of the DUT, and input device volume to make the Signal Quality for both channels OK.");
    }
    ImGui::Spacing();

    if (ImGui::TreeNode("Example"))
    {
        ImGui::Text(std::format("The following is a screenshot of correctly adjusted volume levels:", lastCheckedInputSampleRate).c_str());
        float exampleTextureScale = 0.95 * DpiScale;
        ImGui::Image(exampleTexture, ImVec2(exampleTextureWidth * exampleTextureScale, exampleTextureHeight * exampleTextureScale));
        ImGui::Spacing();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Wiring and Cable Setup"))
    {
        ImGui::PushFont(FontHelper::BoldFont);
        ImGui::Text("Initial Cable Setup");
        ImGui::PopFont();
        ImGui::Indent();
        ImGui::TextWrapped(std::format("Connect your audio devices and cables as described in the diagram at the top of this window.\n\n{}", GuiHelper::CableHelpText(tool, outType)).c_str());
        ImGui::Unindent();

        ImGui::Spacing();
        ImGui::PushFont(FontHelper::BoldFont);
        ImGui::Text("How to Ensure Cable Wiring is Correct");
        ImGui::PopFont();
        ImGui::Indent();
        ImGui::TextWrapped("The peak audio level on the left side of the \"Raw Wave View\" shows the current volume of the channel. "
            "This can be used to give immediate feedback on the audio input of each channel, which can help in determining if your wiring is correct. "
            "Simply mute and unmute or unplug and re-plug the device and watch for a change in this peak audio level. "
            "If your wiring is correct, changes to one channel should not affect the other.");
        ImGui::Unindent();

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
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
            HelpMarker("An input device sample rate of 44100 Hz or higher is recommended. A 48000 Hz sample rate often performs best because it will filter out high frequency noise that may be captured with higher sample rates. This sample rate can be configured in the Windows Settings app.");
        ImGui::Unindent();

        ImGui::Spacing();
        ImGui::PushFont(FontHelper::BoldFont);
        ImGui::Text("Best Practices");
        ImGui::PopFont();
        ImGui::Indent();
        ImGui::TextWrapped("- Disable the microphone boost and all sound effects and audio enhancements for your input device through the Advanced / Additional device properties in the Windows Settings app.\n"
            "- Set the input device volume to around 25 percent as a starting point.\n"
            "- Set the input device sample rate to 48 kHz to filter out high frequency noise.\n"
            "- If using a microphone, point the microphone directly at the left speaker of the DUT and position the microphone as close as possible. Position the mic close to the tweeter if there are separate speaker components.\n"
            "- Turn up the output volume of the DUT.\n"
            "- Clipping: Although audio clipping will not affect the accuracy of measurements, some onboard microphone inputs have dynamic normalization that becomes problematic with high input volumes. Use the Raw Wave View to inspect the waveform for clipping to ensure your volume level is not too high.");
        ImGui::Unindent();

        ImGui::Spacing();
        ImGui::PushFont(FontHelper::BoldFont);
        ImGui::Text("How to Fix"); ImGui::SameLine(); ImGui::TextColored((ImVec4)ImColor::HSV(0, 0.5f, 1.0f), "Crosstalk detected");
        ImGui::PopFont();
        ImGui::Indent();
        ImGui::TextWrapped(std::format("Crosstalk is most often detected when one of the two channels is receiving no audio signal at all. "
            "When no audio signal is present, a very weak crosstalk signal is detected instead. "
            "To address this:\n\n1) Make sure that your wiring is correct and that both channels are receiving audio input using the above \"Wiring and Cable Setup\" instructions.\n"
            "2) Double check your input and output device selection.\n"
            "3) Increase the signal to noise ratio by turning up the output volume of the DUT{}.\n"
            "4) Try manually increasing the Threshold by disabling the \"Automatic threshold detection\" to bring the Threshold above the crosstalk signal.\n"
            "5) If the previous three strategies do not resolve the issue, you may need to use an inline analog volume control on the line-level device to reduce its audio level and, in turn, reduce the crosstalk that it is causing.\n\n"
            "NOTE: In cases that are very rare for digital audio, the audio signal from both outputs may be very closely aligned, resulting in incorrectly detected crosstalk. When this happens, you may need to disable crosstalk detection. "
            "Before doing so, it is important to test a number of devices and become familiar with how this tool works and gain confidence in your wiring/microphone setup and audio levels.",
            tool == Tool::HdmiToDigitalAudio ? "" : " and/or positioning the microphone closer to the DUT's left speaker").c_str());
        ImGui::Unindent();

        ImGui::Spacing();
        ImGui::PushFont(FontHelper::BoldFont);
        ImGui::Text("How to Fix"); ImGui::SameLine(); ImGui::TextColored((ImVec4)ImColor::HSV(0, 0.5f, 1.0f), "Noisy / Quiet");
        ImGui::PopFont();
        ImGui::Indent();
            ImGui::TextWrapped(std::format("A Noisy / Quiet signal quality often happens when there is no audio signal present or the signal to noise ratio is too low. To address this:\n\n"
                "1) Make sure that your wiring is correct and that both channels are receiving audio input using the above \"Wiring and Cable Setup\" instructions.\n"
                "2) Double check your input and output device selection.\n"
                "3) Increase the signal to noise ratio by turning up the output volume of the DUT{}.\n"
                "4) Follow the \"Best Practices\" listed above.",
                tool == Tool::HdmiToDigitalAudio ? "" : " and/or positioning the microphone closer to the DUT's left speaker").c_str());
        ImGui::Unindent();

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Advanced Configuration"))
    {
        ImGui::Checkbox("Override \"Noisy / Quiet\" Signal Quality", overrideNoisyQuiet);
        ImGui::SameLine(); GuiHelper::HelpMarker("Generally, it is a better idea to manually increase the Threshold than to override this feature. See the \"Detailed Instructions and Troubleshooting\" section for more details.");
        ImGui::DragFloat("Output Volume", outputVolume, .001f, .1f, 1, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine(); GuiHelper::HelpMarker("Default: 0.75. There is usually no reason to change this. Increasing this may cause substantial crosstalk.");

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::TreePop();
    }
}

void GuiHelper::TestConfiguration(float DpiScale)
{
    ImGui::PushFont(FontHelper::BoldFont);
    ImGui::Text("Test Configuration");
    ImGui::PopFont();

    ImGui::PushItemWidth(75 * DpiScale);
    ImGui::DragInt("Number of Measurements", &TestConfiguration::NumMeasurements, .05f, 1, 100, "%d", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SameLine(); GuiHelper::HelpMarker("The number of measurements for each of the selected audio formats. A higher number of measurements will give a more accurate average audio latency result, but will take longer to complete.");
    if (ImGui::TreeNode("Advanced Configuration"))
    {
        ImGui::DragFloat("Lead-in Duration (seconds)", &TestConfiguration::LeadInDuration, 0.1f, 0.4f, 10.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine(); GuiHelper::HelpMarker("Increase this if the DUT takes a long time to wake up/sync to a new audio format.");

        ImGui::DragFloat("Recording Length (seconds)", &TestConfiguration::RecordingLegnth, 0.1f, 0.6f, 10.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine(); GuiHelper::HelpMarker("Increase the measurement recording length to measure higher audio latency. The default of 0.9 seconds enables latency measurements up to around 200 or 300 milliseconds, depending on input and output driver latency.");

        ImGui::Checkbox("Save Individual Recording Results", &TestConfiguration::SaveIndividualRecordingResults);
        ImGui::SameLine(); GuiHelper::HelpMarker("Saves detailed individual measurement results in a CSV file for each format that is measured. Useful for troubleshooting.");
        if (TestConfiguration::SaveIndividualRecordingResults)
        {
            ImGui::Indent(0);
            ImGui::Checkbox("Save Individual Recording WAV Files", &TestConfiguration::SaveIndividualWavFiles);
            ImGui::SameLine(); GuiHelper::HelpMarker("Saves .WAV files for each format that is measured. Use software such as Audacity to inspect these recordings. Useful for troubleshooting.\n\nNote: Software such as Audacity may not correctly display waveforms with peaks larger than +/- 1.0. Use the Raw Wave View of the \"Adjust Volumes\" step to correctly determine if clipping is occurring.");
            ImGui::Unindent();
        }

        ImGui::DragInt("Attempts Before Skipping a Format", &TestConfiguration::AttemptsBeforeFail, .05f, 1, 20, "%d", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine(); GuiHelper::HelpMarker("The number of measurement attempts for a specific format before this format is skipped altogether for the remainder of the test. Setting this number too low may cause formats to be incorrectly skipped when the DUT is simply taking time to wake up/sync to a new audio format.");

        ImGui::DragInt("Initial Ignored Time (ms)", &TestConfiguration::InitialIgnoreLength, .05f, 1, 400, "%d", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine(); GuiHelper::HelpMarker("This is the time from the start of the recording that will be ignored. This addresses interruption of measurements caused by pops and clicks at the start of playback. Default: 10 milliseconds.");

        ImGui::Checkbox("Format Switch Tone", &TestConfiguration::InsertFormatSwitch);
        ImGui::SameLine(); GuiHelper::HelpMarker("When only one sample rate is being measured per pass, this feature will insert an audio tone at a different sample rate to force the DUT to resync between passes. This is necessary to determine the range of audio latency that the DUT exhibits for a given audio format. Disabling this will likely result in an incorrectly low audio latency variance.");

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
    const char* title = "Crosstalk Detection Disabled";
    if (openDialog)
    {
        ImGui::OpenPopup(title);
    }
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Crosstalk detection is an important accuracy feature in this tool:\nDisalbing it may result in an incorrect 0 ms audio latency measurement!\n\n"
            "To address a crosstalk problem, try manually increasing the Threshold\ninstead. See the \"Detailed Instructions and Troubleshooting\" section for more details.");

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

void GuiHelper::DialogNegativeLatency(bool openDialog, ImVec2 center)
{
    const char* title = "Error: Incorrect Cable Wiring";
    if (openDialog)
    {
        ImGui::OpenPopup(title);
    }
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Calculations have resulted in a negative latency value, which is impossible.\n"
            "This suggests that your left and right input channels are wired backwards.\n\n"
            "Please go back to the \"Adjust Volumes\" step and ensure your left and right\n"
            "input channels are wired the right away around.");

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