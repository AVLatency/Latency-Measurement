#include "Gui.h"
#include "imgui.h"
#include "resource.h"
#include "AudioEndpointHelper.h"
#include "FontHelper.h"
#include "TestNotes.h"
#include "TestConfiguration.h"
#include "ResultsWriter.h"
#include "shellapi.h"
#include "OutputOffsetProfiles.h"
#include "Defines.h"
#include "GuiHelper.h"
#include "DacLatencyProfiles.h"
#include <format>

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
    bool openNegativeLatencyErrorDialog = false;
    bool openMidTestFilesystemErrorDialog = false;
    bool openNoMesaurementsErrorDialog = false;
    bool openDialogVolumeAdjustDisabledCrosstalk = false;
    if (testManager != nullptr && testManager->ShouldShowNegativeLatencyError && !testManager->HasShownNegativeLatencyError)
    {
        openNegativeLatencyErrorDialog = true;
        testManager->HasShownNegativeLatencyError = true;
    }
    if (testManager != nullptr && testManager->ShouldShowFilesystemError && !testManager->HasShownFilesystemError)
    {
        openMidTestFilesystemErrorDialog = true;
        fileSystemErrorType = FileSystemErrorType::MidTest;
        testManager->HasShownFilesystemError = true;
    }

    bool setAdjustVolumeDefaultState = false;

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
    ImGui::SameLine(); GuiHelper::HelpMarker(std::format("Before starting you must connect your audio devices and cables as described in this diagram.\n\n{}", GuiHelper::CableHelpText((OutputOffsetProfile::OutputType)outputTypeIndex)).c_str());
    float cableMapScale = 0.55 * Gui::DpiScale;
    switch (OutputOffsetProfiles::CurrentProfile()->OutType)
    {
    case OutputOffsetProfile::OutputType::Spdif:
        ImGui::Image((void*)resources.SpdifCableMapTexture.TextureData, ImVec2(resources.SpdifCableMapTexture.Width * cableMapScale, resources.SpdifCableMapTexture.Height * cableMapScale));
        break;
    case OutputOffsetProfile::OutputType::ARC:
        ImGui::Image((void*)resources.ArcCableMapTexture.TextureData, ImVec2(resources.ArcCableMapTexture.Width * cableMapScale, resources.ArcCableMapTexture.Height * cableMapScale));
        break;
    case OutputOffsetProfile::OutputType::eARC:
        ImGui::Image((void*)resources.EArcCableMapTexture.TextureData, ImVec2(resources.EArcCableMapTexture.Width * cableMapScale, resources.EArcCableMapTexture.Height * cableMapScale));
        break;
    case OutputOffsetProfile::OutputType::Analog:
        ImGui::Image((void*)resources.AnalogCableMapTexture.TextureData, ImVec2(resources.AnalogCableMapTexture.Width * cableMapScale, resources.AnalogCableMapTexture.Height * cableMapScale));
        break;
    case OutputOffsetProfile::OutputType::HdmiAudioPassthrough:
        ImGui::Image((void*)resources.HdmiAudioPassthroughCableMapTexture.TextureData, ImVec2(resources.HdmiAudioPassthroughCableMapTexture.Width * cableMapScale, resources.HdmiAudioPassthroughCableMapTexture.Height * cableMapScale));
        break;
    case OutputOffsetProfile::OutputType::Hdmi:
    default:
        ImGui::Image((void*)resources.HdmiCableMapTexture.TextureData, ImVec2(resources.HdmiCableMapTexture.Width * cableMapScale, resources.HdmiCableMapTexture.Height * cableMapScale));
        break;
    }

    if ((OutputOffsetProfile::OutputType)outputTypeIndex != OutputOffsetProfile::OutputType::None)
    {
        ImVec2 originalCursorPosition = ImGui::GetCursorPos();
        ImGui::PushFont(FontHelper::BoldFont);

        ImGui::SameLine();
        ImVec2 newCursorPosition = ImGui::GetCursorPos();

        ImGui::SetCursorPos(newCursorPosition);

        if (outputTypeIndex == (int)OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
        {
            float scale = 0.55 * Gui::DpiScale;
            ImGui::Image((void*)resources.HdmiAudioPassthroughDefinitionTexture.TextureData, ImVec2(resources.HdmiAudioPassthroughDefinitionTexture.Width * scale, resources.HdmiAudioPassthroughDefinitionTexture.Height * scale));
            ImGui::Spacing();
        }

        newCursorPosition.y += 187 * DpiScale;
        ImGui::SetCursorPos(newCursorPosition);
        ImGui::Text(std::format("Measuring {} audio latency", OutputOffsetProfile::OutputTypeName(OutputOffsetProfiles::CurrentProfile()->OutType)).c_str());

        newCursorPosition.y += ImGui::GetTextLineHeightWithSpacing();
        ImGui::SetCursorPos(newCursorPosition);
        std::string referenceDacStr = std::format(" and {}", DacLatencyProfiles::CurrentProfile()->Name);
        ImGui::Text(std::format("Using {}{}", OutputOffsetProfiles::CurrentProfile()->Name,
            (OutputOffsetProfile::OutputType)outputTypeIndex == OutputOffsetProfile::OutputType::HdmiAudioPassthrough ? referenceDacStr : "").c_str());

        ImGui::PopFont();
        ImGui::SetCursorPos(originalCursorPosition);
    }

    if (ImGui::BeginTable("MainViewTopLevelTable", 2, ImGuiTableFlags_Borders, ImVec2(1234 * DpiScale, 0)))
    {
        ImGui::TableSetupColumn("Col1", ImGuiTableColumnFlags_WidthFixed);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        GuiHelper::OptionallyBoldText("1) Getting Started", state == MeasurementToolGuiState::GettingStarted);
        ImGui::Spacing();
        GuiHelper::OptionallyBoldText("2) Input/Output Devices", state == MeasurementToolGuiState::SelectAudioDevices);
        ImGui::Spacing();
        GuiHelper::OptionallyBoldText("3) Adjust Volumes", state == MeasurementToolGuiState::AdjustVolume || state == MeasurementToolGuiState::CancellingAdjustVolume || state == MeasurementToolGuiState::FinishingAdjustVolume);
        ImGui::Spacing();
        GuiHelper::OptionallyBoldText("4) Measurement Config", state == MeasurementToolGuiState::MeasurementConfig);
        ImGui::Spacing();
        GuiHelper::OptionallyBoldText("5) Measurement", state == MeasurementToolGuiState::Measuring || state == MeasurementToolGuiState::CancellingMeasuring);
        ImGui::Spacing();
        GuiHelper::OptionallyBoldText("6) Results", state == MeasurementToolGuiState::Results);
        ImGui::Spacing();

        ImGui::TableNextColumn();

        switch (state)
        {
        case MeasurementToolGuiState::GettingStarted:
        {
            ImGui::Spacing();
            ImGui::Text("Welcome to the AV Latency.com audio latency measurement tool!");
            ImGui::Spacing();
            ImGui::Text("You can find help text by hovering your mouse over these:");
            ImGui::SameLine(); GuiHelper::HelpMarker("Visit the AV Latency.com Toolkit Webpage to view a demonstration video and find out more about how to use this tool.\n\nClick \"Next\" once you've connected all of the cables to get started!");
            ImGui::Spacing();

            if (ImGui::Button("AV Latency.com Toolkit Webpage"))
            {
                ShellExecuteA(NULL, "open", "https://avlatency.com/tools/av-latency-com-toolkit/", NULL, NULL, SW_SHOWNORMAL);
            }

            ImGui::Text("Start by selecting your audio latency type and Dual-Out Reference Device,\n"
                "then connect your cables as shown in the diagram above before continuing.");

            ImGui::Spacing();

            ImGui::SetNextItemWidth(200 * DpiScale);
            if(ImGui::BeginCombo("Audio Latency Type", OutputOffsetProfile::OutputTypeName((OutputOffsetProfile::OutputType)outputTypeIndex).c_str()))
            {
                for (int i = 0; i < (int)OutputOffsetProfile::OutputType::ENUM_LENGTH; i++)
                {
                    const bool is_selected = (outputTypeIndex == i);
                    if (ImGui::Selectable(OutputOffsetProfile::OutputTypeName((OutputOffsetProfile::OutputType)i).c_str(), is_selected))
                    {
                        outputTypeIndex = i;
                        if (outputTypeIndex != (int)OutputOffsetProfile::OutputType::None)
                        {
                            OutputOffsetProfiles::ProfilesSubset* profileSubset = OutputOffsetProfiles::Subsets[(OutputOffsetProfile::OutputType)outputTypeIndex];
                            OutputOffsetProfiles::SelectedProfileIndex = profileSubset->ProfileIndeces[profileSubset->SubsetSelectedIndex];
                        }
                    }
                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            if (outputTypeIndex != (int)OutputOffsetProfile::OutputType::None)
            {
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Dual-Out Reference Device");
                ImGui::PopFont();
                ImGui::SameLine(); GuiHelper::HelpMarker("This profile describes the time offset between the analog output and the HDMI output of the Dual-Out Reference Device for different audio formats.");
                ImGui::Spacing();

                int deviceColWidth = 300;
                int descriptionColWidth = 450;
                if (ImGui::BeginTable("DualOutRefDeviceTable", 3))
                {
                    ImGui::TableSetupColumn("column1", ImGuiTableColumnFlags_WidthFixed, deviceColWidth* DpiScale);
                    ImGui::TableSetupColumn("column2", ImGuiTableColumnFlags_WidthFixed, descriptionColWidth * DpiScale);

                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();

                    if (ImGui::BeginListBox("Dual-Out Reference Device", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
                    {
                        OutputOffsetProfiles::ProfilesSubset* profileSubset = OutputOffsetProfiles::Subsets[(OutputOffsetProfile::OutputType)outputTypeIndex];
                        for (int n = 0; n < profileSubset->ProfileIndeces.size(); n++)
                        {
                            const bool is_selected = (profileSubset->SubsetSelectedIndex == n);
                            if (ImGui::Selectable(OutputOffsetProfiles::Profiles[profileSubset->ProfileIndeces[n]]->Name.c_str(), is_selected))
                            {
                                profileSubset->SubsetSelectedIndex = n;
                                OutputOffsetProfiles::SelectedProfileIndex = profileSubset->ProfileIndeces[profileSubset->SubsetSelectedIndex];
                                if (OutputOffsetProfiles::CurrentProfile()->isNoOffset)
                                {
                                    strcpy_s(TestNotes::Notes.DaulOutRefDevice, "");
                                }
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

                    ImGui::TableNextColumn();

                    if (OutputOffsetProfiles::CurrentProfile()->isNoOffset)
                    {
                        ImGui::PushFont(FontHelper::BoldFont);
                        ImGui::Text("WARNING:");
                        ImGui::PopFont();   
                        ImGui::TextWrapped("Using a Dual-Out Reference Device that is not on the list may result in inaccurate measurements! This is because the offset between its different audio outputs will not be accounted for in the reported measurements.");
                        ImGui::Spacing();
                        ImGui::TextWrapped("If you have another device that is suitable for use with this tool, "
                            "please let me know and I might be able to add support for this device.");
                        ImGui::Spacing();
                        if (ImGui::Button("Open Contact Webpage"))
                        {
                            ShellExecuteA(NULL, "open", "https://avlatency.com/contact/", NULL, NULL, SW_SHOWNORMAL);
                        }
                    }
                    else
                    {
                        OutputOffsetProfile* currentProfile = OutputOffsetProfiles::CurrentProfile();
                        if (currentProfile->Image.TextureData != NULL)
                        {
                            float maxWidth = descriptionColWidth;
                            float maxHeight = 179;
                            float imageScale = min(maxWidth / currentProfile->Image.Width, maxHeight / currentProfile->Image.Height);
                            imageScale *= Gui::DpiScale;
                            ImGui::Image((void*)currentProfile->Image.TextureData, ImVec2(currentProfile->Image.Width * imageScale, currentProfile->Image.Height * imageScale));
                        }
                        ImGui::TextWrapped(currentProfile->Description.c_str());
                    }

                    ImGui::TableNextColumn();

                    ImGui::PushFont(FontHelper::BoldFont);
                    ImGui::Text("Output Offsets");
                    ImGui::SameLine(); GuiHelper::HelpMarker("These offsets will be accounted for in reported measurements!\n\nPositive value: analog output leads digital output\nNegative value: digital output leads analog output\n\nVerified formats will be marked as verified in the results.");
                    ImGui::PopFont();

                    ImGui::Spacing();
                    ImGui::Text("Common Formats:");
                    ImGui::Spacing();
                    for (int i = 0; i < OutputOffsetProfiles::CurrentProfile()->HighlightedVerifiedOffsetsForDisplay.size(); i++)
                    {
                        ImGui::Text(OutputOffsetProfiles::CurrentProfile()->HighlightedVerifiedOffsetsForDisplay[i].c_str());
                    }

                    ImGui::Spacing();

                    if (ImGui::TreeNode("All Verified Formats"))
                    {
                        for (int i = 0; i < OutputOffsetProfiles::CurrentProfile()->VerifiedOffsetsForDisplay.size(); i++)
                        {
                            ImGui::Text(OutputOffsetProfiles::CurrentProfile()->VerifiedOffsetsForDisplay[i].c_str());
                        }
                        ImGui::TreePop();
                    }

                    if (ImGui::TreeNode("All Unverified Formats"))
                    {
                        for (int i = 0; i < OutputOffsetProfiles::CurrentProfile()->UnverifiedOffsetsForDisplay.size(); i++)
                        {
                            ImGui::Text(OutputOffsetProfiles::CurrentProfile()->UnverifiedOffsetsForDisplay[i].c_str());
                        }
                        ImGui::TreePop();
                    }

                    ImGui::EndTable();
                }

                if (outputTypeIndex == (int)OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
                {
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::PushFont(FontHelper::BoldFont);
                    ImGui::Text("Reference DAC");
                    ImGui::PopFont();
                    ImGui::SameLine(); GuiHelper::HelpMarker("This profile describes the amount of time between the digital audio signal entering the DAC's input to the analog output of the DAC. Only DACs that have similar latency for all audio formats are compatable with this tool.");
                    ImGui::Spacing();

                    if (ImGui::BeginTable("RefDacTable", 3))
                    {
                        ImGui::TableSetupColumn("column1", ImGuiTableColumnFlags_WidthFixed, deviceColWidth * DpiScale);
                        ImGui::TableSetupColumn("column2", ImGuiTableColumnFlags_WidthFixed, descriptionColWidth * DpiScale);

                        ImGui::TableNextRow();

                        ImGui::TableNextColumn();

                        if (ImGui::BeginListBox("Reference DAC", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
                        {
                            for (int n = 0; n < DacLatencyProfiles::Profiles.size(); n++)
                            {
                                const bool is_selected = (DacLatencyProfiles::SelectedProfileIndex == n);
                                if (ImGui::Selectable(DacLatencyProfiles::Profiles[n]->Name.c_str(), is_selected))
                                {
                                    DacLatencyProfiles::SelectedProfileIndex = n;
                                    if (DacLatencyProfiles::CurrentProfile() == &DacLatencyProfiles::None)
                                    {
                                        strcpy_s(TestNotes::Notes.DAC, "");
                                    }
                                    SetDutPassthroughOutputType();
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

                        ImGui::TableNextColumn();

                        if (DacLatencyProfiles::CurrentProfile()->isNoLatency)
                        {
                            ImGui::PushFont(FontHelper::BoldFont);
                            ImGui::Text("WARNING:");
                            ImGui::PopFont();
                            ImGui::TextWrapped("Using a DAC that is not on the list may result in inaccurate measurements! This is because the DAC's audio latency will not be accounted for in the reported measurements.");
                            ImGui::Spacing();
                            ImGui::TextWrapped("If you have another device that is suitable for use with this tool, "
                                "please let me know and I might be able to add support for this device.");
                            ImGui::Spacing();
                            if (ImGui::Button("Open Contact Webpage"))
                            {
                                ShellExecuteA(NULL, "open", "https://avlatency.com/contact/", NULL, NULL, SW_SHOWNORMAL);
                            }
                        }
                        else
                        {
                            DacLatencyProfile* currentProfile = DacLatencyProfiles::CurrentProfile();
                            if (currentProfile->Image.TextureData != NULL)
                            {
                                float maxWidth = descriptionColWidth;
                                float maxHeight = 179;
                                float imageScale = min(maxWidth / currentProfile->Image.Width, maxHeight / currentProfile->Image.Height);
                                imageScale *= Gui::DpiScale;
                                ImGui::Image((void*)currentProfile->Image.TextureData, ImVec2(currentProfile->Image.Width * imageScale, currentProfile->Image.Height * imageScale));
                            }
                            ImGui::TextWrapped(currentProfile->Description.c_str());
                        }

                        ImGui::TableNextColumn();

                        ImGui::PushFont(FontHelper::BoldFont);
                        ImGui::Text("DAC Latency");
                        ImGui::SameLine(); GuiHelper::HelpMarker("This latency will be accounted for in reported measurements!\n\nOnly one latency value is used because the exact output format of the DUT may not match the format that is given to it.");
                        ImGui::PopFont();

                        ImGui::Spacing();

                        ImGui::Text(std::format("{} ms", DacLatencyProfiles::CurrentProfile()->Latency).c_str());

                        ImGui::EndTable();
                    }
                }

                if (ImGui::Button("Next"))
                {
                    if (OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::Hdmi
                        || OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
                    {
                        openEdidReminderDialog = true;
                    }
                    else
                    {
                        StartSelectAudioDevices();
                    }
                }

                GuiHelper::DeveloperOptions();

            }

            ImGui::Spacing();
        }
            break;
        case MeasurementToolGuiState::SelectAudioDevices:
        case MeasurementToolGuiState::AdjustVolume:
        case MeasurementToolGuiState::CancellingAdjustVolume:
        case MeasurementToolGuiState::FinishingAdjustVolume:
        {
            if (adjustVolumeManager != nullptr)
            {
                adjustVolumeManager->Tick();
            }

            bool disabled = state > MeasurementToolGuiState::SelectAudioDevices;
            if (disabled)
            {
                ImGui::BeginDisabled();
            }

            ImGui::Spacing();
            if (ImGui::Button("Refresh Audio Devices"))
            {
                RefreshAudioEndpoints();
            }

            bool showNextButton = false;
            if ((OutputOffsetProfiles::CurrentProfile()->isCurrentWindowsAudioFormat && defaultAudioOutputEndpoint == nullptr)
                || outputAudioEndpoints.size() < 1)
            {
                ImGui::Text("Error: cannot find an output audio device.");
            }
            else if (inputAudioEndpoints.size() < 1)
            {
                ImGui::Text("Error: cannot find an input audio device.");
            }
            else
            {
                showNextButton = true;
                if (OutputOffsetProfiles::CurrentProfile()->isCurrentWindowsAudioFormat)
                {
                    ImGui::Text(std::format("Output Device (current Windows audio format): {}", defaultAudioOutputEndpoint->Name).c_str());
                }
                else
                {
                    if (ImGui::BeginCombo("Output Device", SelectedAudioOutputEndpoint().Name.c_str()))
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
                    ImGui::SameLine();
                    if (OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::Hdmi
                        || OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::ARC
                        || OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::eARC
                        || OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
                    {
                        GuiHelper::HelpMarker("Select your HDMI audio output.");
                    }
                    else
                    {
                        GuiHelper::HelpMarker("Select your audio output.");
                    }
                }

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
                std::string helpText = std::format("Select your stereo analog input device.{}",
                        OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough
                        ? "" : " When recording with a microphone, the Mic port must be used on computers that have separate Line In and Mic ports.\n\nAt least 44.1 kHz 16 bit is recommended.");
                ImGui::SameLine(); GuiHelper::HelpMarker(helpText.c_str());
            }

            ImGui::Spacing();
            if (ImGui::Button("Back"))
            {
                state = MeasurementToolGuiState::GettingStarted;
            }
            if (showNextButton)
            {
                ImGui::SameLine();
                if (ImGui::Button("Next"))
                {
                    lastCheckedInputSampleRate = AudioEndpointHelper::GetInputMixFormatSampleRate(inputAudioEndpoints[inputDeviceIndex]);
                    state = MeasurementToolGuiState::AdjustVolume;
                    setAdjustVolumeDefaultState = true;
                    StartAdjustVolume();
                }
            }

            if (disabled)
            {
                ImGui::EndDisabled();
                disabled = false;
            }

            if (state >= MeasurementToolGuiState::AdjustVolume)
            {
                bool previousCrossTalk = TestConfiguration::Ch1CableCrosstalkDetection;
                std::string titleText = "Left Channel Input (Analog Out of Dual-Out Reference Device)";
                if (OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
                {
                    titleText = "Left Channel Input (Analog Out of Dual-Out Reference Device)";
                }
                GuiHelper::AdjustVolumeDisplay("left channel volume", adjustVolumeManager->LeftVolumeAnalysis, DpiScale, adjustVolumeManager->TargetTickMonitorSampleLength * 2, adjustVolumeManager->TargetFullMonitorSampleLength * 2, titleText.c_str(), &TestConfiguration::Ch1AutoThresholdDetection, &TestConfiguration::Ch1DetectionThreshold, &TestConfiguration::Ch1CableCrosstalkDetection, setAdjustVolumeDefaultState);
                if (!TestConfiguration::Ch1CableCrosstalkDetection && previousCrossTalk)
                {
                    openDialogVolumeAdjustDisabledCrosstalk = true;
                }

                titleText = "Right Channel Input (Output of DUT)";
                if (OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
                {
                    titleText = "Right Channel Input (Output of Reference DAC)";
                }
                previousCrossTalk = TestConfiguration::Ch2CableCrosstalkDetection;
                GuiHelper::AdjustVolumeDisplay("right channel volume", adjustVolumeManager->RightVolumeAnalysis, DpiScale, adjustVolumeManager->TargetTickMonitorSampleLength * 2, adjustVolumeManager->TargetFullMonitorSampleLength * 2, titleText.c_str(), &TestConfiguration::Ch2AutoThresholdDetection, &TestConfiguration::Ch2DetectionThreshold, &TestConfiguration::Ch2CableCrosstalkDetection, setAdjustVolumeDefaultState);
                if (!TestConfiguration::Ch2CableCrosstalkDetection && previousCrossTalk)
                {
                    openDialogVolumeAdjustDisabledCrosstalk = true;
                }

                GuiHelper::AdjustVolumeInstructionsTroubleshooting((OutputOffsetProfile::OutputType)outputTypeIndex, lastCheckedInputSampleRate, &TestConfiguration::OutputVolume, &adjustVolumeManager->OverrideNoisyQuiet, (void*)resources.VolumeAdjustExampleTexture.TextureData, resources.VolumeAdjustExampleTexture.Width, resources.VolumeAdjustExampleTexture.Height, DpiScale);
                ImGui::Spacing();
                
                if (state == MeasurementToolGuiState::AdjustVolume)
                {
                    ImGui::Spacing();
                    if (ImGui::Button("Cancel"))
                    {
                        state = MeasurementToolGuiState::CancellingAdjustVolume;
                        if (adjustVolumeManager != nullptr)
                        {
                            adjustVolumeManager->Stop();
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button(adjustVolumeManager->paused ? "Resume" : "Pause"))
                    {
                        adjustVolumeManager->TogglePause();
                    }
                    ImGui::SameLine();
                    if (adjustVolumeManager->paused)
                    {
                        ImGui::Text("(Note: Output audio device remains in use while paused.)");
                    }
                    else
                    {
                        bool disabled = adjustVolumeManager->paused
                            || (!adjustVolumeManager->OverrideNoisyQuiet && adjustVolumeManager->LeftVolumeAnalysis.Grade == AdjustVolumeManager::PeakLevelGrade::Quiet)
                            || (!adjustVolumeManager->OverrideNoisyQuiet && adjustVolumeManager->RightVolumeAnalysis.Grade == AdjustVolumeManager::PeakLevelGrade::Quiet)
                            || adjustVolumeManager->LeftVolumeAnalysis.Grade == AdjustVolumeManager::PeakLevelGrade::Crosstalk
                            || adjustVolumeManager->RightVolumeAnalysis.Grade == AdjustVolumeManager::PeakLevelGrade::Crosstalk;
                        if (disabled)
                        {
                            ImGui::BeginDisabled();
                        }
                        if (ImGui::Button("Finish"))
                        {
                            state = MeasurementToolGuiState::FinishingAdjustVolume;
                            if (adjustVolumeManager != nullptr)
                            {
                                adjustVolumeManager->Stop();
                            }
                        }
                        if (disabled)
                        {
                            ImGui::EndDisabled();
                        }
                    }
                    ImGui::Spacing();
                }
            }

            if (adjustVolumeManager == nullptr ||
                (adjustVolumeManager != nullptr && !adjustVolumeManager->working))
            {
                if (state == MeasurementToolGuiState::CancellingAdjustVolume)
                {
                    state = MeasurementToolGuiState::SelectAudioDevices;
                }
                else if (state == MeasurementToolGuiState::FinishingAdjustVolume)
                {
                    state = MeasurementToolGuiState::MeasurementConfig;
                    SelectedAudioOutputEndpoint().PopulateSupportedFormats(false, IncludeSurroundAsDefault(), true, OutputOffsetProfiles::CurrentProfile()->FormatFilter);
                    strcpy_s(TestNotes::Notes.DutModel, SelectedAudioOutputEndpoint().Name.c_str());
                    if (OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
                    {
                        SetDutPassthroughOutputType();
                    }
                }
            }
        }
            break;
        case MeasurementToolGuiState::MeasurementConfig:
        case MeasurementToolGuiState::Measuring:
        case MeasurementToolGuiState::CancellingMeasuring:
        {
            bool disabled = state > MeasurementToolGuiState::MeasurementConfig;
            if (disabled)
            {
                ImGui::BeginDisabled();
            }

            ImGui::Spacing();
            float lastColumnCursorPosition = 0;
            if (ImGui::BeginTable("MeasurementConfig", 2))
            {
                ImGui::TableSetupColumn("column1", ImGuiTableColumnFlags_WidthFixed, 370 * DpiScale);

                ImGui::TableNextRow();

                ImGui::TableNextColumn();

                if (OutputOffsetProfiles::CurrentProfile()->OutType != OutputOffsetProfile::OutputType::Analog
                    && !OutputOffsetProfiles::CurrentProfile()->isCurrentWindowsAudioFormat)
                {
                    ImGui::PushFont(FontHelper::BoldFont);
                    if (OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::Hdmi
                        || OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::ARC
                        || OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::eARC
                        || OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
                    {
                        ImGui::Text("HDMI Audio Formats (LPCM)");
                        ImGui::PopFont();
                        if (OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
                        {
                            ImGui::SameLine(); GuiHelper::HelpMarker("The audio format given to the Dual-Out Reference Device.\n\nThis is different than the audio format given by the DUT to the Reference DAC! The DUT may choose to transmit a different audio format.");
                        }
                    }
                    else
                    {
                        ImGui::Text("Windows Audio Formats (LPCM)");
                        ImGui::PopFont();
                        ImGui::SameLine(); GuiHelper::HelpMarker("This is the Windows audio format that will be used to send audio to the audio driver."
                            " The final S/PDIF format may be slightly different. For example, 16 bit audio may be sent as 20 bit audio via S/PDIF or the speaker assignment may be disregarded.");
                    }
                    ImGui::Spacing();

                    std::vector<AudioFormat>& supportedFormats = SelectedAudioOutputEndpoint().SupportedFormats;

                    if (ImGui::Button("Select Default"))
                    {
                        for (AudioFormat& format : supportedFormats)
                        {
                            format.UserSelected = false;
                        }
                        SelectedAudioOutputEndpoint().SetDefaultFormats(IncludeSurroundAsDefault(), true);
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

                    ImGui::BeginChild("formatsChildWindow", ImVec2(0, 15 * ImGui::GetTextLineHeightWithSpacing()), true, ImGuiWindowFlags_HorizontalScrollbar);
                    {
                        for (AudioFormat& format : supportedFormats)
                        {
                            ImGui::Checkbox(format.FormatString.c_str(), &format.UserSelected);
                        }
                    }
                    ImGui::EndChild();

                    GuiHelper::FormatDescriptions();

                    ImGui::Spacing();
                    ImGui::Text("");
                }
                GuiHelper::TestConfiguration(DpiScale, OutputOffsetProfiles::CurrentProfile()->OutType);

                ImGui::Spacing();

                ImGui::TableNextColumn();
                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Test Notes");
                ImGui::PopFont();
                ImGui::SameLine(); GuiHelper::HelpMarker("These notes will be included in the .csv spreadsheet result files that are saved in the folder that this app was launched from.");
                ImGui::Spacing();
                
                if (!OutputOffsetProfiles::CurrentProfile()->isNoOffset)
                {
                    ImGui::BeginDisabled();
                    strcpy_s(TestNotes::Notes.DaulOutRefDevice, OutputOffsetProfiles::CurrentProfile()->Name.c_str());
                    ImGui::InputText("Dual-Out Reference Device", TestNotes::Notes.DaulOutRefDevice, IM_ARRAYSIZE(TestNotes::Notes.DaulOutRefDevice));
                    ImGui::EndDisabled();
                }
                else
                {
                    ImGui::InputText("Dual-Out Reference Device", TestNotes::Notes.DaulOutRefDevice, IM_ARRAYSIZE(TestNotes::Notes.DaulOutRefDevice), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                }

                if (OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
                {
                    if (DacLatencyProfiles::CurrentProfile() != &DacLatencyProfiles::None)
                    {
                        ImGui::BeginDisabled();
                        strcpy_s(TestNotes::Notes.DAC, DacLatencyProfiles::CurrentProfile()->Name.c_str());
                        ImGui::InputText("Reference DAC", TestNotes::Notes.DAC, IM_ARRAYSIZE(TestNotes::Notes.DAC));
                        ImGui::EndDisabled();
                    }
                    else
                    {
                        ImGui::InputText("Reference DAC", TestNotes::Notes.DAC, IM_ARRAYSIZE(TestNotes::Notes.DAC), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                    }
                }
                else
                {
                    GuiHelper::OtherCombo("Recording Method", "Recording Method (Other)", &TestNotes::Notes.RecordingMethodIndex, TestNotes::Notes.RecordingMethodOptions, IM_ARRAYSIZE(TestNotes::Notes.RecordingMethodOptions), TestNotes::Notes.RecordingMethodOther, IM_ARRAYSIZE(TestNotes::Notes.RecordingMethodOther));
                }

                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Device Under Test");
                ImGui::PopFont();
                ImGui::InputText("Model", TestNotes::Notes.DutModel, IM_ARRAYSIZE(TestNotes::Notes.DutModel), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                ImGui::InputText("Firmware Version", TestNotes::Notes.DutFirmwareVersion, IM_ARRAYSIZE(TestNotes::Notes.DutFirmwareVersion), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                if (OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
                {
                    GuiHelper::OtherCombo("Output Type", "Output Type (Other)", &TestNotes::Notes.DutPassthroughOutputTypeIndex, TestNotes::Notes.DutPassthroughOutputTypeOptions, IM_ARRAYSIZE(TestNotes::Notes.DutPassthroughOutputTypeOptions), TestNotes::Notes.DutPassthroughOutputTypeOther, IM_ARRAYSIZE(TestNotes::Notes.DutPassthroughOutputTypeOther));
                }
                else
                {
                    GuiHelper::OtherCombo("Output Type", "Output Type (Other)", &TestNotes::Notes.DutOutputTypeIndex, TestNotes::Notes.DutOutputTypeOptions, IM_ARRAYSIZE(TestNotes::Notes.DutOutputTypeOptions), TestNotes::Notes.DutOutputTypeOther, IM_ARRAYSIZE(TestNotes::Notes.DutOutputTypeOther));
                }
                if (OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::Hdmi
                    || OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
                {
                    ImGui::InputText("Video Mode", TestNotes::Notes.DutVideoMode, IM_ARRAYSIZE(TestNotes::Notes.DutVideoMode), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                }
                ImGui::InputText("Audio Settings", TestNotes::Notes.DutAudioSettings, IM_ARRAYSIZE(TestNotes::Notes.DutAudioSettings), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                ImGui::InputText("Other Settings", TestNotes::Notes.DutOtherSettings, IM_ARRAYSIZE(TestNotes::Notes.DutOtherSettings), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                ImGui::Spacing();

                if (OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::Hdmi
                    || OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
                {
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
                        ImGui::Image((void*)resources.WindowsDisplaySettingsTexture.TextureData, ImVec2(resources.WindowsDisplaySettingsTexture.Width * imageScale, resources.WindowsDisplaySettingsTexture.Height * imageScale));
                        ImGui::PopTextWrapPos();
                        ImGui::EndTooltip();
                    }
                    GuiHelper::OtherCombo("Signal Resolution", "Signal Resolution (Other)", &TestNotes::Notes.VideoResIndex, TestNotes::Notes.VideoResOptions, IM_ARRAYSIZE(TestNotes::Notes.VideoResOptions), TestNotes::Notes.VideoResolutionOther, IM_ARRAYSIZE(TestNotes::Notes.VideoResolutionOther));
                    ImGui::InputText("Refresh Rate", TestNotes::Notes.VideoRefreshRate, IM_ARRAYSIZE(TestNotes::Notes.VideoRefreshRate), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                    ImGui::InputText("Bit Depth", TestNotes::Notes.VideoBitDepth, IM_ARRAYSIZE(TestNotes::Notes.VideoBitDepth), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                    ImGui::InputText("Color Format", TestNotes::Notes.VideoColorFormat, IM_ARRAYSIZE(TestNotes::Notes.VideoColorFormat), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                    GuiHelper::OtherCombo("Color Space", "Color Space (Other)", &TestNotes::Notes.VideoColorSpaceIndex, TestNotes::Notes.VideoColorSpaceOptions, IM_ARRAYSIZE(TestNotes::Notes.VideoColorSpaceOptions), TestNotes::Notes.VideoColorSpaceOther, IM_ARRAYSIZE(TestNotes::Notes.VideoColorSpaceOther));
                    ImGui::Spacing();
                }

                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Additional Notes");
                ImGui::PopFont();
                ImGui::InputText("Notes 1", TestNotes::Notes.Notes1, IM_ARRAYSIZE(TestNotes::Notes.Notes1), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                ImGui::InputText("Notes 2", TestNotes::Notes.Notes2, IM_ARRAYSIZE(TestNotes::Notes.Notes2), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                ImGui::InputText("Notes 3", TestNotes::Notes.Notes3, IM_ARRAYSIZE(TestNotes::Notes.Notes3), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                ImGui::InputText("Notes 4", TestNotes::Notes.Notes4, IM_ARRAYSIZE(TestNotes::Notes.Notes4), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);

                lastColumnCursorPosition = ImGui::GetCursorPosX();

                ImGui::EndTable();
            }

            if (disabled)
            {
                ImGui::EndDisabled();
                disabled = false;
            }

            ImGui::Spacing();
            if (state == MeasurementToolGuiState::MeasurementConfig)
            {
                ImGui::SetCursorPosX(lastColumnCursorPosition);
                if (ImGui::Button("Back"))
                {
                    state = MeasurementToolGuiState::SelectAudioDevices;
                }
                ImGui::SameLine();
                if (ImGui::Button("Start Measurement"))
                {
                    StartTest();
                    state = MeasurementToolGuiState::Measuring;
                }
            }
            else
            {
                if (testManager->IsFinished)
                {
                    resultFormatIndex = 0;
                    state = MeasurementToolGuiState::Results;
                    if (testManager->AveragedResults.size() == 0)
                    {
                        openNoMesaurementsErrorDialog = true;
                    }
                }
                else
                {
                    ImGui::Text("Measurement in progres...");
                    float overall = testManager->PassCount / (float)testManager->TotalPasses;
                    ImGui::ProgressBar(overall, ImVec2(-FLT_MIN, 0), std::format("Overall: {:.0f}%", overall * 100).c_str());
                    float currentPass = testManager->RecordingCount / (float)testManager->TotalRecordingsPerPass;
                    ImGui::ProgressBar(currentPass, ImVec2(-FLT_MIN, 0), std::format("Current Pass: {:.0f}%", currentPass * 100).c_str());

                    if (state != MeasurementToolGuiState::CancellingMeasuring)
                    {
                        ImGui::Spacing();
                        if(ImGui::Button("Stop"))
                        {
                            testManager->StopRequested = true;
                            state = MeasurementToolGuiState::CancellingMeasuring;
                        }
                    }
                }
            }
            ImGui::Spacing();
        }
            break;
        case MeasurementToolGuiState::Results:
        {
            std::string passthroughStr = OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough ? " Passthrough" : "";

            ImGui::PushFont(FontHelper::BoldFont);
            ImGui::Text("Measurement Results");
            ImGui::PopFont();
            ImGui::Text(testManager->TestFileString.c_str());
            if (ImGui::Button("Open Results Folder"))
            {
                ShellExecuteA(NULL, "explore", StringHelper::GetRootPath(APP_FOLDER).c_str(), NULL, NULL, SW_SHOWNORMAL);
            }
            ImGui::Spacing();

            if (ImGui::BeginTabBar("ResultsTabs", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Summary"))
                {
                    if (OutputOffsetProfiles::CurrentProfile()->isCurrentWindowsAudioFormat)
                    {
                        std::string latencyStr = "Not measured";
                        for (AveragedResult& result : testManager->SummaryResults)
                        {
                            if (result.Format == nullptr)
                            {
                                latencyStr = std::format("{} ms", round(result.AverageLatency()));
                            }
                        }

                        ImGui::PushFont(FontHelper::HeaderFont);
                        ImGui::Text(std::format("Audio{} Latency: {}", passthroughStr, latencyStr).c_str());
                        ImGui::PopFont();
                        ImGui::Text("- Current Windows audio format");
                    }
                    else
                    {
                        std::string stereoLatency = "Not measured";
                        std::string stereoFormat = "- LPCM 2ch-48kHz-16bit";
                        std::string fiveOneLatency = "Not measured";
                        std::string fiveOneFormat = "- LPCM 6ch-48kHz-16bit";
                        std::string sevenOneLatency = "Not measured";
                        std::string sevenOneFormat = "- LPCM 8ch-48kHz-16bit";

                        for (AveragedResult& result : testManager->SummaryResults)
                        {
                            if (result.Format->WaveFormat->nChannels == 2)
                            {
                                stereoLatency = std::format("{} ms", round(result.AverageLatency()));
                                stereoFormat = std::format("- LPCM {}", result.Format->FormatString);
                            }
                            if (result.Format->WaveFormat->nChannels == 6)
                            {
                                fiveOneLatency = std::format("{} ms", round(result.AverageLatency()));
                                fiveOneFormat = std::format("- LPCM {}", result.Format->FormatString);
                            }
                            if (result.Format->WaveFormat->nChannels == 8)
                            {
                                sevenOneLatency = std::format("{} ms", round(result.AverageLatency()));
                                sevenOneFormat = std::format("- LPCM {}", result.Format->FormatString);
                            }
                        }

                        ImGui::PushFont(FontHelper::HeaderFont);
                        ImGui::Text(std::format("Stereo Audio{} Latency: {}", passthroughStr, stereoLatency).c_str());
                        ImGui::PopFont();
                        ImGui::Text(stereoFormat.c_str());
                        if (IncludeSurroundAsDefault())
                        {
                            ImGui::Spacing();
                            ImGui::PushFont(FontHelper::HeaderFont);
                            ImGui::Text(std::format("5.1 Audio{} Latency: {}", passthroughStr, fiveOneLatency).c_str());
                            ImGui::PopFont();
                            ImGui::Text(fiveOneFormat.c_str());
                            ImGui::Spacing();
                            ImGui::PushFont(FontHelper::HeaderFont);
                            ImGui::Text(std::format("7.1 Audio{} Latency: {}", passthroughStr, sevenOneLatency).c_str());
                            ImGui::PopFont();
                            ImGui::Text(sevenOneFormat.c_str());
                        }
                    }

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Detailed Results"))
                {
                    ImGui::Text("Successful Formats:");
                    ImGui::BeginGroup();

                    const AudioFormat* selectedFormat = nullptr;
                    ImGui::BeginChild("", ImVec2(350 * DpiScale, 15 * ImGui::GetTextLineHeightWithSpacing()), true, ImGuiWindowFlags_HorizontalScrollbar);
                    {
                        int n = 0;
                        for (auto avgResult : testManager->AveragedResults)
                        {
                            const bool is_selected = (resultFormatIndex == n);
                            std::string formatStr = avgResult.Format == nullptr ? "Current Windows audio format" : avgResult.Format->FormatString;
                            if (ImGui::Selectable(formatStr.c_str(), is_selected))
                            {
                                resultFormatIndex = n;
                            }
                            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                            if (is_selected)
                            {
                                ImGui::SetItemDefaultFocus();
                            }

                            if (resultFormatIndex == n)
                            {
                                selectedFormat = avgResult.Format;
                            }

                            n++;
                        }
                    }
                    ImGui::EndChild();

                    ImGui::EndGroup();
                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    for (auto avgResult : testManager->AveragedResults)
                    {
                        if (avgResult.Format == selectedFormat)
                        {
                            ImGui::PushFont(FontHelper::BoldFont);
                            ImGui::Text(std::format("Average Audio{} Latency: {} ms", passthroughStr, round(avgResult.AverageLatency())).c_str());
                            ImGui::PopFont();
                            ImGui::Text(std::format("(rounded from: {} ms)", avgResult.AverageLatency()).c_str());
                            ImGui::Spacing();
                            ImGui::Text(std::format("Min Audio{} Latency: {} ms", passthroughStr, avgResult.MinLatency()).c_str());
                            ImGui::Text(std::format("Max Audio{} Latency: {} ms", passthroughStr, avgResult.MaxLatency()).c_str());
                            ImGui::Text(std::format("Valid Measurements: {}", avgResult.Offsets.size()).c_str());
                            ImGui::Spacing();
                            ImGui::Text(std::format("Output Offset Profile: {}", avgResult.OffsetProfile->Name).c_str());
                            ImGui::Text(std::format("Output Offset Value: {} ms", avgResult.OutputOffsetFromProfile).c_str());
                            ImGui::Text(std::format("Verified Accuracy: {}", avgResult.Verified ? "Yes" : "No").c_str());
                            GuiHelper::VerifiedHelp();
                            if (OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough)
                            {
                                ImGui::Text(std::format("Reference DAC: {}", avgResult.ReferenceDacName).c_str());
                                ImGui::Text(std::format("Reference DAC Audio Latency: {} ms", avgResult.ReferenceDacLatency).c_str());
                            }

                            break;
                        }
                    }
                    ImGui::EndGroup();
                    ImGui::Spacing();
                    if (ImGui::TreeNode("Failed Formats"))
                    {
                        ImGui::PushTextWrapPos(650 * DpiScale);
                        ImGui::TextWrapped(std::format("At some point during the test, {} consecutive measurements were invalid for the following formats. To prevent this from happening, try increasing the \"Lead-in Duration\" under the Advanced Configuration of the Measurement Config.", TestConfiguration::AttemptsBeforeFail).c_str());
                        ImGui::PopTextWrapPos();
                        if (ImGui::BeginListBox("", ImVec2(ImVec2(350 * DpiScale, 10 * ImGui::GetTextLineHeightWithSpacing()))))
                        {
                            for (AudioFormat* format : testManager->FailedFormats)
                            {
                                std::string formatStr = format == nullptr ? "Current Windows audio format" : format->FormatString;
                                ImGui::Text(formatStr.c_str());
                            }
                            ImGui::EndListBox();
                        }
                        ImGui::TreePop();
                    }
                    ImGui::Spacing();
                    GuiHelper::FormatDescriptions();

                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::Spacing();

            if (ImGui::Button("Back"))
            {
                delete(testManager);
                testManager = nullptr;
                state = MeasurementToolGuiState::MeasurementConfig;
            }
            ImGui::Spacing();
        }

            break;
        }

        ImGui::EndTable();
    }

    ImGui::End();

    // Menu modal dialogs:
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();

    if (ShowInitialFilesystemError || openMidTestFilesystemErrorDialog)
    {
        ImGui::OpenPopup("File System Error");
    }
    ShowInitialFilesystemError = false;
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("File System Error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Could not write to file system!");
        ImGui::Spacing();
        ImGui::Text("Measurement results are recorded in the folder that this app is launched from.\n"
            "Please re-launch this app from a location where it can write files, such as Documents, Desktop, a USB drive, etc.");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        switch (fileSystemErrorType)
        {
        case Gui::FileSystemErrorType::Initial:
            if (ImGui::Button("Ignore", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::SameLine();
            ImGui::SetItemDefaultFocus();
            if (ImGui::Button("Exit", ImVec2(120, 0))) { done = true; }
            break;
        case Gui::FileSystemErrorType::MidTest:
        default:
            ImGui::SetItemDefaultFocus();
            if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            break;
        }
        ImGui::EndPopup();
    }

    if (openAboutDialog)
    {
        ImGui::OpenPopup("About");
    }
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("About", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("AV Latency.com Audio Latency Measurement Tool\n\nFind out more about audio/video latency, input lag, and lip sync error at avlatency.com\nFind out more about this tool at github.com/AVLatency/Latency-Measurement");
        
        ImGui::Spacing();
        GuiHelper::DearImGuiLegal();
        ImGui::Spacing();

        ImGui::Separator();
        ImGui::Spacing();

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
        ImGui::Text("If you are using an HDMI audio extractor as your Dual-Out Reference\nDevice, set its EDID mode to match the EDID of your DUT.");
        ImGui::Spacing();

        float imageScale = 0.6 * Gui::DpiScale;
        ImGui::Image((void*)resources.EDIDModeTexture.TextureData, ImVec2(resources.EDIDModeTexture.Width * imageScale, resources.EDIDModeTexture.Height * imageScale));

        ImGui::Text("This EDID mode is often labelled \"TV\" on HDMI audio extractors.");
        ImGui::Spacing();

        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            StartSelectAudioDevices();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (openNoMesaurementsErrorDialog)
    {
        ImGui::OpenPopup("No Valid Measurements");
    }
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("No Valid Measurements", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Audio latency could not be measured. Please revisit the Adjust Volumes step to ensure\nthat everything is configured correctly.\n\n"
            "If you are sure that the volume levels and detection threshold are configured correctly,\nconsider using the Advanced Configuration options in the Measurement Config step.");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    GuiHelper::DialogVolumeAdjustDisabledCrosstalk(openDialogVolumeAdjustDisabledCrosstalk, center);
    GuiHelper::DialogNegativeLatency(openNegativeLatencyErrorDialog, center);

    ImGui::PopFont();

    if (done)
    {
        Finish();
    }

    return done;
}

void Gui::Finish()
{
    if (adjustVolumeManager != nullptr)
    {
        adjustVolumeManager->Stop();
        while (adjustVolumeManager->working)
        {
            adjustVolumeManager->Tick();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    if (testManager != nullptr)
    {
        testManager->StopRequested = true;
        while (!testManager->IsFinished)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

void Gui::RefreshAudioEndpoints()
{
    outputAudioEndpoints = AudioEndpointHelper::GetAudioEndPoints(eRender);
    inputAudioEndpoints = AudioEndpointHelper::GetAudioEndPoints(eCapture);
    if (defaultAudioOutputEndpoint != nullptr)
    {
        delete defaultAudioOutputEndpoint;
    }
    defaultAudioOutputEndpoint = AudioEndpointHelper::GetDefaultAudioEndPoint(eRender);
    outputDeviceIndex = 0;
    inputDeviceIndex = 0;
}

AudioEndpoint& Gui::SelectedAudioOutputEndpoint()
{
    if (OutputOffsetProfiles::CurrentProfile()->isCurrentWindowsAudioFormat)
    {
        return *defaultAudioOutputEndpoint;
    }
    else
    {
        return outputAudioEndpoints[outputDeviceIndex];
    }
}

void Gui::StartSelectAudioDevices()
{
    // Default to MeasureAverageLatency == true for all latency types except for Analog.
    // This is set in the StartSelectAudioDevices() because that's when the user has locked into the
    // OutputOffsetProfile::OutputType
    TestConfiguration::MeasureAverageLatency = OutputOffsetProfiles::CurrentProfile()->OutType != OutputOffsetProfile::OutputType::Analog;

    RefreshAudioEndpoints();
    state = MeasurementToolGuiState::SelectAudioDevices;
}

void Gui::StartAdjustVolume()
{
    // Save the old one if it's still in the middle of working. Otherwise, make a new one.
    if (adjustVolumeManager != nullptr && !adjustVolumeManager->working)
    {
        delete adjustVolumeManager;
        adjustVolumeManager = nullptr;
    }
    if (adjustVolumeManager == nullptr)
    {
        adjustVolumeManager = new AdjustVolumeManager(SelectedAudioOutputEndpoint(), inputAudioEndpoints[inputDeviceIndex], DpiScale * 270, DpiScale * 250, TestConfiguration::Ch1DetectionThreshold, TestConfiguration::Ch2DetectionThreshold);
    }
}

void Gui::StartTest()
{
    // Save the old one if it's still in the middle of working. Otherwise, make a new one.
    if (testManager != nullptr && testManager->IsFinished)
    {
        delete testManager;
        testManager = nullptr;
    }
    if (testManager == nullptr)
    {
        std::vector<AudioFormat*> selectedFormats;
        if (OutputOffsetProfiles::CurrentProfile()->isCurrentWindowsAudioFormat)
        {
            selectedFormats.push_back(nullptr);
        }
        else
        {
            for (AudioFormat& format : SelectedAudioOutputEndpoint().SupportedFormats)
            {
                if (format.UserSelected)
                {
                    selectedFormats.push_back(&format);
                }
            }
        }

        std::string fileString = StringHelper::GetFilenameSafeString(std::format("{} {}", TestNotes::Notes.DutModel, TestNotes::Notes.DutOutputType()));
        fileString = fileString.substr(0, 80); // 80 is a magic number that will keep path lengths reasonable without needing to do a lot of Windows API programming.

        
        DacLatencyProfile* currentDacProfile =
            OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough
            ? DacLatencyProfiles::CurrentProfile() : &DacLatencyProfiles::None;

        testManager = new TestManager(SelectedAudioOutputEndpoint(), inputAudioEndpoints[inputDeviceIndex], selectedFormats, fileString, APP_FOLDER, (IResultsWriter&)ResultsWriter::Writer, OutputOffsetProfiles::CurrentProfile(), currentDacProfile);
    }
}

void Gui::SetDutPassthroughOutputType()
{
    switch (DacLatencyProfiles::CurrentProfile()->InputType)
    {
    case DacLatencyProfile::DacInputType::ARC:
        TestNotes::Notes.DutPassthroughOutputTypeIndex = 1;
        break;
    case DacLatencyProfile::DacInputType::eARC:
        TestNotes::Notes.DutPassthroughOutputTypeIndex = 2;
        break;
    case DacLatencyProfile::DacInputType::SPDIF_Optical:
        TestNotes::Notes.DutPassthroughOutputTypeIndex = 3;
        break;
    case DacLatencyProfile::DacInputType::SPDIF_Coax:
        TestNotes::Notes.DutPassthroughOutputTypeIndex = 4;
        break;
    case DacLatencyProfile::DacInputType::Unknown:
    default:
        TestNotes::Notes.DutPassthroughOutputTypeIndex = 0;
        break;
    }
}

bool Gui::IncludeSurroundAsDefault()
{
    return OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::Hdmi
        || OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::ARC
        || OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::eARC
        || OutputOffsetProfiles::CurrentProfile()->OutType == OutputOffsetProfile::OutputType::HdmiAudioPassthrough;
}
