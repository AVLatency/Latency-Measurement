#pragma once
#include "stb_image.h"
#include "d3d11.h"

bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height, ID3D11Device* g_pd3dDevice);
bool LoadTextureFromData(unsigned char* res_data, DWORD res_size, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height, ID3D11Device* g_pd3dDevice);