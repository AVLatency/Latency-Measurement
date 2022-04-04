#include "Gui.h"
#include "imgui.h"
#include "resource.h"
#include "AudioEndpointHelper.h"
#include "FontHelper.h"
#include "TestNotes.h"
#include "TestConfiguration.h"
#include "ResultsWriter.h"
#include "shellapi.h"
#include "HdmiOutputOffsetProfiles.h"
#include "DacLatencyProfiles.h"
#include "Defines.h"
#include "GuiHelper.h"

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
    bool openMidTestFilesystemErrorDialog = false;
    bool openNoMesaurementsErrorDialog = false;
    bool openDialogVolumeAdjustDisabledCrosstalk = false;
    if (testManager != nullptr && testManager->ShouldShowFilesystemError && !testManager->HasShownFilesystemError)
    {
        openMidTestFilesystemErrorDialog = true;
        fileSystemErrorType = FileSystemErrorType::MidTest;
        testManager->HasShownFilesystemError = true;
    }

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
    ImGui::SameLine(); GuiHelper::HelpMarker(
        "Before starting you must connect your audio devices and cables as described in this diagram.\n\n"
        "For your HDMI Audio Extractor, the time offset between analog audio output and HDMI audio output must be known. For your ARC, eARC, or S/PDIF DAC, the digital to analog latency must be known. A list of capable devices can be found on the GitHub wiki.\n\n"
        "GitHub Wiki: github.com/AVLatency/Latency-Measurement/wiki");
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
            float scale = 0.55 * Gui::DpiScale;
            ImGui::Image((void*)resources.HdmiToDigitalAudioDefinitionTexture , ImVec2(resources.HdmiToDigitalAudioDefinitionTextureWidth * scale, resources.HdmiToDigitalAudioDefinitionTextureHeight * scale));

            ImGui::Text("Welcome to the AV Latency.com HDMI to ARC, eARC, or S/PDIF Latency measurement tool!");
            ImGui::Spacing();
            ImGui::Text("Before starting, please connect your cables as described in the diagram above.");
            ImGui::Spacing();
            ImGui::Text("You can find help text by hovering your mouse over these:");
            ImGui::SameLine(); GuiHelper::HelpMarker("Click \"Next\" once you've connected all of the cables to get started!");
            ImGui::Spacing();

