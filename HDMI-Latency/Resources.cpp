#include "Resources.h"
#include "resource.h"
#include "stb_image_dx11.h"
#include "imgui.h"

Resources::Resources(HINSTANCE hInstance, ID3D11Device* g_pd3dDevice)
{
    // Instructions on how to include binary data in Visual Studio resources: https://blog.kowalczyk.info/article/zy/embedding-binary-resources-on-windows.html
    HGLOBAL     res_handle = NULL;
    HRSRC       res;
    unsigned char* res_data;
    DWORD       res_size;

    // NOTE: providing g_hInstance is important, NULL might not work
    res = FindResource(hInstance, MAKEINTRESOURCE(HDMI_CABLE_MAP), RT_RCDATA);
    if (!res)
        return;
    res_handle = LoadResource(NULL, res);
    if (!res_handle)
        return;
    res_data = (unsigned char*)LockResource(res_handle);
    res_size = SizeofResource(NULL, res);

    bool ret = LoadTextureFromData(res_data, res_size, &my_texture, &my_image_width, &my_image_height, g_pd3dDevice);
    IM_ASSERT(ret);
}
