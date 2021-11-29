#include "FontHelper.h"
#include <cmath>
#include "imgui_impl_dx11.h"

ImFont* FontHelper::RegularFont = nullptr;
ImFont* FontHelper::BoldFont = nullptr;
ImFont* FontHelper::HeaderFont = nullptr;

bool FontHelper::initialized = false;

void FontHelper::LoadFonts(HINSTANCE hInstance, ImGuiIO& io, int regularFontId, int boldFontId, float dpiScale)
{
    if (initialized)
    {
        // TODO: fix memory leak with fonts from previous DPI scales.
        // The following seems to crash. Deleting the ImFont objects also results in a crash later on.
        // ...Not sure how to delete the old ones from memory.
        //io.Fonts->Clear();
    }

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !

    // First is default, don't need to save a ref to it:
    RegularFont = LoadFont(hInstance, io, regularFontId, std::floor(dpiScale * 15));
    IM_ASSERT(RegularFont != NULL);
    BoldFont = LoadFont(hInstance, io, boldFontId, std::floor(dpiScale * 15));
    IM_ASSERT(BoldFont != NULL);
    HeaderFont = LoadFont(hInstance, io, boldFontId, std::floor(dpiScale * 20));
    IM_ASSERT(HeaderFont != NULL);

    // TODO: Unicode support: ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != NULL);

    if (initialized)
    {
        io.Fonts->Build();
        ImGui_ImplDX11_InvalidateDeviceObjects(); // Fixes crash https://github.com/ocornut/imgui/issues/2311#issuecomment-460039964
    }
    initialized = true;
}

ImFont* FontHelper::LoadFont(HINSTANCE hInstance, ImGuiIO& io, int fontResourceId, float fontSize)
{
    // Instructions on how to include binary data in Visual Studio resources: https://blog.kowalczyk.info/article/zy/embedding-binary-resources-on-windows.html
    HGLOBAL     res_handle = NULL;
    HRSRC       res;
    unsigned char* res_data;
    DWORD       res_size;

    // NOTE: providing g_hInstance is important, NULL might not work
    res = FindResource(hInstance, MAKEINTRESOURCE(fontResourceId), RT_RCDATA);
    if (!res)
        return nullptr;
    res_handle = LoadResource(NULL, res);
    if (!res_handle)
        return nullptr;
    res_data = (unsigned char*)LockResource(res_handle);
    res_size = SizeofResource(NULL, res);

    return io.Fonts->AddFontFromMemoryTTF(res_data, res_size, fontSize);
}