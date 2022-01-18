#pragma once
#include <d3d11.h>

class ResourceLoader
{
public:
	static void LoadImage(HINSTANCE hInstance, ID3D11Device* g_pd3dDevice, int resourceID, ID3D11ShaderResourceView** outTexture, int* outWidth, int* outHeight);
};

