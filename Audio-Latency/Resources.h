#pragma once
#include <d3d11.h>
#include "AVLTexture.h"

class Resources
{
public:
	Resources(HINSTANCE hInstance, ID3D11Device* g_pd3dDevice);

	// Resources
	AVLTexture HdmiCableMapTexture = AVLTexture();
	AVLTexture SpdifCableMapTexture = AVLTexture();
	AVLTexture ArcCableMapTexture = AVLTexture();
	AVLTexture EArcCableMapTexture = AVLTexture();
	AVLTexture AnalogCableMapTexture = AVLTexture();
	AVLTexture EDIDModeTexture = AVLTexture();
	AVLTexture WindowsDisplaySettingsTexture = AVLTexture();
	AVLTexture VolumeAdjustExampleTexture = AVLTexture();

	AVLTexture HDV_MB01Texture = AVLTexture();
	AVLTexture AnalogYSplitterTexture = AVLTexture();

private:
	void LoadTexture(HINSTANCE hInstance, ID3D11Device* g_pd3dDevice, UINT type, AVLTexture& image);
};

