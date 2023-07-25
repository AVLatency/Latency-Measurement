#include "DacLatencyProfiles.h"

DacLatencyProfile DacLatencyProfiles::CV121AD_ARC;
DacLatencyProfile DacLatencyProfiles::CV121AD_SPDIF_COAX;
DacLatencyProfile DacLatencyProfiles::CV121AD_SPDIF_OPTICAL;
DacLatencyProfile DacLatencyProfiles::SHARCV1_EARC;
DacLatencyProfile DacLatencyProfiles::None;
std::vector<DacLatencyProfile*> DacLatencyProfiles::Profiles;
int DacLatencyProfiles::SelectedProfileIndex = 0;

void DacLatencyProfiles::InitializeProfiles(ProfileResources& resources)
{
	CV121AD_ARC.Name = "CV121AD: ARC Input";
	CV121AD_ARC.Image = resources.Dac_CV121AD_ARC_Texture;
	CV121AD_ARC.Latency = 0.123; // 2 channel 48 kHz 16 bit. For other formats, this could be off by up to 0.1 ms.
	CV121AD_ARC.InputType = DacLatencyProfile::DacInputType::ARC;
	Profiles.push_back(&CV121AD_ARC);

	CV121AD_SPDIF_COAX.Name = "CV121AD: S/PDIF Coax Input";
	CV121AD_ARC.Image = resources.Dac_CV121AD_SPDIF_COAX_Texture;
	CV121AD_SPDIF_COAX.Latency = 0.118; // 2 channel 48 kHz 16 bit. For other formats, this could be off by up to 0.1 ms.
	CV121AD_SPDIF_COAX.InputType = DacLatencyProfile::DacInputType::SPDIF_Coax;
	Profiles.push_back(&CV121AD_SPDIF_COAX);

	CV121AD_SPDIF_OPTICAL.Name = "CV121AD: S/PDIF Optical Input";
	CV121AD_ARC.Image = resources.Dac_CV121AD_SPDIF_OPTICAL_Texture;
	CV121AD_SPDIF_OPTICAL.Latency = 0.118; // 2 channel 48 kHz 16 bit. For other formats, this could be off by up to 0.1 ms.
	CV121AD_SPDIF_OPTICAL.InputType = DacLatencyProfile::DacInputType::SPDIF_Optical;
	Profiles.push_back(&CV121AD_SPDIF_OPTICAL);

	SHARCV1_EARC.Name = "SHARK v1: eARC Input";
	CV121AD_ARC.Image = resources.Dac_SHARCV1_EARC_Texture;
	SHARCV1_EARC.Latency = 0;
	SHARCV1_EARC.InputType = DacLatencyProfile::DacInputType::eARC;
	Profiles.push_back(&SHARCV1_EARC);

	None.Name = "Other";
	None.Latency = 0;
	None.InputType = DacLatencyProfile::DacInputType::Unknown;
	None.isNoLatency = true;
	Profiles.push_back(&None);
}

DacLatencyProfile* DacLatencyProfiles::CurrentProfile()
{
	return Profiles[SelectedProfileIndex];
}