#ifdef _DEBUG
            // TODO: remove this once I've settled on an amplitude I'm happy with. This doesn't actually adjust anything in real-time because the audio tone is set to repeat.
            ImGui::DragFloat("Lead-In Tone Amplitude", &TestConfiguration::LeadInToneAmplitude, 0.001, 0.002, 1, "%.4f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
            ImGui::SameLine(); GuiHelper::HelpMarker("Default: 0.07. This lead-in tone is used to stop dynamic normalization that exists in some onboard microphone inputs. If your input audio device does not have any dynamic normalization (such as a professional audio interface), you can turn this down to make the audio patterns less annoying to listen to.");

            ImGui::Checkbox("Low Freqency Pitch", &TestConfiguration::LowFreqPitch);
#endif

            if (ImGui::Button("Next"))
            {
                openEdidReminderDialog = true;
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
                ImGui::SameLine(); GuiHelper::HelpMarker("Select your HDMI audio output.");
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
                ImGui::SameLine(); GuiHelper::HelpMarker("You can use either a line in or mic input on your computer.\n\n"
                    "This input device must be configured to have at least two channels. At least 48 kHz 16 bit is recommended for this tool to work well.");

                ImGui::Spacing();
                if (ImGui::Button("Back"))
                {
                    state = MeasurementToolGuiState::GettingStarted;
                }
                ImGui::SameLine();
                if (ImGui::Button("Adjust Volumes"))
                {
                    lastCheckedInputSampleRate = AudioEndpointHelper::GetInputMixFormatSampleRate(inputAudioEndpoints[inputDeviceIndex]);
                    state = MeasurementToolGuiState::AdjustVolume;
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
                GuiHelper::AdjustVolumeDisplay("left channel volume", adjustVolumeManager->LeftVolumeAnalysis, DpiScale, adjustVolumeManager->TargetTickMonitorSampleLength * 2, adjustVolumeManager->TargetFullMonitorSampleLength * 2, "Left Channel Input (HDMI Audio Extractor)", &TestConfiguration::Ch1AutoThresholdDetection, &TestConfiguration::Ch1DetectionThreshold, &TestConfiguration::Ch1CableCrosstalkDetection);
                if (!TestConfiguration::Ch1CableCrosstalkDetection && previousCrossTalk)
                {
                    openDialogVolumeAdjustDisabledCrosstalk = true;
                }
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();

                previousCrossTalk = TestConfiguration::Ch2CableCrosstalkDetection;
                GuiHelper::AdjustVolumeDisplay("right channel volume", adjustVolumeManager->RightVolumeAnalysis, DpiScale, adjustVolumeManager->TargetTickMonitorSampleLength * 2, adjustVolumeManager->TargetFullMonitorSampleLength * 2, "Right Channel Input (DUT)", &TestConfiguration::Ch2AutoThresholdDetection, &TestConfiguration::Ch2DetectionThreshold, &TestConfiguration::Ch2CableCrosstalkDetection);
                if (!TestConfiguration::Ch2CableCrosstalkDetection && previousCrossTalk)
                {
                    openDialogVolumeAdjustDisabledCrosstalk = true;
                }
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();

                GuiHelper::AdjustVolumeInstructionsTroubleshooting(GuiHelper::Tool::HdmiToDigitalAudio, lastCheckedInputSampleRate, &TestConfiguration::OutputVolume, &adjustVolumeManager->OverrideNoisyQuiet, (void*)resources.VolumeAdjustExampleTexture, resources.VolumeAdjustExampleTextureWidth, resources.VolumeAdjustExampleTextureHeight, DpiScale);
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
                    bool disabled = (!adjustVolumeManager->OverrideNoisyQuiet && adjustVolumeManager->LeftVolumeAnalysis.Grade == AdjustVolumeManager::PeakLevelGrade::Quiet)
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
                    outputAudioEndpoints[outputDeviceIndex].PopulateSupportedFormats(false, true, true, HdmiOutputOffsetProfiles::CurrentProfile()->FormatFilter);
                    strcpy_s(TestNotes::Notes.DutModel, outputAudioEndpoints[outputDeviceIndex].Name.c_str());
                    SetDutOutputType();
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
                ImGui::Text("HDMI Audio Extractor");
                ImGui::PopFont();
                ImGui::SameLine(); GuiHelper::HelpMarker("This profile describes the time offset between the analog output and the HDMI output of the HDMI Audio Extractor for different audio formats.");
                ImGui::Spacing();

                if (ImGui::BeginListBox("Audio Extractor", ImVec2(-FLT_MIN, 3 * ImGui::GetTextLineHeightWithSpacing())))
                {
                    for (int n = 0; n < HdmiOutputOffsetProfiles::Profiles.size(); n++)
                    {
                        const bool is_selected = (HdmiOutputOffsetProfiles::SelectedProfileIndex == n);
                        if (ImGui::Selectable(HdmiOutputOffsetProfiles::Profiles[n]->Name.c_str(), is_selected))
                        {
                            HdmiOutputOffsetProfiles::SelectedProfileIndex = n;
                            outputAudioEndpoints[outputDeviceIndex].PopulateSupportedFormats(false, true, true, HdmiOutputOffsetProfiles::CurrentProfile()->FormatFilter);
                            if (HdmiOutputOffsetProfiles::CurrentProfile() == HdmiOutputOffsetProfiles::None)
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

                if (HdmiOutputOffsetProfiles::Profiles[HdmiOutputOffsetProfiles::SelectedProfileIndex] == HdmiOutputOffsetProfiles::HDV_MB01)
                {
                    float imageScale = 0.45 * Gui::DpiScale;
                    ImGui::Image((void*)resources.HDV_MB01Texture, ImVec2(resources.HDV_MB01TextureWidth * imageScale, resources.HDV_MB01TextureHeight * imageScale));
                    ImGui::TextWrapped("The HDV-MB01 is sold under these names:");
                    ImGui::Spacing();
                    ImGui::TextWrapped("- J-Tech Digital JTD18G - H5CH\n"
                        "- Monoprice Blackbird 24278\n"
                        "- OREI HDA - 912\n");
                }
                else if (HdmiOutputOffsetProfiles::Profiles[HdmiOutputOffsetProfiles::SelectedProfileIndex] == HdmiOutputOffsetProfiles::None)
                {
                    ImGui::PushFont(FontHelper::BoldFont);
                    ImGui::Text("WARNING:");
                    ImGui::PopFont();
                    ImGui::TextWrapped("Using an HDMI Audio Extractor that is not on this list may result in inaccurate measurements! This is because the offset between its different audio outputs will not be accounted for in the reported measurements.");
                    ImGui::Spacing();
                    ImGui::TextWrapped("If you have another HDMI Audio Extractor that is suitable for use with this tool, "
                        "please let me know by email to allen"/* spam bot protection */"@"/* spam bot protection */"avlatency.com and I might be able to add support for this device.");
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("ARC, eARC, or S/PDIF DAC");
                ImGui::PopFont();
                ImGui::SameLine(); GuiHelper::HelpMarker("This profile describes the amount of time between the digital audio signal entering the DAC's input to the analog output of the DAC. Only DACs that have similar latency for all audio formats are compatable with this tool.");
                ImGui::Spacing();

                if (ImGui::BeginListBox("DAC", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
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
                            SetDutOutputType();
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

                if (DacLatencyProfiles::CurrentProfile() == &DacLatencyProfiles::CV121AD_ARC
                    || DacLatencyProfiles::CurrentProfile() == &DacLatencyProfiles::CV121AD_SPDIF_COAX
                    || DacLatencyProfiles::CurrentProfile() == &DacLatencyProfiles::CV121AD_SPDIF_OPTICAL)
                {
                    float imageScale = 0.30 * Gui::DpiScale;
                    ImGui::Image((void*)resources.CV121ADTexture, ImVec2(resources.CV121ADTextureWidth * imageScale, resources.CV121ADTextureHeight * imageScale));
                    ImGui::TextWrapped("The CV121AD is sold under these names:");
                    ImGui::Spacing();
                    ImGui::TextWrapped("- MYPIN 192KHz DAC Converter Multifunction Audio Converter");
                }
                else if (DacLatencyProfiles::CurrentProfile() == &DacLatencyProfiles::SHARCV1_EARC)
                {
                    float imageScale = 0.6 * Gui::DpiScale;
                    ImGui::Image((void*)resources.SHARCv1Texture, ImVec2(resources.SHARCv1TextureWidth * imageScale, resources.SHARCv1TextureHeight * imageScale));
                    ImGui::TextWrapped("The SHARC v1 is produced and sold by Thenaudio.");
                }
                else if (DacLatencyProfiles::CurrentProfile() == &DacLatencyProfiles::None)
                {
                    ImGui::PushFont(FontHelper::BoldFont);
                    ImGui::Text("WARNING:");
                    ImGui::PopFont();
                    ImGui::TextWrapped("Using a DAC that is not on this list may result in inaccurate measurements! This is because the DAC's audio latency will not be accounted for in the reported measurements.");
                    ImGui::Spacing();
                    ImGui::TextWrapped("If you have another DAC that is suitable for use with this tool, "
                        "please let me know by email to allen"/* spam bot protection */"@"/* spam bot protection */"avlatency.com and I might be able to add support for this device.");
                }

                ImGui::TableNextColumn();
                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("HDMI Audio Formats (LPCM)");
                ImGui::PopFont();
                ImGui::SameLine(); GuiHelper::HelpMarker("The audio format given to the HDMI Audio Extractor.\n\nThis is different than the audio format given by the DUT to the ARC, eARC, or S/PDIF DAC! The DUT may choose to transmit a different audio format.");
                ImGui::Spacing();

                std::vector<AudioFormat>& supportedFormats = outputAudioEndpoints[outputDeviceIndex].SupportedFormats;

                if (ImGui::Button("Select Default"))
                {
                    for (AudioFormat& format : supportedFormats)
                    {
                        format.UserSelected = false;
                    }
                    outputAudioEndpoints[outputDeviceIndex].SetDefaultFormats(true, true);
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

                TestNotes::Notes.HDMIAudioDeviceUseOutputOffsetProfile = HdmiOutputOffsetProfiles::CurrentProfile() != HdmiOutputOffsetProfiles::None;
                if (TestNotes::Notes.HDMIAudioDeviceUseOutputOffsetProfile)
                {
                    ImGui::BeginDisabled();
                    strcpy_s(TestNotes::Notes.HDMIAudioDevice, HdmiOutputOffsetProfiles::CurrentProfile()->Name.c_str());
                    ImGui::InputText("HDMI Audio Extractor", TestNotes::Notes.HDMIAudioDevice, IM_ARRAYSIZE(TestNotes::Notes.HDMIAudioDevice));
                    ImGui::EndDisabled();
                }
                else
                {
                    ImGui::InputText("HDMI Audio Extractor", TestNotes::Notes.HDMIAudioDevice, IM_ARRAYSIZE(TestNotes::Notes.HDMIAudioDevice), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                }

                TestNotes::Notes.DACUseLatencyProfile = DacLatencyProfiles::CurrentProfile() != &DacLatencyProfiles::None;
                if (TestNotes::Notes.DACUseLatencyProfile)
                {
                    ImGui::BeginDisabled();
                    strcpy_s(TestNotes::Notes.DAC, DacLatencyProfiles::CurrentProfile()->Name.c_str());
                    ImGui::InputText("ARC, eARC, or S/PDIF DAC", TestNotes::Notes.DAC, IM_ARRAYSIZE(TestNotes::Notes.DAC));
                    ImGui::EndDisabled();
                }
                else
                {
                    ImGui::InputText("ARC, eARC, or S/PDIF DAC", TestNotes::Notes.DAC, IM_ARRAYSIZE(TestNotes::Notes.DAC), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                }

                ImGui::PushFont(FontHelper::BoldFont);
                ImGui::Text("Device Under Test");
                ImGui::PopFont();
                ImGui::InputText("Model", TestNotes::Notes.DutModel, IM_ARRAYSIZE(TestNotes::Notes.DutModel), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                ImGui::InputText("Firmware Version", TestNotes::Notes.DutFirmwareVersion, IM_ARRAYSIZE(TestNotes::Notes.DutFirmwareVersion), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                GuiHelper::OtherCombo("Output Type", "Output Type (Other)", &TestNotes::Notes.DutOutputTypeIndex, TestNotes::Notes.DutOutputTypeOptions, IM_ARRAYSIZE(TestNotes::Notes.DutOutputTypeOptions), TestNotes::Notes.DutOutputTypeOther, IM_ARRAYSIZE(TestNotes::Notes.DutOutputTypeOther));
                ImGui::InputText("Video Mode", TestNotes::Notes.DutVideoMode, IM_ARRAYSIZE(TestNotes::Notes.DutVideoMode), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                ImGui::InputText("Audio Settings", TestNotes::Notes.DutAudioSettings, IM_ARRAYSIZE(TestNotes::Notes.DutAudioSettings), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                ImGui::InputText("Other Settings", TestNotes::Notes.DutOtherSettings, IM_ARRAYSIZE(TestNotes::Notes.DutOtherSettings), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
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
                GuiHelper::OtherCombo("Signal Resolution", "Signal Resolution (Other)", &TestNotes::Notes.VideoResIndex, TestNotes::Notes.VideoResOptions, IM_ARRAYSIZE(TestNotes::Notes.VideoResOptions), TestNotes::Notes.VideoResolutionOther, IM_ARRAYSIZE(TestNotes::Notes.VideoResolutionOther));
                ImGui::InputText("Refresh Rate", TestNotes::Notes.VideoRefreshRate, IM_ARRAYSIZE(TestNotes::Notes.VideoRefreshRate), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                ImGui::InputText("Bit Depth", TestNotes::Notes.VideoBitDepth, IM_ARRAYSIZE(TestNotes::Notes.VideoBitDepth), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                ImGui::InputText("Color Format", TestNotes::Notes.VideoColorFormat, IM_ARRAYSIZE(TestNotes::Notes.VideoColorFormat), ImGuiInputTextFlags_CallbackCharFilter, (ImGuiInputTextCallback)GuiHelper::CsvInputFilter);
                GuiHelper::OtherCombo("Color Space", "Color Space (Other)", &TestNotes::Notes.VideoColorSpaceIndex, TestNotes::Notes.VideoColorSpaceOptions, IM_ARRAYSIZE(TestNotes::Notes.VideoColorSpaceOptions), TestNotes::Notes.VideoColorSpaceOther, IM_ARRAYSIZE(TestNotes::Notes.VideoColorSpaceOther));
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
            std::string audioLatencyType = "Audio";
            if (TestNotes::Notes.DutOutputType() != "")
            {
                audioLatencyType = TestNotes::Notes.DutOutputType();
            }

            ImGui::PushFont(FontHelper::BoldFont);
            ImGui::Text("Measurement Results");
            ImGui::PopFont();
            ImGui::Text(testManager->TestFileString.c_str());
            if (ImGui::Button("Open Results Folder"))
            {
                ShellExecuteA(NULL, "explore", StringHelper::GetRootPath(APP_FOLDER).c_str(), NULL, NULL, SW_SHOWDEFAULT);
            }
            ImGui::Spacing();

            if (ImGui::BeginTabBar("ResultsTabs", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Summary"))
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
                    ImGui::Text(std::format("Stereo {} Latency: {}", audioLatencyType, stereoLatency).c_str());
                    ImGui::PopFont();
                    ImGui::Text(stereoFormat.c_str());
                    ImGui::Spacing();
                    ImGui::PushFont(FontHelper::HeaderFont);
                    ImGui::Text(std::format("5.1 {} Latency: {}", audioLatencyType, fiveOneLatency).c_str());
                    ImGui::PopFont();
                    ImGui::Text(fiveOneFormat.c_str());
                    ImGui::Spacing();
                    ImGui::PushFont(FontHelper::HeaderFont);
                    ImGui::Text(std::format("7.1 {} Latency: {}", audioLatencyType, sevenOneLatency).c_str());
                    ImGui::PopFont();
                    ImGui::Text(sevenOneFormat.c_str());

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
                            ImGui::Text(std::format("Average {} Latency: {} ms", audioLatencyType, round(avgResult.AverageLatency())).c_str());
                            ImGui::PopFont();
                            ImGui::Text(std::format("(rounded from: {} ms)", avgResult.AverageLatency()).c_str());
                            ImGui::Spacing();
                            ImGui::Text(std::format("Min {} Latency: {} ms", audioLatencyType, avgResult.MinLatency()).c_str());
                            ImGui::Text(std::format("Max {} Latency: {} ms", audioLatencyType, avgResult.MaxLatency()).c_str());
                            ImGui::Text(std::format("Valid Measurements: {}", avgResult.Offsets.size()).c_str());
                            ImGui::Spacing();
                            ImGui::Text(std::format("Output Offset Profile: {}", avgResult.OutputOffsetProfileName).c_str());
                            ImGui::Text(std::format("Output Offset Value: {} ms", avgResult.OutputOffsetFromProfile).c_str());
                            ImGui::Text(std::format("Verified Accuracy: {}", avgResult.Verified ? "Yes" : "No").c_str());
                            GuiHelper::VerifiedHelp();
                            ImGui::Text(std::format("DAC: {}", avgResult.ReferenceDacName).c_str());
                            ImGui::Text(std::format("DAC Audio Latency: {} ms", avgResult.ReferenceDacLatency).c_str());

                            break;
                        }
                    }
                    ImGui::EndGroup();
                    ImGui::Spacing();
                    if (ImGui::TreeNode("Failed Formats"))
                    {
                        ImGui::PushTextWrapPos(650 * DpiScale);
                        ImGui::TextWrapped(std::format("At some point during the test, {} consecutive measurements were invalid for the following formats. To prevent this from happening, try increasing the \"Attempts Before Skipping a Format\" under the Advanced Configuration of the Measurement Config.", TestConfiguration::AttemptsBeforeFail).c_str());
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
        ImGui::Text("AV Latency.com HDMI to ARC, eARC, or S/PDIF Latency Measurement Tool\n\nFind out more about audio/video latency, input lag, and lip sync error at avlatency.com\nFind out more about this tool at github.com/AVLatency/Latency-Measurement");
        
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
        ImGui::Text("Set the EDID mode of your HDMI Audio Extractor to match the EDID of your DUT.");
        ImGui::Spacing();

        float imageScale = 0.6 * Gui::DpiScale;
        ImGui::Image((void*)resources.EDIDModeTexture, ImVec2(resources.EDIDModeTextureWidth * imageScale, resources.EDIDModeTextureHeight * imageScale));

        ImGui::Text("This EDID mode is often labelled \"TV\" on HDMI audio extractors.");
        ImGui::Spacing();

        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            RefreshAudioEndpoints();
            state = MeasurementToolGuiState::SelectAudioDevices;
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

        testManager = new TestManager(outputAudioEndpoints[outputDeviceIndex], inputAudioEndpoints[inputDeviceIndex], selectedFormats, fileString, APP_FOLDER, (IResultsWriter&)ResultsWriter::Writer, HdmiOutputOffsetProfiles::CurrentProfile(), DacLatencyProfiles::CurrentProfile());
    }
}

void Gui::SetDutOutputType()
{
    switch (DacLatencyProfiles::CurrentProfile()->InputType)
    {
    case DacLatencyProfile::DacInputType::ARC:
        TestNotes::Notes.DutOutputTypeIndex = 1;
        break;
    case DacLatencyProfile::DacInputType::eARC:
        TestNotes::Notes.DutOutputTypeIndex = 2;
        break;
    case DacLatencyProfile::DacInputType::SPDIF_Optical:
        TestNotes::Notes.DutOutputTypeIndex = 3;
        break;
    case DacLatencyProfile::DacInputType::SPDIF_Coax:
        TestNotes::Notes.DutOutputTypeIndex = 4;
        break;
    case DacLatencyProfile::DacInputType::Unknown:
    default:
        TestNotes::Notes.DutOutputTypeIndex = 0;
        break;
    }
}