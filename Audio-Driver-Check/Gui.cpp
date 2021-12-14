#include "Gui.h"
#include "imgui.h"
#include "resource.h"
#include "AudioEndpointHelper.h"
#include "FontHelper.h"
#include "shellapi.h"

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


    bool disabled = state > GuiState::SelectAudioDevice;
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

        std::vector<AudioFormat>& supportedFormats = outputAudioEndpoints[outputDeviceIndex].SupportedFormats;
        std::vector<AudioFormat>& duplicateFormats = outputAudioEndpoints[outputDeviceIndex].DuplicateSupportedFormats;
        if (ImGui::BeginChild("formatsChildWindow", ImVec2(0, 35 * ImGui::GetTextLineHeightWithSpacing()), true, ImGuiWindowFlags_HorizontalScrollbar))
        {
            for (AudioFormat& format : supportedFormats)
            {
                if (ImGui::Selectable(format.FormatString.c_str(), &format.UserSelected))
                {
                    ClearFormatSelection();
                    format.UserSelected = true;
                }
            }

            ImGui::Spacing();
            ImGui::PushFont(FontHelper::BoldFont);
            ImGui::Text("Duplicate/Additional Formats:");
            ImGui::PopFont();
            ImGui::Spacing();

            for (AudioFormat& format : duplicateFormats)
            {
                if (ImGui::Selectable(format.FormatString.c_str(), &format.UserSelected))
                {
                    ClearFormatSelection();
                    format.UserSelected = true;
                }
            }

            ImGui::EndChild();
        }

        FormatDescriptions();
        ImGui::Spacing();

        const char* waveTypeComboItems[] = {
            "Tone + High Frequency" , // 0
            "Tone", // 1
            "High Frequency Tone On/Off", // 2
            "Latency Measurement Pattern", // 3
            "Latency Measurement Volume Adjustment Pattern"}; // 4
        int waveTypeComboCurrentItem = 0;

        switch (waveType)
        {
        case GeneratedSamples::WaveType::LatencyMeasurement:
            waveTypeComboCurrentItem = 3;
            break;
        case GeneratedSamples::WaveType::VolumeAdjustment:
            waveTypeComboCurrentItem = 4;
            break;
        case GeneratedSamples::WaveType::TestPattern_Tone:
            waveTypeComboCurrentItem = 1;
            break;
        case GeneratedSamples::WaveType::TestPattern_ToneHighFreqOnOff:
            waveTypeComboCurrentItem = 2;
            break;
        case GeneratedSamples::WaveType::TestPattern_TonePlusHighFreq:
        default:
            waveTypeComboCurrentItem = 0;
            break;
        }

        ImGui::Combo("Wave Type", &waveTypeComboCurrentItem, waveTypeComboItems, IM_ARRAYSIZE(waveTypeComboItems));

        switch (waveTypeComboCurrentItem)
        {
        case 3:
            waveType = GeneratedSamples::WaveType::LatencyMeasurement;
            break;
        case 4:
            waveType = GeneratedSamples::WaveType::VolumeAdjustment;
            break;
        case 1:
            waveType = GeneratedSamples::WaveType::TestPattern_Tone;
            break;
        case 2:
            waveType = GeneratedSamples::WaveType::TestPattern_ToneHighFreqOnOff;
            break;
        case 0:
        default:
            waveType = GeneratedSamples::WaveType::TestPattern_TonePlusHighFreq;
            break;
        }

        ImGui::Checkbox("First Channel Only", &firstChannelOnly);

        ImGui::Spacing();

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
            if (ImGui::Button("Start Output"))
            {
                state = GuiState::PlayingAudio;
                currentSamples = new GeneratedSamples(waveFormat, waveType);
                output = new WasapiOutput(outputAudioEndpoints[outputDeviceIndex], true, firstChannelOnly, currentSamples->samples, currentSamples->samplesLength, waveFormat);
                SetThreadExecutionState(ES_DISPLAY_REQUIRED); // Prevent display from turning off while running this tool.
                outputThread = new std::thread([this] { output->StartPlayback(); });
            }
        }
    }
    if (disabled)
    {
        ImGui::EndDisabled();
    }

    if (state == GuiState::PlayingAudio)
    {
        if (ImGui::Button("Stop"))
        {
            output->StopPlayback();
            state = GuiState::RequestedStop;
        }
    }
    else if (state == GuiState::RequestedStop)
    {
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

            state = GuiState::SelectAudioDevice;
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
        ImGui::Text("AV Latency.com Audio Latency Check\n\n"
            "This tool will output audio of different formats that are supported by your audio driver.\n"
            "It can be used to verify that your audio driver is correctly switching audio formats.\n\n"
            "Find out more about audio/video latency, input lag, and lip sync error at avlatency.com\nFind out more about this tool at github.com/AVLatency/Latency-Measurement");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }

        ImGui::EndPopup();
    }

    ImGui::PopFont();

    if (done)
    {
        Finish();
    }

    return done;
}

void Gui::Finish()
{
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

void Gui::FormatDescriptions()
{
    if (ImGui::TreeNode("Channel descriptions"))
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
            "LFE: Low Frequency Effect (subwoofer)\n");
        ImGui::TreePop();
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
            outputAudioEndpoints[i].PopulateSupportedFormats(true, false);
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
