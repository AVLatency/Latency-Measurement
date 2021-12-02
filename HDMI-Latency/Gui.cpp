#include "Gui.h"
#include "imgui.h"
#include "resource.h"
#include "AudioEndpointHelper.h"
#include "FontHelper.h"
#include "TestNotes.h"
#include "TestConfiguration.h"

float Gui::DpiScale = 1.0f;
bool Gui::DpiScaleChanged = false;
float Gui::PreviousDpiScale = 1.0f;

Gui::~Gui()
{
    if (adjustVolumeManager != nullptr)
    {
        delete adjustVolumeManager;
        adjustVolumeManager = nullptr;
    }
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
    bool openEdidReminderDialog = false;

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

    ImGui::Text("Cable Diagram:");
    ImGui::SameLine(); HelpMarker(
        "Before starting you must connect your HDMI and audio cables as described in this diagram.\n\n"
        "To record audio output from the Device Under Test (DUT) you can use a microphone or directly connect to the headphone or speaker output of the DUT.\n\n"
        "- Microphone: Make sure to position the mic as close as possible to the speaker because sound travels measurably slow. Position the mic close to the tweeter if there are separate speaker components.\n"
        "- DUT headphone output: Note that speaker and headphone output can sometimes have different latency.\n"
        "- Directly connect to DUT speaker output: Start the volume low as some amplifiers may be capable of high voltage outputs that could damage your audio input device.\n\n"
        "Your \"HDMI Audio Device\" must be capabile of analog audio output AND HDMI audio output at the same time. The time offset between analog audio output and HDMI audio output must be known. A list of capable devices can be found on the GitHub wiki.\n\n"
        "GitHub Wiki: github.com/AVLatency/Latency-Measurement/wiki");
    float cableMapScale = 0.55 * Gui::DpiScale;
    ImGui::Image((void*)resources.CableMapTexture, ImVec2(resources.CableMapTextureWidth * cableMapScale, resources.CableMapTextureHeight * cableMapScale));

    if (ImGui::BeginTable("MainViewTopLevelTable", 2, ImGuiTableFlags_Borders, ImVec2(1235 * DpiScale, 0)))
    {
        ImGui::TableSetupColumn("Col1", ImGuiTableColumnFlags_WidthFixed);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        OptionallyBoldText("1) Getting Started", state == GuiState::GettingStarted);
        ImGui::Spacing();
        OptionallyBoldText("2) Input/Output Devices", state == GuiState::SelectAudioDevices);
        ImGui::Spacing();
        OptionallyBoldText("3) Adjust Volumes", state == GuiState::AdjustVolume || state == GuiState::CancellingAdjustVolume || state == GuiState::FinishingAdjustVolume);
        ImGui::Spacing();
        OptionallyBoldText("4) Measurement Config", state == GuiState::MeasurementConfig);
        ImGui::Spacing();
        OptionallyBoldText("5) Measurement", state == GuiState::Measuring || state == GuiState::CancellingMeasuring);
        ImGui::Spacing();
        OptionallyBoldText("6) Results", state == GuiState::Results);
        ImGui::Spacing();

        ImGui::TableNextColumn();

        switch (state)
        {
        case GuiState::GettingStarted:
        {

            ImGui::Text("Welcome to the AV Latency.com HDMI latency measurement tool!");
            ImGui::Spacing();
            ImGui::Text("Before starting, please connect your cables as described in the diagram above.");
            ImGui::Spacing();
            ImGui::Text("You can find help text by hovering your mouse over these:");
            ImGui::SameLine(); HelpMarker("Click \"Next\" once you've connected all your HDMI and audio cables to get started!");
            ImGui::Spacing();

            if (ImGui::Button("Next"))
            {
                openEdidReminderDialog = true;
            }
        }
            break;
        case GuiState::SelectAudioDevices:
        case GuiState::AdjustVolume:
        case GuiState::CancellingAdjustVolume:
        case GuiState::FinishingAdjustVolume:
        {
            if (adjustVolumeManager != nullptr)
            {
                adjustVolumeManager->Tick();
            }

            bool disabled = state > GuiState::SelectAudioDevices;
            if (disabled)
            {
                ImGui::BeginDisabled();
            }

            if (ImGui::Button("Refresh Audio Devices"))
            {
                RefreshAudioEndpoints();
            }

            if (outputAudioEndpoints.size() < 1 || inputAudioEndpoints.size() < 1)
            {
                ImGui::Text("Error: cannot find an input or output audio device.");
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
                ImGui::SameLine(); HelpMarker("Select your HDMI audio output.");
                if (ImGui::BeginCombo("Input Device", inputAudioEndpoints[inputDeviceIndex].Name.c_str()))
                {
                    for (int i = 0; i < inputAudioEndpoints.size(); i++)
                    {
                        const bool is_selected = (inputDeviceIndex == i);
                        if (ImGui::Selectable(inputAudioEndpoints[i].Name.c_str(), is_selected))
                        {
                            inputDeviceIndex = i;
                        }
                        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::SameLine(); HelpMarker("You can use either a line in or mic input on your computer, but when using certain microphones you may find the mic input works better.\n\n"
                    "This input device must be configured to have at least two channels. It can be any sample rate and bit depth, but at least 48 kHz 16 bit is recommended.");

                ImGui::Spacing();
                if (ImGui::Button("Back"))
                {
                    state = GuiState::GettingStarted;
                }
                ImGui::SameLine();
                if (ImGui::Button("Adjust Volumes"))
                {
                    state = GuiState::AdjustVolume;
                    StartAjdustVolumeAudio();
                }
            }

            if (disabled)
            {
                ImGui::EndDisabled();
                disabled = false;
            }

            if (state >= GuiState::AdjustVolume)
            {
                ImGui::Spacing();
                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Input: Left Channel (HDMI Audio Device)");
                ImGui::PopFont();

                float columnWidth = 110 * DpiScale;
                ImVec2 plotDimensions(100 * DpiScale, 100 * DpiScale);

                if (ImGui::BeginTable("LeftChannelVolumeTable", 3, ImGuiTableFlags_SizingFixedFit))
                {
                    ImGui::TableSetupColumn("column1", ImGuiTableColumnFlags_WidthFixed, columnWidth);

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Reference Image");
                    if (adjustVolumeManager != nullptr && adjustVolumeManager->leftChannelTickReferenceSamples != nullptr)
                    {
                        ImGui::PlotLines("", adjustVolumeManager->leftChannelTickReferenceSamples, adjustVolumeManager->tickMonitorSamplesLength, 0, NULL, -1, 1, plotDimensions);
                    }
                    ImGui::Spacing();

                    ImGui::TableNextColumn();
                    ImGui::Text("Monitor");
                    if (adjustVolumeManager != nullptr && adjustVolumeManager->leftChannelTickMonitorSamples != nullptr)
                    {
                        ImGui::PlotLines("", adjustVolumeManager->leftChannelTickMonitorSamples, adjustVolumeManager->tickMonitorSamplesLength, 0, NULL, -1, 1, plotDimensions);
                    }
                    ImGui::Spacing();
                    //if (adjustVolumeManager != nullptr && adjustVolumeManager->lastInputBufferCopy != nullptr)
                    //{
                    //    auto buffer = adjustVolumeManager->lastInputBufferCopy;
                    //    ImGui::PlotLines("", buffer, adjustVolumeManager->input->recordingBufferLength / 2, 0, NULL, -1, 1, ImVec2(400, 100), sizeof(float) * 2);
                    //}

                    ImGui::TableNextColumn();
                    ImGui::Text("");
                    PeakLevel(adjustVolumeManager->leftChannelGrade, "Adjust the volume of your input device through the Windows control panel to make the monitor amplitude fit with some headroom to spare. "
                        "You may need to turn down the Microphone Boost in the Levels section of Additional device properties.");

                    ImGui::EndTable();
                }

                ImGui::Spacing();
                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Input: Right Channel (DUT)");
                ImGui::PopFont();
                float fullTableWidth = 600 * DpiScale; // Need to set this explictly instead of just using ImGuiTableFlags_SizingFixedFit because there is another table inside that uses ImGuiTableFlags_SizingFixedFit
                if (ImGui::BeginTable("RightChannelVolumeTable", 2, ImGuiTableFlags_None, ImVec2(fullTableWidth, 0)))
                {
                    ImGui::TableSetupColumn("column1", ImGuiTableColumnFlags_WidthFixed, columnWidth);

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Reference Image\n(Normalized)");
                    if (adjustVolumeManager != nullptr && adjustVolumeManager->rightChannelNormalizedTickReferenceSamples != nullptr)
                    {
                        ImGui::PlotLines("", adjustVolumeManager->rightChannelNormalizedTickReferenceSamples, adjustVolumeManager->tickMonitorSamplesLength, 0, NULL, -1, 1, plotDimensions);
                    }
                    ImGui::Spacing();

                    ImGui::TableNextColumn();
                    if (ImGui::BeginTable("RightChannelVolumeMontiorsTable", 3, ImGuiTableFlags_SizingFixedFit))
                    {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("Monitor\n(Normalized)");
                        if (adjustVolumeManager != nullptr && adjustVolumeManager->rightChannelNormalizedTickMonitorSamples != nullptr)
                        {
                            ImGui::PlotLines("", adjustVolumeManager->rightChannelNormalizedTickMonitorSamples, adjustVolumeManager->tickMonitorSamplesLength, 0, NULL, -1, 1, plotDimensions);
                        }
                        ImGui::Spacing();

                        ImGui::TableNextColumn();
                        ImGui::Text("Monitor\n(Raw)");
                        if (adjustVolumeManager != nullptr && adjustVolumeManager->rightChannelTickMonitorSamples != nullptr)
                        {
                            ImGui::PlotLines("", adjustVolumeManager->rightChannelTickMonitorSamples, adjustVolumeManager->tickMonitorSamplesLength, 0, NULL, -1, 1, plotDimensions);
                        }
                        ImGui::Spacing();

                        ImGui::TableNextColumn();
                        ImGui::Text("");
                        ImGui::Text("");
                        PeakLevel(adjustVolumeManager->rightChannelGrade, "Adjust the output volume of your Device Under Test (DUT) to give a consistent normalized recording.\n\n"
                            "When the DUT is muted, this peak level should be \"Quiet\". If it is not, this likely means you are getting cable crosstalk and your mesaurements will incorrectly be 0 ms audio latency!\n\n"
                            "To solve the problem of cable crosstalk, try turning down the output volume in the advanced settings or using a physical, inline volume control on your HDMI Audio Device output.");

                        ImGui::EndTable();
                    }
                    ImGui::EndTable();
                }

                if (ImGui::TreeNode("Advanced Configuration"))
                {
                    ImGui::DragFloat("Output Volume", &TestConfiguration::OutputVolume, .001f, .1f, 1);
                    ImGui::SameLine(); HelpMarker("Should normally be left at 1. If you are experiencing cable crosstalk, you can try turning this volume down or using a physical, inline volume control on your HDMI Audio Device output.");
                    ImGui::DragFloat("Detection Threshold Multiplier", &TestConfiguration::DetectionThresholdMultiplier, .001f, .0001f, 1);
                    ImGui::SameLine(); HelpMarker("Should normally be left at 1. If you are using a microphone that is extremely quiet, lowering this multiplier may help at the risk of incorrectly detecting crosstalk on the left and right audio input.");
                    ImGui::TreePop();
                }

                if (state == GuiState::AdjustVolume)
                {
                    ImGui::Spacing();
                    if (ImGui::Button("Cancel"))
                    {
                        state = GuiState::CancellingAdjustVolume;
                        if (adjustVolumeManager != nullptr)
                        {
                            adjustVolumeManager->Stop();
                        }
                    }
                    ImGui::SameLine();
                    bool disabled = adjustVolumeManager->leftChannelGrade == AdjustVolumeManager::PeakLevelGrade::Quiet
                        || adjustVolumeManager->rightChannelGrade == AdjustVolumeManager::PeakLevelGrade::Quiet;
                    if (disabled)
                    {
                        ImGui::BeginDisabled();
                    }
                    if (ImGui::Button("Finish"))
                    {
                        state = GuiState::FinishingAdjustVolume;
                        if (adjustVolumeManager != nullptr)
                        {
                            adjustVolumeManager->Stop();
                        }
                    }
                    if (disabled)
                    {
                        ImGui::EndDisabled();
                    }
                    ImGui::Spacing();
                }
            }

            if (adjustVolumeManager == nullptr ||
                (adjustVolumeManager != nullptr && !adjustVolumeManager->working))
            {
                if (state == GuiState::CancellingAdjustVolume)
                {
                    state = GuiState::SelectAudioDevices;
                }
                else if (state == GuiState::FinishingAdjustVolume)
                {
                    state = GuiState::MeasurementConfig;
                    outputAudioEndpoints[outputDeviceIndex].PopulateSupportedFormats();
                }
            }
        }
            break;
        case GuiState::MeasurementConfig:
        case GuiState::Measuring:
        case GuiState::CancellingMeasuring:
        {
            bool disabled = state > GuiState::MeasurementConfig;
            if (disabled)
            {
                ImGui::BeginDisabled();
            }

            ImGui::Spacing();
            float lastColumnCursorPosition = 0;
            if (ImGui::BeginTable("MeasurementConfig", 3))
            {
                ImGui::TableSetupColumn("column1", ImGuiTableColumnFlags_WidthFixed, 200 * DpiScale);
                ImGui::TableSetupColumn("column2", ImGuiTableColumnFlags_WidthFixed, 400 * DpiScale);

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Output Offset Profile");
                ImGui::PopFont();
                ImGui::Spacing();

                // TODO: Actually load and list the output offset profiles
                std::vector<const char*> outputOffsetProfiles;
                outputOffsetProfiles.push_back("HDV-MB01");
                outputOffsetProfiles.push_back("None");

                if (ImGui::BeginListBox("", ImVec2(-FLT_MIN, 3 * ImGui::GetTextLineHeightWithSpacing())))
                {
                    for (int n = 0; n < outputOffsetProfiles.size(); n++)
                    {
                        const bool is_selected = (outputOffsetProfileIndex == n);
                        if (ImGui::Selectable(outputOffsetProfiles[n], is_selected))
                        {
                            outputOffsetProfileIndex = n;
                        }
                        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                        if (is_selected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndListBox();
                }
                ImGui::Spacing();

                if (outputOffsetProfiles[outputOffsetProfileIndex] == "HDV-MB01")
                {
                    float imageScale = 0.45 * Gui::DpiScale;
                    ImGui::Image((void*)resources.HDV_MB01Texture, ImVec2(resources.HDV_MB01TextureWidth * imageScale, resources.HDV_MB01TextureHeight * imageScale));
                    ImGui::TextWrapped("The HDV-MB01 is also sold under these names:");
                    ImGui::Spacing();
                    ImGui::TextWrapped("- J-Tech Digital JTD18G - H5CH\n"
                        "- Monoprice Blackbird 24278\n"
                        "- OREI HDA - 912\n");
                }
                else if (outputOffsetProfiles[outputOffsetProfileIndex] == "None")
                {
                    ImGui::TextWrapped("WARNING: using an HDMI Audio Device that does not have an output offset profile may result in inaccurate measurements!");
                }

                ImGui::TableNextColumn();
                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Audio Formats (LPCM)");
                ImGui::PopFont();
                ImGui::Spacing();

                std::vector<AudioFormat>& supportedFormats = outputAudioEndpoints[outputDeviceIndex].SupportedFormats;

                if (ImGui::Button("Select Default"))
                {
                    for (AudioFormat& format : supportedFormats)
                    {
                        format.UserSelected = false;
                    }
                    outputAudioEndpoints[outputDeviceIndex].SelectDefaultFormats();
                }
                ImGui::SameLine();
                if (ImGui::Button("Select All"))
                {
                    for (AudioFormat& format : supportedFormats)
                    {
                        format.UserSelected = true;
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Select None"))
                {
                    for (AudioFormat& format : supportedFormats)
                    {
                        format.UserSelected = false;
                    }
                }

                if (ImGui::BeginChild("formatsChildWindow", ImVec2(0, 300), true, ImGuiWindowFlags_HorizontalScrollbar))
                {
                    for (AudioFormat& format : supportedFormats)
                    {
                        if (false) // TODO: base this on the output offset profile
                        {
                            ImGui::Text("[V]"); ImGui::SameLine();
                        }
                        else
                        {
                            ImGui::Text("[U]"); ImGui::SameLine();
                        }
                        ImGui::SetCursorPosX(DpiScale * 35);
                        ImGui::Checkbox(format.FormatString.c_str(), &format.UserSelected);
                        if (format.WaveFormat->nChannels == 2 && format.WaveFormat->nSamplesPerSec == 48000 && format.WaveFormat->wBitsPerSample == 16)
                        {
                            ImGui::SameLine();
                            ImGui::PushFont(FontHelper::BoldFont);
                            ImGui::Text("[Leo Bodnar]");
                            ImGui::PopFont();
                            ImGui::SameLine(); HelpMarker("This audio format is used by the Leo Bodnar Input Lag Tester. "
                                "For instructions on how to measure and calculate video latency with the Leo Bodnar tool and how to calculate lip sync error with the results from this software, visit avlatency.com.\n\n"
                                "Please note that some TVs will switch to \"PC\" video mode when connected to a computer and will not switch to \"PC\" video mode when using the Leo Bodnar tester. "
                                "Please ensure both tests are using the same video mode to accurately calculate lip sync error.");
                        }
                    }
                    ImGui::EndChild();
                }

                ImGui::Text("[V] Verified");
                ImGui::SameLine(); HelpMarker("The output offset for this audio format has been verified using a tool such as the Murideo SEVEN Generator.");
                ImGui::Text("[U] Unverified");
                ImGui::SameLine(); HelpMarker("The output offset for this audio format has not been verified using tools such as the Murideo SEVEN Generator.\n\n"
                    "Measurement results may not be accurate, depending on whether the HDMI Audio Device has a different output offset for different formats. This does not affect the consistency of results.");

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
                ImGui::Spacing();
                ImGui::Text("");
                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Test Configuration");
                ImGui::PopFont();

                ImGui::PushItemWidth(150);
                ImGui::DragInt("Number of Measurements", &TestConfiguration::NumMeasurements, .05f, 1, 100);
                ImGui::SameLine(); HelpMarker("The number of measurements for each of the selected audio formats. A higher number of measurements will give a more accurate average audio latency result.");
                if (ImGui::TreeNode("Advanced Configuration"))
                {
                    ImGui::DragInt("Attempts Before Skipping a Format", &TestConfiguration::AttemptsBeforeFail, .05f, 1, 10);
                    ImGui::SameLine(); HelpMarker("The number of measurement attempts for a specific format before this format is skipped altogether for the remainder of the test. Setting this number too low may cause formats to be incorrectly skipped when the DUT is simply taking time to wake up/sync to a new audio format.");
                    ImGui::TreePop();
                }
                ImGui::PopItemWidth();
                ImGui::Spacing();

                ImGui::TableNextColumn();
                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Test Notes");
                ImGui::PopFont();
                ImGui::Spacing();

                TestNotes::Notes.HDMIAudioDeviceUseOutputOffsetProfile = outputOffsetProfileIndex != outputOffsetProfiles.size() - 1;
                if (TestNotes::Notes.HDMIAudioDeviceUseOutputOffsetProfile)
                {
                    ImGui::BeginDisabled();
                    char tempStr[128];
                    strcpy(tempStr, outputOffsetProfiles[outputOffsetProfileIndex]);
                    ImGui::InputText("HDMI Audio Device", tempStr, IM_ARRAYSIZE(tempStr));
                    ImGui::EndDisabled();
                }
                else
                {
                    ImGui::InputText("HDMI Audio Device", TestNotes::Notes.HDMIAudioDevice, IM_ARRAYSIZE(TestNotes::Notes.HDMIAudioDevice), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)CsvInputFilter);
                }
                OtherCombo("Recording Method", "Recording Method (Other)", &TestNotes::Notes.RecordingMethodIndex, TestNotes::Notes.RecordingMethodOptions, IM_ARRAYSIZE(TestNotes::Notes.RecordingMethodOptions), TestNotes::Notes.RecordingMethodOther, IM_ARRAYSIZE(TestNotes::Notes.RecordingMethodOther));

                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Device Under Test");
                ImGui::PopFont();
                ImGui::InputText("Model", TestNotes::Notes.DutModel, IM_ARRAYSIZE(TestNotes::Notes.DutModel), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)CsvInputFilter);
                ImGui::InputText("Firmware Version", TestNotes::Notes.DutFirmwareVersion, IM_ARRAYSIZE(TestNotes::Notes.DutFirmwareVersion), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)CsvInputFilter);
                OtherCombo("Output Type", "Output Type (Other)", &TestNotes::Notes.DutOutputTypeIndex, TestNotes::Notes.DutOutputTypeOptions, IM_ARRAYSIZE(TestNotes::Notes.DutOutputTypeOptions), TestNotes::Notes.DutOutputTypeOther, IM_ARRAYSIZE(TestNotes::Notes.DutOutputTypeOther));
                ImGui::InputText("Video Mode", TestNotes::Notes.DutVideoMode, IM_ARRAYSIZE(TestNotes::Notes.DutVideoMode), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)CsvInputFilter);
                ImGui::InputText("Audio Settings", TestNotes::Notes.DutAudioSettings, IM_ARRAYSIZE(TestNotes::Notes.DutAudioSettings), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)CsvInputFilter);
                ImGui::InputText("Other Settings", TestNotes::Notes.DutOtherSettings, IM_ARRAYSIZE(TestNotes::Notes.DutOtherSettings), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)CsvInputFilter);
                ImGui::Spacing();

                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Video Signal");
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                ImGui::PopFont();
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                    ImGui::TextUnformatted("Video signal information can be found in the Windows Advanced Display Settings.");
                    ImGui::Spacing();
                    ImGui::TextUnformatted("Example:");
                    float imageScale = 0.7 * Gui::DpiScale;
                    ImGui::Image((void*)resources.WindowsDisplaySettingsTexture, ImVec2(resources.WindowsDisplaySettingsTextureWidth * imageScale, resources.WindowsDisplaySettingsTextureHeight * imageScale));
                    ImGui::PopTextWrapPos();
                    ImGui::EndTooltip();
                }
                OtherCombo("Signal Resolution", "Signal Resolution (Other)", &TestNotes::Notes.VideoResIndex, TestNotes::Notes.VideoResOptions, IM_ARRAYSIZE(TestNotes::Notes.VideoResOptions), TestNotes::Notes.VideoResolutionOther, IM_ARRAYSIZE(TestNotes::Notes.VideoResolutionOther));
                ImGui::InputText("Refresh Rate", TestNotes::Notes.VideoRefreshRate, IM_ARRAYSIZE(TestNotes::Notes.VideoRefreshRate), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)CsvInputFilter);
                ImGui::InputText("Bit Depth", TestNotes::Notes.VideoBitDepth, IM_ARRAYSIZE(TestNotes::Notes.VideoBitDepth), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)CsvInputFilter);
                ImGui::InputText("Color Format", TestNotes::Notes.VideoColorFormat, IM_ARRAYSIZE(TestNotes::Notes.VideoColorFormat), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)CsvInputFilter);
                OtherCombo("Color Space", "Color Space (Other)", &TestNotes::Notes.VideoColorSpaceIndex, TestNotes::Notes.VideoColorSpaceOptions, IM_ARRAYSIZE(TestNotes::Notes.VideoColorSpaceOptions), TestNotes::Notes.VideoColorSpaceOther, IM_ARRAYSIZE(TestNotes::Notes.VideoColorSpaceOther));
                ImGui::Spacing();

                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Additional Notes");
                ImGui::PopFont();
                ImGui::InputText("Notes 1", TestNotes::Notes.Notes1, IM_ARRAYSIZE(TestNotes::Notes.Notes1), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)CsvInputFilter);
                ImGui::InputText("Notes 2", TestNotes::Notes.Notes2, IM_ARRAYSIZE(TestNotes::Notes.Notes2), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)CsvInputFilter);
                ImGui::InputText("Notes 3", TestNotes::Notes.Notes3, IM_ARRAYSIZE(TestNotes::Notes.Notes3), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)CsvInputFilter);
                ImGui::InputText("Notes 4", TestNotes::Notes.Notes4, IM_ARRAYSIZE(TestNotes::Notes.Notes4), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)CsvInputFilter);

                lastColumnCursorPosition = ImGui::GetCursorPosX();

                ImGui::EndTable();
            }

            if (disabled)
            {
                ImGui::EndDisabled();
                disabled = false;
            }

            ImGui::Spacing();
            if (state == GuiState::MeasurementConfig)
            {
                ImGui::SetCursorPosX(lastColumnCursorPosition);
                if (ImGui::Button("Back"))
                {
                    state = GuiState::SelectAudioDevices;
                }
                ImGui::SameLine();
                if (ImGui::Button("Start Measurement"))
                {
                    state = GuiState::Measuring;
                }
            }
            else
            {
                ImGui::Text("Measurement in progres...");
                ImGui::ProgressBar(.3f);
                ImGui::Text("Currently measuring 2ch-48000Hz-16bit-PCM-0x0...");
                ImGui::ProgressBar(.57f);

                ImGui::Spacing();
                if (ImGui::Button("Stop"))
                {
                    state = GuiState::Results;
                    // TODO: state = GuiState::CancellingMeasuring;
                }
            }
            ImGui::Spacing();
        }
            break;
        case GuiState::Results:
            break;
        }

        ImGui::EndTable();
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
        ImGui::Text("AV Latency.com HDMI Latency Measurement Tool\n\nFind out more about audio/video latency, input lag, and lip sync error at avlatency.com\nFind out more about this tool at github.com/AVLatency/Latency-Measurement");
        ImGui::Separator();

        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }

        ImGui::EndPopup();
    }

    if (openEdidReminderDialog)
    {
        ImGui::OpenPopup("Reminder: EDID Mode");
    }
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Reminder: EDID Mode", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Set the EDID mode of your HDMI Audio Device to match the EDID of your DUT.");
        ImGui::Spacing();

        float imageScale = 0.6 * Gui::DpiScale;
        ImGui::Image((void*)resources.EDIDModeTexture, ImVec2(resources.EDIDModeTextureWidth * imageScale, resources.EDIDModeTextureHeight * imageScale));

        ImGui::Text("For HDMI audio extractors, set the switch to \"TV\" or \"Passthrough\".");
        ImGui::Spacing();

        // TODO: Image of switch set to the TV position

        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            RefreshAudioEndpoints();
            state = GuiState::SelectAudioDevices;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::PopFont();

    return done;
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

