#pragma once
#include <d3d11.h>

struct AVLTexture
{
	ID3D11ShaderResourceView* TextureData = NULL;
	int Width = 0;
	int Height = 0;
};

