#pragma once
#include <d3d11.h>

class Resources
{
public:
	Resources(HINSTANCE hInstance, ID3D11Device* g_pd3dDevice);

	// Resources
	ID3D11ShaderResourceView* CableMapTexture = NULL;
	int CableMapTextureWidth = 0;
	int CableMapTextureHeight = 0;

	ID3D11ShaderResourceView* EDIDModeTexture = NULL;
	int EDIDModeTextureWidth = 0;
	int EDIDModeTextureHeight = 0;

	ID3D11ShaderResourceView* HDV_MB01Texture = NULL;
	int HDV_MB01TextureWidth = 0;
	int HDV_MB01TextureHeight = 0;

	ID3D11ShaderResourceView* WindowsDisplaySettingsTexture = NULL;
	int WindowsDisplaySettingsTextureWidth = 0;
	int WindowsDisplaySettingsTextureHeight = 0;
};

