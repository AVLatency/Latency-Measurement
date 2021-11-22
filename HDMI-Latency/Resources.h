#pragma once
#include <d3d11.h>

class Resources
{
public:
	Resources(HINSTANCE hInstance, ID3D11Device* g_pd3dDevice);

	// Resources
	ID3D11ShaderResourceView* my_texture = NULL;
	int my_image_width = 0;
	int my_image_height = 0;
};

