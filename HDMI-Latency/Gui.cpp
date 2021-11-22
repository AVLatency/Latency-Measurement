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
        ImGui::Text("Getting Started");
        ImGui::Spacing();
        ImGui::Text("Step 1: Input/Output Setup");
        ImGui::Spacing();
        ImGui::Text("Step 2: Measurement Config");
        ImGui::Spacing();

        ImGui::TableSetColumnIndex(1);

        if (ImGui::BeginTabBar("Main Tabs"))
        {
            if (state == GuiState::GettingStarted)
            {
                if (ImGui::BeginTabItem("Getting Started"))
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
                        
                    }

                    ImGui::EndTabItem();
                }
            }
            //else if (state == AppState::Calibration)
            //{
            //    if (ImGui::BeginTabItem("Calibration"))
            //    {

            //        ImGui::EndTabItem();
            //    }
            //}
            //else if (state == AppState::Measurement)
            //{
            //    if (ImGui::BeginTabItem("Measurement"))
            //    {
            //    }
            //}

            ImGui::EndTabBar();
        }

        ImGui::EndTable();
    }

    ImGui::End();

    // Menu modal dialogs:

    if (openAboutDialog)
    {
        ImGui::OpenPopup("About");
    }
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("About", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("AV Latency.com HDMI Latency Measurement Tool\n\nFind out more about audio/video latency, input lag, and lip sync error at avlatency.com\nFind out more about this tool at github.com/AVLatency/Latency-Measurement");
        ImGui::Separator();

        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }

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
