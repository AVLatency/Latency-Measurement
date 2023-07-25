#include "Resources.h"
#include "resource.h"
#include "stb_image_dx11.h"
#include "imgui.h"
#include "ResourceLoader.h"

Resources::Resources(HINSTANCE hInstance, ID3D11Device* g_pd3dDevice)
{
    LoadTexture(hInstance, g_pd3dDevice, HDMI_CABLE_MAP, HdmiCableMapTexture);
    LoadTexture(hInstance, g_pd3dDevice, SPDIF_CABLE_MAP, SpdifCableMapTexture);
    LoadTexture(hInstance, g_pd3dDevice, ARC_CABLE_MAP, ArcCableMapTexture);
    LoadTexture(hInstance, g_pd3dDevice, EARC_CABLE_MAP, EArcCableMapTexture);
    LoadTexture(hInstance, g_pd3dDevice, ANALOG_CABLE_MAP, AnalogCableMapTexture);
    LoadTexture(hInstance, g_pd3dDevice, HDMI_EDID_MODE, EDIDModeTexture);
    LoadTexture(hInstance, g_pd3dDevice, HDMI_WINDOWS_DISPLAY_SETTINGS, WindowsDisplaySettingsTexture);
    LoadTexture(hInstance, g_pd3dDevice, VOLUME_ADJUST_EXAMPLE, VolumeAdjustExampleTexture);

    LoadTexture(hInstance, g_pd3dDevice, HDMI_HDV_MB01, HDV_MB01Texture);
    LoadTexture(hInstance, g_pd3dDevice, ANALOG_Y_SPLITTER, AnalogYSplitterTexture);
    LoadTexture(hInstance, g_pd3dDevice, ARC_EXAMPLE_DEVICE, ArcExampleDeviceTexture);
    LoadTexture(hInstance, g_pd3dDevice, EARC_EXAMPLE_DEVICE, EArcExampleDeviceTexture);
}

void Resources::LoadTexture(HINSTANCE hInstance, ID3D11Device* g_pd3dDevice, UINT type, AVLTexture& image)
{
    ResourceLoader::LoadImage(hInstance, g_pd3dDevice, type, &image.TextureData, &image.Width, &image.Height);
}