#include "Gui.h"
#include "imgui.h"
#include "resource.h"
#include "AudioEndpointHelper.h"
#include "FontHelper.h"
#include "shellapi.h"

float Gui::DpiScale = 1.0f;
bool Gui::DpiScaleChanged = false;
float Gui::PreviousDpiScale = 1.0f;

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

    ImGui::Text("Hello, world!");

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
}
