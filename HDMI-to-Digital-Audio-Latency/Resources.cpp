#include "Resources.h"
#include "resource.h"
#include "stb_image_dx11.h"
#include "imgui.h"
#include "ResourceLoader.h"

Resources::Resources(HINSTANCE hInstance, ID3D11Device* g_pd3dDevice)
{
    LoadTexture(hInstance, g_pd3dDevice, HDMI_CABLE_MAP, CableMapTexture);
    LoadTexture(hInstance, g_pd3dDevice, HDMI_EDID_MODE, EDIDModeTexture);
    LoadTexture(hInstance, g_pd3dDevice, HDMI_HDV_MB01, HDV_MB01Texture);
    LoadTexture(hInstance, g_pd3dDevice, DEVICE_CV121AD, CV121ADTexture);
    LoadTexture(hInstance, g_pd3dDevice, DEVICE_SHARCV1, SHARCv1Texture);
    LoadTexture(hInstance, g_pd3dDevice, HDMI_WINDOWS_DISPLAY_SETTINGS, WindowsDisplaySettingsTexture);
    LoadTexture(hInstance, g_pd3dDevice, HDMI_TO_DIGITAL_AUDIO_DEFINITION, HdmiToDigitalAudioDefinitionTexture);
    LoadTexture(hInstance, g_pd3dDevice, VOLUME_ADJUST_EXAMPLE, VolumeAdjustExampleTexture);
}

void Resources::LoadTexture(HINSTANCE hInstance, ID3D11Device* g_pd3dDevice, UINT type, AVLTexture& image)
{
    ResourceLoader::LoadImage(hInstance, g_pd3dDevice, type, &image.TextureData, &image.Width, &image.Height);
}