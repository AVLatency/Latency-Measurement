#include "Gui.h"
#include "imgui.h"
#include "resource.h"
#include "AudioEndpointHelper.h"
#include "FontHelper.h"
#include "shellapi.h"
#include "TestConfiguration.h"
#include "GuiHelper.h"
#include <format>
#include <algorithm>
#include <fstream> // For std::endl
#include <sstream>
#include "WasapiOutput.h"
#include "AudioGraphOutput.h"
#include "OutputAPIHelper.h"
#include "OutputOffsetProfiles.h"

float Gui::DpiScale = 1.0f;
bool Gui::DpiScaleChanged = false;
float Gui::PreviousDpiScale = 1.0f;

Gui::Gui(Resources& loadedResources) : resources(loadedResources)
{
    RefreshAudioEndpoints();
}

Gui::~Gui()
{
}

bool Gui::DoGui()
{
    bool done = false;

    ImGui::PushFont(FontHelper::RegularFont);

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(main_viewport->WorkSize.x, main_viewport->WorkSize.y), ImGuiCond_Always);
    ImGui::Begin("GUI", NULL, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar);

    bool openAboutDialog = false;

    // Menu Bar
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit", "Alt+F4"))
            {
                done = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            openAboutDialog = ImGui::MenuItem("About", "");
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
    AppDescriptionText();
    ImGui::PopTextWrapPos();
    ImGui::Spacing();

    bool disabled = state > GuiState::SelectAudioDevice;

    if (disabled)
    {
        ImGui::BeginDisabled();
    }
    if (ImGui::BeginCombo("Output API", OutputAPIHelper::OuptutTypeString(outputType).c_str()))
    {
        for (int i = 0; i < (int)OutputAPI::ENUM_LENGTH; i++)
        {
            const bool is_selected = (int)outputType == i;
            if (ImGui::Selectable(OutputAPIHelper::OuptutTypeString((OutputAPI)i).c_str(), is_selected))
            {
                outputType = (OutputAPI)i;
            }
            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    if (disabled)
    {
        ImGui::EndDisabled();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (outputType == OutputAPI::AudioGraph)
    {   
        int samplesPerSec = AudioGraphOutput::CurrentWindowsSampleRate();

        if (disabled)
        {
            ImGui::BeginDisabled();
        }

        WaveTypeSelection(samplesPerSec);

        ImGui::Spacing();
        if (state == GuiState::SelectAudioDevice && ImGui::Button("Start Output"))
        {
            state = GuiState::PlayingAudio;
            GeneratedSamples::Config config(waveType);
            if (waveType == GeneratedSamples::WaveType::TestPattern_ToneHighFreqBlip)
            {
                config.blipFrequency = blipFrequency;
                config.blipSampleLength = blipSampleLength;
            }
            else if (waveType == GeneratedSamples::WaveType::TestPattern_ToneHighFreqOnOff)
            {
                config.onOffFrequency = onOffFrequency;
            }
            currentSamples = new GeneratedSamples(samplesPerSec, config);
            output = new AudioGraphOutput(true, firstChannelOnly, currentSamples->samples, currentSamples->samplesLength);
            SetThreadExecutionState(ES_DISPLAY_REQUIRED); // Prevent display from turning off while running this tool.
            outputThread = new std::thread([this] { output->StartPlayback(); });
        }

        if (disabled)
        {
            ImGui::EndDisabled();
        }

        if (state == GuiState::PlayingAudio)
        {
            if (ImGui::Button("Stop Output"))
            {
                output->StopPlayback();
                state = GuiState::RequestedStop;
            }

            ImGui::Spacing();
            ImGui::SliderFloat("Output Volume", &TestConfiguration::OutputVolume, .001f, 1);

            ImGui::Spacing();
        }
        else if (state == GuiState::RequestedStop)
        {
            if (!output->playbackInProgress)
            {
                Finish(false);
                state = GuiState::SelectAudioDevice;
            }
        }
    }
    else if (outputType == OutputAPI::WasapiExclusive)
    {
        if (disabled)
        {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button("Refresh Audio Devices"))
        {
            RefreshAudioEndpoints();
        }

        if (outputAudioEndpoints.size() < 1)
        {
            ImGui::Text("Error: cannot find an output audio device.");
        }
        else
        {
            if (ImGui::BeginCombo("Output Device", outputAudioEndpoints[outputDeviceIndex].Name.c_str()))
            {
                for (int i = 0; i < outputAudioEndpoints.size(); i++)
                {
                    const bool is_selected = (outputDeviceIndex == i);
                    if (ImGui::Selectable(outputAudioEndpoints[i].Name.c_str(), is_selected))
                    {
                        outputDeviceIndex = i;
                    }
                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            std::stringstream copyableFormatList;
            std::vector<AudioFormat>& supportedFormats = outputAudioEndpoints[outputDeviceIndex].SupportedFormats;
            std::vector<AudioFormat>& duplicateFormats = outputAudioEndpoints[outputDeviceIndex].DuplicateSupportedFormats;
            ImGui::BeginChild("formatsChildWindow", ImVec2(0, 30 * ImGui::GetTextLineHeightWithSpacing()), true, ImGuiWindowFlags_HorizontalScrollbar);
            {
                for (AudioFormat& format : supportedFormats)
                {
                    if (ImGui::Selectable(format.FormatString.c_str(), &format.UserSelected))
                    {
                        ClearFormatSelection();
                        format.UserSelected = true;
                    }
                    copyableFormatList << format.FormatString << std::endl;
                }

                ImGui::Spacing();
                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Duplicate/Additional Formats:");
                copyableFormatList << "Duplicate/Additional Formats:" << std::endl;
                ImGui::PopFont();
                ImGui::Spacing();

                for (AudioFormat& format : duplicateFormats)
                {
                    if (ImGui::Selectable(format.FormatString.c_str(), &format.UserSelected))
                    {
                        ClearFormatSelection();
                        format.UserSelected = true;
                    }
                    copyableFormatList << format.FormatString << std::endl;
                }

            }
            ImGui::EndChild();

            GuiHelper::ChannelDescriptions();
            ImGui::Spacing();

            if (ImGui::Button("Copy format list to clipboard"))
            {
                ImGui::SetClipboardText(copyableFormatList.str().c_str());
            }

            WAVEFORMATEX* waveFormat = nullptr;
            for (AudioFormat& format : supportedFormats)
            {
                if (format.UserSelected)
                {
                    waveFormat = format.WaveFormat;
                    break;
                }
            }
            if (waveFormat == nullptr)
            {
                for (AudioFormat& format : duplicateFormats)
                {
                    if (format.UserSelected)
                    {
                        waveFormat = format.WaveFormat;
                        break;
                    }
                }
            }
            if (waveFormat != nullptr)
            {
                WaveTypeSelection(waveFormat->nSamplesPerSec);

                ImGui::Spacing();
                if (state == GuiState::SelectAudioDevice && ImGui::Button("Start Output"))
                {
                    state = GuiState::PlayingAudio;
                    GeneratedSamples::Config config(waveType);
                    if (waveType == GeneratedSamples::WaveType::TestPattern_ToneHighFreqBlip)
                    {
                        config.blipFrequency = blipFrequency;
                        config.blipSampleLength = blipSampleLength;
                    }
                    else if (waveType == GeneratedSamples::WaveType::TestPattern_ToneHighFreqOnOff)
                    {
                        config.onOffFrequency = onOffFrequency;
                    }
                    currentSamples = new GeneratedSamples(waveFormat->nSamplesPerSec, config);
                    output = new WasapiOutput(outputAudioEndpoints[outputDeviceIndex], true, firstChannelOnly, currentSamples->samples, currentSamples->samplesLength, waveFormat);
                    SetThreadExecutionState(ES_DISPLAY_REQUIRED); // Prevent display from turning off while running this tool.
                    outputThread = new std::thread([this] { output->StartPlayback(); });
                }

                if (disabled)
                {
                    ImGui::EndDisabled();
                }

                if (state == GuiState::PlayingAudio)
                {
                    if (ImGui::Button("Stop Output"))
                    {
                        output->StopPlayback();
                        state = GuiState::RequestedStop;
                    }

                    ImGui::Spacing();
                    ImGui::SliderFloat("Output Volume", &TestConfiguration::OutputVolume, .001f, 1);

                    std::string highSampleBits;
                    std::string lowSampleBits;
                    if (waveType == GeneratedSamples::WaveType::TestPattern_ToneHighFreqBlip
                        || waveType == GeneratedSamples::WaveType::TestPattern_ToneHighFreqOnOff
                        || waveType == GeneratedSamples::WaveType::TestPattern_VisuallyIdentifiable)
                    {
                        if (AudioFormat::GetFormatID(waveFormat) == WAVE_FORMAT_PCM && waveFormat->wBitsPerSample == 16)
                        {
                            highSampleBits = std::format("{:016b}", WasapiOutput::FloatToINT16(1.0f * TestConfiguration::OutputVolume));
                            lowSampleBits = std::format("{:016b}", WasapiOutput::FloatToINT16(-1.0f * TestConfiguration::OutputVolume));
                        }
                        else if (AudioFormat::GetFormatID(waveFormat) == WAVE_FORMAT_PCM && waveFormat->wBitsPerSample == 24)
                        {
                            highSampleBits = std::format("{:032b}", WasapiOutput::FloatToPaddedINT24(1.0f * TestConfiguration::OutputVolume));
                            lowSampleBits = std::format("{:032b}", WasapiOutput::FloatToPaddedINT24(-1.0f * TestConfiguration::OutputVolume));
                        }
                        else if (AudioFormat::GetFormatID(waveFormat) == WAVE_FORMAT_PCM && waveFormat->wBitsPerSample == 32)
                        {
                            highSampleBits = std::format("{:032b}", WasapiOutput::FloatToINT32(1.0f * TestConfiguration::OutputVolume));
                            lowSampleBits = std::format("{:032b}", WasapiOutput::FloatToINT32(-1.0f * TestConfiguration::OutputVolume));
                        }
                        else if (AudioFormat::GetFormatID(waveFormat) == WAVE_FORMAT_IEEE_FLOAT && waveFormat->wBitsPerSample == 32)
                        {
                            highSampleBits = std::vformat("{:032b}", std::make_format_args(1.0f * TestConfiguration::OutputVolume));
                            lowSampleBits = std::vformat("{:032b}", std::make_format_args(-1.0f * TestConfiguration::OutputVolume));
                        }

                        std::replace(lowSampleBits.begin(), lowSampleBits.end(), '-', '1');

                        ImGui::Text("High Sample: "); ImGui::SameLine(); ImGui::Text(highSampleBits.c_str());
                        ImGui::Text("Low Sample: "); ImGui::SameLine(); ImGui::Text(lowSampleBits.c_str());
                    }

                    ImGui::Spacing();

                    if (ImGui::Button("Copy format to clipboard"))
                    {
                        std::stringstream copyableFormatText;
                        copyableFormatText << AudioFormat::GetAudioDataEncodingString(waveFormat);
                        copyableFormatText << '\t' << waveFormat->nChannels
                            << '\t' << waveFormat->nSamplesPerSec
                            << '\t' << waveFormat->wBitsPerSample
                            << '\t' << TestConfiguration::OutputVolume
                            << '\t' << (firstChannelOnly ? "Left" : "All")
                            << "\t0b" << highSampleBits
                            << "\t0b" << lowSampleBits;

                        ImGui::SetClipboardText(copyableFormatText.str().c_str());
                    }

                    ImGui::Spacing();
                }
                else if (state == GuiState::RequestedStop)
                {
                    if (!output->playbackInProgress)
                    {
                        Finish(false);
                        state = GuiState::SelectAudioDevice;
                    }
                }
            }
        }
    }

    ImGui::End();

    // Menu modal dialogs:
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();

    if (openAboutDialog)
    {
        ImGui::OpenPopup("About");
    }
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("About", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("AV Latency.com Audio Generator and Driver Check");
        ImGui::Spacing();
        AppDescriptionText();
        ImGui::Spacing();
        ImGui::Text("Find out more about audio/video latency, input lag, and lip sync error at avlatency.com\nFind out more about this tool at github.com/AVLatency/Latency-Measurement");

        ImGui::Spacing();
        GuiHelper::DearImGuiLegal();
        ImGui::Spacing();

        ImGui::Separator();
        ImGui::Spacing();

        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("OK", ImVec2(120 * DpiScale, 0))) { ImGui::CloseCurrentPopup(); }

        ImGui::EndPopup();
    }

    ImGui::PopFont();

    if (done)
    {
        Finish(true);
    }

    return done;
}

void Gui::Finish(bool requestStop)
{
    if (output != nullptr)
    {
        if (requestStop)
        {
            output->StopPlayback();
        }
        if (!output->playbackInProgress)
        {
            outputThread->join();
            delete outputThread;
            outputThread = nullptr;

            delete output;
            output = nullptr;

            delete currentSamples;
            currentSamples = nullptr;

            SetThreadExecutionState(0); // Reset prevent display from turning off while running this tool.
        }
    }
}

void Gui::AppDescriptionText()
{
    ImGui::TextWrapped("This tool outputs audio test patterns using the same method as other AV Latency.com tools.");
    ImGui::Spacing();
    ImGui::TextWrapped("To accurately measure audio latency, is critically important that your audio driver correctly switches the output signal format."
        " This tool can be used to verify your audio driver's behavior and has been used during development of AV Latency.com tools to ensure that common NVIDIA, AMD, and Intel HDMI audio drivers behave correctly.");
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
void Gui::HelpMarker(const char* desc)
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

void Gui::OptionallyBoldText(const char* text, bool bold)
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

void Gui::RefreshAudioEndpoints()
{
    outputAudioEndpoints = AudioEndpointHelper::GetAudioEndPoints(eRender);
    outputDeviceIndex = 0;
    if (outputAudioEndpoints.size() > 0)
    {
        for (int i = 0; i < outputAudioEndpoints.size(); i++)
        {
            outputAudioEndpoints[i].PopulateSupportedFormats(true, true, false, false, OutputOffsetProfiles::AllFormatsFilter);
        }
    }
}

void Gui::ClearFormatSelection()
{
    for (AudioFormat& format : outputAudioEndpoints[outputDeviceIndex].SupportedFormats)
    {
        format.UserSelected = false;
    }
    for (AudioFormat& format : outputAudioEndpoints[outputDeviceIndex].DuplicateSupportedFormats)
    {
        format.UserSelected = false;
    }
}

void Gui::WaveTypeSelection(int samplesPerSec)
{
    std::string combinedTonesStr = std::format("300 Hz Tone + {} Hz Tone", samplesPerSec / 2);
    std::string blipStr = std::format("{} Hz Blip", samplesPerSec / 2);
    std::string OnOffToneStr = std::format("{} Hz Tone On/Off", samplesPerSec / 2);
    // 300 Hz is in GeneratedSamples
    const char* waveTypeComboItems[] = {
        combinedTonesStr.c_str(), // 0
        "300 Hz Tone", // 1
        "Latency Measurement Pattern", // 2
        "Latency Measurement Volume Adjustment Pattern", // 3
        blipStr.c_str(), // 4
        OnOffToneStr.c_str(), // 5
        "Visually Identifiable Pattern", // 6
        "Format Switch Tone" }; // 7
    int waveTypeComboCurrentItem = 0;

    switch (waveType)
    {
    case GeneratedSamples::WaveType::LatencyMeasurement:
        waveTypeComboCurrentItem = 2;
        break;
    case GeneratedSamples::WaveType::VolumeAdjustment:
        waveTypeComboCurrentItem = 3;
        break;
    case GeneratedSamples::WaveType::TestPattern_Tone:
        waveTypeComboCurrentItem = 1;
        break;
    case GeneratedSamples::WaveType::TestPattern_ToneHighFreqBlip:
        waveTypeComboCurrentItem = 4;
        break;
    case GeneratedSamples::WaveType::TestPattern_ToneHighFreqOnOff:
        waveTypeComboCurrentItem = 5;
        break;
    case GeneratedSamples::WaveType::TestPattern_VisuallyIdentifiable:
        waveTypeComboCurrentItem = 6;
        break;
    case GeneratedSamples::WaveType::FormatSwitch:
        waveTypeComboCurrentItem = 7;
        break;
    case GeneratedSamples::WaveType::TestPattern_TonePlusHighFreq:
    default:
        waveTypeComboCurrentItem = 0;
        break;
    }

    ImGui::Combo("Audio Pattern", &waveTypeComboCurrentItem, waveTypeComboItems, IM_ARRAYSIZE(waveTypeComboItems));

    ImGui::Checkbox("Left Channel Only", &firstChannelOnly);
    ImGui::SameLine(); HelpMarker("Outputs audio only to the left channel. In the case of a mono audio format, this will output audio to the left or center speaker.");

    switch (waveTypeComboCurrentItem)
    {
    case 2:
        waveType = GeneratedSamples::WaveType::LatencyMeasurement;
        break;
    case 3:
        waveType = GeneratedSamples::WaveType::VolumeAdjustment;
        break;
    case 1:
        waveType = GeneratedSamples::WaveType::TestPattern_Tone;
        break;
    case 4:
        waveType = GeneratedSamples::WaveType::TestPattern_ToneHighFreqBlip;
        ImGui::DragFloat("Frequency (Hz)", &blipFrequency, 1, 0.01, 1000000, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine(); HelpMarker("How often the blip will occur, not the frequency of the audio tone.");
        ImGui::InputInt("Sample Length", &blipSampleLength);
        ImGui::SameLine(); HelpMarker("The time duration of this blip will depend on sample rate.");
        break;
    case 5:
        waveType = GeneratedSamples::WaveType::TestPattern_ToneHighFreqOnOff;
        ImGui::DragFloat("On/Off Frequency (Hz)", &onOffFrequency, 1, 0.01, 1000000, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        break;
    case 6:
        waveType = GeneratedSamples::WaveType::TestPattern_VisuallyIdentifiable;
        break;
    case 7:
        waveType = GeneratedSamples::WaveType::FormatSwitch;
        break;
    case 0:
    default:
        waveType = GeneratedSamples::WaveType::TestPattern_TonePlusHighFreq;
        break;
    }
}
