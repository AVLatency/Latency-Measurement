#pragma once
#include "AVLTexture.h"

class ProfileResources
{
public:
	// These textures should match the profiles declared in OutputOffsetProfiles.h and DacLatencyProfiles.h!

	AVLTexture Hdmi_HDV_MB01_Texture;
	AVLTexture Spdif_HDV_MB01_Texture;
	AVLTexture Analog_YSplitter_Texture;
	AVLTexture Arc_ExampleDevice_Texture;
	AVLTexture EArc_ExampleDevice_Texture;

	AVLTexture Dac_CV121AD_ARC_Texture;
	AVLTexture Dac_CV121AD_SPDIF_COAX_Texture;
	AVLTexture Dac_CV121AD_SPDIF_OPTICAL_Texture;
	AVLTexture Dac_SHARCV1_EARC_Texture;
};

