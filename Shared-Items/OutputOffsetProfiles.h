#pragma once
#include "OutputOffsetProfile.h"
#include <vector>
#include "ProfileResources.h"

/// <summary>
/// Describes the offset between digital output and analog output of
/// dual-out reference devices.
/// </summary>
class OutputOffsetProfiles
{
public:
	/// <summary>
	/// Represents a subset of the available output offset profiles.
	/// Generally this is used to filter the profiles to only
	/// include profiles with a specific OutputType.
	/// </summary>
	struct ProfilesSubset
	{
		int SubsetSelectedIndex = 0;
		std::vector<int> ProfileIndeces;
	};

	static OutputOffsetProfile* Hdmi_HDV_MB01;
	static OutputOffsetProfile* Hdmi_None;
	static OutputOffsetProfile* Hdmi_None_CurrentWinAudioFormat;

	static OutputOffsetProfile* Spdif_HDV_MB01;
	static OutputOffsetProfile* Spdif_AYSA11;
	static OutputOffsetProfile* Spdif_LiNKFOR_USB_DAC;
	static OutputOffsetProfile* Spdif_None;
	static OutputOffsetProfile* Spdif_None_CurrentWinAudioFormat;

	static OutputOffsetProfile* Arc_ExampleReferenceSetup;
	static OutputOffsetProfile* Arc_None;
	static OutputOffsetProfile* Arc_None_CurrentWinAudioFormat;

	static OutputOffsetProfile* EArc_ExampleReferenceSetup;
	static OutputOffsetProfile* EArc_None;
	static OutputOffsetProfile* EArc_None_CurrentWinAudioFormat;

	static OutputOffsetProfile* Analog_BasicYSplitter;
	static OutputOffsetProfile* Analog_None;

	static OutputOffsetProfile* HdmiAudioPassthrough_HDV_MB01;
	static OutputOffsetProfile* HdmiAudioPassthrough_None;
	static OutputOffsetProfile* HdmiAudioPassthrough_None_CurrentWinAudioFormat;

	static std::vector<OutputOffsetProfile*> Profiles;
	static int SelectedProfileIndex;

	/// <summary>
	/// These subsets include only a specific OutputType that map
	/// to the index of the master Profiles vector.
	/// </summary>
	static std::map<OutputOffsetProfile::OutputType, ProfilesSubset*> Subsets;

	static void InitializeProfiles(ProfileResources& resources);

	static OutputOffsetProfile* CurrentProfile();

private:
	static void PrepareSubsetListsForGui();
	static void PrepareOffsetStringsForGui();

	static OutputOffsetProfile::OutputOffset None_GetOffset(int numChannels, int sampleRate, int bitDepth);

	static OutputOffsetProfile::OutputOffset Hdmi_HDV_MB01_GetOffset(int numChannels, int sampleRate, int bitDepth);

	static bool CommonSpdifFormatFilter(WAVEFORMATEX*);
	static OutputOffsetProfile::OutputOffset Spdif_HDV_MB01_GetOffset(int numChannels, int sampleRate, int bitDepth);
	static OutputOffsetProfile::OutputOffset Spdif_AYSA11_GetOffset(int numChannels, int sampleRate, int bitDepth);
	static OutputOffsetProfile::OutputOffset Spdif_LiNKFOR_USB_DAC_GetOffset(int numChannels, int sampleRate, int bitDepth);

	static OutputOffsetProfile::OutputOffset Arc_ExampleReferenceSetup_GetOffset(int numChannels, int sampleRate, int bitDepth);

	static OutputOffsetProfile::OutputOffset EArc_ExampleReferenceSetup_GetOffset(int numChannels, int sampleRate, int bitDepth);

	static OutputOffsetProfile::OutputOffset Analog_BasicYSplitter_GetOffset(int numChannels, int sampleRate, int bitDepth);
};