void Gui::OtherCombo(const char* comboName, const char* inputTextName, int* index, const char** options, int optionsLength, char* otherText, int otherTextLength)
{
    ImGui::Combo(comboName, index, options, optionsLength);
    if (*index == optionsLength - 1)
    {
        ImGui::InputText(inputTextName, otherText, otherTextLength, ImGuiInputTextFlags_CallbackCharFilter, CsvInputFilter);
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

void Gui::PeakLevel(AdjustVolumeManager::PeakLevelGrade grade, const char* helpText)
{
    ImGui::Text("Peak level:");
    ImGui::SameLine(); HelpMarker(helpText);
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

int Gui::CsvInputFilter(ImGuiInputTextCallbackData* data)
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

void Gui::RefreshAudioEndpoints()
{
    outputAudioEndpoints = AudioEndpointHelper::GetAudioEndPoints(eRender);
    inputAudioEndpoints = AudioEndpointHelper::GetAudioEndPoints(eCapture);
    outputDeviceIndex = 0;
    inputDeviceIndex = 0;
}

void Gui::StartAjdustVolumeAudio()
{
    // Save the old one if it's still in the middle of working. Otherwise, make a new one.
    if (adjustVolumeManager != nullptr && !adjustVolumeManager->working)
    {
        delete adjustVolumeManager;
        adjustVolumeManager = nullptr;
    }
    if (adjustVolumeManager == nullptr)
    {
        adjustVolumeManager = new AdjustVolumeManager(outputAudioEndpoints[outputDeviceIndex], inputAudioEndpoints[inputDeviceIndex]);
    }
}