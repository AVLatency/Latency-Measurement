#pragma once
#include <d3d11.h>
#include "AVLTexture.h"

class Resources
{
public:
	Resources(HINSTANCE hInstance, ID3D11Device* g_pd3dDevice);

	// Resources
	AVLTexture CableMapTexture = AVLTexture();
	AVLTexture EDIDModeTexture = AVLTexture();
	AVLTexture HDV_MB01Texture = AVLTexture();
	AVLTexture CV121ADTexture = AVLTexture();
	AVLTexture SHARCv1Texture = AVLTexture();
	AVLTexture WindowsDisplaySettingsTexture = AVLTexture();
	AVLTexture HdmiToDigitalAudioDefinitionTexture = AVLTexture();
	AVLTexture VolumeAdjustExampleTexture = AVLTexture();

private:
	void LoadTexture(HINSTANCE hInstance, ID3D11Device* g_pd3dDevice, UINT type, AVLTexture& image);
};

