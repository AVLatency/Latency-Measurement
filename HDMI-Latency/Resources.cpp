#include "Resources.h"
#include "resource.h"
#include "stb_image_dx11.h"
#include "imgui.h"
#include "ResourceLoader.h"

Resources::Resources(HINSTANCE hInstance, ID3D11Device* g_pd3dDevice)
{
    ResourceLoader::LoadImage(hInstance, g_pd3dDevice, HDMI_CABLE_MAP, &CableMapTexture, &CableMapTextureWidth, &CableMapTextureHeight);
    ResourceLoader::LoadImage(hInstance, g_pd3dDevice, HDMI_EDID_MODE, &EDIDModeTexture, &EDIDModeTextureWidth, &EDIDModeTextureHeight);
    ResourceLoader::LoadImage(hInstance, g_pd3dDevice, HDMI_HDV_MB01, &HDV_MB01Texture, &HDV_MB01TextureWidth, &HDV_MB01TextureHeight);
    ResourceLoader::LoadImage(hInstance, g_pd3dDevice, HDMI_WINDOWS_DISPLAY_SETTINGS, &WindowsDisplaySettingsTexture, &WindowsDisplaySettingsTextureWidth, &WindowsDisplaySettingsTextureHeight);
    ResourceLoader::LoadImage(hInstance, g_pd3dDevice, VOLUME_ADJUST_EXAMPLE, &VolumeAdjustExampleTexture, &VolumeAdjustExampleTextureWidth, &VolumeAdjustExampleTextureHeight);
}