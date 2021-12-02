#include "Resources.h"
#include "resource.h"
#include "stb_image_dx11.h"
#include "imgui.h"


Resources::Resources(HINSTANCE hInstance, ID3D11Device* g_pd3dDevice)
{
    LoadImage(hInstance, g_pd3dDevice, HDMI_CABLE_MAP, &CableMapTexture, &CableMapTextureWidth, &CableMapTextureHeight);
    LoadImage(hInstance, g_pd3dDevice, HDMI_EDID_MODE, &EDIDModeTexture, &EDIDModeTextureWidth, &EDIDModeTextureHeight);
    LoadImage(hInstance, g_pd3dDevice, HDMI_HDV_MB01, &HDV_MB01Texture, &HDV_MB01TextureWidth, &HDV_MB01TextureHeight);
    LoadImage(hInstance, g_pd3dDevice, HDMI_WINDOWS_DISPLAY_SETTINGS, &WindowsDisplaySettingsTexture, &WindowsDisplaySettingsTextureWidth, &WindowsDisplaySettingsTextureHeight);
}

void Resources::LoadImage(HINSTANCE hInstance, ID3D11Device* g_pd3dDevice, int resourceID, ID3D11ShaderResourceView** outTexture, int* outWidth, int* outHeight)
{
    // Instructions on how to include binary data in Visual Studio resources: https://blog.kowalczyk.info/article/zy/embedding-binary-resources-on-windows.html
    HGLOBAL     res_handle = NULL;
    HRSRC       res;
    unsigned char* res_data;
    DWORD       res_size;

    // NOTE: providing g_hInstance is important, NULL might not work
    res = FindResource(hInstance, MAKEINTRESOURCE(resourceID), RT_RCDATA);
    if (!res)
        return;
    res_handle = LoadResource(NULL, res);
    if (!res_handle)
        return;
    res_data = (unsigned char*)LockResource(res_handle);
    res_size = SizeofResource(NULL, res);

    bool ret = LoadTextureFromData(res_data, res_size, outTexture, outWidth, outHeight, g_pd3dDevice);
    IM_ASSERT(ret);
}