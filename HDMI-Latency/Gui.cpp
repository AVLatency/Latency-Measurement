#include "Gui.h"
#include "imgui.h"
#include "resource.h"

bool Gui::DoGui()
{
    bool done = false;

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(main_viewport->WorkSize.x, main_viewport->WorkSize.y), ImGuiCond_Always);
    //ImGui::SetNextWindowContentSize(ImVec2(1000, 0.0f));
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
        "You can use either a line in or mic input on your computer, but when using certain microphones you may find the mic input works better.\n\n"
        "To record the Device Under Test (DUT) you can use a microphone or directly connect to the headphone or speaker output of the DUT.\n"
        "- If you use a microphone, make sure to position it as close as possible to the speaker (the tweeter if there are separate speaker components) because sound travels slowly.\n"
        "- If you use DUT headphone output, remember that speaker and headphone output can sometimes have a very different latency.\n"
        "- If you directly connect to DUT speaker output, remember to start the volume low as some devices may be capable of high voltage outputs that could, theoretically, damage your audio input device.\n\n"
        "Your \"HDMI Audio Device\" must be capabile of analog audio output AND HDMI audio output at the same time. The time offset between analog audio output and HDMI audio output must be known. A list of capable devices can be found on the GitHub wiki.\n\n"
        "GitHub Wiki: github.com/AVLatency/Latency-Measurement/wiki");
    float cableMapScale = 0.7;
    ImGui::Image((void*)resources.my_texture, ImVec2(resources.my_image_width * cableMapScale, resources.my_image_height * cableMapScale));

    if (ImGui::BeginTable("MainViewTopLevelTable", 2, ImGuiTableFlags_Borders))
    {
        ImGui::TableSetupColumn("MainViewTopLevelTableCol1", ImGuiTableColumnFlags_WidthFixed);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("1) Getting Started");
        ImGui::Spacing();
        ImGui::Text("2) Input/Output Devices");
        ImGui::Spacing();
        ImGui::Text("3) Adjust Volumes");
        ImGui::Spacing();
        ImGui::Text("4) Measurement Config");
        ImGui::Spacing();
        ImGui::Text("5) Measurement");
        ImGui::Spacing();
        ImGui::Text("6) Results");
        ImGui::Spacing();

        ImGui::TableSetColumnIndex(1);

        switch (state)
        {
        case GuiState::GettingStarted:
        {

            ImGui::Text("Welcome to the AV Latency.com HDMI latency measurement tool!");
            ImGui::Spacing();
            ImGui::Text("Before starting, please connect your cables as described in the diagram above.");
            ImGui::Spacing();
            ImGui::Text("You can find help text by hovering your mouse over these:");
            ImGui::SameLine(); HelpMarker("Click \"Let's Go!\" once you've connected all your HDMI and audio cables to get started!");
            ImGui::Spacing();

            if (ImGui::Button("Let's Go!"))
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
            bool disabled = state > GuiState::SelectAudioDevices;
            if (disabled)
            {
                ImGui::BeginDisabled();
            }

            const char* items[] = { "Device 1", "Device 2" };
            ImGui::Combo("Output Device", &outputDeviceIndex, items, 2);
            ImGui::Combo("Input Device", &inputDeviceIndex, items, 2);

            if (ImGui::Button("Adjust Volumes"))
            {
                state = GuiState::AdjustVolume;
            }

            if (disabled)
            {
                ImGui::EndDisabled();
                disabled = false;
            }

            if (state >= GuiState::AdjustVolume)
            {
                ImGui::Spacing();
                ImGui::Text("Left Channel (HDMI Audio Device)");
                ImGui::SameLine(); HelpMarker("Adjust the volume of your input device through the Windows control panel to make the monitor match the refrence image. "
                    "You may need to turn down the Microphone Boost in the Levels section of Additional device properties.");

                if (ImGui::BeginTable("LeftChannelVolumeTable", 2, ImGuiTableFlags_Borders))
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Reference Image");
                    // TODO: wave view
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("Monitor");
                    // TODO: wave view
                    ImGui::EndTable();
                }

                ImGui::Spacing();
                ImGui::Text("Right Channel (DUT)");
                ImGui::SameLine(); HelpMarker("Adjust the output volume of your Device Under Test (DUT) to give a smooth and clear normalized recording.");
                if (ImGui::BeginTable("RightChannelVolumeTable", 2, ImGuiTableFlags_Borders))
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("Monitor (Raw)");
                    // TODO: wave view
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Reference Image (Normalized)");
                    // TODO: wave view
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("Monitor (Normalized)");
                    // TODO: wave view
                    ImGui::EndTable();
                }

                if (state == GuiState::AdjustVolume)
                {
                    if (ImGui::Button("Cancel"))
                    {
                        state = GuiState::SelectAudioDevices;
                        // TODO: state = GuiState::CancellingAdjustVolume;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Finish"))
                    {
                        state = GuiState::MeasurementConfig;
                        // TODO: state = GuiState::FinishingAdjustVolume;
                    }
                }
            }
        }
            break;
        case GuiState::MeasurementConfig:
        case GuiState::Measuring:
        case GuiState::CancellingMeasuring:
        {

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
        ImGui::Text("For HDMI audio extractors, set the switch to \"TV\" or \"Passthrough\".");

        // TODO: Image of switch set to the TV position

        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            state = GuiState::SelectAudioDevices;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    return done;
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
void Gui::HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
