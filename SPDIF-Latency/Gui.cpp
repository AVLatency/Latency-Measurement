#include "Gui.h"
#include "imgui.h"
#include "resource.h"
#include "AudioEndpointHelper.h"
#include "FontHelper.h"
#include "TestNotes.h"
#include "TestConfiguration.h"
#include "ResultsWriter.h"
#include "shellapi.h"
#include "SpdifOutputOffsetProfiles.h"
#include "Defines.h"
#include "GuiHelper.h"
#include "DacLatencyProfiles.h"

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
    ImGui::SameLine(); GuiHelper::HelpMarker(std::format("Before starting you must connect your audio devices and cables as described in this diagram.\n\n{}", GuiHelper::CableHelpText(GuiHelper::Tool::SpdifAudio)).c_str());
    float cableMapScale = 0.55 * Gui::DpiScale;
    ImGui::Image((void*)resources.CableMapTexture, ImVec2(resources.CableMapTextureWidth * cableMapScale, resources.CableMapTextureHeight * cableMapScale));

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
            ImGui::Text("Welcome to the AV Latency.com S/PDIF latency measurement tool!");
            ImGui::Spacing();
            ImGui::Text("Before starting, please connect your cables as described in the diagram above.");
            ImGui::Spacing();
            ImGui::Text("You can find help text by hovering your mouse over these:");
            ImGui::SameLine(); GuiHelper::HelpMarker("You can visit the AV Latency.com Toolkit Webpage to view a demonstration video and find out more about how to use this tool.\n\nClick \"Next\" once you've connected all of the cables to get started!");
            ImGui::Spacing();

            if (ImGui::Button("AV Latency.com Toolkit Webpage"))
            {
                ShellExecuteA(NULL, "open", "https://avlatency.com/tools/av-latency-com-toolkit/", NULL, NULL, SW_SHOWNORMAL);
            }
            ImGui::SameLine();
            if (ImGui::Button("Next"))
            {
                RefreshAudioEndpoints();
                state = MeasurementToolGuiState::SelectAudioDevices;
            }

            ImGui::Spacing();

            GuiHelper::DeveloperOptions();
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

            if (outputAudioEndpoints.size() < 1)
            {
                ImGui::Text("Error: cannot find an output audio device.");
            }
            else if (inputAudioEndpoints.size() < 1)
            {
                ImGui::Text("Error: cannot find an input audio device.");
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
                ImGui::SameLine(); GuiHelper::HelpMarker("Select your audio output.");
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
                ImGui::SameLine(); GuiHelper::HelpMarker("Select your stereo analog input device. When recording with a microphone, the Mic port must be used on computers that have separate Line In and Mic ports.\n\n"
                    "At least 44.1 kHz 16 bit is recommended.");

                ImGui::Spacing();
                if (ImGui::Button("Back"))
                {
                    state = MeasurementToolGuiState::GettingStarted;
                }
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
                GuiHelper::AdjustVolumeDisplay("left channel volume", adjustVolumeManager->LeftVolumeAnalysis, DpiScale, adjustVolumeManager->TargetTickMonitorSampleLength * 2, adjustVolumeManager->TargetFullMonitorSampleLength * 2, "Left Channel Input (Analog Out of Dual-Out Reference Device)", &TestConfiguration::Ch1AutoThresholdDetection, &TestConfiguration::Ch1DetectionThreshold, &TestConfiguration::Ch1CableCrosstalkDetection, setAdjustVolumeDefaultState);
                if (!TestConfiguration::Ch1CableCrosstalkDetection && previousCrossTalk)
                {
                    openDialogVolumeAdjustDisabledCrosstalk = true;
                }

                previousCrossTalk = TestConfiguration::Ch2CableCrosstalkDetection;
                GuiHelper::AdjustVolumeDisplay("right channel volume", adjustVolumeManager->RightVolumeAnalysis, DpiScale, adjustVolumeManager->TargetTickMonitorSampleLength * 2, adjustVolumeManager->TargetFullMonitorSampleLength * 2, "Right Channel Input (Output of DUT)", &TestConfiguration::Ch2AutoThresholdDetection, &TestConfiguration::Ch2DetectionThreshold, &TestConfiguration::Ch2CableCrosstalkDetection, setAdjustVolumeDefaultState);
                if (!TestConfiguration::Ch2CableCrosstalkDetection && previousCrossTalk)
                {
                    openDialogVolumeAdjustDisabledCrosstalk = true;
                }

                GuiHelper::AdjustVolumeInstructionsTroubleshooting(GuiHelper::Tool::SpdifAudio, lastCheckedInputSampleRate, &TestConfiguration::OutputVolume, &adjustVolumeManager->OverrideNoisyQuiet, (void*)resources.VolumeAdjustExampleTexture, resources.VolumeAdjustExampleTextureWidth, resources.VolumeAdjustExampleTextureHeight, DpiScale);
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
                    outputAudioEndpoints[outputDeviceIndex].PopulateSupportedFormats(false, false, true, SpdifOutputOffsetProfiles::CurrentProfile()->FormatFilter);
                    strcpy_s(TestNotes::Notes.DutModel, outputAudioEndpoints[outputDeviceIndex].Name.c_str());
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
            if (ImGui::BeginTable("MeasurementConfig", 3))
            {
                ImGui::TableSetupColumn("column1", ImGuiTableColumnFlags_WidthFixed, 200 * DpiScale);
                ImGui::TableSetupColumn("column2", ImGuiTableColumnFlags_WidthFixed, 370 * DpiScale);

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Dual-Out Reference Device");
                ImGui::PopFont();
                ImGui::SameLine(); GuiHelper::HelpMarker("This profile describes the time offset between the analog output and the S/PDIF output of the Dual-Out Reference Device for different audio formats. It also filters out Windows audio formats that are not supported by the device.");
                ImGui::Spacing();


                if (ImGui::BeginListBox("Dual-Out Reference Device", ImVec2(-FLT_MIN, 4 * ImGui::GetTextLineHeightWithSpacing())))
                {
                    for (int n = 0; n < SpdifOutputOffsetProfiles::Profiles.size(); n++)
                    {
                        const bool is_selected = (SpdifOutputOffsetProfiles::SelectedProfileIndex == n);
                        if (ImGui::Selectable(SpdifOutputOffsetProfiles::Profiles[n]->Name.c_str(), is_selected))
                        {
                            SpdifOutputOffsetProfiles::SelectedProfileIndex = n;
                            outputAudioEndpoints[outputDeviceIndex].PopulateSupportedFormats(false, false, true, SpdifOutputOffsetProfiles::CurrentProfile()->FormatFilter);
                            if (SpdifOutputOffsetProfiles::CurrentProfile() == SpdifOutputOffsetProfiles::None)
                            {
                                strcpy_s(TestNotes::Notes.HDMIAudioDevice, "");
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

                if (SpdifOutputOffsetProfiles::Profiles[SpdifOutputOffsetProfiles::SelectedProfileIndex] == SpdifOutputOffsetProfiles::HDV_MB01)
                {
                    float imageScale = 0.45 * Gui::DpiScale;
                    ImGui::Image((void*)resources.HDV_MB01Texture, ImVec2(resources.HDV_MB01TextureWidth * imageScale, resources.HDV_MB01TextureHeight * imageScale));
                    ImGui::TextWrapped("The HDV-MB01 is sold under these names:");
                    ImGui::Spacing();
                    ImGui::TextWrapped("- J-Tech Digital JTD18G - H5CH\n"
                        "- Monoprice Blackbird 24278\n"
                        "- OREI HDA - 912\n");

                    if (ImGui::TreeNode("Supported Formats"))
                    {
                        ImGui::TextWrapped("With the right HDMI audio drivers and the right EDID information provided by the HDMI device connected to the HDV-MB01, the following formats are supported by this device:\n\n"
                            "2ch-44.1kHz-16bit\n"
                            "2ch-44.1kHz-24bit\n"
                            "2ch-48kHz-16bit\n"
                            "2ch-48kHz-24bit\n"
                            "2ch-96kHz-16bit\n"
                            "2ch-96kHz-24bit\n"
                            "2ch-192kHz-16bit\n"
                            "2ch-192kHz-24bit\n");
                        
                        ImGui::TreePop();
                    }
                }
                else if (SpdifOutputOffsetProfiles::Profiles[SpdifOutputOffsetProfiles::SelectedProfileIndex] == SpdifOutputOffsetProfiles::None)
                {
                    ImGui::PushFont(FontHelper::BoldFont);
                    ImGui::Text("WARNING:");
                    ImGui::PopFont();
                    ImGui::TextWrapped("Using a Dual-Out Reference Device that is not on this list may result in inaccurate measurements! This is because the offset between its different audio outputs will not be accounted for in the reported measurements.");
                    ImGui::Spacing();
                    ImGui::TextWrapped("If you have another device that is suitable for use with this tool, "
                        "please let me know by email to allen"/* spam bot protection */"@"/* spam bot protection */"avlatency.com and I might be able to add support for this device.");
                }

                ImGui::TableNextColumn();
                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Windows Audio Formats (LPCM)");
                ImGui::PopFont();
                ImGui::SameLine(); GuiHelper::HelpMarker("This is the Windows audio format that will be used to send audio to the audio driver."
                    " The final S/PDIF format may be slightly different. For example, 16 bit audio may be sent as 20 bit audio via S/PDIF or the speaker assignment may be disregarded.");
                ImGui::Spacing();

                std::vector<AudioFormat>& supportedFormats = outputAudioEndpoints[outputDeviceIndex].SupportedFormats;

                if (ImGui::Button("Select Default"))
                {
                    for (AudioFormat& format : supportedFormats)
                    {
                        format.UserSelected = false;
                    }
                    outputAudioEndpoints[outputDeviceIndex].SetDefaultFormats(false, true);
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

                GuiHelper::TestConfiguration(DpiScale);

                ImGui::PopItemWidth();
                ImGui::Spacing();

                ImGui::TableNextColumn();
                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Test Notes");
                ImGui::PopFont();
                ImGui::SameLine(); GuiHelper::HelpMarker("These notes will be included in the .csv spreadsheet result files that are saved in the folder that this app was launched from.");
                ImGui::Spacing();

                TestNotes::Notes.HDMIAudioDeviceUseOutputOffsetProfile = SpdifOutputOffsetProfiles::CurrentProfile() != SpdifOutputOffsetProfiles::None;
                if (TestNotes::Notes.HDMIAudioDeviceUseOutputOffsetProfile)
                {
                    ImGui::BeginDisabled();
                    strcpy_s(TestNotes::Notes.HDMIAudioDevice, SpdifOutputOffsetProfiles::CurrentProfile()->Name.c_str());
                    ImGui::InputText("Dual-Out Reference Device", TestNotes::Notes.HDMIAudioDevice, IM_ARRAYSIZE(TestNotes::Notes.HDMIAudioDevice));
                    ImGui::EndDisabled();
                }
                else
                {
                    ImGui::InputText("Dual-Out Reference Device", TestNotes::Notes.HDMIAudioDevice, IM_ARRAYSIZE(TestNotes::Notes.HDMIAudioDevice), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                }
                GuiHelper::OtherCombo("Recording Method", "Recording Method (Other)", &TestNotes::Notes.RecordingMethodIndex, TestNotes::Notes.RecordingMethodOptions, IM_ARRAYSIZE(TestNotes::Notes.RecordingMethodOptions), TestNotes::Notes.RecordingMethodOther, IM_ARRAYSIZE(TestNotes::Notes.RecordingMethodOther));

                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Device Under Test");
                ImGui::PopFont();
                ImGui::InputText("Model", TestNotes::Notes.DutModel, IM_ARRAYSIZE(TestNotes::Notes.DutModel), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                ImGui::InputText("Firmware Version", TestNotes::Notes.DutFirmwareVersion, IM_ARRAYSIZE(TestNotes::Notes.DutFirmwareVersion), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                GuiHelper::OtherCombo("Output Type", "Output Type (Other)", &TestNotes::Notes.DutOutputTypeIndex, TestNotes::Notes.DutOutputTypeOptions, IM_ARRAYSIZE(TestNotes::Notes.DutOutputTypeOptions), TestNotes::Notes.DutOutputTypeOther, IM_ARRAYSIZE(TestNotes::Notes.DutOutputTypeOther));
                ImGui::InputText("Audio Settings", TestNotes::Notes.DutAudioSettings, IM_ARRAYSIZE(TestNotes::Notes.DutAudioSettings), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                ImGui::InputText("Other Settings", TestNotes::Notes.DutOtherSettings, IM_ARRAYSIZE(TestNotes::Notes.DutOtherSettings), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                ImGui::Spacing();

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
                    std::string stereoLatency = "Not measured";
                    std::string stereoFormat = "- LPCM 2ch-48kHz-16bit";

                    for (AveragedResult& result : testManager->SummaryResults)
                    {
                        if (result.Format->WaveFormat->nChannels == 2)
                        {
                            stereoLatency = std::format("{} ms", round(result.AverageLatency()));
                            stereoFormat = std::format("- LPCM {}", result.Format->FormatString);
                        }
                    }

                    ImGui::PushFont(FontHelper::HeaderFont);
                    ImGui::Text(std::format("Stereo Audio Latency: {}", stereoLatency).c_str());
                    ImGui::PopFont();
                    ImGui::Text(stereoFormat.c_str());

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
                            if (ImGui::Selectable(avgResult.Format->FormatString.c_str(), is_selected))
                            {
                                resultFormatIndex = n;
                            }
                            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                            if (is_selected)
                            {
                                ImGui::SetItemDefaultFocus();
                            }

                            const AudioFormat* format = avgResult.Format;

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
                            ImGui::Text(std::format("Average Audio Latency: {} ms", round(avgResult.AverageLatency())).c_str());
                            ImGui::PopFont();
                            ImGui::Text(std::format("(rounded from: {} ms)", avgResult.AverageLatency()).c_str());
                            ImGui::Spacing();
                            ImGui::Text(std::format("Min Audio Latency: {} ms", avgResult.MinLatency()).c_str());
                            ImGui::Text(std::format("Max Audio Latency: {} ms", avgResult.MaxLatency()).c_str());
                            ImGui::Text(std::format("Valid Measurements: {}", avgResult.Offsets.size()).c_str());
                            ImGui::Spacing();
                            ImGui::Text(std::format("Output Offset Profile: {}", avgResult.OutputOffsetProfileName).c_str());
                            ImGui::Text(std::format("Output Offset Value: {} ms", avgResult.OutputOffsetFromProfile).c_str());
                            ImGui::Text(std::format("Verified Accuracy: {}", avgResult.Verified ? "Yes" : "No").c_str());
                            GuiHelper::VerifiedHelp();

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
                                ImGui::Text(format->FormatString.c_str());
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
        ImGui::Text("AV Latency.com S/PDIF Latency Measurement Tool\n\nFind out more about audio/video latency, input lag, and lip sync error at avlatency.com\nFind out more about this tool at github.com/AVLatency/Latency-Measurement");
        
        ImGui::Spacing();
        GuiHelper::DearImGuiLegal();
        ImGui::Spacing();

        ImGui::Separator();
        ImGui::Spacing();

        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }

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
    outputDeviceIndex = 0;
    inputDeviceIndex = 0;
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
        adjustVolumeManager = new AdjustVolumeManager(outputAudioEndpoints[outputDeviceIndex], inputAudioEndpoints[inputDeviceIndex], DpiScale * 270, DpiScale * 250, TestConfiguration::Ch1DetectionThreshold, TestConfiguration::Ch2DetectionThreshold);
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
        for (AudioFormat& format : outputAudioEndpoints[outputDeviceIndex].SupportedFormats)
        {
            if (format.UserSelected)
            {
                selectedFormats.push_back(&format);
            }
        }

        std::string fileString = StringHelper::GetFilenameSafeString(std::format("{} {}", TestNotes::Notes.DutModel, TestNotes::Notes.DutOutputType()));
        fileString = fileString.substr(0, 80); // 80 is a magic number that will keep path lengths reasonable without needing to do a lot of Windows API programming.

        testManager = new TestManager(outputAudioEndpoints[outputDeviceIndex], inputAudioEndpoints[inputDeviceIndex], selectedFormats, fileString, APP_FOLDER, (IResultsWriter&)ResultsWriter::Writer, SpdifOutputOffsetProfiles::CurrentProfile(), &DacLatencyProfiles::None);
    }
}
